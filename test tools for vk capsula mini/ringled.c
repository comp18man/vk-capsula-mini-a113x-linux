// SPDX-License-Identifier: GPL-2.0
/*
 * ringled - top LED ring, VK Mini
 *
 */

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#define MAX_PIXELS	256
#define BYTES_PER_PIXEL	12		/* 3 colours x 8 bits x 4 SPI bits */
#define MAX_RESET	1024
#define MAX_FRAME	4096		/* spidev's own bufsiz default */

#define DEF_DEVICE	"/dev/spidev0.0"
#define DEF_SPEED	3340000
#define DEF_COUNT	12
#define DEF_BRIGHT	50		/* stock default */
/* Bytes reversed together before sending */
#define SWAP		8

/*
 * 128 zero bytes are 306 us at 3.34 MHz. The original WS2812B asks for
 * 50 us, the V5 revision and several clones want 280
 */
#define DEF_RESET	128

/*
 * Animation frame rate
 */
#define DEF_FPS		30

/* Breathing depth, per cent of --brightness. */
#define DEF_BREATH_MIN	5
#define DEF_BREATH_MAX	100

/*
 * Fire2012, Mark Kriegsman's, as shipped in FastLED's examples. 
 */
#define DEF_COOLING	55
#define DEF_SPARKING	120
#define DEF_SPARK_ZONE	3

struct rgb {
	uint8_t r, g, b;
};

static const char *opt_device = DEF_DEVICE;
static unsigned opt_speed = DEF_SPEED;
static unsigned opt_count = DEF_COUNT;
static unsigned opt_bright = DEF_BRIGHT;
static unsigned opt_reset = DEF_RESET;
static int opt_rebind;
static int opt_pio;
static int opt_dump;
static unsigned opt_breath;		/* breath period in ms, 0 = off */
static unsigned opt_breath_min = DEF_BREATH_MIN;
static unsigned opt_breath_max = DEF_BREATH_MAX;
static unsigned opt_duration;		/* ms, mandatory for animations */
static unsigned opt_fps = DEF_FPS;
static unsigned opt_cooling = DEF_COOLING;
static unsigned opt_sparking = DEF_SPARKING;
static unsigned opt_spark_zone = DEF_SPARK_ZONE;

/* Per-frame brightness, per cent, what breathing moves. */
static unsigned anim_scale = 100;

static volatile sig_atomic_t interrupted;

static void on_signal(int sig)
{
	(void)sig;
	interrupted = 1;
}

static void usage(void)
{
	fputs(
"usage: ringled [options] COMMAND [args]\n"
"\n"
"  off                     all pixels dark\n"
"  all RRGGBB              every pixel the same colour\n"
"  set IDX RRGGBB ...      only the pixels named, the rest dark\n"
"  raw RRGGBB ...          colours in chain order, missing ones dark\n"
"  rainbow [MS]            hues spread around the ring; with MS they\n"
"                          rotate, one full turn every MS\n"
"  fire                    Fire2012 flame simulation; tune it with\n"
"                          --cooling, --sparking and --spark-zone\n"
"\n"
"  Anything that animates needs --duration: every run ends on its own\n"
"  and leaves the ring dark, so this can be fired at an event from the\n"
"  background and forgotten.\n"
"  test                    one white pixel walks the ring once, a second\n"
"                          per step, naming it, then all dark\n"
"\n"
"  -b, --brightness N      scale every channel by N per cent (default 50)\n"
"      --breathe MS        fade in and out, one breath every MS; works\n"
"                          with any command, not just the animations\n"
"      --breathe-range A:B per cent of --brightness at the bottom and\n"
"                          top of a breath (default 5:100)\n"
"      --duration MS       how long to run, then go dark; required by\n"
"                          every animation\n"
"      --fps N             animation frames per second (default 30);\n"
"                          for fire this is the speed of the flame\n"
"      --cooling N         fire: heat lost per frame (default 55).\n"
"                          Lower is taller flames, higher is shorter\n"
"      --sparking N        fire: chance out of 255 of an ember each\n"
"                          frame (default 120). Lower is sparser and\n"
"                          more flickery, higher roars\n"
"      --spark-zone N      fire: pixels at the base that can catch a\n"
"                          spark (default 3)\n"
"  -d, --device PATH       spidev node (default " DEF_DEVICE ")\n"
"  -n, --count N           pixels in the chain (default 12)\n"
"  -s, --speed HZ          SPI clock (default 3340000, matches the encoding)\n"
"  -r, --reset N           zero bytes of latch sent either side of the\n"
"                          frame (default 128, about 300 us each)\n"
"      --rebind            rebind the SPI driver before every frame; only\n"
"                          for kernels without the spi-meson-spicc ENH_CTL0\n"
"                          fix, where every frame but the first after a\n"
"                          bind goes out at the wrong rate\n"
"      --pio               send at 8 bits a word, which is PIO and breaks\n"
"                          the frame every 16 bytes; for comparison only\n"
"      --dump              write the encoded frame to stdout instead of\n"
"                          the device, for checking the encoder\n"
"\n"
"The default is the stock firmware's: its device tree gives all 36 channels\n"
"linux,default-brightness-level = 0x7f, which is half. Nothing caps this --\n"
"neither tree sets a current limit on the ring - but note that twelve\n"
"packages at full white draw around 700 mA, so 100 is worth arriving at\n"
"deliberately rather than by accident.\n",
	      stderr);
}

static int die(const char *what)
{
	fprintf(stderr, "ringled: %s: %s\n", what, strerror(errno));
	return 1;
}

static int fail(const char *msg)
{
	fprintf(stderr, "ringled: %s\n", msg);
	return 1;
}

static int badarg(const char *fmt, const char *arg)
{
	fprintf(stderr, "ringled: ");
	fprintf(stderr, fmt, arg);
	fputc('\n', stderr);
	return 1;
}

static int hexval(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return -1;
}

static int parse_colour(const char *s, struct rgb *out)
{
	int v[6];
	int i;

	if (strlen(s) != 6)
		return -1;

	for (i = 0; i < 6; i++) {
		v[i] = hexval(s[i]);
		if (v[i] < 0)
			return -1;
	}

	out->r = (uint8_t)(v[0] << 4 | v[1]);
	out->g = (uint8_t)(v[2] << 4 | v[3]);
	out->b = (uint8_t)(v[4] << 4 | v[5]);
	return 0;
}

static int parse_uint(const char *s, unsigned *out)
{
	char *end;
	unsigned long v;

	errno = 0;
	v = strtoul(s, &end, 10);
	if (errno || end == s || *end || v > UINT32_MAX)
		return -1;

	*out = (unsigned)v;
	return 0;
}

/*
 * Two data bits -> one SPI byte.
 *
 * These are the vendor driver's own values, read out of its .text: the
 * stock ws2812b driver indexes a four byte table at 0x96a59c8 with two
 * bits at a time, and that table is 88 8c c8 cc.
 *
 * The interesting entry is the one: 1100, not the 1110 the datasheet
 * arithmetic suggests. At 3.34 MHz that is 598 ns high and 598 ns low
 * against the 800 +-150 the WS2812B datasheet asks for, so whatever is
 * fitted here decides on a threshold nearer 500 ns than 800.
 *
 * Sending 1110 instead does not merely stretch the pulse, it starves the
 * gap: consecutive ones leave only 299 ns low between them, too short to
 * read as a bit boundary. A frame of mostly ones then arrives as a
 * near-continuous high with nothing to decode.
 */
static const uint8_t sym[4] = { 0x88, 0x8c, 0xc8, 0xcc };

static uint8_t *enc_byte(uint8_t *dst, uint8_t v)
{
	*dst++ = sym[(v >> 6) & 3];
	*dst++ = sym[(v >> 4) & 3];
	*dst++ = sym[(v >> 2) & 3];
	*dst++ = sym[v & 3];
	return dst;
}

static uint8_t scale(uint8_t v)
{
	return (uint8_t)((unsigned)v * opt_bright * anim_scale / 10000);
}

/* The part takes green first, then red, then blue. */
static uint8_t *enc_pixel(uint8_t *dst, const struct rgb *c)
{
	dst = enc_byte(dst, scale(c->g));
	dst = enc_byte(dst, scale(c->r));
	dst = enc_byte(dst, scale(c->b));
	return dst;
}

/*
 * For kernels without "spi-meson-spicc fix: restore ENH_CTL0 after the
 * controller is re-enabled". There the controller loses its clock divider
 * when unprepare_transfer() disables it after every message, the clk core
 * keeps the rate cached and never writes it again, and only the first frame
 * after a bind goes out at the right rate. Needs root.
 */
static int spicc_rebind(void)
{
	static const char dir[] = "/sys/bus/platform/drivers/meson-spicc";
	char name[64] = "", path[128];
	struct dirent *e;
	int i, fd;
	DIR *d;

	d = opendir(dir);
	if (!d)
		return die(dir);

	while ((e = readdir(d))) {
		size_t n = strlen(e->d_name);

		if (n > 4 && n < sizeof(name) &&
		    !strcmp(e->d_name + n - 4, ".spi")) {
			memcpy(name, e->d_name, n + 1);
			break;
		}
	}
	closedir(d);

	if (!name[0])
		return fail("no meson-spicc device bound to rebind");

	for (i = 0; i < 2; i++) {
		snprintf(path, sizeof(path), "%s/%s", dir,
			 i ? "bind" : "unbind");
		fd = open(path, O_WRONLY);
		if (fd < 0)
			return die(path);
		if (write(fd, name, strlen(name)) < 0) {
			close(fd);
			return die(path);
		}
		close(fd);
	}

	/* devtmpfs recreates the node; it is there in a millisecond or two. */
	for (i = 0; i < 2000; i++) {
		if (access(opt_device, W_OK) == 0)
			return 0;
		usleep(1000);
	}

	return fail("spidev node did not come back after rebind");
}

static int send_frame(const struct rgb *pix)
{
	static uint8_t tx[MAX_FRAME], rx[MAX_FRAME];
	struct spi_ioc_transfer xf;
	size_t len, off;
	unsigned i;
	int fd, ret;

	if (opt_count * BYTES_PER_PIXEL + 2 * opt_reset + 16 > MAX_FRAME)
		return fail("frame too long; lower --count or --reset");

	/*
	 * Latch, data, latch. The leading run is the one that matters: it
	 * guarantees the chain is ready to start a frame no matter what
	 * the pin was doing beforehand. The trailing run makes the part
	 * show what it was just given without waiting for the next
	 * command.
	 */
	len = 0;
	for (i = 0; i < opt_reset; i++)
		tx[len++] = 0x00;
	for (i = 0; i < opt_count; i++)
		len = (size_t)(enc_pixel(tx + len, &pix[i]) - tx);
	for (i = 0; i < opt_reset; i++)
		tx[len++] = 0x00;

	if (!opt_pio) {
		/*
		 * DMA wants whole 64 bit words, and
		 * meson_spicc_transfer_one() rejects anything under 16
		 * bytes because a burst length of one cannot trigger a
		 * DMA burst. Pad with zeros - the line is meant to rest
		 * low after a frame, so the padding is part of the latch
		 * rather than something to work around.
		 */
		while (len % 8 || len < 16)
			tx[len++] = 0x00;

		for (off = 0; SWAP > 1 && off < len; off += SWAP) {
			size_t a = 0, b = SWAP - 1;

			while (a < b) {
				uint8_t t = tx[off + a];

				tx[off + a] = tx[off + b];
				tx[off + b] = t;
				a++;
				b--;
			}
		}
	}

	/* For checking the encoder without hardware: ringled --dump ... | xxd */
	if (opt_dump)
		return fwrite(tx, 1, len, stdout) == len ? 0 : fail("short write");

	if (opt_rebind && spicc_rebind())
		return 1;

	fd = open(opt_device, O_RDWR);
	if (fd < 0)
		return die(opt_device);

	memset(&xf, 0, sizeof(xf));
	xf.tx_buf = (unsigned long)tx;
	xf.rx_buf = (unsigned long)rx;	/* the DMA path maps both ways */
	xf.len = (uint32_t)len;
	xf.speed_hz = (uint32_t)opt_speed;
	xf.bits_per_word = opt_pio ? 8 : 64;

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &xf);
	if (ret < 1) {
		die("SPI_IOC_MESSAGE");
		close(fd);
		return 1;
	}

	close(fd);
	return 0;
}

