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
* @file cpssDxChPha.c
*
* @brief CPSS declarations relate to PHA (programmable header alteration) in the
*   egress processing , that allows enhanced key technologies , such as:
*   In-band Network Telemetry header (INT), NSH metadata, Geneve TLVs, and any
*   new tunnel/shim header that may emerge.
*
*   APPLICABLE DEVICES:      Falcon.
*
*   NOTEs:
*   1. GT_NOT_INITIALIZED will be return for any 'PHA' lib APIs if called before
*       cpssDxChPhaInit(...)
*       (exclude cpssDxChPhaInit(...) itself)
*   2. GT_NOT_INITIALIZED will be return for EPCL APIs trying to enable PHA processing
*       if called before cpssDxChPhaInit(...)
*
* @version   1
********************************************************************************
*/

#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/pha/cpssDxChPha.h>
#include <cpss/dxCh/dxChxGen/pha/private/prvCpssDxChPhaLog.h>

#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/pha/private/prvCpssDxCh_ppa_fw_imem_addr_data.h>

/* indication that thread is not valid/not implemented */
#define FW_INSTRUCTION_POINTER_NOT_VALID_CNS   0xFFFFFFFF

/* ALL addresses of instruction pointers need to be with prefix 0x00400000 */
/* the PHA table need to hold only lower 16 bits (the prefix is added internally by the HW) */
#define FW_INSTRUCTION_DOMAIN_ADDR_CNS      0x00400000
#define FW_INSTRUCTION_DOMAIN_ADDR_MAX_CNS  0x0040FFFF

/* max number of threads that the firmware supports */
#define MAX_THREADS_CNS 32

typedef    GT_U32   FW_THREAD_ID;

#define    THR0_DoNothing                            0
#define    THR1_SRv6                                 1
#define    THR2_SRv6_penultimateEndNode              2
#define    THR3_INT_Ingress_Switch_IPv6              3
#define    THR4_INT_Ingress_Switch_IPv4              4
#define    THR5_INT_Transit_Switch_IPv6              5
#define    THR6_INT_Transit_Switch_IPv4              6
#define    THR7_IOAM_Ingress_Switch_IPv4             7
#define    THR8_IOAM_Ingress_Switch_IPv6             8
#define    THR9_IOAM_Transit_Switch_IPv4             9
#define    THR10_IOAM_Transit_Switch_IPv6           10
#define    THR27_VariableCyclesLengthWithAcclCmd    27
#define    THR28_RemoveAddBytes                     28


typedef enum{
    EXPAND_32B,/* 128B header with maximum expansion 32 bytes; Maximum expansion 32 bytes */
    EXPAND_48B,/* 112B header with maximum expansion 48 bytes; Maximum expansion 48 bytes */
    EXPAND_64B /*  96B header with maximum expansion 64 bytes; Maximum expansion 64 bytes */
}HEADER_WINDOW_SIZE;

typedef enum{
    Outer_L2_Start, /*Start of packet - offset 0*/
    Outer_L3_Start, /*L3 start byte without tunnel start. Tunnel L3 start with tunnel start*/
    Inner_L2_Start, /*Passenger L2 start byte. Relevant only for tunnel start packets with passenger having L2 layer*/
    Inner_L3_Start  /*Passenger L3 start byte. Relevant only for tunnel start packets.*/
}HEADER_WINDOW_ANCHOR;


/* info per FW threadId (instruction pointer) */
typedef struct {
    GT_U32         firmwareInstructionPointer; /*(instruction pointer)*/
    GT_U32         skipCounter;
    HEADER_WINDOW_ANCHOR headerWindowAnchor;
    HEADER_WINDOW_SIZE   headerWindowSize;
}FW_THREAD_INFO;

/*
    NOTE: the  value in sip6FirmwareThreadIdToInstructionPointer are coming from the
    FIRMWARE release in file : PC_addr/Threads_PC.txt
    MUST be aligned with     : FW_Vers/ppa_fw_imem_addr_data.h

    current FIRMWARE         :  FALCON_FW_18_06_00
*/
static const FW_THREAD_INFO  sip6FirmwareThreadIdToInstructionPointer[MAX_THREADS_CNS] =

{                                                                         /* skipCounter */   /* headerWindowAnchore*/   /*headerWindowSize*/
    /* 0*/  {0x400730  /* THR0_DoNothing(); */                          ,         1         ,       Outer_L2_Start     ,     EXPAND_32B       }
    /* 1*/ ,{0x400750  /* THR1_SRv6(); */                               ,         3         ,       Outer_L3_Start     ,     EXPAND_32B       }
    /* 2*/ ,{0x4007d0  /* THR2_SRv6_penultimateEndNode(); */            ,         5         ,       Outer_L3_Start     ,     EXPAND_32B       }
    /* 3*/ ,{0x400880  /* THR3_INT_Ingress_Switch_IPv6(); */            ,         9         ,       Outer_L3_Start     ,     EXPAND_64B       }
    /* 4*/ ,{0x400a60  /* THR4_INT_Ingress_Switch_IPv4(); */            ,        10         ,       Outer_L3_Start     ,     EXPAND_64B       }
    /* 5*/ ,{0x400c50  /* THR5_INT_Transit_Switch_IPv6(); */            ,        10         ,       Outer_L3_Start     ,     EXPAND_64B       }
    /* 6*/ ,{0x400e60  /* THR6_INT_Transit_Switch_IPv4(); */            ,        10         ,       Outer_L3_Start     ,     EXPAND_64B       }
    /* 7*/ ,{0x401080  /* THR7_IOAM_Ingress_Switch_IPv4(); */           ,        15         ,       Outer_L3_Start     ,     EXPAND_64B       }
    /* 8*/ ,{0x401320  /* THR8_IOAM_Ingress_Switch_IPv6(); */           ,        12         ,       Outer_L3_Start     ,     EXPAND_64B       }
    /* 9*/ ,{0x4015c0  /* THR9_IOAM_Transit_Switch_IPv4(); */           ,        11         ,       Outer_L3_Start     ,     EXPAND_64B       }
    /*10*/ ,{0x401860  /* THR10_IOAM_Transit_Switch_IPv6(); */          ,         9         ,       Outer_L3_Start     ,     EXPAND_64B       }
    /*11*/ ,{FW_INSTRUCTION_POINTER_NOT_VALID_CNS                       ,         0         ,             0            ,         0            }
    /*12*/ ,{FW_INSTRUCTION_POINTER_NOT_VALID_CNS                       ,         0         ,             0            ,         0            }
    /*13*/ ,{FW_INSTRUCTION_POINTER_NOT_VALID_CNS                       ,         0         ,             0            ,         0            }
    /*14*/ ,{FW_INSTRUCTION_POINTER_NOT_VALID_CNS                       ,         0         ,             0            ,         0            }
    /*15*/ ,{FW_INSTRUCTION_POINTER_NOT_VALID_CNS                       ,         0         ,             0            ,         0            }
    /*16*/ ,{FW_INSTRUCTION_POINTER_NOT_VALID_CNS                       ,         0         ,             0            ,         0            }
    /*17*/ ,{FW_INSTRUCTION_POINTER_NOT_VALID_CNS                       ,         0         ,             0            ,         0            }
    /*18*/ ,{FW_INSTRUCTION_POINTER_NOT_VALID_CNS                       ,         0         ,             0            ,         0            }
    /*19*/ ,{FW_INSTRUCTION_POINTER_NOT_VALID_CNS                       ,         0         ,             0            ,         0            }
    /*20*/ ,{FW_INSTRUCTION_POINTER_NOT_VALID_CNS                       ,         0         ,             0            ,         0            }
    /*21*/ ,{FW_INSTRUCTION_POINTER_NOT_VALID_CNS                       ,         0         ,             0            ,         0            }
    /*22*/ ,{FW_INSTRUCTION_POINTER_NOT_VALID_CNS                       ,         0         ,             0            ,         0            }
    /*23*/ ,{FW_INSTRUCTION_POINTER_NOT_VALID_CNS                       ,         0         ,             0            ,         0            }
    /*24*/ ,{FW_INSTRUCTION_POINTER_NOT_VALID_CNS                       ,         0         ,             0            ,         0            }
    /*25*/ ,{FW_INSTRUCTION_POINTER_NOT_VALID_CNS                       ,         0         ,             0            ,         0            }
    /*26*/ ,{FW_INSTRUCTION_POINTER_NOT_VALID_CNS                       ,         0         ,             0            ,         0            }
    /*27*/ ,{0x401b00  /* THR27_VariableCyclesLengthWithAcclCmd(); */   ,         2         ,       Outer_L2_Start     ,     EXPAND_32B       }
    /*28*/ ,{0x401b60  /* THR28_RemoveAddBytes(); */                    ,         4         ,       Outer_L2_Start     ,     EXPAND_32B       }
    /*29*/ ,{FW_INSTRUCTION_POINTER_NOT_VALID_CNS                       ,         0         ,             0            ,         0            }
    /*30*/ ,{FW_INSTRUCTION_POINTER_NOT_VALID_CNS                       ,         0         ,             0            ,         0            }
    /*31*/ ,{FW_INSTRUCTION_POINTER_NOT_VALID_CNS                       ,         0         ,             0            ,         0            }
} ;

#ifdef ASIC_SIMULATION

/* CPSS need to call to set the 'instruction_pointer' for the 'threadId' */
GT_STATUS simulationSip6FirmwareThreadIdToInstructionPointerSet
(
    IN GT_U32  threadId,/*0..31*/
    IN GT_U32 instruction_pointer
);

/**
* @internal ASIC_SIMULATION_firmwareInit function
* @endinternal
*
* @brief   ASIC_SIMULATION : init the ASIC_SIMULATION for FW addresses.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - on unknown threadId
*/
static GT_STATUS ASIC_SIMULATION_firmwareInit(void)
{
    GT_U32  threadId;
    GT_STATUS   rc;

    for(threadId = 0 ; threadId < MAX_THREADS_CNS; threadId++)
    {
        if(sip6FirmwareThreadIdToInstructionPointer[threadId].firmwareInstructionPointer == FW_INSTRUCTION_POINTER_NOT_VALID_CNS)
        {
            continue;
        }

        rc = simulationSip6FirmwareThreadIdToInstructionPointerSet(threadId,
            sip6FirmwareThreadIdToInstructionPointer[threadId].firmwareInstructionPointer);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "ASIC_SIMULATION failed to bind threadId[%d] with InstructionPointer[0x%x]",
                threadId,
                sip6FirmwareThreadIdToInstructionPointer[threadId].firmwareInstructionPointer);
        }

    }

    return GT_OK;
}

#endif /*ASIC_SIMULATION*/

/**
* @internal firmwareAddrValidityCheck function
* @endinternal
*
* @brief   Check Firmware addresses.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - on threadId that hold bad address.
*/
static GT_STATUS firmwareAddrValidityCheck(void)
{
    GT_U32  threadId;
    GT_U32  addr;

    for(threadId = 0 ; threadId < MAX_THREADS_CNS; threadId++)
    {
        addr = sip6FirmwareThreadIdToInstructionPointer[threadId].firmwareInstructionPointer;
        if(addr == FW_INSTRUCTION_POINTER_NOT_VALID_CNS)
        {
            continue;
        }

        if((addr & 0xFFFF0000) != FW_INSTRUCTION_DOMAIN_ADDR_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "firmware address for thread [%d] must be in [0x%x..0x%x]\n"
                "but got[0x%x] --> need to fix sip6FirmwareThreadIdToInstructionPointer[]",
                threadId ,
                FW_INSTRUCTION_DOMAIN_ADDR_CNS,
                FW_INSTRUCTION_DOMAIN_ADDR_MAX_CNS,
                addr
                );
        }
    }

    return GT_OK;
}

/**
* @internal firmwareDownload function
* @endinternal
*
* @brief   Download the PHA firmware to the device.
*         Need to download to IMEM of ALL PPGs (4 PPGs)
*         (each PPG hold 8 PPUs)
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  the device number
* @param[in] firmwareDataArr[]        - array of words of firmware data.
* @param[in] firmwareDataNumWords     - number of words in firmwareDataArr[].
*                                      must be multiple of 4.
*                                      meaning (firmwareDataNumWords % 4) must be 0
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad firmwareDataNumWords
*/
static GT_STATUS firmwareDownload
(
    IN GT_U8    devNum,
    IN const GT_U32     firmwareDataArr[],
    IN GT_U32           firmwareDataNumWords
)
{
    GT_STATUS   rc;
    GT_U32  ppg;
    GT_U32  regAddr;
    GT_U32  ii;
    GT_U32  entryWidthInWords = 4;
    GT_U32  entryWidthInBytes = 4*entryWidthInWords;

    if(firmwareDataNumWords % 4)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "firmware data number of words must me multiple of 4 but got [%d] words \n",
            firmwareDataNumWords);
    }

    if(firmwareDataNumWords > _4K)/* size of IMEM in words */
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "firmware data number of words is [%d] but must NOT be more than [%d] words \n",
            firmwareDataNumWords,
            _4K);
    }

    for(ppg = 0 ; ppg < PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.phaInfo.numOfPpg; ppg++)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PHA_MAC(devNum).PPG[ppg].PPG_IMEM_base_addr;

        for(ii = 0 ; ii < firmwareDataNumWords; ii += entryWidthInWords ,
            regAddr += entryWidthInBytes)
        {
            rc = prvCpssHwPpWriteRam(devNum,
                regAddr,entryWidthInWords,
                (GT_U32*)(&firmwareDataArr[ii]));/* casting that removes the 'const' */
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    /* set field <ppa_imem_hold_off> to :
        0x0 = No HoldOff; IMEM responds with data
    */
    regAddr = PRV_DXCH_REG1_UNIT_PHA_MAC(devNum).PPA.ppa_regs.PPACtrl;
    rc = prvCpssHwPpSetRegField(devNum,regAddr,0,1,
        0);/*0x0 = No HoldOff; IMEM responds with data*/
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}


/**
* @internal firmwareInit function
* @endinternal
*
* @brief   init the PHA related firmware configurations
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  the device number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
*
*/
static GT_STATUS firmwareInit
(
    IN GT_U8    devNum
)
{
    GT_STATUS   rc;
    GT_U32  regAddr;
    GT_U32  value;
    GT_U32  phaThreadId;
    CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC    commonInfo;
    CPSS_DXCH_PHA_THREAD_TYPE_ENT    extType;
    CPSS_DXCH_PHA_THREAD_INFO_UNT    extInfo;

    /****************************/
    /* check firmware addresses */
    /****************************/
    rc = firmwareAddrValidityCheck();
    if(rc != GT_OK)
    {
        return rc;
    }

    #ifdef ASIC_SIMULATION
    {
        /*****************************************/
        /* bind firmware addresses to simulation */
        /*****************************************/
        rc = ASIC_SIMULATION_firmwareInit();
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    #endif /*ASIC_SIMULATION*/

    /***********************************/
    /* download firmware to the device */
    /***********************************/
    rc = firmwareDownload(devNum,
        prv_cpss_dxch_falcon_ppa_fw_imem_data,           /*comes from FW_Vers/ppa_fw_imem_addr_data.h */
        PRV_CPSS_DXCH_FALCON_PPA_FW_IMAGE_SIZE);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Set Null Processing Instruction Pointer with 'do nothing' thread 'instruction pointer' */
    regAddr = PRV_DXCH_REG1_UNIT_PHA_MAC(devNum).pha_regs.PPAThreadConfigs.nullProcessingInstructionPointer;
    value = sip6FirmwareThreadIdToInstructionPointer[0].firmwareInstructionPointer &
        (~FW_INSTRUCTION_DOMAIN_ADDR_CNS);

    rc = prvCpssHwPpSetRegField(devNum,regAddr,0,16,value);
    if(rc != GT_OK)
    {
        return rc;
    }


    /****************************************************************/
    /* initialize all the PHA table entries to bind to 'do nothing' */
    /****************************************************************/
    {
        extType = CPSS_DXCH_PHA_THREAD_TYPE_UNUSED_E;
        extInfo.notNeeded = 0;/* dummy ... not used ... don't care */
        cpssOsMemSet(&commonInfo,0,sizeof(commonInfo));

        /* must set valid 'drop code' otherwise function will fail! */
        commonInfo.stallDropCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E;


        for(phaThreadId  = PRV_CPSS_DXCH_PHA_THREAD_ID_MIN_MAC;
            phaThreadId <= PRV_CPSS_DXCH_PHA_THREAD_ID_MAX_MAC;
            phaThreadId++)
        {
            rc = cpssDxChPhaThreadIdEntrySet(
                devNum,phaThreadId,&commonInfo,extType,&extInfo);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPhaFwVersionPrint function
* @endinternal
*
* @brief   debug function to print the PHA related firmware name,date and version
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  the device number
*
* @retval GT_OK                    - on success
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS   prvCpssDxChPhaFwVersionPrint
(
    IN GT_U8    devNum
)
{
    GT_STATUS   rc;
    GT_U32  regAddr;
    GT_U32  value1,value2;
    GT_U32  ppg,ppn;
    char    projName[5];
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E  | CPSS_CH1_DIAMOND_E | CPSS_CH2_E    | CPSS_CH3_E    | CPSS_XCAT_E |
        CPSS_LION_E | CPSS_XCAT2_E       | CPSS_XCAT3_E  | CPSS_LION2_E  |
        CPSS_BOBCAT2_E | CPSS_CAELUM_E   | CPSS_ALDRIN_E | CPSS_AC3X_E   |
        CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    /**************************************/
    /* check that the LIB was initialized */
    /**************************************/
    PRV_CPSS_DXCH_PHA_LIB_INIT_CHECK_MAC(devNum);

    ppg = 0;
    ppn = 0;

    regAddr = PRV_DXCH_REG1_UNIT_PHA_MAC(devNum).PPG[ppg].PPN[ppn].ppn_regs.fwName;
    rc = prvCpssHwPpReadRegister(devNum,regAddr,&value1);
    if(rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_DXCH_REG1_UNIT_PHA_MAC(devNum).PPG[ppg].PPN[ppn].ppn_regs.fwVersion;
    rc = prvCpssHwPpReadRegister(devNum,regAddr,&value2);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(value1 == 0x000badad || value2 == 0x000badad ||
       value1 == 0          || value2 == 0 )
    {
        /*****************************************/
        /* 0x00badadd -->                        */
        /* the GM not supports those registers ! */
        /* and the FW is not loaded in it        */
        /*****************************************/
        cpssOsPrintf("The PHA FW for dev[%d] : was NOT loaded (value1=[0x%8.8x] , value2=[0x%8.8x])!!! \n",
            devNum,value1,value2);

        /* do not fail the INIT */
        return GT_OK;
    }

#if (defined ASIC_SIMULATION && defined CPU_LE)
    /* the FW is in LE CPU , and so is the SW */
    projName[3] = (char)(value1 >> 24);
    projName[2] = (char)(value1 >> 16);
    projName[1] = (char)(value1 >>  8);
    projName[0] = (char)(value1 >>  0);
#else
    /* the FW is in BE CPU , and the SW will see it regardless to CPU endian */
    projName[0] = (char)(value1 >> 24);
    projName[1] = (char)(value1 >> 16);
    projName[2] = (char)(value1 >>  8);
    projName[3] = (char)(value1 >>  0);
#endif
    projName[4] = 0;

    cpssOsPrintf("The PHA FW for dev[%d] : [%s] , date[20%02x/%02x] version[%x] debug[%x] \n"
        , devNum
        , projName
        ,(value2 >> 24) & 0xFF/* year */
        ,(value2 >> 16) & 0xFF/* month*/
        ,(value2 >>  8) & 0xFF/* version */
        ,(value2 >>  0) & 0xFF/* debug*/
        );

    return GT_OK;
}


/**
* @internal firmwareInstructionPointerGet function
* @endinternal
*
* @brief   convert operationType to firmwareInstructionPointer and firmwareThreadId
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] operationType            -  the operation type
*
* @param[out] firmwareInstructionPointerPtr - (pointer to) the firmware Instruction Pointer.
* @param[out] firmwareThreadInfoPtr      - (pointer to) the firmware Thread info
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - on wrong parameter
*/
static GT_STATUS firmwareInstructionPointerGet
(
    IN CPSS_DXCH_PHA_THREAD_TYPE_ENT operationType,
    OUT FW_THREAD_INFO   *firmwareThreadInfoPtr
)
{
    FW_THREAD_ID      firmwareThreadId = 0;

    switch(operationType)
    {
        /** the threadId is not used */
        case CPSS_DXCH_PHA_THREAD_TYPE_UNUSED_E:
            firmwareThreadId = THR0_DoNothing;
            break;

        case CPSS_DXCH_PHA_THREAD_TYPE_IOAM_INGRESS_SWITCH_IPV4_E:
            firmwareThreadId = THR7_IOAM_Ingress_Switch_IPv4;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_IOAM_INGRESS_SWITCH_IPV6_E:
            firmwareThreadId = THR8_IOAM_Ingress_Switch_IPv6;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_IOAM_TRANSIT_SWITCH_IPV4_E:
            firmwareThreadId = THR9_IOAM_Transit_Switch_IPv4;
            break;
            /*...*/
        case CPSS_DXCH_PHA_THREAD_TYPE_IOAM_TRANSIT_SWITCH_IPV6_E:
            firmwareThreadId = THR10_IOAM_Transit_Switch_IPv6;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(operationType);
    }

    if(firmwareThreadId >= MAX_THREADS_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "firmwareThreadId[%d] expected to be less than[%d] ",
            firmwareThreadId , MAX_THREADS_CNS);
    }

    *firmwareThreadInfoPtr = sip6FirmwareThreadIdToInstructionPointer[firmwareThreadId];

    /* convert the 'firmwareThreadId' to the actual firmware address in memory */
    firmwareThreadInfoPtr->firmwareInstructionPointer =
        (sip6FirmwareThreadIdToInstructionPointer[firmwareThreadId].firmwareInstructionPointer) &
        (~FW_INSTRUCTION_DOMAIN_ADDR_CNS);


    return GT_OK;
}

/**
* @internal firmwareInstructionPointerConvert function
* @endinternal
*
* @brief   convert firmwareInstructionPointer to firmwareThreadId
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] firmwareInstructionPointer - the firmware Instruction Pointer.
*
* @param[out] firmwareThreadIdPtr      - (pointer to) the firmware Thread Id (0..31)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - on wrong parameter
*/
static GT_STATUS firmwareInstructionPointerConvert(
    IN  GT_U32   firmwareInstructionPointer,
    OUT FW_THREAD_ID   *firmwareThreadIdPtr
)
{
    FW_THREAD_ID  firmwareThreadId;

    firmwareInstructionPointer |= FW_INSTRUCTION_DOMAIN_ADDR_CNS;

    for(firmwareThreadId = 0 ; firmwareThreadId < MAX_THREADS_CNS; firmwareThreadId++)
    {
        if(firmwareInstructionPointer == sip6FirmwareThreadIdToInstructionPointer[firmwareThreadId].firmwareInstructionPointer)
        {
            *firmwareThreadIdPtr = firmwareThreadId;
            return GT_OK;
        }
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
        "firmwareInstructionPointer[0x%x] was not found as valid address for known firmware thread",
        firmwareInstructionPointer);
}

/**
* @internal prvCpssDxChPhaClockEnable function
* @endinternal
*
* @brief   enable/disable the clock to PPA in the PHA unit in the device.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum   - device number.
* @param[in] enable   - Enable/Disable the clock to PPA in the PHA unit in the device.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPhaClockEnable
(
    IN  GT_U8   devNum,
    IN GT_BOOL  enable
)
{
    GT_U32  regAddr;
    GT_U32  value;

    if(!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, "PHA : Only for sip6 and above");
    }

    regAddr = PRV_DXCH_REG1_UNIT_PHA_MAC(devNum).pha_regs.generalRegs.PHACtrl;
    /* <Disable PPA Clock> */
    value = 1 - BOOL2BIT_MAC(enable);

    return prvCpssHwPpSetRegField(devNum,regAddr,2,1,value);
}

/**
* @internal internal_cpssDxChPhaInit function
* @endinternal
*
* @brief   Init the PHA unit in the device.
*
*   NOTEs:
*   1. GT_NOT_INITIALIZED will be return for any 'PHA' lib APIs if called before
*       cpssDxChPhaInit(...)
*       (exclude cpssDxChPhaInit(...) itself)
*   2. GT_NOT_INITIALIZED will be return for EPCL APIs trying to enable PHA processing
*       if called before cpssDxChPhaInit(...)
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] packetOrderChangeEnable  - Enable/Disable the option for packet order
*                                      change between heavily processed flows and lightly processed flows
*                                      GT_TRUE  - packet order is not maintained
*                                      GT_FALSE  - packet order is maintained

*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_ALREADY_EXIST         - the library was already initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPhaInit
(
    IN  GT_U8   devNum,
    IN GT_BOOL  packetOrderChangeEnable
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E  | CPSS_CH1_DIAMOND_E | CPSS_CH2_E    | CPSS_CH3_E    | CPSS_XCAT_E |
        CPSS_LION_E | CPSS_XCAT2_E       | CPSS_XCAT3_E  | CPSS_LION2_E  |
        CPSS_BOBCAT2_E | CPSS_CAELUM_E   | CPSS_ALDRIN_E | CPSS_AC3X_E   |
        CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.phaInitDone == GT_TRUE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, "The library was already initialized");
    }

    /*
        1. Set < ppaClockEnable > enable
        2. Load the FW to ALL PPG (same FW in all PPGs)
        3. Bind the WM simulation with 'instruction pointer' to 'processing thread'
        4. Use the 'recycleEnable'to know if to set the <Skip Counter> with value 0
            (recycleEnable == GT_FALSE) or with 'real value' (that CPSS got from FW team)
        5. Not allow to set in any API the 'thread-id' != 0 if the 'pha init ' was not called
        6. Set Null Processing Instruction Pointer with 'do nothing' thread 'instruction pointer'
    */

    /* save the value */
    PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.packetOrderChangeEnable = packetOrderChangeEnable;
    /* state that the library was initialized */
    PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.phaInitDone = GT_TRUE;

    /* allow the PPA sub unit in the PHA to get clock
        (as it was disabled during 'phase 1' init)*/
    rc = prvCpssDxChPhaClockEnable(devNum,GT_TRUE);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* init the FW related */
    rc = firmwareInit(devNum);
    if (rc != GT_OK)
    {
        return rc;
    }


    return GT_OK;
}

/**
* @internal cpssDxChPhaInit function
* @endinternal
*
* @brief   Init the PHA unit in the device.
*
*   NOTEs:
*   1. GT_NOT_INITIALIZED will be return for any 'PHA' lib APIs if called before
*       cpssDxChPhaInit(...)
*       (exclude cpssDxChPhaInit(...) itself)
*   2. GT_NOT_INITIALIZED will be return for EPCL APIs trying to enable PHA processing
*       if called before cpssDxChPhaInit(...)
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] packetOrderChangeEnable  - Enable/Disable the option for packet order
*                                      change between heavily processed flows and lightly processed flows
*                                      GT_TRUE  - packet order is not maintained
*                                      GT_FALSE  - packet order is maintained

*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_ALREADY_EXIST         - the library was already initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhaInit
(
    IN  GT_U8   devNum,
    IN GT_BOOL  packetOrderChangeEnable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPhaInit);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, packetOrderChangeEnable));

    rc = internal_cpssDxChPhaInit(devNum, packetOrderChangeEnable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, packetOrderChangeEnable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal buildIoamTemplateForIngressSwitch function
* @endinternal
*
* @brief   check that IOAM For Ingress Switch parameters are valid.
*         haEntry : build 4 words of IOAM header from the info
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; .
*
* @param[in] ioamIngressSwitchPtr  - (pointer to) the IOAM header info.
*
* @param[out] dataPtr[4]           - the 4 words that hold the needed IOAM header format
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - on out of range parameter
*/
static GT_STATUS buildIoamTemplateForIngressSwitch
(
    IN  CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_STC *ioamIngressSwitchPtr,
    OUT GT_U32                 dataPtr[/*4*/]
)
{
    CPSS_DATA_CHECK_MAX_MAC(ioamIngressSwitchPtr->IOAM_Trace_Type , BIT_16 );
    CPSS_DATA_CHECK_MAX_MAC(ioamIngressSwitchPtr->Maximum_Length  ,  BIT_8 );
    CPSS_DATA_CHECK_MAX_MAC(ioamIngressSwitchPtr->Flags           ,  BIT_8 );

    CPSS_DATA_CHECK_MAX_MAC(ioamIngressSwitchPtr->Hop_Lim         ,  BIT_8 );
    CPSS_DATA_CHECK_MAX_MAC(ioamIngressSwitchPtr->node_id         , BIT_24 );

    CPSS_DATA_CHECK_MAX_MAC(ioamIngressSwitchPtr->Type1           ,  BIT_8 );
    CPSS_DATA_CHECK_MAX_MAC(ioamIngressSwitchPtr->IOAM_HDR_len1   ,  BIT_8 );
    CPSS_DATA_CHECK_MAX_MAC(ioamIngressSwitchPtr->Reserved1       ,  BIT_8 );
    CPSS_DATA_CHECK_MAX_MAC(ioamIngressSwitchPtr->Next_Protocol1  ,  BIT_8 );

    CPSS_DATA_CHECK_MAX_MAC(ioamIngressSwitchPtr->Type2           ,  BIT_8 );
    CPSS_DATA_CHECK_MAX_MAC(ioamIngressSwitchPtr->IOAM_HDR_len2   ,  BIT_8 );
    CPSS_DATA_CHECK_MAX_MAC(ioamIngressSwitchPtr->Reserved2       ,  BIT_8 );
    CPSS_DATA_CHECK_MAX_MAC(ioamIngressSwitchPtr->Next_Protocol2  ,  BIT_8 );

    dataPtr[0] =  ioamIngressSwitchPtr->IOAM_Trace_Type  << 16 |
                  ioamIngressSwitchPtr->Maximum_Length   <<  8 |
                  ioamIngressSwitchPtr->Flags;

    dataPtr[1] =  ioamIngressSwitchPtr->Hop_Lim          << 24 |
                  ioamIngressSwitchPtr->node_id;

    dataPtr[2] =  ioamIngressSwitchPtr->Type1           << 24 |
                  ioamIngressSwitchPtr->IOAM_HDR_len1   << 16 |
                  ioamIngressSwitchPtr->Reserved1       <<  8 |
                  ioamIngressSwitchPtr->Next_Protocol1;

    dataPtr[3] =  ioamIngressSwitchPtr->Type2           << 24 |
                  ioamIngressSwitchPtr->IOAM_HDR_len2   << 16 |
                  ioamIngressSwitchPtr->Reserved2       <<  8 |
                  ioamIngressSwitchPtr->Next_Protocol2;

    return GT_OK;
}

/**
* @internal parseIoamTemplateForIngressSwitch function
* @endinternal
*
* @brief   parse 2 words of ETAG from haEntry to IOAM For Ingress Switch parameters
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; .
*
* @param[in] dataPtr[4]           - the 4 words that hold the needed IOAM header format
*
* @param[out] ioamIngressSwitchPtr  - (pointer to) the IOAM header info.
*
* @retval GT_OK                    - on success
*/
static GT_STATUS parseIoamTemplateForIngressSwitch(
    IN GT_U32   dataPtr[/*4*/],
    OUT CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_STC *ioamIngressSwitchPtr
)
{
    ioamIngressSwitchPtr->IOAM_Trace_Type = U32_GET_FIELD_MAC(dataPtr[0],16,16);
    ioamIngressSwitchPtr->Maximum_Length  = U32_GET_FIELD_MAC(dataPtr[0], 8, 8);
    ioamIngressSwitchPtr->Flags           = U32_GET_FIELD_MAC(dataPtr[0], 0, 8);

    ioamIngressSwitchPtr->Hop_Lim         = U32_GET_FIELD_MAC(dataPtr[1],24, 8);
    ioamIngressSwitchPtr->node_id         = U32_GET_FIELD_MAC(dataPtr[1], 0,24);

    ioamIngressSwitchPtr->Type1           = U32_GET_FIELD_MAC(dataPtr[2],24, 8);
    ioamIngressSwitchPtr->IOAM_HDR_len1   = U32_GET_FIELD_MAC(dataPtr[2],16, 8);
    ioamIngressSwitchPtr->Reserved1       = U32_GET_FIELD_MAC(dataPtr[2], 8, 8);
    ioamIngressSwitchPtr->Next_Protocol1  = U32_GET_FIELD_MAC(dataPtr[2], 0, 8);

    ioamIngressSwitchPtr->Type2           = U32_GET_FIELD_MAC(dataPtr[3],24, 8);
    ioamIngressSwitchPtr->IOAM_HDR_len2   = U32_GET_FIELD_MAC(dataPtr[3],16, 8);
    ioamIngressSwitchPtr->Reserved2       = U32_GET_FIELD_MAC(dataPtr[3], 8, 8);
    ioamIngressSwitchPtr->Next_Protocol2  = U32_GET_FIELD_MAC(dataPtr[3], 0, 8);

    return GT_OK;
}


/**
* @internal buildIoamTemplateForTransitSwitch function
* @endinternal
*
* @brief   check that IOAM For Transit Switch parameters are valid.
*         haEntry : build 1 word of IOAM header from the info
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; .
*
* @param[in] ioamTransitSwitchPtr  - (pointer to) the IOAM header info.
*
* @param[out] dataPtr[1]           - the 1 word that hold the needed IOAM header format
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - on out of range parameter
*/
static GT_STATUS buildIoamTemplateForTransitSwitch
(
    IN  CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_STC *ioamTransitSwitchPtr,
    OUT GT_U32                 dataPtr[/*1*/]
)
{

    CPSS_DATA_CHECK_MAX_MAC(ioamTransitSwitchPtr->node_id         , BIT_24 );

    dataPtr[0] =  /* 8 bits of 'Hop_Lim' are 'ignored' and 'ttl' is used by the device */
                  ioamTransitSwitchPtr->node_id;

    return GT_OK;
}

/**
* @internal parseIoamTemplateForTransitSwitch function
* @endinternal
*
* @brief   parse 2 words of ETAG from haEntry to IOAM For Transit Switch parameters
*         haEntry : build 1 word of IOAM header from the info
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; .
*
* @param[in] dataPtr[1]           - the 1 word that hold the needed IOAM header format
*
* @param[out] ioamTransitSwitchPtr  - (pointer to) the IOAM header info.
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - on out of range parameter
*/
static GT_STATUS parseIoamTemplateForTransitSwitch
(
    IN  GT_U32                 dataPtr[/*1*/],
    OUT CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_STC *ioamTransitSwitchPtr
)
{
    /* 8 bits of 'Hop_Lim' are 'ignored' and 'ttl' is used by the device */
    ioamTransitSwitchPtr->node_id         = U32_GET_FIELD_MAC(dataPtr[0], 0,24);

    return GT_OK;
}


/**
* @internal internal_cpssDxChPhaThreadIdEntrySet function
* @endinternal
*
* @brief   Set the entry in the PHA Thread-Id table.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; .
*
* @param[in] devNum                   - device number.
* @param[in] phaThreadId              - the thread-Id.
*                                      (APPLICABLE RANGE: 1..255)
* @param[in] commonInfoPtr            - (pointer to) the common information needed for this threadId.
* @param[in] extType                  - the type of operation that this entry need to do.
*                                      NOTE: this is the type of the firmware thread that should handle the egress packet.
* @param[in] extInfoPtr               - (pointer to) union of operation information related to the 'operationType'
*                                      NOTE: this is the template that is needed by the firmware thread
*                                      that should handle the egress packet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong phaThreadId number or device or extType
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPhaThreadIdEntrySet
(
    IN GT_U8                devNum,
    IN GT_U32               phaThreadId,
    IN CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC *commonInfoPtr,
    IN CPSS_DXCH_PHA_THREAD_TYPE_ENT    extType,
    IN CPSS_DXCH_PHA_THREAD_INFO_UNT    *extInfoPtr
)
{
    GT_STATUS rc;
    FW_THREAD_INFO  firmwareThreadInfo;/* the firmware thread info */
    GT_U32  PPAThreadsConf1Word = 0;/* table 1 :  23 bits */
    GT_U32  PPAThreadsConf2Words[5];/* table 2 : 149 bits */
    GT_U32  hw_busyStallMode;/*hw value for <busyStallMode> */
    GT_U32  hw_skipCounter;  /*hw value for <skipCounter>   */
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT dsaCpuCode;  /* DSA code */
    GT_U32  templateArr[4];/* 4 words of Template */
    CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_STC  *ioamIngressSwitchPtr;
    CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_STC  *ioamTransitSwitchPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E  | CPSS_CH1_DIAMOND_E | CPSS_CH2_E    | CPSS_CH3_E    | CPSS_XCAT_E |
        CPSS_LION_E | CPSS_XCAT2_E       | CPSS_XCAT3_E  | CPSS_LION2_E  |
        CPSS_BOBCAT2_E | CPSS_CAELUM_E   | CPSS_ALDRIN_E | CPSS_AC3X_E   |
        CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    CPSS_NULL_PTR_CHECK_MAC(commonInfoPtr);
    CPSS_NULL_PTR_CHECK_MAC(extInfoPtr);

    /**************************************/
    /* check that the LIB was initialized */
    /**************************************/
    PRV_CPSS_DXCH_PHA_LIB_INIT_CHECK_MAC(devNum);

    /**************************************/
    /* start to :                         */
    /* check validity of INPUT parameters */
    /* and convert to HW values           */
    /**************************************/


    /* check the index to the table */
    PRV_CPSS_DXCH_PHA_THREAD_ID_CHECK_MAC(devNum,phaThreadId);

    /* convert SW cpu code to HW cpu code */
    rc = prvCpssDxChNetIfCpuToDsaCode(commonInfoPtr->stallDropCode,&dsaCpuCode);
    if(rc != GT_OK)
    {
        return rc;
    }

    CPSS_DATA_CHECK_MAX_MAC(commonInfoPtr->statisticalProcessingFactor,BIT_8);

    switch(commonInfoPtr->busyStallMode)
    {
        case CPSS_DXCH_PHA_BUSY_STALL_MODE_PUSH_BACK_E:
            hw_busyStallMode = 0;
            break;
        case CPSS_DXCH_PHA_BUSY_STALL_MODE_DROP_E     :
            hw_busyStallMode = 1;
            break;
        case CPSS_DXCH_PHA_BUSY_STALL_MODE_ORDERED_BYPASS_E:
            hw_busyStallMode = 2;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(commonInfoPtr->busyStallMode);
    }

    cpssOsMemSet(PPAThreadsConf2Words,0,sizeof(PPAThreadsConf2Words));

    switch(extType)
    {
        /** the threadId is not used */
        case CPSS_DXCH_PHA_THREAD_TYPE_UNUSED_E:
            PPAThreadsConf2Words[3] = extInfoPtr->notNeeded;
            break;

        case CPSS_DXCH_PHA_THREAD_TYPE_IOAM_INGRESS_SWITCH_IPV4_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_IOAM_INGRESS_SWITCH_IPV6_E:
            if(extType == CPSS_DXCH_PHA_THREAD_TYPE_IOAM_INGRESS_SWITCH_IPV4_E)
            {
                ioamIngressSwitchPtr = &extInfoPtr->ioamIngressSwitchIpv4;
            }
            else
            {
                ioamIngressSwitchPtr = &extInfoPtr->ioamIngressSwitchIpv6;
            }
            /* check and build specific info */
            rc = buildIoamTemplateForIngressSwitch(ioamIngressSwitchPtr, &templateArr[0]);
            if(rc != GT_OK)
            {
                return rc;
            }
            /* swap the words for the firmware */
            PPAThreadsConf2Words[3] = templateArr[0];
            PPAThreadsConf2Words[2] = templateArr[1];
            PPAThreadsConf2Words[1] = templateArr[2];
            PPAThreadsConf2Words[0] = templateArr[3];
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_IOAM_TRANSIT_SWITCH_IPV4_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_IOAM_TRANSIT_SWITCH_IPV6_E:
            if(extType == CPSS_DXCH_PHA_THREAD_TYPE_IOAM_TRANSIT_SWITCH_IPV4_E)
            {
                ioamTransitSwitchPtr = &extInfoPtr->ioamTransitSwitchIpv4;
            }
            else
            {
                ioamTransitSwitchPtr = &extInfoPtr->ioamTransitSwitchIpv6;
            }
            /* check and build specific info */
            rc = buildIoamTemplateForTransitSwitch(ioamTransitSwitchPtr, &templateArr[0]);
            if(rc != GT_OK)
            {
                return rc;
            }
            /* swap the words for the firmware */
            PPAThreadsConf2Words[3] = templateArr[0];
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(extType);
    }

    /* get info related to the thread type */
    rc = firmwareInstructionPointerGet(
        extType,
        &firmwareThreadInfo);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.packetOrderChangeEnable == GT_TRUE)
    {
        hw_skipCounter = firmwareThreadInfo.skipCounter;
    }
    else
    {
        hw_skipCounter = 0;
    }

    /* do last parameters check */
    CPSS_DATA_CHECK_MAX_MAC(firmwareThreadInfo.firmwareInstructionPointer,BIT_16);
    CPSS_DATA_CHECK_MAX_MAC(hw_skipCounter,BIT_5);
    CPSS_DATA_CHECK_MAX_MAC(hw_busyStallMode,BIT_2);
    CPSS_DATA_CHECK_MAX_MAC(dsaCpuCode,BIT_8);
    CPSS_DATA_CHECK_MAX_MAC(firmwareThreadInfo.headerWindowAnchor,BIT_2);
    CPSS_DATA_CHECK_MAX_MAC(firmwareThreadInfo.headerWindowSize,BIT_2);

    PPAThreadsConf2Words[4] = firmwareThreadInfo.firmwareInstructionPointer |
                              hw_skipCounter << 16;


    /**************************************/
    /* ended to :                         */
    /* check validity of INPUT parameters */
    /* and convert to HW values           */
    /**************************************/

    /**************************************/
    /* start to :                         */
    /* write to the 2 HW tables           */
    /**************************************/

    /* set the entry in table '2' */
    rc = prvCpssDxChWriteTableEntry(devNum,CPSS_DXCH_SIP6_TABLE_PHA_PPA_THREADS_CONF_2_E,
        phaThreadId,&PPAThreadsConf2Words[0]);
    if(rc != GT_OK)
    {
        return rc;
    }

    U32_SET_FIELD_MAC(PPAThreadsConf1Word, 0,8,commonInfoPtr->statisticalProcessingFactor);
    U32_SET_FIELD_MAC(PPAThreadsConf1Word, 9,2,hw_busyStallMode);
    U32_SET_FIELD_MAC(PPAThreadsConf1Word,11,8,dsaCpuCode);
    U32_SET_FIELD_MAC(PPAThreadsConf1Word,19,2,firmwareThreadInfo.headerWindowAnchor);
    U32_SET_FIELD_MAC(PPAThreadsConf1Word,21,2,firmwareThreadInfo.headerWindowSize);

    /* set the entry in table '1' */
    rc = prvCpssDxChWriteTableEntry(devNum,CPSS_DXCH_SIP6_TABLE_PHA_PPA_THREADS_CONF_1_E,
        phaThreadId,&PPAThreadsConf1Word);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* save the thread type (per threadId) in DB , used by 'Get' API when a specific
        'instruction pointer' is represented by more than single 'SW thread type'.
    */
    PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.threadTypeArr[phaThreadId] = extType;

    return rc;
}

/**
* @internal cpssDxChPhaThreadIdEntrySet function
* @endinternal
*
* @brief   Set the entry in the PHA Thread-Id table.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; .
*
* @param[in] devNum                   - device number.
* @param[in] phaThreadId              - the thread-Id.
*                                      (APPLICABLE RANGE: 1..255)
* @param[in] commonInfoPtr            - (pointer to) the common information needed for this threadId.
* @param[in] extType                  - the type of operation that this entry need to do.
*                                      NOTE: this is the type of the firmware thread that should handle the egress packet.
* @param[in] extInfoPtr               - (pointer to) union of operation information related to the 'operationType'
*                                      NOTE: this is the template that is needed by the firmware thread
*                                      that should handle the egress packet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong phaThreadId number or device or extType
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhaThreadIdEntrySet
(
    IN GT_U8                devNum,
    IN GT_U32               phaThreadId,
    IN CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC *commonInfoPtr,
    IN CPSS_DXCH_PHA_THREAD_TYPE_ENT    extType,
    IN CPSS_DXCH_PHA_THREAD_INFO_UNT    *extInfoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPhaThreadIdEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, phaThreadId , commonInfoPtr , extType , extInfoPtr));

    rc = internal_cpssDxChPhaThreadIdEntrySet(devNum, phaThreadId , commonInfoPtr , extType , extInfoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, phaThreadId , commonInfoPtr , extType , extInfoPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPhaThreadIdEntryGet function
* @endinternal
*
* @brief   Get the entry in the PHA Thread-Id table.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; .
*
* @param[in] devNum                   - device number.
* @param[in] phaThreadId              - the thread-Id.
*                                      (APPLICABLE RANGE: 1..255)
* @param[out] commonInfoPtr            - (pointer to) the common information needed for this threadId.
* @param[out] extTypePtr               - (pointer to) the type of operation that this entry need to do.
*                                      NOTE: this is the type of the firmware thread that should handle the egress packet.
* @param[out] extInfoPtr               - (pointer to) union of operation information related to the 'operationType'
*                                      NOTE: this is the template that is needed by the firmware thread
*                                      that should handle the egress packet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong phaThreadId number or device or extType
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPhaThreadIdEntryGet
(
    IN GT_U8                devNum,
    IN GT_U32               phaThreadId,
    OUT CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC *commonInfoPtr,
    OUT CPSS_DXCH_PHA_THREAD_TYPE_ENT    *extTypePtr,
    OUT CPSS_DXCH_PHA_THREAD_INFO_UNT    *extInfoPtr
)
{
    GT_STATUS rc;
    FW_THREAD_ID  firmwareThreadId;/* the firmware thread Id */
    GT_U32  firmwareInstructionPointer;/* the firmware Instruction Pointer */
    GT_U32  PPAThreadsConf1Word;/* table 1 :  23 bits */
    GT_U32  PPAThreadsConf2Words[5];/* table 2 : 149 bits */
    GT_U32  hw_busyStallMode;/*hw value for <busyStallMode> */
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT dsaCpuCode;  /* DSA code */
    CPSS_DXCH_PHA_THREAD_TYPE_ENT  extType;
    GT_U32  templateArr[4];/* 4 words of Template */
    CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_STC  *ioamIngressSwitchPtr;
    CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_STC  *ioamTransitSwitchPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E  | CPSS_CH1_DIAMOND_E | CPSS_CH2_E    | CPSS_CH3_E    | CPSS_XCAT_E |
        CPSS_LION_E | CPSS_XCAT2_E       | CPSS_XCAT3_E  | CPSS_LION2_E  |
        CPSS_BOBCAT2_E | CPSS_CAELUM_E   | CPSS_ALDRIN_E | CPSS_AC3X_E   |
        CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    CPSS_NULL_PTR_CHECK_MAC(commonInfoPtr);
    CPSS_NULL_PTR_CHECK_MAC(extTypePtr);
    CPSS_NULL_PTR_CHECK_MAC(extInfoPtr);

    /**************************************/
    /* check that the LIB was initialized */
    /**************************************/
    PRV_CPSS_DXCH_PHA_LIB_INIT_CHECK_MAC(devNum);

    /* check the index to the table */
    PRV_CPSS_DXCH_PHA_THREAD_ID_CHECK_MAC(devNum,phaThreadId);

    /* reset the 'union' to allow application to compare 'set' values with 'get' values */
    cpssOsMemSet(extInfoPtr,0,sizeof(*extInfoPtr));


    /* get the entry from table '2' */
    rc = prvCpssDxChReadTableEntry(devNum,CPSS_DXCH_SIP6_TABLE_PHA_PPA_THREADS_CONF_2_E,
        phaThreadId,&PPAThreadsConf2Words[0]);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* get the entry from table '1' */
    rc = prvCpssDxChReadTableEntry(devNum,CPSS_DXCH_SIP6_TABLE_PHA_PPA_THREADS_CONF_1_E,
        phaThreadId,&PPAThreadsConf1Word);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* get the 'instruction pointer' */
    firmwareInstructionPointer = U32_GET_FIELD_MAC(PPAThreadsConf2Words[4],0,16);

    /* convert the 'instruction pointer' to 'firmware ThreadId' */
    rc = firmwareInstructionPointerConvert(
        firmwareInstructionPointer,
        &firmwareThreadId);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(firmwareThreadId >= MAX_THREADS_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "firmwareThreadId must be less than [%d] but got[%d]",
            MAX_THREADS_CNS,
            firmwareThreadId);
    }

    /**************************************/
    /* start to :                         */
    /* convert HW values to SW values     */
    /**************************************/

    commonInfoPtr->statisticalProcessingFactor = U32_GET_FIELD_MAC(PPAThreadsConf1Word, 0,8);

    hw_busyStallMode = U32_GET_FIELD_MAC(PPAThreadsConf1Word, 9,2);
    switch(hw_busyStallMode)
    {
        case 0:
            commonInfoPtr->busyStallMode = CPSS_DXCH_PHA_BUSY_STALL_MODE_PUSH_BACK_E;
            break;
        case 1:
            commonInfoPtr->busyStallMode = CPSS_DXCH_PHA_BUSY_STALL_MODE_DROP_E;
            break;
        case 2:
            commonInfoPtr->busyStallMode = CPSS_DXCH_PHA_BUSY_STALL_MODE_ORDERED_BYPASS_E;
            break;
        case 3:
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "unexpected value[%d] from HW for hw_busyStallMode",
                hw_busyStallMode);
    }

    dsaCpuCode = U32_GET_FIELD_MAC(PPAThreadsConf1Word,11,8);

    rc = prvCpssDxChNetIfDsaToCpuCode(dsaCpuCode,&commonInfoPtr->stallDropCode);
    if(rc != GT_OK)
    {
        return rc;
    }

    extType = PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.threadTypeArr[phaThreadId];
    *extTypePtr = extType;

    switch(extType)
    {
        case CPSS_DXCH_PHA_THREAD_TYPE_UNUSED_E:
            extInfoPtr->notNeeded = PPAThreadsConf2Words[3];
            break;

        /** the threadId used , with info according to type */
        case CPSS_DXCH_PHA_THREAD_TYPE_IOAM_INGRESS_SWITCH_IPV4_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_IOAM_INGRESS_SWITCH_IPV6_E:
            if(extType == CPSS_DXCH_PHA_THREAD_TYPE_IOAM_INGRESS_SWITCH_IPV4_E)
            {
                ioamIngressSwitchPtr = &extInfoPtr->ioamIngressSwitchIpv4;
            }
            else
            {
                ioamIngressSwitchPtr = &extInfoPtr->ioamIngressSwitchIpv6;
            }

            /* swap back the words */
            templateArr[0] = PPAThreadsConf2Words[3];
            templateArr[1] = PPAThreadsConf2Words[2];
            templateArr[2] = PPAThreadsConf2Words[1];
            templateArr[3] = PPAThreadsConf2Words[0];

            /* parse HW info to SW info */
            rc = parseIoamTemplateForIngressSwitch(&templateArr[0],ioamIngressSwitchPtr);
            if(rc != GT_OK)
            {
                return rc;
            }

            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_IOAM_TRANSIT_SWITCH_IPV4_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_IOAM_TRANSIT_SWITCH_IPV6_E:
            if(extType == CPSS_DXCH_PHA_THREAD_TYPE_IOAM_TRANSIT_SWITCH_IPV4_E)
            {
                ioamTransitSwitchPtr = &extInfoPtr->ioamTransitSwitchIpv4;
            }
            else
            {
                ioamTransitSwitchPtr = &extInfoPtr->ioamTransitSwitchIpv6;
            }

            /* swap back the words */
            templateArr[0] = PPAThreadsConf2Words[3];

            /* parse HW info to SW info */
            rc = parseIoamTemplateForTransitSwitch(&templateArr[0],ioamTransitSwitchPtr);
            if(rc != GT_OK)
            {
                return rc;
            }

            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "unexpected value[%d] from DB for 'thread type'",
                extType);
    }

    return GT_OK;

}

/**
* @internal cpssDxChPhaThreadIdEntryGet function
* @endinternal
*
* @brief   Get the entry in the PHA Thread-Id table.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; .
*
* @param[in] devNum                   - device number.
* @param[in] phaThreadId              - the thread-Id.
*                                      (APPLICABLE RANGE: 1..255)
* @param[out] commonInfoPtr            - (pointer to) the common information needed for this threadId.
* @param[out] extTypePtr               - (pointer to) the type of operation that this entry need to do.
*                                      NOTE: this is the type of the firmware thread that should handle the egress packet.
* @param[out] extInfoPtr               - (pointer to) union of operation information related to the 'operationType'
*                                      NOTE: this is the template that is needed by the firmware thread
*                                      that should handle the egress packet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong phaThreadId number or device or extType
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhaThreadIdEntryGet
(
    IN GT_U8                devNum,
    IN GT_U32               phaThreadId,
    OUT CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC *commonInfoPtr,
    OUT CPSS_DXCH_PHA_THREAD_TYPE_ENT    *extTypePtr,
    OUT CPSS_DXCH_PHA_THREAD_INFO_UNT    *extInfoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPhaThreadIdEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, phaThreadId , commonInfoPtr , extTypePtr , extInfoPtr));

    rc = internal_cpssDxChPhaThreadIdEntryGet(devNum, phaThreadId , commonInfoPtr , extTypePtr , extInfoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, phaThreadId , commonInfoPtr , extTypePtr , extInfoPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPhaPortThreadIdSet function
* @endinternal
*
* @brief   Per target port ,set the associated thread-Id.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - the target port number
* @param[in] enable                   - enable/disable the use of threadId for the target port.
* @param[in] phaThreadId              - the associated thread-Id
*                                       NOTE: relevant only when enable = GT_TRUE
*                                      (APPLICABLE RANGE: 1..255)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or port or phaThreadId
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChPhaPortThreadIdSet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable,
    IN GT_U32       phaThreadId
)
{
    GT_U32  hw_phaThreadId;/* hw value for <phaThreadId> */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E  | CPSS_CH1_DIAMOND_E | CPSS_CH2_E    | CPSS_CH3_E    | CPSS_XCAT_E |
        CPSS_LION_E | CPSS_XCAT2_E       | CPSS_XCAT3_E  | CPSS_LION2_E  |
        CPSS_BOBCAT2_E | CPSS_CAELUM_E   | CPSS_ALDRIN_E | CPSS_AC3X_E   |
        CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

    /**************************************/
    /* check that the LIB was initialized */
    /**************************************/
    PRV_CPSS_DXCH_PHA_LIB_INIT_CHECK_MAC(devNum);

    if(enable == GT_TRUE)
    {
        /* check the thread-Id value */
        PRV_CPSS_DXCH_PHA_THREAD_ID_CHECK_MAC(devNum,phaThreadId);
        hw_phaThreadId = phaThreadId;
    }
    else
    {
        hw_phaThreadId = 0;
    }

    return prvCpssDxChWriteTableEntryField(devNum,
                                   CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
                                   portNum,
                                   PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                   SIP6_HA_EPORT_TABLE_1_FIELDS_PHA_THREAD_NUMBER_E, /* field name */
                                   PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                   hw_phaThreadId);
}

/**
* @internal cpssDxChPhaPortThreadIdSet function
* @endinternal
*
* @brief   Per target port ,set the associated thread-Id.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - the target port number
* @param[in] enable                   - enable/disable the use of threadId for the target port.
* @param[in] phaThreadId              - the associated thread-Id
*                                       NOTE: relevant only when enable = GT_TRUE
*                                      (APPLICABLE RANGE: 1..255)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or port or phaThreadId
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPhaPortThreadIdSet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable,
    IN GT_U32       phaThreadId
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPhaPortThreadIdSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum , enable , phaThreadId));

    rc = internal_cpssDxChPhaPortThreadIdSet(devNum, portNum , enable , phaThreadId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum , enable , phaThreadId));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPhaPortThreadIdGet function
* @endinternal
*
* @brief   Per target port ,get the associated thread-Id.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - the target port number
* @param[in] enablePtr                - (pointer to)enable/disable the use of threadId for the target port.
* @param[in] phaThreadIdPtr           - (pointer to)the associated thread-Id
*                                       NOTE: relevant only when enable = GT_TRUE
*                                      (APPLICABLE RANGE: 1..255)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChPhaPortThreadIdGet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    OUT GT_BOOL     *enablePtr,
    OUT GT_U32      *phaThreadIdPtr
)
{
    GT_STATUS   rc;
    GT_U32  hw_phaThreadId;/* hw value for <phaThreadId> */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E  | CPSS_CH1_DIAMOND_E | CPSS_CH2_E    | CPSS_CH3_E    | CPSS_XCAT_E |
        CPSS_LION_E | CPSS_XCAT2_E       | CPSS_XCAT3_E  | CPSS_LION2_E  |
        CPSS_BOBCAT2_E | CPSS_CAELUM_E   | CPSS_ALDRIN_E | CPSS_AC3X_E   |
        CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    CPSS_NULL_PTR_CHECK_MAC(phaThreadIdPtr);

    /**************************************/
    /* check that the LIB was initialized */
    /**************************************/
    PRV_CPSS_DXCH_PHA_LIB_INIT_CHECK_MAC(devNum);

    rc = prvCpssDxChReadTableEntryField(devNum,
                                   CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
                                   portNum,
                                   PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                   SIP6_HA_EPORT_TABLE_1_FIELDS_PHA_THREAD_NUMBER_E, /* field name */
                                   PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                   &hw_phaThreadId);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(hw_phaThreadId == 0)
    {
        *enablePtr = GT_FALSE;
        *phaThreadIdPtr = 0;/* 'dont care' */
    }
    else
    {
        *enablePtr = GT_TRUE;
        *phaThreadIdPtr = hw_phaThreadId;
    }

    return GT_OK;
}

/**
* @internal cpssDxChPhaPortThreadIdGet function
* @endinternal
*
* @brief   Per target port ,get the associated thread-Id.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - the target port number
* @param[in] enablePtr                - (pointer to)enable/disable the use of threadId for the target port.
* @param[in] phaThreadIdPtr           - (pointer to)the associated thread-Id
*                                       NOTE: relevant only when enable = GT_TRUE
*                                      (APPLICABLE RANGE: 1..255)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPhaPortThreadIdGet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    OUT GT_BOOL     *enablePtr,
    OUT GT_U32      *phaThreadIdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPhaPortThreadIdGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum , enablePtr , phaThreadIdPtr));

    rc = internal_cpssDxChPhaPortThreadIdGet(devNum, portNum , enablePtr , phaThreadIdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum , enablePtr , phaThreadIdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPhaSourcePortEntrySet function
* @endinternal
*
* @brief   Set per source physical port the entry.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] portNum               - source physical port number.
* @param[in] infoType              - the type of source port info.
* @param[in] portInfoPtr           - (pointer to) the entry info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or infoType
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPhaSourcePortEntrySet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT  infoType,
    IN CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT  *portInfoPtr
)
{
    GT_U32  hwValue; /* value to write to HW */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E  | CPSS_CH1_DIAMOND_E | CPSS_CH2_E    | CPSS_CH3_E    | CPSS_XCAT_E |
        CPSS_LION_E | CPSS_XCAT2_E       | CPSS_XCAT3_E  | CPSS_LION2_E  |
        CPSS_BOBCAT2_E | CPSS_CAELUM_E   | CPSS_ALDRIN_E | CPSS_AC3X_E   |
        CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(portInfoPtr);

    /**************************************/
    /* check that the LIB was initialized */
    /**************************************/
    PRV_CPSS_DXCH_PHA_LIB_INIT_CHECK_MAC(devNum);

    switch(infoType)
    {
        case CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNUSED_E:
            hwValue = *portInfoPtr;/* dummy */
            break;
        case CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_A_E:
            hwValue = *portInfoPtr;/* dummy */
            break;
        case CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_B_E:
            hwValue = *portInfoPtr;/* dummy */
            break;
        case CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_C_E:
            hwValue = *portInfoPtr;/* dummy */
            break;
        /*...*/
        case CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_Z_E:
            hwValue = *portInfoPtr;/* dummy */
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(infoType);
    }

    /* save the port type (per port) in DB ,  used by 'Get' API to
        know how to parse the HW info..
    */
    PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.sourcePortEntryTypeArr[portNum] = infoType;

    /* write the info to the table */
    return prvCpssDxChWriteTableEntry(devNum,
        CPSS_DXCH_SIP6_TABLE_PHA_SOURCE_PHYSICAL_PORT_E,
        portNum,
        &hwValue);
}

/**
* @internal cpssDxChPhaSourcePortEntrySet function
* @endinternal
*
* @brief   Set per source physical port the entry.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] portNum               - source physical port number.
* @param[in] infoType              - the type of source port info.
* @param[in] portInfoPtr           - (pointer to) the entry info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or infoType
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhaSourcePortEntrySet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT  infoType,
    IN CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT  *portInfoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPhaSourcePortEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum , infoType , portInfoPtr));

    rc = internal_cpssDxChPhaSourcePortEntrySet(devNum, portNum , infoType , portInfoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum , infoType , portInfoPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPhaSourcePortEntryGet function
* @endinternal
*
* @brief   Get per source physical port the entry.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] portNum               - source physical port number.
* @param[out] infoTypePtr          - (pointer to) the type of source port info.
* @param[out] portInfoPtr          - (pointer to) the entry info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPhaSourcePortEntryGet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT  *infoTypePtr,
    OUT CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT  *portInfoPtr
)
{
    GT_STATUS   rc;
    GT_U32  hwValue; /* value read from HW */
    CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT infoType;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E  | CPSS_CH1_DIAMOND_E | CPSS_CH2_E    | CPSS_CH3_E    | CPSS_XCAT_E |
        CPSS_LION_E | CPSS_XCAT2_E       | CPSS_XCAT3_E  | CPSS_LION2_E  |
        CPSS_BOBCAT2_E | CPSS_CAELUM_E   | CPSS_ALDRIN_E | CPSS_AC3X_E   |
        CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(infoTypePtr);
    CPSS_NULL_PTR_CHECK_MAC(portInfoPtr);

    /**************************************/
    /* check that the LIB was initialized */
    /**************************************/
    PRV_CPSS_DXCH_PHA_LIB_INIT_CHECK_MAC(devNum);

    /* read the info from the table */
    rc =  prvCpssDxChReadTableEntry(devNum,
        CPSS_DXCH_SIP6_TABLE_PHA_SOURCE_PHYSICAL_PORT_E,
        portNum,
        &hwValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* get the entry format from the DB */
    infoType = PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.sourcePortEntryTypeArr[portNum];
    *infoTypePtr = infoType;

    switch(infoType)
    {
        case CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNUSED_E:
            *portInfoPtr = hwValue;/*dummy*/
            break;
        case CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_A_E:
            *portInfoPtr = hwValue;/*dummy*/
            break;
        case CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_B_E:
            *portInfoPtr = hwValue;/*dummy*/
            break;
        case CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_C_E:
            *portInfoPtr = hwValue;/*dummy*/
            break;
        /*...*/
        case CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_Z_E:
            *portInfoPtr = hwValue;/*dummy*/
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "unexpected value[%d] from DB for 'source port type'",
                infoType);
    }

    return GT_OK;
}

/**
* @internal cpssDxChPhaSourcePortEntryGet function
* @endinternal
*
* @brief   Get per source physical port the entry.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] portNum               - source physical port number.
* @param[out] infoTypePtr          - (pointer to) the type of source port info.
* @param[out] portInfoPtr          - (pointer to) the entry info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhaSourcePortEntryGet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT  *infoTypePtr,
    OUT CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT  *portInfoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPhaSourcePortEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum , infoTypePtr , portInfoPtr));

    rc = internal_cpssDxChPhaSourcePortEntryGet(devNum, portNum , infoTypePtr , portInfoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum , infoTypePtr , portInfoPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPhaTargetPortEntrySet function
* @endinternal
*
* @brief   Set per target physical port the entry.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] portNum               - target physical port number.
* @param[in] infoType              - the type of target port info.
* @param[in] portInfoPtr           - (pointer to) the entry info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or infoType
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPhaTargetPortEntrySet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT  infoType,
    IN CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT  *portInfoPtr
)
{
    GT_U32  hwValue; /* value to write to HW */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E  | CPSS_CH1_DIAMOND_E | CPSS_CH2_E    | CPSS_CH3_E    | CPSS_XCAT_E |
        CPSS_LION_E | CPSS_XCAT2_E       | CPSS_XCAT3_E  | CPSS_LION2_E  |
        CPSS_BOBCAT2_E | CPSS_CAELUM_E   | CPSS_ALDRIN_E | CPSS_AC3X_E   |
        CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(portInfoPtr);

    /**************************************/
    /* check that the LIB was initialized */
    /**************************************/
    PRV_CPSS_DXCH_PHA_LIB_INIT_CHECK_MAC(devNum);

    switch(infoType)
    {
        case CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNUSED_E:
            hwValue = *portInfoPtr;/* dummy */
            break;
        case CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_A_E:
            hwValue = *portInfoPtr;/* dummy */
            break;
        case CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_B_E:
            hwValue = *portInfoPtr;/* dummy */
            break;
        case CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_C_E:
            hwValue = *portInfoPtr;/* dummy */
            break;
        /*...*/
        case CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_Z_E:
            hwValue = *portInfoPtr;/* dummy */
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(infoType);
    }

    /* save the port type (per port) in DB ,  used by 'Get' API to
        know how to parse the HW info..
    */
    PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.targetPortEntryTypeArr[portNum] = infoType;

    /* write the info to the table */
    return prvCpssDxChWriteTableEntry(devNum,
        CPSS_DXCH_SIP6_TABLE_PHA_TARGET_PHYSICAL_PORT_E,
        portNum,
        &hwValue);
}

/**
* @internal cpssDxChPhaTargetPortEntrySet function
* @endinternal
*
* @brief   Set per target physical port the entry.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] portNum               - target physical port number.
* @param[in] infoType              - the type of target port info.
* @param[in] portInfoPtr           - (pointer to) the entry info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or infoType
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhaTargetPortEntrySet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT  infoType,
    IN CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT  *portInfoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPhaTargetPortEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum , infoType , portInfoPtr));

    rc = internal_cpssDxChPhaTargetPortEntrySet(devNum, portNum , infoType , portInfoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum , infoType , portInfoPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPhaTargetPortEntryGet function
* @endinternal
*
* @brief   Get per target physical port the entry.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] portNum               - target physical port number.
* @param[out] infoTypePtr          - the type of target port info.
* @param[out] portInfoPtr          - (pointer to) the entry info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPhaTargetPortEntryGet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT  *infoTypePtr,
    OUT CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT  *portInfoPtr
)
{
    GT_STATUS   rc;
    GT_U32  hwValue; /* value read from HW */
    CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT infoType;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E  | CPSS_CH1_DIAMOND_E | CPSS_CH2_E    | CPSS_CH3_E    | CPSS_XCAT_E |
        CPSS_LION_E | CPSS_XCAT2_E       | CPSS_XCAT3_E  | CPSS_LION2_E  |
        CPSS_BOBCAT2_E | CPSS_CAELUM_E   | CPSS_ALDRIN_E | CPSS_AC3X_E   |
        CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(infoTypePtr);
    CPSS_NULL_PTR_CHECK_MAC(portInfoPtr);

    /**************************************/
    /* check that the LIB was initialized */
    /**************************************/
    PRV_CPSS_DXCH_PHA_LIB_INIT_CHECK_MAC(devNum);

    /* read the info from the table */
    rc =  prvCpssDxChReadTableEntry(devNum,
        CPSS_DXCH_SIP6_TABLE_PHA_TARGET_PHYSICAL_PORT_E,
        portNum,
        &hwValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* get the entry format from the DB */
    infoType = PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.targetPortEntryTypeArr[portNum];
    *infoTypePtr = infoType;

    switch(infoType)
    {
        case CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNUSED_E:
            *portInfoPtr = hwValue;/*dummy*/
            break;
        case CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_A_E:
            *portInfoPtr = hwValue;/*dummy*/
            break;
        case CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_B_E:
            *portInfoPtr = hwValue;/*dummy*/
            break;
        case CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_C_E:
            *portInfoPtr = hwValue;/*dummy*/
            break;
        /*...*/
        case CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_Z_E:
            *portInfoPtr = hwValue;/*dummy*/
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "unexpected value[%d] from DB for 'target port type'",
                infoType);
    }

    return GT_OK;
}

/**
* @internal cpssDxChPhaTargetPortEntryGet function
* @endinternal
*
* @brief   Get per target physical port the entry.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] portNum               - target physical port number.
* @param[out] infoTypePtr          - the type of target port info.
* @param[out] portInfoPtr          - (pointer to) the entry info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhaTargetPortEntryGet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT  *infoTypePtr,
    OUT CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT  *portInfoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPhaTargetPortEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum , infoTypePtr , portInfoPtr));

    rc = internal_cpssDxChPhaTargetPortEntryGet(devNum, portNum , infoTypePtr , portInfoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum , infoTypePtr , portInfoPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPhaHeaderModificationViolationInfoSet function
* @endinternal
*
* @brief   Set the packet Command and the Drop code for case of header modification
*           size violations of header pointers consistency checks
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum          - device number.
* @param[in] dropCode        - Drop code to attach to packets causing header modification
*                              size violations of header pointers consistency checks.
* @param[in] packetCommand   - Packet command to attach to packets violating header
*                              size checks or header pointer consistency checks
*                              Valid values : CPSS_PACKET_CMD_FORWARD_E /
*                                             CPSS_PACKET_CMD_DROP_HARD_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or dropCode or packetCommand
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPhaHeaderModificationViolationInfoSet
(
    IN  GT_U8                       devNum,
    IN  CPSS_NET_RX_CPU_CODE_ENT    dropCode,
    IN  CPSS_PACKET_CMD_ENT         packetCommand
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT dsaCpuCode;  /* DSA code */
    GT_U32  regAddr;
    GT_U32  hwValue; /* value to write to HW */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E  | CPSS_CH1_DIAMOND_E | CPSS_CH2_E    | CPSS_CH3_E    | CPSS_XCAT_E |
        CPSS_LION_E | CPSS_XCAT2_E       | CPSS_XCAT3_E  | CPSS_LION2_E  |
        CPSS_BOBCAT2_E | CPSS_CAELUM_E   | CPSS_ALDRIN_E | CPSS_AC3X_E   |
        CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    /**************************************/
    /* check that the LIB was initialized */
    /**************************************/
    PRV_CPSS_DXCH_PHA_LIB_INIT_CHECK_MAC(devNum);

    /* set bit 0 in the register */
    switch(packetCommand)
    {
        case CPSS_PACKET_CMD_FORWARD_E:
            hwValue = 0x0;
            break;
        case CPSS_PACKET_CMD_DROP_HARD_E:
            hwValue = 0x1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(packetCommand);
    }


    /* convert SW cpu code to HW cpu code */
    rc = prvCpssDxChNetIfCpuToDsaCode(dropCode,&dsaCpuCode);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* set bits 1..8 in the register */
    hwValue |= dsaCpuCode << 1;

    regAddr = PRV_DXCH_REG1_UNIT_PHA_MAC(devNum).pha_regs.
        errorsAndInterrupts.headerModificationViolationConfig;

    return prvCpssHwPpSetRegField(devNum,regAddr,0,9,hwValue);
}

/**
* @internal cpssDxChPhaHeaderModificationViolationInfoSet function
* @endinternal
*
* @brief   Set the packet Command and the Drop code for case of header modification
*           size violations of header pointers consistency checks
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum          - device number.
* @param[in] dropCode        - Drop code to attach to packets causing header modification
*                              size violations of header pointers consistency checks.
* @param[in] packetCommand   - Packet command to attach to packets violating header
*                              size checks or header pointer consistency checks
*                              Valid values : CPSS_PACKET_CMD_FORWARD_E /
*                                             CPSS_PACKET_CMD_DROP_HARD_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or dropCode or packetCommand
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhaHeaderModificationViolationInfoSet
(
    IN  GT_U8                       devNum,
    IN  CPSS_NET_RX_CPU_CODE_ENT    dropCode,
    IN  CPSS_PACKET_CMD_ENT         packetCommand
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPhaHeaderModificationViolationInfoSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, dropCode , packetCommand));

    rc = internal_cpssDxChPhaHeaderModificationViolationInfoSet(devNum, dropCode , packetCommand);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, dropCode , packetCommand));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPhaHeaderModificationViolationInfoGet function
* @endinternal
*
* @brief   Get the packet Command and the Drop code for case of header modification
*           size violations of header pointers consistency checks
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum          - device number.
* @param[out] dropCodePtr    - (pointer to) Drop code to attach to packets causing header modification
*                              size violations of header pointers consistency checks.
* @param[out] packetCommandPtr - (pointer to) Packet command to attach to packets violating header
*                              size checks or header pointer consistency checks
*                              Valid values : CPSS_PACKET_CMD_FORWARD_E /
*                                             CPSS_PACKET_CMD_DROP_HARD_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPhaHeaderModificationViolationInfoGet
(
    IN  GT_U8                       devNum,
    OUT CPSS_NET_RX_CPU_CODE_ENT    *dropCodePtr,
    OUT CPSS_PACKET_CMD_ENT         *packetCommandPtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT dsaCpuCode;  /* DSA code */
    GT_U32  regAddr;
    GT_U32  hwValue; /* value read from the HW */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E  | CPSS_CH1_DIAMOND_E | CPSS_CH2_E    | CPSS_CH3_E    | CPSS_XCAT_E |
        CPSS_LION_E | CPSS_XCAT2_E       | CPSS_XCAT3_E  | CPSS_LION2_E  |
        CPSS_BOBCAT2_E | CPSS_CAELUM_E   | CPSS_ALDRIN_E | CPSS_AC3X_E   |
        CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    CPSS_NULL_PTR_CHECK_MAC(dropCodePtr);
    CPSS_NULL_PTR_CHECK_MAC(packetCommandPtr);

    /**************************************/
    /* check that the LIB was initialized */
    /**************************************/
    PRV_CPSS_DXCH_PHA_LIB_INIT_CHECK_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_PHA_MAC(devNum).pha_regs.
        errorsAndInterrupts.headerModificationViolationConfig;

    rc = prvCpssHwPpGetRegField(devNum,regAddr,0,9,&hwValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* get bit 0 from the register */
    switch(hwValue & 0x1)
    {
        default:
        case 0x0:
            *packetCommandPtr = CPSS_PACKET_CMD_FORWARD_E;
            break;
        case 0x1:
            *packetCommandPtr = CPSS_PACKET_CMD_DROP_HARD_E;
            break;
    }

    /* get bits 1..8 from the register */
    dsaCpuCode = hwValue >> 1;

    /* convert HW cpu code to SW cpu code */
    rc = prvCpssDxChNetIfDsaToCpuCode(dsaCpuCode,dropCodePtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChPhaHeaderModificationViolationInfoGet function
* @endinternal
*
* @brief   Get the packet Command and the Drop code for case of header modification
*           size violations of header pointers consistency checks
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum          - device number.
* @param[out] dropCodePtr    - (pointer to) Drop code to attach to packets causing header modification
*                              size violations of header pointers consistency checks.
* @param[out] packetCommandPtr - (pointer to) Packet command to attach to packets violating header
*                              size checks or header pointer consistency checks
*                              Valid values : CPSS_PACKET_CMD_FORWARD_E /
*                                             CPSS_PACKET_CMD_DROP_HARD_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhaHeaderModificationViolationInfoGet
(
    IN  GT_U8                       devNum,
    OUT CPSS_NET_RX_CPU_CODE_ENT    *dropCodePtr,
    OUT CPSS_PACKET_CMD_ENT         *packetCommandPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPhaHeaderModificationViolationInfoGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, dropCodePtr , packetCommandPtr));

    rc = internal_cpssDxChPhaHeaderModificationViolationInfoGet(devNum, dropCodePtr , packetCommandPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, dropCodePtr , packetCommandPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPhaHeaderModificationViolationCapturedGet function
* @endinternal
*
* @brief  Get the thread ID of the first header size violating packet , and the violation type.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum          - device number.
* @param[out] capturedThreadIdPtr   - (pointer to) The thread Id of the first header size violating packet
* @param[out] violationTypePtr      - (pointer to) Type describes which exact header size violation occurred
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NO_MORE               - when no more modification violations to get info about
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPhaHeaderModificationViolationCapturedGet
(
    IN  GT_U8                       devNum,
    OUT GT_U32                     *capturedThreadIdPtr,
    OUT CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_ENT  *violationTypePtr
)
{
    GT_STATUS   rc;
    GT_U32  portGroupId; /*the port group Id - support multi-port-groups device */
    GT_U32  portGroupsBmp; /* port groups to query - support multi-port-groups device */
    GT_U32  regAddr;
    GT_U32  hwValue; /* value read from the HW */
    GT_BOOL didCoverOnAllInstances = GT_FALSE;
    GT_U32  first_portGroupId, next_portGroupId;
    GT_U32  hw_ThreadId;
    GT_U32  hw_reason;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E  | CPSS_CH1_DIAMOND_E | CPSS_CH2_E    | CPSS_CH3_E    | CPSS_XCAT_E |
        CPSS_LION_E | CPSS_XCAT2_E       | CPSS_XCAT3_E  | CPSS_LION2_E  |
        CPSS_BOBCAT2_E | CPSS_CAELUM_E   | CPSS_ALDRIN_E | CPSS_AC3X_E   |
        CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    CPSS_NULL_PTR_CHECK_MAC(capturedThreadIdPtr);
    CPSS_NULL_PTR_CHECK_MAC(violationTypePtr);

    /**************************************/
    /* check that the LIB was initialized */
    /**************************************/
    PRV_CPSS_DXCH_PHA_LIB_INIT_CHECK_MAC(devNum);

    first_portGroupId =
        PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.phaViolationCapturedNextPortGroupToServe;

    if(0 == PRV_CPSS_IS_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
    {
        portGroupId = first_portGroupId;
        didCoverOnAllInstances = GT_TRUE;/* single instance */
    }
    else
    {
        tryInfoOnNextPortGroup_lbl:
        portGroupId =
            PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.phaViolationCapturedNextPortGroupToServe;

        /* start with bmp of all active port groups */
        portGroupsBmp = PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.activePortGroupsBmp;


        /* look for port group to query */
        while(0 == (portGroupsBmp & (1 << portGroupId)))
        {
            if(portGroupId > PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.lastActivePortGroup)
            {
                portGroupId = PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.firstActivePortGroup;
                break;
            }

            portGroupId++;
        }

        next_portGroupId = portGroupId + 1;
        /* calc the 'next valid port group' ... to be saved at <phaViolationCapturedNextPortGroupToServe> */
        {
            /* look for port group to query */
            while(0 == (portGroupsBmp & (1 << next_portGroupId)))
            {
                if(next_portGroupId > PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.lastActivePortGroup)
                {
                    next_portGroupId = PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.firstActivePortGroup;
                    break;
                }

                next_portGroupId++;
            }
        }

        /* save the info about the next port group to be served */
        PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.phaViolationCapturedNextPortGroupToServe =
            next_portGroupId;

        if(first_portGroupId == next_portGroupId)
        {
            /* next one to serve is the one that we started with */
            didCoverOnAllInstances = GT_TRUE;/* indicate that no more instances to check */
        }

    }

    regAddr = PRV_DXCH_REG1_UNIT_PHA_MAC(devNum).pha_regs.
        errorsAndInterrupts.headerModificationViolationAttributes;

    rc = prvCpssHwPpPortGroupGetRegField(devNum,portGroupId,regAddr,0,10,&hwValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    hw_ThreadId =  hwValue >> 2;/* bits 2..9 */
    hw_reason   =  hwValue & 0x3;/*bits 0..1 */

    if(hw_ThreadId == 0) /* indication that there is no error in this instance */
    {
        if(didCoverOnAllInstances == GT_FALSE)
        {
            /* support multi instances of the PHA unit (one per port group) */
            goto tryInfoOnNextPortGroup_lbl;
        }

        *capturedThreadIdPtr = 0;  /* dummy */
        *violationTypePtr    = 0;  /* dummy */

        return /* not error for the ERROR LOG */ GT_NO_MORE;
    }

    *capturedThreadIdPtr = hw_ThreadId;

    switch(hw_reason)
    {
        case 0:
            *violationTypePtr = CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_FW_INC_OVERSIZE_E;
            break;
        case 1:
            *violationTypePtr = CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_HEADER_OVERSIZE_E;
            break;
        case 2:
            *violationTypePtr = CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_FW_DEC_VIOLATION_E;
            break;
        case 3:
        default:
            *violationTypePtr = CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_HEADER_UNDERSIZE_E;
            break;
    }

    return GT_OK;
}

/**
* @internal cpssDxChPhaHeaderModificationViolationCapturedGet function
* @endinternal
*
* @brief  Get the thread ID of the first header size violating packet , and the violation type.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum          - device number.
* @param[out] capturedThreadIdPtr   - (pointer to) The thread Id of the first header size violating packet
* @param[out] violationTypePtr      - (pointer to) Type describes which exact header size violation occurred
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NO_MORE               - when no more modification violations to get info about
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhaHeaderModificationViolationCapturedGet
(
    IN  GT_U8                       devNum,
    OUT GT_U32                     *capturedThreadIdPtr,
    OUT CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_ENT  *violationTypePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPhaHeaderModificationViolationCapturedGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, capturedThreadIdPtr , violationTypePtr));

    rc = internal_cpssDxChPhaHeaderModificationViolationCapturedGet(devNum, capturedThreadIdPtr , violationTypePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, capturedThreadIdPtr , violationTypePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPhaStatisticalProcessingCounterThreadIdSet function
* @endinternal
*
* @brief   set the PHA thread-Id to count it's statistical processing cancellations.
*           When zero, all PHA threads are counted (i.e. non-zero).
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] phaThreadId              - the associated thread-Id
*                                      (APPLICABLE RANGE: 0..255)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or phaThreadId
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChPhaStatisticalProcessingCounterThreadIdSet
(
    IN GT_U8        devNum,
    IN GT_U32       phaThreadId
)
{
    GT_U32  regAddr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E  | CPSS_CH1_DIAMOND_E | CPSS_CH2_E    | CPSS_CH3_E    | CPSS_XCAT_E |
        CPSS_LION_E | CPSS_XCAT2_E       | CPSS_XCAT3_E  | CPSS_LION2_E  |
        CPSS_BOBCAT2_E | CPSS_CAELUM_E   | CPSS_ALDRIN_E | CPSS_AC3X_E   |
        CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    /**************************************/
    /* check that the LIB was initialized */
    /**************************************/
    PRV_CPSS_DXCH_PHA_LIB_INIT_CHECK_MAC(devNum);

    if(phaThreadId == 0)
    {
        /* this is the only API that we supports '0' as input from the
           application about phaThreadId */
    }
    else
    {
        /* validate the phaThreadId */
        PRV_CPSS_DXCH_PHA_THREAD_ID_CHECK_MAC(devNum,phaThreadId);
    }

    regAddr = PRV_DXCH_REG1_UNIT_PHA_MAC(devNum).pha_regs.
        debugCounters.statisticalProcessingDebugCounterConfig;

    return prvCpssHwPpSetRegField(devNum,regAddr,0,8,phaThreadId);
}

/**
* @internal cpssDxChPhaStatisticalProcessingCounterThreadIdSet function
* @endinternal
*
* @brief   set the PHA thread-Id to count it's statistical processing cancellations.
*           When zero, all PHA threads are counted (i.e. non-zero).
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] phaThreadId              - the associated thread-Id
*                                      (APPLICABLE RANGE: 0..255)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or phaThreadId
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPhaStatisticalProcessingCounterThreadIdSet
(
    IN GT_U8        devNum,
    IN GT_U32       phaThreadId
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPhaStatisticalProcessingCounterThreadIdSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, phaThreadId));

    rc = internal_cpssDxChPhaStatisticalProcessingCounterThreadIdSet(devNum, phaThreadId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, phaThreadId));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPhaStatisticalProcessingCounterThreadIdGet function
* @endinternal
*
* @brief   get the PHA thread-Id to count it's statistical processing cancellations.
*           When zero, all PHA threads are counted (i.e. non-zero).
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[out] phaThreadIdPtr           - (pointer to) the associated thread-Id
*                                      (APPLICABLE RANGE: 0..255)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChPhaStatisticalProcessingCounterThreadIdGet
(
    IN  GT_U8        devNum,
    OUT GT_U32       *phaThreadIdPtr
)
{
    GT_U32  regAddr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E  | CPSS_CH1_DIAMOND_E | CPSS_CH2_E    | CPSS_CH3_E    | CPSS_XCAT_E |
        CPSS_LION_E | CPSS_XCAT2_E       | CPSS_XCAT3_E  | CPSS_LION2_E  |
        CPSS_BOBCAT2_E | CPSS_CAELUM_E   | CPSS_ALDRIN_E | CPSS_AC3X_E   |
        CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    CPSS_NULL_PTR_CHECK_MAC(phaThreadIdPtr);

    /**************************************/
    /* check that the LIB was initialized */
    /**************************************/
    PRV_CPSS_DXCH_PHA_LIB_INIT_CHECK_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_PHA_MAC(devNum).pha_regs.
        debugCounters.statisticalProcessingDebugCounterConfig;

    return prvCpssHwPpGetRegField(devNum,regAddr,0,8,phaThreadIdPtr);
}

/**
* @internal cpssDxChPhaStatisticalProcessingCounterThreadIdGet function
* @endinternal
*
* @brief   get the PHA thread-Id to count it's statistical processing cancellations.
*           When zero, all PHA threads are counted (i.e. non-zero).
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[out] phaThreadIdPtr           - (pointer to) the associated thread-Id
*                                      (APPLICABLE RANGE: 0..255)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPhaStatisticalProcessingCounterThreadIdGet
(
    IN  GT_U8        devNum,
    OUT GT_U32       *phaThreadIdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPhaStatisticalProcessingCounterThreadIdGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, phaThreadIdPtr));

    rc = internal_cpssDxChPhaStatisticalProcessingCounterThreadIdGet(devNum, phaThreadIdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, phaThreadIdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPhaStatisticalProcessingCounterGet function
* @endinternal
*
* @brief   Get the PHA counter value of statistical processing cancellations.
*           Note: the counter is cleared after read (by HW).
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[out] counterPtr              - (pointer)the counter
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChPhaStatisticalProcessingCounterGet
(
    IN GT_U8        devNum,
    OUT GT_U64       *counterPtr
)
{
    GT_U32  regAddr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E  | CPSS_CH1_DIAMOND_E | CPSS_CH2_E    | CPSS_CH3_E    | CPSS_XCAT_E |
        CPSS_LION_E | CPSS_XCAT2_E       | CPSS_XCAT3_E  | CPSS_LION2_E  |
        CPSS_BOBCAT2_E | CPSS_CAELUM_E   | CPSS_ALDRIN_E | CPSS_AC3X_E   |
        CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    CPSS_NULL_PTR_CHECK_MAC(counterPtr);

    /**************************************/
    /* check that the LIB was initialized */
    /**************************************/
    PRV_CPSS_DXCH_PHA_LIB_INIT_CHECK_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_PHA_MAC(devNum).pha_regs.
        debugCounters.statisticalProcessingDebugCounter;

    return prvCpssPortGroupsCounterSummary(devNum,regAddr,0,20,NULL,counterPtr);
}

/**
* @internal cpssDxChPhaStatisticalProcessingCounterGet function
* @endinternal
*
* @brief   Get the PHA counter value of statistical processing cancellations.
*           Note: the counter is cleared after read (by HW).
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[out] counterPtr              - (pointer)the counter
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPhaStatisticalProcessingCounterGet
(
    IN GT_U8        devNum,
    OUT GT_U64       *counterPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPhaStatisticalProcessingCounterGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, counterPtr));

    rc = internal_cpssDxChPhaStatisticalProcessingCounterGet(devNum, counterPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, counterPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

