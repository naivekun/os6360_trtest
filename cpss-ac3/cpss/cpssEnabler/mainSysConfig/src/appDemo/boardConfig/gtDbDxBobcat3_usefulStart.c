#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChRegsVer1.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortIfModeCfgBcat2Resource.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortIfModeCfgBobKResource.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortInterlaken.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortMapping.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortDynamicPizzaArbiter.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/cpssDxChPortDynamicPizzaArbiterWorkConserving.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/cpssDxChPortDynamicPAUnitBW.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortTxQHighSpeedPortsDrv.h>
#include <cpss/dxCh/dxChxGen/cscd/cpssDxChCscd.h>
#include <cpss/dxCh/dxChxGen/phy/cpssDxChPhySmi.h>
#include <appDemo/sysHwConfig/appDemoDb.h>
#include <appDemo/boardConfig/gtDbDxBobcat2Mappings.h>
#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <appDemo/boardConfig/gtDbDxBobcat2UtilsSimClock.h> 
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortTx.h>
#include <cpssDriver/pp/config/prvCpssDrvPpCfgInit.h>

#ifdef __cplusplus 
extern "C" {
#endif /* __cplusplus */

extern GT_STATUS cpssInitSystem
(
    IN  GT_U32  boardIdx, 
    IN  GT_U32  boardRevId,
    IN  GT_U32  reloadEeprom
);

extern GT_STATUS appDemoTraceHwAccessEnable
(
    IN GT_U8                                devNum,
    IN APP_DEMO_TRACE_HW_ACCESS_TYPE_ENT    accessType,
    IN GT_BOOL                              enable
);


extern GT_STATUS appDemoDbEntryAdd
(
    IN  GT_CHAR *namePtr,
    IN  GT_U32  value
);

extern GT_STATUS appDemoDebugDeviceIdSet
(
    IN GT_U8    devNum,
    IN GT_U32   deviceId
);

extern GT_STATUS appDemoBc2PortListInit
(
    IN GT_U8 dev,
    IN PortInitList_STC * portInitList,
    IN GT_BOOL            skipCheckEnable
);


extern GT_STATUS gtBobcat2PortPizzaArbiterIfStateDump
(
    IN  GT_U8  devNum,
    IN  GT_U32 portGroupId
);

extern GT_STATUS gtBobcat2PortPizzaArbiterIfUnitStateDump
(
    IN  GT_U8  devNum,
    IN  GT_U32 portGroupId,
    IN  CPSS_DXCH_PA_UNIT_ENT unit
);

extern GT_STATUS appDemoBobcat3PortMappingDump
(
    IN  GT_U8  dev
);


extern GT_STATUS appDemoBobcat3PortGlobalResourcesPrint
(
    IN GT_U8 dev
);

extern GT_STATUS appDemoBobcat3PortListResourcesPrint
(
    IN GT_U8 dev
);

extern GT_STATUS appDemoBobcat3PortPizzaArbiterResourcesPrint
(
    IN GT_U8 dev
);


extern GT_STATUS appDemoBobcat3PizzaArbiterDPDump
(
    IN GT_U8 dev,
    IN GT_U32 dataPathIdx
);


extern GT_STATUS appDemoPizzaArbiterUnitsDefPrint
(
    GT_U8 dev
);

extern GT_STATUS appDemoBobcat3PortLedConfigPrint
(
    IN GT_U8 dev
);




#ifdef __cplusplus
}
#endif /* __cplusplus */

#ifdef WIN32
    #include <stdio.h>
    #define cpssOsPrintf printf
#endif




GT_U32 bc3Start(GT_U32 cc)
{
    GT_STATUS rc;

    GT_U8 dev = 0;


    #ifdef ASIC_SIMULATION
        #ifndef GM_USED
            appDemoBc3SimCoreClockSet(dev,cc); 
        #else
            cc = cc;
        #endif
    #endif

    rc = cpssInitSystem(29,1,0);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = appDemoPizzaArbiterUnitsDefPrint(dev);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = appDemoBobcat3PortMappingDump(dev);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = appDemoBobcat3PortGlobalResourcesPrint(dev);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = appDemoBobcat3PortListResourcesPrint(dev);
    if (rc != GT_OK)
    {
        return rc;
    }


    rc = gtBobcat2PortPizzaArbiterIfStateDump(dev,0);
    if (rc != GT_OK)
    {
        return rc;
    }
/*
#ifdef WIN32
     PORT_MODE_SET_UT_DEBUG_PRINT_SET(1);
    cpssDxChPortModeSpeedSetUT();
#endif 
    */


    return GT_OK;
}

#if 0

+------+-----------------+---------------------+--------+--------+--------+
| Port |   mapping Type  | mac txq rxdma txdma | rx-dma |   txQ  | tx-dma |
|      |                 |                     | dp loc | dq loc | dp loc |
+------+-----------------+---------------------+--------+--------+--------+
|    0 | ETHERNET        |   0   0     0     0 |  0   0 |  0   0 |  0   0 |
|    1 | ETHERNET        |   1   1     1     1 |  0   1 |  0   1 |  0   1 |
|    2 | ETHERNET        |   2   2     2     2 |  0   2 |  0   2 |  0   2 |
|    3 | ETHERNET        |   3   3     3     3 |  0   3 |  0   3 |  0   3 |
|    4 | ETHERNET        |   4   4     4     4 |  0   4 |  0   4 |  0   4 |
|    5 | ETHERNET        |   5   5     5     5 |  0   5 |  0   5 |  0   5 |
|    6 | ETHERNET        |   6   6     6     6 |  0   6 |  0   6 |  0   6 |
|    7 | ETHERNET        |   7   7     7     7 |  0   7 |  0   7 |  0   7 |
|    8 | ETHERNET        |   8   8     8     8 |  0   8 |  0   8 |  0   8 |
|    9 | ETHERNET        |   9   9     9     9 |  0   9 |  0   9 |  0   9 |
|   10 | ETHERNET        |  10  10    10    10 |  0  10 |  0  10 |  0  10 |
|   11 | ETHERNET        |  11  11    11    11 |  0  11 |  0  11 |  0  11 |
|   63 | CPU-SDMA        |  63  12    72    72 |  0  12 |  0  12 |  0  12 |
+------+-----------------+---------------------+--------+--------+--------+
|   12 | ETHERNET        |  12  96    12    12 |  1   0 |  1   0 |  1   0 |
|   13 | ETHERNET        |  13  97    13    13 |  1   1 |  1   1 |  1   1 |
|   14 | ETHERNET        |  14  98    14    14 |  1   2 |  1   2 |  1   2 |
|   15 | ETHERNET        |  15  99    15    15 |  1   3 |  1   3 |  1   3 |
|   16 | ETHERNET        |  16 100    16    16 |  1   4 |  1   4 |  1   4 |
|   17 | ETHERNET        |  17 101    17    17 |  1   5 |  1   5 |  1   5 |
|   18 | ETHERNET        |  18 102    18    18 |  1   6 |  1   6 |  1   6 |
|   19 | ETHERNET        |  19 103    19    19 |  1   7 |  1   7 |  1   7 |
|   20 | ETHERNET        |  20 104    20    20 |  1   8 |  1   8 |  1   8 |
|   21 | ETHERNET        |  21 105    21    21 |  1   9 |  1   9 |  1   9 |
|   22 | ETHERNET        |  22 106    22    22 |  1  10 |  1  10 |  1  10 |
|   23 | ETHERNET        |  23 107    23    23 |  1  11 |  1  11 |  1  11 |
+------+-----------------+---------------------+--------+--------+--------+
|   24 | ETHERNET        |  24 192    24    24 |  2   0 |  2   0 |  2   0 |
|   25 | ETHERNET        |  25 193    25    25 |  2   1 |  2   1 |  2   1 |
|   26 | ETHERNET        |  26 194    26    26 |  2   2 |  2   2 |  2   2 |
|   27 | ETHERNET        |  27 195    27    27 |  2   3 |  2   3 |  2   3 |
|   28 | ETHERNET        |  28 196    28    28 |  2   4 |  2   4 |  2   4 |
|   29 | ETHERNET        |  29 197    29    29 |  2   5 |  2   5 |  2   5 |
|   30 | ETHERNET        |  30 198    30    30 |  2   6 |  2   6 |  2   6 |
|   31 | ETHERNET        |  31 199    31    31 |  2   7 |  2   7 |  2   7 |
|   32 | ETHERNET        |  32 200    32    32 |  2   8 |  2   8 |  2   8 |
|   33 | ETHERNET        |  33 201    33    33 |  2   9 |  2   9 |  2   9 |
|   34 | ETHERNET        |  34 202    34    34 |  2  10 |  2  10 |  2  10 |
|   35 | ETHERNET        |  35 203    35    35 |  2  11 |  2  11 |  2  11 |
+------+-----------------+---------------------+--------+--------+--------+
|   36 | ETHERNET        |  36 288    36    36 |  3   0 |  3   0 |  3   0 |
|   37 | ETHERNET        |  37 289    37    37 |  3   1 |  3   1 |  3   1 |
|   38 | ETHERNET        |  38 290    38    38 |  3   2 |  3   2 |  3   2 |
|   39 | ETHERNET        |  39 291    39    39 |  3   3 |  3   3 |  3   3 |
|   40 | ETHERNET        |  40 292    40    40 |  3   4 |  3   4 |  3   4 |
|   41 | ETHERNET        |  41 293    41    41 |  3   5 |  3   5 |  3   5 |
|   42 | ETHERNET        |  42 294    42    42 |  3   6 |  3   6 |  3   6 |
|   43 | ETHERNET        |  43 295    43    43 |  3   7 |  3   7 |  3   7 |
|   44 | ETHERNET        |  44 296    44    44 |  3   8 |  3   8 |  3   8 |
|   45 | ETHERNET        |  45 297    45    45 |  3   9 |  3   9 |  3   9 |
|   46 | ETHERNET        |  46 298    46    46 |  3  10 |  3  10 |  3  10 |
|   47 | ETHERNET        |  47 299    47    47 |  3  11 |  3  11 |  3  11 |
+------+-----------------+---------------------+--------+--------+--------+
|   48 | ETHERNET        |  48 384    48    48 |  4   0 |  4   0 |  4   0 |
|   49 | ETHERNET        |  49 385    49    49 |  4   1 |  4   1 |  4   1 |
|   50 | ETHERNET        |  50 386    50    50 |  4   2 |  4   2 |  4   2 |
|   51 | ETHERNET        |  51 387    51    51 |  4   3 |  4   3 |  4   3 |
|   52 | ETHERNET        |  52 388    52    52 |  4   4 |  4   4 |  4   4 |
|   53 | ETHERNET        |  53 389    53    53 |  4   5 |  4   5 |  4   5 |
|   54 | ETHERNET        |  54 390    54    54 |  4   6 |  4   6 |  4   6 |
|   55 | ETHERNET        |  55 391    55    55 |  4   7 |  4   7 |  4   7 |
|   56 | ETHERNET        |  56 392    56    56 |  4   8 |  4   8 |  4   8 |
|   57 | ETHERNET        |  57 393    57    57 |  4   9 |  4   9 |  4   9 |
|   58 | ETHERNET        |  58 394    58    58 |  4  10 |  4  10 |  4  10 |
|   59 | ETHERNET        |  59 395    59    59 |  4  11 |  4  11 |  4  11 |
+------+-----------------+---------------------+--------+--------+--------+
|   64 | ETHERNET        |  64 484    64    64 |  5   4 |  5   4 |  5   4 |
|   65 | ETHERNET        |  65 485    65    65 |  5   5 |  5   5 |  5   5 |
|   66 | ETHERNET        |  66 486    66    66 |  5   6 |  5   6 |  5   6 |
|   67 | ETHERNET        |  67 487    67    67 |  5   7 |  5   7 |  5   7 |
|   68 | ETHERNET        |  68 488    68    68 |  5   8 |  5   8 |  5   8 |
|   69 | ETHERNET        |  69 489    69    69 |  5   9 |  5   9 |  5   9 |
|   70 | ETHERNET        |  70 490    70    70 |  5  10 |  5  10 |  5  10 |
|   71 | ETHERNET        |  71 491    71    71 |  5  11 |  5  11 |  5  11 |
|   80 | ETHERNET        |  60 480    60    60 |  5   0 |  5   0 |  5   0 |
|   81 | ETHERNET        |  61 481    61    61 |  5   1 |  5   1 |  5   1 |
|   82 | ETHERNET        |  62 482    62    62 |  5   2 |  5   2 |  5   2 |
|   83 | ETHERNET        |  63 483    63    63 |  5   3 |  5   3 |  5   3 |
+------+-----------------+---------------------+--------+--------+--------+
#endif

GT_U32 bc3PaTestCore(GT_U32 coreIdx)
{
    GT_STATUS rc;

    GT_U8 dev = 0;
    GT_U32 cc = 525;

    typedef struct 
    {
        GT_U32 coreIdx;
        GT_PHYSICAL_PORT_NUM portNum;
    }CORE_IDX_PORT2CONF_STC;
    CORE_IDX_PORT2CONF_STC portInitList[] = 
    {
         { 0,  0 }
        ,{ 1, 12 }
        ,{ 2, 24 }
        ,{ 3, 36 }
        ,{ 4, 48 }
        ,{ 5, 64 }
    };

    CORE_IDX_PORT2CONF_STC portInitListAll[] = 
    {
         { 0,  0   }
        ,{ 1, 12+1 }
        ,{ 2, 24+2 }
        ,{ 3, 36+3 }
        ,{ 4, 48+4 }
        ,{ 5, 64+5 }
    };


    CPSS_PORTS_BMP_STC initPortsBmp,/* bitmap of ports to init */
                      *initPortsBmpPtr;/* pointer to bitmap */
    GT_U32 i;

    #ifdef ASIC_SIMULATION
        #ifndef GM_USED
            appDemoBc3SimCoreClockSet(dev,cc); 
        #else
            cc = cc;
        #endif
    #endif

    rc = appDemoDbEntryAdd("initSerdesDefaults", 0);
    if (rc != GT_OK)
    {
        return rc;
    }

/* 
    rc = appDemoDbEntryAdd("noCpu", 1);
    if (rc != GT_OK)
    {
        return rc;
    }
    */


    rc = cpssInitSystem(29,1,0);
    if (rc != GT_OK)
    {
        return rc;
    }

    initPortsBmpPtr = &initPortsBmp;
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(initPortsBmpPtr);
    if (0<= coreIdx && coreIdx <= 5)
    {
        CPSS_PORTS_BMP_PORT_SET_MAC(initPortsBmpPtr,portInitList[coreIdx].portNum);

        rc = cpssDxChPortModeSpeedSet(dev, initPortsBmp, GT_TRUE,
                                    CPSS_PORT_INTERFACE_MODE_KR_E, CPSS_PORT_SPEED_10000_E);
        if(GT_OK != rc)
        {
            return rc;
        }
    }
    else 
    {
        initPortsBmpPtr = &initPortsBmp;
        for (i = 0 ; i < sizeof(portInitListAll)/sizeof(portInitListAll[0]); i++)
        {
            CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(initPortsBmpPtr);
            CPSS_PORTS_BMP_PORT_SET_MAC(initPortsBmpPtr,portInitListAll[i].portNum);

            rc = cpssDxChPortModeSpeedSet(dev, initPortsBmp, GT_TRUE,
                                        CPSS_PORT_INTERFACE_MODE_KR_E, CPSS_PORT_SPEED_10000_E);
            if(GT_OK != rc)
            {
                return rc;
            }
        }
    }


    rc = appDemoBobcat3PortMappingDump(dev);
    if (rc != GT_OK)
    {
        return rc;
    }


    rc = appDemoBobcat3PortGlobalResourcesPrint(dev);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = appDemoBobcat3PortListResourcesPrint(dev);
    if (rc != GT_OK)
    {
        return rc;
    }


    rc = gtBobcat2PortPizzaArbiterIfStateDump(dev,0);
    if (rc != GT_OK)
    {
        return rc;
    }
/*
#ifdef WIN32
     PORT_MODE_SET_UT_DEBUG_PRINT_SET(1);
    cpssDxChPortModeSpeedSetUT();
#endif 
    */
    return GT_OK;
}


GT_STATUS bc3_512ports(GT_VOID)
{
    GT_STATUS rc;

    GT_U8 dev = 0;

    rc = appDemoDbEntryAdd("doublePhysicalPorts", 1);
    if (rc != GT_OK)
    {
        return rc;
    }
    rc = cpssInitSystem(29,1,0);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}


GT_STATUS prvCpssDxChPortTxQUnitDQPortRequestMaskGet
(
    IN  GT_U8  devNum,
    IN  GT_U32 dqIdx,
    IN  GT_U32 maskId,
    OUT GT_U32 *maskValPtr
);

#define PRV_CPSS_DXCH_TXQDQ_PORT_REQUEST_MASK_MAX_NUM_CNS 4
#define PRV_CPSS_DXCH_TXQDQ_PORT_REQUEST_MASK_LEN_CNS     8


static GT_STATUS Lion2_cpssDxChPortTxRequestMaskSet
(
    IN GT_U8    devNum,
    IN GT_U32   maskIndex,
    IN GT_U32   maskValue
)
{
    GT_STATUS rc;
    GT_U32 regAddr;   /* register address */
    GT_U32 portGroupId;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E  | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E);

    if (maskIndex >= PRV_CPSS_DXCH_TXQDQ_PORT_REQUEST_MASK_MAX_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (maskValue < 3 || maskValue > 255)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.dq.scheduler.config.portRequestMask;

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
    {
        rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId, regAddr, maskIndex*8, 8, maskValue);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)

    return GT_OK;
}

static GT_STATUS Lion2_cpssDxChPortTxRequestMaskGet
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   maskIndex,
    IN GT_U32   *maskValuePtr
)
{
    GT_STATUS rc;
    GT_U32 regAddr;   /* register address */


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E  | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E);

    if (maskIndex >= PRV_CPSS_DXCH_TXQDQ_PORT_REQUEST_MASK_MAX_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr =  PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.dq.scheduler.config.portRequestMask;
    regAddr =  0x11001604;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, maskIndex*8, 8, /*OUT*/maskValuePtr);
    if(rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}



GT_STATUS reqMaskPrint(GT_U32 systemType)
{
    GT_STATUS rc;

    GT_U8 dev = 0;
    GT_U32 dpIdx;
    GT_U32 reqMaskIdx;
    GT_U32 reqMaskValue;
    GT_U32 reMaskValueArr[8][PRV_CPSS_DXCH_TXQDQ_PORT_REQUEST_MASK_MAX_NUM_CNS] = {0};
    GT_U32 dqNum = 1;

    rc = cpssInitSystem(systemType,1,0);
    if (rc != GT_OK)
    {
        return rc;
    }

    cpssOsTimerWkAfter(1000);


    if(PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
    {
        if (PRV_CPSS_DXCH_BOBCAT2_CHECK_MAC(dev))
        {
            dqNum = 1;
        }
        else
        {
            dqNum = PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.multiDataPath.numTxqDq;
        }

        for (dpIdx = 0; dpIdx < dqNum; dpIdx++)
        {
            for (reqMaskIdx = 0 ; reqMaskIdx < PRV_CPSS_DXCH_TXQDQ_PORT_REQUEST_MASK_MAX_NUM_CNS; reqMaskIdx++)
            {
                rc = prvCpssDxChPortTxQUnitDQPortRequestMaskGet(dev
                                                                ,dpIdx
                                                                ,reqMaskIdx
                                                                ,/*OUT*/&reqMaskValue);
                if (GT_OK != rc )
                {
                    return rc;
                }
                reMaskValueArr[dpIdx][reqMaskIdx] = reqMaskValue;
            }
        }
    }
    else if (PRV_CPSS_DXCH_LION2_FAMILY_CHECK_MAC(dev))
    {
        GT_U32 portGroupId;

        dqNum = 8;
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev,portGroupId)
        {
            for (reqMaskIdx = 0 ; reqMaskIdx < PRV_CPSS_DXCH_TXQDQ_PORT_REQUEST_MASK_MAX_NUM_CNS; reqMaskIdx++)
            {
                rc = Lion2_cpssDxChPortTxRequestMaskGet(dev,portGroupId,reqMaskIdx,&reqMaskValue);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
            reMaskValueArr[portGroupId][reqMaskIdx] = reqMaskValue;
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
    } 
    else if (PRV_CPSS_DXCH_LION_FAMILY_CHECK_MAC(dev))
    {
        dqNum = 1;

        for (reqMaskIdx = 0 ; reqMaskIdx < PRV_CPSS_DXCH_TXQDQ_PORT_REQUEST_MASK_MAX_NUM_CNS; reqMaskIdx++)
        {
            rc = cpssDxChPortTxRequestMaskGet(dev,reqMaskIdx,/*OUT*/&reqMaskValue);
            if (GT_OK != rc )
            {
                return rc;
            }
            reMaskValueArr[0][reqMaskIdx] = reqMaskValue;
        }
    }
    else  
    {
        return GT_NOT_SUPPORTED;
    }

    cpssOsPrintf("\n+-----+--------------------+");
    cpssOsPrintf("\n|  dq |  request mask idx  |");
    cpssOsPrintf("\n| %3s |"," ");
    for (reqMaskIdx = 0 ; reqMaskIdx < PRV_CPSS_DXCH_TXQDQ_PORT_REQUEST_MASK_MAX_NUM_CNS; reqMaskIdx++)
    {
        cpssOsPrintf(" %3d ",reqMaskIdx);
    }
    cpssOsPrintf("|");
    cpssOsPrintf("\n+-----+--------------------+");

    for (dpIdx = 0; dpIdx < dqNum; dpIdx++)
    {
        cpssOsPrintf("\n| %3d |",dpIdx);
        for (reqMaskIdx = 0 ; reqMaskIdx < PRV_CPSS_DXCH_TXQDQ_PORT_REQUEST_MASK_MAX_NUM_CNS; reqMaskIdx++)
        {
            cpssOsPrintf(" %3d ",reMaskValueArr[dpIdx][reqMaskIdx]);
        }
    }
    cpssOsPrintf("\n+-----+--------------------+");
    cpssOsPrintf("\n");

    return GT_OK;
}



typedef struct 
{
    GT_U32 idxInExcel;
    GT_U32 regAddr;
    GT_U32 value;
    GT_U32 mask;
    GT_CHAR *comment;
}REG_FLD_CONFIG_STC;

#define BAD_VALUE (GT_U32)(~0)

GT_STATUS asicUserRegListPrintDiffOnly
(
    IN REG_FLD_CONFIG_STC *regListptr
)
{
    GT_U32 rc;
    GT_U8 dev = 0;
    GT_U32 regIdx;
    GT_U32 data;

    cpssOsPrintf("\n+-------------------------------------------------------------------------------------------------------+");
    cpssOsPrintf("\n|                                                       Diff Register dump                              |");
    cpssOsPrintf("\n+------+------------+------------+------------+------------+--------------------------------------------+");
    cpssOsPrintf("\n| xls# |  regAddr   |   mask     |   dataHW   |   dataSW   | comment                                    |");
    cpssOsPrintf("\n+------+------------+------------+------------+------------+--------------------------------------------+");

    for (regIdx = 0 ; regListptr[regIdx].regAddr != BAD_VALUE ; regIdx++)
    {
        rc = prvCpssDrvHwPpReadRegister(dev,regListptr[regIdx].regAddr, /*OUT*/&data);
        if (rc != GT_OK)
        {
            return rc;
        }

        if ((data & regListptr[regIdx].mask)!= regListptr[regIdx].value)
        {
            cpssOsPrintf("\n| %4d | 0x%08x | 0x%08x | 0x%08x | 0x%08x | %-40s",regListptr[regIdx].idxInExcel
                                                    ,regListptr[regIdx].regAddr
                                                    ,regListptr[regIdx].mask
                                                    ,regListptr[regIdx].value
                                                    ,data,
                                                    regListptr[regIdx].comment);
        }
    }
    cpssOsPrintf("\n+------+------------+------------+------------+------------+--------------------------------------------+");
    return GT_OK;
}


GT_STATUS asicUserRegListPrintAll
(
    IN GT_U8              dev,
    IN REG_FLD_CONFIG_STC *regListptr
)
{
    GT_U32 rc;
    GT_U32 regIdx;
    GT_U32 data;
    GT_BOOL res;
    GT_CHAR *resStr[] =  { " - ", "OK" };

    cpssOsPrintf("\n+-----------------------------------------------------------------------------------------------------------+");
    cpssOsPrintf("\n|                              Diff Register dump                                                           |");
    cpssOsPrintf("\n+------+------------+------------+------------+------------+-----+------------------------------------------+");
    cpssOsPrintf("\n| xls# |  regAddr   |   mask     |   dataHW   |   dataSW   | res | comment                                  |");
    cpssOsPrintf("\n+------+------------+------------+------------+------------+-----+------------------------------------------+");

    for (regIdx = 0 ; regListptr[regIdx].regAddr != BAD_VALUE ; regIdx++)
    {
        cpssOsPrintf("\n| %4d | 0x%08x | 0x%08x | 0x%08x |",regListptr[regIdx].idxInExcel
                                                    ,regListptr[regIdx].regAddr
                                                    ,regListptr[regIdx].mask
                                                    ,regListptr[regIdx].value);

        rc = prvCpssDrvHwPpReadRegister(dev,regListptr[regIdx].regAddr, /*OUT*/&data);
        if (rc != GT_OK)
        {
            return rc;
        }

        res = (GT_BOOL)(data & regListptr[regIdx].mask) == (regListptr[regIdx].value & regListptr[regIdx].mask);
        cpssOsPrintf(" 0x%08x | %3s | %-40s", data
                                            ,resStr[res]
                                            ,regListptr[regIdx].comment);
    }
    cpssOsPrintf("\n+------+------------+------------+------------+------------+-----+------------------------------------------+");
    return GT_OK;
}



REG_FLD_CONFIG_STC  bc3_LED_CONFIG_48xXG_6xCG_2xXG[] = 
{
    /* Register 
        Address                   Value       Mask            Action performed with configuration  */
/* IF 0 */
      { 2  , 0x27000008 ,         0x4         , 0x3F            ,"Disable when link down for Class2                                  "}
     ,{ 3  , 0x27000004 ,         0x100000    , 0x100000        ,"Blink Enable for Class2                                            "}
     ,{ 4  , 0x27000000 ,         0x434000    , 0xFFFF000       ,"Configure LED start to bit 64 and LED End to bit 67                "}
     ,{ 5  , 0x27000000 ,         0x0         , 0x10            ,"Configure LED Light High to active low                             "}
     ,{ 6  , 0x27000120 ,         0x6A        , 0xFFFFFFFF      ,"Configure LED ports 0, 1 and 2 to CG ports and LED Port 4 to XG    "}
     ,{ 7  , 0x27000128 ,         0xEFBEFB    , 0xFFFFFFFF      ,"Configure polarity of Class2 to inverted for LED ports 0,1,2 and 3 "}
     ,{ 8  , 0x100C0030 ,         0x7E0       , 0x7E0           ,"Configure XG port 0 LED number to 0x3F (Errata FE-3763116 )        "}
     ,{ 9  , 0x100C1030 ,         0x7E0       , 0x7E0           ,"Configure XG port 1 LED number to 0x3F (Errata FE-3763116 )        "}
     ,{ 10 , 0x100C2030 ,         0x7E0       , 0x7E0           ,"Configure XG port 2 LED number to 0x3F (Errata FE-3763116 )        "}
     ,{ 11 , 0x100C3030 ,         0x7E0       , 0x7E0           ,"Configure XG port 3 LED number to 0x3F (Errata FE-3763116 )        "}
     ,{ 12 , 0x100C4030 ,         0x7E0       , 0x7E0           ,"Configure XG port 4 LED number to 0x3F (Errata FE-3763116 )        "}
     ,{ 13 , 0x100C5030 ,         0x7E0       , 0x7E0           ,"Configure XG port 5 LED number to 0x3F (Errata FE-3763116 )        "}
     ,{ 14 , 0x100C6030 ,         0x7E0       , 0x7E0           ,"Configure XG port 6 LED number to 0x3F (Errata FE-3763116 )        "}
     ,{ 15 , 0x100C7030 ,         0x7E0       , 0x7E0           ,"Configure XG port 7 LED number to 0x3F (Errata FE-3763116 )        "}
     ,{ 16 , 0x100C8030 ,         0x7E0       , 0x7E0           ,"Configure XG port 8 LED number to 0x3F (Errata FE-3763116 )        "}
     ,{ 17 , 0x100C9030 ,         0x7E0       , 0x7E0           ,"Configure XG port 9 LED number to 0x3F (Errata FE-3763116 )        "}
     ,{ 18 , 0x100CA030 ,         0x7E0       , 0x7E0           ,"Configure XG port 10 LED number to 0x3F (Errata FE-3763116 )       "}
     ,{ 19 , 0x100CB030 ,         0x7E0       , 0x7E0           ,"Configure XG port 11 LED number to 0x3F (Errata FE-3763116 )       "}
     ,{ 20 , 0x10000090 ,         0xFC00      , 0xFC00          ,"Configure Gig port 0 LED number to 0x3F (Errata FE-3763116 )       "}
     ,{ 21 , 0x10001090 ,         0xFC00      , 0xFC00          ,"Configure Gig port 1 LED number to 0x3F (Errata FE-3763116 )       "}
     ,{ 22 , 0x10002090 ,         0xFC00      , 0xFC00          ,"Configure Gig port 2 LED number to 0x3F (Errata FE-3763116 )       "}
     ,{ 23 , 0x10003090 ,         0xFC00      , 0xFC00          ,"Configure Gig port 3 LED number to 0x3F (Errata FE-3763116 )       "}
     ,{ 24 , 0x10004090 ,         0xFC00      , 0xFC00          ,"Configure Gig port 4 LED number to 0x3F (Errata FE-3763116 )       "}
     ,{ 25 , 0x10005090 ,         0xFC00      , 0xFC00          ,"Configure Gig port 5 LED number to 0x3F (Errata FE-3763116 )       "}
     ,{ 26 , 0x10006090 ,         0xFC00      , 0xFC00          ,"Configure Gig port 6 LED number to 0x3F (Errata FE-3763116 )       "}
     ,{ 27 , 0x10007090 ,         0xFC00      , 0xFC00          ,"Configure Gig port 7 LED number to 0x3F (Errata FE-3763116 )       "}
     ,{ 28 , 0x10008090 ,         0xFC00      , 0xFC00          ,"Configure Gig port 8 LED number to 0x3F (Errata FE-3763116 )       "}
     ,{ 29 , 0x10009090 ,         0xFC00      , 0xFC00          ,"Configure Gig port 9 LED number to 0x3F (Errata FE-3763116 )       "}
     ,{ 30 , 0x1000A090 ,         0xFC00      , 0xFC00          ,"Configure Gig port 10 LED number to 0x3F (Errata FE-3763116 )      "}
     ,{ 31 , 0x1000B090 ,         0xFC00      , 0xFC00          ,"Configure Gig port 11 LED number to 0x3F (Errata FE-3763116 )      "}
     ,{ 32 , 0x10340004 ,         0x0         , 0x3F            ,"Configure CG port 0 LED number to 0                                "}
     ,{ 33 , 0x10344004 ,         0x1         , 0x3F            ,"Configure CG port 4 LED number to 1                                "}
     ,{ 34 , 0x10348004 ,         0x2         , 0x3F            ,"Configure CG port 8 LED number to 2                                "}
     ,{ 35 , 0x100E4030 ,         0x60        , 0x7E0           ,"Configure XG port 72 LED number to 3                               "}
     ,{ 36 , 0x27000000 ,         0x0         , 0x10            ,"Configure LED Light High to active low                             "}
     ,{ 37 , 0x27000000 ,         0x1         , 0x1             , "Release LED Stream reset                                          "}
/* IF 1 */
     ,{ 39 , 0x28000008 ,         0x4         , 0x3F            ,"Disable when link down for Class2                                        "}
     ,{ 40 , 0x28000004 ,         0x100000    , 0x100000        ,"Blink Enable for Class2                                                  "}
     ,{ 41 , 0x28000000 ,         0x4E40000   , 0xFFFF000       ,"Configure LED start to bit 64 and LED End to bit 78                      "}
     ,{ 42 , 0x28000120 ,         0x1555556A  , 0xFFFFFFFF      ,"Configure LED ports 0, 1 and 2 to CG ports and LED Ports 4-14 to XG      "}
     ,{ 43 , 0x28000128 ,         0xEFBEFB    , 0x00FFFFFF      ,"Configure polarity of Class2 to inverted for LED ports 0,1,2 and 3       "}
     ,{ 44 , 0x2800012C ,         0xEFBEFB    , 0x00FFFFFF      ,"Configure polarity of Class2 to inverted for LED ports 4,5,6 and 7       "}
     ,{ 45 , 0x28000130 ,         0xEFBEFB    , 0x00FFFFFF      ,"Configure polarity of Class2 to inverted for LED ports 8,9,10 and 11     "}
     ,{ 46 , 0x28000134 ,         0xEFBEFB    , 0x00FFFFFF      ,"Configure polarity of Class2 to inverted for LED ports 12,13, 14 and 15  "}
     ,{ 47 , 0x100CC030 ,         0x7E0       , 0x7E0           ,"Configure XG port 12 LED number to 0x3F (Errata FE-3763116 )             "}
     ,{ 48 , 0x100CD030 ,         0x7E0       , 0x7E0           ,"Configure XG port 13 LED number to 0x3F (Errata FE-3763116 )             "}
     ,{ 49 , 0x100CE030 ,         0x7E0       , 0x7E0           ,"Configure XG port 14 LED number to 0x3F (Errata FE-3763116 )             "}
     ,{ 50 , 0x100CF030 ,         0x7E0       , 0x7E0           ,"Configure XG port 15 LED number to 0x3F (Errata FE-3763116 )             "}
     ,{ 51 , 0x100D0030 ,         0x7E0       , 0x7E0           ,"Configure XG port 16 LED number to 0x3F (Errata FE-3763116 )             "}
     ,{ 52 , 0x100D1030 ,         0x7E0       , 0x7E0           ,"Configure XG port 17 LED number to 0x3F (Errata FE-3763116 )             "}
     ,{ 53 , 0x100D2030 ,         0x7E0       , 0x7E0           ,"Configure XG port 18 LED number to 0x3F (Errata FE-3763116 )             "}
     ,{ 54 , 0x100D3030 ,         0x7E0       , 0x7E0           ,"Configure XG port 19 LED number to 0x3F (Errata FE-3763116 )             "}
     ,{ 55 , 0x100D4030 ,         0x7E0       , 0x7E0           ,"Configure XG port 20 LED number to 0x3F (Errata FE-3763116 )             "}
     ,{ 56 , 0x100D5030 ,         0x7E0       , 0x7E0           ,"Configure XG port 21 LED number to 0x3F (Errata FE-3763116 )             "}
     ,{ 57 , 0x100D6030 ,         0x7E0       , 0x7E0           ,"Configure XG port 22 LED number to 0x3F (Errata FE-3763116 )             "}
     ,{ 58 , 0x100D7030 ,         0x7E0       , 0x7E0           ,"Configure XG port 23 LED number to 0x3F (Errata FE-3763116 )             "}
     ,{ 59 , 0x1000C090 ,         0xFC00      , 0xFC00          ,"Configure Gig port 0 LED number to 0x3F (Errata FE-3763116 )             "}
     ,{ 60 , 0x1000D090 ,         0xFC00      , 0xFC00          ,"Configure Gig port 1 LED number to 0x3F (Errata FE-3763116 )             "}
     ,{ 61 , 0x1000E090 ,         0xFC00      , 0xFC00          ,"Configure Gig port 2 LED number to 0x3F (Errata FE-3763116 )             "}
     ,{ 62 , 0x1000F090 ,         0xFC00      , 0xFC00          ,"Configure Gig port 3 LED number to 0x3F (Errata FE-3763116 )             "}
     ,{ 63 , 0x10010090 ,         0xFC00      , 0xFC00          ,"Configure Gig port 4 LED number to 0x3F (Errata FE-3763116 )             "}
     ,{ 64 , 0x10011090 ,         0xFC00      , 0xFC00          ,"Configure Gig port 5 LED number to 0x3F (Errata FE-3763116 )             "}
     ,{ 65 , 0x10012090 ,         0xFC00      , 0xFC00          ,"Configure Gig port 6 LED number to 0x3F (Errata FE-3763116 )             "}
     ,{ 66 , 0x10013090 ,         0xFC00      , 0xFC00          ,"Configure Gig port 7 LED number to 0x3F (Errata FE-3763116 )             "}
     ,{ 67 , 0x10014090 ,         0xFC00      , 0xFC00          ,"Configure Gig port 8 LED number to 0x3F (Errata FE-3763116 )             "}
     ,{ 68 , 0x10015090 ,         0xFC00      , 0xFC00          ,"Configure Gig port 9 LED number to 0x3F (Errata FE-3763116 )             "}
     ,{ 69 , 0x10016090 ,         0xFC00      , 0xFC00          ,"Configure Gig port 10 LED number to 0x3F (Errata FE-3763116 )            "}
     ,{ 70 , 0x10017090 ,         0xFC00      , 0xFC00          ,"Configure Gig port 11 LED number to 0x3F (Errata FE-3763116 )            "}
     ,{ 71 , 0x1034C004 ,         0x0         , 0x3F            ,"Configure CG port 12 LED number to 0                                     "}
     ,{ 72 , 0x10350004 ,         0x1         , 0x3F            ,"Configure CG port 16 LED number to 1                                     "}
     ,{ 73 , 0x10354004 ,         0x2         , 0x3F            ,"Configure CG port 20 LED number to 2                                     "}
     ,{ 74 , 0x100D8030 ,         0x60        , 0x7E0           ,"Configure XG port 24 LED number to 3                                     "}
     ,{ 75 , 0x100D9030 ,         0x80        , 0x7E0           ,"Configure XG port 25 LED number to 4                                     "}
     ,{ 76 , 0x100DA030 ,         0xA0        , 0x7E0           ,"Configure XG port 26 LED number to 5                                     "}
     ,{ 77 , 0x100DB030 ,         0xC0        , 0x7E0           ,"Configure XG port 27 LED number to 6                                     "}
     ,{ 78 , 0x100DC030 ,         0xE0        , 0x7E0           ,"Configure XG port 28 LED number to 7                                     "}
     ,{ 79 , 0x100DD030 ,         0x100       , 0x7E0           ,"Configure XG port 29 LED number to 8                                     "}
     ,{ 80 , 0x100DE030 ,         0x120       , 0x7E0           ,"Configure XG port 30 LED number to 9                                     "}
     ,{ 81 , 0x100DF030 ,         0x140       , 0x7E0           ,"Configure XG port 31 LED number to 10                                    "}
     ,{ 82 , 0x100E0030 ,         0x160       , 0x7E0           ,"Configure XG port 32 LED number to 11                                    "}
     ,{ 83 , 0x100E1030 ,         0x180       , 0x7E0           ,"Configure XG port 33LED number to 12                                     "}
     ,{ 84 , 0x100E2030 ,         0x1A0       , 0x7E0           ,"Configure XG port 34 LED number to 13                                    "}
     ,{ 85 , 0x100E3030 ,         0x1C0       , 0x7E0           ,"Configure XG port 35 LED number to 14                                    "}
     ,{ 86 , 0x28000000 ,         0x0         , 0x10            ,"Configure LED Light High to active low                                   "}
/* IF 2  */
     ,{  89, 0xA7000008 ,         0x4         , 0x3F            ,"Disable when link down for Class2                                        "}
     ,{  90, 0xA7000004 ,         0x100000    , 0x100000        ,"Blink Enable for Class2                                                  "}
     ,{  91, 0xA7000000 ,         0x4C40000   , 0xFFFF000       ,"Configure LED start to bit 64 and LED End to bit 76                      "}
     ,{  92, 0xA7000120 ,         0x55555555  , 0xFFFFFFFF       ,"Configure LED ports 0-12 to XG                                           "}
     ,{  93, 0xA7000128 ,         0xEFBEFB    , 0x00FFFFFF      ,"Configure polarity of Class2 to inverted for LED ports 0,1,2 and 3       "}
     ,{  94, 0xA700012C ,         0xEFBEFB    , 0x00FFFFFF      ,"Configure polarity of Class2 to inverted for LED ports 4,5,6 and 7       "}
     ,{  95, 0xA7000130 ,         0xEFBEFB    , 0x00FFFFFF      ,"Configure polarity of Class2 to inverted for LED ports 8,9,10 and 11     "}
     ,{  96, 0xA7000134 ,         0xEFBEFB    , 0x00FFFFFF      ,"Configure polarity of Class2 to inverted for LED ports 12,13, 14 and 15  "}
     ,{  97, 0x104C0030 ,         0x0         , 0x7E0           ,"Configure XG port 36 LED number to 0                                     "}
     ,{  98, 0x104C1030 ,         0x20        , 0x7E0           ,"Configure XG port 37 LED number to 1                                     "}
     ,{  99, 0x104C2030 ,         0x40        , 0x7E0           ,"Configure XG port 38 LED number to 2                                     "}
     ,{ 100, 0x104C3030 ,         0x60        , 0x7E0           ,"Configure XG port 39 LED number to 3                                     "}
     ,{ 101, 0x104C4030 ,         0x80        , 0x7E0           ,"Configure XG port 40 LED number to 4                                     "}
     ,{ 102, 0x104C5030 ,         0xA0        , 0x7E0           ,"Configure XG port 41 LED number to 5                                     "}
     ,{ 103, 0x104C6030 ,         0xC0        , 0x7E0           ,"Configure XG port 42 LED number to 6                                     "}
     ,{ 104, 0x104C7030 ,         0xE0        , 0x7E0           ,"Configure XG port 43 LED number to 7                                     "}
     ,{ 105, 0x104C8030 ,         0x100       , 0x7E0           ,"Configure XG port 44 LED number to 8                                     "}
     ,{ 106, 0x104C9030 ,         0x120       , 0x7E0           ,"Configure XG port 45 LED number to 9                                     "}
     ,{ 107, 0x104CA030 ,         0x140       , 0x7E0           ,"Configure XG port 46 LED number to 10                                    "}
     ,{ 108, 0x104CB030 ,         0x160       , 0x7E0           ,"Configure XG port 47 LED number to 11                                    "}
     ,{ 109, 0x104E4030 ,         0x180       , 0x7E0           ,"Configure XG port 73 LED number to 12                                    "}
     ,{ 110, 0xA7000000 ,         0x0         , 0x10            ,"Configure LED Light High to active low                                   "}
/* IF 3 */
     ,{ 113 , 0xA8000008 ,         0x4         , 0x3F            ,"Disable when link down for Class2                                        "}
     ,{ 114 , 0xA8000004 ,         0x100000    , 0x100000        ,"Blink Enable for Class2                                                  "}
     ,{ 115 , 0xA8000000 ,         0x574000    , 0xFFFF000       ,"Configure LED start to bit 64 and LED End to bit 87                      "}
     ,{ 116 , 0xA8000120 ,         0x55555555  , 0xFFFFFFFF      ,"Configure LED ports 0-15XG                                               "}
     ,{ 117 , 0xA8000124 ,         0x5555      , 0xFFFFFFFF      ,"Configure LED ports 16-23 XG                                             "}
     ,{ 118 , 0xA8000128 ,         0xEFBEFB    , 0x00FFFFFF      ,"Configure polarity of Class2 to inverted for LED ports 0,1,2 and 3       "}
     ,{ 119 , 0xA800012C ,         0xEFBEFB    , 0x00FFFFFF      ,"Configure polarity of Class2 to inverted for LED ports 4,5,6 and 7       "}
     ,{ 120 , 0xA8000130 ,         0xEFBEFB    , 0x00FFFFFF      ,"Configure polarity of Class2 to inverted for LED ports 8,9,10 and 11     "}
     ,{ 121 , 0xA8000134 ,         0xEFBEFB    , 0x00FFFFFF      ,"Configure polarity of Class2 to inverted for LED ports 12,13, 14 and 15  "}
     ,{ 122 , 0xA8000138 ,         0xEFBEFB    , 0x00FFFFFF      ,"Configure polarity of Class2 to inverted for LED ports 16,17,18 and 19   "}
     ,{ 123 , 0xA800013C ,         0xEFBEFB    , 0x00FFFFFF      ,"Configure polarity of Class2 to inverted for LED ports 20,21,22 and 23   "}
     ,{ 124 , 0x104CC030 ,         0x0         , 0x7E0           ,"Configure XG port 48 LED number to 0                                     "}
     ,{ 125 , 0x104CD030 ,         0x20        , 0x7E0           ,"Configure XG port 49 LED number to 1                                     "}
     ,{ 126 , 0x104CE030 ,         0x40        , 0x7E0           ,"Configure XG port 50 LED number to 2                                     "}
     ,{ 127 , 0x104CF030 ,         0x60        , 0x7E0           ,"Configure XG port 51 LED number to 3                                     "}
     ,{ 128 , 0x104D0030 ,         0x80        , 0x7E0           ,"Configure XG port 52 LED number to 4                                     "}
     ,{ 129 , 0x104D1030 ,         0xA0        , 0x7E0           ,"Configure XG port 53 LED number to 5                                     "}
     ,{ 130 , 0x104D2030 ,         0xC0        , 0x7E0           ,"Configure XG port 54 LED number to 6                                     "}
     ,{ 131 , 0x104D3030 ,         0xE0        , 0x7E0           ,"Configure XG port 55 LED number to 7                                     "}
     ,{ 132 , 0x104D4030 ,         0x100       , 0x7E0           ,"Configure XG port 56 LED number to 8                                     "}
     ,{ 133 , 0x104D5030 ,         0x120       , 0x7E0           ,"Configure XG port 57 LED number to 9                                     "}
     ,{ 134 , 0x104D6030 ,         0x140       , 0x7E0           ,"Configure XG port 58 LED number to 10                                    "}
     ,{ 135 , 0x104D7030 ,         0x160       , 0x7E0           ,"Configure XG port 59 LED number to 11                                    "}
     ,{ 136 , 0x104D8030 ,         0x180       , 0x7E0           ,"Configure XG port 60 LED number to 12                                    "}
     ,{ 137 , 0x104D9030 ,         0x1A0       , 0x7E0           ,"Configure XG port 61 LED number to 13                                    "}
     ,{ 138 , 0x104DA030 ,         0x1C0       , 0x7E0           ,"Configure XG port 62 LED number to 14                                    "}
     ,{ 139 , 0x104DB030 ,         0x1E0       , 0x7E0           ,"Configure XG port 63 LED number to 15                                    "}
     ,{ 140 , 0x104DC030 ,         0x200       , 0x7E0           ,"Configure XG port 64 LED number to 16                                    "}
     ,{ 141 , 0x104DD030 ,         0x220       , 0x7E0           ,"Configure XG port 65 LED number to 17                                    "}
     ,{ 142 , 0x104DE030 ,         0x240       , 0x7E0           ,"Configure XG port 66 LED number to 18                                    "}
     ,{ 143 , 0x104DF030 ,         0x260       , 0x7E0           ,"Configure XG port 67 LED number to 19                                    "}
     ,{ 144 , 0x104E0030 ,         0x280       , 0x7E0           ,"Configure XG port 68 LED number to 20                                    "}
     ,{ 145 , 0x104E1030 ,         0x2A0       , 0x7E0           ,"Configure XG port 69 LED number to 21                                    "}
     ,{ 146 , 0x104E2030 ,         0x2C0       , 0x7E0           ,"Configure XG port 70 LED number to 22                                    "}
     ,{ 147 , 0x104E3030 ,         0x2E0       , 0x7E0           ,"Configure XG port 71 LED number to 23                                    "}
     ,{ 148 , 0xA8000000 ,         0x0         , 0x10            ,"Configure LED Light High to active low                                   "}
     ,{ BAD_VALUE , BAD_VALUE  ,   BAD_VALUE   , BAD_VALUE       , NULL                                                                      }
};

GT_STATUS bc3LedRegPrint
(
    GT_U8 dev
)
{
    return asicUserRegListPrintAll(dev,&bc3_LED_CONFIG_48xXG_6xCG_2xXG[0]);
}



GT_STATUS bc3Led(GT_VOID)
{
    GT_STATUS rc;
    GT_U8 dev = 0;
    CPSS_PORTS_BMP_STC initPortsBmp;/* bitmap of ports to init */

    rc = appDemoDbEntryAdd ("bc2BoardType",0x90);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = cpssInitSystem(29,1,0);
    if (rc != GT_OK)
    {
        return rc;
    }

    cpssOsTimerWkAfter(1000);
    asicUserRegListPrintAll(dev,&bc3_LED_CONFIG_48xXG_6xCG_2xXG[0]);

    rc = appDemoBobcat3PortLedConfigPrint(dev);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* delete port , check that led position in all MACs is set to 0x3F */ 
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&initPortsBmp);
    CPSS_PORTS_BMP_PORT_SET_MAC(&initPortsBmp,4);
    rc = cpssDxChPortModeSpeedSet(dev, initPortsBmp, GT_FALSE,CPSS_PORT_INTERFACE_MODE_KR4_E, CPSS_PORT_SPEED_100G_E);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = appDemoBobcat3PortLedConfigPrint(dev);
    if (rc != GT_OK)
    {
        return rc;
    }


    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&initPortsBmp);
    CPSS_PORTS_BMP_PORT_SET_MAC(&initPortsBmp,24);
    rc = cpssDxChPortModeSpeedSet(dev, initPortsBmp, GT_FALSE,CPSS_PORT_INTERFACE_MODE_KR_E, CPSS_PORT_SPEED_10000_E);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = appDemoBobcat3PortLedConfigPrint(dev);
    if (rc != GT_OK)
    {
        return rc;
    }


    return GT_OK;
}

/*
config
port-map
ethernet 0/63 type cpu_sdma txq-port 95 
ethernet 0/0 type ethernet_mac txq-port 0 interface-number 0
ethernet 0/4 type ethernet_mac txq-port 4 interface-number 4
ethernet 0/38 type remote_port txq-port 38 interface-number 4
exit
end
*/


static CPSS_DXCH_PORT_MAP_STC bc3defaultMap_1[] =
{ /* Port,            mappingType                                 portGroupm,  intefaceNum,  txQPort,      trafficManegerEnable , tmPortInd*/
     {   0,           CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,           0,            0,        0,               GT_FALSE,         GT_NA }
    ,{   4,           CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,           0,            4,    GT_NA,               GT_FALSE,         GT_NA }
    ,{   37,  CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,           0,            4,       37,               GT_FALSE,         GT_NA }
    ,{   38,  CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,           0,            4,       38,               GT_FALSE,         GT_NA }
    ,{   39,  CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,           0,            4,       39,               GT_FALSE,         GT_NA }
    ,{   63,              CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,           0,        GT_NA,       95,               GT_FALSE,         GT_NA }
    ,{   80,              CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,           0,        GT_NA,      191,               GT_FALSE,         GT_NA }
    ,{   81,              CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,           0,        GT_NA,      383,               GT_FALSE,         GT_NA }
    ,{   82,              CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,           0,        GT_NA,      479,               GT_FALSE,         GT_NA }
};


