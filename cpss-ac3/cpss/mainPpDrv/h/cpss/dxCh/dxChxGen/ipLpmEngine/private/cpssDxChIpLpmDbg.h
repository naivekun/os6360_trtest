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
* @file cpssDxChIpLpmDbg.h
*
* @brief IP LPM debug functions
*
* @version   12
********************************************************************************
*/

#ifndef __cpssDxChIpLpmDbgh
#define __cpssDxChIpLpmDbgh

#include <cpss/dxCh/dxChxGen/ipLpmEngine/private/cpssDxChPrvIpLpmTypes.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal cpssDxChIpLpmDbgRouteTcamDump function
* @endinternal
*
* @brief   This func makes physical router tcam scanning and prints its contents.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] dump                     -  parameter for debugging purposes
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on failure.
*/
GT_STATUS cpssDxChIpLpmDbgRouteTcamDump
(
    IN GT_BOOL dump
);

/**
* @internal cpssDxChIpLpmDbgPatTrieValidityCheck function
* @endinternal
*
* @brief   This function checks Patricia trie validity.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router identifier.
* @param[in] protocol                 - ip protocol
* @param[in] prefixType               - uc/mc prefix type
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_STATE             - on bad state in patricia trie
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpLpmDbgPatTrieValidityCheck
(
  IN GT_U32                     lpmDBId,
  IN GT_U32                     vrId,
  IN CPSS_IP_PROTOCOL_STACK_ENT protocol,
  IN CPSS_UNICAST_MULTICAST_ENT prefixType
);

/**
* @internal cpssDxChIpLpmDbgPatTriePrint function
* @endinternal
*
* @brief   This function prints Patricia trie contents.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] lpmDbId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router identifier.
* @param[in] protocol                 - ip protocol
* @param[in] prefixType               - uc/mc prefix type
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - if lpmDbId or vrId is not found
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpLpmDbgPatTriePrint
(
    IN GT_U32                     lpmDbId,
    IN GT_U32                     vrId,
    IN CPSS_IP_PROTOCOL_STACK_ENT protocol,
    IN CPSS_UNICAST_MULTICAST_ENT prefixType
);

/**
* @internal cpssDxChIpLpmDbgHwValidation function
* @endinternal
*
* @brief   Validate the LPM in HW
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The device number
* @param[in] vrId                     - The virtual router id
* @param[in] protocol                 - The IP protocol
* @param[in] prefixType               - Unicast or multicast
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpLpmDbgHwValidation
(
    IN GT_U8                            devNum,
    IN GT_U32                           vrId,
    IN CPSS_IP_PROTOCOL_STACK_ENT       protocol,
    IN CPSS_UNICAST_MULTICAST_ENT       prefixType
);

/**
* @internal cpssDxChIpLpmDbgShadowValidityCheck function
* @endinternal
*
* @brief   Validate the LPM in Shadow
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] lpmDbId                  - The LPM DB id
* @param[in] vrId                     - The virtual router id
* @param[in] protocol                 - The IP protocol
* @param[in] prefixType               - Unicast or multicast
* @param[in] returnOnFailure          - whether to return on first failure
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpLpmDbgShadowValidityCheck
(
    IN GT_U32                           lpmDbId,
    IN GT_U32                           vrId,
    IN CPSS_IP_PROTOCOL_STACK_ENT       protocol,
    IN CPSS_UNICAST_MULTICAST_ENT       prefixType,
    IN GT_BOOL                          returnOnFailure
);

/**
* @internal cpssDxChIpLpmDbgHwShadowSyncValidityCheck function
* @endinternal
*
* @brief   This function validates synchronization between the SW and HW of the LPM
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The device number
* @param[in] lpmDbId                  - The LPM DB id
* @param[in] vrId                     - The virtual router id
* @param[in] protocol                 - The IP protocol
* @param[in] prefixType               - Unicast or multicast
* @param[in] returnOnFailure          - whether to return on first failure
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpLpmDbgHwShadowSyncValidityCheck
(
    IN GT_U8                            devNum,
    IN GT_U32                           lpmDbId,
    IN GT_U32                           vrId,
    IN CPSS_IP_PROTOCOL_STACK_ENT       protocol,
    IN CPSS_UNICAST_MULTICAST_ENT       prefixType,
    IN GT_BOOL                          returnOnFailure
);

/**
* @internal cpssDxChIpLpmDbgHwShadowValidityAndSyncCheck function
* @endinternal
*
* @brief   This function Validate the LPM in Shadow, Validate the LPM in HW and
*         check synchronization between the SW and HW of the LPM
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The device number
* @param[in] lpmDbId                  - The LPM DB id
* @param[in] vrId                     - The virtual router id
* @param[in] protocol                 - The IP protocol
* @param[in] prefixType               - Unicast or multicast
* @param[in] returnOnFailure          - whether to return on first failure
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpLpmDbgHwShadowValidityAndSyncCheck
(
    IN GT_U8                            devNum,
    IN GT_U32                           lpmDbId,
    IN GT_U32                           vrId,
    IN CPSS_IP_PROTOCOL_STACK_ENT       protocol,
    IN CPSS_UNICAST_MULTICAST_ENT       prefixType,
    IN GT_BOOL                          returnOnFailure
);

/**
* @internal cpssDxChIpLpmDbgPrefixLengthSet function
* @endinternal
*
* @brief   set prefix length used in addManyByOctet APIs
*
* @param[in] protocol                 - ip protocol
* @param[in] prefixType               - uc/mc prefix type
* @param[in] prefixLength             - prefix length
* @param[in] srcPrefixLength          - src prefix length
*/
GT_U32 cpssDxChIpLpmDbgPrefixLengthSet
(
    IN CPSS_IP_PROTOCOL_STACK_ENT protocolStack,
    IN CPSS_UNICAST_MULTICAST_ENT prefixType,
    IN GT_U32                     prefixLength,
    IN GT_U32                     srcPrefixLength
);

/**
* @internal cpssDxChIpLpmDbgIpv4UcPrefixAddMany function
* @endinternal
*
* @brief   This debug function tries to add many sequential IPv4 Unicast prefixes and
*         returns the number of prefixes successfully added.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] startIpAddr              - the first address to add
* @param[in] routeEntryBaseMemAddr    - base memory Address (offset) of the route entry
* @param[in] numOfPrefixesToAdd       - the number of prefixes to add
*
* @param[out] lastIpAddrAddedPtr       - points to the last prefix successfully
*                                      added (NULL to ignore)
* @param[out] numOfPrefixesAddedPtr    - points to the nubmer of prefixes that were
*                                      successfully added (NULL to ignore)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to check the number of the prefixes
*       that can be added. All the prefixes are added with exact match (prefix
*       length 32). The route entry is not written.
*
*/
GT_STATUS cpssDxChIpLpmDbgIpv4UcPrefixAddMany
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPADDR   startIpAddr,
    IN  GT_U32      routeEntryBaseMemAddr,
    IN  GT_U32      numOfPrefixesToAdd,
    OUT GT_IPADDR   *lastIpAddrAddedPtr,
    OUT GT_U32      *numOfPrefixesAddedPtr
);

/**
* @internal cpssDxChIpLpmDbgIpv4UcPrefixAddManyRandom function
* @endinternal
*
* @brief   This function tries to add many random IPv4 Unicast prefixes and
*         returns the number of prefixes successfully added.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] startIpAddr              - the first address to add
* @param[in] routeEntryBaseMemAddr    - base memory Address (offset) of the route entry
* @param[in] numOfPrefixesToAdd       - the number of prefixes to add
* @param[in] isWholeIpRandom          - GT_TRUE: all IP octets calculated by cpssOsRand
*                                      GT_FALSE: only 2 LSB octets calculated by cpssOsRand
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
*
* @param[out] numOfPrefixesAddedPtr    - points to the nubmer of prefixes that were
*                                      successfully added (NULL to ignore)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to check the number of the prefixes
*       that can be added. All the prefixes are added with exact match (prefix
*       length 32). The route entry is not written.
*
*/
GT_STATUS cpssDxChIpLpmDbgIpv4UcPrefixAddManyRandom
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPADDR   startIpAddr,
    IN  GT_U32      routeEntryBaseMemAddr,
    IN  GT_U32      numOfPrefixesToAdd,
    IN  GT_BOOL     isWholeIpRandom,
    IN  GT_BOOL     defragmentationEnable,
    OUT GT_U32      *numOfPrefixesAddedPtr
);

/**
* @internal cpssDxChIpLpmDbgDump function
* @endinternal
*
* @brief   Print hardware LPM dump
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The device number
* @param[in] vrId                     - The virtual router id
* @param[in] protocol                 - The IP protocol
* @param[in] prefixType               - Unicast or multicast
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpLpmDbgDump
(
    IN GT_U8                            devNum,
    IN GT_U32                           vrId,
    IN CPSS_IP_PROTOCOL_STACK_ENT       protocol,
    IN CPSS_UNICAST_MULTICAST_ENT       prefixType
);

/**
* @internal cpssDxChIpLpmDbgHwMemPrint function
* @endinternal
*
* @brief   Print DMM partition debug information for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] lpmDbId                  - The LPM DB id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpLpmDbgHwMemPrint
(
    IN GT_U32                           lpmDbId
);

/**
* @internal cpssDxChIpLpmDbgIpv4UcPrefixAddManyByOctet function
* @endinternal
*
* @brief   This function tries to add many sequential IPv4 Unicast prefixes according
*         to octet order selection and returns the number of prefixes successfully added.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] startIpAddr              - the first address to add
* @param[in] routeEntryBaseMemAddr    - base memory Address (offset) of the route entry
* @param[in] numOfPrefixesToAdd       - the number of prefixes to add
* @param[in] firstOctetToChange       - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] secondOctetToChange      - second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] thirdOctetToChange       - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fourthOctetToChange      - fourth octet to change in the prefix address.
*                                      value of the octet can be up to 255
*
* @param[out] lastIpAddrAddedPtr       - points to the last prefix successfully
*                                      added (NULL to ignore)
* @param[out] numOfPrefixesAddedPtr    - points to the nubmer of prefixes that were
*                                      successfully added (NULL to ignore)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to check the number of the prefixes
*       that can be added. All the prefixes are added with exact match (prefix
*       length 32). The route entry is not written.
*
*/
GT_STATUS cpssDxChIpLpmDbgIpv4UcPrefixAddManyByOctet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPADDR   startIpAddr,
    IN  GT_U32      routeEntryBaseMemAddr,
    IN  GT_U32      numOfPrefixesToAdd,
    IN  GT_U32      firstOctetToChange,
    IN  GT_U32      secondOctetToChange,
    IN  GT_U32      thirdOctetToChange,
    IN  GT_U32      fourthOctetToChange,
    OUT GT_IPADDR   *lastIpAddrAddedPtr,
    OUT GT_U32      *numOfPrefixesAddedPtr
);

/**
* @internal cpssDxChIpLpmDbgIpv4UcPrefixDelManyByOctet function
* @endinternal
*
* @brief   This function tries to delete many sequential IPv4 Unicast prefixes according
*         to octet order selection and returns the number of prefixes successfully deleted.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] startIpAddr              - the first address to add
* @param[in] numOfPrefixesToDel       - the number of prefixes to delete
* @param[in] firstOctetToChange       - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] secondOctetToChange      - second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] thirdOctetToChange       - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fourthOctetToChange      - fourth octet to change in the prefix address.
*                                      value of the octet can be up to 255
*
* @param[out] lastIpAddrDeletedPtr     - points to the last prefix successfully
*                                      deleted (NULL to ignore)
* @param[out] numOfPrefixesDeletedPtr  - points to the nubmer of prefixes that were
*                                      successfully deleted (NULL to ignore)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to delete the number of the prefixes
*       that was added by cpssDxChIpLpmDbgIpv4UcPrefixAddManyByOctet.
*
*/
GT_STATUS cpssDxChIpLpmDbgIpv4UcPrefixDelManyByOctet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPADDR   startIpAddr,
    IN  GT_U32      numOfPrefixesToDel,
    IN  GT_U32      firstOctetToChange,
    IN  GT_U32      secondOctetToChange,
    IN  GT_U32      thirdOctetToChange,
    IN  GT_U32      fourthOctetToChange,
    OUT GT_IPADDR   *lastIpAddrDeletedPtr,
    OUT GT_U32      *numOfPrefixesDeletedPtr
);

/**
* @internal cpssDxChIpLpmDbgIpv4UcPrefixGetManyByOctet function
* @endinternal
*
* @brief   This function tries to get many sequential IPv4 Unicast prefixes (that were
*         added by cpssDxChIpLpmDbgIpv4UcPrefixAddManyByOctet) and returns the number of
*         successfully retrieved prefixes.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] startIpAddr              - the first address to add
* @param[in] numOfPrefixesToGet       - the number of prefixes to get
* @param[in] firstOctetToChange       - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] secondOctetToChange      - second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] thirdOctetToChange       - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fourthOctetToChange      - fourth octet to change in the prefix address.
*                                      value of the octet can be up to 255
*
* @param[out] numOfRetrievedPrefixesPtr - points to the nubmer of prefixes that were
*                                      successfully retrieved (NULL to ignore)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to check the number of the prefixes
*       that were be added.
*
*/
GT_STATUS cpssDxChIpLpmDbgIpv4UcPrefixGetManyByOctet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPADDR   startIpAddr,
    IN  GT_U32      numOfPrefixesToGet,
    IN  GT_U32      firstOctetToChange,
    IN  GT_U32      secondOctetToChange,
    IN  GT_U32      thirdOctetToChange,
    IN  GT_U32      fourthOctetToChange,
    OUT GT_U32      *numOfRetrievedPrefixesPtr
);

/**
* @internal cpssDxChIpLpmDbgIpv6UcPrefixAddManyByOctet function
* @endinternal
*
* @brief   This function tries to add many sequential IPv6 Unicast prefixes according
*         to octet order selection and returns the number of prefixes successfully added.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] startIpAddr              - the first address to add
* @param[in] routeEntryBaseMemAddr    - base memory Address (offset) of the route entry
* @param[in] numOfPrefixesToAdd       - the number of prefixes to add
* @param[in] firstOctetToChange       - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] secondOctetToChange      - second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] thirdOctetToChange       - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fourthOctetToChange      - fourth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fifthOctetToChange       - fifth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] sixthOctetToChange       - sixth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] seventhOctetToChange     - seventh octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] eighthOctetToChange      - eighth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] ninthOctetToChange       - ninth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] tenthOctetToChange       - tenth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] elevenOctetToChange      - eleven octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] twelveOctetToChange      - twelve octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] thirteenOctetToChange    - thirteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fourteenOctetToChange    - fourteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fifteenOctetToChange     - fifteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] sixteenOctetToChange     - sixteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
*
* @param[out] lastIpAddrAddedPtr       - points to the last prefix successfully
*                                      added (NULL to ignore)
* @param[out] numOfPrefixesAddedPtr    - points to the nubmer of prefixes that were
*                                      successfully added (NULL to ignore)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to check the number of the prefixes
*       that can be added. All the prefixes are added with exact match (prefix
*       length 128). The route entry is not written.
*
*/
GT_STATUS cpssDxChIpLpmDbgIpv6UcPrefixAddManyByOctet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPV6ADDR startIpAddr,
    IN  GT_U32      routeEntryBaseMemAddr,
    IN  GT_U32      numOfPrefixesToAdd,
    IN  GT_U32      firstOctetToChange,
    IN  GT_U32      secondOctetToChange,
    IN  GT_U32      thirdOctetToChange,
    IN  GT_U32      fourthOctetToChange,
    IN  GT_U32      fifthOctetToChange,
    IN  GT_U32      sixthOctetToChange,
    IN  GT_U32      seventhOctetToChange,
    IN  GT_U32      eighthOctetToChange,
    IN  GT_U32      ninthOctetToChange,
    IN  GT_U32      tenthOctetToChange,
    IN  GT_U32      elevenOctetToChange,
    IN  GT_U32      twelveOctetToChange,
    IN  GT_U32      thirteenOctetToChange,
    IN  GT_U32      fourteenOctetToChange,
    IN  GT_U32      fifteenOctetToChange,
    IN  GT_U32      sixteenOctetToChange,
    OUT GT_IPV6ADDR *lastIpAddrAddedPtr,
    OUT GT_U32      *numOfPrefixesAddedPtr
);

/**
* @internal cpssDxChIpLpmDbgIpv6UcPrefixDelManyByOctet function
* @endinternal
*
* @brief   This function tries to delete many sequential IPv6 Unicast prefixes according
*         to octet order selection and returns the number of prefixes successfully deleted.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] startIpAddr              - the first address to add
* @param[in] numOfPrefixesToDel       - the number of prefixes to deleted
* @param[in] firstOctetToChange       - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] secondOctetToChange      - second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] thirdOctetToChange       - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fourthOctetToChange      - fourth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fifthOctetToChange       - fifth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] sixthOctetToChange       - sixth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] seventhOctetToChange     - seventh octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] eighthOctetToChange      - eighth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] ninthOctetToChange       - ninth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] tenthOctetToChange       - tenth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] elevenOctetToChange      - eleven octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] twelveOctetToChange      - twelve octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] thirteenOctetToChange    - thirteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fourteenOctetToChange    - fourteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fifteenOctetToChange     - fifteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] sixteenOctetToChange     - sixteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to delete the number of prefixes
*       that was added by cpssDxChIpLpmDbgIpv6UcPrefixAddManyByOctet.
*
*/
GT_STATUS cpssDxChIpLpmDbgIpv6UcPrefixDelManyByOctet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPV6ADDR startIpAddr,
    IN  GT_U32      numOfPrefixesToDel,
    IN  GT_U32      firstOctetToChange,
    IN  GT_U32      secondOctetToChange,
    IN  GT_U32      thirdOctetToChange,
    IN  GT_U32      fourthOctetToChange,
    IN  GT_U32      fifthOctetToChange,
    IN  GT_U32      sixthOctetToChange,
    IN  GT_U32      seventhOctetToChange,
    IN  GT_U32      eighthOctetToChange,
    IN  GT_U32      ninthOctetToChange,
    IN  GT_U32      tenthOctetToChange,
    IN  GT_U32      elevenOctetToChange,
    IN  GT_U32      twelveOctetToChange,
    IN  GT_U32      thirteenOctetToChange,
    IN  GT_U32      fourteenOctetToChange,
    IN  GT_U32      fifteenOctetToChange,
    IN  GT_U32      sixteenOctetToChange,
    OUT GT_IPV6ADDR *lastIpAddrAddedPtr,
    OUT GT_U32      *numOfPrefixesDeletedPtr
);

/**
* @internal cpssDxChIpLpmDbgIpv6UcPrefixGetManyByOctet function
* @endinternal
*
* @brief   This function tries to get many sequential IPv6 Unicast prefixes (that were
*         added by cpssDxChIpLpmDbgIpv6UcPrefixAddManyByOctet) and returns the number of
*         successfully retrieved prefixes.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] startIpAddr              - the first address to add
* @param[in] numOfPrefixesToGet       - the number of prefixes to get
* @param[in] firstOctetToChange       - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] secondOctetToChange      - second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] thirdOctetToChange       - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fourthOctetToChange      - fourth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fifthOctetToChange       - fifth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] sixthOctetToChange       - sixth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] seventhOctetToChange     - seventh octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] eighthOctetToChange      - eighth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] ninthOctetToChange       - ninth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] tenthOctetToChange       - tenth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] elevenOctetToChange      - eleven octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] twelveOctetToChange      - twelve octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] thirteenOctetToChange    - thirteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fourteenOctetToChange    - fourteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fifteenOctetToChange     - fifteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] sixteenOctetToChange     - sixteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
*
* @param[out] numOfRetrievedPrefixesPtr - points to the nubmer of prefixes that were
*                                      successfully retrieved (NULL to ignore)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to check the number of the prefixes
*       that were be added.
*
*/
GT_STATUS cpssDxChIpLpmDbgIpv6UcPrefixGetManyByOctet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPV6ADDR startIpAddr,
    IN  GT_U32      numOfPrefixesToGet,
    IN  GT_U32      firstOctetToChange,
    IN  GT_U32      secondOctetToChange,
    IN  GT_U32      thirdOctetToChange,
    IN  GT_U32      fourthOctetToChange,
    IN  GT_U32      fifthOctetToChange,
    IN  GT_U32      sixthOctetToChange,
    IN  GT_U32      seventhOctetToChange,
    IN  GT_U32      eighthOctetToChange,
    IN  GT_U32      ninthOctetToChange,
    IN  GT_U32      tenthOctetToChange,
    IN  GT_U32      elevenOctetToChange,
    IN  GT_U32      twelveOctetToChange,
    IN  GT_U32      thirteenOctetToChange,
    IN  GT_U32      fourteenOctetToChange,
    IN  GT_U32      fifteenOctetToChange,
    IN  GT_U32      sixteenOctetToChange,
    OUT GT_U32      *numOfRetrievedPrefixesPtr
);

/**
* @internal cpssDxChIpLpmDbgIpv6UcPrefixAddRandomByRange function
* @endinternal
*
* @brief   This function tries to add many random IPv6 Unicast prefixes and
*         returns the number of prefixes successfully added.
*         Random only on last 2 octets.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] startIpAddr              - the first address to add
* @param[in] lastIpAddr               - the last address to add
* @param[in] routeEntryBaseMemAddr    - base memory Address (offset) of the route entry
* @param[in] returnOnFirstFail        - Whether to return on first fail
* @param[in] step                     -  between consecutive addresses
*
* @param[out] lastIpAddrAddedPtr       - points to the last prefix successfully
*                                      added (NULL to ignore)
* @param[out] numOfPrefixesAddedPtr    - points to the nubmer of prefixes that were
*                                      successfully added (NULL to ignore)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to check the number of the prefixes
*       that can be added randomly between range. All the prefixes are added with exact match
*       (prefix length 128). The route entry is not written.
*
*/
GT_STATUS cpssDxChIpLpmDbgIpv6UcPrefixAddRandomByRange
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPV6ADDR startIpAddr,
    IN  GT_IPV6ADDR lastIpAddr,
    IN  GT_U32      routeEntryBaseMemAddr,
    IN  GT_BOOL     returnOnFirstFail,
    IN  GT_U32      step,
    OUT GT_IPV6ADDR *lastIpAddrAddedPtr,
    OUT GT_U32      *numOfPrefixesAddedPtr
);

/**
* @internal cpssDxChIpLpmDbgIpv4McPrefixAddManyByOctet function
* @endinternal
*
* @brief   This function tries to add many sequential IPv4 Multicast prefixes according
*         to octet order selection and returns the number of prefixes successfully added.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] grpStartIpAddr           - the first group address to add
* @param[in] srcStartIpAddr           - the first source address to add
* @param[in] routeEntryBaseMemAddr    - base memory Address (offset) of the route entry
* @param[in] numOfPrefixesToAdd       - the number of prefixes to add
* @param[in] changeGrpAddr            - GT_TRUE: change the group address
*                                      GT_FALSE: don't change the group address
* @param[in] grpFirstOctetToChange    - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpSecondOctetToChange   second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpThirdOctetToChange    - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpFourthOctetToChange   fourth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] changeSrcAddr            - GT_TRUE: change the source address
*                                      GT_FALSE: don't change the source address
* @param[in] srcFirstOctetToChange    - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcSecondOctetToChange   second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcThirdOctetToChange    - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcFourthOctetToChange   fourth octet to change in the prefix address.
*                                      value of the octet can be up to 255
*
* @param[out] grpLastIpAddrAddedPtr    - points to the last group prefix successfully
*                                      added (NULL to ignore)
* @param[out] srcLastIpAddrAddedPtr    - points to the last source prefix successfully
*                                      added (NULL to ignore)
* @param[out] numOfPrefixesAddedPtr    - points to the nubmer of prefixes that were
*                                      successfully added (NULL to ignore)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to check the number of the prefixes
*       that can be added. All the prefixes are added with exact match (prefix
*       length 32). The route entry is not written.
*
*/
GT_STATUS cpssDxChIpLpmDbgIpv4McPrefixAddManyByOctet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPADDR   grpStartIpAddr,
    IN  GT_IPADDR   srcStartIpAddr,
    IN  GT_U32      routeEntryBaseMemAddr,
    IN  GT_U32      numOfPrefixesToAdd,
    IN  GT_BOOL     changeGrpAddr,
    IN  GT_U32      grpFirstOctetToChange,
    IN  GT_U32      grpSecondOctetToChange,
    IN  GT_U32      grpThirdOctetToChange,
    IN  GT_U32      grpFourthOctetToChange,
    IN  GT_BOOL     changeSrcAddr,
    IN  GT_U32      srcFirstOctetToChange,
    IN  GT_U32      srcSecondOctetToChange,
    IN  GT_U32      srcThirdOctetToChange,
    IN  GT_U32      srcFourthOctetToChange,
    OUT GT_IPADDR   *grpLastIpAddrAddedPtr,
    OUT GT_IPADDR   *srcLastIpAddrAddedPtr,
    OUT GT_U32      *numOfPrefixesAddedPtr
);

/**
* @internal cpssDxChIpLpmDbgIpv4McPrefixGetManyByOctet function
* @endinternal
*
* @brief   This function tries to get many sequential IPv4 Multicast prefixes (that were
*         added by cpssDxChIpLpmDbgIpv4McPrefixAddManyByOctet) and returns the number of
*         successfully retrieved prefixes.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] grpStartIpAddr           - the first group address to add
* @param[in] srcStartIpAddr           - the first source address to add
* @param[in] numOfPrefixesToGet       - the number of prefixes to get
* @param[in] changeGrpAddr            - GT_TRUE: change the group address
*                                      GT_FALSE: don't change the group address
* @param[in] grpFirstOctetToChange    - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpSecondOctetToChange   second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpThirdOctetToChange    - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpFourthOctetToChange   fourth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] changeSrcAddr            - GT_TRUE: change the source address
*                                      GT_FALSE: don't change the source address
* @param[in] srcFirstOctetToChange    - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcSecondOctetToChange   second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcThirdOctetToChange    - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcFourthOctetToChange   fourth octet to change in the prefix address.
*                                      value of the octet can be up to 255
*
* @param[out] numOfRetrievedPrefixesPtr - points to the nubmer of prefixes that were
*                                      successfully retrieved (NULL to ignore)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to check the number of the prefixes
*       that were be added.
*
*/
GT_STATUS cpssDxChIpLpmDbgIpv4McPrefixGetManyByOctet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPADDR   grpStartIpAddr,
    IN  GT_IPADDR   srcStartIpAddr,
    IN  GT_U32      numOfPrefixesToGet,
    IN  GT_BOOL     changeGrpAddr,
    IN  GT_U32      grpFirstOctetToChange,
    IN  GT_U32      grpSecondOctetToChange,
    IN  GT_U32      grpThirdOctetToChange,
    IN  GT_U32      grpFourthOctetToChange,
    IN  GT_BOOL     changeSrcAddr,
    IN  GT_U32      srcFirstOctetToChange,
    IN  GT_U32      srcSecondOctetToChange,
    IN  GT_U32      srcThirdOctetToChange,
    IN  GT_U32      srcFourthOctetToChange,
    OUT GT_U32      *numOfRetrievedPrefixesPtr
);

/**
* @internal cpssDxChIpLpmDbgIpv6McPrefixAddManyByOctet function
* @endinternal
*
* @brief   This function tries to add many sequential IPv6 Multicast prefixes according
*         to octet order selection and returns the number of prefixes successfully added.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] grpStartIpAddr           - the first group address to add
*                                      ipGroupRuleIndex      - index of the IPv6 mc group rule in the TCAM.
* @param[in] srcStartIpAddr           - the first source address to add
* @param[in] routeEntryBaseMemAddr    - base memory Address (offset) of the route entry
* @param[in] numOfPrefixesToAdd       - the number of prefixes to add
* @param[in] changeGrpAddr            - GT_TRUE: change the group address
*                                      GT_FALSE: don't change the group address
* @param[in] grpFirstOctetToChange    - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpSecondOctetToChange   second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpThirdOctetToChange    - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpFourthOctetToChange   fourth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpFifthOctetToChange    - fifth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpSixthOctetToChange    - sixth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpSeventhOctetToChange  - seventh octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpEighthOctetToChange   eighth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpNinthOctetToChange    - ninth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpTenthOctetToChange    - tenth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpElevenOctetToChange   eleven octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpTwelveOctetToChange   twelve octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpThirteenOctetToChange - thirteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpFourteenOctetToChange - fourteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpFifteenOctetToChange  - fifteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpSixteenOctetToChange  - sixteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] changeSrcAddr            - GT_TRUE: change the source address
*                                      GT_FALSE: don't change the source address
* @param[in] srcFirstOctetToChange    - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcSecondOctetToChange   second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcThirdOctetToChange    - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcFourthOctetToChange   fourth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcFifthOctetToChange    - fifth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcSixthOctetToChange    - sixth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcSeventhOctetToChange  - seventh octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcEighthOctetToChange   eighth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcNinthOctetToChange    - ninth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcTenthOctetToChange    - tenth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcElevenOctetToChange   eleven octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcTwelveOctetToChange   twelve octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcThirteenOctetToChange - thirteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcFourteenOctetToChange - fourteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcFifteenOctetToChange  - fifteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcSixteenOctetToChange  - sixteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
*
* @param[out] grpLastIpAddrAddedPtr    - points to the last group prefix successfully
*                                      added (NULL to ignore)
* @param[out] srcLastIpAddrAddedPtr    - points to the last source prefix successfully
*                                      added (NULL to ignore)
* @param[out] numOfPrefixesAddedPtr    - points to the nubmer of prefixes that were
*                                      successfully added (NULL to ignore)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to check the number of the prefixes
*       that can be added. All the prefixes are added with exact match (prefix
*       length 128). The route entry is not written.
*
*/
GT_STATUS cpssDxChIpLpmDbgIpv6McPrefixAddManyByOctet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPV6ADDR grpStartIpAddr,
    IN  GT_IPV6ADDR srcStartIpAddr,
    IN  GT_U32      routeEntryBaseMemAddr,
    IN  GT_U32      numOfPrefixesToAdd,
    IN  GT_BOOL     changeGrpAddr,
    IN  GT_U32      grpFirstOctetToChange,
    IN  GT_U32      grpSecondOctetToChange,
    IN  GT_U32      grpThirdOctetToChange,
    IN  GT_U32      grpFourthOctetToChange,
    IN  GT_U32      grpFifthOctetToChange,
    IN  GT_U32      grpSixthOctetToChange,
    IN  GT_U32      grpSeventhOctetToChange,
    IN  GT_U32      grpEighthOctetToChange,
    IN  GT_U32      grpNinthOctetToChange,
    IN  GT_U32      grpTenthOctetToChange,
    IN  GT_U32      grpElevenOctetToChange,
    IN  GT_U32      grpTwelveOctetToChange,
    IN  GT_U32      grpThirteenOctetToChange,
    IN  GT_U32      grpFourteenOctetToChange,
    IN  GT_U32      grpFifteenOctetToChange,
    IN  GT_U32      grpSixteenOctetToChange,
    IN  GT_BOOL     changeSrcAddr,
    IN  GT_U32      srcFirstOctetToChange,
    IN  GT_U32      srcSecondOctetToChange,
    IN  GT_U32      srcThirdOctetToChange,
    IN  GT_U32      srcFourthOctetToChange,
    IN  GT_U32      srcFifthOctetToChange,
    IN  GT_U32      srcSixthOctetToChange,
    IN  GT_U32      srcSeventhOctetToChange,
    IN  GT_U32      srcEighthOctetToChange,
    IN  GT_U32      srcNinthOctetToChange,
    IN  GT_U32      srcTenthOctetToChange,
    IN  GT_U32      srcElevenOctetToChange,
    IN  GT_U32      srcTwelveOctetToChange,
    IN  GT_U32      srcThirteenOctetToChange,
    IN  GT_U32      srcFourteenOctetToChange,
    IN  GT_U32      srcFifteenOctetToChange,
    IN  GT_U32      srcSixteenOctetToChange,
    OUT GT_IPV6ADDR *grpLastIpAddrAddedPtr,
    OUT GT_IPV6ADDR *srcLastIpAddrAddedPtr,
    OUT GT_U32      *numOfPrefixesAddedPtr
);

/**
* @internal cpssDxChIpLpmDbgIpv6McPrefixGetManyByOctet function
* @endinternal
*
* @brief   This function tries to get many sequential IPv6 Multicast prefixes (that were
*         added by cpssDxChIpLpmDbgIpv6McPrefixAddManyByOctet) and returns the number of
*         successfully retrieved prefixes.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] grpStartIpAddr           - the first group address to add
* @param[in] srcStartIpAddr           - the first source address to add
* @param[in] numOfPrefixesToGet       - the number of prefixes to get
* @param[in] changeGrpAddr            - GT_TRUE: change the group address
*                                      GT_FALSE: don't change the group address
* @param[in] grpFirstOctetToChange    - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpSecondOctetToChange   second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpThirdOctetToChange    - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpFourthOctetToChange   fourth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpFifthOctetToChange    - fifth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpSixthOctetToChange    - sixth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpSeventhOctetToChange  - seventh octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpEighthOctetToChange   eighth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpNinthOctetToChange    - ninth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpTenthOctetToChange    - tenth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpElevenOctetToChange   eleven octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpTwelveOctetToChange   twelve octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpThirteenOctetToChange - thirteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpFourteenOctetToChange - fourteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpFifteenOctetToChange  - fifteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpSixteenOctetToChange  - sixteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] changeSrcAddr            - GT_TRUE: change the source address
*                                      GT_FALSE: don't change the source address
* @param[in] srcFirstOctetToChange    - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcSecondOctetToChange   second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcThirdOctetToChange    - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcFourthOctetToChange   fourth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcFifthOctetToChange    - fifth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcSixthOctetToChange    - sixth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcSeventhOctetToChange  - seventh octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcEighthOctetToChange   eighth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcNinthOctetToChange    - ninth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcTenthOctetToChange    - tenth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcElevenOctetToChange   eleven octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcTwelveOctetToChange   twelve octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcThirteenOctetToChange - thirteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcFourteenOctetToChange - fourteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcFifteenOctetToChange  - fifteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcSixteenOctetToChange  - sixteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
*
* @param[out] numOfRetrievedPrefixesPtr - points to the nubmer of prefixes that were
*                                      successfully retrieved (NULL to ignore)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to check the number of the prefixes
*       that were be added.
*
*/
GT_STATUS cpssDxChIpLpmDbgIpv6McPrefixGetManyByOctet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPV6ADDR grpStartIpAddr,
    IN  GT_IPV6ADDR srcStartIpAddr,
    IN  GT_U32      numOfPrefixesToGet,
    IN  GT_BOOL     changeGrpAddr,
    IN  GT_U32      grpFirstOctetToChange,
    IN  GT_U32      grpSecondOctetToChange,
    IN  GT_U32      grpThirdOctetToChange,
    IN  GT_U32      grpFourthOctetToChange,
    IN  GT_U32      grpFifthOctetToChange,
    IN  GT_U32      grpSixthOctetToChange,
    IN  GT_U32      grpSeventhOctetToChange,
    IN  GT_U32      grpEighthOctetToChange,
    IN  GT_U32      grpNinthOctetToChange,
    IN  GT_U32      grpTenthOctetToChange,
    IN  GT_U32      grpElevenOctetToChange,
    IN  GT_U32      grpTwelveOctetToChange,
    IN  GT_U32      grpThirteenOctetToChange,
    IN  GT_U32      grpFourteenOctetToChange,
    IN  GT_U32      grpFifteenOctetToChange,
    IN  GT_U32      grpSixteenOctetToChange,
    IN  GT_BOOL     changeSrcAddr,
    IN  GT_U32      srcFirstOctetToChange,
    IN  GT_U32      srcSecondOctetToChange,
    IN  GT_U32      srcThirdOctetToChange,
    IN  GT_U32      srcFourthOctetToChange,
    IN  GT_U32      srcFifthOctetToChange,
    IN  GT_U32      srcSixthOctetToChange,
    IN  GT_U32      srcSeventhOctetToChange,
    IN  GT_U32      srcEighthOctetToChange,
    IN  GT_U32      srcNinthOctetToChange,
    IN  GT_U32      srcTenthOctetToChange,
    IN  GT_U32      srcElevenOctetToChange,
    IN  GT_U32      srcTwelveOctetToChange,
    IN  GT_U32      srcThirteenOctetToChange,
    IN  GT_U32      srcFourteenOctetToChange,
    IN  GT_U32      srcFifteenOctetToChange,
    IN  GT_U32      srcSixteenOctetToChange,
    OUT GT_U32      *numOfRetrievedPrefixesPtr
);

/**
* @internal cpssDxChIpLpmDbgHwOctetPerBlockPrint function
* @endinternal
*
* @brief   Print Octet per Block debug information for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] lpmDbId                  - The LPM DB id
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - if can't find the lpm DB
*/
GT_STATUS cpssDxChIpLpmDbgHwOctetPerBlockPrint
(
    IN GT_U32                           lpmDbId
);

/**
* @internal cpssDxChIpLpmDbgHwOctetPerProtocolPrint function
* @endinternal
*
* @brief   Print Octet per Protocol debug information for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] lpmDbId                  - The LPM DB id
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - if can't find the lpm DB
*/
GT_STATUS cpssDxChIpLpmDbgHwOctetPerProtocolPrint
(
    IN GT_U32                           lpmDbId
);

/**
* @internal cpssDxChIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters function
* @endinternal
*
* @brief   Print Octet per Protocol LPM lines debug information for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] lpmDbId                  - The LPM DB id
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - if can't find the lpm DB
*/
GT_STATUS cpssDxChIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters
(
    IN GT_U32                           lpmDbId
);

/**
* @internal cpssDxChIpLpmDbgHwOctetPerProtocolPrintLpmMemoryAndCounters function
* @endinternal
*
* @brief   Print Octet per Protocol LPM lines and Banks memory debug information for a
*         specific LPM DB
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] lpmDbId                  - The LPM DB id
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - if can't find the lpm DB
*/
GT_STATUS cpssDxChIpLpmDbgHwOctetPerProtocolPrintLpmMemoryAndCounters
(
    IN GT_U32                           lpmDbId
);

/**
* @internal prvCpssDxChLpmDbgHwOctetPerProtocolPrintLpmMemoryAndCounters function
* @endinternal
*
* @brief   Print Octet per Protocol LPM lines and Banks memory debug information for a
*         specific LPM DB
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*                                       None.
*/
GT_VOID prvCpssDxChLpmDbgHwOctetPerProtocolPrintLpmMemoryAndCounters
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC      *lpmDbPtr
);

/**
* @internal cpssDxChIpLpmDbgIpv4McPrefixDelManyByOctet function
* @endinternal
*
* @brief   This function tries to delete many sequential IPv4 Multicast prefixes according
*         to octet order selection and returns the number of prefixes successfully deleted.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] grpStartIpAddr           - the first group address to deleted
* @param[in] srcStartIpAddr           - the first source address to deleted
* @param[in] numOfPrefixesToDel       - the number of prefixes to delete
* @param[in] changeGrpAddr            - GT_TRUE: change the group address
*                                      GT_FALSE: don't change the group address
* @param[in] grpFirstOctetToChange    - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpSecondOctetToChange   second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpThirdOctetToChange    - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpFourthOctetToChange   fourth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] changeSrcAddr            - GT_TRUE: change the source address
*                                      GT_FALSE: don't change the source address
* @param[in] srcFirstOctetToChange    - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcSecondOctetToChange   second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcThirdOctetToChange    - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcFourthOctetToChange   fourth octet to change in the prefix address.
*                                      value of the octet can be up to 255
*
* @param[out] grpLastIpAddrDeletedPtr  - points to the last group prefix successfully
*                                      deleted (NULL to ignore)
* @param[out] srcLastIpAddrDeletedPtr  - points to the last source prefix successfully
*                                      deleted (NULL to ignore)
* @param[out] numOfPrefixesDeletedPtr  - points to the nubmer of prefixes that were
*                                      successfully deleted (NULL to ignore)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to check the number of the prefixes
*       that can be deleted. All the prefixes are deleted with exact match (prefix
*       length 32).
*
*/
GT_STATUS cpssDxChIpLpmDbgIpv4McPrefixDelManyByOctet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPADDR   grpStartIpAddr,
    IN  GT_IPADDR   srcStartIpAddr,
    IN  GT_U32      numOfPrefixesToDel,
    IN  GT_BOOL     changeGrpAddr,
    IN  GT_U32      grpFirstOctetToChange,
    IN  GT_U32      grpSecondOctetToChange,
    IN  GT_U32      grpThirdOctetToChange,
    IN  GT_U32      grpFourthOctetToChange,
    IN  GT_BOOL     changeSrcAddr,
    IN  GT_U32      srcFirstOctetToChange,
    IN  GT_U32      srcSecondOctetToChange,
    IN  GT_U32      srcThirdOctetToChange,
    IN  GT_U32      srcFourthOctetToChange,
    OUT GT_IPADDR   *grpLastIpAddrDeletedPtr,
    OUT GT_IPADDR   *srcLastIpAddrDeletedPtr,
    OUT GT_U32      *numOfPrefixesDeletedPtr
);

