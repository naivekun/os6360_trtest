#ifndef __wncSysCpldFs_h__
#define __wncSysCpldFs_h__

/* Generic CPLD sysfs file definition macros */
#define SYSFS_RAW_RO_ATTR_DEF(field)  \
struct device_attribute field  \
    = __ATTR(field, S_IRUGO, system_cpld_##field##_raw_read, NULL);

#define SYSFS_RAW_RW_ATTR_DEF(field)  \
struct device_attribute field  \
    = __ATTR(field, S_IRUGO | S_IWUSR, system_cpld_##field##_raw_read, system_cpld_##field##_raw_write);

#define SYSFS_MISC_RO_ATTR_DEF(field, _read)  \
struct device_attribute field  \
    = __ATTR(field, S_IRUGO, _read, NULL);

#define SYSFS_MISC_RW_ATTR_DEF(field, _read, _write)  \
struct device_attribute field  \
    = __ATTR(field, S_IRUGO | S_IWUSR, _read, _write);

#define SYSFS_ATTR_PTR(field)  \
&field.attr


/* Declare system CPLD file system */
SYSFS_RAW_RO_ATTR_DEF(product_id)

SYSFS_RAW_RW_ATTR_DEF(sys_3v3_dis)

SYSFS_RAW_RO_ATTR_DEF(sfp_plus_present_0)
SYSFS_RAW_RO_ATTR_DEF(sfp_plus_tx_fault_0)
SYSFS_RAW_RO_ATTR_DEF(sfp_plus_rx_loss_0)
SYSFS_RAW_RO_ATTR_DEF(sfp_plus_present_1)
SYSFS_RAW_RO_ATTR_DEF(sfp_plus_tx_fault_1)
SYSFS_RAW_RO_ATTR_DEF(sfp_plus_rx_loss_1)
SYSFS_RAW_RO_ATTR_DEF(sfp_plus_present_2)
SYSFS_RAW_RO_ATTR_DEF(sfp_plus_tx_fault_2)
SYSFS_RAW_RO_ATTR_DEF(sfp_plus_rx_loss_2)
SYSFS_RAW_RO_ATTR_DEF(sfp_plus_present_3)
SYSFS_RAW_RO_ATTR_DEF(sfp_plus_tx_fault_3)
SYSFS_RAW_RO_ATTR_DEF(sfp_plus_rx_loss_3)

SYSFS_RAW_RO_ATTR_DEF(pwr_12v_ok)

SYSFS_RAW_RO_ATTR_DEF(cpld_rev)

SYSFS_RAW_RW_ATTR_DEF(i2cready_int_en)
SYSFS_RAW_RW_ATTR_DEF(i2cready_int)
SYSFS_RAW_RW_ATTR_DEF(pd69200_dis_en_sel)
SYSFS_RAW_RW_ATTR_DEF(pd69200_int_reset)
SYSFS_RAW_RO_ATTR_DEF(pd69200_ok)
SYSFS_RAW_RW_ATTR_DEF(poe_rst_n)
SYSFS_RAW_RW_ATTR_DEF(en_pd69200)
SYSFS_RAW_RO_ATTR_DEF(pd69200_i2c_ready)

SYSFS_RAW_RW_ATTR_DEF(vc_blink_count)

SYSFS_RAW_RW_ATTR_DEF(led_blink_rate)
SYSFS_RAW_RW_ATTR_DEF(vc_led_blink)
SYSFS_RAW_RW_ATTR_DEF(vc_led)
SYSFS_RAW_RW_ATTR_DEF(diag_led_blink)
SYSFS_RAW_RW_ATTR_DEF(diag_led)
SYSFS_RAW_RW_ATTR_DEF(pwr_led_blink)
SYSFS_RAW_RW_ATTR_DEF(pwr_led)

SYSFS_RAW_RW_ATTR_DEF(tx_disable_3)
SYSFS_RAW_RW_ATTR_DEF(tx_disable_2)
SYSFS_RAW_RW_ATTR_DEF(tx_disable_1)
SYSFS_RAW_RW_ATTR_DEF(tx_disable_0)

SYSFS_RAW_RW_ATTR_DEF(rst_hgphy)
SYSFS_RAW_RW_ATTR_DEF(rst_phy)

SYSFS_RAW_RW_ATTR_DEF(wdt_timer)
SYSFS_RAW_RW_ATTR_DEF(wdt_int_en)
SYSFS_RAW_RW_ATTR_DEF(wdt_rst_en)
SYSFS_RAW_RW_ATTR_DEF(wdt_counter_en)
SYSFS_RAW_RW_ATTR_DEF(wdt_freq_sel)
SYSFS_RAW_RW_ATTR_DEF(wdt_int_counter_mask)
SYSFS_RAW_RW_ATTR_DEF(wdt_int_flag)

SYSFS_RAW_RW_ATTR_DEF(_12v_pg_int_en)
SYSFS_RAW_RW_ATTR_DEF(pd69200_int_en)
SYSFS_RAW_RW_ATTR_DEF(thm_int_en)
SYSFS_RAW_RW_ATTR_DEF(fan_fail_int_en)
SYSFS_RAW_RW_ATTR_DEF(_12v_pg_int)
SYSFS_RAW_RW_ATTR_DEF(pd69200_int)
SYSFS_RAW_RW_ATTR_DEF(thm_pend)
SYSFS_RAW_RW_ATTR_DEF(fan_fail_int_pend)

SYSFS_RAW_RW_ATTR_DEF(_88e1543_phy_6_int_en)
SYSFS_RAW_RW_ATTR_DEF(_88e1680_phy_5_int_en)
SYSFS_RAW_RW_ATTR_DEF(_88e1680_phy_4_int_en)
SYSFS_RAW_RW_ATTR_DEF(_88e1680_phy_3_int_en)
SYSFS_RAW_RW_ATTR_DEF(_88e1680_phy_2_int_en)
SYSFS_RAW_RW_ATTR_DEF(_88e1680_phy_1_int_en)
SYSFS_RAW_RW_ATTR_DEF(_88e1680_phy_0_int_en)
SYSFS_RAW_RW_ATTR_DEF(_88e3220_phy_9_int_en)
SYSFS_RAW_RW_ATTR_DEF(_88e2010_phy_8_int_en)
SYSFS_RAW_RW_ATTR_DEF(_88e2010_phy_7_int_en)

SYSFS_RAW_RW_ATTR_DEF(sfp_plus_1_int_en)
SYSFS_RAW_RW_ATTR_DEF(sfp_plus_0_int_en)
SYSFS_RAW_RW_ATTR_DEF(sfp_1_int_en)
SYSFS_RAW_RW_ATTR_DEF(sfp_0_int_en)

SYSFS_RAW_RW_ATTR_DEF(_88e1543_phy_6_int)
SYSFS_RAW_RW_ATTR_DEF(_88e1680_phy_5_int)
SYSFS_RAW_RW_ATTR_DEF(_88e1680_phy_4_int)
SYSFS_RAW_RW_ATTR_DEF(_88e1680_phy_3_int)
SYSFS_RAW_RW_ATTR_DEF(_88e1680_phy_2_int)
SYSFS_RAW_RW_ATTR_DEF(_88e1680_phy_1_int)
SYSFS_RAW_RW_ATTR_DEF(_88e1680_phy_0_int)
SYSFS_RAW_RW_ATTR_DEF(_88e3220_phy_9_int)
SYSFS_RAW_RW_ATTR_DEF(_88e2010_phy_8_int)
SYSFS_RAW_RW_ATTR_DEF(_88e2010_phy_7_int)

SYSFS_RAW_RW_ATTR_DEF(sfp_plus_1_int_pending)
SYSFS_RAW_RW_ATTR_DEF(sfp_plus_0_int_pending)
SYSFS_RAW_RW_ATTR_DEF(sfp_1_int_pending)
SYSFS_RAW_RW_ATTR_DEF(sfp_0_int_pending)

SYSFS_RAW_RW_ATTR_DEF(sfp_plus_1_tx_fault_int_en)
SYSFS_RAW_RW_ATTR_DEF(sfp_plus_0_tx_fault_int_en)
SYSFS_RAW_RW_ATTR_DEF(sfp_1_tx_fault_int_en)
SYSFS_RAW_RW_ATTR_DEF(sfp_0_tx_fault_int_en)
SYSFS_RAW_RW_ATTR_DEF(sfp_plus_1_rx_los_int_en)
SYSFS_RAW_RW_ATTR_DEF(sfp_plus_0_rx_los_int_en)
SYSFS_RAW_RW_ATTR_DEF(sfp_1_rx_los_int_en)
SYSFS_RAW_RW_ATTR_DEF(sfp_0_rx_los_int_en)
SYSFS_RAW_RW_ATTR_DEF(sfp_plus_1_tx_fault_int_pend)
SYSFS_RAW_RW_ATTR_DEF(sfp_plus_0_tx_fault_int_pend)
SYSFS_RAW_RW_ATTR_DEF(sfp_1_tx_fault_int_pend)
SYSFS_RAW_RW_ATTR_DEF(sfp_0_tx_fault_int_pend)
SYSFS_RAW_RW_ATTR_DEF(sfp_plus_1_rx_los_pend)
SYSFS_RAW_RW_ATTR_DEF(sfp_plus_0_rx_los_pend)
SYSFS_RAW_RW_ATTR_DEF(sfp_1_rx_los_pend)
SYSFS_RAW_RW_ATTR_DEF(sfp_0_rx_los_pend)

SYSFS_RAW_RW_ATTR_DEF(system_reset_n)
SYSFS_RAW_RW_ATTR_DEF(_9548_rst)

SYSFS_RAW_RW_ATTR_DEF(sfp_plus_2_stack_up)
SYSFS_RAW_RW_ATTR_DEF(sfp_plus_3_stack_up)

SYSFS_RAW_RO_ATTR_DEF(fan2_fail)
SYSFS_RAW_RO_ATTR_DEF(fan1_fail)

SYSFS_RAW_RW_ATTR_DEF(sys_fan_1_pwm)
SYSFS_RAW_RO_ATTR_DEF(sys_fan_1_status)
SYSFS_RAW_RO_ATTR_DEF(sys_fan_2_status)

SYSFS_RAW_RW_ATTR_DEF(usb_ovc_en)
SYSFS_RAW_RW_ATTR_DEF(thm_alert_rst)
SYSFS_RAW_RW_ATTR_DEF(usb_pw_en)
SYSFS_RAW_RO_ATTR_DEF(usb_ovc)

SYSFS_RAW_RW_ATTR_DEF(cpld_programming_en)

SYSFS_RAW_RW_ATTR_DEF(sys_led_pwr_ctrl_en)
SYSFS_RAW_RW_ATTR_DEF(sfp_led_pwr_ctrl_en)

SYSFS_RAW_RO_ATTR_DEF(product_family)
SYSFS_RAW_RO_ATTR_DEF(product_model)

SYSFS_RAW_RO_ATTR_DEF(date_code)

#endif /* __wncSysCpldFs_h__ */
