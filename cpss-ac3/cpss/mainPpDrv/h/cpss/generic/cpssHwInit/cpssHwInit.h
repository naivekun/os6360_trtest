/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file mainPpDrv/h/cpss/generic/cpssHwInit/cpssHwInit.h
*
* @brief Includes CPSS basic Hw initialization functions, and data structures.
*
* @version   38
********************************************************************************
*/
#ifndef __cpssHwInith
#define __cpssHwInith

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssHwInit/cpssHwInit.h>
#include <cpss/generic/extMac/cpssExtMacDrv.h>

/* Defines the max number SRAMs. The number includes CTRL SRAM (wide SRAM) and
   routing SRAM (narrow SRAM)*/
#define CPSS_MAX_NUMBER_OF_EXTERNAL_SRAMS_CNS 5

/* Defines the max number internal SRAMs */
#define CPSS_MAX_NUMBER_LPM_INTERNAL_SRAMS_CNS 4

/**
* @enum CPSS_MAC_SA_LSB_MODE_ENT
 *
 * @brief Enumeration of MAC SA lsb modes ,
 * this relate to "pause packets" and to "routed packets".
*/
typedef enum{

    /** @brief Sets the MAC SA least significant byte
     *  according to the per port setting.
     *  (APPLICABLE DEVICES: DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Puma2; Puma3; ExMx.)
     *  (NOT APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
     */
    CPSS_SA_LSB_PER_PORT_E,

    /** @brief Sets the MAC SA least significant 12-bits
     *  according to the packet's VLAN ID.
     *  (NOT APPLICABLE DEVICES: Falcon)
     */
    CPSS_SA_LSB_PER_PKT_VID_E,

    /** @brief Sets the MAC SA least significant byte
     *  according to the per vlan setting.
     */
    CPSS_SA_LSB_PER_VLAN_E,

    /** @brief Global MAC SA: Sets full 48-bit MAC SA
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
     */
    CPSS_SA_LSB_FULL_48_BIT_GLOBAL

} CPSS_MAC_SA_LSB_MODE_ENT;

/**
* @enum CPSS_SRAM_SIZE_ENT
 *
 * @brief Defines the external SRAM possible sizes.
*/
typedef enum{

    /** SRAM is not connected */
    CPSS_SRAM_SIZE_NOT_USED_E     = 0x0,

    /** SRAM size is 12 KByte */
    CPSS_SRAM_SIZE_12KB_E         = 12 * _1KB,

    /** SRAM size is 32 KByte */
    CPSS_SRAM_SIZE_32KB_E         = _32KB,

    /** SRAM size is 64 KByte */
    CPSS_SRAM_SIZE_64KB_E         = _64KB,

    /** SRAM size is 128 KByte */
    CPSS_SRAM_SIZE_128KB_E        = _128KB,

    /** SRAM size is 256 KByte */
    CPSS_SRAM_SIZE_256KB_E        = _256KB,

    /** SRAM size is 512 KByte */
    CPSS_SRAM_SIZE_512KB_E        = _512KB,

    /** SRAM size is 1 Mbyte */
    CPSS_SRAM_SIZE_1MB_E          = _1MB,

    /** SRAM size is 2 Mbyte */
    CPSS_SRAM_SIZE_2MB_E          = _2MB,

    /** SRAM size is 4 Mbyte */
    CPSS_SRAM_SIZE_4MB_E          = _4MB,

    /** SRAM size is 8 Mbyte */
    CPSS_SRAM_SIZE_8MB_E          = _8MB,

    /** SRAM size is 16 Mbyte */
    CPSS_SRAM_SIZE_16MB_E         = _16MB,

    /** SRAM size is 32 Mbyte */
    CPSS_SRAM_SIZE_32MB_E         = _32MB,

    /** SRAM size is 64 Mbyte */
    CPSS_SRAM_SIZE_64MB_E         = _64MB,

    /** SRAM size is 128 Mbyte */
    CPSS_SRAM_SIZE_128MB_E        = _128MB,

    /** SRAM size is 256 Mbyte */
    CPSS_SRAM_SIZE_256MB_E        = _256MB

} CPSS_SRAM_SIZE_ENT;

/**
* @enum CPSS_SRAM_RATE_ENT
 *
 * @brief Defines the rate of the external SRAM.
*/
typedef enum{

    /** Double data rate */
    CPSS_SRAM_RATE_DDR_E =0,

    /** Quadric data rate */
    CPSS_SRAM_RATE_QDR_E =1

} CPSS_SRAM_RATE_ENT;

/**
* @enum CPSS_SRAM_TYPE_ENT
 *
 * @brief Defines the type of the external SRAM (narrow or wide).
*/
typedef enum{

    /** Narrow SRAM */
    CPSS_SRAM_TYPE_NARROW_E,

    /** Wide SRAM */
    CPSS_SRAM_TYPE_WIDE_E

} CPSS_SRAM_TYPE_ENT;

/**
* @enum CPSS_EXTERNAL_MEMORY_ENT
 *
 * @brief Defines which external memory is it.
*/
typedef enum{

    /** external memory 0 */
    CPSS_EXTERNAL_MEMORY_0_E = 0,

    /** external memory 1 */
    CPSS_EXTERNAL_MEMORY_1_E = 1,

    /** external memory 2 */
    CPSS_EXTERNAL_MEMORY_2_E = 2,

    /** external memory 3 */
    CPSS_EXTERNAL_MEMORY_3_E = 3,

    /** Not connected */
    CPSS_EXTERNAL_MEMORY_INVALID_E = 0x7FFFFFFF

} CPSS_EXTERNAL_MEMORY_ENT;


/**
* @enum CPSS_NARROW_SRAM_CFG_ENT
 *
 * @brief Defines the memory configuration of the narrow SRAM.
*/
typedef enum{

    /** @brief Internal memory for TWIST type asic - backward
     *  compatibility
     */
    CPSS_TWIST_INTERNAL_E,

    /** @brief Only external memory for TWIST type asic -
     *  backward compatibility
     */
    CPSS_TWIST_EXTERNAL_E,

    /** @brief Two external memories that work in 250 MHz and
     *  therefore in order to support wire speed the
     *  internal memories are used as well.
     */
    CPSS_TWO_EXTERNAL_250MHZ_E,

    /** @brief Two external narrow srams each working in 300MHz
     *  It supports IPv4 and IPv6 routing in full wire
     *  speed.
     */
    CPSS_TWO_EXTERNAL_300MHZ_E,

    /** @brief Support taking the Tiger as a drop in to TwistD
     *  boards. With emphasis on capacity (more IPv4 mc
     *  groups, less wire speed)
     */
    CPSS_DROP_IN_CAPACITY_E,

    /** @brief Support taking the Tiger as a drop in to TwistD
     *  boards. With emphasis on speed (more wire speed,
     *  less IPv4 mc groups)
     */
    CPSS_DROP_IN_SPEED_E,

    /** @brief Two internal narrow srams 32Kbyte each working
     *  300 MHz. It supports IPv4 and IPv6 routing in full
     *  wire speed.
     */
    CPSS_TWO_INTERNAL_E,

    /** @brief One 64 Kbyte internal narrow sram for IPv4 only
     *  backward compatible mode.
     */
    CPSS_ONE_INTERNAL_E,

    /** No nram configuration */
    CPSS_NRAM_NOT_EXISTS_E

} CPSS_NARROW_SRAM_CFG_ENT;


/**
* @enum CPSS_SRAM_FREQUENCY_ENT
 *
 * @brief Defines the different Sram frequencies that an sram can have.
*/
typedef enum{

    /** 125 MHz sram frequency. */
    CPSS_SRAM_FREQ_125_MHZ_E = 125,

    /** 166 MHz sram frequency. */
    CPSS_SRAM_FREQ_166_MHZ_E = 166,

    /** 200 MHz sram frequency. */
    CPSS_SRAM_FREQ_200_MHZ_E = 200,

    /** 250 MHz sram frequency. */
    CPSS_SRAM_FREQ_250_MHZ_E = 250,

    /** 300 MHz sram frequency. */
    CPSS_SRAM_FREQ_300_MHZ_E = 300

} CPSS_SRAM_FREQUENCY_ENT;



