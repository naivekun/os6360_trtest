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
* @file gtDbDxBobcat2PhyConfig.h
*
* @brief Initialization board phy config for the Bobcat2 - SIP5 - board.
*
* @version   5
********************************************************************************
*/
#ifndef __gtDbDxBobcat2PhyConfig_H
#define __gtDbDxBobcat2PhyConfig_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <gtExtDrv/drivers/gtGenDrv.h>


typedef enum BC2_BOARD_REV_ID_ENT
{
     BOARD_REV_ID_DB_E        = 1
    ,BOARD_REV_ID_DB_TM_E     = 2
    ,BOARD_REV_ID_RDMTL_E     = 3
    ,BOARD_REV_ID_RDMTL_TM_E  = 4
    ,BOARD_REV_ID_RDMSI_E     = 11
    ,BOARD_REV_ID_RDMSI_TM_E  = 12
}BC2_SYSTEM_TYPE_ENT;

/**
* @enum CPSS_TM_SCENARIO
 *
 * @brief tm scenario mode.
*/
typedef enum
{
    CPSS_TM_48_PORT = 0,
    CPSS_TM_2=2,
    CPSS_TM_3=3,
    CPSS_TM_4=4,
    CPSS_TM_5=5,
    CPSS_TM_6=6,
    CPSS_TM_7=7,
    CPSS_TM_8=8,
    CPSS_TM_9=9,
    CPSS_TM_STRESS=10,
    CPSS_TM_STRESS_1=11,
    CPSS_TM_REV4=12,
    CPSS_TM_USER_20=20
} CPSS_TM_SCENARIO;

 
/**
* @internal bobcat2BoardPhyConfig function
* @endinternal
*
* @brief   Board specific PHY configurations after board initialization.
*
* @param[in] boardRevId               - The board revision Id.
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS bobcat2BoardPhyConfig
(
    IN  GT_U8       boardRevId,
    IN  GT_U8       devNum
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

 

#endif




