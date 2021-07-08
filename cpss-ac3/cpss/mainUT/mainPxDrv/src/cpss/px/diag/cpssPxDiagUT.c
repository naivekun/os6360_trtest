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
* @file cpssPxDiagUT.c
*
* @brief Unit Tests for CPSS PX diagnostic APIs.
*
*
* @version   1
********************************************************************************
*/
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>

#include <cpss/px/diag/cpssPxDiag.h>

UTF_TEST_CASE_MAC(cpssPxDiagRegsNumGet)
{
/*
    ITERATE_DEVICES (Px)
    1.1.1. Call with not-NULL pointers.
    Expected: GT_OK.
    1.1.2. Call regsNumPtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_U32      regsNum;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1.1. Call with not-NULL pointers.
            Expected: GT_OK.
        */
        st = cpssPxDiagRegsNumGet(dev, &regsNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.1.2. Call regsNumPtr[NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssPxDiagRegsNumGet(dev, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxDiagRegsNumGet(dev, &regsNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxDiagRegsNumGet(dev, &regsNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxDiagRegsDump
(
    IN    GT_U8                 devNum,
    IN    GT_PORT_GROUPS_BMP    portGroupsBmp,
    INOUT GT_U32                *regsNumPtr,
    IN    GT_U32                offset,
    OUT   GT_U32                *regAddrPtr,
    OUT   GT_U32                *regDataPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxDiagRegsDump)
{
/*
    ITERATE_DEVICES_PORT_GROUPS (Px)
    1.1.1. Call with regAddrPtr[NULL].
    Expected: GT_BAD_PTR.
    1.1.2. Call with regDataPtr[NULL].
    Expected: GT_BAD_PTR.
    1.1.3. Call with regsNumPtr[NULL].
    Expected: GT_BAD_PTR.
    1.1.4 Call cpssPxDiagRegsDump
          with regsNumPtr bigger than the real registers number.
    Expected: GT_BAD_PARAM.

*/
    GT_STATUS           st = GT_OK;
    GT_U8               dev;
    GT_U32              regsNum = 0;
    GT_U32              *regAddrPtr = 0;
    GT_U32              *regDataPtr = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1.1. Call with regAddrPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxDiagRegsDump(dev, &regsNum, 0, NULL, regDataPtr);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        /*
            1.1.2. Call with regDataPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxDiagRegsDump(dev, &regsNum, 0, regAddrPtr, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        /*
            1.1.3. Call with regsNumPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxDiagRegsDump(dev, NULL, 0, regAddrPtr, regDataPtr);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        /*
            1.1.4 Call cpssPxDiagRegsDump
            with regsNumPtr bigger than the real registers number.
            Expected: GT_BAD_PARAM.
        */
        st = cpssPxDiagRegsNumGet(dev, &regsNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* allocate space for regAddress and regData */
        regAddrPtr = cpssOsMalloc((regsNum+1) * sizeof(GT_U32));
        regDataPtr = cpssOsMalloc((regsNum+1) * sizeof(GT_U32));

        UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)regAddrPtr,
                                "cpssOsMalloc: Memory allocation error.");
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)regDataPtr,
                                "cpssOsMalloc: Memory allocation error.");

        /* clean buffers to avoid trash in log */
        cpssOsMemSet(regAddrPtr, 0, (regsNum+1) * sizeof(GT_U32));
        cpssOsMemSet(regDataPtr, 0, (regsNum+1) * sizeof(GT_U32));
        /*regsNum += 1; --> BUG in DXCH mainUT !!! the '+1' not allow to check the registers !!! */

        st = cpssPxDiagRegsDump(dev, &regsNum, 0, regAddrPtr, regDataPtr);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        if (regAddrPtr != NULL)
        {
            /* free memory if allocated */
            cpssOsFree(regAddrPtr);
            regAddrPtr = (GT_U32*)NULL;
        }

        if (regDataPtr != NULL)
        {
            /* free memory if allocated */
            cpssOsFree(regDataPtr);
            regDataPtr = (GT_U32*)NULL;
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM. */

    /* allocate space for one register */
    regAddrPtr = cpssOsMalloc(sizeof(GT_U32));
    regDataPtr = cpssOsMalloc(sizeof(GT_U32));

    UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)regAddrPtr,
                            "cpssOsMalloc: Memory allocation error.");
    UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)regDataPtr,
                            "cpssOsMalloc: Memory allocation error.");

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        regsNum = 1;
        st = cpssPxDiagRegsDump(dev, &regsNum, 0, regAddrPtr, regDataPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    regsNum = 1;
    st = cpssPxDiagRegsDump(dev, &regsNum, 0, regAddrPtr, regDataPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

    if (regAddrPtr != NULL)
    {
        /* free memory if allocated */
        cpssOsFree(regAddrPtr);
        regAddrPtr = (GT_U32*)NULL;
    }

    if (regDataPtr != NULL)
    {
        /* free memory if allocated */
        cpssOsFree(regDataPtr);
        regDataPtr = (GT_U32*)NULL;
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxDiagResetAndInitControllerRegsNumGet
(
    IN  GT_U8                   devNum,
    OUT GT_U32                  *regsNumPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxDiagResetAndInitControllerRegsNumGet)
{
/*
    1.1 Call for cpssPxDiagResetAndInitControllerRegsNumGet and then
        cpssPxDiagResetAndInitControllerRegsDump with not-NULL pointers.
        Expected: GT_OK.
    1.2 Call cpssPxDiagResetAndInitControllerRegsNumGet with regsNumPtr[NULL].
        Expected: GT_BAD_PTR.
*/
    GT_STATUS                   st   = GT_OK;
    GT_U8                       dev;
    GT_U32                      regsNum = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1 Call for cpssPxDiagResetAndInitControllerRegsNumGet and then
                cpssPxDiagResetAndInitControllerRegsDump with not-NULL pointers.
            Expected: GT_OK.
        */

        /* read the registers number */
        st = cpssPxDiagResetAndInitControllerRegsNumGet(dev, &regsNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.2 Call cpssPxDiagResetAndInitControllerRegsNumGet with regsNumPtr[NULL].
                Expected: GT_BAD_PTR.
        */
        st = cpssPxDiagResetAndInitControllerRegsNumGet(dev, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM. */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxDiagResetAndInitControllerRegsNumGet(dev, &regsNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxDiagResetAndInitControllerRegsNumGet(dev, &regsNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxDiagResetAndInitControllerRegsDump
(
    IN    GT_U8     devNum,
    INOUT GT_U32    *regsNumPtr,
    IN    GT_U32    offset,
    OUT   GT_U32    *regAddrPtr,
    OUT   GT_U32    *regDataPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxDiagResetAndInitControllerRegsDump)
{
/*
    1.1.1. Call with regAddrPtr[NULL].
           Expected: GT_BAD_PTR.
    1.1.2. Call with regDataPtr[NULL].
           Expected: GT_BAD_PTR.
    1.1.3. Call with regsNumPtr[NULL].
           Expected: GT_BAD_PTR.
    1.1.4. Call with regsNumPtr bigger than the real registers number.
           Expected: GT_BAD_PARAM.

*/
    GT_STATUS           st = GT_OK;
    GT_U8               dev;
    GT_U32              regsNum = 0;
    GT_U32              *regAddrPtr = 0;
    GT_U32              *regDataPtr = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1.1. Call with regAddrPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxDiagResetAndInitControllerRegsDump(
                                            dev, &regsNum, 0, NULL, regDataPtr);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        /*
            1.1.2. Call with regDataPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxDiagResetAndInitControllerRegsDump(
                                            dev, &regsNum, 0, regAddrPtr, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        /*
            1.1.3. Call with regsNumPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxDiagResetAndInitControllerRegsDump(
                                            dev, NULL, 0, regAddrPtr, regDataPtr);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        /*
            1.1.4 Call with regsNumPtr bigger than the real registers number.
            Expected: GT_BAD_PARAM.
        */
        st = cpssPxDiagResetAndInitControllerRegsNumGet(dev, &regsNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        if( (GT_FALSE == prvUtfIsGmCompilation()) || (regsNum > 0) )
        {
            /* allocate space for regAddress and regData */
            regAddrPtr = cpssOsMalloc(regsNum * sizeof(GT_U32));
            regDataPtr = cpssOsMalloc(regsNum * sizeof(GT_U32));

            UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)regAddrPtr,
                                             "cpssOsMalloc: Memory allocation error.");
            UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)regDataPtr,
                                             "cpssOsMalloc: Memory allocation error.");

            /*regsNum += 1; --> BUG in DXCH mainUT !!! the '+1' not allow to check the registers !!! */

            st = cpssPxDiagResetAndInitControllerRegsDump(
                                    dev, &regsNum, 0, regAddrPtr, regDataPtr);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            if (regAddrPtr != NULL)
            {
                /* free memory if allocated */
                cpssOsFree(regAddrPtr);
                regAddrPtr = (GT_U32*)NULL;
            }

            if (regDataPtr != NULL)
            {
                /* free memory if allocated */
                cpssOsFree(regDataPtr);
                regDataPtr = (GT_U32*)NULL;
            }
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM. */

    /* allocate space for one register */
    regAddrPtr = cpssOsMalloc(sizeof(GT_U32));
    regDataPtr = cpssOsMalloc(sizeof(GT_U32));

    UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)regAddrPtr,
                            "cpssOsMalloc: Memory allocation error.");
    UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)regDataPtr,
                            "cpssOsMalloc: Memory allocation error.");

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        regsNum = 1;
        st = cpssPxDiagResetAndInitControllerRegsDump(
                                    dev, &regsNum, 0, regAddrPtr, regDataPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    regsNum = 1;
    st = cpssPxDiagResetAndInitControllerRegsDump(
                                    dev, &regsNum, 0, regAddrPtr, regDataPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

    if (regAddrPtr != NULL)
    {
        /* free memory if allocated */
        cpssOsFree(regAddrPtr);
        regAddrPtr = (GT_U32*)NULL;
    }

    if (regDataPtr != NULL)
    {
        /* free memory if allocated */
        cpssOsFree(regDataPtr);
        regDataPtr = (GT_U32*)NULL;
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxDiagRegWrite
(
    IN GT_UINTPTR                      baseAddr,
    IN CPSS_PP_INTERFACE_CHANNEL_ENT   ifChannel,
    IN CPSS_DIAG_PP_REG_TYPE_ENT       regType,
    IN GT_U32                          offset,
    IN GT_U32                          data,
    IN GT_BOOL                         doByteSwap
);
*/
UTF_TEST_CASE_MAC(cpssPxDiagRegWrite)
{
/*
    1.1.1. Call with ifChannel not valid
    Expected: GT_OK.
*/
    GT_STATUS   st   = GT_BAD_PARAM;

    GT_U32                          baseAddr = 0;
    CPSS_PP_INTERFACE_CHANNEL_ENT   ifChannel = 0x55555;
    CPSS_DIAG_PP_REG_TYPE_ENT       regType = 0;
    GT_U32                          offset = 0;
    GT_U32                          data = 0;
    GT_BOOL                         doByteSwap = GT_TRUE;


    st = cpssPxDiagRegWrite(baseAddr, ifChannel, regType, offset, data, doByteSwap);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxDiagRegRead
(
    IN GT_UINTPTR                      baseAddr,
    IN CPSS_PP_INTERFACE_CHANNEL_ENT   ifChannel,
    IN CPSS_DIAG_PP_REG_TYPE_ENT       regType,
    IN GT_U32                          offset,
    OUT GT_U32                         *dataPtr,
    IN GT_BOOL                         doByteSwap
);
*/
static void ut_cpssPxDiagRegRead(IN GT_U8 dev)
{
/*
    1.1.1. Call with dataPtr == NULL
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(dev);

    GT_UINTPTR                      baseAddr = 0;
    CPSS_PP_INTERFACE_CHANNEL_ENT   ifChannel = 0;
    CPSS_PP_INTERFACE_CHANNEL_ENT   ifChannelArr[] = {
            CPSS_CHANNEL_PCI_E,
            CPSS_CHANNEL_PEX_E,
            CPSS_CHANNEL_PEX_MBUS_E,

            CPSS_CHANNEL_LAST_E
            };
    CPSS_DIAG_PP_REG_TYPE_ENT       regType = 0;
    GT_U32                          offset = 0;
    GT_U32                          regsArray[] = {0,0,0,0xFFFFFFFF};
    GT_BOOL                         doByteSwap = GT_TRUE;
    GT_U32                          portGroupId = 0;
    GT_U32                          orig_regValue,regValueRead,newRegValueWrite;
    GT_U32                          ii,jj;
    GT_U32  index = 0;
    APP_DEMO_PP_CONFIG              *appDemoPtr = NULL;

    st = cpssPxDiagRegRead(baseAddr, ifChannel, regType, offset,
                             NULL, doByteSwap);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

    if (prvCpssDrvPpConfig[dev]->mngInterfaceType == CPSS_CHANNEL_PEX_KERNEL_E)
    {
        PRV_UTF_LOG1_MAC("PEX_KERNEL channel connects device %d to CPU\n", dev);
        PRV_UTF_LOG0_MAC("Test skipped \n");
        SKIP_TEST_MAC;
    }

    index = 0;
    appDemoPtr = &appDemoPpConfigList[0];
    /* find the AppDemo that match the dev */
    while(index < APP_DEMO_PP_CONFIG_SIZE_CNS)
    {
        if(appDemoPtr->devNum == dev)
        {
            break;
        }
        index++;
        appDemoPtr++;
    }

    if(index == APP_DEMO_PP_CONFIG_SIZE_CNS)
    {
        /* not found our device ?! */
        UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(APP_DEMO_PP_CONFIG_SIZE_CNS, index);
        return;
    }

    appDemoPtr = &appDemoPpConfigList[index];

    if(appDemoPtr->valid == GT_TRUE &&
       (CPSS_CHANNEL_PCI_E == appDemoPtr->channel ||
        CPSS_CHANNEL_PEX_E == appDemoPtr->channel ||
        CPSS_CHANNEL_PEX_MBUS_E == appDemoPtr->channel) )
    {
        CPSS_HW_INFO_STC *hwInfoPtr;

        PRV_UTF_LOG1_MAC("baseAddr [0x%8.8x] of device \n", baseAddr);

        UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(CPSS_PARAM_NOT_USED_CNS, baseAddr);

        hwInfoPtr = cpssDrvHwPpHwInfoStcPtrGet(dev, portGroupId);
        if (!hwInfoPtr)
        {
            return;
        }

        baseAddr = hwInfoPtr->resource.switching.start;
        if(baseAddr == CPSS_PARAM_NOT_USED_CNS || baseAddr == 0)
        {
            return;
        }

        regType = CPSS_DIAG_PP_REG_INTERNAL_E;

        /* test PCI mode */
        ifChannel = CPSS_CHANNEL_PCI_E;
        if (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            /* appDemo provides 4M window for SIP5 devices. Therefore only new address completion
               should be used for SIP5 and above devices - PEX_MBUS. */
            ifChannel = CPSS_CHANNEL_PEX_MBUS_E;
        }

        PRV_UTF_LOG1_MAC("ifChannel = [0x%8.8x] \n", ifChannel);

        /* read register that hold the 0x11AB to understand the 'swap' value */
        doByteSwap = GT_FALSE;
        st = cpssPxDiagRegRead(baseAddr, ifChannel, regType, 0x50,
                                 &regValueRead, doByteSwap);

        PRV_UTF_LOG1_MAC("regAddr = [0x%8.8x] \n", 0x50);
        PRV_UTF_LOG1_MAC("doByteSwap = [0x%8.8x] \n", doByteSwap);
        PRV_UTF_LOG1_MAC("regValueRead = [0x%8.8x] \n", regValueRead);
        PRV_UTF_LOG1_MAC("st = [0x%8.8x] \n", st);
        if(regValueRead == 0x11AB)
        {
        }
        else
        if(regValueRead == 0xAB110000)
        {
            doByteSwap = GT_TRUE;
            st = cpssPxDiagRegRead(baseAddr, ifChannel, regType, 0x50,
                                     &regValueRead, doByteSwap);
            PRV_UTF_LOG1_MAC("doByteSwap = [0x%8.8x] \n", doByteSwap);
            PRV_UTF_LOG1_MAC("regValueRead = [0x%8.8x] \n", regValueRead);
            PRV_UTF_LOG1_MAC("st = [0x%8.8x] \n", st);
        }

        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK , st);
        UTF_VERIFY_EQUAL0_PARAM_MAC(0x11AB , regValueRead);

        /* PRV_CPSS_DXCH_UNIT_TXDMA_E */
        regsArray[0] = regsAddrPtr->txDMA.txDMAPerSCDMAConfigs.burstLimiterSCDMA[0];

        /* PRV_CPSS_DXCH_UNIT_TX_FIFO_E */
        regsArray[1] = regsAddrPtr->txFIFO.txFIFOPizzaArb.pizzaArbiter.pizzaArbiterConfigReg[0];

        /* PRV_CPSS_DXCH_UNIT_CNC_0_E */
        regsArray[2] = regsAddrPtr->CNC.globalRegs.CNCClearByReadValueRegWord1;

        for(jj = 0 ; regsArray[jj] != 0xFFFFFFFF ; jj++)
        {
            offset = regsArray[jj];
            PRV_UTF_LOG1_MAC("offset = [0x%8.8x] \n", offset);

            /* read register by 'regular driver' API */
            cpssDrvPpHwRegisterRead(dev,portGroupId,offset, &orig_regValue);
            PRV_UTF_LOG1_MAC("orig_regValue = [0x%8.8x] \n", orig_regValue);

            newRegValueWrite = 0xAABBCCDD + offset;
            PRV_UTF_LOG1_MAC("(by driver) newRegValueWrite = [0x%8.8x] \n", newRegValueWrite);
            /* write register by 'regular driver' API */
            cpssDrvPpHwRegisterWrite(dev,portGroupId,offset, newRegValueWrite);

            PRV_UTF_LOG1_MAC("doByteSwap = [0x%8.8x] \n", doByteSwap);

            for(ii = 0 ; ifChannelArr[ii] != CPSS_CHANNEL_LAST_E ; ii++)
            {
                ifChannel = ifChannelArr[ii];

                if(ifChannel == CPSS_CHANNEL_PEX_MBUS_E &&
                    !PRV_CPSS_SIP_5_CHECK_MAC(dev))
                {
                    continue;
                }

                /* appDemo provides 4M window for SIP5 devices. Therefore only new address completion
                   should be used for SIP5 and above devices - PEX_MBUS. */
                if(ifChannel != CPSS_CHANNEL_PEX_MBUS_E)
                {
                    continue;
                }

                PRV_UTF_LOG1_MAC("ifChannel = [0x%8.8x] \n", ifChannel);

                regValueRead = 0;
                st = cpssPxDiagRegRead(baseAddr, ifChannel, regType, offset,
                                         &regValueRead, doByteSwap);
                PRV_UTF_LOG1_MAC("(first)regValueRead = [0x%8.8x] \n", regValueRead);
                PRV_UTF_LOG1_MAC("st = [0x%8.8x] \n", st);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK , st);
                UTF_VERIFY_EQUAL0_PARAM_MAC(newRegValueWrite , regValueRead);

                newRegValueWrite = 0xF55F11FF + offset + (ifChannel << 10);
                PRV_UTF_LOG1_MAC("(by diag)newRegValueWrite = [0x%8.8x] \n", newRegValueWrite);
                st = cpssPxDiagRegWrite(baseAddr, ifChannel, regType, offset,
                                         newRegValueWrite, doByteSwap);
                PRV_UTF_LOG1_MAC("newRegValueWrite = [0x%8.8x] \n", newRegValueWrite);
                PRV_UTF_LOG1_MAC("st = [0x%8.8x] \n", st);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK , st);

                regValueRead = 0;
                st = cpssPxDiagRegRead(baseAddr, ifChannel, regType, offset,
                                         &regValueRead, doByteSwap);
                PRV_UTF_LOG1_MAC("(second)regValueRead = [0x%8.8x] \n", regValueRead);
                PRV_UTF_LOG1_MAC("st = [0x%8.8x] \n", st);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK , st);
                UTF_VERIFY_EQUAL0_PARAM_MAC(newRegValueWrite , regValueRead);
            }

            /* restore register by 'regular driver' API */
            cpssDrvPpHwRegisterWrite(dev,portGroupId,offset, orig_regValue);
        }
    }
}

UTF_TEST_CASE_MAC(cpssPxDiagRegRead)
{
    GT_U8       dev;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        ut_cpssPxDiagRegRead(dev);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxDiagRegTest
(
    IN  GT_SW_DEV_NUM                 devNum,
    IN  GT_U32                        regAddr,
    IN  GT_U32                        regMask,
    IN  CPSS_DIAG_TEST_PROFILE_ENT    profile,
    OUT GT_BOOL                       *testStatusPtr,
    OUT GT_U32                        *readValPtr,
    OUT GT_U32                        *writeValPtr
);
*/
UTF_TEST_CASE_MAC(cpssPxDiagRegTest)
{
/*
    ITERATE_DEVICES (Px)
    1.1.1. Call with not-NULL pointers.
    Expected: GT_OK.
    1.1.2. Call wrong enum values profile.
    Expected: GT_BAD_PARAM.
    1.1.3. Call testStatusPtr[NULL].
    Expected: GT_BAD_PTR.
    1.1.4. Call readValPtr[NULL].
    Expected: GT_BAD_PTR.
    1.1.5. Call writeValPtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;

    GT_U32                         regAddr = 0;
    GT_U32                         regMask = 0;
    CPSS_DIAG_TEST_PROFILE_ENT     profile = CPSS_DIAG_TEST_RANDOM_E;
    GT_BOOL                        testStatus;
    GT_U32                         readVal = 0;
    GT_U32                         writeVal = 0;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1.1. Call with not-NULL pointers.
            Expected: GT_OK.
        */
        st = cpssPxDiagRegTest(dev, regAddr, regMask, profile,
                                 &testStatus, &readVal, &writeVal);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.1.2. Call wrong enum values profile.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssPxDiagRegTest
                            (dev, regAddr, regMask, profile, &testStatus, &readVal, &writeVal),
                            profile);

        /*
            1.1.3. Call testStatusPtr[NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssPxDiagRegTest(dev, regAddr, regMask, profile,
                                 NULL, &readVal, &writeVal);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        /*
            1.1.4. Call readValPtr[NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssPxDiagRegTest(dev, regAddr, regMask, profile,
                                 &testStatus, NULL, &writeVal);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        /*
            1.1.5. Call writeValPtr[NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssPxDiagRegTest(dev, regAddr, regMask, profile,
                                 &testStatus, &readVal, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxDiagRegTest(dev, regAddr, regMask, profile,
                                 &testStatus, &readVal, &writeVal);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxDiagRegTest(dev, regAddr, regMask, profile,
                             &testStatus, &readVal, &writeVal);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxDiagAllRegTest
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_BOOL         *testStatusPtr,
    OUT GT_U32          *badRegPtr,
    OUT GT_U32          *readValPtr,
    OUT GT_U32          *writeValPtr
);
*/
UTF_TEST_CASE_MAC(cpssPxDiagAllRegTest)
{
/*
    ITERATE_DEVICES (Px)
    1.1. Call with non-NULL pointers.
    Expected: GT_OK.
    1.2. Call with testStatusPtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.
    1.3. Call with badRegPtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.
    1.4. Call with readValPtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.
    1.5. Call with writeValPtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_BOOL     testStatus;
    GT_U32      badReg   = 0;
    GT_U32      readVal  = 0;
    GT_U32      writeVal = 0;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-NULL pointers.
            Expected: GT_OK.
        */
        st = cpssPxDiagAllRegTest(dev, &testStatus, &badReg, &readVal, &writeVal);
        if(st != GT_OK)
        {
            PRV_UTF_LOG3_MAC("cpssPxDiagAllRegTest: FAILED on register address[0x%8.8x] with writeVal[0x%8.8x] and readVal[0x%8.8x] \n",
                badReg,readVal,writeVal);
        }
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_TRUE, testStatus, dev);

        /*
            1.2. Call with testStatusPtr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssPxDiagAllRegTest(dev, NULL, &badReg, &readVal, &writeVal);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.3. Call with badRegPtr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssPxDiagAllRegTest(dev, &testStatus, NULL, &readVal, &writeVal);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.4. Call with readValPtr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssPxDiagAllRegTest(dev, &testStatus, &badReg, NULL, &writeVal);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.5. Call with writeValPtr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssPxDiagAllRegTest(dev, &testStatus, &badReg, &readVal, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxDiagAllRegTest(dev, &testStatus, &badReg, &readVal, &writeVal);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxDiagAllRegTest(dev, &testStatus, &badReg, &readVal, &writeVal);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxDiagDeviceTemperatureSensorsSelectSet
(
    IN  GT_SW_DEV_NUM                        devNum,
    IN  CPSS_PX_DIAG_TEMPERATURE_SENSOR_ENT  sensorType
);
*/
UTF_TEST_CASE_MAC(cpssPxDiagDeviceTemperatureSensorsSelectSet)
{
/*
    ITERATE_DEVICES
    1.1. Call with sensorType[CPSS_PX_DIAG_TEMPERATURE_SENSOR_0_E \
                              CPSS_PX_DIAG_TEMPERATURE_SENSOR_AVERAGE_E].
    Expected: GT_OK.
    1.2. Call GET function  with same params.
    Expected: GT_OK and same value as written.
    1.3. Call with out of range sensorType.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    CPSS_PX_DIAG_TEMPERATURE_SENSOR_ENT sensorType    = CPSS_PX_DIAG_TEMPERATURE_SENSOR_0_E;
    CPSS_PX_DIAG_TEMPERATURE_SENSOR_ENT sensorTypeGet = CPSS_PX_DIAG_TEMPERATURE_SENSOR_0_E;
    CPSS_PX_DIAG_TEMPERATURE_SENSOR_ENT sensorTypeSave;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /* save sensor selection */
        st = cpssPxDiagDeviceTemperatureSensorsSelectGet(dev, &sensorTypeSave);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.1. */
        sensorType = CPSS_PX_DIAG_TEMPERATURE_SENSOR_0_E;

        st = cpssPxDiagDeviceTemperatureSensorsSelectSet(dev, sensorType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, sensorType);

        /* 1.2. */
        st = cpssPxDiagDeviceTemperatureSensorsSelectGet(dev, &sensorTypeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(sensorType, sensorTypeGet,
                        "get another sensorType than was set: %d", dev);

        /* 1.1. */
        sensorType = CPSS_PX_DIAG_TEMPERATURE_SENSOR_AVERAGE_E;

        st = cpssPxDiagDeviceTemperatureSensorsSelectSet(dev, sensorType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, sensorType);

        /* 1.2. */
        st = cpssPxDiagDeviceTemperatureSensorsSelectGet(dev, &sensorTypeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(sensorType, sensorTypeGet,
                        "get another sensorType than was set: %d", dev);

        /* 1.1. */
        sensorType = CPSS_PX_DIAG_TEMPERATURE_SENSOR_MAX_E;

        st = cpssPxDiagDeviceTemperatureSensorsSelectSet(dev, sensorType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, sensorType);

        /* 1.2. */
        st = cpssPxDiagDeviceTemperatureSensorsSelectGet(dev, &sensorTypeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(sensorType, sensorTypeGet,
                        "get another sensorType than was set: %d", dev);

        /* 1.3. */
        UTF_ENUMS_CHECK_MAC(cpssPxDiagDeviceTemperatureSensorsSelectSet
                                    (dev, sensorType),
                                    sensorType);

        /* restore sensor type */
        st = cpssPxDiagDeviceTemperatureSensorsSelectSet(dev, sensorTypeSave);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, sensorTypeSave);
    }

    sensorType = CPSS_PX_DIAG_TEMPERATURE_SENSOR_0_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxDiagDeviceTemperatureSensorsSelectSet(dev, sensorType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxDiagDeviceTemperatureSensorsSelectSet(dev, sensorType);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxDiagDeviceTemperatureSensorsSelectGet
(
    IN  GT_SW_DEV_NUM                        devNum,
    OUT CPSS_PX_DIAG_TEMPERATURE_SENSOR_ENT  *sensorTypePtr
);
*/
UTF_TEST_CASE_MAC(cpssPxDiagDeviceTemperatureSensorsSelectGet)
{
/*
    ITERATE_DEVICES
    1.1. Call with non-NULL sensorTypePtr.
    Expected: GT_OK.
    1.2. Call with sensorTypePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    CPSS_PX_DIAG_TEMPERATURE_SENSOR_ENT sensorType = CPSS_PX_DIAG_TEMPERATURE_SENSOR_0_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
            /* 1.1. */
        st = cpssPxDiagDeviceTemperatureSensorsSelectGet(dev, &sensorType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /* 1.2. */
                st = cpssPxDiagDeviceTemperatureSensorsSelectGet(dev, NULL);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, sensorTypePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
            st = cpssPxDiagDeviceTemperatureSensorsSelectGet(dev, &sensorType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxDiagDeviceTemperatureSensorsSelectGet(dev, &sensorType);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxDiagDeviceTemperatureThresholdSet
(
    IN  GT_SW_DEV_NUM  devNum,
    IN  GT_32          thresholdValue
);
*/
UTF_TEST_CASE_MAC(cpssPxDiagDeviceTemperatureThresholdSet)
{
/*
    ITERATE_DEVICES
    1.1. Call with thresholdValue -277 \ 0 \ 198],
    Expected: GT_OK.
    1.2. Call GET function  with same params.
    Expected: GT_OK and same value as written.
    1.3. Call with out of range thresholdValue[-300 \ 250].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    GT_32       thresholdValue    = 0;
    GT_32       thresholdValueGet = 0;
    GT_32       thresholdSave;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* save threshould */
                st = cpssPxDiagDeviceTemperatureThresholdGet(dev, &thresholdSave);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* test low threshoulds on simulation only because it may shutdown board */
#ifdef ASIC_SIMULATION
       /* 1.1. */
        thresholdValue = -40;
        st = cpssPxDiagDeviceTemperatureThresholdSet(dev, thresholdValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, thresholdValue);

        /* 1.2. */
        st = cpssPxDiagDeviceTemperatureThresholdGet(dev, &thresholdValueGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        /* value not exact converted to HW */
        if (((thresholdValue + 1) < thresholdValueGet)  || ((thresholdValue - 1) > thresholdValueGet))
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(thresholdValue, thresholdValueGet,
                            "get another thresholdValue than was set: %d", dev);
        }

        /* 1.1. */
        thresholdValue = 0;

        st = cpssPxDiagDeviceTemperatureThresholdSet(dev, thresholdValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, thresholdValue);

        /* 1.2. */
        st = cpssPxDiagDeviceTemperatureThresholdGet(dev, &thresholdValueGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* value not exact converted to HW */
        if (((thresholdValue + 1) < thresholdValueGet)  || ((thresholdValue - 1) > thresholdValueGet))
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(thresholdValue, thresholdValueGet,
                            "get another thresholdValue than was set: %d", dev);
        }
#endif
        /* 1.1. */
        thresholdValue = 149;
        st = cpssPxDiagDeviceTemperatureThresholdSet(dev, thresholdValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, thresholdValue);

        /* 1.2. */
        st = cpssPxDiagDeviceTemperatureThresholdGet(dev, &thresholdValueGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* value not exact converted to HW */
        if (((thresholdValue + 1) < thresholdValueGet)  || ((thresholdValue - 1) > thresholdValueGet))
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(thresholdValue, thresholdValueGet,
                            "get another thresholdValue than was set: %d", dev);
        }

        /* 1.3. */
        thresholdValue = -300;
        st = cpssPxDiagDeviceTemperatureThresholdSet(dev, thresholdValue);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, thresholdValue);

        /* 1.3. */
        thresholdValue = 250;

        st = cpssPxDiagDeviceTemperatureThresholdSet(dev, thresholdValue);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, thresholdValue);

        thresholdValue = 0;

        /* restore thresholdValue */
        st = cpssPxDiagDeviceTemperatureThresholdSet(dev, thresholdSave);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, thresholdSave);
    }

    thresholdValue = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxDiagDeviceTemperatureThresholdSet(dev, thresholdValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxDiagDeviceTemperatureThresholdSet(dev, thresholdValue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxDiagDeviceTemperatureThresholdGet
(
    IN  GT_SW_DEV_NUM  devNum,
    OUT GT_32          *thresholdValuePtr
);
*/
UTF_TEST_CASE_MAC(cpssPxDiagDeviceTemperatureThresholdGet)
{
/*
    ITERATE_DEVICES
    1.1. Call with non-NULL thresholdValuePtr.
    Expected: GT_OK.
    1.2. Call with thresholdValuePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    GT_32       thresholdValue = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
            /* 1.1. */
        st = cpssPxDiagDeviceTemperatureThresholdGet(dev, &thresholdValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2. */
        st = cpssPxDiagDeviceTemperatureThresholdGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, thresholdValuePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxDiagDeviceTemperatureThresholdGet(dev, &thresholdValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxDiagDeviceTemperatureThresholdGet(dev, &thresholdValue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxDiagDeviceTemperatureGet
(
    IN  GT_SW_DEV_NUM    devNum,
    OUT GT_32            *temperaturePtr
);
*/
UTF_TEST_CASE_MAC(cpssPxDiagDeviceTemperatureGet)
{
/*
    ITERATE_DEVICES(Px)
    1.1. Call with not null temperaturePtr.
    Expected: GT_OK.
    1.2. Call api with wrong temperaturePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8  dev;
    GT_32  temperature;
    GT_BOOL didAnySensore;
    GT_U32  ii;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        didAnySensore = GT_FALSE;
        for(ii = 0 ; ii < (CPSS_PX_DIAG_TEMPERATURE_SENSOR_MAX_E+1); ii ++)
        {
            st = cpssPxDiagDeviceTemperatureSensorsSelectSet(dev,ii);
            if(st == GT_BAD_PARAM)
            {
                /* this sensor is not supported by the device */
                continue;
            }

            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            didAnySensore = GT_TRUE;

            /*
                1.1. Call with not null temperaturePtr.
                Expected: GT_OK.
            */
            st = cpssPxDiagDeviceTemperatureGet(dev, &temperature);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

#ifndef ASIC_SIMULATION
            /* make sure we get acceptable value that is not under 0 Celsius  and not above 200 */
            UTF_VERIFY_EQUAL2_STRING_MAC(0, (temperature <= 0 || temperature >= 200) ? 1 : 0,
                                       "(sensor[%d] --> temperature[%d] lower than 0 or higher than 200 degrees Celsius)",
                                       ii, temperature);
#endif
        }

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, didAnySensore,
                                   "didAnySensore = %d", didAnySensore);

        /*
            1.2. Call api with wrong temperaturePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxDiagDeviceTemperatureGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                   "%d, temperaturePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxDiagDeviceTemperatureGet(dev, &temperature);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxDiagDeviceTemperatureGet(dev, &temperature);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxDiagDeviceVoltageGet
(
    IN  GT_U8     devNum,
    IN  GT_U32    sensorNum,
    OUT GT_U32    *voltagePtr
);
*/
UTF_TEST_CASE_MAC(cpssPxDiagDeviceVoltageGet)
{
    GT_STATUS st = GT_OK;
    GT_U8  dev = 0;
    GT_U32 voltage_milivolts;
    GT_U32 sensorNum;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /*PRV_UTF_LOG0_MAC("starting Voltage sensor test\n");*/
    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*PRV_UTF_LOG1_MAC("dev=%d\n", dev);*/
        for (sensorNum = 0 ; sensorNum < 4 ; sensorNum++)
        {

            st = cpssPxDiagDeviceVoltageGet(dev, sensorNum, &voltage_milivolts );
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            PRV_UTF_LOG3_MAC("dev=%d sensorNum=%d mV=%d\n", dev, sensorNum, voltage_milivolts);
#ifdef ASIC_SIMULATION
            /* in WM for hwValue = 0 and divider_en =0 divider_cfg =1 calculated value should be 1208mV */
            if (GT_OK == st && voltage_milivolts != 1208)
#else
            if (GT_OK == st && (voltage_milivolts <= 900 || voltage_milivolts >= 1150 ))
#endif
            {
                st = GT_OUT_OF_RANGE;
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            }
        }

        /* 4. sensorNumber out-of-range */
        st = cpssPxDiagDeviceVoltageGet(dev, 4, &voltage_milivolts );
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* 5. NULL OUT ptr */
        st = cpssPxDiagDeviceVoltageGet(dev, 0, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxDiagDeviceVoltageGet(dev, 0, &voltage_milivolts );
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxDiagDeviceVoltageGet(dev, 0, &voltage_milivolts );
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssPxDiag suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssPxDiag)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxDiagRegsNumGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxDiagRegsDump)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxDiagResetAndInitControllerRegsNumGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxDiagResetAndInitControllerRegsDump)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxDiagRegWrite)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxDiagRegRead)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxDiagRegTest)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxDiagAllRegTest)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxDiagDeviceTemperatureSensorsSelectSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxDiagDeviceTemperatureSensorsSelectGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxDiagDeviceTemperatureThresholdSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxDiagDeviceTemperatureThresholdGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxDiagDeviceTemperatureGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxDiagDeviceVoltageGet)
UTF_SUIT_END_TESTS_MAC(cpssPxDiag)

