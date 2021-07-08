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
* @file cpssDxChExactMatchUT.c
*
* @brief Unit tests for cpssDxChExactMatch, that provides
* Exact Match CPSS Falcon implementation.
*
* @version   1
********************************************************************************
*/
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/dxCh/dxChxGen/exactMatch/cpssDxChExactMatch.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/* Defines */

/* Default valid value for port/trunk id */
#define TTI_VALID_PORT_TRUNK_CNS        20

/* Default valid value for vlan id */
#define TTI_VALID_VLAN_ID_CNS           100


/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchTtiProfileIdSet)
{
/*
    ITERATE_DEVICES (Falcon)
    1.1. Call function with keyType [CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E /
                                     CPSS_DXCH_TTI_KEY_UDB_IPV6_E /
                                     CPSS_DXCH_TTI_KEY_UDB_UDE6_E],
                           ttiLookupNum [first / second ]
                           and profileId [1,6,15]
    Expected: GT_OK.
    1.2. Call cpssDxChExactMatchTtiProfileIdGet.
    Expected: GT_OK and the same profileId.
    1.3. Call with wrong ttiLookupNum [last]
    Expected: GT_BAD_PARAM.
    1.4. Call with wrong profileId [16]
    Expected: GT_OUT_OF_RANGE.
    1.5. Call with NULL valuePtr and other valid params.
    Expected: GT_BAD_PTR.
    1.6 Call cpssDxChExactMatchTtiProfileIdSet with wrong keyType vlaue.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                           st = GT_OK;
    GT_U8                               devNum;
    CPSS_DXCH_TTI_KEY_TYPE_ENT          keyType;
    CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT    ttiLookupNum;
    GT_BOOL                             enableExactMatchLookup;
    GT_BOOL                             enableExactMatchLookupGet;
    GT_U32                              profileId;
    GT_U32                              profileIdGet;


    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_FALCON_E) ;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* define clients lookup to be TTI*/
        ttiLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
        st = cpssDxChExactMatchClientLookupSet(0,ttiLookupNum,CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, ttiLookupNum);
        ttiLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
        st = cpssDxChExactMatchClientLookupSet(0,ttiLookupNum,CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, ttiLookupNum);


        /*
           1.1.1 Call function with keyType [CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E]
                                   ttiLookupNum [first] and profileId [1]
           Expected: GT_OK.
        */
        keyType = CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E;
        ttiLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
        profileId   = 1;
        enableExactMatchLookup = GT_TRUE;
        st = cpssDxChExactMatchTtiProfileIdSet(devNum, keyType, ttiLookupNum ,enableExactMatchLookup, profileId);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, devNum, keyType, ttiLookupNum ,enableExactMatchLookup, profileId);
        /*
            1.2.1 Call cpssDxChExactMatchTtiProfileIdGet.
            Expected: GT_OK and the same profileId.
        */
        st = cpssDxChExactMatchTtiProfileIdGet(devNum, keyType, ttiLookupNum ,&enableExactMatchLookupGet, &profileIdGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
            "cpssDxChExactMatchTtiProfileIdGet: %d, %d , %d", devNum, keyType,ttiLookupNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(enableExactMatchLookup, enableExactMatchLookupGet,
            "got other enableExactMatchLookupGet then was set: %d", devNum);
        if (enableExactMatchLookup==enableExactMatchLookupGet)
        {
            if(enableExactMatchLookupGet==GT_TRUE)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(profileId, profileIdGet,
                                             "got other profileId then was set: %d", devNum);
            }
            else
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(0, profileIdGet,
                                             "got other profileId then was set: %d", devNum);
            }
        }

        /*
           1.1.2 Call function with keyType [CPSS_DXCH_TTI_KEY_UDB_IPV6_E]
                                   ttiLookupNum [second] and profileId [6]
           Expected: GT_OK.
        */
        keyType = CPSS_DXCH_TTI_KEY_UDB_IPV6_E;
        ttiLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
        profileId   = 6;
        enableExactMatchLookup = GT_TRUE;
        st = cpssDxChExactMatchTtiProfileIdSet(devNum, keyType, ttiLookupNum ,enableExactMatchLookup,profileId);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, devNum, keyType, ttiLookupNum ,enableExactMatchLookup,profileId);
        /*
            1.2.2 Call cpssDxChExactMatchTtiProfileIdGet.
            Expected: GT_OK and the same profileId.
        */
        st = cpssDxChExactMatchTtiProfileIdGet(devNum, keyType, ttiLookupNum ,&enableExactMatchLookupGet, &profileIdGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
            "cpssDxChExactMatchTtiProfileIdGet: %d, %d , %d", devNum, keyType,ttiLookupNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(profileId, profileIdGet,
            "got other profileId then was set: %d", devNum);
        /*
           1.1.3 Call function with keyType [CPSS_DXCH_TTI_KEY_UDB_UDE6_E]
                                   ttiLookupNum [first] and profileId [15]
           Expected: GT_OK.
        */
        keyType = CPSS_DXCH_TTI_KEY_UDB_UDE6_E;
        ttiLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
        profileId   = 15;
        st = cpssDxChExactMatchTtiProfileIdSet(devNum, keyType, ttiLookupNum ,enableExactMatchLookup, profileId);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, devNum, keyType, ttiLookupNum ,enableExactMatchLookup, profileId);
        /*
            1.2.3 Call cpssDxChExactMatchTtiProfileIdGet.
            Expected: GT_OK and the same profileId.
        */
        st = cpssDxChExactMatchTtiProfileIdGet(devNum, keyType, ttiLookupNum ,&enableExactMatchLookupGet, &profileIdGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
            "cpssDxChExactMatchTtiProfileIdGet: %d, %d , %d", devNum, keyType,ttiLookupNum);
       if (enableExactMatchLookup==enableExactMatchLookupGet)
        {
            if(enableExactMatchLookupGet==GT_TRUE)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(profileId, profileIdGet,
                                             "got other profileId then was set: %d", devNum);
            }
            else
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(0, profileIdGet,
                                             "got other profileId then was set: %d", devNum);
            }
        }
        /*
            1.3. Call with wrong ttiLookupNum [last]
            Expected: GT_BAD_PARAM.
        */
        keyType = CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E;
        ttiLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E;
        profileId   = 1;
        enableExactMatchLookup = GT_TRUE;
        st = cpssDxChExactMatchTtiProfileIdSet(devNum, keyType, ttiLookupNum ,enableExactMatchLookup, profileId);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, devNum, keyType, ttiLookupNum ,profileId);
        /*
            1.4. Call with wrong profileId [16]
            Expected: GT_OUT_OF_RANGE.
        */
        ttiLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
        profileId   = 16;
        enableExactMatchLookup = GT_TRUE;
        st = cpssDxChExactMatchTtiProfileIdSet(devNum, keyType, ttiLookupNum ,enableExactMatchLookup, profileId);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OUT_OF_RANGE, st, devNum, keyType, ttiLookupNum ,profileId);

        /* Restore valid params. */
        keyType = CPSS_DXCH_TTI_KEY_UDB_UDE6_E;
        ttiLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
        profileId   = 2;
        enableExactMatchLookup = GT_TRUE;
        /*
            1.5. Call with NULL valuePtr and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChExactMatchTtiProfileIdGet(devNum, keyType, ttiLookupNum ,&enableExactMatchLookup, NULL);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PTR, st, "%d, %d, profileId = NULL",
                                     devNum, keyType,ttiLookupNum);
        /*
            1.6 Call cpssDxChExactMatchTtiProfileIdSet with wrong keyType vlaue.
            Expected: GT_BAD_PARAM.
        */
        keyType = CPSS_DXCH_TTI_KEY_IPV4_E;
        st = cpssDxChExactMatchTtiProfileIdSet(devNum, keyType, ttiLookupNum ,enableExactMatchLookup, profileId);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_BAD_PARAM, st, devNum, keyType, ttiLookupNum ,enableExactMatchLookup, profileId);
        st = cpssDxChExactMatchTtiProfileIdGet(devNum, keyType, ttiLookupNum ,&enableExactMatchLookupGet, &profileIdGet);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_BAD_PARAM, st, devNum, keyType, ttiLookupNum , &enableExactMatchLookupGet, profileIdGet);
    }


    /* 2. For not active devices check that function returns non GT_NOT_APPLICABLE_DEVICE.*/
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_FALCON_E ) ;
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        keyType = CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E;
        ttiLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
        profileId   = 1;
        enableExactMatchLookup = GT_TRUE;
        st = cpssDxChExactMatchTtiProfileIdSet(devNum, keyType, ttiLookupNum ,enableExactMatchLookup, profileId);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum, keyType, ttiLookupNum ,enableExactMatchLookup, profileId);
        st = cpssDxChExactMatchTtiProfileIdGet(devNum, keyType, ttiLookupNum ,&enableExactMatchLookupGet, &profileIdGet);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum, keyType, ttiLookupNum ,enableExactMatchLookupGet, profileIdGet);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchTtiProfileIdGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT          keyType,
    IN  CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT    ttiLookupNum,
    OUT GT_U32                             *profileIdPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchTtiProfileIdGet)
{
/*
    ITERATE_DEVICES (Falcon)
    ITERATE_DEVICES (Falcon)
    1.1. Call function with keyType [CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E /
                                     CPSS_DXCH_TTI_KEY_UDB_IPV6_E /
                                     CPSS_DXCH_TTI_KEY_UDB_UDE6_E],
                           ttiLookupNum [first / second ]
    Expected: GT_OK.
    1.2. Call cpssDxChExactMatchTtiProfileIdGet.
    1.2. Call with out of range keyType and other valid params.
    Expected: NOT GT_OK.
    1.3. Call with out of range ttiLookupNum and other valid params.
    Expected: NOT GT_OK.
    1.4. Call with NULL profileIdPtr and other valid params.
    Expected: GT_BAD_PTR.
    1.5. Call with NULL enableExactMatchLookup and other valid params.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                           st = GT_OK;
    GT_U8                               devNum;
    CPSS_DXCH_TTI_KEY_TYPE_ENT          keyType;
    CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT    ttiLookupNum;
    GT_U32                              profileId;
    GT_BOOL                             enableExactMatchLookup;


    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_FALCON_E) ;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* define clients lookup to be TTI*/
        ttiLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
        st = cpssDxChExactMatchClientLookupSet(0,ttiLookupNum,CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, ttiLookupNum);
        ttiLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
        st = cpssDxChExactMatchClientLookupSet(0,ttiLookupNum,CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, ttiLookupNum);

        /*
            1.1. Call function with keyType [CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E /
                                     CPSS_DXCH_TTI_KEY_UDB_IPV6_E /
                                     CPSS_DXCH_TTI_KEY_UDB_UDE6_E],
                           ttiLookupNum [first / second ]
            Expected: GT_OK.
        */
        /* iterate with entryIndex = 0 */
        keyType = CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E;
        ttiLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;

        st = cpssDxChExactMatchTtiProfileIdGet(devNum, keyType, ttiLookupNum ,&enableExactMatchLookup, &profileId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyType);

        /*
             1.2. Call with out of range keyType and other valid params.
            Expected: NOT GT_OK.
        */

        keyType = (CPSS_DXCH_TTI_KEY_TYPE_ENT)20;

        st = cpssDxChExactMatchTtiProfileIdGet(devNum, keyType, ttiLookupNum ,&enableExactMatchLookup, &profileId);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyType);

         /*
           1.3. Call with out of range ttiLookupNum and other valid params.
            Expected: NOT GT_OK.
         */

        keyType = CPSS_DXCH_TTI_KEY_UDB_IPV6_E;
        ttiLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E;

        st = cpssDxChExactMatchTtiProfileIdGet(devNum, keyType, ttiLookupNum ,&enableExactMatchLookup, &profileId);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyType);

        /*
            1.4. Call with NULL profileIdPtr and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChExactMatchTtiProfileIdGet(devNum, keyType, ttiLookupNum ,&enableExactMatchLookup, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, profileIdPtr = NULL",
                                     devNum, keyType);

        /*
            1.5. Call with NULL enableExactMatchLookup and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChExactMatchTtiProfileIdGet(devNum, keyType, ttiLookupNum ,NULL, &profileId);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, enableExactMatchLookup = NULL",
                                     devNum, keyType);

     }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    keyType = CPSS_DXCH_TTI_KEY_UDB_UDE_E;
    ttiLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_FALCON_E ) ;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChExactMatchTtiProfileIdGet(devNum, keyType, ttiLookupNum ,&enableExactMatchLookup, &profileId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChExactMatchTtiProfileIdGet(devNum, keyType, ttiLookupNum ,&enableExactMatchLookup, &profileId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchPclProfileIdSet
(
   IN  GT_U8                               devNum,
   IN  CPSS_DXCH_PCL_PACKET_TYPE_ENT       packetType,
   IN  CPSS_PCL_DIRECTION_ENT              direction,
   IN  GT_U32                              subProfileId,
   IN  CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT    pclLookupNum,
   IN  GT_U32                              profileId
)
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchPclProfileIdSet)
{
/*
    ITERATE_DEVICES (Falcon)
    1.1. Call function with valid parameters
         Expected: GT_OK.
    1.2  Call cpssDxChExactMatchPclProfileIdGet with non-NULL profileIdPtr.
         Expected: GT_OK and the same profileId .
    2.1  Call function with direction [CPSS_PCL_DIRECTION_EGRESS_E]
         packetType [CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E ,CPSS_DXCH_PCL_PACKET_TYPE_IPV4_UDP_E]
         pclLookupNum  [first,second] and same subProfileid
         Expected: GT_OK.
    2.2  Call cpssDxChTtiExactMatchProfileIdGet.
         Expected: GT_OK and the same profileIds.
    3.1.  Call with wrong pclLookupNum [last]
         Expected: GT_BAD_PARAM.
    3.2. Call with wrong profileId [17]
         Expected: GT_OUT_OF_RANGE.
    3.3. Call with wrong subProfileId [8]
         Expected: GT_BAD_PARAM.
*/
   GT_STATUS                           st;
   GT_U8                               devNum;
   CPSS_DXCH_PCL_PACKET_TYPE_ENT       packetType;
   CPSS_PCL_DIRECTION_ENT              direction;
   GT_U32                              subProfileId;
   CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT    pclLookupNum;
   GT_BOOL                             enableExactMatchLookup;
   GT_BOOL                             enableExactMatchLookupGet;
   GT_U32                              profileId;
   GT_U32                              profileIdGet;


    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_FALCON_E ) ;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* define clients lookup to be TTI*/
        pclLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
        st = cpssDxChExactMatchClientLookupSet(0,pclLookupNum,CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_0_E);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, pclLookupNum);
        pclLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
        st = cpssDxChExactMatchClientLookupSet(0,pclLookupNum,CPSS_DXCH_EXACT_MATCH_CLIENT_EPCL_E);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, pclLookupNum);

        /*
           1.1.1 Call function with valid parameters
           Expected: GT_OK.
        */
        packetType = CPSS_DXCH_PCL_PACKET_TYPE_MPLS_E;
        direction = CPSS_PCL_DIRECTION_INGRESS_E ;
        subProfileId = 3;
        pclLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
        profileId   = 15;
        enableExactMatchLookup = GT_TRUE;
        st = cpssDxChExactMatchPclProfileIdSet(devNum, direction ,packetType, subProfileId, pclLookupNum ,enableExactMatchLookup, profileId);
        UTF_VERIFY_EQUAL7_PARAM_MAC(GT_OK, st, devNum, direction ,packetType, subProfileId, pclLookupNum ,enableExactMatchLookup, profileId);
        /*
            1.2.1 Call cpssDxChTtiExactMatchProfileIdGet.
            Expected: GT_OK and the same profileId.
        */
        st = cpssDxChExactMatchPclProfileIdGet(devNum, direction ,packetType, subProfileId, pclLookupNum ,&enableExactMatchLookupGet, &profileIdGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(enableExactMatchLookup, enableExactMatchLookupGet,
            "got other profileId then was set: %d", devNum);

        if ((enableExactMatchLookup==enableExactMatchLookupGet)&&
            (enableExactMatchLookup==GT_TRUE))
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(profileId, profileIdGet,
            "got other profileId then was set: %d", devNum);
        }

        /*
           1.1.2 Call function with direction [CPSS_PCL_DIRECTION_EGRESS_E] and same parameters
           Expected: GT_OK.
        */
        direction = CPSS_PCL_DIRECTION_EGRESS_E ;
        pclLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
        st = cpssDxChExactMatchPclProfileIdSet(devNum,direction , packetType, subProfileId, pclLookupNum ,enableExactMatchLookup, profileId);
        UTF_VERIFY_EQUAL7_PARAM_MAC(GT_OK, st, devNum,direction , packetType, subProfileId, pclLookupNum ,enableExactMatchLookup, profileId);
        /*
            1.2.2 Call cpssDxChTtiExactMatchProfileIdGet.
            Expected: GT_OK and the same profileId.
        */
        st = cpssDxChExactMatchPclProfileIdGet(devNum, direction, packetType, subProfileId, pclLookupNum ,&enableExactMatchLookupGet, &profileIdGet);
       UTF_VERIFY_EQUAL1_STRING_MAC(enableExactMatchLookup, enableExactMatchLookupGet,
            "got other profileId then was set: %d", devNum);

        if ((enableExactMatchLookup==enableExactMatchLookupGet)&&
            (enableExactMatchLookup==GT_TRUE))
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(profileId, profileIdGet,"got other profileId then was set: %d", devNum);
        }

         /*
           2.1 Call function with direction [CPSS_PCL_DIRECTION_EGRESS_E]
               packetType [CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E ,CPSS_DXCH_PCL_PACKET_TYPE_IPV4_UDP_E]
               pclLookupNum  [first,second]
               and same subProfileid
           Expected: GT_OK.
        */
        packetType = CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E;
        direction = CPSS_PCL_DIRECTION_EGRESS_E ;
        subProfileId = 7;
        pclLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
        profileId   = 9;

        st = cpssDxChExactMatchPclProfileIdSet(devNum, direction ,packetType, subProfileId, pclLookupNum ,enableExactMatchLookup, profileId);
        UTF_VERIFY_EQUAL7_PARAM_MAC(GT_OK, st, devNum, direction ,packetType, subProfileId, pclLookupNum ,enableExactMatchLookup, profileId);

        packetType = CPSS_DXCH_PCL_PACKET_TYPE_IPV4_UDP_E;
        pclLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
        profileId   = 12;

        st = cpssDxChExactMatchPclProfileIdSet(devNum, direction ,packetType, subProfileId, pclLookupNum ,enableExactMatchLookup, profileId);
        UTF_VERIFY_EQUAL7_PARAM_MAC(GT_OK, st, devNum, direction ,packetType, subProfileId, pclLookupNum ,enableExactMatchLookup, profileId);
        /*
            2.2 Call cpssDxChTtiExactMatchProfileIdGet.
            Expected: GT_OK and the same profileIds.
        */
        packetType = CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E;
        subProfileId = 7;
        pclLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;/* hit 1 client is PCL by default */
        profileId   = 9;

        st = cpssDxChExactMatchPclProfileIdGet(devNum,  direction ,packetType,subProfileId, pclLookupNum ,&enableExactMatchLookupGet, &profileIdGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(enableExactMatchLookup, enableExactMatchLookupGet,"got other profileId then was set: %d", devNum);

        if ((enableExactMatchLookup==enableExactMatchLookupGet)&&
            (enableExactMatchLookup==GT_TRUE))
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(profileId, profileIdGet,"got other profileId then was set: %d", devNum);
        }


        packetType = CPSS_DXCH_PCL_PACKET_TYPE_IPV4_UDP_E;
        pclLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
        profileId   = 12;

        st = cpssDxChExactMatchPclProfileIdGet(devNum,  direction ,packetType, subProfileId, pclLookupNum ,&enableExactMatchLookupGet, &profileIdGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(enableExactMatchLookup, enableExactMatchLookupGet,"got other profileId then was set: %d", devNum);

        if ((enableExactMatchLookup==enableExactMatchLookupGet)&&
            (enableExactMatchLookup==GT_TRUE))
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(profileId, profileIdGet,"got other profileId then was set: %d", devNum);
        }

        /*
            3.1 Call with wrong pclLookupNum [last]
            Expected: GT_BAD_PARAM.
        */
        pclLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E;
        st = cpssDxChExactMatchPclProfileIdSet(devNum, direction ,packetType, subProfileId, pclLookupNum ,enableExactMatchLookup, profileId);
        UTF_VERIFY_EQUAL7_PARAM_MAC(GT_BAD_PARAM, st, devNum, direction ,packetType, subProfileId, pclLookupNum ,enableExactMatchLookup, profileId);
        /*
            3.2 Call with wrong profileId [17]
            Expected: GT_OUT_OF_RANGE.
        */
        pclLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
        profileId = 17;
        st = cpssDxChExactMatchPclProfileIdSet(devNum, direction ,packetType, subProfileId, pclLookupNum ,enableExactMatchLookup, profileId);
        UTF_VERIFY_EQUAL7_PARAM_MAC(GT_OUT_OF_RANGE, st, devNum, direction ,packetType, subProfileId, pclLookupNum ,enableExactMatchLookup, profileId);
        /*
            3.3. Call with wrong subProfileId [8]
            Expected: GT_BAD_PARAM.
        */
        profileId = 2;
        subProfileId=8;
        st = cpssDxChExactMatchPclProfileIdSet(devNum, direction ,packetType, subProfileId, pclLookupNum ,enableExactMatchLookup, profileId);
        UTF_VERIFY_EQUAL7_PARAM_MAC(GT_BAD_PARAM, st, devNum, direction, packetType, subProfileId, pclLookupNum ,enableExactMatchLookup, profileId);
    }

    /* 4. For not active devices check that function returns non GT_NOT_APPLICABLE_DEVICE.*/
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_FALCON_E ) ;
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        /* valid parameters*/
        packetType = CPSS_DXCH_PCL_PACKET_TYPE_MPLS_E;
        direction = CPSS_PCL_DIRECTION_INGRESS_E ;
        subProfileId = 3;
        pclLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
        profileId   = 15;
        enableExactMatchLookup = GT_TRUE;
        st = cpssDxChExactMatchPclProfileIdSet(devNum, direction, packetType, subProfileId, pclLookupNum ,enableExactMatchLookup, profileId);
        UTF_VERIFY_EQUAL7_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum, direction, packetType, subProfileId, pclLookupNum ,enableExactMatchLookup, profileId);
    }
}
/*
GT_STATUS cpssDxChExactMatchPclProfileIdGet
(
   IN  GT_U8                               devNum,
   IN  CPSS_DXCH_PCL_PACKET_TYPE_ENT       packetType,
   IN  CPSS_PCL_DIRECTION_ENT              direction,
   IN  GT_U32                              subProfileId,
   IN  CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT    pclLookupNum,
   OUT GT_U32                              *profileIdPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchPclProfileIdGet)
{
/*
    ITERATE_DEVICES (Falcon)
    1.1. Call function with valid parameters
         Expected: GT_OK.
    1.2  Call function with NULL profileIdPtr.
         Expected: GT_BAD_PTR.
    1.3  Call function with NULL enableExactMatchLookupPtr
         Expected: GT_BAD_PTR.
    1.4  Call with wrong pclLookupNum [last]
         Expected: GT_BAD_PARAM.
    1.5  Call with wrong subProfileId [8]
         Expected: GT_BAD_PARAM.
    2.   Call for for not-active devices or devices from non-applicable family
         Expected: GT_NOT_APPLICABLE_DEVICE.
*/
   GT_STATUS                           st;
   GT_U8                               devNum;
   CPSS_DXCH_PCL_PACKET_TYPE_ENT       packetType;
   CPSS_PCL_DIRECTION_ENT              direction;
   GT_U32                              subProfileId;
   CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT    pclLookupNum;
   GT_BOOL                             enableExactMatchLookup;
   GT_U32                              profileId;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_FALCON_E ) ;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call function with valid parameters
                 Expected: GT_OK.
        */
        packetType = CPSS_DXCH_PCL_PACKET_TYPE_MPLS_E;
        direction = CPSS_PCL_DIRECTION_INGRESS_E ;
        subProfileId = 3;
        pclLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
        profileId   = 15;
        st = cpssDxChExactMatchPclProfileIdGet(devNum,direction , packetType, subProfileId, pclLookupNum ,&enableExactMatchLookup, &profileId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
        /*
            1.2  Call function with NULL profileIdPtr.
                 Expected: GT_BAD_PTR.
        */
        st = cpssDxChExactMatchPclProfileIdGet(devNum, direction ,packetType, subProfileId, pclLookupNum ,&enableExactMatchLookup, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, profileIdPtr = NULL", devNum);

       /*  1.3  Call function with NULL enableExactMatchLookupPtr
                Expected: GT_BAD_PTR.
        */
        st = cpssDxChExactMatchPclProfileIdGet(devNum, direction ,packetType, subProfileId, pclLookupNum ,NULL, &profileId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enableExactMatchLookupPtr = NULL", devNum);

        /*
            1.4 Call with wrong pclLookupNum [second]
                Expected: GT_BAD_PARAM.
        */
        pclLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E;
        st = cpssDxChExactMatchPclProfileIdGet(devNum, direction ,packetType, subProfileId, pclLookupNum ,&enableExactMatchLookup, &profileId);
        UTF_VERIFY_EQUAL7_PARAM_MAC(GT_BAD_PARAM, st, devNum, direction ,packetType, subProfileId, pclLookupNum ,enableExactMatchLookup, profileId);
        /*
            1.5 Call with wrong subProfileId [8]
                Expected: GT_BAD_PARAM.
        */
        subProfileId=8;
        pclLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
        st = cpssDxChExactMatchPclProfileIdGet(devNum, direction ,packetType, subProfileId, pclLookupNum ,&enableExactMatchLookup, &profileId);
        UTF_VERIFY_EQUAL7_PARAM_MAC(GT_BAD_PARAM, st, devNum, direction, packetType, subProfileId, pclLookupNum ,enableExactMatchLookup, profileId);
    }
    /* 2. For not active devices check that function returns non GT_NOT_APPLICABLE_DEVICE.*/
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_FALCON_E ) ;
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        /* valid parameters*/
        packetType = CPSS_DXCH_PCL_PACKET_TYPE_MPLS_E;
        direction = CPSS_PCL_DIRECTION_INGRESS_E ;
        subProfileId = 3;
        pclLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
        st = cpssDxChExactMatchPclProfileIdGet(devNum, direction ,packetType, subProfileId, pclLookupNum ,&enableExactMatchLookup, &profileId);
        UTF_VERIFY_EQUAL7_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum, direction ,packetType, subProfileId, pclLookupNum ,enableExactMatchLookup, profileId);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchClientLookupSet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT            exactMatchLookupNum,
    IN  CPSS_DXCH_EXACT_MATCH_CLIENT_ENT            clientType
)
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchClientLookupSet)
{
/*
    ITERATE_DEVICES (SIP6)
    1.1. Call with exactMatchLookupNum [first, second]
    Expected: GT_OK.
    1.2. Call with out of range clientType [CPSS_DXCH_EM_CLIENT_LAST_E].
    Expected: NON GT_OK.
    1.3. Call with out of range exactMatchLookupNum [last].
    Expected: NON GT_OK.

*/

    GT_STATUS   st  = GT_OK;
    GT_U8       devNum = 0;

    CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT            exactMatchLookupNum;
    CPSS_DXCH_EXACT_MATCH_CLIENT_ENT            clientType;
    CPSS_DXCH_EXACT_MATCH_CLIENT_ENT            clientTypeGet;

    /* this feature is on Falcon devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with exactMatchLookupNum [first]
            Expected: GT_OK.
        */
        exactMatchLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
        clientType = CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E;

        st = cpssDxChExactMatchClientLookupSet(devNum, exactMatchLookupNum, clientType);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, exactMatchLookupNum, clientType);

        st = cpssDxChExactMatchClientLookupGet(devNum,  exactMatchLookupNum, &clientTypeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChExactMatchClientLookupGet: %d", devNum);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(clientType, clientTypeGet,
                       "get another clientTypeGet than was set: %d", devNum);

        /*
            1.1. Call with exactMatchLookupNum [second]
            Expected: GT_OK.
        */
        exactMatchLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
        clientType = CPSS_DXCH_EXACT_MATCH_CLIENT_EPCL_E;

        st = cpssDxChExactMatchClientLookupSet(devNum, exactMatchLookupNum, clientType);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, exactMatchLookupNum, clientType);

        st = cpssDxChExactMatchClientLookupGet(devNum,  exactMatchLookupNum, &clientTypeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChExactMatchClientLookupGet: %d", devNum);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(clientType, clientTypeGet,
                       "get another clientTypeGet than was set: %d", devNum);

        /*
           1.2. Call with out of range clientType [CPSS_DXCH_EM_CLIENT_LAST_E].
            Expected: NON GT_OK.
        */
        clientType = CPSS_DXCH_EXACT_MATCH_CLIENT_LAST_E;

        st = cpssDxChExactMatchClientLookupSet(devNum,  exactMatchLookupNum, clientType);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_VALUE, st, devNum, exactMatchLookupNum, clientType);

       st = cpssDxChExactMatchClientLookupGet(devNum,  exactMatchLookupNum, &clientTypeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChExactMatchClientLookupGet: %d", devNum);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(CPSS_DXCH_EXACT_MATCH_CLIENT_EPCL_E, clientTypeGet,
                       "get another emFirstLookupClientGet than was set: %d", devNum);


        /* 1.3. Call with out of range exactMatchLookupNum [last].
            Expected: NON GT_OK.
         */
        exactMatchLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E;
        clientType = CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E;

        st = cpssDxChExactMatchClientLookupSet(devNum,  exactMatchLookupNum, clientType);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, devNum, exactMatchLookupNum, clientType);

        /* return to default values */
        exactMatchLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
        clientType = CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E;

        st = cpssDxChExactMatchClientLookupSet(devNum, exactMatchLookupNum, clientType);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, exactMatchLookupNum, clientType);

    }

    exactMatchLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
    clientType = CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */

    /* this feature is on Falcon devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChExactMatchClientLookupSet(devNum, exactMatchLookupNum, clientType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChExactMatchClientLookupSet(devNum, exactMatchLookupNum, clientType);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchClientLookupGet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT            exactMatchLookupNum,
    OUT CPSS_DXCH_EXACT_MATCH_CLIENT_ENT            *clientTypePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchClientLookupGet)
{
/*
    ITERATE_DEVICES (SIP6)
    1.1. Call with non null clientTypeGet
    Expected: GT_OK.
    1.2. Call with out of range exactMatchLookupNum [last].
    Expected: GT_BAD_PTR.
    1.3. Call with clientTypeGet [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       devNum = 0;

    CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT            exactMatchLookupNum;
    CPSS_DXCH_EXACT_MATCH_CLIENT_ENT            clientTypeGet;

    /* this feature is on Falcon devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with non null EM LookupClientsGet
            Expected: GT_OK.
        */
        exactMatchLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
        st = cpssDxChExactMatchClientLookupGet(devNum, exactMatchLookupNum, &clientTypeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
            1.2. Call with out of range exactMatchLookupNum [last].
            Expected: GT_BAD_PARAM.
        */

        exactMatchLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E;
        st = cpssDxChExactMatchClientLookupGet(devNum, exactMatchLookupNum, &clientTypeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);

        /*
            1.3. Call with clientTypeGet [NULL].
            Expected: GT_BAD_PTR.
        */
        exactMatchLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
        st = cpssDxChExactMatchClientLookupGet(devNum,exactMatchLookupNum, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, clientTypeGet = NULL", devNum);

    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */

    /* this feature is on Falcon devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;

    exactMatchLookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChExactMatchClientLookupGet(devNum, exactMatchLookupNum, &clientTypeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChExactMatchClientLookupGet(devNum, exactMatchLookupNum, &clientTypeGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchActivityBitEnableSet
(
    IN GT_U8   devNum,
    IN GT_BOOL exactMatchActivityBit
);
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchActivityBitEnableSet)
{
/*
    ITERATE_DEVICES (SIP6)
    1.1. Call with exactMatchActivityBit[GT_TRUE]
    Expected: GT_OK.
    1.2. Call with exactMatchActivityBit[GT_FALSE]
    Expected: GT_OK.
*/

    GT_STATUS   st  = GT_OK;
    GT_U8       devNum = 0;
    GT_BOOL     exactMatchActivityBitEnable;
    GT_BOOL     exactMatchActivityBitEnableGet;

    /* this feature is on Falcon devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
           1.1. Call with exactMatchActivityBitEnable[GT_TRUE]
               Expected: GT_OK.
        */
        exactMatchActivityBitEnable=GT_TRUE;

        st = cpssDxChExactMatchActivityBitEnableSet(devNum, exactMatchActivityBitEnable);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChExactMatchActivityBitEnableSet: %d", devNum);

        st = cpssDxChExactMatchActivityBitEnableGet(devNum, &exactMatchActivityBitEnableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChExactMatchActivityBitEnableGet: %d", devNum);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(exactMatchActivityBitEnable, exactMatchActivityBitEnableGet,
                       "get another exactMatchActivityBitEnable than was set: %d", devNum);

        /*
            1.2. Call with exactMatchActivityBitEnable[GT_FALSE]
                Expected: GT_OK.
        */

        exactMatchActivityBitEnable=GT_FALSE;

        st = cpssDxChExactMatchActivityBitEnableSet(devNum, exactMatchActivityBitEnable);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChExactMatchActivityBitEnableSet: %d", devNum);

        st = cpssDxChExactMatchActivityBitEnableGet(devNum, &exactMatchActivityBitEnableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChExactMatchActivityBitEnableGet: %d", devNum);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(exactMatchActivityBitEnable, exactMatchActivityBitEnableGet,
                       "get another exactMatchActivityBitEnable than was set: %d", devNum);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */

    /* this feature is on Falcon devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;

    exactMatchActivityBitEnable=GT_TRUE;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChExactMatchActivityBitEnableSet(devNum, exactMatchActivityBitEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChExactMatchActivityBitEnableSet(devNum, exactMatchActivityBitEnable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchActivityBitEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *exactMatchActivityBitEnablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchActivityBitEnableGet)
{
/*
    ITERATE_DEVICES (SIP6)
    1.1. Call with non null exactMatchActivityBitEnablePtr
    Expected: GT_OK.
    1.2. Call with exactMatchActivityBitEnablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       devNum = 0;
    GT_BOOL     exactMatchActivityBitEnableGet;

    /* this feature is on Falcon devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with non null exactMatchActivityBitEnablePtr
            Expected: GT_OK.
        */
        st = cpssDxChExactMatchActivityBitEnableGet(devNum, &exactMatchActivityBitEnableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChExactMatchActivityBitEnableGet: %d", devNum);

        /*
            1.2. Call with exactMatchActivityBitEnablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChExactMatchActivityBitEnableGet(devNum, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, devNum);

    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */

    /* this feature is on Falcon devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;


    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChExactMatchActivityBitEnableGet(devNum, &exactMatchActivityBitEnableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChExactMatchActivityBitEnableGet(devNum, &exactMatchActivityBitEnableGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchActivityStatusGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    IN  GT_U32                  exactMatchEntryIndex,
    IN  GT_BOOL                 exactMatchClearActivity,
    OUT GT_BOOL                 *exactMatchActivityStatusPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchActivityStatusGet)
{
/*
    ITERATE_DEVICES (SIP6)
    1.1. Call with exactMatchEntryIndex[0] exactMatchClearActivity[GT_FALSE]
    Expected: GT_BAD_VALUE, Exact Match entry is invalid.
    1.2. Call with exactMatchEntryIndex[1] exactMatchClearActivity[GT_TRUE]
    Expected: GT_BAD_VALUE, Exact Match entry is invalid.
    1.3. Call with exactMatchActivityStatusPtr[NULL]
    Expected: GT_OK.
*/

    GT_STATUS               st  = GT_OK;
    GT_U8                   devNum = 0;
    GT_PORT_GROUPS_BMP      portGroupsBmp;
    GT_U32                  portGroupId;
    GT_U32                  exactMatchEntryIndex;
    GT_BOOL                 exactMatchClearActivity;
    GT_BOOL                 exactMatchActivityStatus;

    /* this feature is on Falcon devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* Initialize port group. */
        portGroupId = 1;

        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum, portGroupId)
        {
            /* Set next active port group. */
            portGroupsBmp = (1 << portGroupId);

           /*
               1.1. Call with exactMatchEntryIndex[0] exactMatchClearActivity[GT_FALSE]
                   Expected: GT_BAD_VALUE, Exact Match entry is invalid.
            */

            exactMatchEntryIndex=0;
            exactMatchClearActivity=GT_FALSE;

            st = cpssDxChExactMatchActivityStatusGet(devNum,portGroupsBmp,
                                                     exactMatchEntryIndex,
                                                     exactMatchClearActivity,
                                                     &exactMatchActivityStatus);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_FOUND, st,"cpssDxChExactMatchActivityStatusGet: %d", devNum);

            /*
                1.2. Call with exactMatchEntryIndex[1] exactMatchClearActivity[GT_TRUE]
                    Expected: GT_BAD_VALUE, Exact Match entry is invalid.
            */
            exactMatchEntryIndex=1;
            exactMatchClearActivity=GT_TRUE;

            st = cpssDxChExactMatchActivityStatusGet(devNum,portGroupsBmp,
                                                     exactMatchEntryIndex,
                                                     exactMatchClearActivity,
                                                     &exactMatchActivityStatus);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_FOUND, st,"cpssDxChExactMatchActivityStatusGet: %d", devNum);

            /* 1.3. Call with exactMatchActivityStatusPtr[NULL]
               Expected: GT_OK.*/
            st = cpssDxChExactMatchActivityStatusGet(devNum,portGroupsBmp,
                                                     exactMatchEntryIndex,
                                                     exactMatchClearActivity,
                                                     NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,"cpssDxChExactMatchActivityStatusGet: %d", devNum);
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum, portGroupId)
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */

    /* this feature is on Falcon devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;

    exactMatchEntryIndex=1;
    exactMatchClearActivity=GT_TRUE;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        /* Initialize port group. */
        portGroupId = 1;

        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum, portGroupId)
        {
            /* Set next active port group. */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChExactMatchActivityStatusGet(devNum,portGroupsBmp,
                                                     exactMatchEntryIndex,
                                                     exactMatchClearActivity,
                                                     &exactMatchActivityStatus);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum, portGroupId)
    }

    /* Initialize port group. */
    portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

     st = cpssDxChExactMatchActivityStatusGet(devNum,portGroupsBmp,
                                             exactMatchEntryIndex,
                                             exactMatchClearActivity,
                                             &exactMatchActivityStatus);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchProfileKeyParamsSet
(
    IN GT_U8                                            devNum,
    IN GT_U32                                           exactMatchProfileIndex,
    IN CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC     *keyParamsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchProfileKeyParamsSet)
{
/*
    ITERATE_DEVICES (SIP6)
    1.1. Call with exactMatchProfileIndex, keyParamsPtr
    Expected: GT_OK.
    1.2. Call with out of range exactMatchProfileIndex [16]
    Expected: NON GT_OK.
    1.3. Call with keyParamsPtr [NULL]
    Expected: NON GT_OK.
*/

    GT_STATUS   st  = GT_OK;
    GT_U8       devNum = 0;
    GT_U8       i=0;

    GT_U32                                          exactMatchProfileIndex;
    CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC    keyParams;
    CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC    keyParamsGet;


    /* this feature is on Falcon devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with exactMatchProfileIndex, keyParamsPtr
                Expected: GT_OK.
        */

       exactMatchProfileIndex = 3;
       keyParams.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E;
       keyParams.keyStart=3;
       for (i=0;i<CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS;i++)
       {
           keyParams.mask[i]=i+1;
       }

        st = cpssDxChExactMatchProfileKeyParamsSet(devNum, exactMatchProfileIndex, &keyParams);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, exactMatchProfileIndex, keyParams.keySize, keyParams.keyStart);


        st = cpssDxChExactMatchProfileKeyParamsGet(devNum, exactMatchProfileIndex, &keyParamsGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChExactMatchProfileKeyParamsGet: %d", devNum);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(keyParams.keySize, keyParamsGet.keySize,
                       "get another keyParams.keySize than was set: %d", devNum);

        UTF_VERIFY_EQUAL1_STRING_MAC(keyParams.keyStart, keyParamsGet.keyStart,
                       "get another keyParams.keyStart than was set: %d", devNum);

        for (i=0;i<CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS;i++)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(keyParams.mask[i], keyParamsGet.mask[i],
                       "get another keyParams.mask than was set: %d", devNum);
        }

        /*
           1.2. Call with out of range exactMatchProfileIndex [16]
            Expected: NON GT_OK.

        */
        exactMatchProfileIndex = 16;

        st = cpssDxChExactMatchProfileKeyParamsSet(devNum, exactMatchProfileIndex, &keyParams);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, exactMatchProfileIndex);


        /*
           1.3. Call with keyParamsPtr [NULL]
            Expected: NON GT_OK.

        */
        exactMatchProfileIndex = 5;

        st = cpssDxChExactMatchProfileKeyParamsSet(devNum, exactMatchProfileIndex, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, exactMatchProfileIndex);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */

    /* this feature is on Falcon devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;

    exactMatchProfileIndex = 7;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChExactMatchProfileKeyParamsSet(devNum, exactMatchProfileIndex, &keyParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChExactMatchProfileKeyParamsSet(devNum, exactMatchProfileIndex, &keyParams);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchProfileKeyParamsGet
(
    IN  GT_U8                                           devNum,
    IN  GT_U32                                          exactMatchProfileIndex,
    OUT CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC    *keyParamsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchProfileKeyParamsGet)
{
/*
    ITERATE_DEVICES (SIP6)
    1.1. Call with non null exactMatchProfileIndex, keyParamsPtr
    Expected: GT_OK.
    1.2. Call with out of range exactMatchProfileIndex [16]
    Expected: NON GT_OK.
    1.3. Call with keyParamsPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       devNum = 0;

    GT_U32 exactMatchProfileIndex;
    CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC keyParamsGet;

    /* this feature is on Falcon devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with non null EM LookupClientsGet
            Expected: GT_OK.
        */
        exactMatchProfileIndex=2;

        st = cpssDxChExactMatchProfileKeyParamsGet(devNum, exactMatchProfileIndex, &keyParamsGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
            1.2. Call with out of range exactMatchProfileIndex [16]
                Expected: NON GT_OK.
        */
        exactMatchProfileIndex=16;
        st = cpssDxChExactMatchProfileKeyParamsGet(devNum, exactMatchProfileIndex, &keyParamsGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st, "%d, EM LookupClientsGet = NULL", devNum);

         /*
            1.3. Call with keyParamsPtr [NULL].
                Expected: GT_BAD_PTR.
        */
        exactMatchProfileIndex=1;
        st = cpssDxChExactMatchProfileKeyParamsGet(devNum, exactMatchProfileIndex, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, EM LookupClientsGet = NULL", devNum);

    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */

    /* this feature is on Falcon devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;

    exactMatchProfileIndex=2;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChExactMatchProfileKeyParamsGet(devNum, exactMatchProfileIndex, &keyParamsGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChExactMatchProfileKeyParamsGet(devNum, exactMatchProfileIndex, &keyParamsGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/**
* @internal prvSetExactMatchActionDefaultValues function
* @endinternal
*
* @brief   This routine set default values to an exact match
*          Action
*
* @param[in] actionPtr           - (pointer to) action
* @param[in] type                - action type (TTI/PCL/EPCL)
*/
static GT_VOID prvSetExactMatchActionDefaultValues
(
    IN CPSS_DXCH_EXACT_MATCH_ACTION_UNT         *actionPtr,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT    actionType
)
{
    cpssOsBzero((GT_VOID*) actionPtr, sizeof(CPSS_DXCH_EXACT_MATCH_ACTION_UNT));

    switch(actionType)
    {
    case CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E:
        actionPtr->ttiAction.tunnelTerminate                    = GT_FALSE;
        actionPtr->ttiAction.ttPassengerPacketType              = CPSS_DXCH_TTI_PASSENGER_IPV4V6_E;
        actionPtr->ttiAction.tsPassengerPacketType              = CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E;

        actionPtr->ttiAction.ttHeaderLength                     = 0;
        actionPtr->ttiAction.continueToNextTtiLookup            = GT_FALSE;

        actionPtr->ttiAction.copyTtlExpFromTunnelHeader         = GT_FALSE;

        actionPtr->ttiAction.mplsCommand                        = CPSS_DXCH_TTI_MPLS_NOP_CMD_E;
        actionPtr->ttiAction.mplsTtl                            = 0;

        actionPtr->ttiAction.passengerParsingOfTransitMplsTunnelMode = CPSS_DXCH_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_DISABLE_E;
        actionPtr->ttiAction.passengerParsingOfTransitNonMplsTransitTunnelEnable = GT_TRUE;

        actionPtr->ttiAction.enableDecrementTtl                 = GT_FALSE;

        actionPtr->ttiAction.command                            = CPSS_PACKET_CMD_FORWARD_E;

        actionPtr->ttiAction.redirectCommand                    = CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E;
        actionPtr->ttiAction.egressInterface.type               = CPSS_INTERFACE_PORT_E;
        actionPtr->ttiAction.egressInterface.devPort.hwDevNum     = 0;
        actionPtr->ttiAction.egressInterface.devPort.portNum    = TTI_VALID_PORT_TRUNK_CNS;
        actionPtr->ttiAction.egressInterface.trunkId            = TTI_VALID_PORT_TRUNK_CNS;
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(actionPtr->ttiAction.egressInterface.trunkId);
        actionPtr->ttiAction.egressInterface.vidx               = 0;
        actionPtr->ttiAction.egressInterface.vlanId             = 0;
        actionPtr->ttiAction.egressInterface.hwDevNum             = 0;
        actionPtr->ttiAction.egressInterface.fabricVidx         = 0;
        actionPtr->ttiAction.egressInterface.index              = 0;
        actionPtr->ttiAction.arpPtr                             = 0;
        actionPtr->ttiAction.tunnelStart                        = GT_TRUE;
        actionPtr->ttiAction.tunnelStartPtr                     = 0;
        actionPtr->ttiAction.routerLttPtr                       = 0;
        actionPtr->ttiAction.vrfId                              = 0;

        actionPtr->ttiAction.sourceIdSetEnable                  = GT_FALSE;
        actionPtr->ttiAction.sourceId = 0;

        actionPtr->ttiAction.tag0VlanCmd                        = CPSS_DXCH_TTI_VLAN_DO_NOT_MODIFY_E;
        actionPtr->ttiAction.tag0VlanId                         = TTI_VALID_VLAN_ID_CNS;
        actionPtr->ttiAction.tag1VlanCmd                        = CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E;
        actionPtr->ttiAction.tag1VlanId                         = TTI_VALID_VLAN_ID_CNS;
        actionPtr->ttiAction.tag0VlanPrecedence                 = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;
        actionPtr->ttiAction.nestedVlanEnable                   = GT_FALSE;

        actionPtr->ttiAction.bindToPolicerMeter                 = GT_FALSE;
        actionPtr->ttiAction.bindToPolicer                      = GT_FALSE;
        actionPtr->ttiAction.policerIndex                       = 0;

        actionPtr->ttiAction.qosPrecedence                      = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
        actionPtr->ttiAction.keepPreviousQoS                    = GT_FALSE;
        actionPtr->ttiAction.trustUp                            = GT_FALSE;
        actionPtr->ttiAction.trustDscp                          = GT_FALSE;
        actionPtr->ttiAction.trustExp                           = GT_FALSE;
        actionPtr->ttiAction.qosProfile                         = 0;
        actionPtr->ttiAction.modifyTag0Up                       = CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_UP_E;
        actionPtr->ttiAction.tag1UpCommand                      = CPSS_DXCH_TTI_TAG1_UP_ASSIGN_VLAN1_UNTAGGED_E;
        actionPtr->ttiAction.modifyDscp                         = CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_DSCP_E;
        actionPtr->ttiAction.tag0Up                             = 0;
        actionPtr->ttiAction.tag1Up                             = 0;
        actionPtr->ttiAction.remapDSCP                          = GT_FALSE;

        actionPtr->ttiAction.qosUseUpAsIndexEnable              = GT_FALSE;
        actionPtr->ttiAction.qosMappingTableIndex               = 0;
        actionPtr->ttiAction.mplsLLspQoSProfileEnable           = GT_FALSE;


        actionPtr->ttiAction.pcl0OverrideConfigIndex            = CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E;
        actionPtr->ttiAction.pcl0_1OverrideConfigIndex          = CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E;
        actionPtr->ttiAction.pcl1OverrideConfigIndex            = CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E;
        actionPtr->ttiAction.iPclConfigIndex                    = 0;

        actionPtr->ttiAction.iPclUdbConfigTableEnable           = GT_FALSE;
        actionPtr->ttiAction.iPclUdbConfigTableIndex            = CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E;

        actionPtr->ttiAction.mirrorToIngressAnalyzerEnable      = GT_FALSE;
        actionPtr->ttiAction.mirrorToIngressAnalyzerIndex       = 0;
        actionPtr->ttiAction.userDefinedCpuCode                 = CPSS_NET_FIRST_USER_DEFINED_E + 1;
        actionPtr->ttiAction.bindToCentralCounter               = GT_FALSE;
        actionPtr->ttiAction.centralCounterIndex                = 0;
        actionPtr->ttiAction.vntl2Echo                          = GT_FALSE;
        actionPtr->ttiAction.bridgeBypass                       = GT_FALSE;
        actionPtr->ttiAction.ingressPipeBypass                  = GT_FALSE;
        actionPtr->ttiAction.actionStop                         = GT_FALSE;
        actionPtr->ttiAction.hashMaskIndex                      = 0;
        actionPtr->ttiAction.modifyMacSa                        = GT_FALSE;
        actionPtr->ttiAction.modifyMacDa                        = GT_FALSE;
        actionPtr->ttiAction.ResetSrcPortGroupId                = GT_FALSE;
        actionPtr->ttiAction.multiPortGroupTtiEnable            = GT_FALSE;

        actionPtr->ttiAction.sourceEPortAssignmentEnable        = GT_FALSE;
        actionPtr->ttiAction.sourceEPort                        = TTI_VALID_PORT_TRUNK_CNS;

        actionPtr->ttiAction.flowId                             = 0;
        actionPtr->ttiAction.setMacToMe                         = GT_FALSE;
        actionPtr->ttiAction.rxProtectionSwitchEnable           = GT_FALSE;
        actionPtr->ttiAction.rxIsProtectionPath                 = GT_FALSE;
        actionPtr->ttiAction.pwTagMode                          = CPSS_DXCH_TTI_PW_TAG_DISABLED_MODE_E;

        actionPtr->ttiAction.oamTimeStampEnable                 = GT_FALSE;
        actionPtr->ttiAction.oamOffsetIndex                     = 0;
        actionPtr->ttiAction.oamProcessEnable                   = GT_FALSE;
        actionPtr->ttiAction.oamProfile                         = 0;
        actionPtr->ttiAction.oamChannelTypeToOpcodeMappingEnable= GT_FALSE;
        actionPtr->ttiAction.isPtpPacket                        = GT_FALSE;
        actionPtr->ttiAction.ptpTriggerType                     = CPSS_DXCH_TTI_PTP_TRIGGER_TYPE_PTP_OVER_L2_E;
        actionPtr->ttiAction.ptpOffset                          = 0;

        actionPtr->ttiAction.cwBasedPw                          = GT_FALSE;
        actionPtr->ttiAction.ttlExpiryVccvEnable                = GT_FALSE;
        actionPtr->ttiAction.pwe3FlowLabelExist                 = GT_FALSE;
        actionPtr->ttiAction.pwCwBasedETreeEnable               = GT_FALSE;
        actionPtr->ttiAction.applyNonDataCwCommand              = GT_FALSE;

        actionPtr->ttiAction.unknownSaCommandEnable             = GT_FALSE;
        actionPtr->ttiAction.unknownSaCommand                   = CPSS_PACKET_CMD_FORWARD_E;
        actionPtr->ttiAction.sourceMeshIdSetEnable              = GT_FALSE;
        actionPtr->ttiAction.sourceMeshId                       = 0;

        break;
    case CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E:
    case CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E:

    /*  actionPtr [ pktCmd = CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
        mirror{cpuCode = 0, analyzerPortIndex = GT_FALSE},
        matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 },
        qos { egressPolicy=GT_FALSE, modifyDscp=CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E,
        modifyUp=CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E,
        qos [ ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
        profilePrecedence=GT_FALSE] ] },
        redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E,
        data[routerLttIndex=0] }, policer { policerEnable=GT_FALSE, policerId=0 },
        vlan { egressTaggedModify=GT_FALSE,
        modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE,
        vlanId=100, precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E },
        ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } ].
    */
        actionPtr->pclAction.pktCmd       = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
        actionPtr->pclAction.actionStop   = GT_TRUE;
        actionPtr->pclAction.egressPolicy = GT_FALSE;

        actionPtr->pclAction.mirror.cpuCode = CPSS_NET_CONTROL_SRC_DST_MAC_TRAP_E;
        actionPtr->pclAction.mirror.mirrorToRxAnalyzerPort = GT_FALSE;

        actionPtr->pclAction.matchCounter.enableMatchCount = GT_FALSE;
        actionPtr->pclAction.matchCounter.matchCounterIndex = 0;

        actionPtr->pclAction.qos.ingress.modifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
        actionPtr->pclAction.qos.ingress.modifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
        actionPtr->pclAction.qos.ingress.profileIndex = 0;
        actionPtr->pclAction.qos.ingress.profileAssignIndex = GT_FALSE;
        actionPtr->pclAction.qos.ingress.profilePrecedence =
                                    CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;

        actionPtr->pclAction.redirect.redirectCmd = CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;

        actionPtr->pclAction.redirect.data.outIf.outInterface.type   = CPSS_INTERFACE_VID_E;
        actionPtr->pclAction.redirect.data.outIf.outInterface.vlanId = 100;

        actionPtr->pclAction.redirect.data.outIf.vntL2Echo     = GT_FALSE;
        actionPtr->pclAction.redirect.data.outIf.tunnelStart   = GT_FALSE;
        actionPtr->pclAction.redirect.data.outIf.tunnelType    =
                                CPSS_DXCH_PCL_ACTION_REDIRECT_TUNNEL_TYPE_ETHERNET_E;

        /* logical Source Port applicable only for xCat C0; xCat3 device */
        actionPtr->pclAction.redirect.data.logicalSourceInterface.logicalInterface.type = CPSS_INTERFACE_PORT_E;
        actionPtr->pclAction.redirect.data.logicalSourceInterface.logicalInterface.devPort.hwDevNum = 0;
        actionPtr->pclAction.redirect.data.logicalSourceInterface.logicalInterface.devPort.portNum = 0;
        actionPtr->pclAction.redirect.data.logicalSourceInterface.sourceMeshIdSetEnable = GT_FALSE;
        actionPtr->pclAction.redirect.data.logicalSourceInterface.sourceMeshId = 0;
        actionPtr->pclAction.redirect.data.logicalSourceInterface.userTagAcEnable = GT_FALSE;

        actionPtr->pclAction.policer.policerEnable = GT_FALSE;
        actionPtr->pclAction.policer.policerId     = 0;

        actionPtr->pclAction.vlan.ingress.modifyVlan = CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E;
        actionPtr->pclAction.vlan.ingress.nestedVlan = GT_FALSE;
        actionPtr->pclAction.vlan.ingress.precedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
        actionPtr->pclAction.vlan.ingress.vlanId     = 100;

        actionPtr->pclAction.ipUcRoute.doIpUcRoute       = GT_FALSE;
        actionPtr->pclAction.ipUcRoute.arpDaIndex        = 0;
        actionPtr->pclAction.ipUcRoute.decrementTTL      = GT_FALSE;
        actionPtr->pclAction.ipUcRoute.bypassTTLCheck    = GT_FALSE;
        actionPtr->pclAction.ipUcRoute.icmpRedirectCheck = GT_FALSE;

        actionPtr->pclAction.sourceId.assignSourceId = GT_TRUE;
        actionPtr->pclAction.sourceId.sourceIdValue  = 0;

        /* oam field applicable only for Bobcat2; Caelum; Bobcat3 device */
        actionPtr->pclAction.oam.timeStampEnable = GT_FALSE;
        actionPtr->pclAction.oam.offsetIndex = 0;
        actionPtr->pclAction.oam.oamProcessEnable = GT_FALSE;
        actionPtr->pclAction.oam.oamProfile = 0;

        /* flowId field applicable only for Bobcat2; Caelum; Bobcat3 device */
        actionPtr->pclAction.flowId = 0;

        /* Assigns the unknown-SA-command applicable only for xCat C0; xCat3 device */
        actionPtr->pclAction.unknownSaCommandEnable = GT_FALSE;
        actionPtr->pclAction.unknownSaCommand = CPSS_PACKET_CMD_FORWARD_E;

        break;
    default:
        break;
    }
}

/**
* @internal prvCompareExactMatchInterfaceInfoStructs function
* @endinternal
*
* @brief   This routine set compares 2 Interface info structures.
*
* @param[in] actionName               - name of tested action structs
* @param[in] interfaceInfoName        - name of interface member structs
* @param[in] firstPtr                 - (pointer to) first struct
* @param[in] secondPtr                - (pointer to) second struct
*/
static GT_STATUS prvCompareExactMatchInterfaceInfoStructs
(
    IN GT_CHAR                          *actionName,
    IN GT_CHAR                          *interfaceInfoName,
    IN CPSS_INTERFACE_INFO_STC          *firstPtr,
    IN CPSS_INTERFACE_INFO_STC          *secondPtr
)
{
    GT_STATUS                           st = GT_OK;

    if(firstPtr->type != secondPtr->type)
    {
        PRV_UTF_LOG4_MAC("get another %s.%s.type than was set, expected = %d, received = %d\n",
                         actionName, interfaceInfoName, firstPtr->type,
                         secondPtr->type);
        st = GT_FAIL;
    }

    switch(firstPtr->type)
    {
    case CPSS_INTERFACE_PORT_E:
        if(firstPtr->devPort.hwDevNum != secondPtr->devPort.hwDevNum)
        {
            PRV_UTF_LOG4_MAC("get another %s.%s.devPort.hwDevNum than was set, expected = %d, received = %d\n",
                             actionName, interfaceInfoName,
                             firstPtr->devPort.hwDevNum, secondPtr->devPort.hwDevNum);
            st = GT_FAIL;
        }
        if(firstPtr->devPort.portNum != secondPtr->devPort.portNum)
        {
            PRV_UTF_LOG4_MAC("get another %s.%s.devPort.portNum than was set, expected = %d, received = %d\n",
                             actionName, interfaceInfoName,
                             firstPtr->devPort.portNum, secondPtr->devPort.portNum);
            st = GT_FAIL;
        }
        break;

    case CPSS_INTERFACE_TRUNK_E:
        if(firstPtr->trunkId != secondPtr->trunkId)
        {
            PRV_UTF_LOG4_MAC("get another %s.%s.trunkId than was set, expected = %d, received = %d\n",
                             actionName, interfaceInfoName,
                             firstPtr->trunkId, secondPtr->trunkId);
            st = GT_FAIL;
        }
        break;

    case CPSS_INTERFACE_VIDX_E:
        if(firstPtr->vidx != secondPtr->vidx)
        {
            PRV_UTF_LOG4_MAC("get another %s.%s.vidx than was set, expected = %d, received = %d\n",
                             actionName, interfaceInfoName,
                             firstPtr->vidx, secondPtr->vidx);
            st = GT_FAIL;
        }
        break;

    case CPSS_INTERFACE_VID_E:
        if(firstPtr->vlanId != secondPtr->vlanId)
        {
            PRV_UTF_LOG4_MAC("get another %s.%s.vlanId than was set, expected = %d, received = %d\n",
                             actionName, interfaceInfoName,
                             firstPtr->vlanId, secondPtr->vlanId);
            st = GT_FAIL;
        }
        break;

    case CPSS_INTERFACE_DEVICE_E:
        if(firstPtr->hwDevNum != secondPtr->hwDevNum)
        {
            PRV_UTF_LOG4_MAC("get another %s.%s.hwDevNum than was set, expected = %d, received = %d\n",
                             actionName, interfaceInfoName,
                             firstPtr->hwDevNum, secondPtr->hwDevNum);
            st = GT_FAIL;
        }
        break;

    case CPSS_INTERFACE_FABRIC_VIDX_E:
        if(firstPtr->fabricVidx != secondPtr->fabricVidx)
        {
            PRV_UTF_LOG4_MAC("get another %s.%s.fabricVidx than was set, expected = %d, received = %d\n",
                             actionName, interfaceInfoName,
                             firstPtr->fabricVidx, secondPtr->fabricVidx);
            st = GT_FAIL;
        }
        break;

    case CPSS_INTERFACE_INDEX_E:
        if(firstPtr->index != secondPtr->index)
        {
            PRV_UTF_LOG4_MAC("get another %s.%s.index than was set, expected = %d, received = %d\n",
                             actionName, interfaceInfoName,
                             firstPtr->index, secondPtr->index);
            st = GT_FAIL;
        }
        break;

    default:
        break;
    }

    return st;
}

/**
* @internal prvCompareExactMatchActionStructs function
* @endinternal
*
* @brief   This routine set compares 2 Action structures.
*
* @param[in] devNum                   - device number
* @param[in] name                     -  of tested sctucts objects
* @param[in] actionType               - type of the action to use
* @param[in] firstPtr                 - (pointer to) first struct
* @param[in] secondPtr                - (pointer to) second struct
*/
static GT_STATUS prvCompareExactMatchActionStructs
(
    IN GT_U8                                    devNum,
    IN GT_CHAR                                  *name,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT    actionType,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_UNT         *firstActionPtr,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_UNT         *secondActionPtr
)
{
    GT_STATUS   st = GT_OK, op_st;
    GT_BOOL isEqual,failureWas;

    switch(actionType)
    {
    case CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E:
        if(firstActionPtr->ttiAction.tunnelTerminate != secondActionPtr->ttiAction.tunnelTerminate)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.tunnelTerminate than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.tunnelTerminate,
                             secondActionPtr->ttiAction.tunnelTerminate);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiAction.ttPassengerPacketType != secondActionPtr->ttiAction.ttPassengerPacketType)
        {
            if(((CPSS_DXCH_TTI_MPLS_PUSH1_CMD_E == firstActionPtr->ttiAction.mplsCommand) ||
                (CPSS_DXCH_TTI_MPLS_POP1_CMD_E == firstActionPtr->ttiAction.mplsCommand)  ||
                (CPSS_DXCH_TTI_MPLS_POP2_CMD_E == firstActionPtr->ttiAction.mplsCommand)  ||
                (CPSS_DXCH_TTI_MPLS_POP3_CMD_E == firstActionPtr->ttiAction.mplsCommand)  ||
                (CPSS_DXCH_TTI_MPLS_POP_AND_SWAP_CMD_E == firstActionPtr->ttiAction.mplsCommand)) &&
               (CPSS_DXCH_TTI_PASSENGER_MPLS_E != firstActionPtr->ttiAction.ttPassengerPacketType))
            {
                PRV_UTF_LOG1_MAC("error in test, %s.ttiAction.ttPassengerPacketType is invalid\n",
                                 name);
            }

            PRV_UTF_LOG3_MAC("get another %s.ttiAction.ttPassengerPacketType than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.ttPassengerPacketType,
                             secondActionPtr->ttiAction.ttPassengerPacketType);
            st = GT_FAIL;
        }
        if((CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E == firstActionPtr->ttiAction.redirectCommand) &&
           (GT_TRUE == firstActionPtr->ttiAction.tunnelStart))
        {
            if(firstActionPtr->ttiAction.tsPassengerPacketType != secondActionPtr->ttiAction.tsPassengerPacketType)
            {
                if(((CPSS_DXCH_TTI_MPLS_PUSH1_CMD_E == firstActionPtr->ttiAction.mplsCommand) ||
                    (CPSS_DXCH_TTI_MPLS_POP1_CMD_E == firstActionPtr->ttiAction.mplsCommand)  ||
                    (CPSS_DXCH_TTI_MPLS_POP2_CMD_E == firstActionPtr->ttiAction.mplsCommand)  ||
                    (CPSS_DXCH_TTI_MPLS_POP3_CMD_E == firstActionPtr->ttiAction.mplsCommand)  ||
                    (CPSS_DXCH_TTI_MPLS_POP_AND_SWAP_CMD_E == firstActionPtr->ttiAction.mplsCommand)) &&
                   (CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E != firstActionPtr->ttiAction.tsPassengerPacketType))
                {
                    PRV_UTF_LOG1_MAC("error in test, %s.ttiAction.tsPassengerPacketType is invalid\n",
                                     name);
                }

                PRV_UTF_LOG3_MAC("get another %s.ttiAction.tsPassengerPacketType than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->ttiAction.tsPassengerPacketType,
                                 secondActionPtr->ttiAction.tsPassengerPacketType);
                st = GT_FAIL;
            }
        }

        if(firstActionPtr->ttiAction.copyTtlExpFromTunnelHeader != secondActionPtr->ttiAction.copyTtlExpFromTunnelHeader)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.copyTtlExpFromTunnelHeader than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.copyTtlExpFromTunnelHeader,
                             secondActionPtr->ttiAction.copyTtlExpFromTunnelHeader);
            st = GT_FAIL;
        }

        if(firstActionPtr->ttiAction.mplsCommand != secondActionPtr->ttiAction.mplsCommand)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.mplsCommand than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.mplsCommand,
                             secondActionPtr->ttiAction.mplsCommand);
            st = GT_FAIL;
        }
        if(CPSS_DXCH_TTI_MPLS_NOP_CMD_E != firstActionPtr->ttiAction.mplsCommand)
        {
            if(firstActionPtr->ttiAction.mplsTtl != secondActionPtr->ttiAction.mplsTtl)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.mplsTtl than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->ttiAction.mplsTtl, secondActionPtr->ttiAction.mplsTtl);
                st = GT_FAIL;
            }
            if(firstActionPtr->ttiAction.enableDecrementTtl != secondActionPtr->ttiAction.enableDecrementTtl)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.enableDecrementTtl than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->ttiAction.enableDecrementTtl,
                                 secondActionPtr->ttiAction.enableDecrementTtl);
                st = GT_FAIL;
            }
        }

        if(firstActionPtr->ttiAction.command != secondActionPtr->ttiAction.command)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.command than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.command,
                             secondActionPtr->ttiAction.command);
            st = GT_FAIL;
        }

        if(firstActionPtr->ttiAction.redirectCommand != secondActionPtr->ttiAction.redirectCommand)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.redirectCommand than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.redirectCommand,
                             secondActionPtr->ttiAction.redirectCommand);
            st = GT_FAIL;
        }
        if(CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E == firstActionPtr->ttiAction.redirectCommand)
        {
            op_st = prvCompareExactMatchInterfaceInfoStructs(name,
                                                   "ttiAction.egressInterface",
                                                   &firstActionPtr->ttiAction.egressInterface,
                                                   &secondActionPtr->ttiAction.egressInterface);
            st = GT_OK != op_st ? op_st: st;
            if(firstActionPtr->ttiAction.arpPtr != secondActionPtr->ttiAction.arpPtr)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.arpPtr than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->ttiAction.arpPtr,
                                 secondActionPtr->ttiAction.arpPtr);
                st = GT_FAIL;
            }
            if(firstActionPtr->ttiAction.tunnelStart != secondActionPtr->ttiAction.tunnelStart)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.tunnelStart than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->ttiAction.tunnelStart,
                                 secondActionPtr->ttiAction.tunnelStart);
                st = GT_FAIL;
            }
            if(firstActionPtr->ttiAction.tunnelStartPtr != secondActionPtr->ttiAction.tunnelStartPtr)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.tunnelStartPtr than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->ttiAction.tunnelStartPtr,
                                 secondActionPtr->ttiAction.tunnelStartPtr);
                st = GT_FAIL;
            }
        }

        if(firstActionPtr->ttiAction.sourceIdSetEnable != secondActionPtr->ttiAction.sourceIdSetEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.sourceIdSetEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.sourceIdSetEnable,
                             secondActionPtr->ttiAction.sourceIdSetEnable);
            st = GT_FAIL;
        }
        if (firstActionPtr->ttiAction.sourceIdSetEnable == GT_TRUE)
        {
            if(firstActionPtr->ttiAction.sourceId != secondActionPtr->ttiAction.sourceId)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.sourceId than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->ttiAction.sourceId,
                                 secondActionPtr->ttiAction.sourceId);
                st = GT_FAIL;
            }
        }

        if(firstActionPtr->ttiAction.tag0VlanCmd != secondActionPtr->ttiAction.tag0VlanCmd)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.tag0VlanCmd than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.tag0VlanCmd,
                             secondActionPtr->ttiAction.tag0VlanCmd);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiAction.tag0VlanId != secondActionPtr->ttiAction.tag0VlanId)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.tag0VlanId than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.tag0VlanId,
                             secondActionPtr->ttiAction.tag0VlanId);
            st = GT_FAIL;
        }
        if(CPSS_DXCH_TTI_MPLS_NOP_CMD_E == firstActionPtr->ttiAction.mplsCommand)
        {
            if(firstActionPtr->ttiAction.tag1VlanCmd != secondActionPtr->ttiAction.tag1VlanCmd)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.tag1VlanCmd than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->ttiAction.tag1VlanCmd,
                                 secondActionPtr->ttiAction.tag1VlanCmd);
                st = GT_FAIL;
            }
            if(firstActionPtr->ttiAction.tag1VlanId != secondActionPtr->ttiAction.tag1VlanId)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.tag1VlanId than was set, expected = %d, received = %d\n",
                            name, firstActionPtr->ttiAction.tag1VlanId,
                            secondActionPtr->ttiAction.tag1VlanId);
                st = GT_FAIL;
            }
        }
        if(firstActionPtr->ttiAction.tag0VlanPrecedence != secondActionPtr->ttiAction.tag0VlanPrecedence)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.tag0VlanPrecedence than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.tag0VlanPrecedence,
                             secondActionPtr->ttiAction.tag0VlanPrecedence);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiAction.nestedVlanEnable != secondActionPtr->ttiAction.nestedVlanEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.nestedVlanEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.nestedVlanEnable,
                             secondActionPtr->ttiAction.nestedVlanEnable);
            st = GT_FAIL;
        }

        if(firstActionPtr->ttiAction.bindToPolicerMeter != secondActionPtr->ttiAction.bindToPolicerMeter)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.bindToPolicerMeter than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.bindToPolicerMeter,
                             secondActionPtr->ttiAction.bindToPolicerMeter);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiAction.bindToPolicer != secondActionPtr->ttiAction.bindToPolicer)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.bindToPolicer than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.bindToPolicer,
                             secondActionPtr->ttiAction.bindToPolicer);
            st = GT_FAIL;
        }
        if (GT_TRUE == firstActionPtr->ttiAction.bindToPolicer)
        {
            if(firstActionPtr->ttiAction.policerIndex != secondActionPtr->ttiAction.policerIndex)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.policerIndex than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->ttiAction.policerIndex,
                                 secondActionPtr->ttiAction.policerIndex);
                st = GT_FAIL;
            }
        }

        if(firstActionPtr->ttiAction.qosPrecedence != secondActionPtr->ttiAction.qosPrecedence)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.qosPrecedence than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.qosPrecedence,
                             secondActionPtr->ttiAction.qosPrecedence);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiAction.keepPreviousQoS != secondActionPtr->ttiAction.keepPreviousQoS)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.keepPreviousQoS than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.keepPreviousQoS,
                             secondActionPtr->ttiAction.keepPreviousQoS);
            st = GT_FAIL;
        }
        if (GT_FALSE == firstActionPtr->ttiAction.keepPreviousQoS)
        {
            if(firstActionPtr->ttiAction.trustUp != secondActionPtr->ttiAction.trustUp)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.trustUp than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->ttiAction.trustUp,
                                 secondActionPtr->ttiAction.trustUp);
                st = GT_FAIL;
            }
            if(firstActionPtr->ttiAction.trustDscp != secondActionPtr->ttiAction.trustDscp)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.trustDscp than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->ttiAction.trustDscp,
                                 secondActionPtr->ttiAction.trustDscp);
                st = GT_FAIL;
            }
            if(firstActionPtr->ttiAction.trustExp != secondActionPtr->ttiAction.trustExp)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.trustExp than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->ttiAction.trustExp,
                                 secondActionPtr->ttiAction.trustExp);
                st = GT_FAIL;
            }

            if(firstActionPtr->ttiAction.qosProfile != secondActionPtr->ttiAction.qosProfile)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.qosProfile than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->ttiAction.qosProfile,
                                 secondActionPtr->ttiAction.qosProfile);
                st = GT_FAIL;
            }
        }
        if(firstActionPtr->ttiAction.modifyTag0Up != secondActionPtr->ttiAction.modifyTag0Up)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.modifyTag0Up than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.modifyTag0Up,
                             secondActionPtr->ttiAction.modifyTag0Up);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiAction.tag1UpCommand != secondActionPtr->ttiAction.tag1UpCommand)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.tag1UpCommand than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.tag1UpCommand,
                             secondActionPtr->ttiAction.tag1UpCommand);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiAction.modifyDscp != secondActionPtr->ttiAction.modifyDscp)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.modifyDscp than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.modifyDscp,
                             secondActionPtr->ttiAction.modifyDscp);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiAction.tag0Up != secondActionPtr->ttiAction.tag0Up)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.tag0Up than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.tag0Up,
                             secondActionPtr->ttiAction.tag0Up);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiAction.tag1Up != secondActionPtr->ttiAction.tag1Up)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.tag1Up than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.tag1Up,
                             secondActionPtr->ttiAction.tag1Up);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiAction.remapDSCP != secondActionPtr->ttiAction.remapDSCP)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.remapDSCP than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.remapDSCP,
                             secondActionPtr->ttiAction.remapDSCP);
            st = GT_FAIL;
        }

        if(CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E != firstActionPtr->ttiAction.redirectCommand)
        {
            if(firstActionPtr->ttiAction.pcl0OverrideConfigIndex != secondActionPtr->ttiAction.pcl0OverrideConfigIndex)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.pcl0OverrideConfigIndex than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->ttiAction.pcl0OverrideConfigIndex,
                                 secondActionPtr->ttiAction.pcl0OverrideConfigIndex);
                st = GT_FAIL;
            }
            if(firstActionPtr->ttiAction.pcl0_1OverrideConfigIndex != secondActionPtr->ttiAction.pcl0_1OverrideConfigIndex)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.pcl0_1OverrideConfigIndex than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->ttiAction.pcl0_1OverrideConfigIndex,
                                 secondActionPtr->ttiAction.pcl0_1OverrideConfigIndex);
                st = GT_FAIL;
            }
            if(firstActionPtr->ttiAction.pcl1OverrideConfigIndex != secondActionPtr->ttiAction.pcl1OverrideConfigIndex)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.pcl1OverrideConfigIndex than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->ttiAction.pcl1OverrideConfigIndex,
                                 secondActionPtr->ttiAction.pcl1OverrideConfigIndex);
                st = GT_FAIL;
            }
            if(firstActionPtr->ttiAction.iPclConfigIndex != secondActionPtr->ttiAction.iPclConfigIndex)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.iPclConfigIndex than was set, expected = %d, received = %d\n",
                            name, firstActionPtr->ttiAction.iPclConfigIndex,
                            secondActionPtr->ttiAction.iPclConfigIndex);
                st = GT_FAIL;
            }
        }

        if(firstActionPtr->ttiAction.mirrorToIngressAnalyzerEnable != secondActionPtr->ttiAction.mirrorToIngressAnalyzerEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.mirrorToIngressAnalyzerEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.mirrorToIngressAnalyzerEnable,
                             secondActionPtr->ttiAction.mirrorToIngressAnalyzerEnable);
            st = GT_FAIL;
        }

        if(firstActionPtr->ttiAction.mirrorToIngressAnalyzerIndex != secondActionPtr->ttiAction.mirrorToIngressAnalyzerIndex)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.mirrorToIngressAnalyzerIndex than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.mirrorToIngressAnalyzerIndex,
                             secondActionPtr->ttiAction.mirrorToIngressAnalyzerIndex);
            st = GT_FAIL;
        }

        if((CPSS_PACKET_CMD_MIRROR_TO_CPU_E == firstActionPtr->ttiAction.command) ||
           (CPSS_PACKET_CMD_TRAP_TO_CPU_E == firstActionPtr->ttiAction.command))
        {
            if(firstActionPtr->ttiAction.userDefinedCpuCode != secondActionPtr->ttiAction.userDefinedCpuCode)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.userDefinedCpuCode than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->ttiAction.userDefinedCpuCode,
                                 secondActionPtr->ttiAction.userDefinedCpuCode);
                st = GT_FAIL;
            }
        }
        if(firstActionPtr->ttiAction.bindToCentralCounter != secondActionPtr->ttiAction.bindToCentralCounter)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.bindToCentralCounter than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.bindToCentralCounter,
                             secondActionPtr->ttiAction.bindToCentralCounter);
            st = GT_FAIL;
        }
        if(GT_TRUE == firstActionPtr->ttiAction.bindToCentralCounter)
        {
            if(firstActionPtr->ttiAction.centralCounterIndex != secondActionPtr->ttiAction.centralCounterIndex)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.centralCounterIndex than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->ttiAction.centralCounterIndex,
                                 secondActionPtr->ttiAction.centralCounterIndex);
                st = GT_FAIL;
            }
        }
        if(CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E == firstActionPtr->ttiAction.redirectCommand)
        {
            if(firstActionPtr->ttiAction.vntl2Echo != secondActionPtr->ttiAction.vntl2Echo)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.vntl2Echo than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->ttiAction.vntl2Echo,
                                 secondActionPtr->ttiAction.vntl2Echo);
                st = GT_FAIL;
            }
        }
        if(firstActionPtr->ttiAction.bridgeBypass != secondActionPtr->ttiAction.bridgeBypass)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.bridgeBypass than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.bridgeBypass,
                             secondActionPtr->ttiAction.bridgeBypass);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiAction.ingressPipeBypass != secondActionPtr->ttiAction.ingressPipeBypass)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.ingressPipeBypass than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.ingressPipeBypass,
                             secondActionPtr->ttiAction.ingressPipeBypass);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiAction.actionStop != secondActionPtr->ttiAction.actionStop)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.actionStop than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.actionStop,
                             secondActionPtr->ttiAction.actionStop);
            st = GT_FAIL;
        }

        if(firstActionPtr->ttiAction.sourceEPortAssignmentEnable != secondActionPtr->ttiAction.sourceEPortAssignmentEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.sourceEPortAssignmentEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiAction.sourceEPortAssignmentEnable,
                             secondActionPtr->ttiAction.sourceEPortAssignmentEnable);
            st = GT_FAIL;
        }
        if(GT_TRUE == firstActionPtr->ttiAction.sourceEPortAssignmentEnable)
        {
            if(firstActionPtr->ttiAction.sourceEPort != secondActionPtr->ttiAction.sourceEPort)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.sourceEPort than was set, expected = %d, received = %d",
                                 name, firstActionPtr->ttiAction.sourceEPort,
                                 secondActionPtr->ttiAction.sourceEPort);
                st = GT_FAIL;
            }
        }

        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            if(firstActionPtr->ttiAction.cwBasedPw   != secondActionPtr->ttiAction.cwBasedPw)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.cwBasedPw   than was set, expected = %d, received = %d\n",
                                                 name, firstActionPtr->ttiAction.cwBasedPw,
                                                 secondActionPtr->ttiAction.cwBasedPw);
                st = GT_FAIL;
            }
        }

        if((GT_TRUE == firstActionPtr->ttiAction.tunnelTerminate) ||
          (CPSS_DXCH_TTI_PASSENGER_MPLS_E != firstActionPtr->ttiAction.ttPassengerPacketType))
        {
            if(firstActionPtr->ttiAction.ttHeaderLength != secondActionPtr->ttiAction.ttHeaderLength)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.ttHeaderLength than was set, expected = %d, received = %d\n",
                                                 name, firstActionPtr->ttiAction.ttHeaderLength,
                                                 secondActionPtr->ttiAction.ttHeaderLength);
                st = GT_FAIL;
            }
        }

        if(firstActionPtr->ttiAction.continueToNextTtiLookup != secondActionPtr->ttiAction.continueToNextTtiLookup)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.continueToNextTtiLookup than was set, expected = %d, received = %d\n",
                                             name, firstActionPtr->ttiAction.continueToNextTtiLookup,
                                             secondActionPtr->ttiAction.continueToNextTtiLookup);
            st = GT_FAIL;
        }

        if(firstActionPtr->ttiAction.passengerParsingOfTransitMplsTunnelMode  != secondActionPtr->ttiAction.passengerParsingOfTransitMplsTunnelMode )
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.passengerParsingOfTransitMplsTunnelMode  than was set, expected = %d, received = %d\n",
                                             name, firstActionPtr->ttiAction.passengerParsingOfTransitMplsTunnelMode ,
                                             secondActionPtr->ttiAction.passengerParsingOfTransitMplsTunnelMode );
            st = GT_FAIL;
        }

        if(firstActionPtr->ttiAction.passengerParsingOfTransitNonMplsTransitTunnelEnable   != secondActionPtr->ttiAction.passengerParsingOfTransitNonMplsTransitTunnelEnable  )
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.passengerParsingOfTransitNonMplsTransitTunnelEnable   than was set, expected = %d, received = %d\n",
                                             name, firstActionPtr->ttiAction.passengerParsingOfTransitNonMplsTransitTunnelEnable  ,
                                             secondActionPtr->ttiAction.passengerParsingOfTransitNonMplsTransitTunnelEnable  );
            st = GT_FAIL;
        }

        if(GT_FALSE != firstActionPtr->ttiAction.keepPreviousQoS)
        {
            if(firstActionPtr->ttiAction.qosUseUpAsIndexEnable != secondActionPtr->ttiAction.qosUseUpAsIndexEnable)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.qosUseUpAsIndexEnable   than was set, expected = %d, received = %d\n",
                                                 name, firstActionPtr->ttiAction.qosUseUpAsIndexEnable  ,
                                                 secondActionPtr->ttiAction.qosUseUpAsIndexEnable  );
                st = GT_FAIL;
            }

            if(GT_FALSE == firstActionPtr->ttiAction.qosUseUpAsIndexEnable)
            {
                if(firstActionPtr->ttiAction.qosMappingTableIndex != secondActionPtr->ttiAction.qosMappingTableIndex)
                {
                    PRV_UTF_LOG3_MAC("get another %s.ttiAction.qosUseUpAsIndexEnable   than was set, expected = %d, received = %d\n",
                                                     name, firstActionPtr->ttiAction.qosUseUpAsIndexEnable  ,
                                                     secondActionPtr->ttiAction.qosUseUpAsIndexEnable);
                    st = GT_FAIL;
                }
            }
        }

        if(firstActionPtr->ttiAction.redirectCommand == CPSS_DXCH_TTI_NO_REDIRECT_E &&
            (firstActionPtr->ttiAction.iPclUdbConfigTableEnable   != secondActionPtr->ttiAction.iPclUdbConfigTableEnable  ))
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.iPclUdbConfigTableEnable   than was set, expected = %d, received = %d\n",
                                             name, firstActionPtr->ttiAction.iPclUdbConfigTableEnable  ,
                                             secondActionPtr->ttiAction.iPclUdbConfigTableEnable  );
            st = GT_FAIL;
        }

        if((CPSS_DXCH_TTI_NO_REDIRECT_E == firstActionPtr->ttiAction.redirectCommand) &&
            firstActionPtr->ttiAction.iPclUdbConfigTableEnable   )
        {
            if(firstActionPtr->ttiAction.iPclUdbConfigTableIndex != secondActionPtr->ttiAction.iPclUdbConfigTableIndex)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.iPclUdbConfigTableIndex   than was set, expected = %d, received = %d\n",
                                                 name, firstActionPtr->ttiAction.iPclUdbConfigTableIndex  ,
                                                 secondActionPtr->ttiAction.iPclUdbConfigTableIndex  );
                st = GT_FAIL;
            }
        }

        if(CPSS_DXCH_TTI_NO_REDIRECT_E == firstActionPtr->ttiAction.redirectCommand)
        {
            if(firstActionPtr->ttiAction.flowId != secondActionPtr->ttiAction.flowId)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.flowId   than was set, expected = %d, received = %d\n",
                                                 name, firstActionPtr->ttiAction.flowId,
                                                 secondActionPtr->ttiAction.flowId);
                st = GT_FAIL;
            }
        }

        if(CPSS_DXCH_TTI_NO_REDIRECT_E == firstActionPtr->ttiAction.redirectCommand)
        {
            if(firstActionPtr->ttiAction.flowId != secondActionPtr->ttiAction.flowId)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.flowId   than was set, expected = %d, received = %d\n",
                                                 name, firstActionPtr->ttiAction.flowId,
                                                 secondActionPtr->ttiAction.flowId);
                st = GT_FAIL;
            }
        }

        if(firstActionPtr->ttiAction.setMacToMe   != secondActionPtr->ttiAction.setMacToMe)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.setMacToMe   than was set, expected = %d, received = %d\n",
                                             name, firstActionPtr->ttiAction.setMacToMe,
                                             secondActionPtr->ttiAction.setMacToMe);
            st = GT_FAIL;
        }

        if(firstActionPtr->ttiAction.rxProtectionSwitchEnable   != secondActionPtr->ttiAction.rxProtectionSwitchEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.rxProtectionSwitchEnable   than was set, expected = %d, received = %d\n",
                                             name, firstActionPtr->ttiAction.rxProtectionSwitchEnable,
                                             secondActionPtr->ttiAction.rxProtectionSwitchEnable);
            st = GT_FAIL;
        }

        if(firstActionPtr->ttiAction.rxIsProtectionPath   != secondActionPtr->ttiAction.rxIsProtectionPath)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.rxIsProtectionPath   than was set, expected = %d, received = %d\n",
                                             name, firstActionPtr->ttiAction.rxIsProtectionPath,
                                             secondActionPtr->ttiAction.rxIsProtectionPath);
            st = GT_FAIL;
        }

        if(firstActionPtr->ttiAction.oamProcessEnable   != secondActionPtr->ttiAction.oamProcessEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.oamOffsetIndex   than was set, expected = %d, received = %d\n",
                                             name, firstActionPtr->ttiAction.oamOffsetIndex,
                                             secondActionPtr->ttiAction.oamOffsetIndex);
            st = GT_FAIL;
        }

        if(firstActionPtr->ttiAction.oamProfile   != secondActionPtr->ttiAction.oamProfile)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.oamProfile   than was set, expected = %d, received = %d\n",
                                             name, firstActionPtr->ttiAction.oamProfile,
                                             secondActionPtr->ttiAction.oamProfile);
            st = GT_FAIL;
        }

        if(firstActionPtr->ttiAction.oamChannelTypeToOpcodeMappingEnable   != secondActionPtr->ttiAction.oamChannelTypeToOpcodeMappingEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.oamChannelTypeToOpcodeMappingEnable   than was set, expected = %d, received = %d\n",
                                             name, firstActionPtr->ttiAction.oamChannelTypeToOpcodeMappingEnable,
                                             secondActionPtr->ttiAction.oamChannelTypeToOpcodeMappingEnable);
            st = GT_FAIL;
        }

        if(firstActionPtr->ttiAction.isPtpPacket   != secondActionPtr->ttiAction.isPtpPacket)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.isPtpPacket   than was set, expected = %d, received = %d\n",
                                             name, firstActionPtr->ttiAction.isPtpPacket,
                                             secondActionPtr->ttiAction.isPtpPacket);
            st = GT_FAIL;
        }

        if(GT_TRUE == firstActionPtr->ttiAction.isPtpPacket)
        {
            if(firstActionPtr->ttiAction.ptpTriggerType != secondActionPtr->ttiAction.ptpTriggerType)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.ptpTriggerType   than was set, expected = %d, received = %d\n",
                                                 name, firstActionPtr->ttiAction.ptpTriggerType,
                                                 secondActionPtr->ttiAction.ptpTriggerType);
                st = GT_FAIL;
            }

            if(firstActionPtr->ttiAction.ptpOffset != secondActionPtr->ttiAction.ptpOffset)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.ptpOffset   than was set, expected = %d, received = %d\n",
                                                 name, firstActionPtr->ttiAction.ptpOffset,
                                                 secondActionPtr->ttiAction.ptpOffset);
                st = GT_FAIL;
            }
        }
        else
        {
            if(firstActionPtr->ttiAction.oamTimeStampEnable   != secondActionPtr->ttiAction.oamTimeStampEnable)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.oamTimeStampEnable   than was set, expected = %d, received = %d\n",
                                                 name, firstActionPtr->ttiAction.oamTimeStampEnable,
                                                 secondActionPtr->ttiAction.oamTimeStampEnable);
                st = GT_FAIL;
            }

            if(firstActionPtr->ttiAction.oamOffsetIndex   != secondActionPtr->ttiAction.oamOffsetIndex)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.oamOffsetIndex   than was set, expected = %d, received = %d\n",
                                                 name, firstActionPtr->ttiAction.oamOffsetIndex,
                                                 secondActionPtr->ttiAction.oamOffsetIndex);
                st = GT_FAIL;
            }
        }

        if(firstActionPtr->ttiAction.ttlExpiryVccvEnable   != secondActionPtr->ttiAction.ttlExpiryVccvEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.ttlExpiryVccvEnable   than was set, expected = %d, received = %d\n",
                                             name, firstActionPtr->ttiAction.ttlExpiryVccvEnable,
                                             secondActionPtr->ttiAction.ttlExpiryVccvEnable);
            st = GT_FAIL;
        }

        if(firstActionPtr->ttiAction.pwe3FlowLabelExist   != secondActionPtr->ttiAction.pwe3FlowLabelExist)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.pwe3FlowLabelExist   than was set, expected = %d, received = %d\n",
                                             name, firstActionPtr->ttiAction.pwe3FlowLabelExist,
                                             secondActionPtr->ttiAction.pwe3FlowLabelExist);
            st = GT_FAIL;
        }

        if(firstActionPtr->ttiAction.pwCwBasedETreeEnable   != secondActionPtr->ttiAction.pwCwBasedETreeEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.pwCwBasedETreeEnable   than was set, expected = %d, received = %d\n",
                                             name, firstActionPtr->ttiAction.pwCwBasedETreeEnable,
                                             secondActionPtr->ttiAction.pwCwBasedETreeEnable);
            st = GT_FAIL;
        }

        if(firstActionPtr->ttiAction.applyNonDataCwCommand   != secondActionPtr->ttiAction.applyNonDataCwCommand)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiAction.applyNonDataCwCommand   than was set, expected = %d, received = %d\n",
                                             name, firstActionPtr->ttiAction.applyNonDataCwCommand,
                                             secondActionPtr->ttiAction.applyNonDataCwCommand);
            st = GT_FAIL;
        }

        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            if(firstActionPtr->ttiAction.unknownSaCommandEnable   != secondActionPtr->ttiAction.unknownSaCommandEnable)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.unknownSaCommandEnable   than was set, expected = %d, received = %d\n",
                                                 name, firstActionPtr->ttiAction.unknownSaCommandEnable,
                                                 secondActionPtr->ttiAction.unknownSaCommandEnable);
                st = GT_FAIL;
            }
            if(firstActionPtr->ttiAction.unknownSaCommand   != secondActionPtr->ttiAction.unknownSaCommand)
            {
                PRV_UTF_LOG3_MAC("get another %s.ttiAction.unknownSaCommand   than was set, expected = %d, received = %d\n",
                                                 name, firstActionPtr->ttiAction.unknownSaCommand,
                                                 secondActionPtr->ttiAction.unknownSaCommand);
                st = GT_FAIL;
            }
            if(CPSS_DXCH_TTI_LOGICAL_PORT_ASSIGN_E == firstActionPtr->ttiAction.redirectCommand)
            {
                if(firstActionPtr->ttiAction.sourceMeshIdSetEnable   != secondActionPtr->ttiAction.sourceMeshIdSetEnable)
                {
                    PRV_UTF_LOG3_MAC("get another %s.ttiAction.sourceMeshIdSetEnable   than was set, expected = %d, received = %d\n",
                                                     name, firstActionPtr->ttiAction.sourceMeshIdSetEnable,
                                                     secondActionPtr->ttiAction.sourceMeshIdSetEnable);
                    st = GT_FAIL;
                }
                if(firstActionPtr->ttiAction.sourceMeshId   != secondActionPtr->ttiAction.sourceMeshId)
                {
                    PRV_UTF_LOG3_MAC("get another %s.ttiAction.sourceMeshId   than was set, expected = %d, received = %d\n",
                                                     name, firstActionPtr->ttiAction.sourceMeshId,
                                                     secondActionPtr->ttiAction.sourceMeshId);
                    st = GT_FAIL;
                }
            }
        }

        break;

    case CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E:
            /* verifying values */
            if(firstActionPtr->pclAction.pktCmd != secondActionPtr->pclAction.pktCmd)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.pktCmd than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.pktCmd, secondActionPtr->pclAction.pktCmd);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.actionStop != secondActionPtr->pclAction.actionStop)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.actionStop than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.actionStop, secondActionPtr->pclAction.actionStop);
                st = GT_FAIL;
            }

            /*mirror*/
            if(firstActionPtr->pclAction.mirror.cpuCode != secondActionPtr->pclAction.mirror.cpuCode)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.mirror.cpuCode than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.mirror.cpuCode, secondActionPtr->pclAction.mirror.cpuCode);
                st = GT_FAIL;
            }
            if(firstActionPtr->pclAction.mirror.ingressMirrorToAnalyzerIndex != secondActionPtr->pclAction.mirror.ingressMirrorToAnalyzerIndex)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.mirror.ingressMirrorToAnalyzerIndex than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.mirror.ingressMirrorToAnalyzerIndex, secondActionPtr->pclAction.mirror.ingressMirrorToAnalyzerIndex);
                st = GT_FAIL;
            }
            if(firstActionPtr->pclAction.mirror.mirrorTcpRstAndFinPacketsToCpu != secondActionPtr->pclAction.mirror.mirrorTcpRstAndFinPacketsToCpu)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.mirror.mirrorTcpRstAndFinPacketsToCpu than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.mirror.mirrorTcpRstAndFinPacketsToCpu, secondActionPtr->pclAction.mirror.mirrorTcpRstAndFinPacketsToCpu);
                st = GT_FAIL;
            }
            if(firstActionPtr->pclAction.mirror.mirrorToRxAnalyzerPort != secondActionPtr->pclAction.mirror.mirrorToRxAnalyzerPort)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.mirror.mirrorToRxAnalyzerPort than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.mirror.mirrorToRxAnalyzerPort, secondActionPtr->pclAction.mirror.mirrorToRxAnalyzerPort);
                st = GT_FAIL;
            }
            if(firstActionPtr->pclAction.mirror.mirrorToTxAnalyzerPortEn != secondActionPtr->pclAction.mirror.mirrorToTxAnalyzerPortEn)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.mirror.mirrorToTxAnalyzerPortEn than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.mirror.mirrorToTxAnalyzerPortEn, secondActionPtr->pclAction.mirror.mirrorToTxAnalyzerPortEn);
                st = GT_FAIL;
            }
            if(firstActionPtr->pclAction.mirror.egressMirrorToAnalyzerIndex != secondActionPtr->pclAction.mirror.egressMirrorToAnalyzerIndex)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.mirror.egressMirrorToAnalyzerIndex than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.mirror.egressMirrorToAnalyzerIndex, secondActionPtr->pclAction.mirror.egressMirrorToAnalyzerIndex);
                st = GT_FAIL;
            }
            if(firstActionPtr->pclAction.mirror.egressMirrorToAnalyzerMode != secondActionPtr->pclAction.mirror.egressMirrorToAnalyzerMode)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.mirror.egressMirrorToAnalyzerMode than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.mirror.egressMirrorToAnalyzerMode, secondActionPtr->pclAction.mirror.egressMirrorToAnalyzerMode);
                st = GT_FAIL;
            }

            /*matchCounter*/
            if(firstActionPtr->pclAction.matchCounter.enableMatchCount != secondActionPtr->pclAction.matchCounter.enableMatchCount)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.matchCounter.enableMatchCount than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.matchCounter.enableMatchCount, secondActionPtr->pclAction.matchCounter.enableMatchCount);
                st = GT_FAIL;
            }
            if(firstActionPtr->pclAction.matchCounter.matchCounterIndex != secondActionPtr->pclAction.matchCounter.matchCounterIndex)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.matchCounter.matchCounterIndex than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.matchCounter.matchCounterIndex, secondActionPtr->pclAction.matchCounter.matchCounterIndex);
                st = GT_FAIL;
            }

            /*policer*/
            if(firstActionPtr->pclAction.policer.policerEnable != secondActionPtr->pclAction.policer.policerEnable)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.policer.policerEnable than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.policer.policerEnable, secondActionPtr->pclAction.policer.policerEnable);
                st = GT_FAIL;
            }
            if(firstActionPtr->pclAction.policer.policerId != secondActionPtr->pclAction.policer.policerId)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.policer.policerId than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.policer.policerId, secondActionPtr->pclAction.policer.policerId);
                st = GT_FAIL;
            }

            /*vlan*/
            isEqual = (0 == cpssOsMemCmp(
                  (GT_VOID*) &firstActionPtr->pclAction.vlan.egress,
                  (GT_VOID*) &secondActionPtr->pclAction.vlan.egress,
                  sizeof(secondActionPtr->pclAction.vlan.egress))) ? GT_TRUE : GT_FALSE;

            if(isEqual != GT_TRUE)
            {
                PRV_UTF_LOG0_MAC("get another %s.pclAction.vlan.egress than was set\n");

                st = GT_FAIL;
            }
            isEqual = (0 == cpssOsMemCmp(
                  (GT_VOID*) &firstActionPtr->pclAction.vlan.ingress,
                  (GT_VOID*) &secondActionPtr->pclAction.vlan.ingress,
                  sizeof(secondActionPtr->pclAction.vlan.ingress))) ? GT_TRUE : GT_FALSE;

            if(isEqual != GT_TRUE)
            {
                PRV_UTF_LOG0_MAC("get another %s.pclAction.vlan.ingress than was set\n");

                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.egressPolicy != secondActionPtr->pclAction.egressPolicy)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.egressPolicy than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.egressPolicy, secondActionPtr->pclAction.egressPolicy);
                st = GT_FAIL;
            }


           /*redirect*/

            if(firstActionPtr->pclAction.redirect.redirectCmd != secondActionPtr->pclAction.redirect.redirectCmd)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.redirect.redirectCmd than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.redirect.redirectCmd, secondActionPtr->pclAction.redirect.redirectCmd);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.redirect.data.outIf.tunnelPtr != secondActionPtr->pclAction.redirect.data.outIf.tunnelPtr)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.redirect.data.outIf.tunnelPtr than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.redirect.data.outIf.tunnelPtr, secondActionPtr->pclAction.redirect.data.outIf.tunnelPtr);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.redirect.data.outIf.tunnelStart != secondActionPtr->pclAction.redirect.data.outIf.tunnelStart)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.redirect.data.outIf.tunnelStart than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.redirect.data.outIf.tunnelStart, secondActionPtr->pclAction.redirect.data.outIf.tunnelStart);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.redirect.data.outIf.tunnelType != secondActionPtr->pclAction.redirect.data.outIf.tunnelType)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.redirect.data.outIf.tunnelType than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.redirect.data.outIf.tunnelType, secondActionPtr->pclAction.redirect.data.outIf.tunnelType);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.redirect.data.outIf.vntL2Echo != secondActionPtr->pclAction.redirect.data.outIf.vntL2Echo)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.redirect.data.outIf.vntL2Echo than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.redirect.data.outIf.vntL2Echo, secondActionPtr->pclAction.redirect.data.outIf.vntL2Echo);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.mirror.mirrorToRxAnalyzerPort != secondActionPtr->pclAction.mirror.mirrorToRxAnalyzerPort)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.mirror.mirrorToRxAnalyzerPort than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.mirror.mirrorToRxAnalyzerPort, secondActionPtr->pclAction.mirror.mirrorToRxAnalyzerPort);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.mirror.ingressMirrorToAnalyzerIndex != secondActionPtr->pclAction.mirror.ingressMirrorToAnalyzerIndex)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.mirror.ingressMirrorToAnalyzerIndex than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.mirror.ingressMirrorToAnalyzerIndex, secondActionPtr->pclAction.mirror.ingressMirrorToAnalyzerIndex);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.redirect.redirectCmd != secondActionPtr->pclAction.redirect.redirectCmd)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.redirect.redirectCmd than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.redirect.redirectCmd, secondActionPtr->pclAction.redirect.redirectCmd);
                st = GT_FAIL;
            }

            isEqual = (0 == cpssOsMemCmp(
                  (GT_VOID*) &firstActionPtr->pclAction.redirect.data.modifyMacSa.macSa,
                  (GT_VOID*) &secondActionPtr->pclAction.redirect.data.modifyMacSa.macSa,
                  sizeof(firstActionPtr->pclAction.redirect.data.modifyMacSa.macSa))) ? GT_TRUE : GT_FALSE;

            if(isEqual != GT_TRUE)
            {
                PRV_UTF_LOG0_MAC("get another %s.pclAction.redirect.data.modifyMacSa.macSa than was set\n");
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.redirect.data.modifyMacSa.arpPtr != secondActionPtr->pclAction.redirect.data.modifyMacSa.arpPtr)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.redirect.data.modifyMacSa.arpPtr than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.redirect.data.modifyMacSa.arpPtr, secondActionPtr->pclAction.redirect.data.modifyMacSa.arpPtr);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.oam.timeStampEnable != secondActionPtr->pclAction.oam.timeStampEnable)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.oam.timeStampEnable than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.oam.timeStampEnable, secondActionPtr->pclAction.oam.timeStampEnable);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.oam.offsetIndex != secondActionPtr->pclAction.oam.offsetIndex)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.oam.offsetIndex than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.oam.offsetIndex, secondActionPtr->pclAction.oam.offsetIndex);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.oam.oamProcessEnable != secondActionPtr->pclAction.oam.oamProcessEnable)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.oam.oamProcessEnable than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.oam.oamProcessEnable, secondActionPtr->pclAction.oam.oamProcessEnable);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.oam.oamProfile != secondActionPtr->pclAction.oam.oamProfile)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.oam.oamProfile than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.oam.oamProfile, secondActionPtr->pclAction.oam.oamProfile);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.sourcePort.assignSourcePortEnable != secondActionPtr->pclAction.sourcePort.assignSourcePortEnable)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.sourcePort.assignSourcePortEnable than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.sourcePort.assignSourcePortEnable, secondActionPtr->pclAction.sourcePort.assignSourcePortEnable);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.sourcePort.sourcePortValue != secondActionPtr->pclAction.sourcePort.sourcePortValue)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.sourcePort.sourcePortValue than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.sourcePort.sourcePortValue, secondActionPtr->pclAction.sourcePort.sourcePortValue);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.qos.ingress.up1Cmd != secondActionPtr->pclAction.qos.ingress.up1Cmd)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.qos.ingress.up1Cmd than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.qos.ingress.up1Cmd, secondActionPtr->pclAction.qos.ingress.up1Cmd);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.qos.ingress.up1 != secondActionPtr->pclAction.qos.ingress.up1)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.pclAction.qos.ingress.up1 than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.qos.ingress.up1, secondActionPtr->pclAction.qos.ingress.up1);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.vlan.ingress.vlanId1Cmd != secondActionPtr->pclAction.vlan.ingress.vlanId1Cmd)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.vlan.ingress.vlanId1Cmd than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.vlan.ingress.vlanId1Cmd, secondActionPtr->pclAction.vlan.ingress.vlanId1Cmd);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.vlan.ingress.vlanId1 != secondActionPtr->pclAction.vlan.ingress.vlanId1)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.vlan.ingress.vlanId1 than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.vlan.ingress.vlanId1, secondActionPtr->pclAction.vlan.ingress.vlanId1);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.flowId != secondActionPtr->pclAction.flowId)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.flowId than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.flowId, secondActionPtr->pclAction.flowId);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.setMacToMe != secondActionPtr->pclAction.setMacToMe)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.setMacToMe than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.setMacToMe, secondActionPtr->pclAction.setMacToMe);
                st = GT_FAIL;
            }
        break;
    case CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E:
            /* verifying values */
            /*pktCmd*/

            if(firstActionPtr->pclAction.pktCmd != secondActionPtr->pclAction.pktCmd)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.pktCmd than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.pktCmd, secondActionPtr->pclAction.pktCmd);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.actionStop != secondActionPtr->pclAction.actionStop)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.actionStop than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.actionStop, secondActionPtr->pclAction.actionStop);
                st = GT_FAIL;
            }

            /*matchCounter*/
            failureWas = (0 == cpssOsMemCmp((const GT_VOID*)&firstActionPtr->pclAction.matchCounter,
                                            (const GT_VOID*)&secondActionPtr->pclAction.matchCounter,
                                            sizeof(firstActionPtr->pclAction.matchCounter))) ? GT_FALSE : GT_TRUE;
            if(failureWas == GT_TRUE)
            {
                PRV_UTF_LOG0_MAC("get another %s.pclAction.matchCounter than was set\n");
                st = GT_FAIL;
            }

            /*policer*/
            failureWas = (0 == cpssOsMemCmp((const GT_VOID*)&firstActionPtr->pclAction.policer,
                                            (const GT_VOID*)&secondActionPtr->pclAction.policer,
                                            sizeof(firstActionPtr->pclAction.policer))) ? GT_FALSE : GT_TRUE;
            if(failureWas == GT_TRUE)
            {
                PRV_UTF_LOG0_MAC("get another %s.pclAction.policer than was set\n");
                st = GT_FAIL;
            }

            /*vlan*/
            failureWas = (0 == cpssOsMemCmp((const GT_VOID*)&firstActionPtr->pclAction.vlan,
                                            (const GT_VOID*)&secondActionPtr->pclAction.vlan,
                                            sizeof(firstActionPtr->pclAction.vlan))) ? GT_FALSE : GT_TRUE;
            if(failureWas == GT_TRUE)
            {
                PRV_UTF_LOG0_MAC("get another %s.pclAction.vlan than was set\n");
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.egressPolicy != secondActionPtr->pclAction.egressPolicy)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.egressPolicy than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.egressPolicy, secondActionPtr->pclAction.egressPolicy);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.qos.egress.modifyDscp != secondActionPtr->pclAction.qos.egress.modifyDscp)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.qos.egress.modifyDscp than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.qos.egress.modifyDscp, secondActionPtr->pclAction.qos.egress.modifyDscp);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.qos.egress.modifyUp != secondActionPtr->pclAction.qos.egress.modifyUp)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.qos.egress.modifyUp than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.qos.egress.modifyUp, secondActionPtr->pclAction.qos.egress.modifyUp);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.qos.egress.up1ModifyEnable != secondActionPtr->pclAction.qos.egress.up1ModifyEnable)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.qos.egress.up1ModifyEnable than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.qos.egress.up1ModifyEnable, secondActionPtr->pclAction.qos.egress.up1ModifyEnable);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.qos.egress.up1 != secondActionPtr->pclAction.qos.egress.up1)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.qos.egress.up1 than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.qos.egress.up1, secondActionPtr->pclAction.qos.egress.up1);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.vlan.egress.vlanCmd != secondActionPtr->pclAction.vlan.egress.vlanCmd)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.vlan.egress.vlanCmd than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.vlan.egress.vlanCmd, secondActionPtr->pclAction.vlan.egress.vlanCmd);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.vlan.egress.vlanId1ModifyEnable != secondActionPtr->pclAction.vlan.egress.vlanId1ModifyEnable)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.vlan.egress.vlanId1ModifyEnable than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.vlan.egress.vlanId1ModifyEnable, secondActionPtr->pclAction.vlan.egress.vlanId1ModifyEnable);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.vlan.egress.vlanId1 != secondActionPtr->pclAction.vlan.egress.vlanId1)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.vlan.egress.vlanId1 than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.vlan.egress.vlanId1, secondActionPtr->pclAction.vlan.egress.vlanId1);
                st = GT_FAIL;
            }

            if(firstActionPtr->pclAction.channelTypeToOpcodeMapEnable != secondActionPtr->pclAction.channelTypeToOpcodeMapEnable)
            {
                PRV_UTF_LOG3_MAC("get another %s.pclAction.channelTypeToOpcodeMapEnable than was set, expected = %d, received = %d\n",
                                 name, firstActionPtr->pclAction.channelTypeToOpcodeMapEnable, secondActionPtr->pclAction.channelTypeToOpcodeMapEnable);
                st = GT_FAIL;
            }

            if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.trafficManager.supported)
            {

                if(firstActionPtr->pclAction.tmQueueId != secondActionPtr->pclAction.tmQueueId)
                {
                    PRV_UTF_LOG3_MAC("get another %s.pclAction.tmQueueId than was set, expected = %d, received = %d\n",
                                     name, firstActionPtr->pclAction.tmQueueId, secondActionPtr->pclAction.tmQueueId);
                    st = GT_FAIL;
                }
            }
        break;
    default:
        break;
    }

    return st;
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchProfileDefaultActionSet
(
    IN GT_U8                                    devNum,
    IN GT_U32                                   exactMatchProfileIndex,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT    actionType,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_UNT         *actionPtr,
    IN GT_BOOL                                  defActionEn
)
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchProfileDefaultActionSet)
{
/*
    ITERATE_DEVICES (SIP6)
    1.1. Call cpssDxChExactMatchProfileDefaultActionSet with non-NULL pointers,
         actionPtr{tunnelTerminate[GT_TRUE],
                   passengerPacketType[CPSS_DXCH_TTI_PASSENGER_IPV4_E],
                   copyTtlFromTunnelHeader[GT_FALSE],
                   command [CPSS_PACKET_CMD_FORWARD_E],
                   redirectCommand[CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E],
                   egressInterface{type[CPSS_INTERFACE_TRUNK_E],
                                   trunkId[2]},

                   tunnelStart [GT_TRUE],
                   tunnelStartPtr[0],
                   targetIsTrunk[GT_FALSE],

                   sourceIdSetEnable[GT_TRUE],
                   sourceId[0],

                   vlanCmd[CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E],
                   vlanId[100],
                   vlanPrecedence[CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E],
                   nestedVlanEnable[GT_FALSE],

                   bindToPolicer[GT_FALSE],

                   qosPrecedence[CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E],
                   qosTrustMode[CPSS_DXCH_TTI_QOS_UNTRUST_E],
                   qosProfile[0],
                   modifyUpEnable[CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_UP_E],
                   modifyDscpEnable[CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_DSCP_E],
                   up[0],

                   mirrorToIngressAnalyzerEnable[GT_FALSE],
                   vntl2Echo[GT_FALSE],
                   bridgeBypass[GT_FALSE],
                   actionStop[GT_FALSE]
              }
    Expected: GT_OK and same action as written
    1.2. Call with wrong values exactMatchProfileIndex, other params same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.3. Call with wrong enum values actionType, other params same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.4. Call with actionPtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.

    1.5. set PCL actionPtr [ pktCmd = CPSS_PACKET_CMD_FORWARD_E,
        mirror{cpuCode = 0, analyzerPortIndex = GT_FALSE},
        matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 },
        qos { egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE ,
            qos [ ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
            profilePrecedence=GT_FALSE] ] },
        redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E,
            data[routerLttIndex=0] },
        policer { policerEnable=GT_FALSE, policerId=0 },
        vlan { egressTaggedModify=GT_FALSE,
            modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE,
            vlanId=100, precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E },
        ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } ].

    Expected: GT_OK and same action as written

    1.6. Call function with
            actionPtr { egressPolicy [GT_FALSE]
                        mirror { ingressMirrorToAnalyzerIndex[0 / 3 / 6]},
                        redirect {
                           redirectCmd [
                            CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E /
                            CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E /
                            CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_VIRT_ROUTER_E /
                            CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_REPLACE_MAC_SA_E],
                           data { modifyMacSa {macSa [00:00:00:00:00:00 /
                                                      A5:A5:A5:A5:A5:A5 /
                                                      FF:FF:FF:FF:FF:FF],
                                               arpPtr[0 / 0xA5A5 / BIT_17-1]}}},
                        oam { timeStampEnable [GT_FALSE / GT_TRUE],
                              offsetIndex [0 / 0xA / BIT_4-1],
                              oamProcessEnable [GT_FALSE / GT_TRUE],
                              oamProfile [0 / 1] },
                        sourcePort { assignSourcePortEnable [GT_FALSE / GT_TRUE]
                                 sourcePortValue [0 / 0xA5A5A5A5 / 0xFFFFFFFF] }
                        qos { ingress { up1Cmd[
                          CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_DO_NOT_MODIFY_E /
                          CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_TAG1_UNTAGGED_E /
                          CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_TAG0_UNTAGGED_E /
                          CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_ALL_E],
                                       up1[0 / 5 / BIT_3-1]} }
                        vlan { ingress { vlanId1Cmd[
                     CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_DO_NOT_MODIFY_E /
                     CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_UNTAGGED_E /
                     CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_ALL_E],
                                       vlanId1 [0 / 0xA5A / BIT_12-1]} },
                        flowId [0 / 0xA5A / BIT_12-1],
                        setMacToMe [GT_FALSE / GT_TRUE] }
            and other params from 1.1.
    Expected: GT_OK and same action as written
    1.7. Call function with
            actionPtr { egressPolicy [GT_TRUE]
                        qos { egress { modifyDscp[
                       CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_KEEP_E /
                       CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_MODIFY_OUTER_E /
                       CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_MODIFY_INNER_E],
                                      modifyUp[
                       CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E /
                       CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E /
                       CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E],
                                      up1ModifyEnable [GT_FALSE / GT_TRUE],
                                      up1[0 / 5 / BIT_3-1]} }
                        vlan { egress { vlanCmd[
                       CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E /
                       CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E /
                       CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E],
                                       vlanId1ModifyEnable [GT_FALSE / GT_TRUE],
                                       vlanId1 [0 / 0xA5A / BIT_12-1]} },
                        channelTypeToOpcodeMapEnable [GT_FALSE / GT_TRUE],
                        tmQueueId [0 / 0x2A5A / BIT_14-1] }
            and other params from 1.41.
    Expected: GT_OK and same action as written
*/
    GT_STATUS                                st = GT_OK;
    GT_U8                                    devNum = 0;
    GT_U32                                   exactMatchProfileIndex;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT    actionType;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT         defaultActionData;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT         defaultActionDataGet;
    GT_BOOL                                  defActionEn;
    GT_BOOL                                  defActionEnGet;
    GT_ETHERADDR                             macSa1 = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

    exactMatchProfileIndex = 2;
    actionType = CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E;
    defActionEn = GT_TRUE;

    cpssOsBzero((GT_VOID*) &defaultActionData, sizeof(defaultActionData));
    cpssOsBzero((GT_VOID*) &defaultActionDataGet, sizeof(defaultActionDataGet));

    /* this feature is on Falcon devNumNumices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;

    /* 1. Go over all active devNumNumices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*  actionPtr{ tunnelTerminate[GT_FALSE], passengerPacketType[CPSS_DXCH_TTI_PASSENGER_IPV4_E],
            copyTtlFromTunnelHeader[GT_FALSE], command [CPSS_PACKET_CMD_FORWARD_E],
            redirectCommand[CPSS_DXCH_TTI_NO_REDIRECT_E], egressInterface{type[CPSS_INTERFACE_TRUNK_E],
            trunkId[0], vidx[0], vlanId[0], devNumNumPort {0, 0}}, tunnelStart [GT_FALSE], tunnelStartPtr[0],
            routerLookupPtr[0], vrfId[0], targetIsTrunk[GT_FALSE], virtualSrcPort[0], virtualSrcdevNumNum[0],
            sourceIdSetEnable[GT_FALSE], sourceId[0], vlanCmd[CPSS_DXCH_TTI_VLAN_DO_NOT_MODIFY_E],
            vlanId[0], vlanPrecedence[CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E],
            nestedVlanEnable[GT_FALSE], bindToPolicer[GT_FALSE], policerIndex[0],
            qosPrecedence[CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E],
            qosTrustMode[CPSS_DXCH_TTI_QOS_KEEP_PREVIOUS_E], qosProfile[0],
            modifyUpEnable[CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_UP_E], modifyDscpEnable[CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_DSCP_E],
            up[0], remapDSCP[GT_FALSE], mirrorToIngressAnalyzerEnable[GT_FALSE],
            userDefinedCpuCode[CPSS_NET_CONTROL_E],
            vntl2Echo[GT_FALSE], bridgeBypass[GT_FALSE], actionStop[GT_FALSE] }
        */

        prvSetExactMatchActionDefaultValues(&defaultActionData,CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E);

        st = cpssDxChExactMatchProfileDefaultActionSet(devNum,exactMatchProfileIndex,actionType,&defaultActionData,defActionEn);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "cpssDxChExactMatchProfileDefaultActionSet: %d, %d, %d, %d",
                                         devNum, exactMatchProfileIndex, actionType, defActionEn);

        st = cpssDxChExactMatchProfileDefaultActionGet(devNum,exactMatchProfileIndex,actionType,&defaultActionDataGet,&defActionEnGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchProfileDefaultActionGet: %d, %d, %d", devNum, exactMatchProfileIndex,actionType);
        /* validation values */
        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &defaultActionData, &defaultActionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, exactMatchProfileIndex, actionType);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(defActionEn, defActionEnGet, "get another defActionEnGet than was set: %d", devNum);

       /*
            1.2. Call with out of range exactMatchProfileIndex [16],
                           other params same as in 1.1.
            Expected: non GT_OK.
        */
        exactMatchProfileIndex = 16;

        st = cpssDxChExactMatchProfileDefaultActionSet(devNum,exactMatchProfileIndex,actionType,&defaultActionData,defActionEn);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, devNum, exactMatchProfileIndex, actionType);

        /*
            1.3. Call with wrong enum values actionType, other params same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        exactMatchProfileIndex=3;
        UTF_ENUMS_CHECK_MAC(cpssDxChExactMatchProfileDefaultActionSet
                            (devNum, exactMatchProfileIndex, actionType, &defaultActionData, defActionEn),
                            actionType);

        /*
            1.4. Call with actionPtr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChExactMatchProfileDefaultActionSet(devNum,exactMatchProfileIndex,actionType,NULL,defActionEn);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, "%d, defaultActionData = NULL", devNum);

        /*
           1.5. set PCL actionPtr [ pktCmd = CPSS_PACKET_CMD_FORWARD_E,
            mirror{cpuCode = 0, analyzerPortIndex = GT_FALSE},
            matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 },
            qos { egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE ,
                qos [ ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
                profilePrecedence=GT_FALSE] ] },
            redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E,
                data[routerLttIndex=0] },
            policer { policerEnable=GT_FALSE, policerId=0 },
            vlan { egressTaggedModify=GT_FALSE,
                modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE,
                vlanId=100, precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E },
            ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } ].

        Expected: GT_OK and same action as written
        */

        actionType = CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E;
        exactMatchProfileIndex = 5;

        prvSetExactMatchActionDefaultValues(&defaultActionData,CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E);

        st = cpssDxChExactMatchProfileDefaultActionSet(devNum,exactMatchProfileIndex,actionType,&defaultActionData,defActionEn);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "cpssDxChExactMatchProfileDefaultActionSet: %d, %d, %d, %d",
                                         devNum, exactMatchProfileIndex, actionType, defActionEn);

        st = cpssDxChExactMatchProfileDefaultActionGet(devNum,exactMatchProfileIndex,actionType,&defaultActionDataGet,&defActionEnGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchProfileDefaultActionGet: %d, %d, %d", devNum, exactMatchProfileIndex,actionType);
        /* validation values */
        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &defaultActionData, &defaultActionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, exactMatchProfileIndex, actionType);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(defActionEn, defActionEnGet, "get another defActionEnGet than was set: %d", devNum);

        /*
          1.6. Call function with
                actionPtr { egressPolicy [GT_FALSE]
                            mirror { ingressMirrorToAnalyzerIndex[0 / 3 / 6]},
                            redirect {
                               redirectCmd [
                                CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E /
                                CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E /
                                CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_VIRT_ROUTER_E /
                                CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_REPLACE_MAC_SA_E],
                               data { modifyMacSa {macSa [00:00:00:00:00:00 /
                                                          A5:A5:A5:A5:A5:A5 /
                                                          FF:FF:FF:FF:FF:FF],
                                                   arpPtr[0 / 0xA5A5 / BIT_17-1]}}},
                            oam { timeStampEnable [GT_FALSE / GT_TRUE],
                                  offsetIndex [0 / 0xA / BIT_4-1],
                                  oamProcessEnable [GT_FALSE / GT_TRUE],
                                  oamProfile [0 / 1] },
                            sourcePort { assignSourcePortEnable [GT_FALSE / GT_TRUE]
                                     sourcePortValue [0 / 0xA5A5A5A5 / 0xFFFFFFFF] }
                            qos { ingress { up1Cmd[
                              CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_DO_NOT_MODIFY_E /
                              CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_TAG1_UNTAGGED_E /
                              CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_TAG0_UNTAGGED_E /
                              CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_ALL_E],
                                           up1[0 / 5 / BIT_3-1]} }
                            vlan { ingress { vlanId1Cmd[
                         CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_DO_NOT_MODIFY_E /
                         CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_UNTAGGED_E /
                         CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_ALL_E],
                                           vlanId1 [0 / 0xA5A / BIT_12-1]} },
                            flowId [0 / 0xA5A / BIT_12-1],
                            setMacToMe [GT_FALSE / GT_TRUE] }
                and other params from 1.5.
        Expected: GT_OK and same action as written
        */

            defaultActionData.pclAction.egressPolicy = GT_FALSE;
            defaultActionData.pclAction.mirror.mirrorToRxAnalyzerPort = GT_TRUE;

            defaultActionData.pclAction.mirror.ingressMirrorToAnalyzerIndex = 0;
            defaultActionData.pclAction.redirect.redirectCmd =
                                       CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E;
            defaultActionData.pclAction.redirect.data.modifyMacSa.macSa = macSa1;
            defaultActionData.pclAction.redirect.data.modifyMacSa.arpPtr = 0;
            defaultActionData.pclAction.oam.timeStampEnable = GT_FALSE;
            defaultActionData.pclAction.oam.offsetIndex = 0;
            defaultActionData.pclAction.oam.oamProcessEnable = GT_FALSE;
            defaultActionData.pclAction.oam.oamProfile = 0;
            defaultActionData.pclAction.sourcePort.assignSourcePortEnable = GT_FALSE;
            defaultActionData.pclAction.sourcePort.sourcePortValue = 0;
            defaultActionData.pclAction.qos.ingress.up1Cmd =
                           CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_DO_NOT_MODIFY_E;
            defaultActionData.pclAction.qos.ingress.up1 = 0;
            defaultActionData.pclAction.vlan.ingress.vlanId1Cmd =
                      CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_DO_NOT_MODIFY_E;
            defaultActionData.pclAction.vlan.ingress.vlanId1 = 0;
            defaultActionData.pclAction.flowId = 0;
            defaultActionData.pclAction.setMacToMe = GT_FALSE;

            actionType = CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E;

            st = cpssDxChExactMatchProfileDefaultActionSet(devNum,exactMatchProfileIndex,actionType,&defaultActionData,defActionEn);
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "cpssDxChExactMatchProfileDefaultActionSet: %d, %d, %d, %d",
                                             devNum, exactMatchProfileIndex, actionType, defActionEn);

            st = cpssDxChExactMatchProfileDefaultActionGet(devNum,exactMatchProfileIndex,actionType,&defaultActionDataGet,&defActionEnGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchProfileDefaultActionGet: %d, %d, %d", devNum, exactMatchProfileIndex,actionType);
            /* validation values */
            st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &defaultActionData, &defaultActionDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, exactMatchProfileIndex, actionType);

            /* verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(defActionEn, defActionEnGet, "get another defActionEnGet than was set: %d", devNum);

            /*1.7. Call function with
                actionPtr { egressPolicy [GT_TRUE]
                            qos { egress { modifyDscp[
                           CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_KEEP_E /
                           CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_MODIFY_OUTER_E /
                           CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_MODIFY_INNER_E],
                                          modifyUp[
                           CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E /
                           CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E /
                           CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E],
                                          up1ModifyEnable [GT_FALSE / GT_TRUE],
                                          up1[0 / 5 / BIT_3-1]} }
                            vlan { egress { vlanCmd[
                           CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E /
                           CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E /
                           CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E],
                                           vlanId1ModifyEnable [GT_FALSE / GT_TRUE],
                                           vlanId1 [0 / 0xA5A / BIT_12-1]} },
                            channelTypeToOpcodeMapEnable [GT_FALSE / GT_TRUE],
                            tmQueueId [0 / 0x2A5A / BIT_14-1] }
                and other params from 1.6.
            Expected: GT_OK and same action as written */

            defaultActionData.pclAction.egressPolicy = GT_TRUE;
            defaultActionData.pclAction.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
            defaultActionData.pclAction.actionStop = GT_FALSE;
            defaultActionData.pclAction.redirect.data.modifyMacSa.arpPtr = 0;
            defaultActionData.pclAction.sourcePort.sourcePortValue = 0;

            defaultActionData.pclAction.qos.egress.modifyDscp =
                                CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_KEEP_E;
            defaultActionData.pclAction.qos.egress.modifyUp =
                           CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E;
            defaultActionData.pclAction.qos.egress.up1ModifyEnable = GT_FALSE;
            defaultActionData.pclAction.qos.egress.up1 = 0;
            defaultActionData.pclAction.vlan.egress.vlanCmd =
                           CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E;
            defaultActionData.pclAction.vlan.egress.vlanId1ModifyEnable = GT_FALSE;
            defaultActionData.pclAction.vlan.egress.vlanId1 = 0;
            defaultActionData.pclAction.channelTypeToOpcodeMapEnable = GT_FALSE;
            defaultActionData.pclAction.tmQueueId = 0;


            actionType = CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E;

            st = cpssDxChExactMatchProfileDefaultActionSet(devNum,exactMatchProfileIndex,actionType,&defaultActionData,defActionEn);
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "cpssDxChExactMatchProfileDefaultActionSet: %d, %d, %d, %d",
                                             devNum, exactMatchProfileIndex, actionType, defActionEn);

            st = cpssDxChExactMatchProfileDefaultActionGet(devNum,exactMatchProfileIndex,actionType,&defaultActionDataGet,&defActionEnGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchProfileDefaultActionGet: %d, %d, %d", devNum, exactMatchProfileIndex,actionType);
            /* validation values */
            st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &defaultActionData, &defaultActionDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, exactMatchProfileIndex, actionType);

            /* verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(defActionEn, defActionEnGet, "get another defActionEnGet than was set: %d", devNum);




    }

    /* 2. For not active devNumNumices check that function returns non GT_OK.*/
    /* prepare devNumNumice iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;

    /* Go over all non active devNumNumices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChExactMatchProfileDefaultActionSet(devNum,exactMatchProfileIndex,actionType,&defaultActionData,defActionEn);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for devNumNumice id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChExactMatchProfileDefaultActionSet(devNum,exactMatchProfileIndex,actionType,&defaultActionData,defActionEn);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchProfileDefaultActionGet
(
    IN  GT_U8                                    devNum,
    IN  GT_U32                                   exactMatchProfileIndex,
    IN  CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT    actionType,
    OUT CPSS_DXCH_EXACT_MATCH_ACTION_UNT         *actionPtr,
    OUT GT_BOOL                                  *defActionEnPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchProfileDefaultActionGet)
{
/*
    ITERATE_DEVICES (SIP6)
    1.1. Call with non null actionPtr, defActionEnPtr
    Expected: GT_OK.
    1.2. Call with non null actionPtr, defActionEnPtr and exactMatchProfileIndex=16
    Expected: GT_BAD_PARAM.
    1.3. Call with actionPtr [NULL].
    Expected: GT_BAD_PTR.
    1.4. Call with defActionEnPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                                st = GT_OK;
    GT_U8                                    devNum = 0;
    GT_U32                                   exactMatchProfileIndex;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT    actionType;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT         defaultActionDataGet;
    GT_BOOL                                  defActionEnGet;

    /* this feature is on Falcon devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        actionType=CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E;

        /*
             1.1. Call with non null actionPtr, defActionEnPtr
            Expected: GT_OK.
        */
        exactMatchProfileIndex=3;
        st = cpssDxChExactMatchProfileDefaultActionGet(devNum,exactMatchProfileIndex,actionType,&defaultActionDataGet,&defActionEnGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

         /*
             1.2. Call with non null actionPtr, defActionEnPtr and exactMatchProfileIndex=16
            Expected: GT_BAD_PARAM.
        */
        exactMatchProfileIndex=16;
        st = cpssDxChExactMatchProfileDefaultActionGet(devNum,exactMatchProfileIndex,actionType,&defaultActionDataGet,&defActionEnGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);

        /*
            1.3. Call with actionPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        exactMatchProfileIndex=15;
        st = cpssDxChExactMatchProfileDefaultActionGet(devNum,exactMatchProfileIndex,actionType,NULL,&defActionEnGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cpssDxChExactMatchProfileDefaultActionGet = NULL", devNum);

        /*
             1.4. Call with defActionEnPtr [NULL].
                Expected: GT_BAD_PTR.
        */
        exactMatchProfileIndex=15;
        st = cpssDxChExactMatchProfileDefaultActionGet(devNum,exactMatchProfileIndex,actionType,&defaultActionDataGet,NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cpssDxChExactMatchProfileDefaultActionGet = NULL", devNum);

    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */

    /* this feature is on Falcon devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;

    actionType=CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E;
    exactMatchProfileIndex = 1;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChExactMatchProfileDefaultActionGet(devNum,exactMatchProfileIndex,actionType,&defaultActionDataGet,&defActionEnGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChExactMatchProfileDefaultActionGet(devNum,exactMatchProfileIndex,actionType,&defaultActionDataGet,&defActionEnGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/**
* @internal prvCompareExactMatchExpandedStructs function
* @endinternal
*
* @brief   This routine set compares 2 useExpanded structures.
*
* @param[in] devNum                   - device number
* @param[in] name                     -  of tested sctucts objects
* @param[in] actionType               - type of the action to use
* @param[in] firstPtr                 - (pointer to) first struct
* @param[in] secondPtr                - (pointer to) second struct
*/
static GT_STATUS prvCompareExactMatchExpandedStructs
(
    IN GT_CHAR                                              *name,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT                actionType,
    IN CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT     *firstActionPtr,
    IN CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT     *secondActionPtr
)
{
    GT_STATUS   st = GT_OK;

    switch(actionType)
    {
    case CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E:
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionCommand != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionCommand)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionCommand than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionCommand,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionCommand);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionUserDefinedCpuCode != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionUserDefinedCpuCode)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionUserDefinedCpuCode than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionUserDefinedCpuCode,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionUserDefinedCpuCode);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionMirrorToIngressAnalyzerIndex != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionMirrorToIngressAnalyzerIndex)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionMirrorToIngressAnalyzerIndex than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionMirrorToIngressAnalyzerIndex,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionMirrorToIngressAnalyzerIndex);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionRedirectCommand != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionRedirectCommand)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionRedirectCommand than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionRedirectCommand,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionRedirectCommand);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionEgressInterface != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionEgressInterface)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionEgressInterface than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionEgressInterface,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionEgressInterface);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionVrfId != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionVrfId)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionVrfId than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionVrfId,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionVrfId);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionFlowId != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionFlowId)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionFlowId than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionFlowId,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionFlowId);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionRouterLttPtr != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionRouterLttPtr)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionRouterLttPtr than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionRouterLttPtr,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionRouterLttPtr);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionIpclUdbConfigTable != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionIpclUdbConfigTable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionIpclUdbConfigTable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionIpclUdbConfigTable,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionIpclUdbConfigTable);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionVntl2Echo != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionVntl2Echo)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin. than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionVntl2Echo,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionVntl2Echo);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTunnelStart != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTunnelStart)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionTunnelStart than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTunnelStart,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTunnelStart);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionArpPtr != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionArpPtr)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionArpPtr than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionArpPtr,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionArpPtr);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTunnelIndex != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTunnelIndex)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionTunnelIndex than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTunnelIndex,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTunnelIndex);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPcl0OverrideConfigIndex != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPcl0OverrideConfigIndex)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionPcl0OverrideConfigIndex than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPcl0OverrideConfigIndex,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPcl0OverrideConfigIndex);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPcl1OverrideConfigIndex != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPcl1OverrideConfigIndex)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionPcl1OverrideConfigIndex than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPcl1OverrideConfigIndex,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPcl1OverrideConfigIndex);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPcl2OverrideConfigIndex != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPcl2OverrideConfigIndex)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionPcl2OverrideConfigIndex than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPcl2OverrideConfigIndex,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPcl2OverrideConfigIndex);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionIpclConfigIndex != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionIpclConfigIndex)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionIpclConfigIndex than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionIpclConfigIndex,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionIpclConfigIndex);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTsPassengerPacketType != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTsPassengerPacketType)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionTsPassengerPacketType than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTsPassengerPacketType,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTsPassengerPacketType);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionModifyMacDa != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionModifyMacDa)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionModifyMacDa than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionModifyMacDa,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionModifyMacDa);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionModifyMacSa != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionModifyMacSa)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionModifyMacSa than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionModifyMacSa,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionModifyMacSa);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionBindToCentralCounter != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionBindToCentralCounter)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionBindToCentralCounter than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionBindToCentralCounter,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionBindToCentralCounter);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionCentralCounterIndex != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionCentralCounterIndex)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionCentralCounterIndex than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionCentralCounterIndex,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionCentralCounterIndex);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionBindToPolicerMeter != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionBindToPolicerMeter)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionBindToPolicerMeter than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionBindToPolicerMeter,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionBindToPolicerMeter);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionBindToPolicer != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionBindToPolicer)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionBindToPolicer than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionBindToPolicer,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionBindToPolicer);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPolicerIndex != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPolicerIndex)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionPolicerIndex than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPolicerIndex,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPolicerIndex);
            st = GT_FAIL;
        }
         if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionSourceIdSetEnable!= secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionSourceIdSetEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin. than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionSourceIdSetEnable,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionSourceIdSetEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionSourceId != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionSourceId)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionSourceId than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionSourceId,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionSourceId);
            st = GT_FAIL;
        }
         if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionActionStop != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionActionStop)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionActionStop than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionActionStop,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionActionStop);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionBridgeBypass != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionBridgeBypass)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin. than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionBridgeBypass,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionBridgeBypass);
            st = GT_FAIL;
        }
         if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionIngressPipeBypass != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionIngressPipeBypass)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionIngressPipeBypass than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionIngressPipeBypass,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionIngressPipeBypass);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag0VlanPrecedence != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag0VlanPrecedence)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionTag0VlanPrecedence than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag0VlanPrecedence,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag0VlanPrecedence);
            st = GT_FAIL;
        }
         if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionNestedVlanEnable != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionNestedVlanEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionNestedVlanEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionNestedVlanEnable,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionNestedVlanEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag0VlanCmd != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag0VlanCmd)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionTag0VlanCmd than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag0VlanCmd,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag0VlanCmd);
            st = GT_FAIL;
        }
         if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag0VlanId != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag0VlanId)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin. than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag0VlanId,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag0VlanId);
            st = GT_FAIL;
        }
         if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag1VlanCmd != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag1VlanCmd)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionTag1VlanCmd than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag1VlanCmd,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag1VlanCmd);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag1VlanId != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag1VlanId)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionTag1VlanId than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag1VlanId,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag1VlanId);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionQosPrecedence != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionQosPrecedence)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionQosPrecedence than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionQosPrecedence,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionQosPrecedence);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionQosProfile != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionQosProfile)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionQosProfile than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionQosProfile,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionQosProfile);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionModifyDscp != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionModifyDscp)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionModifyDscp than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionModifyDscp,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionModifyDscp);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionModifyTag0 != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionModifyTag0)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionModifyTag0 than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionModifyTag0,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionModifyTag0);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTrustUp != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTrustUp)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionTrustUp than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTrustUp,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTrustUp);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTrustDscp != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTrustDscp)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionTrustDscp than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTrustDscp,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTrustDscp);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTrustExp != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTrustExp)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionTrustExp than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTrustExp,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTrustExp);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionRemapDscp != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionRemapDscp)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionRemapDscp than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionRemapDscp,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionRemapDscp);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag0Up != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag0Up)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionTag0Up than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag0Up,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag0Up);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag1UpCommand != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag1UpCommand)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionTag1UpCommand than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag1UpCommand,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag1UpCommand);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag1Up != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag1Up)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionTag1Up than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag1Up,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTag1Up);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTtPassengerPacketType != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTtPassengerPacketType)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionTtPassengerPacketType than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTtPassengerPacketType,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTtPassengerPacketType);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionCopyTtlExpFromTunnelHeader != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionCopyTtlExpFromTunnelHeader)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionCopyTtlExpFromTunnelHeader than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionCopyTtlExpFromTunnelHeader,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionCopyTtlExpFromTunnelHeader);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTunnelTerminate != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTunnelTerminate)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionTunnelTerminate than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTunnelTerminate,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTunnelTerminate);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionMplsCommand != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionMplsCommand)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionMplsCommand than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionMplsCommand,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionMplsCommand);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionHashMaskIndex != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionHashMaskIndex)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionHashMaskIndex than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionHashMaskIndex,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionHashMaskIndex);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionQosMappingTableIndex != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionQosMappingTableIndex)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionQosMappingTableIndex than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionQosMappingTableIndex,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionQosMappingTableIndex);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionMplsLLspQoSProfileEnable != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionMplsLLspQoSProfileEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionMplsLLspQoSProfileEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionMplsLLspQoSProfileEnable,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionMplsLLspQoSProfileEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionMplsTtl != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionMplsTtl)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionMplsTtl than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionMplsTtl,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionMplsTtl);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionEnableDecrementTtl != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionEnableDecrementTtl)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionEnableDecrementTtl than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionEnableDecrementTtl,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionEnableDecrementTtl);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionSourceEPortAssignmentEnable != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionSourceEPortAssignmentEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionSourceEPortAssignmentEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionSourceEPortAssignmentEnable,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionSourceEPortAssignmentEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionSourceEPort != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionSourceEPort)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionSourceEPort than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionSourceEPort,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionSourceEPort);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTtHeaderLength != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTtHeaderLength)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionTtHeaderLength than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTtHeaderLength,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTtHeaderLength);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionIsPtpPacket != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionIsPtpPacket)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionIsPtpPacket than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionIsPtpPacket,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionIsPtpPacket);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionOamTimeStampEnable != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionOamTimeStampEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionOamTimeStampEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionOamTimeStampEnable,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionOamTimeStampEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPtpTriggerType != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPtpTriggerType)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionPtpTriggerType than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPtpTriggerType,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPtpTriggerType);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPtpOffset != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPtpOffset)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionPtpOffset than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPtpOffset,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPtpOffset);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionOamOffsetIndex != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionOamOffsetIndex)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionOamOffsetIndex than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionOamOffsetIndex,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionOamOffsetIndex);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionOamProcessWhenGalOrOalExistsEnable != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionOamProcessWhenGalOrOalExistsEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionOamProcessWhenGalOrOalExistsEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionOamProcessWhenGalOrOalExistsEnable,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionOamProcessWhenGalOrOalExistsEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionOamProcessEnable != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionOamProcessEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionOamProcessEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionOamProcessEnable,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionOamProcessEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionCwBasedPw != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionCwBasedPw)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionCwBasedPw than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionCwBasedPw,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionCwBasedPw);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTtlExpiryVccvEnable != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTtlExpiryVccvEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionTtlExpiryVccvEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTtlExpiryVccvEnable,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTtlExpiryVccvEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPwe3FlowLabelExist != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPwe3FlowLabelExist)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionPwe3FlowLabelExist than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPwe3FlowLabelExist,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPwe3FlowLabelExist);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPwCwBasedETreeEnable != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPwCwBasedETreeEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionPwCwBasedETreeEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPwCwBasedETreeEnable,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPwCwBasedETreeEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionQosUseUpAsIndexEnable != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionQosUseUpAsIndexEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionQosUseUpAsIndexEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionQosUseUpAsIndexEnable,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionQosUseUpAsIndexEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionRxIsProtectionPath != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionRxIsProtectionPath)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionRxIsProtectionPath than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionRxIsProtectionPath,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionRxIsProtectionPath);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionRxProtectionSwitchEnable != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionRxProtectionSwitchEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionRxProtectionSwitchEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionRxProtectionSwitchEnable,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionRxProtectionSwitchEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionSetMacToMe != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionSetMacToMe)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionSetMacToMe than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionSetMacToMe,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionSetMacToMe);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionOamProfile != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionOamProfile)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionOamProfile than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionOamProfile,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionOamProfile);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionApplyNonDataCwCommand != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionApplyNonDataCwCommand)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionApplyNonDataCwCommand than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionApplyNonDataCwCommand,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionApplyNonDataCwCommand);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionContinueToNextTtiLookup != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionContinueToNextTtiLookup)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionContinueToNextTtiLookup than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionContinueToNextTtiLookup,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionContinueToNextTtiLookup);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionOamChannelTypeToOpcodeMappingEnable != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionOamChannelTypeToOpcodeMappingEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionOamChannelTypeToOpcodeMappingEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionOamChannelTypeToOpcodeMappingEnable,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionOamChannelTypeToOpcodeMappingEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPassengerParsingOfTransitMplsTunnelMode != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPassengerParsingOfTransitMplsTunnelMode)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionPassengerParsingOfTransitMplsTunnelMode than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPassengerParsingOfTransitMplsTunnelMode,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPassengerParsingOfTransitMplsTunnelMode);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPassengerParsingOfTransitNonMplsTransitTunnelEnable != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPassengerParsingOfTransitNonMplsTransitTunnelEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionPassengerParsingOfTransitNonMplsTransitTunnelEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPassengerParsingOfTransitNonMplsTransitTunnelEnable,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionPassengerParsingOfTransitNonMplsTransitTunnelEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionSkipFdbSaLookupEnable != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionSkipFdbSaLookupEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionSkipFdbSaLookupEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionSkipFdbSaLookupEnable,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionSkipFdbSaLookupEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionIpv6SegmentRoutingEndNodeEnable != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionIpv6SegmentRoutingEndNodeEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionIpv6SegmentRoutingEndNodeEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionIpv6SegmentRoutingEndNodeEnable,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionIpv6SegmentRoutingEndNodeEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTunnelHeaderStartL4Enable != secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTunnelHeaderStartL4Enable)
        {
            PRV_UTF_LOG3_MAC("get another %s.ttiExpandedActionOrigin.overwriteExpandedActionTunnelHeaderStartL4Enable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTunnelHeaderStartL4Enable,
                             secondActionPtr->ttiExpandedActionOrigin.overwriteExpandedActionTunnelHeaderStartL4Enable);
            st = GT_FAIL;
        }
        break;
    case CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E:
    case CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E:
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionUserDefinedCpuCode != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionUserDefinedCpuCode)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionUserDefinedCpuCode than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionUserDefinedCpuCode,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionUserDefinedCpuCode);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPktCmd != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPktCmd)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionPktCmd than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPktCmd,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPktCmd);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionRedirectCommand != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionRedirectCommand)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionRedirectCommand than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionRedirectCommand,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionRedirectCommand);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionEgressInterface != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionEgressInterface)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionEgressInterface than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionEgressInterface,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionEgressInterface);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionMacSa!= secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionMacSa)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionMacSa than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionMacSa,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionMacSa);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionRouterLttPtr != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionRouterLttPtr)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionRouterLttPtr than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionRouterLttPtr,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionRouterLttPtr);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVntl2Echo != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVntl2Echo)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionVntl2Echo than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVntl2Echo,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVntl2Echo);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionTunnelStart != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionTunnelStart)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionTunnelStart than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionTunnelStart,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionTunnelStart);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionArpPtr != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionArpPtr)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionArpPtr than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionArpPtr,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionArpPtr);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionTunnelIndex != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionTunnelIndex)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin. than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionTunnelIndex,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionTunnelIndex);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionTsPassengerPacketType != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionTsPassengerPacketType)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionTsPassengerPacketType than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionTsPassengerPacketType,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionTsPassengerPacketType);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVrfId != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVrfId)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionVrfId than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVrfId,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVrfId);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionActionStop != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionActionStop)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionActionStop than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionActionStop,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionActionStop);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionSetMacToMe != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionSetMacToMe)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionSetMacToMe than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionSetMacToMe,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionSetMacToMe);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPCL1OverrideConfigIndex != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPCL1OverrideConfigIndex)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionPCL1OverrideConfigIndex than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPCL1OverrideConfigIndex,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPCL1OverrideConfigIndex);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPCL2OverrideConfigIndex != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPCL2OverrideConfigIndex)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionPCL2OverrideConfigIndex than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPCL2OverrideConfigIndex,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPCL2OverrideConfigIndex);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionIPCLConfigurationIndex != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionIPCLConfigurationIndex)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionLookupConfig than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionIPCLConfigurationIndex,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionIPCLConfigurationIndex);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPolicerEnable != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPolicerEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionPolicerEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPolicerEnable,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPolicerEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPolicerIndex != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPolicerIndex)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionPolicerIndex than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPolicerIndex,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPolicerIndex);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionMirrorMode != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionMirrorMode)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionMirrorMode than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionMirrorMode,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionMirrorMode);
            st = GT_FAIL;
        }
         if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionMirrorTcpRstAndFinPacketsToCpu != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionMirrorTcpRstAndFinPacketsToCpu)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionMirrorTcpRstAndFinPacketsToCpu than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionMirrorTcpRstAndFinPacketsToCpu,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionMirrorTcpRstAndFinPacketsToCpu);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionMirror != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionMirror)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionMirror than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionMirror,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionMirror);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionModifyMacDA != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionModifyMacDA)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionModifyMacDA than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionModifyMacDA,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionModifyMacDA);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionModifyMacSA != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionModifyMacSA)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin. than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionModifyMacSA,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionModifyMacSA);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionBypassBridge != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionBypassBridge)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionBypassBridge than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionBypassBridge,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionBypassBridge);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionBypassIngressPipe != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionBypassIngressPipe)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionBypassIngressPipe than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionBypassIngressPipe,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionBypassIngressPipe);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionNestedVlanEnable != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionNestedVlanEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionNestedVlanEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionNestedVlanEnable,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionNestedVlanEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVlanPrecedence != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVlanPrecedence)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionVlanPrecedence than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVlanPrecedence,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVlanPrecedence);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVlan0Command != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVlan0Command)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionVlan0Command than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVlan0Command,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVlan0Command);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVlan0 != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVlan0)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionVlan0 than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVlan0,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVlan0);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVlan1Command != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVlan1Command)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionVlan1Command than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVlan1Command,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVlan1Command);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVlan1 != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVlan1)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionVlan1 than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVlan1,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionVlan1);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionSourceIdEnable != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionSourceIdEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionSourceIdEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionSourceIdEnable,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionSourceIdEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionSourceId != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionSourceId)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionSourceId than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionSourceId,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionSourceId);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionMatchCounterEnable != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionMatchCounterEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionMatchCounterEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionMatchCounterEnable,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionMatchCounterEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionMatchCounterIndex != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionMatchCounterIndex)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionMatchCounterIndex than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionMatchCounterIndex,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionMatchCounterIndex);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionQosProfileMakingEnable != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionQosProfileMakingEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionQosProfileMakingEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionQosProfileMakingEnable,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionQosProfileMakingEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionQosPrecedence != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionQosPrecedence)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionQosPrecedence than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionQosPrecedence,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionQosPrecedence);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionQoSProfile != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionQoSProfile)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionQoSProfile than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionQoSProfile,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionQoSProfile);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionQoSModifyDSCP != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionQoSModifyDSCP)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionQoSModifyDSCP than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionQoSModifyDSCP,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionQoSModifyDSCP);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionUp1Command != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionUp1Command)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionUp1Command than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionUp1Command,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionUp1Command);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionModifyUp1 != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionModifyUp1)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionModifyUp1 than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionModifyUp1,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionModifyUp1);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionUp1 != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionUp1)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionUp1 than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionUp1,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionUp1);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionDscpExp != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionDscpExp)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionDscpExp than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionDscpExp,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionDscpExp);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionUp0 != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionUp0)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionUp0 than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionUp0,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionUp0);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionModifyUp0 != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionModifyUp0)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionModifyUp0 than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionModifyUp0,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionModifyUp0);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionOamTimestampEnable != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionOamTimestampEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionOamTimestampEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionOamTimestampEnable,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionOamTimestampEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionOamOffsetIndex != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionOamOffsetIndex)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionOamOffsetIndex than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionOamOffsetIndex,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionOamOffsetIndex);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionOamProcessingEnable != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionOamProcessingEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionOamProcessingEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionOamProcessingEnable,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionOamProcessingEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionOamProfile != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionOamProfile)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionOamProfile than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionOamProfile,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionOamProfile);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionOamChannelTypeToOpcodeMappingEnable != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionOamChannelTypeToOpcodeMappingEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionOamChannelTypeToOpcodeMappingEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionOamChannelTypeToOpcodeMappingEnable,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionOamChannelTypeToOpcodeMappingEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionFlowId != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionFlowId)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin. than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionFlowId,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionFlowId);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionSourcePortEnable != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionSourcePortEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionSourcePortEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionSourcePortEnable,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionSourcePortEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionSourcePort != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionSourcePort)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin. than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionSourcePort,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionSourcePort);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionLatencyMonitor != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionLatencyMonitor)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionLatencyMonitor than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionLatencyMonitor,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionLatencyMonitor);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionLatencyMonitorEnable != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionLatencyMonitorEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionLatencyMonitorEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionLatencyMonitorEnable,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionLatencyMonitorEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionSkipFdbSaLookup != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionSkipFdbSaLookup)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionSkipFdbSaLookup than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionSkipFdbSaLookup,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionSkipFdbSaLookup);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionTriggerInterrupt != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionTriggerInterrupt)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionTriggerInterrupt than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionTriggerInterrupt,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionTriggerInterrupt);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPhaMetadataAssignEnable != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPhaMetadataAssignEnable)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionPhaMetadataAssignEnable than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPhaMetadataAssignEnable,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPhaMetadataAssignEnable);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPhaThreadNumber != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPhaThreadNumber)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionPhaThreadNumber than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPhaThreadNumber,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionPhaThreadNumber);
            st = GT_FAIL;
        }
        if(firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionCutThroughTerminateId != secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionCutThroughTerminateId)
        {
            PRV_UTF_LOG3_MAC("get another %s.pclExpandedActionOrigin.overwriteExpandedActionCutThroughTerminateId than was set, expected = %d, received = %d\n",
                             name, firstActionPtr->pclExpandedActionOrigin.overwriteExpandedActionCutThroughTerminateId,
                             secondActionPtr->pclExpandedActionOrigin.overwriteExpandedActionCutThroughTerminateId);
            st = GT_FAIL;
        }
        break;
    default:
        st = GT_BAD_PARAM;
        break;
    }
    return st;
}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchExpandedActionSet
(
    IN GT_U8                                            devNum,
    IN GT_U32                                           expandedActionIndex,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT            actionType,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_UNT                 *actionPtr,
    IN CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT *expandedActionOriginPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchExpandedActionSet)
{
/*
    ITERATE_DEVICES (SIP6)
    1.1. Call cpssDxChExactMatchExpandedActionSet with non-NULL pointers,
         actionPtr{tunnelTerminate[GT_TRUE],
                   passengerPacketType[CPSS_DXCH_TTI_PASSENGER_IPV4_E],
                   copyTtlFromTunnelHeader[GT_FALSE],
                   command [CPSS_PACKET_CMD_FORWARD_E],
                   redirectCommand[CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E],
                   egressInterface{type[CPSS_INTERFACE_TRUNK_E],
                                   trunkId[2]},

                   tunnelStart [GT_TRUE],
                   tunnelStartPtr[0],
                   targetIsTrunk[GT_FALSE],

                   sourceIdSetEnable[GT_TRUE],
                   sourceId[0],

                   vlanCmd[CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E],
                   vlanId[100],
                   vlanPrecedence[CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E],
                   nestedVlanEnable[GT_FALSE],

                   bindToPolicer[GT_FALSE],

                   qosPrecedence[CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E],
                   qosTrustMode[CPSS_DXCH_TTI_QOS_UNTRUST_E],
                   qosProfile[0],
                   modifyUpEnable[CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_UP_E],
                   modifyDscpEnable[CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_DSCP_E],
                   up[0],

                   mirrorToIngressAnalyzerEnable[GT_FALSE],
                   vntl2Echo[GT_FALSE],
                   bridgeBypass[GT_FALSE],
                   actionStop[GT_FALSE]
              }
    Expected: GT_OK and same action as written
    1.2. Call with wrong values expandedActionIndex, other params same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.3. Call with wrong enum values actionType, other params same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.4. Call with actionPtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.

    1.5. set PCL actionPtr [ pktCmd = CPSS_PACKET_CMD_FORWARD_E,
        mirror{cpuCode = 0, analyzerPortIndex = GT_FALSE},
        matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 },
        qos { egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE ,
            qos [ ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
            profilePrecedence=GT_FALSE] ] },
        redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E,
            data[routerLttIndex=0] },
        policer { policerEnable=GT_FALSE, policerId=0 },
        vlan { egressTaggedModify=GT_FALSE,
            modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE,
            vlanId=100, precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E },
        ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } ].

    Expected: GT_OK and same action as written

    1.6. Call function with
            actionPtr { egressPolicy [GT_FALSE]
                        mirror { ingressMirrorToAnalyzerIndex[0 / 3 / 6]},
                        redirect {
                           redirectCmd [
                            CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E /
                            CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E /
                            CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_VIRT_ROUTER_E /
                            CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_REPLACE_MAC_SA_E],
                           data { modifyMacSa {macSa [00:00:00:00:00:00 /
                                                      A5:A5:A5:A5:A5:A5 /
                                                      FF:FF:FF:FF:FF:FF],
                                               arpPtr[0 / 0xA5A5 / BIT_17-1]}}},
                        oam { timeStampEnable [GT_FALSE / GT_TRUE],
                              offsetIndex [0 / 0xA / BIT_4-1],
                              oamProcessEnable [GT_FALSE / GT_TRUE],
                              oamProfile [0 / 1] },
                        sourcePort { assignSourcePortEnable [GT_FALSE / GT_TRUE]
                                 sourcePortValue [0 / 0xA5A5A5A5 / 0xFFFFFFFF] }
                        qos { ingress { up1Cmd[
                          CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_DO_NOT_MODIFY_E /
                          CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_TAG1_UNTAGGED_E /
                          CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_TAG0_UNTAGGED_E /
                          CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_ALL_E],
                                       up1[0 / 5 / BIT_3-1]} }
                        vlan { ingress { vlanId1Cmd[
                     CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_DO_NOT_MODIFY_E /
                     CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_UNTAGGED_E /
                     CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_ALL_E],
                                       vlanId1 [0 / 0xA5A / BIT_12-1]} },
                        flowId [0 / 0xA5A / BIT_12-1],
                        setMacToMe [GT_FALSE / GT_TRUE] }
            and other params from 1.1.
    Expected: GT_OK and same action as written
    1.7. Call function with
            actionPtr { egressPolicy [GT_TRUE]
                        qos { egress { modifyDscp[
                       CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_KEEP_E /
                       CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_MODIFY_OUTER_E /
                       CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_MODIFY_INNER_E],
                                      modifyUp[
                       CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E /
                       CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E /
                       CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E],
                                      up1ModifyEnable [GT_FALSE / GT_TRUE],
                                      up1[0 / 5 / BIT_3-1]} }
                        vlan { egress { vlanCmd[
                       CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E /
                       CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E /
                       CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E],
                                       vlanId1ModifyEnable [GT_FALSE / GT_TRUE],
                                       vlanId1 [0 / 0xA5A / BIT_12-1]} },
                        channelTypeToOpcodeMapEnable [GT_FALSE / GT_TRUE],
                        tmQueueId [0 / 0x2A5A / BIT_14-1] }
            and other params from 1.41.
    Expected: GT_OK and same action as written

    1.8. Call with more then 8 reduced bytes. other params same as in 1.1.
         Expected: GT_BAD_PARAM.
    */
    GT_STATUS                                       st = GT_OK;
    GT_U8                                           devNum = 0;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT           actionType;
    GT_U32                                          expandedActionIndex;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT                actionData;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT                actionDataGet;
    CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT   expandedActionOriginData;
    CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT   expandedActionOriginDataGet;

    GT_ETHERADDR                             macSa1 = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

    expandedActionIndex = 2;
    actionType = CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E;

    cpssOsBzero((GT_VOID*) &actionData, sizeof(actionData));
    cpssOsBzero((GT_VOID*) &actionDataGet, sizeof(actionDataGet));

    cpssOsMemSet(&expandedActionOriginData,0, sizeof(expandedActionOriginData));
    cpssOsMemSet(&expandedActionOriginDataGet,0, sizeof(expandedActionOriginDataGet));

    expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionActionStop=GT_TRUE;
    expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionBridgeBypass=GT_TRUE;
    expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionCommand=GT_TRUE;
    expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionEgressInterface=GT_TRUE;
    expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionFlowId=GT_TRUE;
    expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionRedirectCommand=GT_TRUE;

    /* this feature is on Falcon devNumNumices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;

    /* 1. Go over all active devNumNumices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*  actionPtr{ tunnelTerminate[GT_FALSE], passengerPacketType[CPSS_DXCH_TTI_PASSENGER_IPV4_E],
            copyTtlFromTunnelHeader[GT_FALSE], command [CPSS_PACKET_CMD_FORWARD_E],
            redirectCommand[CPSS_DXCH_TTI_NO_REDIRECT_E], egressInterface{type[CPSS_INTERFACE_TRUNK_E],
            trunkId[0], vidx[0], vlanId[0], devNumNumPort {0, 0}}, tunnelStart [GT_FALSE], tunnelStartPtr[0],
            routerLookupPtr[0], vrfId[0], targetIsTrunk[GT_FALSE], virtualSrcPort[0], virtualSrcdevNumNum[0],
            sourceIdSetEnable[GT_FALSE], sourceId[0], vlanCmd[CPSS_DXCH_TTI_VLAN_DO_NOT_MODIFY_E],
            vlanId[0], vlanPrecedence[CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E],
            nestedVlanEnable[GT_FALSE], bindToPolicer[GT_FALSE], policerIndex[0],
            qosPrecedence[CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E],
            qosTrustMode[CPSS_DXCH_TTI_QOS_KEEP_PREVIOUS_E], qosProfile[0],
            modifyUpEnable[CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_UP_E], modifyDscpEnable[CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_DSCP_E],
            up[0], remapDSCP[GT_FALSE], mirrorToIngressAnalyzerEnable[GT_FALSE],
            userDefinedCpuCode[CPSS_NET_CONTROL_E],
            vntl2Echo[GT_FALSE], bridgeBypass[GT_FALSE], actionStop[GT_FALSE] }
        */

        prvSetExactMatchActionDefaultValues(&actionData,CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E);

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
        /* validation values */
        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

       /*
            1.2. Call with out of range expandedActionIndex [16],
                           other params same as in 1.1.
            Expected: non GT_OK.
        */
        expandedActionIndex = 16;

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, devNum, expandedActionIndex, actionType);

        /*
            1.3. Call with wrong enum values actionType, other params same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        expandedActionIndex=3;
        UTF_ENUMS_CHECK_MAC(cpssDxChExactMatchExpandedActionSet
                            (devNum, expandedActionIndex, actionType, &actionData, &expandedActionOriginDataGet),
                            actionType);

        /*
            1.4. Call with actionPtr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,NULL,&expandedActionOriginData);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, "%d, actionData = NULL", devNum);

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, "%d, expandedActionOriginData = NULL", devNum);

        /*
           1.5. set PCL actionPtr [ pktCmd = CPSS_PACKET_CMD_FORWARD_E,
            mirror{cpuCode = 0, analyzerPortIndex = GT_FALSE},
            matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 },
            qos { egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE ,
                qos [ ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
                profilePrecedence=GT_FALSE] ] },
            redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E,
                data[routerLttIndex=0] },
            policer { policerEnable=GT_FALSE, policerId=0 },
            vlan { egressTaggedModify=GT_FALSE,
                modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE,
                vlanId=100, precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E },
            ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } ].

        Expected: GT_OK and same action as written
        */

        actionType = CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E;
        expandedActionIndex = 5;

        cpssOsMemSet(&expandedActionOriginData,0, sizeof(expandedActionOriginData));
        cpssOsMemSet(&expandedActionOriginDataGet,0, sizeof(expandedActionOriginDataGet));

        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionCutThroughTerminateId=GT_TRUE;
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPktCmd=GT_TRUE;
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSkipFdbSaLookup=GT_TRUE;
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionRedirectCommand=GT_TRUE;

        prvSetExactMatchActionDefaultValues(&actionData,CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E);

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
        /* validation values */
        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        /*
          1.6. Call function with
                actionPtr { egressPolicy [GT_FALSE]
                            mirror { ingressMirrorToAnalyzerIndex[0 / 3 / 6]},
                            redirect {
                               redirectCmd [
                                CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E /
                                CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E /
                                CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_VIRT_ROUTER_E /
                                CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_REPLACE_MAC_SA_E],
                               data { modifyMacSa {macSa [00:00:00:00:00:00 /
                                                          A5:A5:A5:A5:A5:A5 /
                                                          FF:FF:FF:FF:FF:FF],
                                                   arpPtr[0 / 0xA5A5 / BIT_17-1]}}},
                            oam { timeStampEnable [GT_FALSE / GT_TRUE],
                                  offsetIndex [0 / 0xA / BIT_4-1],
                                  oamProcessEnable [GT_FALSE / GT_TRUE],
                                  oamProfile [0 / 1] },
                            sourcePort { assignSourcePortEnable [GT_FALSE / GT_TRUE]
                                     sourcePortValue [0 / 0xA5A5A5A5 / 0xFFFFFFFF] }
                            qos { ingress { up1Cmd[
                              CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_DO_NOT_MODIFY_E /
                              CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_TAG1_UNTAGGED_E /
                              CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_TAG0_UNTAGGED_E /
                              CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_ALL_E],
                                           up1[0 / 5 / BIT_3-1]} }
                            vlan { ingress { vlanId1Cmd[
                         CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_DO_NOT_MODIFY_E /
                         CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_UNTAGGED_E /
                         CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_ALL_E],
                                           vlanId1 [0 / 0xA5A / BIT_12-1]} },
                            flowId [0 / 0xA5A / BIT_12-1],
                            setMacToMe [GT_FALSE / GT_TRUE] }
                and other params from 1.5.
        Expected: GT_OK and same action as written
        */

            actionData.pclAction.egressPolicy = GT_FALSE;
            actionData.pclAction.mirror.mirrorToRxAnalyzerPort = GT_TRUE;

            actionData.pclAction.mirror.ingressMirrorToAnalyzerIndex = 0;
            actionData.pclAction.redirect.redirectCmd =
                                       CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E;
            actionData.pclAction.redirect.data.modifyMacSa.macSa = macSa1;
            actionData.pclAction.redirect.data.modifyMacSa.arpPtr = 0;
            actionData.pclAction.oam.timeStampEnable = GT_FALSE;
            actionData.pclAction.oam.offsetIndex = 0;
            actionData.pclAction.oam.oamProcessEnable = GT_FALSE;
            actionData.pclAction.oam.oamProfile = 0;
            actionData.pclAction.sourcePort.assignSourcePortEnable = GT_FALSE;
            actionData.pclAction.sourcePort.sourcePortValue = 0;
            actionData.pclAction.qos.ingress.up1Cmd =
                           CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_DO_NOT_MODIFY_E;
            actionData.pclAction.qos.ingress.up1 = 0;
            actionData.pclAction.vlan.ingress.vlanId1Cmd =
                      CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_DO_NOT_MODIFY_E;
            actionData.pclAction.vlan.ingress.vlanId1 = 0;
            actionData.pclAction.flowId = 0;
            actionData.pclAction.setMacToMe = GT_FALSE;

            actionType = CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E;

            cpssOsMemSet(&expandedActionOriginData,0, sizeof(expandedActionOriginData));
            cpssOsMemSet(&expandedActionOriginDataGet,0, sizeof(expandedActionOriginDataGet));

            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionCutThroughTerminateId=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPktCmd=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPhaMetadata=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSkipFdbSaLookup=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionRedirectCommand=GT_TRUE;

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
            /* validation values */
            st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            /*1.7. Call function with
                actionPtr { egressPolicy [GT_TRUE]
                            qos { egress { modifyDscp[
                           CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_KEEP_E /
                           CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_MODIFY_OUTER_E /
                           CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_MODIFY_INNER_E],
                                          modifyUp[
                           CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E /
                           CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E /
                           CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E],
                                          up1ModifyEnable [GT_FALSE / GT_TRUE],
                                          up1[0 / 5 / BIT_3-1]} }
                            vlan { egress { vlanCmd[
                           CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E /
                           CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E /
                           CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E],
                                           vlanId1ModifyEnable [GT_FALSE / GT_TRUE],
                                           vlanId1 [0 / 0xA5A / BIT_12-1]} },
                            channelTypeToOpcodeMapEnable [GT_FALSE / GT_TRUE],
                            tmQueueId [0 / 0x2A5A / BIT_14-1] }
                and other params from 1.6.
            Expected: GT_OK and same action as written */

            actionData.pclAction.egressPolicy = GT_TRUE;
            actionData.pclAction.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
            actionData.pclAction.actionStop = GT_FALSE;
            actionData.pclAction.redirect.data.modifyMacSa.arpPtr = 0;
            actionData.pclAction.sourcePort.sourcePortValue = 0;

            actionData.pclAction.qos.egress.modifyDscp =
                                CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_KEEP_E;
            actionData.pclAction.qos.egress.modifyUp =
                           CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E;
            actionData.pclAction.qos.egress.up1ModifyEnable = GT_FALSE;
            actionData.pclAction.qos.egress.up1 = 0;
            actionData.pclAction.vlan.egress.vlanCmd =
                           CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E;
            actionData.pclAction.vlan.egress.vlanId1ModifyEnable = GT_FALSE;
            actionData.pclAction.vlan.egress.vlanId1 = 0;
            actionData.pclAction.channelTypeToOpcodeMapEnable = GT_FALSE;
            actionData.pclAction.tmQueueId = 0;

            cpssOsMemSet(&expandedActionOriginData,0, sizeof(expandedActionOriginData));
            cpssOsMemSet(&expandedActionOriginDataGet,0, sizeof(expandedActionOriginDataGet));

            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionCutThroughTerminateId=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPktCmd=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionActionStop=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSkipFdbSaLookup=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionRedirectCommand=GT_TRUE;

            actionType = CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E;

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
            /* validation values */
            st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

            /* 1.8. Call with more 9 reduced bytes. other params same as in 1.1.
            Expected: GT_BAD_PARAM.*/
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionUp0=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionModifyUp0=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionMatchCounterEnable=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionMatchCounterIndex=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionVlan1=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionLatencyMonitor=GT_TRUE;
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionUserDefinedCpuCode=GT_TRUE;

            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);

            /* remove one byte from the reduced */
            expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionUserDefinedCpuCode=GT_FALSE;
            st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                             devNum, expandedActionIndex, actionType);
    }

    /* 2. For not active devNumNumices check that function returns non GT_OK.*/
    /* prepare devNumNumice iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;

    /* Go over all non active devNumNumices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for devNumNumice id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchExpandedActionGet
(
    IN GT_U8                                                devNum,
    IN GT_U32                                               expandedActionIndex,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT                actionType,
    OUT CPSS_DXCH_EXACT_MATCH_ACTION_UNT                    *actionPtr,
    OUT CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT    *expandedActionOriginPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchExpandedActionGet)
{
/*
    ITERATE_DEVICES (SIP6)
    1.1. Call with non null actionPtr, expandedActionOriginPtr
    Expected: GT_BAD_VALUE - the entry was not defined yet.
    1.2. Call with non null actionPtr, expandedActionOriginPtr and expandedActionIndex=16
    Expected: GT_BAD_PARAM.
    1.3. Call with actionPtr [NULL].
    Expected: GT_BAD_PTR.
    1.4. Call with expandedActionOriginPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                                        st = GT_OK;
    GT_U8                                            devNum = 0;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT            actionType;
    GT_U32                                           expandedActionIndex;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT                 actionDataGet;
    CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT expandedActionOriginDataGet;

    expandedActionIndex = 2;
    actionType = CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E;

    cpssOsBzero((GT_VOID*) &actionDataGet, sizeof(actionDataGet));
    cpssOsBzero((GT_VOID*) &expandedActionOriginDataGet, sizeof(expandedActionOriginDataGet));

    /* this feature is on Falcon devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        actionType=CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E;

        /*
             1.1. Call with non null actionPtr, expandedActionOriginPtr
            Expected: GT_BAD_VALUE - the entry was not defined yet.
        */
        expandedActionIndex=3;
        st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_VALUE, st, devNum);

         /*
             1.2. Call with non null actionPtr, expandedActionOriginPtr and expandedActionIndex=16
            Expected: GT_BAD_PARAM.
        */
        expandedActionIndex=16;
        st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);

        /*
            1.3. Call with actionPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        expandedActionIndex=15;
        st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,NULL,&expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cpssDxChExactMatchProfileDefaultActionGet = NULL", devNum);

        /*
             1.4. Call with expandedActionOriginPtr [NULL].
                Expected: GT_BAD_PTR.
        */
        expandedActionIndex=15;
        st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cpssDxChExactMatchProfileDefaultActionGet = NULL", devNum);

    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */

    /* this feature is on Falcon devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;

    actionType=CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E;
    expandedActionIndex = 1;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/**
* @internal prvCompareExactMatchEntry function
* @endinternal
*
* @brief   This routine set compares 2 Exact Match Entry
*          structures.
*
* @param[in] devNum                   - device number
* @param[in] name                     -  of tested sctucts objects
* @param[in] actionType               - type of the action to use
* @param[in] firstPtr                 - (pointer to) first struct
* @param[in] secondPtr                - (pointer to) second struct
*/
static GT_STATUS prvCompareExactMatchEntry
(
    IN GT_CHAR                            *name,
    IN CPSS_DXCH_EXACT_MATCH_ENTRY_STC    *firstEntryPtr,
    IN CPSS_DXCH_EXACT_MATCH_ENTRY_STC    *secondEntryPtr
)
{
    GT_STATUS   st = GT_OK;
    GT_BOOL     isEqual;
    GT_U32      numOfBytes;

    if(firstEntryPtr->lookupNum != secondEntryPtr->lookupNum)
    {
        PRV_UTF_LOG3_MAC("get another %s lookupNum than was set, expected = %d, received = %d\n",
                         name, firstEntryPtr->lookupNum, secondEntryPtr->lookupNum);
        st = GT_FAIL;
    }
    if(firstEntryPtr->key.keySize != secondEntryPtr->key.keySize)
    {
        PRV_UTF_LOG3_MAC("get another %s key.keySize than was set, expected = %d, received = %d\n",
                         name, firstEntryPtr->key.keySize, secondEntryPtr->key.keySize);
        st = GT_FAIL;
    }

    switch (firstEntryPtr->key.keySize)
    {
    case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E:
        numOfBytes=5;
        break;
    case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E:
        numOfBytes=19;
        break;
    case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E:
        numOfBytes=33;
        break;
    case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E:
        numOfBytes=47;
        break;
    default:
        PRV_UTF_LOG1_MAC("illegal keySize= %d\n",firstEntryPtr->key.keySize);
        return GT_FAIL;
    }
    isEqual = (0 == cpssOsMemCmp(
          (GT_VOID*) &firstEntryPtr->key.pattern,
          (GT_VOID*) &secondEntryPtr->key.pattern,
          numOfBytes*sizeof(GT_U8))) ? GT_TRUE : GT_FALSE;
    if(isEqual != GT_TRUE)
    {
        PRV_UTF_LOG0_MAC("get another key.pattern than was set\n");

        st = GT_FAIL;
    }
    return st;
}

