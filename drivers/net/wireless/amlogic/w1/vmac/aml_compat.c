/* Stubs for Amlogic BSP hooks that mainline does not provide.
 *
 * On a vendor kernel these live in the platform code. Here the WiFi rail and
 * reset line are driven by the mmc-pwrseq-simple node in the device tree
 * before the SDIO card is probed, so the driver's own power hook has nothing
 * left to do.
 */
#include <linux/module.h>

void set_usb_wifi_power(int is_on)
{
    (void)is_on;
}
