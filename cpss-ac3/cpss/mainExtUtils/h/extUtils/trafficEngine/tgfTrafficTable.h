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
* @file tgfTrafficTable.h
*
* @brief Manage the Rx Tx tables of the traffic sent from CPU and traffic
* received at the CPU
*
* @version   8.
********************************************************************************
*/
#ifndef __tgfTrafficTableh
#define __tgfTrafficTableh

#include <extUtils/rxEventHandler/rxEventHandler.h>
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/******************************************************************************\
 *                              Define section                                *
\******************************************************************************/

/* Rx buffer size */
#define TGF_RX_BUFFER_MAX_SIZE_CNS   0x3FFF

/* Tx buffer size */
#define TGF_TX_BUFFER_MAX_SIZE_CNS   0x3FFF

/* Tx buffer min size */
#define TGF_TX_BUFFER_MIN_SIZE_CNS   0x3C

/* default sleep time in single loop step to get captured packet */
#define TGF_DEFAULT_CAPTURE_SLEEP_TIME_CNS 10

/* number of time loops to get captured packet */
extern GT_U32   prvTgfCaptureTimeLoop;

/******************************************************************************\
 *                              Common types section                          *
\******************************************************************************/

/*******************************************************************************
* TGF_FREE_RX_INFO_FUNC
*
* DESCRIPTION:
*       Prototype of callback function to free the specific rx info format.
*
* INPUTS:
*       specificDeviceFormatPtr - (pointer to) the specific device Rx info format.
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
typedef GT_STATUS (*TGF_FREE_RX_INFO_FUNC)
(
    IN GT_VOID*  specificDeviceFormatPtr
);

/*******************************************************************************
* TGF_COPY_RX_INFO_FUNC
*
* DESCRIPTION:
*       Prototype of callback function to COPY the specific rx info format.
*
* INPUTS:
*       srcSpecificDeviceFormatPtr - (pointer to) the SRC specific device Rx
*                                    info format.
*
* OUTPUTS:
*       dstSpecificDeviceFormatPtr - (pointer to) the DST specific device Rx
*                                    info format.
*
* RETURNS:
*       None
*
* COMMENTS:
*
*******************************************************************************/
typedef GT_STATUS (*TGF_COPY_RX_INFO_FUNC)
(
    IN  const GT_VOID*  srcSpecificDeviceFormatPtr,
    OUT       GT_VOID*  dstSpecificDeviceFormatPtr
);

/**
* @enum TGF_RX_MODE_ENT
 *
 * @brief The table entry adding mode for the received packets
*/
typedef enum{

    /** for cyclic collection. */
    TGF_RX_CYCLIC_E,

    /** for collecting until buffer is full */
    TGF_RX_ONCE_E

} TGF_RX_MODE_ENT;

/**
* @struct TGF_RX_ENTRY
 *
 * @brief Entry of the Rx packet table
*/
typedef struct{

    /** is entry empty flag */
    GT_BOOL isEmpty;

    /** The device number in which the packet was received. */
    GT_U8 devNum;

    /** The Rx queue in which the packet was received. */
    GT_U8 queue;

    /** (pointer to) the specific device Rx info format. */
    GT_VOID* specificDeviceFormatPtr;

    /** callback function to free the specific rx info format, */
    TGF_FREE_RX_INFO_FUNC freeRxInfoFunc;

    /** callback function to copy the specific rx info format, */
    TGF_COPY_RX_INFO_FUNC copyRxInfoFunc;

    /** pointer to the packet buffer (allocated by this C file) */
    GT_U8* packetBufferPtr;

    /** @brief length of original packet (packet may have been too
     *  long , so we didn't keep all of it only 1536 bytes)
     */
    GT_U32 packetOriginalLen;

    /** length of the buffer allocated in packetBufferPtr */
    GT_U32 bufferLen;

} TGF_RX_ENTRY;

/**
* @enum TGF_RE_RUN_STATUS_ENT
 *
 * @brief test re-run variable status
*/
typedef enum{

    /** no re-run possible */
    TGF_RE_RUN_STATUS_DISABLED_E,

    /** re-run is possible */
    TGF_RE_RUN_STATUS_CAPABLE_E,

    /** do re-run if test failed and used events or capture */
    TGF_RE_RUN_STATUS_DO_RE_RUN_E

} TGF_RE_RUN_STATUS_ENT;

/* flag to state the test uses capture or events */
extern TGF_RE_RUN_STATUS_ENT prvTgfReRunVariable;

/******************************************************************************\
 *                            Public API section                              *
\******************************************************************************/
/**
* @internal prvTgfTrafficTableRxStartCapture function
* @endinternal
*
* @brief   Start/Stop collecting the received packets without delay.
*
* @param[in] enable                   - GT_TRUE - start capture
*                                      GT_FALSE - stop capture
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvTgfTrafficTableRxStartCapture
(
    IN GT_BOOL          enable
);

/**
* @internal tgfTrafficTableRxStartCapture function
* @endinternal
*
* @brief   Start/Stop collecting the received packets
*
* @param[in] enable                   - GT_TRUE - start capture
*                                      GT_FALSE - stop capture
*
* @retval GT_OK                    - on success
*/
GT_STATUS tgfTrafficTableRxStartCapture
(
    IN GT_BOOL          enable
);

