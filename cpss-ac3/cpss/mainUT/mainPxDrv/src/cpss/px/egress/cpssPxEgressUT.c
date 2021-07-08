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
* @file cpssPxEgressUT.c
*
* @brief Unit tests for cpssPxEgress.h, that provides
* CPSS PX implementation for egress processing.
*
* @version   1
********************************************************************************
*/
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

#include <cpss/px/cpssPxTypes.h>
#include <cpss/px/egress/cpssPxEgress.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>

/* defines */

#define PACKET_TYPE_MAX_NUM_CNS 32
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxEgressBypassModeSet
(
    IN GT_SW_DEV_NUM     devNum,
    IN GT_BOOL           phaBypassEnable,
    IN GT_BOOL           ppaClockEnable
);
*/
UTF_TEST_CASE_MAC(cpssPxEgressBypassModeSet)
{
    GT_STATUS                   st = GT_OK;
    GT_U8                       dev;
    GT_BOOL                     phaBypassEnable,ppaClockEnable;
    GT_BOOL                     phaBypassEnableGet,ppaClockEnableGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function with phaBypassEnable/ppaClockEnable [GT_FALSE and GT_TRUE].           */
        /* Expected: GT_OK for Px devices and GT_BAD_PARAM for others.    */
        for(phaBypassEnable = GT_FALSE; phaBypassEnable <= GT_TRUE ; phaBypassEnable++)
        {
            for(ppaClockEnable = GT_FALSE; ppaClockEnable <= GT_TRUE ; ppaClockEnable++)
            {
                st = cpssPxEgressBypassModeSet(dev, phaBypassEnable,ppaClockEnable);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, phaBypassEnable,ppaClockEnable);

                /* 1.2. */
                st = cpssPxEgressBypassModeGet(dev, &phaBypassEnableGet,&ppaClockEnableGet);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                UTF_VERIFY_EQUAL1_STRING_MAC(phaBypassEnable, phaBypassEnableGet,
                    "get another phaBypassEnable than was set on device: %d", dev);
                UTF_VERIFY_EQUAL1_STRING_MAC(ppaClockEnable, ppaClockEnableGet,
                    "get another phaBypassEnable than was set on device: %d", dev);
            }
        }
    }

    phaBypassEnable = GT_TRUE;
    ppaClockEnable  = GT_TRUE;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /*2. Go over all non active/non applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. <Call function for non active/non applicable device and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssPxEgressBypassModeSet(dev, phaBypassEnable,ppaClockEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                          */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxEgressBypassModeSet(dev, phaBypassEnable,ppaClockEnable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxEgressBypassModeGet
(
    IN  GT_SW_DEV_NUM     devNum,
    OUT GT_BOOL          *phaBypassEnablePtr,
    OUT GT_BOOL          *ppaClockEnablePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxEgressBypassModeGet)
{
    /*
    ITERATE_DEVICES
    1.1. Call function with not NULL enablePtr
    Expected: GT_OK
    1.2. Call function with NULL enablePtr
    Expected: GT_BAD_PTR
    */
    GT_STATUS                   st = GT_OK;
    GT_U8                       dev;
    GT_BOOL                     phaBypassEnableGet,ppaClockEnableGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. */
        st = cpssPxEgressBypassModeGet(dev, &phaBypassEnableGet,&ppaClockEnableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2. */
        st = cpssPxEgressBypassModeGet(dev, NULL,&ppaClockEnableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /* 1.3. */
        st = cpssPxEgressBypassModeGet(dev, &phaBypassEnableGet,NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /* 1.4. */
        st = cpssPxEgressBypassModeGet(dev, NULL,NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*2. Go over all non active/non applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. <Call function for non active/non applicable device and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssPxEgressBypassModeGet(dev, &phaBypassEnableGet,&ppaClockEnableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                          */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxEgressBypassModeGet(dev, &phaBypassEnableGet,&ppaClockEnableGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxEgressTimestampModeSet
(
    IN GT_SW_DEV_NUM     devNum,
    IN GT_U32            packetType,
    IN GT_BOOL           useTod
)
*/
UTF_TEST_CASE_MAC(cpssPxEgressTimestampModeSet)
{
    GT_STATUS                   st = GT_OK;
    GT_U8                       dev;
    GT_U32                      packetType;
    GT_BOOL                     useTod,useTodGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function with packetType = 0..31 , useTod [GT_FALSE and GT_TRUE].           */
        /* Expected: GT_OK     */
        for(packetType = 0; packetType < PACKET_TYPE_MAX_NUM_CNS ; packetType++)
        {
            useTod =  (packetType % 3) == 1 ? GT_TRUE : GT_FALSE;
            st = cpssPxEgressTimestampModeSet(dev, packetType,useTod);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, packetType,useTod);
        }

        /* 1.2. call get function to check values */
        for(packetType = 0; packetType < PACKET_TYPE_MAX_NUM_CNS ; packetType++)
        {
            useTod =  (packetType % 3) == 1 ? GT_TRUE : GT_FALSE;
            st = cpssPxEgressTimestampModeGet(dev, packetType,&useTodGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(useTod, useTodGet,
                "get another useTod than was set on device: %d", dev);
        }
    }

    packetType = 0;
    useTod = GT_TRUE;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /*2. Go over all non active/non applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. <Call function for non active/non applicable device and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssPxEgressTimestampModeSet(dev, packetType,useTod);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                          */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxEgressTimestampModeSet(dev, packetType,useTod);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxEgressTimestampModeGet
(
    IN GT_SW_DEV_NUM     devNum,
    IN GT_U32            packetType,
    OUT GT_BOOL          *useTodPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxEgressTimestampModeGet)
{
    /*
    ITERATE_DEVICES
    1.1. Call function with not NULL enablePtr
    Expected: GT_OK
    1.2. Call function with NULL enablePtr
    Expected: GT_BAD_PTR
    */
    GT_STATUS                   st = GT_OK;
    GT_U8                       dev;
    GT_U32                      packetType;
    GT_BOOL                     useTod;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(packetType = 0; packetType < PACKET_TYPE_MAX_NUM_CNS ; packetType++)
        {
            /* 1.1. */
            st = cpssPxEgressTimestampModeGet(dev, packetType, &useTod);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* 1.2. */
            st = cpssPxEgressTimestampModeGet(dev, packetType, NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
        }
    }
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    packetType = 0;

    /*2. Go over all non active/non applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. <Call function for non active/non applicable device and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssPxEgressTimestampModeGet(dev, packetType,&useTod);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                          */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxEgressTimestampModeGet(dev, packetType,&useTod);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/* fill 'valid' operation information according to the operation type .
   filled values depend on the 'globalIndex' */
static void fillOperationInfo
(
    IN  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT   operationType,
    OUT CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT  *operationInfoPtr,
    IN  GT_U32                                      globalIndex /* should be 0..(17*32)-1 = 0..543*/
)
{
    CPSS_802_1BR_ETAG_STC               *eTagPtr       = NULL;
    CPSS_PX_REGULAR_DSA_FORWARD_STC     *dsaForwardPtr = NULL;
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_STC   *info_dsa_QCNPtr = NULL;
    CPSS_PX_EXTENDED_DSA_FORWARD_STC   *dsaExtForwardPtr = NULL;
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_MC_STC *pcidPtr;

    /* fill the union with default pattern */
    cpssOsMemSet(operationInfoPtr,0xFF,sizeof(CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT));

    if(operationType == CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_E ||
       operationType == CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CPU_PORT_E)
    {
        eTagPtr = &operationInfoPtr->info_802_1br_E2U.eTag;
    }
    else
    if (operationType == CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M4_PORT_E)
    {
        pcidPtr = &operationInfoPtr->info_802_1br_U2E_MC;
        pcidPtr->pcid[0] = globalIndex % CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M4_PORT_E;
        pcidPtr->pcid[1] = globalIndex % CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M4_PORT_E +1;
        pcidPtr->pcid[2] = globalIndex % CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M4_PORT_E +2;
    }
    else
    if (operationType == CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M8_PORT_E)
    {
        pcidPtr = &operationInfoPtr->info_802_1br_U2E_MC;
        pcidPtr->pcid[0] = globalIndex % CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M8_PORT_E + 1;
        pcidPtr->pcid[1] = globalIndex % CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M8_PORT_E + 2;
        pcidPtr->pcid[2] = globalIndex % CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M8_PORT_E + 3;
        pcidPtr->pcid[3] = globalIndex % CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M8_PORT_E + 4;
        pcidPtr->pcid[4] = globalIndex % CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M8_PORT_E + 5;
        pcidPtr->pcid[5] = globalIndex % CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M8_PORT_E + 6;
        pcidPtr->pcid[6] = globalIndex % CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M8_PORT_E + 7;

    }
    else
    if(operationType == CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_E)
    {
        dsaForwardPtr = &operationInfoPtr->info_dsa_ET2U.dsaForward;
    }
    else
    if(operationType == CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_E)
    {
        dsaForwardPtr = &operationInfoPtr->info_dsa_EU2U.dsaForward;
    }
    else
    if(operationType == CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_E)
    {
        info_dsa_QCNPtr = &operationInfoPtr->info_dsa_QCN;

        info_dsa_QCNPtr->macSa.arEther[0] =  (GT_U8)(0xFF - globalIndex);
        info_dsa_QCNPtr->macSa.arEther[1] =  (GT_U8)(0xFF - globalIndex + 0x55);
        info_dsa_QCNPtr->macSa.arEther[2] =  (GT_U8)(0xFF - globalIndex + 0xaa);
        info_dsa_QCNPtr->macSa.arEther[3] =  (GT_U8)(0xFF + globalIndex + 0xa5);
        info_dsa_QCNPtr->macSa.arEther[4] =  (GT_U8)(0xFF + globalIndex + 0x5a);
        info_dsa_QCNPtr->macSa.arEther[5] =  (GT_U8)(0xFF + globalIndex);

        dsaExtForwardPtr = &info_dsa_QCNPtr->dsaExtForward;

        dsaExtForwardPtr->srcTagged       = BIT2BOOL_MAC((globalIndex & 1));
        dsaExtForwardPtr->hwSrcDev        = globalIndex & 0x1f;      /* 5 bits*/
        dsaExtForwardPtr->srcPortOrTrunk  = 0x7f - dsaExtForwardPtr->hwSrcDev; /* 7 bits*/
        dsaExtForwardPtr->srcIsTrunk      = BIT2BOOL_MAC(((globalIndex>>4) & 1));
        dsaExtForwardPtr->cfi             = ((globalIndex>>3) & 1);  /* 1 bit */
        dsaExtForwardPtr->up              = ((globalIndex>>1) & 3);  /* 3 bits*/
        dsaExtForwardPtr->vid             = 0xFFF - globalIndex;     /*12 bits*/

        dsaExtForwardPtr->egrFilterRegistered = BIT2BOOL_MAC(((globalIndex >> 1) & 1));
        dsaExtForwardPtr->dropOnSource        = BIT2BOOL_MAC(((globalIndex >> 2) & 1));
        dsaExtForwardPtr->packetIsLooped      = BIT2BOOL_MAC(((globalIndex >> 3) & 1));
        dsaExtForwardPtr->wasRouted           = BIT2BOOL_MAC(((globalIndex >> 4) & 1));
        dsaExtForwardPtr->srcId           = (0x2F - globalIndex) & 0x1F; /* 5 bits */
        dsaExtForwardPtr->qosProfileIndex = (0x77 - globalIndex) & 0x7F; /* 7 bits */

        dsaExtForwardPtr->useVidx         = BIT2BOOL_MAC(((globalIndex >> 5) & 1));
        dsaExtForwardPtr->trgVidx         = (0xabc - globalIndex) & 0xFFF; /* 12 bits */
        dsaExtForwardPtr->trgPort         = (0x3c - globalIndex) & 0x3F;   /* 6 bits */
        dsaExtForwardPtr->hwTrgDev        = (0x13 - globalIndex) & 0x1F;   /* 5 bits */

        info_dsa_QCNPtr->cnmTpid =  (GT_U16)(0xF00F - globalIndex);      /*16 bits*/
    }
    else
    if(operationType == CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_UNTAGGED_E)
    {
        eTagPtr = &operationInfoPtr->info_802_1br_E2U.eTag;
        operationInfoPtr->info_802_1br_E2U.vlanTagTpid = (GT_U16)(0xFFFF - globalIndex);      /*16 bits*/
    }
    else
    {
        operationInfoPtr->notNeeded = 0;/*don't care*/
    }

    if(eTagPtr)
    {
        eTagPtr->TPID                 =  0xFFFF - globalIndex;      /*16 bits*/
        eTagPtr->E_PCP                =  globalIndex & 7;           /* 3 bits*/
        eTagPtr->E_DEI                =  globalIndex & 1;           /* 1 bit */
        eTagPtr->Ingress_E_CID_base   =  0xFFF - globalIndex;       /*12 bits*/
        eTagPtr->Direction            =  globalIndex & 1;           /* 1 bit*/
        eTagPtr->Upstream_Specific    =  1 - (globalIndex & 1);     /* 1 bit*/
        eTagPtr->GRP                  =  3 - (globalIndex & 3);     /* 2 bits*/
        eTagPtr->E_CID_base           =  globalIndex + 3000;        /*12 bits*/
        eTagPtr->Ingress_E_CID_ext    =  globalIndex & 0xff;        /* 8 bits*/
        eTagPtr->E_CID_ext            =  0xff - eTagPtr->Ingress_E_CID_ext; /* 8 bits*/
    }

    if(dsaForwardPtr)
    {
        dsaForwardPtr->srcTagged       = BIT2BOOL_MAC((globalIndex & 1));
        dsaForwardPtr->hwSrcDev        = globalIndex & 0x1f;      /* 5 bits*/
        dsaForwardPtr->srcPortOrTrunk  = 0x1f - dsaForwardPtr->hwSrcDev; /* 5 bits*/
        dsaForwardPtr->srcIsTrunk      = BIT2BOOL_MAC(((globalIndex>>4) & 1));
        dsaForwardPtr->cfi             = ((globalIndex>>3) & 1);  /* 1 bit */
        dsaForwardPtr->up              = ((globalIndex>>1) & 3);  /* 3 bits*/
        dsaForwardPtr->vid             = 0xFFF - globalIndex;     /*12 bits*/
    }
}

#define COMPARE_FIELD(fieldName)                                        \
    UTF_VERIFY_EQUAL1_STRING_MAC(expPtr->fieldName, actPtr->fieldName,  \
        "get another [%s] than was set",                                \
        #fieldName)

/* compare expected etag operation information to actual operation information*/
static void compareOperationInfo_eTag(
    IN  CPSS_802_1BR_ETAG_STC  *expPtr,
    IN  CPSS_802_1BR_ETAG_STC  *actPtr
)
{
    COMPARE_FIELD(TPID                 );
    COMPARE_FIELD(E_PCP                );
    COMPARE_FIELD(E_DEI                );
    COMPARE_FIELD(Ingress_E_CID_base   );
    COMPARE_FIELD(Direction            );
    COMPARE_FIELD(Upstream_Specific    );
    COMPARE_FIELD(GRP                  );
    COMPARE_FIELD(E_CID_base           );
    COMPARE_FIELD(Ingress_E_CID_ext    );
    COMPARE_FIELD(E_CID_ext            );
}
/* compare expected etag operation information to actual operation information*/
static void compareOperationInfo_dsaForward(
    IN  CPSS_PX_REGULAR_DSA_FORWARD_STC  *expPtr,
    IN  CPSS_PX_REGULAR_DSA_FORWARD_STC  *actPtr
)
{
    COMPARE_FIELD(srcTagged            );
    COMPARE_FIELD(hwSrcDev             );
    COMPARE_FIELD(srcPortOrTrunk       );
    COMPARE_FIELD(srcIsTrunk           );
    COMPARE_FIELD(cfi                  );
    COMPARE_FIELD(up                   );
    COMPARE_FIELD(vid                  );
}

/* compare expected etag operation information to actual operation information*/
static void compareOperationInfo_QCN(
    IN  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_STC  *expPtr,
    IN  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_STC  *actPtr
)
{
    COMPARE_FIELD(macSa.arEther[0]                      );
    COMPARE_FIELD(macSa.arEther[1]                      );
    COMPARE_FIELD(macSa.arEther[2]                      );
    COMPARE_FIELD(macSa.arEther[3]                      );
    COMPARE_FIELD(macSa.arEther[4]                      );
    COMPARE_FIELD(macSa.arEther[5]                      );

    COMPARE_FIELD(dsaExtForward.srcTagged               );
    COMPARE_FIELD(dsaExtForward.hwSrcDev                );
    COMPARE_FIELD(dsaExtForward.srcPortOrTrunk          );
    COMPARE_FIELD(dsaExtForward.srcIsTrunk              );
    COMPARE_FIELD(dsaExtForward.cfi                     );
    COMPARE_FIELD(dsaExtForward.up                      );
    COMPARE_FIELD(dsaExtForward.vid                     );

    COMPARE_FIELD(dsaExtForward.egrFilterRegistered     );
    COMPARE_FIELD(dsaExtForward.dropOnSource            );
    COMPARE_FIELD(dsaExtForward.packetIsLooped          );
    COMPARE_FIELD(dsaExtForward.wasRouted               );
    COMPARE_FIELD(dsaExtForward.srcId                   );
    COMPARE_FIELD(dsaExtForward.qosProfileIndex         );

    COMPARE_FIELD(dsaExtForward.useVidx                 );
    if(expPtr->dsaExtForward.useVidx == GT_TRUE)
    {
        COMPARE_FIELD(dsaExtForward.trgVidx             );
    }
    else
    {
        COMPARE_FIELD(dsaExtForward.trgPort             );
        COMPARE_FIELD(dsaExtForward.hwTrgDev            );
    }

    COMPARE_FIELD(cnmTpid                               );

}

/* compare expected operation information to actual operation information
   according to the operation type. */
static void compareOperationInfo(
    IN  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT   operationType,
    IN  CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT  *expectedOperationInfoPtr,
    IN  CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT  *actualOperationInfoPtr
)
{
    if(operationType == CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_E)
    {
        compareOperationInfo_eTag(
            &expectedOperationInfoPtr->info_802_1br_E2U.eTag,
            &actualOperationInfoPtr->info_802_1br_E2U.eTag);
    }
    else
    if(operationType == CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_E)
    {
        compareOperationInfo_dsaForward(
            &expectedOperationInfoPtr->info_dsa_ET2U.dsaForward,
            &actualOperationInfoPtr->info_dsa_ET2U.dsaForward);
    }
    else
    if(operationType == CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_E)
    {
        compareOperationInfo_dsaForward(
            &expectedOperationInfoPtr->info_dsa_EU2U.dsaForward,
            &actualOperationInfoPtr->info_dsa_EU2U.dsaForward);
    }
    else
    if(operationType == CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_E)
    {
        compareOperationInfo_QCN(
            &expectedOperationInfoPtr->info_dsa_QCN,
            &actualOperationInfoPtr->info_dsa_QCN);
    }
    else
    if(operationType == CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_UNTAGGED_E)
    {
        compareOperationInfo_eTag(
            &expectedOperationInfoPtr->info_802_1br_E2U.eTag,
            &actualOperationInfoPtr->info_802_1br_E2U.eTag);

        UTF_VERIFY_EQUAL1_STRING_MAC(expectedOperationInfoPtr->info_802_1br_E2U.vlanTagTpid, 
                                     actualOperationInfoPtr->info_802_1br_E2U.vlanTagTpid,
                                     "get another vlanTagTpid[%d] than was set", 
                                     expectedOperationInfoPtr->info_802_1br_E2U.vlanTagTpid);
    }
    else
    {
        /* nothing to compare !!! the 'notNeeded' field is not relevant !!! */
    }
}

/* set info with 'invalid' operation information according to the operation type */
/* return indication if entry can get 'bad param' with the returned info :
    GT_TRUE  - entry can     get 'bad param' with the returned info
    GT_FALSE - entry can NOT get 'bad param' with the returned info
*/
static GT_BOOL fillOperationInfo_invalid
(
    IN  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT   operationType,
    OUT CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT  *operationInfoPtr,
    IN  GT_U32                                      globalIndex /* should be 0..(17*32)-1 = 0..543*/
)
{
    CPSS_802_1BR_ETAG_STC               *eTagPtr       = NULL;
    CPSS_PX_REGULAR_DSA_FORWARD_STC     *dsaForwardPtr = NULL;
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_STC   *info_dsa_QCNPtr = NULL;
    CPSS_PX_EXTENDED_DSA_FORWARD_STC   *dsaExtForwardPtr = NULL;

    /* fill 'valid' operation information according to the operation type */
    fillOperationInfo(operationType,operationInfoPtr,globalIndex);

    if(operationType == CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_E)
    {
        eTagPtr = &operationInfoPtr->info_802_1br_E2U.eTag;
    }
    else
    if(operationType == CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_E)
    {
        dsaForwardPtr = &operationInfoPtr->info_dsa_ET2U.dsaForward;
    }
    else
    if(operationType == CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_E)
    {
        dsaForwardPtr = &operationInfoPtr->info_dsa_EU2U.dsaForward;
    }
    else
    if(operationType == CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_E)
    {
        info_dsa_QCNPtr = &operationInfoPtr->info_dsa_QCN;

        dsaExtForwardPtr = &info_dsa_QCNPtr->dsaExtForward;
        dsaExtForwardPtr->useVidx = GT_FALSE;

        switch(globalIndex % 10) /* 10 fields that are not BOOL and can get bad param */
        {
            default:/*just for compiler warnings*/
            case 0:
                dsaExtForwardPtr->hwSrcDev        = BIT_5;
                break;
            case 1:
                dsaExtForwardPtr->srcPortOrTrunk  = BIT_7;
                break;
            case 2:
                dsaExtForwardPtr->cfi             = BIT_1;
                break;
            case 3:
                dsaExtForwardPtr->up              = BIT_3;
                break;
            case 4:
                dsaExtForwardPtr->vid             = BIT_12;
                break;
            case 5:
                dsaExtForwardPtr->srcId           = BIT_5;
                break;
            case 6:
                dsaExtForwardPtr->qosProfileIndex = BIT_7;
                break;
            case 7:
                dsaExtForwardPtr->trgVidx         = BIT_12;
                dsaExtForwardPtr->useVidx         = GT_TRUE;
                break;
            case 8:
                dsaExtForwardPtr->trgPort         = BIT_6;
                break;
            case 9:
                dsaExtForwardPtr->hwTrgDev        = BIT_5;
                break;
        }
    }
    else
    if(operationType == CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_UNTAGGED_E)
    {
        eTagPtr = &operationInfoPtr->info_802_1br_E2U.eTag;
        operationInfoPtr->info_802_1br_E2U.vlanTagTpid = BIT_16;
    }
    else
    {
        /*GT_FALSE - entry can NOT get 'bad param' with the returned info*/
        return GT_FALSE;
    }

    if(eTagPtr)
    {
        switch(globalIndex % 10) /* 10 fields */
        {
            default:/*just for compiler warnings*/
            case 0:
                eTagPtr->TPID                 =  BIT_16;
                break;
            case 1:
                eTagPtr->E_PCP                =  BIT_3;
                break;
            case 2:
                eTagPtr->E_DEI                =  BIT_1;
                break;
            case 3:
                eTagPtr->Ingress_E_CID_base   =  BIT_12;
                break;
            case 4:
                eTagPtr->Direction            =  BIT_1;
                break;
            case 5:
                eTagPtr->Upstream_Specific    =  BIT_1;
            break;
            case 6:
                eTagPtr->GRP                  =  BIT_2;
                break;
            case 7:
                eTagPtr->E_CID_base           =  BIT_12;
                break;
            case 8:
                eTagPtr->Ingress_E_CID_ext    =  BIT_8;
                break;
            case 9:
                eTagPtr->E_CID_ext            =  BIT_8;
                break;
        }
    }

    if(dsaForwardPtr)
    {
        switch(globalIndex % 5) /* 5 fields that are not BOOL and can get bad param */
        {
            default:/*just for compiler warnings*/
            case 0:
                dsaForwardPtr->hwSrcDev        = BIT_5;
                break;
            case 1:
                dsaForwardPtr->srcPortOrTrunk  = BIT_5;
                break;
            case 2:
                dsaForwardPtr->cfi             = BIT_1;
                break;
            case 3:
                dsaForwardPtr->up              = BIT_3;
                break;
            case 4:
                dsaForwardPtr->vid             = BIT_12;
                break;
        }
    }

    /*GT_TRUE  - entry can     get 'bad param' with the returned info*/
    return GT_TRUE;
}



/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxEgressHeaderAlterationEntrySet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_PX_PACKET_TYPE  packetType,
    IN CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT   operationType,
    IN CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT  *operationInfoPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxEgressHeaderAlterationEntrySet)
{
    GT_STATUS                   st = GT_OK;
    GT_U8                       dev;

    GT_U32                      globalIndex;

    GT_PHYSICAL_PORT_NUM        port;
    CPSS_PX_PACKET_TYPE         packetType;
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT   operationType,operationTypeGet;
    CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT  operationInfo,operationInfoGet;
    GT_BOOL canFail; /* indication that entry in specific operation mode can get bad parameters */

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function with packetType = 0..31 , useTod [GT_FALSE and GT_TRUE].           */
        /* Expected: GT_OK     */
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        globalIndex = 0;
        /* 1.1. call set function to check valid values */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            for(packetType = 0; packetType < PACKET_TYPE_MAX_NUM_CNS ; packetType++ , globalIndex++)
            {
                /* select operation type to set tot the entry */
                operationType = globalIndex % CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE___MUST_BE_LAST___E;

                /* fill 'valid' operation information according to the operation type */
                fillOperationInfo(operationType,&operationInfo,globalIndex);

                /* 1.1. call set function to check valid values */
                st = cpssPxEgressHeaderAlterationEntrySet(dev, port ,packetType, operationType , &operationInfo);
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port ,packetType , operationType);
            }
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        globalIndex = 0;
        /* 1.2. call get function to valid check values */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            for(packetType = 0; packetType < PACKET_TYPE_MAX_NUM_CNS ; packetType++ , globalIndex++)
            {
                /* select operation type to set tot the entry */
                operationType = globalIndex % CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE___MUST_BE_LAST___E;

                /* fill 'valid' operation information according to the operation type */
                fillOperationInfo(operationType,&operationInfo,globalIndex);

                /* 1.2. call get function to valid check values */
                st = cpssPxEgressHeaderAlterationEntryGet(dev, port ,packetType, &operationTypeGet , &operationInfoGet);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                UTF_VERIFY_EQUAL3_STRING_MAC(operationType, operationTypeGet,
                    "get another operationType than was set on device: %d , port: %d ,packetType: %d ",
                    dev,port,packetType);

                if(operationType == operationTypeGet)
                {
                    compareOperationInfo(operationType,&operationInfo,&operationInfoGet);
                }
            }
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        globalIndex = 0;
        /* 1.3. call set function to check invalid values */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            for(packetType = 0; packetType < PACKET_TYPE_MAX_NUM_CNS ; packetType++ , globalIndex++)
            {
                /* select operation type to set tot the entry */
                operationType = globalIndex % CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE___MUST_BE_LAST___E;

                /* set info with 'invalid' operation information according to the operation type */
                canFail = fillOperationInfo_invalid(operationType,&operationInfo,globalIndex);

                /* 1.3. call set function to check invalid values */
                st = cpssPxEgressHeaderAlterationEntrySet(dev, port ,packetType, operationType , &operationInfo);
                if(canFail == GT_TRUE)
                {
                    /* we can cause to the entry to bad param */
                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, port ,packetType , operationType);
                }
                else
                {
                    /* we can NOT cause to the entry to bad param */
                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port ,packetType , operationType);
                }
            }
        }


        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        globalIndex = 0;
        /* 1.4. call get function to valid check values ---
                after we called entries with BAD_PARAM --> check ALL is 'as was before' */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            for(packetType = 0; packetType < PACKET_TYPE_MAX_NUM_CNS ; packetType++ , globalIndex++)
            {
                /* select operation type to set tot the entry */
                operationType = globalIndex % CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE___MUST_BE_LAST___E;

                /* fill 'valid' operation information according to the operation type */
                fillOperationInfo(operationType,&operationInfo,globalIndex);
                /* 1.4. call get function to valid check values ---
                        after we called entries with BAD_PARAM --> check ALL is 'as was before' */
                st = cpssPxEgressHeaderAlterationEntryGet(dev, port ,packetType, &operationTypeGet , &operationInfoGet);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                UTF_VERIFY_EQUAL3_STRING_MAC(operationType, operationTypeGet,
                    "get another operationType than was set on device: %d , port: %d ,packetType: %d ",
                    dev,port,packetType);

                if(operationType == operationTypeGet)
                {
                    compareOperationInfo(operationType,&operationInfo,&operationInfoGet);
                }
            }
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        globalIndex = 0;
        /* 1.4. call get function to valid check values ---
                after we called entries with BAD_PARAM --> check ALL is 'as was before' */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /* break -- to get valid port */
            break;
        }

        operationType = 0;
        packetType    = 0;

        /* fill 'valid' operation information according to the operation type */
        fillOperationInfo(operationType,&operationInfo,globalIndex);
        /* make sure function still PASS -- after all previous changes in parameters */
        /* before we start the 'expecting errors' section !!! */
        st = cpssPxEgressHeaderAlterationEntrySet(dev, port ,packetType, operationType , &operationInfo);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port ,packetType , operationType);


        /* 1.5. call set function with invalid ENUM : <operationType> */
        UTF_ENUMS_CHECK_MAC(cpssPxEgressHeaderAlterationEntrySet(dev, port ,
            packetType, operationType , &operationInfo),
                            operationType);

        operationType = 0;
        /* 1.6. call set function with NULL pointer */
        st = cpssPxEgressHeaderAlterationEntrySet(dev, port ,packetType, operationType , NULL);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PTR, st, dev, port ,packetType , operationType);
       /* 1.7. call set function with bad operationType */
        for(operationType = CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE___MUST_BE_LAST___E;
            operationType < (321 + CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE___MUST_BE_LAST___E);
            operationType += 17)
        {
            st = cpssPxEgressHeaderAlterationEntrySet(dev, port ,packetType, operationType , &operationInfo);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, port ,packetType , operationType);
        }

        /* 1.8. call set function with bad packetType */
        for(packetType = 32;
            packetType < (321 + 32);
            packetType += 17)
        {
            st = cpssPxEgressHeaderAlterationEntrySet(dev, port ,packetType, operationType , &operationInfo);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, port ,packetType , operationType);
        }

        packetType = 0;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        globalIndex = 0;
        /* 1.9. call set function with not valid physical ports */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssPxEgressHeaderAlterationEntrySet(dev, port ,packetType, operationType , &operationInfo);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, port ,packetType , operationType);
        }

    }
    globalIndex   = 0;
    operationType = 0;
    packetType    = 0;
    port = 0;

    /* fill 'valid' operation information according to the operation type */
    fillOperationInfo(operationType,&operationInfo,globalIndex);

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /*2. Go over all non active/non applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. <Call function for non active/non applicable device and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssPxEgressHeaderAlterationEntrySet(dev, port ,packetType, operationType , &operationInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                          */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxEgressHeaderAlterationEntrySet(dev, port ,packetType, operationType , &operationInfo);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxEgressHeaderAlterationEntryGet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_PX_PACKET_TYPE      packetType,
    OUT CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT   *operationTypePtr,
    OUT CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT  *operationInfoPtr
);
*/
UTF_TEST_CASE_MAC(cpssPxEgressHeaderAlterationEntryGet)
{
    GT_STATUS                   st = GT_OK;
    GT_U8                       dev;

    GT_PHYSICAL_PORT_NUM        port;
    CPSS_PX_PACKET_TYPE         packetType;
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT   operationTypeGet;
    CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT  operationInfoGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function with packetType = 0..31 , useTod [GT_FALSE and GT_TRUE].           */
        /* Expected: GT_OK     */
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. call set function to check valid values */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            for(packetType = 0; packetType < PACKET_TYPE_MAX_NUM_CNS ; packetType++)
            {
                /* 1.1. call get function to check valid values */
                st = cpssPxEgressHeaderAlterationEntryGet(dev, port ,packetType, &operationTypeGet , &operationInfoGet);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port ,packetType);

                /* 1.2. call function with NULL pointer */
                st = cpssPxEgressHeaderAlterationEntryGet(dev, port ,packetType, &operationTypeGet , NULL);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, port ,packetType);

                /* 1.3. call function with NULL pointer */
                st = cpssPxEgressHeaderAlterationEntryGet(dev, port ,packetType, NULL , &operationInfoGet);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, port ,packetType);
            }

            /* 1.4. call function with bad packetType */
            for(packetType = 32;
                packetType < (321 + 32);
                packetType += 17)
            {
                st = cpssPxEgressHeaderAlterationEntryGet(dev, port ,packetType, &operationTypeGet , &operationInfoGet);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port ,packetType);
            }
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        packetType = 0;
        /* 1.5. call set function with not valid physical ports */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssPxEgressHeaderAlterationEntryGet(dev, port ,packetType, &operationTypeGet , &operationInfoGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port ,packetType);
        }
    }

    packetType    = 0;
    port = 0;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /*2. Go over all non active/non applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. <Call function for non active/non applicable device and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssPxEgressHeaderAlterationEntryGet(dev, port ,packetType, &operationTypeGet , &operationInfoGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                          */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxEgressHeaderAlterationEntryGet(dev, port ,packetType, &operationTypeGet , &operationInfoGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/* fill 'valid' information according to the type */
static void fillSrcPortInfo(
    IN CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT   infoType,
    IN CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT        *portInfoPtr,
    IN GT_PHYSICAL_PORT_NUM                         portNum /* 0..16 */
)
{
    CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC  *info_802_1brPtr;
    CPSS_PX_EGRESS_SOURCE_PORT_DSA_STC      *info_dsaPtr;
    CPSS_PX_EGRESS_SOURCE_PORT_EVB_STC      *info_evbPtr;

    /* VLAN tag */
    CPSS_PX_VLAN_TAG_STC *vlanTagPtr;

    cpssOsMemSet(portInfoPtr,0xff,sizeof(CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT));

    switch (infoType)
    {
        case CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_802_1BR_E:
            info_802_1brPtr = &portInfoPtr->info_802_1br;
            info_802_1brPtr->pcid       = 0xF01 - portNum;/*12 bits*/
            info_802_1brPtr->srcPortInfo.srcFilteringVector  = (1 << portNum) & 0xffff; /*16 bits*/
            info_802_1brPtr->upstreamPort = portNum % 2;
            break;
        case CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_DSA_E:
            info_dsaPtr = &portInfoPtr->info_dsa;
            info_dsaPtr->srcPortNum = (0x8  + portNum) & 0xF;  /* 4 bits*/
            break;
        case CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_802_1BR_UNTAGGED_E:
            info_802_1brPtr = &portInfoPtr->info_802_1br;
            vlanTagPtr = &portInfoPtr->info_802_1br.srcPortInfo.vlanTag;
            info_802_1brPtr->pcid       = 0xF01 - portNum; /*12 bits*/
            info_802_1brPtr->upstreamPort = portNum % 2;
            vlanTagPtr->vid  = 0xF01 - portNum; /*12 bits*/
            vlanTagPtr->dei = 1;
            vlanTagPtr->pcp = portNum & 0x7;  /* 3 bits*/
            break;
        case CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_EVB_E:
            info_evbPtr = &portInfoPtr->info_evb;
            info_evbPtr->vid = 0xF01 - portNum; /*12 bits*/
            break;
        default:
            /* CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_NOT_USED_E) */
            portInfoPtr->notNeeded = 0;
    }
}

void static compareSrcPortInfo(
    IN CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT   infoType,
    IN CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT        *expectedPortInfoPtr,
    IN CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT        *actualPortInfoPtr
)
{
    switch (infoType)
    {
        case CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_802_1BR_E: 
            {
                CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC  *expPtr;
                CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC  *actPtr;

                expPtr = &expectedPortInfoPtr->info_802_1br;
                actPtr = &actualPortInfoPtr->info_802_1br;

                COMPARE_FIELD(pcid                 );
                COMPARE_FIELD(upstreamPort         );
            }
            {
                CPSS_PX_EGRESS_SRC_PORT_INFO_STC  *expPtr;
                CPSS_PX_EGRESS_SRC_PORT_INFO_STC  *actPtr;
                expPtr = &expectedPortInfoPtr->info_802_1br.srcPortInfo;
                actPtr = &actualPortInfoPtr->info_802_1br.srcPortInfo;

                COMPARE_FIELD(srcFilteringVector   );
            }
            break;
        case CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_DSA_E:
            {
                CPSS_PX_EGRESS_SOURCE_PORT_DSA_STC  *expPtr;
                CPSS_PX_EGRESS_SOURCE_PORT_DSA_STC  *actPtr;

                expPtr = &expectedPortInfoPtr->info_dsa;
                actPtr = &actualPortInfoPtr->info_dsa;

                COMPARE_FIELD(srcPortNum           );
            }
            break;
        case CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_802_1BR_UNTAGGED_E:
            {
                CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC  *expPtr;
                CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC  *actPtr;

                expPtr = &expectedPortInfoPtr->info_802_1br;
                actPtr = &actualPortInfoPtr->info_802_1br;

                COMPARE_FIELD(pcid                 );
                COMPARE_FIELD(upstreamPort         );
            }
            {
                CPSS_PX_VLAN_TAG_STC  *expPtr;
                CPSS_PX_VLAN_TAG_STC  *actPtr;
                expPtr = &expectedPortInfoPtr->info_802_1br.srcPortInfo.vlanTag;
                actPtr = &actualPortInfoPtr->info_802_1br.srcPortInfo.vlanTag;

                COMPARE_FIELD(vid                  );
                COMPARE_FIELD(dei                  );
                COMPARE_FIELD(pcp                  );
            }
            break;
        default:
            break;
    }
}


/* set info with 'invalid' information according to the type */
/* return indication if entry can get 'bad param' with the returned info :
    GT_TRUE  - entry can     get 'bad param' with the returned info
    GT_FALSE - entry can NOT get 'bad param' with the returned info
*/
static GT_BOOL fillSrcPortInfo_invalid(
    IN CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT   infoType,
    IN CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT        *portInfoPtr,
    IN GT_PHYSICAL_PORT_NUM                         portNum /* 0..16 */
)
{
    CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC  *info_802_1brPtr;
    CPSS_PX_EGRESS_SOURCE_PORT_DSA_STC      *info_dsaPtr;
    CPSS_PX_EGRESS_SOURCE_PORT_EVB_STC      *info_evbPtr;

        /* VLAN tag */
    CPSS_PX_VLAN_TAG_STC *vlanTagPtr;

    /* fill 'valid' information according to the type */
    fillSrcPortInfo(infoType,portInfoPtr,portNum);

    switch (infoType)
    {
        case CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_802_1BR_E:
            info_802_1brPtr = &portInfoPtr->info_802_1br;

            if((portNum >> 2) & 1)
            {
                info_802_1brPtr->pcid   = BIT_12;
            }
            else
            {
                info_802_1brPtr->srcPortInfo.srcFilteringVector  = BIT_16;
            }

            /*GT_TRUE  - entry can     get 'bad param' with the returned info*/
            return GT_TRUE;
        case CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_DSA_E:
            info_dsaPtr = &portInfoPtr->info_dsa;

            info_dsaPtr->srcPortNum = BIT_4;
            /*GT_TRUE  - entry can     get 'bad param' with the returned info*/
            return GT_TRUE;
        case CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_802_1BR_UNTAGGED_E:
            info_802_1brPtr = &portInfoPtr->info_802_1br;
            vlanTagPtr = &portInfoPtr->info_802_1br.srcPortInfo.vlanTag;
            if((portNum >> 2) & 1)
            {
                info_802_1brPtr->pcid   = BIT_12;
            }
            else
            {
                vlanTagPtr->vid  = BIT_12;
                vlanTagPtr->dei = BIT_1;
                vlanTagPtr->pcp = BIT_3;
            }

            /*GT_TRUE  - entry can     get 'bad param' with the returned info*/
            return GT_TRUE;
        case CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_EVB_E:
            info_evbPtr = &portInfoPtr->info_evb;
            info_evbPtr->vid = BIT_12;

            /*GT_TRUE  - entry can     get 'bad param' with the returned info*/
            return GT_TRUE;
        default:
            break;
    }

    /*GT_FALSE - entry can NOT get 'bad param' with the returned info*/
    return GT_FALSE;
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxEgressSourcePortEntrySet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT infoType,
    IN CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT      *portInfoPtr
);
*/
UTF_TEST_CASE_MAC(cpssPxEgressSourcePortEntrySet)
{
    GT_STATUS                   st = GT_OK;
    GT_U8                       dev;

    GT_PHYSICAL_PORT_NUM        port;
    CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT   infoType,infoTypeGet;
    CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT        portInfo,portInfoGet;
    GT_BOOL canFail; /* indication that entry in specific operation mode can get bad parameters */

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function with packetType = 0..31 , useTod [GT_FALSE and GT_TRUE].           */
        /* Expected: GT_OK     */
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. call set function to check valid values */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            infoType = port % CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE___MUST_BE_LAST___E;

            /* fill 'valid' information according to the type */
            fillSrcPortInfo(infoType,&portInfo,port);

            /* 1.1. call set function to check valid values */
            st = cpssPxEgressSourcePortEntrySet(dev, port ,infoType, &portInfo);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port ,infoType);
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. call get function to valid check values */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            infoType = port % CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE___MUST_BE_LAST___E;

            /* fill 'valid' information according to the type */
            fillSrcPortInfo(infoType,&portInfo,port);

            /* 1.2. call get function to valid check values */
            st = cpssPxEgressSourcePortEntryGet(dev, port ,&infoTypeGet, &portInfoGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            UTF_VERIFY_EQUAL2_STRING_MAC(infoType, infoTypeGet,
                "get another infoType than was set on device: %d , port: %d ",
                dev,port);

            if(infoType == infoTypeGet)
            {
                compareSrcPortInfo(infoType,&portInfo,&portInfoGet);
            }
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.3. call set function to check invalid values */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            infoType = port % CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE___MUST_BE_LAST___E;

            /* set info with 'invalid' information according to the type */
            canFail = fillSrcPortInfo_invalid(infoType,&portInfo,port);

            /* 1.3. call set function to check invalid values */
            st = cpssPxEgressSourcePortEntrySet(dev, port ,infoType, &portInfo);
            if(canFail == GT_TRUE)
            {
                /* we can cause to the entry to bad param */
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, port ,infoType);
            }
            else
            {
                /* we can NOT cause to the entry to bad param */
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port ,infoType);
            }
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.4. call get function to valid check values ---
                after we called entries with BAD_PARAM --> check ALL is 'as was before' */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            infoType = port % CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE___MUST_BE_LAST___E;

            /* fill 'valid' information according to the type */
            fillSrcPortInfo(infoType,&portInfo,port);

                /* 1.4. call get function to valid check values ---
                        after we called entries with BAD_PARAM --> check ALL is 'as was before' */
            st = cpssPxEgressSourcePortEntryGet(dev, port ,&infoTypeGet, &portInfoGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            UTF_VERIFY_EQUAL2_STRING_MAC(infoType, infoTypeGet,
                "get another infoType than was set on device: %d , port: %d ",
                dev,port);

            if(infoType == infoTypeGet)
            {
                compareSrcPortInfo(infoType,&portInfo,&portInfoGet);
            }
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.4. call get function to valid check values ---
                after we called entries with BAD_PARAM --> check ALL is 'as was before' */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /* break -- to get valid port */
            break;
        }

        infoType    = 0;
        port = 0;
        /* fill 'valid' operation information according to the operation type */
        fillSrcPortInfo(infoType,&portInfo,port);

        /* make sure function still PASS -- after all previous changes in parameters */
        /* before we start the 'expecting errors' section !!! */
        st = cpssPxEgressSourcePortEntrySet(dev, port ,infoType, &portInfo);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);


        /* 1.5. call set function with invalid ENUM : <infoType> */
        UTF_ENUMS_CHECK_MAC(cpssPxEgressSourcePortEntrySet(dev, port ,infoType, &portInfo),
                            infoType);

        infoType = 0;

        /* 1.6. call set function with NULL pointer */
        st = cpssPxEgressSourcePortEntrySet(dev, port ,infoType, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);

        /* 1.7. call set function with bad infoType */
        for(infoType = CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE___MUST_BE_LAST___E;
            infoType < (321 + CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE___MUST_BE_LAST___E);
            infoType += 17)
        {
            st = cpssPxEgressSourcePortEntrySet(dev, port ,infoType, &portInfo);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        infoType = 0;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.9. call set function with not valid physical ports */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssPxEgressSourcePortEntrySet(dev, port ,infoType, &portInfo);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
    }

    infoType    = 0;
    port = 0;

    /* fill 'valid' operation information according to the operation type */
    fillSrcPortInfo(infoType,&portInfo,port);

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /*2. Go over all non active/non applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. <Call function for non active/non applicable device and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssPxEgressSourcePortEntrySet(dev, port ,infoType, &portInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                          */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxEgressSourcePortEntrySet(dev, port ,infoType, &portInfo);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxEgressSourcePortEntryGet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT *infoTypePtr,
    OUT CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT      *portInfoPtr
)*/
UTF_TEST_CASE_MAC(cpssPxEgressSourcePortEntryGet)
{
    GT_STATUS                   st = GT_OK;
    GT_U8                       dev;

    GT_PHYSICAL_PORT_NUM        port;
    CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT   infoTypeGet;
    CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT        portInfoGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function with packetType = 0..31 , useTod [GT_FALSE and GT_TRUE].           */
        /* Expected: GT_OK     */
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. call set function to check valid values */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /* 1.1. call get function to check valid values */
            st = cpssPxEgressSourcePortEntryGet(dev, port , &infoTypeGet , &portInfoGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.2. call function with NULL pointer */
            st = cpssPxEgressSourcePortEntryGet(dev, port ,&infoTypeGet , NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);

            /* 1.3. call function with NULL pointer */
            st = cpssPxEgressSourcePortEntryGet(dev, port ,NULL , &portInfoGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.5. call set function with not valid physical ports */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssPxEgressSourcePortEntryGet(dev, port , &infoTypeGet , &portInfoGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
    }

    port = 0;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /*2. Go over all non active/non applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. <Call function for non active/non applicable device and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssPxEgressSourcePortEntryGet(dev, port , &infoTypeGet , &portInfoGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                          */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxEgressSourcePortEntryGet(dev, port , &infoTypeGet , &portInfoGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/* fill 'valid' information according to the type */
static void fillTrgPortInfo(
    IN CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT   infoType,
    IN CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT        *portInfoPtr,
    IN GT_PHYSICAL_PORT_NUM                         portNum /* 0..16 */
)
{
    CPSS_PX_EGRESS_TARGET_PORT_COMMON_STC  *info_commonPtr;

    cpssOsMemSet(portInfoPtr,0xff,sizeof(CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT));

    switch (infoType)
    {
        case CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_COMMON_E:
            info_commonPtr = &portInfoPtr->info_common;

            info_commonPtr->tpid            = (GT_U16)(0xF55F + portNum);             /*16 bits*/
            info_commonPtr->pcid            = 0xF77 + portNum;              /*12 bits*/
            info_commonPtr->egressDelay     = (BIT_20-1) - portNum;         /*20 bits*/
            break;
        case CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_EVB_E:
            info_commonPtr = &portInfoPtr->info_common;
            info_commonPtr->tpid            = (GT_U16)(0xF55F + portNum);             /*16 bits*/
            break;
        default:
            portInfoPtr->notNeeded = 0;
    }
}

void static compareTrgPortInfo(
    IN CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT   infoType,
    IN CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT        *expectedPortInfoPtr,
    IN CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT        *actualPortInfoPtr
)
{
    if(infoType == CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_COMMON_E)
    {
        CPSS_PX_EGRESS_TARGET_PORT_COMMON_STC  *expPtr;
        CPSS_PX_EGRESS_TARGET_PORT_COMMON_STC  *actPtr;

        expPtr = &expectedPortInfoPtr->info_common;
        actPtr = &actualPortInfoPtr->info_common;

        COMPARE_FIELD(tpid             );
        COMPARE_FIELD(pcid             );
        COMPARE_FIELD(egressDelay      );
    }
}

/* set info with 'invalid' information according to the type */
/* return indication if entry can get 'bad param' with the returned info :
    GT_TRUE  - entry can     get 'bad param' with the returned info
    GT_FALSE - entry can NOT get 'bad param' with the returned info
*/
static GT_BOOL fillTrgPortInfo_invalid(
    IN CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT   infoType,
    IN CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT        *portInfoPtr,
    IN GT_PHYSICAL_PORT_NUM                         portNum /* 0..16 */
)
{
    CPSS_PX_EGRESS_TARGET_PORT_COMMON_STC  *info_commonPtr;

    /* fill 'valid' information according to the type */
    fillTrgPortInfo(infoType,portInfoPtr,portNum);

    if(infoType == CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_COMMON_E)
    {
        info_commonPtr = &portInfoPtr->info_common;

        switch(portNum % 3) /* 4 fields that are not BOOL and can get bad param */
        {
            default:/*just for compiler warnings*/
            case 1:
                info_commonPtr->pcid        = BIT_12;
                break;
            case 2:
                info_commonPtr->egressDelay = BIT_20;
                break;
        }

        /*GT_TRUE  - entry can     get 'bad param' with the returned info*/
        return GT_TRUE;
    }

    /*GT_FALSE - entry can NOT get 'bad param' with the returned info*/
    return GT_FALSE;
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxEgressTargetPortEntrySet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT infoType,
    IN CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT     *portInfoPtr
);
*/
UTF_TEST_CASE_MAC(cpssPxEgressTargetPortEntrySet)
{
    GT_STATUS                   st = GT_OK;
    GT_U8                       dev;

    GT_PHYSICAL_PORT_NUM        port;
    CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT   infoType,infoTypeGet;
    CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT        portInfo,portInfoGet;
    GT_BOOL canFail; /* indication that entry in specific operation mode can get bad parameters */

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function with packetType = 0..31 , useTod [GT_FALSE and GT_TRUE].           */
        /* Expected: GT_OK     */
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. call set function to check valid values */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            infoType = port % CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE___MUST_BE_LAST___E;

            /* fill 'valid' information according to the type */
            fillTrgPortInfo(infoType,&portInfo,port);

            /* 1.1. call set function to check valid values */
            st = cpssPxEgressTargetPortEntrySet(dev, port ,infoType, &portInfo);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port ,infoType);
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. call get function to valid check values */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            infoType = port % CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE___MUST_BE_LAST___E;

            /* fill 'valid' information according to the type */
            fillTrgPortInfo(infoType,&portInfo,port);

            /* 1.2. call get function to valid check values */
            st = cpssPxEgressTargetPortEntryGet(dev, port ,&infoTypeGet, &portInfoGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            UTF_VERIFY_EQUAL2_STRING_MAC(infoType, infoTypeGet,
                "get another infoType than was set on device: %d , port: %d ",
                dev,port);

            if(infoType == infoTypeGet)
            {
                compareTrgPortInfo(infoType,&portInfo,&portInfoGet);
            }
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.3. call set function to check invalid values */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            infoType = port % CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE___MUST_BE_LAST___E;

            /* set info with 'invalid' information according to the type */
            canFail = fillTrgPortInfo_invalid(infoType,&portInfo,port);

            /* 1.3. call set function to check invalid values */
            st = cpssPxEgressTargetPortEntrySet(dev, port ,infoType, &portInfo);
            if(canFail == GT_TRUE)
            {
                /* we can cause to the entry to bad param */
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, port ,infoType);
            }
            else
            {
                /* we can NOT cause to the entry to bad param */
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port ,infoType);
            }
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.4. call get function to valid check values ---
                after we called entries with BAD_PARAM --> check ALL is 'as was before' */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            infoType = port % CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE___MUST_BE_LAST___E;

            /* fill 'valid' information according to the type */
            fillTrgPortInfo(infoType,&portInfo,port);

                /* 1.4. call get function to valid check values ---
                        after we called entries with BAD_PARAM --> check ALL is 'as was before' */
            st = cpssPxEgressTargetPortEntryGet(dev, port ,&infoTypeGet, &portInfoGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            UTF_VERIFY_EQUAL2_STRING_MAC(infoType, infoTypeGet,
                "get another infoType than was set on device: %d , port: %d ",
                dev,port);

            if(infoType == infoTypeGet)
            {
                compareTrgPortInfo(infoType,&portInfo,&portInfoGet);
            }
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.4. call get function to valid check values ---
                after we called entries with BAD_PARAM --> check ALL is 'as was before' */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /* break -- to get valid port */
            break;
        }

        infoType    = 0;

        /* fill 'valid' operation information according to the operation type */
        fillTrgPortInfo(infoType,&portInfo,port);

        /* make sure function still PASS -- after all previous changes in parameters */
        /* before we start the 'expecting errors' section !!! */
        st = cpssPxEgressTargetPortEntrySet(dev, port ,infoType, &portInfo);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);


        /* 1.5. call set function with invalid ENUM : <infoType> */
        UTF_ENUMS_CHECK_MAC(cpssPxEgressTargetPortEntrySet(dev, port ,infoType, &portInfo),
                            infoType);

        infoType = 0;

        /* 1.6. call set function with NULL pointer */
        st = cpssPxEgressTargetPortEntrySet(dev, port ,infoType, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);

        /* 1.7. call set function with bad infoType */
        for(infoType = CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE___MUST_BE_LAST___E;
            infoType < (321 + CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE___MUST_BE_LAST___E);
            infoType += 17)
        {
            st = cpssPxEgressTargetPortEntrySet(dev, port ,infoType, &portInfo);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        infoType = 0;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.9. call set function with not valid physical ports */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssPxEgressTargetPortEntrySet(dev, port ,infoType, &portInfo);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
    }

    infoType    = 0;
    port = 0;

    /* fill 'valid' operation information according to the operation type */
    fillTrgPortInfo(infoType,&portInfo,port);

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /*2. Go over all non active/non applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. <Call function for non active/non applicable device and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssPxEgressTargetPortEntrySet(dev, port ,infoType, &portInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                          */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxEgressTargetPortEntrySet(dev, port ,infoType, &portInfo);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxEgressTargetPortEntryGet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT *infoTypePtr,
    OUT CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT      *portInfoPtr
);
*/
UTF_TEST_CASE_MAC(cpssPxEgressTargetPortEntryGet)
{
    GT_STATUS                   st = GT_OK;
    GT_U8                       dev;

    GT_PHYSICAL_PORT_NUM        port;
    CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT   infoTypeGet;
    CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT        portInfoGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function with packetType = 0..31 , useTod [GT_FALSE and GT_TRUE].           */
        /* Expected: GT_OK     */
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. call set function to check valid values */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /* 1.1. call get function to check valid values */
            st = cpssPxEgressTargetPortEntryGet(dev, port , &infoTypeGet , &portInfoGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.2. call function with NULL pointer */
            st = cpssPxEgressTargetPortEntryGet(dev, port ,&infoTypeGet , NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);

            /* 1.3. call function with NULL pointer */
            st = cpssPxEgressTargetPortEntryGet(dev, port ,NULL , &portInfoGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.5. call set function with not valid physical ports */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssPxEgressTargetPortEntryGet(dev, port , &infoTypeGet , &portInfoGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
    }

    port = 0;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /*2. Go over all non active/non applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. <Call function for non active/non applicable device and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssPxEgressTargetPortEntryGet(dev, port , &infoTypeGet , &portInfoGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                          */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxEgressTargetPortEntryGet(dev, port , &infoTypeGet , &portInfoGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

#define MAX_VLANS_CNS                        4
#define UT_MAX_PORTS                        16
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxEgressVlanTagStateEntrySet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32               vlanId,
    IN CPSS_PX_PORTS_BMP    portsTagging
);
*/
UTF_TEST_CASE_MAC(cpssPxEgressVlanTagStateEntrySet)
{
    GT_STATUS                   st = GT_OK;
    GT_U8                       dev;
    GT_U32                      vlanId;
    GT_PHYSICAL_PORT_NUM        port;
    GT_U32                      vlanArr[MAX_VLANS_CNS] = {1, 12, 350, 4095};
    CPSS_PX_PORTS_BMP           portsTagging, portsTaggingGet;
    GT_U32                      i;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function with valid parameters
           Expected: GT_OK */
        for(i = 0; i < MAX_VLANS_CNS; i++)
        {
            vlanId = vlanArr[i];
            portsTagging = 0;

            for(port = 0; port < UT_MAX_PORTS; port++)
            {
                portsTagging |= 1 << port;

                st = cpssPxEgressVlanTagStateEntrySet(dev, vlanId, portsTagging);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vlanId, portsTagging);

                st = cpssPxEgressVlanTagStateEntryGet(dev, vlanId, &portsTaggingGet);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                UTF_VERIFY_EQUAL2_STRING_MAC(portsTagging, portsTaggingGet,
                    "get another portsTagging than was set on device: %d %d", dev, portsTaggingGet);
            }
        }
        /* 1.2. Call function with not valid parameters
           Expected: GT_BAD_PARAM */
        vlanId += 1;
        st = cpssPxEgressVlanTagStateEntrySet(dev, vlanId, portsTagging);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, vlanId, portsTagging);

        vlanId = 1;
        portsTagging = 0x20000;

        st = cpssPxEgressVlanTagStateEntrySet(dev, vlanId, portsTagging);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, vlanId, portsTagging);
    }

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    portsTagging = 0xFF;
    /*2. Go over all non active/non applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. <Call function for non active/non applicable device and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssPxEgressVlanTagStateEntrySet(dev, vlanId, portsTagging);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                          */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxEgressVlanTagStateEntrySet(dev, vlanId, portsTagging);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxEgressVlanTagStateEntryGet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32               vlanId,
    OUT CPSS_PX_PORTS_BMP   *portsTaggingPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxEgressVlanTagStateEntryGet)
{
    GT_STATUS                   st = GT_OK;
    GT_U8                       dev;
    GT_U32                      vlanId = 100;
    CPSS_PX_PORTS_BMP           portsTaggingGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function with valid parameters
           Expected: GT_OK */
        st = cpssPxEgressVlanTagStateEntryGet(dev, vlanId, &portsTaggingGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2. Call function with not valid parameter
           Expected: GT_BAD_PARAM */
        vlanId = 4096;
        st = cpssPxEgressVlanTagStateEntryGet(dev, vlanId, &portsTaggingGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* 1.3. Call function with NULL pointer
           Expected: GT_BAD_PTR */
        vlanId = 100;
        st = cpssPxEgressVlanTagStateEntryGet(dev, vlanId, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*2. Go over all non active/non applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. <Call function for non active/non applicable device and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssPxEgressVlanTagStateEntryGet(dev, vlanId, &portsTaggingGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                          */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxEgressVlanTagStateEntryGet(dev, vlanId, &portsTaggingGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssPxEgress suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssPxEgress)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxEgressBypassModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxEgressBypassModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxEgressTimestampModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxEgressTimestampModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxEgressHeaderAlterationEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxEgressHeaderAlterationEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxEgressSourcePortEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxEgressSourcePortEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxEgressTargetPortEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxEgressTargetPortEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxEgressVlanTagStateEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxEgressVlanTagStateEntryGet)
UTF_SUIT_END_TESTS_MAC(cpssPxEgress)


