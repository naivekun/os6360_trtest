/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file gtDbDxBobcat2LedIfConfig.c
*
* @brief Initialization functions for the Led Bobcat2/BobK Cetus/Caelum/Aldrin.
*
* @version   1
********************************************************************************
*/
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>
#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <appDemo/boardConfig/appDemoCfgMisc.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfigDefaults.h>
#include <appDemo/boardConfig/gtDbDxBoardTypeConfig.h>
#include <appDemo/boardConfig/gtDbDxBobcat2GenUtils.h>
#include <appDemo/boardConfig/gtDbDxBobcat2LedIfConfig.h>

#include <cpss/generic/cpssHwInit/cpssLedCtrl.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInitLedCtrl.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInitLedCtrl.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortMapping.h>

/*
#define BC2_LED_INTERFACES_NUM_CNS     5
#define CETUS_LED_INTERFACES_NUM_CNS   2
#define CAELUM_LED_INTERFACES_NUM_CNS  5
#define ALDRIN_LED_INTERFACES_NUM_CNS  2
#define BOBCAT3_LED_INTERFACES_NUM_CNS 4
*/

static GT_STATUS bobcat2LedInit
(
    IN GT_U8 devNum
)
{
    GT_PHYSICAL_PORT_NUM        portNum;  /* physical port number */
    GT_U32                      position; /* LED stream position  */
    CPSS_DXCH_PORT_MAP_STC      portMap;  /* port map             */
    GT_STATUS                   rc;       /* return code          */

    for(portNum = 0; portNum < (appDemoPpConfigList[devNum].maxPortNumber); portNum++)
    {
        CPSS_ENABLER_PORT_SKIP_CHECK(devNum, portNum);

        /* skip ports mapped to MACs 60..71  because they use same LED interface as MACs 36..47 */
        rc = cpssDxChPortPhysicalPortMapGet(devNum, portNum, 1, &portMap);
        if(rc != GT_OK)
        {
            continue;
        }

        if (portMap.interfaceNum >= 60)
        {
            continue;
        }

        /* For every physical port configure 'LEDs number'(in range 0..11) */
        position = portNum % 12;
        rc = cpssDxChLedStreamPortPositionSet(devNum, portNum, position);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}



/*------------------------------------------------------------------------------------------------------------------------------*
 * 1.  LEDs Interfaces Mode Select = 0 (not BWC)  (DFX)                                                                         *
 *                                                                                                                              *
 *    /Cider/EBU/BobK/BobK {Current}/Reset And Init Controller/DFX Units/Units/DFX Server Registers/Device General Control 19   *
 *     field 0 - 0  Selects the operation mode of the LED interfaces (defines the ports assigned to each LED interface).        *
 *                  Bobcat2 Backward compatible LED mode                                                                        *
 *                  1 - BWC                                                                                                     *
 *                  0 - not BWC                                                                                                 *
 *                                                                                                                              *
 *------------------------------------------------------------------------------------------------------------------------------*/
#define PRV_CPSS_DXCH_BC2_LED_BACKWARD_COMPATIBILITY_OFFS_CNS 0
#define PRV_CPSS_DXCH_BC2_LED_BACKWARD_COMPATIBILITY_LEN_CNS  1


static GT_STATUS bobkLedStreamBC2BackwardCompatibilityModeSet
(
    IN  GT_U8    devNum,
    IN  GT_BOOL  isBc2BackwardCompatible,
    IN  GT_U32   bc2BoardType
)
{
    GT_STATUS rc;
    GT_U32 regAddr;
    GT_U32 data;


    data = (isBc2BackwardCompatible == GT_TRUE);

    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl19;

    rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum,regAddr,
                                                     PRV_CPSS_DXCH_BC2_LED_BACKWARD_COMPATIBILITY_OFFS_CNS,
                                                     PRV_CPSS_DXCH_BC2_LED_BACKWARD_COMPATIBILITY_LEN_CNS,
                                                     data);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(APP_DEMO_CAELUM_BOARD_DB_CNS == bc2BoardType)
    {
        /* set Bits [14:13] = CPU_ICE is MSYS (0x1) */
        rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, 13, 2, 1);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/*-------------------------------------------------------------------------------
 *  BobK Cetus
 *-------------------------------------------------------------------------------
 *    per port configuration
 *    cpssDxChLedStreamPortPositionSet covers
 *    /Cider/EBU/BobK/BobK {Current}/Switching Core/GOP/Units/GOP/<XLG MAC IP> XLG MAC IP Units%p/XG MIB Counters Control
 *    bit 5-10   Enables selection of the number of this port in the LEDs chain.
 *
 *    0x100C0030 + 0x200000 + 0x1000*(p-56): where p (56-59) represents XLG Port
 *    0x100C0030 + 0x200000 + 0x1000*(p-56): where p (64-71) represents XLG Port
 *
 *
 *                                                        Port    position
 *    Write to address 0x0102C0030 the value 0x0C     --    56         0
 *    Write to address 0x0102C1030 the value 0x6C     --    57         3
 *    Write to address 0x0102C2030 the value 0x2C     --    58         1
 *    Write to address 0x0102C3030 the value 0x4C     --    59         2
 *
 *    Write to address 0x0102C8030 the value 0x10C    --    64         8
 *    Write to address 0x0102C9030 the value 0x16C    --    65        11
 *    Write to address 0x0102CA030 the value 0x12C    --    66         9
 *    Write to address 0x0102CB030 the value 0x14C    --    67        10
 *
 *    Write to address 0x0102CC030 the value 0x8C     --    68         4
 *    Write to address 0x0102CD030 the value 0x0EC    --    69         7
 *    Write to address 0x0102CE030 the value 0x0AC    --    70         5
 *    Write to address 0x0102CF030 the value 0x0CC    --    71         6
 *-----------------------------------------------------------------------------------*/


typedef struct
{
    GT_U32  portMac;
    GT_U32  ledIf;
    GT_U32  ledPosition;
}APPDEMO_MAC_LEDPOSITION_STC;

#define APPDEMO_BAD_VALUE (GT_U32)(~0)

static APPDEMO_MAC_LEDPOSITION_STC cetus_mac_ledPos_Arr[] =
{
     /* portMac           ledIf,    ledPosition    */
     {      56           , 4,    0            }
    ,{      57           , 4,    3            }
    ,{      58           , 4,    1            }
    ,{      59           , 4,    2            }
    ,{      64           , 4,    8            }
    ,{      65           , 4,   11            }
    ,{      66           , 4,    9            }
    ,{      67           , 4,   10            }
    ,{      68           , 4,    4            }
    ,{      69           , 4,    7            }
    ,{      70           , 4,    5            }
    ,{      71           , 4,    6            }
    ,{APPDEMO_BAD_VALUE  , APPDEMO_BAD_VALUE,   APPDEMO_BAD_VALUE}
};

static APPDEMO_MAC_LEDPOSITION_STC caelum_mac_ledPos_Arr[] =
{
     /* portMac           ledIf,    ledPosition    */
     {       0           , 0,     0            }
    ,{       1           , 0,     1            }
    ,{       2           , 0,     2            }
    ,{       3           , 0,     3            }
    ,{       4           , 0,     4            }
    ,{       5           , 0,     5            }
    ,{       6           , 0,     6            }
    ,{       7           , 0,     7            }
    ,{       8           , 0,     8            }
    ,{       9           , 0,     9            }
    ,{      10           , 0,    10            }
    ,{      11           , 0,    11            }
    ,{      12           , 1,     0            }
    ,{      13           , 1,     1            }
    ,{      14           , 1,     2            }
    ,{      15           , 1,     3            }
    ,{      16           , 1,     4            }
    ,{      17           , 1,     5            }
    ,{      18           , 1,     6            }
    ,{      19           , 1,     7            }
    ,{      20           , 1,     8            }
    ,{      21           , 1,     9            }
    ,{      22           , 1,    10            }
    ,{      23           , 1,    11            }
    ,{      24           , 2,     0            }
    ,{      25           , 2,     1            }
    ,{      26           , 2,     2            }
    ,{      27           , 2,     3            }
    ,{      28           , 2,     4            }
    ,{      29           , 2,     5            }
    ,{      30           , 2,     6            }
    ,{      31           , 2,     7            }
    ,{      32           , 2,     8            }
    ,{      33           , 2,     9            }
    ,{      34           , 2,    10            }
    ,{      35           , 2,    11            }
    ,{      36           , 3,     0            }
    ,{      37           , 3,     1            }
    ,{      38           , 3,     2            }
    ,{      39           , 3,     3            }
    ,{      40           , 3,     4            }
    ,{      41           , 3,     5            }
    ,{      42           , 3,     6            }
    ,{      43           , 3,     7            }
    ,{      44           , 3,     8            }
    ,{      45           , 3,     9            }
    ,{      46           , 3,    10            }
    ,{      47           , 3,    11            }
    ,{      56           , 4,     0            }
    ,{      57           , 4,     1            }
    ,{      58           , 4,     2            }
    ,{      59           , 4,     3            }
    ,{      64           , 4,     4            }
    ,{      65           , 4,     5            }
    ,{      66           , 4,     6            }
    ,{      67           , 4,     7            }
    ,{      68           , 4,     8            }
    ,{      69           , 4,     9            }
    ,{      70           , 4,    10            }
    ,{      71           , 4,    11            }
    ,{APPDEMO_BAD_VALUE  , APPDEMO_BAD_VALUE,   APPDEMO_BAD_VALUE }
};


static APPDEMO_MAC_LEDPOSITION_STC aldrin_mac_ledPos_Arr[] =
{
     /* portMac           ledIf,    ledPosition    */
     {       0           , 0,     0            }
    ,{       1           , 0,     1            }
    ,{       2           , 0,     2            }
    ,{       3           , 0,     3            }
    ,{       4           , 0,     4            }
    ,{       5           , 0,     5            }
    ,{       6           , 0,     6            }
    ,{       7           , 0,     7            }
    ,{       8           , 0,     8            }
    ,{       9           , 0,     9            }
    ,{      10           , 0,    10            }
    ,{      11           , 0,    11            }
    ,{      12           , 0,    12            }
    ,{      13           , 0,    13            }
    ,{      14           , 0,    14            }
    ,{      15           , 0,    15            }
    ,{      16           , 0,    16            }
    ,{      17           , 0,    17            }
    ,{      18           , 0,    18            }
    ,{      19           , 0,    19            }
    ,{      20           , 0,    20            }
    ,{      21           , 0,    21            }
    ,{      22           , 0,    22            }
    ,{      23           , 0,    23            }
    ,{      24           , 0,    24            }
    ,{      25           , 0,    25            }
    ,{      26           , 0,    26            }
    ,{      27           , 0,    27            }
    ,{      28           , 0,    28            }
    ,{      29           , 0,    29            }
    ,{      30           , 0,    30            }
    ,{      31           , 0,    31            }
    ,{      32           , 1,     0            }
    ,{APPDEMO_BAD_VALUE  , APPDEMO_BAD_VALUE,   APPDEMO_BAD_VALUE }
};


static APPDEMO_MAC_LEDPOSITION_STC bobcat3_mac_ledPos_Arr[] =
{
      /* portMac           ledIF,    ledPosition,    */
     /* DP 0*/
     {       0           , 0,    0              }
    ,{       4           , 0,    1              }
    ,{       8           , 0,    2              }
    ,{      72           , 0,    3              }
     /* DP 1*/
    ,{      12           , 1,    0              }
    ,{      16           , 1,    1              }
    ,{      20           , 1,    2              }
    /* DP 2 */
    ,{      24           , 1,    3              }
    ,{      25           , 1,    4              }
    ,{      26           , 1,    5              }
    ,{      27           , 1,    6              }
    ,{      28           , 1,    7              }
    ,{      29           , 1,    8              }
    ,{      30           , 1,    9              }
    ,{      31           , 1,   10              }
    ,{      32           , 1,   11              }
    ,{      33           , 1,   12              }
    ,{      34           , 1,   13              }
    ,{      35           , 1,   14              }
    /* DP 3 */
    ,{      36           , 2,    0              }
    ,{      37           , 2,    1              }
    ,{      38           , 2,    2              }
    ,{      39           , 2,    3              }
    ,{      40           , 2,    4              }
    ,{      41           , 2,    5              }
    ,{      42           , 2,    6              }
    ,{      43           , 2,    7              }
    ,{      44           , 2,    8              }
    ,{      45           , 2,    9              }
    ,{      46           , 2,   10              }
    ,{      47           , 2,   11              }
    ,{      73           , 2,   12              }
    /* DP-4 */
    ,{      48           , 3,    0              }
    ,{      49           , 3,    1              }
    ,{      50           , 3,    2              }
    ,{      51           , 3,    3              }
    ,{      52           , 3,    4              }
    ,{      53           , 3,    5              }
    ,{      54           , 3,    6              }
    ,{      55           , 3,    7              }
    ,{      56           , 3,    8              }
    ,{      57           , 3,    9              }
    ,{      58           , 3,   10              }
    ,{      59           , 3,   11              }
    /* DP-5 */
    ,{      60           , 3,    12             }
    ,{      61           , 3,    13             }
    ,{      62           , 3,    14             }
    ,{      63           , 3,    15             }
    ,{      64           , 3,    16             }
    ,{      65           , 3,    17             }
    ,{      66           , 3,    18             }
    ,{      67           , 3,    19             }
    ,{      68           , 3,    20             }
    ,{      69           , 3,    21             }
    ,{      70           , 3,    22             }
    ,{      71           , 3,    23             }
    ,{APPDEMO_BAD_VALUE  , APPDEMO_BAD_VALUE, APPDEMO_BAD_VALUE  }
};

static APPDEMO_MAC_LEDPOSITION_STC aldrin2_mac_ledPos_Arr[] =
{
      /* portMac           ledIF,    ledPosition,    */
     /* DP 0*/
     {       0           , 0,    0              }
    ,{       4           , 0,    1              }
    ,{       8           , 0,    2              }
     /* DP 1*/
    ,{      12           , 1,    0              }
    ,{      16           , 1,    1              }
    ,{      20           , 1,    2              }
    /* DP 2 */
    ,{      24           , 3,    0              }
    ,{      25           , 3,    1              }
    ,{      26           , 3,    2              }
    ,{      27           , 3,    3              }
    ,{      28           , 3,    4              }
    ,{      29           , 3,    5              }
    ,{      30           , 3,    6              }
    ,{      31           , 3,    7              }
    ,{      32           , 3,    8              }
    ,{      33           , 3,    9              }
    ,{      34           , 3,   10              }
    ,{      35           , 3,   11              }
    ,{      36           , 3,   12              }
    ,{      37           , 3,   13              }
    ,{      38           , 3,   14              }
    ,{      39           , 3,   15              }
    ,{      40           , 3,   16              }
    ,{      41           , 3,   17              }
    ,{      42           , 3,   18              }
    ,{      43           , 3,   19              }
    ,{      44           , 3,   20              }
    ,{      45           , 3,   21              }
    ,{      46           , 3,   22              }
    ,{      47           , 3,   23              }
    /* CPU */
    ,{      72           , 3,   24              }
    /* DP-3 */
    ,{      48           , 2,    0              }
    ,{      49           , 2,    1              }
    ,{      50           , 2,    2              }
    ,{      51           , 2,    3              }
    ,{      52           , 2,    4              }
    ,{      53           , 2,    5              }
    ,{      54           , 2,    6              }
    ,{      55           , 2,    7              }
    ,{      56           , 2,    8              }
    ,{      57           , 2,    9              }
    ,{      58           , 2,   10              }
    ,{      59           , 2,   11              }
    ,{      60           , 2,   12              }
    ,{      61           , 2,   13              }
    ,{      62           , 2,   14              }
    ,{      63           , 2,   15              }
    ,{      64           , 2,   16              }
    ,{      65           , 2,   17              }
    ,{      66           , 2,   18              }
    ,{      67           , 2,   19              }
    ,{      68           , 2,   20              }
    ,{      69           , 2,   21              }
    ,{      70           , 2,   22              }
    ,{      71           , 2,   23              }
    ,{APPDEMO_BAD_VALUE  , APPDEMO_BAD_VALUE, APPDEMO_BAD_VALUE  }
};



static GT_VOID findMinMaxPositionByLedIf(GT_U32 ledIf,
                                            APPDEMO_MAC_LEDPOSITION_STC *ledPositionTbl,
                                            GT_U32 *minPositionPtr,
                                            GT_U32 *maxPositionPtr)
{
    GT_U32 i;
    GT_U32  minPosition = (GT_U32) (-1);
    GT_U32  maxPosition = 0;

    for (i = 0 ; ledPositionTbl[i].ledIf !=  APPDEMO_BAD_VALUE; i++)
    {
        if (ledPositionTbl[i].ledIf == ledIf)
        {
            if (ledPositionTbl[i].ledPosition < minPosition)
            {
                minPosition = ledPositionTbl[i].ledPosition;
            }
            if (ledPositionTbl[i].ledPosition > maxPosition)
            {
                maxPosition = ledPositionTbl[i].ledPosition;
            }
        }
    }

    *minPositionPtr = minPosition;
    *maxPositionPtr = maxPosition;
}

static GT_U32 findLedPositionByMac(GT_U32 mac, APPDEMO_MAC_LEDPOSITION_STC *ledPositionTbl)
{
    GT_U32 i;
    for (i = 0 ; ledPositionTbl[i].portMac !=  APPDEMO_BAD_VALUE; i++)
    {
        if (ledPositionTbl[i].portMac == mac)
        {
            return ledPositionTbl[i].ledPosition;
        }
    }
    return APPDEMO_BAD_VALUE;
}

static GT_STATUS bobkCetusLedInit
(
    GT_U8    devNum,
    GT_U32   bc2BoardType
)
{
    GT_STATUS                       rc;
    GT_U32                          ledInterfaceNum;
    GT_U32                          classNum;
    GT_U32                          group;
    GT_PHYSICAL_PORT_NUM            portNum;
    CPSS_LED_CONF_STC               ledConfig;
    CPSS_LED_CLASS_MANIPULATION_STC ledClassManip;
    CPSS_LED_GROUP_CONF_STC         groupConf;
    GT_U32                          position;
    CPSS_DXCH_PORT_MAP_STC          portMap;
    GT_U32                          inactiveClassList[7];
    GT_U32                          i;
    /*-------------------------------------------*
     *  set no BC2 BW compatibility              *
     *-------------------------------------------*/
    rc = bobkLedStreamBC2BackwardCompatibilityModeSet(devNum,GT_FALSE,bc2BoardType);
    if(rc != GT_OK)
    {
        return rc;
    }

    ledInterfaceNum = 4;

    ledConfig.ledOrganize             = CPSS_LED_ORDER_MODE_BY_PORT_E;
    ledConfig.disableOnLinkDown       = GT_FALSE;
    ledConfig.blink0DutyCycle         = CPSS_LED_BLINK_DUTY_CYCLE_1_E; /* 50% */
    ledConfig.blink0Duration          = CPSS_LED_BLINK_DURATION_2_E;   /* 128 ms */
    ledConfig.blink1DutyCycle         = CPSS_LED_BLINK_DUTY_CYCLE_1_E;  /* 50%    */
    ledConfig.blink1Duration          = CPSS_LED_BLINK_DURATION_1_E;    /* 64 ms */
    ledConfig.pulseStretch            = CPSS_LED_PULSE_STRETCH_1_E;
    ledConfig.ledStart                = 0;
    ledConfig.ledEnd                  = 255;
    ledConfig.clkInvert               = GT_FALSE; /* don't care */
    ledConfig.class5select            = CPSS_LED_CLASS_5_SELECT_FIBER_LINK_UP_E;   /* don't care */
    ledConfig.class13select           = CPSS_LED_CLASS_13_SELECT_COPPER_LINK_UP_E; /* don't care */
    ledConfig.invertEnable            = GT_FALSE;        /*   1  Active High  */
    ledConfig.ledClockFrequency       = CPSS_LED_CLOCK_OUT_FREQUENCY_1000_E;


    rc = cpssDxChLedStreamConfigSet(devNum,ledInterfaceNum, &ledConfig);
    if(rc != GT_OK)
    {
        return rc;
    }

    ledClassManip.invertEnable            = GT_FALSE; /* not relevant for BC2/BobK */
    ledClassManip.blinkEnable             = GT_FALSE;
    ledClassManip.blinkSelect             = CPSS_LED_BLINK_SELECT_0_E;
    ledClassManip.forceEnable             = GT_FALSE;
    ledClassManip.forceData               = 0;
    ledClassManip.pulseStretchEnable      = GT_FALSE;
    ledClassManip.disableOnLinkDown       = GT_FALSE;

    inactiveClassList[0] = 5;
    inactiveClassList[1] = 4;
    inactiveClassList[2] = 3;
    inactiveClassList[3] = 1;
    inactiveClassList[4] = 0;
    inactiveClassList[5] = APPDEMO_BAD_VALUE;

    for (i = 0 ; inactiveClassList[i] != APPDEMO_BAD_VALUE; i++)
    {
        classNum = inactiveClassList[i];
        rc = cpssDxChLedStreamClassManipulationSet(devNum
                                                        ,ledInterfaceNum
                                                        ,CPSS_DXCH_LED_PORT_TYPE_XG_E /* don't care , not applicable for BobK */
                                                        ,classNum
                                                        ,&ledClassManip);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    ledClassManip.invertEnable            = GT_FALSE; /* not relevant for BC2/BobK */
    ledClassManip.blinkEnable             = GT_TRUE;
    ledClassManip.blinkSelect             = CPSS_LED_BLINK_SELECT_0_E;
    ledClassManip.forceEnable             = GT_FALSE;
    ledClassManip.forceData               = 0;
    ledClassManip.pulseStretchEnable      = GT_FALSE;
    ledClassManip.disableOnLinkDown       = GT_FALSE;

    classNum = 2;
    rc = cpssDxChLedStreamClassManipulationSet(devNum
                                                    ,ledInterfaceNum
                                                    ,CPSS_DXCH_LED_PORT_TYPE_XG_E /* don't care , not applicable for BobK */
                                                    ,classNum
                                                    ,&ledClassManip);
    if(rc != GT_OK)
    {
        return rc;
    }

    groupConf.classA = 1;
    groupConf.classB = 1;
    groupConf.classC = 1;
    groupConf.classD = 1;

    group = 0;
    rc = cpssDxChLedStreamGroupConfigSet(devNum,ledInterfaceNum,CPSS_DXCH_LED_PORT_TYPE_XG_E, group,&groupConf);
    if(rc != GT_OK)
    {
        return rc;
    }
    group = 1;
    rc = cpssDxChLedStreamGroupConfigSet(devNum,ledInterfaceNum,CPSS_DXCH_LED_PORT_TYPE_XG_E, group,&groupConf);
    if(rc != GT_OK)
    {
        return rc;
    }


    for(portNum = 0; portNum < (appDemoPpConfigList[devNum].maxPortNumber); portNum++)
    {
        CPSS_ENABLER_PORT_SKIP_CHECK(devNum, portNum);

        rc = cpssDxChPortPhysicalPortMapGet(devNum, portNum, 1, &portMap);
        if(rc != GT_OK)
        {
            continue;
        }

        position = findLedPositionByMac(portMap.interfaceNum, &cetus_mac_ledPos_Arr[0]);
        if (position != APPDEMO_BAD_VALUE)
        {
            rc = cpssDxChLedStreamPortPositionSet(devNum, portNum, position);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return GT_OK;
}


static GT_STATUS bobkCaelumLedInit
(
    GT_U8    devNum,
    GT_U32   bc2BoardType
)
{
    GT_STATUS                       rc;
    GT_U32                          ledInterfaceNum;
    GT_U32                          classNum;
    GT_U32                          group;
    GT_PHYSICAL_PORT_NUM            portNum;
    CPSS_LED_CONF_STC               ledConfig;
    CPSS_LED_CLASS_MANIPULATION_STC ledClassManip;
    CPSS_LED_GROUP_CONF_STC         groupConf;
    GT_U32                          position;
    CPSS_DXCH_PORT_MAP_STC          portMap;

    /*-------------------------------------------*
     *  set no BC2 BW compatibility              *
     *-------------------------------------------*/
    rc = bobkLedStreamBC2BackwardCompatibilityModeSet(devNum,GT_FALSE,bc2BoardType);
    if(rc != GT_OK)
    {
        return rc;
    }

    ledConfig.ledOrganize             = CPSS_LED_ORDER_MODE_BY_PORT_E;
    ledConfig.disableOnLinkDown       = GT_FALSE;
    ledConfig.blink0DutyCycle         = CPSS_LED_BLINK_DUTY_CYCLE_1_E; /* 50% */
    ledConfig.blink0Duration          = CPSS_LED_BLINK_DURATION_5_E;   /* 1024 ms */
    ledConfig.blink1DutyCycle         = CPSS_LED_BLINK_DUTY_CYCLE_1_E;  /* 50%    */
    ledConfig.blink1Duration          = CPSS_LED_BLINK_DURATION_1_E;    /* 64 ms */
    ledConfig.pulseStretch            = CPSS_LED_PULSE_STRETCH_0_NO_E;
    ledConfig.ledStart                = 0;
    ledConfig.ledEnd                  = 95;
    ledConfig.clkInvert               = GT_FALSE; /* don't care */
    ledConfig.class5select            = CPSS_LED_CLASS_5_SELECT_FIBER_LINK_UP_E;   /* don't care */
    ledConfig.class13select           = CPSS_LED_CLASS_13_SELECT_COPPER_LINK_UP_E; /* don't care */
    ledConfig.invertEnable            = GT_FALSE;        /*   1  Active High  */
    ledConfig.ledClockFrequency       = CPSS_LED_CLOCK_OUT_FREQUENCY_1000_E;

    for(ledInterfaceNum = 0; ledInterfaceNum < PRV_CPSS_DXCH_CAELUM_LED_IF_NUM_CNS; ledInterfaceNum++)
    {
        rc = cpssDxChLedStreamConfigSet(devNum,ledInterfaceNum, &ledConfig);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    ledClassManip.invertEnable            = GT_FALSE; /* not relevant for BC2/BobK */
    ledClassManip.blinkSelect             = CPSS_LED_BLINK_SELECT_0_E;
    ledClassManip.forceEnable             = GT_FALSE;
    ledClassManip.forceData               = 0;
    ledClassManip.pulseStretchEnable      = GT_FALSE;
    ledClassManip.disableOnLinkDown       = GT_FALSE;

    for(ledInterfaceNum = 0; ledInterfaceNum < PRV_CPSS_DXCH_CAELUM_LED_IF_NUM_CNS; ledInterfaceNum++)
    {
        for (classNum = 0 ; classNum < 6; classNum++)
        {
            if(classNum == 2)
            {
                ledClassManip.blinkEnable             = GT_TRUE;
            }
            else
            {
                ledClassManip.blinkEnable             = GT_FALSE;
            }
            rc = cpssDxChLedStreamClassManipulationSet(devNum
                                                            ,ledInterfaceNum
                                                            ,CPSS_DXCH_LED_PORT_TYPE_XG_E /* don't care , not applicable for BobK */
                                                            ,classNum
                                                            ,&ledClassManip);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    groupConf.classA = 1;
    groupConf.classB = 1;
    groupConf.classC = 2;
    groupConf.classD = 2;

    for(ledInterfaceNum = 0; ledInterfaceNum < PRV_CPSS_DXCH_CAELUM_LED_IF_NUM_CNS; ledInterfaceNum++)
    {
        group = 0;
        rc = cpssDxChLedStreamGroupConfigSet(devNum,ledInterfaceNum,CPSS_DXCH_LED_PORT_TYPE_XG_E, group,&groupConf);
        if(rc != GT_OK)
        {
            return rc;
        }
        group = 1;
        rc = cpssDxChLedStreamGroupConfigSet(devNum,ledInterfaceNum,CPSS_DXCH_LED_PORT_TYPE_XG_E, group,&groupConf);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    for(portNum = 0; portNum < (appDemoPpConfigList[devNum].maxPortNumber); portNum++)
    {
        CPSS_ENABLER_PORT_SKIP_CHECK(devNum, portNum);

        rc = cpssDxChPortPhysicalPortMapGet(devNum, portNum, 1, /*OUT*/&portMap);
        if(rc != GT_OK)
        {
            continue;
        }

        position = findLedPositionByMac(portMap.interfaceNum, &caelum_mac_ledPos_Arr[0]);
        if (position != APPDEMO_BAD_VALUE)
        {
            rc = cpssDxChLedStreamPortPositionSet(devNum, portNum, position);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return GT_OK;
}

static GT_STATUS aldrinLedInit
(
    GT_U8    devNum
)
{
    GT_STATUS                       rc;
    GT_U32                          ledInterfaceNum;
    GT_U32                          classNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    CPSS_LED_CONF_STC               ledConfig;
    CPSS_LED_CLASS_MANIPULATION_STC ledClassManip;
    GT_U32                          position;
    CPSS_DXCH_PORT_MAP_STC          portMap;

    ledConfig.ledOrganize             = CPSS_LED_ORDER_MODE_BY_CLASS_E;
    ledConfig.disableOnLinkDown       = GT_FALSE;
    ledConfig.blink0DutyCycle         = CPSS_LED_BLINK_DUTY_CYCLE_1_E; /* 50% */
    ledConfig.blink0Duration          = CPSS_LED_BLINK_DURATION_2_E;   /* 128 ms */
    ledConfig.blink1DutyCycle         = CPSS_LED_BLINK_DUTY_CYCLE_1_E;  /* 50%    */
    ledConfig.blink1Duration          = CPSS_LED_BLINK_DURATION_1_E;    /* 64 ms */
    ledConfig.pulseStretch            = CPSS_LED_PULSE_STRETCH_1_E;
    ledConfig.ledStart                = 0;
    ledConfig.ledEnd                  = 255;
    ledConfig.clkInvert               = GT_FALSE; /* don't care */
    ledConfig.class5select            = CPSS_LED_CLASS_5_SELECT_FIBER_LINK_UP_E;   /* don't care */
    ledConfig.class13select           = CPSS_LED_CLASS_13_SELECT_COPPER_LINK_UP_E; /* don't care */
    ledConfig.invertEnable            = GT_FALSE;        /*  1-Active High  */
    ledConfig.ledClockFrequency       = CPSS_LED_CLOCK_OUT_FREQUENCY_1000_E;

    for(ledInterfaceNum = 0; ledInterfaceNum < PRV_CPSS_DXCH_ALDRIN_LED_IF_NUM_CNS; ledInterfaceNum++)
    {
        rc = cpssDxChLedStreamConfigSet(devNum, ledInterfaceNum, &ledConfig);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    ledClassManip.invertEnable            = GT_FALSE;
    ledClassManip.blinkSelect             = CPSS_LED_BLINK_SELECT_0_E;
    ledClassManip.forceEnable             = GT_FALSE;
    ledClassManip.forceData               = 0;
    ledClassManip.pulseStretchEnable      = GT_FALSE;
    ledClassManip.disableOnLinkDown       = GT_FALSE;

    for(ledInterfaceNum = 0; ledInterfaceNum < PRV_CPSS_DXCH_ALDRIN_LED_IF_NUM_CNS; ledInterfaceNum++)
    {
        for (classNum = 0; classNum < 6; classNum++)
        {
            if(classNum == 2)
            {
                ledClassManip.blinkEnable             = GT_TRUE;
            }
            else
            {
                ledClassManip.blinkEnable             = GT_FALSE;
            }
            rc = cpssDxChLedStreamClassManipulationSet(devNum
                                                            ,ledInterfaceNum
                                                            ,CPSS_DXCH_LED_PORT_TYPE_XG_E /* don't care for Aldrin */
                                                            ,classNum
                                                            ,&ledClassManip);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    for(portNum = 0; portNum < (appDemoPpConfigList[devNum].maxPortNumber); portNum++)
    {
        CPSS_ENABLER_PORT_SKIP_CHECK(devNum, portNum);

        if (prvCpssDxChPortRemotePortCheck(devNum, portNum))
        {
            /* Skip remote ports */
            continue;
        }
        rc = cpssDxChPortPhysicalPortMapGet(devNum, portNum, 1, &portMap);
        if(rc != GT_OK)
        {
            continue;
        }

        position = findLedPositionByMac(portMap.interfaceNum, &aldrin_mac_ledPos_Arr[0]);
        if (position != APPDEMO_BAD_VALUE)
        {
            rc = cpssDxChLedStreamPortPositionSet(devNum, portNum, position);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return GT_OK;
}



static GT_STATUS bobcat3LedInit
(
    GT_U8    devNum,
    GT_U32   bc2BoardType
)
{
    GT_STATUS                       rc;
    GT_U32                          ledInterfaceNum;
    GT_U32                          classNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    CPSS_LED_CONF_STC               ledConfig;
    CPSS_LED_CLASS_MANIPULATION_STC ledClassManip;
    GT_U32                          position;
    CPSS_DXCH_PORT_MAP_STC          portMap;
    APPDEMO_MAC_LEDPOSITION_STC     *ledPositionTbl;
    GT_BOOL                         stretchClassEnable;

    switch(PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_BOBCAT3_E:
            ledPositionTbl = bobcat3_mac_ledPos_Arr;
            stretchClassEnable = GT_FALSE;
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            ledPositionTbl = aldrin2_mac_ledPos_Arr;
            stretchClassEnable = GT_TRUE;
            break;
        default:
            return GT_NOT_APPLICABLE_DEVICE;
    }

    ledConfig.ledOrganize             = CPSS_LED_ORDER_MODE_BY_CLASS_E;                                /* LedControl.OrganizeMode     [ 1: 1] =   1  by port   */
    ledConfig.ledClockFrequency       = CPSS_LED_CLOCK_OUT_FREQUENCY_1000_E;                           /* LedControl.LED clk freq     [ 2: 3] =   1  1000KHz     */
    ledConfig.invertEnable            = GT_TRUE;        /*   1  Active High  */                        /* LedControl.LED-Light-High   [ 4: 4] =   0 active-low */
    ledConfig.pulseStretch            = CPSS_LED_PULSE_STRETCH_1_E;                                   /* LedControl.Pulse-stretch-div[ 5: 7] =   1 No division */
    /*ledConfig.ledStart                = 64;                                                         *//* LedControl.LED-Start        [ 8:15] = 0x40 */
    /*ledConfig.ledEnd                  = 67;                                                         *//* LedControl.LED-End          [16:23] = 0x43 */

    ledConfig.blink0Duration          = CPSS_LED_BLINK_DURATION_1_E;    /* 64 ms */                     /* blinkGlobalControl.Blink0-Duration [ 0- 2]  = 1 32ms */
    ledConfig.blink0DutyCycle         = CPSS_LED_BLINK_DUTY_CYCLE_1_E;  /* 50%   */                     /* blinkGlobalControl.Blink0-Duty     [ 3- 4]  = 1 25%  */
    ledConfig.blink1Duration          = CPSS_LED_BLINK_DURATION_1_E;    /* 64 ms */                     /* blinkGlobalControl.Blink1-Duration [ 5- 7]  = 1 32ms */
    ledConfig.blink1DutyCycle         = CPSS_LED_BLINK_DUTY_CYCLE_1_E;  /* 50%   */                     /* blinkGlobalControl.Blink1-Duty     [ 8- 9]  = 1 25%  */

    ledConfig.disableOnLinkDown       = GT_FALSE;  /* don't care , see led-class manipulation */
    ledConfig.clkInvert               = GT_FALSE;  /* don't care */
    ledConfig.class5select            = CPSS_LED_CLASS_5_SELECT_FIBER_LINK_UP_E;   /* don't care */
    ledConfig.class13select           = CPSS_LED_CLASS_13_SELECT_COPPER_LINK_UP_E; /* don't care */

    for(ledInterfaceNum = 0; ledInterfaceNum < PRV_CPSS_DXCH_BOBCAT3_LED_IF_NUM_CNS; ledInterfaceNum++)
    {
        findMinMaxPositionByLedIf(ledInterfaceNum,
                                    ledPositionTbl,
                                    &ledConfig.ledStart,
                                    &ledConfig.ledEnd);
        ledConfig.ledStart += 64;   /* Jump to start of Class 2 */
        ledConfig.ledEnd += 64;
        rc = cpssDxChLedStreamConfigSet(devNum,ledInterfaceNum, &ledConfig);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    ledClassManip.invertEnable            = GT_FALSE; /* not relevant for BC2/BobK */
    ledClassManip.blinkSelect             = CPSS_LED_BLINK_SELECT_0_E;                            /* blinkGlobalControl.Blink Sel            [10-17] = 0 blink-0         */
    ledClassManip.forceEnable             = GT_FALSE;                                             /* classesAndGroupConfig.Force En          [24-29] = pos = classNum = 0*/
    ledClassManip.forceData               = 0;                                                    /* classForcedData[class]                  [ 0:31] = 0                 */
    ledClassManip.pulseStretchEnable      = stretchClassEnable;                                   /* classGlobalControl.stretch class        [ 6-11] = pos = classNum = 0/1*/

    for(ledInterfaceNum = 0; ledInterfaceNum < PRV_CPSS_DXCH_BOBCAT3_LED_IF_NUM_CNS; ledInterfaceNum++)
    {
        for (classNum = 0 ; classNum < 6; classNum++)
        {
            if(classNum == 2)
            {
                ledClassManip.disableOnLinkDown       = GT_TRUE;                                  /* classGlobalControl.disable on link down [ 0- 5] = pos = classNum = 0*/
                ledClassManip.blinkEnable             = GT_TRUE;                                  /* blinkGlobalControl.blinkEn     [18-25] = pos = classNum val = 1 */
            }                                                                                     /* blinkGlobalControl.Blink Select[10-17] = pos = classNum val = 0 */
            else
            {
                ledClassManip.disableOnLinkDown       = GT_FALSE;
                ledClassManip.blinkEnable             = GT_FALSE;
            }
            rc = cpssDxChLedStreamClassManipulationSet(devNum
                                                            ,ledInterfaceNum
                                                            ,CPSS_DXCH_LED_PORT_TYPE_XG_E /* don't care , not applicable for BC3 */
                                                            ,classNum
                                                            ,&ledClassManip);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }
    /*--------------------------------------------------------------------------------------------------------*
     * LED. classesAndGroupConfig uses default setting , don't configure   cpssDxChLedStreamGroupConfigSet()  *
     *--------------------------------------------------------------------------------------------------------*
     * now configure port dependent LED configurations                                                        *
     *       - port led position (stream is hard wired                                                        *
     *       - invert polarity                                                                                *
     *--------------------------------------------------------------------------------------------------------*/
    for(portNum = 0; portNum < (appDemoPpConfigList[devNum].maxPortNumber); portNum++)
    {
        CPSS_ENABLER_PORT_SKIP_CHECK(devNum, portNum);

        rc = cpssDxChPortPhysicalPortMapGet(devNum, portNum, 1, /*OUT*/&portMap);
        if(rc != GT_OK)
        {
            continue;
        }

        position = findLedPositionByMac(portMap.interfaceNum, ledPositionTbl);
        if (position != APPDEMO_BAD_VALUE)
        {
            rc = cpssDxChLedStreamPortPositionSet(devNum, portNum, position);
            if(rc != GT_OK)
            {
                return rc;
            }
            rc = cpssDxChLedStreamPortClassPolarityInvertEnableSet(devNum,portNum,/*classNum*/2,/*invertEnable*/1);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return GT_OK;
}


/**
* @internal appDemoLedInterfacesInit function
* @endinternal
*
* @brief   configure led interface
*
* @param[in] dev                      - device number
* @param[in] bc2BoardType             - board type (used in setBC2 backward compatability
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoLedInterfacesInit
(
    GT_U8    dev,
    GT_U32   bc2BoardType
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_PP_CONFIG_STC *pDev;


    rc = GT_OK;
    pDev = PRV_CPSS_DXCH_PP_MAC(dev);

#ifdef CHX_FAMILY
    dev = isHirApp?appDemoPpConfigList[dev].devNum:dev;
#endif

    /* Init LED interfaces */
    switch(pDev->genInfo.devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
            switch (pDev->genInfo.devSubFamily)
            {
                case CPSS_PP_SUB_FAMILY_NONE_E:
                    rc = bobcat2LedInit(dev);
                    CPSS_ENABLER_DBG_TRACE_RC_MAC("bobcat2LedInit", rc);
                    break;
                case CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E:
                    if (APP_DEMO_CETUS_BOARD_DB_CNS == bc2BoardType)
                    {
                        rc = bobkCetusLedInit(dev,bc2BoardType);
                        CPSS_ENABLER_DBG_TRACE_RC_MAC("bobkCetusLedInit", rc);
                    }
                    else if (APP_DEMO_CAELUM_BOARD_DB_CNS == bc2BoardType)
                    {
                        rc = bobkCaelumLedInit(dev,bc2BoardType);
                        CPSS_ENABLER_DBG_TRACE_RC_MAC("bobkCetusLedInit", rc);
                    }
                    else
                    {
                        cpssOsPrintf("LED init was not done on [%d] board type\n", bc2BoardType);
                        rc = GT_OK;
                    }
                    break;
                default:
                    return GT_NOT_SUPPORTED;
            }
            break;

        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            rc = aldrinLedInit(dev);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("aldrinLedInit", rc);
            break;

        case CPSS_PP_FAMILY_DXCH_AC3X_E:
            CPSS_TBD_BOOKMARK       /* need to update it for proper board support */
            break;

        case CPSS_PP_FAMILY_DXCH_BOBCAT3_E:
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            rc = bobcat3LedInit(dev,bc2BoardType);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("bobcat3LedInit", rc);
            break;

        default:
            return GT_NOT_SUPPORTED;
    }
    return rc;
}



