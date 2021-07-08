/********************************************************************************
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
* @file prvCpssPortPcsCfg.h
*
* @brief CPSS implementation for port serdes.
*
*
* @version   1
********************************************************************************
*/

#ifndef prvCpssPortPcsCfg
#define prvCpssPortPcsCfg

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>

/**
* @internal prvCpssPortPcsGearBoxStatusGet function
* @endinternal
*
* @brief   Return PCS Gear Box lock status (true - locked /false - not locked).
*
* @note   APPLICABLE DEVICES:      Lion2; xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] gbLockPtr                - Gear Box lock state on serdes:
*                                      GT_TRUE  - locked;
*                                      GT_FALSE - not locked.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - gbLockPtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPortPcsGearBoxStatusGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  portMacNum,
    OUT GT_BOOL                 *gbLockPtr
);

/**
* @internal prvCpssPortPcsSyncStatusGet function
* @endinternal
*
* @brief   Return PCS Sync status from XGKR sync block.
*
* @note   APPLICABLE DEVICES:      Lion2; xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] syncPtr                  - Sync status from XGKR sync block :
*                                      GT_TRUE  - synced;
*                                      GT_FALSE - not synced.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - syncPtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPortPcsSyncStatusGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  portMacMap,
    OUT GT_BOOL                 *syncPtr
);

/**
* @internal prvCpssPortPcsSyncStableStatusGet function
* @endinternal
*
* @brief   Return Signal Detect state on SerDes when it became be stable(true/false).
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] signalStatePtr           - the stable signal state on serdes:
*                                      GT_TRUE  - signal detected;
*                                      GT_FALSE - no signal.
*
* @retval GT_OK                    - on success.
* @retval GT_TIMEOUT               - on signal is not stable during 1000ms.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - signalStatePtr == NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
*
* @note Using algorithm to detect Serdes Signal to be stable.
*
*/
GT_STATUS prvCpssPortPcsSyncStableStatusGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  portMacMap,
    OUT GT_BOOL                 *signalStatePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* prvCpssPortPcsCfg */

