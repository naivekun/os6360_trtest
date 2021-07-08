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
* @file prvCpssDxChPtp.h
*
* @brief CPSS DXCH private PTP debug API implementation.
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssDxChPtph
#define __prvCpssDxChPtph
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @enum PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_ENT
 *
 * @brief Instance of TAI unit sub type.
 * (APPLICABLE DEVICES: Falcon.)
*/
typedef enum
{
    /** @brief TAI instance sub type Master
     */
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_MASTER_E = 0,

    /** @brief TAI instance sub type CP slave for control pipe0
     */
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_CP_TAI_SLAVE_PIPE0_E,

    /** @brief TAI instance sub type CP slave for control pipe1
     */
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_CP_TAI_SLAVE_PIPE1_E,

    /** @brief TAI instance sub type TXQ slave for control pipe0
     */
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_TXQ_TAI_SLAVE_PIPE0_E,

    /** @brief TAI instance sub type TXQ slave for control pipe1
     */
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_TXQ_TAI_SLAVE_PIPE1_E,

    /** @brief TAI slave instance for GOP0
     */
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_GOP_0_E,

    /** @brief TAI slave instance for GOP1
     */
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_GOP_1_E,

    /** @brief TAI slave instance for GOP2
     */
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_GOP_2_E,

    /** @brief TAI slave instance for GOP3
     */
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_GOP_3_E
}PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_ENT;


/********* enums *********/
/**
 * @enum PRV_CPSS_DXCH_PTP_TAI_INSTANCE_TYPE_ENT
 * @brief Type of TAI instance.
 */
typedef enum{

    /** @brief Master instance in Processing die */
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_TYPE_MASTER_E,

    /** @brief CP slave instance in Processing die */
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_TYPE_CP_SLAVE_E,

    /** @brief TXQ slave instance in Processing die */
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_TYPE_TXQ_SLAVE_E,

    /** @brief Chiplet slave instance in GOP die */
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_TYPE_CHIPLET_SLAVE_E
} PRV_CPSS_DXCH_PTP_TAI_INSTANCE_TYPE_ENT;
 
/********* lib API DB *********/
/**
* @internal prvCpssDxChPtpTaiDebugInstanceSet function
* @endinternal
*
* @brief   Sets the TAI debug instance.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2; xCat; xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum               - device number
* @param[in] tileId               - Processing die identifier
* @param[in] taiInstanceType      - Type of the instance
*                                   (Master, CP_Slave, TXQ_Slave, Chiplet_Slave )
* @param[in] instanceId           - TAI instance identifier for a particular tile.
*                                   Master              - Not used.
*                                   CP_Slave,TXQ_Slave  - 0..1 - processing pipe number.
*                                   Chiplet_Slave       - 0..3 - local GOP die number of specific tile.
*
* @retval GT_OK                   - on success
* @retval GT_BAD_PARAM            - on wrong parameter
*/
GT_STATUS prvCpssDxChPtpTaiDebugInstanceSet
(
    IN    GT_U8                                     devNum,
    IN    GT_U32                                    tileId,
    IN    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_TYPE_ENT   taiInstanceType,
    IN    GT_U32                                    instanceId
);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __prvCpssDxChPtph */
