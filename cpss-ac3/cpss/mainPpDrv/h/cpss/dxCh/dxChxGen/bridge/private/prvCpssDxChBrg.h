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
*/
/**
********************************************************************************
* @file prvCpssDxChBrg.h
*
* @brief Common private bridge declarations.
*
* @version   6
********************************************************************************
*/
#ifndef __prvCpssDxChBrgh
#define __prvCpssDxChBrgh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/common/config/private/prvCpssGenIntDefs.h>

/**
* @internal prvCpssDxChBrgCpuPortBmpConvert function
* @endinternal
*
* @brief   Convert port bitmap according to physical CPU port connection.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Xcat; Xcat2.
*
* @param[in] devNum                   - device number
* @param[in] isWriteConversion        - direction of conversion
*                                      GT_TRUE - write conversion
*                                      GT_FALSE - read conversion
* @param[in] portBitmapPtr            - (pointer to) bmp of ports members in vlan
*                                      CPU port supported
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvCpssDxChBrgCpuPortBmpConvert
(
    IN GT_U8                devNum,
    IN GT_BOOL              isWriteConversion,
    IN CPSS_PORTS_BMP_STC   *portBitmapPtr
);

/**
* @internal prvCpssDxChFullQueueRewind function
* @endinternal
*
* @brief   Rewind Full FU or AU queue.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] isFuQueue                - is FDB/CNC Upload Queue ?
*                                      GT_TRUE  - FDB/CNC Upload Queue
*                                      GT_FALSE - FDB AU Messages Queue
* @param[in] appDescCtrlPtr           - (pointer to Descriptor Control Structure passed by application)
*                                      if NULL specified used primary queues descriptors
*                                      This parameter is optional and allows to application
*                                      to specify the secondary queues.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChFullQueueRewind
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      portGroupId,
    IN  GT_BOOL                     isFuQueue,
    IN  PRV_CPSS_AU_DESC_CTRL_STC   *appDescCtrlPtr
);

/**
* @internal prvCpssDxChFuqOwnerSet function
* @endinternal
*
* @brief   Set FUQ used for CNC / FDB upload.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] isCnc                    - is CNC need to be used ?
*                                      GT_TRUE - CNC need to use the FUQ
*                                      GT_FALSE - FDB upload need to use the FUQ
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChFuqOwnerSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      portGroupId,
    IN  GT_BOOL                     isCnc
);

/**
* @internal prvCpssDxChBrgFdbSizeSet function
* @endinternal
*
* @brief   function to set the FDB size.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
* @param[in] fdbSize                  - the FDB size to set.
*                                      (each device with it's own relevant values)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad device or FDB table size
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChBrgFdbSizeSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  fdbSize
);


/**
* @internal prvCpssDxChBrgFdbHashRequestSend function
* @endinternal
*
* @brief   The function Send Hash request (HR) message to PP to generate all values
*         of hash results that relate to the 'Entry KEY'.
*         The function checks that AU messaging is ready before using it.
*         The function does not check that HR message was processed by PP.
*         For the results Application can call cpssDxChBrgFdbHashResultsGet.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] entryKeyPtr              - (pointer to) entry key
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or entryKeyPtr->entryType
* @retval GT_OUT_OF_RANGE          - one of the parameters of entryKeyPtr is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - the PP is not ready to get a message from CPU.
*                                       (PP still busy with previous message)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Application should synchronize call of cpssDxChBrgFdbMacEntrySet,
*       cpssDxChBrgFdbQaSend , cpssDxChBrgFdbMacEntryDelete , cpssDxChBrgFdbHashRequestSend
*       , cpssDxChBrgFdbPortGroupMacEntrySet, cpssDxChBrgFdbPortGroupQaSend ,
*       cpssDxChBrgFdbPortGroupMacEntryDelete , cpssDxChBrgFdbMacEntryMove functions.
*       Invocations of these functions should be mutual exclusive because they
*       use same HW resources.
*       Application can check that the HR message processing has completed by
*       cpssDxChBrgFdbFromCpuAuMsgStatusGet or cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet.
*
*/
GT_STATUS prvCpssDxChBrgFdbHashRequestSend
(
    IN  GT_U8                       devNum,
    IN  CPSS_MAC_ENTRY_EXT_KEY_STC   *entryKeyPtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChBrgh */

