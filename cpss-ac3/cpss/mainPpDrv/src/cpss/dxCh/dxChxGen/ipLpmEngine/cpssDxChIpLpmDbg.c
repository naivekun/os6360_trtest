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
* @file cpssDxChIpLpmDbg.c
*
* @brief IP LPM debug functions
*
* @version   54
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/ipLpmEngine/cpssDxChIpLpm.h>
#include <cpss/dxCh/dxChxGen/ipLpmEngine/private/cpssDxChPrvIpLpm.h>
#include <cpss/dxCh/dxChxGen/ipLpmEngine/private/prvCpssDxChIpLpmEngineLog.h>
#include <cpss/dxCh/dxChxGen/private/lpm/tcam/prvCpssDxChLpmTcamDbg.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamDbg.h>
#include <cpss/dxCh/dxChxGen/private/routerTunnelTermTcam/prvCpssDxChRouterTunnelTermTcam.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/private/lpm/prvCpssDxChLpmUtils.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/ipLpmEngine/private/cpssDxChIpLpmDbg.h>

/* lpm db skip list ptr */
extern GT_VOID *prvCpssDxChIplpmDbSL;

extern GT_VOID * prvCpssSlSearch
(
    IN GT_VOID        *ctrlPtr,
    IN GT_VOID        *dataPtr
);

#define ipv6_is_bigger_or_equal_MAC( IPv6_addr1_PTR , IPv6_addr2_PTR)                 \
    (cpssOsMemCmp(IPv6_addr1_PTR, IPv6_addr2_PTR, 16) >= 0 )

#define ipv6_is_smaller_or_equal_MAC( IPv6_addr1_PTR , IPv6_addr2_PTR)                 \
    (cpssOsMemCmp(IPv6_addr1_PTR, IPv6_addr2_PTR, 16) <= 0 )

typedef union
{
    GT_U32 Word;
    GT_U8  Bytes[4];
} CPSS_DXCH_IP_CONVERT_UNT;

GT_U8 IPv4Step[4]  = {1,1,1,1};
GT_U8 IPv6Step[16]  = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
GT_U8 srcIPv4Step[4]  = {1,1,1,1};
GT_U8 srcIPv6Step[16]  = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
GT_U32 IPv4PrefixLength = 32;
GT_U32 IPv6PrefixLength = 128;
GT_U32 srcIPv4PrefixLength = 32;
GT_U32 srcIPv6PrefixLength = 128;

/* flag to be used when we want to change the way many prefixes are
   added to the device
   GT_TRUE  : defragmentation is done - using shrink and merge mechanism
   GT_FALSE : defragmentation is NOT done */
GT_BOOL defragmentationEnableFlag = GT_TRUE;


/**
* @internal prvCpssDxChIpLpmDbgIpSwap function
* @endinternal
*
* @brief   convert 32-bits host order to network order
*
* @param[in] val                      - value to convert
*                                       converted value
*/
static GT_U32 prvCpssDxChIpLpmDbgIpSwap
(
    IN  GT_U32  val
)
{
    CPSS_DXCH_IP_CONVERT_UNT v1,v2;

    v1.Word = val;

    v2.Bytes[0] = v1.Bytes[3];
    v2.Bytes[1] = v1.Bytes[2];
    v2.Bytes[2] = v1.Bytes[1];
    v2.Bytes[3] = v1.Bytes[0];

    return v2.Word;

}

/**
* @internal prvCpssDxChIpLpmDbgIsAddrInRange function
* @endinternal
*
* @brief   Check if an address is in a range described by the first address
*         and last address.
* @param[in] currIpAddrPtr            - (pointer to) the address to test.
* @param[in] startIpAddrPtr           - (pointer to) first address in range.
* @param[in] lastIpAddrPtr            - (pointer to) last address in range.
*
* @retval GT_TRUE                  - address is in range.
* @retval GT_FALSE                 - address is not in range.
*/
static GT_BOOL prvCpssDxChIpLpmDbgIsAddrInRange
(
    IN  GT_IPV6ADDR *currIpAddrPtr,
    IN  GT_IPV6ADDR *startIpAddrPtr,
    IN  GT_IPV6ADDR *lastIpAddrPtr
)
{
    return (ipv6_is_bigger_or_equal_MAC(currIpAddrPtr, startIpAddrPtr) &&
            ipv6_is_smaller_or_equal_MAC(currIpAddrPtr, lastIpAddrPtr));
}

/**
* @internal outputIPAddress function
* @endinternal
*
* @brief   Print Ip address
*
* @param[in] ipAddrPtr                - ip address
* @param[in] bytes2Print              - amount of printed bytes
* @param[in] isIpv4                   - Ipv4 or Ipv6 address
*/
static GT_VOID outputIPAddress
(
    IN GT_U8 *ipAddrPtr,
    IN GT_U32 bytes2Print,
    IN GT_BOOL isIpv4
)
{
    while(bytes2Print > 1)
    {
        if (GT_TRUE == isIpv4)
        {
            cpssOsPrintf("%03d.%03d.", *ipAddrPtr, *(ipAddrPtr+1));
        }
        else
        {
            cpssOsPrintf("%02X%02X:", *ipAddrPtr, *(ipAddrPtr+1));
        }

        ipAddrPtr+=2;
        bytes2Print-=2;
    }

    if (bytes2Print > 0)
    {
        if (GT_TRUE == isIpv4)
        {
            cpssOsPrintf("%03d", *ipAddrPtr);
        }
        else
        {
            cpssOsPrintf("%02X", *ipAddrPtr);
        }
    }
}

/**
* @internal prvCpssDxChIpLpmDbgConvertRouteType function
* @endinternal
*
* @brief   Convert CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E to
*         CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E for RAM based LPM whenever input
*         type is CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E and numOfPaths is 0
* @param[in] lpmDBId                  - LPM DB id
* @param[in,out] ipLttEntryPtr            - (pointer to) the original IP LTT entry
* @param[in,out] ipLttEntryPtr            - (pointer to) the changed IP LTT entry
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - if can't find the lpm DB
* @retval GT_BAD_PTR               - on null pointer or if shadowType is uninitialized
*/
static GT_STATUS prvCpssDxChIpLpmDbgConvertRouteType
(
    IN    GT_U32                          lpmDBId,
    INOUT CPSS_DXCH_IP_LTT_ENTRY_STC      *ipLttEntryPtr
)
{
    GT_STATUS                                       status;
    CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT                shadowType;
    CPSS_IP_PROTOCOL_STACK_ENT                      protocolStack;
    CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT              memoryCfg;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC    indexesRange;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC    tcamLpmManagerCapcityCfg;
    GT_PTR                                          tcamManagerHandler;

    memoryCfg.tcamDbCfg.indexesRangePtr = &indexesRange;
    memoryCfg.tcamDbCfg.tcamLpmManagerCapcityCfgPtr = &tcamLpmManagerCapcityCfg;
    memoryCfg.tcamDbCfg.tcamManagerHandlerPtr = &tcamManagerHandler;
    status = cpssDxChIpLpmDBConfigGet(lpmDBId, &shadowType, &protocolStack, &memoryCfg);
    if (status != GT_OK)
    {
        return status;
    }

    if (ipLttEntryPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }

    if ((shadowType == CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E) &&
        (ipLttEntryPtr->routeType == CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E) &&
        (ipLttEntryPtr->numOfPaths == 0))
    {
        ipLttEntryPtr->routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
    }

    if ((shadowType == CPSS_DXCH_IP_LPM_RAM_SIP6_SHADOW_E) &&
        (ipLttEntryPtr->routeType == CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_MULTIPATH_E) &&
        (ipLttEntryPtr->numOfPaths == 0))
    {
        ipLttEntryPtr->routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
    }


    return GT_OK;
}

/**
* @internal internal_cpssDxChIpLpmDbgRouteTcamDump function
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
static GT_STATUS internal_cpssDxChIpLpmDbgRouteTcamDump
(
    IN GT_BOOL dump
)
{
    return prvCpssDxChLpmTcamDbgDumpRouteTcam(dump);
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpLpmDbgRouteTcamDump);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, dump));

    rc = internal_cpssDxChIpLpmDbgRouteTcamDump(dump);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, dump));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpLpmDbgPatTriePrint function
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
static GT_STATUS internal_cpssDxChIpLpmDbgPatTriePrint
(
    IN GT_U32                     lpmDbId,
    IN GT_U32                     vrId,
    IN CPSS_IP_PROTOCOL_STACK_ENT protocol,
    IN CPSS_UNICAST_MULTICAST_ENT prefixType
)
{
    PRV_CPSS_DXCH_LPM_SHADOW_STC                    *lpmDbPtr,tmpLpmDb;
    PRV_CPSS_DXCH_LPM_HW_ENT                        lpmHw;

    tmpLpmDb.lpmDbId = lpmDbId;
    lpmDbPtr = prvCpssSlSearch(prvCpssDxChIplpmDbSL,&tmpLpmDb);
    if (lpmDbPtr == NULL)
    {
        /* can't find the lpm DB */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    lpmHw = prvCpssDxChLpmGetHwType(lpmDbPtr->shadowType);
    switch (lpmHw)
    {
        case PRV_CPSS_DXCH_LPM_HW_TCAM_E:
            return prvCpssDxChLpmTcamDbgPatTriePrint((PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC*)(lpmDbPtr->shadow), vrId,
                                                     protocol, prefixType);

        case PRV_CPSS_DXCH_LPM_HW_RAM_E:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpLpmDbgPatTriePrint);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, lpmDbId, vrId, protocol, prefixType));

    rc = internal_cpssDxChIpLpmDbgPatTriePrint(lpmDbId, vrId, protocol, prefixType);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, lpmDbId, vrId, protocol, prefixType));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpLpmDbgPatTrieValidityCheck function
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
static GT_STATUS internal_cpssDxChIpLpmDbgPatTrieValidityCheck
(
  IN GT_U32                     lpmDBId,
  IN GT_U32                     vrId,
  IN CPSS_IP_PROTOCOL_STACK_ENT protocol,
  IN CPSS_UNICAST_MULTICAST_ENT prefixType
)
{
    PRV_CPSS_DXCH_LPM_SHADOW_STC        *lpmDbPtr,tmpLpmDb;
    PRV_CPSS_DXCH_LPM_HW_ENT            lpmHw;

    tmpLpmDb.lpmDbId = lpmDBId;
    lpmDbPtr = prvCpssSlSearch(prvCpssDxChIplpmDbSL,&tmpLpmDb);
    if (lpmDbPtr == NULL)
    {
        /* can't find the lpm DB */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    lpmHw = prvCpssDxChLpmGetHwType(lpmDbPtr->shadowType);
    switch (lpmHw)
    {
        case PRV_CPSS_DXCH_LPM_HW_TCAM_E:
            return prvCpssDxChLpmTcamDbgPatTrieValidityCheck((PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC*)(lpmDbPtr->shadow),
                                                             vrId, protocol,
                                                             prefixType);

        case PRV_CPSS_DXCH_LPM_HW_RAM_E:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpLpmDbgPatTrieValidityCheck);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, lpmDBId, vrId, protocol, prefixType));

    rc = internal_cpssDxChIpLpmDbgPatTrieValidityCheck(lpmDBId, vrId, protocol, prefixType);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, lpmDBId, vrId, protocol, prefixType));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpLpmDbgDump function
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
static GT_STATUS internal_cpssDxChIpLpmDbgDump
(
    IN GT_U8                            devNum,
    IN GT_U32                           vrId,
    IN CPSS_IP_PROTOCOL_STACK_ENT       protocol,
    IN CPSS_UNICAST_MULTICAST_ENT       prefixType
)
{
    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT    lpmProtocol;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
                                          CPSS_CH1_E | CPSS_CH1_DIAMOND_E |
                                          CPSS_CH2_E | CPSS_CH3_E |
                                          CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E |
                                          CPSS_XCAT2_E | CPSS_LION2_E);
    switch (protocol)
    {
        case CPSS_IP_PROTOCOL_IPV4_E:
            lpmProtocol = PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E;
            break;

        case CPSS_IP_PROTOCOL_IPV6_E:
            lpmProtocol = PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E;
            break;

        case CPSS_IP_PROTOCOL_FCOE_E:
            lpmProtocol = PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if ((prefixType != CPSS_UNICAST_E) && (prefixType != CPSS_MULTICAST_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return prvCpssDxChLpmRamDbgDump(devNum, vrId, lpmProtocol, prefixType);
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpLpmDbgDump);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vrId, protocol, prefixType));

    rc = internal_cpssDxChIpLpmDbgDump(devNum, vrId, protocol, prefixType);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vrId, protocol, prefixType));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpLpmDbgHwMemPrint function
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
static GT_STATUS internal_cpssDxChIpLpmDbgHwMemPrint
(
    IN GT_U32                           lpmDbId
)
{
    PRV_CPSS_DXCH_LPM_SHADOW_STC        *lpmDbPtr,tmpLpmDb;
    PRV_CPSS_DXCH_LPM_HW_ENT            lpmHw;

    tmpLpmDb.lpmDbId = lpmDbId;
    lpmDbPtr = prvCpssSlSearch(prvCpssDxChIplpmDbSL,&tmpLpmDb);
    if (lpmDbPtr == NULL)
    {
        /* can't find the lpm DB */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    lpmHw = prvCpssDxChLpmGetHwType(lpmDbPtr->shadowType);
    switch (lpmHw)
    {
        case PRV_CPSS_DXCH_LPM_HW_TCAM_E:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);

        case PRV_CPSS_DXCH_LPM_HW_RAM_E:
            return prvCpssDxChLpmRamDbgDbHwMemPrint((PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC*)(lpmDbPtr->shadow));

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpLpmDbgHwMemPrint);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, lpmDbId));

    rc = internal_cpssDxChIpLpmDbgHwMemPrint(lpmDbId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, lpmDbId));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpLpmDbgHwValidation function
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
static GT_STATUS internal_cpssDxChIpLpmDbgHwValidation
(
    IN GT_U8                            devNum,
    IN GT_U32                           vrId,
    IN CPSS_IP_PROTOCOL_STACK_ENT       protocol,
    IN CPSS_UNICAST_MULTICAST_ENT       prefixType
)
{
    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT    lpmProtocol;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
                                          CPSS_CH1_E | CPSS_CH1_DIAMOND_E |
                                          CPSS_CH2_E | CPSS_CH3_E |
                                          CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E |
                                          CPSS_XCAT2_E | CPSS_LION2_E);
    switch (protocol)
    {
        case CPSS_IP_PROTOCOL_IPV4_E:
            lpmProtocol = PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E;
            break;

        case CPSS_IP_PROTOCOL_IPV6_E:
            lpmProtocol = PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E;
            break;

        case CPSS_IP_PROTOCOL_FCOE_E:
            lpmProtocol = PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if ((prefixType != CPSS_UNICAST_E) && (prefixType != CPSS_MULTICAST_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return prvCpssDxChLpmRamDbgHwValidation(devNum, vrId, lpmProtocol, prefixType);
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpLpmDbgHwValidation);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vrId, protocol, prefixType));

    rc = internal_cpssDxChIpLpmDbgHwValidation(devNum, vrId, protocol, prefixType);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vrId, protocol, prefixType));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpLpmDbgShadowValidityCheck function
* @endinternal
*
* @brief   Validate the LPM in shadow
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
static GT_STATUS internal_cpssDxChIpLpmDbgShadowValidityCheck
(
    IN GT_U32                           lpmDbId,
    IN GT_U32                           vrId,
    IN CPSS_IP_PROTOCOL_STACK_ENT       protocol,
    IN CPSS_UNICAST_MULTICAST_ENT       prefixType,
    IN GT_BOOL                          returnOnFailure
)
{
    PRV_CPSS_DXCH_LPM_SHADOW_STC        *lpmDbPtr,tmpLpmDb;
    PRV_CPSS_DXCH_LPM_HW_ENT            lpmHw;
    PRV_CPSS_DXCH_LPM_PROTOCOL_BMP      protocolBitmap;

    PRV_CPSS_DXCH_IP_CHECK_PROTOCOL_STACK_MAC(protocol);

    tmpLpmDb.lpmDbId = lpmDbId;
    lpmDbPtr = prvCpssSlSearch(prvCpssDxChIplpmDbSL,&tmpLpmDb);
    if (lpmDbPtr == NULL)
    {
        /* can't find the lpm DB */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    lpmHw = prvCpssDxChLpmGetHwType(lpmDbPtr->shadowType);
    switch (lpmHw)
    {
        case PRV_CPSS_DXCH_LPM_HW_TCAM_E:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);

        case PRV_CPSS_DXCH_LPM_HW_RAM_E:
            prvCpssDxChLpmConvertIpProtocolStackToProtocolBitmap(protocol, &protocolBitmap);
            if ((protocol == CPSS_IP_PROTOCOL_FCOE_E)&&
                (lpmDbPtr->shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP5_SHADOW_E) &&
                (prefixType==CPSS_UNICAST_E))
            {
                PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_SET_MAC(protocolBitmap);
            }

            return prvCpssDxChLpmRamDbgShadowValidityCheck((PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC*)(lpmDbPtr->shadow),
                                                           vrId, protocolBitmap, prefixType, returnOnFailure);

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
}

/**
* @internal cpssDxChIpLpmDbgShadowValidityCheck function
* @endinternal
*
* @brief   Validate the LPM in shadow
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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpLpmDbgShadowValidityCheck);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, lpmDbId, vrId, protocol, prefixType, returnOnFailure));

    rc = internal_cpssDxChIpLpmDbgShadowValidityCheck(lpmDbId, vrId, protocol, prefixType, returnOnFailure);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, lpmDbId, vrId, protocol, prefixType, returnOnFailure));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpLpmDbgHwShadowSyncValidityCheck function
* @endinternal
*
* @brief   This function validates synchronization between the shadow and HW of the LPM
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
static GT_STATUS internal_cpssDxChIpLpmDbgHwShadowSyncValidityCheck
(
    IN GT_U8                            devNum,
    IN GT_U32                           lpmDbId,
    IN GT_U32                           vrId,
    IN CPSS_IP_PROTOCOL_STACK_ENT       protocol,
    IN CPSS_UNICAST_MULTICAST_ENT       prefixType,
    IN GT_BOOL                          returnOnFailure
)
{
    PRV_CPSS_DXCH_LPM_SHADOW_STC        *lpmDbPtr,tmpLpmDb;
    PRV_CPSS_DXCH_LPM_HW_ENT            lpmHw;
    PRV_CPSS_DXCH_LPM_PROTOCOL_BMP      protocolBitmap;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
                                          CPSS_CH1_E | CPSS_CH1_DIAMOND_E |
                                          CPSS_CH2_E | CPSS_CH3_E |
                                          CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E |
                                          CPSS_XCAT2_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_IP_CHECK_PROTOCOL_STACK_MAC(protocol);

    /* run shadow and HW synchronization check, assuming HW validation and shadow validation passed ok */
    tmpLpmDb.lpmDbId = lpmDbId;
    lpmDbPtr = prvCpssSlSearch(prvCpssDxChIplpmDbSL,&tmpLpmDb);
    if (lpmDbPtr == NULL)
    {
        /* can't find the lpm DB */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    lpmHw = prvCpssDxChLpmGetHwType(lpmDbPtr->shadowType);
    switch (lpmHw)
    {
        case PRV_CPSS_DXCH_LPM_HW_TCAM_E:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);

        case PRV_CPSS_DXCH_LPM_HW_RAM_E:
            prvCpssDxChLpmConvertIpProtocolStackToProtocolBitmap(protocol, &protocolBitmap);
            if ((protocol==CPSS_IP_PROTOCOL_FCOE_E)&&
                (lpmDbPtr->shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP5_SHADOW_E)&&
                (prefixType==CPSS_UNICAST_E))
            {
                PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_SET_MAC(protocolBitmap);
            }
            return prvCpssDxChLpmRamDbgHwShadowSyncValidityCheck(devNum,
                                                                 (PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC*)(lpmDbPtr->shadow),
                                                                 vrId, protocolBitmap, prefixType, returnOnFailure);

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
}

/**
* @internal cpssDxChIpLpmDbgHwShadowSyncValidityCheck function
* @endinternal
*
* @brief   This function validates synchronization between the shadow and HW of the LPM
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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpLpmDbgHwShadowSyncValidityCheck);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vrId, protocol, prefixType, returnOnFailure));

    rc = internal_cpssDxChIpLpmDbgHwShadowSyncValidityCheck(devNum, lpmDbId, vrId,
                                                            protocol, prefixType,
                                                            returnOnFailure);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, lpmDbId, vrId, protocol, prefixType, returnOnFailure));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpLpmDbgHwShadowValidityAndSyncCheck function
* @endinternal
*
* @brief   This function Validate the LPM in shadow, Validate the LPM in HW and
*         check synchronization between the shadow and HW of the LPM
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
static GT_STATUS internal_cpssDxChIpLpmDbgHwShadowValidityAndSyncCheck
(
    IN GT_U8                            devNum,
    IN GT_U32                           lpmDbId,
    IN GT_U32                           vrId,
    IN CPSS_IP_PROTOCOL_STACK_ENT       protocol,
    IN CPSS_UNICAST_MULTICAST_ENT       prefixType,
    IN GT_BOOL                          returnOnFailure
)
{
    PRV_CPSS_DXCH_LPM_SHADOW_STC            *lpmDbPtr,tmpLpmDb;
    PRV_CPSS_DXCH_LPM_HW_ENT                lpmHw;
    PRV_CPSS_DXCH_LPM_PROTOCOL_BMP          protocolBitmap;
    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT    lpmProtocol;
    GT_STATUS                               rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
                                          CPSS_CH1_E | CPSS_CH1_DIAMOND_E |
                                          CPSS_CH2_E | CPSS_CH3_E |
                                          CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E |
                                          CPSS_XCAT2_E | CPSS_LION2_E);
    switch (protocol)
    {
        case CPSS_IP_PROTOCOL_IPV4_E:
            lpmProtocol = PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E;
            break;

        case CPSS_IP_PROTOCOL_IPV6_E:
            lpmProtocol = PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E;
            break;

        case CPSS_IP_PROTOCOL_FCOE_E:
            lpmProtocol = PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Wrong protocol type");
    }

    if ((prefixType != CPSS_UNICAST_E) && (prefixType != CPSS_MULTICAST_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Wrong prefix type");
    }

    /* run HW validation */
    rc = prvCpssDxChLpmRamDbgHwValidation(devNum, vrId, lpmProtocol, prefixType);
    if (rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("HW validation failed");
        return rc;
    }

    tmpLpmDb.lpmDbId = lpmDbId;
    lpmDbPtr = prvCpssSlSearch(prvCpssDxChIplpmDbSL,&tmpLpmDb);
    if (lpmDbPtr == NULL)
    {
        /* can't find the lpm DB */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, "LPM DB not found");
    }

    lpmHw = prvCpssDxChLpmGetHwType(lpmDbPtr->shadowType);

    switch (lpmHw)
    {
        case PRV_CPSS_DXCH_LPM_HW_TCAM_E:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, "Invalid shadow type");

        case PRV_CPSS_DXCH_LPM_HW_RAM_E:
            prvCpssDxChLpmConvertIpProtocolStackToProtocolBitmap(protocol, &protocolBitmap);
            if (((protocol==CPSS_IP_PROTOCOL_FCOE_E)||(protocol==CPSS_IP_PROTOCOL_ALL_E))&&
                (lpmDbPtr->shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP5_SHADOW_E)&&
                (prefixType==CPSS_UNICAST_E))
            {
                PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_SET_MAC(protocolBitmap);
            }
            /* run shadow validation */
            rc = prvCpssDxChLpmRamDbgShadowValidityCheck((PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC*)(lpmDbPtr->shadow),
                                                           vrId, protocolBitmap, prefixType, returnOnFailure);
            if (rc != GT_OK)
            {
                CPSS_LOG_INFORMATION_MAC("Shadow validation failed");
                return rc;
            }

            /* run shadow and HW synchronization check */
            rc = prvCpssDxChLpmRamDbgHwShadowSyncValidityCheck(devNum,
                                                               (PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC*)(lpmDbPtr->shadow),
                                                               vrId, protocolBitmap, prefixType, returnOnFailure);
            if (rc != GT_OK)
            {
                CPSS_LOG_INFORMATION_MAC("HW/Shadow synchronization check failed");
                return rc;
            }
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Invalid shadow type");
    }

    return rc;
}

/**
* @internal cpssDxChIpLpmDbgHwShadowValidityAndSyncCheck function
* @endinternal
*
* @brief   This function Validate the LPM in shadow, Validate the LPM in HW and
*         check synchronization between the shadow and HW of the LPM
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
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpLpmDbgHwShadowValidityAndSyncCheck);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vrId, protocol, prefixType, returnOnFailure));

    rc = internal_cpssDxChIpLpmDbgHwShadowValidityAndSyncCheck(devNum, lpmDbId, vrId,
                                                               protocol, prefixType,
                                                               returnOnFailure);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vrId, protocol, prefixType, returnOnFailure));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

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
)
{
    switch (protocolStack)
    {
    case CPSS_IP_PROTOCOL_IPV4_E:
    case CPSS_IP_PROTOCOL_FCOE_E:
        if (prefixType==CPSS_UNICAST_E)
        {
            IPv4PrefixLength = prefixLength;
        }
        else
        {
            IPv4PrefixLength = prefixLength;
            srcIPv4PrefixLength = srcPrefixLength;
        }

        break;
    case CPSS_IP_PROTOCOL_IPV6_E:
        if (prefixType==CPSS_UNICAST_E)
        {
            IPv6PrefixLength = prefixLength;
        }
        else
        {
            IPv6PrefixLength = prefixLength;
            srcIPv6PrefixLength = srcPrefixLength;
        }
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        break;
    }
    return GT_OK;
}

/**
* @internal internal_cpssDxChIpLpmDbgIpv4UcPrefixAddMany function
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
static GT_STATUS internal_cpssDxChIpLpmDbgIpv4UcPrefixAddMany
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPADDR   startIpAddr,
    IN  GT_U32      routeEntryBaseMemAddr,
    IN  GT_U32      numOfPrefixesToAdd,
    OUT GT_IPADDR   *lastIpAddrAddedPtr,
    OUT GT_U32      *numOfPrefixesAddedPtr
)
{
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  i = 0;
    GT_IPADDR                               currentIpAddr;
    GT_U32                                  numOfPrefixesAdded = 0;
    GT_U32                                  numOfPrefixesAlreadyExist = 0;
    GT_IPADDR                               lastIpAddrAdded;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT  nextHopInfo;
    GT_BOOL                                 override = GT_FALSE;
    GT_BOOL                                 defragmentationEnable = defragmentationEnableFlag;
    GT_U32                                  IPv4Val[4];

    currentIpAddr.u32Ip                           = startIpAddr.u32Ip;
    lastIpAddrAdded.u32Ip                         = startIpAddr.u32Ip;

    cpssOsMemSet(&nextHopInfo, 0, sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
    nextHopInfo.ipLttEntry.routeType           = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = routeEntryBaseMemAddr;
    nextHopInfo.ipLttEntry.numOfPaths          = 0;
    nextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;
    rc = prvCpssDxChIpLpmDbgConvertRouteType(lpmDbId, &nextHopInfo.ipLttEntry);
    if (rc != GT_OK)
    {
        return rc;
    }

    for (i = 0 ; i < numOfPrefixesToAdd ; i++)
    {
        /* add the current prefix */
        rc = cpssDxChIpLpmIpv4UcPrefixAdd(lpmDbId,vrId,&currentIpAddr,
                                          IPv4PrefixLength,&nextHopInfo,
                                          override,defragmentationEnable);
        if ((rc != GT_OK) && (rc!=GT_ALREADY_EXIST))
            break;

        if (rc==GT_ALREADY_EXIST)
        {
            rc = GT_OK;
            numOfPrefixesAlreadyExist++;
        }
        else
            numOfPrefixesAdded++;

        lastIpAddrAdded.u32Ip = currentIpAddr.u32Ip;

        IPv4Val[3] = currentIpAddr.arIP[3];
        IPv4Val[2] = currentIpAddr.arIP[2];
        IPv4Val[1] = currentIpAddr.arIP[1];
        IPv4Val[0] = currentIpAddr.arIP[0];

        /* advance to the next IP address */
        if (IPv4Val[3] + IPv4Step[3] <= 255)
            currentIpAddr.arIP[3] += IPv4Step[3];
        else
        {
            currentIpAddr.arIP[3] = 0;
            if (IPv4Val[2] + IPv4Step[2] <= 255)
                currentIpAddr.arIP[2] += IPv4Step[2];
            else
            {
                currentIpAddr.arIP[2] = 0;
                if (IPv4Val[1] + IPv4Step[1] <= 255)
                    currentIpAddr.arIP[1] += IPv4Step[1];
                else
                {
                    currentIpAddr.arIP[1] = 0;
                    if (IPv4Val[0] + IPv4Step[0] <= 255)
                        currentIpAddr.arIP[0] += IPv4Step[0];
                    else
                    {
                        rc = GT_NO_MORE;
                        break;
                    }
                }
            }
        }
    }

    /* print result */
    cpssOsPrintf("\nIpv4UcPrefixAddMany results:");
    if (numOfPrefixesAdded == 0)
    {
        cpssOsPrintf("\n    No prefixes were added at all.\n");
    }
    else
    {
        cpssOsPrintf("\n    Prefixes ");
        outputIPAddress(startIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
        cpssOsPrintf(" - ");
        outputIPAddress(lastIpAddrAdded.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
        cpssOsPrintf(" were added (%d/%d)\n",numOfPrefixesAdded,numOfPrefixesToAdd);
        cpssOsPrintf("    (%d) already existed \n",numOfPrefixesAlreadyExist);
        cpssOsPrintf("    total number of prefixes added plus existed (%d/%d) \n",(numOfPrefixesAlreadyExist+numOfPrefixesAdded),numOfPrefixesToAdd);

    }

    /* update output parameters */
    if (numOfPrefixesAddedPtr != NULL)
        *numOfPrefixesAddedPtr = numOfPrefixesAdded;
    if (lastIpAddrAddedPtr != NULL)
        lastIpAddrAddedPtr->u32Ip = lastIpAddrAdded.u32Ip;

    /* Return to default */
    for (i = 0;i < 4;i++) {
        IPv4Step[i] = 1;
        srcIPv4Step[i] = 1;
    }
    IPv4PrefixLength = 32;
    srcIPv4PrefixLength = 32;

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpLpmDbgIpv4UcPrefixAddMany);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, lpmDbId, vrId, startIpAddr, routeEntryBaseMemAddr,
                            numOfPrefixesToAdd, lastIpAddrAddedPtr, numOfPrefixesAddedPtr));

    rc = internal_cpssDxChIpLpmDbgIpv4UcPrefixAddMany(lpmDbId, vrId, startIpAddr,
                                                      routeEntryBaseMemAddr,
                                                      numOfPrefixesToAdd,
                                                      lastIpAddrAddedPtr,
                                                      numOfPrefixesAddedPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, lpmDbId, vrId, startIpAddr, routeEntryBaseMemAddr,
                              numOfPrefixesToAdd, lastIpAddrAddedPtr, numOfPrefixesAddedPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpLpmDbgIpv4UcPrefixAddManyRandom function
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
static GT_STATUS internal_cpssDxChIpLpmDbgIpv4UcPrefixAddManyRandom
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPADDR   startIpAddr,
    IN  GT_U32      routeEntryBaseMemAddr,
    IN  GT_U32      numOfPrefixesToAdd,
    IN  GT_BOOL     isWholeIpRandom,
    IN  GT_BOOL     defragmentationEnable,
    OUT GT_U32      *numOfPrefixesAddedPtr
)
{
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  i = 0;
    GT_IPADDR                               currentIpAddr;
    GT_U32                                  numOfPrefixesAdded = 0;
    GT_U32                                  numOfPrefixesAlreadyExist = 0;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT  nextHopInfo;
    GT_BOOL                                 override = GT_FALSE;
    GT_U32                                  counter = 0;
    GT_U32                                  maxCounterValue = 0;

    cpssOsMemSet(&nextHopInfo, 0, sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
    nextHopInfo.ipLttEntry.routeType           = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = routeEntryBaseMemAddr;
    nextHopInfo.ipLttEntry.numOfPaths          = 0;
    nextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;
    rc = prvCpssDxChIpLpmDbgConvertRouteType(lpmDbId, &nextHopInfo.ipLttEntry);
    if (rc != GT_OK)
    {
        return rc;
    }

    currentIpAddr.u32Ip                     = startIpAddr.u32Ip;
    currentIpAddr.arIP[3] = (GT_U8)(cpssOsRand() % 256);
    currentIpAddr.arIP[2]= (GT_U8)(cpssOsRand() % 256);

    if (isWholeIpRandom == GT_TRUE)
    {
        currentIpAddr.arIP[1] = (GT_U8)(cpssOsRand() % 256);
        currentIpAddr.arIP[0]= (GT_U8)(cpssOsRand() % 224);
    }
    for (i = 0 ; i < numOfPrefixesToAdd ; i++)
    {
        rc = 100;
        counter = 0;
        while (rc != GT_OK)
        {
            /* add the current prefix */
            rc = cpssDxChIpLpmIpv4UcPrefixAdd(lpmDbId,vrId,&currentIpAddr,
                                          IPv4PrefixLength,&nextHopInfo,
                                          override,defragmentationEnable);
            if (rc != GT_OK)
            {
                if (rc == GT_ALREADY_EXIST)
                {
                    if (counter > 20000)
                        break;
                    currentIpAddr.arIP[3] = (GT_U8)(cpssOsRand() % 256);
                    currentIpAddr.arIP[2]= (GT_U8)(cpssOsRand() % 256);

                    if (isWholeIpRandom == GT_TRUE)
                    {
                        currentIpAddr.arIP[1] = (GT_U8)(cpssOsRand() % 256);
                        currentIpAddr.arIP[0]= (GT_U8)(cpssOsRand() % 224);
                    }
                    counter++;
                    if (counter > maxCounterValue)
                        maxCounterValue = counter;
                }
                else
                {
                    break;
                }
            }
        }

        if ((rc != GT_OK) && (rc!=GT_ALREADY_EXIST))
            break;

        if (rc==GT_ALREADY_EXIST)
        {
            rc = GT_OK;
            numOfPrefixesAlreadyExist++;
        }
        else
            numOfPrefixesAdded++;

        if ((numOfPrefixesAdded % 50) == 0 )
        {
            currentIpAddr.arIP[1] = (GT_U8)(cpssOsRand() % 256);
            currentIpAddr.arIP[0]= (GT_U8)(cpssOsRand() % 224);
        }
        currentIpAddr.arIP[3] = (GT_U8)(cpssOsRand() % 256);
        currentIpAddr.arIP[2]= (GT_U8)(cpssOsRand() % 256);
        if (isWholeIpRandom == GT_TRUE)
        {
            currentIpAddr.arIP[1] = (GT_U8)(cpssOsRand() % 256);
            currentIpAddr.arIP[0]= (GT_U8)(cpssOsRand() % 224);
        }
    }
    /* print result */
    cpssOsPrintf("\nIpv4UcPrefixAddManyRandom results:");
    if (rc != GT_OK)
    {
        cpssOsPrintf("\n error = %x occured on prefix\n",rc);
        outputIPAddress(currentIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
    }

    /* update output parameters */
    if (numOfPrefixesAddedPtr != NULL)
        *numOfPrefixesAddedPtr = numOfPrefixesAdded;

    if (numOfPrefixesAdded == 0)
    {
        cpssOsPrintf("\n    No prefixes were added at all.\n");
    }
    else
    {
        cpssOsPrintf(" were added %d prefixes from %d prefixes)\n",numOfPrefixesAdded,numOfPrefixesToAdd);
    }
    cpssOsPrintf("maxCounterValue =%d\n",maxCounterValue);
    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpLpmDbgIpv4UcPrefixAddManyRandom);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, lpmDbId, vrId, startIpAddr, routeEntryBaseMemAddr,
                            numOfPrefixesToAdd, isWholeIpRandom, defragmentationEnable,
                            numOfPrefixesAddedPtr));

    rc = internal_cpssDxChIpLpmDbgIpv4UcPrefixAddManyRandom(lpmDbId, vrId, startIpAddr,
                                                            routeEntryBaseMemAddr,
                                                            numOfPrefixesToAdd,
                                                            isWholeIpRandom,
                                                            defragmentationEnable,
                                                            numOfPrefixesAddedPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, lpmDbId, vrId, startIpAddr, routeEntryBaseMemAddr,
                              numOfPrefixesToAdd, isWholeIpRandom, numOfPrefixesAddedPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpLpmDbgIpv4UcPrefixAddManyByOctet function
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
static GT_STATUS internal_cpssDxChIpLpmDbgIpv4UcPrefixAddManyByOctet
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
)
{
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  i = 0;
    GT_IPADDR                               currentIpAddr;
    GT_U32                                  numOfPrefixesAdded = 0;
    GT_U32                                  numOfMcPrefixesNotAdded = 0;
    GT_U32                                  numOfPrefixesAlreadyExist = 0;
    GT_IPADDR                               lastIpAddrAdded;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT  nextHopInfo;
    GT_BOOL                                 override = GT_FALSE;
    GT_BOOL                                 defragmentationEnable = defragmentationEnableFlag;
    GT_BOOL                                 octetsArray[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS]={GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE};/* array to hold the octets already selected */
    GT_U32 IPv4Val[4] = {0};

    currentIpAddr.u32Ip                     = startIpAddr.u32Ip;
    lastIpAddrAdded.u32Ip                   = startIpAddr.u32Ip;

    cpssOsMemSet(&nextHopInfo, 0, sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
    nextHopInfo.ipLttEntry.routeType           = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = routeEntryBaseMemAddr;
    nextHopInfo.ipLttEntry.numOfPaths          = 0;
    nextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

    rc = prvCpssDxChIpLpmDbgConvertRouteType(lpmDbId, &nextHopInfo.ipLttEntry);
    if (rc != GT_OK)
    {
        return rc;
    }

    if((firstOctetToChange>3) || (secondOctetToChange>3) || (thirdOctetToChange>3) || (fourthOctetToChange>3))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    /* make sure octet are not selected twice */
    if(octetsArray[firstOctetToChange]==GT_FALSE)
    {
        octetsArray[firstOctetToChange]=GT_TRUE;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if(octetsArray[secondOctetToChange]==GT_FALSE)
    {
        octetsArray[secondOctetToChange]=GT_TRUE;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if(octetsArray[thirdOctetToChange]==GT_FALSE)
    {
        octetsArray[thirdOctetToChange]=GT_TRUE;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if(octetsArray[fourthOctetToChange]==GT_FALSE)
    {
        octetsArray[fourthOctetToChange]=GT_TRUE;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    for (i = 0 ; i < numOfPrefixesToAdd ; )
    {
        /* make sure the prefix is not a MC prefix - not in the range 224/4 - 239/4
           therefore the first octate must not start with 0xE */
        if((currentIpAddr.arIP[0] & 0xF0) != 0xE0)
        {
            /* add the current prefix */
            rc = cpssDxChIpLpmIpv4UcPrefixAdd(lpmDbId,vrId,&currentIpAddr,
                                              IPv4PrefixLength,&nextHopInfo,
                                              override,defragmentationEnable);
            if ((rc != GT_OK) && (rc!=GT_ALREADY_EXIST))
                break;

            if (rc==GT_ALREADY_EXIST)
            {
                rc = GT_OK;
                numOfPrefixesAlreadyExist++;
            }
            else
                numOfPrefixesAdded++;

            i++;
        }
        else
        {
            numOfMcPrefixesNotAdded++;
        }


        lastIpAddrAdded.u32Ip = currentIpAddr.u32Ip;

        IPv4Val[3] = currentIpAddr.arIP[3];
        IPv4Val[2] = currentIpAddr.arIP[2];
        IPv4Val[1] = currentIpAddr.arIP[1];
        IPv4Val[0] = currentIpAddr.arIP[0];

        /* advance to the next IP address */
        if (IPv4Val[firstOctetToChange] + IPv4Step[firstOctetToChange] <= 255)
            currentIpAddr.arIP[firstOctetToChange] += IPv4Step[firstOctetToChange];
        else
        {
            currentIpAddr.arIP[firstOctetToChange] = 0;
            if (IPv4Val[secondOctetToChange] + IPv4Step[secondOctetToChange] <= 255)
                currentIpAddr.arIP[secondOctetToChange] += IPv4Step[secondOctetToChange];
            else
            {
                currentIpAddr.arIP[secondOctetToChange] = 0;
                if (IPv4Val[thirdOctetToChange] + IPv4Step[thirdOctetToChange] <= 255)
                    currentIpAddr.arIP[thirdOctetToChange] += IPv4Step[thirdOctetToChange];
                else
                {
                    currentIpAddr.arIP[thirdOctetToChange] = 0;
                    if (IPv4Val[fourthOctetToChange] + IPv4Step[fourthOctetToChange] <= 255)
                        currentIpAddr.arIP[fourthOctetToChange] += IPv4Step[fourthOctetToChange];
                    else
                    {
                        rc = GT_NO_MORE;
                        break;
                    }
                }
            }
        }
    }

    /* print result */
    cpssOsPrintf("\nIpv4UcPrefixAddMany results:");
    if (numOfPrefixesAdded == 0)
    {
        cpssOsPrintf("\n    No prefixes were added at all.\n");
    }
    else
    {
        cpssOsPrintf("\n    Prefixes ");
        outputIPAddress(startIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
        cpssOsPrintf(" - ");
        outputIPAddress(lastIpAddrAdded.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
        cpssOsPrintf(" were added (%d/%d)\n",numOfPrefixesAdded,numOfPrefixesToAdd);
        cpssOsPrintf("    (%d) MC prefixes not added in this range\n",numOfMcPrefixesNotAdded);
        cpssOsPrintf("    (%d) already existed \n",numOfPrefixesAlreadyExist);
        cpssOsPrintf("    total number of prefixes added plus existed (%d/%d) \n",(numOfPrefixesAlreadyExist+numOfPrefixesAdded),numOfPrefixesToAdd);
    }

    /* update output parameters */
    if (numOfPrefixesAddedPtr != NULL)
        *numOfPrefixesAddedPtr = numOfPrefixesAdded;
    if (lastIpAddrAddedPtr != NULL)
        lastIpAddrAddedPtr->u32Ip = lastIpAddrAdded.u32Ip;

    /* Return to default */
    for (i = 0; i < 4; i++) {
        IPv4Step[i] = 1;
        srcIPv4Step[i] = 1;
    }
    IPv4PrefixLength = 32;
    srcIPv4PrefixLength = 32;

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpLpmDbgIpv4UcPrefixAddManyByOctet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, lpmDbId, vrId, startIpAddr, routeEntryBaseMemAddr,
                            numOfPrefixesToAdd, firstOctetToChange, secondOctetToChange,
                            thirdOctetToChange, fourthOctetToChange, lastIpAddrAddedPtr,
                            numOfPrefixesAddedPtr));

    rc = internal_cpssDxChIpLpmDbgIpv4UcPrefixAddManyByOctet(lpmDbId, vrId, startIpAddr,
                                                             routeEntryBaseMemAddr, numOfPrefixesToAdd,
                                                             firstOctetToChange, secondOctetToChange,
                                                             thirdOctetToChange, fourthOctetToChange,
                                                             lastIpAddrAddedPtr, numOfPrefixesAddedPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, lpmDbId, vrId, startIpAddr, routeEntryBaseMemAddr,
                              numOfPrefixesToAdd, firstOctetToChange, secondOctetToChange,
                              thirdOctetToChange, fourthOctetToChange, lastIpAddrAddedPtr,
                              numOfPrefixesAddedPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpLpmDbgIpv4UcPrefixDelManyByOctet function
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
static GT_STATUS internal_cpssDxChIpLpmDbgIpv4UcPrefixDelManyByOctet
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
)
{
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  i = 0;
    GT_IPADDR                               currentIpAddr;
    GT_U32                                  numOfPrefixesDeleted = 0;
    GT_U32                                  numOfPrefixesAlreadyDeleted = 0;
    GT_U32                                  numOfMcPrefixesNotDeleted = 0;
    GT_IPADDR                               lastIpAddrDeleted;
    GT_BOOL                                 octetsArray[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS]={GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE};/* array to hold the octets already selected */
    GT_U32                                  IPv4Val[4] = {0};

    currentIpAddr.u32Ip                     = startIpAddr.u32Ip;
    lastIpAddrDeleted.u32Ip                 = startIpAddr.u32Ip;

    if((firstOctetToChange>3) || (secondOctetToChange>3) || (thirdOctetToChange>3) || (fourthOctetToChange>3))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    /* make sure octet are not selected twice */
    if(octetsArray[firstOctetToChange]==GT_FALSE)
    {
        octetsArray[firstOctetToChange]=GT_TRUE;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if(octetsArray[secondOctetToChange]==GT_FALSE)
    {
        octetsArray[secondOctetToChange]=GT_TRUE;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if(octetsArray[thirdOctetToChange]==GT_FALSE)
    {
        octetsArray[thirdOctetToChange]=GT_TRUE;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if(octetsArray[fourthOctetToChange]==GT_FALSE)
    {
        octetsArray[fourthOctetToChange]=GT_TRUE;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    for (i = 0 ; i < numOfPrefixesToDel ; )
    {
        /* make sure the prefix is not a MC prefix - not in the range 224/4 - 239/4
           therefore the first octate must not start with 0xE */
        if((currentIpAddr.arIP[0] & 0xF0) != 0xE0)
        {
            /* delete the current prefix */
            rc = cpssDxChIpLpmIpv4UcPrefixDel(lpmDbId,vrId,&currentIpAddr,IPv4PrefixLength);

            if ((rc != GT_OK) && (rc!=GT_NOT_FOUND))
                break;

            if (rc==GT_NOT_FOUND)
            {
                rc = GT_OK;
                numOfPrefixesAlreadyDeleted++;
            }
            else
                numOfPrefixesDeleted++;
            i++;
        }
        else
        {
            numOfMcPrefixesNotDeleted++;
        }


        lastIpAddrDeleted.u32Ip = currentIpAddr.u32Ip;

        IPv4Val[3] = currentIpAddr.arIP[3];
        IPv4Val[2] = currentIpAddr.arIP[2];
        IPv4Val[1] = currentIpAddr.arIP[1];
        IPv4Val[0] = currentIpAddr.arIP[0];

        /* advance to the next IP address */
        if (IPv4Val[firstOctetToChange] + IPv4Step[firstOctetToChange] <= 255)
            currentIpAddr.arIP[firstOctetToChange] += IPv4Step[firstOctetToChange];
        else
        {
            currentIpAddr.arIP[firstOctetToChange] = 0;
            if (IPv4Val[secondOctetToChange] + IPv4Step[secondOctetToChange] <= 255)
                currentIpAddr.arIP[secondOctetToChange] += IPv4Step[secondOctetToChange];
            else
            {
                currentIpAddr.arIP[secondOctetToChange] = 0;
                if (IPv4Val[thirdOctetToChange] + IPv4Step[thirdOctetToChange] <= 255)
                    currentIpAddr.arIP[thirdOctetToChange] += IPv4Step[thirdOctetToChange];
                else
                {
                    currentIpAddr.arIP[thirdOctetToChange] = 0;
                    if (IPv4Val[fourthOctetToChange] + IPv4Step[fourthOctetToChange] <= 255)
                        currentIpAddr.arIP[fourthOctetToChange] += IPv4Step[fourthOctetToChange];
                    else
                    {
                        rc = GT_NO_MORE;
                        break;
                    }
                }
            }
        }
    }

    /* print result */
    cpssOsPrintf("\nIpv4UcPrefixDelMany results:");
    if (numOfPrefixesDeleted == 0)
    {
        cpssOsPrintf("\n    No prefixes were Deleted at all.\n");
    }
    else
    {
        cpssOsPrintf("\n    Prefixes ");
        outputIPAddress(startIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
        cpssOsPrintf(" - ");
        outputIPAddress(lastIpAddrDeleted.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
        cpssOsPrintf(" were Deleted (%d/%d)\n",numOfPrefixesDeleted,numOfPrefixesToDel);
        cpssOsPrintf("    (%d) MC prefixes not Deleted in this range\n",numOfMcPrefixesNotDeleted);
        cpssOsPrintf("    (%d) already deleted \n",numOfPrefixesAlreadyDeleted);
        cpssOsPrintf("    total number of prefixes deleted plus not_found (%d/%d) \n",(numOfPrefixesAlreadyDeleted+numOfPrefixesDeleted),numOfPrefixesToDel);
    }

    /* update output parameters */
    if (numOfPrefixesDeletedPtr != NULL)
        *numOfPrefixesDeletedPtr = numOfPrefixesDeleted;
    if (lastIpAddrDeletedPtr != NULL)
        lastIpAddrDeletedPtr->u32Ip = lastIpAddrDeleted.u32Ip;

    /* Return to default */
    for (i = 0; i < 4; i++) {
        IPv4Step[i] = 1;
        srcIPv4Step[i] = 1;
    }
    IPv4PrefixLength = 32;
    srcIPv4PrefixLength = 32;

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpLpmDbgIpv4UcPrefixDelManyByOctet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, lpmDbId, vrId, startIpAddr, numOfPrefixesToDel,
                            firstOctetToChange, secondOctetToChange, thirdOctetToChange,
                            fourthOctetToChange, lastIpAddrDeletedPtr, numOfPrefixesDeletedPtr));

    rc = internal_cpssDxChIpLpmDbgIpv4UcPrefixDelManyByOctet(lpmDbId, vrId, startIpAddr,
                                                             numOfPrefixesToDel, firstOctetToChange,
                                                             secondOctetToChange, thirdOctetToChange,
                                                             fourthOctetToChange, lastIpAddrDeletedPtr,
                                                             numOfPrefixesDeletedPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, lpmDbId, vrId, startIpAddr, numOfPrefixesToDel,
                              firstOctetToChange, secondOctetToChange, thirdOctetToChange,
                              fourthOctetToChange, lastIpAddrDeletedPtr, numOfPrefixesDeletedPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpLpmDbgIpv4UcPrefixGetManyByOctet function
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
static GT_STATUS internal_cpssDxChIpLpmDbgIpv4UcPrefixGetManyByOctet
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
)
{
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  i = 0;
    GT_IPADDR                               currentIpAddr;
    GT_U32                                  numOfRetrievedPrefixes = 0;
    GT_U32                                  numOfMcPrefixesNotAdded = 0;
    GT_IPADDR                               lastIpAddrAdded;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT  nextHopInfo;
    GT_BOOL                                 octetsArray[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS]={GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE};/* array to hold the octets already selected */
    GT_U32                                  tcamRowIndex=0;
    GT_U32                                  tcamColumnIndex=0;
    GT_U32                                  IPv4Val[4] = { 0 };

    currentIpAddr.u32Ip                     = startIpAddr.u32Ip;
    lastIpAddrAdded.u32Ip                   = startIpAddr.u32Ip;

    cpssOsMemSet(&nextHopInfo, 0, sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));

    if((firstOctetToChange>3) || (secondOctetToChange>3) || (thirdOctetToChange>3) || (fourthOctetToChange>3))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    /* make sure octet are not selected twice */
    if(octetsArray[firstOctetToChange]==GT_FALSE)
    {
        octetsArray[firstOctetToChange]=GT_TRUE;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if(octetsArray[secondOctetToChange]==GT_FALSE)
    {
        octetsArray[secondOctetToChange]=GT_TRUE;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if(octetsArray[thirdOctetToChange]==GT_FALSE)
    {
        octetsArray[thirdOctetToChange]=GT_TRUE;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if(octetsArray[fourthOctetToChange]==GT_FALSE)
    {
        octetsArray[fourthOctetToChange]=GT_TRUE;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    for (i = 0 ; i < numOfPrefixesToGet ; )
    {
        /* make sure the prefix is not a MC prefix - not in the range 224/4 - 239/4
           therefore the first octate must not start with 0xE */
        if((currentIpAddr.arIP[0] & 0xF0) != 0xE0)
        {
            /* get the current prefix */
            rc =  cpssDxChIpLpmIpv4UcPrefixSearch(lpmDbId,
                                                 vrId,
                                                 &currentIpAddr,
                                                 IPv4PrefixLength,
                                                 &nextHopInfo,
                                                 &tcamRowIndex,
                                                 &tcamColumnIndex);
            if (rc != GT_OK)
                break;

            numOfRetrievedPrefixes++;
            i++;
        }
        else
        {
            numOfMcPrefixesNotAdded++;
        }

        lastIpAddrAdded.u32Ip = currentIpAddr.u32Ip;

        IPv4Val[3] = currentIpAddr.arIP[3];
        IPv4Val[2] = currentIpAddr.arIP[2];
        IPv4Val[1] = currentIpAddr.arIP[1];
        IPv4Val[0] = currentIpAddr.arIP[0];

        /* advance to the next IP address */
        if (IPv4Val[firstOctetToChange] + IPv4Step[firstOctetToChange] <= 255)
            currentIpAddr.arIP[firstOctetToChange] += IPv4Step[firstOctetToChange];
        else
        {
            currentIpAddr.arIP[firstOctetToChange] = 0;
            if (IPv4Val[secondOctetToChange] + IPv4Step[secondOctetToChange] <= 255)
                currentIpAddr.arIP[secondOctetToChange] += IPv4Step[secondOctetToChange];
            else
            {
                currentIpAddr.arIP[secondOctetToChange] = 0;
                if (IPv4Val[thirdOctetToChange] + IPv4Step[thirdOctetToChange] <= 255)
                    currentIpAddr.arIP[thirdOctetToChange] += IPv4Step[thirdOctetToChange];
                else
                {
                    currentIpAddr.arIP[thirdOctetToChange] = 0;
                    if (IPv4Val[fourthOctetToChange] + IPv4Step[fourthOctetToChange] <= 255)
                        currentIpAddr.arIP[fourthOctetToChange] += IPv4Step[fourthOctetToChange];
                    else
                    {
                        rc = GT_NO_MORE;
                        break;
                    }
                }
            }
        }
    }

    /* print result */
    cpssOsPrintf("\nIpv4UcPrefixGetMany results:");
    if (numOfRetrievedPrefixes == 0)
    {
        cpssOsPrintf("\n    No prefixes at all.\n");
    }
    else
    {
        cpssOsPrintf("\n    Prefixes ");
        outputIPAddress(startIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
        cpssOsPrintf(" - ");
        outputIPAddress(lastIpAddrAdded.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
        cpssOsPrintf(" were retrieved (%d/%d)\n",numOfRetrievedPrefixes,numOfPrefixesToGet);
        cpssOsPrintf("    (%d) MC prefixes not added in this range\n",numOfMcPrefixesNotAdded);
    }

    /* update output parameters */
    if (numOfRetrievedPrefixesPtr != NULL)
        *numOfRetrievedPrefixesPtr = numOfRetrievedPrefixes;

    /* Return to default */
    for (i = 0; i < 4; i++) {
        IPv4Step[i] = 1;
        srcIPv4Step[i] = 1;
    }
    IPv4PrefixLength = 32;
    srcIPv4PrefixLength = 32;

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpLpmDbgIpv4UcPrefixGetManyByOctet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, lpmDbId, vrId, startIpAddr, numOfPrefixesToGet,
                            firstOctetToChange, secondOctetToChange, thirdOctetToChange,
                            fourthOctetToChange, numOfRetrievedPrefixesPtr));

    rc = internal_cpssDxChIpLpmDbgIpv4UcPrefixGetManyByOctet(lpmDbId, vrId, startIpAddr,
                                                              numOfPrefixesToGet,
                                                              firstOctetToChange,
                                                              secondOctetToChange,
                                                              thirdOctetToChange,
                                                              fourthOctetToChange,
                                                              numOfRetrievedPrefixesPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, lpmDbId, vrId, startIpAddr, numOfPrefixesToGet,
                              firstOctetToChange, secondOctetToChange, thirdOctetToChange,
                              fourthOctetToChange, numOfRetrievedPrefixesPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpLpmDbgIpv6UcPrefixAddManyByOctet function
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
static GT_STATUS internal_cpssDxChIpLpmDbgIpv6UcPrefixAddManyByOctet
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
)
{
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  i = 0;
    GT_U32                                  j = 0;
    GT_IPV6ADDR                             currentIpAddr;
    GT_U32                                  numOfPrefixesAdded = 0;
    GT_U32                                  numOfMcPrefixesNotAdded = 0;
    GT_U32                                  numOfPrefixesAlreadyExist = 0;
    GT_IPV6ADDR                             lastIpAddrAdded;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT  nextHopInfo;
    GT_BOOL                                 override = GT_FALSE;
    GT_BOOL                                 octetsArray[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS]={GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE,
                                                            GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE,
                                                            GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE,
                                                            GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE};/* array to hold the octets already selected */

    GT_U32                                  octetsOrderToChangeArray[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS];
    GT_BOOL                                 defragmentationEnable=defragmentationEnableFlag;
    GT_U32                                  IPv6Val[16] = { 0 };
    PRV_CPSS_DXCH_LPM_SHADOW_STC            *lpmDbPtr,tmpLpmDb;

    tmpLpmDb.lpmDbId = lpmDbId;
    lpmDbPtr = prvCpssSlSearch(prvCpssDxChIplpmDbSL,&tmpLpmDb);
    if (lpmDbPtr == NULL)
    {
        /* can't find the lpm DB */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }


    cpssOsMemSet(&nextHopInfo, 0, sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = routeEntryBaseMemAddr;
    nextHopInfo.ipLttEntry.numOfPaths          = 0;
    nextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;
    if (lpmDbPtr->shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
    {
        nextHopInfo.ipLttEntry.routeType           = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
    }
    else
    {
        nextHopInfo.ipLttEntry.routeType           = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
    }
    rc = prvCpssDxChIpLpmDbgConvertRouteType(lpmDbId, &nextHopInfo.ipLttEntry);
    if (rc != GT_OK)
    {
        return rc;
    }

    cpssOsMemSet(&currentIpAddr, 0, sizeof(GT_IPV6ADDR));

    for(j=0;j<4;j++)
    {
        currentIpAddr.u32Ip[j]     = startIpAddr.u32Ip[j];
        lastIpAddrAdded.u32Ip[j]    = startIpAddr.u32Ip[j];
    }


    octetsOrderToChangeArray[0] = firstOctetToChange;
    octetsOrderToChangeArray[1] = secondOctetToChange;
    octetsOrderToChangeArray[2] = thirdOctetToChange;
    octetsOrderToChangeArray[3] = fourthOctetToChange;
    octetsOrderToChangeArray[4] = fifthOctetToChange;
    octetsOrderToChangeArray[5] = sixthOctetToChange;
    octetsOrderToChangeArray[6] = seventhOctetToChange;
    octetsOrderToChangeArray[7] = eighthOctetToChange;
    octetsOrderToChangeArray[8] = ninthOctetToChange;
    octetsOrderToChangeArray[9] = tenthOctetToChange;
    octetsOrderToChangeArray[10] = elevenOctetToChange;
    octetsOrderToChangeArray[11] = twelveOctetToChange;
    octetsOrderToChangeArray[12] = thirteenOctetToChange;
    octetsOrderToChangeArray[13] = fourteenOctetToChange;
    octetsOrderToChangeArray[14] = fifteenOctetToChange;
    octetsOrderToChangeArray[15] = sixteenOctetToChange;

    for(j=0;j<PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS;j++)
    {
        if(octetsOrderToChangeArray[j]>15)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    for(j=0;j<PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS;j++)
    {
        /* make sure octet are not selected twice */
        if(octetsArray[octetsOrderToChangeArray[j]]==GT_FALSE)
        {
            octetsArray[octetsOrderToChangeArray[j]]=GT_TRUE;
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }


    for (i = 0 ; i < numOfPrefixesToAdd ; )
    {
        /* make sure the prefix is not a MC prefix - not in format ff00::/8
           therefore the first octate must not start with 0xFF */
        if((currentIpAddr.arIP[0]) != 0xFF)
        {

            /* add the current prefix */
            rc = cpssDxChIpLpmIpv6UcPrefixAdd(lpmDbId,
                                              vrId,
                                              &currentIpAddr,
                                              IPv6PrefixLength,
                                              &nextHopInfo,
                                              override,
                                              defragmentationEnable);
            if ((rc != GT_OK) && (rc!=GT_ALREADY_EXIST))
                break;

            if (rc==GT_ALREADY_EXIST)
            {
                rc = GT_OK;
                numOfPrefixesAlreadyExist++;
            }
            else
                numOfPrefixesAdded++;
            i++;
        }
        else
        {
            numOfMcPrefixesNotAdded++;
        }

        for(j=0;j<4;j++)
        {
            lastIpAddrAdded.u32Ip[j]    = currentIpAddr.u32Ip[j];
        }

        for(j=0;j<16;j++)
        {
            IPv6Val[j]    = currentIpAddr.arIP[j];
        }

        /* advance to the next IP address */
        if (IPv6Val[firstOctetToChange] + IPv6Step[firstOctetToChange] <= 255)
            currentIpAddr.arIP[firstOctetToChange] += IPv6Step[firstOctetToChange];
        else
        {
            currentIpAddr.arIP[firstOctetToChange] = 0;
            if (IPv6Val[secondOctetToChange] + IPv6Step[secondOctetToChange] <= 255)
                currentIpAddr.arIP[secondOctetToChange] += IPv6Step[secondOctetToChange];
            else
            {
                currentIpAddr.arIP[secondOctetToChange] = 0;
                if (IPv6Val[thirdOctetToChange] + IPv6Step[thirdOctetToChange] <= 255)
                    currentIpAddr.arIP[thirdOctetToChange] += IPv6Step[thirdOctetToChange];
                else
                {
                    currentIpAddr.arIP[thirdOctetToChange] = 0;
                    if (IPv6Val[fourthOctetToChange] + IPv6Step[fourthOctetToChange] <= 255)
                        currentIpAddr.arIP[fourthOctetToChange] += IPv6Step[fourthOctetToChange];
                    else
                    {
                        currentIpAddr.arIP[fourthOctetToChange] = 0;
                        if (IPv6Val[fifthOctetToChange] + IPv6Step[fifthOctetToChange] <= 255)
                            currentIpAddr.arIP[fifthOctetToChange] += IPv6Step[fifthOctetToChange];
                        else
                        {
                            currentIpAddr.arIP[fifthOctetToChange] = 0;
                            if (IPv6Val[sixthOctetToChange] + IPv6Step[sixthOctetToChange] <= 255)
                                currentIpAddr.arIP[sixthOctetToChange] += IPv6Step[sixthOctetToChange];
                            else
                            {
                                currentIpAddr.arIP[sixthOctetToChange] = 0;
                                if (IPv6Val[seventhOctetToChange] + IPv6Step[seventhOctetToChange] <= 255)
                                    currentIpAddr.arIP[seventhOctetToChange] += IPv6Step[seventhOctetToChange];
                                else
                                {
                                    currentIpAddr.arIP[seventhOctetToChange] = 0;
                                    if (IPv6Val[eighthOctetToChange] + IPv6Step[eighthOctetToChange] <= 255)
                                        currentIpAddr.arIP[eighthOctetToChange] += IPv6Step[eighthOctetToChange];
                                    else
                                    {
                                        currentIpAddr.arIP[eighthOctetToChange] = 0;
                                        if (IPv6Val[ninthOctetToChange] + IPv6Step[ninthOctetToChange] <= 255)
                                            currentIpAddr.arIP[ninthOctetToChange] += IPv6Step[ninthOctetToChange];
                                        else
                                        {
                                            currentIpAddr.arIP[ninthOctetToChange] = 0;
                                            if (IPv6Val[tenthOctetToChange] + IPv6Step[tenthOctetToChange] <= 255)
                                                currentIpAddr.arIP[tenthOctetToChange] += IPv6Step[tenthOctetToChange];
                                            else
                                            {
                                                currentIpAddr.arIP[tenthOctetToChange] = 0;
                                                if (IPv6Val[elevenOctetToChange] + IPv6Step[elevenOctetToChange] <= 255)
                                                    currentIpAddr.arIP[elevenOctetToChange] += IPv6Step[elevenOctetToChange];
                                                else
                                                {
                                                    currentIpAddr.arIP[elevenOctetToChange] = 0;
                                                    if (IPv6Val[twelveOctetToChange] + IPv6Step[twelveOctetToChange] <= 255)
                                                        currentIpAddr.arIP[twelveOctetToChange] += IPv6Step[twelveOctetToChange];
                                                    else
                                                    {
                                                        currentIpAddr.arIP[twelveOctetToChange] = 0;
                                                        if (IPv6Val[thirteenOctetToChange] + IPv6Step[thirteenOctetToChange] <= 255)
                                                            currentIpAddr.arIP[thirteenOctetToChange] += IPv6Step[thirteenOctetToChange];
                                                        else
                                                        {
                                                            currentIpAddr.arIP[thirteenOctetToChange] = 0;
                                                            if (IPv6Val[fourteenOctetToChange] + IPv6Step[fourteenOctetToChange] <= 255)
                                                                currentIpAddr.arIP[fourteenOctetToChange] += IPv6Step[fourteenOctetToChange];
                                                            else
                                                            {
                                                                currentIpAddr.arIP[fourteenOctetToChange] = 0;
                                                                if (IPv6Val[fifteenOctetToChange] + IPv6Step[fifteenOctetToChange] <= 255)
                                                                    currentIpAddr.arIP[fifteenOctetToChange] += IPv6Step[fifteenOctetToChange];
                                                                else
                                                                {
                                                                    currentIpAddr.arIP[fifteenOctetToChange] = 0;
                                                                    if (IPv6Val[sixteenOctetToChange] + IPv6Step[sixteenOctetToChange] <= 255)
                                                                        currentIpAddr.arIP[sixteenOctetToChange] += IPv6Step[sixteenOctetToChange];
                                                                    else
                                                                    {
                                                                        rc = GT_NO_MORE;
                                                                        break;
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    /* print result */
    cpssOsPrintf("\nIpv6UcPrefixAddMany results:");
    if (numOfPrefixesAdded == 0)
    {
        cpssOsPrintf("\n    No prefixes were added at all.\n");
    }
    else
    {
        cpssOsPrintf("\n    Prefixes ");
        outputIPAddress(startIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
        cpssOsPrintf(" - ");
        outputIPAddress(lastIpAddrAdded.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
        cpssOsPrintf(" were added (%d/%d)\n",numOfPrefixesAdded,numOfPrefixesToAdd);
        cpssOsPrintf("    (%d) MC prefixes not added in this range\n",numOfMcPrefixesNotAdded);
        cpssOsPrintf("    (%d) already existed \n",numOfPrefixesAlreadyExist);
        cpssOsPrintf("    total number of prefixes added plus existed (%d/%d) \n",(numOfPrefixesAlreadyExist+numOfPrefixesAdded),numOfPrefixesToAdd);

    }

    /* update output parameters */
    if (numOfPrefixesAddedPtr != NULL)
        *numOfPrefixesAddedPtr = numOfPrefixesAdded;
    if (lastIpAddrAddedPtr != NULL)
    {
        for(j=0;j<4;j++)
        {
            lastIpAddrAddedPtr->u32Ip[j] = lastIpAddrAdded.u32Ip[j];
        }
    }

    /* Return to default */
    for (i = 0; i < 16; i++) {
        IPv6Step[i] = 1;
        srcIPv6Step[i] = 1;

    }
    IPv6PrefixLength = 128;
    srcIPv6PrefixLength = 128;

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpLpmDbgIpv6UcPrefixAddManyByOctet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, lpmDbId, vrId, startIpAddr, routeEntryBaseMemAddr,
                            numOfPrefixesToAdd, firstOctetToChange, secondOctetToChange,
                            thirdOctetToChange, fourthOctetToChange, fifthOctetToChange,
                            sixthOctetToChange, seventhOctetToChange, eighthOctetToChange,
                            ninthOctetToChange, tenthOctetToChange, elevenOctetToChange,
                            twelveOctetToChange, thirteenOctetToChange, fourteenOctetToChange,
                            fifteenOctetToChange, sixteenOctetToChange, lastIpAddrAddedPtr,
                            numOfPrefixesAddedPtr));

    rc = internal_cpssDxChIpLpmDbgIpv6UcPrefixAddManyByOctet(lpmDbId, vrId, startIpAddr,
                                                            routeEntryBaseMemAddr,
                                                            numOfPrefixesToAdd,
                                                            firstOctetToChange,
                                                            secondOctetToChange,
                                                            thirdOctetToChange,
                                                            fourthOctetToChange,
                                                            fifthOctetToChange,
                                                            sixthOctetToChange,
                                                            seventhOctetToChange,
                                                            eighthOctetToChange,
                                                            ninthOctetToChange,
                                                            tenthOctetToChange,
                                                            elevenOctetToChange,
                                                            twelveOctetToChange,
                                                            thirteenOctetToChange,
                                                            fourteenOctetToChange,
                                                            fifteenOctetToChange,
                                                            sixteenOctetToChange,
                                                            lastIpAddrAddedPtr,
                                                            numOfPrefixesAddedPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, lpmDbId, vrId, startIpAddr, routeEntryBaseMemAddr,
                              numOfPrefixesToAdd, firstOctetToChange, secondOctetToChange,
                              thirdOctetToChange, fourthOctetToChange, fifthOctetToChange,
                              sixthOctetToChange, seventhOctetToChange, eighthOctetToChange,
                              ninthOctetToChange, tenthOctetToChange, elevenOctetToChange,
                              twelveOctetToChange, thirteenOctetToChange, fourteenOctetToChange,
                              fifteenOctetToChange, sixteenOctetToChange, lastIpAddrAddedPtr,
                              numOfPrefixesAddedPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpLpmDbgIpv6UcPrefixDelManyByOctet function
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
* @note This debug function is intended to delete the number of prefixes
*       that was added by cpssDxChIpLpmDbgIpv6UcPrefixAddManyByOctet.
*
*/
static GT_STATUS internal_cpssDxChIpLpmDbgIpv6UcPrefixDelManyByOctet
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
    OUT GT_IPV6ADDR *lastIpAddrDeletedPtr,
    OUT GT_U32      *numOfPrefixesDeletedPtr
)
{
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  i = 0;
    GT_U32                                  j = 0;
    GT_IPV6ADDR                             currentIpAddr;
    GT_U32                                  numOfPrefixesDeleted = 0;
    GT_U32                                  numOfPrefixesAlreadyDeleted = 0;
    GT_U32                                  numOfMcPrefixesNotDeleted = 0;
    GT_IPV6ADDR                             lastIpAddrDeleted;
    GT_BOOL                                 octetsArray[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS]={GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE,
                                                            GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE,
                                                            GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE,
                                                            GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE};/* array to hold the octets already selected */

    GT_U32                                  octetsOrderToChangeArray[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS];
    GT_U32                                  IPv6Val[16] = { 0 };

    cpssOsMemSet(&currentIpAddr, 0, sizeof(GT_IPV6ADDR));

    for(j=0;j<4;j++)
    {
        currentIpAddr.u32Ip[j]        = startIpAddr.u32Ip[j];
        lastIpAddrDeleted.u32Ip[j]    = startIpAddr.u32Ip[j];
    }


    octetsOrderToChangeArray[0] = firstOctetToChange;
    octetsOrderToChangeArray[1] = secondOctetToChange;
    octetsOrderToChangeArray[2] = thirdOctetToChange;
    octetsOrderToChangeArray[3] = fourthOctetToChange;
    octetsOrderToChangeArray[4] = fifthOctetToChange;
    octetsOrderToChangeArray[5] = sixthOctetToChange;
    octetsOrderToChangeArray[6] = seventhOctetToChange;
    octetsOrderToChangeArray[7] = eighthOctetToChange;
    octetsOrderToChangeArray[8] = ninthOctetToChange;
    octetsOrderToChangeArray[9] = tenthOctetToChange;
    octetsOrderToChangeArray[10] = elevenOctetToChange;
    octetsOrderToChangeArray[11] = twelveOctetToChange;
    octetsOrderToChangeArray[12] = thirteenOctetToChange;
    octetsOrderToChangeArray[13] = fourteenOctetToChange;
    octetsOrderToChangeArray[14] = fifteenOctetToChange;
    octetsOrderToChangeArray[15] = sixteenOctetToChange;

    for(j=0;j<PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS;j++)
    {
        if(octetsOrderToChangeArray[j]>15)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    for(j=0;j<PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS;j++)
    {
        /* make sure octet are not selected twice */
        if(octetsArray[octetsOrderToChangeArray[j]]==GT_FALSE)
        {
            octetsArray[octetsOrderToChangeArray[j]]=GT_TRUE;
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }


    for (i = 0 ; i < numOfPrefixesToDel ; )
    {
        /* make sure the prefix is not a MC prefix - not in format ff00::/8
           therefore the first octate must not start with 0xFF */
        if((currentIpAddr.arIP[0]) != 0xFF)
        {

            /* delete the current prefix */
            rc = cpssDxChIpLpmIpv6UcPrefixDel(lpmDbId,
                                              vrId,
                                              &currentIpAddr,
                                              IPv6PrefixLength);

            if ((rc != GT_OK) && (rc!=GT_NOT_FOUND))
                break;

            if (rc==GT_NOT_FOUND)
            {
                rc = GT_OK;
                numOfPrefixesAlreadyDeleted++;
            }
            else
                numOfPrefixesDeleted++;
            i++;
        }
        else
        {
            numOfMcPrefixesNotDeleted++;
        }

        for(j=0;j<4;j++)
        {
            lastIpAddrDeleted.u32Ip[j]    = currentIpAddr.u32Ip[j];
        }

        for(j=0;j<16;j++)
        {
            IPv6Val[j]    = currentIpAddr.arIP[j];
        }

        /* advance to the next IP address */
        if (IPv6Val[firstOctetToChange] + IPv6Step[firstOctetToChange] <= 255)
            currentIpAddr.arIP[firstOctetToChange] += IPv6Step[firstOctetToChange];
        else
        {
            currentIpAddr.arIP[firstOctetToChange] = 0;
            if (IPv6Val[secondOctetToChange] + IPv6Step[secondOctetToChange] <= 255)
                currentIpAddr.arIP[secondOctetToChange] += IPv6Step[secondOctetToChange];
            else
            {
                currentIpAddr.arIP[secondOctetToChange] = 0;
                if (IPv6Val[thirdOctetToChange] + IPv6Step[thirdOctetToChange] <= 255)
                    currentIpAddr.arIP[thirdOctetToChange] += IPv6Step[thirdOctetToChange];
                else
                {
                    currentIpAddr.arIP[thirdOctetToChange] = 0;
                    if (IPv6Val[fourthOctetToChange] + IPv6Step[fourthOctetToChange] <= 255)
                        currentIpAddr.arIP[fourthOctetToChange] += IPv6Step[fourthOctetToChange];
                    else
                    {
                        currentIpAddr.arIP[fourthOctetToChange] = 0;
                        if (IPv6Val[fifthOctetToChange] + IPv6Step[fifthOctetToChange] <= 255)
                            currentIpAddr.arIP[fifthOctetToChange] += IPv6Step[fifthOctetToChange];
                        else
                        {
                            currentIpAddr.arIP[fifthOctetToChange] = 0;
                            if (IPv6Val[sixthOctetToChange] + IPv6Step[sixthOctetToChange] <= 255)
                                currentIpAddr.arIP[sixthOctetToChange] += IPv6Step[sixthOctetToChange];
                            else
                            {
                                currentIpAddr.arIP[sixthOctetToChange] = 0;
                                if (IPv6Val[seventhOctetToChange] + IPv6Step[seventhOctetToChange] <= 255)
                                    currentIpAddr.arIP[seventhOctetToChange] += IPv6Step[seventhOctetToChange];
                                else
                                {
                                    currentIpAddr.arIP[seventhOctetToChange] = 0;
                                    if (IPv6Val[eighthOctetToChange] + IPv6Step[eighthOctetToChange] <= 255)
                                        currentIpAddr.arIP[eighthOctetToChange] += IPv6Step[eighthOctetToChange];
                                    else
                                    {
                                        currentIpAddr.arIP[eighthOctetToChange] = 0;
                                        if (IPv6Val[ninthOctetToChange] + IPv6Step[ninthOctetToChange] <= 255)
                                            currentIpAddr.arIP[ninthOctetToChange] += IPv6Step[ninthOctetToChange];
                                        else
                                        {
                                            currentIpAddr.arIP[ninthOctetToChange] = 0;
                                            if (IPv6Val[tenthOctetToChange] + IPv6Step[tenthOctetToChange] <= 255)
                                                currentIpAddr.arIP[tenthOctetToChange] += IPv6Step[tenthOctetToChange];
                                            else
                                            {
                                                currentIpAddr.arIP[tenthOctetToChange] = 0;
                                                if (IPv6Val[elevenOctetToChange] + IPv6Step[elevenOctetToChange] <= 255)
                                                    currentIpAddr.arIP[elevenOctetToChange] += IPv6Step[elevenOctetToChange];
                                                else
                                                {
                                                    currentIpAddr.arIP[elevenOctetToChange] = 0;
                                                    if (IPv6Val[twelveOctetToChange] + IPv6Step[twelveOctetToChange] <= 255)
                                                        currentIpAddr.arIP[twelveOctetToChange] += IPv6Step[twelveOctetToChange];
                                                    else
                                                    {
                                                        currentIpAddr.arIP[twelveOctetToChange] = 0;
                                                        if (IPv6Val[thirteenOctetToChange] + IPv6Step[thirteenOctetToChange] <= 255)
                                                            currentIpAddr.arIP[thirteenOctetToChange] += IPv6Step[thirteenOctetToChange];
                                                        else
                                                        {
                                                            currentIpAddr.arIP[thirteenOctetToChange] = 0;
                                                            if (IPv6Val[fourteenOctetToChange] + IPv6Step[fourteenOctetToChange] <= 255)
                                                                currentIpAddr.arIP[fourteenOctetToChange] += IPv6Step[fourteenOctetToChange];
                                                            else
                                                            {
                                                                currentIpAddr.arIP[fourteenOctetToChange] = 0;
                                                                if (IPv6Val[fifteenOctetToChange] + IPv6Step[fifteenOctetToChange] <= 255)
                                                                    currentIpAddr.arIP[fifteenOctetToChange] += IPv6Step[fifteenOctetToChange];
                                                                else
                                                                {
                                                                    currentIpAddr.arIP[fifteenOctetToChange] = 0;
                                                                    if (IPv6Val[sixteenOctetToChange] + IPv6Step[sixteenOctetToChange] <= 255)
                                                                        currentIpAddr.arIP[sixteenOctetToChange] += IPv6Step[sixteenOctetToChange];
                                                                    else
                                                                    {
                                                                        rc = GT_NO_MORE;
                                                                        break;
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    /* print result */
    cpssOsPrintf("\nIpv6UcPrefixDelMany results:");
    if (numOfPrefixesDeleted == 0)
    {
        cpssOsPrintf("\n    No prefixes were Deleted at all.\n");
    }
    else
    {
        cpssOsPrintf("\n    Prefixes ");
        outputIPAddress(startIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
        cpssOsPrintf(" - ");
        outputIPAddress(lastIpAddrDeleted.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
        cpssOsPrintf(" were Deleted (%d/%d)\n",numOfPrefixesDeleted,numOfPrefixesToDel);
        cpssOsPrintf("    (%d) MC prefixes not Deleted in this range\n",numOfMcPrefixesNotDeleted);
        cpssOsPrintf("    (%d) already deleted \n",numOfPrefixesAlreadyDeleted);
        cpssOsPrintf("    total number of prefixes deleted plus not_found (%d/%d) \n",(numOfPrefixesAlreadyDeleted+numOfPrefixesDeleted),numOfPrefixesToDel);

    }

    /* update output parameters */
    if (numOfPrefixesDeletedPtr != NULL)
        *numOfPrefixesDeletedPtr = numOfPrefixesDeleted;
    if (lastIpAddrDeletedPtr != NULL)
    {
        for(j=0;j<4;j++)
        {
            lastIpAddrDeletedPtr->u32Ip[j] = lastIpAddrDeleted.u32Ip[j];
        }
    }

    /* Return to default */
    for (i = 0; i < 16; i++) {
        IPv6Step[i] = 1;
        srcIPv6Step[i] = 1;
    }
    IPv6PrefixLength = 128;
    srcIPv6PrefixLength = 128;

    return rc;
}

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
    OUT GT_IPV6ADDR *lastIpAddrDeletedPtr,
    OUT GT_U32      *numOfPrefixesDeletedPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpLpmDbgIpv6UcPrefixDelManyByOctet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, lpmDbId, vrId, startIpAddr, numOfPrefixesToDel,
                            firstOctetToChange, secondOctetToChange, thirdOctetToChange,
                            fourthOctetToChange, fifthOctetToChange, sixthOctetToChange,
                            seventhOctetToChange, eighthOctetToChange, ninthOctetToChange,
                            tenthOctetToChange, elevenOctetToChange, twelveOctetToChange,
                            thirteenOctetToChange, fourteenOctetToChange, fifteenOctetToChange,
                            sixteenOctetToChange, lastIpAddrDeletedPtr, numOfPrefixesDeletedPtr));

    rc = internal_cpssDxChIpLpmDbgIpv6UcPrefixDelManyByOctet(lpmDbId, vrId, startIpAddr,
                                                              numOfPrefixesToDel,
                                                              firstOctetToChange,
                                                              secondOctetToChange,
                                                              thirdOctetToChange,
                                                              fourthOctetToChange,
                                                              fifthOctetToChange,
                                                              sixthOctetToChange,
                                                              seventhOctetToChange,
                                                              eighthOctetToChange,
                                                              ninthOctetToChange,
                                                              tenthOctetToChange,
                                                              elevenOctetToChange,
                                                              twelveOctetToChange,
                                                              thirteenOctetToChange,
                                                              fourteenOctetToChange,
                                                              fifteenOctetToChange,
                                                              sixteenOctetToChange,
                                                              lastIpAddrDeletedPtr,
                                                              numOfPrefixesDeletedPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, lpmDbId, vrId, startIpAddr, numOfPrefixesToDel,
                              firstOctetToChange, secondOctetToChange, thirdOctetToChange,
                              fourthOctetToChange, fifthOctetToChange, sixthOctetToChange,
                              seventhOctetToChange, eighthOctetToChange, ninthOctetToChange,
                              tenthOctetToChange, elevenOctetToChange, twelveOctetToChange,
                              thirteenOctetToChange, fourteenOctetToChange, fifteenOctetToChange,
                              sixteenOctetToChange, lastIpAddrDeletedPtr, numOfPrefixesDeletedPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpLpmDbgIpv6UcPrefixGetManyByOctet function
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
static GT_STATUS internal_cpssDxChIpLpmDbgIpv6UcPrefixGetManyByOctet
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
)
{
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  i = 0;
    GT_U32                                  j = 0;
    GT_IPV6ADDR                             currentIpAddr;
    GT_U32                                  numOfRetrievedPrefixes = 0;
    GT_U32                                  numOfMcPrefixesNotAdded = 0;
    GT_IPV6ADDR                             lastIpAddrAdded;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT  nextHopInfo;
    GT_BOOL                                 octetsArray[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS]={GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE,
                                                            GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE,
                                                            GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE,
                                                            GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE};/* array to hold the octets already selected */

    GT_U32                                  octetsOrderToChangeArray[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS];
    GT_U32                                  tcamRowIndex=0;
    GT_U32                                  tcamColumnIndex=0;
    GT_U32                                  IPv6Val[16] = { 0 };

    cpssOsMemSet(&nextHopInfo, 0, sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
    cpssOsMemSet(&currentIpAddr, 0, sizeof(GT_IPV6ADDR));

    for(j=0;j<4;j++)
    {
        currentIpAddr.u32Ip[j]     = startIpAddr.u32Ip[j];
        lastIpAddrAdded.u32Ip[j]    = startIpAddr.u32Ip[j];
    }

    octetsOrderToChangeArray[0] = firstOctetToChange;
    octetsOrderToChangeArray[1] = secondOctetToChange;
    octetsOrderToChangeArray[2] = thirdOctetToChange;
    octetsOrderToChangeArray[3] = fourthOctetToChange;
    octetsOrderToChangeArray[4] = fifthOctetToChange;
    octetsOrderToChangeArray[5] = sixthOctetToChange;
    octetsOrderToChangeArray[6] = seventhOctetToChange;
    octetsOrderToChangeArray[7] = eighthOctetToChange;
    octetsOrderToChangeArray[8] = ninthOctetToChange;
    octetsOrderToChangeArray[9] = tenthOctetToChange;
    octetsOrderToChangeArray[10] = elevenOctetToChange;
    octetsOrderToChangeArray[11] = twelveOctetToChange;
    octetsOrderToChangeArray[12] = thirteenOctetToChange;
    octetsOrderToChangeArray[13] = fourteenOctetToChange;
    octetsOrderToChangeArray[14] = fifteenOctetToChange;
    octetsOrderToChangeArray[15] = sixteenOctetToChange;

    for(j=0;j<PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS;j++)
    {
        if(octetsOrderToChangeArray[j]>15)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    for(j=0;j<PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS;j++)
    {
        /* make sure octet are not selected twice */
        if(octetsArray[octetsOrderToChangeArray[j]]==GT_FALSE)
        {
            octetsArray[octetsOrderToChangeArray[j]]=GT_TRUE;
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    for (i = 0 ; i < numOfPrefixesToGet ; )
    {
        /* make sure the prefix is not a MC prefix - not in format ff00::/8
           therefore the first octate must not start with 0xFF */
        if((currentIpAddr.arIP[0]) != 0xFF)
        {

            /* get the current prefix */
            rc =  cpssDxChIpLpmIpv6UcPrefixSearch(lpmDbId,
                                                 vrId,
                                                 &currentIpAddr,
                                                 IPv6PrefixLength,
                                                 &nextHopInfo,
                                                 &tcamRowIndex,
                                                 &tcamColumnIndex);

            if (rc != GT_OK)
                break;

            numOfRetrievedPrefixes++;
            i++;
        }
        else
        {
            numOfMcPrefixesNotAdded++;
        }

        for(j=0;j<4;j++)
        {
            lastIpAddrAdded.u32Ip[j]    = currentIpAddr.u32Ip[j];
        }


        for(j=0;j<16;j++)
        {
            IPv6Val[j]    = currentIpAddr.arIP[j];
        }


        /* advance to the next IP address */
        if (IPv6Val[firstOctetToChange] + IPv6Step[firstOctetToChange] <= 255)
            currentIpAddr.arIP[firstOctetToChange] += IPv6Step[firstOctetToChange];
        else
        {
            currentIpAddr.arIP[firstOctetToChange] = 0;
            if (IPv6Val[secondOctetToChange] + IPv6Step[secondOctetToChange] <= 255)
                currentIpAddr.arIP[secondOctetToChange] += IPv6Step[secondOctetToChange];
            else
            {
                currentIpAddr.arIP[secondOctetToChange] = 0;
                if (IPv6Val[thirdOctetToChange] + IPv6Step[thirdOctetToChange] <= 255)
                    currentIpAddr.arIP[thirdOctetToChange] += IPv6Step[thirdOctetToChange];
                else
                {
                    currentIpAddr.arIP[thirdOctetToChange] = 0;
                    if (IPv6Val[fourthOctetToChange] + IPv6Step[fourthOctetToChange] <= 255)
                        currentIpAddr.arIP[fourthOctetToChange] += IPv6Step[fourthOctetToChange];
                    else
                    {
                        currentIpAddr.arIP[fourthOctetToChange] = 0;
                        if (IPv6Val[fifthOctetToChange] + IPv6Step[fifthOctetToChange] <= 255)
                            currentIpAddr.arIP[fifthOctetToChange] += IPv6Step[fifthOctetToChange];
                        else
                        {
                            currentIpAddr.arIP[fifthOctetToChange] = 0;
                            if (IPv6Val[sixthOctetToChange] + IPv6Step[sixthOctetToChange] <= 255)
                                currentIpAddr.arIP[sixthOctetToChange] += IPv6Step[sixthOctetToChange];
                            else
                            {
                                currentIpAddr.arIP[sixthOctetToChange] = 0;
                                if (IPv6Val[seventhOctetToChange] + IPv6Step[seventhOctetToChange] <= 255)
                                    currentIpAddr.arIP[seventhOctetToChange] += IPv6Step[seventhOctetToChange];
                                else
                                {
                                    currentIpAddr.arIP[seventhOctetToChange] = 0;
                                    if (IPv6Val[eighthOctetToChange] + IPv6Step[eighthOctetToChange] <= 255)
                                        currentIpAddr.arIP[eighthOctetToChange] += IPv6Step[eighthOctetToChange];
                                    else
                                    {
                                        currentIpAddr.arIP[eighthOctetToChange] = 0;
                                        if (IPv6Val[ninthOctetToChange] + IPv6Step[ninthOctetToChange] <= 255)
                                            currentIpAddr.arIP[ninthOctetToChange] += IPv6Step[ninthOctetToChange];
                                        else
                                        {
                                            currentIpAddr.arIP[ninthOctetToChange] = 0;
                                            if (IPv6Val[tenthOctetToChange] + IPv6Step[tenthOctetToChange] <= 255)
                                                currentIpAddr.arIP[tenthOctetToChange] += IPv6Step[tenthOctetToChange];
                                            else
                                            {
                                                currentIpAddr.arIP[tenthOctetToChange] = 0;
                                                if (IPv6Val[elevenOctetToChange] + IPv6Step[elevenOctetToChange] <= 255)
                                                    currentIpAddr.arIP[elevenOctetToChange] += IPv6Step[elevenOctetToChange];
                                                else
                                                {
                                                    currentIpAddr.arIP[elevenOctetToChange] = 0;
                                                    if (IPv6Val[twelveOctetToChange] + IPv6Step[twelveOctetToChange] <= 255)
                                                        currentIpAddr.arIP[twelveOctetToChange] += IPv6Step[twelveOctetToChange];
                                                    else
                                                    {
                                                        currentIpAddr.arIP[twelveOctetToChange] = 0;
                                                        if (IPv6Val[thirteenOctetToChange] + IPv6Step[thirteenOctetToChange] <= 255)
                                                            currentIpAddr.arIP[thirteenOctetToChange] += IPv6Step[thirteenOctetToChange];
                                                        else
                                                        {
                                                            currentIpAddr.arIP[thirteenOctetToChange] = 0;
                                                            if (IPv6Val[fourteenOctetToChange] + IPv6Step[fourteenOctetToChange] <= 255)
                                                                currentIpAddr.arIP[fourteenOctetToChange] += IPv6Step[fourteenOctetToChange];
                                                            else
                                                            {
                                                                currentIpAddr.arIP[fourteenOctetToChange] = 0;
                                                                if (IPv6Val[fifteenOctetToChange] + IPv6Step[fifteenOctetToChange] <= 255)
                                                                    currentIpAddr.arIP[fifteenOctetToChange] += IPv6Step[fifteenOctetToChange];
                                                                else
                                                                {
                                                                    currentIpAddr.arIP[fifteenOctetToChange] = 0;
                                                                    if (IPv6Val[sixteenOctetToChange] + IPv6Step[sixteenOctetToChange] <= 255)
                                                                        currentIpAddr.arIP[sixteenOctetToChange] += IPv6Step[sixteenOctetToChange];
                                                                    else
                                                                    {
                                                                        rc = GT_NO_MORE;
                                                                        break;
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    /* print result */
    cpssOsPrintf("\nIpv6UcPrefixGetMany results:");
    if (numOfRetrievedPrefixes == 0)
    {
        cpssOsPrintf("\n    No prefixes at all.\n");
    }
    else
    {
        cpssOsPrintf("\n    Prefixes ");
        outputIPAddress(startIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
        cpssOsPrintf(" - ");
        outputIPAddress(lastIpAddrAdded.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
        cpssOsPrintf(" were retrieved (%d/%d)\n",numOfRetrievedPrefixes,numOfPrefixesToGet);
        cpssOsPrintf("    (%d) MC prefixes not added in this range\n",numOfMcPrefixesNotAdded);
    }

    /* update output parameters */
    if (numOfRetrievedPrefixesPtr != NULL)
        *numOfRetrievedPrefixesPtr = numOfRetrievedPrefixes;


    /* Return to default */
    for (i = 0; i < 16; i++) {
        IPv6Step[i] = 1;
        srcIPv6Step[i] = 1;

    }
    IPv6PrefixLength = 128;
    srcIPv6PrefixLength = 128;

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpLpmDbgIpv6UcPrefixGetManyByOctet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, lpmDbId, vrId, startIpAddr,
                            numOfPrefixesToGet, firstOctetToChange, secondOctetToChange,
                            thirdOctetToChange, fourthOctetToChange, fifthOctetToChange,
                            sixthOctetToChange, seventhOctetToChange, eighthOctetToChange,
                            ninthOctetToChange, tenthOctetToChange, elevenOctetToChange,
                            twelveOctetToChange, thirteenOctetToChange, fourteenOctetToChange,
                            fifteenOctetToChange, sixteenOctetToChange, numOfRetrievedPrefixesPtr));

    rc = internal_cpssDxChIpLpmDbgIpv6UcPrefixGetManyByOctet(lpmDbId, vrId, startIpAddr,
                                                             numOfPrefixesToGet,
                                                             firstOctetToChange,
                                                             secondOctetToChange,
                                                             thirdOctetToChange,
                                                             fourthOctetToChange,
                                                             fifthOctetToChange,
                                                             sixthOctetToChange,
                                                             seventhOctetToChange,
                                                             eighthOctetToChange,
                                                             ninthOctetToChange,
                                                             tenthOctetToChange,
                                                             elevenOctetToChange,
                                                             twelveOctetToChange,
                                                             thirteenOctetToChange,
                                                             fourteenOctetToChange,
                                                             fifteenOctetToChange,
                                                             sixteenOctetToChange,
                                                             numOfRetrievedPrefixesPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, lpmDbId, vrId, startIpAddr,
                              numOfPrefixesToGet, firstOctetToChange, secondOctetToChange,
                              thirdOctetToChange, fourthOctetToChange, fifthOctetToChange,
                              sixthOctetToChange, seventhOctetToChange, eighthOctetToChange,
                              ninthOctetToChange, tenthOctetToChange, elevenOctetToChange,
                              twelveOctetToChange, thirteenOctetToChange, fourteenOctetToChange,
                              fifteenOctetToChange, sixteenOctetToChange, numOfRetrievedPrefixesPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpLpmDbgIpv6UcPrefixAddRandomByRange function
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
static GT_STATUS internal_cpssDxChIpLpmDbgIpv6UcPrefixAddRandomByRange
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
)
{
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  i = 0, j = 0;
    GT_IPV6ADDR                             currentIpAddr;
    GT_U32                                  numOfPrefixesAdded = 0;
    GT_U32                                  numOfMcPrefixesNotAdded = 0;
    GT_U32                                  numOfPrefixesAlreadyExist = 0;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT  nextHopInfo;
    GT_BOOL                                 override = GT_FALSE;
    GT_U32                                  numOfIterations = 100000;
    GT_BOOL                                 tcamDefragmentationEnable=GT_FALSE;
    GT_IPV6ADDR                             lastIpAddrAdded;
    GT_U32                                  u32_startIpAddr = 0,u32_lastIpAddr = 0, u32_calc = 0,mod;

    step = step; /* TBD will be used in phase 2 */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
    nextHopInfo.ipLttEntry.routeType           = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = routeEntryBaseMemAddr;
    nextHopInfo.ipLttEntry.numOfPaths          = 0;
    nextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;
    rc = prvCpssDxChIpLpmDbgConvertRouteType(lpmDbId, &nextHopInfo.ipLttEntry);
    if (rc != GT_OK)
    {
        return rc;
    }

    cpssOsMemSet(&currentIpAddr, 0, sizeof(GT_IPV6ADDR));

    for(j=0;j<3;j++)
    {
        currentIpAddr.u32Ip[j]     = startIpAddr.u32Ip[j];
        lastIpAddrAdded.u32Ip[j]   = currentIpAddr.u32Ip[j];
    }

    u32_startIpAddr = prvCpssDxChIpLpmDbgIpSwap(startIpAddr.u32Ip[3]);
    u32_lastIpAddr = prvCpssDxChIpLpmDbgIpSwap(lastIpAddr.u32Ip[3]);
    mod = u32_lastIpAddr - u32_startIpAddr + 1;
    if (mod == 0)
    {
        u32_calc = cpssOsRand() + u32_startIpAddr;   /* whole range FFFF:FFFF */
    }
    else
    {
        u32_calc = (cpssOsRand() % mod) + u32_startIpAddr;
    }

    currentIpAddr.u32Ip[3] = prvCpssDxChIpLpmDbgIpSwap(u32_calc);
    lastIpAddrAdded.u32Ip[3]   = currentIpAddr.u32Ip[3];

    for (i = 0 ;i < numOfIterations ;i++)
    {
         /* make sure the prefix is not a MC prefix - not in format ff00::/8
           therefore the first octate must not start with 0xFF */
        if((currentIpAddr.arIP[0]) != 0xFF)
        {
            if (prvCpssDxChIpLpmDbgIsAddrInRange(&currentIpAddr,&startIpAddr,&lastIpAddr))
            {
                /* add the current prefix */
                rc = cpssDxChIpLpmIpv6UcPrefixAdd(lpmDbId,
                                                  vrId,
                                                  &currentIpAddr,
                                                  IPv6PrefixLength,
                                                  &nextHopInfo,
                                                  override,
                                                  tcamDefragmentationEnable);

                if (returnOnFirstFail && (rc != GT_OK) && (rc!=GT_ALREADY_EXIST))
                    break;

                if (rc==GT_ALREADY_EXIST)
                {
                    rc = GT_OK;
                    numOfPrefixesAlreadyExist++;
                }
                if (rc == GT_OK)
                    numOfPrefixesAdded++;
            }
        }
        else
        {
            numOfMcPrefixesNotAdded++;
        }

        if (mod == 0)
        {
            u32_calc = cpssOsRand() + u32_startIpAddr;   /* whole range FFFF:FFFF */
        }
        else
        {
            u32_calc = (cpssOsRand() % mod) + u32_startIpAddr;
        }

        currentIpAddr.u32Ip[3] = prvCpssDxChIpLpmDbgIpSwap(u32_calc);
        lastIpAddrAdded.u32Ip[3]   = currentIpAddr.u32Ip[3];
    }

    /* print result */
    cpssOsPrintf("\nIpv6UcPrefixAddRandomByRange results:");
    if (rc != GT_OK)
    {
        cpssOsPrintf("\n error = %x occured on prefix\n",rc);
        outputIPAddress(currentIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
    }

    if (numOfPrefixesAdded == 0)
    {
        cpssOsPrintf("\n    No prefixes were added at all.\n");
    }
    else
    {
        cpssOsPrintf("\n    Prefixes ");
        outputIPAddress(startIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
        cpssOsPrintf(" - ");
        outputIPAddress(lastIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
        cpssOsPrintf(" were added (%d/%d)\n",numOfPrefixesAdded,numOfIterations);
        cpssOsPrintf("    (%d) MC prefixes not added in this range\n",numOfMcPrefixesNotAdded);
    }

    /* update output parameters */
    if (numOfPrefixesAddedPtr != NULL)
        *numOfPrefixesAddedPtr = numOfPrefixesAdded;
    if (lastIpAddrAddedPtr != NULL)
    {
        for(i=0;i<4;i++)
        {
            lastIpAddrAddedPtr->u32Ip[i] = lastIpAddrAdded.u32Ip[i];
        }
    }
    return rc;

}
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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpLpmDbgIpv6UcPrefixAddRandomByRange);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, lpmDbId, vrId, startIpAddr, lastIpAddr, routeEntryBaseMemAddr,
                            returnOnFirstFail, step, lastIpAddrAddedPtr, numOfPrefixesAddedPtr));

    rc = internal_cpssDxChIpLpmDbgIpv6UcPrefixAddRandomByRange(lpmDbId, vrId, startIpAddr, lastIpAddr,
                                                            routeEntryBaseMemAddr,returnOnFirstFail,
                                                            step, lastIpAddrAddedPtr, numOfPrefixesAddedPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, lpmDbId, vrId, startIpAddr, lastIpAddr, routeEntryBaseMemAddr,
                              returnOnFirstFail, step, lastIpAddrAddedPtr, numOfPrefixesAddedPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpLpmDbgIpv4McPrefixAddManyByOctet function
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
static GT_STATUS internal_cpssDxChIpLpmDbgIpv4McPrefixAddManyByOctet
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
)
{
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  i = 0;
    GT_U32                                  j = 0;
    GT_IPADDR                               grpCurrentIpAddr;
    GT_IPADDR                               srcCurrentIpAddr;
    GT_U32                                  numOfPrefixesAdded = 0;
    GT_U32                                  numOfPrefixesAlreadyExist = 0;
    GT_IPADDR                               grpLastIpAddrAdded;
    GT_IPADDR                               srcLastIpAddrAdded;
    CPSS_DXCH_IP_LTT_ENTRY_STC              mcRouteLttEntry;
    GT_BOOL                                 override = GT_FALSE;
    GT_BOOL                                 octetsArray[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS]={GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE};/* array to hold the octets already selected */
    GT_U32                                  octetsOrderToChangeArray[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS];
    GT_BOOL                                 defragmentationEnable=defragmentationEnableFlag;
    GT_U32                                  srcIPv4Val[4] = { 0 };
    GT_U32                                  grpIPv4Val[4] = { 0 };
    PRV_CPSS_DXCH_LPM_SHADOW_STC            *lpmDbPtr,tmpLpmDb;

    tmpLpmDb.lpmDbId = lpmDbId;
    lpmDbPtr = prvCpssSlSearch(prvCpssDxChIplpmDbSL,&tmpLpmDb);
    if (lpmDbPtr == NULL)
    {
        /* can't find the lpm DB */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }


    srcCurrentIpAddr.u32Ip                  = srcStartIpAddr.u32Ip;
    srcLastIpAddrAdded.u32Ip                = srcStartIpAddr.u32Ip;

    grpCurrentIpAddr.u32Ip                  = grpStartIpAddr.u32Ip;
    grpLastIpAddrAdded.u32Ip                = grpStartIpAddr.u32Ip;

    cpssOsMemSet(&mcRouteLttEntry, 0, sizeof(CPSS_DXCH_IP_LTT_ENTRY_STC));

    mcRouteLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
    if ((lpmDbPtr->shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E) ||
        (lpmDbPtr->shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP5_SHADOW_E) )
    {
        mcRouteLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
        mcRouteLttEntry.priority = CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_LPM_E;
    }
    mcRouteLttEntry.numOfPaths = 0;
    mcRouteLttEntry.routeEntryBaseIndex = routeEntryBaseMemAddr;
    mcRouteLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

    if(((changeGrpAddr==GT_TRUE)&&(changeSrcAddr==GT_TRUE))||
       ((changeGrpAddr==GT_FALSE)&&(changeSrcAddr==GT_FALSE)))
    {
        cpssOsPrintf("changeGrpAddr should be different from changeSrcAddr\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    if(changeGrpAddr==GT_TRUE)
    {
        octetsOrderToChangeArray[0] = grpFirstOctetToChange;
        octetsOrderToChangeArray[1] = grpSecondOctetToChange;
        octetsOrderToChangeArray[2] = grpThirdOctetToChange;
        octetsOrderToChangeArray[3] = grpFourthOctetToChange;
    }
    if(changeSrcAddr==GT_TRUE)
    {
        octetsOrderToChangeArray[0] = srcFirstOctetToChange;
        octetsOrderToChangeArray[1] = srcSecondOctetToChange;
        octetsOrderToChangeArray[2] = srcThirdOctetToChange;
        octetsOrderToChangeArray[3] = srcFourthOctetToChange;
    }

    for(j=0;j<PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS;j++)
    {
        if(octetsOrderToChangeArray[j]>3)
        {
            cpssOsPrintf("octetToChange can not be bigger then 3\n");
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    for(j=0;j<PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS;j++)
    {
        /* make sure octet are not selected twice */
        if(octetsArray[octetsOrderToChangeArray[j]]==GT_FALSE)
        {
            octetsArray[octetsOrderToChangeArray[j]]=GT_TRUE;
        }
        else
        {
            cpssOsPrintf("octetsOrderToChangeArray[j]=%d was selected twice\n",octetsOrderToChangeArray[j]);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    for (i = 0 ; i < numOfPrefixesToAdd ; )
    {
        /* make sure the grp prefix is a MC prefix - in the range 224/4 - 239/4
           therefore the first octate must start with 0xE
           and the src prefix is not a MC prefix */
        if(((grpCurrentIpAddr.arIP[0] & 0xF0) == 0xE0)&&
           ((srcCurrentIpAddr.arIP[0] & 0xF0) != 0xE0))
        {
            /* add the current prefix */
            rc = cpssDxChIpLpmIpv4McEntryAdd(lpmDbId,
                                             vrId,
                                             &grpCurrentIpAddr,
                                             IPv4PrefixLength,
                                             &srcCurrentIpAddr,
                                             srcIPv4PrefixLength,
                                             &mcRouteLttEntry,
                                             override,
                                             defragmentationEnable);

            if ((rc != GT_OK) && (rc!=GT_ALREADY_EXIST))
                break;

            if (rc==GT_ALREADY_EXIST)
            {
                rc = GT_OK;
                numOfPrefixesAlreadyExist++;
            }
            else
                numOfPrefixesAdded++;

            i++;
        }
        else
        {
            cpssOsPrintf("\n grpStartIpAddr= ");
            outputIPAddress(grpCurrentIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
            cpssOsPrintf(" srcCurrentIpAddr= ");
            outputIPAddress(srcCurrentIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
            cpssOsPrintf(" is not a valid MC address\n");

            /* print result */
            cpssOsPrintf("\nIpv4McPrefixAddMany results:");
            if (numOfPrefixesAdded == 0)
            {
                cpssOsPrintf("\n    No prefixes were added at all.\n");
            }
            else
            {
                cpssOsPrintf("\n    Group Prefixes ");
                outputIPAddress(grpStartIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
                cpssOsPrintf(" - ");
                outputIPAddress(grpLastIpAddrAdded.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
                cpssOsPrintf("\n    Source Prefixes ");
                outputIPAddress(srcStartIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
                cpssOsPrintf(" - ");
                outputIPAddress(srcLastIpAddrAdded.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
                cpssOsPrintf(" were added (%d/%d)\n",numOfPrefixesAdded,numOfPrefixesToAdd);
                cpssOsPrintf("    (%d) already existed \n",numOfPrefixesAlreadyExist);
                cpssOsPrintf("    total number of prefixes added plus existed (%d/%d) \n",(numOfPrefixesAlreadyExist+numOfPrefixesAdded),numOfPrefixesToAdd);

            }

            /* update output parameters */
            if (numOfPrefixesAddedPtr != NULL)
                *numOfPrefixesAddedPtr = numOfPrefixesAdded;
            if (grpLastIpAddrAddedPtr != NULL)
                grpLastIpAddrAddedPtr->u32Ip = grpLastIpAddrAdded.u32Ip;
            if (srcLastIpAddrAddedPtr != NULL)
                srcLastIpAddrAddedPtr->u32Ip = srcLastIpAddrAdded.u32Ip;

            /* Return to default */
            for (i = 0; i < 4; i++) {
                IPv4Step[i] = 1;
                srcIPv4Step[i] = 1;
            }
            IPv4PrefixLength = 32;
            srcIPv4PrefixLength = 32;

            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }


        srcLastIpAddrAdded.u32Ip                = srcCurrentIpAddr.u32Ip;
        grpLastIpAddrAdded.u32Ip                = grpCurrentIpAddr.u32Ip;

        srcIPv4Val[3]=srcCurrentIpAddr.arIP[3];
        srcIPv4Val[2]=srcCurrentIpAddr.arIP[2];
        srcIPv4Val[1]=srcCurrentIpAddr.arIP[1];
        srcIPv4Val[0]=srcCurrentIpAddr.arIP[0];

        grpIPv4Val[3]=grpCurrentIpAddr.arIP[3];
        grpIPv4Val[2]=grpCurrentIpAddr.arIP[2];
        grpIPv4Val[1]=grpCurrentIpAddr.arIP[1];
        grpIPv4Val[0]=grpCurrentIpAddr.arIP[0];

        if(changeSrcAddr==GT_TRUE)
        {
            /* advance to the next IP address */
            if (srcIPv4Val[srcFirstOctetToChange] + srcIPv4Step[srcFirstOctetToChange] <= 255)
                srcCurrentIpAddr.arIP[srcFirstOctetToChange] += srcIPv4Step[srcFirstOctetToChange];
            else
            {
                srcCurrentIpAddr.arIP[srcFirstOctetToChange] = 0;
                if (srcIPv4Val[srcSecondOctetToChange] + srcIPv4Step[srcSecondOctetToChange] <= 255)
                    srcCurrentIpAddr.arIP[srcSecondOctetToChange] += srcIPv4Step[srcSecondOctetToChange];
                else
                {
                    srcCurrentIpAddr.arIP[srcSecondOctetToChange] = 0;
                    if (srcIPv4Val[srcThirdOctetToChange] + srcIPv4Step[srcThirdOctetToChange] <= 255)
                        srcCurrentIpAddr.arIP[srcThirdOctetToChange] += srcIPv4Step[srcThirdOctetToChange];
                    else
                    {
                        srcCurrentIpAddr.arIP[srcThirdOctetToChange] = 0;
                        if (srcIPv4Val[srcFourthOctetToChange] + srcIPv4Step[srcFourthOctetToChange] <= 255)
                            srcCurrentIpAddr.arIP[srcFourthOctetToChange] += srcIPv4Step[srcFourthOctetToChange];
                        else
                        {
                            rc = GT_NO_MORE;
                            break;
                        }
                    }
                }
            }
        }
        else /* changeGrpAddr==GT_TRUE*/
        {
            /* advance to the next IP address */
            if (grpIPv4Val[grpFirstOctetToChange] + IPv4Step[grpFirstOctetToChange] <= 255)
                grpCurrentIpAddr.arIP[grpFirstOctetToChange] += IPv4Step[grpFirstOctetToChange];
            else
            {
                grpCurrentIpAddr.arIP[grpFirstOctetToChange] = 0;
                if (grpIPv4Val[grpSecondOctetToChange] + IPv4Step[grpSecondOctetToChange] <= 255)
                    grpCurrentIpAddr.arIP[grpSecondOctetToChange] += IPv4Step[grpSecondOctetToChange];
                else
                {
                    grpCurrentIpAddr.arIP[grpSecondOctetToChange] = 0;
                    if (grpIPv4Val[grpThirdOctetToChange] + IPv4Step[grpThirdOctetToChange] <= 255)
                        grpCurrentIpAddr.arIP[grpThirdOctetToChange] += IPv4Step[grpThirdOctetToChange];
                    else
                    {
                        grpCurrentIpAddr.arIP[grpThirdOctetToChange] = 0;
                        if (grpIPv4Val[grpFourthOctetToChange] + IPv4Step[grpFourthOctetToChange] <= 255)
                            grpCurrentIpAddr.arIP[grpFourthOctetToChange] += IPv4Step[grpFourthOctetToChange];
                        else
                        {
                            rc = GT_NO_MORE;
                            break;
                        }
                    }
                }
            }
        }
    }

    /* print result */
    cpssOsPrintf("\nIpv4McPrefixAddMany results:");
    if (numOfPrefixesAdded == 0)
    {
        cpssOsPrintf("\n    No prefixes were added at all.\n");
    }
    else
    {
        cpssOsPrintf("\n    Group Prefixes ");
        outputIPAddress(grpStartIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
        cpssOsPrintf(" - ");
        outputIPAddress(grpLastIpAddrAdded.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
        cpssOsPrintf("\n    Source Prefixes ");
        outputIPAddress(srcStartIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
        cpssOsPrintf(" - ");
        outputIPAddress(srcLastIpAddrAdded.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
        cpssOsPrintf(" were added (%d/%d)\n",numOfPrefixesAdded,numOfPrefixesToAdd);
        cpssOsPrintf("    (%d) already existed \n",numOfPrefixesAlreadyExist);
        cpssOsPrintf("    total number of prefixes added plus existed (%d/%d) \n",(numOfPrefixesAlreadyExist+numOfPrefixesAdded),numOfPrefixesToAdd);

    }

    /* update output parameters */
    if (numOfPrefixesAddedPtr != NULL)
        *numOfPrefixesAddedPtr = numOfPrefixesAdded;
    if (grpLastIpAddrAddedPtr != NULL)
        grpLastIpAddrAddedPtr->u32Ip = grpLastIpAddrAdded.u32Ip;
    if (srcLastIpAddrAddedPtr != NULL)
        srcLastIpAddrAddedPtr->u32Ip = srcLastIpAddrAdded.u32Ip;

    /* Return to default */
    for (i = 0; i < 4; i++) {
        IPv4Step[i] = 1;
        srcIPv4Step[i] = 1;
    }
    IPv4PrefixLength = 32;
    srcIPv4PrefixLength = 32;

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpLpmDbgIpv4McPrefixAddManyByOctet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, lpmDbId, vrId, grpStartIpAddr, srcStartIpAddr,
                            routeEntryBaseMemAddr, numOfPrefixesToAdd, changeGrpAddr,
                            grpFirstOctetToChange, grpSecondOctetToChange,
                            grpThirdOctetToChange, grpFourthOctetToChange, changeSrcAddr,
                            srcFirstOctetToChange, srcSecondOctetToChange,
                            srcThirdOctetToChange, srcFourthOctetToChange,
                            grpLastIpAddrAddedPtr, srcLastIpAddrAddedPtr,
                            numOfPrefixesAddedPtr));

    rc = internal_cpssDxChIpLpmDbgIpv4McPrefixAddManyByOctet(lpmDbId, vrId,
                                                             grpStartIpAddr,
                                                             srcStartIpAddr,
                                                             routeEntryBaseMemAddr,
                                                             numOfPrefixesToAdd,
                                                             changeGrpAddr,
                                                             grpFirstOctetToChange,
                                                             grpSecondOctetToChange,
                                                             grpThirdOctetToChange,
                                                             grpFourthOctetToChange,
                                                             changeSrcAddr,
                                                             srcFirstOctetToChange,
                                                             srcSecondOctetToChange,
                                                             srcThirdOctetToChange,
                                                             srcFourthOctetToChange,
                                                             grpLastIpAddrAddedPtr,
                                                             srcLastIpAddrAddedPtr,
                                                             numOfPrefixesAddedPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, lpmDbId, vrId, grpStartIpAddr, srcStartIpAddr,
                              routeEntryBaseMemAddr, numOfPrefixesToAdd, changeGrpAddr,
                              grpFirstOctetToChange, grpSecondOctetToChange,
                              grpThirdOctetToChange, grpFourthOctetToChange, changeSrcAddr,
                              srcFirstOctetToChange, srcSecondOctetToChange,
                              srcThirdOctetToChange, srcFourthOctetToChange,
                              grpLastIpAddrAddedPtr, srcLastIpAddrAddedPtr,
                              numOfPrefixesAddedPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpLpmDbgIpv4McPrefixGetManyByOctet function
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
static GT_STATUS internal_cpssDxChIpLpmDbgIpv4McPrefixGetManyByOctet
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
)
{
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  i = 0;
    GT_U32                                  j = 0;
    GT_IPADDR                               grpCurrentIpAddr;
    GT_IPADDR                               srcCurrentIpAddr;
    GT_U32                                  numOfRetrievedPrefixes = 0;
    GT_IPADDR                               grpLastIpAddrAdded;
    GT_IPADDR                               srcLastIpAddrAdded;
    CPSS_DXCH_IP_LTT_ENTRY_STC              mcRouteLttEntry;
    GT_BOOL                                 octetsArray[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS]={GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE};/* array to hold the octets already selected */
    GT_U32                                  octetsOrderToChangeArray[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS];
    GT_U32                                  tcamGroupRowIndex=0;
    GT_U32                                  tcamGroupColumnIndex=0;
    GT_U32                                  tcamSrcRowIndex=0;
    GT_U32                                  tcamSrcColumnIndex=0;
    GT_U32                                  srcIPv4Val[4] = { 0 };
    GT_U32                                  grpIPv4Val[4] = { 0 };


    srcCurrentIpAddr.u32Ip                  = srcStartIpAddr.u32Ip;
    srcLastIpAddrAdded.u32Ip                = srcStartIpAddr.u32Ip;

    grpCurrentIpAddr.u32Ip                  = grpStartIpAddr.u32Ip;
    grpLastIpAddrAdded.u32Ip                = grpStartIpAddr.u32Ip;

    cpssOsMemSet(&mcRouteLttEntry, 0, sizeof(CPSS_DXCH_IP_LTT_ENTRY_STC));

    if(((changeGrpAddr==GT_TRUE)&&(changeSrcAddr==GT_TRUE))||
       ((changeGrpAddr==GT_FALSE)&&(changeSrcAddr==GT_FALSE)))
    {
        cpssOsPrintf("changeGrpAddr should be different from changeSrcAddr\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    if(changeGrpAddr==GT_TRUE)
    {
        octetsOrderToChangeArray[0] = grpFirstOctetToChange;
        octetsOrderToChangeArray[1] = grpSecondOctetToChange;
        octetsOrderToChangeArray[2] = grpThirdOctetToChange;
        octetsOrderToChangeArray[3] = grpFourthOctetToChange;
    }
    if(changeSrcAddr==GT_TRUE)
    {
        octetsOrderToChangeArray[0] = srcFirstOctetToChange;
        octetsOrderToChangeArray[1] = srcSecondOctetToChange;
        octetsOrderToChangeArray[2] = srcThirdOctetToChange;
        octetsOrderToChangeArray[3] = srcFourthOctetToChange;
    }

    for(j=0;j<PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS;j++)
    {
        if(octetsOrderToChangeArray[j]>3)
        {
            cpssOsPrintf("octetToChange can not be bigger then 3\n");
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    for(j=0;j<PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS;j++)
    {
        /* make sure octet are not selected twice */
        if(octetsArray[octetsOrderToChangeArray[j]]==GT_FALSE)
        {
            octetsArray[octetsOrderToChangeArray[j]]=GT_TRUE;
        }
        else
        {
            cpssOsPrintf("octetsOrderToChangeArray[j]=%d was selected twice\n",octetsOrderToChangeArray[j]);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    for (i = 0 ; i < numOfPrefixesToGet ; )
    {
        /* make sure the grp prefix is a MC prefix - in the range 224/4 - 239/4
           therefore the first octate must start with 0xE
           and the src prefix is not a MC prefix */
        if(((grpCurrentIpAddr.arIP[0] & 0xF0) == 0xE0)&&
           ((srcCurrentIpAddr.arIP[0] & 0xF0) != 0xE0))
        {
            /* get the current prefix */
            rc = cpssDxChIpLpmIpv4McEntrySearch(lpmDbId,vrId,
                                               &grpCurrentIpAddr,
                                               IPv4PrefixLength,
                                               &srcCurrentIpAddr,
                                               srcIPv4PrefixLength,
                                               &mcRouteLttEntry,
                                               &tcamGroupRowIndex,
                                               &tcamGroupColumnIndex,
                                               &tcamSrcRowIndex,
                                               &tcamSrcColumnIndex);

            if (rc != GT_OK)
                break;

            numOfRetrievedPrefixes++;
            i++;
        }
        else
        {
            cpssOsPrintf("\n grpStartIpAddr= ");
            outputIPAddress(grpCurrentIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
            cpssOsPrintf(" srcCurrentIpAddr= ");
            outputIPAddress(srcCurrentIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
            cpssOsPrintf(" is not a valid MC address\n");

            /* print result */
            cpssOsPrintf("\nIpv4UcPrefixGetMany results:");
            if (numOfRetrievedPrefixes == 0)
            {
                cpssOsPrintf("\n    No prefixes at all.\n");
            }
            else
            {
                outputIPAddress(grpStartIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
                cpssOsPrintf(" - ");
                outputIPAddress(grpLastIpAddrAdded.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
                cpssOsPrintf("\n    Source Prefixes ");
                outputIPAddress(srcStartIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
                cpssOsPrintf(" - ");
                outputIPAddress(srcLastIpAddrAdded.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
                cpssOsPrintf(" were retrieved (%d/%d)\n",numOfRetrievedPrefixes,numOfPrefixesToGet);
            }

            /* update output parameters */
            if (numOfRetrievedPrefixesPtr != NULL)
                *numOfRetrievedPrefixesPtr = numOfRetrievedPrefixes;

            /* Return to default */
            for (i = 0; i < 4; i++) {
                IPv4Step[i] = 1;
                srcIPv4Step[i] = 1;
            }
            IPv4PrefixLength = 32;
            srcIPv4PrefixLength = 32;

            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }


        srcLastIpAddrAdded.u32Ip                = srcCurrentIpAddr.u32Ip;
        grpLastIpAddrAdded.u32Ip                = grpCurrentIpAddr.u32Ip;

        srcIPv4Val[3]=srcCurrentIpAddr.arIP[3];
        srcIPv4Val[2]=srcCurrentIpAddr.arIP[2];
        srcIPv4Val[1]=srcCurrentIpAddr.arIP[1];
        srcIPv4Val[0]=srcCurrentIpAddr.arIP[0];

        grpIPv4Val[3]=grpCurrentIpAddr.arIP[3];
        grpIPv4Val[2]=grpCurrentIpAddr.arIP[2];
        grpIPv4Val[1]=grpCurrentIpAddr.arIP[1];
        grpIPv4Val[0]=grpCurrentIpAddr.arIP[0];

        if(changeSrcAddr==GT_TRUE)
        {
            /* advance to the next IP address */
            if (srcIPv4Val[srcFirstOctetToChange] + srcIPv4Step[srcFirstOctetToChange] <= 255)
                srcCurrentIpAddr.arIP[srcFirstOctetToChange] += srcIPv4Step[srcFirstOctetToChange];
            else
            {
                srcCurrentIpAddr.arIP[srcFirstOctetToChange] = 0;
                if (srcIPv4Val[srcSecondOctetToChange] + srcIPv4Step[srcSecondOctetToChange] <= 255)
                    srcCurrentIpAddr.arIP[srcSecondOctetToChange] += srcIPv4Step[srcSecondOctetToChange];
                else
                {
                    srcCurrentIpAddr.arIP[srcSecondOctetToChange] = 0;
                    if (srcIPv4Val[srcThirdOctetToChange] + srcIPv4Step[srcThirdOctetToChange] <= 255)
                        srcCurrentIpAddr.arIP[srcThirdOctetToChange] += srcIPv4Step[srcThirdOctetToChange];
                    else
                    {
                        srcCurrentIpAddr.arIP[srcThirdOctetToChange] = 0;
                        if (srcIPv4Val[srcFourthOctetToChange] + srcIPv4Step[srcFourthOctetToChange] <= 255)
                            srcCurrentIpAddr.arIP[srcFourthOctetToChange] += srcIPv4Step[srcFourthOctetToChange];
                        else
                        {
                            rc = GT_NO_MORE;
                            break;
                        }
                    }
                }
            }
        }
        else /* changeGrpAddr==GT_TRUE*/
        {
            /* advance to the next IP address */
            if (grpIPv4Val[grpFirstOctetToChange] + IPv4Step[grpFirstOctetToChange] <= 255)
                grpCurrentIpAddr.arIP[grpFirstOctetToChange] += IPv4Step[grpFirstOctetToChange];
            else
            {
                grpCurrentIpAddr.arIP[grpFirstOctetToChange] = 0;
                if (grpIPv4Val[grpSecondOctetToChange] + IPv4Step[grpSecondOctetToChange] <= 255)
                    grpCurrentIpAddr.arIP[grpSecondOctetToChange] += IPv4Step[grpSecondOctetToChange];
                else
                {
                    grpCurrentIpAddr.arIP[grpSecondOctetToChange] = 0;
                    if (grpIPv4Val[grpThirdOctetToChange] + IPv4Step[grpThirdOctetToChange] <= 255)
                        grpCurrentIpAddr.arIP[grpThirdOctetToChange] += IPv4Step[grpThirdOctetToChange];
                    else
                    {
                        grpCurrentIpAddr.arIP[grpThirdOctetToChange] = 0;
                        if (grpIPv4Val[grpFourthOctetToChange] + IPv4Step[grpFourthOctetToChange] <= 255)
                            grpCurrentIpAddr.arIP[grpFourthOctetToChange] += IPv4Step[grpFourthOctetToChange];
                        else
                        {
                            rc = GT_NO_MORE;
                            break;
                        }
                    }
                }
            }
        }
    }

    /* print result */
    cpssOsPrintf("\nIpv4UcPrefixGetMany results:");
    if (numOfRetrievedPrefixes == 0)
    {
        cpssOsPrintf("\n    No prefixes at all.\n");
    }
    else
    {
        outputIPAddress(grpStartIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
        cpssOsPrintf(" - ");
        outputIPAddress(grpLastIpAddrAdded.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
        cpssOsPrintf("\n    Source Prefixes ");
        outputIPAddress(srcStartIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
        cpssOsPrintf(" - ");
        outputIPAddress(srcLastIpAddrAdded.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
        cpssOsPrintf(" were retrieved (%d/%d)\n",numOfRetrievedPrefixes,numOfPrefixesToGet);
    }

    /* update output parameters */
    if (numOfRetrievedPrefixesPtr != NULL)
        *numOfRetrievedPrefixesPtr = numOfRetrievedPrefixes;

    /* Return to default */
    for (i = 0; i < 4; i++) {
        IPv4Step[i] = 1;
        srcIPv4Step[i] = 1;
    }
    IPv4PrefixLength = 32;
    srcIPv4PrefixLength = 32;

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpLpmDbgIpv4McPrefixGetManyByOctet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, lpmDbId, vrId, grpStartIpAddr, srcStartIpAddr,
                            numOfPrefixesToGet, changeGrpAddr,
                            grpFirstOctetToChange, grpSecondOctetToChange,
                            grpThirdOctetToChange, grpFourthOctetToChange,
                            changeSrcAddr, srcFirstOctetToChange,
                            srcSecondOctetToChange, srcThirdOctetToChange,
                            srcFourthOctetToChange, numOfRetrievedPrefixesPtr));

    rc = internal_cpssDxChIpLpmDbgIpv4McPrefixGetManyByOctet(lpmDbId, vrId,
                                                             grpStartIpAddr,
                                                             srcStartIpAddr,
                                                             numOfPrefixesToGet,
                                                             changeGrpAddr,
                                                             grpFirstOctetToChange,
                                                             grpSecondOctetToChange,
                                                             grpThirdOctetToChange,
                                                             grpFourthOctetToChange,
                                                             changeSrcAddr,
                                                             srcFirstOctetToChange,
                                                             srcSecondOctetToChange,
                                                             srcThirdOctetToChange,
                                                             srcFourthOctetToChange,
                                                             numOfRetrievedPrefixesPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, lpmDbId, vrId, grpStartIpAddr,
                              srcStartIpAddr, numOfPrefixesToGet, changeGrpAddr,
                              grpFirstOctetToChange, grpSecondOctetToChange,
                              grpThirdOctetToChange, grpFourthOctetToChange,
                              changeSrcAddr, srcFirstOctetToChange,
                              srcSecondOctetToChange, srcThirdOctetToChange,
                              srcFourthOctetToChange, numOfRetrievedPrefixesPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpLpmDbgIpv6McPrefixAddManyByOctet function
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
static GT_STATUS internal_cpssDxChIpLpmDbgIpv6McPrefixAddManyByOctet
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
)
{
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  i = 0;
    GT_U32                                  j = 0;
    GT_IPV6ADDR                             grpCurrentIpAddr;
    GT_IPV6ADDR                             srcCurrentIpAddr;
    GT_U32                                  numOfPrefixesAdded = 0;
    GT_U32                                  numOfPrefixesAlreadyExist = 0;
    GT_IPV6ADDR                             grpLastIpAddrAdded;
    GT_IPV6ADDR                             srcLastIpAddrAdded;
    CPSS_DXCH_IP_LTT_ENTRY_STC              mcRouteLttEntry;
    GT_BOOL                                 override = GT_FALSE;
    GT_BOOL                                 octetsArray[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS]={GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE,
                                                         GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE,
                                                         GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE,
                                                         GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE};/* array to hold the octets already selected */

    GT_U32                                  octetsOrderToChangeArray[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS];
    GT_BOOL                                 defragmentationEnable=defragmentationEnableFlag;
    GT_U32                                  srcIPv6Val[16] = { 0 };
    GT_U32                                  grpIPv6Val[16] = { 0 };
    PRV_CPSS_DXCH_LPM_SHADOW_STC            *lpmDbPtr,tmpLpmDb;

    tmpLpmDb.lpmDbId = lpmDbId;
    lpmDbPtr = prvCpssSlSearch(prvCpssDxChIplpmDbSL,&tmpLpmDb);
    if (lpmDbPtr == NULL)
    {
        /* can't find the lpm DB */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }


    cpssOsMemSet(&mcRouteLttEntry, 0, sizeof(CPSS_DXCH_IP_LTT_ENTRY_STC));
    mcRouteLttEntry.routeType           = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
    if ((lpmDbPtr->shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E) ||
        (lpmDbPtr->shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP5_SHADOW_E) )
    {
        mcRouteLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
        mcRouteLttEntry.priority = CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_LPM_E;
    }

    mcRouteLttEntry.routeEntryBaseIndex = routeEntryBaseMemAddr;
    mcRouteLttEntry.numOfPaths          = 0;
    mcRouteLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

    if(((changeGrpAddr==GT_TRUE)&&(changeSrcAddr==GT_TRUE))||
       ((changeGrpAddr==GT_FALSE)&&(changeSrcAddr==GT_FALSE)))
    {
        cpssOsPrintf("changeGrpAddr should be different from changeSrcAddr\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    if(changeGrpAddr==GT_TRUE)
    {
        octetsOrderToChangeArray[0] = grpFirstOctetToChange;
        octetsOrderToChangeArray[1] = grpSecondOctetToChange;
        octetsOrderToChangeArray[2] = grpThirdOctetToChange;
        octetsOrderToChangeArray[3] = grpFourthOctetToChange;
        octetsOrderToChangeArray[4] = grpFifthOctetToChange;
        octetsOrderToChangeArray[5] = grpSixthOctetToChange;
        octetsOrderToChangeArray[6] = grpSeventhOctetToChange;
        octetsOrderToChangeArray[7] = grpEighthOctetToChange;
        octetsOrderToChangeArray[8] = grpNinthOctetToChange;
        octetsOrderToChangeArray[9] = grpTenthOctetToChange;
        octetsOrderToChangeArray[10] = grpElevenOctetToChange;
        octetsOrderToChangeArray[11] = grpTwelveOctetToChange;
        octetsOrderToChangeArray[12] = grpThirteenOctetToChange;
        octetsOrderToChangeArray[13] = grpFourteenOctetToChange;
        octetsOrderToChangeArray[14] = grpFifteenOctetToChange;
        octetsOrderToChangeArray[15] = grpSixteenOctetToChange;
    }
    if(changeSrcAddr==GT_TRUE)
    {
        octetsOrderToChangeArray[0] = srcFirstOctetToChange;
        octetsOrderToChangeArray[1] = srcSecondOctetToChange;
        octetsOrderToChangeArray[2] = srcThirdOctetToChange;
        octetsOrderToChangeArray[3] = srcFourthOctetToChange;
        octetsOrderToChangeArray[4] = srcFifthOctetToChange;
        octetsOrderToChangeArray[5] = srcSixthOctetToChange;
        octetsOrderToChangeArray[6] = srcSeventhOctetToChange;
        octetsOrderToChangeArray[7] = srcEighthOctetToChange;
        octetsOrderToChangeArray[8] = srcNinthOctetToChange;
        octetsOrderToChangeArray[9] = srcTenthOctetToChange;
        octetsOrderToChangeArray[10] = srcElevenOctetToChange;
        octetsOrderToChangeArray[11] = srcTwelveOctetToChange;
        octetsOrderToChangeArray[12] = srcThirteenOctetToChange;
        octetsOrderToChangeArray[13] = srcFourteenOctetToChange;
        octetsOrderToChangeArray[14] = srcFifteenOctetToChange;
        octetsOrderToChangeArray[15] = srcSixteenOctetToChange;
    }

    for(j=0;j<PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS;j++)
    {
        if(octetsOrderToChangeArray[j]>15)
        {
            cpssOsPrintf("octetToChange can not be bigger then 15\n");
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    for(j=0;j<PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS;j++)
    {
        /* make sure octet are not selected twice */
        if(octetsArray[octetsOrderToChangeArray[j]]==GT_FALSE)
        {
            octetsArray[octetsOrderToChangeArray[j]]=GT_TRUE;
        }
        else
        {
            cpssOsPrintf("octetsOrderToChangeArray[j]=%d was selected twice\n",octetsOrderToChangeArray[j]);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    cpssOsMemSet(&grpCurrentIpAddr, 0, sizeof(GT_IPV6ADDR));
    cpssOsMemSet(&srcCurrentIpAddr, 0, sizeof(GT_IPV6ADDR));

    for(j=0;j<4;j++)
    {
        grpCurrentIpAddr.u32Ip[j]      = grpStartIpAddr.u32Ip[j];
        grpLastIpAddrAdded.u32Ip[j]    = grpStartIpAddr.u32Ip[j];

        srcCurrentIpAddr.u32Ip[j]      = srcStartIpAddr.u32Ip[j];
        srcLastIpAddrAdded.u32Ip[j]    = srcStartIpAddr.u32Ip[j];
    }

   for (i = 0 ; i < numOfPrefixesToAdd ; )
    {
        /* make sure the prefix is not a MC prefix - not in format ff00::/8
           therefore the first octate must not start with 0xFF
           and the src prefix is not a MC prefix */
        if((grpCurrentIpAddr.arIP[0] == 0xFF)&&
           (srcCurrentIpAddr.arIP[0] != 0xFF))
        {
            /* add the current prefix */
            rc = cpssDxChIpLpmIpv6McEntryAdd(lpmDbId,
                                            vrId,
                                            &grpCurrentIpAddr,
                                            IPv6PrefixLength,
                                            &srcCurrentIpAddr,
                                            srcIPv6PrefixLength,
                                            &mcRouteLttEntry,
                                            override,
                                            defragmentationEnable);

            if ((rc != GT_OK) && (rc!=GT_ALREADY_EXIST))
                break;

            if (rc==GT_ALREADY_EXIST)
            {
                rc = GT_OK;
                numOfPrefixesAlreadyExist++;
            }
            else
                numOfPrefixesAdded++;

            i++;
        }
        else
        {
            cpssOsPrintf("\n grpStartIpAddr= ");
            outputIPAddress(grpCurrentIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
            cpssOsPrintf(" srcCurrentIpAddr= ");
            outputIPAddress(srcCurrentIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
            cpssOsPrintf(" is not a valid MC address\n");

             /* print result */
            cpssOsPrintf("\nIpv6McPrefixAddMany results:");
            if (numOfPrefixesAdded == 0)
            {
                cpssOsPrintf("\n    No prefixes were added at all.\n");
            }
            else
            {
                cpssOsPrintf("\n    Group Prefixes ");
                outputIPAddress(grpStartIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
                cpssOsPrintf(" - ");
                outputIPAddress(grpLastIpAddrAdded.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
                cpssOsPrintf("\n    Source Prefixes ");
                outputIPAddress(srcStartIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
                cpssOsPrintf(" - ");
                outputIPAddress(srcLastIpAddrAdded.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
                cpssOsPrintf(" were added (%d/%d)\n",numOfPrefixesAdded,numOfPrefixesToAdd);
                cpssOsPrintf("    (%d) already existed \n",numOfPrefixesAlreadyExist);
                cpssOsPrintf("    total number of prefixes added plus existed (%d/%d) \n",(numOfPrefixesAlreadyExist+numOfPrefixesAdded),numOfPrefixesToAdd);

            }

            /* update output parameters */
            if (numOfPrefixesAddedPtr != NULL)
                *numOfPrefixesAddedPtr = numOfPrefixesAdded;
            if (grpLastIpAddrAddedPtr != NULL)
            {
                for(j=0;j<4;j++)
                {
                    grpLastIpAddrAddedPtr->u32Ip[j] = grpLastIpAddrAdded.u32Ip[j];
                }
            }

            if (srcLastIpAddrAddedPtr != NULL)
            {
                for(j=0;j<4;j++)
                {
                    srcLastIpAddrAddedPtr->u32Ip[j] = srcLastIpAddrAdded.u32Ip[j];
                }
            }

            /* Return to default */
            for (i = 0; i < 16; i++) {
                IPv6Step[i] = 1;
                srcIPv6Step[i] = 1;
            }
            IPv6PrefixLength = 128;
            srcIPv6PrefixLength = 128;

            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        for(j=0;j<4;j++)
        {
            grpLastIpAddrAdded.u32Ip[j]    = grpCurrentIpAddr.u32Ip[j];
            srcLastIpAddrAdded.u32Ip[j]    = srcCurrentIpAddr.u32Ip[j];
        }

        for(j=0;j<16;j++)
        {
            srcIPv6Val[j]    = srcCurrentIpAddr.arIP[j];
            grpIPv6Val[j]    = grpCurrentIpAddr.arIP[j];
        }

        if(changeSrcAddr==GT_TRUE)
        {
            /* advance to the next IP address */
            if (srcIPv6Val[srcFirstOctetToChange] + srcIPv6Step[srcFirstOctetToChange] <= 255)
                srcCurrentIpAddr.arIP[srcFirstOctetToChange] += srcIPv6Step[srcFirstOctetToChange];
            else
            {
                srcCurrentIpAddr.arIP[srcFirstOctetToChange] = 0;
                if (srcIPv6Val[srcSecondOctetToChange] + srcIPv6Step[srcSecondOctetToChange] <= 255)
                    srcCurrentIpAddr.arIP[srcSecondOctetToChange] += srcIPv6Step[srcSecondOctetToChange];
                else
                {
                    srcCurrentIpAddr.arIP[srcSecondOctetToChange] = 0;
                    if (srcIPv6Val[srcThirdOctetToChange] + srcIPv6Step[srcThirdOctetToChange] <= 255)
                        srcCurrentIpAddr.arIP[srcThirdOctetToChange] += srcIPv6Step[srcThirdOctetToChange];
                    else
                    {
                        srcCurrentIpAddr.arIP[srcThirdOctetToChange] = 0;
                        if (srcIPv6Val[srcFourthOctetToChange] + srcIPv6Step[srcFourthOctetToChange] <= 255)
                            srcCurrentIpAddr.arIP[srcFourthOctetToChange] += srcIPv6Step[srcFourthOctetToChange];
                        else
                        {
                            srcCurrentIpAddr.arIP[srcFourthOctetToChange] = 0;
                            if (srcIPv6Val[srcFifthOctetToChange] + srcIPv6Step[srcFifthOctetToChange] <= 255)
                                srcCurrentIpAddr.arIP[srcFifthOctetToChange] += srcIPv6Step[srcFifthOctetToChange];
                            else
                            {
                                srcCurrentIpAddr.arIP[srcFifthOctetToChange] = 0;
                                if (srcIPv6Val[srcSixthOctetToChange] + srcIPv6Step[srcSixthOctetToChange] <= 255)
                                    srcCurrentIpAddr.arIP[srcSixthOctetToChange] += srcIPv6Step[srcSixthOctetToChange];
                                else
                                {
                                    srcCurrentIpAddr.arIP[srcSixthOctetToChange] = 0;
                                    if (srcIPv6Val[srcSeventhOctetToChange] + srcIPv6Step[srcSeventhOctetToChange] <= 255)
                                        srcCurrentIpAddr.arIP[srcSeventhOctetToChange] += srcIPv6Step[srcSeventhOctetToChange];
                                    else
                                    {
                                        srcCurrentIpAddr.arIP[srcSeventhOctetToChange] = 0;
                                        if (srcIPv6Val[srcEighthOctetToChange] + srcIPv6Step[srcEighthOctetToChange] <= 255)
                                            srcCurrentIpAddr.arIP[srcEighthOctetToChange] += srcIPv6Step[srcEighthOctetToChange];
                                        else
                                        {
                                            srcCurrentIpAddr.arIP[srcEighthOctetToChange] = 0;
                                            if (srcIPv6Val[srcNinthOctetToChange] + srcIPv6Step[srcNinthOctetToChange] <= 255)
                                                srcCurrentIpAddr.arIP[srcNinthOctetToChange] += srcIPv6Step[srcNinthOctetToChange];
                                            else
                                            {
                                                srcCurrentIpAddr.arIP[srcNinthOctetToChange] = 0;
                                                if (srcIPv6Val[srcTenthOctetToChange] + srcIPv6Step[srcTenthOctetToChange] <= 255)
                                                    srcCurrentIpAddr.arIP[srcTenthOctetToChange] += srcIPv6Step[srcTenthOctetToChange];
                                                else
                                                {
                                                    srcCurrentIpAddr.arIP[srcTenthOctetToChange] = 0;
                                                    if (srcIPv6Val[srcElevenOctetToChange] + srcIPv6Step[srcElevenOctetToChange] <= 255)
                                                        srcCurrentIpAddr.arIP[srcElevenOctetToChange] += srcIPv6Step[srcElevenOctetToChange];
                                                    else
                                                    {
                                                        srcCurrentIpAddr.arIP[srcElevenOctetToChange] = 0;
                                                        if (srcIPv6Val[srcTwelveOctetToChange] + srcIPv6Step[srcTwelveOctetToChange] <= 255)
                                                            srcCurrentIpAddr.arIP[srcTwelveOctetToChange] += srcIPv6Step[srcTwelveOctetToChange];
                                                        else
                                                        {
                                                            srcCurrentIpAddr.arIP[srcTwelveOctetToChange] = 0;
                                                            if (srcIPv6Val[srcThirteenOctetToChange] + srcIPv6Step[srcThirteenOctetToChange] <= 255)
                                                                srcCurrentIpAddr.arIP[srcThirteenOctetToChange] += srcIPv6Step[srcThirteenOctetToChange];
                                                            else
                                                            {
                                                                srcCurrentIpAddr.arIP[srcThirteenOctetToChange] = 0;
                                                                if (srcIPv6Val[srcFourteenOctetToChange] + srcIPv6Step[srcFourteenOctetToChange] <= 255)
                                                                    srcCurrentIpAddr.arIP[srcFourteenOctetToChange] += srcIPv6Step[srcFourteenOctetToChange];
                                                                else
                                                                {
                                                                    srcCurrentIpAddr.arIP[srcFourteenOctetToChange] = 0;
                                                                    if (srcIPv6Val[srcFifteenOctetToChange] + srcIPv6Step[srcFifteenOctetToChange] <= 255)
                                                                        srcCurrentIpAddr.arIP[srcFifteenOctetToChange] += srcIPv6Step[srcFifteenOctetToChange];
                                                                    else
                                                                    {
                                                                        srcCurrentIpAddr.arIP[srcFifteenOctetToChange] = 0;
                                                                        if (srcIPv6Val[srcSixteenOctetToChange] + srcIPv6Step[srcSixteenOctetToChange] <= 255)
                                                                            srcCurrentIpAddr.arIP[srcSixteenOctetToChange] += srcIPv6Step[srcSixteenOctetToChange];
                                                                        else
                                                                        {
                                                                            rc = GT_NO_MORE;
                                                                            break;
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

        }
        else /* changeGrpAddr==GT_TRUE*/
        {
            /* advance to the next IP address */
            if (grpIPv6Val[grpFirstOctetToChange] + IPv6Step[grpFirstOctetToChange] <= 255)
                grpCurrentIpAddr.arIP[grpFirstOctetToChange] += IPv6Step[grpFirstOctetToChange];
            else
            {
                grpCurrentIpAddr.arIP[grpFirstOctetToChange] = 0;
                if (grpIPv6Val[grpSecondOctetToChange] + IPv6Step[grpSecondOctetToChange] <= 255)
                    grpCurrentIpAddr.arIP[grpSecondOctetToChange] += IPv6Step[grpSecondOctetToChange];
                else
                {
                    grpCurrentIpAddr.arIP[grpSecondOctetToChange] = 0;
                    if (grpIPv6Val[grpThirdOctetToChange] + IPv6Step[grpThirdOctetToChange] <= 255)
                        grpCurrentIpAddr.arIP[grpThirdOctetToChange] += IPv6Step[grpThirdOctetToChange];
                    else
                    {
                        grpCurrentIpAddr.arIP[grpThirdOctetToChange] = 0;
                        if (grpIPv6Val[grpFourthOctetToChange] + IPv6Step[grpFourthOctetToChange] <= 255)
                            grpCurrentIpAddr.arIP[grpFourthOctetToChange] += IPv6Step[grpFourthOctetToChange];
                        else
                        {
                            grpCurrentIpAddr.arIP[grpFourthOctetToChange] = 0;
                            if (grpIPv6Val[grpFifthOctetToChange] + IPv6Step[grpFifthOctetToChange] <= 255)
                                grpCurrentIpAddr.arIP[grpFifthOctetToChange] += IPv6Step[grpFifthOctetToChange];
                            else
                            {
                                grpCurrentIpAddr.arIP[grpFifthOctetToChange] = 0;
                                if (grpIPv6Val[grpSixthOctetToChange] + IPv6Step[grpSixthOctetToChange] <= 255)
                                    grpCurrentIpAddr.arIP[grpSixthOctetToChange] += IPv6Step[grpSixthOctetToChange];
                                else
                                {
                                    grpCurrentIpAddr.arIP[grpSixthOctetToChange] = 0;
                                    if (grpIPv6Val[grpSeventhOctetToChange] + IPv6Step[grpSeventhOctetToChange] <= 255)
                                        grpCurrentIpAddr.arIP[grpSeventhOctetToChange] += IPv6Step[grpSeventhOctetToChange];
                                    else
                                    {
                                        grpCurrentIpAddr.arIP[grpSeventhOctetToChange] = 0;
                                        if (grpIPv6Val[grpEighthOctetToChange] + IPv6Step[grpEighthOctetToChange] <= 255)
                                            grpCurrentIpAddr.arIP[grpEighthOctetToChange] += IPv6Step[grpEighthOctetToChange];
                                        else
                                        {
                                            grpCurrentIpAddr.arIP[grpEighthOctetToChange] = 0;
                                            if (grpIPv6Val[grpNinthOctetToChange] + IPv6Step[grpNinthOctetToChange] <= 255)
                                                grpCurrentIpAddr.arIP[grpNinthOctetToChange] += IPv6Step[grpNinthOctetToChange];
                                            else
                                            {
                                                grpCurrentIpAddr.arIP[grpNinthOctetToChange] = 0;
                                                if (grpIPv6Val[grpTenthOctetToChange] + IPv6Step[grpTenthOctetToChange] <= 255)
                                                    grpCurrentIpAddr.arIP[grpTenthOctetToChange] += IPv6Step[grpTenthOctetToChange];
                                                else
                                                {
                                                    grpCurrentIpAddr.arIP[grpTenthOctetToChange] = 0;
                                                    if (grpIPv6Val[grpElevenOctetToChange] + IPv6Step[grpElevenOctetToChange] <= 255)
                                                        grpCurrentIpAddr.arIP[grpElevenOctetToChange] += IPv6Step[grpElevenOctetToChange];
                                                    else
                                                    {
                                                        grpCurrentIpAddr.arIP[grpElevenOctetToChange] = 0;
                                                        if (grpIPv6Val[grpTwelveOctetToChange] + IPv6Step[grpTwelveOctetToChange] <= 255)
                                                            grpCurrentIpAddr.arIP[grpTwelveOctetToChange] += IPv6Step[grpTwelveOctetToChange];
                                                        else
                                                        {
                                                            grpCurrentIpAddr.arIP[grpTwelveOctetToChange] = 0;
                                                            if (grpIPv6Val[grpThirteenOctetToChange] + IPv6Step[grpThirteenOctetToChange] <= 255)
                                                                grpCurrentIpAddr.arIP[grpThirteenOctetToChange] += IPv6Step[grpThirteenOctetToChange];
                                                            else
                                                            {
                                                                grpCurrentIpAddr.arIP[grpThirteenOctetToChange] = 0;
                                                                if (grpIPv6Val[grpFourteenOctetToChange] + IPv6Step[grpFourteenOctetToChange] <= 255)
                                                                    grpCurrentIpAddr.arIP[grpFourteenOctetToChange] += IPv6Step[grpFourteenOctetToChange];
                                                                else
                                                                {
                                                                    grpCurrentIpAddr.arIP[grpFourteenOctetToChange] = 0;
                                                                    if (grpIPv6Val[grpFifteenOctetToChange] + IPv6Step[grpFifteenOctetToChange] <= 255)
                                                                        grpCurrentIpAddr.arIP[grpFifteenOctetToChange] += IPv6Step[grpFifteenOctetToChange];
                                                                    else
                                                                    {
                                                                        grpCurrentIpAddr.arIP[grpFifteenOctetToChange] = 0;
                                                                        if (grpIPv6Val[grpSixteenOctetToChange] + IPv6Step[grpSixteenOctetToChange] <= 255)
                                                                            grpCurrentIpAddr.arIP[grpSixteenOctetToChange] += IPv6Step[grpSixteenOctetToChange];
                                                                        else
                                                                        {
                                                                            rc = GT_NO_MORE;
                                                                            break;
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    /* print result */
    cpssOsPrintf("\nIpv6McPrefixAddMany results:");
    if (numOfPrefixesAdded == 0)
    {
        cpssOsPrintf("\n    No prefixes were added at all.\n");
    }
    else
    {
        cpssOsPrintf("\n    Group Prefixes ");
        outputIPAddress(grpStartIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
        cpssOsPrintf(" - ");
        outputIPAddress(grpLastIpAddrAdded.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
        cpssOsPrintf("\n    Source Prefixes ");
        outputIPAddress(srcStartIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
        cpssOsPrintf(" - ");
        outputIPAddress(srcLastIpAddrAdded.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
        cpssOsPrintf(" were added (%d/%d)\n",numOfPrefixesAdded,numOfPrefixesToAdd);
        cpssOsPrintf("    (%d) already existed \n",numOfPrefixesAlreadyExist);
        cpssOsPrintf("    total number of prefixes added plus existed (%d/%d) \n",(numOfPrefixesAlreadyExist+numOfPrefixesAdded),numOfPrefixesToAdd);

    }

    /* update output parameters */
    if (numOfPrefixesAddedPtr != NULL)
        *numOfPrefixesAddedPtr = numOfPrefixesAdded;
    if (grpLastIpAddrAddedPtr != NULL)
    {
        for(j=0;j<4;j++)
        {
            grpLastIpAddrAddedPtr->u32Ip[j] = grpLastIpAddrAdded.u32Ip[j];
        }
    }

    if (srcLastIpAddrAddedPtr != NULL)
    {
        for(j=0;j<4;j++)
        {
            srcLastIpAddrAddedPtr->u32Ip[j] = srcLastIpAddrAdded.u32Ip[j];
        }
    }

    /* Return to default */
    for (i = 0; i < 16; i++) {
        IPv6Step[i] = 1;
        srcIPv6Step[i] = 1;
    }
    IPv6PrefixLength = 128;
    srcIPv6PrefixLength = 128;

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpLpmDbgIpv6McPrefixAddManyByOctet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, lpmDbId, vrId, grpStartIpAddr, srcStartIpAddr,
                            routeEntryBaseMemAddr, numOfPrefixesToAdd, changeGrpAddr,
                            grpFirstOctetToChange, grpSecondOctetToChange,
                            grpThirdOctetToChange, grpFourthOctetToChange,
                            grpFifthOctetToChange, grpSixthOctetToChange,
                            grpSeventhOctetToChange, grpEighthOctetToChange,
                            grpNinthOctetToChange, grpTenthOctetToChange,
                            grpElevenOctetToChange, grpTwelveOctetToChange,
                            grpThirteenOctetToChange, grpFourteenOctetToChange,
                            grpFifteenOctetToChange, grpSixteenOctetToChange,
                            changeSrcAddr, srcFirstOctetToChange, srcSecondOctetToChange,
                            srcThirdOctetToChange, srcFourthOctetToChange,
                            srcFifthOctetToChange, srcSixthOctetToChange,
                            srcSeventhOctetToChange, srcEighthOctetToChange,
                            srcNinthOctetToChange, srcTenthOctetToChange,
                            srcElevenOctetToChange, srcTwelveOctetToChange,
                            srcThirteenOctetToChange, srcFourteenOctetToChange,
                            srcFifteenOctetToChange, srcSixteenOctetToChange,
                            grpLastIpAddrAddedPtr, srcLastIpAddrAddedPtr,
                            numOfPrefixesAddedPtr));

    rc = internal_cpssDxChIpLpmDbgIpv6McPrefixAddManyByOctet(lpmDbId, vrId,
                                                             grpStartIpAddr,
                                                             srcStartIpAddr,
                                                             routeEntryBaseMemAddr,
                                                             numOfPrefixesToAdd,
                                                             changeGrpAddr,
                                                             grpFirstOctetToChange,
                                                             grpSecondOctetToChange,
                                                             grpThirdOctetToChange,
                                                             grpFourthOctetToChange,
                                                             grpFifthOctetToChange,
                                                             grpSixthOctetToChange,
                                                             grpSeventhOctetToChange,
                                                             grpEighthOctetToChange,
                                                             grpNinthOctetToChange,
                                                             grpTenthOctetToChange,
                                                             grpElevenOctetToChange,
                                                             grpTwelveOctetToChange,
                                                             grpThirteenOctetToChange,
                                                             grpFourteenOctetToChange,
                                                             grpFifteenOctetToChange,
                                                             grpSixteenOctetToChange,
                                                             changeSrcAddr,
                                                             srcFirstOctetToChange,
                                                             srcSecondOctetToChange,
                                                             srcThirdOctetToChange,
                                                             srcFourthOctetToChange,
                                                             srcFifthOctetToChange,
                                                             srcSixthOctetToChange,
                                                             srcSeventhOctetToChange,
                                                             srcEighthOctetToChange,
                                                             srcNinthOctetToChange,
                                                             srcTenthOctetToChange,
                                                             srcElevenOctetToChange,
                                                             srcTwelveOctetToChange,
                                                             srcThirteenOctetToChange,
                                                             srcFourteenOctetToChange,
                                                             srcFifteenOctetToChange,
                                                             srcSixteenOctetToChange,
                                                             grpLastIpAddrAddedPtr,
                                                             srcLastIpAddrAddedPtr,
                                                             numOfPrefixesAddedPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, lpmDbId, vrId, grpStartIpAddr, srcStartIpAddr,
                              routeEntryBaseMemAddr, numOfPrefixesToAdd, changeGrpAddr,
                              grpFirstOctetToChange, grpSecondOctetToChange,
                              grpThirdOctetToChange, grpFourthOctetToChange,
                              grpFifthOctetToChange, grpSixthOctetToChange,
                              grpSeventhOctetToChange, grpEighthOctetToChange,
                              grpNinthOctetToChange, grpTenthOctetToChange,
                              grpElevenOctetToChange, grpTwelveOctetToChange,
                              grpThirteenOctetToChange, grpFourteenOctetToChange,
                              grpFifteenOctetToChange, grpSixteenOctetToChange,
                              changeSrcAddr, srcFirstOctetToChange, srcSecondOctetToChange,
                              srcThirdOctetToChange, srcFourthOctetToChange,
                              srcFifthOctetToChange, srcSixthOctetToChange,
                              srcSeventhOctetToChange, srcEighthOctetToChange,
                              srcNinthOctetToChange, srcTenthOctetToChange,
                              srcElevenOctetToChange, srcTwelveOctetToChange,
                              srcThirteenOctetToChange, srcFourteenOctetToChange,
                              srcFifteenOctetToChange, srcSixteenOctetToChange,
                              grpLastIpAddrAddedPtr, srcLastIpAddrAddedPtr,
                              numOfPrefixesAddedPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpLpmDbgIpv6McPrefixGetManyByOctet function
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
static GT_STATUS internal_cpssDxChIpLpmDbgIpv6McPrefixGetManyByOctet
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
)
{
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  i = 0;
    GT_U32                                  j = 0;
    GT_IPV6ADDR                             grpCurrentIpAddr;
    GT_IPV6ADDR                             srcCurrentIpAddr;
    GT_U32                                  numOfRetrievedPrefixes = 0;
    GT_IPV6ADDR                             grpLastIpAddrAdded;
    GT_IPV6ADDR                             srcLastIpAddrAdded;
    CPSS_DXCH_IP_LTT_ENTRY_STC              mcRouteLttEntry;
    GT_BOOL                                 octetsArray[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS]={GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE,
                                                            GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE,
                                                            GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE,
                                                            GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE};/* array to hold the octets already selected */

    GT_U32                                  octetsOrderToChangeArray[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS];
    GT_U32                                  tcamGroupRowIndex=0;
    GT_U32                                  tcamSrcRowIndex=0;
    GT_U32                                  srcIPv6Val[16] = { 0 };
    GT_U32                                  grpIPv6Val[16] = { 0 };

    cpssOsMemSet(&mcRouteLttEntry, 0, sizeof(CPSS_DXCH_IP_LTT_ENTRY_STC));

    if(((changeGrpAddr==GT_TRUE)&&(changeSrcAddr==GT_TRUE))||
       ((changeGrpAddr==GT_FALSE)&&(changeSrcAddr==GT_FALSE)))
    {
        cpssOsPrintf("changeGrpAddr should be different from changeSrcAddr\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(changeGrpAddr==GT_TRUE)
    {
        octetsOrderToChangeArray[0] = grpFirstOctetToChange;
        octetsOrderToChangeArray[1] = grpSecondOctetToChange;
        octetsOrderToChangeArray[2] = grpThirdOctetToChange;
        octetsOrderToChangeArray[3] = grpFourthOctetToChange;
        octetsOrderToChangeArray[4] = grpFifthOctetToChange;
        octetsOrderToChangeArray[5] = grpSixthOctetToChange;
        octetsOrderToChangeArray[6] = grpSeventhOctetToChange;
        octetsOrderToChangeArray[7] = grpEighthOctetToChange;
        octetsOrderToChangeArray[8] = grpNinthOctetToChange;
        octetsOrderToChangeArray[9] = grpTenthOctetToChange;
        octetsOrderToChangeArray[10] = grpElevenOctetToChange;
        octetsOrderToChangeArray[11] = grpTwelveOctetToChange;
        octetsOrderToChangeArray[12] = grpThirteenOctetToChange;
        octetsOrderToChangeArray[13] = grpFourteenOctetToChange;
        octetsOrderToChangeArray[14] = grpFifteenOctetToChange;
        octetsOrderToChangeArray[15] = grpSixteenOctetToChange;
    }
    if(changeSrcAddr==GT_TRUE)
    {
        octetsOrderToChangeArray[0] = srcFirstOctetToChange;
        octetsOrderToChangeArray[1] = srcSecondOctetToChange;
        octetsOrderToChangeArray[2] = srcThirdOctetToChange;
        octetsOrderToChangeArray[3] = srcFourthOctetToChange;
        octetsOrderToChangeArray[4] = srcFifthOctetToChange;
        octetsOrderToChangeArray[5] = srcSixthOctetToChange;
        octetsOrderToChangeArray[6] = srcSeventhOctetToChange;
        octetsOrderToChangeArray[7] = srcEighthOctetToChange;
        octetsOrderToChangeArray[8] = srcNinthOctetToChange;
        octetsOrderToChangeArray[9] = srcTenthOctetToChange;
        octetsOrderToChangeArray[10] = srcElevenOctetToChange;
        octetsOrderToChangeArray[11] = srcTwelveOctetToChange;
        octetsOrderToChangeArray[12] = srcThirteenOctetToChange;
        octetsOrderToChangeArray[13] = srcFourteenOctetToChange;
        octetsOrderToChangeArray[14] = srcFifteenOctetToChange;
        octetsOrderToChangeArray[15] = srcSixteenOctetToChange;
    }

    for(j=0;j<PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS;j++)
    {
        if(octetsOrderToChangeArray[j]>15)
        {
            cpssOsPrintf("octetToChange can not be bigger then 15\n");
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    for(j=0;j<PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS;j++)
    {
        /* make sure octet are not selected twice */
        if(octetsArray[octetsOrderToChangeArray[j]]==GT_FALSE)
        {
            octetsArray[octetsOrderToChangeArray[j]]=GT_TRUE;
        }
        else
        {
            cpssOsPrintf("octetsOrderToChangeArray[j]=%d was selected twice\n",octetsOrderToChangeArray[j]);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    cpssOsMemSet(&grpCurrentIpAddr, 0, sizeof(GT_IPV6ADDR));
    cpssOsMemSet(&srcCurrentIpAddr, 0, sizeof(GT_IPV6ADDR));

    for(j=0;j<4;j++)
    {
        grpCurrentIpAddr.u32Ip[j]      = grpStartIpAddr.u32Ip[j];
        grpLastIpAddrAdded.u32Ip[j]    = grpStartIpAddr.u32Ip[j];

        srcCurrentIpAddr.u32Ip[j]      = srcStartIpAddr.u32Ip[j];
        srcLastIpAddrAdded.u32Ip[j]    = srcStartIpAddr.u32Ip[j];
    }

    for (i = 0 ; i < numOfPrefixesToGet ; )
    {
        /* make sure the prefix is a MC prefix - in format ff00::/8
           therefore the first octate must start with 0xFF,
           and the src prefix is not a MC prefix */
        if((grpCurrentIpAddr.arIP[0] == 0xFF)&&
           (srcCurrentIpAddr.arIP[0] != 0xFF))
        {

            /* get the current prefix */
            rc =  cpssDxChIpLpmIpv6McEntrySearch(lpmDbId,
                                                 vrId,
                                                 &grpCurrentIpAddr,
                                                 IPv6PrefixLength,
                                                 &srcCurrentIpAddr,
                                                 srcIPv6PrefixLength,
                                                 &mcRouteLttEntry,
                                                 &tcamGroupRowIndex,
                                                 &tcamSrcRowIndex);
             if (rc != GT_OK)
                    break;

                numOfRetrievedPrefixes++;
                i++;
        }
        else
        {
            cpssOsPrintf("\n grpStartIpAddr= ");
            outputIPAddress(grpCurrentIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
            cpssOsPrintf(" srcCurrentIpAddr= ");
            outputIPAddress(srcCurrentIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
            cpssOsPrintf(" is not a valid MC address\n");

            /* print result */
            cpssOsPrintf("\nIpv6McPrefixGetMany results:");
            if (numOfRetrievedPrefixes == 0)
            {
                cpssOsPrintf("\n    No prefixes at all.\n");
            }
            else
            {

                cpssOsPrintf("\n    Group Prefixes ");
                outputIPAddress(grpStartIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
                cpssOsPrintf(" - ");
                outputIPAddress(grpLastIpAddrAdded.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
                cpssOsPrintf("\n    Source Prefixes ");
                outputIPAddress(srcStartIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
                cpssOsPrintf(" - ");
                outputIPAddress(srcLastIpAddrAdded.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
                cpssOsPrintf(" were retrieved (%d/%d)\n",numOfRetrievedPrefixes,numOfPrefixesToGet);
            }

            /* update output parameters */
            if (numOfRetrievedPrefixesPtr != NULL)
                *numOfRetrievedPrefixesPtr = numOfRetrievedPrefixes;

            /* Return to default */
            for (i = 0; i < 16; i++) {
                IPv6Step[i] = 1;
                srcIPv6Step[i] = 1;
            }
            IPv6PrefixLength = 128;
            srcIPv6PrefixLength = 128;

            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        for(j=0;j<4;j++)
        {
            grpLastIpAddrAdded.u32Ip[j]    = grpCurrentIpAddr.u32Ip[j];
            srcLastIpAddrAdded.u32Ip[j]    = srcCurrentIpAddr.u32Ip[j];
        }

        for(j=0;j<16;j++)
        {
            srcIPv6Val[j]    = srcCurrentIpAddr.arIP[j];
            grpIPv6Val[j]    = grpCurrentIpAddr.arIP[j];
        }

        if(changeSrcAddr==GT_TRUE)
        {
            /* advance to the next IP address */
            if (srcIPv6Val[srcFirstOctetToChange] + srcIPv6Step[srcFirstOctetToChange] <= 255)
                srcCurrentIpAddr.arIP[srcFirstOctetToChange] += srcIPv6Step[srcFirstOctetToChange];
            else
            {
                srcCurrentIpAddr.arIP[srcFirstOctetToChange] = 0;
                if (srcIPv6Val[srcSecondOctetToChange] + srcIPv6Step[srcSecondOctetToChange] <= 255)
                    srcCurrentIpAddr.arIP[srcSecondOctetToChange] += srcIPv6Step[srcSecondOctetToChange];
                else
                {
                    srcCurrentIpAddr.arIP[srcSecondOctetToChange] = 0;
                    if (srcIPv6Val[srcThirdOctetToChange] + srcIPv6Step[srcThirdOctetToChange] <= 255)
                        srcCurrentIpAddr.arIP[srcThirdOctetToChange] += srcIPv6Step[srcThirdOctetToChange];
                    else
                    {
                        srcCurrentIpAddr.arIP[srcThirdOctetToChange] = 0;
                        if (srcIPv6Val[srcFourthOctetToChange] + srcIPv6Step[srcFourthOctetToChange] <= 255)
                            srcCurrentIpAddr.arIP[srcFourthOctetToChange] += srcIPv6Step[srcFourthOctetToChange];
                        else
                        {
                            srcCurrentIpAddr.arIP[srcFourthOctetToChange] = 0;
                            if (srcIPv6Val[srcFifthOctetToChange] + srcIPv6Step[srcFifthOctetToChange] <= 255)
                                srcCurrentIpAddr.arIP[srcFifthOctetToChange] += srcIPv6Step[srcFifthOctetToChange];
                            else
                            {
                                srcCurrentIpAddr.arIP[srcFifthOctetToChange] = 0;
                                if (srcIPv6Val[srcSixthOctetToChange] + srcIPv6Step[srcSixthOctetToChange] <= 255)
                                    srcCurrentIpAddr.arIP[srcSixthOctetToChange] += srcIPv6Step[srcSixthOctetToChange];
                                else
                                {
                                    srcCurrentIpAddr.arIP[srcSixthOctetToChange] = 0;
                                    if (srcIPv6Val[srcSeventhOctetToChange] + srcIPv6Step[srcSeventhOctetToChange] <= 255)
                                        srcCurrentIpAddr.arIP[srcSeventhOctetToChange] += srcIPv6Step[srcSeventhOctetToChange];
                                    else
                                    {
                                        srcCurrentIpAddr.arIP[srcSeventhOctetToChange] = 0;
                                        if (srcIPv6Val[srcEighthOctetToChange] + srcIPv6Step[srcEighthOctetToChange] <= 255)
                                            srcCurrentIpAddr.arIP[srcEighthOctetToChange] += srcIPv6Step[srcEighthOctetToChange];
                                        else
                                        {
                                            srcCurrentIpAddr.arIP[srcEighthOctetToChange] = 0;
                                            if (srcIPv6Val[srcNinthOctetToChange] + srcIPv6Step[srcNinthOctetToChange] <= 255)
                                                srcCurrentIpAddr.arIP[srcNinthOctetToChange] += srcIPv6Step[srcNinthOctetToChange];
                                            else
                                            {
                                                srcCurrentIpAddr.arIP[srcNinthOctetToChange] = 0;
                                                if (srcIPv6Val[srcTenthOctetToChange] + srcIPv6Step[srcTenthOctetToChange] <= 255)
                                                    srcCurrentIpAddr.arIP[srcTenthOctetToChange] += srcIPv6Step[srcTenthOctetToChange];
                                                else
                                                {
                                                    srcCurrentIpAddr.arIP[srcTenthOctetToChange] = 0;
                                                    if (srcIPv6Val[srcElevenOctetToChange] + srcIPv6Step[srcElevenOctetToChange] <= 255)
                                                        srcCurrentIpAddr.arIP[srcElevenOctetToChange] += srcIPv6Step[srcElevenOctetToChange];
                                                    else
                                                    {
                                                        srcCurrentIpAddr.arIP[srcElevenOctetToChange] = 0;
                                                        if (srcIPv6Val[srcTwelveOctetToChange] + srcIPv6Step[srcTwelveOctetToChange] <= 255)
                                                            srcCurrentIpAddr.arIP[srcTwelveOctetToChange] += srcIPv6Step[srcTwelveOctetToChange];
                                                        else
                                                        {
                                                            srcCurrentIpAddr.arIP[srcTwelveOctetToChange] = 0;
                                                            if (srcIPv6Val[srcThirteenOctetToChange] + srcIPv6Step[srcThirteenOctetToChange] <= 255)
                                                                srcCurrentIpAddr.arIP[srcThirteenOctetToChange] += srcIPv6Step[srcThirteenOctetToChange];
                                                            else
                                                            {
                                                                srcCurrentIpAddr.arIP[srcThirteenOctetToChange] = 0;
                                                                if (srcIPv6Val[srcFourteenOctetToChange] + srcIPv6Step[srcFourteenOctetToChange] <= 255)
                                                                    srcCurrentIpAddr.arIP[srcFourteenOctetToChange] += srcIPv6Step[srcFourteenOctetToChange];
                                                                else
                                                                {
                                                                    srcCurrentIpAddr.arIP[srcFourteenOctetToChange] = 0;
                                                                    if (srcIPv6Val[srcFifteenOctetToChange] + srcIPv6Step[srcFifteenOctetToChange] <= 255)
                                                                        srcCurrentIpAddr.arIP[srcFifteenOctetToChange] += srcIPv6Step[srcFifteenOctetToChange];
                                                                    else
                                                                    {
                                                                        srcCurrentIpAddr.arIP[srcFifteenOctetToChange] = 0;
                                                                        if (srcIPv6Val[srcSixteenOctetToChange] + srcIPv6Step[srcSixteenOctetToChange] <= 255)
                                                                            srcCurrentIpAddr.arIP[srcSixteenOctetToChange] += srcIPv6Step[srcSixteenOctetToChange];
                                                                        else
                                                                        {
                                                                            rc = GT_NO_MORE;
                                                                            break;
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        else /* changeGrpAddr==GT_TRUE*/
        {
            /* advance to the next IP address */
            if (grpIPv6Val[grpFirstOctetToChange] + IPv6Step[grpFirstOctetToChange] <= 255)
                grpCurrentIpAddr.arIP[grpFirstOctetToChange] += IPv6Step[grpFirstOctetToChange];
            else
            {
                grpCurrentIpAddr.arIP[grpFirstOctetToChange] = 0;
                if (grpIPv6Val[grpSecondOctetToChange] + IPv6Step[grpSecondOctetToChange] <= 255)
                    grpCurrentIpAddr.arIP[grpSecondOctetToChange] += IPv6Step[grpSecondOctetToChange];
                else
                {
                    grpCurrentIpAddr.arIP[grpSecondOctetToChange] = 0;
                    if (grpIPv6Val[grpThirdOctetToChange] + IPv6Step[grpThirdOctetToChange] <= 255)
                        grpCurrentIpAddr.arIP[grpThirdOctetToChange] += IPv6Step[grpThirdOctetToChange];
                    else
                    {
                        grpCurrentIpAddr.arIP[grpThirdOctetToChange] = 0;
                        if (grpIPv6Val[grpFourthOctetToChange] + IPv6Step[grpFourthOctetToChange] <= 255)
                            grpCurrentIpAddr.arIP[grpFourthOctetToChange] += IPv6Step[grpFourthOctetToChange];
                        else
                        {
                            grpCurrentIpAddr.arIP[grpFourthOctetToChange] = 0;
                            if (grpIPv6Val[grpFifthOctetToChange] + IPv6Step[grpFifthOctetToChange] <= 255)
                                grpCurrentIpAddr.arIP[grpFifthOctetToChange] += IPv6Step[grpFifthOctetToChange];
                            else
                            {
                                grpCurrentIpAddr.arIP[grpFifthOctetToChange] = 0;
                                if (grpIPv6Val[grpSixthOctetToChange] + IPv6Step[grpSixthOctetToChange] <= 255)
                                    grpCurrentIpAddr.arIP[grpSixthOctetToChange] += IPv6Step[grpSixthOctetToChange];
                                else
                                {
                                    grpCurrentIpAddr.arIP[grpSixthOctetToChange] = 0;
                                    if (grpIPv6Val[grpSeventhOctetToChange] + IPv6Step[grpSeventhOctetToChange] <= 255)
                                        grpCurrentIpAddr.arIP[grpSeventhOctetToChange] += IPv6Step[grpSeventhOctetToChange];
                                    else
                                    {
                                        grpCurrentIpAddr.arIP[grpSeventhOctetToChange] = 0;
                                        if (grpIPv6Val[grpEighthOctetToChange] + IPv6Step[grpEighthOctetToChange] <= 255)
                                            grpCurrentIpAddr.arIP[grpEighthOctetToChange] += IPv6Step[grpEighthOctetToChange];
                                        else
                                        {
                                            grpCurrentIpAddr.arIP[grpEighthOctetToChange] = 0;
                                            if (grpIPv6Val[grpNinthOctetToChange] + IPv6Step[grpNinthOctetToChange] <= 255)
                                                grpCurrentIpAddr.arIP[grpNinthOctetToChange] += IPv6Step[grpNinthOctetToChange];
                                            else
                                            {
                                                grpCurrentIpAddr.arIP[grpNinthOctetToChange] = 0;
                                                if (grpIPv6Val[grpTenthOctetToChange] + IPv6Step[grpTenthOctetToChange] <= 255)
                                                    grpCurrentIpAddr.arIP[grpTenthOctetToChange] += IPv6Step[grpTenthOctetToChange];
                                                else
                                                {
                                                    grpCurrentIpAddr.arIP[grpTenthOctetToChange] = 0;
                                                    if (grpIPv6Val[grpElevenOctetToChange] + IPv6Step[grpElevenOctetToChange] <= 255)
                                                        grpCurrentIpAddr.arIP[grpElevenOctetToChange] += IPv6Step[grpElevenOctetToChange];
                                                    else
                                                    {
                                                        grpCurrentIpAddr.arIP[grpElevenOctetToChange] = 0;
                                                        if (grpIPv6Val[grpTwelveOctetToChange] + IPv6Step[grpTwelveOctetToChange] <= 255)
                                                            grpCurrentIpAddr.arIP[grpTwelveOctetToChange] += IPv6Step[grpTwelveOctetToChange];
                                                        else
                                                        {
                                                            grpCurrentIpAddr.arIP[grpTwelveOctetToChange] = 0;
                                                            if (grpIPv6Val[grpThirteenOctetToChange] + IPv6Step[grpThirteenOctetToChange] <= 255)
                                                                grpCurrentIpAddr.arIP[grpThirteenOctetToChange] += IPv6Step[grpThirteenOctetToChange];
                                                            else
                                                            {
                                                                grpCurrentIpAddr.arIP[grpThirteenOctetToChange] = 0;
                                                                if (grpIPv6Val[grpFourteenOctetToChange] + IPv6Step[grpFourteenOctetToChange] <= 255)
                                                                    grpCurrentIpAddr.arIP[grpFourteenOctetToChange] += IPv6Step[grpFourteenOctetToChange];
                                                                else
                                                                {
                                                                    grpCurrentIpAddr.arIP[grpFourteenOctetToChange] = 0;
                                                                    if (grpIPv6Val[grpFifteenOctetToChange] + IPv6Step[grpFifteenOctetToChange] <= 255)
                                                                        grpCurrentIpAddr.arIP[grpFifteenOctetToChange] += IPv6Step[grpFifteenOctetToChange];
                                                                    else
                                                                    {
                                                                        grpCurrentIpAddr.arIP[grpFifteenOctetToChange] = 0;
                                                                        if (grpIPv6Val[grpSixteenOctetToChange] + IPv6Step[grpSixteenOctetToChange] <= 255)
                                                                            grpCurrentIpAddr.arIP[grpSixteenOctetToChange] += IPv6Step[grpSixteenOctetToChange];
                                                                        else
                                                                        {
                                                                            rc = GT_NO_MORE;
                                                                            break;
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    /* print result */
    cpssOsPrintf("\nIpv6McPrefixGetMany results:");
    if (numOfRetrievedPrefixes == 0)
    {
        cpssOsPrintf("\n    No prefixes at all.\n");
    }
    else
    {

        cpssOsPrintf("\n    Group Prefixes ");
        outputIPAddress(grpStartIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
        cpssOsPrintf(" - ");
        outputIPAddress(grpLastIpAddrAdded.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
        cpssOsPrintf("\n    Source Prefixes ");
        outputIPAddress(srcStartIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
        cpssOsPrintf(" - ");
        outputIPAddress(srcLastIpAddrAdded.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
        cpssOsPrintf(" were retrieved (%d/%d)\n",numOfRetrievedPrefixes,numOfPrefixesToGet);
    }

    /* update output parameters */
    if (numOfRetrievedPrefixesPtr != NULL)
        *numOfRetrievedPrefixesPtr = numOfRetrievedPrefixes;

    /* Return to default */
    for (i = 0; i < 16; i++) {
        IPv6Step[i] = 1;
        srcIPv6Step[i] = 1;
    }
    IPv6PrefixLength = 128;
    srcIPv6PrefixLength = 128;

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpLpmDbgIpv6McPrefixGetManyByOctet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, lpmDbId, vrId, grpStartIpAddr, srcStartIpAddr,
                            numOfPrefixesToGet, changeGrpAddr,
                            grpFirstOctetToChange, grpSecondOctetToChange,
                            grpThirdOctetToChange, grpFourthOctetToChange,
                            grpFifthOctetToChange, grpSixthOctetToChange,
                            grpSeventhOctetToChange, grpEighthOctetToChange,
                            grpNinthOctetToChange, grpTenthOctetToChange,
                            grpElevenOctetToChange, grpTwelveOctetToChange,
                            grpThirteenOctetToChange, grpFourteenOctetToChange,
                            grpFifteenOctetToChange, grpSixteenOctetToChange,
                            changeSrcAddr, srcFirstOctetToChange, srcSecondOctetToChange,
                            srcThirdOctetToChange, srcFourthOctetToChange,
                            srcFifthOctetToChange, srcSixthOctetToChange,
                            srcSeventhOctetToChange, srcEighthOctetToChange,
                            srcNinthOctetToChange, srcTenthOctetToChange,
                            srcElevenOctetToChange, srcTwelveOctetToChange,
                            srcThirteenOctetToChange, srcFourteenOctetToChange,
                            srcFifteenOctetToChange, srcSixteenOctetToChange,
                            numOfRetrievedPrefixesPtr));

    rc = internal_cpssDxChIpLpmDbgIpv6McPrefixGetManyByOctet(lpmDbId, vrId,
                                                             grpStartIpAddr,
                                                             srcStartIpAddr,
                                                             numOfPrefixesToGet,
                                                             changeGrpAddr,
                                                             grpFirstOctetToChange,
                                                             grpSecondOctetToChange,
                                                             grpThirdOctetToChange,
                                                             grpFourthOctetToChange,
                                                             grpFifthOctetToChange,
                                                             grpSixthOctetToChange,
                                                             grpSeventhOctetToChange,
                                                             grpEighthOctetToChange,
                                                             grpNinthOctetToChange,
                                                             grpTenthOctetToChange,
                                                             grpElevenOctetToChange,
                                                             grpTwelveOctetToChange,
                                                             grpThirteenOctetToChange,
                                                             grpFourteenOctetToChange,
                                                             grpFifteenOctetToChange,
                                                             grpSixteenOctetToChange,
                                                             changeSrcAddr,
                                                             srcFirstOctetToChange,
                                                             srcSecondOctetToChange,
                                                             srcThirdOctetToChange,
                                                             srcFourthOctetToChange,
                                                             srcFifthOctetToChange,
                                                             srcSixthOctetToChange,
                                                             srcSeventhOctetToChange,
                                                             srcEighthOctetToChange,
                                                             srcNinthOctetToChange,
                                                             srcTenthOctetToChange,
                                                             srcElevenOctetToChange,
                                                             srcTwelveOctetToChange,
                                                             srcThirteenOctetToChange,
                                                             srcFourteenOctetToChange,
                                                             srcFifteenOctetToChange,
                                                             srcSixteenOctetToChange,
                                                             numOfRetrievedPrefixesPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, lpmDbId, vrId, grpStartIpAddr,
                              srcStartIpAddr, numOfPrefixesToGet, changeGrpAddr,
                              grpFirstOctetToChange, grpSecondOctetToChange,
                              grpThirdOctetToChange, grpFourthOctetToChange,
                              grpFifthOctetToChange, grpSixthOctetToChange,
                              grpSeventhOctetToChange, grpEighthOctetToChange,
                              grpNinthOctetToChange, grpTenthOctetToChange,
                              grpElevenOctetToChange, grpTwelveOctetToChange,
                              grpThirteenOctetToChange, grpFourteenOctetToChange,
                              grpFifteenOctetToChange, grpSixteenOctetToChange,
                              changeSrcAddr, srcFirstOctetToChange, srcSecondOctetToChange,
                              srcThirdOctetToChange, srcFourthOctetToChange,
                              srcFifthOctetToChange, srcSixthOctetToChange,
                              srcSeventhOctetToChange, srcEighthOctetToChange,
                              srcNinthOctetToChange, srcTenthOctetToChange,
                              srcElevenOctetToChange, srcTwelveOctetToChange,
                              srcThirteenOctetToChange, srcFourteenOctetToChange,
                              srcFifteenOctetToChange, srcSixteenOctetToChange,
                              numOfRetrievedPrefixesPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpLpmDbgHwOctetPerBlockPrint function
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
static GT_STATUS internal_cpssDxChIpLpmDbgHwOctetPerBlockPrint
(
    IN GT_U32                           lpmDbId
)
{
    PRV_CPSS_DXCH_LPM_SHADOW_STC        *lpmDbPtr,tmpLpmDb;
    PRV_CPSS_DXCH_LPM_HW_ENT            lpmHw;

    tmpLpmDb.lpmDbId = lpmDbId;
    lpmDbPtr = prvCpssSlSearch(prvCpssDxChIplpmDbSL,&tmpLpmDb);
    if (lpmDbPtr == NULL)
    {
        /* can't find the lpm DB */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    lpmHw = prvCpssDxChLpmGetHwType(lpmDbPtr->shadowType);
    switch (lpmHw)
    {
        case PRV_CPSS_DXCH_LPM_HW_TCAM_E:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);

        case PRV_CPSS_DXCH_LPM_HW_RAM_E:
            prvCpssDxChLpmDbgHwOctetPerBlockPrint((PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC*)(lpmDbPtr->shadow));
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}
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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpLpmDbgHwOctetPerBlockPrint);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, lpmDbId));

    rc = internal_cpssDxChIpLpmDbgHwOctetPerBlockPrint(lpmDbId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, lpmDbId));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpLpmDbgHwOctetPerProtocolPrint function
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
static GT_STATUS internal_cpssDxChIpLpmDbgHwOctetPerProtocolPrint
(
    IN GT_U32                           lpmDbId
)
{
    PRV_CPSS_DXCH_LPM_SHADOW_STC        *lpmDbPtr,tmpLpmDb;
    PRV_CPSS_DXCH_LPM_HW_ENT            lpmHw;

    tmpLpmDb.lpmDbId = lpmDbId;
    lpmDbPtr = prvCpssSlSearch(prvCpssDxChIplpmDbSL,&tmpLpmDb);
    if (lpmDbPtr == NULL)
    {
        /* can't find the lpm DB */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    lpmHw = prvCpssDxChLpmGetHwType(lpmDbPtr->shadowType);
    switch (lpmHw)
    {
        case PRV_CPSS_DXCH_LPM_HW_TCAM_E:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);

        case PRV_CPSS_DXCH_LPM_HW_RAM_E:
            prvCpssDxChLpmDbgHwOctetPerProtocolPrint((PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC*)(lpmDbPtr->shadow));
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpLpmDbgHwOctetPerProtocolPrint);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, lpmDbId));

    rc = internal_cpssDxChIpLpmDbgHwOctetPerProtocolPrint(lpmDbId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, lpmDbId));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters function
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
static GT_STATUS internal_cpssDxChIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters
(
    IN GT_U32                           lpmDbId
)
{
    PRV_CPSS_DXCH_LPM_SHADOW_STC        *lpmDbPtr,tmpLpmDb;
    PRV_CPSS_DXCH_LPM_HW_ENT            lpmHw;

    tmpLpmDb.lpmDbId = lpmDbId;
    lpmDbPtr = prvCpssSlSearch(prvCpssDxChIplpmDbSL,&tmpLpmDb);
    if (lpmDbPtr == NULL)
    {
        /* can't find the lpm DB */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    lpmHw = prvCpssDxChLpmGetHwType(lpmDbPtr->shadowType);
    switch (lpmHw)
    {
        case PRV_CPSS_DXCH_LPM_HW_TCAM_E:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);

        case PRV_CPSS_DXCH_LPM_HW_RAM_E:
            prvCpssDxChLpmDbgHwOctetPerProtocolPrintLpmLinesCounters((PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC*)(lpmDbPtr->shadow));
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpLpmDbgHwOctetPerProtocolPrint);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, lpmDbId));

    rc = internal_cpssDxChIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(lpmDbId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, lpmDbId));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChIpLpmDbgHwOctetPerProtocolPrintLpmMemoryAndCounters function
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
static GT_STATUS internal_cpssDxChIpLpmDbgHwOctetPerProtocolPrintLpmMemoryAndCounters
(
    IN GT_U32                           lpmDbId
)
{
    PRV_CPSS_DXCH_LPM_SHADOW_STC        *lpmDbPtr,tmpLpmDb;
    PRV_CPSS_DXCH_LPM_HW_ENT            lpmHw;

    tmpLpmDb.lpmDbId = lpmDbId;
    lpmDbPtr = prvCpssSlSearch(prvCpssDxChIplpmDbSL,&tmpLpmDb);
    if (lpmDbPtr == NULL)
    {
        /* can't find the lpm DB */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    lpmHw = prvCpssDxChLpmGetHwType(lpmDbPtr->shadowType);
    switch (lpmHw)
    {
        case PRV_CPSS_DXCH_LPM_HW_TCAM_E:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);

        case PRV_CPSS_DXCH_LPM_HW_RAM_E:
            prvCpssDxChLpmDbgHwOctetPerProtocolPrintLpmMemoryAndCounters((PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC*)(lpmDbPtr->shadow));
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpLpmDbgHwOctetPerProtocolPrint);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, lpmDbId));

    rc = internal_cpssDxChIpLpmDbgHwOctetPerProtocolPrintLpmMemoryAndCounters(lpmDbId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, lpmDbId));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpLpmDbgIpv4McPrefixDelManyByOctet function
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
* @param[in] grpStartIpAddr           - the first group address to delete
* @param[in] srcStartIpAddr           - the first source address to delete
*                                      routeEntryBaseMemAddr - base memory Address (offset) of the route entry
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
static GT_STATUS internal_cpssDxChIpLpmDbgIpv4McPrefixDelManyByOctet
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
)
{
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  i = 0;
    GT_U32                                  j = 0;
    GT_IPADDR                               grpCurrentIpAddr;
    GT_IPADDR                               srcCurrentIpAddr;
    GT_U32                                  numOfPrefixesDeleted = 0;
    GT_U32                                  numOfPrefixesAlreadyDeleted = 0;
    GT_IPADDR                               grpLastIpAddrDeleted;
    GT_IPADDR                               srcLastIpAddrDeleted;
    GT_BOOL                                 octetsArray[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS]={GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE};/* array to hold the octets already selected */
    GT_U32                                  octetsOrderToChangeArray[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS];
    GT_U32                                  srcIPv4Val[4] = { 0 };
    GT_U32                                  grpIPv4Val[4] = { 0 };

    srcCurrentIpAddr.u32Ip                  = srcStartIpAddr.u32Ip;
    srcLastIpAddrDeleted.u32Ip              = srcStartIpAddr.u32Ip;

    grpCurrentIpAddr.u32Ip                  = grpStartIpAddr.u32Ip;
    grpLastIpAddrDeleted.u32Ip              = grpStartIpAddr.u32Ip;

    if(((changeGrpAddr==GT_TRUE)&&(changeSrcAddr==GT_TRUE))||
       ((changeGrpAddr==GT_FALSE)&&(changeSrcAddr==GT_FALSE)))
    {
        cpssOsPrintf("changeGrpAddr should be different from changeSrcAddr\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    if(changeGrpAddr==GT_TRUE)
    {
        octetsOrderToChangeArray[0] = grpFirstOctetToChange;
        octetsOrderToChangeArray[1] = grpSecondOctetToChange;
        octetsOrderToChangeArray[2] = grpThirdOctetToChange;
        octetsOrderToChangeArray[3] = grpFourthOctetToChange;
    }
    if(changeSrcAddr==GT_TRUE)
    {
        octetsOrderToChangeArray[0] = srcFirstOctetToChange;
        octetsOrderToChangeArray[1] = srcSecondOctetToChange;
        octetsOrderToChangeArray[2] = srcThirdOctetToChange;
        octetsOrderToChangeArray[3] = srcFourthOctetToChange;
    }

    for(j=0;j<PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS;j++)
    {
        if(octetsOrderToChangeArray[j]>3)
        {
            cpssOsPrintf("octetToChange can not be bigger then 3\n");
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    for(j=0;j<PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS;j++)
    {
        /* make sure octet are not selected twice */
        if(octetsArray[octetsOrderToChangeArray[j]]==GT_FALSE)
        {
            octetsArray[octetsOrderToChangeArray[j]]=GT_TRUE;
        }
        else
        {
            cpssOsPrintf("octetsOrderToChangeArray[j]=%d was selected twice\n",octetsOrderToChangeArray[j]);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    for (i = 0 ; i < numOfPrefixesToDel ; )
    {
        /* make sure the grp prefix is a MC prefix - in the range 224/4 - 239/4
           therefore the first octate must start with 0xE
           and the src prefix is not a MC prefix */
        if(((grpCurrentIpAddr.arIP[0] & 0xF0) == 0xE0)&&
           ((srcCurrentIpAddr.arIP[0] & 0xF0) != 0xE0))
        {
            /* del the current prefix */
            rc = cpssDxChIpLpmIpv4McEntryDel(lpmDbId,
                                             vrId,
                                             &grpCurrentIpAddr,
                                             IPv4PrefixLength,
                                             &srcCurrentIpAddr,
                                             srcIPv4PrefixLength);
            if ((rc != GT_OK) && (rc!=GT_NOT_FOUND))
                break;

            if (rc==GT_NOT_FOUND)
            {
                rc = GT_OK;
                numOfPrefixesAlreadyDeleted++;
            }
             else
                numOfPrefixesDeleted++;

            i++;
        }
        else
        {
            cpssOsPrintf("\n grpStartIpAddr= ");
            outputIPAddress(grpCurrentIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
            cpssOsPrintf(" srcCurrentIpAddr= ");
            outputIPAddress(srcCurrentIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
            cpssOsPrintf(" is not a valid MC address\n");

            /* print result */
            cpssOsPrintf("\nIpv4McPrefixDelMany results:");
            if (numOfPrefixesDeleted == 0)
            {
                cpssOsPrintf("\n    No prefixes were deleted at all.\n");
            }
            else
            {
                cpssOsPrintf("\n    Group Prefixes ");
                outputIPAddress(grpStartIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
                cpssOsPrintf(" - ");
                outputIPAddress(grpLastIpAddrDeleted.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
                cpssOsPrintf("\n    Source Prefixes ");
                outputIPAddress(srcStartIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
                cpssOsPrintf(" - ");
                outputIPAddress(srcLastIpAddrDeleted.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
                cpssOsPrintf(" were deleted (%d/%d)\n",numOfPrefixesDeleted,numOfPrefixesToDel);
                cpssOsPrintf("    (%d) already deleted \n",numOfPrefixesAlreadyDeleted);
                cpssOsPrintf("    total number of prefixes deleted plus not_found (%d/%d) \n",(numOfPrefixesAlreadyDeleted+numOfPrefixesDeleted),numOfPrefixesToDel);

            }

            /* update output parameters */
            if (numOfPrefixesDeletedPtr != NULL)
                *numOfPrefixesDeletedPtr = numOfPrefixesDeleted;
            if (grpLastIpAddrDeletedPtr != NULL)
                grpLastIpAddrDeletedPtr->u32Ip = grpLastIpAddrDeleted.u32Ip;
            if (srcLastIpAddrDeletedPtr != NULL)
                srcLastIpAddrDeletedPtr->u32Ip = srcLastIpAddrDeleted.u32Ip;

            /* Return to default */
            for (i = 0; i < 4; i++) {
                IPv4Step[i] = 1;
                srcIPv4Step[i] = 1;
            }
            IPv4PrefixLength = 32;
            srcIPv4PrefixLength = 32;

            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }


        srcLastIpAddrDeleted.u32Ip                = srcCurrentIpAddr.u32Ip;
        grpLastIpAddrDeleted.u32Ip                = grpCurrentIpAddr.u32Ip;

        srcIPv4Val[3]=srcCurrentIpAddr.arIP[3];
        srcIPv4Val[2]=srcCurrentIpAddr.arIP[2];
        srcIPv4Val[1]=srcCurrentIpAddr.arIP[1];
        srcIPv4Val[0]=srcCurrentIpAddr.arIP[0];

        grpIPv4Val[3]=grpCurrentIpAddr.arIP[3];
        grpIPv4Val[2]=grpCurrentIpAddr.arIP[2];
        grpIPv4Val[1]=grpCurrentIpAddr.arIP[1];
        grpIPv4Val[0]=grpCurrentIpAddr.arIP[0];

        if(changeSrcAddr==GT_TRUE)
        {
            /* advance to the next IP address */
            if (srcIPv4Val[srcFirstOctetToChange] + srcIPv4Step[srcFirstOctetToChange] <= 255)
                srcCurrentIpAddr.arIP[srcFirstOctetToChange] += srcIPv4Step[srcFirstOctetToChange];
            else
            {
                srcCurrentIpAddr.arIP[srcFirstOctetToChange] = 0;
                if (srcIPv4Val[srcSecondOctetToChange] + srcIPv4Step[srcSecondOctetToChange] <= 255)
                    srcCurrentIpAddr.arIP[srcSecondOctetToChange] += srcIPv4Step[srcSecondOctetToChange];
                else
                {
                    srcCurrentIpAddr.arIP[srcSecondOctetToChange] = 0;
                    if (srcIPv4Val[srcThirdOctetToChange] + srcIPv4Step[srcThirdOctetToChange] <= 255)
                        srcCurrentIpAddr.arIP[srcThirdOctetToChange] += srcIPv4Step[srcThirdOctetToChange];
                    else
                    {
                        srcCurrentIpAddr.arIP[srcThirdOctetToChange] = 0;
                        if (srcIPv4Val[srcFourthOctetToChange] + srcIPv4Step[srcFourthOctetToChange] <= 255)
                            srcCurrentIpAddr.arIP[srcFourthOctetToChange] += srcIPv4Step[srcFourthOctetToChange];
                        else
                        {
                            rc = GT_NO_MORE;
                            break;
                        }
                    }
                }
            }
        }
        else /* changeGrpAddr==GT_TRUE*/
        {
            /* advance to the next IP address */
            if (grpIPv4Val[grpFirstOctetToChange] + IPv4Step[grpFirstOctetToChange] <= 255)
                grpCurrentIpAddr.arIP[grpFirstOctetToChange] += IPv4Step[grpFirstOctetToChange];
            else
            {
                grpCurrentIpAddr.arIP[grpFirstOctetToChange] = 0;
                if (grpIPv4Val[grpSecondOctetToChange] + IPv4Step[grpSecondOctetToChange] <= 255)
                    grpCurrentIpAddr.arIP[grpSecondOctetToChange] += IPv4Step[grpSecondOctetToChange];
                else
                {
                    grpCurrentIpAddr.arIP[grpSecondOctetToChange] = 0;
                    if (grpIPv4Val[grpThirdOctetToChange] + IPv4Step[grpThirdOctetToChange] <= 255)
                        grpCurrentIpAddr.arIP[grpThirdOctetToChange] += IPv4Step[grpThirdOctetToChange];
                    else
                    {
                        grpCurrentIpAddr.arIP[grpThirdOctetToChange] = 0;
                        if (grpIPv4Val[grpFourthOctetToChange] + IPv4Step[grpFourthOctetToChange] <= 255)
                            grpCurrentIpAddr.arIP[grpFourthOctetToChange] += IPv4Step[grpFourthOctetToChange];
                        else
                        {
                            rc = GT_NO_MORE;
                            break;
                        }
                    }
                }
            }
        }
    }

    /* print result */
    cpssOsPrintf("\nIpv4McPrefixDelMany results:");
    if (numOfPrefixesDeleted == 0)
    {
        cpssOsPrintf("\n    No prefixes were deleted at all.\n");
    }
    else
    {
        cpssOsPrintf("\n    Group Prefixes ");
        outputIPAddress(grpStartIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
        cpssOsPrintf(" - ");
        outputIPAddress(grpLastIpAddrDeleted.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
        cpssOsPrintf("\n    Source Prefixes ");
        outputIPAddress(srcStartIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
        cpssOsPrintf(" - ");
        outputIPAddress(srcLastIpAddrDeleted.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS, GT_TRUE);
        cpssOsPrintf(" were deleted (%d/%d)\n",numOfPrefixesDeleted,numOfPrefixesToDel);
        cpssOsPrintf("    (%d) already deleted \n",numOfPrefixesAlreadyDeleted);
        cpssOsPrintf("    total number of prefixes deleted plus not_found (%d/%d) \n",(numOfPrefixesAlreadyDeleted+numOfPrefixesDeleted),numOfPrefixesToDel);

    }

    /* update output parameters */
    if (numOfPrefixesDeletedPtr != NULL)
        *numOfPrefixesDeletedPtr = numOfPrefixesDeleted;
    if (grpLastIpAddrDeletedPtr != NULL)
        grpLastIpAddrDeletedPtr->u32Ip = grpLastIpAddrDeleted.u32Ip;
    if (srcLastIpAddrDeletedPtr != NULL)
        srcLastIpAddrDeletedPtr->u32Ip = srcLastIpAddrDeleted.u32Ip;

    /* Return to default */
    for (i = 0; i < 4; i++) {
        IPv4Step[i] = 1;
        srcIPv4Step[i] = 1;
    }
    IPv4PrefixLength = 32;
    srcIPv4PrefixLength = 32;

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpLpmDbgIpv4McPrefixDelManyByOctet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, lpmDbId, vrId, grpStartIpAddr, srcStartIpAddr,
                            numOfPrefixesToDel, changeGrpAddr,
                            grpFirstOctetToChange, grpSecondOctetToChange,
                            grpThirdOctetToChange, grpFourthOctetToChange, changeSrcAddr,
                            srcFirstOctetToChange, srcSecondOctetToChange,
                            srcThirdOctetToChange, srcFourthOctetToChange,
                            grpLastIpAddrDeletedPtr, srcLastIpAddrDeletedPtr,
                            numOfPrefixesDeletedPtr));

    rc = internal_cpssDxChIpLpmDbgIpv4McPrefixDelManyByOctet(lpmDbId, vrId,
                                                             grpStartIpAddr,
                                                             srcStartIpAddr,
                                                             numOfPrefixesToDel,
                                                             changeGrpAddr,
                                                             grpFirstOctetToChange,
                                                             grpSecondOctetToChange,
                                                             grpThirdOctetToChange,
                                                             grpFourthOctetToChange,
                                                             changeSrcAddr,
                                                             srcFirstOctetToChange,
                                                             srcSecondOctetToChange,
                                                             srcThirdOctetToChange,
                                                             srcFourthOctetToChange,
                                                             grpLastIpAddrDeletedPtr,
                                                             srcLastIpAddrDeletedPtr,
                                                             numOfPrefixesDeletedPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, lpmDbId, vrId, grpStartIpAddr, srcStartIpAddr,
                              numOfPrefixesToDel, changeGrpAddr,
                              grpFirstOctetToChange, grpSecondOctetToChange,
                              grpThirdOctetToChange, grpFourthOctetToChange, changeSrcAddr,
                              srcFirstOctetToChange, srcSecondOctetToChange,
                              srcThirdOctetToChange, srcFourthOctetToChange,
                              grpLastIpAddrDeletedPtr, srcLastIpAddrDeletedPtr,
                              numOfPrefixesDeletedPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpLpmDbgIpv6McPrefixDelManyByOctet function
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
static GT_STATUS internal_cpssDxChIpLpmDbgIpv6McPrefixDelManyByOctet
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
)
{
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  i = 0;
    GT_U32                                  j = 0;
    GT_IPV6ADDR                             grpCurrentIpAddr;
    GT_IPV6ADDR                             srcCurrentIpAddr;
    GT_U32                                  numOfPrefixesDeleted = 0;
    GT_U32                                  numOfPrefixesAlreadyDeleted = 0;
    GT_IPV6ADDR                             grpLastIpAddrDeleted;
    GT_IPV6ADDR                             srcLastIpAddrDeleted;
    GT_BOOL                                 octetsArray[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS]={GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE,
                                                         GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE,
                                                         GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE,
                                                         GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE};/* array to hold the octets already selected */

    GT_U32                                  octetsOrderToChangeArray[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS];
    GT_U32                                  srcIPv6Val[16] = { 0 };
    GT_U32                                  grpIPv6Val[16] = { 0 };

    if(((changeGrpAddr==GT_TRUE)&&(changeSrcAddr==GT_TRUE))||
       ((changeGrpAddr==GT_FALSE)&&(changeSrcAddr==GT_FALSE)))
    {
        cpssOsPrintf("changeGrpAddr should be different from changeSrcAddr\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    if(changeGrpAddr==GT_TRUE)
    {
        octetsOrderToChangeArray[0] = grpFirstOctetToChange;
        octetsOrderToChangeArray[1] = grpSecondOctetToChange;
        octetsOrderToChangeArray[2] = grpThirdOctetToChange;
        octetsOrderToChangeArray[3] = grpFourthOctetToChange;
        octetsOrderToChangeArray[4] = grpFifthOctetToChange;
        octetsOrderToChangeArray[5] = grpSixthOctetToChange;
        octetsOrderToChangeArray[6] = grpSeventhOctetToChange;
        octetsOrderToChangeArray[7] = grpEighthOctetToChange;
        octetsOrderToChangeArray[8] = grpNinthOctetToChange;
        octetsOrderToChangeArray[9] = grpTenthOctetToChange;
        octetsOrderToChangeArray[10] = grpElevenOctetToChange;
        octetsOrderToChangeArray[11] = grpTwelveOctetToChange;
        octetsOrderToChangeArray[12] = grpThirteenOctetToChange;
        octetsOrderToChangeArray[13] = grpFourteenOctetToChange;
        octetsOrderToChangeArray[14] = grpFifteenOctetToChange;
        octetsOrderToChangeArray[15] = grpSixteenOctetToChange;
    }
    if(changeSrcAddr==GT_TRUE)
    {
        octetsOrderToChangeArray[0] = srcFirstOctetToChange;
        octetsOrderToChangeArray[1] = srcSecondOctetToChange;
        octetsOrderToChangeArray[2] = srcThirdOctetToChange;
        octetsOrderToChangeArray[3] = srcFourthOctetToChange;
        octetsOrderToChangeArray[4] = srcFifthOctetToChange;
        octetsOrderToChangeArray[5] = srcSixthOctetToChange;
        octetsOrderToChangeArray[6] = srcSeventhOctetToChange;
        octetsOrderToChangeArray[7] = srcEighthOctetToChange;
        octetsOrderToChangeArray[8] = srcNinthOctetToChange;
        octetsOrderToChangeArray[9] = srcTenthOctetToChange;
        octetsOrderToChangeArray[10] = srcElevenOctetToChange;
        octetsOrderToChangeArray[11] = srcTwelveOctetToChange;
        octetsOrderToChangeArray[12] = srcThirteenOctetToChange;
        octetsOrderToChangeArray[13] = srcFourteenOctetToChange;
        octetsOrderToChangeArray[14] = srcFifteenOctetToChange;
        octetsOrderToChangeArray[15] = srcSixteenOctetToChange;
    }

    for(j=0;j<PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS;j++)
    {
        if(octetsOrderToChangeArray[j]>15)
        {
            cpssOsPrintf("octetToChange can not be bigger then 15\n");
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    for(j=0;j<PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS;j++)
    {
        /* make sure octet are not selected twice */
        if(octetsArray[octetsOrderToChangeArray[j]]==GT_FALSE)
        {
            octetsArray[octetsOrderToChangeArray[j]]=GT_TRUE;
        }
        else
        {
            cpssOsPrintf("octetsOrderToChangeArray[j]=%d was selected twice\n",octetsOrderToChangeArray[j]);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    cpssOsMemSet(&grpCurrentIpAddr, 0, sizeof(GT_IPV6ADDR));
    cpssOsMemSet(&srcCurrentIpAddr, 0, sizeof(GT_IPV6ADDR));

    for(j=0;j<4;j++)
    {
        grpCurrentIpAddr.u32Ip[j]      = grpStartIpAddr.u32Ip[j];
        grpLastIpAddrDeleted.u32Ip[j]    = grpStartIpAddr.u32Ip[j];

        srcCurrentIpAddr.u32Ip[j]      = srcStartIpAddr.u32Ip[j];
        srcLastIpAddrDeleted.u32Ip[j]    = srcStartIpAddr.u32Ip[j];
    }

   for (i = 0 ; i < numOfPrefixesToDel ; )
    {
        /* make sure the prefix is not a MC prefix - not in format ff00::/8
           therefore the first octate must not start with 0xFF
           and the src prefix is not a MC prefix */
        if((grpCurrentIpAddr.arIP[0] == 0xFF)&&
           (srcCurrentIpAddr.arIP[0] != 0xFF))
        {
            /* delete the current prefix */
            rc = cpssDxChIpLpmIpv6McEntryDel(lpmDbId,
                                            vrId,
                                            &grpCurrentIpAddr,
                                            IPv6PrefixLength,
                                            &srcCurrentIpAddr,
                                            srcIPv6PrefixLength);
            if ((rc != GT_OK) && (rc!=GT_NOT_FOUND))
                break;

            if (rc==GT_NOT_FOUND)
            {
                rc = GT_OK;
                numOfPrefixesAlreadyDeleted++;
            }
            else
                numOfPrefixesDeleted++;
            i++;
        }
        else
        {
            cpssOsPrintf("\n grpStartIpAddr= ");
            outputIPAddress(grpCurrentIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
            cpssOsPrintf(" srcCurrentIpAddr= ");
            outputIPAddress(srcCurrentIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
            cpssOsPrintf(" is not a valid MC address\n");

            /* print result */
            cpssOsPrintf("\nIpv6McPrefixAddMany results:");
            if (numOfPrefixesDeleted == 0)
            {
                cpssOsPrintf("\n    No prefixes were deleted at all.\n");
            }
            else
            {
                cpssOsPrintf("\n    Group Prefixes ");
                outputIPAddress(grpStartIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
                cpssOsPrintf(" - ");
                outputIPAddress(grpLastIpAddrDeleted.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
                cpssOsPrintf("\n    Source Prefixes ");
                outputIPAddress(srcStartIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
                cpssOsPrintf(" - ");
                outputIPAddress(srcLastIpAddrDeleted.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
                cpssOsPrintf(" were deleted (%d/%d)\n",numOfPrefixesDeleted,numOfPrefixesToDel);
                cpssOsPrintf("    (%d) already deleted \n",numOfPrefixesAlreadyDeleted);
                cpssOsPrintf("    total number of prefixes deleted plus not_found (%d/%d) \n",(numOfPrefixesAlreadyDeleted+numOfPrefixesDeleted),numOfPrefixesToDel);

            }

            /* update output parameters */
            if (numOfPrefixesDeletedPtr != NULL)
                *numOfPrefixesDeletedPtr = numOfPrefixesDeleted;
            if (grpLastIpAddrDeletedPtr != NULL)
            {
                for(j=0;j<4;j++)
                {
                    grpLastIpAddrDeletedPtr->u32Ip[j] = grpLastIpAddrDeleted.u32Ip[j];
                }
            }

            if (srcLastIpAddrDeletedPtr != NULL)
            {
                for(j=0;j<4;j++)
                {
                    srcLastIpAddrDeletedPtr->u32Ip[j] = srcLastIpAddrDeleted.u32Ip[j];
                }
            }

            /* Return to default */
            for (i = 0; i < 16; i++) {
                IPv6Step[i] = 1;
                srcIPv6Step[i] = 1;
            }
            IPv6PrefixLength = 128;
            srcIPv6PrefixLength = 128;

            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        for(j=0;j<4;j++)
        {
            grpLastIpAddrDeleted.u32Ip[j]    = grpCurrentIpAddr.u32Ip[j];
            srcLastIpAddrDeleted.u32Ip[j]    = srcCurrentIpAddr.u32Ip[j];
        }

        for(j=0;j<16;j++)
        {
            srcIPv6Val[j]    = srcCurrentIpAddr.arIP[j];
            grpIPv6Val[j]    = grpCurrentIpAddr.arIP[j];
        }

        if(changeSrcAddr==GT_TRUE)
        {
            /* advance to the next IP address */
            if (srcIPv6Val[srcFirstOctetToChange] + srcIPv6Step[srcFirstOctetToChange] <= 255)
                srcCurrentIpAddr.arIP[srcFirstOctetToChange] += srcIPv6Step[srcFirstOctetToChange];
            else
            {
                srcCurrentIpAddr.arIP[srcFirstOctetToChange] = 0;
                if (srcIPv6Val[srcSecondOctetToChange] + srcIPv6Step[srcSecondOctetToChange] <= 255)
                    srcCurrentIpAddr.arIP[srcSecondOctetToChange] += srcIPv6Step[srcSecondOctetToChange];
                else
                {
                    srcCurrentIpAddr.arIP[srcSecondOctetToChange] = 0;
                    if (srcIPv6Val[srcThirdOctetToChange] + srcIPv6Step[srcThirdOctetToChange] <= 255)
                        srcCurrentIpAddr.arIP[srcThirdOctetToChange] += srcIPv6Step[srcThirdOctetToChange];
                    else
                    {
                        srcCurrentIpAddr.arIP[srcThirdOctetToChange] = 0;
                        if (srcIPv6Val[srcFourthOctetToChange] + srcIPv6Step[srcFourthOctetToChange] <= 255)
                            srcCurrentIpAddr.arIP[srcFourthOctetToChange] += srcIPv6Step[srcFourthOctetToChange];
                        else
                        {
                            srcCurrentIpAddr.arIP[srcFourthOctetToChange] = 0;
                            if (srcIPv6Val[srcFifthOctetToChange] + srcIPv6Step[srcFifthOctetToChange] <= 255)
                                srcCurrentIpAddr.arIP[srcFifthOctetToChange] += srcIPv6Step[srcFifthOctetToChange];
                            else
                            {
                                srcCurrentIpAddr.arIP[srcFifthOctetToChange] = 0;
                                if (srcIPv6Val[srcSixthOctetToChange] + srcIPv6Step[srcSixthOctetToChange] <= 255)
                                    srcCurrentIpAddr.arIP[srcSixthOctetToChange] += srcIPv6Step[srcSixthOctetToChange];
                                else
                                {
                                    srcCurrentIpAddr.arIP[srcSixthOctetToChange] = 0;
                                    if (srcIPv6Val[srcSeventhOctetToChange] + srcIPv6Step[srcSeventhOctetToChange] <= 255)
                                        srcCurrentIpAddr.arIP[srcSeventhOctetToChange] += srcIPv6Step[srcSeventhOctetToChange];
                                    else
                                    {
                                        srcCurrentIpAddr.arIP[srcSeventhOctetToChange] = 0;
                                        if (srcIPv6Val[srcEighthOctetToChange] + srcIPv6Step[srcEighthOctetToChange] <= 255)
                                            srcCurrentIpAddr.arIP[srcEighthOctetToChange] += srcIPv6Step[srcEighthOctetToChange];
                                        else
                                        {
                                            srcCurrentIpAddr.arIP[srcEighthOctetToChange] = 0;
                                            if (srcIPv6Val[srcNinthOctetToChange] + srcIPv6Step[srcNinthOctetToChange] <= 255)
                                                srcCurrentIpAddr.arIP[srcNinthOctetToChange] += srcIPv6Step[srcNinthOctetToChange];
                                            else
                                            {
                                                srcCurrentIpAddr.arIP[srcNinthOctetToChange] = 0;
                                                if (srcIPv6Val[srcTenthOctetToChange] + srcIPv6Step[srcTenthOctetToChange] <= 255)
                                                    srcCurrentIpAddr.arIP[srcTenthOctetToChange] += srcIPv6Step[srcTenthOctetToChange];
                                                else
                                                {
                                                    srcCurrentIpAddr.arIP[srcTenthOctetToChange] = 0;
                                                    if (srcIPv6Val[srcElevenOctetToChange] + srcIPv6Step[srcElevenOctetToChange] <= 255)
                                                        srcCurrentIpAddr.arIP[srcElevenOctetToChange] += srcIPv6Step[srcElevenOctetToChange];
                                                    else
                                                    {
                                                        srcCurrentIpAddr.arIP[srcElevenOctetToChange] = 0;
                                                        if (srcIPv6Val[srcTwelveOctetToChange] + srcIPv6Step[srcTwelveOctetToChange] <= 255)
                                                            srcCurrentIpAddr.arIP[srcTwelveOctetToChange] += srcIPv6Step[srcTwelveOctetToChange];
                                                        else
                                                        {
                                                            srcCurrentIpAddr.arIP[srcTwelveOctetToChange] = 0;
                                                            if (srcIPv6Val[srcThirteenOctetToChange] + srcIPv6Step[srcThirteenOctetToChange] <= 255)
                                                                srcCurrentIpAddr.arIP[srcThirteenOctetToChange] += srcIPv6Step[srcThirteenOctetToChange];
                                                            else
                                                            {
                                                                srcCurrentIpAddr.arIP[srcThirteenOctetToChange] = 0;
                                                                if (srcIPv6Val[srcFourteenOctetToChange] + srcIPv6Step[srcFourteenOctetToChange] <= 255)
                                                                    srcCurrentIpAddr.arIP[srcFourteenOctetToChange] += srcIPv6Step[srcFourteenOctetToChange];
                                                                else
                                                                {
                                                                    srcCurrentIpAddr.arIP[srcFourteenOctetToChange] = 0;
                                                                    if (srcIPv6Val[srcFifteenOctetToChange] + srcIPv6Step[srcFifteenOctetToChange] <= 255)
                                                                        srcCurrentIpAddr.arIP[srcFifteenOctetToChange] += srcIPv6Step[srcFifteenOctetToChange];
                                                                    else
                                                                    {
                                                                        srcCurrentIpAddr.arIP[srcFifteenOctetToChange] = 0;
                                                                        if (srcIPv6Val[srcSixteenOctetToChange] + srcIPv6Step[srcSixteenOctetToChange] <= 255)
                                                                            srcCurrentIpAddr.arIP[srcSixteenOctetToChange] += srcIPv6Step[srcSixteenOctetToChange];
                                                                        else
                                                                        {
                                                                            rc = GT_NO_MORE;
                                                                            break;
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        else /* changeGrpAddr==GT_TRUE*/
        {
            /* advance to the next IP address */
            if (grpIPv6Val[grpFirstOctetToChange] + IPv6Step[grpFirstOctetToChange] <= 255)
                grpCurrentIpAddr.arIP[grpFirstOctetToChange] += IPv6Step[grpFirstOctetToChange];
            else
            {
                grpCurrentIpAddr.arIP[grpFirstOctetToChange] = 0;
                if (grpIPv6Val[grpSecondOctetToChange] + IPv6Step[grpSecondOctetToChange] <= 255)
                    grpCurrentIpAddr.arIP[grpSecondOctetToChange] += IPv6Step[grpSecondOctetToChange];
                else
                {
                    grpCurrentIpAddr.arIP[grpSecondOctetToChange] = 0;
                    if (grpIPv6Val[grpThirdOctetToChange] + IPv6Step[grpThirdOctetToChange] <= 255)
                        grpCurrentIpAddr.arIP[grpThirdOctetToChange] += IPv6Step[grpThirdOctetToChange];
                    else
                    {
                        grpCurrentIpAddr.arIP[grpThirdOctetToChange] = 0;
                        if (grpIPv6Val[grpFourthOctetToChange] + IPv6Step[grpFourthOctetToChange] <= 255)
                            grpCurrentIpAddr.arIP[grpFourthOctetToChange] += IPv6Step[grpFourthOctetToChange];
                        else
                        {
                            grpCurrentIpAddr.arIP[grpFourthOctetToChange] = 0;
                            if (grpIPv6Val[grpFifthOctetToChange] + IPv6Step[grpFifthOctetToChange] <= 255)
                                grpCurrentIpAddr.arIP[grpFifthOctetToChange] += IPv6Step[grpFifthOctetToChange];
                            else
                            {
                                grpCurrentIpAddr.arIP[grpFifthOctetToChange] = 0;
                                if (grpIPv6Val[grpSixthOctetToChange] + IPv6Step[grpSixthOctetToChange] <= 255)
                                    grpCurrentIpAddr.arIP[grpSixthOctetToChange] += IPv6Step[grpSixthOctetToChange];
                                else
                                {
                                    grpCurrentIpAddr.arIP[grpSixthOctetToChange] = 0;
                                    if (grpIPv6Val[grpSeventhOctetToChange] + IPv6Step[grpSeventhOctetToChange] <= 255)
                                        grpCurrentIpAddr.arIP[grpSeventhOctetToChange] += IPv6Step[grpSeventhOctetToChange];
                                    else
                                    {
                                        grpCurrentIpAddr.arIP[grpSeventhOctetToChange] = 0;
                                        if (grpIPv6Val[grpEighthOctetToChange] + IPv6Step[grpEighthOctetToChange] <= 255)
                                            grpCurrentIpAddr.arIP[grpEighthOctetToChange] += IPv6Step[grpEighthOctetToChange];
                                        else
                                        {
                                            grpCurrentIpAddr.arIP[grpEighthOctetToChange] = 0;
                                            if (grpIPv6Val[grpNinthOctetToChange] + IPv6Step[grpNinthOctetToChange] <= 255)
                                                grpCurrentIpAddr.arIP[grpNinthOctetToChange] += IPv6Step[grpNinthOctetToChange];
                                            else
                                            {
                                                grpCurrentIpAddr.arIP[grpNinthOctetToChange] = 0;
                                                if (grpIPv6Val[grpTenthOctetToChange] + IPv6Step[grpTenthOctetToChange] <= 255)
                                                    grpCurrentIpAddr.arIP[grpTenthOctetToChange] += IPv6Step[grpTenthOctetToChange];
                                                else
                                                {
                                                    grpCurrentIpAddr.arIP[grpTenthOctetToChange] = 0;
                                                    if (grpIPv6Val[grpElevenOctetToChange] + IPv6Step[grpElevenOctetToChange] <= 255)
                                                        grpCurrentIpAddr.arIP[grpElevenOctetToChange] += IPv6Step[grpElevenOctetToChange];
                                                    else
                                                    {
                                                        grpCurrentIpAddr.arIP[grpElevenOctetToChange] = 0;
                                                        if (grpIPv6Val[grpTwelveOctetToChange] + IPv6Step[grpTwelveOctetToChange] <= 255)
                                                            grpCurrentIpAddr.arIP[grpTwelveOctetToChange] += IPv6Step[grpTwelveOctetToChange];
                                                        else
                                                        {
                                                            grpCurrentIpAddr.arIP[grpTwelveOctetToChange] = 0;
                                                            if (grpIPv6Val[grpThirteenOctetToChange] + IPv6Step[grpThirteenOctetToChange] <= 255)
                                                                grpCurrentIpAddr.arIP[grpThirteenOctetToChange] += IPv6Step[grpThirteenOctetToChange];
                                                            else
                                                            {
                                                                grpCurrentIpAddr.arIP[grpThirteenOctetToChange] = 0;
                                                                if (grpIPv6Val[grpFourteenOctetToChange] + IPv6Step[grpFourteenOctetToChange] <= 255)
                                                                    grpCurrentIpAddr.arIP[grpFourteenOctetToChange] += IPv6Step[grpFourteenOctetToChange];
                                                                else
                                                                {
                                                                    grpCurrentIpAddr.arIP[grpFourteenOctetToChange] = 0;
                                                                    if (grpIPv6Val[grpFifteenOctetToChange] + IPv6Step[grpFifteenOctetToChange] <= 255)
                                                                        grpCurrentIpAddr.arIP[grpFifteenOctetToChange] += IPv6Step[grpFifteenOctetToChange];
                                                                    else
                                                                    {
                                                                        grpCurrentIpAddr.arIP[grpFifteenOctetToChange] = 0;
                                                                        if (grpIPv6Val[grpSixteenOctetToChange] + IPv6Step[grpSixteenOctetToChange] <= 255)
                                                                            grpCurrentIpAddr.arIP[grpSixteenOctetToChange] += IPv6Step[grpSixteenOctetToChange];
                                                                        else
                                                                        {
                                                                            rc = GT_NO_MORE;
                                                                            break;
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    /* print result */
    cpssOsPrintf("\nIpv6McPrefixAddMany results:");
    if (numOfPrefixesDeleted == 0)
    {
        cpssOsPrintf("\n    No prefixes were deleted at all.\n");
    }
    else
    {
        cpssOsPrintf("\n    Group Prefixes ");
        outputIPAddress(grpStartIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
        cpssOsPrintf(" - ");
        outputIPAddress(grpLastIpAddrDeleted.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
        cpssOsPrintf("\n    Source Prefixes ");
        outputIPAddress(srcStartIpAddr.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
        cpssOsPrintf(" - ");
        outputIPAddress(srcLastIpAddrDeleted.arIP, PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS, GT_FALSE);
        cpssOsPrintf(" were deleted (%d/%d)\n",numOfPrefixesDeleted,numOfPrefixesToDel);
        cpssOsPrintf("    (%d) already deleted \n",numOfPrefixesAlreadyDeleted);
        cpssOsPrintf("    total number of prefixes deleted plus not_found (%d/%d) \n",(numOfPrefixesAlreadyDeleted+numOfPrefixesDeleted),numOfPrefixesToDel);
    }

    /* update output parameters */
    if (numOfPrefixesDeletedPtr != NULL)
        *numOfPrefixesDeletedPtr = numOfPrefixesDeleted;
    if (grpLastIpAddrDeletedPtr != NULL)
    {
        for(j=0;j<4;j++)
        {
            grpLastIpAddrDeletedPtr->u32Ip[j] = grpLastIpAddrDeleted.u32Ip[j];
        }
    }

    if (srcLastIpAddrDeletedPtr != NULL)
    {
        for(j=0;j<4;j++)
        {
            srcLastIpAddrDeletedPtr->u32Ip[j] = srcLastIpAddrDeleted.u32Ip[j];
        }
    }

    /* Return to default */
    for (i = 0; i < 16; i++) {
        IPv6Step[i] = 1;
        srcIPv6Step[i] = 1;
    }
    IPv6PrefixLength = 128;
    srcIPv6PrefixLength = 128;

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpLpmDbgIpv6McPrefixDelManyByOctet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, lpmDbId, vrId, grpStartIpAddr, srcStartIpAddr,
                            numOfPrefixesToDel, changeGrpAddr,
                            grpFirstOctetToChange, grpSecondOctetToChange,
                            grpThirdOctetToChange, grpFourthOctetToChange,
                            grpFifthOctetToChange, grpSixthOctetToChange,
                            grpSeventhOctetToChange, grpEighthOctetToChange,
                            grpNinthOctetToChange, grpTenthOctetToChange,
                            grpElevenOctetToChange, grpTwelveOctetToChange,
                            grpThirteenOctetToChange, grpFourteenOctetToChange,
                            grpFifteenOctetToChange, grpSixteenOctetToChange,
                            changeSrcAddr, srcFirstOctetToChange, srcSecondOctetToChange,
                            srcThirdOctetToChange, srcFourthOctetToChange,
                            srcFifthOctetToChange, srcSixthOctetToChange,
                            srcSeventhOctetToChange, srcEighthOctetToChange,
                            srcNinthOctetToChange, srcTenthOctetToChange,
                            srcElevenOctetToChange, srcTwelveOctetToChange,
                            srcThirteenOctetToChange, srcFourteenOctetToChange,
                            srcFifteenOctetToChange, srcSixteenOctetToChange,
                            grpLastIpAddrDeletedPtr, srcLastIpAddrDeletedPtr,
                            numOfPrefixesDeletedPtr));

    rc = internal_cpssDxChIpLpmDbgIpv6McPrefixDelManyByOctet(lpmDbId, vrId,
                                                             grpStartIpAddr,
                                                             srcStartIpAddr,
                                                             numOfPrefixesToDel,
                                                             changeGrpAddr,
                                                             grpFirstOctetToChange,
                                                             grpSecondOctetToChange,
                                                             grpThirdOctetToChange,
                                                             grpFourthOctetToChange,
                                                             grpFifthOctetToChange,
                                                             grpSixthOctetToChange,
                                                             grpSeventhOctetToChange,
                                                             grpEighthOctetToChange,
                                                             grpNinthOctetToChange,
                                                             grpTenthOctetToChange,
                                                             grpElevenOctetToChange,
                                                             grpTwelveOctetToChange,
                                                             grpThirteenOctetToChange,
                                                             grpFourteenOctetToChange,
                                                             grpFifteenOctetToChange,
                                                             grpSixteenOctetToChange,
                                                             changeSrcAddr,
                                                             srcFirstOctetToChange,
                                                             srcSecondOctetToChange,
                                                             srcThirdOctetToChange,
                                                             srcFourthOctetToChange,
                                                             srcFifthOctetToChange,
                                                             srcSixthOctetToChange,
                                                             srcSeventhOctetToChange,
                                                             srcEighthOctetToChange,
                                                             srcNinthOctetToChange,
                                                             srcTenthOctetToChange,
                                                             srcElevenOctetToChange,
                                                             srcTwelveOctetToChange,
                                                             srcThirteenOctetToChange,
                                                             srcFourteenOctetToChange,
                                                             srcFifteenOctetToChange,
                                                             srcSixteenOctetToChange,
                                                             grpLastIpAddrDeletedPtr,
                                                             srcLastIpAddrDeletedPtr,
                                                             numOfPrefixesDeletedPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, lpmDbId, vrId, grpStartIpAddr, srcStartIpAddr,
                              numOfPrefixesToDel, changeGrpAddr,
                              grpFirstOctetToChange, grpSecondOctetToChange,
                              grpThirdOctetToChange, grpFourthOctetToChange,
                              grpFifthOctetToChange, grpSixthOctetToChange,
                              grpSeventhOctetToChange, grpEighthOctetToChange,
                              grpNinthOctetToChange, grpTenthOctetToChange,
                              grpElevenOctetToChange, grpTwelveOctetToChange,
                              grpThirteenOctetToChange, grpFourteenOctetToChange,
                              grpFifteenOctetToChange, grpSixteenOctetToChange,
                              changeSrcAddr, srcFirstOctetToChange, srcSecondOctetToChange,
                              srcThirdOctetToChange, srcFourthOctetToChange,
                              srcFifthOctetToChange, srcSixthOctetToChange,
                              srcSeventhOctetToChange, srcEighthOctetToChange,
                              srcNinthOctetToChange, srcTenthOctetToChange,
                              srcElevenOctetToChange, srcTwelveOctetToChange,
                              srcThirteenOctetToChange, srcFourteenOctetToChange,
                              srcFifteenOctetToChange, srcSixteenOctetToChange,
                              grpLastIpAddrDeletedPtr, srcLastIpAddrDeletedPtr,
                              numOfPrefixesDeletedPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpLpmDbgIpv4SetStep function
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
GT_STATUS internal_cpssDxChIpLpmDbgIpv4SetStep
(
    IN GT_U8  stepArr[4],
    IN GT_U32 prefixLength,
    IN GT_U8  srcStepArr[4],
    IN GT_U32 srcPrefixLen
)
{
    GT_U32 i;
    if (stepArr == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    for (i = 0; i < 4; i++)
    {
        if (stepArr[i] < 1 || stepArr[i] > 32 || srcStepArr[i] < 1 || srcStepArr[i] > 32)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    if (prefixLength > 32 || srcPrefixLen > 32)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    IPv4PrefixLength = prefixLength;
    srcIPv4PrefixLength = srcPrefixLen;

    for (i = 0; i < 4; i++)
    {
        IPv4Step[i] = stepArr[i];
        srcIPv4Step[i] = srcStepArr[i];
    }

    return GT_OK;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpLpmDbgIpv4SetStep);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, stepArr, prefixLength, srcStepArr, srcPrefixLen));

    rc = internal_cpssDxChIpLpmDbgIpv4SetStep(stepArr, prefixLength, srcStepArr, srcPrefixLen);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, stepArr, prefixLength, srcStepArr, srcPrefixLen));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpLpmDbgIpv6SetStep function
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
GT_STATUS internal_cpssDxChIpLpmDbgIpv6SetStep
(
    IN GT_U8  stepArr[16],
    IN GT_U32 prefixLength,
    IN GT_U8 srcStepArr[16],
    IN GT_U32 srcPrefixLen
)
{
    GT_U32 i;
    if (stepArr == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    for (i = 0; i < 16; i++)
    {
        if (stepArr[i] < 1 || stepArr[i] > 32 || srcStepArr[i] < 1 || srcStepArr[i] > 32)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    if (prefixLength > 128 || srcPrefixLen > 128)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    IPv6PrefixLength = prefixLength;
    srcIPv6PrefixLength = srcPrefixLen;

    for (i = 0; i < 16; i++)
    {
        IPv6Step[i] = stepArr[i];
        srcIPv6Step[i] = srcStepArr[i];
    }

    return GT_OK;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpLpmDbgIpv6SetStep);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, stepArr, prefixLength, srcStepArr, srcPrefixLen));

    rc = internal_cpssDxChIpLpmDbgIpv6SetStep(stepArr, prefixLength, srcStepArr, srcPrefixLen);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, stepArr, prefixLength, srcStepArr, srcPrefixLen));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal outputFcoeAddress function
* @endinternal
*
* @brief   Print FCoE address
*
* @param[in] fcoeAddr                 - FCoE address
*/
static GT_VOID outputFcoeAddress
(
    IN GT_FCID fcoeAddr
)
{
    cpssOsPrintf("0x%02X%02X%02X\n", fcoeAddr.fcid[2], fcoeAddr.fcid[1], fcoeAddr.fcid[0]);
}

/**
* @internal internal_cpssDxChIpLpmDbgFcoePrefixAddMany function
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
*                                      startIpAddr           - the first address to add
* @param[in] routeEntryBaseMemAddr    - base memory Address (offset) of the route entry
* @param[in] numOfPrefixesToAdd       - the number of prefixes to add
* @param[in] stepArr[3]               - array of steps for each octet
* @param[in] prefixLength             - prefix length
*
* @param[out] lastFcoeAddrAddedPtr     - points to the last prefix successfully
*                                      added (NULL to ignore)
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
static GT_STATUS internal_cpssDxChIpLpmDbgFcoePrefixAddMany
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
)
{
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  i = 0;
    GT_FCID                                 currentFcoeAddr;
    GT_U32                                  numOfPrefixesAdded = 0;
    GT_U32                                  numOfPrefixesAlreadyExist = 0;
    GT_FCID                                 lastFcoeAddrAdded;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT  nextHopInfo;
    GT_BOOL                                 override = GT_FALSE;
    GT_BOOL                                 defragmentationEnable = defragmentationEnableFlag;
    GT_U32                                  fcoeVal[3];

    currentFcoeAddr                            = startFcoeAddr;
    lastFcoeAddrAdded                          = startFcoeAddr;

    /* check step array values */
    for (i = 0; i < 3; i++)
    {
        if (stepArr[i] < 1 || stepArr[i] > 32)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    /* check prefix length */
    if (prefixLength > 24)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    cpssOsMemSet(&nextHopInfo, 0, sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
    nextHopInfo.ipLttEntry.routeType           = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = routeEntryBaseMemAddr;
    nextHopInfo.ipLttEntry.numOfPaths          = 0;
    nextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;
    rc = prvCpssDxChIpLpmDbgConvertRouteType(lpmDbId, &nextHopInfo.ipLttEntry);
    if (rc != GT_OK)
    {
        return rc;
    }

    for (i = 0 ; i < numOfPrefixesToAdd ; i++)
    {
        /* add the current prefix */
        rc = cpssDxChIpLpmFcoePrefixAdd(lpmDbId,vrId,&currentFcoeAddr,
                                          prefixLength,&nextHopInfo,
                                          override,defragmentationEnable);
        if ((rc != GT_OK) && (rc!=GT_ALREADY_EXIST))
                break;

        if (rc==GT_ALREADY_EXIST)
        {
            rc = GT_OK;
            numOfPrefixesAlreadyExist++;
        }
        else
            numOfPrefixesAdded++;

        lastFcoeAddrAdded = currentFcoeAddr;

        fcoeVal[2] = currentFcoeAddr.fcid[2];
        fcoeVal[1] = currentFcoeAddr.fcid[1];
        fcoeVal[0] = currentFcoeAddr.fcid[0];

        /* advance to the next IP address */
        if (fcoeVal[2] + stepArr[2] <= 255)
            currentFcoeAddr.fcid[2] += stepArr[2];
        else
        {
            currentFcoeAddr.fcid[2] = 0;
            if (fcoeVal[1] + stepArr[1] <= 255)
                currentFcoeAddr.fcid[1] += stepArr[1];
            else
            {
                currentFcoeAddr.fcid[1] = 0;
                if (fcoeVal[0] + stepArr[0] <= 255)
                    currentFcoeAddr.fcid[0] += stepArr[0];
                else
                {
                    rc = GT_NO_MORE;
                    break;
                }
            }
        }
    }

    /* print result */
    cpssOsPrintf("\nIpv4UcPrefixAddMany results:");
    if (numOfPrefixesAdded == 0)
    {
        cpssOsPrintf("\n    No prefixes were added at all.\n");
    }
    else
    {
        cpssOsPrintf("\n    Prefixes ");
        outputFcoeAddress(startFcoeAddr);
        cpssOsPrintf(" - ");
        outputFcoeAddress(lastFcoeAddrAdded);
        cpssOsPrintf(" were added (%d/%d)\n",numOfPrefixesAdded,numOfPrefixesToAdd);
        cpssOsPrintf("    (%d) already existed \n",numOfPrefixesAlreadyExist);
        cpssOsPrintf("    total number of prefixes added plus existed (%d/%d) \n",(numOfPrefixesAlreadyExist+numOfPrefixesAdded),numOfPrefixesToAdd);

    }

    /* update output parameters */
    if (numOfPrefixesAddedPtr != NULL)
        *numOfPrefixesAddedPtr = numOfPrefixesAdded;
    if (lastFcoeAddrAddedPtr != NULL)
        cpssOsMemCpy(lastFcoeAddrAddedPtr, &lastFcoeAddrAdded, sizeof(GT_FCID));

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpLpmDbgFcoePrefixAddMany);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, lpmDbId, vrId, startFcoeAddr, routeEntryBaseMemAddr,
                            numOfPrefixesToAdd, stepArr, prefixLength, lastFcoeAddrAddedPtr, numOfPrefixesAddedPtr));

    rc = internal_cpssDxChIpLpmDbgFcoePrefixAddMany(lpmDbId, vrId, startFcoeAddr,
                                                      routeEntryBaseMemAddr,
                                                      numOfPrefixesToAdd,
                                                      stepArr,
                                                      prefixLength,
                                                      lastFcoeAddrAddedPtr,
                                                      numOfPrefixesAddedPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, lpmDbId, vrId, startFcoeAddr, routeEntryBaseMemAddr,
                              numOfPrefixesToAdd, stepArr, prefixLength, lastFcoeAddrAddedPtr, numOfPrefixesAddedPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpLpmDbgFcoePrefixAddManyRandom function
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
static GT_STATUS internal_cpssDxChIpLpmDbgFcoePrefixAddManyRandom
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_FCID     startFcoeAddr,
    IN  GT_U32      routeEntryBaseMemAddr,
    IN  GT_U32      numOfPrefixesToAdd,
    IN  GT_BOOL     isWholeFCoERandom,
    IN  GT_U32      prefixLength,
    OUT GT_U32      *numOfPrefixesAddedPtr
)
{
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  i = 0;
    GT_FCID                                 currentFcoeAddr;
    GT_U32                                  numOfPrefixesAdded = 0;
    GT_U32                                  numOfPrefixesAlreadyExist = 0;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT  nextHopInfo;
    GT_BOOL                                 override = GT_FALSE;
    GT_BOOL                                 defragmentationEnable = defragmentationEnableFlag;
    GT_U32                                  counter = 0;
    GT_U32                                  maxCounterValue = 0;

    /* check prefix length */
    if (prefixLength > 24)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    cpssOsMemSet(&nextHopInfo, 0, sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
    nextHopInfo.ipLttEntry.routeType           = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = routeEntryBaseMemAddr;
    nextHopInfo.ipLttEntry.numOfPaths          = 0;
    nextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;
    rc = prvCpssDxChIpLpmDbgConvertRouteType(lpmDbId, &nextHopInfo.ipLttEntry);
    if (rc != GT_OK)
    {
        return rc;
    }

    currentFcoeAddr         = startFcoeAddr;
    currentFcoeAddr.fcid[2] = (GT_U8)(cpssOsRand() % 256);
    currentFcoeAddr.fcid[1] = (GT_U8)(cpssOsRand() % 256);
    if (isWholeFCoERandom == GT_TRUE)
    {
        currentFcoeAddr.fcid [0] = (GT_U8)(cpssOsRand() % 256);
    }

    for (i = 0 ; i < numOfPrefixesToAdd ; i++)
    {
        rc = 100;
        counter = 0;
        while (rc != GT_OK)
        {
            /* add the current prefix */
            rc = cpssDxChIpLpmFcoePrefixAdd(lpmDbId,vrId,&currentFcoeAddr,
                                          prefixLength,&nextHopInfo,
                                          override,defragmentationEnable);
            if (rc != GT_OK)
            {
                if (rc == GT_ALREADY_EXIST)
                {
                    if (counter > 20000)
                        break;

                    currentFcoeAddr.fcid[2] = (GT_U8)(cpssOsRand() % 256);
                    currentFcoeAddr.fcid[1] = (GT_U8)(cpssOsRand() % 256);
                    if (isWholeFCoERandom == GT_TRUE)
                    {
                        currentFcoeAddr.fcid[0] = (GT_U8)(cpssOsRand() % 256);
                    }

                    counter++;
                    if (counter > maxCounterValue)
                        maxCounterValue = counter;
                }
                else
                {
                    break;
                }
            }
        }

        if ((rc != GT_OK) && (rc!=GT_ALREADY_EXIST))
            break;

        if (rc==GT_ALREADY_EXIST)
        {
            rc = GT_OK;
            numOfPrefixesAlreadyExist++;
        }
        else
            numOfPrefixesAdded++;

        if ((numOfPrefixesAdded % 50) == 0 )
        {
            currentFcoeAddr.fcid[1] = (GT_U8)(cpssOsRand() % 256);
            currentFcoeAddr.fcid[0] = (GT_U8)(cpssOsRand() % 256);
        }
        currentFcoeAddr.fcid[2]= (GT_U8)(cpssOsRand() % 256);
        if (isWholeFCoERandom == GT_TRUE)
        {
            currentFcoeAddr.fcid[1] = (GT_U8)(cpssOsRand() % 256);
            currentFcoeAddr.fcid[0] = (GT_U8)(cpssOsRand() % 256);
        }
    }
    /* print result */
    cpssOsPrintf("\nFcoePrefixAddManyRandom results:");
    if (rc != GT_OK)
    {
        cpssOsPrintf("\n error = %x occured on prefix\n",rc);
        outputFcoeAddress(currentFcoeAddr);
    }

    /* update output parameters */
    if (numOfPrefixesAddedPtr != NULL)
        *numOfPrefixesAddedPtr = numOfPrefixesAdded;

    if (numOfPrefixesAdded == 0)
    {
        cpssOsPrintf("\n    No prefixes were added at all.\n");
    }
    else
    {
        cpssOsPrintf(" were added %d prefixes from %d prefixes)\n",numOfPrefixesAdded,numOfPrefixesToAdd);
    }
    cpssOsPrintf("maxCounterValue =%d\n",maxCounterValue);
    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpLpmDbgFcoePrefixAddManyRandom);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, lpmDbId, vrId, startFcoeAddr, routeEntryBaseMemAddr,
                            numOfPrefixesToAdd, isWholeFCoERandom, prefixLength, numOfPrefixesAddedPtr));

    rc = internal_cpssDxChIpLpmDbgFcoePrefixAddManyRandom(lpmDbId, vrId, startFcoeAddr,
                                                            routeEntryBaseMemAddr,
                                                            numOfPrefixesToAdd,
                                                            isWholeFCoERandom,
                                                            prefixLength,
                                                            numOfPrefixesAddedPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, lpmDbId, vrId, startFcoeAddr, routeEntryBaseMemAddr,
                              numOfPrefixesToAdd, isWholeFCoERandom, prefixLength, numOfPrefixesAddedPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpLpmDbgFcoePrefixAddManyByOctet function
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
static GT_STATUS internal_cpssDxChIpLpmDbgFcoePrefixAddManyByOctet
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
)
{
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  i = 0;
    GT_FCID                                 currentFcoeAddr;
    GT_U32                                  numOfPrefixesAdded = 0;
    GT_U32                                  numOfPrefixesAlreadyExist = 0;
    GT_FCID                                 lastFcoeAddrAdded;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT  nextHopInfo;
    GT_BOOL                                 override = GT_FALSE;
    GT_BOOL                                 defragmentationEnable = defragmentationEnableFlag;
    GT_BOOL                                 octetsArray[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_FCOE_PROTOCOL_CNS]={GT_FALSE,GT_FALSE,GT_FALSE};/* array to hold the octets already selected */
    GT_U32 fcoeVal[3] = {0};

    currentFcoeAddr                         = startFcoeAddr;
    lastFcoeAddrAdded                       = startFcoeAddr;

    cpssOsMemSet(&nextHopInfo, 0, sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
    nextHopInfo.ipLttEntry.routeType           = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = routeEntryBaseMemAddr;
    nextHopInfo.ipLttEntry.numOfPaths          = 0;
    nextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

    rc = prvCpssDxChIpLpmDbgConvertRouteType(lpmDbId, &nextHopInfo.ipLttEntry);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* check step array values */
    for (i = 0; i < 3; i++)
    {
        if (stepArr[i] < 1 || stepArr[i] > 32)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    /* check prefix length */
    if (prefixLength > 24)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if((firstOctetToChange > 2) || (secondOctetToChange > 2) || (thirdOctetToChange > 2))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    /* make sure octet are not selected twice */
    if(octetsArray[firstOctetToChange]==GT_FALSE)
    {
        octetsArray[firstOctetToChange]=GT_TRUE;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if(octetsArray[secondOctetToChange]==GT_FALSE)
    {
        octetsArray[secondOctetToChange]=GT_TRUE;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if(octetsArray[thirdOctetToChange]==GT_FALSE)
    {
        octetsArray[thirdOctetToChange]=GT_TRUE;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    for (i = 0 ; i < numOfPrefixesToAdd ; )
    {
        /* add the current prefix */
        rc = cpssDxChIpLpmFcoePrefixAdd(lpmDbId,vrId,&currentFcoeAddr,
                                            prefixLength,&nextHopInfo,
                                            override,defragmentationEnable);
        if ((rc != GT_OK) && (rc!=GT_ALREADY_EXIST))
                break;

        if (rc==GT_ALREADY_EXIST)
        {
            rc = GT_OK;
            numOfPrefixesAlreadyExist++;
        }
        else
            numOfPrefixesAdded++;
        i++;

        lastFcoeAddrAdded = currentFcoeAddr;

        fcoeVal[2] = currentFcoeAddr.fcid[2];
        fcoeVal[1] = currentFcoeAddr.fcid[1];
        fcoeVal[0] = currentFcoeAddr.fcid[0];

        /* advance to the next IP address */
        if (fcoeVal[firstOctetToChange] + stepArr[firstOctetToChange] <= 255)
            currentFcoeAddr.fcid[firstOctetToChange] += stepArr[firstOctetToChange];
        else
        {
            currentFcoeAddr.fcid[firstOctetToChange] = 0;
            if (fcoeVal[secondOctetToChange] + stepArr[secondOctetToChange] <= 255)
                currentFcoeAddr.fcid[secondOctetToChange] += stepArr[secondOctetToChange];
            else
            {
                currentFcoeAddr.fcid[secondOctetToChange] = 0;
                if (fcoeVal[thirdOctetToChange] + stepArr[thirdOctetToChange] <= 255)
                    currentFcoeAddr.fcid[thirdOctetToChange] += stepArr[thirdOctetToChange];
                else
                {
                    rc = GT_NO_MORE;
                    break;
                }
            }
        }
    }

    /* print result */
    cpssOsPrintf("\nFcoePrefixAddMany results:");
    if (numOfPrefixesAdded == 0)
    {
        cpssOsPrintf("\n    No prefixes were added at all.\n");
    }
    else
    {
        cpssOsPrintf("\n    Prefixes ");
        outputFcoeAddress(startFcoeAddr);
        cpssOsPrintf(" - ");
        outputFcoeAddress(lastFcoeAddrAdded);
        cpssOsPrintf(" were added (%d/%d)\n",numOfPrefixesAdded,numOfPrefixesToAdd);
        cpssOsPrintf("    (%d) already existed \n",numOfPrefixesAlreadyExist);
        cpssOsPrintf("    total number of prefixes added plus existed (%d/%d) \n",(numOfPrefixesAlreadyExist+numOfPrefixesAdded),numOfPrefixesToAdd);

    }

    /* update output parameters */
    if (numOfPrefixesAddedPtr != NULL)
        *numOfPrefixesAddedPtr = numOfPrefixesAdded;
    if (lastFcoeAddrAddedPtr != NULL)
        cpssOsMemCpy(lastFcoeAddrAddedPtr, &lastFcoeAddrAdded, sizeof(GT_FCID));

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpLpmDbgFcoePrefixAddManyByOctet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, lpmDbId, vrId, startFcoeAddr, routeEntryBaseMemAddr,
                            numOfPrefixesToAdd, firstOctetToChange, secondOctetToChange,
                            thirdOctetToChange, stepArr, prefixLength,
                            lastFcoeAddrAddedPtr, numOfPrefixesAddedPtr));

    rc = internal_cpssDxChIpLpmDbgFcoePrefixAddManyByOctet(lpmDbId, vrId, startFcoeAddr,
                                                             routeEntryBaseMemAddr, numOfPrefixesToAdd,
                                                             firstOctetToChange, secondOctetToChange,
                                                             thirdOctetToChange, stepArr, prefixLength,
                                                             lastFcoeAddrAddedPtr, numOfPrefixesAddedPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, lpmDbId, vrId, startFcoeAddr, routeEntryBaseMemAddr,
                              numOfPrefixesToAdd, firstOctetToChange, secondOctetToChange,
                              thirdOctetToChange, stepArr, prefixLength,
                              lastFcoeAddrAddedPtr, numOfPrefixesAddedPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpLpmDbgFcoePrefixDelManyByOctet function
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
static GT_STATUS internal_cpssDxChIpLpmDbgFcoePrefixDelManyByOctet
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
)
{
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  i = 0;
    GT_FCID                                 currentFcoeAddr;
    GT_U32                                  numOfPrefixesDeleted = 0;
    GT_U32                                  numOfPrefixesAlreadyDeleted = 0;
    GT_FCID                                 lastFcoeAddrDeleted;
    GT_BOOL                                 octetsArray[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_FCOE_PROTOCOL_CNS]={GT_FALSE,GT_FALSE,GT_FALSE};/* array to hold the octets already selected */
    GT_U32 fcoeVal[3] = {0};

    currentFcoeAddr                         = startFcoeAddr;
    lastFcoeAddrDeleted                     = startFcoeAddr;

    /* check step array values */
    for (i = 0; i < 3; i++)
    {
        if (stepArr[i] < 1 || stepArr[i] > 32)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    /* check prefix length */
    if (prefixLength > 24)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if((firstOctetToChange > 2) || (secondOctetToChange > 2) || (thirdOctetToChange > 2))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    /* make sure octet are not selected twice */
    if(octetsArray[firstOctetToChange]==GT_FALSE)
    {
        octetsArray[firstOctetToChange]=GT_TRUE;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if(octetsArray[secondOctetToChange]==GT_FALSE)
    {
        octetsArray[secondOctetToChange]=GT_TRUE;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if(octetsArray[thirdOctetToChange]==GT_FALSE)
    {
        octetsArray[thirdOctetToChange]=GT_TRUE;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    for (i = 0 ; i < numOfPrefixesToDel ; )
    {
        /* add the current prefix */
        rc = cpssDxChIpLpmFcoePrefixDel(lpmDbId,vrId,&currentFcoeAddr,
                                            prefixLength);
        if ((rc != GT_OK) && (rc!=GT_NOT_FOUND))
                break;

        if (rc==GT_NOT_FOUND)
        {
                rc = GT_OK;
                numOfPrefixesAlreadyDeleted++;
        }
        else
            numOfPrefixesDeleted++;
        i++;

        lastFcoeAddrDeleted = currentFcoeAddr;

        fcoeVal[2] = currentFcoeAddr.fcid[2];
        fcoeVal[1] = currentFcoeAddr.fcid[1];
        fcoeVal[0] = currentFcoeAddr.fcid[0];

        /* advance to the next IP address */
        if (fcoeVal[firstOctetToChange] + stepArr[firstOctetToChange] <= 255)
            currentFcoeAddr.fcid[firstOctetToChange] += stepArr[firstOctetToChange];
        else
        {
            currentFcoeAddr.fcid[firstOctetToChange] = 0;
            if (fcoeVal[secondOctetToChange] + stepArr[secondOctetToChange] <= 255)
                currentFcoeAddr.fcid[secondOctetToChange] += stepArr[secondOctetToChange];
            else
            {
                currentFcoeAddr.fcid[secondOctetToChange] = 0;
                if (fcoeVal[thirdOctetToChange] + stepArr[thirdOctetToChange] <= 255)
                    currentFcoeAddr.fcid[thirdOctetToChange] += stepArr[thirdOctetToChange];
                else
                {
                    rc = GT_NO_MORE;
                    break;
                }
            }
        }
    }

    /* print result */
    cpssOsPrintf("\nFcoePrefixDelMany results:");
    if (numOfPrefixesDeleted == 0)
    {
        cpssOsPrintf("\n    No prefixes were added at all.\n");
    }
    else
    {
        cpssOsPrintf("\n    Prefixes ");
        outputFcoeAddress(startFcoeAddr);
        cpssOsPrintf(" - ");
        outputFcoeAddress(lastFcoeAddrDeleted);
        cpssOsPrintf(" were added (%d/%d)\n",numOfPrefixesDeleted,numOfPrefixesToDel);
    }

    /* update output parameters */
    if (numOfPrefixesDeletedPtr != NULL)
        *numOfPrefixesDeletedPtr = numOfPrefixesDeleted;
    if (lastFcoeAddrDeletedPtr != NULL)
        cpssOsMemCpy(lastFcoeAddrDeletedPtr, &lastFcoeAddrDeleted, sizeof(GT_FCID));

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpLpmDbgFcoePrefixDelManyByOctet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, lpmDbId, vrId, startFcoeAddr,numOfPrefixesToDel,
                            firstOctetToChange, secondOctetToChange, thirdOctetToChange,
                            stepArr, prefixLength, lastFcoeAddrDeletedPtr, numOfPrefixesDeletedPtr));

    rc = internal_cpssDxChIpLpmDbgFcoePrefixDelManyByOctet(lpmDbId, vrId, startFcoeAddr, numOfPrefixesToDel,
                                                             firstOctetToChange, secondOctetToChange,
                                                             thirdOctetToChange, stepArr, prefixLength,
                                                             lastFcoeAddrDeletedPtr, numOfPrefixesDeletedPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, lpmDbId, vrId, startFcoeAddr,
                              numOfPrefixesToDel, firstOctetToChange, secondOctetToChange,
                              thirdOctetToChange, stepArr, prefixLength,
                              lastFcoeAddrDeletedPtr, numOfPrefixesDeletedPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpLpmDbgFcoePrefixGetManyByOctet function
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
static GT_STATUS internal_cpssDxChIpLpmDbgFcoePrefixGetManyByOctet
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
)
{
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  i = 0;
    GT_FCID                                 currentFcoeAddr;
    GT_U32                                  numOfRetrievedPrefixes = 0;
    GT_FCID                                 lastFcoeAddrAdded;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT  nextHopInfo;
    GT_BOOL                                 octetsArray[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_FCOE_PROTOCOL_CNS]={GT_FALSE,GT_FALSE,GT_FALSE};/* array to hold the octets already selected */
    GT_U32                                  fcoeVal[3] = { 0 };

    currentFcoeAddr                       = startFcoeAddr;
    lastFcoeAddrAdded                     = startFcoeAddr;

    cpssOsMemSet(&nextHopInfo, 0, sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));

    /* check step array values */
    for (i = 0; i < 3; i++)
    {
        if (stepArr[i] < 1 || stepArr[i] > 32)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    /* check prefix length */
    if (prefixLength > 24)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if((firstOctetToChange > 2) || (secondOctetToChange> 2 ) || (thirdOctetToChange > 2))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    /* make sure octet are not selected twice */
    if(octetsArray[firstOctetToChange]==GT_FALSE)
    {
        octetsArray[firstOctetToChange]=GT_TRUE;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if(octetsArray[secondOctetToChange]==GT_FALSE)
    {
        octetsArray[secondOctetToChange]=GT_TRUE;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if(octetsArray[thirdOctetToChange]==GT_FALSE)
    {
        octetsArray[thirdOctetToChange]=GT_TRUE;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    for (i = 0 ; i < numOfPrefixesToGet ; )
    {
        /* get the current prefix */
        rc =  cpssDxChIpLpmFcoePrefixSearch(lpmDbId,
                                                vrId,
                                                &currentFcoeAddr,
                                                prefixLength,
                                                &nextHopInfo);
        if (rc != GT_OK)
            break;

        numOfRetrievedPrefixes++;
        i++;

        lastFcoeAddrAdded = currentFcoeAddr;

        fcoeVal[2] = currentFcoeAddr.fcid[2];
        fcoeVal[1] = currentFcoeAddr.fcid[1];
        fcoeVal[0] = currentFcoeAddr.fcid[0];

               /* advance to the next IP address */
        if (fcoeVal[firstOctetToChange] + stepArr[firstOctetToChange] <= 255)
            currentFcoeAddr.fcid[firstOctetToChange] += stepArr[firstOctetToChange];
        else
        {
            currentFcoeAddr.fcid[firstOctetToChange] = 0;
            if (fcoeVal[secondOctetToChange] + stepArr[secondOctetToChange] <= 255)
                currentFcoeAddr.fcid[secondOctetToChange] += stepArr[secondOctetToChange];
            else
            {
                currentFcoeAddr.fcid[secondOctetToChange] = 0;
                if (fcoeVal[thirdOctetToChange] + stepArr[thirdOctetToChange] <= 255)
                    currentFcoeAddr.fcid[thirdOctetToChange] += stepArr[thirdOctetToChange];
                else
                {
                    rc = GT_NO_MORE;
                    break;
                }
            }
        }
    }

    /* print result */
    cpssOsPrintf("\nFcoePrefixGetMany results:");
    if (numOfRetrievedPrefixes == 0)
    {
        cpssOsPrintf("\n    No prefixes at all.\n");
    }
    else
    {
        cpssOsPrintf("\n    Prefixes ");
        outputFcoeAddress(startFcoeAddr);
        cpssOsPrintf(" - ");
        outputFcoeAddress(lastFcoeAddrAdded);
        cpssOsPrintf(" were retrieved (%d/%d)\n",numOfRetrievedPrefixes,numOfPrefixesToGet);
    }

    /* update output parameters */
    if (numOfRetrievedPrefixesPtr != NULL)
        *numOfRetrievedPrefixesPtr = numOfRetrievedPrefixes;

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpLpmDbgFcoePrefixGetManyByOctet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, lpmDbId, vrId, startFcoeAddr, numOfPrefixesToGet,
                            firstOctetToChange, secondOctetToChange, thirdOctetToChange,
                            stepArr, prefixLength, numOfRetrievedPrefixesPtr));

    rc = internal_cpssDxChIpLpmDbgFcoePrefixGetManyByOctet(lpmDbId, vrId, startFcoeAddr,
                                                              numOfPrefixesToGet,
                                                              firstOctetToChange,
                                                              secondOctetToChange,
                                                              thirdOctetToChange,
                                                              stepArr, prefixLength,
                                                              numOfRetrievedPrefixesPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, lpmDbId, vrId, startFcoeAddr, numOfPrefixesToGet,
                              firstOctetToChange, secondOctetToChange, thirdOctetToChange,
                              stepArr, prefixLength, numOfRetrievedPrefixesPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChIpLpmDbgDefragEnableSet function
* @endinternal
*
* @brief   This function set the defrag flag.
*         IP wrappers will use this flag as parameter to the CPSS APIs
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
static GT_STATUS internal_cpssDxChIpLpmDbgDefragEnableSet
(
    IN GT_BOOL   defragEnable
)
{
    defragmentationEnableFlag = defragEnable;

    return GT_OK;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpLpmDbgDefragEnableSet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, defragEnable));

    rc = internal_cpssDxChIpLpmDbgDefragEnableSet(defragEnable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, defragEnable));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIpLpmDbgDefragEnableGet function
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
static GT_STATUS internal_cpssDxChIpLpmDbgDefragEnableGet
(
    OUT GT_BOOL   *defragEnablePtr
)
{
    *defragEnablePtr = defragmentationEnableFlag;

    return GT_OK;
}

/**
* @internal cpssDxChIpLpmDbgDefragEnableGet function
* @endinternal
*
* @brief   This function return the defrag flag.
*         IP wrappers will use this flag as parameter to the CPSS APIs
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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpLpmDbgDefragEnableGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, defragEnablePtr));

    rc = internal_cpssDxChIpLpmDbgDefragEnableGet(defragEnablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, defragEnablePtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

