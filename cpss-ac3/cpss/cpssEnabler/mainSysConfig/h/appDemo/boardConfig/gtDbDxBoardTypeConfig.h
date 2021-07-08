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
* @file gtDbDxBoardTypeConfig.h
*
* @brief Boards type definitions
*
* @version   1
********************************************************************************
*/
#ifndef __gtDbDxBoardTypeConfig_H
#define __gtDbDxBoardTypeConfig_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <gtExtDrv/drivers/gtGenDrv.h>

/* Board types: 0 - DB board; 1 - RD MSI board  2 -- RD MTL board */
#define APP_DEMO_BC2_BOARD_DB_CNS               0
#define APP_DEMO_BC2_BOARD_RD_MSI_CNS           1
#define APP_DEMO_BC2_BOARD_RD_MTL_CNS           2
#define APP_DEMO_CAELUM_BOARD_DB_CNS            (EXT_DRV_BOBK_CAELUM_DB_ID)
#define APP_DEMO_CETUS_BOARD_DB_CNS             (EXT_DRV_BOBK_CETUS_DB_ID)
#define APP_DEMO_CETUS_BOARD_LEWIS_RD_CNS       (EXT_DRV_BOBK_CETUS_LEWIS_RD_ID)
#define APP_DEMO_CAELUM_BOARD_CYGNUS_RD_CNS     (EXT_DRV_BOBK_CAELUM_CYGNUS_RD_ID)

#define APP_DEMO_ALDRIN_BOARD_DB_CNS            (EXT_DRV_ALDRIN_DB_ID)
#define APP_DEMO_ALDRIN_BOARD_RD_CNS            (EXT_DRV_ALDRIN_RD_ID)

#define APP_DEMO_BOBCAT3_BOARD_DB_CNS           (EXT_DRV_BOBCAT3_DB_ID)
#define APP_DEMO_BOBCAT3_BOARD_RD_CNS           (EXT_DRV_BOBCAT3_RD_ID)
#define APP_DEMO_ARMSTRONG_BOARD_RD_CNS         (EXT_DRV_ARMSTRONG_RD_ID)

#define APP_DEMO_ALDRIN2_BOARD_DB_CNS           (EXT_DRV_ALDRIN2_DB_ID)
#define APP_DEMO_ALDRIN2_BOARD_RD_CNS           (EXT_DRV_ALDRIN2_RD_ID)

/* xcat3x : A0,Z0 */
#define APP_DEMO_XCAT3X_A0_BOARD_DB_CNS         (EXT_DRV_XCAT3X_A0_DB_ID)
#define APP_DEMO_XCAT3X_Z0_BOARD_DB_CNS         (EXT_DRV_XCAT3X_Z0_DB_ID)

/**
* @struct APPDEMO_SERDES_LANE_POLARITY_STC
 *
 * @brief Per DB/RD board configure the Tx/Rx Polarity values on Serdeses
*/
typedef struct{

    /** number of SERDES lane */
    GT_U32 laneNum;

    /** @brief GT_TRUE
     *  GT_FALSE - no invert
     */
    GT_BOOL invertTx;

    /** @brief GT_TRUE
     *  GT_FALSE - no invert
     */
    GT_BOOL invertRx;

} APPDEMO_SERDES_LANE_POLARITY_STC;

/**
* @internal appDemoDxChBoardTypeGet function
* @endinternal
*
* @brief   This function recognizes type of board.
*         The function reads PHY type for SMI#0 address 4 (port 24 on DB board,
*         port 4 on RD board).
*         If PHY is not connected or PHY ID is 1548M the board is DB.
*         Otherwise the board is RD.
* @param[in] devNum                   - device number
*
* @param[out] boardTypePtr             - pointer to board type.
*                                      0 - DB board
*                                      1 - RD board
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoDxChBoardTypeGet
(
    IN   GT_U8    devNum,
    IN   GT_U8    boardRevID,
    OUT  GT_U32  *boardTypePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */



#endif




