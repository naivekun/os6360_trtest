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
* @file gtDbDxBobcat2GenUtils.h
*
* @brief general utils for BC2 and higher devicesInitialization board phy config for the Bobcat2 - SIP5 - board.
*
* @version   5
********************************************************************************
*/
#ifndef __gtDbDxBobcat2Util_H
#define __gtDbDxBobcat2Util_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <appDemo/boardConfig/gtDbDxBobcat2Mappings.h>
#include <cpss/generic/port/cpssPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortMapping.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/cpssDxChPortPizzaArbiter.h>

GT_CHAR * CPSS_MAPPING_2_STR
(
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT mapEnm
);

GT_CHAR * CPSS_SCHEDULER_PROFILE_2_STR
(
    IN CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT profileId
);


GT_STATUS appDemoDxPossiblePhysPortNumGet
(
    IN GT_U8 devNum,
    OUT GT_U32 *portNumPtr
);

GT_STATUS appDemoBc2PortListInit
(
    IN GT_U8 dev,
    IN PortInitList_STC * portInitList,
    IN GT_BOOL            skipCheckEnable
);

GT_STATUS appDemoBc2PortListDelete
(
    IN GT_U8 dev,
    IN PortInitList_STC * portInitList
);

GT_STATUS appDemoBoardTypeGet
(
    OUT GT_U32 *boardTypePtr
);


GT_STATUS appDemoBobKPortPizzaArbiterResourcesUnitStatisticsGet
(
    IN GT_U8 dev,
    IN GT_U32 localNumOnUnit,
    IN CPSS_DXCH_PA_UNIT_ENT unit,
    IN CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC *pizzaDeviceStatePtr,
    OUT GT_U32 *totalPtr,
    OUT GT_U32 *sliceNptr,
    OUT GT_U32   *minDistPtr,
    OUT GT_U32   *maxDistPtr
);


/*-----------------------------------------------------------------*
 * These data structures are used in Gop Mode register comparison  *
 *-----------------------------------------------------------------*/

#define APPDEMO_BAD_VALUE (GT_U32)(~0)

typedef struct 
{
    GT_U32   idxInExcel;
    GT_CHAR *unitName;
    GT_U32   regAddr;
    GT_U32   data;
    GT_CHAR *tableName;
    GT_CHAR *regName;
    GT_CHAR *cider;
}APPDEMO_UNIT_x_REG_x_VALUE_STC;

#define BOBK_TEST_GOP_CONFIG    0
#define ALDRIN_TEST_GOP_CONFIG  0
#define BC3_TEST_GOP_CONFIG     1


GT_STATUS asicUserScenRegListPrint
(
    APPDEMO_UNIT_x_REG_x_VALUE_STC *regListptr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

 

#endif




