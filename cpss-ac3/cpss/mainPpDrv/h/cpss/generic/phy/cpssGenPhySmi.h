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
* @file cpssGenPhySmi.h
*
* @brief API implementation for port Core Serial Management Interface facility.
*
*
* @version   14
********************************************************************************
*/
#include <cpss/common/phy/cpssGenPhySmi.h>

#ifndef __cpssGenPhySmih
#define __cpssGenPhySmih

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @enum CPSS_PHY_ERRATA_WA_ENT
 *
 * @brief PHY errata workarond list
*/
typedef enum{

    /** @brief Workaround for 88E1240
     *  SGMII lockup problem. Should be used only after packet
     *  processor PHY SMI lib init is finished and SGMII
     *  autonegotiation between the PHY and the packet processor
     *  reached synchronization. The PHY SGMII autonegotiation
     *  should be disabled if the packet processor uses
     *  out-of-band PHY autonegotiation.
     */
    CPSS_PHY_ERRATA_WA_88E1240_SGMII_LOCKUP_E,

    /** @brief Workaround for 88E1340 BGA
     *  package init. If used, should be after hardreset
     *  de-asserted for PHY initialization.
     *  Should only be applied for BGA package.
     */
    CPSS_PHY_ERRATA_WA_88E1340_BGA_INIT_E

} CPSS_PHY_ERRATA_WA_ENT;

/**
* @internal cpssPhyErrataWaExecute function
* @endinternal
*
* @brief   This function execute workaround for PHY errata from a selected list.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device Number.
* @param[in] portNum                  - Port Number.
* @param[in] errataWa                 - the selected workaround to execute
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - on not initialized function pointers for
*                                       read and write phy registers operations.
* @retval GT_BAD_PARAM             - on wrong device, port or requested WA
*/
GT_STATUS cpssPhyErrataWaExecute
(
   IN GT_U8                     devNum,
   IN GT_PHYSICAL_PORT_NUM      portNum,
   IN CPSS_PHY_ERRATA_WA_ENT    errataWa
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssGenPhySmih */


