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
* @file cpssDxChPhaUT.c
*
* @brief Unit tests for cpssDxChPha.h
*
* @version   31
********************************************************************************
*/
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

/* includes */
/* #include <cpss/common/cpssTypes.h> */
#include <cpss/dxCh/dxChxGen/pha/cpssDxChPha.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>


/* check if the SW CPU codes are the same (lead to the same HW value) */
static GT_VOID check_cpuCode(
    IN GT_U8                dev,
    IN  CPSS_NET_RX_CPU_CODE_ENT    cpuCode1,
    IN  CPSS_NET_RX_CPU_CODE_ENT    cpuCode2
)
{
    GT_STATUS            st;
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT dsaCpuCode_1,dsaCpuCode_2;

    if(cpuCode1 == cpuCode2)
    {
        /* already the same value */
        return;
    }

    /* the CPU code can be converted from 'HW' format to other 'SW format'*/
    /* convert SW cpu code to HW cpu code */
    st = prvCpssDxChNetIfCpuToDsaCode(cpuCode1,&dsaCpuCode_1);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    st = prvCpssDxChNetIfCpuToDsaCode(cpuCode2,&dsaCpuCode_2);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

    /* those values MUST be the same !!! */
    UTF_VERIFY_EQUAL1_PARAM_MAC(dsaCpuCode_1,dsaCpuCode_2, dev);
}


/* activate the 'cpssDxChPhaThreadIdEntrySet' and call 'Get' and check it is ok */
static GT_VOID check_cpssDxChPhaThreadIdEntrySet_andGet(
    IN GT_U8                dev,
    IN GT_U32               phaThreadId,
    IN CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC *commonInfoPtr,
    IN CPSS_DXCH_PHA_THREAD_TYPE_ENT    extType,
    IN CPSS_DXCH_PHA_THREAD_INFO_UNT    *extInfoPtr,
    IN GT_STATUS            rc_expected,
    IN GT_BOOL              use_rc_not_expected
)
{
    GT_STATUS            st;
    CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC commonInfoGet;
    CPSS_DXCH_PHA_THREAD_TYPE_ENT    extTypeGet;
    CPSS_DXCH_PHA_THREAD_INFO_UNT    extInfoGet;


    st = cpssDxChPhaThreadIdEntrySet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr);
    if(use_rc_not_expected == GT_FALSE)
    {
        UTF_VERIFY_EQUAL1_PARAM_MAC(rc_expected, st, dev);
    }
    else
    {
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(rc_expected, st, dev);
    }

    if(st != GT_OK)
    {
        return;
    }

    /********************************************************/
    /* check that the Get return the same values as the Set */
    /********************************************************/

    st = cpssDxChPhaThreadIdEntryGet(dev, phaThreadId , &commonInfoGet , &extTypeGet , &extInfoGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

    UTF_VERIFY_EQUAL1_PARAM_MAC(extType, extTypeGet, dev);

    UTF_VERIFY_EQUAL1_PARAM_MAC(commonInfoPtr->statisticalProcessingFactor, commonInfoGet.statisticalProcessingFactor, dev);
    UTF_VERIFY_EQUAL1_PARAM_MAC(commonInfoPtr->busyStallMode,               commonInfoGet.busyStallMode, dev);
    /* the CPU code can be converted from 'HW' format to other 'SW format'*/
    /* convert SW cpu code to HW cpu code */
    check_cpuCode(dev,commonInfoPtr->stallDropCode,commonInfoGet.stallDropCode);

    if(cpssOsMemCmp(extInfoPtr,&extInfoGet,sizeof(extInfoGet)))
    {
        st = GT_BAD_STATE;
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}

/* test common info */
static GT_VOID check_threadCommonInfo(
    IN GT_U8                dev,
    IN CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC *commonInfoPtr,
    IN CPSS_DXCH_PHA_THREAD_TYPE_ENT    extType,
    IN CPSS_DXCH_PHA_THREAD_INFO_UNT    *extInfoPtr
)
{
    GT_STATUS            st;
    GT_U32               phaThreadId;
    CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC    origCommonInfo = *commonInfoPtr;

    /* should be OK */
    phaThreadId = PRV_CPSS_DXCH_PHA_THREAD_ID_MAX_MAC;
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr, GT_OK , GT_FALSE);

    /* check bad parameters */
    phaThreadId++;
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr , GT_BAD_PARAM , GT_FALSE);

    phaThreadId = PRV_CPSS_DXCH_PHA_THREAD_ID_MIN_MAC - 1;
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr , GT_BAD_PARAM , GT_FALSE);

    phaThreadId = PRV_CPSS_DXCH_PHA_THREAD_ID_MAX_MAC;

    /* good parameters */
    commonInfoPtr->statisticalProcessingFactor = BIT_8 - 1;
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr , GT_OK , GT_FALSE);

    /* check out of range valid parameters */
    commonInfoPtr->statisticalProcessingFactor++;
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr , GT_OUT_OF_RANGE , GT_FALSE);
    commonInfoPtr->statisticalProcessingFactor--;


    /* good parameters */
    commonInfoPtr->busyStallMode        = CPSS_DXCH_PHA_BUSY_STALL_MODE_PUSH_BACK_E;
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr , GT_OK , GT_FALSE);

    commonInfoPtr->busyStallMode        = CPSS_DXCH_PHA_BUSY_STALL_MODE_DROP_E;
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr , GT_OK , GT_FALSE);

    commonInfoPtr->busyStallMode        = CPSS_DXCH_PHA_BUSY_STALL_MODE_ORDERED_BYPASS_E;
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr , GT_OK , GT_FALSE);

    /* check bad parameters */
    commonInfoPtr->busyStallMode        ++;
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr , GT_OK , GT_TRUE);

    /*  check bad ENUM values */
    UTF_ENUMS_CHECK_MAC(cpssDxChPhaThreadIdEntrySet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr),
        commonInfoPtr->busyStallMode);

    commonInfoPtr->busyStallMode = CPSS_DXCH_PHA_BUSY_STALL_MODE_PUSH_BACK_E;

    for(commonInfoPtr->stallDropCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E ;
        commonInfoPtr->stallDropCode <= CPSS_NET_LAST_UNKNOWN_HW_CPU_CODE_E ;
        commonInfoPtr->stallDropCode += 17)/* iterate on several values */
    {
        check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr, GT_OK , GT_FALSE);
    }

    /*  check bad ENUM values */
    UTF_ENUMS_CHECK_MAC(cpssDxChPhaThreadIdEntrySet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr),
        commonInfoPtr->stallDropCode);

    commonInfoPtr->stallDropCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E;

    *commonInfoPtr = origCommonInfo;
}

