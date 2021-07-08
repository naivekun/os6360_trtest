#include <Copyright.h>
/**
********************************************************************************
* @file gtPIRL.h
*
* @brief API/Structure definitions for Marvell PIRL functionality.
*
* @version   /
********************************************************************************
*/
/*******************************************************************************
* gtPIRL.h
*
* DESCRIPTION:
*       API/Structure definitions for Marvell PIRL functionality.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*******************************************************************************/

#ifndef __prvCpssDrvGtPIRL_h
#define __prvCpssDrvGtPIRL_h

#include <msApiTypes.h>
#include <gtSysConfig.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/* Exported PIRL Types                                                        */
/******************************************************************************/

/**
* @enum GT_CPSS_PIRL_COUNT_MODE
 *
 * @brief Enumeration of the port egress rate limit counting mode.
*/
typedef enum{

    /** Count the number of frames */
    GT_CPSS_PIRL_COUNT_FRAME = 0,

    /** @brief Count all Layer 1 bytes:
     *  Preamble (8bytes) + Frame's DA to CRC + IFG (12bytes)
     */
    GT_CPSS_PIRL_COUNT_ALL_LAYER1,

    /** Count all Layer 2 bytes: Frame's DA to CRC */
    GT_CPSS_PIRL_COUNT_ALL_LAYER2,

    /** @brief Count all Layer 3 bytes:
     *  Frame's DA to CRC - 18 - 4 (if frame is tagged)
     */
    GT_CPSS_PIRL_COUNT_ALL_LAYER3

} GT_CPSS_PIRL_COUNT_MODE;

/*
 *  typedef: struct GT_CPSS_PIRL_CUSTOM_RATE_LIMIT
 *
 *  Description:  This structure includes all the necessary parameters used to set port
 *                ingress rate limit. It's the output of API 'gpirlCustomSetup' based on users'
 *                target rate and target burst allocation. And also, this structure can be used
 *                in API 'prvCpssDrvGpirlWriteResource' to configure IRL port resource. You can check IRL
 *                sample code for detail usage. If the real Ingress Rate Limit is too much off
 *                from the target rate, please contact FAE and gets the correct parameters.
 *
 *        isValid         - Reserved for future use
 *        ebsLimit        - Excess Burst Size limit on the Yellow bucket ( 0 ~ 0xFFFFFF)
 *                          In Single Rate Two Color Mode, EBSLimit must be cleared to zero.
 *        cbsLimit        - Committed Burst Size limit on the Green bucket
 *        bktIncrement    - the amount of tokens that need to be added for each
 *                          byte of packet information.
 *        bktRateFactorGrn   - the amount of tokens that need to be decremented for each rate
 *                             resource decrement on the Green Bucket.
 *                             The rate for Green is:
 *                                  Rate in bps: Calculated Igress Rate(bps) = (500,000,000 * BRFGrn)/BktInc
 *                                  Rate if fps: Calculated Igress Rate(fps) = (62,000,000 * BRFGrn)/BktInc
 *        bktRateFactorYel   - the amount of tokens that need to be decremented for each rate
 *                             resource decrement on the Yellow Bucket.
 *                             In Single Rate Two Color Mode, BRFYel must be cleared to zero.
 *                             In Single Rate Three Color Mode, BRFYel must be equal to BRFGrn. Both must be non-zero.
 *                                  The rate for (Green + Yellow) is:
 *                                      Rate in bps: Calculated Igress Rate(bps) = (500,000,000 * BRFYel)/BktInc
 *                                      Rate if fps: Calculated Igress Rate(fps) = (62,000,000 * BRFTel)/BktInc
 *                               In Dual Rate Three Color Mode, BRFYel must be greater than BRFGrn
 *                                  The Yellow rate limit is:
 *                                      Rate in bps: Calculated Igress Rate(bps) = (500,000,000 * (BRFYel-BRFGrn)/BktInc
 *                                      Rate if fps: Calculated Igress Rate(fps) = (62,000,000 * (BRFYel-BRFGrn)/BktInc
 *        Delta           - the error ratio between Calculated Rate and Target Rate((Rate - TgtRate)/TgtRate)
 *        countMode          - bytes to be counted for accounting
 *                          GT_PIRL2_COUNT_FRAME,
 *                          GT_PIRL2_COUNT_ALL_LAYER1,
 *                          GT_PIRL2_COUNT_ALL_LAYER2, or
 *                          GT_PIRL2_COUNT_ALL_LAYER3
*/
typedef struct
{
    GT_BOOL       isValid;
    GT_U32        ebsLimit;
    GT_U32        cbsLimit;
    GT_U32        bktIncrement;
    GT_U32        bktRateFactorGrn;
    GT_U32        bktRateFactorYel;
    GT_DOUBLE     Delta;

    /*Count Mode*/
    GT_CPSS_PIRL_COUNT_MODE       countMode;

} GT_CPSS_PIRL_CUSTOM_RATE_LIMIT;

/**
* @enum GT_CPSS_PIRL_FC_DEASSERT
 *
 * @brief Enumeration of the port flow control de-assertion mode on PIRL.
*/
typedef enum{

    /** @brief Deassert when the ingress rate resource has become empty
     *  GT_CPSS_PIRL_FC_DEASSERT_CBS_LIMIT -
     *  De-assert when the ingress rate resource has enough room as
     *  specified by the CBSLimit.
     */
    GT_CPSS_PIRL_FC_DEASSERT_EMPTY = 0,

    GT_CPSS_PIRL_FC_DEASSERT_EBS_LIMIT

} GT_CPSS_PIRL_FC_DEASSERT;

/**
* @enum GT_CPSS_PIRL_ACTION
 *
 * @brief Defines the Action that should be taken when
 * there there are not enough tokens to accept the entire incoming frame
*/
typedef enum{

    /** drop packets */
    PIRL_ACTION_DROP   = 0x0,

    /** end flow control packet */
    PIRL_ACTION_FC     = 0x1

} GT_CPSS_PIRL_ACTION;

/**
* @struct GT_CPSS_PIRL_DATA
 *
 * @brief data structure for PIRL Bucket programing that is resource based.
*/
typedef struct{

    /** various traffic type to be counted */
    GT_U16 bktTypeMask;

    /** indicate to account for tcam flows 0 to 3, but not TCP/UDP data */
    GT_BOOL tcamFlows;

    /** @brief determine the incoming frames that get rate limited using
     *  this ingress rate resource.
     *  GT_TRUE - typeMask And priMask
     *  GT_FALSE - typeMask Or priMask
     */
    GT_BOOL priAndpt;

    /** @brief used to determine what priority type this resource is to account for
     *  via the Pri Select bits below as follows:
     *  GT_TRUE - use the FPri assigned to the frame
     *  GT_FALSE - use the QPri assigned to the frame
     */
    GT_BOOL useFPri;

    /** @brief priority bit mask that each bit indicates one of the eight
     *  priorities. Setting each one of these bits indicates
     *  that this particular rate resource is slated to account for
     *  incoming frames with the enabled bits' priority.
     */
    GT_U8 priSelect;

    /** set to color aware mode or color blind mode */
    GT_BOOL colorAware;

    /** Green&Yellow buckets are connected when in Coloar Aware mode. */
    GT_BOOL accountGrnOverflow;

    /** account discarded frames due to queue congestion */
    GT_BOOL accountQConf;

    /** account filtered frames. Account for All */
    GT_BOOL accountFiltered;

    /** sample one out of so many frames/bytes for a stream of frames */
    GT_BOOL samplingMode;

    /** @brief action should be taken when there are not enough tokens
     *  to accept the entire incoming frame
     *  PIRL_ACTION_DROP - drop frame
     *  PIRL_ACTION_FC - send flow control mode.
     */
    GT_CPSS_PIRL_ACTION actionMode;

    /** @brief flow control de
     *  set to PIRL_ACTION_FC.
     *  GT_CPSS_PIRL_FC_DEASSERT_EMPTY -
     *  De-assert when the ingress rate resource has
     *  become empty.
     *  GT_CPSS_PIRL_FC_DEASSERT_CBS_LIMIT -
     *  De-assert when the ingress rate resource has
     *  enough room as specified by the EBSLimit.
     */
    GT_CPSS_PIRL_FC_DEASSERT fcMode;

    /** @brief When the ingress port associated with this resource is enabled
     *  for Priority Flow Control and if this resource is configured to
     *  use FC Action, then this register is used to indicate the PFC
     *  priority that is to be used on the Priority flow Control frames
     *  generated due to this resource.
     */
    GT_U8 fcPriority;

    GT_CPSS_PIRL_CUSTOM_RATE_LIMIT customSetup;

} GT_CPSS_PIRL_DATA;

/******************************************************************************/
/* Exported PIRL Functions                                                    */
/******************************************************************************/
/**
* @internal prvCpssDrvGpirlInitialize function
* @endinternal
*
* @brief   This routine initializes all PIRL Resources for all ports.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDrvGpirlInitialize
(
    IN  GT_CPSS_QD_DEV              *dev
);

/**
* @internal prvCpssDrvGpirlInitResource function
* @endinternal
*
* @brief   This routine initializes the selected PIRL Resource for a particular port.
*
* @param[in] irlPort                  - target logical port
* @param[in] irlRes                   - resource unit to be accessed
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGpirlInitResource
(
    IN  GT_CPSS_QD_DEV      *dev,
    IN  GT_U32              irlPort,
    IN  GT_U32              irlRes
);

/**
* @internal prvCpssDrvGpirlCustomSetup_sr2c function
* @endinternal
*
* @brief   This function Calculate CBS/EBS/BI/BRFGrn/BRFYel/Delta according to user
*         specific target rate, target burst size and countMode.
* @param[in] tgtRate                  - target rate (units is fps when
*                                      countMode = GT_CPSS_PIRL_COUNT_FRAME, otherwise units
*                                      is bps)
* @param[in] tgtBstSize               - target burst size (units is Byte)
* @param[in,out] customSetup              - countMode in  as input parameter
* @param[in,out] customSetup              - customer setup including
*                                      CBS/EBS/BI/BRFGrn/BRFYel/Delta/countMode
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - if invalid parameter is given
*
* @note We have the following contrain in calculate the final parameters:
*       CBS >= BktIncrement tgtBstSize
*
*/
GT_STATUS prvCpssDrvGpirlCustomSetup_sr2c
(
    IN    GT_CPSS_QD_DEV                    *dev,
    IN    GT_U32                            tgtRate,
    IN    GT_U32                            tgtBstSize,
    INOUT GT_CPSS_PIRL_CUSTOM_RATE_LIMIT    *customSetup
);

/**
* @internal prvCpssDrvGpirlReadResource function
* @endinternal
*
* @brief   This function read Resource bucket parameter from the given resource of port
*
* @param[in] irlPort                  - target logical port
* @param[in] irlRes                   - resource unit to be accessed
*
* @param[out] pirlData                 - IRL Resource data
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGpirlReadResource
(
    IN  GT_CPSS_QD_DEV      *dev,
    IN  GT_CPSS_LPORT       irlPort,
    IN  GT_U32              irlRes,
    OUT GT_CPSS_PIRL_DATA   *pirlData
);

/**
* @internal prvCpssDrvGpirlWriteResource function
* @endinternal
*
* @brief   This function writes Resource bucket parameter to the given resource of port
*
* @param[in] irlPort                  - target port
* @param[in] irlRes                   - resource unit to be accessed
* @param[in] pirlData                 - IRL Resource data
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGpirlWriteResource
(
    IN  GT_CPSS_QD_DEV      *dev,
    IN  GT_CPSS_LPORT       irlPort,
    IN  GT_U32              irlRes,
    IN  GT_CPSS_PIRL_DATA   *pirlData
);

/**
* @internal prvCpssDrvGpirlGetResReg function
* @endinternal
*
* @brief   This routine read general register value from the given resource of the port.
*
* @param[in] irlPort                  - logical port number
* @param[in] irlRes                   - target resource
* @param[in] regOffset                - register address
*
* @param[out] data                     - register value
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGpirlGetResReg
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   irlPort,
    IN  GT_U32          irlRes,
    IN  GT_U32          regOffset,
    OUT GT_U16          *data
);

/**
* @internal prvCpssDrvGpirlSetResReg function
* @endinternal
*
* @brief   This routine set general register value to the given resource of the port..
*
* @param[in] irlPort                  - logical port number
* @param[in] irlRes                   - target resource
* @param[in] regOffset                - register address
* @param[in] data                     - register value
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGpirlSetResReg
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   irlPort,
    IN  GT_U32          irlRes,
    IN  GT_U32          regOffset,
    IN  GT_U16          data
);

#ifdef __cplusplus
}
#endif

#endif /* __prvCpssDrvGtPIRL_h */

