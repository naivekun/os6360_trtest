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
* @file cpssGenPhyVct.h
*
* @brief API definitions for CPSS Marvell Virtual Cable Tester functionality.
*
*
* @version   12
********************************************************************************
*/

#ifndef __cpssGenPhyVcth
#define __cpssGenPhyVcth

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>


/* maximum number of pairs in cable. Mostly: (1,2) (3,6) (4,5) (7,8)   */
#define CPSS_VCT_MDI_PAIR_NUM_CNS     4

/* number of channel pairs in cable: A/B and C/D */
#define CPSS_VCT_CHANNEL_PAIR_NUM_CNS 2

/**
* @enum CPSS_VCT_TEST_STATUS_ENT
 *
 * @brief Enumeration of VCT test status
*/
typedef enum{

    /** virtual cable test failed (can't run the test) */
    CPSS_VCT_TEST_FAIL_E,

    /** normal cable. */
    CPSS_VCT_NORMAL_CABLE_E,

    /** open in cable. */
    CPSS_VCT_OPEN_CABLE_E,

    /** short in cable. */
    CPSS_VCT_SHORT_CABLE_E,

    /** @brief impedance mismatch in cable
     *  (two cables of different quality is connected each other).
     */
    CPSS_VCT_IMPEDANCE_MISMATCH_E,

    /** short between Tx pair and Rx pair 0 */
    CPSS_VCT_SHORT_WITH_PAIR0_E,

    /** short between Tx pair and Rx pair 1 */
    CPSS_VCT_SHORT_WITH_PAIR1_E,

    /** short between Tx pair and Rx pair 2 */
    CPSS_VCT_SHORT_WITH_PAIR2_E,

    /** short between Tx pair and Rx pair 3 */
    CPSS_VCT_SHORT_WITH_PAIR3_E

} CPSS_VCT_TEST_STATUS_ENT;


/**
* @enum CPSS_VCT_NORMAL_CABLE_LEN_ENT
 *
 * @brief Enumeration for normal cable length
*/
typedef enum{

    /** cable length less than 50 meter. */
    CPSS_VCT_LESS_THAN_50M_E,

    /** cable length between 50 - 80 meter. */
    CPSS_VCT_50M_80M_E,

    /** cable length between 80 - 110 meter. */
    CPSS_VCT_80M_110M_E,

    /** cable length between 110 - 140 meter. */
    CPSS_VCT_110M_140M_E,

    /** cable length more than 140 meter. */
    CPSS_VCT_MORE_THAN_140_E,

    /** unknown length. */
    CPSS_VCT_UNKNOWN_LEN_E

} CPSS_VCT_NORMAL_CABLE_LEN_ENT;

/**
* @enum CPSS_VCT_PHY_TYPES_ENT
 *
 * @brief Enumeration of Marvel PHY types
*/
typedef enum{

    /** 10/100M phy, E3082 or E3083 */
    CPSS_VCT_PHY_100M_E = 0,

    /** Gigabit phy, the rest phys */
    CPSS_VCT_PHY_1000M_E,

    /** 10 Gigabit phy, unused */
    CPSS_VCT_PHY_10000M_E,

    /** Gigabit phy which needs work-around */
    CPSS_VCT_PHY_1000M_B_E,

    /** Multi Paged Gigabit phy, (E1112/E1149) */
    CPSS_VCT_PHY_1000M_MP_E,

    /** @brief Multi Paged Gigabit phy with no fiber
     *  support(i.e. E1149, revision C1)
     */
    CPSS_VCT_PHY_1000M_MP_NO_FIBER_E,

    /** @brief Multi Paged Gigabit phy with no fiber
     *  support, next generation
     *  (i.e. E1149R/E1240)
     */
    CPSS_VCT_PHY_1000M_MP_NO_FIBER_NG_E

} CPSS_VCT_PHY_TYPES_ENT;


/**
* @struct CPSS_VCT_STATUS_STC
 *
 * @brief Structure holding VCT results
*/
typedef struct{

    /** VCT test status. */
    CPSS_VCT_TEST_STATUS_ENT testStatus;

    /** for cable failure the estimate fault distance in meters. */
    GT_U8 errCableLen;

} CPSS_VCT_STATUS_STC;

/**
* @struct CPSS_VCT_CABLE_STATUS_STC
 *
 * @brief virtual cable diagnostic status per MDI pair.
*/
typedef struct{

    CPSS_VCT_STATUS_STC cableStatus[CPSS_VCT_MDI_PAIR_NUM_CNS];

    /** cable lenght for normal cable. */
    CPSS_VCT_NORMAL_CABLE_LEN_ENT normalCableLen;

    /** @brief type of phy (100M phy or Gigabit phy)
     *  Comments:
     *  If PHY is PHY_100M cableStatus will have only 2 pairs relevant.
     *  One is RX Pair (cableStatus[0] or cableLen[0]) and
     *  the other is TX Pair (cableStatus[1] or cableLen[1]).
     */
    CPSS_VCT_PHY_TYPES_ENT phyType;

} CPSS_VCT_CABLE_STATUS_STC;

/**
* @enum CPSS_VCT_PAIR_SWAP_ENT
 *
 * @brief Enumeration for pair swap
*/
typedef enum{

    /** @brief channel A on MDI[0] and B on MDI[1]
     *  or channel C on MDI[3] and D on MDI[4]
     */
    CPSS_VCT_CABLE_STRAIGHT_E,

    /** @brief channel B on MDI[0] and A on MDI[1]
     *  or channel D on MDI[3] and C on MDI[4]
     */
    CPSS_VCT_CABLE_CROSSOVER_E,

    /** in FE just two first channels are checked */
    CPSS_VCT_NOT_APPLICABLE_SWAP_E

} CPSS_VCT_PAIR_SWAP_ENT;

/**
* @enum CPSS_VCT_POLARITY_SWAP_ENT
 *
 * @brief Enumeration for pair polarity swap
*/
typedef enum{

    /** good polarity */
    CPSS_VCT_POSITIVE_POLARITY_E,

    /** reversed polarity */
    CPSS_VCT_NEGATIVE_POLARITY_E,

    /** @brief in FE there just one value for
     *  all the cable then just first pair is applicable
     */
    CPSS_VCT_NOT_APPLICABLE_POLARITY_E

} CPSS_VCT_POLARITY_SWAP_ENT;

/**
* @struct CPSS_VCT_PAIR_SKEW_STC
 *
 * @brief pair skew values.
*/
typedef struct{

    /** @brief whether results are valid
     *  (not valid for FE).
     */
    GT_BOOL isValid;

    GT_U32 skew[CPSS_VCT_MDI_PAIR_NUM_CNS];

} CPSS_VCT_PAIR_SKEW_STC;

/**
* @struct CPSS_VCT_EXTENDED_STATUS_STC
 *
 * @brief extended virtual cable diagnostic status per MDI pair/channel
 * pair.
*/
typedef struct{

    /** @brief if results are valid (if not, maybe there is no
     *  gigabit link for GE or 100M link for FE).
     */
    GT_BOOL isValid;

    CPSS_VCT_PAIR_SWAP_ENT pairSwap[CPSS_VCT_CHANNEL_PAIR_NUM_CNS];

    CPSS_VCT_POLARITY_SWAP_ENT pairPolarity[CPSS_VCT_MDI_PAIR_NUM_CNS];

    /** @brief the skew among the four pairs of the cable
     *  (delay between pairs in n-Seconds)
     *  Comments:
     *  If PHY is PHY_100M cableStatus will have only 2 pairs relevant.
     *  in this case:
     *  swap - just pairSwap[0] relevant
     *  polarity - just pairPolarity[0] and pairPolarity[1] are relevant
     *  skew - will be not relevant
     */
    CPSS_VCT_PAIR_SKEW_STC pairSkew;

} CPSS_VCT_EXTENDED_STATUS_STC;

/**
* @struct CPSS_VCT_ACCURATE_CABLE_LEN_STC
 *
 * @brief accurate cable length for each MDI pair.
*/
typedef struct{

    GT_BOOL isValid[CPSS_VCT_MDI_PAIR_NUM_CNS];

    GT_U16 cableLen[CPSS_VCT_MDI_PAIR_NUM_CNS];

} CPSS_VCT_ACCURATE_CABLE_LEN_STC;

/**
* @enum CPSS_VCT_DOWNSHIFT_STATUS_ENT
 *
 * @brief Enumeration for downshift status of the port.
 * When the Giga link cannot be established (for example there is
 * just two pairs is good) the downshift feature in the PHY can
 * down the link to 100MB link, i.e. downshift in link.
*/
typedef enum{

    /** no downshift */
    CPSS_VCT_NO_DOWNSHIFT_E,

    /** downshift was occurs */
    CPSS_VCT_DOWNSHIFT_E,

    /** in FE. */
    CPSS_VCT_NOT_APPLICABLE_DOWNSHIFT_E

} CPSS_VCT_DOWNSHIFT_STATUS_ENT;

/**
* @struct CPSS_VCT_CABLE_EXTENDED_STATUS_STC
 *
 * @brief extended virtual cable diag. status per MDI pair/channel pair.
*/
typedef struct{

    /** extended VCT cable status. */
    CPSS_VCT_EXTENDED_STATUS_STC vctExtendedCableStatus;

    /** @brief accurate cable lenght.
     *  accurateCableLen - not relevant
     */
    CPSS_VCT_ACCURATE_CABLE_LEN_STC accurateCableLen;

    /** @brief downshift status.
     *  Comments:
     *  If PHY is PHY_100M cableStatus will have only 2 pairs relevant.
     *  in this case:
     *  swap - just pairSwap[0] relevant
     *  polarity - just pairPolarity[0] and pairPolarity[1] are relevant
     *  skew - will be not relevant
     *  twoPairDownShift - not relevant
     */
    CPSS_VCT_DOWNSHIFT_STATUS_ENT twoPairDownShift;

} CPSS_VCT_CABLE_EXTENDED_STATUS_STC;

/**
* @enum CPSS_VCT_ACTION_ENT
 *
 * @brief VCT action to be done by the vctGetCableStat function.
*/
typedef enum{

    /** Initializes a new VCT test. */
    CPSS_VCT_START_E,

    /** Reads the results of actually runing test. */
    CPSS_VCT_GET_RES_E,

    /** Aborts runing test. */
    CPSS_VCT_ABORT_E

} CPSS_VCT_ACTION_ENT;

/**
* @enum CPSS_VCT_STATE
 *
 * @brief VCT state machine's states.
*/
typedef enum{

    /** No test is currently running. Ready for a new test. */
    CPSS_VCT_READY_E,

    /** VCT is asserted. */
    CPSS_VCT_STARTED_E,

    /** VCT is asserted. Asserting a second test. */
    CPSS_VCT_REDO_PHASE_1_E,

    /** VCT is asserted. Asserting a third test. */
    CPSS_VCT_REDO_PHASE_2_E

} CPSS_VCT_STATE;

/**
* @enum CPSS_VCT_CABLE_LEN_ENT
 *
 * @brief Enumeration to define types of cable length.
*/
typedef enum{

    /** Less than 10 meters. */
    CPSS_VCT_CABLE_LESS_10M_E,

    /** Greater than 10 meters. */
    CPSS_VCT_CABLE_GREATER_10M_E

} CPSS_VCT_CABLE_LEN_ENT;

/**
* @struct CPSS_VCT_CONFIG_STC
 *
 * @brief Holds cable length, configured by user.
*/
typedef struct{

    /** @brief Greater or less than 10m.
     *  Comments:
     */
    CPSS_VCT_CABLE_LEN_ENT length;

} CPSS_VCT_CONFIG_STC;
/**
* @internal cpssVctCableExtendedStatusGet function
* @endinternal
*
* @brief   This routine returns the extended VCT status per MDI pair/channel pair.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] port                     - physical  number
*
* @param[out] extendedCableStatusPtr   - extended virtual cable diagnostic status
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_SUPPORTED         - on non supported device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note If PHY is CPSS_VCT_PHY_100M_E (FE) cableStatusPtr will have only 2 pairs
*       relevant. In this case:
*       swap - just pairSwap[0] relevant
*       vctExtendedCableStatus.pairSwap[1] = CPSS_VCT_NOT_APPLICABLE_E
*       vctExtendedCableStatus.pairSwap[2] = CPSS_VCT_NOT_APPLICABLE_E
*       vctExtendedCableStatus.pairSwap[3] = CPSS_VCT_NOT_APPLICABLE_E
*       polarity - just pairPolarity[0] relevant
*       vctExtendedCableStatus.pairPolarity[1] =CPSS_VCT_NOT_APPLICABLE_E
*       vctExtendedCableStatus.pairPolarity[2] =CPSS_VCT_NOT_APPLICABLE_E
*       vctExtendedCableStatus.pairPolarity[3] =CPSS_VCT_NOT_APPLICABLE_E
*       skew - not relevant
*       vctExtendedCableStatus.skew[0,1,2,3].IsValid = GT_FALSE
*       accurateCableLen - not relevant
*       vctExtendedCableStatus.accurateCableLen.isValid = GT_FALSE
*       twoPairDownShift - not relevant
*       extendedCableStatusPtr->twoPairDownShift =
*       CPSS_VCT_NOT_APPLICABLE_DOWNSHIFT_E;
*       The API doesn't perform VCT test, but retrieves extended info regarding
*       the above features, which has collected by: cpssVctCableStatusGet API.
*
*/
GT_STATUS cpssVctCableExtendedStatusGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        port,
    OUT CPSS_VCT_CABLE_EXTENDED_STATUS_STC *extendedCableStatusPtr
);

/**
* @internal cpssVctCableStatusGet function
* @endinternal
*
* @brief   This routine asserts / aborts the state-machine based virtual cable test
*         (VCT) for the requested port. When test completes, it returns the the
*         status per MDI pair, otherwize it will return current test status.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dev                      - physical device number
* @param[in] port                     - physical  number
* @param[in] vctAction                - The VCT action to perform on this port.
*
* @param[out] cableStatusPtr           - virtual cable diagnostic status
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_READY             - when action is CPSS_VCT_GET_RES_E and the test is not
*                                       completed yet.
* @retval GT_NOT_SUPPORTED         - on non supported device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Scenario:
*       1)First call with CPSS_VCT_START_E to begin the test
*       2)Call the function with CPSS_VCT_GET_RES_E and wait till GT_OK as
*       relult.
*       If GT_NOT_READY is returned, call the CPSS_VCT_GET_RES_E again after a
*       while in order to let the test be completed.
*
*/
GT_STATUS cpssVctCableStatusGet
(
    IN  GT_U8 dev,
    IN  GT_PHYSICAL_PORT_NUM      port,
    IN  CPSS_VCT_ACTION_ENT       vctAction,
    OUT CPSS_VCT_CABLE_STATUS_STC *cableStatusPtr
);

/**
* @internal cpssVctSetCableDiagConfig function
* @endinternal
*
* @brief   This routine configures VCT diagnostic parameters per port:
*         - cable's length: greater or less then 10 meters.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dev                      - device number
* @param[in] port                     -  number
* @param[in] configPtr                - configuration parameters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong length configuration
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_SUPPORTED         - on non supported device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssVctSetCableDiagConfig
(
    IN  GT_U8         dev,
    IN  GT_PHYSICAL_PORT_NUM   port,
    IN  CPSS_VCT_CONFIG_STC *configPtr
);

/**
* @internal cpssVctLengthOffsetSet function
* @endinternal
*
* @brief   This function sets the offset used in VCT length calcualtions per port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dev                      - device number
* @param[in] port                     -  number
* @param[in] offset                   -  in milimeters.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note According to the PHYs' release notes the tuned formula to calculate
*       length in VCT is of the form:
*       A hwDist - B + offset
*       where:
*       A & B - constants published in the PHYs' release notes.
*       hwDist - the distance value from HW registers to the fault.
*       offset - fine tuning required due to different "MDI traces" on
*       different boards.
*       This function should be called after cpssDxChHwPpPhase1Init() and
*       before runing the VCT.
*
*/
GT_STATUS cpssVctLengthOffsetSet
(
    IN  GT_U8   dev,
    IN  GT_PHYSICAL_PORT_NUM   port,
    IN  GT_32  offset
);

/**
* @internal cpssVctLengthOffsetGet function
* @endinternal
*
* @brief   This function gets the offset used in VCT length calcualtions per port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dev                      - device number
* @param[in] port                     -  number
*
* @param[out] offsetPtr                - (pointer to)offset in milimeters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note According to the PHYs' release notes the tuned formula to calculate
*       length in VCT is of the form:
*       A hwDist - B + offset
*       where:
*       A & B - constants published in the PHYs' release notes.
*       hwDist - the distance value from HW registers to the fault.
*       offset - fine tuning required due to different "MDI traces" on
*       different boards.
*
*/
GT_STATUS cpssVctLengthOffsetGet
(
    IN  GT_U8   dev,
    IN  GT_PHYSICAL_PORT_NUM   port,
    OUT GT_32  *offsetPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssGenPhyVcth */


