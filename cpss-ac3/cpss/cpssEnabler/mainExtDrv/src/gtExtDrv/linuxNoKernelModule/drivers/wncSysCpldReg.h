#ifndef __wncSysCpldReg_h__
#define __wncSysCpldReg_h__

static int system_cpld_raw_read(struct device *dev, struct device_attribute *attr, char *buf,
    int reg_offset, int reg_width, int fld_shift, int fld_width, int fld_mask, char *reg_name);
static int system_cpld_raw_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count,
    int reg_offset, int reg_width, int fld_shift, int fld_width, int fld_mask, char *reg_name);

/* Generic CPLD read function */
#define FLD_RAW_RD_FUNC(_reg, _fld, _wdh) static ssize_t \
system_cpld_##_fld##_raw_read(struct device *dev, struct device_attribute *attr, char *buf) { \
    return system_cpld_raw_read(dev, attr, buf, _reg##_offset, _reg##_width, _fld##_shift, _fld##_width, _fld##_mask, #_reg); \
}

/* Generic CPLD write function */
#define FLD_RAW_WR_FUNC(_reg, _fld, _wdh) static ssize_t \
system_cpld_##_fld##_raw_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { \
    return system_cpld_raw_write(dev, attr, buf, count, _reg##_offset, _reg##_width, _fld##_shift, _fld##_width, _fld##_mask, #_reg); \
}

/* CPLD register definition macros */
#define REG_DEF(_reg, _off, _wdh) \
static unsigned int _reg##_offset = (unsigned int)(_off); \
static unsigned int _reg##_width = (unsigned int)(_wdh);

/* CPLD register field definition macros, with generic read/write function */
#define FLD_RAW_RO_DEF(_reg, _fld, _sft, _wdh) \
static unsigned int _fld##_shift = (unsigned int)(_sft); \
static unsigned int _fld##_width = (unsigned int)(_wdh); \
static unsigned int _fld##_mask = ((((unsigned int)1) << (_wdh)) - 1); \
FLD_RAW_RD_FUNC(_reg, _fld, _wdh)

#define FLD_RAW_RW_DEF(_reg, _fld, _sft, _wdh) \
static unsigned int _fld##_shift = (unsigned int)(_sft); \
static unsigned int _fld##_width = (unsigned int)(_wdh); \
static unsigned int _fld##_mask = ((((unsigned int)1) << (_wdh)) - 1); \
FLD_RAW_RD_FUNC(_reg, _fld, _wdh) FLD_RAW_WR_FUNC(_reg, _fld, _wdh)

/* Declare system CPLD registers */
/*           register name                           offset  width */
/*           --------------------------------------- ------- ----- */
REG_DEF(     product_id_reg,                         0x01,   8)
REG_DEF(     power_suntdown_rst_reg,                 0x02,   8)
REG_DEF(     gbic_status_0_reg,                      0x03,   8)
REG_DEF(     gbic_status_1_reg,                      0x04,   8)
REG_DEF(     pwr_status_reg,                         0x08,   8)
REG_DEF(     cpld_revision_reg,                      0x09,   8)
REG_DEF(     poe_control_reg,                        0x0b,   8)
REG_DEF(     vc_led_blink_count_reg,                 0x10,   8)
REG_DEF(     system_led_1_reg,                       0x11,   8)
REG_DEF(     system_led_2_reg,                       0x12,   8)
REG_DEF(     gbic_control__reg,                      0x14,   8)
REG_DEF(     reset_phy_reg,                          0x19,   8)
REG_DEF(     wd_timer_reg,                           0x1b,   8)
REG_DEF(     wd_timer_control_reg,                   0x1c,   8)
REG_DEF(     wd_int_counter_mask_reg,                0x1d,   8)
REG_DEF(     wd_int_reg,                             0x1e,   8)
REG_DEF(     ext_dev_int_en_reg,                     0x20,   8)
REG_DEF(     ext_dev_int_pending_reg,                0x24,   8)
REG_DEF(     phy_int_en_1_reg,                       0x25,   8)
REG_DEF(     phy_int_en_2_reg,                       0x26,   8)
REG_DEF(     port_int_en_reg,                        0x27,   8)
REG_DEF(     phy_int_1_reg,                          0x28,   8)
REG_DEF(     phy_int_2_reg,                          0x29,   8)
REG_DEF(     port_int_pending_reg,                   0x2a,   8)
REG_DEF(     tx_fault_rx_los_int_en_reg,             0x2c,   8)
REG_DEF(     tx_fault_rx_los_int_pend_reg,           0x2d,   8)
REG_DEF(     reset_system_reg,                       0x2e,   8)
REG_DEF(     ports_led_type_reg,                     0x38,   8)
REG_DEF(     fan_status_reg,                         0x3b,   8)
REG_DEF(     system_fan_1_contrl_reg,                0x3c,   8)
REG_DEF(     system_fan_1_status_reg,                0x3e,   8)
REG_DEF(     system_fan_2_status_reg,                0x3f,   8)
REG_DEF(     usb_control_status_reg,                 0x45,   8)
REG_DEF(     cpld_programming_reg,                   0x4d,   8)
REG_DEF(     led_power_ctrl_reg,                     0x50,   8)
REG_DEF(     product_family_reg,                     0x7c,   8)
REG_DEF(     product_model_reg,                      0x7d,   8)


/* Declare system CPLD register's fields */
/*                      register name               field name                    shift  width */
/*                      ----------------------      ----------------              ------ ----- */
FLD_RAW_RO_DEF(         product_id_reg,             product_id,                   0,      8)
FLD_RAW_RW_DEF(         power_suntdown_rst_reg,     sys_3v3_dis,                  7,      1)
FLD_RAW_RO_DEF(         gbic_status_0_reg,          sfp_plus_present_1,           7,      1)
FLD_RAW_RO_DEF(         gbic_status_0_reg,          sfp_plus_tx_fault_1,          6,      1)
FLD_RAW_RO_DEF(         gbic_status_0_reg,          sfp_plus_rx_loss_1,           5,      1)
FLD_RAW_RO_DEF(         gbic_status_0_reg,          sfp_plus_present_0,           3,      1)
FLD_RAW_RO_DEF(         gbic_status_0_reg,          sfp_plus_tx_fault_0,          2,      1)
FLD_RAW_RO_DEF(         gbic_status_0_reg,          sfp_plus_rx_loss_0,           1,      1)
FLD_RAW_RO_DEF(         gbic_status_1_reg,          sfp_plus_present_3,           7,      1)
FLD_RAW_RO_DEF(         gbic_status_1_reg,          sfp_plus_tx_fault_3,          6,      1)
FLD_RAW_RO_DEF(         gbic_status_1_reg,          sfp_plus_rx_loss_3,           5,      1)
FLD_RAW_RO_DEF(         gbic_status_1_reg,          sfp_plus_present_2,           3,      1)
FLD_RAW_RO_DEF(         gbic_status_1_reg,          sfp_plus_tx_fault_2,          2,      1)
FLD_RAW_RO_DEF(         gbic_status_1_reg,          sfp_plus_rx_loss_2,           1,      1)
FLD_RAW_RO_DEF(         pwr_status_reg,             pwr_12v_ok,                   7,      1)
FLD_RAW_RO_DEF(         cpld_revision_reg,          cpld_rev,                     0,      8)
FLD_RAW_RW_DEF(         poe_control_reg,            i2cready_int_en,              7,      1)
FLD_RAW_RW_DEF(         poe_control_reg,            i2cready_int,                 6,      1)
FLD_RAW_RW_DEF(         poe_control_reg,            pd69200_dis_en_sel,           5,      1)
FLD_RAW_RW_DEF(         poe_control_reg,            pd69200_int_reset,            4,      1)
FLD_RAW_RO_DEF(         poe_control_reg,            pd69200_ok,                   3,      1)
FLD_RAW_RW_DEF(         poe_control_reg,            poe_rst_n,                    2,      1)
FLD_RAW_RW_DEF(         poe_control_reg,            en_pd69200,                   1,      1)
FLD_RAW_RO_DEF(         poe_control_reg,            pd69200_i2c_ready,            0,      1)
FLD_RAW_RW_DEF(         vc_led_blink_count_reg,     vc_blink_count,               0,      4)
FLD_RAW_RW_DEF(         system_led_1_reg,           led_blink_rate,               7,      1)
FLD_RAW_RW_DEF(         system_led_1_reg,           vc_led_blink,                 6,      1)
FLD_RAW_RW_DEF(         system_led_1_reg,           vc_led,                       4,      2)
FLD_RAW_RW_DEF(         system_led_1_reg,           diag_led_blink,               2,      1)
FLD_RAW_RW_DEF(         system_led_1_reg,           diag_led,                     0,      2)  /* Diag/LED0  is "R" only ? It should "RW". */
FLD_RAW_RW_DEF(         system_led_2_reg,           pwr_led,                      2,      2)
FLD_RAW_RW_DEF(         system_led_2_reg,           pwr_led_blink,                1,      1)
FLD_RAW_RW_DEF(         gbic_control__reg,          tx_disable_3,                 3,      1)
FLD_RAW_RW_DEF(         gbic_control__reg,          tx_disable_2,                 2,      1)
FLD_RAW_RW_DEF(         gbic_control__reg,          tx_disable_1,                 1,      1)
FLD_RAW_RW_DEF(         gbic_control__reg,          tx_disable_0,                 0,      1)
FLD_RAW_RW_DEF(         reset_phy_reg,              rst_hgphy,                    1,      1)
FLD_RAW_RW_DEF(         reset_phy_reg,              rst_phy,                      0,      1)
FLD_RAW_RW_DEF(         wd_timer_reg,               wdt_timer,                    0,      8)
FLD_RAW_RW_DEF(         wd_timer_control_reg,       wdt_int_en,                   7,      1)
FLD_RAW_RW_DEF(         wd_timer_control_reg,       wdt_rst_en,                   6,      1)
FLD_RAW_RW_DEF(         wd_timer_control_reg,       wdt_counter_en,               5,      1)
FLD_RAW_RW_DEF(         wd_timer_control_reg,       wdt_freq_sel,                 0,      2)
FLD_RAW_RW_DEF(         wd_int_counter_mask_reg,    wdt_int_counter_mask,         0,      8)
FLD_RAW_RW_DEF(         wd_int_reg,                 wdt_int_flag,                 7,      1)
FLD_RAW_RW_DEF(         ext_dev_int_en_reg,         _12v_pg_int_en,               7,      1)
FLD_RAW_RW_DEF(         ext_dev_int_en_reg,         pd69200_int_en,               2,      1)
FLD_RAW_RW_DEF(         ext_dev_int_en_reg,         thm_int_en,                   1,      1)
FLD_RAW_RW_DEF(         ext_dev_int_en_reg,         fan_fail_int_en,              0,      1)
FLD_RAW_RW_DEF(         ext_dev_int_pending_reg,    _12v_pg_int,                  7,      1)
FLD_RAW_RW_DEF(         ext_dev_int_pending_reg,    pd69200_int,                  2,      1)
FLD_RAW_RW_DEF(         ext_dev_int_pending_reg,    thm_pend,                     1,      1)  /* thm_pend => tmh_int */
FLD_RAW_RW_DEF(         ext_dev_int_pending_reg,    fan_fail_int_pend,            0,      1)
FLD_RAW_RW_DEF(         phy_int_en_1_reg,           _88e1543_phy_6_int_en,        6,      1)
FLD_RAW_RW_DEF(         phy_int_en_1_reg,           _88e1680_phy_5_int_en,        5,      1)
FLD_RAW_RW_DEF(         phy_int_en_1_reg,           _88e1680_phy_4_int_en,        4,      1)
FLD_RAW_RW_DEF(         phy_int_en_1_reg,           _88e1680_phy_3_int_en,        3,      1)
FLD_RAW_RW_DEF(         phy_int_en_1_reg,           _88e1680_phy_2_int_en,        2,      1)
FLD_RAW_RW_DEF(         phy_int_en_1_reg,           _88e1680_phy_1_int_en,        1,      1)
FLD_RAW_RW_DEF(         phy_int_en_1_reg,           _88e1680_phy_0_int_en,        0,      1)
FLD_RAW_RW_DEF(         phy_int_en_2_reg,           _88e3220_phy_9_int_en,        2,      1)
FLD_RAW_RW_DEF(         phy_int_en_2_reg,           _88e2010_phy_8_int_en,        1,      1)
FLD_RAW_RW_DEF(         phy_int_en_2_reg,           _88e2010_phy_7_int_en,        0,      1)
FLD_RAW_RW_DEF(         port_int_en_reg,            sfp_plus_1_int_en,            3,      1)
FLD_RAW_RW_DEF(         port_int_en_reg,            sfp_plus_0_int_en,            2,      1)
FLD_RAW_RW_DEF(         port_int_en_reg,            sfp_1_int_en,                 1,      1)
FLD_RAW_RW_DEF(         port_int_en_reg,            sfp_0_int_en,                 0,      1)
FLD_RAW_RW_DEF(         phy_int_1_reg,              _88e1543_phy_6_int,           6,      1)
FLD_RAW_RW_DEF(         phy_int_1_reg,              _88e1680_phy_5_int,           5,      1)
FLD_RAW_RW_DEF(         phy_int_1_reg,              _88e1680_phy_4_int,           4,      1)
FLD_RAW_RW_DEF(         phy_int_1_reg,              _88e1680_phy_3_int,           3,      1)
FLD_RAW_RW_DEF(         phy_int_1_reg,              _88e1680_phy_2_int,           2,      1)
FLD_RAW_RW_DEF(         phy_int_1_reg,              _88e1680_phy_1_int,           1,      1)
FLD_RAW_RW_DEF(         phy_int_1_reg,              _88e1680_phy_0_int,           0,      1)
FLD_RAW_RW_DEF(         phy_int_2_reg,              _88e3220_phy_9_int,           2,      1)
FLD_RAW_RW_DEF(         phy_int_2_reg,              _88e2010_phy_8_int,           1,      1)
FLD_RAW_RW_DEF(         phy_int_2_reg,              _88e2010_phy_7_int,           0,      1)
FLD_RAW_RW_DEF(         port_int_pending_reg,       sfp_plus_1_int_pending,       3,      1)
FLD_RAW_RW_DEF(         port_int_pending_reg,       sfp_plus_0_int_pending,       2,      1)
FLD_RAW_RW_DEF(         port_int_pending_reg,       sfp_1_int_pending,            1,      1)
FLD_RAW_RW_DEF(         port_int_pending_reg,       sfp_0_int_pending,            0,      1)
FLD_RAW_RW_DEF(         tx_fault_rx_los_int_en_reg, sfp_plus_1_tx_fault_int_en,   7,      1)
FLD_RAW_RW_DEF(         tx_fault_rx_los_int_en_reg, sfp_plus_0_tx_fault_int_en,   6,      1)
FLD_RAW_RW_DEF(         tx_fault_rx_los_int_en_reg, sfp_1_tx_fault_int_en,        5,      1)
FLD_RAW_RW_DEF(         tx_fault_rx_los_int_en_reg, sfp_0_tx_fault_int_en,        4,      1)
FLD_RAW_RW_DEF(         tx_fault_rx_los_int_en_reg, sfp_plus_1_rx_los_int_en,     3,      1)
FLD_RAW_RW_DEF(         tx_fault_rx_los_int_en_reg, sfp_plus_0_rx_los_int_en,     2,      1)
FLD_RAW_RW_DEF(         tx_fault_rx_los_int_en_reg, sfp_1_rx_los_int_en,          1,      1)
FLD_RAW_RW_DEF(         tx_fault_rx_los_int_en_reg, sfp_0_rx_los_int_en,          0,      1)
FLD_RAW_RW_DEF(         tx_fault_rx_los_int_pend_reg, sfp_plus_1_tx_fault_int_pend, 7,      1)
FLD_RAW_RW_DEF(         tx_fault_rx_los_int_pend_reg, sfp_plus_0_tx_fault_int_pend, 6,      1)
FLD_RAW_RW_DEF(         tx_fault_rx_los_int_pend_reg, sfp_1_tx_fault_int_pend,    5,      1)
FLD_RAW_RW_DEF(         tx_fault_rx_los_int_pend_reg, sfp_0_tx_fault_int_pend,    4,      1)
FLD_RAW_RW_DEF(         tx_fault_rx_los_int_pend_reg, sfp_plus_1_rx_los_pend,     3,      1)
FLD_RAW_RW_DEF(         tx_fault_rx_los_int_pend_reg, sfp_plus_0_rx_los_pend,     2,      1)
FLD_RAW_RW_DEF(         tx_fault_rx_los_int_pend_reg, sfp_1_rx_los_pend,          1,      1)
FLD_RAW_RW_DEF(         tx_fault_rx_los_int_pend_reg, sfp_0_rx_los_pend,          0,      1)
FLD_RAW_RW_DEF(         reset_system_reg,           system_reset_n,               0,      1)
FLD_RAW_RW_DEF(         reset_system_reg,           _9548_rst,                    1,      1)
FLD_RAW_RW_DEF(         ports_led_type_reg,         sfp_plus_2_stack_up,          1,      1)
FLD_RAW_RW_DEF(         ports_led_type_reg,         sfp_plus_3_stack_up,          0,      1)
FLD_RAW_RO_DEF(         fan_status_reg,             fan2_fail,                    5,      1)
FLD_RAW_RO_DEF(         fan_status_reg,             fan1_fail,                    4,      1)
FLD_RAW_RW_DEF(         system_fan_1_contrl_reg,    sys_fan_1_pwm,                0,      5)
FLD_RAW_RO_DEF(         system_fan_1_status_reg,    sys_fan_1_status,             0,      8)  /* fan rpm ? */
FLD_RAW_RO_DEF(         system_fan_2_status_reg,    sys_fan_2_status,             0,      8)  /* fan rpm ? */
FLD_RAW_RW_DEF(         usb_control_status_reg,     usb_ovc_en,                   7,      1)
FLD_RAW_RW_DEF(         usb_control_status_reg,     thm_alert_rst,                4,      1)
FLD_RAW_RW_DEF(         usb_control_status_reg,     usb_pw_en,                    1,      1)
FLD_RAW_RO_DEF(         usb_control_status_reg,     usb_ovc,                      0,      1)
FLD_RAW_RW_DEF(         cpld_programming_reg,       cpld_programming_en,          7,      1)
FLD_RAW_RW_DEF(         led_power_ctrl_reg,         sys_led_pwr_ctrl_en,          0,      1)
FLD_RAW_RW_DEF(         led_power_ctrl_reg,         sfp_led_pwr_ctrl_en,          1,      1)
FLD_RAW_RO_DEF(         product_family_reg,         product_family,               0,      8)
FLD_RAW_RO_DEF(         product_model_reg,          product_model,                0,      8)


/* ---------------------- Definitions of CPLD specific fields --------------------------- */
/* CPLD specific read functions */
static int system_cpld_ver_raw_read(struct device *dev, struct device_attribute *attr, char *buf,
    int reg_offset, char *fld_name);

#define FLD_VER_RAW_RO_DEF(_reg, _fld) static ssize_t \
system_cpld_##_fld##_raw_read(struct device *dev, struct device_attribute *attr, char *buf) { \
    return system_cpld_ver_raw_read(dev, attr, buf, _reg##_offset, #_fld); \
}

/* CPLD register definition macros */
#define REG_DEF_SPEC(_reg, _off) \
static unsigned int _reg##_offset = (unsigned int)(_off);

/* Declare CPLD specific registers */
/*                register name                           offset  */
/*                --------------------------------------- ------- */
REG_DEF_SPEC(     date_code_reg,                          0xfe)   /* date_code includes register 0xfe and 0xff */

/* Declare CPLD specific register's fields */
/*                          register name               field name       */
/*                          ----------------------      ---------------- */
FLD_VER_RAW_RO_DEF(         date_code_reg,                 date_code)

#endif /* __wncSysCpldReg_h__ */