/**
* @internal cpssDxChIpLpmDbgIpv6McPrefixDelManyByOctet function
* @endinternal
*
* @brief   This function tries to delete many sequential IPv6 Multicast prefixes according
*         to octet order selection and returns the number of prefixes successfully deleted.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] grpStartIpAddr           - the first group address to delete
* @param[in] srcStartIpAddr           - the first source address to delete
* @param[in] numOfPrefixesToDel       - the number of prefixes to delete
* @param[in] changeGrpAddr            - GT_TRUE: change the group address
*                                      GT_FALSE: don't change the group address
* @param[in] grpFirstOctetToChange    - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpSecondOctetToChange   second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpThirdOctetToChange    - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpFourthOctetToChange   fourth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpFifthOctetToChange    - fifth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpSixthOctetToChange    - sixth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpSeventhOctetToChange  - seventh octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpEighthOctetToChange   eighth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpNinthOctetToChange    - ninth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpTenthOctetToChange    - tenth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpElevenOctetToChange   eleven octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpTwelveOctetToChange   twelve octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpThirteenOctetToChange - thirteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpFourteenOctetToChange - fourteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpFifteenOctetToChange  - fifteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] grpSixteenOctetToChange  - sixteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] changeSrcAddr            - GT_TRUE: change the source address
*                                      GT_FALSE: don't change the source address
* @param[in] srcFirstOctetToChange    - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcSecondOctetToChange   second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcThirdOctetToChange    - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcFourthOctetToChange   fourth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcFifthOctetToChange    - fifth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcSixthOctetToChange    - sixth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcSeventhOctetToChange  - seventh octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcEighthOctetToChange   eighth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcNinthOctetToChange    - ninth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcTenthOctetToChange    - tenth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcElevenOctetToChange   eleven octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcTwelveOctetToChange   twelve octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcThirteenOctetToChange - thirteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcFourteenOctetToChange - fourteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcFifteenOctetToChange  - fifteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] srcSixteenOctetToChange  - sixteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
*
* @param[out] grpLastIpAddrDeletedPtr  - points to the last group prefix successfully
*                                      deleted (NULL to ignore)
* @param[out] srcLastIpAddrDeletedPtr  - points to the last source prefix successfully
*                                      deleted (NULL to ignore)
* @param[out] numOfPrefixesDeletedPtr  - points to the nubmer of prefixes that were
*                                      successfully deleted (NULL to ignore)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to check the number of the prefixes
*       that can be deleted. All the prefixes are deleted with exact match (prefix
*       length 128).
*
*/
GT_STATUS cpssDxChIpLpmDbgIpv6McPrefixDelManyByOctet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPV6ADDR grpStartIpAddr,
    IN  GT_IPV6ADDR srcStartIpAddr,
    IN  GT_U32      numOfPrefixesToDel,
    IN  GT_BOOL     changeGrpAddr,
    IN  GT_U32      grpFirstOctetToChange,
    IN  GT_U32      grpSecondOctetToChange,
    IN  GT_U32      grpThirdOctetToChange,
    IN  GT_U32      grpFourthOctetToChange,
    IN  GT_U32      grpFifthOctetToChange,
    IN  GT_U32      grpSixthOctetToChange,
    IN  GT_U32      grpSeventhOctetToChange,
    IN  GT_U32      grpEighthOctetToChange,
    IN  GT_U32      grpNinthOctetToChange,
    IN  GT_U32      grpTenthOctetToChange,
    IN  GT_U32      grpElevenOctetToChange,
    IN  GT_U32      grpTwelveOctetToChange,
    IN  GT_U32      grpThirteenOctetToChange,
    IN  GT_U32      grpFourteenOctetToChange,
    IN  GT_U32      grpFifteenOctetToChange,
    IN  GT_U32      grpSixteenOctetToChange,
    IN  GT_BOOL     changeSrcAddr,
    IN  GT_U32      srcFirstOctetToChange,
    IN  GT_U32      srcSecondOctetToChange,
    IN  GT_U32      srcThirdOctetToChange,
    IN  GT_U32      srcFourthOctetToChange,
    IN  GT_U32      srcFifthOctetToChange,
    IN  GT_U32      srcSixthOctetToChange,
    IN  GT_U32      srcSeventhOctetToChange,
    IN  GT_U32      srcEighthOctetToChange,
    IN  GT_U32      srcNinthOctetToChange,
    IN  GT_U32      srcTenthOctetToChange,
    IN  GT_U32      srcElevenOctetToChange,
    IN  GT_U32      srcTwelveOctetToChange,
    IN  GT_U32      srcThirteenOctetToChange,
    IN  GT_U32      srcFourteenOctetToChange,
    IN  GT_U32      srcFifteenOctetToChange,
    IN  GT_U32      srcSixteenOctetToChange,
    OUT GT_IPV6ADDR *grpLastIpAddrDeletedPtr,
    OUT GT_IPV6ADDR *srcLastIpAddrDeletedPtr,
    OUT GT_U32      *numOfPrefixesDeletedPtr
);

/**
* @internal cpssDxChIpLpmDbgIpv4SetStep function
* @endinternal
*
* @brief   This function set the step size for each of the 4 IPv4 octets, and the prefix length.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] stepArr[4]               - step size array for IPv4 UC Destination or IPv4 MC Group
* @param[in] prefixLength             - prefix length for IPv4 UC Destination or IPv4 MC Group
* @param[in] srcStepArr[4]            - step size array for IPv4 source
* @param[in] srcPrefixLen             - prefix length for IPv4 source
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
*/
GT_STATUS cpssDxChIpLpmDbgIpv4SetStep
(
    IN GT_U8  stepArr[4],
    IN GT_U32 prefixLength,
    IN GT_U8  srcStepArr[4],
    IN GT_U32 srcPrefixLen
);

/**
* @internal cpssDxChIpLpmDbgIpv6SetStep function
* @endinternal
*
* @brief   This function set the step size for each of the 16 IPv6 octets, and the prefix length.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] stepArr[16]              - step size array for IPv6 UC Destination or IPv6 MC Group
* @param[in] prefixLength             - prefix length for IPv6 UC Destination or IPv6 MC Group
* @param[in] srcStepArr[16]           - step size for IPv6 source
* @param[in] srcPrefixLen             - prefix length for IPv6 source
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
*/
GT_STATUS cpssDxChIpLpmDbgIpv6SetStep
(
    IN GT_U8  stepArr[16],
    IN GT_U32 prefixLength,
    IN GT_U8  srcStepArr[16],
    IN GT_U32 srcPrefixLen
);

/**
* @internal cpssDxChIpLpmDbgFcoePrefixAddMany function
* @endinternal
*
* @brief   This debug function tries to add many sequential FCoE prefixes and
*         returns the number of prefixes successfully added.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] startFcoeAddr            - the first address to add
* @param[in] routeEntryBaseMemAddr    - base memory Address (offset) of the route entry
* @param[in] numOfPrefixesToAdd       - the number of prefixes to add
* @param[in] stepArr[3]               - array of steps for each octet
* @param[in] prefixLength             - prefix length
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note This debug function is intended to check the number of the prefixes
*       that can be added. All the prefixes are added with exact match.
*       The route entry is not written.
*
*/
GT_STATUS cpssDxChIpLpmDbgFcoePrefixAddMany
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_FCID     startFcoeAddr,
    IN  GT_U32      routeEntryBaseMemAddr,
    IN  GT_U32      numOfPrefixesToAdd,
    IN  GT_U8       stepArr[3],
    IN  GT_U32      prefixLength,
    OUT GT_FCID     *lastFcoeAddrAddedPtr,
    OUT GT_U32      *numOfPrefixesAddedPtr
);

/**
* @internal cpssDxChIpLpmDbgFcoePrefixAddManyRandom function
* @endinternal
*
* @brief   This function tries to add many random FCoE prefixes and
*         returns the number of prefixes successfully added.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] startFcoeAddr            - the first address to add
* @param[in] routeEntryBaseMemAddr    - base memory Address (offset) of the route entry
* @param[in] numOfPrefixesToAdd       - the number of prefixes to add
* @param[in] isWholeFCoERandom        - GT_TRUE: all IP octets calculated by cpssOsRand
*                                      GT_FALSE: only 2 LSB octets calculated by cpssOsRand
* @param[in] prefixLength             - prefix length
*
* @param[out] numOfPrefixesAddedPtr    - points to the nubmer of prefixes that were
*                                      successfully added (NULL to ignore)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note This debug function is intended to check the number of the prefixes
*       that can be added. All the prefixes are added with exact match.
*       The route entry is not written.
*
*/
GT_STATUS cpssDxChIpLpmDbgFcoePrefixAddManyRandom
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_FCID     startFcoeAddr,
    IN  GT_U32      routeEntryBaseMemAddr,
    IN  GT_U32      numOfPrefixesToAdd,
    IN  GT_BOOL     isWholeFCoERandom,
    IN  GT_U32      prefixLength,
    OUT GT_U32      *numOfPrefixesAddedPtr
);

/**
* @internal cpssDxChIpLpmDbgFcoePrefixAddManyByOctet function
* @endinternal
*
* @brief   This function tries to add many sequential FCoE prefixes according
*         to octet order selection and returns the number of prefixes successfully added.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] startFcoeAddr            - the first address to add
* @param[in] routeEntryBaseMemAddr    - base memory Address (offset) of the route entry
* @param[in] numOfPrefixesToAdd       - the number of prefixes to add
* @param[in] firstOctetToChange       - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] secondOctetToChange      - second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] thirdOctetToChange       - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] stepArr[3]               - array of steps for each octet
* @param[in] prefixLength             - prefix length
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note This debug function is intended to check the number of the prefixes
*       that can be added. All the prefixes are added with exact match.
*       The route entry is not written.
*
*/
GT_STATUS cpssDxChIpLpmDbgFcoePrefixAddManyByOctet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_FCID     startFcoeAddr,
    IN  GT_U32      routeEntryBaseMemAddr,
    IN  GT_U32      numOfPrefixesToAdd,
    IN  GT_U32      firstOctetToChange,
    IN  GT_U32      secondOctetToChange,
    IN  GT_U32      thirdOctetToChange,
    IN  GT_U8       stepArr[3],
    IN  GT_U32      prefixLength,
    OUT GT_FCID     *lastFcoeAddrAddedPtr,
    OUT GT_U32      *numOfPrefixesAddedPtr
);

/**
* @internal cpssDxChIpLpmDbgFcoePrefixDelManyByOctet function
* @endinternal
*
* @brief   This function tries to delete many sequential FCoE prefixes according
*         to octet order selection and returns the number of prefixes successfully added.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] startFcoeAddr            - the first address to add
* @param[in] numOfPrefixesToDel       - the number of prefixes to add
* @param[in] firstOctetToChange       - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] secondOctetToChange      - second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] thirdOctetToChange       - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] stepArr[3]               - array of steps for each octet
* @param[in] prefixLength             - prefix length
*
* @param[out] lastFcoeAddrDeletedPtr   - points to the last prefix successfully
*                                      added (NULL to ignore)
* @param[out] numOfPrefixesDeletedPtr  - points to the nubmer of prefixes that were
*                                      successfully added (NULL to ignore)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note This debug function is intended to check the number of the prefixes
*       that can be added. All the prefixes are added with exact match.
*       The route entry is not written.
*
*/
GT_STATUS cpssDxChIpLpmDbgFcoePrefixDelManyByOctet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_FCID     startFcoeAddr,
    IN  GT_U32      numOfPrefixesToDel,
    IN  GT_U32      firstOctetToChange,
    IN  GT_U32      secondOctetToChange,
    IN  GT_U32      thirdOctetToChange,
    IN  GT_U8       stepArr[3],
    IN  GT_U32      prefixLength,
    OUT GT_FCID     *lastFcoeAddrDeletedPtr,
    OUT GT_U32      *numOfPrefixesDeletedPtr
);

/**
* @internal cpssDxChIpLpmDbgFcoePrefixGetManyByOctet function
* @endinternal
*
* @brief   This function tries to get many sequential FCoE prefixes (that were
*         added by cpssDxChIpLpmDbgFcoePrefixAddManyByOctet) and returns the number of
*         successfully retrieved prefixes.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
*                                      startIpAddr           - the first address to add
* @param[in] numOfPrefixesToGet       - the number of prefixes to get
* @param[in] firstOctetToChange       - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] secondOctetToChange      - second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] thirdOctetToChange       - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] stepArr[3]               - array of steps for each octet
* @param[in] prefixLength             - prefix length
*
* @param[out] numOfRetrievedPrefixesPtr - points to the nubmer of prefixes that were
*                                      successfully retrieved (NULL to ignore)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note This debug function is intended to check the number of the prefixes
*       that were be added.
*
*/
GT_STATUS cpssDxChIpLpmDbgFcoePrefixGetManyByOctet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_FCID     startFcoeAddr,
    IN  GT_U32      numOfPrefixesToGet,
    IN  GT_U32      firstOctetToChange,
    IN  GT_U32      secondOctetToChange,
    IN  GT_U32      thirdOctetToChange,
    IN  GT_U8       stepArr[3],
    IN  GT_U32      prefixLength,
    OUT GT_U32      *numOfRetrievedPrefixesPtr
);

/**
* @internal cpssDxChIpLpmDbgDefragEnableSet function
* @endinternal
*
* @brief   This function set the defrag flag.
*         IP wrappers that add many prefixes by octet, will use this flag
*         as parameter to the CPSS APIs
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] defragEnable             - GT_TRUE: enable defragmentation of LPM
*                                      GT_FALSE: disable defragmentation of LPM
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
*/
GT_STATUS cpssDxChIpLpmDbgDefragEnableSet
(
    IN GT_BOOL   defragEnable
);

/**
* @internal cpssDxChIpLpmDbgDefragEnableGet function
* @endinternal
*
* @brief   This function return the defrag flag.
*         IP wrappers that add many prefixes by octet, will use this flag
*         as parameter to the CPSS APIs
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2;
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
*
* @param[out] defragEnablePtr          - (pointer to)
*                                      GT_TRUE: enable defragmentation of LPM
*                                      GT_FALSE: disable defragmentation of LPM
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
*/
GT_STATUS cpssDxChIpLpmDbgDefragEnableGet
(
    OUT GT_BOOL   *defragEnablePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChIpLpmDbgh */


