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
* @file prvUtfExtras.h
*
* @brief Internal header which defines API for helpers functions
* which are specific for cpss unit testing.
*
* @version   89
********************************************************************************
*/
#ifndef __prvUtfExtrash
#define __prvUtfExtrash

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <utf/private/prvUtfHelpers.h>
#include <utf/utfMain.h>
#include <extUtils/tgf/prvUtfExtras.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <appDemo/sysHwConfig/appDemoDb.h>

#ifdef IMPL_FA
#include <cpssFa/generic/gen/private/gtCoreFaStructs.h>
#endif /* IMPL_FA */

#ifdef IMPL_XBAR
#include <cpssXbar/generic/private/gtCoreXbarStructs.h>
#endif /* IMPL_XBAR */

#define PRV_TGF_INVALID_VALUE_CNS 0xFFFFFFFF
/******************************************************************************\
 *                              Packet Processor defines                      *
\******************************************************************************/
/* convert value to be in the range of minimal value and the max value .
    note : if value less than the minimum it is not changed !!!
    note : if max value less than the minimal value --> fit value into 0..max !!!
*/
#define ALIGN_RESOURCE_TO_DEV_MAC(value , minimalValue , maxValue)  \
    (   ((maxValue) <= (minimalValue)) ? ((value) % (maxValue)) : \
       ((value) <= (minimalValue) ? (value) :     \
        ((minimalValue) + (((value) - (minimalValue)) % ((maxValue) - (minimalValue))))))


/* convert eport to one in valid range of : 512 .. max */
#define ALIGN_EPORT_TO_512_TILL_MAX_DEV_MAC(eport) \
    ALIGN_RESOURCE_TO_DEV_MAC(eport , 512 , UTF_CPSS_PP_MAX_PORT_NUM_CNS(prvTgfDevNum))

/* convert evid to one in valid range of : 4K .. max */
#define ALIGN_EVID_TO_4K_TILL_MAX_DEV_MAC(evid) \
    ALIGN_RESOURCE_TO_DEV_MAC(evid , _4K , UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(prvTgfDevNum))

/* convert evidx to one in valid range of : 4K .. max */
#define ALIGN_EVIDX_TO_4K_TILL_MAX_DEV_MAC(evidx) \
    ALIGN_RESOURCE_TO_DEV_MAC(evidx , _4K , UTF_CPSS_PP_MAX_VIDX_INDEX_CNS(prvTgfDevNum))

/* max physical port */
#ifdef CHX_FAMILY
    #ifndef UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC
        #define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC
    #endif

    #include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

    /* internal MACRO -- do not call it from any C file !!! */
    /* max physical port */
    #define ____MAX_PHYSICAL_PORTS_NUM_CNS(_devNum,castingType)                 \
        (castingType )(((_devNum) < PRV_CPSS_MAX_PP_DEVICES_CNS) ?              \
                    (PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(_devNum)) : 64)

    /* internal MACRO -- do not call it from any C file !!! */
    /* max port number for 'eport' */
    #define ____MAX_PORT_NUM_CNS(_devNum,castingType)                           \
        (castingType)(((_devNum) < PRV_CPSS_MAX_PP_DEVICES_CNS) ?               \
                    (PRV_CPSS_DXCH_MAX_PORT_NUMBER_MAC(_devNum)) : 64)

    /* max port number for 'eport' when used as 'data' (not as 'key')*/
    #define UTF_CPSS_PP_MAX_PORT_AS_DATA_NUM_CNS(_devNum)                      \
        (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(_devNum) ?              \
         /*ePort field in BC3,Aldrin2*/                                     \
         (PRV_CPSS_SIP_5_20_CHECK_MAC(_devNum) ? _16K    :                     \
         (1+PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(_devNum))):            \
         UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(_devNum))

    /* max physical port */
    /* was UTF_CPSS_PP_MAX_PORTS_NUM_CNS */
    #define UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(_devNum)                  \
        ____MAX_PHYSICAL_PORTS_NUM_CNS(_devNum,GT_PHYSICAL_PORT_NUM)

    /* max port number for 'eport' */
    #define UTF_CPSS_PP_MAX_PORT_NUM_CNS(_devNum)                            \
        ____MAX_PORT_NUM_CNS(_devNum,GT_PORT_NUM)

    /* E ARCH supported by the device */
    #define UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(_devNum)            \
        (((_devNum) < PRV_CPSS_MAX_PP_DEVICES_CNS) ?                     \
           (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(_devNum)) :    \
           GT_FALSE)

    /* E ARCH enabled for the device */
    #define UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(_devNum)              \
        (((_devNum) < PRV_CPSS_MAX_PP_DEVICES_CNS) ?                     \
           (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(_devNum)) :      \
           GT_FALSE)

    /* Max HW devNum supported by the device */
    #define UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(_devNum)                      \
        (((_devNum) < PRV_CPSS_MAX_PP_DEVICES_CNS) ?                     \
            PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(_devNum) ?     \
                BIT_12: /* the device should support 10 bits */          \
                        /* the srcId should support 12 bits  */          \
            (PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_HW_DEV_NUM_MAC(_devNum) + 1) : \
            32)

    /* Max VLAN number */
    #define UTF_CPSS_PP_MAX_VLAN_NUM_CNS(_devNum)                       \
            (GT_U16) (((_devNum) < PRV_CPSS_MAX_PP_DEVICES_CNS) ?                \
             (PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_VID_MAC(_devNum) + 1)  : \
             _4K)

    /* Max VLAN index into table */
    #define UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(_devNum)                       \
            (GT_U16) (((_devNum) < PRV_CPSS_MAX_PP_DEVICES_CNS) ?                \
             (PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_TABLE_VLAN_MAC(_devNum) + 1)  : \
             _4K)

    /* Max EVIDX index into L2 LTT table */
    #define UTF_CPSS_PP_MAX_VIDX_INDEX_CNS(_devNum)                       \
            (GT_U16) (((_devNum) < PRV_CPSS_MAX_PP_DEVICES_CNS) ?                \
             (PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_L2_LTT_MAC(_devNum) + 1)  : \
             _4K)


    /* Max trunks number */
    #define UTF_CPSS_PP_MAX_TRUNK_ID_MAC(_devNum)                          \
        (GT_TRUNK_ID)(((_devNum) < PRV_CPSS_MAX_PP_DEVICES_CNS) ?          \
            (1 + PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_TRUNK_ID_MAC(_devNum)) : \
            128)

    /* Max Source IDs number  */
    #define UTF_CPSS_PP_MAX_SRC_ID_NUM_MAC(_devNum)                      \
        (((_devNum) < PRV_CPSS_MAX_PP_DEVICES_CNS) ?                     \
            (1 + PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_SRC_ID_MAC(_devNum)) : \
            32)

    #define UTF_CPSS_PP_MAX_SRC_ID_INDEX_NUM_MAC(_devNum)              \
        (((_devNum) < PRV_CPSS_MAX_PP_DEVICES_CNS) ?                     \
            (1 + PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_TABLE_SRC_ID_MAC(_devNum)) : \
            32)

    /* Max Source IDs E-tag type location bit*/
    #define UTF_CPSS_PP_MAX_SRC_ID_ETAG_TYPE_LOC_BIT_MAC(_devNum)   16

    #define UTF_CPSS_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(_devNum)          \
        (((_devNum) < PRV_CPSS_MAX_PP_DEVICES_CNS) ?                     \
            (PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(_devNum)) : \
            0)

    /* Max FID number */
    #define UTF_CPSS_PP_MAX_FID_NUM_CNS(_devNum)                       \
            (GT_U16) (((_devNum) < PRV_CPSS_MAX_PP_DEVICES_CNS) ?      \
             (PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_FID_MAC(_devNum) + 1)  : \
             _4K)

    /* Max STG number */
    #define UTF_CPSS_PP_MAX_STG_ID_NUM_CNS(_devNum)                       \
            (((_devNum) < PRV_CPSS_MAX_PP_DEVICES_CNS && PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(_devNum)) ?      \
             (PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_STG_ID_MAC(_devNum) + 1)  : \
             PRV_CPSS_DXCH_STG_MAX_NUM_CNS)


#else /*! CHX_FAMILY*/
    /* dummy macros for non DXCH */
    #undef ALIGN_EPORT_TO_512_TILL_MAX_DEV_MAC
    #define ALIGN_EPORT_TO_512_TILL_MAX_DEV_MAC(dummy) dummy /* not valid for non DXCH !!! */

    /* dummy macros for non DXCH */
    #undef ALIGN_EVID_TO_4K_TILL_MAX_DEV_MAC
    #define ALIGN_EVID_TO_4K_TILL_MAX_DEV_MAC(dummy) dummy   /* not valid for non DXCH !!! */

    /* dummy macros for non DXCH */
    #undef ALIGN_EVIDX_TO_4K_TILL_MAX_DEV_MAC
    #define ALIGN_EVIDX_TO_4K_TILL_MAX_DEV_MAC(dummy) dummy   /* not valid for non DXCH !!! */


    /* max physical port */
    #define UTF_CPSS_PP_MAX_PORTS_NUM_CNS                       64
    #define UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(_devNum)   UTF_CPSS_PP_MAX_PORTS_NUM_CNS
    #define UTF_CPSS_PP_MAX_PORT_NUM_CNS(_devNum)             UTF_CPSS_PP_MAX_PORTS_NUM_CNS

    /* Max HW devNum supported by the device */
    #define UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(_devNum)             128

    /* Max VLAN number */
    #define UTF_CPSS_PP_MAX_VLAN_NUM_CNS(_devNum)             _4K

    /* max vlan index into table */
    #define UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(_devNum)             _4K

    /* max vidx index into table */
    #define UTF_CPSS_PP_MAX_VIDX_INDEX_CNS(_devNum)             _4K

    /* Max trunks number */
    #define UTF_CPSS_PP_MAX_TRUNK_ID_MAC(_devNum)               256

    /* Max Source IDs number  */
    #define UTF_CPSS_PP_MAX_SRC_ID_NUM_MAC(_devNum)             32

    #define UTF_CPSS_PP_MAX_SRC_ID_INDEX_NUM_MAC                UTF_CPSS_PP_MAX_SRC_ID_NUM_MAC

    #define UTF_CPSS_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(_devNum) 0

    /* Max FID number */
    #define UTF_CPSS_PP_MAX_FID_NUM_CNS(_devNum)                _4K

    /* E ARCH supported by the device - the (_devNum != _devNum) is to prevent compilation warning */
    #define UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(_devNum)   ((_devNum != _devNum))

    #define UTF_CPSS_PP_MAX_STG_ID_NUM_CNS(_devNum)             256

#endif /*! CHX_FAMILY*/

#ifdef PX_FAMILY
    #define UTF_CPSS_PX_MAX_PORTS_NUM_CNS            17
    #define UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(_devNum)   UTF_CPSS_PX_MAX_PORTS_NUM_CNS
    #define UTF_CPSS_PX_MAX_PORT_NUM_CNS(_devNum)             UTF_CPSS_PX_MAX_PORTS_NUM_CNS
#endif

/* macro to set <currPortGroupsBmp> with the port group BMP that is relevant to globalPhyPortNum */
/* also set usePortGroupsBmp = GT_TRUE */
#define   TGF_SET_CURRPORT_GROUPS_BMP_WITH_PORT_GROUP_OF_PORT_MAC(globalPhyPortNum) \
    {                                                                    \
        GT_U32  portGroupId;                                             \
        if(!PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))                      \
        {                                                                \
            portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(prvTgfDevNum,globalPhyPortNum); \
        }                                                                \
        else                                                             \
        {                                                                \
            portGroupId = PRV_CPSS_SIP5_GLOBAL_PHYSICAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(prvTgfDevNum,globalPhyPortNum); \
        }                                                                \
                                                                         \
        usePortGroupsBmp = GT_TRUE;                                      \
        currPortGroupsBmp    = (1 << portGroupId);                       \
    }


/* */
/* like macro TGF_SET_CURRPORT_GROUPS_BMP_WITH_PORT_GROUP_OF_PORT_MAC(...) but for
   features that 'port group' is representative 'per tile'
   like FDB and TCAM
*/
#define   TGF_SET_CURRPORT_GROUPS_BMP_WITH_PORT_GROUP_OF_PORT_INSTANCE_PER_TILE_MAC(globalPhyPortNum) \
    {                                                                    \
        if(PRV_CPSS_PP_MAC(prvTgfDevNum)->multiPipe.numOfPipesPerTile)   \
        {                                                                \
            GT_U32  portGroupId,tileId;                                  \
            portGroupId = PRV_CPSS_SIP5_GLOBAL_PHYSICAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(prvTgfDevNum,globalPhyPortNum); \
            tileId = portGroupId / PRV_CPSS_PP_MAC(prvTgfDevNum)->multiPipe.numOfPipesPerTile; \
            /* set representative port group for the tile */             \
            portGroupId = tileId * PRV_CPSS_PP_MAC(prvTgfDevNum)->multiPipe.numOfPipesPerTile; \
                                                                         \
            usePortGroupsBmp = GT_TRUE;                                  \
            currPortGroupsBmp    = (1 << portGroupId);                   \
        }                                                                \
        else                                                             \
        {                                                                \
            TGF_SET_CURRPORT_GROUPS_BMP_WITH_PORT_GROUP_OF_PORT_MAC(globalPhyPortNum);  \
        }                                                                \
                                                                         \
    }

