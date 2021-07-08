#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>

#define irq_to_gpio(irq) ((irq) - gpio_to_irq(0))
#define POWER_FAULT 6
#define RX_LOS      9
#define POE_THERMAL 10
#define WATCHDOG    12
#define PHY         13

typedef struct mpp_irq_s {
    unsigned int yourGPIO;  /* MPP[xx] of AC3 BXH */
    unsigned int irqNumber; /* interrupt number of Linux */
    unsigned int counter;   /* occurrence counter */
} mpp_irq_t;

static mpp_irq_t isr[] = {  {POWER_FAULT, 0, 0},
                            {RX_LOS,      0, 0},
                            {POE_THERMAL, 0, 0},
                            {WATCHDOG,    0, 0},
                            {PHY,         0, 0} };

#define num_of_isr (sizeof(isr)/sizeof(mpp_irq_t))

static irqreturn_t irqHandler(unsigned int irq, void *dev_id)
{
    int i;

    for(i = 0; i < num_of_isr; i++) {
        if (isr[i].irqNumber == irq) {
            printk(KERN_INFO "Interrupt %d from GPIO %d (%d times)\n",
                   irq, irq_to_gpio(irq), ++isr[i].counter);
            return IRQ_HANDLED;
        }
    }

    printk(KERN_WARNING "Unknown Interrupt %d\n", irq);
    return IRQ_NONE;
}

static int __init isr_gpio_init(void)
{
    int i, result = 0;

    printk(KERN_INFO "WNC ISR !\n");

    for(i = 0; i < num_of_isr; i++) {

        if((result = gpio_request(isr[i].yourGPIO, "WNC ISR")))
        {
            printk(KERN_ERR "WNC ISR yourGPIO %d cannot be requested, result=%d\n",
                   isr[i].yourGPIO, result);
            return result;
        }

        if((result = gpio_direction_input(isr[i].yourGPIO)))
        {
            printk(KERN_ERR "WNC ISR yourGPIO %d set input failed, result=%d\n",
                   isr[i].yourGPIO, result);
            return result;
        }

        if((result = gpio_export(isr[i].yourGPIO, true)))
        {
            printk(KERN_ERR "WNC ISR yourGPIO %d export failed, result=%d\n",
                   isr[i].yourGPIO, result);
            return result;
        }

        isr[i].irqNumber = gpio_to_irq(isr[i].yourGPIO);    // map your GPIO to an IRQ
        result = request_irq(isr[i].irqNumber,              // requested interrupt
                             (irq_handler_t) irqHandler,    // pointer to handler function
                             IRQF_TRIGGER_FALLING,              // interrupt mode flag
                             "irqHandler",                  // used in /proc/interrupts
                             NULL);                         // the *dev_id shared interrupt lines, NULL is okay

        if(result)
        {
            printk(KERN_ERR "WNC ISR yourGPIO %d cannot request an interrupt, result=%d\n",
                   isr[i].yourGPIO, result);
        }
        else
        {
            printk(KERN_INFO "WNC ISR yourGPIO %d, irqNumber %d\n", isr[i].yourGPIO, isr[i].irqNumber);
        }
    } /* end of for loop */

    return result;
}

static void __exit isr_gpio_exit(void)
{
    int i;

    for(i = 0; i < num_of_isr; i++) {
        free_irq(isr[i].irqNumber, NULL);
        gpio_free(isr[i].yourGPIO);

        printk(KERN_INFO "WNC ISR free yourGPIO %d, irqNumber %d\n", isr[i].yourGPIO, isr[i].irqNumber);
    }
}

module_init(isr_gpio_init);
module_exit(isr_gpio_exit);

MODULE_DESCRIPTION("ISR for GPIO");
MODULE_AUTHOR("Stane Shieh <Stane.Shieh@wnc.com.tw>");
MODULE_LICENSE("GPL");
