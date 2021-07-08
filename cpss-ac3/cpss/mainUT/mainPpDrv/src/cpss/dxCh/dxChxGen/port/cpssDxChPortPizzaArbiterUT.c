/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* cpssDxChPortTxUT.c
*
* DESCRIPTION:
*       Unit tests for cpssDxChPortTx, that provides
*       CPSS implementation for configuring the Physical Port Tx Traffic Class.
*
* FILE REVISION NUMBER:
*       $Revision: 69 $
*******************************************************************************/
/* includes */
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
already fixed the types of ports from GT_U8 !

NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/cpssDxChPortPizzaArbiter.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/cpssDxChPortPizzaArbiterProfile.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/TablePizzaArbiter/prvCpssDxChPortPizzaArbiter.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortDynamicPizzaArbiter.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/cpssDxChPortDynamicPizzaArbiterWorkConserving.h>
#include <cpss/dxCh/dxChxGen/port/SliceManipulation/prvCpssDxChPortSlices.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortDynamicPAUnitBW.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/cpssDxChPortDynamicPAUnitBW.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortDynamicPAPortSpeedDB.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortDynamicPizzaArbiterWS.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortTxPizzaResourceFalcon.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>
#include <appDemo/boardConfig/appDemoCfgMisc.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortTxQHighSpeedPortsDrv.h>

/*D2D calendars */
#include <cpss/common/labServices/port/gop/port/silicon/general/mvHwsD2dIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#ifdef ASIC_SIMULATION
    #include <asicSimulation/SCIB/scib.h>
#endif

#undef PA_UT_SEQ
#undef PA_UT_PRINT

#if defined(PA_UT_PRINT) || defined(PA_UT_SEQ)
    #define cpssOsPrintf printf
#endif

/* Valid port num value used for testing */
#define HWINIT_VALID_PORTNUM_CNS         0


/*local cpu port number in tx units*/
#define PRV_DIP6_TX_CPU_PORT_NUM_CNS    8

/*slice number in TXDMA pizza */
#define TXD_PIZZA_SLICE_NUM 33

/* for debug function prvCpssDxChPortTxFalconPizzaAndSpeedProfileDebug*/
/*#define falconDebugPizza 1*/

#ifdef falconDebugPizza

#define PRV_DIP6_CPU_PORT_NUM_OF_SLICE 1

static CPSS_PORT_SPEED_ENT falconPortsModes400GOptions[][8] = {
  /*400 *1*/
  /*0*/ {CPSS_PORT_SPEED_400G_E /*0*/, CPSS_PORT_SPEED_NA_E   /*1*/, CPSS_PORT_SPEED_NA_E   /*2*/, CPSS_PORT_SPEED_NA_E   /*3*/,CPSS_PORT_SPEED_NA_E   /*4*/,CPSS_PORT_SPEED_NA_E     /*5*/,CPSS_PORT_SPEED_NA_E   /*6*/ , CPSS_PORT_SPEED_NA_E   /*7*/},
  /*200 * 2*/
  /*1*/ {CPSS_PORT_SPEED_200G_E /*0*/, CPSS_PORT_SPEED_NA_E   /*1*/, CPSS_PORT_SPEED_NA_E   /*2*/, CPSS_PORT_SPEED_NA_E   /*3*/,CPSS_PORT_SPEED_200G_E /*4*/,CPSS_PORT_SPEED_NA_E    /*5*/,CPSS_PORT_SPEED_NA_E    /*6*/ , CPSS_PORT_SPEED_NA_E   /*7*/},
  /*100 * 4*/
  /*2*/ {CPSS_PORT_SPEED_100G_E /*0*/, CPSS_PORT_SPEED_NA_E   /*1*/, CPSS_PORT_SPEED_100G_E /*2*/, CPSS_PORT_SPEED_NA_E   /*3*/,CPSS_PORT_SPEED_100G_E /*4*/,CPSS_PORT_SPEED_NA_E     /*5*/,CPSS_PORT_SPEED_100G_E /*6*/ , CPSS_PORT_SPEED_NA_E   /*7*/},
  /*50 * 8*/
  /*3*/ {CPSS_PORT_SPEED_50000_E/*0*/, CPSS_PORT_SPEED_50000_E/*1*/, CPSS_PORT_SPEED_50000_E/*2*/, CPSS_PORT_SPEED_50000_E/*3*/,CPSS_PORT_SPEED_50000_E/*4*/,CPSS_PORT_SPEED_50000_E  /*5*/,CPSS_PORT_SPEED_50000_E /*6*/, CPSS_PORT_SPEED_50000_E /*7*/},
  /*200*1, 100*2 */
  /*4*/ {CPSS_PORT_SPEED_100G_E /*0*/, CPSS_PORT_SPEED_NA_E   /*1*/, CPSS_PORT_SPEED_100G_E /*2*/, CPSS_PORT_SPEED_NA_E   /*3*/,CPSS_PORT_SPEED_200G_E  /*4*/,CPSS_PORT_SPEED_NA_E    /*5*/,CPSS_PORT_SPEED_NA_E   /*6*/ , CPSS_PORT_SPEED_NA_E   /*7*/},
  /*5*/ {CPSS_PORT_SPEED_200G_E /*0*/, CPSS_PORT_SPEED_NA_E   /*1*/, CPSS_PORT_SPEED_NA_E   /*2*/, CPSS_PORT_SPEED_NA_E   /*3*/,CPSS_PORT_SPEED_100G_E  /*4*/,CPSS_PORT_SPEED_NA_E    /*5*/,CPSS_PORT_SPEED_100G_E /*6*/ , CPSS_PORT_SPEED_NA_E   /*7*/},
  /*200*1, 100*1, 50*2 */
  /*6*/ {CPSS_PORT_SPEED_200G_E /*0*/,  CPSS_PORT_SPEED_NA_E   /*1*/, CPSS_PORT_SPEED_NA_E  /*2*/, CPSS_PORT_SPEED_NA_E    /*3*/,CPSS_PORT_SPEED_100G_E /*4*/,CPSS_PORT_SPEED_NA_E    /*5*/,CPSS_PORT_SPEED_50000_E /*6*/, CPSS_PORT_SPEED_50000_E /*7*/},
  /*7*/ {CPSS_PORT_SPEED_200G_E /*0*/,  CPSS_PORT_SPEED_NA_E   /*1*/, CPSS_PORT_SPEED_NA_E  /*2*/, CPSS_PORT_SPEED_NA_E    /*3*/,CPSS_PORT_SPEED_50000_E/*4*/,CPSS_PORT_SPEED_50000_E /*5*/,CPSS_PORT_SPEED_100G_E  /*6*/, CPSS_PORT_SPEED_NA_E    /*7*/},
  /*8*/ {CPSS_PORT_SPEED_100G_E /*0*/,  CPSS_PORT_SPEED_NA_E   /*1*/, CPSS_PORT_SPEED_50000_E/*2*/, CPSS_PORT_SPEED_50000_E/*3*/,CPSS_PORT_SPEED_200G_E /*4*/,CPSS_PORT_SPEED_NA_E    /*5*/,CPSS_PORT_SPEED_NA_E    /*6*/, CPSS_PORT_SPEED_NA_E    /*7*/},
  /*9*/ {CPSS_PORT_SPEED_50000_E/*0*/, CPSS_PORT_SPEED_50000_E /*1*/, CPSS_PORT_SPEED_100G_E /*2*/, CPSS_PORT_SPEED_NA_E   /*3*/,CPSS_PORT_SPEED_200G_E /*4*/,CPSS_PORT_SPEED_NA_E    /*5*/,CPSS_PORT_SPEED_NA_E    /*6*/, CPSS_PORT_SPEED_NA_E    /*7*/},
  /*200*2, 50*4 */
  /*10*/ {CPSS_PORT_SPEED_50000_E/*0*/, CPSS_PORT_SPEED_50000_E/*1*/, CPSS_PORT_SPEED_50000_E/*2*/, CPSS_PORT_SPEED_50000_E/*3*/,CPSS_PORT_SPEED_200G_E /*4*/,CPSS_PORT_SPEED_NA_E    /*5*/,CPSS_PORT_SPEED_NA_E    /*6*/, CPSS_PORT_SPEED_NA_E    /*7*/},
  /*11*/ {CPSS_PORT_SPEED_200G_E /*0*/, CPSS_PORT_SPEED_NA_E   /*1*/, CPSS_PORT_SPEED_NA_E   /*2*/, CPSS_PORT_SPEED_NA_E   /*3*/,CPSS_PORT_SPEED_50000_E/*4*/,CPSS_PORT_SPEED_50000_E /*5*/,CPSS_PORT_SPEED_50000_E /*6*/, CPSS_PORT_SPEED_50000_E /*7*/},
  /*100*1, 50*6 */
  /*12*/ {CPSS_PORT_SPEED_100G_E /*0*/, CPSS_PORT_SPEED_NA_E   /*1*/, CPSS_PORT_SPEED_50000_E/*2*/, CPSS_PORT_SPEED_50000_E/*3*/,CPSS_PORT_SPEED_50000_E/*4*/,CPSS_PORT_SPEED_50000_E /*5*/,CPSS_PORT_SPEED_50000_E /*6*/, CPSS_PORT_SPEED_50000_E /*7*/},
  /*13*/ {CPSS_PORT_SPEED_50000_E/*0*/, CPSS_PORT_SPEED_50000_E/*1*/, CPSS_PORT_SPEED_100G_E /*2*/, CPSS_PORT_SPEED_NA_E   /*3*/,CPSS_PORT_SPEED_50000_E/*4*/,CPSS_PORT_SPEED_50000_E /*5*/,CPSS_PORT_SPEED_50000_E /*6*/, CPSS_PORT_SPEED_50000_E /*7*/},
  /*14*/ {CPSS_PORT_SPEED_50000_E/*0*/, CPSS_PORT_SPEED_50000_E/*1*/, CPSS_PORT_SPEED_50000_E/*2*/, CPSS_PORT_SPEED_50000_E/*3*/,CPSS_PORT_SPEED_100G_E /*4*/,CPSS_PORT_SPEED_NA_E    /*5*/,CPSS_PORT_SPEED_50000_E /*6*/, CPSS_PORT_SPEED_50000_E /*7*/},
  /*15*/ {CPSS_PORT_SPEED_50000_E/*0*/, CPSS_PORT_SPEED_50000_E/*1*/, CPSS_PORT_SPEED_50000_E/*2*/, CPSS_PORT_SPEED_50000_E/*3*/,CPSS_PORT_SPEED_50000_E/*4*/,CPSS_PORT_SPEED_50000_E /*5*/,CPSS_PORT_SPEED_100G_E  /*6*/, CPSS_PORT_SPEED_NA_E    /*7*/},
  /*100*2, 50*4 */
  /*16*/ {CPSS_PORT_SPEED_100G_E /*0*/, CPSS_PORT_SPEED_NA_E   /*1*/, CPSS_PORT_SPEED_100G_E /*2*/, CPSS_PORT_SPEED_NA_E   /*3*/,CPSS_PORT_SPEED_50000_E/*4*/,CPSS_PORT_SPEED_50000_E /*5*/,CPSS_PORT_SPEED_50000_E /*6*/, CPSS_PORT_SPEED_50000_E /*7*/},
  /*17*/ {CPSS_PORT_SPEED_50000_E/*0*/, CPSS_PORT_SPEED_50000_E/*1*/, CPSS_PORT_SPEED_100G_E /*2*/, CPSS_PORT_SPEED_NA_E   /*3*/,CPSS_PORT_SPEED_100G_E /*4*/,CPSS_PORT_SPEED_NA_E    /*5*/,CPSS_PORT_SPEED_50000_E /*6*/, CPSS_PORT_SPEED_50000_E /*7*/},
  /*18*/ {CPSS_PORT_SPEED_100G_E /*0*/, CPSS_PORT_SPEED_NA_E   /*1*/, CPSS_PORT_SPEED_50000_E/*2*/, CPSS_PORT_SPEED_50000_E/*3*/,CPSS_PORT_SPEED_100G_E /*4*/,CPSS_PORT_SPEED_NA_E    /*5*/,CPSS_PORT_SPEED_50000_E /*6*/, CPSS_PORT_SPEED_50000_E /*7*/},
  /*19*/ {CPSS_PORT_SPEED_100G_E /*0*/, CPSS_PORT_SPEED_NA_E   /*1*/, CPSS_PORT_SPEED_50000_E/*2*/, CPSS_PORT_SPEED_50000_E/*3*/,CPSS_PORT_SPEED_50000_E/*4*/,CPSS_PORT_SPEED_50000_E /*5*/ ,CPSS_PORT_SPEED_100G_E /*6*/, CPSS_PORT_SPEED_NA_E   /*7*/},
  /*20*/ {CPSS_PORT_SPEED_50000_E/*0*/, CPSS_PORT_SPEED_50000_E/*1*/, CPSS_PORT_SPEED_50000_E/*2*/, CPSS_PORT_SPEED_50000_E/*3*/,CPSS_PORT_SPEED_100G_E /*4*/,CPSS_PORT_SPEED_NA_E    /*5*/ ,CPSS_PORT_SPEED_100G_E /*6*/, CPSS_PORT_SPEED_NA_E   /*7*/},
  /*21*/ {CPSS_PORT_SPEED_50000_E/*0*/, CPSS_PORT_SPEED_50000_E/*1*/, CPSS_PORT_SPEED_100G_E /*2*/, CPSS_PORT_SPEED_NA_E   /*3*/,CPSS_PORT_SPEED_50000_E/*4*/,CPSS_PORT_SPEED_50000_E /*5*/ ,CPSS_PORT_SPEED_100G_E /*6*/, CPSS_PORT_SPEED_NA_E   /*7*/},
  /*100*3, 50*2 */
  /*22*/ {CPSS_PORT_SPEED_50000_E/*0*/, CPSS_PORT_SPEED_50000_E/*1*/, CPSS_PORT_SPEED_100G_E /*2*/, CPSS_PORT_SPEED_NA_E   /*3*/,CPSS_PORT_SPEED_100G_E /*4*/,CPSS_PORT_SPEED_NA_E   /*5*/,CPSS_PORT_SPEED_100G_E   /*6*/, CPSS_PORT_SPEED_NA_E    /*7*/},
  /*23*/ {CPSS_PORT_SPEED_100G_E /*0*/, CPSS_PORT_SPEED_NA_E   /*1*/, CPSS_PORT_SPEED_50000_E/*2*/, CPSS_PORT_SPEED_50000_E/*3*/,CPSS_PORT_SPEED_100G_E /*4*/,CPSS_PORT_SPEED_NA_E   /*5*/,CPSS_PORT_SPEED_100G_E   /*6*/, CPSS_PORT_SPEED_NA_E    /*7*/},
  /*24*/ {CPSS_PORT_SPEED_100G_E /*0*/, CPSS_PORT_SPEED_NA_E   /*1*/, CPSS_PORT_SPEED_100G_E /*2*/, CPSS_PORT_SPEED_NA_E   /*3*/,CPSS_PORT_SPEED_50000_E/*4*/,CPSS_PORT_SPEED_50000_E/*5*/,CPSS_PORT_SPEED_100G_E   /*6*/, CPSS_PORT_SPEED_NA_E    /*7*/},
  /*25*/ {CPSS_PORT_SPEED_100G_E /*0*/, CPSS_PORT_SPEED_NA_E   /*1*/, CPSS_PORT_SPEED_100G_E /*2*/, CPSS_PORT_SPEED_NA_E   /*3*/,CPSS_PORT_SPEED_100G_E /*4*/,CPSS_PORT_SPEED_NA_E   /*5*/,CPSS_PORT_SPEED_50000_E  /*6*/, CPSS_PORT_SPEED_50000_E /*7*/},
  /*50,40,25,20,10,2.5,1,0.1 */
  /*26*/ {CPSS_PORT_SPEED_50000_E/*0*/, CPSS_PORT_SPEED_40000_E/*1*/, CPSS_PORT_SPEED_25000_E /*2*/, CPSS_PORT_SPEED_20000_E/*3*/,CPSS_PORT_SPEED_10000_E/*4*/,CPSS_PORT_SPEED_2500_E  /*5*/,CPSS_PORT_SPEED_1000_E /*6*/, CPSS_PORT_SPEED_100_E   /*7*/},
};

static GT_U32 portsModes400GOptionsNum =
    NUM_ELEMENTS_IN_ARR_MAC(falconPortsModes400GOptions);

static PRV_CPSS_SPEED_PROFILE_ENT falconPortsModes400GOptionsExpectedSpeedProfile[][8] = {
/*400 *1*/
  {PRV_CPSS_SPEED_PROFILE_0_E /*0*/, PRV_CPSS_PORT_SPEED_PROFILE_NA_E   /*1*/, PRV_CPSS_PORT_SPEED_PROFILE_NA_E   /*2*/, PRV_CPSS_PORT_SPEED_PROFILE_NA_E   /*3*/,PRV_CPSS_PORT_SPEED_PROFILE_NA_E  /*4*/,PRV_CPSS_PORT_SPEED_PROFILE_NA_E   /*5*/,PRV_CPSS_PORT_SPEED_PROFILE_NA_E   /*6*/ ,  PRV_CPSS_PORT_SPEED_PROFILE_NA_E    /*7*/},
  /*200 * 2*/
  {PRV_CPSS_SPEED_PROFILE_1_E /*0*/, PRV_CPSS_PORT_SPEED_PROFILE_NA_E   /*1*/, PRV_CPSS_PORT_SPEED_PROFILE_NA_E   /*2*/, PRV_CPSS_PORT_SPEED_PROFILE_NA_E   /*3*/,PRV_CPSS_SPEED_PROFILE_1_E /*4*/,PRV_CPSS_PORT_SPEED_PROFILE_NA_E   /*5*/,PRV_CPSS_PORT_SPEED_PROFILE_NA_E   /*6*/ , PRV_CPSS_PORT_SPEED_PROFILE_NA_E    /*7*/},
  /*100 * 4*/
  {PRV_CPSS_SPEED_PROFILE_2_E /*0*/, PRV_CPSS_PORT_SPEED_PROFILE_NA_E   /*1*/, PRV_CPSS_SPEED_PROFILE_2_E         /*2*/, PRV_CPSS_PORT_SPEED_PROFILE_NA_E   /*3*/,PRV_CPSS_SPEED_PROFILE_2_E /*4*/, PRV_CPSS_PORT_SPEED_PROFILE_NA_E   /*5*/,PRV_CPSS_SPEED_PROFILE_2_E   /*6*/ , PRV_CPSS_PORT_SPEED_PROFILE_NA_E /*7*/},
  /*50 * 8*/
  {PRV_CPSS_SPEED_PROFILE_3_E /*0*/, PRV_CPSS_SPEED_PROFILE_3_E         /*1*/, PRV_CPSS_SPEED_PROFILE_3_E         /*2*/, PRV_CPSS_SPEED_PROFILE_3_E         /*3*/,PRV_CPSS_SPEED_PROFILE_3_E /*4*/,PRV_CPSS_SPEED_PROFILE_3_E          /*5*/ ,PRV_CPSS_SPEED_PROFILE_3_E /*6*/, PRV_CPSS_SPEED_PROFILE_3_E /*7*/},
  /*200*1, 100*2 */
  {PRV_CPSS_SPEED_PROFILE_2_E /*0*/, PRV_CPSS_PORT_SPEED_PROFILE_NA_E   /*1*/, PRV_CPSS_SPEED_PROFILE_2_E /*2*/, PRV_CPSS_PORT_SPEED_PROFILE_NA_E   /*3*/,PRV_CPSS_SPEED_PROFILE_1_E  /*4*/, PRV_CPSS_PORT_SPEED_PROFILE_NA_E   /*5*/, PRV_CPSS_PORT_SPEED_PROFILE_NA_E   /*6*/ , PRV_CPSS_PORT_SPEED_PROFILE_NA_E    /*7*/},
  {PRV_CPSS_SPEED_PROFILE_1_E /*0*/, PRV_CPSS_PORT_SPEED_PROFILE_NA_E   /*1*/, PRV_CPSS_PORT_SPEED_PROFILE_NA_E   /*2*/, PRV_CPSS_PORT_SPEED_PROFILE_NA_E   /*3*/,PRV_CPSS_SPEED_PROFILE_2_E  /*4*/,PRV_CPSS_PORT_SPEED_PROFILE_NA_E   /*5*/,PRV_CPSS_SPEED_PROFILE_2_E   /*6*/ , PRV_CPSS_PORT_SPEED_PROFILE_NA_E /*7*/},
  /*200*1, 100*1, 50*2 */
  {PRV_CPSS_SPEED_PROFILE_1_E /*0*/, PRV_CPSS_PORT_SPEED_PROFILE_NA_E   /*1*/, PRV_CPSS_PORT_SPEED_PROFILE_NA_E  /*2*/, PRV_CPSS_PORT_SPEED_PROFILE_NA_E    /*3*/, PRV_CPSS_SPEED_PROFILE_2_E /*4*/, PRV_CPSS_PORT_SPEED_PROFILE_NA_E  /*5*/,PRV_CPSS_SPEED_PROFILE_3_E          /*6*/ , PRV_CPSS_SPEED_PROFILE_3_E       /*7*/},
  {PRV_CPSS_SPEED_PROFILE_1_E /*0*/, PRV_CPSS_PORT_SPEED_PROFILE_NA_E   /*1*/, PRV_CPSS_PORT_SPEED_PROFILE_NA_E  /*2*/, PRV_CPSS_PORT_SPEED_PROFILE_NA_E    /*3*/, PRV_CPSS_SPEED_PROFILE_3_E /*4*/, PRV_CPSS_SPEED_PROFILE_3_E        /*5*/,PRV_CPSS_SPEED_PROFILE_2_E          /*6*/ , PRV_CPSS_PORT_SPEED_PROFILE_NA_E /*7*/},
  {PRV_CPSS_SPEED_PROFILE_2_E /*0*/, PRV_CPSS_PORT_SPEED_PROFILE_NA_E   /*1*/, PRV_CPSS_SPEED_PROFILE_3_E        /*2*/, PRV_CPSS_SPEED_PROFILE_3_E          /*3*/, PRV_CPSS_SPEED_PROFILE_1_E  /*4*/,PRV_CPSS_PORT_SPEED_PROFILE_NA_E   /*5*/,PRV_CPSS_PORT_SPEED_PROFILE_NA_E   /*6*/ , PRV_CPSS_PORT_SPEED_PROFILE_NA_E  /*7*/},
  {PRV_CPSS_SPEED_PROFILE_3_E /*0*/, PRV_CPSS_SPEED_PROFILE_3_E         /*1*/, PRV_CPSS_SPEED_PROFILE_2_E        /*2*/, PRV_CPSS_PORT_SPEED_PROFILE_NA_E    /*3*/, PRV_CPSS_SPEED_PROFILE_1_E  /*4*/, PRV_CPSS_PORT_SPEED_PROFILE_NA_E  /*5*/, PRV_CPSS_PORT_SPEED_PROFILE_NA_E  /*6*/ , PRV_CPSS_PORT_SPEED_PROFILE_NA_E  /*7*/},
  /*200*2, 50*4 */
  {PRV_CPSS_SPEED_PROFILE_3_E /*0*/, PRV_CPSS_SPEED_PROFILE_3_E        /*1*/, PRV_CPSS_SPEED_PROFILE_3_E        /*2*/, PRV_CPSS_SPEED_PROFILE_3_E        /*3*/,PRV_CPSS_SPEED_PROFILE_1_E  /*4*/,PRV_CPSS_PORT_SPEED_PROFILE_NA_E  /*5*/,PRV_CPSS_PORT_SPEED_PROFILE_NA_E  /*6*/ , PRV_CPSS_PORT_SPEED_PROFILE_NA_E  /*7*/},
  {PRV_CPSS_SPEED_PROFILE_1_E /*0*/, PRV_CPSS_PORT_SPEED_PROFILE_NA_E  /*1*/, PRV_CPSS_PORT_SPEED_PROFILE_NA_E  /*2*/, PRV_CPSS_PORT_SPEED_PROFILE_NA_E  /*3*/,PRV_CPSS_SPEED_PROFILE_3_E  /*4*/,PRV_CPSS_SPEED_PROFILE_3_E        /*5*/,PRV_CPSS_SPEED_PROFILE_3_E        /*6*/ , PRV_CPSS_SPEED_PROFILE_3_E        /*7*/},
  /*100*1, 50*6 */
  {PRV_CPSS_SPEED_PROFILE_2_E /*0*/, PRV_CPSS_PORT_SPEED_PROFILE_NA_E /*1*/, PRV_CPSS_SPEED_PROFILE_3_E /*2*/, PRV_CPSS_SPEED_PROFILE_3_E        /*3*/,PRV_CPSS_SPEED_PROFILE_3_E /*4*/,PRV_CPSS_SPEED_PROFILE_3_E        /*5*/,PRV_CPSS_SPEED_PROFILE_3_E   /*6*/ , PRV_CPSS_SPEED_PROFILE_3_E      /*7*/},
  {PRV_CPSS_SPEED_PROFILE_3_E /*0*/, PRV_CPSS_SPEED_PROFILE_3_E       /*1*/, PRV_CPSS_SPEED_PROFILE_2_E /*2*/, PRV_CPSS_PORT_SPEED_PROFILE_NA_E  /*3*/,PRV_CPSS_SPEED_PROFILE_3_E /*4*/,PRV_CPSS_SPEED_PROFILE_3_E        /*5*/,PRV_CPSS_SPEED_PROFILE_3_E   /*6*/ , PRV_CPSS_SPEED_PROFILE_3_E       /*7*/},
  {PRV_CPSS_SPEED_PROFILE_3_E /*0*/, PRV_CPSS_SPEED_PROFILE_3_E       /*1*/, PRV_CPSS_SPEED_PROFILE_3_E /*2*/, PRV_CPSS_SPEED_PROFILE_3_E        /*3*/,PRV_CPSS_SPEED_PROFILE_2_E /*4*/,PRV_CPSS_PORT_SPEED_PROFILE_NA_E  /*5*/,PRV_CPSS_SPEED_PROFILE_3_E   /*6*/ , PRV_CPSS_SPEED_PROFILE_3_E       /*7*/},
  {PRV_CPSS_SPEED_PROFILE_3_E /*0*/, PRV_CPSS_SPEED_PROFILE_3_E       /*1*/, PRV_CPSS_SPEED_PROFILE_3_E /*2*/, PRV_CPSS_SPEED_PROFILE_3_E        /*3*/,PRV_CPSS_SPEED_PROFILE_3_E /*4*/,PRV_CPSS_SPEED_PROFILE_3_E        /*5*/,PRV_CPSS_SPEED_PROFILE_2_E   /*6*/ , PRV_CPSS_PORT_SPEED_PROFILE_NA_E /*7*/},
  /*100*2, 50*4 */
  {PRV_CPSS_SPEED_PROFILE_2_E /*0*/, PRV_CPSS_PORT_SPEED_PROFILE_NA_E   /*1*/, PRV_CPSS_SPEED_PROFILE_2_E /*2*/, PRV_CPSS_PORT_SPEED_PROFILE_NA_E  /*3*/,PRV_CPSS_SPEED_PROFILE_3_E /*4*/,PRV_CPSS_SPEED_PROFILE_3_E        /*5*/,PRV_CPSS_SPEED_PROFILE_3_E   /*6*/, PRV_CPSS_SPEED_PROFILE_3_E       /*7*/},
  {PRV_CPSS_SPEED_PROFILE_3_E /*0*/, PRV_CPSS_SPEED_PROFILE_3_E         /*1*/, PRV_CPSS_SPEED_PROFILE_2_E /*2*/, PRV_CPSS_PORT_SPEED_PROFILE_NA_E  /*3*/,PRV_CPSS_SPEED_PROFILE_2_E /*4*/,PRV_CPSS_PORT_SPEED_PROFILE_NA_E  /*5*/,PRV_CPSS_SPEED_PROFILE_3_E   /*6*/, PRV_CPSS_SPEED_PROFILE_3_E       /*7*/},
  {PRV_CPSS_SPEED_PROFILE_2_E /*0*/, PRV_CPSS_PORT_SPEED_PROFILE_NA_E   /*1*/, PRV_CPSS_SPEED_PROFILE_3_E /*2*/, PRV_CPSS_SPEED_PROFILE_3_E        /*3*/,PRV_CPSS_SPEED_PROFILE_2_E /*4*/,PRV_CPSS_PORT_SPEED_PROFILE_NA_E  /*5*/,PRV_CPSS_SPEED_PROFILE_3_E   /*6*/, PRV_CPSS_SPEED_PROFILE_3_E       /*7*/},
  {PRV_CPSS_SPEED_PROFILE_2_E /*0*/, PRV_CPSS_PORT_SPEED_PROFILE_NA_E   /*1*/, PRV_CPSS_SPEED_PROFILE_3_E /*2*/, PRV_CPSS_SPEED_PROFILE_3_E        /*3*/,PRV_CPSS_SPEED_PROFILE_3_E /*4*/,PRV_CPSS_SPEED_PROFILE_3_E        /*5*/ ,PRV_CPSS_SPEED_PROFILE_2_E  /*6*/, PRV_CPSS_PORT_SPEED_PROFILE_NA_E /*7*/},
  {PRV_CPSS_SPEED_PROFILE_3_E /*0*/, PRV_CPSS_SPEED_PROFILE_3_E         /*1*/, PRV_CPSS_SPEED_PROFILE_3_E /*2*/, PRV_CPSS_SPEED_PROFILE_3_E        /*3*/,PRV_CPSS_SPEED_PROFILE_2_E /*4*/,PRV_CPSS_PORT_SPEED_PROFILE_NA_E  /*5*/ ,PRV_CPSS_SPEED_PROFILE_2_E  /*6*/, PRV_CPSS_PORT_SPEED_PROFILE_NA_E /*7*/},
  {PRV_CPSS_SPEED_PROFILE_3_E /*0*/, PRV_CPSS_SPEED_PROFILE_3_E         /*1*/, PRV_CPSS_SPEED_PROFILE_2_E /*2*/, PRV_CPSS_PORT_SPEED_PROFILE_NA_E  /*3*/,PRV_CPSS_SPEED_PROFILE_3_E /*4*/,PRV_CPSS_SPEED_PROFILE_3_E        /*5*/ ,PRV_CPSS_SPEED_PROFILE_2_E  /*6*/, PRV_CPSS_PORT_SPEED_PROFILE_NA_E /*7*/},
  /*100*3, 50*2 */
  {PRV_CPSS_SPEED_PROFILE_3_E /*0*/, PRV_CPSS_SPEED_PROFILE_3_E         /*1*/, PRV_CPSS_SPEED_PROFILE_2_E /*2*/, PRV_CPSS_PORT_SPEED_PROFILE_NA_E   /*3*/,PRV_CPSS_SPEED_PROFILE_2_E  /*4*/,PRV_CPSS_PORT_SPEED_PROFILE_NA_E   /*5*/,PRV_CPSS_SPEED_PROFILE_2_E   /*6*/, PRV_CPSS_PORT_SPEED_PROFILE_NA_E /*7*/},
  {PRV_CPSS_SPEED_PROFILE_2_E /*0*/, PRV_CPSS_PORT_SPEED_PROFILE_NA_E   /*1*/, PRV_CPSS_SPEED_PROFILE_3_E /*2*/, PRV_CPSS_SPEED_PROFILE_3_E         /*3*/,PRV_CPSS_SPEED_PROFILE_2_E  /*4*/,PRV_CPSS_PORT_SPEED_PROFILE_NA_E   /*5*/,PRV_CPSS_SPEED_PROFILE_2_E   /*6*/, PRV_CPSS_PORT_SPEED_PROFILE_NA_E /*7*/},
  {PRV_CPSS_SPEED_PROFILE_2_E /*0*/, PRV_CPSS_PORT_SPEED_PROFILE_NA_E   /*1*/, PRV_CPSS_SPEED_PROFILE_2_E /*2*/, PRV_CPSS_PORT_SPEED_PROFILE_NA_E   /*3*/,PRV_CPSS_SPEED_PROFILE_3_E  /*4*/,PRV_CPSS_SPEED_PROFILE_3_E         /*5*/,PRV_CPSS_SPEED_PROFILE_2_E   /*6*/, PRV_CPSS_PORT_SPEED_PROFILE_NA_E /*7*/},
  {PRV_CPSS_SPEED_PROFILE_2_E /*0*/, PRV_CPSS_PORT_SPEED_PROFILE_NA_E   /*1*/, PRV_CPSS_SPEED_PROFILE_2_E /*2*/, PRV_CPSS_PORT_SPEED_PROFILE_NA_E   /*3*/,PRV_CPSS_SPEED_PROFILE_2_E  /*4*/,PRV_CPSS_PORT_SPEED_PROFILE_NA_E   /*5*/,PRV_CPSS_SPEED_PROFILE_3_E   /*6*/, PRV_CPSS_SPEED_PROFILE_3_E       /*7*/},
  /*50,40,25,20,10,2.5,1,0.1 */
  {PRV_CPSS_SPEED_PROFILE_3_E /*0*/, PRV_CPSS_SPEED_PROFILE_4_E         /*1*/, PRV_CPSS_SPEED_PROFILE_5_E /*2*/, PRV_CPSS_SPEED_PROFILE_6_E         /*3*/,PRV_CPSS_SPEED_PROFILE_7_E  /*4*/,PRV_CPSS_SPEED_PROFILE_7_E         /*5*/,PRV_CPSS_SPEED_PROFILE_7_E   /*6*/, PRV_CPSS_SPEED_PROFILE_7_E       /*7*/},
};

static GT_U32 falconPortsModes400GOptionsExpectedPizzaSlicesBitMap[][8] = {
  {0xffffffff /*0*/, 0x00000000  /*1*/, 0x00000000 /*2*/, 0x00000000 /*3*/, 0x00000000 /*4*/, 0x00000000   /*5*/, 0x00000000   /*6*/, 0x00000000  /*7*/},
  /*200 * 2*/
  {0x55555555 /*0*/, 0x00000000  /*1*/, 0x00000000 /*2*/, 0x00000000 /*3*/, 0xaaaaaaaa /*4*/, 0x00000000   /*5*/, 0x00000000   /*6*/, 0x00000000  /*7*/},
  /*100 * 4*/
  {0x11111111 /*0*/, 0x00000000  /*1*/, 0x44444444 /*2*/, 0x00000000 /*3*/, 0x22222222 /*4*/, 0x00000000   /*5*/, 0x88888888   /*6*/, 0x00000000  /*7*/},
  /*50 * 8*/
  {0x01010101 /*0*/, 0x10101010  /*1*/, 0x04040404 /*2*/, 0x40404040 /*3*/, 0x02020202 /*4*/, 0x20202020   /*5*/, 0x08080808   /*6*/, 0x80808080  /*7*/},
  /*200*1, 100*2 */
  {0x11111111 /*0*/, 0x00000000  /*1*/, 0x44444444 /*2*/, 0x00000000 /*3*/, 0xaaaaaaaa /*4*/, 0x00000000   /*5*/, 0x00000000   /*6*/, 0x00000000  /*7*/},
  {0x55555555 /*0*/, 0x00000000  /*1*/, 0x00000000 /*2*/, 0x00000000 /*3*/, 0x22222222 /*4*/, 0x00000000   /*5*/, 0x88888888   /*6*/, 0x00000000  /*7*/},
  /*200*1, 100*1, 50*2 */
  {0x55555555 /*0*/, 0x00000000  /*1*/, 0x00000000 /*2*/, 0x00000000 /*3*/, 0x22222222 /*4*/, 0x00000000   /*5*/, 0x08080808   /*6*/, 0x80808080  /*7*/},
  {0x55555555 /*0*/, 0x00000000  /*1*/, 0x00000000 /*2*/, 0x00000000 /*3*/, 0x02020202 /*4*/, 0x20202020   /*5*/, 0x88888888   /*6*/, 0x00000000  /*7*/},
  {0x11111111 /*0*/, 0x00000000  /*1*/, 0x04040404 /*2*/, 0x40404040 /*3*/, 0xaaaaaaaa /*4*/, 0x00000000   /*5*/, 0x00000000   /*6*/, 0x00000000  /*7*/},
  {0x01010101 /*0*/, 0x10101010  /*1*/, 0x44444444 /*2*/, 0x00000000 /*3*/, 0xaaaaaaaa /*4*/, 0x00000000   /*5*/, 0x00000000   /*6*/, 0x00000000  /*7*/},
  /*200*2, 50*4 */
  {0x01010101 /*0*/, 0x10101010  /*1*/, 0x04040404 /*2*/, 0x40404040 /*3*/, 0xaaaaaaaa /*4*/, 0x00000000   /*5*/, 0x00000000   /*6*/, 0x00000000  /*7*/},
  {0x55555555 /*0*/, 0x00000000  /*1*/, 0x00000000 /*2*/, 0x00000000 /*3*/, 0x02020202 /*4*/, 0x20202020   /*5*/, 0x08080808   /*6*/, 0x80808080  /*7*/},
  /*100*1, 50*6 */
  {0x11111111 /*0*/, 0x00000000  /*1*/, 0x04040404 /*2*/, 0x40404040 /*3*/, 0x02020202 /*4*/, 0x20202020   /*5*/, 0x08080808   /*6*/, 0x80808080  /*7*/},
  {0x01010101 /*0*/, 0x10101010  /*1*/, 0x44444444 /*2*/, 0x00000000 /*3*/, 0x02020202 /*4*/, 0x20202020   /*5*/, 0x08080808   /*6*/, 0x80808080  /*7*/},
  {0x01010101 /*0*/, 0x10101010  /*1*/, 0x04040404 /*2*/, 0x40404040 /*3*/, 0x22222222 /*4*/, 0x00000000   /*5*/, 0x08080808   /*6*/, 0x80808080  /*7*/},
  {0x01010101 /*0*/, 0x10101010  /*1*/, 0x04040404 /*2*/, 0x40404040 /*3*/, 0x02020202 /*4*/, 0x20202020   /*5*/, 0x88888888   /*6*/, 0x00000000  /*7*/},
  /*100*2, 50*4 */
  {0x11111111 /*0*/, 0x00000000  /*1*/, 0x44444444 /*2*/, 0x00000000 /*3*/, 0x02020202 /*4*/, 0x20202020   /*5*/, 0x08080808   /*6*/, 0x80808080  /*7*/},
  {0x01010101 /*0*/, 0x10101010  /*1*/, 0x44444444 /*2*/, 0x00000000 /*3*/, 0x22222222 /*4*/, 0x00000000   /*5*/, 0x08080808   /*6*/, 0x80808080  /*7*/},
  {0x11111111 /*0*/, 0x00000000  /*1*/, 0x04040404 /*2*/, 0x40404040 /*3*/, 0x22222222 /*4*/, 0x00000000   /*5*/, 0x08080808   /*6*/, 0x80808080  /*7*/},
  {0x11111111 /*0*/, 0x00000000  /*1*/, 0x04040404 /*2*/, 0x40404040 /*3*/, 0x02020202 /*4*/, 0x20202020   /*5*/, 0x88888888   /*6*/, 0x00000000  /*7*/},
  {0x01010101 /*0*/, 0x10101010  /*1*/, 0x04040404 /*2*/, 0x40404040 /*3*/, 0x22222222 /*4*/, 0x00000000   /*5*/, 0x88888888   /*6*/, 0x00000000  /*7*/},
  {0x01010101 /*0*/, 0x10101010  /*1*/, 0x44444444 /*2*/, 0x00000000 /*3*/, 0x02020202 /*4*/, 0x20202020   /*5*/, 0x88888888   /*6*/, 0x00000000  /*7*/},
  /*100*3, 50*2 */
  {0x01010101 /*0*/, 0x10101010  /*1*/, 0x44444444 /*2*/, 0x00000000 /*3*/, 0x22222222 /*4*/, 0x00000000   /*5*/, 0x88888888   /*6*/, 0x00000000  /*7*/},
  {0x11111111 /*0*/, 0x00000000  /*1*/, 0x04040404 /*2*/, 0x40404040 /*3*/, 0x22222222 /*4*/, 0x00000000   /*5*/, 0x88888888   /*6*/, 0x00000000  /*7*/},
  {0x11111111 /*0*/, 0x00000000  /*1*/, 0x44444444 /*2*/, 0x00000000 /*3*/, 0x02020202 /*4*/, 0x20202020   /*5*/, 0x88888888   /*6*/, 0x00000000  /*7*/},
  {0x11111111 /*0*/, 0x00000000  /*1*/, 0x44444444 /*2*/, 0x00000000 /*3*/, 0x22222222 /*4*/, 0x00000000   /*5*/, 0x08080808   /*6*/, 0x80808080  /*7*/},
  /*50,40,25,20,10,2.5,1,0.1 */
  {0x01010101 /*0*/, 0x10101010  /*1*/, 0x04040404 /*2*/, 0x40404040 /*3*/, 0x02020202 /*4*/, 0x20202020   /*5*/, 0x08080808   /*6*/, 0x80808080  /*7*/},
};

#endif /* falconDebugPizza*/

/* checks 400G options from 1G to 400G in diffrents kinds of serdes speeds andR: R1,R2, R4 R8 */
static GT_U32 D2dserdesSpeedAndNumOfLanes400GOptionts[][9/* 0..7 ports and cpu port*/][2/*pcs lane number, mac lanes number*/] =
{
    /* 0: 400*/
    {{_26_5625G_PAM4,8},{SPEED_NA,0},{SPEED_NA,0},{SPEED_NA,0},{SPEED_NA,0},{SPEED_NA,0},{SPEED_NA,0},{SPEED_NA,0},/*cpu*/{_25_78125G,1}},
    /*1: 400*/
    {{_28_28125G_PAM4,8},{SPEED_NA,0},{SPEED_NA,0},{SPEED_NA,0},{SPEED_NA,0},{SPEED_NA,0},{SPEED_NA,0},{SPEED_NA,0},/*cpu*/{_25_78125G,1}},
    /*2: R4 200 * 2*/
    {{_26_5625G_PAM4,4},{SPEED_NA,0},{SPEED_NA,0},{SPEED_NA,0},{_28_28125G_PAM4,4},{SPEED_NA,0},{SPEED_NA,0},{SPEED_NA,0},/*cpu*/{_25_78125G,1}},
    /*3: R8 200 */
    {{_26_5625G,8},{SPEED_NA,0},{SPEED_NA,0},{SPEED_NA,0},{SPEED_NA,0},{SPEED_NA,0},{SPEED_NA,0},{SPEED_NA,0},/*cpu*/{_25_78125G,1}},
    /*4: R2 100 * 4*/
    {{_26_5625G_PAM4,2},{SPEED_NA,0},{_26_5625G_PAM4,2},{SPEED_NA,0},{_28_28125G_PAM4,2},{SPEED_NA,0},{_26_5625G_PAM4,2},{SPEED_NA,0},/*cpu*/{_25_78125G,1}},
    /*5: R4 100 * 2*/
    {{_28_28125G,4},{SPEED_NA,0},{SPEED_NA,0},{SPEED_NA,0},{_26_5625G,4},{SPEED_NA,0},{SPEED_NA,0},{SPEED_NA,0},/*cpu*/{_28_28125G,1}},
    /*6: 50 * 8*/
    {{_26_5625G_PAM4,1},{_26_5625G_PAM4,1},{_28_28125G_PAM4,1},{_26_5625G_PAM4,1},{_28_28125G_PAM4,1},{_26_5625G_PAM4,1},{_28_28125G_PAM4,1},{_26_5625G_PAM4,1},/*cpu*/{_28_28125G,1}},
    /*7: 200 R4,100 R4*/
    {{_26_5625G_PAM4,4},{SPEED_NA,0},{SPEED_NA,0},{SPEED_NA,0},{_28_28125G,4},{SPEED_NA,0},{SPEED_NA,0},{SPEED_NA,0},/*cpu*/{_25_78125G,1}},
    /*8: 200 R4,100*2 R2*/
    {{_26_5625G_PAM4,4},{SPEED_NA,0},{SPEED_NA,0},{SPEED_NA,0},{_28_28125G_PAM4,2},{SPEED_NA,0},{_28_28125G_PAM4,2},{SPEED_NA,0},/*cpu*/{_25_78125G,1}},
    /*9: R2 R1 50 * 4*/
    {{_26_5625G,2},{SPEED_NA,0},{_28_28125G_PAM4,1},{_26_5625G_PAM4,1},{_28_28125G_PAM4,1},{_26_5625G_PAM4,1},{_28_28125G,2},{SPEED_NA,0},/*cpu*/{_28_28125G,1}},
    /*10: R2 20 *3, 50 R2 */
    {{_10_3125G,2},{SPEED_NA,0},{_10_3125G,2},{SPEED_NA,0},{_10_3125G,2},{SPEED_NA,0},{_28_28125G,2},{SPEED_NA,0},/*cpu*/{_28_28125G,1}},
    /*11: 50,25,50,25,100 R2 ,20 R2*/
    {{_28_28125G_PAM4,1},{_26_5625G,1},{_28_28125G_PAM4,1},{_26_5625G,1},{_28_28125G_PAM4,2},{SPEED_NA,0},{_10_3125G,2},{SPEED_NA,0},/* cpu*/{_25_78125G,1}},
    /*12: 5,10,3,5,10,1,10 R2 (->25*4)*/
    {{_5_15625G,1},{_10_3125G,1},{_3_125G,1},{_5_15625G,1},{_10_3125G,1},{_1_25G,1},{_5_15625G,2},{SPEED_NA,0},/*cpu*/{_25_78125G,1}},
    /*13: R4 40*2*/
    {{_10_3125G,4},{SPEED_NA,0},{SPEED_NA,0},{SPEED_NA,0},{_10_3125G,4},{SPEED_NA,0},{SPEED_NA,0},{SPEED_NA,0},/*cpu*/{_25_78125G,1}},

};

static GT_U32 D2dserdesSpeedAndNumOfLanes400GOptiontsNum =
    NUM_ELEMENTS_IN_ARR_MAC(D2dserdesSpeedAndNumOfLanes400GOptionts);

/* checks 400G options from 5G to 400G in diffrents kinds R and serdes speeds. R1,R2, R4 R8*/
static GT_U32 D2dserdesSpeedAndNumOfLanes400GOptiontsExpected[][9/* 0..7 ports and cpu port*/][2/*pcs slices number, macSlicesNumber*/] =
{
    /* 0: 400*/
    {{80,151},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},/*cpu*/{5,9}},
    /*1: 400*/
    {{80,151},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},/*cpu*/{5,9}},
    /*2: R4 200 * 2*/
    {{40,75},{0,0},{0,0},{0,0},{40,76},{0,0},{0,0},{0,0},/*cpu*/{5,9}},
    /*3: R8 200 */
    {{40,75},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},/*cpu*/{5,9}},
    /*4: R2 100 * 4*/
    {{20,37},{0,0},{20,38},{0,0},{20,38},{0,0},{20,38},{0,0},/*cpu*/{5,9}},
    /*5: R4 100 * 2*/
    {{20,37},{0,0},{0,0},{0,0},{20,38},{0,0},{0,0},{0,0},/*cpu*/{5,9}},
    /*6: 50 * 8*/
    {{10,18},{10,19},{10,19},{10,19},{10,19},{10,19},{10,19},{10,19},/*cpu*/{5,9}},
    /*7: 200 R4,100 R4*/
    {{40,75},{0,0},{0,0},{0,0},{20,38},{0,0},{0,0},{0,0},/*cpu*/{5,9}},
    /*8: 200 R4,100*2 R2*/
    {{40,75},{0,0},{0,0},{0,0},{20,38},{0,0},{20,38},{0,0},/*cpu*/{5,9}},
    /*9: R2 R1 50 * 4*/
    {{10,18},{0,0},{10,19},{10,19},{10,19},{10,19},{10,19},{0,0},/*cpu*/{5,9}},
    /*10: R2 20 *3, 50 R2 */
    {{5,9},{0,0},{5,9},{0,0},{5,9},{0,0},{10,19},{0,0},/*cpu*/{5,9}},
    /*11: 50,25,50,25,100 R2 ,20 R2*/
    {{10,18},{5,9},{10,19},{5,9},{20,38},{0,0},{5,9},{0,0},/* cpu*/{5,9}},
    /*12: 5,10,3,5,10,1,10 R2 (->25*4)*/
    {{5,9},{5,9},{5,9},{5,9},{5,9},{5,9},{5,9},{0,0},/*cpu*/{5,9}},
    /*13: R4 40*2*/
    {{10,18},{0,0},{0,0},{0,0},{10,19},{0,0},{0,0},{0,0},/*cpu*/{5,9}},
};



GT_STATUS portPrioritiesMatrixCompare
(
    IN CPSS_DXCH_PORT_PRIORITY_MATR_STC * matr1Ptr,
    IN CPSS_DXCH_PORT_PRIORITY_MATR_STC * matr2Ptr
)
{
    GT_U32 portIdx;
    CPSS_DXCH_PIZZA_PORT_ID * portId1Ptr;
    CPSS_DXCH_PIZZA_PORT_ID * portId2Ptr;


    if (matr1Ptr == NULL || matr1Ptr == NULL)
    {
        return GT_FAIL;
    }
    if (matr1Ptr->portN != matr2Ptr->portN)
    {
        return GT_FAIL;
    }
    /* CPSS_DXCH_PIZZA_PORT_ID  portPriorityList[PIZZA_MAX_PORTS_PER_PORTGROUP_CNS][PORTPRIORITY_MAX_CNS];*/
    for (portIdx = 0; portIdx < matr1Ptr->portN; portIdx++)
    {
        portId1Ptr = &matr1Ptr->portPriorityList[portIdx][0];
        portId2Ptr = &matr2Ptr->portPriorityList[portIdx][0];

        while( *portId1Ptr != CPSS_PA_INVALID_PORT && *portId2Ptr != CPSS_PA_INVALID_PORT)
        {
            if (*portId1Ptr != *portId2Ptr)
            {
                return GT_FAIL;
            }
            portId1Ptr ++;
            portId2Ptr ++;
        }
        if (*portId1Ptr != *portId2Ptr)
        {
            return GT_FAIL;
        }
    }
    return GT_OK;
}


GT_STATUS portPizzaCfgSliceListBySlicesNumCompare
(
    IN CPSS_DXCH_PORT_PIZZA_SLICENUM_TO_SLICE_LIST_STC * pPizzaConf1Ptr,
    IN CPSS_DXCH_PORT_PIZZA_SLICENUM_TO_SLICE_LIST_STC * pPizzaConf2Ptr
)
{
    if (NULL == pPizzaConf1Ptr && NULL == pPizzaConf2Ptr)
    {
        return GT_OK;
    }
    if (NULL == pPizzaConf1Ptr || NULL == pPizzaConf2Ptr)
    {
        return GT_FAIL;
    }

    if (pPizzaConf1Ptr->portNum != pPizzaConf2Ptr->portNum)
    {
        return GT_FAIL;
    }
    if (pPizzaConf1Ptr->totalSlices != pPizzaConf2Ptr->totalSlices)
    {
        return GT_FAIL;
    }
    while (pPizzaConf1Ptr->pizzaCfg->sliceNum2Config != 0 && pPizzaConf1Ptr->pizzaCfg->sliceNum2Config != 0)
    {
        if (pPizzaConf1Ptr->pizzaCfg->sliceNum2Config != pPizzaConf1Ptr->pizzaCfg->sliceNum2Config)
        {
            return GT_FAIL;
        }
    }
    /*----------------------*/
    /* one of them is 0 !!! */
    /*----------------------*/
    if (pPizzaConf1Ptr->pizzaCfg->sliceNum2Config != pPizzaConf1Ptr->pizzaCfg->sliceNum2Config)
    {
        return GT_FAIL;
    }
    return GT_OK;
}

GT_STATUS pizzaProfileCompare
(
    IN CPSS_DXCH_PIZZA_PROFILE_STC * profile1Ptr,
    IN CPSS_DXCH_PIZZA_PROFILE_STC * profile2Ptr
)
{
    GT_STATUS rc;
    if (NULL == profile1Ptr || NULL == profile2Ptr)
    {
        return GT_BAD_PTR;
    }
    if (profile1Ptr->portNum != profile2Ptr->portNum)
    {
        return GT_FAIL;
    }
    if (profile1Ptr->maxSliceNum != profile2Ptr->maxSliceNum)
    {
        return GT_FAIL;
    }
    if (profile1Ptr->txQDef.pizzaRepetitionNum != profile2Ptr->txQDef.pizzaRepetitionNum)
    {
        return GT_FAIL;
    }
    rc = portPrioritiesMatrixCompare(profile1Ptr->pPortPriorityMatrPtr,profile2Ptr->pPortPriorityMatrPtr);
    if (GT_OK != rc)
    {
        return rc;
    }

    return GT_OK;
}


/*------------------------------------------------------------------------------*/
/* User defined profile                                                         */
/*------------------------------------------------------------------------------*/

static CPSS_DXCH_PORT_SPEED_TO_SLICE_NUM_STC portSpeed2SliceNumListLionB0_UT[] =
{     /* port speed        ,     slices N */
     {   CPSS_PORT_SPEED_10000_E,     1 }
    ,{   CPSS_PORT_SPEED_20000_E,     2 }
    ,{   CPSS_PORT_SPEED_40000_E,     4 }
    ,{   CPSS_PORT_SPEED_100G_E,     10 }
    ,{   CPSS_PORT_SPEED_NA_E,        CPSS_INVALID_SLICE_NUM }
};


static const CPSS_DXCH_PORT_PIZZA_PORTGROUP_SPEED_TO_SLICENUM_STC portGroupPortSpeed2SliceNum_UT =
{
    /* 12 */
    {
         { 0 , &portSpeed2SliceNumListLionB0_UT[0] }
        ,{ 1 , &portSpeed2SliceNumListLionB0_UT[0] }
        ,{ 2 , &portSpeed2SliceNumListLionB0_UT[0] }
        ,{ 3 , &portSpeed2SliceNumListLionB0_UT[0] }
        ,{ 4 , &portSpeed2SliceNumListLionB0_UT[0] }
        ,{ 5 , &portSpeed2SliceNumListLionB0_UT[0] }
        ,{ 6 , &portSpeed2SliceNumListLionB0_UT[0] }
        ,{ 7 , &portSpeed2SliceNumListLionB0_UT[0] }
        ,{ 8 , &portSpeed2SliceNumListLionB0_UT[0] }
        ,{ 9 , &portSpeed2SliceNumListLionB0_UT[0] }
        ,{10 , &portSpeed2SliceNumListLionB0_UT[0] }
        ,{11 , &portSpeed2SliceNumListLionB0_UT[0] }
        ,{CPSS_PA_INVALID_PORT  , NULL     }
    }
};


/*---------------------------------------------------------------------------
**  Port Priority tables
**---------------------------------------------------------------------------
*/

const CPSS_DXCH_PORT_PRIORITY_MATR_STC portPriorityLion2_12ports_UT =
{
/* GT_U32        portN; */12
/* portPriorityList  */  ,{
           /* port  0*/           { 1,  2,  3, CPSS_PA_INVALID_PORT }
           /* port  1*/          ,{ CPSS_PA_INVALID_PORT }
           /* port  2*/          ,{ 3, CPSS_PA_INVALID_PORT }
           /* port  3*/          ,{ CPSS_PA_INVALID_PORT }
           /* port  4*/          ,{ 5, 6, 7, CPSS_PA_INVALID_PORT }
           /* port  5*/          ,{ CPSS_PA_INVALID_PORT }
           /* port  6*/          ,{ 7, CPSS_PA_INVALID_PORT }
           /* port  7*/          ,{ CPSS_PA_INVALID_PORT }
           /* port  8*/          ,{ 10, 11, CPSS_PA_INVALID_PORT }
           /* port  9*/          ,{ 11, CPSS_PA_INVALID_PORT }
           /* port 10*/          ,{ 11, CPSS_PA_INVALID_PORT }
           /* port 11*/          ,{ CPSS_PA_INVALID_PORT }
                          }
};

const CPSS_DXCH_PORT_PIZZA_CFG_PORT_SLICE_LIST_STC pizzaLion2B0_UT_ConfBy1Slice_RegMode =
{
    /* maxPortNum*/ 12
    /* portArr   */,{
        /* PORT 0  */         { 0  , CPSS_PA_INVALID_SLICE }
        /* PORT 1  */        ,{ 8  , CPSS_PA_INVALID_SLICE }
        /* PORT 2  */        ,{ 4  , CPSS_PA_INVALID_SLICE }
        /* PORT 3  */        ,{ 12 , CPSS_PA_INVALID_SLICE }
        /* PORT 4  */        ,{ 2  , CPSS_PA_INVALID_SLICE }
        /* PORT 5  */        ,{ 10 , CPSS_PA_INVALID_SLICE }
        /* PORT 6  */        ,{ 14 , CPSS_PA_INVALID_SLICE }
        /* PORT 7  */        ,{ 6  , CPSS_PA_INVALID_SLICE }
        /* PORT 8  */        ,{ 1  , CPSS_PA_INVALID_SLICE }
        /* PORT 9  */        ,{ 3  , CPSS_PA_INVALID_SLICE }
        /* PORT 10 */        ,{ 5  , CPSS_PA_INVALID_SLICE }
        /* PORT 11 */        ,{ 13 , CPSS_PA_INVALID_SLICE }
                    }
};

const CPSS_DXCH_PORT_PIZZA_CFG_PORT_SLICE_LIST_STC pizzaLion2B0_UT_ConfBy1Slice_ExtMode =
{
    /* maxPortNum*/ 12
    /* portArr   */,{
        /* PORT 0  */         { 0  , CPSS_PA_INVALID_SLICE }
        /* PORT 1  */        ,{ 8  , CPSS_PA_INVALID_SLICE }
        /* PORT 2  */        ,{ 4  , CPSS_PA_INVALID_SLICE }
        /* PORT 3  */        ,{ 12 , CPSS_PA_INVALID_SLICE }
        /* PORT 4  */        ,{ 2  , CPSS_PA_INVALID_SLICE }
        /* PORT 5  */        ,{ 10 , CPSS_PA_INVALID_SLICE }
        /* PORT 6  */        ,{ 14 , CPSS_PA_INVALID_SLICE }
        /* PORT 7  */        ,{ 6  , CPSS_PA_INVALID_SLICE }
        /* PORT 8  */        ,{ 1  , CPSS_PA_INVALID_SLICE }
        /* PORT 9  */        ,{ 3  , CPSS_PA_INVALID_SLICE }
        /* PORT 10 */        ,{ 5  , CPSS_PA_INVALID_SLICE }
        /* PORT 11 */        ,{ 7  , CPSS_PA_INVALID_SLICE }
    }
};


const CPSS_DXCH_PORT_PIZZA_CFG_PORT_SLICE_LIST_STC pizzaLion2B0_UT_ConfBy2Slice =
{
    /* maxPortNum*/ 12
    /* portArr   */,{
        /* PORT 0  */       { 0  , 8  , CPSS_PA_INVALID_SLICE }
        /* PORT 1  */      ,{ CPSS_PA_INVALID_SLICE }
        /* PORT 2  */      ,{ 4  , 12 , CPSS_PA_INVALID_SLICE }
        /* PORT 3  */      ,{ CPSS_PA_INVALID_SLICE }
        /* PORT 4  */      ,{ 2  , 10 , CPSS_PA_INVALID_SLICE }
        /* PORT 5  */      ,{ CPSS_PA_INVALID_SLICE }
        /* PORT 6  */      ,{ 6  , 14 , CPSS_PA_INVALID_SLICE }
        /* PORT 7  */      ,{ CPSS_PA_INVALID_SLICE }
        /* PORT 8  */      ,{ 1  , 9  , CPSS_PA_INVALID_SLICE }
        /* PORT 9  */      ,{ 3  , 11 , CPSS_PA_INVALID_SLICE }
        /* PORT 10 */      ,{ 5  , 13 , CPSS_PA_INVALID_SLICE }
        /* PORT 11 */      ,{ 7  , 15 , CPSS_PA_INVALID_SLICE }
    }
};



const CPSS_DXCH_PORT_PIZZA_CFG_PORT_SLICE_LIST_STC pizzaLion2B0_UT_ConfBy4Slice =
{
    /* maxPortNum*/ 12
    /* portArr   */,{
        /* PORT 0  */       { 0  , 4  , 8  , 12 , CPSS_PA_INVALID_SLICE }
        /* PORT 1  */      ,{ CPSS_PA_INVALID_SLICE }
        /* PORT 2  */      ,{ CPSS_PA_INVALID_SLICE }
        /* PORT 3  */      ,{ CPSS_PA_INVALID_SLICE }
        /* PORT 4  */      ,{ 2  , 6  , 10 , 14 , CPSS_PA_INVALID_SLICE }
        /* PORT 5  */      ,{ CPSS_PA_INVALID_SLICE }
        /* PORT 6  */      ,{ CPSS_PA_INVALID_SLICE }
        /* PORT 7  */      ,{ CPSS_PA_INVALID_SLICE }
        /* PORT 8  */      ,{ 1  , 5  , 9  , 13 , CPSS_PA_INVALID_SLICE }
        /* PORT 9  */      ,{ 3  , 7  , 11 , 15 , CPSS_PA_INVALID_SLICE }
        /* PORT 10 */      ,{ CPSS_PA_INVALID_SLICE }
        /* PORT 11 */      ,{ CPSS_PA_INVALID_SLICE }
    }
};


const CPSS_DXCH_PORT_PIZZA_SLICENUM_TO_SLICE_LIST_STC lion2B0_RegMode_UT =
{
    /* portNum */       12
    /* maxSliceNum */  ,12
    /* pizzaCfg */     ,{
        { 1, (CPSS_DXCH_PORT_PIZZA_CFG_PORT_SLICE_LIST_STC *)&pizzaLion2B0_UT_ConfBy1Slice_RegMode }
        ,{ 2, (CPSS_DXCH_PORT_PIZZA_CFG_PORT_SLICE_LIST_STC *)&pizzaLion2B0_UT_ConfBy2Slice }
        ,{ 4, (CPSS_DXCH_PORT_PIZZA_CFG_PORT_SLICE_LIST_STC *)&pizzaLion2B0_UT_ConfBy4Slice }
        ,{ 0, NULL                                                                     }
    }
};

const CPSS_DXCH_PORT_PIZZA_SLICENUM_TO_SLICE_LIST_STC lion2B0_ExtMode_UT =
{
    /* portNum */       12
    /* maxSliceNum */  ,12
    /* pizzaCfg */     ,{
         { 1, (CPSS_DXCH_PORT_PIZZA_CFG_PORT_SLICE_LIST_STC *)&pizzaLion2B0_UT_ConfBy1Slice_ExtMode }
        ,{ 2, (CPSS_DXCH_PORT_PIZZA_CFG_PORT_SLICE_LIST_STC *)&pizzaLion2B0_UT_ConfBy2Slice }
        ,{ 4, (CPSS_DXCH_PORT_PIZZA_CFG_PORT_SLICE_LIST_STC *)&pizzaLion2B0_UT_ConfBy4Slice }
        ,{ 0, NULL                                                                     }
    }
};


const CPSS_DXCH_PIZZA_PROFILE_STC  pizzaProfileLion2B0Clock_UT_12Ports =
{
    /* portNum        */  12
    /* maxSliceNum    */ ,12
    /* txQDef         */ ,{
    /* TXQ_repetition */     10
    /*    TxQDef      */    ,{
    /*       CPU TYPE */         CPSS_DXCH_CPUPORT_TYPE_EXTERNAL_E
    /*       CPU Port */        ,15
    /*       Slices   */        ,{CPSS_PA_INVALID_SLICE, 0, 0, 0, 0 }
    /*                */     }
    /*                */  }
    ,(CPSS_DXCH_PORT_PIZZA_SLICENUM_TO_SLICE_LIST_STC *)&lion2B0_RegMode_UT
    ,(CPSS_DXCH_PORT_PIZZA_SLICENUM_TO_SLICE_LIST_STC *)&lion2B0_ExtMode_UT
    ,(CPSS_DXCH_PORT_PRIORITY_MATR_STC *)&portPriorityLion2_12ports_UT
};


/* BC2-ALDRIN util to on HIGH-speed ports configuration */
/* called from some tests */

#define MIN_HIGH_SPEED_CNS CPSS_PORT_SPEED_40000_E

GT_VOID utilCpssDxChPortPizzaArbiterHighSpeedPortCheck
(
    IN  GT_U8                devNum,
    IN  CPSS_PORT_SPEED_ENT  minHighSpeed
)
{
    GT_STATUS            rc;
    GT_U32               highPortsNum;
    GT_BOOL              isEnabled;
    GT_PORT_NUM          txqPortNum;
    GT_PHYSICAL_PORT_NUM phyPortNum;
    GT_PORT_NUM          txqHighPortArr[
        PRV_CPSS_DXCH_BOBCAT2_B0_PORT_TXQ_DQ_HIGH_SPEED_PORT_NUM_CNS];
    GT_U32               portIndex;
    CPSS_PORT_SPEED_ENT  portSpeed;
    GT_U32               portSpeedMb;
    GT_U32               minHighSpeedMb;
    GT_BOOL              txqPortHighSpeed;
    GT_U32               checkedFastPorts;
    GT_U32               checkedSlowPorts;

    if ((! PRV_CPSS_SIP_5_10_CHECK_MAC(devNum)) || PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        /* Applicable to BC2_B0, Caelum, Aldrin       */
        /* relevant to BC2_A0, but not supported here */
        return;
    }

    minHighSpeedMb = prvCpssCommonPortSpeedEnumToMbPerSecConvert(minHighSpeed);
    UTF_VERIFY_NOT_EQUAL1_STRING_MAC(
        0, minHighSpeedMb, "not supported minHighSpeed %d\n", minHighSpeed);

    highPortsNum = 0;
    for (portIndex = 0;
          (portIndex < PRV_CPSS_DXCH_BOBCAT2_B0_PORT_TXQ_DQ_HIGH_SPEED_PORT_NUM_CNS); portIndex++)
    {
        rc = prvCpssDxChPortTxQUnitDQHighSpeedPortGet(
            devNum, portIndex, &isEnabled, &txqPortNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "prvCpssDxChPortTxQUnitDQHighSpeedPortGetfailed  portIndex: %d\n", portIndex);
        if (isEnabled == GT_FALSE) continue;
        txqHighPortArr[highPortsNum] = txqPortNum;
        highPortsNum ++;
    }

    checkedFastPorts = 0;
    checkedSlowPorts = 0;
    for (phyPortNum = 0; (phyPortNum < PRV_CPSS_PP_MAC(devNum)->numOfPorts); phyPortNum++)
    {
        rc = prvCpssDxChPortPhysicalPortMapCheckAndConvert(
            devNum, phyPortNum, PRV_CPSS_DXCH_PORT_TYPE_TXQ_E, &txqPortNum);
        if (rc != GT_OK) continue; /* no port mapping */
        rc = cpssDxChPortSpeedGet(devNum, phyPortNum, &portSpeed);
        if (rc != GT_OK) continue; /* no port speed, may be remote */

        portSpeedMb = prvCpssCommonPortSpeedEnumToMbPerSecConvert(portSpeed);
        if (portSpeedMb == 0) continue; /* cannot convert speed to MB/Sec */

        txqPortHighSpeed = GT_FALSE;
        for (portIndex = 0; (portIndex < highPortsNum); portIndex++)
        {
            if (txqHighPortArr[portIndex] == txqPortNum)
            {
                txqPortHighSpeed = GT_TRUE;
                break;
            }
        }
        if (txqPortHighSpeed == GT_FALSE)
        {
            checkedSlowPorts ++;
        }
        else
        {
            checkedFastPorts ++;
        }
        if ((txqPortHighSpeed != GT_FALSE) && (portSpeedMb < minHighSpeedMb))
        {
            UTF_VERIFY_EQUAL3_STRING_MAC(
                GT_OK, rc, "phyPort %d txqPort %s portSpeedMb: %d slow but configured as High Speed\n",
                phyPortNum, txqPortNum, portSpeedMb);
        }
        if ((txqPortHighSpeed == GT_FALSE) && (portSpeedMb >= minHighSpeedMb))
        {
            UTF_VERIFY_EQUAL3_STRING_MAC(
                GT_OK, rc, "phyPort %d txqPort %s portSpeedMb: %d fast but not configured as High Speed\n",
                phyPortNum, txqPortNum, portSpeedMb);
        }
    }
    PRV_UTF_LOG0_MAC("\n======= utilCpssDxChPortPizzaArbiterHighSpeedPortCheck:=======\n");
    PRV_UTF_LOG1_MAC("found %d high speed ports\n", highPortsNum);
    PRV_UTF_LOG2_MAC("checkedSlowPorts %d, checkedFastPorts %d\n", checkedSlowPorts, checkedFastPorts);
}

static GT_STATUS pizzaArbiterUTStorageGet
(
    OUT PRV_CPSS_DXCH_PROFILE_SPEED_CONV_TBL_STORAGE_STC  ** storagePtrPtr
)
{
    extern PRV_CPSS_DXCH_PROFILE_SPEED_CONV_TBL_STORAGE_STC g_usedProfileSpeedConvTableStorage;
    if (storagePtrPtr == NULL)
    {
        return GT_BAD_PTR;
    }
    * storagePtrPtr = &g_usedProfileSpeedConvTableStorage;
    return GT_OK;
}

/*-------------------------------------------------------------------
GT_STATUS cpssDxChPortPizzaArbiterIfUserTableSet
(
IN  GT_U8 devNum,
IN  GT_U32 portGroupBmp,
IN  CPSS_DXCH_PIZZA_PROFILE_STC * pPizzaProfilePtr,
IN  CPSS_DXCH_PORT_PIZZA_PORTGROUP_SPEED_TO_SLICENUM_STC * portGroupPortSpeed2SliceNumPtr
)
-------------------------------------------------------------------*/
GT_VOID prvCpssDxChPortPizzaArbiterIfUserTableSetUT(GT_VOID)
{
    /*
    ITERATE_DEVICES_PHY_CPU_PORTS
    1.1.1.  verify bad core Bitmap zero returns error
    Expected: GT_OK.
    1.1.2.  verify port bitMap is correct, but two pointers bad
    Expected: GT_BAD_PTR.
    1.1.3. set separately correct tables on core 0, than get tables
    verify that tables are correct
    1.1.4. clear profiles, get system profile, compare with user
    verify that them are different

    */

    GT_STATUS   rc     = GT_OK;
    GT_U8       dev;
    GT_U32      portGroupBmp;
    PRV_CPSS_DXCH_PROFILE_SPEED_CONV_TBL_STORAGE_STC * usedProfileSpeedConvTableStoragePtr;
    CPSS_DXCH_PIZZA_PROFILE_STC                          * profilePtr = NULL;
    CPSS_DXCH_PORT_PIZZA_PORTGROUP_SPEED_TO_SLICENUM_STC * speed2SliceConvTablePtr = NULL;


    /* Prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_ALL_FAMILY_E  & ~(UTF_LION2_E)) ; /* UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E); */

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        rc = pizzaArbiterUTStorageGet(/*OUT*/&usedProfileSpeedConvTableStoragePtr);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        /* before each test clear storage */
        rc = userProfileSpeedConvTableStorageInit(usedProfileSpeedConvTableStoragePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        /*----------------------------------------------------*/
        /* verify bad core Bitmap zero returns error          */
        /*----------------------------------------------------*/
        portGroupBmp = 0;
        rc = cpssDxChPortPizzaArbiterIfUserTableSet(dev,portGroupBmp,
            (CPSS_DXCH_PIZZA_PROFILE_STC *)&pizzaProfileLion2B0Clock_UT_12Ports,
            (CPSS_DXCH_PORT_PIZZA_PORTGROUP_SPEED_TO_SLICENUM_STC *)&portGroupPortSpeed2SliceNum_UT);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        rc = cpssDxChPortPizzaArbiterIfUserTableGet(dev,portGroupBmp,
            /*OUT*/&profilePtr,
            /*OUT*/&speed2SliceConvTablePtr);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        /* ---------------------------------------------*/
        /* Port bitMap is correct, but two pointers bad */
        /* I cann't set or get nothing                  */
        /* ---------------------------------------------*/
        portGroupBmp = BIT_0;
        rc = cpssDxChPortPizzaArbiterIfUserTableSet(dev,portGroupBmp,
            (CPSS_DXCH_PIZZA_PROFILE_STC *)NULL,
            (CPSS_DXCH_PORT_PIZZA_PORTGROUP_SPEED_TO_SLICENUM_STC *)NULL);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        rc = cpssDxChPortPizzaArbiterIfUserTableGet(dev,portGroupBmp,
            (CPSS_DXCH_PIZZA_PROFILE_STC **)NULL,
            (CPSS_DXCH_PORT_PIZZA_PORTGROUP_SPEED_TO_SLICENUM_STC **)NULL);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        /*-------------------------------------------------------*/
        /* set separately profile and speed conversion table for */
        /* core 0 only                                           */
        /*-------------------------------------------------------*/
        rc = cpssDxChPortPizzaArbiterIfUserTableSet(dev,portGroupBmp,
            (CPSS_DXCH_PIZZA_PROFILE_STC *)&pizzaProfileLion2B0Clock_UT_12Ports,
            (CPSS_DXCH_PORT_PIZZA_PORTGROUP_SPEED_TO_SLICENUM_STC *)NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        rc = cpssDxChPortPizzaArbiterIfUserTableSet(dev,portGroupBmp,
            (CPSS_DXCH_PIZZA_PROFILE_STC *) NULL,
            (CPSS_DXCH_PORT_PIZZA_PORTGROUP_SPEED_TO_SLICENUM_STC *)&portGroupPortSpeed2SliceNum_UT);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);
        /*-------------------------*/
        /* get profile from core 0 */
        /*-------------------------*/
        rc = cpssDxChPortPizzaArbiterIfUserTableGet(dev,portGroupBmp,
            /*OUT*/&profilePtr,
            /*OUT*/&speed2SliceConvTablePtr
            );
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        /*--------------------------*/
        /* compare !!!              */
        /*--------------------------*/

        rc = portGroupSpeed2SliceNumListCmp((CPSS_DXCH_PORT_PIZZA_PORTGROUP_SPEED_TO_SLICENUM_STC *)&portGroupPortSpeed2SliceNum_UT,
            speed2SliceConvTablePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        rc = pizzaProfileCompare((CPSS_DXCH_PIZZA_PROFILE_STC *)&pizzaProfileLion2B0Clock_UT_12Ports, profilePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);
        /* -------------------------------*/
        /* before each test clear storage */
        /* -------------------------------*/
        rc = userProfileSpeedConvTableStorageInit(usedProfileSpeedConvTableStoragePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);


        rc = cpssDxChPortPizzaArbiterIfUserTableGet(dev,portGroupBmp,
            /*OUT*/&profilePtr,
            /*OUT*/&speed2SliceConvTablePtr
            );
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        /*------------------------------------------------------------------------------*/
        /*     compare system profile with UT profile , shall be different !!!          */
        /*------------------------------------------------------------------------------*/

        rc = portGroupSpeed2SliceNumListCmp((CPSS_DXCH_PORT_PIZZA_PORTGROUP_SPEED_TO_SLICENUM_STC *)&portGroupPortSpeed2SliceNum_UT,
            speed2SliceConvTablePtr);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        rc = pizzaProfileCompare((CPSS_DXCH_PIZZA_PROFILE_STC *)&pizzaProfileLion2B0Clock_UT_12Ports, profilePtr);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        rc = pizzaArbiterUTStorageGet(/*OUT*/&usedProfileSpeedConvTableStoragePtr);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        rc = userProfileSpeedConvTableStorageInit(usedProfileSpeedConvTableStoragePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);
    }
}



typedef enum PRV_UT_SPEED_ENTE
{
    UT_10G  = CPSS_PORT_SPEED_10000_E    /* 3 */
    ,UT_20G  = CPSS_PORT_SPEED_20000_E    /* 8 */
    ,UT_40G  = CPSS_PORT_SPEED_40000_E    /* 9 */
    ,UT_100G = CPSS_PORT_SPEED_100G_E      /* 13 */
}PRV_UT_SPEED_ENT;

typedef struct PizzaTestCaseUT_STCT
{
    GT_U32                   portGroupIdx;
    GT_PHYSICAL_PORT_NUM     portNum;
    PRV_UT_SPEED_ENT         speed;
    GT_BOOL                  isExtModeEnable;
    GT_U32                   isOk; /* 0 == not equal , 1 == equal  GT_OK*/
    GT_U32                   portState[12];
    int                      sliceState[16];
}PizzaTestCaseUT_STC;



const PizzaTestCaseUT_STC systemTestSeq_16_Slices_480MHz[] =
{
     /*----------------------------------------------------------------------------------------------------------------------------------------------------------*/
     /*                                                         port state                                          slice state                                  */
     /*----------------------------------------------------------------------------------------------------------------------------------------------------------*/
     /* PG, port,   speed,        EM, isOk,    0   1   2   3   4   5   6   7   8   9  10  11       0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15 */
     {   0,    0, UT_100G,  GT_FALSE,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  9,  2, 10,  7, -1,  1, -1,  5, -1,  3, 11,  6, -1  } }
    ,{   0,    1, UT_100G,  GT_FALSE,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  9,  2, 10,  7, -1,  1, -1,  5, -1,  3, 11,  6, -1  } }
    ,{   0,    2, UT_100G,  GT_FALSE,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  9,  2, 10,  7, -1,  1, -1,  5, -1,  3, 11,  6, -1  } }
    ,{   0,    3, UT_100G,  GT_FALSE,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  9,  2, 10,  7, -1,  1, -1,  5, -1,  3, 11,  6, -1  } }
    /*------------- Prohibit combinations */
    ,{   0,    1,  UT_40G,  GT_FALSE,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  9,  2, 10,  7, -1,  1, -1,  5, -1,  3, 11,  6, -1  } }
    ,{   0,    2,  UT_40G,  GT_FALSE,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  9,  2, 10,  7, -1,  1, -1,  5, -1,  3, 11,  6, -1  } }
    ,{   0,    3,  UT_40G,  GT_FALSE,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  9,  2, 10,  7, -1,  1, -1,  5, -1,  3, 11,  6, -1  } }
    ,{   0,    5,  UT_40G,  GT_FALSE,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  9,  2, 10,  7, -1,  1, -1,  5, -1,  3, 11,  6, -1  } }
    ,{   0,    6,  UT_40G,  GT_FALSE,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  9,  2, 10,  7, -1,  1, -1,  5, -1,  3, 11,  6, -1  } }
    ,{   0,    7,  UT_40G,  GT_FALSE,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  9,  2, 10,  7, -1,  1, -1,  5, -1,  3, 11,  6, -1  } }
    /*------------- Prohibit combinations */
    ,{   0,    1,  UT_20G,  GT_FALSE,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  9,  2, 10,  7, -1,  1, -1,  5, -1,  3, 11,  6, -1  } }
    ,{   0,    3,  UT_20G,  GT_FALSE,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  9,  2, 10,  7, -1,  1, -1,  5, -1,  3, 11,  6, -1  } }
    ,{   0,    5,  UT_20G,  GT_FALSE,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  9,  2, 10,  7, -1,  1, -1,  5, -1,  3, 11,  6, -1  } }
    ,{   0,    7,  UT_20G,  GT_FALSE,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  9,  2, 10,  7, -1,  1, -1,  5, -1,  3, 11,  6, -1  } }
    /*-------------*/
    /*-----------------------------------------------------------------------------------------------------------------------------------------------------------*/
    /*  PG, port,   speed,        EM, isOk,    0   1   2   3   4   5   6   7   8   9  10  11       0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15 */
    /*-----------------------------------------------------------------------------------------------------------------------------------------------------------*/
    ,{   0,    0,  UT_20G,  GT_FALSE,    1,  { 2,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  9,  2, 10,  7, -1,  0, -1,  5, -1,  3, 11,  6, -1  } }
    ,{   0,    2,  UT_20G,  GT_FALSE,    1,  { 2,  0,  2,  0,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  9,  2, 10,  7, -1,  0, -1,  5, -1,  2, 11,  6, -1  } }
    ,{   0,    4,  UT_20G,  GT_FALSE,    1,  { 2,  0,  2,  0,  2,  0,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  9,  2, 10,  7, -1,  0, -1,  4, -1,  2, 11,  6, -1  } }
    ,{   0,    6,  UT_20G,  GT_FALSE,    1,  { 2,  0,  2,  0,  2,  0,  2,  0,  1,  1,  1,  1 }, {  0,  8,  4,  9,  2, 10,  6, -1,  0, -1,  4, -1,  2, 11,  6, -1  } }
    ,{   0,    8,  UT_20G,  GT_FALSE,    1,  { 2,  0,  2,  0,  2,  0,  2,  0,  2,  1,  1,  1 }, {  0,  8,  4,  9,  2, 10,  6, -1,  0,  8,  4, -1,  2, 11,  6, -1  } }
    ,{   0,    9,  UT_20G,  GT_FALSE,    1,  { 2,  0,  2,  0,  2,  0,  2,  0,  2,  2,  1,  1 }, {  0,  8,  4,  9,  2, 10,  6, -1,  0,  8,  4,  9,  2, 11,  6, -1  } }
    ,{   0,   10,  UT_20G,  GT_FALSE,    1,  { 2,  0,  2,  0,  2,  0,  2,  0,  2,  2,  2,  0 }, {  0,  8,  4,  9,  2, 10,  6, -1,  0,  8,  4,  9,  2, 10,  6, -1  } }
    ,{   0,   11,  UT_20G,  GT_FALSE,    1,  { 2,  0,  2,  0,  2,  0,  2,  0,  2,  2,  2,  2 }, {  0,  8,  4,  9,  2, 10,  6, 11,  0,  8,  4,  9,  2, 10,  6, 11  } }
    /*-------------*/
    ,{   0,    0,  UT_40G,  GT_FALSE,    1,  { 4,  0,  0,  0,  2,  0,  2,  0,  2,  2,  2,  2 }, {  0,  8,  4,  9,  0, 10,  6, 11,  0,  8,  4,  9,  0, 10,  6, 11  } }
    ,{   0,    4,  UT_40G,  GT_FALSE,    1,  { 4,  0,  0,  0,  4,  0,  0,  0,  2,  2,  2,  2 }, {  0,  8,  4,  9,  0, 10,  4, 11,  0,  8,  4,  9,  0, 10,  4, 11  } }
    ,{   0,    8,  UT_40G,  GT_FALSE,    1,  { 4,  0,  0,  0,  4,  0,  0,  0,  4,  2,  0,  2 }, {  0,  8,  4,  9,  0,  8,  4, 11,  0,  8,  4,  9,  0,  8,  4, 11  } }
    ,{   0,    9,  UT_40G,  GT_FALSE,    1,  { 4,  0,  0,  0,  4,  0,  0,  0,  4,  4,  0,  0 }, {  0,  8,  4,  9,  0,  8,  4,  9,  0,  8,  4,  9,  0,  8,  4,  9  } }
    /*------------- Cann't set 20G when more prior port is set up to 40G */
    ,{   0,    2,  UT_20G,  GT_FALSE,    0,  { 4,  0,  0,  0,  4,  0,  0,  0,  4,  4,  0,  0 }, {  0,  8,  4,  9,  0,  8,  4,  9,  0,  8,  4,  9,  0,  8,  4,  9  } }
    ,{   0,    6,  UT_20G,  GT_FALSE,    0,  { 4,  0,  0,  0,  4,  0,  0,  0,  4,  4,  0,  0 }, {  0,  8,  4,  9,  0,  8,  4,  9,  0,  8,  4,  9,  0,  8,  4,  9  } }
    ,{   0,   10,  UT_20G,  GT_FALSE,    0,  { 4,  0,  0,  0,  4,  0,  0,  0,  4,  4,  0,  0 }, {  0,  8,  4,  9,  0,  8,  4,  9,  0,  8,  4,  9,  0,  8,  4,  9  } }
    ,{   0,   11,  UT_20G,  GT_FALSE,    0,  { 4,  0,  0,  0,  4,  0,  0,  0,  4,  4,  0,  0 }, {  0,  8,  4,  9,  0,  8,  4,  9,  0,  8,  4,  9,  0,  8,  4,  9  } }
    /*-------------------------------------------------------------------*/
    /*              restore default configuration                        */
    /*-------------------------------------------------------------------*/
    /*-----------------------------------------------------------------------------------------------------------------------------------------------------------*/
    /*  PG, port,   speed,        EM, isOk,    0   1   2   3   4   5   6   7   8   9  10  11       0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15 */
    /*-----------------------------------------------------------------------------------------------------------------------------------------------------------*/
    ,{   0,    0,  UT_10G,  GT_FALSE,    1,  { 1,  0,  0,  0,  4,  0,  0,  0,  4,  4,  0,  0 }, {  0,  8,  4,  9, -1,  8,  4,  9, -1,  8,  4,  9, -1,  8,  4,  9  } }
    ,{   0,    4,  UT_10G,  GT_FALSE,    1,  { 1,  0,  0,  0,  1,  0,  0,  0,  4,  4,  0,  0 }, {  0,  8,  4,  9, -1,  8, -1,  9, -1,  8, -1,  9, -1,  8, -1,  9  } }
    ,{   0,    8,  UT_10G,  GT_FALSE,    1,  { 1,  0,  0,  0,  1,  0,  0,  0,  1,  4,  0,  0 }, {  0,  8,  4,  9, -1, -1, -1,  9, -1, -1, -1,  9, -1, -1, -1,  9  } }
    ,{   0,    9,  UT_10G,  GT_FALSE,    1,  { 1,  0,  0,  0,  1,  0,  0,  0,  1,  1,  0,  0 }, {  0,  8,  4,  9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    ,{   0,    1,  UT_10G,  GT_FALSE,    1,  { 1,  1,  0,  0,  1,  0,  0,  0,  1,  1,  0,  0 }, {  0,  8,  4,  9, -1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1, -1  } }
    ,{   0,    2,  UT_10G,  GT_FALSE,    1,  { 1,  1,  1,  0,  1,  0,  0,  0,  1,  1,  0,  0 }, {  0,  8,  4,  9,  2, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1, -1  } }
    ,{   0,    3,  UT_10G,  GT_FALSE,    1,  { 1,  1,  1,  1,  1,  0,  0,  0,  1,  1,  0,  0 }, {  0,  8,  4,  9,  2, -1, -1, -1,  1, -1, -1, -1,  3, -1, -1, -1  } }
    ,{   0,    5,  UT_10G,  GT_FALSE,    1,  { 1,  1,  1,  1,  1,  1,  0,  0,  1,  1,  0,  0 }, {  0,  8,  4,  9,  2, -1, -1, -1,  1, -1,  5, -1,  3, -1, -1, -1  } }
    ,{   0,    6,  UT_10G,  GT_FALSE,    1,  { 1,  1,  1,  1,  1,  1,  1,  0,  1,  1,  0,  0 }, {  0,  8,  4,  9,  2, -1, -1, -1,  1, -1,  5, -1,  3, -1,  6, -1  } }
    ,{   0,    7,  UT_10G,  GT_FALSE,    1,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0 }, {  0,  8,  4,  9,  2, -1,  7, -1,  1, -1,  5, -1,  3, -1,  6, -1  } }
    ,{   0,   10,  UT_10G,  GT_FALSE,    1,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0 }, {  0,  8,  4,  9,  2, 10,  7, -1,  1, -1,  5, -1,  3, -1,  6, -1  } }
    ,{   0,   11,  UT_10G,  GT_FALSE,    1,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  9,  2, 10,  7, -1,  1, -1,  5, -1,  3, 11,  6, -1  } }
    /*----------------------------------*/
    /* now same for extended mode       */
    /*----------------------------------*/
    /*-----------------------------------------------------------------------------------------------------------------------------------------------------------*/
    /*  PG, port,   speed,        EM, isOk,    0   1   2   3   4   5   6   7   8   9  10  11       0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15 */
    /*-----------------------------------------------------------------------------------------------------------------------------------------------------------*/
    ,{   0,    0, UT_100G,   GT_TRUE,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  9,  2, 10,  7, -1,  1, -1,  5, -1,  3, 11,  6, -1  } }
    ,{   0,    1, UT_100G,   GT_TRUE,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  9,  2, 10,  7, -1,  1, -1,  5, -1,  3, 11,  6, -1  } }
    ,{   0,    2, UT_100G,   GT_TRUE,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  9,  2, 10,  7, -1,  1, -1,  5, -1,  3, 11,  6, -1  } }
    ,{   0,    3, UT_100G,   GT_TRUE,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  9,  2, 10,  7, -1,  1, -1,  5, -1,  3, 11,  6, -1  } }
    /*------------- Prohibit combinations */
    ,{   0,    1,  UT_40G,   GT_TRUE,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  9,  2, 10,  7, -1,  1, -1,  5, -1,  3, 11,  6, -1  } }
    ,{   0,    2,  UT_40G,   GT_TRUE,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  9,  2, 10,  7, -1,  1, -1,  5, -1,  3, 11,  6, -1  } }
    ,{   0,    3,  UT_40G,   GT_TRUE,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  9,  2, 10,  7, -1,  1, -1,  5, -1,  3, 11,  6, -1  } }
    ,{   0,    5,  UT_40G,   GT_TRUE,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  9,  2, 10,  7, -1,  1, -1,  5, -1,  3, 11,  6, -1  } }
    ,{   0,    6,  UT_40G,   GT_TRUE,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  9,  2, 10,  7, -1,  1, -1,  5, -1,  3, 11,  6, -1  } }
    ,{   0,    7,  UT_40G,   GT_TRUE,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  9,  2, 10,  7, -1,  1, -1,  5, -1,  3, 11,  6, -1  } }
    /*------------- Prohibit combinations */
    ,{   0,    1,  UT_20G,   GT_TRUE,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  9,  2, 10,  7, -1,  1, -1,  5, -1,  3, 11,  6, -1  } }
    ,{   0,    3,  UT_20G,   GT_TRUE,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  9,  2, 10,  7, -1,  1, -1,  5, -1,  3, 11,  6, -1  } }
    ,{   0,    5,  UT_20G,   GT_TRUE,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  9,  2, 10,  7, -1,  1, -1,  5, -1,  3, 11,  6, -1  } }
    ,{   0,    7,  UT_20G,   GT_TRUE,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  9,  2, 10,  7, -1,  1, -1,  5, -1,  3, 11,  6, -1  } }
    /*-------------*/
    /*-----------------------------------------------------------------------------------------------------------------------------------------------------------*/
    /*  PG, port,   speed,        EM, isOk,    0   1   2   3   4   5   6   7   8   9  10  11       0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15 */
    /*-----------------------------------------------------------------------------------------------------------------------------------------------------------*/
    ,{   0,    0,  UT_20G,   GT_TRUE,    1,  { 2,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  9,  2, 10,  7, -1,  0, -1,  5, -1,  3, 11,  6, -1  } }
    ,{   0,    2,  UT_20G,   GT_TRUE,    1,  { 2,  0,  2,  0,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  9,  2, 10,  7, -1,  0, -1,  5, -1,  2, 11,  6, -1  } }
    ,{   0,    4,  UT_20G,   GT_TRUE,    1,  { 2,  0,  2,  0,  2,  0,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  9,  2, 10,  7, -1,  0, -1,  4, -1,  2, 11,  6, -1  } }
    ,{   0,    6,  UT_20G,   GT_TRUE,    1,  { 2,  0,  2,  0,  2,  0,  2,  0,  1,  1,  1,  1 }, {  0,  8,  4,  9,  2, 10,  6, -1,  0, -1,  4, -1,  2, 11,  6, -1  } }
    ,{   0,    8,  UT_20G,   GT_TRUE,    1,  { 2,  0,  2,  0,  2,  0,  2,  0,  2,  1,  1,  1 }, {  0,  8,  4,  9,  2, 10,  6, -1,  0,  8,  4, -1,  2, 11,  6, -1  } }
    ,{   0,    9,  UT_20G,   GT_TRUE,    1,  { 2,  0,  2,  0,  2,  0,  2,  0,  2,  2,  1,  1 }, {  0,  8,  4,  9,  2, 10,  6, -1,  0,  8,  4,  9,  2, 11,  6, -1  } }
    ,{   0,   10,  UT_20G,   GT_TRUE,    1,  { 2,  0,  2,  0,  2,  0,  2,  0,  2,  2,  2,  0 }, {  0,  8,  4,  9,  2, 10,  6, -1,  0,  8,  4,  9,  2, 10,  6, -1  } }
    ,{   0,   11,  UT_20G,   GT_TRUE,    1,  { 2,  0,  2,  0,  2,  0,  2,  0,  2,  2,  2,  2 }, {  0,  8,  4,  9,  2, 10,  6, 11,  0,  8,  4,  9,  2, 10,  6, 11  } }
    /*-------------*/
    ,{   0,    0,  UT_40G,   GT_TRUE,    1,  { 4,  0,  0,  0,  2,  0,  2,  0,  2,  2,  2,  2 }, {  0,  8,  4,  9,  0, 10,  6, 11,  0,  8,  4,  9,  0, 10,  6, 11  } }
    ,{   0,    4,  UT_40G,   GT_TRUE,    1,  { 4,  0,  0,  0,  4,  0,  0,  0,  2,  2,  2,  2 }, {  0,  8,  4,  9,  0, 10,  4, 11,  0,  8,  4,  9,  0, 10,  4, 11  } }
    ,{   0,    8,  UT_40G,   GT_TRUE,    1,  { 4,  0,  0,  0,  4,  0,  0,  0,  4,  2,  0,  2 }, {  0,  8,  4,  9,  0,  8,  4, 11,  0,  8,  4,  9,  0,  8,  4, 11  } }
    ,{   0,    9,  UT_40G,   GT_TRUE,    1,  { 4,  0,  0,  0,  4,  0,  0,  0,  4,  4,  0,  0 }, {  0,  8,  4,  9,  0,  8,  4,  9,  0,  8,  4,  9,  0,  8,  4,  9  } }
    /*------------- Cann't set 20G when more prior port is set up to 40G */
    ,{   0,    2,  UT_20G,   GT_TRUE,    0,  { 4,  0,  0,  0,  4,  0,  0,  0,  4,  4,  0,  0 }, {  0,  8,  4,  9,  0,  8,  4,  9,  0,  8,  4,  9,  0,  8,  4,  9  } }
    ,{   0,    6,  UT_20G,   GT_TRUE,    0,  { 4,  0,  0,  0,  4,  0,  0,  0,  4,  4,  0,  0 }, {  0,  8,  4,  9,  0,  8,  4,  9,  0,  8,  4,  9,  0,  8,  4,  9  } }
    ,{   0,   10,  UT_20G,   GT_TRUE,    0,  { 4,  0,  0,  0,  4,  0,  0,  0,  4,  4,  0,  0 }, {  0,  8,  4,  9,  0,  8,  4,  9,  0,  8,  4,  9,  0,  8,  4,  9  } }
    ,{   0,   11,  UT_20G,   GT_TRUE,    0,  { 4,  0,  0,  0,  4,  0,  0,  0,  4,  4,  0,  0 }, {  0,  8,  4,  9,  0,  8,  4,  9,  0,  8,  4,  9,  0,  8,  4,  9  } }
    /*-------------------------------------------------------------------*/
    /*              restore default configuration                        */
    /*-------------------------------------------------------------------*/
    /*-----------------------------------------------------------------------------------------------------------------------------------------------------------*/
    /*  PG, port,   speed,        EM, isOk,    0   1   2   3   4   5   6   7   8   9  10  11       0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15 */
    /*-----------------------------------------------------------------------------------------------------------------------------------------------------------*/
    ,{   0,    0,  UT_10G,   GT_TRUE,    1,  { 1,  0,  0,  0,  4,  0,  0,  0,  4,  4,  0,  0 }, {  0,  8,  4,  9, -1,  8,  4,  9, -1,  8,  4,  9, -1,  8,  4,  9  } }
    ,{   0,    4,  UT_10G,   GT_TRUE,    1,  { 1,  0,  0,  0,  1,  0,  0,  0,  4,  4,  0,  0 }, {  0,  8,  4,  9, -1,  8, -1,  9, -1,  8, -1,  9, -1,  8, -1,  9  } }
    ,{   0,    8,  UT_10G,   GT_TRUE,    1,  { 1,  0,  0,  0,  1,  0,  0,  0,  1,  4,  0,  0 }, {  0,  8,  4,  9, -1, -1, -1,  9, -1, -1, -1,  9, -1, -1, -1,  9  } }
    ,{   0,    9,  UT_10G,   GT_TRUE,    1,  { 1,  0,  0,  0,  1,  0,  0,  0,  1,  1,  0,  0 }, {  0,  8,  4,  9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    ,{   0,    1,  UT_10G,   GT_TRUE,    1,  { 1,  1,  0,  0,  1,  0,  0,  0,  1,  1,  0,  0 }, {  0,  8,  4,  9, -1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1, -1  } }
    ,{   0,    2,  UT_10G,   GT_TRUE,    1,  { 1,  1,  1,  0,  1,  0,  0,  0,  1,  1,  0,  0 }, {  0,  8,  4,  9,  2, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1, -1  } }
    ,{   0,    3,  UT_10G,   GT_TRUE,    1,  { 1,  1,  1,  1,  1,  0,  0,  0,  1,  1,  0,  0 }, {  0,  8,  4,  9,  2, -1, -1, -1,  1, -1, -1, -1,  3, -1, -1, -1  } }
    ,{   0,    5,  UT_10G,   GT_TRUE,    1,  { 1,  1,  1,  1,  1,  1,  0,  0,  1,  1,  0,  0 }, {  0,  8,  4,  9,  2, -1, -1, -1,  1, -1,  5, -1,  3, -1, -1, -1  } }
    ,{   0,    6,  UT_10G,   GT_TRUE,    1,  { 1,  1,  1,  1,  1,  1,  1,  0,  1,  1,  0,  0 }, {  0,  8,  4,  9,  2, -1, -1, -1,  1, -1,  5, -1,  3, -1,  6, -1  } }
    ,{   0,    7,  UT_10G,   GT_TRUE,    1,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0 }, {  0,  8,  4,  9,  2, -1,  7, -1,  1, -1,  5, -1,  3, -1,  6, -1  } }
    ,{   0,   10,  UT_10G,   GT_TRUE,    1,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0 }, {  0,  8,  4,  9,  2, 10,  7, -1,  1, -1,  5, -1,  3, -1,  6, -1  } }
    ,{   0,   11,  UT_10G,   GT_TRUE,    1,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  9,  2, 10,  7, 11,  1, -1,  5, -1,  3, -1,  6, -1  } }
    ,{   0,   11,  UT_10G,  GT_FALSE,    1,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  9,  2, 10,  7, -1,  1, -1,  5, -1,  3, 11,  6, -1  } }
};


const PizzaTestCaseUT_STC systemTestSeq_12_Slices_360MHz[] =
{
    /*----------------------------------------------------------------------------------------------------------------------------------------------------------*/
    /*                                                         port state                                          slice state                                  */
    /*----------------------------------------------------------------------------------------------------------------------------------------------------------*/
    /* PG, port,   speed,        EM, isOk,    0   1   2   3   4   5   6   7   8   9  10  11       0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15 */
     {   0,    0, UT_100G,  GT_FALSE,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  1,  9,  5,  2, 10,  6,  3, 11,  7, -1, -1, -1, -1  } }
    ,{   0,    1, UT_100G,  GT_FALSE,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  1,  9,  5,  2, 10,  6,  3, 11,  7, -1, -1, -1, -1  } }
    ,{   0,    2, UT_100G,  GT_FALSE,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  1,  9,  5,  2, 10,  6,  3, 11,  7, -1, -1, -1, -1  } }
    ,{   0,    3, UT_100G,  GT_FALSE,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  1,  9,  5,  2, 10,  6,  3, 11,  7, -1, -1, -1, -1  } }
    /*------------- Prohibit combinations */
    ,{   0,    1,  UT_40G,  GT_FALSE,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  1,  9,  5,  2, 10,  6,  3, 11,  7, -1, -1, -1, -1  } }
    ,{   0,    2,  UT_40G,  GT_FALSE,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  1,  9,  5,  2, 10,  6,  3, 11,  7, -1, -1, -1, -1  } }
    ,{   0,    3,  UT_40G,  GT_FALSE,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  1,  9,  5,  2, 10,  6,  3, 11,  7, -1, -1, -1, -1  } }
    ,{   0,    5,  UT_40G,  GT_FALSE,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  1,  9,  5,  2, 10,  6,  3, 11,  7, -1, -1, -1, -1  } }
    ,{   0,    6,  UT_40G,  GT_FALSE,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  1,  9,  5,  2, 10,  6,  3, 11,  7, -1, -1, -1, -1  } }
    ,{   0,    7,  UT_40G,  GT_FALSE,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  1,  9,  5,  2, 10,  6,  3, 11,  7, -1, -1, -1, -1  } }
    ,{   0,    9,  UT_40G,  GT_FALSE,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  1,  9,  5,  2, 10,  6,  3, 11,  7, -1, -1, -1, -1  } }
    ,{   0,   10,  UT_40G,  GT_FALSE,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  1,  9,  5,  2, 10,  6,  3, 11,  7, -1, -1, -1, -1  } }
    ,{   0,   11,  UT_40G,  GT_FALSE,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  1,  9,  5,  2, 10,  6,  3, 11,  7, -1, -1, -1, -1  } }
    /*------------- Prohibit combinations */
    ,{   0,    1,  UT_20G,  GT_FALSE,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  1,  9,  5,  2, 10,  6,  3, 11,  7, -1, -1, -1, -1  } }
    ,{   0,    3,  UT_20G,  GT_FALSE,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  1,  9,  5,  2, 10,  6,  3, 11,  7, -1, -1, -1, -1  } }
    ,{   0,    5,  UT_20G,  GT_FALSE,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  1,  9,  5,  2, 10,  6,  3, 11,  7, -1, -1, -1, -1  } }
    ,{   0,    7,  UT_20G,  GT_FALSE,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  1,  9,  5,  2, 10,  6,  3, 11,  7, -1, -1, -1, -1  } }
    ,{   0,    9,  UT_20G,  GT_FALSE,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  1,  9,  5,  2, 10,  6,  3, 11,  7, -1, -1, -1, -1  } }
    ,{   0,   11,  UT_20G,  GT_FALSE,    0,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  1,  9,  5,  2, 10,  6,  3, 11,  7, -1, -1, -1, -1  } }
    /*-----------------------------------------------------------------------------------------------------------------------------------------------------------*/
    /*  PG, port,   speed,        EM, isOk,    0   1   2   3   4   5   6   7   8   9  10  11       0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15 */
    /*-----------------------------------------------------------------------------------------------------------------------------------------------------------*/
    ,{   0,    0,  UT_20G,  GT_FALSE,    1,  { 2,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  0,  9,  5,  2, 10,  6,  3, 11,  7, -1, -1, -1, -1  } }
    ,{   0,    2,  UT_20G,  GT_FALSE,    1,  { 2,  0,  2,  0,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  0,  9,  5,  2, 10,  6,  2, 11,  7, -1, -1, -1, -1  } }
    ,{   0,    4,  UT_20G,  GT_FALSE,    1,  { 2,  0,  2,  0,  2,  0,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  0,  9,  4,  2, 10,  6,  2, 11,  7, -1, -1, -1, -1  } }
    ,{   0,    6,  UT_20G,  GT_FALSE,    1,  { 2,  0,  2,  0,  2,  0,  2,  0,  1,  1,  1,  1 }, {  0,  8,  4,  0,  9,  4,  2, 10,  6,  2, 11,  6, -1, -1, -1, -1  } }
    ,{   0,    8,  UT_20G,  GT_FALSE,    1,  { 2,  0,  2,  0,  2,  0,  2,  0,  2,  0,  1,  1 }, {  0,  8,  4,  0,  8,  4,  2, 10,  6,  2, 11,  6, -1, -1, -1, -1  } }
    ,{   0,   10,  UT_20G,  GT_FALSE,    1,  { 2,  0,  2,  0,  2,  0,  2,  0,  2,  0,  2,  0 }, {  0,  8,  4,  0,  8,  4,  2, 10,  6,  2, 10,  6, -1, -1, -1, -1  } }
    /*-------------*/
    ,{   0,    0,  UT_40G,  GT_FALSE,    1,  { 4,  0,  0,  0,  2,  0,  2,  0,  2,  0,  2,  0 }, {  0,  8,  4,  0,  8,  4,  0, 10,  6,  0, 10,  6, -1, -1, -1, -1  } }
    ,{   0,    4,  UT_40G,  GT_FALSE,    1,  { 4,  0,  0,  0,  4,  0,  0,  0,  2,  0,  2,  0 }, {  0,  8,  4,  0,  8,  4,  0, 10,  4,  0, 10,  4, -1, -1, -1, -1  } }
    ,{   0,    8,  UT_40G,  GT_FALSE,    1,  { 4,  0,  0,  0,  4,  0,  0,  0,  4,  0,  0,  0 }, {  0,  8,  4,  0,  8,  4,  0,  8,  4,  0,  8,  4, -1, -1, -1, -1  } }
    /*------------- Cann't set 20G when more prior port is set up to 40G */
    ,{   0,    2,  UT_20G,  GT_FALSE,    0,  { 4,  0,  0,  0,  4,  0,  0,  0,  4,  0,  0,  0 }, {  0,  8,  4,  0,  8,  4,  0,  8,  4,  0,  8,  4, -1, -1, -1, -1  } }
    ,{   0,    6,  UT_20G,  GT_FALSE,    0,  { 4,  0,  0,  0,  4,  0,  0,  0,  4,  0,  0,  0 }, {  0,  8,  4,  0,  8,  4,  0,  8,  4,  0,  8,  4, -1, -1, -1, -1  } }
    ,{   0,   10,  UT_20G,  GT_FALSE,    0,  { 4,  0,  0,  0,  4,  0,  0,  0,  4,  0,  0,  0 }, {  0,  8,  4,  0,  8,  4,  0,  8,  4,  0,  8,  4, -1, -1, -1, -1  } }
    /*------------- restore default configuration -----------------------*/
    /*-----------------------------------------------------------------------------------------------------------------------------------------------------------*/
    /*  PG, port,   speed,        EM, isOk,    0   1   2   3   4   5   6   7   8   9  10  11       0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15 */
    /*-----------------------------------------------------------------------------------------------------------------------------------------------------------*/
    ,{   0,    0,  UT_10G,  GT_FALSE,    1,  { 1,  0,  0,  0,  4,  0,  0,  0,  4,  0,  0,  0 }, {  0,  8,  4, -1,  8,  4, -1,  8,  4, -1,  8,  4, -1, -1, -1, -1  } }
    ,{   0,    4,  UT_10G,  GT_FALSE,    1,  { 1,  0,  0,  0,  1,  0,  0,  0,  4,  0,  0,  0 }, {  0,  8,  4, -1,  8, -1, -1,  8, -1, -1,  8, -1, -1, -1, -1, -1  } }
    ,{   0,    8,  UT_10G,  GT_FALSE,    1,  { 1,  0,  0,  0,  1,  0,  0,  0,  1,  0,  0,  0 }, {  0,  8,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    ,{   0,    1,  UT_10G,  GT_FALSE,    1,  { 1,  1,  0,  0,  1,  0,  0,  0,  1,  0,  0,  0 }, {  0,  8,  4,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    ,{   0,    2,  UT_10G,  GT_FALSE,    1,  { 1,  1,  1,  0,  1,  0,  0,  0,  1,  0,  0,  0 }, {  0,  8,  4,  1, -1, -1,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1  } }
    ,{   0,    3,  UT_10G,  GT_FALSE,    1,  { 1,  1,  1,  1,  1,  0,  0,  0,  1,  0,  0,  0 }, {  0,  8,  4,  1, -1, -1,  2, -1, -1,  3, -1, -1, -1, -1, -1, -1  } }
    ,{   0,    5,  UT_10G,  GT_FALSE,    1,  { 1,  1,  1,  1,  1,  1,  0,  0,  1,  0,  0,  0 }, {  0,  8,  4,  1, -1,  5,  2, -1, -1,  3, -1, -1, -1, -1, -1, -1  } }
    ,{   0,    6,  UT_10G,  GT_FALSE,    1,  { 1,  1,  1,  1,  1,  1,  1,  0,  1,  0,  0,  0 }, {  0,  8,  4,  1, -1,  5,  2, -1,  6,  3, -1, -1, -1, -1, -1, -1  } }
    ,{   0,    7,  UT_10G,  GT_FALSE,    1,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0 }, {  0,  8,  4,  1, -1,  5,  2, -1,  6,  3, -1,  7, -1, -1, -1, -1  } }
    ,{   0,    9,  UT_10G,  GT_FALSE,    1,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0 }, {  0,  8,  4,  1,  9,  5,  2, -1,  6,  3, -1,  7, -1, -1, -1, -1  } }
    ,{   0,   10,  UT_10G,  GT_FALSE,    1,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0 }, {  0,  8,  4,  1,  9,  5,  2, 10,  6,  3, -1,  7, -1, -1, -1, -1  } }
    ,{   0,   11,  UT_10G,  GT_FALSE,    1,  { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, {  0,  8,  4,  1,  9,  5,  2, 10,  6,  3, 11,  7, -1, -1, -1, -1  } }
};



#ifdef PA_UT_PRINT

static void utPizzaSim_TestSystem_Test_PrintHdr(void)
{
    cpssOsPrintf("\n--------------------------------------------------------------------------------------------------------------------------");
    cpssOsPrintf("\n                                         port State                     :            slice State                          ");
    cpssOsPrintf("\ntest : dev pg port ext speed : rc :  0  1  2  3  4  5  6  7  8  9 10 11 :  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 ");
    cpssOsPrintf("\n--------------------------------------------------------------------------------------------------------------------------");
}

static void utPizzaSim_TestSystem_PrintCase(GT_U8 dev, int testN, PizzaTestCaseUT_STC * testCasePtr)
{
    typedef struct SpeedNumStr_STCT
    {
        PRV_UT_SPEED_ENT   speed;
        char             * speedStrPtr;
    }SpeedNumStr_STC;

    static SpeedNumStr_STC speed2StrArr[] =
    {
        { UT_10G  ,  "10G"  }
        ,{ UT_20G  ,  "20G"  }
        ,{ UT_40G  ,  "40G"  }
        ,{ UT_100G ,  "100G" }
        ,{ 0       ,  "N/A"  }
    };

    static char * portModeStrArr[] =
    {
        "Reg",
        "Ext"
    };

    GT_U32 i;
    GT_U32 size;
    char * speedStr;

    size = sizeof(speed2StrArr)/sizeof(speed2StrArr[0]);

    speed2StrArr[size-1].speed = testCasePtr->speed;
    for (i = 0 ; speed2StrArr[i].speed != testCasePtr->speed; i++);
    speedStr = speed2StrArr[i].speedStrPtr;

    cpssOsPrintf("\n%4d :",testN);
    cpssOsPrintf(" %3d %2d %4d %3s %5s :",(GT_U32)dev,
        (int)(testCasePtr->portGroupIdx),
        (int)testCasePtr->portNum,
        portModeStrArr[testCasePtr->isExtModeEnable],
        speedStr);
    cpssOsPrintf(" %2d :",testCasePtr->isOk);
}

static void utPizzaSim_TestSystem_PrintPortsState(PizzaTestCaseUT_STC * testCasePtr, GT_PHYSICAL_PORT_NUM portNum)
{
    GT_PHYSICAL_PORT_NUM portIdx;

    for (portIdx  = 0 ; portIdx < portNum ; portIdx ++)
    {
        if (testCasePtr->portState[portIdx] > 0)
        {
            cpssOsPrintf(" %2d",(int)testCasePtr->portState[portIdx]);
        }
        else
        {
            cpssOsPrintf("  -");
        }
    }
    cpssOsPrintf(" :");
}

static void utPizzaSim_TestSystem_PrintSlicesState(PizzaTestCaseUT_STC * testCasePtr, GT_U32 sliceNum)
{
    GT_U32 sliceIdx;
    for (sliceIdx = 0 ; sliceIdx < sliceNum ; sliceIdx++)
    {
        if(testCasePtr->sliceState[sliceIdx] >= 0)
        {
            cpssOsPrintf(" %2d",testCasePtr->sliceState[sliceIdx]);
        }
        else
        {
            cpssOsPrintf(" --");
        }
    }
}

#endif

static void utPizzaSim_TestSystem_BuildPizzaDevInfo(PRV_CPSS_DXCH_PORT_PIZZA_DEV_INFO_STC * pPizzaDevInfo, GT_U8 dev, GT_U32 portGroupIdx)
{
    GT_U32 pg;
    PRV_CPSS_GEN_PP_CONFIG_STC* pDev; /* pointer to device to be processed*/

    pDev = PRV_CPSS_PP_MAC(dev);

    pPizzaDevInfo->devFamilyType        = pDev->devFamily;
    pPizzaDevInfo->devRevision          = pDev->revision;
    pPizzaDevInfo->devCoreClock         = pDev->coreClock;
    pPizzaDevInfo->totalPortGroups      = pDev->portGroupsInfo.numOfPortGroups;
    pPizzaDevInfo->sliceModFun.portGroupSlicesInitFunPtr = (PRV_CPSS_DXCH_PORT_PIZZA_DEV_PORTGROUP_SLICES_INIT_FUNC)NULL; /* already init !!!! */
    pPizzaDevInfo->sliceModFun.sliceOccupyFunPtr   = prvLion2PortPizzaArbiterConfigureSliceOccupy;
    pPizzaDevInfo->sliceModFun.sliceReleaseFunPtr  = prvLion2PortPizzaArbiterConfigureSliceRelease;
    pPizzaDevInfo->sliceModFun.sliceGetStateFunPtr = prvLion2PortPizzaArbiterConfigureSliceGetState;
    for (pg = 0; pg < sizeof(pPizzaDevInfo->portGroupPizzaProfile)/sizeof(pPizzaDevInfo->portGroupPizzaProfile[0]); pg++)
    {
        pPizzaDevInfo->portGroupPizzaProfile[pg] = (CPSS_DXCH_PIZZA_PROFILE_STC*)NULL;
    }

    prvCpssDxChPizzaArbiterSystemProfileGet(/*IN*/pDev->devFamily,pDev->revision,pDev->coreClock,
        &pPizzaDevInfo->portGroupPizzaProfile[portGroupIdx]);

    ActivePortGroupListInit(&pPizzaDevInfo->activePortGroupList);
    ActivePortGroupListAppend(&pPizzaDevInfo->activePortGroupList,portGroupIdx);
}



static GT_STATUS utPizzaSim_TestSystem_TestPortsState
(
    GT_U8 dev,
    PizzaTestCaseUT_STC * testCasePtr
)
{
    GT_STATUS             rc;
    GT_PHYSICAL_PORT_NUM  portNum;
    GT_U32                sliceNum;
    PRV_CPSS_DXCH_PORT_PIZZA_DEV_INFO_STC      pizzaDevInfo;

    utPizzaSim_TestSystem_BuildPizzaDevInfo(&pizzaDevInfo, dev, testCasePtr->portGroupIdx);

    for (portNum = 0 ; portNum < 12 ; portNum++)
    {
        sliceNum = CPSS_PA_INVALID_SLICE;
        rc = prvCpssDxChPizzaArbiterPortGetState(dev, testCasePtr->portGroupIdx, portNum, &pizzaDevInfo,/*OUT*/&sliceNum);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (testCasePtr->portState[portNum] != sliceNum)
        {
            #ifdef PA_UT_PRINT
                utPizzaSim_TestSystem_PrintPortsState(testCasePtr, portNum+1);
            #endif
            return GT_FAIL;
        }
    }
    return GT_OK;
}

static GT_STATUS utPizzaArbiter_TestSingleSliceState
(
    GT_32 sliceStateByTest,
    GT_BOOL isOccupied,
    GT_PHYSICAL_PORT_NUM portNum
)
{
    if (sliceStateByTest == -1  && isOccupied == 1)
    {
        return GT_FAIL;
    }
    if (sliceStateByTest != -1  && isOccupied == 0)
    {
        return GT_FAIL;
    }
    if (isOccupied == 1)
    {
        if ((GT_PHYSICAL_PORT_NUM)sliceStateByTest != portNum)
        {
            return GT_FAIL;
        }
    }
    return GT_OK;
}

static GT_STATUS utPizzaSim_TestSystem_FindSliceInSliceList
(
    IN CPSS_DXCH_PIZZA_SLICE_ID * listPtr,
    IN CPSS_DXCH_PIZZA_SLICE_ID   sliceId,
    OUT GT_BOOL * isFoundPtr
)
{
    GT_U32 idx;

    if (NULL == isFoundPtr)
    {
        return GT_BAD_PTR;
    }
    if (NULL == listPtr)
    {
        *isFoundPtr = GT_FALSE;
        return GT_OK;
    }
    for (idx = 0 ; listPtr[idx] != CPSS_PA_INVALID_SLICE; idx++)
    {
        if (listPtr[idx] == sliceId)
        {
            *isFoundPtr = GT_TRUE;
            return GT_OK;
        }
    }
    *isFoundPtr = GT_FALSE;
    return GT_OK;
}



static GT_U32 utPizzaSim_TestSystem_TxDMA_ConvertSliceIdx
(
    GT_U32 sliceIdx,
    GT_U32 coreClock
)
{
    GT_U32 newSliceIdx;

    newSliceIdx = sliceIdx;
    if (coreClock == 360)
    {
        newSliceIdx = (sliceIdx/3) * 4 + (sliceIdx%3);
    }
    return newSliceIdx;
}


static GT_STATUS utPizzaSim_TestSystem_TestSlicesState_ByUnit_CheckNumberConfiguredSlices
(
    IN GT_U8 dev,
    IN CPSS_DXCH_PIZZA_PROFILE_STC * profilePtr,
    IN CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC  * pizzaDeviceStatePtr
)
{
    GT_U32                unitId;
    GT_U32                TxDMANumOfSlicesShallBeConfigured;
    GT_U32                txQNumOfSlicesShallBeConfigured;

    PRV_CPSS_GEN_PP_CONFIG_STC* pDev; /* pointer to device to be processed*/

    CPSS_DXCH_UNIT32_SLICES_PIZZA_ARBITER_STATE_STC * unitPtrArr[6] = {0};

    if (NULL == profilePtr || NULL == pizzaDeviceStatePtr)
    {
        return GT_BAD_PARAM;
    }

    pDev = PRV_CPSS_PP_MAC(dev);

    if (pDev->revision == 0)
    {
        return GT_NOT_SUPPORTED;
    }


    unitId = 0;
    unitPtrArr[unitId++] = &pizzaDeviceStatePtr->devState.lion2.rxDMA;
    unitPtrArr[unitId++] = &pizzaDeviceStatePtr->devState.lion2.rxDMA_CTU;
    unitPtrArr[unitId++] = &pizzaDeviceStatePtr->devState.lion2.txDMA_CTU;
    unitPtrArr[unitId++] = &pizzaDeviceStatePtr->devState.lion2.BM;
    unitPtrArr[unitId++] = (CPSS_DXCH_UNIT32_SLICES_PIZZA_ARBITER_STATE_STC *)NULL;

    /*---------------------------------------------*/
    /* check all slices above declared are disabled */
    /*---------------------------------------------*/
    for (unitId = 0 ; unitPtrArr[unitId] != (CPSS_DXCH_UNIT32_SLICES_PIZZA_ARBITER_STATE_STC *)NULL ; unitId++)
    {
        if (profilePtr->maxSliceNum != unitPtrArr[unitId]->totalConfiguredSlices)
        {
            return GT_FAIL;
        }
    }


    TxDMANumOfSlicesShallBeConfigured = utPizzaSim_TestSystem_TxDMA_ConvertSliceIdx(profilePtr->maxSliceNum,
        pDev->coreClock);
    if (TxDMANumOfSlicesShallBeConfigured != pizzaDeviceStatePtr->devState.lion2.txDMA.totalConfiguredSlices)
    {
        return GT_FAIL;
    }

    txQNumOfSlicesShallBeConfigured = pizzaDeviceStatePtr->devState.lion2.TxQ.slicesNumInGop;
    if (pizzaDeviceStatePtr->devState.lion2.TxQ.slicesNumInGop != pizzaDeviceStatePtr->devState.lion2.TxQ.totalConfiguredSlices) /* is CPU declared */
    {
        switch (profilePtr->txQDef.cpuPortDef.type)
        {
        case CPSS_DXCH_CPUPORT_TYPE_UNDEFINED_E:
            txQNumOfSlicesShallBeConfigured = pizzaDeviceStatePtr->devState.lion2.TxQ.slicesNumInGop;
            break;
        case CPSS_DXCH_CPUPORT_TYPE_FIXED_E:
            txQNumOfSlicesShallBeConfigured = pizzaDeviceStatePtr->devState.lion2.TxQ.slicesNumInGop * profilePtr->txQDef.pizzaRepetitionNum;
            break;
        case CPSS_DXCH_CPUPORT_TYPE_EXTERNAL_E:
            txQNumOfSlicesShallBeConfigured = pizzaDeviceStatePtr->devState.lion2.TxQ.slicesNumInGop * profilePtr->txQDef.pizzaRepetitionNum + 2;
            break;
        default:
            {
                return GT_FAIL;
            }
        }
    }
    if (pizzaDeviceStatePtr->devState.lion2.TxQ.totalConfiguredSlices != txQNumOfSlicesShallBeConfigured)
    {
        return GT_FAIL;
    }
    return GT_OK;
}


static GT_STATUS utPizzaSim_TestSystem_TestSlicesState_ByUnit_AllSlicesAboveAreDisable
(
    IN GT_U8 dev,
    IN CPSS_DXCH_PIZZA_PROFILE_STC * profilePtr,
    IN CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC  * pizzaDeviceStatePtr
)
{
    GT_U32                slice;
    GT_BOOL               isOccupied;
    GT_U32                unitId;

    CPSS_DXCH_UNIT32_SLICES_PIZZA_ARBITER_STATE_STC * unitPtrArr[6] = {0};
    PRV_CPSS_GEN_PP_CONFIG_STC* pDev; /* pointer to device to be processed*/

    pDev = PRV_CPSS_PP_MAC(dev);

    if (pDev->revision == 0)
    {
        return GT_NOT_SUPPORTED;
    }

    if (NULL == profilePtr || NULL == pizzaDeviceStatePtr)
    {
        return GT_BAD_PARAM;
    }

    unitPtrArr[0] = &pizzaDeviceStatePtr->devState.lion2.rxDMA;
    unitPtrArr[1] = &pizzaDeviceStatePtr->devState.lion2.rxDMA_CTU;
    unitPtrArr[2] = &pizzaDeviceStatePtr->devState.lion2.txDMA;
    unitPtrArr[3] = &pizzaDeviceStatePtr->devState.lion2.txDMA_CTU;
    unitPtrArr[4] = &pizzaDeviceStatePtr->devState.lion2.BM;
    unitPtrArr[5] = (CPSS_DXCH_UNIT32_SLICES_PIZZA_ARBITER_STATE_STC *)NULL;



    /*---------------------------------------------*/
    /* check all slices above declared are disabled */
    /*---------------------------------------------*/
    for (unitId = 0 ; unitPtrArr[unitId] != (CPSS_DXCH_UNIT32_SLICES_PIZZA_ARBITER_STATE_STC *)NULL ; unitId++)
    {
        for (slice = unitPtrArr[unitId]->totalConfiguredSlices ; slice < UNIT_TOTAL_SLICES  ; slice++)
        {
            isOccupied = unitPtrArr[unitId]->slice_enable[slice];
            if (GT_FALSE != isOccupied)
            {
                return GT_FAIL;
            }
        }
    }

    for (slice = pizzaDeviceStatePtr->devState.lion2.TxQ.totalConfiguredSlices ; slice < TxQ_TOTAL_AVAIBLE_SLICES_CNS  ; slice++)
    {
        isOccupied = pizzaDeviceStatePtr->devState.lion2.TxQ.slice_enable[slice];
        if (GT_FALSE != isOccupied)
        {
            return GT_FAIL;
        }
    }
    return GT_OK;
}

static GT_STATUS utPizzaSim_TestSystem_TestSlicesState_ByUnit_CheckAllDisabledSlices
(
    IN GT_U8 dev,
    IN CPSS_DXCH_PIZZA_PROFILE_STC * profilePtr,
    IN CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC  * pizzaDeviceStatePtr
)
{
    GT_STATUS             rc;
    GT_BOOL               isOccupied;
    GT_PHYSICAL_PORT_NUM  portNum;
    GT_U32                txDMASliceId;
    GT_U32                txQSliceId;

    PRV_CPSS_GEN_PP_CONFIG_STC* pDev; /* pointer to device to be processed*/

    pDev = PRV_CPSS_PP_MAC(dev);

    if (pDev->revision == 0)
    {
        return GT_NOT_SUPPORTED;
    }

    if (NULL == profilePtr || NULL == pizzaDeviceStatePtr)
    {
        return GT_BAD_PARAM;
    }

    rc = utPizzaSim_TestSystem_TestSlicesState_ByUnit_AllSlicesAboveAreDisable(dev,profilePtr,pizzaDeviceStatePtr);
    if (GT_OK != rc)
    {
        return rc;
    }
    /*-------------------------------------------------------------------------------------*/
    /* TxDMA : 360MHz :  check whether each 4-th slice is assigned to port 9 and disabled  */
    /*-------------------------------------------------------------------------------------*/
    if (pDev->coreClock == 360)
    {
        for (txDMASliceId = 3 ; txDMASliceId < pizzaDeviceStatePtr->devState.lion2.txDMA.totalConfiguredSlices; txDMASliceId+=4)
        {
            isOccupied =  pizzaDeviceStatePtr->devState.lion2.txDMA.slice_enable[txDMASliceId];
            portNum    =  pizzaDeviceStatePtr->devState.lion2.txDMA.slice_occupied_by[txDMASliceId];
            if (isOccupied == GT_TRUE)
            {
                return GT_FAIL;
            }
            if (portNum != 9)
            {
                return GT_FAIL;
            }
        }
    }

    /*------------------------------------------------------------------------------*/
    /* TxQ : check whether each second slice is disabled                                  */
    /*------------------------------------------------------------------------------*/
    for (txQSliceId = 1 ; txQSliceId < pizzaDeviceStatePtr->devState.lion2.TxQ.totalConfiguredSlices; txQSliceId += 2 )
    {
        isOccupied =  pizzaDeviceStatePtr->devState.lion2.TxQ.slice_enable[txQSliceId];
        portNum    =  pizzaDeviceStatePtr->devState.lion2.TxQ.slice_occupied_by[txQSliceId];
        if (isOccupied == GT_TRUE)  /* slice shall be disabled */
        {
            return GT_FAIL;
        }
    }
    return GT_OK;
}


static GT_STATUS utPizzaSim_TestSystem_TestSlicesState_TxQBuildCPUSliceList
(
    IN GT_U8 dev,
    IN  CPSS_DXCH_PIZZA_PROFILE_STC * profilePtr,
    IN  CPSS_DXCH_UNIT_TXQ_SLICES_PIZZA_ARBITER_STATE_STC * txQUnitPtr,
    OUT CPSS_DXCH_PIZZA_SLICE_ID ** cpuPortSlicesListPtrPtr
)
{
    static CPSS_DXCH_PIZZA_SLICE_ID        cpuPortSlices[2];  /* The last shall be CPSS_PA_INVALIDE_SLICE */
    CPSS_DXCH_PIZZA_SLICE_ID      * cpuPortSlicesListPtr;


    PRV_CPSS_GEN_PP_CONFIG_STC* pDev; /* pointer to device to be processed*/

    pDev = PRV_CPSS_PP_MAC(dev);

    if (pDev->revision == 0)
    {
        return GT_NOT_SUPPORTED;
    }

    if (NULL == profilePtr || NULL == txQUnitPtr || NULL == cpuPortSlicesListPtrPtr)
    {
        return GT_BAD_PTR;
    }

    cpuPortSlicesListPtr = (CPSS_DXCH_PIZZA_SLICE_ID *)NULL;
    if (txQUnitPtr->slicesNumInGop != txQUnitPtr->totalConfiguredSlices)
    {
        switch (profilePtr->txQDef.cpuPortDef.type)
        {
        case CPSS_DXCH_CPUPORT_TYPE_FIXED_E:
            cpuPortSlicesListPtr = &(profilePtr->txQDef.cpuPortDef.cpuPortSlices[0]);
            break;
        case CPSS_DXCH_CPUPORT_TYPE_EXTERNAL_E:
            cpuPortSlices[0] = (CPSS_DXCH_PIZZA_SLICE_ID)(profilePtr->txQDef.pizzaRepetitionNum * 2*profilePtr->maxSliceNum);
            cpuPortSlices[1] = CPSS_PA_INVALID_SLICE;
            cpuPortSlicesListPtr = &cpuPortSlices[0];
            break;
        default:
            cpuPortSlicesListPtr = (CPSS_DXCH_PIZZA_SLICE_ID *)NULL;
            break;
        }
    }
    *cpuPortSlicesListPtrPtr = cpuPortSlicesListPtr;
    return GT_OK;
}


/*------------------------------------------------------------------*/
/* check whether all slices of CPU are really occupied by CPU port  */
/*------------------------------------------------------------------*/
static GT_STATUS utPizzaSim_TestSystem_TestSlicesState_ByUnit_TxqCheckCpuSlices
(
    IN GT_U8 dev,
    IN CPSS_DXCH_PIZZA_PROFILE_STC * profilePtr,
    IN CPSS_DXCH_UNIT_TXQ_SLICES_PIZZA_ARBITER_STATE_STC * txQUnitPtr,
    IN CPSS_DXCH_PIZZA_SLICE_ID * cpuPortSlicesListPtr
)
{
    GT_STATUS             rc;
    GT_BOOL               isOccupied;
    GT_PHYSICAL_PORT_NUM  portNum;
    GT_U32                txQSliceId;
    GT_U32                idx;
    GT_BOOL               isFound;

    PRV_CPSS_GEN_PP_CONFIG_STC* pDev; /* pointer to device to be processed*/

    pDev = PRV_CPSS_PP_MAC(dev);

    if (pDev->revision == 0)
    {
        return GT_NOT_SUPPORTED;
    }

    /*------------------------------------------------------------------*/
    /* check whether all slices of CPU are really occupied by CPU port  */
    /*------------------------------------------------------------------*/
    if (NULL != cpuPortSlicesListPtr)
    {
        for (idx = 0 ; cpuPortSlicesListPtr[idx] != CPSS_PA_INVALID_SLICE; idx++)
        {

            txQSliceId = cpuPortSlicesListPtr[idx];
            isOccupied =  txQUnitPtr->slice_enable[txQSliceId];
            portNum    =  txQUnitPtr->slice_occupied_by[txQSliceId];
            rc = utPizzaArbiter_TestSingleSliceState(profilePtr->txQDef.cpuPortDef.cpuPortNum,isOccupied,portNum);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    /*------------------------------------------------------------------------------*/
    /* check whether all slices not belonging to CPU port are not occupied by  CPU  */
    /*------------------------------------------------------------------------------*/
    for (txQSliceId = 0 ; txQSliceId < txQUnitPtr->totalConfiguredSlices; txQSliceId+=2)
    {
        isOccupied =  txQUnitPtr->slice_enable[txQSliceId];
        portNum    =  txQUnitPtr->slice_occupied_by[txQSliceId];
        if (isOccupied == GT_TRUE)
        {
            rc = utPizzaSim_TestSystem_FindSliceInSliceList(cpuPortSlicesListPtr,(CPSS_DXCH_PIZZA_SLICE_ID)txQSliceId,/*OUT*/&isFound);
            if (GT_OK != rc)
            {
                return rc;
            }
            if (isFound == GT_FALSE) /* SLICE does not belong to CPU */
            {
                if (portNum == profilePtr->txQDef.cpuPortDef.cpuPortNum)
                {
                    return GT_FAIL;
                }
            }
            else /* SLICE shall belong to CPU */
            {
                if (portNum != profilePtr->txQDef.cpuPortDef.cpuPortNum)
                {
                    return GT_FAIL;
                }
            }
        }
    }
    return GT_OK;
}


static GT_STATUS utPizzaSim_TestSystem_TestSlicesState_ByUnit_CheckPortSlices
(
    IN GT_U8 dev,
    IN PizzaTestCaseUT_STC *         testCasePtr,
    IN CPSS_DXCH_PIZZA_PROFILE_STC * profilePtr,
    IN CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC  * pizzaDeviceStatePtr,
    IN CPSS_DXCH_PIZZA_SLICE_ID * cpuPortSlicesListPtr
)
{
    GT_STATUS             rc;
    GT_U32                slice;
    GT_BOOL               isOccupied;
    GT_PHYSICAL_PORT_NUM  portNum;
    GT_U32                unitId;
    GT_U32                txDMASliceId;
    GT_U32                txQSliceId;
    GT_U32                idx;
    GT_BOOL               isFound;
    GT_U32                txQpizzaRepetitionNum;

    CPSS_DXCH_UNIT32_SLICES_PIZZA_ARBITER_STATE_STC * unitPtrArr[6] = {0};
    CPSS_DXCH_UNIT_TXQ_SLICES_PIZZA_ARBITER_STATE_STC * txQUnitPtr;

    PRV_CPSS_GEN_PP_CONFIG_STC* pDev; /* pointer to device to be processed*/

    pDev = PRV_CPSS_PP_MAC(dev);

    if (pDev->revision == 0)
    {
        return GT_NOT_SUPPORTED;
    }

    unitId = 0;
    unitPtrArr[unitId++] = &pizzaDeviceStatePtr->devState.lion2.rxDMA;
    unitPtrArr[unitId++] = &pizzaDeviceStatePtr->devState.lion2.rxDMA_CTU;
    /* unitPtrArr[unitId++] = &pizzaDeviceStatePtr->txDMA; */
    unitPtrArr[unitId++] = &pizzaDeviceStatePtr->devState.lion2.txDMA_CTU;
    unitPtrArr[unitId++] = &pizzaDeviceStatePtr->devState.lion2.BM;
    unitPtrArr[unitId++] = (CPSS_DXCH_UNIT32_SLICES_PIZZA_ARBITER_STATE_STC *)NULL;

    txQUnitPtr = &pizzaDeviceStatePtr->devState.lion2.TxQ;


    for (slice = 0 ; slice < profilePtr->maxSliceNum; slice++)
    {
        /* check regular units */
        for (unitId = 0 ; unitPtrArr[unitId] != NULL ; unitId++)
        {
            isOccupied = unitPtrArr[unitId]->slice_enable[slice];
            portNum    = unitPtrArr[unitId]->slice_occupied_by[slice];

            rc = utPizzaArbiter_TestSingleSliceState(testCasePtr->sliceState[slice],isOccupied,portNum);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        /*--------------------*/
        /*  txDMA             */
        /*--------------------*/
        txDMASliceId = utPizzaSim_TestSystem_TxDMA_ConvertSliceIdx(slice,pDev->coreClock);
        isOccupied = pizzaDeviceStatePtr->devState.lion2.txDMA.slice_enable[txDMASliceId];
        portNum    = pizzaDeviceStatePtr->devState.lion2.txDMA.slice_occupied_by[txDMASliceId];

        rc = utPizzaArbiter_TestSingleSliceState(testCasePtr->sliceState[slice],isOccupied,portNum);
        if (rc != GT_OK)
        {
            return rc;
        }
        /*--------------------*/
        /*  TxQ               */
        /*--------------------*/
        txQpizzaRepetitionNum = profilePtr->txQDef.pizzaRepetitionNum;
        if (txQUnitPtr->slicesNumInGop == txQUnitPtr->totalConfiguredSlices)
        {
            txQpizzaRepetitionNum = 1;
        }
        txQSliceId = 2*slice;
        for (idx = 0; idx < txQpizzaRepetitionNum  ; idx++, txQSliceId+= txQUnitPtr->slicesNumInGop)
        {
            isOccupied =  txQUnitPtr->slice_enable[txQSliceId];
            portNum    =  txQUnitPtr->slice_occupied_by[txQSliceId];

            rc = utPizzaSim_TestSystem_FindSliceInSliceList(cpuPortSlicesListPtr,(CPSS_DXCH_PIZZA_SLICE_ID)txQSliceId,/*OUT*/&isFound);
            if (GT_OK != rc)
            {
                return rc;
            }
            if (isFound == GT_FALSE) /* SLICE does not belong to CPU */
            {
                rc = utPizzaArbiter_TestSingleSliceState(testCasePtr->sliceState[slice],isOccupied,portNum);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }
    return GT_OK;
}



static GT_STATUS utPizzaSim_TestSystem_TestSlicesState_ByUnit
(
    IN GT_U8 dev,
    IN GT_U32 testN,
    IN PizzaTestCaseUT_STC * testCasePtr
)
{
    GT_STATUS             rc;
    PRV_CPSS_DXCH_PORT_PIZZA_DEV_INFO_STC      pizzaDevInfo;
    CPSS_DXCH_PIZZA_SLICE_ID      * cpuPortSlicesListPtr;
    CPSS_DXCH_PIZZA_PROFILE_STC * profilePtr;
    static CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC  pizzaDeviceState;


    PRV_CPSS_GEN_PP_CONFIG_STC* pDev; /* pointer to device to be processed*/

    pDev = PRV_CPSS_PP_MAC(dev);

    if (pDev->revision == 0)
    {
        return GT_NOT_SUPPORTED;
    }

    if (NULL == testCasePtr)
    {
        return GT_BAD_PTR;
    }

    if (testN == 30)
    {
        testN = testN;
    }
    utPizzaSim_TestSystem_BuildPizzaDevInfo(/*OUT*/&pizzaDevInfo,dev, /*IN*/testCasePtr->portGroupIdx);

    profilePtr = pizzaDevInfo.portGroupPizzaProfile[testCasePtr->portGroupIdx];

    /* get device state */
    rc = cpssDxChPortPizzaArbiterDevStateGet(dev,testCasePtr->portGroupIdx, /*OUT*/&pizzaDeviceState);
    if (rc != GT_OK)
    {
        return rc;
    }
    /*----------------------------------------------*/
    /* check number of configuref slices            */
    /*----------------------------------------------*/
    rc = utPizzaSim_TestSystem_TestSlicesState_ByUnit_CheckNumberConfiguredSlices(dev,profilePtr,&pizzaDeviceState);
    if (rc != GT_OK)
    {
        return rc;
    }
    /*------------------------------------------------*/
    /* check all diabled slices                       */
    /*------------------------------------------------*/
    rc = utPizzaSim_TestSystem_TestSlicesState_ByUnit_CheckAllDisabledSlices(dev,profilePtr,&pizzaDeviceState);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*----------------------------------------*/
    /* build list txQ clices occupied by CPU  */
    /*----------------------------------------*/
    rc = utPizzaSim_TestSystem_TestSlicesState_TxQBuildCPUSliceList(dev,profilePtr,&pizzaDeviceState.devState.lion2.TxQ,
        /*OUT*/&cpuPortSlicesListPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*------------------------------------------------------------------*/
    /* check whether all slices of CPU are really occupied by CPU port  */
    /* and slices that not belong to CPU are not occupied by CPU        */
    /*------------------------------------------------------------------*/
    rc = utPizzaSim_TestSystem_TestSlicesState_ByUnit_TxqCheckCpuSlices(dev,profilePtr,&pizzaDeviceState.devState.lion2.TxQ,cpuPortSlicesListPtr);
    if (rc != GT_OK)
    {
        return rc;
    }


    /*--------------------------------*/
    /* CPU clices at TxQ are OK       */
    /* check slices at all units      */
    /*--------------------------------*/
    rc = utPizzaSim_TestSystem_TestSlicesState_ByUnit_CheckPortSlices(dev,testCasePtr,profilePtr,
        &pizzaDeviceState,
        cpuPortSlicesListPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}


static GT_STATUS utPizzaSim_TestSystem_TestSlicesState
(
    GT_U8 dev, PizzaTestCaseUT_STC * testCasePtr
)
{
    GT_STATUS             rc;
    GT_U32                slice;
    GT_BOOL               isOccupied;
    GT_PHYSICAL_PORT_NUM  portNum;
    PRV_CPSS_DXCH_PORT_PIZZA_DEV_INFO_STC      pizzaDevInfo;

    utPizzaSim_TestSystem_BuildPizzaDevInfo(&pizzaDevInfo,dev, testCasePtr->portGroupIdx);

    for (slice = 0 ; slice < sizeof(testCasePtr->sliceState)/sizeof(testCasePtr->sliceState[0]); slice++)
    {
        prvCpssDxChPizzaArbiterSliceGetState(/*IN*/dev,
            /*IN*/testCasePtr->portGroupIdx,
            /*IN*/slice,
            /*IN*/&pizzaDevInfo,
            /*OUT*/&isOccupied,
            /*OUT*/&portNum);

        rc = utPizzaArbiter_TestSingleSliceState(testCasePtr->sliceState[slice],isOccupied,portNum);
        if (rc != GT_OK)
        {
            #ifdef PA_UT_PRINT
                utPizzaSim_TestSystem_PrintSlicesState(testCasePtr, slice+1);
            #endif
            return rc;
        }
    }
    return GT_OK;
}

typedef GT_STATUS (*utTestSlicesState_ByUnitFunc)
    (
    IN GT_U8 dev,
    IN GT_U32 testN,
    IN PizzaTestCaseUT_STC * testCasePtr
    );


typedef struct UT_CLOCK_TESTSEQ_STCT
{
    GT_U32 clock;
    PizzaTestCaseUT_STC * testSeqPtr;
    GT_U32                testSeqLen;
    utTestSlicesState_ByUnitFunc testSlicesStateByUnitFunc;
}UT_CLOCK_TESTSEQ_STC;


static UT_CLOCK_TESTSEQ_STC utClock_TestSeqSuite[] =
{
    {  360
       ,(PizzaTestCaseUT_STC *)&systemTestSeq_12_Slices_360MHz[0]
       ,sizeof(systemTestSeq_12_Slices_360MHz)/sizeof(systemTestSeq_12_Slices_360MHz[0])
       ,utPizzaSim_TestSystem_TestSlicesState_ByUnit
    }
   ,{
        480
       ,(PizzaTestCaseUT_STC *)&systemTestSeq_16_Slices_480MHz[0]
       ,sizeof(systemTestSeq_16_Slices_480MHz)/sizeof(systemTestSeq_16_Slices_480MHz[0])
       ,utPizzaSim_TestSystem_TestSlicesState_ByUnit
    }
};

static GT_STATUS utPizzaSim_TestSystemSelectTestSeq
(
    IN  GT_U8 dev,
    OUT PizzaTestCaseUT_STC      ** testSeqPtrPtr,
    OUT GT_U32                   *  testSeqLenPtr,
    utTestSlicesState_ByUnitFunc  * testSlicesStateByUnitFuncPtr
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_PORT_PIZZA_DEV_INFO_STC      pizzaDevInfo;
    GT_U32 i;

    utPizzaSim_TestSystem_BuildPizzaDevInfo(&pizzaDevInfo, dev, 0);

    if (testSeqPtrPtr == NULL || testSeqLenPtr == NULL || NULL == testSlicesStateByUnitFuncPtr )
    {
        return GT_BAD_PTR;

    }
    for (i = 0 ; i < sizeof(utClock_TestSeqSuite)/sizeof(utClock_TestSeqSuite[0]); i++)
    {
        if (pizzaDevInfo.devCoreClock == utClock_TestSeqSuite[i].clock)
        {
            *testSeqPtrPtr = utClock_TestSeqSuite[i].testSeqPtr;
            *testSeqLenPtr = utClock_TestSeqSuite[i].testSeqLen;
            *testSlicesStateByUnitFuncPtr = utClock_TestSeqSuite[i].testSlicesStateByUnitFunc;
            return GT_OK;
        }
    }
    *testSeqPtrPtr = (PizzaTestCaseUT_STC *)NULL;
    *testSeqLenPtr = 0;
    *testSlicesStateByUnitFuncPtr = (utTestSlicesState_ByUnitFunc)NULL;
    rc = GT_NOT_SUPPORTED;
    return rc;
}

static GT_STATUS utPizzaSim_TestSystem
(
    GT_U8 dev,
    GT_U32 portGroupId
)
{
    GT_STATUS rc ;

    GT_U32                testCaseIdx;
    PizzaTestCaseUT_STC * testCaseSeqPtr;
    GT_U32                totalTests;
    GT_U32                testN;
    CPSS_PORTS_BMP_STC    initPortsBmp; /* bitmap of ports to init */
    GT_PHYSICAL_PORT_NUM  portNum;
    GT_PHYSICAL_PORT_NUM  glPortNum;
    utTestSlicesState_ByUnitFunc testSliceStateByUnitFuncPtr;


    PizzaTestCaseUT_STC * testCasePtr;
    PizzaTestCaseUT_STC   testCase;


    PRV_CPSS_GEN_PP_CONFIG_STC* pDev; /* pointer to device to be processed*/

    pDev = PRV_CPSS_PP_MAC(dev);

    if (pDev->revision == 0)
    {
        return GT_NOT_SUPPORTED;
    }
    /*----------------------------------*/
    /* Move the system to initial state */
    /*----------------------------------*/

    for (portNum = 0 ; portNum < 12 ; portNum+=4)
    {
        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&initPortsBmp);
        glPortNum = PRV_CPSS_LOCAL_PORT_TO_GLOBAL_PORT_CONVERT_MAC(dev,portGroupId,portNum);
        CPSS_PORTS_BMP_PORT_SET_MAC(&initPortsBmp,glPortNum);
        if (portNum == 9 || portNum == 11)
        {
            rc = cpssDxChPortExtendedModeEnableSet(dev, glPortNum, GT_FALSE);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        rc = cpssDxChPortModeSpeedSet(dev,
            &initPortsBmp,
            GT_TRUE,
            CPSS_PORT_INTERFACE_MODE_SR_LR_E,
            CPSS_PORT_SPEED_40000_E);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    utilCpssDxChPortPizzaArbiterHighSpeedPortCheck(
        dev, MIN_HIGH_SPEED_CNS);

    for (portNum = 0 ; portNum < 12 ; portNum++)
    {
        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&initPortsBmp);
        glPortNum = PRV_CPSS_LOCAL_PORT_TO_GLOBAL_PORT_CONVERT_MAC(dev,portGroupId,portNum);
        CPSS_PORTS_BMP_PORT_SET_MAC(&initPortsBmp,glPortNum);
        if (portNum == 9 || portNum == 11)
        {
            rc = cpssDxChPortExtendedModeEnableSet(dev, glPortNum, GT_FALSE);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        rc = cpssDxChPortModeSpeedSet(dev,
            &initPortsBmp,
            GT_TRUE,
            CPSS_PORT_INTERFACE_MODE_SR_LR_E,
            CPSS_PORT_SPEED_10000_E);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    utilCpssDxChPortPizzaArbiterHighSpeedPortCheck(
        dev, MIN_HIGH_SPEED_CNS);

    rc = utPizzaSim_TestSystemSelectTestSeq(dev, /*OUT*/&testCaseSeqPtr, &totalTests, &testSliceStateByUnitFuncPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    testN = 30;

    for (testCaseIdx = 0 ; testCaseIdx < totalTests; testCaseIdx++,testCaseSeqPtr++)
    {
        testCase = *testCaseSeqPtr;
        testCasePtr = &testCase;
        testCasePtr->portGroupIdx = portGroupId;

        #ifdef PA_UT_PRINT
            if(0 == testCaseIdx % 20)
            {
                utPizzaSim_TestSystem_Test_PrintHdr();
            }
            utPizzaSim_TestSystem_PrintCase(dev, testCaseIdx, testCasePtr);
        #endif

        if (testCaseIdx == testN)
        {
            testCaseIdx = testCaseIdx;
        }

        portNum = PRV_CPSS_LOCAL_PORT_TO_GLOBAL_PORT_CONVERT_MAC(dev, testCasePtr->portGroupIdx,testCasePtr->portNum);


        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&initPortsBmp);
        CPSS_PORTS_BMP_PORT_SET_MAC(&initPortsBmp,portNum);

        if (testCasePtr->portNum == 9 || testCasePtr->portNum == 11)
        {
            rc = cpssDxChPortExtendedModeEnableSet(dev, portNum, testCasePtr->isExtModeEnable);
            /* UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, rc, dev); */
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        rc = cpssDxChPortModeSpeedSet(dev,
                                        &initPortsBmp,
                                        GT_TRUE,
                                        CPSS_PORT_INTERFACE_MODE_SR_LR_E,
                                        (CPSS_PORT_SPEED_ENT)testCasePtr->speed);

        if (1 == testCasePtr->isOk)
        {
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        else
        {
            if (rc == GT_OK)
            {
                return GT_FAIL;
            }

        }

        rc = utPizzaSim_TestSystem_TestPortsState(dev,testCasePtr);
        if (rc != GT_OK)
        {
            return rc;
        }

        #ifdef PA_UT_PRINT
            utPizzaSim_TestSystem_PrintPortsState(testCasePtr, 12);
        #endif

        /* Test Slice */
        rc = utPizzaSim_TestSystem_TestSlicesState(dev,testCasePtr);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = (*testSliceStateByUnitFuncPtr)(dev,testCaseIdx,testCasePtr);
        if (rc != GT_OK)
        {
            return rc;
        }
        #ifdef PA_UT_PRINT
            utPizzaSim_TestSystem_PrintSlicesState(testCasePtr, 16);
        #endif
    }

    #ifdef PA_UT_PRINT
        cpssOsPrintf("\nHello. Pizza !\n");
    #endif
    return GT_OK;
}

static GT_VOID prvCpssDxChPortModeSpeedSetUT(GT_VOID)
{
    GT_STATUS   rc     = GT_OK;
    GT_U8       dev;
    PRV_CPSS_DXCH_PROFILE_SPEED_CONV_TBL_STORAGE_STC * usedProfileSpeedConvTableStoragePtr;
    PRV_CPSS_DXCH_ACTIVE_PORT_GROUP_LIST_STC  activePortGroupList;
    ActivePortGroupListIter_STC iter;
    GT_U32 portGroupId;
    PRV_CPSS_GEN_PP_CONFIG_STC* pDev; /* pointer to device to be processed*/

    /* Prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_ALL_FAMILY_E  & ~(UTF_LION2_E)) ; /* UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E); */


    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        pDev = PRV_CPSS_PP_MAC(dev);

        if (pDev->revision == 0)
        {
            continue;
        }

        rc = ActivePortGroupListBuildFromBmp(&activePortGroupList,
            PRV_CPSS_PP_MAC(dev)->portGroupsInfo.activePortGroupsBmp,
            PRV_CPSS_PP_MAC(dev)->portGroupsInfo.firstActivePortGroup,
            PRV_CPSS_PP_MAC(dev)->portGroupsInfo.lastActivePortGroup);

        rc = ActivePortGroupListIterInit(&iter,&activePortGroupList);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        ActivePortGroupListIterReset(&iter);

        for (;;)
        {
            rc = ActivePortGroupListIterGetCur(&iter,&portGroupId);
            if (portGroupId == PRV_CPSS_BAD_PORTGROUP_CNS)
            {
                break;
            }
            rc = utPizzaSim_TestSystem(dev,portGroupId);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

            rc = ActivePortGroupListIterGotNext(&iter);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);
        }


        rc = pizzaArbiterUTStorageGet(/*OUT*/&usedProfileSpeedConvTableStoragePtr);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        rc = userProfileSpeedConvTableStorageInit(usedProfileSpeedConvTableStoragePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);
    }
}


/* UTF_TEST_CASE_MAC(cpssDxChPortPizzaArbiterIfUserTableSet)  */
GT_VOID cpssDxChPortPizzaArbiterIfUserTableSetUT(GT_VOID)
{
    GT_U8       dev;
    /* Prepare iterator for go over all active devices */

#ifdef PA_UT_SEQ
    cpssOsPrintf("\ncpssDxChPortPizzaArbiterIfUserTableSetUT enter");
#endif

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_ALL_FAMILY_E  & ~(UTF_LION2_E)) ; /* UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E); */

    prvCpssDxChPortPizzaArbiterIfUserTableSetUT();
    prvCpssDxChPortModeSpeedSetUT();

#ifdef PA_UT_SEQ
    cpssOsPrintf("\ncpssDxChPortPizzaArbiterIfUserTableSetUT exit");
#endif

}

GT_VOID cpssDxChLion2PortModeSpeedSetUT(GT_VOID)
{
    GT_U8       dev;

#ifdef PA_UT_SEQ
    cpssOsPrintf("\ncpssDxChPortModeSpeedSetUT enter");
#endif

    /* Prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_ALL_FAMILY_E  & ~(UTF_LION2_E)) ; /* UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E); */

    prvCpssDxChPortModeSpeedSetUT();
#ifdef PA_UT_SEQ
    cpssOsPrintf("\ncpssDxChPortModeSpeedSetUT exit");
#endif
}


static GT_STATUS  pcsSlicesNumToExpectedCreditsValuesGet
(
   GT_U32     channelPcsSlice,
   GT_U32     channelIndex,
   GT_U32     *expectedEagleTxCreditsArr,
   GT_U32     *expectedEagleRxCreditsArr,
   GT_U32     *expectedRavenRxCreditsArr,
   GT_U32     *expectedRavenTxCreditsArr
)
{

    GT_U32 channelExpectedBandwidth, PcsSlicesNumFor25g;
    GT_U32 xfast, xslow,xoff,xon,counter;

    PcsSlicesNumFor25g = 5;

    channelExpectedBandwidth = ((channelPcsSlice / PcsSlicesNumFor25g) * 25);
    counter = ((32 * channelExpectedBandwidth)/400);
    xfast = 0x3FF;
    xslow = 0x3FF;
    xon = (counter > 8 ? (6*counter-11) : (counter > 3 ? (5*counter)-1 : (4*counter-1)));
    xoff = (counter > 8 ? (6*counter-10) : (counter > 3 ? (5*counter) : 4*counter));

    /*xfast = ((32 * channelExpectedBandwidth)/400);
    xslow = (xfast +1);
    xon = (16/xfast +2);
    xoff = (xfast > 4? (6*xfast) :(7*xfast));*/
    /*xon*/
    expectedRavenRxCreditsArr[4] = expectedEagleRxCreditsArr[4] = xon;
    /*xoff*/
    expectedRavenRxCreditsArr[5] = expectedEagleRxCreditsArr[5] = xoff;
    /*xfast*/
    expectedRavenRxCreditsArr[6] = expectedEagleRxCreditsArr[6] = xfast;
    /*xslow*/
    expectedRavenRxCreditsArr[7] = expectedEagleRxCreditsArr[7] = xslow;

    /* calculate expected eagle credits according to expected PCS  slices*/
    /*Tx credits*/
    /*tx fifo*/
    expectedEagleTxCreditsArr[1] = (channelIndex*2);
    /*selected bandwidth*/
    expectedEagleTxCreditsArr[2] = (channelPcsSlice == 80 ? 2 : (channelPcsSlice == 40 ? 1 : 0));
    /* tx fifo segments number*/
    expectedEagleTxCreditsArr[3] = (channelIndex == 8? 1 :(channelExpectedBandwidth*16)/400);
    /* init credits*/
    expectedEagleTxCreditsArr[0] = (6*expectedEagleTxCreditsArr[3]);

    /*RX credits*/
    /* init credits: Rx on eagle does not use credits*/
    expectedEagleRxCreditsArr[0] = expectedEagleRxCreditsArr[1] = expectedEagleRxCreditsArr[3] = 0 /* not relevant*/;
    /*selected bandwidth*/
    expectedEagleRxCreditsArr[2] = (channelPcsSlice == 80 ? 2 : (channelPcsSlice == 40 ? 1 : 0));

    /* calculate expected raven credits according to expected PCS  slices*/
    /* Tx credits*/
    /*tx fifo*/
    expectedRavenTxCreditsArr[1] = (channelIndex*3);
    /*selected bandwidth*/
    expectedRavenTxCreditsArr[2] = (channelPcsSlice == 80 ? 2 : (channelPcsSlice == 40 ? 1 : 0));
    /* tx fifo segments number*/
    expectedRavenTxCreditsArr[3] = (channelIndex == 8? 1 :((channelExpectedBandwidth*24)/400));
    /* init credits*/
    expectedRavenTxCreditsArr[0] = (4*expectedRavenTxCreditsArr[3]);

    /* Rx credits*/
    /* init credits*/
    expectedRavenRxCreditsArr[0] = 0;
    /*rx fifo*/
    expectedRavenRxCreditsArr[1] = (channelIndex*4);
    /*selected bandwidth*/
    expectedRavenRxCreditsArr[2] = (channelPcsSlice == 80 ? 2 : (channelPcsSlice == 40 ? 1 : 0));
    /* rx fifo segments number*/
    expectedRavenRxCreditsArr[3] = (channelIndex == 8? 2 :counter);

    return GT_OK;
}



/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPizzaArbiterDevStateGet
(
IN  GT_U8  devNum,
IN  GT_U32 portGroupId,
OUT CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC *pizzaDeviceStatePtr
)
*/
GT_VOID cpssDxChPortPizzaArbiterDevStateGetUT(GT_VOID)
    /* UTF_TEST_CASE_MAC(cpssDxChPortPizzaArbiterDevStateGet) */
{

    /*
    ITERATE_DEVICES (Lion2, Bobcat2, Caelum, Bobcat3)
    1.1.1. Call with valid pizzaDeviceStatePtr[NOT NULL].
    Expected: GT_OK.
    1.1.2. Call with out of range pizzaDeviceStatePtr[NULL].
    Expected: GT_BAD_PTR.
    */

    GT_STATUS                             st                  = GT_OK;
    GT_U8                                 dev                 = 0;
    GT_U32                                portGroupId         = 0;
    static CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC pizzaDeviceState;


    #ifdef PA_UT_SEQ
        cpssOsPrintf("\ncpssDxChPortPizzaArbiterDevStateGetUT enter");
    #endif

    cpssOsMemSet(&pizzaDeviceState, 0,
        sizeof(CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC));

    /* ITERATE_DEVICES (Lion2, Bobcat2, Caelum, Bobcat3) */

    /* prepare device iterator to go through all applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
        UTF_CH2_E | UTF_CH3_E |
        UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E  | UTF_FALCON_E );

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            CPSS_TBD_BOOKMARK_FALCON
            SKIP_TEST_MAC;
        }

        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {

            /*
            1.1.1. Call with valid pizzaDeviceStatePtr[NOT NULL].
            Expected: GT_OK.
            */
            cpssOsMemSet(&pizzaDeviceState, 0,
                sizeof(CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC));
            st = cpssDxChPortPizzaArbiterDevStateGet(dev, portGroupId,
                &pizzaDeviceState);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
            1.1.2. Call with out of range pizzaDeviceStatePtr[NULL].
            Expected: GT_BAD_PTR.
            */
            st = cpssDxChPortPizzaArbiterDevStateGet(dev, portGroupId,NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            cpssOsMemSet(&pizzaDeviceState, 0, sizeof(CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC));
            st = cpssDxChPortPizzaArbiterDevStateGet(dev, portGroupId, &pizzaDeviceState);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupId);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    /* prepare device iterator to go through all applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
        UTF_CH2_E | UTF_CH3_E |
        UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        cpssOsMemSet(&pizzaDeviceState, 0,sizeof(CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC));
        st = cpssDxChPortPizzaArbiterDevStateGet(dev, portGroupId,&pizzaDeviceState);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    cpssOsMemSet(&pizzaDeviceState, 0,sizeof(CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC));
    st = cpssDxChPortPizzaArbiterDevStateGet(dev, portGroupId,&pizzaDeviceState);
    UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_OK, st);

#ifdef PA_UT_SEQ
    cpssOsPrintf("\ncpssDxChPortPizzaArbiterDevStateGetUT exit");
#endif
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPizzaArbiterDevStateInit
(
INOUT CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC *pizzaDeviceStatePtr
)
UTF_TEST_CASE_MAC(cpssDxChPortPizzaArbiterDevStateInit)
*/
GT_VOID cpssDxChPortPizzaArbiterDevStateInitUT(GT_VOID)
{
    /*
    1.1.1. Call with valid pizzaDeviceStatePtr[NOT NULL].
    Expected: GT_OK.
    1.1.2. Call with out of range pizzaDeviceStatePtr[NULL].
    Expected: GT_BAD_PTR.
    */

    GT_STATUS                             st                  = GT_OK;
    static CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC pizzaDeviceState;

    #ifdef PA_UT_SEQ
        cpssOsPrintf("\ncpssDxChPortPizzaArbiterDevStateInitUT enter");
    #endif

    cpssOsMemSet(&pizzaDeviceState, 0, sizeof(CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC));

    /*
    1.1.1. Call with valid pizzaDeviceStatePtr[NOT NULL].
    Expected: GT_OK.
    */
    st = cpssDxChPortPizzaArbiterDevStateInit(&pizzaDeviceState);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /*
    1.1.2. Call with out of range pizzaDeviceStatePtr[NULL].
    Expected: GT_BAD_PTR.
    */
    st = cpssDxChPortPizzaArbiterDevStateInit(NULL);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

    #ifdef PA_UT_SEQ
        cpssOsPrintf("\ncpssDxChPortPizzaArbiterDevStateInitUT exit");
    #endif
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPizzaArbiterIfSliceStateGet
(
IN  GT_U8 devNum,
IN  GT_U32 portGroupIdx,
IN  GT_U32 sliceID,
OUT GT_BOOL              *isOccupiedPtr,
OUT GT_PHYSICAL_PORT_NUM *portNumPtr
)
UTF_TEST_CASE_MAC(cpssDxChPortPizzaArbiterIfSliceStateGet)
*/
GT_VOID cpssDxChPortPizzaArbiterIfSliceStateGetUT(GT_VOID)
{
    /*
    ITERATE_DEVICES (Lion2)
    1.1.1. Call with sliceId[0, 0xFFFF, 0xFFFFFFFF],
    valid isOccupiedPtr[NOT NULL] and valid portNumPtr[NOT NULL].
    Expected: GT_OK.
    1.1.2. Call with out of range isOccupiedPtr[NULL] and
    other parameters same as 1.1.1.
    Expected: GT_BAD_PTR.
    1.1.3. Call with out of range portNumPtr[NULL] and
    other parameters same as 1.1.1.
    Expected: GT_BAD_PTR.
    */

    GT_STATUS                             st                  = GT_OK;
    GT_U8                                 dev                 = 0;
    GT_U32                                portGroupId         = 0;
    GT_U32                                sliceID             = 0;
    GT_BOOL                               isOccupied          = GT_FALSE;
    GT_PHYSICAL_PORT_NUM                  portNum             = 0;

    /* ITERATE_DEVICES (Lion2) */
    #ifdef PA_UT_SEQ
        cpssOsPrintf("\ncpssDxChPortPizzaArbiterIfSliceStateGetUT enter");
    #endif

    #ifdef PA_UT_SEQ
        cpssOsPrintf("\n   cpssDxChPortPizzaArbiterIfSliceStateGetUT stage 1...");
    #endif

    /* prepare device iterator to go through all applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_LION2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {

            /*
            1.1.1. Call with sliceId[0, 0xFFFF, 0xFFFFFFFF],
            valid isOccupiedPtr[NOT NULL] and valid portNumPtr[NOT NULL].
            Expected: GT_OK.
            */
            sliceID = 0;
            st = cpssDxChPortPizzaArbiterIfSliceStateGet(dev, portGroupId, sliceID, &isOccupied,&portNum);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
            1.1.1. Call with sliceId[0, 0xFFFF, 0xFFFFFFFF],
            valid isOccupiedPtr[NOT NULL] and valid portNumPtr[NOT NULL].
            Expected: GT_OK.
            */
            sliceID = 0xFFFF;
            st = cpssDxChPortPizzaArbiterIfSliceStateGet(dev, portGroupId,sliceID, &isOccupied,&portNum);
            UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_OK, st);

            /*
            1.1.1. Call with sliceId[0, 0xFFFF, 0xFFFFFFFF],
            valid isOccupiedPtr[NOT NULL] and valid portNumPtr[NOT NULL].
            Expected: GT_OK.
            */
            sliceID = 0xFFFFFFFF;
            st = cpssDxChPortPizzaArbiterIfSliceStateGet(dev, portGroupId,sliceID, &isOccupied,&portNum);
            UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_OK, st);

            /*
            1.1.2. Call with out of range isOccupiedPtr[NULL] and
            other parameters same as 1.1.1.
            Expected: GT_BAD_PTR.
            */
            sliceID = 0;
            st = cpssDxChPortPizzaArbiterIfSliceStateGet(dev, portGroupId,sliceID, NULL,&portNum);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

            /*
            1.1.3. Call with out of range portNumPtr[NULL] and
            other parameters same as 1.1.1.
            Expected: GT_BAD_PTR.
            */
            sliceID = 0;
            st = cpssDxChPortPizzaArbiterIfSliceStateGet(dev, portGroupId,sliceID, &isOccupied,NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            st = cpssDxChPortPizzaArbiterIfSliceStateGet(dev, portGroupId,
                sliceID, &isOccupied,
                &portNum);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupId);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
    }
    #ifdef PA_UT_SEQ
        cpssOsPrintf("   Done");
    #endif

    #ifdef PA_UT_SEQ
        cpssOsPrintf("\n   cpssDxChPortPizzaArbiterIfSliceStateGetUT stage 2...");
    #endif

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_LION2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPizzaArbiterIfSliceStateGet(dev, portGroupId,sliceID, &isOccupied,&portNum);
        UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_OK, st);
    }

    #ifdef PA_UT_SEQ
        cpssOsPrintf("   Done");
    #endif

        /* 3. Call function with out of bound value for device id.*/
    #ifdef PA_UT_SEQ
        cpssOsPrintf("\n   cpssDxChPortPizzaArbiterIfSliceStateGetUT stage 3...");
    #endif

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPizzaArbiterIfSliceStateGet(dev, portGroupId, sliceID, &isOccupied, &portNum);
    UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_OK, st);

    #ifdef PA_UT_SEQ
        cpssOsPrintf("   Done");
    #endif

    #ifdef PA_UT_SEQ
        cpssOsPrintf("\ncpssDxChPortPizzaArbiterIfSliceStateGetUT exit");
    #endif
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPizzaArbiterIfUserTableGet
(
IN   GT_U8 devNum,
IN   GT_U32 portGroupBmp,
IN   CPSS_DXCH_PIZZA_PROFILE_STC                          **pizzaProfilePtrPtrPtr,
IN   CPSS_DXCH_PORT_PIZZA_PORTGROUP_SPEED_TO_SLICENUM_STC **portGroupPortSpeed2SliceNumPtrPtr
)
UTF_TEST_CASE_MAC(cpssDxChPortPizzaArbiterIfUserTableGet)
*/
GT_VOID cpssDxChPortPizzaArbiterIfUserTableGetUT(GT_VOID)
{
    /*
    ITERATE_DEVICES (Lion2)
    1.1.1. Call with valid pPizzaProfilePtrPtr[NON-NULL] and
    valid value of portGroupPortSpeed2SliceNumPtrPtr[NON-NULL].
    Expected: GT_OK.
    1.1.2. Call with valid pPizzaProfilePtrPtr[NON-NULL] and
    invalid value of portGroupPortSpeed2SliceNumPtrPtr[NULL].
    Expected: GT_OK.
    1.1.3. Call with invalid pPizzaProfilePtrPtr[NULL] and
    valid value of portGroupPortSpeed2SliceNumPtrPtr[NON-NULL].
    Expected: GT_OK.
    1.1.4. Call with out of range pPizzaProfilePtrPtr[NULL] and
    out of range value of portGroupPortSpeed2SliceNumPtrPtr[NULL].
    Expected: GT_BAD_PTR.
    */

    GT_STATUS                                   st                  = GT_OK;
    GT_U8                                       dev                 = 0;
    GT_U32                                      portGroupId         = 0;
    GT_PORT_GROUPS_BMP                          portGroupsBmp       = 1;
    CPSS_DXCH_PIZZA_PROFILE_STC                           * pPizzaProfilePtr = (CPSS_DXCH_PIZZA_PROFILE_STC *)NULL;
    CPSS_DXCH_PORT_PIZZA_PORTGROUP_SPEED_TO_SLICENUM_STC  * portGroupPortSpeed2SliceNumPtr = (CPSS_DXCH_PORT_PIZZA_PORTGROUP_SPEED_TO_SLICENUM_STC *)NULL;

    #ifdef PA_UT_SEQ
        cpssOsPrintf("\ncpssDxChPortPizzaArbiterIfUserTableGetUT enter");
    #endif

        /* ITERATE_DEVICES (Lion2) */

        /* prepare iterator for go over all active devices */
    #ifdef PA_UT_SEQ
        cpssOsPrintf("\n    cpssDxChPortPizzaArbiterIfUserTableGetUT stage 1...");
    #endif

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
        UTF_CH2_E | UTF_CH3_E |
        UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E | UTF_FALCON_E |
         UTF_CPSS_PP_E_ARCH_CNS );

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);

            /*
            1.1.1. Call with valid pPizzaProfilePtrPtr[NON-NULL] and
            valid value of portGroupPortSpeed2SliceNumPtrPtr[NON-NULL].
            Expected: GT_OK.
            */
            st = cpssDxChPortPizzaArbiterIfUserTableGet(dev, portGroupsBmp,&pPizzaProfilePtr,&portGroupPortSpeed2SliceNumPtr);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
            1.1.2. Call with valid pPizzaProfilePtrPtr[NON-NULL] and
            invalid value of portGroupPortSpeed2SliceNumPtrPtr[NULL].
            Expected: GT_OK.
            */
            st = cpssDxChPortPizzaArbiterIfUserTableGet(dev, portGroupsBmp,&pPizzaProfilePtr,NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
            1.1.3. Call with invalid pPizzaProfilePtrPtr[NULL] and
            valid value of portGroupPortSpeed2SliceNumPtrPtr[NON-NULL].
            Expected: GT_OK.
            */
            st = cpssDxChPortPizzaArbiterIfUserTableGet(dev, portGroupsBmp,NULL,&portGroupPortSpeed2SliceNumPtr);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
            1.1.4. Call with out of range pPizzaProfilePtrPtr[NULL] and
            out of range value of portGroupPortSpeed2SliceNumPtrPtr[NULL].
            Expected: GT_BAD_PTR.
            */
            st = cpssDxChPortPizzaArbiterIfUserTableGet(dev, portGroupsBmp,NULL, NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set next non-active port */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChPortPizzaArbiterIfUserTableGet(dev, portGroupsBmp, &pPizzaProfilePtr,&portGroupPortSpeed2SliceNumPtr);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
        st = cpssDxChPortPizzaArbiterIfUserTableGet(dev, portGroupsBmp, &pPizzaProfilePtr, &portGroupPortSpeed2SliceNumPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
    }

    #ifdef PA_UT_SEQ
        cpssOsPrintf("Done");
    #endif

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
    #ifdef PA_UT_SEQ
        cpssOsPrintf("\n    cpssDxChPortPizzaArbiterIfUserTableGetUT stage 2...");
    #endif

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
        UTF_CH2_E | UTF_CH3_E |
        UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E |
        UTF_FALCON_E | UTF_CPSS_PP_E_ARCH_CNS );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPizzaArbiterIfUserTableGet(dev, portGroupsBmp,&pPizzaProfilePtr,&portGroupPortSpeed2SliceNumPtr);
        UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_OK, st);
    }

    #ifdef PA_UT_SEQ
        cpssOsPrintf("Done");
    #endif

        /* 3. Call function with out of bound value for device id.*/
    #ifdef PA_UT_SEQ
        cpssOsPrintf("\n    cpssDxChPortPizzaArbiterIfUserTableGetUT stage 3...");
    #endif

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPizzaArbiterIfUserTableGet(dev, portGroupsBmp, &pPizzaProfilePtr,&portGroupPortSpeed2SliceNumPtr);
    UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_OK, st);
    #ifdef PA_UT_SEQ
        cpssOsPrintf("Done");
    #endif

    #ifdef PA_UT_SEQ
        cpssOsPrintf("\ncpssDxChPortPizzaArbiterIfUserTableGetUT exit");
    #endif
}


GT_U32 PA_UT_BWRES_SET_DEBUG = 0;

GT_STATUS utPADebugPrintSet(GT_VOID)
{
    PA_UT_BWRES_SET_DEBUG = 1;
    return GT_OK;
}


GT_STATUS utRemovePort
(
    IN GT_U8 dev,
    IN GT_PHYSICAL_PORT_NUM               portNum,
    IN CPSS_PORT_INTERFACE_MODE_ENT       ifMode,
    IN CPSS_PORT_SPEED_ENT                speed
)
{
    GT_STATUS rc;
    CPSS_PORTS_BMP_STC portsBmp;
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
    CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp,portNum);
    rc =  cpssDxChPortModeSpeedSet(dev, &portsBmp, GT_FALSE,ifMode,speed);
    return rc;
}

GT_STATUS utConfigurePort
(
    IN GT_U8 dev,
    IN GT_PHYSICAL_PORT_NUM               portNum,
    IN CPSS_PORT_INTERFACE_MODE_ENT       ifMode,
    IN CPSS_PORT_SPEED_ENT                speed
)
{
    GT_STATUS rc;
    CPSS_PORTS_BMP_STC portsBmp;
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
    CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp,portNum);
    rc =  cpssDxChPortModeSpeedSet(dev, &portsBmp, GT_TRUE,ifMode,speed);
    return rc;
}



GT_STATUS utIsThereTM
(
    IN    GT_U8    dev,
    OUT   GT_BOOL *isTmPresent,
    OUT   GT_BOOL *isCpuPresent
)
{
    GT_STATUS                          rc;
    GT_PHYSICAL_PORT_NUM               portNum;
    static CPSS_DXCH_DETAILED_PORT_MAP_STC portMapShadow;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr = &portMapShadow;
    GT_U32                             maxPortNum;

    rc = cpssDxChCfgTableNumEntriesGet(dev, CPSS_DXCH_CFG_TABLE_PHYSICAL_PORT_E, /*OUT*/&maxPortNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    for (portNum = 0; portNum < maxPortNum; portNum++)
    {
        rc = cpssDxChPortPhysicalPortDetailedMapGet(dev,portNum,/*OUT*/portMapShadowPtr);
        if (rc != GT_OK)
        {
            return rc;
        }

        if (portMapShadowPtr->valid == GT_TRUE)
        {
            if (portMapShadowPtr->portMap.tmPortIdx != CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS)
            {
                *isTmPresent = GT_TRUE;
            }
            if (portMapShadowPtr->portMap.mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E)
            {
                *isCpuPresent = GT_TRUE;
            }
        }
    }
    *isTmPresent = GT_FALSE;
    return GT_OK;
}

extern GT_STATUS appDemoBobk2PortGobalResourcesPrint(GT_U8 dev);
extern GT_STATUS appDemoBobk2PortListResourcesPrint(GT_U8 dev);


GT_STATUS utRemoveAllEthernetPorts
(
    IN    GT_U8  dev,
    IN    GT_U32 size,
    INOUT CPSS_PORT_SPEED_ENT            *   speedArr,
    INOUT CPSS_PORT_INTERFACE_MODE_ENT   *  ifModeArr,
    INOUT GT_BOOL                        *toDeleteArr
)
{
    GT_STATUS                          rc;
    GT_PHYSICAL_PORT_NUM               portNum;
    static CPSS_DXCH_DETAILED_PORT_MAP_STC portMapShadow;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr = &portMapShadow;
    CPSS_PORT_SPEED_ENT                speed;
    CPSS_PORT_INTERFACE_MODE_ENT       ifMode;
    GT_U32                             maxPortNum;

    /*-----------------------------------------------*/
    /* collect info about Network ports and total BW */
    /*-----------------------------------------------*/
    if (PA_UT_BWRES_SET_DEBUG == 1)
    {
        cpssOsPrintf("\nRemove all ports");
    }


    rc = cpssDxChCfgTableNumEntriesGet(dev, CPSS_DXCH_CFG_TABLE_PHYSICAL_PORT_E, /*OUT*/&maxPortNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (maxPortNum > size)
    {
        maxPortNum = size;
    }

    if (PA_UT_BWRES_SET_DEBUG == 1)
    {
        appDemoBobk2PortGobalResourcesPrint(dev);
        appDemoBobk2PortListResourcesPrint(dev);
    }

    if (PA_UT_BWRES_SET_DEBUG == 1)
    {
        cpssOsPrintf("\nmaxPortNum = %d", maxPortNum);
    }


    for (portNum = 0; portNum < maxPortNum; portNum++)
    {
        if (speedArr != NULL)
        {
            speedArr[portNum]  = CPSS_PORT_SPEED_NA_E;
        }
        if (ifModeArr != NULL)
        {
            ifModeArr[portNum] = CPSS_PORT_INTERFACE_MODE_NA_E;
        }
        if (toDeleteArr != NULL)
        {
            toDeleteArr[portNum] = GT_FALSE;
        }

        rc = cpssDxChPortPhysicalPortDetailedMapGet(dev,portNum,/*OUT*/portMapShadowPtr);
        if (rc != GT_OK)
        {
            return rc;
        }

        if (portMapShadowPtr->valid == GT_TRUE)
        {
            if (portMapShadowPtr->portMap.mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
            {
                speed = PRV_CPSS_DXCH_PORT_SPEED_MAC(dev, portMapShadowPtr->portMap.macNum);
                ifMode = PRV_CPSS_DXCH_PORT_IFMODE_MAC(dev, portMapShadowPtr->portMap.macNum);
#if 0
                rc = cpssDxChPortInterfaceModeGet(dev,portNum,/*OUT*/&ifMode);
                if (rc != GT_OK)
                {
                    return rc;
                }
#endif
                if (speed != CPSS_PORT_SPEED_NA_E)
                {
                    if (PA_UT_BWRES_SET_DEBUG == 1)
                    {
                        cpssOsPrintf("\n    port %3d speed %d if = %d ... ",portNum,speed,ifMode);
                    }
                    if (PA_UT_BWRES_SET_DEBUG == 1)
                    {
                        cpssOsPrintf("\n    port %3d speed %s if = %s ... ",portNum,CPSS_SPEED_2_STR(speed),CPSS_IF_2_STR(ifMode));
                    }
                    rc = utRemovePort(dev,portNum,ifMode,speed);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }

                    if (speedArr != NULL)
                    {
                        speedArr[portNum] = speed;
                    }
                    if (ifModeArr != NULL)
                    {
                        ifModeArr[portNum] = ifMode;
                    }
                    if (toDeleteArr != NULL)
                    {
                        toDeleteArr[portNum] = GT_TRUE;
                    }
                    if (PA_UT_BWRES_SET_DEBUG == 1)
                    {
                        cpssOsPrintf("OK");
                    }
                }
            }
        }
    }
    if (PA_UT_BWRES_SET_DEBUG == 1)
    {
        appDemoBobk2PortGobalResourcesPrint(dev);
        appDemoBobk2PortListResourcesPrint(dev);
        cpssOsPrintf("\nutRemoveAllEthernetPorts: Done.");
    }
    return GT_OK;
}


GT_STATUS utRestoreAllPorts
(
    IN GT_U8  dev,
    IN CPSS_PORT_SPEED_ENT               *speedArr /* [PRV_CPSS_DXCH_PORT_NUM_CNS] */,
    IN CPSS_PORT_INTERFACE_MODE_ENT     *ifModeArr /* [PRV_CPSS_DXCH_PORT_NUM_CNS] */,
    IN GT_BOOL                        *toDeleteArr /* [PRV_CPSS_DXCH_PORT_NUM_CNS] */
)
{
    GT_STATUS                          rc;
    GT_PHYSICAL_PORT_NUM               portNum;
    CPSS_PORT_SPEED_ENT                speed;
    CPSS_PORT_INTERFACE_MODE_ENT       ifMode;


    if (PA_UT_BWRES_SET_DEBUG == 1)
    {
        cpssOsPrintf("\nRestore all ports");
        appDemoBobk2PortGobalResourcesPrint(dev);
        appDemoBobk2PortListResourcesPrint(dev);
    }
    for (portNum = 0; portNum < PRV_CPSS_MAX_PP_PORTS_NUM_CNS; portNum++)
    {
        if (toDeleteArr[portNum] == GT_TRUE)
        {
            speed  = speedArr[portNum];
            ifMode = ifModeArr[portNum];
            if (PA_UT_BWRES_SET_DEBUG == 1)
            {
                cpssOsPrintf("\n    port %3d speed %s if = %s ... ",portNum,CPSS_SPEED_2_STR(speed),CPSS_IF_2_STR(ifMode));
            }
            rc = utConfigurePort(dev,portNum,ifMode,speed);
            if (rc != GT_OK)
            {
                return rc;
            }
            if (PA_UT_BWRES_SET_DEBUG == 1)
            {
                cpssOsPrintf("OK");
            }
        }
    }
    if (PA_UT_BWRES_SET_DEBUG == 1)
    {
        appDemoBobk2PortGobalResourcesPrint(dev);
        appDemoBobk2PortListResourcesPrint(dev);
    }
    return GT_OK;
}


/*******************************************************************************
* cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionSet
*
* DESCRIPTION:
*       set pipe BW for TxDMA, RXDMA, TxQ, Tx-FIFO, Eth-Tx-FIFO and slice resolution for TxQ
*
* APPLICABLE DEVICES:
*        Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* INPUTS:
*       devNum                       - device number
*       pipeBandwithInGbps           - bandwidth in GBits
*       defMinimalPortSpeedMBps – slice resolution in MBps
*
* OUTPUTS:
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - wrong devNum, combination of BW and slice resolution
*       GT_FAIL         - on error
*
* COMMENTS:
*
*******************************************************************************/
/*
GT_STATUS cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionSet
(
IN GT_U8                     devNum,
IN GT_U32                    pipeBandwithInGbps,
IN GT_U32                    defMinimalPortSpeedMBps
);
*/

GT_VOID cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionSetUT(GT_VOID)
{
    GT_STATUS st  = GT_OK;
    GT_U8     dev = 0;
    GT_U32                   defPipeBandwithInGbps;
    CPSS_DXCH_MIN_SPEED_ENT  defMinimalPortSpeedMBps;

    static CPSS_PORT_SPEED_ENT               speedArr[PRV_CPSS_MAX_PP_PORTS_NUM_CNS];
    static CPSS_PORT_INTERFACE_MODE_ENT     ifModeArr[PRV_CPSS_MAX_PP_PORTS_NUM_CNS];
    static GT_BOOL                        toDeleteArr[PRV_CPSS_MAX_PP_PORTS_NUM_CNS];


    typedef struct
    {
        GT_PHYSICAL_PORT_NUM         portNum;
        CPSS_PORT_SPEED_ENT          speed;
        CPSS_PORT_INTERFACE_MODE_ENT ifMode;
    }PORTINITLIST_STC;

    typedef struct
    {
        CPSS_PP_FAMILY_TYPE_ENT  devType;
        PORTINITLIST_STC        *portInitListPtr[10];
    }DEVTYPE_PORTINITLIST_STC;

    GT_U32 portIdx;

    #define UT_INVALID_PORT_CNS (GT_PHYSICAL_PORT_NUM)(~0)

    static PORTINITLIST_STC bc2_portInitList_4x10G_2x40G[] =
    {
         { 48,  CPSS_PORT_SPEED_10000_E, CPSS_PORT_INTERFACE_MODE_SR_LR_E }
        ,{ 49,  CPSS_PORT_SPEED_10000_E, CPSS_PORT_INTERFACE_MODE_SR_LR_E }
        ,{ 50,  CPSS_PORT_SPEED_10000_E, CPSS_PORT_INTERFACE_MODE_SR_LR_E }
        ,{ 51,  CPSS_PORT_SPEED_10000_E, CPSS_PORT_INTERFACE_MODE_SR_LR_E }
        ,{ 52,  CPSS_PORT_SPEED_10000_E, CPSS_PORT_INTERFACE_MODE_SR_LR_E }
        ,{ 53,  CPSS_PORT_SPEED_10000_E, CPSS_PORT_INTERFACE_MODE_SR_LR_E }
        ,{ 54,  CPSS_PORT_SPEED_10000_E, CPSS_PORT_INTERFACE_MODE_SR_LR_E }
        ,{ 55,  CPSS_PORT_SPEED_10000_E, CPSS_PORT_INTERFACE_MODE_SR_LR_E }
        ,{ 56,  CPSS_PORT_SPEED_40000_E, CPSS_PORT_INTERFACE_MODE_SR_LR_E }
        ,{ 80,  CPSS_PORT_SPEED_40000_E, CPSS_PORT_INTERFACE_MODE_SR_LR_E }
        ,{ UT_INVALID_PORT_CNS,  CPSS_PORT_SPEED_NA_E,    CPSS_PORT_INTERFACE_MODE_NA_E    }
    };

    static PORTINITLIST_STC bc2_portInitList_4x40G[] =
    {
         { 48,  CPSS_PORT_SPEED_40000_E, CPSS_PORT_INTERFACE_MODE_SR_LR_E }
        ,{ 52,  CPSS_PORT_SPEED_40000_E, CPSS_PORT_INTERFACE_MODE_SR_LR_E }
        ,{ 56,  CPSS_PORT_SPEED_40000_E, CPSS_PORT_INTERFACE_MODE_SR_LR_E }
        ,{ 80,  CPSS_PORT_SPEED_40000_E, CPSS_PORT_INTERFACE_MODE_SR_LR_E }
        ,{ UT_INVALID_PORT_CNS,  CPSS_PORT_SPEED_NA_E,    CPSS_PORT_INTERFACE_MODE_NA_E    }
    };

    static PORTINITLIST_STC aldrin_portInitList_4x40G[] =
    {
         {  0,  CPSS_PORT_SPEED_40000_E, CPSS_PORT_INTERFACE_MODE_SR_LR_E }
        ,{  4,  CPSS_PORT_SPEED_40000_E, CPSS_PORT_INTERFACE_MODE_SR_LR_E }
        ,{  8,  CPSS_PORT_SPEED_40000_E, CPSS_PORT_INTERFACE_MODE_SR_LR_E }
        ,{ 12,  CPSS_PORT_SPEED_40000_E, CPSS_PORT_INTERFACE_MODE_SR_LR_E }
        ,{ UT_INVALID_PORT_CNS,  CPSS_PORT_SPEED_NA_E,    CPSS_PORT_INTERFACE_MODE_NA_E    }
    };

    static DEVTYPE_PORTINITLIST_STC dev_portInitList[] =
    {
         { CPSS_PP_FAMILY_DXCH_BOBCAT2_E, { &bc2_portInitList_4x10G_2x40G[0], &bc2_portInitList_4x40G[0],         (PORTINITLIST_STC *)NULL } }
        ,{ CPSS_PP_FAMILY_DXCH_BOBCAT3_E, { &bc2_portInitList_4x10G_2x40G[0], &bc2_portInitList_4x40G[0],         (PORTINITLIST_STC *)NULL } }
        ,{ CPSS_PP_FAMILY_DXCH_ALDRIN_E,  { &aldrin_portInitList_4x40G[0],                                        (PORTINITLIST_STC *)NULL } }
        ,{ CPSS_PP_FAMILY_DXCH_AC3X_E,    { &aldrin_portInitList_4x40G[0],                                        (PORTINITLIST_STC *)NULL } }
        ,{ CPSS_PP_FAMILY_DXCH_ALDRIN2_E, { &bc2_portInitList_4x10G_2x40G[0], &bc2_portInitList_4x40G[0],      (PORTINITLIST_STC *)NULL } }
        /* currently Falcon as BC3 */
        CPSS_TBD_BOOKMARK_FALCON
        ,{ CPSS_PP_FAMILY_DXCH_FALCON_E, { &bc2_portInitList_4x10G_2x40G[0], &bc2_portInitList_4x40G[0],         (PORTINITLIST_STC *)NULL } }
    };

    PORTINITLIST_STC * listPtr;
    static CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC  pizzaDeviceState;
    GT_U32 i;
    GT_BOOL isTmPresent;
    GT_BOOL isCpuPresent;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    PORTINITLIST_STC **portInitList;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
        UTF_CH2_E | UTF_CH3_E |
        UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E |UTF_LION2_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    #if (PA_PipeBWMinPortSpeedResolutionSetUT_DEBUG_PRINT == 1)
        cpssOsPrintf("\nTest start");
    #endif

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if (PA_UT_BWRES_SET_DEBUG == 1)
        {
            cpssOsPrintf("\n    Check TM & CPU present ...");
        }

        st = utIsThereTM(dev,/*OUT*/&isTmPresent,&isCpuPresent);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        if (PA_UT_BWRES_SET_DEBUG == 1)
        {
            cpssOsPrintf("TM = %d  CPU = %d  OK",isTmPresent,isCpuPresent);
        }

        if (PA_UT_BWRES_SET_DEBUG == 1)
        {
            cpssOsPrintf("\n    Get default values of PipeBW and speed resolution ...");
        }

        /* get default values */
        st = cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionGet(dev, /*OUT*/&defPipeBandwithInGbps, /*OUT*/&defMinimalPortSpeedMBps);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        if (PA_UT_BWRES_SET_DEBUG == 1)
        {
            cpssOsPrintf("PipeBW = %d    speed resolution = %d",defPipeBandwithInGbps,defMinimalPortSpeedMBps);
        }

        st = cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionSet(dev, 0, defMinimalPortSpeedMBps);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        st = cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionSet(dev, defPipeBandwithInGbps,CPSS_DXCH_MIN_SPEED_INVALID_E);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        st = cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionSet(dev, 180,CPSS_DXCH_MIN_SPEED_500_Mbps_E); /* not enough slices */
        if (PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_FAIL, st, dev);

            st = cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionSet(dev, 194,CPSS_DXCH_MIN_SPEED_500_Mbps_E); /* not enough slices */
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        if (PRV_CPSS_DXCH_BOBCAT2_A0_CHECK_MAC(dev) || PRV_CPSS_DXCH_BOBCAT2_B0_CHECK_MAC(dev) || PRV_CPSS_DXCH_CETUS_CHECK_MAC(dev))
        {
            st = cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionSet(dev, 160,CPSS_DXCH_MIN_SPEED_1000_Mbps_E);
        }
        else
        {
            if (PRV_CPSS_DXCH_CAELUM_CHECK_MAC(dev))
            {
                st = cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionSet(dev, 170,CPSS_DXCH_MIN_SPEED_1000_Mbps_E);
            }
            else
            {
                st = GT_OK;
            }
        }

        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionSet(dev, 160,CPSS_DXCH_MIN_SPEED_10000_Mbps_E);  /* on 29,1, shall give error !!! */
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_FAIL, st, dev);                                                              /* there are ports with speed smaller */
        /* than 10G */
        if (isTmPresent == GT_FALSE)
        {
            st = utRemoveAllEthernetPorts(dev,PRV_CPSS_MAX_PP_PORTS_NUM_CNS,/*OUT*/&speedArr[0], &ifModeArr[0],&toDeleteArr[0]);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            if (st != GT_OK)
            {
                cpssOsPrintf("\nError port removing %d",st);
                return;
            }
            if (PA_UT_BWRES_SET_DEBUG == 1)
            {
                cpssOsPrintf("\nSystem clean");
            }


            st = cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionSet(dev,180,CPSS_DXCH_MIN_SPEED_1000_Mbps_E);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* not compained with resolution */
            /* with change of 2.5G to 3G it is complained to resolution now
               don't execute
            */
            /*
            st = utConfigurePort(dev,0,CPSS_PORT_INTERFACE_MODE_SGMII_E,CPSS_PORT_SPEED_2500_E);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
            */
            /* find device specific BW list */
            st = prvUtfDeviceFamilyGet(dev, &devFamily);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

            st = GT_NOT_FOUND;
            portInitList = NULL;
            for (i = 0 ; i < sizeof(dev_portInitList)/sizeof(dev_portInitList[0]); i++)
            {
                if (dev_portInitList[i].devType == devFamily)
                {
                    portInitList = &(dev_portInitList[i].portInitListPtr[0]);
                    st = GT_OK;
                    break;
                }
            }
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,st, "port init list by dev: %d empty", dev);

            for (i = 0  ;  portInitList[i] != (PORTINITLIST_STC *)NULL; i++)
            {
                if (PA_UT_BWRES_SET_DEBUG == 1)
                {
                    cpssOsPrintf("\nConfiguration %d",i);
                }
                listPtr = portInitList[i];
                for (portIdx = 0; listPtr[portIdx].portNum != UT_INVALID_PORT_CNS ; portIdx++)
                {
                    CPSS_ENABLER_PORT_SKIP_CHECK(dev,listPtr[portIdx].portNum);

                    if(prvCpssDxChPortRemotePortCheck(dev, listPtr[portIdx].portNum))
                    {
                       st = utConfigurePort(dev,listPtr[portIdx].portNum,listPtr[portIdx].ifMode,listPtr[portIdx].speed);
                       UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, listPtr[portIdx].portNum);
                       continue;
                    }

                    if (PA_UT_BWRES_SET_DEBUG == 1)
                    {
                        cpssOsPrintf("\n    Port %2d : %s ... ",listPtr[portIdx].portNum, CPSS_SPEED_2_STR(listPtr[portIdx].speed));
                    }
                    st = utConfigurePort(dev,listPtr[portIdx].portNum,listPtr[portIdx].ifMode,listPtr[portIdx].speed);
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, listPtr[portIdx].portNum);
                    if (PA_UT_BWRES_SET_DEBUG == 1)
                    {
                        cpssOsPrintf("OK");
                    }
                }
                utilCpssDxChPortPizzaArbiterHighSpeedPortCheck(
                    dev, MIN_HIGH_SPEED_CNS);
                if (PA_UT_BWRES_SET_DEBUG == 1)
                {
                    cpssOsPrintf("\n    Remove all ports");
                }
                st = utRemoveAllEthernetPorts(dev,PRV_CPSS_MAX_PP_PORTS_NUM_CNS,/*OUT*/(CPSS_PORT_SPEED_ENT*)NULL,
                    (CPSS_PORT_INTERFACE_MODE_ENT*)NULL,
                    (GT_BOOL*)NULL);  /* remove all configured ports , don't care what them are */
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            }
            /* check total configured slices is possible to do only after first allocation */
            {
                GT_U32      unitIdx;
                GT_BOOL     unitFound;
                st = cpssDxChPortPizzaArbiterDevStateGet(dev,0, /*OUT*/&pizzaDeviceState);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                unitFound = GT_FALSE;
                for (unitIdx = 0 ; pizzaDeviceState.devState.bobK.unitList[unitIdx] != CPSS_DXCH_PA_UNIT_UNDEFINED_E; unitIdx++)
                {
                    if (pizzaDeviceState.devState.bobK.unitList[unitIdx] == CPSS_DXCH_PA_UNIT_TXQ_0_E )
                    {
                        unitFound = GT_TRUE;
                        break;
                    }
                }
                if (unitFound == GT_FALSE)
                {
                    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_TRUE, unitFound);
                }
                UTF_VERIFY_EQUAL0_STRING_MAC(180, pizzaDeviceState.devState.bobK.unitState[unitIdx].totalConfiguredSlices,"got another slice number at TxQ");
            }

            /* restore default values */
            st = cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionSet(dev, defPipeBandwithInGbps, defMinimalPortSpeedMBps);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            st = utRestoreAllPorts(dev,&speedArr[0], &ifModeArr[0],&toDeleteArr[0]);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        }

        /* restore default values */
        st = cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionSet(dev, defPipeBandwithInGbps, defMinimalPortSpeedMBps);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
        UTF_CH2_E | UTF_CH3_E |
        UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E |UTF_LION2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionSet(dev, defPipeBandwithInGbps, defMinimalPortSpeedMBps);
        UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_OK, st);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionSet(dev, defPipeBandwithInGbps, defMinimalPortSpeedMBps);
    UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_OK, st);
    if (PA_UT_BWRES_SET_DEBUG == 1)
    {
        cpssOsPrintf("\nTest end");
        cpssOsPrintf("\n");
    }
}

GT_VOID cpssDxChPortPizzaArbiter_cleanup_withSystemReset(GT_VOID)
{
    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;
    /* when the function cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionSetUT() fails
        it leave the device in bad ports configuration ... some if not all ports get
        into 'NA' speed and ifMode ... so it impact next tests */
    if(prvTgfResetModeGet() == GT_TRUE)
    {
        /* give priority to reset with HW */
        prvTgfResetAndInitSystem();
    }
    else
    {
        if(prvTgfResetModeGet_gtShutdownAndCoreRestart() == GT_TRUE)
        {
            if (prvUtfIsTrafficManagerUsed() && IS_BOBK_DEV_MAC(prvTgfDevNum))
            {
                /* avoid failure of the process by duplicated TM DRAM init */
                appDemoDbEntryAdd("tmDramFailIgnore", 1);
            }

            /* if can't do reset with HW ... do only SW reset */
            prvTgfResetAndInitSystem_gtShutdownAndCoreRestart();
        }
        else
        {
            /* not applicable device */
            prvUtfSkipTestsSet();
        }
    }
}

GT_VOID cpssDxChPortPizzaArbiter_cleanup_withSystemResetUT(GT_VOID)
{
    utilCpssDxChPortPizzaArbiterHighSpeedPortCheck(
        prvTgfDevNum, MIN_HIGH_SPEED_CNS);
    cpssDxChPortPizzaArbiter_cleanup_withSystemReset();
}



GT_STATUS cpssDxChPortPizzaArbiter_initSystem_on_diffCCUT_CCListSelect
(
    IN  GT_U8     dev,
    OUT GT_U32   *bestrevisionPtr,
    OUT GT_U32 **coreClockListPtrPtr
)
{
    typedef struct
    {
        CPSS_PP_FAMILY_TYPE_ENT     devType;
        CPSS_PP_SUB_FAMILY_TYPE_ENT subFamily;
        GT_U32                      revision;
        GT_U32                      coreClockArr[10];  /* last item shall be zero */
    }DEVTYPE_CC_STC;


    static DEVTYPE_CC_STC  devRevCc[] =
    {
         { CPSS_PP_FAMILY_DXCH_BOBCAT2_E, CPSS_PP_SUB_FAMILY_NONE_E,         0, {                          0 }  } /* BC2 A0 is not interesting */
        ,{ CPSS_PP_FAMILY_DXCH_BOBCAT2_E, CPSS_PP_SUB_FAMILY_NONE_E,         1, { 175, 250, 362, 521,      0 }  } /* BC2 B0   */
        ,{ CPSS_PP_FAMILY_DXCH_BOBCAT2_E, CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E, 0, { 167, 200, 250, 365,      0 }  } /* Bobk Caelum/cetus */
        ,{ CPSS_PP_FAMILY_DXCH_ALDRIN_E,  CPSS_PP_SUB_FAMILY_NONE_E,         0, { 200, 250, 365,           0 }  } /* Aldrin            */
        ,{ CPSS_PP_FAMILY_DXCH_AC3X_E,    CPSS_PP_SUB_FAMILY_NONE_E,         0, { 200, 250, 365,           0 }  } /* Aldrin            */
        ,{ CPSS_PP_FAMILY_DXCH_BOBCAT3_E, CPSS_PP_SUB_FAMILY_NONE_E,         0, {                          0 }  } /* BC3 currently not defined */
        ,{ CPSS_PP_FAMILY_DXCH_ALDRIN2_E, CPSS_PP_SUB_FAMILY_NONE_E,      0, {                          0 }  } /* Aldrin2 currently not defined */
    /* currently Falcon as BC3 */
    CPSS_TBD_BOOKMARK_FALCON
        ,{ CPSS_PP_FAMILY_DXCH_FALCON_E, CPSS_PP_SUB_FAMILY_NONE_E,         0, {                          0 }  } /* BC3 currently not defined */
    };

    GT_STATUS st;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    CPSS_PP_SUB_FAMILY_TYPE_ENT subFamily;
    GT_U32 revision;
    GT_U32 i;
    GT_U32 * coreClockList;
    GT_U32 bestRevision;

    CPSS_NULL_PTR_CHECK_MAC(coreClockListPtrPtr);
    CPSS_NULL_PTR_CHECK_MAC(bestrevisionPtr);

    devFamily = PRV_CPSS_PP_MAC(dev)->devFamily;
    subFamily = PRV_CPSS_PP_MAC(dev)->devSubFamily;
    revision  = PRV_CPSS_PP_MAC(dev)->revision;

   /*----------------------------------*
    * find any suitable revision       *
    *----------------------------------*/

    st = GT_NOT_FOUND;
    coreClockList = (GT_U32*)NULL;
    bestRevision = 0;
    for (i = 0 ; i < sizeof(devRevCc)/sizeof(devRevCc[0]); i++)
    {
        if (devRevCc[i].devType == devFamily && devRevCc[i].subFamily == subFamily)
        {
            if (devRevCc[i].revision <= revision)
            {
                bestRevision  = devRevCc[i].revision;
                coreClockList = &devRevCc[i].coreClockArr[0];
                st = GT_OK;
                break;
            }
        }
    }
    if (st != GT_OK)
    {
        *bestrevisionPtr     = (GT_U32)(-1);
        *coreClockListPtrPtr = (GT_U32 *)NULL;
        return st;
    }

    /*----------------------------------*
     * find best suitable revision      *
     *----------------------------------*/
    for (i = 0 ; i < sizeof(devRevCc)/sizeof(devRevCc[0]); i++)
    {
        if (devRevCc[i].devType == devFamily && devRevCc[i].subFamily == subFamily)
        {
            if (devRevCc[i].revision <= revision)
            {
                if (bestRevision < devRevCc[i].revision)
                {
                    bestRevision  = devRevCc[i].revision;
                    coreClockList = &devRevCc[i].coreClockArr[0];
                }
            }
        }
    }
    *bestrevisionPtr = bestRevision;
    *coreClockListPtrPtr = coreClockList;
    return GT_OK;
}

GT_VOID cpssDxChPortPizzaArbiter_initSystem_on_diffCCUT(GT_VOID)
{
#ifdef ASIC_SIMULATION
    #ifndef GM_USED

        GT_STATUS st  = GT_OK;
        GT_U8     dev = 0;
        CPSS_PP_FAMILY_TYPE_ENT devFamily;
        CPSS_PP_SUB_FAMILY_TYPE_ENT subFamily;
        GT_U32 revision;
        GT_U32 * coreClockList;
        GT_U32   bestRevision;
        GT_U32   i;


        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
            UTF_CH2_E | UTF_CH3_E |
            UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E |UTF_LION2_E | UTF_FALCON_E);

        /* 1. Go over all active devices. */
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
            GT_BOOL isTmPresent;
            GT_BOOL isCpuPresent;

            st = utIsThereTM(dev,&isTmPresent,&isCpuPresent);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            if (isTmPresent == GT_FALSE)
            {
                devFamily = PRV_CPSS_PP_MAC(dev)->devFamily;
                subFamily = PRV_CPSS_PP_MAC(dev)->devSubFamily;
                revision  = PRV_CPSS_PP_MAC(dev)->revision;

                st = cpssDxChPortPizzaArbiter_initSystem_on_diffCCUT_CCListSelect(dev,/*OUT*/&bestRevision,&coreClockList);
                UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK,st, "device %d type %d subFamily %d revision %d: coreClock list not found", dev, devFamily, subFamily,revision);

                if (coreClockList[0] == 0)
                {
                    continue;
                }

                /* reset system to default state */
                prvTgfResetAndInitSystem();

                for (i = 0 ; coreClockList[i] > 0 ; i++)
                {
                    GT_U32 coreClock;
                    coreClock = coreClockList[i];

                    #ifdef ASIC_SIMULATION
                        scibCoreClockRegisterUpdate(dev/*simulation devId*/,coreClock,0/*ignored*/);
                    #endif

                    /* Reset system for new device ID */
                    st = prvTgfResetAndInitSystem_gtShutdownAndCoreRestart();
                    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "prvTgfResetAndInitSystem_gtShutdownAndCoreRestart(): devFamily %d subfamily %d, best-revision %d coreClock %d",
                                                 devFamily,subFamily,bestRevision,coreClock);
                }
                /* reset system to default state */
                prvTgfResetAndInitSystem();
            }
        }
    #else
        prvUtfSkipTestsSet();
        return;
    #endif /* #ifndef GM_USED */
#else
    prvUtfSkipTestsSet();
    return;
#endif /* #ifdef ASIC_SIMULATION */

}



/*******************************************************************************
* cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionGet
*
* DESCRIPTION:
*       get pipe BW for TxDMA, RXDMA, TxQ, Tx-FIFO, Eth-Tx-FIFO and slice resolution for TxQ
*
* APPLICABLE DEVICES:
*        Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* INPUTS:
*       devNum                       - device number
*
* OUTPUTS:
*       pipeBandwithInGbpsPtr           - bandwidth in GBits
*       minimalSliceResolutionInMBpsPtr – min port speed resolution in MBps
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PTR      - wrong devNum, combination of BW and slice resolution
*       GT_FAIL         - on error
*
* COMMENTS:
*
*******************************************************************************/
/*
GT_STATUS cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionGet
(
IN  GT_U8                     devNum,
OUT GT_U32                   *pipeBandwithInGbpsPtr,
OUT GT_U32                   *minimalSliceResolutionInMBpsPtr
);
*/

GT_VOID cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionGetUT(GT_VOID)
{
    GT_STATUS st  = GT_OK;
    GT_U8     dev = 0;
    GT_U32                     defPipeBandwithInGbps;
    CPSS_DXCH_MIN_SPEED_ENT    defMinimalPortSpeedMBps;
    GT_U32                     pipeBandwithInGbps, pipeBandwithInGbps_compare;
    CPSS_DXCH_MIN_SPEED_ENT    minimalPortSpeedMBps, minimalPortSpeedMBps_compare;


    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
        UTF_CH2_E | UTF_CH3_E |
        UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E |UTF_LION2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get default values */
        st = cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionGet(dev, /*OUT*/&defPipeBandwithInGbps, /*OUT*/&defMinimalPortSpeedMBps);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        pipeBandwithInGbps_compare = defPipeBandwithInGbps;
        minimalPortSpeedMBps_compare = defMinimalPortSpeedMBps;

        /* aldrin configured to maximum - skip test case */
        if (!PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(dev))
        {
            pipeBandwithInGbps_compare = 240;
            minimalPortSpeedMBps_compare = CPSS_DXCH_MIN_SPEED_1000_Mbps_E;

            st = cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionSet(dev, /*IN*/pipeBandwithInGbps_compare, /*IN*/minimalPortSpeedMBps_compare);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        st = cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionGet(dev, /*OUT*/&pipeBandwithInGbps, /*OUT*/NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(pipeBandwithInGbps, pipeBandwithInGbps_compare,
            "got another pipeBandwithInGbps: %d", pipeBandwithInGbps);

        st = cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionGet(dev, /*OUT*/NULL, /*OUT*/&minimalPortSpeedMBps);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(minimalPortSpeedMBps, minimalPortSpeedMBps_compare,
            "got another minimalPortSpeedMBps: %d", minimalPortSpeedMBps);

        if (!PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(dev))
        {
            /* restore default values */
            st = cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionSet(dev, defPipeBandwithInGbps, defMinimalPortSpeedMBps);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
        UTF_CH2_E | UTF_CH3_E |
        UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E |UTF_LION2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionGet(dev, &defPipeBandwithInGbps, &defMinimalPortSpeedMBps);
        UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_OK, st);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionGet(dev, &defPipeBandwithInGbps, &defMinimalPortSpeedMBps);
    UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_OK, st);

}

#define DEBUG_WC_TEST 0

#if (DEBUG_WC_TEST == 1)
typedef struct PA_UNIT_2_STR_STCT
{
    CPSS_DXCH_PA_UNIT_ENT  paUnit;
    GT_CHAR               *str;
}PA_UNIT_2_STR_STC;

static PA_UNIT_2_STR_STC pa_unit2str_List[] =
{
     { CPSS_DXCH_PA_UNIT_RXDMA_0_E,              "RXDMA"         }
    ,{ CPSS_DXCH_PA_UNIT_RXDMA_1_E,            "RXDMA_1"       }
    ,{ CPSS_DXCH_PA_UNIT_RXDMA_2_E,            "RXDMA_2"       }
    ,{ CPSS_DXCH_PA_UNIT_TXQ_0_E,                "TXQ"           }
    ,{ CPSS_DXCH_PA_UNIT_TXDMA_0_E,              "TXDMA"         }
    ,{ CPSS_DXCH_PA_UNIT_TXDMA_1_E,            "TXDMA_1"       }
    ,{ CPSS_DXCH_PA_UNIT_TXDMA_2_E,            "TXDMA_2"       }
    ,{ CPSS_DXCH_PA_UNIT_TX_FIFO_0_E,            "TX_FIFO"       }
    ,{ CPSS_DXCH_PA_UNIT_TX_FIFO_1_E,          "TX_FIFO_1"     }
    ,{ CPSS_DXCH_PA_UNIT_TX_FIFO_2_E,          "TX_FIFO_2"     }
    ,{ CPSS_DXCH_PA_UNIT_ETH_TX_FIFO_0_E,        "ETH_TX_FIFO"   }
    ,{ CPSS_DXCH_PA_UNIT_ETH_TX_FIFO_1_E,      "ETH_TX_FIFO_1" }
    ,{ CPSS_DXCH_PA_UNIT_ILKN_TX_FIFO_E,       "ILKN_TX_FIFO"  }
    ,{ CPSS_DXCH_PA_UNIT_RXDMA_GLUE_E,         "RXDMA_GLUE"    }
    ,{ CPSS_DXCH_PA_UNIT_TXDMA_GLUE_E,         "TXDMA_GLUE"    }
    ,{ CPSS_DXCH_PA_UNIT_MPPM_E,               "MPPM"          }
    ,{ CPSS_DXCH_PA_UNIT_UNDEFINED_E,          (GT_CHAR *)NULL         }
};

static GT_CHAR * paUnit22StrFind
(
    CPSS_DXCH_PA_UNIT_ENT  paUnit
)
{
    GT_U32 i;
    static GT_CHAR badUnitNameMsg[30];
    for (i = 0 ; pa_unit2str_List[i].paUnit != CPSS_DXCH_PA_UNIT_UNDEFINED_E; i++)
    {
        if (pa_unit2str_List[i].paUnit  == paUnit)
        {
            return pa_unit2str_List[i].str;
        }
    }
    osSprintf(badUnitNameMsg,"Unit %d : Unknown Name",paUnit);
    return &badUnitNameMsg[0];
}
#endif

GT_VOID cpssDxChPortDynamicPizzaArbiterIfWorkConservingModeSetUT(GT_VOID)
{
    GT_STATUS st  = GT_OK;
    GT_U8     dev = 0;
    GT_U32    unitIdx;
    GT_U32    unitIdx2Check;
    CPSS_DXCH_PA_UNIT_ENT unit,unitCheck;
    GT_BOOL   status;
    GT_BOOL   status2set;
#if (DEBUG_WC_TEST == 1)
    GT_U32    regAddr;
#endif
    CPSS_DXCH_PA_UNIT_ENT  *supportedUnitListPtr;

    GT_BOOL  defStatusArr[CPSS_DXCH_PA_UNIT_MAX_E];

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
        UTF_CH2_E | UTF_CH3_E |
        UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E |UTF_LION2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* for BC2 A0 don't run the test */
        if (PRV_CPSS_PP_MAC(dev)->devSubFamily == CPSS_PP_SUB_FAMILY_NONE_E &&  PRV_CPSS_PP_MAC(dev)->revision == 0)
        {
            SKIP_TEST_MAC;
        }

        #if (DEBUG_WC_TEST==1)
            cpssOsPrintf("\n    Get List of supported units ...");
        #endif
        st = prvCpssDxChPortDynamicPizzaArbiterWSSupportedUnitListGet(dev,/*OUT*/&supportedUnitListPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        #if (DEBUG_WC_TEST==1)
            cpssOsPrintf("OK");
        #endif

        /*------------------------------------------------*/
        /* get defualt values for each unit and set TRUE  */
        /*------------------------------------------------*/
        #if (DEBUG_WC_TEST==1)
            cpssOsPrintf("\n    Get Default values for each of supported units ...");
        #endif

        for (unitIdx = 0 ;  supportedUnitListPtr[unitIdx] != CPSS_DXCH_PA_UNIT_UNDEFINED_E; unitIdx++)
        {
            unit = supportedUnitListPtr[unitIdx];
            #if (DEBUG_WC_TEST==1)
                cpssOsPrintf("%s ",paUnit22StrFind(unit));
            #endif


            if (unit == CPSS_DXCH_PA_UNIT_ILKN_TX_FIFO_E)
            {
                continue;
            }
            st = cpssDxChPortDynamicPizzaArbiterIfWorkConservingModeGet(dev,unit,/*OUT*/&defStatusArr[unit]);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,unit);
        }

        #if (DEBUG_WC_TEST==1)
            cpssOsPrintf("OK");
        #endif


        /*------------------------------------------------*/
        /* set for all units GT_FALSE/GT_TRUE             */
        /* check TxQ ports for corresponding value        */
        /*------------------------------------------------*/
        #if (DEBUG_WC_TEST==1)
            cpssOsPrintf("\n    Check all units for single values ...");
        #endif
        for (status2set = GT_FALSE; status2set <= GT_TRUE; status2set++)
        {
            GT_U32 txqPort;

            /*------------------------------------------------*/
            /* set for all units GT_FALSE/GT_TRUE             */
            /*------------------------------------------------*/
            unit = CPSS_DXCH_PA_UNIT_UNDEFINED_E;
            st = cpssDxChPortDynamicPizzaArbiterIfWorkConservingModeSet(dev,unit,status2set);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,unit);
            #if (DEBUG_WC_TEST==1)
                cpssOsPrintf("\n        Set All %d.",status2set);
            #endif

            #if (DEBUG_WC_TEST==1)
                cpssOsPrintf("\n             Read units:");
            #endif
            for (unitIdx = 0 ;  supportedUnitListPtr[unitIdx] != CPSS_DXCH_PA_UNIT_UNDEFINED_E; unitIdx++)
            {
                unit = supportedUnitListPtr[unitIdx];
                #if (DEBUG_WC_TEST==1)
                    cpssOsPrintf("\n             %s :",paUnit22StrFind(unit));
                #endif

                /*------------------------------------------------*/
                /* check for GT_FALSE/GT_TRUE                     */
                /*------------------------------------------------*/
                if (unit == CPSS_DXCH_PA_UNIT_ILKN_TX_FIFO_E)
                {
                    continue;
                }
                st = cpssDxChPortDynamicPizzaArbiterIfWorkConservingModeGet(dev,unit,&status);
                #if (DEBUG_WC_TEST==1)
                    cpssOsPrintf("%d",status);
                #endif

                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,unit);
                UTF_VERIFY_EQUAL2_STRING_MAC(status, status2set,"unit %d : work conserving mode value shall %d",unit,status2set);
            }
            /*------------------------------------------------*/
            /* check TxQ ports for corresponding value        */
            /*------------------------------------------------*/
            #if (DEBUG_WC_TEST==1)
                cpssOsPrintf("\n             Check Txq ports  %d. Read:",status2set);
            #endif
            for (txqPort = 0 ; txqPort < PRV_CPSS_DXCH_BC2B0_PORT_WORK_PORTN_CNS; txqPort++)
            {
                #if (DEBUG_WC_TEST==1)
                {
                    regAddr = PRV_DXCH_REG1_UNIT_TXQ_DQ_MAC(dev).scheduler.portArbiterConfig.portsArbiterPortWorkConservingEnable[txqPort];
                    cpssOsPrintf("\n             %3d : 0x%8X ",txqPort,regAddr);
                }
                #endif

                st = prvCpssDxChPortDynamicPizzaArbiterIfTxQPortWorkConservingModeGet(dev,txqPort,&status);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,txqPort);
                UTF_VERIFY_EQUAL2_STRING_MAC(status, status2set,"txqport %d : work conserving mode value shall %d",txqPort,status2set);
            }

        }
        #if (DEBUG_WC_TEST==1)
            cpssOsPrintf("\n    OK");
        #endif

        /*-----------------------------------*/
        /* now set all unit to GT_FALSE      */
        /* set single unit to GT_TRUE        */
        /* check that others remain GT_FALSE */
        /*-----------------------------------*/
        #if (DEBUG_WC_TEST==1)
            cpssOsPrintf("\n    For each unit : Set all FALSE, Set single unit to TRUE others shall remain FALSE ...");
        #endif

        unit = CPSS_DXCH_PA_UNIT_UNDEFINED_E;
        st = cpssDxChPortDynamicPizzaArbiterIfWorkConservingModeSet(dev,unit,GT_FALSE);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,unit);
        for (unitIdx = 0 ;  supportedUnitListPtr[unitIdx] != CPSS_DXCH_PA_UNIT_UNDEFINED_E; unitIdx++)
        {
            unit = supportedUnitListPtr[unitIdx];
            if (unit == CPSS_DXCH_PA_UNIT_ILKN_TX_FIFO_E)
            {
                continue;
            }
            #if (DEBUG_WC_TEST==1)
                cpssOsPrintf("%s ",paUnit22StrFind(unit));
            #endif

            st = cpssDxChPortDynamicPizzaArbiterIfWorkConservingModeSet(dev,unit,GT_TRUE);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,unit);
            /*-----------------------------------------------------------*/
            /* check all units but current for default value (GT_FALSE)  */
            /* check curent for GT_TRUE                                  */
            /*-----------------------------------------------------------*/
            for (unitIdx2Check = 0 ;  supportedUnitListPtr[unitIdx2Check] != CPSS_DXCH_PA_UNIT_UNDEFINED_E; unitIdx2Check++)
            {
                unitCheck = supportedUnitListPtr[unitIdx2Check];
                if (unitCheck == CPSS_DXCH_PA_UNIT_ILKN_TX_FIFO_E)
                {
                    continue;
                }
                if (unitCheck == unit)
                {
                    st = cpssDxChPortDynamicPizzaArbiterIfWorkConservingModeGet(dev,unit,&status);
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,unit,status2set);
                    UTF_VERIFY_EQUAL2_STRING_MAC(status, GT_TRUE,"unit %d : work conserving mode value shall %d",unit,GT_TRUE);
                }
                else
                {
                    st = cpssDxChPortDynamicPizzaArbiterIfWorkConservingModeGet(dev,unitCheck,&status);
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,unit,status);
                    UTF_VERIFY_EQUAL2_STRING_MAC(status, GT_FALSE,"unit %d : work conserving mode value shall %d",unit,GT_FALSE);
                }
            }
            st = cpssDxChPortDynamicPizzaArbiterIfWorkConservingModeSet(dev,unit,GT_FALSE);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,unit);
        }
        #if (DEBUG_WC_TEST==1)
            cpssOsPrintf("OK");
        #endif

        /*-----------------------------------*/
        /* now set all unit to GT_TRUE       */
        /* set single unit to  GT_FALSE      */
        /* check that others remain GT_TRUE  */
        /*-----------------------------------*/
        #if (DEBUG_WC_TEST==1)
            cpssOsPrintf("\n    For each unit : Set all TRUE, Set single unit to FALSE others shall remain TRUE ...");
        #endif
        unit = CPSS_DXCH_PA_UNIT_UNDEFINED_E;
        st = cpssDxChPortDynamicPizzaArbiterIfWorkConservingModeSet(dev,unit,GT_TRUE);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,unit);
        for (unitIdx = 0 ;  supportedUnitListPtr[unitIdx] != CPSS_DXCH_PA_UNIT_UNDEFINED_E; unitIdx++)
        {
            unit = supportedUnitListPtr[unitIdx];
            if (unit == CPSS_DXCH_PA_UNIT_ILKN_TX_FIFO_E)
            {
                continue;
            }
            #if (DEBUG_WC_TEST==1)
                cpssOsPrintf("%s ",paUnit22StrFind(unit));
            #endif
            st = cpssDxChPortDynamicPizzaArbiterIfWorkConservingModeSet(dev,unit,GT_FALSE);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,unit);
            /*-----------------------------------------------------------*/
            /* check all units but current for default value (GT_TRUE)   */
            /* check curent for GT_FALSE                                 */
            /*-----------------------------------------------------------*/
            for (unitIdx2Check = 0 ;  supportedUnitListPtr[unitIdx2Check] != CPSS_DXCH_PA_UNIT_UNDEFINED_E; unitIdx2Check++)
            {
                unitCheck = supportedUnitListPtr[unitIdx2Check];
                if (unitCheck == CPSS_DXCH_PA_UNIT_ILKN_TX_FIFO_E)
                {
                    continue;
                }
                if (unitCheck == unit)
                {
                    st = cpssDxChPortDynamicPizzaArbiterIfWorkConservingModeGet(dev,unit,&status);
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,unit,status2set);
                    UTF_VERIFY_EQUAL2_STRING_MAC(status, GT_FALSE,"unit %d : work conserving mode value shall %d",unit,GT_FALSE);
                }
                else
                {
                    st = cpssDxChPortDynamicPizzaArbiterIfWorkConservingModeGet(dev,unitCheck,&status);
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,unit,status);
                    UTF_VERIFY_EQUAL2_STRING_MAC(status, GT_TRUE,"unit %d : work conserving mode value shall %d",unit,GT_TRUE);
                }
            }
            st = cpssDxChPortDynamicPizzaArbiterIfWorkConservingModeSet(dev,unit,GT_TRUE);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,unit);
        }
        #if (DEBUG_WC_TEST==1)
            cpssOsPrintf("OK");
        #endif


        /*--------------------------------------*/
        /* restore default values for each unit */
        /*--------------------------------------*/
        #if (DEBUG_WC_TEST==1)
            cpssOsPrintf("\n    Restore default values ...");
        #endif
        for (unitIdx = 0 ;  supportedUnitListPtr[unitIdx] != CPSS_DXCH_PA_UNIT_UNDEFINED_E; unitIdx++)
        {
            unit = supportedUnitListPtr[unitIdx];
            if (unit == CPSS_DXCH_PA_UNIT_ILKN_TX_FIFO_E)
            {
                continue;
            }
            st = cpssDxChPortDynamicPizzaArbiterIfWorkConservingModeSet(dev,unit,defStatusArr[unit]);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,unit);
        }
        #if (DEBUG_WC_TEST==1)
            cpssOsPrintf("OK");
        #endif

    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
        UTF_CH2_E | UTF_CH3_E |
        UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E |UTF_LION2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        for (unit = (CPSS_DXCH_PA_UNIT_ENT)0 ;  unit < CPSS_DXCH_PA_UNIT_MAX_E; unit++)
        {
            st = cpssDxChPortDynamicPizzaArbiterIfWorkConservingModeSet(dev,unit,GT_TRUE);
            UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_OK, st);
        }
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    for (unit = (CPSS_DXCH_PA_UNIT_ENT)0 ;  unit < CPSS_DXCH_PA_UNIT_MAX_E; unit++)
    {
        st = cpssDxChPortDynamicPizzaArbiterIfWorkConservingModeSet(dev,unit,GT_TRUE);
        UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_OK, st);
    }
    #if (DEBUG_WC_TEST == 1)
        cpssOsPrintf("\nTest finished\n");
    #endif
}


GT_BOOL FindUnitInUnitList(CPSS_DXCH_PA_UNIT_ENT* unitListPtr,CPSS_DXCH_PA_UNIT_ENT unit)
{
    GT_U32 i;
    if (unitListPtr != NULL)
    {
        for (i = 0 ; unitListPtr[i] != CPSS_DXCH_PA_UNIT_UNDEFINED_E; i++)
        {
            if (unitListPtr[i] == unit)
            {
                return GT_TRUE;
            }
        }
    }
    return GT_FALSE;
}

GT_VOID cpssDxChPortDynamicPizzaArbiterIfWorkConservingModeGetUT(GT_VOID)
{
    GT_STATUS st  = GT_OK;
    GT_U8     dev = 0;
    CPSS_DXCH_PA_UNIT_ENT unit;
    GT_BOOL  status;
    GT_U32   unitIdx;
    PRV_CPSS_DXCH_PA_WORKSPACE_STC *workSpacePtr;
    CPSS_DXCH_PA_UNIT_ENT  *supportedUnitListPtr;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
        UTF_CH2_E | UTF_CH3_E |
        UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E |UTF_LION2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if (PRV_CPSS_PP_MAC(dev)->devSubFamily == CPSS_PP_SUB_FAMILY_NONE_E &&  PRV_CPSS_PP_MAC(dev)->revision == 0)
        {
            SKIP_TEST_MAC;
        }

        st = prvCpssDxChPortDynamicPizzaArbiterWSGet(dev,&workSpacePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        supportedUnitListPtr = workSpacePtr->prv_DeviceUnitListPtr;

        /* get default values */
        for (unitIdx = 0 ;  supportedUnitListPtr[unitIdx] != CPSS_DXCH_PA_UNIT_UNDEFINED_E; unitIdx++)
        {
            GT_BOOL isUnitFound;

            unit = supportedUnitListPtr[unitIdx];
            if (unit == CPSS_DXCH_PA_UNIT_ILKN_TX_FIFO_E)
            {
                continue;
            }
            st = cpssDxChPortDynamicPizzaArbiterIfWorkConservingModeGet(dev,unit,&status);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,unit);
            isUnitFound = FindUnitInUnitList(workSpacePtr->workConservingModeOnUnitListPtr,unit);
            if (isUnitFound == GT_FALSE)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(status, GT_FALSE,"unit %d : work conserving mode default value shall GT_FALSE",unit);
            }
            else
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(status,GT_TRUE,"unit %d : work conserving mode default value shall GT_TRUE",unit);

            }

            st = cpssDxChPortDynamicPizzaArbiterIfWorkConservingModeGet(dev,unit,NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev,unit);
        }

        st = cpssDxChPortDynamicPizzaArbiterIfWorkConservingModeGet(dev,CPSS_DXCH_PA_UNIT_UNDEFINED_E,&status);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PARAM, st, "get with CPSS_DXCH_PA_UNIT_UNDEFINED_E unit");

        st = cpssDxChPortDynamicPizzaArbiterIfWorkConservingModeGet(dev,CPSS_DXCH_PA_UNIT_MAX_E,&status);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PARAM, st, "get with CPSS_DXCH_PA_UNIT_MAX_E unit shall GT_BAD_PARAM");

    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
        UTF_CH2_E | UTF_CH3_E |
        UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E |UTF_LION2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        for (unit = (CPSS_DXCH_PA_UNIT_ENT)0 ;  unit < CPSS_DXCH_PA_UNIT_MAX_E; unit++)
        {
            st = cpssDxChPortDynamicPizzaArbiterIfWorkConservingModeGet(dev,unit,&status);
            UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_OK, st);
        }
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    for (unit = (CPSS_DXCH_PA_UNIT_ENT)0 ;  unit < CPSS_DXCH_PA_UNIT_MAX_E; unit++)
    {
        st = cpssDxChPortDynamicPizzaArbiterIfWorkConservingModeGet(dev,unit,&status);
        UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_OK, st);
    }
}




/*-----------------------------------------------------*/
/* Bobcat2 PortModeSpeedSet()                          */
/*-----------------------------------------------------*/
typedef struct
{
    GT_PHYSICAL_PORT_NUM portNum;
    GT_BOOL              isUsed;
}PORTCAPACITY_STC;

typedef struct
{
    GT_U32                             cnt;
    GT_PHYSICAL_PORT_NUM            portNumArr[PRV_CPSS_MAX_PP_PORTS_NUM_CNS];
    CPSS_PORT_SPEED_ENT               speedArr[PRV_CPSS_MAX_PP_PORTS_NUM_CNS];
    CPSS_PORT_INTERFACE_MODE_ENT     ifModeArr[PRV_CPSS_MAX_PP_PORTS_NUM_CNS];
}ListOfConfiguredPorts_STC;


GT_VOID ListOfConfiguredPortsInit
(
    ListOfConfiguredPorts_STC * listPtr
)
{
    listPtr->cnt = 0;
}

GT_VOID ListOfConfiguredPortsAppend
(
    ListOfConfiguredPorts_STC        *listPtr,
    GT_PHYSICAL_PORT_NUM              portNum,
    CPSS_PORT_SPEED_ENT               speed,
    CPSS_PORT_INTERFACE_MODE_ENT      ifMode
)
{
    listPtr->portNumArr[listPtr->cnt] = portNum;
    listPtr->speedArr[listPtr->cnt]   = speed;
    listPtr->ifModeArr[listPtr->cnt]  = ifMode;
    listPtr->cnt++;
}



GT_U32 randi(GT_U32 bnd)
{
    GT_U32 r = cpssOsRand();
    r = r%bnd;
    return r;
}



GT_STATUS generatePortsAccordingToAvailableBW
(
    IN    GT_U8               dev,
    INOUT GT_U32             *totalBwGbpsPtr,
    IN    GT_U32              portSpeed2generateGbps,
    IN    PORTCAPACITY_STC  **portListPtr,
    OUT   GT_U32             *numOfConfPortsPtr,
    OUT   ListOfConfiguredPorts_STC  *confPortListPtr,
    IN    CPSS_PORT_INTERFACE_MODE_ENT  ifMode,
    IN    CPSS_PORT_SPEED_ENT           speed
)
{
    GT_STATUS st;
    GT_U32 numOfPorts;
    GT_U32 idx;
    GT_U32 totalBwGbps = *totalBwGbpsPtr;
    GT_U32 posiblePortN;

    *numOfConfPortsPtr = 0;
    posiblePortN = totalBwGbps/portSpeed2generateGbps;
    if (posiblePortN == 0)
    {
        return GT_OK;
    }
    *numOfConfPortsPtr = numOfPorts = randi(posiblePortN+1);
    if (numOfPorts == 0)
    {
        return GT_OK;
    }
    totalBwGbps -= numOfPorts*portSpeed2generateGbps;
    for (idx = 0 ; portListPtr[idx] != NULL ;  idx++)
    {
        if (portListPtr[idx]->isUsed == GT_FALSE)
        {
            st = utConfigurePort(dev,portListPtr[idx]->portNum,ifMode,speed);
            if (st == GT_OK)
            {
                ListOfConfiguredPortsAppend(confPortListPtr,portListPtr[idx]->portNum,speed,ifMode);
                portListPtr[idx]->isUsed = GT_TRUE;
                numOfPorts--;
                if (numOfPorts == 0)
                {
                    break;
                }
            }
            else
            {
                if (st != GT_BAD_STATE) /* port cann't be configured , resources are accupied by nearby port */
                {
                    return st;
                }
            }
        }
    }
    totalBwGbps += numOfPorts*portSpeed2generateGbps; /* add non used BW */
    *totalBwGbpsPtr = totalBwGbps;
    return GT_OK;
}



GT_STATUS pizzaArbiterCheckUnitsSlices
(
    CPSS_DXCH_BOBCAT2_SLICES_PIZZA_ARBITER_STATE_STC * unitPtr,
    GT_U32   slicesOnUnit,
    GT_U32   nonEmptySlices,
    GT_BOOL  emptySliceEnable
)
{
    GT_U32 sliceIdx;
    GT_U32 cnt;


    if (emptySliceEnable == GT_FALSE)
    {
        if (slicesOnUnit < nonEmptySlices)
        {
            return GT_FAIL;
        }
        if (unitPtr->totalConfiguredSlices != slicesOnUnit)
        {
            return GT_FAIL;
        }
    }
    else
    {
        if (unitPtr->totalConfiguredSlices != slicesOnUnit)
        {
            return GT_FAIL;
        }
    }

    cnt = 0;
    for (sliceIdx = 0; sliceIdx < unitPtr->totalConfiguredSlices; sliceIdx++)
    {
        if (unitPtr->slice_enable[sliceIdx] == GT_TRUE)
        {
            cnt++;
        }
    }

    if (emptySliceEnable == GT_FALSE)
    {
        if (cnt != unitPtr->totalConfiguredSlices)
        {
            return GT_FAIL;
        }
    }
    else
    {
        if (cnt != nonEmptySlices)
        {
            return GT_FAIL;
        }
    }

    for (sliceIdx = unitPtr->totalConfiguredSlices; sliceIdx < unitPtr->totalSlicesOnUnit; sliceIdx++)
    {
        if (unitPtr->slice_enable[sliceIdx] == GT_TRUE)
        {
            return GT_FAIL;
        }
    }
    return GT_OK;
}

GT_STATUS pizzaArbiterCheckUnitsWoTM
(
    GT_U32                reqBWGbps,
    GT_U32                defPipeBandwithInGbps,
    CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC * pizzaDeviceStatePtr
)
{
    GT_STATUS st;
    GT_U32 i;
    CPSS_DXCH_PA_UNIT_ENT unitId;
    GT_BOOL  emptySliceEnable;
    GT_U32   slicesOnUnit;


    for (i = 0 ; pizzaDeviceStatePtr->devState.bobK.unitList[i] != CPSS_DXCH_PA_UNIT_UNDEFINED_E; i++)
    {
        unitId = pizzaDeviceStatePtr->devState.bobK.unitList[i];
        if (   unitId  == CPSS_DXCH_PA_UNIT_RXDMA_0_E
            || unitId  == CPSS_DXCH_PA_UNIT_TXDMA_0_E
            || unitId  == CPSS_DXCH_PA_UNIT_TXQ_0_E
            || unitId  == CPSS_DXCH_PA_UNIT_TX_FIFO_0_E)
        {
            if (unitId  == CPSS_DXCH_PA_UNIT_TXQ_0_E)
            {
                slicesOnUnit     = defPipeBandwithInGbps;
                emptySliceEnable = GT_TRUE;
            }
            else
            {
                slicesOnUnit     = reqBWGbps;
                emptySliceEnable = GT_FALSE;
            }
            st = pizzaArbiterCheckUnitsSlices(&pizzaDeviceStatePtr->devState.bobK.unitState[i],slicesOnUnit,reqBWGbps,emptySliceEnable);
            if (st != GT_OK)
            {
                return st;
            }
        }
    }
#if 0
    st = pizzaArbiterCheckUnitsSlices(&pizzaDeviceStatePtr->devState.bc2.rxDMA,reqBWGbps,reqBWGbps,GT_FALSE);
    if (st != GT_OK)
    {
        return st;
    }

    st = pizzaArbiterCheckUnitsSlices(&pizzaDeviceStatePtr->devState.bc2.txDMA,reqBWGbps,reqBWGbps,GT_FALSE);
    if (st != GT_OK)
    {
        return st;
    }

    st = pizzaArbiterCheckUnitsSlices(&pizzaDeviceStatePtr->devState.bc2.txFIFO,reqBWGbps,reqBWGbps,GT_FALSE);
    if (st != GT_OK)
    {
        return st;
    }


    st = pizzaArbiterCheckUnitsSlices(&pizzaDeviceStatePtr->devState.bc2.TxQ,defPipeBandwithInGbps,reqBWGbps,GT_TRUE);
    if (st != GT_OK)
    {
        return st;
    }
#endif
    return GT_OK;
}

GT_VOID pizzaArbiterPortSciceCountOnUnit
(
    IN  CPSS_DXCH_BOBCAT2_SLICES_PIZZA_ARBITER_STATE_STC *unitPtr,
    OUT GT_U32                                            portSliceCnt[PRV_CPSS_MAX_PP_PORTS_NUM_CNS]
)
{
    GT_U32                 i;
    GT_PHYSICAL_PORT_NUM   portNum;
    for (i = 0 ; i < unitPtr->totalConfiguredSlices; i++)
    {
        if (unitPtr->slice_enable[i] == GT_TRUE)
        {
            portNum = unitPtr->slice_occupied_by[i];
            portSliceCnt[portNum]++;
        }
    }

}