/* update the portGroupsBmp that supports 'per pipe' bmp to change to relative 'per tile' bmp */
/* the macro relevant to numOfTiles >= 2 (and ignored for others) */
#define TGF_MASK_CURR_PORT_GROUPS_BMP_INSTANCE_PER_TILE_MAC(portGroupsBmp) \
    if(PRV_CPSS_PP_MAC(prvTgfDevNum)->multiPipe.numOfTiles >= 2 &&         \
       PRV_CPSS_PP_MAC(prvTgfDevNum)->multiPipe.numOfPipesPerTile)         \
    {                                                                      \
        GT_U32  ii,tileBit;                                                \
        GT_U32  newBmp = 0;                                                \
        for(ii = 0 ; ii < PRV_CPSS_PP_MAC(prvTgfDevNum)->multiPipe.numOfPipes ; ii++)\
        {                                                                  \
            if(portGroupsBmp & (1<<ii))                                    \
            {                                                              \
                /* the pipe need representation by the tile */             \
                tileBit = (ii / PRV_CPSS_PP_MAC(prvTgfDevNum)->multiPipe.numOfPipesPerTile) * \
                                PRV_CPSS_PP_MAC(prvTgfDevNum)->multiPipe.numOfPipesPerTile;   \
                newBmp |= 1 << tileBit;                                    \
            }                                                              \
        }                                                                  \
        /* update the bmp */                                               \
        portGroupsBmp = newBmp;                                            \
    }




/* macro returns true if sdma interface used for cpu traffic by current device */
#define  UTF_CPSS_PP_IS_SDMA_USED_MAC(_devNum)    \
    (((_devNum) < PRV_CPSS_MAX_PP_DEVICES_CNS) && (PRV_CPSS_PP_MAC(_devNum)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_SDMA_E))

#define IS_XCAT3_DEV_MAC(_devNum)    \
    (PRV_CPSS_PP_MAC(_devNum)->devFamily    == CPSS_PP_FAMILY_DXCH_XCAT3_E)

#define IS_LION2_DEV_MAC(_devNum)    \
    (PRV_CPSS_PP_MAC(_devNum)->devFamily    == CPSS_PP_FAMILY_DXCH_LION2_E)

#define IS_BOBK_DEV_MAC(_devNum)    \
    (PRV_CPSS_PP_MAC(_devNum)->devFamily    == CPSS_PP_FAMILY_DXCH_BOBCAT2_E && \
     PRV_CPSS_PP_MAC(_devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E)

#define IS_BOBK_DEV_CETUS_MAC(_devNum)    \
    (IS_BOBK_DEV_MAC(_devNum) &&          \
     PRV_CPSS_DXCH_PP_MAC(_devNum)->hwInfo.multiDataPath.info[0].dataPathNumOfPorts == 0)

#define IS_AC3X_DEV_MAC(_devNum)    \
    (PRV_CPSS_PP_MAC(_devNum)->devFamily    == CPSS_PP_FAMILY_DXCH_AC3X_E)

GT_U32  utfIsMustUseOrigPorts(IN GT_U8  devNum);
/* check if must use only ports from prvTgfPortsArray[] */
#define MUST_USE_ORIG_PORTS_MAC(_devNum) \
    utfIsMustUseOrigPorts(_devNum)

/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !
*/
#ifdef UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC
    typedef     GT_PORT_NUM                  UTF_PORT_NUM;
    typedef     GT_PHYSICAL_PORT_NUM         UTF_PHYSICAL_PORT_NUM;
#else /*!UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC*/
    typedef     GT_U8                        UTF_PORT_NUM;
    typedef     GT_U8                        UTF_PHYSICAL_PORT_NUM;

    /* direct the call to 'old functions support' - GT_U8 */
    #define prvUtfNextPhyPortReset          prvUtfNextPhyPortReset_old
    #define prvUtfNextPhyPortGet            prvUtfNextPhyPortGet_old
    #define prvUtfPortTypeGet               prvUtfPortTypeGet_old
    #define prvUtfPortInterfaceModeGet      prvUtfPortInterfaceModeGet_old
    #define prvUtfPortSpeedGet              prvUtfPortSpeedGet_old
    #define prvUtfPortTypeOptionsGet        prvUtfPortTypeOptionsGet_old
    #define prvUtfPortIsFlexLinkGet         prvUtfPortIsFlexLinkGet_old

    #define prvUtfNextVirtPortReset         prvUtfNextVirtPortReset_old
    #define prvUtfNextVirtPortGet           prvUtfNextVirtPortGet_old

    #define prvUtfNextMacPortReset          prvUtfNextMacPortReset_old
    #define prvUtfNextMacPortGet            prvUtfNextMacPortGet_old

    #define prvUtfNextTqxPortReset          prvUtfNextTxqPortReset_old
    #define prvUtfNextTqxPortGet            prvUtfNextTxqPortGet_old

    #define prvUtfNextRxDmaPortReset        prvUtfNextRxDmaPortReset_old
    #define prvUtfNextRxDmaPortGet          prvUtfNextRxDmaPortGet_old

    #define prvUtfNextTxDmaPortReset        prvUtfNextTxDmaPortReset_old
    #define prvUtfNextTxDmaPortGet          prvUtfNextTxDmaPortGet_old

    /* functions to support port as GT_U8 .
       NOTE: code must not call them directly !
             code must call ONLY the functions WITHOUT the '_old' in the name !!!
    */
    GT_STATUS prvUtfNextPhyPortReset_old
    (
        INOUT GT_U8     *portPtr,
        IN GT_U8        dev
    );
    GT_STATUS prvUtfNextPhyPortGet_old
    (
        INOUT GT_U8     *portPtr,
        IN GT_BOOL      activeIs
    );
    GT_STATUS prvUtfPortTypeGet_old
    (
        IN GT_U8                    dev,
        IN GT_U8                    port,
        OUT PRV_CPSS_PORT_TYPE_ENT  *portTypePtr
    );
    GT_STATUS prvUtfPortTypeOptionsGet_old
    (
        IN GT_U8                            dev,
        IN GT_U8                            port,
        OUT PRV_CPSS_PORT_TYPE_OPTIONS_ENT  *portTypeOptionsPtr
    );
    GT_STATUS prvUtfPortIsFlexLinkGet_old
    (
        IN GT_U8                            dev,
        IN GT_U8                            port,
        OUT GT_BOOL                         *isFlexLinkPtr
    );
    GT_STATUS prvUtfNextVirtPortReset_old
    (
        INOUT GT_U8             *portPtr,
        IN    GT_HW_DEV_NUM    hwDev
    );
    GT_STATUS prvUtfNextVirtPortGet_old
    (
        INOUT GT_U8             *portPtr,
        IN GT_BOOL              activeIs
    );
    GT_STATUS prvUtfNextMacPortReset_old
    (
        INOUT GT_U8             *portPtr,
        IN    GT_U8             dev
    );
    GT_STATUS prvUtfNextMacPortGet_old
    (
        INOUT GT_U8             *portPtr,
        IN GT_BOOL              activeIs
    );
    GT_STATUS prvUtfNextTxqPortReset_old
    (
        INOUT GT_U8             *portPtr,
        IN    GT_U8              dev
    );
    GT_STATUS prvUtfNextTxqPortGet_old
    (
        INOUT GT_U8             *portPtr,
        IN GT_BOOL              activeIs
    );
    GT_STATUS prvUtfNextRxDmaPortReset_old
    (
        INOUT GT_U8             *portPtr,
        IN    GT_U8             dev
    );
    GT_STATUS prvUtfNextRxDmaPortGet_old
    (
        INOUT GT_U8             *portPtr,
        IN GT_BOOL              activeIs
    );
    GT_STATUS prvUtfNextTxDmaPortReset_old
    (
        INOUT GT_U8             *portPtr,
        IN    GT_U8             dev
    );
    GT_STATUS prvUtfNextTxDmaPortGet_old
    (
        INOUT GT_U8             *portPtr,
        IN GT_BOOL              activeIs
    );

#endif /*!UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC*/

/* set type of iterator for next functions:
prvUtfNextGenericPortItaratorTypeSet
prvUtfNextGenericPortReset
prvUtfNextGenericPortGet

replace the need to use:
prvUtfNextPhyPortReset
prvUtfNextPhyPortGet
prvUtfNextDefaultEPortReset
prvUtfNextDefaultEPortGet
prvUtfNextVirtPortReset
prvUtfNextVirtPortGet

============
    UTF_GENERIC_PORT_ITERATOR_TYPE_PHYSICAL_E - physical port iterator -
                        internal use:
                                prvUtfNextPhyPortReset
                                prvUtfNextPhyPortGet
    UTF_GENERIC_PORT_ITERATOR_TYPE_VIRTUAL_E  - puma  - virtual port iterator
                                                SIP5 - eport iterator
                                                other - valid port range
                        internal use:
                                prvUtfNextVirtPortReset
                                prvUtfNextVirtPortGet
    UTF_GENERIC_PORT_ITERATOR_TYPE_DEFAULT_EPORT_E - SIP5 - default eport iterator
                                                other - like virtual iterator
                        internal use:
                                prvUtfNextDefaultEPortReset
                                prvUtfNextDefaultEPortGet

    UTF_GENERIC_PORT_ITERATOR_TYPE_MAC_E - SIP5 - MAC port iterator
                                           other - like physical

    UTF_GENERIC_PORT_ITERATOR_TYPE_TXQ_E - SIP5 - TXQ port iterator
                                           other - like physical

    UTF_GENERIC_PORT_ITERATOR_TYPE_RXDMA_E - Bobcat2; Caelum; Bobcat3; Aldrin RXDMA port iterator,
                                           for others - like physical

    UTF_GENERIC_PORT_ITERATOR_TYPE_TXDMA_E - Bobcat2; Caelum; Bobcat3; Aldrin TXDMA port iterator,
                                           for others - like physical

 */
typedef enum{
    UTF_GENERIC_PORT_ITERATOR_TYPE_PHYSICAL_E,
    UTF_GENERIC_PORT_ITERATOR_TYPE_VIRTUAL_E,
    UTF_GENERIC_PORT_ITERATOR_TYPE_DEFAULT_EPORT_E,
    UTF_GENERIC_PORT_ITERATOR_TYPE_MAC_E,
    UTF_GENERIC_PORT_ITERATOR_TYPE_TXQ_E,
    UTF_GENERIC_PORT_ITERATOR_TYPE_RXDMA_E,
    UTF_GENERIC_PORT_ITERATOR_TYPE_TXDMA_E,

    UTF_GENERIC_PORT_ITERATOR_TYPE___LAST___E
}UTF_GENERIC_PORT_ITERATOR_TYPE_E;

/**
* @internal prvUtfNextGenericPortItaratorTypeSet function
* @endinternal
*
* @brief   set the generic port iterator type
*
* @param[in,out] type                     -   iterator
*
* @retval GT_OK                    -  iterator type OK
* @retval GT_FAIL                  -  iterator type NOT OK
*/
GT_STATUS prvUtfNextGenericPortItaratorTypeSet
(
    INOUT UTF_GENERIC_PORT_ITERATOR_TYPE_E     type
);


/**
* @internal prvUtfNextGenericPortReset function
* @endinternal
*
* @brief   Reset generic ports iterator must be called before go over all ports
*         NOTE: caller should set the 'iterator type' before calling this see prvUtfNextGenericPortItaratorTypeSet
* @param[in,out] portPtr                  -   iterator thru ports
* @param[in] dev                      -   device id of ports
*
* @retval GT_OK                    -  iterator initialization OK
* @retval GT_FAIL                  -  general failure error
*/
GT_STATUS prvUtfNextGenericPortReset
(
    INOUT UTF_PORT_NUM     *portPtr,
    IN GT_U8        dev
);

/**
* @internal prvUtfNextGenericPortGet function
* @endinternal
*
* @brief   This routine returns next active/non active generic port id.
*         Call the function until non GT_OK error. Note that
*         iterator must be initialized before.
*         NOTE: caller should set the 'iterator type' before calling this see prvUtfNextGenericPortItaratorTypeSet
* @param[in,out] portPtr                  -   port id
* @param[in] activeIs                 -   set to GT_TRUE for getting active ports
*/
GT_STATUS prvUtfNextGenericPortGet
(
    INOUT UTF_PORT_NUM       *portPtr,
    IN GT_BOOL              activeIs
);



/* pp families - corresponding bits     */
/* used by iterator through devices     */
#define UTF_CPSS_PP_FAMILY_TWISTC_CNS   UTF_TWISTC_E
#define UTF_CPSS_PP_FAMILY_TWISTD_CNS   UTF_TWISTD_E
#define UTF_CPSS_PP_FAMILY_SAMBA_CNS    UTF_SAMBA_E
#define UTF_CPSS_PP_FAMILY_SALSA_CNS    UTF_SALSA_E
#define UTF_CPSS_PP_FAMILY_TIGER_CNS    UTF_TIGER_E
#define UTF_CPSS_PP_FAMILY_PUMA_CNS     UTF_PUMA_E
/* all the eArch devices */
#define UTF_CPSS_PP_E_ARCH_CNS          (UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_CPSS_PP_ALL_SIP6_CNS)
/* all the sip6 devices */
#define UTF_CPSS_PP_ALL_SIP6_CNS          (UTF_FALCON_E)

/* widely-used family sets                              */
/* used by UTs while call prvUtfDeviceReset function    */

#define UTF_ALL_FAMILIES_SET_CNS    (0xFFFFFFFF)

#define UTF_EXMX_FAMILY_SET_CNS     (UTF_CPSS_PP_FAMILY_TWISTC_CNS | \
                                    UTF_CPSS_PP_FAMILY_TWISTD_CNS |  \
                                    UTF_CPSS_PP_FAMILY_SAMBA_CNS |   \
                                    UTF_CPSS_PP_FAMILY_TIGER_CNS)

#define UTF_EXMXTG_FAMILY_SET_CNS   UTF_CPSS_PP_FAMILY_TIGER_CNS

#define UTF_DXSAL_FAMILY_SET_CNS    UTF_CPSS_PP_FAMILY_SALSA_CNS

#define UTF_EXMXPM_FAMILY_SET_CNS   UTF_CPSS_PP_FAMILY_PUMA_CNS

#define UTF_CPSS_PP_EXMXPM_CHECK_MAC(_devNum) \
     ((PRV_CPSS_PP_MAC(_devNum)->devFamily > CPSS_PP_FAMILY_START_EXMXPM_E)&& \
      (PRV_CPSS_PP_MAC(_devNum)->devFamily < CPSS_PP_FAMILY_END_EXMXPM_E))

/* device is sip 5_0 and not sip 5_10 */
#define UTF_CPSS_SIP5_0_ONLY_CHECK_MAC(_devNum) \
     (PRV_CPSS_SIP_5_CHECK_MAC(_devNum) && !PRV_CPSS_SIP_5_10_CHECK_MAC(_devNum))

/* check if device is AC3X */
#define UTF_CPSS_DXCH_XCAT3X_CHECK_MAC(_devNum)                                               \
    (                                                                                         \
        (PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E)                 \
          &&                                                            \
        (prvTgfXcat3xExists() == GT_TRUE)                               \
    )


/*
 * Description: macro to check and reset device iterator
 *
 * Parameters:
 *      _devNum          - device number iterator
 *      _notAppFamilyBmp - bitmap of not applicable device's families
 *          NOTE: casting (GT_U32) to support (~value) in 64 bits CPU
 *                to avoid : warning (dcc:1244): constant out of range (ARG)
 *
 * Comments: this macro uses default error validation
 *
 */
#define PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(_devNum, _notAppFamilyBmp)            \
    do                                                                         \
    {                                                                          \
        GT_STATUS   _rc = GT_OK;                                               \
                                                                               \
        _rc = prvUtfNextNotApplicableDeviceReset(_devNum, (GT_U32)(_notAppFamilyBmp)); \
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, _rc);                               \
    } while(0)

/* positive macro to PRV_TGF_NOT_APPLIC_DEV_RESET_MAC :

 * Description: macro to check and reset device iterator for explicit families

*/
#define PRV_TGF_APPLIC_DEV_RESET_MAC(_devNum, _appFamilyBmp)              \
    {                                                                     \
        GT_U32  notAppFamilyBmp = UTF_ALL_FAMILY_E - _appFamilyBmp;       \
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(_devNum,notAppFamilyBmp);        \
    }


/*
 * Description: macro to check and reset device iterator
 *
 * Parameters:
 *      _devNum          - device number iterator
 *      _rc              - return code that needs to be used
 *      _notAppFamilyBmp - bitmap of not applicable device's families
 *          NOTE: casting (GT_U32) to support (~value) in 64 bits CPU
 *                to avoid : warning (dcc:1244): constant out of range (ARG)
 *
 * Comments: this macro uses speciric return code and error validation
 *
 */
#define PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(_devNum, _rc, _notAppFamilyBmp)    \
    {                                                                          \
        _rc = prvUtfNextNotApplicableDeviceReset(_devNum, (GT_U32)(_notAppFamilyBmp)); \
        if (GT_OK != _rc)                                                      \
        {                                                                      \
            PRV_UTF_LOG1_MAC("prvUtfNextNotApplicableDeviceReset FAILED, rc = [%d]", _rc);  \
                                                                               \
            return _rc;                                                        \
        }                                                                      \
    }


/*
 * Description: macro to declare that the test should support this family
 *
 * Parameters:
 *      _notAppFamilyBmp - bitmap of applicable device's families
 *
 * Comments: this macro is EMPTY , and used only for references !!
 *           added to allow tracking the PUMA device coverage by the tests.
 *
 */
#define PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(_notAppFamilyBmp)

/*
 * Description: macro to check if the device bmp is not supported
 *
 * Parameters:
 *      _devNum          - device number
 *      _notAppFamilyBmp - bitmap of not applicable device's families
 *          NOTE: casting (GT_U32) to support (~value) in 64 bits CPU
 *                to avoid : warning (dcc:1244): constant out of range (ARG)
 *
 * Comments: this macro uses specific return code and error validation
 *
 */
#define PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(_devNum, _notAppFamilyBmp)         \
    if(GT_FALSE == prvUtfDeviceTestNotSupport(_devNum, (GT_U32)(_notAppFamilyBmp)))    \
        SKIP_TEST_MAC

/*
 * Description: macro to skip long running tests
 *
 * Parameters:
 *      _skipFamilyBmp - bitmap of families that should be skipped
 *          NOTE: casting (GT_U32) to support (~value) in 64 bits CPU
 *                to avoid : warning (dcc:1244): constant out of range (ARG)
 *
 * Comments: none
 *
 */
#define PRV_TGF_SKIP_LONG_TEST_MAC(_skipFamilyBmp)             \
    if (GT_TRUE == prvUtfSkipLongTestsFlagGet((GT_U32)_skipFamilyBmp)) \
        SKIP_TEST_MAC

/*
 * Description: macro to skip test if CPSS API Logging is enabled
 *
 * Parameters: None
 *
 * Comments: none
 *
 */
#define PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC             \
    if (GT_TRUE == prvUtfReduceLogSizeFlagGet())     \
        SKIP_TEST_MAC

/*
 * Description: macro to skip tests during non baseline execution
 *
 * Parameters:
 *
 * Comments: none
 *
 */
#define PRV_TGF_SKIP_NON_BASELINE_TEST_MAC()             \
    if (GT_TRUE == prvUtfSkipNonBaselineTestsFlagGet()) \
        SKIP_TEST_MAC

extern GT_BOOL prvUtfIsGmCompilation(void);
extern GT_U32  cpssDeviceRunCheck_onEmulator(void);


/*
 * Description: macro to skip PASS-OK tests in Golden Model only,
 *              because the GM hold memory leakage ,and tests with many packets
 *              cause more leakage .. till the run 'killed' on:
 *              terminate called after throwing an instance of 'std::bad_alloc'
 *
 * Parameters:
 *      _skipFamilyBmp - bitmap of families that should be skipped
 *          NOTE: casting (GT_U32) to support (~value) in 64 bits CPU
 *                to avoid : warning (dcc:1244): constant out of range (ARG)
 *
 * Comments: none
 *
 */
#define PRV_TGF_SKIP_PASS_OK_LONG_TEST_GM_MAC(_skipFamilyBmp)         \
    if (GT_TRUE == prvUtfSkipLongTestsFlagGet((GT_U32)_skipFamilyBmp) \
        && GT_TRUE == prvUtfIsGmCompilation())                \
    {                                                         \
        cpssOsPrintf("\n Test expected to PASS OK , but skipped to allow full run (%s)\n", #_skipFamilyBmp);\
        SKIP_TEST_MAC                                         \
    }

/*
 * Description: macro to skip PASS-OK tests in Golden Model only,
 *              When running the tests with full LPM memory in GM (LPM_MEM_SIZE = 0xF0000)
 *              till the run 'killed' on:
 *              terminate called after throwing an instance of 'std::bad_alloc'
 *              So, in Falcon GM LPM size was reduced to (LPM_MEM_SIZE = 0x80000).
 *              This will cause some of the tests which involvs IPv6 to fail. These tests PASS
 *              ith full LPM memory

 * Parameters:
 *      _skipFamilyBmp - bitmap of families that should be skipped
 *          NOTE: casting (GT_U32) to support (~value) in 64 bits CPU
 *                to avoid : warning (dcc:1244): constant out of range (ARG)
 *
 * Comments: none
 *
 */
#define PRV_TGF_SKIP_PASS_OK_ON_FULL_LPM_MEM_IN_GM_MAC(_skipFamilyBmp)         \
    if (GT_TRUE == prvUtfSkipLongTestsFlagGet((GT_U32)_skipFamilyBmp) \
        && GT_TRUE == prvUtfIsGmCompilation())                \
    {                                                          \
        cpssOsPrintf("\n Test expected to PASS OK , but skipped to allow full run (%s)\n", #_skipFamilyBmp);\
        SKIP_TEST_MAC                                         \
    }

/*
 * Description: macro to skip long running tests in Golden Model only.
 *
 * Parameters:
 *      _skipFamilyBmp - bitmap of families that should be skipped
 *          NOTE: casting (GT_U32) to support (~value) in 64 bits CPU
 *                to avoid : warning (dcc:1244): constant out of range (ARG)
 *
 * Comments: none
 *
 */
#define PRV_TGF_SKIP_LONG_TEST_GM_MAC(_skipFamilyBmp)         \
    if (GT_TRUE == prvUtfSkipLongTestsFlagGet((GT_U32)_skipFamilyBmp) \
        && GT_TRUE == prvUtfIsGmCompilation())                \
        SKIP_TEST_MAC


/*
 * Description: macro to skip long running tests in EMULATOR only.
 *
 * Parameters:
 *      _skipFamilyBmp - bitmap of families that should be skipped
 *          NOTE: casting (GT_U32) to support (~value) in 64 bits CPU
 *                to avoid : warning (dcc:1244): constant out of range (ARG)
 *
 * Comments: none
 *
 */
#define PRV_TGF_SKIP_LONG_TEST_EMULATOR_MAC(_skipFamilyBmp)           \
    if (GT_TRUE == prvUtfSkipLongTestsFlagGet((GT_U32)_skipFamilyBmp) \
        && GT_TRUE == cpssDeviceRunCheck_onEmulator())                \
        SKIP_TEST_MAC



/* set skip flag and update skip tests num */
#define SKIP_TEST_MAC   \
        {                                                   \
            prvUtfSkipTestsSet();                           \
            return ;                                        \
        }

/* skip test when TM used */
#define PRV_TGF_SKIP_TEST_WHEN_TM_USED_MAC          \
    if (GT_TRUE == prvUtfIsTrafficManagerUsed())    \
        SKIP_TEST_MAC


#define GM_NOT_SUPPORT_THIS_TEST_MAC                    \
    if(GT_TRUE == prvUtfIsGmCompilation())              \
        SKIP_TEST_MAC;

#define GM_FAIL_THIS_TEST_MAC                               \
    if(GT_TRUE == prvUtfIsGmCompilation())              \
    {                                                           \
        UTF_VERIFY_EQUAL0_STRING_MAC(0, 1, "Skip GM crash");    \
        return;                                                 \
    }

/* GM device : do not run the test for specified families
               state it as FAILED !!! (not as skipped ... because need debug)

               1. letting it run is just slowing the run
               2. this test may also fail other tests ...
*/
#define GM_FAMILY_FAIL_THIS_TEST_MAC(dev,_notAppFamilyBmp,_log_string)     \
    if(GT_TRUE == prvUtfIsGmCompilation())                 \
    {                                                      \
        if(GT_FALSE == prvUtfDeviceTestNotSupport(dev, (GT_U32)(_notAppFamilyBmp)))    \
        {                                                  \
            UTF_VERIFY_EQUAL0_STRING_MAC(1,0,_log_string); \
            return;                                        \
        }                                                  \
    }


/*
 * Description: macro to skip failed tests. Jira must be opened for such tests
 *
 * Parameters:
 *      _skipFamilyBmp - bitmap of families that should be skipped
 *          NOTE: casting (GT_U32) to support (~value) in 64 bits CPU
 *                to avoid : warning (dcc:1244): constant out of range (ARG)
 *      _jira_name   - name of Jira for the test
 * Comments: none
 *
 */
#define PRV_TGF_SKIP_FAILED_TEST_MAC(_skipFamilyBmp, _jira_name)         \
    if (GT_TRUE == prvUtfCheckFamilyBmp((GT_U32)_skipFamilyBmp))         \
    {                                                                    \
        PRV_UTF_LOG2_MAC("\nTest fails but skipped for devices %s, see Jira %s!!!\n\n", #_skipFamilyBmp ,#_jira_name);\
        SKIP_TEST_MAC;}


#ifdef ASIC_SIMULATION
#define PRV_TGF_SKIP_SIMULATION_FAILED_TEST_MAC(_skipFamilyBmp, _jira_name)         \
                PRV_TGF_SKIP_FAILED_TEST_MAC(_skipFamilyBmp, _jira_name)
#else
    /* empty macro when the environment not running in Simulation */
    #define PRV_TGF_SKIP_SIMULATION_FAILED_TEST_MAC(_skipFamilyBmp, _jira_name)
#endif /*ASIC_SIMULATION*/


#ifdef ASIC_SIMULATION
    /*macro for ASIC_SIMULATION to skip tests that are not valid for it (like time driven : tail drop / shaper) */
    #define ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC                    \
        SKIP_TEST_MAC
#else
    /* empty macro when the environment not running GM device */
    #define ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC
#endif /*ASIC_SIMULATION*/

/* skip tests that will take long time due to poor performance of the GM device */
/* note : those are for tests that the CPSS due 'loops' and 'iterations' internally.*/
/* for tests that the UT does 'loops' and 'iterations' , the UT should be changed (and not skipped)
   to reduce for the GM the number of 'loops' and 'iterations' */
#define GM_SKIP_TEST_DUE_TO_POOR_PERFORMANCE_MAC    \
    if (GT_TRUE == prvUtfIsGmCompilation())         \
        SKIP_TEST_MAC

/* GM and EMULATOR need to reduce iterations due to poor performance */
#define IS_NEEDED_TO_REDUCE_ITERATIONS_DUE_TO_POOR_PERFORMANCE_MAC    \
    ((cpssDeviceRunCheck_onEmulator() || prvUtfIsGmCompilation()) ? 1 : 0)


/* MACRO skips execution of test for SIP5 GM. Test result is FAIL.
  The string is stored in the UTF Log. */
#define UTF_SIP5_GM_NOT_READY_FAIL_MAC(_log_string) \
    if ((GT_TRUE == prvUtfIsGmCompilation()) && \
        (PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum)))\
    {                                                  \
        UTF_VERIFY_EQUAL0_STRING_MAC(1,0,_log_string); \
        return;\
    }

/* MACRO skips execution of test for SIP6 GM. Test result is FAIL.
  The string is stored in the UTF Log. */
#define UTF_SIP6_GM_NOT_READY_FAIL_MAC(_log_string) \
    if ((GT_TRUE == prvUtfIsGmCompilation()) && \
        (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum)))\
    {                                                  \
        UTF_VERIFY_EQUAL0_STRING_MAC(1,0,_log_string); \
        return;\
    }


/* MACRO skips execution of test for SIP5 GM. Test result is FAIL.
  The string is stored in the UTF Log. */
#define UTF_SIP5_NOT_SIP6_GM_NOT_READY_FAIL_MAC(_log_string) \
    if ((GT_TRUE == prvUtfIsGmCompilation()) && \
        (!PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum) && (PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))))\
    {                                                  \
        UTF_VERIFY_EQUAL0_STRING_MAC(1,0,_log_string); \
        return;\
    }


/* MACRO skips execution of test for SIP6 GM. Test result is OK.
  The string is stored in the UTF Log. */
#define UTF_SIP6_GM_NOT_READY_SKIP_MAC(_log_string) \
    if ((GT_TRUE == prvUtfIsGmCompilation()) &&     \
        (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum)))   \
    {                                               \
        prvUtfLogMessage(_log_string, NULL, 0);     \
        prvUtfSkipTestsSet();                       \
        return;                                     \
    }


/* MACRO skips execution of test for Bobcat3. Test result is FAIL.
  The string is stored in the UTF Log. */
#define UTF_BOBCAT3_NOT_READY_FAIL_MAC(_log_string) \
    if ((PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily  == CPSS_PP_FAMILY_DXCH_BOBCAT3_E))\
    {                                                  \
        UTF_VERIFY_EQUAL0_STRING_MAC(1,0,_log_string); \
        return;\
    }

/* Macro to get max number of virtual ports */
#define UTF_GET_MAX_VIRT_PORTS_NUM_MAC(_devNum) \
    (PRV_CPSS_PP_MAC(_devNum)->numOfVirtPorts)

/* Macro to get max number of physical ports */
#define UTF_GET_MAX_PHY_PORTS_NUM_MAC(_devNum) \
    (PRV_CPSS_PP_MAC(_devNum)->numOfPorts)

#define IS_PORT_XG_E(portType)    \
        (portType>=PRV_CPSS_PORT_XG_E)

#define IS_PORT_GE_E(portType)    \
        (portType==PRV_CPSS_PORT_GE_E)

#define IS_PORT_FE_E(portType)    \
        (portType==PRV_CPSS_PORT_FE_E)

#ifdef CHX_FAMILY
    /* Macro to get max number of default ePorts. for device with not default eports
       the macro returns that physical ports number */
    #define UTF_GET_MAX_DEFAULT_EPORTS_NUM_MAC(_devNum) \
        (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(_devNum) ?      \
         PRV_CPSS_DXCH_MAX_DEFAULT_EPORT_NUMBER_MAC(_devNum) :         \
         UTF_GET_MAX_PHY_PORTS_NUM_MAC(_devNum))
#else /*!CHX_FAMILY*/
    /* Macro to get max number of default ePorts --> return number of virtual ports */
    #define UTF_GET_MAX_DEFAULT_EPORTS_NUM_MAC(_devNum) \
        UTF_GET_MAX_VIRT_PORTS_NUM_MAC(_devNum)
#endif  /*!CHX_FAMILY*/

/* init values for CPSS_INTERFACE_INFO_STC
   use maximal values to catch bugs */
#define UTF_INTERFACE_INIT_VAL_CNS    {0x7FFFFFFF, {0xFF,0xFF}, 0xFFFF, 0xFFFF, 0xFFFF, 0xFF, 0xFFFF, 0xFFFFFFFF}

/* Invalid enum array size */
extern GT_U32 utfInvalidEnumArrSize;

/* Array with invalid enum values to check.
   Using GT_U32 type instead of GT_32 can be helpful to find some bugs */
extern GT_U32 utfInvalidEnumArr[];


/* Macro to mask ports bmp with existing physical ports */
#define UTF_MASK_PORTS_BMP_WITH_EXISTING_MAC(_devNum,currPortsBmp)             \
    CPSS_PORTS_BMP_BITWISE_AND_MAC(&currPortsBmp,&currPortsBmp,&PRV_CPSS_PP_MAC(_devNum)->existingPorts)

/* get the bmp of families that NOT support eArch */
#define UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(_notAppFamilyBmpPtr)                  \
    *(_notAppFamilyBmpPtr) = (UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | \
                              UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E)

/* Add to 'bmp' of families the sip6 devices */
#define UTF_SIP6_ADD_TO_FAMILY_BMP_MAC(familyBmp)                  \
    (familyBmp) |= (UTF_CPSS_PP_ALL_SIP6_CNS)


/* get the bmp of families that support eArch */
#ifdef CHX_FAMILY
    #define UTF_E_ARCH_SUPPORTED_FAMILY_GET_MAC(_appFamilyBmpPtr)                  \
        *(_appFamilyBmpPtr) = UTF_CPSS_PP_E_ARCH_CNS
#else /* !CHX_FAMILY */
    #define UTF_E_ARCH_SUPPORTED_FAMILY_GET_MAC(_appFamilyBmpPtr)                  \
        *(_appFamilyBmpPtr) = UTF_NONE_FAMILY_E
#endif /* CHX_FAMILY */

/* next port step for eArch */
#define UTF_E_ARCH_NEXT_PORT_STEP_CNS   101

/* bypass tests crashed on HW                                 */
/* _appFamilyBmp is bitmap of device types for bypassing test */
#ifndef ASIC_SIMULATION
#define UTF_HW_CRASHED_TEST_BYPASS_MAC(_appFamilyBmp)                               \
    {                                                                               \
        GT_U8 __dev = 0;                                                            \
        GT_U32 __noBypass = 0;                                                      \
        appDemoDbEntryGet("dontBypassCrashedTests", &__noBypass);                   \
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&__dev, (~ (_appFamilyBmp)));              \
        if ((__noBypass == 0) && (GT_OK == prvUtfNextDeviceGet(&__dev, GT_TRUE)))   \
        {                                                                           \
            /* found defice of cpecified type */                                    \
            utfEqualVerify(                                                         \
                GT_FAIL, GT_NOT_APPLICABLE_DEVICE, __LINE__, __FILE__);             \
            utfFailureMsgLog(                                                       \
                 "Crashing test bypassed.", (const GT_UINTPTR *)NULL, 0);           \
            return;                                                                 \
        }                                                                           \
    }
#else /* ASIC_SIMULATION - empty extension */
#define UTF_HW_CRASHED_TEST_BYPASS_MAC(_appFamilyBmpPtr)
#endif /*ASIC_SIMULATION*/

/**
* @enum PRV_TGF_CFG_TABLE_ENT
 *
 * @brief Enumeration for the type of tables that the devices hold..
*/
typedef enum{

    /** table type represent the VLAN table. */
    PRV_TGF_CFG_TABLE_VLAN_E,

    /** table type represent the VIDX (multicast groups) table. */
    PRV_TGF_CFG_TABLE_VIDX_E,

    /** table type represent the FDB table. */
    PRV_TGF_CFG_TABLE_FDB_E,

    /** table type represent the router next hop table. */
    PRV_TGF_CFG_TABLE_ROUTER_NEXT_HOP_E,

    /** table type represent the Mac to Me table. */
    PRV_TGF_CFG_TABLE_MAC_TO_ME_E,

    /** table type represent the ARP/Tunnel Start table (ARP entries). */
    PRV_TGF_CFG_TABLE_ARP_E,

    /** table type represent the ARP/Tunnel Start table (tunnel start entries). */
    PRV_TGF_CFG_TABLE_TUNNEL_START_E,

    /** table type represent the STG (spanning tree groups) table. */
    PRV_TGF_CFG_TABLE_STG_E,

    /** table type represent the action table. */
    PRV_TGF_CFG_TABLE_ACTION_E,

    /** table type represent the TCAM table. */
    PRV_TGF_CFG_TABLE_TCAM_E,

    /** table type represent the policer metering table. */
    PRV_TGF_CFG_TABLE_POLICER_METERS_E,

    /** table type represent the policer counters table. */
    PRV_TGF_CFG_TABLE_POLICER_COUNTERS_E,

    /** table type represent the MLL Pair\Downstream Interface Table table. */
    PRV_TGF_CFG_TABLE_MLL_DIT_E,

    /** table type represent the router lookup translation table (LTT). */
    PRV_TGF_CFG_TABLE_ROUTER_LTT_E,

    /** table type represent the Router Tcam table. */
    PRV_TGF_CFG_TABLE_ROUTER_TCAM_E,

    /** table type represent the TTI table. */
    PRV_TGF_CFG_TABLE_TTI_TCAM_E,

    /** table type represent the QOS profile table. */
    PRV_TGF_CFG_TABLE_QOS_PROFILE_E,

    /** table type represent the centralized counters (CNC) table. */
    PRV_TGF_CFG_TABLE_CNC_E,

    /** table type represent CNC block (the number of conters per CNC block). */
    PRV_TGF_CFG_TABLE_CNC_BLOCK_E,

    /** table type represent trunk table. */
    PRV_TGF_CFG_TABLE_TRUNK_E,

    /** table type represent the FDB lookup table. */
    PRV_TGF_CFG_TABLE_FDB_LUT_E,

    /** table type represent the FDB TCAM table. */
    PRV_TGF_CFG_TABLE_FDB_TCAM_E,

    PRV_TGF_CFG_TABLE_DIT_E,

    /** table type represent the MPLS Table. */
    PRV_TGF_CFG_TABLE_MPLS_E

} PRV_TGF_CFG_TABLE_ENT;

/* macro to start a loop on active port groups */
#define PRV_TGF_PP_START_LOOP_PORT_GROUPS_MAC(_devNum,_portGroupId)     \
        if (((_devNum) >= PRV_CPSS_MAX_PP_DEVICES_CNS))                 \
            return; /* should never happen */                           \
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(_devNum,_portGroupId)

/* macro to end a loop on active port groups */
#define PRV_TGF_PP_END_LOOP_PORT_GROUPS_MAC(_devNum,_portGroupId)     \
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(_devNum,_portGroupId)

/* macro to start a loop on NON-active port groups */
#define PRV_TGF_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(_devNum,_portGroupId)     \
        if (((_devNum) >= PRV_CPSS_MAX_PP_DEVICES_CNS))                 \
            return; /* should never happen */                           \
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(_devNum,_portGroupId)

/* macro to end a loop on NON-active port groups */
#define PRV_TGF_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(_devNum,_portGroupId)     \
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(_devNum,_portGroupId)

/******************************************************************************\
 *                            Fabric Adapters defines                         *
\******************************************************************************/

#ifdef IMPL_FA

/* fa types - corresponding bits            */
/* used by iterator through fabric adapters */
#define UTF_CPSS_FA_TYPE_98FX900A_CNS   BIT_0
#define UTF_CPSS_FA_TYPE_98FX902A_CNS   BIT_1
#define UTF_CPSS_FA_TYPE_98FX910A_CNS   BIT_2
#define UTF_CPSS_FA_TYPE_98FX915A_CNS   BIT_3

/* widely-used type sets                            */
/* used by UTs while call prvUtfFaReset function    */
#define UTF_ALL_FA_TYPES_SET_CNS    (UTF_CPSS_FA_TYPE_98FX900A_CNS | \
                                    UTF_CPSS_FA_TYPE_98FX902A_CNS | \
                                    UTF_CPSS_FA_TYPE_98FX910A_CNS | \
                                    UTF_CPSS_FA_TYPE_98FX915A_CNS)
#endif /* IMPL_FA */


/******************************************************************************\
 *                                 Xbar defines                               *
\******************************************************************************/

#ifdef IMPL_XBAR

/* xbar types - corresponding bits          */
/* used by iterator through xbar            */
#define UTF_CPSS_XBAR_TYPE_98FX900A_CNS   BIT_0
#define UTF_CPSS_XBAR_TYPE_98FX902A_CNS   BIT_1
#define UTF_CPSS_XBAR_TYPE_98FX910A_CNS   BIT_2
#define UTF_CPSS_XBAR_TYPE_98FX915A_CNS   BIT_3
#define UTF_CPSS_XBAR_TYPE_98FX9010A_CNS  BIT_4
#define UTF_CPSS_XBAR_TYPE_98FX9110A_CNS  BIT_5
#define UTF_CPSS_XBAR_TYPE_98FX9210A_CNS  BIT_6

/* widely-used type sets                            */
/* used by UTs while call prvUtfFaReset function    */
#define UTF_ALL_XBAR_TYPES_SET_CNS  (UTF_CPSS_XBAR_TYPE_98FX900A_CNS | \
                                    UTF_CPSS_XBAR_TYPE_98FX902A_CNS  | \
                                    UTF_CPSS_XBAR_TYPE_98FX910A_CNS  | \
                                    UTF_CPSS_XBAR_TYPE_98FX915A_CNS  | \
                                    UTF_CPSS_XBAR_TYPE_98FX9010A_CNS | \
                                    UTF_CPSS_XBAR_TYPE_98FX9110A_CNS | \
                                    UTF_CPSS_XBAR_TYPE_98FX9210A_CNS)

/* External usage environment parameters */
extern CORE_XBAR_CONFIG  coreXbarDevs[PRV_CPSS_MAX_PP_DEVICES_CNS];

/* Macro to get max number of fports */
#define UTF_GET_MAX_FPORTS_NUM_MAC(_devNum) \
    (coreXbarDevs[_devNum].numOfFports)

#endif /* IMPL_XBAR */


/**
* @internal prvUtfExtrasInit function
* @endinternal
*
* @brief   Initialize extras for cpss unit testing.
*         This function inits available devices and ports per device.
*/
GT_STATUS prvUtfExtrasInit
(
    GT_VOID
);


/******************************************************************************\
 *                            Packet Processor UT API                         *
\******************************************************************************/

/**
* @internal prvUtfSetMaxIterationsOnDevice function
* @endinternal
*
* @brief   state the max number of different device that will be returned from
*         prvUtfNextDeviceGet(...)
*         NOTE:
*         1. max iterations supported = 2
*         2. the configuration is reset AFTER every call to prvUtfNextNotApplicableDeviceReset(...)
*         3. default is loop on 'all devices'
*
* @retval GT_OK                    -  All OK
* @retval GT_BAD_PARAM             - maxIterations is bad
*/
GT_STATUS prvUtfSetMaxIterationsOnDevice
(
    IN GT_U32   maxIterations
);

/**
* @internal prvUtfSetSingleIterationPerFamily function
* @endinternal
*
* @brief   state that only single device per family can be returned from
*         prvUtfNextDeviceGet(...)
*         NOTE:
*         1. the configuration is reset AFTER every call to prvUtfNextNotApplicableDeviceReset(...)
*         2. default is 'no limit'
*
* @retval GT_OK                    -  All OK
*/
GT_STATUS prvUtfSetSingleIterationPerFamily(void);

/**
* @internal prvUtfNextNotApplicableDeviceReset function
* @endinternal
*
* @brief   Reset device iterator must be called before go over all devices
*
* @param[in,out] devPtr                   - iterator thru devices
* @param[in] notApplicableFamiliesHandle - handle of non-applicable devices
*
* @retval GT_OK                    -  iterator initialization OK
* @retval GT_FAIL                  -  general failure error
*/
GT_STATUS prvUtfNextNotApplicableDeviceReset
(
    INOUT   GT_U8   *devPtr,
    IN      GT_U32  notApplicableFamiliesHandle
);

/**
* @internal prvUtfNextDeviceReset function
* @endinternal
*
* @brief   Reset device iterator must be called before go over all devices
*
* @param[in,out] devPtr                   -   iterator thru devices
*
* @retval GT_OK                    -  iterator initialization OK
* @retval GT_FAIL                  -  general failure error
*/
GT_STATUS prvUtfNextDeviceReset
(
    INOUT   GT_U8   *devPtr,
    IN      GT_U32  applicableFamiliesHandle
);

/**
* @internal prvUtfNextDeviceGet function
* @endinternal
*
* @brief   This routine returns next active/non active device id.
*         Call the function until non GT_OK error. Note that
*         iterator must be initialized before.
* @param[in,out] devPtr                   -   device id
* @param[in] activeIs                 -   set to GT_TRUE for getting active devices
*/
GT_STATUS prvUtfNextDeviceGet
(
    INOUT GT_U8     *devPtr,
    IN GT_BOOL      activeIs
);

/**
* @internal prvUtfNextPhyPortReset function
* @endinternal
*
* @brief   Reset phy ports iterator must be called before go over all ports
*
* @param[in,out] portPtr                  -   iterator thru ports
* @param[in] dev                      -   device id of ports
*
* @retval GT_OK                    -  iterator initialization OK
* @retval GT_FAIL                  -  general failure error
*/
GT_STATUS prvUtfNextPhyPortReset
(
    INOUT UTF_PHYSICAL_PORT_NUM     *portPtr,
    IN GT_U8        dev
);

/**
* @internal prvUtfNextPhyPortGet function
* @endinternal
*
* @brief   This routine returns next active/non active physical port id.
*         Call the function until non GT_OK error. Note that
*         iterator must be initialized before.
* @param[in,out] portPtr                  -   port id
* @param[in] activeIs                 -   set to GT_TRUE for getting active ports
*/
GT_STATUS prvUtfNextPhyPortGet
(
    INOUT UTF_PHYSICAL_PORT_NUM     *portPtr,
    IN GT_BOOL      activeIs
);

/**
* @internal prvUtfNextDefaultEPortReset function
* @endinternal
*
* @brief   Reset 'Default ePort' iterator must be called before go over all default ePorts.
*         for device that not support default ePorts this iteration is just like
*         prvUtfNextPhyPortReset().
* @param[in] hwDev                    -   HW device id of ports
*
* @param[out] portPtr                  -   (pointer to) first iteration
*
* @retval GT_OK                    -  iterator initialization OK
* @retval GT_FAIL                  -  general failure error
*/
GT_STATUS prvUtfNextDefaultEPortReset
(
    OUT   GT_PORT_NUM     *portPtr,
    IN    GT_HW_DEV_NUM    hwDev
);

/**
* @internal prvUtfNextDefaultEPortGet function
* @endinternal
*
* @brief   This routine returns next active/non active 'default ePort' id.
*         for device that not support default ePorts this iteration is just like
*         prvUtfNextPhyPortGet().
*         Call the function until non GT_OK error. Note that
*         iterator must be initialized before.
* @param[in,out] portPtr                  -   port id
* @param[in] activeIs                 -   set to GT_TRUE for getting active ports
*/
GT_STATUS prvUtfNextDefaultEPortGet
(
    INOUT GT_PORT_NUM       *portPtr,
    IN GT_BOOL              activeIs
);

/**
* @internal prvUtfMaxDefaultEPortNumberGet function
* @endinternal
*
* @brief   Get the number of default ePorts.
*
* @param[out] defaultEPortsNum         - number of defauls ePorts.
*                                      GT_OK           -   Get next port was OK
*                                      GT_FAIL         -   There is no active ports
*                                      COMMENTS:
*                                      None.
*/
GT_STATUS prvUtfMaxDefaultEPortNumberGet
(
    OUT GT_PORT_NUM       *defaultEPortsNum
);

/**
* @internal prvUtfNextVirtPortReset function
* @endinternal
*
* @brief   Reset virtual ports iterator must be called before go over all ports
*
* @param[in,out] portPtr                  -   iterator thru ports
* @param[in] hwDev                    -   HW device id of ports
*
* @retval GT_OK                    -  iterator initialization OK
* @retval GT_FAIL                  -  general failure error
*/
GT_STATUS prvUtfNextVirtPortReset
(
    INOUT UTF_PORT_NUM    *portPtr,
    IN    GT_HW_DEV_NUM    hwDev
);

/**
* @internal prvUtfNextVirtPortGet function
* @endinternal
*
* @brief   This routine returns next active/non active virtual port id.
*         Call the function until non GT_OK error. Note that
*         iterator must be initialized before.
* @param[in,out] portPtr                  -   port id
* @param[in] activeIs                 -   set to GT_TRUE for getting active ports
*/
GT_STATUS prvUtfNextVirtPortGet
(
    INOUT UTF_PORT_NUM       *portPtr,
    IN GT_BOOL              activeIs
);

/**
* @internal prvUtfNextMacPortReset function
* @endinternal
*
* @brief   Reset MAC ports iterator must be called before go over all ports
*
* @param[in,out] portPtr                  -   iterator thru ports
* @param[in] dev                      -   device id of ports
*
* @retval GT_OK                    -  iterator initialization OK
* @retval GT_FAIL                  -  general failure error
*/
GT_STATUS prvUtfNextMacPortReset
(
    INOUT UTF_PHYSICAL_PORT_NUM     *portPtr,
    IN GT_U8        dev
);

/**
* @internal prvUtfNextMacPortGet function
* @endinternal
*
* @brief   This routine returns next active/non active MAC port id.
*         Call the function until non GT_OK error. Note that
*         iterator must be initialized before.
* @param[in,out] portPtr                  -   port id
* @param[in] activeIs                 -   set to GT_TRUE for getting active ports
*/
GT_STATUS prvUtfNextMacPortGet
(
    INOUT UTF_PHYSICAL_PORT_NUM     *portPtr,
    IN GT_BOOL      activeIs
);

/**
* @internal prvUtfNextTxqPortReset function
* @endinternal
*
* @brief   Reset TXQ ports iterator must be called before go over all ports
*
* @param[in,out] portPtr                  -   iterator thru ports
* @param[in] dev                      -   device id of ports
*
* @retval GT_OK                    -  iterator initialization OK
* @retval GT_FAIL                  -  general failure error
*/
GT_STATUS prvUtfNextTxqPortReset
(
    INOUT UTF_PHYSICAL_PORT_NUM     *portPtr,
    IN GT_U8        dev
);

/**
* @internal prvUtfNextTxqPortGet function
* @endinternal
*
* @brief   This routine returns next active/non active TXQ port id.
*         Call the function until non GT_OK error. Note that
*         iterator must be initialized before.
* @param[in,out] portPtr                  -   port id
* @param[in] activeIs                 -   set to GT_TRUE for getting active ports
*/
GT_STATUS prvUtfNextTxqPortGet
(
    INOUT UTF_PHYSICAL_PORT_NUM     *portPtr,
    IN GT_BOOL      activeIs
);

/**
* @internal prvUtfNextRxDmaPortReset function
* @endinternal
*
* @brief   Reset RxDma ports iterator. Must be called before go over all ports.
*
* @param[in,out] portPtr                  -   iterator thru ports
* @param[in] dev                      -   device id of ports
*
* @retval GT_OK                    -  iterator initialization OK
* @retval GT_FAIL                  -  general failure error
*/
GT_STATUS prvUtfNextRxDmaPortReset
(
    INOUT   UTF_PHYSICAL_PORT_NUM   *portPtr,
    IN      GT_U8                   dev
);

/**
* @internal prvUtfNextRxDmaPortGet function
* @endinternal
*
* @brief   This routine returns next active/non active RxDma port id.
*         Call the function until non GT_OK error.
*         Note that iterator must be initialized before.
* @param[in,out] portPtr                  -   port id
* @param[in] activeIs                 -   set to GT_TRUE for getting active ports
*/
GT_STATUS prvUtfNextRxDmaPortGet
(
    INOUT   UTF_PHYSICAL_PORT_NUM   *portPtr,
    IN      GT_BOOL                 activeIs
);

/**
* @internal prvUtfNextTxDmaPortReset function
* @endinternal
*
* @brief   Reset TxDma ports iterator. Must be called before go over all ports.
*
* @param[in,out] portPtr                  -   iterator thru ports
* @param[in] dev                      -   device id of ports
*
* @retval GT_OK                    -  iterator initialization OK
* @retval GT_FAIL                  -  general failure error
*/
GT_STATUS prvUtfNextTxDmaPortReset
(
    INOUT   UTF_PHYSICAL_PORT_NUM   *portPtr,
    IN      GT_U8                   dev
);

/**
* @internal prvUtfNextTxDmaPortGet function
* @endinternal
*
* @brief   This routine returns next active/non active TxDma port id.
*         Call the function until non GT_OK error.
*         Note that iterator must be initialized before.
* @param[in,out] portPtr                  -   port id
* @param[in] activeIs                 -   set to GT_TRUE for getting active ports
*/
GT_STATUS prvUtfNextTxDmaPortGet
(
    INOUT   UTF_PHYSICAL_PORT_NUM   *portPtr,
    IN      GT_BOOL                 activeIs
);

/**
* @internal prvUtfPortTypeGet function
* @endinternal
*
* @brief   This routine returns port type.
*
* @param[in] dev                      -  device id
* @param[in] port                     -  physical port
*
* @param[out] portTypePtr              -   type of port
*                                      GT_OK           -   Get port type was OK
*                                      GT_BAD_PARAM    -   Invalid port or device id
*                                      GT_BAD_PTR      -   Null pointer
*                                      COMMENTS:
*                                      None.
*/
GT_STATUS prvUtfPortTypeGet
(
    IN GT_U8                    dev,
    IN UTF_PHYSICAL_PORT_NUM     port,
    OUT PRV_CPSS_PORT_TYPE_ENT  *portTypePtr
);

/**
* @internal prvUtfPortMacNumberGet function
* @endinternal
*
* @brief   This routine returns MAC number of a port.
*
* @param[in] dev                      -  device id
* @param[in] port                     -  physical port
*
* @param[out] macNumPtr                -   MAC number of port
*                                      GT_OK           -   Get port type was OK
*                                      GT_BAD_PARAM    -   Invalid port or device id
*                                      GT_BAD_PTR      -   Null pointer
*                                      COMMENTS:
*                                      None.
*/
GT_STATUS prvUtfPortMacNumberGet
(
    IN GT_U8                    dev,
    IN GT_PHYSICAL_PORT_NUM     port,
    OUT GT_U32                 *macNumPtr
);


/**
* @internal prvUtfPortMacModuloCalc function
* @endinternal
*
* @brief   This routine returns modulo of MAC number of a port.
*
* @param[in] dev                      -  device id
* @param[in] port                     -  physical port
* @param[in] modulo                   -  modulo
*
* @param[out] modulo                   of MAC number of port
*                                      COMMENTS:
*                                      None.
*/
GT_U32 prvUtfPortMacModuloCalc
(
    IN GT_U8                    dev,
    IN GT_PHYSICAL_PORT_NUM     port,
    IN GT_U32                   modulo
);

/**
* @internal prvUtfPortInterfaceModeGet function
* @endinternal
*
* @brief   This routine returns port interface mode.
*
* @param[in] dev                      -  device id
* @param[in] port                     -  physical port
*
* @param[out] portIfModePtr            -   (pointer to) port interface mode
*                                      GT_OK           -   Get port type was OK
*                                      GT_BAD_PARAM    -   Invalid port or device id
*                                      GT_BAD_PTR      -   Null pointer
*                                      COMMENTS:
*                                      None.
*/
GT_STATUS prvUtfPortInterfaceModeGet
(
    IN GT_U8                            dev,
    IN GT_PHYSICAL_PORT_NUM             port,
    OUT CPSS_PORT_INTERFACE_MODE_ENT    *portIfModePtr
);

/**
* @internal prvUtfPortSpeedGet function
* @endinternal
*
* @brief   This routine returns port speed.
*
* @param[in] dev                      -  device id
* @param[in] port                     -  physical port
*
* @param[out] portSpeedPtr             -   (pointer to) port speed
*                                      GT_OK           -   Get port type was OK
*                                      GT_BAD_PARAM    -   Invalid port or device id
*                                      GT_BAD_PTR      -   Null pointer
*                                      COMMENTS:
*                                      None.
*/
GT_STATUS prvUtfPortSpeedGet
(
    IN GT_U8                    dev,
    IN GT_PHYSICAL_PORT_NUM     port,
    OUT CPSS_PORT_SPEED_ENT     *portSpeedPtr
);

/* TRUE if XG port supports XAUI interface */
#define IS_XAUI_CAPABLE_XG_PORT(portTypeOptions) \
        ((portTypeOptions == PRV_CPSS_XG_PORT_XG_ONLY_E)    ||  \
         (portTypeOptions == PRV_CPSS_XG_PORT_XG_HX_QX_E)   ||  \
         (portTypeOptions == PRV_CPSS_XG_PORT_XLG_SGMII_E)  ||  \
         (portTypeOptions == PRV_CPSS_XG_PORT_CG_SGMII_E))

/* TRUE if FE/GE not supported */
#define GE_NOT_SUPPORTED(portTypeOptions) \
        ((portTypeOptions == PRV_CPSS_XG_PORT_XG_ONLY_E) || \
         (portTypeOptions == PRV_CPSS_XG_PORT_HX_QX_ONLY_E))

/* TRUE if port supports SGMII interface */
#define IS_SGMII_CAPABLE_PORT(portTypeOptions) \
        ((portTypeOptions == PRV_CPSS_GE_PORT_GE_ONLY_E)    ||  \
         (portTypeOptions == PRV_CPSS_XG_PORT_XG_HX_QX_E)   ||  \
         (portTypeOptions == PRV_CPSS_XG_PORT_XLG_SGMII_E)  ||  \
         (portTypeOptions == PRV_CPSS_XG_PORT_CG_SGMII_E))

/**
* @internal prvUtfPortTypeOptionsGet function
* @endinternal
*
* @brief   This routine returns port type options.
*
* @param[in] dev                      -  device id
* @param[in] port                     -  physical port
*
* @param[out] portTypeOptionsPtr       -   type of port
*                                      GT_OK           -   Get port type was OK
*                                      GT_BAD_PARAM    -   Invalid port or device id
*                                      GT_BAD_PTR      -   Null pointer
*                                      COMMENTS:
*                                      None.
*/
GT_STATUS prvUtfPortTypeOptionsGet
(
    IN GT_U8                            dev,
    IN UTF_PHYSICAL_PORT_NUM            port,
    OUT PRV_CPSS_PORT_TYPE_OPTIONS_ENT  *portTypeOptionsPtr
);


/**
* @internal prvUtfPortIsFlexLinkGet function
* @endinternal
*
* @brief   This routine returns port is flex link flag.
*
* @param[in] dev                      -  device id
* @param[in] port                     -  physical port
*
* @param[out] isFlexLinkPtr            -   is flex link
*                                      GT_OK           -   Get port type was OK
*                                      GT_BAD_PARAM    -   Invalid port or device id
*                                      GT_BAD_PTR      -   Null pointer
*                                      COMMENTS:
*                                      None.
*/
GT_STATUS prvUtfPortIsFlexLinkGet
(
    IN GT_U8                    dev,
    IN UTF_PHYSICAL_PORT_NUM    port,
    OUT GT_BOOL                 *isFlexLinkPtr
);


/**
* @internal prvUtfDeviceTypeGet function
* @endinternal
*
* @brief   This routine returns device type. Some tests require device type
*         to make proper function behaviour validation.
* @param[in] dev                      -  device id
*
* @param[out] devTypePtr               -   type of device
*                                      GT_OK           -   Get device type was OK
*                                      GT_BAD_PARAM    -   Invalid device id
*                                      GT_BAD_PTR      -   Null pointer
*                                      COMMENTS:
*                                      None.
*/
GT_STATUS prvUtfDeviceTypeGet
(
    IN GT_U8                    dev,
    OUT CPSS_PP_DEVICE_TYPE     *devTypePtr
);

/**
* @internal prvUtfDeviceFamilyGet function
* @endinternal
*
* @brief   This routine returns device family. Some tests require device family
*         to make proper function behaviour validation.
* @param[in] dev                      -  device id
*
* @param[out] devFamilyPtr             -   family of device
*                                      GT_OK           -   Get device family was OK
*                                      GT_BAD_PARAM    -   Invalid device id
*                                      GT_BAD_PTR      -   Null pointer
*                                      COMMENTS:
*                                      None.
*/
GT_STATUS prvUtfDeviceFamilyGet
(
    IN  GT_U8                        dev,
    OUT CPSS_PP_FAMILY_TYPE_ENT      *devFamilyPtr
);

/**
* @internal prvUtfMaxMcGroupsGet function
* @endinternal
*
* @brief   This routine returns maximum value for MC groups per device.
*
* @param[in] dev                      -  device id
*
* @param[out] maxMcGroupsPtr           -   maximum value for multicast groups
*                                      GT_OK           -   Get max MC groups was OK
*                                      GT_BAD_PARAM    -   Invalid device id
*                                      GT_BAD_PTR      -   Null pointer
*                                      COMMENTS:
*                                      None.
*/
GT_STATUS prvUtfMaxMcGroupsGet
(
    IN GT_U8    dev,
    OUT GT_U16  *maxMcGroupsPtr
);

/**
* @internal prvUtfNumPortsGet function
* @endinternal
*
* @brief   Get number of physical ports in the device
*
* @param[in] dev                      -   device id of ports
*
* @param[out] numOfPortsPtr            - (pointer to)number of physical ports
*
* @retval GT_OK                    -  Get the number of physical ports in the device OK
* @retval GT_BAD_PARAM             -  Invalid device id
* @retval GT_BAD_PTR               -  Null pointer
*/
GT_STATUS prvUtfNumPortsGet
(
    IN GT_U8        dev,
    OUT GT_U32       *numOfPortsPtr
);

/**
* @internal prvUtfNumVirtualPortsGet function
* @endinternal
*
* @brief   Get number of Virtual ports in the device
*
* @param[in] dev                      -   device id of ports
*
* @retval GT_OK                    -  Get the number of virtual ports in the device OK
* @retval GT_BAD_PARAM             -  Invalid device id
* @retval GT_BAD_PTR               -  Null pointer
*/
GT_STATUS prvUtfNumVirtualPortsGet
(
    IN GT_U8        dev,
    OUT GT_U32      *numOfVirtualPortsPtr
);

/**
* @internal prvUtfDeviceRevisionGet function
* @endinternal
*
* @brief   Get revision number of device
*
* @param[in] dev                      -   device id of ports
*
* @param[out] revisionPtr              - (pointer to)revision number
*
* @retval GT_OK                    -  Get revision of device OK
* @retval GT_BAD_PARAM             -  Invalid device id
* @retval GT_BAD_PTR               -  Null pointer
*/
GT_STATUS prvUtfDeviceRevisionGet
(
    IN GT_U8        dev,
    OUT GT_U8       *revisionPtr
);

/**
* @internal prvUtfDeviceTestSupport function
* @endinternal
*
* @brief   check if the device supported for the test type
*
* @param[in] devNum                   -   device id
* @param[in] utTestSupport            - bmp of test types support , see :
*                                      UTF_CPSS_PP_FAMILY_TWISTC_CNS
*                                      UTF_CPSS_PP_FAMILY_TWISTD_CNS
*                                      UTF_CPSS_PP_FAMILY_SAMBA_CNS
*                                      UTF_CPSS_PP_FAMILY_SALSA_CNS
*                                      UTF_CPSS_PP_FAMILY_SOHO_CNS
*                                      UTF_CPSS_PP_FAMILY_CHEETAH_CNS
*                                      UTF_CPSS_PP_FAMILY_CHEETAH2_CNS
*                                      UTF_CPSS_PP_FAMILY_CHEETAH3_CNS
*                                      UTF_CPSS_PP_FAMILY_TIGER_CNS
*                                      UTF_CPSS_PP_FAMILY_PUMA_CNS
*                                      UTF_CPSS_PP_FAMILY_XCAT_CNS
*                                      UTF_CPSS_PP_FAMILY_LION_A_CNS
*                                      UTF_CPSS_PP_FAMILY_LION_CNS
*
* @retval GT_TRUE                  - the device support the test type
* @retval GT_FALSE                 - the device not support the test type
*/
GT_BOOL prvUtfDeviceTestSupport
(
    IN GT_U8        devNum,
    IN GT_U32       utTestSupport
);

/**
* @internal prvUtfDeviceTestNotSupport function
* @endinternal
*
* @brief   Check if the device is not supported for the test type
*
* @param[in] devNum                   - device id
* @param[in] notApplicableDeviceBmp   - bmp of not applicable device types
*
* @retval GT_TRUE                  - the device support the test type
* @retval GT_FALSE                 - the device not support the test type
*/
GT_BOOL prvUtfDeviceTestNotSupport
(
    IN GT_U8        devNum,
    IN GT_U32       notApplicableDeviceBmp
);

/**
* @internal prvUtfSkipTestsNumReset function
* @endinternal
*
* @brief   Reset skip test counter
*/
GT_VOID prvUtfSkipTestsNumReset
(
    GT_VOID
);

/**
* @internal prvUtfSkipTestsNumGet function
* @endinternal
*
* @brief   Get total number of skipped tests
*/
GT_U32 prvUtfSkipTestsNumGet
(
    GT_VOID
);

/**
* @internal prvUtfSkipTestsFlagReset function
* @endinternal
*
* @brief   Reset skip test flag
*/
GT_VOID prvUtfSkipTestsFlagReset
(
    GT_VOID
);

/**
* @internal prvUtfSkipTestsFlagGet function
* @endinternal
*
* @brief   Get current skip test flag
*/
GT_BOOL prvUtfSkipTestsFlagGet
(
    GT_VOID
);

/**
* @internal prvUtfSkipTestsSet function
* @endinternal
*
* @brief   Set skip flag and update number of skipped tests
*/
GT_VOID prvUtfSkipTestsSet
(
    GT_VOID
);

/**
* @internal prvUtfSkipLongTestsFlagSet function
* @endinternal
*
* @brief   Set skip long test flag
*
* @param[in] enable                   - enable\disable skip long tests
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvUtfSkipLongTestsFlagSet
(
    IN GT_BOOL enable
);

/**
* @internal prvUtfSkipLongTestsFlagGet function
* @endinternal
*
* @brief   Get skip long test flag
*
* @param[in] skipFamilyBmp            - bitmap of families for which skip flag applyed
*                                       skip long tests flag
*/
GT_BOOL prvUtfSkipLongTestsFlagGet
(
    IN GT_U32 skipFamilyBmp
);

/**
* @internal prvUtfCheckFamilyBmp function
* @endinternal
*
* @brief   Check that tested device is part of familyBmp
*
* @param[in] familyBmp                - bitmap of families
*
* @retval GT_TRUE                  - tested device is part of familyBmp
* @retval GT_FALSE                 - tested device is not part of familyBmp
*/
GT_BOOL prvUtfCheckFamilyBmp
(
    IN GT_U32 familyBmp
);

/**
* @internal prvUtfSkipNonBaselineTestsFlagGet function
* @endinternal
*
* @brief   Get skip test flag for non baseline tests execution
*/
GT_BOOL prvUtfSkipNonBaselineTestsFlagGet
(
    IN GT_VOID
);

/**
* @internal prvUtfBaselineTestsExecutionFlagGet function
* @endinternal
*
* @brief   Get baseline tests execution flag
*/
GT_BOOL prvUtfBaselineTestsExecutionFlagGet
(
    IN GT_VOID
);

/**
* @internal prvUtfBaselineTestsExecutionFlagSet function
* @endinternal
*
* @brief   Set baseline tests execution flag
*
* @param[in] enable                   - baseline tests execution flag
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvUtfBaselineTestsExecutionFlagSet
(
    IN GT_BOOL enable
);

/**
* @internal prvUtfReduceLogSizeFlagGet function
* @endinternal
*
* @brief   Get a flag of limitation of CPSS API logging
*/
GT_BOOL prvUtfReduceLogSizeFlagGet
(
    IN GT_VOID
);

/**
* @internal prvUtfReduceLogSizeFlagSet function
* @endinternal
*
* @brief   Set a flag of limitation of CPSS API logging
*
* @param[in] enable                   - set (GT_TRUE) or reset(GT_FALSE) the flag
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvUtfReduceLogSizeFlagSet
(
    IN GT_BOOL enable
);

/**
* @internal prvUtfTrunkConvertFromTestToCpss function
* @endinternal
*
* @brief   convert trunkId from test value
*         to value that the CPSS expect
* @param[in,out] trunkIdPtr               - (pointer to) the trunkId of the test
* @param[in,out] trunkIdPtr               - (pointer to) the trunkId of the CPSS
*                                       none
*/
void prvUtfTrunkConvertFromTestToCpss
(
   INOUT GT_TRUNK_ID *trunkIdPtr
);

/**
* @internal prvUtfTrunkConvertFromCpssToTest function
* @endinternal
*
* @brief   convert trunkId from CPSS value to value that the TEST expect (get)
*
* @param[in,out] trunkIdPtr               - (pointer to) the trunkId of the CPSS
* @param[in,out] trunkIdPtr               - (pointer to) the trunkId of the test
*                                       none
*/
void prvUtfTrunkConvertFromCpssToTest
(
   INOUT GT_TRUNK_ID *trunkIdPtr
);

/* Don't do casting to GT_TRUNK_ID for trunkId. it must be of this type already */
#define CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(_trunkId) \
   prvUtfTrunkConvertFromTestToCpss(&_trunkId)

#define CONVERT_TRUNK_ID_CPSS_TO_TEST_MAC(_trunkId) \
   prvUtfTrunkConvertFromCpssToTest(&_trunkId)

/* flag to state that the DXCH (xcat) devices need the WA of trunk-Id conversions */
extern GT_BOOL prvUtfDxChTrunkWaNeeded;


#if (defined EX_FAMILY) || (defined MX_FAMILY)

/**
* @internal prvUtfExMxMaxNumOfPolicerEntriesGet function
* @endinternal
*
* @brief   This routine returns maximum number of policer entries per device.
*
* @param[in] dev                      -  ExMx device id
*/
GT_STATUS prvUtfExMxMaxNumOfPolicerEntriesGet
(
    IN  GT_U8    dev,
    OUT GT_U32   *maxNumOfPolicerEntriesPtr
);

#endif /* (defined EX_FAMILY) || (defined MX_FAMILY) */

/*******************************************************************************
* prvUtfCfgTableNumEntriesGet
*
*       the function return the number of entries of each individual table in
*       the HW.
*
* INPUTS:
*       dev     - physical device number
*       table   - type of the specific table
*
* OUTPUTS:
*       numEntriesPtr - (pointer to) number of entries
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - on wrong device number or table type
*       GT_BAD_PTR      - on NULL pointer
*
* COMMENTS:
*       None.
*******************************************************************************/
GT_STATUS prvUtfCfgTableNumEntriesGet
(
    IN GT_U8                 dev,
    IN PRV_TGF_CFG_TABLE_ENT table,
    OUT GT_U32               *numEntriesPtr
);

/******************************************************************************\
 *                            Fabric Adapter UT API                           *
\******************************************************************************/

#ifdef IMPL_FA
/**
* @internal prvUtfNextFaReset function
* @endinternal
*
* @brief   Reset device fabric adapter iterator. Must be called before go over all
*         device fabric adapters
* @param[in,out] devPtr                   -   iterator thru devices
* @param[in] applicableTypesHandle    - handle of applicable FA devices type
*
* @retval GT_OK                    -  iterator initialization OK
* @retval GT_FAIL                  -  general failure error
*/
GT_STATUS prvUtfNextFaReset
(
    INOUT   GT_U8   *devPtr,
    IN      GT_U32  applicableTypesHandle
);

/**
* @internal prvUtfNextFaGet function
* @endinternal
*
* @brief   This routine returns next active/non active device fabric adapter id.
*         Call the function until non GT_OK error. Note that
*         iterator must be initialized before.
* @param[in,out] devPtr                   - device id
* @param[in] activeIs                 - set to GT_TRUE for getting active FA devices
*/
GT_STATUS prvUtfNextFaGet
(
    INOUT GT_U8     *devPtr,
    IN GT_BOOL      activeIs
);

/**
* @internal prvUtfNextFaPortReset function
* @endinternal
*
* @brief   Reset fabric ports iterator must be called before go over all ports
*
* @param[in,out] portPtr                  - iterator thru FA ports
* @param[in] dev                      - FA device id of ports
*
* @retval GT_OK                    - iterator initialization OK
* @retval GT_FAIL                  - general failure error
*/
GT_STATUS prvUtfNextFaPortReset
(
    INOUT GT_U8     *portPtr,
    IN GT_U8        dev
);

/**
* @internal prvUtfNextFaPortGet function
* @endinternal
*
* @brief   This routine returns next active/non active fabric port id.
*         Call the function until non GT_OK error. Note that
*         iterator must be initialized before.
* @param[in,out] portPtr                  - FA port id
* @param[in] activeIs                 - set to GT_TRUE for getting active ports
*/
GT_STATUS prvUtfNextFaPortGet
(
    INOUT GT_U8     *portPtr,
    IN GT_BOOL      activeIs
);

/**
* @internal prvUtfFaDeviceTypeGet function
* @endinternal
*
* @brief   This routine returns device fabric adapter type. Some tests require
*         device type to make proper function behaviour validation.
* @param[in] dev                      - FA device id
*
* @param[out] devTypePtr               - type of FA device
*                                      GT_OK        - Get device type was OK
*                                      GT_BAD_PARAM - Invalid device id
*                                      GT_BAD_PTR   - Null pointer
*                                      COMMENTS:
*                                      None.
*/
GT_STATUS prvUtfFaDeviceTypeGet
(
    IN GT_U8             dev,
    OUT GT_FA_DEVICE     *devTypePtr
);

/**
* @internal prvUtfFaDeviceRevisionGet function
* @endinternal
*
* @brief   Get revision number of device fabric adapter
*
* @param[in] dev                      - fabric adapter id
*
* @param[out] revisionPtr              - (pointer to)revision number
*
* @retval GT_OK                    - Get revision of device OK
* @retval GT_BAD_PARAM             - Invalid device id
* @retval GT_BAD_PTR               - Null pointer
*/
GT_STATUS prvUtfFaDeviceRevisionGet
(
    IN GT_U8       dev,
    OUT GT_U32     *revisionPtr
);

/**
* @internal prvUtfNumFaPortsGet function
* @endinternal
*
* @brief   get the number of fabric ports in the device
*
* @param[in] dev                      - FA device id of ports
*
* @retval GT_OK                    - get the number of fabric ports in the device OK
* @retval GT_BAD_PARAM             - Invalid device id
* @retval GT_BAD_PTR               - Null pointer
*/
GT_STATUS prvUtfNumFaPortsGet
(
    IN GT_U8        dev,
    OUT GT_U8       *numOfFaPortsPtr
);

/**
* @internal prvUtfFaMaxNumPpGet function
* @endinternal
*
* @brief   get number of packet processors in the device.
*
* @param[in] dev                      - FA device id
*
* @param[out] maxNumDevPtr             - (pointer to)number of packet processors
*
* @retval GT_OK                    - get the number of packet processors in the device OK
* @retval GT_BAD_PARAM             - Invalid device id
* @retval GT_BAD_PTR               - Null pointer
*/
GT_STATUS prvUtfFaMaxNumPpGet
(
    IN  GT_U8              dev,
    OUT GT_FA_MAX_NUM_DEV  *maxNumDevPtr
);

#endif /* IMPL_FA */


/******************************************************************************\
 *                                Xbar UT API                                 *
\******************************************************************************/

#ifdef IMPL_XBAR
/**
* @internal prvUtfNextXbarReset function
* @endinternal
*
* @brief   Reset Xbar device iterator. Must be called before go over all
*         Xbar device
* @param[in,out] devPtr                   -   iterator thru devices
* @param[in] applicableTypesHandle    - handle of applicable Xbar devices type
*
* @retval GT_OK                    -  iterator initialization OK
* @retval GT_FAIL                  -  general failure error
*/
GT_STATUS prvUtfNextXbarReset
(
    INOUT   GT_U8   *devPtr,
    IN      GT_U32  applicableTypesHandle
);

/**
* @internal prvUtfNextXbarGet function
* @endinternal
*
* @brief   This routine returns next active/non active Xbar device id.
*         Call the function until non GT_OK error. Note that
*         iterator must be initialized before.
* @param[in,out] devPtr                   - device id
* @param[in] activeIs                 - set to GT_TRUE for getting active Xbar devices
*/
GT_STATUS prvUtfNextXbarGet
(
    INOUT GT_U8     *devPtr,
    IN GT_BOOL      activeIs
);

/**
* @internal prvUtfNextXbarFaPortReset function
* @endinternal
*
* @brief   Reset fabric ports iterator must be called before go over all ports
*
* @param[in,out] portPtr                  - iterator thru FA ports
* @param[in] dev                      - Xbar device id of ports
*
* @retval GT_OK                    - iterator initialization OK
* @retval GT_FAIL                  - general failure error
*/
GT_STATUS prvUtfNextXbarFaPortReset
(
    INOUT GT_U8     *portPtr,
    IN GT_U8        dev
);

/**
* @internal prvUtfNextXbarFaPortGet function
* @endinternal
*
* @brief   This routine returns next active/non active fabric port id.
*         Call the function until non GT_OK error. Note that
*         iterator must be initialized before.
* @param[in,out] portPtr                  - FA port id
* @param[in] activeIs                 - set to GT_TRUE for getting active ports
*/
GT_STATUS prvUtfNextXbarFaPortGet
(
    INOUT GT_U8     *portPtr,
    IN GT_BOOL      activeIs
);

#endif /* IMPL_XBAR */


typedef GT_STATUS (*PRV_UTF_CPSS_GET_COUNTER_FUN)
(
    IN  GT_U8                   devNum,
    OUT GT_U32                  *counterValue
);

typedef GT_STATUS (*PRV_UTF_CPSS_SET_COUNTER_FUN)
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  counterValue
);

typedef GT_STATUS (*PRV_UTF_CPSS_GET_PER_GROUP_COUNTER_FUN)
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    OUT GT_U32                  *counterValue
);

typedef GT_STATUS (*PRV_UTF_CPSS_SET_PER_GROUP_COUNTER_FUN)
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    IN  GT_U32                  counterValue
);

typedef GT_STATUS (*PRV_UTF_CPSS_GET_COUNTER_PER_REG_FUN)
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  regAddr,
    OUT GT_U32                  *counterValue
);

typedef GT_STATUS (*PRV_UTF_CPSS_SET_COUNTER_PER_REG_FUN)
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  regAddr,
    IN  GT_U32                  counterValue
);

typedef GT_STATUS (*PRV_UTF_CPSS_GET_PER_GROUP_COUNTER_PER_REG_FUN)
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    IN  GT_U32                  regAddr,
    OUT GT_U32                  *counterValue
);

typedef GT_STATUS (*PRV_UTF_CPSS_SET_PER_GROUP_COUNTER_PER_REG_FUN)
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    IN  GT_U32                  regAddr,
    IN  GT_U32                  counterValue
);

/**
* @internal prvUtfPerPortGroupCounterGet function
* @endinternal
*
* @brief   This Function checks counters in the simulation.
*/
GT_VOID prvUtfPerPortGroupCounterGet
(
    IN GT_U32                                   regAddr,
    IN GT_U32                                   fieldOffset,
    IN GT_U32                                   fieldLength,
    IN PRV_UTF_CPSS_GET_COUNTER_FUN             getCounter,
    IN PRV_UTF_CPSS_SET_COUNTER_FUN             setCounter,
    IN PRV_UTF_CPSS_GET_PER_GROUP_COUNTER_FUN   getPerGroupCounter,
    IN PRV_UTF_CPSS_SET_PER_GROUP_COUNTER_FUN   setPerGroupCounter
);

/**
* @internal prvUtfPerPortGroupCounterPerRegGet function
* @endinternal
*
* @brief   This Function checks counters in the simulation for
*         specific register adddress.
* @param[in] regAddr                  - The register's address to write to.
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be written to register.
* @param[in] getCounterPerReg         - Pointer to get counter function.
* @param[in] setCounterPerReg         - Pointer to set counter function.
* @param[in] getPerGroupCounterPerReg - Pointer to get per port group counter function.
* @param[in] setPerGroupCounterPerReg - Pointer to set per port group counter function.
*/
GT_VOID prvUtfPerPortGroupCounterPerRegGet
(
    IN GT_U32                                           regAddr,
    IN GT_U32                                           fieldOffset,
    IN GT_U32                                           fieldLength,
    IN PRV_UTF_CPSS_GET_COUNTER_PER_REG_FUN             getCounterPerReg,
    IN PRV_UTF_CPSS_SET_COUNTER_PER_REG_FUN             setCounterPerReg,
    IN PRV_UTF_CPSS_GET_PER_GROUP_COUNTER_PER_REG_FUN   getPerGroupCounterPerReg,
    IN PRV_UTF_CPSS_SET_PER_GROUP_COUNTER_PER_REG_FUN   setPerGroupCounterPerReg
);

/**
* @internal prvUtfIsPbrModeUsed function
* @endinternal
*
* @brief   This routine returns GT_TRUE if PBR mode used. GT_FALSE otherwise.
*/
GT_BOOL prvUtfIsPbrModeUsed
(
    GT_VOID
);

/**
* @internal prvUtfIsTrafficManagerUsed function
* @endinternal
*
* @brief   This routine returns GT_TRUE if Traffic Manager (TM) used. GT_FALSE otherwise.
*/
GT_BOOL prvUtfIsTrafficManagerUsed
(
    GT_VOID
);

/**
* @internal prvUtfDeviceCoreClockGet function
* @endinternal
*
* @brief   Get core clock in Herz of device
*
* @param[in] dev                      -   device id of ports
*
* @param[out] coreClockPtr             - (pointer to)core clock in Herz
*
* @retval GT_OK                    -  Get revision of device OK
* @retval GT_BAD_PARAM             -  Invalid device id
* @retval GT_BAD_PTR               -  Null pointer
*/
GT_STATUS prvUtfDeviceCoreClockGet
(
    IN GT_U8          dev,
    OUT GT_U32       *coreClockPtr
);


/**
* @internal prvUtfSeedFromStreamNameGet function
* @endinternal
*
* @brief   Generate random seed value from CPSS stream name
*/
GT_U32   prvUtfSeedFromStreamNameGet
(
    GT_VOID
);

/**
* @internal prvUtfRandomVrfIdNumberGet function
* @endinternal
*
* @brief   This function generate random vrfId number in range [0..4095] and
*         print one
*/
GT_U32  prvUtfRandomVrfIdNumberGet
(
    GT_VOID
);

/**
* @internal prvUtfIsGmCompilation function
* @endinternal
*
* @brief   Check if the image was compiled with 'GM_USED' (Golden model - simulation).
*         the 'GM' is 'heavy' and runtime with low performance.
*         so long iterations should be shorten...
*
* @retval GT_TRUE                  - the image was compiled with 'GM_USED'
* @retval GT_FALSE                 - the image was NOT compiled with 'GM_USED'
*/
GT_BOOL prvUtfIsGmCompilation
(
    void
);

/**
* @internal prvUtfExternalTcamIsUsedGet function
* @endinternal
*
* @brief   Get external TCAM usage status
*
* @param[in] dev                      - SW device number
*
* @param[out] isUsed                   - (pointer to) external TCAM is used
*                                      GT_TRUE - external TCAM
*                                      GT_FALSE - internal TCAM
*
* @retval GT_OK                    -  Get revision of device OK
* @retval GT_BAD_PARAM             -  Invalid device id
* @retval GT_BAD_PTR               -  Null pointer
*/
GT_STATUS prvUtfExternalTcamIsUsedGet
(
    IN  GT_U8       dev,
    OUT GT_BOOL     *isUsed
);

/**
* @internal prvUtfIterationReduce function
* @endinternal
*
* @brief   As the GM hold low performance , we need to reduce large iterations on tables.
*         this function allow to limit the number of iterations on a table by setting
*         larger step to iterate the entries in the table.
* @param[in] dev                      - SW device number
* @param[in] tableSize                - number of entries in the table
* @param[in] maxIterations            - maximum iterations required
* @param[in] origStep                 - the step that would be used when special reduce is not needed.
*                                       the actual step to use (after reduce).
*/
GT_U32 prvUtfIterationReduce
(
    IN  GT_U8       dev,
    IN  GT_U32      tableSize,
    IN  GT_U32      maxIterations,
    IN  GT_U32      origStep
);

/**
* @internal prvTgfResetModeGet function
* @endinternal
*
* @brief   Get reset mode for enhanced UT's.
*/
GT_BOOL prvTgfResetModeGet
(
    GT_VOID
);

/**
* @internal prvTgfResetModeSet function
* @endinternal
*
* @brief   Set reset mode for enhanced UT's.
*         allow to temporary treat device as 'non-supporting'
* @param[in] mode                     - GT_TRUE  - state that system supports Reset.
*                                      GT_FALSE - state that system not supports Reset.
*                                       reset mode
*/
GT_VOID prvTgfResetModeSet
(
    GT_BOOL mode
);

/**
  @internal prvTgfRemoveAndInsertSystem(void)
 @endinternal

 @brief function for UT remove and insert all PacketProcessor

 @retval GT_OK               - on success
 @retval GT_NOT_SUPPORTED    - on system that not support it
  **/
GT_VOID prvTgfCpssPpRemoveAndInsert(void);

/**
* @internal prvTgfResetAndInitSystem function
* @endinternal
*
* @brief   Function for enhanced UT reset, includes cpssInitSystem.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_SUPPORTED         - on system that not support it
*/
GT_STATUS prvTgfResetAndInitSystem(void);

/**
* @internal prvTgfResetModeGet_gtShutdownAndCoreRestart function
* @endinternal
*
* @brief   Get reset mode for enhanced UT's, for gtShutdownAndCoreRestart()
*/
GT_BOOL prvTgfResetModeGet_gtShutdownAndCoreRestart
(
    GT_VOID
);
/**
* @internal prvTgfResetAndInitSystem_gtShutdownAndCoreRestart function
* @endinternal
*
* @brief   Function for enhanced UT reset, includes cpssInitSystem.
*         the function emulates the PSS function of gtShutdownAndCoreRestart()
*         with one 'little' restriction , this function NOT clean any HW config !
*
* @retval GT_OK                    - on success
* @retval GT_NOT_SUPPORTED         - on system that not support it
*/
GT_STATUS prvTgfResetAndInitSystem_gtShutdownAndCoreRestart(void);

/**
* @internal tgfStateStartTest function
* @endinternal
*
* @brief   state that test started
*
* @note needed for system reset feature
*
*/
GT_VOID tgfStateStartTest(GT_VOID);

/**
* @internal tgfIsAfterSystemReset function
* @endinternal
*
* @brief   get indication that we are After System Reset.
*
* @note needed for system reset feature
*
*/
GT_BOOL tgfIsAfterSystemReset(GT_VOID);

/**
* @internal prvUtfIsCpuPortMacSupported function
* @endinternal
*
* @brief   Function checks support of MAC layer existence for CPU Port.
*
* @param[in] dev                      -  device id
*
* @retval GT_TRUE                  - CPU Port has MAC and MAC related APIs may be called
* @retval GT_FALSE                 - CPU Port does not have MAC and MAC related APIs returns error code
*/
GT_BOOL prvUtfIsCpuPortMacSupported
(
    IN GT_U8                    dev
);

#ifdef IMPL_GALTIS
typedef GT_VOID (*wrCpssResetLibFuncPtr)(GT_VOID);

/**
* @internal wrCpssRegisterResetCb function
* @endinternal
*
* @brief   Register reset callback.
*
* @retval GT_OK                    - OK
* @retval GT_FULL                  - on full list
*/
GT_STATUS wrCpssRegisterResetCb
(
    wrCpssResetLibFuncPtr funcPtr
);
#endif /*IMPL_GALTIS*/

/**
* @internal prvUtfIsVplsModeUsed function
* @endinternal
*
* @brief   This routine returns GT_TRUE if VPLS mode used. GT_FALSE otherwise.
*
* @param[in] devNum                   - device number
*/
GT_BOOL prvUtfIsVplsModeUsed
(
    IN GT_U8 devNum
);

/**
* @internal utfFirstDevNumGet function
* @endinternal
*
* @brief   This routine returns the first devNum applicable.
*/
GT_U8   utfFirstDevNumGet(void);


/**
* @internal utfExtraTestEnded function
* @endinternal
*
* @brief   Function to state that the current test ended.
*/
void utfExtraTestEnded(void);


/**
* @internal prvUtfValidPortsGet function
* @endinternal
*
* @brief   get valid ports (according to portType) starting from 'start number'
*
* @param[in] dev                      - the device.
* @param[in] startPortNum             - the minimal port number (inclusive)
* @param[in] numOfPortsToGet          - number of ports to get (according to portType)
*
* @param[out] validPortsArr[]          - array of valid port.(according to portType)
*                                      in the case that there are not enough valid MAC ports.
*                                      the array filled with PRV_TGF_INVALID_VALUE_CNS
*                                      Returns :
*                                      None.
*                                      COMMENTS:
*                                      None.
*/
void prvUtfValidPortsGet(
    IN  GT_U8       dev ,
    IN UTF_GENERIC_PORT_ITERATOR_TYPE_E     portType,
    IN  GT_U32      startPortNum,
    OUT GT_U32      validPortsArr[],
    IN  GT_U32      numOfPortsToGet
);

/**
* @internal prvUtfIsDoublePhysicalPortsModeUsed function
* @endinternal
*
* @brief   This routine returns GT_TRUE if 'doublePhysicalPorts' mode used. GT_FALSE otherwise.
*/
GT_BOOL prvUtfIsDoublePhysicalPortsModeUsed(void);


/**
* @internal prvUtfIsNumberPhysicalPortsModeUsed function
* @endinternal
*
* @brief   This routine Return value of physicalPortsNumber  if
*          'numberPhysicalPorts' mode used. GT_FALSE otherwise.
*/
GT_U32 prvUtfIsNumberPhysicalPortsModeUsed(void);


/**
* @internal prvUtfSetE2PhyEqualValue function
* @endinternal
*
* @brief   set eport to point to it's physical port '1:1'
*
* @param[in] portIndex                - port index in array of ports
*                                       None
*/
void prvUtfSetE2PhyEqualValue(
    IN GT_U32 portIndex
);

/**
* @internal utfTcamPortGroupsBmpForCurrentPortGroupId function
* @endinternal
*
* @brief   Creates from port group id tcam port group bmp
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - portgroup ID
*/
GT_PORT_GROUPS_BMP utfTcamPortGroupsBmpForCurrentPortGroupId
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId
);

/**
* @internal prvUtfSetCurrentTestType function
* @endinternal
*
* @brief   Sets current test type for proper device selection with
*         prvUtfNextDeviceGet.
* @param[in] testType                 - test type.
*                                       None.
*/
GT_VOID prvUtfSetCurrentTestType
(
    IN UTF_TEST_TYPE_ENT testType
);

/**
* @internal prvUtfIsAc3xWithPipeSystem function
* @endinternal
*
* @brief   Is current system AC3X + PIPE
*
* @retval GT_TRUE                  - exists
* @retval GT_FALSE                 - not exists
*/
GT_BOOL prvUtfIsAc3xWithPipeSystem
(
    GT_VOID
);

extern void cpssInitSystemGet
(
    OUT GT_U32  *boardIdxPtr,
    OUT GT_U32  *boardRevIdPtr,
    OUT GT_U32  *reloadEepromPtr
);

/**
* @internal prvUtfOffsetFrom0ForUsedPortsGet function
* @endinternal
*
* @brief   the function return the 'offset from 0' for the used ports in
*         prvTgfPortsArray[].
*/
GT_U32 prvUtfOffsetFrom0ForUsedPortsGet(void);
/**
* @internal prvUtfOffsetFrom0ForUsedPortsSet function
* @endinternal
*
* @brief   Set the 'offset from 0' for the used ports in prvTgfPortsArray[].
*
* @param[in] offset                   - the offset
*                                       None.
*/
GT_VOID prvUtfOffsetFrom0ForUsedPortsSet(IN GT_U32   offset);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvUtfExtrash */

