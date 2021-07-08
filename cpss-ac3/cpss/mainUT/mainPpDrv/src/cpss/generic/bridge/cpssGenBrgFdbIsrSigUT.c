/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* cpssExMxPmBrgFdbUT.c
*
* DESCRIPTION:
*       Unit tests for cpssGenBrgFdb, that provides
*       ISR and FDB signalling mechanism create/signal/wait/delete API
*
* FILE REVISION NUMBER:
*       $Revision: 1.18 $
*******************************************************************************/

/* includes */
#include <cpss/generic/bridge/private/prvCpssGenBrgFdbIsrSig.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>


/* defines */

/* Invalid enum */
#define BRG_FDB_INVALID_ENUM_CNS    0x5AAAAAA5


/*----------------------------------------------------------------------------*/
/*
GT_STATUS prvCpssFdbIsrSigCreate
(
    IN CPSS_FDB_ISR_SIG_ENT       sigType
)
*/
UTF_TEST_CASE_MAC(prvCpssFdbIsrSigCreate)
{
/*
    1. Call with sigType [CPSS_QUERY_E /
                          CPSS_MAC_TRIG_ACTION_E].
    Expected: GT_OK.
    2. Call with out of range sigType [0x5AAAAAA5].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;

    CPSS_FDB_ISR_SIG_ENT    sigType = CPSS_QUERY_E;


    /*
        1. Call with sigType [CPSS_QUERY_E /
                              CPSS_MAC_TRIG_ACTION_E].
        Expected: GT_OK.
    */

    /* Call with sigType = CPSS_QUERY_E */
    sigType = CPSS_QUERY_E;

    st = prvCpssFdbIsrSigCreate(sigType);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, sigType);

    /* Call with sigType = CPSS_MAC_TRIG_ACTION_E */
    sigType = CPSS_MAC_TRIG_ACTION_E;

    st = prvCpssFdbIsrSigCreate(sigType);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, sigType);

    /*
        2. Call with out of range sigType [0x5AAAAAA5].
        Expected: GT_BAD_PARAM.
    */
    sigType = BRG_FDB_INVALID_ENUM_CNS;

    st = prvCpssFdbIsrSigCreate(sigType);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, sigType);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS prvCpssFdbIsrSigSignal
(
    IN CPSS_FDB_ISR_SIG_ENT       sigType
)
*/
UTF_TEST_CASE_MAC(prvCpssFdbIsrSigSignal)
{
/*
    1. Call with sigType [CPSS_QUERY_E /
                          CPSS_MAC_TRIG_ACTION_E].
    Expected: GT_OK.
    2. Call with out of range sigType [0x5AAAAAA5].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;

    CPSS_FDB_ISR_SIG_ENT    sigType = CPSS_QUERY_E;


    /*
        1. Call with sigType [CPSS_QUERY_E /
                              CPSS_MAC_TRIG_ACTION_E].
        Expected: GT_OK.
    */

    /* Call with sigType = CPSS_QUERY_E */
    sigType = CPSS_QUERY_E;

    st = prvCpssFdbIsrSigSignal(sigType);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, sigType);

    /* Call with sigType = CPSS_MAC_TRIG_ACTION_E */
    sigType = CPSS_MAC_TRIG_ACTION_E;

    st = prvCpssFdbIsrSigSignal(sigType);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, sigType);

    /*
        2. Call with out of range sigType [0x5AAAAAA5].
        Expected: GT_BAD_PARAM.
    */
    sigType = BRG_FDB_INVALID_ENUM_CNS;

    st = prvCpssFdbIsrSigSignal(sigType);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, sigType);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS prvCpssFdbIsrSigWait
(
    IN CPSS_FDB_ISR_SIG_ENT sigType,
    IN GT_U32               timeout
)
*/
UTF_TEST_CASE_MAC(prvCpssFdbIsrSigWait)
{
/*
    1. Call with sigType [CPSS_QUERY_E /
                          CPSS_MAC_TRIG_ACTION_E],
                 timeout [1 / 0xFFFF].
    Expected: GT_OK.
    2. Call with out of range sigType [0x5AAAAAA5]
                 and other parameters from 1.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;

    CPSS_FDB_ISR_SIG_ENT    sigType = CPSS_QUERY_E;
    GT_U32                  timeout = 0;


    /*
        1. Call with sigType [CPSS_QUERY_E /
                              CPSS_MAC_TRIG_ACTION_E].
        Expected: GT_OK.
    */

    /* Call with sigType = CPSS_QUERY_E */
    sigType = CPSS_QUERY_E;
    timeout = 1;

    st = prvCpssFdbIsrSigWait(sigType, timeout);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, sigType, timeout);

    /* Call with sigType = CPSS_MAC_TRIG_ACTION_E */
    sigType = CPSS_MAC_TRIG_ACTION_E;
    timeout = 0xFFFF;

    st = prvCpssFdbIsrSigWait(sigType, timeout);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, sigType, timeout);

    /*
        2. Call with out of range sigType [0x5AAAAAA5]
                     and other parameters from 1.
        Expected: GT_BAD_PARAM.
    */
    sigType = BRG_FDB_INVALID_ENUM_CNS;

    st = prvCpssFdbIsrSigWait(sigType, timeout);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, sigType);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS prvCpssFdbIsrSigUnmaskDevice
(
    IN GT_U8                      devNum,
    IN CPSS_FDB_ISR_SIG_ENT       sigType
)
*/
UTF_TEST_CASE_MAC(prvCpssFdbIsrSigUnmaskDevice)
{
/*
    ITERATE_DEVICES
    1.1. Call with sigType [CPSS_QUERY_E /
                            CPSS_MAC_TRIG_ACTION_E].
    Expected: GT_OK.
    1.2. Call with out of range sigType [0x5AAAAAA5].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_FDB_ISR_SIG_ENT    sigType = CPSS_QUERY_E;


    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_ALL_FAMILIES_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with sigType [CPSS_QUERY_E /
                                    CPSS_MAC_TRIG_ACTION_E].
            Expected: GT_OK.
        */

        /* Call with sigType = CPSS_QUERY_E */
        sigType = CPSS_QUERY_E;
    
        st = prvCpssFdbIsrSigUnmaskDevice(dev, sigType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, sigType);
    
        /* Call with sigType = CPSS_MAC_TRIG_ACTION_E */
        sigType = CPSS_MAC_TRIG_ACTION_E;
        
        st = prvCpssFdbIsrSigUnmaskDevice(dev, sigType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, sigType);
    
        /*
            1.2. Call with out of range sigType [0x5AAAAAA5].
            Expected: GT_BAD_PARAM.
        */
        sigType = BRG_FDB_INVALID_ENUM_CNS;
    
        st = prvCpssFdbIsrSigUnmaskDevice(dev, sigType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, sigType);
    }

    sigType = CPSS_QUERY_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_ALL_FAMILIES_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = prvCpssFdbIsrSigUnmaskDevice(dev, sigType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = prvCpssFdbIsrSigUnmaskDevice(dev, sigType);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

/*
 * Configuration of cpssGenBrgFdb suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssGenBrgFdb)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssFdbIsrSigCreate)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssFdbIsrSigSignal)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssFdbIsrSigWait)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssFdbIsrSigUnmaskDevice)
UTF_SUIT_END_TESTS_MAC(cpssGenBrgFdb)

