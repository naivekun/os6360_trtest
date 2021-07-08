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
* @file tgfTrunkGen.c
*
* @brief Generic API implementation for Trunk
*
* @version   30
********************************************************************************
*/
#ifdef COW_INTERNAL_COMPILE
    /* those 3 are defined , so remove them */
    #undef EXMXPM_FAMILY
    #undef EX_FAMILY
    #undef CHX_FAMILY

    /* define only the tested one */
    #define CHX_FAMILY

#endif /*COW_INTERNAL_COMPILE*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/private/prvUtfHelpers.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficEngine.h>
#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfTrunkGen.h>


/******************************************************************************\
 *                           Private declarations                             *
\******************************************************************************/

/******************************************************************************\
 *                     Private function's implementation                      *
\******************************************************************************/

/******************************************************************************\
 *                       CPSS generic API section                             *
\******************************************************************************/
#ifdef IMPL_GALTIS
#if (defined CHX_FAMILY)
/* this function is implemented in the Galtis wrappers */
GT_STATUS cpssDxChTrunkDump
(
    IN    GT_U8 devNum
);
#endif/*CHX_FAMILY*/
#endif

#if (defined CHX_FAMILY)
/****************************************************************/
/****** trunk WA 'B' for phy1690 ********************************/
/****************************************************************/
extern GT_STATUS cpssDxChTrunkMemberRemove_phy1690_WA_B
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
);
extern GT_STATUS cpssDxChTrunkMemberAdd_phy1690_WA_B
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
);
extern GT_STATUS cpssDxChTrunkMembersSet_phy1690_WA_B
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN GT_U32                   numOfEnabledMembers,
    IN CPSS_TRUNK_MEMBER_STC    enabledMembersArray[],
    IN GT_U32                   numOfDisabledMembers,
    IN CPSS_TRUNK_MEMBER_STC    disabledMembersArray[]
);
extern GT_U32   appDemo_trunk_phy1690_WA_B;
/****************************************************************/
/****** trunk WA 'C' for phy1690 ********************************/
/****************************************************************/
extern GT_STATUS cpssDxChTrunkMemberRemove_phy1690_WA_C
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
);
extern GT_STATUS cpssDxChTrunkMemberAdd_phy1690_WA_C
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
);
extern GT_STATUS cpssDxChTrunkMembersSet_phy1690_WA_C
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN GT_U32                   numOfEnabledMembers,
    IN CPSS_TRUNK_MEMBER_STC    enabledMembersArray[],
    IN GT_U32                   numOfDisabledMembers,
    IN CPSS_TRUNK_MEMBER_STC    disabledMembersArray[]
);
extern GT_U32   appDemo_trunk_phy1690_WA_C;
/****************************************************************/

static GT_STATUS wr_cpssDxChTrunkMemberRemove
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
)
{
    if(appDemo_trunk_phy1690_WA_B)
    {
        return cpssDxChTrunkMemberRemove_phy1690_WA_B(devNum,trunkId,memberPtr);
    }
    else
    if(appDemo_trunk_phy1690_WA_C)
    {
        return cpssDxChTrunkMemberRemove_phy1690_WA_C(devNum,trunkId,memberPtr);
    }
    else
    {
        return cpssDxChTrunkMemberRemove(devNum,trunkId,memberPtr);
    }
}

static GT_STATUS wr_cpssDxChTrunkMemberAdd
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
)
{
    if(appDemo_trunk_phy1690_WA_B)
    {
        return cpssDxChTrunkMemberAdd_phy1690_WA_B(devNum,trunkId,memberPtr);
    }
    else
    if(appDemo_trunk_phy1690_WA_C)
    {
        return cpssDxChTrunkMemberAdd_phy1690_WA_C(devNum,trunkId,memberPtr);
    }
    else
    {
        return cpssDxChTrunkMemberAdd(devNum,trunkId,memberPtr);
    }
}

static GT_STATUS wr_cpssDxChTrunkMembersSet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN GT_U32                   numOfEnabledMembers,
    IN CPSS_TRUNK_MEMBER_STC    enabledMembersArray[],
    IN GT_U32                   numOfDisabledMembers,
    IN CPSS_TRUNK_MEMBER_STC    disabledMembersArray[]
)
{
    if(appDemo_trunk_phy1690_WA_B)
    {
        return cpssDxChTrunkMembersSet_phy1690_WA_B(devNum,trunkId,
            numOfEnabledMembers,enabledMembersArray,
            numOfDisabledMembers,disabledMembersArray);
    }
    else
    if(appDemo_trunk_phy1690_WA_C)
    {
        return cpssDxChTrunkMembersSet_phy1690_WA_C(devNum,trunkId,
            numOfEnabledMembers,enabledMembersArray,
            numOfDisabledMembers,disabledMembersArray);
    }
    else
    {
        return cpssDxChTrunkMembersSet(devNum,trunkId,
            numOfEnabledMembers,enabledMembersArray,
            numOfDisabledMembers,disabledMembersArray);
    }
}
#endif/*CHX_FAMILY*/


