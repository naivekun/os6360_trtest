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
* @file cpssPpDrvHwPexMbusUT.c
*
* @brief Unit tests for PCI driver.
*
* @version   4
********************************************************************************
*/
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpssDriver/pp/config/prvCpssDrvPpCfgInit.h>
#include <cpssDriver/pp/hardware/prvCpssDrvHwCntl.h>
#include <cpssDriver/pp/hardware/prvCpssDrvObj.h>
#include <gtOs/gtOsTimer.h>
#ifdef ASIC_SIMULATION
#include <asicSimulation/SCIB/scib.h>
#endif /*ASIC_SIMULATION*/

/* PEX_MBUS_ADDR_COMP_REG_MAC(_index) - address of address completion register */

/* address completion registers shadow */
#define PRV_COMPLETION_REG_SHADOW_MAC(_devNum, _portGroupId, _index) \
    prvCpssDrvPpConfig[_devNum]->hwCtrl[_portGroupId].addrCompletShadow[_index]
/* bitmap of indexes of address completion registers used by interrupt handler */

#define PRV_COMPLETION_REG_ISR_BMP_MAC(_devNum, _portGroupId) \
    prvCpssDrvPpConfig[_devNum]->hwCtrl[_portGroupId].isrAddrCompletionRegionsBmp

/* bitmap of indexes of address completion registers used by application */
#define PRV_COMPLETION_REG_APP_BMP_MAC(_devNum, _portGroupId) \
    prvCpssDrvPpConfig[_devNum]->hwCtrl[_portGroupId].appAddrCompletionRegionsBmp



/**
* @internal prvCpssPpDrvHwPexMbusComplRegRegionsAmountGet function
* @endinternal
*
* @brief   The function gets amount of address completion regions according to mode
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Puma2; Puma3.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - Port Group Id for multi Port Group devices
*                                       8 for 8 regions mode, 4 on 4 regions mode and 0 on error.
*/
GT_U32 prvCpssPpDrvHwPexMbusComplRegRegionsAmountGet
(
    IN  GT_U8  devNum,
    IN  GT_U32 portGroupId
)
{
    GT_STATUS   rc;
    GT_U32      fieldData;

    PRV_CPSS_DRV_CHECK_BUS_FUNC_OBJ_MAC(devNum);
    PRV_CPSS_DRV_CHECK_ACTIVE_PORT_GROUP_ID_FOR_READ_OPERATION_MAC(
        devNum, portGroupId);

#ifdef ASIC_SIMULATION
    if (prvCpssDrvPpConfig[devNum]->hwCtrl[portGroupId].simInfo.addrCompletEnable == GT_FALSE)
    {
        return 0;
    }
#endif /*ASIC_SIMULATION*/

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        CPSS_TBD_BOOKMARK_FALCON
        cpssOsPrintf("prvCpssPpDrvHwPexMbusComplRegRegionsAmountGet : Sip6 not use MG address completion like sip5 \n");
        return 0;
    }


    /* get 8-region mode: regAddr = 0x140, set bit16 0- 8 regions, 1 - 4 regions */
    rc = prvCpssDrvHwPpPortGroupGetRegField(
        devNum, portGroupId,
        0x140 /*regAddr*/, 16/*fieldOffset*/, 1/*fieldLength*/,
        &fieldData);
    if (rc != GT_OK)
    {
        return 0;
    }
    return ((fieldData == 0) ? 8 : 4);
}

/**
* @internal cpssPpDrvHwPexMbusComplRegShadowDump function
* @endinternal
*
* @brief   The function dumps address completion registers
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Puma2; Puma3.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - Port Group Id for multi Port Group devices
*                                       OK on success, other on wrong parameters.
*/
GT_STATUS cpssPpDrvHwPexMbusComplRegShadowDump
(
    IN  GT_U8  devNum,
    IN  GT_U32 portGroupId
)
{
    GT_U32      i;

    PRV_CPSS_DRV_CHECK_BUS_FUNC_OBJ_MAC(devNum);
    PRV_CPSS_DRV_CHECK_ACTIVE_PORT_GROUP_ID_FOR_READ_OPERATION_MAC(
        devNum, portGroupId);

    cpssOsPrintf(
        "Address completion registers dump, Dev: %d, PortGroup:%d \n",
        devNum, portGroupId);
    for (i = 0; (i < 8); i++)
    {
        cpssOsPrintf(
            "%8.8X ", PRV_COMPLETION_REG_SHADOW_MAC(devNum, portGroupId, i));
    }
    cpssOsPrintf("\n");

    return GT_OK;
}

/**
* @internal cpssPpDrvHwPexMbusComplRegShadowCoherencyCheck function
* @endinternal
*
* @brief   The function checks address completion registers and it's shadow coherency
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Puma2; Puma3.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - Port Group Id for multi Port Group devices
*                                       OK on success, other on wrong parameters.
*/
GT_STATUS cpssPpDrvHwPexMbusComplRegShadowCoherencyCheck
(
    IN  GT_U8  devNum,
    IN  GT_U32 portGroupId
)
{
    GT_U32      i;
    GT_U32      regionBmp;
    GT_STATUS   rc;
    GT_U32      fieldData;

    PRV_CPSS_DRV_CHECK_BUS_FUNC_OBJ_MAC(devNum);
    PRV_CPSS_DRV_CHECK_ACTIVE_PORT_GROUP_ID_FOR_READ_OPERATION_MAC(
        devNum, portGroupId);

    regionBmp =
        (1 /* 0-th region that always points 0 */
        | PRV_COMPLETION_REG_ISR_BMP_MAC(devNum, portGroupId)
        | PRV_COMPLETION_REG_APP_BMP_MAC(devNum, portGroupId));

    for (i = 0; (i < 8); i++)
    {
        if (((1 << i) & regionBmp) == 0)
        {
            /* bypass registers reserved for other CPUs */
            continue;
        }

        rc = prvCpssDrvHwPpPortGroupGetRegField(
            devNum, portGroupId,
            PEX_MBUS_ADDR_COMP_REG_MAC(i) /*regAddr*/,
            0/*fieldOffset*/, 13/*fieldLength*/,
            &fieldData);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (fieldData != PRV_COMPLETION_REG_SHADOW_MAC(devNum, portGroupId, i))
        {
            return GT_BAD_STATE;
        }
    }

    return GT_OK;
}

#if 0
/**
* @internal prvcpssPpDrvHwPexMbusComplRegShadowFindAddr function
* @endinternal
*
* @brief   The function finds address completion in registers shadow
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Puma2; Puma3.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - Port Group Id for multi Port Group devices
* @param[in] regAddr                  - address to search it's completion
*
* @retval index of found region,   -1 on fail.
*/
static GT_U32 prvcpssPpDrvHwPexMbusComplRegShadowFindAddr
(
    IN  GT_U8  devNum,
    IN  GT_U32 portGroupId,
    IN  GT_U32 regAddr
)
{
    GT_U32      i;
    GT_U32      regionBmp;
    GT_U32      addrCompletion;

    regionBmp      = (1 | PRV_COMPLETION_REG_APP_BMP_MAC(devNum, portGroupId));
    addrCompletion = ((regAddr >> 19) & 0x1FFF);

    for (i = 0; (i < 8); i++)
    {
        /* bypass regions not dedicated to application use */
        if ((regionBmp & (1 << i)) == 0)
            continue;

        if (addrCompletion == PRV_COMPLETION_REG_SHADOW_MAC(devNum, portGroupId, i))
        {
            return i;
        }
    }

    return 0xFFFFFFFF;
}

/**
* @internal prvcpssPpDrvHwPexMbusComplRegReadSequence function
* @endinternal
*
* @brief   The function reads given sequence of addresses checking updates of
*         address completion registers shadow.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Puma2; Puma3.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - Port Group Id for multi Port Group devices
* @param[in] regAddrArr[]             - array of reister addresses
*                                       GT_OK on success, other on error.
*/
static GT_STATUS prvcpssPpDrvHwPexMbusComplRegReadSequence
(
    IN  GT_U8  devNum,
    IN  GT_U32 portGroupId,
    IN  GT_U32 regAmount,
    IN  GT_U32 regAddrArr[],
    IN  GT_U32 regDataArr[],
    OUT GT_U32 *regionBmpPtr
)
{
    GT_STATUS   rc;
    GT_U32      i;
    GT_U32      shadowIndex;

    PRV_CPSS_DRV_CHECK_BUS_FUNC_OBJ_MAC(devNum);
    PRV_CPSS_DRV_CHECK_ACTIVE_PORT_GROUP_ID_FOR_READ_OPERATION_MAC(
        devNum, portGroupId);

    *regionBmpPtr = 0;

    for (i = 0; (i < regAmount); i++)
    {
        rc = prvCpssDrvHwPpPortGroupReadRegister(
            devNum, portGroupId, regAddrArr[i], &(regDataArr[i]));
        if (rc != GT_OK)
        {
            return rc;
        }
        shadowIndex = prvcpssPpDrvHwPexMbusComplRegShadowFindAddr(
            devNum, portGroupId, regAddrArr[i]);
        if (shadowIndex == 0xFFFFFFFF)
        {
            /* index not found */
            return GT_FAIL;
        }
        *regionBmpPtr |= (1 << shadowIndex);
    }

    return GT_OK;
}

/**
* @internal prvcpssPpDrvHwPexMbusComplRegWriteSequence function
* @endinternal
*
* @brief   The function writes given sequence of addresses checking updates of
*         address completion registers shadow.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Puma2; Puma3.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - Port Group Id for multi Port Group devices
* @param[in] regAddrArr[]             - array of reister addresses
*                                      regAddrData      - array of reister data
*
* @param[out] regionBmpPtr             - (pointer to) bitmap of used completion region indexes
*                                       GT_OK on success, other on error.
*/
static GT_STATUS prvcpssPpDrvHwPexMbusComplRegWriteSequence
(
    IN  GT_U8  devNum,
    IN  GT_U32 portGroupId,
    IN  GT_U32 regAmount,
    IN  GT_U32 regAddrArr[],
    OUT GT_U32 regDataArr[],
    OUT GT_U32 *regionBmpPtr
)
{
    GT_STATUS   rc;
    GT_U32      i;
    GT_U32      shadowIndex;

    PRV_CPSS_DRV_CHECK_BUS_FUNC_OBJ_MAC(devNum);
    PRV_CPSS_DRV_CHECK_ACTIVE_PORT_GROUP_ID_FOR_READ_OPERATION_MAC(
        devNum, portGroupId);

    *regionBmpPtr = 0;

    for (i = 0; (i < regAmount); i++)
    {
        rc = prvCpssDrvHwPpPortGroupWriteRegister(
            devNum, portGroupId, regAddrArr[i], regDataArr[i]);
        if (rc != GT_OK)
        {
            return rc;
        }
        shadowIndex = prvcpssPpDrvHwPexMbusComplRegShadowFindAddr(
            devNum, portGroupId, regAddrArr[i]);
        if (shadowIndex == 0xFFFFFFFF)
        {
            /* index not found */
            return GT_FAIL;
        }
        *regionBmpPtr |= (1 << shadowIndex);
    }

    return GT_OK;
}
#endif

static GT_VOID prvBobcat2_8_AddressSequenceGet
(
    OUT GT_U32 regAddrArr[]
)
{
    /* addresses in different regions both for 4 and fo 8 regions modes            */
    regAddrArr[0] = 0x010000B0; /* TTI, IPCL TCP Port Range Comparator             */
    regAddrArr[1] = 0x02000080; /* PCL, L2/L3 Ingress VLAN Counting Enable%i       */
    regAddrArr[2] = 0x03000040; /* L2I, Bridge Security Breach drop counter cfg 0  */
    regAddrArr[3] = 0x04000324; /* FDB, Non multi-hash XOR result                  */
    regAddrArr[4] = 0x055021BC; /* TCAM, MG Hit Group %n Hit Num %i                */
    regAddrArr[5] = 0x06000364; /* IPVX, ECMP Seed                                 */
    regAddrArr[6] = 0x0B0000C0; /* IPLR0, Policer Initial DP<%n>                   */
    regAddrArr[7] = 0x0D007C00; /* EQ, IP Protocol CPU Code Entry <2*%n... 2*%n+1> */
}

/**
* @internal cpssPpDrvHwPexMbusTimeMeasureUtil function
* @endinternal
*
* @brief   Utility for measuring register read/write speed.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Puma2; Puma3.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - Port Group Id for multi Port Group devices
* @param[in] timeLimit                - time limit in milliseconds
* @param[in] sameAddress              - 1  - write and read the same address
*                                      0  - use addresses in different regions
* @param[in] measureReadOnly          - 1  - measure Read Speed Only
*                                      0  - measure Both Read and Write Speed
* @param[in] measureWriteOnly         - 1  - measure Write Speed Only
*                                      0  - measure Both Read and Write Speed
*                                       GT_OK on success, other on error.
*/
GT_STATUS cpssPpDrvHwPexMbusTimeMeasureUtil
(
    IN  GT_U8     devNum,
    IN  GT_U32    portGroupId,
    IN  GT_U32    timeLimit,
    IN  GT_U32    sameAddress,
    IN  GT_U32    measureReadOnly,
    IN  GT_U32    measureWriteOnly
)
{
    GT_STATUS   rc;
    GT_U32      regAmount = 8;
    GT_U32      regAddrArr[8];
    GT_U32      regDataArr[8];
    GT_U32      i;
    GT_U32      numOfCycles = 0;
    GT_U32      startSeconds;
    GT_U32      startNanoSeconds;
    GT_U32      seconds;
    GT_U32      nanoSeconds;
    GT_U32      startInMs = 0;
    GT_U32      timeInMs = 0;
    GT_U32      readsNum;
    GT_U32      writesNum;

    if (measureReadOnly && measureWriteOnly)
    {
        return GT_BAD_PARAM;
    }

    /* get 8 addresses in different regions */
    prvBobcat2_8_AddressSequenceGet(regAddrArr);
    if (sameAddress)
    {
        for (i = 1; (i < 8); i++)
        {
            regAddrArr[i] = regAddrArr[0];
        }
    }

    timeInMs = 0;
    startSeconds = 0xFFFFFFFF;
    startNanoSeconds = 0xFFFFFFFF;
    osTimeRT(&startSeconds, &startNanoSeconds);
    if ((startSeconds == 0xFFFFFFFF) || (startNanoSeconds == 0xFFFFFFFF))
    {
        /* wrong exact time support in system */
        return GT_FAIL;
    }
    startInMs = (((startSeconds & 0xFF) * 1000) + (startNanoSeconds / 1000000));
    cpssOsPrintf("starting 8-reads + 8-writes cycles, startInMs: %d\n", startInMs);

    for (numOfCycles = 0; (1); numOfCycles++)
    {
        /* check time */
        if ((numOfCycles & 0xFFF) == 0)
        {
            osTimeRT(&seconds, &nanoSeconds);
            timeInMs = (((seconds & 0xFF) * 1000) + (nanoSeconds / 1000000));
            if (timeInMs > (startInMs + timeLimit))
                break;
        }

        /* when measuring Write Only read done once only */
        /* to retrieve values that will be written       */
        if ((measureWriteOnly == 0) || (numOfCycles == 0))
        {
            /* read from all addresses */
            for (i = 0; (i < regAmount); i++)
            {
                rc = prvCpssDrvHwPpPortGroupReadRegister(
                    devNum, portGroupId, regAddrArr[i], &(regDataArr[i]));
                if (rc != GT_OK)
                {
                    cpssOsPrintf("read error\n");
                    return rc;
                }
            }
        }

        if (measureReadOnly == 0)
        {
            /* write to all addresses */
            for (i = 0; (i < regAmount); i++)
            {
                rc = prvCpssDrvHwPpPortGroupWriteRegister(
                    devNum, portGroupId, regAddrArr[i], regDataArr[i]);
                if (rc != GT_OK)
                {
                    cpssOsPrintf("write error\n");
                    return rc;
                }
            }
        }
    }

    cpssOsPrintf("test time [%d] Milliseconds\n", (timeInMs - startInMs));
    readsNum  = (measureWriteOnly ? 0 : (numOfCycles * 8));
    writesNum = (measureReadOnly ?  0 : (numOfCycles * 8));
    cpssOsPrintf("reads %d, writes %d\n", readsNum, writesNum);
    cpssOsPrintf(
        "average time in nanoseconds %d\n",
        (((timeInMs - startInMs) * 1000000) / (readsNum + writesNum)));

    return GT_OK;
}


#if 0
UTF_TEST_CASE_MAC(cpssPpDrvHwPexMbus8ComplRegShadowCoherency)
{
    GT_STATUS   rc;
    GT_U8       devNum;
    GT_U32      notAppFamilyBmp = 0;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum,  notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        if (8 != prvCpssPpDrvHwPexMbusComplRegRegionsAmountGet(
            devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS))
        {
            continue;
        }
        cpssPpDrvHwPexMbusComplRegShadowDump(
            devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS);
        rc = cpssPpDrvHwPexMbusComplRegShadowCoherencyCheck(
            devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, devNum);
    }
}

UTF_TEST_CASE_MAC(cpssPpDrvHwPexMbus8ComplRegFullUsing)
{
    GT_STATUS   rc;
    GT_U8       devNum;
    GT_U32      portGroupId = 0;
    GT_U32      notAppFamilyBmp = 0;
    GT_U32      regAmount = 8;
    GT_U32      regAddrArr[8];
    GT_U32      regDataArr[8];
    GT_U32      appRegionBmp;
    GT_U32      usedRegionBmp;

    /* get 8 addresses in different regions */
    prvBobcat2_8_AddressSequenceGet(regAddrArr);

    /* test for Bobcat2 only */
    notAppFamilyBmp = (GT_U32)(~ (UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E));

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum,  notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        if (8 != prvCpssPpDrvHwPexMbusComplRegRegionsAmountGet(
            devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS))
        {
            continue;
        }

        appRegionBmp  = PRV_COMPLETION_REG_APP_BMP_MAC(devNum, portGroupId);
        usedRegionBmp = 0; /* fix compiler warning */

        /* read data sequence */
        rc = prvcpssPpDrvHwPexMbusComplRegReadSequence(
            devNum, portGroupId, regAmount,
            regAddrArr, regDataArr, &usedRegionBmp);
        PRV_UTF_LOG1_MAC("read usedRegionBmp: 0x%X\n", usedRegionBmp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, devNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(appRegionBmp, usedRegionBmp, devNum);

        /* write data sequence - data retieved by read */
        rc = prvcpssPpDrvHwPexMbusComplRegWriteSequence(
            devNum, portGroupId, regAmount,
            regAddrArr, regDataArr, &usedRegionBmp);
        PRV_UTF_LOG1_MAC("write usedRegionBmp: 0x%X\n", usedRegionBmp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, devNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(appRegionBmp, usedRegionBmp, devNum);
    }
}
#endif

UTF_TEST_CASE_MAC(cpssPpDrvHwPexMbus8ComplRegBurst)
{
    GT_STATUS   rc;
    GT_U8       devNum;
    GT_U32      portGroupId = 0;
    GT_U32      notAppFamilyBmp = 0;
    GT_U32      regAddr;
    GT_U32      regPatternArr[16];
    GT_U32      regSaveArr[16];
    GT_U32      regDataArr[16];
    GT_U32      i;
    GT_U32      memCmpRc;

    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* Bobcat2 LPM memory - 0x140000 bytes - more then one region */
    /* use end of 0-th and beginning of 1-th  completion regions  */
    /* 32 bytes in the first and 32 bytes in the second region    */
    regAddr = (0x58000000 + 0x80000 - 32);

    for (i = 0; (i < 16); i++)
    {
        /* initialize pattern to write and reread */
        regPatternArr[i] = i;
    }

    /* test for Bobcat2 and Bobcat3 with big LPM memory */
    notAppFamilyBmp = (GT_U32)(~ (UTF_BOBCAT2_E | UTF_BOBCAT3_E));

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum,  notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        if (8 != prvCpssPpDrvHwPexMbusComplRegRegionsAmountGet(
            devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS))
        {
            continue;
        }

        if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
        {
            regAddr = (0x44000000 + 0x80000 - 32);
        }

        PRV_UTF_LOG0_MAC(" Pattern write/reread test \n");

        /* save found data */
        rc = prvCpssDrvHwPpPortGroupReadRam(
            devNum, portGroupId,
            regAddr, 16/*length*/, regSaveArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, devNum);

        /* write pattern */
        rc = prvCpssDrvHwPpPortGroupWriteRam(
            devNum, portGroupId,
            regAddr, 16/*length*/, regPatternArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, devNum);

        /* read and check full pattern */
        rc = prvCpssDrvHwPpPortGroupReadRam(
            devNum, portGroupId,
            regAddr, 16/*length*/, regDataArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, devNum);
        memCmpRc = cpssOsMemCmp(regPatternArr, regDataArr, sizeof(regPatternArr));
        UTF_VERIFY_EQUAL1_PARAM_MAC(0, memCmpRc, devNum);
        if (memCmpRc != 0)
        {
            PRV_UTF_LOG0_MAC("written: ");
            for (i = 0; (i < 16); i++)
            {
                PRV_UTF_LOG1_MAC("%X ", regPatternArr[i]);
            }
            PRV_UTF_LOG0_MAC("\n");
            PRV_UTF_LOG0_MAC("read: ");
            for (i = 0; (i < 16); i++)
            {
                PRV_UTF_LOG1_MAC("%X ", regDataArr[i]);
            }
            PRV_UTF_LOG0_MAC("\n");
        }

        /* read and check pattern in two 8-words parts */
        rc = prvCpssDrvHwPpPortGroupReadRam(
            devNum, portGroupId,
            regAddr, 8/*length*/, regDataArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, devNum);
        rc = prvCpssDrvHwPpPortGroupReadRam(
            devNum, portGroupId,
            (regAddr + 32), 8/*length*/, &(regDataArr[8]));
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, devNum);
        memCmpRc = cpssOsMemCmp(regPatternArr, regDataArr, sizeof(regPatternArr));
        UTF_VERIFY_EQUAL1_PARAM_MAC(0, memCmpRc, devNum);
        if (memCmpRc != 0)
        {
            PRV_UTF_LOG0_MAC("written: ");
            for (i = 0; (i < 16); i++)
            {
                PRV_UTF_LOG1_MAC("%X ", regPatternArr[i]);
            }
            PRV_UTF_LOG0_MAC("\n");
            PRV_UTF_LOG0_MAC("read: ");
            for (i = 0; (i < 16); i++)
            {
                PRV_UTF_LOG1_MAC("%X ", regDataArr[i]);
            }
            PRV_UTF_LOG0_MAC("\n");
        }

        /* restore found data */
        rc = prvCpssDrvHwPpPortGroupWriteRam(
            devNum, portGroupId,
            regAddr, 16/*length*/, regSaveArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, devNum);
    }
}

UTF_TEST_CASE_MAC(cpssPpDrvHwPexMbus8ComplRegFullUseTiming)
{
    GT_U8       devNum;
    GT_U32      portGroupId = 0;
    GT_U32      notAppFamilyBmp = 0;

    /* test for Bobcat2 only */
    notAppFamilyBmp = (GT_U32)(~ (UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E));

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum,  notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        if (8 != prvCpssPpDrvHwPexMbusComplRegRegionsAmountGet(
            devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS))
        {
            continue;
        }

        cpssPpDrvHwPexMbusTimeMeasureUtil(
            devNum, portGroupId,
            200 /*timeLimit*/, 0/*sameAddress*/,
            0/*measureReadOnly*/, 0/*measureWriteOnly*/);
    }
}


/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssPpDrvHwPexMbus suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssPpDrvHwPexMbus)
#if 0
    UTF_SUIT_DECLARE_TEST_MAC(cpssPpDrvHwPexMbus8ComplRegShadowCoherency)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPpDrvHwPexMbus8ComplRegFullUsing)
#endif
    UTF_SUIT_DECLARE_TEST_MAC(cpssPpDrvHwPexMbus8ComplRegBurst)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPpDrvHwPexMbus8ComplRegFullUseTiming)
UTF_SUIT_END_TESTS_MAC(cpssPpDrvHwPexMbus)



