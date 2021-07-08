#include <Copyright.h>
/*******************************************************************************
* gtPhy.h
*
* DESCRIPTION:
*       API definitions for Marvell Phy functionality.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*******************************************************************************/

#ifndef __prvCpssDrvGtVct_h
#define __prvCpssDrvGtVct_h

#include <msApiTypes.h>
#include <gtSysConfig.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Definition for Advance Virtual Cable Test */

/**
* @enum GT_CPSS_RX_CHANNEL
 *
 * @brief Enumeration of Receiver Channel Assignment
*/
typedef enum{

    /** Channel A */
    GT_CPSS_CHANNEL_A,

    /** Channel B */
    GT_CPSS_CHANNEL_B,

    /** Channel C */
    GT_CPSS_CHANNEL_C,

    /** Channel D */
    GT_CPSS_CHANNEL_D

} GT_CPSS_RX_CHANNEL;

/**
* @enum GT_CPSS_POLARITY_STATUS
 *
 * @brief Enumeration of polarity status
*/
typedef enum{

    /** positive polarity */
    GT_CPSS_POSITIVE,

    /** negative polarity */
    GT_CPSS_NEGATIVE

} GT_CPSS_POLARITY_STATUS;

#define GT_CPSS_MDI_PAIR_NUM         4    /* (1,2),(3,6),(4,5),(7,8) */
#define GT_CPSS_CHANNEL_PAIR_NUM     2    /* (channel A,B),(channel C,D) */

/**
* @enum GT_CPSS_ADV_VCT_TRANS_CHAN_SEL
 *
 * @brief Enumeration of Advanced VCT Transmitter channel select
*/
typedef enum{

    GT_CPSS_ADV_VCT_TCS_NO_CROSSPAIR    = 0,

    GT_CPSS_ADV_VCT_TCS_CROSSPAIR_0     = 0x4,

    GT_CPSS_ADV_VCT_TCS_CROSSPAIR_1     = 0x5,

    GT_CPSS_ADV_VCT_TCS_CROSSPAIR_2     = 0x6,

    GT_CPSS_ADV_VCT_TCS_CROSSPAIR_3     = 0x7

} GT_CPSS_ADV_VCT_TRANS_CHAN_SEL;


typedef enum
{
    /* Advanced VCT Mode */
    GT_CPSS_ADV_VCT_SAVG_2      = 0,
    GT_CPSS_ADV_VCT_SAVG_4      = 1,
    GT_CPSS_ADV_VCT_SAVG_8      = 2,
    GT_CPSS_ADV_VCT_SAVG_16     = 3,
    GT_CPSS_ADV_VCT_SAVG_32     = 4,
    GT_CPSS_ADV_VCT_SAVG_64     = 5,
    GT_CPSS_ADV_VCT_SAVG_128    = 6,
    GT_CPSS_ADV_VCT_SAVG_256    = 7
} GT_CPSS_ADV_VCT_SAMPLE_AVG;

typedef enum
{
    /* Advanced VCT Mode */
    GT_CPSS_ADV_VCT_MAX_PEAK        = 0x00,
    GT_CPSS_ADV_VCT_FIRST_PEAK      = 0x01,
    GT_CPSS_ADV_VCT_OFFSET          = 0x02,
    GT_CPSS_ADV_VCT_SAMPLE_POINT    = 0x03
#if 0
    /* Advanced VCT Transmiter Channel Select */
    GT_CPSS_ADV_VCT_NO_CROSSPAIR    = 0x04,  /* 23_5 bit 13:11 = 000 */
    GT_CPSS_ADV_VCT_CROSSPAIR       = 0x08   /* 23_5 bit 13:11 = 100, 101, 110, 111 */
#endif
} GT_CPSS_ADV_VCT_MOD;

typedef unsigned int GT_CPSS_ADV_VCT_PEAKDET_HYST;

/*
 * typedef: enum GT_CPSS_ADV_VCT_MODE
 *
 * Description: Enumeration of Advanced VCT Mode and Transmitter channel select
 *
 * Enumerations:
 *      GT_CPSS_ADV_VCT_FIRST_PEAK   - first peak above a certain threshold is reported.
 *      GT_CPSS_ADV_VCT_MAX_PEAK     - maximum peak above a certain threshold is reported.
 *      GT_CPSS_ADV_VCT_OFFSE         - offset
 *      GT_CPSS_ADV_VCT_SAMPLE_POINT - sample point
 */
typedef struct
{
    GT_CPSS_ADV_VCT_MOD             mode;
    GT_CPSS_ADV_VCT_TRANS_CHAN_SEL  transChanSel;
    GT_CPSS_ADV_VCT_SAMPLE_AVG      sampleAvg;
    GT_CPSS_ADV_VCT_PEAKDET_HYST    peakDetHyst;
    GT_16                           samplePointDist;
} GT_CPSS_ADV_VCT_MODE;

/**
* @enum GT_CPSS_ADV_VCT_STATUS
 *
 * @brief Enumeration of Advanced VCT status
*/
typedef enum{

    /** @brief advanced virtual cable test failed.
     *  cable lengh cannot be determined.
     */
    GT_CPSS_ADV_VCT_FAIL,

    /** @brief normal cable.
     *  cable lengh may not be determined.
     */
    GT_CPSS_ADV_VCT_NORMAL,

    /** @brief impedance mismatch > 115 ohms.
     *  cable lengh is valid.
     */
    GT_CPSS_ADV_VCT_IMP_GREATER_THAN_115,

    /** @brief impedance mismatch < 85 ohms.
     *  cable lengh is valid.
     */
    GT_CPSS_ADV_VCT_IMP_LESS_THAN_85,

    /** @brief cable open.
     *  cable lengh is valid.
     */
    GT_CPSS_ADV_VCT_OPEN,

    /** @brief cable shorted.
     *  cable lengh is valid.
     */
    GT_CPSS_ADV_VCT_SHORT,

    /** @brief cross pair short.
     *  cable lengh for each channel is valid.
     */
    GT_CPSS_ADV_VCT_CROSS_PAIR_SHORT

} GT_CPSS_ADV_VCT_STATUS;


/*
 * typedef: struct GT_CROSS_PAIR_LIST
 *
 * Description: strucuture for cross pair short channels.
 *
 * Fields:
 *      channel - cross pair short channel list
 *                channel[i] is GT_TRUE if the channel[i] is cross pair short
 *                with the current channel under test.
 *      dist2fault - estimated distance to the shorted location.
 *                   valid only if related channel (above) is GT_TRUE.
 */
typedef struct _GT_CROSS_SHORT_LIST
{
    GT_BOOL     channel[GT_CPSS_MDI_PAIR_NUM];
    GT_16       dist2fault[GT_CPSS_MDI_PAIR_NUM];
} GT_CPSS_CROSS_SHORT_LIST;

/**
* @struct GT_CPSS_ADV_CABLE_STATUS
 *
 * @brief strucuture for advanced cable status.
*/
typedef struct
{
    GT_CPSS_ADV_VCT_STATUS   cableStatus[GT_CPSS_MDI_PAIR_NUM];
    union {
        GT_CPSS_CROSS_SHORT_LIST crossShort;
        GT_16     dist2fault;
        GT_16     offset;
        GT_16     sampleAmp;
    }u[GT_CPSS_MDI_PAIR_NUM];
} GT_CPSS_ADV_CABLE_STATUS;

/**
* @struct GT_CPSS_ADV_EXTENDED_STATUS
 *
 * @brief Currently the 1000Base-T PCS can determine the cable polarity
 * on pairs A,B,C,D; crossover on pairs A,B and C,D; and skew among
 * the pares. These status enhance the capability of the virtual cable tester
*/
typedef struct{

    /** @brief GT_TRUE if this structure have valid information,
     *  GT_FALSE otherwise.
     *  It is valid only if 1000BASE-T Link is up.
     */
    GT_BOOL isValid;

    GT_CPSS_RX_CHANNEL pairSwap[GT_CPSS_MDI_PAIR_NUM];

    GT_CPSS_POLARITY_STATUS pairPolarity[GT_CPSS_MDI_PAIR_NUM];

    GT_U32 pairSkew[GT_CPSS_MDI_PAIR_NUM];

    GT_U32 cableLen[GT_CPSS_MDI_PAIR_NUM];

} GT_CPSS_ADV_EXTENDED_STATUS;

typedef struct _GT_CPSS_SW_VCT_REGISTER
{
    GT_U8    page;
    GT_U8    regOffset;
} GT_CPSS_SW_VCT_REGISTER;


/**
* @internal prvCpssDrvGvctGetAdvCableDiag function
* @endinternal
*
* @brief   This routine perform the advanced virtual cable test for the requested
*         port and returns the the status per MDI pair.
* @param[in] port                     - logical  number.
* @param[in] mode                     - advance VCT  (either First Peak or Maximum Peak)
*
* @param[out] cableStatus              - the port copper cable status.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGvctGetAdvCableDiag
(
    IN  GT_CPSS_QD_DEV              *dev,
    IN  GT_CPSS_LPORT               port,
    IN  GT_CPSS_ADV_VCT_MODE        mode,
    OUT GT_CPSS_ADV_CABLE_STATUS    *cableStatus
);


/**
* @internal prvCpssDrvGvctGetAdvExtendedStatus function
* @endinternal
*
* @brief   This routine retrieves extended cable status, such as Pair Poloarity,
*         Pair Swap, and Pair Skew. Note that this routine will be success only
*         if 1000Base-T Link is up.
*         DSP based cable length is also provided.
* @param[in] dev                      - pointer to GT driver structure returned from mdLoadDriver
* @param[in] port                     - logical  number.
*
* @param[out] extendedStatus           - the extended cable status.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGvctGetAdvExtendedStatus
(
    IN  GT_CPSS_QD_DEV              *dev,
    IN  GT_CPSS_LPORT               port,
    OUT GT_CPSS_ADV_EXTENDED_STATUS *extendedStatus
);

#ifdef __cplusplus
}
#endif

#endif /* __prvCpssDrvGtVct_h */

