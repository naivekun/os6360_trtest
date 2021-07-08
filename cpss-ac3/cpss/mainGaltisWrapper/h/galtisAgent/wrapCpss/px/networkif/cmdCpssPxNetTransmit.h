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
*/
/**
********************************************************************************
* @file cmdCpssPxNetTransmit.h
*
* @brief This file implements Receive/Transmit functionaly user exit functions
* for Galtis.
*
* @version   1
********************************************************************************
*/

#ifndef __cmdCpssPxNetTransmith__
#define __cmdCpssPxNetTransmith__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <cpss/px/networkIf/cpssPxNetIfTypes.h>
#include <cpss/common/networkIf/cpssGenNetIfTypes.h>
#include <galtisAgent/wrapCpss/Gen/networkIf/wrapCpssGenNetIf.h>
/*
 * typedef: structure PX_PKT_DESC_STC
 *
 * Description: structure of Tx parameters , that contain full information on
 *              how to send TX packet from CPU to PP.
 *
 * fields:
 *      devNum          - device number.
 *      valid           - This flag indicates that the entry is valid (or not)
 *      entryID         - Entry ID number for Delete/Get/ChangeAllign operations
 *
 *      sdmaInfo        - the info needed when send packets using the SDMA.
 *
 *      pcktsNum        - Number of packets to send.
 *      gap             - The time is calculated in multiples of 64 clock cycles
 *                        Valid values are 0 (no delay between packets to
 *                        CPU), through 0x3FFE (1,048,544 Clk cycles).
 *                        0x3FFF - has same effect as 0.
 *      waitTime        - The wait time before moving to the next entry.
 *      pcktData        - Array of pointers to packet binary data.
 *      pcktDataLen     - Array of lengths of pcktData pointers.
 *      numOfBuffers    - number of buffers used by pcktData.
 *      numSentPackets  - Number of sent packets.
 *
*/
typedef struct{
    GT_SW_DEV_NUM                   devNum;
    GT_BOOL                         valid;
    GT_U32                          entryId;

    CPSS_PX_NET_TX_PARAMS_STC       sdmaInfo;
 
    GT_U32                          pcktsNum;
    GT_U32                          gap;
    GT_U32                          waitTime;
    GT_U8                         * pcktData[MAX_NUM_OF_SDMA_BUFFERS_PER_CHAIN];
    GT_U32                          pcktDataLen[MAX_NUM_OF_SDMA_BUFFERS_PER_CHAIN];
    GT_U32                          numOfBuffers;
    GT_U32                          numSentPackets;
}PX_PKT_DESC_STC;


/*
 * typedef: structure PX_PKT_GEN_DESC_STC
 *
 * Description: structure of Tx generator parameters, that contain full information on
 *              how to transmit TX packet in scope of traffic generator.
 *
 * fields:
 *      packetGenDescIndex  - index in global descriptor tables
 *      packetId            - parameter of traffic generator API.
 */
typedef struct {
    GT_U32                          packetGenDescIndex;
    GT_U32                          packetId;
}PX_PKT_GEN_DESC_STC;

/*
 * Typedef: struct PX_MODE_SETINGS_STC
 *
 * Description: Packet Description structure
 *
 *      cyclesNum - The maximum number of loop cycles (-1 = forever)
 *      gap       - The time to wit between two cycles.
 *
 */

typedef struct
{
    GT_U32  cyclesNum;
    GT_U32  gap;
} PX_MODE_SETINGS_STC;

/**
* @internal cmdCpssPxTxSetPacketDesc function
* @endinternal
*
* @brief   Creates new transmit parameters description entry in the table.
*
* @param[in] devNum                   - device number
*                                      entryID      - Entry ID of packet descriptor to return.
* @param[in] packetDesc               - Pointer to Packet descriptor new entry in the table
*
* @param[out] newEntryIndex            - The index of the new entry in the table.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note GalTis:
*       Table - cmdTxPcktDescs
*       Toolkit:
*       Using - <netSendPktByLport, netSendPktByIf, netSendPktByVid,
*       netFreeTxGtBuf, gtBufCreate>
*
*/
GT_STATUS cmdCpssPxTxSetPacketDesc
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_U32                      entryId,
    IN  PX_PKT_DESC_STC           * packetDesc,
    OUT GT_U32                    * newEntryIndex
);

/**
* @internal cmdCpssPxTxGetPacketDesc function
* @endinternal
*
* @brief   Returns specific packet descriptor entry from the table.
*
* @param[in,out] entryIndex               - Entry index in packet descriptor table.
*
* @param[out] packetDesc               - Packet descriptor from the table.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_EMPTY                 - on invalid index
*
* @note GalTis:
*       Table - cmdTxPacketDescs
*
*/
GT_STATUS cmdCpssPxTxGetPacketDesc
(
    INOUT GT_U32                  * entryIndex,
    OUT PX_PKT_DESC_STC           * packetDesc
);

/**
* @internal cmdCpssPxTxBeginGetPacketDesc function
* @endinternal
*
* @brief   Indicates for cmdTxGetPacketDesc function that user go to get many
*         records in one session. This API make exact copy of table so that
*         user will get table without changes that can be made during get
*         operation. This mechanism is best suitable for Refresh table
*         operation.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note GalTis:
*       Table - cmdTxPacketDescs
*       Toolkit:
*
*/
GT_STATUS cmdCpssPxTxBeginGetPacketDesc
(
    void
);

/**
* @internal cmdCpssPxTxEndGetPacketDesc function
* @endinternal
*
* @brief   Finish operation began by cmdCpssPxTxBeginGetPacketDesc API function
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note GalTis:
*       Table - cmdTxPacketDescs
*
*/
GT_STATUS cmdCpssPxTxEndGetPacketDesc
(
    void
);

/**
* @internal cmdCpssPxTxClearPacketDesc function
* @endinternal
*
* @brief   Clear all packet descriptors entries from the table.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note GalTis:
*       Table - cmdTxPacketDescs
*
*/
GT_STATUS cmdCpssPxTxClearPacketDesc
(
    void
);

/**
* @internal cmdCpssPxTxDelPacketDesc function
* @endinternal
*
* @brief   Delete packet descriptor entry from the table.
*
* @param[in] devNum                   - device number
* @param[in] entryID                  - Entry ID of packet descriptor to delete.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note After delete operation all record will be shifted
*       so the table will be not fragmented.
*       GalTis:
*       Table - cmdTxPcktDescs
*       Toolkit:
*
*/
GT_STATUS cmdCpssPxTxDelPacketDesc
(
    IN  GT_SW_DEV_NUM       devNum,
    IN  GT_U32              entryID
);

/**
* @internal cmdCpssPxTxStart function
* @endinternal
*
* @brief   Starts transmition of packets
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note GalTis:
*       Command  - cmdCpssPxTxStart
*
*/
GT_STATUS cmdCpssPxTxStart
(
    void
);

/**
* @internal cmdCpssPxTxStop function
* @endinternal
*
* @brief   Stop transmition of packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note GalTis:
*       Command  - cmdCpssPxTxStop
*
*/
GT_STATUS cmdCpssPxTxStop
(
    void
);

/**
* @internal cmdCpssPxTxSetMode function
* @endinternal
*
* @brief   This command will set the the transmit parameters.
*
* @param[in] cyclesNum                - The maximum number of loop cycles (-1 = forever)
* @param[in] gap                      - The time to wit between two cycles
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note GalTis:
*       Command   - cmdTxSetMode
*
*/
GT_STATUS cmdCpssPxTxSetMode
(
    IN GT_U32                       cyclesNum,
    IN GT_U32                       gap
);

/**
* @internal cmdCpssPxTxGetMode function
* @endinternal
*
* @brief   This command will get the the transmit parameters.
*
* @param[out] cyclesNum                - The maximum number of loop cycles (-1 = forever)
* @param[out] gap                      - The time to wit between two cycles.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note GalTis:
*       Command - cmdTxGetMode
*
*/
GT_STATUS cmdCpssPxTxGetMode
(
    OUT GT_U32                    * cyclesNum,
    OUT GT_U32                    * gap
);

/**
* @internal cmdCpssPxConvertPacketDesc function
* @endinternal
*
* @brief   Convert packet desciptor structure to CPSS_DXCH_NET_TX_PARAMS_STC
*
* @param[in] netTxPacketDescPtr       - pointer to TX packet descriptor structure
*
* @param[out] cpssNetTxPacketDescPtr   - pointer to CPSS TX packet descriptor structure
*                                       None
*/
GT_VOID cmdCpssPxConvertPacketDesc
(
    IN PX_PKT_DESC_STC            * netTxPacketDescPtr,
    OUT CPSS_PX_NET_TX_PARAMS_STC * cpssNetTxPacketDescPtr
);

/**
* @internal cmdCpssPxTxGenPacketDescSet function
* @endinternal
*
* @brief   Bind TX generator transmit parameters description entry to DB.
*
* @param[in] packetGenDescPtr         - pointer to TX Packet generator descriptor
*                                      packetGenDescTblEntryPtrPtr - pointer to pointer to TX packet generator descriptor table entry
* @param[in,out] packetGenDescTblSizePtr  - pointer to TX packet generator descriptor tale size
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_CPU_MEM        - on memory allocation fail
* @retval GT_ALREADY_EXIST         - on already exist entry
*/
GT_STATUS cmdCpssPxTxGenPacketDescSet
(
    IN  PX_PKT_GEN_DESC_STC       * packetGenDescPtr,
    INOUT PX_PKT_GEN_DESC_STC    ** packetGenDescTblEntryPtrPtr,
    INOUT GT_U32                  * packetGenDescTblSizePtr,
    OUT GT_U32                    * newEntryIndexPtr
);

/**
* @internal cmdCpssPxTxGenPacketDescDelete function
* @endinternal
*
* @brief   Delete TX generator transmit parameters description entry.
*
* @param[in] devNum                   - Device number.
* @param[in] txQueue                  - Tx queue.
* @param[in] packetId                 - Packet ID to delete.
*                                      packetGenDescTblEntryPtrPtr - pointer to pointer to TX packet generator table
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - on entry not found
* @retval GT_BAD_PTR               - on bad entry pointer
* @retval GT_EMPTY                 - on empty table
*/
GT_STATUS cmdCpssPxTxGenPacketDescDelete
(
    IN GT_SW_DEV_NUM                devNum,
    IN GT_U8                        txQueue,
    IN GT_U32                       packetId,
    INOUT PX_PKT_GEN_DESC_STC    ** packetGenDescTblEntryPtrPtr,
    INOUT GT_U32                  * packetGenDescTblSizePtr
);

/**
* @internal cmdCpssPxTxGenPacketDescClearAll function
* @endinternal
*
* @brief   Clear TX generator transmit parameters DB.
*
* @param[in] devNum                   - Device number.
* @param[in] txQueue                  - Tx queue.
*                                      packetGenDescTblEntryPtrPtr - pointer to pointer to TX packet generator table
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - on entry not found
* @retval GT_EMPTY                 - on empty table
*/
GT_STATUS cmdCpssPxTxGenPacketDescClearAll
(
    IN GT_SW_DEV_NUM                devNum,
    IN GT_U8                        txQueue,
    INOUT PX_PKT_GEN_DESC_STC    ** packetGenDescTblEntryPtrPtr
);

/**
* @internal cmdCpssPxTxGenPacketDescGet function
* @endinternal
*
* @brief   Returns TX generator packet descriptor entry from the table.
*
* @param[in] entryIndex               - table entry index
* @param[in] packetGenDescTblEntryPtr
*                                      - pointer to TX packet generator table
* @param[in] packetGenDescTblSize
*                                      - TX packet generator table size
*
* @param[out] packetGenDescPtr         - pointer to entry in TX packet generator descriptor table.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on bad entry pointer
* @retval GT_OUT_OF_RANGE          - on invalid index
* @retval GT_EMPTY                 - on empty table
*/
GT_STATUS cmdCpssPxTxGenPacketDescGet
(
    IN  GT_U32                      entryIndex,
    IN  PX_PKT_GEN_DESC_STC       * packetGenDescTblEntryPtr,
    IN  GT_U32                      packetGenDescTblSize,
    OUT PX_PKT_GEN_DESC_STC       * packetGenDescPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*__cmdCpssPxNetTransmith__*/



