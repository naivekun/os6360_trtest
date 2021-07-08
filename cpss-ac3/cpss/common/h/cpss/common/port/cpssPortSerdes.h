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
* @file cpssPortSerdes.h
*
* @brief CPSS implementation for port serdes.
*
*
* @version   1
********************************************************************************
*/

#ifndef __cpssPortSerdes__
#define __cpssPortSerdes__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal cpssPortSerdesCyclicAutoTuneStatusGet function
* @endinternal
*
* @brief   Get the tune status of the port.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] tune                     -  mode
*
* @param[out] tuneStatusPtr            - (pointer to) status of tune
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS cpssPortSerdesCyclicAutoTuneStatusGet
(
    IN  GT_U8                                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                        portNum,
    IN  CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT    tune,
    OUT CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT  *tuneStatusPtr
);

/**
* @internal prvCpssPortSerdesLunchSingleiCal function
* @endinternal
*
* @brief   run single iCal.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssPortSerdesLunchSingleiCal
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
);

/**
* @internal cpssPortTxRxDefaultsDbEntryGet function
* @endinternal
*
* @brief   Get SerDes RX and TX default parameters.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; xCat3; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portMode                 - port mode to override the serdes parameters with.
*                                      (APPLICABLE DEVICES Lion2; Bobcat2; xCat3)
* @param[in] serdesSpeed              - SerDes speed/baud rate
*                                      (APPLICABLE DEVICES Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe)
*
* @param[out] txParamsPtr              - tx parameters
* @param[out] ctleParamsPtr            - rx (CTLE) parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS  cpssPortTxRxDefaultsDbEntryGet
(
    IN  GT_U8                         devNum,
    IN  MV_HWS_PORT_STANDARD          portMode,
    IN  MV_HWS_SERDES_SPEED           serdesSpeed,
    OUT CPSS_PORT_SERDES_TX_CONFIG_STC *txParamsPtr,
    OUT CPSS_PORT_SERDES_RX_CONFIG_STC *ctleParamsPtr
);

/**
* @internal cpssPortSerdesCyclicAutoTune function
* @endinternal
*
* @brief   Set the tune status of the port.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*                                      trainDb      - (pointer to) default training database
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS  cpssPortSerdesCyclicAutoTune
(
    IN GT_U8                   devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    INOUT PRV_CPSS_PORT_SERDES_CYCLIC_TUNE_DB_STC *cookieEnclosingDB
);

/**
* @internal cpssPortSerdesCyclicAutoTuneDefaultsSet function
* @endinternal
*
* @brief   Set cyclic tune database defaults.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] hwsPortMode              - hws port mode
* @param[in,out] trainDb                  - (pointer to) default training database
* @param[in,out] trainDb                  - (pointer to) default training database to be updated
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS cpssPortSerdesCyclicAutoTuneDefaultsSet
(
    IN    GT_U8 devNum,
    IN    MV_HWS_PORT_STANDARD    hwsPortMode,
    INOUT PRV_CPSS_PORT_SERDES_CYCLIC_TUNE_DB_STC *trainDb
);

/**
* @internal cpssPortSerdesCyclicAutoTuneDefaultsGet function
* @endinternal
*
* @brief   Get cyclic tune database defaults.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Pipe.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] hwsPortMode              - hws port mode
*                                      trainDb      - (pointer to) default training database
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS cpssPortSerdesCyclicAutoTuneDefaultsGet
(
    IN    GT_U8                  devNum,
    IN    MV_HWS_PORT_STANDARD   hwsPortMode,
    INOUT PRV_CPSS_PORT_SERDES_CYCLIC_TUNE_SEQUENCE_STC *trainSequenceDb
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssPortSerdes__ */

