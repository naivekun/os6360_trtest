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
* @file prvCpssEmulatorMode.h
*
* @brief Includes definitions for running the device in EMULATOR mode.
* The EMULATOR mode allow the Design team of the PP to test the RTL before
* tape-out , and by this save time and money.
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssEmulatorMode_h
#define __prvCpssEmulatorMode_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/********* Include files ******************************************************/
#include <cpss/extServices/os/gtOs/gtGenTypes.h>

/**
* @internal cpssDeviceRunSet_onEmulator function
* @endinternal
*
* @brief   State that running on EMULATOR. (all devices considered to be running on
*         EMULATOR)
*         (when running on EMULATOR) This function must be called before
*         'phase 1' init of the device.
*
* @note   APPLICABLE DEVICES:      All.
* @note   NOT APPLICABLE DEVICES:  None.
*
*                                       None.
*/
void  cpssDeviceRunSet_onEmulator(void);

/**
* @internal cpssDeviceRunCheck_onEmulator function
* @endinternal
*
* @brief   Check if running on EMULATOR.
*         (when running on EMULATOR) This function must be called before
*         'phase 1' init of the device.
*
* @note   APPLICABLE DEVICES:      All.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval 0                        - not running on EMULATOR.
* @retval 1                        - running on EMULATOR.
*/
GT_U32  cpssDeviceRunCheck_onEmulator(void);

/**
* @internal prvCpssOnEmulatorSupportedAddrCheck function
* @endinternal
*
* @brief   This function checks existences of base addresses units in the Emulator
*         of the given device.
*         since the Emulator not supports all units , the CPSS wants to avoid
*         accessing those addresses.
*         the CPSS binds the Emulator with this functions to 'cpssDriver' so the
*         driver before accessing the Emulator can know not to access the device
*         on such addresses.
*         NOTE: purpose is to remove Emulator 'ERRORS' ,and even 'crash' of
*         Emulator on some unknown addresses.
*
* @note   APPLICABLE DEVICES:      Aldrin; Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - thr PP's device number to init the struct for.
* @param[in] portGroupId              - port group ID
* @param[in] regAddr                  - register address
*
* @retval GT_TRUE                  - the unit/address is   implemented in the Emulator
* @retval GT_FALSE                 - the unit/address is NOT implemented in the Emulator
*
* @note Should be called ONLY in 'EMULATOR Mode'
*
*/
GT_BOOL prvCpssOnEmulatorSupportedAddrCheck(
    IN  GT_U8       devNum,
    IN  GT_U32      portGroupId,
    IN  GT_U32      regAddr
);

/**
* @internal cpssDeviceRunCheck_onEmulator_isAldrinFull function
* @endinternal
*
* @brief   check if 'Aldrin' on Emulator is 'FULL' or missing DP[0,1] and GOPs[ports 0..23] and TCAM.
*
* @note   APPLICABLE DEVICES:      All.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval 0                        - Aldrin is not 'FULL' and missing: missing DP[0,1] and GOPs[ports 0..23] and TCAM.
* @retval 1                        - Aldrin is 'FULL'.
*/
GT_U32  cpssDeviceRunCheck_onEmulator_isAldrinFull(void);

/**
* @internal cpssDeviceRunCheck_onEmulator_AldrinFull function
* @endinternal
*
* @brief   State that Aldrin running on EMULATOR as 'FULL' or not.
*         ('FULL' or missing DP[0,1] and GOPs[ports 0..23] and TCAM.)
*         (when running on EMULATOR) This function must be called before
*         'phase 1' init of the device.
*
* @note   APPLICABLE DEVICES:      All.
* @note   NOT APPLICABLE DEVICES:  None.
*
*                                       None.
*/
void  cpssDeviceRunCheck_onEmulator_AldrinFull(GT_U32 isFull);


/**
* @enum PRV_CPSS_DXCH_FALCON_CIDER_VERSION_ENT
 *
 * @brief Enumeration of Falcon Cider releases that we use.
 *        needed mainly for difference between Emulator to GM/WM versions.
 *
*/
typedef enum{
    /* VALUE 0 MUST not be used !!!
        see internal use of PRV_CPSS_DXCH_FALCON_CIDER_VERSION__LATEST__CNS */
    PRV_CPSS_DXCH_FALCON_CIDER_VERSION_17_11_11_E = 1,/*Nov 2017. AKA CC (code complete)   */
    PRV_CPSS_DXCH_FALCON_CIDER_VERSION_18_02_22_E = 2,/*Feb 2018. AKA IF (interface freeze)*/
    PRV_CPSS_DXCH_FALCON_CIDER_VERSION_18_03_13_E = 3,/*Mar 2018. AKA TF (top freeze)      */
    /* PRV_CPSS_DXCH_FALCON_CIDER_VERSION_18_04_22_E Apr 2018. AKA RTLF (RTL freeze) candidate */
    /* 18_04_22 design version already absolette - all references in CPSS renamed to 18_06_06  */
    PRV_CPSS_DXCH_FALCON_CIDER_VERSION_18_06_06_E = 4,/*June 2018. AKA RTLF_V2.0 (RTL freeze) candidate */
    PRV_CPSS_DXCH_FALCON_CIDER_VERSION_18_07_09_E = 5,/*July 2018. AKA RTLF_V3.0 (RTL freeze) candidate */
    PRV_CPSS_DXCH_FALCON_CIDER_VERSION___LAST___E

}PRV_CPSS_DXCH_FALCON_CIDER_VERSION_ENT;

/**
* @internal prvCpssDxchFalconCiderVersionGet function
* @endinternal
*
* @brief   get the current Cider version that is used.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  All but Falcon.
*
* @return the version
*/
PRV_CPSS_DXCH_FALCON_CIDER_VERSION_ENT prvCpssDxchFalconCiderVersionGet(void);

/**
* @internal prvCpssDxchFalconCiderVersionSet function
* @endinternal
*
* @brief   set the current Cider version that is used.
*
*   version
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  All but Falcon.
*
* @param[in] version  - the Cider version
*
* @return the version
*/
void prvCpssDxchFalconCiderVersionSet(
    IN PRV_CPSS_DXCH_FALCON_CIDER_VERSION_ENT   version
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssEmulatorMode_h */


