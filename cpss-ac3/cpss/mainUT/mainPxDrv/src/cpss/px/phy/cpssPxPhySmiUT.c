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
* @file cpssPxPhySmiUT.c
*
* @brief Unit tests for cpssPxPhySmi APIs.
*
* @version   1
********************************************************************************
*/
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/px/phy/cpssPxPhySmi.h>
#include <cpss/common/smi/cpssGenSmi.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

#define PHY_SMI_VALID_PHY_PORT_CNS  0

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPhySmiMdcDivisionFactorGet
(
    IN  GT_U8                                 dev,
    OUT CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT  *divisionFactorPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPhySmiMdcDivisionFactorGet) 
{ 
/*
    ITERATE_DEVICES_PHY_PORTS(Pipe)
    1.1. Call with not null divisionFactor pointer.
    Expected: GT_OK.
    1.2. Call api with wrong divisionFactorPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK; 
    
    GT_U8                                 dev; 
    CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT  divisionFactor; 
    
    /* there is no SMI/XSMI in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC; 
    
    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE)) 
    {
        /*
            1.1. Call with not null divisionFactor pointer.
            Expected: GT_OK.
        */
        
        st = cpssPxPhySmiMdcDivisionFactorGet(dev, &divisionFactor); 
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "dev = %d", dev);
        
        /*
            1.2. Call api with wrong divisionFactorPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPhySmiMdcDivisionFactorGet(dev, NULL); 
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, divisionFactorPtr = NULL", dev);
    }
    
    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    
    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE)) 
    {
        st = cpssPxPhySmiMdcDivisionFactorGet(dev, &divisionFactor); 
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }
    
    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS; 
    
    st = cpssPxPhySmiMdcDivisionFactorGet(dev, &divisionFactor); 
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPhySmiMdcDivisionFactorSet
(
    IN  GT_U8                                dev,
    IN  CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT divisionFactor
)
*/
UTF_TEST_CASE_MAC(cpssPxPhySmiMdcDivisionFactorSet) 
{ 
/*
    ITERATE_DEVICES_PHY_PORTS(PxXcat and above)
    1.1. Call with divisionFactor[CPSS_PHY_SMI_MDC_DIVISION_FACTOR_16_E /
                                  CPSS_PHY_SMI_MDC_DIVISION_FACTOR_32_E /
                                  CPSS_PHY_SMI_MDC_DIVISION_FACTOR_64_E /
                                  CPSS_PHY_SMI_MDC_DIVISION_FACTOR_128_E /
                                  CPSS_PHY_SMI_MDC_DIVISION_FACTOR_256_E],
    Expected: GT_OK.
    1.2. Call cpssPxPhySmiMdcDivisionFactorGet
           with the same parameters.
    Expected: GT_OK and the same values than was set.
    1.3. Call api with wrong divisionFactor [wrong enum values].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st = GT_OK; 
    
    GT_U8                                dev; 
    CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT divisionFactor = CPSS_PHY_SMI_MDC_DIVISION_FACTOR_16_E; 
    CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT divisionFactorGet = CPSS_PHY_SMI_MDC_DIVISION_FACTOR_16_E; 
    
    /* there is no SMI/XSMI in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC; 
    
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
 
    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE)) 
    {
        /*
            1.1. Call with divisionFactor[CPSS_PHY_SMI_MDC_DIVISION_FACTOR_16_E /
                                          CPSS_PHY_SMI_MDC_DIVISION_FACTOR_32_E /
                                          CPSS_PHY_SMI_MDC_DIVISION_FACTOR_64_E /
                                          CPSS_PHY_SMI_MDC_DIVISION_FACTOR_128_E /
                                          CPSS_PHY_SMI_MDC_DIVISION_FACTOR_256_E],
            Expected: GT_OK.
        */
        
        for (divisionFactor = CPSS_PHY_SMI_MDC_DIVISION_FACTOR_16_E; 
              divisionFactor <= CPSS_PHY_SMI_MDC_DIVISION_FACTOR_256_E; 
              divisionFactor++)
        {
            st = cpssPxPhySmiMdcDivisionFactorSet(dev, divisionFactor); 
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, 
                                         "cpssPxPhySmiMdcDivisionFactorSet: %d ", dev);
            /*
                1.2. Call cpssPxPhySmiMdcDivisionFactorGet
                       with the same parameters.
                Expected: GT_OK and the same values than was set.
            */
            st = cpssPxPhySmiMdcDivisionFactorGet(dev, &divisionFactorGet); 
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, 
                                         "cpssPxPhySmiMdcDivisionFactorGet: %d ", dev); 
            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(divisionFactor, divisionFactorGet, 
                                         "got another divisionFactor then was set: %d", dev);
        }
        

        /*
            1.3. Call api with wrong divisionFactor [wrong enum values].
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssPxPhySmiMdcDivisionFactorSet
                           (dev, divisionFactor), divisionFactor);
    }
    
    /* restore correct values */
    divisionFactor = CPSS_PHY_SMI_MDC_DIVISION_FACTOR_16_E; 
    
    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    
    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE)) 
    {
        st = cpssPxPhySmiMdcDivisionFactorSet(dev, divisionFactor); 
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }
    
    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS; 
    
    st = cpssPxPhySmiMdcDivisionFactorSet(dev, divisionFactor); 
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPhyXsmiMdcDivisionFactorGet
(
    IN  GT_U8                                 dev,
    OUT CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT  *divisionFactorPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPhyXsmiMdcDivisionFactorGet) 
{ 
/*
    ITERATE_DEVICES(Pipe)
    1.1. Call with not null divisionFactorPtr.
    Expected: GT_OK.
    1.2. Call api with wrong divisionFactorPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK; 
    
    GT_U8                                 dev; 
    CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT  divisionFactor; 
    
    /* there is no SMI/XSMI in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC; 
    
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    
    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE)) 
    {
        /*
            1.1. Call with not null divisionFactorPtr.
            Expected: GT_OK.
        */
        st = cpssPxPhyXsmiMdcDivisionFactorGet(dev, &divisionFactor); 
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev); 
        
        /*
            1.2. Call api with wrong divisionFactorPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPhyXsmiMdcDivisionFactorGet(dev, NULL); 
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, 
                                     "%d, divisionFactorPtr = NULL", dev);
    }
    
    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    
    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE)) 
    {
        st = cpssPxPhyXsmiMdcDivisionFactorGet(dev, &divisionFactor); 
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }
    
    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS; 
    
    st = cpssPxPhyXsmiMdcDivisionFactorGet(dev, &divisionFactor); 
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPhyXsmiMdcDivisionFactorSet
(
    IN  GT_U8                                dev,
    IN  CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT divisionFactor
)
*/
UTF_TEST_CASE_MAC(cpssPxPhyXsmiMdcDivisionFactorSet) 
{ 
/*
    ITERATE_DEVICES(PxXcat and above)
    1.1 call cpssPxPhyXsmiMdcDivisionFactorSet for every divisionFactor in
            CPSS_PHY_SMI_MDC_DIVISION_FACTOR_8_E
            CPSS_PHY_SMI_MDC_DIVISION_FACTOR_32_E
            CPSS_PHY_SMI_MDC_DIVISION_FACTOR_64_E
            CPSS_PHY_SMI_MDC_DIVISION_FACTOR_256_E],
    Expected: GT_OK

    1.2. if GT_OK, call cpssPxPhyXsmiMdcDivisionFactorGet
           with the same parameters.
    Expected: GT_OK and the same values than was set.

    1.3. Call api with wrong divisionFactor [wrong enum values].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st = GT_OK; 
    
    GT_U8                                dev; 
    CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT divisionFactor; 
    CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT divisionFactorGet; 
    GT_U32 divisionFactorsArr[] = {
        CPSS_PHY_SMI_MDC_DIVISION_FACTOR_8_E,
        CPSS_PHY_SMI_MDC_DIVISION_FACTOR_32_E,
        CPSS_PHY_SMI_MDC_DIVISION_FACTOR_64_E,
        CPSS_PHY_SMI_MDC_DIVISION_FACTOR_256_E
    };
    GT_U32 divisionFactorsArrSize = sizeof(divisionFactorsArr) / sizeof(GT_U32);
    GT_U32 i;

    /* there is no SMI/XSMI in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC; 
    
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    
    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE)) 
    {
        /*
            1.1. call cpssPxPhyXsmiMdcDivisionFactorSet with all valid division factors
        */
      
        for (i = 0; i < divisionFactorsArrSize; i++) 
        {
            divisionFactor = divisionFactorsArr[i];

            /* check if this divisionFactor is applicable for the device */
            st = cpssPxPhyXsmiMdcDivisionFactorSet(dev, divisionFactor); 
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, divisionFactor); 
            
            /*
              1.2. Call cpssPxPhyXsmiMdcDivisionFactorGet
              with the same parameters.
              Expected: GT_OK and the same values than was set.
            */
            st = cpssPxPhyXsmiMdcDivisionFactorGet(dev, &divisionFactorGet); 
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, 
                                         "cpssPxPhyXsmiMdcDivisionFactorGet: %d, %d", 
                                         dev, divisionFactor); 
            UTF_VERIFY_EQUAL2_STRING_MAC(divisionFactor, divisionFactorGet, 
                                         "got another divisionFactor then was set: %d, %d", 
                                         dev, divisionFactor);
        }
    }
    
    /* 2. For not-active devices and devices from non-applicable devFamily */
    /* check that function returns GT_BAD_PARAM.                        */
    
    divisionFactor = CPSS_PHY_SMI_MDC_DIVISION_FACTOR_8_E;
    
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    
    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE)) 
    {
        st = cpssPxPhyXsmiMdcDivisionFactorSet(dev, divisionFactor); 
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }
    
    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS; 
    
    st = cpssPxPhyXsmiMdcDivisionFactorSet(dev, divisionFactor); 
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
 * Configuration of cppPxPhySmi suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssPxPhySmi)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPhySmiMdcDivisionFactorGet) 
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPhySmiMdcDivisionFactorSet) 
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPhyXsmiMdcDivisionFactorGet) 
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPhyXsmiMdcDivisionFactorSet)
UTF_SUIT_END_TESTS_MAC(cpssPxPhySmi)

