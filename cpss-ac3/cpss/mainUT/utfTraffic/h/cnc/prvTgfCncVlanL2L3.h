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
* @file prvTgfCncVlanL2L3.h
*
* @brief Centralized Counters (Cnc)
*
* @version   3
********************************************************************************
*/
#ifndef __prvTgfCncVlanL2L3
#define __prvTgfCncVlanL2L3

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define PRV_TGF_CNC_TEST_TAGGED_PACKET_CNS 0

#if PRV_TGF_CNC_TEST_TAGGED_PACKET_CNS
#define PRV_TGF_CNC_TEST_VLAN_TAG_SIZE_CNS TGF_VLAN_TAG_SIZE_CNS
#else
#define PRV_TGF_CNC_TEST_VLAN_TAG_SIZE_CNS 0
#endif /*PRV_TGF_CNC_TEST_TAGGED_PACKET_CNS*/

#define PRV_TGF_CNC_BLOCKS_NUM_MAC()        prvTgfCncFineTuningBlockAmountGet()

#define PRV_TGF_CNC_BLOCK_ENTRIES_NUM_MAC() prvTgfCncFineTuningBlockSizeGet()

/* index of port in port-array for send port */
#define PRV_TGF_CNC_SEND_PORT_INDEX_CNS        1

/* index of port in port-array for receive (egress) port */
#define PRV_TGF_CNC_RECEIVE_PORT_INDEX_CNS     2

/* VID used in test */
#define PRV_TGF_CNC_TEST_VID0 5

/* User Priorityes used in test */
#define PRV_TGF_CNC_TEST_UP0 0

/* CFI used in test */
#define PRV_TGF_CNC_TEST_CFI0 0

/* used Qos profile */
#define PRV_TGF_CNC_TEST_QOS_PROFILE0 0

/* tested block index */
#define PRV_TGF_CNC_TESTED_BLOCK_NUM_MAC() (PRV_TGF_CNC_BLOCKS_NUM_MAC() - 1)

/**
* @internal prvTgfCncTestVidDefConfigurationSet function
* @endinternal
*
* @brief   None
*/
GT_STATUS prvTgfCncTestVidDefConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfCncTestCncBlockConfigure function
* @endinternal
*
* @brief   This function sets configuration of Cnc Block
*
* @param[in] blockNum                 - CNC block number
* @param[in] client                   - CNC client
*                                      valid range see in datasheet of specific device.
* @param[in] enable                   - the client  to update the block
*                                      GT_TRUE - enable, GT_FALSE - disable
*                                      It is forbidden to enable update the same
*                                      block by more then one client. When an
*                                      application enables some client it is responsible
*                                      to disable all other clients it enabled before
* @param[in] indexRangesBmp[]         - the counter index ranges bitmap
*                                      DxCh3 and DxChXcat devices have 8 ranges of counter indexes,
*                                      each 2048 indexes (0..(2K-1), 2K..(4K-1), ..., 14K..(16K-1))
*                                      the n-th bit 1 value maps the n-th index range
*                                      to the block (2048 counters)
*                                      The Lion devices has 64 ranges (512 indexes each).
*                                      Lion2 devices have 32 ranges (512 indexes each).
*                                      Allowed to map more then one range to the block
*                                      but it will cause updating the same counters via
*                                      different indexes.
* @param[in] format                   - CNC counter HW format,
*                                      relevant only for Lion and above
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       - trunk library was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - some values are out of range
* @retval GT_BAD_PARAM             - on illegal parameters
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_ALREADY_EXIST         - one of the members already exists in another trunk
*/
GT_STATUS prvTgfCncTestCncBlockConfigure
(
    IN GT_U32                            blockNum,
    IN PRV_TGF_CNC_CLIENT_ENT            client,
    IN GT_BOOL                           enable,
    IN GT_U32                            indexRangesBmp[],
    IN PRV_TGF_CNC_COUNTER_FORMAT_ENT    format
);

/**
* @internal prvTgfCncTestCommonReset function
* @endinternal
*
* @brief   This function resets configuration of Cnc
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       - trunk library was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - some values are out of range
* @retval GT_BAD_PARAM             - on illegal parameters
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_ALREADY_EXIST         - one of the members already exists in another trunk
*/
GT_STATUS prvTgfCncTestCommonReset
(
    GT_VOID
);

/**
* @internal prvTgfCncTestNotZeroCountersDump function
* @endinternal
*
* @brief   This function Dumps all non zero counters
*/
GT_STATUS prvTgfCncTestNotZeroCountersDump
(
    GT_VOID
);

/**
* @internal prvTgfCncTestVlanL2L3Configure function
* @endinternal
*
* @brief   This function configures Cnc VlanL2L3 tests
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       - trunk library was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - some values are out of range
* @retval GT_BAD_PARAM             - on illegal parameters
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_ALREADY_EXIST         - one of the members already exists in another trunk
*/
GT_STATUS prvTgfCncTestVlanL2L3Configure
(
    GT_VOID
);

/**
* @internal prvTgfCncTestSendPacketAndCheck function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet:
*         Check that counter's packet count is equail to burstCount
* @param[in] packetInfoPtr            - (pointer to) the packet info
* @param[in] burstCount               -  burst size
* @param[in] blockNum                 -  CNC block number
* @param[in] counterIdx               -  index of counter to check
* @param[in] byteCountPattern         -  expecteded byte counter value
*                                      if 0xFFFFFFFF - the check skipped
* @param[in] cncIncrPerPacket         - number of CNC increments per packet
*                                       None
*/
GT_VOID prvTgfCncTestSendPacketAndCheck
(
    IN TGF_PACKET_STC   *packetInfoPtr,
    IN GT_U32            burstCount,
    IN GT_U32            blockNum,
    IN GT_U32            counterIdx,
    IN GT_U32            byteCountPattern,
    IN GT_U32            cncIncrPerPacket
);

/**
* @internal prvTgfCncTestVlanL2L3TrafficGenerateAndCheck function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet:
*         Check counters.
*/
GT_VOID prvTgfCncTestVlanL2L3TrafficGenerateAndCheck
(
    GT_VOID
);

/**
* @internal prvTgfCncTestSendPacketAndCheckEx function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet:
*         Check counters.
* @param[in] packetInfoPtr            - (pointer to) the packet info
* @param[in] burstCount               -  burst size
* @param[in] blockNum                 -  CNC block number
* @param[in] format                   -  counter format
* @param[in] counterIdx               -  index of counter to check
* @param[in] expectedCounterPtr       -  expecteded counter value
* @param[in] expectedWraparound       - GT_TRUE - expected wraparound
*                                      of the given counter
*                                      GT_FALSE - no wraparound expected
*                                       None
*/
GT_VOID prvTgfCncTestSendPacketAndCheckEx
(
    IN TGF_PACKET_STC                 *packetInfoPtr,
    IN GT_U32                         burstCount,
    IN GT_U32                         blockNum,
    IN PRV_TGF_CNC_COUNTER_FORMAT_ENT format,
    IN GT_U32                         counterIdx,
    IN PRV_TGF_CNC_COUNTER_STC        *initCounterPtr,
    IN PRV_TGF_CNC_COUNTER_STC        *expectedCounterPtr,
    IN GT_BOOL                        expectedWraparound
);

/**
* @internal prvTgfCncTestVlanL2L3TrafficTestFormats function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet:
*         Check counters.
*/
GT_VOID prvTgfCncTestVlanL2L3TrafficTestFormats
(
    GT_VOID
);

/**
* @internal prvTgfCncTestVlanL2L3TrafficTestWraparound function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet:
*         Check counters.
*/
GT_VOID prvTgfCncTestVlanL2L3TrafficTestWraparound
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfCncVlanL2L3 */


