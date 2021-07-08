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
* @file cpssDrvPpConInit.c
*
* @brief Low level driver initialization of PPs, and declarations of global
* variables
*
* @version   79
********************************************************************************
*/

#if (defined PX_FAMILY)
    #define ALLOW_PX_CODE
#endif

#include <cpssDriver/pp/interrupts/generic/prvCpssDrvInterrupts.h>
#include <cpssDriver/pp/config/prvCpssDrvPpCfgInit.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpssDriver/pp/hardware/prvCpssDrvHwCntl.h>
#include <cpssDriver/pp/hardware/prvCpssDrvObj.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvDxExMxInterrupts.h>
#include <cpssDriver/pp/hardware/prvCpssDrvHwPpPortGroupCntl.h>

extern GT_U16   prvCpssDrvDebugDeviceId[PRV_CPSS_MAX_PP_DEVICES_CNS];

#ifdef ASIC_SIMULATION
    /* Simulation H files */
    #include <asicSimulation/SInit/sinit.h>
#endif /*ASIC_SIMULATION*/

CPSS_TBD_BOOKMARK_FALCON    /* need to find proper solution for the driver */
static CPSS_PP_FAMILY_TYPE_ENT nextDevFamily = CPSS_PP_FAMILY_LAST_E;
static GT_U32  mg0UnitBaseAddr = 0;
static GT_U32  dfxUnitBaseAddr = 0;
static GT_U32  allowMultiMgSdmaInit = 1;

/* address of SDMA configuration register addresses */
#define SDMA_CFG_REG_ADDR       (mg0UnitBaseAddr + 0x2800)

/* DX address of PCI Pad Calibration Configuration register addresses */
#define DX_PCI_PAD_CALIB_REG_ADDR       0x0108

/* DX address of PCI Status and Command Register addresses */
#define DX_PCI_STATUS_CMD_REG_ADDR       0x04

/* DX address of the vendor id register address */
#define VENDOR_ID_REG_ADDR           (mg0UnitBaseAddr + 0x50)

#define DEVICE_ID_AND_REV_REG_ADDR_CNS (mg0UnitBaseAddr + PRV_CPSS_DEVICE_ID_AND_REV_REG_ADDR_CNS)

/* DFX address of the vendor id register address */
#define DFX_DEVICE_ID_AND_REV_REG_ADDR_CNS      (dfxUnitBaseAddr + 0xF8240)

#define DFX_JTAG_DEVICE_ID_AND_REV_REG_ADDR_CNS (dfxUnitBaseAddr + 0xF8244)

/* PORT_GROUPS_INFO_STC - info about the port groups
        NOTE: assumption that all port groups of the device are the same !
    numOfPortGroups - number of port groups
    portsBmp        - the actual ports bmp of each port group
                        for example: Lion 0xFFF (12 ports)
    maxNumPorts      - number of ports in range
                        for example: Lion 16 ports (12 out of 16 in each port group)
*/
typedef struct {
    GT_U32  numOfPortGroups;
    GT_U32  portsBmp;
    GT_U32  maxNumPorts;
} PORT_GROUPS_INFO_STC;

#ifdef GM_USED
extern GT_STATUS prvCpssDrvPpIntDefDxChBobcat2Init__GM(GT_U8 devNum);
#endif

/* flag to state that running on emulator */
static GT_U32  cpssDeviceRun_onEmulator_active = 0;
/**
* @internal cpssDeviceRunCheck_onEmulator function
* @endinternal
*
* @brief   Check if running on EMULATOR.
*         (when running on EMULATOR) This function must be called before
*         'phase 1' init of the device.
*
* @note   APPLICABLE DEVICES:      All.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval 0                        - not running on EMULATOR.
* @retval 1                        - running on EMULATOR.
*/
GT_U32  cpssDeviceRunCheck_onEmulator(void)
{
    return cpssDeviceRun_onEmulator_active;
}
/**
* @internal cpssDeviceRunSet_onEmulator function
* @endinternal
*
* @brief   State that running on EMULATOR. (all devices considered to be running on
*         EMULATOR)
*         (when running on EMULATOR) This function must be called before
*         'phase 1' init of the device.
*
* @note   APPLICABLE DEVICES:      All.
* @note   NOT APPLICABLE DEVICES:  None.
*
*                                       None.
*/
void  cpssDeviceRunSet_onEmulator(void)
{
    /* NOTE: must be called before 'phase 1' init */
    cpssDeviceRun_onEmulator_active = 1;
}
extern GT_BOOL prvCpssDxChHwIsUnitUsed
(
    IN GT_U32                   devNum,
    IN GT_U32                   portGroupId,
    IN GT_U32                   regAddr
);
extern GT_BOOL prvCpssPxHwIsUnitUsed
(
    IN GT_U32                   devNum,
    IN GT_U32                   portGroupId,
    IN GT_U32                   regAddr
);

/* check if register address supported on emulator */
GT_BOOL prvCpssOnEmulatorSupportedAddrCheck(
    IN  GT_U8       devNum,
    IN  GT_U32      portGroupId,
    IN  GT_U32      regAddr
)
{
#ifdef CHX_FAMILY
    if(prvCpssDrvPpConfig[devNum] && CPSS_IS_DXCH_FAMILY_MAC(prvCpssDrvPpConfig[devNum]->devFamily))
    {
        if(GT_FALSE == prvCpssDxChHwIsUnitUsed(devNum,portGroupId,regAddr))
        {
            /* the memory/register is not in supported memory space */
            return GT_FALSE;
        }
    }
#endif /*CHX_FAMILY*/

#ifdef PX_FAMILY
    if(prvCpssDrvPpConfig[devNum] && CPSS_IS_PX_FAMILY_MAC(prvCpssDrvPpConfig[devNum]->devFamily))
    {
        if(GT_FALSE == prvCpssPxHwIsUnitUsed(devNum,portGroupId,regAddr))
        {
            /* the memory/register is not in supported memory space */
            return GT_FALSE;
        }
    }
#endif /*PX_FAMILY*/

    return GT_TRUE;
}

static GT_U32  onEmulator_isAldrinFull = 1;
/**
* @internal cpssDeviceRunCheck_onEmulator_isAldrinFull function
* @endinternal
*
* @brief   check if 'Aldrin' on Emulator is 'FULL' or missing DP[0,1] and GOPs[ports 0..23] and TCAM.
*
* @note   APPLICABLE DEVICES:      All.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval 0                        - Aldrin is not 'FULL' and missing: missing DP[0,1] and GOPs[ports 0..23] and TCAM.
* @retval 1                        - Aldrin is 'FULL'.
*/
GT_U32  cpssDeviceRunCheck_onEmulator_isAldrinFull(void)
{
    return onEmulator_isAldrinFull;
}
/**
* @internal cpssDeviceRunCheck_onEmulator_AldrinFull function
* @endinternal
*
* @brief   State that Aldrin running on EMULATOR as 'FULL' or not.
*         ('FULL' or missing DP[0,1] and GOPs[ports 0..23] and TCAM.)
*         (when running on EMULATOR) This function must be called before
*         'phase 1' init of the device.
*
* @note   APPLICABLE DEVICES:      All.
* @note   NOT APPLICABLE DEVICES:  None.
*
*                                       None.
*/
void  cpssDeviceRunCheck_onEmulator_AldrinFull(GT_U32 isFull)
{
    onEmulator_isAldrinFull = isFull;
}

#define PRV_CPSS_DXCH_FALCON_CIDER_VERSION__LATEST__CNS  0/* currently is 'March 2018' (default if not set) with other value */
static PRV_CPSS_DXCH_FALCON_CIDER_VERSION_ENT falconCiderVersion        = PRV_CPSS_DXCH_FALCON_CIDER_VERSION__LATEST__CNS;
static PRV_CPSS_DXCH_FALCON_CIDER_VERSION_ENT falconCiderVersion_latest = PRV_CPSS_DXCH_FALCON_CIDER_VERSION_18_07_09_E/*former18_04_22*/;
/**
* @internal prvCpssDxchFalconCiderVersionGet function
* @endinternal
*
* @brief   get the current Cider version that is used.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  All but Falcon.
*
* @return the version
*/
PRV_CPSS_DXCH_FALCON_CIDER_VERSION_ENT prvCpssDxchFalconCiderVersionGet(void)
{
    if(falconCiderVersion == PRV_CPSS_DXCH_FALCON_CIDER_VERSION__LATEST__CNS)
    {
        return falconCiderVersion_latest;
    }

    return  falconCiderVersion;
}

/**
* @internal prvCpssDxchFalconCiderVersionGet function
* @endinternal
*
* @brief   get the current Cider version that is used.
*
*   version
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  All but Falcon.
*
* @param[in] version  - the Cider version
*
* @return the version
*/
void prvCpssDxchFalconCiderVersionSet(
    IN PRV_CPSS_DXCH_FALCON_CIDER_VERSION_ENT   version
)
{
    falconCiderVersion = version;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
#define BMP_CONTINUES_PORTS_MAC(x)  ((1<<(x)) - 1)

/* macro to set range of ports in bmp --
   NOTE :  it must be ((startPort & 0x1f) + numPorts) <= 31
*/
#define PORT_RANGE_MAC(startPort,numPorts)\
    (GT_U32)(BMP_CONTINUES_PORTS_MAC(numPorts) << ((startPort) & 0x1f ))

static const CPSS_PORTS_BMP_STC portsBmp24_25_27 = {{(PORT_RANGE_MAC(24,2) |
                                                      PORT_RANGE_MAC(27,1)) ,0}};

static const CPSS_PORTS_BMP_STC portsBmp0to23_27 = {{(PORT_RANGE_MAC(0,24) |
                                                      PORT_RANGE_MAC(27,1)) ,0}};

static const CPSS_PORTS_BMP_STC portsBmpCh3_8_Xg = {{(PORT_RANGE_MAC(24,4) |
                                                      PORT_RANGE_MAC(12,1) |
                                                      PORT_RANGE_MAC(10,1) |
                                                      PORT_RANGE_MAC(4 ,1) |
                                                      PORT_RANGE_MAC(0 ,1)) ,0}};

static const CPSS_PORTS_BMP_STC portsBmpCh3Xg =    {{(PORT_RANGE_MAC(24,4) |
                                                      PORT_RANGE_MAC(22,1) |
                                                      PORT_RANGE_MAC(16,1) |
                                                      PORT_RANGE_MAC(12,1) |
                                                      PORT_RANGE_MAC(10,1) |
                                                      PORT_RANGE_MAC(4 ,1) |
                                                      PORT_RANGE_MAC(0 ,1)) ,0}};

static const CPSS_PORTS_BMP_STC portsBmp0to3_24_25 = {{(PORT_RANGE_MAC(0,4) |
                                                        PORT_RANGE_MAC(24,2)) ,0}};

static const CPSS_PORTS_BMP_STC portsBmp0to7_24_25 = {{(PORT_RANGE_MAC(0,8) |
                                                        PORT_RANGE_MAC(24,2)) ,0}};

static const CPSS_PORTS_BMP_STC portsBmp0to15_24_25 = {{(PORT_RANGE_MAC(0,16) |
                                                         PORT_RANGE_MAC(24,2)) ,0}};

static const CPSS_PORTS_BMP_STC portsBmp0to15_24to27 = {{(PORT_RANGE_MAC(0,16) |
                                                          PORT_RANGE_MAC(24,4)) ,0}};

static const CPSS_PORTS_BMP_STC portsBmp0to12_24to27 = {{(PORT_RANGE_MAC(0,12) |
                                                          PORT_RANGE_MAC(24,4)) ,0}};

static const CPSS_PORTS_BMP_STC portsBmp0to7_24to27 = {{(PORT_RANGE_MAC(0,8) |
                                                        PORT_RANGE_MAC(24,4)) ,0}};

static const CPSS_PORTS_BMP_STC portsBmp0to5_24to27 = {{(PORT_RANGE_MAC(0,6) |
                                                        PORT_RANGE_MAC(24,4)) ,0}};

static const CPSS_PORTS_BMP_STC portsBmp24to32 = {{0xFF000000 ,0x00000001}};

static const CPSS_PORTS_BMP_STC portsBmp_0to11_16to27_32_36_40_41_48_52_56_57_15_31 =
    {{(PORT_RANGE_MAC(0,12) /*0..11 */| PORT_RANGE_MAC(16,12) /*16..27 */),
      (BIT_0 |BIT_4 |BIT_8 |BIT_9 |/* 32,36,40,41*/ BIT_16 |BIT_20 |BIT_24 |BIT_25)/*48,52,56,57*/}};

/* macro for bmp of ports : 0..47 , 56..59 , 64..71 */
#define PORTS_BMP_0_TO_47_56_TO_59_64_TO_71                 \
    {{PORT_RANGE_MAC(0,16) | PORT_RANGE_MAC(16,16) ,        \
      PORT_RANGE_MAC(32,16) | PORT_RANGE_MAC(56,4) ,        \
      PORT_RANGE_MAC(64,8) , 0 }}

/* bmp of ports : 0..47 , 56..59 , 64..71 */
static const CPSS_PORTS_BMP_STC portsBmp0to47_56to59_64to71 =
    PORTS_BMP_0_TO_47_56_TO_59_64_TO_71;

/* macro for bmp of ports : 0..47 , 56..59 , 62, 64..71 */
#define PORTS_BMP_0_TO_47_56_TO_59_62_64_TO_71                              \
    {{PORT_RANGE_MAC(0,16) | PORT_RANGE_MAC(16,16) ,                        \
      PORT_RANGE_MAC(32,16) | PORT_RANGE_MAC(56,4) | PORT_RANGE_MAC(62,1),  \
      PORT_RANGE_MAC(64,8) , 0 }}

/* bmp of ports : 0..47 , 56..59 , 62, 64..71 */
static const CPSS_PORTS_BMP_STC portsBmp0to47_62_56to59_64to71 =
    PORTS_BMP_0_TO_47_56_TO_59_62_64_TO_71;


/* macro for bmp of ports : 0..23 , 56..59 , 64..71 */
#define PORTS_BMP_0_TO_23_56_TO_59_64_TO_71                 \
    {{PORT_RANGE_MAC(0,24),                                 \
      PORT_RANGE_MAC(56,4),                                 \
      PORT_RANGE_MAC(64,8) , 0 }}

/* macro for bmp of ports : 0..23 , 56..59 , 62, 64..71 */
#define PORTS_BMP_0_TO_23_56_TO_59_62_64_TO_71              \
    {{PORT_RANGE_MAC(0,24),                                 \
      PORT_RANGE_MAC(56,4) | PORT_RANGE_MAC(62,1),          \
      PORT_RANGE_MAC(64,8) , 0 }}

/* macro for bmp of ports : 0..27 , 56..59 , 62, 64..71 */
#define PORTS_BMP_0_TO_27_56_TO_59_62_64_TO_71              \
    {{PORT_RANGE_MAC(0,28),                                 \
      PORT_RANGE_MAC(56,4) | PORT_RANGE_MAC(62,1),          \
      PORT_RANGE_MAC(64,8) , 0 }}

/* macro for bmp of ports :  56..59 , 64..71 */
#define PORTS_BMP_56_TO_59_64_TO_71                         \
    {{0 ,                                                   \
      PORT_RANGE_MAC(56,4) ,                                \
      PORT_RANGE_MAC(64,8) , 0 }}

/* bmp of ports :         56..59 , 64..71 */
static const CPSS_PORTS_BMP_STC portsBmp56to59_64to71 =
    PORTS_BMP_56_TO_59_64_TO_71;

/* macro for bmp of ports :  56..59 , 62, 64..71 */
#define PORTS_BMP_56_TO_59_62_64_TO_71                      \
    {{0 ,                                                   \
      PORT_RANGE_MAC(56,4) | PORT_RANGE_MAC(62,1),          \
      PORT_RANGE_MAC(64,8) , 0 }}

/* bmp of ports :         56..59 , 62, 64..71 */
static const CPSS_PORTS_BMP_STC portsBmp56to59_62_64to71 =
    PORTS_BMP_56_TO_59_62_64_TO_71;

/* macro for bmp of ports : 0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 56..59, 62, 64..71 */
#define PORTS_BMP_0_4_8_12_16_20_24_28_32_36_40_44_56_TO_59_62_64_TO_71                   \
    {{PORT_RANGE_MAC( 0,1) | PORT_RANGE_MAC( 4,1) | PORT_RANGE_MAC( 8,1) | PORT_RANGE_MAC( 12,1) | PORT_RANGE_MAC(16,1) | PORT_RANGE_MAC(20,1) | PORT_RANGE_MAC(24,1) | PORT_RANGE_MAC(28,1),\
      PORT_RANGE_MAC(32,1) | PORT_RANGE_MAC(36,1) | PORT_RANGE_MAC(40,1) | PORT_RANGE_MAC(44,1) | PORT_RANGE_MAC(56,4) | PORT_RANGE_MAC(62,1),   \
      PORT_RANGE_MAC(64,8) , 0 }}

/* macro for bmp of ports : 4..11, 16..71 */
#define PORTS_BMP_4_TO_11_16_TO_72                              \
    {{PORT_RANGE_MAC(4,8) | PORT_RANGE_MAC(16,16) ,             \
      PORT_RANGE_MAC(32,16) | PORT_RANGE_MAC(48,16) ,           \
      PORT_RANGE_MAC(64,9), 0 }}

/* macro for bmp of ports : 0..35 , 47..59 */
#define PORTS_BMP_0_TO_35_47_TO_59                          \
    {{PORT_RANGE_MAC(0,16) | PORT_RANGE_MAC(16,16) ,        \
      PORT_RANGE_MAC(32,4) | PORT_RANGE_MAC(47, 13) , 0 }}

/* macro for bmp of ports : 0..23, 28..31, 36..39, 44..47, 52..55, 60..63, 68..71 */
#define PORTS_BMP_0_TO_23_28_TO_31_36_TO_39_44_TO_47_52_TO_55_60_TO_63_68_TO_71   \
    {{PORT_RANGE_MAC(0,24) | PORT_RANGE_MAC(28, 4) ,         \
      PORT_RANGE_MAC(36,4) | PORT_RANGE_MAC(44, 4) | PORT_RANGE_MAC(52,4) | PORT_RANGE_MAC(60, 4),   \
      PORT_RANGE_MAC(68, 4), 0 }}

/* macro for bmp of ports : 0..23 , 47 */
#define PORTS_BMP_0_TO_23_47                                \
    {{PORT_RANGE_MAC(0,24), PORT_RANGE_MAC(47,1), 0 }}

static const CPSS_PORTS_BMP_STC portsBmp4to11_16to72 =
    PORTS_BMP_4_TO_11_16_TO_72;

static const CPSS_PORTS_BMP_STC portsBmp0to35_47to59 =
    PORTS_BMP_0_TO_35_47_TO_59;

static const CPSS_PORTS_BMP_STC portsBmp48_special_map =
    PORTS_BMP_0_TO_23_28_TO_31_36_TO_39_44_TO_47_52_TO_55_60_TO_63_68_TO_71;

static const CPSS_PORTS_BMP_STC portsBmp0to23_47 =
    PORTS_BMP_0_TO_23_47;

static const CPSS_PORTS_BMP_STC portsBmp0to15 = {{PORT_RANGE_MAC(0,16) ,0}};
static const CPSS_PORTS_BMP_STC portsBmp4to13 = {{PORT_RANGE_MAC(4,10) ,0}};
static const CPSS_PORTS_BMP_STC portsBmp4to15 = {{PORT_RANGE_MAC(4,12) ,0}};
static const CPSS_PORTS_BMP_STC portsBmp4to7_13_15 = {{PORT_RANGE_MAC(4,4) | PORT_RANGE_MAC(13,1) | PORT_RANGE_MAC(15,1) ,0}};

/*****************************************************************************
* Global
******************************************************************************/
/* pointer to the object that control ExMxDx */
static PRV_CPSS_DRV_GEN_OBJ_STC drvGenExMxDxObj;
PRV_CPSS_DRV_GEN_OBJ_STC *prvCpssDrvGenExMxDxObjPtr = &drvGenExMxDxObj;

/* pointer to the object that control PCI */
PRV_CPSS_DRV_MNG_INF_OBJ_STC *prvCpssDrvMngInfPciPtr = NULL;

/* pointer to the object that control PEX with 8 address completion regions */
PRV_CPSS_DRV_MNG_INF_OBJ_STC *prvCpssDrvMngInfPexMbusPtr = NULL;

/* pointer to the object that control PCI -- for HA standby */
PRV_CPSS_DRV_MNG_INF_OBJ_STC *prvCpssDrvMngInfPciHaStandbyPtr = NULL;

/* pointer to the object that control SMI */
PRV_CPSS_DRV_MNG_INF_OBJ_STC *prvCpssDrvMngInfSmiPtr = NULL;

/* pointer to the object that control TWSI */
PRV_CPSS_DRV_MNG_INF_OBJ_STC *prvCpssDrvMngInfTwsiPtr = NULL;

/* pointer to the object that control PEX */
/*PRV_CPSS_DRV_MNG_INF_OBJ_STC *prvCpssDrvMngInfPexPtr = NULL;*/

/*****************************************************************************
* Externals
******************************************************************************/


/*****************************************************************************
* Forward declarations
******************************************************************************/

/*****************************************************************************
* Local variables
******************************************************************************/
#define END_OF_TABLE    0xFFFFFFFF

/*****************/
/* xCat3 devices */
/*****************/
static const  CPSS_PP_DEVICE_TYPE xcat3_24_2legalDevTypes[] =
{   PRV_CPSS_XCAT3_24GE_2STACK_PORTS_DEVICES,
    END_OF_TABLE };

static const  CPSS_PP_DEVICE_TYPE xcat3_24_4legalDevTypes[] =
{   PRV_CPSS_XCAT3_24GE_4STACK_PORTS_DEVICES,
    END_OF_TABLE };

static const  CPSS_PP_DEVICE_TYPE xcat3_16_4legalDevTypes[] =
{   PRV_CPSS_XCAT3_16GE_4STACK_PORTS_DEVICES,
    END_OF_TABLE };

static const  CPSS_PP_DEVICE_TYPE xcat3_8_4legalDevTypes[] =
{   PRV_CPSS_XCAT3_8GE_4STACK_PORTS_DEVICES,
    END_OF_TABLE };

static const  CPSS_PP_DEVICE_TYPE xcat3_8_2legalDevTypes[] =
{   PRV_CPSS_XCAT3_8GE_2STACK_PORTS_DEVICES,
    END_OF_TABLE };

/****************/
/* pipe devices */
/****************/
static const  CPSS_PP_DEVICE_TYPE pipe_12_4legalDevTypes[] =
{   CPSS_98PX1012_CNS,
    CPSS_98PX1016_CNS,
    CPSS_98PX1015_CNS,
    END_OF_TABLE };
static const  CPSS_PP_DEVICE_TYPE pipe_8_2legalDevTypes[] =
{   CPSS_98PX1008_CNS,
    END_OF_TABLE };
static const  CPSS_PP_DEVICE_TYPE pipe_8_4legalDevTypes[] =
{   CPSS_98PX1024_CNS,
    END_OF_TABLE };
static const  CPSS_PP_DEVICE_TYPE pipe_4_2legalDevTypes[] =
{   CPSS_98PX1022_CNS,
    END_OF_TABLE };


/****************/
/* lion2 devices */
/****************/

/* lion2 with port groups 0,1,2,3,4,5,6,7 */
static const  CPSS_PP_DEVICE_TYPE lion2_port_groups01234567legalDevTypes[] = {CPSS_LION2_PORT_GROUPS_01234567_CNS , END_OF_TABLE };

/* lion2-hooper with port groups 0,1,2,3 */
static const  CPSS_PP_DEVICE_TYPE lion2_hooper_port_groups0123legalDevTypes[] = {CPSS_LION2_HOOPER_PORT_GROUPS_0123_CNS , END_OF_TABLE };

static const PORT_GROUPS_INFO_STC lionPortGroups =
{
    4 /*numOfPortGroups*/ ,
    PORT_RANGE_MAC(0,12) /*portsBmp*/   ,
    16 /*maxNumPorts*/
};

static const PORT_GROUPS_INFO_STC lion2PortGroups =
{
    8 /*numOfPortGroups*/ ,
    PORT_RANGE_MAC(0,12) /*portsBmp*/   ,
    16 /*maxNumPorts*/
};

/* macro to set any of 4 port groups as active(exists) in a bitmap */
#define SET_4_PORT_GROUPS_MAC(portGroup0Exists,portGroup1Exists,portGroup2Exists,portGroup3Exists) \
    ((portGroup3Exists) << 3) | ((portGroup2Exists) << 2) |                        \
    ((portGroup1Exists) << 1) | (portGroup0Exists)

/* bmp of lion port groups , for 'known' devices */
static const GT_U32 portGroups01__bmp = SET_4_PORT_GROUPS_MAC(1,1,0,0);
static const GT_U32 portGroups0123bmp = SET_4_PORT_GROUPS_MAC(1,1,1,1);
static const GT_U32 portGroups01234567bmp = 0xff;

/* number of ports in the lion device according to the last active port group */
#define LION_NUM_PORTS_FROM_LAST_PORT_GROUP_MAC(lastPortGroup) (((lastPortGroup + 1) * 16)-4)

/* Bobcat2 devices */
static const  CPSS_PP_DEVICE_TYPE bobcat2_legalDevTypes[] =
{   PRV_CPSS_BOBCAT2_DEVICES,
    END_OF_TABLE};
/* Bobk caelum devices */
static const  CPSS_PP_DEVICE_TYPE bobkCaelum_legalDevTypes[] =
{   PRV_CPSS_BOBK_CAELUM_DEVICES,
    END_OF_TABLE};
/* Bobk cetus devices */
static const  CPSS_PP_DEVICE_TYPE bobkCetus_legalDevTypes[] =
{   PRV_CPSS_BOBK_CETUS_DEVICES,
    END_OF_TABLE};

/* Aldrin devices */
static const  CPSS_PP_DEVICE_TYPE aldrin_legalDevTypes[] =
{   PRV_CPSS_ALDRIN_DEVICES,
    END_OF_TABLE};

/* AC3X devices */
static const  CPSS_PP_DEVICE_TYPE ac3x_legalDevTypes[] =
{   PRV_CPSS_AC3X_DEVICES,
    END_OF_TABLE};

/* Bobcat3 devices */
static const  CPSS_PP_DEVICE_TYPE bobcat3_legalDevTypes[] =
{   PRV_CPSS_BOBCAT3_ALL_DEVICES,
    END_OF_TABLE};

/* Armstrong devices */
static const  CPSS_PP_DEVICE_TYPE armstrong_legalDevTypes[] =
{   PRV_CPSS_ARMSTRONG_ALL_DEVICES,
    END_OF_TABLE};

/* Aldrin2 devices */
static const  CPSS_PP_DEVICE_TYPE aldrin2_72_legalDevTypes[] =
{   PRV_CPSS_ALDRIN2_72_PORTS_DEVICES,
    END_OF_TABLE};

/* Aldrin2 devices */
static const  CPSS_PP_DEVICE_TYPE aldrin2_64_legalDevTypes[] =
{   PRV_CPSS_ALDRIN2_64_PORTS_DEVICES,
    END_OF_TABLE};

/* Aldrin2 devices */
static const  CPSS_PP_DEVICE_TYPE aldrin2_48_legalDevTypes[] =
{   PRV_CPSS_ALDRIN2_48_PORTS_DEVICES,
    END_OF_TABLE};

/* Aldrin2 devices */
static const  CPSS_PP_DEVICE_TYPE aldrin2_24_legalDevTypes[] =
{   PRV_CPSS_ALDRIN2_24_PORTS_DEVICES,
    END_OF_TABLE};

#ifndef GM_USED /* Falcon devices for WM and BM (but not GM) */
/* Falcon devices */
static const  CPSS_PP_DEVICE_TYPE falcon_3_2_legalDevTypes[] =
{
    CPSS_98CX8520_CNS       ,
    END_OF_TABLE};
/* Falcon devices */
static const  CPSS_PP_DEVICE_TYPE falcon_6_4_legalDevTypes[] =
{
    CPSS_98CX8540_CNS       ,
    CPSS_98CX8540_10_CNS    ,
    END_OF_TABLE};
/* Falcon devices */
static const  CPSS_PP_DEVICE_TYPE falcon_12_8_legalDevTypes[] =
{
    CPSS_98CX8580_CNS       ,
    END_OF_TABLE};
#else /* device for the 'GM' to use */
/* Falcon devices */
static const  CPSS_PP_DEVICE_TYPE falcon_GM_legalDevTypes[] =
{   PRV_CPSS_FALCON_ALL_DEVICES,
    END_OF_TABLE};
#endif


/* alias to make line shorter */
#define LION2_CNS           CPSS_PP_FAMILY_DXCH_LION2_E
#define BOBCAT2_CNS         CPSS_PP_FAMILY_DXCH_BOBCAT2_E
#define BOBCAT3_CNS         CPSS_PP_FAMILY_DXCH_BOBCAT3_E
#define ALDRIN_CNS          CPSS_PP_FAMILY_DXCH_ALDRIN_E
#define AC3X_CNS            CPSS_PP_FAMILY_DXCH_AC3X_E
#define XCAT3_CNS           CPSS_PP_FAMILY_DXCH_XCAT3_E
#define ALDRIN2_CNS            CPSS_PP_FAMILY_DXCH_ALDRIN2_E
#define PIPE_CNS            CPSS_PX_FAMILY_PIPE_E
#define FALCON_CNS          CPSS_PP_FAMILY_DXCH_FALCON_E

#define NUM_OF_TILES_OFFSET     16
#define NUM_OF_TILES_AND_PORTS(numOfTiles,numOfPorts)   \
    (((numOfTiles) << NUM_OF_TILES_OFFSET) | (numOfPorts))

#define GET_NUM_OF_TILES(value)  ((value) >> NUM_OF_TILES_OFFSET)

#define GET_NUM_OF_PORTS(value)  ((value) & ((1<<NUM_OF_TILES_OFFSET)-1))


static GT_U32   debug_force_numOfPorts = 0;
void debug_force_numOfPorts_set(GT_U32 num)
{
    debug_force_numOfPorts = num;
}

/* cpssSupportedTypes :
    purpose :
        DB to hold the device types that the CPSS support

fields :
    devFamily - device family
    numOfPorts - number of ports in the device
    devTypeArray - array of devices for the device family that has same number
                   of ports
    defaultPortsBmpPtr - pointer to special ports bitmap.
                         if this is NULL , that means that the default bitmap
                         defined by the number of ports that considered to be
                         sequential for 0 to the numOfPorts
    portGroupInfoPtr - port groups info.
                    NULL --> meaning 'non multi port groups' device
                    otherwise hold info about the port groups of the device
    activePortGroupsBmpPtr - active port groups bitmap.
                    relevant only when portGroupInfoPtr != NULL
                    can't be NULL when portGroupInfoPtr != NULL
*/
typedef struct {
    CPSS_PP_FAMILY_TYPE_ENT     devFamily;
    GT_U32                      numOfPorts;
    const CPSS_PP_DEVICE_TYPE   *devTypeArray;
    const CPSS_PORTS_BMP_STC    *defaultPortsBmpPtr;
    const PORT_GROUPS_INFO_STC  *portGroupInfoPtr;
    const GT_U32                *activePortGroupsBmpPtr;
}CPSS_SUPPORTED_TYPES_STC;
const CPSS_SUPPORTED_TYPES_STC cpssSupportedTypes[] =
{
     /* Lion2 devices */                                             /* active port groups */
    {LION2_CNS ,LION_NUM_PORTS_FROM_LAST_PORT_GROUP_MAC(7) ,lion2_port_groups01234567legalDevTypes, NULL ,&lion2PortGroups,&portGroups01234567bmp},

     /* Lion2 Hooper devices */                                             /* active port groups */
    {LION2_CNS ,LION_NUM_PORTS_FROM_LAST_PORT_GROUP_MAC(3) ,lion2_hooper_port_groups0123legalDevTypes, NULL ,&lionPortGroups,&portGroups0123bmp},

    /* Bobcat2 supports 72 MACs, 74 DMAs. */
    {BOBCAT2_CNS ,72 ,bobcat2_legalDevTypes, NULL,NULL,NULL},

    /* Bobk supports range of 72 MACs, 74 DMAs. */
    /* bobk Caelum : */
    /* actual macs are in : 0..47 (GE) , 56..59 (XLG) , 64..71 (XLG) */
    /* there are also 2 TX DMA : 72(cpu),73(TM) */
    /* there is  also 1 RX DMA : 72(cpu) */
    /* NOTE: same MACs as for 'CPSS_98DX4221_CNS' */
    {BOBCAT2_CNS ,72 ,bobkCaelum_legalDevTypes, &portsBmp0to47_62_56to59_64to71,NULL,NULL},

    /* bobk Cetus : */
    /* actual macs are in : 56..59 (XLG) , 64..71 (XLG) */
    /* there are also 2 TX DMA : 72(cpu),73(TM) */
    /* there is  also 1 RX DMA : 72(cpu) */
    {BOBCAT2_CNS ,72 ,bobkCetus_legalDevTypes,  &portsBmp56to59_62_64to71      ,NULL,NULL},

    /* Aldrin : */
    /* actual macs are in : 0..31 */
    /* there are also 1 external CPU DMA (MG port) : 32*/
    {ALDRIN_CNS ,33 ,aldrin_legalDevTypes,  NULL,NULL,NULL},

    /* AC3X : */
    /* actual macs are in : 0..31 */
    /* there are also 1 external CPU DMA (MG port) : 32*/
    {AC3X_CNS ,33 ,ac3x_legalDevTypes,  NULL,NULL,NULL},

#ifndef GM_USED
    /* Bobcat3 supports 74 MACs, 78 DMAs. */
    {BOBCAT3_CNS ,74 ,bobcat3_legalDevTypes,   NULL,NULL,NULL},
    {BOBCAT3_CNS ,74 ,armstrong_legalDevTypes, NULL,NULL,NULL},
#else /*GM_USED*/
    /* Bobcat3 GM supports single DP : 12 MACs, 13 DMAs. */
    {BOBCAT3_CNS ,12 ,bobcat3_legalDevTypes,   NULL,NULL,NULL},
    {BOBCAT3_CNS ,12 ,armstrong_legalDevTypes, NULL,NULL,NULL},
#endif  /*GM_USED*/

    /* Aldrin2 supports 73 MACs, 77 DMAs. */
    {ALDRIN2_CNS    ,73 ,aldrin2_72_legalDevTypes, NULL,NULL,NULL},

     /* Aldrin2 : */
    /* actual macs are in : 4..11, 16..71 */
    /* there is also 1 external CPU DMA (MG port) : 72*/
    {ALDRIN2_CNS    ,73 ,aldrin2_64_legalDevTypes, &portsBmp4to11_16to72,NULL,NULL},

    /* Aldrin2 : */
    /* actual macs are in : 0..35, 48..59 */
    /* there is also 1 external CPU DMA (MG port) : 47*/
    {ALDRIN2_CNS    ,60 ,aldrin2_48_legalDevTypes, &portsBmp0to35_47to59,NULL,NULL},

    /* Aldrin2 : */
    /* actual macs are in : 0..23 */
    /* there is also 1 external CPU DMA (MG port) : 47*/
    {ALDRIN2_CNS    ,48 ,aldrin2_24_legalDevTypes, &portsBmp0to23_47,NULL,NULL},

    /* xCat3 devices  - 24network, 2stack */
    /* xCat3 devices  - 24network, 2stack */
    {XCAT3_CNS    ,24+2 ,xcat3_24_2legalDevTypes, NULL,NULL,NULL},

    /* xCat3 devices  - 24network, 4stack */
    {XCAT3_CNS    ,24+4 ,xcat3_24_4legalDevTypes, NULL,NULL,NULL},/*ports 0..27 */

    /* xCat3 devices  - 16network, 2stack */
    {XCAT3_CNS    ,24+4 ,xcat3_16_4legalDevTypes, &portsBmp0to15_24to27,NULL,NULL}, /*ports 0..15,24..27 */

    /* xCat3 devices  - 8network, 4stack */
    {XCAT3_CNS    ,24+4 ,xcat3_8_4legalDevTypes, &portsBmp0to7_24to27,NULL,NULL}, /*ports 0..7,24..27 */

    /* xCat3 devices  - 8network, 2stack */
    {XCAT3_CNS    ,24+2 ,xcat3_8_2legalDevTypes, &portsBmp0to7_24_25,NULL,NULL}, /*ports 0..7,24..25 */

    /* PX : PIPE devices  - 12 network, 4 stack */
    {PIPE_CNS     ,12+4 ,pipe_12_4legalDevTypes, &portsBmp0to15,NULL,NULL}, /*ports 0..15 (NOT include CPU port) */

    /* PX : PIPE devices  - 8 network, 2 stack */
    {PIPE_CNS     ,14 ,pipe_8_2legalDevTypes, &portsBmp4to13,NULL,NULL}, /*ports 4..13 (NOT include CPU port) */

    /* PX : PIPE devices  - 8 network, 4 stack */
    {PIPE_CNS     ,16 ,pipe_8_4legalDevTypes, &portsBmp4to15,NULL,NULL}, /*ports 4..15 (NOT include CPU port) */

    /* PX : PIPE devices  - 4 network, 2 stack */
    {PIPE_CNS     ,16 ,pipe_4_2legalDevTypes, &portsBmp4to7_13_15,NULL,NULL}, /*ports 4..7,13,15 (NOT include CPU port) */

#ifndef GM_USED
    /* Falcon 3.2 Tera supports  65 MACs, +3 SDMAs. */
    {FALCON_CNS ,NUM_OF_TILES_AND_PORTS(1,64+1)  ,falcon_3_2_legalDevTypes,   NULL,NULL,NULL},
    /* Falcon 6.4 Tera supports 130 MACs, +6 SDMAs. */
    {FALCON_CNS ,NUM_OF_TILES_AND_PORTS(2,128+2) ,falcon_6_4_legalDevTypes,   NULL,NULL,NULL},
    /* Falcon 12.8 Tera supports 258 MACs, +6 SDMAs. */
    {FALCON_CNS ,NUM_OF_TILES_AND_PORTS(4,256+2) ,falcon_12_8_legalDevTypes,  NULL,NULL,NULL},
#else /*GM_USED*/
    /* Falcon GM supports single DP : 8 MACs, 9 DMAs. */
    {FALCON_CNS ,NUM_OF_TILES_AND_PORTS(1,8) ,falcon_GM_legalDevTypes,    NULL,NULL,NULL},
#endif  /*GM_USED*/
    /* End of list      */
    {(CPSS_PP_FAMILY_TYPE_ENT)END_OF_TABLE   ,0 , NULL,NULL, NULL, NULL}
};

/*const*/ CPSS_SUPPORTED_TYPES_STC onEmulator_cpssSupportedTypes[] =
{
    /* Aldrin : limited number of ports */
    /* actual macs are in : 24..31 */
    /* there is also 1 external CPU DMA (MG port) : 32*/
    /* NOTE: this line is 'modified' at run time if running in 'Aldrin FULL' mode */
    /*       modified to use 'NULL' instead of '&portsBmp24to32' */
    {ALDRIN_CNS ,33 ,aldrin_legalDevTypes,  &portsBmp24to32,NULL,NULL},
    {ALDRIN2_CNS,73 ,aldrin2_72_legalDevTypes, NULL,NULL,NULL},

    /* PX : PIPE devices  - 12 network, 4 stack */
    {PIPE_CNS    ,12+4 ,pipe_12_4legalDevTypes, &portsBmp0to15,NULL,NULL}, /*ports 0..15 (NOT include CPU port) */

    /* PX : PIPE devices  - 8 network, 2 stack */
    {PIPE_CNS    ,14 ,pipe_8_2legalDevTypes, &portsBmp4to13,NULL,NULL},

    /* PX : PIPE devices  - 8 network, 4 stack */
    {PIPE_CNS    ,16 ,pipe_8_4legalDevTypes, &portsBmp4to15,NULL,NULL},

    /* PX : PIPE devices  - 4 network, 2 stack */
    {PIPE_CNS    ,16 ,pipe_4_2legalDevTypes, &portsBmp4to7_13_15,NULL,NULL},

#ifndef GM_USED
    /* Falcon 3.2 Tera supports  65 MACs, +3 SDMAs. */
    {FALCON_CNS ,NUM_OF_TILES_AND_PORTS(1,64+1)  ,falcon_3_2_legalDevTypes,   NULL,NULL,NULL},
    /* Falcon 6.4 Tera supports 130 MACs, +6 SDMAs. */
    {FALCON_CNS ,NUM_OF_TILES_AND_PORTS(2,128+2) ,falcon_6_4_legalDevTypes,   NULL,NULL,NULL},
    /* Falcon 12.8 Tera supports 258 MACs, +6 SDMAs. */
    {FALCON_CNS ,NUM_OF_TILES_AND_PORTS(4,256+2) ,falcon_12_8_legalDevTypes,  NULL,NULL,NULL},
#endif  /*!GM_USED*/

    /* End of list      */
    {(CPSS_PP_FAMILY_TYPE_ENT)END_OF_TABLE   ,0 , NULL,NULL, NULL, NULL}
};

typedef struct{
    GT_U32  regAddr;
    GT_U32  regValue;
}DEFAULT_REG_STC;
/*
    on emulator - Aldrin (limited to DP[2] and not TCAM):
    below the registers' values in the MG for neutralizing the missing units.
    For the missing units you should receive "badad" if you try to read them
    and the write will be lost.
*/
static const DEFAULT_REG_STC onEmulator_AldrinDefaultRegArr[] =
{
     {0x000000E4        ,0x707DE85F}
    ,{0x000000E8        ,0xF820E007}
    ,{0x000000EC        ,0xFF00FF81}
    ,{0x00000140        ,0x00008100}
    ,{0x000000F0        ,0xE3A     }
    /* must be last */
    ,{END_OF_TABLE      ,END_OF_TABLE}
};

/*
    on emulator - Aldrin (no limitations on units):
    below the registers' values in the MG for neutralizing the missing units.
    For the missing units you should receive "badad" if you try to read them
    and the write will be lost.
*/
static const DEFAULT_REG_STC onEmulator_AldrinDefaultRegArr_full[] =
{
     {0x000000E4        ,0x70FDE87F}
    ,{0x000000E8        ,0xF820E0C7}
    ,{0x000000EC        ,0xFF30FF81}
    ,{0x000000F0        ,0xFFA     }
    ,{0x00000140        ,0x00008100}
    /* must be last */
    ,{END_OF_TABLE      ,END_OF_TABLE}
};

/*
    on emulator - Aldrin2 :
    special settings that needed for Emulator.
    Currently - Empty.
*/
static const DEFAULT_REG_STC onEmulator_Aldrin2DefaultRegArr[] =
{
    /* must be last */
     {END_OF_TABLE      ,END_OF_TABLE}
};

/*
    on emulator - Pipe :
    special settings that needed for Emulator.
    Currently - Empty.
*/
static const DEFAULT_REG_STC onEmulator_PipeDefaultRegArr[] =
{
    /* must be last */
     {END_OF_TABLE      ,END_OF_TABLE}
};

/*
    on emulator - Falcon :
    special settings that needed for Emulator.
    Currently - Empty.
*/
static const DEFAULT_REG_STC onEmulator_FalconDefaultRegArr[] =
{
    /* must be last */
     {END_OF_TABLE      ,END_OF_TABLE}
};

/* support formula of addresses like : 0x13000000 + 0x1000*s: where s (0-32) represents serdes */
typedef struct{
    GT_U32  baseRegAddr;
    GT_U32  stepsInBytes;
    GT_U32  startStepIndex;
    GT_U32  numOfSteps;
    GT_U32  regValue;
}DEFAULT_REG_FORMULA_STC;

/* un-reset the SERDESes (set bits 2,3) */
#define UNRESET_SERDES_CNS  0x0000880d
#define OPEN_SERDES_LINK_CNS     0 /* 0 - not open SERDESes , 1 - open SERDESes */
/*
    on emulator - Aldrin  - support formulas
    0x13000000 + 0x1000*s: where s (0-32) represents serdes
*/
static const DEFAULT_REG_FORMULA_STC onEmulator_AldrinDefaultRegArr_full_formula[] =
{
#if OPEN_SERDES_LINK_CNS
    /* un-reset the SERDESes (set bits 2,3) */
     {0x13000004        ,0x1000/*steps between SERDESs*/ ,0/*start SERDES*/ , 33 /*number of SERDESes*/ , UNRESET_SERDES_CNS /* value*/}
    /* must be last */
    ,
#endif /*OPEN_SERDES_LINK_CNS*/
    {END_OF_TABLE      ,END_OF_TABLE , END_OF_TABLE , END_OF_TABLE , END_OF_TABLE}
};

/*
    on emulator - Aldrin2 - support formulas
    0x13000000 + 0x1000 * (a-9) + 0x80000: where a (72-72) represents CPU_SERDES
    0x13000000 + 0x1000 * (a-24) + 0x80000: where a (24-71) represents SERDES_Num
    0x13000000 + 0x1000 * a: where a (0-23) represents SERDES_Num.
*/
static const DEFAULT_REG_FORMULA_STC onEmulator_Aldrin2DefaultRegArr_formula[] =
{
#if OPEN_SERDES_LINK_CNS
    /* un-reset the SERDESes (set bits 2,3) */
     {0x13000004            ,0x1000/*steps between SERDESs*/ ,    0/*start SERDES*/ , 24    /*number of SERDESes*/ , UNRESET_SERDES_CNS /* value*/}
    ,{0x13000004 + 0x80000  ,0x1000/*steps between SERDESs*/ ,24-24/*start SERDES*/ , 72-24 /*number of SERDESes*/ , UNRESET_SERDES_CNS /* value*/}
    ,{0x13000004 + 0x80000  ,0x1000/*steps between SERDESs*/ ,72- 9/*start SERDES*/ , 1     /*number of SERDESes*/ , UNRESET_SERDES_CNS /* value*/}
    ,
#endif /*OPEN_SERDES_LINK_CNS*/

    /* must be last */
    {END_OF_TABLE      ,END_OF_TABLE , END_OF_TABLE , END_OF_TABLE , END_OF_TABLE}
};

/*
    on emulator - Pipe  - support formulas
    0x13000000 + 0x1000*a : where a (0-15) represents SERDES_Num
*/
static const DEFAULT_REG_FORMULA_STC onEmulator_PipeDefaultRegArr_formula[] =
{
#if OPEN_SERDES_LINK_CNS
    /* un-reset the SERDESes (set bits 2,3) */
     {0x13000004        ,0x1000/*steps between SERDESs*/ ,0/*start SERDES*/ , 16 /*number of SERDESes*/ , UNRESET_SERDES_CNS /* value*/}
    ,
#endif /*OPEN_SERDES_LINK_CNS*/

    /* must be last */
    {END_OF_TABLE      ,END_OF_TABLE , END_OF_TABLE , END_OF_TABLE , END_OF_TABLE}
};

/*
    on emulator - Falcon  - support formulas
*/
static const DEFAULT_REG_FORMULA_STC onEmulator_FalconDefaultRegArr_formula[] =
{
    /* must be last */
    {END_OF_TABLE      ,END_OF_TABLE , END_OF_TABLE , END_OF_TABLE , END_OF_TABLE}
};

/* cpssSpecialDevicesBmp -
 * Purpose : DB to hold the devices with special ports BMP
 *
 * NOTE : devices that his port are sequential for 0 to the numOfPorts
 *        not need to be in this Array !!!
 *
 * fields :
 *  devType - device type that has special ports bmp (that is different from
 *            those of other devices of his family with the same numOfPort)
 *  existingPorts - the special ports bmp of the device
 *
*/
static const struct {
    CPSS_PP_DEVICE_TYPE         devType;
    CPSS_PORTS_BMP_STC          existingPorts;
}cpssSpecialDevicesBmp[] =
{
    /* bobcat2 MACs : 0..23 , 56..59 , 64..71 */
    {CPSS_98DX4220_CNS , PORTS_BMP_0_TO_23_56_TO_59_64_TO_71},

    /* bobcat2 MACs : 0..47, 56..59 , 64..71 */
    {CPSS_98DX4221_CNS , PORTS_BMP_0_TO_47_56_TO_59_64_TO_71},

    /* bobcat2 MACs : 0..47, 60..71 */
    {CPSS_98DX4222_CNS , {{PORT_RANGE_MAC(0,16) | PORT_RANGE_MAC(16,16) , PORT_RANGE_MAC(32,16) | PORT_RANGE_MAC(60,4) , PORT_RANGE_MAC(64,8) , 0 }}},

    /* bobcat2 MACs : 52..55 , 56..59, 64..67, 68..71 */
    {CPSS_98DX8216_CNS , {{0, PORT_RANGE_MAC(52,8) , PORT_RANGE_MAC(64,8) , 0 }}},

    /* bobcat2 MACs : 0...8, 48..63, 64..71 */
    {CPSS_98DX8219_CNS , {{PORT_RANGE_MAC(0,8), PORT_RANGE_MAC(48,16),  PORT_RANGE_MAC(64,8) , 0 }}},

    /* bobcat2 MACs : 48..51 , 52..55 , 56..59, 60..63, 64..67, 68..71 */
    {CPSS_98DX8224_CNS , {{0, PORT_RANGE_MAC(48,16) , PORT_RANGE_MAC(64,8) , 0 }}},

    /* bobcat2 MACs : 32..47, 52..59 */
    {CPSS_98DX4253_CNS , {{0, PORT_RANGE_MAC(32,16) | PORT_RANGE_MAC(52,8) , 0 , 0 }}},

    /* Cetus MACs : 64..67, 68..71 */
    {CPSS_98DX8208_CNS , {{0, PORT_RANGE_MAC(62,1), PORT_RANGE_MAC(64,8) , 0 }}},

    /* Caelum MACs : 0..23 , 56..59 , 62, 64..71 */
    {CPSS_98DX4204_CNS , PORTS_BMP_0_TO_23_56_TO_59_62_64_TO_71},

    /* Caelum MACs : 0..23 , 56..59 , 62, 64..71 */
    {CPSS_98DX4210_CNS , PORTS_BMP_0_TO_23_56_TO_59_62_64_TO_71},

    /* Caelum MACs : 0..23 , 56..59 , 62, 64..71 */
    {CPSS_98DX3346_CNS,  PORTS_BMP_0_TO_23_56_TO_59_62_64_TO_71},

    /* Caelum MACs : 0..27 , 56..59 , 62, 64..71 */
    {CPSS_98DX3345_CNS,  PORTS_BMP_0_TO_27_56_TO_59_62_64_TO_71},

    /* Caelum MACs : 0..27 , 56..59 , 62, 64..71 */
    {CPSS_98DX4212_CNS , PORTS_BMP_0_TO_27_56_TO_59_62_64_TO_71},

    /* Aldrin Z0 MACs : 0, 4, 8 , 12, 16, 20, 24, 28, 32, 36, 40, 44, 56..59 , 62, 64..71 */
    {CPSS_98DX8332_Z0_CNS , PORTS_BMP_0_4_8_12_16_20_24_28_32_36_40_44_56_TO_59_62_64_TO_71},

    /* Aldrin MACs: 0..7, 32 */
    {CPSS_98DX8308_CNS, {{PORT_RANGE_MAC(0,8), 0x00000001}}},

    /* Aldrin MACs: 0..11, 32 */
    {CPSS_98DX8312_CNS, {{PORT_RANGE_MAC(0,12), 0x00000001}}},

    /* Aldrin MACs: 0..15, 32 */
    {CPSS_98DX8316_CNS, {{PORT_RANGE_MAC(0,16), 0x00000001}}},

    /* Aldrin MACs: 0..23, 32 */
    {CPSS_98DX8324_CNS, {{PORT_RANGE_MAC(0,24), 0x00000001}}},

    /* AC3X MACs: 0..17, 32 */
    {CPSS_98DX3256_CNS, {{PORT_RANGE_MAC(0,18), 0x00000001}}},

    /* AC3X MACs: 0..23, 32 */
    {CPSS_98DX3257_CNS, {{PORT_RANGE_MAC(0,24), 0x00000001}}},

    /* Aldrin MACs: 0..15, 32 */
    {CPSS_98DXH831_CNS, {{PORT_RANGE_MAC(0,16), 0x00000001}}},

    /* Aldrin MACs: 0..23, 32 */
    {CPSS_98DXH832_CNS, {{PORT_RANGE_MAC(0,24), 0x00000001}}},

    /* End of list      */
    {END_OF_TABLE   ,{{0,0}} }
};

#ifdef CHX_FAMILY

static GT_STATUS prvCpssDrvLion2FirstPacketWA
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN PRV_CPSS_DRV_PP_PHASE_1_INPUT_INFO_STC  *ppInInfoPtr
);

static GT_STATUS prvCpssDrvLion2MMPcsUnReset
(
    GT_U8                   devNum,
    GT_U32                  portGroupId
);

#endif /* CHX_FAMILY */

#ifdef ASIC_SIMULATION
typedef enum{
    BIG_ENDIAN_E = 1, /* HOST is 'Big endian' */
    LITTLE_ENDIAN_E   /* HOST is 'Little endian' */
}ENDIAN_ENT;

/**
* @internal checkHostEndian function
* @endinternal
*
* @brief   This function check if the HOST is 'Big endian' or 'Little endian'
*
* @retval one of ENDIAN_ENT        - BIG_ENDIAN_E / LITTLE_ENDIAN_E
*/
static ENDIAN_ENT  checkHostEndian(void)
{
    GT_U32  wordMem = 0x11223344;
    GT_U8  *charMemPtr = (GT_U8*)(&wordMem);

    if(charMemPtr[0] == 0x11)
    {
        /* 'Big endian' - the bits 24..31 sit in byte0 */
        return BIG_ENDIAN_E;
    }

    /* 'Little endian' - the bits 24..31 sit in byte3 */
    return LITTLE_ENDIAN_E;
}
#endif /*ASIC_SIMULATION*/

/**
* @internal phase1Part1Init function
* @endinternal
*
* @brief   This function is called by prvCpssDrvHwPpPhase1Init()
*         the function bind the PCI/SMI/TWSI driver functions to the driver object
*         functions for this device.
* @param[in] devNum                   - The PP's device number to be initialized.
* @param[in] ppInInfoPtr              - (pointer to)   the info needed for initialization of
*                                      the driver for this PP
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - bad management interface value
* @retval GT_NOT_IMPLEMENTED       - the needed driver was not compiled correctly
*/
static GT_STATUS phase1Part1Init
(
    IN  GT_U8                devNum,
    IN  PRV_CPSS_DRV_PP_PHASE_1_INPUT_INFO_STC  *ppInInfoPtr
)
{
    GT_STATUS rc;

    /********************************************/
    /* bind the BUS part to the ExMxDx BUS part */
    /********************************************/


    /* set rc as "not implemented" to suggest that the specific SMI/PCI/TWSI is
       not part of the image although needed in the image */
    rc = GT_NOT_IMPLEMENTED;
    /* driver object bind */
    if(ppInInfoPtr->mngInterfaceType == CPSS_CHANNEL_PCI_E)
    {
#if defined GT_PCI  /*|| defined GT_PEX*/
        rc = GT_OK;
        if(ppInInfoPtr->ppHAState == CPSS_SYS_HA_MODE_ACTIVE_E)
        {
            prvCpssDrvPpObjConfig[devNum]->busPtr = prvCpssDrvMngInfPciPtr;
        }
        else
        {
            prvCpssDrvPpObjConfig[devNum]->busPtr = prvCpssDrvMngInfPciHaStandbyPtr;
        }
#endif
    }
    /* SMI */
    else if (ppInInfoPtr->mngInterfaceType == CPSS_CHANNEL_SMI_E)
    {
#ifdef GT_SMI
        rc = GT_OK;
        prvCpssDrvPpObjConfig[devNum]->busPtr = prvCpssDrvMngInfSmiPtr;
#endif
    }
 /* TWSI */
    else if (ppInInfoPtr->mngInterfaceType == CPSS_CHANNEL_TWSI_E)
    {
#ifdef GT_I2C
        rc = GT_OK;
        prvCpssDrvPpObjConfig[devNum]->busPtr = prvCpssDrvMngInfTwsiPtr;
#endif
    }
    else if(CPSS_CHANNEL_IS_PEX_MAC(ppInInfoPtr->mngInterfaceType))
    {
#if defined GT_PCI  /*|| defined GT_PEX*/
        rc = GT_OK;
        if(ppInInfoPtr->ppHAState == CPSS_SYS_HA_MODE_ACTIVE_E)
        {
            prvCpssDrvPpObjConfig[devNum]->busPtr = prvCpssDrvMngInfPciPtr;
        }
        else
        {
            prvCpssDrvPpObjConfig[devNum]->busPtr = prvCpssDrvMngInfPciHaStandbyPtr;
        }
#endif
    }
    else if(CPSS_CHANNEL_IS_PEX_MBUS_MAC(ppInInfoPtr->mngInterfaceType))
    {
#if defined GT_PCI  /*|| defined GT_PEX*/
        rc = GT_OK;
        if(ppInInfoPtr->ppHAState == CPSS_SYS_HA_MODE_ACTIVE_E)
        {
            prvCpssDrvPpObjConfig[devNum]->busPtr = prvCpssDrvMngInfPexMbusPtr;
        }
        else
        {
            prvCpssDrvPpObjConfig[devNum]->busPtr = prvCpssDrvMngInfPciHaStandbyPtr;
        }
#endif
    }
    else
    {
        /* unknown interface type */
        rc = GT_BAD_PARAM;
    }
    return rc;
}

/**
* @internal phase1Part2Init function
* @endinternal
*
* @brief   This function is called by prvCpssDrvHwPpPhase1Init()
*         This function allocate the memory for the DB of the driver for this
*         device , and set some of it's values.
* @param[in] devNum                   - The PP's device number to be initialized.
* @param[in] ppInInfoPtr              - (pointer to)   the info needed for initialization of
*                                      the driver for this PP
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_OUT_OF_CPU_MEM        - failed to allocate CPU memory,
* @retval GT_BAD_STATE             - the driver is not in state that ready for initialization
*                                       for the device
*/
static GT_STATUS phase1Part2Init
(
    IN  GT_U8                devNum,
    IN  PRV_CPSS_DRV_PP_PHASE_1_INPUT_INFO_STC  *ppInInfoPtr
)
{
    PRV_CPSS_DRIVER_PP_CONFIG_STC *ppConfigPtr;/* Points to the Device's Driver*/
    GT_U32      portGroupId;/* the port group Id */
    GT_BOOL     activePortGroupFound = GT_FALSE; /*indication that there is active port group */

    if(prvCpssDrvPpConfig[devNum] != NULL)
    {
        /* the device already exists ?! */
        /* we can't override it ! , the device must be empty before we
           initialize it */
        /* check that "remove device" was done properly */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    /* allocate the device a DB */
    ppConfigPtr = cpssOsMalloc(sizeof(PRV_CPSS_DRIVER_PP_CONFIG_STC));
    if(ppConfigPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    /* set default values */
    cpssOsMemSet(ppConfigPtr,0,sizeof(PRV_CPSS_DRIVER_PP_CONFIG_STC));
    /* bind the memory to the global DB */
    prvCpssDrvPpConfig[devNum] = ppConfigPtr;

    /* set value */
    ppConfigPtr->hwDevNum = devNum;
    ppConfigPtr->drvHwIfSupportedBmp = PRV_CPSS_HW_IF_BMP_MAC(ppInInfoPtr->mngInterfaceType);
    ppConfigPtr->mngInterfaceType = ppInInfoPtr->mngInterfaceType;
    if (CPSS_CHANNEL_IS_PCI_COMPATIBLE_MAC(ppInInfoPtr->mngInterfaceType))
    {
        /* in most cases we will treat the "PCI" and "PEX" the same */
        ppConfigPtr->drvHwIfSupportedBmp |= PRV_CPSS_HW_IF_BMP_MAC(CPSS_CHANNEL_PCI_E);
    }

    prvCpssDrvPpObjConfig[devNum]->HAState = ppInInfoPtr->ppHAState;

    for(portGroupId = 0; portGroupId < ppInInfoPtr->numOfPortGroups; portGroupId++)
    {
        ppConfigPtr->hwInfo[portGroupId] = ppInInfoPtr->hwInfo[portGroupId];

        if(ppConfigPtr->hwInfo[portGroupId].busType != CPSS_HW_INFO_BUS_TYPE_NONE_E)
        {
            /* active port group */
            activePortGroupFound = GT_TRUE;
        }

        ppConfigPtr->hwCtrl[portGroupId].isrAddrCompletionRegionsBmp  =
            ppInInfoPtr->isrAddrCompletionRegionsBmp;
        ppConfigPtr->hwCtrl[portGroupId].appAddrCompletionRegionsBmp  =
            ppInInfoPtr->appAddrCompletionRegionsBmp;

    }

    if(activePortGroupFound == GT_FALSE)
    {
        /* at least one port group need to be active */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvCpssDrvHwPpPrePhase1NextDevFamilySet function
* @endinternal
*
* @brief   This function called before 'phase 1' init of the cpss and the cpssDriver.
*         in order to give 'hint' for the cpssDriver about the family of the device
*         that will be 'next' processed by prvCpssDrvHwPpPhase1Init(...).
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvCpssDrvHwPpPrePhase1NextDevFamilySet(IN CPSS_PP_FAMILY_TYPE_ENT devFamily)
{
    nextDevFamily = devFamily;
    return GT_OK;
}

/**
* @internal prvCpssDrvHwPpPrePhase1NextDevFamilyGet function
* @endinternal
*
* @brief   return the 'nextDevFamily' that was set by prvCpssDrvHwPpPrePhase1NextDevFamilySet().
*
* @retval GT_OK                    - on success
*/
CPSS_PP_FAMILY_TYPE_ENT prvCpssDrvHwPpPrePhase1NextDevFamilyGet(GT_VOID)
{
    return nextDevFamily;
}

/**
* @internal driverHwCntlInitAndDeviceIdGet function
* @endinternal
*
* @brief   This function is called by prvCpssDrvHwPpPhase1Init()
*         This function call driverHwCntlInit to initialize the HW control.
*         The function get the deviceType from HW , doByteSwap
*         Note : for standby HA -- the doByteSwap must be figured when the system
*         changed to HA active
* @param[in] devNum                   - The PP's device number to be initialized.
* @param[in] portGroupId              - port group Id.
* @param[in] ppInInfoPtr              - (pointer to)   the info needed for initialization of
*                                      the driver for this PP
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS driverHwCntlInitAndDeviceIdGet
(
    IN  GT_U8                devNum,
    IN  GT_U32               portGroupId,
    IN  PRV_CPSS_DRV_PP_PHASE_1_INPUT_INFO_STC  *ppInInfoPtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_DRIVER_PP_CONFIG_STC *ppConfigPtr;/* Points to the Device's Driver*/
    GT_U32  readDeviceId ,readVendorId;/*deviceId,vendor Id as read from HW */
    GT_U32  tempDevId = 0;/* the device type built from device id , vendor id */
    GT_U32  regData;     /* register data */
    GT_U32  address;/* address to access */
    GT_U32  useDfxServerForDeviceId = 0;

    ppConfigPtr = prvCpssDrvPpConfig[devNum];

    switch(nextDevFamily)
    {
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            mg0UnitBaseAddr = PRV_CPSS_FALCON_MG0_BASE_ADDRESS_CNS;
            dfxUnitBaseAddr = PRV_CPSS_FALCON_DFX_BASE_ADDRESS_CNS;
            allowMultiMgSdmaInit = 0;
#ifndef GM_USED
            /* Falcon still expected to support the MG register */
            useDfxServerForDeviceId = 0;
#endif /*GM_USED*/
            break;
        default:
            mg0UnitBaseAddr = 0;
            dfxUnitBaseAddr = 0;
            allowMultiMgSdmaInit = 1;
            break;
    }

    if(0 == PRV_CPSS_DRV_HW_IF_PCI_COMPATIBLE_MAC(devNum))
    {
        /* The "do byte swap" relate only to the PCI bus , since on PCI we read
           4 BYTES and don't know if was read correctly

           so for non-PCI we set doByteSwap = GT_FALSE
        */

        /* Init Hw Cntl parameters. --
           must be done prior to any read/write registers ...*/
        rc = prvCpssDrvHwCntlInit(devNum,portGroupId,
                         &(ppConfigPtr->hwInfo[portGroupId]),
                         GT_FALSE);/* isDiag */
        if (rc!= GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

        /* read DeviceID */
        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId ,DEVICE_ID_AND_REV_REG_ADDR_CNS,
                                                4, 16, &readDeviceId);
        if (rc!= GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

        /* read VendorID */
        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId ,VENDOR_ID_REG_ADDR, 0, 16, &readVendorId) ;
        if (rc!= GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

        tempDevId = (readDeviceId << 16) | readVendorId ;
    }
    else  /* PCI/PEX compatible */
    {
        /* Init Hw Cntl parameters. The value of doByteSwap is unknown
         * Assume only prvCpssDrvHwPpReadInternalPciReg() will be used till
         * correct value of doByteSwap known */
        rc = prvCpssDrvHwCntlInit(devNum,portGroupId,
                         &(ppConfigPtr->hwInfo[portGroupId]),
                         GT_FALSE);/* isDiag */
        if (rc!= GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

        /* Read from the PCI channel */
        if(ppInInfoPtr->ppHAState == CPSS_SYS_HA_MODE_ACTIVE_E)
        {
            switch(ppInInfoPtr->devIdLocationType)
            {
                case PRV_CPSS_DRV_DEV_ID_LOCATION_PCI_CONFIG_CYCLE_ADDR_0x00000000_E:
                case PRV_CPSS_DRV_DEV_ID_LOCATION_PEX_CONFIGURATION_HEADER_ADDR_0X00070000_E:
                    if(ppInInfoPtr->devIdLocationType ==
                        PRV_CPSS_DRV_DEV_ID_LOCATION_PEX_CONFIGURATION_HEADER_ADDR_0X00070000_E)
                    {
                        address = 0x70000;
                    }
                    else
                    {
                        address = 0;
                    }

                    /* perform the direct PCI/PEX memory read access */
                    rc = prvCpssDrvHwPpPortGroupReadInternalPciReg(devNum, portGroupId,address, &tempDevId);
                    if (rc!= GT_OK)
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

                    if((tempDevId & 0xFFFF) != MARVELL_VENDOR_ID)
                    {
                        /* not expected value for the register */
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
                    }
                    break;

                case PRV_CPSS_DRV_DEV_ID_LOCATION_DEVICE_ADDR_0x0000004C_E:
                /* try to read register 0x50 to get the 0x11AB (MARVELL_VENDOR_ID) */

                    /* read VendorID */
                    rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId,VENDOR_ID_REG_ADDR, &readVendorId) ;
                    if (rc!= GT_OK)
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

                    if((readVendorId & 0xFFFF) != MARVELL_VENDOR_ID)
                    {
                        /* not expected value for the register */
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, "Expected 'MARVELL VENDOR ID' [0x11AB] in register[0x%8.8x] but got [0x%4.4x]",
                            VENDOR_ID_REG_ADDR,
                            (readVendorId & 0xFFFF));
                    }

                    if(useDfxServerForDeviceId == 0)
                    {
                        /* read DeviceID from MG register */
                        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, DEVICE_ID_AND_REV_REG_ADDR_CNS,
                                                                4,16, &readDeviceId);
                    }
                    else
                    {
                        /* read DeviceID from DFX Server register */
                        rc = prvCpssDrvHwPpResetAndInitControllerGetRegField(devNum,DFX_DEVICE_ID_AND_REV_REG_ADDR_CNS,
                                0,16,&readDeviceId);
                    }

                    if (rc!= GT_OK)
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

                    tempDevId = (readDeviceId << 16) | readVendorId ;

                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }/*switch(ppInInfoPtr->devIdLocationType) */

            if(PRV_CPSS_DRV_ERRATA_GET_MAC(devNum,PRV_CPSS_DRV_ERRATA_PCI_SLAVE_UNABLE_TO_ABORT_TRANSACTION_E) == GT_TRUE)
            {
                /* Initialize the PCI Timer and Retry register with 0xFF.   */
                /* Perform a read modify write on the register.             */
                rc = prvCpssDrvHwPpPortGroupReadInternalPciReg(devNum,portGroupId,0x104,&regData);
                if (rc!= GT_OK)
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

                regData = (regData & 0xFFFF0000) | 0xFFFF;  /*set both timer values to maximum */

                rc = prvCpssDrvHwPpPortGroupWriteInternalPciReg(devNum,portGroupId,0x104,regData);
                if (rc!= GT_OK)
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }
        /* now doByteSwap has correct value */
    }/* PCI/PEX compatible */

    if(ppInInfoPtr->ppHAState == CPSS_SYS_HA_MODE_STANDBY_E)
    {
        /* get the info that the high availability holds for that device */
        tempDevId  = sysGenGlobalInfo.cpssPpHaDevData[devNum]->deviceId;
    }

    /* device type was stored in the internal DB */
    if (prvCpssDrvDebugDeviceId[devNum] != 0x0)
    {
        tempDevId = ((prvCpssDrvDebugDeviceId[devNum] << 16) | MARVELL_VENDOR_ID);
    }

#ifdef CHX_FAMILY
#ifndef GM_USED /* BC2 GM does not support DFX registers */
    /* devFamily isn't defined yet */
    if(((tempDevId>>16)&0xFC00) == 0xFC00)
    {
        rc = prvCpssDrvHwPpResetAndInitControllerGetRegField(devNum,
                                                             DFX_DEVICE_ID_AND_REV_REG_ADDR_CNS,
                                                             0,
                                                             16,
                                                             &regData);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        tempDevId |= (regData << 16);

    }
#endif /* GM_USED */

    if(((tempDevId>>16) == 0xEC00) || ((tempDevId>>16) == 0x8000))
    {
        if(1 <= portGroupId)
        {/* In current revision of Lion2/Hooper Register 0x4C is always 0xEC00/0x8000.
            Read DFX S@R register bits [24-21] to get proper device type,
            DFX Unit exists only in port group 1 */
            if(GT_OK != prvCpssDrvHwPpPortGroupGetRegField(devNum, 1,
                                                            0x18F8200, 21, 4, &regData))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
            }
            else
            {
                tempDevId |= (regData << 16);
            }
        }

        if((tempDevId>>20) != 0x800)
        {/* run only for Lion2 A0 */ /* Lion2 Hooper has also revision 0 - but WA not needed */
            /* RevisionID read */
            prvCpssDrvHwPpGetRegField(devNum,
                            DEVICE_ID_AND_REV_REG_ADDR_CNS, 0, 4, &regData);
            if(0 == regData)
            {
                rc = prvCpssDrvLion2FirstPacketWA(devNum, portGroupId, ppInInfoPtr);
                if(rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }
            }
        }
    }
#endif

    /* temporary setting that may change if the CPSS decide to address this
       device with other device type ! */
    ppConfigPtr->devType = tempDevId;

    return GT_OK;
}


/**
* @internal deviceTypeInfoGet function
* @endinternal
*
* @brief   This function sets the device info :
*         number of ports ,
*         deviceType ,
*         and return bmp of ports , for a given device.
* @param[in] devNum                   - The Pp's device number.
*
* @param[out] existingPortsPtr         - (pointer to)the BMP of existing ports
*                                       GT_OK on success,
*
* @retval GT_BAD_STATE             - the 'multi port groups' support of the device has differences
*                                       between the application and the cpssDrv DB
*                                       GT_NOT_FOUND if the given pciDevType is illegal.
* @retval GT_NOT_SUPPORTED         - not properly supported device in DB
*/
static GT_STATUS deviceTypeInfoGet
(
    IN  GT_U8       devNum,
    OUT CPSS_PORTS_BMP_STC  *existingPortsPtr
)
{
    GT_STATUS   rc;/* return code*/
    GT_U32   ii;/* index in cpssSupportedTypes */
    GT_U32   jj;/* index in cpssSupportedTypes[ii].devTypeArray */
    GT_U32   kk;/* index in cpssSpecialDevicesBmp */
    GT_BOOL found = GT_FALSE;
    PRV_CPSS_DRIVER_PP_CONFIG_STC *ppConfigPtr;/* Points to the Device's Driver*/
    GT_U32  port;/*port iterator*/
    GT_U32  portInCurrentPortGroup;/*port number in current port group*/
    CPSS_PORTS_BMP_STC portsInPortGroupBmp;/*bitmap of the local ports of the port group */
    CPSS_PORTS_BMP_STC fullPortsBmp; /* full ports bmp of this device */
    GT_BOOL            needToForceOutPorts = GT_FALSE;/* indication if ports need to be forced out of the 'existing ports' */
    CPSS_PORTS_BMP_STC forcedOutPortsBmp;/* bmp of ports to be forced out of the 'existing ports' */
    const CPSS_SUPPORTED_TYPES_STC *cpssSupportedTypesPtr;/* pointer to list of supported devices*/
    const DEFAULT_REG_STC *onEmulator_defaultRegPtr = NULL;/*pointer to default registers for 'Emulator' uses */
    const DEFAULT_REG_FORMULA_STC *onEmulator_defaultReg_formulaPtr = NULL;/*pointer to default formula registers for 'Emulator' uses */

    cpssSupportedTypesPtr = cpssSupportedTypes;

    if(cpssDeviceRunCheck_onEmulator())
    {
        cpssSupportedTypesPtr = (const CPSS_SUPPORTED_TYPES_STC *)onEmulator_cpssSupportedTypes;
        if(cpssDeviceRunCheck_onEmulator_isAldrinFull())
        {
            /* get the info about our device */
            ii = 0;
            while(onEmulator_cpssSupportedTypes[ii].devFamily != END_OF_TABLE)
            {
                if(onEmulator_cpssSupportedTypes[ii].devFamily == ALDRIN_CNS &&
                   onEmulator_cpssSupportedTypes[ii].defaultPortsBmpPtr)
                {
                    /* modify to limit no ports */
                    onEmulator_cpssSupportedTypes[ii].defaultPortsBmpPtr = NULL;

                    break;
                }
                ii++;
            }
        }
    }

    ppConfigPtr = prvCpssDrvPpConfig[devNum];

    /* get the info about our device */
    ii = 0;
    while(cpssSupportedTypesPtr[ii].devFamily != END_OF_TABLE)
    {
        jj = 0;
        while(cpssSupportedTypesPtr[ii].devTypeArray[jj] != END_OF_TABLE)
        {
            if(ppConfigPtr->devType == cpssSupportedTypesPtr[ii].devTypeArray[jj])
            {
                found = GT_TRUE;
                break;
            }
            jj++;
        }

        if(found == GT_TRUE)
        {
            break;
        }
        ii++;
    }

    if(found == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, "devType[0x%4.4x] not exists in 'Supported devices' list",
            ppConfigPtr->devType);
    }

    if(cpssDeviceRunCheck_onEmulator())
    {
        /* on Emulator we may need to init some registers for proper behavior */
        switch(cpssSupportedTypesPtr[ii].devFamily)
        {
            case ALDRIN_CNS:
                if(cpssDeviceRunCheck_onEmulator_isAldrinFull())
                {
                    onEmulator_defaultRegPtr = onEmulator_AldrinDefaultRegArr_full;
                    onEmulator_defaultReg_formulaPtr = onEmulator_AldrinDefaultRegArr_full_formula;
                }
                else
                {
                    onEmulator_defaultRegPtr = onEmulator_AldrinDefaultRegArr;
                }
                break;
            case ALDRIN2_CNS:
                onEmulator_defaultRegPtr = onEmulator_Aldrin2DefaultRegArr;
                onEmulator_defaultReg_formulaPtr = onEmulator_Aldrin2DefaultRegArr_formula;
                break;
            case PIPE_CNS:
                onEmulator_defaultRegPtr = onEmulator_PipeDefaultRegArr;
                onEmulator_defaultReg_formulaPtr = onEmulator_PipeDefaultRegArr_formula;
                break;
            case FALCON_CNS:
                onEmulator_defaultRegPtr = onEmulator_FalconDefaultRegArr;
                onEmulator_defaultReg_formulaPtr = onEmulator_FalconDefaultRegArr_formula;
                break;
            default:
                break;
        }
    }

    if(onEmulator_defaultRegPtr)
    {
        for(jj = 0 ; onEmulator_defaultRegPtr[jj].regAddr != END_OF_TABLE ; jj++)
        {
            rc = prvCpssDrvHwPpWriteRegister(devNum,
                onEmulator_defaultRegPtr[jj].regAddr,
                onEmulator_defaultRegPtr[jj].regValue);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    if(onEmulator_defaultReg_formulaPtr)
    {
        /* set formula of addresses */
        for(jj = 0 ; onEmulator_defaultReg_formulaPtr[jj].baseRegAddr != END_OF_TABLE ; jj++)
        {
            for(kk = onEmulator_defaultReg_formulaPtr[jj].startStepIndex ;
                kk < (onEmulator_defaultReg_formulaPtr[jj].startStepIndex +
                      onEmulator_defaultReg_formulaPtr[jj].numOfSteps);
                kk++)
            {
                rc = prvCpssDrvHwPpWriteRegister(devNum,
                    (onEmulator_defaultReg_formulaPtr[jj].baseRegAddr +
                     onEmulator_defaultReg_formulaPtr[jj].stepsInBytes * kk) ,
                    onEmulator_defaultReg_formulaPtr[jj].regValue);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }


    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&forcedOutPortsBmp);

    /* do 'multi port groups' device info check */
    if(cpssSupportedTypesPtr[ii].portGroupInfoPtr)
    {
        /*************************************************/
        /* the recognized device is 'multi port groups' device */
        /*************************************************/

        /* check that application stated that the device is 'multi port groups' device */
        if(prvCpssDrvPpConfig[devNum]->portGroupsInfo.isMultiPortGroupDevice == GT_FALSE)
        {
            /* application not aware to the 'multi port groups' device ?! */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        if(prvCpssDrvPpConfig[devNum]->portGroupsInfo.numOfPortGroups >
            cpssSupportedTypesPtr[ii].portGroupInfoPtr->numOfPortGroups)
        {
            /* application think more port groups then really exists */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        if(cpssSupportedTypesPtr[ii].activePortGroupsBmpPtr == NULL)
        {
            /* the DB of the device not hold the active port groups bitmap */
            /* internal DB error ! */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }


        if(*cpssSupportedTypesPtr[ii].activePortGroupsBmpPtr !=
           prvCpssDrvPpConfig[devNum]->portGroupsInfo.activePortGroupsBmp)
        {
            if(prvCpssDrvPpConfig[devNum]->portGroupsInfo.activePortGroupsBmp == 0)
            {
                /* the device must have at least one active port group */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            /* application allowed to force CPSS to not access existing port group.
               (this port group may exist in the device but not connected by the PEX) */
            for(kk = 0 ; kk < cpssSupportedTypesPtr[ii].portGroupInfoPtr->numOfPortGroups; kk++)
            {
                if((0 == ((*cpssSupportedTypesPtr[ii].activePortGroupsBmpPtr) & (1 << kk))) &&
                   (     prvCpssDrvPpConfig[devNum]->portGroupsInfo.activePortGroupsBmp & (1 << kk)))
                {
                    /* application think that this is active port group but it is not */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                }

                if((     ((*cpssSupportedTypesPtr[ii].activePortGroupsBmpPtr) & (1 << kk))) &&
                   (0 == (prvCpssDrvPpConfig[devNum]->portGroupsInfo.activePortGroupsBmp & (1 << kk))))
                {
                    /* application force CPSS to not access existing port group.
                       (this port group may exist in the device but not connected by the PEX) */
                    needToForceOutPorts = GT_TRUE;

                    /* update the bmp of 'forced out ports' */
                    forcedOutPortsBmp.ports[kk >> 1] |= (0xFFFF << ((kk & 1) * 16));
                }
            }
        }
    }
    else if(prvCpssDrvPpConfig[devNum]->portGroupsInfo.isMultiPortGroupDevice == GT_TRUE)
    {
        /* application stated that this is 'multi port groups' device */
        /* but according to DB this is 'non multi port groups' device */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    ppConfigPtr->numOfPorts = GET_NUM_OF_PORTS(cpssSupportedTypesPtr[ii].numOfPorts);
    ppConfigPtr->numOfTiles = GET_NUM_OF_TILES(cpssSupportedTypesPtr[ii].numOfPorts);
    ppConfigPtr->devFamily = cpssSupportedTypesPtr[ii].devFamily;

    if(debug_force_numOfPorts)
    {
        /* allow to test GM limitations on simulation :
           BC3 the GM supports only 13 ports and not full 74 ports
        */
        ppConfigPtr->numOfPorts = debug_force_numOfPorts;
    }

    if(cpssSupportedTypesPtr[ii].defaultPortsBmpPtr == NULL)
    {
        if(cpssSupportedTypesPtr[ii].portGroupInfoPtr == NULL)
        {
            /* no special default for the ports BMP , use continuous ports bmp */
            prvCpssDrvPortsFullMaskGet(ppConfigPtr->numOfPorts,existingPortsPtr);
        }
        else
        {
            /* 'multi port groups' device */
            /* by default we build it's bmp according to the port groups info */

            portInCurrentPortGroup = 0;
            CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(existingPortsPtr);
            CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsInPortGroupBmp);

            portsInPortGroupBmp.ports[0] = cpssSupportedTypesPtr[ii].portGroupInfoPtr->portsBmp;

            for(port = 0 ; port < ppConfigPtr->numOfPorts ; port++,portInCurrentPortGroup++)
            {
                if(portInCurrentPortGroup == cpssSupportedTypesPtr[ii].portGroupInfoPtr->maxNumPorts)
                {
                    portInCurrentPortGroup = 0;
                }

                if(ppConfigPtr->hwInfo[((port >> 4) & (CPSS_MAX_PORT_GROUPS_CNS - 1))/*portGroupId*/].busType == CPSS_HW_INFO_BUS_TYPE_NONE_E)
                {
                    /* non active port group */
                    continue;
                }

                if(CPSS_PORTS_BMP_IS_PORT_SET_MAC((&portsInPortGroupBmp),portInCurrentPortGroup))
                {
                    CPSS_PORTS_BMP_PORT_SET_MAC(existingPortsPtr,port);
                }
            }
        }
    }
    else
    {
        /* use special default for the ports BMP */
        *existingPortsPtr = *cpssSupportedTypesPtr[ii].defaultPortsBmpPtr;
    }

    /* Aldrin2 - special ports map */
    if (cpssSupportedTypesPtr[ii].devFamily == ALDRIN2_CNS)
    {
        if (ppConfigPtr->devType == CPSS_98DX8410_CNS)
        {
            ppConfigPtr->numOfPorts = 72;
            *existingPortsPtr = portsBmp48_special_map;
        }
    }

    /****************************************************************/
    /* add here specific devices BMP of ports that are not standard */
    /****************************************************************/
    kk = 0;
    while(cpssSpecialDevicesBmp[kk].devType != END_OF_TABLE)
    {
        if(cpssSpecialDevicesBmp[kk].devType == ppConfigPtr->devType)
        {
            *existingPortsPtr = cpssSpecialDevicesBmp[kk].existingPorts;
            break;
        }
        kk++;
    }

    /* clear from the existing ports the ports beyond the 'Number of ports' */
    prvCpssDrvPortsFullMaskGet(ppConfigPtr->numOfPorts,&fullPortsBmp);

    CPSS_PORTS_BMP_BITWISE_AND_MAC(existingPortsPtr,existingPortsPtr,&fullPortsBmp);

    if(needToForceOutPorts == GT_TRUE)
    {
        /* remove the 'forced out ports' from the 'existing ports bmp'*/
        CPSS_PORTS_BMP_BITWISE_AND_NOT_MAC(existingPortsPtr,existingPortsPtr,&forcedOutPortsBmp);
    }


    return GT_OK;
}

/**
* @internal sdmaSwapModeAndDeviceRevisionGet function
* @endinternal
*
* @brief   This function is called by prvCpssDrvHwPpPhase1Init()
*         This function do PCI and DMA calibration .
*         and read the 'revision' of the device.
* @param[in] devNum                   - The PP's device number to be initialized.
* @param[in] ppInInfoPtr              - (pointer to)   the info needed for initialization of
*                                      the driver for this PP
*
* @param[out] revisionIdPtr            - (pointer to)the revision of the PP
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS sdmaSwapModeAndDeviceRevisionGet
(
    IN  GT_U8       devNum,
    IN  PRV_CPSS_DRV_PP_PHASE_1_INPUT_INFO_STC  *ppInInfoPtr,
    OUT GT_U8       *revisionIdPtr
)
{
    GT_U32  tempData = 0;/* data from the register */
    GT_U32 ii;
#ifndef GM_USED
    PRV_CPSS_DRIVER_PP_CONFIG_STC *ppConfigPtr;/* Points to the Device's Driver*/
    ppConfigPtr = prvCpssDrvPpConfig[devNum];
#endif

    /* Set the Rx / Tx glue byte swapping configuration     */
    if (PRV_CPSS_DRV_HW_IF_PCI_COMPATIBLE_MAC(devNum))
    {
        /* SDMA LE/BE should works the same since the CPU get the data as raw
           byte */
        /* Enable byte swap, Enable word swap  */

        prvCpssDrvHwPpSetRegField(devNum, SDMA_CFG_REG_ADDR, 6, 2, 3);
        prvCpssDrvHwPpSetRegField(devNum,SDMA_CFG_REG_ADDR,23,2,0);

        if(allowMultiMgSdmaInit)
        {
            for(ii = 0; ii < (CPSS_4_SDMA_CPU_PORTS_CNS-1); ii++)
            {
                prvCpssDrvHwPpResourceSetRegField(devNum,0,CPSS_DRV_HW_RESOURCE_MG1_CORE_E +ii,SDMA_CFG_REG_ADDR,6,2,3);
                prvCpssDrvHwPpResourceSetRegField(devNum,0,CPSS_DRV_HW_RESOURCE_MG1_CORE_E +ii,SDMA_CFG_REG_ADDR,23,2,0);
            }
        }

#ifdef ASIC_SIMULATION
        /*
            this section is needed for the distributed simulation.
            when the read/write DMA from Asic are done via socket.

            and because both descriptors and frames are on the DMA memory,
            we need to set the way that Device put frame on the socket.

            because the DMA parser on the CPU side will set the data from the
            socket the same way for descriptors (that are little endian format)
            and the frames .
        */
        if(sasicgSimulationRoleIsApplication == GT_TRUE)
        {
            GT_U32  swapBytes;
            /* override the setting of SDMA swap/not swap bytes */
            if(BIG_ENDIAN_E == checkHostEndian())
            {
                /* when PP read/write frame form/to DMA it should do it non-swapped
                    I.E the frame should be placed on socket 'Big endean'
                 */
                swapBytes = 0;
            }
            else
            {
                /* when PP read/write frame form/to DMA it should do it swapped
                    I.E the frame should be placed on socket 'Little endean'
                 */
                swapBytes = 1;
            }

            prvCpssDrvHwPpSetRegField(devNum,SDMA_CFG_REG_ADDR, 6,1,swapBytes);
            prvCpssDrvHwPpSetRegField(devNum,SDMA_CFG_REG_ADDR,23,1,swapBytes);

            if(allowMultiMgSdmaInit)
            {
                for(ii = 0; ii < (CPSS_4_SDMA_CPU_PORTS_CNS-1); ii++)
                {
                    prvCpssDrvHwPpResourceSetRegField(devNum,0,CPSS_DRV_HW_RESOURCE_MG1_CORE_E +ii,SDMA_CFG_REG_ADDR,6,1,swapBytes);
                    prvCpssDrvHwPpResourceSetRegField(devNum,0,CPSS_DRV_HW_RESOURCE_MG1_CORE_E +ii,SDMA_CFG_REG_ADDR,23,1,swapBytes);
                }
            }
        }
#endif /*ASIC_SIMULATION*/
    }

    /* get the revision id of the PP */
    /* RevisionID read */
    prvCpssDrvHwPpGetRegField(devNum,
                    DEVICE_ID_AND_REV_REG_ADDR_CNS, 0, 4, &tempData) ;

/* The GM not supports the DFX server registers */
#ifndef GM_USED
    if(ppConfigPtr->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E  ||
       ppConfigPtr->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E   ||
       ppConfigPtr->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E     ||
       ppConfigPtr->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E  ||
       ppConfigPtr->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E  ||
       ppConfigPtr->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
    {
        if(prvCpssDrvHwPpResetAndInitControllerGetRegField(devNum,
                                                             DFX_JTAG_DEVICE_ID_AND_REV_REG_ADDR_CNS,
                                                             28,
                                                             4,
                                                             &tempData) != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
        }
    }
#endif /*ASIC_SIMULATION*/

    if(ppInInfoPtr->ppHAState == CPSS_SYS_HA_MODE_ACTIVE_E)
    {
        *revisionIdPtr = (GT_U8)tempData;
    }
    else
    {
        *revisionIdPtr = sysGenGlobalInfo.cpssPpHaDevData[devNum]->revision;
    }

    return GT_OK;
}
/**
* @internal driverDxExMxHwPpPhase1Init function
* @endinternal
*
* @brief   This function is called by cpss "phase 1" device family functions,
*         in order to enable PP Hw access,
*         the device number provided to this function may
*         be changed when calling prvCpssDrvPpHwPhase2Init().
* @param[in] devNum                   - The PP's device number to be initialized.
* @param[in] ppInInfoPtr              - (pointer to)   the info needed for initialization of
*                                      the driver for this PP
*
* @param[out] ppOutInfoPtr             - (pointer to)the info that driver return to caller.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_OUT_OF_CPU_MEM        - failed to allocate CPU memory,
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS driverDxExMxHwPpPhase1Init
(
    IN  GT_U8                devNum,
    IN  PRV_CPSS_DRV_PP_PHASE_1_INPUT_INFO_STC  *ppInInfoPtr,
    OUT PRV_CPSS_DRV_PP_PHASE_1_OUTPUT_INFO_STC *ppOutInfoPtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_DRIVER_PP_CONFIG_STC *ppConfigPtr;/* Points to the Device's Driver*/
    GT_U32      portGroupId;/* the port group Id */
    GT_U32      dummyReg;/* dummy reg address */

    /* the function bind the PCI/SMI/TWSI driver functions to the driver object
       functions for this device.*/
    rc = phase1Part1Init(devNum,ppInInfoPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* NOTE :
        since the function of phase1Part2Init(...) will allocate dynamic memory
        we need to free it and restore NULL state if error occur in this current
        function driverDxExMxHwPpPhase1Init(...) after calling
        phase1Part2Init(...)

        see the use of label exit_cleanly_lbl
    */


   /* This function allocate the memory for the DB of the driver for this
      device , and set some of it's values.*/
    rc = phase1Part2Init(devNum,ppInInfoPtr);
    if(rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    if (CPSS_CHANNEL_PCI_E == ppInInfoPtr->mngInterfaceType)
    {
        /* relevant only for PCI devices */
        /* all PCI devices need it */
        PRV_CPSS_DRV_ERRATA_SET_MAC(
            devNum,
            PRV_CPSS_DRV_ERRATA_PCI_SLAVE_UNABLE_TO_ABORT_TRANSACTION_E);
    }


    ppConfigPtr = prvCpssDrvPpConfig[devNum];

    ppConfigPtr->portGroupsInfo.isMultiPortGroupDevice =
        (ppInInfoPtr->numOfPortGroups == 1)
            ? GT_FALSE : GT_TRUE;

    for(portGroupId = 0; portGroupId < ppInInfoPtr->numOfPortGroups; portGroupId++)
    {
        if(ppConfigPtr->hwInfo[portGroupId].busType == CPSS_HW_INFO_BUS_TYPE_NONE_E)
        {
            /* non active port group */
            continue;
        }

        /* This function call driverHwCntlInit to initialize the HW control.
           The function get the deviceType from HW , doByteSwap */
        rc = driverHwCntlInitAndDeviceIdGet(devNum,portGroupId,ppInInfoPtr);
        if(rc != GT_OK)
        {
            goto exit_cleanly_lbl;
        }
    }

    /* starting from this point the DB of : prvCpssDrvPpConfig[devNum]->portGroupsInfo
       is ready because initialized in the prvCpssDrvHwCntlInit(..) that called
       from phase1Part3Init(..)

       so we can start use the macros for loops on port groups (supports also 'non multi port group' device):
       PRV_CPSS_DRV_START_LOOP_PORT_GROUPS_MAC
       PRV_CPSS_DRV_END_LOOP_PORT_GROUPS_MAC
    */


    /* This function sets the device info : number of ports , deviceType ,
      and return bmp of ports , for a given device. */
    rc = deviceTypeInfoGet(devNum,
                           &ppOutInfoPtr->existingPorts);
    if(rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    ppOutInfoPtr->devType = ppConfigPtr->devType;
    ppOutInfoPtr->numOfPorts = ppConfigPtr->numOfPorts;
    ppOutInfoPtr->numOfTiles = ppConfigPtr->numOfTiles;
    ppOutInfoPtr->devFamily = ppConfigPtr->devFamily;

    /*only summary interrupts are unmasked by default */
    /* according to the interrupt scan tree. */
    ppConfigPtr->intCtrl.needInterruptMaskDefaultValueInit = GT_TRUE;
            /* bind the ISR callback routine done before signalling app. */
    ppConfigPtr->intCtrl.bindTheIsrCbRoutineToFdbTrigEnded = GT_FALSE;

    if (((ppOutInfoPtr->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E) ||
         (ppOutInfoPtr->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E) ||
         /*(ppOutInfoPtr->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E) ||*/
         (ppConfigPtr->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E) ||
         (ppOutInfoPtr->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E) ||
        (ppOutInfoPtr->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E) ||
         (ppOutInfoPtr->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)||
         CPSS_IS_PX_FAMILY_MAC(ppOutInfoPtr->devFamily))
        && !CPSS_CHANNEL_IS_PEX_MBUS_MAC(ppInInfoPtr->mngInterfaceType))
    {
        /* Set address completion for backward compatible */
        rc = prvCpssDrvHwPpSetRegField(devNum, 0x140, 16, 1, 1);
        if(rc != GT_OK)
        {
            goto exit_cleanly_lbl;
        }
    }

    for(portGroupId = 0; portGroupId < ppInInfoPtr->numOfPortGroups; portGroupId++)
    {
        prvCpssDrvPpConfig[devNum]->hwCtrl[portGroupId].doReadAfterWrite =
            PRV_CPSS_DRV_ERRATA_GET_MAC(devNum,PRV_CPSS_DRV_ERRATA_PCI_READ_AFTER_WRITE_E);
    }

    if (ppOutInfoPtr->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
    {
#ifdef PRV_CPSS_DRV_ERRATA_REFERENCE_CNS  /* macro never defined --> for references purpose only  */
    PRV_CPSS_DXCH_LION_RM_SDMA_ACTIVATION_WA_E /*Lion RM#2701*/
#endif /*PRV_CPSS_DRV_ERRATA_REFERENCE_CNS*/
        /* Lion RM#2701: SDMA activation */
        /* the code must be before calling sdmaSwapModeAndDeviceRevisionGet(...) because attempt
           to access register 0x2800 will cause the PEX to hang */
        rc = prvCpssDrvHwPpSetRegField(devNum,0x58,20,1,1);
        if(rc != GT_OK)
        {
            goto exit_cleanly_lbl;
        }
    }

    /* to SDMA swapping issues and get the revision of the PP
       before init of interrupts , as the interrupts may need info
       about the 'revision' of the PP */
    rc = sdmaSwapModeAndDeviceRevisionGet(devNum,ppInInfoPtr,&ppOutInfoPtr->revision);
    if(rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    switch(ppOutInfoPtr->devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_XCAT3_E:
            if(ppOutInfoPtr->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
            {
            #ifdef CHX_FAMILY
                /*Interrupts initialization for the xCat3 devices.*/
                prvCpssDrvPpIntDefDxChXcat3Init(devNum);
            #else
                rc = GT_NOT_SUPPORTED;
                goto exit_cleanly_lbl;
            #endif
            }
            /* no break ... */

        case CPSS_PP_FAMILY_DXCH_LION2_E:
            if(ppOutInfoPtr->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
            {
            #ifdef CHX_FAMILY
                /*Interrupts initialization for the Lion2 devices.*/
                prvCpssDrvPpIntDefDxChLion2Init(devNum);
            #else
                rc = GT_NOT_SUPPORTED;
                goto exit_cleanly_lbl;
            #endif
            }

            /*only summary interrupts are unmasked by default */
            /* according to the interrupt scan tree. */
            ppConfigPtr->intCtrl.needInterruptMaskDefaultValueInit = GT_FALSE;

            break;

        case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
            switch(ppOutInfoPtr->devType)
            {
                case CPSS_BOBK_ALL_DEVICES_CASES_MAC:
                    /* indicate the sub family */
                    ppConfigPtr->devSubFamily = CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E;
                    break;
                default:
                    break;
            }
            #ifdef CHX_FAMILY

#ifdef GM_USED
                /* Interrupts initialization for Bobcat2 devices */
                rc = prvCpssDrvPpIntDefDxChBobcat2Init__GM(devNum); /* in Lion2 file */
#else
                if (ppConfigPtr->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E)
                {
                    rc = prvCpssDrvPpIntDefDxChBobKInit(devNum, ppOutInfoPtr->revision);
                }
                else
                {
                    rc = prvCpssDrvPpIntDefDxChBobcat2Init(devNum, ppOutInfoPtr->revision); /* in BC2 file */
                }
#endif
                if(rc != GT_OK)
                {
                    goto exit_cleanly_lbl;
                }

                /*only summary interrupts are unmasked by default */
                /* according to the interrupt scan tree. */
                ppConfigPtr->intCtrl.needInterruptMaskDefaultValueInit = GT_FALSE;
                break;
            #else
                rc = GT_NOT_SUPPORTED;
                goto exit_cleanly_lbl;
            #endif

        case CPSS_PP_FAMILY_DXCH_BOBCAT3_E:
            #ifdef CHX_FAMILY
#ifdef GM_USED
                /* Interrupts initialization for Bobcat2 devices */
                rc = prvCpssDrvPpIntDefDxChBobcat2Init__GM(devNum); /* in Lion2 file */
#else /*!GM_USED*/
                rc = prvCpssDrvPpIntDefDxChBobcat3Init(devNum,ppOutInfoPtr->revision);
#endif /*!GM_USED*/
                if(rc != GT_OK)
                {
                    goto exit_cleanly_lbl;
                }

                /*only summary interrupts are unmasked by default */
                /* according to the interrupt scan tree. */
                ppConfigPtr->intCtrl.needInterruptMaskDefaultValueInit = GT_FALSE;
                break;
            #else
                rc = GT_NOT_SUPPORTED;
                goto exit_cleanly_lbl;
            #endif
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
        case CPSS_PP_FAMILY_DXCH_AC3X_E:
            #ifdef CHX_FAMILY
#ifdef GM_USED
                /* Interrupts initialization for Bobcat2 devices */
                rc = prvCpssDrvPpIntDefDxChBobcat2Init__GM(devNum); /* in Lion2 file */
#else
                rc = prvCpssDrvPpIntDefDxChAldrinInit(devNum, ppOutInfoPtr->revision);
#endif
                if(rc != GT_OK)
                {
                    goto exit_cleanly_lbl;
                }

                /*only summary interrupts are unmasked by default */
                /* according to the interrupt scan tree. */
                ppConfigPtr->intCtrl.needInterruptMaskDefaultValueInit = GT_FALSE;
                break;
            #else
                rc = GT_NOT_SUPPORTED;
                goto exit_cleanly_lbl;
            #endif
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            #ifdef CHX_FAMILY
#ifdef GM_USED
                /* Interrupts initialization for Bobcat2 devices */
                rc = prvCpssDrvPpIntDefDxChBobcat2Init__GM(devNum); /* in Lion2 file */
#else /*!GM_USED*/
                rc = prvCpssDrvPpIntDefDxChAldrin2Init(devNum,ppOutInfoPtr->revision);
#endif /*!GM_USED*/
                if(rc != GT_OK)
                {
                    goto exit_cleanly_lbl;
                }

                /*only summary interrupts are unmasked by default */
                /* according to the interrupt scan tree. */
                ppConfigPtr->intCtrl.needInterruptMaskDefaultValueInit = GT_FALSE;
                break;
            #else
                rc = GT_NOT_SUPPORTED;
                goto exit_cleanly_lbl;
            #endif

        case CPSS_PX_FAMILY_PIPE_E:
            #ifdef ALLOW_PX_CODE
                rc = prvCpssDrvPpIntDefPxPipeInit(devNum, ppOutInfoPtr->revision);
                if(rc != GT_OK)
                {
                    goto exit_cleanly_lbl;
                }
                break;
            #else
                rc = GT_NOT_SUPPORTED;
                goto exit_cleanly_lbl;
            #endif
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            #ifdef CHX_FAMILY
#ifdef GM_USED
                /* Interrupts initialization for Bobcat2 devices */
                rc = prvCpssDrvPpIntDefDxChBobcat2Init__GM(devNum); /* in Lion2 file */
#else /*!GM_USED*/
                rc = prvCpssDrvPpIntDefDxChFalconInit(devNum,ppOutInfoPtr->revision);
#endif /*!GM_USED*/
                if(rc != GT_OK)
                {
                    goto exit_cleanly_lbl;
                }

                /*only summary interrupts are unmasked by default */
                /* according to the interrupt scan tree. */
                ppConfigPtr->intCtrl.needInterruptMaskDefaultValueInit = GT_FALSE;
                break;
            #else
                rc = GT_NOT_SUPPORTED;
                goto exit_cleanly_lbl;
            #endif
        default:
            rc = GT_NOT_SUPPORTED;
            goto exit_cleanly_lbl;
    }
    /* save the sub family */
    ppOutInfoPtr->devSubFamily = ppConfigPtr->devSubFamily;

    /* bind the ISR callback routine done before signalling app. */
    rc = prvCpssDrvExMxDxHwPpPexAddrGet(ppOutInfoPtr->devFamily,&dummyReg,&ppConfigPtr->intCtrl.intSumMaskRegAddr);
    if(rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

exit_cleanly_lbl:
    if(rc != GT_OK)
    {
        if(rc != GT_BAD_STATE && /* another device is in this memory --
                                    don't free the memory */
           prvCpssDrvPpConfig[devNum])
        {
            /* release the allocation */
            cpssOsFree(prvCpssDrvPpConfig[devNum]);
            prvCpssDrvPpConfig[devNum] = NULL;
        }
    }

    return rc;
}

/**
* @internal driverDxExMxHwPpRamBurstConfigSet function
* @endinternal
*
* @brief   Sets the Ram burst information for a given device.
*
* @param[in] devNum                   - The Pp's device number.
* @param[in] ramBurstInfoPtr          - A list of Ram burst information for this device.
* @param[in] burstInfoLen             - Number of valid entries in ramBurstInfo.
*
* @retval GT_OK                    - on success,
* @retval GT_OUT_OF_CPU_MEM        - failed to allocate CPU memory,
* @retval GT_ALREADY_EXIST         - the ram already initialized
*
* @note function also bound to the "pci-standby" for HA
*
*/
static GT_STATUS driverDxExMxHwPpRamBurstConfigSet
(
    IN  GT_U8               devNum,
    IN  PRV_CPSS_DRV_RAM_BURST_INFO_STC   *ramBurstInfoPtr,
    IN  GT_U8               burstInfoLen
)
{
    GT_U32  portGroupId;

    if(CPSS_IS_DXCH_FAMILY_MAC(prvCpssDrvPpConfig[devNum]->devFamily) ||
       CPSS_IS_PX_FAMILY_MAC(prvCpssDrvPpConfig[devNum]->devFamily))
    {
        /* no more to do */
        return GT_OK;
    }

    PRV_CPSS_DRV_START_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
    {
        if(prvCpssDrvPpConfig[devNum]->hwCtrl[portGroupId].ramBurstInfo != NULL ||
           prvCpssDrvPpConfig[devNum]->hwCtrl[portGroupId].ramBurstInfoLen != 0 )
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        prvCpssDrvPpConfig[devNum]->hwCtrl[portGroupId].ramBurstInfo =
            cpssOsMalloc(sizeof(PRV_CPSS_DRV_RAM_BURST_INFO_STC)* burstInfoLen);
        if(prvCpssDrvPpConfig[devNum]->hwCtrl[portGroupId].ramBurstInfo == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }

        cpssOsMemCpy(prvCpssDrvPpConfig[devNum]->hwCtrl[portGroupId].ramBurstInfo,ramBurstInfoPtr,
                 sizeof(PRV_CPSS_DRV_RAM_BURST_INFO_STC) * burstInfoLen);

        prvCpssDrvPpConfig[devNum]->hwCtrl[portGroupId].ramBurstInfoLen = burstInfoLen;
    }
    PRV_CPSS_DRV_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)

    return GT_OK;
}

/**
* @internal driverExMxDxHwPpHaModeSet function
* @endinternal
*
* @brief   function to set CPU High Availability mode of operation.
*
* @note   APPLICABLE DEVICES:      all ExMx devices
* @param[in] devNum                   - the device number.
* @param[in] mode                     - active or standby
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on failure.
*/
static GT_STATUS driverExMxDxHwPpHaModeSet
(
    IN  GT_U8   devNum,
    IN  CPSS_SYS_HA_MODE_ENT mode
)
{
    GT_STATUS   rc;
    PRV_CPSS_DRIVER_PP_CONFIG_STC *ppConfigPtr;/* Points to the Device's Driver*/
    GT_U32      portGroupId;/* the port group Id */

    if(0 == PRV_CPSS_DRV_HW_IF_PCI_COMPATIBLE_MAC(devNum))
    {
        /* nothing to update -- not supported option yet */
        return GT_OK;
    }

    ppConfigPtr =  prvCpssDrvPpConfig[devNum];

    PRV_CPSS_DRV_START_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
    {
        if(mode == CPSS_SYS_HA_MODE_ACTIVE_E)
        {
            /* update the pointer to the functions object -- to use the PCI */
            prvCpssDrvPpObjConfig[devNum]->busPtr = prvCpssDrvMngInfPciPtr;
        }
        else
        {
            /* update the pointer to the functions object -- to emulate the PCI */
            prvCpssDrvPpObjConfig[devNum]->busPtr = prvCpssDrvMngInfPciHaStandbyPtr;
        }

        /* we need to re-initialize the HW control driver --
           because we changed the OBJ of the prvCpssDrvHwCntlInit */
        /* Init Hw Cntl parameters. Now we know the value of doByteSwap */
        rc = prvCpssDrvHwCntlInit(devNum,portGroupId,
                         &(ppConfigPtr->hwInfo[portGroupId]),
                         GT_FALSE);/* isDiag */
        if (rc!= GT_OK)
            return rc;
    }
    PRV_CPSS_DRV_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
    prvCpssDrvPpObjConfig[devNum]->HAState = mode;

    return GT_OK;
}

#ifdef CHX_FAMILY

/**
* @internal prvCpssDrvLion2FirstPacketWA function
* @endinternal
*
* @brief   This function engages Lion2 A0 517 (DIP) FE-3832472 "First Packet drop"
*         erratum WA, which must be before any device init activity
* @param[in] devNum                   - The PP device number to read from.
* @param[in] portGroupId              - number of core(port group)
* @param[in] ppInInfoPtr              - (pointer to)   the info needed for initialization of
*                                      the driver for this PP
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - if HW access failed
*
* @note Should be engaged only for A0 revision.
*
*/
static GT_STATUS prvCpssDrvLion2FirstPacketWA
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN PRV_CPSS_DRV_PP_PHASE_1_INPUT_INFO_STC  *ppInInfoPtr
)
{
    GT_STATUS   rc;     /* return code */
    GT_U32      pcsNum; /* number of PCS lane */
    GT_U32      regAddr;/* register address */
#ifndef ASIC_SIMULATION
    GT_U32      timeOut;/* counter for wait after reast */
#endif
    GT_U32      regData;/* register data */

    /* Lion2 517 (DIP) FE-3832472 "First Packet drop" erratum WA */
    rc = prvCpssDrvLion2MMPcsUnReset(devNum,portGroupId);
    if(rc != GT_OK)
        return rc;
    /* stop MAC clock */
    for(pcsNum = 0; pcsNum < 16; pcsNum++)
    {
        regAddr = 0x88C0414 + pcsNum * 0x1000;
        rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr,
                                                12, 1, 0);
        if (rc!= GT_OK)
            return rc;
    }

    /* execute device reset just when coming to last port group */
    if(portGroupId == ppInInfoPtr->numOfPortGroups-1)
    {
        /* MG Soft Reset */
        rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, 1,0x18F805C,
                                                8, 1, 0);
        if (rc!= GT_OK)
            return rc;

        rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, 1,0x18F8060,
                                                8, 1, 0);
        if (rc!= GT_OK)
            return rc;

        rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, 1,0x18F800C,
                                                1, 1, 0);
        if (rc!= GT_OK)
            return rc;

        cpssOsTimerWkAfter(10);

        /* dummy reads to ensure right delay */
        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, 1, 0x58,
                                                 &regData);
        if (rc!= GT_OK)
            return rc;

        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, 1, 0xF80002C,
                                                 &regData);
        if (rc!= GT_OK)
            return rc;

        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, 1, 0xF800038,
                                                 &regData);
        if (rc!= GT_OK)
            return rc;

#ifndef ASIC_SIMULATION
        /* Verify SW reset finished */
        timeOut = 10;
        do
        {
            rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, 1,0x18F800C,
                                            1, 1, &regData);
            if (rc!= GT_OK)
                return rc;

            timeOut--;
            cpssOsTimerWkAfter(10);
        }while((regData != 1) && (timeOut > 0));

        if((regData != 1) && (timeOut == 0))
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, LOG_ERROR_NO_MSG);
#endif
    }/* end if(portGroupId == */

    return GT_OK;
}

/**
* @internal prvCpssDrvLion2MMPcsUnReset function
* @endinternal
*
* @brief   Unreset Lion2 MMPCS
*
* @param[in] devNum                   - system device number
* @param[in] portGroupId              - port group (core) number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS prvCpssDrvLion2MMPcsUnReset
(
    GT_U8                   devNum,
    GT_U32                  portGroupId
)
{
    GT_STATUS   rc;     /* return code */
    GT_U32      regAddr; /* address of register */
    GT_U32      pcsNum; /* PCS lane number */

    for(pcsNum = 0; pcsNum < 16; pcsNum++)
    {
        regAddr = 0x088c054c + pcsNum * 0x1000;
        rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId ,regAddr,
                                                0, 1, 1);
        if (rc!= GT_OK)
            return rc;
    }

    for(pcsNum = 0; pcsNum < 16; pcsNum++)
    {
        regAddr = 0x088c054c + pcsNum * 0x1000;
        rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId ,regAddr,
                                                1, 1, 1);
        if (rc!= GT_OK)
            return rc;
    }

    for(pcsNum = 0; pcsNum < 16; pcsNum++)
    {
        regAddr = 0x088c054c + pcsNum * 0x1000;
        rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId ,regAddr,
                                                2, 1, 1);
        if (rc!= GT_OK)
            return rc;
    }

    for(pcsNum = 0; pcsNum < 16; pcsNum++)
    {
        regAddr = 0x088c054c + pcsNum * 0x1000;
        rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId ,regAddr,
                                                5, 1, 1);
        if (rc!= GT_OK)
            return rc;
    }

    for(pcsNum = 0; pcsNum < 16; pcsNum++)
    {
        regAddr = 0x088c054c + pcsNum * 0x1000;
        rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId ,regAddr,
                                                6, 1, 1);
        if (rc!= GT_OK)
            return rc;
    }

    for(pcsNum = 0; pcsNum < 16; pcsNum++)
    {
        regAddr = 0x088c054c + pcsNum * 0x1000;
        rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId ,regAddr,
                                                4, 1, 1);
        if (rc!= GT_OK)
            return rc;
    }

    for(pcsNum = 0; pcsNum < 16; pcsNum++)
    {
        regAddr = 0x088c054c + pcsNum * 0x1000;
        rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId ,regAddr,
                                                3, 1, 1);
        if (rc!= GT_OK)
            return rc;
    }

    for(pcsNum = 0; pcsNum < 16; pcsNum++)
    {
        regAddr = 0x088c054c + pcsNum * 0x1000;
        rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId ,regAddr,
                                                7, 4, 0x4);
        if (rc!= GT_OK)
            return rc;
    }

    return GT_OK;
}

