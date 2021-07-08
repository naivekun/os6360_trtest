#include <linux/module.h>
#include <linux/i2c.h>

#include "wncSysCpldReg.h"
#include "wncSysCpldFs.h"

static int debug_flag = 0;

struct system_cpld_data {
    struct mutex lock;
    struct i2c_client *client;
    struct device_attribute bin;
};
struct system_cpld_data *system_cpld;

static const struct i2c_device_id system_cpld_ids[] = {
    { "os6360_sys_cpld", 0 },
    { /* END OF LIST */ }
};
MODULE_DEVICE_TABLE(i2c, system_cpld_ids);

static int system_cpld_raw_read(struct device *dev, struct device_attribute *attr, char *buf,
    int reg_offset, int reg_width, int fld_shift, int fld_width, int fld_mask, char *reg_name)
{
    unsigned int reg_val = 0, fld_val;
    static int debug_flag;
    struct system_cpld_data *data = dev_get_drvdata(dev);
    struct i2c_client *client = data->client;
    int err;

    if(reg_width != 8)
    {
        printk("%s: Register table width setting failed.\n", reg_name);
        return -EINVAL;
    }

    mutex_lock(&data->lock);

    if((err = i2c_smbus_read_byte_data(client, (u8)reg_offset)) < 0)
    {
        /* CPLD read error condition */;
        mutex_unlock(&data->lock);
        printk("%s: i2c read failed, error code = %d.\n", reg_name, err);
        return err;
    }
    reg_val = err;

    if(debug_flag)
    {
        printk("%s: reg_offset = %d, width = %d, cur value = 0x%x.\n", reg_name, reg_offset, reg_width, reg_val);
    }

    mutex_unlock(&data->lock);

    if(fld_width == reg_width)
    {
        fld_val = reg_val & fld_mask;
    }
    else
    {
        fld_val = (reg_val >> fld_shift) & fld_mask;
    }

    return sprintf(buf, "0x%x\n", fld_val);
}

static int system_cpld_raw_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count,
    int reg_offset, int reg_width, int fld_shift, int fld_width, int fld_mask, char *reg_name)
{
    int ret_code;
    unsigned int reg_val, fld_val;
    unsigned long val;
    static int debug_flag;
    struct system_cpld_data *data = dev_get_drvdata(dev);
    struct i2c_client *client = data->client;

    if(reg_width != 8)
    {
        printk("%s: Register table width setting failed.\n", reg_name);
        return -EINVAL;
    }

    /* Parse buf and store to fld_val */
    if((ret_code = kstrtoul(buf, 16, &val)))
    {
        printk("%s: Conversion value = %s failed, errno = %d.\n", reg_name, buf, ret_code);
        return ret_code;
    }
    fld_val = (unsigned int)val;

    mutex_lock(&data->lock);

    if((ret_code = i2c_smbus_read_byte_data(client, (u8)reg_offset)) < 0)
    {
        /* Handle CPLD read error condition */;
        mutex_unlock(&data->lock);
        printk("%s: i2c read failed, error code = %d.\n",  reg_name, ret_code);
        return ret_code;
    }
    reg_val = ret_code;

    if(debug_flag)
    {
        printk("%s: offset = %d, width = %d, cur value = 0x%x.\n", reg_name, reg_offset, reg_width, reg_val);
    }

    if(fld_width == reg_width)
    {
        reg_val = fld_val & fld_mask;
    }
    else
    {
        reg_val = (reg_val & ~(fld_mask << fld_shift)) |
                    ((fld_val & (fld_mask)) << fld_shift);
    }

    if((ret_code = i2c_smbus_write_byte_data(client, (u8)reg_offset, (u8)reg_val)) != 0)
    {
        /* Handle CPLD write error condition */;
        mutex_unlock(&data->lock);
        printk("%s: i2c write failed, error code = %d.\n",  reg_name, ret_code);
        return ret_code;
    }
    else if(debug_flag)
    {
        printk("%s: offset = %d, width = %d, new value = 0x%x.\n", reg_name, reg_offset, reg_width, reg_val);
    }

    mutex_unlock(&data->lock);

    return count;
}

/* ---------------------- Read function for specific fields --------------------------- */
static int system_cpld_ver_raw_read(struct device *dev, struct device_attribute *attr, char *buf,
    int reg_offset, char *fld_name)
{
    unsigned int fld_val = 0;
    static int debug_flag;
    struct system_cpld_data *data = dev_get_drvdata(dev);
    struct i2c_client *client = data->client;
    int err_lo, err_hi;

    mutex_lock(&data->lock);

    if((err_hi = i2c_smbus_read_byte_data(client, (u8)reg_offset)) < 0)
    {
        /* CPLD read error condition */;
        mutex_unlock(&data->lock);
        printk("%s: I2C read high byte failed, error code = %d.\n", fld_name, err_hi);
        return err_hi;
    }

    if((err_lo = i2c_smbus_read_byte_data(client, (u8)(reg_offset + 1))) < 0)
    {
        /* CPLD read error condition */;
        mutex_unlock(&data->lock);
        printk("%s: I2C read low byte failed, error code = %d.\n", fld_name, err_lo);
        return err_lo;
    }

    if(debug_flag)
    {
        printk("%s: reg_offset = %d, cur value = 0x%x.\n", fld_name, reg_offset, err_hi);
        printk("%s: reg_offset = %d, cur value = 0x%x.\n", fld_name, reg_offset + 1, err_lo);
    }

    mutex_unlock(&data->lock);

    fld_val = (err_hi << 8) + err_lo;

    return sprintf(buf, "0x%x\n", fld_val);
}

/*-------------------- Special file for debug ---------------------- */
static ssize_t system_cpld_debug_read(struct device *dev, struct device_attribute *attr,
             char *buf)
{
    return sprintf(buf, "%d\n", debug_flag);
}


static ssize_t system_cpld_debug_write(struct device *dev, struct device_attribute *attr,
            const char *buf, size_t count)
{
    int temp;
    int error;

    error = kstrtoint(buf, 10, &temp);
    if(error)
    {
        printk(KERN_INFO "%s: Conversion value = %s failed.\n", __FUNCTION__, buf);
        return count;
    }
    debug_flag = temp;

    if(debug_flag)
        printk("%s, debug_flag = %d\n", __FUNCTION__, debug_flag);

    return count;
}
SYSFS_MISC_RW_ATTR_DEF(debug, system_cpld_debug_read, system_cpld_debug_write)


/* ---------------- Define misc group ---------------------------- */
static struct attribute *misc_attributes[] = {
    SYSFS_ATTR_PTR(product_id),                   /* register: product_id_reg */

    SYSFS_ATTR_PTR(sys_3v3_dis),                  /* register: power_suntdown_rst_reg */

    SYSFS_ATTR_PTR(sfp_plus_present_0),           /* register: gbic_status_0_reg */
    SYSFS_ATTR_PTR(sfp_plus_tx_fault_0),          /* register: gbic_status_0_reg */
    SYSFS_ATTR_PTR(sfp_plus_rx_loss_0),           /* register: gbic_status_0_reg */
    SYSFS_ATTR_PTR(sfp_plus_present_1),           /* register: gbic_status_0_reg */
    SYSFS_ATTR_PTR(sfp_plus_tx_fault_1),          /* register: gbic_status_0_reg */
    SYSFS_ATTR_PTR(sfp_plus_rx_loss_1),           /* register: gbic_status_0_reg */
    SYSFS_ATTR_PTR(sfp_plus_present_2),           /* register: gbic_status_1_reg */
    SYSFS_ATTR_PTR(sfp_plus_tx_fault_2),          /* register: gbic_status_1_reg */
    SYSFS_ATTR_PTR(sfp_plus_rx_loss_2),           /* register: gbic_status_1_reg */
    SYSFS_ATTR_PTR(sfp_plus_present_3),           /* register: gbic_status_1_reg */
    SYSFS_ATTR_PTR(sfp_plus_tx_fault_3),          /* register: gbic_status_1_reg */
    SYSFS_ATTR_PTR(sfp_plus_rx_loss_3),           /* register: gbic_status_1_reg */

    SYSFS_ATTR_PTR(pwr_12v_ok),                   /* register: pwr_status_reg */

    SYSFS_ATTR_PTR(cpld_rev),                     /* register: cpld_revision_reg */

    SYSFS_ATTR_PTR(i2cready_int_en),              /* register: poe_control_reg */
    SYSFS_ATTR_PTR(i2cready_int),                 /* register: poe_control_reg */
    SYSFS_ATTR_PTR(pd69200_dis_en_sel),           /* register: poe_control_reg */
    SYSFS_ATTR_PTR(pd69200_int_reset),            /* register: poe_control_reg */
    SYSFS_ATTR_PTR(pd69200_ok),                   /* register: poe_control_reg */
    SYSFS_ATTR_PTR(poe_rst_n),                    /* register: poe_control_reg */
    SYSFS_ATTR_PTR(en_pd69200),                   /* register: poe_control_reg */
    SYSFS_ATTR_PTR(pd69200_i2c_ready),            /* register: poe_control_reg */

    SYSFS_ATTR_PTR(vc_blink_count),               /* register: vc_led_blink_count_reg */

    SYSFS_ATTR_PTR(led_blink_rate),               /* register: system_led_1_reg */
    SYSFS_ATTR_PTR(vc_led_blink),                 /* register: system_led_1_reg */
    SYSFS_ATTR_PTR(vc_led),                       /* register: system_led_1_reg */
    SYSFS_ATTR_PTR(diag_led_blink),               /* register: system_led_1_reg */
    SYSFS_ATTR_PTR(diag_led),                     /* register: system_led_1_reg */
    SYSFS_ATTR_PTR(pwr_led_blink),                /* register: system_led_2_reg */
    SYSFS_ATTR_PTR(pwr_led),                      /* register: system_led_2_reg */

    SYSFS_ATTR_PTR(tx_disable_3),                 /* register: gbic_control__reg */
    SYSFS_ATTR_PTR(tx_disable_2),                 /* register: gbic_control__reg */
    SYSFS_ATTR_PTR(tx_disable_1),                 /* register: gbic_control__reg */
    SYSFS_ATTR_PTR(tx_disable_0),                 /* register: gbic_control__reg */

    SYSFS_ATTR_PTR(rst_hgphy),                    /* register: reset_phy_reg */
    SYSFS_ATTR_PTR(rst_phy),                      /* register: reset_phy_reg */

    SYSFS_ATTR_PTR(wdt_timer),                    /* register: wd_timer_reg */
    SYSFS_ATTR_PTR(wdt_int_en),                   /* register: wd_timer_control_reg */
    SYSFS_ATTR_PTR(wdt_rst_en),                   /* register: wd_timer_control_reg */
    SYSFS_ATTR_PTR(wdt_counter_en),               /* register: wd_timer_control_reg */
    SYSFS_ATTR_PTR(wdt_freq_sel),                 /* register: wd_timer_control_reg */
    SYSFS_ATTR_PTR(wdt_int_counter_mask),         /* register: wd_int_counter_mask_reg */
    SYSFS_ATTR_PTR(wdt_int_flag),                 /* register: wd_int_reg */

    SYSFS_ATTR_PTR(_12v_pg_int_en),               /* register: ext_dev_int_en_reg */
    SYSFS_ATTR_PTR(pd69200_int_en),               /* register: ext_dev_int_en_reg */
    SYSFS_ATTR_PTR(thm_int_en),                   /* register: ext_dev_int_en_reg */
    SYSFS_ATTR_PTR(fan_fail_int_en),              /* register: ext_dev_int_en_reg */
    SYSFS_ATTR_PTR(_12v_pg_int),                  /* register: ext_dev_int_pending_reg */
    SYSFS_ATTR_PTR(pd69200_int),                  /* register: ext_dev_int_pending_reg */
    SYSFS_ATTR_PTR(thm_pend),                     /* register: ext_dev_int_pending_reg */
    SYSFS_ATTR_PTR(fan_fail_int_pend),            /* register: ext_dev_int_pending_reg */

    SYSFS_ATTR_PTR(_88e1543_phy_6_int_en),        /* register: phy_int_en_1_reg */
    SYSFS_ATTR_PTR(_88e1680_phy_5_int_en),        /* register: phy_int_en_1_reg */
    SYSFS_ATTR_PTR(_88e1680_phy_4_int_en),        /* register: phy_int_en_1_reg */
    SYSFS_ATTR_PTR(_88e1680_phy_3_int_en),        /* register: phy_int_en_1_reg */
    SYSFS_ATTR_PTR(_88e1680_phy_2_int_en),        /* register: phy_int_en_1_reg */
    SYSFS_ATTR_PTR(_88e1680_phy_1_int_en),        /* register: phy_int_en_1_reg */
    SYSFS_ATTR_PTR(_88e1680_phy_0_int_en),        /* register: phy_int_en_1_reg */
    SYSFS_ATTR_PTR(_88e3220_phy_9_int_en),        /* register: phy_int_en_2_reg */
    SYSFS_ATTR_PTR(_88e2010_phy_8_int_en),        /* register: phy_int_en_2_reg */
    SYSFS_ATTR_PTR(_88e2010_phy_7_int_en),        /* register: phy_int_en_2_reg */
    
