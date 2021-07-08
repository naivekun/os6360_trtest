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
* @file cpssPcl.h
*
* @brief Common PCL definitions.
*
*
* @version   6
********************************************************************************
*/
#ifndef __cpssPclh
#define __cpssPclh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @enum CPSS_PCL_DIRECTION_ENT
 *
 * @brief PCL direction.
*/
typedef enum{

    /** The PCL will work on ingress traffic. */
    CPSS_PCL_DIRECTION_INGRESS_E = 0,

    /** The PCL will work on egress traffic. */
    CPSS_PCL_DIRECTION_EGRESS_E

} CPSS_PCL_DIRECTION_ENT;

/**
* @enum CPSS_PCL_LOOKUP_NUMBER_ENT
 *
 * @brief Number of lookup for the PCL
*/
typedef enum{

    /** @brief first lookup #0.
     *  CPSS_PCL_LOOKUP_0_E using means to configure both.
     */
    CPSS_PCL_LOOKUP_0_E = 0,

    /** second lookup #1. */
    CPSS_PCL_LOOKUP_1_E,

    /** @brief the 0-th sub lookup of lookup #0
     *  (APPLICABLE DEVICES: xCat2)
     */
    CPSS_PCL_LOOKUP_0_0_E,

    /** @brief the 1-th sub lookup of lookup #0
     *  (APPLICABLE DEVICES: xCat2)
     *  The values below are aliases for old names that are more suitable
     *  for Functional Specification documents terminology of Bobcat2 and above PPs.
     *  xCat2 and Lion2 LOOKUP_0_0 and LOOKUP_0_1
     *  already have no shared parameters in HW, but terminology yet is old and
     */
    CPSS_PCL_LOOKUP_0_1_E,

    /** Lookup 0, Alias for CPSS_PCL_LOOKUP_0_0_E */
    CPSS_PCL_LOOKUP_NUMBER_0_E = CPSS_PCL_LOOKUP_0_0_E,

    /** Lookup 1, Alias for CPSS_PCL_LOOKUP_0_1_E */
    CPSS_PCL_LOOKUP_NUMBER_1_E = CPSS_PCL_LOOKUP_0_1_E,

    /** Lookup 2, Alias for CPSS_PCL_LOOKUP_1_E */
    CPSS_PCL_LOOKUP_NUMBER_2_E = CPSS_PCL_LOOKUP_1_E

} CPSS_PCL_LOOKUP_NUMBER_ENT;

/**
* @enum CPSS_PCL_RULE_SIZE_ENT
 *
 * @brief This enum describes sizes of Policy Rules.
*/
typedef enum{

    /** standard size rule */
    CPSS_PCL_RULE_SIZE_STD_E,

    /** @brief 30-bytes rule - synonim of CPSS_PCL_RULE_SIZE_STD_E
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_PCL_RULE_SIZE_30_BYTES_E = CPSS_PCL_RULE_SIZE_STD_E,

    /** extended size rule */
    CPSS_PCL_RULE_SIZE_EXT_E,

    /** @brief 60-bytes rule - synonim of CPSS_PCL_RULE_SIZE_EXT_E
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_PCL_RULE_SIZE_60_BYTES_E = CPSS_PCL_RULE_SIZE_EXT_E,

    /** @brief ultra size rule
     *  (APPLICABLE DEVICES: Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_PCL_RULE_SIZE_ULTRA_E,

    /** @brief 80-bytes rule - synonim of CPSS_PCL_RULE_SIZE_ULTRA_E
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_PCL_RULE_SIZE_80_BYTES_E = CPSS_PCL_RULE_SIZE_ULTRA_E,

    /** @brief 10-bytes rule
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_PCL_RULE_SIZE_10_BYTES_E,

    /** @brief 20-bytes rule
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_PCL_RULE_SIZE_20_BYTES_E,

    /** @brief 40-bytes rule
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_PCL_RULE_SIZE_40_BYTES_E,

    /** @brief 50-bytes rule
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    CPSS_PCL_RULE_SIZE_50_BYTES_E

} CPSS_PCL_RULE_SIZE_ENT;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssPclh */


