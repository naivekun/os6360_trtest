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
* @file cmdCpssDxChNetTransmit.h
*
* @brief This file implements Receive/Transmit functionaly user exit functions
* for Galtis.
*
* @version   24
********************************************************************************
*/

#ifndef __cmdCpssDxChNetTransmith__
#define __cmdCpssDxChNetTransmith__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <cpss/common/networkIf/cpssGenNetIfTypes.h>
#include <cpss/generic/cos/cpssCosTypes.h>
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfTypes.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgVlan.h>
#include <galtisAgent/wrapCpss/Gen/networkIf/wrapCpssGenNetIf.h>

/**
* @enum DXCH_TX_CMD_ENT
 *
 * @brief Enumeration of Transmit command Types
*/
typedef enum{

    DXCH_TX_BY_VIDX = 0,

    DXCH_TX_BY_PORT

} DXCH_TX_CMD_ENT;


/*
 * typedef: structure DXCH_PKT_DESC_STC
 *
 * Description: structure of Tx parameters , that contain full information on
 *              how to send TX packet from CPU to PP.
 *
 * fields:
 *      cmdType         - Defines type of transmition (VLAN, Port).
 *      valid           - This flag indicates that the entry is valid (or not)
 *      entryID         - Entry ID number for Delete/Get/ChangeAllign operations
 *      txSyncMode      - should the packet be send in a sync mode or not
 *      packetIsTagged - the packet (buffers) contain the vlan TAG inside.
 *                  GT_TRUE  - the packet contain the vlan TAG inside.
 *                  GT_FALSE - the packet NOT contain the vlan TAG inside.
 *
 *      cookie  - The user's data to be returned to the Application when in
 *                the Application's buffers can be re-used .
 *
 *      sdmaInfo - the info needed when send packets using the SDMA.
 *
 *      dsaParam - the DSA parameters
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
    DXCH_TX_CMD_ENT                 cmdType;
    GT_BOOL                         valid;
    GT_U32                          entryId;
    GT_BOOL                         txSyncMode;

    GT_BOOL                         packetIsTagged;
    /*GT_PTR                        cookie;*/
    CPSS_DXCH_NET_SDMA_TX_PARAMS_STC    sdmaInfo;
    CPSS_DXCH_NET_DSA_PARAMS_STC        dsaParam;

    GT_U32                          pcktsNum;
    GT_U32                          gap;
    GT_U32                          waitTime;
    GT_U8                           *pcktData[MAX_NUM_OF_SDMA_BUFFERS_PER_CHAIN];
    GT_U32                          pcktDataLen[MAX_NUM_OF_SDMA_BUFFERS_PER_CHAIN];
    GT_U32                          numOfBuffers;
    GT_U32                          numSentPackets;

}DXCH_PKT_DESC_STC; /*CPSS_NET_TX_PARAMS_STC;*/

/**
* @enum DXCH_TX_GEN_ACTION_TYPE_ENT
 *
 * @brief Enumeration of traffic generator entry action
*/
typedef enum{

    /** execute add procedure, save packet id in the DB. */
    DXCH_TX_GEN_ACTION_ADD_E,

    /** execute update procedure, use packet ID to call “update” API */
    DXCH_TX_GEN_ACTION_UPDATE_E

} DXCH_TX_GEN_ACTION_TYPE_ENT;

/*
 * typedef: structure DXCH_PKT_GEN_DESC_STC
 *
 * Description: structure of Tx generator parameters, that contain full information on
 *              how to transmit TX packet in scope of traffic generator.
 *
 * fields:
 *      actionType          - add/update entry
 *      dsaType             - the type of DSA tag (FORWARD/FROM_CPU)
 *      packetGenDescIndex  - index in global descriptor tables
 *      packetId            - parameter of traffic generator API.
 */
typedef struct {
    DXCH_TX_GEN_ACTION_TYPE_ENT     actionType;
    CPSS_DXCH_NET_DSA_CMD_ENT       dsaType;
    GT_U32                          packetGenDescIndex;
    GT_U32                          packetId;
}DXCH_PKT_GEN_DESC_STC;

/**
* @enum DXCH_MAC_TYPE
 *
 * @brief Defines the different Mac-Da types of a transmitted packet.
*/
typedef enum{

    /** MAC_DA[0] = 1'b0 */
    UNICAST_MAC,

    /** MAC_DA[0] = 1'b1 */
    MULTICAST_MAC,

    /** MAC_DA = 0xFFFFFFFF */
    BROADCAST_MAC

} DXCH_MAC_TYPE;

/**
* @internal cmdCpssDxChTxSetPacketDesc function
* @endinternal
*
* @brief   Creates new transmit parameters description entry in the table.
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
GT_STATUS cmdCpssDxChTxSetPacketDesc
(
    IN  GT_U32                  entryId,
    IN  DXCH_PKT_DESC_STC       *packetDesc,
    IN  GT_BOOL                 extended,
    OUT GT_U32                  *newEntryIndex
);

/**
* @internal cmdCpssDxChTxGetPacketDesc function
* @endinternal
*
* @brief   Returns specific packet descriptor entry from the table.
*
* @param[in,out] entryIndex               - Entry index in packet descriptor table.
* @param[in] dsaCmd                   - DSA tag commands
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
GT_STATUS cmdCpssDxChTxGetPacketDesc
(
    INOUT GT_U32            *entryIndex,
    IN CPSS_DXCH_NET_DSA_CMD_ENT  dsaCmd,
    OUT DXCH_PKT_DESC_STC   *packetDesc
);

/**
* @internal cmdCpssDxChTxBeginGetPacketDesc function
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
GT_STATUS cmdCpssDxChTxBeginGetPacketDesc
(
void
);

/**
* @internal cmdCpssDxChTxEndGetPacketDesc function
* @endinternal
*
* @brief   Finish operation began by cmdCpssDxChTxBeginGetPacketDesc API function
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note GalTis:
*       Table - cmdTxPacketDescs
*
*/
GT_STATUS cmdCpssDxChTxEndGetPacketDesc
(
    void
);

/**
* @internal cmdCpssDxChTxClearPacketDesc function
* @endinternal
*
* @brief   Clear all packet descriptors entries from the table.
*
* @param[in] dsaCmd                   - DSA tag commands
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note GalTis:
*       Table - cmdTxPacketDescs
*
*/
GT_STATUS cmdCpssDxChTxClearPacketDesc
(
    IN CPSS_DXCH_NET_DSA_CMD_ENT  dsaCmd
);

/**
* @internal cmdCpssDxChTxDelPacketDesc function
* @endinternal
*
* @brief   Delete packet descriptor entry from the table.
*
* @param[in] entryID                  - Entry ID of packet descriptor to delete.
* @param[in] dsaCmd                   - DSA tag commands
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
GT_STATUS cmdCpssDxChTxDelPacketDesc
(
    IN  GT_U32      entryID,
    IN CPSS_DXCH_NET_DSA_CMD_ENT  dsaCmd
);

/**
* @internal cmdCpssDxChTxStart function
* @endinternal
*
* @brief   Starts transmition of packets
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note GalTis:
*       Command  - cmdCpssDxChTxStart
*       Toolkit:
*       Interface - <prestera/tapi/networkif/commands.api>
*
*/
GT_STATUS cmdCpssDxChTxStart
(
void
);

/**
* @internal cmdCpssDxChTxStop function
* @endinternal
*
* @brief   Stop transmition of packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note GalTis:
*       Command  - cmdCpssDxChTxStop
*       Toolkit:
*       Interface - <prestera/tapi/networkif/commands.api>
*       Using   - <netSendPktByLport, netSendPktByIf, netSendPktByVid,
*       netFreeTxGtBuf, gtBufCreate>
*
*/
GT_STATUS cmdCpssDxChTxStop
(
void
);

/**
* @internal cmdCpssDxChTxSetMode function
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
*       Toolkit:
*       Interface  - <prestera/tapi/networkif/commands.api>
*
*/
GT_STATUS cmdCpssDxChTxSetMode
(
    IN GT_U32   cyclesNum,
    IN GT_U32   gap
);

/**
* @internal cmdCpssDxChTxGetMode function
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
*       Toolkit:
*       Interface - <prestera/tapi/networkif/commands.api>
*
*/
GT_STATUS cmdCpssDxChTxGetMode
(
    OUT GT_U32 *cyclesNum,
    OUT GT_U32 *gap
);

/**
* @internal cmdCpssDxChConvertPacketDesc function
* @endinternal
*
* @brief   Convert packet desciptor structure to CPSS_DXCH_NET_TX_PARAMS_STC
*
* @param[in] netTxPacketDescPtr       - pointer to TX packet descriptor structure
*
* @param[out] cpssNetTxPacketDescPtr   - pointer to CPSS TX packet descriptor structure
*                                       None
*/
GT_VOID cmdCpssDxChConvertPacketDesc
(
    IN DXCH_PKT_DESC_STC                * netTxPacketDescPtr,
    OUT CPSS_DXCH_NET_TX_PARAMS_STC     * cpssNetTxPacketDescPtr
);

/**
* @internal cmdCpssDxChTxGenPacketDescSet function
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
GT_STATUS cmdCpssDxChTxGenPacketDescSet
(
    IN  DXCH_PKT_GEN_DESC_STC       * packetGenDescPtr,
    INOUT DXCH_PKT_GEN_DESC_STC     ** packetGenDescTblEntryPtrPtr,
    INOUT GT_U32                    * packetGenDescTblSizePtr,
    OUT GT_U32                      * newEntryIndexPtr
);

/**
* @internal cmdCpssDxChTxGenPacketDescDelete function
* @endinternal
*
* @brief   Delete TX generator transmit parameters description entry.
*
* @param[in] devNum                   - Device number.
* @param[in] txQueue                  - Tx queue.
* @param[in] portGroup                - Port group ID.
* @param[in] packetId                 - Packet ID to delete.
*                                      packetGenDescTblEntryPtrPtr - pointer to pointer to TX packet generator table
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - on entry not found
* @retval GT_BAD_PTR               - on bad entry pointer
* @retval GT_EMPTY                 - on empty table
*/
GT_STATUS cmdCpssDxChTxGenPacketDescDelete
(
    IN GT_U8                            devNum,
    IN GT_U8                            txQueue,
    IN GT_U32                           portGroup,
    IN GT_U32                           packetId,
    INOUT DXCH_PKT_GEN_DESC_STC      ** packetGenDescTblEntryPtrPtr,
    INOUT GT_U32                      * packetGenDescTblSizePtr
);

/**
* @internal cmdCpssDxChTxGenPacketDescClearAll function
* @endinternal
*
* @brief   Clear TX generator transmit parameters DB.
*
* @param[in] devNum                   - Device number.
* @param[in] txQueue                  - Tx queue.
* @param[in] portGroup                - Port group ID.
*                                      packetGenDescTblEntryPtrPtr - pointer to pointer to TX packet generator table
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - on entry not found
* @retval GT_EMPTY                 - on empty table
*/
GT_STATUS cmdCpssDxChTxGenPacketDescClearAll
(
    IN GT_U8                            devNum,
    IN GT_U8                            txQueue,
    IN GT_U32                           portGroup,
    INOUT DXCH_PKT_GEN_DESC_STC      ** packetGenDescTblEntryPtrPtr
);

/**
* @internal cmdCpssDxChTxGenPacketDescGet function
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
GT_STATUS cmdCpssDxChTxGenPacketDescGet
(
    IN  GT_U32                      entryIndex,
    IN  DXCH_PKT_GEN_DESC_STC       * packetGenDescTblEntryPtr,
    IN  GT_U32                      packetGenDescTblSize,
    OUT DXCH_PKT_GEN_DESC_STC       * packetGenDescPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*__cmdCpssDxChNetTransmith__*/