/**
* @internal prvTgfTrunkMembersSet function
* @endinternal
*
* @brief   This function set the trunk members
*
* @param[in] trunkId                  - trunk id
* @param[in] enabledMembersArray[]    - (array of) enabled members
* @param[in] numOfEnabledMembers      - number of enabled members in the array
* @param[in] disabledMembersArray[]   - (array of) disabled members
* @param[in] numOfDisabledMembers     - number of enabled members in the array
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
GT_STATUS prvTgfTrunkMembersSet
(
    IN GT_TRUNK_ID                    trunkId,
    IN GT_U32                         numOfEnabledMembers,
    IN CPSS_TRUNK_MEMBER_STC          enabledMembersArray[],
    IN GT_U32                         numOfDisabledMembers,
    IN CPSS_TRUNK_MEMBER_STC          disabledMembersArray[]
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
   GT_U8       devNum  = 0;
   GT_STATUS   rc, rc1 = GT_OK;
   GT_U32      i;

   /* prepare device iterator */
   PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

   /* go over all active devices */
   while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
   {
       for(i = 0 ; i < numOfEnabledMembers ; i++)
       {
            /* need to remove casting and to fix code after lion2 development is done */
            CPSS_TBD_BOOKMARK_EARCH

           rc = prvUtfHwDeviceNumberGet((GT_U8)enabledMembersArray[i].hwDevice,
                                        &enabledMembersArray[i].hwDevice);
           if (GT_OK != rc)
           {
               PRV_UTF_LOG1_MAC(
                   "[TGF]: prvUtfHwDeviceNumberGet FAILED, rc = [%d]", rc);

               return rc;
           }
       }

       for(i = 0 ; i < numOfDisabledMembers ; i++)
       {
            /* need to remove casting and to fix code after lion2 development is done */
            CPSS_TBD_BOOKMARK_EARCH

           rc = prvUtfHwDeviceNumberGet((GT_U8)disabledMembersArray[i].hwDevice,
                                        &disabledMembersArray[i].hwDevice);
           if (GT_OK != rc)
           {
               PRV_UTF_LOG1_MAC(
                   "[TGF]: prvUtfHwDeviceNumberGet FAILED, rc = [%d]", rc);

               return rc;
           }
       }

       /* call device specific API */
#ifdef EXMXPM_FAMILY
       rc = cpssExMxPmTrunkMembersSet(
#else
       rc = wr_cpssDxChTrunkMembersSet(
#endif
            devNum, trunkId, numOfEnabledMembers, enabledMembersArray,
            numOfDisabledMembers, disabledMembersArray);
       if (GT_OK != rc)
       {
#ifdef EXMXPM_FAMILY
           PRV_UTF_LOG1_MAC(
               "[TGF]: cpssExMxPmTrunkMembersSet FAILED, rc = [%d]", rc);
#else
           PRV_UTF_LOG1_MAC(
               "[TGF]: cpssDxChTrunkMembersSet FAILED, rc = [%d]", rc);
#endif

           rc1 = rc;
       }

       for(i = 0 ; i < numOfEnabledMembers ; i++)
       {
            /* need to remove casting and to fix code after lion2 development is done */
            CPSS_TBD_BOOKMARK_EARCH

           rc = prvUtfSwFromHwDeviceNumberGet(enabledMembersArray[i].hwDevice,
                                              &enabledMembersArray[i].hwDevice);
           if (GT_OK != rc)
           {
               PRV_UTF_LOG1_MAC(
                   "[TGF]: prvUtfSwFromHwDeviceNumberGet FAILED, rc = [%d]", rc);

               return rc;
           }
       }

       for(i = 0 ; i < numOfDisabledMembers ; i++)
       {
            /* need to remove casting and to fix code after lion2 development is done */
            CPSS_TBD_BOOKMARK_EARCH

           rc = prvUtfSwFromHwDeviceNumberGet(disabledMembersArray[i].hwDevice,
                                              &disabledMembersArray[i].hwDevice);
           if (GT_OK != rc)
           {
               PRV_UTF_LOG1_MAC(
                   "[TGF]: prvUtfSwFromHwDeviceNumberGet FAILED, rc = [%d]", rc);

               return rc;
           }
       }
   }

   return rc1;

#else
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTrunkHashGlobalModeSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set the general hashing mode of trunk hash generation.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
* @param[in] hashMode                 - hash mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or hash mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none
*
*/
GT_STATUS prvTgfTrunkHashGlobalModeSet
(
    IN PRV_TGF_TRUNK_LBH_GLOBAL_MODE_ENT  hashMode
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
   GT_U8       devNum  = 0;
   GT_STATUS   rc, rc1 = GT_OK;

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_TRUNK_LBH_GLOBAL_MODE_ENT  cpssHashMode;
#else
    CPSS_DXCH_TRUNK_LBH_GLOBAL_MODE_ENT  cpssHashMode;
#endif

    switch (hashMode)
    {
        case PRV_TGF_TRUNK_LBH_PACKETS_INFO_E:
#ifdef EXMXPM_FAMILY
            cpssHashMode = CPSS_EXMXPM_TRUNK_LBH_PACKETS_INFO_E;
#else
            cpssHashMode = CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_E;
#endif
            break;
        case PRV_TGF_TRUNK_LBH_INGRESS_PORT_E:
#ifdef EXMXPM_FAMILY
            cpssHashMode = CPSS_EXMXPM_TRUNK_LBH_INGRESS_PORT_E;
#else
            cpssHashMode = CPSS_DXCH_TRUNK_LBH_INGRESS_PORT_E;
#endif
            break;

#ifdef EXMXPM_FAMILY
        case PRV_TGF_TRUNK_LBH_PERFECT_E:
            cpssHashMode = CPSS_EXMXPM_TRUNK_LBH_PERFECT_E;
            break;
#endif

#ifndef EXMXPM_FAMILY
        case PRV_TGF_TRUNK_LBH_PACKETS_INFO_CRC_E:
            cpssHashMode = CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_CRC_E;
            break;
#endif
        default:
            return GT_BAD_PARAM;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
#ifdef EXMXPM_FAMILY
        rc = cpssExMxPmTrunkHashGlobalModeSet(devNum, cpssHashMode);
#else
        rc = cpssDxChTrunkHashGlobalModeSet(devNum, cpssHashMode);
#endif
        if (GT_OK != rc)
        {
#ifdef EXMXPM_FAMILY
            PRV_UTF_LOG1_MAC(
                "[TGF]: cpssExMxPmTrunkHashGlobalModeSet FAILED, rc = [%d]", rc);
#else
            PRV_UTF_LOG1_MAC(
                "[TGF]: cpssDxChTrunkHashGlobalModeSet FAILED, rc = [%d]", rc);
#endif

            rc1 = rc;
        }
    }

    return rc1;
#else
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTrunkHashGlobalModeGet function
* @endinternal
*
* @brief   Get the general hashing mode of trunk hash generation
*
* @param[in] devNum                   - the device number
*
* @param[out] hashModePtr              - (pointer to) hash mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or hash mode
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none
*
*/
GT_STATUS prvTgfTrunkHashGlobalModeGet
(
    IN  GT_U8                               devNum,
    OUT PRV_TGF_TRUNK_LBH_GLOBAL_MODE_ENT  *hashModePtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
   GT_STATUS   rc = GT_OK;

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_TRUNK_LBH_GLOBAL_MODE_ENT  cpssHashMode;
#else
    CPSS_DXCH_TRUNK_LBH_GLOBAL_MODE_ENT  cpssHashMode;
#endif

    /* call device specific API */
#ifdef EXMXPM_FAMILY
    rc = cpssExMxPmTrunkHashGlobalModeGet(devNum, &cpssHashMode);
#else
    rc = cpssDxChTrunkHashGlobalModeGet(devNum, &cpssHashMode);
#endif
    if (GT_OK != rc)
    {
#ifdef EXMXPM_FAMILY
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmTrunkHashGlobalModeSet FAILED, rc = [%d]", rc);
#else
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChTrunkHashGlobalModeSet FAILED, rc = [%d]", rc);
#endif
        return rc;
    }

    /* convert from device specific format */
    switch (cpssHashMode)
    {
#ifdef EXMXPM_FAMILY
        case CPSS_EXMXPM_TRUNK_LBH_PACKETS_INFO_E:
#else
        case CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_E:
#endif
            *hashModePtr = PRV_TGF_TRUNK_LBH_PACKETS_INFO_E;

            break;
#ifdef EXMXPM_FAMILY
        case CPSS_EXMXPM_TRUNK_LBH_INGRESS_PORT_E:
#else
        case CPSS_DXCH_TRUNK_LBH_INGRESS_PORT_E:
#endif
            *hashModePtr = PRV_TGF_TRUNK_LBH_INGRESS_PORT_E;
            break;
#ifdef EXMXPM_FAMILY
        case CPSS_EXMXPM_TRUNK_LBH_PERFECT_E:
            *hashModePtr = PRV_TGF_TRUNK_LBH_PERFECT_E;
            break;
#endif

#ifdef EXMXPM_FAMILY
#else
        case CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_CRC_E:
            *hashModePtr = PRV_TGF_TRUNK_LBH_PACKETS_INFO_CRC_E;
            break;
#endif
        default:
            return GT_BAD_PARAM;
    }

    return rc;

#else
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTrunkHashMplsModeEnableSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Enable/Disable the device from considering the MPLS information,
*         when calculating the trunk hashing index for a packet.
*         Note:
*         1. Not relevant to NON-MPLS packets.
*         2. Relevant when cpssExMxPmTrunkHashGlobalModeSet(devNum,
*         CPSS_EXMXPM_TRUNK_LBH_PACKETS_INFO_E)
* @param[in] enable                   - GT_FALSE - MPLS parameter are not used in trunk hash index
*                                      GT_TRUE  - The following function is added to the trunk load
*                                      balancing hash:
*                                      MPLSTrunkHash = (mpls_label0[5:0] & mpls_trunk_lbl0_mask) ^
*                                      (mpls_label1[5:0] & mpls_trunk_lbl1_mask) ^
*                                      (mpls_label2[5:0] & mpls_trunk_lbl2_mask)
*                                      NOTE:
*                                      If any of MPLS Labels 0:2 do not exist in the packet,
*                                      the default value 0x0 is used for TrunkHash calculation
*                                      instead.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTrunkHashMplsModeEnableSet
(
    IN GT_BOOL enable
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
   GT_U8       devNum  = 0;
   GT_STATUS   rc, rc1 = GT_OK;


   /* prepare device iterator */
   PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

   /* go over all active devices */
   while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
   {
       /* call device specific API */
#ifdef EXMXPM_FAMILY
       rc = cpssExMxPmTrunkHashMplsModeEnableSet(devNum, enable);
#else
       rc = cpssDxChTrunkHashMplsModeEnableSet(devNum, enable);
#endif
       if (GT_OK != rc)
       {
#ifdef EXMXPM_FAMILY
           PRV_UTF_LOG1_MAC(
               "[TGF]: cpssDxChTrunkHashMplsModeEnableSet FAILED, rc = [%d]", rc);
#else
           PRV_UTF_LOG1_MAC(
               "[TGF]: cpssDxChTrunkHashMplsModeEnableSet FAILED, rc = [%d]", rc);
#endif

           rc1 = rc;
       }
   }

   return rc1;
#else
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTrunkHashMaskSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set the masks for the various packet fields being used at the Trunk
*         hash calculations
* @param[in] maskedField              - field to apply the mask on
* @param[in] maskValue                - The mask value to be used (0..0x3F)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or maskedField
* @retval GT_OUT_OF_RANGE          - maskValue > 0x3F
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTrunkHashMaskSet
(
    IN PRV_TGF_TRUNK_LBH_MASK_ENT     maskedField,
    IN GT_U8                          maskValue
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
   GT_U8       devNum  = 0;
   GT_STATUS   rc, rc1 = GT_OK;
#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_TRUNK_LBH_MASK_ENT  maskField;
#else
    CPSS_DXCH_TRUNK_LBH_MASK_ENT    maskField;
#endif

    switch (maskedField)
    {
        case PRV_TGF_TRUNK_LBH_MASK_MAC_DA_E:
#ifdef EXMXPM_FAMILY
            maskField = CPSS_EXMXPM_TRUNK_LBH_MASK_MAC_DA_E;
#else
            maskField = CPSS_DXCH_TRUNK_LBH_MASK_MAC_DA_E;
#endif
            break;
        case PRV_TGF_TRUNK_LBH_MASK_MAC_SA_E:
#ifdef EXMXPM_FAMILY
            maskField = CPSS_EXMXPM_TRUNK_LBH_MASK_MAC_SA_E;
#else
            maskField = CPSS_DXCH_TRUNK_LBH_MASK_MAC_SA_E;
#endif
            break;
        case PRV_TGF_TRUNK_LBH_MASK_MPLS_LABEL0_E:
#ifdef EXMXPM_FAMILY
            maskField = CPSS_EXMXPM_TRUNK_LBH_MASK_MPLS_LABEL0_E;
#else
            maskField = CPSS_DXCH_TRUNK_LBH_MASK_MPLS_LABEL0_E;
#endif
            break;
        case PRV_TGF_TRUNK_LBH_MASK_MPLS_LABEL1_E:
#ifdef EXMXPM_FAMILY
            maskField = CPSS_EXMXPM_TRUNK_LBH_MASK_MPLS_LABEL1_E;
#else
            maskField = CPSS_DXCH_TRUNK_LBH_MASK_MPLS_LABEL1_E;
#endif
            break;
        case PRV_TGF_TRUNK_LBH_MASK_MPLS_LABEL2_E:
#ifdef EXMXPM_FAMILY
            maskField = CPSS_EXMXPM_TRUNK_LBH_MASK_MPLS_LABEL2_E;
#else
            maskField = CPSS_DXCH_TRUNK_LBH_MASK_MPLS_LABEL2_E;
#endif
            break;
        case PRV_TGF_TRUNK_LBH_MASK_IPV4_DIP_E:
#ifdef EXMXPM_FAMILY
            maskField = CPSS_EXMXPM_TRUNK_LBH_MASK_IPV4_DIP_E;
#else
            maskField = CPSS_DXCH_TRUNK_LBH_MASK_IPV4_DIP_E;
#endif
            break;
        case PRV_TGF_TRUNK_LBH_MASK_IPV4_SIP_E:
#ifdef EXMXPM_FAMILY
            maskField = CPSS_EXMXPM_TRUNK_LBH_MASK_IPV4_SIP_E;
#else
            maskField = CPSS_DXCH_TRUNK_LBH_MASK_IPV4_SIP_E;
#endif
            break;
        case PRV_TGF_TRUNK_LBH_MASK_IPV6_DIP_E:
#ifdef EXMXPM_FAMILY
            maskField = CPSS_EXMXPM_TRUNK_LBH_MASK_IPV6_DIP_E;
#else
            maskField = CPSS_DXCH_TRUNK_LBH_MASK_IPV6_DIP_E;
#endif
            break;
        case PRV_TGF_TRUNK_LBH_MASK_IPV6_SIP_E:
#ifdef EXMXPM_FAMILY
            maskField = CPSS_EXMXPM_TRUNK_LBH_MASK_IPV6_SIP_E;
#else
            maskField = CPSS_DXCH_TRUNK_LBH_MASK_IPV6_SIP_E;
#endif
            break;
        case PRV_TGF_TRUNK_LBH_MASK_IPV6_FLOW_E:
#ifdef EXMXPM_FAMILY
            maskField = CPSS_EXMXPM_TRUNK_LBH_MASK_IPV6_FLOW_E;
#else
            maskField = CPSS_DXCH_TRUNK_LBH_MASK_IPV6_FLOW_E;
#endif
            break;
        case PRV_TGF_TRUNK_LBH_MASK_L4_DST_PORT_E:
#ifdef EXMXPM_FAMILY
            maskField = CPSS_EXMXPM_TRUNK_LBH_MASK_L4_DST_PORT_E;
#else
            maskField = CPSS_DXCH_TRUNK_LBH_MASK_L4_DST_PORT_E;
#endif
            break;
        case PRV_TGF_TRUNK_LBH_MASK_L4_SRC_PORT_E:
#ifdef EXMXPM_FAMILY
            maskField = CPSS_EXMXPM_TRUNK_LBH_MASK_L4_SRC_PORT_E;
#else
            maskField = CPSS_DXCH_TRUNK_LBH_MASK_L4_SRC_PORT_E;
#endif
            break;
        default: return GT_BAD_PARAM;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
#ifdef EXMXPM_FAMILY
        rc = cpssExMxPmTrunkHashMaskSet(devNum, maskField, maskValue);
#else
        rc = cpssDxChTrunkHashMaskSet(devNum, maskField, maskValue);
#endif
        if (GT_OK != rc)
        {
#ifdef EXMXPM_FAMILY
            PRV_UTF_LOG1_MAC(
                "[TGF]: cpssExMxPmTrunkHashMaskSet FAILED, rc = [%d]", rc);
#else
            PRV_UTF_LOG1_MAC(
                "[TGF]: cpssDxChTrunkHashMaskSet FAILED, rc = [%d]", rc);
#endif

            rc1 = rc;
        }
    }

    return rc1;

#else
    return GT_BAD_STATE;
#endif
}


/**
* @internal prvTgfTrunkHashIpShiftSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set the shift being done to IP addresses prior to hash calculations.
* @param[in] protocolStack            - Set the shift to either IPv4 or IPv6 IP addresses.
* @param[in] isSrcIp                  - GT_TRUE  = Set the shift to IPv4/6 source addresses.
*                                      GT_FALSE = Set the shift to IPv4/6 destination addresses.
* @param[in] shiftValue               - The shift to be done.
*                                      IPv4 valid shift: 0-3 bytes (Value = 0: no shift).
*                                      IPv6 valid shift: 0-15 bytes (Value = 0: no shift).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or protocolStack
* @retval GT_OUT_OF_RANGE          - shiftValue > 3 for IPv4 , shiftValue > 15 for IPv6
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTrunkHashIpShiftSet
(
    IN CPSS_IP_PROTOCOL_STACK_ENT   protocolStack,
    IN GT_BOOL                      isSrcIp,
    IN GT_U32                       shiftValue
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
   GT_U8       devNum  = 0;
   GT_STATUS   rc, rc1 = GT_OK;

   /* prepare device iterator */
   PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

   /* go over all active devices */
   while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
   {
       /* call device specific API */
#ifdef EXMXPM_FAMILY
       rc = cpssExMxPmTrunkHashIpShiftSet(
#else
       rc = cpssDxChTrunkHashIpShiftSet(
#endif
           devNum, protocolStack, isSrcIp, shiftValue);
       if (GT_OK != rc)
       {
#ifdef EXMXPM_FAMILY
           PRV_UTF_LOG1_MAC(
               "[TGF]: cpssExMxPmTrunkHashIpShiftSet FAILED, rc = [%d]", rc);
#else
           PRV_UTF_LOG1_MAC(
               "[TGF]: cpssDxChTrunkHashIpShiftSet FAILED, rc = [%d]", rc);
#endif

           rc1 = rc;
       }
   }

   return rc1;

#else
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTrunkHashIpModeSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Enable/Disable the device from considering the IP SIP/DIP information,
*         when calculation the trunk hashing index for a packet.
*         Relevant when the is IPv4 or IPv6 and <TrunkHash Mode> = 0.
*         Note:
*         1. Not relevant to NON Ip packets.
*         2. Not relevant to multi-destination packets (include routed IPM).
*         3. Not relevant when cpssDxChTrunkHashGlobalModeSet(devNum,
*         CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E)
* @param[in] enable                   - GT_FALSE - IP data is not added to the trunk load balancing
*                                      hash.
*                                      GT_TRUE - The following function is added to the trunk load
*                                      balancing hash, if the packet is IPv6.
*                                      IPTrunkHash = according to setting of API
*                                      cpssDxChTrunkHashIpv6ModeSet(...)
*                                      else packet is IPv4:
*                                      IPTrunkHash = SIP[5:0]^DIP[5:0]^SIP[21:16]^DIP[21:16].
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTrunkHashIpModeSet
(
    IN GT_BOOL enable
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
   GT_U8       devNum  = 0;
   GT_STATUS   rc, rc1 = GT_OK;

   /* prepare device iterator */
   PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

   /* go over all active devices */
   while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
   {
       /* call device specific API */
#ifdef EXMXPM_FAMILY
       rc = cpssExMxPmTrunkHashIpModeEnableSet(devNum, enable);
#else
       rc = cpssDxChTrunkHashIpModeSet(devNum, enable);
#endif
       if (GT_OK != rc)
       {
#ifdef EXMXPM_FAMILY
           PRV_UTF_LOG1_MAC(
               "[TGF]: cpssExMxPmTrunkHashIpModeEnableSet FAILED, rc = [%d]", rc);
#else
           PRV_UTF_LOG1_MAC(
               "[TGF]: cpssDxChTrunkHashIpModeSet FAILED, rc = [%d]", rc);
#endif

           rc1 = rc;
       }
   }

   return rc1;

#else
    return GT_BAD_STATE;
#endif
}


/**
* @internal prvTgfTrunkDesignatedMemberSet function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function Configures per-trunk the designated member -
*         value should be stored (to DB) even designated member is not currently
*         a member of Trunk.
*         Setting value replace previously assigned designated member.
*         NOTE that:
*         under normal operation this should not be used with cascade Trunks,
*         due to the fact that in this case it affects all types of traffic -
*         not just Multi-destination as in regular Trunks.
*         Diagram 1 : Regular operation - Traffic distribution on all enabled
*         members (when no specific designated member defined)
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         index \ member %  M1 %  M2  %  M3 % M4 %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 0    %  1  %  0  %  0  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 1    %  0  %  1  %  0  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 2    %  0  %  0  %  1  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 3    %  0  %  0  %  0  % 1  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 4    %  1  %  0  %  0  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 5    %  0  %  1  %  0  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 6    %  0  %  0  %  1  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 7    %  0  %  0  %  0  % 1  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Diagram 2: Traffic distribution once specific designated member defined
*         (M3 in this case - which is currently enabled member in trunk)
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         index \ member %  M1 %  M2  %  M3 % M4 %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 0    %  0  %  0  %  1  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 1    %  0  %  0  %  1  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 2    %  0  %  0  %  1  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 3    %  0  %  0  %  1  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 4    %  0  %  0  %  1  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 5    %  0  %  0  %  1  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 6    %  0  %  0  %  1  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 7    %  0  %  0  %  1  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number , or
*                                       bad member parameters :
*                                       (device & 0xE0) != 0 means that the HW can't support
*                                       this value , since HW has 5 bit
*                                       for device number
*                                       (port & 0xC0) != 0 means that the HW can't support
*                                       this value , since HW has 6 bit
*                                       for port number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_ALREADY_EXIST         - this member already exists in another trunk.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTrunkDesignatedMemberSet
(
    IN GT_TRUNK_ID              trunkId,
    IN GT_BOOL                  enable,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
)
{
#if (defined CHX_FAMILY)
   GT_U8       devNum  = 0;
   GT_STATUS   rc, rc1 = GT_OK;
   CPSS_TRUNK_MEMBER_STC localMember, *localMemberPtr;

   /* prepare device iterator */
   PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

   if( NULL != memberPtr)
   {
       localMember = *memberPtr;

        /* need to remove casting and to fix code after lion2 development is done */
        CPSS_TBD_BOOKMARK_EARCH

       rc = prvUtfHwDeviceNumberGet((GT_U8)(memberPtr->hwDevice), &localMember.hwDevice);
       if (GT_OK != rc)
       {
           PRV_UTF_LOG1_MAC(
               "[TGF]: prvUtfHwDeviceNumberGet FAILED, rc = [%d]", rc);

           return rc;
       }
       localMemberPtr = &localMember;
   }
   else
       localMemberPtr = NULL;

   /* go over all active devices */
   while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
   {
       /* call device specific API */
       rc = cpssDxChTrunkDesignatedMemberSet(devNum, trunkId,enable, localMemberPtr);
       if (GT_OK != rc)
       {
           PRV_UTF_LOG1_MAC(
               "[TGF]: cpssDxChTrunkDesignatedMemberSet FAILED, rc = [%d]", rc);
           rc1 = rc;
       }
   }

   return rc1;

#else
    TGF_PARAM_NOT_USED(trunkId);
    TGF_PARAM_NOT_USED(enable);
    TGF_PARAM_NOT_USED(memberPtr);

    return GT_BAD_STATE;
#endif

}

/**
* @internal prvTgfTrunkDbDesignatedMemberGet function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function get Configuration per-trunk the designated member -
*         value should be stored (to DB) even designated member is not currently
*         a member of Trunk.
*         function uses the DB (no HW operations)
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
* @param[in] devNum                   - the device number
* @param[in] trunkId                  - the trunk id.
*
* @param[out] enablePtr                - (pointer to) enable/disable designated trunk member.
*                                      GT_TRUE -
*                                      1. Clears all current Trunk member's designated bits
*                                      2. If input designated member, is currently an
*                                      enabled-member at Trunk (in local device) enable its
*                                      bits on all indexes
*                                      3. Store designated member at the DB (new DB parameter
*                                      should be created for that)
*                                      GT_FALSE -
*                                      1. Redistribute current Trunk members bits (on all enabled members)
*                                      2. Clear designated member at  the DB
* @param[out] memberPtr                - (pointer to) the designated member of the trunk.
*                                      relevant only when enable = GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTrunkDbDesignatedMemberGet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    OUT GT_BOOL                 *enablePtr,
    OUT CPSS_TRUNK_MEMBER_STC   *memberPtr
)
{
#if (defined CHX_FAMILY)
   GT_STATUS   rc;

   /* call device specific API */
   rc = cpssDxChTrunkDbDesignatedMemberGet(devNum, trunkId,enablePtr,memberPtr);
   if (GT_OK != rc)
   {
       PRV_UTF_LOG1_MAC(
           "[TGF]: cpssDxChTrunkDbDesignatedMemberGet FAILED, rc = [%d]", rc);

       return rc;
   }

   if( NULL != memberPtr)
   {
        /* need to remove casting and to fix code after lion2 development is done */
        CPSS_TBD_BOOKMARK_EARCH

       rc = prvUtfSwFromHwDeviceNumberGet(memberPtr->hwDevice, &memberPtr->hwDevice);
       if (GT_OK != rc)
       {
           PRV_UTF_LOG1_MAC(
               "[TGF]: prvUtfSwFromHwDeviceNumberGet FAILED, rc = [%d]", rc);

       }
   }

   return rc;

#else
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(trunkId);
    TGF_PARAM_NOT_USED(enablePtr);
    TGF_PARAM_NOT_USED(memberPtr);

    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTrunkDbIsMemberOfTrunk function
* @endinternal
*
* @brief   Function Relevant mode : High level mode
*         Checks if a member (device,port) is a trunk member.
*         if it is trunk member the function retrieve the trunkId of the trunk.
*         function uses the DB (no HW operations)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
* @param[in] memberPtr                - (pointer to) the member to check if is trunk member
*
* @param[out] trunkIdPtr               - (pointer to) trunk id of the port .
*                                      this pointer allocated by the caller.
*                                      this can be NULL pointer if the caller not require the
*                                      trunkId(only wanted to know that the member belongs to a
*                                      trunk)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_NOT_FOUND             - the pair (devNum,port) not a trunk member
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTrunkDbIsMemberOfTrunk
(
    IN  GT_U8                   devNum,
    IN  CPSS_TRUNK_MEMBER_STC   *memberPtr,
    OUT GT_TRUNK_ID             *trunkIdPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
   GT_STATUS   rc;
   CPSS_TRUNK_MEMBER_STC localMember, *localMemberPtr;

   if( NULL != memberPtr)
   {
       localMember = *memberPtr;
       rc = prvUtfHwDeviceNumberGet((GT_U8)memberPtr->hwDevice, &(localMember.hwDevice));
       if (GT_OK != rc)
       {
           PRV_UTF_LOG1_MAC(
               "[TGF]: prvUtfHwDeviceNumberGet FAILED, rc = [%d]", rc);

           return rc;
       }
       localMemberPtr = &localMember;
   }
   else
       localMemberPtr = NULL;

   /* call device specific API */
#ifdef EXMXPM_FAMILY
    rc = cpssExMxPmTrunkDbIsMemberOfTrunk(devNum, localMemberPtr , trunkIdPtr);
    #else
    rc = cpssDxChTrunkDbIsMemberOfTrunk(devNum, localMemberPtr , trunkIdPtr);
#endif
    if (GT_OK != rc && rc != GT_NOT_FOUND)
    {
#ifdef EXMXPM_FAMILY
       PRV_UTF_LOG1_MAC(
           "[TGF]: cpssExMxPmTrunkDbIsMemberOfTrunk FAILED, rc = [%d]", rc);
#else
       PRV_UTF_LOG1_MAC(
           "[TGF]: cpssDxChTrunkDbIsMemberOfTrunk FAILED, rc = [%d]", rc);
#endif
    }

    return rc;

#else
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(trunkId);
    TGF_PARAM_NOT_USED(enablePtr);
    TGF_PARAM_NOT_USED(memberPtr);

    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTrunkDesignatedPortsEntryGet function
* @endinternal
*
* @brief   Function Relevant mode : All modes
*         Get the designated trunk table specific entry.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
* @param[in] devNum                   - the device number
* @param[in] entryIndex               - the index in the designated ports bitmap table
*
* @param[out] designatedPortsPtr       - (pointer to) designated ports bitmap
*
* @retval GT_OK                    - successful completion
* @retval GT_FAIL                  - an error occurred.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_OUT_OF_RANGE          - entryIndex exceed the number of HW table.
* @retval the index must be in range (0 - 7)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTrunkDesignatedPortsEntryGet
(
    IN  GT_U8               devNum,
    IN  GT_U32              entryIndex,
    OUT  CPSS_PORTS_BMP_STC  *designatedPortsPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
   GT_STATUS   rc;

   /* call device specific API */
#ifdef EXMXPM_FAMILY
   rc = cpssExMxPmTrunkDesignatedPortsEntryGet(devNum, entryIndex , designatedPortsPtr);
#else
   rc = cpssDxChTrunkDesignatedPortsEntryGet(devNum, entryIndex , designatedPortsPtr);
#endif
   if (GT_OK != rc)
   {
#ifdef EXMXPM_FAMILY
       PRV_UTF_LOG1_MAC(
           "[TGF]: cpssExMxPmTrunkDesignatedPortsEntryGet FAILED, rc = [%d]", rc);
#else
       PRV_UTF_LOG1_MAC(
           "[TGF]: cpssDxChTrunkDesignatedPortsEntryGet FAILED, rc = [%d]", rc);
#endif
   }

   return rc;

#else
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(entryIndex);
    TGF_PARAM_NOT_USED(designatedPortsPtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTrunkMemberAdd function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function add member to the trunk in the device.
*         If member is already in this trunk , function do nothing and
*         return GT_OK.
*         Notes about designated trunk table:
*         If (no designated defined)
*         re-distribute MC/Cascade trunk traffic among the enabled members,
*         now taking into account also the added member
*         else
*         1. If added member is not the designated member - set its relevant bits to 0
*         2. If added member is the designated member & it's enabled,
*         set its relevant bits on all indexes to 1.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
* @param[in] trunkId                  - the trunk id.
* @param[in] memberPtr                - (pointer to)the member to add to the trunk.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number , or
*                                       bad member parameters :
*                                       (device & 0xE0) != 0 means that the HW can't support
*                                       this value , since HW has 5 bit
*                                       for device number
*                                       (port & 0xC0) != 0 means that the HW can't support
*                                       this value , since HW has 6 bit
*                                       for port number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_ALREADY_EXIST         - this member already exists in another trunk.
* @retval GT_FULL                  - trunk already contains maximum supported members
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTrunkMemberAdd
(
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
   GT_U8       devNum  = 0;
   GT_STATUS   rc, rc1 = GT_OK;
   CPSS_TRUNK_MEMBER_STC localMember, *localMemberPtr;

   /* prepare device iterator */
   PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

   if( NULL != memberPtr)
   {
       localMember = *memberPtr;

        /* need to remove casting and to fix code after lion2 development is done */
        CPSS_TBD_BOOKMARK_EARCH

       rc = prvUtfHwDeviceNumberGet((GT_U8)memberPtr->hwDevice, &localMember.hwDevice);
       if (GT_OK != rc)
       {
           PRV_UTF_LOG1_MAC(
               "[TGF]: prvUtfHwDeviceNumberGet FAILED, rc = [%d]", rc);

           return rc;
       }
       localMemberPtr = &localMember;
   }
   else
       return GT_BAD_PTR;

   /* go over all active devices */
   while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
   {
       /* call device specific API */
#ifdef EXMXPM_FAMILY
       rc = cpssExMxPmTrunkMemberAdd(devNum, trunkId, localMemberPtr);
#else
       rc = wr_cpssDxChTrunkMemberAdd(devNum, trunkId, localMemberPtr);
#endif
       if (GT_OK != rc)
       {
#ifdef EXMXPM_FAMILY
           PRV_UTF_LOG1_MAC(
               "[TGF]: cpssExMxPmTrunkMemberAdd FAILED, rc = [%d]", rc);
#else
           PRV_UTF_LOG1_MAC(
               "[TGF]: cpssDxChTrunkDesignatedMemberSet FAILED, rc = [%d]", rc);
#endif
           rc1 = rc;
       }
   }

   return rc1;

#else
    TGF_PARAM_NOT_USED(trunkId);
    TGF_PARAM_NOT_USED(memberPtr);

    return GT_BAD_STATE;
#endif

}

/**
* @internal prvTgfTrunkMemberRemove function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function remove member from a trunk in the device.
*         If member not exists in this trunk , function do nothing and
*         return GT_OK.
*         Notes about designated trunk table:
*         If (no designated defined)
*         re-distribute MC/Cascade trunk traffic among the enabled members -
*         now taking into account also the removed member
*         else
*         1. If removed member is not the designated member - nothing to do
*         2. If removed member is the designated member set its relevant bits
*         on all indexes to 1.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
* @param[in] trunkId                  - the trunk id.
* @param[in] memberPtr                - (pointer to)the member to remove from the trunk.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number , or
*                                       bad member parameters :
*                                       (device & 0xE0) != 0 means that the HW can't support
*                                       this value , since HW has 5 bit
*                                       for device number
*                                       (port & 0xC0) != 0 means that the HW can't support
*                                       this value , since HW has 6 bit
*                                       for port number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTrunkMemberRemove
(
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
   GT_U8       devNum  = 0;
   GT_STATUS   rc, rc1 = GT_OK;
   CPSS_TRUNK_MEMBER_STC localMember, *localMemberPtr;

   /* prepare device iterator */
   PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

   if( NULL != memberPtr)
   {
       localMember = *memberPtr;

       /* need to remove casting and to fix code after lion2 development is done */
        CPSS_TBD_BOOKMARK_EARCH

       rc = prvUtfHwDeviceNumberGet((GT_U8)memberPtr->hwDevice, &localMember.hwDevice);
       if (GT_OK != rc)
       {
           PRV_UTF_LOG1_MAC(
               "[TGF]: prvUtfHwDeviceNumberGet FAILED, rc = [%d]", rc);

           return rc;
       }
       localMemberPtr = &localMember;
   }
   else
       return GT_BAD_PTR;

   /* go over all active devices */
   while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
   {
       /* call device specific API */
#ifdef EXMXPM_FAMILY
       rc = cpssExMxPmTrunkMemberRemove(devNum, trunkId, localMemberPtr);
#else
       rc = wr_cpssDxChTrunkMemberRemove(devNum, trunkId, localMemberPtr);
#endif
       if (GT_OK != rc)
       {
#ifdef EXMXPM_FAMILY
           PRV_UTF_LOG1_MAC(
               "[TGF]: cpssExMxPmTrunkMemberRemove FAILED, rc = [%d]", rc);
#else
           PRV_UTF_LOG1_MAC(
               "[TGF]: cpssDxChTrunkMemberRemove FAILED, rc = [%d]", rc);
#endif
           rc1 = rc;
       }
   }

   return rc1;

#else
    TGF_PARAM_NOT_USED(trunkId);
    TGF_PARAM_NOT_USED(memberPtr);

    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTrunkMemberDisable function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function disable (enabled)existing member of trunk in the device.
*         If member is already disabled in this trunk , function do nothing and
*         return GT_OK.
*         Notes about designated trunk table:
*         If (no designated defined)
*         re-distribute MC/Cascade trunk traffic among the enabled members -
*         now taking into account also the disabled member
*         else
*         1. If disabled member is not the designated member - set its relevant bits to 0
*         2. If disabled member is the designated member set its relevant bits
*         on all indexes to 0.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
* @param[in] trunkId                  - the trunk id.
* @param[in] memberPtr                - (pointer to)the member to disable in the trunk.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number , or
*                                       bad member parameters :
*                                       (device & 0xE0) != 0 means that the HW can't support
*                                       this value , since HW has 5 bit
*                                       for device number
*                                       (port & 0xC0) != 0 means that the HW can't support
*                                       this value , since HW has 6 bit
*                                       for port number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_FOUND             - this member not found (member not exist) in the trunk
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTrunkMemberDisable
(
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
   GT_U8       devNum  = 0;
   GT_STATUS   rc, rc1 = GT_OK;
   CPSS_TRUNK_MEMBER_STC localMember, *localMemberPtr;

   /* prepare device iterator */
   PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

   if( NULL != memberPtr)
   {
       localMember = *memberPtr;

        /* need to remove casting and to fix code after lion2 development is done */
        CPSS_TBD_BOOKMARK_EARCH

       rc = prvUtfHwDeviceNumberGet((GT_U8)memberPtr->hwDevice, &localMember.hwDevice);
       if (GT_OK != rc)
       {
           PRV_UTF_LOG1_MAC(
               "[TGF]: prvUtfHwDeviceNumberGet FAILED, rc = [%d]", rc);

           return rc;
       }
       localMemberPtr = &localMember;
   }
   else
       localMemberPtr = NULL;

   /* go over all active devices */
   while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
   {
       /* call device specific API */
#ifdef EXMXPM_FAMILY
       rc = cpssExMxPmTrunkMemberDisable(devNum, trunkId, localMemberPtr);
#else
       rc = cpssDxChTrunkMemberDisable(devNum, trunkId, localMemberPtr);
#endif
       if (GT_OK != rc)
       {
#ifdef EXMXPM_FAMILY
           PRV_UTF_LOG1_MAC(
               "[TGF]: cpssExMxPmTrunkMemberDisable FAILED, rc = [%d]", rc);
#else
           PRV_UTF_LOG1_MAC(
               "[TGF]: cpssDxChTrunkMemberDisable FAILED, rc = [%d]", rc);
#endif
           rc1 = rc;
       }
   }

   return rc1;

#else
    TGF_PARAM_NOT_USED(trunkId);
    TGF_PARAM_NOT_USED(memberPtr);

    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTrunkMemberEnable function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function enable (disabled)existing member of trunk in the device.
*         If member is already enabled in this trunk , function do nothing and
*         return GT_OK.
*         Notes about designated trunk table:
*         If (no designated defined)
*         re-distribute MC/Cascade trunk traffic among the enabled members -
*         now taking into account also the enabled member
*         else
*         1. If enabled member is not the designated member - set its relevant bits to 0
*         2. If enabled member is the designated member set its relevant bits
*         on all indexes to 1.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
* @param[in] trunkId                  - the trunk id.
* @param[in] memberPtr                - (pointer to)the member to enable in the trunk.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number , or
*                                       bad member parameters :
*                                       (device & 0xE0) != 0 means that the HW can't support
*                                       this value , since HW has 5 bit
*                                       for device number
*                                       (port & 0xC0) != 0 means that the HW can't support
*                                       this value , since HW has 6 bit
*                                       for port number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_FOUND             - this member not found (member not exist) in the trunk
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTrunkMemberEnable
(
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
   GT_U8       devNum  = 0;
   GT_STATUS   rc, rc1 = GT_OK;
   CPSS_TRUNK_MEMBER_STC localMember, *localMemberPtr;

   /* prepare device iterator */
   PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

   if( NULL != memberPtr)
   {
       localMember = *memberPtr;

        /* need to remove casting and to fix code after lion2 development is done */
        CPSS_TBD_BOOKMARK_EARCH

       rc = prvUtfHwDeviceNumberGet((GT_U8)memberPtr->hwDevice, &localMember.hwDevice);
       if (GT_OK != rc)
       {
           PRV_UTF_LOG1_MAC(
               "[TGF]: prvUtfHwDeviceNumberGet FAILED, rc = [%d]", rc);

           return rc;
       }
       localMemberPtr = &localMember;
   }
   else
       localMemberPtr = NULL;

   /* go over all active devices */
   while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
   {
       /* call device specific API */
#ifdef EXMXPM_FAMILY
       rc = cpssExMxPmTrunkMemberEnable(devNum, trunkId, localMemberPtr);
#else
       rc = cpssDxChTrunkMemberEnable(devNum, trunkId, localMemberPtr);
#endif
       if (GT_OK != rc)
       {
#ifdef EXMXPM_FAMILY
           PRV_UTF_LOG1_MAC(
               "[TGF]: cpssExMxPmTrunkMemberEnable FAILED, rc = [%d]", rc);
#else
           PRV_UTF_LOG1_MAC(
               "[TGF]: cpssDxChTrunkMemberEnable FAILED, rc = [%d]", rc);
#endif
           rc1 = rc;
       }
   }

   return rc1;

#else
    TGF_PARAM_NOT_USED(trunkId);
    TGF_PARAM_NOT_USED(memberPtr);

    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTrunkCascadeTrunkPortsSet function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function sets the 'cascade' trunk with the specified 'Local ports'
*         overriding any previous setting.
*         The cascade trunk may be invalidated/unset by portsMembersPtr = NULL.
*         Local ports are ports of only configured device.
*         Cascade trunk is:
*         - members are ports of only configured device pointed by devNum
*         - trunk members table is empty (see cpssDxChTrunkTableEntrySet)
*         Therefore it cannot be used as target by ingress engines like FDB,
*         Router, TTI, Ingress PCL and so on.
*         - members ports trunk ID is not set (see cpssDxChTrunkPortTrunkIdSet).
*         Therefore packets those ingresses in member ports are not associated with trunk
*         - all members are enabled only and cannot be disabled.
*         - may be used for cascade traffic and pointed by the 'Device map table'
*         as the local target to reach to the 'Remote device'.
*         (For 'Device map table' refer to cpssDxChCscdDevMapTableSet(...))
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
* @param[in] devNum                   - device number
* @param[in] trunkId                  - trunk id
* @param[in] portsMembersPtr          - (pointer to) local ports bitmap to be members of the
*                                      cascade trunk.
*                                      NULL - meaning that the trunk-id is 'invalidated' and
*                                      trunk-type will be changed to : CPSS_TRUNK_TYPE_FREE_E
*                                      not-NULL - meaning that the trunk-type will be : CPSS_TRUNK_TYPE_CASCADE_E
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_INITIALIZED       - the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - there are ports in the bitmap that not supported by
*                                       the device.
* @retval GT_BAD_PARAM             - bad device number , or bad trunkId number , or number
*                                       of ports (in the bitmap) larger then the number of
*                                       entries in the 'Designated trunk table'
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_CASCADE_E
* @retval GT_ALREADY_EXIST         - one of the members already exists in another trunk ,
*                                       or this trunk hold members defined using cpssDxChTrunkMembersSet(...)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. This function does not set ports are 'Cascade ports' (and also not
*       check that ports are 'cascade').
*       2. This function sets only next tables :
*       a. the designated trunk table:
*       distribute MC/Cascade trunk traffic among the members
*       b. the 'Non-trunk' table entry.
*       3. because this function not set the 'Trunk members' table entry , the
*       application should not point to this trunk from any ingress unit , like:
*       FDB , PCL action redirect , NH , TTI action redirect , PVE ...
*       (it should be pointed ONLY from the device map table)
*       4. Application can manipulate the 'Per port' trunk-id , for those ports ,
*       using the 'Low level API' of : cpssDxChTrunkPortTrunkIdSet(...)
*       5. this API supports only trunks with types : CPSS_TRUNK_TYPE_FREE_E or
*       CPSS_TRUNK_TYPE_CASCADE_E.
*       6. next APIs are not supported from trunk with type : CPSS_TRUNK_TYPE_CASCADE_E
*       cpssDxChTrunkMembersSet ,
*       cpssDxChTrunkMemberAdd , cpssDxChTrunkMemberRemove,
*       cpssDxChTrunkMemberEnable , cpssDxChTrunkMemberDisable
*       cpssDxChTrunkDbEnabledMembersGet , cpssDxChTrunkDbDisabledMembersGet
*       cpssDxChTrunkDesignatedMemberSet , cpssDxChTrunkDbDesignatedMemberGet
*       Comparing the 2 function :
*       cpssDxChTrunkCascadeTrunkPortsSet  |  cpssDxChTrunkMembersSet
*       ----------------------------------------------------------------------------
*       1. purpose 'Cascade trunk'        | 1. purpose 'Network trunk' , and
*       |  also 'Cascade trunk' with up to
*       |  8 members
*       ----------------------------------------------------------------------------
*       2. supported number of members depends  | 2. supports up to 8 members
*       on number of entries in the      |  (also in Lion).
*       'Designated trunk table'       |
*       -- Lion supports 64 entries (so up to |
*       64 ports in the 'Cascade trunk'). |
*       -- all other devices supports 8    |
*       entries (so up to 8 ports in the  |
*       'Cascade trunk').         |
*       ----------------------------------------------------------------------------
*       3. manipulate only 'Non-trunk' table and | 3. manipulate all trunk tables :
*       'Designated trunk' table       | 'Per port' trunk-id , 'Trunk members',
*       | 'Non-trunk' , 'Designated trunk' tables.
*       ----------------------------------------------------------------------------
*       4. ingress unit must not point to this  | 4. no restriction on ingress/egress
*       trunk (because 'Trunk members' entry |  units.
*       hold no ports)            |
*       ----------------------------------------------------------------------------
*       5. not associated with trunk Id on    | 5. for cascade trunk : since 'Per port'
*       ingress                | the trunk-Id is set , then load balance
*       | according to 'Local port' for traffic
*       | that ingress cascade trunk and
*       | egress next cascade trunk , will
*       | egress only from one of the egress
*       | trunk ports. (because all ports associated
*       | with the trunk-id)
*       ----------------------------------------------------------------------------
*
*/
GT_STATUS prvTgfTrunkCascadeTrunkPortsSet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_PORTS_BMP_STC       *portsMembersPtr
)
{
#if (defined CHX_FAMILY)
       /* call device specific API */
    return cpssDxChTrunkCascadeTrunkPortsSet(devNum, trunkId, portsMembersPtr);
#else
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(trunkId);
    TGF_PARAM_NOT_USED(portsMembersPtr);

    return GT_BAD_STATE;
#endif
}


/**
* @internal prvTgfTrunkHashCrcParametersSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set the CRC hash parameter , relevant for CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_CRC_E .
*
* @note   APPLICABLE DEVICES:      Lion and above devices.
* @param[in] crcMode                  - The CRC mode .
* @param[in] crcSeed                  - The seed used by the CRC computation .
*                                      when crcMode is CRC_6 mode : crcSeed is in range of 0..0x3f (6 bits value)
*                                      when crcMode is CRC_16 mode : crcSeed is in range of 0..0xffff (16 bits value)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or crcMode
* @retval GT_OUT_OF_RANGE          - crcSeed out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note related to feature 'CRC hash mode'
*
*/
GT_STATUS prvTgfTrunkHashCrcParametersSet
(
    IN PRV_TGF_TRUNK_LBH_CRC_MODE_ENT     crcMode,
    IN GT_U32                               crcSeed
)
{
    PRV_UTF_LOG2_MAC("prvTgfTrunkHashCrcParametersSet(%d , %d) \n",
        crcMode,crcSeed);

#if (defined CHX_FAMILY)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;
    CPSS_DXCH_TRUNK_LBH_CRC_MODE_ENT dxCrcMode;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);

    switch(crcMode)
    {
        case PRV_TGF_TRUNK_LBH_CRC_6_MODE_E:
           dxCrcMode = CPSS_DXCH_TRUNK_LBH_CRC_6_MODE_E ;
           crcSeed &= 0x3F;/* support generic code */
           break;
        case PRV_TGF_TRUNK_LBH_CRC_16_AND_PEARSON_MODE_E:
           dxCrcMode = CPSS_DXCH_TRUNK_LBH_CRC_16_AND_PEARSON_MODE_E ;
           crcSeed &= 0xFFFF;/* support generic code */
           break;
        case PRV_TGF_TRUNK_LBH_CRC_32_MODE_E:
           dxCrcMode = CPSS_DXCH_TRUNK_LBH_CRC_32_MODE_E;
           break;
        default:
            return GT_BAD_PARAM;
    }

    /* go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChTrunkHashCrcParametersSet(devNum, dxCrcMode, crcSeed);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC(
               "[TGF]: cpssDxChTrunkHashCrcParametersSet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }

    return rc1;
}
#else
    return GT_BAD_STATE;
#endif
}


/**
* @internal prvTgfTrunkHashCrcParametersGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Get the CRC hash parameter , relevant for CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_CRC_E .
*
* @note   APPLICABLE DEVICES:      Lion and above devices.
* @param[in] devNum                   - device number.
*
* @param[out] crcModePtr               - (pointer to) The CRC mode .
* @param[out] crcSeedPtr               - (pointer to) The seed used by the CRC computation .
*                                      when crcMode is CRC_6 mode : crcSeed  (6 bits value)
*                                      when crcMode is CRC_16 mode : crcSeed (16 bits value)
*                                      when crcMode is CRC_32 mode : crcSeed (32 bits value)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or crcMode
* @retval GT_OUT_OF_RANGE          - crcSeed out of range.
*
* @note related to feature 'CRC hash mode'
*
*/
GT_STATUS prvTgfTrunkHashCrcParametersGet
(
    IN   GT_U8                               devNum,
    OUT  PRV_TGF_TRUNK_LBH_CRC_MODE_ENT      *crcModePtr,
    OUT  GT_U32                              *crcSeedPtr
)
{

#if (defined CHX_FAMILY)
{
    GT_STATUS   rc;
    CPSS_DXCH_TRUNK_LBH_CRC_MODE_ENT dxCrcMode;
    /* call device specific API */
    rc = cpssDxChTrunkHashCrcParametersGet(devNum, &dxCrcMode, crcSeedPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC(
           "[TGF]: cpssDxChTrunkHashCrcParametersGet FAILED, rc = [%d]", rc);
        return rc;
    }

    switch(dxCrcMode)
    {
        case CPSS_DXCH_TRUNK_LBH_CRC_6_MODE_E:
           *crcModePtr = PRV_TGF_TRUNK_LBH_CRC_6_MODE_E ;
           break;
        case CPSS_DXCH_TRUNK_LBH_CRC_16_AND_PEARSON_MODE_E:
           *crcModePtr = PRV_TGF_TRUNK_LBH_CRC_16_AND_PEARSON_MODE_E ;
           break;
        case CPSS_DXCH_TRUNK_LBH_CRC_32_MODE_E:
           *crcModePtr = PRV_TGF_TRUNK_LBH_CRC_32_MODE_E ;
           break;
        default:
            return GT_BAD_STATE;
    }

    return rc;
}
#else
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(crcModePtr);
    TGF_PARAM_NOT_USED(crcSeedPtr);
    return GT_BAD_STATE;
#endif
}


/**
* @internal prvTgfTrunkPortHashMaskInfoSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set port-based hash mask info.
*
* @note   APPLICABLE DEVICES:      Lion and above devices.
* @param[in] devNum                   - The device number.
* @param[in] portNum                  - The port number.
* @param[in] overrideEnable           - enable/disable the override
* @param[in] index                    - the  to use when 'Override enabled'.
*                                      (values 0..15) , relevant only when overrideEnable = GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or portNum
* @retval GT_OUT_OF_RANGE          - when overrideEnable is enabled and index out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note related to feature 'CRC hash mode'
*
*/
GT_STATUS prvTgfTrunkPortHashMaskInfoSet
(
    IN GT_U8     devNum,
    IN GT_PORT_NUM     portNum,
    IN GT_BOOL   overrideEnable,
    IN GT_U32    index
)
{
#if (defined CHX_FAMILY)
       /* call device specific API */
    return cpssDxChTrunkPortHashMaskInfoSet(devNum, portNum, overrideEnable, index);
#else
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(overrideEnable);
    TGF_PARAM_NOT_USED(index);

    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTrunkHashMaskCrcEntrySet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set the entry of masks in the specified index in 'CRC hash mask table'.
*
* @note   APPLICABLE DEVICES:      Lion and above devices.
* @param[in] useIndexOrPacketType     - GT_TRUE - use <index>
*                                      GT_FALSE - use <packetType>
* @param[in] index                    - the table  (0..15)
* @param[in] packetType               - packet type.
* @param[in] entryPtr                 - (pointer to) The entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or index
* @retval GT_OUT_OF_RANGE          - one of the fields in entryPtr are out of range
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note related to feature 'CRC hash mode'
*
*/
GT_STATUS prvTgfTrunkHashMaskCrcEntrySet
(
    IN GT_BOOL                        useIndexOrPacketType,
    IN GT_U32                         index,
    IN PRV_TGF_PCL_PACKET_TYPE_ENT    packetType,
    IN PRV_TGF_TRUNK_LBH_CRC_MASK_ENTRY_STC *entryPtr
)
{
    PRV_UTF_LOG3_MAC("prvTgfTrunkHashMaskCrcEntrySet(%d,%d,%d) \n",
        useIndexOrPacketType,
        index,
        packetType);

#if (defined CHX_FAMILY)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;
    CPSS_DXCH_TRUNK_LBH_CRC_MASK_ENTRY_STC   dxChEntry;
    GT_U32  finalIndex;
    GT_BOOL isIpv6 = GT_FALSE;

    if(entryPtr == NULL)
    {
        return GT_BAD_PARAM;
    }

    dxChEntry.l4DstPortMaskBmp        =     entryPtr->l4DstPortMaskBmp;
    dxChEntry.l4SrcPortMaskBmp        =     entryPtr->l4SrcPortMaskBmp;
    dxChEntry.ipv6FlowMaskBmp         =     entryPtr->ipv6FlowMaskBmp;
    dxChEntry.ipDipMaskBmp            =     entryPtr->ipDipMaskBmp;
    dxChEntry.ipSipMaskBmp            =     entryPtr->ipSipMaskBmp;
    dxChEntry.macDaMaskBmp            =     entryPtr->macDaMaskBmp;
    dxChEntry.macSaMaskBmp            =     entryPtr->macSaMaskBmp;
    dxChEntry.mplsLabel0MaskBmp       =     entryPtr->mplsLabel0MaskBmp;
    dxChEntry.mplsLabel1MaskBmp       =     entryPtr->mplsLabel1MaskBmp;
    dxChEntry.mplsLabel2MaskBmp       =     entryPtr->mplsLabel2MaskBmp;
    dxChEntry.localSrcPortMaskBmp     =     entryPtr->localSrcPortMaskBmp;
    dxChEntry.udbsMaskBmp             =     entryPtr->udbsMaskBmp;

    dxChEntry.symmetricMacAddrEnable  =     entryPtr->symmetricMacAddrEnable  ;
    dxChEntry.symmetricIpv4AddrEnable =     entryPtr->symmetricIpv4AddrEnable ;
    dxChEntry.symmetricIpv6AddrEnable =     entryPtr->symmetricIpv6AddrEnable ;
    dxChEntry.symmetricL4PortEnable   =     entryPtr->symmetricL4PortEnable   ;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);

    if(useIndexOrPacketType == GT_TRUE)
    {
        finalIndex = index;
    }
    else
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(prvTgfDevNum) == GT_TRUE)
        {
            switch(packetType)
            {
                case PRV_TGF_PCL_PACKET_TYPE_IPV4_TCP_E:       finalIndex = 0;         break;
                case PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E:       finalIndex = 1;         break;
                case PRV_TGF_PCL_PACKET_TYPE_MPLS_E:           finalIndex = 2;         break;
                case PRV_TGF_PCL_PACKET_TYPE_IPV4_FRAGMENT_E:  finalIndex = 3;         break;
                case PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E:     finalIndex = 4;         break;
                case PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E: finalIndex = 5;         break;
                case PRV_TGF_PCL_PACKET_TYPE_IPV6_E:
                    finalIndex = 6;
                    isIpv6 = GT_TRUE;/* allow to set also ipv6 TCP,UDP - for BWC */
                    break;

                case PRV_TGF_PCL_PACKET_TYPE_UDE_E:            finalIndex = 9;         break;
                case PRV_TGF_PCL_PACKET_TYPE_UDE_1_E:          finalIndex = 10;        break;
                case PRV_TGF_PCL_PACKET_TYPE_UDE_2_E:          finalIndex = 11;        break;
                case PRV_TGF_PCL_PACKET_TYPE_UDE_3_E:          finalIndex = 12;        break;
                case PRV_TGF_PCL_PACKET_TYPE_UDE_4_E:          finalIndex = 13;        break;
                    default:
                        return GT_BAD_PARAM;

            }
        }
        else
        {
            switch(packetType)
            {
                case PRV_TGF_PCL_PACKET_TYPE_IPV4_TCP_E:       finalIndex = 0;         break;
                case PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E:       finalIndex = 1;         break;
                case PRV_TGF_PCL_PACKET_TYPE_MPLS_E:           finalIndex = 2;         break;
                case PRV_TGF_PCL_PACKET_TYPE_IPV4_FRAGMENT_E:  finalIndex = 3;         break;
                case PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E:     finalIndex = 4;         break;
                case PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E: finalIndex = 5;         break;
                case PRV_TGF_PCL_PACKET_TYPE_UDE_E:            finalIndex = 6;         break;
                case PRV_TGF_PCL_PACKET_TYPE_IPV6_E:           finalIndex = 7;         break;
                case PRV_TGF_PCL_PACKET_TYPE_UDE_1_E:          finalIndex = 8;         break;
                case PRV_TGF_PCL_PACKET_TYPE_UDE_2_E:          finalIndex = 9;         break;
                case PRV_TGF_PCL_PACKET_TYPE_UDE_3_E:          finalIndex = 10;        break;
                case PRV_TGF_PCL_PACKET_TYPE_UDE_4_E:          finalIndex = 11;        break;
                    default:
                        return GT_BAD_PARAM;

            }
        }

        finalIndex += 16;

    }


    /* go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE &&
            isIpv6 == GT_TRUE)
        {
            /* call device specific API */
            rc = cpssDxChTrunkHashMaskCrcEntrySet(devNum, finalIndex + 1/*TCP*/, &dxChEntry);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC(
                   "[TGF]: cpssDxChTrunkHashMaskCrcEntrySet (ipv6 TCP) FAILED, rc = [%d]", rc);
                rc1 = rc;
            }

            /* call device specific API */
            rc = cpssDxChTrunkHashMaskCrcEntrySet(devNum, finalIndex + 2/*UDP*/, &dxChEntry);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC(
                   "[TGF]: cpssDxChTrunkHashMaskCrcEntrySet (ipv6 UDP) FAILED, rc = [%d]", rc);
                rc1 = rc;
            }
        }

        /* call device specific API */
        rc = cpssDxChTrunkHashMaskCrcEntrySet(devNum, finalIndex, &dxChEntry);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC(
               "[TGF]: cpssDxChTrunkHashMaskCrcEntrySet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }

    return rc1;
}
#else
    TGF_PARAM_NOT_USED(index);
    TGF_PARAM_NOT_USED(entryPtr);

    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTrunkHashPearsonValueSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set the Pearson hash value for the specific index.
*         NOTE: the Pearson hash used when CRC-16 mode is used.
*
* @note   APPLICABLE DEVICES:      Lion and above devices.
* @param[in] index                    - the table  (0..63)
* @param[in] value                    - the Pearson hash  (0..63)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or index
* @retval GT_OUT_OF_RANGE          - value > 63
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note related to feature 'CRC hash mode'
*
*/
GT_STATUS prvTgfTrunkHashPearsonValueSet
(
    IN GT_U32                         index,
    IN GT_U32                         value
)
{
#if (defined CHX_FAMILY)
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);

    /* go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChTrunkHashPearsonValueSet(devNum, index, value);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC(
               "[TGF]: cpssDxChTrunkHashPearsonValueSet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }

    return rc1;

#else
    TGF_PARAM_NOT_USED(index);
    TGF_PARAM_NOT_USED(value);

    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTrunkHashDesignatedTableModeSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set the hash mode of the designated trunk table .
*
* @note   APPLICABLE DEVICES:      All Devices
* @param[in] mode                     - The designated table hashing mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number or mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTrunkHashDesignatedTableModeSet
(
    IN PRV_TGF_TRUNK_DESIGNATED_TABLE_MODE_ENT    mode
)
{
#if (defined CHX_FAMILY)
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;
    CPSS_DXCH_TRUNK_DESIGNATED_TABLE_MODE_ENT   dxMode;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    switch(mode)
    {
        case PRV_TGF_TRUNK_DESIGNATED_TABLE_USE_INGRESS_HASH_E  :
            dxMode = CPSS_DXCH_TRUNK_DESIGNATED_TABLE_USE_INGRESS_HASH_E;
            break;
        case PRV_TGF_TRUNK_DESIGNATED_TABLE_USE_INGRESS_HASH_AND_VID_E:
            dxMode = CPSS_DXCH_TRUNK_DESIGNATED_TABLE_USE_INGRESS_HASH_AND_VID_E;
            break;
        case PRV_TGF_TRUNK_DESIGNATED_TABLE_USE_SOURCE_INFO_E   :
            dxMode = CPSS_DXCH_TRUNK_DESIGNATED_TABLE_USE_SOURCE_INFO_E;
            break;
        default:
            return GT_BAD_PARAM;
    }


    /* go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChTrunkHashDesignatedTableModeSet(devNum, dxMode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC(
               "[TGF]: cpssDxChTrunkHashDesignatedTableModeSet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }

    return rc1;

#else
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;
    CPSS_EXMXPM_TRUNK_LBH_DESIGNATED_MODE_ENT   exMxPmMode;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    switch(mode)
    {
        case PRV_TGF_TRUNK_DESIGNATED_TABLE_USE_INGRESS_HASH_E  :
        case PRV_TGF_TRUNK_DESIGNATED_TABLE_USE_INGRESS_HASH_AND_VID_E:
            return GT_BAD_PARAM;
        case PRV_TGF_TRUNK_DESIGNATED_TABLE_USE_SOURCE_INFO_E   :
            exMxPmMode = CPSS_EXMXPM_TRUNK_LBH_DESIGNATED_MODE_VIDX_VID_SRC_INTERFACE_E;
            break;
        default:
            return GT_BAD_PARAM;
    }


    /* go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmTrunkHashDesignatedTableModeSet(devNum, exMxPmMode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC(
               "[TGF]: cpssExMxPmTrunkHashDesignatedTableModeSet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }

    return rc1;
#endif
}

/**
* @internal prvTgfTrunkMcLocalSwitchingEnableSet function
* @endinternal
*
* @brief   Enable/Disable sending multi-destination packets back to its source
*         trunk on the local device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or bad trunkId number
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. the behavior of multi-destination traffic ingress from trunk is
*       not-affected by setting of cpssDxChBrgVlanLocalSwitchingEnableSet
*       and not-affected by setting of cpssDxChBrgPortEgressMcastLocalEnable
*       2. the functionality manipulates the 'non-trunk' table entry of the trunkId
*
*/
GT_STATUS prvTgfTrunkMcLocalSwitchingEnableSet
(
    IN GT_TRUNK_ID      trunkId,
    IN GT_BOOL          enable
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
   GT_U8       devNum  = 0;
   GT_STATUS   rc, rc1 = GT_OK;

   /* prepare device iterator */
   PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

   /* go over all active devices */
   while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
   {
       /* call device specific API */
#ifdef EXMXPM_FAMILY
       rc = cpssExMxPmTrunkMcLocalSwitchingEnableSet(devNum, trunkId, enable);
#else
       rc = cpssDxChTrunkMcLocalSwitchingEnableSet(devNum, trunkId, enable);
#endif
       if (GT_OK != rc)
       {
#ifdef EXMXPM_FAMILY
           PRV_UTF_LOG1_MAC(
               "[TGF]: cpssExMxPmTrunkMcLocalSwitchingEnableSet FAILED, rc = [%d]", rc);
#else
           PRV_UTF_LOG1_MAC(
               "[TGF]: cpssDxChTrunkMcLocalSwitchingEnableSet FAILED, rc = [%d]", rc);
#endif
           rc1 = rc;
       }
   }

   return rc1;

#else
    TGF_PARAM_NOT_USED(trunkId);
    TGF_PARAM_NOT_USED(enable);

    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTrunkHashIpAddMacModeSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set the use of mac address bits to trunk hash calculation when packet is
*         IP and the "Ip trunk hash mode enabled".
*         Note:
*         1. Not relevant to NON Ip packets.
*         2. Not relevant to multi-destination packets (include routed IPM).
*         3. Not relevant when cpssDxChTrunkHashGlobalModeSet(devNum,
*         CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTrunkHashIpAddMacModeSet
(
    IN GT_BOOL  enable
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
   GT_U8       devNum  = 0;
   GT_STATUS   rc, rc1 = GT_OK;

   /* prepare device iterator */
   PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

   /* go over all active devices */
   while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
   {
       /* call device specific API */
#ifdef EXMXPM_FAMILY
       rc = cpssExMxPmTrunkHashAddMacModeEnableSet(devNum, enable);
#else
       rc = cpssDxChTrunkHashIpAddMacModeSet(devNum, enable);
#endif
       if (GT_OK != rc)
       {
#ifdef EXMXPM_FAMILY
           PRV_UTF_LOG1_MAC(
               "[TGF]: cpssExMxPmTrunkHashAddMacModeEnableSet FAILED, rc = [%d]", rc);
#else
           PRV_UTF_LOG1_MAC(
               "[TGF]: cpssDxChTrunkHashIpAddMacModeSet FAILED, rc = [%d]", rc);
#endif
           rc1 = rc;
       }
   }

   return rc1;

#else
    TGF_PARAM_NOT_USED(enable);

    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTrunkHashIpAddMacModeGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Get the use of mac address bits to trunk hash calculation when packet is
*         IP and the "Ip trunk hash mode enabled".
*         Note:
*         1. Not relevant to NON Ip packets.
*         2. Not relevant to multi-destination packets (include routed IPM).
*         3. Not relevant when cpssDxChTrunkHashGlobalModeSet(devNum,
*         CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @param[out] enablePtr                - (pointer to)enable/disable feature
*                                      GT_FALSE - If the packet is an IP packet MAC data is not
*                                      added to the Trunk load balancing hash.
*                                      GT_TRUE - The following function is added to the trunk load
*                                      balancing hash:
*                                      MACTrunkHash = MAC_SA[5:0]^MAC_DA[5:0].
*                                      NOTE: When the packet is not an IP packet and
*                                      <TrunkLBH Mode> = 0, the trunk load balancing
*                                      hash = MACTrunkHash, regardless of this setting.
*                                      If the packet is IPv4/6-over-X tunnel-terminated,
*                                      the mode is always GT_FALSE (since there is no
*                                      passenger packet MAC header).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTrunkHashIpAddMacModeGet
(
    OUT GT_BOOL  *enablePtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
   GT_U8       devNum  = 0;
   GT_STATUS   rc, rc1 = GT_OK;

   /* prepare device iterator */
   PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

   /* go over all active devices */
   while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
   {
       /* call device specific API */
#ifdef EXMXPM_FAMILY
       rc = cpssExMxPmTrunkHashAddMacModeEnableGet(devNum, enablePtr);
#else
       rc = cpssDxChTrunkHashIpAddMacModeGet(devNum, enablePtr);
#endif
       if (GT_OK != rc)
       {
#ifdef EXMXPM_FAMILY
           PRV_UTF_LOG1_MAC(
               "[TGF]: cpssExMxPmTrunkHashAddMacModeEnableGet FAILED, rc = [%d]", rc);
#else
           PRV_UTF_LOG1_MAC(
               "[TGF]: cpssDxChTrunkHashIpAddMacModeGet FAILED, rc = [%d]", rc);
#endif
           rc1 = rc;
       }

       break;
   }

   return rc1;

#else
    TGF_PARAM_NOT_USED(enablePtr);

    return GT_BAD_STATE;
#endif
}


/**
* @internal prvTgfTrunkHashIpv6ModeSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set the hash generation function for Ipv6 packets.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; ExMxPm.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] hashMode                 - the Ipv6 hash mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or hash mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTrunkHashIpv6ModeSet
(
    IN PRV_TGF_TRUNK_IPV6_HASH_MODE_ENT   hashMode
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
   GT_U8       devNum  = 0;
   GT_STATUS   rc, rc1 = GT_OK;
   #ifdef CHX_FAMILY
   CPSS_DXCH_TRUNK_IPV6_HASH_MODE_ENT   dxHashMode = (GT_U32)hashMode;
   #endif /*CHX_FAMILY*/
   #ifdef EXMXPM_FAMILY
   CPSS_EXMXPM_TRUNK_LBH_IPV6_MODE_ENT   exMxPmHashMode = (GT_U32)hashMode;
   #endif /*EXMXPM_FAMILY*/

   /* prepare device iterator */
   PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

   /* go over all active devices */
   while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
   {
       /* call device specific API */
#ifdef EXMXPM_FAMILY
       rc = cpssExMxPmTrunkHashIpv6ModeSet(devNum, exMxPmHashMode);
#else
       rc = cpssDxChTrunkHashIpv6ModeSet(devNum, dxHashMode);
#endif
       if (GT_OK != rc)
       {
#ifdef EXMXPM_FAMILY
           PRV_UTF_LOG1_MAC(
               "[TGF]: cpssExMxPmTrunkHashIpv6ModeSet FAILED, rc = [%d]", rc);
#else
           PRV_UTF_LOG1_MAC(
               "[TGF]: cpssDxChTrunkHashIpv6ModeSet FAILED, rc = [%d]", rc);
#endif
           rc1 = rc;
       }
   }

   return rc1;

#else
    TGF_PARAM_NOT_USED(trunkId);
    TGF_PARAM_NOT_USED(memberPtr);

    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTrunkHashIpv6ModeGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Get the hash generation function for Ipv6 packets.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; ExMxPm.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
*
* @param[out] hashModePtr              - (pointer to)the Ipv6 hash mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or hash mode
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTrunkHashIpv6ModeGet
(
    IN GT_U8                                devNum,
    OUT PRV_TGF_TRUNK_IPV6_HASH_MODE_ENT   *hashModePtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
   GT_STATUS   rc = GT_OK;

   #ifdef CHX_FAMILY
   CPSS_DXCH_TRUNK_IPV6_HASH_MODE_ENT   dxHashMode;
   #endif /*CHX_FAMILY*/
   #ifdef EXMXPM_FAMILY
   CPSS_EXMXPM_TRUNK_LBH_IPV6_MODE_ENT   exMxPmHashMode;
   #endif /*EXMXPM_FAMILY*/

    /* call device specific API */
#ifdef EXMXPM_FAMILY
    rc = cpssExMxPmTrunkHashIpv6ModeGet(devNum, &exMxPmHashMode);
    *hashModePtr = (GT_U32)exMxPmHashMode;
#else
    rc = cpssDxChTrunkHashIpv6ModeGet(devNum, &dxHashMode);
    *hashModePtr = (GT_U32)dxHashMode;
#endif
    if (GT_OK != rc)
    {
#ifdef EXMXPM_FAMILY
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmTrunkHashIpv6ModeGet FAILED, rc = [%d]", rc);
#else
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChTrunkHashIpv6ModeGet FAILED, rc = [%d]", rc);
#endif
        return rc;
    }

    /* convert from device specific format */

    return GT_OK;

#else
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTrunkHashL4ModeSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Enable/Disable the device from considering the L4 TCP/UDP
*         source/destination port information, when calculation the trunk hashing
*         index for a packet.
*         Note:
*         1. Not relevant to NON TCP/UDP packets.
*         2. The Ipv4 hash must also be enabled , otherwise the L4 hash mode
*         setting not considered.
*         3. Not relevant to multi-destination packets (include routed IPM).
*         4. Not relevant when cpssDxChTrunkHashGlobalModeSet(devNum,
*         CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; ExMxPm.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] hashMode                 - L4 hash mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTrunkHashL4ModeSet
(
    IN PRV_TGF_TRUNK_L4_LBH_MODE_ENT hashMode
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
   GT_U8       devNum  = 0;
   GT_STATUS   rc, rc1 = GT_OK;
   #ifdef CHX_FAMILY
   CPSS_DXCH_TRUNK_L4_LBH_MODE_ENT   dxHashMode = (GT_U32)hashMode;
   #endif /*CHX_FAMILY*/
   #ifdef EXMXPM_FAMILY
   CPSS_EXMXPM_TRUNK_LBH_L4_MODE_ENT   exMxPmHashMode = (GT_U32)hashMode;
   #endif /*EXMXPM_FAMILY*/

   /* prepare device iterator */
   PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

   /* go over all active devices */
   while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
   {
       /* call device specific API */
#ifdef EXMXPM_FAMILY
       rc = cpssExMxPmTrunkHashL4ModeSet(devNum, exMxPmHashMode);
#else
       rc = cpssDxChTrunkHashL4ModeSet(devNum, dxHashMode);
#endif
       if (GT_OK != rc)
       {
#ifdef EXMXPM_FAMILY
           PRV_UTF_LOG1_MAC(
               "[TGF]: cpssExMxPmTrunkHashL4ModeSet FAILED, rc = [%d]", rc);
#else
           PRV_UTF_LOG1_MAC(
               "[TGF]: cpssDxChTrunkHashL4ModeSet FAILED, rc = [%d]", rc);
#endif
           rc1 = rc;
       }
   }

   return rc1;

#else
    TGF_PARAM_NOT_USED(trunkId);
    TGF_PARAM_NOT_USED(memberPtr);

    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTrunkHashL4ModeGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         get the Enable/Disable of device from considering the L4 TCP/UDP
*         source/destination port information, when calculation the trunk hashing
*         index for a packet.
*         Note:
*         1. Not relevant to NON TCP/UDP packets.
*         2. The Ipv4 hash must also be enabled , otherwise the L4 hash mode
*         setting not considered.
*         3. Not relevant to multi-destination packets (include routed IPM).
*         4. Not relevant when cpssDxChTrunkHashGlobalModeSet(devNum,
*         CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; ExMxPm.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
*
* @param[out] hashModePtr              - (pointer to)L4 hash mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTrunkHashL4ModeGet
(
    IN GT_U8   devNum,
    OUT PRV_TGF_TRUNK_L4_LBH_MODE_ENT *hashModePtr
)
{
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
   GT_STATUS   rc = GT_OK;

   #ifdef CHX_FAMILY
   CPSS_DXCH_TRUNK_L4_LBH_MODE_ENT   dxHashMode;
   #endif /*CHX_FAMILY*/
   #ifdef EXMXPM_FAMILY
   CPSS_EXMXPM_TRUNK_LBH_L4_MODE_ENT   exMxPmHashMode;
   #endif /*EXMXPM_FAMILY*/

    /* call device specific API */
#ifdef EXMXPM_FAMILY
    rc = cpssExMxPmTrunkHashL4ModeGet(devNum, &exMxPmHashMode);
    *hashModePtr = (GT_U32)exMxPmHashMode;
#else
    rc = cpssDxChTrunkHashL4ModeGet(devNum, &dxHashMode);
    *hashModePtr = (GT_U32)dxHashMode;
#endif
    if (GT_OK != rc)
    {
#ifdef EXMXPM_FAMILY
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmTrunkHashL4ModeGet FAILED, rc = [%d]", rc);
#else
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChTrunkHashL4ModeGet FAILED, rc = [%d]", rc);
#endif
        return rc;
    }

    /* convert from device specific format */

    return GT_OK;

#else
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTrunkDbMembersSortingEnableSet function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function enable/disable 'sorting' of trunk members in the
*         'trunk members table' and in the 'designated trunk table'
*         This mode not effect 'cascade trunks' (that not need synchronization between devices)
*         'sorting enabled' : when the application will add/remove/set members in a trunk
*         cpss will make sure to set the trunk members into the
*         device in an order that is not effected by the 'history'
*         of 'add/remove' members to/from the trunk.
*         'sorting disabled' : (legacy mode / default mode) when the application
*         will add/remove/set members in a trunk cpss will set
*         the trunk members into the device in an order that is
*         effected by the 'history' of 'add/remove' members
*         to/from the trunk.
*         function uses the DB (no HW operations)
*         The API start to effect only on trunks that do operations (add/remove/set members)
*         after the API call.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  ExMxPm.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - enable/disable the sorting
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_INITIALIZED       - the trunk library was not initialized for the device
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTrunkDbMembersSortingEnableSet
(
    IN GT_U8                    devNum,
    IN GT_BOOL                  enable
)
{
#if (defined CHX_FAMILY)
   /* call device specific API */
   return cpssDxChTrunkDbMembersSortingEnableSet(devNum, enable);
#else
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enable);

    return GT_NOT_SUPPORTED;
#endif
}

/**
* @internal prvTgfTrunkDbMembersSortingEnableGet function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         Gets the enable/disable 'sorting' of trunk members in the 'trunk members table'
*         and in the 'designated trunk table'
*         This mode not effect 'cascade trunks' (that not need synchronization between devices)
*         function uses the DB (no HW operations)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) the sorting is enabled/disabled
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_INITIALIZED       - the trunk library was not initialized for the device
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTrunkDbMembersSortingEnableGet
(
    IN GT_U8                    devNum,
    OUT GT_BOOL                 *enablePtr
)
{
#if (defined CHX_FAMILY)
   /* call device specific API */
   return cpssDxChTrunkDbMembersSortingEnableGet(devNum, enablePtr);
#else
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enablePtr);

    return GT_NOT_SUPPORTED;
#endif
}

/**
* @internal prvTgfTrunkCascadeTrunkWithWeightedPortsSet function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function sets the 'cascade' trunk with the Weighted specified 'Local ports'
*         overriding any previous setting.
*         the weights effect the number of times that each member will get representation
*         in the 'designated table' .
*         The cascade trunk may be invalidated by numOfMembers = 0.
*         Local ports are ports of only configured device.
*         Cascade trunk is:
*         - members are ports of only configured device pointed by devNum
*         - trunk members table is empty (see cpssDxChTrunkTableEntrySet)
*         Therefore it cannot be used as target by ingress engines like FDB,
*         Router, TTI, Ingress PCL and so on.
*         - members ports trunk ID are set (see cpssDxChTrunkPortTrunkIdSet).
*         Therefore packets ingresses in member ports are associated with trunk
*         - all members are enabled only and cannot be disabled.
*         - may be used for cascade traffic and pointed by the 'Device map table'
*         as the local target to reach to the 'Remote device'.
*         (For 'Device map table' refer to cpssDxChCscdDevMapTableSet(...))
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - trunk id
* @param[in] numOfMembers             - number of members in the array.
*                                      value 0 - meaning that the trunk-id is 'invalidated' and
*                                      trunk-type will be changed to : CPSS_TRUNK_TYPE_FREE_E
*                                      value != 0 - meaning that the trunk-type will be : CPSS_TRUNK_TYPE_CASCADE_E
* @param[in] weightedMembersArray[]   - (array of) members of the cascade trunk.
*                                      each member hold relative weight (relative to Weight of all members)
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_INITIALIZED       - the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - there are members that not supported by the device.
* @retval GT_BAD_PARAM             - bad device number , or bad trunkId number , or bad total
*                                       weights (see restrictions below)
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_CASCADE_E
* @retval GT_ALREADY_EXIST         - one of the members already exists in another trunk ,
*                                       or this trunk hold members defined using cpssDxChTrunkMembersSet(...)
* @retval GT_BAD_PTR               - when numOfMembers != 0 and weightedMembersArray = NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. This function does not set ports as 'Cascade ports' (and also not
*       check that ports are 'cascade').
*       2. This function sets only next tables :
*       a. the designated trunk table:
*       distribute MC/Cascade trunk traffic among the members according to their weight
*       b. the 'Non-trunk' table entry.
*       c. 'per port' trunkId
*       3. because this function not set the 'Trunk members' table entry , the
*       application should not point to this trunk from any ingress unit , like:
*       FDB , PCL action redirect , NH , TTI action redirect , PVE ...
*       (it should be pointed ONLY from the device map table)
*       4. this API supports only trunks with types : CPSS_TRUNK_TYPE_FREE_E or
*       CPSS_TRUNK_TYPE_CASCADE_E.
*       5. next APIs are not supported from trunk with type : CPSS_TRUNK_TYPE_CASCADE_E
*       cpssDxChTrunkMembersSet ,
*       cpssDxChTrunkMemberAdd , cpssDxChTrunkMemberRemove,
*       cpssDxChTrunkMemberEnable , cpssDxChTrunkMemberDisable
*       cpssDxChTrunkDbEnabledMembersGet , cpssDxChTrunkDbDisabledMembersGet
*       cpssDxChTrunkDesignatedMemberSet , cpssDxChTrunkDbDesignatedMemberGet
*       6. the total weights of all the trunk members is restricted to :
*       a. must be equal to power of 2 (1,2,4,8,16,32,64...)
*       b. must not be larger then actual designated table size of the device.
*       (Lion ,Lion2 : 64 , other devices : 8)
*       c. not relevant when single member exists
*       Comparing the 2 function :
*       cpssDxChTrunkCascadeTrunkWithWeightedPortsSet |  cpssDxChTrunkMembersSet
*       ----------------------------------------------------------------------------
*       1. purpose 'Cascade trunk'        | 1. purpose 'Network trunk' , and
*       |  also 'Cascade trunk' with up to
*       |  8 members
*       ----------------------------------------------------------------------------
*       2. supported number of members depends  | 2. supports up to 8 members
*       on number of entries in the      |  (also in Lion).
*       'Designated trunk table'       |
*       -- Lion supports 64 entries (so up to |
*       64 ports in the 'Cascade trunk'). |
*       -- all other devices supports 8    |
*       entries (so up to 8 ports in the  |
*       'Cascade trunk').         |
*       ----------------------------------------------------------------------------
*       3. manipulate next trunk tables :    | 3. manipulate all trunk tables :
*       'Per port' trunk-id          | 'Per port' trunk-id , 'Trunk members',
*       'Non-trunk' , 'Designated trunk'   | 'Non-trunk' , 'Designated trunk' tables.
*       ----------------------------------------------------------------------------
*       4. ingress unit must not point to this  | 4. no restriction on ingress/egress
*       trunk (because 'Trunk members' entry |  units.
*       hold no ports)            |
*
*/
GT_STATUS prvTgfTrunkCascadeTrunkWithWeightedPortsSet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN GT_U32                   numOfMembers,
    IN CPSS_TRUNK_WEIGHTED_MEMBER_STC       weightedMembersArray[]
)
{
#if (defined CHX_FAMILY)
    GT_STATUS rc;
    GT_U32    i;
    for(i = 0 ; i < numOfMembers ; i++)
    {
        /* need to remove casting and to fix code after lion2 development is done */
        CPSS_TBD_BOOKMARK_EARCH

        rc = prvUtfHwDeviceNumberGet((GT_U8)weightedMembersArray[i].member.hwDevice,
                                     &weightedMembersArray[i].member.hwDevice);
        if (GT_OK != rc)
        {
           PRV_UTF_LOG1_MAC(
               "[TGF]: prvUtfHwDeviceNumberGet FAILED, rc = [%d]", rc);

           return rc;
        }
    }

       /* call device specific API */
    return cpssDxChTrunkCascadeTrunkWithWeightedPortsSet(devNum, trunkId, numOfMembers,weightedMembersArray);
#else
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(trunkId);
    TGF_PARAM_NOT_USED(numOfMembers);
    TGF_PARAM_NOT_USED(weightedMembersArray);

    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTrunkCascadeTrunkWithWeightedPortsGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         This function gets the Weighted 'Local ports' of 'cascade' trunk .
*         the weights reflect the number of times that each member is represented
*         in the 'designated table'
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - trunk id
* @param[in,out] numOfMembersPtr          - (pointer to) max num of members to retrieve - this value refer to the number of
*                                      members that the array of weightedMembersArray[] can retrieve.
*                                      OUTPUTS:
* @param[in,out] numOfMembersPtr          - (pointer to) the actual num of members in the trunk
*                                      (up to : Lion,Lion2 : 64 , others : 8)
* @param[in] weightedMembersArray[]   - (array of) members that are members of the cascade trunk.
*                                      each member hold relative weight (relative to Weight of all members)
* @param[in,out] numOfMembersPtr          - (pointer to) the actual num of members in the trunk
*                                      (up to : Lion,Lion2 : 64 , others : 8)
*
* @param[out] weightedMembersArray[]   - (array of) members that are members of the cascade trunk.
*                                      each member hold relative weight (relative to Weight of all members)
*                                      OUTPUTS:
*                                      None.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_CASCADE_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTrunkCascadeTrunkWithWeightedPortsGet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    INOUT GT_U32                   *numOfMembersPtr,
    OUT CPSS_TRUNK_WEIGHTED_MEMBER_STC       weightedMembersArray[]
)
{
#if (defined CHX_FAMILY)
    GT_STATUS rc;
    GT_U32    i;

    /* call device specific API */
    rc = cpssDxChTrunkCascadeTrunkWithWeightedPortsGet(devNum, trunkId, numOfMembersPtr,weightedMembersArray);
    if (GT_OK != rc)
    {
       PRV_UTF_LOG1_MAC(
           "[TGF]: cpssDxChTrunkCascadeTrunkWithWeightedPortsGet FAILED, rc = [%d]", rc);

       return rc;
    }

    for(i = 0 ; i < (*numOfMembersPtr) ; i++)
    {
        rc = prvUtfSwFromHwDeviceNumberGet(weightedMembersArray[i].member.hwDevice,
                                       &(weightedMembersArray[i].member.hwDevice));
        if (GT_OK != rc)
        {
           PRV_UTF_LOG1_MAC(
               "[TGF]: prvUtfSwFromHwDeviceNumberGet FAILED, rc = [%d]", rc);

           return rc;
        }
    }

    return GT_OK;

#else
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(trunkId);
    TGF_PARAM_NOT_USED(numOfMembersPtr);
    TGF_PARAM_NOT_USED(weightedMembersArray);

    return GT_BAD_STATE;
#endif
}


/**
* @internal prvTgfTrunkTrunkDump function
* @endinternal
*
* @brief   debug function to dump the trunk info from DB and the HW.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*                                      OUTPUTS:
*                                      None.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTrunkTrunkDump
(
    IN    GT_U8 devNum
)
{
#if (defined CHX_FAMILY) && (defined IMPL_GALTIS)
    return cpssDxChTrunkDump(devNum);
#else
    TGF_PARAM_NOT_USED(devNum);

    return GT_NOT_IMPLEMENTED;
#endif
}

/**
* @internal prvTgfTrunkHashCrcSaltByteSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Get the CRC Salt value for specific byte .
*         relevant for CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_CRC_E , CPSS_DXCH_TRUNK_LBH_CRC_32_MODE_E.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The device number.
* @param[in] byteIndex                - the byte index one of the 70 bytes (APPLICABLE RANGES: 0..69)
* @param[in] saltValue                - the salt value of the byte (8 bits)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or byteIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note related to feature 'CRC 32 hash mode'
*
*/
GT_STATUS prvTgfTrunkHashCrcSaltByteSet
(
    IN GT_U8    devNum,
    IN GT_U32   byteIndex,
    IN GT_U8    saltValue
)
{
    PRV_UTF_LOG3_MAC("prvTgfTrunkHashCrcSaltByteSet(%d,%d,%d) \n",
        devNum,
        byteIndex,
        saltValue);

#if (defined CHX_FAMILY)
    return cpssDxChTrunkHashCrcSaltByteSet(devNum,byteIndex,saltValue);
#else
    return GT_NOT_IMPLEMENTED;
#endif
}

/**
* @internal prvTgfTrunkHashMaskCrcParamOverrideSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Set which fields of the CRC hash input bytes are override by other fields.
*         for example : option to use FlowID instead of UDB21,UDB22.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number.
* @param[in] fieldType                - the field type that used to override.
* @param[in] override                 -  the original field.
*                                      GT_TRUE  - override.
*                                      GT_FALSE - not override.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or fieldType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note related to feature 'CRC hash mode'
*
*/
GT_STATUS prvTgfTrunkHashMaskCrcParamOverrideSet
(
    IN GT_U8                                            devNum,
    IN PRV_TGF_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_ENT   fieldType,
    IN GT_BOOL                                          override
)
{
    PRV_UTF_LOG3_MAC("prvTgfTrunkHashMaskCrcParamOverrideSet(%d,%d,%d) \n",
        devNum,
        fieldType,
        override);

#if (defined CHX_FAMILY)
    {
        CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_ENT dxFieldType;

        switch(fieldType)
        {
            case  PRV_TGF_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_UDB21_UDB22_BY_FLOW_ID_E:
                dxFieldType = CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_UDB21_UDB22_BY_FLOW_ID_E;
                break;
            case  PRV_TGF_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_MPLS_LABEL0_BY_EVLAN_E:
                dxFieldType = CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_MPLS_LABEL0_BY_EVLAN_E;
                break;
            case  PRV_TGF_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_MPLS_LABEL1_BY_ORIGINAL_SOURCE_EPORT_E:
                dxFieldType = CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_MPLS_LABEL1_BY_ORIGINAL_SOURCE_EPORT_E;
                break;
            case  PRV_TGF_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_MPLS_LABEL2_BY_LOCAL_SOURCE_EPORT_E:
                dxFieldType = CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_MPLS_LABEL2_BY_LOCAL_SOURCE_EPORT_E;
                break;
            case  PRV_TGF_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_IPV6_ADDR_24MSB_BY_24UDBS_E:
                dxFieldType = CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_IPV6_ADDR_24MSB_BY_24UDBS_E;
                break;
            default:
                return GT_BAD_PARAM;
        }


        return cpssDxChTrunkHashMaskCrcParamOverrideSet(devNum,dxFieldType,override);
    }
#else
    return GT_NOT_IMPLEMENTED;
#endif
}



/**
* @internal prvTgfTrunkPortTrunkIdSet function
* @endinternal
*
* @brief   Function Relevant mode : Low level mode
*         Set the trunkId field in the port's control register in the device
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - physical port number.
* @param[in] memberOfTrunk            - is the port associated with the trunk
*                                      GT_FALSE - the port is set as "not member" in the trunk
*                                      GT_TRUE  - the port is set with the trunkId
* @param[in] trunkId                  - the trunk to which the port associate with
*                                      This field indicates the trunk group number (ID) to which the
*                                      port is a member.
*                                      1 through "max number of trunks" = The port is a member of the trunk
*                                      this value relevant only when memberOfTrunk = GT_TRUE
*
* @retval GT_OK                    - successful completion
* @retval GT_FAIL                  - an error occurred.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad port number , or
*                                       bad trunkId number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The trunkId configuration should be done after the configuration of the
*       ePort to portNum in the translation table.
*       In case of a change in the translation table this API should be called
*       again, since the configuration done here will not be correct any more.
*
*/
GT_STATUS prvTgfTrunkPortTrunkIdSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_BOOL                  memberOfTrunk,
    IN GT_TRUNK_ID              trunkId
)
{
    PRV_UTF_LOG4_MAC("prvTgfTrunkPortTrunkIdSet(%d,%d,%d,%d) \n",
        devNum,
        portNum,
        memberOfTrunk,
        trunkId);

#ifdef EXMXPM_FAMILY
    return cpssExMxPmTrunkPortTrunkIdSet(devNum,portNum,memberOfTrunk,trunkId);
#elif (defined CHX_FAMILY)
    return cpssDxChTrunkPortTrunkIdSet(devNum,portNum,memberOfTrunk,trunkId);
#else
    return GT_NOT_IMPLEMENTED;
#endif
}


/**
* @internal prvTgfTrunkHashIndexCalculate function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         Calculate hash index of member load-balancing group for given hash client.
* @param[in] devNum                   - the device number.
* @param[in] packetType               - packet type.
* @param[in] hashInputDataPtr         - (pointer to) hash input data.
*                                      hashClientType     - client that uses HASH.
*                                      (APPLICABLE VALUES: CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_TRUNK_E,
*                                      CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L2_ECMP_E,
*                                      CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L3_ECMP_E)
* @param[in] numberOfMembers          - number of members in load-balancing group.
*
* @param[out] hashIndexPtr             - (pointer to) calculated by means of ingress
*                                      hash index of load-balancing group member.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or index
* @retval GT_OUT_OF_RANGE          - one of the fields are out of range
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This API is relevant when <Random Enable> flag = GT_FALSE.
*       It determines whether the load balancing is based on the
*       ingress hash or on a 16-bit pseudo-random.
*       The only supported hash mode is Enhanced CRC-based hash mode.
*
*/
GT_STATUS prvTgfTrunkHashIndexCalculate
(
    IN GT_U8                                        devNum,
    IN PRV_TGF_PCL_PACKET_TYPE_ENT                  packetType,
    IN PRV_TGF_TRUNK_LBH_INPUT_DATA_STC             *hashInputDataPtr,
    IN PRV_TGF_TRUNK_HASH_CLIENT_TYPE_ENT           hashEntityType,
    IN GT_U32                                       numberOfMembers,
    OUT GT_U32                                      *hashIndexPtr
)
{
#if (defined CHX_FAMILY)
     CPSS_DXCH_PCL_PACKET_TYPE_ENT                dxPacketType;
     CPSS_DXCH_TRUNK_LBH_INPUT_DATA_STC           dxHashInputData;
     CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT         dxHashEntityType;

    cpssOsMemSet(&dxHashInputData,0,sizeof(dxHashInputData));

     /* set dx packet type */
    switch(packetType)
    {
    case PRV_TGF_PCL_PACKET_TYPE_IPV4_TCP_E:
        dxPacketType = CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E;
        break;
    case PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E:
        dxPacketType = CPSS_DXCH_PCL_PACKET_TYPE_IPV4_UDP_E;
        break;
    case PRV_TGF_PCL_PACKET_TYPE_MPLS_E:
        dxPacketType = CPSS_DXCH_PCL_PACKET_TYPE_MPLS_E;
        break;
    case PRV_TGF_PCL_PACKET_TYPE_IPV4_FRAGMENT_E:
        dxPacketType = CPSS_DXCH_PCL_PACKET_TYPE_IPV4_FRAGMENT_E;
        break;
    case PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E:
        dxPacketType = CPSS_DXCH_PCL_PACKET_TYPE_IPV4_OTHER_E;
        break;
    case PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E:
        dxPacketType = CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E;
        break;
    case PRV_TGF_PCL_PACKET_TYPE_IPV6_E: /* the existing test uses such enum */
        dxPacketType = PRV_TGF_PCL_PACKET_TYPE_IPV6_OTHER_E;/* sip5 need this value in the CPSS */
        break;
    case PRV_TGF_PCL_PACKET_TYPE_IPV6_OTHER_E:
        dxPacketType = CPSS_DXCH_PCL_PACKET_TYPE_IPV6_OTHER_E;
        break;
    case PRV_TGF_PCL_PACKET_TYPE_IPV6_TCP_E:
        dxPacketType = CPSS_DXCH_PCL_PACKET_TYPE_IPV6_TCP_E;
        break;
    case PRV_TGF_PCL_PACKET_TYPE_IPV6_UDP_E:
        dxPacketType = CPSS_DXCH_PCL_PACKET_TYPE_IPV6_UDP_E;
        break;
    case PRV_TGF_PCL_PACKET_TYPE_UDE_E:
        dxPacketType = CPSS_DXCH_PCL_PACKET_TYPE_UDE_E;
        break;
    case PRV_TGF_PCL_PACKET_TYPE_UDE_1_E:
        dxPacketType = CPSS_DXCH_PCL_PACKET_TYPE_UDE1_E;
        break;
    case PRV_TGF_PCL_PACKET_TYPE_UDE_2_E:
        dxPacketType = CPSS_DXCH_PCL_PACKET_TYPE_UDE2_E;
        break;
    case PRV_TGF_PCL_PACKET_TYPE_UDE_3_E:
        dxPacketType = CPSS_DXCH_PCL_PACKET_TYPE_UDE3_E;
        break;
    case PRV_TGF_PCL_PACKET_TYPE_UDE_4_E:
        dxPacketType = CPSS_DXCH_PCL_PACKET_TYPE_UDE4_E;
        break;
    case PRV_TGF_PCL_PACKET_TYPE_UDE5_E:
        dxPacketType = CPSS_DXCH_PCL_PACKET_TYPE_UDE5_E;
        break;
    case PRV_TGF_PCL_PACKET_TYPE_UDE6_E:
        dxPacketType = CPSS_DXCH_PCL_PACKET_TYPE_UDE6_E;
        break;
    default:
        return GT_BAD_PARAM;
    }
    /* set dx hash input data */
     cpssOsMemCpy((GT_VOID*)dxHashInputData.ipDipArray,(GT_VOID*) hashInputDataPtr->ipDipArray,
                 sizeof(dxHashInputData.ipDipArray));
     cpssOsMemCpy((GT_VOID*)dxHashInputData.ipSipArray,(GT_VOID*) hashInputDataPtr->ipSipArray,
                 sizeof(dxHashInputData.ipSipArray));
     cpssOsMemCpy((GT_VOID*)dxHashInputData.ipv6FlowArray,(GT_VOID*) hashInputDataPtr->ipv6FlowArray,
                 sizeof(dxHashInputData.ipv6FlowArray));
     cpssOsMemCpy((GT_VOID*)dxHashInputData.l4DstPortArray,(GT_VOID*) hashInputDataPtr->l4DstPortArray,
                 sizeof(dxHashInputData.l4DstPortArray));
     cpssOsMemCpy((GT_VOID*)dxHashInputData.l4SrcPortArray,(GT_VOID*) hashInputDataPtr->l4SrcPortArray,
                 sizeof(dxHashInputData.l4SrcPortArray));
     cpssOsMemCpy((GT_VOID*)dxHashInputData.localSrcPortArray,(GT_VOID*) hashInputDataPtr->localSrcPortArray,
                 sizeof(dxHashInputData.localSrcPortArray));
     cpssOsMemCpy((GT_VOID*)dxHashInputData.macDaArray,(GT_VOID*) hashInputDataPtr->macDaArray,
                 sizeof(dxHashInputData.macDaArray));
     cpssOsMemCpy((GT_VOID*)dxHashInputData.macSaArray,(GT_VOID*) hashInputDataPtr->macSaArray,
                 sizeof(dxHashInputData.macSaArray));
     cpssOsMemCpy((GT_VOID*)dxHashInputData.mplsLabel0Array,(GT_VOID*) hashInputDataPtr->mplsLabel0Array,
                 sizeof(dxHashInputData.mplsLabel0Array));
     cpssOsMemCpy((GT_VOID*)dxHashInputData.mplsLabel1Array,(GT_VOID*) hashInputDataPtr->mplsLabel1Array,
                 sizeof(dxHashInputData.mplsLabel1Array));
     cpssOsMemCpy((GT_VOID*)dxHashInputData.mplsLabel2Array,(GT_VOID*) hashInputDataPtr->mplsLabel2Array,
                 sizeof(dxHashInputData.mplsLabel2Array));
     cpssOsMemCpy((GT_VOID*)dxHashInputData.udbsArray,(GT_VOID*) hashInputDataPtr->udbsArray,
                 sizeof(dxHashInputData.udbsArray));


    /* set hash entity type */
     switch(hashEntityType)
     {
     case PRV_TGF_TRUNK_HASH_CLIENT_TYPE_TRUNK_E:
         dxHashEntityType = CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_TRUNK_E;
         break;
     case PRV_TGF_TRUNK_HASH_CLIENT_TYPE_L2_ECMP_E:
         dxHashEntityType = CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L2_ECMP_E;
         break;
     case PRV_TGF_TRUNK_HASH_CLIENT_TYPE_L3_ECMP_E:
         dxHashEntityType = CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L3_ECMP_E;
         break;
     default:
         return GT_BAD_PARAM;
     }

    return cpssDxChTrunkHashIndexCalculate(devNum,dxPacketType,&dxHashInputData,dxHashEntityType,numberOfMembers,hashIndexPtr);

#else
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(packetType);
    TGF_PARAM_NOT_USED(hashInputDataPtr);
    TGF_PARAM_NOT_USED(hashEntityType);
    TGF_PARAM_NOT_USED(numberOfMembers);
    TGF_PARAM_NOT_USED(hashIndexPtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTrunkHashNumBitsSet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes.
*         Set the number of bits for client.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number.
* @param[in] hashClient               - hash client (see PRV_TGF_TRUNK_HASH_CLIENT_TYPE_ENT)
* @param[in] startBit                 - start bit (APPLICABLE RANGES: 0..31)
* @param[in] numOfBits                - number of bits (APPLICABLE RANGES: 1..16)
*                                      NOTE: For TRUNK_DESIGNATED_TABLE, IPCL, IOAM, EOAM
* @param[in] numOfBits                must not exceed value 6.
*                                      For Ingress pipe clients (IPCL, IOAM, TRUNK,
*                                      L3ECMP, L2ECMP, TRUNK_DESIGNATED_TABLE)
* @param[in] startBit                 + numOfBits) must not exceed value 32.
*                                      For egress pipe clients (EOAM)
* @param[in] startBit                 + numOfBits) must not exceed value 12.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_OUT_OF_RANGE          - startBit or numOfBits not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTrunkHashNumBitsSet
(
    IN GT_U8                                devNum,
    IN PRV_TGF_TRUNK_HASH_CLIENT_TYPE_ENT   hashClient,
    IN GT_U32                               startBit,
    IN GT_U32                               numOfBits
)
{
#if (defined CHX_FAMILY)
    CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT dxclient;
    switch(hashClient)
    {
        case PRV_TGF_TRUNK_HASH_CLIENT_TYPE_TRUNK_E:
            dxclient = CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_TRUNK_E;
            break;
        case PRV_TGF_TRUNK_HASH_CLIENT_TYPE_L2_ECMP_E:
            dxclient = CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L2_ECMP_E;
            break;
        case PRV_TGF_TRUNK_HASH_CLIENT_TYPE_L3_ECMP_E:
            dxclient = CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L3_ECMP_E;
            break;
        case PRV_TGF_TRUNK_HASH_CLIENT_TYPE_TRUNK_DESIGNATED_TABLE_E:
            dxclient = CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_TRUNK_DESIGNATED_TABLE_E;
            break;
        case PRV_TGF_TRUNK_HASH_CLIENT_TYPE_IPCL_E:
            dxclient = CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_IPCL_E;
            break;
        case PRV_TGF_TRUNK_HASH_CLIENT_TYPE_INGRESS_OAM_E:
            dxclient = CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_INGRESS_OAM_E;
            break;
        case PRV_TGF_TRUNK_HASH_CLIENT_TYPE_EGRESS_OAM_E:
            dxclient = CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_EGRESS_OAM_E;
            break;
        default:
            return GT_BAD_PARAM;
    }
    return cpssDxChTrunkHashNumBitsSet(devNum, dxclient, startBit,
        numOfBits);

#else
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(hashClient);
    TGF_PARAM_NOT_USED(startBit);
    TGF_PARAM_NOT_USED(numOfBits);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfTrunkHashNumBitsGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes.
*         Get the number of bits for client.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number.
* @param[in] hashClient               - hash client (see PRV_TGF_TRUNK_HASH_CLIENT_TYPE_ENT)
* @param[in] startBitPtr              - (pointer to) start bit
* @param[in] numOfBitsPtr             - (pointer to) number of bits
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - NULL pointer to startBitPtr or numOfBitsPtr
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_OUT_OF_RANGE          - startBit or numOfBits not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTrunkHashNumBitsGet
(
    IN  GT_U8                                devNum,
    IN  PRV_TGF_TRUNK_HASH_CLIENT_TYPE_ENT   hashClient,
    OUT GT_U32                               *startBitPtr,
    OUT GT_U32                               *numOfBitsPtr
)
{
#if (defined CHX_FAMILY)
    CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT dxclient;
    switch(hashClient)
    {
        case PRV_TGF_TRUNK_HASH_CLIENT_TYPE_TRUNK_E:
            dxclient = CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_TRUNK_E;
            break;
        case PRV_TGF_TRUNK_HASH_CLIENT_TYPE_L2_ECMP_E:
            dxclient = CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L2_ECMP_E;
            break;
        case PRV_TGF_TRUNK_HASH_CLIENT_TYPE_L3_ECMP_E:
            dxclient = CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L3_ECMP_E;
            break;
        case PRV_TGF_TRUNK_HASH_CLIENT_TYPE_TRUNK_DESIGNATED_TABLE_E:
            dxclient = CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_TRUNK_DESIGNATED_TABLE_E;
            break;
        case PRV_TGF_TRUNK_HASH_CLIENT_TYPE_IPCL_E:
            dxclient = CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_IPCL_E;
            break;
        case PRV_TGF_TRUNK_HASH_CLIENT_TYPE_INGRESS_OAM_E:
            dxclient = CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_INGRESS_OAM_E;
            break;
        case PRV_TGF_TRUNK_HASH_CLIENT_TYPE_EGRESS_OAM_E:
            dxclient = CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_EGRESS_OAM_E;
            break;
        default:
            return GT_BAD_PARAM;
    }
    return cpssDxChTrunkHashNumBitsGet(devNum, dxclient, startBitPtr,
        numOfBitsPtr);

#else
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(hashClient);
    TGF_PARAM_NOT_USED(startBitPtr);
    TGF_PARAM_NOT_USED(numOfBitsPtr);
    return GT_BAD_STATE;
#endif
}


