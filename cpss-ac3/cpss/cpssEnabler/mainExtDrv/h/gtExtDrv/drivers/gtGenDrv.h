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
* @file gtGenDrv.h
*
* @brief Generic extDrv functions
*
* @version   1
********************************************************************************
*/
#ifndef __gtGenDrvh
#define __gtGenDrvh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <gtExtDrv/os/extDrvOs.h>

/* BobK Customer Boards */
#define EXT_DRV_BOBK_CUSTOMER_BOARD_ID_BASE         0x40
#define EXT_DRV_BOBK_CETUS_CUSTOMER_BOARD_ID0       (EXT_DRV_BOBK_CUSTOMER_BOARD_ID_BASE + 0)
#define EXT_DRV_BOBK_CAELUM_CUSTOMER_BOARD_ID1      (EXT_DRV_BOBK_CUSTOMER_BOARD_ID_BASE + 1)
#define EXT_DRV_BOBK_CUSTOMER_MAX_BOARD_ID          (EXT_DRV_BOBK_CUSTOMER_BOARD_ID_BASE + 2)
#define EXT_DRV_BOBK_CUSTOMER_BOARD_NUM             (EXT_DRV_BOBK_CUSTOMER_MAX_BOARD_ID - EXT_DRV_BOBK_CUSTOMER_BOARD_ID_BASE)

/* BobK Marvell Boards */
#define EXT_DRV_BOBK_MARVELL_BOARD_ID_BASE          0x50
#define EXT_DRV_BOBK_CETUS_DB_ID                    (EXT_DRV_BOBK_MARVELL_BOARD_ID_BASE + 0)
#define EXT_DRV_BOBK_CAELUM_DB_ID                   (EXT_DRV_BOBK_MARVELL_BOARD_ID_BASE + 1)
#define EXT_DRV_BOBK_CETUS_LEWIS_RD_ID              (EXT_DRV_BOBK_MARVELL_BOARD_ID_BASE + 2)
#define EXT_DRV_BOBK_CAELUM_CYGNUS_RD_ID            (EXT_DRV_BOBK_MARVELL_BOARD_ID_BASE + 3)
#define EXT_DRV_BOBK_MARVELL_MAX_BOARD_ID           (EXT_DRV_BOBK_MARVELL_BOARD_ID_BASE + 4)
#define EXT_DRV_BOBK_MARVELL_BOARD_NUM              (EXT_DRV_BOBK_MARVELL_MAX_BOARD_ID - EXT_DRV_BOBK_MARVELL_BOARD_ID_BASE)

/* Aldrin Marvell Boards */
#define EXT_DRV_ALDRIN_MARVELL_BOARD_ID_BASE        0x70
#define EXT_DRV_ALDRIN_DB_ID                        (EXT_DRV_ALDRIN_MARVELL_BOARD_ID_BASE + 0)
#define EXT_DRV_ALDRIN_RD_ID                        (EXT_DRV_ALDRIN_MARVELL_BOARD_ID_BASE + 1)


/* Bobcat3 Marvell Boards */
#define EXT_DRV_BOBCAT3_MARVELL_BOARD_ID_BASE        0x90
#define EXT_DRV_BOBCAT3_DB_ID                        (EXT_DRV_BOBCAT3_MARVELL_BOARD_ID_BASE + 0)
#define EXT_DRV_BOBCAT3_RD_ID                        (EXT_DRV_BOBCAT3_MARVELL_BOARD_ID_BASE + 1)
#define EXT_DRV_ARMSTRONG_RD_ID                      (EXT_DRV_BOBCAT3_MARVELL_BOARD_ID_BASE + 2)

#define EXT_DRV_ALDRIN2_DB_ID                        (EXT_DRV_BOBCAT3_MARVELL_BOARD_ID_BASE + 3)
#define EXT_DRV_ALDRIN2_RD_ID                        (EXT_DRV_BOBCAT3_MARVELL_BOARD_ID_BASE + 4)

/* AC3X Marvell Boards */
#define EXT_DRV_XCAT3X_MARVELL_BOARD_ID_BASE        0x80
#define EXT_DRV_XCAT3X_A0_DB_ID    /*A0*/           (EXT_DRV_XCAT3X_MARVELL_BOARD_ID_BASE + 0)
#define EXT_DRV_XCAT3X_Z0_DB_ID    /*Z0*/           (EXT_DRV_XCAT3X_MARVELL_BOARD_ID_BASE + 1)


/**
* @internal extDrvSoCInit function
* @endinternal
*
* @brief   Initialize SoC related drivers
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*
* @note This API is called by after cpssExtServicesBind()
*
*/
GT_STATUS extDrvSoCInit(void);

/**
* @internal extDrvBoardIdGet function
* @endinternal
*
* @brief   Get BoardId value
*
* @param[out] boardIdPtr               - boardId
*                                      None.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*/
GT_STATUS extDrvBoardIdGet
(
    OUT GT_U32  *boardIdPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __gtGenDrvh */