static void clear(struct rgb *pix)
{
	memset(pix, 0, sizeof(*pix) * opt_count);
}


static void rainbow(struct rgb *pix, unsigned offset)
{
	unsigned i;

	for (i = 0; i < opt_count; i++) {
		unsigned pos = (i * 1536 / opt_count + offset) % 1536;
		unsigned phase = pos / 256;
		uint8_t up = (uint8_t)(pos % 256);
		uint8_t down = (uint8_t)(255 - up);

		switch (phase) {
		case 0: pix[i] = (struct rgb){ 255, up, 0 }; break;
		case 1: pix[i] = (struct rgb){ down, 255, 0 }; break;
		case 2: pix[i] = (struct rgb){ 0, 255, up }; break;
		case 3: pix[i] = (struct rgb){ 0, down, 255 }; break;
		case 4: pix[i] = (struct rgb){ up, 0, 255 }; break;
		default: pix[i] = (struct rgb){ 255, 0, down }; break;
		}
	}
}

static unsigned long long now_ms(void)
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (unsigned long long)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

static void nap_ms(unsigned ms)
{
	struct timespec ts = { ms / 1000, (long)(ms % 1000) * 1000000 };

	nanosleep(&ts, NULL);
}


static unsigned breath_scale(unsigned long long t)
{
	double x = (double)(t % opt_breath) / opt_breath;
	double tri = x < 0.5 ? x * 2 : (1.0 - x) * 2;
	double s = tri * tri * (3.0 - 2.0 * tri);

	return opt_breath_min +
	       (unsigned)((opt_breath_max - opt_breath_min) * s + 0.5);
}

/*
 * HeatColor from FastLED
 */
static struct rgb heat_colour(uint8_t t)
{
	unsigned t192 = (((unsigned)t * 191) >> 8) + (t ? 1 : 0);
	uint8_t ramp = (uint8_t)((t192 & 0x3f) << 2);

	if (t192 & 0x80)
		return (struct rgb){ 255, 255, ramp };
	if (t192 & 0x40)
		return (struct rgb){ 255, ramp, 0 };
	return (struct rgb){ ramp, 0, 0 };
}

/*
 * Fire2012. Cool every cell, drift the heat one pixel along and blur it,
 * throw an ember in at the base, then colour by temperature.
 */
static void fire_half(uint8_t *heat, unsigned n)
{
	unsigned i, k, zone, y, v, c;

	if (!n)
		return;

	for (i = 0; i < n; i++) {
		c = (unsigned)rand() % ((opt_cooling * 10) / n + 2);
		heat[i] = heat[i] > c ? (uint8_t)(heat[i] - c) : 0;
	}

	for (k = n - 1; k >= 2 && k < n; k--)
		heat[k] = (uint8_t)((heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3);

	if ((unsigned)(rand() & 0xff) < opt_sparking) {
		zone = n < opt_spark_zone ? n : opt_spark_zone;
		y = (unsigned)rand() % zone;
		v = heat[y] + 160 + (unsigned)rand() % 96;
		heat[y] = (uint8_t)(v > 255 ? 255 : v);
	}
}

static void fire_step(uint8_t *heat, struct rgb *pix)
{
	unsigned n = opt_count, up = (n + 1) / 2, down = n - up, i;

	fire_half(heat, up);
	fire_half(heat + up, down);

	for (i = 0; i < up; i++)
		pix[i] = heat_colour(heat[i]);
	for (i = 0; i < down; i++)
		pix[n - 1 - i] = heat_colour(heat[up + i]);
}

enum anim { ANIM_HOLD, ANIM_SPIN, ANIM_FIRE };


static int animate(struct rgb *pix, enum anim mode, unsigned spin_ms)
{
	static uint8_t heat[MAX_PIXELS];
	struct rgb base[MAX_PIXELS];
	unsigned long long t0 = now_ms(), t;

	memcpy(base, pix, sizeof(*pix) * opt_count);
	srand((unsigned)(now_ms() ^ (unsigned long long)getpid()));

	while (!interrupted) {
		t = now_ms() - t0;
		if (t >= opt_duration)
			break;

		if (opt_breath)
			anim_scale = breath_scale(t);

		switch (mode) {
		case ANIM_SPIN:
			rainbow(pix, (unsigned)(t % spin_ms) * 1536 / spin_ms);
			break;
		case ANIM_FIRE:
			fire_step(heat, pix);
			break;
		default:
			memcpy(pix, base, sizeof(*pix) * opt_count);
			break;
		}

		if (send_frame(pix))
			return 1;

		nap_ms(1000 / opt_fps);
	}

	clear(pix);
	if (send_frame(pix))
		return 1;

	return interrupted ? 130 : 0;
}


static int walk(struct rgb *pix)
{
	unsigned i;

	for (i = 0; i < opt_count && !interrupted; i++) {
		printf("pixel %u\n", i);
		fflush(stdout);

		clear(pix);
		pix[i] = (struct rgb){ 255, 255, 255 };
		if (send_frame(pix))
			return 1;

		sleep(1);
	}

	clear(pix);
	if (send_frame(pix))
		return 1;

	return interrupted ? 130 : 0;
}

int main(int argc, char **argv)
{
	static struct rgb pix[MAX_PIXELS];
	struct sigaction sa;
	const char *cmd;
	unsigned i;
	int a = 1;

	while (a < argc && argv[a][0] == '-' && argv[a][1]) {
		const char *o = argv[a];
		const char *val = (a + 1 < argc) ? argv[a + 1] : NULL;

		if (!strcmp(o, "-h") || !strcmp(o, "--help")) {
			usage();
			return 0;
		} else if (!strcmp(o, "--pio")) {
			opt_pio = 1;
			a++;
			continue;
		} else if (!strcmp(o, "--dump")) {
			opt_dump = 1;
			a++;
			continue;
		} else if (!strcmp(o, "--rebind")) {
			opt_rebind = 1;
			a++;
			continue;
		} else if (!strcmp(o, "--no-rebind")) {
			opt_rebind = 0;
			a++;
			continue;
		}

		if (!val)
			return badarg("%s needs a value", o);

		if (!strcmp(o, "-d") || !strcmp(o, "--device"))
			opt_device = val;
		else if (!strcmp(o, "--breathe")) {
			if (parse_uint(val, &opt_breath) || !opt_breath)
				return badarg("bad breath period '%s'", val);
		} else if (!strcmp(o, "--duration")) {
			if (parse_uint(val, &opt_duration) || !opt_duration)
				return badarg("bad duration '%s'", val);
		} else if (!strcmp(o, "--fps")) {
			if (parse_uint(val, &opt_fps) || !opt_fps || opt_fps > 120)
				return badarg("fps wants 1 to 120, got '%s'", val);
		} else if (!strcmp(o, "--cooling")) {
			if (parse_uint(val, &opt_cooling) || opt_cooling > 255)
				return badarg("cooling wants 0 to 255, got '%s'", val);
		} else if (!strcmp(o, "--sparking")) {
			if (parse_uint(val, &opt_sparking) || opt_sparking > 255)
				return badarg("sparking wants 0 to 255, got '%s'", val);
		} else if (!strcmp(o, "--spark-zone")) {
			if (parse_uint(val, &opt_spark_zone) || !opt_spark_zone)
				return badarg("spark zone wants at least 1, got '%s'", val);
		} else if (!strcmp(o, "--breathe-range")) {
			char *sep = strchr(val, ':');
			unsigned lo, hi;

			if (!sep || parse_uint(sep + 1, &hi))
				return badarg("range wants A:B, got '%s'", val);
			*sep = '\0';
			if (parse_uint(val, &lo) || lo > 100 || hi > 100 || lo >= hi)
				return badarg("range wants 0 <= A < B <= 100, got '%s'", val);
			opt_breath_min = lo;
			opt_breath_max = hi;
		}
		else if (!strcmp(o, "-s") || !strcmp(o, "--speed")) {
			if (parse_uint(val, &opt_speed))
				return badarg("bad speed '%s'", val);
		} else if (!strcmp(o, "-b") || !strcmp(o, "--brightness")) {
			if (parse_uint(val, &opt_bright) || opt_bright > 100)
				return badarg("brightness must be 0..100, got '%s'", val);
		} else if (!strcmp(o, "-n") || !strcmp(o, "--count")) {
			if (parse_uint(val, &opt_count) ||
			    opt_count == 0 || opt_count > MAX_PIXELS)
				return badarg("count must be 1..256, got '%s'", val);
		} else if (!strcmp(o, "-r") || !strcmp(o, "--reset")) {
			if (parse_uint(val, &opt_reset) || opt_reset > MAX_RESET)
				return badarg("reset must be 0..1024, got '%s'", val);
		} else {
			return badarg("unknown option '%s'", o);
		}

		a += 2;
	}

	if (a >= argc) {
		usage();
		return 1;
	}

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = on_signal;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	cmd = argv[a++];
	clear(pix);

	if (!strcmp(cmd, "off")) {
		if (a != argc)
			return badarg("%s takes no arguments", cmd);
	} else if (!strcmp(cmd, "all")) {
		struct rgb c;

		if (argc - a != 1)
			return badarg("%s takes one colour", cmd);
		if (parse_colour(argv[a], &c))
			return badarg("bad colour '%s', want six hex digits", argv[a]);
		for (i = 0; i < opt_count; i++)
			pix[i] = c;
	} else if (!strcmp(cmd, "set")) {
		if (a == argc || (argc - a) % 2)
			return badarg("%s takes INDEX RRGGBB pairs", cmd);
		while (a < argc) {
			unsigned idx;

			if (parse_uint(argv[a], &idx) || idx >= opt_count)
				return badarg("index outside the chain: '%s'", argv[a]);
			if (parse_colour(argv[a + 1], &pix[idx]))
				return badarg("bad colour '%s', want six hex digits",
					      argv[a + 1]);
			a += 2;
		}
	} else if (!strcmp(cmd, "raw")) {
		if (a == argc)
			return badarg("%s takes at least one colour", cmd);
		if ((unsigned)(argc - a) > opt_count)
			return fail("more colours than pixels");
		for (i = 0; a < argc; a++, i++)
			if (parse_colour(argv[a], &pix[i]))
				return badarg("bad colour '%s', want six hex digits",
					      argv[a]);
	} else if (!strcmp(cmd, "rainbow")) {
		unsigned ms = 0;

		if (argc - a > 1)
			return badarg("%s takes at most one argument", cmd);
		if (a < argc && (parse_uint(argv[a], &ms) || !ms))
			return badarg("bad rotation period '%s'", argv[a]);

		rainbow(pix, 0);
		if (ms) {
			if (!opt_duration)
				return fail("rotating needs --duration MS");
			return animate(pix, ANIM_SPIN, ms);
		}
	} else if (!strcmp(cmd, "fire")) {
		if (a != argc)
			return badarg("%s takes no arguments", cmd);
		if (!opt_duration)
			return fail("fire needs --duration MS");
		return animate(pix, ANIM_FIRE, 0);
	} else if (!strcmp(cmd, "test")) {
		if (a != argc)
			return badarg("%s takes no arguments", cmd);

		return walk(pix);
	} else {
		return badarg("unknown command '%s'", cmd);
	}

	if (opt_breath && !opt_duration)
		return fail("--breathe needs --duration MS");
	if (opt_breath || opt_duration)
		return animate(pix, ANIM_HOLD, 0);

	return send_frame(pix);
}