/* test info based on : CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_STC */
static GT_VOID check_CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_STC(
    IN GT_U8                dev,
    IN GT_U32               phaThreadId,
    IN CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC *commonInfoPtr,
    IN CPSS_DXCH_PHA_THREAD_TYPE_ENT    extType,
    IN CPSS_DXCH_PHA_THREAD_INFO_UNT    *extInfoPtr,
    IN CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_STC   *infoPtr
)
{
    infoPtr->IOAM_Trace_Type = BIT_16-1;/* 16 bits value */
    infoPtr->Maximum_Length  =  BIT_8-1;/*  8 bits value */
    infoPtr->Flags           =  BIT_8-1;/*  8 bits value */
    infoPtr->Hop_Lim         =  BIT_8-1;/*  8 bits value */
    infoPtr->node_id         = BIT_24-1;/* 24 bits value */
    infoPtr->Type1           =  BIT_8-1;/*  8 bits value , 'IOAM' */
    infoPtr->IOAM_HDR_len1   =  BIT_8-1;/*  8 bits value */
    infoPtr->Reserved1       =  BIT_8-1;/*  8 bits value */
    infoPtr->Next_Protocol1  =  BIT_8-1;/*  8 bits value */
    infoPtr->Type2           =  BIT_8-1;/*  8 bits value , 'IOAM_E2E' */
    infoPtr->IOAM_HDR_len2   =  BIT_8-1;/*  8 bits value */
    infoPtr->Reserved2       =  BIT_8-1;/*  8 bits value */
    infoPtr->Next_Protocol2  =  BIT_8-1;/*  8 bits value */

    /* check valid parameters */
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr, GT_OK , GT_FALSE);

    /* check common info */
    check_threadCommonInfo(dev, commonInfoPtr , extType , extInfoPtr);

    /* check out of range valid parameters */

    infoPtr->IOAM_Trace_Type++;
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr, GT_OUT_OF_RANGE , GT_FALSE);

    infoPtr->IOAM_Trace_Type--;
    infoPtr->Maximum_Length ++;
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr, GT_OUT_OF_RANGE , GT_FALSE);

    infoPtr->Maximum_Length --;
    infoPtr->Flags          ++;
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr, GT_OUT_OF_RANGE , GT_FALSE);

    infoPtr->Flags          --;
    infoPtr->Hop_Lim        ++;
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr, GT_OUT_OF_RANGE , GT_FALSE);

    infoPtr->Hop_Lim        --;
    infoPtr->node_id        ++;
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr, GT_OUT_OF_RANGE , GT_FALSE);

    infoPtr->node_id        --;
    infoPtr->Type1          ++;
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr, GT_OUT_OF_RANGE , GT_FALSE);

    infoPtr->Type1          --;
    infoPtr->IOAM_HDR_len1  ++;
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr, GT_OUT_OF_RANGE , GT_FALSE);

    infoPtr->IOAM_HDR_len1  --;
    infoPtr->Reserved1      ++;
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr, GT_OUT_OF_RANGE , GT_FALSE);

    infoPtr->Reserved1      --;
    infoPtr->Next_Protocol1 ++;
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr, GT_OUT_OF_RANGE , GT_FALSE);

    infoPtr->Next_Protocol1 --;
    infoPtr->Type2          ++;
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr, GT_OUT_OF_RANGE , GT_FALSE);

    infoPtr->Type2          --;
    infoPtr->IOAM_HDR_len2  ++;
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr, GT_OUT_OF_RANGE , GT_FALSE);

    infoPtr->IOAM_HDR_len2  --;
    infoPtr->Reserved2      ++;
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr, GT_OUT_OF_RANGE , GT_FALSE);

    infoPtr->Reserved2      --;
    infoPtr->Next_Protocol2 ++;
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr, GT_OUT_OF_RANGE , GT_FALSE);

    infoPtr->Next_Protocol2 --;

    /* we must be ok now ! otherwise we may fail cases on other ranges */
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr, GT_OK , GT_FALSE);

}

/* test info based on : CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_STC */
static GT_VOID check_CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_STC(
    IN GT_U8                dev,
    IN GT_U32               phaThreadId,
    IN CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC *commonInfoPtr,
    IN CPSS_DXCH_PHA_THREAD_TYPE_ENT    extType,
    IN CPSS_DXCH_PHA_THREAD_INFO_UNT    *extInfoPtr,
    IN CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_STC   *infoPtr
)
{

    infoPtr->node_id         = BIT_24-1;/* 24 bits value */

    /* check valid parameters */
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr , GT_OK , GT_FALSE);

    /* check common info */
    check_threadCommonInfo(dev, commonInfoPtr , extType , extInfoPtr);

    /* check out of range valid parameters */

    infoPtr->node_id        ++;
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr , GT_OUT_OF_RANGE , GT_FALSE);

    infoPtr->node_id        --;

    /* we must be ok now ! otherwise we may fail cases on other ranges */
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr , GT_OK , GT_FALSE);
}
/*
GT_STATUS cpssDxChPhaThreadIdEntrySet
(
    IN GT_U8                devNum,
    IN GT_U32               phaThreadId,
    IN CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC *commonInfoPtr,
    IN CPSS_DXCH_PHA_THREAD_TYPE_ENT    extType,
    IN CPSS_DXCH_PHA_THREAD_INFO_UNT    *extInfoPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPhaThreadIdEntrySet)
{
    GT_STATUS            st;
    GT_U8                dev;
    GT_U32               phaThreadId;
    CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC commonInfo;
    CPSS_DXCH_PHA_THREAD_TYPE_ENT    extType;
    CPSS_DXCH_PHA_THREAD_INFO_UNT    extInfo;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        phaThreadId = PRV_CPSS_DXCH_PHA_THREAD_ID_MIN_MAC;
        cpssOsMemSet(&commonInfo,0,sizeof(commonInfo));
        /* must set valid 'drop code' otherwise function will fail! */
        commonInfo.stallDropCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E;
        cpssOsMemSet(&extInfo,0,sizeof(extInfo));

        extType = CPSS_DXCH_PHA_THREAD_TYPE_UNUSED_E;
        extInfo.notNeeded = 0x12345678;/* dummy ... not used ... don't care */

        /* check NULL pointers */
        st = cpssDxChPhaThreadIdEntrySet(dev, phaThreadId , NULL , extType , &extInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        st = cpssDxChPhaThreadIdEntrySet(dev, phaThreadId , &commonInfo , extType , NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /* check valid parameters */
        check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , &commonInfo , extType , &extInfo , GT_OK , GT_FALSE);

        cpssOsMemSet(&extInfo,0,sizeof(extInfo));

        /* check type : IOAM ingress switch ipv4 */
        check_CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_STC(dev,
            phaThreadId , &commonInfo ,
            CPSS_DXCH_PHA_THREAD_TYPE_IOAM_INGRESS_SWITCH_IPV4_E , &extInfo ,
            &extInfo.ioamIngressSwitchIpv4);

        /* check type : IOAM ingress switch ipv6 */
        check_CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_STC(dev,
            phaThreadId , &commonInfo ,
            CPSS_DXCH_PHA_THREAD_TYPE_IOAM_INGRESS_SWITCH_IPV6_E , &extInfo ,
            &extInfo.ioamIngressSwitchIpv6);

        cpssOsMemSet(&extInfo,0,sizeof(extInfo));

        /* check type : IOAM transit switch ipv4 */
        check_CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_STC(dev,
            phaThreadId , &commonInfo ,
            CPSS_DXCH_PHA_THREAD_TYPE_IOAM_TRANSIT_SWITCH_IPV4_E , &extInfo ,
            &extInfo.ioamTransitSwitchIpv4);

        /* check type : IOAM transit switch ipv6 */
        check_CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_STC(dev,
            phaThreadId , &commonInfo ,
            CPSS_DXCH_PHA_THREAD_TYPE_IOAM_TRANSIT_SWITCH_IPV6_E , &extInfo ,
            &extInfo.ioamTransitSwitchIpv6);
    }

    phaThreadId = 1;
    cpssOsMemSet(&commonInfo,0,sizeof(commonInfo));
    /* must set valid 'drop code' otherwise function will fail! */
    commonInfo.stallDropCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E;

    extType = CPSS_DXCH_PHA_THREAD_TYPE_UNUSED_E;
    extInfo.notNeeded = 0;/* dummy ... not used ... don't care */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhaThreadIdEntrySet(dev, phaThreadId , &commonInfo , extType , &extInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPhaThreadIdEntrySet(dev, phaThreadId , &commonInfo , extType , &extInfo);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}

