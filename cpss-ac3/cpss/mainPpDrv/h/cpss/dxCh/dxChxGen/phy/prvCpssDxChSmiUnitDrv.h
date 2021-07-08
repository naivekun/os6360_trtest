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
* @file prvCpssDxChSmiUnitDrv.h
*
* @brief SMI unit interface (Switching Core - GOP - SMI).
*
* @version   1
********************************************************************************
*/
#ifndef __PRV_CPSS_DXCH_SMI_UNIT_DRV_H
#define __PRV_CPSS_DXCH_SMI_UNIT_DRV_H

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define NUMBER_OF_SMI_PER_LMS_UNIT_CNS      2
#define NUMBER_OF_PORTS_PER_LMS_UNIT_CNS    24

#define NUMBER_OF_PORTS_PER_SMI_UNIT_CNS    16

/**
* @internal prvCpssDxChSMIInfoByPPSet function
* @endinternal
*
* @brief   set SMI info
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChSMIInfoByPPSet
(
    GT_U8 devNum
);

/**
* @internal prvCpssDxChSMIDrvInit function
* @endinternal
*
* @brief   Mac SMI driver init
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note it uses prv_dxChMacTG as single-tone
*
*/
GT_STATUS prvCpssDxChSMIDrvInit
(
    IN    GT_U8                   devNum
);

GT_STATUS prvCpssDxChSMIDrvManagementRegAddrGet
(
    IN  GT_U8 devNum,
    IN  GT_U32 smiInstance,
    OUT GT_U32 * regAddrPtr
);

GT_STATUS prvCpssDxChSMIDrvMiscConfigRegAddrGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  smiInstance,
    OUT GT_U32 *regAddrPtr
);


GT_STATUS prvCpssDxChSMIDrvPHYAutoNegConfigRegAddrGet
(
    IN  GT_U8 devNum,
    IN  GT_U32 smiInstance,
    OUT GT_U32 * regAddrPtr
);


/**
* @internal prvCpssDxChSMIInvertMDCSet function
* @endinternal
*
* @brief   Enable/Disable InvertMDC
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] smiInstance              - smi instance
* @param[in] state                    - GT_TRUE:  invert MDC
*                                      GT_FALSE: don't invert MDC
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - on port is not initialized (interface/speed)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. Before calling this function the port must be connected to packet
*       generator (cpssDxChDiagPacketGeneratorConnectSet),
*       otherwise GT_BAD_STATE is returned.
*
*/
extern GT_STATUS prvCpssDxChSMIInvertMDCSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               portGroupId,
    IN  GT_U32               smiInstance,
    IN  GT_BOOL              state
);




/**
* @internal prvCpssDxChSMIAutoNegStateSet function
* @endinternal
*
* @brief   Phy auto negaotiation set
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] smiInstance              - smi instance
* @param[in] state                    - GT_TRUE:  Auto-Negotiation NotPerformed
*                                      GT_FALSE: Auto-Negotiation Performed
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - on port is not initialized (interface/speed)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChSMIAutoNegStateSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               portGroupId,
    IN  GT_U32               smiInstance,
    OUT GT_BOOL              state
);


/**
* @internal prvCpssDxChSMIAutoNegStateGet function
* @endinternal
*
* @brief   Phy auto negaotiation get
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] smiInstance              - smi instance
*
* @param[out] statePtr                 - GT_TRUE:  Auto-Negotiation NotPerformed
*                                      GT_FALSE: Auto-Negotiation Performed
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - on port is not initialized (interface/speed)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. Before calling this function the port must be connected to packet
*       generator (cpssDxChDiagPacketGeneratorConnectSet),
*       otherwise GT_BAD_STATE is returned.
*
*/
GT_STATUS prvCpssDxChSMIAutoNegStateGet
(
    IN  GT_U8                devNum,
    IN  GT_U32               portGroupId,
    IN  GT_U32               smiInstance,
    OUT GT_BOOL             *statePtr
);

/**
* @internal prvCpssDxChSMIAutoPollNumOfPortsSet function
* @endinternal
*
* @brief   Set the SMI Auto Polling number of ports.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] smiInstance              - smi instance
* @param[in] portGroupId              - portGroupId
* @param[in] state                    - SMI Auto Polling number of ports.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - on port is not initialized (interface/speed)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChSMIAutoPollNumOfPortsSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               portGroupId,
    IN  GT_U32               smiInstance,
    OUT GT_U32               state
);


/**
* @internal prvCpssDxChSMIAutoPollNumOfPortsGet function
* @endinternal
*
* @brief   Get the SMI Auto Polling number of ports.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] smiInstance              - smi instance
* @param[in] portGroupId              - portGroupId
*
* @param[out] statePtr                 - SMI Auto Polling number of ports.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - on port is not initialized (interface/speed)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChSMIAutoPollNumOfPortsGet
(
    IN  GT_U8                devNum,
    IN  GT_U32               portGroupId,
    IN  GT_U32               smiInstance,
    OUT GT_U32              *statePtr
);

/**
* @internal prvCpssDxChSMIPortAutopollingPlaceGet function
* @endinternal
*
* @brief   Get the SMI HW port (Autopolling)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - portGroupId
* @param[in] smiInstance              - smi instance
* @param[in] smiLocalPort             - smi local port
*
* @param[out] autoPollingPlacePtr      - SMI Auto Polling number of ports.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - on port is not initialized (interface/speed)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChSMIPortAutopollingPlaceGet
(
    IN  GT_U8                devNum,
    IN  GT_U32               portGroupId,
    IN  GT_U32               smiInstance,
    IN  GT_U32               smiLocalPort,
    OUT GT_U32              *autoPollingPlacePtr
);


/**
* @internal prvCpssDxChSMIPortPhyAddSet function
* @endinternal
*
* @brief   Set the SMI local port phy adderss
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - portGroupId
* @param[in] smiInstance              - smi instance
* @param[in] smiLocalPort             - smi local port
* @param[in] phyAddr                  - SMI Auto Polling number of ports.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - on port is not initialized (interface/speed)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChSMIPortPhyAddSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               portGroupId,
    IN  GT_U32               smiInstance,
    IN  GT_U32               smiLocalPort,
    OUT GT_U32               phyAddr
);

/**
* @internal prvCpssDxChSMIPortPhyAddGet function
* @endinternal
*
* @brief   Get the SMI local port phy adderss
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - portGroupId
* @param[in] smiInstance              - smi instance
* @param[in] smiLocalPort             - smi local port
*
* @param[out] phyAddrPtr               - phy address
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - on port is not initialized (interface/speed)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChSMIPortPhyAddGet
(
    IN  GT_U8                devNum,
    IN  GT_U32               portGroupId,
    IN  GT_U32               smiInstance,
    IN  GT_U32               smiLocalPort,
    OUT GT_U32              *phyAddrPtr
);

/**
* @internal prvCpssDxChSMIPortAutoMediaDetectStatusSet function
* @endinternal
*
* @brief   Set the SMI local port Auto Media Detect Status
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - portGroupId
* @param[in] smiInstance              - smi instance
* @param[in] smiLocalPort             - smi local port
* @param[in] status                   - Auto Media Detect Status
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - on port is not initialized (interface/speed)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChSMIPortAutoMediaDetectStatusSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               portGroupId,
    IN  GT_U32               smiInstance,
    IN  GT_U32               smiLocalPort,
    OUT GT_U32               status
);

/**
* @internal prvCpssDxChSMIPortAutoMediaDetectStatusGet function
* @endinternal
*
* @brief   Get the SMI local port Auto Media Detect Status
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - portGroupId
* @param[in] smiInstance              - smi instance
* @param[in] smiLocalPort             - smi local port
*
* @param[out] statusPtr                - Auto Media Detect Status
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - on port is not initialized (interface/speed)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChSMIPortAutoMediaDetectStatusGet
(
    IN  GT_U8                devNum,
    IN  GT_U32               portGroupId,
    IN  GT_U32               smiInstance,
    IN  GT_U32               smiLocalPort,
    OUT GT_U32              *statusPtr
);


GT_STATUS prvCpssDxChSMIMdcDivisionFactorSet
(
    IN  GT_U8    devNum,
    IN  GT_U8    portGroupId,
    IN  GT_U32   smiInstance,
    IN  CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT divisionFactor
);

GT_STATUS prvCpssDxChSMIMdcDivisionFactorGet
(
    IN  GT_U8    devNum,
    IN  GT_U8    portGroupId,
    IN  GT_U32   smiInstance,
    OUT CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT *divisionFactorPtr
);



typedef struct PRV_CPSS_DXCH_SMI_STATE_STC
{
    GT_U32 autoPollNumOfPortsArr[CPSS_PHY_SMI_INTERFACE_MAX_E];
    GT_U32 phyAddrRegArr        [CPSS_PHY_SMI_INTERFACE_MAX_E][NUMBER_OF_PORTS_PER_SMI_UNIT_CNS];
}PRV_CPSS_DXCH_SMI_STATE_STC;

GT_STATUS prvCpssDxChSMIStateGet
(
    IN  GT_U8    devNum,
    IN  GT_U8    portGroupId,
    OUT PRV_CPSS_DXCH_SMI_STATE_STC * statePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