GT_STATUS pizzaArbiterCheckPortsWoTM
(
    GT_U8                 dev,
    CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC * pizzaDeviceStatePtr
)
{
    GT_STATUS st;
    CPSS_DXCH_DETAILED_PORT_MAP_STC  portMapShadow;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr = &portMapShadow;
    GT_PHYSICAL_PORT_NUM   portNum;
    GT_BOOL isInitilized;
    GT_U32  speedInMBit;
    GT_U32  slice2BeConf;
    static GT_U32 portSliceCnt[CPSS_DXCH_PA_UNIT_MAX_E][PRV_CPSS_MAX_PP_PORTS_NUM_CNS];
    GT_U32  maxPortNum;
    CPSS_DXCH_PA_UNIT_ENT unitId;
    GT_U32 i;

    st = cpssDxChCfgTableNumEntriesGet(dev, CPSS_DXCH_CFG_TABLE_PHYSICAL_PORT_E, /*OUT*/&maxPortNum);
    if (GT_OK != st)
    {
        return st;
    }


    cpssOsMemSet(portSliceCnt,0,sizeof(portSliceCnt));

    for (i = 0 ; pizzaDeviceStatePtr->devState.bobK.unitList[i] != CPSS_DXCH_PA_UNIT_UNDEFINED_E; i++)
    {
        unitId = pizzaDeviceStatePtr->devState.bobK.unitList[i];
        if (   unitId  == CPSS_DXCH_PA_UNIT_RXDMA_0_E
            || unitId  == CPSS_DXCH_PA_UNIT_TXDMA_0_E
            || unitId  == CPSS_DXCH_PA_UNIT_TXQ_0_E
            || unitId  == CPSS_DXCH_PA_UNIT_TX_FIFO_0_E)
        {
            pizzaArbiterPortSciceCountOnUnit(&pizzaDeviceStatePtr->devState.bobK.unitState[unitId],/*OUT*/portSliceCnt[unitId]);
        }
    }
#if 0
    pizzaArbiterPortSciceCountOnUnit(&(pizzaDeviceStatePtr->devState.bc2.rxDMA), portSliceCnt[CPSS_DXCH_PA_UNIT_RXDMA_0_E]  );
    pizzaArbiterPortSciceCountOnUnit(&(pizzaDeviceStatePtr->devState.bc2.rxDMA), portSliceCnt[CPSS_DXCH_PA_UNIT_TXDMA_0_E]  );
    pizzaArbiterPortSciceCountOnUnit(&(pizzaDeviceStatePtr->devState.bc2.txFIFO),portSliceCnt[CPSS_DXCH_PA_UNIT_TX_FIFO_0_E]);
    pizzaArbiterPortSciceCountOnUnit(&(pizzaDeviceStatePtr->devState.bc2.TxQ),   portSliceCnt[CPSS_DXCH_PA_UNIT_TXQ_0_E]    );
#endif

    for (portNum = 0; portNum < maxPortNum; portNum++)
    {
        st = cpssDxChPortPhysicalPortDetailedMapGet(dev,portNum,/*OUT*/portMapShadowPtr);
        if (st != GT_OK)
        {
            return st;
        }
        if (portMapShadowPtr->valid == GT_TRUE)
        {
            if (portMapShadowPtr->portMap.mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
            {
                st = prvCpssDxChPortDynamicPAPortSpeedDBGet(dev,portNum,&isInitilized,&speedInMBit);
                if (st != GT_OK)
                {
                    return st;
                }

                if (isInitilized == GT_FALSE)
                {
                    speedInMBit = 0;
                }
                slice2BeConf = speedInMBit/1000;
                /*---------------------*/
                /*  rx DMA             */
                /*---------------------*/
                if (portSliceCnt[CPSS_DXCH_PA_UNIT_RXDMA_0_E][portMapShadowPtr->portMap.rxDmaNum] != slice2BeConf)
                {
                    return GT_FAIL;
                }
                /*---------------------*/
                /* tx DMA */
                /*---------------------*/
                if (portSliceCnt[CPSS_DXCH_PA_UNIT_TXDMA_0_E][portMapShadowPtr->portMap.txDmaNum] != slice2BeConf)
                {
                    return GT_FAIL;
                }
                /*---------------------*/
                /* tx FIFO */
                /*---------------------*/
                if (portSliceCnt[CPSS_DXCH_PA_UNIT_TXDMA_0_E][portMapShadowPtr->portMap.txDmaNum] != slice2BeConf)
                {
                    return GT_FAIL;
                }
                /*---------------------*/
                /* tx Q */
                /*---------------------*/
                if (portSliceCnt[CPSS_DXCH_PA_UNIT_TXDMA_0_E][portMapShadowPtr->portMap.txqNum] != slice2BeConf)
                {
                    return GT_FAIL;
                }
            }
        }
    }


    return GT_OK;
}

GT_STATUS pizzaArbiterCheckWoTM
(
    GT_U8  dev,
    GT_U32 defPipeBandwithInGbps,
    GT_U32 reqBWGbps
)
{
    GT_STATUS st;
    static CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC pizzaDeviceState;

    st = cpssDxChPortPizzaArbiterDevStateGet(dev,0, /*OUT*/&pizzaDeviceState);
    if (st != GT_OK)
    {
        return st;
    }

    st = pizzaArbiterCheckUnitsWoTM(reqBWGbps,defPipeBandwithInGbps,&pizzaDeviceState);
    if (st != GT_OK)
    {
        return st;
    }

    st = pizzaArbiterCheckPortsWoTM(dev,&pizzaDeviceState);
    if (st != GT_OK)
    {
        return st;
    }

    return GT_OK;
}



GT_U32  PA_UT_VAR_N_DEBUG = 0;
GT_VOID utPADebugVarNSet(GT_U32 combinationN)
{
    PA_UT_VAR_N_DEBUG = combinationN;
}


typedef struct
{
    GT_U32                        speedGbps4PA;
    CPSS_PORT_SPEED_ENT           speedEnm;
    CPSS_PORT_INTERFACE_MODE_ENT  ifMode;
    PORTCAPACITY_STC             *portList[100];
}UT_PA_SpeedPortSupportList_STC;

typedef struct
{
    PORTCAPACITY_STC               portArr[PRV_CPSS_MAX_PP_PORTS_NUM_CNS];
    UT_PA_SpeedPortSupportList_STC speedPortSupportList[CPSS_PORT_SPEED_NA_E];
    GT_BOOL                        useOfEmptySlicesEn;
    /* processing data */
    GT_U32                         usedSpeedN;
    GT_U32                         generatedPortNArr[CPSS_PORT_SPEED_NA_E];
}UT_PA_WS_STC;


typedef struct
{
    GT_U32                        speedGbps4PA;
    CPSS_PORT_SPEED_ENT           speedEnm;
    CPSS_PORT_INTERFACE_MODE_ENT  ifMode;
    GT_PHYSICAL_PORT_NUM          portList[100];
}UT_PA_SPEED_PORT_SUPPORT_INIT_LIST_STC;

typedef struct
{
    UT_PA_SPEED_PORT_SUPPORT_INIT_LIST_STC  speedPortSupportList[CPSS_PORT_SPEED_NA_E];
    GT_BOOL                                 useOfEmptySlicesEn;
}UT_PA_WS_INIT_STC;

#define UT_INVALID_PORT       ((GT_U32)(~0))

GT_VOID UT_PA_WS_Init
(
    OUT UT_PA_WS_STC      *ut_pa_wsPtr,
    IN  UT_PA_WS_INIT_STC *ut_pa_initPtr
)
{
    GT_U32 i;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_U32 speedIdx;
    GT_U32 portIdx;


    cpssOsMemSet(ut_pa_wsPtr,0,sizeof(*ut_pa_wsPtr));
    /* basic init of ws to empty one */
    for (i = 0; i < sizeof(ut_pa_wsPtr->portArr)/sizeof(ut_pa_wsPtr->portArr[0]); i++)
    {
        ut_pa_wsPtr->portArr[i].portNum = i;
        ut_pa_wsPtr->portArr[i].isUsed  = GT_FALSE;
    }

    for (speedIdx = 0; speedIdx < sizeof(ut_pa_wsPtr->speedPortSupportList)/sizeof(ut_pa_wsPtr->speedPortSupportList[0]); speedIdx++)
    {
        ut_pa_wsPtr->speedPortSupportList[speedIdx].speedGbps4PA = 0;
        ut_pa_wsPtr->speedPortSupportList[speedIdx].speedEnm     = CPSS_PORT_SPEED_NA_E;
        ut_pa_wsPtr->speedPortSupportList[speedIdx].ifMode       = CPSS_PORT_INTERFACE_MODE_NA_E;
    }
    /* basic init according to user info  */
    for (speedIdx = 0; ut_pa_initPtr->speedPortSupportList[speedIdx].speedGbps4PA != 0; speedIdx++)
    {
        ut_pa_wsPtr->usedSpeedN++;
        ut_pa_wsPtr->speedPortSupportList[speedIdx].speedGbps4PA = ut_pa_initPtr->speedPortSupportList[speedIdx].speedGbps4PA;
        ut_pa_wsPtr->speedPortSupportList[speedIdx].speedEnm     = ut_pa_initPtr->speedPortSupportList[speedIdx].speedEnm;
        ut_pa_wsPtr->speedPortSupportList[speedIdx].ifMode       = ut_pa_initPtr->speedPortSupportList[speedIdx].ifMode;
        for (portIdx = 0; ut_pa_initPtr->speedPortSupportList[speedIdx].portList[portIdx] != UT_INVALID_PORT; portIdx++)
        {
            portNum = ut_pa_initPtr->speedPortSupportList[speedIdx].portList[portIdx];
            ut_pa_wsPtr->speedPortSupportList[speedIdx].portList[portIdx] = &ut_pa_wsPtr->portArr[portNum];
        }
    }
    ut_pa_wsPtr->useOfEmptySlicesEn = ut_pa_initPtr->useOfEmptySlicesEn;
}

GT_VOID UT_PA_WS_Clean
(
    INOUT UT_PA_WS_STC      *ut_pa_wsPtr
)
{
    GT_U32 i;
    for (i = 0; i < sizeof(ut_pa_wsPtr->portArr)/sizeof(ut_pa_wsPtr->portArr[0]); i++)
    {
        ut_pa_wsPtr->portArr[i].isUsed  = GT_FALSE;

    }
    for (i = 0; i < sizeof(ut_pa_wsPtr->generatedPortNArr)/sizeof(ut_pa_wsPtr->generatedPortNArr[0]); i++)
    {
        ut_pa_wsPtr->generatedPortNArr[i] = 0;
    }
}

GT_U32 UT_PA_WS_SpeedNGet
(
    INOUT UT_PA_WS_STC      *ut_pa_wsPtr

)
{
    return ut_pa_wsPtr->usedSpeedN;
}

GT_U32 UT_PA_WS_SpeedGbps4PAGet
(
    INOUT UT_PA_WS_STC                  *ut_pa_wsPtr,
    IN    GT_U32                         idx,
    OUT   CPSS_PORT_SPEED_ENT           *speedEnmPtr,
    OUT   CPSS_PORT_INTERFACE_MODE_ENT  *ifModePtr

)
{
    if (idx < ut_pa_wsPtr->usedSpeedN)
    {
        if (speedEnmPtr != NULL)
        {
            *speedEnmPtr = ut_pa_wsPtr->speedPortSupportList[idx].speedEnm;
        }
        if (ifModePtr != NULL)
        {
            *ifModePtr = ut_pa_wsPtr->speedPortSupportList[idx].ifMode;
        }
        return ut_pa_wsPtr->speedPortSupportList[idx].speedGbps4PA;
    }
    else
    {
        if (speedEnmPtr != NULL)
        {
            *speedEnmPtr = CPSS_PORT_SPEED_NA_E;
        }
        if (ifModePtr != NULL)
        {
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_NA_E;
        }
        return 0;
    }
}

PORTCAPACITY_STC ** UT_PA_WS_SpeedPortListGet
(
    INOUT UT_PA_WS_STC                  *ut_pa_wsPtr,
    IN    GT_U32                         idx
)
{
    if (idx < ut_pa_wsPtr->usedSpeedN)
    {
        return &ut_pa_wsPtr->speedPortSupportList[idx].portList[0];
    }
    else
    {
        return (PORTCAPACITY_STC **)NULL;
    }
}

GT_STATUS  UT_PA_WS_SpeedGeneratedPortNSet
(
    INOUT UT_PA_WS_STC                  *ut_pa_wsPtr,
    IN    GT_U32                         idx,
    IN    GT_U32                         generatedPortN
)
{
    if (idx < ut_pa_wsPtr->usedSpeedN)
    {
        ut_pa_wsPtr->generatedPortNArr[idx] = generatedPortN;
        return GT_OK;
    }
    else
    {
        return GT_BAD_PARAM;
    }
}

GT_U32 UT_PA_WS_SpeedGeneratedPortNGet
(
    INOUT UT_PA_WS_STC                  *ut_pa_wsPtr,
    IN    GT_U32                         idx
)
{
    if (idx < ut_pa_wsPtr->usedSpeedN)
    {
        return ut_pa_wsPtr->generatedPortNArr[idx];
    }
    else
    {
        return (GT_U32)(~0);
    }
}



UT_PA_WS_STC      ut_pa_ws;

UT_PA_WS_INIT_STC bc2_ut_pa_init =
{
    {
        {
            40, CPSS_PORT_SPEED_40000_E, CPSS_PORT_INTERFACE_MODE_SR_LR_E
           ,{
                48,             52,             56,             80, UT_INVALID_PORT
            }
        }
       ,{
            20, CPSS_PORT_SPEED_20000_E, CPSS_PORT_INTERFACE_MODE_SR_LR_E
           ,{
                48,     50,     52,     54,     56,     58,     80, UT_INVALID_PORT
            }
        }
       ,{
            10, CPSS_PORT_SPEED_10000_E, CPSS_PORT_INTERFACE_MODE_SR_LR_E
           ,{
                48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 80, UT_INVALID_PORT
            }
        }
       ,{
            1,CPSS_PORT_SPEED_1000_E, CPSS_PORT_INTERFACE_MODE_QSGMII_E
            ,{
                 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11,
                12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
                24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
                36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
                UT_INVALID_PORT
            }
        }
       ,{
            0, CPSS_PORT_SPEED_NA_E, CPSS_PORT_INTERFACE_MODE_NA_E
           ,{
                UT_INVALID_PORT
            }
       }

    }
    /* useOfEmptySlicesEn */,GT_FALSE
};


GT_VOID cpssDxChBobcat2PortModeSpeedSetUT(GT_VOID)
{
    GT_STATUS st  = GT_OK;
    GT_U8     dev = 0;
    GT_U32                   defPipeBandwithInGbps;
    CPSS_DXCH_MIN_SPEED_ENT  defMinimalPortSpeedMBps;

    static CPSS_PORT_SPEED_ENT               speedArr[PRV_CPSS_MAX_PP_PORTS_NUM_CNS];
    static CPSS_PORT_INTERFACE_MODE_ENT     ifModeArr[PRV_CPSS_MAX_PP_PORTS_NUM_CNS];
    static GT_BOOL                        toDeleteArr[PRV_CPSS_MAX_PP_PORTS_NUM_CNS];
    GT_U32 combinationIdx;
    GT_BOOL isTmPresent;
    GT_BOOL isCpuPresent;
    GT_U32  combinationN = 10000;

    GT_U32 startTime;
    GT_U32 endTime;
    static ListOfConfiguredPorts_STC confPortList;


    /* state that max 2 device will be tested for aging . */

    prvUtfSetMaxIterationsOnDevice(2);


    if (PA_UT_VAR_N_DEBUG != 0)
    {
        combinationN = PA_UT_VAR_N_DEBUG;
    }
    else
    {

        #ifdef ASIC_SIMULATION
                combinationN = 1000;
        #else
                combinationN = 100;
        #endif

        if (GT_TRUE == prvUtfSkipNonBaselineTestsFlagGet())
        {
            combinationN = 100;
        }
    }

    /* Prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
        UTF_CH2_E | UTF_CH3_E |
        UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E |UTF_LION2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        UT_PA_WS_Init(/*OUT*/&ut_pa_ws,/*IN*/&bc2_ut_pa_init);

        startTime = cpssOsTime();


        st = utIsThereTM(dev,&isTmPresent,&isCpuPresent);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* get default values */
        st = cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionGet(dev, /*OUT*/&defPipeBandwithInGbps, /*OUT*/&defMinimalPortSpeedMBps);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = utRemoveAllEthernetPorts(dev,PRV_CPSS_MAX_PP_PORTS_NUM_CNS,/*OUT*/&speedArr[0], &ifModeArr[0],&toDeleteArr[0]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        if (isTmPresent == GT_FALSE)
        {
            st = cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionSet(dev,defPipeBandwithInGbps,CPSS_DXCH_MIN_SPEED_1000_Mbps_E);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            utilCpssDxChPortPizzaArbiterHighSpeedPortCheck(
                dev, MIN_HIGH_SPEED_CNS);

            cpssOsSrand(0);
            for (combinationIdx = 0 ; combinationIdx < combinationN; combinationIdx++)
            {
                GT_U32 BW2Generate;
                GT_U32 accBW2;
                GT_U32 idx;
                GT_U32 speedN;
                GT_U32 speedIdx;
                GT_U32 speedGbps;
                CPSS_PORT_SPEED_ENT           speedEnm;
                CPSS_PORT_INTERFACE_MODE_ENT  ifMode;
                PORTCAPACITY_STC**            portListPtr;
                GT_U32                        numOfGenPorts;




                UT_PA_WS_Clean(&ut_pa_ws);

                ListOfConfiguredPortsInit(&confPortList);

                BW2Generate = randi(defPipeBandwithInGbps);
                if (BW2Generate == 0)
                {
                    BW2Generate = defPipeBandwithInGbps;  /* 1G for CPU port */
                }
                BW2Generate = BW2Generate-1*isCpuPresent;
                /* generate 40G */
                speedN = UT_PA_WS_SpeedNGet(&ut_pa_ws);

                accBW2 = 0;
                for (speedIdx = 0; speedIdx < speedN; speedIdx++)
                {

                    speedGbps = UT_PA_WS_SpeedGbps4PAGet(&ut_pa_ws,speedIdx,/*OUT*/&speedEnm,&ifMode);
                    portListPtr = UT_PA_WS_SpeedPortListGet(&ut_pa_ws,speedIdx);

                    st = generatePortsAccordingToAvailableBW(dev,/*INOUT*/&BW2Generate,/*IN*/speedGbps,/*IN*/portListPtr,/*OUT*/&numOfGenPorts,&confPortList, CPSS_PORT_INTERFACE_MODE_SR_LR_E, CPSS_PORT_SPEED_40000_E);
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, combinationIdx, speedGbps);

                    st = UT_PA_WS_SpeedGeneratedPortNSet(&ut_pa_ws,speedIdx,numOfGenPorts);
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, combinationIdx, speedGbps);

                    accBW2 += speedGbps*numOfGenPorts;
                }
                if (PA_UT_BWRES_SET_DEBUG == 1)
                {
                    cpssOsPrintf("\nVariant %4d:",combinationIdx);
                    for (speedIdx = 0; speedIdx < speedN; speedIdx++)
                    {
                        speedGbps     = UT_PA_WS_SpeedGbps4PAGet(&ut_pa_ws,speedIdx,/*OUT*/NULL,NULL);
                        numOfGenPorts = UT_PA_WS_SpeedGeneratedPortNGet(&ut_pa_ws,speedIdx);
                        cpssOsPrintf("\n    %2dG = %2d",speedGbps,numOfGenPorts);
                    }
                }
                else
                {
                    if (combinationIdx %500 == 0)
                    {
                        cpssOsPrintf("\n");
                    }
                    if (combinationIdx % 20 == 0)
                    {
                        cpssOsPrintf(" %4d",combinationIdx);
                    }
                }
                st = pizzaArbiterCheckWoTM(dev,defPipeBandwithInGbps,accBW2);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


                if (PA_UT_BWRES_SET_DEBUG == 1)
                {
                    cpssOsPrintf("\n    Remove ports");
                }
                for (idx = 0 ; idx < confPortList.cnt; idx++)
                {
                    if (PA_UT_BWRES_SET_DEBUG == 1)
                    {
                        cpssOsPrintf("\n        port %3d speed %s if = %s ... ",
                            confPortList.portNumArr[idx],CPSS_SPEED_2_STR(confPortList.speedArr[idx]),CPSS_IF_2_STR(confPortList.ifModeArr[idx]));
                    }
                    st = utRemovePort(dev,confPortList.portNumArr[idx],confPortList.ifModeArr[idx],confPortList.speedArr[idx]);
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, combinationIdx, confPortList.portNumArr[idx]);
                    if (PA_UT_BWRES_SET_DEBUG == 1)
                    {
                        cpssOsPrintf("OK");
                    }
                }
                if (PA_UT_BWRES_SET_DEBUG == 1)
                {
                    endTime = cpssOsTime();
                    cpssOsPrintf("\n    %3d variants takes %d sec",combinationIdx+1,endTime-startTime);
                }
            }
        }

        /* restore default values */
        st = cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionSet(dev, defPipeBandwithInGbps, defMinimalPortSpeedMBps);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = utRestoreAllPorts(dev,&speedArr[0], &ifModeArr[0],&toDeleteArr[0]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        endTime = cpssOsTime();
        cpssOsPrintf("\nTest takes %d sec\n",endTime-startTime);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
        UTF_CH2_E | UTF_CH3_E |
        UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E |UTF_LION2_E | UTF_FALCON_E);

    /* go over all non active devices */
    defPipeBandwithInGbps = 120;
    defMinimalPortSpeedMBps = CPSS_DXCH_MIN_SPEED_500_Mbps_E;

    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionSet(dev, defPipeBandwithInGbps, defMinimalPortSpeedMBps);
        UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_OK, st);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionSet(dev, defPipeBandwithInGbps, defMinimalPortSpeedMBps);
    UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_OK, st);

}

CPSS_PORT_SPEED_ENT prv_speed_list[] =
{
    CPSS_PORT_SPEED_1000_E,     /* 2 */
    CPSS_PORT_SPEED_10000_E,    /* 3 */
    CPSS_PORT_SPEED_12000_E,    /* 4 */
    CPSS_PORT_SPEED_2500_E,     /* 5 */
    CPSS_PORT_SPEED_5000_E,     /* 6 */
    CPSS_PORT_SPEED_20000_E,    /* 8 */
    CPSS_PORT_SPEED_40000_E,    /* 9 */
    CPSS_PORT_SPEED_11800_E,    /* 16  */ /*used in combination with CPSS_PORT_INTERFACE_MODE_XHGS_E or CPSS_PORT_INTERFACE_MODE_XHGS_SR_E */
    CPSS_PORT_SPEED_47200_E     /* 17  */ /*used in combination with CPSS_PORT_INTERFACE_MODE_XHGS_E or CPSS_PORT_INTERFACE_MODE_XHGS_SR_E */

};

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPizzaArbiterIfConfigSet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_SPEED_ENT  speed
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortPizzaArbiterIfConfigSet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (DxCh)
    1.1.1. Call with correct speeds.
    Expected: GT_OK.
    1.1.2. Call with wrong enum values speed.
    Expected: GT_BAD_PARAM.
*/

    GT_STATUS              st       = GT_OK;

    GT_U8                  dev;
    GT_U8                  i;
    GT_PHYSICAL_PORT_NUM   port     = HWINIT_VALID_PORTNUM_CNS;
    CPSS_PORT_SPEED_ENT    speed    = CPSS_PORT_SPEED_1000_E;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_XCAT_E | UTF_XCAT2_E | UTF_XCAT3_E |
                                           UTF_LION_E | UTF_LION2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {

            /*
               1.1.1. Call with correct speeds.
               Expected: NOT GT_BAD_PARAM.
            */
            for (i = 0 ; i < sizeof(prv_speed_list)/sizeof(prv_speed_list[0]); i++)
            {
                speed = prv_speed_list[i];
                st = cpssDxChPortPizzaArbiterIfConfigSet(dev, port, speed);
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, speed);
            }

            /*
               1.1.2. Call with wrong enum values speed
               Expected: GT_BAD_PARAM
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChPortPizzaArbiterIfConfigSet
                                (dev, port, speed),
                                speed);
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        speed = CPSS_PORT_SPEED_1000_E;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortPizzaArbiterIfConfigSet(dev, port, speed);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortPizzaArbiterIfConfigSet(dev, port, speed);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    speed = CPSS_PORT_SPEED_1000_E;
    port = HWINIT_VALID_PORTNUM_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT2_E | UTF_XCAT3_E | UTF_LION_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPizzaArbiterIfConfigSet(dev, port, speed);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortPizzaArbiterIfConfigSet(dev, port, speed);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortResourcesConfigSet
(
    IN  GT_U8                          devNum,
    IN  GT_U8                          portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT   ifMode
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortResourcesConfigSet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (DxCh)
    1.1.1. Call with correct parameters ifMode and speed.
    Expected: GT_OK.
    1.1.2 Call with out of range ifMode.
    Expected: GT_BAD_PARAM.
    1.1.3 Call with out of range speed.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_U8       i;
    GT_PHYSICAL_PORT_NUM          port   = HWINIT_VALID_PORTNUM_CNS;
    CPSS_PORT_INTERFACE_MODE_ENT  ifMode = CPSS_PORT_INTERFACE_MODE_NA_E;
    CPSS_PORT_SPEED_ENT           speed  = CPSS_PORT_SPEED_NA_E;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_XCAT_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with with correct parameters ifMode and speed.
                Expected: NOT GT_BAD_PARAM.
            */
            st = cpssDxChPortInterfaceModeGet(dev, port, &ifMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPortInterfaceModeGet: %d, %d", dev, port);

            /*ifMode = CPSS_PORT_INTERFACE_MODE_GMII_E;*/
            for (i = 0 ; i < sizeof(prv_speed_list)/sizeof(prv_speed_list[0]); i++)
            {
                if ((
                    (prv_speed_list[i] == CPSS_PORT_SPEED_5000_E))||(prv_speed_list[i] == CPSS_PORT_SPEED_12000_E))
                {
                    continue;
                }
                speed = prv_speed_list[i];
                st = cpssDxChPortResourcesConfigSet(dev, port, ifMode, speed);

                if (CPSS_PORT_INTERFACE_MODE_NA_E == ifMode)
                {
                    UTF_VERIFY_EQUAL4_STRING_MAC(GT_BAD_PARAM, st,
                        "cpssDxChPortResourcesConfigSet: %d, %d, %d, %d", dev, port, ifMode, speed);
                }
                else
                {
                    UTF_VERIFY_NOT_EQUAL4_STRING_MAC(GT_BAD_PARAM, st,
                        "cpssDxChPortResourcesConfigSet: %d, %d, %d, %d", dev, port, ifMode, speed);
                }
            }

            /*
                1.1.2. Call with wrong enum values ifMode.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChPortResourcesConfigSet
                                    (dev, port, ifMode, speed),
                                    ifMode);
            /*
                1.1.3. Call with wrong enum values speed.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChPortResourcesConfigSet
                                    (dev, port, ifMode, speed),
                                    speed);
        }

        ifMode = CPSS_PORT_INTERFACE_MODE_SGMII_E;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortResourcesConfigSet(dev, port, ifMode, speed);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortResourcesConfigSet(dev, port, ifMode, speed);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    ifMode = CPSS_PORT_INTERFACE_MODE_SGMII_E;

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = HWINIT_VALID_PORTNUM_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_XCAT_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortResourcesConfigSet(dev, port, ifMode, speed);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortResourcesConfigSet(dev, port, ifMode, speed);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}

/*---------------------------------------------------------------------------*/
/*Falcon TxPizza data path unit test
  The test set port speed and check that the right values was configured
  check values:
  pizza slices number
  pizza slices configuration accordin to port speed, port mode and port Num
  pizza speed profile configuration according to port speed
  Check right values for invalid port or ifMode or speed or devNum
  */
UTF_TEST_CASE_MAC(prvCpssDxChPortTxFalconPizzaAndSpeedProfileSet)
{
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_U8       i,j;
    GT_PHYSICAL_PORT_NUM          port   = HWINIT_VALID_PORTNUM_CNS;
    CPSS_PORT_INTERFACE_MODE_ENT  ifMode = CPSS_PORT_INTERFACE_MODE_NA_E;
    CPSS_PORT_SPEED_ENT           speed  = CPSS_PORT_SPEED_NA_E;
    CPSS_PORT_INTERFACE_MODE_ENT  previfMode = CPSS_PORT_INTERFACE_MODE_NA_E;
    CPSS_PORT_SPEED_ENT           prevspeed  = CPSS_PORT_SPEED_NA_E;
    GT_U32    txPizzaSliceNumber = 33;
    GT_U32    txGlobalIndex; /*txdma , txfifo index */
    GT_U32    channelDpIndex;
    PRV_CPSS_SPEED_PROFILE_ENT    expectedSpeedProfile;
    GT_U32    expectedPizzaSliceForPort;
    GT_U32    PizzaSlicesNumber;
    PRV_CPSS_SPEED_PROFILE_ENT    txDPSpeedProfileConfigurations[9 /* 8 ports + 1 cpu ports*/] = {0};
    GT_U32    txDPPizzaConfigurations[9 /* 8 ports + 1 cpu ports*/] = {0};

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~(UTF_FALCON_E));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        st = prvCpssDxChFalconPortPizzaArbiterInit(dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
           /*
                1.1.1. Call cpssDxChPortInterfaceModeGet with correct parameters ifMode and speed.
                Expected: NOT GT_BAD_PARAM                                                       .
                For correct ifMode and speed, check that the right values was configured:                                                                                                                            .
                1. speed profile                                                                                                                                                                                        .
                2. number of slice in pizza                                                                                                                                                                                         .
                3. pizza size - max slice number                                                                                                                                                                                 .
                speed 400G(port 0),speed 200G (port 0, 4) speed 100G (port 0,2,4,6)                                                                                                                                                                                                                  .
                for now speed 200G, 400G and 100G ( 100G in 2 ports and not in 4 ports) are not supported. TBD :)                                                                                                                                                                                                                                                                                    .
                                                                                                                                                                                                                                                                                                                                                                                               .
            */
            st = prvCpssDxChGetTxIndexAndChannelIndexfromPortNum(dev, port, &txGlobalIndex, &channelDpIndex );
            if (channelDpIndex > 8)
            {
                continue;
            }

            st = cpssDxChPortInterfaceModeGet(dev, port, &ifMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPortInterfaceModeGet: %d, %d", dev, port);

            /* check for speed (G) that can be set to any port : 50,40,25,20,10,2.5,1 */
            j = 0;
            for (i = 0 ; i < sizeof(prv_speed_list)/sizeof(prv_speed_list[0]); i++)
            {
                if (!(
                     prv_speed_list[i] == CPSS_PORT_SPEED_10000_E)||(prv_speed_list[i] == CPSS_PORT_SPEED_50000_E) ||
                     (prv_speed_list[i] == CPSS_PORT_SPEED_20000_E) )
                    {
                    continue;
                    }
                /* 1,2.5,25,40,50*/
                speed = prv_speed_list[i];
                /*cpu port */
                if ((channelDpIndex == 8) && (speed != CPSS_PORT_SPEED_10000_E))
                {
                   continue;
                }
                st = prvCpssDxChTxPortPizzaArbiterMaxSliceSet(dev, txGlobalIndex, PRV_CPSS_DXCH_UNIT_TXDMA_E, txPizzaSliceNumber);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
                st = prvCpssDxChTxPortPizzaArbiterMaxSliceSet(dev, txGlobalIndex, PRV_CPSS_DXCH_UNIT_TX_FIFO_E, txPizzaSliceNumber);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

                if (j != 0)
                {
                    /* deleate allocate resource for the port*/
                    st = prvCpssDxChTxPortSpeedPizzaResourcesSet(dev, port, previfMode, prevspeed, GT_FALSE);
                    if (CPSS_PORT_INTERFACE_MODE_NA_E == previfMode)
                    {
                        UTF_VERIFY_EQUAL4_STRING_MAC(GT_NOT_INITIALIZED, st,
                        "cpssDxChPortResourcesConfigSet: %d, %d, %d, %d", dev, port, previfMode, prevspeed);
                    }
                    else
                    {
                        UTF_VERIFY_NOT_EQUAL4_STRING_MAC(GT_NOT_INITIALIZED, st,
                        "cpssDxChPortResourcesConfigSet: %d, %d, %d, %d", dev, port, previfMode, prevspeed);
                    }
                }

                /* set new resources for the port*/
                st = prvCpssDxChTxPortSpeedPizzaResourcesSet(dev, port, ifMode, speed,GT_TRUE);
                if ((CPSS_PORT_INTERFACE_MODE_NA_E == ifMode) && (channelDpIndex != 8) )
                {
                    UTF_VERIFY_EQUAL4_STRING_MAC(GT_NOT_INITIALIZED, st,
                        "cpssDxChPortResourcesConfigSet: %d, %d, %d, %d", dev, port, ifMode, speed);
                    continue;
                }
                else
                {
                    UTF_VERIFY_NOT_EQUAL4_STRING_MAC(GT_NOT_INITIALIZED, st,
                        "cpssDxChPortResourcesConfigSet: %d, %d, %d, %d", dev, port, ifMode, speed);
                }
                /*check that the configuration was correct*/
                /*get expected values*/
                st = prvCpssDxChTxPortSpeedProfileAndPizzaSliceGet(dev,port,ifMode,speed,&expectedPizzaSliceForPort,&expectedSpeedProfile);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

                /* get the configured values*/
                st = prvCpssDxChPortTxPizzaSizeGet(dev, txGlobalIndex,&PizzaSlicesNumber);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

                st = prvCpssDxChPortTxSpeedProfileGet(dev, txGlobalIndex, txDPSpeedProfileConfigurations );
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

                st = prvCpssDxChPortTxPizzaSliceGet(dev, txGlobalIndex, txDPPizzaConfigurations );
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

                /* check that the right values was configured: pizza slice num , speed profile , pizza size*/
                UTF_VERIFY_EQUAL4_STRING_MAC(txPizzaSliceNumber, PizzaSlicesNumber,
                         "prvCpssDxChPortTxPizzaSizeGet: %d, %d, %d, %d", dev, port, ifMode, speed);

                UTF_VERIFY_EQUAL4_STRING_MAC(expectedSpeedProfile, txDPSpeedProfileConfigurations[channelDpIndex],
                        "prvCpssDxChPortTxSpeedProfileGet: %d, %d, %d, %d", dev, port, ifMode, speed);

                UTF_VERIFY_EQUAL4_STRING_MAC(expectedPizzaSliceForPort, txDPPizzaConfigurations[channelDpIndex],
                        "prvCpssDxChPortTxPizzaSliceGet: %d, %d, %d, %d", dev, port, ifMode, speed);
                previfMode = ifMode;
                prevspeed = speed;
                j++;
            }
            st = prvCpssDxChTxPortSpeedPizzaResourcesSet(dev, port, previfMode, prevspeed, GT_FALSE);
            if ((CPSS_PORT_INTERFACE_MODE_NA_E == previfMode) && (channelDpIndex != 8))
            {
                UTF_VERIFY_EQUAL4_STRING_MAC(GT_NOT_INITIALIZED, st,
                                             "cpssDxChPortResourcesConfigSet: %d, %d, %d, %d", dev, port, previfMode, prevspeed);
            }
            else
            {
                UTF_VERIFY_NOT_EQUAL4_STRING_MAC(GT_NOT_INITIALIZED, st,
            "cpssDxChPortResourcesConfigSet: %d, %d, %d, %d", dev, port, previfMode, prevspeed);
            }
            j = 0;
        }

        ifMode = CPSS_PORT_INTERFACE_MODE_SGMII_E;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = prvCpssDxChTxPortSpeedPizzaResourcesSet(dev, port, ifMode, speed, GT_TRUE);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = prvCpssDxChTxPortSpeedPizzaResourcesSet(dev, port, ifMode, speed, GT_TRUE);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    ifMode = CPSS_PORT_INTERFACE_MODE_SGMII_E;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = HWINIT_VALID_PORTNUM_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~(UTF_FALCON_E));

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = prvCpssDxChTxPortSpeedPizzaResourcesSet(dev, port, ifMode, speed, GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = prvCpssDxChTxPortSpeedPizzaResourcesSet(dev, port, ifMode, speed, GT_TRUE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}


#ifdef falconDebugPizza
/*---------------------------------------------------------------------------*/
/* prvCpssDxChPortTxFalconPizzaAndSpeedProfileDebug
 Falcon TxPizza data path debug test
 the test go over all posible options for 400G DP configuration.
 2 steps:
 1) set all DP in the same configuration,
 for each dp check pizza size, port speed profile and port pizza slice bitmap
 go over all dp again and remove the cunfiguration

 2) for each DP set diffrent 400G configuration option
 for each dp check pizza size, port speed profile and port pizza slice bitmap
 go over all dp again and remove the cunfiguration
  */
UTF_TEST_CASE_MAC(prvCpssDxChPortTxFalconPizzaAndSpeedProfileDebug)
{
    /* NOTE: this debug test relevant only when using
       prvCpssDxChPortDebugFalconSerDesFromPortSpeedGet in prvCpssDxChTxPortSpeedPizzaResourcesSet*/
    GT_STATUS                    st  = GT_OK;
    GT_U8                        dev;
    GT_U32                       ii;
    GT_U32                       dpModeOptionIndex;
    GT_U32                       DPCoreNum;
    GT_PHYSICAL_PORT_NUM         portNum;
    GT_U32                       dpIndex;   /*DP index */
    GT_U32                       portIndex; /*local channel index */
    CPSS_PORT_INTERFACE_MODE_ENT ifMode = CPSS_PORT_INTERFACE_MODE_NA_E; /* in this debug test the serdes defined according to port speed only*/
    CPSS_PORT_SPEED_ENT          portSpeed;
    GT_U32                       PizzaSlicesNumber;  /*pizza size*/
    GT_U32                       *expectedDPPizzaSliceBitMap; /* per port slices bit map   */
    GT_U32                       txDPPizzaSliceBitMap[8] = {0};       /* per port slices bit map   */
    PRV_CPSS_SPEED_PROFILE_ENT   *expectedDPSpeedProfile;           /*per port speed profile */
    PRV_CPSS_SPEED_PROFILE_ENT   txDPSpeedProfile[9];                 /*per port speed profile include cpu port */

    /* cpu */
    CPSS_PORT_SPEED_ENT          cpuPortSpeed = CPSS_PORT_SPEED_10000_E; /* Fixed CPU port speed 10G in falcon*/
    PRV_CPSS_SPEED_PROFILE_ENT   cpuPortSpeedProfile = PRV_CPSS_SPEED_PROFILE_7_E;
    GT_U32                       cpuPortsInDP[MAX_DP_CNS] = {0};
    GT_U32                       cpuSliceNumber = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~(UTF_FALCON_E));

    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* number of DP cores in test*/
        DPCoreNum = PRV_CPSS_PP_MAC(dev)->multiPipe.numOfTiles * MAX_DP_PER_TILE_CNS;

        PRV_UTF_LOG0_MAC("\n=======  stage 1: go over all DP for all 400G modes options  =======\n");

        /* reset all pizza slices to invalidate*/
        st = prvCpssDxChFalconPortPizzaArbiterInit(dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*1. set all DP in the same way*/
        /* go over all options to configured 400G DP core*/
        for ( ii = 0 ; ii < portsModes400GOptionsNum; ii++)
        {
            /* get expected pizza slices and profile in DP for current option. not include the cpu port!*/
            expectedDPPizzaSliceBitMap = falconPortsModes400GOptionsExpectedPizzaSlicesBitMap[ii];
            expectedDPSpeedProfile = falconPortsModes400GOptionsExpectedSpeedProfile[ii];

            st = prvUtfNextTxDmaPortReset(&portNum, dev);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* 1.1. For all active devices go over all available physical ports. */
            while(GT_OK == prvUtfNextTxDmaPortGet(&portNum, GT_TRUE))
            {
                /*get dp index, and channel index */
                st = prvCpssDxChGetTxIndexAndChannelIndexfromPortNum(dev, portNum, &dpIndex, &portIndex );

                PRV_UTF_LOG4_MAC("try to set option %d, portNum %d, dpIndex %d, portIndex %d\n",ii,portNum,dpIndex,portIndex);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

                if (portIndex == PRV_DIP6_TX_CPU_PORT_NUM_CNS ) /* cpu port*/
                {
                    if (cpuPortsInDP[dpIndex] == PRV_DIP6_CPU_PORT_NUM_OF_SLICE)
                    {
                        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, GT_FAIL);
                    }
                    cpuPortsInDP[dpIndex]++;
                    portSpeed = cpuPortSpeed; /* Fixed CPU port speed 10G*/
                }
                else
                if (falconPortsModes400GOptions[ii][portIndex] == CPSS_PORT_SPEED_NA_E)
                {
                    continue;
                }
                portSpeed = falconPortsModes400GOptions[ii][portIndex];
                /* set pizza slices configurations and speed profile*/
                st =  prvCpssDxChTxPortSpeedPizzaResourcesSet(dev, portNum, ifMode, portSpeed, GT_TRUE);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,"prvCpssDxChPortDebugfalconPizzaTest: prvCpssDxChTxPortSpeedPizzaResourcesSet failed, port %d, speed %d\n",portNum, portSpeed);
            }

            /* 1.2 go over all DP in tile check that all dp configured right*/
            for (dpIndex = 0 ; dpIndex < DPCoreNum; dpIndex++ )
            {
                /*check pizza size*/
                st = prvCpssDxChPortTxPizzaSizeGet(dev, dpIndex, &PizzaSlicesNumber);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

                UTF_VERIFY_EQUAL2_STRING_MAC(PizzaSlicesNumber, TXD_PIZZA_SLICE_NUM,"prvCpssDxChPortDebugFalconPortSpeed: pizza slice number: %d, %d", PizzaSlicesNumber, TXD_PIZZA_SLICE_NUM);

                /*get configured speed profile */
                st = prvCpssDxChPortTxSpeedProfileGet(dev, dpIndex, txDPSpeedProfile );
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

                /* get DP pizza slices bit map*/
                st = prvCpssDxChTxDPPizzaSliceBitMapGet(dev, dpIndex, txDPPizzaSliceBitMap);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"prvCpssDxChPortDebugFalconPortSpeed call prvCpssDxChTxDPPizzaSliceBitMapGet dp %d\n", dpIndex);

                /* go over all ports and compare slices bit map   and speed profile*/
                for (portIndex = 0 ; portIndex < FALCON_PORTS_PER_DP_CNS ; portIndex++)
                {
                    /* app demo dp 15 have only 3 ports in falcon 6.4*/
                    if ((dpIndex == 15) && (portIndex > 2))
                    {
                        continue;
                    }

                    PRV_UTF_LOG5_MAC("\n=======  stage 1 : option %d, go over dp %d , port %d, pizza bit map [0x%8.8x] speed profile %d  =======\n",ii, dpIndex, portIndex,txDPPizzaSliceBitMap[portIndex],txDPSpeedProfile[portIndex]);
                    UTF_VERIFY_EQUAL5_STRING_MAC(expectedDPPizzaSliceBitMap[portIndex], txDPPizzaSliceBitMap[portIndex],"prvCpssDxChPortDebugFalconPortSpeed: option %d: port %d in  have slice bit map [0x%8.8x] different than expected [0x%8.8x] for speed %d\n",ii, portIndex, txDPPizzaSliceBitMap[portIndex],expectedDPPizzaSliceBitMap[portIndex],falconPortsModes400GOptions[ii][portIndex]);

                    if (expectedDPSpeedProfile[portIndex] != PRV_CPSS_PORT_SPEED_PROFILE_NA_E)
                    {
                        UTF_VERIFY_EQUAL4_STRING_MAC(expectedDPSpeedProfile[portIndex], txDPSpeedProfile[portIndex],"prvCpssDxChPortDebugFalconPortSpeed: port %d have speed profile %d different than expected %d for speed %d", portIndex, txDPSpeedProfile[portIndex], expectedDPSpeedProfile[portIndex],falconPortsModes400GOptions[ii][portIndex]);
                    }
                }

                /* check cpu port configuration speed profile and bit map*/
                if (cpuPortsInDP[dpIndex] > 0)
                {
                    UTF_VERIFY_EQUAL3_STRING_MAC(cpuPortSpeedProfile, txDPSpeedProfile[PRV_DIP6_TX_CPU_PORT_NUM_CNS],"cpu port in DP %d speed profile is %d, excepted %d", dpIndex, txDPSpeedProfile[PRV_DIP6_TX_CPU_PORT_NUM_CNS], cpuPortSpeedProfile);
                    st = prvCpssDxChTxCpuPizzaGet(dev, dpIndex, &cpuSliceNumber);
                    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
                    UTF_VERIFY_EQUAL2_STRING_MAC(cpuSliceNumber, PRV_DIP6_CPU_PORT_NUM_OF_SLICE,"cpu port in dp %d slice number is %d\n",dpIndex,cpuSliceNumber);
                }
            }

            st = prvUtfNextTxDmaPortReset(&portNum, dev);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* 1.3. For all active devices go over all available physical ports. and set pizza slices to invalid */
            while(GT_OK == prvUtfNextTxDmaPortGet(&portNum, GT_TRUE))
            {
                st = prvCpssDxChGetTxIndexAndChannelIndexfromPortNum(dev, portNum, &dpIndex, &portIndex );
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
                if (portIndex == PRV_DIP6_TX_CPU_PORT_NUM_CNS )
                {
                    portSpeed = cpuPortSpeed; /* Fixed CPU port speed 10G*/
                }
                else
                if (falconPortsModes400GOptions[ii][portIndex] == CPSS_PORT_SPEED_NA_E)
                {
                    continue;
                }
                cpuPortsInDP[dpIndex] = 0;
                st = prvCpssDxChTxPortSpeedPizzaResourcesSet(dev, portNum, CPSS_PORT_INTERFACE_MODE_NA_E, falconPortsModes400GOptions[ii][portIndex], GT_FALSE);
                UTF_VERIFY_EQUAL2_STRING_MAC(st,GT_OK,"cpssDxChPortDebugFalconPortSpeedFromPortNumGet call prvCpssDxChPortDebugFalconPizzaTest with port number %d, speed %d failed\n", portIndex,falconPortsModes400GOptions[ii][portIndex]);
            }
        }

        PRV_UTF_LOG0_MAC("\n=======  stage 2 : go over all DP and set different 400G modes options  =======\n");

        /*2. set different speed to each DP*/

        st = prvUtfNextTxDmaPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 2.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextTxDmaPortGet(&portNum, GT_TRUE))
        {
            /*get txindex, and channel index */
            st = prvCpssDxChGetTxIndexAndChannelIndexfromPortNum(dev, portNum, &dpIndex, &portIndex);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            dpModeOptionIndex = (dpIndex < portsModes400GOptionsNum ? dpIndex : portsModes400GOptionsNum%dpIndex );

            if (portIndex == PRV_DIP6_TX_CPU_PORT_NUM_CNS )
            {
                /* cpu ports in dp 0,1,2,3 are not supported*/
                if (cpuPortsInDP[dpIndex] == 1)
                {
                    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, GT_FAIL);
                }
                cpuPortsInDP[dpIndex]++;
                portSpeed = cpuPortSpeed /* Fixed CPU port speed 10G*/ ;
            }
            else
            if (falconPortsModes400GOptions[dpModeOptionIndex][portIndex] == CPSS_PORT_SPEED_NA_E)
            {
                continue;
            }
            portSpeed = falconPortsModes400GOptions[dpModeOptionIndex][portIndex];
            /* set pizza slices configurations and speed profile*/
            st =  prvCpssDxChTxPortSpeedPizzaResourcesSet(dev, portNum, ifMode, portSpeed, GT_TRUE);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,"prvCpssDxChPortDebugfalconPizzaTest: prvCpssDxChTxPortSpeedPizzaResourcesSet failed, port %d, speed %d\n",portNum, portSpeed);
        }

        /* 2.2. go over all DP in tile check that all dp configured right*/
        for (dpIndex = 0 ; dpIndex < DPCoreNum; dpIndex++ )
        {
            dpModeOptionIndex = (dpIndex < portsModes400GOptionsNum ? dpIndex : portsModes400GOptionsNum%dpIndex );

            /* get expected pizza slices and speed profile in DP for this option*/
            expectedDPPizzaSliceBitMap = falconPortsModes400GOptionsExpectedPizzaSlicesBitMap[dpModeOptionIndex];
            expectedDPSpeedProfile = falconPortsModes400GOptionsExpectedSpeedProfile[dpModeOptionIndex];

            /*check pizza size*/
            st = prvCpssDxChPortTxPizzaSizeGet(dev, dpIndex, &PizzaSlicesNumber);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            UTF_VERIFY_EQUAL2_STRING_MAC(PizzaSlicesNumber, TXD_PIZZA_SLICE_NUM,"prvCpssDxChPortDebugFalconPortSpeed: pizza slice number: %d, %d", PizzaSlicesNumber, TXD_PIZZA_SLICE_NUM);

             /*get configured speed profile */
            st = prvCpssDxChPortTxSpeedProfileGet(dev, dpIndex, txDPSpeedProfile );
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* get DP pizza slices bit map*/
            st = prvCpssDxChTxDPPizzaSliceBitMapGet(dev, dpIndex, txDPPizzaSliceBitMap);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"prvCpssDxChPortDebugFalconPortSpeed call prvCpssDxChTxDPPizzaSliceBitMapGet dp %d\n", dpIndex);

            /* go over all ports and compare slices bit map   and speed profile*/
            for (portIndex = 0 ; portIndex < FALCON_PORTS_PER_DP_CNS ; portIndex++)
            {
                /* app demo dp 15 have only 3 ports in falcon 6.4*/
                if ((dpIndex == 15) && (portIndex > 2))
                {
                    continue;
                }
                PRV_UTF_LOG4_MAC("\n=======  stage 2 : go over dp %d , port %d, pizza bit map [0x%8.8x] speed profile %d  =======\n",dpIndex, portIndex,txDPPizzaSliceBitMap[portIndex],txDPSpeedProfile[portIndex]);
                UTF_VERIFY_EQUAL4_STRING_MAC(txDPPizzaSliceBitMap[portIndex], expectedDPPizzaSliceBitMap[portIndex],"prvCpssDxChPortDebugFalconPortSpeed: port %d have slice bit map [0x%8.8x] different than expected [0x%8.8x] for speed %d", portIndex, txDPPizzaSliceBitMap[portIndex],expectedDPPizzaSliceBitMap[portIndex],falconPortsModes400GOptions[dpIndex]);
                if (expectedDPSpeedProfile[portIndex] != PRV_CPSS_PORT_SPEED_PROFILE_NA_E)
                {
                    UTF_VERIFY_EQUAL4_STRING_MAC(expectedDPSpeedProfile[portIndex], txDPSpeedProfile[portIndex],"prvCpssDxChPortDebugFalconPortSpeed: port %d have speed profile %d different than expected %d for speed %d", portIndex, txDPSpeedProfile[portIndex], expectedDPSpeedProfile[portIndex],falconPortsModes400GOptions[dpIndex]);
                }
            }

            /* check cpu port configuration speed profile and bit map*/
            if (cpuPortsInDP[dpIndex] > 0)
            {
                UTF_VERIFY_EQUAL3_STRING_MAC(cpuPortSpeedProfile, txDPSpeedProfile[PRV_DIP6_TX_CPU_PORT_NUM_CNS],"cpu port in DP %d speed profile is %d, excepted %d", dpIndex, txDPSpeedProfile[PRV_DIP6_TX_CPU_PORT_NUM_CNS], cpuPortSpeedProfile);
                st = prvCpssDxChTxCpuPizzaGet(dev, dpIndex, &cpuSliceNumber);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
                UTF_VERIFY_EQUAL2_STRING_MAC(cpuSliceNumber, PRV_DIP6_CPU_PORT_NUM_OF_SLICE,"prvCpssDxChPortDebugFalconPortSpeed: in dp %d cpu slice number is %d\n",dpIndex,cpuSliceNumber);
             }
        }

        st = prvUtfNextTxDmaPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 2.3. For all active devices go over all available physical ports. delete port speed configurations- set the port pizza slices to invalid */
        while(GT_OK == prvUtfNextTxDmaPortGet(&portNum, GT_TRUE))
        {
            st = prvCpssDxChGetTxIndexAndChannelIndexfromPortNum(dev, portNum, &dpIndex, &portIndex );
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            dpModeOptionIndex = (dpIndex < portsModes400GOptionsNum ? dpIndex : portsModes400GOptionsNum%dpIndex );

            if (portIndex == PRV_DIP6_TX_CPU_PORT_NUM_CNS )
            {
                portSpeed = cpuPortSpeed; /* Fixed CPU port speed 10G*/
            }
            else
            if (falconPortsModes400GOptions[dpModeOptionIndex][portIndex] == CPSS_PORT_SPEED_NA_E)
            {
                continue;
            }
            cpuPortsInDP[dpIndex] = 0;
            st = prvCpssDxChTxPortSpeedPizzaResourcesSet(dev, portNum, CPSS_PORT_INTERFACE_MODE_NA_E, falconPortsModes400GOptions[dpModeOptionIndex][portIndex], GT_FALSE);
            UTF_VERIFY_EQUAL2_STRING_MAC(st,GT_OK,"cpssDxChPortDebugFalconPortSpeedFromPortNumGet call prvCpssDxChPortDebugFalconPizzaTest with port number %d, speed %d failed\n", portIndex,falconPortsModes400GOptions[dpModeOptionIndex][portIndex]);
        }
    }

}


#endif /*falconDebugPizza*/


/* cpssHwsD2dPcsAndMacCalendarsAndCreditsSet
 Falcon d2d Pcs and Mac RX TDM calendars and Mac credits test
 the test go over posible options for 400G DP configuration.
 the option includes R8, R4, R2, R1 . all differernts kind of R for the same bandwidth expected the same slices and credits values
 the test sets all DP in the same configuration,
 for each DP checks Pcs calendar configuration and mac calendar configuration.
 go over all dp again and remove the cunfiguration.
 check that the right amount of slices was removred according to channel bandwidth

 done for "eagle side" ( even d2d indexes) and for "raven side" ( odd d2d indexes)

 in each d2d there are 0 - 7 ports , cpu port 8 or 16.
  */
UTF_TEST_CASE_MAC(cpssHwsD2dPcsAndMacCalendarsAndCreditsSet)
{
    GT_STATUS                    st  = GT_OK;
    GT_U8                        dev;
    GT_PHYSICAL_PORT_NUM         port     = HWINIT_VALID_PORTNUM_CNS;
    GT_U32                       ii, jj;
    GT_U32                       D2dUnitsNum; /* num of d2d units in device*/
    GT_U32                       channel;
    GT_U32                       d2dIdx;   /*D2D index */
    GT_U32                       d2dNumRaven;/*d2d Raven number*/
    GT_U32                       d2dNumEagle; /* d2d Eagle Number*/
    GT_U32                       channelIndex; /*local channel index */
    GT_U32                       serdesSpeedIndex = 0;
    GT_U32                       numOfLanesIndex = 1;
    MV_HWS_SERDES_SPEED          serdesSpeed;
    GT_U8                        numOfLanes;
    GT_U32                       cpuPortsInD2d[MAX_DP_CNS] = {0};
    GT_U32                       removedSlicesEagle[FALCON_PORTS_PER_DP_CNS+1][2];
    GT_U32                       removedSlicesRaven[FALCON_PORTS_PER_DP_CNS +1][2];

    /* credits */
    GT_U32          ravenTxCredits[4];
    GT_U32          ravenRxCredits[8];
    GT_U32          eagleTxCredits[4];
    GT_U32          eagleRxCredits[8];
    GT_U32          expectedRavenTxCredits[4];
    GT_U32          expectedRavenRxCredits[8];
    GT_U32          expectedEagleTxCredits[4];
    GT_U32          expectedEagleRxCredits[8];
    GT_U32          channelPcsSlice;


    cpssOsMemSet(&ravenTxCredits,0,(4*sizeof(GT_U32)));
    cpssOsMemSet(&ravenRxCredits,0,(8*sizeof(GT_U32)));
    cpssOsMemSet(&eagleTxCredits,0,(4*sizeof(GT_U32)));
    cpssOsMemSet(&expectedRavenTxCredits,0,(4*sizeof(GT_U32)));
    cpssOsMemSet(&expectedRavenRxCredits,0,(8*sizeof(GT_U32)));
    cpssOsMemSet(&expectedEagleTxCredits,0,(4*sizeof(GT_U32)));
    cpssOsMemSet(&expectedEagleRxCredits,0,(8*sizeof(GT_U32)));

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~(UTF_FALCON_E));

    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* number of D2d in test: each d2d connect to DPcore */
        D2dUnitsNum = PRV_CPSS_PP_MAC(dev)->multiPipe.numOfTiles * MAX_DP_PER_TILE_CNS;

        PRV_UTF_LOG0_MAC("\n=======  go over all D2d for all 400G modes options  =======\n");

        /* go over all options to configured 400G d2d core*/
        for ( ii = 0 ; ii < D2dserdesSpeedAndNumOfLanes400GOptiontsNum; ii++)
        {
            st = prvUtfNextMacPortReset(&port, dev); /* gop ports*/
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            /* 1. For all active devices go over all available physical ports. */
            while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
            {
                st = prvCpssDxChGetTxIndexAndChannelIndexfromPortNum(dev, port, &d2dIdx, &channel );
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

                /* Init D2D */
                d2dNumRaven = PRV_HWS_D2D_SET_D2D_NUM_MAC(d2dIdx, 1);
                d2dNumEagle  = PRV_HWS_D2D_SET_D2D_NUM_MAC(d2dIdx, 0);

                if ((ii == 0) && (channel == 0)) /* new d2d unit IN THE FIRST TIME*/
                {
                    st = mvHwsD2dDisableAll50GChannel(dev,d2dNumEagle);
                    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

                    st = mvHwsD2dDisableAll50GChannel(dev,d2dNumRaven);
                    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
                }

                /* the port is belong to another channel*/
                if (D2dserdesSpeedAndNumOfLanes400GOptionts[ii][channel][serdesSpeedIndex] == SPEED_NA)
                {
                    continue;
                }

                serdesSpeed = D2dserdesSpeedAndNumOfLanes400GOptionts[ii][channel][serdesSpeedIndex];
                numOfLanes  = D2dserdesSpeedAndNumOfLanes400GOptionts[ii][channel][numOfLanesIndex];

                if (channel < FALCON_PORTS_PER_DP_CNS) /* not cpu port*/
                {
                    /* eagle: set channel calendars and credits*/
                    st = mvHwsD2dConfigChannel(dev,d2dNumEagle,channel,serdesSpeed,numOfLanes);
                    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,": mvHwsD2dConfigChannel failed, channelIndex %d, serdesSpeed %d, numOfLanes %d\n",channel, serdesSpeed, numOfLanes);

                    /* raven: set channel calendars and credits*/
                    st = mvHwsD2dConfigChannel(dev,d2dNumRaven,channel,serdesSpeed,numOfLanes);
                    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,": mvHwsD2dConfigChannel failed, channelIndex %d, serdesSpeed %d, numOfLanes %d\n",channel, serdesSpeed, numOfLanes);
                }
                else /* cpu port. in the 400G option cpu port in index 8*/
                if (channel == PRV_DIP6_TX_CPU_PORT_NUM_CNS)
                {
                     if (cpuPortsInD2d[d2dIdx] == 1)
                     {
                         UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, GT_FAIL);
                     }
                     cpuPortsInD2d[d2dIdx]++;

                    /* eagle: cpu port 8: set channel calendars and credits*/
                    st = mvHwsD2dConfigChannel(dev,d2dNumEagle,channel,serdesSpeed,numOfLanes);
                    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,": mvHwsD2dConfigChannel failed, channelIndex %d, serdesSpeed %d, numOfLanes %d\n",channel, serdesSpeed, numOfLanes);

                    /* raven: cpu port 16: set channel calendars and credits*/
                    st = mvHwsD2dConfigChannel(dev,d2dNumRaven,16,serdesSpeed,numOfLanes);
                    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,": mvHwsD2dConfigChannel failed, channelIndex %d, serdesSpeed %d, numOfLanes %d\n",channel, serdesSpeed, numOfLanes);
                }
                else
                {
                    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, GT_FAIL);
                }
            }

            /*  go over all d2d in the device check that all was configured right*/
            for (d2dIdx = 0 ; d2dIdx < D2dUnitsNum; d2dIdx++ )
            {
                d2dNumRaven = PRV_HWS_D2D_SET_D2D_NUM_MAC(d2dIdx, 1);
                d2dNumEagle = PRV_HWS_D2D_SET_D2D_NUM_MAC(d2dIdx, 0);

                cpssOsMemSet(&removedSlicesEagle,0,(2*(FALCON_PORTS_PER_DP_CNS+1)*sizeof(GT_U32)));
                cpssOsMemSet(&removedSlicesRaven,0,(2*(FALCON_PORTS_PER_DP_CNS+1)*sizeof(GT_U32)));

                /* get Eagle D2D slices configuraton*/
                st = mvHwsD2dGetChannelsConfigurations(dev,d2dNumEagle,removedSlicesEagle);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "get channel configuratin for eagle faild");

                /* get Raven D2D slices configuraton*/
                st = mvHwsD2dGetChannelsConfigurations(dev,d2dNumRaven,removedSlicesRaven);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"get channel configuratin for raven faild");

                /* go over all ports and compare PCS and MAC calendars */
                for (channelIndex = 0 ; channelIndex < FALCON_PORTS_PER_DP_CNS ; channelIndex++)
                {
                    /* app demo dp 15 have only 3 ports in falcon 6.4*/
                    if ((d2dIdx == 15) && (channelIndex > 2))
                    {
                        continue;
                    }

                    /*Eagle compare*/
                    if ( removedSlicesEagle[channelIndex][0] != D2dserdesSpeedAndNumOfLanes400GOptiontsExpected[ii][channelIndex][0] )
                    {
                        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, GT_FAIL,"eagle pcs: option %d,channelIndex %d, removed %d, expected %d\n",ii, channelIndex, removedSlicesEagle[channelIndex][0],D2dserdesSpeedAndNumOfLanes400GOptiontsExpected[ii][channelIndex][0]);
                    }
                    if ( removedSlicesEagle[channelIndex][1] != D2dserdesSpeedAndNumOfLanes400GOptiontsExpected[ii][channelIndex][1] )
                    {
                        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, GT_FAIL,"eagle mac: option %d,channelIndex %d, removed %d, expected %d\n",ii,channelIndex, removedSlicesEagle[channelIndex][1],D2dserdesSpeedAndNumOfLanes400GOptiontsExpected[ii][channelIndex][1]);
                    }

                    /* Raven compare*/
                    if ( removedSlicesEagle[channelIndex][0] != D2dserdesSpeedAndNumOfLanes400GOptiontsExpected[ii][channelIndex][0] )
                    {
                        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, GT_FAIL,"raven pcs: option %d,channelIndex %d, removed %d, expected %d\n",ii,channelIndex, removedSlicesEagle[channelIndex][0],D2dserdesSpeedAndNumOfLanes400GOptiontsExpected[ii][channelIndex][0]);
                    }
                    if ( removedSlicesEagle[channelIndex][1] != D2dserdesSpeedAndNumOfLanes400GOptiontsExpected[ii][channelIndex][1] )
                    {
                        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, GT_FAIL,"raven mac: option %d, channelIndex %d, removed %d, expected %d\n",ii, channelIndex, removedSlicesEagle[channelIndex][1],D2dserdesSpeedAndNumOfLanes400GOptiontsExpected[ii][channelIndex][1]);
                    }
                 }

                 /*cpu port*/
                 if (cpuPortsInD2d[d2dIdx] == 1)
                 {
                     channelIndex = PRV_DIP6_TX_CPU_PORT_NUM_CNS;
                     /*Eagle compare*/
                     if ( removedSlicesEagle[channelIndex][0] != D2dserdesSpeedAndNumOfLanes400GOptiontsExpected[ii][channelIndex][0] )
                     {
                         UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, GT_FAIL,"channelIndex %d, removed %d, expected %d\n",channelIndex, removedSlicesEagle[channelIndex][0],D2dserdesSpeedAndNumOfLanes400GOptiontsExpected[ii][channelIndex][0]);
                     }

                     if ( removedSlicesRaven[channelIndex][1] != D2dserdesSpeedAndNumOfLanes400GOptiontsExpected[ii][channelIndex][1] )
                     {
                         UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, GT_FAIL,"channelIndex %d, removed %d, expected %d\n",channelIndex, removedSlicesRaven[channelIndex][1],D2dserdesSpeedAndNumOfLanes400GOptiontsExpected[ii][channelIndex][1]);
                     }
                 }
                /* go over all ports and compare MAC credits */
                for (channelIndex = 0 ; channelIndex < FALCON_PORTS_PER_DP_CNS ; channelIndex++)
                {
                    /* app demo dp 15 have only 3 ports in falcon 6.4*/
                    if ((d2dIdx == 15) && (channelIndex > 2))
                    {
                        continue;
                    }

                    /* Get expected PCS slices number.,the credits values derived from this number*/
                    channelPcsSlice = D2dserdesSpeedAndNumOfLanes400GOptiontsExpected[ii][channelIndex][0/*pcs index*/];
                    if (channelPcsSlice == 0)
                    {
                        continue;
                    }

                    /*Get Eagle MAC Tx and Rx channel credits */
                    st = mvHwsD2dMacCreditsGet(dev,d2dNumEagle,channelIndex,eagleTxCredits,eagleRxCredits);
                    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"get channel credits for eagle faild");
                    /*Get Raven MAC Tx and Rx channel credits */

                    st = mvHwsD2dMacCreditsGet(dev,d2dNumRaven,channelIndex,ravenTxCredits,ravenRxCredits);
                    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"get channel credits for raven faild");

                    /*Get expected credits values for Rx and Tx in Eagle ad Raven according to expeted PCS slices*/
                    pcsSlicesNumToExpectedCreditsValuesGet(channelPcsSlice,channelIndex,expectedEagleTxCredits,expectedEagleRxCredits,expectedRavenRxCredits,expectedRavenTxCredits);

                    /* compare eagle credits*/
                    /* Tx credits*/
                    for (jj = 0; jj < 4 /* TX credits parameters number*/ ;jj++)
                    {
                        if (expectedEagleTxCredits[jj] != eagleTxCredits[jj])
                        {
                            UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, GT_FAIL,"eagle Tx credits: option %d, channelIndex %d, credit index %d exist %d, expected %d\n",ii, channelIndex,jj, eagleTxCredits[jj],expectedEagleTxCredits[jj]);
                        }
                    }
                    /* Rx credits*/
                    /* Rx on eagle does not use credits, indexes 0 -3 are for credits so here the test skip this */
                    for (jj = 4; jj < 8 /* RX credits parameters number*/ ;jj++)
                    {
                        if (expectedEagleRxCredits[jj] != eagleRxCredits[jj])
                        {
                            UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, GT_FAIL,"eagle Rx credits: option %d, channelIndex %d, credit index %d exist %d, expected %d\n",ii, channelIndex,jj, eagleRxCredits[jj],expectedEagleRxCredits[jj]);
                        }
                    }
                    /* compare raven credits*/
                   /* Tx credits*/
                    for (jj = 0; jj < 4 /* TX credits parameters number*/;jj++)
                    {
                        if (expectedRavenTxCredits[jj] != ravenTxCredits[jj])
                        {
                            UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, GT_FAIL,"raven Tx credits: option %d, channelIndex %d, credit index %d exist %d, expected %d\n",ii, channelIndex,jj, ravenTxCredits[jj],expectedRavenTxCredits[jj]);
                        }
                    }
                    /* Rx credits*/
                    for (jj = 0; jj < 8 /* RX credits parameters number*/ ;jj++)
                    {
                        if (expectedRavenRxCredits[jj] != ravenRxCredits[jj])
                        {
                            UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, GT_FAIL,"raven Rx credits: option %d, channelIndex %d, credit index %d exist %d, expected %d\n",ii, channelIndex,jj, ravenRxCredits[jj],expectedRavenRxCredits[jj]);
                        }
                    }
                }
            } /* validate config*/

            /*  go over all d2d in the device and do disable channel*/
            for (d2dIdx = 0 ; d2dIdx < D2dUnitsNum; d2dIdx++ )
            {
                d2dNumRaven = PRV_HWS_D2D_SET_D2D_NUM_MAC(d2dIdx, 1);
                d2dNumEagle  = PRV_HWS_D2D_SET_D2D_NUM_MAC(d2dIdx, 0);
                /* go over all ports and compare slices bit map   and speed profile*/
                for (channelIndex = 0 ; channelIndex <= FALCON_PORTS_PER_DP_CNS /* include cpu port*/; channelIndex++)
                {
                    /* app demo dp 15 have only 3 ports in falcon 6.4*/
                    if ((d2dIdx == 15) && (channelIndex > 2))
                    {
                        continue;
                    }

                    if (D2dserdesSpeedAndNumOfLanes400GOptionts[ii][channelIndex][serdesSpeedIndex] == SPEED_NA)
                    {
                        continue;
                    }

                    serdesSpeed = D2dserdesSpeedAndNumOfLanes400GOptionts[ii][channelIndex][serdesSpeedIndex];
                    numOfLanes = D2dserdesSpeedAndNumOfLanes400GOptionts[ii][channelIndex][numOfLanesIndex];

                    if (channelIndex < FALCON_PORTS_PER_DP_CNS) /* not cpu port*/
                    {
                        st = mvHwsD2dDisableChannel(dev,d2dNumEagle,channelIndex);
                        UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, st,": mvHwsD2dDisableChannel failed, ii : %d,  d2d index %d, channelIndex %d, serdesSpeed %d, numOfLanes %d\n",ii,d2dIdx, channelIndex, serdesSpeed, numOfLanes);

                        st = mvHwsD2dDisableChannel(dev, d2dNumRaven, channelIndex);
                        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,": mvHwsD2dDisableChannel failed,  d2d index %d, channelIndex %d, serdesSpeed %d, numOfLanes %d\n",d2dIdx, channelIndex, serdesSpeed, numOfLanes);
                    }
                    /* check cpu port configuration calendars*/
                    else
                    if ((cpuPortsInD2d[d2dIdx] == 1) && (channelIndex == PRV_DIP6_TX_CPU_PORT_NUM_CNS))
                    {
                        st = mvHwsD2dDisableChannel(dev,d2dNumEagle,PRV_DIP6_TX_CPU_PORT_NUM_CNS);
                        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,": mvHwsD2dDisableChannel failed, channelIndex %d, serdesSpeed %d, numOfLanes %d\n",channelIndex, serdesSpeed, numOfLanes);

                        st = mvHwsD2dDisableChannel(dev,d2dNumRaven,16);
                        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,": mvHwsD2dDisableChannel failed, channelIndex %d, serdesSpeed %d, numOfLanes %d\n",channelIndex, serdesSpeed, numOfLanes);
                        cpuPortsInD2d[d2dIdx] = 0;
                    }
                }
            }/* disable config*/
        }/* 400G options*/
    }/* devs*/
}

/*
GT_STATUS cpssDxChPortPizzaArbiterBWModeSet
(
    IN  GT_U8                          devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    IN  CPSS_PORT_PA_BW_MODE_ENT       portMode
);
*/
GT_VOID cpssDxChPortPizzaArbiterBWModeSetUT(GT_VOID)
{
    GT_STATUS st  = GT_OK;
    GT_U8     dev = 0;
    CPSS_PORT_PA_BW_MODE_ENT       defaultPortMode = CPSS_PORT_PA_BW_MODE_REGULAR_E;
    CPSS_PORT_PA_BW_MODE_ENT       otherPortMode   = CPSS_PORT_PA_BW_EXT_MODE_1_E;
    CPSS_PORT_PA_BW_MODE_ENT       portModeGet;
    GT_U32                         portMacNum;
    GT_PHYSICAL_PORT_NUM           port;

    /* Prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
        UTF_CH2_E | UTF_CH3_E |
        UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E |UTF_LION2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev); /* gop ports*/
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvCpssDxChPortPhysicalPortMapCheckAndConvert(
                dev, port, PRV_CPSS_DXCH_PORT_TYPE_MAC_E, &portMacNum);
            if (st != GT_OK)
            {
                /* ports without local MAC - remote or SDMA */
                /* function should return GT_BAD_PARAM      */
                st = cpssDxChPortPizzaArbiterBWModeSet(
                    dev, port, defaultPortMode);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

                continue;
            }
            /* ports with local MAC */
            /* attempt to set and check other mode */
            st = cpssDxChPortPizzaArbiterBWModeSet(
                dev, port, otherPortMode);
            if (st == GT_OK)
            {
                st = cpssDxChPortPizzaArbiterBWModeGet(
                    dev, port, &portModeGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                UTF_VERIFY_EQUAL2_PARAM_MAC(otherPortMode, portModeGet, dev, port);
            }
            /* restore default mode any case and check */
            st = cpssDxChPortPizzaArbiterBWModeSet(
                dev, port, defaultPortMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            st = cpssDxChPortPizzaArbiterBWModeGet(
                dev, port, &portModeGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            UTF_VERIFY_EQUAL2_PARAM_MAC(defaultPortMode, portModeGet, dev, port);
        }
        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortPizzaArbiterBWModeSet(
                dev, port, defaultPortMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortPizzaArbiterBWModeSet(
            dev, port, defaultPortMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    port = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
        UTF_CH2_E | UTF_CH3_E |
        UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E |UTF_LION2_E | UTF_FALCON_E);

    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPizzaArbiterBWModeSet(
            dev, port, defaultPortMode);
        UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_OK, st);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPizzaArbiterBWModeSet(
        dev, port, defaultPortMode);
    UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_OK, st);
}

/*
GT_STATUS cpssDxChPortPizzaArbiterBWModeGet
(
    IN   GT_U8                          devNum,
    IN   GT_PHYSICAL_PORT_NUM           portNum,
    OUT  CPSS_PORT_PA_BW_MODE_ENT       *portModePtr
);
*/
GT_VOID cpssDxChPortPizzaArbiterBWModeGetUT(GT_VOID)
{
    GT_STATUS st  = GT_OK;
    GT_U8     dev = 0;
    CPSS_PORT_PA_BW_MODE_ENT       portModeGet;
    GT_U32                         portMacNum;
    GT_PHYSICAL_PORT_NUM           port;

    /* Prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
        UTF_CH2_E | UTF_CH3_E |
        UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E |UTF_LION2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev); /* gop ports*/
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvCpssDxChPortPhysicalPortMapCheckAndConvert(
                dev, port, PRV_CPSS_DXCH_PORT_TYPE_MAC_E, &portMacNum);
            if (st != GT_OK)
            {
                /* ports without local MAC - remote or SDMA */
                /* function should return GT_BAD_PARAM      */
                st = cpssDxChPortPizzaArbiterBWModeGet(
                    dev, port, &portModeGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

                continue;
            }
            /* ports with local MAC - should initial be in default mode */
            st = cpssDxChPortPizzaArbiterBWModeGet(
                dev, port, &portModeGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* chek null pointer behavior */
            st = cpssDxChPortPizzaArbiterBWModeGet(
                dev, port, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);
        }
        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPortPizzaArbiterBWModeGet(
                dev, port, &portModeGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChPortPizzaArbiterBWModeGet(
            dev, port, &portModeGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    port = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
        UTF_CH2_E | UTF_CH3_E |
        UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E |UTF_LION2_E | UTF_FALCON_E);

    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortPizzaArbiterBWModeGet(
            dev, port, &portModeGet);
        UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_OK, st);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortPizzaArbiterBWModeGet(
        dev, port, &portModeGet);
    UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_OK, st);
}

/*---------------------------------------------------------------------------------*/
/*
* Configuration of cpssDxChPortPizzaArbiter suit
*/
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChLion2PortPizzaArbiter)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPizzaArbiterIfUserTableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPizzaArbiterDevStateGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPizzaArbiterDevStateInit)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPizzaArbiterIfSliceStateGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPizzaArbiterIfUserTableGet)
UTF_SUIT_END_TESTS_MAC(cpssDxChLion2PortPizzaArbiter)


UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChBobcat2PortPizzaArbiter)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChLion2PortModeSpeedSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBobcat2PortModeSpeedSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortDynamicPizzaArbiterIfWorkConservingModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortDynamicPizzaArbiterIfWorkConservingModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPizzaArbiterIfConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortResourcesConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPizzaArbiterBWModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPizzaArbiterBWModeGet)
    /* next test must be last to do cleanup */
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPizzaArbiter_cleanup_withSystemReset)
UTF_SUIT_END_TESTS_MAC(cpssDxChBobcat2PortPizzaArbiter)


UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChFalconPortPizzaArbiter)
    UTF_SUIT_DECLARE_TEST_MAC(cpssHwsD2dPcsAndMacCalendarsAndCreditsSet)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssDxChPortTxFalconPizzaAndSpeedProfileSet)
#ifdef falconDebugPizza
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssDxChPortTxFalconPizzaAndSpeedProfileDebug)
#endif
UTF_SUIT_END_TESTS_MAC(cpssDxChFalconPortPizzaArbiter)

UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChInitSystemOnDiffCC)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPizzaArbiter_initSystem_on_diffCC)
UTF_SUIT_END_TESTS_MAC(cpssDxChInitSystemOnDiffCC)



#if 0
GT_STATUS utfSuit_cpssDxChBobcat2PortPizzaArbiter(GT_VOID)
{
    GT_STATUS st = GT_OK;
    static const GT_VOID *s_List[] =
    {
        "cpssDxChBobcat2PortPizzaArbiter"
        ,"cpssDxChLion2PortModeSpeedSet"                          , cpssDxChLion2PortModeSpeedSetUT
        ,"cpssDxChBobcat2PortModeSpeedSet"                        , cpssDxChBobcat2PortModeSpeedSetUT
        ,"cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionSet", cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionSetUT
        ,"cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionGet", cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionGetUT
        ,0
    };
    st = utfSuitAdd(s_List);
    return st;
}
#endif