/**
* @struct CPSS_SRAM_CFG_STC
 *
 * @brief Includes external Sram configuration values
 * This struct is used for External Wide & Narrow Srams.
*/
typedef struct{

    /** The external Sram size. */
    CPSS_SRAM_SIZE_ENT sramSize;

    /** Holds the Sram clock frequency. */
    CPSS_SRAM_FREQUENCY_ENT sramFrequency;

    /** This field indicates the rate of external memory. */
    CPSS_SRAM_RATE_ENT sramRate;

    /** @brief Indicates to which external memory the memory
     *  parameters are related.
     */
    CPSS_EXTERNAL_MEMORY_ENT externalMemory;

    /** Determines if it's narrow SRAM or wide SRAM. */
    CPSS_SRAM_TYPE_ENT sramType;

} CPSS_SRAM_CFG_STC;



/**
* @struct CPSS_BUFF_MGMT_CFG_STC
 *
 * @brief Includes common buffer management configuration values, to be
 * set to Pp's registers at initialization.
*/
typedef struct{

    /** @brief when required, indicates to overwrite the PP's default
     *  values.
     *  GT_TRUE - overwrite,
     *  GT_FALSE - ignore the rest of this struct's data.
     */
    GT_BOOL overwriteDef;

    /** @brief The number of buffers allocated in the PP Buffer
     *  DRAM. This value must not exceed the maximal allowed
     *  number of buffers (depending on the device type).
     */
    GT_U16 numPpBuffers;

    /** @brief The total number of buffers used by the uplink
     *  device
     *  Comments:
     *  - Refer to the data-sheet for complete description of these parameters.
     *  - Setting an undefined values, like reserved values, may result in
     *  unpredictable results.
     */
    GT_U16 numUplinkBuffers;

} CPSS_BUFF_MGMT_CFG_STC;


/**
* @enum CPSS_BUFFER_DRAM_PARAM_ENT
 *
 * @brief Type of buffer DRAM.
*/
typedef enum{

    /** Use default DRAM configuration. */
    CPSS_BUFFER_DRAM_USE_DEFAULT_DRAM_CONF_E = 0,

    /** SAMSUNG AVer (K4D28163HDTC40) 250 Mhz */
    CPSS_BUFFER_DRAM_SAMSUNG_REV_A_250MHZ_E = 1,

    /** SAMSUNG EVer (K4D261638ETC40) 250 Mhz */
    CPSS_BUFFER_DRAM_SAMSUNG_REV_E_250MHZ_E,

    /** Hynix HY5DU283222F33 250 Mhz */
    CPSS_BUFFER_DRAM_HYNIX_HY5DU283222F_33_250MHZ_E,

    /** SAMSUNG AVer (K4D28163HDTC40) 300 Mhz */
    CPSS_BUFFER_DRAM_SAMSUNG_REV_A_300MHZ_E,

    /** SAMSUNG EVer (K4D261638ETC40) 300 Mhz */
    CPSS_BUFFER_DRAM_SAMSUNG_REV_E_300MHZ_E

} CPSS_BUFFER_DRAM_PARAM_ENT;

/**
* @enum CPSS_FLOW_DRAM_PARAM_ENT
 *
 * @brief Type of flow DRAM.
*/
typedef enum{

    /** Use default DRAM configuration. */
    CPSS_FLOW_DRAM_USE_DEFAULT_DRAM_CONF_E = 0,

    /** SAMSUNG A-Ver (K4D28163HD-TC40) 250 Mhz */
    CPSS_FLOW_DRAM_SAMSUNG_REV_A_250MHZ_E = 1,

    /** SAMSUNG E-Ver (K4D261638E-TC40) 250 Mhz */
    CPSS_FLOW_DRAM_SAMSUNG_REV_E_250MHZ_E,

    /** SAMSUNG A-Ver (K4D28163HD-TC40) 166 Mhz */
    CPSS_FLOW_DRAM_SAMSUNG_REV_A_166MHZ_E,

    /** SAMSUNG E-Ver (K4D261638E-TC40) 166 Mhz */
    CPSS_FLOW_DRAM_SAMSUNG_REV_E_166MHZ_E,

    /** SAMSUNG A-Ver (K4D28163HD-TC40) 200 Mhz */
    CPSS_FLOW_DRAM_SAMSUNG_REV_A_200MHZ_E,

    /** SAMSUNG E-Ver (K4D261638E-TC40) 200 Mhz */
    CPSS_FLOW_DRAM_SAMSUNG_REV_E_200MHZ_E

} CPSS_FLOW_DRAM_PARAM_ENT;


/* Delimiter for registers list regs */
#define CPSS_REG_LIST_DELIMITER_CNS         0xFFFFFFFF

/* Delimiter for phy addr regs */
#define CPSS_REG_PHY_ADDR_TYPE_CNS          0xEEEEEEEE

/**
* @struct CPSS_REG_VALUE_INFO_STC
 *
 * @brief Holds a specific register value to be used for configuring a
 * device's register in system.
 * This structure is a part of a values' list to be
 * written into the device in the initialization sequence.
*/
typedef struct{

    /** Holds the register address to write to. */
    GT_U32 addrType;

    /** @brief Holds the valid bits out of regVal to be written to the
     *  register, value of 0xFFFFFFFF will write to the whole
     *  register.
     */
    GT_U32 regMask;

    /** @brief Holds the register value to write (after a bit wise & with
     *  regMask).
     */
    GT_U32 regVal;

    /** @brief Number of repeatitions this register writes. (Useful
     *  for DFCDL's values configuration).
     */
    GT_U32 repeatCount;

} CPSS_REG_VALUE_INFO_STC;

/**
* @enum CPSS_PP_UPLINK_CONFIG_ENT
 *
 * @brief Enumeration of the different uplink devices.
*/
typedef enum{

    /** Uplink connected to another PP. */
    CPSS_PP_UPLINK_BACK_TO_BACK_E = 0,

    /** Uplink connected to FA device. */
    CPSS_PP_UPLINK_FA_E ,

    /** The Uplink isn't connected. */
    CPSS_PP_NO_UPLINK_E

} CPSS_PP_UPLINK_CONFIG_ENT;


/**
* @enum CPSS_AU_MESSAGE_LENGTH_ENT
 *
 * @brief packet buffer sizes
*/
typedef enum{

    /** AU message is 16 byte length */
    CPSS_AU_MESSAGE_LENGTH_4_WORDS_E  = 0,

    /** AU message is 32 byte length */
    CPSS_AU_MESSAGE_LENGTH_8_WORDS_E

} CPSS_AU_MESSAGE_LENGTH_ENT;

/**
* @enum CPSS_BUF_MODE_ENT
 *
 * @brief packet buffer sizes
*/
typedef enum{

    /** 1536 byte buffers */
    CPSS_BUF_1536_E  = 0,

    /** 2K byte buffers */
    CPSS_BUF_2K_E,

    /** 10K byte buffers */
    CPSS_BUF_10K_E

} CPSS_BUF_MODE_ENT;

/**
* @enum CPSS_BANK_MODE_ENT
 *
 * @brief packet bank modes
*/
typedef enum{

    /** no smart bank */
    CPSS_NO_SMART_BANK_E = 0,

    /** partial smart bank */
    CPSS_PARTIAL_SMART_BANK_E,

    /** regular smart bank */
    CPSS_REGULAR_SMART_BANK_E

} CPSS_BANK_MODE_ENT;


/**
* @enum CPSS_DRAM_SIZE_ENT
 *
 * @brief Packet Processor DRAM sizes
*/
typedef enum{

    /** External DRAM is not connected. */
    CPSS_DRAM_NOT_USED_E   = 0x0,

    /** DRAM size is 64KByte. */
    CPSS_DRAM_64KB_E       = _64KB,

    /** DRAM size is 128KByte. */
    CPSS_DRAM_128KB_E      = _128KB,

    /** DRAM size is 8 MByte. */
    CPSS_DRAM_8MB_E        = _8MB,

    /** DRAM size is 16 MByte. */
    CPSS_DRAM_16MB_E       = _16MB,

    /** DRAM size is 32 MByte. */
    CPSS_DRAM_32MB_E       = _32MB,

    /** DRAM size is 64 MByte. */
    CPSS_DRAM_64MB_E       = _64MB,

    /** DRAM size is 128 MByte. */
    CPSS_DRAM_128MB_E      = _128MB,

    /** DRAM size is 256 MByte. */
    CPSS_DRAM_256MB_E      = _256MB,

    /** DRAM size is 512 MByte. */
    CPSS_DRAM_512MB_E      = _512MB,

    /** @brief DRAM size is 1 GByte.
     *  Notes: Some Prestera devices do not have external memory interfaces
     */
    CPSS_DRAM_1GB_E        = _1GB

} CPSS_DRAM_SIZE_ENT;