/**
* @internal tgfTrafficTableRxModeSet function
* @endinternal
*
* @brief   Set received packets collection mode and parameters
*
* @param[in] rxMode                   - collection mode
* @param[in] buferSize                - buffer size
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_CPU_MEM        - memory allocation Failure
*/
GT_STATUS tgfTrafficTableRxModeSet
(
    IN TGF_RX_MODE_ENT  rxMode,
    IN GT_U32           buferSize
);

/**
* @internal tgfTrafficTableRxPcktReceiveCb function
* @endinternal
*
* @brief   Function called to handle incoming Rx packet in the CPU
*
* @param[in] devNum                   - Device number.
* @param[in] queueIdx                 - The queue from which this packet was received.
* @param[in] numOfBuff                - Num of used buffs in packetBuffs.
* @param[in] packetBuffs[]            - The received packet buffers list.
* @param[in] buffLen[]                - List of buffer lengths for packetBuffs.
* @param[in] rxParamsPtr              - (pointer to)information parameters of received packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS tgfTrafficTableRxPcktReceiveCb
(
    IN GT_UINTPTR                cookie,
    IN RX_EV_HANDLER_EV_TYPE_ENT evType,
    IN GT_U8            devNum,
    IN GT_U8            queueIdx,
    IN GT_U32           numOfBuff,
    IN GT_U8            *packetBuffs[],
    IN GT_U32           buffLen[],
    IN GT_VOID          *rxParamsPtr
);

/**
* @internal tgfTrafficTableRxPcktTblClear function
* @endinternal
*
* @brief   Clear table
*
* @retval GT_OK                    - on success
*/
GT_STATUS tgfTrafficTableRxPcktTblClear
(
    GT_VOID
);

/**
* @internal tgfTrafficTableRxPcktGet function
* @endinternal
*
* @brief   Get entry from rxNetworkIf table
*
* @param[in,out] currentIndex             - the current entry's index
* @param[in,out] packetBufLenPtr          - the length of the user space for the packet
*
* @param[out] packetBufPtr             - packet's buffer (pre allocated by the user)
* @param[in,out] packetBufLenPtr          - the length of the copied packet to gtBuf
* @param[out] packetLenPtr             - the Rx packet original length
* @param[out] devNumPtr                - packet's device number
* @param[out] queuePtr                 - The Rx queue in which the packet was received.
* @param[out] specificDeviceFormatPtr  - (pointer to) the specific device Rx info format.
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - index is out of range
* @retval GT_NOT_FOUND             - entry in index is entry not valid
*/
GT_STATUS tgfTrafficTableRxPcktGet
(
    INOUT GT_U32        currentIndex,
    OUT   GT_U8         *packetBufPtr,
    INOUT GT_U32        *packetBufLenPtr,
    OUT   GT_U32        *packetLenPtr,
    OUT   GT_U8         *devNumPtr,
    OUT   GT_U8         *queuePtr,
    OUT   GT_VOID       *specificDeviceFormatPtr
);

/**
* @internal tgfTrafficTableRxPcktIsCaptureReady function
* @endinternal
*
* @brief   Check if the engine is ready to capture rx packets
*
* @retval GT_TRUE                  - the engine is ready to capture rx packets
* @retval GT_FALSE                 - the engine is NOT ready to capture rx packets
*/
GT_BOOL tgfTrafficTableRxPcktIsCaptureReady
(
    GT_VOID
);

/**
* @internal tgfTrafficTableNetRxPacketCbRegister function
* @endinternal
*
* @brief   Register a CB function to be called on every RX packet to CPU
*
* @param[in] rxPktReceiveCbFun        - CB function to be called on every RX packet to CPU
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_STATE             - om illegal state
*
* @note Function is implemented in the AppDemo application
*
*/
GT_STATUS tgfTrafficTableNetRxPacketCbRegister
(
    IN  RX_EV_PACKET_RECEIVE_CB_FUN  rxPktReceiveCbFun
);

/**
* @internal tgfTrafficTableRxDoCountOnlySet function
* @endinternal
*
* @brief   Function sets Counting Only for incoming Rx packet in the CPU
*
* @param[in] doCountOnly              - GT_TRUE - only count packets
*                                      GT_FALSE - make regular capture treatment.
*                                       previouse state of Do Count Only
*/
GT_BOOL tgfTrafficTableRxDoCountOnlySet
(
    IN GT_BOOL         doCountOnly
);

/**
* @internal tgfTrafficTableRxToTxSet function
* @endinternal
*
* @brief   Function sets Rx To Tx mode of Rx Packet callback
*
* @param[in] doRxToTx                 - GT_TRUE - each Rx packet goes to Tx from CPU
*                                      GT_FALSE - make regular capture treatment.
*                                       previouse state of Rx To Tx mode
*/
GT_BOOL tgfTrafficTableRxToTxSet
(
    IN GT_BOOL         doRxToTx
);

/**
* @internal tgfTrafficTableRxCountGet function
* @endinternal
*
* @brief   Function returns counter of RX packets
*/
GT_U32 tgfTrafficTableRxCountGet
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __tgfTrafficTableh */