#endif /* CHX_FAMILY */

/**
* @internal prvCpssDrvDxExMxInitObject function
* @endinternal
*
* @brief   This function creates and initializes ExMxDx device driver object
*
* @param[in] devNum                   - The PP device number to read from.
*
* @retval GT_OK                    - on success
* @retval GT_ALREADY_EXIST         - if the driver object have been created before
*/
GT_STATUS prvCpssDrvDxExMxInitObject
(
     IN GT_U8   devNum
)
{
    GT_STATUS rc;

    /****************************************************/
    /* bind the generic part to the ExMxDx generic part */
    /****************************************************/
    prvCpssDrvPpObjConfig[devNum]->genPtr = prvCpssDrvGenExMxDxObjPtr;
    /* the bus part will be initialized in "phase 1" -
       when we get the parameter */
    prvCpssDrvPpObjConfig[devNum]->busPtr = NULL;
    prvCpssDrvPpObjConfig[devNum]->HAState = CPSS_SYS_HA_MODE_ACTIVE_E;

    /* driver object initialization */
    prvCpssDrvGenExMxDxObjPtr->drvHwPpCfgPhase1Init = driverDxExMxHwPpPhase1Init;
    prvCpssDrvGenExMxDxObjPtr->drvHwPpSetRamBurstConfig = driverDxExMxHwPpRamBurstConfigSet;
    prvCpssDrvGenExMxDxObjPtr->drvHwPpHaModeSet = driverExMxDxHwPpHaModeSet;
    prvCpssDrvGenExMxDxObjPtr->drvHwPpInterruptsTreeGet = prvCpssDrvExMxDxHwPpInterruptsTreeGet;
    rc = prvCpssDrvDxExMxEventsInitObject();

    if(rc != GT_OK)
        return rc;

    rc = prvCpssDrvDxExMxIntInitObject();

    return rc;
}