/*----------------------------------------------------------------------------*/
/*
cpssDxChExactMatchPortGroupEntrySet
(
    IN GT_U8                                    devNum,
    IN GT_PORT_GROUPS_BMP                       portGroupsBmp,
    IN GT_U32                                   index,
    IN GT_U32                                   expandedActionIndex,
    IN CPSS_DXCH_EXACT_MATCH_ENTRY_STC          *entryPtr,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT    actionType,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_UNT         *actionPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchPortGroupEntrySet)
{
/*
    ITERATE_DEVICES (SIP6)
    1.1. Call cpssDxChExactMatchExpandedActionSet with non-NULL pointers,
         actionPtr{tunnelTerminate[GT_TRUE],
                   passengerPacketType[CPSS_DXCH_TTI_PASSENGER_IPV4_E],
                   copyTtlFromTunnelHeader[GT_FALSE],
                   command [CPSS_PACKET_CMD_FORWARD_E],
                   redirectCommand[CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E],
                   egressInterface{type[CPSS_INTERFACE_TRUNK_E],
                                   trunkId[2]},

                   tunnelStart [GT_TRUE],
                   tunnelStartPtr[0],
                   targetIsTrunk[GT_FALSE],

                   sourceIdSetEnable[GT_TRUE],
                   sourceId[0],

                   vlanCmd[CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E],
                   vlanId[100],
                   vlanPrecedence[CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E],
                   nestedVlanEnable[GT_FALSE],

                   bindToPolicer[GT_FALSE],

                   qosPrecedence[CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E],
                   qosTrustMode[CPSS_DXCH_TTI_QOS_UNTRUST_E],
                   qosProfile[0],
                   modifyUpEnable[CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_UP_E],
                   modifyDscpEnable[CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_DSCP_E],
                   up[0],

                   mirrorToIngressAnalyzerEnable[GT_FALSE],
                   vntl2Echo[GT_FALSE],
                   bridgeBypass[GT_FALSE],
                   actionStop[GT_FALSE]
              }
    Expected: GT_OK and same action as written
    1.2. Call cpssDxChExactMatchPortGroupEntrySet index=0,keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E,
        expandedActionIndexand other params same as in 1.1.
    Expected: GT_OK and same action as written
    1.3. Call with wrong enum values actionType, other params same as in 1.2.
    Expected: GT_BAD_PARAM.
    1.4. Call with wrong actionType that do not fit the action type TTI in the expanded,
        other params same as in 1.2.
    Expected: GT_BAD_PARAM.
    1.5. Call with wrong value expandedActionIndex - not valid, other params same as in 1.2.
    Expected: GT_BAD_PARAM.
    1.6. Call with wrong value index - that do not fit the keyType, other params same as in 1.2.
    Expected: GT_BAD_PARAM.
    1.7. Call with wrong value lookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E other params same as in 1.2.
    Expected: GT_BAD_PARAM.
    1.8. Call with entryPtr [NULL], other params same as in 1.2.
    Expected: GT_BAD_PTR.
    1.9. Call with actionPtr [NULL], other params same as in 1.2.
    Expected: GT_BAD_PTR.

    1.10. set PCL actionPtr [ pktCmd = CPSS_PACKET_CMD_FORWARD_E,
        mirror{cpuCode = 0, analyzerPortIndex = GT_FALSE},
        matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 },
        qos { egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE ,
            qos [ ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
            profilePrecedence=GT_FALSE] ] },
        redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E,
            data[routerLttIndex=0] },
        policer { policerEnable=GT_FALSE, policerId=0 },
        vlan { egressTaggedModify=GT_FALSE,
            modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE,
            vlanId=100, precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E },
        ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } ].

    Expected: GT_OK and same action as written

    1.11. Call with wrong actionType that do not fit the action type PCL in the expanded,
        other params same as in 1.2.
        Expected: GT_BAD_PARAM.

    1.12. invalidate entries defined
        Expected: GT_TRUE.*/

    GT_STATUS                                           st = GT_OK;
    GT_U8                                               i=0;
    GT_U8                                               devNum = 0;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT               actionType;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT               actionTypeGet;
    GT_U32                                              expandedActionIndex;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT                    actionData;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT                    actionDataGet;
    CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT    expandedActionOriginData;
    CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT    expandedActionOriginDataGet;


    GT_U32                                   index=0;
    CPSS_DXCH_EXACT_MATCH_ENTRY_STC          exactMatchEntry;
    CPSS_DXCH_EXACT_MATCH_ENTRY_STC          exactMatchEntryGet;
    GT_BOOL                                  valid;
    GT_BOOL                                  exactMatchActivityStatus;

    expandedActionIndex = 2;
    actionType = CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E;

    cpssOsBzero((GT_VOID*) &actionData, sizeof(actionData));
    cpssOsBzero((GT_VOID*) &actionDataGet, sizeof(actionDataGet));
    cpssOsBzero((GT_VOID*) &exactMatchEntry, sizeof(exactMatchEntry));
    cpssOsBzero((GT_VOID*) &exactMatchEntryGet, sizeof(exactMatchEntryGet));
    cpssOsBzero((GT_VOID*) &exactMatchEntry.key.pattern, sizeof(exactMatchEntry.key.pattern));

    cpssOsMemSet(&expandedActionOriginData,0, sizeof(expandedActionOriginData));
    cpssOsMemSet(&expandedActionOriginDataGet,0, sizeof(expandedActionOriginDataGet));

    expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionActionStop=GT_TRUE;
    expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionBridgeBypass=GT_TRUE;
    expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionCommand=GT_TRUE;
    expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionEgressInterface=GT_TRUE;
    expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionFlowId=GT_TRUE;
    expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionRedirectCommand=GT_TRUE;

    /* this feature is on Falcon devNumNumices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;

    /* 1. Go over all active devNumNumices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*  actionPtr{ tunnelTerminate[GT_FALSE], passengerPacketType[CPSS_DXCH_TTI_PASSENGER_IPV4_E],
            copyTtlFromTunnelHeader[GT_FALSE], command [CPSS_PACKET_CMD_FORWARD_E],
            redirectCommand[CPSS_DXCH_TTI_NO_REDIRECT_E], egressInterface{type[CPSS_INTERFACE_TRUNK_E],
            trunkId[0], vidx[0], vlanId[0], devNumNumPort {0, 0}}, tunnelStart [GT_FALSE], tunnelStartPtr[0],
            routerLookupPtr[0], vrfId[0], targetIsTrunk[GT_FALSE], virtualSrcPort[0], virtualSrcdevNumNum[0],
            sourceIdSetEnable[GT_FALSE], sourceId[0], vlanCmd[CPSS_DXCH_TTI_VLAN_DO_NOT_MODIFY_E],
            vlanId[0], vlanPrecedence[CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E],
            nestedVlanEnable[GT_FALSE], bindToPolicer[GT_FALSE], policerIndex[0],
            qosPrecedence[CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E],
            qosTrustMode[CPSS_DXCH_TTI_QOS_KEEP_PREVIOUS_E], qosProfile[0],
            modifyUpEnable[CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_UP_E], modifyDscpEnable[CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_DSCP_E],
            up[0], remapDSCP[GT_FALSE], mirrorToIngressAnalyzerEnable[GT_FALSE],
            userDefinedCpuCode[CPSS_NET_CONTROL_E],
            vntl2Echo[GT_FALSE], bridgeBypass[GT_FALSE], actionStop[GT_FALSE] }
        */

        prvSetExactMatchActionDefaultValues(&actionData,CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E);

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
        /* validation values */
        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        /*
         1.2. Call cpssDxChExactMatchPortGroupEntrySet index=0,keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E,
            expandedActionIndex and other params same as in 1.1.
            Expected: GT_OK and same action as written */

         index=0;
         exactMatchEntry.key.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;
         for (i=0;i<5;i++)/* 5 bytes in CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E*/
         {
             exactMatchEntry.key.pattern[i] = i;
         }
         exactMatchEntry.lookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;

         st = cpssDxChExactMatchPortGroupEntrySet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,
                                              expandedActionIndex,&exactMatchEntry,actionType,&actionData);
         UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, index, expandedActionIndex,actionType);

         st = cpssDxChExactMatchPortGroupEntryGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,&valid,
                                                  &actionTypeGet,&actionDataGet,&exactMatchEntryGet,&expandedActionIndex);
         UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChExactMatchPortGroupEntryGet: %d, %d, %d\n", devNum, index);
         /* validation values */
         UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, valid,"error - entry is not valid\n");
         UTF_VERIFY_EQUAL1_STRING_MAC(actionType, actionTypeGet,"get another actionTypeGet than was set: %d\n", devNum);

         st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
         UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d\n",devNum, expandedActionIndex, actionType);

         st = prvCompareExactMatchEntry("exactMatchEntry", &exactMatchEntry, &exactMatchEntryGet);
         UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"exactMatchEntry != exactMatchEntryGet\n");

         /* check activity status */
         st = cpssDxChExactMatchActivityStatusGet(devNum,
                                                  CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                  index,
                                                  GT_FALSE,
                                                  &exactMatchActivityStatus);
         UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChExactMatchActivityStatusGet: %d", devNum);
         UTF_VERIFY_EQUAL0_STRING_MAC(GT_FALSE, exactMatchActivityStatus,"error - entry is active\n");


        /*1.3. Call with wrong enum values actionType, other params same as in 1.2.
        Expected: GT_BAD_PARAM.*/
         st = cpssDxChExactMatchPortGroupEntrySet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,
                                                  expandedActionIndex,&exactMatchEntry,
                                                  CPSS_DXCH_EXACT_MATCH_ACTION_LAST_E,&actionData);
         UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, devNum, index, expandedActionIndex,CPSS_DXCH_EXACT_MATCH_ACTION_LAST_E);

         /*1.4. Call with wrong actionType that do not fit the action type TTI in the expanded,
            other params same as in 1.2.
            Expected: GT_BAD_PARAM.*/
         st = cpssDxChExactMatchPortGroupEntrySet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,
                                                  expandedActionIndex,&exactMatchEntry,
                                                  CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E,&actionData);
         UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, devNum, index, expandedActionIndex,CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E);

        /* 1.5. Call with wrong value expandedActionIndex - not valid, other params same as in 1.2.
            Expected: GT_BAD_PARAM.  */
         expandedActionIndex=4;
         st = cpssDxChExactMatchPortGroupEntrySet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,
                                                  expandedActionIndex,&exactMatchEntry,
                                                  actionType,&actionData);
         UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, devNum, index, expandedActionIndex,actionType);

         /* 1.6. Call with wrong value index - that do not fit the keyType, other params same as in 1.2.
            Expected: GT_BAD_PARAM.*/
         expandedActionIndex=2;
         index=1;/* index must be %2==0 for SIZE_19B_E*/
         exactMatchEntry.key.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E;
         st = cpssDxChExactMatchPortGroupEntrySet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,
                                                  expandedActionIndex,&exactMatchEntry,
                                                  actionType,&actionData);
         UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, devNum, index, expandedActionIndex,actionType);

         /*1.7. Call with wrong value lookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E other params same as in 1.2.
           Expected: GT_BAD_PARAM.*/
         expandedActionIndex=2;
         index=2;
         exactMatchEntry.key.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E;
         exactMatchEntry.lookupNum = CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E;
         st = cpssDxChExactMatchPortGroupEntrySet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,
                                                  expandedActionIndex,&exactMatchEntry,
                                                  actionType,&actionData);
         UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, devNum, index, expandedActionIndex,actionType);

         /*  1.8. Call with entryPtr [NULL], other params same as in 1.2.
            Expected: GT_BAD_PTR.   */
         st = cpssDxChExactMatchPortGroupEntrySet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,
                                                  expandedActionIndex,NULL,
                                                  actionType,&actionData);
         UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, "%d, exactMatchEntry = NULL", devNum);


    /* 1.9. Call with actionPtr [NULL], other params same as in 1.2.
        Expected: GT_BAD_PTR.*/
         expandedActionIndex=2;
         index=2;
         exactMatchEntry.key.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E;
         exactMatchEntry.lookupNum = CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
         st = cpssDxChExactMatchPortGroupEntrySet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,
                                                  expandedActionIndex,&exactMatchEntry,
                                                  actionType,NULL);
         UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, "%d, actionData = NULL", devNum);

         /*
           1.10. set PCL actionPtr [ pktCmd = CPSS_PACKET_CMD_FORWARD_E,
            mirror{cpuCode = 0, analyzerPortIndex = GT_FALSE},
            matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 },
            qos { egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE ,
                qos [ ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
                profilePrecedence=GT_FALSE] ] },
            redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E,
                data[routerLttIndex=0] },
            policer { policerEnable=GT_FALSE, policerId=0 },
            vlan { egressTaggedModify=GT_FALSE,
                modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE,
                vlanId=100, precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E },
            ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } ].

        Expected: GT_OK and same action as written
        */

        actionType = CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E;
        expandedActionIndex = 5;

        cpssOsMemSet(&expandedActionOriginData,0, sizeof(expandedActionOriginData));
        cpssOsMemSet(&expandedActionOriginDataGet,0, sizeof(expandedActionOriginDataGet));

        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionCutThroughTerminateId=GT_TRUE;
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPktCmd=GT_TRUE;
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSkipFdbSaLookup=GT_TRUE;
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionRedirectCommand=GT_TRUE;

        prvSetExactMatchActionDefaultValues(&actionData,CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E);

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
        /* validation values */
        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        /*
         1.11. Call cpssDxChExactMatchPortGroupEntrySet index=0,keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E,
            expandedActionIndex and other params same as in 1.1.
            Expected: GT_OK and same action as written */
        index=28;
        exactMatchEntry.key.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E;
        exactMatchEntry.lookupNum = CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
        exactMatchEntry.key.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;
        for (i=0;i<47;i++)/* 47 bytes in CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E*/
        {
         exactMatchEntry.key.pattern[i] = i;
        }
        st = cpssDxChExactMatchPortGroupEntrySet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,
                                                  expandedActionIndex,&exactMatchEntry,
                                                  actionType,&actionData);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, index, expandedActionIndex,actionType);

        st = cpssDxChExactMatchPortGroupEntryGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,&valid,
                                                 &actionTypeGet,&actionDataGet,&exactMatchEntryGet,&expandedActionIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChExactMatchPortGroupEntryGet: %d, %d, %d\n", devNum, index);
        /* validation values */
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, valid,"error - entry is not valid\n");
        UTF_VERIFY_EQUAL1_STRING_MAC(actionType, actionTypeGet,"get another actionTypeGet than was set: %d\n", devNum);

        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d\n",devNum, expandedActionIndex, actionType);

        st = prvCompareExactMatchEntry("exactMatchEntry", &exactMatchEntry, &exactMatchEntryGet);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"exactMatchEntry != exactMatchEntryGet\n");


        /*1.12. Call with wrong actionType that do not fit the action type PCL in the expanded,
            other params same as in 1.2.
            Expected: GT_BAD_PARAM.*/
        index=60;
        exactMatchEntry.key.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E;
        exactMatchEntry.lookupNum = CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
        st = cpssDxChExactMatchPortGroupEntrySet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,
                                                  expandedActionIndex,&exactMatchEntry,
                                                  CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E,&actionData);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, devNum, index, expandedActionIndex,CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E);


        /*  1.12. invalidate entries defined
            Expected: GT_TRUE.*/
        index = 0;
        st = cpssDxChExactMatchPortGroupEntryInvalidate(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, index);
        index = 28;
        st = cpssDxChExactMatchPortGroupEntryInvalidate(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, index);

         /*  1.12. invalidate entries that is not defined/already non valid
            Expected: GT_OK.*/
        index = 70;
        st = cpssDxChExactMatchPortGroupEntryInvalidate(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, index);
    }

    /* 2. For not active devNumNumices check that function returns non GT_OK.*/
    /* prepare devNumNumice iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;

    index=6;
    expandedActionIndex=5;
    exactMatchEntry.key.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;
    exactMatchEntry.lookupNum = CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
    actionType=CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E;

    /* Go over all non active devNumNumices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
         st = cpssDxChExactMatchPortGroupEntrySet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,
                                                  expandedActionIndex,&exactMatchEntry,
                                                  actionType,&actionData);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for devNumNumice id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChExactMatchPortGroupEntrySet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,
                                             expandedActionIndex,&exactMatchEntry,
                                             actionType,&actionData);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchPortGroupEntryGet
(
    IN GT_U8                                    devNum,
    IN GT_PORT_GROUPS_BMP                       portGroupsBmp,
    IN GT_U32                                   index,
    OUT GT_BOOL                                 *validPtr,
    OUT CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT   *actionTypePtr,
    OUT CPSS_DXCH_EXACT_MATCH_ACTION_UNT        *actionPtr,
    OUT CPSS_DXCH_EXACT_MATCH_ENTRY_STC         *entryPtr,
    OUT GT_U32                                  *expandedActionIndexPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchPortGroupEntryGet)
{
/*
    ITERATE_DEVICES (SIP6)
    1.1. Call with non null validPtr, actionTypePtr,actionPtr,entryPtr,expandedActionIndexPtr
    Expected: GT_OK - validPtr = GT_FALSE the entry was not defined yet.
    1.2. Call with validPtr [NULL].
    Expected: GT_BAD_PARAM.
    1.3. Call with actionTypePtr [NULL].
    Expected: GT_BAD_PARAM.
    1.4. Call with actionPtr [NULL].
    Expected: GT_BAD_PARAM.
    1.5. Call with entryPtr [NULL].
    Expected: GT_BAD_PTR.
    1.6. Call with expandedActionIndexPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                                       st = GT_OK;
    GT_U8                                           devNum = 0;
    GT_U32                                          index;
    GT_BOOL                                         valid;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT           actionTypeGet;
    GT_U32                                          expandedActionIndexGet;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT                actionDataGet;
    CPSS_DXCH_EXACT_MATCH_ENTRY_STC                 exactMatchEntryGet;


    cpssOsBzero((GT_VOID*) &actionDataGet, sizeof(actionDataGet));
    cpssOsBzero((GT_VOID*) &exactMatchEntryGet, sizeof(exactMatchEntryGet));
    cpssOsBzero((GT_VOID*) &exactMatchEntryGet.key.pattern, sizeof(exactMatchEntryGet.key.pattern));

    /* this feature is on Falcon devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
           1.1. Call with non null validPtr, actionTypePtr,actionPtr,entryPtr,expandedActionIndexPtr
           Expected: GT_OK - validPtr = GT_FALSE the entry was not defined yet.
        */
        index=4;
        st = cpssDxChExactMatchPortGroupEntryGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,&valid,
                                                  &actionTypeGet,&actionDataGet,&exactMatchEntryGet,&expandedActionIndexGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_FALSE, valid,"error - entry is valid, expected non valid\n");

        /*1.2. Call with validPtr [NULL].
        Expected: GT_BAD_PARAM.*/
        st = cpssDxChExactMatchPortGroupEntryGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,NULL,
                                                  &actionTypeGet,&actionDataGet,&exactMatchEntryGet,&expandedActionIndexGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cpssDxChExactMatchPortGroupEntryGet = NULL", devNum);

        /*1.3. Call with actionTypePtr [NULL].
        Expected: GT_BAD_PARAM.*/
        st = cpssDxChExactMatchPortGroupEntryGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,&valid,
                                                  NULL,&actionDataGet,&exactMatchEntryGet,&expandedActionIndexGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cpssDxChExactMatchPortGroupEntryGet = NULL", devNum);

        /*1.4. Call with actionPtr [NULL].
        Expected: GT_BAD_PARAM.*/
        st = cpssDxChExactMatchPortGroupEntryGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,&valid,
                                                  &actionTypeGet,NULL,&exactMatchEntryGet,&expandedActionIndexGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cpssDxChExactMatchPortGroupEntryGet = NULL", devNum);

        /*1.5. Call with entryPtr [NULL].
        Expected: GT_BAD_PTR.*/
        st = cpssDxChExactMatchPortGroupEntryGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,&valid,
                                                  &actionTypeGet,&actionDataGet,NULL,&expandedActionIndexGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cpssDxChExactMatchPortGroupEntryGet = NULL", devNum);

        /*1.6. Call with expandedActionIndexPtr [NULL].
        Expected: GT_BAD_PTR.*/
        st = cpssDxChExactMatchPortGroupEntryGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,&valid,
                                                  &actionTypeGet,&actionDataGet,&exactMatchEntryGet,NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cpssDxChExactMatchPortGroupEntryGet = NULL", devNum);

    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */

    /* this feature is on Falcon devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;
    index=2;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
         st = cpssDxChExactMatchPortGroupEntryGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,&valid,
                                                  &actionTypeGet,&actionDataGet,&exactMatchEntryGet,&expandedActionIndexGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

     st = cpssDxChExactMatchPortGroupEntryGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,&valid,
                                                  &actionTypeGet,&actionDataGet,&exactMatchEntryGet,&expandedActionIndexGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchHashCrcMultiResultsByParamsCalc
(
    IN  GT_U8                            devNum,
    IN  CPSS_DXCH_EXACT_MATCH_KEY_STC   *entryKeyPtr,
    OUT GT_U32                          *numberOfElemInCrcMultiHashArrPtr,
    OUT GT_U32                           exactMatchCrcMultiHashArr[]
)
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchHashCrcMultiResultsByParamsCalc)
{
/*
    ITERATE_DEVICES (Falcon)
    1.1. Call function with entryKey.keySize[CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E]
                            and entryKey.pattern[0]={0};
    Expected: GT_OK.
    1.1.2. Call function with entryKey.keySize[CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E]
                            and entryKey.pattern[0]={0};
    Expected: GT_OK.

    1.2 Call cpssDxChExactMatchHashCrcMultiResultsByParamsCalc with fixed pattern .
    Expected: GT_OK and exactMatchCrcMultiHashArr have same indexes as expectedBankIndex .
    NOTE : The number of identical indexes depends on numberOfElemInCrcMultiHash value.

    1.3. Call with NULL entryKey and other valid params.
    Expected: GT_BAD_PTR.
    1.4. Call with NULL numberOfElemInCrcMultiHashArrPtr and other valid params.
    Expected: GT_BAD_PTR.
    1.5. Call Call with NULL exactMatchCrcMultiHashArr and other valid params.
    Expected: GT_BAD_PTR.
    1.6 Call cpssDxChExactMatchHashCrcMultiResultsByParamsCalc with wrong keySize vlaue.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                           st = GT_OK;
    GT_U8                               devNum;
    CPSS_DXCH_EXACT_MATCH_KEY_STC       entryKey;
    GT_U32                              numberOfElemInCrcMultiHash;
    GT_U32                              exactMatchCrcMultiHashArr[CPSS_DXCH_EXACT_MATCH_MAX_NUM_BANKS_CNS]={0};
    GT_U32                              ii;

    GT_U8 testInput[]=      {0xb0,0x3d,0xe5,0x98,0xcc,0x49,0x47,0x7c,0xf8,0x48,0x63,0xce,0xf8,0xea,0x1c,0x8a,0x2b,0xb9,
                             0x6a,0x9e,0xa7,0x92,0xaf,0x2b,0xcb,0x74,0xa5,0x60,0x23,0x0,0x95,0x9,0xa8,0x24,0x27,0x91,0x8e,
                             0xa7,0x72,0xb8,0x7e,0x77,0x99,0xdb,0x96,0xf0,0x76,0x0} ;

    GT_U32 expectedBankIndex[] ={0x4d,0x1bd0,0x112e,0xf0e,0x21b,0x1cff,0x2bd,0x1f4b,0x1656,0x1004,0x1482,0x994,0xc5e,0x1de6,0xec1,0x18b9};

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_CNS) ;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
           1.1.1 Call function with entryKey.keySize[CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E]
                                    and entryKey.pattern[0]={0};
           Expected: GT_OK.
        */
        entryKey.keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;
        cpssOsMemSet(entryKey.pattern, 0, sizeof(entryKey.pattern));

        st = cpssDxChExactMatchHashCrcMultiResultsByParamsCalc(devNum, &entryKey, &numberOfElemInCrcMultiHash ,&exactMatchCrcMultiHashArr[0]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
           1.1.2 Call function with keyType [CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E]
                                    and entryKey.pattern[0]={0};
           Expected: GT_OK.
        */
        entryKey.keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E;
        st = cpssDxChExactMatchHashCrcMultiResultsByParamsCalc(devNum, &entryKey, &numberOfElemInCrcMultiHash ,&exactMatchCrcMultiHashArr[0]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);


        /*
            1.2 Call cpssDxChExactMatchHashCrcMultiResultsByParamsCalc with fixed pattern .
            Expected: GT_OK and exactMatchCrcMultiHashArr have same indexes as expectedBankIndex .
            NOTE :The number of identical indexes depends on numberOfElemInCrcMultiHash value.
        */
        entryKey.keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;
        for (ii=0;ii<CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS;ii++)
        {
            entryKey.pattern[ii]=testInput[ii];
        }
        st = cpssDxChExactMatchHashCrcMultiResultsByParamsCalc(devNum, &entryKey,&numberOfElemInCrcMultiHash ,&exactMatchCrcMultiHashArr[0]);

        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*expectedBankIndex values are without knowledge how meny bankes there is .
          we need to change the basic value to the format expected*/
        for (ii=0;ii<numberOfElemInCrcMultiHash;ii++)
        {
            expectedBankIndex[ii]=((expectedBankIndex[ii]*numberOfElemInCrcMultiHash) + ii); /*shift the basic value 2/3/4 bits (depends on how meny banks)
                                                                                               and adding the bank number */
        }
        for (ii=0;ii<numberOfElemInCrcMultiHash;ii++)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(exactMatchCrcMultiHashArr[ii], expectedBankIndex[ii],
                                         "got other index from exactMatchCrcMultiHashArr then expected: %d , index number %d ", devNum, ii);
        }

        /*
            1.3. Call with NULL entryKey and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChExactMatchHashCrcMultiResultsByParamsCalc(devNum,NULL,&numberOfElemInCrcMultiHash,&exactMatchCrcMultiHashArr[0]);

        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, " entryKey = NULL");

        /*
            1.4. Call with NULL numberOfElemInCrcMultiHashArr and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChExactMatchHashCrcMultiResultsByParamsCalc(devNum,&entryKey,NULL ,&exactMatchCrcMultiHashArr[0]);

        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, " numberOfElemInCrcMultiHashArr = NULL");

        /*
            1.5. Call Call with NULL exactMatchCrcMultiHashArr and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChExactMatchHashCrcMultiResultsByParamsCalc(devNum,&entryKey,&numberOfElemInCrcMultiHash ,NULL);

        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, " exactMatchCrcMultiHashArr = NULL");

        /*
            1.6 Call cpssDxChExactMatchHashCrcMultiResultsByParamsCalc with wrong keySize vlaue.
             Expected: GT_BAD_PARAM.
        */
        entryKey.keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_LAST_E;

        st = cpssDxChExactMatchHashCrcMultiResultsByParamsCalc(devNum,&entryKey,&numberOfElemInCrcMultiHash,&exactMatchCrcMultiHashArr[0]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);

    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */

    /* this feature is on Falcon devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        entryKey.keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;
        cpssOsMemSet(entryKey.pattern, 0, sizeof(entryKey.pattern));

        st = cpssDxChExactMatchHashCrcMultiResultsByParamsCalc(devNum, &entryKey, &numberOfElemInCrcMultiHash ,&exactMatchCrcMultiHashArr[0]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChExactMatchHashCrcMultiResultsByParamsCalc(devNum, &entryKey, &numberOfElemInCrcMultiHash ,&exactMatchCrcMultiHashArr[0]);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchRulesDump
(
    IN GT_U8                              devNum,
    IN GT_PORT_GROUPS_BMP                 portGroupsBmp,
    IN GT_U32                             startIndex,
    IN GT_U32                             numOfRules
)
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchRulesDump)
{
/*
    ITERATE_DEVICES (SIP6)
    1.1. Call cpssDxChExactMatchExpandedActionSet with non-NULL pointers,
         actionPtr{tunnelTerminate[GT_TRUE],
                   passengerPacketType[CPSS_DXCH_TTI_PASSENGER_IPV4_E],
                   copyTtlFromTunnelHeader[GT_FALSE],
                   command [CPSS_PACKET_CMD_FORWARD_E],
                   redirectCommand[CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E],
                   egressInterface{type[CPSS_INTERFACE_TRUNK_E],
                                   trunkId[2]},

                   tunnelStart [GT_TRUE],
                   tunnelStartPtr[0],
                   targetIsTrunk[GT_FALSE],

                   sourceIdSetEnable[GT_TRUE],
                   sourceId[0],

                   vlanCmd[CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E],
                   vlanId[100],
                   vlanPrecedence[CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E],
                   nestedVlanEnable[GT_FALSE],

                   bindToPolicer[GT_FALSE],

                   qosPrecedence[CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E],
                   qosTrustMode[CPSS_DXCH_TTI_QOS_UNTRUST_E],
                   qosProfile[0],
                   modifyUpEnable[CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_UP_E],
                   modifyDscpEnable[CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_DSCP_E],
                   up[0],

                   mirrorToIngressAnalyzerEnable[GT_FALSE],
                   vntl2Echo[GT_FALSE],
                   bridgeBypass[GT_FALSE],
                   actionStop[GT_FALSE]
              }
    Expected: GT_OK and same action as written
    1.2. Call cpssDxChExactMatchPortGroupEntrySet index=gotFromHashCalc,keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E,
        expandedActionIndex and other params same as in 1.1.
    Expected: GT_OK and same action as written
    1.3. Call cpssDxChExactMatchPortGroupEntrySet index=gotFromHashCalc,keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E,
        expandedActionIndex and other params same as in 1.1.
    Expected: GT_OK and same action as written
    1.4. Call cpssDxChExactMatchPortGroupEntrySet index=gotFromHashCalc,keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E,
        expandedActionIndex and other params same as in 1.1.
    Expected: GT_OK and same action as written
    1.5. Call cpssDxChExactMatchPortGroupEntrySet index=gotFromHashCalc,keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E,
        expandedActionIndex and other params same as in 1.1.
    Expected: GT_OK and same action as written

    1.6. set PCL actionPtr [ pktCmd = CPSS_PACKET_CMD_FORWARD_E,
        mirror{cpuCode = 0, analyzerPortIndex = GT_FALSE},
        matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 },
        qos { egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE ,
            qos [ ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
            profilePrecedence=GT_FALSE] ] },
        redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E,
            data[routerLttIndex=0] },
        policer { policerEnable=GT_FALSE, policerId=0 },
        vlan { egressTaggedModify=GT_FALSE,
            modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE,
            vlanId=100, precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E },
        ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } ].

    Expected: GT_OK and same action as written

    1.7. Call cpssDxChExactMatchPortGroupEntrySet index=gotFromHashCalc,keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E,
        expandedActionIndex and other params same as in 1.6.
    Expected: GT_OK and same action as written
    1.8. Call cpssDxChExactMatchPortGroupEntrySet index=gotFromHashCalc,keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E,
        expandedActionIndex and other params same as in 1.6.
    Expected: GT_OK and same action as written
    1.9. Call cpssDxChExactMatchPortGroupEntrySet index=gotFromHashCalc,keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E,
        expandedActionIndex and other params same as in 1.6.
    Expected: GT_OK and same action as written
    1.10. Call cpssDxChExactMatchPortGroupEntrySet index=gotFromHashCalc,keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E,
        expandedActionIndex and other params same as in 1.6.
    Expected: GT_OK and same action as written

    1.11. Call cpssDxChExactMatchRulesDump startIndex=0,numOfRules=32K,
    Expected: GT_OK

    1.12. Call cpssDxChExactMatchRulesDump startIndex=32K,numOfRules=5,
    Expected: GT_BAD_PARAM

    1.13. invalidate entries defined
        Expected: GT_TRUE.
    */

    GT_STATUS                                           st = GT_OK;
    GT_U8                                               i=0;
    GT_U8                                               devNum = 0;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT               actionType;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT               actionTypeGet;
    GT_U32                                              expandedActionIndex;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT                    actionData;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT                    actionDataGet;
    CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT    expandedActionOriginData;
    CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT    expandedActionOriginDataGet;


    GT_U32                                  index=0;
    CPSS_DXCH_EXACT_MATCH_ENTRY_STC         exactMatchEntry;
    CPSS_DXCH_EXACT_MATCH_ENTRY_STC         exactMatchEntryGet;
    GT_BOOL                                 valid;
    GT_BOOL                                 exactMatchActivityStatus;

    GT_U32                                  numberOfElemInCrcMultiHash;
    GT_U32                                  exactMatchCrcMultiHashArr[CPSS_DXCH_EXACT_MATCH_MAX_NUM_BANKS_CNS]={0};
    GT_U32                                  indexToInvalidate[8];

    CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC    keyParams;
    CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC    keyParamsGet;

    expandedActionIndex = 2;
    actionType = CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E;

    cpssOsBzero((GT_VOID*) &actionData, sizeof(actionData));
    cpssOsBzero((GT_VOID*) &actionDataGet, sizeof(actionDataGet));
    cpssOsBzero((GT_VOID*) &exactMatchEntry, sizeof(exactMatchEntry));
    cpssOsBzero((GT_VOID*) &exactMatchEntryGet, sizeof(exactMatchEntryGet));
    cpssOsBzero((GT_VOID*) &exactMatchEntry.key.pattern, sizeof(exactMatchEntry.key.pattern));

    cpssOsMemSet(&expandedActionOriginData,0, sizeof(expandedActionOriginData));
    cpssOsMemSet(&expandedActionOriginDataGet,0, sizeof(expandedActionOriginDataGet));

    expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionActionStop=GT_TRUE;
    expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionBridgeBypass=GT_TRUE;
    expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionCommand=GT_TRUE;
    expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionEgressInterface=GT_TRUE;
    expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionFlowId=GT_TRUE;
    expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionRedirectCommand=GT_TRUE;

    /* this feature is on Falcon devNumNumices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;

    /* 1. Go over all active devNumNumices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*  actionPtr{ tunnelTerminate[GT_FALSE], passengerPacketType[CPSS_DXCH_TTI_PASSENGER_IPV4_E],
            copyTtlFromTunnelHeader[GT_FALSE], command [CPSS_PACKET_CMD_FORWARD_E],
            redirectCommand[CPSS_DXCH_TTI_NO_REDIRECT_E], egressInterface{type[CPSS_INTERFACE_TRUNK_E],
            trunkId[0], vidx[0], vlanId[0], devNumNumPort {0, 0}}, tunnelStart [GT_FALSE], tunnelStartPtr[0],
            routerLookupPtr[0], vrfId[0], targetIsTrunk[GT_FALSE], virtualSrcPort[0], virtualSrcdevNumNum[0],
            sourceIdSetEnable[GT_FALSE], sourceId[0], vlanCmd[CPSS_DXCH_TTI_VLAN_DO_NOT_MODIFY_E],
            vlanId[0], vlanPrecedence[CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E],
            nestedVlanEnable[GT_FALSE], bindToPolicer[GT_FALSE], policerIndex[0],
            qosPrecedence[CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E],
            qosTrustMode[CPSS_DXCH_TTI_QOS_KEEP_PREVIOUS_E], qosProfile[0],
            modifyUpEnable[CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_UP_E], modifyDscpEnable[CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_DSCP_E],
            up[0], remapDSCP[GT_FALSE], mirrorToIngressAnalyzerEnable[GT_FALSE],
            userDefinedCpuCode[CPSS_NET_CONTROL_E],
            vntl2Echo[GT_FALSE], bridgeBypass[GT_FALSE], actionStop[GT_FALSE] }
        */

        prvSetExactMatchActionDefaultValues(&actionData,CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E);

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
        /* validation values */
        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

       /*
         1.2. Call cpssDxChExactMatchPortGroupEntrySet index=getFromHashCalc,keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E,
            expandedActionIndex and other params same as in 1.1.
            Expected: GT_OK and same action as written */

        keyParams.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;
        keyParams.keyStart=1;
        for (i=0;i<CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS;i++)
        {
           keyParams.mask[i]=i+1;
        }

        st = cpssDxChExactMatchProfileKeyParamsSet(devNum, expandedActionIndex, &keyParams);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, expandedActionIndex, keyParams.keySize, keyParams.keyStart);


        st = cpssDxChExactMatchProfileKeyParamsGet(devNum, expandedActionIndex, &keyParamsGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChExactMatchProfileKeyParamsGet: %d", devNum);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(keyParams.keySize, keyParamsGet.keySize,
                   "get another keyParams.keySize than was set: %d", devNum);

        UTF_VERIFY_EQUAL1_STRING_MAC(keyParams.keyStart, keyParamsGet.keyStart,
                   "get another keyParams.keyStart than was set: %d", devNum);

        for (i=0;i<CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS;i++)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(keyParams.mask[i], keyParamsGet.mask[i],
                   "get another keyParams.mask than was set: %d", devNum);
        }

        exactMatchEntry.key.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;
        for (i=0;i<5;i++)/* 5 bytes in CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E*/
        {
            exactMatchEntry.key.pattern[i] = i;
        }
        exactMatchEntry.lookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;

        st = cpssDxChExactMatchHashCrcMultiResultsByParamsCalc(devNum, &exactMatchEntry.key, &numberOfElemInCrcMultiHash ,&exactMatchCrcMultiHashArr[0]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        index=exactMatchCrcMultiHashArr[0];
        indexToInvalidate[0]=index;

        st = cpssDxChExactMatchPortGroupEntrySet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,
                                          expandedActionIndex,&exactMatchEntry,actionType,&actionData);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, index, expandedActionIndex,actionType);

        st = cpssDxChExactMatchPortGroupEntryGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,&valid,
                                              &actionTypeGet,&actionDataGet,&exactMatchEntryGet,&expandedActionIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChExactMatchPortGroupEntryGet: %d, %d, %d\n", devNum, index);
        /* validation values */
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, valid,"error - entry is not valid\n");
        UTF_VERIFY_EQUAL1_STRING_MAC(actionType, actionTypeGet,"get another actionTypeGet than was set: %d\n", devNum);

        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d\n",devNum, expandedActionIndex, actionType);

        st = prvCompareExactMatchEntry("exactMatchEntry", &exactMatchEntry, &exactMatchEntryGet);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"exactMatchEntry != exactMatchEntryGet\n");

        /* check activity status */
        st = cpssDxChExactMatchActivityStatusGet(devNum,
                                              CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                              index,
                                              GT_FALSE,
                                              &exactMatchActivityStatus);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChExactMatchActivityStatusGet: %d", devNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_FALSE, exactMatchActivityStatus,"error - entry is active\n");


        /* 1.3. Call cpssDxChExactMatchPortGroupEntrySet index=getFromHashCalc,keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E,
        expandedActionIndex and other params same as in 1.1.
        Expected: GT_OK and same action as written */

        exactMatchEntry.key.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E;
        for (i=0;i<19;i++)/* 19 bytes in CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E*/
        {
         exactMatchEntry.key.pattern[i] = i;
        }
        exactMatchEntry.lookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;

        st = cpssDxChExactMatchHashCrcMultiResultsByParamsCalc(devNum, &exactMatchEntry.key, &numberOfElemInCrcMultiHash ,&exactMatchCrcMultiHashArr[0]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        index=exactMatchCrcMultiHashArr[0];
        indexToInvalidate[1]=index;

        st = cpssDxChExactMatchPortGroupEntrySet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,
                                          expandedActionIndex,&exactMatchEntry,actionType,&actionData);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, index, expandedActionIndex,actionType);

        st = cpssDxChExactMatchPortGroupEntryGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,&valid,
                                              &actionTypeGet,&actionDataGet,&exactMatchEntryGet,&expandedActionIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChExactMatchPortGroupEntryGet: %d, %d, %d\n", devNum, index);
        /* validation values */
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, valid,"error - entry is not valid\n");
        UTF_VERIFY_EQUAL1_STRING_MAC(actionType, actionTypeGet,"get another actionTypeGet than was set: %d\n", devNum);

        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d\n",devNum, expandedActionIndex, actionType);

        st = prvCompareExactMatchEntry("exactMatchEntry", &exactMatchEntry, &exactMatchEntryGet);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"exactMatchEntry != exactMatchEntryGet\n");

        /* check activity status */
        st = cpssDxChExactMatchActivityStatusGet(devNum,
                                              CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                              index,
                                              GT_FALSE,
                                              &exactMatchActivityStatus);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChExactMatchActivityStatusGet: %d", devNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_FALSE, exactMatchActivityStatus,"error - entry is active\n");

       /* 1.4. Call cpssDxChExactMatchPortGroupEntrySet index=getFromHashCalc,keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E,
            expandedActionIndex and other params same as in 1.1.
            Expected: GT_OK and same action as written */

         exactMatchEntry.key.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E;
         for (i=0;i<33;i++)/* 33 bytes in CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E*/
         {
             exactMatchEntry.key.pattern[i] = i;
         }
         exactMatchEntry.lookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;

         st = cpssDxChExactMatchHashCrcMultiResultsByParamsCalc(devNum, &exactMatchEntry.key, &numberOfElemInCrcMultiHash ,&exactMatchCrcMultiHashArr[0]);
         UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

         index=exactMatchCrcMultiHashArr[0];
         indexToInvalidate[2]=index;

         st = cpssDxChExactMatchPortGroupEntrySet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,
                                              expandedActionIndex,&exactMatchEntry,actionType,&actionData);
         UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, index, expandedActionIndex,actionType);

         st = cpssDxChExactMatchPortGroupEntryGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,&valid,
                                                  &actionTypeGet,&actionDataGet,&exactMatchEntryGet,&expandedActionIndex);
         UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChExactMatchPortGroupEntryGet: %d, %d, %d\n", devNum, index);
         /* validation values */
         UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, valid,"error - entry is not valid\n");
         UTF_VERIFY_EQUAL1_STRING_MAC(actionType, actionTypeGet,"get another actionTypeGet than was set: %d\n", devNum);

         st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
         UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d\n",devNum, expandedActionIndex, actionType);

         st = prvCompareExactMatchEntry("exactMatchEntry", &exactMatchEntry, &exactMatchEntryGet);
         UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"exactMatchEntry != exactMatchEntryGet\n");

         /* check activity status */
         st = cpssDxChExactMatchActivityStatusGet(devNum,
                                                  CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                  index,
                                                  GT_FALSE,
                                                  &exactMatchActivityStatus);
         UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChExactMatchActivityStatusGet: %d", devNum);
         UTF_VERIFY_EQUAL0_STRING_MAC(GT_FALSE, exactMatchActivityStatus,"error - entry is active\n");

         /* 1.5. Call cpssDxChExactMatchPortGroupEntrySet index=getFromHashCalc,keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E,
            expandedActionIndex and other params same as in 1.1.
            Expected: GT_OK and same action as written */

         exactMatchEntry.key.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E;
         for (i=0;i<47;i++)/* 47 bytes in CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E*/
         {
             exactMatchEntry.key.pattern[i] = i;
         }
         exactMatchEntry.lookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;

         st = cpssDxChExactMatchHashCrcMultiResultsByParamsCalc(devNum, &exactMatchEntry.key, &numberOfElemInCrcMultiHash ,&exactMatchCrcMultiHashArr[0]);
         UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

         index=exactMatchCrcMultiHashArr[0];
         indexToInvalidate[3]=index;

         st = cpssDxChExactMatchPortGroupEntrySet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,
                                              expandedActionIndex,&exactMatchEntry,actionType,&actionData);
         UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, index, expandedActionIndex,actionType);

         st = cpssDxChExactMatchPortGroupEntryGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,&valid,
                                                  &actionTypeGet,&actionDataGet,&exactMatchEntryGet,&expandedActionIndex);
         UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChExactMatchPortGroupEntryGet: %d, %d, %d\n", devNum, index);
         /* validation values */
         UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, valid,"error - entry is not valid\n");
         UTF_VERIFY_EQUAL1_STRING_MAC(actionType, actionTypeGet,"get another actionTypeGet than was set: %d\n", devNum);

         st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
         UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d\n",devNum, expandedActionIndex, actionType);

         st = prvCompareExactMatchEntry("exactMatchEntry", &exactMatchEntry, &exactMatchEntryGet);
         UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"exactMatchEntry != exactMatchEntryGet\n");

         /* check activity status */
         st = cpssDxChExactMatchActivityStatusGet(devNum,
                                                  CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                  index,
                                                  GT_FALSE,
                                                  &exactMatchActivityStatus);
         UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChExactMatchActivityStatusGet: %d", devNum);
         UTF_VERIFY_EQUAL0_STRING_MAC(GT_FALSE, exactMatchActivityStatus,"error - entry is active\n");

         /*
           1.6. set PCL actionPtr [ pktCmd = CPSS_PACKET_CMD_FORWARD_E,
            mirror{cpuCode = 0, analyzerPortIndex = GT_FALSE},
            matchCounter { enableMatchCount = GT_FALSE, matchCounterIndex = 0 },
            qos { egressPolicy=GT_FALSE, modifyDscp=GT_FALSE, modifyUp=GT_FALSE ,
                qos [ ingress[profileIndex=0, profileAssignIndex=GT_FALSE,
                profilePrecedence=GT_FALSE] ] },
            redirect { CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E,
                data[routerLttIndex=0] },
            policer { policerEnable=GT_FALSE, policerId=0 },
            vlan { egressTaggedModify=GT_FALSE,
                modifyVlan=CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E, nestedVlan=GT_FALSE,
                vlanId=100, precedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E },
            ipUcRoute { doIpUcRoute=GT_FALSE, 0, GT_FALSE, GT_FALSE, GT_FALSE } ].

        Expected: GT_OK and same action as written
        */

        actionType = CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E;
        expandedActionIndex = 5;

        cpssOsMemSet(&expandedActionOriginData,0, sizeof(expandedActionOriginData));
        cpssOsMemSet(&expandedActionOriginDataGet,0, sizeof(expandedActionOriginDataGet));

        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionCutThroughTerminateId=GT_TRUE;
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPktCmd=GT_TRUE;
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSkipFdbSaLookup=GT_TRUE;
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionRedirectCommand=GT_TRUE;

        prvSetExactMatchActionDefaultValues(&actionData,CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E);

        st = cpssDxChExactMatchExpandedActionSet(devNum,expandedActionIndex,actionType,&actionData,&expandedActionOriginData);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionSet: %d, %d, %d",
                                         devNum, expandedActionIndex, actionType);

        st = cpssDxChExactMatchExpandedActionGet(devNum,expandedActionIndex,actionType,&actionDataGet,&expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChExactMatchExpandedActionGet: %d, %d, %d", devNum, expandedActionIndex,actionType);
        /* validation values */
        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        st = prvCompareExactMatchExpandedStructs("expandedActionOriginData", actionType, &expandedActionOriginData, &expandedActionOriginDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d, %d, %d",devNum, expandedActionIndex, actionType);

        /*
         1.7. Call cpssDxChExactMatchPortGroupEntrySet index=gotFromHashCalc,keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E,
            expandedActionIndex and other params same as in 1.1.
            Expected: GT_OK and same action as written */

        keyParams.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E;
        keyParams.keyStart=2;
        for (i=0;i<CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS;i++)
        {
           keyParams.mask[i]=i+1;
        }

        st = cpssDxChExactMatchProfileKeyParamsSet(devNum, expandedActionIndex, &keyParams);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, expandedActionIndex, keyParams.keySize, keyParams.keyStart);


        st = cpssDxChExactMatchProfileKeyParamsGet(devNum, expandedActionIndex, &keyParamsGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChExactMatchProfileKeyParamsGet: %d", devNum);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(keyParams.keySize, keyParamsGet.keySize,
                   "get another keyParams.keySize than was set: %d", devNum);

        UTF_VERIFY_EQUAL1_STRING_MAC(keyParams.keyStart, keyParamsGet.keyStart,
                   "get another keyParams.keyStart than was set: %d", devNum);

        for (i=0;i<CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS;i++)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(keyParams.mask[i], keyParamsGet.mask[i],
                   "get another keyParams.mask than was set: %d", devNum);
        }

        exactMatchEntry.key.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;
        exactMatchEntry.lookupNum = CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
        for (i=0;i<5;i++)/* 5 bytes in CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E*/
        {
         exactMatchEntry.key.pattern[i] = i*2;
        }

        st = cpssDxChExactMatchHashCrcMultiResultsByParamsCalc(devNum, &exactMatchEntry.key, &numberOfElemInCrcMultiHash ,&exactMatchCrcMultiHashArr[0]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        index=exactMatchCrcMultiHashArr[0];
        indexToInvalidate[4]=index;

        st = cpssDxChExactMatchPortGroupEntrySet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,
                                                  expandedActionIndex,&exactMatchEntry,
                                                  actionType,&actionData);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, index, expandedActionIndex,actionType);

        st = cpssDxChExactMatchPortGroupEntryGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,&valid,
                                                 &actionTypeGet,&actionDataGet,&exactMatchEntryGet,&expandedActionIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChExactMatchPortGroupEntryGet: %d, %d, %d\n", devNum, index);
        /* validation values */
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, valid,"error - entry is not valid\n");
        UTF_VERIFY_EQUAL1_STRING_MAC(actionType, actionTypeGet,"get another actionTypeGet than was set: %d\n", devNum);

        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d\n",devNum, expandedActionIndex, actionType);

        st = prvCompareExactMatchEntry("exactMatchEntry", &exactMatchEntry, &exactMatchEntryGet);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"exactMatchEntry != exactMatchEntryGet\n");

        /*
         1.8. Call cpssDxChExactMatchPortGroupEntrySet index=gotFromHashCalc,keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E,
            expandedActionIndex and other params same as in 1.1.
            Expected: GT_OK and same action as written */
        exactMatchEntry.key.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E;
        exactMatchEntry.lookupNum = CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
        for (i=0;i<19;i++)/* 19 bytes in CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E*/
        {
         exactMatchEntry.key.pattern[i] = i*2;
        }

        st = cpssDxChExactMatchHashCrcMultiResultsByParamsCalc(devNum, &exactMatchEntry.key, &numberOfElemInCrcMultiHash ,&exactMatchCrcMultiHashArr[0]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        index=exactMatchCrcMultiHashArr[0];
        indexToInvalidate[5]=index;

        st = cpssDxChExactMatchPortGroupEntrySet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,
                                                  expandedActionIndex,&exactMatchEntry,
                                                  actionType,&actionData);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, index, expandedActionIndex,actionType);

        st = cpssDxChExactMatchPortGroupEntryGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,&valid,
                                                 &actionTypeGet,&actionDataGet,&exactMatchEntryGet,&expandedActionIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChExactMatchPortGroupEntryGet: %d, %d, %d\n", devNum, index);
        /* validation values */
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, valid,"error - entry is not valid\n");
        UTF_VERIFY_EQUAL1_STRING_MAC(actionType, actionTypeGet,"get another actionTypeGet than was set: %d\n", devNum);

        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d\n",devNum, expandedActionIndex, actionType);

        st = prvCompareExactMatchEntry("exactMatchEntry", &exactMatchEntry, &exactMatchEntryGet);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"exactMatchEntry != exactMatchEntryGet\n");

        /*
         1.9. Call cpssDxChExactMatchPortGroupEntrySet index=gotFromHashCalc,keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E,
            expandedActionIndex and other params same as in 1.1.
            Expected: GT_OK and same action as written */
        exactMatchEntry.key.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E;
        exactMatchEntry.lookupNum = CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
        for (i=0;i<33;i++)/* 33 bytes in CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E*/
        {
         exactMatchEntry.key.pattern[i] = i*2;
        }

        st = cpssDxChExactMatchHashCrcMultiResultsByParamsCalc(devNum, &exactMatchEntry.key, &numberOfElemInCrcMultiHash ,&exactMatchCrcMultiHashArr[0]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        index=exactMatchCrcMultiHashArr[0];
        indexToInvalidate[6]=index;

        st = cpssDxChExactMatchPortGroupEntrySet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,
                                                  expandedActionIndex,&exactMatchEntry,
                                                  actionType,&actionData);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, index, expandedActionIndex,actionType);

        st = cpssDxChExactMatchPortGroupEntryGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,&valid,
                                                 &actionTypeGet,&actionDataGet,&exactMatchEntryGet,&expandedActionIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChExactMatchPortGroupEntryGet: %d, %d, %d\n", devNum, index);
        /* validation values */
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, valid,"error - entry is not valid\n");
        UTF_VERIFY_EQUAL1_STRING_MAC(actionType, actionTypeGet,"get another actionTypeGet than was set: %d\n", devNum);

        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d\n",devNum, expandedActionIndex, actionType);

        st = prvCompareExactMatchEntry("exactMatchEntry", &exactMatchEntry, &exactMatchEntryGet);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"exactMatchEntry != exactMatchEntryGet\n");

        /*
         1.10. Call cpssDxChExactMatchPortGroupEntrySet index=gotFromHashCalc,keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E,
            expandedActionIndex and other params same as in 1.1.
            Expected: GT_OK and same action as written */
        exactMatchEntry.key.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E;
        exactMatchEntry.lookupNum = CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
        for (i=0;i<47;i++)/* 47 bytes in CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E*/
        {
         exactMatchEntry.key.pattern[i] = i*2;
        }

        st = cpssDxChExactMatchHashCrcMultiResultsByParamsCalc(devNum, &exactMatchEntry.key, &numberOfElemInCrcMultiHash ,&exactMatchCrcMultiHashArr[0]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        index=exactMatchCrcMultiHashArr[0];
        indexToInvalidate[7]=index;

        st = cpssDxChExactMatchPortGroupEntrySet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,
                                                  expandedActionIndex,&exactMatchEntry,
                                                  actionType,&actionData);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, index, expandedActionIndex,actionType);

        st = cpssDxChExactMatchPortGroupEntryGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,index,&valid,
                                                 &actionTypeGet,&actionDataGet,&exactMatchEntryGet,&expandedActionIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChExactMatchPortGroupEntryGet: %d, %d, %d\n", devNum, index);
        /* validation values */
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, valid,"error - entry is not valid\n");
        UTF_VERIFY_EQUAL1_STRING_MAC(actionType, actionTypeGet,"get another actionTypeGet than was set: %d\n", devNum);

        st = prvCompareExactMatchActionStructs(devNum, "action", actionType, &actionData, &actionDataGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d\n",devNum, expandedActionIndex, actionType);

        st = prvCompareExactMatchEntry("exactMatchEntry", &exactMatchEntry, &exactMatchEntryGet);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"exactMatchEntry != exactMatchEntryGet\n");


        /*1.11. Call cpssDxChExactMatchRulesDump startIndex=0,numOfRules=32K,
        Expected: GT_OK*/
        st = cpssDxChExactMatchRulesDump(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,0,(_32K-1));
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChExactMatchRulesDump: %d\n", devNum);

        /*1.12. Call cpssDxChExactMatchRulesDump startIndex=32K,numOfRules=5,
        Expected: GT_OUT_OF_RANGE */
        st = cpssDxChExactMatchRulesDump(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,_32K,5);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OUT_OF_RANGE, st, "cpssDxChExactMatchRulesDump: %d\n", devNum);

        /*  1.13. invalidate 8 entries that were defined
            Expected: GT_OK.*/
        for (i=0;i<8;i++)
        {
            st = cpssDxChExactMatchPortGroupEntryInvalidate(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, indexToInvalidate[i]);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, indexToInvalidate[i]);
        }
    }

    /* 2. For not active devNumNumices check that function returns non GT_OK.*/
    /* prepare devNumNumice iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum,UTF_CPSS_PP_ALL_SIP6_CNS) ;

    /* Go over all non active devNumNumices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChExactMatchRulesDump(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,0,5);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for devNum id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChExactMatchRulesDump(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,0,5);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChExactMatch suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChExactMatch)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchTtiProfileIdSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchTtiProfileIdGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchPclProfileIdSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchPclProfileIdGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchClientLookupSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchClientLookupGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchActivityBitEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchActivityBitEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchActivityStatusGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchProfileKeyParamsSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchProfileKeyParamsGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchProfileDefaultActionSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchProfileDefaultActionGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchExpandedActionSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchExpandedActionGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchPortGroupEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchPortGroupEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchHashCrcMultiResultsByParamsCalc)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchRulesDump)
UTF_SUIT_END_TESTS_MAC(cpssDxChExactMatch)


