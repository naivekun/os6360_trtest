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
* @file cpssPxCosUT.c
*
* @brief Unit tests for cpssPxCos that provides
* CPSS implementation for Pipe CoS processing.
*
* @version   1
********************************************************************************
*/

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

#include <cpss/px/cpssPxTypes.h>
#include <cpss/px/cos/cpssPxCos.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>

/* defines */

/* Default valid value for port id */
#define PORT_CTRL_VALID_PHY_PORT_CNS  0

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxCosFormatEntrySet
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  CPSS_PX_PACKET_TYPE                 packetType,
    IN  CPSS_PX_COS_FORMAT_ENTRY_STC        *cosFormatEntryPtr
);
*/ 
UTF_TEST_CASE_MAC(cpssPxCosFormatEntrySet)
{
/*
    ITERATE_DEVICE (Go over all Px devices)
    1.1. Call with packetType[0..31], with valid cosFormatEntryPtr.
    Expected: GT_OK.
    1.2. Call with out of range packetType[32] and other params from 1.1.
    Expected: GT_OUT_OF_RANGE.
    1.3. Call with cosFormatEntryPtr->cosMode == LAST.
    Expected: GT_BAD_PARAM.
    1.4. Call with cosFormatEntryPtr->cosMode == PACKET_L2 and cosByteOffset == 100.
    Expected: GT_BAD_PARAM.
    1.5. Call with cosFormatEntryPtr->cosMode == FORMAT_ENTRY and tc == 10.
    Expected: GT_BAD_PARAM.
    1.6. Call with cosFormatEntryGet.cosMode = CPSS_PX_COS_MODE_PACKET_L2_E
    and cosFormatEntryPtr->cosBitOffset + cosFormatEntryPtr->cosNumOfBits > 8
    Expected: GT_OK.
    1.7. Call with cosFormatEntryPtr == NULL and other params from 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS           st  = GT_OK;
    GT_U8               dev;
    CPSS_PX_PACKET_TYPE packetType;
    GT_U32              index; 
    GT_U32              numOfFormatEntries;

    CPSS_PX_COS_FORMAT_ENTRY_STC    cosFormatEntryArr[] =
    {
        {CPSS_PX_COS_MODE_PORT_E},
        {CPSS_PX_COS_MODE_PACKET_DSA_E, 12, 0, 7},
        {CPSS_PX_COS_MODE_PACKET_L2_E, 13, 0, 4},
        {CPSS_PX_COS_MODE_PACKET_L3_E,  1, 2, 6},
        {CPSS_PX_COS_MODE_PACKET_MPLS_E,  3, 1, 3},
        {CPSS_PX_COS_MODE_FORMAT_ENTRY_E, 0, 0, 0, {2, CPSS_DP_GREEN_E, 5, 0}},
        {CPSS_PX_COS_MODE_PACKET_DSA_E, 18, 4, 7}
    };
    CPSS_PX_COS_FORMAT_ENTRY_STC    cosFormatEntryGet;
    numOfFormatEntries = sizeof(cosFormatEntryArr) / sizeof(cosFormatEntryArr[0]);

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    { 
        /*  1.1. Call with packetType[0..31], with valid cosFormatEntryPtr.
            Expected: GT_OK. */
        for (packetType = 0; packetType < CPSS_PX_PACKET_TYPE_MAX_CNS; packetType++)
        {
            index = packetType % numOfFormatEntries;
            st = cpssPxCosFormatEntrySet(dev,
                    packetType,
                    &cosFormatEntryArr[index]);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, packetType);

            st = cpssPxCosFormatEntryGet(dev, packetType, &cosFormatEntryGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, packetType); 

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(cosFormatEntryArr[index].cosMode,
                                        cosFormatEntryGet.cosMode,
                       "get another cosMode than was set: %d", packetType); 
            UTF_VERIFY_EQUAL1_STRING_MAC(cosFormatEntryArr[index].cosByteOffset,
                                        cosFormatEntryGet.cosByteOffset,
                       "get another cosByteOffset than was set: %d", packetType); 
            UTF_VERIFY_EQUAL1_STRING_MAC(cosFormatEntryArr[index].cosBitOffset,
                                        cosFormatEntryGet.cosBitOffset,
                       "get another cosBitOffset than was set: %d", packetType); 
            UTF_VERIFY_EQUAL1_STRING_MAC(cosFormatEntryArr[index].cosAttributes.trafficClass,
                                        cosFormatEntryGet.cosAttributes.trafficClass,
                       "get another trafficClass than was set: %d", packetType); 
            UTF_VERIFY_EQUAL1_STRING_MAC(cosFormatEntryArr[index].cosAttributes.dropPrecedence,
                                        cosFormatEntryGet.cosAttributes.dropPrecedence,
                       "get another dropPrecedence than was set: %d", packetType); 
            UTF_VERIFY_EQUAL1_STRING_MAC(cosFormatEntryArr[index].cosAttributes.userPriority,
                                        cosFormatEntryGet.cosAttributes.userPriority,
                       "get another userPriority than was set: %d", packetType); 
            UTF_VERIFY_EQUAL1_STRING_MAC(cosFormatEntryArr[index].cosAttributes.dropEligibilityIndication,
                                        cosFormatEntryGet.cosAttributes.dropEligibilityIndication,
                       "get another dropEligibilityIndication than was set: %d", packetType); 
        }
        /*  1.2. Call with out of range packetType[32] and other params from 1.1.
            Expected: GT_OUT_OF_RANGE. */
        st = cpssPxCosFormatEntrySet(dev, packetType, &cosFormatEntryArr[0]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, packetType); 

        /*  1.3. Call with cosFormatEntryPtr->cosMode == LAST.
            Expected: GT_BAD_PARAM. */
        cosFormatEntryGet = cosFormatEntryArr[0];
        cosFormatEntryGet.cosMode = CPSS_PX_COS_MODE_LAST_E;
        st = cpssPxCosFormatEntrySet(dev, 0, &cosFormatEntryGet);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);

        /*  1.4. Call with cosFormatEntryPtr->cosMode == PACKET_L2 and cosByteOffset == 100.
            Expected: GT_BAD_PARAM. */
        cosFormatEntryGet.cosMode = CPSS_PX_COS_MODE_PACKET_L2_E;
        cosFormatEntryGet.cosByteOffset = 100;
        st = cpssPxCosFormatEntrySet(dev, 0, &cosFormatEntryGet);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);

        /*  1.5. Call with cosFormatEntryPtr->cosMode == FORMAT_ENTRY and tc == 10.
            Expected: GT_BAD_PARAM. */
        cosFormatEntryGet.cosMode = CPSS_PX_COS_MODE_FORMAT_ENTRY_E;
        cosFormatEntryGet.cosAttributes.trafficClass = 10;
        st = cpssPxCosFormatEntrySet(dev, 0, &cosFormatEntryGet);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);

        /*  1.6. Call with cosFormatEntryPtr->cosBitOffset + cosFormatEntryPtr->cosNumOfBits > 8
            Expected: GT_OK */
        cosFormatEntryGet.cosMode = CPSS_PX_COS_MODE_PACKET_L2_E;
        cosFormatEntryGet.cosByteOffset = 12;
        cosFormatEntryGet.cosBitOffset = 6;
        cosFormatEntryGet.cosNumOfBits = 3;
        st = cpssPxCosFormatEntrySet(dev, 0, &cosFormatEntryGet);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*  1.7. Call with cosFormatEntryPtr == NULL and other params from 1.1.
            Expected: GT_BAD_PTR. */
        st = cpssPxCosFormatEntrySet(dev, 0, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxCosFormatEntrySet(dev, 0, &cosFormatEntryArr[0]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    } 
 
    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxCosFormatEntrySet(dev, 0, &cosFormatEntryArr[0]);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);  
}

/*
GT_STATUS cpssPxCosFormatEntryGet
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  CPSS_PX_PACKET_TYPE                 packetType,
    OUT CPSS_PX_COS_FORMAT_ENTRY_STC        *cosFormatEntryPtr
);
*/
UTF_TEST_CASE_MAC(cpssPxCosFormatEntryGet)
{
/*
    ITERATE_DEVICE (Go over all Px devices)
    1.1. Call with packetType[0..31], with valid cosFormatEntryPtr.
    Expected: GT_OK.
    1.2. Call with out of range packetType[32].
    Expected: GT_OUT_OF_RANGE.
    1.3. Call with cosFormatEntryPtr == NULL.
    Expected: GT_BAD_PTR.

*/

    GT_STATUS           st     = GT_OK;
    GT_U8               dev;
    CPSS_PX_PACKET_TYPE packetType;

    CPSS_PX_COS_FORMAT_ENTRY_STC    cosFormatEntryGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. Call with packetType[0..31], with valid cosFormatEntryPtr.
            Expected: GT_OK. */
        for (packetType = 0; packetType < CPSS_PX_PACKET_TYPE_MAX_CNS; packetType++)
        {
            st = cpssPxCosFormatEntryGet(dev, packetType, &cosFormatEntryGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, packetType); 
        }

        /*  1.2. Call with out of range packetType[32].
            Expected: GT_OUT_OF_RANGE. */
        st = cpssPxCosFormatEntryGet(dev, packetType, &cosFormatEntryGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, packetType); 

        /*  1.3. Call with cosFormatEntryPtr == NULL.
            Expected: GT_BAD_PTR. */
        st = cpssPxCosFormatEntryGet(dev, 0, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st); 
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxCosFormatEntryGet(dev, 0, &cosFormatEntryGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    } 

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxCosFormatEntryGet(dev, 0, &cosFormatEntryGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);  
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxCosPortAttributesSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PX_COS_ATTRIBUTES_STC  *cosAttributesPtr
);
*/ 
UTF_TEST_CASE_MAC(cpssPxCosPortAttributesSet)
{
/*
    ITERATE_DEVICE (Go over all Px devices)
    1.1. Call with portNum[0..16], with valid cosAttributesPtr.
    Expected: GT_OK.
    1.2. Call with out of range portNum[17] and other params from 1.1.
    Expected: GT_BAD_PARAM.
    1.3. Call with cosAttributesPtr->trafficClass == 10.
    Expected: GT_BAD_PARAM.
    1.4. Call with cosAttributesPtr == NULL and other params from 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS               st  = GT_OK;
    GT_U8                   dev;
    GT_PHYSICAL_PORT_NUM    portNum;

    CPSS_PX_COS_ATTRIBUTES_STC    cosAttributes;
    CPSS_PX_COS_ATTRIBUTES_STC    cosAttributesGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    { 
        /*  1.1. Call with portNum[0..16], with valid cosFormatEntryPtr.
            Expected: GT_OK. */
        for (portNum = 0; portNum < PRV_CPSS_PX_PORTS_NUM_CNS; portNum++)
        {
            cosAttributes.trafficClass = portNum % 8;
            cosAttributes.dropPrecedence = (CPSS_DP_LEVEL_ENT) (portNum % CPSS_DP_LAST_E);
            cosAttributes.userPriority = (portNum * 3) % 8;
            cosAttributes.dropEligibilityIndication = portNum % 2;
            st = cpssPxCosPortAttributesSet(dev,
                    portNum,
                    &cosAttributes);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, portNum);

            st = cpssPxCosPortAttributesGet(dev, portNum, &cosAttributesGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, portNum); 

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(cosAttributes.trafficClass,
                                        cosAttributesGet.trafficClass,
                       "get another trafficClass than was set: %d", portNum); 
            UTF_VERIFY_EQUAL1_STRING_MAC(cosAttributes.dropPrecedence,
                                        cosAttributesGet.dropPrecedence,
                       "get another dropPrecedence than was set: %d", portNum); 
            UTF_VERIFY_EQUAL1_STRING_MAC(cosAttributes.userPriority,
                                        cosAttributesGet.userPriority,
                       "get another userPriority than was set: %d", portNum); 
            UTF_VERIFY_EQUAL1_STRING_MAC(cosAttributes.dropEligibilityIndication,
                                        cosAttributesGet.dropEligibilityIndication,
                       "get another dropEligibilityIndication than was set: %d", portNum); 
        }
        /*  1.2. Call with out of range portNum[17] and other params from 1.1.
            Expected: GT_BAD_PARAM. */
        st = cpssPxCosPortAttributesSet(dev, portNum, &cosAttributes);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, portNum); 

        /*  1.3. Call with cosAttributesPtr->trafficClass == 10.
            Expected: GT_BAD_PARAM. */
        cosAttributesGet = cosAttributes;
        cosAttributesGet.trafficClass = 10;
        st = cpssPxCosPortAttributesSet(dev, 0, &cosAttributesGet);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);

        /*  1.4. Call with cosAttributesPtr == NULL and other params from 1.1.
            Expected: GT_BAD_PTR. */
        st = cpssPxCosPortAttributesSet(dev, 0, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxCosPortAttributesSet(dev, 0, &cosAttributes);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    } 
 
    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxCosPortAttributesSet(dev, 0, &cosAttributes);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);  
}

