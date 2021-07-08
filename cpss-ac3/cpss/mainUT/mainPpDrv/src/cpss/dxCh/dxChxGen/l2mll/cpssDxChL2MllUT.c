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
* @file cpssDxChL2MllUT.c
*
* @brief Unit tests for CPSS DXCH L2 MLL APIs
*
* @version   17
********************************************************************************
*/
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files
   that already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/dxCh/dxChxGen/cpssDxChTypes.h>
#include <cpss/dxCh/dxChxGen/l2mll/cpssDxChL2Mll.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/generic/bridge/private/prvCpssBrgVlanTypes.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChL2MllCounterGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  index,
    OUT GT_U32  *counterPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChL2MllCounterGet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with index[0, 1, 2], non-null counter.
    Expected: GT_OK.
    1.2. Call with out of range index[3] and other valid params.
    Expected: NON GT_OK.
    1.3. Call with out of range counterPtr and other valid params.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS       st          = GT_OK;
    GT_U8           dev         = 0;
    GT_U32          index       = 0;
    GT_U32          counter     = 0;
    GT_STATUS   expectedSt;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_FALSE && PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == GT_FALSE)
        {
            expectedSt = GT_NOT_APPLICABLE_DEVICE;
        }
        else
        {
            expectedSt = GT_OK;
        }

        /*
            1.1. Call with index[0, 1, 2], non-null counter.
            Expected: GT_OK.
        */

        /* call with index[0] */
        index = 0;

        st = cpssDxChL2MllCounterGet(dev, index, &counter);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        /* call with index[1] */
        index = 1;

        st = cpssDxChL2MllCounterGet(dev, index, &counter);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        /* call with index[2] */
        index = 2;

        st = cpssDxChL2MllCounterGet(dev, index, &counter);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        /*
            1.2. Call with out of range index[3] and other valid params.
            Expected: NON GT_OK.
        */
        index = 3;

        st = cpssDxChL2MllCounterGet(dev, index, &counter);
        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }


        /* restore value */
        index = 2;

        /*
            1.3. Call with out of range counterPtr[NULL] and
                other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChL2MllCounterGet(dev, index, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* restore value */
    index = 2;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChL2MllCounterGet(dev, index, &counter);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChL2MllCounterGet(dev, index, &counter);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChL2MllExceptionCountersGet
(
    IN  GT_U8                                   devNum,
    OUT CPSS_DXCH_L2_MLL_EXCEPTION_COUNTERS_STC  *countersPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChL2MllExceptionCountersGet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with non-null countersPtr.
    Expected: GT_OK.
    1.2. Call with out of range countersPtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                               st        = GT_OK;
    GT_U8                                   dev       = 0;
    CPSS_DXCH_L2_MLL_EXCEPTION_COUNTERS_STC counters  = {0};
    GT_STATUS   expectedSt;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_FALSE && PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == GT_FALSE)
        {
            expectedSt = GT_NOT_APPLICABLE_DEVICE;
        }
        else
        {
            expectedSt = GT_OK;
        }

        /*
            1.1. Call with non-null countersPtr.
            Expected: GT_OK.
        */
        st = cpssDxChL2MllExceptionCountersGet(dev, &counters);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        /*
            1.2. Call with out of range countersPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChL2MllExceptionCountersGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                         "cpssDxChL2MllExceptionCountersGet: %d",
                                         dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChL2MllExceptionCountersGet(dev, &counters);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChL2MllExceptionCountersGet(dev, &counters);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


static void compareL2Mll(
    IN GT_U8                         dev,
    IN CPSS_DXCH_L2_MLL_ENTRY_STC   *mllEntryPtr,
    IN CPSS_DXCH_L2_MLL_ENTRY_STC   *mllEntryRetPtr
)
{

    UTF_VERIFY_EQUAL1_STRING_MAC(mllEntryPtr->last,
                                 mllEntryRetPtr->last,
                                 "got another mllEntryPtr->last: %d",
                                 mllEntryRetPtr->last);
    UTF_VERIFY_EQUAL1_STRING_MAC(mllEntryPtr->unknownUcFilterEnable,
                                 mllEntryRetPtr->unknownUcFilterEnable,
                                 "got another mllEntryPtr->unknownUcFilterEnable: %d",
                                 mllEntryRetPtr->unknownUcFilterEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(mllEntryPtr->unregMcFilterEnable,
                                 mllEntryRetPtr->unregMcFilterEnable,
                                 "got another mllEntryPtr->unregMcFilterEnable: %d",
                                 mllEntryRetPtr->unregMcFilterEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(mllEntryPtr->bcFilterEnable,
                                 mllEntryRetPtr->bcFilterEnable,
                                 "got another mllEntryPtr->bcFilterEnable: %d",
                                 mllEntryRetPtr->bcFilterEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(mllEntryPtr->mcLocalSwitchingEnable,
                                 mllEntryRetPtr->mcLocalSwitchingEnable,
                                 "got another mllEntryPtr->mcLocalSwitchingEnable: %d",
                                 mllEntryRetPtr->mcLocalSwitchingEnable);
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == GT_TRUE)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(mllEntryPtr->maxHopCountEnable,
                                     mllEntryRetPtr->maxHopCountEnable,
                                     "got another mllEntryPtr->maxHopCountEnable: %d",
                                     mllEntryRetPtr->maxHopCountEnable);
        if(mllEntryPtr->maxHopCountEnable)
        {

            UTF_VERIFY_EQUAL1_STRING_MAC(mllEntryPtr->maxOutgoingHopCount,
                                         mllEntryRetPtr->maxOutgoingHopCount,
                                         "got another mllEntryPtr->maxOutgoingHopCount: %d",
                                         mllEntryRetPtr->maxOutgoingHopCount);

        }
    }

    UTF_VERIFY_EQUAL1_STRING_MAC(mllEntryPtr->egressInterface.type,
                                 mllEntryRetPtr->egressInterface.type,
                                 "got another mllEntryPtr->egressInterface.type: %d",
                                 mllEntryRetPtr->egressInterface.type);

    switch(mllEntryPtr->egressInterface.type)
    {
        case CPSS_INTERFACE_PORT_E:
            UTF_VERIFY_EQUAL1_STRING_MAC(mllEntryPtr->egressInterface.devPort.portNum,
                                         mllEntryRetPtr->egressInterface.devPort.portNum,
                                         "got another mllEntryPtr->egressInterface.devPort.portNum: %d",
                                         mllEntryRetPtr->egressInterface.devPort.portNum);
            UTF_VERIFY_EQUAL1_STRING_MAC(mllEntryPtr->egressInterface.devPort.hwDevNum,
                                         mllEntryRetPtr->egressInterface.devPort.hwDevNum,
                                         "got another mllEntryPtr->egressInterface.devPort.hwDevNum: %d",
                                         mllEntryRetPtr->egressInterface.devPort.hwDevNum);
            break;
        case CPSS_INTERFACE_TRUNK_E:
            UTF_VERIFY_EQUAL1_STRING_MAC(mllEntryPtr->egressInterface.trunkId,
                                         mllEntryRetPtr->egressInterface.trunkId,
                                         "got another mllEntryPtr->egressInterface.trunkId: %d",
                                         mllEntryRetPtr->egressInterface.trunkId);
            break;
        case CPSS_INTERFACE_VIDX_E:
            UTF_VERIFY_EQUAL1_STRING_MAC(mllEntryPtr->egressInterface.vidx,
                                         mllEntryRetPtr->egressInterface.vidx,
                                         "got another mllEntryPtr->egressInterface.vidx: %d",
                                         mllEntryRetPtr->egressInterface.vidx);
            break;
        case CPSS_INTERFACE_VID_E:
            /* nothing important*/
            break;
        default:
            break;
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == GT_TRUE)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(mllEntryPtr->maskBitmap,
                                     mllEntryRetPtr->maskBitmap,
                                     "got another mllEntryPtr->maskBitmap: %d",
                                     mllEntryRetPtr->maskBitmap);
        UTF_VERIFY_EQUAL1_STRING_MAC(mllEntryPtr->ttlThreshold,
                                     mllEntryRetPtr->ttlThreshold,
                                     "got another mllEntryPtr->ttlThreshold: %d",
                                     mllEntryRetPtr->ttlThreshold);
        UTF_VERIFY_EQUAL1_STRING_MAC(mllEntryPtr->bindToMllCounterEnable,
                                     mllEntryRetPtr->bindToMllCounterEnable,
                                     "got another mllEntryPtr->bindToMllCounterEnable: %d",
                                     mllEntryRetPtr->bindToMllCounterEnable);
        if(mllEntryPtr->bindToMllCounterEnable == GT_TRUE)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(mllEntryPtr->mllCounterIndex,
                                         mllEntryRetPtr->mllCounterIndex,
                                         "got another mllEntryPtr->mllCounterIndex: %d",
                                         mllEntryRetPtr->mllCounterIndex);
        }
    }

    UTF_VERIFY_EQUAL1_STRING_MAC(mllEntryPtr->onePlusOneFilteringEnable,
                                 mllEntryRetPtr->onePlusOneFilteringEnable,
                                 "got another mllEntryPtr->onePlusOneFilteringEnable: %d",
                                 mllEntryRetPtr->onePlusOneFilteringEnable);

    UTF_VERIFY_EQUAL1_STRING_MAC(mllEntryPtr->meshId,
                                 mllEntryRetPtr->meshId,
                                 "got another mllEntryPtr->meshId: %d",
                                 mllEntryRetPtr->meshId);
}

static void compareL2MllPair(
    IN GT_U8                                        dev,
    IN CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT           mllPairWriteForm,
    IN CPSS_DXCH_L2_MLL_PAIR_STC                    *mllPairEntryPtr,
    IN CPSS_DXCH_L2_MLL_PAIR_STC                    *mllPairEntryRetPtr
)
{
    if((mllPairWriteForm==CPSS_DXCH_PAIR_READ_WRITE_FIRST_ONLY_E)||
       (mllPairWriteForm==CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E))
    {
        compareL2Mll(dev, &(mllPairEntryPtr->firstMllNode), &(mllPairEntryRetPtr->firstMllNode));
    }
    if((mllPairWriteForm==CPSS_DXCH_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E)||
       (mllPairWriteForm==CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E))
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntryPtr->nextPointer,
                                 mllPairEntryPtr->nextPointer,
                                 "got another mllPairEntryPtr->nextPointer: %d",
                                 mllPairEntryPtr->nextPointer);

        UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntryPtr->entrySelector,
                                 mllPairEntryPtr->entrySelector,
                                 "got another mllPairEntryPtr->entrySelector: %d",
                                 mllPairEntryPtr->entrySelector);

        compareL2Mll(dev, &(mllPairEntryPtr->secondMllNode), &(mllPairEntryRetPtr->secondMllNode));
    }

    return;

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChL2MllPairWrite
(
    IN GT_U8                                        devNum,
    IN GT_U32                                       mllPairEntryIndex,
    IN CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT           mllPairWriteForm,
    IN CPSS_DXCH_L2_MLL_PAIR_STC                    *mllPairEntryPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChL2MllPairWrite)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with index[0, max/2, max],
                   mllEntry{
                        nextMllPointer[0, max/2, max],
                        last[GT_TRUE, GT_FALSE, GT_TRUE],
                        unknownUcFilterEnable[GT_FALSE, GT_TRUE, GT_TRUE],
                        unregMcFilterEnable[GT_FALSE, GT_TRUE, GT_TRUE],
                        bcFilterEnable[GT_TRUE, GT_FALSE, GT_FALSE],
                        mcLocalSwitchingEnable[GT_TRUE, GT_FALSE, GT_TRUE],
                        maxHopCountEnable[GT_FALSE, GT_TRUE, GT_TRUE],
                        maxOutgoingHopCount[0, 55, 63],
                        egressInterface{
                            type[CPSS_INTERFACE_PORT_E,
                                 CPSS_INTERFACE_TRUNK_E,
                                 CPSS_INTERFACE_VIDX_E],
                            devPort{
                                devNum[devNum],
                                portNum[0]
                            }
                            trunkId[1],
                            vidx[10],
                            vlanId[100],
                            devNum[0],
                            fabricVidx[0],
                            index[0]
                        }
                        maskBitmap[0, 0x3FF, 0x4000],
                        ttlThreshold[0, 0xA5, 0xFF],
                        bindToMllCounterEnable[GT_FALSE, GT_TRUE, GT_TRUE],
                        mllCounterIndex[0, 1, 2]
                        onePlusOneFilteringEnable[GT_TRUE,GT_FALSE,GT_FALSE]
                        meshId[0, 0xA5/2, 255/3],
                   }
    Expected: GT_OK.

    1.2. Call cpssDxChL2MllPairRead.
    Expected: GT_OK and the same values as was set.

    1.3. Call with out of range index[max+1] and other valid params.
    Expected: NON GT_OK.

    1.4. Call with out of range mllPairEntry.firstMllNode.nextMllPointer[max+1] and other valid params.
    Expected: NON GT_OK.

    1.5. Call with mllPairEntry.firstMllNode.maxHopCountEnable[GT_TRUE], out of range
        mllPairEntry.firstMllNode.maxOutgoingHopCount[BIT_6] (is relevant) and other valid params.
    Expected: NON GT_OK.

    1.6. Call with mllPairEntry.firstMllNode.maxHopCountEnable[GT_FALSE], out of range
        mllPairEntry.firstMllNode.maxOutgoingHopCount[BIT_6] (not relevant) and other valid params.
    Expected: GT_OK.

    1.7. Call with out of range mllPairEntry.firstMllNode.egressInterface.type and other valid params.
    Expected: GT_BAD_PARAM.

    1.8. Call with mllPairEntry.firstMllNode.egressInterface.type[CPSS_INTERFACE_PORT_E], out of range
        mllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum[UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS+1] (is relevant)
        and other valid params.
    Expected: NON GT_OK.

    1.9. Call with mllPairEntry.firstMllNode.egressInterface.type[CPSS_INTERFACE_TRUNK_E], out of range
        mllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum[UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS+1] (not relevant)
        and other valid params.
    Expected: GT_OK.

    1.10. Call with mllPairEntry.firstMllNode.egressInterface.type[CPSS_INTERFACE_PORT_E], out of range
        mllPairEntry.firstMllNode.egressInterface.devPort.portNum[UTF_CPSS_PP_MAX_PORTS_NUM_CNS+1] (is relevant)
        and other valid params.
    Expected: NON GT_OK.

    1.11. Call with mllPairEntry.firstMllNode.egressInterface.type[CPSS_INTERFACE_TRUNK_E], out of range
        mllPairEntry.firstMllNode.egressInterface.devPort.portNum[UTF_CPSS_PP_MAX_PORTS_NUM_CNS+1] (not relevant)
        and other valid params.
    Expected: GT_OK.

    1.12. Call with mllPairEntry.firstMllNode.egressInterface.type[CPSS_INTERFACE_TRUNK_E], out of range
        mllPairEntry.firstMllNode.egressInterface.trunkId[UTF_CPSS_PP_MAX_TRUNK_ID_MAC+1] (is relevant)
        and other valid params.
    Expected: NON GT_OK.

    1.13. Call with mllPairEntry.firstMllNode.egressInterface.type[CPSS_INTERFACE_PORT_E], out of range
        mllPairEntry.firstMllNode.egressInterface.trunkId[UTF_CPSS_PP_MAX_TRUNK_ID_MAC+1] (not relevant)
        and other valid params.
    Expected: GT_OK.

    1.14. Call with mllPairEntry.firstMllNode.egressInterface.type[CPSS_INTERFACE_VIDX_E], out of range
        mllPairEntry.firstMllNode.egressInterface.vidx[4096] (is relevant) and other valid params.
    Expected: NON GT_OK.

    1.15. Call with mllPairEntry.firstMllNode.egressInterface.type[CPSS_INTERFACE_PORT_E], out of range
        mllPairEntry.firstMllNode.egressInterface.vidx[4096] (not relevant) and other valid params.
    Expected: GT_OK.

    1.16. Call with mllPairEntry.firstMllNode.egressInterface.type[CPSS_INTERFACE_VID_E], out of range
        mllPairEntry.firstMllNode.egressInterface.vlanId[UTF_CPSS_PP_MAX_VLAN_NUM_CNS] (is relevant) and
        other valid params.
    Expected: NON GT_OK.

    1.17. Call with mllPairEntry.firstMllNode.egressInterface.type[CPSS_INTERFACE_PORT_E], out of range
        mllPairEntry.firstMllNode.egressInterface.vlanId[UTF_CPSS_PP_MAX_VLAN_NUM_CNS] (not relevant) and
        other valid params.
    Expected: GT_OK.

    1.18. Call with mllPairEntry.firstMllNode.egressInterface.type[CPSS_INTERFACE_PORT_E], out of range
        mllPairEntry.firstMllNode.egressInterface.hwDevNum[0xFFFFFFFF] (not relevant) and other valid params.
    Expected: GT_OK.

    1.19. Call with mllPairEntry.firstMllNode.egressInterface.type[CPSS_INTERFACE_PORT_E], out of range
        mllPairEntry.firstMllNode.egressInterface.fabricVidx[0xFFFF] (not relevant) and other valid params.
    Expected: GT_OK.

    1.20. Call with mllPairEntry.firstMllNode.egressInterface.type[CPSS_INTERFACE_PORT_E], out of range
        mllPairEntry.firstMllNode.egressInterface.index[0xFFFFFFFF] (not relevant) and other valid params.
    Expected: GT_OK.

    1.21. Call with out of range mllPairEntry.firstMllNode.maskBitmap[BIT_15] and other valid params.
    Expected: NON GT_OK.

    1.22. Call with out of range mllPairEntry.firstMllNode.ttlThreshold[BIT_8] and other valid params.
    Expected: NON GT_OK.

    1.23. Call with mllPairEntry.firstMllNode.bindToMllCounterEnable[GT_TRUE], out of range
        mllPairEntry.firstMllNode.mllCounterIndex[3] (is relevant) and other valid params.
    Expected: NON GT_OK.

    1.24. Call with mllPairEntry.firstMllNode.bindToMllCounterEnable[GT_FALSE], out of range
        mllPairEntry.firstMllNode.mllCounterIndex[3] (not relevant) and other valid params.
    Expected: GT_OK.

    1.25. Call with out of range mllEntryPtr[NULL] and other valid params.
    Expected: GT_BAD_PTR.

    1.26. Call with out of range mllPairEntry.firstMllNode.meshId[256/4] and other valid params.
    Expected: GT_OUT_OF_RANGE.

    [For xCat and xCat3 devices]

    1.27. Call with tunnelStartEnable[GT_FALSE]
                    tunnelStartPassengerType[CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E]
                    and other valid params.
    Expected: GT_OK.

    1.28. Call with tunnelStartEnable[GT_TRUE]
                    tunnelStartPassengerType[CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E]
                    and other valid params.
    Expected: GT_OK.

    1.29. Call with  out of range tunnelStartPointer and other valid params.
    Expected: GT_OUT_OF_RANGE.

    1.30. Call with out of range tunnelStartPassengerType and other valid params.
    Expected: GT_BAD_PARAM.
*/

    GT_STATUS                                       st          = GT_OK;
    GT_U8                                           dev         = 0;

    GT_U32                                       mllPairEntryIndex=0;
    CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT           mllPairWriteForm=CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E;
    CPSS_DXCH_L2_MLL_PAIR_STC                    mllPairEntry;
    CPSS_DXCH_L2_MLL_PAIR_STC                    mllPairEntryRet;

    GT_U32                                       maxL2MllEntries;
    GT_STATUS   expectedSt;

    cpssOsBzero((GT_VOID*) &mllPairEntry, sizeof(mllPairEntry));
    cpssOsBzero((GT_VOID*) &mllPairEntryRet, sizeof(mllPairEntryRet));

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with mllPairEntryIndex[0, max/2, max],
                           mllPairEntry{
                                nextMllPointer[0, max/2, max],
                                last[GT_TRUE, GT_FALSE, GT_TRUE],
                                unknownUcFilterEnable[GT_FALSE, GT_TRUE, GT_TRUE],
                                unregMcFilterEnable[GT_FALSE, GT_TRUE, GT_TRUE],
                                bcFilterEnable[GT_TRUE, GT_FALSE, GT_FALSE],
                                mcLocalSwitchingEnable[GT_TRUE, GT_FALSE, GT_TRUE],
                                maxHopCountEnable[GT_FALSE, GT_TRUE, GT_TRUE],
                                maxOutgoingHopCount[0, 55, 63],
                                egressInterface{
                                    type[CPSS_INTERFACE_PORT_E,
                                         CPSS_INTERFACE_TRUNK_E,
                                         CPSS_INTERFACE_VIDX_E],
                                    devPort{
                                        devNum[devNum],
                                        portNum[0]
                                    }
                                    trunkId[1],
                                    vidx[10],
                                    vlanId[100],
                                    devNum[0],
                                    fabricVidx[0],
                                    mllPairEntryIndex[0]
                                }
                                maskBitmap[0, 0x3FF, 0x4000],
                                ttlThreshold[0, 0xA5, 0xFF],
                                bindToMllCounterEnable[GT_FALSE, GT_TRUE, GT_TRUE],
                                mllCounterIndex[0, 1, 2]
                                onePlusOneFilteringEnable[GT_TRUE,GT_FALSE,GT_FALSE]
                                meshId[0, 0xA5/2, 255/3],
                           }
        Expected: GT_OK.
        */
        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_FALSE && PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == GT_FALSE)
        {
            expectedSt = GT_NOT_APPLICABLE_DEVICE;
        }
        else
        {
            expectedSt = GT_OK;
        }

        maxL2MllEntries =
            PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.mllPairs;

        mllPairEntry.firstMllNode.tunnelStartPointer = 1;

        mllPairWriteForm = CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E;

        /* call with mllPairEntryIndex[0] */
        mllPairEntryIndex = 300;

        mllPairEntry.nextPointer = mllPairEntryIndex + 500 ;
        mllPairEntry.entrySelector = 0;

        mllPairEntry.firstMllNode.last = GT_FALSE;
        mllPairEntry.firstMllNode.unknownUcFilterEnable = GT_FALSE;
        mllPairEntry.firstMllNode.unregMcFilterEnable = GT_FALSE;
        mllPairEntry.firstMllNode.bcFilterEnable = GT_TRUE;
        mllPairEntry.firstMllNode.mcLocalSwitchingEnable = GT_TRUE;
        mllPairEntry.firstMllNode.maxHopCountEnable = GT_FALSE;
        mllPairEntry.firstMllNode.maxOutgoingHopCount = 0;

        mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_PORT_E;
        mllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum = dev;
        mllPairEntry.firstMllNode.egressInterface.devPort.portNum = 0;
        mllPairEntry.firstMllNode.egressInterface.trunkId = 1;
        mllPairEntry.firstMllNode.egressInterface.vidx = 10;
        mllPairEntry.firstMllNode.egressInterface.vlanId = 100;
        mllPairEntry.firstMllNode.egressInterface.hwDevNum = 0;
        mllPairEntry.firstMllNode.egressInterface.fabricVidx = 0;
        mllPairEntry.firstMllNode.egressInterface.index = 0;

        mllPairEntry.firstMllNode.maskBitmap = 0;
        mllPairEntry.firstMllNode.ttlThreshold = 0;
        mllPairEntry.firstMllNode.bindToMllCounterEnable = GT_FALSE;
        mllPairEntry.firstMllNode.mllCounterIndex = 0;
        mllPairEntry.firstMllNode.meshId = 0;
        mllPairEntry.firstMllNode.onePlusOneFilteringEnable=GT_TRUE;


        mllPairEntry.secondMllNode.last = GT_TRUE;
        mllPairEntry.secondMllNode.unknownUcFilterEnable = GT_TRUE;
        mllPairEntry.secondMllNode.unregMcFilterEnable = GT_TRUE;
        mllPairEntry.secondMllNode.bcFilterEnable = GT_FALSE;
        mllPairEntry.secondMllNode.mcLocalSwitchingEnable = GT_FALSE;
        mllPairEntry.secondMllNode.maxHopCountEnable = GT_TRUE;
        mllPairEntry.secondMllNode.maxOutgoingHopCount = 55;

        mllPairEntry.secondMllNode.egressInterface.type = CPSS_INTERFACE_TRUNK_E;
        mllPairEntry.secondMllNode.egressInterface.devPort.hwDevNum = dev;
        mllPairEntry.secondMllNode.egressInterface.devPort.portNum = 0;
        mllPairEntry.secondMllNode.egressInterface.trunkId = 1;
        mllPairEntry.secondMllNode.egressInterface.vidx = 10;
        mllPairEntry.secondMllNode.egressInterface.vlanId = 100;
        mllPairEntry.secondMllNode.egressInterface.hwDevNum = 0;
        mllPairEntry.secondMllNode.egressInterface.fabricVidx = 0;
        mllPairEntry.secondMllNode.egressInterface.index = 0;

        mllPairEntry.secondMllNode.maskBitmap = 0x3FF;
        mllPairEntry.secondMllNode.ttlThreshold = 0xA5;
        mllPairEntry.secondMllNode.bindToMllCounterEnable = GT_TRUE;
        mllPairEntry.secondMllNode.mllCounterIndex = 1;
        mllPairEntry.secondMllNode.meshId = 0;
        mllPairEntry.secondMllNode.onePlusOneFilteringEnable = GT_FALSE;

        st = cpssDxChL2MllPairWrite(dev, mllPairEntryIndex,mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        /*
            1.2. Call cpssDxChL2MllEntryGet.
            Expected: GT_OK and the same values as was set.
        */
        st = cpssDxChL2MllPairRead(dev, mllPairEntryIndex,mllPairWriteForm, &mllPairEntryRet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);
        /* check with previous set of parms, except
                mllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum,
                mllPairEntry.firstMllNode.egressInterface.devPort.portNum,
                mllPairEntry.firstMllNode.egressInterface.vidx,
                mllPairEntry.firstMllNode.egressInterface.vlanId,
                mllPairEntry.firstMllNode.egressInterface.hwDevNum,
                mllPairEntry.firstMllNode.egressInterface.fabricVidx,
                mllPairEntry.firstMllNode.egressInterface.index
            (this set of parms is not relevant with CPSS_INTERFACE_TRUNK_E) */
        if (GT_OK == st)
        {
            compareL2MllPair(dev, mllPairWriteForm,&mllPairEntry,&mllPairEntryRet);
        }

        /* call with index[max/2] */
        mllPairEntryIndex = maxL2MllEntries/2;

        mllPairWriteForm = CPSS_DXCH_PAIR_READ_WRITE_FIRST_ONLY_E;

        mllPairEntry.nextPointer = mllPairEntryIndex + 4;

        mllPairEntry.firstMllNode.last = GT_FALSE;
        mllPairEntry.firstMllNode.unknownUcFilterEnable = GT_TRUE;
        mllPairEntry.firstMllNode.unregMcFilterEnable = GT_TRUE;
        mllPairEntry.firstMllNode.bcFilterEnable = GT_FALSE;
        mllPairEntry.firstMllNode.mcLocalSwitchingEnable = GT_FALSE;
        mllPairEntry.firstMllNode.maxHopCountEnable = GT_TRUE;
        mllPairEntry.firstMllNode.maxOutgoingHopCount = 55;

        mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_TRUNK_E;
        mllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum = dev;
        mllPairEntry.firstMllNode.egressInterface.devPort.portNum = 0;
        mllPairEntry.firstMllNode.egressInterface.trunkId = 1;
        mllPairEntry.firstMllNode.egressInterface.vidx = 10;
        mllPairEntry.firstMllNode.egressInterface.vlanId = 100;
        mllPairEntry.firstMllNode.egressInterface.hwDevNum = 0;
        mllPairEntry.firstMllNode.egressInterface.fabricVidx = 0;
        mllPairEntry.firstMllNode.egressInterface.index = 0;

        mllPairEntry.firstMllNode.maskBitmap = 0x3FF;
        mllPairEntry.firstMllNode.ttlThreshold = 0xA5;
        mllPairEntry.firstMllNode.bindToMllCounterEnable = GT_TRUE;
        mllPairEntry.firstMllNode.mllCounterIndex = 1;
        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            mllPairEntry.firstMllNode.meshId = 2;
        }
        else
        {
            mllPairEntry.firstMllNode.meshId = 0xA5;
        }
        mllPairEntry.firstMllNode.onePlusOneFilteringEnable = GT_FALSE;

        st = cpssDxChL2MllPairWrite(dev, mllPairEntryIndex,mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        /*
            1.2. Call cpssDxChL2MllEntryGet.
            Expected: GT_OK and the same values as was set.
        */
        st = cpssDxChL2MllPairRead(dev, mllPairEntryIndex,mllPairWriteForm, &mllPairEntryRet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);
        /* check with previous set of parms, except
                mllPairEntry.firstMllNode.egressInterface.trunkId,
                mllPairEntry.firstMllNode.egressInterface.vidx,
                mllPairEntry.firstMllNode.egressInterface.vlanId,
                mllPairEntry.firstMllNode.egressInterface.hwDevNum,
                mllPairEntry.firstMllNode.egressInterface.fabricVidx,
                mllPairEntry.firstMllNode.egressInterface.index
            (this set of parms is not relevant with CPSS_INTERFACE_PORT_E) */
        if (GT_OK == st)
        {
            compareL2MllPair(dev, mllPairWriteForm,&mllPairEntry,&mllPairEntryRet);
        }


        mllPairWriteForm = CPSS_DXCH_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E;

        mllPairEntry.nextPointer = mllPairEntryIndex + 5;

        mllPairEntry.secondMllNode.last = GT_TRUE;
        mllPairEntry.secondMllNode.unknownUcFilterEnable = GT_TRUE;
        mllPairEntry.secondMllNode.unregMcFilterEnable = GT_TRUE;
        mllPairEntry.secondMllNode.bcFilterEnable = GT_FALSE;
        mllPairEntry.secondMllNode.mcLocalSwitchingEnable = GT_FALSE;
        mllPairEntry.secondMllNode.maxHopCountEnable = GT_TRUE;
        mllPairEntry.secondMllNode.maxOutgoingHopCount = 55;

        mllPairEntry.secondMllNode.egressInterface.type = CPSS_INTERFACE_TRUNK_E;
        mllPairEntry.secondMllNode.egressInterface.devPort.hwDevNum = dev;
        mllPairEntry.secondMllNode.egressInterface.devPort.portNum = 0;
        mllPairEntry.secondMllNode.egressInterface.trunkId = 1;
        mllPairEntry.secondMllNode.egressInterface.vidx = 10;
        mllPairEntry.secondMllNode.egressInterface.vlanId = 100;
        mllPairEntry.secondMllNode.egressInterface.hwDevNum = 0;
        mllPairEntry.secondMllNode.egressInterface.fabricVidx = 0;
        mllPairEntry.secondMllNode.egressInterface.index = 0;

        mllPairEntry.secondMllNode.maskBitmap = 0x3FF;
        mllPairEntry.secondMllNode.ttlThreshold = 0xA5;
        mllPairEntry.secondMllNode.bindToMllCounterEnable = GT_TRUE;
        mllPairEntry.secondMllNode.mllCounterIndex = 1;
        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            mllPairEntry.firstMllNode.meshId = 2;
        }
        else
        {
            mllPairEntry.firstMllNode.meshId = 0xA5;
        }
        mllPairEntry.secondMllNode.onePlusOneFilteringEnable = GT_TRUE;

        st = cpssDxChL2MllPairWrite(dev, mllPairEntryIndex,mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        /*
            1.2. Call cpssDxChL2MllEntryGet.
            Expected: GT_OK and the same values as was set.
        */
        st = cpssDxChL2MllPairRead(dev, mllPairEntryIndex,mllPairWriteForm, &mllPairEntryRet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);
        /* check with previous set of parms, except
                mllPairEntry.firstMllNode.egressInterface.trunkId,
                mllPairEntry.firstMllNode.egressInterface.vidx,
                mllPairEntry.firstMllNode.egressInterface.vlanId,
                mllPairEntry.firstMllNode.egressInterface.hwDevNum,
                mllPairEntry.firstMllNode.egressInterface.fabricVidx,
                mllPairEntry.firstMllNode.egressInterface.index
            (this set of parms is not relevant with CPSS_INTERFACE_PORT_E) */
        if (GT_OK == st)
        {
            compareL2MllPair(dev, mllPairWriteForm,&mllPairEntry,&mllPairEntryRet);
        }


        /* call with mllPairEntryIndex[max] */
        mllPairEntryIndex = maxL2MllEntries - 1;

        mllPairWriteForm = CPSS_DXCH_PAIR_READ_WRITE_FIRST_ONLY_E;

        mllPairEntry.firstMllNode.last = GT_TRUE;
        mllPairEntry.firstMllNode.unknownUcFilterEnable = GT_TRUE;
        mllPairEntry.firstMllNode.unregMcFilterEnable = GT_TRUE;
        mllPairEntry.firstMllNode.bcFilterEnable = GT_FALSE;
        mllPairEntry.firstMllNode.mcLocalSwitchingEnable = GT_TRUE;
        mllPairEntry.firstMllNode.maxHopCountEnable = GT_TRUE;
        mllPairEntry.firstMllNode.maxOutgoingHopCount = 63;

        mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_VIDX_E;
        mllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum = dev;
        mllPairEntry.firstMllNode.egressInterface.devPort.portNum = 0;
        mllPairEntry.firstMllNode.egressInterface.trunkId = 1;
        mllPairEntry.firstMllNode.egressInterface.vidx = 10;
        mllPairEntry.firstMllNode.egressInterface.vlanId = 100;
        mllPairEntry.firstMllNode.egressInterface.hwDevNum = 0;
        mllPairEntry.firstMllNode.egressInterface.fabricVidx = 0;
        mllPairEntry.firstMllNode.egressInterface.index = 0;

        mllPairEntry.firstMllNode.maskBitmap = 0x4000;
        mllPairEntry.firstMllNode.ttlThreshold = 0xFF;
        mllPairEntry.firstMllNode.bindToMllCounterEnable = GT_TRUE;
        mllPairEntry.firstMllNode.mllCounterIndex = 2;
        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            mllPairEntry.firstMllNode.meshId = 3;
        }
        else
        {
            mllPairEntry.firstMllNode.meshId = 0xFF;
        }
        mllPairEntry.firstMllNode.onePlusOneFilteringEnable = GT_FALSE;


        st = cpssDxChL2MllPairWrite(dev, mllPairEntryIndex,mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        /*
            1.2. Call cpssDxChL2MllEntryGet.
            Expected: GT_OK and the same values as was set.
        */
        st = cpssDxChL2MllPairRead(dev, mllPairEntryIndex,mllPairWriteForm, &mllPairEntryRet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);
        /* check with previous set of parms, except
                mllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum,
                mllPairEntry.firstMllNode.egressInterface.devPort.portNum,
                mllPairEntry.firstMllNode.egressInterface.trunkId,
                mllPairEntry.firstMllNode.egressInterface.vlanId,
                mllPairEntry.firstMllNode.egressInterface.hwDevNum,
                mllPairEntry.firstMllNode.egressInterface.fabricVidx,
                mllPairEntry.firstMllNode.egressInterface.index
            (this set of parms is not relevant with CPSS_INTERFACE_VIDX_E) */
        if (GT_OK == st)
        {
            compareL2MllPair(dev, mllPairWriteForm,&mllPairEntry,&mllPairEntryRet);
        }

        /*
            1.3. Call with out of range mllPairEntryIndex[max+1] and other valid params.
            Expected: NON GT_OK.
        */
        mllPairEntryIndex = maxL2MllEntries;
        st = cpssDxChL2MllPairWrite(dev, mllPairEntryIndex,mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* restore value */
        mllPairEntryIndex = maxL2MllEntries - 1;

        /*
            1.4. Call with out of range mllPairEntry.nextPointer[max+1] and other valid params.
            Expected: NON GT_OK.
        */
        mllPairWriteForm = CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E;
        mllPairEntry.nextPointer = maxL2MllEntries + 1;
        mllPairEntry.firstMllNode.last=GT_FALSE;
        mllPairEntry.secondMllNode.last=GT_FALSE;
        st = cpssDxChL2MllPairWrite(dev, mllPairEntryIndex,mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* restore value */
        mllPairWriteForm = CPSS_DXCH_PAIR_READ_WRITE_FIRST_ONLY_E;
        mllPairEntry.firstMllNode.last=GT_TRUE;

        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            /*
                1.5. Call with mllPairEntry.firstMllNode.maxHopCountEnable[GT_TRUE], out of range
                    mllPairEntry.firstMllNode.maxOutgoingHopCount[BIT_6] (is relevant) and other valid params.
                Expected: NON GT_OK.
            */

            mllPairEntry.firstMllNode.maxHopCountEnable = GT_TRUE;
            mllPairEntry.firstMllNode.maxOutgoingHopCount = BIT_6;

            st = cpssDxChL2MllPairWrite(dev, mllPairEntryIndex,mllPairWriteForm, &mllPairEntry);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* restore value */
            mllPairEntry.firstMllNode.maxHopCountEnable = GT_TRUE;
            mllPairEntry.firstMllNode.maxOutgoingHopCount = 63;

            /*
                1.6. Call with mllPairEntry.firstMllNode.maxHopCountEnable[GT_FALSE], out of range
                    mllPairEntry.firstMllNode.maxOutgoingHopCount[BIT_6] (not relevant) and other valid params.
                Expected: GT_OK.
            */
            mllPairEntry.firstMllNode.maxHopCountEnable = GT_FALSE;
            mllPairEntry.firstMllNode.maxOutgoingHopCount = BIT_6;

            st = cpssDxChL2MllPairWrite(dev, mllPairEntryIndex,mllPairWriteForm, &mllPairEntry);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* restore value */
            mllPairEntry.firstMllNode.maxHopCountEnable = GT_TRUE;
            mllPairEntry.firstMllNode.maxOutgoingHopCount = 63;

        }

        /*
            1.7. Call with out of range mllPairEntry.firstMllNode.egressInterface.type and other valid params.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChL2MllPairWrite
                            (dev, mllPairEntryIndex,mllPairWriteForm, &mllPairEntry),
                            mllPairEntry.firstMllNode.egressInterface.type);


        /*
            1.8. Call with mllPairEntry.firstMllNode.egressInterface.type[CPSS_INTERFACE_PORT_E], out of range
                mllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum[UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS+1] (is relevant)
                and other valid params.
            Expected: NON GT_OK.
        */
        mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_PORT_E;
        mllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(dev) + 1;

        st = cpssDxChL2MllPairWrite(dev, mllPairEntryIndex,mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* restore value */
        mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_VIDX_E;
        mllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum = dev;

        /*
            1.9. Call with mllPairEntry.firstMllNode.egressInterface.type[CPSS_INTERFACE_TRUNK_E], out of range
                mllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum[UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS+1] (not relevant)
                and other valid params.
            Expected: GT_OK.
        */
        mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_TRUNK_E;
        mllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(dev) + 1;

        st = cpssDxChL2MllPairWrite(dev, mllPairEntryIndex,mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        /* restore value */
        mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_VIDX_E;
        mllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum = dev;

        /*
            1.10. Call with mllPairEntry.firstMllNode.egressInterface.type[CPSS_INTERFACE_PORT_E], out of range
                mllPairEntry.firstMllNode.egressInterface.devPort.portNum[UTF_CPSS_PP_MAX_PORTS_NUM_CNS+1] (is relevant)
                and other valid params.
            Expected: NON GT_OK.
        */
        mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_PORT_E;
        mllPairEntry.firstMllNode.egressInterface.devPort.portNum = PRV_CPSS_SIP_5_20_CHECK_MAC(dev) ? BIT_14 : BIT_13;/*UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev) + 1;*/

        st = cpssDxChL2MllPairWrite(dev, mllPairEntryIndex,mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* restore value */
        mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_VIDX_E;
        mllPairEntry.firstMllNode.egressInterface.devPort.portNum = 0;

        /*
            1.11. Call with mllPairEntry.firstMllNode.egressInterface.type[CPSS_INTERFACE_TRUNK_E], out of range
                mllPairEntry.firstMllNode.egressInterface.devPort.portNum[UTF_CPSS_PP_MAX_PORTS_NUM_CNS+1] (not relevant)
                and other valid params.
            Expected: GT_OK.
        */
        mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_TRUNK_E;
        mllPairEntry.firstMllNode.egressInterface.devPort.portNum = PRV_CPSS_SIP_5_20_CHECK_MAC(dev) ? BIT_14 : BIT_13;/*UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev) + 1;*/

        st = cpssDxChL2MllPairWrite(dev, mllPairEntryIndex,mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        /* restore value */
        mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_VIDX_E;
        mllPairEntry.firstMllNode.egressInterface.devPort.portNum = 0;

        /*
            1.12. Call with mllPairEntry.firstMllNode.egressInterface.type[CPSS_INTERFACE_TRUNK_E], out of range
                mllPairEntry.firstMllNode.egressInterface.trunkId[UTF_CPSS_PP_MAX_TRUNK_ID_MAC+1] (is relevant)
                and other valid params.
            Expected: NON GT_OK.
        */
        mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_TRUNK_E;
        mllPairEntry.firstMllNode.egressInterface.trunkId = (GT_TRUNK_ID) (UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev) + 1);

        st = cpssDxChL2MllPairWrite(dev, mllPairEntryIndex,mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* restore value */
        mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_VIDX_E;
        mllPairEntry.firstMllNode.egressInterface.trunkId = 1;

        /*
            1.13. Call with mllPairEntry.firstMllNode.egressInterface.type[CPSS_INTERFACE_PORT_E], out of range
                mllPairEntry.firstMllNode.egressInterface.trunkId[UTF_CPSS_PP_MAX_TRUNK_ID_MAC+1] (not relevant)
                and other valid params.
            Expected: GT_OK.
        */
        mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_PORT_E;
        mllPairEntry.firstMllNode.egressInterface.trunkId = (GT_TRUNK_ID) (UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev) + 1);

        st = cpssDxChL2MllPairWrite(dev, mllPairEntryIndex,mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        /* restore value */
        mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_VIDX_E;
        mllPairEntry.firstMllNode.egressInterface.trunkId = 1;

        /*
            1.14. Call with mllPairEntry.firstMllNode.egressInterface.type[CPSS_INTERFACE_VIDX_E], out of range
                mllPairEntry.firstMllNode.egressInterface.vidx[4096] (is relevant) and other valid params.
            Expected: NON GT_OK.
        */
        mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_VIDX_E;
        mllPairEntry.firstMllNode.egressInterface.vidx = 4096;
        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            mllPairEntry.firstMllNode.egressInterface.vidx = 8192;
        }
        st = cpssDxChL2MllPairWrite(dev, mllPairEntryIndex, mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* restore value */
        mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_VIDX_E;
        mllPairEntry.firstMllNode.egressInterface.vidx = 10;

        /*
            1.15. Call with mllPairEntry.firstMllNode.egressInterface.type[CPSS_INTERFACE_PORT_E], out of range
                mllPairEntry.firstMllNode.egressInterface.vidx[4096] (not relevant) and other valid params.
            Expected: GT_OK.
        */
        mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_PORT_E;
        mllPairEntry.firstMllNode.egressInterface.vidx = 4096;

        st = cpssDxChL2MllPairWrite(dev, mllPairEntryIndex,mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        /* restore value */
        mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_VIDX_E;
        mllPairEntry.firstMllNode.egressInterface.vidx = 10;

        /*
            1.16. Call with mllPairEntry.firstMllNode.egressInterface.type[CPSS_INTERFACE_VID_E], out of range
                mllPairEntry.firstMllNode.egressInterface.vlanId[UTF_CPSS_PP_MAX_VLAN_NUM_CNS+1] (is relevant) and
                other valid params.
            Expected: GT_OK. because VID is ignored in this case
        */
        mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_VID_E;
        mllPairEntry.firstMllNode.egressInterface.vlanId = (GT_U16) (UTF_CPSS_PP_MAX_VLAN_NUM_CNS(dev) + 1);

        st = cpssDxChL2MllPairWrite(dev, mllPairEntryIndex,mllPairWriteForm, &mllPairEntry);
        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);
        }

        /* restore value */
        mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_VIDX_E;
        mllPairEntry.firstMllNode.egressInterface.vlanId = 100;

        /*
            1.17. Call with mllPairEntry.firstMllNode.egressInterface.type[CPSS_INTERFACE_PORT_E], out of range
                mllPairEntry.firstMllNode.egressInterface.vlanId[UTF_CPSS_PP_MAX_VLAN_NUM_CNS] (not relevant) and
                other valid params.
            Expected: GT_OK.
        */
        mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_PORT_E;
        mllPairEntry.firstMllNode.egressInterface.vlanId = (GT_U16) (UTF_CPSS_PP_MAX_VLAN_NUM_CNS(dev) + 1);

        st = cpssDxChL2MllPairWrite(dev, mllPairEntryIndex,mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        /* restore value */
        mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_VIDX_E;
        mllPairEntry.firstMllNode.egressInterface.vlanId = 100;

        /*
            1.18. Call with mllPairEntry.firstMllNode.egressInterface.type[CPSS_INTERFACE_PORT_E], out of range
                mllPairEntry.firstMllNode.egressInterface.hwDevNum[0xFFFFFFFF] (not relevant) and other valid params.
            Expected: GT_OK.
        */
        mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_PORT_E;
        mllPairEntry.firstMllNode.egressInterface.hwDevNum = 0xFFFFFFFF;

        st = cpssDxChL2MllPairWrite(dev, mllPairEntryIndex,mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        /* restore value */
        mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_VIDX_E;
        mllPairEntry.firstMllNode.egressInterface.hwDevNum = 0;

        /*
            1.19. Call with mllPairEntry.firstMllNode.egressInterface.type[CPSS_INTERFACE_PORT_E], out of range
                mllPairEntry.firstMllNode.egressInterface.fabricVidx[0xFFFF] (not relevant) and other valid params.
            Expected: GT_OK.
        */
        mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_PORT_E;
        mllPairEntry.firstMllNode.egressInterface.fabricVidx = 0xFFFF;

        st = cpssDxChL2MllPairWrite(dev, mllPairEntryIndex,mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        /* restore value */
        mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_VIDX_E;
        mllPairEntry.firstMllNode.egressInterface.fabricVidx = 0;

        /*
            1.20. Call with mllPairEntry.firstMllNode.egressInterface.type[CPSS_INTERFACE_PORT_E], out of range
                mllPairEntry.firstMllNode.egressInterface.index[0xFFFFFFFF] (not relevant) and other valid params.
            Expected: GT_OK.
        */
        mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_PORT_E;
        mllPairEntry.firstMllNode.egressInterface.index = 0xFFFFFFFF;

        st = cpssDxChL2MllPairWrite(dev, mllPairEntryIndex,mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        /* restore value */
        mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_VIDX_E;
        mllPairEntry.firstMllNode.egressInterface.index = 0;

        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            /*
                1.21. Call with out of range mllPairEntry.firstMllNode.maskBitmap[BIT_15] and other valid params.
                Expected: NON GT_OK.
            */
            mllPairEntry.firstMllNode.maskBitmap = BIT_15;

            st = cpssDxChL2MllPairWrite(dev, mllPairEntryIndex,mllPairWriteForm, &mllPairEntry);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* restore value */
            mllPairEntry.firstMllNode.maskBitmap = 0x4000;

            /*
                1.22. Call with out of range mllPairEntry.firstMllNode.ttlThreshold[BIT_8] and other valid params.
                Expected: NON GT_OK.
            */
            mllPairEntry.firstMllNode.ttlThreshold = BIT_8;

            st = cpssDxChL2MllPairWrite(dev, mllPairEntryIndex,mllPairWriteForm, &mllPairEntry);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* restore value */
            mllPairEntry.firstMllNode.ttlThreshold = 0xFF;

            /*
                1.23. Call with mllPairEntry.firstMllNode.bindToMllCounterEnable[GT_TRUE], out of range
                    mllPairEntry.firstMllNode.mllCounterIndex[3] (is relevant) and other valid params.
                Expected: NON GT_OK.
            */
            mllPairEntry.firstMllNode.bindToMllCounterEnable = GT_TRUE;
            mllPairEntry.firstMllNode.mllCounterIndex = 3;

            st = cpssDxChL2MllPairWrite(dev, mllPairEntryIndex,mllPairWriteForm, &mllPairEntry);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* restore value */
            mllPairEntry.firstMllNode.bindToMllCounterEnable = GT_TRUE;
            mllPairEntry.firstMllNode.mllCounterIndex = 2;

            /*
                1.24. Call with mllPairEntry.firstMllNode.bindToMllCounterEnable[GT_FALSE], out of range
                    mllPairEntry.firstMllNode.mllCounterIndex[3] (not relevant) and other valid params.
                Expected: GT_OK.
            */
            mllPairEntry.firstMllNode.bindToMllCounterEnable = GT_FALSE;
            mllPairEntry.firstMllNode.mllCounterIndex = 3;

            st = cpssDxChL2MllPairWrite(dev, mllPairEntryIndex,mllPairWriteForm, &mllPairEntry);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* restore value */
            mllPairEntry.firstMllNode.bindToMllCounterEnable = GT_TRUE;
            mllPairEntry.firstMllNode.mllCounterIndex = 2;
        }

        /*
            1.25. Call with out of range mllPairEntry[NULL] and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChL2MllPairWrite(dev, mllPairEntryIndex,mllPairWriteForm, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.26. Call with out of range mllPairEntry.firstMllNode.meshId[256/4] and other valid params.
            Expected: GT_OUT_OF_RANGE.
        */
        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            mllPairEntry.firstMllNode.meshId = 4;
        }
        else
        {
            mllPairEntry.firstMllNode.meshId = 256;
        }
        st = cpssDxChL2MllPairWrite(dev, mllPairEntryIndex,mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* restore value */
        mllPairEntry.firstMllNode.meshId = 0;

        /*
            1.27. Call with tunnelStartEnable[GT_FALSE]
                    tunnelStartPassengerType[CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E]
                    and other valid params.
            Expected: GT_OK.
        */
        mllPairEntry.firstMllNode.tunnelStartEnable = GT_FALSE;
        mllPairEntry.firstMllNode.tunnelStartPassengerType = CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E;

        st = cpssDxChL2MllPairWrite(dev, mllPairEntryIndex,mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        st = cpssDxChL2MllPairRead(dev, mllPairEntryIndex, mllPairWriteForm, &mllPairEntryRet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        if (expectedSt == GT_OK)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntry.firstMllNode.tunnelStartEnable,
                                         mllPairEntryRet.firstMllNode.tunnelStartEnable,
                                         "get another tunnelStartEnable than was set: %d", dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntry.firstMllNode.tunnelStartPassengerType,
                                         mllPairEntryRet.firstMllNode.tunnelStartPassengerType,
                                         "get another tunnelStartPassengerType than was set: %d", dev);
        }
        /*
            1.28. Call with tunnelStartEnable[GT_TRUE]
                    tunnelStartPassengerType[CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E]
                    and other valid params.
            Expected: GT_OK.
        */
        mllPairEntry.firstMllNode.tunnelStartEnable = GT_TRUE;
        mllPairEntry.firstMllNode.tunnelStartPassengerType = CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E;

        st = cpssDxChL2MllPairWrite(dev, mllPairEntryIndex,mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        st = cpssDxChL2MllPairRead(dev, mllPairEntryIndex, mllPairWriteForm, &mllPairEntryRet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        if (expectedSt == GT_OK && PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntry.firstMllNode.tunnelStartEnable,
                                         mllPairEntryRet.firstMllNode.tunnelStartEnable,
                                         "get another tunnelStartEnable than was set: %d", dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(mllPairEntry.firstMllNode.tunnelStartPassengerType,
                                         mllPairEntryRet.firstMllNode.tunnelStartPassengerType,
                                         "get another tunnelStartPassengerType than was set: %d", dev);
        }

        /*
            1.29. Call with out of range tunnelStartPointer and other valid params.
            Expected: GT_OUT_OF_RANGE.
        */
        mllPairEntry.firstMllNode.tunnelStartPointer = 4096;

        st = cpssDxChL2MllPairWrite(dev, mllPairEntryIndex,mllPairWriteForm, &mllPairEntry);
        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            expectedSt = GT_OUT_OF_RANGE;
        }
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        /* restore value */
        mllPairEntry.firstMllNode.tunnelStartPointer = 1;

        /*
            1.30. Call with out of range tunnelStartPassengerType and other valid params.
            Expected: GT_BAD_PARAM.
        */
                if (PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_TRUE)
                {
                        UTF_ENUMS_CHECK_MAC(cpssDxChL2MllPairWrite
                                                (dev, mllPairEntryIndex,mllPairWriteForm, &mllPairEntry),
                                                    mllPairEntry.firstMllNode.tunnelStartPassengerType);
                }
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* restore value */
    mllPairEntryIndex = 0;

    mllPairEntry.firstMllNode.last = GT_TRUE;
    mllPairEntry.firstMllNode.unknownUcFilterEnable = GT_TRUE;
    mllPairEntry.firstMllNode.unregMcFilterEnable = GT_TRUE;
    mllPairEntry.firstMllNode.bcFilterEnable = GT_FALSE;
    mllPairEntry.firstMllNode.mcLocalSwitchingEnable = GT_TRUE;
    mllPairEntry.firstMllNode.maxHopCountEnable = GT_TRUE;
    mllPairEntry.firstMllNode.maxOutgoingHopCount = 63;

    mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_VIDX_E;
    mllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum = dev;
    mllPairEntry.firstMllNode.egressInterface.devPort.portNum = 0;
    mllPairEntry.firstMllNode.egressInterface.trunkId = 1;
    mllPairEntry.firstMllNode.egressInterface.vidx = 10;
    mllPairEntry.firstMllNode.egressInterface.vlanId = 100;
    mllPairEntry.firstMllNode.egressInterface.hwDevNum = 0;
    mllPairEntry.firstMllNode.egressInterface.fabricVidx = 0;
    mllPairEntry.firstMllNode.egressInterface.index = 0;

    mllPairEntry.firstMllNode.maskBitmap = 0x4000;
    mllPairEntry.firstMllNode.ttlThreshold = 0xFF;
    mllPairEntry.firstMllNode.bindToMllCounterEnable = GT_TRUE;
    mllPairEntry.firstMllNode.mllCounterIndex = 2;
    mllPairEntry.firstMllNode.meshId = 0;
    mllPairEntry.firstMllNode.onePlusOneFilteringEnable = GT_FALSE;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChL2MllPairWrite(dev, mllPairEntryIndex,mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
            st = cpssDxChL2MllPairWrite(dev, mllPairEntryIndex,mllPairWriteForm, &mllPairEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChL2MllPairRead
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      mllPairEntryIndex,
    IN  CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT          mllPairReadForm,
    OUT CPSS_DXCH_L2_MLL_PAIR_STC                  *mllPairEntryPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChL2MllPairRead)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with mllPairEntryIndex[0, max/2, max], non-null mllEntry.
    Expected: GT_OK.
    1.2. Call with out of range mllPairEntryIndex[max+1] and other valid params.
    Expected: NON GT_OK.
    1.3. Call with out of range mllPairEntry and other valid params.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                                    st          = GT_OK;
    GT_U8                                        dev         = 0;
    GT_U32                                       maxL2MllEntries;

    GT_U32                                       mllPairEntryIndex=0;
    CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT           mllPairWriteForm=CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E;
    CPSS_DXCH_L2_MLL_PAIR_STC                    mllPairEntry;
    GT_STATUS   expectedSt;

    cpssOsBzero((GT_VOID*) &mllPairEntry, sizeof(mllPairEntry));

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_FALSE && PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == GT_FALSE)
        {
            expectedSt = GT_NOT_APPLICABLE_DEVICE;
        }
        else
        {
            expectedSt = GT_OK;
        }

        /*
            1.1. Call with mllPairEntryIndex[0, max/2, max], non-null mllPairEntry.
            Expected: GT_OK.
        */

        mllPairWriteForm = CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E;

        maxL2MllEntries =
            PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.mllPairs;

        /* write (by zeroes) L2 MLL entries before reading to avoid a garbage
         * in the OUT-parameter mllPairEntryPtr of cpssDxChL2MllPairRead.
         * The garbage causes a differences in comparing of CPSS API log
         * results between different baselines. */
        st = cpssDxChL2MllPairWrite(dev, 0,
                                    CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E, &mllPairEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        st = cpssDxChL2MllPairWrite(dev, maxL2MllEntries / 2,
                                    CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E, &mllPairEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        st = cpssDxChL2MllPairWrite(dev, maxL2MllEntries -1,
                                    CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E, &mllPairEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);


        /* call with mllPairEntryIndex[0] */
        mllPairEntryIndex = 0;
        st = cpssDxChL2MllPairRead(dev, mllPairEntryIndex, mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        /* call with mllPairEntryIndex[max/2] */
        mllPairEntryIndex = maxL2MllEntries / 2;
        st = cpssDxChL2MllPairRead(dev, mllPairEntryIndex, mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        /* call with mllPairEntryIndex[max] */
        mllPairEntryIndex = maxL2MllEntries - 1;
        st = cpssDxChL2MllPairRead(dev, mllPairEntryIndex, mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        mllPairWriteForm = CPSS_DXCH_PAIR_READ_WRITE_FIRST_ONLY_E;

        /* call with mllPairEntryIndex[0] */
        mllPairEntryIndex = 0;
        st = cpssDxChL2MllPairRead(dev, mllPairEntryIndex, mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        /* call with mllPairEntryIndex[max/2] */
        mllPairEntryIndex = maxL2MllEntries / 2;
        st = cpssDxChL2MllPairRead(dev, mllPairEntryIndex, mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        /* call with mllPairEntryIndex[max] */
        mllPairEntryIndex = maxL2MllEntries - 1;
        st = cpssDxChL2MllPairRead(dev, mllPairEntryIndex, mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        mllPairWriteForm = CPSS_DXCH_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E;

        /* call with mllPairEntryIndex[0] */
        mllPairEntryIndex = 0;
        st = cpssDxChL2MllPairRead(dev, mllPairEntryIndex, mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        /* call with mllPairEntryIndex[max/2] */
        mllPairEntryIndex = maxL2MllEntries / 2;
        st = cpssDxChL2MllPairRead(dev, mllPairEntryIndex, mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        /* call with mllPairEntryIndex[max] */
        mllPairEntryIndex = maxL2MllEntries - 1;
        st = cpssDxChL2MllPairRead(dev, mllPairEntryIndex, mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        mllPairWriteForm = CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E;

        /*
            1.2. Call with out of range mllPairEntryIndex[max+1] and other valid params.
            Expected: NON GT_OK.
        */
        mllPairEntryIndex = maxL2MllEntries;
        st = cpssDxChL2MllPairRead(dev, mllPairEntryIndex, mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* restore value */
        mllPairEntryIndex = maxL2MllEntries / 2;

        /*
            1.3. Call with  mllPairEntry[NULL] and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChL2MllPairRead(dev, mllPairEntryIndex, mllPairWriteForm, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* restore value */
    mllPairEntryIndex = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChL2MllPairRead(dev, mllPairEntryIndex, mllPairWriteForm, &mllPairEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChL2MllPairRead(dev, mllPairEntryIndex, mllPairWriteForm, &mllPairEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChL2MllLookupForAllEvidxEnableSet
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChL2MllLookupForAllEvidxEnableSet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1. Call with enable[GT_FALSE, GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChL2MllLookupForAllEvidxEnableGet.
    Expected: GT_OK and the same values as was set.
*/
    GT_STATUS                               st          = GT_OK;
    GT_U8                                   dev         = 0;
    GT_BOOL                                 enable      = GT_FALSE;
    GT_BOOL                                 enableRet   = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E |
                                           UTF_XCAT2_E | UTF_XCAT3_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with enable[GT_FALSE, GT_TRUE].
            Expected: GT_OK.
        */

        /* call with enable[GT_FALSE] */
        enable = GT_FALSE;

        st = cpssDxChL2MllLookupForAllEvidxEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChL2MllLookupForAllEvidxEnableGet.
                Expected: GT_OK and the same values as was set.
        */
        st = cpssDxChL2MllLookupForAllEvidxEnableGet(dev, &enableRet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableRet,
                                         "got another enable: %d", enableRet);
        }

        /* call with enable[GT_TRUE] */
        enable = GT_TRUE;

        st = cpssDxChL2MllLookupForAllEvidxEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChL2MllLookupForAllEvidxEnableGet.
                Expected: GT_OK and the same values as was set.
        */
        st = cpssDxChL2MllLookupForAllEvidxEnableGet(dev, &enableRet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableRet,
                                         "got another enable: %d", enableRet);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E |
                                           UTF_XCAT2_E | UTF_XCAT3_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChL2MllLookupForAllEvidxEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChL2MllLookupForAllEvidxEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChL2MllLookupForAllEvidxEnableGet
(
    IN GT_U8    devNum,
    OUT GT_BOOL *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChL2MllLookupForAllEvidxEnableGet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1. Call with non-null enablePtr.
    Expected: GT_OK.
    1.2. Call with out of range enablePtr[NULL] and other valid params.
    Expected: NON GT_OK.
*/
    GT_STATUS                               st              = GT_OK;
    GT_U8                                   dev             = 0;
    GT_BOOL                                 enable          = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChL2MllLookupForAllEvidxEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
        1.2. Call with out of range enablePtr[NULL] and other valid params.
        Expected: NON GT_OK.
        */
        st = cpssDxChL2MllLookupForAllEvidxEnableGet(dev, NULL);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChL2MllLookupForAllEvidxEnableGet: %d", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChL2MllLookupForAllEvidxEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChL2MllLookupForAllEvidxEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChL2MllLookupMaxVidxIndexSet
(
    IN GT_U8   devNum,
    IN GT_U32  maxVidxIndex
)
*/
UTF_TEST_CASE_MAC(cpssDxChL2MllLookupMaxVidxIndexSet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1. Call with maxVidxIndex[100],
    Expected: GT_OK
    1.2. Call cpssDxChL2MllLookupMaxVidxIndexGet.
    Expected: GT_OK and the same value as was set.
    1.3. Call with maxVidxIndex[0xFFF],
    Expected: GT_OK
    1.4. Call cpssDxChL2MllLookupMaxVidxIndexGet.
    Expected: GT_OK and the same value as was set.
    1.5. Call with maxVidxIndex[0xFFF+1].
    Expected: GT_OUT_OF_RANGE

*/
    GT_STATUS                               st                = GT_OK;
    GT_U8                                   dev               = 0;
    GT_U32                                  maxVidxIndex      = 0;
    GT_U32                                  maxVidxIndexRet   = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with maxVidxIndex[100].
            Expected: GT_OK
        */

        maxVidxIndex=100;

        st = cpssDxChL2MllLookupMaxVidxIndexSet(dev, maxVidxIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChL2MllLookupMaxVidxIndexGet.
                Expected: GT_OK and the same value as was set.
        */
        st = cpssDxChL2MllLookupMaxVidxIndexGet(dev, &maxVidxIndexRet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(maxVidxIndex, maxVidxIndexRet,
                                         "got another maxVidxIndexRet: %d", maxVidxIndexRet);
        }

        /*
            1.3. Call with maxVidxIndex[0xFFF].
            Expected: GT_OK
        */

        maxVidxIndex=0xFFF;

        st = cpssDxChL2MllLookupMaxVidxIndexSet(dev, maxVidxIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        /*
            1.4. Call cpssDxChL2MllLookupMaxVidxIndexGet.
                Expected: GT_OK and the same value as was set.
        */
        st = cpssDxChL2MllLookupMaxVidxIndexGet(dev, &maxVidxIndexRet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(maxVidxIndex, maxVidxIndexRet,
                                         "got another maxVidxIndexRet: %d", maxVidxIndexRet);
        }

        /*
            1.5. Call with maxVidxIndex[0xFFF+1].
            Expected: GT_OK
        */

        maxVidxIndex=(0xFFF+1);

        st = cpssDxChL2MllLookupMaxVidxIndexSet(dev, maxVidxIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChL2MllLookupMaxVidxIndexSet(dev, maxVidxIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChL2MllLookupForAllEvidxEnableSet(dev, maxVidxIndex);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChL2MllLookupMaxVidxIndexGet
(
    IN  GT_U8    devNum,
    OUT GT_U32   *maxVidxIndexPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChL2MllLookupMaxVidxIndexGet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1. Call with non-null maxVidxIndexPtr.
    Expected: GT_OK.
    1.2. Call with out of range maxVidxIndexPtr[NULL] and other valid params.
    Expected: NON GT_OK.
*/
    GT_STATUS                               st              = GT_OK;
    GT_U8                                   dev             = 0;
    GT_U32                                  maxVidxIndex    = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChL2MllLookupMaxVidxIndexGet(dev, &maxVidxIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
        1.2. Call with out of range enablePtr[NULL] and other valid params.
        Expected: NON GT_OK.
        */
        st = cpssDxChL2MllLookupMaxVidxIndexGet(dev, NULL);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChL2MllLookupMaxVidxIndexGet: %d", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChL2MllLookupMaxVidxIndexGet(dev, &maxVidxIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChL2MllLookupMaxVidxIndexGet(dev, &maxVidxIndex);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChL2MllSetCntInterfaceCfg
(
    IN GT_U8                                            devNum,
    IN GT_U32                                           mllCntSet,
    IN CPSS_DXCH_L2MLL_COUNTER_SET_INTERFACE_CFG_STC    *interfaceCfgPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChL2MllSetCntInterfaceCfg)
{
/*
ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
1.1. Call with mllCntSet [0] and interfaceCfgPtr
     {portTrunkCntMode[CPSS_DXCH_L2MLL_DISREGARD_PORT_TRUNK_CNT_MODE_E /
     CPSS_DXCH_L2MLL_PORT_CNT_MODE_E / CPSS_DXCH_L2MLL_TRUNK_CNT_MODE_E],
     ipMode [CPSS_IP_PROTOCOL_IPV4_E / CPSS_IP_PROTOCOL_IPV6_E /
     CPSS_IP_PROTOCOL_IPV4V6_E], vlanMode [CPSS_DXCH_L2MLL_DISREGARD_VLAN_CNT_MODE_E /
     CPSS_DXCH_L2MLL_USE_VLAN_CNT_MODE_E], devNum [dev], portTrunk {port[0], trunk[0]},
     vlanId [BRG_VLAN_TESTED_VLAN_ID_CNS = 100]}.
Expected: GT_OK.
1.2. Call with out of range mllCntSet [5] and other valid parameters from 1.1.
Expected: NOT GT_OK.
1.3. Call with wrong enum values interfaceCfgPtr->portTrunkCntMode
     and other valid parameters from 1.1.
Expected: GT_BAD_PARAM.
1.4. Call with wrong enum values interfaceCfgPtr->ipMode
     and other valid parameters from 1.1.
Expected: GT_BAD_PARAM.
1.5. Call with wrong enum values interfaceCfgPtr->vlanMode
     and other valid parameters from 1.1.
Expected: GT_BAD_PARAM.
1.6. Call with interfaceCfgPtr {portTrunkCntMode[CPSS_DXCH_L2MLL_PORT_CNT_MODE_E],
     out of range portTrunk.port [UTF_CPSS_PP_MAX_PORTS_NUM_CNS]
     and other valid parameters from 1.1.
Expected: NOT GT_OK.
1.7. Call with interfaceCfgPtr {portTrunkCntMode[CPSS_DXCH_L2MLL_TRUNK_CNT_MODE_E],
     out of range portTrunk.trunk [UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev) = 128]
     and other valid parameters from 1.1.
Expected: NOT GT_OK.
1.8. Call with interfaceCfgPtr {portTrunkCntMode[CPSS_DXCH_L2MLL_TRUNK_CNT_MODE_E]
     (in this case trunk is not relevant), out of range portTrunk.port
     [UTF_CPSS_PP_MAX_PORTS_NUM_CNS] and other valid parameters from 1.1.
Expected: GT_BAD_PARAM
1.9. Call with interfaceCfgPtr {portTrunkCntMode[CPSS_DXCH_L2MLL_PORT_CNT_MODE_E]
     (in this case trunk is not relevant), out of range portTrunk.trunk
    [UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev) = 128]  and other valid parameters from 1.1.
Expected: GT_BAD_PARAM
1.10. Call with interfaceCfgPtr {portTrunkCntMode[CPSS_DXCH_L2MLL_PORT_CNT_MODE_E]
     (in this case trunk is not relevant), out of range portTrunk.port = 0
     and other valid parameters from 1.1.
Expected: GT_OK.
1.11. Call with interfaceCfgPtr {vlanMode [CPSS_DXCH_L2MLL_USE_VLAN_CNT_MODE_E],
      out of range vlanId [PRV_CPSS_MAX_NUM_VLANS_CNS = 4096]
      and other valid parameters from 1.1.
Expected: non GT_OK.
1.12. Call with interfaceCfgPtr {vlanMode [CPSS_DXCH_L2MLL_DISREGARD_VLAN_CNT_MODE_E]
      (in this case vlanId is not relevant), out of range vlanId
      [PRV_CPSS_MAX_NUM_VLANS_CNS = 4096] and other valid parameters from 1.1.
Expected: GT_OK.
1.12. Call with interfaceCfgPtr [NULL] and other valid parameters from 1.1.
Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_U32                                     mllCntSet;
    CPSS_DXCH_L2MLL_COUNTER_SET_INTERFACE_CFG_STC interfaceCfg;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                       UTF_CH2_E | UTF_CH3_E |
                                       UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                       UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call with mllCntSet [0] and interfaceCfgPtr
          {portTrunkCntMode[CPSS_DXCH_L2MLL_DISREGARD_PORT_TRUNK_CNT_MODE_E /
          CPSS_DXCH_L2MLL_PORT_CNT_MODE_E / CPSS_DXCH_L2MLL_TRUNK_CNT_MODE_E],
          ipMode [CPSS_IP_PROTOCOL_IPV4_E / CPSS_IP_PROTOCOL_IPV6_E /
          CPSS_IP_PROTOCOL_IPV4V6_E], vlanMode [CPSS_DXCH_L2MLL_DISREGARD_VLAN_CNT_MODE_E
          / CPSS_DXCH_L2MLL_USE_VLAN_CNT_MODE_E], devNum [dev], portTrunk {port[0],
          trunk[0]}, vlanId [BRG_VLAN_TESTED_VLAN_ID_CNS = 100]}.Expected: GT_OK.   */

        mllCntSet = 0;
        cpssOsBzero((GT_VOID*) &interfaceCfg, sizeof(interfaceCfg));
        interfaceCfg.portTrunkCntMode = CPSS_DXCH_L2MLL_DISREGARD_PORT_TRUNK_CNT_MODE_E;
        interfaceCfg.ipMode = CPSS_IP_PROTOCOL_IPV4_E;
        interfaceCfg.vlanMode = CPSS_DXCH_L2MLL_DISREGARD_VLAN_CNT_MODE_E;
        interfaceCfg.hwDevNum = dev;
        interfaceCfg.portTrunk.port = 0;
        interfaceCfg.portTrunk.trunk = 0;
        interfaceCfg.vlanId = 100;

        st = cpssDxChL2MllSetCntInterfaceCfg(dev, mllCntSet, &interfaceCfg);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mllCntSet);

        interfaceCfg.portTrunkCntMode = CPSS_DXCH_L2MLL_TRUNK_CNT_MODE_E;
        interfaceCfg.ipMode = CPSS_IP_PROTOCOL_IPV4V6_E;
        interfaceCfg.vlanMode = CPSS_DXCH_L2MLL_USE_VLAN_CNT_MODE_E;

        st = cpssDxChL2MllSetCntInterfaceCfg(dev, mllCntSet, &interfaceCfg);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mllCntSet);

        /* restore: */

        interfaceCfg.portTrunkCntMode = CPSS_DXCH_L2MLL_DISREGARD_PORT_TRUNK_CNT_MODE_E;
        interfaceCfg.ipMode = CPSS_IP_PROTOCOL_IPV4_E;
        interfaceCfg.vlanMode = CPSS_DXCH_L2MLL_DISREGARD_VLAN_CNT_MODE_E;

        /* 1.2. Call with out of range mllCntSet [5] and other valid
           parameters from 1.1. Expected: NOT GT_OK.    */

        mllCntSet = 5;

        st = cpssDxChL2MllSetCntInterfaceCfg(dev, mllCntSet, &interfaceCfg);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, mllCntSet);

        mllCntSet = 0;

        /* 1.3. Call with wrong enum values interfaceCfgPtr->portTrunkCntMode
            and other valid parameters from 1.1. Expected: GT_BAD_PARAM.*/
        UTF_ENUMS_CHECK_MAC(cpssDxChL2MllSetCntInterfaceCfg
                            (dev, mllCntSet, &interfaceCfg),
                            interfaceCfg.portTrunkCntMode);

        /* 1.4. Call with wrong enum values interfaceCfgPtr->ipMode  and
           other valid parameters from 1.1. Expected: GT_BAD_PARAM. */
        UTF_ENUMS_CHECK_MAC(cpssDxChL2MllSetCntInterfaceCfg
                            (dev, mllCntSet, &interfaceCfg),
                            interfaceCfg.ipMode);

        /* 1.5. Call with wrong enum values interfaceCfgPtr->vlanMode
           and other valid parameters from 1.1. Expected: GT_BAD_PARAM. */
        UTF_ENUMS_CHECK_MAC(cpssDxChL2MllSetCntInterfaceCfg
                            (dev, mllCntSet, &interfaceCfg),
                            interfaceCfg.vlanMode);

        /* 1.8. Call with interfaceCfgPtr {portTrunkCntMode[CPSS_DXCH_L2MLL_TRUNK_CNT_MODE_E]
           (in this case port is not relevant but part of union), so set trunk
           and other valid parameters from 1.1. Expected: GT_OK. */

        interfaceCfg.portTrunkCntMode = CPSS_DXCH_L2MLL_TRUNK_CNT_MODE_E;
        interfaceCfg.portTrunk.trunk = 0;

        st = cpssDxChL2MllSetCntInterfaceCfg(dev, mllCntSet, &interfaceCfg);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
         "%d, interfaceCfg.portTrunkCntMode = %d, interfaceCfg.portTrunk.port = %d",
                dev, interfaceCfg.portTrunkCntMode, interfaceCfg.portTrunk.port);
        /* restore */
        interfaceCfg.portTrunkCntMode = CPSS_DXCH_L2MLL_DISREGARD_PORT_TRUNK_CNT_MODE_E;
        interfaceCfg.portTrunk.port = 0;


        /* 1.9. Call with interfaceCfgPtr {portTrunkCntMode[CPSS_DXCH_L2MLL_PORT_CNT_MODE_E]
           (in this case trunk is not relevant), out of range portTrunk.trunk
           [UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev) = 128] and other valid parameters from 1.1.
           Expected: GT_BAD_PARAM. */

        interfaceCfg.portTrunkCntMode = CPSS_DXCH_L2MLL_TRUNK_CNT_MODE_E;
        interfaceCfg.portTrunk.trunk = UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev);

        st = cpssDxChL2MllSetCntInterfaceCfg(dev, mllCntSet, &interfaceCfg);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st,
            "%d, interfaceCfg.portTrunkCntMode = %d, interfaceCfg.portTrunk.trunk = %d",
                    dev, interfaceCfg.portTrunkCntMode, interfaceCfg.portTrunk.trunk);


        /* 1.10. Call with interfaceCfgPtr {portTrunkCntMode[CPSS_DXCH_L2MLL_PORT_CNT_MODE_E]
           (in this case trunk is not relevant), oportTrunk.port =0
           and other valid parameters from 1.1.
           Expected: GT_OK */

        interfaceCfg.portTrunkCntMode = CPSS_DXCH_L2MLL_PORT_CNT_MODE_E;
        interfaceCfg.portTrunk.port = 0;

        st = cpssDxChL2MllSetCntInterfaceCfg(dev, mllCntSet, &interfaceCfg);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "%d, interfaceCfg.portTrunkCntMode = %d, interfaceCfg.portTrunk.port = %d",
                                     dev, interfaceCfg.portTrunkCntMode, interfaceCfg.portTrunk.port);

        /* restore */
        interfaceCfg.portTrunkCntMode = CPSS_DXCH_L2MLL_DISREGARD_PORT_TRUNK_CNT_MODE_E;
        interfaceCfg.portTrunk.trunk = 0;

        /* 1.11. Call with
           interfaceCfgPtr {vlanMode [CPSS_DXCH_L2MLL_DISREGARD_VLAN_CNT_MODE_E]
           (in this case vlanId is not relevant), out of range vlanId
           [PRV_CPSS_MAX_NUM_VLANS_CNS = 4096] and other valid parameters from 1.1.
           Expected: GT_OK. */

        interfaceCfg.vlanId = PRV_CPSS_MAX_NUM_VLANS_CNS;

        st = cpssDxChL2MllSetCntInterfaceCfg(dev, mllCntSet, &interfaceCfg);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                "%d, interfaceCfg.vlanMode = %d, interfaceCfg.vlanId = %d",
                          dev, interfaceCfg.vlanMode, interfaceCfg.vlanId);
        /* restore */
        interfaceCfg.vlanId = 100;


        /* 1.12. Call with
           interfaceCfgPtr [NULL] and other valid parameters from 1.1.
           Expected: GT_BAD_PTR.    */

        st = cpssDxChL2MllSetCntInterfaceCfg(dev, mllCntSet, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                     "%d, interfaceCfgPtr = NULL", dev);
    }

    mllCntSet = 0;
    cpssOsBzero((GT_VOID*) &interfaceCfg, sizeof(interfaceCfg));
    interfaceCfg.portTrunkCntMode = CPSS_DXCH_L2MLL_DISREGARD_PORT_TRUNK_CNT_MODE_E;
    interfaceCfg.ipMode = CPSS_IP_PROTOCOL_IPV4_E;
    interfaceCfg.vlanMode = CPSS_DXCH_L2MLL_DISREGARD_VLAN_CNT_MODE_E;
    interfaceCfg.hwDevNum = dev;
    interfaceCfg.portTrunk.port = 0;
    interfaceCfg.portTrunk.trunk = 0;
    interfaceCfg.vlanId = 100;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChL2MllSetCntInterfaceCfg(dev, mllCntSet, &interfaceCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChL2MllSetCntInterfaceCfg(dev, mllCntSet, &interfaceCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChL2MllMcCntGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  mllCntSet,
    OUT GT_U32  *mllOutMCPktsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChL2MllMcCntGet)
{
/*
ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
1.1. Call with mllCntSet [0] and non-null mllOutMCPktsPtr.
Expected: GT_OK.
1.2. Call with mllCntSet [10] and non-null mllOutMCPktsPtr.
Expected: GT_BAD_PARAM.
1.3. Call with mllCntSet [0] and mllOutMCPkts [NULL].
Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_U32  mllCntSet;
    GT_U32  mllOutMCPkts;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                       UTF_CH2_E | UTF_CH3_E |
                                       UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                       UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call with mllCntSet [0] and non-null mllOutMCPktsPtr.
           Expected: GT_OK. */

        mllCntSet = 0;

        st = cpssDxChL2MllMcCntGet(dev, mllCntSet, &mllOutMCPkts);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mllCntSet);

        /* 1.2. Call with mllCntSet [10] and non-null mllOutMCPktsPtr.
                Expected: GT_BAD_PARAM. */

        mllCntSet = 10;

        st = cpssDxChL2MllMcCntGet(dev, mllCntSet, &mllOutMCPkts);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, mllCntSet);

        /* 1.3. Call with mllCntSet [0] and mllOutMCPkts [NULL].
                Expected: GT_BAD_PTR.   */

        mllCntSet = 0;

        st = cpssDxChL2MllMcCntGet(dev, mllCntSet, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, mllOutMCPktsPtr = NULL", dev);
    }

    mllCntSet = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChL2MllMcCntGet(dev, mllCntSet, &mllOutMCPkts);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChL2MllMcCntGet(dev, mllCntSet, &mllOutMCPkts);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChL2MllSilentDropCntGet
(
    IN  GT_U8  devNum,
    OUT GT_U32 *silentDropPktsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChL2MllSilentDropCntGet)
{
/*
ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
1.1. Call with non-null dropPktsPtr. Expected: GT_OK.
1.2. Call with dropPktsPtr [NULL]. Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_U32                  silentDropPkts;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call with non-null dropPktsPtr. Expected: GT_OK.    */

        st = cpssDxChL2MllSilentDropCntGet(dev, &silentDropPkts);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2. Call with dropPktsPtr [NULL]. Expected: GT_BAD_PTR.*/

        st = cpssDxChL2MllSilentDropCntGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, silentDropPkts = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChL2MllSilentDropCntGet(dev, &silentDropPkts);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChL2MllSilentDropCntGet(dev, &silentDropPkts);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChL2MllLttEntrySet
(
    IN GT_U8                            devNum,
    IN GT_U32                           index,
    IN CPSS_DXCH_L2_MLL_LTT_ENTRY_STC   *lttEntryPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChL2MllLttEntrySet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with index[0, max/2, max],
                   lttEntry{
                        mllPointer[0, max/2, max],
                        mllMaskProfileEnable[GT_FALSE, GT_TRUE, GT_TRUE],
                        mllMaskProfile[0, 11, 14]
                   }
    Expected: GT_OK.
    1.2. Call cpssDxChL2MllLttEntryGet.
    Expected: GT_OK and the same values as was set.
    1.3. Call with out of range index[max+1] and other valid params.
    Expected: NON GT_OK.
    1.4. Call with out of range lttEntry.mllPointer[max+1] and other valid params.
    Expected: NON GT_OK.
    1.5. Call with out of range lttEntry.entrySelector[2] and other valid params.
    Expected: NON GT_OK.
    1.6. Call with mllMaskProfileEnable[GT_TRUE],
        out of range lttEntry.mllMaskProfile[15] (is relevant) and other valid params.
    Expected: NON GT_OK.
    1.7. Call with mllMaskProfileEnable[GT_FALSE],
        out of range lttEntry.mllMaskProfile[15] (not relevant) and other valid params.
    Expected: GT_OK.
*/

    GT_STATUS                                       st          = GT_OK;
    GT_U8                                           dev         = 0;
    GT_U32                                          index       = 0;
    CPSS_DXCH_L2_MLL_LTT_ENTRY_STC                  lttEntry    = {0};
    CPSS_DXCH_L2_MLL_LTT_ENTRY_STC                  lttEntryRet = {0};
    GT_U32                                          maxL2MllEntries;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with index[0, max/2, max],
                           lttEntry{
                                mllPointer[0, max/2, max],
                                mllMaskProfileEnable[GT_FALSE, GT_TRUE, GT_TRUE],
                                mllMaskProfile[0, 11, 14]
                            }
            Expected: GT_OK.
        */

        maxL2MllEntries =
            PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.mllPairs * 2;

        /* call with index[0] */
        index = 0;
        lttEntry.mllPointer = 0;
        lttEntry.entrySelector = CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;
        lttEntry.mllMaskProfileEnable = GT_FALSE;
        lttEntry.mllMaskProfile = 0;

        st = cpssDxChL2MllLttEntrySet(dev, index, &lttEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChL2MllLttEntryGet.
            Expected: GT_OK and the same values as was set.
        */
        st = cpssDxChL2MllLttEntryGet(dev, index, &lttEntryRet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        /* check with previous set of parms, except lttEntry.mllMaskProfile
            (with this set of parms lttEntry.mllMaskProfile is not relevant) */
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(lttEntry.mllPointer,
                                         lttEntryRet.mllPointer,
                                         "got another lttEntry.mllPointer: %d",
                                         lttEntryRet.mllPointer);
            UTF_VERIFY_EQUAL1_STRING_MAC(lttEntry.entrySelector,
                                         lttEntryRet.entrySelector,
                                         "got another lttEntry.entrySelector: %d",
                                         lttEntryRet.entrySelector);
            UTF_VERIFY_EQUAL1_STRING_MAC(lttEntry.mllMaskProfileEnable,
                                         lttEntryRet.mllMaskProfileEnable,
                                         "got another lttEntry.mllMaskProfileEnable: %d",
                                         lttEntryRet.mllMaskProfileEnable);
        }

        /* call with index[max/2] */
        index = PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_L2_LTT_MAC(dev) / 2;
        lttEntry.mllPointer = maxL2MllEntries / 2;
        lttEntry.entrySelector = CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_SECOND_MLL_E;
        lttEntry.mllMaskProfileEnable = GT_TRUE;
        lttEntry.mllMaskProfile = 11;

        st = cpssDxChL2MllLttEntrySet(dev, index, &lttEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChL2MllLttEntryGet.
            Expected: GT_OK and the same values as was set.
        */
        st = cpssDxChL2MllLttEntryGet(dev, index, &lttEntryRet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(lttEntry.mllPointer,
                                         lttEntryRet.mllPointer,
                                         "got another lttEntry.mllPointer: %d",
                                         lttEntryRet.mllPointer);
            UTF_VERIFY_EQUAL1_STRING_MAC(lttEntry.entrySelector,
                                         lttEntryRet.entrySelector,
                                         "got another lttEntry.entrySelector: %d",
                                         lttEntryRet.entrySelector);
            UTF_VERIFY_EQUAL1_STRING_MAC(lttEntry.mllMaskProfileEnable,
                                         lttEntryRet.mllMaskProfileEnable,
                                         "got another lttEntry.mllMaskProfileEnable: %d",
                                         lttEntryRet.mllMaskProfileEnable);
            UTF_VERIFY_EQUAL1_STRING_MAC(lttEntry.mllMaskProfile,
                                         lttEntryRet.mllMaskProfile,
                                         "got another lttEntry.mllMaskProfile: %d",
                                         lttEntryRet.mllMaskProfile);
        }

        /* call with index[max] */
        index = PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_L2_LTT_MAC(dev);
        lttEntry.mllPointer = maxL2MllEntries - 1;
        lttEntry.entrySelector = CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;
        lttEntry.mllMaskProfileEnable = GT_TRUE;
        lttEntry.mllMaskProfile = 14;

        st = cpssDxChL2MllLttEntrySet(dev, index, &lttEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChL2MllLttEntryGet.
            Expected: GT_OK and the same values as was set.
        */
        st = cpssDxChL2MllLttEntryGet(dev, index, &lttEntryRet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(lttEntry.mllPointer,
                                         lttEntryRet.mllPointer,
                                         "got another lttEntry.mllPointer: %d",
                                         lttEntryRet.mllPointer);
            UTF_VERIFY_EQUAL1_STRING_MAC(lttEntry.entrySelector,
                                         lttEntryRet.entrySelector,
                                         "got another lttEntry.entrySelector: %d",
                                         lttEntryRet.entrySelector);
            UTF_VERIFY_EQUAL1_STRING_MAC(lttEntry.mllMaskProfileEnable,
                                         lttEntryRet.mllMaskProfileEnable,
                                         "got another lttEntry.mllMaskProfileEnable: %d",
                                         lttEntryRet.mllMaskProfileEnable);
            UTF_VERIFY_EQUAL1_STRING_MAC(lttEntry.mllMaskProfile,
                                         lttEntryRet.mllMaskProfile,
                                         "got another lttEntry.mllMaskProfile: %d",
                                         lttEntryRet.mllMaskProfile);
        }

        /*
            1.3. Call with out of range index[max+1] and other valid params.
            Expected: NON GT_OK.
        */
        index = PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_L2_LTT_MAC(dev) + 1;
        lttEntry.mllPointer = maxL2MllEntries / 2;
        lttEntry.entrySelector = CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;
        lttEntry.mllMaskProfileEnable = GT_TRUE;
        lttEntry.mllMaskProfile = 11;

        st = cpssDxChL2MllLttEntrySet(dev, index, &lttEntry);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* restore value */
        index = PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_L2_LTT_MAC(dev);


        /*
            1.4. Call with out of range entrySelector[2] and other valid params.
            Expected: NON GT_OK.
        */
        index = PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_L2_LTT_MAC(dev);
        lttEntry.mllPointer = maxL2MllEntries / 2;
        lttEntry.entrySelector = 2;
        lttEntry.mllMaskProfileEnable = GT_TRUE;
        lttEntry.mllMaskProfile = 11;

        st = cpssDxChL2MllLttEntrySet(dev, index, &lttEntry);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* restore value */
        lttEntry.entrySelector = CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;

        /*
            1.5. Call with out of range lttEntry.mllPointer[max+1] and
                other valid params.
            Expected: NON GT_OK.
        */
        lttEntry.mllPointer = maxL2MllEntries;
        lttEntry.entrySelector = CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;

        st = cpssDxChL2MllLttEntrySet(dev, index, &lttEntry);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* restore value */
        lttEntry.mllPointer = maxL2MllEntries / 2;
        lttEntry.entrySelector = CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;

        /*
            1.6. Call with mllMaskProfileEnable[GT_TRUE],
                out of range lttEntry.mllMaskProfile[15] (is relevant) and
                other valid params.
            Expected: NON GT_OK.
        */
        lttEntry.mllMaskProfile = 15;

        st = cpssDxChL2MllLttEntrySet(dev, index, &lttEntry);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* restore value */
        lttEntry.mllMaskProfile = 11;

        /*
            1.7. Call with mllMaskProfileEnable[GT_FALSE],
                out of range lttEntry.mllMaskProfile[15] (not relevant) and
                other valid params.
            Expected: GT_OK.
        */
        lttEntry.mllMaskProfile = 15;
        lttEntry.mllMaskProfileEnable = GT_FALSE;

        st = cpssDxChL2MllLttEntrySet(dev, index, &lttEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* restore values */
        lttEntry.mllMaskProfile = 11;
        lttEntry.mllMaskProfileEnable = GT_TRUE;
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChL2MllLttEntrySet(dev, index, &lttEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChL2MllLttEntrySet(dev, index, &lttEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChL2MllLttEntryGet
(
    IN  GT_U8                            devNum,
    IN  GT_U32                           index,
    OUT CPSS_DXCH_L2_MLL_LTT_ENTRY_STC   *lttEntryPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChL2MllLttEntryGet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with index[0, max/2, max], non-null lttEntry.
    Expected: GT_OK.
    1.2. Call with out of range index[max+1] and other valid params.
    Expected: NON GT_OK.
    1.3. Call with out of range lttEntryPtr and other valid params.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                                       st          = GT_OK;
    GT_U8                                           dev         = 0;
    GT_U32                                          index       = 0;
    CPSS_DXCH_L2_MLL_LTT_ENTRY_STC                  lttEntry    = {0};

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with index[0, max/2, max], non-null lttEntry.
            Expected: GT_OK.
        */

        /* call with index[0] */
        index = 0;
        st = cpssDxChL2MllLttEntryGet(dev, index, &lttEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* call with index[max/2] */
        index = PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_L2_LTT_MAC(dev)/2;
        st = cpssDxChL2MllLttEntryGet(dev, index, &lttEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* call with index[max] */
        index = PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_L2_LTT_MAC(dev);
        st = cpssDxChL2MllLttEntryGet(dev, index, &lttEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with out of range index[max+1] and other valid params.
            Expected: NON GT_OK.
        */
        index = PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_L2_LTT_MAC(dev) + 1;
        st = cpssDxChL2MllLttEntryGet(dev, index, &lttEntry);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* restore value */
        index = PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_L2_LTT_MAC(dev);

        /*
            1.3. Call with out of range schedulingModePtr[NULL] and
                other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChL2MllLttEntryGet(dev, index, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* restore value */
    index = 0xA5A5;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChL2MllLttEntryGet(dev, index, &lttEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChL2MllLttEntryGet(dev, index, &lttEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChL2MllPortGroupCounterGet
(
    IN  GT_U8               devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    IN  GT_U32              index,
    OUT GT_U32              *counterPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChL2MllPortGroupCounterGet)
{
/*
    ITERATE_DEVICES_PORT_GROUPS (SIP5)
    1.1.1. Call with index[0, 1, 2] and non-null counterPtr.
    Expected: GT_OK.
    1.1.2. Call with out of range index[3] and other valid params.
    Expected: NON GT_OK.
    1.1.3. Call with out of range countersPtr [NULL] and other valid params.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                               st                  = GT_OK;
    GT_U8                                   dev                 = 0;
    GT_U32                                  portGroupId         = 0;
    GT_PORT_GROUPS_BMP                      portGroupsBmp       = 1;
    GT_U32                                  index               = 0;
    GT_U32                                  counter             = 0;
    GT_STATUS   expectedSt;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_FALSE && PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == GT_FALSE)
        {
            expectedSt = GT_NOT_APPLICABLE_DEVICE;
        }
        else
        {
            expectedSt = GT_OK;
        }

        /* initialize port group */
        portGroupsBmp = 1;

        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set non-active port group bitmap */
            portGroupsBmp = (1 << portGroupId);
            /*
                1.1.1. Call with index[0, 1, 2] and non-null counterPtr.
                Expected: GT_OK.
            */

            /* call with index [0] */
            index = 0;

            st = cpssDxChL2MllPortGroupCounterGet(dev, portGroupsBmp, index, &counter);
            UTF_VERIFY_EQUAL2_PARAM_MAC(expectedSt, st, dev, portGroupsBmp);

            /* call with index [1] */
            index = 1;

            st = cpssDxChL2MllPortGroupCounterGet(dev, portGroupsBmp, index, &counter);
            UTF_VERIFY_EQUAL2_PARAM_MAC(expectedSt, st, dev, portGroupsBmp);

            /* call with index [2] */
            index = 2;

            st = cpssDxChL2MllPortGroupCounterGet(dev, portGroupsBmp, index, &counter);
            UTF_VERIFY_EQUAL2_PARAM_MAC(expectedSt, st, dev, portGroupsBmp);

            /* restore value */
            index = 1;

            /*
                1.1.2. Call with out of range countersPtr [NULL] and
                    other valid params.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChL2MllPortGroupCounterGet(dev, portGroupsBmp, index, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev, portGroupId)

        /* restore port group */
        portGroupsBmp = 1;

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set non-active port group bitmap */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChL2MllPortGroupCounterGet(dev, portGroupsBmp, index, &counter);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev, portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        st = cpssDxChL2MllPortGroupCounterGet(dev, portGroupsBmp, index, &counter);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expectedSt, st, dev, portGroupsBmp);
    }

    portGroupsBmp = 1;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );


    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChL2MllPortGroupCounterGet(dev, portGroupsBmp, index, &counter);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChL2MllPortGroupCounterGet(dev, portGroupsBmp, index, &counter);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChL2MllPortGroupExceptionCountersGet
(
    IN  GT_U8                                   devNum,
    IN GT_PORT_GROUPS_BMP                       portGroupsBmp,
    OUT CPSS_DXCH_L2_MLL_EXCEPTION_COUNTERS_STC  *countersPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChL2MllPortGroupExceptionCountersGet)
{
/*
    ITERATE_DEVICES_PORT_GROUPS (SIP5)
    1.1.1. Call with non-null countersPtr.
    Expected: GT_OK.
    1.1.2. Call with out of range countersPtr [NULL] and other valid params.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                               st                  = GT_OK;
    GT_U8                                   dev                 = 0;
    GT_U32                                  portGroupId         = 0;
    GT_PORT_GROUPS_BMP                      portGroupsBmp       = 1;
    CPSS_DXCH_L2_MLL_EXCEPTION_COUNTERS_STC counters;
    GT_STATUS   expectedSt;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_FALSE && PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == GT_FALSE)
        {
            expectedSt = GT_NOT_APPLICABLE_DEVICE;
        }
        else
        {
            expectedSt = GT_OK;
        }

        /* initialize port group */
        portGroupsBmp = 1;

        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set non-active port group bitmap */
            portGroupsBmp = (1 << portGroupId);

            /*
                1.1.1. Call with non-null countersPtr.
                Expected: GT_OK.
            */
            st = cpssDxChL2MllPortGroupExceptionCountersGet(dev, portGroupsBmp,
                                                            &counters);
            UTF_VERIFY_EQUAL2_PARAM_MAC(expectedSt, st, dev, portGroupsBmp);

            /*
                1.1.2. Call with out of range countersPtr [NULL] and
                    other valid params.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChL2MllPortGroupExceptionCountersGet(dev, portGroupsBmp,
                                                            NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev, portGroupId)

        /* restore port group */
        portGroupsBmp = 1;

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set non-active port group bitmap */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChL2MllPortGroupExceptionCountersGet(dev, portGroupsBmp,
                                                            &counters);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev, portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        st = cpssDxChL2MllPortGroupExceptionCountersGet(dev, portGroupsBmp,
                                                        &counters);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expectedSt, st, dev, portGroupsBmp);
    }

    portGroupsBmp = 1;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );


    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChL2MllPortGroupExceptionCountersGet(dev, portGroupsBmp,
                                                        &counters);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChL2MllPortGroupExceptionCountersGet(dev, portGroupsBmp,
                                                    &counters);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}



/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChL2MllTtlExceptionConfigurationSet
(
    IN GT_U8                    devNum,
    IN GT_BOOL                  trapEnable,
    IN CPSS_NET_RX_CPU_CODE_ENT cpuCode
)
*/
UTF_TEST_CASE_MAC(cpssDxChL2MllTtlExceptionConfigurationSet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with trapEnable [GT_FALSE, GT_TRUE, GT_TRUE],
                   cpuCode[CPSS_NET_CONTROL_E,
                           CPSS_NET_EGRESS_MIRRORED_TO_ANLYZER_E,
                           CPSS_NET_FIRST_USER_DEFINED_E].
    Expected: GT_OK.
    1.2. Call cpssDxChL2MllTtlExceptionConfigurationGet.
    Expected: GT_OK and the same values as was set.
    1.3. Call with out of range cpuCode and other valid params.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                       st                  = GT_OK;
    GT_U8                           dev                 = 0;
    GT_BOOL                         trapEnable          = GT_FALSE;
    GT_BOOL                         tmpTrapEnable       = GT_FALSE;
    CPSS_NET_RX_CPU_CODE_ENT        cpuCode             = 0;
    CPSS_NET_RX_CPU_CODE_ENT        tmpCpuCode          = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with trapEnable [GT_FALSE, GT_TRUE, GT_TRUE],
                           cpuCode[CPSS_NET_CONTROL_E,
                                   CPSS_NET_EGRESS_MIRRORED_TO_ANLYZER_E,
                                   CPSS_NET_FIRST_USER_DEFINED_E].
            Expected: GT_OK.
        */

        /* call with cpuCode[CPSS_NET_CONTROL_E] */
        cpuCode = CPSS_NET_CONTROL_E;

        st = cpssDxChL2MllTtlExceptionConfigurationSet(dev, trapEnable, cpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* check with previous set of parms */
        st = cpssDxChL2MllTtlExceptionConfigurationGet(dev,
                                                       &tmpTrapEnable,
                                                       &tmpCpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(trapEnable, tmpTrapEnable,
                                         "got another trapEnable: %d",
                                         tmpTrapEnable);

            UTF_VERIFY_EQUAL1_STRING_MAC(cpuCode, tmpCpuCode,
                                         "got another cpuCode: %d",
                                         tmpCpuCode);
        }

        /* call with cpuCode[CPSS_NET_EGRESS_MIRRORED_TO_ANLYZER_E] */
        cpuCode = CPSS_NET_EGRESS_MIRRORED_TO_ANLYZER_E;

        st = cpssDxChL2MllTtlExceptionConfigurationSet(dev, trapEnable, cpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* check with previous set of parms */
        st = cpssDxChL2MllTtlExceptionConfigurationGet(dev,
                                                       &tmpTrapEnable,
                                                       &tmpCpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(trapEnable, tmpTrapEnable,
                                         "got another trapEnable: %d",
                                         tmpTrapEnable);

            UTF_VERIFY_EQUAL1_STRING_MAC(cpuCode, tmpCpuCode,
                                         "got another cpuCode: %d",
                                         tmpCpuCode);
        }

        /* call with cpuCode [CPSS_NET_FIRST_USER_DEFINED_E] */
        cpuCode = CPSS_NET_FIRST_USER_DEFINED_E;

        st = cpssDxChL2MllTtlExceptionConfigurationSet(dev, trapEnable, cpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChL2MllTtlExceptionConfigurationGet.
            Expected: GT_OK and the same values as was set.
        */
        st = cpssDxChL2MllTtlExceptionConfigurationGet(dev,
                                                       &tmpTrapEnable,
                                                       &tmpCpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(trapEnable, tmpTrapEnable,
                                         "got another trapEnable: %d",
                                         tmpTrapEnable);

            UTF_VERIFY_EQUAL1_STRING_MAC(cpuCode, tmpCpuCode,
                                         "got another cpuCode: %d",
                                         tmpCpuCode);
        }

        /* restore value */
        cpuCode = CPSS_NET_CONTROL_E;

        /*
            1.3. Call with out of range cpuCode and other valid params.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChL2MllTtlExceptionConfigurationSet
                            (dev, trapEnable, cpuCode),
                            cpuCode);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* restore values */
    cpuCode = CPSS_NET_CONTROL_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChL2MllTtlExceptionConfigurationSet(dev, trapEnable, cpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChL2MllTtlExceptionConfigurationSet(dev, trapEnable, cpuCode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChL2MllTtlExceptionConfigurationGet
(
    IN  GT_U8                    devNum,
    OUT GT_BOOL                  *trapEnablePtr,
    OUT CPSS_NET_RX_CPU_CODE_ENT *cpuCodePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChL2MllTtlExceptionConfigurationGet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with non-null trapEnablePtr, non-null cpuCodePtr.
    Expected: GT_OK.
    1.2. Call with out of range trapEnablePtr[NULL] and other valid params.
    Expected: NON GT_OK.
    1.3. Call with out of range cpuCodePtr[NULL] and other valid params.
    Expected: NON GT_OK.
*/
    GT_STATUS                       st              = GT_OK;
    GT_U8                           dev             = 0;
    GT_BOOL                         trapEnable      = GT_FALSE;
    CPSS_NET_RX_CPU_CODE_ENT        cpuCode         = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null trapEnablePtr, non-null cpuCodePtr.
            Expected: GT_OK.
        */

        st = cpssDxChL2MllTtlExceptionConfigurationGet(dev, &trapEnable, &cpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with out of range trapEnablePtr[NULL] and
                other valid params.
            Expected: NON GT_OK.
        */
        st = cpssDxChL2MllTtlExceptionConfigurationGet(dev, NULL, &cpuCode);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChL2MllTtlExceptionConfigurationGet: %d",
                                         dev);

        /*
            1.3. Call with out of range cpuCodePtr[NULL] and other valid params.
            Expected: NON GT_OK.
        */
        st = cpssDxChL2MllTtlExceptionConfigurationGet(dev, &trapEnable, NULL);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChL2MllTtlExceptionConfigurationGet: %d",
                                         dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChL2MllTtlExceptionConfigurationGet(dev, &trapEnable, &cpuCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChL2MllTtlExceptionConfigurationGet(dev, &trapEnable, &cpuCode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChL2MllMultiTargetPortEnableSet
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChL2MllMultiTargetPortEnableSet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1. Call with enable[GT_FALSE, GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChL2MllMultiTargetPortEnableGet.
    Expected: GT_OK and the same values as was set.
*/
    GT_STATUS                               st          = GT_OK;
    GT_U8                                   dev         = 0;
    GT_BOOL                                 enable      = GT_FALSE;
    GT_BOOL                                 enableRet   = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with enable[GT_FALSE, GT_TRUE].
            Expected: GT_OK.
        */

        /* call with enable[GT_FALSE] */
        enable = GT_FALSE;

        st = cpssDxChL2MllMultiTargetPortEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChL2MllMultiTargetPortEnableGet.
                Expected: GT_OK and the same values as was set.
        */
        st = cpssDxChL2MllMultiTargetPortEnableGet(dev, &enableRet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableRet,
                                         "got another enable: %d", enableRet);
        }

        /* call with enable[GT_TRUE] */
        enable = GT_TRUE;

        st = cpssDxChL2MllMultiTargetPortEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChL2MllMultiTargetPortEnableGet.
                Expected: GT_OK and the same values as was set.
        */
        st = cpssDxChL2MllMultiTargetPortEnableGet(dev, &enableRet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableRet,
                                         "got another enable: %d", enableRet);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChL2MllMultiTargetPortEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChL2MllMultiTargetPortEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChL2MllMultiTargetPortEnableGet
(
    IN GT_U8    devNum,
    OUT GT_BOOL *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChL2MllMultiTargetPortEnableGet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1. Call with non-null enablePtr.
    Expected: GT_OK.
    1.2. Call with out of range enablePtr[NULL] and other valid params.
    Expected: NON GT_OK.
*/
    GT_STATUS                               st              = GT_OK;
    GT_U8                                   dev             = 0;
    GT_BOOL                                 enable          = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChL2MllMultiTargetPortEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
        1.2. Call with out of range enablePtr[NULL] and other valid params.
        Expected: NON GT_OK.
        */
        st = cpssDxChL2MllMultiTargetPortEnableGet(dev, NULL);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChL2MllMultiTargetPortEnableGet: %d", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChL2MllMultiTargetPortEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChL2MllMultiTargetPortEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChL2MllMultiTargetPortSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      value,
    IN  GT_U32                      mask
)
*/
UTF_TEST_CASE_MAC(cpssDxChL2MllMultiTargetPortSet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1. Call with values and masks
    Expected: GT_OK.
    1.2. Call cpssDxChL2MllMultiTargetPortEnableGet.
    Expected: GT_OK and the same values as was set.
*/
    GT_STATUS                               st          = GT_OK;
    GT_U8                                   dev         = 0;
    GT_U32                                  value       = 0;
    GT_U32                                  valueRet;
    GT_U32                                  mask        = 0;
    GT_U32                                  maskRet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        value = mask = 0;
        st = cpssDxChL2MllMultiTargetPortSet(dev, value, mask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChL2MllMultiTargetPortGet(dev, &valueRet, &maskRet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(value, valueRet,
                                         "got another value: %d", valueRet);
            UTF_VERIFY_EQUAL1_STRING_MAC(mask, maskRet,
                                         "got another mask: %d", maskRet);
        }

        value = 0x600;
        mask = 0x1234;
        st = cpssDxChL2MllMultiTargetPortSet(dev, value, mask);

        if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            /* expected error:
            ERROR GT_BAD_PARAM in function: prvCpssDxChSip6GlobalEportMaskCheck,
            file cpssdxchcfginit.c, line[4504]. the MASK[0x01234] must be with continues bits
            */
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            st = cpssDxChL2MllMultiTargetPortGet(dev, &valueRet, &maskRet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            if (GT_OK == st)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(value, valueRet,
                                             "got another value: %d", valueRet);
                UTF_VERIFY_EQUAL1_STRING_MAC(mask, maskRet,
                                             "got another mask: %d", maskRet);
            }
        }

        value = mask = 0x1ffff;
        st = cpssDxChL2MllMultiTargetPortSet(dev, value, mask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChL2MllMultiTargetPortGet(dev, &valueRet, &maskRet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(value, valueRet,
                                         "got another value: %d", valueRet);
            UTF_VERIFY_EQUAL1_STRING_MAC(mask, maskRet,
                                         "got another mask: %d", maskRet);
        }

        value = 0x20000;
        st = cpssDxChL2MllMultiTargetPortSet(dev, value, mask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        value = 0x1ffff;
        mask = 0x20000;
        st = cpssDxChL2MllMultiTargetPortSet(dev, value, mask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        mask = 0x1ffff;
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChL2MllMultiTargetPortSet(dev, value, mask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChL2MllMultiTargetPortSet(dev, value, mask);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChL2MllMultiTargetPortGet
(
    IN  GT_U8                       devNum,
    OUT GT_U32                      *valuePtr,
    OUT GT_U32                      *maskPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChL2MllMultiTargetPortGet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1. Call with non-null valuePtr, maskPtr.
    Expected: GT_OK
    1.2. Call with null valuePtr[NULL], maskPtr[NULL] and other valid params.
    Expected: GT_BAD_PTR
*/
    GT_STATUS                               st              = GT_OK;
    GT_U8                                   dev             = 0;
    GT_U32                                  value           = 0;
    GT_U32                                  mask            = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null valuePtr, maskPtr
            Expected: GT_OK.
        */
        st = cpssDxChL2MllMultiTargetPortGet(dev, &value, &mask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
        1.2. Call with out of range valuePtr[NULL] and other valid params
        Expected: GT_BAD_PTR
        */
        st = cpssDxChL2MllMultiTargetPortGet(dev, NULL, &mask);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                     "cpssDxChL2MllMultiTargetPortGet: %d", dev);

        /*
        1.3. Call with out of range maskPtr[NULL] and other valid params
        Expected: GT_BAD_PTR
        */
        st = cpssDxChL2MllMultiTargetPortGet(dev, &value, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                     "cpssDxChL2MllMultiTargetPortGet: %d", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChL2MllMultiTargetPortGet(dev, &value, &mask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChL2MllMultiTargetPortGet(dev, &value, &mask);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChL2MllMultiTargetPortBaseSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      portBase
)
*/
UTF_TEST_CASE_MAC(cpssDxChL2MllMultiTargetPortBaseSet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1. Call with portBase
    Expected: GT_OK.
    1.2. Call cpssDxChL2MllMultiTargetPortBaseGet.
    Expected: GT_OK and the same values as was set.
*/
    GT_STATUS                               st          = GT_OK;
    GT_U8                                   dev         = 0;
    GT_U32                                  portBase    = 0;
    GT_U32                                  portBaseRet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        portBase = 0;
        st = cpssDxChL2MllMultiTargetPortBaseSet(dev, portBase);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChL2MllMultiTargetPortBaseGet(dev, &portBaseRet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(portBase, portBaseRet,
                                         "got another port base: %d", portBaseRet);
        }

        portBase = 0x10000;
        st = cpssDxChL2MllMultiTargetPortBaseSet(dev, portBase);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChL2MllMultiTargetPortBaseGet(dev, &portBaseRet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(portBase, portBaseRet,
                                         "got another port base: %d", portBaseRet);
        }

        portBase = 0x1ffff;
        st = cpssDxChL2MllMultiTargetPortBaseSet(dev, portBase);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChL2MllMultiTargetPortBaseGet(dev, &portBaseRet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(portBase, portBaseRet,
                                         "got another port base: %d", portBaseRet);
        }

        portBase = 0x20000;
        st = cpssDxChL2MllMultiTargetPortBaseSet(dev, portBase);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        portBase = 0x1ffff;
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChL2MllMultiTargetPortBaseSet(dev, portBase);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChL2MllMultiTargetPortBaseSet(dev, portBase);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChL2MllMultiTargetPortBaseGet
(
    IN  GT_U8                       devNum,
    OUT GT_U32                      *portBasePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChL2MllMultiTargetPortBaseGet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1. Call with non-null portBasePtr.
    Expected: GT_OK
    1.2. Call with null portBasePtr[NULL] and other valid params.
    Expected: GT_BAD_PTR
*/
    GT_STATUS                               st              = GT_OK;
    GT_U8                                   dev             = 0;
    GT_U32                                  portBase        = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null portBasePtr
            Expected: GT_OK.
        */
        st = cpssDxChL2MllMultiTargetPortBaseGet(dev, &portBase);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
        1.2. Call with out of range portBasePtr[NULL] and other valid params
        Expected: GT_BAD_PTR
        */
        st = cpssDxChL2MllMultiTargetPortBaseGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                     "cpssDxChL2MllMultiTargetPortBaseGet: %d", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChL2MllMultiTargetPortBaseGet(dev, &portBase);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChL2MllMultiTargetPortBaseGet(dev, &portBase);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChL2MllPortToVidxBaseSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      vidxBase
)
*/
UTF_TEST_CASE_MAC(cpssDxChL2MllPortToVidxBaseSet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1. Call with vidxBase
    Expected: GT_OK.
    1.2. Call cpssDxChL2MllPortToVidxBaseGet.
    Expected: GT_OK and the same values as was set.
*/
    GT_STATUS                               st          = GT_OK;
    GT_U8                                   dev         = 0;
    GT_U32                                  vidxBase    = 0;
    GT_U32                                  vidxBaseRet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        vidxBase = 0;
        st = cpssDxChL2MllPortToVidxBaseSet(dev, vidxBase);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChL2MllPortToVidxBaseGet(dev, &vidxBaseRet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(vidxBase, vidxBaseRet,
                                         "got another vidx base: %d", vidxBaseRet);
        }

        vidxBase = 0x1000;
        st = cpssDxChL2MllPortToVidxBaseSet(dev, vidxBase);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChL2MllPortToVidxBaseGet(dev, &vidxBaseRet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(vidxBase, vidxBaseRet,
                                         "got another vidx base: %d", vidxBaseRet);
        }

        vidxBase = 0xffff;
        st = cpssDxChL2MllPortToVidxBaseSet(dev, vidxBase);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChL2MllPortToVidxBaseGet(dev, &vidxBaseRet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        if (GT_OK == st)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(vidxBase, vidxBaseRet,
                                         "got another vidx base: %d", vidxBaseRet);
        }

        vidxBase = 0x10000;
        st = cpssDxChL2MllPortToVidxBaseSet(dev, vidxBase);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        vidxBase = 0xffff;
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChL2MllPortToVidxBaseSet(dev, vidxBase);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChL2MllPortToVidxBaseSet(dev, vidxBase);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChL2MllPortToVidxBaseGet
(
    IN  GT_U8                       devNum,
    OUT GT_U32                      *vidxBasePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChL2MllPortToVidxBaseGet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1. Call with non-null vidxBasePtr.
    Expected: GT_OK
    1.2. Call with null vidxBasePtr[NULL] and other valid params.
    Expected: GT_BAD_PTR
*/
    GT_STATUS                               st              = GT_OK;
    GT_U8                                   dev             = 0;
    GT_U32                                  vidxBase        = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null vidxBasePtr
            Expected: GT_OK.
        */
        st = cpssDxChL2MllPortToVidxBaseGet(dev, &vidxBase);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
        1.2. Call with out of range vidxBasePtr[NULL] and other valid params
        Expected: GT_BAD_PTR
        */
        st = cpssDxChL2MllPortToVidxBaseGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                     "cpssDxChL2MllPortToVidxBaseGet: %d", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChL2MllPortToVidxBaseGet(dev, &vidxBase);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChL2MllPortToVidxBaseGet(dev, &vidxBase);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
(
    GT_U8                    devNum,
    GT_BOOL                  enable,
    GT_U32                   meshIdOffset,
    GT_U32                   meshIdSize
)
*/
UTF_TEST_CASE_MAC(cpssDxChL2MllSourceBasedFilteringConfigurationSet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1. Call with valid values of enable[GT_TRUE],
                                   meshIdOffset [0 / 6 / 11],
                                   meshIdSize [1 / 4 / 8]
    Expected: GT_OK and the same values it was set.
    1.2. Call function with out of range meshIdOffset [12] and other parameters
         same as in 1.1
    Expected: GT_BAD_PARAM.
    1.3. Call function with out of range meshIdSize [9] and other parameters
         same as in 1.1
    Expected: GT_BAD_PARAM.
*/

    GT_STATUS   st                 = GT_OK;
    GT_U8       devNum             = 0;
    GT_BOOL     enable             = GT_FALSE;
    GT_U32      meshIdOffset       = 0;
    GT_U32      meshIdSize         = 0;
    GT_BOOL     enableGet          = GT_FALSE;
    GT_U32      meshIdOffsetGet    = 0;
    GT_U32      meshIdSizeGet      = 0;
    GT_BOOL     notAppFamilyBmp    = 0;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with valid values of enable[GT_TRUE],
                                           meshIdOffset [0 / 6 / 11]
                                           meshIdSize [1 / 4 / 8]
            Expected: GT_OK and the same values it was set.
        */

        /*
           1.1.1 Call with enable[GT_TRUE], meshIdOffset [0], meshIdSize [1]
           Expected: GT_OK.
        */
        enable       = GT_TRUE;
        meshIdOffset = 0;
        meshIdSize   = 1;
        st = cpssDxChL2MllSourceBasedFilteringConfigurationSet(devNum, enable,
                                                               meshIdOffset,
                                                               meshIdSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
            1.1.1. Call cpssDxChL2MllSourceBasedFilteringConfigurationGet
            Expected: GT_OK and the same values it was set.
        */
        st = cpssDxChL2MllSourceBasedFilteringConfigurationGet(devNum,
                                                               &enableGet,
                                                               &meshIdOffsetGet,
                                                               &meshIdSizeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                     "cpssDxChL2MllSourceBasedFilteringConfigurationGet: %d %d",
                     devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(meshIdOffset, meshIdOffsetGet,
                     "cpssDxChL2MllSourceBasedFilteringConfigurationGet: %d %d",
                     devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(meshIdSize, meshIdSizeGet,
                     "cpssDxChL2MllSourceBasedFilteringConfigurationGet: %d %d",
                     devNum);

        /*
           1.1.2 Call with enable[GT_TRUE], meshIdOffset [6], meshIdSize [4]
           Expected: GT_OK.
        */
        meshIdOffset = 6;
        meshIdSize   = 4;
        st = cpssDxChL2MllSourceBasedFilteringConfigurationSet(devNum, enable,
                                                               meshIdOffset,
                                                               meshIdSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
           1.1.2. Call cpssDxChL2MllSourceBasedFilteringConfigurationGet
           Expected: GT_OK and the same values it was set.
        */
        st = cpssDxChL2MllSourceBasedFilteringConfigurationGet(devNum,
                                                               &enableGet,
                                                               &meshIdOffsetGet,
                                                               &meshIdSizeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                     "cpssDxChL2MllSourceBasedFilteringConfigurationGet: %d %d",
                     devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(meshIdOffset, meshIdOffsetGet,
                     "cpssDxChL2MllSourceBasedFilteringConfigurationGet: %d %d",
                     devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(meshIdSize, meshIdSizeGet,
                     "cpssDxChL2MllSourceBasedFilteringConfigurationGet: %d %d",
                     devNum);

        /*
           1.1.3 Call with enable[GT_TRUE], meshIdOffset [11], meshIdSize [8]
           Expected: GT_OK.
        */
        meshIdOffset = 11;
        meshIdSize   = 8;
        st = cpssDxChL2MllSourceBasedFilteringConfigurationSet(devNum, enable,
                                                               meshIdOffset,
                                                               meshIdSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
           1.1.3. Call cpssDxChL2MllSourceBasedFilteringConfigurationGet
           Expected: GT_OK and the same values it was set.
        */
        st = cpssDxChL2MllSourceBasedFilteringConfigurationGet(devNum,
                                                               &enableGet,
                                                               &meshIdOffsetGet,
                                                               &meshIdSizeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                     "cpssDxChL2MllSourceBasedFilteringConfigurationGet: %d %d",
                     devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(meshIdOffset, meshIdOffsetGet,
                     "cpssDxChL2MllSourceBasedFilteringConfigurationGet: %d %d",
                     devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(meshIdSize, meshIdSizeGet,
                     "cpssDxChL2MllSourceBasedFilteringConfigurationGet: %d %d",
                     devNum);

        /*
            1.2. Call function with out of range meshIdOffset [12] and
                 other parameters same as in 1.1
        */
        meshIdOffset = 12;
        meshIdSize   = 1;
        st = cpssDxChL2MllSourceBasedFilteringConfigurationSet(devNum, enable,
                                                               meshIdOffset,
                                                               meshIdSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, devNum);

        /*
            1.3. Call function with out of range meshIdSize [9] and
                 other parameters same as in 1.1
            Expected: GT_OUT_OF_RANGE.
        */
        meshIdOffset = 0;
        meshIdSize   = 9;
        st = cpssDxChL2MllSourceBasedFilteringConfigurationSet(devNum, enable,
                                                               meshIdOffset,
                                                               meshIdSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, devNum);

    }

    /*
        2. For not-active devices and devices from non-applicable family
        check that function returns GT_NOT_APPLICABLE_DEVICE.
    */

    /* restore correct values */
    enable       = GT_TRUE;
    meshIdOffset = 0;
    meshIdSize   = 1;

    /* prepare iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChL2MllSourceBasedFilteringConfigurationSet(devNum, enable,
                                                               meshIdOffset,
                                                               meshIdSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id.*/
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChL2MllSourceBasedFilteringConfigurationSet(devNum, enable,
                                                           meshIdOffset,
                                                           meshIdSize);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
(
    GT_U8                    devNum,
    GT_BOOL                  *enablePtr,
    GT_U32                   *meshIdOffsetPtr,
    GT_U32                   *meshIdSizePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChL2MllSourceBasedFilteringConfigurationGet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1. Call function with valid values of enable [non-NULL]
                                            meshIdOffset [non-NULL]
                                            meshIdSize [non-NULL]
    Expected: GT_OK and the same values it was set.
    1.2. Call function with invalid enablePtr [NULL] and other
         parameters same as in 1.1
    Expected: GT_BAD_PTR.
    1.3. Call function with invalid meshIdOffsetPtr [NULL] and other
         parameters same as in 1.1
    Expected: GT_BAD_PTR.
    1.4. Call function with invalid meshIdSizePtr [NULL] and other
         parameters same as in 1.1
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st              = GT_OK;
    GT_U8       devNum          = 0;
    GT_BOOL     enable          = GT_FALSE;
    GT_U32      meshIdOffset    = 0;
    GT_U32      meshIdSize      = 0;
    GT_BOOL     notAppFamilyBmp = 0;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call function with enable [non-NULL]
                                    meshIdOffset [non-NULL]
                                    meshIdSize [non-NULL]
            Expected: GT_OK.
        */

        st = cpssDxChL2MllSourceBasedFilteringConfigurationGet(devNum, &enable,
                                                               &meshIdOffset,
                                                               &meshIdSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
            1.2. Call function with invalid enablePtr [NULL] and other
                 parameters same as in 1.1
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChL2MllSourceBasedFilteringConfigurationGet(devNum, NULL,
                                                               &meshIdOffset,
                                                               &meshIdSize);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                        "cpssDxChL2MllSourceBasedFilteringConfigurationGet: %d",
                        devNum);

        /*
            1.3. Call function with invalid meshIdOffsetPtr [NULL] and other
                 parameters same as in 1.1
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChL2MllSourceBasedFilteringConfigurationGet(devNum, &enable,
                                                               NULL,
                                                               &meshIdSize);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                        "cpssDxChL2MllSourceBasedFilteringConfigurationGet: %d",
                        devNum);

        /*
            1.4. Call function with invalid meshIdSizePtr [NULL] and other
                 parameters same as in 1.1
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChL2MllSourceBasedFilteringConfigurationGet(devNum, &enable,
                                                               &meshIdOffset,
                                                               NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                        "cpssDxChL2MllSourceBasedFilteringConfigurationGet: %d",
                        devNum);
    }

    /*
       2. For not-active devices and devices from non-applicable family
          check that function returns GT_NOT_APPLICABLE_DEVICE.
    */

    /* prepare iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChL2MllSourceBasedFilteringConfigurationGet(devNum, &enable,
                                                               &meshIdOffset,
                                                               &meshIdSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id.*/
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChL2MllSourceBasedFilteringConfigurationGet(devNum, &enable,
                                                           &meshIdOffset,
                                                           &meshIdSize);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChL2MllPortGroupMcCntGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    IN  GT_U32                  mllCntSet,
    OUT GT_U32                  *mllOutMCPktsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChL2MllPortGroupMcCntGet)
{
    /*
        ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
        1.1.1. Call with mllCntSet[0,1] and valid mllOutMCPktsPtr[NON-NULL].
        Expected: GT_OK.
        1.1.2. Call with out of range mllCntSet[2] and
             other values same as 1.1.1.
        Expected: GT_BAD_PARAM.
        1.1.3. Call with out of range silentDropPktsPtr[NULL] and
             other values same as 1.1.1.
        Expected: GT_BAD_PTR.
    */

        GT_STATUS           st                  = GT_OK;
        GT_U8               dev                 = 0;
        GT_U32              notAppFamilyBmp     = 0;
        GT_U32              portGroupId         = 0;
        GT_PORT_GROUPS_BMP  portGroupsBmp       = 1;
        GT_U32              mllCntSet           = 0;
        GT_U32              mllOutMCPkts        = 0;

        /* ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3) */
        UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

        /* 1. Go over all active devices. */
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {

            /* 1.1. Go over all active port groups. */
            PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
            {
                /* set next active port */
                portGroupsBmp = (1 << portGroupId);

                /*
                    1.1.1. Call with mllCntSet[0] and valid
                        mllOutMCPktsPtr[NON-NULL].
                    Expected: GT_OK.
                */
                mllCntSet = 0;
                st = cpssDxChL2MllPortGroupMcCntGet(dev, portGroupsBmp,
                                                mllCntSet, &mllOutMCPkts);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /*
                    1.1.1. Call with mllCntSet[1] and valid
                        mllOutMCPktsPtr[NON-NULL].
                    Expected: GT_OK.
                */
                mllCntSet = 1;
                st = cpssDxChL2MllPortGroupMcCntGet(dev, portGroupsBmp,
                                                mllCntSet, &mllOutMCPkts);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /*
                    1.1.2. Call with out of range mllCntSet[2] and
                         other values same as 1.1.1.
                    Expected: GT_BAD_PARAM.
                */
                mllCntSet = 2;
                st = cpssDxChL2MllPortGroupMcCntGet(dev, portGroupsBmp,
                                                mllCntSet, &mllOutMCPkts);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

                /* restore valid value */
                mllCntSet = 1;

                /*
                    1.1.3. Call with out of range silentDropPktsPtr[NULL] and
                         other values same as 1.1.1.
                    Expected: GT_BAD_PTR.
                */
                st = cpssDxChL2MllPortGroupMcCntGet(dev, portGroupsBmp,
                                                mllCntSet, NULL);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

            }
            PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

            /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
            PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
            {
                /* set next non-active port */
                portGroupsBmp = (1 << portGroupId);

                st = cpssDxChL2MllPortGroupMcCntGet(dev, portGroupsBmp,
                                                mllCntSet, &mllOutMCPkts);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
            }
            PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)

            /* 1.3. For unaware port groups check that function returns GT_OK. */
            portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
            st = cpssDxChL2MllPortGroupMcCntGet(dev, portGroupsBmp,
                                            mllCntSet, &mllOutMCPkts);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
        }

        /* 2. For not-active devices and devices from non-applicable family */
        /* check that function returns GT_BAD_PARAM.                        */

        /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

        /* go over all non active devices */
        while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
        {
            st = cpssDxChL2MllPortGroupMcCntGet(dev, portGroupsBmp,
                                            mllCntSet, &mllOutMCPkts);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
        }

        /* 3. Call function with out of bound value for device id.*/
        dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
        st = cpssDxChL2MllPortGroupMcCntGet(dev, portGroupsBmp,
                                        mllCntSet, &mllOutMCPkts);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChL2MllPortGroupSilentDropCntGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    OUT GT_U32                  *silentDropPktsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChL2MllPortGroupSilentDropCntGet)
{
    /*
        ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
        1.1.1. Call with valid silentDropPktsPtr[NON-NULL].
        Expected: GT_OK.
        1.1.2. Call with invalid silentDropPktsPtr[NULL].
        Expected: GT_BAD_PTR.
    */

        GT_STATUS           st                  = GT_OK;
        GT_U8               dev                 = 0;
        GT_U32              notAppFamilyBmp     = 0;
        GT_U32              portGroupId         = 0;
        GT_PORT_GROUPS_BMP  portGroupsBmp       = 1;
        GT_U32              silentDropPkts      = 0;

        /* ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3) */
        UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

        /* 1. Go over all active devices. */
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {

            /* 1.1. Go over all active port groups. */
            PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
            {
                /* set next active port */
                portGroupsBmp = (1 << portGroupId);

                /*
                    1.1.1. Call with valid silentDropPktsPtr[NON-NULL].
                    Expected: GT_OK.
                */
                st = cpssDxChL2MllPortGroupSilentDropCntGet(dev, portGroupsBmp,
                                                            &silentDropPkts);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /*
                    1.1.2. Call with invalid silentDropPktsPtr[NULL].
                    Expected: GT_BAD_PTR.
                */
                st = cpssDxChL2MllPortGroupSilentDropCntGet(dev, portGroupsBmp,
                                                            NULL);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
            }
            PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

            /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
            PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
            {
                /* set next non-active port */
                portGroupsBmp = (1 << portGroupId);

                st = cpssDxChL2MllPortGroupSilentDropCntGet(dev, portGroupsBmp,
                                                            &silentDropPkts);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
            }
            PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)

            /* 1.3. For unaware port groups check that function returns GT_OK. */
            portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
            st = cpssDxChL2MllPortGroupSilentDropCntGet(dev, portGroupsBmp,
                                                        &silentDropPkts);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
        }

        /* 2. For not-active devices and devices from non-applicable family */
        /* check that function returns GT_BAD_PARAM.                        */

        /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

        /* go over all non active devices */
        while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
        {
            st = cpssDxChL2MllPortGroupSilentDropCntGet(dev, portGroupsBmp,
                                                        &silentDropPkts);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
        }

        /* 3. Call function with out of bound value for device id.*/
        dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
        st = cpssDxChL2MllPortGroupSilentDropCntGet(dev, portGroupsBmp,
                                                    &silentDropPkts);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChL2MllVidxEnableSet)
{
/*
    1.1. Call with valid parameters
                vidx [0/0xA5A/BIT_12-1]
                enable [GT_TRUE/GT_FALSE].
    Expected: GT_OK.
    1.2. Call cpssDxChL2MllVidxEnableGet.
    Expected: GT_OK and the same enableGet value as was set.
    1.3. Call with out of range vidx [BIT_12].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st     = GT_OK;
    GT_U8       dev;

    GT_U16    vidx    = 0;
    GT_BOOL   enable     = GT_FALSE;
    GT_BOOL   enableGet  = GT_FALSE;
    GT_BOOL   expectedSt = GT_OK;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_FALSE)
        {
            expectedSt = GT_NOT_APPLICABLE_DEVICE;
        }
        else
        {
            expectedSt = GT_OK;
        }

        enable  = GT_TRUE;

        /* 1.1 */
        vidx = 0;

        st = cpssDxChL2MllVidxEnableSet(dev, vidx, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        /* 1.2 */
        st = cpssDxChL2MllVidxEnableGet(dev,  vidx, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);
        if (expectedSt == GT_OK)
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
            "get another enable - %d ", dev);
        }


        /* 1.1 */
        vidx = 0xA5A;

        st = cpssDxChL2MllVidxEnableSet(dev, vidx, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        /* 1.2 */
        st = cpssDxChL2MllVidxEnableGet(dev,  vidx, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);
        if (expectedSt == GT_OK)
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
            "get another enable - %d ", dev);
        }

        /* 1.1 */
        vidx = BIT_12-1;

        st = cpssDxChL2MllVidxEnableSet(dev, vidx, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        /* 1.2 */
        st = cpssDxChL2MllVidxEnableGet(dev,  vidx, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);
        if (expectedSt == GT_OK)
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
            "get another enable - %d ", dev);
        }

        enable  = GT_FALSE;

        st = cpssDxChL2MllVidxEnableSet(dev, vidx, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        /* 1.2 */
        st = cpssDxChL2MllVidxEnableGet(dev,  vidx, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);
        if (expectedSt == GT_OK)
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
            "get another enable - %d ", dev);
        }

        /* 1.1 */
        vidx = 0xA5A;

        st = cpssDxChL2MllVidxEnableSet(dev, vidx, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        /* 1.2 */
        st = cpssDxChL2MllVidxEnableGet(dev,  vidx, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);
        if (expectedSt == GT_OK)
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
            "get another enable - %d ", dev);
        }

        /* 1.1 */
        vidx = BIT_12-1;

        st = cpssDxChL2MllVidxEnableSet(dev, vidx, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        /* 1.2 */
        st = cpssDxChL2MllVidxEnableGet(dev,  vidx, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);
        if (expectedSt == GT_OK)
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
            "get another enable - %d ", dev);
        }

        /* 1.3 */
        vidx = BIT_12;
        st = cpssDxChL2MllVidxEnableSet(dev, vidx, enable);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        vidx = 0;

    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChL2MllVidxEnableSet(dev, vidx, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChL2MllVidxEnableSet(dev, vidx, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChL2MllVidxEnableGet)
{
/*
    1.1.1. Call with not NULL enablePtr and valid parameters
            vidx [0/0xA5A/BIT_12-1].
    Expected: GT_OK.
    1.1.2. Call with NULL enableGet.
    Expected: GT_OK.
    1.1.3. Call with out of range vidx [BIT_12].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st       = GT_OK;
    GT_U8       dev;

    GT_U16    vidx       = 0;
    GT_BOOL   enableGet  = GT_FALSE;
    GT_BOOL   expectedSt = GT_OK;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_FALSE)
        {
            expectedSt = GT_NOT_APPLICABLE_DEVICE;
        }
        else
        {
            expectedSt = GT_OK;
        }

        /* 1.1 */
        vidx    = 0;
        st = cpssDxChL2MllVidxEnableGet(dev, vidx, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        vidx    = 0xA5A;
        st = cpssDxChL2MllVidxEnableGet(dev, vidx, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        vidx    = BIT_12-1;
        st = cpssDxChL2MllVidxEnableGet(dev, vidx, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        /* 1.2 */
        st = cpssDxChL2MllVidxEnableGet(dev, vidx, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /* 1.3 */
        vidx = BIT_12;
        st = cpssDxChL2MllVidxEnableGet(dev, vidx, &enableGet);
                if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_FALSE)
        {
            expectedSt = GT_NOT_APPLICABLE_DEVICE;
        }
        else
        {
            expectedSt = GT_OUT_OF_RANGE;
        }
        UTF_VERIFY_EQUAL1_PARAM_MAC(expectedSt, st, dev);

        vidx = 0;
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChL2MllVidxEnableGet(dev, vidx, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChL2MllVidxEnableGet(dev, vidx, &enableGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChL2MLLVirtualPortToMllMappingTableEntrySet
(
    IN GT_U8                             devNum,
    IN GT_U8                             targetDevNum,
    IN GT_U8                             targetPortNum,
    IN GT_U32                            mllPointer
)
*/
UTF_TEST_CASE_MAC(cpssDxChL2MLLVirtualPortToMllMappingTableEntrySet)
{
/*
    ITERATE_DEVICES (xCat)
    1.1. Call with targetDevNum[0, 7, 15],
                   targetPortNum[0, 8, 63],
                   mllPointer[0, max/2, max].
    Expected: GT_OK.
    1.2. Call cpssDxChL2MLLVirtualPortToMllMappingTableEntryGet.
    Expected: GT_OK and the same values as was set.
    1.3. Call with out of range targetDevNum[16] and other valid params.
    Expected: GT_BAD_PARAM.
    1.4. Call with out of range targetPortNum[64] and other valid params.
    Expected: GT_BAD_PARAM.
    1.5. Call with out of range mllPointer[max+1] and other valid params.
    Expected: NON GT_OK.
*/

    GT_STATUS       st              = GT_OK;
    GT_U8           dev             = 0;
    GT_U8           targetDevNum    = 0;
    GT_U8           targetPortNum   = 0;
    GT_U32          mllPointer      = 0;
    GT_U32          mllPointerRet   = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            /*
                1.1. Call with targetDevNum[0, 7, 15],
                               targetPortNum[0, 8, 63],
                               mllPointer[0, max/2, max].
                Expected: GT_OK.
            */

            /* call with targetDevNum[0] */
            targetDevNum = 0;
            targetPortNum = 0;
            mllPointer = 0;

            st = cpssDxChL2MLLVirtualPortToMllMappingTableEntrySet(dev,
                                                                   targetDevNum,
                                                                   targetPortNum,
                                                                   mllPointer);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.2. Call cpssDxChL2MLLVirtualPortToMllMappingTableEntryGet.
                Expected: GT_OK and the same values as was set.
            */
            st = cpssDxChL2MLLVirtualPortToMllMappingTableEntryGet(dev,
                                                                   targetDevNum,
                                                                   targetPortNum,
                                                                   &mllPointerRet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            if (GT_OK == st)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(mllPointer, mllPointerRet,
                                             "got another mllPointer: %d",
                                             mllPointerRet);
            }

            /* call with targetDevNum[7] */
            targetDevNum = 7;
            targetPortNum = 8;
            mllPointer = (PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.mllPairs - 1)/2;

            st = cpssDxChL2MLLVirtualPortToMllMappingTableEntrySet(dev,
                                                                   targetDevNum,
                                                                   targetPortNum,
                                                                   mllPointer);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.2. Call cpssDxChL2MLLVirtualPortToMllMappingTableEntryGet.
                Expected: GT_OK and the same values as was set.
            */
            st = cpssDxChL2MLLVirtualPortToMllMappingTableEntryGet(dev,
                                                                   targetDevNum,
                                                                   targetPortNum,
                                                                   &mllPointerRet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            if (GT_OK == st)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(mllPointer, mllPointerRet,
                                             "got another mllPointer: %d",
                                             mllPointerRet);
            }

            /* call with targetDevNum[15] */
            targetDevNum = 15;
            targetPortNum = 63;
            mllPointer = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.mllPairs - 1;

            st = cpssDxChL2MLLVirtualPortToMllMappingTableEntrySet(dev,
                                                                   targetDevNum,
                                                                   targetPortNum,
                                                                   mllPointer);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.2. Call cpssDxChL2MLLVirtualPortToMllMappingTableEntryGet.
                Expected: GT_OK and the same values as was set.
            */
            st = cpssDxChL2MLLVirtualPortToMllMappingTableEntryGet(dev,
                                                                   targetDevNum,
                                                                   targetPortNum,
                                                                   &mllPointerRet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            if (GT_OK == st)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(mllPointer, mllPointerRet,
                                             "got another mllPointer: %d",
                                             mllPointerRet);
            }


            /*
                1.3. Call with out of range targetDevNum[16] and other valid params.
                Expected: GT_BAD_PARAM.
            */
            targetDevNum = 16;

            st = cpssDxChL2MLLVirtualPortToMllMappingTableEntrySet(dev,
                                                                   targetDevNum,
                                                                   targetPortNum,
                                                                   mllPointer);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            /* restore value */
            targetDevNum = 15;

            /*
                1.4. Call with out of range targetPortNum[64] and other valid params.
                Expected: GT_BAD_PARAM.
            */
            targetPortNum = 64;

            st = cpssDxChL2MLLVirtualPortToMllMappingTableEntrySet(dev,
                                                                   targetDevNum,
                                                                   targetPortNum,
                                                                   mllPointer);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            /* restore value */
            targetPortNum = 63;

            /*
                1.5. Call with out of range mllPointer[max+1] and other valid params.
                Expected: NON GT_OK.
            */
            mllPointer = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.mllPairs;

            st = cpssDxChL2MLLVirtualPortToMllMappingTableEntrySet(dev,
                                                                   targetDevNum,
                                                                   targetPortNum,
                                                                   mllPointer);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* restore value */
            mllPointer = 0;
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChL2MLLVirtualPortToMllMappingTableEntrySet(dev,
                                                               targetDevNum,
                                                               targetPortNum,
                                                               mllPointer);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChL2MLLVirtualPortToMllMappingTableEntrySet(dev,
                                                           targetDevNum,
                                                           targetPortNum,
                                                           mllPointer);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChL2MLLVirtualPortToMllMappingTableEntryGet
(
    IN  GT_U8                                devNum,
    IN  GT_U8                                targetDevNum,
    IN  GT_U8                                targetPortNum,
    OUT GT_U32                              *mllPointer
)
*/
UTF_TEST_CASE_MAC(cpssDxChL2MLLVirtualPortToMllMappingTableEntryGet)
{
/*
    ITERATE_DEVICES (xCat)
    1.1. Call with targetDevNum[0, 7, 15],
                   targetPortNum[0, 8, 63],
                   and non-null mllPointerPtr.
    Expected: GT_OK.
    1.2. Call with out of range targetDevNum[16] and other valid params.
    Expected: GT_BAD_PARAM.
    1.3. Call with out of range targetPortNum[64] and other valid params.
    Expected: GT_BAD_PARAM.
    1.4. Call with out of range mllPointerPtr[NULL] and other valid params.
    Expected: GT_BAD_PTR.
*/

    GT_STATUS       st              = GT_OK;
    GT_U8           dev             = 0;
    GT_U8           targetDevNum    = 0;
    GT_U8           targetPortNum   = 0;
    GT_U32          mllPointer      = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            /*
                1.1. Call with targetDevNum[0, 7, 15],
                               targetPortNum[0, 8, 63],
                               and valid mllPointerPtr.
                Expected: GT_OK.
            */

            /* call with targetDevNum[0] */
            targetDevNum = 0;
            targetPortNum = 0;

            st = cpssDxChL2MLLVirtualPortToMllMappingTableEntryGet(dev,
                                                                   targetDevNum,
                                                                   targetPortNum,
                                                                   &mllPointer);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call with targetDevNum[7] */
            targetDevNum = 7;
            targetPortNum = 8;

            st = cpssDxChL2MLLVirtualPortToMllMappingTableEntryGet(dev,
                                                                   targetDevNum,
                                                                   targetPortNum,
                                                                   &mllPointer);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call with targetDevNum[15] */
            targetDevNum = 15;
            targetPortNum = 63;

            st = cpssDxChL2MLLVirtualPortToMllMappingTableEntryGet(dev,
                                                                   targetDevNum,
                                                                   targetPortNum,
                                                                   &mllPointer);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.2. Call with out of range targetDevNum[16] and other valid params.
                Expected: GT_BAD_PARAM.
            */
            targetDevNum = 16;

            st = cpssDxChL2MLLVirtualPortToMllMappingTableEntryGet(dev,
                                                                   targetDevNum,
                                                                   targetPortNum,
                                                                   &mllPointer);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            /* restore value */
            targetDevNum = 15;

            /*
                1.3. Call with out of range targetPortNum[64] and other valid params.
                Expected: GT_BAD_PARAM.
            */
            targetPortNum = 64;

            st = cpssDxChL2MLLVirtualPortToMllMappingTableEntryGet(dev,
                                                                   targetDevNum,
                                                                   targetPortNum,
                                                                   &mllPointer);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            /* restore value */
            targetPortNum = 63;

            /*
                1.4. Call with out of range mllPointerPtr[NULL] and other valid params.
                Expected: GT_BAD_PTR.
            */

            st = cpssDxChL2MLLVirtualPortToMllMappingTableEntryGet(dev,
                                                                   targetDevNum,
                                                                   targetPortNum,
                                                                   NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChL2MLLVirtualPortToMllMappingTableEntryGet(dev,
                                                               targetDevNum,
                                                               targetPortNum,
                                                               &mllPointer);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChL2MLLVirtualPortToMllMappingTableEntryGet(dev,
                                                           targetDevNum,
                                                           targetPortNum,
                                                           &mllPointer);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChL2Mll suit
 */

UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChL2Mll)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChL2MllPairWrite)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChL2MllPairRead)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChL2MllLookupForAllEvidxEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChL2MllLookupForAllEvidxEnableGet)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChL2MllLookupMaxVidxIndexSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChL2MllLookupMaxVidxIndexGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChL2MllSetCntInterfaceCfg)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChL2MllMcCntGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChL2MllSilentDropCntGet)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChL2MllLttEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChL2MllLttEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChL2MllTtlExceptionConfigurationSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChL2MllTtlExceptionConfigurationGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChL2MllCounterGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChL2MllExceptionCountersGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChL2MllPortGroupCounterGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChL2MllPortGroupExceptionCountersGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChL2MllMultiTargetPortEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChL2MllMultiTargetPortEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChL2MllMultiTargetPortSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChL2MllMultiTargetPortGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChL2MllMultiTargetPortBaseSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChL2MllMultiTargetPortBaseGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChL2MllPortToVidxBaseSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChL2MllPortToVidxBaseGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChL2MllSourceBasedFilteringConfigurationSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChL2MllSourceBasedFilteringConfigurationGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChL2MllPortGroupMcCntGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChL2MllPortGroupSilentDropCntGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChL2MllVidxEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChL2MllVidxEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChL2MLLVirtualPortToMllMappingTableEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChL2MLLVirtualPortToMllMappingTableEntryGet)
UTF_SUIT_END_TESTS_MAC(cpssDxChL2Mll)


