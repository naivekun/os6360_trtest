#include <Copyright.h>
/**
********************************************************************************
* @file gtPortRmon.h
*
* @brief API/Structure definitions for Marvell MIBS functionality.
*
* @version   /
********************************************************************************
*/
/*******************************************************************************
* gtPortRmon.h
*
* DESCRIPTION:
*       API/Structure definitions for Marvell MIBS functionality.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*******************************************************************************/

#ifndef __prvCpssDrvGtPortRmon_h
#define __prvCpssDrvGtPortRmon_h

#include <msApiTypes.h>
#include <gtSysConfig.h>

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************/
/* Exported MIBS Types                                                      */
/****************************************************************************/
/*
 *  typedef: struct GT_CPSS_STATS_COUNTERS
 *
 *  Description: MIB counter
 *
 */
#define GT_CPSS_TYPE_BANK 0x400
typedef enum
{
    /* Bank 0 */
    CPSS_STATS_InGoodOctetsLo = 0,
    CPSS_STATS_InGoodOctetsHi,
    CPSS_STATS_InBadOctets,

    CPSS_STATS_OutFCSErr,
    CPSS_STATS_InUnicasts,
    CPSS_STATS_Deferred,             /* offset 5 */
    CPSS_STATS_InBroadcasts,
    CPSS_STATS_InMulticasts,
    CPSS_STATS_64Octets,
    CPSS_STATS_127Octets,
    CPSS_STATS_255Octets,            /* offset 10 */
    CPSS_STATS_511Octets,
    CPSS_STATS_1023Octets,
    CPSS_STATS_MaxOctets,
    CPSS_STATS_OutOctetsLo,
    CPSS_STATS_OutOctetsHi,
    CPSS_STATS_OutUnicasts,          /* offset 16 */
    CPSS_STATS_Excessive,
    CPSS_STATS_OutMulticasts,
    CPSS_STATS_OutBroadcasts,
    CPSS_STATS_Single,
    CPSS_STATS_OutPause,
    CPSS_STATS_InPause,
    CPSS_STATS_Multiple,
    CPSS_STATS_InUndersize,          /* offset 24 */
    CPSS_STATS_InFragments,
    CPSS_STATS_InOversize,
    CPSS_STATS_InJabber,
    CPSS_STATS_InRxErr,
    CPSS_STATS_InFCSErr,
    CPSS_STATS_Collisions,
    CPSS_STATS_Late,                 /* offset 31 */
    /* Bank 1 */
    CPSS_STATS_InDiscards      = GT_CPSS_TYPE_BANK+0x00,
    CPSS_STATS_InFiltered      = GT_CPSS_TYPE_BANK+0x01,
    CPSS_STATS_InAccepted      = GT_CPSS_TYPE_BANK+0x02,
    CPSS_STATS_InBadAccepted   = GT_CPSS_TYPE_BANK+0x03,
    CPSS_STATS_InGoodAvbClassA = GT_CPSS_TYPE_BANK+0x04,
    CPSS_STATS_InGoodAvbClassB = GT_CPSS_TYPE_BANK+0x05,
    CPSS_STATS_InBadAvbClassA  = GT_CPSS_TYPE_BANK+0x06,
    CPSS_STATS_InBadAvbClassB  = GT_CPSS_TYPE_BANK+0x07,
    CPSS_STATS_TCAMCounter0    = GT_CPSS_TYPE_BANK+0x08,
    CPSS_STATS_TCAMCounter1    = GT_CPSS_TYPE_BANK+0x09,
    CPSS_STATS_TCAMCounter2    = GT_CPSS_TYPE_BANK+0x0a,
    CPSS_STATS_TCAMCounter3    = GT_CPSS_TYPE_BANK+0x0b,
    CPSS_STATS_InDroppedAvbA   = GT_CPSS_TYPE_BANK+0x0c,
    CPSS_STATS_InDroppedAvbB   = GT_CPSS_TYPE_BANK+0x0d,
    CPSS_STATS_InDaUnknown     = GT_CPSS_TYPE_BANK+0x0e,
    CPSS_STATS_InMGMT          = GT_CPSS_TYPE_BANK+0x0f,
    CPSS_STATS_OutQueue0       = GT_CPSS_TYPE_BANK+0x10,
    CPSS_STATS_OutQueue1       = GT_CPSS_TYPE_BANK+0x11,
    CPSS_STATS_OutQueue2       = GT_CPSS_TYPE_BANK+0x12,
    CPSS_STATS_OutQueue3       = GT_CPSS_TYPE_BANK+0x13,
    CPSS_STATS_OutQueue4       = GT_CPSS_TYPE_BANK+0x14,
    CPSS_STATS_OutQueue5       = GT_CPSS_TYPE_BANK+0x15,
    CPSS_STATS_OutQueue6       = GT_CPSS_TYPE_BANK+0x16,
    CPSS_STATS_OutQueue7       = GT_CPSS_TYPE_BANK+0x17,
    CPSS_STATS_OutCutThrough   = GT_CPSS_TYPE_BANK+0x18,
    CPSS_STATS_OutOctetsA      = GT_CPSS_TYPE_BANK+0x1a,
    CPSS_STATS_OutOctetsB      = GT_CPSS_TYPE_BANK+0x1b,
    CPSS_STATS_OutYel          = GT_CPSS_TYPE_BANK+0x1c,
    CPSS_STATS_OutDroppedYel   = GT_CPSS_TYPE_BANK+0x1d,
    CPSS_STATS_OutDiscards     = GT_CPSS_TYPE_BANK+0x1e,
    CPSS_STATS_OutMGMT         = GT_CPSS_TYPE_BANK+0x1f

} GT_CPSS_STATS_COUNTERS;

/*
 *  typedef: struct GT_CPSS_STATS_COUNTER_SET
 *
 *  Description: MIB Counter Set
 *
 */
typedef struct _GT_STATS_COUNTER_SET
{
    /* Bank 0 */
    GT_U32    InGoodOctetsLo;     /* offset 0 */
    GT_U32    InGoodOctetsHi;     /* offset 1 */
    GT_U32    InBadOctets;        /* offset 2 */
    GT_U32    OutFCSErr;          /* offset 3 */
    GT_U32    InUnicasts;         /* offset 4 */
    GT_U32    Deferred;           /* offset 5 */
    GT_U32    InBroadcasts;       /* offset 6 */
    GT_U32    InMulticasts;       /* offset 7 */
    /*
        Histogram Counters : Rx Only, Tx Only, or both Rx and Tx
        (refer to Histogram Mode)
    */
    GT_U32    Octets64;         /* 64 Octets, offset 8 */
    GT_U32    Octets127;        /* 65 to 127 Octets, offset 9 */
    GT_U32    Octets255;        /* 128 to 255 Octets, offset 10 */
    GT_U32    Octets511;        /* 256 to 511 Octets, offset 11 */
    GT_U32    Octets1023;       /* 512 to 1023 Octets, offset 12 */
    GT_U32    OctetsMax;        /* 1024 to Max Octets, offset 13 */
    GT_U32    OutOctetsLo;      /* offset 14 */
    GT_U32    OutOctetsHi;      /* offset 15 */
    GT_U32    OutUnicasts;      /* offset 16 */
    GT_U32    Excessive;        /* offset 17 */
    GT_U32    OutMulticasts;    /* offset 18 */
    GT_U32    OutBroadcasts;    /* offset 19 */
    GT_U32    Single;           /* offset 20 */

    GT_U32    OutPause;         /* offset 21 */
    GT_U32    InPause;          /* offset 22 */
    GT_U32    Multiple;         /* offset 23 */
    GT_U32    InUndersize;        /* offset 24 */
    GT_U32    InFragments;        /* offset 25 */
    GT_U32    InOversize;         /* offset 26 */
    GT_U32    InJabber;           /* offset 27 */
    GT_U32    InRxErr;          /* offset 28 */
    GT_U32    InFCSErr;         /* offset 29 */
    GT_U32    Collisions;       /* offset 30 */
    GT_U32    Late;             /* offset 31 */
    /* Bank 1 */
    GT_U32    InDiscards;       /* offset 0x00 */
    GT_U32    InFiltered;       /* offset 0x01 */
    GT_U32    InAccepted;       /* offset 0x02 */
    GT_U32    InBadAccepted;    /* offset 0x03 */
    GT_U32    InGoodAvbClassA;  /* offset 0x04 */
    GT_U32    InGoodAvbClassB;  /* offset 0x05 */
    GT_U32    InBadAvbClassA ;  /* offset 0x06 */
    GT_U32    InBadAvbClassB ;  /* offset 0x07 */
    GT_U32    TCAMCounter0;     /* offset 0x08 */
    GT_U32    TCAMCounter1;     /* offset 0x09 */
    GT_U32    TCAMCounter2;     /* offset 0x0a */
    GT_U32    TCAMCounter3;     /* offset 0x0b */
    GT_U32    InDroppedAvbA;    /* offset 0x0c */
    GT_U32    InDroppedAvbB;    /* offset 0x0d */
    GT_U32    InDaUnknown ;     /* offset 0x0e */
    GT_U32    InMGMT;           /* offset 0x0f */
    GT_U32    OutQueue0;        /* offset 0x10 */
    GT_U32    OutQueue1;        /* offset 0x11 */
    GT_U32    OutQueue2;        /* offset 0x12 */
    GT_U32    OutQueue3;        /* offset 0x13 */
    GT_U32    OutQueue4;        /* offset 0x14 */
    GT_U32    OutQueue5;        /* offset 0x15 */
    GT_U32    OutQueue6;        /* offset 0x16 */
    GT_U32    OutQueue7;        /* offset 0x17 */
    GT_U32    OutCutThrough;    /* offset 0x18 */
    GT_U32    reserved_19 ;     /* offset 0x19 */
    GT_U32    OutOctetsA;       /* offset 0x1a */
    GT_U32    OutOctetsB;       /* offset 0x1b */
    GT_U32    OutYel;           /* offset 0x1c */
    GT_U32    OutDroppedYel;    /* offset 0x1d */
    GT_U32    OutDiscards;      /* offset 0x1e */
    GT_U32    OutMGMT;          /* offset 0x1f */

} GT_CPSS_STATS_COUNTER_SET;

/**
* @enum GT_CPSS_HISTOGRAM_MODE
 *
 * @brief Enumeration of the histogram counters mode.
*/
typedef enum{

    /** In this mode, Rx Histogram Counters are counted. */
    GT_CPSS_COUNT_RX_ONLY = 1,

    /** In this mode, Tx Histogram Counters are counted. */
    GT_CPSS_COUNT_TX_ONLY,

    /** In this mode, Rx and Tx Histogram Counters are counted. */
    GT_CPSS_COUNT_RX_TX

} GT_CPSS_HISTOGRAM_MODE;

/****************************************************************************/
/* Exported MIBS Functions                                                  */
/****************************************************************************/
/**
* @internal prvCpssDrvGstatsFlushAll function
* @endinternal
*
* @brief   Flush All RMON counters for all ports.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDrvGstatsFlushAll
(
    IN GT_CPSS_QD_DEV* dev
);

/**
* @internal prvCpssDrvGstatsFlushPort function
* @endinternal
*
* @brief   Flush All RMON counters for a given port.
*
* @param[in] port                     - the logical  number.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGstatsFlushPort
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port
);

/**
* @internal prvCpssDrvGstatsGetPortCounter function
* @endinternal
*
* @brief   This routine gets a specific counter of the given port
*
* @param[in] port                     - the logical  number.
* @param[in] counter                  - the  which will be read
*
* @param[out] statsData                - points to 32bit data storage for the MIB counter
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGstatsGetPortCounter
(
    IN  GT_CPSS_QD_DEV          *dev,
    IN  GT_CPSS_LPORT           port,
    IN  GT_CPSS_STATS_COUNTERS  counter,
    OUT GT_U32                  *statsData
);

/**
* @internal prvCpssDrvGstatsGetPortAllCounters function
* @endinternal
*
* @brief   This routine gets all RMON counters of the given port
*
* @param[in] port                     - the logical  number.
*
* @param[out] statsCounterSet          - points to GT_CPSS_STATS_COUNTER_SET for the MIB counters
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGstatsGetPortAllCounters
(
    IN  GT_CPSS_QD_DEV              *dev,
    IN  GT_CPSS_LPORT               port,
    OUT GT_CPSS_STATS_COUNTER_SET   *statsCounterSet
);
/**
* @internal prvCpssDrvGstatsGetHistogramMode function
* @endinternal
*
* @brief   This routine gets the Histogram Counters Mode.
*
* @param[out] mode                     - Histogram Mode (GT_CPSS_COUNT_RX_ONLY, GT_CPSS_COUNT_TX_ONLY,
*                                      and GT_CPSS_COUNT_RX_TX)
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGstatsGetHistogramMode
(
    IN  GT_CPSS_QD_DEV          *dev,
    OUT GT_CPSS_HISTOGRAM_MODE  *mode
);

/**
* @internal prvCpssDrvGstatsSetHistogramMode function
* @endinternal
*
* @brief   This routine sets the Histogram Counters Mode.
*
* @param[in] mode                     - Histogram Mode (GT_CPSS_COUNT_RX_ONLY, GT_CPSS_COUNT_TX_ONLY,
*                                      and GT_CPSS_COUNT_RX_TX)
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGstatsSetHistogramMode
(
    IN  GT_CPSS_QD_DEV              *dev,
    IN  GT_CPSS_HISTOGRAM_MODE      mode
);

#ifdef __cplusplus
}
#endif

#endif /* __prvCpssDrvGtPortRmon_h */

