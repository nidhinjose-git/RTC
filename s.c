#include <linux/module.h>   /* Needed by all modules */
#include <linux/kernel.h>   /* Needed for KERN_INFO */

#include <linux/time.h>
#include <linux/err.h>
#include <linux/rtc.h>

#include <linux/pci.h>

static int fake_rtc_read_time(struct device *dev, struct rtc_time *tm) 
{ 
    //This is wrong to_rtc_device() won't work here
    //struct rtc_device *rtc = to_rtc_device(dev);
    static int counter = -1;

    printk("read_time() callback: struct device* address = %p\n", dev);
    counter++;
    //printk(KERN_INFO "Reading time rtc-%d", rtc->id);
    printk(KERN_INFO "Reading time rtc");
    tm->tm_sec = (44 + counter) % 60;

    tm->tm_min = 33;
    tm->tm_hour = 22;
    tm->tm_mday = 11;
    tm->tm_wday = 7;
    tm->tm_mon = 6;
    tm->tm_year = 121;
    return rtc_valid_tm(tm); 
} 
 
static int fake_rtc_set_time(struct device *dev, struct rtc_time *tm) 
{ 
   printk(KERN_INFO "Writing time");
   return 0; 
} 

static const struct rtc_class_ops fake_rtc_ops = { 
   .read_time = fake_rtc_read_time, 
   .set_time = fake_rtc_set_time 
}; 

void release(struct device *dev)
{
    printk(KERN_INFO "Release function called\n");
}

struct rtc_device *rtc = NULL;
struct rtc_device *rtc2 = NULL;
struct pci_dev *pci_device = NULL;
struct device dev = {
    .parent = NULL,
    .release = release
};

int init_modulea(void)
{
    printk(KERN_INFO "Module inserted\n");
    pci_device = pci_get_device(0x8086, 0x100e, NULL);
    if (pci_device == NULL) {
        printk("PCI device not found\n");
        return -1;
    }
    pci_dev_put(pci_device);
    printk("Addr of pci_device->dev.kobj = %p\n", &pci_device->dev.kobj);
    printk("pci device found, refcount = %u struct device address = %p\n\n", 
           kref_read(&pci_device->dev.kobj.kref), &pci_device->dev);
    //return 0;
    rtc = rtc_device_register("My_RTC", &pci_device->dev, &fake_rtc_ops, THIS_MODULE);
/*
    if (device_register(&dev)) {
    printk("Device registre failed\n");
    return -1;
}
rtc = rtc_device_register("My_RTC", &dev, &fake_rtc_ops, THIS_MODULE);
  */
    

    if (IS_ERR(rtc)) 
         return PTR_ERR(rtc);
    printk("RTC driver registered rtc-%d, rtc->dev.parent = %p\n", rtc->id, rtc->dev.parent);
    printk("rtc->dev.kobj.parent = \"%s\", %p\n", 
         kobject_name(rtc->dev.kobj.parent),rtc->dev.kobj.parent);
    printk("rtc->dev.kobj.parent->parent = \"%s\" %p\n",
         kobject_name(rtc->dev.kobj.parent->parent),rtc->dev.kobj.parent);
    printk("PCI device New ref_count = %u\n\n", kref_read(&pci_device->dev.kobj.kref));

    /* We cannot support UIE mode because we don't have an irq line.
     * This would prevenet ioctl(4, RTC_UIE_ON)  causing problems while
     * running hwclock -r on this rtc driver
     */
    rtc->uie_unsupported = 1;

    rtc2 = rtc_device_register("My_RTC2", &pci_device->dev, &fake_rtc_ops, THIS_MODULE);
    if (IS_ERR(rtc)) {
        printk("Second RTC driver registration failed\n");
         //return PTR_ERR(rtc);
        return 0;
    }
    printk("Second RTC driver registered rtc-%d, rtc2->dev.parent = %p\n", rtc2->id, rtc2->dev.parent);
    printk("rtc2->dev.kobj.parent = \"%s\" %p\n",
            kobject_name(rtc->dev.kobj.parent), rtc2->dev.kobj.parent);
    printk("rtc2->dev.kobj.parent->parent = \"%s\", %p\n",
            kobject_name(rtc2->dev.kobj.parent->parent), rtc2->dev.kobj.parent->parent);
    printk("PCI device New ref_count = %u\n", kref_read(&pci_device->dev.kobj.kref));
    
    return 0;
}


void cleanup_modulea(void)
{
    printk(KERN_INFO "Goodbye world 1.\n");

    if (rtc)
        rtc_device_unregister(rtc);
    if (rtc2)
        rtc_device_unregister(rtc2);
}

module_init(init_modulea);
module_exit(cleanup_modulea);
MODULE_AUTHOR("Roberts module");
MODULE_DESCRIPTION("A Simple Hello World module");
MODULE_LICENSE("GPL");
