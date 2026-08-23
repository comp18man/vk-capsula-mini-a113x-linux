// SPDX-License-Identifier: GPL-2.0-only
/*
 * Driver for the NTP8918 Audio Amplifier
 *
 * Copyright (c) 2024, SaluteDevices. All Rights Reserved.
 *
 * Author: Igor Prusov <ivprusov@salutedevices.com>
 */

#include <linux/kernel.h>
#include <linux/reset.h>
#include <linux/gpio/consumer.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/i2c.h>
#include <linux/regmap.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>

#include <sound/initval.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-component.h>
#include <sound/tlv.h>

#include "ntpfw.h"

#define NTP8918_RATES   (SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 | \
			 SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_96000)

#define NTP8918_FORMATS     (SNDRV_PCM_FMTBIT_S16_LE | \
			     SNDRV_PCM_FMTBIT_S20_3LE | \
			     SNDRV_PCM_FMTBIT_S24_LE | \
			     SNDRV_PCM_FMTBIT_S32_LE)

#define NTP8918_INPUT_FMT			0x0
#define  NTP8918_INPUT_FMT_MASTER_MODE		BIT(0)
#define  NTP8918_INPUT_FMT_GSA_MODE		BIT(1)
#define NTP8918_GSA_FMT				0x1
#define  NTP8918_GSA_BS_MASK			GENMASK(3, 2)
#define  NTP8918_GSA_BS(x)			((x) << 2)
#define  NTP8918_GSA_RIGHT_J			BIT(0)
#define  NTP8918_GSA_LSB			BIT(1)
#define NTP8918_MCLK_FREQ_CTRL			0x2
#define  NTP8918_MCLK_FREQ_MCF			GENMASK(1, 0)
#define NTP8918_MASTER_VOL			0x0C
#define NTP8918_CHNL_A_VOL			0x17
#define NTP8918_CHNL_B_VOL			0x18
#define NTP8918_SOFT_MUTE			0x33
#define  NTP8918_SOFT_MUTE_SM1			BIT(0)
#define  NTP8918_SOFT_MUTE_SM2			BIT(1)
#define NTP8918_PWM_SWITCH			0x34
#define NTP8918_PWM_MASK_CTRL0			0x35
#define REG_MAX					0x7e

#define NTP8918_FW_NAME		"eq_8918.bin"
#define NTP8918_FW_MAGIC	0x38393138	/* "8918" */

struct ntp8918_priv {
	struct i2c_client *i2c;
	struct clk *bck;
	struct reset_control *reset;
	unsigned int format;
	unsigned int slots;
	unsigned int slot_width;
	unsigned int mcf;
	unsigned int input_fmt;
	unsigned int gsa_fmt;
	bool configured;
	bool fw_loaded;

	struct snd_soc_component *component;
	struct gpio_desc *hp_det;
	struct delayed_work hp_work;
	bool hp_seen;
	bool hp_inserted;
	bool stream_muted;
	struct mutex mute_lock;
};

#define NTP8918_HP_DEBOUNCE_MS	40
#define NTP8918_HP_POLL_MS	300
#define NTP8918_HP_BACKSTOP_MS	2000

static const DECLARE_TLV_DB_SCALE(ntp8918_master_vol_scale, -12550, 50, 0);

static int ntp8918_put_master_vol(struct snd_kcontrol *kcontrol,
				  struct snd_ctl_elem_value *ucontrol);

/*
 * The volume needs a put of its own: writing the master volume register
 * restarts the part's own on/off sequencer, and that sequence ends by
 * releasing the soft mute. Anything the driver had muted -- a jack in the
 * socket, a stream that has not started -- comes back the moment someone
 * touches the slider, so the mute is written again behind every change.
 */
