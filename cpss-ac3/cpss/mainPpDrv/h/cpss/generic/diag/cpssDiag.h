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
* @file cpssDiag.h
*
* @brief General diagnostic definitions and data structures for PP.
*
* @version   6
********************************************************************************
*/
#ifndef __cpssDiagh
#define __cpssDiagh

#include <cpss/common/cpssTypes.h>
#include <cpss/common/diag/cpssCommonDiag.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @enum CPSS_DIAG_PP_MEM_TYPE_ENT
 *
 * @brief Packet Process memory type.
*/
typedef enum{

    /** @brief The external packet buffer
     *  SDRAM.
     *  (APPLICABLE DEVICES: ExMx; ExMxPm.)
     *  (APPLICABLE DEVICES: DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.)
     */
    CPSS_DIAG_PP_MEM_BUFFER_DRAM_E,

    /** @brief The external Wide SRAM, or
     *  internal Control SRAM if the
     *  device was configured
     *  without external wide-SRAM.
     *  (APPLICABLE DEVICES: ExMx.)
     */
    CPSS_DIAG_PP_MEM_WIDE_SRAM_E,

    /** @brief the external Narrow SRAM, or
     *  internal IP SRAM if the
     *  device was configured
     *  without external wide-SRAM.
     *  Not relevant for EX1x6.
     *  (APPLICABLE DEVICES: ExMx; ExMxPm.)
     */
    CPSS_DIAG_PP_MEM_NARROW_SRAM_E,

    /** @brief Internal Narrow SRAM 0
     *  (EX1x6 only)
     */
    CPSS_DIAG_PP_MEM_INTERNAL_NARROW_SRAM_0_E,

    /** @brief Internal Narrow SRAM 1
     *  (EX1x6 only)
     */
    CPSS_DIAG_PP_MEM_INTERNAL_NARROW_SRAM_1_E,

    /** @brief External Narrow SRAM 0
     *  (EX1x6 only)
     */
    CPSS_DIAG_PP_MEM_EXTERNAL_NARROW_SRAM_0_E,

    /** @brief External Narrow SRAM 1
     *  (EX1x6 only)
     */
    CPSS_DIAG_PP_MEM_EXTERNAL_NARROW_SRAM_1_E,

    /** @brief The external flow DRAM.
     *  (APPLICABLE DEVICES: ExMx.)
     */
    CPSS_DIAG_PP_MEM_FLOW_DRAM_E,

    /** @brief Internal MAC table memory.
     *  (APPLICABLE DEVICES: ExMx; ExMxPm.)
     *  (APPLICABLE DEVICES: DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.)
     */
    CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E,

    /** @brief Internal VLAN table memory.
     *  (APPLICABLE DEVICES: ExMx; ExMxPm.)
     *  (APPLICABLE DEVICES: DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.)
     */
    CPSS_DIAG_PP_MEM_VLAN_TBL_MEM_E

} CPSS_DIAG_PP_MEM_TYPE_ENT;


/**
* @enum CPSS_DIAG_DATA_INTEGRITY_TCAM_ARRAY_TYPE_ENT
 *
 * @brief TCAM array type.
*/
typedef enum{

    /** Array X. */
    CPSS_DIAG_DATA_INTEGRITY_TCAM_ARRAY_TYPE_X_E,

    /** Array Y. */
    CPSS_DIAG_DATA_INTEGRITY_TCAM_ARRAY_TYPE_Y_E

} CPSS_DIAG_DATA_INTEGRITY_TCAM_ARRAY_TYPE_ENT;

/**
* @struct CPSS_DIAG_DATA_INTEGRITY_TCAM_MEMORY_LOCATION_STC
 *
 * @brief TCAM Memory location indexes
*/
typedef struct{

    /** The type of the array that the parity error was detected on */
    CPSS_DIAG_DATA_INTEGRITY_TCAM_ARRAY_TYPE_ENT arrayType;

    /** @brief for Policy TCAM: index of the standard rule in the TCAM.
     *  See cpssDxChPclRuleSet
     *  for Router TCAM: line index in TCAM to write to (same as TTI index)
     *  Comments:
     *  None
     */
    GT_U32 ruleIndex;

} CPSS_DIAG_DATA_INTEGRITY_TCAM_MEMORY_LOCATION_STC;


/**
* @union CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_UNT
 *
 * @brief Holds the two possible memory location formats.
 *
*/

typedef union{
    /** regular memory location indexes */
    CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC memLocation;

    /** special memory location indexes for MPPM memory */
    CPSS_DIAG_DATA_INTEGRITY_MPPM_MEMORY_LOCATION_STC mppmMemLocation;

    /** special memory location indexes for TCAM memory */
    CPSS_DIAG_DATA_INTEGRITY_TCAM_MEMORY_LOCATION_STC tcamMemLocation;

} CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_UNT;



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDiagh */


