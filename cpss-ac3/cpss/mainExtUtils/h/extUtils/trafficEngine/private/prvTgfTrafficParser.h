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
* @file prvTgfTrafficParser.h
* @version   5
********************************************************************************
*/
#ifndef __prvTgfTrafficParserh
#define __prvTgfTrafficParserh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/******************************************************************************\
 *                            Private API section                              *
\******************************************************************************/

/**
* @internal prvTgfTrafficEnginePacketParse function
* @endinternal
*
* @brief   Parse packet from bytes to parts and fields
*
* @param[in] bufferPtr                - (pointer to) the buffer that hold the packet bytes
* @param[in] bufferLength             - length of the buffer
*
* @param[out] packetInfoPtrPtr         - (pointer to) packet fields info
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - parser caused bad state that should not happen
*
* @note The packetInfoPtrPtr is pointer to static memory that is 'reused' for
*       every new packet that need 'parse' , so use this packet info before
*       calling to new 'parse'
*
*/
GT_STATUS prvTgfTrafficEnginePacketParse
(
    IN  GT_U8              *bufferPtr,
    IN  GT_U32              bufferLength,
    OUT TGF_PACKET_STC    **packetInfoPtrPtr
);

/**
* @internal prvTgfTrafficGeneratorPacketDsaTagBuild function
* @endinternal
*
* @brief   Build DSA tag bytes on the packet from the DSA tag parameters .
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dsaInfoPtr               - (pointer to) the DSA parameters that need to be built into
*                                      the packet
*
* @param[out] dsaBytesPtr              - pointer to the start of DSA tag in the packet
*                                      This pointer in the packet should hold space for :
*                                      4 bytes when using regular DSA tag.
*                                      8 bytes when using extended DSA tag.
*                                      This function only set values into the 4 or 8 bytes
*                                      according to the parameters in parameter dsaInfoPtr.
*                                      NOTE : this memory is the caller's responsibility (to
*                                      allocate / free)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTrafficGeneratorPacketDsaTagBuild
(
    IN  TGF_PACKET_DSA_TAG_STC         *dsaInfoPtr,
    OUT GT_U8                          *dsaBytesPtr
);

/**
* @internal prvTgfTrafficGeneratorPacketDsaTagParse function
* @endinternal
*
* @brief   parse the DSA tag bytes from the packet into the DSA tag parameters .
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dsaBytesPtr              - pointer to the start of DSA tag in the packet.
*
* @param[out] dsaInfoPtr               - (pointer to) the DSA parameters that parsed from the packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTrafficGeneratorPacketDsaTagParse
(
    IN  GT_U8                          *dsaBytesPtr,
    OUT TGF_PACKET_DSA_TAG_STC         *dsaInfoPtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfTrafficParserh */


