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
* @file prvCpssDxChHwRegAddrVer1.c
*
* @brief This file includes definitions of all different 'eArch' registers
* addresses to be used by all functions accessing PP registers.
* a variable of type PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC should be allocated
* for each PP type.
*
* @version   101
********************************************************************************
*/
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChModuleConfig.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInitLedCtrl.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInitLedCtrl.h>
#include <cpss/generic/cpssHwInit/private/prvCpssHwInit.h>
#include <cpss/common/port/private/prvCpssPortTypes.h>
#include <cpss/dxCh/dxChxGen/phy/prvCpssDxChSmiUnitDrv.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortDynamicPAUnitDrv.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <port/silicon/bobcat3/mvHwsBobcat3PortIf.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>

/* skip non exists DMA in ports BMP */
#define SKIP_NON_EXISTS_PORT_IN_BMP(bmpPtr,portNum)         \
    if(0 == CPSS_PORTS_BMP_IS_PORT_SET_MAC(bmpPtr,portNum)) \
    {                                                       \
        continue;                                           \
    }


/*calc number of words form number of bits */
#define NUM_WORDS_FROM_BITS_MAC(x)   (((x) + 31)>>5)

/* SDMA id of the CPU */
#define CPU_SDMA_ID 72
/* NOTE : the 0xFE as start of address is to bypass arithmetic issue in
    cpssDrvPpHwPciCntlSim.c /cpssDrvPpHwPciCntl.c that do 'next unit' calculation
    (if done on 0xFF --> (0xFF + 1) get 0 on GT_U8) */
#define UNIT_MASK_9_MSB_CNS   0xFF800000

extern GT_U32 prvDxChBobcat2UnitsBaseAddr[PRV_CPSS_DXCH_UNIT_LAST_E + 1];  /* Bobcat2 unit base address array */
extern GT_U32 prvDxChBobkUnitsBaseAddr   [PRV_CPSS_DXCH_UNIT_LAST_E + 1];  /* Bobk unit base address array */
extern GT_U32 prvDxChBobcat3UnitsBaseAddr[PRV_CPSS_DXCH_UNIT_LAST_E + 1];  /* Bobcat3 unit base address array */
extern GT_U32 prvDxChAldrinUnitsBaseAddr[PRV_CPSS_DXCH_UNIT_LAST_E + 1];   /* Aldrin unit base address array */
extern GT_U32 prvDxChAldrin2UnitsBaseAddr[PRV_CPSS_DXCH_UNIT_LAST_E + 1];/* Aldrin2 unit base address array */
extern GT_U32 prvDxChFalconUnitsBaseAddr [PRV_CPSS_DXCH_UNIT_LAST_E + 1];/* Falcon unit base address array */

static void txqDqInit
(
    IN  GT_U8 devNum,
    IN  GT_U32  index,
    IN  GT_U32  numTxqDqPortsPerUnit_firstBlock,
    IN  GT_U32  numTxqDqPortsPerUnit_secondBlock,
    IN  GT_U32  lastTxDmaIndex

);

static void txqDqDmaRelatedGlobalPortUpdate
(
    IN  GT_U8 devNum,
    IN  GT_U32  index
);

static void txqDqUpdateUnit0AsGlobalPortsAddr
(
    IN  GT_U8   devNum,
    IN  GT_U32  currentUnitIndex,
    IN  GT_U32  portIn_unit_0,
    IN  GT_U32  portInCurrentUnit,
    IN  GT_U32  numOfPorts
);

static void txqPdxInit_sip6
(
    IN  GT_U8 devNum
);

static void txqPdsInit_sip6
(
    IN  GT_U8 devNum,
    IN  GT_U32 pdsUnit
);


static void txqSdqInit_sip6
(
    IN  GT_U8 devNum,
    IN  GT_U32 sdqUnit
);


static void txqPsiInit_sip6
(
    IN  GT_U8 devNum
);

static void txqPfccInit_sip6
(
    IN  GT_U8 devNum
);

static void txqQfcInit_sip6
(
  IN  GT_U8 devNum,
  IN GT_U32 qfcUnit
);



static GT_STATUS packetBufferInit_sip6
(
    IN  GT_U8   devNum
);

static void rxdmaInit_sip6
(
    IN  GT_U8 devNum,
    IN  GT_U32  index,
    IN  GT_U32  lastRxDmaIndex,
    IN CPSS_PORTS_BMP_STC *validRxDmaBmpPtr
);


static void txdmaInit_sip6
(
    IN  GT_U8 devNum,
    IN  GT_U32  index,
    IN  GT_U32  lastRxDmaIndex,
    IN CPSS_PORTS_BMP_STC *validRxDmaBmpPtr
);


static void txfifoInit_sip6
(
    IN  GT_U8 devNum,
    IN  GT_U32  index,
    IN  GT_U32  lastRxDmaIndex,
    IN CPSS_PORTS_BMP_STC *validRxDmaBmpPtr
);

static void shmInit_sip6
(
    IN  GT_U8 devNum
);
static void sip6EagleD2dCpInit_regDb
(
    IN  GT_U8   devNum
);
static void phaInit_sip6
(
    IN  GT_U8 devNum
);


static GT_STATUS   bindToUnusedUnitsCheckFunction(IN GT_U8 devNum);

#define  NUM_ENTRIES_FOR_8_MSBITS_BASE_ADDR_CNS         BIT_8/*256*/

/* filled in runtime */
static PRV_CPSS_DXCH_UNIT_ENT prvDxChBobcat2UnitIdPer8MSBitsBaseAddrArr[NUM_ENTRIES_FOR_8_MSBITS_BASE_ADDR_CNS] =
{
    0,
};
static PRV_CPSS_DXCH_UNIT_ENT prvDxChBobcat3UnitIdPer8MSBitsBaseAddrArr[NUM_ENTRIES_FOR_8_MSBITS_BASE_ADDR_CNS] =
{
    0,
};
static PRV_CPSS_DXCH_UNIT_ENT prvDxChAldrinUnitIdPer8MSBitsBaseAddrArr[NUM_ENTRIES_FOR_8_MSBITS_BASE_ADDR_CNS] =
{
    0,
};
static PRV_CPSS_DXCH_UNIT_ENT prvDxChAldrin2UnitIdPer8MSBitsBaseAddrArr[NUM_ENTRIES_FOR_8_MSBITS_BASE_ADDR_CNS] =
{
    0,
};
/*  static PRV_CPSS_DXCH_UNIT_ENT prvDxChFalconUnitIdPer8MSBitsBaseAddrArr[NUM_ENTRIES_FOR_8_MSBITS_BASE_ADDR_CNS]
    NOTE: the Falcon not support 'fixed prefix' of 8 bits to the units.
    it supports 'flex ranges' each with different size , that not allow to use such array.

    see function prvCpssDxChFalconHwRegAddrToUnitIdConvert(...) that implement the logic.

*/

/* NOTE: next table (large units) not relevant to Falcon */
/*array for large units (Bobcat2; Caelum; Aldrin; AC3X; Bobcat3)*/
static struct{
    PRV_CPSS_DXCH_UNIT_ENT unitId;
    GT_U32                 numOfSubUnits;/* in 8 MSBits units*/
}prvDxChLargeUnitsArr[]=
{
     {PRV_CPSS_DXCH_UNIT_EGF_SHT_E,8}

    /* mast be last */
    ,{END_OF_TABLE_INDICATOR_CNS , 0}
};

/* Lion2 unit base address array */
static GT_U32 prvDxChLion2UnitsBaseAddr[PRV_CPSS_DXCH_UNIT_LAST_E + 1] =
{
/*PRV_CPSS_DXCH_UNIT___NOT_VALID___E*/ NON_VALID_BASE_ADDR_CNS             ,
/*UNIT_TTI_E,            */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_TTI_CNS    ,
/*UNIT_PCL_E,            */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_PCL_CNS    ,
/*UNIT_L2I_E,            */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_L2I_CNS    ,
/*UNIT_FDB_E,            */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_FDB_CNS    ,
/*UNIT_EQ_E,             */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_EQ_CNS     ,
/*UNIT_LPM_E,            */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_LPM_CNS    ,
/*UNIT_EGF_EFT_E,        */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_EGF_EFT_CNS,
/*UNIT_EGF_QAG_E,        */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_EGF_QAG_CNS,
/*UNIT_EGF_SHT_E,        */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_EGF_SHT_CNS,
/*UNIT_HA_E,             */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_HA_CNS     ,
/*UNIT_ETS_E,            */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_ETS_CNS    ,
/*UNIT_MLL_E,            */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_MLL_CNS    ,
/*UNIT_IPLR_E,           */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_IPLR_CNS   ,
/*UNIT_EPLR_E,           */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_EPLR_CNS   ,
/*UNIT_IPVX_E,           */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_IPVX_CNS   ,
/*UNIT_IOAM_E,           */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_IOAM_CNS   ,
/*UNIT_EOAM_E,           */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_EOAM_CNS   ,
/*UNIT_TCAM_E,           */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_TCAM_CNS   ,
/*UNIT_RXDMA_E,          */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_RXDMA_CNS  ,
/*UNIT_EPCL_E,           */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_EPCL_CNS   ,
/*UNIT_TM_FCU_E,         */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_TM_FCU_CNS                     ,
/*UNIT_TM_DROP_E,        */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_TM_DROP_CNS                    ,
/*UNIT_TM_QMAP_E,        */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_TM_QMAP_CNS                    ,
/*UNIT_TM_E,             */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_TM_CNS                         ,
/*UNIT_GOP_E,            */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_GOP_CNS                        ,
/*UNIT_MIB_E,            */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_MIB_CNS                        ,
/*UNIT_SERDES_E,         */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_SERDES_CNS                     ,
/*UNIT_ERMRK_E,          */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_ERMRK_CNS                      ,
/*UNIT_BM_E,             */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_BM_CNS                         ,
/*UNIT_TM_INGRESS_GLUE_E,*/    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_TM_INGRESS_GLUE_CNS            ,
/*UNIT_TM_EGRESS_GLUE_E, */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_TM_EGRESS_GLUE_CNS             ,
/*UNIT_ETH_TXFIFO_E,     */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_ETH_TXFIFO_CNS                 ,
/*UNIT_ILKN_TXFIFO_E,    */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_ILKN_TXFIFO_CNS                ,
/*UNIT_ILKN_E,           */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_ILKN_CNS                       ,
/*UNIT_LMS0_0_E,         */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_LMS0_0_CNS                     ,
/*UNIT_LMS0_1_E,         */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_LMS0_1_CNS                     ,
/*UNIT_LMS0_2_E,         */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_LMS0_2_CNS                     ,
/*UNIT_LMS0_3_E,         */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_LMS0_3_CNS                     ,
/*UNIT_TX_FIFO_E,        */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_TX_FIFO_CNS                    ,
/*UNIT_BMA_E,            */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_BMA_CNS                        ,
/*UNIT_CNC_0_E,          */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_CNC_0_CNS                      ,
/*UNIT_CNC_1_E,          */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_CNC_1_CNS                      ,
/*UNIT_TXQ_QUEUE_E,      */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_TXQ_QUEUE_CNS                  ,
/*UNIT_TXQ_LL_E,         */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_TXQ_LL_CNS                     ,
/*UNIT_TXQ_PFC_E,        */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_TXQ_PFC_CNS                    ,
/*UNIT_TXQ_QCN_E,        */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_TXQ_QCN_CNS                    ,
/*UNIT_TXQ_DQ_E,         */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_TXQ_DQ_CNS                     ,
/*UNIT_DFX_SERVER_E,     */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_DFX_SERVER_CNS                 ,
/*UNIT_MPPM_E,           */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_MPPM_CNS                       ,
/*UNIT_LMS1_0_E,         */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_LMS1_0_CNS                     ,
/*UNIT_LMS1_1_E,         */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_LMS1_1_CNS                     ,
/*UNIT_LMS1_2_E,         */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_LMS1_2_CNS                     ,
/*UNIT_LMS1_3_E,         */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_LMS1_3_CNS                     ,
/*UNIT_LMS2_0_E,         */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_LMS2_0_CNS                     ,
/*UNIT_LMS2_1_E,         */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_LMS2_1_CNS                     ,
/*UNIT_LMS2_2_E,         */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_LMS2_2_CNS                     ,
/*UNIT_LMS2_3_E,         */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_LMS2_3_CNS                     ,
/*UNIT_MPPM_1_E,         */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_MPPM_1_CNS                     ,
/*UNIT_CTU_0_E,          */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_CTU_0_CNS                      ,
/*UNIT_CTU_1_E,          */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_CTU_1_CNS                      ,
/*UNIT_TXQ_SHT_E ,       */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_TXQ_SHT_CNS                    ,
/*UNIT_TXQ_EGR0_E,       */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_TXQ_EGR0_CNS                   ,
/*UNIT_TXQ_EGR1_E,       */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_TXQ_EGR1_CNS                   ,
/*UNIT_TXQ_DIST_E,       */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_TXQ_DIST_CNS                   ,
/*UNIT_IPLR_1_E,         */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_IPLR_1_CNS                     ,
/*UNIT_TXDMA_E,          */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_TXDMA_CNS                      ,
/*UNIT_MG_E,             */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_MG_CNS                         ,
/*UNIT_TCC_PCL_E,        */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_TCC_PCL_CNS                    ,
/*UNIT_TCC_IPVX_E,       */    PRV_CPSS_DXCH_LION2_BASE_ADDRESS_TCC_IPVX_CNS                   ,

    END_OF_TABLE_INDICATOR_CNS/*last 'non valid' */
};

/* number of elements in unit */
#define NUM_ELEMENTS_IN_UNIT_MAC(unit,element) (sizeof(unit) / sizeof(element))

/* macro to set parameters for function regUnitBaseAddrSet(...) */
#define REG_UNIT_INFO_MAC(devNum, unitBase, unitName)                   \
        (void*)(&PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->unitName),    \
        NUM_ELEMENTS_IN_UNIT_MAC(PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->unitName , GT_U32) , \
        unitBase

/* macro to set parameters for function regUnitPerPortAddrUpdate(...) */
#define REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, unitName1,unitName2 , regName ,startIndexUnit1,startIndexUnit2,numOfPorts, \
        saveLastNumOfPorts,saveToIndex,localPortMapArr) \
        &PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->unitName1.regName[0],   \
        &PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->unitName2.regName[0],   \
        startIndexUnit1,                                                  \
        startIndexUnit2,                                                  \
        numOfPorts,                                                       \
        saveLastNumOfPorts,                                               \
        saveToIndex,                                                      \
        localPortMapArr


/* number of rx/tx DMA and txfifo 'per DP unit' */
#define BC3_NUM_PORTS_DMA   13

#define PRV_CPSS_DXCH_BOBCAT3_LPM_UNIT_OFFSET_GAP_MAC(lpmNum)              (lpmNum<<24)

/**
* @internal regUnitPerPortAddrUpdate function
* @endinternal
*
* @brief   This function replaces registers from unit2 to unit1.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] unitStart1Ptr            - pointer to start of the unit1
* @param[in] unitStart2Ptr            - pointer to start of the unit2
* @param[in] startIndexUnit1          - the number of ports to skip in unitStart1Ptr
* @param[in] startIndexUnit2          - the number of ports to skip in unitStart2Ptr
* @param[in] numOfPorts               - number of ports to copy
* @param[in] saveOrMoveLastNumOfPorts - number of ports from the end of the unitStart1Ptr[]
*                                      that need to be either
*                                      - saved before overridden by unitStart2Ptr[] when localPortMapArr == NULL
*                                      OR
*                                      - moved when localPortMapArr != NULL
*                                      saveToOrMoveIndex - the index in unitStart1Ptr[] to either save or move the 'last ports'
*                                      valid when saveOrMoveLastNumOfPorts != 0
*                                      meaning : copy unitStart2Ptr from index = startIndexUnit2 to index = (startIndexUnit2 + numOfPorts - 1)
*                                      into unitStart2Ptr from index = startIndexUnit1 to index = (startIndexUnit1 + numOfPorts - 1)
* @param[in] localPortMapArr          - array with index is relative port in Unit2 and value is local port in Unit2
*                                      may be NULL in 1:1 map
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_CPU_MEM        - on malloc failed
* @retval GT_BAD_PARAM             - wrong device type to operate
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static void regUnitPerPortAddrUpdate
(
    IN GT_U32    *unitStart1Ptr,
    IN GT_U32    *unitStart2Ptr,
    IN GT_U32    startIndexUnit1,
    IN GT_U32    startIndexUnit2,
    IN GT_U32    numOfPorts,
    IN GT_U32    saveOrMoveLastNumOfPorts,
    IN GT_U32    saveOrMoveToIndex,
    IN const GT_U32 *localPortMapArr
)
{
    GT_U32  ii,jj,kk;
    GT_U32  mm;

    ii = startIndexUnit1;
    jj = startIndexUnit2;

    if(localPortMapArr)
    {
        numOfPorts -= saveOrMoveLastNumOfPorts;
        for(kk = 0 ; kk < numOfPorts ; kk++)
        {
            unitStart1Ptr[ii] = unitStart2Ptr[localPortMapArr[jj]];
            ii++;
            jj++;
        }

        if(saveOrMoveLastNumOfPorts > 0)
        {
            ii = saveOrMoveToIndex;
            /* jj - continue from previous loop */
            /* move last indexes to other place - used for aldrin CPU port moving to position 72 */
            for(mm = 0; mm < saveOrMoveLastNumOfPorts; mm++)
            {
                unitStart1Ptr[ii] = unitStart2Ptr[localPortMapArr[jj]];
                ii++;
                jj++;
            }
        }
    }
    else
    {
        if(saveOrMoveLastNumOfPorts > 0)
            /* save last indexes to other place to will not be totally deleted */
            for(mm = 0; mm < saveOrMoveLastNumOfPorts; mm++)
            {
                unitStart1Ptr[saveOrMoveToIndex + mm] = unitStart1Ptr[numOfPorts - saveOrMoveLastNumOfPorts + mm];
            }

        for(kk = 0 ; kk < numOfPorts ; kk++)
        {
            unitStart1Ptr[ii] = unitStart2Ptr[jj];
            ii++;
            jj++;
        }
    }
}


extern void prvCpssDefaultAddressUnusedSet
(
    INOUT void   *startPtr,
    IN GT_U32   numOfRegs
);

#define STC_AND_SIZE_MAC(stcName) \
    (GT_U32*)&(stcName) , (sizeof(stcName) / 4)


#define DESTROY_STC_MAC(stcName) \
    regDestroyArr(STC_AND_SIZE_MAC(stcName))


#define ARRAY_AND_SIZE_MAC(arrRegName) \
    (arrRegName) , (sizeof(arrRegName) / sizeof((arrRegName)[0]))

/* destroy compilation array of registers */
#define DESTROY_COMPILATION_ARRAY_REG_MAC(arrRegName) \
    regDestroyArr(ARRAY_AND_SIZE_MAC(arrRegName))

/* destroy dynamic allocated array of registers */
#define DESTROY_DYNAMIC_ALLOC_ARRAY_REG_MAC(arrRegName)     \
    if(arrRegName)                                          \
    {                                                       \
        /* can free the allocation , and set as NULL */     \
        cpssOsFree(arrRegName);                             \
        arrRegName = NULL;                                  \
    }

/* destroy single register */
#define DESTROY_SINGLE_REG_MAC(regName) \
    regDestroy(&(regName))



/**
* @internal prvCpssDxChHwUnitBaseAddrGet function
* @endinternal
*
* @brief   This function retrieves base address of unit.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                      there are some special 'devNum' :
*                                      see LION2_DEVICE_INDICATION_CNS ,
*                                      BOBK_DEVICE_INDICATION_CNS ,
*                                      BC3_DEVICE_INDICATION_CNS
* @param[in] unitId                   - the ID of the address space unit.
*
* @param[out] errorPtr                 - (pointer to) indication that function did error.
*
* @retval on success               - return the base address
* @retval on error                 - return address that ((address % 4) != 0)
*/
GT_U32 prvCpssDxChHwUnitBaseAddrGet
(
    IN GT_U32                   devNum,
    IN PRV_CPSS_DXCH_UNIT_ENT   unitId,
    OUT GT_BOOL                 *errorPtr
)
{
    GT_U32  baseAddr = 1;/* default to cause fatal error on simulation */

    if(errorPtr)
    {
        *errorPtr = GT_TRUE;
    }

    if(unitId >= PRV_CPSS_DXCH_UNIT_LAST_E)
    {
        /* error */
        return baseAddr;
    }

    switch(devNum)
    {
        case LION2_DEVICE_INDICATION_CNS:
            /* indication the we need Lion2 DB regardless to the actual devNum */
            goto forceLion2_lbl;
        case BOBK_DEVICE_INDICATION_CNS:
            /* indication the we need Bobk DB regardless to the actual devNum */
            goto forceBobk_lbl;
        case BC3_DEVICE_INDICATION_CNS:
            /* indication the we need BC3 DB regardless to the actual devNum */
            goto forceBc3_lbl;
        case ALDRIN2_DEVICE_INDICATION_CNS:
            /* indication the we need Aldrin2 DB regardless to the actual devNum */
            goto forceAldrin2_lbl;
        case FALCON_DEVICE_INDICATION_CNS:
            /* indication the we need Falcon DB regardless to the actual devNum */
            goto forceFalcon_lbl;
        default:
            break;
    }

    /* need to use prvCpssDrvPpConfig[] because the 'DXCH DB' may not exists yet !
       (during phase1 init of the driver itself) */
    if(devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS ||
       NULL == prvCpssDrvPpConfig[devNum])
    {
        /* error */
        return baseAddr;
    }

    switch(prvCpssDrvPpConfig[devNum]->devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_LION2_E:
            forceLion2_lbl:
            baseAddr = prvDxChLion2UnitsBaseAddr[unitId];
            break;
        case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
            if(prvCpssDrvPpConfig[devNum]->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E)
            {
                forceBobk_lbl:
                baseAddr = prvDxChBobkUnitsBaseAddr[unitId];
            }
            else
            {
                baseAddr = prvDxChBobcat2UnitsBaseAddr[unitId];
            }
            break;
        case CPSS_PP_FAMILY_DXCH_BOBCAT3_E:
            forceBc3_lbl:
            baseAddr = prvDxChBobcat3UnitsBaseAddr[unitId];
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            forceAldrin2_lbl:
            baseAddr = prvDxChAldrin2UnitsBaseAddr[unitId];
            break;
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            forceFalcon_lbl:
            baseAddr = prvDxChFalconUnitsBaseAddr[unitId];
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
        case CPSS_PP_FAMILY_DXCH_AC3X_E:
            baseAddr = prvDxChAldrinUnitsBaseAddr[unitId];
            break;
        case CPSS_PP_FAMILY_DXCH_XCAT3_E:
            if( PRV_CPSS_DXCH_UNIT_GOP_E == unitId )
            {
                baseAddr = 0x12000000;
            }
            else if( PRV_CPSS_DXCH_UNIT_SERDES_E == unitId )
            {
                baseAddr = 0x13000000;
            }
            else if( PRV_CPSS_DXCH_UNIT_MIB_E == unitId )
            {
                baseAddr = 0x11000000;
            }
            break;
        default:
            /* error */
            return baseAddr;
    }

    if(errorPtr && (baseAddr != NON_VALID_BASE_ADDR_CNS))
    {
        *errorPtr = GT_FALSE;
    }

    if(baseAddr == NON_VALID_BASE_ADDR_CNS)
    {
        baseAddr |= 1;/* indication of not valid unit */
    }

    return baseAddr;
}

static PRV_CPSS_DXCH_UNIT_ENT gopPortUnits[MAX_PIPES_CNS] =
{
     PRV_CPSS_DXCH_UNIT_GOP_E
    ,PRV_CPSS_DXCH_UNIT_GOP1_E
};

/**
* @internal getGopPortBaseAddress function
* @endinternal
*
* @brief   get base address of port in the GOP
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - on bad state of logic
*/
static GT_U32  getGopPortBaseAddress(
    IN GT_U8    devNum,
    IN GT_U32   portNum
)
{
    GT_STATUS   rc;
    static GT_U32  prevPipeIndex = 0xFFFFFFFF;/* previous pipe index (in case of multi-pipe) */
    GT_U32         portIndex;/* 'local port' index (in case of multi-pipe) */
    GT_U32         pipeIndex;/* pipe index (in case of multi-pipe) */
    static GT_U32  gopBaseAddr = 0xFFFFFFFF; /* gop memory base address */

    if(!PRV_CPSS_IS_MULTI_PIPES_DEVICE_MAC(devNum))
    {
        return prvCpssDxChHwUnitBaseAddrGet(devNum,gopPortUnits[0],NULL);
    }

    rc = prvCpssDxChHwPpGopGlobalMacPortNumToLocalMacPortInPipeConvert(devNum,
        portNum/*global port*/,
        &pipeIndex,
        &portIndex/* local port */);
    if(rc != GT_OK)
    {
        return 0xFFFFFFFF;
    }

    /* optimization , calculate base addr only when unitIndex is changed */
    if(pipeIndex != prevPipeIndex)
    {
        /* get base address of the unit according to the pipeIndex */
        gopBaseAddr = prvCpssDxChHwUnitBaseAddrGet(devNum,gopPortUnits[pipeIndex],NULL);
    }

    /* update the previous pipeId */
    prevPipeIndex = pipeIndex;

    return gopBaseAddr;
}

/**
* @internal gopAndMibOffsetToAddressAdd function
* @endinternal
*
* @brief   This function add 'pipe offset' (according to pipeId) to MIB/GOP register addresses .
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] pipeId                   - the Pipe Id of the ports
* @param[in,out] startPtr                 - pointer to start of register addresses
* @param[in] numOfRegs                - number of registers to update
* @param[in,out] startPtr                 - pointer to start of register addresses that where initialized
*/
static void gopAndMibOffsetToAddressAdd
(
    IN GT_U8    devNum,
    IN GT_U32   pipeId,
    INOUT void  *startPtr,
    IN GT_U32   numOfRegs
)
{
    GT_U32        ii;
    GT_U32       *regsAddrPtr32;
    GT_U32        regsAddrPtr32Size;
    PRV_CPSS_DXCH_UNIT_ENT unit;
    GT_U32        gop_pipe0Base = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_GOP_E,NULL);
    GT_U32        gop_pipe1Base = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_GOP1_E,NULL);
    GT_U32        pipe1Offset;
    GT_U32        gop_pipe1Offset = gop_pipe1Base - gop_pipe0Base;
    GT_U32        mib_pipe0Base = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_MIB_E,NULL);
    GT_U32        mib_pipe1Base = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_MIB1_E,NULL);
    GT_U32        mib_pipe1Offset = mib_pipe1Base - mib_pipe0Base;
    GT_U32        raven2Base = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_BASE_RAVEN_2_E,NULL);
    GT_U32        raven_pipe1Offset = raven2Base;

    if(pipeId > 1)/* FALCON : 2..7 */
    {
        /* support Falcon 6.4/12.8 with 4 or 8 pipes */
        gop_pipe1Offset = prvCpssSip6OffsetFromFirstInstanceGet(devNum,pipeId,PRV_CPSS_DXCH_UNIT_GOP_E);
        mib_pipe1Offset = prvCpssSip6OffsetFromFirstInstanceGet(devNum,pipeId,PRV_CPSS_DXCH_UNIT_MIB_E);
    }


    /*Fill the all words in the pointer with initial value*/
    regsAddrPtr32       = startPtr;
    regsAddrPtr32Size   = numOfRegs;

    for( ii = 0; ii < regsAddrPtr32Size; ii++ )
    {
        if(regsAddrPtr32[ii] == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            continue;
        }

        unit = prvCpssDxChHwRegAddrToUnitIdConvert (devNum, regsAddrPtr32[ii]);
        switch(unit)
        {
            case PRV_CPSS_DXCH_UNIT_MIB_E:
                pipe1Offset = mib_pipe1Offset;
                break;
            case PRV_CPSS_DXCH_UNIT_GOP_E:
                pipe1Offset = gop_pipe1Offset;
                break;
            case PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E:
            case PRV_CPSS_DXCH_UNIT_BASE_RAVEN_1_E:
                pipe1Offset = raven_pipe1Offset;
                break;
            default:
                /* unknown unit ??? ... need special care */
                pipe1Offset = 1;/* will cause error somewhere */
                break;
        }
        regsAddrPtr32[ii] += pipe1Offset;
    }

    return;
}

/**
* @internal addressCopy function
* @endinternal
*
* @brief   This function copy register addresses , from src to dest .
*/
static void addressCopy
(
    INOUT void   *trgPtr,
    INOUT void   *srcPtr,
    IN GT_U32   numOfRegs
)
{
    GT_U32        ii;
    GT_U32       *regsAddrPtr32;
    GT_U32        regsAddrPtr32Size;
    GT_U32       *regsAddrPtr32_src = srcPtr;

    /*Fill the all words in the pointer with initial value*/
    regsAddrPtr32       = trgPtr;
    regsAddrPtr32Size   = numOfRegs;

    for( ii = 0; ii < regsAddrPtr32Size; ii++ )
    {
        regsAddrPtr32[ii] = regsAddrPtr32_src[ii];
    }

    return;
}

/**
* @internal updateGopOldRegDbWithMultiPipe function
* @endinternal
*
* @brief   update the 'old DB' of per port with values on 'multi-pipe'
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] pipeId                   - the Pipe Id for the GOP register to update
*                                      NOTE: pipeId should NOT be 0 !
*                                       none
*/
static void  updateGopOldRegDbWithMultiPipe(
    IN GT_U8    devNum,
    IN GT_U32   pipeId
)
{
    GT_U32        regsAddrPtr32Size;
    struct dxch_perPortRegs *portRegsPtr;
    GT_U32 portMacNum;
    GT_U32 portOffset;
    GT_U32 portMax;
    GT_U32       *regsAddrPtr32;
    GT_U32  portCpuIndexAtPipeId;
    GT_U32  portCpu_macCounters;

    portOffset = PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPortsPerPipe * pipeId;
    portMax    = PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPortsPerPipe;
    portCpuIndexAtPipeId = (pipeId-1) + PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPortsPerPipe * PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipes;
    portCpu_macCounters = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portCpuIndexAtPipeId].macCounters;/*save value*/

    /*Fill all the words in the struct with initial value*/
    regsAddrPtr32Size = sizeof(*portRegsPtr) / 4;

    if(prvCpssDrvPpConfig[devNum]->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E &&
       PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles == 4)
    {
        /* CPU ports are in pipe 0,1 ... not supported at this time */
        goto afterCpuPort_lbl;
    }


    if(pipeId == 1)
    {
        /* 1. copy CPU port of pipe 0 from 'local position' to 'global position' (first CPU port) */
        portMacNum = portMax;
        portRegsPtr = &PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portCpuIndexAtPipeId];
        /* copy addresses from pipe 0 */
        addressCopy(portRegsPtr,
            &PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portMacNum],
            regsAddrPtr32Size);
        /*restore mac counter that was set by function bcat2MacMibCountersInit(...) */
        PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portCpuIndexAtPipeId].macCounters = portCpu_macCounters;
    }

    if(prvCpssDrvPpConfig[devNum]->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
    {
        if(pipeId == 1)
        {
           if(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles >= 2)
           {
                /* NOTE: if we have 2 CPU ports we can't update the addresses with 'pipe 1' offset
                   we must keep 'first CPU' with addresses of 'pipe 0' until copy */

                /* we have second CPU GOP port , copy it from first CPU port */
                portMacNum = portCpuIndexAtPipeId;
                portRegsPtr = &PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portCpuIndexAtPipeId+1];
                /* copy addresses from pipe 0 */
                addressCopy(portRegsPtr,
                    &PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portMacNum],
                    regsAddrPtr32Size);
           }

           /* 3. set the first CPU port  with pipe 1 offset */
            portMacNum = portCpuIndexAtPipeId;
            portRegsPtr = &PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portMacNum];
            /* add offsets of pipe 1 */
            gopAndMibOffsetToAddressAdd(devNum,pipeId, portRegsPtr,regsAddrPtr32Size);
        }

        if(pipeId == 2)
        {
           /* 4. set the second CPU port  with pipe 1 offset */
            portMacNum = portCpuIndexAtPipeId;
            portRegsPtr = &PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portMacNum];
            /* add offsets of pipe 1 */
            gopAndMibOffsetToAddressAdd(devNum,pipeId, portRegsPtr,regsAddrPtr32Size);
        }
    }
    else
    {
        /* 2. copy port 72 to 73 with pipe 1 offset */
        portMacNum = portCpuIndexAtPipeId;
        portRegsPtr = &PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portMacNum + 1];
        /* copy addresses from pipe 0 */
        addressCopy(portRegsPtr,
            &PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portMacNum],
            regsAddrPtr32Size);
        /* add offsets of pipe 1 */
        gopAndMibOffsetToAddressAdd(devNum,pipeId, portRegsPtr,regsAddrPtr32Size);
    }

afterCpuPort_lbl:
    /* 3. copy ports 0..35 to ports 36..71 with pipe 1 offset */
    for(portMacNum = 0 ; portMacNum < portMax ; portMacNum++)
    {
        /* support macs that not exists */
        PRV_CPSS_DXCH_SKIP_NOT_EXIST_PORT_MAC(devNum, portMacNum);

        portRegsPtr = &PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portMacNum + portOffset];

        /* copy addresses from pipe 0 */
        addressCopy(portRegsPtr,
            &PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portMacNum],
            regsAddrPtr32Size);

        /* add offsets of pipe 1 */
        gopAndMibOffsetToAddressAdd(devNum,pipeId, portRegsPtr,regsAddrPtr32Size);
    }

    if(!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* MAC TG register addresses */
        regsAddrPtr32Size = sizeof(PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->packGenConfig[0]) / 4;
        for(portMacNum = 0 ; portMacNum < portMax ; portMacNum+=4)
        {
            /* support macs that not exists */
            PRV_CPSS_DXCH_SKIP_NOT_EXIST_PORT_MAC(devNum, portMacNum);

            regsAddrPtr32 = (GT_U32*)&PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->packGenConfig[(portOffset + portMacNum)/4];

            /* copy addresses from pipe 0 */
            addressCopy(regsAddrPtr32,
                &PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->packGenConfig[ (portMacNum/4)],
                regsAddrPtr32Size);

            /* add offsets of pipe 1 */
            gopAndMibOffsetToAddressAdd(devNum,pipeId, regsAddrPtr32,regsAddrPtr32Size);
        }
    }

    if(!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* FCA register addresses */
        regsAddrPtr32Size = sizeof(PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.FCA[0]) / 4;
        for(portMacNum = 0 ; portMacNum < portMax ; portMacNum++)
        {
            /* support macs that not exists */
            PRV_CPSS_DXCH_SKIP_NOT_EXIST_PORT_MAC(devNum, portMacNum);

            regsAddrPtr32 = (GT_U32*)&PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.FCA[portOffset + portMacNum];

            /* copy addresses from pipe 0 */
            addressCopy(regsAddrPtr32,
                &PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.FCA[portMacNum],
                regsAddrPtr32Size);

            /* add offsets of pipe 1 */
            gopAndMibOffsetToAddressAdd(devNum,pipeId, regsAddrPtr32,regsAddrPtr32Size);
        }
    }

    /* PTP register addresses */
    /* Number of 4 bytes (GT_U32) fields in a single element of the unit array */
    regsAddrPtr32Size = sizeof(PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.PTP[0]) / 4;
    for(portMacNum = 0 ; portMacNum < portMax ; portMacNum++)
    {
        /* support macs that not exists */
        PRV_CPSS_DXCH_SKIP_NOT_EXIST_PORT_MAC(devNum, portMacNum);

        /* Pointer to the fields of an element in the unit array
           that correspond to the current portMac at pipe 1 */
        regsAddrPtr32 = (GT_U32*)&PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.PTP[portOffset + portMacNum];

        /* Copy the fields values from the current portMac at pipe 0
           to the corresponding fields of portMac at pipe 1 */
        addressCopy(regsAddrPtr32,
            &PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.PTP[portMacNum],
            regsAddrPtr32Size);

        /* Add offset of pipe 1 to all fields of the portMac at pipe 1*/
        gopAndMibOffsetToAddressAdd(devNum,pipeId, regsAddrPtr32,regsAddrPtr32Size);
    }

    if(!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* CG register addresses */
        /* Number of 4 bytes (GT_U32) fields in a single element of the unit array */
        regsAddrPtr32Size = sizeof(PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.CG[0]) / 4;
        for(portMacNum = 0 ; portMacNum < portMax ; portMacNum++)
        {
            /* support macs that not exists */
            PRV_CPSS_DXCH_SKIP_NOT_EXIST_PORT_MAC(devNum, portMacNum);

            /* Pointer to the fields of an element in the unit array
               that correspond to the current portMac at pipe 1 */
            regsAddrPtr32 = (GT_U32*)&PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.CG[portOffset + portMacNum];

            /* Copy the fields values from the current portMac at pipe 0
               to the corresponding fields of portMac at pipe 1 */
            addressCopy(regsAddrPtr32,
                &PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.CG[portMacNum],
                regsAddrPtr32Size);

            /* Add offset of pipe 1 to all fields of the portMac at pipe 1*/
            gopAndMibOffsetToAddressAdd(devNum,pipeId, regsAddrPtr32,regsAddrPtr32Size);
        }
    }
}


/**
* @internal aldrin2SubunitBaseAddrGet function
* @endinternal
*
* @brief   This function retrieves base address of subunit.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] subunitId                - the ID of the address space unit.
* @param[in] indexArr                 - (pointer to) array of indexes that identify the
*                                      subunit instance.
*
* @retval on success               - return the base address
* @retval on error                 - return address that ((address % 4) != 0)
*
* @note For multi-port-group devices expected local port.
*
*/
static GT_U32 aldrin2SubunitBaseAddrGet
(
    IN GT_U8                       devNum,
    IN PRV_CPSS_SUBUNIT_ENT        subunitId,
    IN GT_U32                      *indexArr
)
{
    GT_U32 gopBase;
    GT_U32 subUnitOffset;
    PRV_CPSS_DXCH_UNIT_ENT  unitId;
    GT_U32  portNum;

    switch (subunitId)
    {
        case PRV_CPSS_SUBUNIT_GOP_TAI_E:
            /* two TAI units */
            if(indexArr[1] == 0)
            {
                unitId = PRV_CPSS_DXCH_UNIT_TAI_E;
            }
            else
            if(indexArr[1] == 1)
            {
                unitId = PRV_CPSS_DXCH_UNIT_TAI1_E;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(1, LOG_ERROR_NO_MSG);
            }

            return prvCpssDxChHwUnitBaseAddrGet(devNum,unitId,NULL);

        case PRV_CPSS_SUBUNIT_GOP_PTP_E:
        case PRV_CPSS_SUBUNIT_GOP_FCA_E:
            if(PRV_CPSS_SUBUNIT_GOP_FCA_E == subunitId)
            {
                subUnitOffset = 0x00180600;
            }
            else
            {
                subUnitOffset = 0x00180800;
            }

            portNum = indexArr[0];
            if(portNum < 73)
            {
                hwsAldrin2GopAddrCalc(portNum/*port number*/,subUnitOffset/*relative offset*/,&gopBase);
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(1, LOG_ERROR_NO_MSG);
            }

            return gopBase;
        case PRV_CPSS_SUBUNIT_GOP_CG_E:
            portNum = indexArr[0];

            /* CG port is supported on ports by multiples of 4 and up to 23*/
            if ( portNum > 23 || ( (portNum%4) !=0 ) )
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(1, LOG_ERROR_NO_MSG);
            }

            hwsAldrin2GopAddrCalc(portNum/*port number*/,0/*relative offset*/,&gopBase);

            return gopBase;
        default:
            break;
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(1, LOG_ERROR_NO_MSG);
}

/**
* @internal falconTaiSubunitBaseAddrGet function
* @endinternal
*
* @brief   This function retrieves base address of TAI subunit.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] indexArr                 - (pointer to) array of indexes that identify the
*                                      subunit instance.
*
* @retval on success               - return the base address
* @retval on error                 - return address that ((address % 4) != 0)
*
*
*/
static GT_U32 falconTaiSubunitBaseAddrGet
(
    IN GT_U8                       devNum,
    IN GT_U32                      *indexArr
)
{
    GT_U32 baseAddr;
    GT_U32 unitIndex;
    GT_U32 tileId;
    PRV_CPSS_DXCH_UNIT_ENT  unitId;
    PRV_CPSS_DXCH_UNIT_ENT  subUnit0Array[FALCON_PER_TILE_TAI_SUB_UNITS_NUMBER_CNS] =
    {
        PRV_CPSS_DXCH_UNIT_TAI_E,
        PRV_CPSS_DXCH_UNIT_TAI_SLAVE_PIPE0_TAI0_E,
        PRV_CPSS_DXCH_UNIT_TAI_SLAVE_PIPE1_TAI0_E,
        PRV_CPSS_DXCH_UNIT_TXQ_TAI_SLAVE_PIPE0_TAI0_E,
        PRV_CPSS_DXCH_UNIT_TXQ_TAI_SLAVE_PIPE1_TAI0_E,
        PRV_CPSS_DXCH_UNIT_RAVEN_0_TAI0_E,
        PRV_CPSS_DXCH_UNIT_RAVEN_1_TAI0_E,
        PRV_CPSS_DXCH_UNIT_RAVEN_2_TAI0_E,
        PRV_CPSS_DXCH_UNIT_RAVEN_3_TAI0_E
    };

    tileId = indexArr[0] / FALCON_PER_TILE_TAI_SUB_UNITS_NUMBER_CNS;

    /* 9 TAI units per tile */
    /* indexArr[0] is TAI subunit */
    /* indexArr[1] is TAI index */
    unitIndex = indexArr[0] % FALCON_PER_TILE_TAI_SUB_UNITS_NUMBER_CNS;

    unitId = subUnit0Array[unitIndex];

    if(indexArr[1] == 1)
    {
        unitId = unitId + 1;
    }

    baseAddr = prvCpssDxChHwUnitBaseAddrGet(devNum, unitId, NULL);

    /* Add tile relative address */
    return baseAddr + FALCON_TILE_OFFSET_CNS * tileId;
}

/**
* @internal falconSubunitBaseAddrGet function
* @endinternal
*
* @brief   This function retrieves base address of subunit.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] subunitId                - the ID of the address space unit.
* @param[in] indexArr                 - (pointer to) array of indexes that identify the
*                                      subunit instance.
*
* @retval on success               - return the base address
* @retval on error                 - return address that ((address % 4) != 0)
*
* @note For multi-port-group devices expected local port.
*
*/
static GT_U32 falconSubunitBaseAddrGet
(
    IN GT_U8                       devNum,
    IN PRV_CPSS_SUBUNIT_ENT        subunitId,
    IN GT_U32                      *indexArr
)
{
    GT_U32 gopBase;
    GT_U32 subUnitOffset;
    GT_U32  portNum;

    /* currently Falcon as BC3 */
    CPSS_TBD_BOOKMARK_FALCON

    switch (subunitId)
    {
        case PRV_CPSS_SUBUNIT_GOP_TAI_E:

            return falconTaiSubunitBaseAddrGet(devNum, indexArr);

#ifndef FALCON_ASIC_SIMULATION
        case PRV_CPSS_SUBUNIT_GOP_PTP_E:
            portNum = indexArr[0];
            subUnitOffset = 0;/* we call explicit unit 'RAVEN_TSU' */
            hwsFalconAddrCalc(devNum,   HWS_UNIT_BASE_ADDR_TYPE_RAVEN_TSU_E ,
                portNum/*port number*/,subUnitOffset/*relative offset*/,&gopBase);
            return gopBase;
#else   /*FALCON_ASIC_SIMULATION*/
        case PRV_CPSS_SUBUNIT_GOP_PTP_E:
#endif /*FALCON_ASIC_SIMULATION*/
        case PRV_CPSS_SUBUNIT_GOP_FCA_E:
            if(PRV_CPSS_SUBUNIT_GOP_FCA_E == subunitId)
            {
                subUnitOffset = 0x00180600;
            }
            else
            {

                subUnitOffset = 0x00180800;
            }

            portNum = indexArr[0];
            hwsFalconGopAddrCalc(devNum,portNum/*port number*/,subUnitOffset/*relative offset*/,&gopBase);

            return gopBase;
        case PRV_CPSS_SUBUNIT_GOP_CG_E:
            portNum = indexArr[0];

            /* CG port is supported on ports by multiples of 4 */
            /* the last one or 2 ports are GOP CPU ports , that are not CG */
            if ( portNum >= (GT_U32)(PRV_CPSS_PP_MAC(devNum)->numOfPorts-2) ||
               ( (portNum%4) !=0 ) )
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(1, LOG_ERROR_NO_MSG);
            }

            hwsFalconGopAddrCalc(devNum,portNum/*port number*/,0/*relative offset*/,&gopBase);

            return gopBase;

        default:
            break;
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(1, LOG_ERROR_NO_MSG);
}

/**
* @internal prvCpssDxChHwSubunitBaseAddrGet function
* @endinternal
*
* @brief   This function retrieves base address of subunit.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] subunitId                - the ID of the address space unit.
* @param[in] indexArr                 - (pointer to) array of indexes that identify the
*                                      subunit instance.
*
* @retval on success               - return the base address
* @retval on error                 - return address that ((address % 4) != 0)
*
* @note For multi-port-group devices expected local port.
*
*/
GT_U32 prvCpssDxChHwSubunitBaseAddrGet
(
    IN GT_U8                       devNum,
    IN PRV_CPSS_SUBUNIT_ENT   subunitId,
    IN GT_U32                      *indexArr
)
{
    GT_U32 gopBase = 0;
    GT_U32 subUnitOffset;
    PRV_CPSS_DXCH_UNIT_ENT  unitId;
    GT_U32  portNum;
    GT_U32 portNumOffset;

    if ((devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS) ||
       (NULL == prvCpssDrvPpConfig[devNum]))
    {
        return 1;/* error */
    }

    if(prvCpssDrvPpConfig[devNum]->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
    {
        return aldrin2SubunitBaseAddrGet(devNum,subunitId,indexArr);
    }
    if(prvCpssDrvPpConfig[devNum]->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
    {
        return falconSubunitBaseAddrGet(devNum,subunitId,indexArr);
    }

    if(prvCpssDrvPpConfig[devNum]->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
    {
        /* In BC3 we fill the first 36 ports unit, the we copy pase them to the rest
            36 ports with the proper pipe offset */
        switch (subunitId)
        {
            case PRV_CPSS_SUBUNIT_GOP_TAI_E:
                /* two TAI units */
                /* indexArr[0] is GOP index not relevant for BC3 */
                /* indexArr[1] is TAI index */
                if(indexArr[1] == 0)
                {
                    unitId = PRV_CPSS_DXCH_UNIT_TAI_E;
                }
                else
                if(indexArr[1] == 1)
                {
                    unitId = PRV_CPSS_DXCH_UNIT_TAI1_E;
                }
                else
                {
                    return 1; /* error */
                }

                return prvCpssDxChHwUnitBaseAddrGet(devNum,unitId,NULL);

            case PRV_CPSS_SUBUNIT_GOP_PTP_E:
            case PRV_CPSS_SUBUNIT_GOP_FCA_E:
                if(PRV_CPSS_SUBUNIT_GOP_FCA_E == subunitId)
                {
                    subUnitOffset = 0x00180600;
                }
                else
                {
                    subUnitOffset = 0x00180800;
                }

                portNum = indexArr[0];
                /* Base + pipe offset */
                gopBase = getGopPortBaseAddress(devNum,portNum);

                /* Ports 72 and 73 are actually ports 36 in pipe 0 and pipe 1 */
                if (portNum <= 71)
                {
                    portNumOffset = (portNum % 36);
                }
                else if ((portNum == 72) || (portNum == 73))
                {
                    portNumOffset = 36;
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }

                return gopBase + subUnitOffset + (0x1000 * portNumOffset);
            case PRV_CPSS_SUBUNIT_GOP_CG_E:
                portNum = indexArr[0];

                /* CG port is supported on ports by multiples of 4 and up to 71*/
                if ( portNum>71 || ( (portNum%4) !=0 ) )
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }

                /* Base + pipe offset */
                gopBase = getGopPortBaseAddress(devNum,portNum);

                /* Port offset: 36 ports (0-35) with CG units */
                return gopBase + ( (portNum%36) * 0x1000 );
            default:
                break;
        }

        return 1;/* error */
    }

    if((prvCpssDrvPpConfig[devNum]->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E) ||
        (prvCpssDrvPpConfig[devNum]->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E))
    {
        switch (subunitId)
        {
            case PRV_CPSS_SUBUNIT_GOP_TAI_E:
                /* single TAI unit */
                unitId = PRV_CPSS_DXCH_UNIT_TAI_E;
                return prvCpssDxChHwUnitBaseAddrGet(devNum,unitId,NULL);

            case PRV_CPSS_SUBUNIT_GOP_PTP_E:
            case PRV_CPSS_SUBUNIT_GOP_FCA_E:
                if(PRV_CPSS_SUBUNIT_GOP_FCA_E == subunitId)
                {
                    subUnitOffset = 0x00180600;
                }
                else
                {
                    subUnitOffset = 0x00180800;
                }

                portNum = indexArr[0];
                gopBase = getGopPortBaseAddress(devNum,portNum);

                return gopBase + subUnitOffset + (0x1000 * indexArr[0]);
            default:
                break;
        }

        return 1;/* error */
    }

    switch (prvCpssDrvPpConfig[devNum]->devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
            gopBase = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_GOP_E,NULL);
            break;
        default:
            return 1;/* error */
    }

    switch (prvCpssDrvPpConfig[devNum]->devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
            switch (subunitId)
            {
                case PRV_CPSS_SUBUNIT_GOP_TAI_E:

                    /* pairs of subunits per GOP   */
                    /* indexArr[0] is GOP index 0-8, ILKN index 9*/
                    /* indexArr[1] is TAI index 0-1*/

                    if (indexArr[1] > 1)
                    {
                        return 1; /* error */
                    }

                    if (indexArr[0] < 3)
                    {
                        /* GOPs 0-2 for ports 0-15, 16-31, 32-48 */
                        return
                            (gopBase + 0x00180A00 +
                             (0x1000 * (indexArr[1] + (indexArr[0] * 16))));
                    }
                    else if (indexArr[0] < 5)
                    {
                        /* GOPs 3-4 for ports 48-51, 52-55 */
                        return
                            (gopBase + 0x00180A00 +
                             (0x1000 * (indexArr[1] + 48 + ((indexArr[0] - 3) * 4))));
                    }
                    else if (indexArr[0] < 9)
                    {
                        /* GOPs 5-8 for ports 56-59, 60-63, 64-67, 68-71 */
                        return
                            (gopBase + 0x00180A00 + 0x200000 +
                             (0x1000 * (indexArr[1] + ((indexArr[0] - 5) * 4))));
                    }
                    else if (indexArr[0] < 10)
                    {
                        /* ILKN - only for adress calculation                   */
                        /* correct even for devices that have no ILKN interface */
                        /* It is address of Global TAI                          */
                        gopBase =
                            prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_ILKN_E,NULL);

                        return
                            (gopBase + 0x00002800 + (0x800 * (indexArr[1])));
                    }
                    else
                    {
                        return 1; /* error */
                    }

                case PRV_CPSS_SUBUNIT_GOP_PTP_E:
                case PRV_CPSS_SUBUNIT_GOP_FCA_E:
                    if(PRV_CPSS_SUBUNIT_GOP_FCA_E == subunitId)
                    {
                        subUnitOffset = 0x00180600;
                    }
                    else
                    {
                        subUnitOffset = 0x00180800;
                    }

                    /* subunit per port 0-71      */
                    /* indexArr[0] is port number */
                    if (indexArr[0] < 56)
                    {
                        /* Bobcat2 network ports */
                        return (gopBase + subUnitOffset + (0x1000 * indexArr[0]));
                    }
                    else if (indexArr[0] < 72)
                    {
                        /* Bobcat2 fabric ports */
                        return
                            (gopBase + subUnitOffset + 0x200000 +
                                (0x1000 * (indexArr[0] - 56)));
                    }
                    else if ((PRV_CPSS_SUBUNIT_GOP_PTP_E == subunitId) &&
                             (indexArr[0] < 73 ))
                    {
                        /* ILKN */
                        if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_ilkn.supported == GT_FALSE)
                        {
                            return 0xFFFFFFFF;/* need to ignore */
                        }
                        /* WA due to devcie stuck problam while accessing this */
                        /* registers range - meanwhile accessing the PTP       */
                        /* registers range of the "previous" PTP.              */
                        return
                            (gopBase + subUnitOffset + 0x200000 +
                                (0x1000 * ((indexArr[0]-1) - 56)));
                        /*
                        gopBase =
                            prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_ILKN_E,NULL);
                        return
                            (gopBase + 0x2000);
                        */
                    }
                    else
                    {
                        return 1; /* error */
                    }
                case PRV_CPSS_SUBUNIT_GOP_PR_E:
                    /* actually ILKN unit memory space : 0x1F000000 !!! */
                    subUnitOffset = 0x0F000000;
                    return (gopBase + subUnitOffset);

                default:
                    return 1; /* error */
            }
            break;
        default:
            return 1;/* error */
    }
}

/**
* @internal regDestroy function
* @endinternal
*
* @brief   destroy register info --> the register not relevant to the device
*
* @param[in] regPtr                   - pointer to the register info.
*                                       None.
*/
static void regDestroy(
    IN GT_U32 *regPtr
)
{
    *regPtr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
    return;
}
/**
* @internal regDestroyArr function
* @endinternal
*
* @brief   destroy array of registers info --> the registers not relevant to the device
*
* @param[in] regArrPtr                - pointer to the registers array info.
* @param[in] numOfElements            - number of elements
*                                       None.
*/
static void regDestroyArr(
    IN GT_U32 *regArrPtr,
    IN GT_U32 numOfElements
)
{
    prvCpssDefaultAddressUnusedSet(regArrPtr,numOfElements);
}

/**
* @internal dbReset function
* @endinternal
*
* @brief   reset the DB
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None
*/
static void dbReset
(
    IN  GT_U8 devNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_U32       *regsAddrPtr32;
    GT_U32        regsAddrPtr32Size;

    /*Fill all the words in the struct with initial value*/
    regsAddrPtr32 = (GT_U32*)regsAddrPtr;
    regsAddrPtr32Size = sizeof(PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC) / 4;

    prvCpssDefaultAddressUnusedSet(regsAddrPtr32,regsAddrPtr32Size);
}

/**
* @internal resetAndInitControllerDbReset function
* @endinternal
*
* @brief   reset the Reset and Init Controller DB
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None
*/
static void resetAndInitControllerDbReset
(
    IN  GT_U8 devNum
)
{
    PRV_CPSS_RESET_AND_INIT_CTRL_REGS_ADDR_STC *regsAddrPtr;
    GT_U32       *regsAddrPtr32;
    GT_U32        regsAddrPtr32Size;

    regsAddrPtr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum);

    /*Fill all the words in the struct with initial value*/
    regsAddrPtr32 = (GT_U32*)regsAddrPtr;
    regsAddrPtr32Size =
                    sizeof(PRV_CPSS_RESET_AND_INIT_CTRL_REGS_ADDR_STC)/4;

    prvCpssDefaultAddressUnusedSet(regsAddrPtr32,regsAddrPtr32Size);
}

/**
* @internal regUnitBaseAddrSet function
* @endinternal
*
* @brief   This function replaces base address of all registers in this unit.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] unitStartPtr             - pointer to start of the unit
* @param[in] numOfElements            - number of elements in the unit (num of registers)
* @param[in] newBaseAddr              - new base address for the unit
*
* @param[out] unitStartPtr             - unit register are updated
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_CPU_MEM        - on malloc failed
* @retval GT_BAD_PARAM             - wrong device type to operate
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static void regUnitBaseAddrSet(
    IN GT_U32    *unitStartPtr,
    IN GT_U32    numOfElements,
    IN GT_U32    newBaseAddr
)
{
    GT_U32  ii;

    for(ii = 0 ; ii < numOfElements ; ii++)
    {
        if(unitStartPtr[ii] == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            /* skip registers that are not used in the 'legacy device' */
            continue;
        }

        if(newBaseAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            /* the whole unit is not valid ! --> set all addresses to 'not used' */
            unitStartPtr[ii] = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
        }
        else
        {
            /* addresses must be '0' based -- to support 'huge units' */
            unitStartPtr[ii] = unitStartPtr[ii] + newBaseAddr;
        }
    }
}

/**
* @internal ttiInit function
* @endinternal
*
* @brief   init the DB - TTI
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None
*/
static void ttiInit
(
    IN  GT_U8 devNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_U32  isSip5_20 = PRV_CPSS_SIP_5_20_CHECK_MAC(devNum);
    GT_U32  isSip6 = PRV_CPSS_SIP_6_CHECK_MAC(devNum);

    {/*start of unit TTI */
        {/*start of unit VLANAssignment */
            {/*300+i*4*/
                GT_U32    i;
                for(i = 0 ; i <= 3 ; i++) {
                    regsAddrPtr->TTI.VLANAssignment.ingrTPIDConfig[i] =
                        0x300+i*4;
                }/* end of loop i */
            }/*300+i*4*/
            {/*310+(0x8*t)+(0x4*p)*/
                GT_U32    t,p;
                for(t = 0 ; t <= 1 ; t++) {
                    for(p = 0 ; p <= 1 ; p++) {
                        regsAddrPtr->TTI.VLANAssignment.ingrTagProfileTPIDSelect[t][p] =
                            0x310+(0x8*t)+(0x4*p);
                    }/* end of loop p */
                }/* end of loop t */
            }/*310+(0x8*t)+(0x4*p)*/
            regsAddrPtr->TTI.VLANAssignment.ingrTPIDTagType = 0x00000320;

        }/*end of unit VLANAssignment */


        {/*start of unit userDefinedBytes */
            {/*b0+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 7 ; n++) {
                    regsAddrPtr->TTI.userDefinedBytes.IPCLTCPPortRangeComparatorConfig[n] =
                        0xb0+n*0x4;
                }/* end of loop n */
            }/*b0+n*0x4*/
            {/*d0+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 7 ; n++) {
                    regsAddrPtr->TTI.userDefinedBytes.IPCLUDPPortRangeComparatorConfig[n] =
                        0xd0+n*0x4;
                }/* end of loop n */
            }/*d0+n*0x4*/

        }/*end of unit userDefinedBytes */


        {/*start of unit TTIUnitGlobalConfigs */
            regsAddrPtr->TTI.TTIUnitGlobalConfigs.TTIUnitGlobalConfig = 0x00000000;
            regsAddrPtr->TTI.TTIUnitGlobalConfigs.TTIUnitGlobalConfigForCCPorts = 0x00000064;
            regsAddrPtr->TTI.TTIUnitGlobalConfigs.TTIUnitGlobalConfigExt = 0x00000140;

        }/*end of unit TTIUnitGlobalConfigs */


        {/*start of unit TTIEngine */
            regsAddrPtr->TTI.TTIEngine.TTIEngineInterruptCause = 0x00000004;
            regsAddrPtr->TTI.TTIEngine.TTIEngineInterruptMask = 0x00000008;
            regsAddrPtr->TTI.TTIEngine.TTIEngineConfig = 0x0000000c;
            regsAddrPtr->TTI.TTIEngine.TTIPCLIDConfig0 = 0x00000010;
            regsAddrPtr->TTI.TTIEngine.TTIPCLIDConfig1 = 0x00000014;
            regsAddrPtr->TTI.TTIEngine.TTILookupIPExceptionCommands = 0x00000068;
            {/*120+0x4*p*/
                GT_U32    p;
                for(p = 0 ; p <= 7 ; p++) {
                    regsAddrPtr->TTI.TTIEngine.TTIPktTypeKeys[p] =
                        0x120+0x4*p;
                }/* end of loop p */
            }/*120+0x4*p*/
            regsAddrPtr->TTI.TTIEngine.TTILookupIPExceptionCodes0 = 0x00000110;
            regsAddrPtr->TTI.TTIEngine.TTIKeySegmentMode0 = 0x00000114;
            regsAddrPtr->TTI.TTIEngine.TTIKeyType = 0x00000118;
            regsAddrPtr->TTI.TTIEngine.TTILookupMPLSExceptionCommands = 0x0000011c;
            regsAddrPtr->TTI.TTIEngine.TTILookupIPExceptionCodes1 = 0x00000144;
            /*only in Falcon*/
            if (isSip6)
            {
                {/*250+0x4*p*/
                    GT_U32    p;
                    for(p = 0 ; p <= 9 ; p++) {
                        regsAddrPtr->TTI.TTIEngine.TTIPacketTypeExactMatchProfileId[p] =
                            0x250+0x4*p;
                    }/* end of loop p */
                }/*250+0x4*p*/
            }/*end of isSip6 */


        }/*end of unit TTIEngine */


        {/*start of unit trunkHashSimpleHash */
            regsAddrPtr->TTI.trunkHashSimpleHash.trunkHashConfigReg0 = 0x00000084;
            regsAddrPtr->TTI.trunkHashSimpleHash.trunkHashConfigReg1 = 0x00000088;
            regsAddrPtr->TTI.trunkHashSimpleHash.trunkHashConfigReg2 = 0x0000008c;

        }/*end of unit trunkHashSimpleHash */

        /* Only In Aldrin/xCat3x (not in BC3/ALDRIN2) */
        if(PRV_CPSS_SIP_5_16_CHECK_MAC(devNum) &&
           !PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
        {/*start of unit SOHO CFCC */
            regsAddrPtr->TTI.sohoCcfc.sohoCcfcGlobal = 0x000001e8;
            {/*01ec+s*4*/
                GT_U32    s;
                for(s = 0 ; s <= 3 ; s++) {
                    regsAddrPtr->TTI.sohoCcfc.sohoCcfcSpeedToTimer[s] =
                        0x01ec+s*0x4;
                }/* end of loop s */
            }/*01ec+s*4*/

        }/*end of unit SOHO CFCC */


        {/*start of unit TRILLConfigs */
            regsAddrPtr->TTI.TRILLConfigs.TRILLGlobalConfigs0 = 0x00000200;
            regsAddrPtr->TTI.TRILLConfigs.TRILLGlobalConfigs1 = 0x00000204;
            regsAddrPtr->TTI.TRILLConfigs.TRILLExceptionCommands0 = 0x00000208;
            regsAddrPtr->TTI.TRILLConfigs.TRILLExceptionCommands1 = 0x0000020c;
            regsAddrPtr->TTI.TRILLConfigs.TRILLGlobalConfigs2 = 0x00000210;

        }/*end of unit TRILLConfigs */


        {/*start of unit TRILLAdjacency */
            {/*6008+e*0xc*/
                GT_U32    e;
                for(e = 0 ; e <= 127 ; e++) {
                    regsAddrPtr->TTI.TRILLAdjacency.adjacencyEntryReg2[e] =
                        0x6008+e*0xc;
                }/* end of loop e */
            }/*6008+e*0xc*/
            {/*6004+0xc*e*/
                GT_U32    e;
                for(e = 0 ; e <= 127 ; e++) {
                    regsAddrPtr->TTI.TRILLAdjacency.adjacencyEntryReg1[e] =
                        0x6004+0xc*e;
                }/* end of loop e */
            }/*6004+0xc*e*/
            {/*6000+e*0xc*/
                GT_U32    e;
                for(e = 0 ; e <= 127 ; e++) {
                    regsAddrPtr->TTI.TRILLAdjacency.adjacencyEntryReg0[e] =
                        0x6000+e*0xc;
                }/* end of loop e */
            }/*6000+e*0xc*/

        }/*end of unit TRILLAdjacency */


        {/*start of unit PTP */
            {/*3040+d*0x10+i*0x4*/
                GT_U32    i,d;
                for(i = 0 ; i <= 3 ; i++) {
                    for(d = 0 ; d <= 3 ; d++) {
                        regsAddrPtr->TTI.PTP._1588V1DomainDomainNumber[i][d] =
                            0x3040+d*0x10+i*0x4;
                    }/* end of loop d */
                }/* end of loop i */
            }/*3040+d*0x10+i*0x4*/
            {/*309c+0x4*d*/
                GT_U32    d;
                for(d = 0 ; d <= 4 ; d++) {
                    regsAddrPtr->TTI.PTP.PTPExceptionCheckingModeDomain[d] =
                        0x309c+0x4*d;
                }/* end of loop d */
            }/*309c+0x4*d*/
            if(!isSip5_20)
            {/*3010+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 7 ; n++) {
                    regsAddrPtr->TTI.PTP.piggyBackTSConfigs[n] =
                        0x3010+0x4*n;
                }/* end of loop n */
            }/*3010+0x4*n*/
            regsAddrPtr->TTI.PTP.PTPConfigs = 0x00003030;
            regsAddrPtr->TTI.PTP.PTPOverUDPDestinationPorts = 0x00003004;
            regsAddrPtr->TTI.PTP.PTPExceptionsAndCPUCodeConfig = 0x0000300c;
            regsAddrPtr->TTI.PTP.PTPEtherTypes = 0x00003000;
            regsAddrPtr->TTI.PTP.PTPExceptionsCntr = 0x00003034;
            regsAddrPtr->TTI.PTP._1588V2DomainDomainNumber = 0x00003090;
            regsAddrPtr->TTI.PTP.timestampEtherTypes = 0x00003094;
            regsAddrPtr->TTI.PTP.timestampConfigs = 0x00003098;

        }/*end of unit PTP */


        {/*start of unit pseudoWire */
            {/*1504+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 14 ; n++) {
                    regsAddrPtr->TTI.pseudoWire.MPLSOAMChannelTypeProfile[n] =
                        0x1504+0x4*n;
                }/* end of loop n */
            }/*1504+0x4*n*/
            regsAddrPtr->TTI.pseudoWire.PWE3Config = 0x00001500;
            regsAddrPtr->TTI.pseudoWire.PWTagModeExceptionConfig = 0x00001540;

        }/*end of unit pseudoWire */


        {/*start of unit protocolMatch */
            {/*90+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 5 ; n++) {
                    regsAddrPtr->TTI.protocolMatch.protocolsConfig[n] =
                        0x90+n*0x4;
                }/* end of loop n */
            }/*90+n*0x4*/
            regsAddrPtr->TTI.protocolMatch.protocolsEncapsulationConfig0 = 0x000000a8;
            regsAddrPtr->TTI.protocolMatch.protocolsEncapsulationConfig1 = 0x000000ac;

        }/*end of unit protocolMatch */

        if(!isSip5_20)
        {/*start of unit portMap */
            {/*10b0+4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    regsAddrPtr->TTI.portMap.portGroupMap[n] =
                        0x10b0+4*n;
                }/* end of loop n */
            }/*10b0+4*n*/
            {/*1000+n*4*/
                GT_U32    n;
                for(n = 0 ; n <= 42 ; n++) {
                    regsAddrPtr->TTI.portMap.portListMap[n] =
                        0x1000+n*4;
                }/* end of loop n */
            }/*1000+n*4*/

        }/*end of unit portMap */

        {/*start of unit phyPortAndEPortAttributes */
            {/*110004+p*4*/
                GT_U32    p;
                GT_U32    numPorts = (isSip6 ? 287 : 255);
                for(p = 0 ; p <= numPorts ; p++) {
                    regsAddrPtr->TTI.phyPortAndEPortAttributes.myPhysicalPortAttributes[p] =
                        0x110004+p*4;
                }/* end of loop p */
            }/*110004+p*4*/
            /* NOTE: in sip6 we will not duplicate addresses in the DB , we will
               use the generic engine that knows to give aggress in other pipe
               according to 'port group num' */
            if (!isSip6 && PRV_CPSS_IS_MULTI_PIPES_DEVICE_MAC(devNum))
            {
                {/*80110004+p*4*/
                    GT_U32    p;
                    GT_U32    numPorts = 255 ;
                    GT_U32    tti1Offset = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_TTI_1_E,NULL);
                    GT_U32    tti0Offset = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_TTI_E,NULL);
                    GT_U32    pipeOffset = tti1Offset-tti0Offset;
                    for(p = 0 ; p <= numPorts ; p++) {
                        regsAddrPtr->TTI.phyPortAndEPortAttributes.myPhysicalPortAttributes2[p] =
                            ((0x110004+p*4) + pipeOffset);
                    }/* end of loop p */
                }/*80110004+p*4*/
            }
            regsAddrPtr->TTI.phyPortAndEPortAttributes.remotePhysicalPortAssignmentGlobalConfig = 0x00110000;

            if (isSip6)
            {
                regsAddrPtr->TTI.phyPortAndEPortAttributes.remotePhysicalPortAssignmentGlobalConfigExt = 0x00110500;
            }
        }/*end of unit phyPortAndEPortAttributes */


        {/*start of unit MRUException */
            {/*170+n*4*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    regsAddrPtr->TTI.MRUException.MRUSize[n] =
                        0x170+n*4;
                }/* end of loop n */
            }/*170+n*4*/
            regsAddrPtr->TTI.MRUException.MRUCommandAndCode = 0x00000180;

        }/*end of unit MRUException */


        {/*start of unit MAC2ME */
            {/*160c+n*0x20*/
                GT_U32    n;
                for(n = 0 ; n <= 127 ; n++) {
                    regsAddrPtr->TTI.MAC2ME.MAC2MEHighMask[n] =
                        0x160c+n*0x20;
                }/* end of loop n */
            }/*160c+n*0x20*/
            {/*1600+n*0x20*/
                GT_U32    n;
                for(n = 0 ; n <= 127 ; n++) {
                    regsAddrPtr->TTI.MAC2ME.MAC2MELow[n] =
                        0x1600+n*0x20;
                }/* end of loop n */
            }/*1600+n*0x20*/
            {/*1604+n*0x20*/
                GT_U32    n;
                for(n = 0 ; n <= 127 ; n++) {
                    regsAddrPtr->TTI.MAC2ME.MAC2MEHigh[n] =
                        0x1604+n*0x20;
                }/* end of loop n */
            }/*1604+n*0x20*/
            {/*1608+n*0x20*/
                GT_U32    n;
                for(n = 0 ; n <= 127 ; n++) {
                    regsAddrPtr->TTI.MAC2ME.MAC2MELowMask[n] =
                        0x1608+n*0x20;
                }/* end of loop n */
            }/*1608+n*0x20*/
            {/*1610+0x20*n*/
                GT_U32    n;
                for(n = 0 ; n <= 127 ; n++) {
                    regsAddrPtr->TTI.MAC2ME.MAC2MESrcInterface[n] =
                        0x1610+0x20*n;
                }/* end of loop n */
            }/*1610+0x20*n*/
            {/*1614+0x20*n*/
                GT_U32    n;
                for(n = 0 ; n <= 127 ; n++) {
                    regsAddrPtr->TTI.MAC2ME.MAC2MESrcInterfaceSel[n] =
                        0x1614+0x20*n;
                }/* end of loop n */
            }/*1614+0x20*n*/

        }/*end of unit MAC2ME */


        {/*start of unit IPv6SolicitedNode */
            regsAddrPtr->TTI.IPv6SolicitedNode.IPv6SolicitedNodeMcAddrConfigReg0 = 0x00000040;
            regsAddrPtr->TTI.IPv6SolicitedNode.IPv6SolicitedNodeMcAddrConfigReg1 = 0x00000044;
            regsAddrPtr->TTI.IPv6SolicitedNode.IPv6SolicitedNodeMcAddrConfigReg2 = 0x00000048;
            regsAddrPtr->TTI.IPv6SolicitedNode.IPv6SolicitedNodeMcAddrConfigReg3 = 0x0000004c;
            regsAddrPtr->TTI.IPv6SolicitedNode.IPv6SolicitedNodeMcAddrMaskReg0 = 0x00000050;
            regsAddrPtr->TTI.IPv6SolicitedNode.IPv6SolicitedNodeMcAddrMaskReg1 = 0x00000054;
            regsAddrPtr->TTI.IPv6SolicitedNode.IPv6SolicitedNodeMcAddrMaskReg2 = 0x00000058;
            regsAddrPtr->TTI.IPv6SolicitedNode.IPv6SolicitedNodeMcAddrMaskReg3 = 0x0000005c;

        }/*end of unit IPv6SolicitedNode */


        {/*start of unit FCOE */
            regsAddrPtr->TTI.FCOE.FCoEGlobalConfig = 0x00000070;
            regsAddrPtr->TTI.FCOE.FCoEExceptionConfigs = 0x00000074;
            regsAddrPtr->TTI.FCOE.FCoEExceptionCntrs0 = 0x00000078;
            regsAddrPtr->TTI.FCOE.FCoEExceptionCntrs1 = 0x0000007c;
            regsAddrPtr->TTI.FCOE.FCoEExceptionCntrs2 = 0x00000080;

        }/*end of unit FCOE */


        {/*start of unit duplicationConfigs */
            regsAddrPtr->TTI.duplicationConfigs.duplicationConfigs0 = 0x00000150;
            regsAddrPtr->TTI.duplicationConfigs.duplicationConfigs1 = 0x00000154;
            regsAddrPtr->TTI.duplicationConfigs.MPLSMCDALow = 0x00000158;
            regsAddrPtr->TTI.duplicationConfigs.MPLSMCDAHigh = 0x0000015c;
            regsAddrPtr->TTI.duplicationConfigs.MPLSMCDALowMask = 0x00000160;
            regsAddrPtr->TTI.duplicationConfigs.MPLSMCDAHighMask = 0x00000164;

        }/*end of unit duplicationConfigs */


        {/*1a4+n*4*/
            GT_U32    n;
            for(n = 0 ; n <= 2 ; n++) {
                regsAddrPtr->TTI.TTIKRuleMatchCntr[n] =
                    0x1a4+n*4;
            }/* end of loop n */
        }/*1a4+n*4*/
        {/*100+4*i*/
            GT_U32    i;
            for(i = 0 ; i <= 3 ; i++) {
                regsAddrPtr->TTI.TTIUserDefinedEtherTypes[i] =
                    0x100+4*i;
            }/* end of loop i */
        }/*100+4*i*/
        regsAddrPtr->TTI.TTIIPv6GREEtherType = 0x00000034;
        regsAddrPtr->TTI.TTIIPv4GREEthertype = 0x00000018;
        regsAddrPtr->TTI.specialEtherTypes = 0x00000028;
        regsAddrPtr->TTI.sourceIDAssignment0 = 0x0000014c;
        regsAddrPtr->TTI.receivedFlowCtrlPktsCntr = 0x000001d0;
        regsAddrPtr->TTI.preRouteExceptionMasks = 0x0000002c;
        {/*f0+4*i*/
            GT_U32    i;
            for(i = 0 ; i <= 3 ; i++) {
                regsAddrPtr->TTI.PCLUserDefinedEtherTypes[i] =
                    0xf0+4*i;
            }/* end of loop i */
        }/*f0+4*i*/
        regsAddrPtr->TTI.MPLSEtherTypes = 0x00000030;
        regsAddrPtr->TTI.IPv6ExtensionValue = 0x00000038;
        regsAddrPtr->TTI.IPMinimumOffset = 0x00000020;
        regsAddrPtr->TTI.IPLengthChecks = 0x000001cc;
        regsAddrPtr->TTI.ECCTwoErrorsCntr = 0x000001dc;
        regsAddrPtr->TTI.ECCOneErrorCntr = 0x000001d8;
        regsAddrPtr->TTI.DSAConfig = 0x00000060;
        regsAddrPtr->TTI.droppedFlowCtrlPktsCntr = 0x000001d4;
        regsAddrPtr->TTI.CNAndFCConfig = 0x00000024;
        regsAddrPtr->TTI.CFMEtherType = 0x000001c8;
        regsAddrPtr->TTI.TTIMetalFix = 0x000013f8;

    }/*end of unit TTI */

    if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
    {
        regsAddrPtr->TTI.oamLabelGlobalConfiguration0 = 0x0000001c;
        regsAddrPtr->TTI.oamLabelGlobalConfiguration1 = 0x0000003c;
        regsAddrPtr->TTI.TTIEngine.TTILookupIPExceptionCodes1 = 0x00000114;
        regsAddrPtr->TTI.TTIEngine.TTIKeySegmentMode0 = 0x00000144;
        regsAddrPtr->TTI.TTIEngine.TTIKeySegmentMode1 = 0x00000148;
        regsAddrPtr->TTI.sourceIDAssignment0 = 0x0000014c;
        regsAddrPtr->TTI.sourceIDAssignment1 = 0x00000150;
        regsAddrPtr->TTI.duplicationConfigs.duplicationConfigs0 = 0x00000154;
        regsAddrPtr->TTI.duplicationConfigs.duplicationConfigs1 = 0x00000158;
        regsAddrPtr->TTI.duplicationConfigs.MPLSMCDALow = 0x0000015c;
        regsAddrPtr->TTI.duplicationConfigs.MPLSMCDAHigh = 0x00000160;
        regsAddrPtr->TTI.duplicationConfigs.MPLSMCDALowMask = 0x00000164;
        regsAddrPtr->TTI.duplicationConfigs.MPLSMCDAHighMask = 0x00000168;
        {/*190+n*4*/
            GT_U32    n;
            for(n = 0 ; n <= 11 ; n++) {
                regsAddrPtr->TTI.TTIKRuleMatchCntr[n] =
                    0x190+n*4;
            }/* end of loop n */
        }/*190+n*4*/

        DESTROY_SINGLE_REG_MAC(regsAddrPtr->TTI.CFMEtherType);
        DESTROY_SINGLE_REG_MAC(regsAddrPtr->TTI.ECCOneErrorCntr);
        DESTROY_SINGLE_REG_MAC(regsAddrPtr->TTI.ECCTwoErrorsCntr);
    }

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* cut through */
        regsAddrPtr->TTI.ttiCutThroughConfiguration          = 0x0000006C;
        regsAddrPtr->TTI.cutThroughByteCountExtractFailCntr  = 0x00000184;
        regsAddrPtr->TTI.cutThroughHdrIntegrityExceptionCfg1 = 0x00000188;
        regsAddrPtr->TTI.cutThroughHdrIntegrityExceptionCfg2 = 0x0000018C;
        /* ECN dropped configuration */
        regsAddrPtr->TTI.ECNDroppedConfiguration             = 0x0000016C;
    }
}

/**
* @internal l2iInit function
* @endinternal
*
* @brief   init the DB - L2I
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None
*/
static void l2iInit
(
    IN  GT_U8 devNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_U32  numPhyPorts = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.maxPhysicalPorts;


    {/*start of unit L2I */
        {/*start of unit VLANRangesConfig */
            regsAddrPtr->L2I.VLANRangesConfig.ingrVLANRangeConfig = 0x00000130;

        }/*end of unit VLANRangesConfig */


        {/*start of unit VLANMRUProfilesConfig */
            {/*140+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    regsAddrPtr->L2I.VLANMRUProfilesConfig.VLANMRUProfileConfig[n] =
                        0x140+n*0x4;
                }/* end of loop n */
            }/*140+n*0x4*/

        }/*end of unit VLANMRUProfilesConfig */


        {/*start of unit UDPBcMirrorTrapUDPRelayConfig */
            {/*1100+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 11 ; n++) {
                    regsAddrPtr->L2I.UDPBcMirrorTrapUDPRelayConfig.UDPBcDestinationPortConfigTable[n] =
                        0x1100+n*0x4;
                }/* end of loop n */
            }/*1100+n*0x4*/

        }/*end of unit UDPBcMirrorTrapUDPRelayConfig */


        {/*start of unit securityBreachStatus */
            regsAddrPtr->L2I.securityBreachStatus.securityBreachStatus0 = 0x00001400;
            regsAddrPtr->L2I.securityBreachStatus.securityBreachStatus1 = 0x00001404;
            regsAddrPtr->L2I.securityBreachStatus.securityBreachStatus2 = 0x00001408;
            regsAddrPtr->L2I.securityBreachStatus.securityBreachStatus3 = 0x0000140c;

        }/*end of unit securityBreachStatus */


        {/*start of unit MACBasedQoSTable */
            {/*1000+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 6 ; n++) {
                    regsAddrPtr->L2I.MACBasedQoSTable.MACQoSTableEntry[n] =
                        0x1000+n*0x4;
                }/* end of loop n */
            }/*1000+n*0x4*/

        }/*end of unit MACBasedQoSTable */


        {/*start of unit layer2BridgeMIBCntrs */
            {/*153c+n*0x20*/
                GT_U32    n;
                for(n = 0 ; n <= 1 ; n++) {
                    regsAddrPtr->L2I.layer2BridgeMIBCntrs.setVLANIngrFilteredPktCount[n] =
                        0x153c+n*0x20;
                }/* end of loop n */
            }/*153c+n*0x20*/
            {/*1530+0x20*n*/
                GT_U32    n;
                for(n = 0 ; n <= 1 ; n++) {
                    regsAddrPtr->L2I.layer2BridgeMIBCntrs.cntrsSetConfig0[n] =
                        0x1530+0x20*n;
                }/* end of loop n */
            }/*1530+0x20*n*/
            {/*1534+0x20*n*/
                GT_U32    n;
                for(n = 0 ; n <= 1 ; n++) {
                    regsAddrPtr->L2I.layer2BridgeMIBCntrs.cntrsSetConfig1[n] =
                        0x1534+0x20*n;
                }/* end of loop n */
            }/*1534+0x20*n*/
            {/*1538+n*0x20*/
                GT_U32    n;
                for(n = 0 ; n <= 1 ; n++) {
                    regsAddrPtr->L2I.layer2BridgeMIBCntrs.setIncomingPktCount[n] =
                        0x1538+n*0x20;
                }/* end of loop n */
            }/*1538+n*0x20*/
            {/*1540+n*0x20*/
                GT_U32    n;
                for(n = 0 ; n <= 1 ; n++) {
                    regsAddrPtr->L2I.layer2BridgeMIBCntrs.setSecurityFilteredPktCount[n] =
                        0x1540+n*0x20;
                }/* end of loop n */
            }/*1540+n*0x20*/
            {/*1544+n*0x20*/
                GT_U32    n;
                for(n = 0 ; n <= 1 ; n++) {
                    regsAddrPtr->L2I.layer2BridgeMIBCntrs.setBridgeFilteredPktCount[n] =
                        0x1544+n*0x20;
                }/* end of loop n */
            }/*1544+n*0x20*/
            regsAddrPtr->L2I.layer2BridgeMIBCntrs.MACAddrCount0 = 0x00001500;
            regsAddrPtr->L2I.layer2BridgeMIBCntrs.MACAddrCount1 = 0x00001504;
            regsAddrPtr->L2I.layer2BridgeMIBCntrs.MACAddrCount2 = 0x00001508;
            regsAddrPtr->L2I.layer2BridgeMIBCntrs.hostOutgoingMcPktCount = 0x00001520;
            regsAddrPtr->L2I.layer2BridgeMIBCntrs.hostOutgoingPktsCount = 0x00001514;
            regsAddrPtr->L2I.layer2BridgeMIBCntrs.hostIncomingPktsCount = 0x00001510;
            regsAddrPtr->L2I.layer2BridgeMIBCntrs.hostOutgoingBcPktCount = 0x00001524;
            regsAddrPtr->L2I.layer2BridgeMIBCntrs.matrixSourceDestinationPktCount = 0x00001528;

        }/*end of unit layer2BridgeMIBCntrs */


        {/*start of unit IPv6McBridgingBytesSelection */
            regsAddrPtr->L2I.IPv6McBridgingBytesSelection.IPv6MCBridgingBytesSelectionConfig = 0x00000120;

        }/*end of unit IPv6McBridgingBytesSelection */


        {/*start of unit ingrPortsRateLimitConfig */
            {/*12a0+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 7 ; n++) {
                    regsAddrPtr->L2I.ingrPortsRateLimitConfig.portSpeedOverrideEnable[n] =
                        0x12a0+n*0x4;
                }/* end of loop n */
            }/*12a0+n*0x4*/
            {/*1240+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 7 ; n++) {
                    regsAddrPtr->L2I.ingrPortsRateLimitConfig.MIISpeedGranularity[n] =
                        0x1240+n*0x4;
                }/* end of loop n */
            }/*1240+n*0x4*/
            {/*1260+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 7 ; n++) {
                    regsAddrPtr->L2I.ingrPortsRateLimitConfig.GMIISpeedGranularity[n] =
                        0x1260+n*0x4;
                }/* end of loop n */
            }/*1260+n*0x4*/
            {/*1280+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 7 ; n++) {
                    regsAddrPtr->L2I.ingrPortsRateLimitConfig.XGSpeedGranularity[n] =
                        0x1280+n*0x4;
                }/* end of loop n */
            }/*1280+n*0x4*/
            regsAddrPtr->L2I.ingrPortsRateLimitConfig.ingrRateLimitConfig0 = 0x00001200;
            regsAddrPtr->L2I.ingrPortsRateLimitConfig.ingrRateLimitConfig3 = 0x0000120c;
            regsAddrPtr->L2I.ingrPortsRateLimitConfig.ingrRateLimitConfig2 = 0x00001208;
            regsAddrPtr->L2I.ingrPortsRateLimitConfig.ingrRateLimitConfig1 = 0x00001204;
            regsAddrPtr->L2I.ingrPortsRateLimitConfig.ingrRateLimitDropCntr310 = 0x00001220;
            regsAddrPtr->L2I.ingrPortsRateLimitConfig.ingrRateLimitDropCntr3932 = 0x00001224;

        }/*end of unit ingrPortsRateLimitConfig */


        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            regsAddrPtr->L2I.globalEportConifguration.globalEportMinValue  = 0x00000160;
            regsAddrPtr->L2I.globalEportConifguration.globalEportMaxValue  = 0x00000164;
        }
        else
        {/*start of unit globalEportConifguration */
            regsAddrPtr->L2I.globalEportConifguration.globalEPortValue = 0x00000160;
            regsAddrPtr->L2I.globalEportConifguration.globalEPortMask = 0x00000164;

        }/*end of unit globalEportConifguration */


        {/*start of unit ctrlTrafficToCPUConfig */
            {/*c00+n*0x8 +t*0x4*/
                GT_U32    n,t;
                for(n = 0 ; n <= 7 ; n++) {
                    for(t = 0 ; t <= 1 ; t++) {
                        regsAddrPtr->L2I.ctrlTrafficToCPUConfig.IPv6McLinkLocalCPUCodeIndex[n][t] =
                            0xc00+n*0x8 +t*0x4;
                    }/* end of loop t */
                }/* end of loop n */
            }/*c00+n*0x8 +t*0x4*/
            {/*d00+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 7 ; n++) {
                    regsAddrPtr->L2I.ctrlTrafficToCPUConfig.IPv6McLinkLocalReged[n] =
                        0xd00+n*0x4;
                }/* end of loop n */
            }/*d00+n*0x4*/
            {/*b00+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 7 ; n++) {
                    regsAddrPtr->L2I.ctrlTrafficToCPUConfig.IPv6McLinkLocalConfig[n] =
                        0xb00+n*0x4;
                }/* end of loop n */
            }/*b00+n*0x4*/
            {/*e04+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 1 ; n++) {
                    regsAddrPtr->L2I.ctrlTrafficToCPUConfig.IPv6ICMPMsgTypeConfig[n] =
                        0xe04+n*0x4;
                }/* end of loop n */
            }/*e04+n*0x4*/
            regsAddrPtr->L2I.ctrlTrafficToCPUConfig.IPv6ICMPCommand = 0x00000e00;
            {/*a00+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 7 ; n++) {
                    regsAddrPtr->L2I.ctrlTrafficToCPUConfig.IPv4McLinkLocalReged[n] =
                        0xa00+n*0x4;
                }/* end of loop n */
            }/*a00+n*0x4*/
            {/*900+n*0x8+ t*0x4*/
                GT_U32    n,t;
                for(n = 0 ; n <= 7 ; n++) {
                    for(t = 0 ; t <= 1 ; t++) {
                        regsAddrPtr->L2I.ctrlTrafficToCPUConfig.IPv4McLinkLocalCPUCodeIndex[n][t] =
                            0x900+n*0x8+ t*0x4;
                    }/* end of loop t */
                }/* end of loop n */
            }/*900+n*0x8+ t*0x4*/
            {/*800+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 7 ; n++) {
                    regsAddrPtr->L2I.ctrlTrafficToCPUConfig.IPv4McLinkLocalConfig[n] =
                        0x800+n*0x4;
                }/* end of loop n */
            }/*800+n*0x4*/
            {/*700+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 7 ; n++) {
                    regsAddrPtr->L2I.ctrlTrafficToCPUConfig.IEEEReservedMcReged[n] =
                        0x700+n*0x4;
                }/* end of loop n */
            }/*700+n*0x4*/
            {/*600+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 15 ; n++) {
                    regsAddrPtr->L2I.ctrlTrafficToCPUConfig.IEEEReservedMcCPUIndex[n] =
                        0x600+n*0x4;
                }/* end of loop n */
            }/*600+n*0x4*/
            {/*580+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 15 ; n++) {
                    regsAddrPtr->L2I.ctrlTrafficToCPUConfig.IEEEReservedMcConfig7[n] =
                        0x580+n*0x4;
                }/* end of loop n */
            }/*580+n*0x4*/
            {/*500+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 15 ; n++) {
                    regsAddrPtr->L2I.ctrlTrafficToCPUConfig.IEEEReservedMcConfig6[n] =
                        0x500+n*0x4;
                }/* end of loop n */
            }/*500+n*0x4*/
            {/*480+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 15 ; n++) {
                    regsAddrPtr->L2I.ctrlTrafficToCPUConfig.IEEEReservedMcConfig5[n] =
                        0x480+n*0x4;
                }/* end of loop n */
            }/*480+n*0x4*/
            {/*400+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 15 ; n++) {
                    regsAddrPtr->L2I.ctrlTrafficToCPUConfig.IEEEReservedMcConfig4[n] =
                        0x400+n*0x4;
                }/* end of loop n */
            }/*400+n*0x4*/
            {/*380+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 15 ; n++) {
                    regsAddrPtr->L2I.ctrlTrafficToCPUConfig.IEEEReservedMcConfig3[n] =
                        0x380+n*0x4;
                }/* end of loop n */
            }/*380+n*0x4*/
            {/*300+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 15 ; n++) {
                    regsAddrPtr->L2I.ctrlTrafficToCPUConfig.IEEEReservedMcConfig2[n] =
                        0x300+n*0x4;
                }/* end of loop n */
            }/*300+n*0x4*/
            {/*280+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 15 ; n++) {
                    regsAddrPtr->L2I.ctrlTrafficToCPUConfig.IEEEReservedMcConfig1[n] =
                        0x280+n*0x4;
                }/* end of loop n */
            }/*280+n*0x4*/
            {/*200+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 15 ; n++) {
                    regsAddrPtr->L2I.ctrlTrafficToCPUConfig.IEEEReservedMcConfig0[n] =
                        0x200+n*0x4;
                }/* end of loop n */
            }/*200+n*0x4*/

        }/*end of unit ctrlTrafficToCPUConfig */


        {/*start of unit bridgeEngineInterrupts */
            regsAddrPtr->L2I.bridgeEngineInterrupts.bridgeInterruptCause = 0x00002100;
            regsAddrPtr->L2I.bridgeEngineInterrupts.bridgeInterruptMask = 0x00002104;

        }/*end of unit bridgeEngineInterrupts */


        {/*start of unit bridgeEngineConfig */
            regsAddrPtr->L2I.bridgeEngineConfig.bridgeGlobalConfig0 = 0x00000000;
            regsAddrPtr->L2I.bridgeEngineConfig.bridgeGlobalConfig1 = 0x00000004;
            regsAddrPtr->L2I.bridgeEngineConfig.bridgeCommandConfig0 = 0x00000020;
            regsAddrPtr->L2I.bridgeEngineConfig.bridgeCommandConfig1 = 0x00000024;
            regsAddrPtr->L2I.bridgeEngineConfig.bridgeCommandConfig2 = 0x00000028;
            regsAddrPtr->L2I.bridgeEngineConfig.bridgeCommandConfig3 = 0x0000002c;
            regsAddrPtr->L2I.bridgeEngineConfig.bridgeSecurityBreachDropCntrCfg0 = 0x00000040;
            regsAddrPtr->L2I.bridgeEngineConfig.bridgeSecurityBreachDropCntrCfg1 = 0x00000044;
            regsAddrPtr->L2I.bridgeEngineConfig.bridgecpuPortConfig = 0x00000048;

        }/*end of unit bridgeEngineConfig */


        {/*start of unit bridgeDropCntrAndSecurityBreachDropCntrs */
            regsAddrPtr->L2I.bridgeDropCntrAndSecurityBreachDropCntrs.globalSecurityBreachFilterCntr = 0x00001300;
            regsAddrPtr->L2I.bridgeDropCntrAndSecurityBreachDropCntrs.portVLANSecurityBreachDropCntr = 0x00001304;
            regsAddrPtr->L2I.bridgeDropCntrAndSecurityBreachDropCntrs.bridgeFilterCntr = 0x00001308;

        }/*end of unit bridgeDropCntrAndSecurityBreachDropCntrs */


        {/*start of unit bridgeAccessMatrix */
            {/*180+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 7 ; n++) {
                    regsAddrPtr->L2I.bridgeAccessMatrix.bridgeAccessMatrixLine[n] =
                        0x180+n*0x4;
                }/* end of loop n */
            }/*180+n*0x4*/
            regsAddrPtr->L2I.bridgeAccessMatrix.bridgeAccessMatrixDefault = 0x00000170;

        }/*end of unit bridgeAccessMatrix */

        {/*start of unit eVlanMirroringToAnalyser */
            {/*1600+n*0x4*/
                GT_U32    n;

                for(n = 0 ; n < numPhyPorts/32 ; n++) {
                    regsAddrPtr->L2I.eVlanMirroringToAnalyser.eVlanMirroringToAnalyserEnable[n] =
                        0x1600+n*0x4;
                }/* end of loop n */
            }/*1600+n*0x4*/

        }/*end of unit eVlanMirroringToAnalyser */

        if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
        {
            {/*12a0+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 7 ; n++) {
                    regsAddrPtr->L2I.ingrPortsRateLimitConfig.CGSpeedGranularity[n] =
                        0x12a0+n*0x4;
                }/* end of loop n */
            }/*12a0+n*0x4*/
            {/*12c0+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 7 ; n++) {
                    regsAddrPtr->L2I.ingrPortsRateLimitConfig.portSpeedOverrideEnable[n] =
                        0x12c0+n*0x4;
                }/* end of loop n */
            }/*12c0+n*0x4*/

            regsAddrPtr->L2I.l2iBadAddressLatchReg = 0x00002120;

            {/*800+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    regsAddrPtr->L2I.ctrlTrafficToCPUConfig.IPv4McLinkLocalConfig[n] =
                        0x800+n*0x4;
                }/* end of loop n */
            }/*800+n*0x4*/
            {/*b00+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    regsAddrPtr->L2I.ctrlTrafficToCPUConfig.IPv6McLinkLocalConfig[n] =
                        0xb00+n*0x4;
                }/* end of loop n */
            }/*b00+n*0x4*/
        }
    }/*end of unit L2I */
}

/**
* @internal emInit function
* @endinternal
*
* @brief   init the DB - EM
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2;
*         DxCh3; xCat; xCat3; Lion; xCat2; Lion2 Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None
*/
static void emInit
(
    IN  GT_U8 devNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    {/* EM */
        {/*start of EMGlobalConfiguration*/
            regsAddrPtr->EM.EMGlobalConfiguration.EMGlobalConfiguration1 = 0x0004;
            regsAddrPtr->EM.EMGlobalConfiguration.EMGlobalConfiguration2 = 0x0008;
            regsAddrPtr->EM.EMGlobalConfiguration.EMCRCHashInitConfiguration = 0x000c;
        }/*end of EMGlobalConfiguration*/


        {/*start of EMIndirectAccess*/
            regsAddrPtr->EM.EMIndirectAccess.EMIndirectAccessControl = 0x0130;
            {/* 0x134 + n*4 */
                GT_U32    n;
                for (n = 0 ; n <= 4 ; n++) {
                    regsAddrPtr->EM.EMIndirectAccess.EMIndirectAccessData[n] =
                        0x134+4*n;
                }/* end of loop n */
            }/* 0x134 + n*4 */
            regsAddrPtr->EM.EMIndirectAccess.EMIndirectAccessAccess = 0x0150;
        }/*end of EMIndirectAccess*/


        {/*start of EMInterrupt*/
            regsAddrPtr->EM.EMInterrupt.EMInterruptCauseReg = 0x01b0;
            regsAddrPtr->EM.EMInterrupt.EMInterruptMaskReg = 0x01b4;
        }/*end of EMInterrupt*/


        {/*start of EMHashResults*/
            {/* 0x300 + n*4 */
                GT_U32    n;
                for (n = 0 ; n <= 7 ; n++) {
                    regsAddrPtr->EM.EMhashResults.EMMultiHashCRCResultReg[n] =
                        0x300+n*4 ;
                }/* end of loop n */
            }/* 0x300 + n*4 */
        }/*end of EMHashResults*/


        {/*start of EMSchedulerConfiguration*/
            regsAddrPtr->EM.EMSchedulerConfiguration.EMSchedulerSWRRControl = 0x0380;
            regsAddrPtr->EM.EMSchedulerConfiguration.EMSchedulerSWRRConfig = 0x0384;
        }/*end of EMSchedulerConfiguration*/

    }/*end of unit EM */
}


/**
* @internal fdbInit function
* @endinternal
*
* @brief   init the DB - FDB
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None
*/
static void fdbInit
(
    IN  GT_U8 devNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_U32  isSip6    = PRV_CPSS_SIP_6_CHECK_MAC(devNum);

    {/*start of unit FDB[16] */
        {/*start of unit FDBCore */
            {/*start of unit sourceAddrNotLearntCntr */
                regsAddrPtr->FDB.FDBCore.sourceAddrNotLearntCntr.learnedEntryDiscardsCount = 0x00000060;

            }/*end of unit sourceAddrNotLearntCntr */


            {/*start of unit FDB */
                regsAddrPtr->FDB.FDBCore.FDB.MTCPipeDispatcher = 0x00000400;

            }/*end of unit FDB */


            {/*start of unit FDBInterrupt */
                regsAddrPtr->FDB.FDBCore.FDBInterrupt.FDBInterruptCauseReg = 0x000001b0;
                regsAddrPtr->FDB.FDBCore.FDBInterrupt.FDBInterruptMaskReg = 0x000001b4;

            }/*end of unit FDBInterrupt */


            {/*start of unit FDBInternal */
                regsAddrPtr->FDB.FDBCore.FDBInternal.metalFix = 0x00000170;
                regsAddrPtr->FDB.FDBCore.FDBInternal.FDBIdleState = 0x00000174;

            }/*end of unit FDBInternal */


            {/*start of unit FDBIndirectAccess */
                {/*134+n*4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 4 ; n++) {
                        regsAddrPtr->FDB.FDBCore.FDBIndirectAccess.FDBIndirectAccessData[n] =
                            0x134+n*4;
                    }/* end of loop n */
                }/*134+n*4*/
                regsAddrPtr->FDB.FDBCore.FDBIndirectAccess.FDBIndirectAccessCtrl = 0x00000130;
                regsAddrPtr->FDB.FDBCore.FDBIndirectAccess.FDBIndirectAccessAccess = 0x00000150;

            }/*end of unit FDBIndirectAccess */


            {/*start of unit FDBHashResults */
                {/*300+0x4*n*/
                    GT_U32    n;
                    for(n = 0 ; n <= 7 ; n++) {
                        regsAddrPtr->FDB.FDBCore.FDBHashResults.FDBMultiHashCRCResultReg[n] =
                            0x300+0x4*n;
                    }/* end of loop n */
                }/*300+0x4*n*/
                regsAddrPtr->FDB.FDBCore.FDBHashResults.FDBNonMulti_hash_crc_result = 0x00000320;
                regsAddrPtr->FDB.FDBCore.FDBHashResults.FDBXorHash = 0x00000324;

            }/*end of unit FDBHashResults */


            {/*start of unit FDBGlobalConfig */
                regsAddrPtr->FDB.FDBCore.FDBGlobalConfig.FDBGlobalConfig = 0x00000000;
                regsAddrPtr->FDB.FDBCore.FDBGlobalConfig.FDBGlobalConfig1 = 0x00000004;
                regsAddrPtr->FDB.FDBCore.FDBGlobalConfig.FDBCRCHashInitConfig = 0x0000000c;
                regsAddrPtr->FDB.FDBCore.FDBGlobalConfig.bridgeAccessLevelConfig = 0x00000010;
                regsAddrPtr->FDB.FDBCore.FDBGlobalConfig.FDBAgingWindowSize = 0x00000014;

                if(isSip6)
                {
                    regsAddrPtr->FDB.FDBCore.FDBGlobalConfig.FDBGlobalConfig2 = 0x00000008;
                }

            }/*end of unit FDBGlobalConfig */


            {/*start of unit FDBCntrsUpdate */
                regsAddrPtr->FDB.FDBCore.FDBCntrsUpdate.FDBCntrsUpdateCtrl = 0x00000340;

            }/*end of unit FDBCntrsUpdate */


            {/*start of unit FDBBankCntrs */
                {/*200+n * 0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 15 ; n++) {
                        regsAddrPtr->FDB.FDBCore.FDBBankCntrs.FDBBankCntr[n] =
                            0x200+n * 0x4;
                    }/* end of loop n */
                }/*200+n * 0x4*/
                regsAddrPtr->FDB.FDBCore.FDBBankCntrs.FDBBankRank0 = 0x00000240;
                regsAddrPtr->FDB.FDBCore.FDBBankCntrs.FDBBankRank1 = 0x00000244;
                regsAddrPtr->FDB.FDBCore.FDBBankCntrs.FDBBankRank2 = 0x00000248;
                regsAddrPtr->FDB.FDBCore.FDBBankCntrs.FDBBankRank3 = 0x0000024c;
                regsAddrPtr->FDB.FDBCore.FDBBankCntrs.FDBBankRank4 = 0x00000250;
                regsAddrPtr->FDB.FDBCore.FDBBankCntrs.FDBBankRank5 = 0x00000254;
                regsAddrPtr->FDB.FDBCore.FDBBankCntrs.FDBBankRank6 = 0x00000258;
                regsAddrPtr->FDB.FDBCore.FDBBankCntrs.FDBBankRank7 = 0x0000025c;
                regsAddrPtr->FDB.FDBCore.FDBBankCntrs.FDBBankRank8 = 0x00000260;
                regsAddrPtr->FDB.FDBCore.FDBBankCntrs.FDBBankRank9 = 0x00000264;
                regsAddrPtr->FDB.FDBCore.FDBBankCntrs.FDBBankRank13 = 0x00000274;
                regsAddrPtr->FDB.FDBCore.FDBBankCntrs.FDBBankRank12 = 0x00000270;
                regsAddrPtr->FDB.FDBCore.FDBBankCntrs.FDBBankRank11 = 0x0000026c;
                regsAddrPtr->FDB.FDBCore.FDBBankCntrs.FDBBankRank10 = 0x00000268;
                regsAddrPtr->FDB.FDBCore.FDBBankCntrs.FDBBankRank14 = 0x00000278;
                regsAddrPtr->FDB.FDBCore.FDBBankCntrs.FDBBankRank15 = 0x0000027c;
                regsAddrPtr->FDB.FDBCore.FDBBankCntrs.FDBBankCntrsCtrl = 0x00000280;

            }/*end of unit FDBBankCntrs */


            {/*start of unit FDBAddrUpdateMsgs */
                {/*start of unit AUMsgToCPU */
                    regsAddrPtr->FDB.FDBCore.FDBAddrUpdateMsgs.AUMsgToCPU.AUMsgToCPU = 0x00000090;
                    regsAddrPtr->FDB.FDBCore.FDBAddrUpdateMsgs.AUMsgToCPU.AUFIFOToCPUConfig = 0x00000094;

                }/*end of unit AUMsgToCPU */


                {/*start of unit AUMsgFromCPU */
                    {/*d0+n*4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 5 ; n++) {
                            regsAddrPtr->FDB.FDBCore.FDBAddrUpdateMsgs.AUMsgFromCPU.AUMsgFromCPUWord[n] =
                                0xd0+n*4;
                        }/* end of loop n */
                    }/*d0+n*4*/
                    regsAddrPtr->FDB.FDBCore.FDBAddrUpdateMsgs.AUMsgFromCPU.AUMsgFromCPUCtrl = 0x000000c0;

                }/*end of unit AUMsgFromCPU */


            }/*end of unit FDBAddrUpdateMsgs */


            {/*start of unit FDBAction */
                regsAddrPtr->FDB.FDBCore.FDBAction.FDBActionGeneral = 0x00000020;
                regsAddrPtr->FDB.FDBCore.FDBAction.FDBAction0 = 0x00000024;
                regsAddrPtr->FDB.FDBCore.FDBAction.FDBAction1 = 0x00000028;
                regsAddrPtr->FDB.FDBCore.FDBAction.FDBAction2 = 0x0000002c;
                regsAddrPtr->FDB.FDBCore.FDBAction.FDBAction3 = 0x00000030;
                regsAddrPtr->FDB.FDBCore.FDBAction.FDBAction4 = 0x00000034;
                regsAddrPtr->FDB.FDBCore.FDBAction.FDBActionOldParameters = 0x00000038;
                regsAddrPtr->FDB.FDBCore.FDBAction.FDBActionNewParameters = 0x0000003c;

            }/*end of unit FDBAction */

            { /*start of unit AUMsgRateLimitingCounter*/
                regsAddrPtr->FDB.FDBCore.AUMsgRateLimitingCounter.AUMsgRateLimCnt = 0x00000070;
            } /*end of unit AUMsgRateLimitingCounter*/

            { /*start of unit FDBScedulerConfiguration*/
                regsAddrPtr->FDB.FDBCore.FDBScedulerConfiguration.FDBScedulerSWRRControl = 0x00000380;
                regsAddrPtr->FDB.FDBCore.FDBScedulerConfiguration.FDBScedulerSWRRConfig  = 0x00000384;
            } /*end of unit FDBScedulerConfiguration*/

        }/*end of unit FDBCore */


    }/*end of unit FDB */
}

/**
* @internal eqInit function
* @endinternal
*
* @brief   init the DB - EQ
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None
*/
static void eqInit
(
    IN  GT_U8 devNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_U32  isSip5_20 = PRV_CPSS_SIP_5_20_CHECK_MAC(devNum);
    GT_U32  isSip6    = PRV_CPSS_SIP_6_CHECK_MAC(devNum);
    {/*start of unit EQ */
        {/*start of unit toCpuPktRateLimiters */
            regsAddrPtr->EQ.toCpuPktRateLimiters.CPUCodeRateLimiterDropCntr = 0x00000068;
            {/*80000+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 254 ; n++) {
                    regsAddrPtr->EQ.toCpuPktRateLimiters.toCpuRateLimiterPktCntr[n] =
                        0x80000+n*0x4;
                }/* end of loop n */
            }/*80000+n*0x4*/

        }/*end of unit toCpuPktRateLimiters */


        {/*start of unit SCTRateLimitersInterrupt */
            {/*10020+(n-1)*0x10*/
                GT_U32    n;
                GT_U32    nMax = isSip6 ? 16 : isSip5_20 ? 32 : 16;
                for(n = 1 ; n <= nMax ; n++) {
                    regsAddrPtr->EQ.SCTRateLimitersInterrupt.CPUCodeRateLimitersInterruptCause[n-1] =
                        0x10020+(n-1)*0x10;
                }/* end of loop n */
            }/*10020+(n-1)*0x10*/
            {/*10024+(n-1)*0x10*/
                GT_U32    n;
                GT_U32    nMax = isSip6 ? 16 : isSip5_20 ? 32 : 16;
                for(n = 1 ; n <= nMax ; n++) {
                    regsAddrPtr->EQ.SCTRateLimitersInterrupt.CPUCodeRateLimitersInterruptMask[n-1] =
                        0x10024+(n-1)*0x10;
                }/* end of loop n */
            }/*10024+(n-1)*0x10*/
            regsAddrPtr->EQ.SCTRateLimitersInterrupt.CPUCodeRateLimitersInterruptSummary = 0x00010000;
            regsAddrPtr->EQ.SCTRateLimitersInterrupt.CPUCodeRateLimitersInterruptSummaryMask = 0x00010004;

        }/*end of unit SCTRateLimitersInterrupt */


        {/*start of unit preEgrInterrupt */
            regsAddrPtr->EQ.preEgrInterrupt.preEgrInterruptSummary = 0x00000058;
            regsAddrPtr->EQ.preEgrInterrupt.preEgrInterruptSummaryMask = 0x0000005c;

        }/*end of unit preEgrInterrupt */


        {/*start of unit preEgrEngineGlobalConfig */
            {/*a100+r*0x4*/
                GT_U32    r;
                GT_U32    rMax = (isSip6    ? 31 :
                                  isSip5_20 ? 15 : 7);
                for(r = 0 ; r <= rMax ; r++) {
                    regsAddrPtr->EQ.preEgrEngineGlobalConfig.duplicationOfPktsToCPUConfig[r] =
                        0xa100+r*0x4;
                }/* end of loop r */
            }/*a100+r*0x4*/
            {/*a028+r*0x4*/
                GT_U32    r;
                for(r = 0 ; r <= 3 ; r++) {
                    regsAddrPtr->EQ.preEgrEngineGlobalConfig.MTUTableEntry[r] =
                        0xa028+r*0x4;
                }/* end of loop r */
            }/*a028+r*0x4*/
            regsAddrPtr->EQ.preEgrEngineGlobalConfig.preEgrEngineGlobalConfig = 0x0000a000;
            regsAddrPtr->EQ.preEgrEngineGlobalConfig.MTUGlobalConfig = 0x0000a020;
            regsAddrPtr->EQ.preEgrEngineGlobalConfig.lossyDropConfig = 0x0000a008;
            regsAddrPtr->EQ.preEgrEngineGlobalConfig.cscdTrunkHashBitSelectionConfig = 0x0000a010;
            regsAddrPtr->EQ.preEgrEngineGlobalConfig.targetEPortMTUExceededCntr = 0x0000a024;

            regsAddrPtr->EQ.preEgrEngineGlobalConfig.incomingWrrArbiterWeights = 0x0000af00;
            regsAddrPtr->EQ.preEgrEngineGlobalConfig.dupFifoWrrArbiterWeights = 0x0000af08;

        }/*end of unit preEgrEngineGlobalConfig */


        {/*start of unit mirrToAnalyzerPortConfigs */
            {/*b200+i*4*/
                GT_U32    i;
                for(i = 0 ; i <= 6 ; i++) {
                    regsAddrPtr->EQ.mirrToAnalyzerPortConfigs.mirrorInterfaceParameterReg[i] =
                        0xb200+i*4;
                }/* end of loop i */
            }/*b200+i*4*/
            regsAddrPtr->EQ.mirrToAnalyzerPortConfigs.analyzerPortGlobalConfig = 0x0000b000;
            if(isSip6)
            {
                {/*b04c+i*4*/
                    GT_U32    i;
                    for(i = 0 ; i <= 6 ; i++) {
                        regsAddrPtr->EQ.mirrToAnalyzerPortConfigs.ingrStatisticMirrToAnalyzerPortRatioConfig[i] =
                            0xb04c+i*4;
                    }/* end of loop i */
                }/*b04c+i*4*/
                {/*b00c+i*4*/
                    GT_U32    i;
                    for(i = 0 ; i <= 6 ; i++) {
                        regsAddrPtr->EQ.mirrToAnalyzerPortConfigs.ingrStatisticMirrToAnalyzerPortEnableConfig[i] =
                            0xb00c+i*4;
                    }/* end of loop i */
                }/*b00c+i*4*/
            }
            else
            {
                regsAddrPtr->EQ.mirrToAnalyzerPortConfigs.ingrStatisticMirrToAnalyzerPortConfig = 0x0000b004;
            }
            regsAddrPtr->EQ.mirrToAnalyzerPortConfigs.ingrAndEgrMonitoringToAnalyzerQoSConfig = 0x0000b008;

        }/*end of unit mirrToAnalyzerPortConfigs */


        {/*start of unit L2ECMP */
            if(isSip6)
            {
                regsAddrPtr->EQ.L2ECMP.ePortECMPEPortMinimum = 0x0000502C;
                regsAddrPtr->EQ.L2ECMP.ePortECMPEPortMaximum = 0x00005030;
                regsAddrPtr->EQ.L2ECMP.ePortECMPHashBitSelectionConfig = 0x00005028;
            }
            else
            {
                regsAddrPtr->EQ.L2ECMP.ePortECMPEPortValue = 0x00005010;
                regsAddrPtr->EQ.L2ECMP.ePortECMPEPortMask = 0x00005014;
                regsAddrPtr->EQ.L2ECMP.ePortECMPHashBitSelectionConfig = 0x00005030;
            }
            regsAddrPtr->EQ.L2ECMP.ePortECMPLTTIndexBaseEPort = 0x00005018;
            regsAddrPtr->EQ.L2ECMP.ePortECMPLFSRConfig = 0x00005020;
            regsAddrPtr->EQ.L2ECMP.ePortECMPLFSRSeed = 0x00005024;
            regsAddrPtr->EQ.L2ECMP.trunkLFSRConfig = 0x0000d000;
            regsAddrPtr->EQ.L2ECMP.trunkLFSRSeed = 0x0000d004;
            regsAddrPtr->EQ.L2ECMP.trunkHashBitSelectionConfig = 0x0000d010;

        }/*end of unit L2ECMP */

        if(!isSip6)
        {/*start of unit ingrSTCInterrupt */
            {/*620+(n-1)*16*/
                GT_U32    n;
                GT_U32    nMax = isSip5_20 ? 32 : 16;
                for(n = 1 ; n <= nMax ; n++) {
                    regsAddrPtr->EQ.ingrSTCInterrupt.ingrSTCInterruptCause[n-1] =
                        0x620+(n-1)*16;
                }/* end of loop n */
            }/*620+(n-1)*16*/
            {/*624+(n-1)*16*/
                GT_U32    n;
                GT_U32    nMax = isSip5_20 ? 32 : 16;
                for(n = 1 ; n <= nMax ; n++) {
                    regsAddrPtr->EQ.ingrSTCInterrupt.ingrSTCInterruptMask[n-1] =
                        0x624+(n-1)*16;
                }/* end of loop n */
            }/*624+(n-1)*16*/
            regsAddrPtr->EQ.ingrSTCInterrupt.ingrSTCInterruptSummary = 0x00000600;
            regsAddrPtr->EQ.ingrSTCInterrupt.ingrSTCInterruptSummaryMask = 0x00000604;

            if(isSip5_20)
            {
                regsAddrPtr->EQ.ingrSTCInterrupt.ingrSTCInterruptSummary1 = 0x00000608;
                regsAddrPtr->EQ.ingrSTCInterrupt.ingrSTCInterruptSummary1Mask = 0x0000060c;
            }
        }/*end of unit ingrSTCInterrupt */


        if(!isSip6)
        {/*start of unit ingrSTCConfig */
            regsAddrPtr->EQ.ingrSTCConfig.ingrSTCConfig = 0x0000001c;

        }/*end of unit ingrSTCConfig */


        {/*start of unit ingrForwardingRestrictions */
            regsAddrPtr->EQ.ingrForwardingRestrictions.ingrForwardingRestrictionsDroppedPktsCntr = 0x0002000c;
            regsAddrPtr->EQ.ingrForwardingRestrictions.ingrForwardingRestrictionsDropCode = 0x00020010;

        }/*end of unit ingrForwardingRestrictions */


        {/*start of unit ingrDropCntr */
            regsAddrPtr->EQ.ingrDropCntr.ingrDropCntrConfig = 0x0000003c;
            regsAddrPtr->EQ.ingrDropCntr.ingrDropCntr = 0x00000040;

        }/*end of unit ingrDropCntr */


        {/*start of unit ingrDropCodesCntr */
            regsAddrPtr->EQ.ingrDropCodesCntr.dropCodesCntrConfig = 0x0000006c;
            regsAddrPtr->EQ.ingrDropCodesCntr.dropCodesCntr = 0x00000070;

        }/*end of unit ingrDropCodesCntr */

        if(isSip6)
        {/*start of unit ingrDropCodeLatching */
            regsAddrPtr->EQ.ingrDropCodeLatching.ingrDropCodeLatching = 0x000000b0;
            regsAddrPtr->EQ.ingrDropCodeLatching.lastDropCode = 0x000000b4;

        }/*end of unit ingrDropCodeLatching */

        {/*start of unit ePort */
            regsAddrPtr->EQ.ePort.ePortGlobalConfig = 0x00005000;
            if(!isSip6)
            {
                regsAddrPtr->EQ.ePort.protectionLOCWriteMask = 0x00005050;
                regsAddrPtr->EQ.ePort.protectionSwitchingRXExceptionConfig = 0x00005054;
            }
            else
            {
                regsAddrPtr->EQ.ePort.protectionLOCWriteMask = 0x00005054;
                regsAddrPtr->EQ.ePort.protectionSwitchingRXExceptionConfig = 0x00005058;
            }

        }/*end of unit ePort */


        {/*start of unit criticalECCCntrs */
            regsAddrPtr->EQ.criticalECCCntrs.criticalECCConfig = 0x00000074;
            regsAddrPtr->EQ.criticalECCCntrs.BMClearCriticalECCErrorCntr = 0x00000078;
            regsAddrPtr->EQ.criticalECCCntrs.BMClearCriticalECCErrorParams = 0x0000007c;
            regsAddrPtr->EQ.criticalECCCntrs.BMIncCriticalECCErrorCntr = 0x00000088;
            regsAddrPtr->EQ.criticalECCCntrs.BMIncCriticalECCErrorParams = 0x0000008c;
            regsAddrPtr->EQ.criticalECCCntrs.BMCriticalECCInterruptCause = 0x000000a0;
            regsAddrPtr->EQ.criticalECCCntrs.BMCriticalECCInterruptMask = 0x000000a4;

        }/*end of unit criticalECCCntrs */


        {/*start of unit CPUTargetDeviceConfig */
            regsAddrPtr->EQ.CPUTargetDeviceConfig.CPUTargetDeviceConfigReg0 = 0x00000110;
            regsAddrPtr->EQ.CPUTargetDeviceConfig.CPUTargetDeviceConfigReg1 = 0x00000114;
            regsAddrPtr->EQ.CPUTargetDeviceConfig.CPUTargetDeviceConfigReg2 = 0x00000118;
            regsAddrPtr->EQ.CPUTargetDeviceConfig.CPUTargetDeviceConfigReg3 = 0x0000011c;

        }/*end of unit CPUTargetDeviceConfig */


        {/*start of unit applicationSpecificCPUCodes */
            {/*7c00+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    regsAddrPtr->EQ.applicationSpecificCPUCodes.IPProtocolCPUCodeEntry[n] =
                        0x7c00+n*0x4;
                }/* end of loop n */
            }/*7c00+n*0x4*/
            {/*7000+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 15 ; n++) {
                    regsAddrPtr->EQ.applicationSpecificCPUCodes.TCPUDPDestPortRangeCPUCodeEntryWord0[n] =
                        0x7000+n*0x4;
                }/* end of loop n */
            }/*7000+n*0x4*/
            {/*7800+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 15 ; n++) {
                    regsAddrPtr->EQ.applicationSpecificCPUCodes.TCPUDPDestPortRangeCPUCodeEntryWord1[n] =
                        0x7800+n*0x4;
                }/* end of loop n */
            }/*7800+n*0x4*/
            regsAddrPtr->EQ.applicationSpecificCPUCodes.IPProtocolCPUCodeValidConfig = 0x00007c10;

        }/*end of unit applicationSpecificCPUCodes */


    }/*end of unit EQ */
}

/**
* @internal lpmInit function
* @endinternal
*
* @brief   init the DB - LPM
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None
*/
static void lpmInit
(
    IN  GT_U8 devNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /*start of unit LPM */
        regsAddrPtr->LPM.globalConfig       = 0x00f00000;
        regsAddrPtr->LPM.directAccessMode   = 0x00f00010;
        regsAddrPtr->LPM.exceptionStatus    = 0x00f00020;
        regsAddrPtr->LPM.LPMGeneralIntCause = 0x00f00120;
        regsAddrPtr->LPM.LPMGeneralIntMask  = 0x00f00130;
    }  /*end of unit LPM */
    else
    {/*start of unit LPM */
        regsAddrPtr->LPM.globalConfig       = 0x00D00000;
        regsAddrPtr->LPM.directAccessMode   = 0x00D00010;
        regsAddrPtr->LPM.exceptionStatus    = 0x00D00020;
        regsAddrPtr->LPM.LPMInterruptsCause = 0x00D00100;
        regsAddrPtr->LPM.LPMInterruptsMask  = 0x00D00110;
        regsAddrPtr->LPM.LPMGeneralIntCause = 0x00D00120;
        regsAddrPtr->LPM.LPMGeneralIntMask  = 0x00D00130;
    }/*end of unit LPM */
}

/**
* @internal tcamInit function
* @endinternal
*
* @brief   init the DB - TCAM
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None
*/
static void tcamInit
(
    IN  GT_U8 devNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_U32  maxFloors = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tcam.maxFloors;
    GT_BOOL isSip6 = PRV_CPSS_SIP_6_CHECK_MAC(devNum);
    GT_BOOL isSip5_25 = PRV_CPSS_SIP_5_25_CHECK_MAC(devNum);

    {/*start of unit TCAM */
        {/*start of unit tcamInterrupts */
            regsAddrPtr->TCAM.tcamInterrupts.TCAMInterruptsSummaryMask = 0x00501000;
            regsAddrPtr->TCAM.tcamInterrupts.TCAMInterruptsSummaryCause = 0x00501004;
            regsAddrPtr->TCAM.tcamInterrupts.TCAMInterruptMask = 0x00501008;
            regsAddrPtr->TCAM.tcamInterrupts.TCAMInterruptCause = 0x0050100c;
            regsAddrPtr->TCAM.tcamInterrupts.tcamParityErrorAddr = 0x00501010;

        }/*end of unit tcamInterrupts */


        {/*start of unit tcamArrayConf */
            regsAddrPtr->TCAM.tcamArrayConf.tcamActiveFloors = 0x00504010;

        }/*end of unit tcamArrayConf */


        {/*start of unit parityDaemonRegs */
            regsAddrPtr->TCAM.parityDaemonRegs.parityDaemonCtrl = 0x00503000;
            regsAddrPtr->TCAM.parityDaemonRegs.parityDaemonWriteAdressStart = 0x00503004;
            regsAddrPtr->TCAM.parityDaemonRegs.parityDaemonWriteAdressEnd = 0x00503008;
            regsAddrPtr->TCAM.parityDaemonRegs.parityDaemonWriteAdressJump = 0x0050300c;
            regsAddrPtr->TCAM.parityDaemonRegs.parityDaemonReadAdressStart = 0x00503010;
            regsAddrPtr->TCAM.parityDaemonRegs.parityDaemonReadAdressEnd = 0x00503014;
            regsAddrPtr->TCAM.parityDaemonRegs.parityDaemonReadAdressJump = 0x00503018;
            regsAddrPtr->TCAM.parityDaemonRegs.parityDaemonDefaultWriteDataX0 = 0x0050301c;
            regsAddrPtr->TCAM.parityDaemonRegs.parityDaemonDefaultWriteDataX1 = 0x00503020;
            regsAddrPtr->TCAM.parityDaemonRegs.parityDaemonDefaultWriteDataY0 = 0x00503028;
            regsAddrPtr->TCAM.parityDaemonRegs.parityDaemonDefaultWriteDataX2 = 0x00503024;
            regsAddrPtr->TCAM.parityDaemonRegs.parityDaemonDefaultWriteDataY1 = 0x0050302c;
            regsAddrPtr->TCAM.parityDaemonRegs.parityDaemonDefaultWriteDataY2 = 0x00503030;
            regsAddrPtr->TCAM.parityDaemonRegs.parityDaemonStatus = 0x0050303c;

        }/*end of unit parityDaemonRegs */


        {/*start of unit mgAccess */
            GT_U32  maxGroups = isSip6 ? 3 : 4;

            {/*5021a8+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= maxGroups ; n++) {
                    regsAddrPtr->TCAM.mgAccess.mgKeySizeAndTypeGroup[n] =
                        0x5021a8+n*0x4;
                }/* end of loop n */
            }/*5021a8+n*0x4*/
            {/*5021bc+i*0x4+n*0x14*/
                GT_U32    n,i;
                for(n = 0 ; n <= maxGroups ; n++) {
                    for(i = 0 ; i <= 3 ; i++) {
                        regsAddrPtr->TCAM.mgAccess.mgHitGroupHitNum[n][i] =
                            0x5021bc+i*0x4+n*0x14;
                    }/* end of loop i */
                }/* end of loop n */
            }/*5021bc+i*0x4+n*0x14*/
            {/*502004+i*0x4+n*0x54*/
                GT_U32    n,i;
                for(n = 0 ; n <= maxGroups ; n++) {
                    for(i = 0 ; i <= 20 ; i++) {
                        regsAddrPtr->TCAM.mgAccess.mgKeyGroupChunk[n][i] =
                            0x502004+i*0x4+n*0x54;
                    }/* end of loop i */
                }/* end of loop n */
            }/*502004+i*0x4+n*0x54*/
            regsAddrPtr->TCAM.mgAccess.mgCompRequest = 0x00502000;
            regsAddrPtr->TCAM.mgAccess.mgKeyTypeUpdateReq = 0x00502300;
            regsAddrPtr->TCAM.mgAccess.keyTypeData = 0x00502304;

        }/*end of unit mgAccess */


        {/*500000+n*0x4*/
            GT_U32    n;
            for(n = 0 ; n <= 4 ; n++) {
                regsAddrPtr->TCAM.groupClientEnable[n] =
                    0x500000+n*0x4;
            }/* end of loop n */
        }/*500000+n*0x4*/
        {/*500100+n*0x8*/
            GT_U32    n;
            for(n = 0 ; n < maxFloors ; n++) {
                regsAddrPtr->TCAM.tcamHitNumAndGroupSelFloor[n] =
                    0x500100+n*0x8;
            }/* end of loop n */
        }/*500100+n*0x8*/

        regsAddrPtr->TCAM.tcamGlobalConfig = 0x507008;
        if (isSip5_25 && (! isSip6))
        {
            regsAddrPtr->TCAM.tcamAnswersFloorSample = 0x50700C;
        }

        if (isSip6)
        {/*start of unit exactMatchProfilesTable */
            {/*507080+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 1 ; n++) {
                    regsAddrPtr->TCAM.exactMatchProfilesTable.exactMatchPortMapping[n] =
                        0x507080+n*0x4;
                }/* end of loop n */
            }/*507080+n*0x4*/
            {/*507088+32*4*(n-1)*/
                GT_U32    n;
                for(n = 1 ; n <= 15 ; n++) {
                    regsAddrPtr->TCAM.exactMatchProfilesTable.exactMatchProfileTableControl[n] =
                        0x507088+32*4*(n-1);
                }/* end of loop n */
            }/*507088+32*4*(n-1)*/
            {/*50708C+4*i+32*4*(n-1)*/
                GT_U32    n,i;
                for(n = 1 ; n <= 15 ; n++) {
                    for(i = 0 ; i <= 11 ; i++) {
                        regsAddrPtr->TCAM.exactMatchProfilesTable.exactMatchProfileTableMaskData[n][i] =
                            0x50708C+4*i+32*4*(n-1);
                    }/* end of loop i */
                }/* end of loop n */
            }/*50708C+4*i+32*4*(n-1)*/
            {/*5070C0+4*i+32*4*(n-1)*/
                GT_U32    n,i;
                for(n = 1 ; n <= 15 ; n++) {
                    for(i = 0 ; i <= 7 ; i++) {
                        regsAddrPtr->TCAM.exactMatchProfilesTable.exactMatchProfileTableDefaultActionData[n][i] =
                            0x5070C0+4*i+32*4*(n-1);
                    }/* end of loop i */
                }/* end of loop n */
            }/*5070C0+4*i+32*4*(n-1)*/

        }/*end of unit exactMatchProfilesTable */

        if (isSip6)
        {/*508000+4*i+32*4*n*/
            GT_U32    n;
            GT_U32    i;
            for(n = 0 ; n < 16 ; n++) {
                for(i = 0 ; i < 31 ; i++) {
                    regsAddrPtr->TCAM.exactMatchActionAssignment[n][i] =
                        0x508000+4*i+32*4*n;
                }/* end of loop i */
            }/* end of loop n */
        }/*508000+4*i+32*4*n*/

    }/*end of unit TCAM */
}

static void egfInit_sip6
(
    IN  GT_U8 devNum
);
/**
* @internal egfInit function
* @endinternal
*
* @brief   init the DB - EGF (EFT,SHT,QAG)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None
*/
static void egfInit
(
    IN  GT_U8 devNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_U32  numPhyPorts = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.maxPhysicalPorts;
    GT_U32  numWordsForBitPerPort = NUM_WORDS_FROM_BITS_MAC(numPhyPorts);
    GT_U32  isSip5_16 = PRV_CPSS_SIP_5_16_CHECK_MAC(devNum);
    GT_U32  isSip5_20 = PRV_CPSS_SIP_5_20_CHECK_MAC(devNum);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        egfInit_sip6(devNum);
        return;
    }

    {/*start of unit EGF_eft */
        {/*start of unit MCFIFO */
            {/*start of unit MCFIFOConfigs */
                if(isSip5_20)
                {/*2200+4*n*/
                    GT_U32    n;
                    for(n = 0 ; n <= 31 ; n++) {
                        regsAddrPtr->EGF_eft.MCFIFO.MCFIFOConfigs.portToMcFIFOAttributionReg[n] =
                            0x2200+4*n;
                    }/* end of loop n */
                }/*2200+4*n*/
                else
                {/*21f0+4*n*/
                    GT_U32    n;
                    for(n = 0 ; n < (2*numWordsForBitPerPort) ; n++) {
                        regsAddrPtr->EGF_eft.MCFIFO.MCFIFOConfigs.portToMcFIFOAttributionReg[n] =
                            0x21f0+4*n;
                    }/* end of loop n */
                }/*21f0+4*n*/
                if(isSip5_20)
                {/*2280+4*n*/
                    GT_U32    n;
                    for(n = 0 ; n <= 15 ; n++) {
                        regsAddrPtr->EGF_eft.MCFIFO.MCFIFOConfigs.portToHemisphereMapReg[n] =
                            0x2280+4*n;
                    }/* end of loop n */
                }/*2280+4*n*/
                else
                {/*2240+4*n*/
                    GT_U32    n;
                    for(n = 0 ; n < numWordsForBitPerPort ; n++) {
                        regsAddrPtr->EGF_eft.MCFIFO.MCFIFOConfigs.portToHemisphereMapReg[n] =
                            0x2240+4*n;
                    }/* end of loop n */
                }/*2240+4*n*/
                regsAddrPtr->EGF_eft.MCFIFO.MCFIFOConfigs.MCFIFOGlobalConfigs = 0x00002100;
                regsAddrPtr->EGF_eft.MCFIFO.MCFIFOConfigs.MCFIFOArbiterWeights0 = 0x00002108;
                regsAddrPtr->EGF_eft.MCFIFO.MCFIFOConfigs.lossyDropAndPfcUcThreshold = 0x00002300;

            }/*end of unit MCFIFOConfigs */


        }/*end of unit MCFIFO */


        {/*start of unit global */
            {/*1150+4*n*/
                GT_U32    n;
                GT_U32    nMax;

                nMax = isSip5_20 ? 128 : 64;
                for(n = 0 ; n < nMax ; n++) {
                    regsAddrPtr->EGF_eft.global.cpuPortMapOfReg[n] =
                        0x1150+4*n;
                }/* end of loop n */
            }/*1150+4*n*/
            regsAddrPtr->EGF_eft.global.cpuPortDist          = 0x00001000;
            regsAddrPtr->EGF_eft.global.portIsolationLookup0 = 0x00001020;
            regsAddrPtr->EGF_eft.global.portIsolationLookup1 = 0x00001030;
            regsAddrPtr->EGF_eft.global.clearedUCDist        = 0x00001040;
            regsAddrPtr->EGF_eft.global.EFTInterruptsCause   = 0x000010a0;
            regsAddrPtr->EGF_eft.global.EFTInterruptsMask    = 0x000010b0;
            regsAddrPtr->EGF_eft.global.ECCConf              = 0x00001130;
            regsAddrPtr->EGF_eft.global.ECCStatus            = 0x00001140;

        }/*end of unit global */


        {/*start of unit egrFilterConfigs */
            {/*100f0+0x4*n*/
                GT_U32    n;
                GT_U32    offset = isSip5_20 ? 0x10180 : 0x100f0;
                for(n = 0 ; n < numWordsForBitPerPort ; n++) {
                    regsAddrPtr->EGF_eft.egrFilterConfigs.enFCTriggerByCNFrameOnPort[n] =
                        offset+0x4*n;
                }/* end of loop n */
            }/*100f0+0x4*n*/
            {/*100d0+0x4*n*/
                GT_U32    n;
                GT_U32    offset = isSip5_20 ? 0x10140 : 0x100d0;
                for(n = 0 ; n < numWordsForBitPerPort ; n++) {
                    regsAddrPtr->EGF_eft.egrFilterConfigs.enCNFrameTxOnPort[n] =
                        offset+0x4*n;
                }/* end of loop n */
            }/*100d0+0x4*n*/
            {/*10050+0x4*n*/
                GT_U32    n;
                GT_U32    offset = isSip5_20 ? 0x100c0 : 0x10050;
                for(n = 0 ; n < numWordsForBitPerPort ; n++) {
                    regsAddrPtr->EGF_eft.egrFilterConfigs.unregedBCFilterEn[n] =
                        offset+0x4*n;
                }/* end of loop n */
            }/*10050+0x4*n*/
            {/*10010+0x4*n*/
                GT_U32    n;
                GT_U32    offset = isSip5_20 ? 0x10040 : 0x10010;
                for(n = 0 ; n < numWordsForBitPerPort ; n++) {
                    regsAddrPtr->EGF_eft.egrFilterConfigs.unknownUCFilterEn[n] =
                        offset+0x4*n;
                }/* end of loop n */
            }/*10010+0x4*n*/
            {/*10030+0x4*n*/
                GT_U32    n;
                GT_U32    offset = isSip5_20 ? 0x10080 : 0x10030;
                for(n = 0 ; n < numWordsForBitPerPort ; n++) {
                    regsAddrPtr->EGF_eft.egrFilterConfigs.unregedMCFilterEn[n] =
                        offset+0x4*n;
                }/* end of loop n */
            }/*10030+0x4*n*/
            {/*100b0+0x4*n*/
                GT_U32    n;
                GT_U32    offset = isSip5_20 ? 0x10100 : 0x100b0;
                for(n = 0 ; n < numWordsForBitPerPort ; n++) {
                    regsAddrPtr->EGF_eft.egrFilterConfigs.OAMLoopbackFilterEn[n] =
                        offset+0x4*n;
                }/* end of loop n */
            }/*100b0+0x4*n*/
            if(!isSip5_20)
            {/*10120+4*n*/
                GT_U32    n;
                GT_U32    offset = 0x10120;
                GT_U32    numRegs = (8*numWordsForBitPerPort);
                for(n = 0 ; n < numRegs ; n++) {
                    regsAddrPtr->EGF_eft.egrFilterConfigs.virtual2PhysicalPortRemap[n] =
                        offset+4*n;
                }/* end of loop n */
            }/*10120+4*n*/
            if(!isSip5_20)
            {/*10220+4*n*/
                GT_U32    n;
                GT_U32    offset = 0x10220;
                for(n = 0 ; n < (2*numWordsForBitPerPort) ; n++) {
                    regsAddrPtr->EGF_eft.egrFilterConfigs.physicalPortLinkStatusMask[n] =
                        offset+4*n;
                }/* end of loop n */
            }/*10220+4*n*/
            else
            {/*101c0+4*n*/
                GT_U32    n;
                GT_U32    offset = 0x101c0;
                for(n = 0 ; n < numWordsForBitPerPort ; n++) {
                    regsAddrPtr->EGF_eft.egrFilterConfigs.physicalPortLinkStatusMask[n] =
                        offset+4*n;
                }/* end of loop n */
            }/*101c0+4*n*/
            regsAddrPtr->EGF_eft.egrFilterConfigs.egrFiltersGlobal = 0x00010000;
            regsAddrPtr->EGF_eft.egrFilterConfigs.egrFiltersEnable = 0x00010004;

        }/*end of unit egrFilterConfigs */


        {/*start of unit deviceMapConfigs */
            {/*3000+4*p*/
                GT_U32    p;
                for(p = 0 ; p <= 7 ; p++) {
                    regsAddrPtr->EGF_eft.deviceMapConfigs.devMapTableAddrConstructProfile[p] =
                        0x3000+4*p;
                }/* end of loop p */
            }/*3000+4*p*/
            {/*3020+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n < numWordsForBitPerPort ; n++) {
                    regsAddrPtr->EGF_eft.deviceMapConfigs.localSrcPortMapOwnDevEn[n] =
                        0x3020+0x4*n;
                }/* end of loop n */
            }/*3020+0x4*n*/
            {/*3040+0x4*n*/
                GT_U32    n;
                GT_U32    offset = isSip5_20 ? 0x3060 : 0x3040;
                for(n = 0 ; n < numWordsForBitPerPort ; n++) {
                    regsAddrPtr->EGF_eft.deviceMapConfigs.localTrgPortMapOwnDevEn[n] =
                        offset+0x4*n;
                }/* end of loop n */
            }/*3040+0x4*n*/
            {/*3060+4*n*/
                GT_U32    n;
                GT_U32    offset = isSip5_20 ? 0x3100 : 0x3060;
                GT_U32    numRegs = isSip5_20 ? numPhyPorts/8 : numPhyPorts;
                for(n = 0 ; n < numRegs ; n++) {
                    regsAddrPtr->EGF_eft.deviceMapConfigs.portAddrConstructMode[n] =
                        offset+4*n;
                }/* end of loop n */
            }/*3060+4*n*/

        }/*end of unit deviceMapConfigs */

        {/*start of unit counters */
            regsAddrPtr->EGF_eft.counters.eftDescEcc1ErrorCntr = 0x00001120;
        }

    }/*end of unit EGF_eft */


    {/*start of unit EGF_qag */
        {/*start of unit uniMultiArb */
            regsAddrPtr->EGF_qag.uniMultiArb.uniMultiCtrl = 0x00f00100;
            regsAddrPtr->EGF_qag.uniMultiArb.uniWeights = 0x00f00104;
            regsAddrPtr->EGF_qag.uniMultiArb.multiWeights = 0x00f00108;
            regsAddrPtr->EGF_qag.uniMultiArb.multiShaperCtrl = 0x00f0010c;

        }/*end of unit uniMultiArb */


        {/*start of unit global */
            regsAddrPtr->EGF_qag.global.QAGInterruptsCause = 0x00f00010;
            regsAddrPtr->EGF_qag.global.QAGInterruptsMask = 0x00f00020;
            if(isSip5_16)
            {
                regsAddrPtr->EGF_qag.global.QAGMetalFix = 0x00f00030;
            }

        }/*end of unit global */


        {/*start of unit distributor */
            if(! isSip5_20)
            {/*e00a04+4*n*/
                GT_U32    n;
                for(n = 0 ; n < numWordsForBitPerPort ; n++) {
                    regsAddrPtr->EGF_qag.distributor.useVlanTag1ForTagStateReg[n] =
                        0xe00a04+4*n;
                }/* end of loop n */
            }/*e00a04+4*n*/
            {/*e00180+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 7 ; n++) {
                    regsAddrPtr->EGF_qag.distributor.toAnalyzerLoopback[n] =
                        0xe00180+0x4*n;
                }/* end of loop n */
            }/*e00180+0x4*n*/
            if(! isSip5_20)
            {/*e001e0+4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    regsAddrPtr->EGF_qag.distributor.TCRemap[n] =
                        0xe001e0+4*n;
                }/* end of loop n */
            }/*e001e0+4*n*/
            if(! isSip5_20)
            {/*e00050+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n < numWordsForBitPerPort ; n++) {
                    regsAddrPtr->EGF_qag.distributor.stackRemapEn[n] =
                        0xe00050+0x4*n;
                }/* end of loop n */
            }/*e00050+0x4*n*/
            if(! isSip5_20)
            {/*e00130+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n < 16 ; n++) {
                    regsAddrPtr->EGF_qag.distributor.perTrgPhyPortLbProfile[n] =
                        0xe00130+0x4*n;
                }/* end of loop n */
            }/*e00130+0x4*n*/
            if(! isSip5_20)
            {/*e000f0+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n < 16 ; n++) {
                    regsAddrPtr->EGF_qag.distributor.perSrcPhyPortLbProfile[n] =
                        0xe000f0+0x4*n;
                }/* end of loop n */
            }/*e000f0+0x4*n*/
            regsAddrPtr->EGF_qag.distributor.loopbackForwardingGlobalConfig = 0x00e00a00;
            regsAddrPtr->EGF_qag.distributor.loopbackEnableConfig[0] = 0x00e00170;
            if(! isSip5_20)
            {/*e001c0+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n < numWordsForBitPerPort ; n++) {
                    regsAddrPtr->EGF_qag.distributor.evlanMirrEnable[n] =
                        0xe001c0+0x4*n;
                }/* end of loop n */
            }/*e001c0+0x4*n*/
            if(! isSip5_20)
            {/*e005fc+4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 15 ; n++) {
                    regsAddrPtr->EGF_qag.distributor.DPRemap[n] =
                        0xe005fc+4*n;
                }/* end of loop n */
            }/*e005fc+4*n*/
            regsAddrPtr->EGF_qag.distributor.distributorGeneralConfigs = 0x00e00000;

        }/*end of unit distributor */


    }/*end of unit EGF_qag */



    {/*start of unit EGF_sht */
        {/*start of unit global */
            {/*60200f0+0x4*n*/
                GT_U32    n;
                GT_U32    offset = isSip5_20 ? 0x60201c0 : 0x60200f0;
                for(n = 0 ; n < numWordsForBitPerPort ; n++) {
                    regsAddrPtr->EGF_sht.global.eportStpState[n] =
                        offset+0x4*n;
                }/* end of loop n */
            }/*60200f0+0x4*n*/
            {/*60200b0+0x4*n*/
                GT_U32    n;
                GT_U32    offset = isSip5_20 ? 0x6020140 : 0x60200b0;
                for(n = 0 ; n < numWordsForBitPerPort ; n++) {
                    regsAddrPtr->EGF_sht.global.eportEvlanFilteringEnable[n] =
                        offset+0x4*n;
                }/* end of loop n */
            }/*60200b0+0x4*n*/
            regsAddrPtr->EGF_sht.global.SHTInterruptsCause = 0x06020010;
            {/*6020030+0x4*n*/
                GT_U32    n;
                GT_U32    offset = isSip5_20 ? 0x6020040 : 0x6020030;
                for(n = 0 ; n < numWordsForBitPerPort ; n++) {
                    regsAddrPtr->EGF_sht.global.UCSrcIDFilterEn[n] =
                        offset+0x4*n;
                }/* end of loop n */
            }/*6020030+0x4*n*/
            regsAddrPtr->EGF_sht.global.SHTEgrFiltersEnable = 0x06020008;
            {/*6020270+0x4*n*/
                GT_U32    n;
                GT_U32    offset = isSip5_20 ? 0x60208c0 : 0x6020270;
                for(n = 0 ; n < numWordsForBitPerPort ; n++) {
                    regsAddrPtr->EGF_sht.global.MCLocalEn[n] =
                        offset+0x4*n;
                }/* end of loop n */
            }/*6020270+0x4*n*/
            {/*6020240+0x4*n*/
                GT_U32    n;
                GT_U32    offset = isSip5_20 ? 0x6020880 : 0x6020240;
                for(n = 0 ; n < numWordsForBitPerPort ; n++) {
                    regsAddrPtr->EGF_sht.global.ignorePhySrcMcFilterEn[n] =
                        offset+0x4*n;
                }/* end of loop n */
            }/*6020240+0x4*n*/
            regsAddrPtr->EGF_sht.global.eportVlanEgrFiltering =
                                isSip5_20 ? 0x0602000c : 0x06020310;

            {/*60200d0+0x4*n*/
                GT_U32    n;
                GT_U32    offset = isSip5_20 ? 0x6020180 : 0x60200d0;
                for(n = 0 ; n < numWordsForBitPerPort ; n++) {
                    regsAddrPtr->EGF_sht.global.eportStpStateMode[n] =
                        offset+0x4*n;
                }/* end of loop n */
            }/*60200d0+0x4*n*/
            {/*6020190+0x4*n*/
                GT_U32    n;
                GT_U32    offset = isSip5_20 ? 0x60202c0 : 0x6020190;
                for(n = 0 ; n < numWordsForBitPerPort ; n++) {
                    regsAddrPtr->EGF_sht.global.eportOddOnlySrcIdFilterEnable[n] =
                        offset+0x4*n;
                }/* end of loop n */
            }/*6020190+0x4*n*/
            {/*6020090+0x4*n*/
                GT_U32    n;
                GT_U32    offset = isSip5_20 ? 0x6020100 : 0x6020090;
                for(n = 0 ; n < numWordsForBitPerPort ; n++) {
                    regsAddrPtr->EGF_sht.global.eportRoutedForwardRestricted[n] =
                        offset+0x4*n;
                }/* end of loop n */
            }/*6020090+0x4*n*/
            {/*6020130+0x4*n*/
                GT_U32    n;
                GT_U32    offset = isSip5_20 ? 0x6020200 : 0x6020130;
                for(n = 0 ; n < (2*numWordsForBitPerPort) ; n++) {
                    regsAddrPtr->EGF_sht.global.eportPortIsolationMode[n] =
                        offset+0x4*n;
                }/* end of loop n */
            }/*6020130+0x4*n*/
            regsAddrPtr->EGF_sht.global.SHTGlobalConfigs = 0x06020000;
            {/*6020170+0x4*n*/
                GT_U32    n;
                GT_U32    offset = isSip5_20 ? 0x6020280 : 0x6020170;
                for(n = 0 ; n < numWordsForBitPerPort ; n++) {
                    regsAddrPtr->EGF_sht.global.eportIPMCRoutedFilterEn[n] =
                        offset+0x4*n;
                }/* end of loop n */
            }/*6020170+0x4*n*/
            {/*6020050+0x4*n*/
                GT_U32    n;
                GT_U32    offset = isSip5_20 ? 0x6020080 : 0x6020050;
                for(n = 0 ; n < numWordsForBitPerPort ; n++) {
                    regsAddrPtr->EGF_sht.global.eportFromCPUForwardRestricted[n] =
                        offset+0x4*n;
                }/* end of loop n */
            }/*6020050+0x4*n*/
            regsAddrPtr->EGF_sht.global.SHTInterruptsMask = 0x06020020;
            {/*6020070+0x4*n*/
                GT_U32    n;
                GT_U32    offset = isSip5_20 ? 0x60200c0 : 0x6020070;
                for(n = 0 ; n < numWordsForBitPerPort ; n++) {
                    regsAddrPtr->EGF_sht.global.eportBridgedForwardRestricted[n] =
                        offset+0x4*n;
                }/* end of loop n */
            }/*6020070+0x4*n*/

            regsAddrPtr->EGF_sht.global.meshIdConfigs = 0x06020004;
            {/*6020320+0x4*n*/
                GT_U32    n;
                GT_U32    offset = isSip5_20 ? 0x6020300 : 0x6020320;
                for(n = 0 ; n < (4*numWordsForBitPerPort) ; n++) {
                    regsAddrPtr->EGF_sht.global.ePortMeshId[n] =
                        offset+0x4*n;
                }/* end of loop n */
            }/*6020320+0x4*n*/
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
            {
                {/*60203A0+0x4*n*/
                    GT_U32    n;
                    GT_U32    offset = isSip5_20 ? 0x6020800 : 0x60203a0;
                    for (n = 0; n < numWordsForBitPerPort ; n++) {
                        regsAddrPtr->EGF_sht.global.dropOnEportVid1MismatchEn[n] =
                            offset+0x4*n;
                    }/* end of loop n */
                }/*60203A0+0x4*n*/
                {/*6020400+0x4*n*/
                    GT_U32    n;
                    for (n = 0; n < (16*numWordsForBitPerPort) ; n++) {
                        regsAddrPtr->EGF_sht.global.eportAssociatedVid1[n] =
                            0x6020400+0x4*n;
                    }/* end of loop n */
                }/*6020400+0x4*n*/
            }
        }/*end of unit global */


    }/*end of unit EGF_sht */

    if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
    {
        regsAddrPtr->EGF_eft.counters.eftDescEcc1ErrorCntr = 0x00004008;

        DESTROY_SINGLE_REG_MAC(regsAddrPtr->EGF_eft.MCFIFO.MCFIFOConfigs.lossyDropAndPfcUcThreshold);
        {/*e00170+0x4*n*/
            GT_U32    n;
            for(n = 0; n <= 3 ; n++) {
                regsAddrPtr->EGF_qag.distributor.loopbackEnableConfig[n] =
                    0xe00170+0x4*n;
            }/* end of loop n */
        }/*e00170+0x4*n*/
    }
}

/**
* @internal haInit function
* @endinternal
*
* @brief   init the DB - HA (header alteration) unit
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None
*/
static void haInit
(
    IN  GT_U8 devNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    {/*start of unit HA */
        regsAddrPtr->HA.invalidPTPHeaderCntr = 0x00000014;
        regsAddrPtr->HA.TRILLEtherType = 0x00000558;
        {/*500+n*4*/
            GT_U32    n;
            for(n = 0 ; n <= 7 ; n++) {
                regsAddrPtr->HA.TPIDConfigReg[n] =
                    0x500+n*4;
            }/* end of loop n */
        }/*500+n*4*/
        regsAddrPtr->HA.routerMACSABaseReg1 = 0x00000144;
        {/*20+(w-1)*4*/
            GT_U32    w;
            for(w = 1 ; w <= 15 ; w++) {
                regsAddrPtr->HA.PWCtrlWord[w-1] =
                    0x20+(w-1)*4;
            }/* end of loop w */
        }/*20+(w-1)*4*/
        regsAddrPtr->HA.PWETreeEnable = 0x00000060;
        regsAddrPtr->HA.invalidPTPOutgoingPiggybackCntr = 0x00000018;
        regsAddrPtr->HA.PTPTimestampTagEthertype = 0x0000000c;
        regsAddrPtr->HA.PTPAndTimestampingExceptionConfig = 0x00000010;
        regsAddrPtr->HA.MPLSEtherType = 0x00000550;
        regsAddrPtr->HA.MPLSChannelTypeProfileReg7 = 0x0000016c;
        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            regsAddrPtr->HA.PTPExceptionCPUCodeConfig = 0x000000A0;
        }

        {/*150+4*r*/
            GT_U32    r;
            for(r = 0 ; r <= 6 ; r++) {
                regsAddrPtr->HA.MPLSChannelTypeProfileReg[r] =
                    0x150+4*r;
            }/* end of loop r */
        }/*150+4*r*/
        regsAddrPtr->HA.IPLengthOffset = 0x00000560;
        {/*120+4*n*/
            GT_U32    n;
            for(n = 0 ; n <= 7 ; n++) {
                regsAddrPtr->HA.egrUDPPortRangeComparatorConfig[n] =
                    0x120+4*n;
            }/* end of loop n */
        }/*120+4*n*/
        regsAddrPtr->HA.invalidPTPIncomingPiggybackCntr = 0x0000001c;
        regsAddrPtr->HA.egrPolicyDIPSolicitationData1 = 0x00000084;
        regsAddrPtr->HA.ingrAnalyzerVLANTagConfig = 0x00000408;
        regsAddrPtr->HA.IEtherType = 0x00000554;
        regsAddrPtr->HA.haGlobalConfig1 = 0x00000004;
        regsAddrPtr->HA.haGlobalConfig = 0x00000000;
        regsAddrPtr->HA.HAInterruptMask = 0x00000304;
        regsAddrPtr->HA.HAInterruptCause = 0x00000300;
        regsAddrPtr->HA.ethernetOverGREProtocolType = 0x0000055c;
        regsAddrPtr->HA.routerMACSABaseReg0 = 0x00000140;
        {/*100+4*n*/
            GT_U32    n;
            for(n = 0 ; n <= 7 ; n++) {
                regsAddrPtr->HA.egrTCPPortRangeComparatorConfig[n] =
                    0x100+4*n;
            }/* end of loop n */
        }/*100+4*n*/
        regsAddrPtr->HA.egrPolicyDIPSolicitationMask3 = 0x0000009c;
        regsAddrPtr->HA.egrPolicyDIPSolicitationMask2 = 0x00000098;
        regsAddrPtr->HA.egrPolicyDIPSolicitationMask1 = 0x00000094;
        regsAddrPtr->HA.egrPolicyDIPSolicitationMask0 = 0x00000090;
        regsAddrPtr->HA.egrPolicyDIPSolicitationData3 = 0x0000008c;
        regsAddrPtr->HA.egrPolicyDIPSolicitationData2 = 0x00000088;
        regsAddrPtr->HA.tunnelStartFragmentIDConfig = 0x00000410;
        regsAddrPtr->HA.egrPolicyDIPSolicitationData0 = 0x00000080;
        regsAddrPtr->HA.egrAnalyzerVLANTagConfig = 0x0000040c;
        regsAddrPtr->HA.dataECCStatus = 0x00000064;
        regsAddrPtr->HA.CPIDReg1 = 0x00000434;
        regsAddrPtr->HA.CPIDReg0 = 0x00000430;
        regsAddrPtr->HA.congestionNotificationConfig = 0x00000420;
        regsAddrPtr->HA.CNMHeaderConfig = 0x00000424;
        regsAddrPtr->HA.BPEConfigReg2 = 0x0000006c;
        regsAddrPtr->HA.BPEConfigReg1 = 0x00000068;

        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            regsAddrPtr->HA.QCNModifications = 0x00000428;
        }

        if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
        {
            regsAddrPtr->HA.haBadAddrLatchReg = 0x00000308;
            regsAddrPtr->HA.NATConfig = 0x00000070;
            regsAddrPtr->HA.NATExceptionDropCounter = 0x000000314;
        }

        if(PRV_CPSS_SIP_5_25_CHECK_MAC(devNum))
        {
            regsAddrPtr->HA.haGlobalConfig2 = 0x00000714;
        }

    }/*end of unit HA */
}


/**
* @internal mllInit function
* @endinternal
*
* @brief   init the DB - MLL (IP_MLL and L2_MLL) unit
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None
*/
static void mllInit
(
    IN  GT_U8 devNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_U32  isSip6    = PRV_CPSS_SIP_6_CHECK_MAC(devNum);

    {/*start of unit MLL */
        {/*start of unit multiTargetRateShape */
            regsAddrPtr->MLL.multiTargetRateShape.multiTargetRateShapeConfig = 0x00000210;
            regsAddrPtr->MLL.multiTargetRateShape.multiTargetRateShapeConfig2 = 0x00000218;
            regsAddrPtr->MLL.multiTargetRateShape.multiTargetMTUReg = 0x0000021c;
            regsAddrPtr->MLL.multiTargetRateShape.tokenBucketBaseLine = 0x00000220;

        }/*end of unit multiTargetRateShape */


        {/*start of unit mcLinkedListMLLTables */
            {/*100+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 63 ; n++) {
                    regsAddrPtr->MLL.mcLinkedListMLLTables.qoSProfileToMultiTargetTCQueuesReg[n] =
                        0x100+n*0x4;
                }/* end of loop n */
            }/*100+n*0x4*/
            {/*400+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 1 ; n++) {
                    regsAddrPtr->MLL.mcLinkedListMLLTables.extQoSModeMCQueueSelectionReg[n] =
                        0x400+n*0x4;
                }/* end of loop n */
            }/*400+n*0x4*/

        }/*end of unit mcLinkedListMLLTables */


        {/*start of unit multiTargetVsUcSDWRRAndStrictPriorityScheduler */
            regsAddrPtr->MLL.multiTargetVsUcSDWRRAndStrictPriorityScheduler.mcUcSDWRRAndStrictPriorityConfig = 0x00000214;

        }/*end of unit multiTargetVsUcSDWRRAndStrictPriorityScheduler */


        {/*start of unit multiTargetTCQueuesAndArbitrationConfig */
            regsAddrPtr->MLL.multiTargetTCQueuesAndArbitrationConfig.multitargetTCQueuesGlobalConfig = 0x00000004;

            if(isSip6)
            {
                regsAddrPtr->MLL.multiTargetTCQueuesAndArbitrationConfig.multitargetTCQueuesPriorityConfig =  0x00000208;
            }
            else
            {
                regsAddrPtr->MLL.multiTargetTCQueuesAndArbitrationConfig.multitargetTCQueuesStrictPriorityEnableConfig = 0x00000200;
            }


            regsAddrPtr->MLL.multiTargetTCQueuesAndArbitrationConfig.multitargetTCQueuesWeightConfig = 0x00000204;

        }/*end of unit multiTargetTCQueuesAndArbitrationConfig */


        {/*start of unit multiTargetEPortMap */

            if(isSip6)
            {
                regsAddrPtr->MLL.multiTargetEPortMap.multiTargetGlobalEPortMinValue = 0x00000310;
                regsAddrPtr->MLL.multiTargetEPortMap.multiTargetGlobalEPortMaxValue = 0x00000314;
            }
            else
            {
                regsAddrPtr->MLL.multiTargetEPortMap.multiTargetEPortValue = 0x00000300;
                regsAddrPtr->MLL.multiTargetEPortMap.multiTargetEPortMask = 0x00000304;
            }
            regsAddrPtr->MLL.multiTargetEPortMap.multiTargetEPortBase = 0x00000308;
            regsAddrPtr->MLL.multiTargetEPortMap.ePortToEVIDXBase = 0x0000030c;

        }/*end of unit multiTargetEPortMap */


        {/*start of unit MLLOutInterfaceCntrs */
            {/*c84+n*0x100*/
                GT_U32    n;
                for(n = 0 ; n <= 1 ; n++) {
                    regsAddrPtr->MLL.MLLOutInterfaceCntrs.L2MLLOutInterfaceCntrConfig1[n] =
                        0xc84+n*0x100;
                }/* end of loop n */
            }/*c84+n*0x100*/
            {/*c80+n*0x100*/
                GT_U32    n;
                for(n = 0 ; n <= 1 ; n++) {
                    regsAddrPtr->MLL.MLLOutInterfaceCntrs.L2MLLOutInterfaceCntrConfig[n] =
                        0xc80+n*0x100;
                }/* end of loop n */
            }/*c80+n*0x100*/
            {/*e20+(n-1)*0x4*/
                GT_U32    n;
                for(n = 1 ; n <= 3 ; n++) {
                    regsAddrPtr->MLL.MLLOutInterfaceCntrs.L2MLLValidProcessedEntriesCntr[n-1] =
                        0xe20+(n-1)*0x4;
                }/* end of loop n */
            }/*e20+(n-1)*0x4*/
            regsAddrPtr->MLL.MLLOutInterfaceCntrs.L2MLLTTLExceptionCntr = 0x00000e08;
            regsAddrPtr->MLL.MLLOutInterfaceCntrs.L2MLLSkippedEntriesCntr = 0x00000e04;
            regsAddrPtr->MLL.MLLOutInterfaceCntrs.L2MLLSilentDropCntr = 0x00000e00;
            {/*c00+n*0x100*/
                GT_U32    n;
                for(n = 0 ; n <= 1 ; n++) {
                    regsAddrPtr->MLL.MLLOutInterfaceCntrs.L2MLLOutMcPktsCntr[n] =
                        0xc00+n*0x100;
                }/* end of loop n */
            }/*c00+n*0x100*/
            regsAddrPtr->MLL.MLLOutInterfaceCntrs.MLLSilentDropCntr = 0x00000800;
            regsAddrPtr->MLL.MLLOutInterfaceCntrs.MLLMCFIFODropCntr = 0x00000804;
            regsAddrPtr->MLL.MLLOutInterfaceCntrs.IPMLLSkippedEntriesCntr = 0x00000b04;
            {/*900+n*0x100*/
                GT_U32    n;
                for(n = 0 ; n <= 1 ; n++) {
                    regsAddrPtr->MLL.MLLOutInterfaceCntrs.IPMLLOutMcPktsCntr[n] =
                        0x900+n*0x100;
                }/* end of loop n */
            }/*900+n*0x100*/
            {/*984+n*0x100*/
                GT_U32    n;
                for(n = 0 ; n <= 1 ; n++) {
                    regsAddrPtr->MLL.MLLOutInterfaceCntrs.IPMLLOutInterfaceCntrConfig1[n] =
                        0x984+n*0x100;
                }/* end of loop n */
            }/*984+n*0x100*/
            {/*980+n*0x100*/
                GT_U32    n;
                for(n = 0 ; n <= 1 ; n++) {
                    regsAddrPtr->MLL.MLLOutInterfaceCntrs.IPMLLOutInterfaceCntrConfig[n] =
                        0x980+n*0x100;
                }/* end of loop n */
            }/*980+n*0x100*/

        }/*end of unit MLLOutInterfaceCntrs */


        {/*start of unit MLLGlobalCtrl */
            regsAddrPtr->MLL.MLLGlobalCtrl.MLLGlobalCtrl = 0x00000000;
            regsAddrPtr->MLL.MLLGlobalCtrl.MLLLookupTriggerConfig = 0x0000000c;
            regsAddrPtr->MLL.MLLGlobalCtrl.globalTTLExceptionConfig = 0x00000010;
            if(isSip6)
            {
                regsAddrPtr->MLL.MLLGlobalCtrl.L2mllGlobalEPortMinValue = 0x00000068;
                regsAddrPtr->MLL.MLLGlobalCtrl.L2mllGlobalEPortMaxValue = 0x0000006c;
            }
            else
            {
                regsAddrPtr->MLL.MLLGlobalCtrl.globalEPortRangeConfig = 0x00000014;
                regsAddrPtr->MLL.MLLGlobalCtrl.globalEPortRangeMask = 0x00000018;
            }
            regsAddrPtr->MLL.MLLGlobalCtrl.IPMLLTableResourceSharingConfig = 0x00000020;
            regsAddrPtr->MLL.MLLGlobalCtrl.L2MLLTableResourceSharingConfig = 0x00000024;
            regsAddrPtr->MLL.MLLGlobalCtrl.MLLInterruptCauseReg = 0x00000030;
            regsAddrPtr->MLL.MLLGlobalCtrl.MLLInterruptMaskReg = 0x00000034;
            regsAddrPtr->MLL.MLLGlobalCtrl.MLLReplicatedTrafficCtrl = 0x00000040;
            regsAddrPtr->MLL.MLLGlobalCtrl.sourceBasedL2MLLFiltering = 0x00000054;
            regsAddrPtr->MLL.MLLGlobalCtrl.MLLEccStatus = 0x00000058;
            regsAddrPtr->MLL.MLLGlobalCtrl.MLLMCSourceID = 0x00000060;

        }/*end of unit MLLGlobalCtrl */


    }/*end of unit MLL */
}

/**
* @internal pclInit function
* @endinternal
*
* @brief   init the DB - PCL unit (IPCL)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None
*/
static void pclInit
(
    IN  GT_U8 devNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_U32  isSip5_20 = PRV_CPSS_SIP_5_20_CHECK_MAC(devNum);
    GT_U32  isSip6    = PRV_CPSS_SIP_6_CHECK_MAC(devNum);
    GT_U32  iPcl0Bypass    = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.pcl.iPcl0Bypass;

    {/*start of unit PCL */
        {/*start of unit OAMConfigs */
            {/*740+4*n*/
                GT_U32    n;
                for(n = 1 ; n <= 15 ; n++) {
                    regsAddrPtr->PCL.OAMConfigs.channelTypeOAMOpcode[n-1] =
                        0x740+4*n;
                }/* end of loop n */
            }/*740+4*n*/
            regsAddrPtr->PCL.OAMConfigs.OAMRDIConfigs = 0x00000730;
            regsAddrPtr->PCL.OAMConfigs.OAMMEGLevelModeConfigs = 0x00000734;
            regsAddrPtr->PCL.OAMConfigs.OAMRFlagConfigs = 0x00000738;
            regsAddrPtr->PCL.OAMConfigs.RBitAssignmentEnable = 0x0000073c;

        }/*end of unit OAMConfigs */


        {/*start of unit HASH */
            {/*5c0+4*i*/
                GT_U32    i;
                for(i = 0 ; i <= 15 ; i++) {
                    regsAddrPtr->PCL.HASH.pearsonHashTable[i] =
                        0x5c0+4*i;
                }/* end of loop i */
            }/*5c0+4*i*/
            regsAddrPtr->PCL.HASH.CRCHashConfig = 0x00000020;
            {/*804+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 19 ; n++) {
                    regsAddrPtr->PCL.HASH.CRC32Salt[n] =
                        0x804+0x4*n;
                }/* end of loop n */
            }/*804+0x4*n*/
            regsAddrPtr->PCL.HASH.CRC32Seed = 0x00000800;

        }/*end of unit HASH */


        regsAddrPtr->PCL.ingrPolicyGlobalConfig = 0x00000000;
        regsAddrPtr->PCL.PCLUnitInterruptCause = 0x00000004;
        regsAddrPtr->PCL.PCLUnitInterruptMask = 0x00000008;
        regsAddrPtr->PCL.policyEngineConfig = 0x0000000c;
        regsAddrPtr->PCL.policyEngineUserDefinedBytesConfig = 0x00000014;
        {/*34+4*n*/
            GT_U32    n;
            GT_U32    nMax = iPcl0Bypass ? 1 : 2;
            GT_U32    base = iPcl0Bypass ? 0x38 : 0x34;
            for(n = 0 ; n <= nMax ; n++) {
                regsAddrPtr->PCL.lookupSourceIDMask[n] =
                    base+4*n;
            }/* end of loop n */
        }/*34+4*n*/
        regsAddrPtr->PCL.policyEngineUserDefinedBytesExtConfig = 0x00000024;
        {/*74+0x4*n*/
            GT_U32    n;
            for(n = 0 ; n <= 1 ; n++) {
                regsAddrPtr->PCL.TCAMKeyTypeMap[n] =
                    0x74+0x4*n;
            }/* end of loop n */
        }/*74+0x4*n*/
        {/*80+4*i*/
            GT_U32    i;
            GT_U32    nMax = isSip6 ? 31 : isSip5_20 ? 15 : 7;
            for(i = 0 ; i <= nMax ; i++) {
                regsAddrPtr->PCL.L2L3IngrVLANCountingEnable[i] =
                    0x80+4*i;
            }/* end of loop i */
        }/*80+4*i*/
        regsAddrPtr->PCL.countingModeConfig = 0x00000070;

        if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
        {
            regsAddrPtr->PCL.pclBadAddressLatchReg = 0x00000010;
            regsAddrPtr->PCL.CFMEtherType = 0x00000018;
        }

        if(isSip6)
        {
            regsAddrPtr->PCL.CRCHashModes = 0x00000100;
            {/*200+4*i*/
                GT_U32    i;
                for(i = 0 ; i <= 127 ; i++) {
                    regsAddrPtr->PCL.exactMatchProfileIdMapEntry[i] =
                        0x200+4*i;
                }/* end of loop i */
            }/*200+4*i*/
        }
    }/*end of unit PCL */
}

/**
* @internal epclInit function
* @endinternal
*
* @brief   init the DB - EPCL unit (EPCL)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None
*/
static void epclInit
(
    IN  GT_U8 devNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_U32  isSip6    = PRV_CPSS_SIP_6_CHECK_MAC(devNum);

    {/*start of unit EPCL */
        regsAddrPtr->EPCL.egrPolicyGlobalConfig = 0x00000000;
        regsAddrPtr->EPCL.EPCLOAMPktDetection = 0x00000004;
        regsAddrPtr->EPCL.EPCLOAMMEGLevelMode = 0x00000008;
        regsAddrPtr->EPCL.EPCLOAMRDI = 0x0000000c;
        regsAddrPtr->EPCL.EPCLInterruptCause = 0x00000010;
        regsAddrPtr->EPCL.EPCLInterruptMask = 0x00000014;
        {/*30+4*s*/
            GT_U32    s;
            for(s = 0 ; s <= 4 ; s++) {
                regsAddrPtr->EPCL.EPCLChannelTypeToOAMOpcodeMap[s] =
                    0x30+4*s;
            }/* end of loop s */
        }/*30+4*s*/
        regsAddrPtr->EPCL.EPCLReservedBitMask = 0x00000020;
        regsAddrPtr->EPCL.TCAMKeyTypeMap = 0x00000074;

    }/*end of unit EPCL */

    if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
    {
        regsAddrPtr->EPCL.EPCLBadAddrLatchReg = 0x00000018;
        regsAddrPtr->EPCL.oamRBbitConfigReg = 0x0000001c;
        regsAddrPtr->EPCL.EPCLFifoConfiguration = 0x000000a0;
    }

    if(isSip6)
    {

        regsAddrPtr->EPCL.PHAMetadataMask1 = 0x00000100;
        regsAddrPtr->EPCL.PHAMetadataMask2 = 0x00000104;
        regsAddrPtr->EPCL.PHAMetadataMask3 = 0x00000108;
        regsAddrPtr->EPCL.PHAMetadataMask4 = 0x0000010C;
        regsAddrPtr->EPCL.PHAThreadNumMask = 0x00000110;
        {/*120+4*i*/
            GT_U32    i;
            for(i = 0 ; i <= 7 ; i++) {
                regsAddrPtr->EPCL.latencyMonitoringSamplingConfig[i] =
                    0x120+4*i;
            }/* end of loop i */
        }/*120+4*i*/
        {/*300+4*i*/
            GT_U32    i;
            for(i = 0 ; i <= 63 ; i++) {
                regsAddrPtr->EPCL.latencyMonitoringProfile2SamplingProfile[i] =
                    0x00000300+4*i;
            }/* end of loop i */
        }/*300+4*i*/
        regsAddrPtr->EPCL.latencyMonitoringControl = 0x00000164;
    }

}

/* DMA of CPU port (MG[0]) :
    72 - BC2,Bobk,Aldrin
    74 - BC3,Aldrin2
    65 - Falcon
*/
#define  CPU_PORT_DMA_CNS(devNum)   \
        PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.cpuPortInfo.info[0].dmaNum

/* copy from index to index */
#define COPY_REG_ADDR_MAC(regArr,srcIndex,offset , trgIndex) \
    regArr[(int)trgIndex - (int)offset] = regArr[srcIndex]


/**
* @internal rxdmaInit function
* @endinternal
*
* @brief   init the DB - RXDMA unit (per unit index)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] index                    - unit index
*                                       None
*/
static void rxdmaInit
(
    IN  GT_U8 devNum,
    IN  GT_U32  index,
    IN  GT_U32  lastRxDmaIndex,
    IN CPSS_PORTS_BMP_STC *validRxDmaBmpPtr
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    {/*start of unit rxDMA */
        {/*start of unit singleChannelDMAConfigs */
            {/*304+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= lastRxDmaIndex ; n++) {
                    SKIP_NON_EXISTS_PORT_IN_BMP(validRxDmaBmpPtr,n);
                    regsAddrPtr->rxDMA[index].singleChannelDMAConfigs.SCDMAConfig0[n] =
                        0x304+n*0x4;
                }/* end of loop n */
            }/*304+n*0x4*/
            {/*450+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= lastRxDmaIndex ; n++) {
                    SKIP_NON_EXISTS_PORT_IN_BMP(validRxDmaBmpPtr,n);
                    regsAddrPtr->rxDMA[index].singleChannelDMAConfigs.SCDMAConfig1[n] =
                        0x450+n*0x4;
                }/* end of loop n */
            }/*450+n*0x4*/
            {/*600+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= lastRxDmaIndex ; n++) {
                    SKIP_NON_EXISTS_PORT_IN_BMP(validRxDmaBmpPtr,n);
                    regsAddrPtr->rxDMA[index].singleChannelDMAConfigs.SCDMACTPktIndentification[n] =
                        0x600+n*0x4;
                }/* end of loop n */
            }/*600+n*0x4*/

        }/*end of unit singleChannelDMAConfigs */


        {/*start of unit globalRxDMAConfigs */
            {/*start of unit rejectConfigs */
                regsAddrPtr->rxDMA[index].globalRxDMAConfigs.rejectConfigs.rejectPktCommand = 0x000000a0;
                regsAddrPtr->rxDMA[index].globalRxDMAConfigs.rejectConfigs.rejectCPUCode = 0x000000a4;

            }/*end of unit rejectConfigs */


            {/*start of unit globalConfigCutThrough */
                regsAddrPtr->rxDMA[index].globalRxDMAConfigs.globalConfigCutThrough.CTEtherTypeIdentification = 0x00000084;
                regsAddrPtr->rxDMA[index].globalRxDMAConfigs.globalConfigCutThrough.CTUPEnable = 0x00000088;
                if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
                {
                    regsAddrPtr->rxDMA[index].globalRxDMAConfigs.globalConfigCutThrough.
                        CTGeneralConfiguration = 0x0000008C;
                }

            }/*end of unit globalConfigCutThrough */


            {/*start of unit buffersLimitProfiles */
                GT_U32    n;
                regsAddrPtr->rxDMA[index].globalRxDMAConfigs.buffersLimitProfiles.portsBuffersLimitProfileXONXOFFConfig[0] = 0;
                regsAddrPtr->rxDMA[index].globalRxDMAConfigs.buffersLimitProfiles.portsBuffersLimitProfileRxlimitConfig[0] = 0x00000004;
                for(n = 1 ; n <= 7 ; n++) {
                    regsAddrPtr->rxDMA[index].globalRxDMAConfigs.buffersLimitProfiles.portsBuffersLimitProfileXONXOFFConfig[n] =
                        regsAddrPtr->rxDMA[index].globalRxDMAConfigs.buffersLimitProfiles.portsBuffersLimitProfileXONXOFFConfig[n - 1] + 8;
                    regsAddrPtr->rxDMA[index].globalRxDMAConfigs.buffersLimitProfiles.portsBuffersLimitProfileRxlimitConfig[n] =
                        regsAddrPtr->rxDMA[index].globalRxDMAConfigs.buffersLimitProfiles.portsBuffersLimitProfileRxlimitConfig[n - 1] + 8;
                }/* end of loop n */
            }/*end of unit buffersLimitProfiles */

        }/*end of unit globalRxDMAConfigs */

        { /* start of unit SCDMA debug */
            GT_U32    n;
            for(n = 0 ; n <= lastRxDmaIndex ; n++) {
                SKIP_NON_EXISTS_PORT_IN_BMP(validRxDmaBmpPtr,n);
                regsAddrPtr->rxDMA[index].scdmaDebug.portBuffAllocCnt[n] = 0x1900+n*0x4;
            }
        }
        {/*start of unit SIP_COMMON_MODULES */
            {/*start of unit pizzaArbiter */
                {/*2508+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 84 ; n++) {
                        regsAddrPtr->rxDMA[index].SIP_COMMON_MODULES.pizzaArbiter.pizzaArbiterConfigReg[n] =
                            0x2508+n*0x4;
                    }/* end of loop n */
                }/*2508+n*0x4*/
                regsAddrPtr->rxDMA[index].SIP_COMMON_MODULES.pizzaArbiter.pizzaArbiterCtrlReg = 0x00002500;
                regsAddrPtr->rxDMA[index].SIP_COMMON_MODULES.pizzaArbiter.pizzaArbiterStatusReg = 0x00002504;

            }/*end of unit pizzaArbiter */


        }/*end of unit SIP_COMMON_MODULES */

        {/*start of unit interrupts */
            {/*d0c+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= lastRxDmaIndex ; n++) {
                    SKIP_NON_EXISTS_PORT_IN_BMP(validRxDmaBmpPtr,n);
                    regsAddrPtr->rxDMA[index].interrupts.rxDMASCDMAInterruptMask[n] =
                        0xd0c+n*0x4;
                }/* end of loop n */
            }/*d0c+n*0x4*/
            {/*908+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= lastRxDmaIndex ; n++) {
                    SKIP_NON_EXISTS_PORT_IN_BMP(validRxDmaBmpPtr,n);
                    regsAddrPtr->rxDMA[index].interrupts.rxDMASCDMAInterruptCause[n] =
                        0x908+n*0x4;
                }/* end of loop n */
            }/*908+n*0x4*/
            /*
            regsAddrPtr->rxDMA[index].interrupts.rxDMAInterrupt0Cause = 0x00000900;
            regsAddrPtr->rxDMA[index].interrupts.rxDMAInterrupt0Mask = 0x00000904;
            regsAddrPtr->rxDMA[index].interrupts.rxDMAInterruptSummary0Cause = 0x00001028;
            regsAddrPtr->rxDMA[index].interrupts.rxDMAInterruptSummary0Mask = 0x0000102c;
            regsAddrPtr->rxDMA[index].interrupts.rxDMAInterruptSummary1Cause = 0x00001030;
            regsAddrPtr->rxDMA[index].interrupts.rxDMAInterruptSummary1Mask = 0x00001034;
            regsAddrPtr->rxDMA[index].interrupts.rxDMAInterruptSummary2Cause = 0x00001038;
            regsAddrPtr->rxDMA[index].interrupts.rxDMAInterruptSummary2Mask = 0x0000103c;
            regsAddrPtr->rxDMA[index].interrupts.rxDMAInterruptSummary3Cause = 0x00001040;
            regsAddrPtr->rxDMA[index].interrupts.rxDMAInterruptSummary3Mask = 0x00001044;
            regsAddrPtr->rxDMA[index].interrupts.rxDMAInterruptSummaryCause = 0x00001048;
            regsAddrPtr->rxDMA[index].interrupts.rxDMAInterruptSummaryMask = 0x0000104c;
            */

        }/*end of unit interrupts */

        { /* start of unit debug */

            regsAddrPtr->rxDMA[index].debug.countersAndCountersStatuses.currentNumOfPacketsStoredInMppm = 0x0000123C;

        }/*end of unit debug */

        if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
        {
            {/*start of unit SIP_COMMON_MODULES */
                {/*start of unit pizzaArbiter */
                    {/*2a08+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 84 ; n++) {
                            regsAddrPtr->rxDMA[index].SIP_COMMON_MODULES.pizzaArbiter.pizzaArbiterConfigReg[n] =
                                0x2a08+n*0x4;
                        }/* end of loop n */
                    }/*2a08+n*0x4*/
                    regsAddrPtr->rxDMA[index].SIP_COMMON_MODULES.pizzaArbiter.pizzaArbiterCtrlReg = 0x00002a00;
                    regsAddrPtr->rxDMA[index].SIP_COMMON_MODULES.pizzaArbiter.pizzaArbiterStatusReg = 0x00002a04;

                }/*end of unit pizzaArbiter */


            }/*end of unit SIP_COMMON_MODULES */

            {/*start of unit singleChannelDMAConfigs */
                {/*804+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= lastRxDmaIndex ; n++) {
                        SKIP_NON_EXISTS_PORT_IN_BMP(validRxDmaBmpPtr,n);
                        regsAddrPtr->rxDMA[index].singleChannelDMAConfigs.SCDMAConfig0[n] =
                            0x804+n*0x4;
                    }/* end of loop n */
                }/*804+n*0x4*/
                {/*950+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= lastRxDmaIndex ; n++) {
                        SKIP_NON_EXISTS_PORT_IN_BMP(validRxDmaBmpPtr,n);
                        regsAddrPtr->rxDMA[index].singleChannelDMAConfigs.SCDMAConfig1[n] =
                            0x950+n*0x4;
                    }/* end of loop n */
                }/*950+n*0x4*/
                {/*b00+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= lastRxDmaIndex ; n++) {
                        SKIP_NON_EXISTS_PORT_IN_BMP(validRxDmaBmpPtr,n);
                        regsAddrPtr->rxDMA[index].singleChannelDMAConfigs.SCDMACTPktIndentification[n] =
                            0xb00+n*0x4;
                    }/* end of loop n */
                }/*b00+n*0x4*/

            }/*end of unit singleChannelDMAConfigs */
            { /* start of unit SCDMA debug */
                GT_U32    n;
                for(n = 0 ; n <= lastRxDmaIndex ; n++) {
                    SKIP_NON_EXISTS_PORT_IN_BMP(validRxDmaBmpPtr,n);
                    regsAddrPtr->rxDMA[index].scdmaDebug.portBuffAllocCnt[n] = 0x1e00+n*0x4;
                }
            }
            {/*start of unit interrupts */
                {/*120c+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= lastRxDmaIndex ; n++) {
                        SKIP_NON_EXISTS_PORT_IN_BMP(validRxDmaBmpPtr,n);
                        regsAddrPtr->rxDMA[index].interrupts.rxDMASCDMAInterruptMask[n] =
                            0x120c+n*0x4;
                    }/* end of loop n */
                }/*120c+n*0x4*/
                {/*e08+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= lastRxDmaIndex ; n++) {
                        SKIP_NON_EXISTS_PORT_IN_BMP(validRxDmaBmpPtr,n);
                        regsAddrPtr->rxDMA[index].interrupts.rxDMASCDMAInterruptCause[n] =
                            0xe08+n*0x4;
                    }/* end of loop n */
                }/*e08+n*0x4*/
                /*
                regsAddrPtr->rxDMA[index].interrupts.rxDMAInterrupt0Cause = 0x00000e00;
                regsAddrPtr->rxDMA[index].interrupts.rxDMAInterrupt0Mask = 0x00000e04;
                regsAddrPtr->rxDMA[index].interrupts.rxDMAInterruptSummary0Cause = 0x00001528;
                regsAddrPtr->rxDMA[index].interrupts.rxDMAInterruptSummary0Mask = 0x0000152c;
                regsAddrPtr->rxDMA[index].interrupts.rxDMAInterruptSummary1Cause = 0x00001530;
                regsAddrPtr->rxDMA[index].interrupts.rxDMAInterruptSummary1Mask = 0x00001534;
                regsAddrPtr->rxDMA[index].interrupts.rxDMAInterruptSummary2Cause = 0x00001538;
                regsAddrPtr->rxDMA[index].interrupts.rxDMAInterruptSummary2Mask = 0x0000153c;
                regsAddrPtr->rxDMA[index].interrupts.rxDMAInterruptSummary3Cause = 0x00001540;
                regsAddrPtr->rxDMA[index].interrupts.rxDMAInterruptSummary3Mask = 0x00001544;
                regsAddrPtr->rxDMA[index].interrupts.rxDMAInterruptSummaryCause = 0x00001548;
                regsAddrPtr->rxDMA[index].interrupts.rxDMAInterruptSummaryMask = 0x0000154c;
                */

            }/*end of unit interrupts */
            { /* start of unit debug */

                regsAddrPtr->rxDMA[index].debug.countersAndCountersStatuses.currentNumOfPacketsStoredInMppm = 0x0000173C;

            }/*end of unit debug */
        }

        if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
        {/*start of unit preIngrPrioritizationConfStatus */
            {/*000000c4+n * 0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.VLANUPPriorityArrayProfile[n] =
                        0x000000c4+n * 0x4;
                }/* end of loop n */
            }/*000000c4+n * 0x4*/
            {/*000003a8+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 1 ; n++) {
                    regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.MPLSEtherTypeConf[n] =
                        0x000003a8+n*0x4;
                }/* end of loop n */
            }/*000003a8+n*0x4*/
            {/*00000450+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.VIDConf[n] =
                        0x00000450+n*0x4;
                }/* end of loop n */
            }/*00000450+n*0x4*/
            {/*000003c8+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.genericEtherType[n] =
                        0x000003c8+n*0x4;
                }/* end of loop n */
            }/*000003c8+n*0x4*/
            {/*000000e4+m*0x4 + n*0x20*/
                GT_U32    n,m;
                for(n = 0 ; n <= 3 ; n++) {
                    for(m = 0 ; m <= 7 ; m++) {
                        regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.qoSDSAPriorityArrayProfileSegment[n][m] =
                            0x000000e4+m*0x4 + n*0x20;
                    }/* end of loop m */
                }/* end of loop n */
            }/*000000e4+m*0x4 + n*0x20*/
            regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.PIPPriorityDropGlobalCntrsClear = 0x00000740;
            regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.PIPMedPriorityDropGlobalCntrMsb = 0x00000724;
            regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.PIPMedPriorityDropGlobalCntrLsb = 0x00000720;
            regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.PIPLowPriorityDropGlobalCntrMsb = 0x00000714;
            if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
            {
                regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.PIPVeryHighPriorityDropGlobalCntrLsb = 0x00000760;
                regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.PIPVeryHighPriorityDropGlobalCntrMsb = 0x00000764;
            }
            else
            {
                regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.PIPPrioThresholds = 0x000000b0;
                regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.preingressPrioritizationEnable = 0x000000b4;
            }

            {/*00000398+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.VLANEtherTypeConf[n] =
                        0x00000398+n*0x4;
                }/* end of loop n */
            }/*00000398+n*0x4*/
            {/*00000408+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.MACDA2MsbBytesConf[n] =
                        0x00000408+n*0x4;
                }/* end of loop n */
            }/*00000408+n*0x4*/
            regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.PIPHighPriorityDropGlobalCntrLsb = 0x00000730;
            {/*00000174+n * 0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.MPLSEXPPriorityArrayProfile[n] =
                        0x00000174+n * 0x4;
                }/* end of loop n */
            }/*00000174+n * 0x4*/
            {/*00000630+n * 0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.UPDSATagArrayProfile[n] =
                        0x00000630+n * 0x4;
                }/* end of loop n */
            }/*00000630+n * 0x4*/
            {/*000002f0+n * 0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.MACDAPriorityArrayProfile[n] =
                        0x000002f0+n * 0x4;
                }/* end of loop n */
            }/*000002f0+n * 0x4*/
            {/*00000428+n *0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.MACDAMask4LsbConf[n] =
                        0x00000428+n *0x4;
                }/* end of loop n */
            }/*00000428+n *0x4*/
            {/*0000043c+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.MACDAMask2MsbConf[n] =
                        0x0000043c+n*0x4;
                }/* end of loop n */
            }/*0000043c+n*0x4*/
            {/*000003e8+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.MACDA4LsbBytesConf[n] =
                        0x000003e8+n*0x4;
                }/* end of loop n */
            }/*000003e8+n*0x4*/
            regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.PIPLowPriorityDropGlobalCntrLSb = 0x00000710;
            {/*000004d0+m*0x4 + n*0x40*/
                GT_U32    n,m;
                for(n = 0 ; n <= 3 ; n++) {
                    for(m = 0 ; m <= 15 ; m++) {
                        regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.IPv6TCPriorityArrayProfileSegment[n][m] =
                            0x000004d0+m*0x4 + n*0x40;
                    }/* end of loop m */
                }/* end of loop n */
            }/*000004d0+m*0x4 + n*0x40*/
            regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.IPv6EtherTypeConf = 0x00000374;
            {/*00000198+m*0x4 + n*0x40*/
                GT_U32    n,m;
                for(n = 0 ; n <= 3 ; n++) {
                    for(m = 0 ; m <= 15 ; m++) {
                        regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.IPv4ToSPriorityArrayProfileSegment[n][m] =
                            0x00000198+m*0x4 + n*0x40;
                    }/* end of loop m */
                }/* end of loop n */
            }/*00000198+m*0x4 + n*0x40*/
            regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.IPv4EtherTypeConf = 0x00000350;
            regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.PIPHighPriorityDropGlobalCntrMsb = 0x00000734;
            if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
            {
                regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.PIPDropGlobalCounterTrigger  = 0x00000738;
            }
            {/*00000320+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.etherTypePriorityArrayProfile[n] =
                        0x00000320+n*0x4;
                }/* end of loop n */
            }/*00000320+n*0x4*/

        }/*end of unit preIngrPrioritizationConfStatus */

        if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
        {
            /* manually added : rxDMA.singleChannelDMAPip */
            {/*3A00+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= lastRxDmaIndex ; n++) {
                    SKIP_NON_EXISTS_PORT_IN_BMP(validRxDmaBmpPtr,n);
                    regsAddrPtr->rxDMA[index].singleChannelDMAPip.SCDMA_n_PIP_Config[n] =
                        0x3A00+n*0x4;
                }/* end of loop n */
            }/*3A00+n*0x4*/

            {/*3500+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= lastRxDmaIndex ; n++) {
                    SKIP_NON_EXISTS_PORT_IN_BMP(validRxDmaBmpPtr,n);
                    regsAddrPtr->rxDMA[index].singleChannelDMAPip.SCDMA_n_Drop_Pkt_Counter[n] =
                        0x3500+n*0x4;
                }/* end of loop n */
            }/*3500+n*0x4*/
        }

    }/*end of unit rxDMA */
}

/**
* @internal ingressAggregatorInit function
* @endinternal
*
* @brief   Init the DB - Ingress Aggregator unit (per pipe index)
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None
*/
static void ingressAggregatorInit
(
    IN  GT_U8   devNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_U32  numOfSupportedUnits;
    GT_U32 ii;
    GT_U32  unitAddr;
   if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
    {
        numOfSupportedUnits = 2;/* 1 per 3 RxDMA units */
    }
    else
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
    {
        numOfSupportedUnits = 2;/* 1 per 4 RxDMA units */
    }
    else
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
    {
        numOfSupportedUnits = 1;/* 1 per 4 RxDMA units */
    }
    else
    {
        /* no 'PRV_CPSS_DXCH_UNIT_IA_E' unit */
        return ;
    }

    for(ii = 0; ii < numOfSupportedUnits; ii++)
    {
        regsAddrPtr->ingressAggregator[ii].PIPPrioThresholds0          = 0x0000;
        regsAddrPtr->ingressAggregator[ii].PIPPrioThresholds1          = 0x0004;
        regsAddrPtr->ingressAggregator[ii].PIPEnable                   = 0x0008;
     }

    unitAddr = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_IA_E,NULL);
    regUnitBaseAddrSet(REG_UNIT_INFO_MAC(
        devNum,unitAddr,ingressAggregator[0]));

    if(numOfSupportedUnits >= 2)
    {
        unitAddr = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_IA_1_E,NULL);
        regUnitBaseAddrSet(REG_UNIT_INFO_MAC(
            devNum,unitAddr,ingressAggregator[1]));
    }

}

/**
* @internal rxdmaUpdateUnit0AsGlobalPortsAddr function
* @endinternal
*
* @brief   update the DB - RXDMA unit - after multi units set 'per port' addresses.
*         the DB of RXDMA[0] updated for the ports with 'global port index'
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] currentUnitIndex         - the index of current RXDMA[] unit
* @param[in] portIn_unit_0            - the 'global port' index (in unit 0)
* @param[in] portInCurrentUnit        - the 'local port'  index (in current unit)
* @param[in] numOfPorts               - number of ports to update.
* @param[in] isFirstUpdate            - indication of first update
* @param[in] isLastUpdate             - indication of last update
*                                       None
*/
static void rxdmaUpdateUnit0AsGlobalPortsAddr
(
    IN  GT_U8   devNum,
    IN  GT_U32  currentUnitIndex,
    IN  GT_U32  portIn_unit_0,
    IN  GT_U32  portInCurrentUnit,
    IN  GT_U32  numOfPorts,
    IN  GT_BOOL isFirstUpdate,
    IN  GT_BOOL isLastUpdate
)
{
    GT_U32    saveLastNumOfPorts;/*number of ports from the end of the unitStart1Ptr[]
                                  that need to be saved before overridden by unitStart2Ptr[]*/
    GT_U32    saveToIndex;/*the index in unitStart1Ptr[] to save the 'last ports'
                                  valid when saveLastNumOfPorts != 0*/
    const GT_U32 *localDmaMapArr; /* local DMAs in specific DP, NULL in cases 1:1 mapping */

    localDmaMapArr = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[currentUnitIndex].localDmaMapArr;

    if(isLastUpdate == GT_TRUE &&
       PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.supportRelativePortNum)
    {
        /* do not let the DB be overridden with the CPU port of last DP */
        /* because we saved the CPU port of the first DP */
        numOfPorts -=
            (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[0].cpuPortDmaNum ? 1 : 0);
    }

    if(isFirstUpdate == GT_TRUE &&
       PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.supportRelativePortNum)
    {
        /* update the DB with the CPU port address on the lowest existing DP */
        /* save CPU port */
        saveLastNumOfPorts =
            (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[0].cpuPortDmaNum ? 1 : 0);
        saveToIndex = CPU_PORT_DMA_CNS(devNum);
    }
    else
    {
        if((localDmaMapArr != NULL) && (isLastUpdate == GT_TRUE))
        {
            /* move CPU port registers addresses to position 72 */
            saveLastNumOfPorts = 1;
            saveToIndex = CPU_PORT_DMA_CNS(devNum);
        }
        else
        {
            saveLastNumOfPorts = 0;
            saveToIndex = 0;
        }
    }
    /************************************/
    /*   update the per port addresses  */
    /*   of the rxdma unit 0 with those */
    /*   of unit 1                      */
    /************************************/
    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* rxDMA */
        regUnitPerPortAddrUpdate(
            REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, rxDMA[0] , rxDMA[currentUnitIndex] , globalRxDMAConfigs.preIngrPrioritizationConfStatus.channelPIPConfigReg ,
                portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex,localDmaMapArr)
            );
        /* sip6_rxDMA */
        regUnitPerPortAddrUpdate(
            REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, sip6_rxDMA[0] , sip6_rxDMA[currentUnitIndex] , configs.cutThrough.channelCTConfig ,
                portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex,localDmaMapArr)
            );
        regUnitPerPortAddrUpdate(
            REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, sip6_rxDMA[0] , sip6_rxDMA[currentUnitIndex] , configs.channelConfig.channelToLocalDevSourcePort ,
                portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex,localDmaMapArr)
            );
        regUnitPerPortAddrUpdate(
            REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, sip6_rxDMA[0] , sip6_rxDMA[currentUnitIndex] , configs.channelConfig.PCHConfig ,
                portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex,localDmaMapArr)
            );
        regUnitPerPortAddrUpdate(
            REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, sip6_rxDMA[0] , sip6_rxDMA[currentUnitIndex] , configs.channelConfig.channelGeneralConfigs ,
                portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex,localDmaMapArr)
            );
        regUnitPerPortAddrUpdate(
            REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, sip6_rxDMA[0] , sip6_rxDMA[currentUnitIndex] , configs.channelConfig.channelCascadePort ,
                portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex,localDmaMapArr)
            );
    }
    else
    {
        regUnitPerPortAddrUpdate(
            REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, rxDMA[0] , rxDMA[currentUnitIndex] , singleChannelDMAConfigs.SCDMAConfig0 ,
                portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex,localDmaMapArr)
            );
        regUnitPerPortAddrUpdate(
            REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, rxDMA[0] , rxDMA[currentUnitIndex] , singleChannelDMAConfigs.SCDMAConfig1,
                portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex,localDmaMapArr)
            );
        regUnitPerPortAddrUpdate(
            REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, rxDMA[0] , rxDMA[currentUnitIndex] , singleChannelDMAConfigs.SCDMACTPktIndentification,
                portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex,localDmaMapArr)
            );

        regUnitPerPortAddrUpdate(
            REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, rxDMA[0] , rxDMA[currentUnitIndex] , scdmaDebug.portBuffAllocCnt,
                portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex,localDmaMapArr)
            );

        regUnitPerPortAddrUpdate(
            REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, rxDMA[0] , rxDMA[currentUnitIndex] , interrupts.rxDMASCDMAInterruptMask,
                portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex,localDmaMapArr)
            );

        regUnitPerPortAddrUpdate(
            REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, rxDMA[0] , rxDMA[currentUnitIndex] , interrupts.rxDMASCDMAInterruptCause,
                portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex,localDmaMapArr)
            );

        regUnitPerPortAddrUpdate(
            REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, rxDMA[0] , rxDMA[currentUnitIndex] , singleChannelDMAPip.SCDMA_n_PIP_Config,
                portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex,localDmaMapArr)
            );

        regUnitPerPortAddrUpdate(
            REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, rxDMA[0] , rxDMA[currentUnitIndex] , singleChannelDMAPip.SCDMA_n_Drop_Pkt_Counter,
                portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex,localDmaMapArr)
            );
    }

}

/**
* @internal txdmaInit function
* @endinternal
*
* @brief   init the DB - TXDMA unit (per unit index)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] index                    - unit index
*                                       None
*/
static void txdmaInit
(
    IN  GT_U8 devNum,
    IN  GT_U32  index,
    IN  GT_U32  lastTxDmaIndex,
    IN CPSS_PORTS_BMP_STC *validTxDmaBmpPtr
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    {/*start of unit txDMA */
        {/*start of unit txDMAThresholdsConfigs */
            {/*00001020+r*0x4*/
                GT_U32    r;
                for(r = 0 ; r <= 0 ; r++) {
                    regsAddrPtr->txDMA[index].txDMAThresholdsConfigs.RXDMAUpdatesFIFOsThresholds[r] =
                        0x00001020+r*0x4;
                }/* end of loop r */
            }/*00001020+r*0x4*/
            regsAddrPtr->txDMA[index].txDMAThresholdsConfigs.clearFIFOThreshold = 0x00001000;
            regsAddrPtr->txDMA[index].txDMAThresholdsConfigs.headerReorderFifoThreshold = 0x0000100C;
            regsAddrPtr->txDMA[index].txDMAThresholdsConfigs.payloadReorderFifoThreshold = 0x00001010;

        }/*end of unit txDMAThresholdsConfigs */


        {/*start of unit txDMAPerSCDMAConfigs */
            {/*0000300c+0x20*p*/
                GT_U32    p;
                for(p = 0 ; p <= lastTxDmaIndex ; p++) {
                    SKIP_NON_EXISTS_PORT_IN_BMP(validTxDmaBmpPtr,p);
                    regsAddrPtr->txDMA[index].txDMAPerSCDMAConfigs.burstLimiterSCDMA[p] =
                        0x0000300c+0x20*p;
                }/* end of loop p */
            }/*0000300c+0x20*p*/
            {/*00003000+p*0x20*/
                GT_U32    p;
                for(p = 0 ; p <= lastTxDmaIndex ; p++) {
                    SKIP_NON_EXISTS_PORT_IN_BMP(validTxDmaBmpPtr,p);
                    regsAddrPtr->txDMA[index].txDMAPerSCDMAConfigs.FIFOsThresholdsConfigsSCDMAReg1[p] =
                        0x00003000+p*0x20;
                }/* end of loop p */
            }/*00003000+p*0x20*/
            {/*00003008+0x20*p*/
                GT_U32    p;
                for(p = 0 ; p <= lastTxDmaIndex ; p++) {
                    SKIP_NON_EXISTS_PORT_IN_BMP(validTxDmaBmpPtr,p);
                    regsAddrPtr->txDMA[index].txDMAPerSCDMAConfigs.txFIFOCntrsConfigsSCDMA[p] =
                        0x00003008+0x20*p;
                }/* end of loop p */
            }/*00003008+0x20*p*/
            {/*00003010+p*0x20*/
                GT_U32    p;
                for(p = 0 ; p <= lastTxDmaIndex ; p++) {
                    SKIP_NON_EXISTS_PORT_IN_BMP(validTxDmaBmpPtr,p);
                    regsAddrPtr->txDMA[index].txDMAPerSCDMAConfigs.SCDMAConfigs[p] =
                        0x00003010+p*0x20;
                }/* end of loop p */
            }/*00003010+p*0x20*/
            {/*00003014+p*0x20*/
                GT_U32    p;
                for(p = 0 ; p <= lastTxDmaIndex ; p++) {
                    SKIP_NON_EXISTS_PORT_IN_BMP(validTxDmaBmpPtr,p);
                    regsAddrPtr->txDMA[index].txDMAPerSCDMAConfigs.SCDMAMetalFixReg[p] =
                        0x00003014+p*0x20;
                }/* end of loop p */
            }/*00003014+p*0x20*/
        }/*end of unit txDMAPerSCDMAConfigs */


        {/*start of unit txDMAInterrupts */
            /*
            regsAddrPtr->txDMA[index].txDMAInterrupts.SCDMAIDFIFOOverrunInterruptCauseReg1 = 0x00002000;
            regsAddrPtr->txDMA[index].txDMAInterrupts.SCDMAIDFIFOOverrunInterruptMaskReg1 = 0x00002004;
            regsAddrPtr->txDMA[index].txDMAInterrupts.SCDMAIDFIFOOverrunInterruptCauseReg2 = 0x00002008;
            regsAddrPtr->txDMA[index].txDMAInterrupts.SCDMAIDFIFOOverrunInterruptMaskReg2 = 0x0000200c;
            regsAddrPtr->txDMA[index].txDMAInterrupts.SCDMAWaitingForNextReplyInterruptMaskReg3 = 0x00002034;
            regsAddrPtr->txDMA[index].txDMAInterrupts.SCDMAIDFIFOOverrunInterruptMaskReg3 = 0x00002014;
            regsAddrPtr->txDMA[index].txDMAInterrupts.RXDMAUpdatesFIFOsOverrunInterruptCauseReg1 = 0x00002018;
            regsAddrPtr->txDMA[index].txDMAInterrupts.RXDMAUpdatesFIFOsOverrunInterruptMaskReg1 = 0x0000201c;
            regsAddrPtr->txDMA[index].txDMAInterrupts.SCDMAWaitingForNextReplyInterruptCauseReg1 = 0x00002020;
            regsAddrPtr->txDMA[index].txDMAInterrupts.SCDMAWaitingForNextReplyInterruptMaskReg1 = 0x00002024;
            regsAddrPtr->txDMA[index].txDMAInterrupts.SCDMAWaitingForNextReplyInterruptCauseReg2 = 0x00002028;
            regsAddrPtr->txDMA[index].txDMAInterrupts.SCDMAWaitingForNextReplyInterruptMaskReg2 = 0x0000202c;
            regsAddrPtr->txDMA[index].txDMAInterrupts.SCDMAWaitingForNextReplyInterruptCauseReg3 = 0x00002030;
            regsAddrPtr->txDMA[index].txDMAInterrupts.SCDMAIDFIFOOverrunInterruptCauseReg3 = 0x00002010;
            regsAddrPtr->txDMA[index].txDMAInterrupts.txDMAGeneralCauseReg1 = 0x00002038;
            regsAddrPtr->txDMA[index].txDMAInterrupts.txDMAGeneralMaskReg1 = 0x0000203c;
            regsAddrPtr->txDMA[index].txDMAInterrupts.txDMASummaryCauseReg = 0x00002040;
            regsAddrPtr->txDMA[index].txDMAInterrupts.txDMASummaryMaskReg = 0x00002044;
            */

        }/*end of unit txDMAInterrupts */


        {/*start of unit txDMAGlobalConfigs */
            regsAddrPtr->txDMA[index].txDMAGlobalConfigs.txDMAGlobalConfig1      = 0x00000000;
            regsAddrPtr->txDMA[index].txDMAGlobalConfigs.txDMAGlobalConfig3      = 0x00000008;
            regsAddrPtr->txDMA[index].txDMAGlobalConfigs.ingrCoreIDToMPPMMap     = 0x0000000c;
            regsAddrPtr->txDMA[index].txDMAGlobalConfigs.TXDMAMetalFixReg        = 0x00000010;
            if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)) /* BC3 and higher */
            {
                GT_U32 i;
                for (i = 0; i < 10 ; i++)
                {
                    regsAddrPtr->txDMA[index].txDMAGlobalConfigs.srcSCDMASpeedTbl[i] = 0x000000220 + i*0x4;
                }
                for (i = 0 ; i < 128; i++)
                {
                    regsAddrPtr->txDMA[index].txDMAGlobalConfigs.localDevSrcPort2DMANumberMap[i] = 0x000000020 + i*0x4;
                }
            }
        }/*end of unit txDMAGlobalConfigs */


        {/*start of unit txDMADescCriticalFieldsECCConfigs */
            regsAddrPtr->txDMA[index].txDMADescCriticalFieldsECCConfigs.dataECCConfig = 0x00000500;
            regsAddrPtr->txDMA[index].txDMADescCriticalFieldsECCConfigs.dataECCStatus = 0x00000504;

        }/*end of unit txDMADescCriticalFieldsECCConfigs */


        {/*start of unit txDMADebug */
            {/*start of unit informativeDebug */
                {/*00005100+c*0x4*/
                    GT_U32    c;
                    for(c = 0 ; c <= 0 ; c++) {
                        regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.nextBCFIFOFillLevel[c] =
                            0x00005100+c*0x4;
                    }/* end of loop c */
                }/*00005100+c*0x4*/
                {/*00005120+c*0x4*/
                    GT_U32    c;
                    for(c = 0 ; c <= 0 ; c++) {
                        regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.nextBCFIFOMaxFillLevel[c] =
                            0x00005120+c*0x4;
                    }/* end of loop c */
                }/*00005120+c*0x4*/
                {/*00005200+p*0x4*/
                    GT_U32    p;
                    for(p = 0 ; p <= lastTxDmaIndex ; p++) {
                        SKIP_NON_EXISTS_PORT_IN_BMP(validTxDmaBmpPtr,p);
                        regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.SCDMAStatusReg1[p] =
                            0x00005200+p*0x4;
                    }/* end of loop p */
                }/*00005200+p*0x4*/
                {/*00005500+p*0x4*/
                    GT_U32    p;
                    for(p = 0 ; p <= lastTxDmaIndex ; p++) {
                        SKIP_NON_EXISTS_PORT_IN_BMP(validTxDmaBmpPtr,p);
                        regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.SCDMAStatusReg2[p] =
                            0x00005500+p*0x4;
                    }/* end of loop p */
                }/*00005500+p*0x4*/
                {/*00005700+p*0x4*/
                    GT_U32    p;
                    for(p = 0 ; p <= lastTxDmaIndex ; p++) {
                        SKIP_NON_EXISTS_PORT_IN_BMP(validTxDmaBmpPtr,p);
                        regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.SCDMAPrefFIFOFillLevel[p] =
                            0x00005700+p*0x4;
                    }/* end of loop p */
                }/*00005700+p*0x4*/
                {/*00005900+p*0x4*/
                    GT_U32    p;
                    for(p = 0 ; p <= lastTxDmaIndex ; p++) {
                        SKIP_NON_EXISTS_PORT_IN_BMP(validTxDmaBmpPtr,p);
                        regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.SCDMAPrefFIFOMaxFillLevel[p] =
                            0x00005900+p*0x4;
                    }/* end of loop p */
                }/*00005900+p*0x4*/
                regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.descOverrunReg1 = 0x00005000;
                regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.descOverrunReg2 = 0x00005004;
                regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.descOverrunReg3 = 0x00005008;
                regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.clearFIFOFillLevel = 0x0000500c;
                regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.clearFIFOMaxFillLevel = 0x00005010;
                regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.headerReorderFIFOFillLevel = 0x00005014;
                regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.headerReorderFIFOMaxFillLevel = 0x0000501c;
                regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.payloadReorderFIFOFillLevel = 0x00005020;
                regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.payloadReorderFIFOMaxFillLevel = 0x00005024;
                regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.descIDFIFOFillLevel = 0x00005028;
                regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.descIDFIFOMaxFillLevel = 0x0000502c;
                regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.debugBusCtrlReg = 0x00005030;
                regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.txDMADebugBus = 0x00005034;
                regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.incomingDescsCntrLsb = 0x00005038;
                regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.outgoingDescsCntrLsb = 0x0000503c;
                regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.outgoingClearRequestsCntrLsb = 0x00005040;
                regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.MPPMReadRequestsCntr = 0x00005044;
                regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.MPPMReadAnswersCntrLsb = 0x00005048;
                regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.outgoingPayloadsCntrLsb = 0x0000504c;
                regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.outgoingPayloadsEOPCntrLsb = 0x00005050;
                regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.outgoingHeadersEOPCntrLsb = 0x00005054;
                regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.outgoingCTBCUpdatesCntr = 0x00005058;
                regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.HADescIDDBFreeIDFIFOFillLevel = 0x0000505c;
                regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.internalDescIDDBFreeIDFIFOFillLevel = 0x00005060;
                regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.headerReorderMemIDDBFreeIDFIFOFillLevel = 0x00005070;
                regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.payloadReorderMemIDDBFreeIDFIFOFillLevel = 0x00005074;

            }/*end of unit informativeDebug */


        }/*end of unit txDMADebug */


        {/*start of unit txDMAPizzaArb */
            {/*start of unit pizzaArbiter */
                {/*00004008+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 84 ; n++) {
                        regsAddrPtr->txDMA[index].txDMAPizzaArb.pizzaArbiter.pizzaArbiterConfigReg[n] =
                            0x00004008+n*0x4;
                    }/* end of loop n */
                }/*00004008+n*0x4*/
                regsAddrPtr->txDMA[index].txDMAPizzaArb.pizzaArbiter.pizzaArbiterCtrlReg = 0x00004000;
                regsAddrPtr->txDMA[index].txDMAPizzaArb.pizzaArbiter.pizzaArbiterStatusReg = 0x00004004;

            }/*end of unit pizzaArbiter */


        }/*end of unit txDMAPizzaArb */

        if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
        {
            regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.incomingDescsCntrLsb = 0x00005038;
            regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.incomingDescsCntrMsb = 0x0000503c;
            regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.outgoingDescsCntrLsb = 0x00005040;
            regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.outgoingDescsCntrMsb = 0x00005044;
            regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.outgoingClearRequestsCntrLsb = 0x00005048;
            regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.outgoingClearRequestsCntrMsb = 0x0000504c;
            regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.MPPMReadRequestsCntr = 0x00005050;
            regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.MPPMReadAnswersCntrLsb = 0x00005054;
            regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.MPPMReadAnswersCntrMsb = 0x00005058;
            regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.outgoingPayloadsCntrLsb = 0x0000505c;
            regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.outgoingPayloadsCntrMsb = 0x00005060;
            regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.outgoingPayloadsEOPCntrLsb = 0x00005064;
            regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.outgoingPayloadsEOPCntrMsb = 0x00005068;
            regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.outgoingHeadersEOPCntrLsb = 0x0000506c;
            regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.outgoingHeadersEOPCntrMsb = 0x00005070;
            regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.outgoingCTBCUpdatesCntr = 0x00005074;
            regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.internalDescIDDBFreeIDFIFOFillLevel = 0x00005080;
            regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.headerReorderMemIDDBFreeIDFIFOFillLevel = 0x00005084;
            regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.payloadReorderMemIDDBFreeIDFIFOFillLevel = 0x00005088;
            {/*0000300c+0x20*p*/
                GT_U32    p;
                for(p = 0 ; p <= lastTxDmaIndex ; p++) {
                    SKIP_NON_EXISTS_PORT_IN_BMP(validTxDmaBmpPtr,p);
                    regsAddrPtr->txDMA[index].txDMAPerSCDMAConfigs.txFIFOCntrsConfigsSCDMA[p] =
                        0x0000300c+0x20*p;
                }/* end of loop p */
            }/*0000300c+0x20*p*/

            {/*00003018+0x20*p*/
                GT_U32    p;
                for(p = 0 ; p <= lastTxDmaIndex ; p++) {
                    SKIP_NON_EXISTS_PORT_IN_BMP(validTxDmaBmpPtr,p);
                    regsAddrPtr->txDMA[index].txDMAPerSCDMAConfigs.burstLimiterSCDMA[p] =
                        0x00003018+0x20*p;
                }/* end of loop p */
            }/*00003018+0x20*p*/

        }

        if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
        {
            {/*0000301C+p*0x20*/
                GT_U32    p;
                for(p = 0 ; p <= lastTxDmaIndex ; p++) {
                    SKIP_NON_EXISTS_PORT_IN_BMP(validTxDmaBmpPtr,p);
                    regsAddrPtr->txDMA[index].txDMAPerSCDMAConfigs.SCDMAConfigs2[p] = 0x0000301C+p*0x20;
                }/* end of loop p */
            }/*0000301C+p*0x20*/
        }
        if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
        {
            {/*0000301C+p*0x20*/
                GT_U32    p;
                for(p = 0 ; p <= lastTxDmaIndex ; p++) {
                    SKIP_NON_EXISTS_PORT_IN_BMP(validTxDmaBmpPtr,p);
                    regsAddrPtr->txDMA[index].txDMAPerSCDMAConfigs.SCDMAConfigs1                 [p] = 0x00003010+p*0x20;
                    regsAddrPtr->txDMA[index].txDMAPerSCDMAConfigs.SCDMARateLimitResidueVectorThr[p] = 0x00003A00+p*0x4;
                }/* end of loop p */
            }/*0000301C+p*0x20*/
        }

    }/*end of unit txDMA */
}

/**
* @internal txdmaUpdateUnit0AsGlobalPortsAddr function
* @endinternal
*
* @brief   update the DB - TXDMA unit - after multi units set 'per port' addresses.
*         the DB of TXDMA[0] updated for the ports with 'global port index'
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] currentUnitIndex         - the index of current TXDMA[] unit
* @param[in] portIn_unit_0            - the 'global port' index (in unit 0)
* @param[in] portInCurrentUnit        - the 'local port'  index (in current unit)
* @param[in] numOfPorts               - number of ports to update.
* @param[in] isFirstUpdate            - indication of first update
* @param[in] isLastUpdate             - indication of last update
*                                       None
*/
static void txdmaUpdateUnit0AsGlobalPortsAddr
(
    IN  GT_U8 devNum,
    IN  GT_U32  currentUnitIndex,
    IN  GT_U32  portIn_unit_0,
    IN  GT_U32  portInCurrentUnit,
    IN  GT_U32  numOfPorts,
    IN  GT_BOOL isFirstUpdate,
    IN  GT_BOOL isLastUpdate
)
{
    GT_U32    saveLastNumOfPorts;/*number of ports from the end of the unitStart1Ptr[]
                                 that need to be saved before overridden by unitStart2Ptr[]*/
    GT_U32    saveToIndex;/*the index in unitStart1Ptr[] to save the 'last ports'
                                  valid when saveLastNumOfPorts != 0*/
    const GT_U32 *localDmaMapArr; /* local DMAs in specific DP, NULL in cases 1:1 mapping */

    localDmaMapArr = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[currentUnitIndex].localDmaMapArr;


    if(isLastUpdate == GT_TRUE &&
       PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.supportRelativePortNum)
    {
        /* do not let the DB be overridden with the CPU port of last DP */
        /* because we saved the CPU port of the first DP */
        numOfPorts -=
            (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[0].cpuPortDmaNum ? 1 : 0)+
            (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[0].tmDmaNum ? 1 : 0);
    }

    if(isFirstUpdate == GT_TRUE &&
       PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.supportRelativePortNum)
    {
        /* update the DB with the CPU port address on the lowest existing DP */
        /* save CPU port */
        saveLastNumOfPorts =
            (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[0].cpuPortDmaNum ? 1 : 0)+
            (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[0].tmDmaNum ? 1 : 0);

        saveToIndex = CPU_PORT_DMA_CNS(devNum);
    }
    else
    {
        if((localDmaMapArr != NULL) && (isLastUpdate == GT_TRUE))
        {
            /* move CPU port registers addresses to position 72 */
            saveLastNumOfPorts = 1;
            saveToIndex = CPU_PORT_DMA_CNS(devNum);
        }
        else
        {
            saveLastNumOfPorts = 0;
            saveToIndex = 0;
        }
    }
    /************************************/
    /*   update the per port addresses  */
    /*   of the txdma unit 0 with those */
    /*   of unit 1                      */
    /************************************/
    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
           /* sip6_txDMA */
        regUnitPerPortAddrUpdate(
            REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, sip6_txDMA[0] , sip6_txDMA[currentUnitIndex] , configs.channelConfigs.speedProfile,
                portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex,localDmaMapArr)
            );
        regUnitPerPortAddrUpdate(
            REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, sip6_txDMA[0] , sip6_txDMA[currentUnitIndex] , configs.channelConfigs.channelReset,
                portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex,localDmaMapArr)
            );
        regUnitPerPortAddrUpdate(
            REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, sip6_txDMA[0] , sip6_txDMA[currentUnitIndex] , configs.channelConfigs.descFIFOBase,
                portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex,localDmaMapArr)
            );
        regUnitPerPortAddrUpdate(
            REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, sip6_txDMA[0] , sip6_txDMA[currentUnitIndex] , configs.channelConfigs.interPacketRateLimiterConfig,
                portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex,localDmaMapArr)
            );
        regUnitPerPortAddrUpdate(
            REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, sip6_txDMA[0] , sip6_txDMA[currentUnitIndex] , configs.channelConfigs.interCellRateLimiterConfig,
                portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex,localDmaMapArr)
            );
        regUnitPerPortAddrUpdate(
            REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, sip6_txDMA[0] , sip6_txDMA[currentUnitIndex] , configs.channelConfigs.inerPacketRateLimiterConfig,
                portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex,localDmaMapArr)
            );
    }
    else
    {

        regUnitPerPortAddrUpdate(
            REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, txDMA[0] , txDMA[currentUnitIndex] , txDMAPerSCDMAConfigs.burstLimiterSCDMA,
                portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex,localDmaMapArr)
            );

        regUnitPerPortAddrUpdate(
            REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, txDMA[0] , txDMA[currentUnitIndex] , txDMAPerSCDMAConfigs.FIFOsThresholdsConfigsSCDMAReg1,
                portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex,localDmaMapArr)
            );

        regUnitPerPortAddrUpdate(
            REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, txDMA[0] , txDMA[currentUnitIndex] , txDMAPerSCDMAConfigs.txFIFOCntrsConfigsSCDMA,
                portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex,localDmaMapArr)
            );

        regUnitPerPortAddrUpdate(
            REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, txDMA[0] , txDMA[currentUnitIndex] , txDMAPerSCDMAConfigs.SCDMAConfigs,
                portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex,localDmaMapArr)
            );

        regUnitPerPortAddrUpdate(
            REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, txDMA[0] , txDMA[currentUnitIndex] , txDMAPerSCDMAConfigs.SCDMAConfigs1,
                portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex,localDmaMapArr)
            );

        regUnitPerPortAddrUpdate(
            REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, txDMA[0] , txDMA[currentUnitIndex] , txDMAPerSCDMAConfigs.SCDMAConfigs2,
                portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex,localDmaMapArr)
            );


        regUnitPerPortAddrUpdate(
            REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, txDMA[0] , txDMA[currentUnitIndex] , txDMAPerSCDMAConfigs.SCDMAMetalFixReg,
                portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex,localDmaMapArr)
            );

        regUnitPerPortAddrUpdate(
            REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, txDMA[0] , txDMA[currentUnitIndex] , txDMAPerSCDMAConfigs.SCDMARateLimitResidueVectorThr,
                portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex,localDmaMapArr)
            );

        regUnitPerPortAddrUpdate(
            REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, txDMA[0] , txDMA[currentUnitIndex] , txDMADebug.informativeDebug.SCDMAStatusReg1,
                portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex,localDmaMapArr)
            );

        regUnitPerPortAddrUpdate(
            REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, txDMA[0] , txDMA[currentUnitIndex] , txDMADebug.informativeDebug.SCDMAStatusReg2,
                portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex,localDmaMapArr)
            );

        regUnitPerPortAddrUpdate(
            REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, txDMA[0] , txDMA[currentUnitIndex] , txDMADebug.informativeDebug.SCDMAPrefFIFOFillLevel,
                portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex,localDmaMapArr)
            );

        regUnitPerPortAddrUpdate(
            REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, txDMA[0] , txDMA[currentUnitIndex] , txDMADebug.informativeDebug.SCDMAPrefFIFOMaxFillLevel,
                portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex,localDmaMapArr)
            );
    }
}

/**
* @internal txfifoInit function
* @endinternal
*
* @brief   init the DB - TXFIFO unit (per unit index)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] index                    - unit index
*                                       None
*/
static void txfifoInit
(
    IN  GT_U8 devNum,
    IN  GT_U32  index,
    IN  GT_U32  lastTxFifoIndex,
    IN CPSS_PORTS_BMP_STC *validTxFifoBmpPtr
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    {/*start of unit txFIFO */
        {/*start of unit txFIFOShiftersConfig */
            {/*00000600+0x4*p*/
                GT_U32    p;
                for(p = 0 ; p <= lastTxFifoIndex ; p++) {
                    SKIP_NON_EXISTS_PORT_IN_BMP(validTxFifoBmpPtr,p);
                    regsAddrPtr->txFIFO[index].txFIFOShiftersConfig.SCDMAShiftersConf[p] =
                        0x00000600+0x4*p;
                }/* end of loop p */
            }/*00000600+0x4*p*/

        }/*end of unit txFIFOShiftersConfig */


        {/*start of unit txFIFOLinkedLists */
            {/*start of unit payloadLinkedList */
                {/*start of unit payloadLinkListRAM */
                    regsAddrPtr->txFIFO[index].txFIFOLinkedLists.payloadLinkedList.payloadLinkListRAM.payloadLinkListAddr = 0x00024000;

                }/*end of unit payloadLinkListRAM */


            }/*end of unit payloadLinkedList */


            {/*start of unit headerLinkedList */
                {/*start of unit headerLinkListRAM */
                    regsAddrPtr->txFIFO[index].txFIFOLinkedLists.headerLinkedList.headerLinkListRAM.headerLinkListAddr = 0x00014000;

                }/*end of unit headerLinkListRAM */


            }/*end of unit headerLinkedList */


        }/*end of unit txFIFOLinkedLists */


        {/*start of unit txFIFOInterrupts */
            /*
            regsAddrPtr->txFIFO[index].txFIFOInterrupts.txFIFOGeneralCauseReg1 = 0x00000400;
            regsAddrPtr->txFIFO[index].txFIFOInterrupts.txFIFOGeneralMaskReg1 = 0x00000404;
            regsAddrPtr->txFIFO[index].txFIFOInterrupts.shifterSOPEOPFIFONotReadyInterruptCauseReg1 = 0x00000408;
            regsAddrPtr->txFIFO[index].txFIFOInterrupts.shifterSOPEOPFIFONotReadyInterruptMaskReg1 = 0x0000040c;
            regsAddrPtr->txFIFO[index].txFIFOInterrupts.shifterSOPEOPFIFONotReadyInterruptCauseReg2 = 0x00000410;
            regsAddrPtr->txFIFO[index].txFIFOInterrupts.shifterSOPEOPFIFONotReadyInterruptMaskReg2 = 0x00000414;
            regsAddrPtr->txFIFO[index].txFIFOInterrupts.shifterSOPEOPFIFONotReadyInterruptCauseReg3 = 0x00000418;
            regsAddrPtr->txFIFO[index].txFIFOInterrupts.shifterSOPEOPFIFONotReadyInterruptMaskReg3 = 0x0000041c;
            regsAddrPtr->txFIFO[index].txFIFOInterrupts.CTByteCountArrivedLateInterruptCauseReg1 = 0x00000420;
            regsAddrPtr->txFIFO[index].txFIFOInterrupts.CTByteCountArrivedLateInterruptMaskReg1 = 0x00000424;
            regsAddrPtr->txFIFO[index].txFIFOInterrupts.CTByteCountArrivedLateInterruptCauseReg2 = 0x00000428;
            regsAddrPtr->txFIFO[index].txFIFOInterrupts.CTByteCountArrivedLateInterruptMaskReg2 = 0x0000042c;
            regsAddrPtr->txFIFO[index].txFIFOInterrupts.CTByteCountArrivedLateInterruptCauseReg3 = 0x00000430;
            regsAddrPtr->txFIFO[index].txFIFOInterrupts.CTByteCountArrivedLateInterruptMaskReg3 = 0x00000434;
            regsAddrPtr->txFIFO[index].txFIFOInterrupts.txFIFOSummaryCauseReg = 0x00000440;
            regsAddrPtr->txFIFO[index].txFIFOInterrupts.txFIFOSummaryMaskReg = 0x00000444;
            */

        }/*end of unit txFIFOInterrupts */


        {/*start of unit txFIFOGlobalConfig */
            {/*00000008+p*0x4*/
                GT_U32    p;
                for(p = 0 ; p <= lastTxFifoIndex ; p++) {
                    SKIP_NON_EXISTS_PORT_IN_BMP(validTxFifoBmpPtr,p);
                    regsAddrPtr->txFIFO[index].txFIFOGlobalConfig.SCDMAPayloadThreshold[p] =
                        0x00000008+p*0x4;
                }/* end of loop p */
            }/*00000008+p*0x4*/
            regsAddrPtr->txFIFO[index].txFIFOGlobalConfig.txFIFOMetalFixReg = 0x00000004;

        }/*end of unit txFIFOGlobalConfig */


        {/*start of unit txFIFOEnd2EndECCConfigs */
            {/*00000508+0x4*i*/
                GT_U32    i;
                for(i = 0 ; i <= 1 ; i++) {
                    regsAddrPtr->txFIFO[index].txFIFOEnd2EndECCConfigs.dataECCConfig[i] =
                        0x00000508+0x4*i;
                }/* end of loop i */
            }/*00000508+0x4*i*/
            {/*00000518+0x4*i*/
                GT_U32    i;
                for(i = 0 ; i <= 1 ; i++) {
                    regsAddrPtr->txFIFO[index].txFIFOEnd2EndECCConfigs.dataECCStatus[i] =
                        0x00000518+0x4*i;
                }/* end of loop i */
            }/*00000518+0x4*i*/

        }/*end of unit txFIFOEnd2EndECCConfigs */


        {/*start of unit txFIFODebug */
            {/*start of unit informativeDebug */
                {/*00001b00+p*0x4*/
                    GT_U32    p;
                    for(p = 0 ; p <= lastTxFifoIndex ; p++) {
                        SKIP_NON_EXISTS_PORT_IN_BMP(validTxFifoBmpPtr,p);
                        regsAddrPtr->txFIFO[index].txFIFODebug.informativeDebug.SCDMAStatusReg6[p] =
                            0x00001b00+p*0x4;
                    }/* end of loop p */
                }/*00001b00+p*0x4*/
                {/*00001100+p*0x4*/
                    GT_U32    p;
                    for(p = 0 ; p <= lastTxFifoIndex ; p++) {
                        SKIP_NON_EXISTS_PORT_IN_BMP(validTxFifoBmpPtr,p);
                        regsAddrPtr->txFIFO[index].txFIFODebug.informativeDebug.SCDMAStatusReg1[p] =
                            0x00001100+p*0x4;
                    }/* end of loop p */
                }/*00001100+p*0x4*/
                {/*00001300+p*0x4*/
                    GT_U32    p;
                    for(p = 0 ; p <= lastTxFifoIndex ; p++) {
                        SKIP_NON_EXISTS_PORT_IN_BMP(validTxFifoBmpPtr,p);
                        regsAddrPtr->txFIFO[index].txFIFODebug.informativeDebug.SCDMAStatusReg2[p] =
                            0x00001300+p*0x4;
                    }/* end of loop p */
                }/*00001300+p*0x4*/
                {/*00001500+p*0x4*/
                    GT_U32    p;
                    for(p = 0 ; p <= lastTxFifoIndex ; p++) {
                        SKIP_NON_EXISTS_PORT_IN_BMP(validTxFifoBmpPtr,p);
                        regsAddrPtr->txFIFO[index].txFIFODebug.informativeDebug.SCDMAStatusReg3[p] =
                            0x00001500+p*0x4;
                    }/* end of loop p */
                }/*00001500+p*0x4*/
                {/*00001700+p*0x4*/
                    GT_U32    p;
                    for(p = 0 ; p <= lastTxFifoIndex ; p++) {
                        SKIP_NON_EXISTS_PORT_IN_BMP(validTxFifoBmpPtr,p);
                        regsAddrPtr->txFIFO[index].txFIFODebug.informativeDebug.SCDMAStatusReg4[p] =
                            0x00001700+p*0x4;
                    }/* end of loop p */
                }/*00001700+p*0x4*/
                {/*00001900+p*0x4*/
                    GT_U32    p;
                    for(p = 0 ; p <= lastTxFifoIndex ; p++) {
                        SKIP_NON_EXISTS_PORT_IN_BMP(validTxFifoBmpPtr,p);
                        regsAddrPtr->txFIFO[index].txFIFODebug.informativeDebug.SCDMAStatusReg5[p] =
                            0x00001900+p*0x4;
                    }/* end of loop p */
                }/*00001900+p*0x4*/
                regsAddrPtr->txFIFO[index].txFIFODebug.informativeDebug.debugBusCtrlReg = 0x00001000;
                regsAddrPtr->txFIFO[index].txFIFODebug.informativeDebug.txFIFODebugBus = 0x00001004;
                regsAddrPtr->txFIFO[index].txFIFODebug.informativeDebug.LLEOPCntrLsb = 0x0000100c;
                regsAddrPtr->txFIFO[index].txFIFODebug.informativeDebug.incomingHeadersCntrLsb = 0x00001008;
                regsAddrPtr->txFIFO[index].txFIFODebug.informativeDebug.headerIDDBFreeIDFIFOFillLevel = 0x00001010;
                regsAddrPtr->txFIFO[index].txFIFODebug.informativeDebug.payloadIDDBFreeIDFIFOFillLevel = 0x00001014;
                regsAddrPtr->txFIFO[index].txFIFODebug.informativeDebug.CTBCIDDBFreeIDFIFOFillLevel = 0x00001018;

            }/*end of unit informativeDebug */


        }/*end of unit txFIFODebug */


        {/*start of unit txFIFOPizzaArb */
            {/*start of unit pizzaArbiter */
                {/*00000808+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 84 ; n++) {
                        regsAddrPtr->txFIFO[index].txFIFOPizzaArb.pizzaArbiter.pizzaArbiterConfigReg[n] =
                            0x00000808+n*0x4;
                    }/* end of loop n */
                }/*00000808+n*0x4*/
                regsAddrPtr->txFIFO[index].txFIFOPizzaArb.pizzaArbiter.pizzaArbiterCtrlReg = 0x00000800;
                regsAddrPtr->txFIFO[index].txFIFOPizzaArb.pizzaArbiter.pizzaArbiterStatusReg = 0x00000804;

            }/*end of unit pizzaArbiter */


        }/*end of unit txFIFOPizzaArb */

        if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
        {
            regsAddrPtr->txFIFO[index].txFIFODebug.informativeDebug.LLEOPCntrLsb = 0x00001010;
            regsAddrPtr->txFIFO[index].txFIFODebug.informativeDebug.LLEOPCntrMsb = 0x00001014;
            regsAddrPtr->txFIFO[index].txFIFODebug.informativeDebug.headerIDDBFreeIDFIFOFillLevel = 0x00001018;
            regsAddrPtr->txFIFO[index].txFIFODebug.informativeDebug.incomingHeadersCntrMsb = 0x0000100c;
            regsAddrPtr->txFIFO[index].txFIFODebug.informativeDebug.payloadIDDBFreeIDFIFOFillLevel = 0x0000101c;
            regsAddrPtr->txFIFO[index].txFIFODebug.informativeDebug.CTBCIDDBFreeIDFIFOFillLevel = 0x00001020;
        }

    }/*end of unit txFIFO */
}

/**
* @internal txfifoUpdateUnit0AsGlobalPortsAddr function
* @endinternal
*
* @brief   update the DB - txFIFO unit - after multi units set 'per port' addresses.
*         the DB of txFIFO[0] updated for the ports with 'global port index'
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] currentUnitIndex         - the index of current txFIFO[] unit
* @param[in] portIn_unit_0            - the 'global port' index (in unit 0)
* @param[in] portInCurrentUnit        - the 'local port'  index (in current unit)
* @param[in] numOfPorts               - number of ports to update.
* @param[in] isFirstUpdate            - indication of first update
* @param[in] isLastUpdate             - indication of last update
*                                       None
*/
static void txfifoUpdateUnit0AsGlobalPortsAddr
(
    IN  GT_U8 devNum,
    IN  GT_U32  currentUnitIndex,
    IN  GT_U32  portIn_unit_0,
    IN  GT_U32  portInCurrentUnit,
    IN  GT_U32  numOfPorts,
    IN  GT_BOOL isFirstUpdate,
    IN  GT_BOOL isLastUpdate
)
{
    IN GT_U32    saveLastNumOfPorts;/*number of ports from the end of the unitStart1Ptr[]
                                  that need to be saved before overridden by unitStart2Ptr[]*/
    IN GT_U32    saveToIndex;/*the index in unitStart1Ptr[] to save the 'last ports'
                                  valid when saveLastNumOfPorts != 0*/
    const GT_U32 *localDmaMapArr; /* local DMAs in specific DP, NULL in cases 1:1 mapping */

    localDmaMapArr = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[currentUnitIndex].localDmaMapArr;

    if(isLastUpdate == GT_TRUE &&
       PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.supportRelativePortNum)
    {
        /* do not let the DB be overridden with the CPU port of last DP */
        /* because we saved the CPU port of the first DP */
        numOfPorts -=
            (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[0].cpuPortDmaNum ? 1 : 0)+
            (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[0].tmDmaNum ? 1 : 0);
    }

    if(isFirstUpdate == GT_TRUE &&
       PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.supportRelativePortNum)
    {
        /* update the DB with the CPU port address on the lowest existing DP */
        /* save CPU port */
        saveLastNumOfPorts =
            (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[0].cpuPortDmaNum ? 1 : 0)+
            (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[0].tmDmaNum ? 1 : 0);

        saveToIndex = CPU_PORT_DMA_CNS(devNum);
    }
    else
    {
        if((localDmaMapArr != NULL) && (isLastUpdate == GT_TRUE))
        {
            /* move CPU port registers addresses to position 72 */
            saveLastNumOfPorts = 1;
            saveToIndex = CPU_PORT_DMA_CNS(devNum);
        }
        else
        {
            saveLastNumOfPorts = 0;
            saveToIndex = 0;
        }
    }
    /************************************/
    /*   update the per port addresses  */
    /*   of the txFIFO unit 0 with those */
    /*   of unit 1                      */
    /************************************/
    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
           /* sip6_txFIFO */
        regUnitPerPortAddrUpdate(
            REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, sip6_txFIFO[0] , sip6_txFIFO[currentUnitIndex] , configs.channelConfigs.speedProfile,
                portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex,localDmaMapArr)
            );
        regUnitPerPortAddrUpdate(
            REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, sip6_txFIFO[0] , sip6_txFIFO[currentUnitIndex] , configs.channelConfigs.descFIFOBase,
                portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex,localDmaMapArr)
            );
        regUnitPerPortAddrUpdate(
            REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, sip6_txFIFO[0] , sip6_txFIFO[currentUnitIndex] , configs.channelConfigs.tagFIFOBase,
                portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex,localDmaMapArr)
            );

    }
    else
    {

        regUnitPerPortAddrUpdate(
            REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, txFIFO[0] , txFIFO[currentUnitIndex] , txFIFOShiftersConfig.SCDMAShiftersConf,
                portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex,localDmaMapArr)
            );


        regUnitPerPortAddrUpdate(
            REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, txFIFO[0] , txFIFO[currentUnitIndex] , txFIFOGlobalConfig.SCDMAPayloadThreshold,
                portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex,localDmaMapArr)
            );

        regUnitPerPortAddrUpdate(
            REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, txFIFO[0] , txFIFO[currentUnitIndex] , txFIFODebug.informativeDebug.SCDMAStatusReg1,
                portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex,localDmaMapArr)
            );

        regUnitPerPortAddrUpdate(
            REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, txFIFO[0] , txFIFO[currentUnitIndex] , txFIFODebug.informativeDebug.SCDMAStatusReg2,
                portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex,localDmaMapArr)
            );

        regUnitPerPortAddrUpdate(
            REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, txFIFO[0] , txFIFO[currentUnitIndex] , txFIFODebug.informativeDebug.SCDMAStatusReg3,
                portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex,localDmaMapArr)
            );

        regUnitPerPortAddrUpdate(
            REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, txFIFO[0] , txFIFO[currentUnitIndex] , txFIFODebug.informativeDebug.SCDMAStatusReg4,
                portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex,localDmaMapArr)
            );

        regUnitPerPortAddrUpdate(
            REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, txFIFO[0] , txFIFO[currentUnitIndex] , txFIFODebug.informativeDebug.SCDMAStatusReg5,
                portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex,localDmaMapArr)
            );

        regUnitPerPortAddrUpdate(
            REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, txFIFO[0] , txFIFO[currentUnitIndex] , txFIFODebug.informativeDebug.SCDMAStatusReg6,
                portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex,localDmaMapArr)
            );
    }

}

/* NOTE: in SIP6 using function prvCpssSip6OffsetFromFirstInstanceGet(...) ,
    so using only index 0 in this array */
static PRV_CPSS_DXCH_UNIT_ENT rxDmaUnits[MAX_DP_CNS] =
{
     PRV_CPSS_DXCH_UNIT_RXDMA_E
    ,PRV_CPSS_DXCH_UNIT_RXDMA1_E
    ,PRV_CPSS_DXCH_UNIT_RXDMA2_E
    ,PRV_CPSS_DXCH_UNIT_RXDMA3_E
    ,PRV_CPSS_DXCH_UNIT_RXDMA4_E
    ,PRV_CPSS_DXCH_UNIT_RXDMA5_E
};
/* NOTE: in SIP6 using function prvCpssSip6OffsetFromFirstInstanceGet(...) ,
    so using only index 0 in this array */
static PRV_CPSS_DXCH_UNIT_ENT txDmaUnits[MAX_DP_CNS] =
{
     PRV_CPSS_DXCH_UNIT_TXDMA_E
    ,PRV_CPSS_DXCH_UNIT_TXDMA1_E
    ,PRV_CPSS_DXCH_UNIT_TXDMA2_E
    ,PRV_CPSS_DXCH_UNIT_TXDMA3_E
    ,PRV_CPSS_DXCH_UNIT_TXDMA4_E
    ,PRV_CPSS_DXCH_UNIT_TXDMA5_E
};
/* NOTE: in SIP6 using function prvCpssSip6OffsetFromFirstInstanceGet(...) ,
    so using only index 0 in this array */
static PRV_CPSS_DXCH_UNIT_ENT txFifoUnits[MAX_DP_CNS] =
{
     PRV_CPSS_DXCH_UNIT_TX_FIFO_E
    ,PRV_CPSS_DXCH_UNIT_TX_FIFO1_E
    ,PRV_CPSS_DXCH_UNIT_TX_FIFO2_E
    ,PRV_CPSS_DXCH_UNIT_TX_FIFO3_E
    ,PRV_CPSS_DXCH_UNIT_TX_FIFO4_E
    ,PRV_CPSS_DXCH_UNIT_TX_FIFO5_E
};
/* NOTE: NOT relevant in SIP6 */
static PRV_CPSS_DXCH_UNIT_ENT txqDqUnits[MAX_DP_CNS] =
{
     PRV_CPSS_DXCH_UNIT_TXQ_DQ_E
    ,PRV_CPSS_DXCH_UNIT_TXQ_DQ1_E
    ,PRV_CPSS_DXCH_UNIT_TXQ_DQ2_E
    ,PRV_CPSS_DXCH_UNIT_TXQ_DQ3_E
    ,PRV_CPSS_DXCH_UNIT_TXQ_DQ4_E
    ,PRV_CPSS_DXCH_UNIT_TXQ_DQ5_E
};

static GT_U32 bc3_arr[]={74,75,72,76,77,73};

                  /*DP index : 0  1  2  3      index 4 is 'trick' for CPU port that also in DP[0] */
static GT_U32 aldrin2_arr[]     ={77,75,76,74,              72};
static GT_U32 aldrin2_indexArr[]={23,24,24,24,              24};
static GT_U32 aldrin2_startIndexUnit0[4] = {0/*12 ports*/,12/*12 ports*/,24/*24 ports*/,48/*24 ports*/};

/**
* @internal dmaCpuGlobalPortUpdate function
* @endinternal
*
* @brief   update for RxDMA,TxDMA,TxFIFO the global port addresses
*         BOBCAT3 : copy address from DP[0..5] 'local' DMA12 to DP[0] 'global' DMA [74,75,72,76,77,73]
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] index                    - unit index
*                                       None
*/
static void dmaCpuGlobalPortUpdate
(
    IN  GT_U8 devNum,
    IN  GT_U32  index
)
{
    GT_STATUS   rc;
    GT_U32 newIndex;/* global index for the 'special ports' of DP[index]*/
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32  lastRxDmaIndex,
            lastTxDmaIndex,
            lastTxFifoIndex;

    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
    {
        newIndex =  bc3_arr[index];

        lastRxDmaIndex  = 12;
        lastTxDmaIndex  = 12;
        lastTxFifoIndex = 12;
    }
    else
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
    {
        lastRxDmaIndex  = FALCON_PORTS_PER_DP_CNS;
        lastTxDmaIndex  = FALCON_PORTS_PER_DP_CNS;
        lastTxFifoIndex = FALCON_PORTS_PER_DP_CNS;

        /* get the DP of the 'special DMAs' */
        rc = prvCpssFalconDmaLocalNumInDpToGlobalNumConvert(devNum,
            index,lastRxDmaIndex,&newIndex/*globalDmaNum*/);
        if(rc != GT_OK)
        {
            /* the current DP hold no CPU port */
            return;
        }
    }
    else
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
    {
        if(index == 0)
        {
            /* 'trick' call this function ALSO with index [4] to get support also
                for the additional CPU port in dp[0] */
            dmaCpuGlobalPortUpdate(devNum,4);
        }

        newIndex =  aldrin2_arr[index];

        lastRxDmaIndex  = aldrin2_indexArr[index];
        lastTxDmaIndex  = aldrin2_indexArr[index];
        lastTxFifoIndex = aldrin2_indexArr[index];

        if(index == 4)
        {
            /* the below code must use index '0' and forget about the 'trick' */
            index = 0;
        }
    }
    else
    {
        /* logic below not relevant to the device */
        return ;
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

doNextCpuPortInCurrentDp_lbl:

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        {
            GT_U32 minus_offset = index * (sizeof(regsAddrPtr->rxDMA[0]) / sizeof(GT_U32));
            /* copy last RxDma to cpu port position */
            COPY_REG_ADDR_MAC(regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.channelPIPConfigReg , lastRxDmaIndex, minus_offset/*target to DP*/ ,newIndex);
        }
        {
            GT_U32 minus_offset = index * (sizeof(regsAddrPtr->sip6_rxDMA[0]) / sizeof(GT_U32));
            /* copy last RxDma to cpu port position */
            COPY_REG_ADDR_MAC(regsAddrPtr->sip6_rxDMA[index].configs.cutThrough.channelCTConfig                , lastRxDmaIndex, minus_offset/*target to DP*/ ,newIndex);
            COPY_REG_ADDR_MAC(regsAddrPtr->sip6_rxDMA[index].configs.channelConfig.channelToLocalDevSourcePort , lastRxDmaIndex, minus_offset/*target to DP*/ ,newIndex);
            COPY_REG_ADDR_MAC(regsAddrPtr->sip6_rxDMA[index].configs.channelConfig.PCHConfig                   , lastRxDmaIndex, minus_offset/*target to DP*/ ,newIndex);
            COPY_REG_ADDR_MAC(regsAddrPtr->sip6_rxDMA[index].configs.channelConfig.channelGeneralConfigs       , lastRxDmaIndex, minus_offset/*target to DP*/ ,newIndex);
            COPY_REG_ADDR_MAC(regsAddrPtr->sip6_rxDMA[index].configs.channelConfig.channelCascadePort          , lastRxDmaIndex, minus_offset/*target to DP*/ ,newIndex);
        }
    }
    else
    {
        GT_U32 minus_offset = index * (sizeof(regsAddrPtr->rxDMA[0]) / sizeof(GT_U32));
        /* copy last RxDma to cpu port position */
        COPY_REG_ADDR_MAC(regsAddrPtr->rxDMA[index].singleChannelDMAConfigs.SCDMAConfig0              , lastRxDmaIndex, minus_offset/*target to DP*/ ,newIndex);
        COPY_REG_ADDR_MAC(regsAddrPtr->rxDMA[index].singleChannelDMAConfigs.SCDMAConfig1              , lastRxDmaIndex, minus_offset/*target to DP*/ ,newIndex);
        COPY_REG_ADDR_MAC(regsAddrPtr->rxDMA[index].singleChannelDMAConfigs.SCDMACTPktIndentification , lastRxDmaIndex, minus_offset/*target to DP*/ ,newIndex);
        COPY_REG_ADDR_MAC(regsAddrPtr->rxDMA[index].scdmaDebug.portBuffAllocCnt                       , lastRxDmaIndex, minus_offset/*target to DP*/ ,newIndex);
        COPY_REG_ADDR_MAC(regsAddrPtr->rxDMA[index].interrupts.rxDMASCDMAInterruptMask                , lastRxDmaIndex, minus_offset/*target to DP*/ ,newIndex);
        COPY_REG_ADDR_MAC(regsAddrPtr->rxDMA[index].interrupts.rxDMASCDMAInterruptCause               , lastRxDmaIndex, minus_offset/*target to DP*/ ,newIndex);
        COPY_REG_ADDR_MAC(regsAddrPtr->rxDMA[index].singleChannelDMAPip.SCDMA_n_PIP_Config            , lastRxDmaIndex, minus_offset/*target to DP*/ ,newIndex);
        COPY_REG_ADDR_MAC(regsAddrPtr->rxDMA[index].singleChannelDMAPip.SCDMA_n_Drop_Pkt_Counter      , lastRxDmaIndex, minus_offset/*target to DP*/ ,newIndex);
    }
    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        GT_U32 minus_offset = index * (sizeof(regsAddrPtr->sip6_txDMA[0]) / sizeof(GT_U32));

        COPY_REG_ADDR_MAC(regsAddrPtr->sip6_txDMA[index].configs.channelConfigs.channelReset                , lastTxDmaIndex, minus_offset/*target to DP*/ ,newIndex);
        COPY_REG_ADDR_MAC(regsAddrPtr->sip6_txDMA[index].configs.channelConfigs.descFIFOBase                , lastTxDmaIndex, minus_offset/*target to DP*/ ,newIndex);
        COPY_REG_ADDR_MAC(regsAddrPtr->sip6_txDMA[index].configs.channelConfigs.inerPacketRateLimiterConfig , lastTxDmaIndex, minus_offset/*target to DP*/ ,newIndex);
        COPY_REG_ADDR_MAC(regsAddrPtr->sip6_txDMA[index].configs.channelConfigs.interCellRateLimiterConfig  , lastTxDmaIndex, minus_offset/*target to DP*/ ,newIndex);
        COPY_REG_ADDR_MAC(regsAddrPtr->sip6_txDMA[index].configs.channelConfigs.interPacketRateLimiterConfig, lastTxDmaIndex, minus_offset/*target to DP*/ ,newIndex);
        COPY_REG_ADDR_MAC(regsAddrPtr->sip6_txDMA[index].configs.channelConfigs.speedProfile                , lastTxDmaIndex, minus_offset/*target to DP*/ ,newIndex);
    }
    else
     {
        GT_U32 minus_offset = index * (sizeof(regsAddrPtr->txDMA[0]) / sizeof(GT_U32));

        COPY_REG_ADDR_MAC(regsAddrPtr->txDMA[index].txDMAPerSCDMAConfigs.burstLimiterSCDMA                , lastTxDmaIndex, minus_offset/*target to DP*/ ,newIndex);
        COPY_REG_ADDR_MAC(regsAddrPtr->txDMA[index].txDMAPerSCDMAConfigs.FIFOsThresholdsConfigsSCDMAReg1  , lastTxDmaIndex, minus_offset/*target to DP*/ ,newIndex);
        COPY_REG_ADDR_MAC(regsAddrPtr->txDMA[index].txDMAPerSCDMAConfigs.txFIFOCntrsConfigsSCDMA          , lastTxDmaIndex, minus_offset/*target to DP*/ ,newIndex);
        COPY_REG_ADDR_MAC(regsAddrPtr->txDMA[index].txDMAPerSCDMAConfigs.SCDMAConfigs                     , lastTxDmaIndex, minus_offset/*target to DP*/ ,newIndex);
        COPY_REG_ADDR_MAC(regsAddrPtr->txDMA[index].txDMAPerSCDMAConfigs.SCDMAConfigs1                    , lastTxDmaIndex, minus_offset/*target to DP*/ ,newIndex);
        COPY_REG_ADDR_MAC(regsAddrPtr->txDMA[index].txDMAPerSCDMAConfigs.SCDMAConfigs2                    , lastTxDmaIndex, minus_offset/*target to DP*/ ,newIndex);
        COPY_REG_ADDR_MAC(regsAddrPtr->txDMA[index].txDMAPerSCDMAConfigs.SCDMAMetalFixReg                 , lastTxDmaIndex, minus_offset/*target to DP*/ ,newIndex);
        COPY_REG_ADDR_MAC(regsAddrPtr->txDMA[index].txDMAPerSCDMAConfigs.SCDMARateLimitResidueVectorThr   , lastTxDmaIndex, minus_offset/*target to DP*/ ,newIndex);
        COPY_REG_ADDR_MAC(regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.SCDMAStatusReg1           , lastTxDmaIndex, minus_offset/*target to DP*/ ,newIndex);
        COPY_REG_ADDR_MAC(regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.SCDMAStatusReg2           , lastTxDmaIndex, minus_offset/*target to DP*/ ,newIndex);
        COPY_REG_ADDR_MAC(regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.SCDMAPrefFIFOFillLevel    , lastTxDmaIndex, minus_offset/*target to DP*/ ,newIndex);
        COPY_REG_ADDR_MAC(regsAddrPtr->txDMA[index].txDMADebug.informativeDebug.SCDMAPrefFIFOMaxFillLevel , lastTxDmaIndex, minus_offset/*target to DP*/ ,newIndex);

    }
    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {

        GT_U32 minus_offset = index * (sizeof(regsAddrPtr->sip6_txFIFO[0]) / sizeof(GT_U32));
        /* copy last RxDma to cpu port position */
        COPY_REG_ADDR_MAC(regsAddrPtr->sip6_txFIFO[index].configs.channelConfigs.speedProfile                , lastTxFifoIndex, minus_offset/*target to DP*/ ,newIndex);
        COPY_REG_ADDR_MAC(regsAddrPtr->sip6_txFIFO[index].configs.channelConfigs.descFIFOBase                , lastTxFifoIndex, minus_offset/*target to DP*/ ,newIndex);
        COPY_REG_ADDR_MAC(regsAddrPtr->sip6_txFIFO[index].configs.channelConfigs.tagFIFOBase                 , lastTxFifoIndex, minus_offset/*target to DP*/ ,newIndex);

    }
    else
    {
       GT_U32 minus_offset = index * (sizeof(regsAddrPtr->txFIFO[0]) / sizeof(GT_U32));
        /* copy last RxDma to cpu port position */
        COPY_REG_ADDR_MAC(regsAddrPtr->txFIFO[index].txFIFOShiftersConfig.SCDMAShiftersConf       , lastTxFifoIndex, minus_offset/*target to DP*/ ,newIndex);
        COPY_REG_ADDR_MAC(regsAddrPtr->txFIFO[index].txFIFOGlobalConfig.SCDMAPayloadThreshold     , lastTxFifoIndex, minus_offset/*target to DP*/ ,newIndex);
        COPY_REG_ADDR_MAC(regsAddrPtr->txFIFO[index].txFIFODebug.informativeDebug.SCDMAStatusReg1 , lastTxFifoIndex, minus_offset/*target to DP*/ ,newIndex);
        COPY_REG_ADDR_MAC(regsAddrPtr->txFIFO[index].txFIFODebug.informativeDebug.SCDMAStatusReg2 , lastTxFifoIndex, minus_offset/*target to DP*/ ,newIndex);
        COPY_REG_ADDR_MAC(regsAddrPtr->txFIFO[index].txFIFODebug.informativeDebug.SCDMAStatusReg3 , lastTxFifoIndex, minus_offset/*target to DP*/ ,newIndex);
        COPY_REG_ADDR_MAC(regsAddrPtr->txFIFO[index].txFIFODebug.informativeDebug.SCDMAStatusReg4 , lastTxFifoIndex, minus_offset/*target to DP*/ ,newIndex);
        COPY_REG_ADDR_MAC(regsAddrPtr->txFIFO[index].txFIFODebug.informativeDebug.SCDMAStatusReg5 , lastTxFifoIndex, minus_offset/*target to DP*/ ,newIndex);
        COPY_REG_ADDR_MAC(regsAddrPtr->txFIFO[index].txFIFODebug.informativeDebug.SCDMAStatusReg6 , lastTxFifoIndex, minus_offset/*target to DP*/ ,newIndex);
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* get NEXT global DMA port that may occupy this DP */
        rc = prvCpssFalconDmaLocalNumInDpToGlobalNumConvert_getNext(devNum,
            index,lastRxDmaIndex,&newIndex/*globalDmaNum*/);
        if(rc == GT_OK)
        {
            /* we got 'next' newIndex !!! */
            /* do logic for it too        */
            goto doNextCpuPortInCurrentDp_lbl;
        }
    }

}



/**
* @internal dataPathInit function
* @endinternal
*
* @brief   init for DATA PATH . init for RxDma,txDma,txFifo,txq-DQ.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                      index   - unit index
*                                       None
*/
static void dataPathInit
(
    IN  GT_U8 devNum,
    IN  GT_U32 * unitBasePtr
)
{
    GT_U32  ii,jj;
    GT_U32  supportMultiDataPath = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.supportMultiDataPath;
    GT_U32  validDataPathBmp;/* bmp of existing DPs (data path)*/
    GT_U32  portIn_unit_0;/* current port in unit 0 (global port) */
    GT_U32  port_inUnit_ii;/* current port in unit ii (local port) */
    GT_U32  numOfPorts;/* number of ports in the unit ii*/
    GT_U32  numOfRxSdmaPorts;/* number of RX SDMA ports in the unit ii*/
    GT_BOOL isFirstUpdate; /* indication of first update */
    GT_BOOL isLastUpdate;  /* indication of last update  */
    GT_U32  lastRxDmaIndex;/* last index for 'per port' in the RxDma unit */
    GT_U32  lastTxDmaIndex;/* last index for 'per port' in the TxDma/TxFifo unit */
    GT_U32  numTxqDqPortsPerUnit_firstBlock_lastIndex;/* number of txq-dq , per port indexes in first block */
    GT_U32  numTxqDqPortsPerUnit_secondBlock_lastIndex;/* number of txq-dq , per port indexes in first block (valid for device with more than 72 ports per DQ) */
    GT_U32  currRxDmaBaseAddr,currTxDmaBaseAddr,currTxFifoBaseAddr;/*current base address of DP units */
    GT_U32  dp0RxDmaBaseAddr = 0,dp0TxDmaBaseAddr = 0,dp0TxFifoBaseAddr = 0;/*base address of unit in DP[0]*/
    GT_U32  offsetFromDp0;/* offset of the current DP unit from DP[0]*/

    CPSS_PORTS_BMP_STC validRxDmaBmp[MAX_DP_CNS];/* bmp of valid ports in the RxDma[ii] */
    CPSS_PORTS_BMP_STC validTxDmaBmp[MAX_DP_CNS];/* bmp of valid ports in the TxDma[ii]/TxFifo[ii] */

    /* init the BMP to hold all ports */
    for(jj = 0 ; jj < MAX_DP_CNS ; jj++)
    {
        CPSS_PORTS_BMP_PORT_SET_ALL_MAC(&(validRxDmaBmp[jj]));
        CPSS_PORTS_BMP_PORT_SET_ALL_MAC(&(validTxDmaBmp[jj]));
    }

    numTxqDqPortsPerUnit_firstBlock_lastIndex = (SIP_5_DQ_NUM_PORTS_CNS - 1);
    numTxqDqPortsPerUnit_secondBlock_lastIndex = PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) ?
                    (SIP_5_20_DQ_NUM_PORTS_CNS - SIP_5_DQ_NUM_PORTS_CNS) :
                    0;

    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
    {
        /*TXQ-DQ*/
        numTxqDqPortsPerUnit_firstBlock_lastIndex = 24;
        numTxqDqPortsPerUnit_secondBlock_lastIndex = 0;

        /* RxDma */
        lastRxDmaIndex = 24;
        /* TxDma/Fifo */
        lastTxDmaIndex = 24;

        validRxDmaBmp[0].ports[0] = validTxDmaBmp[0].ports[0] =
                (BIT_12-1) /* ports 0..11 */ | (BIT_23 | BIT_24);/*ports 23,24*/
        validRxDmaBmp[1].ports[0] = validTxDmaBmp[1].ports[0] =
                (BIT_12-1) /* ports 0..11 */ |          (BIT_24);/*port 24*/
    }
    else
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
    {
        lastRxDmaIndex = 12;
        lastTxDmaIndex = 12;
    }
    else
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
    {
        lastRxDmaIndex = FALCON_PORTS_PER_DP_CNS;
        lastTxDmaIndex = FALCON_PORTS_PER_DP_CNS;
    }
    else
    {
        lastRxDmaIndex = CPU_PORT_DMA_CNS(devNum);
        lastTxDmaIndex = 73;
    }


    if(supportMultiDataPath)
    {
        validDataPathBmp = 0;
        for(ii = 0 ; ii < PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp; ii++)
        {
            if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[ii].dataPathNumOfPorts == 0)
            {
                /* not valid DP (data path) */
                continue;
            }

            validDataPathBmp |= (BIT_0 << ii);
        }
    }
    else
    {
        validDataPathBmp = BIT_0;
    }

    isFirstUpdate = GT_TRUE;
    isLastUpdate = GT_FALSE;
    ii = 0;
    for(jj = 0 ; jj < MAX_DP_CNS ; jj++)
    {
        if (0 == (validDataPathBmp & (1<<jj)))
        {
            continue;
        }

        if(validDataPathBmp < (GT_U32)(1 << (jj+1)))
        {
            isLastUpdate = GT_TRUE;
        }

        /* NOTE: the first time we get here ii is 0 ,
           but jj can be more than 0 !!!

           we initialize the rxDMA[0] with the register values , because we want
           the CPSS to use rxDMA[0] and not be aware to multi-units.
        */
        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            if(jj == 0)
            {
                dp0RxDmaBaseAddr  = prvCpssDxChHwUnitBaseAddrGet(devNum,rxDmaUnits[0] ,NULL);
                dp0TxDmaBaseAddr  = prvCpssDxChHwUnitBaseAddrGet(devNum,txDmaUnits[0] ,NULL);
                dp0TxFifoBaseAddr = prvCpssDxChHwUnitBaseAddrGet(devNum,txFifoUnits[0],NULL);

                currRxDmaBaseAddr  = dp0RxDmaBaseAddr;
                currTxDmaBaseAddr  = dp0TxDmaBaseAddr;
                currTxFifoBaseAddr = dp0TxFifoBaseAddr;
            }
            else
            {
                offsetFromDp0 = prvCpssSip6OffsetFromFirstInstanceGet(devNum,jj,rxDmaUnits[0]);
                currRxDmaBaseAddr = dp0RxDmaBaseAddr + offsetFromDp0;

                offsetFromDp0 = prvCpssSip6OffsetFromFirstInstanceGet(devNum,jj,txDmaUnits[0]);
                currTxDmaBaseAddr = dp0TxDmaBaseAddr + offsetFromDp0;

                offsetFromDp0 = prvCpssSip6OffsetFromFirstInstanceGet(devNum,jj,txFifoUnits[0]);
                currTxFifoBaseAddr = dp0TxFifoBaseAddr + offsetFromDp0;
            }
        }
        else
        {
            currRxDmaBaseAddr  = unitBasePtr[rxDmaUnits[jj]];
            currTxDmaBaseAddr  = unitBasePtr[txDmaUnits[jj]];
            currTxFifoBaseAddr = unitBasePtr[txFifoUnits[jj]];
        }

        /****************/
        /* init unit ii */
        /****************/
        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            rxdmaInit_sip6(devNum,ii,lastRxDmaIndex,&validRxDmaBmp[ii]);
            /* set the addresses of the rxdma unit registers according to the needed base address */
            /* some of the registers in 'rxDMA' */
            regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, currRxDmaBaseAddr, rxDMA[ii]));
            /* other registers in 'sip6_rxDMA' */
            regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, currRxDmaBaseAddr, sip6_rxDMA[ii]));
        }
        else
        {
            rxdmaInit(devNum,ii,lastRxDmaIndex,&validRxDmaBmp[ii]);
            /* set the addresses of the rxdma unit registers according to the needed base address */
            regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, currRxDmaBaseAddr, rxDMA[ii]));
        }

        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
             txdmaInit_sip6(devNum,ii,lastTxDmaIndex,&validTxDmaBmp[ii]);

             /* set the addresses of the txdma unit registers according to the needed base address */
             regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, currTxDmaBaseAddr, sip6_txDMA[ii]));

             txfifoInit_sip6(devNum,ii,lastTxDmaIndex,&validTxDmaBmp[ii]);

             /* set the addresses of the txfifo unit registers according to the needed base address */
             regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, currTxFifoBaseAddr, sip6_txFIFO[ii]));
        }
        else
        {
            txdmaInit(devNum,ii,lastTxDmaIndex,&validTxDmaBmp[ii]);
            /* set the addresses of the txdma unit registers according to the needed base address */
            regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, currTxDmaBaseAddr, txDMA[ii]));

            txfifoInit(devNum,ii,lastTxDmaIndex,&validTxDmaBmp[ii]);
            /* set the addresses of the txfifo unit registers according to the needed base address */
            regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, currTxFifoBaseAddr, txFIFO[ii]));
        }

        /* CPU port(s) : update for RxDMA,TxDMA,TxFIFO the global port addresses */
        dmaCpuGlobalPortUpdate(devNum,ii);


        if((ii != 0) ||
           (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[ii].localDmaMapArr != NULL))/* have multi data path , set 'global' ports */
        {
            portIn_unit_0 = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[ii].dataPathFirstPort;
            numOfPorts    = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[ii].dataPathNumOfPorts;

            if(isLastUpdate == GT_TRUE &&
               PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
            {
                /* in Aldrin2 we hold dataPathNumOfPorts WITHOUT the 'CPU ethernet' port
                    and WITHOUT the CPU-SDMA port(s)
                */

                /* the function below expect to get extra port ...
                    since logic doing 'decrement' in those functions */
                numOfPorts++;
            }

            numOfRxSdmaPorts = numOfPorts;
            if(isLastUpdate &&
               (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.rxNumPortsPerDp < PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.txNumPortsPerDp))
            {
                /* RX SDMA does not have TM port that is counted in dataPathNumOfPorts.
                   Number of ports of last RX SDMA unit must be corrected. */
                --numOfRxSdmaPorts;
            }

            if(0 == PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.supportRelativePortNum)
            {
                /* all the Data path units hold 'global ports' */
                port_inUnit_ii = portIn_unit_0; /* global port */
            }
            else
            {
                /* all the Data path units hold 'local ports' */
                port_inUnit_ii = 0;/* local port */
            }

            /* update the DB - RXDMA unit - set 'global' ports 'per port' addresses. */
            rxdmaUpdateUnit0AsGlobalPortsAddr(devNum,ii,portIn_unit_0,port_inUnit_ii,numOfRxSdmaPorts,
                isFirstUpdate,isLastUpdate);

            /* update the DB - TXDMA unit - set 'global' ports 'per port' addresses. */
            txdmaUpdateUnit0AsGlobalPortsAddr(devNum,ii,portIn_unit_0,port_inUnit_ii,numOfPorts,
                isFirstUpdate,isLastUpdate);

            /* update the DB - txfifo unit - set 'global' ports 'per port' addresses. */
            txfifoUpdateUnit0AsGlobalPortsAddr(devNum,ii,portIn_unit_0,port_inUnit_ii,numOfPorts,
                isFirstUpdate,isLastUpdate);

            if(isFirstUpdate == GT_TRUE)
            {
                isFirstUpdate = GT_FALSE;
            }
        }


        ii = jj + 1;/* after the first time ii set to be like jj
                      (the jj will get ++ in the next operation) */
    }

    if (PRV_CPSS_DXCH_CETUS_CHECK_MAC(devNum))
    {
        PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->rxDMA[1] = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->rxDMA[0];
        PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->txDMA[1] = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->txDMA[0];
        PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->txFIFO[1] = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->txFIFO[0];
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* SIP6 not support TXQ-DQ unit (was replaced by TXQ-SDQ) */
        validDataPathBmp = 0;
    }
    else
    if(supportMultiDataPath)
    {
        validDataPathBmp = (BIT_0 << PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.numTxqDq) - 1;
    }
    else
    {
        validDataPathBmp = BIT_0;
    }

    if(!PRV_CPSS_SIP_6_CHECK_MAC(devNum))/* no TXQ-DQ in sip6 */
    {
        isFirstUpdate = GT_TRUE;
        isLastUpdate = GT_FALSE;
        for(ii = 0 ; ii < MAX_DP_CNS ; ii++)
        {
            if (0 == (validDataPathBmp & (1<<ii)))
            {
                continue;
            }

            /* init the TXQ_DQ unit*/
            txqDqInit(devNum,ii,numTxqDqPortsPerUnit_firstBlock_lastIndex,numTxqDqPortsPerUnit_secondBlock_lastIndex,lastTxDmaIndex);
            /* set the addresses of the TXQ_DQ unit registers according to the needed base address */
            regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[txqDqUnits[ii]], TXQ.dq[ii]));

            if(validDataPathBmp == BIT_0)
            {
                break;
            }

            txqDqDmaRelatedGlobalPortUpdate(devNum,ii);

            /* update the DB - txq-DQ unit - set 'global' ports 'per port' addresses. */
            numOfPorts    = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.txqDqNumPortsPerDp;
            portIn_unit_0 = ii*numOfPorts;
            port_inUnit_ii = 0;
            txqDqUpdateUnit0AsGlobalPortsAddr(devNum,ii,portIn_unit_0,port_inUnit_ii,numOfPorts);
        }
    }

}


/**
* @internal ethTxFifoInit function
* @endinternal
*
* @brief   init the DB - ethTxFifo unit
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None
*/
static void ethTxFifoInit
(
    IN  GT_U8 devNum,
    IN  GT_U32  idx
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);


    {/*start of unit SIP_ETH_TXFIFO */
        {/*start of unit txFIFOShiftersConfig */
            {/*00000600+0x4*p*/
                GT_U32    p;
                for(p = 0 ; p <= 72 ; p++) {
                    regsAddrPtr->SIP_ETH_TXFIFO[idx].txFIFOShiftersConfig.SCDMAShiftersConf[p] =
                        0x00000600+0x4*p;
                }/* end of loop p */
            }/*00000600+0x4*p*/

        }/*end of unit txFIFOShiftersConfig */


        {/*start of unit txFIFOPizzaArb */
            {/*start of unit pizzaArbiter */
                {/*00000808+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 84 ; n++) {
                        regsAddrPtr->SIP_ETH_TXFIFO[idx].txFIFOPizzaArb.pizzaArbiter.pizzaArbiterConfigReg[n] =
                            0x00000808+n*0x4;
                    }/* end of loop n */
                }/*00000808+n*0x4*/
                regsAddrPtr->SIP_ETH_TXFIFO[idx].txFIFOPizzaArb.pizzaArbiter.pizzaArbiterCtrlReg = 0x00000800;
                regsAddrPtr->SIP_ETH_TXFIFO[idx].txFIFOPizzaArb.pizzaArbiter.pizzaArbiterStatusReg = 0x00000804;

            }/*end of unit pizzaArbiter */


        }/*end of unit txFIFOPizzaArb */


        {/*start of unit txFIFOLinkedLists */
            {/*start of unit payloadLinkedList */
                {/*start of unit payloadLinkListRAM */
                    regsAddrPtr->SIP_ETH_TXFIFO[idx].txFIFOLinkedLists.payloadLinkedList.payloadLinkListRAM.payloadLinkListAddr = 0x00054000;

                }/*end of unit payloadLinkListRAM */


            }/*end of unit payloadLinkedList */


            {/*start of unit headerLinkedList */
                {/*start of unit headerLinkListRAM */
                    regsAddrPtr->SIP_ETH_TXFIFO[idx].txFIFOLinkedLists.headerLinkedList.headerLinkListRAM.headerLinkListAddr = 0x00022000;

                }/*end of unit headerLinkListRAM */


            }/*end of unit headerLinkedList */


        }/*end of unit txFIFOLinkedLists */


        {/*start of unit txFIFOInterrupts */
            regsAddrPtr->SIP_ETH_TXFIFO[idx].txFIFOInterrupts.txFIFOGeneralCauseReg1 = 0x00000400;
            regsAddrPtr->SIP_ETH_TXFIFO[idx].txFIFOInterrupts.txFIFOGeneralMaskReg1 = 0x00000404;
            regsAddrPtr->SIP_ETH_TXFIFO[idx].txFIFOInterrupts.shifterSOPEOPFIFONotReadyInterruptCauseReg1 = 0x00000408;
            regsAddrPtr->SIP_ETH_TXFIFO[idx].txFIFOInterrupts.shifterSOPEOPFIFONotReadyInterruptMaskReg1 = 0x0000040c;
            regsAddrPtr->SIP_ETH_TXFIFO[idx].txFIFOInterrupts.shifterSOPEOPFIFONotReadyInterruptCauseReg2 = 0x00000410;
            regsAddrPtr->SIP_ETH_TXFIFO[idx].txFIFOInterrupts.shifterSOPEOPFIFONotReadyInterruptMaskReg2 = 0x00000414;
            regsAddrPtr->SIP_ETH_TXFIFO[idx].txFIFOInterrupts.shifterSOPEOPFIFONotReadyInterruptCauseReg3 = 0x00000418;
            regsAddrPtr->SIP_ETH_TXFIFO[idx].txFIFOInterrupts.shifterSOPEOPFIFONotReadyInterruptMaskReg3 = 0x0000041c;
            regsAddrPtr->SIP_ETH_TXFIFO[idx].txFIFOInterrupts.CTByteCountArrivedLateInterruptCauseReg1 = 0x00000420;
            regsAddrPtr->SIP_ETH_TXFIFO[idx].txFIFOInterrupts.CTByteCountArrivedLateInterruptMaskReg1 = 0x00000424;
            regsAddrPtr->SIP_ETH_TXFIFO[idx].txFIFOInterrupts.CTByteCountArrivedLateInterruptCauseReg2 = 0x00000428;
            regsAddrPtr->SIP_ETH_TXFIFO[idx].txFIFOInterrupts.CTByteCountArrivedLateInterruptMaskReg2 = 0x0000042c;
            regsAddrPtr->SIP_ETH_TXFIFO[idx].txFIFOInterrupts.CTByteCountArrivedLateInterruptCauseReg3 = 0x00000430;
            regsAddrPtr->SIP_ETH_TXFIFO[idx].txFIFOInterrupts.CTByteCountArrivedLateInterruptMaskReg3 = 0x00000434;
            regsAddrPtr->SIP_ETH_TXFIFO[idx].txFIFOInterrupts.txFIFOSummaryCauseReg = 0x00000440;
            regsAddrPtr->SIP_ETH_TXFIFO[idx].txFIFOInterrupts.txFIFOSummaryMaskReg = 0x00000444;

        }/*end of unit txFIFOInterrupts */


        {/*start of unit txFIFOGlobalConfig */
            {/*00000008+p*0x4*/
                GT_U32    p;
                for(p = 0 ; p <= 72 ; p++) {
                    regsAddrPtr->SIP_ETH_TXFIFO[idx].txFIFOGlobalConfig.SCDMAPayloadThreshold[p] =
                        0x00000008+p*0x4;
                }/* end of loop p */
            }/*00000008+p*0x4*/
            regsAddrPtr->SIP_ETH_TXFIFO[idx].txFIFOGlobalConfig.txFIFOGlobalConfigs = 0x00000000;
            regsAddrPtr->SIP_ETH_TXFIFO[idx].txFIFOGlobalConfig.TXFIFOMetalFixReg = 0x00000004;

        }/*end of unit txFIFOGlobalConfig */


        {/*start of unit txFIFOEnd2EndECCConfigs */
            {/*00000518+0x4*i*/
                GT_U32    i;
                for(i = 0 ; i <= 0 ; i++) {
                    regsAddrPtr->SIP_ETH_TXFIFO[idx].txFIFOEnd2EndECCConfigs.dataECCStatus[i] =
                        0x00000518+0x4*i;
                }/* end of loop i */
            }/*00000518+0x4*i*/
            {/*00000508+0x4*i*/
                GT_U32    i;
                for(i = 0 ; i <= 0 ; i++) {
                    regsAddrPtr->SIP_ETH_TXFIFO[idx].txFIFOEnd2EndECCConfigs.dataECCConfig[i] =
                        0x00000508+0x4*i;
                }/* end of loop i */
            }/*00000508+0x4*i*/
            regsAddrPtr->SIP_ETH_TXFIFO[idx].txFIFOEnd2EndECCConfigs.eccErrorCntr = 0x00000528;
            regsAddrPtr->SIP_ETH_TXFIFO[idx].txFIFOEnd2EndECCConfigs.ECCFailedAddr = 0x00000538;
            regsAddrPtr->SIP_ETH_TXFIFO[idx].txFIFOEnd2EndECCConfigs.eccErrorType = 0x00000548;
            regsAddrPtr->SIP_ETH_TXFIFO[idx].txFIFOEnd2EndECCConfigs.lockErronousEvent = PRV_CPSS_SW_PTR_ENTRY_UNUSED/*0x00000552 --> Errata for bad address */;

        }/*end of unit txFIFOEnd2EndECCConfigs */


        {/*start of unit txFIFODebug */
            {/*start of unit informativeDebug */
                {/*00001b00+p*0x4*/
                    GT_U32    p;
                    for(p = 0 ; p <= 72 ; p++) {
                        regsAddrPtr->SIP_ETH_TXFIFO[idx].txFIFODebug.informativeDebug.SCDMAStatusReg6[p] =
                            0x00001b00+p*0x4;
                    }/* end of loop p */
                }/*00001b00+p*0x4*/
                {/*00001100+p*0x4*/
                    GT_U32    p;
                    for(p = 0 ; p <= 72 ; p++) {
                        regsAddrPtr->SIP_ETH_TXFIFO[idx].txFIFODebug.informativeDebug.SCDMAStatusReg1[p] =
                            0x00001100+p*0x4;
                    }/* end of loop p */
                }/*00001100+p*0x4*/
                {/*00001900+p*0x4*/
                    GT_U32    p;
                    for(p = 0 ; p <= 72 ; p++) {
                        regsAddrPtr->SIP_ETH_TXFIFO[idx].txFIFODebug.informativeDebug.SCDMAStatusReg5[p] =
                            0x00001900+p*0x4;
                    }/* end of loop p */
                }/*00001900+p*0x4*/
                {/*00001700+p*0x4*/
                    GT_U32    p;
                    for(p = 0 ; p <= 72 ; p++) {
                        regsAddrPtr->SIP_ETH_TXFIFO[idx].txFIFODebug.informativeDebug.SCDMAStatusReg4[p] =
                            0x00001700+p*0x4;
                    }/* end of loop p */
                }/*00001700+p*0x4*/
                {/*00001500+p*0x4*/
                    GT_U32    p;
                    for(p = 0 ; p <= 72 ; p++) {
                        regsAddrPtr->SIP_ETH_TXFIFO[idx].txFIFODebug.informativeDebug.SCDMAStatusReg3[p] =
                            0x00001500+p*0x4;
                    }/* end of loop p */
                }/*00001500+p*0x4*/
                {/*00001300+p*0x4*/
                    GT_U32    p;
                    for(p = 0 ; p <= 72 ; p++) {
                        regsAddrPtr->SIP_ETH_TXFIFO[idx].txFIFODebug.informativeDebug.SCDMAStatusReg2[p] =
                            0x00001300+p*0x4;
                    }/* end of loop p */
                }/*00001300+p*0x4*/
                regsAddrPtr->SIP_ETH_TXFIFO[idx].txFIFODebug.informativeDebug.debugBusCtrlReg = 0x00001000;
                regsAddrPtr->SIP_ETH_TXFIFO[idx].txFIFODebug.informativeDebug.txFIFODebugBus = 0x00001004;
                regsAddrPtr->SIP_ETH_TXFIFO[idx].txFIFODebug.informativeDebug.LLEOPCntr = 0x0000100c;
                regsAddrPtr->SIP_ETH_TXFIFO[idx].txFIFODebug.informativeDebug.incomingHeadersCntr = 0x00001008;
                regsAddrPtr->SIP_ETH_TXFIFO[idx].txFIFODebug.informativeDebug.headerIDDBFreeIDFIFOFillLevel = 0x00001010;
                regsAddrPtr->SIP_ETH_TXFIFO[idx].txFIFODebug.informativeDebug.payloadIDDBFreeIDFIFOFillLevel = 0x00001014;
                regsAddrPtr->SIP_ETH_TXFIFO[idx].txFIFODebug.informativeDebug.CTBCIDDBFreeIDFIFOFillLevel = 0x00001018;

            }/*end of unit informativeDebug */


            {/*start of unit activeDebug */
                {/*00003100+0x4*p*/
                    GT_U32    p;
                    for(p = 0 ; p <= 72 ; p++) {
                        regsAddrPtr->SIP_ETH_TXFIFO[idx].txFIFODebug.activeDebug.debugSCDMA[p] =
                            0x00003100+0x4*p;
                    }/* end of loop p */
                }/*00003100+0x4*p*/
                regsAddrPtr->SIP_ETH_TXFIFO[idx].txFIFODebug.activeDebug.globalActiveDebugReg = 0x00003000;
                regsAddrPtr->SIP_ETH_TXFIFO[idx].txFIFODebug.activeDebug.globalActiveDebugConfReg = 0x00003004;
                regsAddrPtr->SIP_ETH_TXFIFO[idx].txFIFODebug.activeDebug.debugSCDMAConfReg = 0x00003300;

            }/*end of unit activeDebug */


        }/*end of unit txFIFODebug */


    }/*end of unit SIP_ETH_TXFIFO */
}


/**
* @internal ilknTxFifoInit function
* @endinternal
*
* @brief   init the DB - ilkn Tx Fifo unit
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None
*/
static void ilknTxFifoInit
(
    IN  GT_U8 devNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    {/*start of unit SIP_ILKN_TXFIFO */
        {/*start of unit ilknTxFIFOInterrupts */
            regsAddrPtr->SIP_ILKN_TXFIFO.ilknTxFIFOInterrupts.ilknTxfifoGeneralInterruptCause = 0x00000020;
            regsAddrPtr->SIP_ILKN_TXFIFO.ilknTxFIFOInterrupts.ilknTxfifoGeneralInterruptMask = 0x00000030;
            regsAddrPtr->SIP_ILKN_TXFIFO.ilknTxFIFOInterrupts.streamIDDataMemFullIlknTxfifoInterruptCauseReg1 = 0x00000040;
            regsAddrPtr->SIP_ILKN_TXFIFO.ilknTxFIFOInterrupts.streamIDDataMemFullIlknTxfifoInterruptMaskReg1 = 0x00000044;
            regsAddrPtr->SIP_ILKN_TXFIFO.ilknTxFIFOInterrupts.streamIDDescMemFullIlknTxfifoInterruptCauseReg1 = 0x00000048;
            regsAddrPtr->SIP_ILKN_TXFIFO.ilknTxFIFOInterrupts.streamIDDescMemFullIlknTxfifoInterruptMaskReg1 = 0x0000004c;
            regsAddrPtr->SIP_ILKN_TXFIFO.ilknTxFIFOInterrupts.streamIDDataMemFullIlknTxfifoInterruptCauseReg2 = 0x00000050;
            regsAddrPtr->SIP_ILKN_TXFIFO.ilknTxFIFOInterrupts.streamIDDataMemFullIlknTxfifoInterruptMaskReg2 = 0x00000054;
            regsAddrPtr->SIP_ILKN_TXFIFO.ilknTxFIFOInterrupts.streamIDDescMemFullIlknTxfifoInterruptCauseReg2 = 0x00000058;
            regsAddrPtr->SIP_ILKN_TXFIFO.ilknTxFIFOInterrupts.streamIDDescMemFullIlknTxfifoInterruptMaskReg2 = 0x0000005c;
            regsAddrPtr->SIP_ILKN_TXFIFO.ilknTxFIFOInterrupts.streamIDDataMemFullIlknTxfifoInterruptCauseReg3 = 0x00000060;
            regsAddrPtr->SIP_ILKN_TXFIFO.ilknTxFIFOInterrupts.streamIDDataMemFullIlknTxfifoInterruptMaskReg3 = 0x00000064;
            regsAddrPtr->SIP_ILKN_TXFIFO.ilknTxFIFOInterrupts.streamIDDescMemFullIlknTxfifoInterruptCauseReg3 = 0x00000068;
            regsAddrPtr->SIP_ILKN_TXFIFO.ilknTxFIFOInterrupts.streamIDDescMemFullIlknTxfifoInterruptMaskReg3 = 0x0000006c;
            regsAddrPtr->SIP_ILKN_TXFIFO.ilknTxFIFOInterrupts.ilknTxfifoUnderrunInterruptCauseReg1 = 0x00000070;
            regsAddrPtr->SIP_ILKN_TXFIFO.ilknTxFIFOInterrupts.ilknTxfifoUnderrunInterruptMaskReg1 = 0x00000074;
            regsAddrPtr->SIP_ILKN_TXFIFO.ilknTxFIFOInterrupts.ilknTxfifoUnderrunInterruptCauseReg2 = 0x00000078;
            regsAddrPtr->SIP_ILKN_TXFIFO.ilknTxFIFOInterrupts.ilknTxfifoUnderrunInterruptMaskReg2 = 0x0000007c;
            regsAddrPtr->SIP_ILKN_TXFIFO.ilknTxFIFOInterrupts.ilknTxfifoUnderrunInterruptCauseReg3 = 0x00000080;
            regsAddrPtr->SIP_ILKN_TXFIFO.ilknTxFIFOInterrupts.ilknTxfifoUnderrunInterruptMaskReg3 = 0x00000084;
            regsAddrPtr->SIP_ILKN_TXFIFO.ilknTxFIFOInterrupts.ilknTxfifoSummaryCauseReg = 0x000000a0;
            regsAddrPtr->SIP_ILKN_TXFIFO.ilknTxFIFOInterrupts.ilknTxfifoSummaryMaskReg = 0x000000a4;

        }/*end of unit ilknTxFIFOInterrupts */


        {/*start of unit ilknTxFIFOGlobalConfig */
            {/*00000500+p*0x4*/
                GT_U32    p;
                for(p = 0 ; p <= 63 ; p++) {
                    regsAddrPtr->SIP_ILKN_TXFIFO.ilknTxFIFOGlobalConfig.portDynamicMemoryAlocation[p] =
                        0x00000500+p*0x4;
                }/* end of loop p */
            }/*00000500+p*0x4*/
            {/*00000200+p*0x4*/
                GT_U32    p;
                for(p = 0 ; p <= 63 ; p++) {
                    regsAddrPtr->SIP_ILKN_TXFIFO.ilknTxFIFOGlobalConfig.portTransmittingThreshold[p] =
                        0x00000200+p*0x4;
                }/* end of loop p */
            }/*00000200+p*0x4*/
            {/*00000360+p*0x4*/
                GT_U32    p;
                for(p = 0 ; p <= 63 ; p++) {
                    regsAddrPtr->SIP_ILKN_TXFIFO.ilknTxFIFOGlobalConfig.portChannelEnable[p] =
                        0x00000360+p*0x4;
                }/* end of loop p */
            }/*00000360+p*0x4*/
            regsAddrPtr->SIP_ILKN_TXFIFO.ilknTxFIFOGlobalConfig.globalConfig = 0x00000350;

        }/*end of unit ilknTxFIFOGlobalConfig */


        {/*start of unit ilknTxFIFODebug */
            {/*00006d00+p*0x4*/
                GT_U32    p;
                for(p = 0 ; p <= 63 ; p++) {
                    regsAddrPtr->SIP_ILKN_TXFIFO.ilknTxFIFODebug.dataFifoFillLevelForPort[p] =
                        0x00006d00+p*0x4;
                }/* end of loop p */
            }/*00006d00+p*0x4*/

        }/*end of unit ilknTxFIFODebug */


        {/*start of unit ilknTxFIFOArbiter */
            {/*00001510+s*0x4*/
                GT_U32    s;
                for(s = 0 ; s <= 1063 ; s++) {
                    regsAddrPtr->SIP_ILKN_TXFIFO.ilknTxFIFOArbiter.pizzaSlicePortMap[s] =
                        0x00001510+s*0x4;
                }/* end of loop s */
            }/*00001510+s*0x4*/
            regsAddrPtr->SIP_ILKN_TXFIFO.ilknTxFIFOArbiter.pizzaArbiterCtrlReg = 0x00001500;
            regsAddrPtr->SIP_ILKN_TXFIFO.ilknTxFIFOArbiter.pizzaArbiterStatusReg = 0x00001504;

        }/*end of unit ilknTxFIFOArbiter */


    }/*end of unit SIP_ILKN_TXFIFO */

}




/**
* @internal oamInit function
* @endinternal
*
* @brief   init the DB - OAM unit - ingress or egress
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] oamUnitIndex             - PRV_CPSS_DXCH_UNIT_IOAM_E / PRV_CPSS_DXCH_UNIT_EOAM_E
*                                       None
*/
static void oamInit
(
    IN  GT_U8 devNum,
    IN PRV_CPSS_DXCH_UNIT_ENT oamUnitIndex

)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_U32  index =  (oamUnitIndex == PRV_CPSS_DXCH_UNIT_IOAM_E) ? 0 : 1;
    GT_U32  isSip5_15 = PRV_CPSS_SIP_5_15_CHECK_MAC(devNum);

    {/*start of unit OAMUnit */
        regsAddrPtr->OAMUnit[index].OAMGlobalCtrl = 0x00000000;
        {/*e0+4*n*/
            GT_U32    n;
            for(n = 0 ; n <= 1 ; n++) {
                regsAddrPtr->OAMUnit[index].agingEntryGroupStatus[n] = isSip5_15 ? 0x900+4*n :
                    0xe0+4*n;
            }/* end of loop n */
        }/*e0+4*n*/
        {/*2c0+4*n*/
            GT_U32    n;
            for(n = 0 ; n <= 1 ; n++) {
                regsAddrPtr->OAMUnit[index].summaryExceptionGroupStatus[n] = isSip5_15 ? 0x800+4*n :
                    0x2c0+4*n;
            }/* end of loop n */
        }/*2c0+4*n*/
        {/*2a0+n*4*/
            GT_U32    n;
            for(n = 0 ; n <= 1 ; n++) {
                regsAddrPtr->OAMUnit[index].txPeriodExceptionGroupStatus[n] = isSip5_15 ? 0x700+4*n :
                    0x2a0+n*4;
            }/* end of loop n */
        }/*2a0+n*4*/
        regsAddrPtr->OAMUnit[index].sourceInterfaceMismatchCntr = isSip5_15 ? 0x00000a04 : 0x00000304;
        regsAddrPtr->OAMUnit[index].OAMGlobalCtrl1 = 0x00000004;
        {/*280+4*n*/
            GT_U32    n;
            for(n = 0 ; n <= 1 ; n++) {
                regsAddrPtr->OAMUnit[index].RDIStatusChangeExceptionGroupStatus[n] = isSip5_15 ? 0x600+4*n :
                    0x280+4*n;
            }/* end of loop n */
        }/*280+4*n*/
        regsAddrPtr->OAMUnit[index].RDIStatusChangeCntr = isSip5_15 ? 0x00000a14 : 0x00000314;
        regsAddrPtr->OAMUnit[index].OAMUnitInterruptMask = 0x000000f4;
        regsAddrPtr->OAMUnit[index].OAMUnitInterruptCause = 0x000000f0;
        {/*d4+n*4*/
            GT_U32    n;
            GT_U32    nMax = isSip5_15 ? 3 : 2;
            for(n = 0 ; n <= nMax ; n++) {
                regsAddrPtr->OAMUnit[index].OAMTableAccessData[n] =
                    0xd4+n*4;
            }/* end of loop n */
        }/*d4+n*4*/
        regsAddrPtr->OAMUnit[index].OAMTableAccessCtrl = 0x000000d0;
        {/*30+4*n*/
            GT_U32    n;
            for(n = 0 ; n <= 3 ; n++) {
                regsAddrPtr->OAMUnit[index].OAMLossMeasurementOpcodes[n] =
                    0x30+4*n;
            }/* end of loop n */
        }/*30+4*n*/
        {/*28+4*n*/
            GT_U32    n;
            for(n = 0 ; n <= 1 ; n++) {
                regsAddrPtr->OAMUnit[index].OAMLMCountedOpcodes[n] =
                    0x28+4*n;
            }/* end of loop n */
        }/*28+4*n*/
        {/*70+4*n*/
            GT_U32    n;
            for(n = 0 ; n <= 3 ; n++) {
                regsAddrPtr->OAMUnit[index].OAMKeepAliveOpcodes[n] =
                    0x70+4*n;
            }/* end of loop n */
        }/*70+4*n*/
        regsAddrPtr->OAMUnit[index].OAMInvalidKeepaliveExceptionConfigs = 0x00000020;
        {/*94+8*n*/
            GT_U32    n;
            for(n = 0 ; n <= 7 ; n++) {
                regsAddrPtr->OAMUnit[index].agingPeriodEntryHigh[n] =
                    0x94+8*n;
            }/* end of loop n */
        }/*94+8*n*/
        regsAddrPtr->OAMUnit[index].dualEndedLM = 0x00000084;
        regsAddrPtr->OAMUnit[index].OAMExceptionConfigs2 = 0x0000001c;
        regsAddrPtr->OAMUnit[index].OAMExceptionConfigs1 = 0x00000018;
        regsAddrPtr->OAMUnit[index].OAMExceptionConfigs = 0x00000014;
        {/*50+4*n*/
            GT_U32    n;
            for(n = 0 ; n <= 3 ; n++) {
                regsAddrPtr->OAMUnit[index].OAMDelayMeasurementOpcodes[n] =
                    0x50+4*n;
            }/* end of loop n */
        }/*50+4*n*/
        {/*240+4*n*/
            GT_U32    n;
            for(n = 0 ; n <= 1 ; n++) {
                regsAddrPtr->OAMUnit[index].MEGLevelGroupStatus[n] = isSip5_15 ? 0x400+4*n :
                    0x240+4*n;
            }/* end of loop n */
        }/*240+4*n*/
        regsAddrPtr->OAMUnit[index].MEGLevelExceptionCntr = isSip5_15 ? 0x00000a00 : 0x00000300;
        {/*260+4*n*/
            GT_U32    n;
            for(n = 0 ; n <= 1 ; n++) {
                regsAddrPtr->OAMUnit[index].sourceInterfaceGroupStatus[n] = isSip5_15 ? 0x500+4*n :
                    0x260+4*n;
            }/* end of loop n */
        }/*260+4*n*/
        regsAddrPtr->OAMUnit[index].invalidKeepaliveHashCntr = isSip5_15 ? 0x00000a08 : 0x00000308;
        {/*220+4*n*/
            GT_U32    n;
            for(n = 0 ; n <= 1 ; n++) {
                regsAddrPtr->OAMUnit[index].invalidKeepaliveGroupStatus[n] = isSip5_15 ? 0x300+4*n :
                    0x220+4*n;
            }/* end of loop n */
        }/*220+4*n*/
        regsAddrPtr->OAMUnit[index].hashBitsSelection = 0x00000024;
        {/*200+4*n*/
            GT_U32    n;
            for(n = 0 ; n <= 1 ; n++) {
                regsAddrPtr->OAMUnit[index].excessKeepaliveGroupStatus[n] =
                    0x200+4*n;
            }/* end of loop n */
        }/*200+4*n*/
        regsAddrPtr->OAMUnit[index].excessKeepaliveCntr = isSip5_15 ? 0x00000a0c : 0x0000030c;
        regsAddrPtr->OAMUnit[index].keepaliveAgingCntr = isSip5_15 ? 0x00000a10 : 0x00000310;
        {/*90+8*n*/
            GT_U32    n;
            for(n = 0 ; n <= 7 ; n++) {
                regsAddrPtr->OAMUnit[index].agingPeriodEntryLow[n] =
                    0x90+8*n;
            }/* end of loop n */
        }/*90+8*n*/
        regsAddrPtr->OAMUnit[index].txPeriodExceptionCntr = isSip5_15 ? 0x00000a18 : 0x00000318;
        regsAddrPtr->OAMUnit[index].summaryExceptionCntr = isSip5_15 ? 0x00000a1c : 0x0000031c;

    }/*end of unit OAMUnit */
}

/**
* @internal ermrkInit function
* @endinternal
*
* @brief   init the DB - ermrk unit
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None
*/
static void ermrkInit
(
    IN  GT_U8 devNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    {/*start of unit ERMRK */
        regsAddrPtr->ERMRK.ERMRKGlobalConfig = 0x00000000;
        regsAddrPtr->ERMRK.ERMRKInterruptCause = 0x00000004;
        regsAddrPtr->ERMRK.ERMRKInterruptMask = 0x00000008;
        regsAddrPtr->ERMRK.ERMRKPTPConfig = 0x00000010;
        regsAddrPtr->ERMRK.timestampingUDPChecksumMode = 0x00000014;
        regsAddrPtr->ERMRK.timestampQueueMsgType = 0x00000018;
        regsAddrPtr->ERMRK.timestampQueueEntryID = 0x0000001c;
        regsAddrPtr->ERMRK.PTPNTPOffset = 0x0000010c;
        {/*40+s*4*/
            GT_U32    s;
            for(s = 0 ; s <= 11 ; s++) {
                regsAddrPtr->ERMRK.ERMRKUP2UPMapTable[s] =
                    0x40+s*4;
            }/* end of loop s */
        }/*40+s*4*/
        regsAddrPtr->ERMRK.ERMRKPTPInvalidTimestampCntr = 0x00000020;
        regsAddrPtr->ERMRK.ERMRKDP2CFITable = 0x00000038;
        {/*70+s*4*/
            GT_U32    s;
            for(s = 0 ; s <= 11 ; s++) {
                regsAddrPtr->ERMRK.ERMRKEXP2EXPMapTable[s] =
                    0x70+s*4;
            }/* end of loop s */
        }/*70+s*4*/
        {
            {/*110+4*q*/
                GT_U32    q;
                for(q = 0 ; q <= 3 ; q++) {
                    regsAddrPtr->ERMRK.timestampQueueEntryWord0[q] = 0x110+4*q;
                }/* end of loop q */
            }/*110+4*q*/
            {/*120+4*q*/
                GT_U32    q;
                for(q = 0 ; q <= 3 ; q++) {
                    regsAddrPtr->ERMRK.timestampQueueEntryWord1[q] = 0x120+4*q;
                }/* end of loop q */
            }/*120+4*q*/
            {/*130+4*q*/
                GT_U32    q;
                for(q = 0 ; q <= 1 ; q++) {
                    regsAddrPtr->ERMRK.timestampQueueEntryWord2[q] = 0x130+4*q;
                }/* end of loop q */
            }/*130+4*q*/
        }
        regsAddrPtr->ERMRK.timestampQueueEntryIDClear = 0x000000e4;
        regsAddrPtr->ERMRK.ECCErrorInjection = 0x00000024;
    }/*end of unit ERMRK */

    if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
    {
        regsAddrPtr->ERMRK.ERMRKBadAddrLatch = 0x00000028;
    }
}
/**
* @internal bmInit function
* @endinternal
*
* @brief   init the DB - bm unit
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None
*/
static void bmInit
(
    IN  GT_U8 devNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_U32  num_BMBufferTermination;
    GT_U32  num_rxDMANextUpdatesFIFOMaxFillLevel;
    GT_U32  num_rxDMANextUpdatesFIFOFillLevel;
    GT_U32  num_SCDMAInProgRegs;


    if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum)) /* bobk*/
    {
        num_BMBufferTermination                 = 4     +1;/*same as bc2*/
        num_rxDMANextUpdatesFIFOMaxFillLevel    = 3     +1;/*2 less than bc2*/
        num_rxDMANextUpdatesFIFOFillLevel       = 3     +1;/*2 less than bc2*/
        num_SCDMAInProgRegs                     = 147   +1;/*much more than bc2*/
    }
    else /* bobcat2 */
    if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum)) /* bobcat2 B0*/
    {
        num_BMBufferTermination                 = 4     +1;/*same as bc2*/
        num_rxDMANextUpdatesFIFOMaxFillLevel    = 3     +1;/*2 less than bc2 A0*/
        num_rxDMANextUpdatesFIFOFillLevel       = 3     +1;/*2 less than bc2 A0*/
        num_SCDMAInProgRegs                     = 113   +1;
    }
    else /* bobcat2 */
    {
        num_BMBufferTermination                 = 4     +1;
        num_rxDMANextUpdatesFIFOMaxFillLevel    = 5     +1;
        num_rxDMANextUpdatesFIFOFillLevel       = 5     +1;
        num_SCDMAInProgRegs                     = 113   +1;
    }


    {/*start of unit BM */
        {/*start of unit BMInterrupts */
            regsAddrPtr->BM.BMInterrupts.BMGeneralCauseReg1 = 0x00000300;
            regsAddrPtr->BM.BMInterrupts.BMGeneralMaskReg1 = 0x00000304;
            regsAddrPtr->BM.BMInterrupts.BMGeneralCauseReg2 = 0x00000308;
            regsAddrPtr->BM.BMInterrupts.BMGeneralMaskReg2 = 0x0000030c;
            regsAddrPtr->BM.BMInterrupts.BMSummaryCauseReg = 0x00000310;
            regsAddrPtr->BM.BMInterrupts.BMSummaryMaskReg = 0x00000314;

        }/*end of unit BMInterrupts */


        {/*start of unit BMGlobalConfigs */
            {/*00000050+0x4*i*/
                GT_U32    i;
                for(i = 0 ; i < num_BMBufferTermination ; i++) {
                    regsAddrPtr->BM.BMGlobalConfigs.BMBufferTermination[i] =
                        0x00000050+0x4*i;
                }/* end of loop i */
            }/*00000050+0x4*i*/
            regsAddrPtr->BM.BMGlobalConfigs.BMGlobalConfig1 = 0x00000000;
            regsAddrPtr->BM.BMGlobalConfigs.BMGlobalConfig2 = 0x00000004;
            regsAddrPtr->BM.BMGlobalConfigs.BMPauseConfig = 0x00000010;
            regsAddrPtr->BM.BMGlobalConfigs.BMBufferLimitConfig1 = 0x00000014;
            regsAddrPtr->BM.BMGlobalConfigs.BMBufferLimitConfig2 = 0x00000018;
            regsAddrPtr->BM.BMGlobalConfigs.BMMetalFixReg = 0x00000020;
            /* Aldrin, AC3X, Bobcat3 */
            if (PRV_CPSS_SIP_5_16_CHECK_MAC(devNum))
            {
                regsAddrPtr->BM.BMGlobalConfigs.BMGlobalConfig3 = 0x00000008;
            }

        }/*end of unit BMGlobalConfigs */


        {/*start of unit BMDebug */
            {/*00000480+c*0x4*/
                GT_U32    c;
                for(c = 0 ; c < num_rxDMANextUpdatesFIFOMaxFillLevel ; c++) {
                    regsAddrPtr->BM.BMDebug.rxDMANextUpdatesFIFOMaxFillLevel[c] =
                        0x00000480+c*0x4;
                }/* end of loop c */
            }/*00000480+c*0x4*/
            {/*00000460+c*0x4*/
                GT_U32    c;
                for(c = 0 ; c < num_rxDMANextUpdatesFIFOFillLevel ; c++) {
                    regsAddrPtr->BM.BMDebug.rxDMANextUpdatesFIFOFillLevel[c] =
                        0x00000460+c*0x4;
                }/* end of loop c */
            }/*00000460+c*0x4*/
            {/*00000500+s*0x4*/
                GT_U32    s;
                for(s = 0 ; s < num_SCDMAInProgRegs ; s++) {
                    regsAddrPtr->BM.BMDebug.SCDMAInProgRegs[s] =
                        0x00000500+s*0x4;
                }/* end of loop s */
            }/*00000500+s*0x4*/

        }/*end of unit BMDebug */


        {/*start of unit BMCntrs */
            regsAddrPtr->BM.BMCntrs.BMGlobalBufferCntr = 0x00000200;
            regsAddrPtr->BM.BMCntrs.rxDMAsAllocationsCntr = 0x00000204;
            regsAddrPtr->BM.BMCntrs.BMFreeBuffersInsideBMCntr = 0x00000208;
            regsAddrPtr->BM.BMCntrs.BMRxDMAPendingAllocationCntr = 0x0000020c;
            regsAddrPtr->BM.BMCntrs.BMCTClearsInsideBMCntr = 0x00000210;
            regsAddrPtr->BM.BMCntrs.incomingCTClearsCntr = 0x00000214;
            regsAddrPtr->BM.BMCntrs.incomingRegularClearsCntr = 0x00000218;
            regsAddrPtr->BM.BMCntrs.nextTableAccessCntr = 0x0000021c;
            regsAddrPtr->BM.BMCntrs.nextTableWriteAccessCntr = 0x00000220;
            regsAddrPtr->BM.BMCntrs.BMTerminatedBuffersCntr = 0x00000224;
            regsAddrPtr->BM.BMCntrs.BMLostClearRequestsCntr = 0x00000228;

        }/*end of unit BMCntrs */


    }/*end of unit BM */

}


/**
* @internal gopTaiInit function
* @endinternal
*
* @brief   init the DB - TAI unit
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] gopIndex                 - index of GOP
* @param[in] taiIndex                 - index of TAI inside of GOP
*                                       None
*/
static void gopTaiInit
(
    IN  GT_U8  devNum,
    IN  GT_U32 gopIndex,
    IN  GT_U32 taiIndex
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    /*start of unit TAI */
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].TAIInterruptCause = 0x00000000;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].TAIInterruptMask = 0x00000004;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].TAICtrlReg0 = 0x00000008;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].TAICtrlReg1 = 0x0000000c;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].timeCntrFunctionConfig0 = 0x00000010;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].timeCntrFunctionConfig1 = 0x00000014;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].timeCntrFunctionConfig2 = 0x00000018;
    if(!PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        /* TODStepNanoConfigHigh deprecated starting from Caelum (BobK) */
        regsAddrPtr->GOP.TAI[gopIndex][taiIndex].TODStepNanoConfigHigh = 0x0000001c;
    }
    else
    {
        /* frequencyAdjustTimeWindow added starting from Caelum (BobK) */
        regsAddrPtr->GOP.TAI[gopIndex][taiIndex].frequencyAdjustTimeWindow = 0x0000001c;
    }
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].TODStepNanoConfigLow = 0x00000020;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].TODStepFracConfigHigh = 0x00000024;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].TODStepFracConfigLow = 0x00000028;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].timeAdjustmentPropagationDelayConfigHigh = 0x0000002c;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].timeAdjustmentPropagationDelayConfigLow = 0x00000030;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].triggerGenerationTODSecHigh = 0x00000034;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].triggerGenerationTODSecMed = 0x00000038;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].triggerGenerationTODSecLow = 0x0000003c;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].triggerGenerationTODNanoHigh = 0x00000040;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].triggerGenerationTODNanoLow = 0x00000044;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].triggerGenerationTODFracHigh = 0x00000048;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].triggerGenerationTODFracLow = 0x0000004c;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].timeLoadValueSecHigh = 0x00000050;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].timeLoadValueSecMed = 0x00000054;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].timeLoadValueSecLow = 0x00000058;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].timeLoadValueNanoHigh = 0x0000005c;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].timeLoadValueNanoLow = 0x00000060;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].timeLoadValueFracHigh = 0x00000064;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].timeLoadValueFracLow = 0x00000068;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].timeCaptureValue0SecHigh = 0x0000006c;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].timeCaptureValue0SecMed = 0x00000070;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].timeCaptureValue0SecLow = 0x00000074;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].timeCaptureValue0NanoHigh = 0x00000078;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].timeCaptureValue0NanoLow = 0x0000007c;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].timeCaptureValue0FracHigh = 0x00000080;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].timeCaptureValue0FracLow = 0x00000084;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].timeCaptureValue1SecHigh = 0x00000088;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].timeCaptureValue1SecMed = 0x0000008c;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].timeCaptureValue1SecLow = 0x00000090;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].timeCaptureValue1NanoHigh = 0x00000094;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].timeCaptureValue1NanoLow = 0x00000098;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].timeCaptureValue1FracHigh = 0x0000009c;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].timeCaptureValue1FracLow = 0x000000a0;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].timeCaptureStatus = 0x000000a4;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].timeUpdateCntr = 0x000000a8;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].PClkClockCycleConfigLow = 0x000000d8;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].PClkClockCycleConfigHigh = 0x000000d4;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].incomingClockInCountingConfigLow = 0x000000f8;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].incomingClockInCountingConfigHigh = 0x000000f4;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].generateFunctionMaskSecMed = 0x000000b0;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].generateFunctionMaskSecLow = 0x000000b4;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].generateFunctionMaskSecHigh = 0x000000ac;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].generateFunctionMaskNanoLow = 0x000000bc;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].generateFunctionMaskNanoHigh = 0x000000b8;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].generateFunctionMaskFracLow = 0x000000c4;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].generateFunctionMaskFracHigh = 0x000000c0;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].externalClockPropagationDelayConfigLow = 0x000000f0;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].externalClockPropagationDelayConfigHigh = 0x000000ec;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].driftThresholdConfigLow = 0x000000e0;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].driftThresholdConfigHigh = 0x000000dc;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].driftAdjustmentConfigLow = 0x000000cc;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].driftAdjustmentConfigHigh = 0x000000c8;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].clockCycleConfigLow = 0x000000e8;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].clockCycleConfigHigh = 0x000000e4;
    regsAddrPtr->GOP.TAI[gopIndex][taiIndex].captureTriggerCntr = 0x000000d0;
}

/**
* @internal gopFcaInit function
* @endinternal
*
* @brief   init the DB - FCA unit
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] macNum                   - number of MAC unit
*                                       None
*/
static void gopFcaInit
(
    IN  GT_U8  devNum,
    IN  GT_U32 macNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    {/*0018060c+i*0x4*/
        GT_U32    i;
        for(i = 0 ; i <= 7 ; i++) {
            regsAddrPtr->GOP.FCA[macNum].LLFCDatabase[i] =
                0xc+i*0x4;
        }/* end of loop i */
    }/*0018060c+i*0x4*/
    {/*0018062c+p*0x4*/
        GT_U32    p;
        for(p = 0 ; p <= 7 ; p++) {
            regsAddrPtr->GOP.FCA[macNum].portSpeedTimer[p] =
                0x2c+p*0x4;
        }/* end of loop p */
    }/*0018062c+p*0x4*/
    {/*00180684+0x4*p*/
        GT_U32    p;
        for(p = 0 ; p <= 7 ; p++) {
            regsAddrPtr->GOP.FCA[macNum].lastSentTimer[p] =
                0x84+0x4*p;
        }/* end of loop p */
    }/*00180684+0x4*p*/
    regsAddrPtr->GOP.FCA[macNum].FCACtrl               = 0x0;
    regsAddrPtr->GOP.FCA[macNum].DBReg0                = 0x4;
    regsAddrPtr->GOP.FCA[macNum].DBReg1                = 0x8;
    regsAddrPtr->GOP.FCA[macNum].LLFCFlowCtrlWindow0   = 0x4c;
    regsAddrPtr->GOP.FCA[macNum].LLFCFlowCtrlWindow1   = 0x50;
    regsAddrPtr->GOP.FCA[macNum].MACDA0To15            = 0x54;
    regsAddrPtr->GOP.FCA[macNum].MACDA16To31           = 0x58;
    regsAddrPtr->GOP.FCA[macNum].MACDA32To47           = 0x5c;
    regsAddrPtr->GOP.FCA[macNum].MACSA0To15            = 0x60;
    regsAddrPtr->GOP.FCA[macNum].MACSA16To31           = 0x64;
    regsAddrPtr->GOP.FCA[macNum].MACSA32To47           = 0x68;
    regsAddrPtr->GOP.FCA[macNum].L2CtrlReg0            = 0x6c;
    regsAddrPtr->GOP.FCA[macNum].L2CtrlReg1            = 0x70;
    regsAddrPtr->GOP.FCA[macNum].DSATag0To15           = 0x74;
    regsAddrPtr->GOP.FCA[macNum].DSATag16To31          = 0x78;
    regsAddrPtr->GOP.FCA[macNum].DSATag32To47          = 0x7c;
    regsAddrPtr->GOP.FCA[macNum].DSATag48To63          = 0x80;
    regsAddrPtr->GOP.FCA[macNum].PFCPriorityMask       = 0x104;
    regsAddrPtr->GOP.FCA[macNum].sentPktsCntrLSb       = 0x108;
    regsAddrPtr->GOP.FCA[macNum].sentPktsCntrMSb       = 0x10c;
    regsAddrPtr->GOP.FCA[macNum].periodicCntrLSB       = 0x110;
    regsAddrPtr->GOP.FCA[macNum].periodicCntrMSB       = 0x114;

    return;
}

/**
* @internal gopPtpInit function
* @endinternal
*
* @brief   init the DB - GOP PTP unit
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] portNum                  - port number
*                                       None
*/
static void gopPtpInit
(
    IN  GT_U8  devNum,
    IN  GT_U32 portNum
)
{
    GT_U32 segOffset;
    GT_U32 localPort; 
         
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /*start of unit TSU */
        regsAddrPtr->GOP.PTP[portNum][0].PTPInterruptCause = 0x00000000;
        regsAddrPtr->GOP.PTP[portNum][0].PTPInterruptMask = 0x00000004;
        regsAddrPtr->GOP.PTP[portNum][0].PTPGeneralCtrl = 0x00000008;
                                     
        /* SIP_6 use one register for[0] NTP Offset */
        regsAddrPtr->GOP.PTP[portNum][0].NTPPTPOffsetLow = 0x0000000c;
        regsAddrPtr->GOP.PTP[portNum][0].txPipeStatusDelay = 0x00000010;
                                     
        /* SIP_6 use two registers fo[0]r queue entry read */
        regsAddrPtr->GOP.PTP[portNum][0].PTPTXTimestampQueue0Reg0 = 0x00000024;
        regsAddrPtr->GOP.PTP[portNum][0].PTPTXTimestampQueue0Reg1 = 0x00000028;
        regsAddrPtr->GOP.PTP[portNum][0].PTPTXTimestampQueue1Reg0 = 0x0000002c;
        regsAddrPtr->GOP.PTP[portNum][0].PTPTXTimestampQueue1Reg1 = 0x00000030;

        regsAddrPtr->GOP.PTP[portNum][0].totalPTPPktsCntr = 0x00000220;

        regsAddrPtr->GOP.PTP[portNum][0].egressTimestampQueue = 0x00000020;

        localPort = portNum % 8;

        /* 200G/400G ports */
        if((localPort == 0) || (localPort == 4))
        {
            /* Copy relative offsets for regular ports to SEG ports */
            regsAddrPtr->GOP.PTP[portNum][1] = regsAddrPtr->GOP.PTP[portNum][0];

            /* Calculate offset for 200/400G ports  */
            segOffset = (localPort == 0 ) ? 0x4000 * 8 /* localPort[0] + 8 --> index 8 */ : 
                                            0x4000 * 5 /* localPort[4] + 5 --> index 9 */ ;
            /*start of unit TSU */
            regsAddrPtr->GOP.PTP[portNum][1].PTPInterruptCause += segOffset;
            regsAddrPtr->GOP.PTP[portNum][1].PTPInterruptMask += segOffset;
            regsAddrPtr->GOP.PTP[portNum][1].PTPGeneralCtrl += segOffset;

            /* SIP_6 use one register for[0] NTP Offset */
            regsAddrPtr->GOP.PTP[portNum][1].NTPPTPOffsetLow += segOffset;
            regsAddrPtr->GOP.PTP[portNum][1].txPipeStatusDelay += segOffset;

            /* SIP_6 use two registers fo[0]r queue entry read */
            regsAddrPtr->GOP.PTP[portNum][1].PTPTXTimestampQueue0Reg0 += segOffset;
            regsAddrPtr->GOP.PTP[portNum][1].PTPTXTimestampQueue0Reg1 += segOffset;
            regsAddrPtr->GOP.PTP[portNum][1].PTPTXTimestampQueue1Reg0 += segOffset;
            regsAddrPtr->GOP.PTP[portNum][1].PTPTXTimestampQueue1Reg1 += segOffset;

            regsAddrPtr->GOP.PTP[portNum][1].totalPTPPktsCntr += segOffset;

            regsAddrPtr->GOP.PTP[portNum][1].egressTimestampQueue += segOffset;
        }
    }
    else
    {
        /*start of unit PTP */
        regsAddrPtr->GOP.PTP[portNum][0].PTPInterruptCause = 0x00000000;
        regsAddrPtr->GOP.PTP[portNum][0].PTPInterruptMask = 0x00000004;
        regsAddrPtr->GOP.PTP[portNum][0].PTPGeneralCtrl = 0x00000008;
        regsAddrPtr->GOP.PTP[portNum][0].PTPTXTimestampQueue0Reg0 = 0x0000000c;
        regsAddrPtr->GOP.PTP[portNum][0].PTPTXTimestampQueue0Reg1 = 0x00000010;
        regsAddrPtr->GOP.PTP[portNum][0].PTPTXTimestampQueue0Reg2 = 0x00000014;
        regsAddrPtr->GOP.PTP[portNum][0].PTPTXTimestampQueue1Reg0 = 0x00000018;
        regsAddrPtr->GOP.PTP[portNum][0].PTPTXTimestampQueue1Reg1 = 0x0000001c;
        regsAddrPtr->GOP.PTP[portNum][0].PTPTXTimestampQueue1Reg2 = 0x00000020;
        regsAddrPtr->GOP.PTP[portNum][0].totalPTPPktsCntr = 0x00000024;
        regsAddrPtr->GOP.PTP[portNum][0].PTPv1PktCntr = 0x00000028;
        regsAddrPtr->GOP.PTP[portNum][0].PTPv2PktCntr = 0x0000002c;
        regsAddrPtr->GOP.PTP[portNum][0].Y1731PktCntr = 0x00000030;
        regsAddrPtr->GOP.PTP[portNum][0].NTPTsPktCntr = 0x00000034;
        regsAddrPtr->GOP.PTP[portNum][0].NTPReceivePktCntr = 0x00000038;
        regsAddrPtr->GOP.PTP[portNum][0].NTPTransmitPktCntr = 0x0000003c;
        regsAddrPtr->GOP.PTP[portNum][0].WAMPPktCntr = 0x00000040;
        regsAddrPtr->GOP.PTP[portNum][0].addCorrectedTimeActionPktCntr = 0x00000058;
        regsAddrPtr->GOP.PTP[portNum][0].NTPPTPOffsetHigh = 0x00000070;
        regsAddrPtr->GOP.PTP[portNum][0].noneActionPktCntr = 0x00000044;
        regsAddrPtr->GOP.PTP[portNum][0].forwardActionPktCntr = 0x00000048;
        regsAddrPtr->GOP.PTP[portNum][0].dropActionPktCntr = 0x0000004c;
        regsAddrPtr->GOP.PTP[portNum][0].captureIngrTimeActionPktCntr = 0x0000006c;
        regsAddrPtr->GOP.PTP[portNum][0].captureAddTimeActionPktCntr = 0x0000005c;
        regsAddrPtr->GOP.PTP[portNum][0].captureAddIngrTimeActionPktCntr = 0x00000068;
        regsAddrPtr->GOP.PTP[portNum][0].captureAddCorrectedTimeActionPktCntr = 0x00000060;
        regsAddrPtr->GOP.PTP[portNum][0].captureActionPktCntr = 0x00000050;
        regsAddrPtr->GOP.PTP[portNum][0].addTimeActionPktCntr = 0x00000054;
        regsAddrPtr->GOP.PTP[portNum][0].addIngrTimeActionPktCntr = 0x00000064;
        regsAddrPtr->GOP.PTP[portNum][0].NTPPTPOffsetLow = 0x00000074;
        if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
        {
            regsAddrPtr->GOP.PTP[portNum][0].txPipeStatusDelay = 0x00000078;
        }
    }
}


/**
* @internal gopCgInit function
* @endinternal
*
* @brief   init the DB - GOP CG unit
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin;
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] portNum                  - port number
*                                       None
*/
static void gopCgInit
(
    IN  GT_U8  devNum,
    IN  GT_U32 portNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    /*start of unit CG */
    regsAddrPtr->GOP.CG[portNum].CG_CONVERTERS.CGMAControl0 = 0x340000;
    regsAddrPtr->GOP.CG[portNum].CG_CONVERTERS.CGMAControl1 = 0x340004;
    regsAddrPtr->GOP.CG[portNum].CG_CONVERTERS.CGMAConvertersStatus = 0x340020;
    regsAddrPtr->GOP.CG[portNum].CG_CONVERTERS.CGMAConvertersResets = 0x340010;
    regsAddrPtr->GOP.CG[portNum].CG_CONVERTERS.CGMAConvertersIpStatus = 0x340028;
    regsAddrPtr->GOP.CG[portNum].CG_CONVERTERS.CGMAConvertersIpStatus2 = 0x34002C;
    regsAddrPtr->GOP.CG[portNum].CG_CONVERTERS.CGMAConvertersFcControl0 = 0x340060;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CGPORTMACCommandConfig = 0x340408;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CGPORTMACaddress0 = 0x34040c;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CGPORTMACaddress1 = 0x340410;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CGPORTMACFrmLength = 0x340414;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CGPORTMACRxCrcOpt = 0x340448;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CGPORTMACStatus = 0x340440;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CGPORTMACStatNConfig = 0x340478;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CGPORTMACTxIpgLength = 0x340444;

    /* CG MIB counters */
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_IF_OUT_OCTETS_E] = 0x3404D0;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_IF_OUT_MC_PKTS_E] = 0x340510;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_IF_OUT_ERRORS_E] = 0x3404F8;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_IF_OUT_UC_PKTS_E] = 0x340508;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_IF_OUT_BC_PKTS_E] = 0x340518;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_IF_IN_OCTETS_E] = 0x3404D8;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_IF_IN_ERRORS_E] = 0x340590;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_IF_IN_BC_PKTS_E] = 0x3404F0;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_IF_IN_MC_PKTS_E] = 0x3404E8;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_IF_IN_UC_PKTS_E] = 0x3404E0;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_FRAMES_TRANSMITTED_OK_E] = 0x340480;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_FRAMES_RECIEVED_OK_E] = 0x340488;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_FRAME_CHECK_SEQUENCE_ERRORS_E] = 0x340490;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_MAC_CONTROL_FRAMES_TRANSMITTED_E] = 0x340618;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_MAC_CONTROL_FRAMES_RECIEVED_E] = 0x340620;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_PAUSE_MAC_CTRL_FRAMES_TRANSMITTED_E] = 0x3404A0;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_PAUSE_MAC_CTRL_FRAMES_RECIEVED_E] = 0x3404A8;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_CBFC_PAUSE_FRAMES_TRANSMITTED_0_E] = 0x340598;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_CBFC_PAUSE_FRAMES_TRANSMITTED_1_E] = 0x3405A0;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_CBFC_PAUSE_FRAMES_TRANSMITTED_2_E] = 0x3405A8;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_CBFC_PAUSE_FRAMES_TRANSMITTED_3_E] = 0x3405B0;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_CBFC_PAUSE_FRAMES_TRANSMITTED_4_E] = 0x3405B8;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_CBFC_PAUSE_FRAMES_TRANSMITTED_5_E] = 0x3405C0;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_CBFC_PAUSE_FRAMES_TRANSMITTED_6_E] = 0x3405C8;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_CBFC_PAUSE_FRAMES_TRANSMITTED_7_E] = 0x3405D0;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_CBFC_PAUSE_FRAMES_RECIEVED_0_E] = 0x3405D8;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_CBFC_PAUSE_FRAMES_RECIEVED_1_E] = 0x3405E0;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_CBFC_PAUSE_FRAMES_RECIEVED_2_E] = 0x3405E8;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_CBFC_PAUSE_FRAMES_RECIEVED_3_E] = 0x3405F0;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_CBFC_PAUSE_FRAMES_RECIEVED_4_E] = 0x3405F8;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_CBFC_PAUSE_FRAMES_RECIEVED_5_E] = 0x340600;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_CBFC_PAUSE_FRAMES_RECIEVED_6_E] = 0x340608;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_CBFC_PAUSE_FRAMES_RECIEVED_7_E] = 0x340610;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_ETHER_STATS_OCTETS_E] = 0x340528;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_ETHER_STATS_UNDERSIZE_PKTS_E] = 0x340538;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_ETHER_STATS_OVERSIZE_PKTS_E] = 0x340578;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_ETHER_STATS_JABBERS_E] = 0x340580;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_ETHER_STATS_DROP_EVENTS_E] = 0x340520;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_ETHER_STATS_FRAGMENTS_E] = 0x340588;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_ETHER_STATS_PKTS_E] = 0x340530;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_ETHER_STATS_PTKS_64OCTETS_E] = 0x340540;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_ETHER_STATS_PTKS_65TO127_OCTETS_E] = 0x340548;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_ETHER_STATS_PTKS_128TO255_OCTETS_E] = 0x340550;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_ETHER_STATS_PTKS_256TO511_OCTETS_E] = 0x340558;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_ETHER_STATS_PTKS_512TO1023_OCTETS_E] = 0x340560;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_ETHER_STATS_PTKS_1024TO1518_OCTETS_E] = 0x340568;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_ETHER_STATS_PTKS_1519TOMAX_OCTETS_E] = 0x340570;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_ALIGNMENT_ERRORS_E] = 0x340498;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_FRAME_TOO_LONG_ERRORS_E] = 0x3404B0;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_IN_RANGE_LENGTH_ERRORS_E] = 0x3404B8;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_VLAN_RECIEVED_OK_E] = 0x3404C8;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_VLAN_TRANSMITED_OK_E] = 0x3404C0;

}

extern GT_UREG_DATA   falconUnitBaseAddrCalc(
    GT_U8                           devNum,
    HWS_UNIT_BASE_ADDR_TYPE_ENT     unitId,
    GT_UOPT                         portNum
);

/**
* @internal gopMtiInit function
* @endinternal
*
* @brief   init the DB - GOP MSDB unit
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2;
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] portNum                  - port number
*                                       None
*/
static void gopMsdbInit
(
    IN  GT_U8  devNum,
    IN  GT_U32 portNum
)
    {
        PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
        GT_U32 portBaseAddr;
        GT_U32 i;
        portBaseAddr = falconUnitBaseAddrCalc(devNum, HWS_UNIT_BASE_ADDR_TYPE_RAVEN_MSDB_E, portNum);
        if(portBaseAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            /*start of MSDB unit */
            for(i=0;i<9;i++)
            {
                regsAddrPtr->GOP.MSDB[portNum].fcuChannelTXControl[i] = portBaseAddr + 0x00000400+i*4;
                regsAddrPtr->GOP.MSDB[portNum].fcuChannelRXControl[i] = portBaseAddr + 0x00000500+i*4;
            }
            regsAddrPtr->GOP.MSDB[portNum].fcuRXTimer = portBaseAddr + 0x00000600;
            regsAddrPtr->GOP.MSDB[portNum].fcuControl = portBaseAddr + 0x00000604;
        }
        return;
    }



/**
* @internal gopMtiInit function
* @endinternal
*
* @brief   init the DB - GOP MTI unit
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2;
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] portNum                  - port number
*                                       None
*/
static void gopMtiInit
(
    IN  GT_U8  devNum,
    IN  GT_U32 portNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_U32 portBaseAddr;
    GT_U32 localPortInUnit;
    GT_U32 segmentPortNum;

    portBaseAddr = falconUnitBaseAddrCalc(devNum, HWS_UNIT_BASE_ADDR_TYPE_RAVEN_MTI64_MAC_E, portNum);

    if(portBaseAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        /*start of MTI64 MAC unit */
        regsAddrPtr->GOP.MTI[portNum].MTI64_MAC.commandConfig = portBaseAddr + 0x8;
        regsAddrPtr->GOP.MTI[portNum].MTI64_MAC.frmLength = portBaseAddr + 0x14;
        regsAddrPtr->GOP.MTI[portNum].MTI64_MAC.macAddr0 = portBaseAddr + 0x0c ;
        regsAddrPtr->GOP.MTI[portNum].MTI64_MAC.macAddr1 = portBaseAddr + 0x10;
        regsAddrPtr->GOP.MTI[portNum].MTI64_MAC.status = portBaseAddr + 0x40;
        regsAddrPtr->GOP.MTI[portNum].MTI64_MAC.cl01PauseQuanta = portBaseAddr + 0x54;
        regsAddrPtr->GOP.MTI[portNum].MTI64_MAC.cl23PauseQuanta = portBaseAddr + 0x58;
        regsAddrPtr->GOP.MTI[portNum].MTI64_MAC.cl45PauseQuanta = portBaseAddr + 0x5c;
        regsAddrPtr->GOP.MTI[portNum].MTI64_MAC.cl67PauseQuanta = portBaseAddr + 0x60;
        regsAddrPtr->GOP.MTI[portNum].MTI64_MAC.cl01QuantaThresh = portBaseAddr + 0x64;
        regsAddrPtr->GOP.MTI[portNum].MTI64_MAC.cl23QuantaThresh = portBaseAddr + 0x68;
        regsAddrPtr->GOP.MTI[portNum].MTI64_MAC.cl45QuantaThresh = portBaseAddr + 0x6c;
        regsAddrPtr->GOP.MTI[portNum].MTI64_MAC.cl67QuantaThresh = portBaseAddr + 0x70;
        regsAddrPtr->GOP.MTI[portNum].MTI64_MAC.rxPauseStatus = portBaseAddr + 0x74;
    }

    portBaseAddr = falconUnitBaseAddrCalc(devNum, HWS_UNIT_BASE_ADDR_TYPE_RAVEN_MTI400_MAC_E, portNum);

    if(portBaseAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        /*start of MTI64 MAC unit */
        regsAddrPtr->GOP.MTI[portNum].MTI400_MAC.commandConfig = portBaseAddr + 0x8;
        regsAddrPtr->GOP.MTI[portNum].MTI400_MAC.frmLength = portBaseAddr + 0x14;
        regsAddrPtr->GOP.MTI[portNum].MTI400_MAC.macAddr0 = portBaseAddr + 0x0c;
        regsAddrPtr->GOP.MTI[portNum].MTI400_MAC.macAddr1 = portBaseAddr + 0x10;
        regsAddrPtr->GOP.MTI[portNum].MTI400_MAC.status = portBaseAddr + 0x40;
        regsAddrPtr->GOP.MTI[portNum].MTI400_MAC.cl01PauseQuanta = portBaseAddr + 0x54;
        regsAddrPtr->GOP.MTI[portNum].MTI400_MAC.cl23PauseQuanta = portBaseAddr + 0x58;
        regsAddrPtr->GOP.MTI[portNum].MTI400_MAC.cl45PauseQuanta = portBaseAddr + 0x5c;
        regsAddrPtr->GOP.MTI[portNum].MTI400_MAC.cl67PauseQuanta = portBaseAddr + 0x60;
        regsAddrPtr->GOP.MTI[portNum].MTI400_MAC.cl01QuantaThresh = portBaseAddr + 0x64;
        regsAddrPtr->GOP.MTI[portNum].MTI400_MAC.cl23QuantaThresh = portBaseAddr + 0x68;
        regsAddrPtr->GOP.MTI[portNum].MTI400_MAC.cl45QuantaThresh = portBaseAddr + 0x6c;
        regsAddrPtr->GOP.MTI[portNum].MTI400_MAC.cl67QuantaThresh = portBaseAddr + 0x70;
        regsAddrPtr->GOP.MTI[portNum].MTI400_MAC.rxPauseStatus = portBaseAddr + 0x74;
    }

    portBaseAddr = falconUnitBaseAddrCalc(devNum, HWS_UNIT_BASE_ADDR_TYPE_RAVEN_MTI_MAC_STATISTICS_E, portNum);
    if(portBaseAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regsAddrPtr->GOP.MTI_STATISTICS[portNum/8].dataHi = portBaseAddr + 0x0;
        regsAddrPtr->GOP.MTI_STATISTICS[portNum/8].status = portBaseAddr + 0x4;
        regsAddrPtr->GOP.MTI_STATISTICS[portNum/8].config = portBaseAddr + 0x8;
        regsAddrPtr->GOP.MTI_STATISTICS[portNum/8].control = portBaseAddr + 0xC;
        regsAddrPtr->GOP.MTI_STATISTICS[portNum/8].clearValueLo = portBaseAddr + 0x10;
        regsAddrPtr->GOP.MTI_STATISTICS[portNum/8].clearValueHi = portBaseAddr + 0x14;
        regsAddrPtr->GOP.MTI_STATISTICS[portNum/8].dataHiCdc = portBaseAddr + 0x1C;
        regsAddrPtr->GOP.MTI_STATISTICS[portNum/8].MTI_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_MTI_RX_ETHER_STATS_OCTETS_E] = portBaseAddr + 0x20;
        regsAddrPtr->GOP.MTI_STATISTICS[portNum/8].MTI_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_MTI_RX_OCTETS_RECEIVED_OK_E] = portBaseAddr + 0x24;
        regsAddrPtr->GOP.MTI_STATISTICS[portNum/8].MTI_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_MTI_RX_A_ALIGNMENT_ERRORS_E] = portBaseAddr + 0x28;
        regsAddrPtr->GOP.MTI_STATISTICS[portNum/8].MTI_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_MTI_RX_A_MAC_CONTROL_FRAMES_RECIEVED_E] = portBaseAddr + 0x2c;
        regsAddrPtr->GOP.MTI_STATISTICS[portNum/8].MTI_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_MTI_RX_A_FRAME_TOO_LONG_ERRORS_E] = portBaseAddr + 0x30;
        regsAddrPtr->GOP.MTI_STATISTICS[portNum/8].MTI_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_MTI_RX_A_IN_RANGE_LENGTH_ERRORS_E] = portBaseAddr + 0x34;
        regsAddrPtr->GOP.MTI_STATISTICS[portNum/8].MTI_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_MTI_RX_A_FRAMES_RECIEVED_OK_E] = portBaseAddr + 0x38;
        regsAddrPtr->GOP.MTI_STATISTICS[portNum/8].MTI_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_MTI_RX_A_FRAME_CHECK_SEQUENCE_ERRORS_E] = portBaseAddr + 0x3c;
        regsAddrPtr->GOP.MTI_STATISTICS[portNum/8].MTI_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_MTI_RX_VLAN_RECIEVED_OK_E] = portBaseAddr + 0x40;
        regsAddrPtr->GOP.MTI_STATISTICS[portNum/8].MTI_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_MTI_RX_IF_IN_ERRORS_E] = portBaseAddr + 0x44;
        regsAddrPtr->GOP.MTI_STATISTICS[portNum/8].MTI_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_MTI_RX_IF_IN_UC_PKTS_E] = portBaseAddr + 0x48;
        regsAddrPtr->GOP.MTI_STATISTICS[portNum/8].MTI_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_MTI_RX_IF_IN_MC_PKTS_E] = portBaseAddr + 0x4c;
        regsAddrPtr->GOP.MTI_STATISTICS[portNum/8].MTI_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_MTI_RX_IF_IN_BC_PKTS_E] = portBaseAddr + 0x50;
        regsAddrPtr->GOP.MTI_STATISTICS[portNum/8].MTI_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_MTI_RX_ETHER_STATS_DROP_EVENTS_E] = portBaseAddr + 0x54;
        regsAddrPtr->GOP.MTI_STATISTICS[portNum/8].MTI_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_MTI_RX_ETHER_STATS_PKTS_E] = portBaseAddr + 0x58;
        regsAddrPtr->GOP.MTI_STATISTICS[portNum/8].MTI_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_MTI_RX_ETHER_STATS_UNDERSIZE_PKTS_E] = portBaseAddr + 0x5c;
        regsAddrPtr->GOP.MTI_STATISTICS[portNum/8].MTI_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_MTI_RX_ETHER_STATS_PTKS_64OCTETS_E] = portBaseAddr + 0x60;
        regsAddrPtr->GOP.MTI_STATISTICS[portNum/8].MTI_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_MTI_RX_ETHER_STATS_PTKS_65TO127_OCTETS_E] = portBaseAddr + 0x64;
        regsAddrPtr->GOP.MTI_STATISTICS[portNum/8].MTI_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_MTI_RX_ETHER_STATS_PTKS_128TO255_OCTETS_E] = portBaseAddr + 0x68;
        regsAddrPtr->GOP.MTI_STATISTICS[portNum/8].MTI_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_MTI_RX_ETHER_STATS_PTKS_256TO511_OCTETS_E] = portBaseAddr + 0x6c;
        regsAddrPtr->GOP.MTI_STATISTICS[portNum/8].MTI_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_MTI_RX_ETHER_STATS_PTKS_512TO1023_OCTETS_E] = portBaseAddr + 0x70;
        regsAddrPtr->GOP.MTI_STATISTICS[portNum/8].MTI_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_MTI_RX_ETHER_STATS_PTKS_1024TO1518_OCTETS_E] = portBaseAddr + 0x74;
        regsAddrPtr->GOP.MTI_STATISTICS[portNum/8].MTI_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_MTI_RX_ETHER_STATS_PTKS_1519TOMAX_OCTETS_E] = portBaseAddr + 0x78;
        regsAddrPtr->GOP.MTI_STATISTICS[portNum/8].MTI_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_MTI_RX_ETHER_STATS_OVERSIZE_PKTS_E] = portBaseAddr + 0x7c;
        regsAddrPtr->GOP.MTI_STATISTICS[portNum/8].MTI_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_MTI_RX_ETHER_STATS_JABBERS_E] = portBaseAddr + 0x80;
        regsAddrPtr->GOP.MTI_STATISTICS[portNum/8].MTI_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_MTI_RX_ETHER_STATS_FRAGMENTS_E] = portBaseAddr + 0x84;
        regsAddrPtr->GOP.MTI_STATISTICS[portNum/8].MTI_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_0_E] = portBaseAddr + 0x88;
        regsAddrPtr->GOP.MTI_STATISTICS[portNum/8].MTI_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_1_E] = portBaseAddr + 0x8c;
        regsAddrPtr->GOP.MTI_STATISTICS[portNum/8].MTI_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_2_E] = portBaseAddr + 0x90;
        regsAddrPtr->GOP.MTI_STATISTICS[portNum/8].MTI_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_3_E] = portBaseAddr + 0x94;
        regsAddrPtr->GOP.MTI_STATISTICS[portNum/8].MTI_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_4_E] = portBaseAddr + 0x98;
        regsAddrPtr->GOP.MTI_STATISTICS[portNum/8].MTI_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_5_E] = portBaseAddr + 0x9c;
        regsAddrPtr->GOP.MTI_STATISTICS[portNum/8].MTI_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_6_E] = portBaseAddr + 0xa0;
        regsAddrPtr->GOP.MTI_STATISTICS[portNum/8].MTI_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_7_E] = portBaseAddr + 0xa4;
        regsAddrPtr->GOP.MTI_STATISTICS[portNum/8].MTI_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_8_E] = portBaseAddr + 0xa8;
        regsAddrPtr->GOP.MTI_STATISTICS[portNum/8].MTI_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_9_E] = portBaseAddr + 0xac;
        regsAddrPtr->GOP.MTI_STATISTICS[portNum/8].MTI_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_10_E] = portBaseAddr + 0xb0;
        regsAddrPtr->GOP.MTI_STATISTICS[portNum/8].MTI_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_11_E] = portBaseAddr + 0xb4;
        regsAddrPtr->GOP.MTI_STATISTICS[portNum/8].MTI_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_12_E] = portBaseAddr + 0xb8;
        regsAddrPtr->GOP.MTI_STATISTICS[portNum/8].MTI_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_13_E] = portBaseAddr + 0xbc;
        regsAddrPtr->GOP.MTI_STATISTICS[portNum/8].MTI_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_14_E] = portBaseAddr + 0xc0;
        regsAddrPtr->GOP.MTI_STATISTICS[portNum/8].MTI_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_15_E] = portBaseAddr + 0xc4;
        regsAddrPtr->GOP.MTI_STATISTICS[portNum/8].MTI_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_MTI_RX_A_PAUSE_MAC_CTRL_FRAMES_RECIEVED_E] = portBaseAddr + 0xc8;
    }

    portBaseAddr = falconUnitBaseAddrCalc(devNum, HWS_UNIT_BASE_ADDR_TYPE_RAVEN_MTI_EXT_E, portNum);

    if(portBaseAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        /*start of MTI_EXT unit */
        localPortInUnit = portNum % 8;
        regsAddrPtr->GOP.MTI[portNum].MTI_EXT.portControl = portBaseAddr + 0x84 + localPortInUnit*0x18;

        /* 200G/400G ports */
        if((localPortInUnit == 0) || (localPortInUnit == 4))
        {
            segmentPortNum = (localPortInUnit) ? 1 : 0;
            regsAddrPtr->GOP.MTI[portNum].MTI_EXT.segPortControl = portBaseAddr + 0x5C + segmentPortNum*0x14;
        }
    }

    return;
}


/**
* @internal prvCpssDxChCgPortDbInvalidate function
* @endinternal
*
* @brief   Invalidate or init CG port register database
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] portNum                  - CG MAC port number
* @param[in] cgUnitInvalidate         - invalidate status
*                                      GT_TRUE - invalidate DB for given MAC port
*                                      GT_FALSE - init DB for given MAC port
*                                       None
*/
GT_STATUS prvCpssDxChCgPortDbInvalidate
(
    IN  GT_U8 devNum,
    IN  GT_U32 portNum,
    IN  GT_BOOL cgUnitInvalidate
)
{
    devNum=devNum;
    portNum=portNum;
    cgUnitInvalidate=cgUnitInvalidate;
/* please talk to alex how to fix the CG problem */
#if 0
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_U32  subunitBase;
    GT_U32  indexArr[2];

    if(cgUnitInvalidate == GT_TRUE)
    {
        DESTROY_STC_MAC(regsAddrPtr->GOP.CG[portNum]);
    }
    else
    {
        /* Setting index as the port number */
        indexArr[0] = portNum;

        /* Get CG subunit base address */
        subunitBase = prvCpssDxChHwSubunitBaseAddrGet(
                devNum, PRV_CPSS_SUBUNIT_GOP_CG_E, indexArr);

        gopCgInit(devNum, portNum);

        /* Add base address to the subunit registers */
        regUnitBaseAddrSet(
            REG_UNIT_INFO_MAC(devNum, subunitBase, GOP.CG[portNum].CG_CONVERTERS));
        regUnitBaseAddrSet(
            REG_UNIT_INFO_MAC(devNum, subunitBase, GOP.CG[portNum].CG_PORT.CG_PORT_MAC));

    }
#endif
    return GT_OK;
}

/**
* @internal gopPrInit function
* @endinternal
*
* @brief   init the DB - GOP PR unit
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None
*/
static void gopPrInit
(
    IN  GT_U8  devNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr =
                                        PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    {/*start of unit PRMIB */
        regsAddrPtr->GOP.PR.PRMIB.MIBChannelSelect = 0x00001ef8;
        regsAddrPtr->GOP.PR.PRMIB.MIBGeneralConfigs = 0x00001efc;
        regsAddrPtr->GOP.PR.PRMIB.MIBWraparoundInterruptCause0 = 0x00001f00;
        regsAddrPtr->GOP.PR.PRMIB.MIBWraparoundInterruptMask0 = 0x00001f04;
        regsAddrPtr->GOP.PR.PRMIB.MIBWraparoundInterruptCause1 = 0x00001f08;
        regsAddrPtr->GOP.PR.PRMIB.MIBWraparoundInterruptMask1 = 0x00001f0c;
        regsAddrPtr->GOP.PR.PRMIB.MIBWraparoundInterruptCause2 = 0x00001f10;
        regsAddrPtr->GOP.PR.PRMIB.MIBWraparoundInterruptMask2 = 0x00001f14;

        regsAddrPtr->GOP.PR.PRMIB.MIBCountersBase = 0x00001E00;

    }/*end of unit PRMIB */


    {/*start of unit PRInterrupts */
        regsAddrPtr->GOP.PR.PRInterrupts.rxFullInterruptCause0 = 0x00001a00;
        regsAddrPtr->GOP.PR.PRInterrupts.rxFullInterruptMask0 = 0x00001a04;
        regsAddrPtr->GOP.PR.PRInterrupts.rxFullInterruptCause1 = 0x00001a08;
        regsAddrPtr->GOP.PR.PRInterrupts.rxFullInterruptMask1 = 0x00001a0c;
        regsAddrPtr->GOP.PR.PRInterrupts.rxFullInterruptCause2 = 0x00001a10;
        regsAddrPtr->GOP.PR.PRInterrupts.rxFullInterruptMask2 = 0x00001a14;
        regsAddrPtr->GOP.PR.PRInterrupts.noSOPInterruptCause0 = 0x00001a20;
        regsAddrPtr->GOP.PR.PRInterrupts.noSOPInterruptMask0 = 0x00001a24;
        regsAddrPtr->GOP.PR.PRInterrupts.noSOPInterruptCause1 = 0x00001a28;
        regsAddrPtr->GOP.PR.PRInterrupts.noSOPInterruptMask2 = 0x00001a34;
        regsAddrPtr->GOP.PR.PRInterrupts.noSOPInterruptCause2 = 0x00001a30;
        regsAddrPtr->GOP.PR.PRInterrupts.noSOPInterruptMask1 = 0x00001a2c;
        regsAddrPtr->GOP.PR.PRInterrupts.fillOverrunInterruptCause0 = 0x00001a40;
        regsAddrPtr->GOP.PR.PRInterrupts.fillOverrunInterruptMask0 = 0x00001a44;
        regsAddrPtr->GOP.PR.PRInterrupts.fillOverrunInterruptCause1 = 0x00001a48;
        regsAddrPtr->GOP.PR.PRInterrupts.fillOverrunInterruptMask1 = 0x00001a4c;
        regsAddrPtr->GOP.PR.PRInterrupts.noEOPInterruptMask2 = 0x00001aa4;
        regsAddrPtr->GOP.PR.PRInterrupts.fillOverrunInterruptMask2 = 0x00001a54;
        regsAddrPtr->GOP.PR.PRInterrupts.noEOPInterruptMask0 = 0x00001a94;
        regsAddrPtr->GOP.PR.PRInterrupts.fillUnderrunInterruptMask0 = 0x00001a64;
        regsAddrPtr->GOP.PR.PRInterrupts.fillUnderrunInterruptCause1 = 0x00001a68;
        regsAddrPtr->GOP.PR.PRInterrupts.fillUnderrunInterruptMask1 = 0x00001a6c;
        regsAddrPtr->GOP.PR.PRInterrupts.fillUnderrunInterruptCause2 = 0x00001a70;
        regsAddrPtr->GOP.PR.PRInterrupts.fillUnderrunInterruptMask2 = 0x00001a74;
        regsAddrPtr->GOP.PR.PRInterrupts.PRGeneralCauseReg = 0x00001a80;
        regsAddrPtr->GOP.PR.PRInterrupts.PRGeneralMaskReg = 0x00001a84;
        regsAddrPtr->GOP.PR.PRInterrupts.noEOPInterruptCause0 = 0x00001a90;
        regsAddrPtr->GOP.PR.PRInterrupts.fillUnderrunInterruptCause0 = 0x00001a60;
        regsAddrPtr->GOP.PR.PRInterrupts.noEOPInterruptCause1 = 0x00001a98;
        regsAddrPtr->GOP.PR.PRInterrupts.noEOPInterruptMask1 = 0x00001a9c;
        regsAddrPtr->GOP.PR.PRInterrupts.noEOPInterruptCause2 = 0x00001aa0;
        regsAddrPtr->GOP.PR.PRInterrupts.fillOverrunInterruptCause2 = 0x00001a50;
        regsAddrPtr->GOP.PR.PRInterrupts.PRSummaryCauseReg = 0x00001b00;
        regsAddrPtr->GOP.PR.PRInterrupts.PRSummaryMaskReg = 0x00001b04;

    }/*end of unit PRInterrupts */


    {/*start of unit PRFunctional */
        {/*00001808+0x4*n*/
            GT_U32    n;
            for(n = 0 ; n <= 63 ; n++) {
                regsAddrPtr->GOP.PR.PRFunctional.channelConfig[n] =
                    0x00001808+0x4*n;
            }/* end of loop n */
        }/*00001808+0x4*n*/
        regsAddrPtr->GOP.PR.PRFunctional.minMaxPkts = 0x00001800;
        regsAddrPtr->GOP.PR.PRFunctional.general = 0x00001804;

    }/*end of unit PRFunctional */

}

/**
* @internal gopIlknInit function
* @endinternal
*
* @brief   init the DB - GOP ILKN unit
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None
*/
static void gopIlknInit
(
    IN  GT_U8  devNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr =
                                        PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    {/* start of unit ILKN */
        {/*1f00140+n*0x4*/
            GT_U32    n;
            for(n = 0 ; n <= 7 ; n++)
            {
                regsAddrPtr->GOP.ILKN.ILKN_COUNT_BLK_TYPE_ERR_H[n] = 0x0000140+n*0x4;
            }/* end of loop n */
        }/*1f00140+n*0x4*/
        {/*1f00280+n*0x4*/
            GT_U32    n;
            for(n = 0 ; n <= 15 ; n++)
            {
                regsAddrPtr->GOP.ILKN.ILKN_COUNT_BLK_TYPE_ERR_L[n] = 0x0000280+n*0x4;
            }/* end of loop n */
        }/*1f00280+n*0x4*/
        regsAddrPtr->GOP.ILKN.ILKN_STAT_EN = 0x00000238;
        regsAddrPtr->GOP.ILKN.ILKN_RX_PKT_GEN = 0x00000074;
        regsAddrPtr->GOP.ILKN.ILKN_REMOTE_LANE_STATUS = 0x00000108;
        regsAddrPtr->GOP.ILKN.ILKN_PTR_RST = 0x00000070;
        regsAddrPtr->GOP.ILKN.ILKN_MAC_CFG_0 = 0x00000000;
        regsAddrPtr->GOP.ILKN.ILKN_PCS_PARAM = 0x0000023c;
        regsAddrPtr->GOP.ILKN.ILKN_MAC_CFG_1 = 0x00000008;
        regsAddrPtr->GOP.ILKN.ILKN_MAC_CFG_2 = 0x0000000c;
        regsAddrPtr->GOP.ILKN.ILKN_MAC_CFG_3 = 0x00000010;
        regsAddrPtr->GOP.ILKN.ILKN_MAC_CFG_4 = 0x00000014;
        regsAddrPtr->GOP.ILKN.ILKN_MAC_CFG_5 = 0x00000018;
        regsAddrPtr->GOP.ILKN.ILKN_PCS_DEBUG_1 = 0x00000220;
        regsAddrPtr->GOP.ILKN.ILKN_PCS_DEBUG_0 = 0x00000210;
        regsAddrPtr->GOP.ILKN.ILKN_PCS_CFG_2 = 0x00000208;
        regsAddrPtr->GOP.ILKN.ILKN_PCS_CFG_1 = 0x00000204;
        regsAddrPtr->GOP.ILKN.ILKN_PCS_CFG_0 = 0x00000200;
        regsAddrPtr->GOP.ILKN.ILKN_OVF_7 = 0x000003ac;
        regsAddrPtr->GOP.ILKN.ILKN_OVF_6 = 0x000003a8;
        regsAddrPtr->GOP.ILKN.ILKN_MAC_RL_CFG_1 = 0x00000048;
        regsAddrPtr->GOP.ILKN.ILKN_OVF_4 = 0x000003a0;
        regsAddrPtr->GOP.ILKN.ILKN_MAC_CFG_12 = 0x00000058;
        regsAddrPtr->GOP.ILKN.ILKN_CH_FC_CFG_1 = 0x00000060;
        regsAddrPtr->GOP.ILKN.ILKN_OVF_1 = 0x00000384;
        regsAddrPtr->GOP.ILKN.ILKN_OVF_0 = 0x00000380;
        regsAddrPtr->GOP.ILKN.ILKN_MAC_RL_CFG_2 = 0x00000050;
        regsAddrPtr->GOP.ILKN.ILKN_INT_CLR = 0x00000080;
        regsAddrPtr->GOP.ILKN.ILKN_MAC_RL_CFG_0 = 0x00000040;
        regsAddrPtr->GOP.ILKN.ILKN_MAC_CBFC_BM_0_1 = 0x000000c4;
        regsAddrPtr->GOP.ILKN.ILKN_MAC_CFG_9 = 0x00000030;
        regsAddrPtr->GOP.ILKN.ILKN_MAC_CFG_8 = 0x00000024;
        regsAddrPtr->GOP.ILKN.ILKN_MAC_CFG_7 = 0x00000020;
        regsAddrPtr->GOP.ILKN.ILKN_MAC_CFG_6 = 0x0000001c;
        regsAddrPtr->GOP.ILKN.ILKN_MAC_CBFC_BM_2_0 = 0x000000d8;
        regsAddrPtr->GOP.ILKN.ILKN_MAC_CBFC_BM_2_1 = 0x000000dc;
        regsAddrPtr->GOP.ILKN.ILKN_MAC_CBFC_BM_2_2 = 0x000000e0;
        regsAddrPtr->GOP.ILKN.ILKN_MAC_CBFC_BM_3_0 = 0x000000e4;
        regsAddrPtr->GOP.ILKN.ILKN_MAC_CBFC_BM_3_1 = 0x000000e8;
        regsAddrPtr->GOP.ILKN.ILKN_MAC_CFG_11 = 0x00000038;
        regsAddrPtr->GOP.ILKN.ILKN_MAC_CFG_10 = 0x00000034;
        regsAddrPtr->GOP.ILKN.ILKN_WORD_LOCK_STATUS = 0x00000100;
        regsAddrPtr->GOP.ILKN.ILKN_SYNC_LOCK_STATUS = 0x00000104;
        regsAddrPtr->GOP.ILKN.ILKN_MAC_CBFC_BM_CNTL = 0x000000f0;
        regsAddrPtr->GOP.ILKN.ILKN_MAC_CBFC_BM_3_2 = 0x000000ec;
        regsAddrPtr->GOP.ILKN.ILKN_ERR_GEN_STATUS = 0x00000110;
        regsAddrPtr->GOP.ILKN.ILKN_PCS_DEBUG_DATA = 0x00000114;
        regsAddrPtr->GOP.ILKN.ILKN_MAC_FC_STATUS = 0x00000120;
        regsAddrPtr->GOP.ILKN.ILKN_PROT_ERR_STATUS = 0x00000124;
        regsAddrPtr->GOP.ILKN.ILKN_LINK_STATUS = 0x00000128;
        regsAddrPtr->GOP.ILKN.ILKN_MAC_CBFC_BM_1_2 = 0x000000d4;
        regsAddrPtr->GOP.ILKN.ILKN_MAC_CBFC_BM_1_1 = 0x000000d0;
        regsAddrPtr->GOP.ILKN.ILKN_MAC_CBFC_BM_1_0 = 0x000000cc;
        regsAddrPtr->GOP.ILKN.ILKN_MAC_CBFC_BM_0_2 = 0x000000c8;
        regsAddrPtr->GOP.ILKN.ILKN_EN = 0x0000020c;
        regsAddrPtr->GOP.ILKN.ILKN_MAC_CBFC_BM_0_0 = 0x000000c0;
        regsAddrPtr->GOP.ILKN.ILKN_LL_FC_STATUS = 0x0000012c;
        regsAddrPtr->GOP.ILKN.ILKN_FIFO_RST = 0x00000218;
        regsAddrPtr->GOP.ILKN.ILKN_LANE_SWAP = 0x00000214;
        regsAddrPtr->GOP.ILKN.ILKN_PCS_DEBUG_2 = 0x00000224;
        regsAddrPtr->GOP.ILKN.ILKN_PCS_DEBUG_3 = 0x00000228;
        regsAddrPtr->GOP.ILKN.ILKN_PCS_DEBUG_4 = 0x0000022c;
        regsAddrPtr->GOP.ILKN.ILKN_PCS_DEBUG_5 = 0x00000230;
        {/*1f00300+n*0x4*/
            GT_U32    n;
            for(n = 0 ; n <= 15 ; n++)
            {
                regsAddrPtr->GOP.ILKN.ILKN_COUNT_WORD_SYNC_ERR_L[n] = 0x00000300+n*0x4;
            }/* end of loop n */
        }/*1f00300+n*0x4*/
        {/*1f001c0+n*0x4*/
            GT_U32    n;
            for(n = 0 ; n <= 7 ; n++)
            {
                regsAddrPtr->GOP.ILKN.ILKN_COUNT_WORD_SYNC_ERR_H[n] = 0x000001c0+n*0x4;
            }/* end of loop n */
        }/*1f001c0+n*0x4*/
        regsAddrPtr->GOP.ILKN.ILKN_COUNT_TX_PKT_LOW = 0x00000270;
        regsAddrPtr->GOP.ILKN.ILKN_COUNT_TX_PKT_HIGH = 0x00000274;
        regsAddrPtr->GOP.ILKN.ILKN_COUNT_TX_BYTE_LOW = 0x00000260;
        regsAddrPtr->GOP.ILKN.ILKN_COUNT_TX_BYTE_HIGH = 0x00000264;
        regsAddrPtr->GOP.ILKN.ILKN_COUNT_TX_BAD_PKT_LOW = 0x00000268;
        regsAddrPtr->GOP.ILKN.ILKN_COUNT_TX_BAD_PKT_HIGH = 0x0000026c;
        regsAddrPtr->GOP.ILKN.ILKN_COUNT_RX_PKT_LOW = 0x00000250;
        regsAddrPtr->GOP.ILKN.ILKN_COUNT_RX_PKT_HIGH = 0x00000254;
        regsAddrPtr->GOP.ILKN.ILKN_COUNT_RX_CRC_ERROR_LOW = 0x00000258;
        regsAddrPtr->GOP.ILKN.ILKN_COUNT_RX_CRC_ERROR_HIGH = 0x0000025c;
        regsAddrPtr->GOP.ILKN.ILKN_COUNT_RX_BYTE_LOW = 0x00000240;
        regsAddrPtr->GOP.ILKN.ILKN_COUNT_RX_BYTE_HIGH = 0x00000244;
        regsAddrPtr->GOP.ILKN.ILKN_COUNT_RX_BURST_SIZE_ERROR_L = 0x00000370;
        regsAddrPtr->GOP.ILKN.ILKN_COUNT_RX_BURST_SIZE_ERROR_H = 0x00000374;
        regsAddrPtr->GOP.ILKN.ILKN_COUNT_RX_BAD_PKT_LOW = 0x00000248;
        regsAddrPtr->GOP.ILKN.ILKN_COUNT_RX_BAD_PKT_HIGH = 0x0000024c;
        regsAddrPtr->GOP.ILKN.ILKN_COUNT_RX_ALIGNMENT_FAILURE = 0x00000344;
        regsAddrPtr->GOP.ILKN.ILKN_COUNT_RX_ALIGNMENT_ERROR = 0x00000340;
        {/*1f002c0+n*0x4*/
            GT_U32    n;
            for(n = 0 ; n <= 15 ; n++)
            {
                regsAddrPtr->GOP.ILKN.ILKN_COUNT_DIAG_CRC_ERR_L[n] = 0x000002c0+n*0x4;
            }/* end of loop n */
        }/*1f002c0+n*0x4*/
        {/*1f00180+n*0x4*/
            GT_U32    n;
            for(n = 0 ; n <= 7 ; n++)
            {
                regsAddrPtr->GOP.ILKN.ILKN_COUNT_DIAG_CRC_ERR_H[n] = 0x00000180+n*0x4;
            }/* end of loop n */
        }/*1f00180+n*0x4*/
        regsAddrPtr->GOP.ILKN.ILKN_OVF_2 = 0x00000388;
        regsAddrPtr->GOP.ILKN.ILKN_OVF_3 = 0x0000038c;
        regsAddrPtr->GOP.ILKN.ILKN_CH_FC_CFG_2 = 0x00000068;
        regsAddrPtr->GOP.ILKN.ILKN_OVF_5 = 0x000003a4;
        regsAddrPtr->GOP.ILKN.ILKN_CH_FC_CFG_0 = 0x00000004;
        regsAddrPtr->GOP.ILKN.ILKN_ALIGN_STATUS = 0x0000010c;

    }/* enf of unit ILKN */

    {/* start of unit ILKN_WRP*/
        regsAddrPtr->GOP.ILKN_WRP.sdFcLedConvertersControl0 = 0x00000810;
        regsAddrPtr->GOP.ILKN_WRP.txdmaConverterControl0    = 0x00000820;
        regsAddrPtr->GOP.ILKN_WRP.rxdmaConverterPacketSizeRestriction = 0x0000084C;
    }/* enf of unit ILKN_WRP */


    return;
}

/**
* @internal gopMacTGInit function
* @endinternal
*
* @brief   init the DB - GOP-MacTG unit register file
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None
*/
static void gopMacTGInit
(
    IN  GT_U8  devNum
)
{
    GT_U32 i;
    GT_U32 patterRegId;
    GT_U32 offset;
    GT_U32 tgIdx;
    GT_U32 portIncrement;
    GT_BOOL isDeviceSip_5_15;
    GT_BOOL isDeviceSip_5_20;

    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *addrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    isDeviceSip_5_15 = PRV_CPSS_SIP_5_15_CHECK_MAC(devNum);
    isDeviceSip_5_20 = PRV_CPSS_SIP_5_20_CHECK_MAC(devNum);

    /* used for compatibility between devices that have    */
    /* 1 packet generator per 4 MACs and devices that have */
    /* 1 packet generator per 1 MAC                        */
    portIncrement = (isDeviceSip_5_15 && (!isDeviceSip_5_20)) ? 1 : 4;

    /* MAC TG register addresses */
    for (i = 0; i < 74; i += portIncrement)
    {
        /* support macs that not exists in bobk */
        PRV_CPSS_DXCH_SKIP_NOT_EXIST_PORT_MAC(devNum, i);

        /* used for compatibility between devices that have    */
        /* 1 packet generator per 4 MACs and devices that have */
        /* 1 packet generator per 1 MAC                        */
        tgIdx = i / portIncrement;

        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
        {
            if(i < 73)/*0..72*/
            {
                hwsAldrin2GopAddrCalc(i/*port number*/,0/*relative offset*/,&offset);
                /* remove the base Address of the GOP */
                offset &= 0x00FFFFFF;
            }
            else
            {
                /* should not happen */
                continue;
            }
        }
        else
        if ((isDeviceSip_5_20) && (i >= 36))
        {
            offset = (i - 36) * 0x1000 + PRV_CPSS_DXCH_BOBCAT3_PIPE1_GOP_OFFSET_CNS;
        }
        else if ((!isDeviceSip_5_20) && (i >= 56))
        {
            offset = (i - 56) * 0x1000 + 0x200000;
        }
        else
        {
            offset = i * 0x1000;
        }

        addrPtr->packGenConfig[tgIdx].macDa[0]             = 0x00180C00 + offset;
        addrPtr->packGenConfig[tgIdx].macDa[1]             = 0x00180C04 + offset;
        addrPtr->packGenConfig[tgIdx].macDa[2]             = 0x00180C08 + offset;
        addrPtr->packGenConfig[tgIdx].macSa[0]             = 0x00180C0C + offset;
        addrPtr->packGenConfig[tgIdx].macSa[1]             = 0x00180C10 + offset;
        addrPtr->packGenConfig[tgIdx].macSa[2]             = 0x00180C14 + offset;
        addrPtr->packGenConfig[tgIdx].etherType            = 0x00180C18 + offset;
        addrPtr->packGenConfig[tgIdx].vlanTag              = 0x00180C1C + offset;
        addrPtr->packGenConfig[tgIdx].packetLength         = 0x00180C20 + offset;
        addrPtr->packGenConfig[tgIdx].packetCount          = 0x00180C24 + offset;
        addrPtr->packGenConfig[tgIdx].ifg                  = 0x00180C28 + offset;
        addrPtr->packGenConfig[tgIdx].macDaIncrementLimit  = 0x00180C2C + offset;
        addrPtr->packGenConfig[tgIdx].controlReg0          = 0x00180C40 + offset;
        for (patterRegId = 0; patterRegId < 31; patterRegId++)
        {
            addrPtr->packGenConfig[tgIdx].dataPattern[patterRegId] = 0x00180C50 + offset + 4 * patterRegId;
        }
    }
}

/**
* @internal ipInit function
* @endinternal
*
* @brief   init the DB - IP unit
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None
*/
static void ipInit
(
    IN  GT_U8 devNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_U32  isSip5_20 = PRV_CPSS_SIP_5_20_CHECK_MAC(devNum);
    GT_U32  isSip5_25 = PRV_CPSS_SIP_5_25_CHECK_MAC(devNum);
    GT_U32  isSip6 = PRV_CPSS_SIP_6_CHECK_MAC(devNum);

    {/*start of unit IPvX */
        {/*start of unit routerManagementCntrsSets */
            {/*90c+0x100*n*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    regsAddrPtr->IPvX.routerManagementCntrsSets.routerManagementInUcNonRoutedExceptionPktsCntrSet[n] =
                        0x90c+0x100*n;
                }/* end of loop n */
            }/*90c+0x100*n*/
            {/*91c+0x100*n*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    regsAddrPtr->IPvX.routerManagementCntrsSets.routerManagementInMcTrappedMirroredPktCntrSet[n] =
                        0x91c+0x100*n;
                }/* end of loop n */
            }/*91c+0x100*n*/
            {/*900+0x100*n*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    regsAddrPtr->IPvX.routerManagementCntrsSets.routerManagementInUcPktsCntrSet[n] =
                        0x900+0x100*n;
                }/* end of loop n */
            }/*900+0x100*n*/
            {/*904+0x100*n*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    regsAddrPtr->IPvX.routerManagementCntrsSets.routerManagementInMcPktsCntrSet[n] =
                        0x904+0x100*n;
                }/* end of loop n */
            }/*904+0x100*n*/
            {/*908+0x100*n*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    regsAddrPtr->IPvX.routerManagementCntrsSets.routerManagementInUcNonRoutedNonException[n] =
                        0x908+0x100*n;
                }/* end of loop n */
            }/*908+0x100*n*/
            {/*910+0x100*n*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    regsAddrPtr->IPvX.routerManagementCntrsSets.routerManagementInMcNonRoutedNonExceptionPktsCntrSet[n] =
                        0x910+0x100*n;
                }/* end of loop n */
            }/*910+0x100*n*/
            {/*914+0x100*n*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    regsAddrPtr->IPvX.routerManagementCntrsSets.routerManagementInMcNonRoutedExceptionPktsCntrSet[n] =
                        0x914+0x100*n;
                }/* end of loop n */
            }/*914+0x100*n*/
            {/*918+0x100*n*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    regsAddrPtr->IPvX.routerManagementCntrsSets.routerManagementInUcTrappedMirroredPktCntrSet[n] =
                        0x918+0x100*n;
                }/* end of loop n */
            }/*918+0x100*n*/
            {/*920+0x100*n*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    regsAddrPtr->IPvX.routerManagementCntrsSets.routerManagementInMcRPFFailCntrSet[n] =
                        0x920+0x100*n;
                }/* end of loop n */
            }/*920+0x100*n*/
            {/*924+0x100*n*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    regsAddrPtr->IPvX.routerManagementCntrsSets.routerManagementOutUcPktCntrSet[n] =
                        0x924+0x100*n;
                }/* end of loop n */
            }/*924+0x100*n*/
            {/*980+0x100*n*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    regsAddrPtr->IPvX.routerManagementCntrsSets.routerManagementCntrsSetConfig0[n] =
                        0x980+0x100*n;
                }/* end of loop n */
            }/*980+0x100*n*/
            {/*984+0x100*n*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    regsAddrPtr->IPvX.routerManagementCntrsSets.routerManagementCntrsSetConfig1[n] =
                        0x984+0x100*n;
                }/* end of loop n */
            }/*984+0x100*n*/
            if(isSip6)
            {/*988+0x100*n*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    regsAddrPtr->IPvX.routerManagementCntrsSets.routerManagementCntrsSetConfig2[n] =
                        0x988+0x100*n;
                }/* end of loop n */
            }/*988+0x100*n*/
            regsAddrPtr->IPvX.routerManagementCntrsSets.routerManagementIncomingPktCntr = 0x00000944;
            regsAddrPtr->IPvX.routerManagementCntrsSets.routerManagementOutgoingPktCntr = 0x00000948;
            if(!isSip5_20)
            {
                regsAddrPtr->IPvX.routerManagementCntrsSets.routerManagementInSIPFilterCntr = 0x00000958;
            }

        }/*end of unit routerManagementCntrsSets */


        {/*start of unit routerGlobalCtrl */
            regsAddrPtr->IPvX.routerGlobalCtrl.routerGlobalCtrl0 = 0x00000000;
            {/*4+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    regsAddrPtr->IPvX.routerGlobalCtrl.routerMTUConfigReg[n] =
                        0x4+n*0x4;
                }/* end of loop n */
            }/*4+n*0x4*/
            regsAddrPtr->IPvX.routerGlobalCtrl.routerGlobalCtrl1 = 0x00000020;
            regsAddrPtr->IPvX.routerGlobalCtrl.routerGlobalCtrl2 = 0x00000024;
            if(isSip5_25)
            {
                regsAddrPtr->IPvX.routerGlobalCtrl.routerGlobalCtrl3 = 0x00000028;
            }
        }/*end of unit routerGlobalCtrl */


        {/*start of unit routerDropCntr */
            regsAddrPtr->IPvX.routerDropCntr.routerDropCntr = 0x00000950;
            regsAddrPtr->IPvX.routerDropCntr.routerDropCntrConfig = 0x00000954;

        }/*end of unit routerDropCntr */


        {/*start of unit routerBridgedPktsExceptionCntr */
            regsAddrPtr->IPvX.routerBridgedPktsExceptionCntr.routerBridgedPktExceptionsCntr = 0x00000940;

        }/*end of unit routerBridgedPktsExceptionCntr */


        {/*start of unit IPv6Scope */
            {/*278+n*4*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    regsAddrPtr->IPvX.IPv6Scope.IPv6UcScopeTableReg[n] =
                        0x278+n*4;
                }/* end of loop n */
            }/*278+n*4*/
            {/*288+n*4*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    regsAddrPtr->IPvX.IPv6Scope.IPv6McScopeTableReg[n] =
                        0x288+n*4;
                }/* end of loop n */
            }/*288+n*4*/
            {/*250+n*4*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    regsAddrPtr->IPvX.IPv6Scope.IPv6UcScopePrefix[n] =
                        0x250+n*4;
                }/* end of loop n */
            }/*250+n*4*/
            {/*260+n*4*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    regsAddrPtr->IPvX.IPv6Scope.IPv6UcScopeLevel[n] =
                        0x260+n*4;
                }/* end of loop n */
            }/*260+n*4*/

        }/*end of unit IPv6Scope */


        {/*start of unit IPv6GlobalCtrl */
            regsAddrPtr->IPvX.IPv6GlobalCtrl.IPv6CtrlReg0 = 0x00000200;
            regsAddrPtr->IPvX.IPv6GlobalCtrl.IPv6CtrlReg1 = 0x00000204;

        }/*end of unit IPv6GlobalCtrl */


        {/*start of unit IPv4GlobalCtrl */
            regsAddrPtr->IPvX.IPv4GlobalCtrl.IPv4CtrlReg0 = 0x00000100;
            regsAddrPtr->IPvX.IPv4GlobalCtrl.IPv4CtrlReg1 = 0x00000104;

        }/*end of unit IPv4GlobalCtrl */


        {/*start of unit internalRouter */
            regsAddrPtr->IPvX.internalRouter.routerInterruptCause = 0x00000970;
            regsAddrPtr->IPvX.internalRouter.routerInterruptMask = 0x00000974;

        }/*end of unit internalRouter */


        {/*start of unit FCoEGlobalCtrl */
            regsAddrPtr->IPvX.FCoEGlobalCtrl.routerFCoEGlobalConfig = 0x00000380;

        }/*end of unit FCoEGlobalCtrl */


        {/*start of unit ECMPRoutingConfig */
            regsAddrPtr->IPvX.ECMPRoutingConfig.ECMPConfig = 0x00000360;
            regsAddrPtr->IPvX.ECMPRoutingConfig.ECMPSeed = 0x00000364;

        }/*end of unit ECMPRoutingConfig */


        {/*start of unit CPUCodes */
            regsAddrPtr->IPvX.CPUCodes.CPUCodes0 = 0x00000e00;
            regsAddrPtr->IPvX.CPUCodes.CPUCodes1 = 0x00000e04;
            regsAddrPtr->IPvX.CPUCodes.CPUCodes2 = 0x00000e08;
            regsAddrPtr->IPvX.CPUCodes.CPUCodes3 = 0x00000e0c;
            regsAddrPtr->IPvX.CPUCodes.CPUCodes4 = 0x00000e10;
            regsAddrPtr->IPvX.CPUCodes.CPUCodes5 = 0x00000e14;
            regsAddrPtr->IPvX.CPUCodes.CPUCodes6 = 0x00000e18;
            regsAddrPtr->IPvX.CPUCodes.CPUCodes7 = 0x00000e1c;
            regsAddrPtr->IPvX.CPUCodes.CPUCodes8 = 0x00000e20;
            regsAddrPtr->IPvX.CPUCodes.CPUCodes9 = 0x00000e24;
            regsAddrPtr->IPvX.CPUCodes.CPUCodes10 = 0x00000e28;
            regsAddrPtr->IPvX.CPUCodes.CPUCodes11 = 0x00000e2c;

        }/*end of unit CPUCodes */

        if(isSip6)
        {
            regsAddrPtr->IPvX.FdbIpLookup.FdbIpv4RouteLookupMask = 0x00000f00;
            {/*f04+n*4*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    regsAddrPtr->IPvX.FdbIpLookup.FdbIpv6RouteLookupMask[n] = 0x00000f04+n*4;
                }/* end of loop n */
            }/*f04+n*4*/
            regsAddrPtr->IPvX.FdbRouteLookup.FdbHostLookup = 0x00000f20;
        }

        if(isSip5_20)
        {
            /*Router FIFOs threshold 1*/
            regsAddrPtr->IPvX.routerFifoThreshold1Reg = 0x00000978;
        }
    }/*end of unit IPvX */
}

static void plrInit_sip6
(
    IN  GT_U8  devNum,
    IN  GT_U32 stageNum
);
/**
* @internal plrInit function
* @endinternal
*
* @brief   init the DB - PLR unit
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] stageNum                 - PLR stage number
*                                       None
*/
static void plrInit
(
    IN  GT_U8  devNum,
    IN  GT_U32 stageNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_U32  isSip5_20 = PRV_CPSS_SIP_5_20_CHECK_MAC(devNum);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        plrInit_sip6(devNum,stageNum);
        return;
    }

    /*start of unit PLR */
    {/*c0+n*4*/
        GT_U32    n;
        for(n = 0 ; n <= 63 ; n++) {
            regsAddrPtr->PLR[stageNum].policerInitialDP[n] =
                0xc0+n*4;
        }/* end of loop n */
    }/*c0+n*4*/
    regsAddrPtr->PLR[stageNum].policerCtrl0 = 0x00000000;
    regsAddrPtr->PLR[stageNum].policerCtrl1 = 0x00000004;
    regsAddrPtr->PLR[stageNum].globalBillingCntrIndexingModeConfig0 = 0x00000008;
    regsAddrPtr->PLR[stageNum].policerScanCtrl = 0x000000b8;
    {/*228+n*4*/
        GT_U32    n;
        GT_U32    nMax = isSip5_20 ? 15 : 7;
        for(n = 0 ; n <= nMax ; n++) {
            regsAddrPtr->PLR[stageNum].policerPortMeteringReg[n] =
                0x228+n*4;
        }/* end of loop n */
    }/*228+n*4*/
    {/*250+x*0x4*/
        GT_U32    x;
        GT_U32    nMax = isSip5_20 ? 31 : 15;
        GT_U32    offset = isSip5_20 ? 0x270 : 0x250;
        for(x = 0 ; x <= nMax ; x++) {
            regsAddrPtr->PLR[stageNum].policerPerPortRemarkTableID[x] =
                offset+x*0x4;
        }/* end of loop x */
    }/*250+x*0x4*/
    regsAddrPtr->PLR[stageNum].policerOAM = 0x00000224;
    regsAddrPtr->PLR[stageNum].policerMRU = 0x00000010;
    regsAddrPtr->PLR[stageNum].IPFIXSecondsMSbTimerStampUpload = 0x00000020;
    regsAddrPtr->PLR[stageNum].policerCountingBaseAddr = 0x00000024;
    regsAddrPtr->PLR[stageNum].policerMeteringBaseAddr = 0x00000028;
    regsAddrPtr->PLR[stageNum].policerIPFIXPktCountWAThreshold = 0x00000034;
    regsAddrPtr->PLR[stageNum].policerIPFIXDroppedPktCountWAThreshold = 0x00000030;
    regsAddrPtr->PLR[stageNum].policerIPFIXByteCountWAThresholdMSB = 0x0000003c;
    regsAddrPtr->PLR[stageNum].policerIPFIXByteCountWAThresholdLSB = 0x00000038;
    regsAddrPtr->PLR[stageNum].policerInterruptMask = 0x00000204;
    regsAddrPtr->PLR[stageNum].policerInterruptCause = 0x00000200;
    {/*74+n*4*/
        GT_U32    n;
        for(n = 0 ; n <= 7 ; n++) {
            regsAddrPtr->PLR[stageNum].policerTableAccessData[n] =
                0x74+n*4;
        }/* end of loop n */
    }/*74+n*4*/
    regsAddrPtr->PLR[stageNum].IPFIXSampleEntriesLog1 = 0x0000004c;
    regsAddrPtr->PLR[stageNum].policerError = 0x00000050;
    regsAddrPtr->PLR[stageNum].policerErrorCntr = 0x00000054;
    regsAddrPtr->PLR[stageNum].meteringAndCountingRAMSize0 = 0x00000060;
    regsAddrPtr->PLR[stageNum].meteringAndCountingRAMSize1 = 0x00000064;
    regsAddrPtr->PLR[stageNum].meteringAndCountingRAMSize2 = 0x00000068;
    regsAddrPtr->PLR[stageNum].policerTableAccessCtrl = 0x00000070;
    regsAddrPtr->PLR[stageNum].policerMeteringRefreshScanRateLimit = 0x000000b0;
    regsAddrPtr->PLR[stageNum].policerMeteringRefreshScanAddrRange = 0x000000b4;
    regsAddrPtr->PLR[stageNum].IPFIXSecondsLSbTimerStampUpload = 0x0000001c;
    if(!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {/*208+n*4*/
        GT_U32    n;
        for(n = 0 ; n <= 1 ; n++) {
            regsAddrPtr->PLR[stageNum].policerShadow[n] =
                0x208+n*4;
        }/* end of loop n */
    }/*208+n*4*/
    regsAddrPtr->PLR[stageNum].IPFIXSampleEntriesLog0 = 0x00000048;
    regsAddrPtr->PLR[stageNum].IPFIXNanoTimerStampUpload = 0x00000018;
    regsAddrPtr->PLR[stageNum].IPFIXCtrl = 0x00000014;
    regsAddrPtr->PLR[stageNum].hierarchicalPolicerCtrl = 0x0000002c;
    regsAddrPtr->PLR[stageNum].globalBillingCntrIndexingModeConfig1 = 0x0000000c;
    if(isSip5_20)
    {
        GT_U32    p;
        for(p = 0 ; p < 512 ; p ++) {
            regsAddrPtr->PLR[stageNum].portAndPktTypeTranslationTable[p] =
                0x1800+p*0x4;
        }/* end of loop p */
    }
    else
    {/*1800+p*2<perPort>*/
        GT_U32    p;
        for(p = 0 ; p <= 255 ; p += 2) {
            regsAddrPtr->PLR[stageNum].portAndPktTypeTranslationTable[p/2] =
                0x1800+(p/2)*0x4;
        }/* end of loop p */
    }/*1800+p*2*/

    regsAddrPtr->PLR[stageNum].policerManagementCntrsTbl = 0x00000500;
    regsAddrPtr->PLR[stageNum].policerCountingTbl = isSip5_20 ? 0x00200000 : 0x00140000;
    regsAddrPtr->PLR[stageNum].policerTimerTbl = 0x00000300;
    regsAddrPtr->PLR[stageNum].ipfixWrapAroundAlertTbl = 0x00000800;

    if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
    {
        regsAddrPtr->PLR[stageNum].ipfixAgingAlertTbl = 0x00001000;
        regsAddrPtr->PLR[stageNum].policerBadAddressLatch = 0x00002000;
        if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
        {
            if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
            {
                regsAddrPtr->PLR[stageNum].meteringAddressingModeCfg0 = 0x00000040;
                regsAddrPtr->PLR[stageNum].meteringAddressingModeCfg1 = 0x00000044;
            }
            else
            {
                regsAddrPtr->PLR[stageNum].meteringAddressingModeCfg0 = 0x00000290;
                regsAddrPtr->PLR[stageNum].meteringAddressingModeCfg1 = 0x00000294;
            }

            {/*2100+(p*4)*/
                GT_U32    p;
                for (p = 0 ; (p <= 127); p++) {
                    regsAddrPtr->PLR[stageNum].mef10_3_bucket0MaxRateArr[p] =
                        0x2100 + (p * 4);
                }/* end of loop p */
            }/*2100+(p*4)*/
            {/*2300+(p*4)*/
                GT_U32    p;
                for (p = 0 ; (p <= 127); p++) {
                    regsAddrPtr->PLR[stageNum].mef10_3_bucket1MaxRateArr[p] =
                        0x2300 + (p * 4);
                }/* end of loop p */
            }/*2300+(p*4)*/
            {/*2500+(p*4)*/
                GT_U32    p;
                for (p = 0 ; (p <= 127); p++) {
                    regsAddrPtr->PLR[stageNum].qosProfile2PriorityMapArr[p] =
                        0x2500 + (p * 4);
                }/* end of loop p */
            }/*2500+(p*4)*/

            {/*3550+(p*4) - added manually */
                GT_U32    p;
                for (p = 0 ; (p <= 1); p++) {
                    regsAddrPtr->PLR[stageNum].debug_LFSRSeed[p] =
                        0x3550 + (p * 4);
                }
            }/*3550+(p*4) - added manually */

            regsAddrPtr->PLR[stageNum].debug_PrngControl1 =  0x00003560;
            if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
            {
                regsAddrPtr->PLR[stageNum].debug_PrngControl2 =  0x00003564;
            }

        }
    }
    else
    {
        regsAddrPtr->PLR[stageNum].ipfixAgingAlertTbl = 0x00000900;
    }

    if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        {/*0002040+4*t*/
            GT_U32    t;
            for(t = 0 ; t <= 6 ; t++) {
                regsAddrPtr->PLR[stageNum].globalConfig.countingConfigTableThreshold[t] =
                    0x0002040+4*t;
            }/* end of loop t */
        }/*0002040+4*t*/
        {/*0002020+4*n*/
            GT_U32    n;
            for(n = 0 ; n <= 7 ; n++) {
                regsAddrPtr->PLR[stageNum].globalConfig.countingConfigTableSegment[n] =
                    0x0002020+4*n;
            }/* end of loop n */
        }/*0002020+4*n*/

        regsAddrPtr->PLR[stageNum].globalConfig.statisticalCountingConfig0 = 0x00002010;
        regsAddrPtr->PLR[stageNum].globalConfig.statisticalCountingConfig1 = 0x00002014;

    }

    /*end of unit PLR */
}

/**
* @internal cncInit function
* @endinternal
*
* @brief   init the DB - CNC unit
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] instance                 - CNC  number
*                                       None
*/
static void cncInit
(
    IN  GT_U8  devNum,
    IN  GT_U32 instance
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_U32  isSip5_20 = PRV_CPSS_SIP_5_20_CHECK_MAC(devNum);

    /* unit perBlockRegs */

    /* unit wraparound */
    {/*00001498+n*0x10+m*0x4*/
        GT_U32    n,m;

        for(n = 0; n <= 15; n++)
        {
            for(m = 0; m <= 3; m++)
            {
                regsAddrPtr->CNC[instance].perBlockRegs.
                    wraparound.CNCBlockWraparoundStatusReg[n][m] =
                    0x00001498 + (n * 0x10) + (m * 0x4);
            }/* end of loop m */
        }/* end of loop n */

    }/*00001498+n*0x10+m*0x4*/

    /* unit rangeCount */
    {/*00001c98+0x10*n+j*0x4*/
        GT_U32    j,n;

        for(j = 0; j <= 1; j++)
        {
            for(n = 0; n <= 15; n++)
            {
                regsAddrPtr->CNC[instance].perBlockRegs.
                    rangeCount.CNCBlockRangeCountEnable[j][n] =
                    0x00001c98 + (0x10 * n) + (j * 0x4);
            }/* end of loop n */
        }/* end of loop j */

    }/*00001c98+0x10*n+j*0x4*/

    /* unit clientEnable */
    {/*00001298+n*0x4*/
        GT_U32    n;

        for(n = 0; n <= 15; n++)
        {
            regsAddrPtr->CNC[instance].perBlockRegs.
                clientEnable.CNCBlockConfigReg0[n] =
                0x00001298 + (n * 0x4);
        }/* end of loop n */

    }/*00001298+n*0x4*/

    /* unit entryMode */
    if(isSip5_20)
    {/*00001398+n*0x4*/
        GT_U32    n;

        for(n = 0; n <= 1; n++)
        {
            regsAddrPtr->CNC[instance].perBlockRegs.
                entryMode.CNCBlocksCounterEntryModeRegister[n] =
                0x00001398 + (n * 0x4);
        }/* end of loop n */

    }/*00001398+n*0x4*/


    /* unit memory */
    {/*0x00010000+n*0x2000*/
        GT_U32    n;

        for(n = 0; n <= 15; n++)
        {
            regsAddrPtr->CNC[instance].perBlockRegs.
                memory.baseAddress[n] =
                0x00010000 + (n * 0x2000);
        }/* end of loop n */

    }/*0x00010000+n*0x2000*/

    /* unit globalRegs */
    regsAddrPtr->CNC[instance].globalRegs.CNCGlobalConfigReg                    = 0x00000000;
    regsAddrPtr->CNC[instance].globalRegs.CNCFastDumpTriggerReg                 = 0x00000030;
    regsAddrPtr->CNC[instance].globalRegs.CNCClearByReadValueRegWord0           = 0x00000040;
    regsAddrPtr->CNC[instance].globalRegs.CNCClearByReadValueRegWord1           = 0x00000044;
    regsAddrPtr->CNC[instance].globalRegs.CNCInterruptSummaryCauseReg           = 0x00000100;
    regsAddrPtr->CNC[instance].globalRegs.CNCInterruptSummaryMaskReg            = 0x00000104;
    regsAddrPtr->CNC[instance].globalRegs.wraparoundFunctionalInterruptCauseReg = 0x00000190;
    regsAddrPtr->CNC[instance].globalRegs.wraparoundFunctionalInterruptMaskReg  = 0x000001a4;
    regsAddrPtr->CNC[instance].globalRegs.rateLimitFunctionalInterruptCauseReg  = 0x000001b8;
    regsAddrPtr->CNC[instance].globalRegs.rateLimitFunctionalInterruptMaskReg   = 0x000001cc;
    regsAddrPtr->CNC[instance].globalRegs.miscFunctionalInterruptCauseReg       = 0x000001e0;
    regsAddrPtr->CNC[instance].globalRegs.miscFunctionalInterruptMaskReg        = 0x000001e4;
}

/**
* @internal txqDqInit function
* @endinternal
*
* @brief   This function inits the DB - per TXQ_DQ unit.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Falcon.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] index                    - unit index
*                                       None.
*
* @note 1. Not called for SIP6 devices !!!
*
*/
static void txqDqInit
(
    IN  GT_U8 devNum,
    IN  GT_U32  index,
    IN  GT_U32  numTxqDqPortsPerUnit_firstBlock,
    IN  GT_U32  numTxqDqPortsPerUnit_secondBlock,
    IN  GT_U32  lastTxDmaIndex
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_U32  isSip5_25 = PRV_CPSS_SIP_5_25_CHECK_MAC(devNum);
    GT_U32  isSip5_20 = PRV_CPSS_SIP_5_20_CHECK_MAC(devNum);
    GT_U32  ii;

    { /*start of unit TXQ */

        { /*start of unit dq */

            { /*start of unit statisticalAndCPUTrigEgrMirrToAnalyzerPort */
                { /*0000d040+i*0x4*/
                    GT_U32    i;
                    if(isSip5_20 == 0)
                    {
                        for(i = 0; i <= 31; i++)
                        {
                            regsAddrPtr->TXQ.dq[index].statisticalAndCPUTrigEgrMirrToAnalyzerPort.egrAnalyzerEnableIndex[i] =
                            0x0000d040+i*0x4;
                        } /* end of loop i */
                    }
                    else
                    {
                        /* we not need all 72 registers ... only 64 (512/8) */
                        GT_U32  numRegs = 64;
                        /* duplication for multi port groups done by function
                           txqDqUpdateUnit0AsGlobalPortsAddr(...) */
                        if(isSip5_25)
                        {
                            numRegs = 128/8;/* HW supports 25 registers , but only 16 will be used */
                                            /* the '25' is 'mistake' */
                        }

                        for(i = 0; i < numRegs; i++)
                        {
                            regsAddrPtr->TXQ.dq[index].statisticalAndCPUTrigEgrMirrToAnalyzerPort.egrAnalyzerEnableIndex[i] =
                            0x0000d040+i*0x4;
                        } /* end of loop i */
                    }
                } /*0000d040+i*0x4*/
                regsAddrPtr->TXQ.dq[index].statisticalAndCPUTrigEgrMirrToAnalyzerPort.STCStatisticalTxSniffConfig = 0x0000d000;
                regsAddrPtr->TXQ.dq[index].statisticalAndCPUTrigEgrMirrToAnalyzerPort.egrCTMTrigger = 0x0000d004;
                regsAddrPtr->TXQ.dq[index].statisticalAndCPUTrigEgrMirrToAnalyzerPort.CTMAnalyzerIndex = 0x0000d010;
                regsAddrPtr->TXQ.dq[index].statisticalAndCPUTrigEgrMirrToAnalyzerPort.STCAnalyzerIndex = 0x0000d014;

            } /*end of unit statisticalAndCPUTrigEgrMirrToAnalyzerPort */


            { /*start of unit shaper */
                { /*00004100+n*0x4*/
                    GT_U32    n;
                    for(n = 0; n <= numTxqDqPortsPerUnit_firstBlock; n++)
                    {
                        regsAddrPtr->TXQ.dq[index].shaper.portTokenBucketMode[n] =
                            0x00004100+n*0x4;

                    } /* end of loop n */
                    {
                        GT_U32  baseAddr = 0x00010000;
                        /* continue with n */
                        for(ii = 0 ; ii < numTxqDqPortsPerUnit_secondBlock ; ii++,n++)
                        {
                            regsAddrPtr->TXQ.dq[index].shaper.portTokenBucketMode[n] =
                                baseAddr+n*0x4;
                        }
                    }
                } /*00004100+n*0x4*/
                { /*00004300+n * 0x4*/
                    GT_U32    n;
                    for(n = 0; n <= numTxqDqPortsPerUnit_firstBlock; n++)
                    {
                        regsAddrPtr->TXQ.dq[index].shaper.portDequeueEnable[n] =
                        0x00004300+n * 0x4;
                    } /* end of loop n */
                    {
                        GT_U32  baseAddr = 0x00010100;
                        /* continue with n */
                        for(ii = 0 ; ii < numTxqDqPortsPerUnit_secondBlock ; ii++,n++)
                        {
                            regsAddrPtr->TXQ.dq[index].shaper.portDequeueEnable[n] =
                                baseAddr+n*0x4;
                        }
                    }
                } /*00004300+n * 0x4*/
                regsAddrPtr->TXQ.dq[index].shaper.tokenBucketUpdateRate = 0x00004000;
                regsAddrPtr->TXQ.dq[index].shaper.tokenBucketBaseLine = 0x00004004;
                regsAddrPtr->TXQ.dq[index].shaper.CPUTokenBucketMTU = 0x00004008;
                regsAddrPtr->TXQ.dq[index].shaper.portsTokenBucketMTU = 0x0000400c;

            } /*end of unit shaper */

            { /*start of unit scheduler */
                { /*start of unit schedulerConfig */
                    { /*00001040+n * 0x4*/
                        GT_U32    n;
                        for(n = 0; n <= numTxqDqPortsPerUnit_firstBlock; n++)
                        {
                            regsAddrPtr->TXQ.dq[index].scheduler.schedulerConfig.conditionalMaskForPort[n] =
                            0x00001040+n * 0x4;
                        } /* end of loop n */
                        {
                            GT_U32  baseAddr = 0x0000F000;
                            /* continue with n */
                            for(ii = 0 ; ii < numTxqDqPortsPerUnit_secondBlock ; ii++,n++)
                            {
                                regsAddrPtr->TXQ.dq[index].scheduler.schedulerConfig.conditionalMaskForPort[n] =
                                    baseAddr+n*0x4;
                            }
                        }
                    } /*00001040+n * 0x4*/
                    regsAddrPtr->TXQ.dq[index].scheduler.schedulerConfig.schedulerConfig = 0x00001000;
                    regsAddrPtr->TXQ.dq[index].scheduler.schedulerConfig.pktLengthForTailDropDequeue = 0x00001008;
                    regsAddrPtr->TXQ.dq[index].scheduler.schedulerConfig.highSpeedPorts[index] = 0x0000100c;

                } /*end of unit schedulerConfig */


                { /*start of unit priorityArbiterWeights */
                    { /*000012c0+p*0x4*/
                        GT_U32    p;
                        for(p = 0; p <= 15; p++)
                        {
                            regsAddrPtr->TXQ.dq[index].scheduler.priorityArbiterWeights.profileSDWRRGroup[p] =
                            0x000012c0+p*0x4;
                        } /* end of loop p */
                    } /*000012c0+p*0x4*/
                    { /*00001240+p*0x4*/
                        GT_U32    p;
                        for(p = 0; p <= 15; p++)
                        {
                            regsAddrPtr->TXQ.dq[index].scheduler.priorityArbiterWeights.profileSDWRRWeightsConfigReg0[p] =
                            0x00001240+p*0x4;
                        } /* end of loop p */
                    } /*00001240+p*0x4*/
                    { /*00001280+p*0x4*/
                        GT_U32    p;
                        for(p = 0; p <= 15; p++)
                        {
                            regsAddrPtr->TXQ.dq[index].scheduler.priorityArbiterWeights.profileSDWRRWeightsConfigReg1[p] =
                            0x00001280+p*0x4;
                        } /* end of loop p */
                    } /*00001280+p*0x4*/
                    { /*00001300+p*0x4*/
                        GT_U32    p;
                        for(p = 0; p <= 15; p++)
                        {
                            regsAddrPtr->TXQ.dq[index].scheduler.priorityArbiterWeights.profileSDWRREnable[p] =
                            0x00001300+p*0x4;
                        } /* end of loop p */
                    } /*00001300+p*0x4*/
                    { /*00001380+n * 0x4*/
                        GT_U32    n;
                        for(n = 0; n <= numTxqDqPortsPerUnit_firstBlock; n++)
                        {
                            regsAddrPtr->TXQ.dq[index].scheduler.priorityArbiterWeights.portSchedulerProfile[n] =
                            0x00001380+n * 0x4;
                        } /* end of loop n */
                        {
                            GT_U32  baseAddr = 0x0000F100;
                            /* continue with n */
                            for(ii = 0 ; ii < numTxqDqPortsPerUnit_secondBlock ; ii++,n++)
                            {
                                regsAddrPtr->TXQ.dq[index].scheduler.priorityArbiterWeights.portSchedulerProfile[n] =
                                    baseAddr+n*0x4;
                            }
                        }
                    } /*00001380+n * 0x4*/

                } /*end of unit priorityArbiterWeights */


                { /*start of unit portShaper */
                    { /*0000190c+0x4*n*/
                        GT_U32    n;
                        for(n = 0; n <= numTxqDqPortsPerUnit_firstBlock; n++)
                        {
                            regsAddrPtr->TXQ.dq[index].scheduler.portShaper.portRequestMaskSelector[n] =
                            0x0000190c+0x4*n;
                        } /* end of loop n */
                        {
                            GT_U32  baseAddr = 0x0000F300;
                            /* continue with n */
                            for(ii = 0 ; ii < numTxqDqPortsPerUnit_secondBlock ; ii++,n++)
                            {
                                regsAddrPtr->TXQ.dq[index].scheduler.portShaper.portRequestMaskSelector[n] =
                                    baseAddr+n*0x4;
                            }
                        }
                    } /*0000190c+0x4*n*/
                    regsAddrPtr->TXQ.dq[index].scheduler.portShaper.portRequestMask = 0x00001904;

                } /*end of unit portShaper */

                { /*start of unit portArbiterConfig */
                    { /*00001510+n*0x4*/
                        GT_U32    n;
                        for(n = 0; n <= numTxqDqPortsPerUnit_firstBlock; n++)
                        {
                            regsAddrPtr->TXQ.dq[index].scheduler.portArbiterConfig.portsArbiterPortWorkConservingEnable[n] =
                            0x00001510+n*0x4;
                        } /* end of loop n */
                        {
                            GT_U32  baseAddr = 0x0000F200;
                            /* continue with n */
                            for(ii = 0 ; ii < numTxqDqPortsPerUnit_secondBlock ; ii++,n++)
                            {
                                regsAddrPtr->TXQ.dq[index].scheduler.portArbiterConfig.portsArbiterPortWorkConservingEnable[n] =
                                    baseAddr+n*0x4;
                            }
                        }
                    } /*00001510+n*0x4*/
                    { /*00001650+n*4*/
                        GT_U32    n;
                        /*-----------------------------------------------*
                         * BC2/BobK/BC3 used 85 registers, Aldrin, AC3X -- 96  *
                         *-----------------------------------------------*/
                        GT_U32    size = sizeof(regsAddrPtr->TXQ.dq[index].scheduler.portArbiterConfig.portsArbiterMap)/
                                         sizeof(regsAddrPtr->TXQ.dq[index].scheduler.portArbiterConfig.portsArbiterMap[0]);
                        GT_U32    N;
                        switch (PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devFamily)
                        {
                            case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
                            case CPSS_PP_FAMILY_DXCH_BOBCAT3_E:/* currently BC3 is treated as BC2 */
                            case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:/* currently ALDRIN2 is treated as BC3 */
                            case CPSS_PP_FAMILY_DXCH_FALCON_E:/* currently Falcon is treated as BC3 */
                                N = PRV_CPSS_DXCH_BC2_PA_MAP_REGNUM_CNS;
                            break;
                            case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
                            case CPSS_PP_FAMILY_DXCH_AC3X_E:
                                N = PRV_CPSS_DXCH_Aldrin_PA_TXQ_MAP_REGNUM_CNS;
                            break;
                            default:
                                {
                                    N = size;
                                    CPSS_LOG_INFORMATION_MAC("TXQ pizza arbiter register init : unknown device");
                                }
                        }
                        if (N >= size)
                        {
                             CPSS_LOG_INFORMATION_MAC("TXQ pizza arbiter map register size greater than size");
                        }
                        for(n = 0; n < N; n++)
                        {
                            regsAddrPtr->TXQ.dq[index].scheduler.portArbiterConfig.portsArbiterMap[n] =
                            0x00001650+n*4;
                        } /* end of loop n */
                    } /*00001650+n*4*/
                    regsAddrPtr->TXQ.dq[index].scheduler.portArbiterConfig.portsArbiterConfig = 0x00001500;
                    regsAddrPtr->TXQ.dq[index].scheduler.portArbiterConfig.portsArbiterStatus = 0x00001800;

                } /*end of unit portArbiterConfig */


            } /*end of unit scheduler */

            { /*start of unit global */
                { /*start of unit memoryParityError */
                    regsAddrPtr->TXQ.dq[index].global.memoryParityError.tokenBucketPriorityParityErrorCntr = 0x00000800;
                    regsAddrPtr->TXQ.dq[index].global.memoryParityError.parityErrorBadAddr = 0x00000808;

                } /*end of unit memoryParityError */


                { /*start of unit interrupt */
                    GT_U32  pMax = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.txqDqNumPortsPerDp ?
                           ((PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.txqDqNumPortsPerDp + 30) / 31) - 1 :
                           3;

                    { /*00000610+p*0x4*/
                        GT_U32    p;
                        for(p = 0; p <= pMax; p++)
                        {
                            regsAddrPtr->TXQ.dq[index].global.interrupt.flushDoneInterruptCause[p] =
                            0x00000610+p*0x4;
                        } /* end of loop p */
                    } /*00000610+p*0x4*/
                    { /*00000620+p*0x4*/
                        GT_U32    p;
                        for(p = 0; p <= pMax; p++)
                        {
                            regsAddrPtr->TXQ.dq[index].global.interrupt.flushDoneInterruptMask[p] =
                            0x00000620+p*0x4;
                        } /* end of loop p */
                    } /*00000620+p*0x4*/
                    { /*00000630+p*0x4*/
                        GT_U32    p;
                        for(p = 0; p <= pMax; p++)
                        {
                            regsAddrPtr->TXQ.dq[index].global.interrupt.egrSTCInterruptCause[p] =
                            0x00000630+p*0x4;
                        } /* end of loop p */
                    } /*00000630+p*0x4*/
                    { /*00000640+p*0x4*/
                        GT_U32    p;
                        for(p = 0; p <= pMax; p++)
                        {
                            regsAddrPtr->TXQ.dq[index].global.interrupt.egrSTCInterruptMask[p] =
                            0x00000640+p*0x4;
                        } /* end of loop p */
                    } /*00000640+p*0x4*/
                    regsAddrPtr->TXQ.dq[index].global.interrupt.txQDQInterruptSummaryCause = 0x00000600;
                    regsAddrPtr->TXQ.dq[index].global.interrupt.txQDQInterruptSummaryMask = 0x00000604;
                    regsAddrPtr->TXQ.dq[index].global.interrupt.txQDQMemoryErrorInterruptCause = 0x00000650;
                    regsAddrPtr->TXQ.dq[index].global.interrupt.txQDQMemoryErrorInterruptMask = 0x00000660;
                    regsAddrPtr->TXQ.dq[index].global.interrupt.txQDQGeneralInterruptCause = 0x00000670;
                    regsAddrPtr->TXQ.dq[index].global.interrupt.txQDQGeneralInterruptMask = 0x00000680;

                } /*end of unit interrupt */


                { /*start of unit globalDQConfig */
                    { /*000002c0+n*0x4*/
                        GT_U32    n;
                        for(n = 0; n <= numTxqDqPortsPerUnit_firstBlock; n++)
                        {
                            regsAddrPtr->TXQ.dq[index].global.globalDQConfig.portToDMAMapTable[n] =
                            0x000002c0+n*0x4;
                        } /* end of loop n */
                        {
                            GT_U32  baseAddr = 0x0000E100;
                            /* continue with n */
                            for(ii = 0 ; ii < numTxqDqPortsPerUnit_secondBlock ; ii++,n++)
                            {
                                regsAddrPtr->TXQ.dq[index].global.globalDQConfig.portToDMAMapTable[n] =
                                    baseAddr+n*0x4;
                            }
                        }
                    } /*000002c0+n*0x4*/
                    { /*0000020c+p*0x4*/
                        GT_U32    p;
                        for(p = 0; p <= 15; p++)
                        {
                            regsAddrPtr->TXQ.dq[index].global.globalDQConfig.profileByteCountModification[p] =
                            0x0000020c+p*0x4;
                        } /* end of loop p */
                    } /*0000020c+p*0x4*/
                    { /*00000004+n*0x4*/
                        GT_U32    n;
                        for(n = 0; n <= numTxqDqPortsPerUnit_firstBlock; n++)
                        {
                            regsAddrPtr->TXQ.dq[index].global.globalDQConfig.portBCConstantValue[n] =
                            0x00000004+n*0x4;
                        } /* end of loop n */
                        {
                            GT_U32  baseAddr = 0x0000E000;
                            /* continue with n */
                            for(ii = 0 ; ii < numTxqDqPortsPerUnit_secondBlock ; ii++,n++)
                            {
                                regsAddrPtr->TXQ.dq[index].global.globalDQConfig.portBCConstantValue[n] =
                                    baseAddr+n*0x4;
                            }
                        }
                    } /*00000004+n*0x4*/
                    regsAddrPtr->TXQ.dq[index].global.globalDQConfig.globalDequeueConfig = 0x00000000;
                    regsAddrPtr->TXQ.dq[index].global.globalDQConfig.BCForCutThrough = 0x00000208;

                    if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
                    {
                        regsAddrPtr->TXQ.dq[index].global.globalDQConfig.creditCountersReset[0] = 0x00000260;
                        if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.txqDqNumPortsPerDp == 0 ||
                           PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.txqDqNumPortsPerDp > 32)
                        {
                            regsAddrPtr->TXQ.dq[index].global.globalDQConfig.creditCountersReset[1] = 0x00000264;
                            regsAddrPtr->TXQ.dq[index].global.globalDQConfig.creditCountersReset[2] = 0x00000268;
                        }
                        else
                        {
                            /* support for device with only 1..32 DQ-ports */
                            /* not have creditCountersReset[1,2] */
                        }
                    }

                } /*end of unit globalDQConfig */

                { /*start of unit flushTrig */
                    { /*00000400+n * 0x4*/
                        GT_U32    n;
                        for(n = 0; n <= numTxqDqPortsPerUnit_firstBlock; n++)
                        {
                            regsAddrPtr->TXQ.dq[index].global.flushTrig.portTxQFlushTrigger[n] =
                            0x00000400+n * 0x4;
                        } /* end of loop n */
                        {
                            GT_U32  baseAddr = 0x0000E400;
                            /* continue with n */
                            for(ii = 0 ; ii < numTxqDqPortsPerUnit_secondBlock ; ii++,n++)
                            {
                                regsAddrPtr->TXQ.dq[index].global.flushTrig.portTxQFlushTrigger[n] =
                                    baseAddr+n*0x4;
                            }
                        }
                    } /*00000400+n * 0x4*/

                } /*end of unit flushTrig */


                { /*start of unit ECCCntrs */
                    regsAddrPtr->TXQ.dq[index].global.ECCCntrs.DQIncECCErrorCntr = 0x00000920;
                    regsAddrPtr->TXQ.dq[index].global.ECCCntrs.DQClrECCErrorCntr = 0x00000924;

                } /*end of unit ECCCntrs */


                { /*start of unit dropCntrs */
                    regsAddrPtr->TXQ.dq[index].global.dropCntrs.egrMirroredDroppedCntr = 0x00000900;
                    regsAddrPtr->TXQ.dq[index].global.dropCntrs.STCDroppedCntr = 0x00000904;

                } /*end of unit dropCntrs */

                if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
                {
                    { /*00000B00+n * 0x4*/
                        GT_U32    n;
                        GT_U32    maxDmaPerDq = 73;
                        if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
                        {
                            maxDmaPerDq = lastTxDmaIndex + 1;
                        }

                        for(n = 0; n < maxDmaPerDq; n++)
                        {
                            regsAddrPtr->TXQ.dq[index].global.creditCounters.txdmaPortCreditCounter[n] =
                                      0x00000b00+n * 0x4;
                        } /* end of loop n */
                    } /*00000b00+n * 0x4*/
                }
            } /*end of unit global */


            { /*start of unit flowCtrlResponse */
                { /*0000c100+n*0x4*/
                    GT_U32    n;
                    for(n = 0; n <= 15; n++)
                    {
                        regsAddrPtr->TXQ.dq[index].flowCtrlResponse.schedulerProfilePFCTimerToPriorityQueueMapEnable[n] =
                        0x0000c100+n*0x4;
                    } /* end of loop n */
                } /*0000c100+n*0x4*/
                { /*0000c180+0x40*t+0x4*p*/
                    GT_U32    p,t;
                    for(p = 0; p <= 15; p++)
                    {
                        for(t = 0; t <= 7; t++)
                        {
                            regsAddrPtr->TXQ.dq[index].flowCtrlResponse.schedulerProfilePriorityQueuePFCFactor[p][t] =
                            0x0000c180+0x40*t+0x4*p;
                        } /* end of loop t */
                    } /* end of loop p */
                } /*0000c180+0x40*t+0x4*p*/
                { /*0000c008+n*0x4*/
                    GT_U32    n;
                    for(n = 0; n <= 15; n++)
                    {
                        regsAddrPtr->TXQ.dq[index].flowCtrlResponse.schedulerProfileLLFCXOFFValue[n] =
                        0x0000c008+n*0x4;
                    } /* end of loop n */
                } /*0000c008+n*0x4*/
                { /*0000c500+p*4*/
                    GT_U32    p;
                    /* in sip5.20 implemented as table, not registers */
                    if(0 == PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
                    {
                        for(p = 0; p <= 255; p++)
                        {
                            regsAddrPtr->TXQ.dq[index].flowCtrlResponse.portToTxQPortMap[p] =
                                0x0000c500+p*4;
                        } /* end of loop p */
                    }
                } /*0000c500+p*4*/
                { /*0000c080+n*0x4*/
                    GT_U32    n;
                    for(n = 0; n <= 7; n++)
                    {
                        regsAddrPtr->TXQ.dq[index].flowCtrlResponse.PFCTimerToPriorityQueueMap[n] =
                        0x0000c080+n*0x4;
                    } /* end of loop n */
                } /*0000c080+n*0x4*/
                regsAddrPtr->TXQ.dq[index].flowCtrlResponse.flowCtrlResponseConfig = 0x0000c000;

                if(isSip5_25)
                {
                    regsAddrPtr->TXQ.dq[index].flowCtrlResponse.ttiToPfcResponseTableEntry = 0x0000c9fc;
                }

            } /*end of unit flowCtrlResponse */

        } /*end of unit dq */

        if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
        {
            { /*0000100c+n*0x4*/
                GT_U32    n;
                for(n = 0; n <= 1; n++)
                {
                    regsAddrPtr->TXQ.dq[index].scheduler.schedulerConfig.highSpeedPorts[n] =
                        0x0000100c+n*0x4;
                } /* end of loop n */
            } /*0000100c+n*0x4*/

            regsAddrPtr->TXQ.dq[index].scheduler.portShaper.fastPortShaperThreshold =
                0x00001908;
        }

    } /*end of unit TXQ */

}
/* NOTE: not called for sip6 devices */
static void txqDqDmaRelatedGlobalPortUpdate
(
    IN  GT_U8 devNum,
    IN  GT_U32  index
)
{
    GT_U32 newIndex;
    GT_U32 lastTxDmaIndex;
    GT_U32 startIndexUnit0;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32  i;

    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
    {
        newIndex =  bc3_arr[index];

        lastTxDmaIndex = 12;
        startIndexUnit0 = lastTxDmaIndex*index;
    }
    else
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
    {
        if(index == 0)
        {
            /* 'trick' call this function ALSO with index [4] to get support also
                for the additional CPU port in dp[0] */
            txqDqDmaRelatedGlobalPortUpdate(devNum,4);
        }

        newIndex =  aldrin2_arr[index];
        lastTxDmaIndex  = aldrin2_indexArr[index];

        startIndexUnit0 = lastTxDmaIndex*index;

        if(index == 4)
        {
            /* the below code must use index '0' and forget about the 'trick' */
            index = 0;
        }

        startIndexUnit0 = aldrin2_startIndexUnit0[index];
    }
    else
    {
        /* logic below not relevant to the device */
        return ;
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    if(index != 0) /* no need to copy to the same point */
    {
        for (i = 0 ; i < lastTxDmaIndex; i++)
        {
            regsAddrPtr->TXQ.dq[0].global.creditCounters.txdmaPortCreditCounter[i+startIndexUnit0] = regsAddrPtr->TXQ.dq[index].global.creditCounters.txdmaPortCreditCounter[i];
        }
    }

    regsAddrPtr->TXQ.dq[0].global.creditCounters.txdmaPortCreditCounter[newIndex] = regsAddrPtr->TXQ.dq[index].global.creditCounters.txdmaPortCreditCounter[lastTxDmaIndex];

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.txqDqNumPortsPerDp == 0 ||
       PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.txqDqNumPortsPerDp > 32)
    {
        regsAddrPtr->TXQ.dq[0].global.globalDQConfig.creditCountersReset[3*index + 0] = regsAddrPtr->TXQ.dq[index].global.globalDQConfig.creditCountersReset[0];
        regsAddrPtr->TXQ.dq[0].global.globalDQConfig.creditCountersReset[3*index + 1] = regsAddrPtr->TXQ.dq[index].global.globalDQConfig.creditCountersReset[1];
        regsAddrPtr->TXQ.dq[0].global.globalDQConfig.creditCountersReset[3*index + 2] = regsAddrPtr->TXQ.dq[index].global.globalDQConfig.creditCountersReset[2];
    }
    else
    {
        /* 1..32 DQ-ports only in first register */
        regsAddrPtr->TXQ.dq[0].global.globalDQConfig.creditCountersReset[index + 0] = regsAddrPtr->TXQ.dq[index].global.globalDQConfig.creditCountersReset[0];
    }


#if 0
    {

        GT_U32 minus_offset = index * (sizeof(regsAddrPtr->TXQ.dq[0]) / sizeof(GT_U32));

        /* copy last RxDma to cpu port position */
        COPY_REG_ADDR_MAC(regsAddrPtr->TXQ.dq[0].global.creditCounters.txdmaPortCreditCounter        , lastTxFifoIndex, minus_offset/*target to DP*/ ,newIndex);
    }
#endif
}


/**
* @internal txqDqUpdateUnit0AsGlobalPortsAddr function
* @endinternal
*
* @brief   update the DB - txq-DQ unit - after multi units set 'per port' addresses.
*         the DB of TXQ.dq[0] updated for the ports with 'global port index'
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] currentUnitIndex         - the index of current TXQ.dq[] unit
* @param[in] portIn_unit_0            - the 'global port' index (in unit 0)
* @param[in] portInCurrentUnit        - the 'local port'  index (in current unit)
* @param[in] numOfPorts               - number of ports to update.
*                                       None
*
* @note NOTE: not called for sip6 devices
*
*/
static void txqDqUpdateUnit0AsGlobalPortsAddr
(
    IN  GT_U8   devNum,
    IN  GT_U32  currentUnitIndex,
    IN  GT_U32  portIn_unit_0,
    IN  GT_U32  portInCurrentUnit,
    IN  GT_U32  numOfPorts
)
{
    GT_U32    saveLastNumOfPorts=0;/*number of ports from the end of the unitStart1Ptr[]
                                  that need to be saved before overridden by unitStart2Ptr[]*/
    GT_U32    saveToIndex=0;/*the index in unitStart1Ptr[] to save the 'last ports'
                                  valid when saveLastNumOfPorts != 0*/

    /************************************/
    /*   update the per port addresses  */
    /*   of the txq-DQ unit 0 with those */
    /*   of unit 1                      */
    /************************************/

    regUnitPerPortAddrUpdate(
        REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, TXQ.dq[0] , TXQ.dq[currentUnitIndex] , scheduler.portArbiterConfig.portsArbiterPortWorkConservingEnable,
            portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex, NULL)
        );

    regUnitPerPortAddrUpdate(
        REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, TXQ.dq[0] , TXQ.dq[currentUnitIndex] , scheduler.portShaper.portRequestMaskSelector,
            portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex, NULL)
        );

    regUnitPerPortAddrUpdate(
        REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, TXQ.dq[0] , TXQ.dq[currentUnitIndex] , scheduler.priorityArbiterWeights.portSchedulerProfile,
            portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex, NULL)
        );

    regUnitPerPortAddrUpdate(
        REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, TXQ.dq[0] , TXQ.dq[currentUnitIndex] , scheduler.schedulerConfig.conditionalMaskForPort,
            portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex, NULL)
        );

    regUnitPerPortAddrUpdate(
        REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, TXQ.dq[0] , TXQ.dq[currentUnitIndex] , shaper.portDequeueEnable,
            portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex, NULL)
        );

    regUnitPerPortAddrUpdate(
        REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, TXQ.dq[0] , TXQ.dq[currentUnitIndex] , shaper.portTokenBucketMode,
            portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex, NULL)
        );

    regUnitPerPortAddrUpdate(
        REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, TXQ.dq[0] , TXQ.dq[currentUnitIndex] , global.globalDQConfig.portToDMAMapTable,
            portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex, NULL)
        );

    regUnitPerPortAddrUpdate(
        REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, TXQ.dq[0] , TXQ.dq[currentUnitIndex] , global.globalDQConfig.portBCConstantValue,
            portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex, NULL)
        );

    regUnitPerPortAddrUpdate(
        REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, TXQ.dq[0] , TXQ.dq[currentUnitIndex] , global.flushTrig.portTxQFlushTrigger,
            portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex, NULL)
        );
}

/**
* @internal txqQcnInit function
* @endinternal
*
* @brief   This function inits the DB - TXQ_QCN unit.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None.
*/
static void txqQcnInit
(
    IN  GT_U8 devNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_U32  isSip5_20 = PRV_CPSS_SIP_5_20_CHECK_MAC(devNum);

    {/*start of unit qcn */
        {/*00000b40+0x4*n*/
            GT_U32    n;
            if(0 == isSip5_20)
            {
                for(n = 0 ; n <= 8 ; n++) {
                    regsAddrPtr->TXQ.qcn.CCFCSpeedIndex[n] =
                        0x00000b40+0x4*n;
                }/* end of loop n */
            }
            else
            {
                for(n = 0 ; n <= 71 ; n++) {
                    regsAddrPtr->TXQ.qcn.CCFCSpeedIndex[n] =
                        0x00000d00+0x4*n;
                }/* end of loop n */
            }
        }/*00000b40+0x4*n*/
        {/*00000090+0x4*n*/
            GT_U32    n;
            for(n = 0 ; n <= 7 ; n++) {
                regsAddrPtr->TXQ.qcn.CNSampleTbl[n] =
                    0x00000090+0x4*n;
            }/* end of loop n */
        }/*00000090+0x4*n*/
        {/*00000300+0x20*p + 0x4*t*/
            GT_U32    p,t, maxProfile;
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
            {
                maxProfile = 15;
            }
            else
            {
                maxProfile = 7;
            }

            for(p = 0 ; p <= maxProfile ; p++) {
                for(t = 0 ; t <= 7 ; t++) {
                    regsAddrPtr->TXQ.qcn.profilePriorityQueueCNThreshold[p][t] =
                        0x00000300+0x20*p + 0x4*t;
                }/* end of loop t */
            }/* end of loop p */
        }/*00000300+0x20*p + 0x4*t*/
        regsAddrPtr->TXQ.qcn.feedbackMIN = 0x00000050;
        regsAddrPtr->TXQ.qcn.feedbackMAX = 0x00000060;
        regsAddrPtr->TXQ.qcn.CNGlobalConfig = 0x00000000;
        {/*00000b20+0x4*n*/
            GT_U32    n;
            if(0 == isSip5_20)
            {
                for(n = 0 ; n <= 7 ; n++) {
                    regsAddrPtr->TXQ.qcn.enFCTriggerByCNFrameOnPort[n] =
                        0x00000b20+0x4*n;
                }/* end of loop n */
            }
            else
            {
                for(n = 0 ; n <= 15 ; n++) {
                    regsAddrPtr->TXQ.qcn.enFCTriggerByCNFrameOnPort[n] =
                        0x00000c00+0x4*n;
                }/* end of loop n */
            }
        }/*00000b20+0x4*n*/
        {/*00000b00+0x4*n*/
            GT_U32    n;
            if(0 == isSip5_20)
            {
                for(n = 0 ; n <= 7 ; n++) {
                    regsAddrPtr->TXQ.qcn.enCNFrameTxOnPort[n] =
                        0x00000b00+0x4*n;
                }/* end of loop n */
            }
            else
            {
                for(n = 0 ; n <= 15 ; n++) {
                    regsAddrPtr->TXQ.qcn.enCNFrameTxOnPort[n] =
                        0x00000b00+0x4*n;
                }/* end of loop n */
            }
        }/*00000b00+0x4*n*/
        regsAddrPtr->TXQ.qcn.feedbackCalcConfigs = 0x00000040;
        regsAddrPtr->TXQ.qcn.txqECCConf = 0x00000130;
        regsAddrPtr->TXQ.qcn.descECCSingleErrorCntr = 0x00000120;
        regsAddrPtr->TXQ.qcn.descECCDoubleErrorCntr = 0x00000124;
        regsAddrPtr->TXQ.qcn.CNTargetAssignment = 0x00000004;
        regsAddrPtr->TXQ.qcn.CNDropCntr = 0x000000f0;
        regsAddrPtr->TXQ.qcn.QCNInterruptCause = 0x00000100;
        regsAddrPtr->TXQ.qcn.QCNInterruptMask = 0x00000110;
        regsAddrPtr->TXQ.qcn.CNDescAttributes = 0x00000008;
        regsAddrPtr->TXQ.qcn.CNBufferFIFOParityErrorsCntr = 0x000000e0;
        regsAddrPtr->TXQ.qcn.CNBufferFIFOOverrunsCntr = 0x000000d0;
        regsAddrPtr->TXQ.qcn.ECCStatus = 0x00000140;

        /* Cn Available Buffer for QCN DBA in Aldrin2 */
        if(PRV_CPSS_SIP_5_25_CHECK_MAC(devNum))
        {
            regsAddrPtr->TXQ.qcn.cnAvailableBuffers = 0x0000000c;
        }
    }/*end of unit qcn */

}

/**
* @internal txqBmxInit function
* @endinternal
*
* @brief   This function inits the DB - TXQ_QCN unit.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin;
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None.
*/
static void txqBmxInit
(
    IN  GT_U8 devNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_U32  isSip5_20 = PRV_CPSS_SIP_5_20_CHECK_MAC(devNum);

    {/*start of unit bmx */
        if(isSip5_20)
        {
            regsAddrPtr->TXQ.bmx.almostFullDebugRegister = 0x00000000;
            regsAddrPtr->TXQ.bmx.fillLevelDebugRegister = 0x00000004;
        }
    }/*end of unit bmx */

}

/**
* @internal txqLLInit function
* @endinternal
*
* @brief   This function inits the DB - TXQ_LL unit.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None.
*/
static void txqLLInit
(
    IN  GT_U8 devNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_U32  isSip5_25 = PRV_CPSS_SIP_5_25_CHECK_MAC(devNum);
    GT_U32  isSip5_20 = PRV_CPSS_SIP_5_20_CHECK_MAC(devNum);
    GT_U32  isSip5_10 = PRV_CPSS_SIP_5_10_CHECK_MAC(devNum);

    {/*start of unit ll */
        {/*start of unit global */
            {/*start of unit portsLinkEnableStatus */
                GT_U32    p;
                GT_U32    nMax = isSip5_25 ? 3 : isSip5_20 ? 17 : 2;
                GT_U32    offset = isSip5_20 ? 0x00112100 : 0x000a0600;
                for(p = 0 ; p <= nMax ; p++) {
                    regsAddrPtr->TXQ.ll.global.portsLinkEnableStatus.portsLinkEnableStatus[p] =
                        offset + 0x4*p;
                }
            }/*end of unit portsLinkEnableStatus */


            {/*start of unit interrupts */
                regsAddrPtr->TXQ.ll.global.interrupts.LLInterruptCause =
                        isSip5_20 ? 0x00112008 : 0x000a0508;
                regsAddrPtr->TXQ.ll.global.interrupts.LLInterruptMask =
                        isSip5_20 ? 0x0011200c : 0x000a050c;

            }/*end of unit interrupts */

            if(!isSip5_20)   /* BC3 does not have such registers */
            {/*start of unit globalLLConfig */
                {/*000a0300+0x4*p*/
                    GT_U32    p;
                    GT_U32    offset = 0x000a0300;
                    for(p = 0 ; p <= 15 ; p++) {
                        regsAddrPtr->TXQ.ll.global.globalLLConfig.profilePriorityQueueToPoolAssociation[p] =
                            offset+0x4*p;
                    }/* end of loop p */
                }/*000a0300+0x4*p*/
                {/*000a0040+n * 0x4*/
                    GT_U32    n;
                    GT_U32    nMax = 71;
                    GT_U32    offset = isSip5_10 ? 0x000a0044 : 0x000a0040;
                    for(n = 0 ; n <= nMax ; n++) {
                        regsAddrPtr->TXQ.ll.global.globalLLConfig.portProfile[n] =
                            offset+n * 0x4;
                    }/* end of loop n */
                }/*000a0040+n * 0x4*/
                {/*000a0004+n*0x4*/
                    GT_U32    n;
                    GT_U32    nMax = 7;
                    GT_U32    offset = 0x000a0004;

                    for(n = 0 ; n <= nMax ; n++) {
                        regsAddrPtr->TXQ.ll.global.globalLLConfig.highSpeedPort[n] =
                            offset+n*0x4;
                    }/* end of loop n */
                }/*000a0004+n*0x4*/

            }/*end of unit globalLLConfig */

            if(isSip5_25)
            {/*start of unit debug bus */
                regsAddrPtr->TXQ.ll.global.debugBus.llMetalFix = 0x00113000;

            }/*end of unit debug bus */


        }/*end of unit global */
    }/*end of unit ll */

}

/**
* @internal txqPfcInit function
* @endinternal
*
* @brief   This function inits the DB - TXQ_PFC unit.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None.
*/
static void txqPfcInit
(
    IN  GT_U8 devNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_U32  isSip5_20 = PRV_CPSS_SIP_5_20_CHECK_MAC(devNum);

    {/*start of unit pfc */
        {/*start of unit PFCInterrupts */
            regsAddrPtr->TXQ.pfc.PFCInterrupts.PFCPortGroupGlobalOverUnderflowInterruptCause = 0x00001330;
            regsAddrPtr->TXQ.pfc.PFCInterrupts.PFCPortGroupGlobalOverUnderflowInterruptMask = 0x00001350;
            regsAddrPtr->TXQ.pfc.PFCInterrupts.PFCInterruptSummaryCause = 0x00001300;
            regsAddrPtr->TXQ.pfc.PFCInterrupts.PFCInterruptSummaryMask = 0x00001304;
            regsAddrPtr->TXQ.pfc.PFCInterrupts.PFCParityInterruptCause = 0x00001310;
            regsAddrPtr->TXQ.pfc.PFCInterrupts.PFCCntrsOverUnderFlowInterruptMask = 0x00001324;
            regsAddrPtr->TXQ.pfc.PFCInterrupts.PFCParityInterruptMask = 0x00001314;
            regsAddrPtr->TXQ.pfc.PFCInterrupts.PFCFIFOsOverrunsInterruptCause = 0x000013b0;
            regsAddrPtr->TXQ.pfc.PFCInterrupts.PFCFIFOsOverrunsInterruptMask = 0x000013b4;
            regsAddrPtr->TXQ.pfc.PFCInterrupts.PFCMsgsSetsOverrunsInterruptCause = 0x000013b8;
            regsAddrPtr->TXQ.pfc.PFCInterrupts.PFCMsgsSetsOverrunsInterruptMask = 0x000013bc;
            regsAddrPtr->TXQ.pfc.PFCInterrupts.PFCCntrsOverUnderFlowInterruptCause = 0x00001320;

        }/*end of unit PFCInterrupts */


        {/*00000170+0x4*n*/
            GT_U32    n;
            if(0 == isSip5_20)
            {
                for(n = 0 ; n <= 63 ; n++) {
                    regsAddrPtr->TXQ.pfc.LLFCTargetPortRemap[n] =
                        0x00000170+0x4*n;
                }/* end of loop n */
            }
            else
            {
                for(n = 0 ; n <= 143 ; n++) {
                    regsAddrPtr->TXQ.pfc.LLFCTargetPortRemap[n] =
                        0x000002F0+0x4*n;
                }/* end of loop n */
            }
        }/*00000170+0x4*n*/
        {/*00000010+0x4*n*/
            GT_U32    n;
            if(0 == isSip5_20)
            {
                for(n = 0 ; n <= 7 ; n++) {
                    regsAddrPtr->TXQ.pfc.portFCMode[n] =
                        0x00000010+0x4*n;
                }/* end of loop n */
            }
            else
            {
                for(n = 0 ; n <= 15 ; n++) {
                    regsAddrPtr->TXQ.pfc.portFCMode[n] =
                        0x000006F0+0x4*n;
                }/* end of loop n */
            }
        }/*00000010+0x4*n*/
        {/*00000900+p*4*/
            GT_U32    p;
            if(0 == isSip5_20)
            {
                for(p = 0 ; p <= 255 ; p++) {
                    regsAddrPtr->TXQ.pfc.PFCSourcePortToPFCIndexMap[p] =
                        0x00000900+p*4;
                }/* end of loop p */
            }
            else
            {
                for(p = 0 ; p <= 511 ; p++) {
                    regsAddrPtr->TXQ.pfc.PFCSourcePortToPFCIndexMap[p] =
                        0x00001C00+p*4;
                }/* end of loop p */
            }
        }/*00000900+p*4*/
        {/*000000f0+0x4*r*/
            GT_U32    r;
            if(0 == isSip5_20)
            {
                for(r = 0 ; r <= 31 ; r++) {
                    regsAddrPtr->TXQ.pfc.PFCPortProfile[r] =
                        0x000000f0+0x4*r;
                }/* end of loop r */
            }
            else
            {
                for(r = 0 ; r <= 63 ; r++) {
                    regsAddrPtr->TXQ.pfc.PFCPortProfile[r] =
                        0x000000f0+0x4*r;
                }/* end of loop r */
            }
        }/*000000f0+0x4*r*/
        {/*00000050+0x4*p*/
            GT_U32    p;
            for(p = 0 ; p <= 0 ; p++) {
                regsAddrPtr->TXQ.pfc.PFCPortGroupFlowCtrlIndicationFIFOOverrunEntry0[p] =
                    0x00000050+0x4*p;
            }/* end of loop p */
        }/*00000050+0x4*p*/
        {/*00000070+0x4*p*/
            GT_U32    p;
            for(p = 0 ; p <= 0 ; p++) {
                regsAddrPtr->TXQ.pfc.PFCPortGroupFlowCtrlIndicationFIFOOverrunEntry1[p] =
                    0x00000070+0x4*p;
            }/* end of loop p */
        }/*00000070+0x4*p*/
        {/*00000090+0x4*p*/
            GT_U32    p;
            for(p = 0 ; p <= 0 ; p++) {
                regsAddrPtr->TXQ.pfc.PFCPortGroupFlowCtrlIndicationFIFOOverrunCntr[p] =
                    0x00000090+0x4*p;
            }/* end of loop p */
        }/*00000090+0x4*p*/
        {/*000000b0+0x4*p*/
            GT_U32    p;
            for(p = 0 ; p <= 0 ; p++) {
                regsAddrPtr->TXQ.pfc.PFCPortGroupEqFIFOOverrunCntr[p] =
                    0x000000b0+0x4*p;
            }/* end of loop p */
        }/*000000b0+0x4*p*/
        {/*00000030+0x4*p*/
            GT_U32    p;
            for(p = 0 ; p <= 0 ; p++) {
                regsAddrPtr->TXQ.pfc.PFCPortGroupCntrsStatus[p] =
                    0x00000030+0x4*p;
            }/* end of loop p */
        }/*00000030+0x4*p*/
        {/*000000d0+0x4*p*/
            GT_U32    p;
            for(p = 0 ; p <= 0 ; p++) {
                regsAddrPtr->TXQ.pfc.PFCPortGroupCntrsParityErrorsCntr[p] =
                    0x000000d0+0x4*p;
            }/* end of loop p */
        }/*000000d0+0x4*p*/
        {/*00014200+0x20*p + 0x4*t*/
            GT_U32    t,p;
            for(t = 0 ; t <= 7 ; t++) {
                for(p = 0 ; p <= 0 ; p++) {
                    regsAddrPtr->TXQ.pfc.PFCGroupOfPortsTCXonThresholds[t][p] =
                        0x00014200+0x20*p + 0x4*t;
                }/* end of loop p */
            }/* end of loop t */
        }/*00014200+0x20*p + 0x4*t*/
        {/*00014000+0x20*p + 0x4*t*/
            GT_U32    t,p;
            for(t = 0 ; t <= 7 ; t++) {
                for(p = 0 ; p <= 0 ; p++) {
                    regsAddrPtr->TXQ.pfc.PFCGroupOfPortsTCXoffThresholds[t][p] =
                        0x00014000+0x20*p + 0x4*t;
                }/* end of loop p */
            }/* end of loop t */
        }/*00014000+0x20*p + 0x4*t*/
        {/*00014400+0x20*p + 0x4*t*/
            GT_U32    t,p;
            for(t = 0 ; t <= 7 ; t++) {
                for(p = 0 ; p <= 0 ; p++) {
                    regsAddrPtr->TXQ.pfc.PFCGroupOfPortsTCDropThresholds[t][p] =
                        0x00014400+0x20*p + 0x4*t;
                }/* end of loop p */
            }/* end of loop t */
        }/*00014400+0x20*p + 0x4*t*/
        {/*00000d00+0x20*p + 0x4*t*/
            GT_U32    p,t;
            for(p = 0 ; p <= 0 ; p++) {
                for(t = 0 ; t <= 7 ; t++) {
                    regsAddrPtr->TXQ.pfc.PFCGroupOfPortsTCCntr[p][t] =
                        0x00000d00+0x20*p + 0x4*t;
                }/* end of loop t */
            }/* end of loop p */
        }/*00000d00+0x20*p + 0x4*t*/
        regsAddrPtr->TXQ.pfc.PFCTriggerGlobalConfig = 0x00000000;
        /* Register definition for PFC DBA in Aldrin2 */
        if(PRV_CPSS_SIP_5_25_CHECK_MAC(devNum))
        {/*dba*/
            {
                GT_U32 t,p;
                for(p = 0 ; p <= 7 ; p++)
                {
                    for(t = 0 ; t <= 7 ; t++)
                    {
                        regsAddrPtr->TXQ.pfc.dba.PFCProfileTCAlphaThresholds[p][t] = 0x003000 +0x20*p + 0x4*t;
                    }
                }
            }
            regsAddrPtr->TXQ.pfc.dba.PFCAvailableBuffers = 0x003100;
            regsAddrPtr->TXQ.pfc.dba.PFCDynamicBufferAllocationDisable = 0x003104;
        }/*end dba*/

    }/*end of unit pfc */

    if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
    {
        {/*000000a0+0x4*p*/
            GT_U32    p;
            for(p = 0 ; p <= 0 ; p++) {
                regsAddrPtr->TXQ.pfc.PFCPortGroupFlowCtrlIndicationFIFOOverrunCntr[p] =
                    0x000000a0+0x4*p;
            }/* end of loop p */
        }/*000000a0+0x4*p*/
        {/*00000060+0x4*p*/
            GT_U32    p;
            for(p = 0 ; p <= 0 ; p++) {
                regsAddrPtr->TXQ.pfc.PFCPortGroupFlowCtrlIndicationFIFOOverrunEntry1[p] =
                    0x00000060+0x4*p;
            }/* end of loop p */
        }/*00000060+0x4*p*/

        regsAddrPtr->TXQ.pfc.PFCInterrupts.PFCPortGroupGlobalOverUnderflowInterruptCause = 0x00001370;
        regsAddrPtr->TXQ.pfc.PFCInterrupts.PFCPortGroupGlobalOverUnderflowInterruptMask = 0x00001390;

        DESTROY_COMPILATION_ARRAY_REG_MAC(regsAddrPtr->TXQ.pfc.PFCPortGroupEqFIFOOverrunCntr);
    }

}


/**
* @internal txqQueueInit function
* @endinternal
*
* @brief   This function inits the DB - TXQ_Q unit.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None.
*/
static void txqQueueInit
(
    IN  GT_U8 devNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_U32  isSip5_20 = PRV_CPSS_SIP_5_20_CHECK_MAC(devNum);

    {/*start of unit queue */
        {/*start of unit tailDrop */
            {/*start of unit tailDropLimits */
                {/*000a0a20+t*0x4*/
                    GT_U32    t;
                    for(t = 0 ; t <= 7 ; t++) {
                        regsAddrPtr->TXQ.queue.tailDrop.tailDropLimits.poolBufferLimits[t] =
                            0x000a0a20+t*0x4;
                    }/* end of loop t */
                }/*000a0a20+t*0x4*/
                {/*000a0a00+t*0x4*/
                    GT_U32    t;
                    for(t = 0 ; t <= 7 ; t++) {
                        regsAddrPtr->TXQ.queue.tailDrop.tailDropLimits.poolDescLimits[t] =
                            0x000a0a00+t*0x4;
                    }/* end of loop t */
                }/*000a0a00+t*0x4*/
                {/*000a0820+0x4*n*/
                    GT_U32    n;
                    for(n = 0 ; n <= 15 ; n++) {
                        regsAddrPtr->TXQ.queue.tailDrop.tailDropLimits.profilePortDescLimits[n] =
                            0x000a0820+0x4*n;
                    }/* end of loop n */
                }/*000a0820+0x4*n*/
                {/*000a0900+0x4*n*/
                    GT_U32    n;
                    for(n = 0 ; n <= 15 ; n++) {
                        regsAddrPtr->TXQ.queue.tailDrop.tailDropLimits.profilePortBufferLimits[n] =
                            0x000a0900+0x4*n;
                    }/* end of loop n */
                }/*000a0900+0x4*n*/
                regsAddrPtr->TXQ.queue.tailDrop.tailDropLimits.globalDescsLimit = 0x000a0800;
                regsAddrPtr->TXQ.queue.tailDrop.tailDropLimits.globalBufferLimit = 0x000a0810;

            }/*end of unit tailDropLimits */


            {/*start of unit tailDropCntrs */
                {/*000a4130+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 71 ; n++) {
                        regsAddrPtr->TXQ.queue.tailDrop.tailDropCntrs.portBuffersCntr[n] =
                            0x000a4130+n*0x4;
                    }/* end of loop n */
                    if(isSip5_20)
                    {
                        for(; n <= 575 ; n++) {
                            regsAddrPtr->TXQ.queue.tailDrop.tailDropCntrs.portBuffersCntr[n] =
                                0x000b2800+n*0x4;
                        }/* end of loop n */
                    }
                }/*000a4130+n*0x4*/
                {/*000a4010+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 71 ; n++) {
                        regsAddrPtr->TXQ.queue.tailDrop.tailDropCntrs.portDescCntr[n] =
                            0x000a4010+n*0x4;
                    }/* end of loop n */
                    if(isSip5_20)
                    {
                        for(; n <= 575 ; n++) {
                            regsAddrPtr->TXQ.queue.tailDrop.tailDropCntrs.portDescCntr[n] =
                                0x000b2000+n*0x4;
                        }/* end of loop n */
                    }
                }/*000a4010+n*0x4*/
                {/*000a4250+t*0x4*/
                    GT_U32    t;
                    for(t = 0 ; t <= 7 ; t++) {
                        regsAddrPtr->TXQ.queue.tailDrop.tailDropCntrs.priorityDescCntr[t] =
                            0x000a4250+t*0x4;
                    }/* end of loop t */
                }/*000a4250+t*0x4*/
                {/*000a4290+t*0x4*/
                    GT_U32    t;
                    for(t = 0 ; t <= 7 ; t++) {
                        regsAddrPtr->TXQ.queue.tailDrop.tailDropCntrs.priorityBuffersCntr[t] =
                            0x000a4290+t*0x4;
                    }/* end of loop t */
                }/*000a4290+t*0x4*/
                regsAddrPtr->TXQ.queue.tailDrop.tailDropCntrs.totalDescCntr = 0x000a4000;
                regsAddrPtr->TXQ.queue.tailDrop.tailDropCntrs.totalBuffersCntr = 0x000a4004;

            }/*end of unit tailDropCntrs */


            {/*start of unit tailDropConfig */
                {/*000a0200+n * 0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 71 ; n++) {
                        regsAddrPtr->TXQ.queue.tailDrop.tailDropConfig.portTailDropCNProfile[n] =
                            0x000a0200+n * 0x4;
                    }/* end of loop n */
                    if(isSip5_20)
                    {
                        for(; n <= 575 ; n++) {
                            regsAddrPtr->TXQ.queue.tailDrop.tailDropConfig.portTailDropCNProfile[n] =
                                0x000b1800+n*0x4;
                        }/* end of loop n */
                    }
                }/*000a0200+n * 0x4*/
                {/*000a0050+n * 0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 71 ; n++) {
                        regsAddrPtr->TXQ.queue.tailDrop.tailDropConfig.portEnqueueEnable[n] =
                            0x000a0050+n * 0x4;
                    }/* end of loop n */
                    if(isSip5_20)
                    {
                        for(; n <= 575 ; n++) {
                            regsAddrPtr->TXQ.queue.tailDrop.tailDropConfig.portEnqueueEnable[n] =
                                0x000b1000+n*0x4;
                        }/* end of loop n */
                    }
                }/*000a0050+n * 0x4*/
                {/*000a0004+p*0x4*/
                    GT_U32    p;
                    for(p = 0 ; p <= 15 ; p++) {
                        regsAddrPtr->TXQ.queue.tailDrop.tailDropConfig.profileTailDropConfig[p] =
                            0x000a0004+p*0x4;
                    }/* end of loop p */
                }/*000a0004+p*0x4*/
                {/*000a03d0+0x4*p*/
                    GT_U32    p;
                    for(p = 0 ; p <= 15 ; p++) {
                        regsAddrPtr->TXQ.queue.tailDrop.tailDropConfig.profilePriorityQueueToPoolAssociation[p] =
                            0x000a03d0+0x4*p;
                    }/* end of loop p */
                }/*000a03d0+0x4*p*/
                {/*000a03a0+t*0x4*/
                    GT_U32    t;
                    for(t = 0 ; t <= 7 ; t++) {
                        regsAddrPtr->TXQ.queue.tailDrop.tailDropConfig.profilePriorityQueueEnablePoolUsage[t] =
                            0x000a03a0+t*0x4;
                    }/* end of loop t */
                }/*000a03a0+t*0x4*/
                {/*000a05c0+0x4*p*/
                    GT_U32    p;
                    for(p = 0 ; p <= 15 ; p++) {
                        regsAddrPtr->TXQ.queue.tailDrop.tailDropConfig.profilePoolWRTDEn[p] =
                            0x000a05c0+0x4*p;
                    }/* end of loop p */
                }/*000a05c0+0x4*p*/
                {/*000a0580+0x4*p*/
                    GT_U32    p;
                    for(p = 0 ; p <= 15 ; p++) {
                        regsAddrPtr->TXQ.queue.tailDrop.tailDropConfig.profileQueueWRTDEn[p] =
                            0x000a0580+0x4*p;
                    }/* end of loop p */
                }/*000a0580+0x4*p*/
                {/*000a0540+0x4*p*/
                    GT_U32    p;
                    for(p = 0 ; p <= 15 ; p++) {
                        regsAddrPtr->TXQ.queue.tailDrop.tailDropConfig.profilePortWRTDEn[p] =
                            0x000a0540+0x4*p;
                    }/* end of loop p */
                }/*000a0540+0x4*p*/
                {/*000a0500+0x4*p*/
                    GT_U32    p;
                    for(p = 0 ; p <= 15 ; p++) {
                        regsAddrPtr->TXQ.queue.tailDrop.tailDropConfig.profileMaxQueueWRTDEn[p] =
                            0x000a0500+0x4*p;
                    }/* end of loop p */
                }/*000a0500+0x4*p*/
                {/*000a0600+0x4*p*/
                    GT_U32    p;
                    for(p = 0 ; p <= 15 ; p++) {
                        regsAddrPtr->TXQ.queue.tailDrop.tailDropConfig.profileECNMarkEn[p] =
                            0x000a0600+0x4*p;
                    }/* end of loop p */
                }/*000a0600+0x4*p*/
                {/*000a0360+0x4*n*/
                    GT_U32    n;
                    for(n = 0 ; n <= 15 ; n++) {
                        regsAddrPtr->TXQ.queue.tailDrop.tailDropConfig.profileCNMTriggeringEnable[n] =
                            0x000a0360+0x4*n;
                    }/* end of loop n */
                }/*000a0360+0x4*n*/
                regsAddrPtr->TXQ.queue.tailDrop.tailDropConfig.globalTailDropConfig = 0x000a0000;
                regsAddrPtr->TXQ.queue.tailDrop.tailDropConfig.pktLengthForTailDropEnqueue = 0x000a0044;
                regsAddrPtr->TXQ.queue.tailDrop.tailDropConfig.maxQueueWRTDMasks = 0x000a0440;
                regsAddrPtr->TXQ.queue.tailDrop.tailDropConfig.portWRTDMasks = 0x000a0444;
                regsAddrPtr->TXQ.queue.tailDrop.tailDropConfig.queueWRTDMasks = 0x000a0448;
                regsAddrPtr->TXQ.queue.tailDrop.tailDropConfig.poolWRTDMasks = 0x000a044c;
                if(PRV_CPSS_SIP_5_25_CHECK_MAC(devNum))
                {
                    regsAddrPtr->TXQ.queue.tailDrop.tailDropConfig.dynamicAvailableBuffers = 0x000a0450;
                }
            }/*end of unit tailDropConfig */


            {/*start of unit resourceHistogram */
                {/*start of unit resourceHistogramLimits */
                    regsAddrPtr->TXQ.queue.tailDrop.resourceHistogram.resourceHistogramLimits.resourceHistogramLimitReg1 = 0x000a9500;
                    regsAddrPtr->TXQ.queue.tailDrop.resourceHistogram.resourceHistogramLimits.resourceHistogramLimitReg2 = 0x000a9504;

                }/*end of unit resourceHistogramLimits */


                {/*start of unit resourceHistogramCntrs */
                    {/*000a9510+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 3 ; n++) {
                            regsAddrPtr->TXQ.queue.tailDrop.resourceHistogram.resourceHistogramCntrs.resourceHistogramCntr[n] =
                                0x000a9510+n*0x4;
                        }/* end of loop n */
                    }/*000a9510+n*0x4*/

                }/*end of unit resourceHistogramCntrs */


            }/*end of unit resourceHistogram */


            {/*start of unit mcFilterLimits */
                regsAddrPtr->TXQ.queue.tailDrop.mcFilterLimits.mirroredPktsToAnalyzerPortDescsLimit = 0x000a9000;
                regsAddrPtr->TXQ.queue.tailDrop.mcFilterLimits.ingrMirroredPktsToAnalyzerPortBuffersLimit = 0x000a9004;
                regsAddrPtr->TXQ.queue.tailDrop.mcFilterLimits.egrMirroredPktsToAnalyzerPortBuffersLimit = 0x000a9010;
                regsAddrPtr->TXQ.queue.tailDrop.mcFilterLimits.mcDescsLimit = 0x000a9020;
                regsAddrPtr->TXQ.queue.tailDrop.mcFilterLimits.mcBuffersLimit = 0x000a9030;

            }/*end of unit mcFilterLimits */


            {/*start of unit muliticastFilterCntrs */
                regsAddrPtr->TXQ.queue.tailDrop.muliticastFilterCntrs.snifferDescsCntr = 0x000a9200;
                regsAddrPtr->TXQ.queue.tailDrop.muliticastFilterCntrs.ingrSnifferBuffersCntr = 0x000a9204;
                regsAddrPtr->TXQ.queue.tailDrop.muliticastFilterCntrs.egrSnifferBuffersCntr = 0x000a9208;
                regsAddrPtr->TXQ.queue.tailDrop.muliticastFilterCntrs.mcDescsCntr = 0x000a920c;
                regsAddrPtr->TXQ.queue.tailDrop.muliticastFilterCntrs.mcBuffersCntr = 0x000a9210;

            }/*end of unit muliticastFilterCntrs */


            {/*start of unit FCUAndQueueStatisticsLimits */
                {/*000aa1b0+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 7 ; n++) {
                        regsAddrPtr->TXQ.queue.tailDrop.FCUAndQueueStatisticsLimits.XONLimit[n] =
                            0x000aa1b0+n*0x4;
                    }/* end of loop n */
                }/*000aa1b0+n*0x4*/
                {/*000aa110+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 7 ; n++) {
                        regsAddrPtr->TXQ.queue.tailDrop.FCUAndQueueStatisticsLimits.XOFFLimit[n] =
                            0x000aa110+n*0x4;
                    }/* end of loop n */
                }/*000aa110+n*0x4*/
                {/*000aa0d0+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 15 ; n++) {
                        regsAddrPtr->TXQ.queue.tailDrop.FCUAndQueueStatisticsLimits.queueXONLimitProfile[n] =
                            0x000aa0d0+n*0x4;
                    }/* end of loop n */
                }/*000aa0d0+n*0x4*/
                {/*000aa090+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 15 ; n++) {
                        regsAddrPtr->TXQ.queue.tailDrop.FCUAndQueueStatisticsLimits.queueXOFFLimitProfile[n] =
                            0x000aa090+n*0x4;
                    }/* end of loop n */
                }/*000aa090+n*0x4*/
                {/*000aa050+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 15 ; n++) {
                        regsAddrPtr->TXQ.queue.tailDrop.FCUAndQueueStatisticsLimits.portXONLimitProfile[n] =
                            0x000aa050+n*0x4;
                    }/* end of loop n */
                }/*000aa050+n*0x4*/
                {/*000aa004+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 15 ; n++) {
                        regsAddrPtr->TXQ.queue.tailDrop.FCUAndQueueStatisticsLimits.portXOFFLimitProfile[n] =
                            0x000aa004+n*0x4;
                    }/* end of loop n */
                }/*000aa004+n*0x4*/
                regsAddrPtr->TXQ.queue.tailDrop.FCUAndQueueStatisticsLimits.globalXONLimit = 0x000aa230;
                regsAddrPtr->TXQ.queue.tailDrop.FCUAndQueueStatisticsLimits.globalXOFFLimit = 0x000aa210;
                regsAddrPtr->TXQ.queue.tailDrop.FCUAndQueueStatisticsLimits.FCUMode = 0x000aa000;

            }/*end of unit FCUAndQueueStatisticsLimits */


        }/*end of unit tailDrop */


        {/*start of unit global */
            if(PRV_CPSS_SIP_5_25_CHECK_MAC(devNum))
            {
                regsAddrPtr->TXQ.queue.global.portBufferStat = 0x00090C00;
                regsAddrPtr->TXQ.queue.global.queueBufferStat = 0x00090C04;
            }

            {/*start of unit interrupt */
                {/*start of unit queueStatistics */
                    {/*000908c0+0x4*i*/
                        GT_U32    i;
                        for(i = 0 ; i <= 17 ; i++) {
                            regsAddrPtr->TXQ.queue.global.interrupt.queueStatistics.lowCrossingStatusReg[i] =
                                0x000908c0+0x4*i;
                        }/* end of loop i */
                    }/*000908c0+0x4*i*/
                    {/*00090680+p*0x4*/
                        GT_U32    p;
                        for(p = 0 ; p <= 71 ; p++) {
                            regsAddrPtr->TXQ.queue.global.interrupt.queueStatistics.highCrossingInterruptMaskPort[p] =
                                0x00090680+p*0x4;
                        }/* end of loop p */
                    }/*00090680+p*0x4*/
                    regsAddrPtr->TXQ.queue.global.interrupt.queueStatistics.lowCrossingInterruptSummaryMaskPorts61To31 = 0x0009088c;
                    regsAddrPtr->TXQ.queue.global.interrupt.queueStatistics.lowCrossingInterruptSummaryMaskPorts30To0 = 0x00090884;
                    regsAddrPtr->TXQ.queue.global.interrupt.queueStatistics.lowCrossingInterruptSummaryCausePorts71To62 = 0x00090890;
                    regsAddrPtr->TXQ.queue.global.interrupt.queueStatistics.lowCrossingInterruptSummaryCausePorts61To31 = 0x00090888;
                    regsAddrPtr->TXQ.queue.global.interrupt.queueStatistics.lowCrossingInterruptSummaryCausePorts30To0 = 0x00090880;
                    {/*00090280+p*0x4*/
                        GT_U32    p;
                        for(p = 0 ; p <= 71 ; p++) {
                            regsAddrPtr->TXQ.queue.global.interrupt.queueStatistics.lowCrossingInterruptMaskPort[p] =
                                0x00090280+p*0x4;
                        }/* end of loop p */
                    }/*00090280+p*0x4*/
                    {/*00090080+0x4*p*/
                        GT_U32    p;
                        for(p = 0 ; p <= 71 ; p++) {
                            regsAddrPtr->TXQ.queue.global.interrupt.queueStatistics.lowCrossingInterruptCausePort[p] =
                                0x00090080+0x4*p;
                        }/* end of loop p */
                    }/*00090080+0x4*p*/
                    {/*000909a0+0x4*i*/
                        GT_U32    i;
                        for(i = 0 ; i <= 17 ; i++) {
                            regsAddrPtr->TXQ.queue.global.interrupt.queueStatistics.highCrossingStatusReg[i] =
                                0x000909a0+0x4*i;
                        }/* end of loop i */
                    }/*000909a0+0x4*i*/
                    regsAddrPtr->TXQ.queue.global.interrupt.queueStatistics.highCrossingInterruptSummaryMaskPorts71To62 = 0x000908ac;
                    regsAddrPtr->TXQ.queue.global.interrupt.queueStatistics.lowCrossingInterruptSummaryMaskPorts71To62 = 0x00090894;
                    regsAddrPtr->TXQ.queue.global.interrupt.queueStatistics.highCrossingInterruptSummaryCausePorts30To0 = 0x00090898;
                    regsAddrPtr->TXQ.queue.global.interrupt.queueStatistics.highCrossingInterruptSummaryMaskPorts30To0 = 0x0009089c;
                    regsAddrPtr->TXQ.queue.global.interrupt.queueStatistics.highCrossingInterruptSummaryCausePorts61To31 = 0x000908a0;
                    regsAddrPtr->TXQ.queue.global.interrupt.queueStatistics.highCrossingInterruptSummaryMaskPorts61To31 = 0x000908a4;
                    regsAddrPtr->TXQ.queue.global.interrupt.queueStatistics.highCrossingInterruptSummaryCausePorts71To62 = 0x000908a8;
                    {/*00090480+0x4*p*/
                        GT_U32    p;
                        for(p = 0 ; p <= 71 ; p++) {
                            regsAddrPtr->TXQ.queue.global.interrupt.queueStatistics.highCrossingInterruptCausePort[p] =
                                0x00090480+0x4*p;
                        }/* end of loop p */
                    }/*00090480+0x4*p*/

                }/*end of unit queueStatistics */


                regsAddrPtr->TXQ.queue.global.interrupt.transmitQueueInterruptSummaryCause = 0x00090000;
                regsAddrPtr->TXQ.queue.global.interrupt.transmitQueueInterruptSummaryMask = 0x00090004;
                regsAddrPtr->TXQ.queue.global.interrupt.generalInterrupt1Cause = 0x00090008;
                regsAddrPtr->TXQ.queue.global.interrupt.generalInterrupt1Mask = 0x0009000c;
                regsAddrPtr->TXQ.queue.global.interrupt.portDescFullInterruptCause0 = 0x00090040;
                regsAddrPtr->TXQ.queue.global.interrupt.portDescFullInterruptMask0 = 0x00090044;
                regsAddrPtr->TXQ.queue.global.interrupt.portDescFullInterruptCause1 = 0x00090048;
                regsAddrPtr->TXQ.queue.global.interrupt.portDescFullInterruptMask1 = 0x0009004c;
                regsAddrPtr->TXQ.queue.global.interrupt.portDescFullInterruptCause2 = 0x00090050;
                regsAddrPtr->TXQ.queue.global.interrupt.portDescFullInterruptMask2 = 0x00090054;
                regsAddrPtr->TXQ.queue.global.interrupt.portBufferFullInterruptCause0 = 0x00090060;
                regsAddrPtr->TXQ.queue.global.interrupt.portBufferFullInterruptMask0 = 0x00090064;
                regsAddrPtr->TXQ.queue.global.interrupt.portBufferFullInterruptCause1 = 0x00090068;
                regsAddrPtr->TXQ.queue.global.interrupt.portBufferFullInterruptMask1 = 0x0009006c;
                regsAddrPtr->TXQ.queue.global.interrupt.portBufferFullInterruptCause2 = 0x00090070;
                regsAddrPtr->TXQ.queue.global.interrupt.portBufferFullInterruptMask2 = 0x00090074;

            }/*end of unit interrupt */


            {/*start of unit ECCCntrs */
                regsAddrPtr->TXQ.queue.global.ECCCntrs.TDClrECCErrorCntr = 0x00090a40;

            }/*end of unit ECCCntrs */


            {/*start of unit dropCntrs */
                {/*00090a00+p*0x4*/
                    GT_U32    p;
                    for(p = 0 ; p <= 3 ; p++) {
                        regsAddrPtr->TXQ.queue.global.dropCntrs.clearPktsDroppedCntrPipe[p] =
                            0x00090a00+p*0x4;
                    }/* end of loop p */
                }/*00090a00+p*0x4*/

            }/*end of unit dropCntrs */

            {/*start of unit Debug Bus */
                if(PRV_CPSS_SIP_5_25_CHECK_MAC(devNum))
                {
                    regsAddrPtr->TXQ.queue.global.debugBus.queueMetalFix = 0x00090a80;
                }
            }/*end of unit Debug Bus */

        }/*end of unit global */


        {/*start of unit peripheralAccess */
            {/*start of unit peripheralAccessMisc */
                {/*00093004+0x4*n*/
                    GT_U32    n;
                    for(n = 0 ; n <= 3 ; n++) {
                        regsAddrPtr->TXQ.queue.peripheralAccess.peripheralAccessMisc.portGroupDescToEQCntr[n] =
                            0x00093004+0x4*n;
                    }/* end of loop n */
                }/*00093004+0x4*n*/
                regsAddrPtr->TXQ.queue.peripheralAccess.peripheralAccessMisc.peripheralAccessMisc = 0x00093000;
                regsAddrPtr->TXQ.queue.peripheralAccess.peripheralAccessMisc.QCNIncArbiterCtrl = 0x00093020;

            }/*end of unit peripheralAccessMisc */


            {/*start of unit egrMIBCntrs */
                {/*00093208+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 1 ; n++) {
                        regsAddrPtr->TXQ.queue.peripheralAccess.egrMIBCntrs.txQMIBCntrsPortSetConfig[n] =
                            0x00093208+n*0x4;
                    }/* end of loop n */
                }/*00093208+n*0x4*/
                {/*00093200+0x4*n*/
                    GT_U32    n;
                    for(n = 0 ; n <= 1 ; n++) {
                        regsAddrPtr->TXQ.queue.peripheralAccess.egrMIBCntrs.txQMIBCntrsSetConfig[n] =
                            0x00093200+0x4*n;
                    }/* end of loop n */
                }/*00093200+0x4*n*/
                {/*00093250+0x4*n*/
                    GT_U32    n;
                    for(n = 0 ; n <= 1 ; n++) {
                        regsAddrPtr->TXQ.queue.peripheralAccess.egrMIBCntrs.setTailDroppedPktCntr[n] =
                            0x00093250+0x4*n;
                    }/* end of loop n */
                }/*00093250+0x4*n*/
                {/*00093210+0x4*n*/
                    GT_U32    n;
                    for(n = 0 ; n <= 1 ; n++) {
                        regsAddrPtr->TXQ.queue.peripheralAccess.egrMIBCntrs.setOutgoingUcPktCntr[n] =
                            0x00093210+0x4*n;
                    }/* end of loop n */
                }/*00093210+0x4*n*/
                {/*00093220+0x4*n*/
                    GT_U32    n;
                    for(n = 0 ; n <= 1 ; n++) {
                        regsAddrPtr->TXQ.queue.peripheralAccess.egrMIBCntrs.setOutgoingMcPktCntr[n] =
                            0x00093220+0x4*n;
                    }/* end of loop n */
                }/*00093220+0x4*n*/
                {/*00093230+0x4*n*/
                    GT_U32    n;
                    for(n = 0 ; n <= 1 ; n++) {
                        regsAddrPtr->TXQ.queue.peripheralAccess.egrMIBCntrs.setOutgoingBcPktCntr[n] =
                            0x00093230+0x4*n;
                    }/* end of loop n */
                }/*00093230+0x4*n*/
                {/*00093290+0x4*n*/
                    GT_U32    n;
                    for(n = 0 ; n <= 1 ; n++) {
                        regsAddrPtr->TXQ.queue.peripheralAccess.egrMIBCntrs.setMcFIFO7_4DroppedPktsCntr[n] =
                            0x00093290+0x4*n;
                    }/* end of loop n */
                }/*00093290+0x4*n*/
                {/*00093280+0x4*n*/
                    GT_U32    n;
                    for(n = 0 ; n <= 1 ; n++) {
                        regsAddrPtr->TXQ.queue.peripheralAccess.egrMIBCntrs.setMcFIFO3_0DroppedPktsCntr[n] =
                            0x00093280+0x4*n;
                    }/* end of loop n */
                }/*00093280+0x4*n*/
                {/*00093260+0x4*n*/
                    GT_U32    n;
                    for(n = 0 ; n <= 1 ; n++) {
                        regsAddrPtr->TXQ.queue.peripheralAccess.egrMIBCntrs.setCtrlPktCntr[n] =
                            0x00093260+0x4*n;
                    }/* end of loop n */
                }/*00093260+0x4*n*/
                {/*00093240+0x4*n*/
                    GT_U32    n;
                    for(n = 0 ; n <= 1 ; n++) {
                        regsAddrPtr->TXQ.queue.peripheralAccess.egrMIBCntrs.setBridgeEgrFilteredPktCntr[n] =
                            0x00093240+0x4*n;
                    }/* end of loop n */
                }/*00093240+0x4*n*/

            }/*end of unit egrMIBCntrs */


            {/*start of unit CNCModes */
                regsAddrPtr->TXQ.queue.peripheralAccess.CNCModes.CNCModes = 0x000936a0;

            }/*end of unit CNCModes */


        }/*end of unit peripheralAccess */


    }/*end of unit queue */

}

/**
* @internal tmDropInit function
* @endinternal
*
* @brief   init the DB - TM DROP unit
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None
*/
static void tmDropInit
(
    IN  GT_U8  devNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    {/*0000000+k*0x04*/
        GT_U32    k;
        for (k = 0 ; k <= 7 ; k++)
        {
            regsAddrPtr->TMDROP.portsInterface[k] =
                0x0000000 + (k * 0x04);
        }/* end of loop k */
    }/*0000000+k*0x04*/
    {/*0000020+k*0x04*/
        GT_U32    k;
        for (k = 0 ; k <= 1 ; k++)
        {
            regsAddrPtr->TMDROP.TMDROPTMTCMap8To87[k] =
                0x0000020 + (k * 0x04);
        }/* end of loop k */
    }/*0000020+k*0x04*/
    regsAddrPtr->TMDROP.globalConfigs = 0x00000028;
    regsAddrPtr->TMDROP.TMDROPInterruptCause = 0x00000030;
    regsAddrPtr->TMDROP.TMDROPInterruptMask = 0x00000034;
}

/**
* @internal tmQMapInit function
* @endinternal
*
* @brief   init the DB - TMQMAP unit
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None
*/
static void tmQMapInit
(
    IN  GT_U8  devNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    {/*0000004+k*0x04*/
        GT_U32    k;
        for(k = 0; (k <= 5); k++) {
            regsAddrPtr->TMQMAP.TC2TCMap8To16[k] =
                0x0000004 + (k * 0x04);
        }/* end of loop k */
    }/*0000004+k*0x04*/

    regsAddrPtr->TMQMAP.TMQMAPGlobalConfig   = 0x00000000;
    regsAddrPtr->TMQMAP.TMQMAPInterruptCause = 0x00000020;
    regsAddrPtr->TMQMAP.TMQMAPInterruptMask  = 0x00000024;
}

/**
* @internal tmIngressGlueInit function
* @endinternal
*
* @brief   init the DB - TM_INGRESS_GLUE unit
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None
*/
static void tmIngressGlueInit
(
    IN  GT_U8  devNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    regsAddrPtr->TM_INGR_GLUE.globalConfig             = 0x00000000;
    regsAddrPtr->TM_INGR_GLUE.debug                    = 0x00000004;
    regsAddrPtr->TM_INGR_GLUE.TMIngrGlueInterruptCause = 0x00000008;
    regsAddrPtr->TM_INGR_GLUE.TMIngrGlueInterruptMask  = 0x0000000C;
}

/**
* @internal tmEgressGlueInit function
* @endinternal
*
* @brief   init the DB - TM_EGRESS_GLUE unit
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None
*/
static void tmEgressGlueInit
(
    IN  GT_U8  devNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    {/*00000600+4*c*/
        GT_U32    c;
        for  (c = 0; (c <= 99); c++)
        {
            regsAddrPtr->TM_EGR_GLUE.statisticalAgingTimeCntrMsb[c] =
                0x00000600 + (4 * c);
        }/* end of loop c */
    }/*00000600+4*c*/
    {/*00000400+c*4*/
        GT_U32    c;
        for  (c = 0; (c <= 99); c++)
        {
            regsAddrPtr->TM_EGR_GLUE.statisticalAgingTimeCntrLsb[c] =
                0x00000400 + (4 * c);
        }/* end of loop c */
    }/*00000400+c*4*/
    regsAddrPtr->TM_EGR_GLUE.TMEgrGlueInterruptMask = 0x00000028;
    regsAddrPtr->TM_EGR_GLUE.TMEgrGlueInterruptCause = 0x00000024;
    {/*00001400+4*c*/
        GT_U32    c;
        for  (c = 0; (c <= 99); c++)
        {
            regsAddrPtr->TM_EGR_GLUE.statisticalMinAgeTime[c] =
                0x00001400 + (4 * c);
        }/* end of loop c */
    }/*00001400+4*c*/
    regsAddrPtr->TM_EGR_GLUE.statisticalMinAgeTimeShadow = 0x0000200c;
    {/*00001200+4*c*/
        GT_U32    c;
        for  (c = 0; (c <= 99); c++)
        {
            regsAddrPtr->TM_EGR_GLUE.statisticalMaxAgeTime[c] =
                0x00001200 + (4 * c);
        }/* end of loop c */
    }/*00001200+4*c*/
    regsAddrPtr->TM_EGR_GLUE.statisticalMaxAgeTimeShadow = 0x00002008;
    regsAddrPtr->TM_EGR_GLUE.statisticalAgingTimeCntrsWraparoundInterruptMask3 = 0x0000011c;
    regsAddrPtr->TM_EGR_GLUE.statisticalAgingTimeCntrsWraparoundInterruptMask2 = 0x00000118;
    regsAddrPtr->TM_EGR_GLUE.statisticalAgingTimeCntrsWraparoundInterruptMask1 = 0x00000114;
    regsAddrPtr->TM_EGR_GLUE.statisticalAgingTimeCntrsWraparoundInterruptMask0 = 0x00000110;
    regsAddrPtr->TM_EGR_GLUE.statisticalAgingTimeCntrsWraparoundInterruptCause3 = 0x0000010c;
    regsAddrPtr->TM_EGR_GLUE.statisticalAgingTimeCntrsWraparoundInterruptCause2 = 0x00000108;
    regsAddrPtr->TM_EGR_GLUE.statisticalAgingTimeCntrsWraparoundInterruptCause1 = 0x00000104;
    regsAddrPtr->TM_EGR_GLUE.statisticalAgingTimeCntrsWraparoundInterruptCause0 = 0x00000100;
    regsAddrPtr->TM_EGR_GLUE.statisticalAgingTimeCntrsMsbShadow = 0x00002004;
    regsAddrPtr->TM_EGR_GLUE.statisticalAgingTimeCntrsLsbShadow = 0x00002000;
    regsAddrPtr->TM_EGR_GLUE.TMEgrGlueSummaryCause = 0x0000002c;
    regsAddrPtr->TM_EGR_GLUE.TMEgrGlueSummaryMask = 0x00000030;
    {/*00000200+c*4*/
        GT_U32    c;
        for  (c = 0; (c <= 99); c++)
        {
            regsAddrPtr->TM_EGR_GLUE.statisticalAgingQueueID[c] =
                0x00000200 + (4 * c);
        }/* end of loop c */
    }/*00000200+c*4*/
    regsAddrPtr->TM_EGR_GLUE.statisticalAgingPktCntrsWraparoundInterruptMask3 = 0x00000054;
    regsAddrPtr->TM_EGR_GLUE.statisticalAgingPktCntrsWraparoundInterruptMask2 = 0x00000050;
    regsAddrPtr->TM_EGR_GLUE.statisticalAgingPktCntrsWraparoundInterruptMask1 = 0x0000004c;
    regsAddrPtr->TM_EGR_GLUE.statisticalAgingPktCntrsWraparoundInterruptMask0 = 0x00000048;
    regsAddrPtr->TM_EGR_GLUE.statisticalAgingPktCntrsWraparoundInterruptCause3 = 0x00000044;
    regsAddrPtr->TM_EGR_GLUE.statisticalAgingPktCntrsWraparoundInterruptCause2 = 0x00000040;
    regsAddrPtr->TM_EGR_GLUE.statisticalAgingPktCntrsWraparoundInterruptCause1 = 0x0000003c;
    regsAddrPtr->TM_EGR_GLUE.statisticalAgingPktCntrsWraparoundInterruptCause0 = 0x00000038;
    {/*00000800+c*4*/
        GT_U32    c;
        for  (c = 0; (c <= 99); c++)
        {
            regsAddrPtr->TM_EGR_GLUE.statisticalAgingPktCntr[c] =
                0x00000800 + (4 * c);
        }/* end of loop c */
    }/*00000800+c*4*/
    regsAddrPtr->TM_EGR_GLUE.statisticalAgingOctetCntrsWraparoundInterruptMask3 = 0x0000013c;
    regsAddrPtr->TM_EGR_GLUE.statisticalAgingOctetCntrsWraparoundInterruptMask2 = 0x00000138;
    regsAddrPtr->TM_EGR_GLUE.statisticalAgingOctetCntrsWraparoundInterruptMask1 = 0x00000134;
    regsAddrPtr->TM_EGR_GLUE.statisticalAgingOctetCntrsWraparoundInterruptMask0 = 0x00000130;
    regsAddrPtr->TM_EGR_GLUE.statisticalAgingOctetCntrsWraparoundInterruptCause3 = 0x0000012c;
    regsAddrPtr->TM_EGR_GLUE.statisticalAgingOctetCntrsWraparoundInterruptCause2 = 0x00000128;
    regsAddrPtr->TM_EGR_GLUE.statisticalAgingOctetCntrsWraparoundInterruptCause1 = 0x00000124;
    regsAddrPtr->TM_EGR_GLUE.statisticalAgingOctetCntrsWraparoundInterruptCause0 = 0x00000120;
    regsAddrPtr->TM_EGR_GLUE.statisticalAgingOctetCntrsMsbShadow = 0x00002014;
    regsAddrPtr->TM_EGR_GLUE.statisticalAgingOctetCntrsLsbShadow = 0x00002010;
    {/*00001000+c*4*/
        GT_U32    c;
        for  (c = 0; (c <= 99); c++)
        {
            regsAddrPtr->TM_EGR_GLUE.statisticalAgingOctetCntrMsb[c] =
                0x00001000 + (4 * c);
        }/* end of loop c */
    }/*00001000+c*4*/
    {/*00000c00+c*4*/
        GT_U32    c;
        for  (c = 0; (c <= 99); c++)
        {
            regsAddrPtr->TM_EGR_GLUE.statisticalAgingOctetCntrLsb[c] =
                0x00000c00 + (4 * c);
        }/* end of loop c */
    }/*00000c00+c*4*/
    regsAddrPtr->TM_EGR_GLUE.statisticalAgingAgedOutPktCntrsWraparoundInterruptMask3 = 0x0000015c;
    regsAddrPtr->TM_EGR_GLUE.statisticalAgingAgedOutPktCntrsWraparoundInterruptMask2 = 0x00000158;
    regsAddrPtr->TM_EGR_GLUE.statisticalAgingAgedOutPktCntrsWraparoundInterruptMask1 = 0x00000154;
    regsAddrPtr->TM_EGR_GLUE.statisticalAgingAgedOutPktCntrsWraparoundInterruptMask0 = 0x00000150;
    regsAddrPtr->TM_EGR_GLUE.statisticalAgingAgedOutPktCntrsWraparoundInterruptCause3 = 0x0000014c;
    regsAddrPtr->TM_EGR_GLUE.statisticalAgingAgedOutPktCntrsWraparoundInterruptCause2 = 0x00000148;
    regsAddrPtr->TM_EGR_GLUE.statisticalAgingAgedOutPktCntrsWraparoundInterruptCause1 = 0x00000144;
    regsAddrPtr->TM_EGR_GLUE.statisticalAgingAgedOutPktCntrsWraparoundInterruptCause0 = 0x00000140;
    regsAddrPtr->TM_EGR_GLUE.statisticalAgingAgedOutPktCntrsShadow = 0x00002018;
    {/*00001600+c*4*/
        GT_U32    c;
        for  (c = 0; (c <= 99); c++)
        {
            regsAddrPtr->TM_EGR_GLUE.statisticalAgingAgedOutPktCntr[c] =
                0x00001600 + (4 * c);
        }/* end of loop c */
    }/*00001600+c*4*/
    regsAddrPtr->TM_EGR_GLUE.queueTimerConfig = 0x00000064;
    regsAddrPtr->TM_EGR_GLUE.queueTimer = 0x00000060;
    regsAddrPtr->TM_EGR_GLUE.portInterface2 = 0x00000078;
    regsAddrPtr->TM_EGR_GLUE.portInterface1 = 0x00000074;
    regsAddrPtr->TM_EGR_GLUE.portInterface0 = 0x00000070;
    {/*00001a00+c*4*/
        GT_U32    c;
        for  (c = 0; (c <= 191); c++)
        {
            regsAddrPtr->TM_EGR_GLUE.perPortAgedOutPktCntr[c] =
                0x00001a00 + (4 * c);
        }/* end of loop c */
    }/*00001a00+c*4*/
    regsAddrPtr->TM_EGR_GLUE.perPortAgedOutPktCntrsWraparoundInterruptMask6 = 0x00002058;
    regsAddrPtr->TM_EGR_GLUE.perPortAgedOutPktCntrsWraparoundInterruptMask5 = 0x00002054;
    regsAddrPtr->TM_EGR_GLUE.perPortAgedOutPktCntrsWraparoundInterruptMask4 = 0x00002050;
    regsAddrPtr->TM_EGR_GLUE.perPortAgedOutPktCntrsWraparoundInterruptMask3 = 0x0000204c;
    regsAddrPtr->TM_EGR_GLUE.perPortAgedOutPktCntrsWraparoundInterruptMask2 = 0x00002048;
    regsAddrPtr->TM_EGR_GLUE.perPortAgedOutPktCntrsWraparoundInterruptMask1 = 0x00002044;
    regsAddrPtr->TM_EGR_GLUE.perPortAgedOutPktCntrsWraparoundInterruptMask0 = 0x00002040;
    regsAddrPtr->TM_EGR_GLUE.perPortAgedOutPktCntrsWraparoundInterruptCause6 = 0x00002038;
    regsAddrPtr->TM_EGR_GLUE.perPortAgedOutPktCntrsWraparoundInterruptCause5 = 0x00002034;
    regsAddrPtr->TM_EGR_GLUE.perPortAgedOutPktCntrsWraparoundInterruptCause4 = 0x00002030;
    regsAddrPtr->TM_EGR_GLUE.perPortAgedOutPktCntrsWraparoundInterruptCause3 = 0x0000202c;
    regsAddrPtr->TM_EGR_GLUE.perPortAgedOutPktCntrsWraparoundInterruptCause2 = 0x00002028;
    regsAddrPtr->TM_EGR_GLUE.perPortAgedOutPktCntrsWraparoundInterruptCause1 = 0x00002024;
    regsAddrPtr->TM_EGR_GLUE.perPortAgedOutPktCntrsWraparoundInterruptCause0 = 0x00002020;
    regsAddrPtr->TM_EGR_GLUE.globalConfig = 0x00000000;
    regsAddrPtr->TM_EGR_GLUE.ECCStatus = 0x0000000c;
    {/*00002100+4*c*/
        GT_U32    c;
        for  (c = 0; (c <= 191); c++)
        {
            regsAddrPtr->TM_EGR_GLUE.dropPortAllOutgoingPkts[c] =
                0x00002100 + (4 * c);
        }/* end of loop c */
    }/*00002100+4*c*/
    regsAddrPtr->TM_EGR_GLUE.dropAgedPktCntrConfig = 0x00000004;
    regsAddrPtr->TM_EGR_GLUE.dropAgedPktCntr = 0x00000008;
    regsAddrPtr->TM_EGR_GLUE.debug = 0x00000010;
}

/**
* @internal tmFcuInit function
* @endinternal
*
* @brief   init the DB - TM_FCU unit
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None
*/
static void tmFcuInit
(
    IN  GT_U8  devNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    {/*0000200 + (4 * w)*/
        GT_U32    w;
        for(w = 0; (w <= 63); w++)
        {
            regsAddrPtr->TM_FCU.TMEgrFlowCtrlCntrs[w] =
                0x0000200 + (4 * w);
        }/* end of loop w */
    }/*0000200 + (4 * w)*/
    {/*0000300 + (4 * w)*/
        GT_U32    w;
        for(w = 0; (w <= 63); w++)
        {
            regsAddrPtr->TM_FCU.TMEgrFlowCtrlXOFFThresholds[w] =
                0x0000300 + (4 * w);
        }/* end of loop w */
    }/*0000300 + (4 * w)*/
    {/*0000400 + (4 * w)*/
        GT_U32    w;
        for(w = 0; (w <= 63); w++)
        {
            regsAddrPtr->TM_FCU.TMEgrFlowCtrlXONThresholds[w] =
                0x0000400 + (4 * w);
        }/* end of loop w */
    }/*0000400 + (4 * w)*/
    regsAddrPtr->TM_FCU.TMFCUGlobalConfigs = 0x00000000;
}

/**
* @internal lmsInit function
* @endinternal
*
* @brief   init the DB - LMS unit
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] instance                 - CNC  number
*                                       None
*/
static void lmsInit
(
    IN  GT_U8  devNum,
    IN  GT_U32 instance
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    {/*start of unit LMS_0 */
        {/*start of unit LMS0 */
            {/*start of unit LMS0Group0 */
                {/*start of unit SMIConfig */
                    regsAddrPtr->GOP.LMS[instance].LMS0.LMS0Group0.SMIConfig.PHYAddrReg0forPorts0Through5 = 0x00004030;
                    regsAddrPtr->GOP.LMS[instance].LMS0.LMS0Group0.SMIConfig.PHYAutoNegConfigReg0 = 0x00004034;
                    regsAddrPtr->GOP.LMS[instance].LMS0.LMS0Group0.SMIConfig.SMI0Management = 0x00004054;
                    regsAddrPtr->GOP.LMS[instance].LMS0.LMS0Group0.SMIConfig.LMS0MiscConfigs = 0x00004200;

                }/*end of unit SMIConfig */


                {/*start of unit portsMIBCntrsInterrupts */
                    regsAddrPtr->GOP.LMS[instance].LMS0.LMS0Group0.portsMIBCntrsInterrupts.portsGOP0MIBsInterruptCause = 0x00004000;
                    regsAddrPtr->GOP.LMS[instance].LMS0.LMS0Group0.portsMIBCntrsInterrupts.portsGOP0MIBsInterruptMask = 0x00004004;

                }/*end of unit portsMIBCntrsInterrupts */


                {/*start of unit portsMACSourceAddr */
                    regsAddrPtr->GOP.LMS[instance].LMS0.LMS0Group0.portsMACSourceAddr.sourceAddrMiddle = 0x00004024;
                    regsAddrPtr->GOP.LMS[instance].LMS0.LMS0Group0.portsMACSourceAddr.sourceAddrHigh = 0x00004028;

                }/*end of unit portsMACSourceAddr */


                {/*start of unit portsMACMIBCntrs */
                    regsAddrPtr->GOP.LMS[instance].LMS0.LMS0Group0.portsMACMIBCntrs.MIBCntrsCtrlReg0forPorts0Through5 = 0x00004020;

                }/*end of unit portsMACMIBCntrs */


                {/*start of unit portsAndMIBCntrsInterruptSummary */
                    regsAddrPtr->GOP.LMS[instance].LMS0.LMS0Group0.portsAndMIBCntrsInterruptSummary.MIBCntrsInterruptSummary = 0x00004010;

                }/*end of unit portsAndMIBCntrsInterruptSummary */


                {/*start of unit periodicFCRegs */
                    regsAddrPtr->GOP.LMS[instance].LMS0.LMS0Group0.periodicFCRegs.gigPortsPeriodicFCCntrMaxValueReg = 0x00004140;

                }/*end of unit periodicFCRegs */


                {/*start of unit LEDInterfaceConfig */
                    regsAddrPtr->GOP.LMS[instance].LMS0.LMS0Group0.LEDInterfaceConfig.LEDInterface0CtrlReg0forPorts0Through11AndStackPort = 0x00004100;
                    regsAddrPtr->GOP.LMS[instance].LMS0.LMS0Group0.LEDInterfaceConfig.LEDInterface0Group01ConfigRegforPorts0Through11 = 0x00004104;
                    regsAddrPtr->GOP.LMS[instance].LMS0.LMS0Group0.LEDInterfaceConfig.LEDInterface0Class01ManipulationRegforPorts0Through11 = 0x00004108;
                    regsAddrPtr->GOP.LMS[instance].LMS0.LMS0Group0.LEDInterfaceConfig.LEDInterface0Class4ManipulationRegforPorts0Through11 = 0x0000410c;
                    regsAddrPtr->GOP.LMS[instance].LMS0.LMS0Group0.LEDInterfaceConfig.stackLEDInterface0Class04ManipulationRegforStackPorts = 0x00005100;
                    regsAddrPtr->GOP.LMS[instance].LMS0.LMS0Group0.LEDInterfaceConfig.stackLEDInterface0Class59ManipulationRegforStackPorts = 0x00005104;
                    regsAddrPtr->GOP.LMS[instance].LMS0.LMS0Group0.LEDInterfaceConfig.LEDInterface0FlexlinkPortsDebugSelectRegforStackPort = 0x00005110;
                    regsAddrPtr->GOP.LMS[instance].LMS0.LMS0Group0.LEDInterfaceConfig.LEDInterface0FlexlinkPortsDebugSelectReg1forStackPort = 0x00005114;

                }/*end of unit LEDInterfaceConfig */


            }/*end of unit LMS0Group0 */


            {/*start of unit LMS0Group1 */
                {/*start of unit SMIConfig */
                    regsAddrPtr->GOP.LMS[instance].LMS0.LMS0Group1.SMIConfig.flowCtrlAdvertiseForFiberMediaSelectedConfigReg0forPorts0Through11 = 0x01004024;
                    regsAddrPtr->GOP.LMS[instance].LMS0.LMS0Group1.SMIConfig.PHYAddrReg1forPorts6Through11 = 0x01004030;
                    regsAddrPtr->GOP.LMS[instance].LMS0.LMS0Group1.SMIConfig.PHYAutoNegConfigReg1 = 0x01004034;

                }/*end of unit SMIConfig */


                {/*start of unit portsMIBCntrsInterrupts */
                    regsAddrPtr->GOP.LMS[instance].LMS0.LMS0Group1.portsMIBCntrsInterrupts.triSpeedPortsGOP1MIBsInterruptCause = 0x01004000;
                    regsAddrPtr->GOP.LMS[instance].LMS0.LMS0Group1.portsMIBCntrsInterrupts.triSpeedPortsGOP1MIBsInterruptMask = 0x01004004;

                }/*end of unit portsMIBCntrsInterrupts */


                {/*start of unit portsMACMIBCntrs */
                    regsAddrPtr->GOP.LMS[instance].LMS0.LMS0Group1.portsMACMIBCntrs.MIBCntrsCtrlReg0forPorts6Through11 = 0x01004020;

                }/*end of unit portsMACMIBCntrs */


                {/*start of unit portsAndMIBCntrsInterruptSummary */
                    regsAddrPtr->GOP.LMS[instance].LMS0.LMS0Group1.portsAndMIBCntrsInterruptSummary.portsMIBCntrsInterruptSummaryMask = 0x01005110;
                    regsAddrPtr->GOP.LMS[instance].LMS0.LMS0Group1.portsAndMIBCntrsInterruptSummary.portsInterruptSummaryMask = 0x01005114;

                }/*end of unit portsAndMIBCntrsInterruptSummary */


                {/*start of unit periodicFCRegs */
                    regsAddrPtr->GOP.LMS[instance].LMS0.LMS0Group1.periodicFCRegs.twoAndHalfGigPortsPeriodicFCCntrMaxValueReg = 0x01004140;

                }/*end of unit periodicFCRegs */


                {/*start of unit LEDInterfaceConfig */
                    regsAddrPtr->GOP.LMS[instance].LMS0.LMS0Group1.LEDInterfaceConfig.LEDInterface0CtrlReg1AndClass6ManipulationRegforPorts0Through11 = 0x01004100;
                    regsAddrPtr->GOP.LMS[instance].LMS0.LMS0Group1.LEDInterfaceConfig.LEDInterface0Group23ConfigRegforPorts0Through11 = 0x01004104;
                    regsAddrPtr->GOP.LMS[instance].LMS0.LMS0Group1.LEDInterfaceConfig.LEDInterface0Class23ManipulationRegforPorts0Through11 = 0x01004108;
                    regsAddrPtr->GOP.LMS[instance].LMS0.LMS0Group1.LEDInterfaceConfig.LEDInterface0Class5ManipulationRegforPorts0Through11 = 0x0100410c;
                    regsAddrPtr->GOP.LMS[instance].LMS0.LMS0Group1.LEDInterfaceConfig.stackLEDInterface0Class1011ManipulationRegforStackPorts = 0x01005100;
                    regsAddrPtr->GOP.LMS[instance].LMS0.LMS0Group1.LEDInterfaceConfig.stackLEDInterface0Group01ConfigRegforStackPort = 0x01005104;

                }/*end of unit LEDInterfaceConfig */


                regsAddrPtr->GOP.LMS[instance].LMS0.LMS0Group1.stackPortsModeReg = 0x01004144;

            }/*end of unit LMS0Group1 */


        }/*end of unit LMS0 */


        {/*start of unit LMS1 */
            {/*start of unit LMS1Group0 */
                {/*start of unit SMIConfig */
                    regsAddrPtr->GOP.LMS[instance].LMS1.LMS1Group0.SMIConfig.PHYAddrReg2forPorts12Through17 = 0x02004030;
                    regsAddrPtr->GOP.LMS[instance].LMS1.LMS1Group0.SMIConfig.PHYAutoNegConfigReg2 = 0x02004034;
                    regsAddrPtr->GOP.LMS[instance].LMS1.LMS1Group0.SMIConfig.SMI1Management = 0x02004054;
                    regsAddrPtr->GOP.LMS[instance].LMS1.LMS1Group0.SMIConfig.LMS1MiscConfigs = 0x02004200;

                }/*end of unit SMIConfig */


                {/*start of unit portsMIBCntrsInterrupts */
                    regsAddrPtr->GOP.LMS[instance].LMS1.LMS1Group0.portsMIBCntrsInterrupts.portsGOP2MIBsInterruptCause = 0x02004000;
                    regsAddrPtr->GOP.LMS[instance].LMS1.LMS1Group0.portsMIBCntrsInterrupts.portsGOP2MIBsInterruptMask = 0x02004004;

                }/*end of unit portsMIBCntrsInterrupts */


                {/*start of unit portsMACMIBCntrs */
                    regsAddrPtr->GOP.LMS[instance].LMS1.LMS1Group0.portsMACMIBCntrs.MIBCntrsCtrlReg0forPorts12Through17 = 0x02004020;

                }/*end of unit portsMACMIBCntrs */


                {/*start of unit portsInterruptSummary */
                    regsAddrPtr->GOP.LMS[instance].LMS1.LMS1Group0.portsInterruptSummary.portsInterruptSummary = 0x02004010;

                }/*end of unit portsInterruptSummary */


                {/*start of unit LEDInterfaceConfig */
                    regsAddrPtr->GOP.LMS[instance].LMS1.LMS1Group0.LEDInterfaceConfig.LEDInterface1CtrlReg0forPorts12Through23AndStackPort = 0x02004100;
                    regsAddrPtr->GOP.LMS[instance].LMS1.LMS1Group0.LEDInterfaceConfig.LEDInterface1Group01ConfigRegforPorts12Through23 = 0x02004104;
                    regsAddrPtr->GOP.LMS[instance].LMS1.LMS1Group0.LEDInterfaceConfig.LEDInterface1Class01ManipulationRegforPorts12Through23 = 0x02004108;
                    regsAddrPtr->GOP.LMS[instance].LMS1.LMS1Group0.LEDInterfaceConfig.LEDInterface1Class4ManipulationRegforPorts12Through23 = 0x0200410c;
                    regsAddrPtr->GOP.LMS[instance].LMS1.LMS1Group0.LEDInterfaceConfig.stackLEDInterface1Class04ManipulationRegforStackPorts = 0x02005100;
                    regsAddrPtr->GOP.LMS[instance].LMS1.LMS1Group0.LEDInterfaceConfig.stackLEDInterface1Class59ManipulationRegforStackPorts = 0x02005104;
                    regsAddrPtr->GOP.LMS[instance].LMS1.LMS1Group0.LEDInterfaceConfig.LEDInterface1FlexlinkPortsDebugSelectRegforStackPort = 0x02005110;
                    regsAddrPtr->GOP.LMS[instance].LMS1.LMS1Group0.LEDInterfaceConfig.LEDInterface1FlexlinkPortsDebugSelectReg1forStackPort = 0x02005114;

                }/*end of unit LEDInterfaceConfig */


            }/*end of unit LMS1Group0 */


            {/*start of unit LMS1Group1 */
                {/*start of unit SMIConfig */
                    regsAddrPtr->GOP.LMS[instance].LMS1.LMS1Group1.SMIConfig.flowCtrlAdvertiseForFiberMediaSelectedConfigReg1forPorts12Through23 = 0x03004024;
                    regsAddrPtr->GOP.LMS[instance].LMS1.LMS1Group1.SMIConfig.PHYAddrReg3forPorts18Through23 = 0x03004030;
                    regsAddrPtr->GOP.LMS[instance].LMS1.LMS1Group1.SMIConfig.PHYAutoNegConfigReg3 = 0x03004034;

                }/*end of unit SMIConfig */


                {/*start of unit portsMIBCntrsInterrupts */
                    regsAddrPtr->GOP.LMS[instance].LMS1.LMS1Group1.portsMIBCntrsInterrupts.portsGOP3AndStackMIBsInterruptCause = 0x03004000;
                    regsAddrPtr->GOP.LMS[instance].LMS1.LMS1Group1.portsMIBCntrsInterrupts.portsGOP3AndStackMIBsInterruptMask = 0x03004004;

                }/*end of unit portsMIBCntrsInterrupts */


                {/*start of unit portsMACMIBCntrs */
                    regsAddrPtr->GOP.LMS[instance].LMS1.LMS1Group1.portsMACMIBCntrs.MIBCntrsCtrlReg0forPorts18Through23 = 0x03004020;

                }/*end of unit portsMACMIBCntrs */


                {/*start of unit periodicFCRegs */
                    regsAddrPtr->GOP.LMS[instance].LMS1.LMS1Group1.periodicFCRegs.stackPorts1GPeriodicFCCntrMaxValueReg = 0x03004140;

                }/*end of unit periodicFCRegs */


                {/*start of unit LEDInterfaceConfig */
                    regsAddrPtr->GOP.LMS[instance].LMS1.LMS1Group1.LEDInterfaceConfig.LEDInterface1CtrlReg1AndClass6ManipulationforPorts12Through23 = 0x03004100;
                    regsAddrPtr->GOP.LMS[instance].LMS1.LMS1Group1.LEDInterfaceConfig.LEDInterface1Group23ConfigRegforPorts12Through23 = 0x03004104;
                    regsAddrPtr->GOP.LMS[instance].LMS1.LMS1Group1.LEDInterfaceConfig.LEDInterface1Class23ManipulationRegforPorts12Through23 = 0x03004108;
                    regsAddrPtr->GOP.LMS[instance].LMS1.LMS1Group1.LEDInterfaceConfig.LEDInterface1Class5ManipulationRegforPorts12Through23 = 0x0300410c;
                    regsAddrPtr->GOP.LMS[instance].LMS1.LMS1Group1.LEDInterfaceConfig.stackLEDInterface1Class1011ManipulationRegforStackPort = 0x03005100;
                    regsAddrPtr->GOP.LMS[instance].LMS1.LMS1Group1.LEDInterfaceConfig.stackLEDInterface1Group01ConfigRegforStackPort = 0x03005104;

                }/*end of unit LEDInterfaceConfig */


            }/*end of unit LMS1Group1 */


        }/*end of unit LMS1 */


    }/*end of unit LMS_0 */
}


/**
* @internal prv_smiInstanceInit function
* @endinternal
*
* @brief   init the DB - SMI unit
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] instanceCnt              - CNC instance number
*                                       None
*/
static void prv_smiInstanceInit
(
    IN  GT_U8  devNum,
    IN  GT_U32 instanceCnt
)
{
    GT_U32 phyRegCnt;
    GT_U32 phyRegN;

    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    regsAddrPtr->GOP.SMI[instanceCnt].SMIManagement            = 0x00000000;
    regsAddrPtr->GOP.SMI[instanceCnt].SMIMiscConfiguration     = 0x00000004;
    regsAddrPtr->GOP.SMI[instanceCnt].PHYAutoNegotiationConfig = 0x00000008;

    phyRegN = sizeof(regsAddrPtr->GOP.SMI[instanceCnt].PHYAddress)/sizeof(regsAddrPtr->GOP.SMI[instanceCnt].PHYAddress[0]);
    for (phyRegCnt = 0 ; phyRegCnt < phyRegN; phyRegCnt++)
    {
        regsAddrPtr->GOP.SMI[instanceCnt].PHYAddress[phyRegCnt] = 0x0000000C + 0x4*phyRegCnt;
    }
}

/**
* @internal prv_smiInit function
* @endinternal
*
* @brief   init the DB - SMI unit
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None
*/
static void prv_smiInit
(
    IN  GT_U8  devNum
)
{
    GT_U32 i;
    GT_U32 instanceCnt;
    GT_U32  baseAddr;
    PRV_CPSS_DXCH_PP_CONFIG_STC * pDev;
    PRV_CPSS_DXCH_UNIT_ENT  smiUnitId;

    pDev = PRV_CPSS_DXCH_PP_MAC(devNum);
    for(i = 0 ; pDev->hwInfo.smi_support.activeSMIList[i] != (GT_U32)CPSS_PHY_SMI_INTERFACE_INVALID_E; i++)
    {
        instanceCnt = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.smi_support.activeSMIList[i];
        prv_smiInstanceInit(devNum,instanceCnt);

        if(!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            baseAddr = prvCpssDxChHwUnitBaseAddrGet(devNum,(PRV_CPSS_DXCH_UNIT_ENT)(PRV_CPSS_DXCH_UNIT_SMI_0_E + instanceCnt),NULL);
        }
        else
        {
            /* in Falcon 12.8 :
               explicit unitId to support 'SMI 2' and SMI 3' that exists as units in tile 2 :
               SMI 2 = PRV_CPSS_DXCH_UNIT_SMI_0_E + PRV_CPSS_DXCH_UNIT_DISTANCE_BETWEEN_TILES_CNS
               SMI 3 = PRV_CPSS_DXCH_UNIT_SMI_1_E + PRV_CPSS_DXCH_UNIT_DISTANCE_BETWEEN_TILES_CNS
            */
            smiUnitId   = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.smi_support.sip6_smiUnitIdArr[i];
            baseAddr = prvCpssDxChHwUnitBaseAddrGet(devNum,smiUnitId,NULL);
        }


        /* set the addresses of the SMI unit registers according to the needed base address */
        regUnitBaseAddrSet(
            REG_UNIT_INFO_MAC(devNum,baseAddr,GOP.SMI[instanceCnt]));
    }
}



/**
* @internal prv_ledInstanceInit function
* @endinternal
*
* @brief   init the DB - LMS unit
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] instanceCnt              - CNC instance number
*                                       None
*/
static void prv_ledInstanceInit
(
    IN  GT_U8  devNum,
    IN  GT_U32 instanceCnt
)
{
    GT_U32  ii;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_U32  offset = 0;

    GT_U32 portIndicationSelectN;
    GT_U32 portTypeConfigN;
    GT_U32 portIndicationPolarityN;
    /* NOTE: this function NOT applicable for bc2_A0 !!! */

    typedef struct
    {
        GT_U32 portIndicationSelectRegN;
        GT_U32 portTypeConfigRegN;
        GT_U32 portIndicationPolarityRegN;
    }LED_IF_REG_NUM_STC;

    static LED_IF_REG_NUM_STC aldrinIfRegNArr[PRV_CPSS_DXCH_ALDRIN_LED_IF_NUM_CNS] =
    {
         { PRV_CPSS_DXCH_ALDRIN_LED_IF_0_PORTS_NUM_CNS, PRV_CPSS_DXCH_ALDRIN_LED_IF_0_PORTS_TYPE_CONF_REG_NUM_CNS, PRV_CPSS_DXCH_ALDRIN_LED_IF_0_PORTS_IND_POLARITY_REG_NUM_CNS }
        ,{ PRV_CPSS_DXCH_ALDRIN_LED_IF_1_PORTS_NUM_CNS, PRV_CPSS_DXCH_ALDRIN_LED_IF_1_PORTS_TYPE_CONF_REG_NUM_CNS, PRV_CPSS_DXCH_ALDRIN_LED_IF_1_PORTS_IND_POLARITY_REG_NUM_CNS }
    };


    static LED_IF_REG_NUM_STC bc3IfRegNArr[PRV_CPSS_DXCH_BOBCAT3_LED_IF_NUM_CNS] =
    {
         { PRV_CPSS_DXCH_BOBCAT3_LED_IF_0_PORTS_NUM_CNS, PRV_CPSS_DXCH_BOBCAT3_LED_IF_0_PORTS_TYPE_CONF_REG_NUM_CNS, PRV_CPSS_DXCH_BOBCAT3_LED_IF_0_PORTS_IND_POLARITY_REG_NUM_CNS }
        ,{ PRV_CPSS_DXCH_BOBCAT3_LED_IF_1_PORTS_NUM_CNS, PRV_CPSS_DXCH_BOBCAT3_LED_IF_1_PORTS_TYPE_CONF_REG_NUM_CNS, PRV_CPSS_DXCH_BOBCAT3_LED_IF_1_PORTS_IND_POLARITY_REG_NUM_CNS }
        ,{ PRV_CPSS_DXCH_BOBCAT3_LED_IF_2_PORTS_NUM_CNS, PRV_CPSS_DXCH_BOBCAT3_LED_IF_2_PORTS_TYPE_CONF_REG_NUM_CNS, PRV_CPSS_DXCH_BOBCAT3_LED_IF_2_PORTS_IND_POLARITY_REG_NUM_CNS }
        ,{ PRV_CPSS_DXCH_BOBCAT3_LED_IF_3_PORTS_NUM_CNS, PRV_CPSS_DXCH_BOBCAT3_LED_IF_3_PORTS_TYPE_CONF_REG_NUM_CNS, PRV_CPSS_DXCH_BOBCAT3_LED_IF_3_PORTS_IND_POLARITY_REG_NUM_CNS }
    };
    static LED_IF_REG_NUM_STC aldrin2IfRegNArr[PRV_CPSS_DXCH_BOBCAT3_LED_IF_NUM_CNS] =
    {
         { 12, 1, 3 } /*DP[0] local ports 0..11 */
        ,{ 12, 1, 3 } /*DP[1] local ports 0..11*/
        ,{ 24, 2, 6 } /*DP[2] local ports 0..23*/
        ,{ 25, 2, 7 } /*DP[3] local ports 0..23 , DP[0] local port 24 */
    };

#if 0
    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E)
    {
        if(PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_NONE_E &&   /* BC2_B0*/
           instanceCnt == 3)
        {
            /* strange offset for those addresses ...
               take the offset with in the lower 24 bits */
            offset = 0x00FFFFFF & (0xbc00000 * instanceCnt);
        }
    }
#endif

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E) /* all BC2 B0 BobK Cetus/ Caelum */
    {
        portIndicationSelectN   = 12;
        portTypeConfigN         = 1;
        portIndicationPolarityN = 3;
    }
    else if (PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(devNum))
    {
        portIndicationSelectN   = aldrinIfRegNArr[instanceCnt].portIndicationSelectRegN;
        portTypeConfigN         = aldrinIfRegNArr[instanceCnt].portTypeConfigRegN;
        portIndicationPolarityN = aldrinIfRegNArr[instanceCnt].portIndicationPolarityRegN;
    }
    else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
    {
        portIndicationSelectN   = bc3IfRegNArr[instanceCnt].portIndicationSelectRegN;
        portTypeConfigN         = bc3IfRegNArr[instanceCnt].portTypeConfigRegN;
        portIndicationPolarityN = bc3IfRegNArr[instanceCnt].portIndicationPolarityRegN;
    }
    else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
    {
        /* currently Falcon as BC3 */
        CPSS_TBD_BOOKMARK_FALCON
        portIndicationSelectN   = bc3IfRegNArr[instanceCnt].portIndicationSelectRegN;
        portTypeConfigN         = bc3IfRegNArr[instanceCnt].portTypeConfigRegN;
        portIndicationPolarityN = bc3IfRegNArr[instanceCnt].portIndicationPolarityRegN;
    }
    else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
    {
        portIndicationSelectN   = aldrin2IfRegNArr[instanceCnt].portIndicationSelectRegN;
        portTypeConfigN         = aldrin2IfRegNArr[instanceCnt].portTypeConfigRegN;
        portIndicationPolarityN = aldrin2IfRegNArr[instanceCnt].portIndicationPolarityRegN;
    }
    else
    {
        /* yet unknown ASIC */
        return;
    }



    /* --------------------------------------*/
    /* process with device                   */
    /* --------------------------------------*/
    regsAddrPtr->GOP.LED[instanceCnt].LEDControl                 = offset + 0x00000000;
    regsAddrPtr->GOP.LED[instanceCnt].blinkGlobalControl         = offset + 0x00000004;
    regsAddrPtr->GOP.LED[instanceCnt].classGlobalControl         = offset + 0x00000008;
    regsAddrPtr->GOP.LED[instanceCnt].classesAndGroupConfig      = offset + 0x0000000C;
    for(ii = 0 ; ii < 6 ; ii++)
    {
        regsAddrPtr->GOP.LED[instanceCnt].classForcedData[ii] =
            offset + 0x00000010 + 4*ii;
    }
    for(ii = 0 ; ii < portIndicationSelectN ; ii++)
    {
        regsAddrPtr->GOP.LED[instanceCnt].portIndicationSelect[ii] =
            offset + 0x00000028 + 4*ii;
    }
    for(ii = 0 ; ii < portTypeConfigN ; ii++)
    {
        regsAddrPtr->GOP.LED[instanceCnt].portTypeConfig[ii] =
            offset + 0x00000120 + 4*ii;
    }
    for(ii = 0 ; ii < portIndicationPolarityN ; ii++)
    {
        regsAddrPtr->GOP.LED[instanceCnt].portIndicationPolarity[ii] =
            offset + 0x00000128 + 4*ii;
    }
    for(ii = 0 ; ii < 6 ; ii++)
    {
        regsAddrPtr->GOP.LED[instanceCnt].classRawData[ii] =
            offset + 0x00000148 + 4*ii;
    }
}

/**
* @internal prv_ledInit function
* @endinternal
*
* @brief   init the DB - LED unit
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                      instance    - CNC instance number
*                                       None
*/
static void prv_ledInit
(
    IN  GT_U8  devNum
)
{
    GT_U32 baseAddr;
    GT_U32 instanceCnt;
    GT_U32 instanceN;
    GT_U32 defInstN;

    instanceN = prvCpssLedStreamNumOfInterfacesInPortGroupGet(devNum);
    if (instanceN == 0)
    {
        /*-------------------------------------------*
         *  Undefined ASIC : LED API will not work
         *-------------------------------------------*/
        return;
    }

    defInstN = sizeof(PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.LED)/sizeof(PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.LED[0]);

    if (instanceN > defInstN)
    {
        /*------------------------------------------------------------------------------------
         *  LED : number of instances in PRV_CPSS_DXCH_DEV_REGS_VER1_MAC is insufficent :
         *  DB wrong : LED API will not work
         *-------------------------------------------------------------------------------------*/
        return;
    }
    for (instanceCnt = 0 ; instanceCnt < instanceN ; instanceCnt++)
    {
        prv_ledInstanceInit(devNum,instanceCnt);
        /* set the addresses of the SMI unit registers according to the needed base address */
        baseAddr = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_LED_0_E + instanceCnt,NULL);
        regUnitBaseAddrSet(
            REG_UNIT_INFO_MAC(devNum,baseAddr,GOP.LED[instanceCnt]));
    }
}


/**
* @internal mgInit function
* @endinternal
*
* @brief   init the DB - MG unit
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None
*/
static void mgInit
(
    IN  GT_U8  devNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_U32  offset;
    GT_U32  nMax;
    GT_U32  ii;

    if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
    {
        offset = 0x00030000;
    }
    else
    {
        offset = 0x00040000;
    }

    {
        GT_U32    n;

        if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
        {
            /* single XSMI per MG */
            nMax = 0;
        }
        else
        {
            nMax = 1;
        }

        for (n = 0; n <= nMax; n++)
        {
            regsAddrPtr->MG.XSMI[n].XSMIManagement =
                offset + 0x00000000 + (n * 0x2000);
            regsAddrPtr->MG.XSMI[n].XSMIAddress =
                offset + 0x00000008 + (n * 0x2000);
            regsAddrPtr->MG.XSMI[n].XSMIConfiguration =
                offset + 0x0000000C + (n * 0x2000);
            regsAddrPtr->MG.XSMI[n].XSMIInterruptCause =
                offset + 0x00000010 + (n * 0x2000);
            regsAddrPtr->MG.XSMI[n].XSMIInterruptMask =
                offset + 0x00000014 + (n * 0x2000);
        }
    }

    /* DFX interrupts summary registers */
    PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.globalInterrupt.dfxInterruptCause  = 0x000000AC;
    PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.globalInterrupt.dfxInterruptMask   = 0x000000B0;
    PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.globalInterrupt.dfx1InterruptCause = 0x000000B8;
    PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.globalInterrupt.dfx1InterruptMask  = 0x000000BC;

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        /* Address Decoding */
        /* next registers are set during initialization by the 'uboot'/external driver */
        /* (from out side the CPSS) , but the CPSS DB need to know them for :
            1. 'soft reset' - need to restore those values.
            2. multi-MG support : the CPSS duplicate MG[0] to all others MGs
        */
        for (ii = 0; ii < 6; ii++)
        {
            regsAddrPtr->MG.addressDecoding.baseAddress[ii] =       0x0000020C + ii * 8;
            regsAddrPtr->MG.addressDecoding.windowSize[ii]        =       0x00000210 + ii * 8;
            regsAddrPtr->MG.addressDecoding.highAddressRemap[ii] =  0x000023C + ii * 4;
            /* Window Control Register */
            regsAddrPtr->MG.addressDecoding.windowControl[ii]=  0x0000254 + ii * 4;
        }

        regsAddrPtr->MG.addressDecoding.UnitDefaultID   = 0x0000204;

        /* Co-Processor*/
#ifndef ASIC_SIMULATION_NO_HW
        regsAddrPtr->MG.confiProcessor.memoryBase = 0x80000;
#endif
    }
}

/**
* @internal erepInit function
* @endinternal
*
* @brief   init the DB - EREP unit
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None
*/
static void erepInit
(
    IN  GT_U8 devNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum) == 0)
    {
        return;
    }

    regsAddrPtr->EREP.configurations.pchHeaderEnableBase           = 0x00000000; /*1024 registers*/
    regsAddrPtr->EREP.configurations.replicationsArbiterConfigs    = 0x00001000;
    regsAddrPtr->EREP.configurations.qcnConfigs0                   = 0x00001004;
    regsAddrPtr->EREP.configurations.qcnConfigs1                   = 0x00001008;
    regsAddrPtr->EREP.configurations.dropCodeConfigs               = 0x0000100C;
    regsAddrPtr->EREP.configurations.hbuArbiterConfigs             = 0x00001010;
    regsAddrPtr->EREP.configurations.erepGlobalConfigs             = 0x00001014;
    regsAddrPtr->EREP.configurations.dropCodeStatus                = 0x00001018;

    regsAddrPtr->EREP.counters.qcnOutgoingCounter                           = 0x00002000;
    regsAddrPtr->EREP.counters.sniffOutgoingCounter                         = 0x00002004;
    regsAddrPtr->EREP.counters.trapOutgoingCounter                          = 0x00002008;
    regsAddrPtr->EREP.counters.mirrorOutgoingCounter                        = 0x0000200C;
    regsAddrPtr->EREP.counters.qcnFifoFullDroppedPacketsCounter             = 0x00002010;
    regsAddrPtr->EREP.counters.sniffFifoFullDroppedPacketsCounter           = 0x00002014;
    regsAddrPtr->EREP.counters.trapFifoFullDroppedPacketsCounter            = 0x00002018;
    regsAddrPtr->EREP.counters.mirrorFifoFullDroppedPacketsCounter          = 0x0000201C;
    regsAddrPtr->EREP.counters.erepHeaderExceedCounter                      = 0x00002020;
    regsAddrPtr->EREP.counters.incNotReadyCounter                           = 0x00002024;
    regsAddrPtr->EREP.counters.mirrorReplicationCancelledDueIncBackpressure = 0x00002028;
    regsAddrPtr->EREP.counters.sniffReplicationCancelledDueIncBackpressure  = 0x0000202C;
    regsAddrPtr->EREP.counters.trapReplicationCancelledDueIncBackpressure   = 0x00002030;
    regsAddrPtr->EREP.counters.hbuPacketsOutgoingForwardCounter             = 0x00002034;

    regsAddrPtr->EREP.interrupts.erepInterruptsCause   = 0x00003000;
    regsAddrPtr->EREP.interrupts.erepInterruptsMask    = 0x00003004;
}

/**
* @internal preQInit function
* @endinternal
*
* @brief   init the DB - PREQ unit
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None
*/
static void preQInit
(
    IN  GT_U8 devNum
)
{
    GT_U32 ii;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum) == 0)
    {
        return;
    }

     regsAddrPtr->PREQ.globalConfig                 = 0x00000000;
     regsAddrPtr->PREQ.globalTailDropLimit          = 0x00000404;
     regsAddrPtr->PREQ.pool0TailDropLimit           = 0x0000040C;
     regsAddrPtr->PREQ.pool1TailDropLimit           = 0x00000410;
     regsAddrPtr->PREQ.mcAvailableBuffers           = 0x00000414;
     regsAddrPtr->PREQ.globalTailDropConfig         = 0x0000042C;
     regsAddrPtr->PREQ.fbMin                        = 0x0000034C;
     regsAddrPtr->PREQ.fbMax                        = 0x00000350;
     regsAddrPtr->PREQ.mcTDConfigurationLowPrio     = 0x0000041C;
     regsAddrPtr->PREQ.mcTDConfigurationHighPrio    = 0x00000420;
     regsAddrPtr->PREQ.tdPool0AvailableBuffers      = 0x00000424;
     regsAddrPtr->PREQ.tdPool1AvailableBuffers      = 0x00000428;
     regsAddrPtr->PREQ.queueMaxLimitConfig          = 0x00000430;
     regsAddrPtr->PREQ.globalQcnConfig              = 0x00000340;
     regsAddrPtr->PREQ.qcnPool0AvailableBuffers     = 0x00000344;
     regsAddrPtr->PREQ.qcnPool1AvailableBuffers     = 0x00000348;

     regsAddrPtr->PREQ.mirroringGlobalConfig        = 0x0000010C;

    for (ii = 0; (ii < 32); ii++)
    {
        regsAddrPtr->PREQ.qcnTrigger[ii] = 0x00000358 + (ii * 4);
    }

    for (ii = 0; (ii < 128); ii++)
    {
        regsAddrPtr->PREQ.portMapping[ii] = 0x00000110 + (ii * 4);
    }

    for (ii = 0; (ii < 2); ii++)
    {
        regsAddrPtr->PREQ.egrMIBCntrs.egrMIBCntrsPortSetConfig[ii]   = 0x00000708 + (ii * 4);
        regsAddrPtr->PREQ.egrMIBCntrs.egrMIBCntrsSetConfig[ii]       = 0x00000700 + (ii * 4);
        regsAddrPtr->PREQ.egrMIBCntrs.egrMIBTailDroppedPktCntr[ii]   = 0x00000750 + (ii * 4);
        regsAddrPtr->PREQ.egrMIBCntrs.egrMIBOutgoingUcPktCntr[ii]    = 0x00000710 + (ii * 4);
        regsAddrPtr->PREQ.egrMIBCntrs.egrMIBOutgoingMcPktCntr[ii]    = 0x00000720 + (ii * 4);
        regsAddrPtr->PREQ.egrMIBCntrs.egrMIBOutgoingBcPktCntr[ii]    = 0x00000730 + (ii * 4);
        regsAddrPtr->PREQ.egrMIBCntrs.egrMIBCtrlPktCntr[ii]          = 0x00000760 + (ii * 4);
    }
}

/**
* @internal bcat2MacMibCountersInit function
* @endinternal
*
* @brief   Init MIB MAC counters memory address.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - on bad state of logic
*/
static GT_STATUS bcat2MacMibCountersInit
(
    IN  GT_U8 devNum
)
{
    GT_STATUS                       rc;/* return code */
    PRV_CPSS_DXCH_PP_REGS_ADDR_STC  *addrPtr;
    GT_U32                          i;          /* iterator */
    GT_U32                          mibBaseAddr, /* mib memory base address */
                                    mibOffset; /* offset per port */
    GT_U32                          portType;/*iterator*/
    GT_U32                          portIndex;/* 'local port' index (in case of multi-pipe) */
    GT_U32                          pipeIndex;/* pipe index (in case of multi-pipe) */

    addrPtr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum);
    mibBaseAddr = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_MIB_E,NULL);

    for (i = 0; i < PRV_CPSS_PP_MAC(devNum)->numOfPorts; i++)
    {
        /* destroy unused GOP registers */
        DESTROY_SINGLE_REG_MAC(addrPtr->macRegs.perPortRegs[i].externalUnitsIntmask);
        for(portType = 0 ; portType < PRV_CPSS_PORT_NOT_APPLICABLE_E ; portType++)
        {
            DESTROY_SINGLE_REG_MAC(addrPtr->macRegs.perPortRegs[i].macRegsPerType[portType].pcs40GCommonCtrl);
            DESTROY_SINGLE_REG_MAC(addrPtr->macRegs.perPortRegs[i].macRegsPerType[portType].pcs40GCommonStatus);
            DESTROY_SINGLE_REG_MAC(addrPtr->macRegs.perPortRegs[i].macRegsPerType[portType].pcs40GGearBoxStatus);
            DESTROY_SINGLE_REG_MAC(addrPtr->macRegs.perPortRegs[i].macRegsPerType[portType].pcs40GFecDecStatus);
        }

        /* skip non exists MAC ports */
        if(!PRV_CPSS_PHY_PORT_IS_EXIST_MAC(devNum, i))
        {
            /* destroy unused GOP registers that initialized during 'legacy code' */
            DESTROY_STC_MAC(addrPtr->macRegs.perPortRegs[i]);

            continue;
        }

        if(PRV_CPSS_IS_MULTI_PIPES_DEVICE_MAC(devNum))
        {
            rc = prvCpssDxChHwPpGopGlobalMacPortNumToLocalMacPortInPipeConvert(devNum,
                i/*global port*/,
                &pipeIndex,
                &portIndex/* local port */);
            if(rc != GT_OK)
            {
                return rc;
            }

            if(pipeIndex != 0)
            {
                /* function updateGopOldRegDbWithMultiPipe(...) will set addresses of
                   pipe 1 according to those of pipe 0 + the proper 'pipe 1' offset*/
                continue;
            }
        }
        else
        {
            portIndex = i;
        }

        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
        {
            hwsAldrin2MibAddrCalc(portIndex/*port number*/,0/*relative offset*/,&mibOffset);
            /* remove the base Address of the MIB */
            mibOffset &= 0x00FFFFFF;
        }
        else
        {
#ifdef GM_USED
            mibOffset = portIndex * 0x400;
#else
            if(portIndex < 56)
            {
                mibOffset = portIndex * 0x400;
            }
            else
            {
                mibOffset = (portIndex-56) * 0x400;
                mibOffset += 0x800000;
            }
#endif /* GM_USED */
        }

        addrPtr->macRegs.perPortRegs[i].macCounters = mibBaseAddr + mibOffset;
    }

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_TRUE)
    {
        /* destroy other legacy mac registers */
        DESTROY_STC_MAC(addrPtr->macRegs.cpuPortRegs);
    }

    return GT_OK;
}

/**
* @internal bobkDMAGluePizzaArbiterUnitRegOffsetInit function
* @endinternal
*
* @brief   bobk DMA Glue Pizza Arbiter Init
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2.
*
* @param[in] pizzaArbiterPtr          - pointer to glue pizza unit in register structure
* @param[in] offset                   - ofsset of unit
*                                       None.
*/
GT_STATUS bobkDMAGluePizzaArbiterUnitRegOffsetInit
(
    IN PRV_CPSS_DXCH_PP_DP_GLUE_PIZZA_ARBITER_STC * pizzaArbiterPtr,
    IN GT_U32 offset,
    IN GT_U32 regNum
)
{
    GT_U32 i;

    if (regNum > sizeof(pizzaArbiterPtr->pizzaArbiterConfigReg)/sizeof(pizzaArbiterPtr->pizzaArbiterConfigReg[0]))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    pizzaArbiterPtr->pizzaArbiterCtrlReg   = offset + 0x0000000;
    pizzaArbiterPtr->pizzaArbiterStatusReg = offset + 0x0000004;
    for (i = 0; i < regNum; i++)
    {
        pizzaArbiterPtr->pizzaArbiterConfigReg[i] = offset + 0x0000008 + i*4;
    }
    return GT_OK;
}


/**
* @internal bobkRxDMAGlueInit function
* @endinternal
*
* @brief   Init RX-DMA-GLUE register Init
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None.
*/
GT_STATUS bobkRxDMAGlueInit
(
    GT_U8 devNum,
    GT_U32 regNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC  *regsAddrVer1;
    regsAddrVer1 = &PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1;
    return bobkDMAGluePizzaArbiterUnitRegOffsetInit(&regsAddrVer1->RX_DMA_GLUE.pizzaArbiter,0,regNum);
}



/**
* @internal bobkTxDMAGlueInit function
* @endinternal
*
* @brief   Init RX-DMA-GLUE register Init
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None.
*/
GT_STATUS bobkTxDMAGlueInit
(
    GT_U8 devNum,
    GT_U32 regNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC  *regsAddrVer1;
    regsAddrVer1 = &PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1;
    return bobkDMAGluePizzaArbiterUnitRegOffsetInit(&regsAddrVer1->TX_DMA_GLUE.pizzaArbiter,0,regNum);
}



/**
* @internal mppmInit function
* @endinternal
*
* @brief   Init MPPM unit registers
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None.
*/
static GT_VOID mppmInit
(
    GT_U8 devNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC  *regsAddrVer1;

    regsAddrVer1 = &PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1;

    regsAddrVer1->MPPM.dataIntegrity.lastFailingBuffer     = 0x00000510;
    regsAddrVer1->MPPM.dataIntegrity.lastFailingSegment    = 0x00000514;
    regsAddrVer1->MPPM.dataIntegrity.statusFailedSyndrome  = 0x00000518;
}


/**
* @internal bobkMPPMInit function
* @endinternal
*
* @brief   Init RX-DMA-GLUE register Init
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None.
*/
GT_STATUS bobkMPPMInit
(
    GT_U8 devNum,
    GT_U32 regNum
)
{
    GT_U32 i;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC  *regsAddrVer1;
    PRV_CPSS_DXCH_PP_MPPM_PIZZA_ARBITER_STC *pizzaArbiterPtr;

    mppmInit(devNum);

    regsAddrVer1 = &PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1;
    pizzaArbiterPtr = &regsAddrVer1->MPPM.pizzaArbiter;

    if (regNum > sizeof(pizzaArbiterPtr->pizzaArbiterConfigReg)/sizeof(pizzaArbiterPtr->pizzaArbiterConfigReg[0]))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    pizzaArbiterPtr->pizzaArbiterCtrlReg   = 0x0000200;
    pizzaArbiterPtr->pizzaArbiterStatusReg = 0x0000204;
    for (i = 0; i < regNum; i++)
    {
        pizzaArbiterPtr->pizzaArbiterConfigReg[i] = 0x0000208 + i*4;
    }
    return GT_OK;
}


/**
* @internal prvCpssDxChHwRegAddrToUnitIdConvert function
* @endinternal
*
* @brief   This function convert register address to unit id.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] regAddr                  - the register address to get it's base address unit Id.
*                                       the unitId for the given address
*/
PRV_CPSS_DXCH_UNIT_ENT prvCpssDxChHwRegAddrToUnitIdConvert
(
    IN GT_U32                   devNum,
    IN GT_U32                   regAddr
)
{
    GT_U32  *unitsBaseAddrPtr = NULL;
    GT_U32  regAddrBaseAddr;
    GT_U32  ii;
    PRV_CPSS_DXCH_UNIT_ENT  *unitIdPtr = NULL;

    switch(devNum)
    {
        case LION2_DEVICE_INDICATION_CNS:
            /* indication the we need Lion2 DB regardless to the actual devNum */
            goto forceLion2_lbl;
        case BOBK_DEVICE_INDICATION_CNS:
            /* indication the we need Bobk DB regardless to the actual devNum */
            goto forceBobk_lbl;
        case BC3_DEVICE_INDICATION_CNS:
            /* indication the we need BC3 DB regardless to the actual devNum */
            goto forceBc3_lbl;
        case FALCON_DEVICE_INDICATION_CNS:
            /* indication the we need Falcon DB regardless to the actual devNum */
            goto forceFalcon_lbl;
        default:
            break;
    }

    if(devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS ||
       NULL == prvCpssDrvPpConfig[devNum])
    {
        /* error */
        return PRV_CPSS_DXCH_UNIT_LAST_E;
    }

    switch(prvCpssDrvPpConfig[devNum]->devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_LION2_E:
            forceLion2_lbl:
            unitsBaseAddrPtr = prvDxChLion2UnitsBaseAddr;
            break;
        case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
            forceBobk_lbl:
            unitIdPtr = prvDxChBobcat2UnitIdPer8MSBitsBaseAddrArr;
            break;
        case CPSS_PP_FAMILY_DXCH_BOBCAT3_E:
            forceBc3_lbl:
            unitIdPtr = prvDxChBobcat3UnitIdPer8MSBitsBaseAddrArr;
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            unitIdPtr = prvDxChAldrin2UnitIdPer8MSBitsBaseAddrArr;
            break;
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            forceFalcon_lbl:
            return prvCpssDxChFalconHwRegAddrToUnitIdConvert(devNum,regAddr);
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
        case CPSS_PP_FAMILY_DXCH_AC3X_E:
            unitIdPtr = prvDxChAldrinUnitIdPer8MSBitsBaseAddrArr;
            break;
        default:
            /* error */
            return PRV_CPSS_DXCH_UNIT_LAST_E;
    }

    if(unitIdPtr)
    {
        return unitIdPtr[(regAddr >> 24)];
    }
    else if(unitsBaseAddrPtr)
    {
        regAddrBaseAddr = regAddr & UNIT_MASK_9_MSB_CNS;

        for(ii = 0 ; ii < PRV_CPSS_DXCH_UNIT_LAST_E ; ii++,unitsBaseAddrPtr++)
        {
            if(regAddrBaseAddr == (*unitsBaseAddrPtr))
            {
                /*found the unit*/
                return ii;
            }
        }
    }

    return PRV_CPSS_DXCH_UNIT_LAST_E;
}

/**
* @internal convertRegAddrToNewUnitsBaseAddr function
* @endinternal
*
* @brief   the assumption is that the 'old address' is based on Lion2 units base addresses !
*         so need to convert it to address space of current device
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in,out] oldNewRegPtr             - pointer to the 'old' reg address (from Lion2)
* @param[in,out] oldNewRegPtr             - pointer to the 'new' reg address - after aligned to the current device.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_CPU_MEM        - on malloc failed
* @retval GT_BAD_PARAM             - wrong device type to operate
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS convertRegAddrToNewUnitsBaseAddr
(
    IN GT_U8    devNum,
    INOUT GT_U32 *oldNewRegPtr
)
{
    GT_BOOL     error;
    PRV_CPSS_DXCH_UNIT_ENT currenyUnitId;
    GT_U32      unitMask = UNIT_MASK_9_MSB_CNS;/* mask of the 9 MSBits */
    GT_U32      currentAddr,currentUnitBaseAddr;
    GT_U32      newBaseAddr;

    currentAddr = *oldNewRegPtr;

    currentUnitBaseAddr = currentAddr & unitMask;

    /* look for the ID of the current unit */
    currenyUnitId =
        prvCpssDxChHwRegAddrToUnitIdConvert(LION2_DEVICE_INDICATION_CNS/*lion2 indication!!!*/,
            currentAddr);
    if(currenyUnitId >= PRV_CPSS_DXCH_UNIT_LAST_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    newBaseAddr = prvCpssDxChHwUnitBaseAddrGet(devNum,currenyUnitId,&error);
    if(error == GT_TRUE )
    {
        /* assume that this unit is not supported by the current device
           so 'invalidate' this register ! */
        *oldNewRegPtr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
        return GT_OK;
    }

    /* update the address value */
    *oldNewRegPtr = currentAddr - currentUnitBaseAddr + newBaseAddr;

    return GT_OK;
}

static GT_BOOL sip6DontConvertAddr
(
    IN GT_U8    devNum,
    IN GT_U32   regAddr
)
{

#ifndef FALCON_ASIC_SIMULATION
    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum)) {/*SD area*/
        if (((regAddr & 0xFF0000) == 0x340000) ||
            ((regAddr & 0xFF0000) == 0x350000))
        {
            return GT_TRUE;
        }
        else
        {
            return GT_FALSE;
        }
    }
    else

#else
    devNum = devNum;
    regAddr = regAddr;
#endif
    {
        return GT_FALSE;
    }
}


/**
* @internal convertOldRegDbToNewUnitsBaseAddr_limitedRange function
* @endinternal
*
* @brief   the rest of the registers that we not manage to 'destroy' in the old DB.
*         we need to convert align their addresses to base addresses of the actual
*         corresponding units
*         the assumption is that the 'old DB' is based on Lion2 units base addresses !
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] oldRegPtr                - pointer to the old reg DB that need to be converted
* @param[in] numOfEntries             - number of entries in oldRegPtr
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_CPU_MEM        - on malloc failed
* @retval GT_BAD_PARAM             - wrong device type to operate
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS convertOldRegDbToNewUnitsBaseAddr_limitedRange
(
    IN GT_U8    devNum,
    IN GT_U32 *oldRegPtr,
    IN GT_U32 numOfEntries
)
{
    GT_U32       *regsAddrPtr32;
    GT_U32        regsAddrPtr32Size;
    GT_U32      ii;
    GT_U32      *currentSwPtr;
    GT_STATUS   rc;
    PRV_CPSS_BOOKMARK_STC *bookmark;
    GT_U32  currentAddr;
    GT_U32      currentNumOfEntries;

    regsAddrPtr32 = oldRegPtr;
    regsAddrPtr32Size = numOfEntries;

    for( ii = 0; ii < regsAddrPtr32Size; ii++ )
    {
        currentAddr = regsAddrPtr32[ii];
        if(currentAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            continue;
        }
        if (sip6DontConvertAddr(devNum, currentAddr)) {
            continue;
        }
        if (currentAddr == PRV_CPSS_SW_PTR_BOOKMARK_CNS)
        {
            /*
                GT_U32      swPortControlBookmark;  [0]
                GT_U32      swPortControlType;      [1]
                GT_U32      swPortControlSize;      [2]
                GT_U32      *portControl GT_PACKED; [3]
            */
            bookmark = (PRV_CPSS_BOOKMARK_STC *)(&(regsAddrPtr32[ii]));

            if(bookmark->type & PRV_CPSS_SW_TYPE_WRONLY_CNS)
            {
                /* there is no 'pointer' to registers array , just the next N
                   registers are indicated as 'write only'
                */

                /* step over the bookmark and lets continue from there ...
                    BUT there is no field of "'GT_U32  *nextPtr" */
                ii += ((sizeof(PRV_CPSS_BOOKMARK_STC) - sizeof(bookmark->nextPtr))/sizeof(GT_U32)) - 1;
            }
            else
            {
                currentNumOfEntries =  bookmark->size / sizeof(GT_U32);
                currentSwPtr        = bookmark->nextPtr;
                /* recursive call */
                rc = convertOldRegDbToNewUnitsBaseAddr_limitedRange(devNum,currentSwPtr,currentNumOfEntries);
                if(rc != GT_OK)
                {
                    return rc;
                }

                /* step over the bookmark and lets continue from there */
                ii += (sizeof(PRV_CPSS_BOOKMARK_STC)/sizeof(GT_U32)) - 1;
            }

            continue;
        }

        rc = convertRegAddrToNewUnitsBaseAddr(devNum,&regsAddrPtr32[ii]);
        if(rc != GT_OK)
        {
            return rc;
        }

    }

    return GT_OK;

}

/**
* @internal convertOldRegDbToNewUnitsBaseAddr function
* @endinternal
*
* @brief   the rest of the registers that we not manage to 'destroy' in the old DB.
*         we need to convert align their addresses to base addresses of the actual
*         corresponding units
*         the assumption is that the 'old DB' is based on Lion2 units base addresses !
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_CPU_MEM        - on malloc failed
* @retval GT_BAD_PARAM             - wrong device type to operate
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS convertOldRegDbToNewUnitsBaseAddr
(
    IN  GT_U8 devNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum);
    GT_U32       *regsAddrPtr32;
    GT_U32        regsAddrPtr32Size;

    /*Fill all the words in the struct with initial value*/
    regsAddrPtr32 = (GT_U32*)regsAddrPtr;
    regsAddrPtr32Size = sizeof(PRV_CPSS_DXCH_PP_REGS_ADDR_STC) / 4;

    return convertOldRegDbToNewUnitsBaseAddr_limitedRange(devNum,regsAddrPtr32,regsAddrPtr32Size);


}


/**
* @internal prvCpssDxChNonSip5HwRegAddrVer1Init function
* @endinternal
*
* @brief   This function initializes the registers structure for pre eArch devices.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
*/
GT_STATUS prvCpssDxChNonSip5HwRegAddrVer1Init
(
    IN  GT_U8 devNum
)
{
    /* reset the DB */
    dbReset(devNum);

    return GT_OK;
}


/**
* @internal prvCpssDxChHwRegAddrIsAddrInTableArray function
* @endinternal
*
* @brief   check if register address is in one of the listed tables.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] infoArr                  - the array of listed tables to check if the register is part of it
* @param[in] regAddr                  - the register to check
*
* @retval GT_TRUE                  - the register is in the listed tables.
* @retval GT_FALSE                 - the register is not in the listed tables.
*/
GT_BOOL  prvCpssDxChHwRegAddrIsAddrInTableArray(
    IN GT_SW_DEV_NUM    devNum,
    IN PRV_CPSS_DXCH_HW_RED_ADDR_OFFSET_IN_TABLE_INFO_STC   *infoArr,
    IN GT_U32       regAddr
)
{
    PRV_CPSS_DXCH_TABLES_INFO_STC *tableInfoPtr; /* pointer to table info */
    PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC *tablePtr;/* pointer to direct table info */
    GT_U32  ii,offset,numOfEntries,baseAddress;
    GT_U32  addrMask = PRV_CPSS_SIP_6_CHECK_MAC(devNum) ? 0xFFFFFFFF : UNIT_OFFSET_MASK_CNS;

    regAddr &= addrMask;/* we compare only offset in the unit */


    for(ii = 0 ; infoArr[ii].tableType != CPSS_DXCH_TABLE_LAST_E ; ii ++)
    {
        tableInfoPtr = PRV_TABLE_INFO_PTR_GET_MAC(devNum,infoArr[ii].tableType);

        if(tableInfoPtr->writeAccessType == PRV_CPSS_DXCH_DIRECT_ACCESS_E)
        {
            tablePtr =
                 (PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC*)(tableInfoPtr->writeTablePtr);
        }
        else
        if(tableInfoPtr->readAccessType == PRV_CPSS_DXCH_DIRECT_ACCESS_E)
        {
            tablePtr =
                 (PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC*)(tableInfoPtr->readTablePtr);
        }
        else
        {
            continue;
        }

        if(tablePtr == NULL)
        {
            continue;
        }

        baseAddress = tablePtr->baseAddress & addrMask;

        numOfEntries = infoArr[ii].numOfEntries ?
                       infoArr[ii].numOfEntries :
                       tableInfoPtr->maxNumOfEntries;

        if( baseAddress                                             > regAddr ||
           (baseAddress + (tablePtr->step/*bytes*/ * numOfEntries)) <= regAddr)
        {
            /* not in range of this table */
            continue;
        }

        offset = regAddr - (tablePtr->baseAddress & addrMask);

        if((offset % tablePtr->step) < (tableInfoPtr->entrySize/*words*/ * 4))
        {
            /* we have a match. The address is with in this table range */
            return GT_TRUE;
        }

        /* do not break .... to support grouts that are not in steps of '4'
           between table entries

           allow to check in other tables
        */
    }

    return GT_FALSE;
}

/**
* @internal prvCpssDxChHwRegAddrIsAddrInRegDb1Array function
* @endinternal
*
* @brief   check if register address is in the array of registers
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] base_dbRegAddrPtr        - (pointer to) the start of the 'reg DB'
* @param[in] infoArr                  - the array of registers to check if the register is part of it
* @param[in] regAddr                  - the register to check
*
* @retval GT_TRUE                  - the register is in the array
* @retval GT_FALSE                 - the register is not in the array
*/
static GT_BOOL  regAddrIsAddrInRegDbGenericArray(
    IN GT_SW_DEV_NUM    devNum,
    GT_U32              *base_dbRegAddrPtr,
    IN GT_U32           infoAddrOffset,
    IN PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC   *infoArr,
    IN GT_U32       regAddr
)
{
    GT_U32  ii,jj;
    GT_U32  *dbRegAddrPtr;
    GT_U32  numOfRegisters;
    GT_U32  addrMask = PRV_CPSS_SIP_6_CHECK_MAC(devNum) ? 0xFFFFFFFF : UNIT_OFFSET_MASK_CNS;

    regAddr &= addrMask;/* we compare only offset in the unit */

    /* NEXT is needed for SIP6 that can't use 'mask' of the LSBits */
    /* the regAddr is in 'instance 0' of the unit */
    /* but the 'infoArr' point to address from 'instance 1' of the unit , so we need to align to it */
    regAddr += infoAddrOffset;

    for(ii = 0 ; infoArr[ii].registerOffsetPtr != END_OF_TABLE_CNS ; ii ++)
    {
        dbRegAddrPtr = base_dbRegAddrPtr + (infoArr[ii].registerOffsetPtr - (GT_U32*)NULL);
        numOfRegisters = infoArr[ii].numOfRegisters;

        if(numOfRegisters == 0)
        {
            /* place holder */
            continue;
        }

        /* assuming that the dbRegAddrPtr is in ascending order */
        if((dbRegAddrPtr[0] & addrMask               ) > regAddr ||
           (dbRegAddrPtr[numOfRegisters-1] & addrMask) < regAddr)
        {
            /* not in range of this group */
            continue;
        }

        for(jj = 0 ; jj < numOfRegisters; jj++)
        {
            if((dbRegAddrPtr[jj] & addrMask) == regAddr)
            {
                /* found */
                return GT_TRUE;
            }
        }

        /* do not break .... to support grouts that are not in steps of '4'
           between register addresses

           allow to check in other groups
        */
    }

    return GT_FALSE;
}

/**
* @internal prvCpssDxChHwRegAddrIsAddrInRegDb1Array function
* @endinternal
*
* @brief   check if register address is in the array of registers
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] infoArr                  - the array of registers to check if the register is part of it
* @param[in] regAddr                  - the register to check
*
* @retval GT_TRUE                  - the register is in the array
* @retval GT_FALSE                 - the register is not in the array
*/
GT_BOOL  prvCpssDxChHwRegAddrIsAddrInRegDb1Array(
    IN GT_SW_DEV_NUM    devNum,
    IN GT_U32           infoAddrOffset,
    IN PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC   *infoArr,
    IN GT_U32       regAddr
)
{
    GT_U32  *base_dbRegAddrPtr = (GT_PTR)PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    return regAddrIsAddrInRegDbGenericArray(devNum,base_dbRegAddrPtr,infoAddrOffset,infoArr,regAddr);
}

/**
* @internal prvCpssDxChHwRegAddrIsAddrInRegDbArray function
* @endinternal
*
* @brief   check if register address is in the array of registers (PRV_CPSS_DXCH_PP_REGS_ADDR_STC)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] infoAddrOffset           - offset of the addresses in infoArr[] from the 'instance 0' of the unit.
*                                      NOTE: should be 0 for sip5 devices.
*                                      should have meaningfully value for sip6 devices
* @param[in] infoArr                  - the array of registers to check if the register is part of it
* @param[in] regAddr                  - the register to check
*
* @retval GT_TRUE                  - the register is in the array
* @retval GT_FALSE                 - the register is not in the array
*/
GT_BOOL  prvCpssDxChHwRegAddrIsAddrInRegDbArray(
    IN GT_SW_DEV_NUM    devNum,
    IN GT_U32           infoAddrOffset,
    IN PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC   *infoArr,
    IN GT_U32       regAddr
)
{
    GT_U32  *base_dbRegAddrPtr = (GT_PTR)PRV_CPSS_DXCH_DEV_REGS_MAC(devNum);

    return regAddrIsAddrInRegDbGenericArray(devNum,base_dbRegAddrPtr,infoAddrOffset,infoArr,regAddr);
}
/**
* @internal prvCpssDxChHwRegAddrIsAddrInDfxRegDbArray function
* @endinternal
*
* @brief   check if register address is in the array of registers (PRV_CPSS_DXCH_RESET_AND_INIT_CTRL_REGS_ADDR_STC)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] infoAddrOffset           - offset of the addresses in infoArr[] from the 'instance 0' of the unit.
*                                      NOTE: should be 0 for sip5 devices.
*                                      should have meaningfully value for sip6 devices
* @param[in] infoArr                  - the array of registers to check if the register is part of it
* @param[in] regAddr                  - the register to check
*
* @retval GT_TRUE                  - the register is in the array
* @retval GT_FALSE                 - the register is not in the array
*/
GT_BOOL  prvCpssDxChHwRegAddrIsAddrInDfxRegDbArray(
    IN GT_SW_DEV_NUM    devNum,
    IN GT_U32           infoAddrOffset,
    IN PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC   *infoArr,
    IN GT_U32       regAddr
)
{
    GT_U32  *base_dbRegAddrPtr = (GT_PTR)PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum);

    return regAddrIsAddrInRegDbGenericArray(devNum,base_dbRegAddrPtr,infoAddrOffset,infoArr,regAddr);
}

/*check if register address is per port in RxDMA unit */
static GT_BOOL  bobcat3_mustNotDuplicate_rxdmaAddr(
    IN GT_U8        devNum,
    IN GT_U32       regAddr
)
{
    static PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC perPortAddrArr[]=
    {
         {REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(rxDMA[NON_FIRST_UNIT_INDEX_CNS].singleChannelDMAConfigs.SCDMAConfig0)                 , BC3_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(rxDMA[NON_FIRST_UNIT_INDEX_CNS].singleChannelDMAConfigs.SCDMAConfig1)                 , BC3_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(rxDMA[NON_FIRST_UNIT_INDEX_CNS].singleChannelDMAConfigs.SCDMACTPktIndentification)    , BC3_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(rxDMA[NON_FIRST_UNIT_INDEX_CNS].scdmaDebug.portBuffAllocCnt)                          , BC3_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(rxDMA[NON_FIRST_UNIT_INDEX_CNS].interrupts.rxDMASCDMAInterruptMask)                   , BC3_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(rxDMA[NON_FIRST_UNIT_INDEX_CNS].interrupts.rxDMASCDMAInterruptCause)                  , BC3_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(rxDMA[NON_FIRST_UNIT_INDEX_CNS].singleChannelDMAPip.SCDMA_n_PIP_Config)               , BC3_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(rxDMA[NON_FIRST_UNIT_INDEX_CNS].singleChannelDMAPip.SCDMA_n_Drop_Pkt_Counter)         , BC3_NUM_PORTS_DMA}
         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };

    return prvCpssDxChHwRegAddrIsAddrInRegDb1Array(devNum,0,perPortAddrArr,regAddr);
}

/*check if register address is per port in TxDMA unit */
static GT_BOOL  bobcat3_mustNotDuplicate_txdmaAddr(
    IN GT_U8        devNum,
    IN GT_U32       regAddr
)
{
    static PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC perPortAddrArr[]=
    {
         {REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txDMA[NON_FIRST_UNIT_INDEX_CNS].txDMAPerSCDMAConfigs.burstLimiterSCDMA               ), BC3_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txDMA[NON_FIRST_UNIT_INDEX_CNS].txDMAPerSCDMAConfigs.FIFOsThresholdsConfigsSCDMAReg1 ), BC3_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txDMA[NON_FIRST_UNIT_INDEX_CNS].txDMAPerSCDMAConfigs.txFIFOCntrsConfigsSCDMA         ), BC3_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txDMA[NON_FIRST_UNIT_INDEX_CNS].txDMAPerSCDMAConfigs.SCDMAConfigs                    ), BC3_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txDMA[NON_FIRST_UNIT_INDEX_CNS].txDMAPerSCDMAConfigs.SCDMAConfigs1                   ), BC3_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txDMA[NON_FIRST_UNIT_INDEX_CNS].txDMAPerSCDMAConfigs.SCDMAConfigs2                   ), BC3_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txDMA[NON_FIRST_UNIT_INDEX_CNS].txDMAPerSCDMAConfigs.SCDMAMetalFixReg                ), BC3_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txDMA[NON_FIRST_UNIT_INDEX_CNS].txDMAPerSCDMAConfigs.SCDMARateLimitResidueVectorThr  ), BC3_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txDMA[NON_FIRST_UNIT_INDEX_CNS].txDMADebug.informativeDebug.SCDMAStatusReg1          ), BC3_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txDMA[NON_FIRST_UNIT_INDEX_CNS].txDMADebug.informativeDebug.SCDMAStatusReg2          ), BC3_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txDMA[NON_FIRST_UNIT_INDEX_CNS].txDMADebug.informativeDebug.SCDMAPrefFIFOFillLevel   ), BC3_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txDMA[NON_FIRST_UNIT_INDEX_CNS].txDMADebug.informativeDebug.SCDMAPrefFIFOMaxFillLevel), BC3_NUM_PORTS_DMA}
         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };

    return prvCpssDxChHwRegAddrIsAddrInRegDb1Array(devNum,0,perPortAddrArr,regAddr);
}

/*check if register address is per port in TxFifo unit */
static GT_BOOL  bobcat3_mustNotDuplicate_txfifoAddr(
    IN GT_U8        devNum,
    IN GT_U32       regAddr
)
{
    static PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC perPortAddrArr[]=
    {
         {REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txFIFO[NON_FIRST_UNIT_INDEX_CNS].txFIFOShiftersConfig.SCDMAShiftersConf       ),  BC3_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txFIFO[NON_FIRST_UNIT_INDEX_CNS].txFIFOGlobalConfig.SCDMAPayloadThreshold     ),  BC3_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txFIFO[NON_FIRST_UNIT_INDEX_CNS].txFIFODebug.informativeDebug.SCDMAStatusReg1 ),  BC3_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txFIFO[NON_FIRST_UNIT_INDEX_CNS].txFIFODebug.informativeDebug.SCDMAStatusReg2 ),  BC3_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txFIFO[NON_FIRST_UNIT_INDEX_CNS].txFIFODebug.informativeDebug.SCDMAStatusReg3 ),  BC3_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txFIFO[NON_FIRST_UNIT_INDEX_CNS].txFIFODebug.informativeDebug.SCDMAStatusReg4 ),  BC3_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txFIFO[NON_FIRST_UNIT_INDEX_CNS].txFIFODebug.informativeDebug.SCDMAStatusReg5 ),  BC3_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txFIFO[NON_FIRST_UNIT_INDEX_CNS].txFIFODebug.informativeDebug.SCDMAStatusReg6 ),  BC3_NUM_PORTS_DMA}
         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };

    return prvCpssDxChHwRegAddrIsAddrInRegDb1Array(devNum,0,perPortAddrArr,regAddr);
}

/* number of DQ ports 'per DQ unit' */
#define BC3_NUM_PORTS_DQ_PART_1    72
#define BC3_NUM_PORTS_DQ_PART_2    (96-BC3_NUM_PORTS_DQ_PART_1)
#define BC3_NUM_PORTS_DQ_TOTAL      96

/*check if register address is per port in Txq DQ unit */
static GT_BOOL  bobcat3_mustNotDuplicate_txqDqAddr(
    IN GT_U8        devNum,
    IN GT_U32       regAddr
)
{
    static PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC perPortAddrArr[]=
    {
        /* handle consecutive addresses for lower 72 ports .
           this needed to allow better performance of prvCpssDxChHwRegAddrIsAddrInRegDb1Array(...)
           because it check for 'lower address' and 'highest address' of each range.
           and since the 96 ports are split 72 and 24 it exists in ranges that covers full DQ unit... so need to split
        */

        /* handle part 1 registers (from port 0..71)  */
         {REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(TXQ.dq[NON_FIRST_UNIT_INDEX_CNS].scheduler.portArbiterConfig.portsArbiterPortWorkConservingEnable ),  BC3_NUM_PORTS_DQ_PART_1}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(TXQ.dq[NON_FIRST_UNIT_INDEX_CNS].scheduler.portShaper.portRequestMaskSelector                     ),  BC3_NUM_PORTS_DQ_PART_1}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(TXQ.dq[NON_FIRST_UNIT_INDEX_CNS].scheduler.priorityArbiterWeights.portSchedulerProfile            ),  BC3_NUM_PORTS_DQ_PART_1}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(TXQ.dq[NON_FIRST_UNIT_INDEX_CNS].scheduler.schedulerConfig.conditionalMaskForPort                 ),  BC3_NUM_PORTS_DQ_PART_1}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(TXQ.dq[NON_FIRST_UNIT_INDEX_CNS].shaper.portDequeueEnable                                         ),  BC3_NUM_PORTS_DQ_PART_1}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(TXQ.dq[NON_FIRST_UNIT_INDEX_CNS].shaper.portTokenBucketMode                                       ),  BC3_NUM_PORTS_DQ_PART_1}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(TXQ.dq[NON_FIRST_UNIT_INDEX_CNS].global.globalDQConfig.portToDMAMapTable                          ),  BC3_NUM_PORTS_DQ_PART_1}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(TXQ.dq[NON_FIRST_UNIT_INDEX_CNS].global.globalDQConfig.portBCConstantValue                        ),  BC3_NUM_PORTS_DQ_PART_1}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(TXQ.dq[NON_FIRST_UNIT_INDEX_CNS].global.flushTrig.portTxQFlushTrigger                             ),  BC3_NUM_PORTS_DQ_PART_1}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(TXQ.dq[NON_FIRST_UNIT_INDEX_CNS].global.interrupt.flushDoneInterruptCause                         ),  4}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(TXQ.dq[NON_FIRST_UNIT_INDEX_CNS].global.interrupt.egrSTCInterruptCause                            ),  4}

        /* handle part 2 registers (from port 72..95) */
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(TXQ.dq[NON_FIRST_UNIT_INDEX_CNS].scheduler.portArbiterConfig.portsArbiterPortWorkConservingEnable[BC3_NUM_PORTS_DQ_PART_1] ),  BC3_NUM_PORTS_DQ_PART_2}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(TXQ.dq[NON_FIRST_UNIT_INDEX_CNS].scheduler.portShaper.portRequestMaskSelector                    [BC3_NUM_PORTS_DQ_PART_1] ),  BC3_NUM_PORTS_DQ_PART_2}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(TXQ.dq[NON_FIRST_UNIT_INDEX_CNS].scheduler.priorityArbiterWeights.portSchedulerProfile           [BC3_NUM_PORTS_DQ_PART_1] ),  BC3_NUM_PORTS_DQ_PART_2}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(TXQ.dq[NON_FIRST_UNIT_INDEX_CNS].scheduler.schedulerConfig.conditionalMaskForPort                [BC3_NUM_PORTS_DQ_PART_1] ),  BC3_NUM_PORTS_DQ_PART_2}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(TXQ.dq[NON_FIRST_UNIT_INDEX_CNS].shaper.portDequeueEnable                                        [BC3_NUM_PORTS_DQ_PART_1] ),  BC3_NUM_PORTS_DQ_PART_2}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(TXQ.dq[NON_FIRST_UNIT_INDEX_CNS].shaper.portTokenBucketMode                                      [BC3_NUM_PORTS_DQ_PART_1] ),  BC3_NUM_PORTS_DQ_PART_2}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(TXQ.dq[NON_FIRST_UNIT_INDEX_CNS].global.globalDQConfig.portToDMAMapTable                         [BC3_NUM_PORTS_DQ_PART_1] ),  BC3_NUM_PORTS_DQ_PART_2}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(TXQ.dq[NON_FIRST_UNIT_INDEX_CNS].global.globalDQConfig.portBCConstantValue                       [BC3_NUM_PORTS_DQ_PART_1] ),  BC3_NUM_PORTS_DQ_PART_2}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(TXQ.dq[NON_FIRST_UNIT_INDEX_CNS].global.flushTrig.portTxQFlushTrigger                            [BC3_NUM_PORTS_DQ_PART_1] ),  BC3_NUM_PORTS_DQ_PART_2}

         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };

    static PRV_CPSS_DXCH_HW_RED_ADDR_OFFSET_IN_TABLE_INFO_STC perPortTableArr[] =
    {         /* table type */                                                       /* number of entries */
        {CPSS_DXCH_LION_TABLE_TXQ_EGRESS_STC_E,                                  0/*take from tabled DB*/},
        {CPSS_DXCH_LION_TABLE_TXQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG_E,         0/*take from tabled DB*/},
        {CPSS_DXCH_LION_TABLE_TXQ_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG_E,0/*take from tabled DB*/},

        /* must be last */
        {CPSS_DXCH_TABLE_LAST_E , 0}
    };

    return prvCpssDxChHwRegAddrIsAddrInRegDb1Array(devNum,0,perPortAddrArr,regAddr) ||
           prvCpssDxChHwRegAddrIsAddrInTableArray(devNum,perPortTableArr,regAddr) ;
}

/* check if register address is per port in TTI[0] unit AND must not be duplicated to TTI[1] */
static GT_BOOL  bobcat3_mustNotDuplicate_ttiAddr(
    IN GT_U8        devNum,
    IN GT_U32       regAddr
)
{
    static PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC perPortAddrArr[]=
    {
        {REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(TTI.phyPortAndEPortAttributes.myPhysicalPortAttributes) , 256}
         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };

    return prvCpssDxChHwRegAddrIsAddrInRegDb1Array(devNum,0,perPortAddrArr,regAddr);
}


/* check if register address is per port in EQ[0] unit AND must not be duplicated to EQ[1] */
static GT_BOOL  bobcat3_mustNotDuplicate_eqAddr(
    IN GT_U8        devNum,
    IN GT_U32       regAddr
)
{
    static PRV_CPSS_DXCH_HW_RED_ADDR_OFFSET_IN_TABLE_INFO_STC perPortTableArr[] =
    {         /* table type */                                        /* number of entries */
        {CPSS_DXCH_SIP5_TABLE_EQ_INGRESS_STC_PHYSICAL_PORT_E,    0/*take from tabled DB*/},

        /* must be last */
        {CPSS_DXCH_TABLE_LAST_E , 0}
    };

    return prvCpssDxChHwRegAddrIsAddrInTableArray(devNum,perPortTableArr,regAddr) ;
}


static GT_BOOL  bobcat3_mustNotDuplicate_lpmMemAddr(
    IN GT_U8        devNum,
    IN GT_U32       regAddr
)
{
    static PRV_CPSS_DXCH_HW_RED_ADDR_OFFSET_IN_TABLE_INFO_STC tableArr[] =
    {         /* table type */                                        /* number of entries */
        {CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,    0/*take from tabled DB*/},

        /* must be last */
        {CPSS_DXCH_TABLE_LAST_E , 0}
    };

    return prvCpssDxChHwRegAddrIsAddrInTableArray(devNum,tableArr,regAddr) ;
}

static GT_BOOL  bobcat3_mustNotDuplicate_haAddr(
    IN GT_U8        devNum,
    IN GT_U32       regAddr
)
{
    static PRV_CPSS_DXCH_HW_RED_ADDR_OFFSET_IN_TABLE_INFO_STC tableArr[] =
    {         /* table type */                                        /* number of entries */
        {CPSS_DXCH2_TABLE_TUNNEL_START_CONFIG_E,    0/*take from tabled DB*/},

        /* must be last */
        {CPSS_DXCH_TABLE_LAST_E , 0}
    };

    return prvCpssDxChHwRegAddrIsAddrInTableArray(devNum,tableArr,regAddr) ;
}

/**
* @internal prvCpssBobkDuplicatedMultiPortGroupsGet_byDevNum function
* @endinternal
*
* @brief   Check if register address is duplicated in bobk device.
*
* @note   APPLICABLE DEVICES:      BobK.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lon2.
*
* @param[in] dev                      - the device number
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
* @param[in] dev                      Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] regAddr                  - register address to access.
*
* @param[out] portGroupsBmpPtr         - (pointer to) the updated port groups
*                                      relevant only when function returns GT_TRUE
* @param[out] isAdditionalRegDupNeededPtr - (pointer to)
*                                      GT_TRUE - the caller need to duplicate the value to additional
*                                      register in all 'port groups bmp'.
*                                      GT_FALSE - the caller NOT need to duplicate the value to additional
*                                      register.
* @param[out] additionalRegDupPtr      - (pointer to)
*                                      register address of additional register duplication.
*                                      Only relevant when isAdditionalRegDupNeeded is GT_TRUE.
* @param[out] maskDrvPortGroupsPtr     = (pointer to)
*                                      GT_TRUE  - mask the cpssDrv active ports only
*                                      GT_FALSE - mask the CPSS    active ports only
*
* @retval GT_TRUE                  - the caller need to use the updated 'port groups bmp'
* @retval GT_FALSE                 - the caller NOT need to use the updated 'port groups bmp'
*/
GT_BOOL prvCpssBobkDuplicatedMultiPortGroupsGet_byDevNum
(
    IN GT_SW_DEV_NUM            dev,
    IN GT_U32                   portGroupId,
    IN GT_U32                   regAddr,
    OUT  GT_PORT_GROUPS_BMP     *portGroupsBmpPtr,
    OUT  GT_BOOL                *isAdditionalRegDupNeededPtr,
    OUT  PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_INFO_STC *additionalRegDupPtr,
    OUT  GT_BOOL                *maskDrvPortGroupsPtr
)
{
    PRV_CPSS_DXCH_UNIT_ENT  unitId;
    GT_U32  additionalBaseAddr;
    GT_U8 devNum = (GT_U8)dev;

    unitId = prvCpssDxChHwRegAddrToUnitIdConvert(devNum, regAddr);
    switch(unitId)
    {
        case PRV_CPSS_DXCH_UNIT_RXDMA_E:
            additionalBaseAddr = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_RXDMA1_E,NULL);
            break;
        case PRV_CPSS_DXCH_UNIT_TXDMA_E:
            additionalBaseAddr = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_TXDMA1_E,NULL);
            break;
        case PRV_CPSS_DXCH_UNIT_TX_FIFO_E:
            additionalBaseAddr = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_TX_FIFO1_E,NULL);
            break;
        default:
            /* we not need to duplicate the address */
            return GT_FALSE;

    }

    /* the device not supports multi cores ... so state 'unaware' */
    *portGroupsBmpPtr = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    *isAdditionalRegDupNeededPtr = GT_TRUE;
    /* base address is 8 MSBits added with 24 LSBits of original address */
    additionalRegDupPtr->additionalAddressesArr[0] = additionalBaseAddr | (regAddr & 0x00FFFFFF);
    additionalRegDupPtr->numOfAdditionalAddresses = 1;

    (void)portGroupId;
    (void)(*maskDrvPortGroupsPtr);

    return GT_TRUE;
}

/**
* @internal prvCpssBobcat3DuplicatedMultiPortGroupsGet_byDevNum function
* @endinternal
*
* @brief   Check if register address is duplicated in bobcat3 device.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lon2.
*
* @param[in] dev                      - the device number
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
* @param[in] dev                      Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] regAddr                  - register address to access.
*
* @param[out] portGroupsBmpPtr         - (pointer to) the updated port groups
*                                      relevant only when function returns GT_TRUE
* @param[out] isAdditionalRegDupNeededPtr - (pointer to)
*                                      GT_TRUE - the caller need to duplicate the value to additional
*                                      register in all 'port groups bmp'.
*                                      GT_FALSE - the caller NOT need to duplicate the value to additional
*                                      register.
* @param[out] additionalRegDupPtr      - (pointer to)
*                                      register address of additional register duplication.
*                                      Only relevant when isAdditionalRegDupNeeded is GT_TRUE.
* @param[out] maskDrvPortGroupsPtr     = (pointer to)
*                                      GT_TRUE  - mask the cpssDrv active ports only
*                                      GT_FALSE - mask the CPSS    active ports only
*
* @retval GT_TRUE                  - the caller need to use the updated 'port groups bmp'
* @retval GT_FALSE                 - the caller NOT need to use the updated 'port groups bmp'
*/
GT_BOOL prvCpssBobcat3DuplicatedMultiPortGroupsGet_byDevNum
(
    IN GT_SW_DEV_NUM            dev,
    IN GT_U32                   portGroupId,
    IN GT_U32                   regAddr,
    OUT  GT_PORT_GROUPS_BMP     *portGroupsBmpPtr,
    OUT  GT_BOOL                *isAdditionalRegDupNeededPtr,
    OUT  PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_INFO_STC *additionalRegDupPtr,
    OUT  GT_BOOL                *maskDrvPortGroupsPtr
)
{
    PRV_CPSS_DXCH_UNIT_ENT  unitId;
    GT_U32  *additionalBaseAddrPtr;
    GT_U32  dupIndex;
    GT_U32  ii;
    GT_BOOL unitPerPipe;
    GT_U8 devNum = (GT_U8)dev;

    if(!PRV_CPSS_IS_MULTI_PIPES_DEVICE_MAC(devNum))
    {
        *portGroupsBmpPtr = BIT_0;
        /* the GM device not supports pipe 1 and also not support multiple DP units */
        /* so do not give any duplications */
        return GT_FALSE;
    }

    *portGroupsBmpPtr = BIT_0;/* initialization that indicated that unit is single instance
        needed by prv_cpss_multi_port_groups_bmp_check_specific_unit_func(...) */

    /* initial needed variables */
    dupIndex = 0;
    unitPerPipe = GT_FALSE;
    additionalBaseAddrPtr = &additionalRegDupPtr->additionalAddressesArr[0];

    unitId = prvCpssDxChHwRegAddrToUnitIdConvert(devNum, regAddr);
    switch(unitId)
    {
        case PRV_CPSS_DXCH_UNIT_RXDMA_E:
            if(GT_TRUE == bobcat3_mustNotDuplicate_rxdmaAddr(devNum, regAddr))
            {
                /* the address is forbidden for duplication */
                return GT_FALSE;
            }

            additionalBaseAddrPtr[dupIndex++] = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_RXDMA1_E,NULL);
            additionalBaseAddrPtr[dupIndex++] = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_RXDMA2_E,NULL);
            additionalBaseAddrPtr[dupIndex++] = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_RXDMA3_E,NULL);
            additionalBaseAddrPtr[dupIndex++] = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_RXDMA4_E,NULL);
            additionalBaseAddrPtr[dupIndex++] = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_RXDMA5_E,NULL);
            break;
    case PRV_CPSS_DXCH_UNIT_TXDMA_E:
            if(GT_TRUE == bobcat3_mustNotDuplicate_txdmaAddr(devNum, regAddr))
            {
                /* the address is forbidden for duplication */
                return GT_FALSE;
            }
            additionalBaseAddrPtr[dupIndex++] = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_TXDMA1_E,NULL);
            additionalBaseAddrPtr[dupIndex++] = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_TXDMA2_E,NULL);
            additionalBaseAddrPtr[dupIndex++] = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_TXDMA3_E,NULL);
            additionalBaseAddrPtr[dupIndex++] = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_TXDMA4_E,NULL);
            additionalBaseAddrPtr[dupIndex++] = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_TXDMA5_E,NULL);
            break;
        case PRV_CPSS_DXCH_UNIT_TX_FIFO_E:
            if(GT_TRUE == bobcat3_mustNotDuplicate_txfifoAddr(devNum, regAddr))
            {
                /* the address is forbidden for duplication */
                return GT_FALSE;
            }

            additionalBaseAddrPtr[dupIndex++] = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_TX_FIFO1_E,NULL);
            additionalBaseAddrPtr[dupIndex++] = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_TX_FIFO2_E,NULL);
            additionalBaseAddrPtr[dupIndex++] = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_TX_FIFO3_E,NULL);
            additionalBaseAddrPtr[dupIndex++] = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_TX_FIFO4_E,NULL);
            additionalBaseAddrPtr[dupIndex++] = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_TX_FIFO5_E,NULL);
            break;
        case PRV_CPSS_DXCH_UNIT_TXQ_DQ_E:
            if(GT_TRUE == bobcat3_mustNotDuplicate_txqDqAddr(devNum, regAddr))
            {
                /* the address is forbidden for duplication */
                return GT_FALSE;
            }

            additionalBaseAddrPtr[dupIndex++] = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_TXQ_DQ1_E,NULL);
            additionalBaseAddrPtr[dupIndex++] = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_TXQ_DQ2_E,NULL);
            additionalBaseAddrPtr[dupIndex++] = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_TXQ_DQ3_E,NULL);
            additionalBaseAddrPtr[dupIndex++] = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_TXQ_DQ4_E,NULL);
            additionalBaseAddrPtr[dupIndex++] = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_TXQ_DQ5_E,NULL);
            break;
        case PRV_CPSS_DXCH_UNIT_LPM_E:
             if(GT_TRUE == bobcat3_mustNotDuplicate_lpmMemAddr(devNum, regAddr))
            {
                /* the address is forbidden for duplication */
                return GT_FALSE;
            }

            for(ii = 1; ii < 4 ; ii++)
            {
                additionalBaseAddrPtr[dupIndex++] =
                    prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_LPM_E,NULL)+PRV_CPSS_DXCH_BOBCAT3_LPM_UNIT_OFFSET_GAP_MAC(ii);
            }

            break;
        case PRV_CPSS_DXCH_UNIT_LPM_1_E:
            /* this unit is not for duplication */
            return GT_FALSE;
        case PRV_CPSS_DXCH_UNIT_SERDES_E                      :
            /* probably ALL is per port ... and no global config */
            return GT_FALSE;
        case PRV_CPSS_DXCH_UNIT_MIB_E                         :
            /* probably ALL is per port ... and no global config */
            return GT_FALSE;
        case PRV_CPSS_DXCH_UNIT_GOP_E                         :
            /* probably ALL is per port ... and no global config */
            return GT_FALSE;
        case PRV_CPSS_DXCH_UNIT_TAI_E                         :
            CPSS_TBD_BOOKMARK_BOBCAT3
            return GT_FALSE;

        case PRV_CPSS_DXCH_UNIT_TTI_E                         :
            if(GT_TRUE == bobcat3_mustNotDuplicate_ttiAddr(devNum, regAddr))
            {
                /* the address is forbidden for duplication */
                return GT_FALSE;
            }
            additionalBaseAddrPtr[dupIndex++] = regAddr | PRV_CPSS_DXCH_BOBCAT3_PIPE1_OFFSET_CNS;

            unitPerPipe = GT_TRUE;
            break;

        case PRV_CPSS_DXCH_UNIT_EQ_E                         :
            if(GT_TRUE == bobcat3_mustNotDuplicate_eqAddr(devNum, regAddr))
            {
                /* the address is forbidden for duplication */
                return GT_FALSE;
            }
            additionalBaseAddrPtr[dupIndex++] = regAddr | PRV_CPSS_DXCH_BOBCAT3_PIPE1_OFFSET_CNS;

            unitPerPipe = GT_TRUE;
            break;

        case PRV_CPSS_DXCH_UNIT_HA_E                          :
            if(GT_TRUE == bobcat3_mustNotDuplicate_haAddr(devNum, regAddr))
            {
                /* the address is forbidden for duplication */
                return GT_FALSE;
            }
            additionalBaseAddrPtr[dupIndex++] = regAddr | PRV_CPSS_DXCH_BOBCAT3_PIPE1_OFFSET_CNS;

            unitPerPipe = GT_TRUE;
            break;

        case PRV_CPSS_DXCH_UNIT_EGF_SHT_E                     :
        case PRV_CPSS_DXCH_UNIT_PCL_E                         :
        case PRV_CPSS_DXCH_UNIT_L2I_E                         :
        case PRV_CPSS_DXCH_UNIT_IPVX_E                        :
        case PRV_CPSS_DXCH_UNIT_IPLR_E                        :
        case PRV_CPSS_DXCH_UNIT_IPLR_1_E                      :
        case PRV_CPSS_DXCH_UNIT_IOAM_E                        :
        case PRV_CPSS_DXCH_UNIT_MLL_E                         :
        case PRV_CPSS_DXCH_UNIT_EGF_EFT_E                     :
        case PRV_CPSS_DXCH_UNIT_CNC_0_E                       :
        case PRV_CPSS_DXCH_UNIT_CNC_1_E                       :
        case PRV_CPSS_DXCH_UNIT_ERMRK_E                       :
        case PRV_CPSS_DXCH_UNIT_EREP_E                        :
        case PRV_CPSS_DXCH_UNIT_PREQ_E                        :
        case PRV_CPSS_DXCH_UNIT_EPCL_E                        :
        case PRV_CPSS_DXCH_UNIT_EPLR_E                        :
        case PRV_CPSS_DXCH_UNIT_EOAM_E                        :
        case PRV_CPSS_DXCH_UNIT_RXDMA_GLUE_E                  :
        case PRV_CPSS_DXCH_UNIT_TXDMA_GLUE_E                  :
        case PRV_CPSS_DXCH_UNIT_IA_E                          :
            /* ALL those are units that ALL it's config should be also on pipe 1 */
            /* NOTE: if one of those units hold some special registers ....
                     take it out of this generic case */
            additionalBaseAddrPtr[dupIndex++] = regAddr | PRV_CPSS_DXCH_BOBCAT3_PIPE1_OFFSET_CNS;

            unitPerPipe = GT_TRUE;
            break;

        default:
            /* we get here for 'non first instance' of duplicated units
               and for units that are 'shared' between pipes (single instance) */

            /* we not need to duplicate the address */
            return GT_FALSE;

    }

    if(unitPerPipe == GT_TRUE &&
       dupIndex == 1/* just make sure of additional single pipe */)
    {
        /* support multi-pipe awareness by parameter <portGroupId> */
        switch(portGroupId)
        {
            case CPSS_PORT_GROUP_UNAWARE_MODE_CNS:
                /* allow the loops on the 2 addresses (orig+additional) to be
                    accessed , each one in different iteration in the loop of :
                    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(...)
                    or
                    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(...)
                */

                additionalRegDupPtr->use_originalAddressPortGroup = GT_TRUE;
                additionalRegDupPtr->originalAddressPortGroup = 0; /* pipe 0 */

                additionalRegDupPtr->use_portGroupsArr = GT_TRUE;
                additionalRegDupPtr->portGroupsArr[0] = 1;/* pipe 1 */

                *portGroupsBmpPtr = 3;/* BIT_0 | BIT_1 */
                break;
            case 0:
                *portGroupsBmpPtr = BIT_0;
                dupIndex = 0;/* access only to pipe 0 --> no duplications */
                break;
            case 1:
                *portGroupsBmpPtr = BIT_1;
                /* access only to pipe 1 */
                additionalRegDupPtr->originalAddressIsNotValid = GT_TRUE;
                /* the only valid address is the one in additionalBaseAddrPtr[0] */
                break;
            default:
                /* should not get here */
                return GT_FALSE;
        }
    }
    else
    {
        /* the other units not care about multi-pipe */
    }

    if(dupIndex == 0)
    {
        return GT_FALSE;
    }

    additionalRegDupPtr->numOfAdditionalAddresses = dupIndex;

    *isAdditionalRegDupNeededPtr = GT_TRUE;

    for(ii = 0; ii < dupIndex ; ii++)
    {
        /* base address is 8 MSBits added with 24 LSBits of original address */
        additionalBaseAddrPtr[ii] |= (regAddr & 0x00FFFFFF);
    }

    *maskDrvPortGroupsPtr = GT_FALSE;

    return GT_TRUE;
}

#define ALDRIN2_NUM_PORTS_DMA                  25
/* unit 2,3 hold ports 0..24 (unit 1 - hold 0..11,24)*/
#define ALDRIN2_NON_FIRST_UNIT_INDEX_CNS       2

/*check if register address is per port in RxDMA unit */
static GT_BOOL  aldrin2_mustNotDuplicate_rxdmaAddr(
    IN GT_U8        devNum,
    IN GT_U32       regAddr
)
{
    static PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC perPortAddrArr[]=
    {
         {REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(rxDMA[ALDRIN2_NON_FIRST_UNIT_INDEX_CNS].singleChannelDMAConfigs.SCDMAConfig0)                 , ALDRIN2_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(rxDMA[ALDRIN2_NON_FIRST_UNIT_INDEX_CNS].singleChannelDMAConfigs.SCDMAConfig1)                 , ALDRIN2_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(rxDMA[ALDRIN2_NON_FIRST_UNIT_INDEX_CNS].singleChannelDMAConfigs.SCDMACTPktIndentification)    , ALDRIN2_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(rxDMA[ALDRIN2_NON_FIRST_UNIT_INDEX_CNS].scdmaDebug.portBuffAllocCnt)                          , ALDRIN2_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(rxDMA[ALDRIN2_NON_FIRST_UNIT_INDEX_CNS].interrupts.rxDMASCDMAInterruptMask)                   , ALDRIN2_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(rxDMA[ALDRIN2_NON_FIRST_UNIT_INDEX_CNS].interrupts.rxDMASCDMAInterruptCause)                  , ALDRIN2_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(rxDMA[ALDRIN2_NON_FIRST_UNIT_INDEX_CNS].singleChannelDMAPip.SCDMA_n_PIP_Config)               , ALDRIN2_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(rxDMA[ALDRIN2_NON_FIRST_UNIT_INDEX_CNS].singleChannelDMAPip.SCDMA_n_Drop_Pkt_Counter)         , ALDRIN2_NUM_PORTS_DMA}
         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };

    return prvCpssDxChHwRegAddrIsAddrInRegDb1Array(devNum,0,perPortAddrArr,regAddr);
}


/*check if register address is per port in TxDMA unit */
static GT_BOOL  aldrin2_mustNotDuplicate_txdmaAddr(
    IN GT_U8        devNum,
    IN GT_U32       regAddr
)
{
    static PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC perPortAddrArr[]=
    {
         {REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txDMA[ALDRIN2_NON_FIRST_UNIT_INDEX_CNS].txDMAPerSCDMAConfigs.burstLimiterSCDMA               ), ALDRIN2_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txDMA[ALDRIN2_NON_FIRST_UNIT_INDEX_CNS].txDMAPerSCDMAConfigs.FIFOsThresholdsConfigsSCDMAReg1 ), ALDRIN2_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txDMA[ALDRIN2_NON_FIRST_UNIT_INDEX_CNS].txDMAPerSCDMAConfigs.txFIFOCntrsConfigsSCDMA         ), ALDRIN2_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txDMA[ALDRIN2_NON_FIRST_UNIT_INDEX_CNS].txDMAPerSCDMAConfigs.SCDMAConfigs                    ), ALDRIN2_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txDMA[ALDRIN2_NON_FIRST_UNIT_INDEX_CNS].txDMAPerSCDMAConfigs.SCDMAConfigs1                   ), ALDRIN2_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txDMA[ALDRIN2_NON_FIRST_UNIT_INDEX_CNS].txDMAPerSCDMAConfigs.SCDMAConfigs2                   ), ALDRIN2_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txDMA[ALDRIN2_NON_FIRST_UNIT_INDEX_CNS].txDMAPerSCDMAConfigs.SCDMAMetalFixReg                ), ALDRIN2_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txDMA[ALDRIN2_NON_FIRST_UNIT_INDEX_CNS].txDMAPerSCDMAConfigs.SCDMARateLimitResidueVectorThr  ), ALDRIN2_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txDMA[ALDRIN2_NON_FIRST_UNIT_INDEX_CNS].txDMADebug.informativeDebug.SCDMAStatusReg1          ), ALDRIN2_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txDMA[ALDRIN2_NON_FIRST_UNIT_INDEX_CNS].txDMADebug.informativeDebug.SCDMAStatusReg2          ), ALDRIN2_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txDMA[ALDRIN2_NON_FIRST_UNIT_INDEX_CNS].txDMADebug.informativeDebug.SCDMAPrefFIFOFillLevel   ), ALDRIN2_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txDMA[ALDRIN2_NON_FIRST_UNIT_INDEX_CNS].txDMADebug.informativeDebug.SCDMAPrefFIFOMaxFillLevel), ALDRIN2_NUM_PORTS_DMA}
         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };

    return prvCpssDxChHwRegAddrIsAddrInRegDb1Array(devNum,0,perPortAddrArr,regAddr);
}

/*check if register address is per port in TxFifo unit */
static GT_BOOL  aldrin2_mustNotDuplicate_txfifoAddr(
    IN GT_U8        devNum,
    IN GT_U32       regAddr
)
{
    static PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC perPortAddrArr[]=
    {
         {REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txFIFO[ALDRIN2_NON_FIRST_UNIT_INDEX_CNS].txFIFOShiftersConfig.SCDMAShiftersConf       ),  ALDRIN2_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txFIFO[ALDRIN2_NON_FIRST_UNIT_INDEX_CNS].txFIFOGlobalConfig.SCDMAPayloadThreshold     ),  ALDRIN2_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txFIFO[ALDRIN2_NON_FIRST_UNIT_INDEX_CNS].txFIFODebug.informativeDebug.SCDMAStatusReg1 ),  ALDRIN2_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txFIFO[ALDRIN2_NON_FIRST_UNIT_INDEX_CNS].txFIFODebug.informativeDebug.SCDMAStatusReg2 ),  ALDRIN2_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txFIFO[ALDRIN2_NON_FIRST_UNIT_INDEX_CNS].txFIFODebug.informativeDebug.SCDMAStatusReg3 ),  ALDRIN2_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txFIFO[ALDRIN2_NON_FIRST_UNIT_INDEX_CNS].txFIFODebug.informativeDebug.SCDMAStatusReg4 ),  ALDRIN2_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txFIFO[ALDRIN2_NON_FIRST_UNIT_INDEX_CNS].txFIFODebug.informativeDebug.SCDMAStatusReg5 ),  ALDRIN2_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txFIFO[ALDRIN2_NON_FIRST_UNIT_INDEX_CNS].txFIFODebug.informativeDebug.SCDMAStatusReg6 ),  ALDRIN2_NUM_PORTS_DMA}
         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };

    return prvCpssDxChHwRegAddrIsAddrInRegDb1Array(devNum,0,perPortAddrArr,regAddr);
}

/* number of DQ ports 'per DQ unit' */
#define ALDRIN2_NUM_PORTS_DQ    25

/*check if register address is per port in Txq DQ unit */
static GT_BOOL  aldrin2_mustNotDuplicate_txqDqAddr(
    IN GT_U8        devNum,
    IN GT_U32       regAddr
)
{
    static PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC perPortAddrArr[]=
    {
        /* handle part 1 registers (from port 0..24)  */
         {REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(TXQ.dq[NON_FIRST_UNIT_INDEX_CNS].scheduler.portArbiterConfig.portsArbiterPortWorkConservingEnable ),  ALDRIN2_NUM_PORTS_DQ}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(TXQ.dq[NON_FIRST_UNIT_INDEX_CNS].scheduler.portShaper.portRequestMaskSelector                     ),  ALDRIN2_NUM_PORTS_DQ}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(TXQ.dq[NON_FIRST_UNIT_INDEX_CNS].scheduler.priorityArbiterWeights.portSchedulerProfile            ),  ALDRIN2_NUM_PORTS_DQ}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(TXQ.dq[NON_FIRST_UNIT_INDEX_CNS].scheduler.schedulerConfig.conditionalMaskForPort                 ),  ALDRIN2_NUM_PORTS_DQ}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(TXQ.dq[NON_FIRST_UNIT_INDEX_CNS].shaper.portDequeueEnable                                         ),  ALDRIN2_NUM_PORTS_DQ}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(TXQ.dq[NON_FIRST_UNIT_INDEX_CNS].shaper.portTokenBucketMode                                       ),  ALDRIN2_NUM_PORTS_DQ}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(TXQ.dq[NON_FIRST_UNIT_INDEX_CNS].global.globalDQConfig.portToDMAMapTable                          ),  ALDRIN2_NUM_PORTS_DQ}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(TXQ.dq[NON_FIRST_UNIT_INDEX_CNS].global.globalDQConfig.portBCConstantValue                        ),  ALDRIN2_NUM_PORTS_DQ}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(TXQ.dq[NON_FIRST_UNIT_INDEX_CNS].global.flushTrig.portTxQFlushTrigger                             ),  ALDRIN2_NUM_PORTS_DQ}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(TXQ.dq[NON_FIRST_UNIT_INDEX_CNS].global.interrupt.flushDoneInterruptCause                         ),  1}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(TXQ.dq[NON_FIRST_UNIT_INDEX_CNS].global.interrupt.egrSTCInterruptCause                            ),  1}

         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };

    static PRV_CPSS_DXCH_HW_RED_ADDR_OFFSET_IN_TABLE_INFO_STC perPortTableArr[] =
    {         /* table type */                                                       /* number of entries */
        {CPSS_DXCH_LION_TABLE_TXQ_EGRESS_STC_E,                                  0/*take from tabled DB*/},
        {CPSS_DXCH_LION_TABLE_TXQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG_E,         0/*take from tabled DB*/},
        {CPSS_DXCH_LION_TABLE_TXQ_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG_E,0/*take from tabled DB*/},

        /* must be last */
        {CPSS_DXCH_TABLE_LAST_E , 0}
    };

    return prvCpssDxChHwRegAddrIsAddrInRegDb1Array(devNum,0,perPortAddrArr,regAddr) ||
           prvCpssDxChHwRegAddrIsAddrInTableArray(devNum,perPortTableArr,regAddr) ;
}

/**
* @internal prvCpssAldrin2DuplicatedMultiPortGroupsGet_byDevNum function
* @endinternal
*
* @brief   Check if register address is duplicated in Aldrin2 device.
*
* @note   APPLICABLE DEVICES:      Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lon2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in] dev                      - the device number
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
* @param[in] dev                      Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] regAddr                  - register address to access.
*
* @param[out] portGroupsBmpPtr         - (pointer to) the updated port groups
*                                      relevant only when function returns GT_TRUE
* @param[out] isAdditionalRegDupNeededPtr - (pointer to)
*                                      GT_TRUE - the caller need to duplicate the value to additional
*                                      register in all 'port groups bmp'.
*                                      GT_FALSE - the caller NOT need to duplicate the value to additional
*                                      register.
* @param[out] additionalRegDupPtr      - (pointer to)
*                                      register address of additional register duplication.
*                                      Only relevant when isAdditionalRegDupNeeded is GT_TRUE.
* @param[out] maskDrvPortGroupsPtr     = (pointer to)
*                                      GT_TRUE  - mask the cpssDrv active ports only
*                                      GT_FALSE - mask the CPSS    active ports only
*
* @retval GT_TRUE                  - the caller need to use the updated 'port groups bmp'
* @retval GT_FALSE                 - the caller NOT need to use the updated 'port groups bmp'
*/
GT_BOOL prvCpssAldrin2DuplicatedMultiPortGroupsGet_byDevNum
(
    IN GT_SW_DEV_NUM            dev,
    IN GT_U32                   portGroupId,
    IN GT_U32                   regAddr,
    OUT  GT_PORT_GROUPS_BMP     *portGroupsBmpPtr,
    OUT  GT_BOOL                *isAdditionalRegDupNeededPtr,
    OUT  PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_INFO_STC *additionalRegDupPtr,
    OUT  GT_BOOL                *maskDrvPortGroupsPtr
)
{
    PRV_CPSS_DXCH_UNIT_ENT  unitId;
    GT_U32  *additionalBaseAddrPtr;
    GT_U32  dupIndex;
    GT_U32  ii;
    GT_U8 devNum = (GT_U8)dev;

    (void)portGroupId;

    *portGroupsBmpPtr = BIT_0;/* initialization that indicated that unit is single instance
        needed by prv_cpss_multi_port_groups_bmp_check_specific_unit_func(...) */

    /* initial needed variables */
    dupIndex = 0;
    additionalBaseAddrPtr = &additionalRegDupPtr->additionalAddressesArr[0];

    unitId = prvCpssDxChHwRegAddrToUnitIdConvert(devNum, regAddr);
    switch(unitId)
    {
        case PRV_CPSS_DXCH_UNIT_RXDMA_E:
            if(GT_TRUE == aldrin2_mustNotDuplicate_rxdmaAddr(devNum, regAddr))
            {
                /* the address is forbidden for duplication */
                return GT_FALSE;
            }

            additionalBaseAddrPtr[dupIndex++] = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_RXDMA1_E,NULL);
            additionalBaseAddrPtr[dupIndex++] = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_RXDMA2_E,NULL);
            additionalBaseAddrPtr[dupIndex++] = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_RXDMA3_E,NULL);
            break;
        case PRV_CPSS_DXCH_UNIT_TXDMA_E:
            if(GT_TRUE == aldrin2_mustNotDuplicate_txdmaAddr(devNum, regAddr))
            {
                /* the address is forbidden for duplication */
                return GT_FALSE;
            }

            additionalBaseAddrPtr[dupIndex++] = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_TXDMA1_E,NULL);
            additionalBaseAddrPtr[dupIndex++] = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_TXDMA2_E,NULL);
            additionalBaseAddrPtr[dupIndex++] = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_TXDMA3_E,NULL);
            break;
        case PRV_CPSS_DXCH_UNIT_TX_FIFO_E:
            if(GT_TRUE == aldrin2_mustNotDuplicate_txfifoAddr(devNum, regAddr))
            {
                /* the address is forbidden for duplication */
                return GT_FALSE;
            }

            additionalBaseAddrPtr[dupIndex++] = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_TX_FIFO1_E,NULL);
            additionalBaseAddrPtr[dupIndex++] = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_TX_FIFO2_E,NULL);
            additionalBaseAddrPtr[dupIndex++] = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_TX_FIFO3_E,NULL);
            break;
        case PRV_CPSS_DXCH_UNIT_TXQ_DQ_E:
            if(GT_TRUE == aldrin2_mustNotDuplicate_txqDqAddr(devNum, regAddr))
            {
                /* the address is forbidden for duplication */
                return GT_FALSE;
            }

            additionalBaseAddrPtr[dupIndex++] = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_TXQ_DQ1_E,NULL);
            additionalBaseAddrPtr[dupIndex++] = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_TXQ_DQ2_E,NULL);
            additionalBaseAddrPtr[dupIndex++] = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_TXQ_DQ3_E,NULL);
            break;
        case PRV_CPSS_DXCH_UNIT_LPM_E:
            /* use bobcat3 function ... as no relevant changes on this issue */
            if(GT_TRUE == bobcat3_mustNotDuplicate_lpmMemAddr(devNum, regAddr))
            {
                /* the address is forbidden for duplication */
                return GT_FALSE;
            }

            for(ii = 1; ii < 2 ; ii++)
            {
                additionalBaseAddrPtr[dupIndex++] =
                    prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_LPM_E,NULL)+PRV_CPSS_DXCH_BOBCAT3_LPM_UNIT_OFFSET_GAP_MAC(ii);
            }

            break;
        default:
            /* we get here for 'non first instance' of duplicated units
               and for units that hold single instance */

            /* we not need to duplicate the address */
            return GT_FALSE;

    }

    additionalRegDupPtr->numOfAdditionalAddresses = dupIndex;

    *isAdditionalRegDupNeededPtr = GT_TRUE;

    for(ii = 0; ii < dupIndex ; ii++)
    {
        /* base address is 8 MSBits added with 24 LSBits of original address */
        additionalBaseAddrPtr[ii] |= (regAddr & 0x00FFFFFF);
    }

    *maskDrvPortGroupsPtr = GT_FALSE;

    return GT_TRUE;
}

/* number of rx/tx DMA and txfifo 'per DP unit' */
#define ALDRIN_NUM_PORTS_DMA   12

/*check if register address is per port in RxDMA unit */
static GT_BOOL  aldrin_mustNotDuplicate_rxdmaAddr(
    IN GT_U8        devNum,
    IN GT_U32       regAddr
)
{
    static PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC perPortAddrArr[]=
    {
         {REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(rxDMA[0].singleChannelDMAConfigs.SCDMAConfig0)                 , ALDRIN_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(rxDMA[0].singleChannelDMAConfigs.SCDMAConfig1)                 , ALDRIN_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(rxDMA[0].scdmaDebug.portBuffAllocCnt)                          , ALDRIN_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(rxDMA[0].interrupts.rxDMASCDMAInterruptMask)                   , ALDRIN_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(rxDMA[0].interrupts.rxDMASCDMAInterruptCause)                  , ALDRIN_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(rxDMA[0].singleChannelDMAPip.SCDMA_n_PIP_Config)               , ALDRIN_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(rxDMA[0].singleChannelDMAPip.SCDMA_n_Drop_Pkt_Counter)         , ALDRIN_NUM_PORTS_DMA}
         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };

    return prvCpssDxChHwRegAddrIsAddrInRegDb1Array(devNum,0,perPortAddrArr,regAddr);
}

/*check if register address is per port in TxDMA unit */
static GT_BOOL  aldrin_mustNotDuplicate_txdmaAddr(
    IN GT_U8        devNum,
    IN GT_U32       regAddr
)
{
    static PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC perPortAddrArr[]=
    {
         {REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txDMA[0].txDMAPerSCDMAConfigs.burstLimiterSCDMA               ), ALDRIN_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txDMA[0].txDMAPerSCDMAConfigs.FIFOsThresholdsConfigsSCDMAReg1 ), ALDRIN_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txDMA[0].txDMAPerSCDMAConfigs.txFIFOCntrsConfigsSCDMA         ), ALDRIN_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txDMA[0].txDMAPerSCDMAConfigs.SCDMAConfigs                    ), ALDRIN_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txDMA[0].txDMAPerSCDMAConfigs.SCDMAMetalFixReg                ), ALDRIN_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txDMA[0].txDMADebug.informativeDebug.SCDMAStatusReg1          ), ALDRIN_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txDMA[0].txDMADebug.informativeDebug.SCDMAStatusReg2          ), ALDRIN_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txDMA[0].txDMADebug.informativeDebug.SCDMAPrefFIFOFillLevel   ), ALDRIN_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txDMA[0].txDMADebug.informativeDebug.SCDMAPrefFIFOMaxFillLevel), ALDRIN_NUM_PORTS_DMA}
         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };

    return prvCpssDxChHwRegAddrIsAddrInRegDb1Array(devNum,0,perPortAddrArr,regAddr);
}

/*check if register address is per port in TxFifo unit */
static GT_BOOL  aldrin_mustNotDuplicate_txfifoAddr(
    IN GT_U8        devNum,
    IN GT_U32       regAddr
)
{
    static PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC perPortAddrArr[]=
    {
         {REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txFIFO[0].txFIFOShiftersConfig.SCDMAShiftersConf       ),  ALDRIN_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txFIFO[0].txFIFOGlobalConfig.SCDMAPayloadThreshold     ),  ALDRIN_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txFIFO[0].txFIFODebug.informativeDebug.SCDMAStatusReg1 ),  ALDRIN_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txFIFO[0].txFIFODebug.informativeDebug.SCDMAStatusReg2 ),  ALDRIN_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txFIFO[0].txFIFODebug.informativeDebug.SCDMAStatusReg3 ),  ALDRIN_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txFIFO[0].txFIFODebug.informativeDebug.SCDMAStatusReg4 ),  ALDRIN_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txFIFO[0].txFIFODebug.informativeDebug.SCDMAStatusReg5 ),  ALDRIN_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(txFIFO[0].txFIFODebug.informativeDebug.SCDMAStatusReg6 ),  ALDRIN_NUM_PORTS_DMA}
         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };

    return prvCpssDxChHwRegAddrIsAddrInRegDb1Array(devNum,0,perPortAddrArr,regAddr);
}

/**
* @internal prvCpssAldrinDuplicatedMultiPortGroupsGet_byDevNum function
* @endinternal
*
* @brief   Check if register address is duplicated in aldrin device.
*
* @note   APPLICABLE DEVICES:      Aldrin.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; ExMxPm.
*
* @param[in] dev                      - the device number
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
* @param[in] dev                      Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] regAddr                  - register address to access.
*
* @param[out] portGroupsBmpPtr         - (pointer to) the updated port groups
*                                      relevant only when function returns GT_TRUE
* @param[out] isAdditionalRegDupNeededPtr - (pointer to)
*                                      GT_TRUE - the caller need to duplicate the value to additional
*                                      register in all 'port groups bmp'.
*                                      GT_FALSE - the caller NOT need to duplicate the value to additional
*                                      register.
* @param[out] additionalRegDupPtr      - (pointer to)
*                                      register address of additional register duplication.
*                                      Only relevant when isAdditionalRegDupNeeded is GT_TRUE.
* @param[out] maskDrvPortGroupsPtr     = (pointer to)
*                                      GT_TRUE  - mask the cpssDrv active ports only
*                                      GT_FALSE - mask the CPSS    active ports only
*
* @retval GT_TRUE                  - the caller need to use the updated 'port groups bmp'
* @retval GT_FALSE                 - the caller NOT need to use the updated 'port groups bmp'
*/
GT_BOOL prvCpssAldrinDuplicatedMultiPortGroupsGet_byDevNum
(
    IN GT_SW_DEV_NUM            dev,
    IN GT_U32                   portGroupId,
    IN GT_U32                   regAddr,
    OUT  GT_PORT_GROUPS_BMP     *portGroupsBmpPtr,
    OUT  GT_BOOL                *isAdditionalRegDupNeededPtr,
    OUT  PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_INFO_STC *additionalRegDupPtr,
    OUT  GT_BOOL                *maskDrvPortGroupsPtr
)
{
    PRV_CPSS_DXCH_UNIT_ENT  unitId;
    GT_U32  *additionalBaseAddrPtr = &additionalRegDupPtr->additionalAddressesArr[0];
    GT_U32  dupIndex = 0;
    GT_U32  ii;
    GT_U8 devNum = (GT_U8)dev;

    (void)portGroupId;

    unitId = prvCpssDxChHwRegAddrToUnitIdConvert(devNum, regAddr);
    switch(unitId)
    {
        case PRV_CPSS_DXCH_UNIT_RXDMA_E:
            if(GT_TRUE == aldrin_mustNotDuplicate_rxdmaAddr(devNum, regAddr))
            {
                /* the address is forbidden for duplication */
                return GT_FALSE;
            }

            additionalBaseAddrPtr[dupIndex++] = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_RXDMA1_E,NULL);
            additionalBaseAddrPtr[dupIndex++] = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_RXDMA2_E,NULL);
            break;
        case PRV_CPSS_DXCH_UNIT_TXDMA_E:
            if(GT_TRUE == aldrin_mustNotDuplicate_txdmaAddr(devNum, regAddr))
            {
                /* the address is forbidden for duplication */
                return GT_FALSE;
            }

            additionalBaseAddrPtr[dupIndex++] = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_TXDMA1_E,NULL);
            additionalBaseAddrPtr[dupIndex++] = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_TXDMA2_E,NULL);
            break;
        case PRV_CPSS_DXCH_UNIT_TX_FIFO_E:
            if(GT_TRUE == aldrin_mustNotDuplicate_txfifoAddr(devNum, regAddr))
            {
                /* the address is forbidden for duplication */
                return GT_FALSE;
            }

            additionalBaseAddrPtr[dupIndex++] = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_TX_FIFO1_E,NULL);
            additionalBaseAddrPtr[dupIndex++] = prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_TX_FIFO2_E,NULL);
            break;
        default:
            /* we not need to duplicate the address */
            return GT_FALSE;

    }

    additionalRegDupPtr->numOfAdditionalAddresses = dupIndex;

    /* the device not supports multi cores ... so state 'unaware' */
    *portGroupsBmpPtr = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    *isAdditionalRegDupNeededPtr = GT_TRUE;

    for(ii = 0; ii < dupIndex ; ii++)
    {
        /* base address is 8 MSBits added with 24 LSBits of original address */
        additionalBaseAddrPtr[ii] |= (regAddr & 0x00FFFFFF);
    }

    (void)(*maskDrvPortGroupsPtr);
    return GT_TRUE;
}

/**
* @internal lmuInit function
* @endinternal
*
* @brief   sip6 : init the DB - LM unit
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                         - device number
* @param[in] globalLmuNum                   - LMU number
*
*/
static void lmuInit
(
    IN  GT_U8   devNum,
    IN  GT_U32  globalLmuNum
)
{
    GT_U32 ii;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    regsAddrPtr->LMU[globalLmuNum].averageCoefficient = 0x00008020;

    for(ii = 0; ii < 6; ii++)
    {
        regsAddrPtr->LMU[globalLmuNum].profileStatisticsReadData[ii] =
            0x00008030 + 0x4 * ii;
    }

    for(ii = 0; ii < 17; ii++)
    {
        regsAddrPtr->LMU[globalLmuNum].latencyOverThreshold_cause[ii] = 0x00008100;
        regsAddrPtr->LMU[globalLmuNum].latencyOverThreshold_mask[ii] = 0x00008180;
    }
    regsAddrPtr->LMU[globalLmuNum].summary_cause = 0x00008230;
    regsAddrPtr->LMU[globalLmuNum].summary_mask = 0x00008234;
    regsAddrPtr->LMU[globalLmuNum].channelEnable = 0x00008000;
}

/**
* @internal prvCpssDxChHwRegAddrVer1Init function
* @endinternal
*
* @brief   This function initializes the registers struct for eArch devices.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_CPU_MEM        - on malloc failed
* @retval GT_BAD_PARAM             - wrong device type to operate
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChHwRegAddrVer1Init
(
    IN  GT_U8 devNum
)
{
    GT_U32 * unitBasePtr;
    PRV_CPSS_DXCH_UNIT_ENT * unitIdPtr;
    GT_U32  ii,jj;
    GT_U32  subunitBase;
    GT_U32  indexArr[2];
    GT_U32  unit8MSBits;
    GT_U32  maxPtpPorts = 73;/*72 ports + last one for interlaken !!! */
    GT_U32  maxFcaPorts = 72;/*72 ports */
    GT_U32  maxCgPorts = 0;
    GT_U32  maxMtiPorts = 0;
    GT_BOOL supportSingleTaiUnit;
    GT_BOOL supportILKN;
    GT_BOOL supportTM = GT_TRUE;
    GT_BOOL supportLMS;
    GT_STATUS rc;
    GT_U32 totalTaiGop;
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    supportILKN = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_ilkn.supported;
    supportLMS = (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_lms.notSupported == GT_TRUE) ?
                GT_FALSE :/* instead of LMS we have : SMI and LED units */
                GT_TRUE;
    supportSingleTaiUnit = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_tai.supportSingleInstance;

    rc = prvCpssDxChUnitBaseTableGet(devNum,/*OUT*/&unitBasePtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
    {
        unitIdPtr = prvDxChAldrin2UnitIdPer8MSBitsBaseAddrArr;
        maxFcaPorts = maxPtpPorts = 73;
        supportTM = GT_FALSE;

        /* CG unit */
        maxCgPorts = 6 * 4;/* 6 ports , but current iterator skip those that
                              not divide by 4 */
    }
    else
    /* In BC3 we fill the first 36 ports unit, then we copy paste them to the rest
       36 ports with the proper pipe offset */
    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
    {
        /* unitBasePtr = prvDxChBobcat3UnitsBaseAddr;  */
        unitIdPtr = prvDxChBobcat3UnitIdPer8MSBitsBaseAddrArr;

        maxFcaPorts = maxPtpPorts = 36;/* currently only 36 ports from each pipe , no interlaken */
        supportTM = GT_FALSE;

        /* CG unit */
        maxCgPorts = 36;
    }
    else
    /* In Falcon we fill the first 36 ports unit, then we copy paste them to the rest
       36 ports with the proper pipe offset */
    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
    {
        /*
            NOTE: the Falcon not support 'fixed prefix' of 8 bits to the units.
            it supports 'flex ranges' each with different size , that not allow to use such array.
        */
        unitIdPtr = NULL;

        maxFcaPorts = maxPtpPorts = PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPortsPerPipe;/* currently only 32 ports from each pipe , no interlaken */
        supportTM = GT_FALSE;

        /* CG unit */
        maxCgPorts = maxFcaPorts = 0;
        /* Mti unit */
        maxMtiPorts = PRV_CPSS_MAX_MAC_PORTS_NUM_CNS - 2; /* CPU ports should be defined separately in different structure */
    }
    else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E)
    {
        unitIdPtr = prvDxChBobcat2UnitIdPer8MSBitsBaseAddrArr;
    }
    else if ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E) ||
            (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E))
    {
        unitIdPtr = prvDxChAldrinUnitIdPer8MSBitsBaseAddrArr;
        maxFcaPorts = maxPtpPorts = 33;
        supportTM = GT_FALSE;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }


    if(unitIdPtr != NULL)
    {
        for(ii = 0 ; ii < NUM_ENTRIES_FOR_8_MSBITS_BASE_ADDR_CNS ; ii++)
        {
            /* init the table with 'not valid' */
            unitIdPtr[ii] = PRV_CPSS_DXCH_UNIT_LAST_E;
        }

        /* array of unitBasePtr[] is with different size per family/sub family
           iterations need to be until it's end and not according to 'constant'  */
        for(ii = 0 ; unitBasePtr[ii] != END_OF_TABLE_INDICATOR_CNS ; ii++)
        {
            /* according to unitId we get the 8 MSBits of the address*/
            unit8MSBits = unitBasePtr[ii] >> 24;
            if(unitIdPtr[unit8MSBits] != PRV_CPSS_DXCH_UNIT_LAST_E)
            {
                /* support BC3 units that the '2 pipes' are in the same '8 MSbits' */
                continue;
            }
            /* with those 8 MSBits we access the unitId table and save the unitId (ii)*/
            unitIdPtr[unit8MSBits] = ii;
        }

        /* fill also the 'large units' */
        for(ii = 0 ; prvDxChLargeUnitsArr[ii].unitId != END_OF_TABLE_INDICATOR_CNS; ii++)
        {
            unit8MSBits =
                (unitBasePtr[prvDxChLargeUnitsArr[ii].unitId]) >> 24;

            for(jj = 0 ; jj < prvDxChLargeUnitsArr[ii].numOfSubUnits; jj++)
            {
                unitIdPtr[unit8MSBits + jj] = prvDxChLargeUnitsArr[ii].unitId;
            }
        }
    }

    bindToUnusedUnitsCheckFunction(devNum);

    /* reset the DB */
    dbReset(devNum);

    /* init the TTI unit*/
    ttiInit(devNum);
    /* set the addresses of the TTI unit registers according to the needed base address */
    regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_TTI_E], TTI));

    /* init the L2I unit*/
    l2iInit(devNum);
    /* set the addresses of the L2I unit registers according to the needed base address */
    regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_L2I_E], L2I));

    /* init the FDB unit*/
    fdbInit(devNum);
    /* set the addresses of the FDB unit registers according to the needed base address */
    regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_FDB_E], FDB));

    /* init the EQ unit*/
    eqInit(devNum);
    /* set the addresses of the EQ unit registers according to the needed base address */
    regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_EQ_E], EQ));

    /* init the TCAM unit*/
    tcamInit(devNum);
    /* set the addresses of the TCAM unit registers according to the needed base address */
    regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_TCAM_E], TCAM));

    /* init the LPM unit */
    lpmInit(devNum);
    /* set the addresses of the LPM unit registers according to the needed base address */
    regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum,unitBasePtr[PRV_CPSS_DXCH_UNIT_LPM_E], LPM));

    /* init the EGF unit (EFT,SHT,QAG) */
    egfInit(devNum);
    /* set the addresses of the EGF unit registers according to the needed base address */
    regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_EGF_EFT_E], EGF_eft));
    regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_EGF_QAG_E], EGF_qag));
    regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_EGF_SHT_E], EGF_sht));

    /* init the HA unit */
    haInit(devNum);
    /* set the addresses of the HA unit registers according to the needed base address */
    regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_HA_E], HA));

    /* init the MLL unit */
    mllInit(devNum);
    /* set the addresses of the MLL unit registers according to the needed base address */
    regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_MLL_E], MLL));

    /* init the PCL unit (IPCL) */
    pclInit(devNum);
    /* set the addresses of the PCL unit registers (IPCL) according to the needed base address */
    regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_PCL_E], PCL));

    /* init the EPCL unit (EPCL) */
    epclInit(devNum);
    /* set the addresses of the PCL unit registers (EPCL) according to the needed base address */
    regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_EPCL_E], EPCL));

    /* init for RxDma,txDma,txFifo */
    dataPathInit(devNum,unitBasePtr);

    /* init for ingress aggregator */
    ingressAggregatorInit(devNum);

    /* init ingress OAM */
    oamInit(devNum,PRV_CPSS_DXCH_UNIT_IOAM_E);
    /* set the addresses of the ingress OAM unit registers according to the needed base address */
    regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_IOAM_E], OAMUnit[0]));
    /* init egress OAM */
    oamInit(devNum,PRV_CPSS_DXCH_UNIT_EOAM_E);
    /* set the addresses of the egress OAM unit registers according to the needed base address */
    regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_EOAM_E], OAMUnit[1]));

    /* init ermrk (egress mark/remark) */
    ermrkInit(devNum);
    /* set the addresses of the ermrk unit registers according to the needed base address */
    regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_ERMRK_E], ERMRK));


    /* init BM */
    bmInit(devNum);
    /* set the addresses of the BM unit registers according to the needed base address */
    regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_BM_E], BM));

    /* init TAI (Time Application Interface) */
    if(supportSingleTaiUnit == GT_FALSE)
    {
        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            /* 1 Tile = 1 Master + 2 CP Slave + 2 TxQ + 4 Ravens*/
            totalTaiGop = PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles *
                                    FALCON_PER_TILE_TAI_SUB_UNITS_NUMBER_CNS;
        }
        else
        {
            totalTaiGop = (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)) ? 1 : 10;
        }

        for (ii = 0; (ii < totalTaiGop); ii++)
        {
            for (jj = 0; (jj < 2); jj++)
            {
                gopTaiInit(devNum, ii /*gopIndex*/, jj /*taiIndex*/);
                indexArr[0] = ii;
                indexArr[1] = jj;
                subunitBase = prvCpssDxChHwSubunitBaseAddrGet(
                    devNum, PRV_CPSS_SUBUNIT_GOP_TAI_E, indexArr);

                /* set the addresses of the TAI unit registers according to the needed base address */
                regUnitBaseAddrSet(
                    REG_UNIT_INFO_MAC(devNum, subunitBase, GOP.TAI[ii][jj]));
            }
        }
    }
    else
    {
        /* single TAI device */
        gopTaiInit(devNum, 0 /*gopIndex*/, 0 /*taiIndex*/);
        subunitBase = unitBasePtr[PRV_CPSS_DXCH_UNIT_TAI_E];
        regUnitBaseAddrSet(
            REG_UNIT_INFO_MAC(devNum, subunitBase, GOP.TAI[0][0]));
    }

    /* init GOP MTI */
    for (ii = 0 ; ii < maxMtiPorts ; ii++)
    {
        /* Init GOP MTI registers addresses */
        gopMtiInit(devNum, ii);

        /* Init GOP MSDB registers addresses */
        gopMsdbInit(devNum, ii);
    }

    /* init GOP CG */
    for (ii = 0 ; ii < maxCgPorts ; ii++)
    {
        /* Setting index as the port number */
        indexArr[0] = ii;

        /* CG subunit is available for ports in multiple of 4 */
        if ( ii % 4 !=0 )
        {
            continue;
        }

        /* Get CG subunit base address */
        subunitBase = prvCpssDxChHwSubunitBaseAddrGet(
                devNum, PRV_CPSS_SUBUNIT_GOP_CG_E, indexArr);

        /* Init CG subunit registers addresses */
        gopCgInit(devNum, ii);

        /* Add base address to the subunit registers */
        regUnitBaseAddrSet(
            REG_UNIT_INFO_MAC(devNum, subunitBase, GOP.CG[ii].CG_CONVERTERS));
        regUnitBaseAddrSet(
            REG_UNIT_INFO_MAC(devNum, subunitBase, GOP.CG[ii].CG_PORT.CG_PORT_MAC));
    }

    /* init GOP PTP  */
    for (ii = 0; (ii < maxPtpPorts); ii++)
    {
        if(ii != CPU_SDMA_ID)/*NOTE: index 72 used for interlaken (not as PTP of MAC)*/
        {
            /* skip non exists MAC ports */
            PRV_CPSS_DXCH_SKIP_NOT_EXIST_PORT_MAC(devNum,ii);
        }

        indexArr[0] = ii;
        subunitBase = prvCpssDxChHwSubunitBaseAddrGet(
            devNum, PRV_CPSS_SUBUNIT_GOP_PTP_E, indexArr);
        if(subunitBase == 0xFFFFFFFF)
        {
            /* ILKN ... for device that not supports ILKN*/
            continue;
        }

        gopPtpInit(devNum, ii /*macNum*/);

        /* set the addresses of the PTP unit registers according to the needed base address */
        regUnitBaseAddrSet(
            REG_UNIT_INFO_MAC(devNum, subunitBase, GOP.PTP[ii]));

        if(ii < maxFcaPorts)
        {
            gopFcaInit(devNum, ii /*macNum*/);
            subunitBase = prvCpssDxChHwSubunitBaseAddrGet(
                devNum, PRV_CPSS_SUBUNIT_GOP_FCA_E, indexArr);

            /* set the addresses of the FCA unit registers according to the needed base address */
            regUnitBaseAddrSet(
                REG_UNIT_INFO_MAC(devNum, subunitBase, GOP.FCA[ii]));
        }
    }

    if(supportILKN == GT_TRUE)
    {
        gopPrInit(devNum);
        subunitBase = prvCpssDxChHwSubunitBaseAddrGet(
            devNum, PRV_CPSS_SUBUNIT_GOP_PR_E, indexArr);
        /* set the addresses of the GOP.PR unit registers according to the needed base address */
        regUnitBaseAddrSet(
            REG_UNIT_INFO_MAC(devNum, subunitBase, GOP.PR));

        gopIlknInit(devNum);
        regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_ILKN_E], GOP.ILKN));
        regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_ILKN_E], GOP.ILKN_WRP));
    }

    ipInit(devNum);
    /* set the addresses of the ip unit registers according to the needed base address */
    regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_IPVX_E], IPvX));

    /* init the PLR unit (3 stages - IPLR, IPLR_1 & EPLR) */
    for (ii = 0; (ii < 3); ii++)
    {
        plrInit(devNum, ii);
    }
    /* set the addresses of the PLR unit registers according to the needed base address */
    regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_IPLR_E],   PLR[0]));
    regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_IPLR_1_E], PLR[1]));
    regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_EPLR_E],   PLR[2]));

    /* init the CNC unit (2 instances) */
    for (ii = 0; (ii < 2); ii++)
    {
        cncInit(devNum, ii);
    }
    /* set the addresses of the CNC unit registers according to the needed base address */
    regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_CNC_0_E], CNC[0]));
    regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_CNC_1_E], CNC[1]));

    /* TXQ unit init & destroy */
    if(!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* init the TXQ_QCN unit*/
        txqQcnInit(devNum);
        /* set the addresses of the TXQ_QCN unit registers according to the needed base address */
        regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_TXQ_QCN_E], TXQ.qcn));

        /* init the TXQ_BMX unit*/
        txqBmxInit(devNum);
        /* set the addresses of the TXQ_QCN unit registers according to the needed base address */
        regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_TXQ_BMX_E], TXQ.bmx));

        /* init the TXQ_LL unit*/
        txqLLInit(devNum);
        /* set the addresses of the TXQ_LL unit registers according to the needed base address */
        regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_TXQ_LL_E], TXQ.ll));

        /* init the TXQ_PFC unit*/
        txqPfcInit(devNum);
        /* set the addresses of the TXQ_PFC unit registers according to the needed base address */
        regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_TXQ_PFC_E], TXQ.pfc));

        /* init the TXQ_QUEUE unit*/
        txqQueueInit(devNum);
        /* set the addresses of the TXQ_QUEUE unit registers according to the needed base address */
        regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E], TXQ.queue));
    }
    else
    {
        txqPdxInit_sip6(devNum);

        regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_TXQ_PDX_E], TXQ.pdx));

        regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_TXQ_PDX_PAC_0_E], TXQ.pdx_pac[0]));

        regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_TXQ_PDX_PAC_1_E], TXQ.pdx_pac[1]));

        txqPsiInit_sip6(devNum);

        regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_TXQ_PSI_E], TXQ.psi_regs));

        for(ii=0;ii<CPSS_DXCH_SIP6_PDS_UNITS_PER_TILE_MAC;ii++)
        {

            txqPdsInit_sip6(devNum,ii);

            regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS0_E+ii], TXQ.pds[ii]));

            txqSdqInit_sip6(devNum,ii);

            regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ0_E+ii], TXQ.sdq[ii]));
         }

         txqPfccInit_sip6(devNum);

          regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_TXQ_PFCC_E], TXQ.pfcc));

         for(ii=0;ii<CPSS_DXCH_SIP6_QFC_UNITS_PER_TILE_MAC;ii++)
         {
            txqQfcInit_sip6(devNum,ii);

            regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC0_E+ii], TXQ.qfc[ii]));
         }

        /* no update addresses needed - this function already added correct bases */
        packetBufferInit_sip6(devNum);

        for(ii = 0; ii < PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles; ii++)
        {
            for(jj = 0; jj < FALCON_RAVENS_PER_TILE; jj++)
            {
                lmuInit(devNum, PRV_DXCH_LMU_NUM_MAC(ii, jj, 0));
                regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum,
                    FALCON_TILE_OFFSET_CNS * ii +
                    unitBasePtr[PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E + jj] +
                    0x00430000, LMU[PRV_DXCH_LMU_NUM_MAC(ii, jj, 0)]));

                lmuInit(devNum, PRV_DXCH_LMU_NUM_MAC(ii, jj, 1));
                regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum,
                    FALCON_TILE_OFFSET_CNS * ii +
                    unitBasePtr[PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E + jj] +
                    0x004B0000, LMU[PRV_DXCH_LMU_NUM_MAC(ii, jj, 1)]));
            }
        }

    }

    /* init the Bobcat2 only units */
    if (supportTM == GT_TRUE)
    {
        tmDropInit(devNum);
        regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_TM_DROP_E], TMDROP));

        tmQMapInit(devNum);
        regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_TM_QMAP_E], TMQMAP));

        tmIngressGlueInit(devNum);
        regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_TM_INGRESS_GLUE_E], TM_INGR_GLUE));

        tmEgressGlueInit(devNum);
        regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_TM_EGRESS_GLUE_E], TM_EGR_GLUE));

        tmFcuInit(devNum);
        regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_TM_FCU_E], TM_FCU));

        switch (PRV_CPSS_PP_MAC(devNum)->devFamily)
        {
            case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
                switch (PRV_CPSS_PP_MAC(devNum)->devSubFamily)
                {
                    case CPSS_PP_SUB_FAMILY_NONE_E: /* BC2 */
                        ethTxFifoInit(devNum,0);
                        regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_ETH_TXFIFO_E], SIP_ETH_TXFIFO[0]));
                        if(supportILKN == GT_TRUE)
                        {
                            ilknTxFifoInit(devNum);
                            regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_ILKN_TXFIFO_E], SIP_ILKN_TXFIFO));
                        }
                    break;
                    case CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E:
                        ethTxFifoInit(devNum,0);
                        regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_ETH_TXFIFO_E], SIP_ETH_TXFIFO[0]));

                        ethTxFifoInit(devNum,1);
                        regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_ETH_TXFIFO1_E], SIP_ETH_TXFIFO[1]));
                    break;
                    default:
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                    }
                }
            break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
    }

    if(supportLMS == GT_TRUE)
    {
        /* init the LMS unit (3 instances)*/
        for (ii = 0; ii < 3; ii++)
        {
            lmsInit(devNum, ii);
        }

        /* set the addresses of the LMS unit registers according to the needed base address */
        regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_LMS0_0_E], GOP.LMS[0]));
        regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_LMS1_0_E], GOP.LMS[1]));
        regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_LMS2_0_E], GOP.LMS[2]));
    }
    else /* the LMS unit was split into 2 separate units SMI , LED */
    {
        prv_smiInit(devNum);
        prv_ledInit(devNum);
    }

    /* init TG (gop) registers */
    gopMacTGInit(devNum);
    /* set the addresses of the MG unit registers according to the needed base address */
    regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_GOP_E], packGenConfig));


    /* init MG unit */
    mgInit(devNum);
    /* set the addresses of the MG unit registers according to the needed base address */
    regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_MG_E], MG));

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* init EREP unit */
        erepInit(devNum);
        /* set the addresses of the EREP unit registers according to the needed base address */
        regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_EREP_E], EREP));

        /* init PREQ unit */
        preQInit(devNum);
        /* set the addresses of the EREP unit registers according to the needed base address */
        regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_PREQ_E], PREQ));

        /* init SHM unit */
        shmInit_sip6(devNum);
        /* set the addresses of the SHM unit registers according to the needed base address */
        regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_SHM_E], SHM));

        /* init the D2D CP of eagle */
        sip6EagleD2dCpInit_regDb(devNum);

        /* init the EM unit*/
        emInit(devNum);
        /* set the addresses of the EM unit registers according to the needed base address */
        regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_EM_E], EM));


        /* init PHA unit */
        phaInit_sip6(devNum);
        /* set the addresses of the SHM unit registers according to the needed base address */
        regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_PHA_E], PHA));

    }


    /* the rest of the registers that we not manage to 'destroy' in the old DB.
       we need to convert align their addresses to base addresses of the actual
       corresponding units
        the assumption is that the 'old DB' is based on Lion2 units base addresses !
    */
    rc = convertOldRegDbToNewUnitsBaseAddr(devNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = bcat2MacMibCountersInit(devNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    /*--------------------------------------------------------------*
     *   Init RXDMA /TXDMA GLUE if required / BobK/aldrin
     *   pay attention:
     *      +------------------+-------------+-----------+------+
     *      |registers' number |   RxDmAGlue | TxDMAGlue | MPPM |
     *      +------------------+-------------+-----------+------+
     *      |           BobK   |     37      |    37     |  93  |
     *      |          Aldrin  |     78      |    37     |  93  |
     *      |           BC3    |     ???     |   ???     |  ??? |
     *      +------------------+-------------+-----------+------+
     *--------------------------------------------------------------*/
    switch (PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
            switch (PRV_CPSS_PP_MAC(devNum)->devSubFamily)
            {
                case CPSS_PP_SUB_FAMILY_NONE_E: /* BC2 */
                    mppmInit(devNum);
                    regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_MPPM_E], MPPM));
                    /* do nothing */
                break;
                case CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E:
                    rc = bobkRxDMAGlueInit(devNum,PRV_CPSS_DXCH_BobK_PA_RX_TX_DMA_GLUE_MAP_REGNUM_CNS);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                    rc = bobkTxDMAGlueInit(devNum,PRV_CPSS_DXCH_BobK_PA_RX_TX_DMA_GLUE_MAP_REGNUM_CNS);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                    rc = bobkMPPMInit(devNum,PRV_CPSS_DXCH_BobK_PA_MPPM_MAP_REGNUM_CNS);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }

                    regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_RXDMA_GLUE_E], RX_DMA_GLUE));
                    regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_TXDMA_GLUE_E], TX_DMA_GLUE));
                    regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_MPPM_E], MPPM));
                break;
                default:
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                }
            }
        break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
        case CPSS_PP_FAMILY_DXCH_AC3X_E:
            rc = bobkRxDMAGlueInit(devNum,PRV_CPSS_DXCH_Aldrin_PA_RX_DMA_GLUE_MAP_REGNUM_CNS);
            if (rc != GT_OK)
            {
                return rc;
            }

            rc = bobkTxDMAGlueInit(devNum,PRV_CPSS_DXCH_Aldrin_PA_TX_DMA_GLUE_MAP_REGNUM_CNS);
            if (rc != GT_OK)
            {
                return rc;
            }

            rc = bobkMPPMInit(devNum,PRV_CPSS_DXCH_BobK_PA_MPPM_MAP_REGNUM_CNS);
            if (rc != GT_OK)
            {
                return rc;
            }

            regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_RXDMA_GLUE_E], RX_DMA_GLUE));
            regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_TXDMA_GLUE_E], TX_DMA_GLUE));
            regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_MPPM_E], MPPM));
        break;
        case CPSS_PP_FAMILY_DXCH_BOBCAT3_E:
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            mppmInit(devNum);
            regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_MPPM_E], MPPM));
            break;
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            /* currently do nothing , add support in future */
            break;
        default:
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
    }

    /* the device hold also GOP per pipe */
    if(PRV_CPSS_IS_MULTI_PIPES_DEVICE_MAC(devNum))
    {
        GT_U32  pipeId;
        /* update the DB of 'per port' MAC registers , about the extra pipes */
        for(pipeId = 1; pipeId < PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipes; pipeId++)
        {
            updateGopOldRegDbWithMultiPipe(devNum,pipeId);
        }
    }

    /* Until here, 36 ports (0-35) were filled, and copied to other 36 port
       half in case of multi-pipe device. In BC3 we have another case */
    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
    {
        /* In Bobcat3, for PTP and FCA we have to fill two more ports: 72, 73
           that correspond to port 36 pipe 0 and port 36 pipe 1*/
        for (ii = 72; (ii <= 73); ii++)
        {
            /* Get base address for port ii for PTP unit */
            indexArr[0] = ii;
            subunitBase = prvCpssDxChHwSubunitBaseAddrGet(
                devNum, PRV_CPSS_SUBUNIT_GOP_PTP_E, indexArr);

            /* Init PTP in element of port ii */
            gopPtpInit(devNum, ii);

            /* set the addresses of the PTP unit registers according to the given base address */
            regUnitBaseAddrSet(
                REG_UNIT_INFO_MAC(devNum, subunitBase, GOP.PTP[ii]));

            /* Get base address for port ii for FCA unit */
            subunitBase = prvCpssDxChHwSubunitBaseAddrGet(
                devNum, PRV_CPSS_SUBUNIT_GOP_FCA_E, indexArr);

            /* Init FCA in element of port ii */
            gopFcaInit(devNum, ii /*macNum*/);

            /* set the addresses of the FCA unit registers according to the given base address */
            regUnitBaseAddrSet(
                REG_UNIT_INFO_MAC(devNum, subunitBase, GOP.FCA[ii]));
        }
    }

    /* Until here, 36 ports (0-35) were filled, and copied to other 36 port
       half in case of multi-pipe device. In BC3 we have another case */
    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E
        && PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles == 1)
    {
        /* currently Falcon as 3.2 Tera device */

        /* In Bobcat3, for PTP and FCA we have to fill one more port: 64
           that correspond to port 32 pipe 1 */
        ii = 64;

        if(ii < PRV_CPSS_PP_MAC(devNum)->numOfPorts)
        {
            /* Get base address for port ii for PTP unit */
            indexArr[0] = ii;
            subunitBase = prvCpssDxChHwSubunitBaseAddrGet(
                devNum, PRV_CPSS_SUBUNIT_GOP_PTP_E, indexArr);

            /* Init PTP in element of port ii */
            gopPtpInit(devNum, ii);

            /* set the addresses of the PTP unit registers according to the given base address */
            regUnitBaseAddrSet(
                REG_UNIT_INFO_MAC(devNum, subunitBase, GOP.PTP[ii]));

            /* Get base address for port ii for FCA unit */
            subunitBase = prvCpssDxChHwSubunitBaseAddrGet(
                devNum, PRV_CPSS_SUBUNIT_GOP_FCA_E, indexArr);

            /* Init FCA in element of port ii */
            gopFcaInit(devNum, ii /*macNum*/);

            /* set the addresses of the FCA unit registers according to the given base address */
            regUnitBaseAddrSet(
                REG_UNIT_INFO_MAC(devNum, subunitBase, GOP.FCA[ii]));
        }
    }


    return GT_OK;
}

/**
* @internal prvCpssDxChHwResetAndInitControllerRegAddrInit function
* @endinternal
*
* @brief   This function initializes the Reset and Init Controller registers
*         struct for eArch devices.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_CPU_MEM        - on malloc failed
* @retval GT_BAD_PARAM             - wrong device type to operate
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChHwResetAndInitControllerRegAddrInit
(
    IN  GT_U8 devNum
)
{
    GT_STATUS rc;
    GT_U32 * unitBasePtr;
    GT_BOOL isSip6 = PRV_CPSS_SIP_6_CHECK_MAC(devNum);
    PRV_CPSS_RESET_AND_INIT_CTRL_REGS_ADDR_STC *regsAddrPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E| CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    resetAndInitControllerDbReset(devNum);

    regsAddrPtr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum);

    {/*start of unit DFXServerUnitsDeviceSpecificRegs */
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceResetCtrl = 0x000f800c;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.initializationStatusDone = 0x000f8014;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.configSkipInitializationMatrix = 0x000f8020;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.RAMInitSkipInitializationMatrix = 0x000f8030;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.trafficManagerSkipInitializationMatrix = 0x000f8040;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.tableSkipInitializationMatrix = 0x000f8060;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.SERDESSkipInitializationMatrix = 0x000f8064;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.EEPROMSkipInitializationMatrix = 0x000f8068;
        /*0x000f806c : replaced with 0x000f8058 because:
          0x000f806c was not impact 'pex reset' --> erratum.
          so we need to use register 0x000f8058 that is 'skip CPU reset' and it
          also skip PEX registers (BARs : 0,1,2)

          note: registers  0x000f8050,0x000f8054 are 'pipe0,pip1' reset registers

          NOTE: when PCIeSkipInitializationMatrix = 0x000f806c and not 0x000f8058
            bit 5 PCIe0RstOutMaskTrst --> must be 0 in register:
            <Control and Management Sub System> sip_msys_Units/Runit_RFU/SOC_Misc/RSTOUTn Mask
        */
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.PCIeSkipInitializationMatrix = /*0x000f806c*/0x000f8058;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.DDRSkipInitializationMatrix = 0x000f8048;

        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceSAR1 = 0x000f8200;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceSAR2 = 0x000f8204;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceCtrl0 = 0x000f8250;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceCtrl4 = 0x000f8260;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceCtrl5 = 0x000f8264;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceCtrl8 = 0x000f8270;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceCtrl10 = 0x000f8278;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceCtrl12 = 0x000f8280;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceCtrl21 = 0x000f82a4;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceCtrl20 = 0x000f82a0;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceCtrl19 = 0x000f829c;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceCtrl18 = 0x000f8298;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceCtrl15 = 0x000f828c;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceCtrl14 = 0x000f8288;
        if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum) == 0)
        {
            regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceStatus0 = 0x000f82b0;
            regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceStatus1 = 0x000f82b4;
        }
        else
        {
            regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceStatus0 = 0x000f8c80;
            regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceStatus1 = 0x000f8c84;
        }
        if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
        {
            /* DFX Registers Configuration Skip Initialization Matrix for BobK*/
            regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.DFXSkipInitializationMatrix = 0x000f8098;
        }

        if(PRV_CPSS_SIP_5_16_CHECK_MAC(devNum))
        {
            /* MBUS Control Skip Initialization Matrix for Aldrin */
            regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.PCIeSkipInitializationMatrix = 0x000f8044;
        }

        if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
        {   /* Bobcat3/Armstrong */
            regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceCtrl1 = 0x000f8254;
            regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceCtrl16 = 0x000f8290;
            /* MBUS Control Skip Initialization Matrix */
            regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.PCIeSkipInitializationMatrix = 0x000f804C;
        }

        if(PRV_CPSS_SIP_5_25_CHECK_MAC(devNum))
        {   /* Aldrin2 */
            /* CPU Control Skip Initialization Matrix */
            regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.PCIeSkipInitializationMatrix = 0x000f8040;
            regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceCtrl33 = 0x000F8D24;
        }

        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceCtrl28 = 0x000F8D10;
            regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceCtrl29 = 0x000F8D14;
        }

        if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.configSkipInitializationMatrix = 0x000f8020;
            regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.SRRSkipInitializationMatrix = 0x000f8024;
            regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.BISTSkipInitializationMatrix = 0x000f8028;
            regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.SoftRepairSkipInitializationMatrix = 0x000f802c;
            regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.RAMInitSkipInitializationMatrix = 0x000f8030;
            regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.RegxGenxSkipInitializationMatrix = 0x000f8034;
            regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.MultiActionSkipInitializationMatrix = 0x000f8038;
            regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.tableSkipInitializationMatrix = 0x000f8060;
            regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.SERDESSkipInitializationMatrix = 0x000f8064;
            regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.EEPROMSkipInitializationMatrix = 0x000f8068;
            regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.PCIeSkipInitializationMatrix = 0x000f806c;
            regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.DeviceEEPROMSkipInitializationMatrix = 0x000f8090;
            regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.D2DLinkSkipInitializationMatrix = 0x000f8094;
            regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.DFXRegistersSkipInitializationMatrix = 0x000f8098;
            regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.DFXPipeSkipInitializationMatrix = 0x000f809C;
            regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.Core_TileSkipInitializationMatrix = 0x000f8040;
            regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.Core_MngSkipInitializationMatrix = 0x000f8044;
            regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.Core_D2DSkipInitializationMatrix = 0x000f8048;
            regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.Core_RavenSkipInitializationMatrix = 0x000f804C;
            regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.GOP_D2DSkipInitializationMatrix = 0x000f8040;
            regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.GOP_MainSkipInitializationMatrix = 0x000f8044;
            regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.GOP_CNMSkipInitializationMatrix = 0x000f8048;
            regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.GOP_GWSkipInitializationMatrix = 0x000f804C;
        }

    }/*end of unit DFXServerUnitsDeviceSpecificRegs */


    {/*start of unit DFXServerUnits */
        {/*start of unit DFXServerRegs */
            {/*f812c+28*a*/
                GT_U32    a;
                for(a = 0 ; a <= 0 ; a++) {
                    regsAddrPtr->DFXServerUnits.DFXServerRegs.AVSDisabledCtrl2[a] =
                        0xf812c+28*a;
                }/* end of loop a */
            }/*f812c+28*a*/
            {/*f8128+28*a*/
                GT_U32    a;
                for(a = 0 ; a <= 0 ; a++) {
                    regsAddrPtr->DFXServerUnits.DFXServerRegs.AVSDisabledCtrl1[a] =
                        0xf8128+28*a;
                }/* end of loop a */
            }/*f8128+28*a*/
            {/*f8450+t*4*/
                GT_U32    t;
                for(t = 1 ; t <= 4 ; t++) {
                    regsAddrPtr->DFXServerUnits.DFXServerRegs.serverXBARTargetPortConfig[t-1] =
                        0xf8450+t*4;
                }/* end of loop t */
            }/*f8450+t*4*/
            regsAddrPtr->DFXServerUnits.DFXServerRegs.serverStatus = 0x000f8010;
            regsAddrPtr->DFXServerUnits.DFXServerRegs.serverInterruptSummaryCause = 0x000f8100;
            regsAddrPtr->DFXServerUnits.DFXServerRegs.serverInterruptSummaryMask = 0x000f8104;
            regsAddrPtr->DFXServerUnits.DFXServerRegs.serverInterruptMask = 0x000f810c;
            regsAddrPtr->DFXServerUnits.DFXServerRegs.serverInterruptCause = 0x000f8108;
            regsAddrPtr->DFXServerUnits.DFXServerRegs.serverAddrSpace = 0x000f8018;
            regsAddrPtr->DFXServerUnits.DFXServerRegs.pipeSelect = 0x000f8000;

            if(!isSip6)
            {
                regsAddrPtr->DFXServerUnits.DFXServerRegs.temperatureSensor28nmCtrlLSB = 0x000f8070;
                regsAddrPtr->DFXServerUnits.DFXServerRegs.temperatureSensor28nmCtrlMSB = 0x000f8074;
                regsAddrPtr->DFXServerUnits.DFXServerRegs.temperatureSensorStatus      = 0x000f8078;
            }
            else
            {
                regsAddrPtr->DFXServerUnits.DFXServerRegs.temperatureSensorStatus      = 0x000f8078;
                regsAddrPtr->DFXServerUnits.DFXServerRegs.temperatureSensor28nmCtrlLSB = 0x000f807c;
                regsAddrPtr->DFXServerUnits.DFXServerRegs.temperatureSensor28nmCtrlMSB = 0x000f8080;
            }

            {/*f8134+28*a*/
                GT_U32    a;
                for(a = 0 ; a <= 0 ; a++) {
                    regsAddrPtr->DFXServerUnits.DFXServerRegs.AVSStatus[a] =
                        0xf8134+28*a;
                }/* end of loop a */
            }/*f8134+28*a*/
            {/*f8138+28*a*/
                GT_U32    a;
                for(a = 0 ; a <= 0 ; a++) {
                    regsAddrPtr->DFXServerUnits.DFXServerRegs.AVSMinStatus[a] =
                        0xf8138+28*a;
                }/* end of loop a */
            }/*f8138+28*a*/
            {/*f813c+28*a*/
                GT_U32    a;
                for(a = 0 ; a <= 0 ; a++) {
                    regsAddrPtr->DFXServerUnits.DFXServerRegs.AVSMaxStatus[a] =
                        0xf813c+28*a;
                }/* end of loop a */
            }/*f813c+28*a*/
            {/*f8130+28*a*/
                GT_U32    a;
                for(a = 0 ; a <= 0 ; a++) {
                    regsAddrPtr->DFXServerUnits.DFXServerRegs.AVSEnabledCtrl[a] =
                        0xf8130+28*a;
                }/* end of loop a */
            }/*f8130+28*a*/
            regsAddrPtr->DFXServerUnits.DFXServerRegs.snoopBusStatus = 0x000f8220;
            regsAddrPtr->DFXServerUnits.DFXServerRegs.deviceIDStatus = 0x000f8240;
            regsAddrPtr->DFXServerUnits.DFXServerRegs.JTAGDeviceIDStatus = 0x000f8244;
            regsAddrPtr->DFXServerUnits.DFXServerRegs.PLLCoreConfig = 0x000f82e4;
            regsAddrPtr->DFXServerUnits.DFXServerRegs.PLLCoreParameters = 0x000f82e0;

        }/*end of unit DFXServerRegs */

    }/*end of unit DFXServerUnits */

    {/*start of unit DFXClientUnits*/
        /* skip DFXClientUnits during registers dump printing */
        regsAddrPtr->dfxClientBookmark     = PRV_CPSS_SW_PTR_BOOKMARK_CNS;
        regsAddrPtr->dfxClientBookmarkType = PRV_CPSS_SW_TYPE_WRONLY_CNS;
        regsAddrPtr->dfxClientBookmarkSize = sizeof(regsAddrPtr->DFXClientUnits);

        regsAddrPtr->DFXClientUnits.clientControl     = 0x00002000;
        regsAddrPtr->DFXClientUnits.clientStatus      = 0x00002010;
        regsAddrPtr->DFXClientUnits.clientDataControl = 0x00002008;
        {
            GT_U32    a;
            for(a = 0 ; a <= 3 ; a++) {
                regsAddrPtr->DFXClientUnits.clientRamBISTInfo[a] = 0x2040+4*a;
            }/* end of loop a */
        }
        regsAddrPtr->DFXClientUnits.BISTControl       = 0x00002070;
        regsAddrPtr->DFXClientUnits.BISTOpCode        = 0x00002074;
        regsAddrPtr->DFXClientUnits.BISTMaxAddress    = 0x00002080;
        regsAddrPtr->DFXClientUnits.dummyWrite        = 0x000020b0;
        {
            GT_U32    a;
            for(a = 0 ; a <= 3 ; a++) {
                regsAddrPtr->DFXClientUnits.clientRamMC_Group[a] = 0x20E0+4*a;
            }/* end of loop a */
        }
    }/*end of unit DFXClientUnits*/

    {/*start of unit DFXRam*/
        /* skip DFXRam during registers dump printing */
        regsAddrPtr->dfxRamBookmark     = PRV_CPSS_SW_PTR_BOOKMARK_CNS;
        regsAddrPtr->dfxRamBookmarkType = PRV_CPSS_SW_TYPE_WRONLY_CNS;
        regsAddrPtr->dfxRamBookmarkSize = sizeof(regsAddrPtr->DFXRam);

        regsAddrPtr->DFXRam.memoryControl    = 0x14;
    }/*end of unit DFXRam*/

    if (isSip6)
    {
        /* set the addresses of the DFX unit registers according to the needed base address */
        rc = prvCpssDxChUnitBaseTableGet(devNum,/*OUT*/&unitBasePtr);
        if ((rc == GT_OK) &&
            (unitBasePtr[PRV_CPSS_DXCH_UNIT_DFX_SERVER_E] != NON_VALID_BASE_ADDR_CNS))
        {
            regUnitBaseAddrSet(
                (GT_U32*) &regsAddrPtr->DFXServerUnitsDeviceSpecificRegs,
                sizeof(regsAddrPtr->DFXServerUnitsDeviceSpecificRegs) / sizeof(GT_U32),
                unitBasePtr[PRV_CPSS_DXCH_UNIT_DFX_SERVER_E]);
            regUnitBaseAddrSet(
                (GT_U32*) &regsAddrPtr->DFXServerUnits,
                sizeof(regsAddrPtr->DFXServerUnits) / sizeof(GT_U32),
                unitBasePtr[PRV_CPSS_DXCH_UNIT_DFX_SERVER_E]);
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChXcat3HwResetAndInitControllerRegAddrInit function
* @endinternal
*
* @brief   This function initializes the Reset and Init Controller (DFX) registers
*         struct for xCat3 devices.
* @param[in] devNum                   - The PP's device number to init the struct for.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_CPU_MEM        - on malloc failed
* @retval GT_BAD_PARAM             - wrong device type to operate
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChXcat3HwResetAndInitControllerRegAddrInit
(
    IN  GT_U8 devNum
)
{
    PRV_CPSS_RESET_AND_INIT_CTRL_REGS_ADDR_STC *regsAddrPtr;

    resetAndInitControllerDbReset(devNum);

    regsAddrPtr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum);

    {/*start of unit DFXServerUnitsDeviceSpecificRegs */
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceResetCtrl = 0x000F800C;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.initializationStatusDone = 0x000F8014;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.configSkipInitializationMatrix = 0x000F8020;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.RAMInitSkipInitializationMatrix = 0x000F8030;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.tableSkipInitializationMatrix = 0x000F8060;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.SERDESSkipInitializationMatrix = 0x000F8064;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.EEPROMSkipInitializationMatrix = 0x000F8068;

        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.PCIeSkipInitializationMatrix = 0x000f806c;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceSAR1 = 0x000F8200;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceSAR2 = 0x000F8204;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceSAR1Override = 0x000F82D4;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceSAR2Override = 0x000F82D8;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceCtrl16 = 0x000F8290;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceCtrl14 = 0x000F8288;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceCtrl0 = 0x000F8250;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.pllMiscConfig = 0x000F82F4;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceCtrl10 = 0x000F8278;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceCtrl6 = 0x000F8268;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceCtrl7 = 0x000f826c;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceCtrl9 = 0x000F8274;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.pllMiscParameters = 0x000F82F0;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.CPUSkipInitializationMatrix = 0x000f8044;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.DFXSkipInitializationMatrix = 0x000f8048;

    }/*end of unit DFXServerUnitsDeviceSpecificRegs */

    {/*start of unit DFXServerUnits */
        regsAddrPtr->DFXServerUnits.DFXServerRegs.serverStatus = 0x000f8010;
        regsAddrPtr->DFXServerUnits.DFXServerRegs.snoopBusStatus = 0x000f8220;
        regsAddrPtr->DFXServerUnits.DFXServerRegs.temperatureSensor28nmCtrlLSB = 0x000f8070;
        regsAddrPtr->DFXServerUnits.DFXServerRegs.temperatureSensor28nmCtrlMSB = 0x000f8074;
        regsAddrPtr->DFXServerUnits.DFXServerRegs.temperatureSensorStatus = 0x000f8078;
        regsAddrPtr->DFXServerUnits.DFXServerRegs.pipeSelect = 0x000F8000;

    }/*end of unit DFXServerUnits */

    {/*start of unit DFXClientUnits*/
        /* skip DFXClientUnits during registers dump printing */
        regsAddrPtr->dfxClientBookmark     = PRV_CPSS_SW_PTR_BOOKMARK_CNS;
        regsAddrPtr->dfxClientBookmarkType = PRV_CPSS_SW_TYPE_WRONLY_CNS;
        regsAddrPtr->dfxClientBookmarkSize = sizeof(regsAddrPtr->DFXClientUnits);

        regsAddrPtr->DFXClientUnits.clientControl     = 0x00002000;
        regsAddrPtr->DFXClientUnits.clientStatus      = 0x00002010;
        regsAddrPtr->DFXClientUnits.clientDataControl = 0x00002008;
        {
            GT_U32    a;
            for(a = 0 ; a <= 3 ; a++) {
                regsAddrPtr->DFXClientUnits.clientRamBISTInfo[a] = 0x2040+4*a;
            }/* end of loop a */
        }
        regsAddrPtr->DFXClientUnits.BISTControl       = 0x00002070;
        regsAddrPtr->DFXClientUnits.BISTOpCode        = 0x00002074;
        regsAddrPtr->DFXClientUnits.BISTMaxAddress    = 0x00002080;
        regsAddrPtr->DFXClientUnits.dummyWrite        = 0x000020b0;
        {
            GT_U32    a;
            for(a = 0 ; a <= 3 ; a++) {
                regsAddrPtr->DFXClientUnits.clientRamMC_Group[a] = 0x20E0+4*a;
            }/* end of loop a */
        }
    }/*end of unit DFXClientUnits*/

    {/*start of unit DFXRam*/
        /* skip DFXRam during registers dump printing */
        regsAddrPtr->dfxRamBookmark     = PRV_CPSS_SW_PTR_BOOKMARK_CNS;
        regsAddrPtr->dfxRamBookmarkType = PRV_CPSS_SW_TYPE_WRONLY_CNS;
        regsAddrPtr->dfxRamBookmarkSize = sizeof(regsAddrPtr->DFXRam);

        regsAddrPtr->DFXRam.memoryControl    = 0x14;
    }/*end of unit DFXRam*/

    return GT_OK;
}

/* Raven and Falcon DFX address : */
#define DFX_TEMPERATURE_SENSOR_ADDR_FIRST   0x000F8078
#define DFX_TEMPERATURE_SENSOR_ADDR_LAST    0x000F808C

#define DFX_AVS_ADDR_FIRST      0x000F8120
#define DFX_AVS_ADDR_LAST       0x000F813C

#define DFX_PLL_ADDR_0          0x000F82E0
#define DFX_PLL_ADDR_1          0x000F82E4
#define DFX_PLL_ADDR_2          0x000F8490
#define DFX_PLL_ADDR_3          0x000F8494

/* ONLY Falcon DFX address : */
#define DFX_PLL_ADDR_LAST       0x000F8314

static GT_U32   trace_ADDRESS_NOT_SUPPORTED_MAC  = 0;
void trace_ADDRESS_NOT_SUPPORTED_MAC_set(GT_U32 enable)
{
    trace_ADDRESS_NOT_SUPPORTED_MAC = enable;
}

#define EMULATOR_RETURN_ON_ADDRESS_NOT_SUPPORTED_MAC(reasonString)           \
    {                                                                        \
        if(trace_ADDRESS_NOT_SUPPORTED_MAC)                                  \
        {                                                                    \
            cpssOsPrintf("Emulator : address [0x%8.8x] skipped [%s]\n",      \
                regAddr,                                                     \
                reasonString);                                               \
        }                                                                    \
                                                                             \
        return GT_FALSE;                                                     \
    }


/**
* @internal prvCpssDxChHwIsUnitUsed function
* @endinternal
*
* @brief   This function checks existences of base addresses units in the Emulator/VERIFIER
*         of the GM for the given device.
*         since the Emulator/VERIFIER not supports all units , the CPSS wants to avoid
*         accessing the Emulator/VERIFIER.
*         the CPSS binds the simulation with this functions so the simulation before
*         accessing the GM (and Emulator/VERIFIER) can know no to call them on such addresses.
*         NOTE: purpose is to reduce Emulator/VERIFIER 'ERRORS' in LOG , and even 'crash' of
*         Emulator/VERIFIER on some unknown addresses.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - thr PP's device number to init the struct for.
* @param[in] portGroupId              - port group ID
* @param[in] regAddr                  - register address
*
* @retval GT_TRUE                  - the unit is implemented in the Emulator/VERIFIER
* @retval GT_FALSE                 - the unit is NOT implemented in the Emulator/VERIFIER
*/
GT_BOOL prvCpssDxChHwIsUnitUsed
(
    IN GT_U32                   devNum,
    IN GT_U32                   portGroupId,
    IN GT_U32                   regAddr
)
{
    PRV_CPSS_DXCH_UNIT_ENT unitId;  /* Unit ID */
#ifdef GM_USED  /* the GM supports address completion */
    GT_U32 gmUsed = 1;
#else
    GT_U32 gmUsed = 0;
#endif
    GT_U32 relativeRegAddr;
    GT_U32 relativeRegAddr1;

    portGroupId = portGroupId;

    if(!PRV_CPSS_PP_MAC(devNum))
    {
        /* the device not defined yet at the CPSS
           (function called from the driver early stages !) */

        /* since we can't check the address ... assume it exists */
        return GT_TRUE;
    }


    if (!PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        /* CPSS not aware that this GM device need to skip accessing to any unit */
        return GT_TRUE;
    }

    unitId = prvCpssDxChHwRegAddrToUnitIdConvert((GT_U8)devNum, regAddr);

    if(gmUsed)
    {
        if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            switch(unitId)
            {
                case PRV_CPSS_DXCH_UNIT_TCAM_E:
                    return GT_TRUE;/* the unit was added to GM code */

                case PRV_CPSS_DXCH_UNIT_LPM_E:
                    return GT_TRUE;/* the CPSS expected to set it properly */

                /* new units in the control pipe that expected to be supported
                   by the GM */
                case PRV_CPSS_DXCH_UNIT_EREP_E:
                case PRV_CPSS_DXCH_UNIT_PREQ_E:
                case PRV_CPSS_DXCH_UNIT_TXQ_PDX_E       :
                case PRV_CPSS_DXCH_UNIT_TXQ_PDX_PAC_0_E :
                case PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ0_E:
                case PRV_CPSS_DXCH_UNIT_IA_E:
                    return GT_TRUE;

                case PRV_CPSS_DXCH_UNIT_RXDMA_E:
                    return GT_TRUE;  /* at least RXDMA[0] exists */
                case PRV_CPSS_DXCH_UNIT_RXDMA1_E:
                case PRV_CPSS_DXCH_UNIT_RXDMA2_E:
                case PRV_CPSS_DXCH_UNIT_RXDMA3_E:
                    return GT_FALSE;  /* The GM currently not hold the RxDma and TxDma , it uses the HBU */

                case PRV_CPSS_DXCH_UNIT_TXDMA_E:
                case PRV_CPSS_DXCH_UNIT_TXDMA1_E:
                case PRV_CPSS_DXCH_UNIT_TXDMA2_E:
                case PRV_CPSS_DXCH_UNIT_TXDMA3_E:
                    return GT_FALSE;  /* The GM currently not hold the RxDma and TxDma , it uses the HBU */

                case PRV_CPSS_DXCH_UNIT_TX_FIFO_E:
                case PRV_CPSS_DXCH_UNIT_TX_FIFO1_E:
                case PRV_CPSS_DXCH_UNIT_TX_FIFO2_E:
                case PRV_CPSS_DXCH_UNIT_TX_FIFO3_E:
                    return GT_FALSE;  /* The GM TxFifo , it uses the HBU */

                case PRV_CPSS_DXCH_UNIT_SHM_E:
                    return GT_FALSE;  /* The GM not hold this unit */

                case PRV_CPSS_DXCH_UNIT_TXQ_PFCC_E:
                    return GT_FALSE;  /* The GM not hold this unit */

                case PRV_CPSS_DXCH_UNIT_EM_E:
                    return GT_TRUE;

                default:
                    /* check if unit is SIP5 supported unit */
                    break;
            }
        }

        switch(unitId)
        {   /* List of existing units */
            case PRV_CPSS_DXCH_UNIT_TTI_E:
            case PRV_CPSS_DXCH_UNIT_PCL_E:
            case PRV_CPSS_DXCH_UNIT_L2I_E:
            case PRV_CPSS_DXCH_UNIT_FDB_E:
            case PRV_CPSS_DXCH_UNIT_EQ_E:
            case PRV_CPSS_DXCH_UNIT_LPM_E:
            case PRV_CPSS_DXCH_UNIT_EGF_EFT_E:
            case PRV_CPSS_DXCH_UNIT_EGF_QAG_E:
            case PRV_CPSS_DXCH_UNIT_EGF_SHT_E:
            case PRV_CPSS_DXCH_UNIT_HA_E:
            case PRV_CPSS_DXCH_UNIT_ETS_E:
            case PRV_CPSS_DXCH_UNIT_MLL_E:
            case PRV_CPSS_DXCH_UNIT_IPLR_E:
            case PRV_CPSS_DXCH_UNIT_IPLR_1_E:
            case PRV_CPSS_DXCH_UNIT_EPLR_E:
            case PRV_CPSS_DXCH_UNIT_IPVX_E:
            case PRV_CPSS_DXCH_UNIT_IOAM_E:
            case PRV_CPSS_DXCH_UNIT_EOAM_E:
            case PRV_CPSS_DXCH_UNIT_TCAM_E:
            case PRV_CPSS_DXCH_UNIT_RXDMA_E:
            case PRV_CPSS_DXCH_UNIT_EPCL_E:
            case PRV_CPSS_DXCH_UNIT_MIB_E:
            case PRV_CPSS_DXCH_UNIT_ERMRK_E:
            case PRV_CPSS_DXCH_UNIT_CNC_0_E:
            case PRV_CPSS_DXCH_UNIT_CNC_1_E:
            case PRV_CPSS_DXCH_UNIT_TM_DROP_E:
            case PRV_CPSS_DXCH_UNIT_TM_QMAP_E:
            case PRV_CPSS_DXCH_UNIT_MG_E:
            case PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E:
            case PRV_CPSS_DXCH_UNIT_TXQ_LL_E:
            /*case PRV_CPSS_DXCH_UNIT_TXQ_PFC_E:*/
            case PRV_CPSS_DXCH_UNIT_TXQ_QCN_E:
            case PRV_CPSS_DXCH_UNIT_TXQ_BMX_E:
            case PRV_CPSS_DXCH_UNIT_TXQ_DQ_E:
            /* new in bobk */
    /* GM not implemented the second unit
            case PRV_CPSS_DXCH_UNIT_RXDMA1_E:
            case PRV_CPSS_DXCH_UNIT_TXDMA1_E:
            case PRV_CPSS_DXCH_UNIT_TX_FIFO1_E:
    */
                return GT_TRUE;

                /*dp[1]*/
            case PRV_CPSS_DXCH_UNIT_TXQ_DQ1_E:
            case PRV_CPSS_DXCH_UNIT_TXDMA1_E:
            case PRV_CPSS_DXCH_UNIT_RXDMA1_E:
                /*dp[2]*/
            case PRV_CPSS_DXCH_UNIT_TXQ_DQ2_E:
            case PRV_CPSS_DXCH_UNIT_TXDMA2_E:
            case PRV_CPSS_DXCH_UNIT_RXDMA2_E:
                /*dp[3]*/
            case PRV_CPSS_DXCH_UNIT_TXQ_DQ3_E:
            case PRV_CPSS_DXCH_UNIT_TXDMA3_E:
            case PRV_CPSS_DXCH_UNIT_RXDMA3_E:

            case PRV_CPSS_DXCH_UNIT_BM_E:
            case PRV_CPSS_DXCH_UNIT_BMA_E: /* supports port mapping */

                if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
                {
                    /* the CPU SDMA of MG0 is port 24 in DP[0] , so we need to allow to configure it */
                    return GT_TRUE;
                }
                else
                {
                    return GT_FALSE;
                }

            default:
                switch(regAddr & 0xFF000000)
                {
                    case PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_LPM_1_CNS:
                        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
                        {
                            /* in Aldrin2 the GM supports the 2 LPMs (was not in BC3 GM)*/
                            return GT_TRUE;
                        }
                    default:
                        break;
                }

                return GT_FALSE;
        }
    }

    /* code for emulator on Falcon
       (called only when cpssDeviceRunCheck_onEmulator())*/
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
    {
        if(regAddr >= 0x04000000 &&
           regAddr  < 0x08000000)
        {
            /* avoid access to 'multicast' region that the CPSS currently not intended to access .

               getting here means that CPSS still 'think' in BC3 style !!!

               avoid potential 'stuck'/'unexpected behavior' of the device ,
               in the 'multicast' region
            */
            EMULATOR_RETURN_ON_ADDRESS_NOT_SUPPORTED_MAC("'multicast' region");
        }


        switch(unitId)
        {
            case PRV_CPSS_DXCH_UNIT_EAGLE_D2D_CP_0_IN_RAVEN_E:
            case PRV_CPSS_DXCH_UNIT_EAGLE_D2D_CP_1_IN_RAVEN_E:
                /*Eagle D2D-CP internal        524032        B                00280000        002FFEFF*/
                /*Eagle D2D-CP AMB internal        256       B                002FFF00        002FFFFF*/

                /* give explicit indication that we allow it */
                return GT_TRUE;


            case PRV_CPSS_DXCH_UNIT_SERDES_E:
                /* 'legacy' unsupported unit */
                EMULATOR_RETURN_ON_ADDRESS_NOT_SUPPORTED_MAC("'legacy' unsupported unit");
            case PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E: /*Emulator support*/
            case PRV_CPSS_DXCH_UNIT_BASE_RAVEN_1_E:
            case PRV_CPSS_DXCH_UNIT_BASE_RAVEN_2_E:
            case PRV_CPSS_DXCH_UNIT_BASE_RAVEN_3_E:

                relativeRegAddr = regAddr & 0x00FFFFFF;

                CPSS_TBD_BOOKMARK_FALCON_EMULATOR
                /* Currently disable Raven access due to device stuck on first access to Raven from PEX */
#if 0   /*allow to access the Raven */
                if(cpssDeviceRunCheck_onEmulator())
                    EMULATOR_RETURN_ON_ADDRESS_NOT_SUPPORTED_MAC("Full Raven bypass");
#endif /*0*/

                /* allow limited access to : Raven A0/SD1/Units/<SD1> SD1 wrapper %s*/
                /* 0x00340000 + 0x1000*s: where s (0-16) represents serdes */
                switch(regAddr & 0x00FE0FFF) /*steps of 0x1000*/
                {
                    case 0x00340000:/*SERDES External Configuration 0*/
                    case 0x00340004:/*SERDES External Configuration 1*/
                        return GT_TRUE;
                    default:
                        break;
                }


                switch(regAddr & 0x00FFFF00)
                {
                    case 0x00644C00:
                        /*/Cider/Switching Dies/Raven A0/Raven_A0 {Current}/Device Units/<AVAGO_PMRO> AVAGO PMRO */
                        EMULATOR_RETURN_ON_ADDRESS_NOT_SUPPORTED_MAC("Raven_A0 {Current}/Device Units/<AVAGO_PMRO> AVAGO PMRO");
                    case 0x00640000:
                        /*/Cider/Switching Dies/Raven A0/Raven_A0 {Current}/Device Units/<AVAGO_SBUS_MASTER> SBUS_MASTER */
                        EMULATOR_RETURN_ON_ADDRESS_NOT_SUPPORTED_MAC("Raven_A0 {Current}/Device Units/<AVAGO_SBUS_MASTER> SBUS_MASTER");
                    case 0x00600000:
                        /*/Cider/Switching Dies/Raven A0/Raven_A0 {Current}/Device Units/<SBC> SBC Units */
                        EMULATOR_RETURN_ON_ADDRESS_NOT_SUPPORTED_MAC("Raven_A0 {Current}/Device Units/<SBC> SBC Units");
                    default:
                        break;
                }


                /*AVG SD        17        KB        6        00640400        006447FF*/
                if(relativeRegAddr >= 0x00640400 &&
                   relativeRegAddr <  0x006447FF)
                {
                    /*/Cider/Switching Dies/Raven A0/Raven_A0 {Current}/Device Units/<AVAGO_SERDES> AVAGO_SERDES_16NM */
                    /*0x00640400 + 0x400*s: where s (0-16) represents serdes*/
                    EMULATOR_RETURN_ON_ADDRESS_NOT_SUPPORTED_MAC("Raven_A0 {Current}/Device Units/<AVAGO_SERDES> AVAGO_SERDES_16NM");
                }

                /*TSEN        1        KB        6        00644800        00644BFF*/
                if(relativeRegAddr >= 0x00644800 &&
                   relativeRegAddr <  0x00644BFF)
                {
                    /*/Cider/Switching Dies/Raven A0/Raven_A0 {Current}/Device Units/<AVAGO_TEMP_SENS> Avago_temp_sense */
                    EMULATOR_RETURN_ON_ADDRESS_NOT_SUPPORTED_MAC("Raven_A0 {Current}/Device Units/<AVAGO_TEMP_SENS> Avago_temp_sense");
                }

                /*D2D0        64        KB        6        00680000        0068FFFF
                  D2D1        64        KB        6        00690000        0069FFFF*/
                if((relativeRegAddr & 0x00FF0000) == 0x00680000 || /*<D2D0>*/
                   (relativeRegAddr & 0x00FF0000) == 0x00690000 )  /*<D2D1>*/
                {
                    relativeRegAddr1 = relativeRegAddr & 0x0000FFFF;/*remove the 0x680000 or 0x690000*/

                    if(relativeRegAddr1 < 0x4000)
                    {
                        /* SERDES registers */
                        EMULATOR_RETURN_ON_ADDRESS_NOT_SUPPORTED_MAC("Raven_A0 {Current}/Device Units/D2D 0 or 1 /SERDES registers");
                    }

                    return GT_TRUE;
                }

                /*DFX        1        MB        7        00700000        007FFFFF*/
                if((relativeRegAddr & 0x00FF0000) == 0x00700000)
                {
                    relativeRegAddr1 = relativeRegAddr & 0x000FFFFF;/*remove the 0x700000*/
                    /* DFX registers */
                    switch(relativeRegAddr1)
                    {
                        /* PLL registers */
                        case DFX_PLL_ADDR_0:
                        /*/Cider/Switching Dies/Raven A0/Raven_A0 {Current}/Reset And Initialization/DFX/DFX Units/DFX Server Registers/PLL core Parameters*/
                        case DFX_PLL_ADDR_1:
                        /*/Cider/Switching Dies/Raven A0/Raven_A0 {Current}/Reset And Initialization/DFX/DFX Units/DFX Server Registers/PLL core Config*/
                        case DFX_PLL_ADDR_2:
                        /*/Cider/Switching Dies/Raven A0/Raven_A0 {Current}/Reset And Initialization/DFX/DFX Units/DFX Server Registers/PLL Counter Control*/
                        case DFX_PLL_ADDR_3:
                        /*/Cider/Switching Dies/Raven A0/Raven_A0 {Current}/Reset And Initialization/DFX/DFX Units/DFX Server Registers/PLL Counter Status*/
                            EMULATOR_RETURN_ON_ADDRESS_NOT_SUPPORTED_MAC("Raven_A0 {Current}/Reset And Initialization/DFX/DFX Units/DFX Server Registers/PLL registers");

                        default:
                            break;
                    }

                    if(relativeRegAddr1 >= DFX_AVS_ADDR_FIRST &&
                       relativeRegAddr1 <= DFX_AVS_ADDR_LAST)
                    {
                        /*DFX - AVS registers*/
                        /*/Cider/Switching Dies/Raven A0/Raven_A0 {Current}/Reset And Initialization/DFX/DFX Units/DFX Server Registers/Server AVS Control*/
                        /*...*/
                        /*/Cider/Switching Dies/Raven A0/Raven_A0 {Current}/Reset And Initialization/DFX/DFX Units/DFX Server Registers/avs Max Status*/
                        EMULATOR_RETURN_ON_ADDRESS_NOT_SUPPORTED_MAC("Raven_A0 {Current}/Reset And Initialization/DFX/DFX Units/DFX Server Registers/AVS registers");
                    }

                    if(relativeRegAddr1 >= DFX_TEMPERATURE_SENSOR_ADDR_FIRST &&
                       relativeRegAddr1 <= DFX_TEMPERATURE_SENSOR_ADDR_LAST)
                    {
                        /* DFX - Temperature Sensor */
                        /*/Cider/Switching Dies/Raven A0/Raven_A0 {Current}/Reset And Initialization/DFX/DFX Units/DFX Server Registers/Temperature Sensor Status*/
                        /*...*/
                        /*/Cider/Switching Dies/Raven A0/Raven_A0 {Current}/Reset And Initialization/DFX/DFX Units/DFX Server Registers/External Temperature Sensor Status*/
                        EMULATOR_RETURN_ON_ADDRESS_NOT_SUPPORTED_MAC("Raven_A0 {Current}/Reset And Initialization/DFX/DFX Units/DFX Server Registers/Temperature Sensor");
                    }
                }


                return GT_TRUE;/* supports the Raven register */

            case PRV_CPSS_DXCH_UNIT_DFX_SERVER_E:
                relativeRegAddr1 = regAddr & 0x000FFFFF;/*remove the prefix of the unit*/
                /* DFX registers */
                switch(relativeRegAddr1)
                {
                    /* PLL registers */
                    case DFX_PLL_ADDR_2:
                    case DFX_PLL_ADDR_3:
                        EMULATOR_RETURN_ON_ADDRESS_NOT_SUPPORTED_MAC("eagle /DFX/DFX Units/DFX Server Registers/PLL Registers");

                    default:
                        break;
                }

                if(relativeRegAddr1 >= DFX_PLL_ADDR_0 &&
                   relativeRegAddr1 <= DFX_PLL_ADDR_LAST)
                {
                    /*DFX - PLL registers*/
                    EMULATOR_RETURN_ON_ADDRESS_NOT_SUPPORTED_MAC("eagle /DFX/DFX Units/DFX Server Registers/PLL Registers");
                }

                if(relativeRegAddr1 >= DFX_AVS_ADDR_FIRST &&
                   relativeRegAddr1 <= DFX_AVS_ADDR_LAST)
                {
                    /*DFX - AVS registers*/
                    EMULATOR_RETURN_ON_ADDRESS_NOT_SUPPORTED_MAC("eagle /DFX/DFX Units/DFX Server Registers/AVS Registers");
                }

                if(relativeRegAddr1 >= DFX_TEMPERATURE_SENSOR_ADDR_FIRST &&
                   relativeRegAddr1 <= DFX_TEMPERATURE_SENSOR_ADDR_LAST)
                {
                    /* DFX - Temperature Sensor */
                    EMULATOR_RETURN_ON_ADDRESS_NOT_SUPPORTED_MAC("eagle /DFX/DFX Units/DFX Server Registers/Temperature Sensor");
                }

                return GT_TRUE;/* supports the DFX register */


            case PRV_CPSS_DXCH_UNIT_EAGLE_D2D_0_E:
            case PRV_CPSS_DXCH_UNIT_EAGLE_D2D_1_E:
            case PRV_CPSS_DXCH_UNIT_EAGLE_D2D_2_E:
            case PRV_CPSS_DXCH_UNIT_EAGLE_D2D_3_E:
            case PRV_CPSS_DXCH_UNIT_EAGLE_D2D_4_E:
            case PRV_CPSS_DXCH_UNIT_EAGLE_D2D_5_E:
            case PRV_CPSS_DXCH_UNIT_EAGLE_D2D_6_E:
            case PRV_CPSS_DXCH_UNIT_EAGLE_D2D_7_E:
                relativeRegAddr1 = regAddr & 0x0000FFFF;/*remove the prefix of the unit*/

                if(relativeRegAddr1 < 0x4000)
                {
                    /* SERDES registers */
                    EMULATOR_RETURN_ON_ADDRESS_NOT_SUPPORTED_MAC("eagle /Device Units/D2D 0..7/SERDES registers");
                }

                return GT_TRUE;/* supports the D2D register */

            default:
                /* supports the unit */
                break;
        }

        /* supports the unit */
        return GT_TRUE;
    }

    /* code for emulator on ALDRIN2
       (called only when cpssDeviceRunCheck_onEmulator())*/
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
    {
        switch(regAddr >> 24)
        {
            case 0x24: /* SBC Controller */
            case 0xA4: /* SBC Controller */
                /* NOT supports the unit */
                return GT_FALSE;
            default:
                break;
        }

        switch(unitId)
        {   /* List of NON existing units */
            case PRV_CPSS_DXCH_UNIT_SERDES_E:
                /* allow limited access to : GOP/<SD_WRAPPER> SD_IP Units<<%s>>*/
                /* 0x13000000 + 0x1000*s: where s (0-32) represents serdes */
                switch(regAddr & 0x00000FFF) /*steps of 0x1000*/
                {
                    case 0x000:/*SERDES External Configuration 0*/
                    case 0x004:/*SERDES External Configuration 1*/
                        return GT_TRUE;
                    default:
                        break;
                }
                return GT_FALSE;

            case PRV_CPSS_DXCH_UNIT_SMI_0_E:
            case PRV_CPSS_DXCH_UNIT_SMI_1_E:
            case PRV_CPSS_DXCH_UNIT_SMI_2_E:
            case PRV_CPSS_DXCH_UNIT_SMI_3_E:
            case PRV_CPSS_DXCH_UNIT_LMS2_0_E:/* by 'mistake' gets to 'LMS2_0' instead of 'SMI_0' */
            case PRV_CPSS_DXCH_UNIT_LMS2_1_E:/* by 'mistake' gets to 'LMS2_1' instead of 'SMI_1' */
            case PRV_CPSS_DXCH_UNIT_LMS2_2_E:/* by 'mistake' gets to 'LMS2_2' instead of 'SMI_2' */
            case PRV_CPSS_DXCH_UNIT_LMS2_3_E:/* by 'mistake' gets to 'LMS2_3' instead of 'SMI_3' */

                /* NOT supports the unit */
                return GT_FALSE;
            default:
                /* supports the unit */
                break;
        }

        /* supports the unit */
        return GT_TRUE;
    }

    /* code for emulator on ALDRIN
       (called only when cpssDeviceRunCheck_onEmulator())*/
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E)
    {
        if((regAddr & 0xFF000000) == 0x60000000)
        {
            /*SERDES And SBC Controller*/
            /* NOT supports the unit */
            return GT_FALSE;
        }


        switch(unitId)
        {   /* List of NON existing units */
            case PRV_CPSS_DXCH_UNIT_TCAM_E:
            case PRV_CPSS_DXCH_UNIT_RXDMA_E:
            case PRV_CPSS_DXCH_UNIT_RXDMA1_E:
            case PRV_CPSS_DXCH_UNIT_TXDMA_E:
            case PRV_CPSS_DXCH_UNIT_TXDMA1_E:
            case PRV_CPSS_DXCH_UNIT_TX_FIFO_E:
            case PRV_CPSS_DXCH_UNIT_TX_FIFO1_E:
                if(cpssDeviceRunCheck_onEmulator_isAldrinFull())
                {
                    return GT_TRUE;
                }
                return GT_FALSE;

            case PRV_CPSS_DXCH_UNIT_SERDES_E:
                /* allow limited access to : GOP/<SD_WRAPPER> SD_IP Units<<%s>>*/
                /* 0x13000000 + 0x1000*s: where s (0-32) represents serdes */
                switch(regAddr & 0x00000FFF) /*steps of 0x1000*/
                {
                    case 0x000:/*SERDES External Configuration 0*/
                    case 0x004:/*SERDES External Configuration 1*/
                        return GT_TRUE;
                    default:
                        break;
                }
                return GT_FALSE;


            case PRV_CPSS_DXCH_UNIT_SMI_0_E:
            case PRV_CPSS_DXCH_UNIT_SMI_1_E:
            case PRV_CPSS_DXCH_UNIT_SMI_2_E:
            case PRV_CPSS_DXCH_UNIT_SMI_3_E:
            case PRV_CPSS_DXCH_UNIT_LMS2_0_E:/* by 'mistake' gets to 'LMS2_0' instead of 'SMI_0' */
            case PRV_CPSS_DXCH_UNIT_LMS2_1_E:/* by 'mistake' gets to 'LMS2_1' instead of 'SMI_1' */
            case PRV_CPSS_DXCH_UNIT_LMS2_2_E:/* by 'mistake' gets to 'LMS2_2' instead of 'SMI_2' */
            case PRV_CPSS_DXCH_UNIT_LMS2_3_E:/* by 'mistake' gets to 'LMS2_3' instead of 'SMI_3' */

                /* NOT supports the unit */
                return GT_FALSE;

            case PRV_CPSS_DXCH_UNIT_GOP_E:/*accessed from 'interrupts init mask registers' */
                if(cpssDeviceRunCheck_onEmulator_isAldrinFull())
                {
                    return GT_TRUE;
                }

                {
                    GT_U32  portNum = (regAddr >> 12) & 0x3f;/*steps of 0x1000*/
                    if(!PRV_CPSS_PHY_PORT_IS_EXIST_MAC(devNum, portNum))
                    {
                        return GT_FALSE;
                    }

                    return GT_TRUE;
                }
            /*NOTE : next units PRV_CPSS_DXCH_UNIT_MIB_E ,
                                PRV_CPSS_DXCH_UNIT_SERDES_E
                are not mapped in 'interrupts init mask registers' so not accessed
            */

            default:
                /* supports the unit */
                return GT_TRUE;
        }
    }

    /* unit supported */
    return GT_TRUE;
}

#ifdef ASIC_SIMULATION /* needed for GM*/
/* checks existence of unit according to address */
typedef GT_BOOL (* SMEM_GM_HW_IS_UNIT_USED_PROC_FUN ) (
                   IN GT_U32       cpssDevNum,
                   IN GT_U32       portGroupId,
                   IN GT_U32       regAddr
);

/* init unit memory use checking function */
extern void smemGmUnitUsedFuncInit
(
    IN GT_U32       simDeviceId,
    IN GT_U32       cpssDevNum,
    IN SMEM_GM_HW_IS_UNIT_USED_PROC_FUN unitUsedFuncPtr
);

/* convert CPSS devNum, coreId to simulation deviceId */
extern GT_STATUS prvCpssDrvDevNumToSimDevIdConvert
(
    IN  GT_U8       devNum,
    IN  GT_U32      portGroupId,
    OUT GT_U32      *simDeviceIdPtr
);
#endif /*ASIC_SIMULATION*/

static GT_STATUS   bindToUnusedUnitsCheckFunction(IN GT_U8 devNum)
{
    if (PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
#ifdef ASIC_SIMULATION /* needed for GM only*/
        GT_STATUS rc;
        GT_U32      simDeviceId;
        /* Convert devNum to simDeviceId */
        rc = prvCpssDrvDevNumToSimDevIdConvert(devNum, 0, &simDeviceId);
        if(rc != GT_OK)
        {
            return rc;
        }

        /*
            bind the simulation with CallBack function to advice about addresses
            that belongs to 'non exists' units in the VERIFIER.

            the simulation will 'ignore' write to such register , and will
            'read' value of '0x00badadd'.

            the prvCpssDxChHwIsUnitUsed(...) should be called
            only after prvCpssDxChHwRegAddrVer1Init(...)
        */

        smemGmUnitUsedFuncInit(simDeviceId, devNum, prvCpssDxChHwIsUnitUsed);
#endif /*ASIC_SIMULATION*/
    }
    return GT_OK;
}


/**
* @internal egfInit_sip6 function
* @endinternal
*
* @brief   sip6 : init the DB - EGF (EFT,SHT,QAG)
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None
*/
static void egfInit_sip6
(
    IN  GT_U8 devNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    {/*start of unit EGF_sht */
        {/*start of unit global */
            {/*start of unit miscellaneous */
/*              regsAddrPtr->EGF_sht.global.miscellaneous.badAddrLatchReg = 0x000b1140;*/

            }/*end of unit miscellaneous */


            {/*00b07c0+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 511 ; n++) {
                    regsAddrPtr->EGF_sht.global.eportAssociatedVid1[n] =
                        0x00b07c0+n*0x4;
                }/* end of loop n */
            }/*00b07c0+n*0x4*/
            {/*00b0040+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    regsAddrPtr->EGF_sht.global.UCSrcIDFilterEn[n] =
                        0x00b0040+0x4*n;
                }/* end of loop n */
            }/*00b0040+0x4*n*/
            regsAddrPtr->EGF_sht.global.meshIdConfigs = 0x000b0004;
            {/*00b10c0+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    regsAddrPtr->EGF_sht.global.MCLocalEn[n] =
                        0x00b10c0+0x4*n;
                }/* end of loop n */
            }/*00b10c0+0x4*n*/
            regsAddrPtr->EGF_sht.global.SHTInterruptsMask = 0x000b0020;
            {/*00b1040+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    regsAddrPtr->EGF_sht.global.ignorePhySrcMcFilterEn[n] =
                        0x00b1040+0x4*n;
                }/* end of loop n */
            }/*00b1040+0x4*n*/
            regsAddrPtr->EGF_sht.global.SHTGlobalConfigs = 0x000b0000;
            {/*00b02c0+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    regsAddrPtr->EGF_sht.global.eportStpStateMode[n] =
                        0x00b02c0+0x4*n;
                }/* end of loop n */
            }/*00b02c0+0x4*n*/
            {/*00b0340+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    regsAddrPtr->EGF_sht.global.eportStpState[n] =
                        0x00b0340+0x4*n;
                }/* end of loop n */
            }/*00b0340+0x4*n*/
            {/*00b01c0+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    regsAddrPtr->EGF_sht.global.eportRoutedForwardRestricted[n] =
                        0x00b01c0+0x4*n;
                }/* end of loop n */
            }/*00b01c0+0x4*n*/
            {/*00b03c0+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 63 ; n++) {
                    regsAddrPtr->EGF_sht.global.eportPortIsolationMode[n] =
                        0x00b03c0+0x4*n;
                }/* end of loop n */
            }/*00b03c0+0x4*n*/
            {/*00b0540+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    regsAddrPtr->EGF_sht.global.eportOddOnlySrcIdFilterEnable[n] =
                        0x00b0540+0x4*n;
                }/* end of loop n */
            }/*00b0540+0x4*n*/
            {/*00b05c0+n * 0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 127 ; n++) {
                    regsAddrPtr->EGF_sht.global.ePortMeshId[n] =
                        0x00b05c0+n * 0x4;
                }/* end of loop n */
            }/*00b05c0+n * 0x4*/
            {/*00b04c0+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    regsAddrPtr->EGF_sht.global.eportIPMCRoutedFilterEn[n] =
                        0x00b04c0+0x4*n;
                }/* end of loop n */
            }/*00b04c0+0x4*n*/
            {/*00b00c0+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    regsAddrPtr->EGF_sht.global.eportFromCPUForwardRestricted[n] =
                        0x00b00c0+0x4*n;
                }/* end of loop n */
            }/*00b00c0+0x4*n*/
            {/*00b0240+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    regsAddrPtr->EGF_sht.global.eportEvlanFilteringEnable[n] =
                        0x00b0240+0x4*n;
                }/* end of loop n */
            }/*00b0240+0x4*n*/
            regsAddrPtr->EGF_sht.global.eportVlanEgrFiltering = 0x000b000c;
            {/*00b0fc0+n * 0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    regsAddrPtr->EGF_sht.global.dropOnEportVid1MismatchEn[n] =
                        0x00b0fc0+n * 0x4;
                }/* end of loop n */
            }/*00b0fc0+n * 0x4*/
            {/*00b0140+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    regsAddrPtr->EGF_sht.global.eportBridgedForwardRestricted[n] =
                        0x00b0140+0x4*n;
                }/* end of loop n */
            }/*00b0140+0x4*n*/
            regsAddrPtr->EGF_sht.global.SHTInterruptsCause = 0x000b0010;
            regsAddrPtr->EGF_sht.global.SHTEgrFiltersEnable = 0x000b0008;

        }/*end of unit global */


    }/*end of unit EGF_sht */

    {/*start of unit EGF_eft */
        {/*start of unit mcFIFO */
            {/*start of unit mcFIFOConfigs */
                {/*0002200+4*n*/
                    GT_U32    n;
                    for(n = 0 ; n <= 31 ; n++) {
                        regsAddrPtr->EGF_eft.MCFIFO.MCFIFOConfigs.portToMcFIFOAttributionReg[n] =
                            0x0002200+4*n;
                    }/* end of loop n */
                }/*0002200+4*n*/
                regsAddrPtr->EGF_eft.MCFIFO.MCFIFOConfigs.MCFIFOGlobalConfigs = 0x00002100;
                regsAddrPtr->EGF_eft.MCFIFO.MCFIFOConfigs.MCFIFOArbiterWeights0 = 0x00002108;
                regsAddrPtr->EGF_eft.MCFIFO.MCFIFOConfigs.priorityMulticast = 0x00002280;

            }/*end of unit mcFIFOConfigs */


        }/*end of unit mcFIFO */


        {/*start of unit miscellaneous */
            /*regsAddrPtr->EGF_eft.miscellaneous.badAddrLatchReg = 0x00005000;*/

        }/*end of unit miscellaneous */


        {/*start of unit global */
            {/*0001350+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    regsAddrPtr->EGF_eft.global.bypassEGFTargetEPortAttributes[n] =
                        0x0001350+n*0x4;
                }/* end of loop n */
            }/*0001350+n*0x4*/
            {/*0001150+4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 127 ; n++) {
                    regsAddrPtr->EGF_eft.global.cpuPortMapOfReg[n] =
                        0x0001150+4*n;
                }/* end of loop n */
            }/*0001150+4*n*/
            regsAddrPtr->EGF_eft.global.EFTInterruptsMask = 0x000010b0;
            regsAddrPtr->EGF_eft.global.portIsolationLookup0 = 0x00001020;
            regsAddrPtr->EGF_eft.global.portIsolationLookup1 = 0x00001030;
            regsAddrPtr->EGF_eft.global.EFTInterruptsCause = 0x000010a0;
            regsAddrPtr->EGF_eft.global.cpuPortDist = 0x00001000;
            regsAddrPtr->EGF_eft.global.ECCConf = 0x00001130;
            regsAddrPtr->EGF_eft.global.ECCStatus = 0x00001140;

        }/*end of unit global */


        {/*start of unit egrFilterConfigs */
            {/*0006100+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    regsAddrPtr->EGF_eft.egrFilterConfigs.unregedMCFilterEn[n] =
                        0x0006100+0x4*n;
                }/* end of loop n */
            }/*0006100+0x4*n*/
            {/*0006180+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    regsAddrPtr->EGF_eft.egrFilterConfigs.unregedBCFilterEn[n] =
                        0x0006180+0x4*n;
                }/* end of loop n */
            }/*0006180+0x4*n*/
            {/*0006080+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    regsAddrPtr->EGF_eft.egrFilterConfigs.unknownUCFilterEn[n] =
                        0x0006080+0x4*n;
                }/* end of loop n */
            }/*0006080+0x4*n*/
            {/*0006380+4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    regsAddrPtr->EGF_eft.egrFilterConfigs.physicalPortLinkStatusMask[n] =
                        0x0006380+4*n;
                }/* end of loop n */
            }/*0006380+4*n*/
            {/*0006200+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    regsAddrPtr->EGF_eft.egrFilterConfigs.OAMLoopbackFilterEn[n] =
                        0x0006200+0x4*n;
                }/* end of loop n */
            }/*0006200+0x4*n*/
            {/*0006300+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    regsAddrPtr->EGF_eft.egrFilterConfigs.enFCTriggerByCNFrameOnPort[n] =
                        0x0006300+0x4*n;
                }/* end of loop n */
            }/*0006300+0x4*n*/
            {/*0006280+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    regsAddrPtr->EGF_eft.egrFilterConfigs.enCNFrameTxOnPort[n] =
                        0x0006280+0x4*n;
                }/* end of loop n */
            }/*0006280+0x4*n*/
            regsAddrPtr->EGF_eft.egrFilterConfigs.egrFiltersGlobal = 0x00006000;
            regsAddrPtr->EGF_eft.egrFilterConfigs.egrFiltersEnable = 0x00006004;

        }/*end of unit egrFilterConfigs */


        {/*start of unit deviceMapConfigs */
            {/*0003080+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    regsAddrPtr->EGF_eft.deviceMapConfigs.localSrcPortMapOwnDevEn[n] =
                        0x0003080+0x4*n;
                }/* end of loop n */
            }/*0003080+0x4*n*/
            {/*0003100+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    regsAddrPtr->EGF_eft.deviceMapConfigs.localTrgPortMapOwnDevEn[n] =
                        0x0003100+0x4*n;
                }/* end of loop n */
            }/*0003100+0x4*n*/
            {/*0003180+4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 127 ; n++) {
                    regsAddrPtr->EGF_eft.deviceMapConfigs.portAddrConstructMode[n] =
                        0x0003180+4*n;
                }/* end of loop n */
            }/*0003180+4*n*/
            {/*0003000+4*p*/
                GT_U32    p;
                for(p = 0 ; p <= 7 ; p++) {
                    regsAddrPtr->EGF_eft.deviceMapConfigs.devMapTableAddrConstructProfile[p] =
                        0x0003000+4*p;
                }/* end of loop p */
            }/*0003000+4*p*/

        }/*end of unit deviceMapConfigs */


        {/*start of unit cntrs */
            {/*0004010+4*n*/
                /*GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    regsAddrPtr->EGF_eft.counters.TXQ0MCFIFODropCntr[n] =
                        0x0004010+4*n;
                }*//* end of loop n */
            }/*0004010+4*n*/
            regsAddrPtr->EGF_eft.counters.eftDescEcc1ErrorCntr = 0x00004000;
            /*
            regsAddrPtr->EGF_eft.counters.descEccNonCriticalDropCounter = 0x00004004;
            regsAddrPtr->EGF_eft.counters.eftDescEcc1ErrorCntr = 0x00004008;
            */

        }/*end of unit cntrs */

        {/*start of unit egress MIB counters */
            GT_U32    n;
            for(n = 0 ; n <= 1 ; n++)
            {
                regsAddrPtr->EGF_eft.egrMIBCntrs.egrMIBCntrsPortSetConfig[n]       = 0x00006408 + (4 * n) ;
                regsAddrPtr->EGF_eft.egrMIBCntrs.egrMIBCntrsSetConfig[n]           = 0x00006400 + (4 * n);
                regsAddrPtr->EGF_eft.egrMIBCntrs.egrMIBMcFIFODroppedPktsCntr[n]    = 0x00006418 + (4 * n);
                regsAddrPtr->EGF_eft.egrMIBCntrs.egrMIBBridgeEgrFilteredPktCntr[n] = 0x00006410 + (4 * n);
                regsAddrPtr->EGF_eft.egrMIBCntrs.egrMIBMcFilteredPktCntr[n]        = 0x00006420 + (4 * n);
            }
        }/*end of unit egress MIB counters */

    }/*end of unit EGF_eft */

    {/*start of unit EGF_qag */
        {/*start of unit uniMultiArb */
            regsAddrPtr->EGF_qag.uniMultiArb.uniMultiCtrl = 0x000b0100;
            regsAddrPtr->EGF_qag.uniMultiArb.uniWeights = 0x000b0104;
            regsAddrPtr->EGF_qag.uniMultiArb.multiWeights = 0x000b0108;
            regsAddrPtr->EGF_qag.uniMultiArb.multiShaperCtrl = 0x000b010c;

        }/*end of unit uniMultiArb */


        {/*start of unit miscellaneous */
            /*regsAddrPtr->EGF_qag.miscellaneous.badAddrLatchReg = 0x000b0200;*/

        }/*end of unit miscellaneous */


        {/*start of unit global */
            regsAddrPtr->EGF_qag.global.QAGInterruptsCause = 0x000b0000;
            regsAddrPtr->EGF_qag.global.QAGInterruptsMask = 0x000b0010;

        }/*end of unit global */


        {/*start of unit distributor */
            {/*c9b0170+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    regsAddrPtr->EGF_qag.distributor.loopbackEnableConfig[n] =
                        0x000a0170+0x4*n;
                }/* end of loop n */
            }/*000a0170+0x4*n*/
            {/*000a0180+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 7 ; n++) {
                    regsAddrPtr->EGF_qag.distributor.toAnalyzerLoopback[n] =
                        0x000a0180+0x4*n;
                }/* end of loop n */
            }/*000a0180+0x4*n*/
            regsAddrPtr->EGF_qag.distributor.distributorGeneralConfigs = 0x000a0000;
            regsAddrPtr->EGF_qag.distributor.TCLoopbackConfig = 0x000a0004;
            regsAddrPtr->EGF_qag.distributor.eVIDXActivityStatusWriteMask = 0x000a0008;
            regsAddrPtr->EGF_qag.distributor.voQAssignmentGeneralConfig0 = 0x000a0010;
            regsAddrPtr->EGF_qag.distributor.pktHashForVoQAssignment0 = 0x000a0014;
            regsAddrPtr->EGF_qag.distributor.pktHashForVoQAssignment1 = 0x000a0018;
            regsAddrPtr->EGF_qag.distributor.voQAssignmentGeneralConfig1 = 0x000a0020;
            regsAddrPtr->EGF_qag.distributor.loopbackForwardingGlobalConfig = 0x000a0a00;
            regsAddrPtr->EGF_qag.distributor.cutThroughConfig = 0x000a0a04;

        }/*end of unit distributor */


    }/*end of unit EGF_qag */
}


/**
* @internal plrInit_sip6 function
* @endinternal
*
* @brief   sip 6 : init the DB - PLR unit
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] plrUnit                  - PLR stage number
*                                       None
*/
static void plrInit_sip6
(
    IN  GT_U8  devNum,
    IN  GT_U32 plrUnit
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

#if 0
    /* NOTE :  the next registers replaced with new table :
    CPSS_DXCH_SIP6_TABLE_INGRESS_POLICER_0_QOS_ATTRIBUTE_TABLE_E,
    CPSS_DXCH_SIP6_TABLE_INGRESS_POLICER_1_QOS_ATTRIBUTE_TABLE_E,
    CPSS_DXCH_SIP6_TABLE_EGRESS_POLICER_QOS_ATTRIBUTE_TABLE_E,
    */
    regsAddrPtr->PLR[plrUnit].policerInitialDP[n] =
    regsAddrPtr->PLR[plrUnit].qosProfile2PriorityMapArr[p] =
    /* NOTE :  the next registers replaced with new table :
    CPSS_DXCH_SIP6_TABLE_INGRESS_POLICER_0_PORT_ATTRIBUTE_TABLE_E,
    CPSS_DXCH_SIP6_TABLE_INGRESS_POLICER_1_PORT_ATTRIBUTE_TABLE_E,
    CPSS_DXCH_SIP6_TABLE_EGRESS_POLICER_PORT_ATTRIBUTE_TABLE_E,
     */
    regsAddrPtr->PLR[plrUnit].policerPortMeteringReg[n] =
    regsAddrPtr->PLR[plrUnit].portAndPktTypeTranslationTable[p/2] =


#endif

    {/*start of unit globalConfig */
        regsAddrPtr->PLR[plrUnit].globalConfig.statisticalCountingConfig0 = 0x00002010;
        regsAddrPtr->PLR[plrUnit].globalConfig.statisticalCountingConfig1 = 0x00002014;
        regsAddrPtr->PLR[plrUnit].globalConfig.meteringAnalyzerIndexes = 0x00000050;

        {/*0002040+4*t*/
            GT_U32    t;
            for(t = 0 ; t <= 6 ; t++) {
                regsAddrPtr->PLR[plrUnit].globalConfig.countingConfigTableThreshold[t] =
                    0x0002040+4*t;
            }/* end of loop t */
        }/*0002040+4*t*/
        {/*0002020+4*n*/
            GT_U32    n;
            for(n = 0 ; n <= 7 ; n++) {
                regsAddrPtr->PLR[plrUnit].globalConfig.countingConfigTableSegment[n] =
                    0x0002020+4*n;
            }/* end of loop n */
        }/*0002020+4*n*/
        regsAddrPtr->PLR[plrUnit].policerMeteringBaseAddr = 0x00000028;
        regsAddrPtr->PLR[plrUnit].policerCountingBaseAddr = 0x00000024;
        regsAddrPtr->PLR[plrUnit].policerCtrl1 = 0x00000004;
        regsAddrPtr->PLR[plrUnit].policerCtrl0 = 0x00000000;
        regsAddrPtr->PLR[plrUnit].policerMRU = 0x00000010;
        regsAddrPtr->PLR[plrUnit].meteringAddressingModeCfg0 = 0x00000040;
        regsAddrPtr->PLR[plrUnit].hierarchicalPolicerCtrl = 0x0000002c;
        regsAddrPtr->PLR[plrUnit].globalBillingCntrIndexingModeConfig1 = 0x0000000c;
        regsAddrPtr->PLR[plrUnit].globalBillingCntrIndexingModeConfig0 = 0x00000008;
        regsAddrPtr->PLR[plrUnit].meteringAddressingModeCfg1 = 0x00000044;
        regsAddrPtr->PLR[plrUnit].policerOAM = 0x00000224;

    }/*end of unit globalConfig */

    {/*start of unit IPFIX */
        regsAddrPtr->PLR[plrUnit].IPFIXCtrl = 0x00000014;
        regsAddrPtr->PLR[plrUnit].IPFIXNanoTimerStampUpload = 0x00000018;
        regsAddrPtr->PLR[plrUnit].IPFIXSecondsLSbTimerStampUpload = 0x0000001c;
        regsAddrPtr->PLR[plrUnit].IPFIXSecondsMSbTimerStampUpload = 0x00000020;
        regsAddrPtr->PLR[plrUnit].policerIPFIXDroppedPktCountWAThreshold = 0x00000030;
        regsAddrPtr->PLR[plrUnit].policerIPFIXPktCountWAThreshold = 0x00000034;
        regsAddrPtr->PLR[plrUnit].policerIPFIXByteCountWAThresholdLSB = 0x00000038;
        regsAddrPtr->PLR[plrUnit].policerIPFIXByteCountWAThresholdMSB = 0x0000003c;
        regsAddrPtr->PLR[plrUnit].IPFIXSampleEntriesLog0 = 0x00000048;
        regsAddrPtr->PLR[plrUnit].IPFIXSampleEntriesLog1 = 0x0000004c;

    }/*end of unit IPFIX */

    {/*start of unit Miscellaneous */
        regsAddrPtr->PLR[plrUnit].policerInterruptMask = 0x00000204;
        regsAddrPtr->PLR[plrUnit].policerInterruptCause = 0x00000200;
        regsAddrPtr->PLR[plrUnit].policerBadAddressLatch = 0x00002000;

    }/*end of unit Miscellaneous */

    {/*start of unit Indirect Table Access */
        {/*74+n*4*/
            GT_U32    n;
            for(n = 0 ; n <= 7 ; n++) {
                regsAddrPtr->PLR[plrUnit].policerTableAccessData[n] =
                    0x74+n*4;
            }/* end of loop n */
        }/*74+n*4*/
        regsAddrPtr->PLR[plrUnit].policerTableAccessCtrl = 0x00000070;

    }/*end of unit Indirect Table Access */

    {/*start of unit scan */
        regsAddrPtr->PLR[plrUnit].policerMeteringRefreshScanRateLimit = 0x000000b0;
        regsAddrPtr->PLR[plrUnit].policerMeteringRefreshScanAddrRange = 0x000000b4;
        regsAddrPtr->PLR[plrUnit].policerScanCtrl = 0x000000b8;

    }/*end of unit scan */

    {/*2100+(p*4)*/
        GT_U32    p;
        for (p = 0 ; (p <= 127); p++) {
            regsAddrPtr->PLR[plrUnit].mef10_3_bucket0MaxRateArr[p] =
                0x2100 + (p * 4);
        }/* end of loop p */
    }/*2100+(p*4)*/
    {/*2300+(p*4)*/
        GT_U32    p;
        for (p = 0 ; (p <= 127); p++) {
            regsAddrPtr->PLR[plrUnit].mef10_3_bucket1MaxRateArr[p] =
                0x2300 + (p * 4);
        }/* end of loop p */
    }/*2300+(p*4)*/

    {/*3550+(p*4)*/
        GT_U32    p;
        for (p = 0 ; (p <= 1); p++) {
            regsAddrPtr->PLR[plrUnit].debug_LFSRSeed[p] =
                0x3550 + (p * 4);
        }
    }/*3550+(p*4)*/
    regsAddrPtr->PLR[plrUnit].debug_PrngControl1 =  0x00003560;
    regsAddrPtr->PLR[plrUnit].debug_PrngControl2 =  0x00003564;

    regsAddrPtr->PLR[plrUnit].policerCountingTbl   = 0x000C0000;
    regsAddrPtr->PLR[plrUnit].policerTimerTbl      = 0x00000300;
    regsAddrPtr->PLR[plrUnit].ipfixWrapAroundAlertTbl = 0x00000800;
    regsAddrPtr->PLR[plrUnit].ipfixAgingAlertTbl   = 0x00001000;

}


void txqSdqInit_sip6
(
  IN  GT_U8 devNum,
  IN GT_U32 sdqUnit
)
{
        PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqUnit].global_config = 0x00000004;    /*REGISTER: Global config */
        PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqUnit].SDQ_Metal_Fix = 0x00000000;    /*REGISTER: SDQ Metal Fix */
        PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqUnit].SDQ_Interrupt_Functional_Cause = 0x00000208;    /*REGISTER: SDQ Interrupt Functional Cause */
        PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqUnit].sdq_interrupt_functional_mask = 0x0000020c;    /*REGISTER: SDQ Interrupt Functional Mask */
        PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqUnit].SDQ_Interrupt_Summary_Mask = 0x00000204;    /*REGISTER: SDQ Interrupt Summary Mask */
        PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqUnit].SDQ_Interrupt_Summary_Cause = 0x00000200;    /*REGISTER: SDQ Interrupt Summary Cause */
        PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqUnit].SDQ_Interrupt_Debug_Mask = 0x00000214;    /*REGISTER: SDQ Interrupt Debug Mask */
        PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqUnit].SDQ_Interrupt_Debug_Cause = 0x00000210;    /*REGISTER: SDQ Interrupt Debug Cause */
        PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqUnit].QCN_Config = 0x00000500;    /*REGISTER: QCN Config */
        PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqUnit].Sdq_Idle = 0x00006000;    /*REGISTER: SDQ IDLE */
        PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqUnit].Illegal_Bad_Address = 0x00006004;    /*REGISTER: Illegal Bad Address */
        PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqUnit].QCN_Message_Drop_Counter = 0x00006008;    /*REGISTER: QCN Message Drop Counter */
        PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqUnit].Queue_Dryer_FIFO_Max_Peak = 0x00006014;    /*REGISTER: Queue Dryer FIFO Max Peak */
        PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqUnit].PDQ_Message_Output_FIFO_Max_Peak = 0x00006018;    /*REGISTER: PDQ Message Output FIFO Max Peak */
        PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqUnit].QCN_Message_Drop_Counter_Control = 0x0000600c;    /*REGISTER: QCN Message Drop Counter Control */
        PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqUnit].QCN_Message_FIFO_Max_Peak = 0x0000601c;    /*REGISTER: QCN Message FIFO Max Peak */
        PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqUnit].QCN_Message_FIFO_Fill_Level = 0x00006010;    /*REGISTER: QCN Message FIFO Fill Level */
        PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqUnit].Elig_Func_Error_Capture = 0x00006020;    /*REGISTER: Elig Func Error Capture */
        PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqUnit].Select_Func_Error_Capture = 0x00006024;    /*REGISTER: Select Func Error Capture */
}




static void txqPdsInit_sip6
(
    IN  GT_U8 devNum,
    IN  GT_U32 pdsUnit
)
{
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsUnit].Max_PDS_size_limit_for_pdx = 0x00031008;    /*REGISTER: Max PDS Size Limit for PDX */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsUnit].PDS_Metal_Fix = 0x00031000;    /*REGISTER: PDS Metal Fix */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsUnit].Global_PDS_CFG = 0x00031004;    /*REGISTER: Global PDS CFG */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsUnit].FIFOs_Limits = 0x00031028;    /*REGISTER: FIFOs Limits */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsUnit].FIFOs_DQ_Disable = 0x0003102c;    /*REGISTER: FIFOs DQ Disable */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsUnit].Max_PDS_size_limit_for_PB = 0x0003100c;    /*REGISTER: Max PDS Size Limit for PB */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsUnit].Max_Num_Of_Long_Queues = 0x00031010;    /*REGISTER: Max Num Of Long Queues */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsUnit].Tail_Size_for_PB_Wr = 0x00031014;    /*REGISTER: Tail Size for PB Wr */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsUnit].PID_Empty_Limit_for_PDX = 0x00031018;    /*REGISTER: PID Empty Limit for PDX */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsUnit].PID_Empty_Limit_for_PB = 0x0003101c;    /*REGISTER: PID Empty Limit for PB */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsUnit].NEXT_Empty_Limit_for_PDX = 0x00031020;    /*REGISTER: NEXT Empty Limit for PDX */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsUnit].NEXT_Empty_Limit_for_PB = 0x00031024;    /*REGISTER: NEXT Empty Limit for PB */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsUnit].PB_Full_Limit = 0x00031030;    /*REGISTER: PB Full Limit */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsUnit].PDS_Interrupt_Debug_Cause = 0x00032010;    /*REGISTER: PDS Interrupt Debug Cause */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsUnit].PDS_Interrupt_Debug_Mask = 0x00032014;    /*REGISTER: PDS Interrupt Debug Mask */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsUnit].PDS_Last_Address_Violation = 0x00032018;    /*REGISTER: PDS Last Address Violation */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsUnit].PDS_Interrupt_Summary_Mask = 0x00032004;    /*REGISTER: PDS Interrupt Summary Mask */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsUnit].PDS_Interrupt_Summary_Cause = 0x00032000;    /*REGISTER: PDS Interrupt Summary Cause */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsUnit].PDS_Interrupt_Functional_Mask = 0x0003200c;    /*REGISTER: PDS Interrupt Functional Mask */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsUnit].PDS_Interrupt_Functional_Cause = 0x00032008;    /*REGISTER: PDS Interrupt Functional Cause */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsUnit].Total_PDS_Counter = 0x00033004;    /*REGISTER: Total PDS Counter */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsUnit].PDS_Cache_Counter = 0x00033008;    /*REGISTER: PDS Cache Counter */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsUnit].Idle_Register = 0x00033000;    /*REGISTER: Idle Register */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsUnit].Free_Next_FIFO_Max_Peak = 0x00034108;    /*REGISTER: Free Next FIFO Max Peak */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsUnit].Free_Next_FIFO_Min_Peak = 0x0003410c;    /*REGISTER: Free Next FIFO Min Peak */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsUnit].Free_PID_FIFO_Max_Peak = 0x00034110;    /*REGISTER: Free PID FIFO Max Peak */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsUnit].Free_PID_FIFO_Min_Peak = 0x00034114;    /*REGISTER: Free PID FIFO Min Peak */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsUnit].Data_Read_Order_FIFO_Max_Peak = 0x00034118;    /*REGISTER: Data Read Order FIFO Max Peak */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsUnit].Head_Answer_FIFO_Max_Peak = 0x00034120;    /*REGISTER: Head Answer FIFO Max Peak */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsUnit].PB_Read_Line_Buff_FIFO_Max_Peak = 0x00034128;    /*REGISTER: PB Read Line Buff FIFO Max Peak */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsUnit].PB_Read_Req_FIFO_Max_Peak = 0x00034130;    /*REGISTER: PB Read Req FIFO Max Peak */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsUnit].PB_Write_Req_FIFO_Max_Peak = 0x0003413c;    /*REGISTER: PB Write Req FIFO Max Peak */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsUnit].PB_Write_Reply_FIFO_Max_Peak = 0x00034144;    /*REGISTER: PB Write Reply FIFO Max Peak */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsUnit].PB_Read_FIFO_State_Return_FIFO_Max_Peak = 0x0003414c;    /*REGISTER: PB Read FIFO State Return FIFO Max Peak */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsUnit].Tail_Ans_FIFO_Max_Peak = 0x00034158;    /*REGISTER: Tail Ans FIFO Max Peak */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsUnit].PB_Wr_Line_Buff_Ctrl_FIFO_Max_Peak = 0x00034164;    /*REGISTER: PB Wr Line Buff Ctrl FIFO Max Peak */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsUnit].PB_Wr_Line_Buff_Data_FIFO_Max_Peak = 0x0003416c;    /*REGISTER: PB Wr Line Buff Data FIFO Max Peak */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsUnit].PB_FIFO_State_Read_Order_FIFO_Max_Peak = 0x00034174;    /*REGISTER: PB FIFO State Read Order FIFO Max Peak */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsUnit].Frag_On_The_Air_Cntr_Queue_Status = 0x00034200;    /*REGISTER: Frag On The Air Cntr Queue Status */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsUnit].Datas_BMX_Addr_Out_Of_Range_Port = 0x00034204;    /*REGISTER: Datas BMX Addr Out Of Range Port */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsUnit].PB_Write_FIFO_State_Return_FIFO_Max_Peak = 0x00034150;    /*REGISTER: PB Write FIFO State Return FIFO Max Peak */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsUnit].PB_Read_Write_Order_FIFO_Max_Peak = 0x00034154;    /*REGISTER: PB Read Write Order FIFO Max Peak */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsUnit].FIFO_State_Latency_Max_Peak = 0x00034178;    /*REGISTER: FIFO_State Latency Max Peak */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsUnit].FIFO_State_Latency_Min_Peak = 0x0003417c;    /*REGISTER: FIFO_State Latency Min Peak */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsUnit].Long_Queue_Counter = 0x0003300c;    /*REGISTER: Long Queue Counter */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsUnit].Long_Queue_Count_Max_Peak = 0x00034100;    /*REGISTER: Long Queue Count Max Peak */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsUnit].PDS_Cache_Count_Max_Peak = 0x000340fc;    /*REGISTER: PDS Cache Count Max Peak */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsUnit].Total_PDS_Count_Max_Peak = 0x000340f8;    /*REGISTER: Total PDS Count Max Peak */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsUnit].Active_Queue_Counter = 0x00033010;    /*REGISTER: Active Queue Counter */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsUnit].Active_Queue_Count_Max_Peak = 0x00034104;    /*REGISTER: Active Queue Count Max Peak */
}




static void txqPdxInit_sip6(IN  GT_U8 devNum)
{
    GT_U32 i;


        PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pdx.pdx_metal_fix = 0x0001f004;    /*REGISTER: Metal Fix */
        PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pdx.DB_Read_Control = 0x00004000;    /*REGISTER: QCX Read Control */
        PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pdx.QCX_Read_qu_db = 0x00004004;    /*REGISTER: QCX Read  Qu DB */
        PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pdx.QCX_Read_Port_DB = 0x00004008;    /*REGISTER: QCX Read  Port DB */
        PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pdx.QCX_Read_Reorder_Fifos = 0x0000400c;    /*REGISTER: QCX Read  Reorder Fifos */
        PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pdx.pdx2pdx_interface_mapping = 0x00010000;    /*REGISTER: pdx2pdx interface mapping */
        PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pdx.Interrupt_Cause = 0x0001f010;    /*REGISTER: Interrupt Cause */
        PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pdx.Interrupt_Mask = 0x0001f014;    /*REGISTER: Interrupt Mask */
        PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pdx.Address_Violation = 0x0001f018;    /*REGISTER: Address Violation */
        PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pdx.pfcc_burst_fifo_thr = 0x0001800c;    /*REGISTER: PFCC Burst Fifo Threshold */
        PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pdx.global_burst_fifo_thr = 0x00018008;    /*REGISTER: Global Burst Fifo Threshold */
        PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pdx.micro_Memory_Traffic_Counter = 0x00018010;    /*REGISTER: Micro Memory Traffic Counter */
        PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pdx.global_burst_fifo_available_entries = 0x00018000;    /*REGISTER: Global Burst Fifo Available Entries */
        PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pdx.max_bank_fill_levels = 0x00018030;    /*REGISTER: Max Bank fill levels */
        PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pdx.drop_Counter = 0x00018014;    /*REGISTER: Drop Counter */
        PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pdx.Drop_Counter_Mask = 0x00018018;    /*REGISTER: Drop Counter Mask */
        PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pdx.MIN_Global_Burst_Fifo_Available_Entries = 0x00018004;    /*REGISTER: MIN Global Burst Fifo Available Entries */


    for(i=0;i<2;i++)
    {
        PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pdx_pac[i].pdx_pac_metal_fix = 0x00001f04;    /*REGISTER: PDX PAC metal fix */
        PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pdx_pac[i].Interrupt_Cause = 0x00001f10;    /*REGISTER: Interrupt Cause */
        PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pdx_pac[i].Interrupt_Mask = 0x00001f14;    /*REGISTER: Interrupt Mask */
        PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pdx_pac[i].Address_Violation = 0x00001f18;    /*REGISTER: Address Violation */
    }

}

static void txqPfccInit_sip6(IN  GT_U8 devNum)
{
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pfcc.global_pfcc_cfg = 0x00000000;    /*REGISTER: Global pfcc CFG */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pfcc.pool_CFG = 0x00000004;    /*REGISTER: pool CFG */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pfcc.PFCC_metal_fix = 0x0000000c;    /*REGISTER: PFCC metal fix */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pfcc.TC_to_pool_CFG = 0x00000008;    /*REGISTER: TC to pool CFG */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pfcc.PFCC_Interrupt_Cause = 0x00000100;    /*REGISTER: PFCC Interrupt Cause */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pfcc.PFCC_Interrupt_Mask = 0x00000104;    /*REGISTER: PFCC Interrupt Mask */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pfcc.Address_Violation = 0x00000108;    /*REGISTER: Address Violation */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pfcc.PFCC_Cycle_Counter = 0x00000210;    /*REGISTER: PFCC Cycle Counter */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pfcc.Aggregation_Request_Gen_SM_State = 0x00000214;    /*REGISTER: Aggregation Request Gen SM State */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pfcc.Source_Port_Requested_For_Read = 0x00000400;    /*REGISTER: Source Port Requested For Read */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pfcc.Source_Port_Counter = 0x00000404;    /*REGISTER: Source Port Counter Value */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pfcc.pb_occupancy_status = 0x00000540;    /*REGISTER: pb_occupancy_status */
}
static void txqPsiInit_sip6(IN  GT_U8 devNum)
{
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.psi_regs.Credit_Value = 0x00000130;    /*REGISTER: Credit Value */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.psi_regs.PSI_Interrupt_Cause = 0x00000134;    /*REGISTER: PSI_Interrupt_Cause */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.psi_regs.PSI_Interrupt_Mask = 0x00000138;    /*REGISTER: PSI_Interrupt_Mask */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.psi_regs.PSI_Last_Address_Violation = 0x0000013c;    /*REGISTER: PSI Last Address Violation */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.psi_regs.PSI_Metal_Fix_Register = 0x00000000;    /*REGISTER: PSI Metal Fix Register */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.psi_regs.QmrSchPLenBursts = 0x00000124;    /*REGISTER: QmrSchPLenBursts */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.psi_regs.QmrSchBurstsSent = 0x00000128;    /*REGISTER: QmrSchBurstsSent */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.psi_regs.MSG_FIFO_Max_Peak = 0x00000140;    /*REGISTER: MSG FIFO Max Peak */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.psi_regs.MSG_FIFO_Min_Peak = 0x00000144;    /*REGISTER: MSG FIFO Min Peak */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.psi_regs.MSG_FIFO_Fill_Level = 0x00000148;    /*REGISTER: MSG FIFO Fill Level */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.psi_regs.Queue_Status_Read_Request = 0x0000014c;    /*REGISTER: Queue Status Read Request */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.psi_regs.Queue_Status_Read_Reply = 0x00000150;    /*REGISTER: Queue Status Read Reply */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.psi_regs.Debug_CFG_Register = 0x00000154;    /*REGISTER: Debug CFG Register */
}


static void txqQfcInit_sip6
(
  IN  GT_U8 devNum,
  IN GT_U32 qfcUnit
)
{
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcUnit].global_pb_limit = 0x00002400;    /*REGISTER: Global PB Limit */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcUnit].QFC_Interrupt_functional_Cause = 0x00000100;    /*REGISTER: QFC Interrupt Functional Cause */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcUnit].QFC_Interrupt_functional_Mask = 0x00000104;    /*REGISTER: QFC Interrupt Functional Mask */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcUnit].Address_Violation = 0x00000118;    /*REGISTER: Address Violation */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcUnit].qfc_interrupt_debug_cause = 0x00000108;    /*REGISTER: QFC Interrupt Debug Cause */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcUnit].QFC_Interrupt_Debug_Mask = 0x0000010c;    /*REGISTER: QFC Interrupt Debug Mask */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcUnit].QFC_Interrupt_Summary_Cause = 0x00000110;    /*REGISTER: QFC Interrupt Summary Cause */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcUnit].QFC_Interrupt_Summary_Mask = 0x00000114;    /*REGISTER: QFC Interrupt Summary Mask */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcUnit].qfc_metal_fix = 0x00000000;    /*REGISTER: qfc metal fix */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcUnit].Global_PFC_conf = 0x00000004;    /*REGISTER: Global PFC conf */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcUnit].PFC_messages_Drop_counter = 0x00000300;    /*REGISTER: PFC messages Drop counter */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcUnit].global_PFC_status = 0x00000600;    /*REGISTER: global PFC status */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcUnit].Uburst_Event_FIFO_Max_Peak = 0x00000604;    /*REGISTER: Uburst Event FIFO Max Peak */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcUnit].Uburst_Event_FIFO_Min_Peak = 0x00000608;    /*REGISTER: Uburst Event FIFO Min Peak */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcUnit].PFC_Messages_counter = 0x0000060c;    /*REGISTER: PFC Messages counter */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcUnit].Xoff_Size_Indirect_Read_Access = 0x00000610;    /*REGISTER: Xoff Size Indirect Read Access */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcUnit].HR_Counters_Indirect_Read_Access = 0x00003000;    /*REGISTER: HR Counters Indirect Read Access */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcUnit].Port_TC_HR_Counter_Min_Max_Conf = 0x00003100;    /*REGISTER: Port TC HR Counter Min Max Conf */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcUnit].HR_Counter_Max_Peak = 0x00003104;    /*REGISTER: HR Counter Max Peak */
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcUnit].HR_Counter_Min_Peak = 0x00003108;    /*REGISTER: HR Counter Min Peak */

    /*Added manually*/

    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcUnit].Counter_Table_0_Indirect_Read_Data = 0x00000700;
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcUnit].Counter_Table_0_Indirect_Read_Address = 0x00000704;
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcUnit].Counter_Table_1_Indirect_Read_Data = 0x00000708;
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcUnit].Counter_Table_1_Indirect_Read_Address = 0x0000070C;
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcUnit].Counter_Table_2_Indirect_Read_Data = 0x00000710;
    PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[qfcUnit].Counter_Table_2_Indirect_Read_Address = 0x00000714;


}


/**
* @internal rxdmaInit_sip6 function
* @endinternal
*
* @brief   sip6 : init the DB - RXDMA unit (per unit index)
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] index                    - unit index
*                                       None
*/
static void rxdmaInit_sip6
(
    IN  GT_U8 devNum,
    IN  GT_U32  index,
    IN  GT_U32  lastRxDmaIndex,
    IN CPSS_PORTS_BMP_STC *validRxDmaBmpPtr
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
#ifdef GM_USED
    GT_U32 gmUsed = 1;
#else
    GT_U32 gmUsed = 0;
#endif

    if((PRV_CPSS_DXCH_FALCON_CIDER_VERSION_17_11_11_E  >= prvCpssDxchFalconCiderVersionGet())|| /* gm*/(1 == gmUsed))
    {
        { /*start of unit sip6_rxDMA[] */
            {/*start of unit status */
                regsAddrPtr->sip6_rxDMA[index].status.pktDropReport = 0x00002050;

            }/*end of unit status */


            {/*start of unit configs */
                {/*start of unit rejectConfigs */
                    regsAddrPtr->sip6_rxDMA[index].configs.rejectConfigs.rejectPktCommand = 0x000007a8;
                    regsAddrPtr->sip6_rxDMA[index].configs.rejectConfigs.rejectCPUCode = 0x000007ac;

                }/*end of unit rejectConfigs */


                {/*start of unit preIngrPrioritizationConfStatus */
                    {/*00007d0+n * 0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 3 ; n++) {
                            regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.VLANUPPriorityArrayProfile[n] =
                                0x00007d0+n * 0x4;
                        }/* end of loop n */
                    }/*00007d0+n * 0x4*/
                    {/*0000aa4+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 3 ; n++) {
                            regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.VLANEtherTypeConf[n] =
                                0x0000aa4+n*0x4;
                        }/* end of loop n */
                    }/*0000aa4+n*0x4*/
                    {/*0000b5c+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 3 ; n++) {
                            regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.VIDConf[n] =
                                0x0000b5c+n*0x4;
                        }/* end of loop n */
                    }/*0000b5c+n*0x4*/
                    {/*0000d3c+n * 0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 3 ; n++) {
                            regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.UPDSATagArrayProfile[n] =
                                0x0000d3c+n * 0x4;
                        }/* end of loop n */
                    }/*0000d3c+n * 0x4*/
                    {/*00007f0+m*0x4 + n*0x20*/
                        GT_U32    n,m;
                        for(n = 0 ; n <= 3 ; n++) {
                            for(m = 0 ; m <= 7 ; m++) {
                                regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.qoSDSAPriorityArrayProfileSegment[n][m] =
                                    0x00007f0+m*0x4 + n*0x20;
                            }/* end of loop m */
                        }/* end of loop n */
                    }/*00007f0+m*0x4 + n*0x20*/
                    {/*0000e74+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 3 ; n++) {
                            regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.profileTPID[n] =
                                0x0000e74+n*0x4;
                        }/* end of loop n */
                    }/*0000e74+n*0x4*/
                    regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.preingressPrioritizationEnable = 0x000007c0;
                    regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.PIPVeryHighPriorityDropGlobalCntrMsb = 0x00000e70;
                    regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.PIPVeryHighPriorityDropGlobalCntrLsb = 0x00000e6c;
                    regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.PIPPriorityDropGlobalCntrsClear = 0x00000e4c;
                    regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.PIPMedPriorityDropGlobalCntrMsb = 0x00000e30;
                    regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.PIPMedPriorityDropGlobalCntrLsb = 0x00000e2c;
                    regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.PIPLowPriorityDropGlobalCntrMsb = 0x00000e20;
                    regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.PIPLowPriorityDropGlobalCntrLSb = 0x00000e1c;
                    regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.PIPHighPriorityDropGlobalCntrMsb = 0x00000e40;
                    regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.PIPHighPriorityDropGlobalCntrLsb = 0x00000e3c;
                    {/*0000f74+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 3 ; n++) {
                            regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.OSTPIDsConfReg[n] =
                                0x0000f74+n*0x4;
                        }/* end of loop n */
                    }/*0000f74+n*0x4*/
                    {/*0000880+n * 0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 3 ; n++) {
                            regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.MPLSEXPPriorityArrayProfile[n] =
                                0x0000880+n * 0x4;
                        }/* end of loop n */
                    }/*0000880+n * 0x4*/
                    {/*0000ab4+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 1 ; n++) {
                            regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.MPLSEtherTypeConf[n] =
                                0x0000ab4+n*0x4;
                        }/* end of loop n */
                    }/*0000ab4+n*0x4*/
                    {/*00009fc+n * 0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 3 ; n++) {
                            regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.MACDAPriorityArrayProfile[n] =
                                0x00009fc+n * 0x4;
                        }/* end of loop n */
                    }/*00009fc+n * 0x4*/
                    {/*0000b34+n *0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 3 ; n++) {
                            regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.MACDAMask4LsbConf[n] =
                                0x0000b34+n *0x4;
                        }/* end of loop n */
                    }/*0000b34+n *0x4*/
                    {/*0000b48+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 3 ; n++) {
                            regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.MACDAMask2MsbConf[n] =
                                0x0000b48+n*0x4;
                        }/* end of loop n */
                    }/*0000b48+n*0x4*/
                    {/*0000af4+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 3 ; n++) {
                            regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.MACDA4LsbBytesConf[n] =
                                0x0000af4+n*0x4;
                        }/* end of loop n */
                    }/*0000af4+n*0x4*/
                    {/*0000b14+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 3 ; n++) {
                            regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.MACDA2MsbBytesConf[n] =
                                0x0000b14+n*0x4;
                        }/* end of loop n */
                    }/*0000b14+n*0x4*/
                    {/*0001174+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 3 ; n++) {
                            regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.latencySensitiveEtherTypeConf[n] =
                                0x0001174+n*0x4;
                        }/* end of loop n */
                    }/*0001174+n*0x4*/
                    {/*0000bdc+m*0x4 + n*0x40*/
                        GT_U32    n,m;
                        for(n = 0 ; n <= 3 ; n++) {
                            for(m = 0 ; m <= 15 ; m++) {
                                regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.IPv6TCPriorityArrayProfileSegment[n][m] =
                                    0x0000bdc+m*0x4 + n*0x40;
                            }/* end of loop m */
                        }/* end of loop n */
                    }/*0000bdc+m*0x4 + n*0x40*/
                    regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.IPv6EtherTypeConf = 0x00000a80;
                    {/*00008a4+m*0x4 + n*0x40*/
                        GT_U32    n,m;
                        for(n = 0 ; n <= 3 ; n++) {
                            for(m = 0 ; m <= 15 ; m++) {
                                regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.IPv4ToSPriorityArrayProfileSegment[n][m] =
                                    0x00008a4+m*0x4 + n*0x40;
                            }/* end of loop m */
                        }/* end of loop n */
                    }/*00008a4+m*0x4 + n*0x40*/
                    regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.IPv4EtherTypeConf = 0x00000a5c;
                    {/*0000ad4+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 3 ; n++) {
                            regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.genericEtherType[n] =
                                0x0000ad4+n*0x4;
                        }/* end of loop n */
                    }/*0000ad4+n*0x4*/
                    {/*0000a2c+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 3 ; n++) {
                            regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.etherTypePriorityArrayProfile[n] =
                                0x0000a2c+n*0x4;
                        }/* end of loop n */
                    }/*0000a2c+n*0x4*/
                    regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.PIPDropGlobalCounterTrigger = 0x00000e44;
                    {/*0001074+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= lastRxDmaIndex/*8*/ ; n++) { /* manually fixed from : for(n = 0 ; n <= 63 ; n++) { */
                            SKIP_NON_EXISTS_PORT_IN_BMP(validRxDmaBmpPtr,n);
                            regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.channelPIPConfigReg[n] =
                                0x0001074+n*0x4;
                        }/* end of loop n */
                    }/*0001074+n*0x4*/

                }/*end of unit preIngrPrioritizationConfStatus */


                {/*start of unit dropThresholds */
                    regsAddrPtr->sip6_rxDMA[index].configs.dropThresholds.lowPrioDropThreshold0 = 0x00000014;
                    regsAddrPtr->sip6_rxDMA[index].configs.dropThresholds.lowPrioDropThreshold1 = 0x00000018;
                    regsAddrPtr->sip6_rxDMA[index].configs.dropThresholds.lowPrioDropMapConfig = 0x0000001c;

                }/*end of unit dropThresholds */


                {/*start of unit dataAccumulator */
                    regsAddrPtr->sip6_rxDMA[index].configs.dataAccumulator.PBFillLevelThresholds = 0x00000000;
                    regsAddrPtr->sip6_rxDMA[index].configs.dataAccumulator.maxTailBC = 0x00000004;

                }/*end of unit dataAccumulator */


                {/*start of unit cutThrough */
                    {/*0000038+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 3 ; n++) {
                            regsAddrPtr->sip6_rxDMA[index].configs.cutThrough.CTGenericConfigs[n] =
                                0x0000038+n*0x4;
                        }/* end of loop n */
                    }/*0000038+n*0x4*/
                    {/*0000050+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= lastRxDmaIndex/*8*/ ; n++) {
                            SKIP_NON_EXISTS_PORT_IN_BMP(validRxDmaBmpPtr,n);
                            regsAddrPtr->sip6_rxDMA[index].configs.cutThrough.channelCTConfig[n] =
                                0x0000050+n*0x4;
                        }/* end of loop n */
                    }/*0000050+n*0x4*/
                    regsAddrPtr->sip6_rxDMA[index].configs.cutThrough.CTGeneralConfig = 0x0000002c;
                    regsAddrPtr->sip6_rxDMA[index].configs.cutThrough.CTErrorConfig = 0x00000030;
                    regsAddrPtr->sip6_rxDMA[index].configs.cutThrough.CTUPEnable = 0x00000034;

                }/*end of unit cutThrough */


                {/*start of unit channelConfig */
                    {/*00004a8+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= lastRxDmaIndex/*8*/ ; n++) {
                            SKIP_NON_EXISTS_PORT_IN_BMP(validRxDmaBmpPtr,n);
                            regsAddrPtr->sip6_rxDMA[index].configs.channelConfig.channelToLocalDevSourcePort[n] =
                                0x00004a8+n*0x4;
                        }/* end of loop n */
                    }/*00004a8+n*0x4*/
                    {/*00003a8+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= lastRxDmaIndex/*8*/ ; n++) {
                            SKIP_NON_EXISTS_PORT_IN_BMP(validRxDmaBmpPtr,n);
                            regsAddrPtr->sip6_rxDMA[index].configs.channelConfig.PCHConfig[n] =
                                0x00003a8+n*0x4;
                        }/* end of loop n */
                    }/*00003a8+n*0x4*/
                    {/*00005a8+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= lastRxDmaIndex/*8*/ ; n++) {
                            SKIP_NON_EXISTS_PORT_IN_BMP(validRxDmaBmpPtr,n);
                            regsAddrPtr->sip6_rxDMA[index].configs.channelConfig.channelGeneralConfigs[n] =
                                0x00005a8+n*0x4;
                        }/* end of loop n */
                    }/*00005a8+n*0x4*/
                    {/*00002a8+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= lastRxDmaIndex/*8*/ ; n++) {
                            SKIP_NON_EXISTS_PORT_IN_BMP(validRxDmaBmpPtr,n);
                            regsAddrPtr->sip6_rxDMA[index].configs.channelConfig.channelCascadePort[n] =
                                0x00002a8+n*0x4;
                        }/* end of loop n */
                    }/*00002a8+n*0x4*/

                }/*end of unit channelConfig */


            }/*end of unit configs */


            {/*start of unit DFX */
                regsAddrPtr->sip6_rxDMA[index].DFX.ECCErrorInjection = 0x000020f4;
                regsAddrPtr->sip6_rxDMA[index].DFX.ECCErrorMode = 0x000020f8;

            }/*end of unit DFX */


            {/*start of unit debug */
                regsAddrPtr->sip6_rxDMA[index].debug.headerMemoryFullDrop0 = 0x00000200;
                regsAddrPtr->sip6_rxDMA[index].debug.headerMemoryFullDrop1 = 0x00000204;

            }/*end of unit debug */
        }/*end of unit sip6_rxDMA[] */
    }
    else
    {
        { /*start of unit sip6_rxDMA[] */

            {/*start of unit configs */
                {/*start of unit rejectConfigs */
                    regsAddrPtr->sip6_rxDMA[index].configs.rejectConfigs.rejectPktCommand = 0x00001000;
                    regsAddrPtr->sip6_rxDMA[index].configs.rejectConfigs.rejectCPUCode = 0x00001004;

                }/*end of unit rejectConfigs */

                {/*start of unit preIngrPrioritizationConfStatus */
                    {/*00007d0+n * 0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 3 ; n++) {
                            regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.VLANUPPriorityArrayProfile[n] =
                                0x00001080+n * 0x4;
                        }/* end of loop n */
                    }/*00007d0+n * 0x4*/
                    {/*0000b5c+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 3 ; n++) {
                            regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.VIDConf[n] =
                                0x000012b0+n*0x4;
                        }/* end of loop n */
                    }/*0000b5c+n*0x4*/
                    {/*0000d3c+n * 0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 3 ; n++) {
                            regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.UPDSATagArrayProfile[n] =
                                0x000013c0+n * 0x4;
                        }/* end of loop n */
                    }/*0000d3c+n * 0x4*/
                    {/*00007f0+m*0x4 + n*0x20*/
                        GT_U32    n,m;
                        for(n = 0 ; n <= 3 ; n++) {
                            for(m = 0 ; m <= 7 ; m++) {
                                regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.qoSDSAPriorityArrayProfileSegment[n][m] =
                                    0x00001090+m*0x4 + n*0x20;
                            }/* end of loop m */
                        }/* end of loop n */
                    }/*00007f0+m*0x4 + n*0x20*/
                    {/*0000e74+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 3 ; n++) {
                            regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.profileTPID[n] =
                                0x000013d0+n*0x4;
                        }/* end of loop n */
                    }/*0000e74+n*0x4*/
                    regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.preingressPrioritizationEnable = 0x000001c08;
                   {/*0000f74+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 3 ; n++) {
                            regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.OSTPIDsConfReg[n] =
                                0x000013e0+n*0x4;
                        }/* end of loop n */
                    }/*0000f74+n*0x4*/
                    {/*0000880+n * 0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 3 ; n++) {
                            regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.MPLSEXPPriorityArrayProfile[n] =
                                0x00001110+n * 0x4;
                        }/* end of loop n */
                    }/*0000880+n * 0x4*/
                    {/*0000ab4+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 1 ; n++) {
                            regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.MPLSEtherTypeConf[n] =
                                0x00001258+n*0x4;
                        }/* end of loop n */
                    }/*0000ab4+n*0x4*/
                    {/*00009fc+n * 0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 3 ; n++) {
                            regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.MACDAPriorityArrayProfile[n] =
                                0x00001220+n * 0x4;
                        }/* end of loop n */
                    }/*00009fc+n * 0x4*/
                    {/*0000b34+n *0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 3 ; n++) {
                            regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.MACDAMask4LsbConf[n] =
                                0x00001290+n *0x4;
                        }/* end of loop n */
                    }/*0000b34+n *0x4*/
                    {/*0000b48+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 3 ; n++) {
                            regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.MACDAMask2MsbConf[n] =
                                0x000012a0+n*0x4;
                        }/* end of loop n */
                    }/*0000b48+n*0x4*/
                    {/*0000af4+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 3 ; n++) {
                            regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.MACDA4LsbBytesConf[n] =
                                0x00001270+n*0x4;
                        }/* end of loop n */
                    }/*0000af4+n*0x4*/
                    {/*0000b14+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 3 ; n++) {
                            regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.MACDA2MsbBytesConf[n] =
                                0x00001280+n*0x4;
                        }/* end of loop n */
                    }/*0000b14+n*0x4*/
                    {/*0001174+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 3 ; n++) {
                            regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.latencySensitiveEtherTypeConf[n] =
                                0x000014f0+n*0x4;
                        }/* end of loop n */
                    }/*0001174+n*0x4*/
                    {/*0000bdc+m*0x4 + n*0x40*/
                        GT_U32    n,m;
                        for(n = 0 ; n <= 3 ; n++) {
                            for(m = 0 ; m <= 15 ; m++) {
                                regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.IPv6TCPriorityArrayProfileSegment[n][m] =
                                    0x000012c0+m*0x4 + n*0x40;
                            }/* end of loop m */
                        }/* end of loop n */
                    }/*0000bdc+m*0x4 + n*0x40*/
                    regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.IPv6EtherTypeConf = 0x00001244;
                    {/*00008a4+m*0x4 + n*0x40*/
                        GT_U32    n,m;
                        for(n = 0 ; n <= 3 ; n++) {
                            for(m = 0 ; m <= 15 ; m++) {
                                regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.IPv4ToSPriorityArrayProfileSegment[n][m] =
                                    0x00001120+m*0x4 + n*0x40;
                            }/* end of loop m */
                        }/* end of loop n */
                    }/*00008a4+m*0x4 + n*0x40*/
                    regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.IPv4EtherTypeConf = 0x00001240;
                    {/*0000ad4+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 3 ; n++) {
                            regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.genericEtherType[n] =
                                0x00001260+n*0x4;
                        }/* end of loop n */
                    }/*0000ad4+n*0x4*/
                    {/*0000a2c+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 3 ; n++) {
                            regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.etherTypePriorityArrayProfile[n] =
                                0x00001230+n*0x4;
                        }/* end of loop n */
                    }/*0000a2c+n*0x4*/
                    {/*0001074+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= lastRxDmaIndex/*8*/ ; n++) { /* manually fixed from : for(n = 0 ; n <= 63 ; n++) { */
                            SKIP_NON_EXISTS_PORT_IN_BMP(validRxDmaBmpPtr,n);
                            regsAddrPtr->rxDMA[index].globalRxDMAConfigs.preIngrPrioritizationConfStatus.channelPIPConfigReg[n] =
                                0x00013f0+n*0x4;
                        }/* end of loop n */
                    }/*0001074+n*0x4*/

                }/*end of unit preIngrPrioritizationConfStatus */


                {/*start of unit dropThresholds */
                    regsAddrPtr->sip6_rxDMA[index].configs.dropThresholds.lowPrioDropThreshold0 = 0x00000080;
                    regsAddrPtr->sip6_rxDMA[index].configs.dropThresholds.lowPrioDropThreshold1 = 0x00000084;

                }/*end of unit dropThresholds */


                {/*start of unit dataAccumulator */
                    regsAddrPtr->sip6_rxDMA[index].configs.dataAccumulator.PBFillLevelThresholds = 0x00000000;
                    regsAddrPtr->sip6_rxDMA[index].configs.dataAccumulator.maxTailBC = 0x00000004;

                }/*end of unit dataAccumulator */

                {/*start of unit cutThrough */
                    {/*0000038+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 3 ; n++) {
                            regsAddrPtr->sip6_rxDMA[index].configs.cutThrough.CTGenericConfigs[n] =
                                0x0000010c+n*0x4;
                        }/* end of loop n */
                    }/*0000038+n*0x4*/
                    {/*0000050+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= lastRxDmaIndex/*8*/ ; n++) {
                            SKIP_NON_EXISTS_PORT_IN_BMP(validRxDmaBmpPtr,n);
                            regsAddrPtr->sip6_rxDMA[index].configs.cutThrough.channelCTConfig[n] =
                                0x00000120+n*0x4;
                        }/* end of loop n */
                    }/*0000050+n*0x4*/
                    regsAddrPtr->sip6_rxDMA[index].configs.cutThrough.CTGeneralConfig = 0x00000100;
                    regsAddrPtr->sip6_rxDMA[index].configs.cutThrough.CTErrorConfig = 0x00000104;
                    regsAddrPtr->sip6_rxDMA[index].configs.cutThrough.CTUPEnable = 0x00000108;

                }/*end of unit cutThrough */

                {/*start of unit channelConfig */
                    {/*00004a8+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= lastRxDmaIndex/*8*/ ; n++) {
                            SKIP_NON_EXISTS_PORT_IN_BMP(validRxDmaBmpPtr,n);
                            regsAddrPtr->sip6_rxDMA[index].configs.channelConfig.channelToLocalDevSourcePort[n] =
                                0x0000a00+n*0x4;
                        }/* end of loop n */
                    }/*00004a8+n*0x4*/
                    {/*00003a8+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= lastRxDmaIndex/*8*/ ; n++) {
                            SKIP_NON_EXISTS_PORT_IN_BMP(validRxDmaBmpPtr,n);
                            regsAddrPtr->sip6_rxDMA[index].configs.channelConfig.PCHConfig[n] =
                                0x0000900+n*0x4;
                        }/* end of loop n */
                    }/*00003a8+n*0x4*/
                    {/*00005a8+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= lastRxDmaIndex/*8*/ ; n++) {
                            SKIP_NON_EXISTS_PORT_IN_BMP(validRxDmaBmpPtr,n);
                            regsAddrPtr->sip6_rxDMA[index].configs.channelConfig.channelGeneralConfigs[n] =
                                0x0000b00+n*0x4;
                        }/* end of loop n */
                    }/*00005a8+n*0x4*/
                    {/*00002a8+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= lastRxDmaIndex/*8*/ ; n++) {
                            SKIP_NON_EXISTS_PORT_IN_BMP(validRxDmaBmpPtr,n);
                            regsAddrPtr->sip6_rxDMA[index].configs.channelConfig.channelCascadePort[n] =
                                0x0000800+n*0x4;
                        }/* end of loop n */
                    }/*00002a8+n*0x4*/

                }/*end of unit channelConfig */

            }/*end of unit configs */
        }/*end of unit sip6_rxDMA[] */
    }
}
/**
* @internal packetBufferInit function
* @endinternal
 *
* @brief   init the DB - Packet Buffer unit
 *
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
*                                       None
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] tileIndex                - The index of the instance (one per tile).
*/
static GT_STATUS packetBufferInit_sip6
(
    IN  GT_U8   devNum
)
{
    GT_STATUS rc;
    GT_U32 ii, jj;
    GT_U32 *unitBasesArr;
    GT_U32 base;
    GT_U32 relativeBase;
    GT_U32 unitId;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    GT_U32 base_pbCounter;
    GT_U32 base_gpcGrpPacketWrite;
    GT_U32 base_gpcPacketWrite;
    GT_U32 step_gpcPacketWrite;
    GT_U32 base_gpcGrpPacketRead;
    GT_U32 base_gpcPacketRead;
    GT_U32 step_gpcPacketRead;
    GT_U32 base_gpcGrpCellRead;
    GT_U32 base_gpcCellRead;
    GT_U32 step_gpcCellRead;

    /* common for all versions */
    base_gpcGrpPacketWrite = 0x1000;
    base_gpcPacketWrite    = 0;
    step_gpcPacketWrite    = 0x400;
    base_gpcGrpPacketRead  = 0x2000;
    base_gpcPacketRead     = 0;
    step_gpcPacketRead     = 0x1000;
    base_gpcGrpCellRead    = 0x200;
    base_gpcCellRead       = 0;
    step_gpcCellRead       = 0x100;

    if ((PRV_CPSS_DXCH_FALCON_CIDER_VERSION_17_11_11_E
        >= prvCpssDxchFalconCiderVersionGet()))
    {
        base_pbCounter         = 0x4000;
    }
    else
    {
        /* 18_02_22 */
        base_pbCounter         = 0x0000;
    }

    /* no register addresses update needed    */
    /* this function addes correct bases      */

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum) == 0)
    {
        return GT_OK;
    }

    rc = prvCpssDxChUnitBaseTableGet(devNum,/*OUT*/&unitBasesArr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* pbCenter */
    unitId = PRV_CPSS_DXCH_UNIT_PB_CENTER_BLK_E;
    base = unitBasesArr[unitId];
    if ((PRV_CPSS_DXCH_FALCON_CIDER_VERSION_17_11_11_E
        >= prvCpssDxchFalconCiderVersionGet()))
    {
        regsAddrPtr->sip6_packetBuffer.pbCenter.npmRandomizerEnable    = base + 0x00005000;
        regsAddrPtr->sip6_packetBuffer.pbCenter.tileId                 = base + 0x00005204;
        regsAddrPtr->sip6_packetBuffer.pbCenter.tileMode               = base + 0x00005208;
    }
    else
    {
        /* 18_02_22 */
        regsAddrPtr->sip6_packetBuffer.pbCenter.npmRandomizerEnable    = base + 0x00001000;
        regsAddrPtr->sip6_packetBuffer.pbCenter.tileId                 = base + 0x00001404;
        regsAddrPtr->sip6_packetBuffer.pbCenter.tileMode               = base + 0x00001408;
    }

    /* pbCounter inside bpCenter */
    unitId = PRV_CPSS_DXCH_UNIT_PB_CENTER_BLK_E;
    base = unitBasesArr[unitId] + base_pbCounter;
    regsAddrPtr->sip6_packetBuffer.pbCounter.enablePbc         = base + 0x0000000C;
    regsAddrPtr->sip6_packetBuffer.pbCounter.enableMasterMode  = base + 0x00000010;
    regsAddrPtr->sip6_packetBuffer.pbCounter.enableInterTileTransact  = base + 0x00000014;
    regsAddrPtr->sip6_packetBuffer.pbCounter.cfgTile           = base + 0x0000002C;
    regsAddrPtr->sip6_packetBuffer.pbCounter.resizeSmbCells    = base + 0x0000001C;
    regsAddrPtr->sip6_packetBuffer.pbCounter.resizeSmbMcCells  = base + 0x00000020;
    regsAddrPtr->sip6_packetBuffer.pbCounter.resizeSmbSumCells = base + 0x00000030;
    regsAddrPtr->sip6_packetBuffer.pbCounter.resizeNpmWords    = base + 0x00000024;
    regsAddrPtr->sip6_packetBuffer.pbCounter.resizeNpmMcWords  = base + 0x00000028;
    regsAddrPtr->sip6_packetBuffer.pbCounter.resizeNpmSumWords = base + 0x00000034;
    regsAddrPtr->sip6_packetBuffer.pbCounter.cfgFillThreshold  = base + 0x00000038;
    /* fill counters */
    regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbLocalMax            = base + 0x00000100;
    regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbLocalMin            = base + 0x00000104;
    regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbLocalSum            = base + 0x0000015c;
    regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbLocalMcSum          = base + 0x00000108;
    regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmLocalMax            = base + 0x0000010C;
    regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmLocalMin            = base + 0x00000110;
    regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmLocalSum            = base + 0x0000018C;
    regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmLocalMcSum          = base + 0x00000114;
    regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbMax                 = base + 0x00000118;
    regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbMin                 = base + 0x0000011C;
    regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbSum                 = base + 0x00000130;
    regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbMcSum               = base + 0x00000120;
    regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbUcSum               = base + 0x00000134;
    regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmMax                 = base + 0x00000124;
    regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmMin                 = base + 0x00000128;
    regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmSum                 = base + 0x00000160;
    regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmMcSum               = base + 0x0000012C;
    regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmUcSum               = base + 0x00000164;
    regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbDiff                = base + 0x00000138;
    regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmDiff                = base + 0x00000168;
    regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbOvertimeMaxSum      = base + 0x00000144;
    regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbOvertimeMaxMcSum    = base + 0x0000013C;
    regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbOvertimeMaxUcSum    = base + 0x00000140;
    regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbOvertimeMinSum      = base + 0x00000150;
    regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbOvertimeMinMcSum    = base + 0x00000148;
    regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbOvertimeMinUcSum    = base + 0x0000014C;
    regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbOvertimeMaxDiff     = base + 0x00000154;
    regsAddrPtr->sip6_packetBuffer.pbCounter.dbgSmbOvertimeMinDiff     = base + 0x00000158;
    regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmOvertimeMaxSum      = base + 0x00000174;
    regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmOvertimeMaxMcSum    = base + 0x0000016C;
    regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmOvertimeMaxUcSum    = base + 0x00000170;
    regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmOvertimeMinSum      = base + 0x00000180;
    regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmOvertimeMinMcSum    = base + 0x00000178;
    regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmOvertimeMinUcSum    = base + 0x0000017C;
    regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmOvertimeMaxDiff     = base + 0x00000184;
    regsAddrPtr->sip6_packetBuffer.pbCounter.dbgNpmOvertimeMinDiff     = base + 0x00000188;

    /* smbWriteArbiter inside bpCenter */
    unitId = PRV_CPSS_DXCH_UNIT_PB_SMB_WRITE_ARBITER_E;
    base = unitBasesArr[unitId];
    regsAddrPtr->sip6_packetBuffer.smbWriteArbiter.cfgEnable = base + 0x00000004;

    /* gpcGrpPacketWrite */
    for (ii = 0; (ii < (MAX_DP_PER_TILE_CNS / 4)); ii++)
    {
        switch (ii)
        {
            case 0: unitId = PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_WRITE_0_E; break;
            case 1: unitId = PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_WRITE_1_E; break;
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        base = unitBasesArr[unitId] + base_gpcGrpPacketWrite;
        regsAddrPtr->sip6_packetBuffer.gpcGrpPacketWrite[ii].npmRandomizerEnable   = base + 0x00000014;
    }

    /* gpcPacketWrite */
    for (ii = 0; (ii < MAX_DP_PER_TILE_CNS); ii++)
    {
        switch (ii / 4)
        {
            case 0: unitId = PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_WRITE_0_E; break;
            case 1: unitId = PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_WRITE_1_E; break;
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        base = unitBasesArr[unitId] + base_gpcPacketWrite + (step_gpcPacketWrite * (ii % 4));
        regsAddrPtr->sip6_packetBuffer.gpcPacketWrite[ii].cfgEnable = base + 0x00000000;
    }

    /* gpcGrpPacketRead */
    for (ii = 0; (ii < (MAX_DP_PER_TILE_CNS / 2)); ii++)
    {
        switch (ii)
        {
            case 0: unitId = PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_0_E; break;
            case 1: unitId = PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_1_E; break;
            case 2: unitId = PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_2_E; break;
            case 3: unitId = PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_3_E; break;
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        base = unitBasesArr[unitId] + base_gpcGrpPacketRead;
        regsAddrPtr->sip6_packetBuffer.gpcGrpPacketRead[ii].badAddress = base + 0x00000014;
    }

    /* gpcPacketRead */
    for (ii = 0; (ii < MAX_DP_PER_TILE_CNS); ii++)
    {
        switch (ii / 2)
        {
            case 0: unitId = PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_0_E; break;
            case 1: unitId = PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_1_E; break;
            case 2: unitId = PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_2_E; break;
            case 3: unitId = PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_3_E; break;
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        base = unitBasesArr[unitId] + base_gpcPacketRead + (step_gpcPacketRead * (ii % 2));
        for (jj = 0; (jj < 10); jj++)
        {
            regsAddrPtr->sip6_packetBuffer.gpcPacketRead[ii].channelEnable[jj] =
                base + (jj * 4) + 0x00000080;
            regsAddrPtr->sip6_packetBuffer.gpcPacketRead[ii].channelReset[jj]  =
                base + (jj * 4) + 0x00000180;

            if (PRV_CPSS_DXCH_FALCON_CIDER_VERSION_18_03_13_E
                <= prvCpssDxchFalconCiderVersionGet())
            {
                regsAddrPtr->sip6_packetBuffer.gpcPacketRead[ii].channelShaper[jj]  =
                    base + (jj * 4) + 0x00000280;
            }
        }
        /* gpcPacketRead[ii].packetCount..., fifoSegment, arbiter */
        if (PRV_CPSS_DXCH_FALCON_CIDER_VERSION_18_03_13_E
            <= prvCpssDxchFalconCiderVersionGet())
        {
            relativeBase = 0x100;
        }
        else
        {
            relativeBase = 0;
        }
        regsAddrPtr->sip6_packetBuffer.gpcPacketRead[ii].packetCountEnable =
            base + relativeBase + 0x00000280;
        regsAddrPtr->sip6_packetBuffer.gpcPacketRead[ii].packetCountReset  =
            base + relativeBase + 0x00000284;

        for (jj = 0; (jj < 10); jj++)
        {
            regsAddrPtr->sip6_packetBuffer.gpcPacketRead[ii].fifoSegmentStart[jj]  =
                base + relativeBase + (jj * 4) + 0x00000600;
        }
        for (jj = 0; (jj < 20); jj++)
        {
            regsAddrPtr->sip6_packetBuffer.gpcPacketRead[ii].fifoSegmentNext[jj]  =
                base + relativeBase + (jj * 4) + 0x00000300;
        }
        regsAddrPtr->sip6_packetBuffer.gpcPacketRead[ii].cfgUpdateTrigger  =
            base + relativeBase + 0x00000700;

        regsAddrPtr->sip6_packetBuffer.gpcPacketRead[ii].arbiterPointHighSpeedChannel0En =
            base + relativeBase + 0x00000704;
        for (jj = 0; (jj < 5); jj++)
        {
            regsAddrPtr->sip6_packetBuffer.gpcPacketRead[ii].arbiterPointGroup1[jj]  =
                base + relativeBase + (jj * 4) + 0x00000714;
        }
        for (jj = 0; (jj < 5); jj++)
        {
            regsAddrPtr->sip6_packetBuffer.gpcPacketRead[ii].arbiterPointGroup2[jj]  =
                base + relativeBase + (jj * 4) + 0x00000728;
        }
        for (jj = 0; (jj < 32); jj++)
        {
            regsAddrPtr->sip6_packetBuffer.gpcPacketRead[ii].arbiterCalendarSlot[jj]  =
                base + relativeBase + (jj * 4) + 0x0000073C;
        }
        regsAddrPtr->sip6_packetBuffer.gpcPacketRead[ii].arbiterCalendarMax  =
            base + relativeBase + 0x000007BC;
    }

    /* gpcGrpCellRead */
    for (ii = 0; (ii < (MAX_DP_PER_TILE_CNS / 2)); ii++)
    {
        switch (ii)
        {
            case 0: unitId = PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_0_E; break;
            case 1: unitId = PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_1_E; break;
            case 2: unitId = PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_2_E; break;
            case 3: unitId = PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_3_E; break;
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        base = unitBasesArr[unitId] + base_gpcGrpCellRead;
        regsAddrPtr->sip6_packetBuffer.gpcGrpCellRead[ii].badRegister = base + 0x00000014;
    }

    /* gpcCellRead */
    for (ii = 0; (ii < MAX_DP_PER_TILE_CNS); ii++)
    {
        switch (ii / 2)
        {
            case 0: unitId = PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_0_E; break;
            case 1: unitId = PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_1_E; break;
            case 2: unitId = PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_2_E; break;
            case 3: unitId = PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_3_E; break;
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        base = unitBasesArr[unitId] + base_gpcCellRead + (step_gpcCellRead * (ii % 2));
        regsAddrPtr->sip6_packetBuffer.gpcCellRead[ii].initialization = base + 0x00000000;
    }

    /* npmMc */
    for (ii = 0; (ii < 3); ii++)
    {
        switch (ii)
        {
            case 0: unitId = PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_0_E; break;
            case 1: unitId = PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_1_E; break;
            case 2: unitId = PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_2_E; break;
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        base = unitBasesArr[unitId];
        regsAddrPtr->sip6_packetBuffer.npmMc[ii].cfgEnable         = base + 0x00000000;
        regsAddrPtr->sip6_packetBuffer.npmMc[ii].cfgUnit           = base + 0x00000300;
        regsAddrPtr->sip6_packetBuffer.npmMc[ii].cfgRefs           = base + 0x00000400;
        for (jj = 0; (jj < 4); jj++)
        {
            regsAddrPtr->sip6_packetBuffer.npmMc[ii].insList[jj]
                = base + 0x00000210 + (jj * 4);
        }
        regsAddrPtr->sip6_packetBuffer.npmMc[ii].intReinsertMask   = base + 0x0000011C;
    }

    /* smbMc */
    for (ii = 0; (ii < 6); ii++)
    {
        switch (ii)
        {
            case 0: unitId = PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_0_0_E; break;
            case 1: unitId = PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_0_1_E; break;
            case 2: unitId = PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_1_0_E; break;
            case 3: unitId = PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_1_1_E; break;
            case 4: unitId = PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_2_0_E; break;
            case 5: unitId = PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_2_1_E; break;
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        base = unitBasesArr[unitId];
        regsAddrPtr->sip6_packetBuffer.smbMc[ii].cfgEnable = base + 0x00000004;
    }

    return GT_OK;
}

/**
* @internal txdmaInit_sip6 function
* @endinternal
*
* @brief   sip6 : init the DB - TXDMA unit (per unit index)
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] index                    - unit
*                                       None
*/
static void txdmaInit_sip6
(
    IN  GT_U8 devNum,
    IN  GT_U32  index,
    IN  GT_U32  lastTxDmaIndex,
    IN CPSS_PORTS_BMP_STC *validTxDmaBmpPtr
)
{

    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

     {/*start of unit sip6_txDMA[] */
        {/*start of unit globalConfigs */
            regsAddrPtr->sip6_txDMA[index].configs.globalConfigs.txDMAGlobalConfig = 0x00000000;
            regsAddrPtr->sip6_txDMA[index].configs.globalConfigs.pizzaArbiter.pizzaArbiterCtrlReg = 0x00000050;
            {/*0000100+p*0x4*/
                GT_U32    p;
                for (p = 0 ; p < 33 /* pizza slices number*/; p++) {
                    regsAddrPtr->sip6_txDMA[index].configs.globalConfigs.pizzaArbiter.pizzaArbiterConfigReg[p] =
                        0x00000100 + p*0x4;
                }/* end if loop p*/
            } /*0000100+p*0x4*/

        }/*end of unit globalConfigs */

        {/*start of unit speedProfileConfigurations */
            {/*0x00000600 + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p < MAX_SPEED_PROFILE_NUM_CNS; p++) {
                    regsAddrPtr->sip6_txDMA[index].configs.speedProfileConfigs.descFIFODepth[p] =
                        0x00000600 + p*0x4;
                    }/* end of loop p */
            }/*0x00000600 + p*0x4*/
            {/*0x00000700  + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p < MAX_SPEED_PROFILE_NUM_CNS; p++) {
                    regsAddrPtr->sip6_txDMA[index].configs.speedProfileConfigs.sdqMaxCredits[p] =
                        0x00000700  + p*0x4;
                    }/* end of loop p */
            }/*0x00000700  + p*0x4*/
            {/*0x00000800 + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p < MAX_SPEED_PROFILE_NUM_CNS; p++) {
                    regsAddrPtr->sip6_txDMA[index].configs.speedProfileConfigs.sdqThresholdBytes[p] =
                        0x00000800 + p*0x4;
                    }/* end of loop p */
            }/*0x00000800 + p*0x4*/
            {/*0x00000900 + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p < MAX_SPEED_PROFILE_NUM_CNS; p++) {
                    regsAddrPtr->sip6_txDMA[index].configs.speedProfileConfigs.maxWordCredits[p] =
                        0x00000900 + p*0x4;
                    }/* end of loop p */
            }/*0x00000900 + p*0x4*/
            {/*0x00000A00 + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p < MAX_SPEED_PROFILE_NUM_CNS; p++) {
                    regsAddrPtr->sip6_txDMA[index].configs.speedProfileConfigs.maxCellsCredits[p] =
                        0x00000A00 + p*0x4;
                    }/* end of loop p */
            }/*0x00000A00 + p*0x4*/
            {/*0x00000B00 + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p < MAX_SPEED_PROFILE_NUM_CNS; p++) {
                    regsAddrPtr->sip6_txDMA[index].configs.speedProfileConfigs.maxDescCredits[p] =
                        0x00000B00 + p*0x4;
                    }/* end of loop p */
            }/*0x00000B00 + p*0x4*/
            {/*0x00000C00 + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p < MAX_SPEED_PROFILE_NUM_CNS; p++) {
                    regsAddrPtr->sip6_txDMA[index].configs.speedProfileConfigs.interPacketRateLimiter[p] =
                        0x00000C00 + p*0x4;
                    }/* end of loop p */
            }/*0x00000C00 + p*0x4*/
             {/*0x00000D00 + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p < MAX_SPEED_PROFILE_NUM_CNS; p++) {
                    regsAddrPtr->sip6_txDMA[index].configs.speedProfileConfigs.interCellRateLimiter[p] =
                        0x00000D00 + p*0x4;
                    }/* end of loop p */
            }/*0x00000D00 + p*0x4*/

        }/*end of unit speedProfileConfigurations */

        {/*start of unit channelConfigs */
            {/*0x00002000  + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p <= lastTxDmaIndex/*8*/ ; p++) { /* manually fixed from : for(p = 0 ; p < 64 ; n++) { */
                    SKIP_NON_EXISTS_PORT_IN_BMP(validTxDmaBmpPtr,p);
                    regsAddrPtr->sip6_txDMA[index].configs.channelConfigs.speedProfile[p] =
                        0x00002000 +p*0x4;
                }/* end of loop n */
            }/*0x00002000  + p*0x4*/
            {/*0x00002100   + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p <= lastTxDmaIndex/*8*/ ; p++) { /* manually fixed from : for(p = 0 ; p < 64 ; n++) { */
                    SKIP_NON_EXISTS_PORT_IN_BMP(validTxDmaBmpPtr,p);
                    regsAddrPtr->sip6_txDMA[index].configs.channelConfigs.channelReset[p] =
                        0x00002100  +p*0x4;
                }/* end of loop n */
            }/*0x00002100   + p*0x4*/
            {/*0x00002200   + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p <= lastTxDmaIndex/*8*/ ; p++) { /* manually fixed from : for(p = 0 ; p < 64 ; n++) { */
                    SKIP_NON_EXISTS_PORT_IN_BMP(validTxDmaBmpPtr,p);
                    regsAddrPtr->sip6_txDMA[index].configs.channelConfigs.descFIFOBase[p] =
                        0x00002200  +p*0x4;
                }/* end of loop n */
            }/*0x00002200   + p*0x4*/
            {/*0x00002300  + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p <= lastTxDmaIndex/*8*/ ; p++) { /* manually fixed from : for(p = 0 ; p < 64 ; n++) { */
                    SKIP_NON_EXISTS_PORT_IN_BMP(validTxDmaBmpPtr,p);
                    regsAddrPtr->sip6_txDMA[index].configs.channelConfigs.interPacketRateLimiterConfig[p] =
                        0x00002300 +p*0x4;
                }/* end of loop n */
            }/*0x00002300  + p*0x4*/
            {/*0x00002400  + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p <= lastTxDmaIndex/*8*/ ; p++) { /* manually fixed from : for(p = 0 ; p < 64 ; n++) { */
                    SKIP_NON_EXISTS_PORT_IN_BMP(validTxDmaBmpPtr,p);
                    regsAddrPtr->sip6_txDMA[index].configs.channelConfigs.interCellRateLimiterConfig[p] =
                        0x00002400 +p*0x4;
                }/* end of loop n */
            }/*0x00002400  + p*0x4*/
            {/*0x00002500  + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p <= lastTxDmaIndex/*8*/ ; p++) { /* manually fixed from : for(p = 0 ; p < 64 ; n++) { */
                    SKIP_NON_EXISTS_PORT_IN_BMP(validTxDmaBmpPtr,p);
                    regsAddrPtr->sip6_txDMA[index].configs.channelConfigs.inerPacketRateLimiterConfig[p] =
                        0x00002500 +p*0x4;
                }/* end of loop n */
            }/*0x00002500  + p*0x4*/
        }/*end of unit channelConfigs */

        {/*start of unit TDSConfiguration */
            regsAddrPtr->sip6_txDMA[index].TDSConfigs.globalConfig = 0x00004000;
            regsAddrPtr->sip6_txDMA[index].TDSConfigs.channelReset = 0x00004004;
            regsAddrPtr->sip6_txDMA[index].TDSConfigs.cellRateLimiterConfig = 0x00004008;
            regsAddrPtr->sip6_txDMA[index].TDSConfigs.cellRateLimiter = 0x0000400C;

        }/*end of unit TDSConfiguration */

     }/*end of unit sip6_txDMA[] */
}


/**
* @internal txfifoInit_sip6 function
* @endinternal
*
* @brief   sip6 : init the DB - TXFIFO unit (per unit index)
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] index                    - unit
*                                       None
*/
static void txfifoInit_sip6
(
    IN  GT_U8 devNum,
    IN  GT_U32  index,
    IN  GT_U32  lastTxFifoIndex,
    IN CPSS_PORTS_BMP_STC *validTxFifoBmpPtr
)
{

    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

     {/*start of unit sip6_txFIFO[] */
        {/*start of unit globalConfigs */
            regsAddrPtr->sip6_txFIFO[index].configs.globalConfigs.globalConfig1 = 0x00000000;
            regsAddrPtr->sip6_txFIFO[index].configs.globalConfigs.packetsMaximalReadLatencyFromPacketPuffer = 0x00000004;
            regsAddrPtr->sip6_txFIFO[index].configs.globalConfigs.wordBufferCmn8wr4rdBufferConfig = 0x00000008;
            regsAddrPtr->sip6_txFIFO[index].configs.globalConfigs.pizzaArbiter.pizzaArbiterCtrlReg = 0x00000050;
            {/*0000100+p*0x4*/
                GT_U32    p;
                for ( p = 0 ; p < 33 /*pizza slixes number*/ ; p++) {
                    regsAddrPtr->sip6_txFIFO[index].configs.globalConfigs.pizzaArbiter.pizzaArbiterConfigReg[p] =
                        0x00000100 + p*0x4;
                }/* end if loop p*/
            } /*0000100+p*0x4*/

        }/*end of unit globalConfigs */

        {/*start of unit speedProfileConfigs */
            {/*0x00000600 + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p < MAX_SPEED_PROFILE_NUM_CNS; p++) {
                    regsAddrPtr->sip6_txFIFO[index].configs.speedProfileConfigs.descFIFODepth[p] =
                        0x00000600 + p*0x4;
                    }/* end of loop p */
            }/*0x00000600 + p*0x4*/
            {/*0x00000700  + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p < MAX_SPEED_PROFILE_NUM_CNS; p++) {
                    regsAddrPtr->sip6_txFIFO[index].configs.speedProfileConfigs.tagFIFODepth[p] =
                        0x00000700  + p*0x4;
                    }/* end of loop p */
            }/*0x00000700  + p*0x4*/

        }/*end of unit speedProfileConfigs */

        {/*start of unit channelConfigs */
            {/*0x00001000  + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p <= lastTxFifoIndex/*8*/ ; p++) { /* manually fixed from : for(p = 0 ; p < 64 ; n++) { */
                    SKIP_NON_EXISTS_PORT_IN_BMP(validTxFifoBmpPtr,p);
                    regsAddrPtr->sip6_txFIFO[index].configs.channelConfigs.speedProfile[p] =
                        0x00001000 +p*0x4;
                }/* end of loop n */
            }/*0x00001000  + p*0x4*/
            {/*0x00001100   + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p <= lastTxFifoIndex/*8*/ ; p++) { /* manually fixed from : for(p = 0 ; p < 64 ; n++) { */
                    SKIP_NON_EXISTS_PORT_IN_BMP(validTxFifoBmpPtr,p);
                    regsAddrPtr->sip6_txFIFO[index].configs.channelConfigs.descFIFOBase[p] =
                        0x00001100  +p*0x4;
                }/* end of loop n */
            }/*0x00001100   + p*0x4*/
            {/*0x00001200   + p*0x4*/
                GT_U32    p;
                for(p = 0 ; p <= lastTxFifoIndex/*8*/ ; p++) { /* manually fixed from : for(p = 0 ; p < 64 ; n++) { */
                    SKIP_NON_EXISTS_PORT_IN_BMP(validTxFifoBmpPtr,p);
                    regsAddrPtr->sip6_txFIFO[index].configs.channelConfigs.tagFIFOBase[p] =
                        0x00001200  +p*0x4;
                }/* end of loop n */
            }/*0x00001200   + p*0x4*/

        }/*end of unit channelConfigs */

        {/*start of unit debug*/
            regsAddrPtr->sip6_txFIFO[index].debug.statusBadAddr = 0x00005000;
        }/*end of unit debug*/

     }/*end of unit sip6_txFIFO[] */
}

/**
* @internal shmInit_sip6 function
* @endinternal
*
* @brief   sip6 : init the DB - SHM unit. shared memories (FDB/EM/ARP-TS/LPM)
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None
*/
static void shmInit_sip6
(
    IN  GT_U8 devNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    {/*start of unit SHM */
        {/*00000000+n*0x4*/
            GT_U32    n;
            for(n = 0 ; n <= 39 ; n++) {
                regsAddrPtr->SHM.block_port_select[n] =
                    0x00000000+n*0x4;
            }/* end of loop n */
        }/*00000000+n*0x4*/
        regsAddrPtr->SHM.shm_engines_size_mode = 0x000000a0;
        regsAddrPtr->SHM.bad_address_latch = 0x000000d4;
        regsAddrPtr->SHM.shm_error_sum_cause = 0x000000e0;
        regsAddrPtr->SHM.shm_error_sum_mask = 0x000000e4;
        regsAddrPtr->SHM.lpm_illegal_address = 0x000000f0;
        regsAddrPtr->SHM.fdb_illegal_address = 0x000000f4;
        regsAddrPtr->SHM.em_illegal_address = 0x000000f8;
        regsAddrPtr->SHM.arp_illegal_address = 0x000000fc;
        regsAddrPtr->SHM.lpm_aging_illegal_address = 0x00000100;

    }/*end of unit SHM */

}

/**
* @internal EagleD2dCpInRaven_sip6 function
* @endinternal
*
* @brief   sip6 : init the DB - EagleD2dCpInRaven unit.
*          registers in address space of Raven ... but belong to eagle.
*          NOTE: there are 2 D2D CP in dual tile , each serve tile
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] d2dCpIndex               - D2D CP index
*                                       None
*/
static void EagleD2dCpInRaven_sip6
(
    IN  GT_U8   devNum,
    IN  GT_U32  d2dCpIndex
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    /*0x02800000 + c*0x80000: where c (0-1) represents D2D_CP*/
    regsAddrPtr->EagleD2dCpInRaven[d2dCpIndex].D2D_CP.D2D_CP_RF.cfgTapBroadcastEn = 0x0000001C;

    {/*0x0287FF80 + 0x8*N: where N (0-7) represents Windows*/
        GT_U32 N;
        for(N = 0 ; N < 8; N++)
        {
            regsAddrPtr->EagleD2dCpInRaven[d2dCpIndex].D2D_CP_AMB.D2D_CP_A2M.
                AXI_to_MBUS_Bridge.Bridge_Windows[N].BridgeWindowControl = 0x0007FF80 + 0x8*N;
            regsAddrPtr->EagleD2dCpInRaven[d2dCpIndex].D2D_CP_AMB.D2D_CP_A2M.
                AXI_to_MBUS_Bridge.Bridge_Windows[N].BridgeWindowBase    = 0x0007FF84 + 0x8*N;
        }
    }

}

/**
* @internal sip6EagleD2dCpInit_regDb function
* @endinternal
*
* @brief   sip6 : init the DB - EagleD2dCpInRaven unit.
*          registers in address space of Raven ... but belong to eagle.
*          NOTE: there are 2 D2D CP in dual tile , each serve tile
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None
*/
static void sip6EagleD2dCpInit_regDb
(
    IN  GT_U8   devNum
)
{
    static PRV_CPSS_DXCH_UNIT_ENT EagleD2dCpInRavenUnit = PRV_CPSS_DXCH_UNIT_EAGLE_D2D_CP_0_IN_RAVEN_E;

    GT_U32  currEagleD2dCpInRavenBaseAddr;
    GT_U32  d2d0EagleD2dCpInRavenBaseAddr = 0;
    GT_U32  offsetFromD2dCp0;
    GT_U32  d2dCpIndex;/* D2D index */
    GT_U32  numOfD2dCp = PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;

    for(d2dCpIndex = 0 ; d2dCpIndex < numOfD2dCp ; d2dCpIndex++)
    {
        if(d2dCpIndex == 0)
        {
            d2d0EagleD2dCpInRavenBaseAddr = prvCpssDxChHwUnitBaseAddrGet(devNum,EagleD2dCpInRavenUnit,NULL);
            currEagleD2dCpInRavenBaseAddr = d2d0EagleD2dCpInRavenBaseAddr;
        }
        else
        {
            offsetFromD2dCp0 = prvCpssSip6OffsetFromFirstInstanceGet(devNum,d2dCpIndex,EagleD2dCpInRavenUnit);
            currEagleD2dCpInRavenBaseAddr = d2d0EagleD2dCpInRavenBaseAddr + offsetFromD2dCp0;
        }

        /* init D2D registers of Eagle that in memory space of Raven */
        EagleD2dCpInRaven_sip6(devNum,d2dCpIndex);
        /* set the addresses of the EagleD2dCpInRaven unit registers according to the needed base address */
        regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, currEagleD2dCpInRavenBaseAddr, EagleD2dCpInRaven[d2dCpIndex]));
    }
}


/**
* @internal phaInit_sip6 function
* @endinternal
*
* @brief   sip6 : init the DB - PHA unit. (programmable HA)
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None
*/
static void phaInit_sip6
(
    IN  GT_U8 devNum
)
{
    GT_U32  ppg,ppn;
    GT_U32  ppgOffset,ppnOffset,totalOffset;

    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    {/*startof unit pha_regs */
        {/*start of unit PPAThreadConfigs */
            regsAddrPtr->PHA.pha_regs.PPAThreadConfigs.nullProcessingInstructionPointer = 0x007fff20;

        }/*end of unit PPAThreadConfigs */


        {/*start of unit generalRegs */
            regsAddrPtr->PHA.pha_regs.generalRegs.PHACtrl = 0x007fff00;
            regsAddrPtr->PHA.pha_regs.generalRegs.PHAStatus = 0x007fff04;

        }/*end of unit generalRegs */


        {/*start of unit errorsAndInterrupts */
            regsAddrPtr->PHA.pha_regs.errorsAndInterrupts.headerModificationViolationConfig = 0x007fff60;
            regsAddrPtr->PHA.pha_regs.errorsAndInterrupts.PPAClockDownErrorConfig = 0x007fff68;
            regsAddrPtr->PHA.pha_regs.errorsAndInterrupts.headerModificationViolationAttributes = 0x007fff70;
            regsAddrPtr->PHA.pha_regs.errorsAndInterrupts.PHABadAddrLatch = 0x007fff74;
            regsAddrPtr->PHA.pha_regs.errorsAndInterrupts.PHAInternalErrorCause = 0x007fff80;
            regsAddrPtr->PHA.pha_regs.errorsAndInterrupts.PHAInternalErrorMask = 0x007fff84;
            regsAddrPtr->PHA.pha_regs.errorsAndInterrupts.PHAInterruptSumCause = 0x007fff88;
            regsAddrPtr->PHA.pha_regs.errorsAndInterrupts.PHAInterruptSumMask = 0x007fff8c;

        }/*end of unit errorsAndInterrupts */


        {/*start of unit debugCounters */
            regsAddrPtr->PHA.pha_regs.debugCounters.statisticalProcessingDebugCounterConfig = 0x007FFF90;
            regsAddrPtr->PHA.pha_regs.debugCounters.statisticalProcessingDebugCounter       = 0x007FFF94;

        }/*end of unit debugCounters */

    }/*end of unit pha_regs */

    {/*start of unit PPA */
        {/*start of unit ppa_regs */
            regsAddrPtr->PHA.PPA.ppa_regs.PPACtrl = 0x007eff00;
            regsAddrPtr->PHA.PPA.ppa_regs.PPAInterruptSumCause = 0x007eff10;
            regsAddrPtr->PHA.PPA.ppa_regs.PPAInterruptSumMask = 0x007eff14;
            regsAddrPtr->PHA.PPA.ppa_regs.PPAInternalErrorCause = 0x007eff20;
            regsAddrPtr->PHA.PPA.ppa_regs.PPAInternalErrorMask = 0x007eff24;
            regsAddrPtr->PHA.PPA.ppa_regs.PPABadAddrLatch = 0x007eff70;

        }/*end of unit ppPHA.a_regs */


    }/*end of unit PPA */

    /*m * 0x80000: where m (0-2) represents PPG*/
    for(ppg = 0 ; ppg < 3; ppg++) /* per PPG */
    {
        ppgOffset = ppg * 0x80000;

        {/*start of unit ppg_regs */
            regsAddrPtr->PHA.PPG[ppg].ppg_regs.PPGInterruptSumCause = 0x0007ff00 + ppgOffset;
            regsAddrPtr->PHA.PPG[ppg].ppg_regs.PPGInterruptSumMask = 0x0007ff04 + ppgOffset;
            regsAddrPtr->PHA.PPG[ppg].ppg_regs.PPGInternalErrorCause = 0x0007ff10 + ppgOffset;
            regsAddrPtr->PHA.PPG[ppg].ppg_regs.PPGInternalErrorMask = 0x0007ff14 + ppgOffset;
            regsAddrPtr->PHA.PPG[ppg].ppg_regs.PPGBadAddrLatch = 0x0007ff70 + ppgOffset;

        }/*end of unit ppg_regs */

        /* manually added */
        regsAddrPtr->PHA.PPG[ppg].PPG_IMEM_base_addr = 0x00040000 + ppgOffset;/* base address of this memory (4 words per entry) */

        /* 0x4000 * p: where p (0-8) represents PPN*/
        for(ppn = 0 ; ppn < 9; ppn++) /* per PPN (PER PPG) */
        {/*start of unit ppn_regs */
            ppnOffset = ppn * 0x4000 ;
            totalOffset = ppgOffset + ppnOffset;

            {/*3050+0x10 * t*/
                GT_U32    t;
                for(t = 0 ; t <= 1 ; t++) {
                    regsAddrPtr->PHA.PPG[ppg].PPN[ppn].ppn_regs.TODWord0[t] =
                        0x3050+0x10 * t + totalOffset;
                }/* end of loop t */
            }/*3050+0x10 * t*/
            {/*3054+0x10 * t*/
                GT_U32    t;
                for(t = 0 ; t <= 1 ; t++) {
                    regsAddrPtr->PHA.PPG[ppg].PPN[ppn].ppn_regs.TODWord1[t] =
                        0x3054+0x10 * t + totalOffset;
                }/* end of loop t */
            }/*3054+0x10 * t*/
            {/*3058+0x10 * t*/
                GT_U32    t;
                for(t = 0 ; t <= 1 ; t++) {
                    regsAddrPtr->PHA.PPG[ppg].PPN[ppn].ppn_regs.TODWord2[t] =
                        0x3058+0x10 * t + totalOffset;
                }/* end of loop t */
            }/*3058+0x10 * t*/

            regsAddrPtr->PHA.PPG[ppg].PPN[ppn].ppn_regs.fwName      = 0x00000000 + totalOffset;
            regsAddrPtr->PHA.PPG[ppg].PPN[ppn].ppn_regs.fwVersion   = 0x00000004 + totalOffset;
            regsAddrPtr->PHA.PPG[ppg].PPN[ppn].ppn_regs.PPNDoorbell = 0x00003010 + totalOffset;
            regsAddrPtr->PHA.PPG[ppg].PPN[ppn].ppn_regs.pktHeaderAccessOffset = 0x00003024 + totalOffset;
            regsAddrPtr->PHA.PPG[ppg].PPN[ppn].ppn_regs.PPNInternalErrorCause = 0x00003040 + totalOffset;
            regsAddrPtr->PHA.PPG[ppg].PPN[ppn].ppn_regs.PPNInternalErrorMask = 0x00003044 + totalOffset;
            regsAddrPtr->PHA.PPG[ppg].PPN[ppn].ppn_regs.PPNBadAddrLatch = 0x00003070 + totalOffset;

        }/*end of unit ppn_regs */

    }


}
