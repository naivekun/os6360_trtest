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
* @file tgfVntGen.h
*
* @brief Generic API for Virtual Network Tester (VNT) Technology facility API.
* VNT features:
* - Operation, Administration, and Maintenance (OAM).
* - Connectivity Fault Management (CFM).
*
* @version   2.
********************************************************************************
*/
#ifndef __tgfVntGenh
#define __tgfVntGenh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef CHX_FAMILY
    #include <cpss/dxCh/dxChxGen/vnt/cpssDxChVnt.h>
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    #include <cpss/exMxPm/exMxPmGen/vnt/cpssExMxPmVnt.h>
#endif /* EXMXPM_FAMILY */


/******************************************************************************\
 *                       CPSS generic API section                             *
\******************************************************************************/

/**
* @enum PRV_TGF_VNT_CFM_REPLY_MODE_ENT
 *
 * @brief Enumeration for CFM opcode change mode.
*/
typedef enum{

    /** @brief When replying to a CFM message, the opcode is taken from
     *  CFM LBR Opcode value.
     */
    PRV_TGF_VNT_CFM_REPLY_MODE_CHANGE_OPCODE_E,

    /** @brief When replying to a CFM message, the least significant bit of
     *  the opcode is changed to zero.
     */
    PRV_TGF_VNT_CFM_REPLY_MODE_CHANGE_LSBIT_E

} PRV_TGF_VNT_CFM_REPLY_MODE_ENT;

/**
* @internal prvTgfVntOamPortLoopBackModeEnableSet function
* @endinternal
*
* @brief   Enable/Disable 802.3ah Loopback mode on the specified port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or port
*/
GT_STATUS prvTgfVntOamPortLoopBackModeEnableSet
(
    IN GT_U32                          portNum,
    IN GT_BOOL                        enable
);

/**
* @internal prvTgfVntOamPortLoopBackModeEnableGet function
* @endinternal
*
* @brief   Get 802.3ah Loopback mode on the specified port
*
* @param[out] enablePtr                - (pointer to) Loopback mode status
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or port
* @retval GT_BAD_PTR               - on one of the parameters is NULL pointer
*/
GT_STATUS prvTgfVntOamPortLoopBackModeEnableGet
(
    IN  GT_U8                         portNum,
    OUT GT_BOOL                      *enablePtr
);

/**
* @internal prvTgfVntOamPortUnidirectionalEnableSet function
* @endinternal
*
* @brief   Enable/Disable the port for unidirectional transmit
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or port
*/
GT_STATUS prvTgfVntOamPortUnidirectionalEnableSet
(
    IN GT_U32                          portNum,
    IN GT_BOOL                        enable
);

/**
* @internal prvTgfVntOamPortUnidirectionalEnableGet function
* @endinternal
*
* @brief   Gets the current status of unidirectional transmit for the port
*
* @param[out] enablePtr                - (pointer to) current status of unidirectional transmit
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or port
* @retval GT_BAD_PTR               - on one of the parameters is NULL pointer
*/
GT_STATUS prvTgfVntOamPortUnidirectionalEnableGet
(
    IN  GT_U8                         portNum,
    OUT GT_BOOL                      *enablePtr
);

/**
* @internal prvTgfVntCfmEtherTypeSet function
* @endinternal
*
* @brief   Sets the EtherType to identify CFM (Connectivity Fault Management) PDUs
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_OUT_OF_RANGE          - on wrong etherType values
*/
GT_STATUS prvTgfVntCfmEtherTypeSet
(
    IN GT_U32                         etherType
);

/**
* @internal prvTgfVntCfmEtherTypeGet function
* @endinternal
*
* @brief   Gets the EtherType to identify CFM (Connectivity Fault Management) PDUs
*
* @param[in] devNum                   - device number
*
* @param[out] etherTypePtr             - (pointer to) CFM (Connectivity Fault Management) EtherType
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on one of the parameters is NULL pointer
*/
GT_STATUS prvTgfVntCfmEtherTypeGet
(
    IN  GT_U8                         devNum,
    OUT GT_U32                       *etherTypePtr
);

