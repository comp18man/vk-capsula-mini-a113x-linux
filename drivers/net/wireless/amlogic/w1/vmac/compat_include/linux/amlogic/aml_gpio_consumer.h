/* Stub for the Amlogic BSP GPIO header, which mainline does not ship.
 * The driver is built with USE_GPIO_IRQ=0 (in-band SDIO interrupt instead),
 * so these values only need to exist, not to be meaningful.
 */
#ifndef _AML_GPIO_CONSUMER_STUB_H_
#define _AML_GPIO_CONSUMER_STUB_H_

#define GPIOX_6     234
#define GPIOX_7     235
#define GPIOX_10    238
#define GPIOX_20    248

#define FILTER_NUM0 0
#define FILTER_NUM4 4

#define GPIO_IRQ_HIGH    0
#define GPIO_IRQ_LOW     1
#define GPIO_IRQ_RISING  2
#define GPIO_IRQ_FALLING 3

#define AML_GPIO_IRQ(irq, filter, type) (0)

#endif
