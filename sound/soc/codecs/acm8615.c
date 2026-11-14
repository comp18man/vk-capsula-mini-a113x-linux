// SPDX-License-Identifier: GPL-2.0-only
/*
 * ACME Semiconductor ACM8615 mono class-D amplifier
 *
 */

#include <linux/bitfield.h>
#include <linux/delay.h>
#include <linux/firmware.h>
#include <linux/i2c.h>
#include <linux/lockdep.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/property.h>
#include <linux/regmap.h>

#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/tlv.h>

#include "amp-hp-det.h"

#define ACM8615_PAGE_LEN		0x100
#define ACM8615_PAGES			8
#define ACM8615_REG(page, reg)		((page) * ACM8615_PAGE_LEN + (reg))
#define ACM8615_MAX_REG			(ACM8615_PAGES * ACM8615_PAGE_LEN - 1)

#define ACM8615_PAGE_SEL		0x00
#define ACM8615_AMP_CTRL1		0x01
#define  ACM8615_FAULT_CLR		BIT(7)
#define ACM8615_STATE_CTRL		0x04
/*
 * The datasheet has the mute in bit 2. The parts on the board (die ID 1)
 * do not keep bit 2 and mute on bit 3 instead, the left-channel mute of the
 * stereo parts. Both are set, as the vendor driver does.
 */
#define  ACM8615_MUTE			GENMASK(3, 2)
#define  ACM8615_STATE_DIG_OFF		0x0
#define  ACM8615_STATE_PLAY		0x3
#define ACM8615_I2S_FMT1		0x07
#define  ACM8615_44K_INPUT		BIT(7)
#define  ACM8615_44K_EN			BIT(6)
#define  ACM8615_DATA_FMT		GENMASK(5, 4)
#define  ACM8615_DATA_FMT_I2S		0
#define  ACM8615_DATA_FMT_RIGHT_J	2
#define  ACM8615_DATA_FMT_LEFT_J	3
#define  ACM8615_WORD_LEN		GENMASK(1, 0)
#define ACM8615_VOLUME			0x0f
#define  ACM8615_VOLUME_0DB		0xd0
#define ACM8615_DIE_ID			0x15
#define ACM8615_FAULT_RPT1		0x17

#define ACM8615_RATES	(SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 | \
			 SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_88200 | \
			 SNDRV_PCM_RATE_96000)

#define ACM8615_FORMATS	(SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S24_LE | \
			 SNDRV_PCM_FMTBIT_S32_LE)

#define ACM8615_DEFAULT_FW	"acm8615.bin"

struct acm8615_priv {
	struct device *dev;
	struct regmap *regmap;
	struct amp_hp_det hp;
	struct mutex lock;		/* mute state below */
	unsigned int data_fmt;
	bool stream_muted;
	bool switched_on;
};

/*
 * Not in the datasheet. The vendor driver sends it ahead of every profile and
 * the stock init blob opens with it byte for byte; the ACM8625 takes the same
 * sequence with the 0xfd and 0xfe values swapped.
 */
static const struct reg_sequence acm8615_unlock[] = {
	{ ACM8615_STATE_CTRL, ACM8615_STATE_DIG_OFF },
	{ ACM8615_REG(0, 0xfc), 0x86 },
	{ ACM8615_REG(0, 0xfd), 0x15 },
	{ ACM8615_REG(0, 0xfe), 0x25 },
	{ ACM8615_REG(1, 0x02), 0x20 },
};

static int acm8615_apply_mute(struct acm8615_priv *acm)
{
	bool mute = acm->stream_muted || !acm->switched_on ||
		    amp_hp_det_inserted(&acm->hp);

	lockdep_assert_held(&acm->lock);

	return regmap_write(acm->regmap, ACM8615_STATE_CTRL,
			    (mute ? ACM8615_MUTE : 0) | ACM8615_STATE_PLAY);
}

static void acm8615_hp_changed(void *data)
{
	acm8615_apply_mute(data);
}

static void acm8615_clear_faults(struct acm8615_priv *acm)
{
	unsigned int faults;

	if (regmap_read(acm->regmap, ACM8615_FAULT_RPT1, &faults) || !faults)
		return;

	dev_warn_ratelimited(acm->dev, "fault 0x%02x, clearing\n", faults);
	regmap_update_bits(acm->regmap, ACM8615_AMP_CTRL1, ACM8615_FAULT_CLR,
			   ACM8615_FAULT_CLR);
}

static int acm8615_switch_get(struct snd_kcontrol *kcontrol,
			      struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_soc_kcontrol_component(kcontrol);
	struct acm8615_priv *acm = snd_soc_component_get_drvdata(component);

	ucontrol->value.integer.value[0] = acm->switched_on;

	return 0;
}

static int acm8615_switch_put(struct snd_kcontrol *kcontrol,
			      struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_soc_kcontrol_component(kcontrol);
	struct acm8615_priv *acm = snd_soc_component_get_drvdata(component);
	bool on = ucontrol->value.integer.value[0];
	int ret = 0;

	mutex_lock(&acm->lock);
	if (on != acm->switched_on) {
		acm->switched_on = on;
		ret = acm8615_apply_mute(acm);
		if (!ret)
			ret = 1;
	}
	mutex_unlock(&acm->lock);

	return ret;
}

static const DECLARE_TLV_DB_SCALE(acm8615_vol_tlv, -10400, 50, 1);

/* Capped at 0 dB, as far as the stock firmware ever took it. */
static const struct snd_kcontrol_new acm8615_controls[] = {
	SOC_SINGLE_TLV("Playback Volume", ACM8615_VOLUME, 0,
		       ACM8615_VOLUME_0DB, 0, acm8615_vol_tlv),
	SOC_SINGLE_BOOL_EXT("Playback Switch", 0,
			    acm8615_switch_get, acm8615_switch_put),
};

static const struct snd_soc_dapm_widget acm8615_dapm_widgets[] = {
	SND_SOC_DAPM_DAC("DAC", "Playback", SND_SOC_NOPM, 0, 0),
	SND_SOC_DAPM_OUTPUT("OUT"),
};

static const struct snd_soc_dapm_route acm8615_dapm_routes[] = {
	{ "OUT", NULL, "DAC" },
};

static const struct snd_soc_component_driver acm8615_component = {
	.controls		= acm8615_controls,
	.num_controls		= ARRAY_SIZE(acm8615_controls),
	.dapm_widgets		= acm8615_dapm_widgets,
	.num_dapm_widgets	= ARRAY_SIZE(acm8615_dapm_widgets),
	.dapm_routes		= acm8615_dapm_routes,
	.num_dapm_routes	= ARRAY_SIZE(acm8615_dapm_routes),
	.endianness		= 1,
};

static int acm8615_set_fmt(struct snd_soc_dai *dai, unsigned int fmt)
{
	struct acm8615_priv *acm = snd_soc_component_get_drvdata(dai->component);

	if ((fmt & SND_SOC_DAIFMT_CLOCK_PROVIDER_MASK) != SND_SOC_DAIFMT_CBC_CFC)
		return -EINVAL;

	if ((fmt & SND_SOC_DAIFMT_INV_MASK) != SND_SOC_DAIFMT_NB_NF)
		return -EINVAL;

	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
		acm->data_fmt = ACM8615_DATA_FMT_I2S;
		break;
	case SND_SOC_DAIFMT_RIGHT_J:
		acm->data_fmt = ACM8615_DATA_FMT_RIGHT_J;
		break;
	case SND_SOC_DAIFMT_LEFT_J:
		acm->data_fmt = ACM8615_DATA_FMT_LEFT_J;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int acm8615_hw_params(struct snd_pcm_substream *substream,
			     struct snd_pcm_hw_params *params,
			     struct snd_soc_dai *dai)
{
	struct acm8615_priv *acm = snd_soc_component_get_drvdata(dai->component);
	unsigned int val;

	switch (params_width(params)) {
	case 16:
		val = FIELD_PREP(ACM8615_WORD_LEN, 0);
		break;
	case 24:
		val = FIELD_PREP(ACM8615_WORD_LEN, 2);
		break;
	case 32:
		val = FIELD_PREP(ACM8615_WORD_LEN, 3);
		break;
	default:
		return -EINVAL;
	}

	switch (params_rate(params)) {
	case 44100:
	case 88200:
		val |= ACM8615_44K_INPUT | ACM8615_44K_EN;
		break;
	}

	val |= FIELD_PREP(ACM8615_DATA_FMT, acm->data_fmt);

	return regmap_write(acm->regmap, ACM8615_I2S_FMT1, val);
}

static int acm8615_mute_stream(struct snd_soc_dai *dai, int mute, int stream)
{
	struct acm8615_priv *acm = snd_soc_component_get_drvdata(dai->component);
	int ret;

	mutex_lock(&acm->lock);
	if (!mute)
		acm8615_clear_faults(acm);
	acm->stream_muted = mute;
	ret = acm8615_apply_mute(acm);
	mutex_unlock(&acm->lock);

	return ret;
}

static const struct snd_soc_dai_ops acm8615_dai_ops = {
	.set_fmt		= acm8615_set_fmt,
	.hw_params		= acm8615_hw_params,
	.mute_stream		= acm8615_mute_stream,
	.no_capture_mute	= 1,
};

static struct snd_soc_dai_driver acm8615_dai = {
	.name = "acm8615-amplifier",
	.playback = {
		.stream_name	= "Playback",
		.channels_min	= 1,
		.channels_max	= 2,
		.rates		= ACM8615_RATES,
		.formats	= ACM8615_FORMATS,
	},
	.ops = &acm8615_dai_ops,
};

static int acm8615_load_profile(struct acm8615_priv *acm,
				const struct firmware *fw)
{
	unsigned int page = 0;
	size_t i;
	int ret;

	if (fw->size % 2)
		return -EINVAL;

	for (i = 0; i < fw->size; i += 2) {
		if (fw->data[i] == ACM8615_PAGE_SEL &&
		    fw->data[i + 1] >= ACM8615_PAGES)
			return -EINVAL;
	}

	for (i = 0; i < fw->size; i += 2) {
		u8 reg = fw->data[i];
		u8 val = fw->data[i + 1];

		if (reg == ACM8615_PAGE_SEL) {
			page = val;
			continue;
		}

		ret = regmap_write(acm->regmap, ACM8615_REG(page, reg), val);
		if (ret)
			return ret;
	}

	return 0;
}

static int acm8615_setup(struct acm8615_priv *acm)
{
	const char *name = ACM8615_DEFAULT_FW;
	const struct firmware *fw;
	int ret;

	ret = regmap_multi_reg_write(acm->regmap, acm8615_unlock,
				     ARRAY_SIZE(acm8615_unlock));
	if (ret)
		return ret;

	/* The vendor driver gives the DSP this long to come up. */
	fsleep(5000);

	device_property_read_string(acm->dev, "firmware-name", &name);

	ret = request_firmware(&fw, name, acm->dev);
	if (ret) {
		dev_warn(acm->dev, "no profile %s (%d), running on defaults\n",
			 name, ret);
		return 0;
	}

	ret = acm8615_load_profile(acm, fw);
	release_firmware(fw);
	if (ret)
		return dev_err_probe(acm->dev, ret, "failed to load %s\n", name);

	return 0;
}

static void acm8615_power_off(void *data)
{
	struct acm8615_priv *acm = data;

	regmap_write(acm->regmap, ACM8615_STATE_CTRL, ACM8615_STATE_DIG_OFF);
	fsleep(6000);
}

static const struct regmap_range_cfg acm8615_ranges[] = {
	{
		.range_min	= 0,
		.range_max	= ACM8615_MAX_REG,
		.selector_reg	= ACM8615_PAGE_SEL,
		.selector_mask	= 0xff,
		.window_start	= 0,
		.window_len	= ACM8615_PAGE_LEN,
	},
};

static const struct regmap_config acm8615_regmap = {
	.reg_bits	= 8,
	.val_bits	= 8,
	.max_register	= ACM8615_MAX_REG,
	.ranges		= acm8615_ranges,
	.num_ranges	= ARRAY_SIZE(acm8615_ranges),
	.cache_type	= REGCACHE_NONE,
};

static int acm8615_i2c_probe(struct i2c_client *i2c)
{
	struct device *dev = &i2c->dev;
	struct acm8615_priv *acm;
	unsigned int die_id;
	int ret;

	acm = devm_kzalloc(dev, sizeof(*acm), GFP_KERNEL);
	if (!acm)
		return -ENOMEM;

	acm->dev = dev;
	acm->stream_muted = true;
	acm->switched_on = true;
	i2c_set_clientdata(i2c, acm);

	ret = devm_mutex_init(dev, &acm->lock);
	if (ret)
		return ret;

	acm->regmap = devm_regmap_init_i2c(i2c, &acm8615_regmap);
	if (IS_ERR(acm->regmap))
		return dev_err_probe(dev, PTR_ERR(acm->regmap),
				     "failed to init regmap\n");

	ret = regmap_read(acm->regmap, ACM8615_DIE_ID, &die_id);
	if (ret)
		return dev_err_probe(dev, ret, "not responding\n");
	dev_dbg(dev, "die id 0x%02x\n", die_id);

	ret = acm8615_setup(acm);
	if (ret)
		return ret;

	ret = devm_add_action_or_reset(dev, acm8615_power_off, acm);
	if (ret)
		return ret;

	ret = devm_amp_hp_det_init(dev, &acm->hp, i2c->irq, &acm->lock,
				   acm8615_hp_changed, acm);
	if (ret)
		return ret;

	mutex_lock(&acm->lock);
	ret = acm8615_apply_mute(acm);
	mutex_unlock(&acm->lock);
	if (ret)
		return ret;

	return devm_snd_soc_register_component(dev, &acm8615_component,
					       &acm8615_dai, 1);
}

static const struct i2c_device_id acm8615_i2c_id[] = {
	{ "acm8615" },
	{ }
};
MODULE_DEVICE_TABLE(i2c, acm8615_i2c_id);

static const struct of_device_id acm8615_of_match[] = {
	{ .compatible = "acmesemi,acm8615" },
	{ }
};
MODULE_DEVICE_TABLE(of, acm8615_of_match);

static struct i2c_driver acm8615_i2c_driver = {
	.driver = {
		.name		= "acm8615",
		.of_match_table	= acm8615_of_match,
		.probe_type	= PROBE_PREFER_ASYNCHRONOUS,
	},
	.probe		= acm8615_i2c_probe,
	.id_table	= acm8615_i2c_id,
};
module_i2c_driver(acm8615_i2c_driver);

MODULE_DESCRIPTION("ACME Semiconductor ACM8615 amplifier driver");
MODULE_LICENSE("GPL");
