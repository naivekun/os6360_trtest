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
* @file gtDbDxAc3x.c
*
* @brief Initialization functions for the AC3X - board.
*
* @version   1
********************************************************************************
*/
#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <appDemo/boardConfig/appDemoCfgMisc.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfigDefaults.h>
#include <appDemo/sysHwConfig/gtAppDemoTmConfig.h>
#include <appDemo/sysHwConfig/appDemoDb.h>
#include <appDemo/boardConfig/gtBoardsConfigFuncs.h>
#include <appDemo/boardConfig/gtDbDxBobcat2PhyConfig.h>
#include <appDemo/boardConfig/gtDbDxBobcat2Mappings.h>
#include <appDemo/boardConfig/gtDbDxBoardTypeConfig.h>
#include <appDemo/userExit/dxCh/appDemoDxChEventHandle.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortTx.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortPfc.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCn.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#ifdef PX_FAMILY
    #include <cpss/px/ingress/cpssPxIngress.h>
    #include <cpss/px/port/cpssPxPortCtrl.h>
#endif

#ifdef INCLUDE_UTF
#include <common/tgfCommon.h>
extern void utfSetMustUseOrigPorts(IN GT_BOOL  useOrigPorts);

#endif /*INCLUDE_UTF*/


#define NOT_VALID_CNS   0xFFFFFFFF
#define NOT_USED_CNS    0xEFEFEFEF

enum{
    /* the cpssInitSystem (32,1,0) - is '3 or 6 PHYs 1690' */
    boardRevId_default = 1,
    /* the cpssInitSystem (32,2,0) - is 'no PHYs 1690' */
    boardRevId_NO_PHY_1690 = 2,
    /* the cpssInitSystem (32,3,0) - is '4 PHYs 1690' */
    boardRevId_4_PHY_1690  = 3,
    /* the cpssInitSystem (32,4,0) - is '3 or 6 PHYs 1690' (same as 1) but using polling by default */
    boardRevId_polling_default = 4,
    /* the cpssInitSystem (32,5,0) - is '4 PHYs 1690' + 2 PHYs 2180 + PIPE*/
    boardRevId_4_PHY_1690_2_PHY_2180_PIPE = 5,
    boardRevId_last
};

/***********************/
/***** debug tools *****/
/*****  start      *****/
/***********************/

/* flag to set the code to handle/not the EGF filter on remote ports */
static GT_BOOL xcat3x_debug_GlobalEgfFilter = GT_TRUE;
/* flag to set the code to handle/not the TXQ shaper on remote ports */
static GT_BOOL xcat3x_debug_GlobalShaper = GT_TRUE;

/* function that set the code to handle/not the EGF filter on remote ports */
void xcat3x_debug_GlobalEgfFilterSet(IN GT_BOOL allow)
{
    xcat3x_debug_GlobalEgfFilter = allow;
}
/* function that set the code to handle/not the TXQ shaper on remote ports */
void xcat3x_debug_GlobalShaperSet(IN GT_BOOL allow)
{
    xcat3x_debug_GlobalShaper = allow;
}
/***********************/
/***** debug tools *****/
/*****  end        *****/
/***********************/

/* Amount of cascade ports used at this device */
static  GT_U32      ac3x_board_numCscdPorts;

static GT_U8   genericBoardRevision = 1;

/* genericBoardConfigFun will hold the generic board configuration */
/* we will use those pointers as "set default" values that we can override */
static GT_BOARD_CONFIG_FUNCS genericBoardConfigFun;

#define MAC_PORT_TO_88E1690_0_CNS   12  /* SERDES 12,13 */
#define MAC_PORT_TO_88E1690_1_CNS   14  /* SERDES 14,15 */
#define MAC_PORT_TO_88E1690_2_CNS   16  /* SERDES 16,17 */
#define MAC_PORT_TO_88E1690_3_CNS   18  /* SERDES 18,19 */
#define MAC_PORT_TO_88E1690_4_CNS   20  /* SERDES 20,21 */
#define MAC_PORT_TO_88E1690_5_CNS   22  /* SERDES 22,23 */
#define NA_PG                       0
#define MAC_PORT_98DX3248_TO_88E1690_0_CNS    0  /* SERDES  0, 1 */
#define MAC_PORT_98DX3248_TO_88E1690_1_CNS    2  /* SERDES  2, 3 */
#define MAC_PORT_98DX3248_TO_88E1690_2_CNS    4  /* SERDES  4, 5 */
#define MAC_PORT_98DX3248_TO_88E1690_3_CNS    6  /* SERDES  6, 7 */
#define MAC_PORT_98DX3248_TO_88E1690_4_CNS    8  /* SERDES  8, 9 */
#define MAC_PORT_98DX3248_TO_88E1690_5_CNS   10  /* SERDES 10,11 */

#define SIZE_OF_MAC(arr)    sizeof(arr)/sizeof(arr[0])

static GT_BOOL  usePollingOn88e1690LinkChange = GT_FALSE;

extern CPSS_DXCH_PORT_MAP_STC  *appDemoPortMapPtr;
/* number of elements in appDemoPortMapPtr */
extern GT_U32                  appDemoPortMapSize;

extern boardRevId2PortsInitList_STC *appDemo_boardRevId2PortsInitListPtr;
/* number of elements in appDemo_boardRevId2PortsInitListPtr */
extern GT_U32                  appDemo_boardRevId2PortsSize;

extern APPDEMO_BC2_SERDES_POLARITY_STC *appDemoPolarityArrayPtr;
extern GT_U32                           appDemoPolarityArraySize;

extern GT_STATUS userForceBoardType(IN GT_U32 boardType);

static GT_U32   ports_to_88e1690_arr[] = {72,73,74,75,76,77,NOT_VALID_CNS};
static GT_U32   mask_1690_Ports = 0;

enum{
    CCFC_TXQ_0  = 52,
    CCFC_TXQ_1  = 53,
    CCFC_TXQ_2  = 54,
    CCFC_TXQ_3  = 55,
    CCFC_TXQ_4  = 56,
    CCFC_TXQ_5  = 57
};

/* the cascade ports of the DX device not need TXQ port !
   because traffic is not pointed to those 'physical ports' */
/* traffic should not be directed to this tqx port */
#define CASCADE_PORT_NOT_NEED_TXQ_PORT_CNS          GT_NA

static CPSS_DXCH_PORT_SERDES_AC3X1690TX_PARAM_CONFIG_STC ac3xTxRxuaiCfg[] =
{ /*portMac*/  /*emph0*/  /*txAmp*/  /*emph1*/
    {12,       4,        0,           0},
    {14,       4,        0,           0},
    {16,       8,        0,           0},
    {18,       10,       0,           0},
    {20,       15,       0,           0},
    {22,       12,       0,           0},
    {24,       12,       0,           0},
    {26,       12,       0,           0},
    {28,       12,       0,           0},
    {30,       12,       0,           0},
};

static CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  schedulerProfile =
        CPSS_PORT_TX_SCHEDULER_PROFILE_10_E;

enum{ /* hold all needed constants for this function */
    ratePermillage_base = 1000,
    speed_10mbps   =   10*1000,/* value in Kbps */
    speed_100mbps  =  100*1000,/* value in Kbps */
    speed_1000mbps = 1000*1000,/* value in Kbps */
    speed_10Gbps   = 10*1000*1000,/* value in Kbps */
    baseline_value = 0x400800,/* Value must be higher than (0xffff*512/8) when PFC is used,
                                and this value allows in addition a relatively small 2KB burst. */
    burstSize_value = (baseline_value / _4K) + 1,/* Value must be higher than baseline,
                                                    and multiple of 4KB */

    epsilon_10Mb  = 20,  /* guarantee 10M */
    epsilon_100Mb = 180,  /* guarantee 100M */
    epsilon_1Gb   = 2200, /* guarantee 1G */

    /* Values for non-480 MHz core clock */
    epsilon_365MHz_10Mb  = 30,  /* guarantee 10M */
    epsilon_365MHz_100Mb = 600,  /* guarantee 100M */
    epsilon_365MHz_1Gb   = 2400, /* guarantee 1G */

    lastValue = BIT_16
};

static CPSS_DXCH_PORT_MAP_STC xcat3x_Map_24_remote_ports[] =
{ /* Port,            mappingType                              portGroup, intefaceNum, txQPort,      */
    {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,        0,       0}
   ,{   4, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,        4,       4}
   ,{   8, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,        8,       8}
   ,{   9, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,        9,       9}
   ,{  10, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       10,      10}
   ,{  11, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       11,      11}

   ,{  72, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       MAC_PORT_TO_88E1690_0_CNS,      CASCADE_PORT_NOT_NEED_TXQ_PORT_CNS}
   ,{  73, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       MAC_PORT_TO_88E1690_1_CNS,      CASCADE_PORT_NOT_NEED_TXQ_PORT_CNS}
   ,{  74, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       MAC_PORT_TO_88E1690_2_CNS,      CASCADE_PORT_NOT_NEED_TXQ_PORT_CNS}

   /* connection to 88E1690 #0 */
   ,{  12, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_0_CNS,      12}
   ,{  13, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_0_CNS,      13}
   ,{  14, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_0_CNS,      14}
   ,{  15, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_0_CNS,      15}
   ,{  16, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_0_CNS,      16}
   ,{  17, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_0_CNS,      17}
   ,{  18, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_0_CNS,      18}
   ,{  19, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_0_CNS,      19}

   /* connection to 88E1690 #1 */
   ,{  20, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_1_CNS,      20}
   ,{  21, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_1_CNS,      21}
   ,{  22, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_1_CNS,      22}
   ,{  23, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_1_CNS,      23}
   ,{  24, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_1_CNS,      24}
   ,{  25, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_1_CNS,      25}
   ,{  26, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_1_CNS,      26}
   ,{  27, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_1_CNS,      27}

   /* connection to 88E1690 #2 */
   ,{  28, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_2_CNS,      28}
   ,{  29, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_2_CNS,      29}
   ,{  30, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_2_CNS,      30}
   ,{  31, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_2_CNS,      31}
   ,{  32, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_2_CNS,      32}
   ,{  33, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_2_CNS,      33}
   ,{  34, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_2_CNS,      34}
   ,{  35, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_2_CNS,      35}


   ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,                   NA_PG,    GT_NA,      63}
};

static CPSS_DXCH_PORT_MAP_STC xcat3x_Map_48_remote_ports[] =
{ /* Port,            mappingType                              portGroup, intefaceNum, txQPort,      */
    {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,        0,       0}
   ,{   1, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,        1,       1}
   ,{   2, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,        2,       2}
   ,{   3, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,        3,       3}
   ,{   4, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,        4,       4}
   ,{   5, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,        5,       5}
   ,{   6, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,        6,       6}
   ,{   7, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,        7,       7}
   ,{   8, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,        8,       8}
   ,{   9, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,        9,       9}
   ,{  10, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       10,      10}
   ,{  11, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       11,      11}

   ,{  72, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       MAC_PORT_TO_88E1690_0_CNS,      CASCADE_PORT_NOT_NEED_TXQ_PORT_CNS}
   ,{  73, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       MAC_PORT_TO_88E1690_1_CNS,      CASCADE_PORT_NOT_NEED_TXQ_PORT_CNS}
   ,{  74, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       MAC_PORT_TO_88E1690_2_CNS,      CASCADE_PORT_NOT_NEED_TXQ_PORT_CNS}
   ,{  75, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       MAC_PORT_TO_88E1690_3_CNS,      CASCADE_PORT_NOT_NEED_TXQ_PORT_CNS}
   ,{  76, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       MAC_PORT_TO_88E1690_4_CNS,      CASCADE_PORT_NOT_NEED_TXQ_PORT_CNS}
   ,{  77, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       MAC_PORT_TO_88E1690_5_CNS,      CASCADE_PORT_NOT_NEED_TXQ_PORT_CNS}

   /* connection to 88E1690 #0 */
   ,{  12, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_0_CNS,      12}
   ,{  13, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_0_CNS,      13}
   ,{  14, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_0_CNS,      14}
   ,{  15, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_0_CNS,      15}
   ,{  16, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_0_CNS,      16}
   ,{  17, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_0_CNS,      17}
   ,{  18, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_0_CNS,      18}
   ,{  19, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_0_CNS,      19}

   /* connection to 88E1690 #1 */
   ,{  20, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_1_CNS,      20}
   ,{  21, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_1_CNS,      21}
   ,{  22, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_1_CNS,      22}
   ,{  23, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_1_CNS,      23}
   ,{  24, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_1_CNS,      24}
   ,{  25, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_1_CNS,      25}
   ,{  26, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_1_CNS,      26}
   ,{  27, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_1_CNS,      27}

   /* connection to 88E1690 #2 */
   ,{  28, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_2_CNS,      28}
   ,{  29, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_2_CNS,      29}
   ,{  30, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_2_CNS,      30}
   ,{  31, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_2_CNS,      31}
   ,{  32, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_2_CNS,      32}
   ,{  33, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_2_CNS,      33}
   ,{  34, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_2_CNS,      34}
   ,{  35, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_2_CNS,      35}

   /* connection to 88E1690 #3 */
   ,{  36, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_3_CNS,      36}
   ,{  37, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_3_CNS,      37}
   ,{  38, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_3_CNS,      38}
   ,{  39, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_3_CNS,      39}
   ,{  40, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_3_CNS,      40}
   ,{  41, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_3_CNS,      41}
   ,{  42, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_3_CNS,      42}
   ,{  43, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_3_CNS,      43}

   /* connection to 88E1690 #4 */
   ,{  44, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_4_CNS,      44}
   ,{  45, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_4_CNS,      45}
   ,{  46, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_4_CNS,      46}
   ,{  47, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_4_CNS,      47}
   ,{  48, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_4_CNS,      48}
   ,{  49, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_4_CNS,      49}
   ,{  50, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_4_CNS,      50}
   ,{  51, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_4_CNS,      51}

   /* connection to 88E1690 #5 */
   ,{  52, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_5_CNS,      64}
   ,{  53, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_5_CNS,      65}
   ,{  54, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_5_CNS,      66}
   ,{  55, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_5_CNS,      67}
   ,{  56, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_5_CNS,      68}
   ,{  57, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_5_CNS,      69}
   ,{  58, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_5_CNS,      70}
   ,{  59, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_5_CNS,      71}


   ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,                   NA_PG,    GT_NA,      63}
};

static CPSS_DXCH_PORT_MAP_STC xcat3plus_Map_48_remote_20x10G_ports[] =
{ /* Port,            mappingType                              portGroup, interfaceNum, txQPort,      */
    /* 20x10G ports */
    {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       20,       0}
   ,{   1, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       21,       1}
   ,{   2, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       22,       2}
   ,{   3, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       23,       3}
   ,{   4, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       24,       4}
   ,{   5, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       25,       5}
   ,{   6, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       26,       6}
   ,{   7, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       27,       7}
   ,{   8, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       28,       8}
   ,{   9, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       29,       9}
   ,{  10, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       30,      10}
   ,{  11, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       31,      11}
   ,{  64, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       12,      52}
   ,{  65, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       13,      53}
   ,{  66, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       14,      54}
   ,{  67, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       15,      55}
   ,{  68, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       16,      56}
   ,{  69, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       17,      57}
   ,{  70, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       18,      58}
   ,{  71, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       19,      59}

   ,{  72, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       MAC_PORT_98DX3248_TO_88E1690_0_CNS,      CASCADE_PORT_NOT_NEED_TXQ_PORT_CNS}
   ,{  73, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       MAC_PORT_98DX3248_TO_88E1690_1_CNS,      CASCADE_PORT_NOT_NEED_TXQ_PORT_CNS}
   ,{  74, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       MAC_PORT_98DX3248_TO_88E1690_2_CNS,      CASCADE_PORT_NOT_NEED_TXQ_PORT_CNS}
   ,{  75, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       MAC_PORT_98DX3248_TO_88E1690_3_CNS,      CASCADE_PORT_NOT_NEED_TXQ_PORT_CNS}
   ,{  76, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       MAC_PORT_98DX3248_TO_88E1690_4_CNS,      CASCADE_PORT_NOT_NEED_TXQ_PORT_CNS}
   ,{  77, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       MAC_PORT_98DX3248_TO_88E1690_5_CNS,      CASCADE_PORT_NOT_NEED_TXQ_PORT_CNS}

   /* connection to 88E1690 #0 */
   ,{  12, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_98DX3248_TO_88E1690_0_CNS,      12}
   ,{  13, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_98DX3248_TO_88E1690_0_CNS,      13}
   ,{  14, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_98DX3248_TO_88E1690_0_CNS,      14}
   ,{  15, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_98DX3248_TO_88E1690_0_CNS,      15}
   ,{  16, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_98DX3248_TO_88E1690_0_CNS,      16}
   ,{  17, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_98DX3248_TO_88E1690_0_CNS,      17}
   ,{  18, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_98DX3248_TO_88E1690_0_CNS,      18}
   ,{  19, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_98DX3248_TO_88E1690_0_CNS,      19}

   /* connection to 88E1690 #1 */
   ,{  20, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_98DX3248_TO_88E1690_1_CNS,      20}
   ,{  21, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_98DX3248_TO_88E1690_1_CNS,      21}
   ,{  22, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_98DX3248_TO_88E1690_1_CNS,      22}
   ,{  23, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_98DX3248_TO_88E1690_1_CNS,      23}
   ,{  24, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_98DX3248_TO_88E1690_1_CNS,      24}
   ,{  25, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_98DX3248_TO_88E1690_1_CNS,      25}
   ,{  26, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_98DX3248_TO_88E1690_1_CNS,      26}
   ,{  27, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_98DX3248_TO_88E1690_1_CNS,      27}

   /* connection to 88E1690 #2 */
   ,{  28, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_98DX3248_TO_88E1690_2_CNS,      28}
   ,{  29, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_98DX3248_TO_88E1690_2_CNS,      29}
   ,{  30, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_98DX3248_TO_88E1690_2_CNS,      30}
   ,{  31, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_98DX3248_TO_88E1690_2_CNS,      31}
   ,{  32, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_98DX3248_TO_88E1690_2_CNS,      32}
   ,{  33, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_98DX3248_TO_88E1690_2_CNS,      33}
   ,{  34, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_98DX3248_TO_88E1690_2_CNS,      34}
   ,{  35, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_98DX3248_TO_88E1690_2_CNS,      35}

   /* connection to 88E1690 #3 */
   ,{  36, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_98DX3248_TO_88E1690_3_CNS,      36}
   ,{  37, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_98DX3248_TO_88E1690_3_CNS,      37}
   ,{  38, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_98DX3248_TO_88E1690_3_CNS,      38}
   ,{  39, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_98DX3248_TO_88E1690_3_CNS,      39}
   ,{  40, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_98DX3248_TO_88E1690_3_CNS,      40}
   ,{  41, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_98DX3248_TO_88E1690_3_CNS,      41}
   ,{  42, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_98DX3248_TO_88E1690_3_CNS,      42}
   ,{  43, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_98DX3248_TO_88E1690_3_CNS,      43}

   /* connection to 88E1690 #4 */
   ,{  44, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_98DX3248_TO_88E1690_4_CNS,      44}
   ,{  45, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_98DX3248_TO_88E1690_4_CNS,      45}
   ,{  46, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_98DX3248_TO_88E1690_4_CNS,      46}
   ,{  47, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_98DX3248_TO_88E1690_4_CNS,      47}
   ,{  48, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_98DX3248_TO_88E1690_4_CNS,      48}
   ,{  49, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_98DX3248_TO_88E1690_4_CNS,      49}
   ,{  50, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_98DX3248_TO_88E1690_4_CNS,      50}
   ,{  51, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_98DX3248_TO_88E1690_4_CNS,      51}

   /* connection to 88E1690 #5 */
   ,{  52, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_98DX3248_TO_88E1690_5_CNS,      64}
   ,{  53, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_98DX3248_TO_88E1690_5_CNS,      65}
   ,{  54, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_98DX3248_TO_88E1690_5_CNS,      66}
   ,{  55, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_98DX3248_TO_88E1690_5_CNS,      67}
   ,{  56, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_98DX3248_TO_88E1690_5_CNS,      68}
   ,{  57, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_98DX3248_TO_88E1690_5_CNS,      69}
   ,{  58, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_98DX3248_TO_88E1690_5_CNS,      70}
   ,{  59, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_98DX3248_TO_88E1690_5_CNS,      71}


   ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,                   NA_PG,    GT_NA,      63}
};

static CPSS_DXCH_PORT_MAP_STC xcat3x_Map_0_remote_ports[] =
{ /* Port,            mappingType                              portGroup, intefaceNum, txQPort,       TM Enable ,     tmPortInd*/
    {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        0,       0,           GT_FALSE,          GT_NA}
   ,{   1, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        1,       1,           GT_FALSE,          GT_NA}
   ,{   2, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        2,       2,           GT_FALSE,          GT_NA}
   ,{   3, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        3,       3,           GT_FALSE,          GT_NA}
   ,{   4, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        4,       4,           GT_FALSE,          GT_NA}
   ,{   5, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        5,       5,           GT_FALSE,          GT_NA}
   ,{   6, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        6,       6,           GT_FALSE,          GT_NA}
   ,{   7, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        7,       7,           GT_FALSE,          GT_NA}
   ,{   8, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        8,       8,           GT_FALSE,          GT_NA}
   ,{   9, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        9,       9,           GT_FALSE,          GT_NA}
   ,{  10, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       10,      10,           GT_FALSE,          GT_NA}
   ,{  11, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       11,      11,           GT_FALSE,          GT_NA}
   ,{  12, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       12,      12,           GT_FALSE,          GT_NA}
   ,{  13, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       13,      13,           GT_FALSE,          GT_NA}
   ,{  14, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       14,      14,           GT_FALSE,          GT_NA}
   ,{  15, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       15,      15,           GT_FALSE,          GT_NA}
   ,{  16, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       16,      16,           GT_FALSE,          GT_NA}
   ,{  17, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       17,      17,           GT_FALSE,          GT_NA}
   ,{  18, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       18,      18,           GT_FALSE,          GT_NA}
   ,{  19, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       19,      19,           GT_FALSE,          GT_NA}
   ,{  20, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       20,      20,           GT_FALSE,          GT_NA}
   ,{  21, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       21,      21,           GT_FALSE,          GT_NA}
   ,{  22, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       22,      22,           GT_FALSE,          GT_NA}
   ,{  23, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       23,      23,           GT_FALSE,          GT_NA}
   ,{  24, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       24,      24,           GT_FALSE,          GT_NA}
   ,{  25, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       25,      25,           GT_FALSE,          GT_NA}
   ,{  26, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       26,      26,           GT_FALSE,          GT_NA}
   ,{  27, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       27,      27,           GT_FALSE,          GT_NA}
   ,{  28, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       28,      28,           GT_FALSE,          GT_NA}
   ,{  29, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       29,      29,           GT_FALSE,          GT_NA}
   ,{  30, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       30,      30,           GT_FALSE,          GT_NA}
   ,{  31, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       31,      31,           GT_FALSE,          GT_NA}
   ,{  32, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       32,      32,           GT_FALSE,          GT_NA}
   ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,                   0,    GT_NA,      63,           GT_FALSE,          GT_NA}
};

static CPSS_DXCH_PORT_MAP_STC xcat3x_Map_32_remote_ports[] =
{ /* Port,            mappingType                              portGroup, intefaceNum, txQPort,      */
    {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,        0,       0}

   ,{   4, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,        4,       4}
   ,{   5, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,        5,       5}
   ,{   6, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,        6,       6}
   ,{   7, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,        7,       7}
   ,{   8, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,        8,       8}
   ,{   9, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,        9,       9}
   ,{  10, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       10,      10}
   ,{  11, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       11,      11}
/*************************************/
    /* NOTE: next ports uses interface number that is not 1:1 with physical port number */
    /* NOTE: next ports uses TXQ-queue        that is     1:1 with physical port number */
   ,{  12, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       24,      12}
   ,{  13, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       25,      13}
   ,{  14, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       26,      14}
   ,{  15, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       27,      15}
   ,{  16, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       28,      16}
   ,{  17, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       29,      17}
   ,{  18, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       30,      18}
   ,{  19, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       31,      19}
/*************************************/
   ,{  72, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       MAC_PORT_TO_88E1690_0_CNS,      CASCADE_PORT_NOT_NEED_TXQ_PORT_CNS}
   ,{  73, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       MAC_PORT_TO_88E1690_1_CNS,      CASCADE_PORT_NOT_NEED_TXQ_PORT_CNS}
/*1690#2 not exists :   ,{  74, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       MAC_PORT_TO_88E1690_2_CNS,      CASCADE_PORT_NOT_NEED_TXQ_PORT_CNS}*/
   ,{  75, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       MAC_PORT_TO_88E1690_3_CNS,      CASCADE_PORT_NOT_NEED_TXQ_PORT_CNS}
   ,{  76, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       MAC_PORT_TO_88E1690_4_CNS,      CASCADE_PORT_NOT_NEED_TXQ_PORT_CNS}
/*1690#5 not exists :   ,{  77, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       MAC_PORT_TO_88E1690_5_CNS,      CASCADE_PORT_NOT_NEED_TXQ_PORT_CNS}*/
/*************************************/

   /* connection to 88E1690 #0 */
   ,{  20, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_0_CNS,      20}
   ,{  21, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_0_CNS,      21}
   ,{  22, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_0_CNS,      22}
   ,{  23, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_0_CNS,      23}
   ,{  24, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_0_CNS,      24}
   ,{  25, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_0_CNS,      25}
   ,{  26, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_0_CNS,      26}
   ,{  27, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_0_CNS,      27}

   /* connection to 88E1690 #1 */
   ,{  28, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_1_CNS,      28}
   ,{  29, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_1_CNS,      29}
   ,{  30, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_1_CNS,      30}
   ,{  31, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_1_CNS,      31}
   ,{  32, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_1_CNS,      32}
   ,{  33, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_1_CNS,      33}
   ,{  34, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_1_CNS,      34}
   ,{  35, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_1_CNS,      35}

   /* connection to 88E1690 #3 */
   ,{  36, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_3_CNS,      36}
   ,{  37, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_3_CNS,      37}
   ,{  38, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_3_CNS,      38}
   ,{  39, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_3_CNS,      39}
   ,{  40, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_3_CNS,      40}
   ,{  41, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_3_CNS,      41}
   ,{  42, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_3_CNS,      42}
   ,{  43, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_3_CNS,      43}

   /* connection to 88E1690 #4 */
   ,{  44, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_4_CNS,      44}
   ,{  45, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_4_CNS,      45}
   ,{  46, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_4_CNS,      46}
   ,{  47, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_4_CNS,      47}
   ,{  48, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_4_CNS,      48}
   ,{  49, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_4_CNS,      49}
   ,{  50, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_4_CNS,      50}
   ,{  51, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, MAC_PORT_TO_88E1690_4_CNS,      51}

   ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,                   NA_PG,    GT_NA,      63}
};

#ifdef PX_FAMILY
static CPSS_DXCH_PORT_MAP_STC xcat3x_Map_4phy1690_2phy2180_pipe[] =
{ /* Port,            mappingType                              portGroup, intefaceNum, txQPort,      */
    /* 12.5G * 4 to PIPE */
    {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,  0,   0}
   ,{   1, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,  1,   1}
   ,{   2, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,  2,   2}
   ,{   3, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,  3,   3}

   /* 10G * 4 to front panel */
   ,{   4, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,  4,   4}
   ,{   5, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,  5,   5}
   ,{   6, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,  6,   6}
   ,{   7, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,  7,   7}

   /* PHY2180(0) 2.5G * 16 */
   ,{   8, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,  8,   8}
   ,{   9, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,  9,   9}
   ,{  10, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG, 10,  10}
   ,{  11, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG, 11,  11}
   ,{  12, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG, 12,  12}
   ,{  13, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG, 13,  13}
   ,{  14, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG, 14,  14}
   ,{  15, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG, 15,  15}
   ,{  16, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG, 16,  16}
   ,{  17, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG, 17,  17}
   ,{  18, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG, 18,  18}
   ,{  19, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG, 19,  19}
   ,{  20, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG, 20,  20}
   ,{  21, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG, 21,  21}
   ,{  22, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG, 22,  22}
   ,{  23, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG, 23,  23}

   /* PHY1690(0) RXAUI */
   ,{  72, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG, 24,  CASCADE_PORT_NOT_NEED_TXQ_PORT_CNS}
   /* PHY1690(1) RXAUI */
   ,{  73, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG, 26,  CASCADE_PORT_NOT_NEED_TXQ_PORT_CNS}
   /* PHY1690(2) RXAUI */
   ,{  75, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG, 28,  CASCADE_PORT_NOT_NEED_TXQ_PORT_CNS}
   /* PHY1690(3) RXAUI */
   ,{  76, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG, 30,  CASCADE_PORT_NOT_NEED_TXQ_PORT_CNS}
/*************************************/

   /* connection to 88E1690 #0 */
   ,{  24, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, 24,   31}
   ,{  25, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, 24,   32}
   ,{  26, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, 24,   33}
   ,{  27, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, 24,   34}
   ,{  28, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, 24,   35}
   ,{  29, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, 24,   36}
   ,{  30, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, 24,   37}
   ,{  31, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, 24,   38}

   /* connection to 88E1690 #1 */
   ,{  32, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, 26,   39}
   ,{  33, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, 26,   40}
   ,{  34, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, 26,   41}
   ,{  35, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, 26,   42}
   ,{  36, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, 26,   43}
   ,{  37, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, 26,   44}
   ,{  38, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, 26,   45}
   ,{  39, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, 26,   46}

    /* connection to 88E1690 #3 */
   ,{  40, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, 28,   47}
   ,{  41, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, 28,   48}
   ,{  42, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, 28,   49}
   ,{  43, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, 28,   50}
   ,{  44, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, 28,   51}
   ,{  45, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, 28,   58}
   ,{  46, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, 28,   59}
   ,{  47, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, 28,   60}

   /* connection to 88E1690 #4 */
   ,{  48, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, 30,   61}
   ,{  49, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, 30,   62}
   ,{  50, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, 30,   64}
   ,{  51, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, 30,   65}
   ,{  52, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, 30,   66}
   ,{  53, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, 30,   67}
   ,{  54, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, 30,   68}
   ,{  55, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG, 30,   69}

   ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,                   NA_PG, GT_NA,63}
};
#endif

/* xCat3X provide 161 Gbps BW */
static PortInitList_STC portInitlist_xcat3x_480MHz_29_1_BW_161_32x10G[] =
{
    /* 2*40G ports */
     { PORT_LIST_TYPE_LIST,      {0,4,          APP_INV_PORT_CNS },  CPSS_PORT_SPEED_40000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
    /* 4*10G ports */
    ,{ PORT_LIST_TYPE_LIST,      {8,9,10,11,    APP_INV_PORT_CNS },  CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E }

    /* 6 cascade ports : 10G RXAUI ports */
    ,{ PORT_LIST_TYPE_LIST,      {72,73,74,75,76,77, APP_INV_PORT_CNS },  CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_RXAUI_E   }

    /* 48 remote physical ports */
    ,{ PORT_LIST_TYPE_INTERVAL,  {12,12+(8*6 - 1),1, APP_INV_PORT_CNS },  CPSS_PORT_SPEED_1000_E,   CPSS_PORT_INTERFACE_MODE_SGMII_E  }

    ,{ PORT_LIST_TYPE_EMPTY,     {               APP_INV_PORT_CNS},  CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E     }
};

static PortInitList_STC portInitlist_xcat3x_480MHz_rev_2[] =
{
    /* 1*40G ports */
     { PORT_LIST_TYPE_LIST,      {0,           APP_INV_PORT_CNS },   CPSS_PORT_SPEED_40000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
    /* 4*10G ports */
    ,{ PORT_LIST_TYPE_INTERVAL,  {4,  7, 1,      APP_INV_PORT_CNS},  CPSS_PORT_SPEED_10000_E,   CPSS_PORT_INTERFACE_MODE_SR_LR_E  }
    /* 24*2.5G ports  */
    ,{ PORT_LIST_TYPE_INTERVAL,  {8, 31, 1,      APP_INV_PORT_CNS},  CPSS_PORT_SPEED_2500_E,   CPSS_PORT_INTERFACE_MODE_SGMII_E    }
    ,{ PORT_LIST_TYPE_LIST,      {32,            APP_INV_PORT_CNS},  CPSS_PORT_SPEED_1000_E,    CPSS_PORT_INTERFACE_MODE_SGMII_E   }
    ,{ PORT_LIST_TYPE_EMPTY,     {               APP_INV_PORT_CNS},  CPSS_PORT_SPEED_NA_E,      CPSS_PORT_INTERFACE_MODE_NA_E      }
};

#ifdef PX_FAMILY
/* xCat3X RD-AC3X-32G16POE-A */
static PortInitList_STC portInitlist_xcat3x_480MHz_rev_5[] =
{
    /* 2*25G ports */
     { PORT_LIST_TYPE_LIST,      {0, 2,         APP_INV_PORT_CNS },    CPSS_PORT_SPEED_25000_E,  CPSS_PORT_INTERFACE_MODE_KR2_E    }

    /* 4*10G ports to the front panel */
    ,{ PORT_LIST_TYPE_LIST,      {4,5,6,7,      APP_INV_PORT_CNS },    CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E    }

    /* 16*2.5G ports for 2180 PHYs*/
    ,{ PORT_LIST_TYPE_INTERVAL,  {8,23,1,       APP_INV_PORT_CNS },    CPSS_PORT_SPEED_2500_E,  CPSS_PORT_INTERFACE_MODE_SGMII_E  }

    /* 4 cascade ports : 10G RXAUI ports */
    ,{ PORT_LIST_TYPE_LIST,      {72,73,75,76,  APP_INV_PORT_CNS },    CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_RXAUI_E }

    /* 32 remote physical ports */
    ,{ PORT_LIST_TYPE_INTERVAL,  {24,24+(8*4 - 1),1, APP_INV_PORT_CNS},CPSS_PORT_SPEED_1000_E,   CPSS_PORT_INTERFACE_MODE_SGMII_E }

    ,{ PORT_LIST_TYPE_EMPTY,     {               APP_INV_PORT_CNS},    CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E    }
};
#endif

/* xCat3X provide 161 Gbps BW */
static PortInitList_STC portInitlist_xcat3x_480MHz_rev_3[] =
{
    /* 1*40G ports */
     { PORT_LIST_TYPE_LIST,      {0,           APP_INV_PORT_CNS },    CPSS_PORT_SPEED_40000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
    /* 16*2.5G ports */
    ,{ PORT_LIST_TYPE_INTERVAL,  {4,19,1,       APP_INV_PORT_CNS },    CPSS_PORT_SPEED_2500_E,  CPSS_PORT_INTERFACE_MODE_SGMII_E }

    /* 4 cascade ports : 10G RXAUI ports */
    ,{ PORT_LIST_TYPE_LIST,      {72,73,75,76,  APP_INV_PORT_CNS },    CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_RXAUI_E   }

    /* 32 remote physical ports */
    ,{ PORT_LIST_TYPE_INTERVAL,  {20,20+(8*4 - 1),1, APP_INV_PORT_CNS},CPSS_PORT_SPEED_1000_E,   CPSS_PORT_INTERFACE_MODE_SGMII_E  }

    ,{ PORT_LIST_TYPE_EMPTY,     {               APP_INV_PORT_CNS},    CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E     }
};

#ifdef PX_FAMILY
static PortInitList_STC portInitlist_pipe[] =
{
     /* 2*25G KR2 ports */
     { PORT_LIST_TYPE_INTERVAL,  {4,  6, 2, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E , CPSS_PORT_INTERFACE_MODE_KR2_E}
     /* 2*25G  KR ports */
    ,{ PORT_LIST_TYPE_INTERVAL,  {13,15, 2, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}

    ,{ PORT_LIST_TYPE_EMPTY,     {         APP_INV_PORT_CNS},  CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E}
};
#endif

static boardRevId2PortsInitList_STC prv_xcat3x_boardRevId2PortsInitList[]=
{
    /* clock, revid,  port list */
     {  480,   boardRevId_default,     CPSS_PP_SUB_FAMILY_NONE_E, 0x7,  &portInitlist_xcat3x_480MHz_29_1_BW_161_32x10G[0] } /* 1G to CPU */
};

static boardRevId2PortsInitList_STC prv_xcat3x_rev_2_PortsInitList[]=
{
    {  480,   boardRevId_NO_PHY_1690, CPSS_PP_SUB_FAMILY_NONE_E, 0x7,  &portInitlist_xcat3x_480MHz_rev_2[0] } /* 1G to CPU */
};

static boardRevId2PortsInitList_STC prv_xcat3x_rev_3_PortsInitList[]=
{
    {  480,   boardRevId_4_PHY_1690,  CPSS_PP_SUB_FAMILY_NONE_E, 0x7,  &portInitlist_xcat3x_480MHz_rev_3[0] } /* 1G to CPU */
};

#ifdef PX_FAMILY
static boardRevId2PortsInitList_STC prv_xcat3x_rev_5_PortsInitList[]=
{
    {  480,   boardRevId_4_PHY_1690_2_PHY_2180_PIPE,  CPSS_PP_SUB_FAMILY_NONE_E, 0x7,  &portInitlist_xcat3x_480MHz_rev_5[0] } /* 1G to CPU */
};
#endif

/* xCat3X provide 161 Gbps BW */
static PortInitList_STC portInitlist_xcat3plus_480MHz_BW_161_32x10G[] =
{
    /* 12*10G ports */
     { PORT_LIST_TYPE_INTERVAL,  {0,11,1,        APP_INV_PORT_CNS },  CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E }

    /* 6 cascade ports : 10G RXAUI ports */
    ,{ PORT_LIST_TYPE_LIST,      {72,73,74,75,76,77, APP_INV_PORT_CNS },  CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_RXAUI_E   }

    /* 48 remote physical ports */
    ,{ PORT_LIST_TYPE_INTERVAL,  {12,12+(8*6 - 1),1, APP_INV_PORT_CNS },  CPSS_PORT_SPEED_1000_E,   CPSS_PORT_INTERFACE_MODE_SGMII_E  }

    /*  8*10G ports */
    ,{ PORT_LIST_TYPE_INTERVAL,  {64,71,1,        APP_INV_PORT_CNS },  CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E }

    ,{ PORT_LIST_TYPE_EMPTY,     {               APP_INV_PORT_CNS},  CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E     }
};

static boardRevId2PortsInitList_STC prv_xcat3plus_boardRevId2PortsInitList[]=
{
    /* clock, revid,  port list */
     {  480,   boardRevId_default,     CPSS_PP_SUB_FAMILY_NONE_E, 0x7,  &portInitlist_xcat3plus_480MHz_BW_161_32x10G[0] } /* 1G to CPU */
};

static APPDEMO_BC2_SERDES_POLARITY_STC xcat3xA0PolarityArray[] =
{
    { 0, 0x1, GT_FALSE,     GT_FALSE} /* dummy*/
};


#define REMOTE_PHYSICAL_TO_88E1690_PORT_MAC(basePhysicalPort) \
    {basePhysicalPort+1 , 1},                                 \
    {basePhysicalPort+2 , 2},                                 \
    {basePhysicalPort+3 , 3},                                 \
    {basePhysicalPort+4 , 4},                                 \
    {basePhysicalPort+5 , 5},                                 \
    {basePhysicalPort+6 , 6},                                 \
    {basePhysicalPort+7 , 7},                                 \
    {basePhysicalPort+8 , 8}

static CPSS_DXCH_CFG_REMOTE_PHY_MAC_PORT_INFO_STC macPorts[][8] =
{
     {REMOTE_PHYSICAL_TO_88E1690_PORT_MAC(11+8*0)}   /* ports of 88e1690 #1 */
    ,{REMOTE_PHYSICAL_TO_88E1690_PORT_MAC(11+8*1)}   /* ports of 88e1690 #2 */
    ,{REMOTE_PHYSICAL_TO_88E1690_PORT_MAC(11+8*2)}   /* ports of 88e1690 #3 */
    ,{REMOTE_PHYSICAL_TO_88E1690_PORT_MAC(11+8*3)}   /* ports of 88e1690 #4 */
    ,{REMOTE_PHYSICAL_TO_88E1690_PORT_MAC(11+8*4)}   /* ports of 88e1690 #5 */
    ,{REMOTE_PHYSICAL_TO_88E1690_PORT_MAC(11+8*5)}   /* ports of 88e1690 #6 */
};

static CPSS_DXCH_CFG_REMOTE_PHY_MAC_INFO_STC   physicalToInfoArr[] =
{
      {CPSS_DXCH_CFG_REMOTE_PHY_MAC_TYPE_88E1690_E ,CPSS_CHANNEL_SMI_E, CPSS_PHY_SMI_INTERFACE_0_E,
        0x1/*busBaseAddr*/,20/*interruptMultiPurposePinIndex*/ ,CCFC_TXQ_0/*internalFcTxqPort*/,
        8/*numOfRemotePorts*/, &macPorts[0][0]
         , GT_FALSE,    /* Overidden by appDemoDb variable */
         CPSS_NET_FIRST_USER_DEFINED_E + 33
      }

     ,{CPSS_DXCH_CFG_REMOTE_PHY_MAC_TYPE_88E1690_E ,CPSS_CHANNEL_SMI_E, CPSS_PHY_SMI_INTERFACE_0_E,
        0x2/*busBaseAddr*/,21/*interruptMultiPurposePinIndex*/ ,CCFC_TXQ_1/*internalFcTxqPort*/,
        8/*numOfRemotePorts*/, &macPorts[1][0]
         , GT_FALSE,    /* Overidden by appDemoDb variable */
         CPSS_NET_FIRST_USER_DEFINED_E + 33
      }

     ,{CPSS_DXCH_CFG_REMOTE_PHY_MAC_TYPE_88E1690_E ,CPSS_CHANNEL_SMI_E, CPSS_PHY_SMI_INTERFACE_0_E,
        0x3/*busBaseAddr*/,22/*interruptMultiPurposePinIndex*/ ,CCFC_TXQ_2/*internalFcTxqPort*/,
        8/*numOfRemotePorts*/, &macPorts[2][0]
         , GT_FALSE,    /* Overidden by appDemoDb variable */
         CPSS_NET_FIRST_USER_DEFINED_E + 33
      }

     ,{CPSS_DXCH_CFG_REMOTE_PHY_MAC_TYPE_88E1690_E ,CPSS_CHANNEL_SMI_E, CPSS_PHY_SMI_INTERFACE_1_E,
        0x4/*busBaseAddr*/,28/*interruptMultiPurposePinIndex*/ ,CCFC_TXQ_3/*internalFcTxqPort*/,
        8/*numOfRemotePorts*/, &macPorts[3][0]
         , GT_FALSE,    /* Overidden by appDemoDb variable */
         CPSS_NET_FIRST_USER_DEFINED_E + 33
      }

     ,{CPSS_DXCH_CFG_REMOTE_PHY_MAC_TYPE_88E1690_E ,CPSS_CHANNEL_SMI_E, CPSS_PHY_SMI_INTERFACE_1_E,
        0x5/*busBaseAddr*/,29/*interruptMultiPurposePinIndex*/ ,CCFC_TXQ_4/*internalFcTxqPort*/,
        8/*numOfRemotePorts*/, &macPorts[4][0]
         , GT_FALSE,    /* Overidden by appDemoDb variable */
         CPSS_NET_FIRST_USER_DEFINED_E + 33
      }

     ,{CPSS_DXCH_CFG_REMOTE_PHY_MAC_TYPE_88E1690_E ,CPSS_CHANNEL_SMI_E, CPSS_PHY_SMI_INTERFACE_1_E,
        0x6/*busBaseAddr*/,30/*interruptMultiPurposePinIndex*/ ,CCFC_TXQ_5/*internalFcTxqPort*/,
        8/*numOfRemotePorts*/, &macPorts[5][0]
         , GT_FALSE,    /* Overidden by appDemoDb variable */
         CPSS_NET_FIRST_USER_DEFINED_E + 33
      }

    ,{NOT_VALID_CNS}
};

static CPSS_DXCH_CFG_REMOTE_PHY_MAC_PORT_INFO_STC macPorts_rev_5[][8] =
{
     {REMOTE_PHYSICAL_TO_88E1690_PORT_MAC(24+8*0)}   /* ports of 88e1690 #1 */
    ,{REMOTE_PHYSICAL_TO_88E1690_PORT_MAC(24+8*1)}   /* ports of 88e1690 #2 */
    ,{{NOT_USED_CNS}}                                /* dummy entry */
    ,{REMOTE_PHYSICAL_TO_88E1690_PORT_MAC(24+8*2)}   /* ports of 88e1690 #3 */
    ,{REMOTE_PHYSICAL_TO_88E1690_PORT_MAC(24+8*3)}   /* ports of 88e1690 #4 */
    ,{{NOT_USED_CNS}}                                /* dummy entry */
};

/* Global pointer to connection info table between remote device and local device */
static CPSS_DXCH_CFG_REMOTE_PHY_MAC_INFO_STC   * physicalToInfoArrGlobalPtr;
/* Global pointer to connection info table between (PHY port number) remote MAC port number
   and it's (DX port number) remote physical port number */
static CPSS_DXCH_CFG_REMOTE_PHY_MAC_PORT_INFO_STC (*macPortsArrGlobalPtr)[8];
static GT_U32 macPortsSize;

static CPSS_DXCH_CFG_REMOTE_PHY_MAC_INFO_STC   physicalToInfoArr_rev_5[] =
{
      {CPSS_DXCH_CFG_REMOTE_PHY_MAC_TYPE_88E1690_E ,CPSS_CHANNEL_SMI_E, CPSS_PHY_SMI_INTERFACE_1_E,
        0x4/*busBaseAddr*/,28/*interruptMultiPurposePinIndex*/ ,CCFC_TXQ_0/*internalFcTxqPort*/,
        8/*numOfRemotePorts*/, &macPorts_rev_5[0][0]}

      ,{CPSS_DXCH_CFG_REMOTE_PHY_MAC_TYPE_88E1690_E ,CPSS_CHANNEL_SMI_E, CPSS_PHY_SMI_INTERFACE_1_E,
        0x3/*busBaseAddr*/,27/*interruptMultiPurposePinIndex*/ ,CCFC_TXQ_1/*internalFcTxqPort*/,
        8/*numOfRemotePorts*/, &macPorts_rev_5[1][0]}

      ,{NOT_USED_CNS}                                  /* dummy entry */

     ,{CPSS_DXCH_CFG_REMOTE_PHY_MAC_TYPE_88E1690_E ,CPSS_CHANNEL_SMI_E, CPSS_PHY_SMI_INTERFACE_0_E,
        0x1/*busBaseAddr*/,21/*interruptMultiPurposePinIndex*/ ,CCFC_TXQ_2/*internalFcTxqPort*/,
        8/*numOfRemotePorts*/, &macPorts_rev_5[3][0]}

     ,{CPSS_DXCH_CFG_REMOTE_PHY_MAC_TYPE_88E1690_E ,CPSS_CHANNEL_SMI_E, CPSS_PHY_SMI_INTERFACE_0_E,
        0x2/*busBaseAddr*/,22/*interruptMultiPurposePinIndex*/ ,CCFC_TXQ_3/*internalFcTxqPort*/,
        8/*numOfRemotePorts*/, &macPorts_rev_5[4][0]}

     ,{NOT_USED_CNS}                                  /* dummy entry */

    ,{NOT_VALID_CNS}
};

/* the Caelum RUnit connected to the CPS sub system ... we don't want to go there ...*/
/* so we are using 'polling' on link status change ... */
#define POLLING__INTERRUPT_NOT_VIA_PP_CNS    0xFFFFFFFF
static CPSS_DXCH_CFG_REMOTE_PHY_MAC_INFO_STC   polling__physicalToInfoArr[] =
{
      {CPSS_DXCH_CFG_REMOTE_PHY_MAC_TYPE_88E1690_E ,CPSS_CHANNEL_SMI_E, CPSS_PHY_SMI_INTERFACE_0_E,
        0x1/*busBaseAddr*/,POLLING__INTERRUPT_NOT_VIA_PP_CNS ,CCFC_TXQ_0/*internalFcTxqPort*/,
        8/*numOfRemotePorts*/, &macPorts[0][0]
         , GT_FALSE,    /* Overidden by appDemoDb variable */
         CPSS_NET_FIRST_USER_DEFINED_E + 33
      }

     ,{CPSS_DXCH_CFG_REMOTE_PHY_MAC_TYPE_88E1690_E ,CPSS_CHANNEL_SMI_E, CPSS_PHY_SMI_INTERFACE_0_E,
        0x2/*busBaseAddr*/,POLLING__INTERRUPT_NOT_VIA_PP_CNS ,CCFC_TXQ_1/*internalFcTxqPort*/,
        8/*numOfRemotePorts*/, &macPorts[1][0]
         , GT_FALSE,    /* Overidden by appDemoDb variable */
         CPSS_NET_FIRST_USER_DEFINED_E + 33
      }

     ,{CPSS_DXCH_CFG_REMOTE_PHY_MAC_TYPE_88E1690_E ,CPSS_CHANNEL_SMI_E, CPSS_PHY_SMI_INTERFACE_0_E,
        0x3/*busBaseAddr*/,POLLING__INTERRUPT_NOT_VIA_PP_CNS ,CCFC_TXQ_2/*internalFcTxqPort*/,
        8/*numOfRemotePorts*/, &macPorts[2][0]
         , GT_FALSE,    /* Overidden by appDemoDb variable */
         CPSS_NET_FIRST_USER_DEFINED_E + 33
      }

     ,{CPSS_DXCH_CFG_REMOTE_PHY_MAC_TYPE_88E1690_E ,CPSS_CHANNEL_SMI_E, CPSS_PHY_SMI_INTERFACE_1_E,
        0x4/*busBaseAddr*/,POLLING__INTERRUPT_NOT_VIA_PP_CNS ,CCFC_TXQ_3/*internalFcTxqPort*/,
        8/*numOfRemotePorts*/, &macPorts[3][0]
         , GT_FALSE,    /* Overidden by appDemoDb variable */
         CPSS_NET_FIRST_USER_DEFINED_E + 33
      }

     ,{CPSS_DXCH_CFG_REMOTE_PHY_MAC_TYPE_88E1690_E ,CPSS_CHANNEL_SMI_E, CPSS_PHY_SMI_INTERFACE_1_E,
        0x5/*busBaseAddr*/,POLLING__INTERRUPT_NOT_VIA_PP_CNS ,CCFC_TXQ_4/*internalFcTxqPort*/,
        8/*numOfRemotePorts*/, &macPorts[4][0]
         , GT_FALSE,    /* Overidden by appDemoDb variable */
         CPSS_NET_FIRST_USER_DEFINED_E + 33
      }

     ,{CPSS_DXCH_CFG_REMOTE_PHY_MAC_TYPE_88E1690_E ,CPSS_CHANNEL_SMI_E, CPSS_PHY_SMI_INTERFACE_1_E,
        0x6/*busBaseAddr*/,POLLING__INTERRUPT_NOT_VIA_PP_CNS ,CCFC_TXQ_5/*internalFcTxqPort*/,
        8/*numOfRemotePorts*/, &macPorts[5][0]
         , GT_FALSE,    /* Overidden by appDemoDb variable */
         CPSS_NET_FIRST_USER_DEFINED_E + 33
      }

    ,{NOT_VALID_CNS}
};


typedef struct{
    GT_BOOL     linkUp;
}POLLING__PORT_INFO_STC;

typedef struct{
    GT_BOOL isValid;
    POLLING__PORT_INFO_STC portsInfo[8];
}POLLING__88E1690_INFO_STC;

static POLLING__88E1690_INFO_STC polling__info_macPorts[6];

typedef struct{
    GT_BOOL valid;
    CPSS_HW_INFO_STC hwInfo[APP_DEMO_CPSS_MAX_NUM_PORT_GROUPS_CNS];
} PIPE_CONFIGURATION_STC;

#ifdef PX_FAMILY
static PIPE_CONFIGURATION_STC pipeConfig;

extern GT_STATUS px_appDemoInitSequence
(
    IN GT_U32             devIndex,
    IN GT_SW_DEV_NUM      devNum,
    IN CPSS_HW_INFO_STC   *hwInfoPtr,
    IN PortInitList_STC   *portInitListPtr
);
#endif

static GT_STATUS prvAppDemoPortXgPscLanesSwapSet
(
    GT_U8    devNum
    )
{
    GT_U32 rxSerdesLaneArr[CPSS_DXCH_PORT_XG_PSC_LANES_NUM_CNS] = { 2, 3, 0, 1 };
    GT_U32 txSerdesLaneArr[CPSS_DXCH_PORT_XG_PSC_LANES_NUM_CNS] = { 2, 3, 0, 1 };

    GT_U32 rx_1SerdesLaneArr[CPSS_DXCH_PORT_XG_PSC_LANES_NUM_CNS] = { 2, 3, 0, 1 };
    GT_U32 tx_1SerdesLaneArr[CPSS_DXCH_PORT_XG_PSC_LANES_NUM_CNS] = { 2, 3, 0, 1 };

    GT_STATUS rc;

    rc = cpssDxChPortXgPscLanesSwapSet(devNum, 75, rxSerdesLaneArr, txSerdesLaneArr);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortXgPscLanesSwapSet", rc);
    if (GT_OK != rc)
    {
        return rc;
    }

    rc = cpssDxChPortXgPscLanesSwapSet(devNum, 76, rx_1SerdesLaneArr, tx_1SerdesLaneArr);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortXgPscLanesSwapSet", rc);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

static GT_STATUS prvAppDemoLedInit
(
    GT_U8    devNum,
    GT_U8    boardRevId
)
{
    GT_STATUS                       rc;
    GT_U32                          ledInterfaceNum;
    CPSS_DXCH_LED_PHY_PORT_CONF_STC ledPerPortConfig;
    GT_U32                          classNum;
    CPSS_LED_CONF_STC               ledConfig;
    CPSS_LED_CLASS_MANIPULATION_STC ledClassManip;
    GT_U32 i;

    if (boardRevId != boardRevId_4_PHY_1690_2_PHY_2180_PIPE)
    {
        return GT_OK;
    }


    ledPerPortConfig.led0Select = CPSS_DXCH_LED_PHY_LINK_ACT;
    ledPerPortConfig.led1Select = CPSS_DXCH_LED_PHY_LINK_ACT;
    ledPerPortConfig.pulseStretch = 2;
    ledPerPortConfig.blinkRate = 2;

    for(i = 24; i < 56; i++)
    {
        rc = cpssDxChLedPhyControlPerPortSet(0, i, &ledPerPortConfig);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChLedPhyControlPerPortSet", rc);
        if(rc != GT_OK)
        {
            return rc;
        }

    }

    ledInterfaceNum = 0;

    ledConfig.ledOrganize             = CPSS_LED_ORDER_MODE_BY_CLASS_E;
    ledConfig.disableOnLinkDown       = GT_TRUE;
    ledConfig.blink0DutyCycle         = CPSS_LED_BLINK_DUTY_CYCLE_2_E;
    ledConfig.blink0Duration          = CPSS_LED_BLINK_DURATION_2_E;
    ledConfig.blink1DutyCycle         = CPSS_LED_BLINK_DUTY_CYCLE_0_E;
    ledConfig.blink1Duration          = CPSS_LED_BLINK_DURATION_0_E;
    ledConfig.pulseStretch            = CPSS_LED_PULSE_STRETCH_1_E;
    ledConfig.ledStart                = 64;
    ledConfig.ledEnd                  = 67;
    ledConfig.clkInvert               = GT_FALSE;
    ledConfig.class5select            = CPSS_LED_CLASS_5_SELECT_HALF_DUPLEX_E;
    ledConfig.class13select           = CPSS_LED_CLASS_13_SELECT_LINK_DOWN_E;
    ledConfig.invertEnable            = GT_TRUE;
    ledConfig.ledClockFrequency       = CPSS_LED_CLOCK_OUT_FREQUENCY_1000_E;


    rc = cpssDxChLedStreamConfigSet(devNum, ledInterfaceNum, &ledConfig);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChLedStreamConfigSet", rc);
    if(rc != GT_OK)
    {
        return rc;
    }

    classNum = 2;

    ledClassManip.invertEnable         = GT_TRUE;
    ledClassManip.blinkEnable          = GT_TRUE;
    ledClassManip.blinkSelect          = CPSS_LED_BLINK_SELECT_0_E;
    ledClassManip.forceEnable          = GT_FALSE;
    ledClassManip.forceData            = 0;
    ledClassManip.pulseStretchEnable   = GT_TRUE;
    ledClassManip.disableOnLinkDown    = GT_TRUE;

    rc = cpssDxChLedStreamClassManipulationSet(devNum
                                                    ,ledInterfaceNum
                                                    ,CPSS_DXCH_LED_PORT_TYPE_XG_E
                                                    ,classNum
                                                    ,&ledClassManip);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChLedStreamClassManipulationSet", rc);

    return rc;
}

#define PIPE_DEV_NUM 1
#define PIPE_DEV_IDX 1

/**
* @internal getBoardInfo function
* @endinternal
*
* @brief   Return the board configuration info.
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] isB2bSystem              - GT_TRUE, the system is a B2B system.
* @param[out] numOfPp                  - Number of Packet processors in system.
* @param[out] numOfFa                  - Number of Fabric Adaptors in system.
* @param[out] numOfXbar                - Number of Crossbar devices in system.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS getBoardInfo
(
    IN  GT_U8   boardRevId,
    OUT GT_U8   *numOfPp,
    OUT GT_U8   *numOfFa,
    OUT GT_U8   *numOfXbar,
    OUT GT_BOOL *isB2bSystem
)
{
    GT_STATUS rc;

    CPSS_PP_DEVICE_TYPE   deviceId;
    GT_PHYSICAL_PORT_NUM  currentRemotePortNum, firstRemotePortNum;
    GT_U32  ii,jj;

    rc = genericBoardConfigFun.boardGetInfo(genericBoardRevision,numOfPp,numOfFa,
                                            numOfXbar,isB2bSystem);
    if(rc != GT_OK)
    {
        return rc;
    }

    switch(appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(0)].devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_AC3X_E:
            userForceBoardType(APP_DEMO_XCAT3X_A0_BOARD_DB_CNS);
            break;
        case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
            userForceBoardType(APP_DEMO_XCAT3X_Z0_BOARD_DB_CNS);
            break;
        default:
            break;
    }

    /* Determine if device is 24 or 48 1G ports */
    rc = appDemoDebugDeviceIdGet(appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(0)].devNum,
                                    &deviceId);
    if (rc == GT_NO_SUCH)
    {
        deviceId = (appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(0)].pciInfo.pciDevVendorId.devId << 16) |
                    appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(0)].pciInfo.pciDevVendorId.vendorId;
        rc = GT_OK;
    }
    if(rc != GT_OK)
    {
        return rc;
    }
    switch(deviceId)
    {
    case CPSS_98DX3248_CNS:
        /* 48x1G + 20x10G */
        appDemoPortMapSize = SIZE_OF_MAC(xcat3plus_Map_48_remote_20x10G_ports);
        appDemoPortMapPtr  = &xcat3plus_Map_48_remote_20x10G_ports[0];

        /* use all 6 cascade ports */
        mask_1690_Ports = 0x3f;
        break;

    case CPSS_98DX3256_CNS:
        /* 24x1G + 4x10G + 2x40G */
        appDemoPortMapSize = SIZE_OF_MAC(xcat3x_Map_24_remote_ports);
        appDemoPortMapPtr  = &xcat3x_Map_24_remote_ports[0];
        /* use only 3 cascade ports */
        mask_1690_Ports = 0x07;
        break;

    case CPSS_98DX3257_CNS:
    default:
        /* 48x1G + 4x10G + 2x40G */
        appDemoPortMapSize = SIZE_OF_MAC(xcat3x_Map_48_remote_ports);
        appDemoPortMapPtr  = &xcat3x_Map_48_remote_ports[0];

        /* use all 6 cascade ports */
        mask_1690_Ports = 0x3f;
        break;
    }

    appDemo_boardRevId2PortsInitListPtr = &prv_xcat3x_boardRevId2PortsInitList[0];
    appDemo_boardRevId2PortsSize = SIZE_OF_MAC(prv_xcat3x_boardRevId2PortsInitList);

    if(boardRevId != boardRevId_4_PHY_1690_2_PHY_2180_PIPE)
    {
        /* Set mac ports global pointer */
        macPortsArrGlobalPtr = macPorts;
        macPortsSize = SIZE_OF_MAC(macPorts);
        physicalToInfoArrGlobalPtr = physicalToInfoArr;
    }
    else
    {
        /* Set mac ports global pointer */
        macPortsArrGlobalPtr = macPorts_rev_5;
        macPortsSize = SIZE_OF_MAC(macPorts_rev_5);
        physicalToInfoArrGlobalPtr = physicalToInfoArr_rev_5;
    }

#ifdef PX_FAMILY
    cpssOsMemSet(&pipeConfig, 0, sizeof(pipeConfig));
#endif

    if(boardRevId == boardRevId_NO_PHY_1690)
    {
        appDemo_boardRevId2PortsInitListPtr = &prv_xcat3x_rev_2_PortsInitList[0];
        appDemo_boardRevId2PortsSize = SIZE_OF_MAC(prv_xcat3x_rev_2_PortsInitList);

        appDemoPortMapSize = SIZE_OF_MAC(xcat3x_Map_0_remote_ports);
        appDemoPortMapPtr  = &xcat3x_Map_0_remote_ports[0];
        /* use no cascade ports */
        mask_1690_Ports = 0x00;

        /* no shaper needed since no remote ports */
        xcat3x_debug_GlobalShaperSet(GT_FALSE);
    }
    else
    if(boardRevId == boardRevId_4_PHY_1690)
    {
        appDemo_boardRevId2PortsInitListPtr = &prv_xcat3x_rev_3_PortsInitList[0];
        appDemo_boardRevId2PortsSize = SIZE_OF_MAC(prv_xcat3x_rev_2_PortsInitList);

        appDemoPortMapSize = SIZE_OF_MAC(xcat3x_Map_32_remote_ports);
        appDemoPortMapPtr  = &xcat3x_Map_32_remote_ports[0];
        /* use 4 cascade ports */
        mask_1690_Ports = 0x1b;/*b011011*/
    }
#ifdef PX_FAMILY
    else if(boardRevId == boardRevId_4_PHY_1690_2_PHY_2180_PIPE)
    {
        appDemo_boardRevId2PortsInitListPtr = &prv_xcat3x_rev_5_PortsInitList[0];
        appDemo_boardRevId2PortsSize = SIZE_OF_MAC(prv_xcat3x_rev_5_PortsInitList);

        appDemoPortMapSize = SIZE_OF_MAC(xcat3x_Map_4phy1690_2phy2180_pipe);
        appDemoPortMapPtr  = &xcat3x_Map_4phy1690_2phy2180_pipe[0];
        /* use 4 cascade ports */
        mask_1690_Ports = 0x1b;/*b011011*/
        pipeConfig.valid = GT_TRUE;
    }
#endif
    else
    if(deviceId == CPSS_98DX3248_CNS)
    {
        appDemo_boardRevId2PortsInitListPtr = &prv_xcat3plus_boardRevId2PortsInitList[0];
        appDemo_boardRevId2PortsSize = SIZE_OF_MAC(prv_xcat3plus_boardRevId2PortsInitList);
    }

    firstRemotePortNum = CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS;

    for(ii = 0 ; ii < appDemoPortMapSize ; ii++)
    {
        if(appDemoPortMapPtr[ii].mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E)
        {
            firstRemotePortNum = appDemoPortMapPtr[ii].physicalPortNumber;
            break;
        }
    }

    if(firstRemotePortNum != CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS)
    {
        currentRemotePortNum = firstRemotePortNum;

        /* Assumption : all remote ports are consecutive */

        for(ii = 0 ; ports_to_88e1690_arr[ii] != NOT_VALID_CNS ; ii++)
        {
            if(0 == ((1<<ii) & mask_1690_Ports))
            {
                /* this cascade port is not part of this xCat3x */
                continue;
            }

            for(jj = 0 ; jj < 8; jj++)
            {
                macPortsArrGlobalPtr[ii][jj].remotePhysicalPortNum = currentRemotePortNum;

                currentRemotePortNum++;
            }
        }
    }

    appDemoPolarityArrayPtr = &xcat3xA0PolarityArray[0];/*dummy*/
    appDemoPolarityArraySize = 0;/* force to skip 'polarity' */

    return GT_OK;
}


/**
* @internal getPpPhase1Config function
* @endinternal
*
* @brief   Returns the configuration parameters for corePpHwPhase1Init().
*
* @param[in] boardRevId               - The board revision Id.
* @param[in] devIdx                   - The Pp Index to get the parameters for.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS getPpPhase1Config
(
    IN  GT_U8                       boardRevId,
    IN  GT_U8                       devIdx,
    OUT CPSS_PP_PHASE1_INIT_PARAMS    *phase1Params
)
{
    GT_STATUS rc;

    boardRevId = boardRevId;

    rc = genericBoardConfigFun.boardGetPpPh1Params(genericBoardRevision,devIdx,phase1Params);
    if(rc != GT_OK)
    {
        return rc;
    }


    /* no extra/override info needed */

    return GT_OK;
}

static GT_STATUS xcat3x_treatOnRemotePorts_txqShaper(
    IN GT_U8     devNum,
    IN GT_PHYSICAL_PORT_NUM  portNum,
    IN GT_BOOL  linkUp,
    IN GT_BOOL  force_1G_OnLinkDown
);

/**
* @internal xcat3x_RemotePortsMacCountersByPacketParse function
* @endinternal
*
* @brief   Parses packet containing Ethernet MAC counters.
*         The result is stored at shadow buffers, and it can be retrieved
*         by function cpssDxChPortMacCountersCaptureOnPortGet()
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] queueIdx                 - The queue from which this packet was received.
* @param[in] numOfBuff                - Num of used buffs in packetBuffsArrPtr.
* @param[in] packetBuffsArrPtr[]      - (pointer to)The received packet buffers list.
* @param[in] buffLenArr[]             - List of buffer lengths for packetBuffsArrPtr.
* @param[in] rxParamsPtr              - (pointer to)information parameters of received packets
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The input parameters are obtained by function cpssDxChNetIfSdmaRxPacketGet()
*       The output parameter are the parsed port number and MAC counter values.
*
*/
static GT_STATUS xcat3x_RemotePortsMacCountersByPacketParse
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           queueIdx,
    IN  GT_U32                          numOfBuff,
    IN  GT_U8                          *packetBuffsArrPtr[],
    IN  GT_U32                          buffLenArr[],
    IN  void                           *rxParamsPtr
)
{
    return cpssDxChPortRemotePortsMacCountersByPacketParse(
            devNum,
            numOfBuff,
            packetBuffsArrPtr,
            buffLenArr,
            (CPSS_DXCH_NET_RX_PARAMS_STC *) rxParamsPtr);
}

/**
* @internal configBoardAfterPhase1 function
* @endinternal
*
* @brief   This function performs all needed configurations that should be done
*         after phase1.
* @param[in] boardRevId               - The board revision Id.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS configBoardAfterPhase1
(
    IN  GT_U8   boardRevId
)
{
    GT_STATUS rc;
    GT_U8       devNum = SYSTEM_DEV_NUM_MAC(0);
    GT_U32      ii,jj;
    GT_U32      portNum;
    GT_U8       tcQueue;
    GT_U32      bcValue = 16;/* The device use L2 byte count by default.
                                But we need L1 rate.
                                Also, need to subtract the DSA tag length.
                                So need to add 16 bytes for shaper*/

    GT_BOOL     isLinkUp = GT_FALSE;
    GT_U32      useMibCounterReadByPacket = 0;
    CPSS_DXCH_CFG_REMOTE_PHY_MAC_INFO_STC   physicalToInfo;

    rc = genericBoardConfigFun.boardAfterPhase1Config(genericBoardRevision);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Get method for accessing MIB counters from appDemo DB */
#ifndef ASIC_SIMULATION
    appDemoDbEntryGet("useMibCounterReadByPacket", &useMibCounterReadByPacket);
#endif  /* ASIC_SIMULATION */

    /* Get method for getting link state from board revision Id / appDemo DB */
    usePollingOn88e1690LinkChange = (boardRevId == boardRevId_polling_default);
    appDemoDbEntryGet("usePollingOn88e1690LinkChange", (GT_U32 *)&usePollingOn88e1690LinkChange);

    /* init the DB */
    if (
        (appDemoPpConfigList[devNum].deviceId == CPSS_98DX8316_CNS ||
         appDemoPpConfigList[devNum].deviceId == CPSS_98DXH831_CNS))
    {
        /* 16x1G device */
        ac3x_board_numCscdPorts = 2;
    }
    else
    if (
        (appDemoPpConfigList[devNum].deviceId == CPSS_98DX3256_CNS))
    {
        /* 24x1G device */
        ac3x_board_numCscdPorts = 3;
    }
    else
    {
        if (boardRevId != boardRevId_4_PHY_1690_2_PHY_2180_PIPE)
        {
            /* 48x1G device */
            ac3x_board_numCscdPorts = 6;
        }
        else
        {
            /* 48x1G device */
            ac3x_board_numCscdPorts = 4;
        }
    }
    for(ii = 0 ; ii < SIZE_OF_MAC(polling__info_macPorts) ; ii++)
    {
        if(ii < ac3x_board_numCscdPorts)
        {
            polling__info_macPorts[ii].isValid = GT_TRUE;
            cpssOsMemSet(polling__info_macPorts[ii].portsInfo,0,sizeof(polling__info_macPorts[ii].portsInfo));

        }
        else
        {
            polling__info_macPorts[ii].isValid = GT_FALSE;
        }
    }

    for(ii = 0 ; ports_to_88e1690_arr[ii] != NOT_VALID_CNS ; ii++)
    {
        if(0 == ((1<<ii) & mask_1690_Ports))
        {
            /* this cascade port is not part of this xCat3x */
            continue;
        }

        if(usePollingOn88e1690LinkChange == GT_FALSE)
        {
            physicalToInfo = physicalToInfoArrGlobalPtr[ii];
        }
        else
        {
            if(polling__info_macPorts[ii].isValid == GT_FALSE)
            {
                /* skipped */
                continue;
            }

            physicalToInfo = polling__physicalToInfoArr[ii];
        }

        if (appDemoPortMapPtr == xcat3plus_Map_48_remote_20x10G_ports)
        {
            physicalToInfo.internalFcTxqPort = CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS;
        }
        physicalToInfo.useMibCounterReadByPacket = (useMibCounterReadByPacket != 0);
        rc = cpssDxChCfgPortRemotePhyMacBind(devNum,ports_to_88e1690_arr[ii],&physicalToInfo);

        if(rc != GT_OK)
        {
            return rc;
        }

        for(jj = 0 ; jj < 8; jj++)
        {
            portNum = macPortsArrGlobalPtr[ii][jj].remotePhysicalPortNum;
            if (xcat3x_debug_GlobalEgfFilter == GT_TRUE)
            {
                /* the CPSS (inside cpssDxChCfgPortRemotePhyMacBind()) set all the
                   88e1690 ports in 'force link up' in the EGF filter */
                /* we need to set them as 'force link down' in the EGF filter ...
                    and wait for real indication of link change from the device

                    see function xcat3x_treatOnRemotePorts_egfLinkChange() that will
                    be called on link change event
                */
                rc = cpssDxChPortLinkStatusGet(devNum,portNum,&isLinkUp);
                if(rc != GT_OK)
                {
                    return rc;
                }

                rc = cpssDxChBrgEgrFltPortLinkEnableSet(devNum,portNum,
                    isLinkUp ?
                        CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E :
                        CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_DOWN_E);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }

            if(xcat3x_debug_GlobalShaper == GT_TRUE)
            {
                /* bind all the ports to the same scheduler profile */
                rc = cpssDxChPortTxBindPortToSchedulerProfileSet(devNum,portNum,
                    schedulerProfile);
                if(rc != GT_OK)
                {
                    return rc;
                }

                /* set all ports to add the 20 preamble bytes */
                rc = cpssDxChPortTxByteCountChangeValueSet(devNum,portNum,
                    bcValue);
                if(rc != GT_OK)
                {
                    return rc;
                }

                /* configure shaper */
                rc = cpssDxChPortTxShaperBaselineSet(devNum,baseline_value);
                if(rc != GT_OK)
                {
                    return rc;
                }
                rc = xcat3x_treatOnRemotePorts_txqShaper(devNum,portNum,isLinkUp, GT_TRUE);
                if(rc != GT_OK)
                {
                    return rc;
                }

                /* enable the shaper for the remote port */
                rc = cpssDxChPortTxShaperEnableSet(devNum,portNum,GT_TRUE);
                if(rc != GT_OK)
                {
                    return rc;
                }
                for (tcQueue = 0; tcQueue < 8; tcQueue++)
                {
                    rc = cpssDxChPortTxQShaperEnableSet(devNum,portNum,tcQueue,GT_TRUE);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }
                }
            }
        }/*jj*/
    }/*ii*/

    if(xcat3x_debug_GlobalShaper == GT_TRUE)
    {
        /* set the scheduler profile to add the 20 bytes */
        rc = cpssDxChPortProfileTxByteCountChangeEnableSet(devNum,
            schedulerProfile,
            CPSS_DXCH_PORT_TX_BC_CHANGE_ENABLE_SHAPER_ONLY_E,
            CPSS_ADJUST_OPERATION_ADD_E);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* Globally enable PFC response */
        rc = cpssDxChPortPfcEnableSet(devNum,
                                CPSS_DXCH_PORT_PFC_ENABLE_TRIGGERING_AND_RESPONSE_E);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    if (useMibCounterReadByPacket != 0)
    {
        rc = appDemoDxChNetRxPacketCbRegister(xcat3x_RemotePortsMacCountersByPacketParse);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal getPpPhase2Config function
* @endinternal
*
* @brief   Returns the configuration parameters for corePpHwPhase2Init().
*
* @param[in] boardRevId               - The board revision Id.
* @param[in] oldDevNum                - The old Pp device number to get the parameters for.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS getPpPhase2Config
(
    IN  GT_U8                       boardRevId,
    IN  GT_U8                       oldDevNum,
    OUT CPSS_PP_PHASE2_INIT_PARAMS    *phase2Params
)
{
    GT_STATUS rc;

    boardRevId = boardRevId;

    rc = genericBoardConfigFun.boardGetPpPh2Params(genericBoardRevision,oldDevNum,phase2Params);
    if(rc != GT_OK)
    {
        return rc;
    }


    /* no extra/override info needed */

    return GT_OK;
}

/**
* @internal xcat3x_treatOnRemotePorts_egfLinkChange function
* @endinternal
*
* @brief   function called for remote physical ports to set EGF link change filter on the port,
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] linkUp                   - indication that the port is currently 'up'/'down'
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note the function called from the event handler task that recognized :
*       link_change / auto-negotiation ended
*
*/
static GT_STATUS xcat3x_treatOnRemotePorts_egfLinkChange(
    IN GT_U8     devNum,
    IN GT_PHYSICAL_PORT_NUM  portNum,
    IN GT_BOOL  linkUp
)
{
    /* need to set the EGF link status filter according to new state of the port */
    return cpssDxChBrgEgrFltPortLinkEnableSet(devNum, portNum,
        (linkUp == GT_TRUE ?
         CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E :
         CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_DOWN_E));
}

static GT_U32   speedKbpsArr[2][3] = {
    {
        /* Values for 480 MHz core clock */
        speed_10mbps   + epsilon_10Mb,  /* value for speed 10   Mbps */
        speed_100mbps  + epsilon_100Mb, /* value for speed 100  Mbps */
        speed_1000mbps + epsilon_1Gb    /* value for speed 1    Gbps */
    },
    {
        /* Values for non-480 MHz core clock */
        speed_10mbps   + epsilon_365MHz_10Mb,  /* value for speed 10   Mbps */
        speed_100mbps  + epsilon_365MHz_100Mb, /* value for speed 100  Mbps */
        speed_1000mbps + epsilon_365MHz_1Gb    /* value for speed 1    Gbps */
    }
};
/**
* @internal xcat3x_treatOnRemotePorts_txqShaper function
* @endinternal
*
* @brief   function called for remote physical ports to set TXQ shaper on the port,
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] linkUp                   - indication that the port is currently 'up'/'down'
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note the function called from the event handler task that recognized :
*       link_change / auto-negotiation ended
*
*/
static GT_STATUS xcat3x_treatOnRemotePorts_txqShaper(
    IN GT_U8     devNum,
    IN GT_PHYSICAL_PORT_NUM  portNum,
    IN GT_BOOL  linkUp,
    IN GT_BOOL  force_1G_OnLinkDown
)
{
    GT_STATUS rc;
    GT_U16    burstSize;
    GT_U32    maxRate;
    CPSS_PORT_SPEED_ENT speed;
    GT_U32    coreClockIndex;   /* Index into table according to core clock */
    GT_U32  speed_kbps;
    GT_U8     tcQueue;

    if(force_1G_OnLinkDown && (linkUp == GT_FALSE))
    {
        speed = CPSS_PORT_SPEED_1000_E;
    }
    else
    {
        if(linkUp == GT_FALSE)
        {
            return GT_OK;
        }
        rc = cpssDxChPortSpeedGet(devNum,portNum,&speed);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    burstSize = burstSize_value;

    if (PRV_CPSS_PP_MAC(devNum)->coreClock < 480)
    {
        coreClockIndex = 1;
    }
    else
    {
        coreClockIndex = 0;
    }

    if(speed == CPSS_PORT_SPEED_10_E)
    {
        speed_kbps = speedKbpsArr[coreClockIndex][0];
    }
    else
    if(speed == CPSS_PORT_SPEED_100_E)
    {
        speed_kbps = speedKbpsArr[coreClockIndex][1];
    }
    else/*1000*/
    {
        speed_kbps = speedKbpsArr[coreClockIndex][2];
    }

    maxRate = speed_kbps;

    rc = cpssDxChPortTxShaperProfileSet(devNum,portNum,burstSize,&maxRate);
    if(rc != GT_OK)
    {
        return rc;
    }
    for (tcQueue = 0; tcQueue < 8; tcQueue++)
    {
        rc = cpssDxChPortTxQShaperProfileSet(devNum,portNum,tcQueue,burstSize,&maxRate);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /* Update Speed index, used for CFCC to Pause timer conversion */
    rc = cpssDxChPortCnSpeedIndexSet(devNum,portNum,speed);
    if(rc != GT_OK)
    {
        return rc;
    }

/*
    if(maxRate != speed_kbps)
    {
        cpssOsPrintf("shaper on port[%d] : ask for speed_kbps[%d] got maxRate[%d]\n",
            portNum,speed_kbps,maxRate);
    }
    else
    {
        cpssOsPrintf("shaper on port[%d] : ask for speed_kbps[%d] \n",
            portNum,speed_kbps);
    }
*/
    return rc;

}

void xcat3x_debug_testPortRates(
    IN GT_U8   devNum,
    IN GT_U32   portNum
)
{
    GT_STATUS rc;
    GT_U16    burstSize;
    GT_U32    maxRate;
    CPSS_PORT_SPEED_ENT speed;
    GT_U32  speed_kbps;
    GT_U32  neededSpeed;
    GT_U32  ii;

    burstSize = burstSize_value;

    for(speed = CPSS_PORT_SPEED_10_E ; speed <= CPSS_PORT_SPEED_1000_E ; speed++)
    {
        if(speed == CPSS_PORT_SPEED_10_E)
        {
            speed_kbps = speed_10mbps;
        }
        else
        if(speed == CPSS_PORT_SPEED_100_E)
        {
            speed_kbps = speed_100mbps;
        }
        else/*1000*/
        {
            speed_kbps = speed_1000mbps;
        }

        neededSpeed = speed_kbps;

        ii = 0;
        do
        {
            maxRate = speed_kbps;
            speed_kbps++;

            /*cpssOsPrintf("iteration [%d] \n",ii);*/
            ii++;

            rc = cpssDxChPortTxShaperProfileSet(devNum,portNum,burstSize,&maxRate);
            if(rc != GT_OK)
            {
                cpssOsPrintf("error on port [%d] on neededSpeed[%d Kbps] : for maxRate[%d Kbps] \n",
                    portNum,neededSpeed,speed_kbps);

                continue;
            }

        }while(maxRate < neededSpeed);

        cpssOsPrintf("speed[%d Kbps] : got maxRate[%d Kbps] for speed_kbps[%d Kbps] \n",
            neededSpeed,maxRate,speed_kbps);
    }
}


/**
* @internal xcat3x_treatOnRemotePorts function
* @endinternal
*
* @brief   function called for remote physical ports to set extra config on the port,
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] linkUp                   - indication that the port is currently 'up'/'down'
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note the function called from the event handler task that recognized :
*       link_change / auto-negotiation ended
*
*/
static GT_STATUS xcat3x_treatOnRemotePorts(
    IN GT_U8     devNum,
    IN GT_PHYSICAL_PORT_NUM  portNum,
    IN GT_BOOL  linkUp
)
{
    GT_STATUS rc;

    if(xcat3x_debug_GlobalEgfFilter == GT_TRUE)
    {
        rc = xcat3x_treatOnRemotePorts_egfLinkChange(devNum,portNum,linkUp);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    if(xcat3x_debug_GlobalShaper == GT_TRUE)
    {
        rc = xcat3x_treatOnRemotePorts_txqShaper(devNum,portNum,linkUp, GT_FALSE);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal configBoardAfterPhase2 function
* @endinternal
*
* @brief   This function performs all needed configurations that should be done
*         after phase2.
* @param[in] boardRevId               - The board revision Id.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS configBoardAfterPhase2
(
    IN  GT_U8   boardRevId
)
{
    GT_STATUS rc;

    boardRevId = boardRevId;

    rc = genericBoardConfigFun.boardAfterPhase2Config(genericBoardRevision);
    if(rc != GT_OK)
    {
        return rc;
    }

#ifdef PX_FAMILY
    if(GT_TRUE == pipeConfig.valid)
    {
        rc = extDrvPexConfigure(
            appDemoPpConfigList[PIPE_DEV_IDX].pciInfo.pciBusNum,
            appDemoPpConfigList[PIPE_DEV_IDX].pciInfo.pciIdSel,
            appDemoPpConfigList[PIPE_DEV_IDX].pciInfo.funcNo,
            MV_EXT_DRV_CFG_FLAG_NEW_ADDRCOMPL_E,
            &(pipeConfig.hwInfo[0])
        );

        rc = px_appDemoInitSequence(PIPE_DEV_IDX, PIPE_DEV_NUM, pipeConfig.hwInfo, portInitlist_pipe);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Disable packets from CPU to be forwarded to AC3X */
        rc = cpssPxIngressPortMapEntrySet(PIPE_DEV_NUM, CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E, 16, 0x1FFF0, GT_FALSE);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Forwarding FROM external ports TO AC3X */
        rc = cpssPxIngressPortRedirectSet(PIPE_DEV_NUM, 13, BIT_4);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressPortRedirectSet", rc);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = cpssPxIngressPortRedirectSet(PIPE_DEV_NUM, 15, BIT_6);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressPortRedirectSet", rc);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Forwarding FROM AC3X TO external ports */
        rc = cpssPxIngressPortRedirectSet(PIPE_DEV_NUM, 4, BIT_13);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressPortRedirectSet", rc);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = cpssPxIngressPortRedirectSet(PIPE_DEV_NUM, 6, BIT_15);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressPortRedirectSet", rc);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = cpssPxPortSerdesLanePolaritySet(PIPE_DEV_NUM, 0, 13, GT_TRUE, GT_TRUE);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxPortSerdesLanePolaritySet", rc);
        if (rc != GT_OK)
        {
            return rc;
        }

    }
#endif

    appDemoCallBackOnRemotePortsFunc = xcat3x_treatOnRemotePorts;
    /* no extra/override info needed */
    return GT_OK;
}


/**
* @internal getPpLogicalInitParams function
* @endinternal
*
* @brief   Returns the parameters needed for sysPpLogicalInit() function.
*
* @param[in] boardRevId               - The board revision Id.
* @param[in] devNum                   - The Pp device number to get the parameters for.
*
* @param[out] ppLogInitParams          - Pp logical init parameters struct.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS getPpLogicalInitParams
(
    IN  GT_U8           boardRevId,
    IN  GT_U8           devNum,
    OUT CPSS_PP_CONFIG_INIT_STC    *ppLogInitParams
)
{
    GT_STATUS rc;

    boardRevId = boardRevId;

    rc = genericBoardConfigFun.boardGetPpLogInitParams(genericBoardRevision,devNum,ppLogInitParams);
    if(rc != GT_OK)
    {
        return rc;
    }
    /* AC3X - 1664 LPM lines */
    ppLogInitParams->maxNumOfPbrEntries = 1664*4;


    /* no extra/override info needed */

    return GT_OK;
}


/**
* @internal getTapiLibInitParams function
* @endinternal
*
* @brief   Returns Tapi library initialization parameters.
*
* @param[in] boardRevId               - The board revision Id.
* @param[in] devNum                   - The Pp device number to get the parameters for.
*
* @param[out] libInitParams            - Library initialization parameters.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS getTapiLibInitParams
(
    IN  GT_U8                       boardRevId,
    IN  GT_U8                       devNum,
    OUT APP_DEMO_LIB_INIT_PARAMS    *libInitParams
)
{
    GT_STATUS rc;

    boardRevId = boardRevId;

    rc = genericBoardConfigFun.boardGetLibInitParams(genericBoardRevision,devNum,libInitParams);
    if(rc != GT_OK)
    {
        return rc;
    }


    /* no extra/override info needed */

    return GT_OK;
}

extern GT_STATUS appDemoEniEvTreat
(
    GT_U8                   devNum,
    GT_U32                  uniEv,
    GT_U32                  evExtData
);

/* the taskId of the polling task */
static GT_TASK checkLinkStatusOn88e1690Ports_tid = 0;
/* value 1 means that need to kill the polling task */
static GT_U32  checkLinkStatusOn88e1690Ports_taskNeedToDie = 0;
/* value 1 means that the polling task is running */
static GT_U32  checkLinkStatusOn88e1690Ports_taskRunning = 0;

static GT_U32  sleepTimeBetweenPolling = 100;
/*debug function for fine tuning ... if needed */
void debug_sleepTimeBetweenPollingSet(IN GT_U32 newTime)
{
    if(sleepTimeBetweenPolling == 0)
    {
        sleepTimeBetweenPolling = 100;
    }
    else
    {
        sleepTimeBetweenPolling = newTime;
    }
}

/**
* @internal checkLinkStatusOn88e1690Ports function
* @endinternal
*
* @brief   called from the task of polling the ports of the 88e1690 MAC-PHY devices
*         on the DX device.
* @param[in] devNum                   - the DX device number.
*
* @retval GT_OK                    - loop on all ports finished
* @retval GT_ABORTED               - when task need to end
*/
static GT_STATUS checkLinkStatusOn88e1690Ports(IN GT_U8    devNum)
{
    GT_STATUS   rc;
    GT_U32  ii,jj;
    GT_BOOL  currentState;
    GT_U32  portNum;

    for(ii = 0 ; ii < macPortsSize; ii++)
    {
        if(checkLinkStatusOn88e1690Ports_taskNeedToDie)
        {
            /* we got indication to kill this task */
            return /* not error for the log */ GT_ABORTED;
        }

        osTimerWkAfter(sleepTimeBetweenPolling);

        if(polling__info_macPorts[ii].isValid == GT_FALSE)
        {
            continue;
        }

        for(jj = 0 ; jj < 8 ; jj++)
        {
            if(checkLinkStatusOn88e1690Ports_taskNeedToDie)
            {
                /* we got indication to kill this task */
                return /* not error for the log */ GT_ABORTED;
            }

            portNum = macPortsArrGlobalPtr[ii][jj].remotePhysicalPortNum;

            rc = cpssDxChPortLinkStatusGet(devNum,portNum,&currentState);
            if(rc == GT_OK)
            {
                if(currentState != polling__info_macPorts[ii].portsInfo[jj].linkUp)
                {
                    /* update the DB */
                    polling__info_macPorts[ii].portsInfo[jj].linkUp = currentState;

                    /* notify the event handler (allow it to count the 'event') */
                    (void)appDemoEniEvTreat(devNum,
                        CPSS_PP_PORT_LINK_STATUS_CHANGED_E,
                        portNum);
                }
            }
        }
    }

    return GT_OK;
}

/*******************************************************************************
* intScanPolling88e1690Task
*
* DESCRIPTION:
*       task for polling the ports of the 88e1690 MAC-PHY devices on the DX device.
*
* INPUTS:
*       devNumPtr - pointer to devNum.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       1 - task ended
*
* COMMENTS:
*
*******************************************************************************/
static unsigned __TASKCONV intScanPolling88e1690Task
(
    IN GT_VOID * devNumPtr
)
{
    GT_U8   devNum = *(GT_U8*)devNumPtr;

    checkLinkStatusOn88e1690Ports_taskRunning = 1;

    /*osTimerWkAfter(5000);*/

    /* polling on the ISR to check if there are interrupts waiting */
    while(1)
    {
        /* invoke ISR until all interrupts have been treated */
        (void)checkLinkStatusOn88e1690Ports(devNum);

        if(checkLinkStatusOn88e1690Ports_taskNeedToDie)
        {
            /* we got indication to kill this task */
            break;
        }
    }

    /* reset all relevant flags */
    checkLinkStatusOn88e1690Ports_taskRunning = 0;
    checkLinkStatusOn88e1690Ports_taskNeedToDie = 0;
    checkLinkStatusOn88e1690Ports_tid = 0;

    return 1;
}

extern void prvTgfForceEDsaRemotePortsFixedSet(IN GT_U32 eDSAFixed);

static APPDEMO_SERDES_LANE_POLARITY_STC  ac3_PIPE_RD_PolarityArray[] =
{
/* laneNum  invertTx    invertRx */
    { 0, GT_FALSE, GT_TRUE },
    { 1, GT_FALSE, GT_FALSE},
    { 2, GT_TRUE , GT_TRUE },
    { 3, GT_FALSE, GT_FALSE},
    { 4, GT_FALSE, GT_TRUE },
    { 5, GT_FALSE, GT_TRUE },
    { 6, GT_FALSE, GT_TRUE },
    { 7, GT_FALSE, GT_TRUE },
    { 8, GT_TRUE , GT_TRUE },
    { 9, GT_TRUE , GT_TRUE },
    {10, GT_FALSE, GT_TRUE },
    {11, GT_TRUE , GT_TRUE },
    {12, GT_TRUE , GT_TRUE },
    {13, GT_TRUE , GT_TRUE },
    {14, GT_FALSE, GT_TRUE },
    {15, GT_TRUE , GT_FALSE},
    {16, GT_FALSE, GT_TRUE },
    {17, GT_TRUE , GT_TRUE },
    {18, GT_TRUE , GT_TRUE },
    {19, GT_FALSE, GT_TRUE },
    {20, GT_FALSE, GT_TRUE },
    {21, GT_TRUE , GT_TRUE },
    {22, GT_TRUE , GT_TRUE },
    {24, GT_TRUE , GT_TRUE },
    {25, GT_TRUE , GT_FALSE},
    {26, GT_FALSE, GT_TRUE },
    {27, GT_TRUE , GT_FALSE},
    {29, GT_TRUE , GT_TRUE },
    {31, GT_FALSE, GT_TRUE }
};

/**
* @internal prvAppDemoSerdesPolarityConfigSet function
* @endinternal
*
* @brief   Per DB/RD board configure the Tx/Rx Polarity values on Serdeses
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number
* @param[in] boardRevId               - board revision ID
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvAppDemoSerdesPolarityConfigSet
(
    IN GT_U8    devNum,
    IN GT_U8    boardRevId
)
{
    GT_STATUS                           rc;
    GT_U32                              i;
    APPDEMO_SERDES_LANE_POLARITY_STC    *currentPolarityArrayPtr;
    GT_U32                              polarityArraySize;

    if (boardRevId != boardRevId_4_PHY_1690_2_PHY_2180_PIPE)
    {
        return GT_OK;
    }

    /* Set sereds lane polarity */
    currentPolarityArrayPtr = ac3_PIPE_RD_PolarityArray;
    polarityArraySize = sizeof(ac3_PIPE_RD_PolarityArray)/sizeof(APPDEMO_SERDES_LANE_POLARITY_STC);

    for (i = 0; i < polarityArraySize; i++)
    {
        rc = cpssDxChPortSerdesLanePolaritySet(devNum, 0,
                                               currentPolarityArrayPtr[i].laneNum,
                                               currentPolarityArrayPtr[i].invertTx,
                                               currentPolarityArrayPtr[i].invertRx);

        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortSerdesLanePolaritySet", rc);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

#ifndef ASIC_SIMULATION
extern GT_STATUS appAc3xRdEnableSpeed
(
    IN GT_U32 devNum,
    IN GT_U16 portNum,
    IN GT_U16 speed_bits
);
#endif

/**
* @internal afterInitBoardConfig function
* @endinternal
*
* @brief   PSS Board specific configurations that should be done after board
*         initialization.
* @param[in] boardRevId               - The board revision Id.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS afterInitBoardConfig
(
    IN  GT_U8                       boardRevId
)
{
    GT_STATUS   rc;
    GT_U8       devNum = SYSTEM_DEV_NUM_MAC(0);
#ifndef ASIC_SIMULATION
    GT_U16      portNum;
#endif

    boardRevId = boardRevId;


    if (appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(0)].devFamily ==CPSS_PP_FAMILY_DXCH_AC3X_E )
    {
        cpssDxChPortSerdesRxauiManualTxConfigSet(devNum, sizeof(ac3xTxRxuaiCfg) / sizeof(CPSS_DXCH_PORT_SERDES_AC3X1690TX_PARAM_CONFIG_STC), ac3xTxRxuaiCfg);
    }

    /* let the generic board do it's things */
    rc = genericBoardConfigFun.boardAfterInitConfig(genericBoardRevision);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(usePollingOn88e1690LinkChange == GT_TRUE)
    {
        /* reset all relevant flags */
        checkLinkStatusOn88e1690Ports_taskRunning = 0;
        checkLinkStatusOn88e1690Ports_taskNeedToDie = 0;
        checkLinkStatusOn88e1690Ports_tid = 0;

        /* create the polling task */
        rc = osTaskCreate("Polling88e1690",
            200/*EV_HANDLER_MAX_PRIO*/ ,
            _32KB,
            intScanPolling88e1690Task,(GT_VOID*)(&devNum),
            &checkLinkStatusOn88e1690Ports_tid);
        if(rc != GT_OK)
        {
            return rc;
        }

        while(checkLinkStatusOn88e1690Ports_taskRunning == 0)
        {
            /* wait for the task to start running */
            osTimerWkAfter(1);
        }
    }

    rc = prvAppDemoLedInit(devNum, boardRevId);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("prvAppDemoLedInit", rc);
    if(GT_OK != rc)
    {
        return rc;
    }

    rc = prvAppDemoSerdesPolarityConfigSet(devNum, boardRevId);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("prvAppDemoSerdesPolarityConfigSet", rc);
    if(GT_OK != rc)
    {
        return rc;
    }

    if (boardRevId == boardRevId_4_PHY_1690_2_PHY_2180_PIPE)
    {
        rc = prvAppDemoPortXgPscLanesSwapSet(devNum);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("prvAppDemoPortXgPscLanesSwapSet", rc);
        if (GT_OK != rc)
        {
            return rc;
        }

#ifndef ASIC_SIMULATION
        /* Set PHY 88E2180 port to 2.5G */
        for ( portNum = 8; portNum < 24 ; portNum++ )
        {
           rc = appAc3xRdEnableSpeed(devNum, portNum, 0x800);
           CPSS_ENABLER_DBG_TRACE_RC_MAC("appAc3xRdEnableSpeed", rc);
           if (GT_OK != rc)
           {
               return rc;
           }
        }
#endif
    }

#ifdef INCLUDE_UTF
    prvTgfXcat3xExistsSet(GT_TRUE);
    /* we not want 0x8100 inside the DSA tag (or any other TPID) because the
       CPSS always 'replace' with 0x8100 from traffic that comes from the
       PP to the CPU */
    tgfTrafficGeneratorEtherTypeIgnoreSet(GT_TRUE);
#ifdef  ASIC_SIMULATION
    /*prvTgfForceEDsaRemotePortsFixedSet(1);*/
#else
    if (boardRevId == boardRevId_4_PHY_1690_2_PHY_2180_PIPE)
    {
        rc = cpssDrvPpHwInternalPciRegWrite(devNum, 0, 0x18000, 0x00002222);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDrvPpHwInternalPciRegWrite", rc);

        rc = cpssDrvPpHwInternalPciRegWrite(devNum, 0, 0x18004, 0x11000000);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDrvPpHwInternalPciRegWrite", rc);
    }
#endif /*ASIC_SIMULATION*/

#endif /*INCLUDE_UTF*/
    return GT_OK;
}


/**
* @internal gtDbDxAc3xBoardReg_BoardCleanDbDuringSystemReset function
* @endinternal
*
* @brief   clear the DB of the specific board config file , as part of the 'system rest'
*         to allow the 'cpssInitSystem' to run again as if it is the first time it runs
* @param[in] boardRevId               - The board revision Id.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS gtDbDxAc3xBoardReg_BoardCleanDbDuringSystemReset
(
    IN  GT_U8   boardRevId
)
{
    GT_STATUS   rc;

    boardRevId = boardRevId;

    if(checkLinkStatusOn88e1690Ports_tid)
    {
        /* indicate the polling task to exit the task */
        checkLinkStatusOn88e1690Ports_taskNeedToDie = 1;

        /* wait for task to state that not running any more */
        while(checkLinkStatusOn88e1690Ports_taskRunning)
        {
            osTimerWkAfter(10);
        }

        osTimerWkAfter(1);
    }

    rc = genericBoardConfigFun.boardCleanDbDuringSystemReset(genericBoardRevision);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;

}

/**
* @internal gtDbDxAc3xBoardReg function
* @endinternal
*
* @brief   Registration function for the AC3x (xCat3) board .
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbDxAc3xBoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
)
{
    GT_STATUS   rc;

    boardRevId = boardRevId;

    if(boardCfgFuncs == NULL)
    {
        return GT_FAIL;
    }

    rc = gtDbDxBobcat2BoardReg(genericBoardRevision,&genericBoardConfigFun);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* set the actual pointer to use for this board */
    boardCfgFuncs->boardGetInfo                 = getBoardInfo;
    boardCfgFuncs->boardGetPpPh1Params          = getPpPhase1Config;
    boardCfgFuncs->boardAfterPhase1Config       = configBoardAfterPhase1;
    boardCfgFuncs->boardGetPpPh2Params          = getPpPhase2Config;
    boardCfgFuncs->boardAfterPhase2Config       = configBoardAfterPhase2;
    boardCfgFuncs->boardGetPpLogInitParams      = getPpLogicalInitParams;
    boardCfgFuncs->boardGetLibInitParams        = getTapiLibInitParams;
    boardCfgFuncs->boardAfterInitConfig         = afterInitBoardConfig;
    boardCfgFuncs->boardCleanDbDuringSystemReset = gtDbDxAc3xBoardReg_BoardCleanDbDuringSystemReset;

    return GT_OK;
}

GT_STATUS prvCpssMacPhyPortRegistersPrint
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      phyMacDeviceNum,
    IN  GT_U32                      phyPort
);

GT_STATUS  ac3xPhyPortDump(
    IN  GT_U8                       devNum,
    IN  GT_U32                      phyMacDeviceNum,
    IN  GT_U32                      phyPort
)
{
return prvCpssMacPhyPortRegistersPrint(devNum, phyMacDeviceNum, phyPort);
}

/***********************************************************/
/***************** trunk phy1690_WA_B **********************/
/***********************************************************/
extern GT_STATUS cpssDxChTrunkInit_phy1690_WA_B
(
    IN GT_U8                    devNum,
    IN GT_U32                   firstGlobalEportForTrunkId1,
    IN GT_U32                   numEports
);

/* indication for Galtis/TGF wrappers call the WA 'B' APIs */
GT_U32   appDemo_trunk_phy1690_WA_B = 0;
/*
    init the trunk WA - B
*/
GT_STATUS appDemo_init_phy1690_WA_B
(
    IN GT_U8                    devNum
)
{
    GT_STATUS rc;
    CPSS_DXCH_CFG_GLOBAL_EPORT_STC globalInfo;
    CPSS_DXCH_CFG_GLOBAL_EPORT_STC l2EcmpInfo;
    GT_U32  firstGlobalEport = 512;
    GT_U32  maxTrunks = 64;

    rc = cpssDxChTrunkInit_phy1690_WA_B(devNum,
        firstGlobalEport+1/*firstGlobalEportForTrunkId1*/,
        maxTrunks);/*numEports*/
    if(rc != GT_OK)
    {
        return rc;
    }

    l2EcmpInfo.enable = GT_FALSE;

    /* set range for EPORTs that are in range 512..max */
    globalInfo.enable  = GT_TRUE;
    globalInfo.pattern = firstGlobalEport;/*512*/
    globalInfo.mask    = firstGlobalEport;/*512*/

    rc = cpssDxChCfgGlobalEportSet(devNum,&globalInfo,&l2EcmpInfo);
    if(rc != GT_OK)
    {
        return rc;
    }

    appDemo_trunk_phy1690_WA_B = 1;

    return GT_OK;
}

/***********************************************************/
/***************** trunk phy1690_WA_C **********************/
/***********************************************************/
extern GT_STATUS cpssDxChTrunkInit_phy1690_WA_C
(
    IN GT_U8                    devNum
);

/* indication for Galtis/TGF wrappers call the WA 'C' APIs */
GT_U32   appDemo_trunk_phy1690_WA_C = 0;
/*
    init the trunk WA - C
*/
GT_STATUS appDemo_init_phy1690_WA_C
(
    IN GT_U8                    devNum
)
{
    GT_STATUS rc;

    rc = cpssDxChTrunkInit_phy1690_WA_C(devNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    appDemo_trunk_phy1690_WA_C = 1;

    return GT_OK;
}