    SYSFS_ATTR_PTR(sfp_plus_1_int_en),            /* register: port_int_en_reg */
    SYSFS_ATTR_PTR(sfp_plus_0_int_en),            /* register: port_int_en_reg */
    SYSFS_ATTR_PTR(sfp_1_int_en),                 /* register: port_int_en_reg */
    SYSFS_ATTR_PTR(sfp_0_int_en),                 /* register: port_int_en_reg */

    SYSFS_ATTR_PTR(_88e1543_phy_6_int),           /* register: phy_int_1_reg */
    SYSFS_ATTR_PTR(_88e1680_phy_5_int),           /* register: phy_int_1_reg */
    SYSFS_ATTR_PTR(_88e1680_phy_4_int),           /* register: phy_int_1_reg */
    SYSFS_ATTR_PTR(_88e1680_phy_3_int),           /* register: phy_int_1_reg */
    SYSFS_ATTR_PTR(_88e1680_phy_2_int),           /* register: phy_int_1_reg */
    SYSFS_ATTR_PTR(_88e1680_phy_1_int),           /* register: phy_int_1_reg */
    SYSFS_ATTR_PTR(_88e1680_phy_0_int),           /* register: phy_int_1_reg */
    SYSFS_ATTR_PTR(_88e3220_phy_9_int),           /* register: phy_int_2_reg */
    SYSFS_ATTR_PTR(_88e2010_phy_8_int),           /* register: phy_int_2_reg */
    SYSFS_ATTR_PTR(_88e2010_phy_7_int),           /* register: phy_int_2_reg */

    SYSFS_ATTR_PTR(sfp_plus_1_int_pending),       /* register: port_int_pending_reg */
    SYSFS_ATTR_PTR(sfp_plus_0_int_pending),       /* register: port_int_pending_reg */
    SYSFS_ATTR_PTR(sfp_1_int_pending),            /* register: port_int_pending_reg */
    SYSFS_ATTR_PTR(sfp_0_int_pending),            /* register: port_int_pending_reg */

    SYSFS_ATTR_PTR(sfp_plus_1_tx_fault_int_en),   /* register: tx_fault_rx_los_int_en_reg */
    SYSFS_ATTR_PTR(sfp_plus_0_tx_fault_int_en),   /* register: tx_fault_rx_los_int_en_reg */
    SYSFS_ATTR_PTR(sfp_1_tx_fault_int_en),        /* register: tx_fault_rx_los_int_en_reg */
    SYSFS_ATTR_PTR(sfp_0_tx_fault_int_en),        /* register: tx_fault_rx_los_int_en_reg */
    SYSFS_ATTR_PTR(sfp_plus_1_rx_los_int_en),     /* register: tx_fault_rx_los_int_en_reg */
    SYSFS_ATTR_PTR(sfp_plus_0_rx_los_int_en),     /* register: tx_fault_rx_los_int_en_reg */
    SYSFS_ATTR_PTR(sfp_1_rx_los_int_en),          /* register: tx_fault_rx_los_int_en_reg */
    SYSFS_ATTR_PTR(sfp_0_rx_los_int_en),          /* register: tx_fault_rx_los_int_en_reg */
    SYSFS_ATTR_PTR(sfp_plus_1_tx_fault_int_pend), /* register: tx_fault_rx_los_int_pend_reg */
    SYSFS_ATTR_PTR(sfp_plus_0_tx_fault_int_pend), /* register: tx_fault_rx_los_int_pend_reg */
    SYSFS_ATTR_PTR(sfp_1_tx_fault_int_pend),      /* register: tx_fault_rx_los_int_pend_reg */
    SYSFS_ATTR_PTR(sfp_0_tx_fault_int_pend),      /* register: tx_fault_rx_los_int_pend_reg */
    SYSFS_ATTR_PTR(sfp_plus_1_rx_los_pend),       /* register: tx_fault_rx_los_int_pend_reg */
    SYSFS_ATTR_PTR(sfp_plus_0_rx_los_pend),       /* register: tx_fault_rx_los_int_pend_reg */
    SYSFS_ATTR_PTR(sfp_1_rx_los_pend),            /* register: tx_fault_rx_los_int_pend_reg */
    SYSFS_ATTR_PTR(sfp_0_rx_los_pend),            /* register: tx_fault_rx_los_int_pend_reg */

    SYSFS_ATTR_PTR(system_reset_n),               /* register: reset_system_reg */
    SYSFS_ATTR_PTR(_9548_rst),                    /* register: reset_system_reg */

    SYSFS_ATTR_PTR(sfp_plus_2_stack_up),          /* register: ports_led_type_reg */
    SYSFS_ATTR_PTR(sfp_plus_3_stack_up),          /* register: ports_led_type_reg */

    SYSFS_ATTR_PTR(fan2_fail),                    /* register: fan_status_reg */
    SYSFS_ATTR_PTR(fan1_fail),                    /* register: fan_status_reg */

    SYSFS_ATTR_PTR(sys_fan_1_pwm),                /* register: system_fan_1_contrl_reg */
    SYSFS_ATTR_PTR(sys_fan_1_status),             /* register: system_fan_1_status_reg */
    SYSFS_ATTR_PTR(sys_fan_2_status),             /* register: system_fan_2_status_reg */

    SYSFS_ATTR_PTR(usb_ovc_en),                   /* register: usb_control_status_reg */
    SYSFS_ATTR_PTR(thm_alert_rst),                /* register: usb_control_status_reg */
    SYSFS_ATTR_PTR(usb_pw_en),                    /* register: usb_control_status_reg */
    SYSFS_ATTR_PTR(usb_ovc),                      /* register: usb_control_status_reg */

    SYSFS_ATTR_PTR(cpld_programming_en),          /* register: cpld_programming_reg */
    SYSFS_ATTR_PTR(sys_led_pwr_ctrl_en),          /* register: led_power_ctrl_reg */
    SYSFS_ATTR_PTR(sfp_led_pwr_ctrl_en),          /* register: led_power_ctrl_reg */
    SYSFS_ATTR_PTR(product_family),               /* register: product_family_reg */
    SYSFS_ATTR_PTR(product_model),                /* register: product_model_reg */
    SYSFS_ATTR_PTR(date_code),                    /* register: date_code_reg */
    NULL
};
static const struct attribute_group system_cpld_group_misc = {
    .attrs = misc_attributes,
};

static int system_cpld_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int err;

    /* allocate memory to system_cpld */
    system_cpld = devm_kzalloc(&client->dev, sizeof(struct system_cpld_data), GFP_KERNEL);

    if(!system_cpld)
        return -ENOMEM;

    mutex_init(&system_cpld->lock);

    err = sysfs_create_group(&client->dev.kobj, &system_cpld_group_misc);
    if(err)
    {
        printk("%s: Error creeat misc group.\n", __FUNCTION__);
    }

    system_cpld->client = client;
    i2c_set_clientdata(client, system_cpld);

    printk(KERN_INFO "%s: System CPLD created.\n", __FUNCTION__);

    return 0;
}

static int system_cpld_remove(struct i2c_client *client)
{
    sysfs_remove_group(&client->dev.kobj, &system_cpld_group_misc);

    printk(KERN_INFO "%s: System CPLD removed.\n", __FUNCTION__);
    return 0;
}

static struct i2c_driver system_cpld_driver = {
    .driver = {
        .name = "os6360_sys_cpld",
        .owner = THIS_MODULE,
    },
    .probe = system_cpld_probe,
    .remove = system_cpld_remove,
    .id_table = system_cpld_ids,
};

static int __init system_cpld_init(void)
{
    printk(KERN_INFO "%s: init.\n", __FUNCTION__);
    return i2c_add_driver(&system_cpld_driver);
}
module_init(system_cpld_init);

static void __exit system_cpld_exit(void)
{
    printk(KERN_INFO "%s: exit.\n", __FUNCTION__);
    i2c_del_driver(&system_cpld_driver);
}
module_exit(system_cpld_exit);

MODULE_DESCRIPTION("Driver for system CPLD");
MODULE_AUTHOR("Kily Chen <Kily.Chen@wnc.com.tw>");
MODULE_LICENSE("GPL");