GT_STATUS bc3Remote10G()
{
    GT_U8 dev = 0;
    GT_STATUS rc;
    GT_U32                      portMapArraySize;
    CPSS_DXCH_PORT_MAP_STC     *portMapArrayPtr;
    CPSS_PORTS_BMP_STC              portsBmp;

    rc = appDemoDbEntryAdd("initSerdesDefaults", 0);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = cpssInitSystem(29,1,0);
    if (rc != GT_OK)
    {
        return rc;
    }

    portMapArrayPtr  = &bc3defaultMap_1[0];
    portMapArraySize = sizeof(bc3defaultMap_1)/sizeof(bc3defaultMap_1[0]);

    rc = cpssDxChPortPhysicalPortMapSet(dev,  portMapArraySize, portMapArrayPtr);
    if (GT_OK != rc)
    {
        return rc;
    }

    rc = appDemoBobcat3PortMappingDump(dev);
    if (GT_OK != rc)
    {
        return rc;
    }

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
    CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp,0);

    rc = cpssDxChPortModeSpeedSet(dev, portsBmp, GT_TRUE,
                                CPSS_PORT_INTERFACE_MODE_KR4_E, CPSS_PORT_SPEED_100G_E);
    if(GT_OK != rc)
    {
        return rc;
    }


    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
    CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp,4);

    rc = cpssDxChPortModeSpeedSet(dev, portsBmp, GT_TRUE,
                                CPSS_PORT_INTERFACE_MODE_KR_E, CPSS_PORT_SPEED_25000_E);
    if(GT_OK != rc)
    {
        return rc;
    }

    cpssOsPrintf("\n+-----------------------------------------");
    cpssOsPrintf("\n| After cascade port 10G configuration    ");
    cpssOsPrintf("\n+-----------------------------------------");
    appDemoBobcat3PortGlobalResourcesPrint(dev);
    appDemoBobcat3PortListResourcesPrint(dev);

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
    CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp,37);
    CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp,39);
    CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp,38);

    rc = cpssDxChPortModeSpeedSet(dev, portsBmp, GT_TRUE,
                                CPSS_PORT_INTERFACE_MODE_KR_E, CPSS_PORT_SPEED_10000_E);
    if(GT_OK != rc)
    {
        return rc;
    }

    cpssOsPrintf("\n+-----------------------------------------");
    cpssOsPrintf("\n| After 10G remote configuration");
    cpssOsPrintf("\n+-----------------------------------------");
    appDemoBobcat3PortGlobalResourcesPrint(dev);
    appDemoBobcat3PortListResourcesPrint(dev);

    appDemoBobcat3PizzaArbiterDPDump(dev,0);
    appDemoBobcat3PortPizzaArbiterResourcesPrint(dev);
    return GT_OK;
}


GT_STATUS bc3newDevice()
{
    GT_U8 dev = 0;
    GT_STATUS rc;

    rc = prvCpssDrvDebugDeviceIdSet(dev, CPSS_98CX8410_CNS);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = cpssInitSystem(29,1,0);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}