/**
* @enum CPSS_DRAM_FREQUENCY_ENT
 *
 * @brief Defines the different DRAM frequencies that a DRAM can have.
*/
typedef enum{

    /** 100 MHz dram frequency. */
    CPSS_DRAM_FREQ_100_MHZ_E = 100,

    /** 125 MHz DRAM frequency. */
    CPSS_DRAM_FREQ_125_MHZ_E = 125,

    /** 166 MHz DRAM frequency. */
    CPSS_DRAM_FREQ_166_MHZ_E = 166,

    /** 200 MHz DRAM frequency. */
    CPSS_DRAM_FREQ_200_MHZ_E = 200,

    /** 250 MHz DRAM frequency. */
    CPSS_DRAM_FREQ_250_MHZ_E = 250,

    /** 300 MHz DRAM frequency. */
    CPSS_DRAM_FREQ_300_MHZ_E = 300,

    /** 667 MHz DRAM frequency. */
    CPSS_DRAM_FREQ_667_MHZ_E = 667,

    /** 800 MHz DRAM frequency. */
    CPSS_DRAM_FREQ_800_MHZ_E = 800,

    /** 933 MHz DRAM frequency. */
    CPSS_DRAM_FREQ_933_MHZ_E = 933,

    /** 1066 MHz dram frequency. */
    CPSS_DRAM_FREQ_1066_MHZ_E = 1066

} CPSS_DRAM_FREQUENCY_ENT;

/**
* @enum CPSS_DRAM_PAGE_SIZE_ENT
 *
 * @brief memory page size in the external DRAM
*/
typedef enum{

    /** page size 512 Bytes */
    CPSS_DRAM_PAGE_SIZE_512B_E = 512,

    /** page size 1K Bytes */
    CPSS_DRAM_PAGE_SIZE_1K_E   = 1024,

    /** page size 2K Bytes */
    CPSS_DRAM_PAGE_SIZE_2K_E   = 2048,

    /** page size 4K Bytes */
    CPSS_DRAM_PAGE_SIZE_4K_E   = 4096

} CPSS_DRAM_PAGE_SIZE_ENT;

/**
* @enum CPSS_DRAM_BUS_WIDTH_ENT
 *
 * @brief Defines the DRAM interface bus width in bits.
*/
typedef enum{

    /** 4 bits bus width. */
    CPSS_DRAM_BUS_WIDTH_4_E,

    /** 8 bits bus width. */
    CPSS_DRAM_BUS_WIDTH_8_E,

    /** 16 bits bus width. */
    CPSS_DRAM_BUS_WIDTH_16_E,

    /** 32 bits bus width. */
    CPSS_DRAM_BUS_WIDTH_32_E

} CPSS_DRAM_BUS_WIDTH_ENT;

/**
* @enum CPSS_DRAM_TEMPERATURE_ENT
 *
 * @brief Defines the DRAM operation temperature
 * (refresh interval related).
*/
typedef enum{

    /** Up to 85C operating temperature. */
    CPSS_DRAM_TEMPERATURE_NORMAL_E,

    /** Operating temperature can exceed 85C. */
    CPSS_DRAM_TEMPERATURE_HIGH_E

} CPSS_DRAM_TEMPERATURE_ENT;

/**
* @enum CPSS_DRAM_SPEED_BIN_ENT
 *
 * @brief Defines DRAM speed bining (JEDEC standard name).
*/
typedef enum{

    /** DDR3-800D JEDEC standard name. */
    CPSS_DRAM_SPEED_BIN_DDR3_800D_E,

    /** DDR3-800E JEDEC standard name. */
    CPSS_DRAM_SPEED_BIN_DDR3_800E_E,

    /** DDR3-1066E JEDEC standard name. */
    CPSS_DRAM_SPEED_BIN_DDR3_1066E_E,

    /** DDR3-1066F JEDEC standard name. */
    CPSS_DRAM_SPEED_BIN_DDR3_1066F_E,

    /** DDR3-1066G JEDEC standard name. */
    CPSS_DRAM_SPEED_BIN_DDR3_1066G_E,

    /** DDR3-1333F JEDEC standard name. */
    CPSS_DRAM_SPEED_BIN_DDR3_1333F_E,

    /** DDR3-1333G JEDEC standard name. */
    CPSS_DRAM_SPEED_BIN_DDR3_1333G_E,

    /** DDR3-1333H JEDEC standard name. */
    CPSS_DRAM_SPEED_BIN_DDR3_1333H_E,

    /** DDR3-1333J JEDEC standard name. */
    CPSS_DRAM_SPEED_BIN_DDR3_1333J_E,

    /** DDR3-1600G JEDEC standard name. */
    CPSS_DRAM_SPEED_BIN_DDR3_1600G_E,

    /** DDR3-1600H JEDEC standard name. */
    CPSS_DRAM_SPEED_BIN_DDR3_1600H_E,

    /** DDR3-1600J JEDEC standard name. */
    CPSS_DRAM_SPEED_BIN_DDR3_1600J_E,

    /** DDR3-1600K JEDEC standard name. */
    CPSS_DRAM_SPEED_BIN_DDR3_1600K_E,

    /** DDR3-1866J JEDEC standard name. */
    CPSS_DRAM_SPEED_BIN_DDR3_1866J_E,

    /** DDR3-1866K JEDEC standard name. */
    CPSS_DRAM_SPEED_BIN_DDR3_1866K_E,

    /** DDR3-1866L JEDEC standard name. */
    CPSS_DRAM_SPEED_BIN_DDR3_1866L_E,

    /** DDR3-1866M JEDEC standard name. */
    CPSS_DRAM_SPEED_BIN_DDR3_1866M_E,

    /** DDR3-2133K JEDEC standard name. */
    CPSS_DRAM_SPEED_BIN_DDR3_2133K_E,

    /** DDR3-2133L JEDEC standard name. */
    CPSS_DRAM_SPEED_BIN_DDR3_2133L_E,

    /** DDR3-2133M JEDEC standard name. */
    CPSS_DRAM_SPEED_BIN_DDR3_2133M_E,

    /** DDR3-2133N JEDEC standard name. */
    CPSS_DRAM_SPEED_BIN_DDR3_2133N_E

} CPSS_DRAM_SPEED_BIN_ENT;

/**
* @struct CPSS_DRAM_CFG_STC
 *
 * @brief Includes external DRAM configuration values.
 * This structure is used for Flow & Buffers DRAMs.
*/
typedef struct
{
    CPSS_DRAM_SIZE_ENT      dramSize;
    CPSS_DRAM_FREQUENCY_ENT dramFrequency;
    GT_BOOL                 overwriteDef;
    GT_U16                  mrs;
    GT_U16                  emrs;
    struct
    {
        GT_U8   tdqss;
        GT_U8   trcd;
        GT_U8   trp;
        GT_U8   twr;
        GT_U8   twtr;
        GT_U8   tras;
        GT_U8   trrd;
        GT_U8   tmrd;
        GT_U8   trfc;
        GT_U8   rd2rdDelay;
        GT_U8   rd2wrDelay;
        GT_U8   writeCount;
        GT_U8   readCount;
    }                       dramTiming;
    GT_U16                  refreshCounter;
    GT_U8                   trCdWr;
    GT_U8                   operation;
    CPSS_DRAM_PAGE_SIZE_ENT dramPageSize;
}CPSS_DRAM_CFG_STC;

/**
* @struct CPSS_RLDRAM_CFG_STC
 *
 * @brief This struct defines specific hardware parameters relevant when RLDRAM is used
 * for the Buffer Memory.
*/
typedef struct
{
    CPSS_DRAM_SIZE_ENT  dramSize;
    GT_BOOL             overwriteDef;
    GT_U32              mrs;
    GT_U32              refreshLimit;
    struct
    {
        GT_U32 tWL;
        GT_U32 tRL;
        GT_U32 tRC;
        GT_U32 tMPSC;
    }dramTiming;
    GT_U32 pwrUpDelay;
    GT_U32 pwrUpRefDelay;
}
CPSS_RLDRAM_CFG_STC;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* cpssHwInit */


