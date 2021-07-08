/*******************************************************************************
*              (c), Copyright 2006, Marvell International Ltd.                 *
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
* @file cpssCommonDiag.h
*
* @brief Common CPSS Diag functions.
*
* @version   1
********************************************************************************
*/
#ifndef __cpssCommonDiagh__
#define __cpssCommonDiagh__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/common/cpssHwInit/cpssHwInit.h>

/**
* @enum CPSS_DIAG_PP_REG_TYPE_ENT
 *
 * @brief Packet Process registers type.
*/
typedef enum{

    /** @brief PP internal register space. */
    CPSS_DIAG_PP_REG_INTERNAL_E,

    /** @brief PP pci config and PEX register space. */
    CPSS_DIAG_PP_REG_PCI_CFG_E

} CPSS_DIAG_PP_REG_TYPE_ENT;

/**
* @enum CPSS_DIAG_TEST_PROFILE_ENT
 *
 * @brief Test profile.
*/
typedef enum{

    /** @brief write random values */
    CPSS_DIAG_TEST_RANDOM_E,

    /** @brief write incremental values */
    CPSS_DIAG_TEST_INCREMENTAL_E,

    /** @brief toggles all the bits */
    CPSS_DIAG_TEST_BIT_TOGGLE_E,

    /** @brief write 0xAAAAAAAA/0x55555555 */
    CPSS_DIAG_TEST_AA_55_E

} CPSS_DIAG_TEST_PROFILE_ENT;

/**
* @enum CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_ENT
 *
 * @brief This enum defines memory error injection type
*/
typedef enum{

    /** @brief single bit of the data
     *  will be inverted in the next write transaction.
     */
    CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_SINGLE_E,

    /** @brief two bits of the data
     *  will be inverted in the next write transaction.
     */
    CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_MULTIPLE_E

} CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_ENT;


/**
* @enum CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT
 *
 * @brief This enum defines memory error protection type
*/
typedef enum{

    /** @brief not protected */
    CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E,

    /** @brief ECC protected */
    CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ECC_E,

    /** @brief parity protected */
    CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_PARITY_E

} CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT;


/**
* @enum CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT
 *
 * @brief This enum defines ECC/parity error cause type
*/
typedef enum{

    /** @brief parity errors. */
    CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E,

    /** @brief single ECC errors. */
    CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E,

    /** @brief multiple ECC errors. */
    CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E,

    /** @brief single and multiple ECC errors. */
    CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E

} CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT;


/**
* @struct CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC
 *
 * @brief Memory location indexes
*/
typedef struct{

    /** @brief DFX pipe ID */
    GT_U32 dfxPipeId;

    /** @brief DFX client ID */
    GT_U32 dfxClientId;

    /** @brief DFX memory ID
     *  Comments:
     *  None
     */
    GT_U32 dfxMemoryId;

} CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC;

/**
* @struct CPSS_DIAG_DATA_INTEGRITY_MPPM_MEMORY_LOCATION_STC
 *
 * @brief MPPM Memory location indexes
*/
typedef struct{

    /** @brief port group ID */
    GT_U32 portGroupId;

    /** @brief MPPM ID */
    GT_U32 mppmId;

    /** @brief MPPM bank ID */
    GT_U32 bankId;

} CPSS_DIAG_DATA_INTEGRITY_MPPM_MEMORY_LOCATION_STC;

/**
* @enum CPSS_DIAG_DATA_INTEGRITY_MEMORY_USAGE_TYPE_ENT
*
* @brief Data Integrity types of 'Memory usage'
*/
typedef enum{

    /** @brief the CPSS hold no information about how to treat this memory usage. */
    CPSS_DIAG_DATA_INTEGRITY_MEMORY_USAGE_TYPE_UNKNOWN_E,

    /** @brief the memory is used for 'configurations' */
    CPSS_DIAG_DATA_INTEGRITY_MEMORY_USAGE_TYPE_CONFIGURATION_E,

    /** @brief the memory is used for 'data' (usually buffers) */
    CPSS_DIAG_DATA_INTEGRITY_MEMORY_USAGE_TYPE_DATA_E,

    /** @brief the memory is used for 'state' (usually internal headers,fifos,
     *  state machines,descriptors... )
     */
    CPSS_DIAG_DATA_INTEGRITY_MEMORY_USAGE_TYPE_STATE_E

} CPSS_DIAG_DATA_INTEGRITY_MEMORY_USAGE_TYPE_ENT;

/**
* @enum CPSS_DIAG_DATA_INTEGRITY_CORRECTION_METHOD_ENT
*
* @brief Data Integrity types of correction methods
*/
typedef enum{

    /** @brief The CPSS hold no information about how to treat this error. */
    CPSS_DIAG_DATA_INTEGRITY_CORRECTION_METHOD_UNKNOWN_E,

    /** @brief None (no correction method application can decide how to treat that) */
    CPSS_DIAG_DATA_INTEGRITY_CORRECTION_METHOD_NONE_E,

    /** @brief Drop/Resent (HW dropped packet/possibly resent other) */
    CPSS_DIAG_DATA_INTEGRITY_CORRECTION_METHOD_DROP_OR_RESEND_E,

    /** @brief HW correction (HW automatically fix this single error ECC) */
    CPSS_DIAG_DATA_INTEGRITY_CORRECTION_METHOD_HW_CORRECTION_E,

    /** @brief SW correction (SW can fix that memory, assuming it has mirror copy) */
    CPSS_DIAG_DATA_INTEGRITY_CORRECTION_METHOD_SW_CORRECTION_E,

    /** @brief Reboot chip (fatal error no way to fix that, PP must go reset) */
    CPSS_DIAG_DATA_INTEGRITY_CORRECTION_METHOD_REBOOT_CHIP_E,

    /** @brief Scrub (can be resolved by percase special SW handling) */
    CPSS_DIAG_DATA_INTEGRITY_CORRECTION_METHOD_SCRUB_E

} CPSS_DIAG_DATA_INTEGRITY_CORRECTION_METHOD_ENT;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssCommonDiagh__ */


