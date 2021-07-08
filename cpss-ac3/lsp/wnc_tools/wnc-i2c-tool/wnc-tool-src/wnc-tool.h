#define tool_version "v0.0.1"
#define tool_name "wnc_i2c"

#define I2C_BUS_NAME "mv64xxx_i2c"

#define AC3_I2C_ADAPTOR 0
#define AC3_SFP_EE_ADDR   0x50

/*Loopback Module*/
#define LBM_PCA9554_ADDR 0x38
#define LBM_PCA9554_IN_REG 0x00
#define LBM_PCA9554_OUT_REG 0x01
#define LBM_PCA9554_CFG_REG 0x03

#define AIKIDO_I2C_MUX_ADDR 0x0c
#define AIKIDO_SFP_PRESENT_ADDR 0x14
#define AIKIDO_SFP_TX_DIS_ADDR 0x10
#define AIKIDO_SFP_RX_LOS_ADDR 0x18

#define CABLE_MODEL_START 0
#define CABLE_MODEL_SIZE 64
#define CABLE_SERIAL_START 64
#define CABLE_SERIAL_SIZE 32

/*#define AIKIDO_MTD "/dev/mtd3"*/

/* Return the mask including "numOfBits" bits. for 0..31 bits   */
#define BIT_MASK_0_31_MAC(numOfBits) (~(0xFFFFFFFF << (numOfBits)))

/* the macro of BIT_MASK_0_31_MAC() in VC will act in wrong way for 32 bits , and will
   result 0 instead of 0xffffffff

   so macro BIT_MASK_MAC - is improvement of BIT_MASK_0_31_MAC to support 32 bits
*/
#define BIT_MASK_MAC(numOfBits)    ((numOfBits) == 32 ? 0xFFFFFFFF : BIT_MASK_0_31_MAC(numOfBits))

/* Calculate the field mask for a given offset & length */
/* e.g.: BIT_MASK_MAC(8,2) = 0xFFFFFCFF                     */
#define FIELD_MASK_NOT_MAC(offset,len)                      \
        (~(BIT_MASK_MAC((len)) << (offset)))

/* Calculate the field mask for a given offset & length */
/* e.g.: BIT_MASK_MAC(8,2) = 0x00000300                     */
#define FIELD_MASK_MAC(offset,len)                      \
        ( (BIT_MASK_MAC((len)) << (offset)) )

/* Returns the info located at the specified offset & length in data.   */
#define U32_GET_FIELD_MAC(data,offset,length)           \
        (((data) >> (offset)) & BIT_MASK_MAC(length))

/* Sets the field located at the specified offset & length in data.     */
#define U32_SET_FIELD_MAC(data,offset,length,val)           \
   (data) = (((data) & FIELD_MASK_NOT_MAC((offset),(length))) | ((val) <<(offset)))

/* for setting data of GT_U16.
   Sets the field located at the specified offset & length in data.     */
#define U16_SET_FIELD_MAC(data,offset,length,val)           \
   (data) = (GT_U16)(((data) & FIELD_MASK_NOT_MAC((offset),(length))) | ((val) <<(offset)))

/* Sets the field located at the specified offset & length in data.
    the field may be with value with more bits then length */
#define U32_SET_FIELD_MASKED_MAC(data,offset,length,val)           \
   (U32_SET_FIELD_MAC((data),(offset),(length),((val) & BIT_MASK_MAC(length))))

/* Error messages */
#define errmsg(fmt, ...)  ({                                                \
	fprintf(stderr, "%s: error!: " fmt "\n", tool_name, ##__VA_ARGS__); \
	-1;                                                                 \
})
#define errmsg_die(fmt, ...) do {                                           \
	exit(errmsg(fmt, ##__VA_ARGS__));                                   \
} while(0)

/* System error messages */
#define sys_errmsg(fmt, ...)  ({                                            \
	int _err = errno;                                                   \
	errmsg(fmt, ##__VA_ARGS__);                                         \
	fprintf(stderr, "%*serror %d (%s)\n", (int)sizeof(tool_name) + 1,\
		"", _err, strerror(_err));                                  \
	-1;                                                                 \
})

#define sys_errmsg_die(fmt, ...) do {                                       \
	exit(sys_errmsg(fmt, ##__VA_ARGS__));                               \
} while(0)

void show_usage(void);
void read_port_status(int port);
int ac3_aikido_set_i2c_mux(int port);
int ac3_aikido_get_sfp_present(int port);
void ac3_aikido_clear_i2c_mux(void);
void sfp_io_test(int port);
void set_sfp_loading(int loading);

