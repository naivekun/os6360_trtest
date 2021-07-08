/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*
* prvCpssDxChTmGlue.h
*
* DESCRIPTION:
*       Traffic Manager Glue - common private PFC API declarations.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#ifndef __prvCpssDxChTmGluePfch
#define __prvCpssDxChTmGluePfch

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>

#define PRV_CPSS_DXCH_TM_GLUE_PFC_MAX_PORT_CNS  64
#define PRV_CPSS_DXCH_TM_GLUE_PFC_MAX_TC_CNS     8
#define PRV_CPSS_DXCH_TM_GLUE_PFC_MAX_CNODE_CNS BIT_9

/* check that the TM physical port number is valid return GT_BAD_PARAM on error */
#define PRV_CPSS_DXCH_TM_GLUE_PFC_PHY_PORT_NUM_CHECK_MAC(_portNum)  \
    if ((_portNum) >= PRV_CPSS_DXCH_TM_GLUE_PFC_MAX_PORT_CNS)       \
    {                                                               \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                        \
    }


typedef struct
{
    GT_U16 TcPort2CnodeSwTable[PRV_CPSS_DXCH_TM_GLUE_PFC_MAX_PORT_CNS][PRV_CPSS_DXCH_TM_GLUE_PFC_MAX_TC_CNS];
} PRV_CPSS_DXCH_TM_GLUE_PFC_DEV_INFO_STC;

typedef struct
{
    GT_PHYSICAL_PORT_NUM port;
    GT_U32               tc;
    GT_U16               cNode;
} PRV_CPSS_DXCH_TM_GLUE_PFC_CNODE_INFO_STC;

extern GT_BOOL prvPfcTmGlueCnodeMappingOnce;


/**
* @internal prvCpssDxChTmGluePfcTmDbGetTcPortByCNode function
* @endinternal
*
* @brief   get traffic class and port by cNode from SW DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] cNodeValue               - C node value (APPLICABLE RANGES: 0...511).
*
* @param[out] portPtr                  - (poinetr to) source physical port number.
* @param[out] tcPtr                    - (pointer to) traffic class.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number,
*                                       port or traffic class.
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvCpssDxChTmGluePfcTmDbGetTcPortByCNode
(
    IN GT_U8                devNum,
    IN GT_U32               cNodeValue,
    IN GT_PHYSICAL_PORT_NUM *portPtr,
    IN GT_U32               *tcPtr

);

/**
* @internal prvCpssDxChTmGluePfcTmDbGetCNodeByTcPort function
* @endinternal
*
* @brief   get cNode by traffic class and port from SW DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] port                     - source physical port number.
* @param[in] tc                       - traffic class (APPLICABLE RANGES: 0..7).
*
* @param[out] cNodePtr                 - (pointer to) C node value.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number,
*                                       port or traffic class.
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvCpssDxChTmGluePfcTmDbGetCNodeByTcPort
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM port,
    IN GT_U32               tc,
    IN GT_U32               *cNodePtr
);

/**
* @internal prvCpssDxChTmGluePfcTmDbUpdateTcPortCNode function
* @endinternal
*
* @brief   update cNodes traffic class and port changes to SW DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
*                                      cNodeValue  - C node value (APPLICABLE RANGES: 0...511).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number,
*                                       port or traffic class.
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvCpssDxChTmGluePfcTmDbUpdateTcPortCNode
(
    IN GT_U8                                    devNum,
    IN PRV_CPSS_DXCH_TM_GLUE_PFC_CNODE_INFO_STC *cNode1Ptr,
    IN PRV_CPSS_DXCH_TM_GLUE_PFC_CNODE_INFO_STC *cNode2Ptr
);

/**
* @internal prvCpssDxChTmGluePfcPortMappingGet function
* @endinternal
*
* @brief   Get physical port to pfc port mapping.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number. (APPLICABLE RANGES: 0..255).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number, or port.
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvCpssDxChTmGluePfcPortMappingGet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_PHYSICAL_PORT_NUM    *pfcPortNumPtr
);

/**
* @internal prvCpssDxChTmGluePfcInit function
* @endinternal
*
* @brief   Init TM GLUE PFC software Data Base.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvCpssDxChTmGluePfcInit
(
    IN GT_U8 devNum
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChTmGluePfch */



