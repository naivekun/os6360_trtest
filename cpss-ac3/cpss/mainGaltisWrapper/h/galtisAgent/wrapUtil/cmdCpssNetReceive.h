/*******************************************************************************
*                Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
* cmdNetReceive.h
*
* DESCRIPTION:
*       This file implements Packet Capture functionally user exit functions
*       for Galtis.
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 3 $
*
******************************************************************************/

#ifndef __cmdNetReceive_h__
#define __cmdNetReceive_h__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define GALTIS_RX_BUFFER_MAX_SIZE_CNS   0x600   /*1536*/


/*******************************************************************************
* FREE_RX_INFO_FUNC
*
* DESCRIPTION:
*       prototype of callback function to free the specific rx info format.
*
* INPUTS:
*      specificDeviceFormatPtr - (pointer to) the specific device Rx info format.
*                             format of DXCH / EXMX /DXSAL ...
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*
*
*******************************************************************************/
typedef void (*FREE_RX_INFO_FUNC)
(
    IN void*  specificDeviceFormatPtr
);

/*******************************************************************************
* COPY_RX_INFO_FUNC
*
* DESCRIPTION:
*       prototype of callback function to COPY the specific rx info format.
*
* INPUTS:
*      srcSpecificDeviceFormatPtr - (pointer to) the SOURCE specific device Rx
*                                   info format.
*                                   format of DXCH / EXMX /DXSAL ...
*
* OUTPUTS:
*      dstSpecificDeviceFormatPtr - (pointer to) the DESTINATION specific device Rx
*                                   info format.
*                                   format of DXCH / EXMX /DXSAL ...
*
*
* RETURNS:
*       None
*
* COMMENTS:
*
*
*******************************************************************************/
typedef void (*COPY_RX_INFO_FUNC)
(
    IN  const void*  srcSpecificDeviceFormatPtr,
    OUT void*        dstSpecificDeviceFormatPtr
);

/**
* @enum GALTIS_RX_MODE_ENT
 *
 * @brief The table entry adding mode for the received packets
*/
typedef enum{

    /** for cyclic collection. */
    GALTIS_RX_CYCLIC_E,

    /** for collecting until buffer is full */
    GALTIS_RX_ONCE_E

} GALTIS_RX_MODE_ENT;

/**
* @internal cmdCpssRxStartCapture function
* @endinternal
*
* @brief   Start collecting the received packets
*
* @retval GT_OK                    - on success
*
* @note GalTis:
*       Command - cmdCpssRxStartCapture
*
*/
GT_STATUS cmdCpssRxStartCapture(void);

/**
* @internal cmdCpssRxStopCapture function
* @endinternal
*
* @brief   Stop collecting the received packets
*
* @retval GT_OK                    - on success
*
* @note GalTis:
*       Command - cmdCpssRxStopCapture
*
*/
GT_STATUS cmdCpssRxStopCapture(void);

/**
* @internal cmdCpssRxSetMode function
* @endinternal
*
* @brief   set received packets collection mode and parameters
*
* @param[in] rxMode                   - collection mode
* @param[in] buferSize                - buffer size
* @param[in] numOfEnteries            - number of entries
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_CPU_MEM        - memory allocation Failure
*
* @note GalTis:
*       Command - cmdCpssRxSetMode
*
*/
GT_STATUS cmdCpssRxSetMode
(
    IN GALTIS_RX_MODE_ENT   rxMode,
    IN GT_U32       buferSize,
    IN GT_U32       numOfEnteries
);


/**
* @internal cmdCpssRxGetMode function
* @endinternal
*
* @brief   Get received packets collection mode and parameters
*
* @param[out] modePtr                  - GALTIS_RX_CYCLIC_E or GALTIS_RX_ONCE_E
* @param[out] buffSizePtr              - packet buffer size
* @param[out] numEntriesPtr            - number of entries
*
* @retval GT_OK                    - on success
*
* @note GalTis:
*       Command - cmdCpssRxGetMode
*
*/
GT_STATUS cmdCpssRxGetMode
(
    OUT GALTIS_RX_MODE_ENT  *modePtr,
    OUT GT_U32      *buffSizePtr,
    OUT GT_U32      *numEntriesPtr
);

/**
* @internal cmdCpssRxPktReceive function
* @endinternal
*
* @brief   Receive packet callback function . the caller wants to register its
*         packet info with the manager array.
*         This function give services to all the rx packets info formats.
* @param[in] devNum                   - The device number in which the packet was received.
* @param[in] queue                    - The Rx  in which the packet was received.
* @param[in] specificDeviceFormatPtr  - (pointer to) the specific device Rx info format.
*                                      format of DXCH / EXMX /DXSAL ...
*                                      NOTE : this pointer is allocated by the specific
*                                      device "C" file.
* @param[in] freeRxInfoFunc           - callback function to free the specific rx info format,
* @param[in] copyRxInfoFunc           - callback function to copy the specific rx info format,
* @param[in] numOfBuff                - Num of used buffs in packetBuffs
* @param[in] packetBuffs[]            - The received packet buffers list
* @param[in] buffLen[]                - List of buffer lengths for packetBuffs
*
* @retval GT_OK                    - on success
* @retval GT_FULL                  - when buffer is full
* @retval GT_BAD_STATE             - the galtis-cmd not ready/set to "save" the packet with info
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_OUT_OF_CPU_MEM        - on CPU memory allocation failure
*
* @note GalTis:
*       None
*
*/
GT_STATUS cmdCpssRxPktReceive
(
    IN GT_U8           devNum,
    IN GT_U8           queue,
    IN void*           specificDeviceFormatPtr,
    IN FREE_RX_INFO_FUNC freeRxInfoFunc,
    IN COPY_RX_INFO_FUNC copyRxInfoFunc,
    IN GT_U32          numOfBuff,
    IN GT_U8           *packetBuffs[],
    IN GT_U32          buffLen[]
);

/**
* @internal cmdCpssRxPktClearTbl function
* @endinternal
*
* @brief   clear cmdCpssRxPktClearTbl table
*
* @retval GT_OK                    - on success
*
* @note GalTis:
*       Table - rxNetIf
*
*/
GT_STATUS cmdCpssRxPktClearTbl(void);

/**
* @internal cmdCpssRxPkGetFirst function
* @endinternal
*
* @brief   Get Next entry from rxNetworkIf table
*
* @param[in,out] packetBufLenPtr          - the length of the user space for the packet
*
* @param[out] currentIndexPtr          - get current entry's index
* @param[out] packetBufPtr             - packet's buffer (pre allocated by the user)
* @param[in,out] packetBufLenPtr          - the length of the copied packet to gtBuf
* @param[out] packetLenPtr             - the Rx packet original length
* @param[out] devNumPtr                - packet's device number
* @param[out] queuePtr                 - The Rx queue in which the packet was received.
* @param[out] specificDeviceFormatPtr  - (pointer to) the specific device Rx info format.
*                                      format of DXCH / EXMX /DXSAL ...
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - when no more entries
*
* @note GalTis:
*       Table - rxNetworkIf
*
*/
GT_STATUS cmdCpssRxPkGetFirst
(
    OUT GT_U32          *currentIndexPtr,
    OUT GT_U8           *packetBufPtr,
    INOUT GT_U32        *packetBufLenPtr,
    OUT GT_U32          *packetLenPtr,
    OUT GT_U8           *devNumPtr,
    OUT GT_U8           *queuePtr,
    OUT void*           specificDeviceFormatPtr

);

/**
* @internal cmdCpssRxPkGetNext function
* @endinternal
*
* @brief   Get Next entry from rxNetworkIf table
*
* @param[in,out] packetBufLenPtr          - the length of the user space for the packet
* @param[in,out] currentIndexPtr          - the PREVIOUS entry's index
* @param[in,out] currentIndexPtr          - get current entry's index
*
* @param[out] packetBufPtr             - packet's buffer (pre allocated by the user)
* @param[in,out] packetBufLenPtr          - the length of the copied packet to gtBuf
* @param[out] packetLenPtr             - the Rx packet original length
* @param[out] devNumPtr                - packet's device number
* @param[out] queuePtr                 - The Rx queue in which the packet was received.
* @param[out] specificDeviceFormatPtr  - (pointer to) the specific device Rx info format.
*                                      format of DXCH / EXMX /DXSAL ...
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - when no more entries
*
* @note GalTis:
*       Table - rxNetworkIf
*
*/
GT_STATUS cmdCpssRxPkGetNext
(
    INOUT GT_U32        *currentIndexPtr,
    OUT GT_U8           *packetBufPtr,
    INOUT GT_U32        *packetBufLenPtr,
    OUT GT_U32          *packetLenPtr,
    OUT GT_U8           *devNumPtr,
    OUT GT_U8           *queuePtr,
    OUT void*           specificDeviceFormatPtr

);


/**
* @internal cmdCpssRxPktIsCaptureReady function
* @endinternal
*
* @brief   check if the galtis-cmd is ready to capture rx packets
*
* @retval GT_TRUE                  - the galtis-cmd is ready to capure rx packets
* @retval GT_FALSE                 - the galtis-cmd is NOT ready to capure rx packets
*/
GT_BOOL cmdCpssRxPktIsCaptureReady(void);

/**
* @enum GT_RX_ENTRY_TYPE_ENT
 *
 * @brief The table entry type of packet
*/
typedef enum{

    /** generic packet */
    GT_RX_ENTRY_TYPE_GENERIC_E,

    /** @brief packet trapped to the CPU by the ENH-UT engine
     *  and is considered internal packet
     */
    GT_RX_ENTRY_TYPE_CAPTURED_BY_ENH_UT_E,

    GT_RX_ENTRY_TYPE___LAST___E

} GT_RX_ENTRY_TYPE_ENT;

/***** Capture packet table entry  *******************************/

/**
* @struct CMD_RX_ENTRY
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

    /** @brief (pointer to) the specific device Rx info format.
     *  format of DXCH / EXMX /DXSAL ...
     *  NOTE : this pointer is allocated by the specific
     *  device "C" file.
     */
    void* specificDeviceFormatPtr;

    /** callback function to free the specific rx info format, */
    FREE_RX_INFO_FUNC freeRxInfoFunc;

    /** callback function to copy the specific rx info format, */
    COPY_RX_INFO_FUNC copyRxInfoFunc;

    /** pointer to the packet buffer (allocated by this C file) */
    GT_U8* packetBufferPtr;

    /** @brief length of original packet (packet may have been too
     *  long , so we didn't keep all of it only 1536 bytes)
     */
    GT_U32 packetOriginalLen;

    /** length of the buffer allocated in packetBufferPtr */
    GT_U32 bufferLen;

    /** indication about the type of the entry */
    GT_RX_ENTRY_TYPE_ENT entryType;

} CMD_RX_ENTRY;




#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cmdNetReceive_h__ */




