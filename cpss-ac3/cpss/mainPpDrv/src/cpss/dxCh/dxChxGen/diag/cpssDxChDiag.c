/*******************************************************************************
*              (c), Copyright 2006, Marvell International Ltd.                 *
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
* @file cpssDxChDiag.c
*
* @brief CPSS DXCH Diagnostic API
*
* PRBS sequence for tri-speed ports:
* 1. Enable PRBS checker on receiver port (cpssDxChDiagPrbsPortCheckEnableSet)
* 2. Check that checker initialization is done (cpssDxChDiagPrbsPortCheckReadyGet)
* 3. Set CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS_E transmit mode on transmiting port
* (cpssDxChDiagPrbsPortTransmitModeSet)
* 4. Enable PRBS generator on transmiting port (cpssDxChDiagPrbsPortGenerateEnableSet)
* 5. Check results on receiving port (cpssDxChDiagPrbsPortStatusGet)
*
* PRBS sequence for XG ports:
* 1. Set CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS7_E or CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS23_E
* transmit mode on both ports(cpssDxChDiagPrbsPortTransmitModeSet)
* 2. Enable PRBS checker on receiver port (cpssDxChDiagPrbsPortCheckEnableSet)
* 3. Enable PRBS generator on transmiting port (cpssDxChDiagPrbsPortGenerateEnableSet)
* 4. Check results on receiving port (cpssDxChDiagPrbsPortStatusGet)
*
* @version   102
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/diag/private/prvCpssDxChDiagLog.h>
#include <cpss/dxCh/dxChxGen/diag/cpssDxChDiag.h>
#include <cpss/dxCh/dxChxGen/diag/private/prvCpssDxChDiag.h>
#include <cpss/dxCh/dxChxGen/diag/private/prvCpssDxChDiagDataIntegrityMainMappingDb.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwTables.h>
#include <cpss/generic/diag/private/prvCpssGenDiag.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPort.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/pcl/private/prvCpssDxChPcl.h>
#include <cpss/dxCh/dxChxGen/private/routerTunnelTermTcam/prvCpssDxChRouterTunnelTermTcam.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpssDriver/pp/hardware/cpssDriverPpHw.h>

#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>

#include <cpss/generic/diag/private/serdesOpt/SerdesRxOptimizer.h>
#include <cpss/generic/diag/private/serdesOpt/private/serdesOptPrivate.h>
#include <cpss/generic/diag/private/serdesOpt/private/SerdesRxOptAppIf.h>

#include <mvDdr3TrainingIpDb.h>
#include <mvDdr3TrainingIpDef.h>
#include <mvDdr3TrainingIpBist.h>
#include <mvDdr3TrainingIpFlow.h>
#include <mvDdr3TrainingIpEngine.h>
#include <mvHwsDdr3Bc2.h>

#ifdef OS_MALLOC_MEMORY_LEAKAGE_DBG
static GT_VOID *myCpssOsMalloc
(
    IN GT_U32 size
)
{
    return cpssOsMalloc_MemoryLeakageDbg(size,__FILE__,__LINE__);
}
static GT_VOID myCpssOsFree
(
    IN GT_VOID* const memblock
)
{
    cpssOsFree_MemoryLeakageDbg(memblock,__FILE__,__LINE__);
}
#endif /*OS_MALLOC_MEMORY_LEAKAGE_DBG*/

#ifdef CPSS_DEBUG
#define CPSS_DEBUG_DIAG
#endif

#ifdef CPSS_DEBUG_DIAG
#define DBG_INFO(x)     cpssOsPrintf x
#else
#define DBG_INFO(x)
#endif

#define BUFFER_DRAM_PAGE_SIZE_CNS 64        /* buffer DRAM page size in bytes */
/* buffer DRAM page size in 32 bit words */
#define BUFFER_DRAM_PAGE_SIZE_IN_WORDS_CNS (BUFFER_DRAM_PAGE_SIZE_CNS / 4)
#define LION_BUFFER_DRAM_PAGE_SIZE_CNS 128  /* buffer DRAM page (line) size in bytes */
#define MAC_ENTRY_SIZE_CNS        16        /* MAC table entry size in bytes  */
#define CH_VLAN_ENTRY_SIZE_CNS    12        /* VLAN table entry size for CH in bytes */
#define CH2_VLAN_ENTRY_SIZE_CNS   16        /* VLAN table entry size for CH2,3.. in bytes */

/* VLAN table entry size for XCAT in bytes, the real "used" size is 179 bits,
   6 words */
#define XCAT_VLAN_ENTRY_SIZE_CNS  32

/* Egress VLAN Table entry size for Lion B0, Lion2 in bytes according to
   address space alignment, the real "used" size is 267 bits, 9 words */
#define LION_B0_EGR_VLAN_ENTRY_SIZE_CNS    64

/* macro to get the alignment as number of words from the number of bits in the entry */
#define BITS_TO_BYTES_ALIGNMENT_MAC(bits) \
    (((bits) > 512) ? 128 :      \
     ((bits) > 256) ?  64 :      \
     ((bits) > 128) ?  32 :      \
     ((bits) >  64) ?  16 :      \
     ((bits) >  32) ?   8 :   4)


/* The macro checks function return status and returns it if it's not GT_OK */
#define PRV_CH_DIAG_CHECK_RC_MAC(status)    \
    GT_STATUS retStatus = (status);         \
    if(retStatus != GT_OK)                  \
        return retStatus

#define PRV_CH_DIAG_MINIMAL_SLEEP_TIME_CNS 1 /* minimal sleep time in miliseconds */

/* default delay for serdes optimization algorithm */
#define PRV_CPSS_DXCH_DIAG_SERDES_TUNE_PRBS_DEFAULT_DELAY_CNS 200

/* should serdes RX optimization algorithm pring intermidiate reports */
static GT_BOOL diagSerdesTuningTracePrintEnable = GT_FALSE;

#define PRV_DXCH_DIAG_MAX_XPCS_LANES_NUM_MAC(devNum)                               \
(GT_U32)(((CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily) ||   \
          (PRV_CPSS_SIP_5_CHECK_MAC(devNum))) ? \
          (PRV_CPSS_DXCH_LION2_NUM_LANES_CNS-1) : (PRV_CPSS_DXCH_NUM_LANES_CNS-1))

/* TCAM Compare BIST: Parameters and Expected Result Configurations */
/* from AN-294: TCAM BIST Sequence for the 98CX8265 and 98CX8266 Devices */
static CPSS_DIAG_PP_MEM_BIST_COMPARE_MEM_DATA_STC
    prvCpssDxChBistCompareMemDataArray[CPSS_DIAG_PP_MEM_BIST_COMPARE_TESTS_NUM_CNS] =
{
{0x1E3233, 0x1E3033, 0xB41},
{0x1E3433, 0x1E3233, 0xB41},
{0x1E3633, 0x000000, 0xB41},
{0x61F20F, 0x61F00F, 0x6341},
{0x61F40F, 0x61F20F, 0x6341},
{0x61F60F, 0x000000, 0x6341},
{0x4CF21B, 0x4CF01B, 0xF11},
{0x4CF61B, 0x000000, 0xF11},
{0x7E3333, 0x7E3133, 0x11},
{0x7E3533, 0x7E3333, 0x11},
{0x7E3733, 0x000000, 0x11},
{0xFE330F, 0xFE310F, 0x41},
{0xFE350F, 0xFE330F, 0x41},
{0xFE370F, 0x000000, 0x41},
{0x1E3203, 0x1E3003, 0xB45},
{0x1E3403, 0x1E3203, 0xB45},
{0x1E3603, 0x000000, 0xB45},
{0x06723F, 0x06703F, 0x6F50},
{0x06763F, 0x000000, 0x6F50},
{0xFE3333, 0xFE3133, 0x1},
{0xFE3533, 0xFE3333, 0x1},
{0xFE3733, 0x000000, 0x1},
{0x79B30F, 0x79B10F, 0x1},
{0x79B50F, 0x79B30F, 0x1},
{0x79B70F, 0x000000, 0x1}
};

/**
* @internal prvCpssDxChDiagMemoryBistClearMemAfterTest function
* @endinternal
*
* @brief   Clear memory after runing BIST (Built-in self-test) on specified memory.
*
* @note   APPLICABLE DEVICES:      Lion.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] memBistType              - type of memory
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS prvCpssDxChDiagMemoryBistClearMemAfterTest
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               portGroupId,
    IN  CPSS_DIAG_PP_MEM_BIST_TYPE_ENT       memBistType
)
{
    GT_STATUS   rc;                   /* return code  */
    GT_U32      iterations;           /* Counter for loops */
    GT_U32      maxIterations;        /* maximum iterations */
    static GT_U32 zeroBuff[6] = {0};  /* zero buffer */

    if (memBistType == CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E)
    {
        maxIterations = (0 == PRV_CPSS_DXCH_LION_FAMILY_CHECK_MAC(devNum)) ?
                                    (4*PRV_DXCH3_PCL_TCAM_RAW_NUMBER_CNS ) : _3K;
        DBG_INFO(("memClear:maxIterations=%d\n", maxIterations));
        for( iterations = 0; iterations < maxIterations; iterations++)
        {
            /*  Resets the TCAM data to the default value */
            rc = prvCpssDxChPclTcamStdRuleXandYSet(
                    devNum, portGroupId,
                    iterations, zeroBuff, zeroBuff,
                    1 /* xValid0 */,1 /* yValid0 */,1 /* xValid */,1 /* yValid */,
                    0 /* xSize */,0 /* ySize */,0 /* xSpare */,0 /* ySpare  */);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    else if (memBistType == CPSS_DIAG_PP_MEM_BIST_ROUTER_TCAM_E)
    {
        maxIterations = PRV_CPSS_DXCH_ROUTER_TUNNEL_TERM_GET_MAX_SIZE_MAC(devNum);
        DBG_INFO(("memClear:maxIterations=%d\n", maxIterations));
        for( iterations = 0; iterations < maxIterations ; iterations++)
        {
            rc = prvCpssDxChRouterTunnelTermTcamInvalidateLine(
                    devNum,portGroupId,iterations);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChDiagMemoryBistPureMemTestRun function
* @endinternal
*
* @brief   Runs Pure Memory BIST (Built-in self-test) on specified memory.
*
* @note   APPLICABLE DEVICES:      Lion.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] memBistType              - type of memory
* @param[in] timeOut                  - maximal time in milisecond to wait for BIST finish.
* @param[in] clearMemoryAfterTest     - GT_TRUE  - Clear memory after test.
*                                      GT_FALSE - Don't clear memory after test.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_UNFIXABLE_BIST_ERROR  - on unfixable problem in the TCAM
*/
static GT_STATUS prvCpssDxChDiagMemoryBistPureMemTestRun
(
    IN  GT_U8                                devNum,
    IN  CPSS_DIAG_PP_MEM_BIST_TYPE_ENT       memBistType,
    IN  GT_U32                               timeOut,
    IN  GT_BOOL                              clearMemoryAfterTest
)
{
    GT_STATUS   rc;                   /* return code  */
    GT_U32      tcamTestConfigAddr;   /* Policy TCAM Test Config Status Register */
    GT_U32      tcamConfig0Addr;        /* TCAM Config0 Register */
    GT_U32      interruptCauseReg;      /* interrupt cause register - ECC interrupt */
    GT_U32      bistOutcome;          /* TCAM Bist outcome */
    GT_U32      regData;              /* register data */
    GT_U32      iterations;           /* Counter for loops */
    GT_U32      maxIterations;        /* maximum iterations */
    GT_U32      value = 0;
    GT_U32      portGroupId;
    GT_BOOL     testFailed = GT_FALSE;

    switch (memBistType)
    {
        case CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E:
            tcamTestConfigAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                pclRegs.policyTcamTest.configStatusReg;
            interruptCauseReg = (0 == PRV_CPSS_DXCH_LION_FAMILY_CHECK_MAC(devNum))
                                    ? 0xB800010 : 0xB800004;
            tcamConfig0Addr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                pclRegs.pclTcamConfig0;
            DBG_INFO(("PureMemTest:CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E \n"));
            break;
        case CPSS_DIAG_PP_MEM_BIST_ROUTER_TCAM_E:
            tcamTestConfigAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                ipRegs.routerTcamTest.configStatusReg;
            interruptCauseReg = (0 == PRV_CPSS_DXCH_LION_FAMILY_CHECK_MAC(devNum))
                                    ? 0x02040130 : 0x0D800060;
            tcamConfig0Addr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                ipRegs.ipTcamConfig0;
            DBG_INFO(("PureMemTest:CPSS_DIAG_PP_MEM_BIST_ROUTER_TCAM_E \n"));
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    maxIterations = timeOut / PRV_CH_DIAG_MINIMAL_SLEEP_TIME_CNS ;

    /* small delay for case of repeating test calls */
    cpssOsTimerWkAfter(1000);

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
    {
        if(memBistType == CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E)
        {
            /* set maximum TCAM Table size */
            rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,portGroupId,tcamConfig0Addr,1, 4, &value);
            if (rc != GT_OK)
            {
                return rc;
            }

            if (value != 0)
            {
                rc = prvCpssDrvHwPpPortGroupSetRegField(devNum,portGroupId,tcamConfig0Addr,1, 4, 0);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }

        /* Trigger MBist */
        rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum, portGroupId, tcamTestConfigAddr, 0x30005);
        if( rc != GT_OK )
        {
            return rc;
        }

        cpssOsTimerWkAfter(1000);

        iterations = 0;
        /* now busy wait until the BIST is finished */
        do
        {
            /* check that number of iteration not over the limit */
            if (iterations++ >= maxIterations)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, LOG_ERROR_NO_MSG);
            }
            cpssOsTimerWkAfter(PRV_CH_DIAG_MINIMAL_SLEEP_TIME_CNS);

            rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId, tcamTestConfigAddr, &regData);
            if( rc != GT_OK )
            {
                return rc;
            }

        } while ((regData & 0x8)== 0);

        bistOutcome = (regData & 0x70) >> 4;
        DBG_INFO(("portGroupId = %d, MBIST bistOutcome = %d \n", portGroupId, bistOutcome));
        if (bistOutcome != 0 && bistOutcome != 3 &&
             bistOutcome != 4 && bistOutcome != 5)
        {
            testFailed = GT_TRUE;
        }

        /* Return to normal operation mode */
        rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum, portGroupId, tcamTestConfigAddr, 0);
        if( rc != GT_OK )
        {
            return rc;
        }

        /* Clear on read the ECC interrupt */
        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId, interruptCauseReg, &regData);
        if( rc != GT_OK )
        {
            return rc;
        }

        if (GT_TRUE == clearMemoryAfterTest)
        {
            rc = prvCpssDxChDiagMemoryBistClearMemAfterTest(devNum,portGroupId,memBistType);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        if(memBistType == CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E)
        {
            if (value != 0)
            {/* restore TCAM table size if needed */
                rc = prvCpssDrvHwPpPortGroupSetRegField(devNum,portGroupId,tcamConfig0Addr,1, 4, value);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)

    if(GT_TRUE == testFailed)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_UNFIXABLE_BIST_ERROR, LOG_ERROR_NO_MSG);

    return GT_OK;
}

/**
* @internal prvCpssDxChDiagMemoryBistCompareMemTestRun function
* @endinternal
*
* @brief   Runs Compare Memory BIST (Built-in self-test) on specified memory.
*
* @note   APPLICABLE DEVICES:      Lion.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] memBistType              - type of memory
* @param[in] timeOut                  - maximal time in milisecond to wait for BIST finish.
* @param[in] clearMemoryAfterTest     - GT_TRUE  - Clear memory after test.
*                                      GT_FALSE - Don't clear memory after test.
* @param[in] testsToRunBmpPtr         - bitmap of required test related for
*                                      CPSS_DIAG_PP_MEM_BIST_COMPARE_TEST_E
*                                      (from 0 to CPSS_DIAG_PP_MEM_BIST_COMPARE_TESTS_NUM_CNS,
*                                      so minimum bitmap size 32 bits, maximum depends on future
*                                      number of tests); 1 - run test, 0 - don't run test.
*
* @param[out] testsResultBmpPtr        - pointer to bitmap with results of compare memory test,
*                                      !!! pay attention: 0 - test pass, 1 - test failed;
*                                      if NULL, just final result of test will be returned;
*                                      regarding size see testsToRunBmpPtr above
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_UNFIXABLE_BIST_ERROR  - on unfixable problem in the TCAM
*
* @note 1. This function should not be called under traffic.
*       2. Bist destroys the content of the memory. If clearMemoryAfterTest set
*       function will clear the content of the memory after the test.
*       Application responsible to restore the content after the function
*       completion.
*
*/
static GT_STATUS prvCpssDxChDiagMemoryBistCompareMemTestRun
(
    IN  GT_U8                                devNum,
    IN  CPSS_DIAG_PP_MEM_BIST_TYPE_ENT       memBistType,
    IN  GT_U32                               timeOut,
    IN  GT_BOOL                              clearMemoryAfterTest,
    IN  GT_U32                               *testsToRunBmpPtr,
    OUT GT_U32                               *testsResultBmpPtr
)
{
    GT_STATUS   rc;                   /* return code  */
    GT_U32      tcamTestConfigAddr;   /* Policy TCAM Test Config Status Register */
    GT_U32      tcamOpcodeExtHitReAddr; /* TCAM Opcode Expected Hit Rel Register */
    GT_U32      tcamOpcodeCommandAddr;  /* TCAM Opcode Command Register */
    GT_U32      tcamConfig0Addr;        /* TCAM Config0 Register */
    GT_U32      tcamOpcodeCommandData;  /* TCAM Opcode Command data */
    GT_U32      interruptCauseReg;      /* interrupt cause register - ECC interrupt */
    GT_U32      bistOutcome;          /* TCAM Bist outcome */
    GT_U32      regData;              /* register data */
    GT_U32      iterations;           /* Counter for loops */
    GT_U32      maxIterations;        /* maximum iterations */
    GT_U32      value;
    GT_U32      portGroupId;
    GT_U32      u32Bmp, i;
    GT_BOOL     testFailed = GT_FALSE;
    CPSS_DIAG_PP_MEM_BIST_COMPARE_MEM_DATA_STC *compBistDataPtr; /* pointer to
                                            element in BIST compare memory test array */

    switch (memBistType)
    {
        case CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E:
            tcamTestConfigAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                pclRegs.policyTcamTest.configStatusReg;
            tcamOpcodeExtHitReAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                pclRegs.policyTcamTest.opcodeExpectedHitReg;
            tcamOpcodeCommandAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                pclRegs.policyTcamTest.opcodeCommandReg;
            tcamOpcodeCommandData = 0x1E3233;
            interruptCauseReg = (0 == PRV_CPSS_DXCH_LION_FAMILY_CHECK_MAC(devNum))
                                    ? 0xB800010 : 0xB800004;
            tcamConfig0Addr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                pclRegs.pclTcamConfig0;
            DBG_INFO(("CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E \n"));
            break;
        case CPSS_DIAG_PP_MEM_BIST_ROUTER_TCAM_E:
            tcamTestConfigAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                ipRegs.routerTcamTest.configStatusReg;
            tcamOpcodeExtHitReAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                ipRegs.routerTcamTest.opcodeExpectedHitReg;
            tcamOpcodeCommandAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                ipRegs.routerTcamTest.opcodeCommandReg;
            tcamOpcodeCommandData = 0x1E3033;
            interruptCauseReg = (0 == PRV_CPSS_DXCH_LION_FAMILY_CHECK_MAC(devNum))
                                    ? 0x02040130 : 0x0D800060;
            tcamConfig0Addr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                ipRegs.ipTcamConfig0;
            DBG_INFO(("CPSS_DIAG_PP_MEM_BIST_ROUTER_TCAM_E \n"));
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    maxIterations = timeOut / PRV_CH_DIAG_MINIMAL_SLEEP_TIME_CNS ;

    /* small delay for case of repeating test calls */
    cpssOsTimerWkAfter(1000);

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
    {
        /* set maximum size of TCAM Table */
        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,portGroupId,tcamConfig0Addr,1, 4, &value);
        if (rc != GT_OK)
        {
            return rc;
        }

        if (value != 0)
        {
            rc = prvCpssDrvHwPpPortGroupSetRegField(devNum,portGroupId,tcamConfig0Addr,1, 4, 0);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        if(NULL != testsResultBmpPtr)
        {
            for(i = 0; i < ((CPSS_DIAG_PP_MEM_BIST_COMPARE_TESTS_NUM_CNS/32)+1); i++)
            {
                testsResultBmpPtr[i] = 0;
            }
        }

        for(i = 0; i < CPSS_DIAG_PP_MEM_BIST_COMPARE_TESTS_NUM_CNS; i++)
        {
            u32Bmp = testsToRunBmpPtr[i/CPSS_DIAG_PP_MEM_BIST_COMPARE_TESTS_NUM_CNS];

            if(0 == (u32Bmp & (1 << (i%CPSS_DIAG_PP_MEM_BIST_COMPARE_TESTS_NUM_CNS))))
                continue;

            compBistDataPtr = &prvCpssDxChBistCompareMemDataArray[i];

            /* Set CompBist parameters */
            tcamOpcodeCommandData = (memBistType == CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E) ?
                                            compBistDataPtr->pclOpCode : compBistDataPtr->rtrOpCode;
            if(0 == tcamOpcodeCommandData)
                continue;

            rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum, portGroupId, tcamOpcodeCommandAddr, tcamOpcodeCommandData);
            if( rc != GT_OK )
            {
                return rc;
            }

            /* Set CompBist expected results */
            rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum, portGroupId, tcamOpcodeExtHitReAddr,
                                                                    compBistDataPtr->expectedHitVal);
            if( rc != GT_OK )
            {
                return rc;
            }

            /* Trigger CompBist */
            rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum, portGroupId, tcamTestConfigAddr, 0x30006);
            if( rc != GT_OK )
            {
                return rc;
            }

            iterations = 0;
            /* now busy wait until the BIST is finished */
            do
            {
                /* check that number of iteration not over the limit */
                if (iterations++ >= maxIterations)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, LOG_ERROR_NO_MSG);
                }
                cpssOsTimerWkAfter(PRV_CH_DIAG_MINIMAL_SLEEP_TIME_CNS);

                rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId, tcamTestConfigAddr, &regData);
                if( rc != GT_OK )
                {
                    return rc;
                }

            } while ((regData & 0x8) == 0);

            bistOutcome = (regData & 0x70) >> 4;
            DBG_INFO(("test #%d portGroupId = %d, COMPBIST bistOutcome = %d \n", i, portGroupId, bistOutcome));

            if (bistOutcome != 0 && bistOutcome != 3 &&
                 bistOutcome != 4 && bistOutcome != 5)
            {
                if(NULL != testsResultBmpPtr)
                    testsResultBmpPtr[i/CPSS_DIAG_PP_MEM_BIST_COMPARE_TESTS_NUM_CNS] |= (1 << i);
                else
                    testFailed = GT_TRUE;
            }

        } /* end of for(i = 0; i < CPSS_DIAG_PP_MEM_BIST_COMPARE_TESTS_NUM_CNS; i++) */

        /* Return to normal operation mode */
        rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum, portGroupId, tcamTestConfigAddr, 0);
        if( rc != GT_OK )
        {
            return rc;
        }

        /* Clear on read the ECC interrupt */
        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId, interruptCauseReg, &regData);
        if( rc != GT_OK )
        {
            return rc;
        }

        if (GT_TRUE == clearMemoryAfterTest)
        {
            rc = prvCpssDxChDiagMemoryBistClearMemAfterTest(devNum,portGroupId,memBistType);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        if (value != 0)
        {/* restore TCAM table size if needed */
            rc = prvCpssDrvHwPpPortGroupSetRegField(devNum,portGroupId,tcamConfig0Addr,1, 4, value);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)

    if(NULL != testsResultBmpPtr)
    {
        for(i = 0; i < ((CPSS_DIAG_PP_MEM_BIST_COMPARE_TESTS_NUM_CNS/32)+1); i++)
        {
            if(testsResultBmpPtr[i] != 0)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_UNFIXABLE_BIST_ERROR, LOG_ERROR_NO_MSG);
        }
    }

    if(GT_TRUE == testFailed)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_UNFIXABLE_BIST_ERROR, LOG_ERROR_NO_MSG);

    return GT_OK;
}

/**
* @internal getMemBaseFromType function
* @endinternal
*
* @brief   This routine translates memType to PP base address.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] memType                  - The packet processor memory type to verify.
*
* @param[out] memBasePtr               - memory base address.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad memory type
* @retval GT_FAIL                  - on error
*
* @note Supported RAM types: only CPSS_DIAG_BUFFER_DRAM_E
*
*/
static GT_STATUS getMemBaseFromType
(
    IN  GT_U8                       devNum,
    IN CPSS_DIAG_PP_MEM_TYPE_ENT  memType,
    OUT GT_U32                     *memBasePtr
)
{
    CPSS_DXCH_TABLE_ENT tableType;

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        switch (memType)
        {
            case CPSS_DIAG_PP_MEM_VLAN_TBL_MEM_E:
                /* Egress VLAN Table      */
                tableType = CPSS_DXCH_LION_TABLE_VLAN_EGRESS_E;
                break;
            default:
                /* the device do not support CPU access to buffer memory */
                /* the device do not support CPU access to FDB memory */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        *memBasePtr = prvCpssDxChTableBaseAddrGet(devNum,tableType,NULL);
        return GT_OK;
    }


    switch (memType)
    {
        case CPSS_DIAG_PP_MEM_BUFFER_DRAM_E:
            if ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E)  ||
                (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E))
            {
                *memBasePtr = 0x06800000;
            }
            else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
            {
                *memBasePtr = 0x0C000000;
            }
            else if ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E))
            {
                *memBasePtr = 0x10000000;
            }
            else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
            {
                /* Lion2 do not support CPU access to buffer memory */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            else
            {
                *memBasePtr = 0x06900000;
            }
            break;

        case CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E:
            *memBasePtr = 0x06400000;
            break;

        case CPSS_DIAG_PP_MEM_VLAN_TBL_MEM_E:
            if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E ||
                PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
            {
                /* lion or lion2 devices */
                /* Egress VLAN Table */
                *memBasePtr = 0x11800000;
            }
            else if ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) ||
                     (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E))
            {
                /* xCat or xCat2 devices */
                *memBasePtr = 0x0A200000;
            }
            else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
            {
                /* xCat3 devices */
                *memBasePtr = 0x03A00000;
            }
            else
            {
                *memBasePtr = 0x0A400000;

            }

            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/**
* @internal getTableType function
* @endinternal
*
* @brief   Gets the table type from the memory address
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] addr                     - Memory address
*
* @param[out] tableTypePtr             - Table type
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
*/
static GT_STATUS getTableType
(
    IN GT_U8 devNum,
    IN  GT_U32                   addr,
    OUT CPSS_DXCH_TABLE_ENT  *tableTypePtr
)
{
    GT_U32  addrMsb = addr & 0xFFF00000;

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        if(addrMsb ==
            prvCpssDxChTableBaseAddrGet(devNum,CPSS_DXCH_LION_TABLE_VLAN_EGRESS_E,NULL))
        {
            /* Egress VLAN table is supported */
            *tableTypePtr = CPSS_DXCH_LION_TABLE_VLAN_EGRESS_E;
            return GT_OK;
        }

        if(addrMsb ==
            prvCpssDxChTableBaseAddrGet(devNum,CPSS_DXCH_TABLE_FDB_E,NULL))
        {
            /* FDB address -->
               NOTE: we will not get here because CPU can't access this table in those devices !!! */
            *tableTypePtr = CPSS_DXCH_TABLE_FDB_E;
            return GT_OK;
        }

        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }


    switch (addrMsb)
    {
        case 0x06400000:
            *tableTypePtr = CPSS_DXCH_TABLE_FDB_E;
            break;

        case 0x0A400000:
            if (! PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
            {
                *tableTypePtr = CPSS_DXCH_TABLE_VLAN_E;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }
            break;

        case 0x0A200000:
            if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) ||
               (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E))
            {
                /* xCat and xCat2 */
                *tableTypePtr = CPSS_DXCH_TABLE_VLAN_E;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }
            break;

        case 0x03A00000:
            if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
            {
                /* xCat3 */
                *tableTypePtr = CPSS_DXCH_TABLE_VLAN_E;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }
            break;

        case 0x11800000:
            if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E ||
                PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
            {
                /* Lion or Lion2 */
                /* Only Egress VLAN table is supported */
                *tableTypePtr = CPSS_DXCH_LION_TABLE_VLAN_EGRESS_E;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }
            break;
        case 0x11900000:
            if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
            {
                /* Lion2 */
                /* Egress VLAN table is supported */
                *tableTypePtr = CPSS_DXCH_LION_TABLE_VLAN_EGRESS_E;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/**
* @internal getEntrySize function
* @endinternal
*
* @brief   Gets table entry size for a table type
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] tableType                - table type
*
* @param[out] entrySizePtr             - entry size in bytes
*
* @retval GT_OK                    - on success
* @retval GT_NOT_SUPPORTED         - table type is not supported
*/
static GT_STATUS getEntrySize
(
    IN  GT_U8                   devNum,
    IN  CPSS_DXCH_TABLE_ENT tableType,
    OUT GT_U32                  *entrySizePtr
)
{
    PRV_CPSS_DXCH_TABLES_INFO_STC *tableInfoPtr;
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        switch(tableType)
        {
            case CPSS_DXCH_LION_TABLE_VLAN_EGRESS_E:
            case CPSS_DXCH_TABLE_FDB_E:
                tableInfoPtr =
                    &PRV_CPSS_DXCH_PP_MAC(devNum)->accessTableInfoPtr[tableType];
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }

        *entrySizePtr = BITS_TO_BYTES_ALIGNMENT_MAC((tableInfoPtr->entrySize*32));
        return GT_OK;
    }



    if (tableType == CPSS_DXCH_TABLE_VLAN_E)
    {
        if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_CHEETAH_E)
        {
            *entrySizePtr = CH_VLAN_ENTRY_SIZE_CNS;
        }
        else if (PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
        {
            /* Lion */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
        else if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
        {
            /* xCat, xCat3, xCat2 */
            *entrySizePtr = XCAT_VLAN_ENTRY_SIZE_CNS;
        }
        else
        {
            /* DxCh 2,3 and xCat A0 */
            *entrySizePtr = CH2_VLAN_ENTRY_SIZE_CNS;
        }
    }
    else if (tableType == CPSS_DXCH_LION_TABLE_VLAN_EGRESS_E)
    {
        if ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)||
            (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E))
        {
            /* Lion , Lion2 */
            *entrySizePtr = LION_B0_EGR_VLAN_ENTRY_SIZE_CNS;
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
    }
    else if (tableType == CPSS_DXCH_TABLE_FDB_E)
    {
        *entrySizePtr = MAC_ENTRY_SIZE_CNS;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/**
* @internal getEntryIndexAndWordNum function
* @endinternal
*
* @brief   Gets the entry index and the word number in order to access table field
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] tableType                - Table type
* @param[in] addr                     - memory address
*
* @param[out] entryIndexPtr            - index of the entry in the table
* @param[out] wordNumPtr               - number of entry for the address
*
* @retval GT_OK                    - on success
* @retval GT_NOT_SUPPORTED         - table type is not supported
* @retval GT_FAIL                  - on hardware error
*/
static GT_STATUS getEntryIndexAndWordNum
(
    IN  GT_U8                   devNum,
    IN  CPSS_DXCH_TABLE_ENT tableType,
    IN  GT_U32                  addr,
    OUT GT_U32                  *entryIndexPtr,
    OUT GT_U32                  *wordNumPtr
)
{
    GT_STATUS status;       /* returned status */
    GT_U32 entrySize;       /* entry size */
    GT_U32 memBase;         /* base address for the memory */

    /* Get memory base address */
    if ((tableType == CPSS_DXCH_TABLE_VLAN_E) ||
        (tableType == CPSS_DXCH_LION_TABLE_VLAN_EGRESS_E))
    {
        status = getMemBaseFromType (devNum, CPSS_DIAG_PP_MEM_VLAN_TBL_MEM_E, &memBase);
    }
    else if (tableType == CPSS_DXCH_TABLE_FDB_E)
    {
        status = getMemBaseFromType (devNum, CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E, &memBase);
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    if (status != GT_OK)
    {
        return status;
    }

    /* Get table entry size */
    status = getEntrySize (devNum, tableType, &entrySize);
    if (status != GT_OK)
    {
        return status;
    }

    /* Calculate table entry index */
    *entryIndexPtr = (addr - memBase) / entrySize;

    /* Calculate word number within table entry */
    *wordNumPtr = ((addr - memBase) % (entrySize)) / 4;

    return status;
}

/**
* @internal getEntryIndexAndNumOfEntries function
* @endinternal
*
* @brief   Gets the number of entries to access and first entry index in order to
*         access table field
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] addr                     - Address offset to read/write
* @param[in] devNum                   - device number
* @param[in] tableType                - Table type
* @param[in] entrySize                - entry size in bytes
* @param[in] length                   - Number of words (4 bytes) to read/write
*
* @param[out] firstEntryIndexPtr       - First entry index
* @param[out] numOfEntriesPtr          - number of entries to read/write
*
* @retval GT_OK                    - on success
* @retval GT_NOT_SUPPORTED         - table type is not supported
* @retval GT_FAIL                  - on hardware error
*/
static GT_STATUS getEntryIndexAndNumOfEntries
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  addr,
    IN  CPSS_DXCH_TABLE_ENT tableType,
    IN  GT_U32                  entrySize,
    IN  GT_U32                  length,
    OUT GT_U32                  *firstEntryIndexPtr,
    OUT GT_U32                  *numOfEntriesPtr
)
{
    GT_STATUS status;       /* returned status */
    GT_U32 memBase;         /* base address for the memory */

    entrySize /= 4;     /* Convert table entry from bytes to words */
    *numOfEntriesPtr = length / entrySize;

    /* Get memory base address */
    if (tableType == CPSS_DXCH_TABLE_VLAN_E)
    {
        /* devNum is not relevant for VLAN Table */
        status = getMemBaseFromType (devNum, CPSS_DIAG_PP_MEM_VLAN_TBL_MEM_E, &memBase);
    }
    else if (tableType == CPSS_DXCH_TABLE_FDB_E)
    {
        /* devNum is not relevant for FDB Table */
        status = getMemBaseFromType (devNum, CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E, &memBase);
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    if (status != GT_OK)
    {
        return status;
    }

    /* Calculate table entry index */
    *firstEntryIndexPtr = (addr - memBase) / (entrySize * 4);

    return GT_OK;
}

/**
* @internal getMacTableSize function
* @endinternal
*
* @brief   Gets the size of the MAC table.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
*
* @return MAC table size in bytes
*
* @note NONE
*
*/
static GT_U32 getMacTableSize
(
    IN  GT_U8   devNum
)
{
    GT_STATUS   rc;
    GT_U32                                      size;
    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC*    fineTuningPtr;
    GT_U32 entrySize;

    fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning;

    rc = getEntrySize(devNum,CPSS_DXCH_TABLE_FDB_E,&entrySize);
    if(rc != GT_OK)
    {
        entrySize = 0;
    }

    size = (fineTuningPtr->tableSize.fdb * entrySize);

    return size;
}

/**
* @internal getVlanTableSize function
* @endinternal
*
* @brief   Gets the size of the VLAN table.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
*
* @return VLAN table size in bytes
*/
static GT_U32 getVlanTableSize
(
    IN  GT_U8   devNum
)
{
    GT_STATUS   rc;
    GT_U32 size;
    GT_U32 entrySize;
    GT_U32 numEntries;
    CPSS_DXCH_TABLE_ENT tableType;       /* type of table */

    if (PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        /* Lion and Lion2 and above */
        numEntries = PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_TABLE_VLAN_MAC(devNum) + 1;
        tableType = CPSS_DXCH_LION_TABLE_VLAN_EGRESS_E;
    }
    else
    {
        /* xCat2,xCat and below */
        tableType = CPSS_DXCH_TABLE_VLAN_E;
        numEntries = _4KB;
    }

    rc = getEntrySize(devNum,tableType,&entrySize);
    if(rc != GT_OK)
    {
        entrySize = 0;
    }

    size = entrySize * numEntries;

    return size;
}

/**
* @internal getBanksParams function
* @endinternal
*
* @brief   Gets the banks start addresses and the banks sizes.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number to test
*
* @param[out] bank0StartPtr            - base address of bank 0
* @param[out] bank0SizePtr             - size in MB of bank 0
* @param[out] bank1StartPtr            - base address of bank 1
* @param[out] bank1SizePtr             - size in MB of bank 1
*/
static GT_VOID getBanksParams
(
    IN  GT_U8   devNum,
    OUT GT_U32 *bank0StartPtr,
    OUT GT_U32 *bank0SizePtr,
    OUT GT_U32 *bank1StartPtr,
    OUT GT_U32 *bank1SizePtr
)
{
    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC *fineTuningPtr;

    *bank0StartPtr = 0x06900000; /* Buffers memory bank 0 word 0 */

    if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E)  ||
       (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E))
    {
        fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning;

        *bank0StartPtr = 0x06800000;        /* Buffers memory bank 1 word 0 */
        *bank1StartPtr = 0x07000000;        /* Buffers memory bank 1 word 0 */
        /* Note: This is bits, not bytes */
        *bank0SizePtr = (fineTuningPtr->tableSize.bufferMemory) / 2;
        *bank1SizePtr = (fineTuningPtr->tableSize.bufferMemory) / 2;
    }
    else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
    {
        fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning;

        *bank0StartPtr = 0x0C000000;        /* Buffers memory bank 1 word 0 */
        *bank1StartPtr = 0x0D000000;        /* Buffers memory bank 1 word 0 */
        /* Note: This is bits, not bytes */
        *bank0SizePtr = (fineTuningPtr->tableSize.bufferMemory) / 2;
        *bank1SizePtr = (fineTuningPtr->tableSize.bufferMemory) / 2;
    }
    else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)
    {
        /* lion */

        fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning;

        /* bit 20 used as bank select */
        *bank0StartPtr = 0x10000000;
        *bank0SizePtr = fineTuningPtr->tableSize.bufferMemory;
        /* Not relevant for Lion */
        *bank1StartPtr = 0x00000000;
        *bank1SizePtr = 0;
    }
    else
    {
        /* device do not support CPU access to the buffer memory */
        *bank0StartPtr = 0x00000000;
        *bank0SizePtr = 0;
        *bank1StartPtr = 0x00000000;
        *bank1SizePtr = 0;
    }

    *bank0SizePtr /= 8;                    /* convert to bytes */
    *bank1SizePtr /= 8;                    /* convert to bytes */
}

/**
* @internal hwBuffMemBankGet function
* @endinternal
*
* @brief   Gets the bank and page for a given addr in the memory BUFFER RAM.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number to test
* @param[in] addr                     - The address to check.
* @param[in] length                   - The  from the addr in WORDS(32 bits).
*
* @param[out] bankPtr                  - (pointer to) the HW bank for the given address
* @param[out] pagePtr                  - (pointer to)the HW page for the given address
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - the address is not in the Cheetah buffer RAM memory space.
*
* @note Cheetah1 buffer RAM is divided into two spaces (banks).
*       each bank is 3Mbit == 384K byte == 96K words.
*       bank location:
*       bank0: 0x06900000...0x0695FFFC
*       bank1: 0x06980000...0x069DFFFC
*       For other devices see datasheet to get RAM size and addresses
*
*/
static GT_STATUS hwBuffMemBankGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  addr,
    IN  GT_U32  length,
    OUT GT_U32 *bankPtr,
    OUT GT_U32 *pagePtr
)
{
    GT_U32     bank0Start;
    GT_U32     bank0Size;
    GT_U32     bank1Start;
    GT_U32     bank1Size;
    GT_U32     pageSize;
    GT_U32     regAddr;

    getBanksParams (devNum, &bank0Start, &bank0Size, &bank1Start, &bank1Size);
    pageSize  = BUFFER_DRAM_PAGE_SIZE_CNS; /* each page is 64 bytes (16 words)*/

    /* bank identify */
    if(getMemBaseFromType(devNum, CPSS_DIAG_PP_MEM_BUFFER_DRAM_E, &regAddr))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    if(regAddr != bank0Start)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

    /* each bank is 3MB */
    if(addr >= bank0Start && (addr < (bank0Start+bank0Size)))
    {
        *bankPtr = 0;
        *pagePtr = (addr - bank0Start) / pageSize;
        /* check if all the length is in bank0 */
        if((addr + length) > (bank0Start + bank0Size))
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    else if(addr >= bank1Start && (addr < (bank1Start+bank1Size)))
    {
        *bankPtr = 1;
        *pagePtr = (addr - bank1Start) / pageSize;
        if((addr + length) > (bank1Start + bank1Size))
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    else
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

    return GT_OK;
}

/**
* @internal hwPpBufferRamInWordsRead function
* @endinternal
*
* @brief   Read the memory WORD by WORD.(32 bits).
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portGroupId              - the port group Id , to support multi-port-group devices
*                                      that need to access specific port group.
* @param[in] addr                     -  address to start writing from.
* @param[in] length                   -  in WORDS (32 bit) must be in portion of 8.
*
* @param[out] dataPtr                  - (pointer to) an array containing the data to be read.
*
* @note If the memory is 32 bits in each WORD (like Cheetah BUFFER_DRAM) each
*       address points to a 32 bits WORD and the memory segmentation is 1
*       instead of 4 in normal operation.
*       Using prvCpssDrvHwPpReadRam with length of more then 1 WORD would read
*       offset of 0,4,8... while WORD of 32 bits needs offset of 0,1,2,3...
*
*/
static GT_STATUS hwPpBufferRamInWordsRead
(
    IN  GT_U8  devNum,
    IN  GT_U32 portGroupId,
    IN  GT_U32 addr,
    IN  GT_U32 length,
    OUT GT_U32 *dataPtr
)
{
    GT_U32    readLoop;
    GT_STATUS ret = GT_OK;

    /* Check if the device is Cheetah3,
       then Buffer Memory Bank address differs from Buffer Memory Bank Write
       Register address (Bank0 = 0x06800000, Bank1 = 0x07000000) */
    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_CHEETAH3_E)
    {
        addr = addr - 0x100000;

    }

    for(readLoop=0; readLoop < length; ++readLoop)
    {
        /* read only one WORD at the time */
        ret = prvCpssDrvHwPpPortGroupReadRam(devNum, portGroupId, addr + 4 * readLoop, 1,
                                    &(dataPtr[readLoop]));
    }

    return ret;
}

/**
* @internal hwPpRamBuffMemPageWrite function
* @endinternal
*
* @brief   BUFFER_DRAM (buff mem) memory pages write.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] portGroupId              - the port group Id , to support multi-port-group devices
*                                      that need to access specific port group
* @param[in] addr                     - Address offset to write to.
* @param[in] length                   -  in WORDS (32 bit) must be in portion of 16.
* @param[in] dataPtr                  - (pointer to) data to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_TIMEOUT               - after max time that the PP not finished action
*
* @note Writting to this memory is not direct.
*       buffMemBank0Write is expecting to be written 16 times before triggering
*       for writting the buffered data into the memory.
*       Triggering is combined with the bank and page for HW writting.
*       The bank and page is pointing to the abs address segment.
*
*/
static GT_STATUS hwPpRamBuffMemPageWrite
(
    IN GT_U8  devNum,
    IN GT_U32 portGroupId,
    IN GT_U32 addr,
    IN GT_U32 length,
    IN GT_U32 *dataPtr
)
{
    GT_U32    regAddr;          /* register address */
    GT_U32    writeLoop;
    GT_U32    bank;
    GT_U32    page;
    GT_U32    lengthPos = 0;    /* The current writting location in WORDS */
    GT_U32    absAdr;           /* the current abs addr. to write to      */
    GT_U32    hwData;
    GT_STATUS ret = GT_OK;

    /* buffer dram must be written in portion of 16 */
    if ((length % (BUFFER_DRAM_PAGE_SIZE_CNS / 4)) != 0)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    if ((addr % (BUFFER_DRAM_PAGE_SIZE_CNS / 4)) != 0)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

    while(lengthPos < length)
    {
        absAdr = addr + lengthPos;

        /* find bank and page */
        ret = hwBuffMemBankGet (devNum, absAdr, BUFFER_DRAM_PAGE_SIZE_CNS / 4,
                                &bank, &page);
        if (ret != GT_OK)
        {
            return ret;
        }

        /* step 1: Make sure the previous write is done */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            addrSpace.buffMemWriteControl;

#ifndef ASIC_SIMULATION
        /* wait for bit 0 to clear */
        ret = prvCpssPortGroupBusyWait(devNum,portGroupId,regAddr,0,GT_FALSE);
        if(ret != GT_OK)
        {
            return ret;
        }
#endif /*!ASIC_SIMULATION*/
        /* step 2: Write the 16 words of the page data */
        if(bank == 0)
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            addrSpace.buffMemBank0Write;
        else
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            addrSpace.buffMemBank1Write;

        for (writeLoop = 0; writeLoop < BUFFER_DRAM_PAGE_SIZE_CNS / 4 ; ++writeLoop)
        {
            ret = prvCpssDrvHwPpPortGroupWriteRegister(devNum, portGroupId, regAddr,
                                              dataPtr[lengthPos+writeLoop]);
            if (ret != GT_OK)
            {
                return ret;
            }

        }

        /* step 3: Trigger the Write transaction */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            addrSpace.buffMemWriteControl;

        /* set hw data to write to access register */
        hwData = 1; /* WrTrig = 1 */

        /* check the device type */
        if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_CHEETAH3_E)
        {/* DxCh3 */

            hwData |= (page << 1); /* PageAddr (15 bits)*/
            hwData |= (bank << 16); /* WrBank */
        }
        else if ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) ||
                 (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E) ||
                 (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E))
        {/* xCat and xCat2 */

            hwData |= (page << 1); /* PageAddr (14 bits )*/
            hwData |= (bank << 15); /* WrBank */
        }
        else
        {/* Other DxCh devices */

            hwData |= (page << 1); /* PageAddr (13 bits )*/
            hwData |= (bank << 14); /* WrBank */
        }

        /* Write the data to Buffers Memory Write Access Register */
        ret = prvCpssDrvHwPpPortGroupWriteRegister(devNum, portGroupId, regAddr, hwData);
        if (ret != GT_OK)
        {
            return ret;
        }

        lengthPos += (BUFFER_DRAM_PAGE_SIZE_CNS / 4);
    }
    return ret;
}

/**
* @internal hwPpRamBuffMemWordWrite function
* @endinternal
*
* @brief   BUFFER_DRAM (buff mem) memory word write.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] portGroupId              - the port group Id , to support multi-port-group devices
*                                      that need to access specific port group
* @param[in] addr                     - Address offset to write to.
* @param[in] data                     - the  to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_TIMEOUT               - after max time that the PP not finished action
*
* @note Since writing to buffer memory is done in pages, the whole page is read
*       and stored to a temporary buffer. Then the data in the relevant word is
*       changed and the whole temporary buffer is written into the memory.
*
*/
static GT_STATUS hwPpRamBuffMemWordWrite
(
    IN GT_U8  devNum,
    IN GT_U32 portGroupId,
    IN GT_U32 addr,
    IN GT_U32 data
)
{
    GT_U32    tempData[BUFFER_DRAM_PAGE_SIZE_IN_WORDS_CNS]; /* temporary buffer */
    GT_U32    bank;                            /* bank number to write to */
    GT_U32    page;                            /* page number to write to */
    GT_U32    pageAddr;                        /* address of the page to read */
    GT_U32    bank0Start;                      /* start address of bank 0 */
    GT_U32    bank0Size;                       /* size of bank 0 - unused */
    GT_U32    bank1Start;                      /* start address of bank 1 */
    GT_U32    bank1Size;                       /* size of bank 1 - unused */
    GT_U32    wordNum;                         /* word number in a page */
    GT_STATUS status;                          /* returned status */

    /* Find bank and page */
    status = hwBuffMemBankGet (devNum, addr, BUFFER_DRAM_PAGE_SIZE_CNS / 4, &bank,
                               &page);
    if (status != GT_OK)
    {
        return status;
    }

    /* Find the start address of the page */
    getBanksParams (devNum, &bank0Start, &bank0Size, &bank1Start, &bank1Size);
    if (bank == 0)
    {
        pageAddr = bank0Start + (page * BUFFER_DRAM_PAGE_SIZE_CNS);
    }
    else /* bank 1 */
    {
        pageAddr = bank1Start + (page * BUFFER_DRAM_PAGE_SIZE_CNS);
    }


    /* Read the whole page */
    status = hwPpBufferRamInWordsRead (devNum, portGroupId, pageAddr,
                                       BUFFER_DRAM_PAGE_SIZE_IN_WORDS_CNS,
                                       tempData);
    if (status != GT_OK)
    {
        return status;
    }

    /* Change temporary buffer and write it to the buffer memory */
    wordNum = (addr - pageAddr) / 4;
    if (wordNum >= BUFFER_DRAM_PAGE_SIZE_IN_WORDS_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    tempData[wordNum] = data;

    return hwPpRamBuffMemPageWrite (devNum, portGroupId, pageAddr,
                                    BUFFER_DRAM_PAGE_SIZE_IN_WORDS_CNS,
                                    tempData);
}

/**
* @internal hwPpRamBuffMemLionPageWrite function
* @endinternal
*
* @brief   BUFFER_DRAM (buff mem) Lion memory pages write.
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] portGroupId              - the port group Id , to support multi-port-group devices
*                                      that need to access specific port group
* @param[in] addr                     - Address offset to write to.
* @param[in] length                   - length in WORDS (32 bit) must be in portion of 32.
* @param[in] dataPtr                  - (pointer to) data to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_TIMEOUT               - after max time that the PP not finished action
*
* @note Access to this memory is direct.
*       It is enough to write the first word from the data in the line, as all
*       words in the line are written with the same word.
*
*/
static GT_STATUS hwPpRamBuffMemLionPageWrite
(
    IN GT_U8  devNum,
    IN GT_U32 portGroupId,
    IN GT_U32 addr,
    IN GT_U32 length,
    IN GT_U32 *dataPtr
)
{
    GT_U32    regAddr;          /* register address                             */
    GT_U32    hwData;           /* hw data to write to each word of the line    */
    GT_U32    lineStart;        /* The first line to write                      */
    GT_U32    linesNum;         /* The number of lines to write                 */
    GT_U32    line;             /* Loop iterator                                */
    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC *fineTuningPtr;
    GT_STATUS ret = GT_OK;

    /* buffer dram must be written in portion of 32 */
    if (length % ((LION_BUFFER_DRAM_PAGE_SIZE_CNS / 4)) != 0)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    if ((addr % (LION_BUFFER_DRAM_PAGE_SIZE_CNS / 4)) != 0)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->addrSpace.buffMemBank0Write;
    fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning;

    /* calculate the first line to write */
    lineStart = ((addr - regAddr) / LION_BUFFER_DRAM_PAGE_SIZE_CNS);

    /* calculate the number of lines to write */
    linesNum = (length / (LION_BUFFER_DRAM_PAGE_SIZE_CNS / 4));

    /* check validity */
    if (((lineStart + linesNum) * LION_BUFFER_DRAM_PAGE_SIZE_CNS * 8) >
        fineTuningPtr->tableSize.bufferMemory)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* line bits in the address are 8..19 and bank is bit 20 */
    regAddr += (lineStart << 8);

    /* it is enough to write only one word to each line,
       so the first word will be written to each line  */
    for (line = lineStart; line < (linesNum + lineStart); line++)
    {
        hwData = dataPtr[((line - lineStart) * (LION_BUFFER_DRAM_PAGE_SIZE_CNS / 4))];

        ret = prvCpssDrvHwPpPortGroupWriteRegister(devNum, portGroupId,
                                                   regAddr, hwData);
        if (ret != GT_OK)
        {
            return ret;
        }

        /* increment line in the address */
        regAddr += 0x100;
    }

    return GT_OK;
}

/**
* @internal hwPpBufferLionRamInWordsRead function
* @endinternal
*
* @brief   Read the memory WORD by WORD.(32 bits).
*
* @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - device number.
* @param[in] portGroupId              - the port group Id , to support multi-port-group devices
*                                      that need to access specific port group.
* @param[in] addr                     -  address to start reading from.
* @param[in] length                   - length in WORDS (32 bit) must be in portion of 32.
*
* @param[out] dataPtr                  - (pointer to) an array containing the data to be read.
*
* @note Access to this memory is direct.
*       It is enough to read the first word from the data in the line, as all
*       words in the line are written with the same word.
*
*/
static GT_STATUS hwPpBufferLionRamInWordsRead
(
    IN  GT_U8  devNum,
    IN  GT_U32 portGroupId,
    IN  GT_U32 addr,
    IN  GT_U32 length,
    OUT GT_U32 *dataPtr
)
{
    GT_U32    regAddr;          /* register address                             */
    GT_U32    hwData;           /* hw data to write to each word of the line    */
    GT_U32    lineStart;        /* The first line to write                      */
    GT_U32    linesNum;         /* The number of lines to write                 */
    GT_U32    wordsNumInLine;   /* The number of words in each line             */
    GT_U32    wordInd;          /* The index of the word in dataPtr             */
    GT_U32    line;             /* Loop iterator                                */
    GT_U32    i;                /* Loop iterator                                */
    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC *fineTuningPtr;
    GT_STATUS ret = GT_OK;

    /* buffer dram must be written in portion of 32 */
    if (length % ((LION_BUFFER_DRAM_PAGE_SIZE_CNS / 4)) != 0)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    if ((addr % (LION_BUFFER_DRAM_PAGE_SIZE_CNS / 4)) != 0)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->addrSpace.buffMemBank0Write;
    fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning;

    /* calculate the first line to write */
    lineStart = ((addr - regAddr) / LION_BUFFER_DRAM_PAGE_SIZE_CNS);

    /* calculate the number of lines to write */
    linesNum = (length / (LION_BUFFER_DRAM_PAGE_SIZE_CNS / 4));

    /* check validity */
    if (((lineStart + linesNum) * LION_BUFFER_DRAM_PAGE_SIZE_CNS * 8) >
        fineTuningPtr->tableSize.bufferMemory)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    wordsNumInLine = (LION_BUFFER_DRAM_PAGE_SIZE_CNS / 4);

    /* line bits in the address are 8..19 and bank is bit 20 */
    regAddr += (lineStart << 8);

    wordInd = 0;

    /* Read all words from the line and compare them to the first one,
       all words should be equal */
    for (line = lineStart; line < (linesNum + lineStart); line++)
    {
        for (i = 0; i < wordsNumInLine; i++)
        {
            ret = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId,
                                                       regAddr, &hwData);
            if (ret != GT_OK)
            {
                return ret;
            }

            dataPtr[wordInd] = hwData;

            /* check that current and previous words are equal */
            /* compare current word with the previous one */
            if (i > 0)
            {
                if (dataPtr[wordInd] != dataPtr[(wordInd - 1)])
                {
                    /* Change the first word in the line to the wrong one */
                    dataPtr[((line - lineStart) * wordsNumInLine)] = dataPtr[wordInd];
                }
            }

            wordInd++;

            /* increment word in the address, bits 2..6 */
            regAddr += 0x4;
        }

        /* increment line in the address */
        regAddr += 0x100;
    }

    return GT_OK;
}

/**
* @internal diagTestBuffBanksMem function
* @endinternal
*
* @brief   This routine performs a memory test on memory banks which do not have to
*         be contiguous.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The local device number to test
* @param[in] memType                  - The packet processor memory type to verify.
* @param[in] startOffset              - The offset address to start the test from.
* @param[in] size                     - The memory size in bytes to test (start from offset).
* @param[in] profile                  - The test profile.
* @param[in] burstSize                - The memory burst size.
*
* @param[out] testStatusPtr            - GT_TRUE if the test succeeded or GT_FALSE for failure
* @param[out] addrPtr                  - Address offset of memory error, if testStatusPtr is
*                                      GT_FALSE. Irrelevant if testStatusPtr is GT_TRUE.
* @param[out] readValPtr               - Contains the value read from the register which caused
*                                      the failure if testStatusPtr is GT_FALSE. Irrelevant if
*                                      testStatusPtr is GT_TRUE.
* @param[out] writeValPtr              - Contains the value written to the register which caused
*                                      the failure if testStatusPtr is GT_FALSE. Irrelevant if
*                                      testStatusPtr is GT_TRUE.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note This routine supports only buffer DRAM memory type.
*
*/
static GT_STATUS diagTestBuffBanksMem
(
    IN GT_U8                        devNum,
    IN CPSS_DIAG_PP_MEM_TYPE_ENT    memType,
    IN GT_U32                       startOffset,
    IN GT_U32                       size,
    IN CPSS_DIAG_TEST_PROFILE_ENT   profile,
    IN GT_U32                       burstSize,
    OUT GT_BOOL                     *testStatusPtr,
    OUT GT_U32                      *addrPtr,
    OUT GT_U32                      *readValPtr,
    OUT GT_U32                      *writeValPtr
)
{
    GT_U32      savedBurstSize;
    GT_U32      bank0Start;
    GT_U32      bank0Size;
    GT_U32      bank1Start;
    GT_U32      bank1Size;
    GT_U32      regAddr;
    GT_U32      ageAutoEnable;
    PRV_CPSS_DIAG_RAM_SPECIAL_TREATMENT_FUNC_STC specialRamFuncs;
    GT_U32      portGroupsBmp;
    GT_STATUS   ret = GT_OK;
    GT_STATUS   status = GT_OK;

    cpssOsMemSet (&specialRamFuncs, 0,
                  sizeof(PRV_CPSS_DIAG_RAM_SPECIAL_TREATMENT_FUNC_STC));

    /* this routine supports only buffer DRAM memory test */
    if (memType != CPSS_DIAG_PP_MEM_BUFFER_DRAM_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* in Lion the access is direct, in other devices - indirect */
    if (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_LION_E)
    {
        specialRamFuncs.prvCpssDiagHwPpRamBufMemWriteFuncPtr =
            &hwPpRamBuffMemPageWrite;
        specialRamFuncs.prvCpssDiagHwPpRamInWordsReadFuncPtr =
            &hwPpBufferRamInWordsRead;
    }
    else
    {
        specialRamFuncs.prvCpssDiagHwPpRamBufMemWriteFuncPtr =
            &hwPpRamBuffMemLionPageWrite;
        specialRamFuncs.prvCpssDiagHwPpRamInWordsReadFuncPtr =
            &hwPpBufferLionRamInWordsRead;
    }

    getBanksParams (devNum, &bank0Start, &bank0Size, &bank1Start, &bank1Size);

    /* validate the mem test size does not overflow the total banks size */
    if (size > (bank0Size + bank1Size))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* adjust the banks start address and size according to test config */
    if (startOffset < bank0Size)
    {
        /* the start address is within the first bank */
        bank0Start += startOffset;
        bank0Size  -= startOffset;

        if (bank0Size >= size)
        {
            /* the test is within the first bank only */
            bank0Size = size;
            bank1Size = 0;
        }
        else
        {
            /* the test goes on afterwards to bank1, set the bank size */
            bank1Size = size - bank0Size;
        }
    }
    else
    {
        /* the start address is within the second bank */
        bank1Start += (startOffset - bank0Size);
        bank1Size   = size;
        bank0Size = 0;
    }

    /* save and set the new burst size */
    savedBurstSize = prvCpssDiagBurstSizeGet();
    ret = prvCpssDiagBurstSizeSet(burstSize);
    if (ret != GT_OK)
    {
        return ret;
    }

    /* Buffer Management Aging Configuration Register */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)-> bufferMng.bufMngAgingConfig;
    ret = prvCpssDrvHwPpGetRegField (devNum, regAddr, 10, 1, &ageAutoEnable);
    if (ret != GT_OK)
    {
        PRV_CH_DIAG_CHECK_RC_MAC(prvCpssDiagBurstSizeSet(savedBurstSize));
        return ret;
    }


    /* test the first bank */
    if (bank0Size > 0)
    {
        /* Clear ageAutoEnable bit in Buffer Management Aging Configuration
           Register */
        if (ageAutoEnable == 1)
        {
            ret = prvCpssDrvHwPpSetRegField (devNum, regAddr, 10, 1, 0);
            if (ret != GT_OK)
            {
                PRV_CH_DIAG_CHECK_RC_MAC(prvCpssDiagBurstSizeSet(savedBurstSize));
                return ret;
            }
        }

        /* Get valid portGroup bitmap */
        if(GT_FALSE ==
            prvCpssDuplicatedMultiPortGroupsGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
             bank0Start,&portGroupsBmp,NULL,NULL))
        {
            portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
        }

        ret = prvCpssDiagAnyMemTest(devNum, portGroupsBmp, bank0Start, bank0Size, profile,
                                    specialRamFuncs, testStatusPtr, addrPtr,
                                    readValPtr, writeValPtr);

        /* Restore ageAutoEnable bit in Buffer Management Aging Configuration
           Register to its previous value */
        if (ageAutoEnable == 1)
        {
            status = prvCpssDrvHwPpSetRegField (devNum, regAddr, 10, 1, ageAutoEnable);
            if (status != GT_OK)
            {
                PRV_CH_DIAG_CHECK_RC_MAC(prvCpssDiagBurstSizeSet(savedBurstSize));
                return ((ret != GT_OK) ? ret : status);
            }
        }
        if (ret != GT_OK)
        {
            PRV_CH_DIAG_CHECK_RC_MAC(prvCpssDiagBurstSizeSet(savedBurstSize));
            return ret;
        }

        if (*testStatusPtr == GT_FALSE)
        {
            ret = prvCpssDiagBurstSizeSet(savedBurstSize);
            if (ret != GT_OK)
            {
                return ret;
            }
            return GT_OK;
        }
    }

    /* test the second bank */
    if (bank1Size > 0)
    {
        /* Clear ageAutoEnable bit in Buffer Management Aging Configuration
           Register */
        if (ageAutoEnable == 1)
        {
            ret = prvCpssDrvHwPpSetRegField (devNum, regAddr, 10, 1, 0);
            if (ret != GT_OK)
            {
                PRV_CH_DIAG_CHECK_RC_MAC(prvCpssDiagBurstSizeSet(savedBurstSize));
                return ret;
            }

        }

        /* Get valid portGroup bitmap */
        if(GT_FALSE ==
            prvCpssDuplicatedMultiPortGroupsGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
             bank0Start,&portGroupsBmp,NULL,NULL))
        {
            portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
        }

        ret = prvCpssDiagAnyMemTest(devNum, portGroupsBmp, bank1Start, bank1Size, profile,
                                    specialRamFuncs, testStatusPtr, addrPtr,
                                    readValPtr, writeValPtr);

        /* Restore ageAutoEnable bit in Buffer Management Aging Configuration
           Register to its previous value */
        if (ageAutoEnable == 1)
        {
            status = prvCpssDrvHwPpSetRegField (devNum, regAddr, 10, 1, ageAutoEnable);
            if (status != GT_OK)
            {
                PRV_CH_DIAG_CHECK_RC_MAC(prvCpssDiagBurstSizeSet(savedBurstSize));
                return ((ret != GT_OK) ? ret : status);
            }
        }
    }

    /* restore the original burst size */
    if (ret != GT_OK)
    {
        PRV_CH_DIAG_CHECK_RC_MAC(prvCpssDiagBurstSizeSet(savedBurstSize));
        return ret;
    }
    else
    {
        return prvCpssDiagBurstSizeSet(savedBurstSize);
    }
}

/**
* @internal hwPpTableEntryWrite function
* @endinternal
*
* @brief   Indirect write to PP's table.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP device number to write to
* @param[in] portGroupId              - the port group Id , to support multi-port-group devices
*                                      that need to access specific port group
* @param[in] addr                     - Address offset to write to
* @param[in] length                   - Number of Words (4 bytes) to write
* @param[in] dataPtr                  - An array containing the data to be written
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
*/
static GT_STATUS hwPpTableEntryWrite
(
    IN GT_U8 devNum,
    IN GT_U32 portGroupId,
    IN GT_U32 addr,
    IN GT_U32 length,
    IN GT_U32 *dataPtr
)
{
    CPSS_DXCH_TABLE_ENT tableType;       /* type of table */
    GT_U32                  firstEntryIndex; /* index of first entry to write */
    GT_U32                  numOfEntries;    /* number of entries to write */
    GT_U32                  entrySize;       /* entry size in bytes */
    GT_U32                  i;               /* loop index */
    GT_STATUS               status;          /* returned status */

    /* Find table type by memory address */
    status = getTableType (devNum,addr, &tableType);
    if (status != GT_OK)
    {
        return status;
    }

    /* Get entry size */
    status = getEntrySize (devNum, tableType, &entrySize);
    if (status != GT_OK)
    {
        return status;
    }

    /* Get the first entry to write to and the number of entries to write */
    status = getEntryIndexAndNumOfEntries (devNum, addr, tableType,
                                           entrySize, length,
                                           &firstEntryIndex, &numOfEntries);
    if (status != GT_OK)
    {
        return status;
    }

    /* write to all the entries */
    for (i = 0; i < numOfEntries; i++)
    {
        status = prvCpssDxChPortGroupWriteTableEntry (devNum, portGroupId, tableType,
                                             firstEntryIndex + i, dataPtr);
        if (status != GT_OK)
        {
            return status;
        }

        dataPtr += entrySize / 4;

    }
    return GT_OK;
}

/**
* @internal hwPpTableEntryRead function
* @endinternal
*
* @brief   Indirect read from PP's table.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP device number to read from
* @param[in] portGroupId              - the port group Id , to support multi-port-group devices
*                                      that need to access specific port group
* @param[in] addr                     - Address offset to read from
* @param[in] length                   - Number of Words (4 bytes) to read
*
* @param[out] dataPtr                  - An array containing the read data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
static GT_STATUS hwPpTableEntryRead
(
    IN GT_U8 devNum,
    IN GT_U32 portGroupId,
    IN GT_U32 addr,
    IN GT_U32 length,
    OUT GT_U32 *dataPtr
)
{
    CPSS_DXCH_TABLE_ENT tableType;       /* type of table */
    GT_U32                  firstEntryIndex; /* index of first entry to read */
    GT_U32                  numOfEntries;    /* number of entries to read */
    GT_U32                  entrySize;       /* entry size in bytes */
    GT_U32                  i;               /* loop index */
    GT_STATUS               status;          /* returned status */

    /* Find table type by memory address */
    status = getTableType (devNum,addr, &tableType);
    if (status != GT_OK)
    {
        return status;
    }

    /* Get entry size */
    status = getEntrySize (devNum, tableType, &entrySize);
    if (status != GT_OK)
    {
        return status;
    }

    /* Get the first entry to read from and the number of entries to read */
    status = getEntryIndexAndNumOfEntries (devNum, addr, tableType,
                                           entrySize, length,
                                           &firstEntryIndex, &numOfEntries);
    if (status != GT_OK)
    {
        return status;
    }

    /* read from all the entries */
    for (i = 0; i < numOfEntries; i++)
    {
        status = prvCpssDxChPortGroupReadTableEntry (devNum, portGroupId, tableType,
                                            firstEntryIndex + i, dataPtr);
        if (status != GT_OK)
        {
            return status;
        }

        dataPtr += entrySize / 4;
    }
    return GT_OK;
}


/**
* @internal hwPpTableEntryFieldWrite function
* @endinternal
*
* @brief   Indirect write to PP's table field.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP device number to write to
* @param[in] portGroupId              - the port group Id , to support multi-port-group devices
*                                      that need to access specific port group
* @param[in] addr                     - Address offset to write to
* @param[in] data                     - Data to be written
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
*/
static GT_STATUS hwPpTableEntryFieldWrite
(
    IN GT_U8 devNum,
    IN GT_U32 portGroupId,
    IN GT_U32 addr,
    IN GT_U32 data
)
{
    CPSS_DXCH_TABLE_ENT tableType;    /* table type */
    GT_STATUS               status;       /* returned status */
    GT_U32                  entryIndex;   /* index of entry to write */
    GT_U32                  wordNum;      /* word number inside the entry */

    /* Find table type by memory address */
    status = getTableType (devNum,addr, &tableType);
    if (status != GT_OK)
    {
        return status;
    }

    /* Retrieve the entry index and the word number */
    status = getEntryIndexAndWordNum (devNum, tableType, addr, &entryIndex,
                                      &wordNum);
    if (status != GT_OK)
    {
        return status;
    }

    /* Write field */
    return prvCpssDxChPortGroupWriteTableEntryField(devNum, portGroupId,
                                                    tableType, entryIndex,
                                                    wordNum, 0, 32, data);
}

/**
* @internal hwPpTableEntryFieldRead function
* @endinternal
*
* @brief   Indirect read from PP's table.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP device number to read from
* @param[in] portGroupId              - the port group Id , to support multi-port-group devices
*                                      that need to access specific port group
* @param[in] addr                     - Address offset to read from
*
* @param[out] dataPtr                  - (pointer to) the read data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
static GT_STATUS hwPpTableEntryFieldRead
(
    IN GT_U8 devNum,
    IN GT_U32 portGroupId,
    IN GT_U32 addr,
    OUT GT_U32 *dataPtr
)
{
    CPSS_DXCH_TABLE_ENT tableType;    /* table type */
    GT_STATUS               status;       /* returned status */
    GT_U32                  entryIndex;   /* index of entry to write */
    GT_U32                  wordNum;      /* word number inside the entry */

    /* Find table type by memory address */
    status = getTableType (devNum,addr, &tableType);
    if (status != GT_OK)
    {
        return status;
    }

    /* Retrieve the entry index and the word number */
    status = getEntryIndexAndWordNum (devNum, tableType, addr, &entryIndex,
                                      &wordNum);
    if (status != GT_OK)
    {
        return status;
    }

    /* Read field */
    return prvCpssDxChPortGroupReadTableEntryField (devNum, portGroupId,
                                                    tableType, entryIndex,
                                                    wordNum, 0, 32, dataPtr);
}

#define PRV_CPSS_DXCH_BOBCAT3_PIPE1_GOP_OFFSET_CNS                0x00400000
/*******************************************************************************
* cgPortAddrValid
*
* Description:
*      Check CG MAC register validity in particular port group.
*
* INPUTS:
*       devNum          - device number
*       portGroupId     - the port group Id , to support multi-port-group
*                           devices that need to access specific port group
*       regAddr         - Register address
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_TRUE    - register exists in specific port group
*       GT_FALSE   - register doesn't exist in specific port group
*
* Comment:
*       None.
*
 *******************************************************************************/
static GT_BOOL cgPortAddrValid
(
    IN GT_U8  devNum,
    IN GT_U32 portGroupId,
    IN GT_U32 regAddr
)
{
    GT_U32 portMacNum;
    GT_U32 port0_cg_regAddr;
    GT_U32 port_cg_regAddr_mask;
    CPSS_PORT_INTERFACE_MODE_ENT portInterfaceMode;
    PRV_CPSS_PORT_TYPE_ENT portMacType;
    GT_U32  pipe1PortOffset = 0;
    PRV_CPSS_DXCH_UNIT_ENT  unitId;

    portGroupId = portGroupId;

    unitId = prvCpssDxChHwRegAddrToUnitIdConvert(devNum,regAddr);
    if(unitId != PRV_CPSS_DXCH_UNIT_GOP_E)
    {
        /* the address not belong to GOP memory space */
        return GT_TRUE;
    }

    /* get the register addr of CG MAC of port 0 */
    portMacNum = 0;
    port0_cg_regAddr     = 0x00340000;
    port_cg_regAddr_mask = 0x003C0000;/*pipe 0 : 0x10340000 / 0x10350000 / 0x10360000  */
                                      /*pipe 1 : 0x10740000 / 0x10750000 / 0x10760000  */
    if((port0_cg_regAddr & port_cg_regAddr_mask) != (regAddr & port_cg_regAddr_mask))
    {
        /* the address not belong to CG MAC */
        return GT_TRUE;
    }

    if(PRV_CPSS_DXCH_BOBCAT3_PIPE1_GOP_OFFSET_CNS & regAddr)
    {
        /* address in PIPE 1 */
        pipe1PortOffset = 36;/* bobcat3*/
    }

    /* the address is in CG MAC */
    portMacNum = (regAddr / 0x1000) & 0x3f;
    portMacNum += pipe1PortOffset;

    portMacType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);
    portInterfaceMode = PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum,portMacNum);
    if(portMacType       != PRV_CPSS_PORT_CG_E ||
       (portInterfaceMode != CPSS_PORT_INTERFACE_MODE_KR4_E &&
        portInterfaceMode != CPSS_PORT_INTERFACE_MODE_SR_LR4_E))
    {
        /* the CG MAC is NOT in proper speed or mode */
        /* access to it's registers may HANG the PEX */

        return GT_FALSE;
    }

    /* the CG MAC is in proper speed and mode */
    return GT_TRUE;
}

/*******************************************************************************
* checkIfRegisterExist
*
* Description:
*      Check register existance in particular port group.
*
* INPUTS:
*       devNum          - device number
*       portGroupId     - the port group Id , to support multi-port-group
*                           devices that need to access specific port group
*       regAddr         - Register address
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_TRUE    - register exists in specific port group
*       GT_FALSE   - register doesn't exist in specific port group
*
* Comment:
*       None.
*
 *******************************************************************************/
static GT_BOOL checkIfRegisterExist
(
    IN GT_U8  devNum,
    IN GT_U32 portGroupId,
    IN GT_U32 regAddr
)
{
    GT_PORT_GROUPS_BMP  portGroupsBmp;  /*port groups bmp */
    GT_BOOL             existFlag = GT_FALSE;
    GT_STATUS   rc; /* return status */
    GT_U32      unitId; /* register unit ID */
    GT_U32      serdesUnitId; /* SERDES unit ID */
    GT_U32      serdesNum; /* SERDES number */

    if (PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        /* filter out INTERLAKEN registers */
        if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_ilkn.supported &&
           (regAddr >= PRV_DXCH_REG1_UNIT_ILKN_MAC(devNum).ILKN_MAC_CFG_0) &&
           (regAddr <= PRV_DXCH_REG1_UNIT_ILKN_MAC(devNum).ILKN_OVF_7))
        {
            return GT_FALSE;
        }

        if(GT_FALSE == cgPortAddrValid(devNum,portGroupId,regAddr))
        {
            /* filter out CG mac registers that not supported */
            return GT_FALSE;
        }
    }

    if (portGroupId == CPSS_PORT_GROUP_UNAWARE_MODE_CNS)
    {
        /* this is single port group device.
           it's need not to make extra checks. */
        return GT_TRUE;
    }

    if(GT_FALSE ==
        prvCpssDuplicatedMultiPortGroupsGet(devNum,portGroupId,
             regAddr,&portGroupsBmp,NULL,NULL))
    {
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    }

    if ((1 << portGroupId) & portGroupsBmp)
    {
        existFlag = GT_TRUE;
    }

    if ((existFlag == GT_TRUE) &&
        (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E))
    {
        /* filter out not initialized SERDES registers */
        unitId = (regAddr >> 23) & 0x3F;
        serdesUnitId = (PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[0].serdesExternalReg1 >> 23) & 0x3F;
        if (unitId == serdesUnitId)
        {
            /* check that register is in COMPHY unit
             0x09800200 + t*0x400: where t (0-23) represents SERDES */
            if (0x200 & regAddr)
            {
                /* SERDES number calculation */
                serdesNum = (regAddr & 0x7FFFFF) / 0x400;

                if (serdesNum >= PRV_CPSS_LION2_SERDES_NUM_CNS)
                {
                    return GT_FALSE;
                }

                existFlag = GT_FALSE;

                /* get SERDES init status */
                rc = prvCpssDxChLpCheckSerdesInitStatus(devNum, portGroupId, serdesNum);
                if (rc == GT_OK)
                {
                    /* SERDES is ready */
                    existFlag = GT_TRUE;
                }
            }
        }
    }

    return existFlag;
}

/**
* @internal internal_cpssDxChDiagMemTest function
* @endinternal
*
* @brief   Performs memory test on a specified memory location and size for a
*         specified memory type.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number to test
* @param[in] memType                  - The packet processor memory type to verify.
* @param[in] startOffset              - The offset address to start the test from.
* @param[in] size                     - The memory size in byte to test (start from offset).
* @param[in] profile                  - The test profile.
*
* @param[out] testStatusPtr            - (pointer to) test status. GT_TRUE if the test succeeded or GT_FALSE for failure
* @param[out] addrPtr                  - (pointer to) address offset of memory error, if testStatusPtr is
*                                      GT_FALSE. Irrelevant if testStatusPtr is GT_TRUE.
* @param[out] readValPtr               - (pointer to) value read from the register which caused
*                                      the failure if testStatusPtr is GT_FALSE. Irrelevant if
*                                      testStatusPtr is GT_TRUE.
* @param[out] writeValPtr              - (pointer to) value written to the register which caused
*                                      the failure if testStatusPtr is GT_FALSE. Irrelevant if
*                                      testStatusPtr is GT_TRUE.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong memory type
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The test is done by writing and reading a test pattern.
*       The function may be called after Phase 2 initialization.
*       The test is destructive and leaves the memory corrupted.
*       Supported memories:
*       - Buffer DRAM
*       - MAC table memory
*       - VLAN table memory
*       For buffer DRAM:
*       startOffset must be aligned to 64 Bytes and size must be in 64 bytes
*       resolution.
*       For MAC table:
*       startOffset must be aligned to 16 Bytes and size must be in 16 bytes
*       resolution.
*       For VLAN table:
*       DX CH devices: startOffset must be aligned to 12 Bytes and size must
*       be in 12 bytes resolution.
*       DX CH2 and above devices: startOffset must be aligned to 16 Bytes
*       and size must be in 16 bytes resolution.
*/
static GT_STATUS internal_cpssDxChDiagMemTest
(
    IN GT_U8                          devNum,
    IN CPSS_DIAG_PP_MEM_TYPE_ENT      memType,
    IN GT_U32                         startOffset,
    IN GT_U32                         size,
    IN CPSS_DIAG_TEST_PROFILE_ENT     profile,
    OUT GT_BOOL                       *testStatusPtr,
    OUT GT_U32                        *addrPtr,
    OUT GT_U32                        *readValPtr,
    OUT GT_U32                        *writeValPtr
)
{
    GT_U32    memBase;                                /* RAM base Address */
    GT_U32    testBase;                               /* test base address */
    GT_STATUS ret;                                    /* returned value */
    GT_U32    savedBurstSize;                         /* system burst size */
    GT_U32    maskArray[LION_BUFFER_DRAM_PAGE_SIZE_CNS / 4];   /* mask array */
    GT_U32    maskArraySizeInWords;                   /* the size of mask array in words */
    CPSS_MAC_ACTION_MODE_ENT    macActionMode = CPSS_ACT_AUTO_E;                      /* MAC action mode */
    GT_U32    i;                                      /* loop index */
    PRV_CPSS_DIAG_RAM_SPECIAL_TREATMENT_FUNC_STC specialRamFuncs;
    GT_U32    vlanEntrySize;                          /* Vlan entry size in bytes */
    GT_U32    portGroupsBmp;                          /* Bitmap of valid port groups */
    GT_U32    macEntrySize = 0;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(testStatusPtr);
    CPSS_NULL_PTR_CHECK_MAC(addrPtr);
    CPSS_NULL_PTR_CHECK_MAC(readValPtr);
    CPSS_NULL_PTR_CHECK_MAC(writeValPtr);

    /* check input parameters */
    if (memType == CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E)
    {
        ret = getEntrySize(devNum,CPSS_DXCH_TABLE_FDB_E,&macEntrySize);
        if(ret != GT_OK)
        {
            macEntrySize = 0;
        }

        if (((startOffset % macEntrySize) != 0) ||
            ((size % macEntrySize) != 0))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        if ((startOffset + size) > getMacTableSize(devNum))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else if (memType == CPSS_DIAG_PP_MEM_VLAN_TBL_MEM_E)
    {
        if ((startOffset + size) > getVlanTableSize(devNum))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else if (memType == CPSS_DIAG_PP_MEM_BUFFER_DRAM_E)
    {
        if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E ||
            PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            /* the device does not support CPU access to buffer memory. */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        if (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_LION_E)
        {
            /* xCat2, xCat, xCat3, DxCh3, DxCh2 and DxCh1 */
            if (((startOffset % BUFFER_DRAM_PAGE_SIZE_CNS) != 0) ||
                ((size % BUFFER_DRAM_PAGE_SIZE_CNS) != 0))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }
        else
        {
            /* Lion */
            if (((startOffset % LION_BUFFER_DRAM_PAGE_SIZE_CNS) != 0) ||
                ((size % LION_BUFFER_DRAM_PAGE_SIZE_CNS) != 0))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    cpssOsMemSet (&specialRamFuncs, 0,
                  sizeof(PRV_CPSS_DIAG_RAM_SPECIAL_TREATMENT_FUNC_STC));


    if (memType == CPSS_DIAG_PP_MEM_BUFFER_DRAM_E)
    {
        if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)
        {
            /* Lion */
            maskArraySizeInWords = (LION_BUFFER_DRAM_PAGE_SIZE_CNS / 4);

            /* unmask the lsb word */
            maskArray[0] = 0xffffffff;

            /* The word is written to all words in the line - mask all words,
               staring from second word */
            for (i = 1; i < maskArraySizeInWords; i++)
            {
                maskArray[i] = 0x0;
            }
        }
        else /* xCat2, xCat, xCat3, DxCh3, DxCh2 and DxCh1 */
        {
            maskArraySizeInWords = (BUFFER_DRAM_PAGE_SIZE_CNS / 4);

            /* All the bits are read/write - unmask all of them */
            for (i = 0; i < maskArraySizeInWords; i++)
            {
                maskArray[i] = 0xffffffff;
            }
        }

        ret = prvCpssDiagMemMaskArraySet (maskArraySizeInWords, maskArray);
        if (ret != GT_OK)
        {
            return ret;
        }
        return diagTestBuffBanksMem (devNum, memType, startOffset, size,
                                     profile, (maskArraySizeInWords * 4),
                                     testStatusPtr, addrPtr, readValPtr,
                                     writeValPtr);
    }


    /* if we reached this place then memory is either MAC table or VLAN table */
    if ((memType == CPSS_DIAG_PP_MEM_VLAN_TBL_MEM_E) &&
        (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum)))
    {
        /* xCat and above, direct access to the tables, do nothing */
    }
    else
    {
        /* indirect access to the tables */
        specialRamFuncs.prvCpssDiagHwPpMemoryReadFuncPtr = &hwPpTableEntryRead;
        specialRamFuncs.prvCpssDiagHwPpMemoryWriteFuncPtr = &hwPpTableEntryWrite;
    }

    /*Prepare Address Space Partitioning setings*/
    ret = getMemBaseFromType(devNum, memType, &memBase);
    if (ret != GT_OK)
    {
        return ret;
    }

    /*Save default burst size*/
    savedBurstSize = prvCpssDiagBurstSizeGet();

    testBase = memBase + startOffset;

    if (memType == CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E)
    {
        ret = prvCpssDiagBurstSizeSet (macEntrySize);
        if (ret != GT_OK)
        {
            return ret;
        }

        /* set array mask */
        maskArray[0] = 0xffffffff;
        maskArray[1] = 0xffffffff;
        maskArray[2] = 0xffffffff;
        maskArray[3] = 0x3ff;

        if(macEntrySize > 16)
        {
            maskArray[3] = 0xffffffff;
            maskArray[4] = (BIT_10 - 1);/*128..137*/
        }

        ret = prvCpssDiagMemMaskArraySet (macEntrySize/4, maskArray);
        if (ret != GT_OK)
        {
            return ret;
        }

        ret = cpssDxChBrgFdbMacTriggerModeGet(devNum,&macActionMode);
        if (ret != GT_OK)
        {
            return ret;
        }

        /* Set MAC action mode to triggered aging */
        if (macActionMode == CPSS_ACT_AUTO_E)
        {
            ret = cpssDxChBrgFdbMacTriggerModeSet(devNum,CPSS_ACT_TRIG_E);
            if (ret != GT_OK)
            {
                return ret;
            }
        }

    }
    else if (memType == CPSS_DIAG_PP_MEM_VLAN_TBL_MEM_E)
    {
        if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
        {
            ret = getEntrySize(devNum,CPSS_DXCH_LION_TABLE_VLAN_EGRESS_E,&vlanEntrySize);
            if(ret != GT_OK)
            {
                return ret;
            }

            /* set array mask */
            maskArray[0] = 0xffffffff;
            maskArray[1] = 0xffffffff;
            maskArray[2] = 0xffffffff;
            maskArray[3] = 0xffffffff;
            maskArray[4] = 0xffffffff;
            maskArray[5] = 0xffffffff;
            maskArray[6] = 0xffffffff;
            maskArray[7] = 0xffffffff;
            maskArray[8] = 0x1ff;

            for (i = 9; i < (vlanEntrySize / 4); i++)
            {
                maskArray[i] = 0x0;
            }
        }
        else
        {
            switch (PRV_CPSS_PP_MAC(devNum)->devFamily)
            {
                case CPSS_PP_FAMILY_DXCH_XCAT_E:
                case CPSS_PP_FAMILY_DXCH_XCAT3_E:
                case CPSS_PP_FAMILY_DXCH_XCAT2_E:
                    /* xCat, xCat3 and xCat2 */
                    vlanEntrySize = XCAT_VLAN_ENTRY_SIZE_CNS;

                    /* set array mask */
                    maskArray[0] = 0xffffffff;
                    maskArray[1] = 0xffffffff;
                    maskArray[2] = 0xffffffff;
                    maskArray[3] = 0xffffffff;
                    maskArray[4] = 0xffffffff;
                    maskArray[5] = 0x7ffff;
                    maskArray[6] = 0x0;
                    maskArray[7] = 0x0;
                    break;

                case CPSS_PP_FAMILY_DXCH_LION_E:
                case CPSS_PP_FAMILY_DXCH_LION2_E:
                    /* Lion */
                    ret = getEntrySize(devNum,CPSS_DXCH_LION_TABLE_VLAN_EGRESS_E,&vlanEntrySize);
                    if(ret != GT_OK)
                    {
                        return ret;
                    }

                    /* set array mask */
                    maskArray[0] = 0xffffffff;
                    maskArray[1] = 0xffffffff;
                    maskArray[2] = 0xffffffff;
                    maskArray[3] = 0xffffffff;
                    maskArray[4] = 0xffffffff;
                    maskArray[5] = 0xffffffff;
                    maskArray[6] = 0xffffffff;
                    maskArray[7] = 0xffffffff;
                    maskArray[8] = 0x1ff;

                    for (i = 9; i < (vlanEntrySize / 4); i++)
                    {
                        maskArray[i] = 0x0;
                    }
                    break;

                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
        }


        if (((startOffset % vlanEntrySize) != 0) ||
            ((size % vlanEntrySize) != 0))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* Set VLAN entry Burst size */
        ret = prvCpssDiagBurstSizeSet (vlanEntrySize);
        if (ret != GT_OK)
        {
            return ret;
        }

        /* Configure Mask Array */
        ret = prvCpssDiagMemMaskArraySet ((vlanEntrySize / 4), maskArray);
        if (ret != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    }


    /* Get valid portGroup bitmap */
    if(GT_FALSE ==
        prvCpssDuplicatedMultiPortGroupsGet(
            devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
            testBase,&portGroupsBmp,NULL,NULL))

    {
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    }

    ret = prvCpssDiagAnyMemTest(devNum, portGroupsBmp, testBase, size, profile,
                                specialRamFuncs, testStatusPtr, addrPtr,
                                readValPtr, writeValPtr);
    if (ret != GT_OK)
    {
        return ret;
    }

    ret = prvCpssDiagBurstSizeSet(savedBurstSize);
    if (ret != GT_OK)
    {
        return ret;
    }

    if (memType == CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E)
    {
        /* Restore MAC action mode */
        if (macActionMode == CPSS_ACT_AUTO_E)
        {
            ret = cpssDxChBrgFdbMacTriggerModeSet(devNum,macActionMode);
            if (ret != GT_OK)
            {
                return ret;
            }
        }

    }

    return GT_OK;
}

/**
* @internal cpssDxChDiagMemTest function
* @endinternal
*
* @brief   Performs memory test on a specified memory location and size for a
*         specified memory type.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number to test
* @param[in] memType                  - The packet processor memory type to verify.
* @param[in] startOffset              - The offset address to start the test from.
* @param[in] size                     - The memory size in byte to test (start from offset).
* @param[in] profile                  - The test profile.
*
* @param[out] testStatusPtr            - (pointer to) test status. GT_TRUE if the test succeeded or GT_FALSE for failure
* @param[out] addrPtr                  - (pointer to) address offset of memory error, if testStatusPtr is
*                                      GT_FALSE. Irrelevant if testStatusPtr is GT_TRUE.
* @param[out] readValPtr               - (pointer to) value read from the register which caused
*                                      the failure if testStatusPtr is GT_FALSE. Irrelevant if
*                                      testStatusPtr is GT_TRUE.
* @param[out] writeValPtr              - (pointer to) value written to the register which caused
*                                      the failure if testStatusPtr is GT_FALSE. Irrelevant if
*                                      testStatusPtr is GT_TRUE.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong memory type
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The test is done by writing and reading a test pattern.
*       The function may be called after Phase 2 initialization.
*       The test is destructive and leaves the memory corrupted.
*       Supported memories:
*       - Buffer DRAM
*       - MAC table memory
*       - VLAN table memory
*       For buffer DRAM:
*       startOffset must be aligned to 64 Bytes and size must be in 64 bytes
*       resolution.
*       For MAC table:
*       startOffset must be aligned to 16 Bytes and size must be in 16 bytes
*       resolution.
*       For VLAN table:
*       DX CH devices: startOffset must be aligned to 12 Bytes and size must
*       be in 12 bytes resolution.
*       DX CH2 and above devices: startOffset must be aligned to 16 Bytes
*       and size must be in 16 bytes resolution.
*/
GT_STATUS cpssDxChDiagMemTest
(
    IN GT_U8                          devNum,
    IN CPSS_DIAG_PP_MEM_TYPE_ENT      memType,
    IN GT_U32                         startOffset,
    IN GT_U32                         size,
    IN CPSS_DIAG_TEST_PROFILE_ENT     profile,
    OUT GT_BOOL                       *testStatusPtr,
    OUT GT_U32                        *addrPtr,
    OUT GT_U32                        *readValPtr,
    OUT GT_U32                        *writeValPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagMemTest);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, memType, startOffset, size, profile, testStatusPtr, addrPtr, readValPtr, writeValPtr));

    rc = internal_cpssDxChDiagMemTest(devNum, memType, startOffset, size, profile, testStatusPtr, addrPtr, readValPtr, writeValPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, memType, startOffset, size, profile, testStatusPtr, addrPtr, readValPtr, writeValPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChDiagAllMemTest function
* @endinternal
*
* @brief   Performs memory test for all the internal and external memories.
*         Tested memories:
*         - Buffer DRAM
*         - MAC table memory
*         - VLAN table memory
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number to test
*
* @param[out] testStatusPtr            - GT_TRUE if the test succeeded or GT_FALSE for failure
* @param[out] addrPtr                  - Address offset of memory error, if testStatusPtr is
*                                      GT_FALSE. Irrelevant if testStatusPtr is GT_TRUE.
* @param[out] readValPtr               - Contains the value read from the register which caused
*                                      the failure if testStatusPtr is GT_FALSE. Irrelevant if
*                                      testStatusPtr is GT_TRUE.
* @param[out] writeValPtr              - Contains the value written to the register which caused
*                                      the failure if testStatusPtr is GT_FALSE. Irrelevant if
*                                      testStatusPtr is GT_TRUE.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The test is done by invoking cpssDxChDiagMemTest in loop for all the
*       memory types and for AA-55, random and incremental patterns.
*       The function may be called after Phase 2 initialization.
*       The test is destructive and leaves the memory corrupted.
*/
static GT_STATUS internal_cpssDxChDiagAllMemTest
(
    IN GT_U8                     devNum,
    OUT GT_BOOL                  *testStatusPtr,
    OUT GT_U32                   *addrPtr,
    OUT GT_U32                   *readValPtr,
    OUT GT_U32                   *writeValPtr
)
{
    CPSS_DRAM_SIZE_ENT size;           /* size of DRAM */
    CPSS_DIAG_PP_MEM_TYPE_ENT memType; /* memory type */
    GT_STATUS status;                  /* returned status */
    GT_U32    bank0Start;              /* start of buffer DRAM bank 0 (unused)*/
    GT_U32    bank0Size;               /* size of buffer DRAM bank 0 */
    GT_U32    bank1Start;              /* start of buffer DRAM bank 1 (unused)*/
    GT_U32    bank1Size;               /* size of buffer DRAM bank 1 */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(testStatusPtr);
    CPSS_NULL_PTR_CHECK_MAC(addrPtr);
    CPSS_NULL_PTR_CHECK_MAC(readValPtr);
    CPSS_NULL_PTR_CHECK_MAC(writeValPtr);

    /* Lion2 do not support CPU access to the buffer memory */
    if ((PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_LION2_E) &&
        (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_FALSE))
    {
        /* Check Buffer RAM */
        memType = CPSS_DIAG_PP_MEM_BUFFER_DRAM_E;
        getBanksParams (devNum, &bank0Start, &bank0Size, &bank1Start, &bank1Size);

        size = bank0Size + bank1Size;
        status = prvCpssDiagMemoryForAllProfiles (devNum, memType, size,
                                                  &cpssDxChDiagMemTest,
                                                  testStatusPtr, addrPtr,
                                                  readValPtr, writeValPtr);
        PRV_CPSS_DIAG_CHECK_TEST_RESULT_MAC(status, *testStatusPtr);
    }

    /* Bobcat2; Caelum; Bobcat3 do not support CPU access to the FDB memory */
    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        /* Check MAC table memory */
        memType = CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E;
        size = getMacTableSize (devNum);
        status = prvCpssDiagMemoryForAllProfiles (devNum, memType, size,
                                                  &cpssDxChDiagMemTest,
                                                  testStatusPtr, addrPtr,
                                                  readValPtr, writeValPtr);
        PRV_CPSS_DIAG_CHECK_TEST_RESULT_MAC(status, *testStatusPtr);
    }

    /* Check VLAN table memory */
    memType = CPSS_DIAG_PP_MEM_VLAN_TBL_MEM_E;
    size = getVlanTableSize (devNum);
    status = prvCpssDiagMemoryForAllProfiles (devNum, memType, size,
                                              &cpssDxChDiagMemTest,
                                              testStatusPtr, addrPtr,
                                              readValPtr, writeValPtr);
    return status;
}

/**
* @internal cpssDxChDiagAllMemTest function
* @endinternal
*
* @brief   Performs memory test for all the internal and external memories.
*         Tested memories:
*         - Buffer DRAM
*         - MAC table memory
*         - VLAN table memory
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number to test
*
* @param[out] testStatusPtr            - GT_TRUE if the test succeeded or GT_FALSE for failure
* @param[out] addrPtr                  - Address offset of memory error, if testStatusPtr is
*                                      GT_FALSE. Irrelevant if testStatusPtr is GT_TRUE.
* @param[out] readValPtr               - Contains the value read from the register which caused
*                                      the failure if testStatusPtr is GT_FALSE. Irrelevant if
*                                      testStatusPtr is GT_TRUE.
* @param[out] writeValPtr              - Contains the value written to the register which caused
*                                      the failure if testStatusPtr is GT_FALSE. Irrelevant if
*                                      testStatusPtr is GT_TRUE.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The test is done by invoking cpssDxChDiagMemTest in loop for all the
*       memory types and for AA-55, random and incremental patterns.
*       The function may be called after Phase 2 initialization.
*       The test is destructive and leaves the memory corrupted.
*/
GT_STATUS cpssDxChDiagAllMemTest
(
    IN GT_U8                     devNum,
    OUT GT_BOOL                  *testStatusPtr,
    OUT GT_U32                   *addrPtr,
    OUT GT_U32                   *readValPtr,
    OUT GT_U32                   *writeValPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagAllMemTest);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, testStatusPtr, addrPtr, readValPtr, writeValPtr));

    rc = internal_cpssDxChDiagAllMemTest(devNum, testStatusPtr, addrPtr, readValPtr, writeValPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, testStatusPtr, addrPtr, readValPtr, writeValPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChDiagMemWrite function
* @endinternal
*
* @brief   performs a single 32 bit data write to one of the PP memory spaces.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
* @param[in] memType                  - The packet processor memory type
* @param[in] offset                   - The offset address to write to
* @param[in] data                     - data to write
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_TIMEOUT               - after max time that the PP not finished action
* @retval GT_BAD_PARAM             - on wrong devNum, memType or offset
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note offset must be aligned to 4 Bytes.
*       The function may be called after Phase 2 initialization.
*/
static GT_STATUS internal_cpssDxChDiagMemWrite
(
    IN GT_U8                          devNum,
    IN CPSS_DIAG_PP_MEM_TYPE_ENT      memType,
    IN GT_U32                         offset,
    IN GT_U32                         data
)
{
    return cpssDxChDiagPortGroupMemWrite(devNum,
                                         CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                         memType, offset, data);
}

/**
* @internal cpssDxChDiagMemWrite function
* @endinternal
*
* @brief   performs a single 32 bit data write to one of the PP memory spaces.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
* @param[in] memType                  - The packet processor memory type
* @param[in] offset                   - The offset address to write to
* @param[in] data                     - data to write
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_TIMEOUT               - after max time that the PP not finished action
* @retval GT_BAD_PARAM             - on wrong devNum, memType or offset
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note offset must be aligned to 4 Bytes.
*       The function may be called after Phase 2 initialization.
*/
GT_STATUS cpssDxChDiagMemWrite
(
    IN GT_U8                          devNum,
    IN CPSS_DIAG_PP_MEM_TYPE_ENT      memType,
    IN GT_U32                         offset,
    IN GT_U32                         data
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagMemWrite);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, memType, offset, data));

    rc = internal_cpssDxChDiagMemWrite(devNum, memType, offset, data);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, memType, offset, data));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChDiagMemRead function
* @endinternal
*
* @brief   performs a single 32 bit data read from one of the PP memory spaces.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
* @param[in] memType                  - The packet processor memory type
* @param[in] offset                   - The offset address to read from
*
* @param[out] dataPtr                  - (pointer to) read data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_BAD_PARAM             - on wrong devNum, memType or offset
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note offset must be aligned to 4 Bytes.
*       The function may be called after Phase 2 initialization.
*
*/
static GT_STATUS internal_cpssDxChDiagMemRead
(
    IN GT_U8                          devNum,
    IN CPSS_DIAG_PP_MEM_TYPE_ENT      memType,
    IN GT_U32                         offset,
    OUT GT_U32                        *dataPtr
)
{
    return cpssDxChDiagPortGroupMemRead(devNum,
                                        CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                        memType, offset, dataPtr);
}

/**
* @internal cpssDxChDiagMemRead function
* @endinternal
*
* @brief   performs a single 32 bit data read from one of the PP memory spaces.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
* @param[in] memType                  - The packet processor memory type
* @param[in] offset                   - The offset address to read from
*
* @param[out] dataPtr                  - (pointer to) read data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_BAD_PARAM             - on wrong devNum, memType or offset
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note offset must be aligned to 4 Bytes.
*       The function may be called after Phase 2 initialization.
*
*/
GT_STATUS cpssDxChDiagMemRead
(
    IN GT_U8                          devNum,
    IN CPSS_DIAG_PP_MEM_TYPE_ENT      memType,
    IN GT_U32                         offset,
    OUT GT_U32                        *dataPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagMemRead);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, memType, offset, dataPtr));

    rc = internal_cpssDxChDiagMemRead(devNum, memType, offset, dataPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, memType, offset, dataPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChDiagRegWrite function
* @endinternal
*
* @brief   Performs single 32 bit data write to one of the PP PCI configuration or
*         registers.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] baseAddr                 - The base address to access the device
* @param[in] ifChannel                - interface channel (PCI/SMI/TWSI)
* @param[in] regType                  - The register type
* @param[in] offset                   - the register offset
* @param[in] data                     - data to write
* @param[in] doByteSwap               - GT_TRUE:  byte swap will be done on the written data
*                                      GT_FALSE: byte swap will not be done on the written data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong input parameter
*
* @note The function may be called before Phase 1 initialization and
*       can NOT be used after cpssHwPpPhase1 to read/write registers
*
*/
static GT_STATUS internal_cpssDxChDiagRegWrite
(
    IN GT_UINTPTR                      baseAddr,
    IN CPSS_PP_INTERFACE_CHANNEL_ENT   ifChannel,
    IN CPSS_DIAG_PP_REG_TYPE_ENT       regType,
    IN GT_U32                          offset,
    IN GT_U32                          data,
    IN GT_BOOL                         doByteSwap
)
{
    return prvCpssDiagRegWrite (baseAddr, ifChannel, regType, offset, data,
                                doByteSwap);
}

/**
* @internal cpssDxChDiagRegWrite function
* @endinternal
*
* @brief   Performs single 32 bit data write to one of the PP PCI configuration or
*         registers.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] baseAddr                 - The base address to access the device
* @param[in] ifChannel                - interface channel (PCI/SMI/TWSI)
* @param[in] regType                  - The register type
* @param[in] offset                   - the register offset
* @param[in] data                     - data to write
* @param[in] doByteSwap               - GT_TRUE:  byte swap will be done on the written data
*                                      GT_FALSE: byte swap will not be done on the written data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong input parameter
*
* @note The function may be called before Phase 1 initialization and
*       can NOT be used after cpssHwPpPhase1 to read/write registers
*
*/
GT_STATUS cpssDxChDiagRegWrite
(
    IN GT_UINTPTR                      baseAddr,
    IN CPSS_PP_INTERFACE_CHANNEL_ENT   ifChannel,
    IN CPSS_DIAG_PP_REG_TYPE_ENT       regType,
    IN GT_U32                          offset,
    IN GT_U32                          data,
    IN GT_BOOL                         doByteSwap
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagRegWrite);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, baseAddr, ifChannel, regType, offset, data, doByteSwap));

    rc = internal_cpssDxChDiagRegWrite(baseAddr, ifChannel, regType, offset, data, doByteSwap);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, baseAddr, ifChannel, regType, offset, data, doByteSwap));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChDiagRegRead function
* @endinternal
*
* @brief   Performs single 32 bit data read from one of the PP PCI configuration or
*         registers.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] baseAddr                 - The base address to access the device
* @param[in] ifChannel                - interface channel (PCI/SMI/TWSI)
* @param[in] regType                  - The register type
* @param[in] offset                   - the register offset
* @param[in] doByteSwap               - GT_TRUE:  byte swap will be done on the read data
*                                      GT_FALSE: byte swap will not be done on the read data
*
* @param[out] dataPtr                  - read data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong input parameter
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note The function may be called before Phase 1 initialization and
*       can NOT be used after cpssHwPpPhase1 to read/write registers
*
*/
static GT_STATUS internal_cpssDxChDiagRegRead
(
    IN GT_UINTPTR                      baseAddr,
    IN CPSS_PP_INTERFACE_CHANNEL_ENT   ifChannel,
    IN CPSS_DIAG_PP_REG_TYPE_ENT       regType,
    IN GT_U32                          offset,
    OUT GT_U32                         *dataPtr,
    IN GT_BOOL                         doByteSwap
)
{
    CPSS_NULL_PTR_CHECK_MAC(dataPtr);

    return prvCpssDiagRegRead (baseAddr, ifChannel, regType, offset, dataPtr,
                               doByteSwap);
}

/**
* @internal cpssDxChDiagRegRead function
* @endinternal
*
* @brief   Performs single 32 bit data read from one of the PP PCI configuration or
*         registers.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] baseAddr                 - The base address to access the device
* @param[in] ifChannel                - interface channel (PCI/SMI/TWSI)
* @param[in] regType                  - The register type
* @param[in] offset                   - the register offset
* @param[in] doByteSwap               - GT_TRUE:  byte swap will be done on the read data
*                                      GT_FALSE: byte swap will not be done on the read data
*
* @param[out] dataPtr                  - read data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong input parameter
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note The function may be called before Phase 1 initialization and
*       can NOT be used after cpssHwPpPhase1 to read/write registers
*
*/
GT_STATUS cpssDxChDiagRegRead
(
    IN GT_UINTPTR                      baseAddr,
    IN CPSS_PP_INTERFACE_CHANNEL_ENT   ifChannel,
    IN CPSS_DIAG_PP_REG_TYPE_ENT       regType,
    IN GT_U32                          offset,
    OUT GT_U32                         *dataPtr,
    IN GT_BOOL                         doByteSwap
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagRegRead);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, baseAddr, ifChannel, regType, offset, dataPtr, doByteSwap));

    rc = internal_cpssDxChDiagRegRead(baseAddr, ifChannel, regType, offset, dataPtr, doByteSwap);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, baseAddr, ifChannel, regType, offset, dataPtr, doByteSwap));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChDiagPhyRegWrite function
* @endinternal
*
* @brief   Performs single 32 bit data write to one of the PHY registers.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] baseAddr                 - The base address to access the device
* @param[in] ifChannel                - interface channel (PCI/SMI/TWSI)
* @param[in] smiRegOffset             - The SMI register offset
* @param[in] phyAddr                  - phy address to access
* @param[in] offset                   - PHY register offset
* @param[in] data                     - data to write
* @param[in] doByteSwap               - GT_TRUE:  byte swap will be done on the written data
*                                      GT_FALSE: byte swap will not be done on the written data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong input parameter
*
* @note The function may be called before Phase 1 initialization.
*
*/
static GT_STATUS internal_cpssDxChDiagPhyRegWrite
(
    IN GT_U32                          baseAddr,
    IN CPSS_PP_INTERFACE_CHANNEL_ENT   ifChannel,
    IN GT_U32                          smiRegOffset,
    IN GT_U32                          phyAddr,
    IN GT_U32                          offset,
    IN GT_U32                          data,
    IN GT_BOOL                         doByteSwap
)
{
    return prvCpssDiagPhyRegWrite (baseAddr, ifChannel, smiRegOffset, phyAddr,
                                   offset, data, doByteSwap);
}

/**
* @internal cpssDxChDiagPhyRegWrite function
* @endinternal
*
* @brief   Performs single 32 bit data write to one of the PHY registers.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] baseAddr                 - The base address to access the device
* @param[in] ifChannel                - interface channel (PCI/SMI/TWSI)
* @param[in] smiRegOffset             - The SMI register offset
* @param[in] phyAddr                  - phy address to access
* @param[in] offset                   - PHY register offset
* @param[in] data                     - data to write
* @param[in] doByteSwap               - GT_TRUE:  byte swap will be done on the written data
*                                      GT_FALSE: byte swap will not be done on the written data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong input parameter
*
* @note The function may be called before Phase 1 initialization.
*
*/
GT_STATUS cpssDxChDiagPhyRegWrite
(
    IN GT_U32                          baseAddr,
    IN CPSS_PP_INTERFACE_CHANNEL_ENT   ifChannel,
    IN GT_U32                          smiRegOffset,
    IN GT_U32                          phyAddr,
    IN GT_U32                          offset,
    IN GT_U32                          data,
    IN GT_BOOL                         doByteSwap
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagPhyRegWrite);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, baseAddr, ifChannel, smiRegOffset, phyAddr, offset, data, doByteSwap));

    rc = internal_cpssDxChDiagPhyRegWrite(baseAddr, ifChannel, smiRegOffset, phyAddr, offset, data, doByteSwap);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, baseAddr, ifChannel, smiRegOffset, phyAddr, offset, data, doByteSwap));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChDiagPhyRegRead function
* @endinternal
*
* @brief   Performs single 32 bit data read from one of the PHY registers.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] baseAddr                 - The base address to access the device
* @param[in] ifChannel                - interface channel (PCI/SMI/TWSI)
* @param[in] smiRegOffset             - The SMI register offset
* @param[in] phyAddr                  - phy address to access
* @param[in] offset                   - PHY register offset
* @param[in] doByteSwap               - GT_TRUE:  byte swap will be done on the read data
*                                      GT_FALSE: byte swap will not be done on the read data
*
* @param[out] dataPtr                  - (pointer to) read data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong input parameter
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note The function may be called before Phase 1 initialization.
*
*/
static GT_STATUS internal_cpssDxChDiagPhyRegRead
(
    IN GT_U32                          baseAddr,
    IN CPSS_PP_INTERFACE_CHANNEL_ENT   ifChannel,
    IN GT_U32                          smiRegOffset,
    IN GT_U32                          phyAddr,
    IN GT_U32                          offset,
    OUT GT_U32                         *dataPtr,
    IN GT_BOOL                         doByteSwap
)
{
    CPSS_NULL_PTR_CHECK_MAC(dataPtr);

    return prvCpssDiagPhyRegRead (baseAddr, ifChannel, smiRegOffset, phyAddr,
                                  offset, dataPtr, doByteSwap);
}

/**
* @internal cpssDxChDiagPhyRegRead function
* @endinternal
*
* @brief   Performs single 32 bit data read from one of the PHY registers.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] baseAddr                 - The base address to access the device
* @param[in] ifChannel                - interface channel (PCI/SMI/TWSI)
* @param[in] smiRegOffset             - The SMI register offset
* @param[in] phyAddr                  - phy address to access
* @param[in] offset                   - PHY register offset
* @param[in] doByteSwap               - GT_TRUE:  byte swap will be done on the read data
*                                      GT_FALSE: byte swap will not be done on the read data
*
* @param[out] dataPtr                  - (pointer to) read data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong input parameter
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note The function may be called before Phase 1 initialization.
*
*/
GT_STATUS cpssDxChDiagPhyRegRead
(
    IN GT_U32                          baseAddr,
    IN CPSS_PP_INTERFACE_CHANNEL_ENT   ifChannel,
    IN GT_U32                          smiRegOffset,
    IN GT_U32                          phyAddr,
    IN GT_U32                          offset,
    OUT GT_U32                         *dataPtr,
    IN GT_BOOL                         doByteSwap
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagPhyRegRead);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, baseAddr, ifChannel, smiRegOffset, phyAddr, offset, dataPtr, doByteSwap));

    rc = internal_cpssDxChDiagPhyRegRead(baseAddr, ifChannel, smiRegOffset, phyAddr, offset, dataPtr, doByteSwap);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, baseAddr, ifChannel, smiRegOffset, phyAddr, offset, dataPtr, doByteSwap));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChDiagRegsNumGet function
* @endinternal
*
* @brief   Gets the number of registers for the PP.
*         Used to allocate memory for cpssDxChDiagRegsDump.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
*
* @param[out] regsNumPtr               - (pointer to) number of registers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*
*/
static GT_STATUS internal_cpssDxChDiagRegsNumGet
(
    IN  GT_U8     devNum,
    OUT GT_U32    *regsNumPtr
)
{
    return cpssDxChDiagPortGroupRegsNumGet(devNum,
                                           CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                           regsNumPtr);
}

/**
* @internal cpssDxChDiagRegsNumGet function
* @endinternal
*
* @brief   Gets the number of registers for the PP.
*         Used to allocate memory for cpssDxChDiagRegsDump.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
*
* @param[out] regsNumPtr               - (pointer to) number of registers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*
*/
GT_STATUS cpssDxChDiagRegsNumGet
(
    IN  GT_U8     devNum,
    OUT GT_U32    *regsNumPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagRegsNumGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, regsNumPtr));

    rc = internal_cpssDxChDiagRegsNumGet(devNum, regsNumPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, regsNumPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChDiagResetAndInitControllerRegsNumGet function
* @endinternal
*
* @brief   Gets the number of registers for the Reset and Init Controller.
*         Used to allocate memory for cpssDxChDiagResetAndInitControllerRegsDump.
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The device number
*
* @param[out] regsNumPtr               - (pointer to) number of registers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*
*/
static GT_STATUS internal_cpssDxChDiagResetAndInitControllerRegsNumGet
(
    IN  GT_U8     devNum,
    OUT GT_U32    *regsNumPtr
)
{
    GT_U32 *regsListPtr;    /* holds all elements in */
                     /* PRV_CPSS_DXCH_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC */
    GT_U32 regsListSize;    /* size of regsListPtr */
    GT_STATUS   ret;        /* function return result */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
                                          CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E |
                                          CPSS_CH3_E | CPSS_XCAT_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(regsNumPtr);

    if(PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        *regsNumPtr = 0;
        return GT_OK;
    }

    regsListPtr =
        (GT_U32*)PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum);
    regsListSize = sizeof(PRV_CPSS_RESET_AND_INIT_CTRL_REGS_ADDR_STC) /
                                                                sizeof(GT_U32);
    ret = prvCpssDiagRegsNumGet(devNum,
                                CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                regsListPtr,
                                regsListSize,
                                0,
                                &checkIfRegisterExist,
                                regsNumPtr);
    if (ret != GT_OK)
    {
        return ret;
    }

    return GT_OK;
}

/**
* @internal cpssDxChDiagResetAndInitControllerRegsNumGet function
* @endinternal
*
* @brief   Gets the number of registers for the Reset and Init Controller.
*         Used to allocate memory for cpssDxChDiagResetAndInitControllerRegsDump.
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The device number
*
* @param[out] regsNumPtr               - (pointer to) number of registers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*
*/
GT_STATUS cpssDxChDiagResetAndInitControllerRegsNumGet
(
    IN  GT_U8     devNum,
    OUT GT_U32    *regsNumPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagResetAndInitControllerRegsNumGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, regsNumPtr));

    rc = internal_cpssDxChDiagResetAndInitControllerRegsNumGet(devNum, regsNumPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, regsNumPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChDiagRegsDump function
* @endinternal
*
* @brief   Dumps the device register addresses and values according to the given
*         starting offset and number of registers to dump.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
* @param[in,out] regsNumPtr            in: number of registers to dump. This number
*                                          must not be bigger than the number of registers
*                                          that can be dumped (starting at offset).
*                                      out: number of registers that were actually dumped
* @param[in] offset                   - The first register address to dump
* @param[out] regAddrPtr               - (pointer to) addresses of the dumped registers
*                                      The addresses are taken from the register DB.
* @param[out] regDataPtr               - (pointer to) data in the dumped registers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*       To dump all the registers the user may call cpssDxChDiagRegsNumGet in
*       order to get the number of registers of the PP.
*/
static GT_STATUS internal_cpssDxChDiagRegsDump
(
    IN    GT_U8     devNum,
    INOUT GT_U32    *regsNumPtr,
    IN    GT_U32    offset,
    OUT   GT_U32    *regAddrPtr,
    OUT   GT_U32    *regDataPtr
)
{
    return cpssDxChDiagPortGroupRegsDump(devNum,
                                         CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                         regsNumPtr,
                                         offset,
                                         regAddrPtr,
                                         regDataPtr);
}

/**
* @internal cpssDxChDiagRegsDump function
* @endinternal
*
* @brief   Dumps the device register addresses and values according to the given
*         starting offset and number of registers to dump.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
* @param[in,out] regsNumPtr            in: number of registers to dump. This number
*                                          must not be bigger than the number of registers
*                                          that can be dumped (starting at offset).
*                                      out: number of registers that were actually dumped
* @param[in] offset                   - The first register address to dump
* @param[out] regAddrPtr               - (pointer to) addresses of the dumped registers
*                                      The addresses are taken from the register DB.
* @param[out] regDataPtr               - (pointer to) data in the dumped registers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*       To dump all the registers the user may call cpssDxChDiagRegsNumGet in
*       order to get the number of registers of the PP.
*/
GT_STATUS cpssDxChDiagRegsDump
(
    IN    GT_U8     devNum,
    INOUT GT_U32    *regsNumPtr,
    IN    GT_U32    offset,
    OUT   GT_U32    *regAddrPtr,
    OUT   GT_U32    *regDataPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagRegsDump);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, regsNumPtr, offset, regAddrPtr, regDataPtr));

    rc = internal_cpssDxChDiagRegsDump(devNum, regsNumPtr, offset, regAddrPtr, regDataPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, regsNumPtr, offset, regAddrPtr, regDataPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChDiagResetAndInitControllerRegsDump function
* @endinternal
*
* @brief   Dumps the Reset and Init controller register addresses and values
*         according to the given starting offset and number of registers to dump.
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The device number
* @param[in,out] regsNumPtr            in: number of registers to dump. This number
*                                          must not be bigger than the number of registers
*                                          that can be dumped (starting at offset).
*                                      out: number of registers that were actually dumped
* @param[in] offset                   - The first register address to dump
* @param[out] regAddrPtr               - (pointer to) addresses of the dumped registers
*                                      The addresses are taken from the register DB.
* @param[out] regDataPtr               - (pointer to) data in the dumped registers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*       To dump all the registers the user may call
*       cpssDxChDiagResetAndInitControllerRegsNumGet in order to get the number
*       of registers of the Reset and Init Controller.
*/
static GT_STATUS internal_cpssDxChDiagResetAndInitControllerRegsDump
(
    IN    GT_U8     devNum,
    INOUT GT_U32    *regsNumPtr,
    IN    GT_U32    offset,
    OUT   GT_U32    *regAddrPtr,
    OUT   GT_U32    *regDataPtr
)
{
    GT_U32 *regsListPtr;  /* holds all elements in */
                     /* PRV_CPSS_DXCH_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC */
    GT_U32 regsListSize;  /* size of regsListPtr */
    GT_STATUS   ret;        /* function return result */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
                                          CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E |
                                          CPSS_CH3_E | CPSS_XCAT_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(regsNumPtr);
    CPSS_NULL_PTR_CHECK_MAC(regAddrPtr);
    CPSS_NULL_PTR_CHECK_MAC(regDataPtr);

    if(PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        return GT_OK;
    }

    regsListPtr =
        (GT_U32*)PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum);
    regsListSize = sizeof(PRV_CPSS_RESET_AND_INIT_CTRL_REGS_ADDR_STC) /
                                                                sizeof(GT_U32);

    ret = prvCpssDiagResetAndInitControllerRegsDataGet(devNum,
                                                       regsListPtr,
                                                       regsListSize,
                                                       offset,
                                                       &checkIfRegisterExist,
                                                       regsNumPtr,
                                                       regAddrPtr,
                                                       regDataPtr);
    if (ret != GT_OK)
    {
        return ret;
    }

    return GT_OK;
}

/**
* @internal cpssDxChDiagResetAndInitControllerRegsDump function
* @endinternal
*
* @brief   Dumps the Reset and Init controller register addresses and values
*         according to the given starting offset and number of registers to dump.
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The device number
* @param[in,out] regsNumPtr            in: number of registers to dump. This number
*                                          must not be bigger than the number of registers
*                                          that can be dumped (starting at offset).
*                                      out: number of registers that were actually dumped
* @param[in] offset                   - The first register address to dump
* @param[out] regAddrPtr               - (pointer to) addresses of the dumped registers
*                                      The addresses are taken from the register DB.
* @param[out] regDataPtr               - (pointer to) data in the dumped registers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*       To dump all the registers the user may call
*       cpssDxChDiagResetAndInitControllerRegsNumGet in order to get the number
*       of registers of the Reset and Init Controller.
*/
GT_STATUS cpssDxChDiagResetAndInitControllerRegsDump
(
    IN    GT_U8     devNum,
    INOUT GT_U32    *regsNumPtr,
    IN    GT_U32    offset,
    OUT   GT_U32    *regAddrPtr,
    OUT   GT_U32    *regDataPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagResetAndInitControllerRegsDump);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, regsNumPtr, offset, regAddrPtr, regDataPtr));

    rc = internal_cpssDxChDiagResetAndInitControllerRegsDump(devNum, regsNumPtr, offset, regAddrPtr, regDataPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, regsNumPtr, offset, regAddrPtr, regDataPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChDiagRegTest function
* @endinternal
*
* @brief   Tests the device read/write ability of a specific register.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
* @param[in] regAddr                  - Register to test
* @param[in] regMask                  - Register mask. The test verifies only the non-masked
*                                      bits.
* @param[in] profile                  - The test profile
*
* @param[out] testStatusPtr            - (pointer to) to test result. GT_TRUE if the test succeeded or GT_FALSE for failure
* @param[out] readValPtr               - (pointer to) value read from the register if testStatusPtr is
*                                      GT_FALSE. Irrelevant if testStatusPtr is GT_TRUE.
* @param[out] writeValPtr              - (pointer to) value written to the register if testStatusPtr is
*                                      GT_FALSE. Irrelevant if testStatusPtr is GT_TRUE.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*       After the register is tested, the original value prior to the test is
*       restored.
*
*/
static GT_STATUS internal_cpssDxChDiagRegTest
(
    IN GT_U8                          devNum,
    IN GT_U32                         regAddr,
    IN GT_U32                         regMask,
    IN CPSS_DIAG_TEST_PROFILE_ENT     profile,
    OUT GT_BOOL                       *testStatusPtr,
    OUT GT_U32                        *readValPtr,
    OUT GT_U32                        *writeValPtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(testStatusPtr);
    CPSS_NULL_PTR_CHECK_MAC(readValPtr);
    CPSS_NULL_PTR_CHECK_MAC(writeValPtr);

    return prvCpssDiagRegTest (devNum, regAddr, regMask, profile, testStatusPtr,
                               readValPtr, writeValPtr);
}

/**
* @internal cpssDxChDiagRegTest function
* @endinternal
*
* @brief   Tests the device read/write ability of a specific register.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
* @param[in] regAddr                  - Register to test
* @param[in] regMask                  - Register mask. The test verifies only the non-masked
*                                      bits.
* @param[in] profile                  - The test profile
*
* @param[out] testStatusPtr            - (pointer to) to test result. GT_TRUE if the test succeeded or GT_FALSE for failure
* @param[out] readValPtr               - (pointer to) value read from the register if testStatusPtr is
*                                      GT_FALSE. Irrelevant if testStatusPtr is GT_TRUE.
* @param[out] writeValPtr              - (pointer to) value written to the register if testStatusPtr is
*                                      GT_FALSE. Irrelevant if testStatusPtr is GT_TRUE.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*       After the register is tested, the original value prior to the test is
*       restored.
*
*/
GT_STATUS cpssDxChDiagRegTest
(
    IN GT_U8                          devNum,
    IN GT_U32                         regAddr,
    IN GT_U32                         regMask,
    IN CPSS_DIAG_TEST_PROFILE_ENT     profile,
    OUT GT_BOOL                       *testStatusPtr,
    OUT GT_U32                        *readValPtr,
    OUT GT_U32                        *writeValPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagRegTest);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, regAddr, regMask, profile, testStatusPtr, readValPtr, writeValPtr));

    rc = internal_cpssDxChDiagRegTest(devNum, regAddr, regMask, profile, testStatusPtr, readValPtr, writeValPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, regAddr, regMask, profile, testStatusPtr, readValPtr, writeValPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChDiagAllRegTest function
* @endinternal
*
* @brief   Tests the device read/write ability of all the registers.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
*
* @param[out] testStatusPtr            -is GT_FALSE. Irrelevant if testStatusPtr
* @param[out] badRegPtr                - (pointer to) address of the register which caused
*                                        the failure if testStatusPtr is GT_FALSE.
*                                        Irrelevant if is GT_TRUE.
* @param[out] readValPtr               - (pointer to) value read from the register which caused
*                                        the failure if testStatusPtr is GT_FALSE.
*                                        Irrelevant if is GT_TRUE.
* @param[out] writeValPtr              - (pointer to) value written to the register which caused
*                                        the failure if testStatusPtr is GT_FALSE.
*                                        Irrelevant if is GT_TRUE.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The test is done by invoking cpssDxChDiagRegTest in loop for all the
*       diagnostics registers and for all the patterns.
*       After each register is tested, the original value prior to the test is
*       restored.
*       The function may be called after Phase 1 initialization.
*/
static GT_STATUS internal_cpssDxChDiagAllRegTest
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *testStatusPtr,
    OUT GT_U32  *badRegPtr,
    OUT GT_U32  *readValPtr,
    OUT GT_U32  *writeValPtr
)
{
    GT_U32 regsArray[8];        /* diagnostic registers : all 32 bits of the register must be 'RW' (read+write) */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(testStatusPtr);
    CPSS_NULL_PTR_CHECK_MAC(badRegPtr);
    CPSS_NULL_PTR_CHECK_MAC(readValPtr);
    CPSS_NULL_PTR_CHECK_MAC(writeValPtr);

    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regsArray[0] = PRV_DXCH_REG1_UNIT_EGF_EFT_MAC(devNum).deviceMapConfigs.localTrgPortMapOwnDevEn[0];
        if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            regsArray[1] = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
                sip6_packetBuffer.pbCenter.npmRandomizerEnable;
        }
        else
        {
            regsArray[1] = PRV_DXCH_REG1_UNIT_TXQ_DQ_MAC(devNum).scheduler.priorityArbiterWeights.profileSDWRRWeightsConfigReg0[0];
        }
        regsArray[2] = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).IPv6McBridgingBytesSelection.IPv6MCBridgingBytesSelectionConfig;
        if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_lms.notSupported == GT_FALSE)
        {
            regsArray[3] = PRV_DXCH_REG1_UNIT_LMS_MAC(devNum, 0).LMS0.LMS0Group0.LEDInterfaceConfig.LEDInterface0CtrlReg0forPorts0Through11AndStackPort;
        }
        else
        {
            regsArray[3] = PRV_DXCH_REG1_UNIT_PCL_MAC(devNum).L2L3IngrVLANCountingEnable[0];
        }
        regsArray[4] = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).ctrlTrafficToCPUConfig.IEEEReservedMcConfig0[0];
        regsArray[5] = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).layer2BridgeMIBCntrs.MACAddrCount0;
        regsArray[6] = PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.FDBAddrUpdateMsgs.AUMsgFromCPU.AUMsgFromCPUWord[0];
        regsArray[7] = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->IPvX.IPv4GlobalCtrl.IPv4CtrlReg0;
    }
    else
    {
        if ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)||
            (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E))
        {
            regsArray[0] = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.egr.global.egressInterruptMask;
            regsArray[1] = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.dq.shaper.tokenBucketUpdateRate;
            regsArray[7] = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ipRegs.routerGlobalReg;
        }
        else
        {
            regsArray[0] = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->interrupts.txqIntMask;
            regsArray[1] = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.egressFilter;
            regsArray[7] = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->egrTxQConf.txPortRegs[0].wrrWeights0;
        }

        if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
        {
            regsArray[2] = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.vlanMruProfilesConfigReg;
        }
        else
        {
            regsArray[2] = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.vlan1;
        }


        regsArray[3] = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ledRegs.ledControl[0];
        regsArray[4] = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.macRangeFltrBase[0];
        regsArray[5] = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.brgMacCntr0 + 4;
        regsArray[6] = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.addrUpdate[0];
    }

    return prvCpssDiagAllRegTest (devNum, regsArray, NULL/*regMasksPtr*/, 8, testStatusPtr,
                                  badRegPtr, readValPtr, writeValPtr);
}

/**
* @internal cpssDxChDiagAllRegTest function
* @endinternal
*
* @brief   Tests the device read/write ability of all the registers.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
*
* @param[out] testStatusPtr            -is GT_FALSE. Irrelevant if testStatusPtr
* @param[out] badRegPtr                - (pointer to) address of the register which caused
*                                        the failure if testStatusPtr is GT_FALSE.
*                                        Irrelevant if is GT_TRUE.
* @param[out] readValPtr               - (pointer to) value read from the register which caused
*                                        the failure if testStatusPtr is GT_FALSE.
*                                        Irrelevant if is GT_TRUE.
* @param[out] writeValPtr              - (pointer to) value written to the register which caused
*                                        the failure if testStatusPtr is GT_FALSE.
*                                        Irrelevant if is GT_TRUE.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The test is done by invoking cpssDxChDiagRegTest in loop for all the
*       diagnostics registers and for all the patterns.
*       After each register is tested, the original value prior to the test is
*       restored.
*       The function may be called after Phase 1 initialization.
*/
GT_STATUS cpssDxChDiagAllRegTest
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *testStatusPtr,
    OUT GT_U32  *badRegPtr,
    OUT GT_U32  *readValPtr,
    OUT GT_U32  *writeValPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagAllRegTest);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, testStatusPtr, badRegPtr, readValPtr, writeValPtr));

    rc = internal_cpssDxChDiagAllRegTest(devNum, testStatusPtr, badRegPtr, readValPtr, writeValPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, testStatusPtr, badRegPtr, readValPtr, writeValPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/*******************************************************************************
 PRBS sequence for tri-speed ports:
 1.


*******************************************************************************/

/**
* @internal internal_cpssDxChDiagPrbsPortTransmitModeSet function
* @endinternal
*
* @brief   Set transmit mode for specified port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for XAUI/HGS port
* @param[in] mode                     - transmit mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for Tri-speed ports of all DxCh devices
*       and XAUI/HGS ports of DxCh3 and above devices.
*       CPU port doesn't support the transmit mode.
*
*/
static GT_STATUS internal_cpssDxChDiagPrbsPortTransmitModeSet
(
    IN   GT_U8                    devNum,
    IN   GT_PHYSICAL_PORT_NUM     portNum,
    IN   GT_U32                   laneNum,
    IN   CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT   mode
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      macCtrl2RegAddr; /* register address of macCtrl2 */
    PRV_CPSS_DXCH_PORT_STATE_STC   portStateStc;  /* current port state */
    GT_U32      value;      /* register field value */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    PRV_CPSS_PORT_TYPE_ENT portMacType; /* current MAC unit used by port */
    GT_U32 portMacNum; /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);
    portMacType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);

    if(portMacType < PRV_CPSS_PORT_XG_E)
    {
        /* FE ports not support PRBS */
        if(portMacType == PRV_CPSS_PORT_FE_E)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        switch(mode)
        {
            case CPSS_DXCH_DIAG_TRANSMIT_MODE_REGULAR_E:
                value = 0;
                break;
            case CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS_E:
                value = 1;
                break;
            case CPSS_DXCH_DIAG_TRANSMIT_MODE_ZEROS_E:
                value = 2;
                break;
            case CPSS_DXCH_DIAG_TRANSMIT_MODE_ONES_E:
                value = 3;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        /* disable port if we need */
        rc = prvCpssDxChPortStateDisableAndGet(devNum,portNum,&portStateStc);
        if(rc != GT_OK)
        {
            return rc;
        }

        PRV_CPSS_DXCH_PORT_MAC_CTRL2_REG_MAC(devNum,portMacNum,portMacType,&macCtrl2RegAddr);
        /* set SelectDataToTransmit */
        if (prvCpssDrvHwPpPortGroupSetRegField(devNum,portGroupId,macCtrl2RegAddr,12,2,value) != GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);

        /* restore port enable state */
        rc = prvCpssDxChPortStateRestore(devNum,portNum, &portStateStc);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        if (PRV_CPSS_DXCH3_FAMILY_CHECK_MAC(devNum) == 0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }

        if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }

        if (laneNum > PRV_DXCH_DIAG_MAX_XPCS_LANES_NUM_MAC(devNum))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        switch(mode)
        {
            case CPSS_DXCH_DIAG_TRANSMIT_MODE_REGULAR_E:
                value = 0;
                break;
            case CPSS_DXCH_DIAG_TRANSMIT_MODE_CYCLIC_E:
                value = 4;
                break;
            case CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS7_E:
                value = 5;
                break;
            case CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS23_E:
                value = 6;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            macRegs.perPortRegs[portMacNum].laneConfig0[laneNum];
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

        if (prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId,regAddr, 13, 3, value) != GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssDxChDiagPrbsPortTransmitModeSet function
* @endinternal
*
* @brief   Set transmit mode for specified port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for XAUI/HGS port
* @param[in] mode                     - transmit mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for Tri-speed ports of all DxCh devices
*       and XAUI/HGS ports of DxCh3 and above devices.
*       CPU port doesn't support the transmit mode.
*
*/
GT_STATUS cpssDxChDiagPrbsPortTransmitModeSet
(
    IN   GT_U8                    devNum,
    IN   GT_PHYSICAL_PORT_NUM     portNum,
    IN   GT_U32                   laneNum,
    IN   CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT   mode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagPrbsPortTransmitModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, mode));

    rc = internal_cpssDxChDiagPrbsPortTransmitModeSet(devNum, portNum, laneNum, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChDiagPrbsPortTransmitModeGet function
* @endinternal
*
* @brief   Get transmit mode for specified port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for XAUI/HGS port
*
* @param[out] modePtr                  - (pointer to) transmit mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for Tri-speed ports of all DxCh devices
*       and XAUI/HGS ports of DxCh3 and above devices.
*       CPU port doesn't support the transmit mode.
*
*/
static GT_STATUS internal_cpssDxChDiagPrbsPortTransmitModeGet
(
    IN   GT_U8                    devNum,
    IN   GT_PHYSICAL_PORT_NUM     portNum,
    IN   GT_U32                   laneNum,
    OUT  CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT   *modePtr
)
{
    GT_U32      regAddr; /* register address */
    GT_U32      value;   /* register field value */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    PRV_CPSS_PORT_TYPE_ENT portMacType;
    GT_U32 portMacNum; /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(modePtr);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);
    portMacType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);


    if(portMacType < PRV_CPSS_PORT_XG_E)
    {
        /* FE ports not support PRBS */
        if(portMacType == PRV_CPSS_PORT_FE_E)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        PRV_CPSS_DXCH_PORT_MAC_CTRL2_REG_MAC(devNum,portMacNum,portMacType,&regAddr);

        if (prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId,regAddr, 12, 2, &value)!= GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
        switch (value)
        {
            case 0:
                *modePtr = CPSS_DXCH_DIAG_TRANSMIT_MODE_REGULAR_E;
                break;
            case 1:
                *modePtr = CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS_E;
                break;
            case 2:
                *modePtr = CPSS_DXCH_DIAG_TRANSMIT_MODE_ZEROS_E;
                break;
            case 3:
                *modePtr = CPSS_DXCH_DIAG_TRANSMIT_MODE_ONES_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        if (PRV_CPSS_DXCH3_FAMILY_CHECK_MAC(devNum) == 0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }

        if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }

        if (laneNum > PRV_DXCH_DIAG_MAX_XPCS_LANES_NUM_MAC(devNum))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            macRegs.perPortRegs[portMacNum].laneConfig0[laneNum];
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

        if (prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId,regAddr, 13, 3, &value)!= GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
        if ((value & 0x4) == 0)
        {
            *modePtr = CPSS_DXCH_DIAG_TRANSMIT_MODE_REGULAR_E;
        }
        else
        {
            switch (value & 0x3)
            {
                case 0:
                    *modePtr = CPSS_DXCH_DIAG_TRANSMIT_MODE_CYCLIC_E;
                    break;
                case 1:
                    *modePtr = CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS7_E;
                    break;
                case 2:
                    *modePtr = CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS23_E;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
            }
        }
    }
    return GT_OK;
}

/**
* @internal cpssDxChDiagPrbsPortTransmitModeGet function
* @endinternal
*
* @brief   Get transmit mode for specified port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for XAUI/HGS port
*
* @param[out] modePtr                  - (pointer to) transmit mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for Tri-speed ports of all DxCh devices
*       and XAUI/HGS ports of DxCh3 and above devices.
*       CPU port doesn't support the transmit mode.
*
*/
GT_STATUS cpssDxChDiagPrbsPortTransmitModeGet
(
    IN   GT_U8                    devNum,
    IN   GT_PHYSICAL_PORT_NUM     portNum,
    IN   GT_U32                   laneNum,
    OUT  CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT   *modePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagPrbsPortTransmitModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, modePtr));

    rc = internal_cpssDxChDiagPrbsPortTransmitModeGet(devNum, portNum, laneNum, modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChDiagPrbsPortGenerateEnableSet function
* @endinternal
*
* @brief   Enable/Disable PRBS (Pseudo Random Bit Generator) pattern generation
*         per Port and per lane.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for XAUI/HGS port
* @param[in] enable                   - GT_TRUE - PRBS pattern generation is enabled
*                                      GT_FALSE - PRBS pattern generation is disabled
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for Tri-speed ports of all DxCh devices
*       and XAUI/HGS ports of DxCh3 and above devices.
*       When operating, enable the PRBS checker before the generator.
*
*/
static GT_STATUS internal_cpssDxChDiagPrbsPortGenerateEnableSet
(
    IN   GT_U8          devNum,
    IN   GT_PHYSICAL_PORT_NUM  portNum,
    IN   GT_U32         laneNum,
    IN   GT_BOOL        enable
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      macCtrl2RegAddr;
    PRV_CPSS_DXCH_PORT_STATE_STC      portStateStc;  /* current port state */
    GT_U32      value;      /* register field value */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    PRV_CPSS_PORT_TYPE_ENT portMacType;
    GT_U32 portMacNum; /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);
    portMacType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);

    value = (enable == GT_TRUE) ? 1 : 0;

    if(portMacType < PRV_CPSS_PORT_XG_E)
    {
        /* FE ports not support PRBS */
        if(portMacType == PRV_CPSS_PORT_FE_E)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        /* disable port if we need */
        rc = prvCpssDxChPortStateDisableAndGet(devNum,portNum,&portStateStc);
        if(rc != GT_OK)
            return rc;

        PRV_CPSS_DXCH_PORT_MAC_CTRL2_REG_MAC(devNum,portMacNum,portMacType,&macCtrl2RegAddr);
        /* set PRBS Generate enable */
        if (prvCpssDrvHwPpPortGroupSetRegField(devNum,portGroupId,macCtrl2RegAddr,11,1,value) != GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);

        /* restore port enable state */
        rc = prvCpssDxChPortStateRestore(devNum,portNum, &portStateStc);
        if(rc != GT_OK)
            return rc;
    }
    else
    {
        if (PRV_CPSS_DXCH3_FAMILY_CHECK_MAC(devNum) == 0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }

        if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }

        if (laneNum > PRV_DXCH_DIAG_MAX_XPCS_LANES_NUM_MAC(devNum))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            macRegs.perPortRegs[portMacNum].laneConfig0[laneNum];
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

        if (prvCpssDrvHwPpPortGroupSetRegField(devNum,portGroupId, regAddr, 12, 1, value) != GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssDxChDiagPrbsPortGenerateEnableSet function
* @endinternal
*
* @brief   Enable/Disable PRBS (Pseudo Random Bit Generator) pattern generation
*         per Port and per lane.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for XAUI/HGS port
* @param[in] enable                   - GT_TRUE - PRBS pattern generation is enabled
*                                      GT_FALSE - PRBS pattern generation is disabled
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for Tri-speed ports of all DxCh devices
*       and XAUI/HGS ports of DxCh3 and above devices.
*       When operating, enable the PRBS checker before the generator.
*
*/
GT_STATUS cpssDxChDiagPrbsPortGenerateEnableSet
(
    IN   GT_U8          devNum,
    IN   GT_PHYSICAL_PORT_NUM  portNum,
    IN   GT_U32         laneNum,
    IN   GT_BOOL        enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagPrbsPortGenerateEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, enable));

    rc = internal_cpssDxChDiagPrbsPortGenerateEnableSet(devNum, portNum, laneNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChDiagPrbsPortGenerateEnableGet function
* @endinternal
*
* @brief   Get the status of PRBS (Pseudo Random Bit Generator) pattern generation
*         per port and per lane.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for XAUI/HGS port
*
* @param[out] enablePtr                - GT_TRUE - PRBS pattern generation is enabled
*                                      GT_FALSE - PRBS pattern generation is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for Tri-speed ports of all DxCh devices
*       and XAUI/HGS ports of DxCh3 and above devices.
*
*/
static GT_STATUS internal_cpssDxChDiagPrbsPortGenerateEnableGet
(
    IN   GT_U8          devNum,
    IN   GT_PHYSICAL_PORT_NUM  portNum,
    IN   GT_U32         laneNum,
    OUT  GT_BOOL        *enablePtr
)
{
    GT_U32      regAddr; /* register address */
    GT_U32      value;   /* register field value */
    GT_U32      offset;  /* field offset */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    PRV_CPSS_PORT_TYPE_ENT portMacType;
    GT_U32 portMacNum; /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);



    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);
    portMacType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);

    if(portMacType < PRV_CPSS_PORT_XG_E)
    {
        /* FE ports not support PRBS */
        if(portMacType == PRV_CPSS_PORT_FE_E)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        PRV_CPSS_DXCH_PORT_MAC_CTRL2_REG_MAC(devNum,portMacNum,portMacType,&regAddr);
        offset = 11;
    }
    else
    {
        if (PRV_CPSS_DXCH3_FAMILY_CHECK_MAC(devNum) == 0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }

        if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }

        if (laneNum > PRV_DXCH_DIAG_MAX_XPCS_LANES_NUM_MAC(devNum))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            macRegs.perPortRegs[portMacNum].laneConfig0[laneNum];
        offset = 12;
    }
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    if (prvCpssDrvHwPpPortGroupGetRegField(devNum,portGroupId, regAddr, offset, 1, &value)!= GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);

    *enablePtr = (value == 1) ? GT_TRUE : GT_FALSE;
    return GT_OK;
}

/**
* @internal cpssDxChDiagPrbsPortGenerateEnableGet function
* @endinternal
*
* @brief   Get the status of PRBS (Pseudo Random Bit Generator) pattern generation
*         per port and per lane.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for XAUI/HGS port
*
* @param[out] enablePtr                - GT_TRUE - PRBS pattern generation is enabled
*                                      GT_FALSE - PRBS pattern generation is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for Tri-speed ports of all DxCh devices
*       and XAUI/HGS ports of DxCh3 and above devices.
*
*/
GT_STATUS cpssDxChDiagPrbsPortGenerateEnableGet
(
    IN   GT_U8          devNum,
    IN   GT_PHYSICAL_PORT_NUM  portNum,
    IN   GT_U32         laneNum,
    OUT  GT_BOOL        *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagPrbsPortGenerateEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, enablePtr));

    rc = internal_cpssDxChDiagPrbsPortGenerateEnableGet(devNum, portNum, laneNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChDiagPrbsPortCheckEnableSet function
* @endinternal
*
* @brief   Enable/Disable PRBS (Pseudo Random Bit Generator) checker per port and
*         per lane.
*         When the checker is enabled, it seeks to lock onto the incoming bit
*         stream, and once this is achieved the PRBS checker starts counting the
*         number of bit errors. Tne number of errors can be retrieved by
*         cpssDxChDiagPrbsGigPortStatusGet API.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for XAUI/HGS port
* @param[in] enable                   - GT_TRUE - PRBS checker is enabled
*                                      GT_FALSE - PRBS checker is disabled
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for Tri-speed ports of all DxCh devices
*       and XAUI/HGS ports of DxCh3 and above devices.
*       When operating, enable the PRBS checker before the generator.
*
*/
static GT_STATUS internal_cpssDxChDiagPrbsPortCheckEnableSet
(
    IN   GT_U8          devNum,
    IN   GT_PHYSICAL_PORT_NUM portNum,
    IN   GT_U32         laneNum,
    IN   GT_BOOL        enable
)
{
    GT_U32      regAddr; /* register address */
    GT_U32      value;   /* register field value */
    GT_U32      offset;  /* field offset */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    PRV_CPSS_PORT_TYPE_ENT portMacType;
    GT_U32 portMacNum; /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);
    portMacType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);

    if(portMacType < PRV_CPSS_PORT_XG_E)
    {
        /* FE ports not support PRBS */
        if(portMacType == PRV_CPSS_PORT_FE_E)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        PRV_CPSS_DXCH_PORT_MAC_CTRL2_REG_MAC(devNum,portMacNum,portMacType,&regAddr);
        offset = 10;
    }
    else
    {
        if (PRV_CPSS_DXCH3_FAMILY_CHECK_MAC(devNum) == 0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }

        if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }

        if (laneNum > PRV_DXCH_DIAG_MAX_XPCS_LANES_NUM_MAC(devNum))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            macRegs.perPortRegs[portMacNum].laneConfig0[laneNum];
        offset = 11;
    }

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    value = (enable == GT_TRUE) ? 1 : 0;

    return prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId,regAddr, offset, 1, value);
}

/**
* @internal cpssDxChDiagPrbsPortCheckEnableSet function
* @endinternal
*
* @brief   Enable/Disable PRBS (Pseudo Random Bit Generator) checker per port and
*         per lane.
*         When the checker is enabled, it seeks to lock onto the incoming bit
*         stream, and once this is achieved the PRBS checker starts counting the
*         number of bit errors. Tne number of errors can be retrieved by
*         cpssDxChDiagPrbsGigPortStatusGet API.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for XAUI/HGS port
* @param[in] enable                   - GT_TRUE - PRBS checker is enabled
*                                      GT_FALSE - PRBS checker is disabled
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for Tri-speed ports of all DxCh devices
*       and XAUI/HGS ports of DxCh3 and above devices.
*       When operating, enable the PRBS checker before the generator.
*
*/
GT_STATUS cpssDxChDiagPrbsPortCheckEnableSet
(
    IN   GT_U8          devNum,
    IN   GT_PHYSICAL_PORT_NUM portNum,
    IN   GT_U32         laneNum,
    IN   GT_BOOL        enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagPrbsPortCheckEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, enable));

    rc = internal_cpssDxChDiagPrbsPortCheckEnableSet(devNum, portNum, laneNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChDiagPrbsPortCheckEnableGet function
* @endinternal
*
* @brief   Get the status (enabled or disabled) of PRBS (Pseudo Random Bit Generator)
*         checker per port and per lane.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for XAUI/HGS port
*
* @param[out] enablePtr                - (pointer to) PRBS checker state.
*                                      GT_TRUE - PRBS checker is enabled
*                                      GT_FALSE - PRBS checker is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for Tri-speed ports of all DxCh devices
*       and XAUI/HGS ports of DxCh3 and above devices.
*
*/
static GT_STATUS internal_cpssDxChDiagPrbsPortCheckEnableGet
(
    IN   GT_U8          devNum,
    IN   GT_PHYSICAL_PORT_NUM  portNum,
    IN   GT_U32         laneNum,
    OUT  GT_BOOL        *enablePtr
)
{
    GT_U32      regAddr; /* register address */
    GT_U32      value;   /* register field value */
    GT_U32      offset;  /* field offset */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    PRV_CPSS_PORT_TYPE_ENT portMacType;
    GT_U32 portMacNum; /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);
    portMacType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);

    if(portMacType < PRV_CPSS_PORT_XG_E)
    {
        /* FE ports not support PRBS */
        if(portMacType == PRV_CPSS_PORT_FE_E)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        PRV_CPSS_DXCH_PORT_MAC_CTRL2_REG_MAC(devNum,portMacNum,portMacType,&regAddr);

        offset = 10;
    }
    else
    {
        if (PRV_CPSS_DXCH3_FAMILY_CHECK_MAC(devNum) == 0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
        if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }

        if (laneNum > PRV_DXCH_DIAG_MAX_XPCS_LANES_NUM_MAC(devNum))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            macRegs.perPortRegs[portMacNum].laneConfig0[laneNum];
        offset = 11;
    }

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    if (prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId,regAddr, offset, 1, &value)!= GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);

    *enablePtr = (value == 1) ? GT_TRUE : GT_FALSE;
    return GT_OK;
}

/**
* @internal cpssDxChDiagPrbsPortCheckEnableGet function
* @endinternal
*
* @brief   Get the status (enabled or disabled) of PRBS (Pseudo Random Bit Generator)
*         checker per port and per lane.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for XAUI/HGS port
*
* @param[out] enablePtr                - (pointer to) PRBS checker state.
*                                      GT_TRUE - PRBS checker is enabled
*                                      GT_FALSE - PRBS checker is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for Tri-speed ports of all DxCh devices
*       and XAUI/HGS ports of DxCh3 and above devices.
*
*/
GT_STATUS cpssDxChDiagPrbsPortCheckEnableGet
(
    IN   GT_U8          devNum,
    IN   GT_PHYSICAL_PORT_NUM  portNum,
    IN   GT_U32         laneNum,
    OUT  GT_BOOL        *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagPrbsPortCheckEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, enablePtr));

    rc = internal_cpssDxChDiagPrbsPortCheckEnableGet(devNum, portNum, laneNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChDiagPrbsPortCheckReadyGet function
* @endinternal
*
* @brief   Get the PRBS checker ready status.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] isReadyPtr               - (pointer to) PRBS checker state.
*                                      GT_TRUE - PRBS checker is ready.
*                                      PRBS checker has completed the initialization phase.
*                                      GT_FALSE - PRBS checker is not ready.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for the Tri-speed ports.
*       The Check ready status indicates that the PRBS checker has completed
*       the initialization phase. The PRBS generator at the transmit side may
*       be enabled.
*
*/
static GT_STATUS internal_cpssDxChDiagPrbsPortCheckReadyGet
(
    IN   GT_U8          devNum,
    IN   GT_PHYSICAL_PORT_NUM  portNum,
    OUT  GT_BOOL        *isReadyPtr
)
{
    GT_U32      regAddr; /* register address */
    GT_U32      value;   /* register field value */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32 portMacNum; /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(isReadyPtr);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

    if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum) >= PRV_CPSS_PORT_XG_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    /* FE ports not support PRBS */
    if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum) == PRV_CPSS_PORT_FE_E)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
        macRegs.perPortRegs[portMacNum].prbsCheckStatus;

    if (prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId,regAddr, 1, 1, &value)!= GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);

    *isReadyPtr = (value == 1) ? GT_TRUE : GT_FALSE;
    return GT_OK;
}

/**
* @internal cpssDxChDiagPrbsPortCheckReadyGet function
* @endinternal
*
* @brief   Get the PRBS checker ready status.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] isReadyPtr               - (pointer to) PRBS checker state.
*                                      GT_TRUE - PRBS checker is ready.
*                                      PRBS checker has completed the initialization phase.
*                                      GT_FALSE - PRBS checker is not ready.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for the Tri-speed ports.
*       The Check ready status indicates that the PRBS checker has completed
*       the initialization phase. The PRBS generator at the transmit side may
*       be enabled.
*
*/
GT_STATUS cpssDxChDiagPrbsPortCheckReadyGet
(
    IN   GT_U8          devNum,
    IN   GT_PHYSICAL_PORT_NUM  portNum,
    OUT  GT_BOOL        *isReadyPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagPrbsPortCheckReadyGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, isReadyPtr));

    rc = internal_cpssDxChDiagPrbsPortCheckReadyGet(devNum, portNum, isReadyPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, isReadyPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChDiagPrbsPortStatusGet function
* @endinternal
*
* @brief   Get PRBS (Pseudo Random Bit Generator) Error Counter and Checker Locked
*         status per port and per lane.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for XAUI/HGS port
*
* @param[out] checkerLockedPtr         - (pointer to) checker locked state.
*                                      GT_TRUE - checker is locked on the sequence stream.
*                                      GT_FALSE - checker isn't locked on the sequence
*                                      stream.
* @param[out] errorCntrPtr             - (pointer to) PRBS Error counter. This counter represents
*                                      the number of bit mismatches detected since
*                                      the PRBS checker of the port has locked.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note PRBS Error counter is cleared on read.
*       Applicable only for Tri-speed ports of all DxCh devices
*       and XAUI/HGS ports of DxCh3 and above devices.
*
*/
static GT_STATUS internal_cpssDxChDiagPrbsPortStatusGet
(
    IN   GT_U8          devNum,
    IN   GT_PHYSICAL_PORT_NUM  portNum,
    IN   GT_U32         laneNum,
    OUT  GT_BOOL        *checkerLockedPtr,
    OUT  GT_U32         *errorCntrPtr
)
{
    GT_U32      statusRegAddr;  /* register address for locked status*/
    GT_U32      counterRegAddr; /* register address for error counter*/
    GT_U32      value;          /* register field value */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32 portMacNum; /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(checkerLockedPtr);
    CPSS_NULL_PTR_CHECK_MAC(errorCntrPtr);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

    if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum) < PRV_CPSS_PORT_XG_E)
    {
        /* FE ports not support PRBS */
        if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum) == PRV_CPSS_PORT_FE_E)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        statusRegAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            macRegs.perPortRegs[portMacNum].prbsCheckStatus;

        counterRegAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            macRegs.perPortRegs[portMacNum].prbsErrorCounter;

    }
    else
    {
        if (PRV_CPSS_DXCH3_FAMILY_CHECK_MAC(devNum) == 0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }

        if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }

        if (laneNum > PRV_DXCH_DIAG_MAX_XPCS_LANES_NUM_MAC(devNum))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        statusRegAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            macRegs.perPortRegs[portMacNum].laneStatus[laneNum];

        counterRegAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            macRegs.perPortRegs[portMacNum].prbsErrorCounterLane[laneNum];
    }

    if((PRV_CPSS_SW_PTR_ENTRY_UNUSED == statusRegAddr) ||
            (PRV_CPSS_SW_PTR_ENTRY_UNUSED == counterRegAddr))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    /* get lock status */
    if (prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId,statusRegAddr, 0, 1, &value)!= GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);

    *checkerLockedPtr = (value == 1) ? GT_TRUE : GT_FALSE;

    /* get error counter */
    if (prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId,counterRegAddr, 0, 16, errorCntrPtr)!= GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);

    return GT_OK;
}

/**
* @internal cpssDxChDiagPrbsPortStatusGet function
* @endinternal
*
* @brief   Get PRBS (Pseudo Random Bit Generator) Error Counter and Checker Locked
*         status per port and per lane.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for XAUI/HGS port
*
* @param[out] checkerLockedPtr         - (pointer to) checker locked state.
*                                      GT_TRUE - checker is locked on the sequence stream.
*                                      GT_FALSE - checker isn't locked on the sequence
*                                      stream.
* @param[out] errorCntrPtr             - (pointer to) PRBS Error counter. This counter represents
*                                      the number of bit mismatches detected since
*                                      the PRBS checker of the port has locked.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note PRBS Error counter is cleared on read.
*       Applicable only for Tri-speed ports of all DxCh devices
*       and XAUI/HGS ports of DxCh3 and above devices.
*
*/
GT_STATUS cpssDxChDiagPrbsPortStatusGet
(
    IN   GT_U8          devNum,
    IN   GT_PHYSICAL_PORT_NUM  portNum,
    IN   GT_U32         laneNum,
    OUT  GT_BOOL        *checkerLockedPtr,
    OUT  GT_U32         *errorCntrPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagPrbsPortStatusGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, checkerLockedPtr, errorCntrPtr));

    rc = internal_cpssDxChDiagPrbsPortStatusGet(devNum, portNum, laneNum, checkerLockedPtr, errorCntrPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, checkerLockedPtr, errorCntrPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChDiagPrbsCyclicDataSet function
* @endinternal
*
* @brief   Set cylic data for transmition. See cpssDxChDiagPrbsPortTransmitModeSet.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number
* @param[in] cyclicDataArr[4]         - cyclic data array
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_SUPPORTED         - request is not supported for this port type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for the XAUI/HGS ports.
*
*/
static GT_STATUS internal_cpssDxChDiagPrbsCyclicDataSet
(
    IN   GT_U8          devNum,
    IN   GT_PHYSICAL_PORT_NUM  portNum,
    IN   GT_U32         laneNum,
    IN   GT_U32         cyclicDataArr[4]
)
{
    GT_U32      regAddr;  /* register address */
    GT_U32      i;
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32 portMacNum; /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(cyclicDataArr);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

    if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum) < PRV_CPSS_PORT_XG_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    if (laneNum > PRV_DXCH_DIAG_MAX_XPCS_LANES_NUM_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
        macRegs.perPortRegs[portMacNum].cyclicData[laneNum];
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    for (i =0; i < 4;i++)
    {
        if (prvCpssDrvHwPpPortGroupWriteRegister(devNum,portGroupId,regAddr,cyclicDataArr[i]) != GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
        regAddr += 4;
    }

    return GT_OK;
}

/**
* @internal cpssDxChDiagPrbsCyclicDataSet function
* @endinternal
*
* @brief   Set cylic data for transmition. See cpssDxChDiagPrbsPortTransmitModeSet.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number
* @param[in] cyclicDataArr[4]         - cyclic data array
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_SUPPORTED         - request is not supported for this port type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for the XAUI/HGS ports.
*
*/
GT_STATUS cpssDxChDiagPrbsCyclicDataSet
(
    IN   GT_U8          devNum,
    IN   GT_PHYSICAL_PORT_NUM  portNum,
    IN   GT_U32         laneNum,
    IN   GT_U32         cyclicDataArr[4]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagPrbsCyclicDataSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, cyclicDataArr));

    rc = internal_cpssDxChDiagPrbsCyclicDataSet(devNum, portNum, laneNum, cyclicDataArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, cyclicDataArr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChDiagPrbsCyclicDataGet function
* @endinternal
*
* @brief   Get cylic data for transmition. See cpssDxChDiagPrbsPortTransmitModeSet.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number
*
* @param[out] cyclicDataArr[4]         - cyclic data array
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - request is not supported for this port type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for the XAUI/HGS ports.
*
*/
static GT_STATUS internal_cpssDxChDiagPrbsCyclicDataGet
(
    IN   GT_U8          devNum,
    IN   GT_PHYSICAL_PORT_NUM  portNum,
    IN   GT_U32         laneNum,
    OUT  GT_U32         cyclicDataArr[4]
)
{
    GT_U32      regAddr;  /* register address */
    GT_U32      i;
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32 portMacNum; /* MAC number */


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(cyclicDataArr);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

    if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum) < PRV_CPSS_PORT_XG_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    if (laneNum > PRV_DXCH_DIAG_MAX_XPCS_LANES_NUM_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
        macRegs.perPortRegs[portMacNum].cyclicData[laneNum];
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    for (i =0; i < 4;i++)
    {
        if (prvCpssDrvHwPpPortGroupReadRegister(devNum,portGroupId,regAddr,&cyclicDataArr[i]) != GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
        regAddr += 4;
    }

    return GT_OK;
}

/**
* @internal cpssDxChDiagPrbsCyclicDataGet function
* @endinternal
*
* @brief   Get cylic data for transmition. See cpssDxChDiagPrbsPortTransmitModeSet.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number
*
* @param[out] cyclicDataArr[4]         - cyclic data array
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - request is not supported for this port type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable only for the XAUI/HGS ports.
*
*/
GT_STATUS cpssDxChDiagPrbsCyclicDataGet
(
    IN   GT_U8          devNum,
    IN   GT_PHYSICAL_PORT_NUM  portNum,
    IN   GT_U32         laneNum,
    OUT  GT_U32         cyclicDataArr[4]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagPrbsCyclicDataGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, cyclicDataArr));

    rc = internal_cpssDxChDiagPrbsCyclicDataGet(devNum, portNum, laneNum, cyclicDataArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, cyclicDataArr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChDiagPrbsSerdesIndexGet function
* @endinternal
*
* @brief   Get SERDES index for lane of a port.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for flexLink ports (APPLICABLE RANGES: 0..3)
*
* @param[out] portGroupIdPtr           - (pointer to) the port group Id
*                                      - support multi-port-groups device
* @param[out] serdesIndexPtr           - (pointer to) SERDES index
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number, device or lane number
* @retval GT_NOT_SUPPORTED         - request is not supported for this port type
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. Supported only for GE and FlexLink ports.
*
*/
static GT_STATUS prvCpssDxChDiagPrbsSerdesIndexGet
(
    IN   GT_U8          devNum,
    IN   GT_PHYSICAL_PORT_NUM portNum,
    IN   GT_U32         laneNum,
    OUT  GT_U32         *portGroupIdPtr,
    OUT  GT_U32         *serdesIndexPtr
)
{
    GT_STATUS rc = GT_OK;    /* function return value */
    GT_U32 firstSerdes;      /* index of first SERDES for port */
    GT_U32 numOfSerdesLanes; /* number of SERDES lanes for port */
    GT_U32 portMacNum;      /* MAC number */

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    *portGroupIdPtr = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);

    if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum) == PRV_CPSS_PORT_FE_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChPortNumberOfSerdesLanesGet(devNum, portNum,
                                               &firstSerdes, &numOfSerdesLanes);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E)
    {
        /* xCat2 has only one SERDES for a port */
        *serdesIndexPtr = firstSerdes;
    }
    else if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E)
            || (PRV_CPSS_SIP_5_CHECK_MAC(devNum))
            || (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E))
    {
        if (PRV_CPSS_DXCH_IS_FLEX_LINK_MAC(devNum,portMacNum) == GT_TRUE)
        {
            if (laneNum >= numOfSerdesLanes)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            /* stack/flex ports have up to 4 SERDESes per port */
            *serdesIndexPtr = firstSerdes + laneNum;
        }
        else
        {
            /* each network port SERDES is used for 4 ports */
            *serdesIndexPtr = firstSerdes;
        }
    }
    else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)
    {
        if (laneNum >= numOfSerdesLanes)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        *serdesIndexPtr = firstSerdes + laneNum;
    }
    else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
    {/* in Lion2 serdes #3 could be replaced by serdes #4 i.e.
        lane #numOfSerdesLanes can replace lane #numOfSerdesLanes-1 */
        if (laneNum > numOfSerdesLanes)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        *serdesIndexPtr = firstSerdes + laneNum;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    if((PRV_CPSS_PP_MAC(devNum)->devFamily < CPSS_PP_FAMILY_DXCH_LION2_E) &&
            (!PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
    {
        /* check if SERDES is initialized and make proper initialization
           if it is needed */
        rc = prvCpssDxChLpCheckAndInitNeededSerdes(devNum, *portGroupIdPtr,
                                                   *serdesIndexPtr, 1);
    }

    return rc;
}


/**
* @internal internal_cpssDxChDiagPrbsSerdesTestEnableSet function
* @endinternal
*
* @brief   Enable/Disable SERDES PRBS (Pseudo Random Bit Generator) test mode.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for flexLink ports (APPLICABLE RANGES: 0..3)
* @param[in] enable                   - GT_TRUE - test enabled
*                                      GT_FALSE - test disabled
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number, device or lane number
* @retval GT_NOT_SUPPORTED         - request is not supported for this port type
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1.Transmit mode should be set before enabling test mode.
*       See test cpssDxChDiagPrbsSerdesTransmitModeSet.
*       2. Supported only for GE and FlexLink ports.
*
*/
static GT_STATUS internal_cpssDxChDiagPrbsSerdesTestEnableSet
(
    IN   GT_U8          devNum,
    IN   GT_PHYSICAL_PORT_NUM  portNum,
    IN   GT_U32         laneNum,
    IN   GT_BOOL        enable
)
{
    GT_U32 regAddr;    /* register address */
    GT_U32 serdesIndex;/* SERDES index */
    GT_U32 value;      /* register field value */
    GT_STATUS rc;      /* function return value */
    GT_U32 portGroupId;/*the port group Id - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    /* check input parameters and get SERDES index */
    rc = prvCpssDxChDiagPrbsSerdesIndexGet(devNum, portNum, laneNum,
                                           &portGroupId, &serdesIndex);
    if (rc != GT_OK)
    {
        return rc;
    }

    if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
        || (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
        || (PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
    {
        MV_HWS_SERDES_TX_PATTERN  txPattern; /* PRBS sequence type */
        MV_HWS_SERDES_TEST_GEN_MODE hwsMode; /* enable/disable PRBS RX/TX test */

        rc = mvHwsSerdesTestGenGet(devNum, portGroupId, serdesIndex,
                                   HWS_DEV_SERDES_TYPE(devNum), &txPattern,
                                   &hwsMode);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* for BobK/BC3 PRBS mode is stored in SW port database */
        if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
        {
            switch(PRV_CPSS_DXCH_PP_MAC(devNum)->port.prbsMode[portNum])
            {
                case CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS7_E:
                    txPattern = PRBS7;
                    break;
                case CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS9_E:
                    txPattern = PRBS9;
                    break;
                case CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS15_E:
                    txPattern = PRBS15;
                    break;
                case CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS23_E:
                    txPattern = PRBS23;
                    break;
                case CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS31_E:
                    txPattern = PRBS31;
                    break;
                case CPSS_DXCH_DIAG_TRANSMIT_MODE_1T_E:
                    txPattern = _1T;
                    break;
                case CPSS_DXCH_DIAG_TRANSMIT_MODE_2T_E:
                    txPattern = _2T;
                    break;
                case CPSS_DXCH_DIAG_TRANSMIT_MODE_5T_E:
                    txPattern = _5T;
                    break;
                case CPSS_DXCH_DIAG_TRANSMIT_MODE_10T_E:
                    txPattern = _10T;
                    break;
                case CPSS_DXCH_DIAG_TRANSMIT_MODE_DFETraining:
                    txPattern = DFETraining;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }

        hwsMode = (GT_TRUE == enable) ? SERDES_TEST : SERDES_NORMAL;
        rc = mvHwsSerdesTestGen(devNum, portGroupId, serdesIndex, txPattern,
                                HWS_DEV_SERDES_TYPE(devNum), hwsMode);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        value = BOOL2BIT_MAC(enable);
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdesIndex].phyTestReg0;
        return prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId,regAddr, 15,1,value);
    }

    return GT_OK;
}

/**
* @internal cpssDxChDiagPrbsSerdesTestEnableSet function
* @endinternal
*
* @brief   Enable/Disable SERDES PRBS (Pseudo Random Bit Generator) test mode.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for flexLink ports (APPLICABLE RANGES: 0..3)
* @param[in] enable                   - GT_TRUE - test enabled
*                                      GT_FALSE - test disabled
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number, device or lane number
* @retval GT_NOT_SUPPORTED         - request is not supported for this port type
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1.Transmit mode should be set before enabling test mode.
*       See test cpssDxChDiagPrbsSerdesTransmitModeSet.
*       2. Supported only for GE and FlexLink ports.
*
*/
GT_STATUS cpssDxChDiagPrbsSerdesTestEnableSet
(
    IN   GT_U8          devNum,
    IN   GT_PHYSICAL_PORT_NUM  portNum,
    IN   GT_U32         laneNum,
    IN   GT_BOOL        enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagPrbsSerdesTestEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, enable));

    rc = internal_cpssDxChDiagPrbsSerdesTestEnableSet(devNum, portNum, laneNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChDiagPrbsSerdesTestEnableGet function
* @endinternal
*
* @brief   Get the status of PRBS (Pseudo Random Bit Generator) test mode.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for flexLink ports (APPLICABLE RANGES: 0..3)
*
* @param[out] enablePtr                - GT_TRUE - test enabled
*                                      GT_FALSE - test disabled
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number, device or lane number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - request is not supported for this port type
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Supported only for GE and FlexLink ports.
*
*/
static GT_STATUS internal_cpssDxChDiagPrbsSerdesTestEnableGet
(
    IN   GT_U8          devNum,
    IN   GT_PHYSICAL_PORT_NUM  portNum,
    IN   GT_U32         laneNum,
    OUT  GT_BOOL       *enablePtr
)
{
    GT_U32 regAddr;    /* register address */
    GT_U32 serdesIndex;/* SERDES index */
    GT_U32 value;      /* register field value */
    GT_STATUS rc;      /* function return value */
    GT_U32 portGroupId;/*the port group Id - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* check input parameters and get SERDES index */
    rc = prvCpssDxChDiagPrbsSerdesIndexGet(devNum, portNum, laneNum,
                                           &portGroupId, &serdesIndex);
    if (rc != GT_OK)
    {
        return rc;
    }

    if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
        || (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
        || (PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
    {
        MV_HWS_SERDES_TX_PATTERN  txPattern; /* PRBS sequence type */
        MV_HWS_SERDES_TEST_GEN_MODE hwsMode; /* enable/disable PRBS RX/TX test */

        rc = mvHwsSerdesTestGenGet(devNum, portGroupId, serdesIndex,
                                   HWS_DEV_SERDES_TYPE(devNum), &txPattern,
                                   &hwsMode);
        if (rc != GT_OK)
        {
            return rc;
        }

        *enablePtr = (SERDES_TEST == hwsMode);
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdesIndex].phyTestReg0;
        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId,regAddr, 15,1,&value);
        if (rc != GT_OK)
        {
            return rc;
        }
        *enablePtr = BIT2BOOL_MAC(value);
    }

    return GT_OK;
}

/**
* @internal cpssDxChDiagPrbsSerdesTestEnableGet function
* @endinternal
*
* @brief   Get the status of PRBS (Pseudo Random Bit Generator) test mode.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for flexLink ports (APPLICABLE RANGES: 0..3)
*
* @param[out] enablePtr                - GT_TRUE - test enabled
*                                      GT_FALSE - test disabled
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number, device or lane number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - request is not supported for this port type
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Supported only for GE and FlexLink ports.
*
*/
GT_STATUS cpssDxChDiagPrbsSerdesTestEnableGet
(
    IN   GT_U8          devNum,
    IN   GT_PHYSICAL_PORT_NUM  portNum,
    IN   GT_U32         laneNum,
    OUT  GT_BOOL       *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagPrbsSerdesTestEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, enablePtr));

    rc = internal_cpssDxChDiagPrbsSerdesTestEnableGet(devNum, portNum, laneNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChDiagPrbsSerdesTransmitModeSet function
* @endinternal
*
* @brief   Set transmit mode for SERDES PRBS on specified port/lane.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for flexLink ports (APPLICABLE RANGES: 0..3)
* @param[in] mode                     - transmit mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device or lane number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - request is not supported for this port type
*                                       or unsupported transmit mode.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChDiagPrbsSerdesTransmitModeSet
(
    IN   GT_U8                            devNum,
    IN   GT_PHYSICAL_PORT_NUM             portNum,
    IN   GT_U32                           laneNum,
    IN   CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT mode
)
{
    GT_U32 regAddr;    /* register address */
    GT_U32 serdesIndex;/* SERDES index */
    GT_STATUS rc;      /* function return value */
    GT_U32 portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32  prbsCode;      /* value to write to register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    /* check input parameters and get SERDES index */
    rc = prvCpssDxChDiagPrbsSerdesIndexGet(devNum, portNum, laneNum,
                                           &portGroupId, &serdesIndex);
    if (rc != GT_OK)
    {
        return rc;
    }

    if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
        || (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
        || (PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
    {
        MV_HWS_SERDES_TX_PATTERN  txPattern; /* PRBS sequence type */
        MV_HWS_SERDES_TEST_GEN_MODE hwsMode; /* enable/disable PRBS RX/TX test */

        rc = mvHwsSerdesTestGenGet(devNum, portGroupId, serdesIndex,
                                   HWS_DEV_SERDES_TYPE(devNum), &txPattern,
                                   &hwsMode);
        if (rc != GT_OK)
        {
            return rc;
        }

        switch(mode)
        {
            case CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS7_E:
                txPattern = PRBS7;
                break;
            case CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS9_E:
                txPattern = PRBS9;
                break;
            case CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS15_E:
                txPattern = PRBS15;
                break;
            case CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS23_E:
                txPattern = PRBS23;
                break;
            case CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS31_E:
                txPattern = PRBS31;
                break;
            case CPSS_DXCH_DIAG_TRANSMIT_MODE_1T_E:
                txPattern = _1T;
                break;
            case CPSS_DXCH_DIAG_TRANSMIT_MODE_2T_E:
                txPattern = _2T;
                break;
            case CPSS_DXCH_DIAG_TRANSMIT_MODE_5T_E:
                txPattern = _5T;
                break;
            case CPSS_DXCH_DIAG_TRANSMIT_MODE_10T_E:
                txPattern = _10T;
                break;
            case CPSS_DXCH_DIAG_TRANSMIT_MODE_DFETraining:
                txPattern = DFETraining;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* store current PRBS mode for BobK/BC3 SERDES in port DB */
        if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
        {
            PRV_CPSS_DXCH_PP_MAC(devNum)->port.prbsMode[portNum] = mode;
        }
        else
        {
            rc = mvHwsSerdesTestGen(devNum, portGroupId, serdesIndex, txPattern,
                                    HWS_DEV_SERDES_TYPE(devNum), hwsMode);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdesIndex].phyTestReg0;
        rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId,regAddr, 0,4,0xE);
        if (rc != GT_OK)
        {
            return rc;
        }

        switch (mode)
        {
            case CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS7_E:
                prbsCode = 0x80;
                break;
            case CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS15_E:
                prbsCode = 0x82;
                break;
            case CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS23_E:
                prbsCode = 0x83;
                break;
            case CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS31_E:
                prbsCode = 0x84;
                break;
            default :
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdesIndex].phyTestDataReg5;
        return prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId,regAddr, 0,16,prbsCode);
    }

    return GT_OK;
}

/**
* @internal cpssDxChDiagPrbsSerdesTransmitModeSet function
* @endinternal
*
* @brief   Set transmit mode for SERDES PRBS on specified port/lane.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for flexLink ports (APPLICABLE RANGES: 0..3)
* @param[in] mode                     - transmit mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device or lane number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - request is not supported for this port type
*                                       or unsupported transmit mode.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChDiagPrbsSerdesTransmitModeSet
(
    IN   GT_U8                            devNum,
    IN   GT_PHYSICAL_PORT_NUM             portNum,
    IN   GT_U32                           laneNum,
    IN   CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT mode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagPrbsSerdesTransmitModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, mode));

    rc = internal_cpssDxChDiagPrbsSerdesTransmitModeSet(devNum, portNum, laneNum, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChDiagPrbsSerdesTransmitModeGet function
* @endinternal
*
* @brief   Get transmit mode for SERDES PRBS on specified port/lane.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for flexLink ports (APPLICABLE RANGES: 0..3)
*
* @param[out] modePtr                  - transmit mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device or lane number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on unkonown transmit mode
* @retval GT_NOT_SUPPORTED         - request is not supported for this port type
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChDiagPrbsSerdesTransmitModeGet
(
    IN   GT_U8                             devNum,
    IN   GT_PHYSICAL_PORT_NUM              portNum,
    IN   GT_U32                            laneNum,
    OUT  CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT *modePtr
)
{
    GT_U32 regAddr;    /* register address */
    GT_U32 serdesIndex;/* SERDES index */
    GT_STATUS rc;      /* function return value */
    GT_U32 value;      /* register value */
    GT_U32 portGroupId;/*the port group Id - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    CPSS_NULL_PTR_CHECK_MAC(modePtr);

    /* check input parameters and get SERDES index */
    rc = prvCpssDxChDiagPrbsSerdesIndexGet(devNum, portNum, laneNum,
                                           &portGroupId, &serdesIndex);
    if (rc != GT_OK)
    {
        return rc;
    }

    if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
        || (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
        || (PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
    {
        if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
        {
            *modePtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.prbsMode[portNum];
        }
        else
        {
            MV_HWS_SERDES_TX_PATTERN  txPattern; /* PRBS sequence type */
            MV_HWS_SERDES_TEST_GEN_MODE hwsMode; /* enable/disable PRBS RX/TX test */

            rc = mvHwsSerdesTestGenGet(devNum, portGroupId, serdesIndex,
                                       HWS_DEV_SERDES_TYPE(devNum), &txPattern,
                                       &hwsMode);
            if (rc != GT_OK)
            {
                return rc;
            }

            switch(txPattern)
            {
                case PRBS7:
                    *modePtr = CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS7_E;
                    break;
                case PRBS9:
                    *modePtr = CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS9_E;
                    break;
                case PRBS15:
                    *modePtr = CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS15_E;
                    break;
                case PRBS23:
                    *modePtr = CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS23_E;
                    break;
                case PRBS31:
                    *modePtr = CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS31_E;
                    break;
                case _1T:
                    *modePtr = CPSS_DXCH_DIAG_TRANSMIT_MODE_1T_E;
                    break;
                case _2T:
                    *modePtr = CPSS_DXCH_DIAG_TRANSMIT_MODE_2T_E;
                    break;
                case _5T:
                    *modePtr = CPSS_DXCH_DIAG_TRANSMIT_MODE_5T_E;
                    break;
                case _10T:
                    *modePtr = CPSS_DXCH_DIAG_TRANSMIT_MODE_10T_E;
                    break;
                case DFETraining:
                    *modePtr = CPSS_DXCH_DIAG_TRANSMIT_MODE_DFETraining;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdesIndex].phyTestReg0;
        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId,regAddr, 0,4,&value);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (value != 0xE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdesIndex].phyTestDataReg5;
        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId,regAddr, 0,16,&value);
        if (rc != GT_OK)
        {
            return rc;
        }

        switch (value)
        {
            case 0x80:
                *modePtr = CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS7_E;
                break;
            case 0x82:
                *modePtr = CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS15_E;
                break;
            case 0x83:
                *modePtr = CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS23_E;
                break;
            case 0x84:
                *modePtr = CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS31_E;
                break;
            default :
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChDiagPrbsSerdesTransmitModeGet function
* @endinternal
*
* @brief   Get transmit mode for SERDES PRBS on specified port/lane.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for flexLink ports (APPLICABLE RANGES: 0..3)
*
* @param[out] modePtr                  - transmit mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device or lane number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on unkonown transmit mode
* @retval GT_NOT_SUPPORTED         - request is not supported for this port type
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChDiagPrbsSerdesTransmitModeGet
(
    IN   GT_U8                             devNum,
    IN   GT_PHYSICAL_PORT_NUM              portNum,
    IN   GT_U32                            laneNum,
    OUT  CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT *modePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagPrbsSerdesTransmitModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, modePtr));

    rc = internal_cpssDxChDiagPrbsSerdesTransmitModeGet(devNum, portNum, laneNum, modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChDiagPrbsSerdesStatusGet function
* @endinternal
*
* @brief   Get SERDES PRBS (Pseudo Random Bit Generator) pattern detector state,
*         error counter and pattern counter.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for flexLink ports (APPLICABLE RANGES: 0..3)
*
* @param[out] lockedPtr                - (pointer to) Pattern detector state.
*                                      GT_TRUE - Pattern detector had locked onto the pattern.
*                                      GT_FALSE - Pattern detector is not locked onto
*                                      the pattern.
* @param[out] errorCntrPtr             - (pointer to) PRBS Error counter. This counter represents
*                                      the number of bit mismatches detected since
*                                      the PRBS checker of the port has locked.
* @param[out] patternCntrPtr           - (pointer to) Pattern counter. Number of 40-bit patterns
*                                      received since acquiring pattern lock.
*                                      NOT APPLICABLE:Caelum,
*                                      Aldrin, AC3X, Bobcat3,
*                                      Aldrin2, Falcon.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number, device or lane number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - request is not supported for this port type
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Supported only for GE and FlexLink ports.
*
*/
static GT_STATUS internal_cpssDxChDiagPrbsSerdesStatusGet
(
    IN   GT_U8     devNum,
    IN   GT_PHYSICAL_PORT_NUM  portNum,
    IN   GT_U32    laneNum,
    OUT  GT_BOOL  *lockedPtr,
    OUT  GT_U32   *errorCntrPtr,
    OUT  GT_U64   *patternCntrPtr
)
{
    GT_U32 regAddr;    /* register address */
    GT_U32 serdesIndex;/* SERDES index */
    GT_U32 value;      /* register field value */
    GT_STATUS rc;      /* function return value */
    GT_U32 i;
    GT_U32 portGroupId;/*the port group Id - support multi-port-groups device */
    GT_BOOL enableTest;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    CPSS_NULL_PTR_CHECK_MAC(lockedPtr);
    CPSS_NULL_PTR_CHECK_MAC(errorCntrPtr);
    CPSS_NULL_PTR_CHECK_MAC(patternCntrPtr);

    rc = cpssDxChDiagPrbsSerdesTestEnableGet(devNum, portNum, laneNum, &enableTest);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (!enableTest)
    {
        *lockedPtr = GT_FALSE;
        *errorCntrPtr = 0;
        patternCntrPtr->l[0] = 0;
        patternCntrPtr->l[1] = 0;
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* check input parameters and get SERDES index */
    rc = prvCpssDxChDiagPrbsSerdesIndexGet(devNum, portNum, laneNum,
                                           &portGroupId, &serdesIndex);
    if (rc != GT_OK)
    {
        return rc;
    }

    if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
        || (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
        || (PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
    {
        MV_HWS_SERDES_TEST_GEN_STATUS status;

        GT_BOOL clearOnReadEnable;
        GT_BOOL counterAccunulateMode;
        rc = cpssDxChDiagPrbsSerdesCounterClearOnReadEnableGet(devNum, portNum, laneNum, &clearOnReadEnable);
        if (rc != GT_OK)
        {
            return rc;
        }
        if(clearOnReadEnable == GT_TRUE)
        {
            counterAccunulateMode = GT_FALSE;
        }
        else
        {
            counterAccunulateMode = GT_TRUE;
        }

        /* clean buffer to avoid random trash in LOG */
        cpssOsMemSet(&status, 0, sizeof(status));

        rc = mvHwsSerdesTestGenStatus(devNum, portGroupId, serdesIndex,
                                      HWS_DEV_SERDES_TYPE(devNum),
                                      0 /* txPattern not used */, counterAccunulateMode /* clear on read mode - GT_FALSE */, &status);
        if (rc != GT_OK)
        {
            return rc;
        }

        *lockedPtr = BIT2BOOL_MAC(status.lockStatus);
        *errorCntrPtr = status.errorsCntr;
        *patternCntrPtr = status.txFramesCntr;
    }
    else
    {
        /* get PHY Test Pattern Lock */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdesIndex].phyTestReg1;
        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId,regAddr, 14,1,&value);
        if (rc != GT_OK)
        {
            return rc;
        }
        *lockedPtr = BIT2BOOL_MAC(value);

        /* get PHY Test Error Count */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdesIndex].phyTestPrbsErrCntReg1;
        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId,regAddr, 0,16,&value);
        if (rc != GT_OK)
        {
            return rc;
        }
        *errorCntrPtr = value;
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdesIndex].phyTestPrbsErrCntReg0;
        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId,regAddr, 0,16,&value);
        if (rc != GT_OK)
        {
            return rc;
        }
        *errorCntrPtr += (value << 16);

        /* get PHY Test Pattern Count  */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdesIndex].phyTestPrbsCntReg2;
        patternCntrPtr->l[0] = 0;
        patternCntrPtr->l[1] = 0;
        for (i =0; i < 3; i++)
        {
            rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId,regAddr, 0,16,&value);
            if (rc != GT_OK)
            {
                return rc;
            }
            patternCntrPtr->l[i/2] = (value & 0xffff) << ((i&1)?16:0);
            regAddr -=4;
        }
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChDiagPrbsSerdesCounterClearOnReadEnableGet function
* @endinternal
*
* @brief   Get Prbs Counter Clear on read enable or disable status per port and lane
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number
*
* @param[out] enablePtr                - (pointer to) enable
*                                      GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChDiagPrbsSerdesCounterClearOnReadEnableGet
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_U32                laneNum,
    OUT GT_BOOL              *enablePtr
)
{
    GT_U32    portMacNum; /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* check input parameters */
    if(laneNum >= PRV_CPSS_MAX_PORT_LANES_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "laneNum[%d] >= max[%d]",
            laneNum,PRV_CPSS_MAX_PORT_LANES_CNS);
    }

    *enablePtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.prbsSerdesCountersClearOnReadEnable[portMacNum][laneNum];
    return GT_OK;
}

/**
* @internal internal_cpssDxChDiagPrbsSerdesCounterClearOnReadEnableSet function
* @endinternal
*
* @brief   Enable or disable Prbs Counter Clear on read status per port and lane
*         Can be run after port creation.
*         After port reconfiguration (cpssDxChPortModeSpeedSet) Should be run once again.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChDiagPrbsSerdesCounterClearOnReadEnableSet
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_U32                laneNum,
    IN  GT_BOOL               enable
)
{
    GT_U32    portMacNum; /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    /* check input parameters */
    if(laneNum >= PRV_CPSS_MAX_PORT_LANES_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "laneNum[%d] >= max[%d]",
            laneNum,PRV_CPSS_MAX_PORT_LANES_CNS);
    }

    PRV_CPSS_DXCH_PP_MAC(devNum)->port.prbsSerdesCountersClearOnReadEnable[portMacNum][laneNum] = enable;
    return GT_OK;
}

/**
* @internal cpssDxChDiagPrbsSerdesCounterClearOnReadEnableSet function
* @endinternal
*
* @brief   Enable or disable Prbs Counter Clear on read status per port and lane
*         Can be run after port creation.
*         After port reconfiguration (cpssDxChPortModeSpeedSet) Should be run once again.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChDiagPrbsSerdesCounterClearOnReadEnableSet
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_U32                laneNum,
    IN  GT_BOOL               enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagPrbsSerdesCounterClearOnReadEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, enable));

    rc = internal_cpssDxChDiagPrbsSerdesCounterClearOnReadEnableSet(devNum, portNum, laneNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal cpssDxChDiagPrbsSerdesCounterClearOnReadEnableGet function
* @endinternal
*
* @brief   Get Prbs Counter Clear on read enable or disable status per port and lane
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number
*
* @param[out] enablePtr                - (pointer to) enable
*                                      GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChDiagPrbsSerdesCounterClearOnReadEnableGet
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_U32                laneNum,
    OUT GT_BOOL              *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagPrbsSerdesCounterClearOnReadEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, enablePtr));

    rc = internal_cpssDxChDiagPrbsSerdesCounterClearOnReadEnableGet(devNum, portNum, laneNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal cpssDxChDiagPrbsSerdesStatusGet function
* @endinternal
*
* @brief   Get SERDES PRBS (Pseudo Random Bit Generator) pattern detector state,
*         error counter and pattern counter.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number, relevant only for flexLink ports (APPLICABLE RANGES: 0..3)
*
* @param[out] lockedPtr                - (pointer to) Pattern detector state.
*                                      GT_TRUE - Pattern detector had locked onto the pattern.
*                                      GT_FALSE - Pattern detector is not locked onto
*                                      the pattern.
* @param[out] errorCntrPtr             - (pointer to) PRBS Error counter. This counter represents
*                                      the number of bit mismatches detected since
*                                      the PRBS checker of the port has locked.
* @param[out] patternCntrPtr           - (pointer to) Pattern counter. Number of 40-bit patterns
*                                      received since acquiring pattern lock.
*                                      NOT APPLICABLE:Caelum,
*                                      Aldrin, AC3X, Bobcat3,
*                                      Aldrin2, Falcon.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number, device or lane number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - request is not supported for this port type
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Supported only for GE and FlexLink ports.
*
*/
GT_STATUS cpssDxChDiagPrbsSerdesStatusGet
(
    IN   GT_U8     devNum,
    IN   GT_PHYSICAL_PORT_NUM  portNum,
    IN   GT_U32    laneNum,
    OUT  GT_BOOL  *lockedPtr,
    OUT  GT_U32   *errorCntrPtr,
    OUT  GT_U64   *patternCntrPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagPrbsSerdesStatusGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, lockedPtr, errorCntrPtr, patternCntrPtr));

    rc = internal_cpssDxChDiagPrbsSerdesStatusGet(devNum, portNum, laneNum, lockedPtr, errorCntrPtr, patternCntrPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, lockedPtr, errorCntrPtr, patternCntrPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChDiagDfxXsbRegAddrGet function
* @endinternal
*
* @brief   This function returns address of DFX XSB register.
*         To access the relevant pipe should be already mapped.
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] clientNum                - DFX Client number
* @param[in] xsbNum                   - DFX XSB number
* @param[in] regOffset                - register offset in XSB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_TIMEOUT               - on BIST timeout
*/
GT_U32    prvCpssDxChDiagDfxXsbRegAddrGet
(
    IN  GT_U8                                        devNum,
    IN  GT_U32                                       clientNum,
    IN  GT_U32                                       xsbNum,
    IN  GT_U32                                       regOffset
)
{
    GT_U32    dfxRegBase, regAddr;

    regAddr =
        PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnits.DFXServerRegs.pipeSelect;;
    dfxRegBase = (regAddr & 0xFFF00000);
    /* address bit map:                */
    /* bits 31:20 - base               */
    /* bits 19:15 - clent number 0..27 */
    /* bit  14    - 1 means XSB select */
    /* bits 13:9  - XSB number         */
    /* bits 8:8   - XSB register       */
    return (dfxRegBase | (clientNum << 15) | (1 << 14) | (xsbNum << 9) | regOffset);
}

/**
* @internal prvCpssDxChDiagDfxTcamBistRestore function
* @endinternal
*
* @brief   This function restores access to memories after BIST specified TCAMs of device.
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] tcamsAmount              - amount of TCAMs
*                                      (APPLICABLE RANGES:1-32)
* @param[in] dfxPipeNumArr[]          - (pointer to) array of DFX Pipe numbers
* @param[in] dfxClientNumArr[]        - (pointer to) array of DFX Client numbers
* @param[in] xsbNumArr[]              - (pointer to) array of DFX XSB numbers
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS    prvCpssDxChDiagDfxTcamBistRestore
(
    IN  GT_U8                                        devNum,
    IN  GT_U32                                       tcamsAmount,
    IN  GT_U32                                       dfxPipeNumArr[],
    IN  GT_U32                                       dfxClientNumArr[],
    IN  GT_U32                                       xsbNumArr[]
)
{
    GT_STATUS rc;
    GT_U32    lastDfxPipeNum;
    GT_U32    tcamNum;
    GT_U32    regAddr;

    if (PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    if (tcamsAmount == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* wrong pipe num to cause first pipe mapping */
    lastDfxPipeNum = 0xFFFFFFFF;

    /* start all bists */
    for (tcamNum = 0; (tcamNum < tcamsAmount); tcamNum++)
    {
        if (lastDfxPipeNum != dfxPipeNumArr[tcamNum])
        {
            lastDfxPipeNum = dfxPipeNumArr[tcamNum];
            rc = prvCpssDfxMemoryPipeIdSet(
                devNum, lastDfxPipeNum);
            if (rc != GT_OK) return rc;
        }
        regAddr = prvCpssDxChDiagDfxXsbRegAddrGet(
            devNum, dfxClientNumArr[tcamNum], xsbNumArr[tcamNum], 0x50 /*regOffset*/);

        /* restore register to power on state */
        rc = prvCpssDrvHwPpResetAndInitControllerWriteReg(
            devNum, regAddr, 0x00000203);
        if (rc != GT_OK) return rc;

   }

    return GT_OK;
}

/**
* @internal prvCpssDxChDiagDfxTcamBistRun function
* @endinternal
*
* @brief   This function runs BIST specified TCAMs of device at the same time.
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] tcamsAmount              - amount of TCAMs
*                                      (APPLICABLE RANGES:1-32)
* @param[in] dfxPipeNumArr[]          - (pointer to) array of DFX Pipe numbers
* @param[in] dfxClientNumArr[]        - (pointer to) array of DFX Client numbers
* @param[in] xsbNumArr[]              - (pointer to) array of DFX XSB numbers
* @param[in] timeout                  - timeout in millisecons
*
* @param[out] testStatusPtr            - (pointer to)GT_TRUE - no errors, GT_FALSE - errors
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_TIMEOUT               - on BIST timeout
*/
GT_STATUS    prvCpssDxChDiagDfxTcamBistRun
(
    IN  GT_U8                                        devNum,
    IN  GT_U32                                       tcamsAmount,
    IN  GT_U32                                       dfxPipeNumArr[],
    IN  GT_U32                                       dfxClientNumArr[],
    IN  GT_U32                                       xsbNumArr[],
    IN  GT_U32                                       timeout,
    OUT GT_BOOL                                      *testStatusPtr
)
{
    GT_STATUS rc;
    GT_U32    lastDfxPipeNum;
    GT_U32    tcamNum;
    GT_U32    regAddr;
    GT_U32    count;
    GT_U32    data;
    GT_U32    polledMask;
    GT_U32    checkMask;
    GT_U32    checkPattern;
    GT_U32    allFinished;
    GT_U32    bistErrorsNum;

    CPSS_NULL_PTR_CHECK_MAC(testStatusPtr);
    if (PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    if (tcamsAmount == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (timeout == 0)
    {
        timeout = 1;
    }

    *testStatusPtr = GT_FALSE; /* default to be overriden */

    /* wrong pipe num to cause first pipe mapping */
    lastDfxPipeNum = 0xFFFFFFFF;

    /* start all bists */
    for (tcamNum = 0; (tcamNum < tcamsAmount); tcamNum++)
    {
        if (lastDfxPipeNum != dfxPipeNumArr[tcamNum])
        {
            lastDfxPipeNum = dfxPipeNumArr[tcamNum];
            rc = prvCpssDfxMemoryPipeIdSet(
                devNum, lastDfxPipeNum);
            if (rc != GT_OK) return rc;
        }
        regAddr = prvCpssDxChDiagDfxXsbRegAddrGet(
            devNum, dfxClientNumArr[tcamNum], xsbNumArr[tcamNum], 0x50 /*regOffset*/);

        /* start tcam_bist_config */
        rc = prvCpssDrvHwPpResetAndInitControllerWriteReg(
            devNum, regAddr, 0x00010001);
        if (rc != GT_OK) return rc;

        /* start_tcams_bist */
        rc = prvCpssDrvHwPpResetAndInitControllerWriteReg(
            devNum, regAddr, 0x00010005);
        if (rc != GT_OK) return rc;
   }

    /* wait and check all bists */
    polledMask    = 0x08;
    checkMask     = 0x70;
    checkPattern  = 0x00;
    allFinished   = 0; /* preventing compiler warning */
    bistErrorsNum = 0; /* preventing compiler warning */
    for (count = 0; (count <= timeout); count++)
    {
        allFinished   = 1;
        bistErrorsNum = 0;
        for (tcamNum = 0; (tcamNum < tcamsAmount); tcamNum++)
        {
            if (lastDfxPipeNum != dfxPipeNumArr[tcamNum])
            {
                lastDfxPipeNum = dfxPipeNumArr[tcamNum];
                rc = prvCpssDfxMemoryPipeIdSet(
                    devNum, lastDfxPipeNum);
                if (rc != GT_OK) return rc;
            }
            regAddr = prvCpssDxChDiagDfxXsbRegAddrGet(
                devNum, dfxClientNumArr[tcamNum], xsbNumArr[tcamNum], 0x50 /*regOffset*/);

            /* start tcam_bist_config */
            rc = prvCpssDrvHwPpResetAndInitControllerReadReg(
                devNum, regAddr, &data);
            if (rc != GT_OK) return rc;

            if ((data & polledMask) != polledMask)
            {
                allFinished = 0; /* BIST not finished */
                continue;
            }
            if ((data & checkMask) != checkPattern)
            {
                bistErrorsNum ++; /* BIST error */
                return GT_OK;
            }
       }
       if (allFinished != 0) break;
       cpssOsTimerWkAfter(1); /* 1 millisecond */
    }

    rc = prvCpssDxChDiagDfxTcamBistRestore(
         devNum, tcamsAmount, dfxPipeNumArr, dfxClientNumArr, xsbNumArr);
    if (rc != GT_OK) return rc;

    if (allFinished == 0)
    {
        *testStatusPtr = GT_FALSE; /* BIST error */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, LOG_ERROR_NO_MSG);
    }

    *testStatusPtr = ((bistErrorsNum == 0) ? GT_TRUE : GT_FALSE);
    return GT_OK;
}

/**
* @internal internal_cpssDxChDiagMemoryBistsRun function
* @endinternal
*
* @brief   Runs BIST (Built-in self-test) on specified memory.
*
* @note   APPLICABLE DEVICES:      DxCh3; Lion; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] memBistType              - type of memory
* @param[in] bistTestType             - BIST test type (pure memory test/compare memory test)
* @param[in] timeOut                  - maximal time in milisecond to wait for BIST finish.
* @param[in] clearMemoryAfterTest     - GT_TRUE  - Clear memory after test.
*                                      GT_FALSE - Don't clear memory after test.
* @param[in] testsToRunBmpPtr         - bitmap of required test related for
*                                      CPSS_DIAG_PP_MEM_BIST_COMPARE_TEST_E
*                                      (from 0 to CPSS_DIAG_PP_MEM_BIST_COMPARE_TESTS_NUM_CNS,
*                                      so minimum bitmap size 32 bits, maximum depends on future
*                                      number of tests); 1 - run test, 0 - don't run test.
*                                      Bit 0 set in bitmap means run test 0.0 from "Table 1: TCAM Compare BIST:
*                                      Parameters and Expected Result Configurations"
*                                      (see full tests list and definitions in Application Notes)
*
* @param[out] testsResultBmpPtr        - pointer to bitmap with results of compare memory test,
*                                      !!! pay attention: 0 - test pass, 1 - test failed;
*                                      if NULL, just final result of test will be returned;
*                                      regarding size see testsToRunBmpPtr above
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_UNFIXABLE_BIST_ERROR  - on unfixable problem in the TCAM
*
* @note 1. This function should not be called under traffic.
*       2. Bist destroys the content of the memory. If clearMemoryAfterTest set
*       function will clear the content of the memory after the test.
*       Application responsible to restore the content after the function
*       completion.
*
*/
static GT_STATUS internal_cpssDxChDiagMemoryBistsRun
(
    IN  GT_U8                                devNum,
    IN  CPSS_DIAG_PP_MEM_BIST_TYPE_ENT       memBistType,
    IN  CPSS_DIAG_PP_MEM_BIST_TEST_TYPE_ENT  bistTestType,
    IN  GT_U32                               timeOut,
    IN  GT_BOOL                              clearMemoryAfterTest,
    IN  GT_U32                               *testsToRunBmpPtr,
    OUT GT_U32                               *testsResultBmpPtr
)
{
    GT_STATUS   rc;                   /* return code  */
    GT_U32      tcamTestConfigAddr;   /* Policy TCAM Test Config Status Register */
    GT_U32      tcamOpcodeExtHitReAddr; /* TCAM Opcode Expected Hit Rel Register */
    GT_U32      tcamOpcodeExpectedHitRel; /* TCAM Opcode Expected Hit Rel value */
    GT_U32      tcamOpcodeCommandAddr;  /* TCAM Opcode Command Register */
    GT_U32      tcamConfig0Addr;        /* TCAM Config0 Register */
    GT_U32      tcamOpcodeCommandData;  /* TCAM Opcode Command data */
    GT_U32      interruptCauseReg;      /* interrupt cause register - ECC interrupt */
    GT_U32      bistOutcome;          /* TCAM Bist outcome */
    GT_U32      regData;              /* register data */
    GT_U32      iterations;           /* Counter for loops */
    GT_U32      maxIterations;        /* maximum iterations */
    static GT_U32 zeroBuff[6] = {0};  /* zero buffer */
    GT_U32      value;
    GT_U32      portGroupId;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_XCAT_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);


    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)
    {
        switch (bistTestType)
        {
            case CPSS_DIAG_PP_MEM_BIST_PURE_MEMORY_TEST_E:
                return prvCpssDxChDiagMemoryBistPureMemTestRun(devNum, memBistType,
                                                               timeOut, clearMemoryAfterTest);

            case CPSS_DIAG_PP_MEM_BIST_COMPARE_TEST_E:
                return prvCpssDxChDiagMemoryBistCompareMemTestRun(devNum,memBistType,
                                                                  timeOut, clearMemoryAfterTest, testsToRunBmpPtr, testsResultBmpPtr);

            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_CHEETAH3_E)
    {

        switch (memBistType)
        {
            case CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E:
                tcamTestConfigAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                    pclRegs.policyTcamTest.configStatusReg;
                tcamOpcodeExtHitReAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                    pclRegs.policyTcamTest.opcodeExpectedHitReg;
                tcamOpcodeCommandAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                    pclRegs.policyTcamTest.opcodeCommandReg;
                tcamOpcodeCommandData = 0x1E3233;
                tcamOpcodeExpectedHitRel = 0xB41;
                interruptCauseReg = (0 == PRV_CPSS_DXCH_LION_FAMILY_CHECK_MAC(devNum))
                                        ? 0xB800010 : 0xB800004;
                tcamConfig0Addr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                    pclRegs.pclTcamConfig0;
                DBG_INFO(("CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E \n"));
                break;
            case CPSS_DIAG_PP_MEM_BIST_ROUTER_TCAM_E:
                tcamTestConfigAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                    ipRegs.routerTcamTest.configStatusReg;
                tcamOpcodeExtHitReAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                    ipRegs.routerTcamTest.opcodeExpectedHitReg;
                tcamOpcodeCommandAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                    ipRegs.routerTcamTest.opcodeCommandReg;
                tcamOpcodeCommandData = 0x1E3033;
                tcamOpcodeExpectedHitRel = 0xB41;
                interruptCauseReg = (0 == PRV_CPSS_DXCH_LION_FAMILY_CHECK_MAC(devNum))
                                        ? 0x02040130 : 0x0D800060;
                tcamConfig0Addr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                    ipRegs.ipTcamConfig0;
                DBG_INFO(("CPSS_DIAG_PP_MEM_BIST_ROUTER_TCAM_E \n"));
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        maxIterations = timeOut / PRV_CH_DIAG_MINIMAL_SLEEP_TIME_CNS ;

        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
        {
            /* set default value to TCAM config register[1:4] */
            rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,portGroupId,tcamConfig0Addr,1, 4, &value);
            if (rc != GT_OK)
            {
                return rc;
            }

            if (value != 0)
            {
                rc = prvCpssDrvHwPpPortGroupSetRegField(devNum,portGroupId,tcamConfig0Addr,1, 4, 0);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }

            /* Trigger MBist */
            rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum, portGroupId, tcamTestConfigAddr, 0x10085);
            if( rc != GT_OK )
            {
                return rc;
            }

            iterations = 0;
            /* now busy wait until the BIST is finished */
            do
            {
                /* check that number of iteration not over the limit */
                if (iterations++ >= maxIterations)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, LOG_ERROR_NO_MSG);
                }
                cpssOsTimerWkAfter(PRV_CH_DIAG_MINIMAL_SLEEP_TIME_CNS);

                rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId, tcamTestConfigAddr, &regData);
                if( rc != GT_OK )
                {
                    return rc;
                }

            } while ((regData & 0x8)== 0);

            bistOutcome = (regData & 0x70) >> 4;
            DBG_INFO(("MBIST bistOutcome = %d \n",bistOutcome));
            if (bistOutcome != 0 && bistOutcome != 3)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_UNFIXABLE_BIST_ERROR, LOG_ERROR_NO_MSG);
            }

            /* Set CompBist expected results */
            rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum, portGroupId, tcamOpcodeExtHitReAddr, tcamOpcodeExpectedHitRel);
            if( rc != GT_OK )
            {
                return rc;
            }

            /* Set CompBist parameters */
            rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum, portGroupId, tcamOpcodeCommandAddr, tcamOpcodeCommandData);
            if( rc != GT_OK )
            {
                return rc;
            }

            /* Trigger CompBist */
            rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum, portGroupId, tcamTestConfigAddr, 0x10086);
            if( rc != GT_OK )
            {
                return rc;
            }

            iterations = 0;
            /* now busy wait until the BIST is finished */
            do
            {
                /* check that number of iteration not over the limit */
                if (iterations++ >= maxIterations)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, LOG_ERROR_NO_MSG);
                }
                cpssOsTimerWkAfter(PRV_CH_DIAG_MINIMAL_SLEEP_TIME_CNS);

                rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId, tcamTestConfigAddr, &regData);
                if( rc != GT_OK )
                {
                    return rc;
                }

            } while ((regData & 0x8) == 0);

            bistOutcome = (regData & 0x70) >> 4;
            DBG_INFO(("COMPBIST bistOutcome = %d \n",bistOutcome));

            if (bistOutcome != 0 && bistOutcome != 3 &&
                 bistOutcome != 4 && bistOutcome != 5)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_UNFIXABLE_BIST_ERROR, LOG_ERROR_NO_MSG);
            }
            /* Set TCAM IF back to Management */
            rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId,tcamTestConfigAddr,16,1,0);
            if( rc != GT_OK )
            {
                return rc;
            }
            /* Clear on read the ECC interrupt */
            rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId, interruptCauseReg, &regData);
            if( rc != GT_OK )
            {
                return rc;
            }

            /* restore the value of TCAM config register[1:4] */
            if (value != 0)
            {
                rc = prvCpssDrvHwPpPortGroupSetRegField(devNum,portGroupId,tcamConfig0Addr,1, 4, value);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }

            if (clearMemoryAfterTest == GT_TRUE)
            {
                if (memBistType == CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E)
                {
                    maxIterations = (0 == PRV_CPSS_DXCH_LION_FAMILY_CHECK_MAC(devNum)) ?
                                                (4*PRV_DXCH3_PCL_TCAM_RAW_NUMBER_CNS ) : _3K;
                    for( iterations = 0; iterations < maxIterations; iterations++)
                    {
                        /*  Resets the TCAM data to the default value */
                        rc = prvCpssDxChPclTcamStdRuleXandYSet(
                                devNum, portGroupId,
                                iterations, zeroBuff, zeroBuff,
                                1 /* xValid0 */,1 /* yValid0 */,1 /* xValid */,1 /* yValid */,
                                0 /* xSize */,0 /* ySize */,0 /* xSpare */,0 /* ySpare  */);
                        if (rc != GT_OK)
                        {
                            return rc;
                        }
                    }

                }
                else if (memBistType == CPSS_DIAG_PP_MEM_BIST_ROUTER_TCAM_E)
                {
                    maxIterations = PRV_CPSS_DXCH_ROUTER_TUNNEL_TERM_GET_MAX_SIZE_MAC(devNum);
                    for( iterations = 0; iterations < maxIterations ; iterations++)
                    {
                        rc = prvCpssDxChRouterTunnelTermTcamInvalidateLine(
                                devNum,portGroupId,iterations);
                        if (rc != GT_OK)
                        {
                            return rc;
                        }
                    }

                }
            }

        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
    }
    else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
    {
        GT_U32 dfxPipeNum, dfxClientNum, xsbNum;
        GT_BOOL testStatus;
        testStatus = GT_TRUE;
        rc         = GT_OK;
        switch (memBistType)
        {
            case CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E:
                DBG_INFO(("CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E \n"));
                dfxPipeNum   = 0;
                dfxClientNum = 4;
                xsbNum       = 0;
                rc = prvCpssDxChDiagDfxTcamBistRun(
                    devNum, 1 /*tcamsAmount*/,
                    &dfxPipeNum, &dfxClientNum, &xsbNum,
                    timeOut, &testStatus);
                break;
            case CPSS_DIAG_PP_MEM_BIST_ROUTER_TCAM_E:
                DBG_INFO(("CPSS_DIAG_PP_MEM_BIST_ROUTER_TCAM_E \n"));
                dfxPipeNum   = 0;
                dfxClientNum = 2;
                xsbNum       = 0;
                rc = prvCpssDxChDiagDfxTcamBistRun(
                    devNum, 1 /*tcamsAmount*/,
                    &dfxPipeNum, &dfxClientNum, &xsbNum,
                    timeOut, &testStatus);
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        if (rc != GT_OK)
        {
            if (testsResultBmpPtr != NULL) *testsResultBmpPtr = 0;
            return rc;
        }
        else if (testStatus == GT_FALSE)
        {
            if (testsResultBmpPtr != NULL) *testsResultBmpPtr = 0;
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_UNFIXABLE_BIST_ERROR, LOG_ERROR_NO_MSG);
        }
        else
        {
            if (testsResultBmpPtr != NULL) *testsResultBmpPtr = 1;
            return GT_OK;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChDiagMemoryBistsRun function
* @endinternal
*
* @brief   Runs BIST (Built-in self-test) on specified memory.
*
* @note   APPLICABLE DEVICES:      DxCh3; Lion; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] memBistType              - type of memory
* @param[in] bistTestType             - BIST test type (pure memory test/compare memory test)
* @param[in] timeOut                  - maximal time in milisecond to wait for BIST finish.
* @param[in] clearMemoryAfterTest     - GT_TRUE  - Clear memory after test.
*                                      GT_FALSE - Don't clear memory after test.
* @param[in] testsToRunBmpPtr         - bitmap of required test related for
*                                      CPSS_DIAG_PP_MEM_BIST_COMPARE_TEST_E
*                                      (from 0 to CPSS_DIAG_PP_MEM_BIST_COMPARE_TESTS_NUM_CNS,
*                                      so minimum bitmap size 32 bits, maximum depends on future
*                                      number of tests); 1 - run test, 0 - don't run test.
*                                      Bit 0 set in bitmap means run test 0.0 from "Table 1: TCAM Compare BIST:
*                                      Parameters and Expected Result Configurations"
*                                      (see full tests list and definitions in Application Notes)
*
* @param[out] testsResultBmpPtr        - pointer to bitmap with results of compare memory test,
*                                      !!! pay attention: 0 - test pass, 1 - test failed;
*                                      if NULL, just final result of test will be returned;
*                                      regarding size see testsToRunBmpPtr above
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_UNFIXABLE_BIST_ERROR  - on unfixable problem in the TCAM
*
* @note 1. This function should not be called under traffic.
*       2. Bist destroys the content of the memory. If clearMemoryAfterTest set
*       function will clear the content of the memory after the test.
*       Application responsible to restore the content after the function
*       completion.
*
*/
GT_STATUS cpssDxChDiagMemoryBistsRun
(
    IN  GT_U8                                devNum,
    IN  CPSS_DIAG_PP_MEM_BIST_TYPE_ENT       memBistType,
    IN  CPSS_DIAG_PP_MEM_BIST_TEST_TYPE_ENT  bistTestType,
    IN  GT_U32                               timeOut,
    IN  GT_BOOL                              clearMemoryAfterTest,
    IN  GT_U32                               *testsToRunBmpPtr,
    OUT GT_U32                               *testsResultBmpPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagMemoryBistsRun);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, memBistType, bistTestType, timeOut, clearMemoryAfterTest, testsToRunBmpPtr, testsResultBmpPtr));

    rc = internal_cpssDxChDiagMemoryBistsRun(devNum, memBistType, bistTestType, timeOut, clearMemoryAfterTest, testsToRunBmpPtr, testsResultBmpPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, memBistType, bistTestType, timeOut, clearMemoryAfterTest, testsToRunBmpPtr, testsResultBmpPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChDiagMemoryBistBlockStatusGet function
* @endinternal
*
* @brief   Gets redundancy block status.
*
* @note   APPLICABLE DEVICES:      DxCh3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] memBistType              - type of memory
* @param[in] blockIndex               - block index
*                                      CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E:
*                                      valid blockIndex: 0 to 13.
*                                      CPSS_DIAG_PP_MEM_BIST_ROUTER_TCAM_E:
*                                      valid blockIndex: 0 to 19.
*
* @param[out] blockFixedPtr            - (pointer to) block status
*                                      GT_TRUE - row in the block was fixed
*                                      GT_FALSE - row in the block wasn't fixed
* @param[out] replacedIndexPtr         - (pointer to) replaced row index.
*                                      Only valid if  blockFixedPtr is GT_TRUE.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*
* @note Each redundancy block contains 256 rows. Block 0: rows 0 to 255,
*       block 1: raws 256 to 511 ...
*       One raw replacement is possible though the full redundancy block
*       of 256 rows.
*
*/
static GT_STATUS internal_cpssDxChDiagMemoryBistBlockStatusGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DIAG_PP_MEM_BIST_TYPE_ENT  memBistType,
    IN  GT_U32                          blockIndex,
    OUT GT_BOOL                         *blockFixedPtr,
    OUT GT_U32                          *replacedIndexPtr
)
{
    GT_U32 regAdrr;
    GT_U32 regData;
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E |
                                          CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    CPSS_NULL_PTR_CHECK_MAC(blockFixedPtr);
    CPSS_NULL_PTR_CHECK_MAC(replacedIndexPtr);

    switch (memBistType)
    {
        case CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E:
            if (blockIndex >= 14)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            regAdrr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                pclRegs.policyTcamTest.pointerRelReg[blockIndex];
            break;
        case CPSS_DIAG_PP_MEM_BIST_ROUTER_TCAM_E:
            if (blockIndex >= 20)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            regAdrr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                ipRegs.routerTcamTest.pointerRelReg[blockIndex];
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDrvHwPpReadRegister(devNum, regAdrr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }
    *blockFixedPtr = BIT2BOOL_MAC((regData >> 8) & 1);
    if (*blockFixedPtr == GT_TRUE)
    {
        *replacedIndexPtr = regData & 0xFF;
    }
    return GT_OK;
}

/**
* @internal cpssDxChDiagMemoryBistBlockStatusGet function
* @endinternal
*
* @brief   Gets redundancy block status.
*
* @note   APPLICABLE DEVICES:      DxCh3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] memBistType              - type of memory
* @param[in] blockIndex               - block index
*                                      CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E:
*                                      valid blockIndex: 0 to 13.
*                                      CPSS_DIAG_PP_MEM_BIST_ROUTER_TCAM_E:
*                                      valid blockIndex: 0 to 19.
*
* @param[out] blockFixedPtr            - (pointer to) block status
*                                      GT_TRUE - row in the block was fixed
*                                      GT_FALSE - row in the block wasn't fixed
* @param[out] replacedIndexPtr         - (pointer to) replaced row index.
*                                      Only valid if  blockFixedPtr is GT_TRUE.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*
* @note Each redundancy block contains 256 rows. Block 0: rows 0 to 255,
*       block 1: raws 256 to 511 ...
*       One raw replacement is possible though the full redundancy block
*       of 256 rows.
*
*/
GT_STATUS cpssDxChDiagMemoryBistBlockStatusGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DIAG_PP_MEM_BIST_TYPE_ENT  memBistType,
    IN  GT_U32                          blockIndex,
    OUT GT_BOOL                         *blockFixedPtr,
    OUT GT_U32                          *replacedIndexPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagMemoryBistBlockStatusGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, memBistType, blockIndex, blockFixedPtr, replacedIndexPtr));

    rc = internal_cpssDxChDiagMemoryBistBlockStatusGet(devNum, memBistType, blockIndex, blockFixedPtr, replacedIndexPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, memBistType, blockIndex, blockFixedPtr, replacedIndexPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChDiagDeviceTemperatureSensorsSelectSet function
* @endinternal
*
* @brief   Select Temperature Sensors.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] sensorType               - Sensor type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum, sensorType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS internal_cpssDxChDiagDeviceTemperatureSensorsSelectSet
(
    IN  GT_U8   devNum,
    IN  CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_ENT  sensorType
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      value;      /* register field value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
         CPSS_LION_E | CPSS_XCAT2_E);

    if(PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        return GT_OK;
    }

    switch(sensorType)
    {
        case CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_0_E:
            value = 0;
            break;
        case CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_1_E:
            value = 1;
            break;
        case CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_2_E:
            value = 2;
            break;
        case CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_3_E:
            value = 3;
            break;
        case CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_4_E:
            value = 4;
            break;
        case CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_MAX_E:
            if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            value = 6;
            break;
        case CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_AVERAGE_E:
            value = 7;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Set Temperature Sensor */
    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                    dfxUnits.server.temperatureSensorControlMsb;

        rc = prvCpssHwPpSetRegField(devNum, regAddr, 25, 3, value);
    }
    else /* PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE || */
    {    /* PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E */
        switch (PRV_CPSS_PP_MAC(devNum)->devFamily)
        {
            case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
                if (PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_NONE_E)
                {
                    if ((value >= 2) && (value <= 4))
                    {
                        /* sensors 2..4 are not connected in bobcat2 (only sensors 0,1) */
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                    }
                }
                else
                {
                    /* Cetus and Caelum */
                    if ((value >= 1) && (value <= 4))
                    {
                        /* sensors 1..4 are not connected in Cetus and Caelum */
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                    }
                }

                break;
            case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            case CPSS_PP_FAMILY_DXCH_AC3X_E:
            case CPSS_PP_FAMILY_DXCH_XCAT3_E:
                if ((value >= 1) && (value <= 4))
                {
                    /* sensors 1..4 are not connected in Aldrin, AC3X and xCat3 */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }

                break;
            case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
                if (value == 4)
                {
                    /* sensor 4 is not connected in Aldrin2 */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }

                break;
            default: /* BC3 */
                break;
        }

        regAddr =
            PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                      DFXServerUnits.DFXServerRegs.temperatureSensor28nmCtrlMSB;
        rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(
                                                 devNum, regAddr, 28, 3, value);
    }

    return rc;
}

/**
* @internal cpssDxChDiagDeviceTemperatureSensorsSelectSet function
* @endinternal
*
* @brief   Select Temperature Sensors.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] sensorType               - Sensor type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum, sensorType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS cpssDxChDiagDeviceTemperatureSensorsSelectSet
(
    IN  GT_U8   devNum,
    IN  CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_ENT  sensorType
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagDeviceTemperatureSensorsSelectSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, sensorType));

    rc = internal_cpssDxChDiagDeviceTemperatureSensorsSelectSet(devNum, sensorType);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, sensorType));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChDiagDeviceTemperatureSensorsSelectGet function
* @endinternal
*
* @brief   Get Temperature Sensors Select.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - device number
*
* @param[out] sensorTypePtr            - Pointer to Sensor type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS internal_cpssDxChDiagDeviceTemperatureSensorsSelectGet
(
    IN  GT_U8   devNum,
    OUT CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_ENT  *sensorTypePtr
)
{
    GT_U32      regAddr;    /* register address */
    GT_U32      value;      /* register field value */
    GT_STATUS   rc;         /* return status */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
         CPSS_LION_E | CPSS_XCAT2_E);

    CPSS_NULL_PTR_CHECK_MAC(sensorTypePtr);

    if(PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        *sensorTypePtr = CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_0_E;
        return GT_OK;
    }

    /* Get Temperature Sensor */
    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                    dfxUnits.server.temperatureSensorControlMsb;

        rc = prvCpssHwPpGetRegField(devNum, regAddr, 25, 3, &value);
    }
    else /* PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE || */
    {    /* PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E */
        regAddr =
            PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                      DFXServerUnits.DFXServerRegs.temperatureSensor28nmCtrlMSB;
        rc = prvCpssDrvHwPpResetAndInitControllerGetRegField(
                                                devNum, regAddr, 28, 3, &value);
    }

    if(rc != GT_OK)
    {
        return rc;
    }

    switch(value)
    {
        case 0:
            *sensorTypePtr = CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_0_E;
            break;
        case 1:
            *sensorTypePtr = CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_1_E;
            break;
        case 2:
            *sensorTypePtr = CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_2_E;
            break;
        case 3:
            *sensorTypePtr = CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_3_E;
            break;
        case 4:
            *sensorTypePtr = CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_4_E;
            break;
        case 6:
            if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
            *sensorTypePtr = CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_MAX_E;
            break;
        case 7:
            *sensorTypePtr = CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_AVERAGE_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    return rc;

}

/**
* @internal cpssDxChDiagDeviceTemperatureSensorsSelectGet function
* @endinternal
*
* @brief   Get Temperature Sensors Select.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - device number
*
* @param[out] sensorTypePtr            - Pointer to Sensor type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS cpssDxChDiagDeviceTemperatureSensorsSelectGet
(
    IN  GT_U8   devNum,
    OUT CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_ENT  *sensorTypePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagDeviceTemperatureSensorsSelectGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, sensorTypePtr));

    rc = internal_cpssDxChDiagDeviceTemperatureSensorsSelectGet(devNum, sensorTypePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, sensorTypePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChDiagDeviceTemperatureThresholdSet function
* @endinternal
*
* @brief   Set Threshold for Interrupt. If Temperature Sensors is equal or above
*         the threshold, interrupt is activated.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] thresholdValue           - Threshold value in Celsius degrees.
*                                      (APPLICABLE RANGES: Lion2 -142..228)
*                                      (APPLICABLE RANGES: xCat3, Bobcat2, Caelum, Bobcat3; Aldrin2; Falcon, Aldrin, AC3X -277..198)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_OUT_OF_RANGE          - on wrong thresholdValue
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS internal_cpssDxChDiagDeviceTemperatureThresholdSet
(
    IN  GT_U8  devNum,
    IN  GT_32  thresholdValue
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      value;      /* register field value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
         CPSS_LION_E | CPSS_XCAT2_E);

    if(PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        return GT_OK;
    }

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
    {
        /* Calculate temperature by formula below: */
        /* RegValue = 315.3  - Tjunc * 1.3825 */

        value = (3153000 - thresholdValue * 13825) / 10000;

        if(value >= BIT_9)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                    dfxUnits.server.temperatureSensorControlMsb;

        rc = prvCpssHwPpSetRegField(devNum, regAddr, 16, 9, value);
    }
    else /* PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE || */
    {    /* PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E */
        if((thresholdValue < -40) || (thresholdValue > 150))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* TSEN28 Formula from IP doc: y = 2.1445*x + 584.75,
           where x=T, y=Temperature Sensor Readout */
        value = (5847500 + thresholdValue * 21445)/10000;

        if(value >= BIT_10)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        regAddr =
            PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                      DFXServerUnits.DFXServerRegs.temperatureSensor28nmCtrlMSB;
        rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(
                                                devNum, regAddr, 16, 10, value);
    }

    return rc;
}

/**
* @internal cpssDxChDiagDeviceTemperatureThresholdSet function
* @endinternal
*
* @brief   Set Threshold for Interrupt. If Temperature Sensors is equal or above
*         the threshold, interrupt is activated.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] thresholdValue           - Threshold value in Celsius degrees.
*                                      (APPLICABLE RANGES: Lion2 -142..228)
*                                      (APPLICABLE RANGES: xCat3, Bobcat2, Caelum, Bobcat3; Aldrin2; Falcon, Aldrin, AC3X -277..198)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_OUT_OF_RANGE          - on wrong thresholdValue
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS cpssDxChDiagDeviceTemperatureThresholdSet
(
    IN  GT_U8  devNum,
    IN  GT_32  thresholdValue
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagDeviceTemperatureThresholdSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, thresholdValue));

    rc = internal_cpssDxChDiagDeviceTemperatureThresholdSet(devNum, thresholdValue);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, thresholdValue));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChDiagDeviceTemperatureThresholdGet function
* @endinternal
*
* @brief   Get Threshold for Interrupt. If Temperature Sensors is equal or above
*         the threshold, interrupt is activated.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - device number
*
* @param[out] thresholdValuePtr        - pointer to Threshold value in Celsius degrees.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS internal_cpssDxChDiagDeviceTemperatureThresholdGet
(
    IN  GT_U8  devNum,
    OUT GT_32  *thresholdValuePtr
)
{
    GT_STATUS   rc;         /* return status */
    GT_U32      regAddr;    /* register address */
    GT_U32      value;      /* register field value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
         CPSS_LION_E | CPSS_XCAT2_E);

    CPSS_NULL_PTR_CHECK_MAC(thresholdValuePtr);

    if(PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        *thresholdValuePtr = 110;
        return GT_OK;
    }

    /* Get Threshold for Interrupt. */
    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                    dfxUnits.server.temperatureSensorControlMsb;

        rc = prvCpssHwPpGetRegField(devNum, regAddr, 16, 9, &value);
    }
    else /* PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE || */
    {    /* PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E */
        regAddr =
            PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                      DFXServerUnits.DFXServerRegs.temperatureSensor28nmCtrlMSB;
        rc = prvCpssDrvHwPpResetAndInitControllerGetRegField(
                                                devNum, regAddr, 16, 10, &value);
    }

    if(rc != GT_OK)
    {
        return rc;
    }

    /* Calculate temperature by formulas below: */
    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
    {
        /* T(in Celsius) = (315.3 - hwValue )/1.3825 */
        *thresholdValuePtr = (GT_32)(3153000 - value * 10000) / 13825;
    }
    else /* PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE || */
    {    /* PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E */
        /* TSEN28 Formula from IP doc: y = 2.1445*x + 584.75,
           where x=T, y=Temperature Sensor Readout */
        /* T(in Celsius) = (hwValue - 584.75)/2.1445 */
        *thresholdValuePtr = (GT_32)(value * 10000 - 5847500) / 21445;
    }

    return rc;
}

/**
* @internal cpssDxChDiagDeviceTemperatureThresholdGet function
* @endinternal
*
* @brief   Get Threshold for Interrupt. If Temperature Sensors is equal or above
*         the threshold, interrupt is activated.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - device number
*
* @param[out] thresholdValuePtr        - pointer to Threshold value in Celsius degrees.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS cpssDxChDiagDeviceTemperatureThresholdGet
(
    IN  GT_U8  devNum,
    OUT GT_32  *thresholdValuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagDeviceTemperatureThresholdGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, thresholdValuePtr));

    rc = internal_cpssDxChDiagDeviceTemperatureThresholdGet(devNum, thresholdValuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, thresholdValuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChDiagDeviceTemperatureGet function
* @endinternal
*
* @brief   Gets the PP temperature.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat.
*
* @param[in] devNum                   - device number
*
* @param[out] temperaturePtr           - (pointer to) temperature in Celsius degrees
*                                      (can be negative)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS internal_cpssDxChDiagDeviceTemperatureGet
(
    IN  GT_U8    devNum,
    OUT GT_32    *temperaturePtr
)
{
    GT_STATUS  rc;            /* return code       */
    GT_U32     regAddr;       /* register address  */
    GT_U32     hwValue;       /* HW Value          */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E);

    CPSS_NULL_PTR_CHECK_MAC(temperaturePtr);

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E ||
        PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        if(PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
        {
            *temperaturePtr = 0;
            return GT_OK;
        }
    }

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)
    {
        /* lion */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            txqVer1.sht.dfx.dfxStatus0;
    }
    else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E ||
             PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E)
    {
        /* xCat2; lion2 */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            dfxUnits.server.temperatureSensorStatus;
    }
    else
    {
        /* xCat3; Bobcat2; Caelum; Bobcat3; Aldrin2 */
        regAddr =
            PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                           DFXServerUnits.DFXServerRegs.temperatureSensorStatus;
    }

    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
    {
        /* lion2 */
        rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 9, &hwValue);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Calculate temperature by formula below: */
        /* T(in Celsius) = (315.3 - hwValue )/1.3825 */

        *temperaturePtr = ((3153000 - ((GT_32)hwValue * 10000)) / 13825);


    }
    else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E ||
             PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)
    {
        /* xCat2; lion */
        rc = prvCpssDrvHwPpPortGroupGetRegField(
            devNum, 0 /*portGroupId*/, regAddr, 19, 9, &hwValue);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Calculate temperature by formula below: */
        /* T(in Celsius) = (322 - hwValue )/1.3625 */

        *temperaturePtr = ((3220000 - ((GT_32)hwValue * 10000)) / 13625);

    }
    else /* PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE || */
    {    /* PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E */
        /* xCat3; Bobcat2; Caelum; Bobcat3; Aldrin2 */
        rc = prvCpssDrvHwPpResetAndInitControllerGetRegField(
                                              devNum, regAddr, 0, 11, &hwValue);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Calculate temperature by formula below: */
        /* TSEN28 Formula from IP doc: y = 2.1445*x + 584.75,
           where x=T, y=Temperature Sensor Readout */
        /* T(in Celsius) = (hwValue - 584.75)/2.1445 */
        /* hwValue & 0x3FF - from bit 0 to bit 9 */
        *temperaturePtr = (GT_32)((hwValue & 0x3FF) * 10000 - 5847500) / 21445;

        /*Temperature Sensor Redout is Valid*/
#ifndef ASIC_SIMULATION
        /* hwValue & 0x400 - bit 10 */
        if((hwValue & 0x400) == 0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_READY, LOG_ERROR_NO_MSG);
        }
#endif

    }

    return GT_OK;
}

/**
* @internal cpssDxChDiagDeviceTemperatureGet function
* @endinternal
*
* @brief   Gets the PP temperature.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat.
*
* @param[in] devNum                   - device number
*
* @param[out] temperaturePtr           - (pointer to) temperature in Celsius degrees
*                                      (can be negative)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS cpssDxChDiagDeviceTemperatureGet
(
    IN  GT_U8    devNum,
    OUT GT_32    *temperaturePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagDeviceTemperatureGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, temperaturePtr));

    rc = internal_cpssDxChDiagDeviceTemperatureGet(devNum, temperaturePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, temperaturePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChDiagDeviceVoltageGet function
* @endinternal
*
* @brief   Gets the PP sensor voltage.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] sensorNum                - sensor number (APPLICABLE RANGES: 0..3)
*
* @param[out] voltagePtr               - (pointer to) voltage in milivolts
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - null pointer
*/
static GT_STATUS internal_cpssDxChDiagDeviceVoltageGet
(
    IN  GT_U8     devNum,
    IN  GT_U32    sensorNum,
    OUT GT_U32    *voltagePtr
)
{
    GT_STATUS  rc;
    GT_U32     regAddrTempSensor;       /* register addresses */
    GT_U32     regAddrTempCtrlSensorLSB;
    GT_U32     regAddrTempCtrlSensorMSB;
    GT_U32     hwValue;       /* HW Value */
    GT_U32    divider_enabled;
    GT_U32    divider_config;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
            CPSS_CH1_E | CPSS_CH1_DIAMOND_E  |  CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
            CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_XCAT3_E);
    CPSS_NULL_PTR_CHECK_MAC(voltagePtr);
    if (sensorNum >= 4)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddrTempCtrlSensorLSB = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                DFXServerUnits.DFXServerRegs.temperatureSensor28nmCtrlLSB;
    regAddrTempCtrlSensorMSB = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                DFXServerUnits.DFXServerRegs.temperatureSensor28nmCtrlMSB;
    regAddrTempSensor = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                DFXServerUnits.DFXServerRegs.temperatureSensorStatus;

    /* set VSEN mode */
    rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddrTempCtrlSensorLSB, 6, 1, 0);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDrvHwPpResetAndInitControllerGetRegField(devNum, regAddrTempCtrlSensorLSB, 31, 1, &divider_enabled);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDrvHwPpResetAndInitControllerGetRegField(devNum, regAddrTempCtrlSensorMSB, 3, 1, &divider_config);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* choose sensor to read voltage */
    rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddrTempCtrlSensorLSB, 3, 2, sensorNum); /* sensor ID  from 0 to 3 */
    if (rc != GT_OK)
    {
        return rc;
    }

    /* WAIT is necessary here to give time for correct value to settle in tempRegister,
       may depend on 'Tsen Readout Avg' */
    cpssOsTimerWkAfter(100);

    /* readout raw value */
    rc = prvCpssDrvHwPpResetAndInitControllerGetRegField(devNum,  regAddrTempSensor, 0, 10, &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (divider_enabled)
    {
        if (divider_config)
        {
            /* DIV_EN = 1  DIV_CFG = 1 */
            /* hwValue = -981.63 * V + 1595.8 */
            *voltagePtr = (159580 - hwValue * 100) * 1000 / 98163;
        }
        else
        {
            /* DIV_EN = 1  DIV_CFG = 0 */
            /* hwValue = -662.37 * V + 1605.7 */
            *voltagePtr = (160570 - hwValue * 100) * 1000 / 66237;
        }
    }
    else
    {
        /* DIV_EN = 0 */
        /* hwValue = -1322.6 * V + 1598.5 */
        *voltagePtr = (15985 - hwValue * 10)*1000 / 13226;
    }

    /* reset  back to TSEN mode */
    rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddrTempCtrlSensorLSB, 6, 1, 1);
    return rc;
}

/**
* @internal cpssDxChDiagDeviceVoltageGet function
* @endinternal
*
* @brief   Gets the PP sensor voltage.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] sensorNum                - sensor number (APPLICABLE RANGES: 0..3)
*
* @param[out] voltagePtr               - (pointer to) voltage in milivolts
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - null pointer
*/
GT_STATUS cpssDxChDiagDeviceVoltageGet
(
    IN  GT_U8     devNum,
    IN  GT_U32    sensorNum,
    OUT GT_U32    *voltagePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagDeviceVoltageGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, sensorNum, voltagePtr));

    rc = internal_cpssDxChDiagDeviceVoltageGet(devNum, sensorNum, voltagePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, sensorNum, voltagePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChDiagPortGroupMemWrite function
* @endinternal
*
* @brief   performs a single 32 bit data write to one of the specific port group in
*         PP memory spaces.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Aldrin2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] memType                  - The packet processor memory type
* @param[in] offset                   - The offset address to write to
* @param[in] data                     - data to write
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_TIMEOUT               - after max time that the PP not finished action
* @retval GT_BAD_PARAM             - on wrong devNum, memType or offset
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note offset must be aligned to 4 Bytes.
*       The function may be called after Phase 2 initialization.
*
*/
static GT_STATUS internal_cpssDxChDiagPortGroupMemWrite
(
    IN GT_U8                          devNum,
    IN GT_PORT_GROUPS_BMP             portGroupsBmp,
    IN CPSS_DIAG_PP_MEM_TYPE_ENT      memType,
    IN GT_U32                         offset,
    IN GT_U32                         data
)
{
    GT_U32      memBase;            /* holds RAM base Address */
    GT_STATUS   ret;                /* function return result */
    GT_U32      addr;               /* address to write */
    GT_U32      bank0Start;         /* start address of the Memory Bank0 */
    GT_U32      bank0Size;          /* size of the Memory Bank0 */
    GT_U32      bank1Start;         /* start address of the Memory Bank1 */
    GT_U32      bank1Size;          /* size of the Memory Bank1 */
    GT_U32      lineAndBunkNum;     /* The number of line and bank to write to
                                       (Relevant only for Lion) */
    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC *fineTuningPtr;
    GT_U32      portGroupId;  /*the port group Id - support multi-port-groups device */
    GT_U32      validPortGroupsBmp; /* bitmap of valid portGroups       */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum, portGroupsBmp);

    /* check input parameters */
    if (offset % 4)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    /*Prepare Address Space Partitioning settings*/
    ret = getMemBaseFromType(devNum, memType, &memBase);
    if (ret != GT_OK)
    {
        return ret;
    }

    /* Get bitmap of valid portGroups */
    if(GT_FALSE ==
        prvCpssDuplicatedMultiPortGroupsGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
            memBase,&validPortGroupsBmp,NULL,NULL))
    {
        validPortGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    }

    portGroupsBmp &= validPortGroupsBmp;

    if (portGroupsBmp == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    /* calculate the base address */
    addr = memBase + offset;

    if ((memType == CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E) ||
        (memType == CPSS_DIAG_PP_MEM_VLAN_TBL_MEM_E))
    {
        /* loop on all active port groups in the bmp */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
        {
            ret = hwPpTableEntryFieldWrite (devNum, portGroupId, addr, data);
            if (ret != GT_OK)
            {
                return ret;
            }

        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)

        return GT_OK;
    }
    else if (memType == CPSS_DIAG_PP_MEM_BUFFER_DRAM_E)
    {
        /* NOTE:
           getMemBaseFromType(...) function already knows that :
            Lion2,Bobcat2,Caelum,Bobcat3 are not supported
        */

        /* check if the device is Lion */
        if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)
        {
            /* The data will be written to all words in this line */
            lineAndBunkNum = (offset / LION_BUFFER_DRAM_PAGE_SIZE_CNS);

            fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning;

            /* check validity */
            if ((lineAndBunkNum * LION_BUFFER_DRAM_PAGE_SIZE_CNS * 8) >
                fineTuningPtr->tableSize.bufferMemory)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

                addr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->addrSpace.buffMemBank0Write +
                          (lineAndBunkNum << 8);

            /* loop on all active port groups in the bmp */
            PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
            {
                ret = prvCpssDrvHwPpPortGroupWriteRegister(devNum, portGroupId,
                                                           addr, data);
                if (ret != GT_OK)
                {
                    return ret;
                }
            }
            PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
        }
        else /* xCat2, xCat, xCat3, DxCh3, DxCh2 and DxCh1 */
        {
            /* check if the device is Cheetah3 */
            /* In Cheetah and Cheetah2 the Banks are sequential, so when the offset slides
               to Bank1, the address will be still (memBase + offset).
               In Cheetah3, the Banks are not sequential, so when the offset slides to
               Bank1, the address should be recalculated */
            if (PRV_CPSS_PP_MAC(devNum)->devFamily >= CPSS_PP_FAMILY_CHEETAH3_E)
            {
                getBanksParams (devNum, &bank0Start, &bank0Size, &bank1Start, &bank1Size);

                /* check if the offset is for Bank1 */
                if (offset >= bank0Size)
                {
                    addr = bank1Start + (offset - bank0Size);
                }
            }

            /* loop on all active port groups in the bmp */
            PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
            {
                ret = hwPpRamBuffMemWordWrite (devNum, portGroupId, addr, data);
                if (ret != GT_OK)
                {
                    return ret;
                }
            }
            PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
        }

        return GT_OK;
    }
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
}

/**
* @internal cpssDxChDiagPortGroupMemWrite function
* @endinternal
*
* @brief   performs a single 32 bit data write to one of the specific port group in
*         PP memory spaces.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Aldrin2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] memType                  - The packet processor memory type
* @param[in] offset                   - The offset address to write to
* @param[in] data                     - data to write
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_TIMEOUT               - after max time that the PP not finished action
* @retval GT_BAD_PARAM             - on wrong devNum, memType or offset
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note offset must be aligned to 4 Bytes.
*       The function may be called after Phase 2 initialization.
*
*/
GT_STATUS cpssDxChDiagPortGroupMemWrite
(
    IN GT_U8                          devNum,
    IN GT_PORT_GROUPS_BMP             portGroupsBmp,
    IN CPSS_DIAG_PP_MEM_TYPE_ENT      memType,
    IN GT_U32                         offset,
    IN GT_U32                         data
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagPortGroupMemWrite);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, memType, offset, data));

    rc = internal_cpssDxChDiagPortGroupMemWrite(devNum, portGroupsBmp, memType, offset, data);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, memType, offset, data));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChDiagPortGroupMemRead function
* @endinternal
*
* @brief   performs a single 32 bit data read from one of the specific port group
*         in the PP memory spaces.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Aldrin2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported,
*                                      in this case read is done from first active port group.
* @param[in] memType                  - The packet processor memory type
* @param[in] offset                   - The offset address to read from
*
* @param[out] dataPtr                  - (pointer to) read data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_BAD_PARAM             - on wrong devNum, memType or offset
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note offset must be aligned to 4 Bytes.
*       The function may be called after Phase 2 initialization.
*
*/
static GT_STATUS internal_cpssDxChDiagPortGroupMemRead
(
    IN GT_U8                          devNum,
    IN GT_PORT_GROUPS_BMP             portGroupsBmp,
    IN CPSS_DIAG_PP_MEM_TYPE_ENT      memType,
    IN GT_U32                         offset,
    OUT GT_U32                        *dataPtr
)
{
    GT_U32      memBase;            /* holds RAM base Address */
    GT_STATUS   ret;                /* function return result */
    GT_U32      addr;               /* address to read */
    GT_U32      bank0Start;         /* start address of the Memory Bank0 */
    GT_U32      bank0Size;          /* size of the Memory Bank0 */
    GT_U32      bank1Start;         /* start address of the Memory Bank1 */
    GT_U32      bank1Size;          /* size of the Memory Bank1 */
    GT_U32      lineAndBunkNum;     /* The number of line and bank to write to
                                       (Relevant only for Lion) */
    GT_U32      wordNum;            /* Word number in the line          */
    GT_U32      portGroupId;        /* port group Id */
    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC *fineTuningPtr;
    GT_U32      validPortGroupsBmp; /* bitmap of valid portGroups       */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(dataPtr);

    /* check input parameters */
    if(offset % 4)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Prepare Address Space Partitioning settings */
    ret = getMemBaseFromType(devNum, memType, &memBase);
    if(ret != GT_OK)
    {
        return ret;
    }

    /* Get bitmap of valid portGroups */
    if(GT_FALSE ==
        prvCpssDuplicatedMultiPortGroupsGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
            memBase,&validPortGroupsBmp,NULL,NULL))
    {
        validPortGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    }

    portGroupsBmp &= validPortGroupsBmp;

    if (portGroupsBmp == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Get the first active port group id */
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(devNum, portGroupsBmp,
                                                        portGroupId);


    /* calculate the test base address */
    addr = memBase + offset;

    if ((memType == CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E) ||
        (memType == CPSS_DIAG_PP_MEM_VLAN_TBL_MEM_E))
    {
        return hwPpTableEntryFieldRead (devNum, portGroupId, addr, dataPtr);
    }
    else if (memType == CPSS_DIAG_PP_MEM_BUFFER_DRAM_E)
    {
        /* check if the device is Lion */
        if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)
        {
            /* calculate line and bunk number */
            lineAndBunkNum = (offset / LION_BUFFER_DRAM_PAGE_SIZE_CNS);

            /* calculate word number */
            wordNum = (offset % (LION_BUFFER_DRAM_PAGE_SIZE_CNS / 4));

            fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning;

            /* check validity */
            if ((lineAndBunkNum * LION_BUFFER_DRAM_PAGE_SIZE_CNS * 8) >
                fineTuningPtr->tableSize.bufferMemory)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

                addr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->addrSpace.buffMemBank0Write +
                       (lineAndBunkNum << 8) + (wordNum << 2);

            return prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId, addr, dataPtr);
        }
        else /* xCat2, xCat, xCat3, DxCh3, DxCh2 and DxCh1 */
        {
            /* check if the device is Cheetah3 */
            if (PRV_CPSS_PP_MAC(devNum)->devFamily >= CPSS_PP_FAMILY_CHEETAH3_E)
            {
                getBanksParams (devNum, &bank0Start, &bank0Size, &bank1Start, &bank1Size);

                /* check if the offset is for Bank1 */
                if (offset >= bank0Size)
                {
                    addr = bank1Start + (offset - bank0Size);
                }
            }

            return hwPpBufferRamInWordsRead (devNum, portGroupId, addr, 1, dataPtr);
        }
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
}

/**
* @internal cpssDxChDiagPortGroupMemRead function
* @endinternal
*
* @brief   performs a single 32 bit data read from one of the specific port group
*         in the PP memory spaces.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Aldrin2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported,
*                                      in this case read is done from first active port group.
* @param[in] memType                  - The packet processor memory type
* @param[in] offset                   - The offset address to read from
*
* @param[out] dataPtr                  - (pointer to) read data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_BAD_PARAM             - on wrong devNum, memType or offset
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note offset must be aligned to 4 Bytes.
*       The function may be called after Phase 2 initialization.
*
*/
GT_STATUS cpssDxChDiagPortGroupMemRead
(
    IN GT_U8                          devNum,
    IN GT_PORT_GROUPS_BMP             portGroupsBmp,
    IN CPSS_DIAG_PP_MEM_TYPE_ENT      memType,
    IN GT_U32                         offset,
    OUT GT_U32                        *dataPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagPortGroupMemRead);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, memType, offset, dataPtr));

    rc = internal_cpssDxChDiagPortGroupMemRead(devNum, portGroupsBmp, memType, offset, dataPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, memType, offset, dataPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChDiagPortGroupRegsNumGet function
* @endinternal
*
* @brief   Gets the number of registers for the specific port group in the PP.
*         Used to allocate memory for cpssDxChDiagRegsDump.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Aldrin2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported,
*                                      in this case read is done from first active port group.
*
* @param[out] regsNumPtr               - (pointer to) number of registers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*
*/
static GT_STATUS internal_cpssDxChDiagPortGroupRegsNumGet
(
    IN  GT_U8               devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    OUT GT_U32              *regsNumPtr
)
{
    GT_U32 *regsListPtr;    /* holds all elements in PRV_CPSS_DXCH_DEV_REGS_MAC */
    GT_U32 regsListSize;    /* size of regsListPtr */
    GT_U32  portGroupId;    /*the port group Id - support multi-port-groups device */
    GT_STATUS   ret;        /* function return result */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(regsNumPtr);
    /* Get the first active port group id */
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(devNum, portGroupsBmp,
                                                        portGroupId);

    regsListPtr = (GT_U32*)PRV_CPSS_DXCH_DEV_REGS_MAC(devNum);
    regsListSize = sizeof(PRV_CPSS_DXCH_PP_REGS_ADDR_STC) / sizeof(GT_U32);
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* ASUMPTION : PRV_CPSS_DXCH_DEV_REGS_VER1_MAC() is continues to
                       PRV_CPSS_DXCH_DEV_REGS_MAC() */
        regsListSize += sizeof(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC) / sizeof(GT_U32);
    }

    ret = prvCpssDiagRegsNumGet (devNum, portGroupId, regsListPtr, regsListSize, 0,
                                 &checkIfRegisterExist, regsNumPtr);
    if (ret != GT_OK)
    {
        return ret;
    }

    return GT_OK;
}

/**
* @internal cpssDxChDiagPortGroupRegsNumGet function
* @endinternal
*
* @brief   Gets the number of registers for the specific port group in the PP.
*         Used to allocate memory for cpssDxChDiagRegsDump.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Aldrin2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported,
*                                      in this case read is done from first active port group.
*
* @param[out] regsNumPtr               - (pointer to) number of registers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*
*/
GT_STATUS cpssDxChDiagPortGroupRegsNumGet
(
    IN  GT_U8               devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    OUT GT_U32              *regsNumPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagPortGroupRegsNumGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, regsNumPtr));

    rc = internal_cpssDxChDiagPortGroupRegsNumGet(devNum, portGroupsBmp, regsNumPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, regsNumPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChDiagPortGroupRegsDump function
* @endinternal
*
* @brief   Dumps the device register addresses and values for the specific port
*         group in the PP.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Aldrin2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported,
*                                      in this case read is done from first active port group.
* @param[in,out] regsNumPtr            in: number of registers to dump. This number
*                                          must not be bigger than the number of registers
*                                          that can be dumped (starting at offset).
*                                      out: number of registers that were actually dumped
* @param[in] offset                   - The first register address to dump
* @param[out] regAddrPtr               - (pointer to) addresses of the dumped registers
* @param[out] regDataPtr               - (pointer to) data in the dumped registers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*       To dump all the registers the user may call cpssDxChDiagRegsNumGet in
*       order to get the number of registers of the PP.
*/
static GT_STATUS internal_cpssDxChDiagPortGroupRegsDump
(
    IN    GT_U8                 devNum,
    IN    GT_PORT_GROUPS_BMP    portGroupsBmp,
    INOUT GT_U32                *regsNumPtr,
    IN    GT_U32                offset,
    OUT   GT_U32                *regAddrPtr,
    OUT   GT_U32                *regDataPtr
)
{
    GT_U32 *regsListPtr;  /* holds all elements in PRV_CPSS_DXCH_DEV_REGS_MAC */
    GT_U32 regsListSize;  /* size of regsListPtr */
    GT_U32  portGroupId;    /*the port group Id - support multi-port-groups device */
    GT_STATUS   ret;        /* function return result */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(regsNumPtr);
    CPSS_NULL_PTR_CHECK_MAC(regAddrPtr);
    CPSS_NULL_PTR_CHECK_MAC(regDataPtr);
    /* Get the first active port group id */
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(devNum, portGroupsBmp,
                                                        portGroupId);

    regsListPtr = (GT_U32*)PRV_CPSS_DXCH_DEV_REGS_MAC(devNum);
    regsListSize = sizeof(PRV_CPSS_DXCH_PP_REGS_ADDR_STC) / sizeof(GT_U32);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* ASUMPTION : PRV_CPSS_DXCH_DEV_REGS_VER1_MAC() is continues to
                       PRV_CPSS_DXCH_DEV_REGS_MAC() */
        regsListSize += sizeof(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC) / sizeof(GT_U32);
    }
    ret = prvCpssDiagRegsDataGet (devNum, portGroupId, regsListPtr, regsListSize,
                                  offset, &checkIfRegisterExist, regsNumPtr,
                                  regAddrPtr, regDataPtr);
    if (ret != GT_OK)
    {
        return ret;
    }

    return GT_OK;
}

/**
* @internal cpssDxChDiagPortGroupRegsDump function
* @endinternal
*
* @brief   Dumps the device register addresses and values for the specific port
*         group in the PP.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Aldrin2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported,
*                                      in this case read is done from first active port group.
* @param[in,out] regsNumPtr            in: number of registers to dump. This number
*                                          must not be bigger than the number of registers
*                                          that can be dumped (starting at offset).
*                                      out: number of registers that were actually dumped
* @param[in] offset                   - The first register address to dump
* @param[out] regAddrPtr               - (pointer to) addresses of the dumped registers
* @param[out] regDataPtr               - (pointer to) data in the dumped registers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function may be called after Phase 1 initialization.
*       To dump all the registers the user may call cpssDxChDiagRegsNumGet in
*       order to get the number of registers of the PP.
*/
GT_STATUS cpssDxChDiagPortGroupRegsDump
(
    IN    GT_U8                 devNum,
    IN    GT_PORT_GROUPS_BMP    portGroupsBmp,
    INOUT GT_U32                *regsNumPtr,
    IN    GT_U32                offset,
    OUT   GT_U32                *regAddrPtr,
    OUT   GT_U32                *regDataPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagPortGroupRegsDump);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, regsNumPtr, offset, regAddrPtr, regDataPtr));

    rc = internal_cpssDxChDiagPortGroupRegsDump(devNum, portGroupsBmp, regsNumPtr, offset, regAddrPtr, regDataPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, regsNumPtr, offset, regAddrPtr, regDataPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal mvDeviceInfoGetDevType function
* @endinternal
*
* @brief   Provide type of requested device to optimizer
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
*
* @retval Unknown                  - on not existing or not applicable device.
* @retval device type              - otherwise
*/
MV_SERDES_TEST_DEV_TYPE mvDeviceInfoGetDevType
(
    IN GT_U32    devNum
)
{
    if(0 == PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
        return Unknown;

    if(0 == PRV_CPSS_DXCH_FAMILY_CHECK_MAC(devNum))
        return Unknown;

    if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)
        && ( PRV_CPSS_PP_MAC(devNum)->revision >= 2 ))
    {
        return LionB;
    }
    else if ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) ||
             (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E))
    {
        return Xcat;
    }
    else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E)
    {
        return BullsEye;
    }
    else
        return Unknown;
}

/**
* @internal internal_cpssDxChDiagSerdesTuningSystemInit function
* @endinternal
*
* @brief   Initialize Serdes optimization system. Allocates resources for algorithm
*         which will be released by cpssDxChDiagSerdesTuningSystemClose
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - PP device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*
* @note If same device used for TX and RX then call the
*       function just once, but if TX generated by
*       different device then run this API before the test
*       once on both devices.
*
*/
static GT_STATUS internal_cpssDxChDiagSerdesTuningSystemInit
(
    IN    GT_U8  devNum
)
{
    MV_SERDES_OPT_REPORT_TYPE  reportType;
    MV_SERDES_OPT_OS_FUNC_PTR  funcStruct;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E |
                                            CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

#ifdef OS_MALLOC_MEMORY_LEAKAGE_DBG
    funcStruct.osFreePtr    = myCpssOsFree;
    funcStruct.osMallocPtr  = myCpssOsMalloc;
#else /*! OS_MALLOC_MEMORY_LEAKAGE_DBG*/
    funcStruct.osFreePtr    = cpssOsFree;
    funcStruct.osMallocPtr  = cpssOsMalloc;
#endif /*! OS_MALLOC_MEMORY_LEAKAGE_DBG*/
    funcStruct.osMemCpyPtr  = cpssOsMemCpy;
    funcStruct.osMemSetPtr  = cpssOsMemSet;
    funcStruct.osPrintfPtr  = cpssOsPrintf;
    funcStruct.osRandPtr    = cpssOsRand;
    funcStruct.osSemBCreatePtr  = (MV_SOPT_OS_SIG_SEM_BIN_CREATE_FUNC)cpssOsSigSemBinCreate;
    funcStruct.osSemDelPtr      = cpssOsSigSemDelete;
    funcStruct.osSemSignalPtr   = cpssOsSigSemSignal;
    funcStruct.osSemWaitPtr = cpssOsSigSemWait;
    funcStruct.osSprintfPtr = cpssOsSprintf;
    funcStruct.osTimerWkPtr = cpssOsTimerWkAfter;

    if (mvSerdesTunningSystemInit(&funcStruct) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

    reportType = (GT_TRUE == diagSerdesTuningTracePrintEnable) ?  Verbose : Silence;
    mvSerdesTunningReportTypeSet(reportType);

    return GT_OK;
}

/**
* @internal cpssDxChDiagSerdesTuningSystemInit function
* @endinternal
*
* @brief   Initialize Serdes optimization system. Allocates resources for algorithm
*         which will be released by cpssDxChDiagSerdesTuningSystemClose
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - PP device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*
* @note If same device used for TX and RX then call the
*       function just once, but if TX generated by
*       different device then run this API before the test
*       once on both devices.
*
*/
GT_STATUS cpssDxChDiagSerdesTuningSystemInit
(
    IN    GT_U8  devNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagSerdesTuningSystemInit);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum));

    rc = internal_cpssDxChDiagSerdesTuningSystemInit(devNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChDiagSerdesTuningTracePrintEnable function
* @endinternal
*
* @brief   Enable/disable intermidiate reports print.
*         Status set per system not per device; devNum used just for applicable
*         device check
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - PP device number
* @param[in] enable                   - enable/disable intermidiate reports print
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChDiagSerdesTuningTracePrintEnable
(
    IN    GT_U8  devNum,
    IN    GT_BOOL enable
)
{
    MV_SERDES_OPT_REPORT_TYPE  reportType;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E |
                                            CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    diagSerdesTuningTracePrintEnable = enable;

    reportType = (GT_TRUE == diagSerdesTuningTracePrintEnable) ?  Verbose : Silence;

    mvSerdesTunningReportTypeSet(reportType);

    return GT_OK;
}

/**
* @internal cpssDxChDiagSerdesTuningTracePrintEnable function
* @endinternal
*
* @brief   Enable/disable intermidiate reports print.
*         Status set per system not per device; devNum used just for applicable
*         device check
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - PP device number
* @param[in] enable                   - enable/disable intermidiate reports print
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChDiagSerdesTuningTracePrintEnable
(
    IN    GT_U8  devNum,
    IN    GT_BOOL enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagSerdesTuningTracePrintEnable);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChDiagSerdesTuningTracePrintEnable(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChDiagSerdesTuningSystemClose function
* @endinternal
*
* @brief   Stop Tx if it was engaged and free all resources allocated by tuning
*         algorithm.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - PP device number
*
* @retval GT_OK                    - on success
* @retval GT_NOT_INITIALIZED       - if the system was not initialized
*
* @note If same device used for TX and RX then call the
*       function just once, but if TX generated by
*       different device then run this API before the test
*       once on both devices.
*
*/
static GT_STATUS internal_cpssDxChDiagSerdesTuningSystemClose
(
    IN    GT_U8  devNum
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E |
                                            CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    mvSerdesTunningStopTx();

    if(mvSerdesTunningSystemClose() != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);

    return GT_OK;
}

/**
* @internal cpssDxChDiagSerdesTuningSystemClose function
* @endinternal
*
* @brief   Stop Tx if it was engaged and free all resources allocated by tuning
*         algorithm.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - PP device number
*
* @retval GT_OK                    - on success
* @retval GT_NOT_INITIALIZED       - if the system was not initialized
*
* @note If same device used for TX and RX then call the
*       function just once, but if TX generated by
*       different device then run this API before the test
*       once on both devices.
*
*/
GT_STATUS cpssDxChDiagSerdesTuningSystemClose
(
    IN    GT_U8  devNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagSerdesTuningSystemClose);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum));

    rc = internal_cpssDxChDiagSerdesTuningSystemClose(devNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChDiagSerdesTuningRxTune function
* @endinternal
*
* @brief   Run the SerDes optimization algorithm and save its results in CPSS
*         internal SW DB for further usage in SerDes power up sequence.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - PP device number
* @param[in] portLaneArrPtr           - array of pairs (port;lane) where to run optimization
*                                      algorithm
* @param[in] portLaneArrLength        - number of pairs in portLaneArrPtr
* @param[in] prbsType                 - Type of PRBS used for test.
* @param[in] prbsTime                 - duration[ms] of the PRBS test during suggested setup
*                                      verification.
*                                      If (CPSS_DXCH_PORT_SERDES_TUNE_PRBS_TIME_DEFAULT_CNS
*                                      == prbsTime) then default
*                                      PRV_CPSS_DXCH_PORT_SERDES_TUNE_PRBS_DEFAULT_DELAY_CNS
*                                      will be used
* @param[in] optMode                  - optimization algorithm mode
*
* @param[out] optResultArrPtr          - Array of algorithm results, must be of length enough
*                                      to keep results for all tested lanes
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error, signal detect fail
* @retval GT_NOT_INITIALIZED       - if the system was not initialized
* @retval GT_BAD_PARAM             - if one of parameters is wrong
*
* @note If algorithm fails for some lane as result of HW problem appropriate
*       entry in optResultArrPtr will be set to 0xffffffff
*       Marvell recommends running the following PRBS type per interface type:
*       - SERDES using 8/10 bit encoding (DHX,QSGMII,HGS4,HGS,XAUI,QX,
*       SGMII 2.5,SGMII,100FX): PRBS7
*       - RXAUI: PRBS15
*       - SERDES using 64/66 bit encoding (XLG): PRBS31
*
*/
static GT_STATUS internal_cpssDxChDiagSerdesTuningRxTune
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_DIAG_SERDES_TUNE_PORT_LANE_STC    *portLaneArrPtr,
    IN  GT_U32                                      portLaneArrLength,
    IN  CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT            prbsType,
    IN  GT_U32                                      prbsTime,
    IN  CPSS_DXCH_DIAG_SERDES_OPTIMIZE_MODE_ENT     optMode,
    OUT CPSS_DXCH_PORT_SERDES_TUNE_STC              *optResultArrPtr
)
{
    GT_STATUS   rc; /* return code */
    GT_U32      i,  /* iterator */
                portNum,    /* number of port occupying lane */
                laneNum,    /* number of lane to test */
                startSerdes,    /* number of first serdes occupied by port */
                serdesNumPerPort,/* number of serdeses occupied by port */
                portGroupId;/* port group for multi-port-group devices */
    MV_SERDES_OPT_RESULTS serdesOptResult;/* resulting values algorithm calculated
                                            for given lane */
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode;
    MV_SERDES_OPT_MODE  testMode;
    MV_PRBS_TYPE mvPrbsType;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E |
                                            CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);
    CPSS_NULL_PTR_CHECK_MAC(portLaneArrPtr);
    CPSS_NULL_PTR_CHECK_MAC(optResultArrPtr);

    switch(prbsType)
    {
        case CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS7_E:
            mvPrbsType = PRBS_7;
            break;

        case CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS15_E:
            mvPrbsType = PRBS_15;
            break;

        case CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS23_E:
            mvPrbsType = PRBS_23;
            break;

        case CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS31_E:
            mvPrbsType = PRBS_31;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch(optMode)
    {
        case CPSS_DXCH_DIAG_SERDES_OPTIMIZE_MODE_ACCUR_AWARE_E:
            testMode = AccurAware;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* register lanes before test start */
    for(i = 0; i < portLaneArrLength; i++)
    {
        portNum = portLaneArrPtr[i].portNum;
        rc = cpssDxChPortInterfaceModeGet(devNum, portNum, &ifMode);
        if((rc != GT_OK) && (rc != GT_BAD_STATE))
            return rc;

        if((rc = prvCpssDxChPortIfModeSerdesNumGet(devNum, portNum, ifMode, &startSerdes,
                                                    &serdesNumPerPort)) != GT_OK)
        {
            return rc;
        }

        /* expect port to be configured to appropriate mode/speed before test run */
        if(portLaneArrPtr[i].laneNum >= serdesNumPerPort)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

        /* algorithm sees lanes as flat sequence */
        laneNum = startSerdes + portLaneArrPtr[i].laneNum;
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,portNum);
        rc = mvSerdesTunningRxLaneRegister(devNum, portGroupId, laneNum, mvPrbsType);
        if(rc != GT_OK)
        {
            if(diagSerdesTuningTracePrintEnable)
            {
                cpssOsPrintf("mvSerdesTunningRxLaneRegister fail:laneNum=%d,prbsType=%d\n",
                                laneNum, prbsType);
            }
            /* could be no memory, wrong dev type, problem in skip list */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    }

    if (CPSS_DXCH_DIAG_SERDES_TUNE_PRBS_TIME_DEFAULT_CNS == prbsTime)
        prbsTime = PRV_CPSS_DXCH_DIAG_SERDES_TUNE_PRBS_DEFAULT_DELAY_CNS;

    rc = mvSerdesTunningStart(testMode, prbsTime);
    if(rc != GT_OK)
    {/* at this moment just this error returned by serdes optimizer */
        if(diagSerdesTuningTracePrintEnable)
        {
            cpssOsPrintf("mvSerdesTunningStart fail:testMode=%d,prbsTime=%d\n",
                            testMode, prbsTime);
        }
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_MORE, LOG_ERROR_NO_MSG);
    }

    for(i = 0; i < portLaneArrLength; i++)
    {
        portNum = portLaneArrPtr[i].portNum;
        rc = cpssDxChPortInterfaceModeGet(devNum, portNum, &ifMode);
        if((rc != GT_OK) && (rc != GT_BAD_STATE))
            return rc;

        if((rc = prvCpssDxChPortIfModeSerdesNumGet(devNum, portNum, ifMode, &startSerdes,
                                                    &serdesNumPerPort)) != GT_OK)
        {
            return rc;
        }

        /* algorithm sees lanes as flat sequence */
        laneNum = startSerdes + portLaneArrPtr[i].laneNum;
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,portNum);
        serdesOptResult = mvSerdesTunningReport(devNum, portGroupId, laneNum);
        if(serdesOptResult.signalDetectFail || serdesOptResult.prbsNotLocked)
        {
            if(diagSerdesTuningTracePrintEnable)
            {
                cpssOsPrintf("mvSerdesTunningReport(devNum=%d,portGroupId=%d,laneNum=%d):\n",
                                devNum, portGroupId, laneNum);
                cpssOsPrintf("signalDetectFail=%d,prbsNotLocked=%d\n",
                                serdesOptResult.signalDetectFail,
                                serdesOptResult.prbsNotLocked);
            }
            cpssOsMemSet(&(optResultArrPtr[i]), 0xff,
                    sizeof(CPSS_DXCH_PORT_SERDES_TUNE_STC));
        }
        else
        {
            optResultArrPtr[i].dfe      = serdesOptResult.dfeBestVal;
            optResultArrPtr[i].ffeR     = serdesOptResult.ffeRBestVal;
            optResultArrPtr[i].ffeC     = serdesOptResult.ffeCBestVal;
            optResultArrPtr[i].sampler  = serdesOptResult.samplerCenter;
            optResultArrPtr[i].sqlch    = serdesOptResult.sqlchVal;
            if(diagSerdesTuningTracePrintEnable)
            {
                cpssOsPrintf("dfe=0x%x,ffeR=0x%x,ffeC=0x%x,sampler=0x%x,sqlch=0x%x\n",
                            optResultArrPtr[i].dfe,
                            optResultArrPtr[i].ffeR,
                            optResultArrPtr[i].ffeC,
                            optResultArrPtr[i].sampler,
                            optResultArrPtr[i].sqlch);
            }
        }
        rc = mvSerdesTunningRxLaneUnregister(devNum, portGroupId, laneNum);
        if(rc != GT_OK)
        {
            if(diagSerdesTuningTracePrintEnable)
            {
                cpssOsPrintf("mvSerdesTunningRxLaneUnregister fail:portGroupId=%d,laneNum=%d\n",
                            portGroupId, laneNum);
            }
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChDiagSerdesTuningRxTune function
* @endinternal
*
* @brief   Run the SerDes optimization algorithm and save its results in CPSS
*         internal SW DB for further usage in SerDes power up sequence.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - PP device number
* @param[in] portLaneArrPtr           - array of pairs (port;lane) where to run optimization
*                                      algorithm
* @param[in] portLaneArrLength        - number of pairs in portLaneArrPtr
* @param[in] prbsType                 - Type of PRBS used for test.
* @param[in] prbsTime                 - duration[ms] of the PRBS test during suggested setup
*                                      verification.
*                                      If (CPSS_DXCH_PORT_SERDES_TUNE_PRBS_TIME_DEFAULT_CNS
*                                      == prbsTime) then default
*                                      PRV_CPSS_DXCH_PORT_SERDES_TUNE_PRBS_DEFAULT_DELAY_CNS
*                                      will be used
* @param[in] optMode                  - optimization algorithm mode
*
* @param[out] optResultArrPtr          - Array of algorithm results, must be of length enough
*                                      to keep results for all tested lanes
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error, signal detect fail
* @retval GT_NOT_INITIALIZED       - if the system was not initialized
* @retval GT_BAD_PARAM             - if one of parameters is wrong
*
* @note If algorithm fails for some lane as result of HW problem appropriate
*       entry in optResultArrPtr will be set to 0xffffffff
*       Marvell recommends running the following PRBS type per interface type:
*       - SERDES using 8/10 bit encoding (DHX,QSGMII,HGS4,HGS,XAUI,QX,
*       SGMII 2.5,SGMII,100FX): PRBS7
*       - RXAUI: PRBS15
*       - SERDES using 64/66 bit encoding (XLG): PRBS31
*
*/
GT_STATUS cpssDxChDiagSerdesTuningRxTune
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_DIAG_SERDES_TUNE_PORT_LANE_STC    *portLaneArrPtr,
    IN  GT_U32                                      portLaneArrLength,
    IN  CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT            prbsType,
    IN  GT_U32                                      prbsTime,
    IN  CPSS_DXCH_DIAG_SERDES_OPTIMIZE_MODE_ENT     optMode,
    OUT CPSS_DXCH_PORT_SERDES_TUNE_STC              *optResultArrPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagSerdesTuningRxTune);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portLaneArrPtr, portLaneArrLength, prbsType, prbsTime, optMode, optResultArrPtr));

    rc = internal_cpssDxChDiagSerdesTuningRxTune(devNum, portLaneArrPtr, portLaneArrLength, prbsType, prbsTime, optMode, optResultArrPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portLaneArrPtr, portLaneArrLength, prbsType, prbsTime, optMode, optResultArrPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChDiagRegDefaultsEnableSet function
* @endinternal
*
* @brief   Set the initRegDefaults flag to allow system init with no HW writes.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] enable                   -   GT_TRUE  - Enable init with register defaults.
*                                      GT_FALSE - Normal init.
*
* @retval GT_OK                    - on success
*/
static GT_STATUS internal_cpssDxChDiagRegDefaultsEnableSet
(
    IN  GT_BOOL     enable
)
{
    dxChInitRegDefaults = (enable == GT_FALSE)?GT_FALSE:GT_TRUE;
    return GT_OK;
}

/**
* @internal cpssDxChDiagRegDefaultsEnableSet function
* @endinternal
*
* @brief   Set the initRegDefaults flag to allow system init with no HW writes.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] enable                   -   GT_TRUE  - Enable init with register defaults.
*                                      GT_FALSE - Normal init.
*
* @retval GT_OK                    - on success
*/
GT_STATUS cpssDxChDiagRegDefaultsEnableSet
(
    IN  GT_BOOL     enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagRegDefaultsEnableSet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, enable));

    rc = internal_cpssDxChDiagRegDefaultsEnableSet(enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, enable));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChDiagFastBootSkipOwnDeviceInitEnableSet function
* @endinternal
*
* @brief   Set the fastBootSkipOwnDeviceInit flag to allow system init with no HW write
*         to the device Device_ID within a Prestera chipset.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] enable                   -   GT_TRUE  - Enable init with no HW write to the device Device_ID.
*                                      GT_FALSE - Normal init.
*
* @retval GT_OK                    - on success
*/
static GT_STATUS internal_cpssDxChDiagFastBootSkipOwnDeviceInitEnableSet
(
    IN  GT_BOOL     enable
)
{
    dxChFastBootSkipOwnDeviceInit = (enable == GT_FALSE)?GT_FALSE:GT_TRUE;
    return GT_OK;
}

/**
* @internal cpssDxChDiagFastBootSkipOwnDeviceInitEnableSet function
* @endinternal
*
* @brief   Set the fastBootSkipOwnDeviceInit flag to allow system init with no HW write
*         to the device Device_ID within a Prestera chipset.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] enable                   -   GT_TRUE  - Enable init with no HW write to the device Device_ID.
*                                      GT_FALSE - Normal init.
*
* @retval GT_OK                    - on success
*/
GT_STATUS cpssDxChDiagFastBootSkipOwnDeviceInitEnableSet
(
    IN  GT_BOOL     enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagFastBootSkipOwnDeviceInitEnableSet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, enable));

    rc = internal_cpssDxChDiagFastBootSkipOwnDeviceInitEnableSet(enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, enable));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal prvCpssDxChDiagBistStageStartSet function
* @endinternal
*
* @brief   Start Stop MC BIST stage for memory set operation with pattern defined
*         by opcode.
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - PP device number
* @param[in] opCode                   - BIST stage opcode may be one of:
*                                      set RAM with pattern 0x00 - opCode = 0x0008
*                                      set RAM with pattern 0x55 - opCode = 0x0018
*                                      set RAM with pattern 0xAA - opCode = 0x0028
*                                      set RAM with pattern 0xFF - opCode = 0x0038
* @param[in] enable                   - GT_TRUE:  Enable and Start.
*                                      GT_FALSE: Disable and Stop.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
*/
static GT_STATUS prvCpssDxChDiagBistStageStartSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  opCode,
    IN  GT_BOOL enable
)
{
    GT_STATUS rc;        /* return code           */
    GT_U32    regAddr;   /* address of register   */
    GT_U32    fieldData; /* field's data to write */

    /* Before enable BIST make sure that:
       - Client mode is Register access but not BIST
       - BIST with OpCode are disabled
       - OpCode operation is disabled.
       This is mandatory because BIST enabled by default. */

    /* Set Client mode to be Register access (BIST disable - normal mode). */
    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                                                 DFXClientUnits.clientControl;
    fieldData = 0;
    rc = prvCpssDfxClientSetRegField(
            devNum, 0, PRV_CPSS_DFX_CLIENT_MC_CNS, regAddr, 0, 2, fieldData);
    if( GT_OK != rc )
    {
        return rc;
    }

    /* Disable BIST with OpCode */
    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                                                 DFXClientUnits.BISTControl;
    fieldData = 0x2010;
    rc = prvCpssDfxClientSetRegField(
            devNum, 0, PRV_CPSS_DFX_CLIENT_MC_CNS, regAddr, 0, 23, fieldData);
    if( GT_OK != rc )
    {
        return rc;
    }

    /* stop OpCode operation */
    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                                                 DFXClientUnits.BISTOpCode;
    fieldData = 0x0;
    rc = prvCpssDfxClientSetRegField(
            devNum, 0, PRV_CPSS_DFX_CLIENT_MC_CNS,regAddr, 0, 17, fieldData);
    if( GT_OK != rc )
    {
        return rc;
    }

    if (enable == GT_FALSE)
    {
        /* the code above disables BIST */
        return GT_OK;
    }

    /* Set Client mode to be BIST (enable - set memory)*/
    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                                                 DFXClientUnits.clientControl;
    fieldData = 1;
    rc = prvCpssDfxClientSetRegField(
            devNum, 0, PRV_CPSS_DFX_CLIENT_MC_CNS, regAddr, 0, 2, fieldData);
    if( GT_OK != rc )
    {
        return rc;
    }

    /* Enable BIST with OpCode enabled */
    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                                                 DFXClientUnits.BISTControl;
    fieldData = 0x2013;
    rc = prvCpssDfxClientSetRegField(
            devNum, 0, PRV_CPSS_DFX_CLIENT_MC_CNS, regAddr, 0, 23, fieldData);
    if( GT_OK != rc )
    {
        return rc;
    }

    /* start OpCode operation according to opCode */
    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                                                 DFXClientUnits.BISTOpCode;
    fieldData = (opCode << 1) | 1;
    rc = prvCpssDfxClientSetRegField(
            devNum, 0, PRV_CPSS_DFX_CLIENT_MC_CNS,regAddr, 0, 17, fieldData);

    return rc;
}

/* constant for not valid DFX pipe number */
#define DFX_NOT_VALID_PIPE_CNS 0xFFFFFFFF

/**
* @internal prvCpssDxChDiagBistMsysClientGet function
* @endinternal
*
* @brief   Get pipe and client number for MSYS DFX unit.
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - PP device number
*
* @param[out] pipePtr                  - (pointer to) DFX pipe number of MSYS unit
* @param[out] clientPtr                - (pointer to) DFX client number of MSYS unit
*                                       none
*/
GT_VOID prvCpssDxChDiagBistMsysClientGet
(
    IN  GT_U8       devNum,
    OUT GT_U32      *pipePtr,
    OUT GT_U32      *clientPtr
)
{
    switch(PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
            if (PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E)
            {/* caelum devices */
                *pipePtr = 1;
                *clientPtr = 5;
            }
            else
            {
                if (!PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
                {/* Bobcat2 A0 */
                    *pipePtr = 1;
                    *clientPtr = 0;
                }
                else
                {/* Bobcat2 B0 */
                    *pipePtr = 2;
                    *clientPtr = 0;
                }
            }
            break;
        case CPSS_PP_FAMILY_DXCH_XCAT3_E:
                *pipePtr = 1;
                *clientPtr = 0;
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
        case CPSS_PP_FAMILY_DXCH_AC3X_E:
                *pipePtr = 0;
                *clientPtr = 1;
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
                *pipePtr = 1;
                *clientPtr = 21;
            break;
        default: *pipePtr = *clientPtr = DFX_NOT_VALID_PIPE_CNS; break;
    }

    return;
}

/* list of CPU and MSYS related RAM's DFX client information */
static PRV_CPSS_DIAG_DATA_INTEGRITY_DFX_FIRST_STAGE_MAPPING_DATA_STC bc2A0CpuRamListArr[] = {{1,0},{1,19},{1,20},{1,21},{1,22},{1,23},{1,24}};
static PRV_CPSS_DIAG_DATA_INTEGRITY_DFX_FIRST_STAGE_MAPPING_DATA_STC bc2CpuRamListArr[] = {{2,0},{2,1},{2,2},{2,3},{2,4},{2,5},{2,6}};
static PRV_CPSS_DIAG_DATA_INTEGRITY_DFX_FIRST_STAGE_MAPPING_DATA_STC caelumCpuRamListArr[] = {{1,1}, {1,2}, {1,3}, {1,4}, {1,5}, {2,0},{2,1}};
static PRV_CPSS_DIAG_DATA_INTEGRITY_DFX_FIRST_STAGE_MAPPING_DATA_STC xcat3CpuRamListArr[] = {{1,0},{1,1},{1,2},{1,3},{1,4},{1,5},{1,6}};
static PRV_CPSS_DIAG_DATA_INTEGRITY_DFX_FIRST_STAGE_MAPPING_DATA_STC aldrinCpuRamListArr[] = {{0,1}};
static PRV_CPSS_DIAG_DATA_INTEGRITY_DFX_FIRST_STAGE_MAPPING_DATA_STC aldrin2CpuRamListArr[] = {{1,21}};

/* {2,0} is  mgmt-client  (PEX/MBUS memories are here). Device stuck if don't exclude it */
static PRV_CPSS_DIAG_DATA_INTEGRITY_DFX_FIRST_STAGE_MAPPING_DATA_STC bobcat3SkipRamListArr[] = {{2,0}, {2,18}};

/* list of TM Clock related RAM's DFX client information */
static PRV_CPSS_DIAG_DATA_INTEGRITY_DFX_FIRST_STAGE_MAPPING_DATA_STC bc2A0TmClockRamListArr[]  = {{0,15},{0,16},{0,18},{0,19},{0,21},{0,22},{1,4},{1,5},{1,6}};
static PRV_CPSS_DIAG_DATA_INTEGRITY_DFX_FIRST_STAGE_MAPPING_DATA_STC bc2TmClockRamListArr[]    = {{0,15},{0,16},{0,18},{0,19},{0,21},{0,22},{1,3},{1,4},{1,5}};
static PRV_CPSS_DIAG_DATA_INTEGRITY_DFX_FIRST_STAGE_MAPPING_DATA_STC caelumTmClockRamListArr[] = {{0,9},{0,11},{0,12},{0,14},{0,15},{3,19},{3,20}};

/*******************************************************************************
* prvCpssDxChDiagBistCpuClientsListGet
*
* DESCRIPTION:
*       Get pipe and client list for MSYS and CPU related RAMs.
*
* APPLICABLE DEVICES:
*       Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* NOT APPLICABLE DEVICES:
*       DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* INPUTS:
*       devNum        - PP device number
*
* OUTPUTS:
*       sizePtr       - (pointer to) size of list
*
* RETURNS:
*       pointer to list of DFX clients. Returns NULL for not supported device.
*
* COMMENTS:
*
******************************************************************************/
static PRV_CPSS_DIAG_DATA_INTEGRITY_DFX_FIRST_STAGE_MAPPING_DATA_STC * prvCpssDxChDiagBistCpuClientsListGet
(
    IN  GT_U8       devNum,
    OUT  GT_U32    *sizePtr
)
{
    PRV_CPSS_DIAG_DATA_INTEGRITY_DFX_FIRST_STAGE_MAPPING_DATA_STC * cpuRamListPtr; /* pointer to CPU clients */
    GT_U32 listSize;    /* size of CPU clients list */

    /* assign default values */
    cpuRamListPtr = NULL;
    listSize = 0;

    switch (PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
            if (!PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
            {
                cpuRamListPtr = bc2A0CpuRamListArr;
                listSize = sizeof(bc2A0CpuRamListArr) / sizeof(bc2A0CpuRamListArr[0]);
            }
            else if (PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_NONE_E)
            {
                /* Bobcat2 B0 */
                cpuRamListPtr = bc2CpuRamListArr;
                listSize = sizeof(bc2CpuRamListArr) / sizeof(bc2CpuRamListArr[0]);
            }
            else
            {
                /* Caelum devices */
                cpuRamListPtr = caelumCpuRamListArr;
                listSize = sizeof(caelumCpuRamListArr) / sizeof(caelumCpuRamListArr[0]);
            }
            break;
        case CPSS_PP_FAMILY_DXCH_XCAT3_E:
            /* xCat3 devices */
            cpuRamListPtr = xcat3CpuRamListArr;
            listSize = sizeof(xcat3CpuRamListArr) / sizeof(xcat3CpuRamListArr[0]);
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
        case CPSS_PP_FAMILY_DXCH_AC3X_E:
            /* Aldrin devices */
            cpuRamListPtr = aldrinCpuRamListArr;
            listSize = sizeof(aldrinCpuRamListArr) / sizeof(aldrinCpuRamListArr[0]);
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            /* Aldrin2 devices */
            cpuRamListPtr = aldrin2CpuRamListArr;
            listSize = sizeof(aldrin2CpuRamListArr) / sizeof(aldrin2CpuRamListArr[0]);
            break;

        default: /* other families are not supported yet */
            break;
    }

    *sizePtr = listSize;
    return cpuRamListPtr;
}

/*******************************************************************************
* prvCpssDxChDiagBistTmClockClientsListGet
*
* DESCRIPTION:
*       Get pipe and client list for TM Clock related RAMs.
*
* APPLICABLE DEVICES:
*       Bobcat2; Caelum.
*
* NOT APPLICABLE DEVICES:
*       DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* INPUTS:
*       devNum        - PP device number
*
* OUTPUTS:
*       sizePtr       - (pointer to) size of list
*
* RETURNS:
*       pointer to list of DFX clients. Returns NULL for not supported device.
*
* COMMENTS:
*
******************************************************************************/
static PRV_CPSS_DIAG_DATA_INTEGRITY_DFX_FIRST_STAGE_MAPPING_DATA_STC * prvCpssDxChDiagBistTmClockClientsListGet
(
    IN  GT_U8       devNum,
    OUT  GT_U32    *sizePtr
)
{
    PRV_CPSS_DIAG_DATA_INTEGRITY_DFX_FIRST_STAGE_MAPPING_DATA_STC * clientsListPtr; /* pointer to DFX clients */
    GT_U32 listSize;    /* size of DFX clients list */

    /* assign default values */
    clientsListPtr = NULL;
    listSize = 0;

    switch (PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
            if (!PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
            {
                clientsListPtr = bc2A0TmClockRamListArr;
                listSize = sizeof(bc2A0TmClockRamListArr) / sizeof(bc2A0TmClockRamListArr[0]);
            }
            else if (PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_NONE_E)
            {
                /* Bobcat2 B0 */
                clientsListPtr = bc2TmClockRamListArr;
                listSize = sizeof(bc2TmClockRamListArr) / sizeof(bc2TmClockRamListArr[0]);
            }
            else
            {
                /* Caelum devices */
                clientsListPtr = caelumTmClockRamListArr;
                listSize = sizeof(caelumTmClockRamListArr) / sizeof(caelumTmClockRamListArr[0]);
            }
            break;
        default: /* other families do not have TM */
            break;
    }

    *sizePtr = listSize;
    return clientsListPtr;
}

/**
* @internal prvCpssDxChDiagBistCheckSkipPipe function
* @endinternal
*
* @brief   Check skip of DFX pipe for BIST
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - PP device number
* @param[in] pipe                     - DFX pipe number
*
* @retval GT_TRUE                  - skip BIST
* @retval GT_FALSE                 - does not skip BIST
*/
GT_BOOL prvCpssDxChDiagBistCheckSkipPipe
(
    IN  GT_U8       devNum,
    IN  GT_U32      pipe
)
{
    /* skip not existing in Cetus clients */
    if ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E) &&
        (PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E))
    {
        switch(PRV_CPSS_PP_MAC(devNum)->devType)
        {
            case CPSS_BOBK_CETUS_DEVICES_CASES_MAC:
                /* both Caelum and Cetus use same DB. DFX Pipe 3 is not exist in Cetus. */
                if (pipe == 3)
                {
                    return GT_TRUE;
                }
                break;
            default:
                break;
        }

    }

    return GT_FALSE;
}

/**
* @internal prvCpssDxChDiagBistCheckSkipOptionalClient function
* @endinternal
*
* @brief   Check skip of optional DFX client for BIST
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - PP device number
* @param[in] pipe                     - DFX pipe number
* @param[in] client                   - DFX client number
* @param[in] skipCpuMemory            - skip internal CPU related memories from output
*
* @retval GT_TRUE                  - skip BIST
* @retval GT_FALSE                 - does not skip BIST
*/
static GT_BOOL prvCpssDxChDiagBistCheckSkipOptionalClient
(
    IN  GT_U8       devNum,
    IN  GT_U32      pipe,
    IN  GT_U32      client,
    IN  GT_BOOL     skipCpuMemory
)
{
    PRV_CPSS_DIAG_DATA_INTEGRITY_DFX_FIRST_STAGE_MAPPING_DATA_STC * clientsListPtr; /* pointer to DFX clients list */
    GT_U32 listSize;    /* size of DFX clients list */
    GT_U32 ii;          /* iterator */
    GT_U32 msysPipe;    /* MSYS DFX Pipe number   */
    GT_U32 msysClient;  /* MSYS DFX Client number */

    /* check skip DFX pipe */
    if (prvCpssDxChDiagBistCheckSkipPipe(devNum,pipe) == GT_TRUE)
    {
        return GT_TRUE;
    }

    if (skipCpuMemory)
    {
        /* get list of CPU clients including MSYS */
        clientsListPtr =  prvCpssDxChDiagBistCpuClientsListGet(devNum, &listSize);
        if (clientsListPtr)
        {
            for (ii = 0; ii < listSize; ii++)
            {
                if ((pipe == clientsListPtr[ii].dfxPipeIndex) && (client == clientsListPtr[ii].dfxClientIndex))
                {
                    return GT_TRUE;
                }
            }
        }
    }
    else
    {
        /* anyway skip MSYS client to avoid PEX/MBUS stuck during BIST */
        prvCpssDxChDiagBistMsysClientGet(devNum,&msysPipe,&msysClient);
        if((msysPipe == pipe) && (msysClient == client))
        {
            return GT_TRUE;
        }
    }

    /* skip TM Clock clients for devices with disabled TM */
    if((PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.trafficManager.supported == GT_TRUE) &&
       (PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.featureInfo.TmSupported == GT_FALSE))
    {
        /* get list of TM Clock clients */
        clientsListPtr =  prvCpssDxChDiagBistTmClockClientsListGet(devNum, &listSize);
        if (clientsListPtr)
        {
            for (ii = 0; ii < listSize; ii++)
            {
                if ((pipe == clientsListPtr[ii].dfxPipeIndex) && (client == clientsListPtr[ii].dfxClientIndex))
                {
                    return GT_TRUE;
                }
            }
        }
    }

    /* skipe mgmt-client in bobcat3 to avoid PEX/BMUS stuck during BIST */
    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
    {
        clientsListPtr = bobcat3SkipRamListArr;
        for (ii=0; ii < sizeof(bobcat3SkipRamListArr) / sizeof(bobcat3SkipRamListArr[0]); ii++)
        {
            if ((pipe == clientsListPtr[ii].dfxPipeIndex) &&
                (client == clientsListPtr[ii].dfxClientIndex))
            {
                return GT_TRUE;
            }
        }
    }

    return GT_FALSE;
}


/**
* @internal prvCpssDxChDiagBistCheckSkipClient function
* @endinternal
*
* @brief   Check skip of DFX client for BIST
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - PP device number
* @param[in] pipe                     - DFX pipe number
* @param[in] client                   - DFX client number
*
* @retval GT_TRUE                  - skip BIST
* @retval GT_FALSE                 - does not skip BIST
*/
GT_BOOL prvCpssDxChDiagBistCheckSkipClient
(
    IN  GT_U8       devNum,
    IN  GT_U32      pipe,
    IN  GT_U32      client
)
{
    /* skip CPU memories for BIST */
    return prvCpssDxChDiagBistCheckSkipOptionalClient(devNum,pipe,client,GT_TRUE);
}

/**
* @internal prvCpssDxChDiagBistSummaryDataLoggingEnableSet function
* @endinternal
*
* @brief   Enable/disable summary of all clients BIST operations in single snoop
*         bus status.
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - PP device number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS prvCpssDxChDiagBistSummaryDataLoggingEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_BOOL     enable
)
{
    GT_STATUS rc;   /* return status */

    if (enable == GT_TRUE)
    {
        /* Enable_summary_data_logging - accumulate all bits */
        rc = prvCpssDfxClientSetRegField(
                devNum,
                0, PRV_CPSS_DFX_CLIENT_MC_CNS,
                PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                                                   DFXClientUnits.clientDataControl,
                0, 5, 0x1F);
        if( GT_OK != rc )
        {
            return rc;
        }

        /* Start enable_summary_data_logging */
        rc = prvCpssDfxClientSetRegField(
                devNum,
                0, PRV_CPSS_DFX_CLIENT_MC_CNS,
                PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                                                     DFXClientUnits.clientControl,
                3, 1, 1);
        if( GT_OK != rc )
        {
            return rc;
        }

        /* transactions above use MC messages. It's takes some time to update state
           machines inside clients. Provide some time to guarantee that all clients
           are configured before activate logging. */
        cpssOsTimerWkAfter(1);

        /* Dummy write to Data Logging client to activate the data logging */
        rc = prvCpssDfxClientSetRegField(
                devNum,
                0, PRV_CPSS_DFX_CLIENT_LOG_CNS,
                PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                                                     DFXClientUnits.dummyWrite,
                0, 32, 0x00000000);

    }
    else
    {
        /* Stop enable_summary_data_logging */
        rc = prvCpssDfxClientSetRegField(
                devNum,
                0, PRV_CPSS_DFX_CLIENT_MC_CNS,
                PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                                                     DFXClientUnits.clientControl,
                3, 1, 0);
    }

    return rc;
}

/**
* @internal prvCpssDxChDiagBistMiscConfigSet function
* @endinternal
*
* @brief   Set miscellaneous configurations before BIST operations.
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - PP device number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS prvCpssDxChDiagBistMiscConfigSet
(
    IN  GT_U8                                       devNum
)
{
    GT_U32 waClients[][2] = {{0,4}, {0,15}, {0,1}, {0,11}, {0,22}, {0,24}, {1,0}};
                                                    /* DFX client ids in WA */
    GT_U32 ii;                                      /* loop iterator */
    GT_STATUS rc;                                   /* return status */
    GT_U32    regAddr;                              /* register's address */

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
    {
        /* there is no special configuration for xCat3 */
        return GT_OK;
    }

    if (!PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
    {
        /* Bobcat2 A0 WA for DFX bug in the BIST operation: */
        /* During BIST there are read during write checks, due to a bug some of */
        /* these checkings are disabled */
        for ( ii = 0 ; ii < (sizeof(waClients)/sizeof(GT_U32)/2) ; ii++ )
        {
            if (prvCpssDxChDiagBistCheckSkipClient(devNum, waClients[ii][0], waClients[ii][1]))
                continue;

            /* removed MUX level */
            regAddr =  PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                                                      DFXClientUnits.BISTMaxAddress;

            rc = prvCpssDfxClientSetRegField(devNum, waClients[ii][0], waClients[ii][1],
                                             regAddr, 28, 2, 0);
            if( GT_OK != rc )
            {
                return rc;
            }
        }
    }

    return GT_OK;
}


/**
* @internal prvCpssDxChDiagBistClientsListBuild function
* @endinternal
*
* @brief   Function creates list of applicable DFX clients for BIST operation.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - PP device number
* @param[in] skipCpuMemory            - skip internal CPU related memories from output
*
* @param[out] dfxClientsBmpArr[PRV_CPSS_DXCH_DFX_MAX_PIPES_CNS] - array of DFX clients bitmaps, index is pipe
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail of algorithm
*/
static GT_STATUS prvCpssDxChDiagBistClientsListBuild
(
    IN  GT_U8    devNum,
    IN  GT_BOOL  skipCpuMemory,
    OUT GT_U32   dfxClientsBmpArr[PRV_CPSS_DFX_MAX_PIPES_CNS]
)
{
    GT_U32 pipe;    /* DFX pipe id */
    GT_U32 client;  /* DFX client id */
    GT_U32 ii;      /* loop iterator */
    PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC    *dbArrayPtr; /* pointer to data integrity DB */
    GT_U32                                          dbArrayEntryNum; /* size of data integrity DB */

    /* assign DB pointer and size */
    prvCpssDxChDiagDataIntegrityDbPointerSet(devNum, &dbArrayPtr, &dbArrayEntryNum);
    if ((dbArrayPtr == NULL) || (dbArrayEntryNum == 0))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    cpssOsMemSet(dfxClientsBmpArr, 0, PRV_CPSS_DFX_MAX_PIPES_CNS* sizeof(GT_U32));

    for( ii = 0 ; ii < dbArrayEntryNum ; ii++ )
    {
        pipe = PRV_CPSS_DIAG_DATA_INTEGRITY_MAP_KEY_PIPE_GET(dbArrayPtr[ii].key);
        client = PRV_CPSS_DIAG_DATA_INTEGRITY_MAP_KEY_CLIENT_GET(dbArrayPtr[ii].key);

        if (pipe >= PRV_CPSS_DFX_MAX_PIPES_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        if (client >= PRV_CPSS_DFX_MAX_CLIENTS_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        if (dfxClientsBmpArr[pipe] & (1 << client))
        {
            /* client is already in list */
            continue;
        }

        if (prvCpssDxChDiagBistCheckSkipOptionalClient(devNum, pipe, client,skipCpuMemory))
            continue;

        dfxClientsBmpArr[pipe] |= (1 << client);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChDiagBistClientsListDump function
* @endinternal
*
* @brief   Function dumps list of applicable DFX clients for BIST operation.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - PP device number
* @param[in] skipCpuMemory            - skip internal CPU related memories from output
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail of algorithm
*/
GT_STATUS prvCpssDxChDiagBistClientsListDump
(
    IN  GT_U8    devNum,
    IN  GT_BOOL  skipCpuMemory
)
{
    GT_U32   dfxClientsBmpArr[PRV_CPSS_DFX_MAX_PIPES_CNS];
    GT_U32   ii;
    GT_STATUS rc;

    rc = prvCpssDxChDiagBistClientsListBuild(devNum,skipCpuMemory,dfxClientsBmpArr);
    if (rc != GT_OK)
    {
        return rc;
    }

    cpssOsPrintf("DFX Clients for BIST:\n");
    for (ii=0; ii < PRV_CPSS_DFX_MAX_PIPES_CNS; ii++)
    {
        cpssOsPrintf("Pipe %d: 0x%08X\n", ii, dfxClientsBmpArr[ii]);
    }

    return rc;
}

/**
* @internal prvCpssDxChDiagBistMcDistribByDbSet function
* @endinternal
*
* @brief   Configure all relevant clients to be in the multicast distribution list.
*         Configuration done according to memory map DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2; Lion; Lion2; xCat3.
*
* @param[in] devNum                   - PP device number
* @param[in] skipCpuMemory            - skip internal CPU related memories from MC distribution
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS prvCpssDxChDiagBistMcDistribByDbSet
(
    IN  GT_U8    devNum,
    IN  GT_BOOL  skipCpuMemory
)
{
    /* array of DFX clients bitmaps, index is pipe */
    GT_U32   dfxClientsBmpArr[PRV_CPSS_DFX_MAX_PIPES_CNS];
    GT_U32   regAddr;  /* register's address     */
    GT_U32   pipe;     /* DFX pipe               */
    GT_U32   client;   /* DFX client             */
    GT_STATUS rc;      /* return code            */

    rc = prvCpssDxChDiagBistClientsListBuild(devNum,skipCpuMemory,dfxClientsBmpArr);
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                                             DFXClientUnits.clientControl;

    for (pipe = 0; pipe < PRV_CPSS_DFX_MAX_PIPES_CNS; pipe++)
    {
        if (dfxClientsBmpArr[pipe] == 0)
        {
            /* there are no clients on the pipe */
            continue;
        }

        for (client = 0; client < PRV_CPSS_DFX_MAX_CLIENTS_CNS; client++)
        {
            if (dfxClientsBmpArr[pipe] & (1 << client))
            {
                /* configure client */
                rc = prvCpssDfxClientSetRegField(
                        devNum, pipe, client, regAddr, 6, 1, 1);
                if( GT_OK != rc )
                {
                    return rc;
                }
            }
        }
    }

    return rc;
}

/**
* @internal prvCpssDxChDiagBistMcDistribSet function
* @endinternal
*
* @brief   Configure all relevant clients to be in the multicast distribution list.
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - PP device number
* @param[in] skipCpuMemory            - skip internal CPU related memories from MC distribution
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS prvCpssDxChDiagBistMcDistribSet
(
    IN  GT_U8   devNum,
    IN  GT_BOOL skipCpuMemory
)
{
    GT_STATUS rc;       /* return status          */
    GT_U32 msysPipe;    /* MSYS DFX Pipe number   */
    GT_U32 msysClient;  /* MSYS DFX Client number */
    GT_U32 regAddr;     /* register's address     */
    PRV_CPSS_DIAG_DATA_INTEGRITY_DFX_FIRST_STAGE_MAPPING_DATA_STC * cpuRamListPtr; /* pointer to CPU clients */
    GT_U32 listSize;    /* size of CPU clients list */
    GT_U32 ii;          /* iterator */
    GT_U32 useDbForMcSet;

    /* configure pipe#0 and client#0 as reference one for MC access in prvCpssDfxClientSetRegField */
    PRV_CPSS_PP_MAC(devNum)->diagInfo.dfxMcAccessGetPipe = 0;
    PRV_CPSS_PP_MAC(devNum)->diagInfo.dfxMcAccessGetClient = 0;

    /* xCat3 does not have DB and may use usual DFX BC configuration */
    useDbForMcSet = PRV_CPSS_SIP_5_CHECK_MAC(devNum);

    if (useDbForMcSet)
    {
        rc = prvCpssDxChDiagBistMcDistribByDbSet(devNum,skipCpuMemory);
        if( GT_OK != rc )
        {
            return rc;
        }
    }
    else
    {

        /* Enable all clients to be in the multicast distribution group */
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                                                 DFXClientUnits.clientControl;
        rc = prvCpssDfxClientSetRegField(
                devNum, 0, PRV_CPSS_DFX_CLIENT_BC_CNS, regAddr, 6, 1, 1);
        if( GT_OK != rc )
        {
            return rc;
        }

        prvCpssDxChDiagBistMsysClientGet(devNum,&msysPipe,&msysClient);

        if( msysPipe != DFX_NOT_VALID_PIPE_CNS)
        {
            /* Remove MSYS from MC distribution. MSYS client has PEX and MBUS
               memories those cannot be changed because device became not accessible
               by CPU. */
            rc = prvCpssDfxClientSetRegField(devNum, msysPipe, msysClient,regAddr,
                                             6, 1, 0);
            if( GT_OK != rc )
            {
                return rc;
            }
        }

        if (skipCpuMemory)
        {
            /* get list of CPU clients including MSYS */
            cpuRamListPtr = prvCpssDxChDiagBistCpuClientsListGet(devNum, &listSize);
            if (cpuRamListPtr != NULL)
            {
                for (ii = 0; ii < listSize; ii++)
                {
                    /* Remove clients from MC distribution.*/
                    rc = prvCpssDfxClientSetRegField(devNum, cpuRamListPtr[ii].dfxPipeIndex,
                                                     cpuRamListPtr[ii].dfxClientIndex, regAddr,
                                                     6, 1, 0);
                    if( GT_OK != rc )
                    {
                        return rc;
                    }
                }
            }
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChDiagBistMcDistribForOneClientSet function
* @endinternal
*
* @brief   Configure only one client to be in the multicast distribution list.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - PP device number
* @param[in] specificPipe             - specific DFX pipe
* @param[in] specificClient           - specific DFX client
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS prvCpssDxChDiagBistMcDistribForOneClientSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  specificPipe,
    IN  GT_U32  specificClient
)
{
    GT_STATUS rc;       /* return status          */
    GT_U32 regAddr;     /* register's address     */

    /* configure pipe and client as reference one for MC access in prvCpssDfxClientSetRegField */
    PRV_CPSS_PP_MAC(devNum)->diagInfo.dfxMcAccessGetPipe = specificPipe;
    PRV_CPSS_PP_MAC(devNum)->diagInfo.dfxMcAccessGetClient = specificClient;

    /* disable all clients in the multicast distribution group */
    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                                             DFXClientUnits.clientControl;

    rc = prvCpssDfxClientSetRegField(
            devNum, 0, PRV_CPSS_DFX_CLIENT_BC_CNS, regAddr, 6, 1, 0);
    if( GT_OK != rc )
    {
        return rc;
    }

    /* enable specific client to be in MC distribution group */
    rc = prvCpssDfxClientSetRegField(devNum, specificPipe, specificClient,regAddr,
                                     6, 1, 1);
    return rc;
}

/**
* @internal prvCpssDxChDiagBistStatusCheck function
* @endinternal
*
* @brief  Check BIST status of all relevant clients.
*         Configuration done according to memory map DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2; Lion; Lion2; xCat3.
*
* @param[in] devNum                   - PP device number
* @param[in] dfxClientsBmpArr[PRV_CPSS_DFX_MAX_PIPES_CNS] - array of DFX clients bitmaps, index is pipe
*
* @param[out] resultsStatusPtr        - (pointer to) BIST results status
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS prvCpssDxChDiagBistStatusCheck
(
    IN  GT_U8    devNum,
    IN  GT_U32   dfxClientsBmpArr[],
    OUT CPSS_DXCH_DIAG_BIST_STATUS_ENT  *resultsStatusPtr
)
{
    GT_STATUS                           rc;                         /* return code          */
    PRV_CPSS_DFX_DIAG_BIST_STATUS_ENT   prvResultsStatus;           /* generic result result */


    /* call generic BIST status check */
    rc = prvCpssDfxDiagBistStatusCheck(devNum, dfxClientsBmpArr, &prvResultsStatus);
    if (GT_OK != rc)
    {
        return rc;
    }

    switch (prvResultsStatus)
    {
        case PRV_CPSS_DFX_DIAG_BIST_STATUS_NOT_READY_E:
            *resultsStatusPtr = CPSS_DXCH_DIAG_BIST_STATUS_NOT_READY_E;
            break;
        case PRV_CPSS_DFX_DIAG_BIST_STATUS_PASS_E:
            *resultsStatusPtr = CPSS_DXCH_DIAG_BIST_STATUS_PASS_E;
            break;
        case PRV_CPSS_DFX_DIAG_BIST_STATUS_FAIL_E:
            *resultsStatusPtr = CPSS_DXCH_DIAG_BIST_STATUS_FAIL_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChDiagRamsByBistSet function
* @endinternal
*
* @brief   Use BIST to set physical RAMs with specific pattern values.
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - PP device number
* @param[in] pattern                  - pattern to set in RAMs.
*                                      May be one of the list: 0; 0x55; 0xAA; 0xFF.
* @param[in] skipCpuMemory            - skip internal CPU related memories from MC distribution
* @param[in] specificClient           - specific DFX client, used when specificClientOnly = GT_TRUE
*                                      - GT_TRUE - execute BIST on specific DFX client only.
*                                      FDX client defined by specificPipe and
* @param[in] specificClient- specific DFX client, used when specificClientOnly = GT_TRUE
*                                      - GT_FALSE - execute BIST on all DFX clients.
* @param[in] specificPipe             - specific DFX pipe, used when specificClientOnly = GT_TRUE
* @param[in] specificClient           - specific DFX client, used when specificClientOnly = GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_TIMEOUT               - on BIST timeout
* @retval GT_BAD_STATE             - on BIST failure
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Triggering the BIST will cause memory content corruption!!!
*       Executing BIST cause multiple CPSS_PP_DATA_INTEGRITY_ERROR_E events
*       which can be masked by the application using cpssEventDeviceMaskSet API.
*
*/
GT_STATUS prvCpssDxChDiagRamsByBistSet
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      pattern,
    IN  GT_BOOL                                     skipCpuMemory,
    IN  GT_BOOL                                     specificClientOnly,
    IN  GT_U32                                      specificPipe,
    IN  GT_U32                                      specificClient
)
{
    GT_STATUS rc;           /* return status */
    GT_STATUS rcFinal;      /* final return status */
    GT_U32    regAddr;      /* register's address */
    GT_U32    regData;      /* data of register */
    GT_U32    opCode;       /* BIST stage opCode */
    GT_U32    counter;      /* polling counter */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |
                                            CPSS_XCAT2_E | CPSS_LION_E | CPSS_LION2_E);

    if(PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        return GT_OK;
    }

    switch (pattern)
    {
        case 0:    opCode = 0x0008; break;
        case 0x55: opCode = 0x0018; break;
        case 0xAA: opCode = 0x0028; break;
        case 0xFF: opCode = 0x0038; break;
        default:CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* make pre BIST configurations */
    rc = prvCpssDxChDiagBistMiscConfigSet(devNum);
    if( GT_OK != rc )
    {
        return rc;
    }

    if(specificClientOnly)
    {
        /* Enable specific client to be in the multicast group */
        rc = prvCpssDxChDiagBistMcDistribForOneClientSet(devNum, specificPipe, specificClient);
        if( GT_OK != rc )
        {
            return rc;
        }
    }
    else
    {
        /* Enable all relevant clients to be in the multicast group */
        rc = prvCpssDxChDiagBistMcDistribSet(devNum, skipCpuMemory);
        if( GT_OK != rc )
        {
            return rc;
        }
    }

    /* Start and Enable BIST stage in all clients those are
       members of multicast group */
    rc = prvCpssDxChDiagBistStageStartSet(devNum, opCode, GT_TRUE);
    if( GT_OK != rc )
    {
        return rc;
    }

    /* Enable summary data logging */
    rc = prvCpssDxChDiagBistSummaryDataLoggingEnableSet(devNum, GT_TRUE);
    if( GT_OK != rc )
    {
        return rc;
    }

    rcFinal = GT_OK;
    counter = 0;
    do
    {
        if (1000 < counter++)
        {
            /* it's HW problem that BIST takes more the 1 second */
            rcFinal = GT_TIMEOUT;
            break;
        }

        cpssOsTimerWkAfter(1);

        /* Checking snoop bus status - BIST status & result */
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                                        DFXServerUnits.DFXServerRegs.snoopBusStatus;
        rc = prvCpssDrvHwPpResetAndInitControllerGetRegField(devNum, regAddr,
                                                             0, 2, &regData);
        if( GT_OK != rc )
        {
            return rc;
        }
#ifdef ASIC_SIMULATION
        /* simulation does not support the feature */
        regData = 1;
#endif
        switch(regData)
        {
            case 0:
            case 2: /* BIST in progress yet */
                    break;
            case 1: /* BIST done */
                    break;
            case 3: /* BIST failed */
                    rcFinal = GT_BAD_STATE;

                    /* exit from loop */
                    regData = 1;
                    break;
            default:
                /* SW error, code that should never be reached*/
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    }
    while (regData != 1);

    /* Disable summary data logging */
    rc = prvCpssDxChDiagBistSummaryDataLoggingEnableSet(devNum, GT_FALSE);
    if( GT_OK != rc )
    {
        return rc;
    }

    /* Stop and Disable MC Bist stage */
    rc = prvCpssDxChDiagBistStageStartSet(devNum, opCode, GT_FALSE);
    if( GT_OK != rc )
    {
        return rc;
    }

    /* The disable BIST multicast command takes several microseconds to be
       finished. Wait 1 millisecond to guaranty that BIST mode disabled.
       This is mandatory for possible following soft reset. */
    cpssOsTimerWkAfter(1);

    return rcFinal;
}
/**
* @internal prvCpssDxChDiagBistAllRamSet function
* @endinternal
*
* @brief   Use BIST to set physical RAMs with specific pattern values.
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - PP device number
* @param[in] pattern                  - pattern to set in RAMs.
*                                      May be one of the list: 0; 0x55; 0xAA; 0xFF.
* @param[in] skipCpuMemory            - skip internal CPU related memories from MC distribution
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_TIMEOUT               - on BIST timeout
* @retval GT_BAD_STATE             - on BIST failure
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Triggering the BIST will cause memory content corruption!!!
*       Executing BIST cause multiple CPSS_PP_DATA_INTEGRITY_ERROR_E events
*       which can be masked by the application using cpssEventDeviceMaskSet API.
*
*/
GT_STATUS prvCpssDxChDiagBistAllRamSet
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      pattern,
    IN  GT_BOOL                                     skipCpuMemory
)
{
    return prvCpssDxChDiagRamsByBistSet(devNum,pattern,skipCpuMemory,
                                        GT_FALSE /* all memories */,
                                        0 /* not used for all memories */,
                                        0 /* not used for all memories */);
}

/**
* @internal prvCpssDxChDiagBistRamsChoose function
* @endinternal
*
* @brief   Choose specific RAMs for BIST
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat2; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - PP device number
* @param[in] pipe                     - DFX  the client belongs to.
*                                      (not relevant for BC, MC or Data logging clients).
* @param[in] client                   - DFX  to write to.
* @param[in] ramsBmp[4]               - bitmap of RAMs for BIST
*                                      - NULL - all rams
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChDiagBistRamsChoose
(
    IN  GT_U8    devNum,
    IN  GT_U32   pipe,
    IN  GT_U32   client,
    IN  GT_U32   ramsBmp[4]
)
{
    GT_STATUS   rc;      /* return code */
    GT_U32      regAddr; /* register address */
    GT_U32      regData; /* register data */
    GT_U32      clientClockSel;   /* Client Clock Select field data */
    GT_U32      ramSel;           /* RAM Select field data          */
    GT_U32      ramMcGroupRefSel; /* RAM MC Groups Reference Select field data */
    GT_U32      ii;               /* iterator */

    if (ramsBmp)
    {
        /* 0x1 = Select_Group; Test clock is selected to defined RAM wrapper(s).*/
        clientClockSel = 1;

        /* configure group of RAMs */
        ramSel = 126;

        /* use clientRamMC_Group for group of RAMs setting */
        ramMcGroupRefSel = 0xF;
    }
    else
    {
        /* set HW default values */
        clientClockSel = ramSel = ramMcGroupRefSel = 0;

        /* use Broadcast for restore fields to default in all clients */
        client = PRV_CPSS_DFX_CLIENT_BC_CNS;
    }

    /* configure Client Clock Select */
    regAddr =  PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                                                      DFXClientUnits.clientControl;
    rc = prvCpssDfxClientSetRegField(
            devNum, pipe, client, regAddr, 2, 1, clientClockSel);
    if( GT_OK != rc )
    {
        return rc;
    }

    /* configure RAM Select */
    rc = prvCpssDfxClientSetRegField(
            devNum, pipe, client, regAddr, 12, 7, ramSel);
    if( GT_OK != rc )
    {
        return rc;
    }

    /* configure RAM MC Groups Reference Select */
    regAddr =  PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                                                      DFXClientUnits.clientDataControl;
    rc = prvCpssDfxClientSetRegField(
            devNum, pipe, client, regAddr, 15, 4, ramMcGroupRefSel);
    if( GT_OK != rc )
    {
        return rc;
    }

    for (ii = 0; ii < 4; ii++)
    {
        regData = (ramsBmp) ? ramsBmp[ii] : 0;
        regAddr =  PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                                                          DFXClientUnits.clientRamMC_Group[ii];
        rc = prvCpssDfxClientSetRegField(devNum, pipe, client, regAddr, 0, 32, regData);
        if( GT_OK != rc )
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChDiagBistCaelumFixBist function
* @endinternal
*
* @brief   Function fixes BIST Max Address register default value.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - PP device number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_TIMEOUT               - on BIST timeout
* @retval GT_BAD_STATE             - on BIST failure
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChDiagBistCaelumFixBist
(
    IN  GT_U8   devNum
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      dfxPipe;    /* DFX pipe number */
    GT_U32      dfxClient;  /* DFX client number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E |
                                          CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
                                          CPSS_XCAT2_E | CPSS_LION_E | CPSS_LION2_E |
                                          CPSS_BOBCAT2_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    if(PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        return GT_OK;
    }

    dfxPipe = 0;
    dfxClient = 21;

    /* fix "Max Address" for BIST */
    regAddr =  PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                                                      DFXClientUnits.BISTMaxAddress;
    rc = prvCpssDfxClientSetRegField(
            devNum, dfxPipe, dfxClient, regAddr, 0, 16, 32767);

    return rc;
}
/**
* @internal internal_cpssDxChDiagBistTriggerAllSet function
* @endinternal
*
* @brief   Trigger the starting of BIST on device physical RAMs.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - PP device number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Triggering the BIST will cause memory content corruption!!!
*       Executing BIST cause multiple CPSS_PP_DATA_INTEGRITY_ERROR_E events
*       which can be masked by the application using cpssEventDeviceMaskSet API.
*
*/
static GT_STATUS internal_cpssDxChDiagBistTriggerAllSet
(
    IN  GT_U8 devNum
)
{
    GT_STATUS rc;             /* return status */
    GT_U32  dontUseDataLog;   /* use or not Data Logging feature */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
                                            CPSS_XCAT2_E | CPSS_LION_E | CPSS_LION2_E);

    if(PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        return GT_OK;
    }

    /* make pre BIST configurations */
    rc = prvCpssDxChDiagBistMiscConfigSet(devNum);
    if( GT_OK != rc )
    {
        return rc;
    }

    /* Enable all relevant clients to be in the multicast group */
    rc = prvCpssDxChDiagBistMcDistribSet(devNum, GT_TRUE);
    if( GT_OK != rc )
    {
        return rc;
    }

    /* Start and Enable BIST in all clients those are
       members of multicast group */
    rc = prvCpssDfxDiagBistStartSet(devNum, GT_TRUE);
    if( GT_OK != rc )
    {
        return rc;
    }

    dontUseDataLog = PRV_CPSS_SIP_5_CHECK_MAC(devNum);

    if (!dontUseDataLog)
    {
        /* Enable summary data logging */
        rc = prvCpssDxChDiagBistSummaryDataLoggingEnableSet(devNum, GT_TRUE);
    }

    return rc;
}

/**
* @internal cpssDxChDiagBistTriggerAllSet function
* @endinternal
*
* @brief   Trigger the starting of BIST on device physical RAMs.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - PP device number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Triggering the BIST will cause memory content corruption!!!
*       Executing BIST cause multiple CPSS_PP_DATA_INTEGRITY_ERROR_E events
*       which can be masked by the application using cpssEventDeviceMaskSet API.
*
*/
GT_STATUS cpssDxChDiagBistTriggerAllSet
(
    IN  GT_U8                                       devNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagBistTriggerAllSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum));

    rc = internal_cpssDxChDiagBistTriggerAllSet(devNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChDiagBistResultsGet function
* @endinternal
*
* @brief   Retrieve the BIST results.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - PP device number
* @param[in,out] resultsNumPtr        - in: max num of results that can be
*                                           reported due to size limit of resultsArr[].
*                                       out: the number of failures detected.
*                                            Relevant only if BIST failed.
* @param[out] resultsStatusPtr         - (pointer to) the status of the BIST.
* @param[out] resultsArr[]             - (pointer to) the BIST failures.
*                                      Relevant only if BIST failed.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_FAIL                  - on SW error, code that never be reached.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChDiagBistResultsGet
(
    IN  GT_U8                                       devNum,
    OUT CPSS_DXCH_DIAG_BIST_STATUS_ENT              *resultsStatusPtr,
    OUT CPSS_DXCH_DIAG_BIST_RESULT_STC              resultsArr[],
    INOUT GT_U32                                    *resultsNumPtr
)
{
    GT_U32 regAddr; /* register address */
    GT_U32 regData; /* register data */
    GT_U32 pipe;    /* DFX pipe id */
    GT_U32 activePipe; /* active pipe for read access */
    GT_U32 client;  /* DFX client id */
    GT_U32 ram;     /* DFX RAM id */
    GT_U32 ii;      /* loop iterator */
    GT_U32 resultsCounter;  /* counter for number of reported failures */
    GT_STATUS rc;   /* return status */
    PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC    *dbArrayPtr; /* pointer to data integrity DB */
    GT_U32                                          dbArrayEntryNum; /* size of data integrity DB */
    GT_U32  dontUseDataLog; /* use or not Data Logging feature */
    GT_U32  dfxClientsBmpArr[PRV_CPSS_DFX_MAX_PIPES_CNS];

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
                                            CPSS_XCAT2_E | CPSS_LION_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(resultsStatusPtr);
    CPSS_NULL_PTR_CHECK_MAC(resultsNumPtr);
    CPSS_NULL_PTR_CHECK_MAC(resultsArr);

    if(PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        *resultsStatusPtr = CPSS_DXCH_DIAG_BIST_STATUS_PASS_E;
        return GT_OK;
    }

    dontUseDataLog = PRV_CPSS_SIP_5_CHECK_MAC(devNum);

    if (dontUseDataLog)
    {
        rc = prvCpssDxChDiagBistClientsListBuild(devNum, GT_TRUE, dfxClientsBmpArr);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssDxChDiagBistStatusCheck(devNum, dfxClientsBmpArr, resultsStatusPtr);
        if( GT_OK != rc )
        {
            return rc;
        }

        if (*resultsStatusPtr == CPSS_DXCH_DIAG_BIST_STATUS_NOT_READY_E)
        {
            return GT_OK;
        }
    }
    else
    {
        /* Checking snoop bus status - BIST status & result */
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                                        DFXServerUnits.DFXServerRegs.snoopBusStatus;
        rc = prvCpssDrvHwPpResetAndInitControllerGetRegField(devNum, regAddr,
                                                             0, 2, &regData);
        if( GT_OK != rc )
        {
            return rc;
        }

        switch(regData)
        {
            case 0:
            case 2: *resultsStatusPtr = CPSS_DXCH_DIAG_BIST_STATUS_NOT_READY_E;
                    return GT_OK;
            case 1: *resultsStatusPtr = CPSS_DXCH_DIAG_BIST_STATUS_PASS_E;
                    break;
            case 3: *resultsStatusPtr = CPSS_DXCH_DIAG_BIST_STATUS_FAIL_E;
                    break;
            default:
                /* SW error, code that should never be reached*/
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        /* Disable summary data logging */
        rc = prvCpssDxChDiagBistSummaryDataLoggingEnableSet(devNum, GT_FALSE);
        if( GT_OK != rc )
        {
            return rc;
        }
    }

    if( CPSS_DXCH_DIAG_BIST_STATUS_PASS_E == *resultsStatusPtr )
    {
        /* Stop and Disable MC Bist */
        return prvCpssDfxDiagBistStartSet(devNum, GT_FALSE);
    }

    resultsCounter = 0;

    /* Activate pipe 0 */
    rc = prvCpssDfxMemoryPipeIdSet(devNum, 0);
    if( GT_OK != rc )
    {
        return rc;
    }

    /* assign DB pointer and size */
    prvCpssDxChDiagDataIntegrityDbPointerSet(devNum, &dbArrayPtr, &dbArrayEntryNum);
    if ((dbArrayPtr == NULL) || (dbArrayEntryNum == 0))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    activePipe = 0;

    for( ii = 0 ; ii < dbArrayEntryNum ; ii++ )
    {
        pipe = PRV_CPSS_DIAG_DATA_INTEGRITY_MAP_KEY_PIPE_GET(dbArrayPtr[ii].key);

        /* check skip DFX pipe */
        if (prvCpssDxChDiagBistCheckSkipPipe(devNum,pipe) == GT_TRUE)
            continue;

        if( pipe != activePipe )
        {
            /* Activate pipe as "activePipe" */
            rc = prvCpssDfxMemoryPipeIdSet(devNum, pipe);
            if( GT_OK != rc )
            {
                return rc;
            }

            activePipe = pipe;
        }

        client = PRV_CPSS_DIAG_DATA_INTEGRITY_MAP_KEY_CLIENT_GET(dbArrayPtr[ii].key);

        if (prvCpssDxChDiagBistCheckSkipClient(devNum, pipe, client))
            continue;

        ram = PRV_CPSS_DIAG_DATA_INTEGRITY_MAP_KEY_MEMORY_GET(dbArrayPtr[ii].key);

        rc = prvCpssDfxClientRegAddressGet(client,
               PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                                      DFXClientUnits.clientRamBISTInfo[ram/32],
                                           &regAddr);
        if( GT_OK != rc )
        {
            return rc;
        }

        rc = prvCpssDrvHwPpResetAndInitControllerGetRegField(
                                                    devNum, regAddr,
                                                    ram%32, 1, &regData);
        if( GT_OK != rc )
        {
            return rc;
        }

        if( 1 == regData )
        {
            if( resultsCounter < *resultsNumPtr )
            {
                resultsArr[resultsCounter].location.dfxPipeId = pipe;
                resultsArr[resultsCounter].location.dfxClientId = client;
                resultsArr[resultsCounter].location.dfxMemoryId = ram;
                resultsArr[resultsCounter].memType = dbArrayPtr[ii].memType;
            }
            resultsCounter++;
        }
    }

    *resultsNumPtr = resultsCounter;

    /* Stop MC Bist */
    rc = prvCpssDfxDiagBistStartSet(devNum, GT_FALSE);

    return rc;
}

/**
* @internal cpssDxChDiagBistResultsGet function
* @endinternal
*
* @brief   Retrieve the BIST results.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - PP device number
* @param[in,out] resultsNumPtr        - in: max num of results that can be
*                                           reported due to size limit of resultsArr[].
*                                       out: the number of failures detected.
*                                            Relevant only if BIST failed.
* @param[out] resultsStatusPtr         - (pointer to) the status of the BIST.
* @param[out] resultsArr[]             - (pointer to) the BIST failures.
*                                      Relevant only if BIST failed.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_FAIL                  - on SW error, code that never be reached.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChDiagBistResultsGet
(
    IN  GT_U8                                       devNum,
    OUT CPSS_DXCH_DIAG_BIST_STATUS_ENT              *resultsStatusPtr,
    OUT CPSS_DXCH_DIAG_BIST_RESULT_STC              resultsArr[],
    INOUT GT_U32                                    *resultsNumPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagBistResultsGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, resultsStatusPtr, resultsArr, resultsNumPtr));

    rc = internal_cpssDxChDiagBistResultsGet(devNum, resultsStatusPtr, resultsArr, resultsNumPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, resultsStatusPtr, resultsArr, resultsNumPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChDiagExternalMemoriesBistCfgReset function
* @endinternal
*
* @brief   This function resets BIST configuration in given set of DDR units.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] accessType               - single memory or all topology accessable memories
* @param[in] ifNum                    - memory number, relevant only when accessType is single
*                                      Bobcat2 devices support 5 External Memory units.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
*/
static GT_STATUS    prvCpssDxChDiagExternalMemoriesBistCfgReset
(
    IN  GT_U8                   devNum,
    IN  MV_HWS_ACCESS_TYPE      accessType,
    IN  GT_U32                  ifNum
)
{
    /* BIST Stop */
    CHECK_STATUS(mvHwsDdr3TipIFWrite(
        devNum, accessType, ifNum, ODPG_DATA_CONTROL_REG, 0, MASK_ALL_BITS));
    return GT_OK;
}

/**
* @internal prvCpssDxChDiagExternalMemoriesBistStop function
* @endinternal
*
* @brief   This function stops BIST in given set of DDR units.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] accessType               - single memory or all topology accessable memories
* @param[in] ifNum                    - memory number, relevant only when accessType is single
*                                      Bobcat2 devices support 5 External Memory units.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
*/
static GT_STATUS    prvCpssDxChDiagExternalMemoriesBistStop
(
    IN  GT_U8                   devNum,
    IN  MV_HWS_ACCESS_TYPE      accessType,
    IN  GT_U32                  ifNum
)
{
    /* BIST Stop */
    CHECK_STATUS(mvHwsDdr3TipIFWrite(
        devNum, accessType, ifNum, ODPG_DATA_CONTROL_REG,
        (GT_U32)(1 << 30), (GT_U32)(0x3 << 30)));
    return GT_OK;
}

/**
* @internal prvCpssDxChDiagExternalMemoriesBistStart function
* @endinternal
*
* @brief   This function starts BIST in given set of DDR units.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] pattern                  - read/write memory  Id.
* @param[in] accessType               - single memory or all topology accessable memories
* @param[in] ifNum                    - memory number, relevant only when accessType is single
*                                      Bobcat2 devices support 5 External Memory units.
* @param[in] direction                - read or write memory
* @param[in] testedAreaOffset         - Tested Area Offset in 64-bit units.
*                                      Relevant only when testWholeMemory is GT_FALSE.
* @param[in] testedAreaLength         - Tested Area Length in 64-bit units.
*                                      Relevant only when testWholeMemory is GT_FALSE.
* @param[in] pattern                  - pattern for writing to the memory.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
*/
static GT_STATUS    prvCpssDxChDiagExternalMemoriesBistStart
(
    IN  GT_U8                   devNum,
    IN  MV_HWS_PATTERN          pattern,
    IN  MV_HWS_ACCESS_TYPE      accessType,
    IN  GT_U32                  ifNum,
    IN  MV_HWS_DIRECTION        direction,
    IN  GT_U32                  testedAreaOffset,
    IN  GT_U32                  testedAreaLength
)
{
    GT_U32    csNum = 0;
    GT_U32    txBurstSize;
    GT_U32    delayBetweenBurst;
    GT_U32    rdMode;
    PatternInfo* patternTable = ddr3TipGetPatternTable();

    /* BIST Stop */
    CHECK_STATUS(mvHwsDdr3TipIFWrite(
        devNum,  accessType, ifNum,  ODPG_DATA_CONTROL_REG,
        (GT_U32)(1 << 30)  , (GT_U32)(0x3 << 30)));
    /* clear BIST Finished Bit */
    CHECK_STATUS(mvHwsDdr3TipIFWrite(
        devNum, accessType, ifNum, ODPG_BIST_DONE, 0, 1));
    csNum = 0;/* CS = 0 */
    /* ODPG Write enable from BIST */
    CHECK_STATUS(mvHwsDdr3TipIFWrite(
        devNum, accessType,  ifNum, ODPG_DATA_CONTROL_REG,  0x1, 0x1));
    /* ODPG Read enable/disable from BIST */
    CHECK_STATUS(mvHwsDdr3TipIFWrite(
        devNum, accessType, ifNum, ODPG_DATA_CONTROL_REG,
        (direction==OPER_READ) ? 0x2 : 0, 0x2));
    CHECK_STATUS(ddr3TipLoadPatternToOdpg(
        devNum, accessType, ifNum, pattern, testedAreaOffset));
    CHECK_STATUS(mvHwsDdr3TipIFWrite(
        devNum, accessType, ifNum,  ODPG_PATTERN_ADDR_OFFSET_REG,
        testedAreaOffset, MASK_ALL_BITS));
    CHECK_STATUS(mvHwsDdr3TipIFWrite(
        devNum,   accessType, ifNum,   ODPG_DATA_BUF_SIZE_REG,
        testedAreaLength, MASK_ALL_BITS));
    txBurstSize = (direction == OPER_WRITE) ? patternTable[pattern].txBurstSize : 0;
    delayBetweenBurst = (direction == OPER_WRITE) ? 2 : 0;
    rdMode = (direction == OPER_WRITE) ? 1 : 0;
    CHECK_STATUS(ddr3TipConfigureOdpg(
        devNum, accessType, ifNum, direction, patternTable[pattern].numOfPhasesTx,
        txBurstSize, patternTable[pattern].numOfPhasesRx, delayBetweenBurst,
        rdMode,  csNum, STRESS_NONE, DURATION_ADDRESS));
    /* BIST_START trigger*/
    CHECK_STATUS(mvHwsDdr3TipIFWrite(
        devNum,  accessType, ifNum,  ODPG_DATA_CONTROL_REG,
        (GT_U32)(1 << 31), (GT_U32)(1 << 31)));

    return GT_OK;
}

/**
* @internal prvCpssDxChDiagExternalMemoriesBistIsFinished function
* @endinternal
*
* @brief   This function checks if BIST on given DDR unit is finished.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] ifNum                    - memory number
*                                      Bobcat2 devices support 5 External Memory units.
* @param[in] clearReadyBit            - GT_TRUE - clear Ready Bit, GT_FALSE - unchange Ready Bit
*
* @param[out] isFinishedPtr            - (pointer to) GT_TRUE - finished, GT_FALSE - not finished.
*                                      NULL can be specified (when function called to clear Ready Bit only)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
*/
static GT_STATUS    prvCpssDxChDiagExternalMemoriesBistIsFinished
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  ifNum,
    IN  GT_BOOL                 clearReadyBit,
    OUT GT_BOOL                 *isFinishedPtr
)
{
    GT_U32 readData[MAX_INTERFACE_NUM];

    CHECK_STATUS(mvHwsDdr3TipIFRead(
        devNum, ACCESS_TYPE_UNICAST, ifNum, ODPG_BIST_DONE, readData, MASK_ALL_BITS));
    if (isFinishedPtr != NULL)
    {
        *isFinishedPtr = ((readData[ifNum] & 1) == 0) ? GT_FALSE : GT_TRUE;
    }
    if (clearReadyBit != GT_FALSE)
    {
        CHECK_STATUS(mvHwsDdr3TipIFWrite(
            devNum, ACCESS_TYPE_UNICAST, ifNum, ODPG_BIST_DONE, 0, 1));
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChDiagExternalMemoriesBistWaitForFinish function
* @endinternal
*
* @brief   This function waits for finish of BIST on given set of DDR units.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] extMemoBitmap            - bitmap of External DRAM units.
*                                      Bobcat2 devices support 5 External DRAM units.
* @param[in] timeout                  - timeout in milliseconds
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_TIMEOUT               - at timeout
*/
static GT_STATUS    prvCpssDxChDiagExternalMemoriesBistWaitForFinish
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  extMemoBitmap,
    IN  GT_U32                  timeout
)
{
    GT_STATUS       retVal;                  /* return code                  */
    GT_U32          i;                       /* loop index                   */
    GT_U32          notFinisfedBitmap;       /* not Finisfed Memories Bitmap */
    GT_BOOL         isFinished;              /* is BIST finished             */
    GT_U32          maxLoops;                /* max Loops                    */
    GT_U32          timeStep;                /* time Step                    */


    notFinisfedBitmap = extMemoBitmap;
    timeStep          = 50; /* milliseconds */
    maxLoops          = (timeout + timeStep) / timeStep;
    isFinished        = GT_FALSE; /* fixing compiler warning */

    for (; (maxLoops > 0); maxLoops--)
    {
        for (i = 0; (i < CPSS_DXCH_DIAG_EXT_MEMORY_INF_NUM_CNS); i++)
        {
            if (((notFinisfedBitmap >> i) & 1) == 0)
            {
                continue;
            }
            retVal = prvCpssDxChDiagExternalMemoriesBistIsFinished(
                devNum, i, GT_FALSE/*clearReadyBit*/, &isFinished);
            if (retVal != GT_OK)
            {
                return retVal;
            }
            if (isFinished != GT_FALSE)
            {
                notFinisfedBitmap &= (~ (1 << i));
                if (notFinisfedBitmap == 0)
                {
                    return GT_OK;
                }
            }
        }
        cpssOsTimerWkAfter(timeStep);
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, LOG_ERROR_NO_MSG);
}

/**
* @internal prvCpssDxChDiagExternalMemoriesBistMRSUpdate function
* @endinternal
*
* @brief   This function updates MRS state after mvHwsDdr3TipSelectDdrController.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
*/
GT_STATUS    prvCpssDxChDiagExternalMemoriesBistMRSUpdate
(
    IN  GT_U8                   devNum
)
{
    GT_U32              interfaceId;
    GT_U32              busCnt;
    MV_HWS_TOPOLOGY_MAP *mvHwsTopologyPtr;         /* topology map    */
    GT_U32              csMask[MAX_INTERFACE_NUM]; /* bitmap inverted */

    mvHwsTopologyPtr = ddr3TipGetTopologyMap(devNum);
    if (mvHwsTopologyPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* calculate interface cs mask */
    for (interfaceId = 0; interfaceId < MAX_INTERFACE_NUM; interfaceId++)
    {
        /* cs enable is active low */
        csMask[interfaceId] = CS_BIT_MASK;
        if (IS_INTERFACE_ACTIVE(mvHwsTopologyPtr->interfaceActiveMask, interfaceId) == GT_FALSE)
        {
            continue;
        }

        for (busCnt=0; busCnt<ddr3TipDevAttrGet(devNum, MV_ATTR_OCTET_PER_INTERFACE); busCnt++)
        {
            csMask[interfaceId] &=
                ~(mvHwsTopologyPtr->interfaceParams[interfaceId].asBusParams[busCnt].csBitmask);
        }
    }

    CHECK_STATUS(ddr3TipWriteMRSCmd(devNum, csMask, MRS1_CMD, 0, (3 << 3)));

    return GT_OK;
}

/**
* @internal internal_cpssDxChDiagExternalMemoriesBistRun function
* @endinternal
*
* @brief   This function runs BIST in given set of DDR units.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] extMemoBitmap            - bitmap of External DRAM units.
*                                      Bobcat2 devices support 5 External DRAM units.
* @param[in] testWholeMemory          - GT_TRUE - test all memory,
*                                      GT_FALSE - test area specified by
* @param[in] testedAreaOffset         and testedAreaLength only.
* @param[in] testedAreaOffset         - Tested Area Offset in 64-bit units.
*                                      Relevant only when testWholeMemory is GT_FALSE.
* @param[in] testedAreaLength         - Tested Area Length in 64-bit units.
*                                      Relevant only when testWholeMemory is GT_FALSE.
* @param[in] pattern                  - pattern for writing to the memory.
*
* @param[out] testStatusPtr            - (pointer to)GT_TRUE - no errors, GT_FALSE - errors
* @param[out] errorInfoArr[CPSS_DXCH_DIAG_EXT_MEMORY_INF_NUM_CNS] - array of Error Info structures for external DRAMs.
*                                      An array should contain 5 structures even not all
*                                      5 memories tested. Counters for bypassed memories
*                                      will contain zeros.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This function should be called when External DRAM was already initialized.
*       The cpssDxChTmGlueDramInit is used for External DRAM initialization.
*
*/
static GT_STATUS internal_cpssDxChDiagExternalMemoriesBistRun
(
    IN  GT_U8                                        devNum,
    IN  GT_U32                                       extMemoBitmap,
    IN  GT_BOOL                                      testWholeMemory,
    IN  GT_U32                                       testedAreaOffset,
    IN  GT_U32                                       testedAreaLength,
    IN  CPSS_DXCH_DIAG_EXT_MEMORY_BIST_PATTERN_ENT   pattern,
    OUT GT_BOOL                                      *testStatusPtr,
    OUT CPSS_DXCH_DIAG_EXT_MEMORY_INF_BIST_ERROR_STC errorInfoArr[CPSS_DXCH_DIAG_EXT_MEMORY_INF_NUM_CNS]
)
{
    GT_STATUS       retVal;                  /* return code                  */
    GT_U32          i;                       /* loop index                   */
    BistResult      stBistResult;            /* BIST result                  */
    MV_HWS_PATTERN  hwSrvPattern;            /* HW SRV paterrn               */
    GT_U32          saveInterfaceActiveMask; /* save inteface mask           */
    MV_HWS_TOPOLOGY_MAP *mvHwsTopologyPtr;   /* topology map                 */
    GT_U32          firstIf;                 /* first interface              */
    GT_U32          bit64MemorySize;         /* Memory Size in 64bit units   */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(
        devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
        | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
        CPSS_XCAT2_E | CPSS_LION_E | CPSS_LION2_E );
    CPSS_NULL_PTR_CHECK_MAC(errorInfoArr);

    mvHwsTopologyPtr = ddr3TipGetTopologyMap(devNum);
    if (mvHwsTopologyPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    if ((extMemoBitmap &
        (~ BIT_MASK_MAC(CPSS_DXCH_DIAG_EXT_MEMORY_INF_NUM_CNS))) != 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if ((extMemoBitmap &
        mvHwsTopologyPtr->interfaceActiveMask) == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    firstIf = 0xFFFFFFFF;
    for (i = 0; (i < CPSS_DXCH_DIAG_EXT_MEMORY_INF_NUM_CNS); i++)
    {
        if ((extMemoBitmap >> i) & 1)
        {
            firstIf = i;
            break;
        }
    }

    if (firstIf == 0xFFFFFFFF)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (pattern)
    {
        case CPSS_DXCH_DIAG_EXT_MEMORY_BIST_PATTERN_BASIC_E:
            hwSrvPattern = PATTERN_PBS1;
            break;
        case CPSS_DXCH_DIAG_EXT_MEMORY_BIST_PATTERN_STRESS_E:
            hwSrvPattern = PATTERN_KILLER_DQ0;
            break;
        case CPSS_DXCH_DIAG_EXT_MEMORY_BIST_PATTERN_STRESS_1_E:
            hwSrvPattern = PATTERN_FULL_SSO0;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (mvHwsTopologyPtr->interfaceParams[firstIf].memorySize)
    {
        case MEM_512M: bit64MemorySize = 0x00800000; break;
        case MEM_1G:   bit64MemorySize = 0x01000000; break;
        case MEM_2G:   bit64MemorySize = 0x02000000; break;
        case MEM_4G:   bit64MemorySize = 0x04000000; break;
        case MEM_8G:   bit64MemorySize = 0x08000000; break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    if (testWholeMemory != GT_FALSE)
    {
        testedAreaOffset = 0;
        /* convert length to 64-bit inits */
        testedAreaLength = bit64MemorySize;
    }
    else
    {
        if ((testedAreaOffset + testedAreaLength) > bit64MemorySize)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    /* switch topology map in global mvHwsDdr3Tip lib variables to devNum */
    retVal = mvHwsDdr3TipLoadTopologyMap(
        devNum, mvHwsTopologyPtr);
    if( GT_OK != retVal )
    {
        return retVal;
    }

    /* set active device bitmask for ACCESS_TYPE_MULTICAST */
    saveInterfaceActiveMask = mvHwsTopologyPtr->interfaceActiveMask;
    mvHwsTopologyPtr->interfaceActiveMask = (GT_U8)extMemoBitmap;
    retVal = mvHwsDdr3TipSelectDdrController(devNum, GT_TRUE);
    if (retVal != GT_OK)
    {
        mvHwsTopologyPtr->interfaceActiveMask = (GT_U8)saveInterfaceActiveMask;
        mvHwsDdr3TipSelectDdrController(devNum, GT_TRUE);
        return retVal;
    }
#if 0 /* This code commented out up to adding correct support */
      /* for BIST compatible with TM initialisation           */
    retVal = prvCpssDxChDiagExternalMemoriesBistMRSUpdate(devNum);
    if (retVal != GT_OK)
    {
        mvHwsTopologyPtr->interfaceActiveMask = saveInterfaceActiveMask;
        mvHwsDdr3TipSelectDdrController(devNum, GT_TRUE);
        return retVal;
    }
#endif
    /* pattern write phase */
    retVal = prvCpssDxChDiagExternalMemoriesBistStart(
        devNum, hwSrvPattern, ACCESS_TYPE_MULTICAST, 0/*ifNum*/,
        OPER_WRITE, testedAreaOffset, testedAreaLength);
    if (retVal != GT_OK)
    {
        prvCpssDxChDiagExternalMemoriesBistStop(
            devNum, ACCESS_TYPE_MULTICAST, 0/*ifNum*/);
        mvHwsTopologyPtr->interfaceActiveMask = (GT_U8)saveInterfaceActiveMask;
        mvHwsDdr3TipSelectDdrController(devNum, GT_TRUE);
        return retVal;
    }

    retVal = prvCpssDxChDiagExternalMemoriesBistWaitForFinish(
        devNum, extMemoBitmap, 2000 /*timeout*/);
    if (retVal != GT_OK)
    {
        prvCpssDxChDiagExternalMemoriesBistStop(
            devNum, ACCESS_TYPE_MULTICAST, 0/*ifNum*/);
        mvHwsTopologyPtr->interfaceActiveMask = (GT_U8)saveInterfaceActiveMask;
        mvHwsDdr3TipSelectDdrController(devNum, GT_TRUE);
        return retVal;
    }

    /* pattern read phase */
    retVal = prvCpssDxChDiagExternalMemoriesBistStart(
        devNum, hwSrvPattern, ACCESS_TYPE_MULTICAST, 0/*ifNum*/,
        OPER_READ, testedAreaOffset, testedAreaLength);
    if (retVal != GT_OK)
    {
        prvCpssDxChDiagExternalMemoriesBistStop(
            devNum, ACCESS_TYPE_MULTICAST, 0/*ifNum*/);
        mvHwsTopologyPtr->interfaceActiveMask = (GT_U8)saveInterfaceActiveMask;
        mvHwsDdr3TipSelectDdrController(devNum, GT_TRUE);
        return retVal;
    }

    retVal = prvCpssDxChDiagExternalMemoriesBistWaitForFinish(
        devNum, extMemoBitmap, 2000 /*timeout*/);
    if (retVal != GT_OK)
    {
        prvCpssDxChDiagExternalMemoriesBistStop(
            devNum, ACCESS_TYPE_MULTICAST, 0/*ifNum*/);
        mvHwsTopologyPtr->interfaceActiveMask = (GT_U8)saveInterfaceActiveMask;
        mvHwsDdr3TipSelectDdrController(devNum, GT_TRUE);
        return retVal;
    }

    prvCpssDxChDiagExternalMemoriesBistStop(
        devNum, ACCESS_TYPE_MULTICAST, 0/*ifNum*/);

    prvCpssDxChDiagExternalMemoriesBistCfgReset(
        devNum, ACCESS_TYPE_MULTICAST, 0/*ifNum*/);

    /* restore active device bitmask */
    mvHwsTopologyPtr->interfaceActiveMask = (GT_U8)saveInterfaceActiveMask;
    mvHwsDdr3TipSelectDdrController(devNum, GT_TRUE);

    *testStatusPtr = GT_TRUE;
    for(i = 0; (i < CPSS_DXCH_DIAG_EXT_MEMORY_INF_NUM_CNS); i++)
    {
        if (((extMemoBitmap >> i) & 1) == 0)
        {
            errorInfoArr[i].errCounter     = 0;
            errorInfoArr[i].lastFailedAddr = 0;
            continue;
        }

        retVal = ddr3TipBistReadResult(devNum, i, &stBistResult);
        if (retVal != GT_OK)
        {
            return retVal;
        }
        errorInfoArr[i].errCounter = stBistResult.bistErrorCnt;
        errorInfoArr[i].lastFailedAddr = stBistResult.bistLastFailAddr;
        if (errorInfoArr[i].errCounter != 0)
        {
            *testStatusPtr = GT_FALSE;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChDiagExternalMemoriesBistRun function
* @endinternal
*
* @brief   This function runs BIST in given set of DDR units.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] extMemoBitmap            - bitmap of External DRAM units.
*                                      Bobcat2 devices support 5 External DRAM units.
* @param[in] testWholeMemory          - GT_TRUE - test all memory,
*                                      GT_FALSE - test area specified by
* @param[in] testedAreaOffset         and testedAreaLength only.
* @param[in] testedAreaOffset         - Tested Area Offset in 64-bit units.
*                                      Relevant only when testWholeMemory is GT_FALSE.
* @param[in] testedAreaLength         - Tested Area Length in 64-bit units.
*                                      Relevant only when testWholeMemory is GT_FALSE.
* @param[in] pattern                  - pattern for writing to the memory.
*
* @param[out] testStatusPtr            - (pointer to)GT_TRUE - no errors, GT_FALSE - errors
* @param[out] errorInfoArr[CPSS_DXCH_DIAG_EXT_MEMORY_INF_NUM_CNS] - array of Error Info structures for external DRAMs.
*                                      An array should contain 5 structures even not all
*                                      5 memories tested. Counters for bypassed memories
*                                      will contain zeros.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This function should be called when External DRAM was already initialized.
*       The cpssDxChTmGlueDramInit is used for External DRAM initialization.
*
*/
GT_STATUS    cpssDxChDiagExternalMemoriesBistRun
(
    IN  GT_U8                                        devNum,
    IN  GT_U32                                       extMemoBitmap,
    IN  GT_BOOL                                      testWholeMemory,
    IN  GT_U32                                       testedAreaOffset,
    IN  GT_U32                                       testedAreaLength,
    IN  CPSS_DXCH_DIAG_EXT_MEMORY_BIST_PATTERN_ENT   pattern,
    OUT GT_BOOL                                      *testStatusPtr,
    OUT CPSS_DXCH_DIAG_EXT_MEMORY_INF_BIST_ERROR_STC errorInfoArr[CPSS_DXCH_DIAG_EXT_MEMORY_INF_NUM_CNS]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagExternalMemoriesBistRun);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, extMemoBitmap, testWholeMemory, testedAreaOffset, testedAreaLength, pattern, testStatusPtr, errorInfoArr));

    rc = internal_cpssDxChDiagExternalMemoriesBistRun(devNum, extMemoBitmap, testWholeMemory, testedAreaOffset, testedAreaLength, pattern, testStatusPtr, errorInfoArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, extMemoBitmap, testWholeMemory, testedAreaOffset, testedAreaLength, pattern, testStatusPtr, errorInfoArr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
