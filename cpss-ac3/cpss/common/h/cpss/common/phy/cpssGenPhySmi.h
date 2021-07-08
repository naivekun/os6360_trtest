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
* @file common/h/cpss/common/phy/cpssGenPhySmi.h
*
* @brief API implementation for port Core Serial Management Interface facility.
*
*
* @version   14
********************************************************************************
*/

#ifndef __cpssGenPhySmi_h
#define __cpssGenPhySmi_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>

/**
* @enum CPSS_PHY_SMI_INTERFACE_ENT
 *
 * @brief Generic Serial Management Interface numbering
 * used for both usual SMI and XSMI
*/
typedef enum{

    /** SMI interface\controller 0 */
    CPSS_PHY_SMI_INTERFACE_0_E = 0

    /** SMI interface\controller 1 */
    ,CPSS_PHY_SMI_INTERFACE_1_E

    /** @brief SMI interface\controller 2
     *  (APPLICABLE DEVICES: Bobcat2)
     */
    ,CPSS_PHY_SMI_INTERFACE_2_E

    /** @brief SMI interface\controller 3
     *  (APPLICABLE DEVICES: Bobcat2)
     */
    ,CPSS_PHY_SMI_INTERFACE_3_E

    /** to simplify validity check */
    ,CPSS_PHY_SMI_INTERFACE_MAX_E

    ,CPSS_PHY_SMI_INTERFACE_INVALID_E = (~0)

} CPSS_PHY_SMI_INTERFACE_ENT;

/**
* @enum CPSS_PHY_XSMI_INTERFACE_ENT
 *
 * @brief XSMI interface number.
*/
typedef enum{

    /** XSMI0 master interface number */
    CPSS_PHY_XSMI_INTERFACE_0_E,

    /** XSMI1 master interface number */
    CPSS_PHY_XSMI_INTERFACE_1_E,

    /** @brief XSMI2 master interface number
     *  (APPLICABLE DEVICES: Bobcat3, Aldrin2, Falcon)
     */
    CPSS_PHY_XSMI_INTERFACE_2_E,

    /** @brief XSMI3 master interface number
     *  (APPLICABLE DEVICES: Bobcat3, Aldrin2, Falcon)
     */
    CPSS_PHY_XSMI_INTERFACE_3_E,

    /** @brief to simplify validity check */
    CPSS_PHY_XSMI_INTERFACE_MAX_E
} CPSS_PHY_XSMI_INTERFACE_ENT;

/**
* @enum CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT
 *
 * @brief Generic Serial Management Interface clock (MDC)
 * division factor
*/
typedef enum{

    /** 8 division: Core clock divided by 8 */
    CPSS_PHY_SMI_MDC_DIVISION_FACTOR_8_E,

    /** 16 division: Core clock divided by 16 */
    CPSS_PHY_SMI_MDC_DIVISION_FACTOR_16_E,

    /** 32 division: Core clock divided by 32 */
    CPSS_PHY_SMI_MDC_DIVISION_FACTOR_32_E,

    /** 64 division: Core clock divided by 64 */
    CPSS_PHY_SMI_MDC_DIVISION_FACTOR_64_E,

    /** 128 division: Core clock divided by 128 */
    CPSS_PHY_SMI_MDC_DIVISION_FACTOR_128_E,

    /** 256 division: Core clock divided by 256 */
    CPSS_PHY_SMI_MDC_DIVISION_FACTOR_256_E,

    /** 512 division: Core clock divided by 512 */
    CPSS_PHY_SMI_MDC_DIVISION_FACTOR_512_E,

    /** to simplify validity check */
    CPSS_PHY_SMI_MDC_DIVISION_FACTOR_MAX_E

} CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT;


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssGenPhySmi_h */