/**
* @internal prvTgfVntCfmLbrOpcodeSet function
* @endinternal
*
* @brief   Sets the CFM (Connectivity Fault Management) LBR (Loopback Response) opcode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum, opcode.
* @retval GT_OUT_OF_RANGE          - on wrong opcode values.
*/
GT_STATUS prvTgfVntCfmLbrOpcodeSet
(
    IN GT_U32                         opcode
);

/**
* @internal prvTgfVntCfmLbrOpcodeGet function
* @endinternal
*
* @brief   Gets the current CFM (Connectivity Fault Management)
*         LBR (Loopback Response) opcode
* @param[in] devNum                   - device number
*
* @param[out] opcodePtr                - (pointer to) CFM LBR Opcode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on one of the parameters is NULL pointer
*/
GT_STATUS prvTgfVntCfmLbrOpcodeGet
(
    IN  GT_U8                         devNum,
    OUT GT_U32                       *opcodePtr
);

/**
* @internal prvTgfVntCfmReplyModeSet function
* @endinternal
*
* @brief   Sets the CFM (Connectivity Fault Management)
*         LBR (Loopback Response) opcode change mode.
* @param[in] mode                     - CFM Opcode change mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, change mode.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfVntCfmReplyModeSet
(
    IN PRV_TGF_VNT_CFM_REPLY_MODE_ENT   mode
);

/**
* @internal prvTgfVntCfmReplyModeGet function
* @endinternal
*
* @brief   Gets the CFM (Connectivity Fault Management)
*         LBR (Loopback Response) opcode change mode.
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - (pointer to) CFM Opcode change mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, change mode.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - output parameter is NULL pointer.
*/
GT_STATUS prvTgfVntCfmReplyModeGet
(
    IN GT_U8                            devNum,
    IN PRV_TGF_VNT_CFM_REPLY_MODE_ENT   *modePtr
);

/**
* @internal prvTgfVntPortLastReadTimeStampGet function
* @endinternal
*
* @brief   Gets the last-read-time-stamp counter value that represent the exact
*         timestamp of the last read operation to the PP registers
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] timeStampValuePtr        - (pointer to) timestamp of the last read operation
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on one of the parameters is NULL pointer
*/
GT_STATUS prvTgfVntPortLastReadTimeStampGet
(
    IN  GT_U8                         devNum,
    IN  GT_PHYSICAL_PORT_NUM          portNum,
    OUT GT_U32                       *timeStampValuePtr
);

/**
* @internal prvTgfVntLastReadTimeStampGet function
* @endinternal
*
* @brief   Gets the last-read-time-stamp counter value that represent the exact
*         timestamp of the last read operation to the PP registers
* @param[in] devNum                   - device number
*
* @param[out] timeStampValuePtr        - (pointer to) timestamp of the last read operation
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on one of the parameters is NULL pointer
*/
GT_STATUS prvTgfVntLastReadTimeStampGet
(
    IN  GT_U8                         devNum,
    OUT GT_U32                       *timeStampValuePtr
);

/**
* @internal prvTgfVntOamPortPduTrapEnableSet function
* @endinternal
*
* @brief   Enable/Disable trap to CPU of 802.3ah Link Layer Control protocol on
*         the specified port.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
*/
GT_STATUS prvTgfVntOamPortPduTrapEnableSet
(
    IN GT_U32                          portNum,
    IN GT_BOOL                        enable
);

/**
* @internal prvTgfVntOamPortPduTrapEnableGet function
* @endinternal
*
* @brief   Get trap to CPU status(Enable/Disable) of 802.3ah Link Layer Control protocol on
*         the specified port.
*
* @param[out] enablePtr                - pointer to Loopback mode status:
*                                      GT_TRUE:  Enable trap to CPU of 802.3ah Link Layer Control protocol.
*                                      GT_FALSE: Disable trap to CPU of 802.3ah Link Layer Control protocol.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_BAD_PTR               - on bad pointer of enablePtr
*/
GT_STATUS prvTgfVntOamPortPduTrapEnableGet
(
    IN  GT_U8                         portNum,
    OUT GT_BOOL                      *enablePtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __tgfVntGenh */