/*
GT_STATUS cpssPxCosPortAttributesGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_PX_COS_ATTRIBUTES_STC  *cosAttributesPtr
);
*/
UTF_TEST_CASE_MAC(cpssPxCosPortAttributesGet)
{
/*
    ITERATE_DEVICE (Go over all Px devices)
    1.1. Call with portNum[0..16], with valid cosAttributesPtr.
    Expected: GT_OK.
    1.2. Call with out of range portNum[17].
    Expected: GT_BAD_PARAM.
    1.3. Call with cosAttributesPtr == NULL.
    Expected: GT_BAD_PTR.

*/

    GT_STATUS               st     = GT_OK;
    GT_U8                   dev;
    GT_PHYSICAL_PORT_NUM    portNum;

    CPSS_PX_COS_ATTRIBUTES_STC    cosAttributesGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. Call with portNum[0..16], with valid cosAttributesPtr.
            Expected: GT_OK. */
        for (portNum = 0; portNum < PRV_CPSS_PX_PORTS_NUM_CNS; portNum++)
        {
            st = cpssPxCosPortAttributesGet(dev, portNum, &cosAttributesGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, portNum); 
        }

        /*  1.2. Call with out of range portNum[17].
            Expected: GT_BAD_PARAM. */
        st = cpssPxCosPortAttributesGet(dev, portNum, &cosAttributesGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, portNum); 

        /*  1.3. Call with cosAttributesPtr == NULL.
            Expected: GT_BAD_PTR. */
        st = cpssPxCosPortAttributesGet(dev, 0, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st); 
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxCosPortAttributesGet(dev, 0, &cosAttributesGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    } 

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxCosPortAttributesGet(dev, 0, &cosAttributesGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);  
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxCosPortL2MappingSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_U32                      l2Index,
    IN  CPSS_PX_COS_ATTRIBUTES_STC  *cosAttributesPtr
);
*/ 
UTF_TEST_CASE_MAC(cpssPxCosPortL2MappingSet)
{
/*
    ITERATE_DEVICE (Go over all Px devices)
    1.1. Call with portNum[0..16], with valid cosAttributesPtr.
    Expected: GT_OK.
    1.2. Call with out of range portNum[17] and other params from 1.1.
    Expected: GT_BAD_PARAM.
    1.3. Call with cosAttributesPtr->trafficClass == 10.
    Expected: GT_BAD_PARAM.
    1.4. Call with cosAttributesPtr == NULL and other params from 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS               st  = GT_OK;
    GT_U8                   dev;
    GT_PHYSICAL_PORT_NUM    portNum;

    CPSS_PX_COS_ATTRIBUTES_STC    cosAttributes;
    CPSS_PX_COS_ATTRIBUTES_STC    cosAttributesGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    { 
        /*  1.1. Call with portNum[0..16], with valid cosFormatEntryPtr.
            Expected: GT_OK. */
        for (portNum = 0; portNum < PRV_CPSS_PX_PORTS_NUM_CNS; portNum++)
        {
            cosAttributes.trafficClass = portNum % 8;
            cosAttributes.dropPrecedence = (CPSS_DP_LEVEL_ENT) (portNum % CPSS_DP_LAST_E);
            cosAttributes.userPriority = (portNum * 3) % 8;
            cosAttributes.dropEligibilityIndication = portNum % 2;
            st = cpssPxCosPortL2MappingSet(dev,
                    portNum,
                    portNum % 16,
                    &cosAttributes);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, portNum);

            st = cpssPxCosPortL2MappingGet(dev, portNum, portNum % 16, &cosAttributesGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, portNum); 

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(cosAttributes.trafficClass,
                                        cosAttributesGet.trafficClass,
                       "get another trafficClass than was set: %d", portNum); 
            UTF_VERIFY_EQUAL1_STRING_MAC(cosAttributes.dropPrecedence,
                                        cosAttributesGet.dropPrecedence,
                       "get another dropPrecedence than was set: %d", portNum); 
            UTF_VERIFY_EQUAL1_STRING_MAC(cosAttributes.userPriority,
                                        cosAttributesGet.userPriority,
                       "get another userPriority than was set: %d", portNum); 
            UTF_VERIFY_EQUAL1_STRING_MAC(cosAttributes.dropEligibilityIndication,
                                        cosAttributesGet.dropEligibilityIndication,
                       "get another dropEligibilityIndication than was set: %d", portNum); 
        }
        /*  1.2. Call with out of range portNum[17] and other params from 1.1.
            Expected: GT_BAD_PARAM. */
        st = cpssPxCosPortL2MappingSet(dev, portNum, portNum % 16, &cosAttributes);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, portNum);

        /*  1.3. Call with cosAttributesPtr->trafficClass == 10.
            Expected: GT_BAD_PARAM. */
        cosAttributesGet = cosAttributes;
        cosAttributesGet.trafficClass = 10;
        st = cpssPxCosPortL2MappingSet(dev, 0, 0, &cosAttributesGet);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);

        /*  1.4. Call with cosAttributesPtr == NULL and other params from 1.1.
            Expected: GT_BAD_PTR. */
        st = cpssPxCosPortL2MappingSet(dev, 0, 0, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxCosPortL2MappingSet(dev, 0, 0, &cosAttributes);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    } 
 
    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxCosPortL2MappingSet(dev, 0, 0, &cosAttributes);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);  
}

/*
GT_STATUS cpssPxCosPortL2MappingGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_U32                      l2Index,
    OUT CPSS_PX_COS_ATTRIBUTES_STC  *cosAttributesPtr
);
*/
UTF_TEST_CASE_MAC(cpssPxCosPortL2MappingGet)
{
/*
    ITERATE_DEVICE (Go over all Px devices)
    1.1. Call with portNum[0..16], with valid cosAttributesPtr.
    Expected: GT_OK.
    1.2. Call with out of range portNum[17].
    Expected: GT_BAD_PARAM.
    1.3. Call with cosAttributesPtr == NULL.
    Expected: GT_BAD_PTR.

*/

    GT_STATUS               st     = GT_OK;
    GT_U8                   dev;
    GT_PHYSICAL_PORT_NUM    portNum;

    CPSS_PX_COS_ATTRIBUTES_STC    cosAttributesGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. Call with portNum[0..16], with valid cosAttributesPtr.
            Expected: GT_OK. */
        for (portNum = 0; portNum < PRV_CPSS_PX_PORTS_NUM_CNS; portNum++)
        {
            st = cpssPxCosPortL2MappingGet(dev, portNum, portNum % 16, &cosAttributesGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, portNum);
        }

        /*  1.2. Call with out of range portNum[17].
            Expected: GT_BAD_PARAM. */
        st = cpssPxCosPortL2MappingGet(dev, portNum, portNum % 16, &cosAttributesGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, portNum); 

        /*  1.3. Call with cosAttributesPtr == NULL.
            Expected: GT_BAD_PTR. */
        st = cpssPxCosPortL2MappingGet(dev, 0, 0, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st); 
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxCosPortL2MappingGet(dev, 0, 0, &cosAttributesGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    } 

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxCosPortL2MappingGet(dev, 0, 0, &cosAttributesGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);  
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS 
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_U32                      l3Index,
    IN  CPSS_PX_COS_ATTRIBUTES_STC  *cosAttributesPtr
);
*/ 
UTF_TEST_CASE_MAC(cpssPxCosL3MappingSet)
{
/*
    ITERATE_DEVICE (Go over all Px devices)
    1.1. Call with l3Index[0..63], with valid cosAttributesPtr.
    Expected: GT_OK.
    1.2. Call with out of range l3Index[64] and other params from 1.1.
    Expected: GT_OUT_OF_RANGE.
    1.3. Call with cosAttributesPtr->trafficClass == 10.
    Expected: GT_BAD_PARAM.
    1.4. Call with cosAttributesPtr == NULL and other params from 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_U32      l3Index;

    CPSS_PX_COS_ATTRIBUTES_STC    cosAttributes;
    CPSS_PX_COS_ATTRIBUTES_STC    cosAttributesGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    { 
        /*  1.1. Call with l3Index[0..63], with valid cosFormatEntryPtr.
            Expected: GT_OK. */
        for (l3Index = 0; l3Index < BIT_6; l3Index++)
        {
            cosAttributes.trafficClass = l3Index % 8;
            cosAttributes.dropPrecedence = (CPSS_DP_LEVEL_ENT) (l3Index % CPSS_DP_LAST_E);
            cosAttributes.userPriority = (l3Index * 3) % 8;
            cosAttributes.dropEligibilityIndication = l3Index % 2;
            st = cpssPxCosL3MappingSet(dev, l3Index, &cosAttributes);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, l3Index);

            st = cpssPxCosL3MappingGet(dev, l3Index, &cosAttributesGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, l3Index); 

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(cosAttributes.trafficClass,
                                        cosAttributesGet.trafficClass,
                       "get another trafficClass than was set: %d", l3Index); 
            UTF_VERIFY_EQUAL1_STRING_MAC(cosAttributes.dropPrecedence,
                                        cosAttributesGet.dropPrecedence,
                       "get another dropPrecedence than was set: %d", l3Index); 
            UTF_VERIFY_EQUAL1_STRING_MAC(cosAttributes.userPriority,
                                        cosAttributesGet.userPriority,
                       "get another userPriority than was set: %d", l3Index); 
            UTF_VERIFY_EQUAL1_STRING_MAC(cosAttributes.dropEligibilityIndication,
                                        cosAttributesGet.dropEligibilityIndication,
                       "get another dropEligibilityIndication than was set: %d", l3Index); 
        }
        /*  1.2. Call with out of range l3Index[64] and other params from 1.1.
            Expected: GT_OUT_OF_RANGE. */
        st = cpssPxCosL3MappingSet(dev, l3Index, &cosAttributes);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, l3Index);

        /*  1.3. Call with cosAttributesPtr->trafficClass == 10.
            Expected: GT_BAD_PARAM. */
        cosAttributesGet = cosAttributes;
        cosAttributesGet.trafficClass = 10;
        st = cpssPxCosL3MappingSet(dev, 0, &cosAttributesGet);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);

        /*  1.4. Call with cosAttributesPtr == NULL and other params from 1.1.
            Expected: GT_BAD_PTR. */
        st = cpssPxCosL3MappingSet(dev, 0, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxCosL3MappingSet(dev, 0, &cosAttributes);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    } 
 
    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxCosL3MappingSet(dev, 0, &cosAttributes);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);  
}

/*
GT_STATUS cpssPxCosL3MappingGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_U32                      l3Index,
    OUT CPSS_PX_COS_ATTRIBUTES_STC  *cosAttributesPtr
);
*/
UTF_TEST_CASE_MAC(cpssPxCosL3MappingGet)
{
/*
    ITERATE_DEVICE (Go over all Px devices)
    1.1. Call with l3Index[0..63], with valid cosAttributesPtr.
    Expected: GT_OK.
    1.2. Call with out of range l3Index[64].
    Expected: GT_OUT_OF_RANGE.
    1.3. Call with cosAttributesPtr == NULL.
    Expected: GT_BAD_PTR.

*/

    GT_STATUS   st     = GT_OK;
    GT_U8       dev;
    GT_U32      l3Index;

    CPSS_PX_COS_ATTRIBUTES_STC    cosAttributesGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. Call with l3Index[0..63], with valid cosAttributesPtr.
            Expected: GT_OK. */
        for (l3Index = 0; l3Index < BIT_6; l3Index++)
        {
            st = cpssPxCosL3MappingGet(dev, l3Index, &cosAttributesGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, l3Index);
        }

        /*  1.2. Call with out of range l3Index[64].
            Expected: GT_OUT_OF_RANGE. */
        st = cpssPxCosL3MappingGet(dev, l3Index, &cosAttributesGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, l3Index); 

        /*  1.3. Call with cosAttributesPtr == NULL.
            Expected: GT_BAD_PTR. */
        st = cpssPxCosL3MappingGet(dev, 0, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st); 
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxCosL3MappingGet(dev, 0, &cosAttributesGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    } 

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxCosL3MappingGet(dev, 0, &cosAttributesGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);  
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxCosMplsMappingSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_U32                      mplsIndex,
    IN  CPSS_PX_COS_ATTRIBUTES_STC  *cosAttributesPtr
);
*/ 
UTF_TEST_CASE_MAC(cpssPxCosMplsMappingSet)
{
/*
    ITERATE_DEVICE (Go over all Px devices)
    1.1. Call with mplsIndex[0..7], with valid cosAttributesPtr.
    Expected: GT_OK.
    1.2. Call with out of range mplsIndex[8] and other params from 1.1.
    Expected: GT_OUT_OF_RANGE.
    1.3. Call with cosAttributesPtr->trafficClass == 10.
    Expected: GT_BAD_PARAM.
    1.4. Call with cosAttributesPtr == NULL and other params from 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_U32      mplsIndex;

    CPSS_PX_COS_ATTRIBUTES_STC    cosAttributes;
    CPSS_PX_COS_ATTRIBUTES_STC    cosAttributesGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    { 
        /*  1.1. Call with mplsIndex[0..7], with valid cosFormatEntryPtr.
            Expected: GT_OK. */
        for (mplsIndex = 0; mplsIndex < BIT_3; mplsIndex++)
        {
            cosAttributes.trafficClass = mplsIndex % 8;
            cosAttributes.dropPrecedence = (CPSS_DP_LEVEL_ENT) (mplsIndex % CPSS_DP_LAST_E);
            cosAttributes.userPriority = (mplsIndex * 3) % 8;
            cosAttributes.dropEligibilityIndication = mplsIndex % 2;
            st = cpssPxCosMplsMappingSet(dev, mplsIndex, &cosAttributes);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, mplsIndex);

            st = cpssPxCosMplsMappingGet(dev, mplsIndex, &cosAttributesGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, mplsIndex); 

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(cosAttributes.trafficClass,
                                        cosAttributesGet.trafficClass,
                       "get another trafficClass than was set: %d", mplsIndex); 
            UTF_VERIFY_EQUAL1_STRING_MAC(cosAttributes.dropPrecedence,
                                        cosAttributesGet.dropPrecedence,
                       "get another dropPrecedence than was set: %d", mplsIndex); 
            UTF_VERIFY_EQUAL1_STRING_MAC(cosAttributes.userPriority,
                                        cosAttributesGet.userPriority,
                       "get another userPriority than was set: %d", mplsIndex); 
            UTF_VERIFY_EQUAL1_STRING_MAC(cosAttributes.dropEligibilityIndication,
                                        cosAttributesGet.dropEligibilityIndication,
                       "get another dropEligibilityIndication than was set: %d", mplsIndex); 
        }
        /*  1.2. Call with out of range mplsIndex[8] and other params from 1.1.
            Expected: GT_OUT_OF_RANGE. */
        st = cpssPxCosMplsMappingSet(dev, mplsIndex, &cosAttributes);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, mplsIndex);

        /*  1.3. Call with cosAttributesPtr->trafficClass == 10.
            Expected: GT_BAD_PARAM. */
        cosAttributesGet = cosAttributes;
        cosAttributesGet.trafficClass = 10;
        st = cpssPxCosMplsMappingSet(dev, 0, &cosAttributesGet);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);

        /*  1.4. Call with cosAttributesPtr == NULL and other params from 1.1.
            Expected: GT_BAD_PTR. */
        st = cpssPxCosMplsMappingSet(dev, 0, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxCosMplsMappingSet(dev, 0, &cosAttributes);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    } 
 
    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxCosMplsMappingSet(dev, 0, &cosAttributes);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);  
}

/*
GT_STATUS cpssPxCosMplsMappingGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_U32                      mplsIndex,
    OUT CPSS_PX_COS_ATTRIBUTES_STC  *cosAttributesPtr
);
*/
UTF_TEST_CASE_MAC(cpssPxCosMplsMappingGet)
{
/*
    ITERATE_DEVICE (Go over all Px devices)
    1.1. Call with mplsIndex[0..7], with valid cosAttributesPtr.
    Expected: GT_OK.
    1.2. Call with out of range mplsIndex[8].
    Expected: GT_OUT_OF_RANGE.
    1.3. Call with cosAttributesPtr == NULL.
    Expected: GT_BAD_PTR.

*/

    GT_STATUS   st     = GT_OK;
    GT_U8       dev;
    GT_U32      mplsIndex;

    CPSS_PX_COS_ATTRIBUTES_STC    cosAttributesGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. Call with mplsIndex[0..7], with valid cosAttributesPtr.
            Expected: GT_OK. */
        for (mplsIndex = 0; mplsIndex < BIT_3; mplsIndex++)
        {
            st = cpssPxCosMplsMappingGet(dev, mplsIndex, &cosAttributesGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, mplsIndex);
        }

        /*  1.2. Call with out of range mplsIndex[8].
            Expected: GT_OUT_OF_RANGE. */
        st = cpssPxCosMplsMappingGet(dev, mplsIndex, &cosAttributesGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, mplsIndex); 

        /*  1.3. Call with cosAttributesPtr == NULL.
            Expected: GT_BAD_PTR. */
        st = cpssPxCosMplsMappingGet(dev, 0, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st); 
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxCosMplsMappingGet(dev, 0, &cosAttributesGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    } 

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxCosMplsMappingGet(dev, 0, &cosAttributesGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);  
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxCosDsaMappingSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_U32                      dsaIndex,
    IN  CPSS_PX_COS_ATTRIBUTES_STC  *cosAttributesPtr
);
*/ 
UTF_TEST_CASE_MAC(cpssPxCosDsaMappingSet)
{
/*
    ITERATE_DEVICE (Go over all Px devices)
    1.1. Call with dsaIndex[0..127], with valid cosAttributesPtr.
    Expected: GT_OK.
    1.2. Call with out of range dsaIndex[128] and other params from 1.1.
    Expected: GT_OUT_OF_RANGE.
    1.3. Call with cosAttributesPtr->trafficClass == 10.
    Expected: GT_BAD_PARAM.
    1.4. Call with cosAttributesPtr == NULL and other params from 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_U32      dsaIndex;

    CPSS_PX_COS_ATTRIBUTES_STC    cosAttributes;
    CPSS_PX_COS_ATTRIBUTES_STC    cosAttributesGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    { 
        /*  1.1. Call with dsaIndex[0..127], with valid cosFormatEntryPtr.
            Expected: GT_OK. */
        for (dsaIndex = 0; dsaIndex < BIT_7; dsaIndex++)
        {
            cosAttributes.trafficClass = dsaIndex % 8;
            cosAttributes.dropPrecedence = (CPSS_DP_LEVEL_ENT) (dsaIndex % CPSS_DP_LAST_E);
            cosAttributes.userPriority = (dsaIndex * 3) % 8;
            cosAttributes.dropEligibilityIndication = dsaIndex % 2;
            st = cpssPxCosDsaMappingSet(dev, dsaIndex, &cosAttributes);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dsaIndex);

            st = cpssPxCosDsaMappingGet(dev, dsaIndex, &cosAttributesGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dsaIndex); 

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(cosAttributes.trafficClass,
                                        cosAttributesGet.trafficClass,
                       "get another trafficClass than was set: %d", dsaIndex); 
            UTF_VERIFY_EQUAL1_STRING_MAC(cosAttributes.dropPrecedence,
                                        cosAttributesGet.dropPrecedence,
                       "get another dropPrecedence than was set: %d", dsaIndex); 
            UTF_VERIFY_EQUAL1_STRING_MAC(cosAttributes.userPriority,
                                        cosAttributesGet.userPriority,
                       "get another userPriority than was set: %d", dsaIndex); 
            UTF_VERIFY_EQUAL1_STRING_MAC(cosAttributes.dropEligibilityIndication,
                                        cosAttributesGet.dropEligibilityIndication,
                       "get another dropEligibilityIndication than was set: %d", dsaIndex); 
        }
        /*  1.2. Call with out of range dsaIndex[128] and other params from 1.1.
            Expected: GT_OUT_OF_RANGE. */
        st = cpssPxCosDsaMappingSet(dev, dsaIndex, &cosAttributes);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dsaIndex);

        /*  1.3. Call with cosAttributesPtr->trafficClass == 10.
            Expected: GT_BAD_PARAM. */
        cosAttributesGet = cosAttributes;
        cosAttributesGet.trafficClass = 10;
        st = cpssPxCosDsaMappingSet(dev, 0, &cosAttributesGet);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);

        /*  1.4. Call with cosAttributesPtr == NULL and other params from 1.1.
            Expected: GT_BAD_PTR. */
        st = cpssPxCosDsaMappingSet(dev, 0, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxCosDsaMappingSet(dev, 0, &cosAttributes);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    } 
 
    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxCosDsaMappingSet(dev, 0, &cosAttributes);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);  
}

/*
GT_STATUS cpssPxCosDsaMappingGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_U32                      dsaIndex,
    OUT CPSS_PX_COS_ATTRIBUTES_STC  *cosAttributesPtr
);
*/
UTF_TEST_CASE_MAC(cpssPxCosDsaMappingGet)
{
/*
    ITERATE_DEVICE (Go over all Px devices)
    1.1. Call with dsaIndex[0..127], with valid cosAttributesPtr.
    Expected: GT_OK.
    1.2. Call with out of range dsaIndex[128].
    Expected: GT_OUT_OF_RANGE.
    1.3. Call with cosAttributesPtr == NULL.
    Expected: GT_BAD_PTR.

*/

    GT_STATUS   st     = GT_OK;
    GT_U8       dev;
    GT_U32      dsaIndex;

    CPSS_PX_COS_ATTRIBUTES_STC    cosAttributesGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1. Call with dsaIndex[0..127], with valid cosAttributesPtr.
            Expected: GT_OK. */
        for (dsaIndex = 0; dsaIndex < BIT_7; dsaIndex++)
        {
            st = cpssPxCosDsaMappingGet(dev, dsaIndex, &cosAttributesGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dsaIndex);
        }

        /*  1.2. Call with out of range dsaIndex[128].
            Expected: GT_OUT_OF_RANGE. */
        st = cpssPxCosDsaMappingGet(dev, dsaIndex, &cosAttributesGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dsaIndex); 

        /*  1.3. Call with cosAttributesPtr == NULL.
            Expected: GT_BAD_PTR. */
        st = cpssPxCosDsaMappingGet(dev, 0, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st); 
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxCosDsaMappingGet(dev, 0, &cosAttributesGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    } 

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxCosDsaMappingGet(dev, 0, &cosAttributesGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);  
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssPxIngress suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssPxCos)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxCosFormatEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxCosFormatEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxCosPortAttributesSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxCosPortAttributesGet) 
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxCosPortL2MappingSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxCosPortL2MappingGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxCosL3MappingSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxCosL3MappingGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxCosMplsMappingSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxCosMplsMappingGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxCosDsaMappingSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxCosDsaMappingGet)
UTF_SUIT_END_TESTS_MAC(cpssPxCos)