/*
GT_STATUS cpssDxChPhaThreadIdEntryGet
(
    IN GT_U8                devNum,
    IN GT_U32               phaThreadId,
    OUT CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC *commonInfoPtr,
    OUT CPSS_DXCH_PHA_THREAD_TYPE_ENT    *extTypePtr,
    OUT CPSS_DXCH_PHA_THREAD_INFO_UNT    *extInfoPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPhaThreadIdEntryGet)
{
    GT_STATUS            st;
    GT_U8                dev;
    GT_U32               phaThreadId;
    CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC commonInfo;
    CPSS_DXCH_PHA_THREAD_TYPE_ENT    extType;
    CPSS_DXCH_PHA_THREAD_INFO_UNT    extInfo;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        phaThreadId = PRV_CPSS_DXCH_PHA_THREAD_ID_MIN_MAC;

        /* check NULL pointers */
        st = cpssDxChPhaThreadIdEntryGet(dev, phaThreadId , NULL , &extType , &extInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        st = cpssDxChPhaThreadIdEntryGet(dev, phaThreadId , &commonInfo , NULL , &extInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        st = cpssDxChPhaThreadIdEntryGet(dev, phaThreadId , &commonInfo , &extType , NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /* check valid parameters */
        st = cpssDxChPhaThreadIdEntryGet(dev, phaThreadId , &commonInfo , &extType , &extInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        phaThreadId = PRV_CPSS_DXCH_PHA_THREAD_ID_MAX_MAC;
        st = cpssDxChPhaThreadIdEntryGet(dev, phaThreadId , &commonInfo , &extType , &extInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* check invalid parameters */
        phaThreadId = PRV_CPSS_DXCH_PHA_THREAD_ID_MAX_MAC + 1;
        st = cpssDxChPhaThreadIdEntryGet(dev, phaThreadId , &commonInfo , &extType , &extInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        phaThreadId = PRV_CPSS_DXCH_PHA_THREAD_ID_MIN_MAC - 1;
        st = cpssDxChPhaThreadIdEntryGet(dev, phaThreadId , &commonInfo , &extType , &extInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

    }

    phaThreadId = 1;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhaThreadIdEntryGet(dev, phaThreadId , &commonInfo , &extType , &extInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPhaThreadIdEntryGet(dev, phaThreadId , &commonInfo , &extType , &extInfo);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}

/*
GT_STATUS cpssDxChPhaPortThreadIdSet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable,
    IN GT_U32       phaThreadId
);
*/
UTF_TEST_CASE_MAC(cpssDxChPhaPortThreadIdSet)
{
    GT_STATUS        st;
    GT_U8            dev;
    GT_PORT_NUM      port;
    GT_BOOL          enable,enableGet;
    GT_U32           phaThreadId,phaThreadIdGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available ePorts. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            enable = GT_FALSE;

            phaThreadId = 0xFFFFFFFF;/* ignored due to enable = GT_FALSE */
            st = cpssDxChPhaPortThreadIdSet(dev,port,enable,phaThreadId);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            st = cpssDxChPhaPortThreadIdGet(dev,port,&enableGet,&phaThreadIdGet);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            UTF_VERIFY_EQUAL0_PARAM_MAC(enable, enableGet);

            enable = GT_TRUE;

            phaThreadId = PRV_CPSS_DXCH_PHA_THREAD_ID_MIN_MAC - 1;
            st = cpssDxChPhaPortThreadIdSet(dev,port,enable,phaThreadId);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);

            phaThreadId = PRV_CPSS_DXCH_PHA_THREAD_ID_MAX_MAC + 1;
            st = cpssDxChPhaPortThreadIdSet(dev,port,enable,phaThreadId);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);

            /* min */
            phaThreadId = PRV_CPSS_DXCH_PHA_THREAD_ID_MIN_MAC;
            st = cpssDxChPhaPortThreadIdSet(dev,port,enable,phaThreadId);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            st = cpssDxChPhaPortThreadIdGet(dev,port,&enableGet,&phaThreadIdGet);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            UTF_VERIFY_EQUAL0_PARAM_MAC(enable, enableGet);
            UTF_VERIFY_EQUAL0_PARAM_MAC(phaThreadId, phaThreadIdGet);

            /* max */
            phaThreadId = PRV_CPSS_DXCH_PHA_THREAD_ID_MIN_MAC;
            st = cpssDxChPhaPortThreadIdSet(dev,port,enable,phaThreadId);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            st = cpssDxChPhaPortThreadIdGet(dev,port,&enableGet,&phaThreadIdGet);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            UTF_VERIFY_EQUAL0_PARAM_MAC(enable, enableGet);
            UTF_VERIFY_EQUAL0_PARAM_MAC(phaThreadId, phaThreadIdGet);
        }

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available ePorts. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            enable = GT_TRUE;
            phaThreadId = PRV_CPSS_DXCH_PHA_THREAD_ID_MIN_MAC;
            st = cpssDxChPhaPortThreadIdSet(dev,port,enable,phaThreadId);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);
        }
    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* go over all non active devices */
    port = 0;
    enable = GT_FALSE;
    phaThreadId = 1;
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhaPortThreadIdSet(dev,port,enable,phaThreadId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPhaPortThreadIdSet(dev,port,enable,phaThreadId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChPhaPortThreadIdGet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    OUT GT_BOOL     *enablePtr,
    OUT GT_U32      *phaThreadIdPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPhaPortThreadIdGet)
{
    GT_STATUS        st;
    GT_U8            dev;
    GT_PORT_NUM      port;
    GT_BOOL          enable;
    GT_U32           phaThreadId;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available ePorts. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            st = cpssDxChPhaPortThreadIdGet(dev,port,&enable,&phaThreadId);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* check NULL pointer */
            st = cpssDxChPhaPortThreadIdGet(dev,port,NULL,&phaThreadId);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

            st = cpssDxChPhaPortThreadIdGet(dev,port,&enable,NULL);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);
        }

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available ePorts. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPhaPortThreadIdGet(dev,port,&enable,&phaThreadId);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);
        }
    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* go over all non active devices */
    port = 0;
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhaPortThreadIdGet(dev,port,&enable,&phaThreadId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPhaPortThreadIdGet(dev,port,&enable,&phaThreadId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*
GT_STATUS cpssDxChPhaPortThreadIdSet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT  infoType,
    IN CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT  *portInfoPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPhaSourcePortEntrySet)
{
    GT_STATUS        st;
    GT_U8            dev;
    GT_PHYSICAL_PORT_NUM      port;
    CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT infoType,infoTypeGet;
    CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT portInfo,portInfoGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available ePorts. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            infoType = CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNUSED_E;
            portInfo = 15;/* dummy */

            st = cpssDxChPhaSourcePortEntrySet(dev,port,infoType,&portInfo);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            st = cpssDxChPhaSourcePortEntryGet(dev,port,&infoTypeGet,&portInfoGet);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            UTF_VERIFY_EQUAL0_PARAM_MAC(infoType, infoTypeGet);
            UTF_VERIFY_EQUAL0_PARAM_MAC(portInfo, portInfoGet);

            /* check NULL pointer */
            st = cpssDxChPhaSourcePortEntrySet(dev,port,infoType,NULL);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available ePorts. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            infoType = CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNUSED_E;
            portInfo = 15;/* dummy */
            st = cpssDxChPhaSourcePortEntrySet(dev,port,infoType,&portInfo);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);
        }
    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* go over all non active devices */
    port = 0;
    infoType = CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNUSED_E;
    portInfo = 15;/* dummy */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhaSourcePortEntrySet(dev,port,infoType,&portInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPhaSourcePortEntrySet(dev,port,infoType,&portInfo);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChPhaSourcePortEntryGet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT  *infoTypePtr,
    OUT CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT  *portInfoPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPhaSourcePortEntryGet)
{
    GT_STATUS        st;
    GT_U8            dev;
    GT_PHYSICAL_PORT_NUM      port;
    CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT infoType;
    CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT portInfo;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available ePorts. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            st = cpssDxChPhaSourcePortEntryGet(dev,port,&infoType,&portInfo);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* check NULL pointer */
            st = cpssDxChPhaSourcePortEntryGet(dev,port,&infoType,NULL);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

            st = cpssDxChPhaSourcePortEntryGet(dev,port,NULL,&portInfo);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available ePorts. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPhaSourcePortEntryGet(dev,port,&infoType,&portInfo);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);
        }
    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* go over all non active devices */
    port = 0;
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhaSourcePortEntryGet(dev,port,&infoType,&portInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPhaSourcePortEntryGet(dev,port,&infoType,&portInfo);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChPhaPortThreadIdSet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT  infoType,
    IN CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT  *portInfoPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPhaTargetPortEntrySet)
{
    GT_STATUS        st;
    GT_U8            dev;
    GT_PHYSICAL_PORT_NUM      port;
    CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT infoType,infoTypeGet;
    CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT portInfo,portInfoGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available ePorts. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            infoType = CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNUSED_E;
            portInfo = 15;/* dummy */

            st = cpssDxChPhaTargetPortEntrySet(dev,port,infoType,&portInfo);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            st = cpssDxChPhaTargetPortEntryGet(dev,port,&infoTypeGet,&portInfoGet);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            UTF_VERIFY_EQUAL0_PARAM_MAC(infoType, infoTypeGet);
            UTF_VERIFY_EQUAL0_PARAM_MAC(portInfo, portInfoGet);

            /* check NULL pointer */
            st = cpssDxChPhaTargetPortEntrySet(dev,port,infoType,NULL);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available ePorts. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            infoType = CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNUSED_E;
            portInfo = 15;/* dummy */
            st = cpssDxChPhaTargetPortEntrySet(dev,port,infoType,&portInfo);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);
        }
    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* go over all non active devices */
    port = 0;
    infoType = CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNUSED_E;
    portInfo = 15;/* dummy */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhaTargetPortEntrySet(dev,port,infoType,&portInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPhaTargetPortEntrySet(dev,port,infoType,&portInfo);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChPhaTargetPortEntryGet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT  *infoTypePtr,
    OUT CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT  *portInfoPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPhaTargetPortEntryGet)
{
    GT_STATUS        st;
    GT_U8            dev;
    GT_PHYSICAL_PORT_NUM      port;
    CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT infoType;
    CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT portInfo;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available ePorts. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            st = cpssDxChPhaTargetPortEntryGet(dev,port,&infoType,&portInfo);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* check NULL pointer */
            st = cpssDxChPhaTargetPortEntryGet(dev,port,&infoType,NULL);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

            st = cpssDxChPhaTargetPortEntryGet(dev,port,NULL,&portInfo);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available ePorts. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPhaTargetPortEntryGet(dev,port,&infoType,&portInfo);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);
        }
    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* go over all non active devices */
    port = 0;
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhaTargetPortEntryGet(dev,port,&infoType,&portInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPhaTargetPortEntryGet(dev,port,&infoType,&portInfo);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*
GT_STATUS cpssDxChPhaHeaderModificationViolationInfoSet
(
    IN  GT_U8                       devNum,
    IN  CPSS_NET_RX_CPU_CODE_ENT    dropCode,
    IN  CPSS_PACKET_CMD_ENT         packetCommand
);
*/
UTF_TEST_CASE_MAC(cpssDxChPhaHeaderModificationViolationInfoSet)
{
    GT_STATUS        st;
    GT_U8            dev;
    CPSS_NET_RX_CPU_CODE_ENT    dropCode,dropCodeGet;
    CPSS_PACKET_CMD_ENT         packetCommand,packetCommandGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        dropCode      = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E;
        packetCommand = CPSS_PACKET_CMD_DROP_HARD_E;

        st = cpssDxChPhaHeaderModificationViolationInfoSet(dev,dropCode,packetCommand);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChPhaHeaderModificationViolationInfoGet(dev,&dropCodeGet,&packetCommandGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        /* the CPU code can be converted from 'HW' format to other 'SW format'*/
        /* convert SW cpu code to HW cpu code */
        check_cpuCode(dev,dropCode,dropCodeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(packetCommand, packetCommandGet, dev);


        for(dropCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E ;
            dropCode <= CPSS_NET_LAST_UNKNOWN_HW_CPU_CODE_E ;
            dropCode += 19)/* iterate on several values */
        {
            st = cpssDxChPhaHeaderModificationViolationInfoSet(dev,dropCode,packetCommand);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        /*  check bad ENUM values */
        UTF_ENUMS_CHECK_MAC(cpssDxChPhaHeaderModificationViolationInfoSet(dev,dropCode,packetCommand),
            dropCode);

        dropCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E + 111;

        for(packetCommand = CPSS_PACKET_CMD_FORWARD_E ;
            packetCommand <= CPSS_PACKET_CMD_DEFAULT_ROUTE_ENTRY_E;
            packetCommand++)
        {
            st = cpssDxChPhaHeaderModificationViolationInfoSet(dev,dropCode,packetCommand);
            if(packetCommand == CPSS_PACKET_CMD_FORWARD_E ||
               packetCommand == CPSS_PACKET_CMD_DROP_HARD_E)
            {
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                st = cpssDxChPhaHeaderModificationViolationInfoGet(dev,&dropCodeGet,&packetCommandGet);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                /* the CPU code can be converted from 'HW' format to other 'SW format'*/
                /* convert SW cpu code to HW cpu code */
                check_cpuCode(dev,dropCode,dropCodeGet);
                UTF_VERIFY_EQUAL1_PARAM_MAC(packetCommand, packetCommandGet, dev);
            }
            else
            {
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
            }
        }
    }
    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* go over all non active devices */
    dropCode      = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E;
    packetCommand = CPSS_PACKET_CMD_FORWARD_E;
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhaHeaderModificationViolationInfoSet(dev,dropCode,packetCommand);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPhaHeaderModificationViolationInfoSet(dev,dropCode,packetCommand);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChPhaHeaderModificationViolationInfoGet
(
    IN  GT_U8                       devNum,
    OUT CPSS_NET_RX_CPU_CODE_ENT    *dropCodePtr,
    OUT CPSS_PACKET_CMD_ENT         *packetCommandPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPhaHeaderModificationViolationInfoGet)
{
    GT_STATUS        st;
    GT_U8            dev;
    CPSS_NET_RX_CPU_CODE_ENT    dropCode;
    CPSS_PACKET_CMD_ENT         packetCommand;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssDxChPhaHeaderModificationViolationInfoGet(dev,&dropCode,&packetCommand);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* check NULL pointer */
        st = cpssDxChPhaHeaderModificationViolationInfoGet(dev,&dropCode,NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        st = cpssDxChPhaHeaderModificationViolationInfoGet(dev,NULL,&packetCommand);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }
    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhaHeaderModificationViolationInfoGet(dev,&dropCode,&packetCommand);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPhaHeaderModificationViolationInfoGet(dev,&dropCode,&packetCommand);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChPhaHeaderModificationViolationCapturedGet
(
    IN  GT_U8                       devNum,
    OUT GT_U32                     *capturedThreadIdPtr,
    OUT CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_ENT  *violationTypePtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPhaHeaderModificationViolationCapturedGet)
{
    GT_STATUS        st;
    GT_U8            dev;
    GT_U32                      capturedThreadId;
    CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_ENT  violationType;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        do{
            st = cpssDxChPhaHeaderModificationViolationCapturedGet(dev,&capturedThreadId,&violationType);
            if(st == GT_NO_MORE)
            {
                /* this is valid value ! ... there was no violation to report about */
                break;
            }
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }while(st == GT_OK);

        /* check NULL pointer */
        st = cpssDxChPhaHeaderModificationViolationCapturedGet(dev,&capturedThreadId,NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        st = cpssDxChPhaHeaderModificationViolationCapturedGet(dev,NULL,&violationType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }
    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhaHeaderModificationViolationCapturedGet(dev,&capturedThreadId,&violationType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPhaHeaderModificationViolationCapturedGet(dev,&capturedThreadId,&violationType);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*
GT_STATUS cpssDxChPhaStatisticalProcessingCounterThreadIdSet
(
    IN GT_U8        devNum,
    IN GT_U32       phaThreadId
);
*/
UTF_TEST_CASE_MAC(cpssDxChPhaStatisticalProcessingCounterThreadIdSet)
{
    GT_STATUS        st;
    GT_U8            dev;
    GT_U32           phaThreadId,phaThreadIdGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        phaThreadId = PRV_CPSS_DXCH_PHA_THREAD_ID_MIN_MAC;
        st = cpssDxChPhaStatisticalProcessingCounterThreadIdSet(dev,phaThreadId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChPhaStatisticalProcessingCounterThreadIdGet(dev,&phaThreadIdGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(phaThreadId, phaThreadIdGet, dev);

        phaThreadId = PRV_CPSS_DXCH_PHA_THREAD_ID_MAX_MAC;
        st = cpssDxChPhaStatisticalProcessingCounterThreadIdSet(dev,phaThreadId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChPhaStatisticalProcessingCounterThreadIdGet(dev,&phaThreadIdGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(phaThreadId, phaThreadIdGet, dev);

        phaThreadId = 0;
        st = cpssDxChPhaStatisticalProcessingCounterThreadIdSet(dev,phaThreadId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChPhaStatisticalProcessingCounterThreadIdGet(dev,&phaThreadIdGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(phaThreadId, phaThreadIdGet, dev);

        phaThreadId = PRV_CPSS_DXCH_PHA_THREAD_ID_MAX_MAC + 1;
        st = cpssDxChPhaStatisticalProcessingCounterThreadIdSet(dev,phaThreadId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }
    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    phaThreadId = PRV_CPSS_DXCH_PHA_THREAD_ID_MIN_MAC;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhaStatisticalProcessingCounterThreadIdSet(dev,phaThreadId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPhaStatisticalProcessingCounterThreadIdSet(dev,phaThreadId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChPhaStatisticalProcessingCounterThreadIdGet
(
    IN  GT_U8        devNum,
    OUT GT_U32       *phaThreadIdPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPhaStatisticalProcessingCounterThreadIdGet)
{
    GT_STATUS        st;
    GT_U8            dev;
    GT_U32           phaThreadId;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssDxChPhaStatisticalProcessingCounterThreadIdGet(dev,&phaThreadId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* check NULL pointer */
        st = cpssDxChPhaStatisticalProcessingCounterThreadIdGet(dev,NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

    }
    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhaStatisticalProcessingCounterThreadIdGet(dev,&phaThreadId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPhaStatisticalProcessingCounterThreadIdGet(dev,&phaThreadId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChPhaStatisticalProcessingCounterGet
(
    IN  GT_U8        devNum,
    OUT GT_U64       *counterPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPhaStatisticalProcessingCounterGet)
{
    GT_STATUS        st;
    GT_U8            dev;
    GT_U64           counter;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssDxChPhaStatisticalProcessingCounterGet(dev,&counter);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* check NULL pointer */
        st = cpssDxChPhaStatisticalProcessingCounterGet(dev,NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

    }
    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhaStatisticalProcessingCounterGet(dev,&counter);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPhaStatisticalProcessingCounterGet(dev,&counter);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChPhaInit
(
    IN  GT_U8   devNum,
    IN GT_BOOL  packetOrderChangeEnable
);
*/
UTF_TEST_CASE_MAC(cpssDxChPhaInit)
{
    GT_STATUS        st;
    GT_U8            dev;
    GT_BOOL  packetOrderChangeEnable = GT_TRUE;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 'already exists' the 'appDemo' already initialized the PHA */
        st = cpssDxChPhaInit(dev,packetOrderChangeEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_ALREADY_EXIST, st, dev);
    }
    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhaInit(dev,packetOrderChangeEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPhaInit(dev,packetOrderChangeEnable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChPha suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChPha)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhaThreadIdEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhaThreadIdEntryGet)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhaPortThreadIdSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhaPortThreadIdGet)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhaSourcePortEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhaSourcePortEntryGet)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhaTargetPortEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhaTargetPortEntryGet)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhaHeaderModificationViolationInfoSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhaHeaderModificationViolationInfoGet)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhaHeaderModificationViolationCapturedGet)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhaStatisticalProcessingCounterThreadIdSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhaStatisticalProcessingCounterThreadIdGet)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhaStatisticalProcessingCounterGet)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhaInit)
UTF_SUIT_END_TESTS_MAC(cpssDxChPha)