static const struct snd_kcontrol_new ntp8918_vol_control[] = {
	{
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
		.name = "Playback Volume",
		.access = SNDRV_CTL_ELEM_ACCESS_TLV_READ |
			  SNDRV_CTL_ELEM_ACCESS_READWRITE,
		.tlv.p = ntp8918_master_vol_scale,
		.info = snd_soc_info_volsw,
		.get = snd_soc_get_volsw,
		.put = ntp8918_put_master_vol,
		.private_value = SOC_SINGLE_VALUE(NTP8918_MASTER_VOL, 0,
						  0x04, 0xff, 0, 0),
	},
	SOC_SINGLE("Playback Switch", NTP8918_PWM_MASK_CTRL0, 1, 1, 1),
};

static void ntp8918_reset_gpio(struct ntp8918_priv *ntp8918)
{
	/*
	 * Proper initialization sequence for NTP8918 amplifier requires driving
	 * /RESET signal low during power up for at least 0.1us. The sequence is,
	 * according to NTP8918 datasheet, 6.2 Timing Sequence 1:
	 * Deassert for T2 >= 1ms...
	 */
	reset_control_deassert(ntp8918->reset);
	fsleep(1000);

	/* ...Assert for T3 >= 0.1us... */
	reset_control_assert(ntp8918->reset);
	fsleep(1);

	/* ...Deassert, and wait for T4 >= 0.5ms before sound on sequence. */
	reset_control_deassert(ntp8918->reset);
	fsleep(500);
}

static const struct reg_sequence ntp8918_sound_off[] = {
	{ NTP8918_MASTER_VOL, 0 },
};

static const struct reg_sequence ntp8918_sound_on[] = {
	{ NTP8918_MASTER_VOL, 0b11 },
};

static int ntp8918_load_firmware(struct ntp8918_priv *ntp8918)
{
	int ret;

	ret = ntpfw_load(ntp8918->i2c, NTP8918_FW_NAME, NTP8918_FW_MAGIC);
	if (ret == -ENOENT) {
		dev_warn_once(&ntp8918->i2c->dev, "Could not find firmware %s\n",
			      NTP8918_FW_NAME);
		return 0;
	}

	return ret;
}

static int ntp8918_snd_suspend(struct snd_soc_component *component)
{
	struct ntp8918_priv *ntp8918 = snd_soc_component_get_drvdata(component);

	regcache_cache_only(component->regmap, true);

	regmap_multi_reg_write_bypassed(component->regmap,
					ntp8918_sound_off,
					ARRAY_SIZE(ntp8918_sound_off));

	/*
	 * According to NTP8918 datasheet, 6.2 Timing Sequence 1:
	 * wait after sound off for T6 >= 0.5ms
	 */
	fsleep(500);
	reset_control_assert(ntp8918->reset);

	regcache_mark_dirty(component->regmap);
	clk_disable_unprepare(ntp8918->bck);

	return 0;
}

static int ntp8918_snd_resume(struct snd_soc_component *component)
{
	struct ntp8918_priv *ntp8918 = snd_soc_component_get_drvdata(component);
	int ret;

	ret = clk_prepare_enable(ntp8918->bck);
	if (ret)
		return ret;

	ntp8918_reset_gpio(ntp8918);

	regmap_multi_reg_write_bypassed(component->regmap,
					ntp8918_sound_on,
					ARRAY_SIZE(ntp8918_sound_on));

	ret = ntp8918_load_firmware(ntp8918);
	if (ret) {
		dev_err(&ntp8918->i2c->dev, "Failed to load firmware\n");
		return ret;
	}

	regcache_cache_only(component->regmap, false);
	snd_soc_component_cache_sync(component);

	return 0;
}

/*
 * The master volume register doubles as an on/off command. The datasheet gives
 * values 0x00 to 0x03 as switching the power stage, and the last of them is
 * documented as leaving the soft mute off; anything the slider writes lands
 * above that range, but the part runs the sequence all the same and 0x33, 0x34
 * and 0x35 read back as zero afterwards. regmap never sees that happen, so its
 * cache still holds the muted value and update_bits would conclude there is
 * nothing to send. A plain write always reaches the bus.
 */
static int ntp8918_apply_mute(struct ntp8918_priv *ntp8918)
{
	unsigned int mask = NTP8918_SOFT_MUTE_SM1 | NTP8918_SOFT_MUTE_SM2;
	bool mute = ntp8918->stream_muted || ntp8918->hp_inserted;

	if (!ntp8918->component)
		return 0;

	return snd_soc_component_write(ntp8918->component, NTP8918_SOFT_MUTE,
				       mute ? mask : 0);
}

static int ntp8918_put_master_vol(struct snd_kcontrol *kcontrol,
				  struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component =
		snd_soc_kcontrol_component(kcontrol);
	struct ntp8918_priv *ntp8918 = snd_soc_component_get_drvdata(component);
	unsigned int pwm_mask;
	int ret;

	ret = snd_soc_put_volsw(kcontrol, ucontrol);
	if (ret < 0)
		return ret;

	pwm_mask = snd_soc_component_read(component, NTP8918_PWM_MASK_CTRL0);
	snd_soc_component_write(component, NTP8918_PWM_MASK_CTRL0, pwm_mask);

	mutex_lock(&ntp8918->mute_lock);
	ntp8918_apply_mute(ntp8918);
	mutex_unlock(&ntp8918->mute_lock);

	return ret;
}

/*
 * The pad interrupt cannot be asked for both edges: on this generation the
 * controller has no such mode and set_type answers -EINVAL, which is why the
 * vendor driver watches the falling one alone. So the trigger is turned around
 * after every change to face the transition that can still happen, and a slow
 * poll runs underneath to heal the one that slips through while the trigger is
 * being turned.
 *
 * confirm asks for the same reading twice, which is what the poll uses to sit
 * out the contact settling; the interrupt path has already waited instead.
 */
static void ntp8918_hp_update(struct ntp8918_priv *ntp8918, bool confirm)
{
	bool now = gpiod_get_value_cansleep(ntp8918->hp_det) > 0;

	mutex_lock(&ntp8918->mute_lock);
	if ((!confirm || now == ntp8918->hp_seen) &&
	    now != ntp8918->hp_inserted) {
		ntp8918->hp_inserted = now;
		dev_dbg(&ntp8918->i2c->dev, "headphone jack %s\n",
			now ? "inserted, speaker muted" : "removed");
		ntp8918_apply_mute(ntp8918);
	}
	ntp8918->hp_seen = now;
	mutex_unlock(&ntp8918->mute_lock);

	if (ntp8918->i2c->irq > 0)
		irq_set_irq_type(ntp8918->i2c->irq,
				 now ? IRQ_TYPE_EDGE_RISING :
				       IRQ_TYPE_EDGE_FALLING);
}

static void ntp8918_hp_stop(void *data)
{
	struct ntp8918_priv *ntp8918 = data;

	cancel_delayed_work_sync(&ntp8918->hp_work);
}

static irqreturn_t ntp8918_hp_irq(int irq, void *data)
{
	struct ntp8918_priv *ntp8918 = data;

	msleep(NTP8918_HP_DEBOUNCE_MS);
	ntp8918_hp_update(ntp8918, false);

	return IRQ_HANDLED;
}

static void ntp8918_hp_poll(struct work_struct *work)
{
	struct ntp8918_priv *ntp8918 = container_of(work, struct ntp8918_priv,
						    hp_work.work);
	unsigned int period = ntp8918->i2c->irq > 0 ? NTP8918_HP_BACKSTOP_MS
						    : NTP8918_HP_POLL_MS;

	ntp8918_hp_update(ntp8918, true);
	schedule_delayed_work(&ntp8918->hp_work, msecs_to_jiffies(period));
}

static int ntp8918_probe(struct snd_soc_component *component)
{
	struct ntp8918_priv *ntp8918 = snd_soc_component_get_drvdata(component);
	struct device *dev = component->dev;
	int ret;

	ret = snd_soc_add_component_controls(component, ntp8918_vol_control,
			ARRAY_SIZE(ntp8918_vol_control));
	if (ret)
		return dev_err_probe(dev, ret, "Failed to add controls\n");

	/*
	 * Read once on the way in, so a plug already sitting in the socket at
	 * boot is honoured instead of waiting for someone to pull it out. No
	 * write here: the first unmute applies it anyway, and this runs before
	 * the part has been told anything.
	 */
	mutex_lock(&ntp8918->mute_lock);
	ntp8918->component = component;
	if (ntp8918->hp_det) {
		ntp8918->hp_inserted =
			gpiod_get_value_cansleep(ntp8918->hp_det) > 0;
		ntp8918->hp_seen = ntp8918->hp_inserted;
	}
	mutex_unlock(&ntp8918->mute_lock);

	/*
	 * Upstream writes the firmware here. This board writes it on the
	 * first unmute instead, see ntp8918_digital_mute(). The part is
	 * reachable at probe - the vendor driver initialises it there and
	 * succeeds - but buys two things:
	 * missing or rejected blob cannot fail probe and take the whole
	 * sound card with it, and the blob lands next to the format
	 * registers rather than a second of wall time away from them.
	 */
	return 0;
}

static const struct snd_soc_dapm_widget ntp8918_dapm_widgets[] = {
	SND_SOC_DAPM_DAC("AIFIN", "Playback", SND_SOC_NOPM, 0, 0),

	SND_SOC_DAPM_OUTPUT("OUT1"),
	SND_SOC_DAPM_OUTPUT("OUT2"),
};

static const struct snd_soc_dapm_route ntp8918_dapm_routes[] = {
	{ "OUT1", NULL, "AIFIN" },
	{ "OUT2", NULL, "AIFIN" },
};

static void ntp8918_remove(struct snd_soc_component *component)
{
	struct ntp8918_priv *ntp8918 = snd_soc_component_get_drvdata(component);

	mutex_lock(&ntp8918->mute_lock);
	ntp8918->component = NULL;
	mutex_unlock(&ntp8918->mute_lock);
}

static const struct snd_soc_component_driver soc_component_ntp8918 = {
	.probe = ntp8918_probe,
	.remove = ntp8918_remove,
	.suspend = ntp8918_snd_suspend,
	.resume = ntp8918_snd_resume,
	.dapm_widgets = ntp8918_dapm_widgets,
	.num_dapm_widgets = ARRAY_SIZE(ntp8918_dapm_widgets),
	.dapm_routes = ntp8918_dapm_routes,
	.num_dapm_routes = ARRAY_SIZE(ntp8918_dapm_routes),
};

static int ntp8918_hw_params(struct snd_pcm_substream *substream,
			    struct snd_pcm_hw_params *params,
			    struct snd_soc_dai *dai)
{
	struct snd_soc_component *component = dai->component;
	struct ntp8918_priv *ntp8918 = snd_soc_component_get_drvdata(component);
	unsigned int input_fmt = 0;
	unsigned int gsa_fmt = 0;
	unsigned int mcf;
	int bclk;

	/*
	 * Nothing here goes out over I2C. __soc_pcm_hw_params() runs the
	 * codec DAI before the CPU DAI, so at this point the TDM interface
	 * has not been given a rate yet and the link is still idle. Work
	 * out what the registers should hold and write them at unmute,
	 * once the interface is actually running.
	 *
	 * snd_soc_params_to_bclk() derives the bit clock from the stream
	 * alone -- sample width times channels times rate -- which is only
	 * right when the frame is packed. On a TDM link the interface
	 * clocks whole slots, so 16-bit samples in 32-bit slots produce
	 * twice that. Use the slot geometry when the machine driver has
	 * told us about it; axg-card does, through set_tdm_slot below.
	 */
	if (ntp8918->slots && ntp8918->slot_width)
		bclk = ntp8918->slots * ntp8918->slot_width * params_rate(params);
	else
		bclk = snd_soc_params_to_bclk(params);

	switch (bclk) {
	case 3072000:
	case 2822400:
		mcf = 0;
		break;
	case 6144000:
		mcf = 1;
		break;
	case 2048000:
		mcf = 2;
		break;
	default:
		return -EINVAL;
	}

	switch (ntp8918->format) {
	case SND_SOC_DAIFMT_I2S:
		break;
	case SND_SOC_DAIFMT_RIGHT_J:
		input_fmt |= NTP8918_INPUT_FMT_GSA_MODE;
		gsa_fmt |= NTP8918_GSA_RIGHT_J;
		break;
	case SND_SOC_DAIFMT_LEFT_J:
		input_fmt |= NTP8918_INPUT_FMT_GSA_MODE;
		break;
	}

	if (input_fmt & NTP8918_INPUT_FMT_GSA_MODE) {
		switch (params_width(params)) {
		case 24:
			gsa_fmt |= NTP8918_GSA_BS(0);
			break;
		case 20:
			gsa_fmt |= NTP8918_GSA_BS(1);
			break;
		case 18:
			gsa_fmt |= NTP8918_GSA_BS(2);
			break;
		case 16:
			gsa_fmt |= NTP8918_GSA_BS(3);
			break;
		default:
			return -EINVAL;
		}
	}

	ntp8918->mcf = mcf;
	ntp8918->input_fmt = input_fmt;
	ntp8918->gsa_fmt = gsa_fmt;
	ntp8918->configured = true;

	return 0;
}

/*
 * The register writes hw_params used to do, moved to the point where the
 * part can actually take them. See the comment in ntp8918_digital_mute().
 */
static int ntp8918_apply_config(struct snd_soc_component *component)
{
	struct ntp8918_priv *ntp8918 = snd_soc_component_get_drvdata(component);
	unsigned int gsa_fmt_mask;
	int ret;

	ret = snd_soc_component_update_bits(component, NTP8918_MCLK_FREQ_CTRL,
					    NTP8918_MCLK_FREQ_MCF, ntp8918->mcf);
	if (ret < 0)
		return ret;

	ret = snd_soc_component_update_bits(component, NTP8918_INPUT_FMT,
					    NTP8918_INPUT_FMT_MASTER_MODE |
					    NTP8918_INPUT_FMT_GSA_MODE,
					    ntp8918->input_fmt);
	if (ret < 0)
		return ret;

	if (!(ntp8918->input_fmt & NTP8918_INPUT_FMT_GSA_MODE))
		return 0;

	gsa_fmt_mask = NTP8918_GSA_BS_MASK |
		       NTP8918_GSA_RIGHT_J |
		       NTP8918_GSA_LSB;
	ret = snd_soc_component_update_bits(component, NTP8918_GSA_FMT,
					    gsa_fmt_mask, ntp8918->gsa_fmt);

	return ret < 0 ? ret : 0;
}

static int ntp8918_set_fmt(struct snd_soc_dai *dai, unsigned int fmt)
{
	struct snd_soc_component *component = dai->component;
	struct ntp8918_priv *ntp8918 = snd_soc_component_get_drvdata(component);

	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
	case SND_SOC_DAIFMT_RIGHT_J:
	case SND_SOC_DAIFMT_LEFT_J:
		ntp8918->format = fmt & SND_SOC_DAIFMT_FORMAT_MASK;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int ntp8918_set_tdm_slot(struct snd_soc_dai *dai, unsigned int tx_mask,
				unsigned int rx_mask, int slots, int slot_width)
{
	struct ntp8918_priv *ntp8918 = snd_soc_component_get_drvdata(dai->component);

	ntp8918->slots = slots;
	ntp8918->slot_width = slot_width;

	return 0;
}

static int ntp8918_digital_mute(struct snd_soc_dai *dai, int mute, int stream)
{
	struct snd_soc_component *component = dai->component;
	struct ntp8918_priv *ntp8918 = snd_soc_component_get_drvdata(component);
	int ret;

	/*
	 * This is where every write to the part happens.
	 *
	 * __soc_pcm_prepare() runs snd_soc_dapm_stream_event() and only then
	 * digital_mute(0), so by this point DAPM has taken the bias to
	 * PREPARE and the TDM formatter has started sclk and lrclk at the
	 * rate the CPU DAI worked out in hw_params. Everything the
	 * amplifier is about to be told is therefore true of the link that
	 * is running, which is not the case any earlier.
	 *
	 * It is also the last callback that may sleep, which
	 * request_firmware() needs -- trigger runs atomically.
	 *
	 * Order: the vendor blob first, since it rewrites most of the map,
	 * then the format and clock-ratio registers on top of it, then
	 * unmute.
	 */
	if (!mute) {
		if (!ntp8918->fw_loaded) {
			ret = ntp8918_load_firmware(ntp8918);
			if (ret) {
				dev_warn(component->dev,
					 "firmware not applied (%d); running on defaults\n",
					 ret);
			} else {
				ntp8918->fw_loaded = true;

				/*
				 * The blob writes the register map wholesale,
				 * master volume included, so anything the
				 * mixer had already been told is now gone
				 * from the part while regmap still believes
				 * it is there. Put the cache back on top.
				 *
				 * This is what makes a volume restored at
				 * boot -- by alsactl, before any stream
				 * exists -- survive the first playback
				 * instead of being silently reset to the
				 * vendor's 0 dB.
				 */
				regcache_mark_dirty(component->regmap);
				ret = regcache_sync(component->regmap);
				if (ret < 0)
					return ret;
			}
		}

		if (ntp8918->configured) {
			ret = ntp8918_apply_config(component);
			if (ret)
				return ret;
		}
	}

	mutex_lock(&ntp8918->mute_lock);
	ntp8918->stream_muted = mute;
	ret = ntp8918_apply_mute(ntp8918);
	mutex_unlock(&ntp8918->mute_lock);

	return ret < 0 ? ret : 0;
}

static const struct snd_soc_dai_ops ntp8918_dai_ops = {
	.hw_params = ntp8918_hw_params,
	.set_fmt = ntp8918_set_fmt,
	.set_tdm_slot = ntp8918_set_tdm_slot,
	.mute_stream = ntp8918_digital_mute,
};

static struct snd_soc_dai_driver ntp8918_dai = {
	.name = "ntp8918-amplifier",
	.playback = {
		.stream_name = "Playback",
		.channels_min = 1,
		.channels_max = 2,
		.rates = NTP8918_RATES,
		.formats = NTP8918_FORMATS,
	},
	.ops = &ntp8918_dai_ops,
};

static const struct regmap_config ntp8918_regmap = {
	.reg_bits = 8,
	.val_bits = 8,
	.max_register = REG_MAX,
	.cache_type = REGCACHE_MAPLE,
};

static int ntp8918_i2c_probe(struct i2c_client *i2c)
{
	struct ntp8918_priv *ntp8918;
	int ret;
	struct regmap *regmap;

	ntp8918 = devm_kzalloc(&i2c->dev, sizeof(*ntp8918), GFP_KERNEL);
	if (!ntp8918)
		return -ENOMEM;

	ntp8918->i2c = i2c;

	ret = devm_mutex_init(&i2c->dev, &ntp8918->mute_lock);
	if (ret)
		return ret;

	ntp8918->hp_det = devm_gpiod_get_optional(&i2c->dev, "hp-det", GPIOD_IN);
	if (IS_ERR(ntp8918->hp_det))
		return dev_err_probe(&i2c->dev, PTR_ERR(ntp8918->hp_det),
				     "Failed to get jack detect gpio\n");

	if (ntp8918->hp_det) {
		ntp8918->hp_inserted =
			gpiod_get_value_cansleep(ntp8918->hp_det) > 0;
		ntp8918->hp_seen = ntp8918->hp_inserted;

		INIT_DELAYED_WORK(&ntp8918->hp_work, ntp8918_hp_poll);
		ret = devm_add_action_or_reset(&i2c->dev, ntp8918_hp_stop,
					       ntp8918);
		if (ret)
			return ret;

		if (i2c->irq > 0) {
			ret = devm_request_threaded_irq(&i2c->dev, i2c->irq,
					NULL, ntp8918_hp_irq, IRQF_ONESHOT,
					"ntp8918-hp-det", ntp8918);
			if (ret)
				return dev_err_probe(&i2c->dev, ret,
						"Failed to claim jack detect\n");

			/*
			 * The tree can only name the edge for an empty socket,
			 * so aim it at whichever transition is still ahead.
			 */
			irq_set_irq_type(i2c->irq, ntp8918->hp_inserted ?
					 IRQ_TYPE_EDGE_RISING :
					 IRQ_TYPE_EDGE_FALLING);
		} else {
			dev_info(&i2c->dev,
				 "no jack interrupt, polling the line\n");
		}

		schedule_delayed_work(&ntp8918->hp_work,
				      msecs_to_jiffies(NTP8918_HP_POLL_MS));
	}

	ntp8918->reset = devm_reset_control_get_shared(&i2c->dev, NULL);
	if (IS_ERR(ntp8918->reset))
		return dev_err_probe(&i2c->dev, PTR_ERR(ntp8918->reset), "Failed to get reset\n");

	dev_set_drvdata(&i2c->dev, ntp8918);

	ntp8918_reset_gpio(ntp8918);

	regmap = devm_regmap_init_i2c(i2c, &ntp8918_regmap);
	if (IS_ERR(regmap))
		return dev_err_probe(&i2c->dev, PTR_ERR(regmap),
				     "Failed to allocate regmap\n");

	/*
	 * Get the bit clock before registering the component, not after.
	 *
	 * Registering can complete the sound card, and when it does the
	 * component probe -- which is where the firmware is written over
	 * I2C -- runs synchronously from inside this call. Do it the other
	 * way round and ntp8918->bck is still NULL at that point, so the
	 * clk_set_rate() there is a silent no-op (the clk API accepts NULL),
	 * the part never sees a bit clock, and every write is NAKed.
	 */
	ntp8918->bck = devm_clk_get_enabled(&i2c->dev, "bck");
	if (IS_ERR(ntp8918->bck))
		return dev_err_probe(&i2c->dev, PTR_ERR(ntp8918->bck), "failed to get bck clock\n");

	ret = devm_snd_soc_register_component(&i2c->dev, &soc_component_ntp8918,
					      &ntp8918_dai, 1);
	if (ret)
		return dev_err_probe(&i2c->dev, ret,
				     "Failed to register component\n");

	return 0;
}

static const struct i2c_device_id ntp8918_i2c_id[] = {
	{ "ntp8918" },
	{}
};
MODULE_DEVICE_TABLE(i2c, ntp8918_i2c_id);

static const struct of_device_id ntp8918_of_match[] = {
	{.compatible = "neofidelity,ntp8918"},
	{}
};
MODULE_DEVICE_TABLE(of, ntp8918_of_match);

static struct i2c_driver ntp8918_i2c_driver = {
	.probe = ntp8918_i2c_probe,
	.id_table = ntp8918_i2c_id,
	.driver = {
		.name = "ntp8918",
		.of_match_table = ntp8918_of_match,
	},
};
module_i2c_driver(ntp8918_i2c_driver);

MODULE_AUTHOR("Igor Prusov <ivprusov@salutedevices.com>");
MODULE_DESCRIPTION("NTP8918 Audio Amplifier Driver");
MODULE_LICENSE("GPL");
