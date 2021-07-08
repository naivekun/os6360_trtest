/*******************************************************************************
*                Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
*/
/**
********************************************************************************
* @file mvHwsGeneralCpll.h
*
* @brief
*
* @version   1
********************************************************************************
*/

#ifndef __mvHwServicesGeneralCpll_H
#define __mvHwServicesGeneralCpll_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gtOs/gtGenTypes.h>

#define MAX_CPLL_DATA_INDEX 36
#define MAX_RAVEN_CPLL_DATA_INDEX 20

/**
* @internal mvHwsBobKSrcClockModeConfigSet function
* @endinternal
*
* @brief   Configure the setting of registers and requested CPLL(0, 1, 2) on SERDES
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - SERDES number
* @param[in] extConfigRegValue        - setting value for SERDES_EXTERNAL_CONFIGURATION_0
* @param[in] setGeneralCtlReg17       - set/clear bit #19 in DEVICE_GENERAL_CONTROL_17
*                                      GT_FALSE: clear bit #19
*                                      GT_TRUE: set bit #19
* @param[in] setGeneralCtlReg20       - set/clear bit #11 in DEVICE_GENERAL_CONTROL_20
*                                      GT_FALSE: clear bit #11
*                                      GT_TRUE: set bit #11
* @param[in] cpllNum                  - cpll number
* @param[in] inFreq                   - input frequency
* @param[in] outFreq                  - output frequency
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsBobKSrcClockModeConfigSet
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          serdesNum,
    GT_U32                          extConfigRegValue,
    GT_U32                          setGeneralCtlReg17,
    GT_U32                          setGeneralCtlReg20,
    MV_HWS_CPLL_NUMBER              cpllNum,
    MV_HWS_CPLL_INPUT_FREQUENCY     inFreq,
    MV_HWS_CPLL_OUTPUT_FREQUENCY    outFreq
);

/**
* @internal mvHwsGeneralSrcClockModeConfigSet function
* @endinternal
*
* @brief   Configure the setting of registers and requested CPLL(0) on SERDES
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - SERDES number
* @param[in] extConfigRegValue        - setting value for SERDES_EXTERNAL_CONFIGURATION_0
* @param[in] setGeneralCtlReg16       - set/clear bit in DEVICE_GENERAL_CONTROL_XX
*                                       GT_FALSE: clear bit
*                                       GT_TRUE: set bit
* @param[in] cpllNum                  - cpll number
* @param[in] inFreq                   - input frequency
* @param[in] outFreq                  - output frequency
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsGeneralSrcClockModeConfigSet
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          serdesNum,
    GT_U32                          extConfigRegValue,
    GT_U32                          setGeneralCtlReg16,
    MV_HWS_CPLL_NUMBER              cpllNum,
    MV_HWS_CPLL_INPUT_FREQUENCY     inFreq,
    MV_HWS_CPLL_OUTPUT_FREQUENCY    outFreq
);


/**
* @internal mvHwsCpllControl function
* @endinternal
*
* @brief   Configure the requested CPLL (0, 1, 2) or bypass on specific CPLL
*
* @param[in] devNum                   - system device number
* @param[in] cpllNum                  - cpll number
*                                      bypassEn  - bypass enable/disable
* @param[in] inFreq                   - input frequency
* @param[in] outFreq                  - output frequency
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsCpllControl
(
    CPSS_HW_DRIVER_STC              *driverPtr,
    GT_U8                           devNum,
    MV_HWS_CPLL_NUMBER              cpllNum,
    GT_BOOL                         bypass_en,
    MV_HWS_CPLL_INPUT_FREQUENCY     inFreq,
    MV_HWS_CPLL_OUTPUT_FREQUENCY    outFreq
);

/**
* @internal mvHwsSerdesClockGlobalControl function
* @endinternal
*
* @brief   Global configuration per Serdes for referance clock control source.
*         Need to be done before any Serdes in the system is powered up.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - SERDES number
* @param[in] srcClock                 - Input clock source
* @param[in] inFreq                   - input frequency
* @param[in] outFreq                  - output frequency
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesClockGlobalControl
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          serdesNum,
    MV_HWS_INPUT_SRC_CLOCK          srcClock,
    MV_HWS_CPLL_INPUT_FREQUENCY     inFreq,
    MV_HWS_CPLL_OUTPUT_FREQUENCY    outFreq
);

/**
* @internal mvHwsSerdesClockGlobalControlGet function
* @endinternal
*
* @brief   Getting Input clock source after
*         Global configuration per Serdes for referance clock control source.
* @param[in] devNum                   - system device number
* @param[in] serdesNum                - SERDES number
*
* @param[out] srcClockPtr              - Input clock source
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - srcClockPtr is NULL
* @retval GT_BAD_STATE             - on bad state
*/
GT_STATUS mvHwsSerdesClockGlobalControlGet
(
    GT_U8                           devNum,
    GT_U32                          serdesNum,
    MV_HWS_INPUT_SRC_CLOCK         *srcClockPtr
);


#ifdef __cplusplus
}
#endif

#endif /* __mvHwServicesGeneralCpll_H */



