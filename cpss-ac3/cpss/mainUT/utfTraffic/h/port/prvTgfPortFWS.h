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
* @file prvTgfPortFWS.h
*
* @brief Port Full Wire Speed testing
*
* @version   5
********************************************************************************
*/
#ifndef __prvTgfPortFWSh
#define __prvTgfPortFWSh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfFWSLoopConfigTrafficGenerate function
* @endinternal
*
* @brief   Set configuration and generate full wire speed traffic.
*
* @param[in] firstVid                 - first VID for configuration
* @param[in] packetInfoPtr            - packet to send
* @param[in] configEnable             - GT_TRUE configure all needed setting and send traffic
*                                      - GT_FALSE do not change configuation and send traffic
*                                       None
*/
GT_STATUS  prvTgfFWSLoopConfigTrafficGenerate
(
    IN GT_U16               firstVid,
    IN TGF_PACKET_STC      *packetInfoPtr,
    IN GT_BOOL              configEnable
);

/**
* @internal prvTgfFWSRestore function
* @endinternal
*
* @brief   Stop FWS traffic and restore configuration.
*/
GT_VOID prvTgfFWSRestore
(
    GT_VOID
);

/**
* @internal prvTgfFWSTrafficSpeedMeasure function
* @endinternal
*
* @brief   Measure traffic speed for all ports.
*
* @param[in] granularityBitsNum       - amount of bits to shift left.
*                                      64-bit counter converted to 32-bit value by formula
*                                      val32 = (counter.l[0] >> granularityBitsNum) | (counter.l[1] << (32 - granularityBitsNum))
* @param[in] counterTypesNum          - number of MAC Counters types accomulated for measure.
* @param[in] counterTypesArr[]        - (pointer to))array of MAC Counters types accomulated for measure.
*
* @param[out] portRxRateArr[CPSS_MAX_PORTS_NUM_CNS] - Rx rate of port in 1024-bits per second
*
* @retval GT_OK                    - on OK
* @retval GT_FAIL                  - on failure
*/
GT_STATUS  prvTgfFWSTrafficSpeedMeasure
(
    IN   GT_U32                     granularityBitsNum,
    IN   GT_U32                     counterTypesNum,
    IN   CPSS_PORT_MAC_COUNTERS_ENT counterTypesArr[],
    OUT  GT_U32                     portRxRateArr[CPSS_MAX_PORTS_NUM_CNS]
);

/**
* @internal prvTgfFWSTrafficBandwidthCheck function
* @endinternal
*
* @brief   Check that traffic is FWS.
*
* @param[in] counterTypesNum          - number of MAC Counters types accomulated for measure.
* @param[in] counterTypesArr[]        - (pointer to))array of MAC Counters types accomulated for measure.
* @param[in] printRatesToLog          - GT_TRUE - print rates to log, GT_FALSE - no print rates.
*
* @retval GT_OK                    - on OK
* @retval GT_FAIL                  - on failure
*/
GT_STATUS  prvTgfFWSTrafficBandwidthCheck
(
  IN   GT_U32                     counterTypesNum,
  IN   CPSS_PORT_MAC_COUNTERS_ENT counterTypesArr[],
  IN   GT_BOOL                    printRatesToLog
);

/**
* @internal prvTgfFWSTrafficCheck function
* @endinternal
*
* @brief   Check that traffic is FWS.
*
* @param[in] packetSize               - packet size in bytes including CRC
* @param[in] logRateErrors            - GT_TRUE - log rate errors
*                                      GT_FALSE - do not log rate errors
*
* @param[out] portRxRateArr[CPSS_MAX_PORTS_NUM_CNS] - Rx rate of port in packets per second, may be NULL
*
* @retval GT_OK                    - on OK
* @retval GT_FAIL                  - on failure
*/
GT_STATUS  prvTgfFWSTrafficCheck
(
  IN   GT_U32  packetSize,
  IN   GT_BOOL logRateErrors,
  OUT  GT_U32  portRxRateArr[CPSS_MAX_PORTS_NUM_CNS]
);

/**
* @internal prvTgfFWSTrafficRatePrint function
* @endinternal
*
* @brief   Print traffic.
*
* @param[in] packetSize               - packet size in bytes including CRC
* @param[in] portRxRateArr[CPSS_MAX_PORTS_NUM_CNS] - Rx rate of port in packets per second
*
* @retval GT_OK                    - on OK
* @retval GT_FAIL                  - on failure
*/
GT_VOID  prvTgfFWSTrafficRatePrint
(
  IN  GT_U32  packetSize,
  IN  GT_U32  portRxRateArr[CPSS_MAX_PORTS_NUM_CNS]
);

/**
* @internal prvTgfFWSTrafficStop function
* @endinternal
*
* @brief   Stop FWS traffic but do not change configuration
*/
GT_STATUS prvTgfFWSTrafficStop
(
    GT_VOID
);

/**
* @internal prvTgfFWSPortsNumGet function
* @endinternal
*
* @brief   Get number of ports participating in FWS test
*/
GT_U32 prvTgfFWSPortsNumGet
(
    GT_VOID
);

/**
* @internal prvTgfPortFWSSkipPort function
* @endinternal
*
* @brief   Debug tool to remove or add ports from/to the FWS test
*
* @param[in] portNum                  - port number to remove or add from test
* @param[in] enable                   - remove or add port from/to test
*                                       None
*/
GT_U32  prvTgfPortFWSSkipPort
(
    GT_PHYSICAL_PORT_NUM    portNum,
    GT_U32 enable
);

/**
* @internal prvTgfFWSTraffCount function
* @endinternal
*
* @brief   Set burst count and number of iterations in the FWS test
*
* @param[in] newBurst                 - new burst count
* @param[in] newIter                  - new iterations count
*                                       old burst count
*/
GT_U32 prvTgfFWSTraffCount
(
    IN GT_U32 newBurst,
    IN GT_U32 newIter
);

/*******************************************************************************
* PRV_TGF_UNDER_FWS_CB_FUNC
*
* DESCRIPTION:
*     Call back for FWS function TYPEDEF for prvTgfFwsGenericTestUnderFws()
*
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*
*******************************************************************************/
typedef void (*PRV_TGF_UNDER_FWS_CB_FUNC)
(
  void
);

/**
* @internal prvTgfFwsGenericTestUnderFws function
* @endinternal
*
* @brief   This function call the cb function (test) under FWS and stop the FWS after that
*         1) generate FWS
*         2) checks the rate
*         3) call the CB function -- see cbFuncPtr
*         4)stop the traffic
*         when using this function the caller test have to use prvTgfFWSRestore when restore.
* @param[in] cbFuncPtr                - pointer to CB function to be called under FWS.  when NULL the function
*                                      just generate traffic without stop it.
*                                       None
*/
GT_VOID prvTgfFwsGenericTestUnderFws
(
    IN PRV_TGF_UNDER_FWS_CB_FUNC cbFuncPtr
);

/**
* @internal prvTgfFwsGenericFullBandwidthTest function
* @endinternal
*
* @brief   This function call the cb function (test) under FWS and stop the FWS after that
*         1) generate Full Bandwidth traffic
*         2) checks bandwidth
*         3) call the CB function -- see cbFuncPtr
*         4)stop the traffic
*         when using this function the caller test have to use prvTgfFWSRestore when restore.
* @param[in] cbFuncPtr                - pointer to CB function to be called under FWS. when NULL the function
*                                      just generate traffic without stop it.
*                                       None
*/
GT_VOID prvTgfFwsGenericFullBandwidthTest
(
    IN PRV_TGF_UNDER_FWS_CB_FUNC cbFuncPtr
);

/**
* @internal prvTgfFWSUseFloodingSet function
* @endinternal
*
* @brief   Configure FWS engine to use either known unicast or flooded traffic
*
* @param[in] status                   -- 0 - known unicast traffic is used
*                                      1 - flooded traffic is used
*                                       None
*/
GT_VOID prvTgfFWSUseFloodingSet
(
  IN GT_U32 status
);

/**
* @internal prvTgfPortFwsSkipPortsSet function
* @endinternal
*
* @brief   when prvTgfPortFwsSkipPorts = 0 ,prvTgfPortFWSSkipPort clear the ports bit map
*
* @param[in] status                   -- set status
*                                       None
*/
GT_VOID prvTgfPortFwsSkipPortsSet
(
    IN GT_U32 status
);

/**
* @internal prvTgfPortFwsSkipPortsGet function
* @endinternal
*
* @brief   when prvTgfPortFwsSkipPorts = 0 ,prvTgfPortFWSSkipPort clear the ports bit map
*
* @param[out] statusPtr                -- get status
*                                       None
*/
GT_VOID prvTgfPortFwsSkipPortsGet
(
   OUT GT_U32 *statusPtr
);

/**
* @internal prvTgfPortFwsSkipPortsBmpSet function
* @endinternal
*
* @brief   set prvTgfPortFwsSkipPortsBmp
*
* @param[in] bmapStatusPtr            -- set value
*                                       None
*/
GT_VOID prvTgfPortFwsSkipPortsBmpSet
(
  IN CPSS_PORTS_BMP_STC *bmapStatusPtr
);


/**
* @internal prvTgfPortFwsSkipPortsBmpGet function
* @endinternal
*
* @brief   set prvTgfPortFwsSkipPortsBmp
*
* @param[out] origBmapStatusPtr        -- get value
*                                       None
*/
GT_VOID prvTgfPortFwsSkipPortsBmpGet
(
   OUT CPSS_PORTS_BMP_STC *origBmapStatusPtr
);

/**
* @internal prvTgfPclFwsUseSeparateTcamBlockPerLookupGet function
* @endinternal
*
* @brief   Checks if need use separate TCAM block per PCL lookup.
*          TCAM BW for Aldrin2 and BC3 supports only one lookup per block in FWS.
*          Separate blocks must be used for such devices.
*
* @retval GT_FALSE                - same TCAM block may be used
* @retval GT_TRUE                 - separate TCAM blocks must be used
*
*/
GT_BOOL prvTgfPclFwsUseSeparateTcamBlockPerLookupGet(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfPortFWSh */



