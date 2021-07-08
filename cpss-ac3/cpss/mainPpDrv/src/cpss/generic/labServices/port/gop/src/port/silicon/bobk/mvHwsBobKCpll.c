/*******************************************************************************
*            Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
* mvHwsBobKCpllInit.c
*
* DESCRIPTION:
*       BobK CPLL init
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>

#include <cpss/common/labServices/port/gop/port/silicon/general/mvHwsGeneralCpll.h>
/************************* definition *****************************************************/

/**
* @internal mvHwsAldrinSrcClockModeConfigSet function
* @endinternal
*
* @brief   Configure the setting of registers and requested CPLL(0) on SERDES
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - SERDES number
* @param[in] extConfigRegValue        - setting value for SERDES_EXTERNAL_CONFIGURATION_0
* @param[in] setGeneralCtlReg11       - set/clear bit #31 in DEVICE_GENERAL_CONTROL_11
*                                      GT_FALSE: clear bit #31
*                                      GT_TRUE: set bit #31
*                                      cpllNum  - cpll number
* @param[in] inFreq                   - input frequency
* @param[in] outFreq                  - output frequency
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAldrinSrcClockModeConfigSet
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          serdesNum,
    GT_U32                          extConfigRegValue,
    GT_U32                          setGeneralCtlReg11,
    MV_HWS_CPLL_INPUT_FREQUENCY     inFreq,
    MV_HWS_CPLL_OUTPUT_FREQUENCY    outFreq
)
{
    GT_U32  data;

    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SERDES_EXTERNAL_CONFIGURATION_0, extConfigRegValue, (1 << 8)));

    if ((setGeneralCtlReg11 == 1) || (setGeneralCtlReg11 == 0))
    {
        CHECK_STATUS(hwsServerRegGetFuncPtr(devNum, DEVICE_GENERAL_CONTROL_11, &data));
        data = (setGeneralCtlReg11) ? (data | (1 << 31)) : (data & ~(1 << 31));
        CHECK_STATUS(hwsServerRegSetFuncPtr(devNum, DEVICE_GENERAL_CONTROL_11, data));
    }

    return mvHwsCpllControl(NULL, devNum, CPLL0/*don't care param*/, GT_FALSE, inFreq, outFreq);
}



