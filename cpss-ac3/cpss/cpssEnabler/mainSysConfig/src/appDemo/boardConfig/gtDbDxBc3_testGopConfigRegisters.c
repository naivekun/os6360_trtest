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
#include <appDemo/boardConfig/gtDbDxBobcat2GenUtils.h> 

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


extern GT_STATUS appDemoPizzaArbiterUnitsDefPrint
(
    GT_U8 dev
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#ifdef WIN32
    #include <stdio.h>
    #define cpssOsPrintf printf
#endif

typedef struct 
{
    GT_U32                            coreClock;
    GT_U32                            initSystemType;
    PortInitList_STC                 *portInitlistPtr;
    CPSS_DXCH_PORT_MAP_STC           *mapPtr;
    APPDEMO_UNIT_x_REG_x_VALUE_STC   *regList2Print;
}APPDEMO_ASIC_GOP_COMPARISON_SCENARIO_DATA_STC;

typedef struct 
{
    GT_U32                                           scenarioId;
    APPDEMO_ASIC_GOP_COMPARISON_SCENARIO_DATA_STC  scenarioData;
}APPDEMO_ASIC_woTM_GOP_COMPARISON_SCENARIO_STC;


#define BAD_VALUE  APPDEMO_BAD_VALUE

/* 
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
*/
#if 0
PRV_CPSS_DXCH_GLOBAL2LOCAL_DMA_CONV_STC  bc3_globalDma2Local_initList[] =
{    /* globalDma,       dp-index,  localDma */
      {      0,       {         0,         0 }    }
     ,{      1,       {         0,         1 }    }
     ,{      2,       {         0,         2 }    }
     ,{      3,       {         0,         3 }    }
     ,{      4,       {         0,         4 }    }
     ,{      5,       {         0,         5 }    }
     ,{      6,       {         0,         6 }    }
     ,{      7,       {         0,         7 }    }
     ,{      8,       {         0,         8 }    }
     ,{      9,       {         0,         9 }    }
     ,{     10,       {         0,        10 }    }
     ,{     11,       {         0,        11 }    }
     /*-----------------------------------------------*/
     ,{     12,       {         1,         0 }    }
     ,{     13,       {         1,         1 }    }
     ,{     14,       {         1,         2 }    }
     ,{     15,       {         1,         3 }    }
     ,{     16,       {         1,         4 }    }
     ,{     17,       {         1,         5 }    }
     ,{     18,       {         1,         6 }    }
     ,{     19,       {         1,         7 }    }
     ,{     20,       {         1,         8 }    }
     ,{     21,       {         1,         9 }    }
     ,{     22,       {         1,        10 }    }
     ,{     23,       {         1,        11 }    }
     /*-----------------------------------------------*/
     ,{     24,       {         2,         0 }    }
     ,{     25,       {         2,         1 }    }
     ,{     26,       {         2,         2 }    }
     ,{     27,       {         2,         3 }    }
     ,{     28,       {         2,         4 }    }
     ,{     29,       {         2,         5 }    }
     ,{     30,       {         2,         6 }    }
     ,{     31,       {         2,         7 }    }
     ,{     32,       {         2,         8 }    }
     ,{     33,       {         2,         9 }    }
     ,{     34,       {         2,        10 }    }
     ,{     35,       {         2,        11 }    }
     /*-----------------------------------------------*/
     ,{     36,       {         3,         0 }    }
     ,{     37,       {         3,         1 }    }
     ,{     38,       {         3,         2 }    }
     ,{     39,       {         3,         3 }    }
     ,{     40,       {         3,         4 }    }
     ,{     41,       {         3,         5 }    }
     ,{     42,       {         3,         6 }    }
     ,{     43,       {         3,         7 }    }
     ,{     44,       {         3,         8 }    }
     ,{     45,       {         3,         9 }    }
     ,{     46,       {         3,        10 }    }
     ,{     47,       {         3,        11 }    }
     /*-----------------------------------------------*/
     ,{     48,       {         4,         0 }    }
     ,{     49,       {         4,         1 }    }
     ,{     50,       {         4,         2 }    }
     ,{     51,       {         4,         3 }    }
     ,{     52,       {         4,         4 }    }
     ,{     53,       {         4,         5 }    }
     ,{     54,       {         4,         6 }    }
     ,{     55,       {         4,         7 }    }
     ,{     56,       {         4,         8 }    }
     ,{     57,       {         4,         9 }    }
     ,{     58,       {         4,        10 }    }
     ,{     59,       {         4,        11 }    }
     /*-----------------------------------------------*/
     ,{     60,       {         5,         0 }    }
     ,{     61,       {         5,         1 }    }
     ,{     62,       {         5,         2 }    }
     ,{     63,       {         5,         3 }    }
     ,{     64,       {         5,         4 }    }
     ,{     65,       {         5,         5 }    }
     ,{     66,       {         5,         6 }    }
     ,{     67,       {         5,         7 }    }
     ,{     68,       {         5,         8 }    }
     ,{     69,       {         5,         9 }    }
     ,{     70,       {         5,        10 }    }
     ,{     71,       {         5,        11 }    }
     /*-----------------------------------------------*/
     ,{     72,       {         0,        12 }    } /* CPU */
     ,{  BAD_VALUE,   { BAD_VALUE, BAD_VALUE }    }
};
#endif

#if 0
    static CPSS_DXCH_PORT_MAP_STC bc3_HwVsSf_Map[] =
    { /* Port,               mappingType                             portGroupm, intefaceNum,    txQPort,         TM , tmPortInd*/              /* globalDma,       dp-index,  localDma */
    /*DP[0]*/
    /* 0*/     {  48,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,            0,         0,       GT_FALSE,   GT_NA}        /* {      0,       {         0,         0 }    } */
    /* 4*/    ,{  49,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,            4,         4,       GT_FALSE,   GT_NA}        /* {      4,       {         0,         4 }    } */
    /* 8*/    ,{  50,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,            8,         8,       GT_FALSE,   GT_NA}        /* {      8,       {         0,         8 }    } */
    /*DP[1]*/
    /* 0*/    ,{  51,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,           12,        96,       GT_FALSE,   GT_NA}        /* {     12,       {         0,         0 }    } */
    /* 4*/    ,{  52,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,           16,       100,       GT_FALSE,   GT_NA}        /* {     16,       {         0,         4 }    } */
    /* 8*/    ,{  53,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,           20,       104,       GT_FALSE,   GT_NA}        /* {     20,       {         0,         8 }    } */
    /*DP[2]*/
    /* 0*/    ,{   0,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,           24,       192,       GT_FALSE,   GT_NA}        /* {     24,       {         2,         0 }    } */
    /* 1*/    ,{   1,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,           25,       193,       GT_FALSE,   GT_NA}        /* {     25,       {         2,         1 }    } */
    /* 2*/    ,{   2,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,           26,       194,       GT_FALSE,   GT_NA}        /* {     26,       {         2,         2 }    } */
    /* 3*/    ,{   3,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,           27,       195,       GT_FALSE,   GT_NA}        /* {     27,       {         2,         3 }    } */
    /* 4*/    ,{   4,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,           28,       196,       GT_FALSE,   GT_NA}        /* {     28,       {         2,         4 }    } */
    /* 5*/    ,{   5,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,           29,       197,       GT_FALSE,   GT_NA}        /* {     29,       {         2,         5 }    } */
    /* 6*/    ,{   6,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,           30,       198,       GT_FALSE,   GT_NA}        /* {     30,       {         2,         6 }    } */
    /* 7*/    ,{   7,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,           31,       199,       GT_FALSE,   GT_NA}        /* {     31,       {         2,         7 }    } */
    /* 8*/    ,{   8,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,           32,       200,       GT_FALSE,   GT_NA}        /* {     32,       {         2,         8 }    } */
    /* 9*/    ,{   9,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,           33,       201,       GT_FALSE,   GT_NA}        /* {     33,       {         2,         9 }    } */
    /*10*/    ,{  10,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,           34,       202,       GT_FALSE,   GT_NA}        /* {     34,       {         2,        10 }    } */
    /*11*/    ,{  11,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,           35,       203,       GT_FALSE,   GT_NA}        /* {     35,       {         2,        11 }    } */
    /*DP[3]*/
    /* 0*/    ,{  12,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,           36,       288,       GT_FALSE,   GT_NA}        /* {     36,       {         3,         0 }    } */
    /* 1*/    ,{  13,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,           37,       289,       GT_FALSE,   GT_NA}        /* {     37,       {         3,         1 }    } */
    /* 2*/    ,{  14,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,           38,       290,       GT_FALSE,   GT_NA}        /* {     38,       {         3,         2 }    } */
    /* 3*/    ,{  15,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,           39,       291,       GT_FALSE,   GT_NA}        /* {     39,       {         3,         3 }    } */
    /* 4*/    ,{  16,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,           40,       292,       GT_FALSE,   GT_NA}        /* {     40,       {         3,         4 }    } */
    /* 5*/    ,{  17,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,           41,       293,       GT_FALSE,   GT_NA}        /* {     41,       {         3,         5 }    } */
    /* 6*/    ,{  18,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,           42,       294,       GT_FALSE,   GT_NA}        /* {     42,       {         3,         6 }    } */
    /* 7*/    ,{  19,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,           43,       295,       GT_FALSE,   GT_NA}        /* {     43,       {         3,         7 }    } */
    /* 8*/    ,{  20,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,           44,       296,       GT_FALSE,   GT_NA}        /* {     44,       {         3,         8 }    } */
    /* 9*/    ,{  21,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,           45,       297,       GT_FALSE,   GT_NA}        /* {     45,       {         3,         9 }    } */
    /*10*/    ,{  22,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,           46,       298,       GT_FALSE,   GT_NA}        /* {     46,       {         3,        10 }    } */
    /*11*/    ,{  23,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,           47,       299,       GT_FALSE,   GT_NA}        /* {     47,       {         3,        11 }    } */
    /*DP[4]*/
    /* 0*/    ,{  24,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,           48,       384,       GT_FALSE,   GT_NA}        /* {     48,       {         4,         0 }    } */
    /* 1*/    ,{  25,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,           49,       385,       GT_FALSE,   GT_NA}        /* {     49,       {         4,         1 }    } */
    /* 2*/    ,{  26,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,           50,       386,       GT_FALSE,   GT_NA}        /* {     50,       {         4,         2 }    } */
    /* 3*/    ,{  27,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,           51,       387,       GT_FALSE,   GT_NA}        /* {     51,       {         4,         3 }    } */
    /* 4*/    ,{  28,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,           52,       388,       GT_FALSE,   GT_NA}        /* {     52,       {         4,         4 }    } */
    /* 5*/    ,{  29,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,           53,       389,       GT_FALSE,   GT_NA}        /* {     53,       {         4,         5 }    } */
    /* 6*/    ,{  30,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,           54,       390,       GT_FALSE,   GT_NA}        /* {     54,       {         4,         6 }    } */
    /* 7*/    ,{  31,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,           55,       391,       GT_FALSE,   GT_NA}        /* {     55,       {         4,         7 }    } */
    /* 8*/    ,{  32,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,           56,       392,       GT_FALSE,   GT_NA}        /* {     56,       {         4,         8 }    } */
    /* 9*/    ,{  33,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,           57,       393,       GT_FALSE,   GT_NA}        /* {     57,       {         4,         9 }    } */
    /*10*/    ,{  34,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,           58,       394,       GT_FALSE,   GT_NA}        /* {     58,       {         4,        10 }    } */
    /*11*/    ,{  35,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,           59,       395,       GT_FALSE,   GT_NA}        /* {     59,       {         4,        11 }    } */
    /*DP[5]*/
    /* 0*/    ,{  36,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,           60,       480,       GT_FALSE,   GT_NA}        /* {     60,       {         5,         0 }    } */
    /* 1*/    ,{  37,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,           61,       481,       GT_FALSE,   GT_NA}        /* {     61,       {         5,         1 }    } */
    /* 2*/    ,{  38,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,           62,       482,       GT_FALSE,   GT_NA}        /* {     62,       {         5,         2 }    } */
    /* 3*/    ,{  39,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,           63,       483,       GT_FALSE,   GT_NA}        /* {     63,       {         5,         3 }    } */
    /* 4*/    ,{  40,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,           64,       484,       GT_FALSE,   GT_NA}        /* {     64,       {         5,         4 }    } */
    /* 5*/    ,{  41,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,           65,       485,       GT_FALSE,   GT_NA}        /* {     65,       {         5,         5 }    } */
    /* 6*/    ,{  42,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,           66,       486,       GT_FALSE,   GT_NA}        /* {     66,       {         5,         6 }    } */
    /* 7*/    ,{  43,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,           67,       487,       GT_FALSE,   GT_NA}        /* {     67,       {         5,         7 }    } */
    /* 8*/    ,{  44,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,           68,       488,       GT_FALSE,   GT_NA}        /* {     68,       {         5,         8 }    } */
    /* 9*/    ,{  45,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,           69,       489,       GT_FALSE,   GT_NA}        /* {     69,       {         5,         9 }    } */
    /*10*/    ,{  46,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,           70,       490,       GT_FALSE,   GT_NA}        /* {     70,       {         5,        10 }    } */
    /*11*/    ,{  47,    CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,          0,           71,       491,       GT_FALSE,   GT_NA}        /* {     71,       {         5,        11 }    } */
    /*12*/    ,{  63,        CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,          0,        GT_NA,        12,       GT_FALSE,   GT_NA}        /* {     72,       {         0,        12 }    } */
              ,{ BAD_VALUE,   CPSS_DXCH_PORT_MAPPING_TYPE_INVALID_E,  BAD_VALUE,    BAD_VALUE, BAD_VALUE,       GT_FALSE, BAD_VALUE}
    };


    static PortInitList_STC bc3_M3_portInitlist_48x25G_6x100G_CPU[] =
    {
        /* core 0 */
         { PORT_LIST_TYPE_LIST,      { 48, 49, 50,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_100G_E,  CPSS_PORT_INTERFACE_MODE_KR4_E      } /* DP 0 */
        ,{ PORT_LIST_TYPE_LIST,      { 51, 52, 53,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_100G_E,  CPSS_PORT_INTERFACE_MODE_KR4_E      } /* DP 1 */
        ,{ PORT_LIST_TYPE_INTERVAL,  {  0, 47,  1,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E, CPSS_PORT_INTERFACE_MODE_KR_E       } /* DP 2,3,4,5 */
        ,{ PORT_LIST_TYPE_EMPTY,     {                 APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,    CPSS_PORT_INTERFACE_MODE_NA_E       }
    };

    static PortInitList_STC bc3_M5_portInitlist_48x10G_6x100G_CPU[] =
    {
        /* core 0 */
         { PORT_LIST_TYPE_LIST,      { 48, 49, 50,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_100G_E,  CPSS_PORT_INTERFACE_MODE_KR4_E      } /* DP 0 */
        ,{ PORT_LIST_TYPE_LIST,      { 51, 52, 53,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_100G_E,  CPSS_PORT_INTERFACE_MODE_KR4_E      } /* DP 1 */
        ,{ PORT_LIST_TYPE_INTERVAL,  {  0, 47,  1,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E, CPSS_PORT_INTERFACE_MODE_KR_E       } /* DP 2,3,4,5 */
        ,{ PORT_LIST_TYPE_EMPTY,     {                 APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,    CPSS_PORT_INTERFACE_MODE_NA_E       }
    };
#endif

static PortInitList_STC bc3_M3_portInitlist_48x25G_6x100G_CPU[] =
{
    /* core 0 */
     { PORT_LIST_TYPE_LIST,      {  0,  4,  8,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_100G_E,  CPSS_PORT_INTERFACE_MODE_KR4_E      } /* DP 0 */
    ,{ PORT_LIST_TYPE_LIST,      { 12, 16, 20,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_100G_E,  CPSS_PORT_INTERFACE_MODE_KR4_E      } /* DP 1 */
    ,{ PORT_LIST_TYPE_INTERVAL,  { 24, 35,  1,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E, CPSS_PORT_INTERFACE_MODE_KR_E       } /* DP 2 */
    ,{ PORT_LIST_TYPE_INTERVAL,  { 36, 47,  1,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E, CPSS_PORT_INTERFACE_MODE_KR_E       } /* DP 3 */
    ,{ PORT_LIST_TYPE_LIST,      { 72,             APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E, CPSS_PORT_INTERFACE_MODE_KR_E       } /* DP 3 */
    ,{ PORT_LIST_TYPE_INTERVAL,  { 48, 58,  1,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E, CPSS_PORT_INTERFACE_MODE_KR_E       } /* DP 4 */
    ,{ PORT_LIST_TYPE_LIST,      { 79,             APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E, CPSS_PORT_INTERFACE_MODE_KR_E       } /* DP 4 */
    ,{ PORT_LIST_TYPE_INTERVAL,  { 64, 71,  1,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E, CPSS_PORT_INTERFACE_MODE_KR_E       } /* DP 5 */
    ,{ PORT_LIST_TYPE_INTERVAL,  { 80, 83,  1,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E, CPSS_PORT_INTERFACE_MODE_KR_E       } /* DP 5 */
    ,{ PORT_LIST_TYPE_LIST,      { 73,             APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E, CPSS_PORT_INTERFACE_MODE_KR_E       } /* DP 5 */
    ,{ PORT_LIST_TYPE_EMPTY,     {                 APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,    CPSS_PORT_INTERFACE_MODE_NA_E       }
};

static PortInitList_STC bc3_M5_portInitlist_48x10G_6x100G_CPU[] =
{
    /* core 0 */
     { PORT_LIST_TYPE_LIST,      {  0,  4,  8,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_100G_E,  CPSS_PORT_INTERFACE_MODE_KR4_E      } /* DP 0 */
    ,{ PORT_LIST_TYPE_LIST,      { 12, 16, 20,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_100G_E,  CPSS_PORT_INTERFACE_MODE_KR4_E      } /* DP 1 */
    ,{ PORT_LIST_TYPE_INTERVAL,  { 24, 35,  1,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E, CPSS_PORT_INTERFACE_MODE_KR_E       } /* DP 2 */
    ,{ PORT_LIST_TYPE_INTERVAL,  { 36, 47,  1,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E, CPSS_PORT_INTERFACE_MODE_KR_E       } /* DP 3 */
    ,{ PORT_LIST_TYPE_INTERVAL,  { 48, 58,  1,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E, CPSS_PORT_INTERFACE_MODE_KR_E       } /* DP 4 */
    ,{ PORT_LIST_TYPE_LIST,      { 79,             APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E, CPSS_PORT_INTERFACE_MODE_KR_E       } /* DP 4 */
    ,{ PORT_LIST_TYPE_INTERVAL,  { 64, 71,  1,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E, CPSS_PORT_INTERFACE_MODE_KR_E       } /* DP 5 */
    ,{ PORT_LIST_TYPE_INTERVAL,  { 80, 83,  1,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E, CPSS_PORT_INTERFACE_MODE_KR_E       } /* DP 5 */
    ,{ PORT_LIST_TYPE_EMPTY,     {                 APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,    CPSS_PORT_INTERFACE_MODE_NA_E       }
};



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern APPDEMO_UNIT_x_REG_x_VALUE_STC bc3_M3_regList_6x100G_48x25G_CPU[];
extern APPDEMO_UNIT_x_REG_x_VALUE_STC bc3_M5_regList_6x100G_48x10G_CPU[];


#ifdef __cplusplus
}
#endif /* __cplusplus */


static APPDEMO_ASIC_woTM_GOP_COMPARISON_SCENARIO_STC bc3_scenarioList[] = 
{  /* scenario Id,     coreClock,   InitSys,   port list to init,                                                  mapping,          regList2Print               */
     {          3,   {       525,         1,   &bc3_M3_portInitlist_48x25G_6x100G_CPU[0],                             NULL,     &bc3_M3_regList_6x100G_48x25G_CPU[0]  } }
    ,{          5,   {       525,         1,   &bc3_M5_portInitlist_48x10G_6x100G_CPU[0],                             NULL,     &bc3_M5_regList_6x100G_48x10G_CPU[0]  } }
    ,{      60003,   {       600,         1,   &bc3_M3_portInitlist_48x25G_6x100G_CPU[0],                             NULL,     &bc3_M3_regList_6x100G_48x25G_CPU[0]  } }
    ,{      60005,   {       600,         1,   &bc3_M5_portInitlist_48x10G_6x100G_CPU[0],                             NULL,     &bc3_M5_regList_6x100G_48x10G_CPU[0]  } }
    ,{      62503,   {       625,         1,   &bc3_M3_portInitlist_48x25G_6x100G_CPU[0],                             NULL,     &bc3_M3_regList_6x100G_48x25G_CPU[0]  } }
    ,{      62505,   {       625,         1,   &bc3_M5_portInitlist_48x10G_6x100G_CPU[0],                             NULL,     &bc3_M5_regList_6x100G_48x10G_CPU[0]  } }
    ,{  BAD_VALUE,   { BAD_VALUE, BAD_VALUE,   (PortInitList_STC*)NULL,                      (CPSS_DXCH_PORT_MAP_STC*)NULL,     (APPDEMO_UNIT_x_REG_x_VALUE_STC*)NULL } }
};



GT_STATUS bc3UserScen
(
    GT_U32 scenario,
    GT_U32 printReg
)
{
    GT_STATUS rc;
    GT_U8 dev = 0;
    APPDEMO_ASIC_GOP_COMPARISON_SCENARIO_DATA_STC * scenarioDataPtr = (APPDEMO_ASIC_GOP_COMPARISON_SCENARIO_DATA_STC*)NULL;

    GT_U32 i;
    GT_U32 mapSize = 0;

    for (i = 0 ; bc3_scenarioList[i].scenarioId != BAD_VALUE; i++)
    {
        if (bc3_scenarioList[i].scenarioId == scenario)
        {
            scenarioDataPtr = &bc3_scenarioList[i].scenarioData;
            break;
        }
    }

    if (scenarioDataPtr == NULL)  /* not found */
    {
        cpssOsPrintf("\n-->ERROR BC3 scenario %d : : Undefined scenario",scenario );
        cpssOsPrintf("\n");
        return GT_NOT_FOUND;
    }

    #ifdef ASIC_SIMULATION
        rc = appDemoBobKSimCoreClockSet(dev,scenarioDataPtr->coreClock);
        if (rc != GT_OK)
        {
            return rc;
        }
    #endif
    rc = appDemoDbEntryAdd("initSerdesDefaults", 0);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = cpssInitSystem(29,scenarioDataPtr->initSystemType,0);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = appDemoBobcat3PortListResourcesPrint(dev);
    if (rc != GT_OK)
    {
        return rc;
    }


    if (scenarioDataPtr->mapPtr != NULL)
    {
        if (scenarioDataPtr->mapPtr != NULL)
        {
            /*------------------------------------------------------------------------*
             * count number of valid lines till CPSS_DXCH_PORT_MAPPING_TYPE_INVALID_E *
             *------------------------------------------------------------------------*/
            for (i = 0 ; scenarioDataPtr->mapPtr[i].mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_INVALID_E; i++);
            mapSize = i;

            /*------------------------------------------------------------------------*
             * Set new mapping                                                        *
             *------------------------------------------------------------------------*/
            rc = cpssDxChPortPhysicalPortMapSet(dev,  mapSize, scenarioDataPtr->mapPtr);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortPhysicalPortMapSet", rc);
            if (GT_OK != rc)
            {
                return rc;
            }
            /* update port mapping in appDemo DB */
            rc = appDemoDxChMaxMappedPortSet(dev,  mapSize, scenarioDataPtr->mapPtr);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDxChMaxMappedPortSet", rc);
            if (GT_OK != rc)
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

    rc = appDemoBc2PortListInit(dev,scenarioDataPtr->portInitlistPtr,GT_FALSE);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (printReg)
    {

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
        if (scenarioDataPtr->regList2Print != NULL)
        {
            rc = asicUserScenRegListPrint(scenarioDataPtr->regList2Print);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        */
        cpssOsPrintf("\n\n");
    }
    return GT_OK;
}





