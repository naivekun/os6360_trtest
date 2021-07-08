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
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortDynamicPAPortSpeedDB.h>  
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

extern GT_STATUS gtBobcat2PortMappingDump
(
    IN  GT_U8  dev
);


extern GT_STATUS appDemoBobk2PortListMacPcsStatePrint
(
    IN GT_U8 dev
);

extern GT_STATUS appDemoBobk2PortGobalResourcesPrint
(
    IN GT_U8 dev
);


extern GT_STATUS appDemoBobk2PortListResourcesPrint
(
    IN GT_U8 dev
);

extern GT_STATUS gtBobcat2TxQHighSpeedPortPrint
(
    IN  GT_U8  dev
);

extern GT_STATUS gtBobcat2TxQHighSpeed_DQ_LL_Dump
(
    IN  GT_U8  dev
);


extern void appDemoBc2SetCustomPortsMapping
(
    IN GT_BOOL enable
);

extern GT_STATUS appDemoBc2TmPortsInit
(
    IN GT_U8 dev
);



extern GT_STATUS appDemoPizzaArbiterUnitsDefPrint
(
    GT_U8 dev
);


/* 
    API to configure TmPorts, with non default ports mapping
*/
extern void appDemoBc2SetCustomPortsMapping(GT_BOOL enable);
extern GT_STATUS appDemoBc2TmPortsInit(GT_U8 dev);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#ifdef WIN32
    #include <stdio.h>
    #define cpssOsPrintf printf
#endif

#define BAD_VALUE APPDEMO_BAD_VALUE

APPDEMO_UNIT_x_REG_x_VALUE_STC cetus_LedReg[] = 
{
    /*                                unit_name,               address,   data0,     reg_name */
      {    1 , "XLG MAC / XG MibCounter 56 0x0102C0030" , 0x0102C0030 , 0x0000000C , "XLG MAC / XG MibCounter 56 0x0102C0030" , "--" }
    , {    2 , "XLG MAC / XG MibCounter 57 0x0102C1030" , 0x0102C1030 , 0x0000006C , "XLG MAC / XG MibCounter 57 0x0102C1030" , "--" }
    , {    3 , "XLG MAC / XG MibCounter 58 0x0102C2030" , 0x0102C2030 , 0x0000002C , "XLG MAC / XG MibCounter 58 0x0102C2030" , "--" }
    , {    4 , "XLG MAC / XG MibCounter 59 0x0102C3030" , 0x0102C3030 , 0x0000004C , "XLG MAC / XG MibCounter 59 0x0102C3030" , "--" }
    , {    5 , "XLG MAC / XG MibCounter 64 0x0102C8030" , 0x0102C8030 , 0x0000010C , "XLG MAC / XG MibCounter 64 0x0102C8030" , "--" }
    , {    6 , "XLG MAC / XG MibCounter 65 0x0102C9030" , 0x0102C9030 , 0x0000016C , "XLG MAC / XG MibCounter 65 0x0102C9030" , "--" }
    , {    7 , "XLG MAC / XG MibCounter 66 0x0102CA030" , 0x0102CA030 , 0x0000012C , "XLG MAC / XG MibCounter 66 0x0102CA030" , "--" }
    , {    8 , "XLG MAC / XG MibCounter 67 0x0102CB030" , 0x0102CB030 , 0x0000014C , "XLG MAC / XG MibCounter 67 0x0102CB030" , "--" }
    , {    9 , "XLG MAC / XG MibCounter 68 0x0102CC030" , 0x0102CC030 , 0x0000008C , "XLG MAC / XG MibCounter 68 0x0102CC030" , "--" }
    , {   10 , "XLG MAC / XG MibCounter 69 0x0102CD030" , 0x0102CD030 , 0x000000EC , "XLG MAC / XG MibCounter 69 0x0102CD030" , "--" }
    , {   11 , "XLG MAC / XG MibCounter 70 0x0102CE030" , 0x0102CE030 , 0x000000AC , "XLG MAC / XG MibCounter 70 0x0102CE030" , "--" }
    , {   12 , "XLG MAC / XG MibCounter 71 0x0102CF030" , 0x0102CF030 , 0x000000CC , "XLG MAC / XG MibCounter 71 0x0102CF030" , "--" }
    , {   13 , "0x050000000                           " , 0x050000000 , 0x00FF0035 , "0x050000000                           " , "--" }
    , {   14 , "0x050000008                           " , 0x050000008 , 0x00000000 , "0x050000008                           " , "--" }
    , {   15 , "0x050000004                           " , 0x050000004 , 0x0010012A , "0x050000004                           " , "--" }
    , {   16 , "0x05000000C                           " , 0x05000000C , 0x00249249 , "0x05000000C                           " , "--" }
    , { BAD_VALUE, (GT_CHAR *)NULL                      ,   BAD_VALUE ,  BAD_VALUE ,  (GT_CHAR *)NULL                         ,  (GT_CHAR *)NULL }
};


GT_STATUS cetusTestLeds
(
    GT_VOID
)
{
    GT_STATUS rc;
    rc = cpssInitSystem(29,1,0);
    if (rc != GT_OK)
    {
        return rc;
    }
    rc = asicUserScenRegListPrint(&cetus_LedReg[0]);
    if (rc != GT_OK)
    {
        return rc;
    }
    cpssOsPrintf("\n\n");
    return GT_OK;
}

/*---------------------------------------------------------------*
 *     Core Clock: 365MHz:                                       *
 *     o    48G and 40G should be configured as fast port        *
 *     Core Clock: 250MHz:                                       *
 *     o    48G, 40G and 24G should be configured as fast port   *
 *     Core Clock: 200MHz:                                       *
 *     o    40G, 24G and 20G should be configured as fast port   *
 *     o    48G isn’t supported in this core clock.              *
 *     Core Clock: 167MHz:                                       *
 *     o    24G and 20G should be configured as fast port        *
 *     o    48G and 40G aren’t supported in this core clock.     *
 *---------------------------------------------------------------*/
GT_STATUS cetusTestTxQ_365Mhz
(
    GT_U32  coreClock
)
{
    typedef struct 
    {
        GT_U32 coreClock;
        PortInitList_STC * initListPtr;
    }coreClock_x_PortInitList_STC;


    static PortInitList_STC bobkCetus_portInitlist_4x10G_2x40G[] = 
    {
         { PORT_LIST_TYPE_LIST,      { 56,57,58,59, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
        ,{ PORT_LIST_TYPE_LIST,      { 64,68,       APP_INV_PORT_CNS }, CPSS_PORT_SPEED_40000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
        ,{ PORT_LIST_TYPE_EMPTY,     {              APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E      }
    };

    static PortInitList_STC bobkCetus_portInitlist_4x10G_2x20G[] = 
    {
         { PORT_LIST_TYPE_LIST,      { 56,57,58,59, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
        ,{ PORT_LIST_TYPE_LIST,      { 64,68,       APP_INV_PORT_CNS }, CPSS_PORT_SPEED_20000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
        ,{ PORT_LIST_TYPE_EMPTY,     {              APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E      }
    };

    static PortInitList_STC bobkCetus_portInitlist_4x10G_1x20G[] = 
    {
         { PORT_LIST_TYPE_LIST,      { 56,57,58,59, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
        ,{ PORT_LIST_TYPE_LIST,      { 64,          APP_INV_PORT_CNS }, CPSS_PORT_SPEED_20000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
        ,{ PORT_LIST_TYPE_EMPTY,     {              APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E      }
    };


    static coreClock_x_PortInitList_STC coreClock_x_PortInitList_Arr[] = 
    {
         { 167,  &bobkCetus_portInitlist_4x10G_1x20G[0] }
        ,{ 200,  &bobkCetus_portInitlist_4x10G_2x20G[0] }
        ,{ 250,  &bobkCetus_portInitlist_4x10G_2x40G[0] }
        ,{ 365,  &bobkCetus_portInitlist_4x10G_2x40G[0] }
        ,{   0,  NULL                                   }
    };

    GT_U8 dev = 0;
    GT_STATUS rc;
    GT_U32    i;


    #ifdef ASIC_SIMULATION
        rc = appDemoBobKSimCoreClockSet(dev,coreClock);
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


    rc = cpssInitSystem(29,1,0);
    if (rc != GT_OK)
    {
        return rc;
    }

    for (i = 0 ;  coreClock_x_PortInitList_Arr[i].coreClock > 0; i++)
    {
        if (coreClock_x_PortInitList_Arr[i].coreClock == coreClock)
        {
            break;
        }
    }
    if (coreClock_x_PortInitList_Arr[i].coreClock != coreClock)
    {
        cpssOsPrintf("\n--> ERR : coreclockk %d Mhz : PortInitList not fpund", coreClock);
        return GT_NOT_IMPLEMENTED;
    }
    rc = appDemoBc2PortListInit(dev,coreClock_x_PortInitList_Arr[i].initListPtr,GT_FALSE);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = appDemoBobk2PortListResourcesPrint(dev);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = gtBobcat2PortPizzaArbiterIfStateDump(dev,0);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = gtBobcat2TxQHighSpeedPortPrint(dev);
    if (rc != GT_OK)
    {
        return rc;
    }

    cpssOsPrintf("\n\n");
    return GT_OK;
}


GT_STATUS cetusTestTxQ_HighSpeedPorts
(
    GT_U32  coreClock
)
{
    CPSS_PORTS_BMP_STC initPortsBmp;/* bitmap of ports to init */
    GT_U8 dev = 0;
    GT_STATUS rc;


    #ifdef ASIC_SIMULATION
        rc = appDemoBobKSimCoreClockSet(dev,coreClock);
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


    rc = cpssInitSystem(29,1,0);
    if (rc != GT_OK)
    {
        return rc;
    }


    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&initPortsBmp);
    CPSS_PORTS_BMP_PORT_SET_MAC(&initPortsBmp,56);
    CPSS_PORTS_BMP_PORT_SET_MAC(&initPortsBmp,64);

    rc = cpssDxChPortModeSpeedSet(dev, initPortsBmp, GT_TRUE,
                                    CPSS_PORT_INTERFACE_MODE_KR_E,
                                    CPSS_PORT_SPEED_40000_E);

    if(rc != GT_OK)
    {
        return rc;
    }

    rc = appDemoBobk2PortListResourcesPrint(dev);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*
    rc = gtBobcat2PortPizzaArbiterIfStateDump(dev,0);
    if (rc != GT_OK)
    {
        return rc;
    }
    */

    rc = gtBobcat2TxQHighSpeedPortPrint(dev);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = gtBobcat2TxQHighSpeed_DQ_LL_Dump(dev);
    if (rc != GT_OK)
    {
        return rc;
    }

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&initPortsBmp);
    CPSS_PORTS_BMP_PORT_SET_MAC(&initPortsBmp,56);

    rc = cpssDxChPortModeSpeedSet(dev, initPortsBmp, GT_FALSE,
                                    CPSS_PORT_INTERFACE_MODE_KR_E,
                                    CPSS_PORT_SPEED_40000_E);


    rc = appDemoBobk2PortListResourcesPrint(dev);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = gtBobcat2TxQHighSpeed_DQ_LL_Dump(dev);
    if (rc != GT_OK)
    {
        return rc;
    }
    cpssOsPrintf("\n\n");
    return GT_OK;
}



GT_STATUS user_defined_scenario_tm
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U8 dev = 0;


    static CPSS_DXCH_PORT_MAP_STC cetusMap[] =
    { /* Port,            mappingType                           portGroupm, intefaceNum, txQPort, trafficManegerEnable , tmPortInd*/
         {   0,  CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,              0,        0,       0,           GT_TRUE,              0}
        ,{   1,  CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,              0,        1,       1,           GT_TRUE,              1}
        ,{   2,  CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,              0,        2,       2,           GT_TRUE,              2}
        ,{   3,  CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,              0,        3,       3,           GT_TRUE,              3}
        ,{   4,  CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,              0,        4,       4,           GT_TRUE,              4}
        ,{   5,  CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,              0,        5,       5,           GT_TRUE,              5}
        ,{   6,  CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,              0,        6,       6,           GT_TRUE,              6}
        ,{   7,  CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,              0,        7,       7,           GT_TRUE,              7}
        ,{   8,  CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,              0,        8,       8,           GT_TRUE,              8}
        ,{   9,  CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,              0,        9,       9,           GT_TRUE,              9}
        ,{   10, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,              0,       10,      10,           GT_TRUE,             10}
        ,{   11, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,              0,       11,      11,           GT_TRUE,             11}
        ,{   12, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,              0,       12,      12,           GT_TRUE,             12}
        ,{   13, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,              0,       13,      13,           GT_TRUE,             13}
        ,{   14, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,              0,       14,      14,           GT_TRUE,             14}
        ,{   15, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,              0,       15,      15,           GT_TRUE,             15}
        ,{   16, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,              0,       16,      16,           GT_TRUE,             16}
        ,{   17, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,              0,       17,      17,           GT_TRUE,             17}
        ,{   18, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,              0,       18,      18,           GT_TRUE,             18}
        ,{   19, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,              0,       19,      19,           GT_TRUE,             19}
        ,{   20, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,              0,       20,      20,           GT_TRUE,             20}
        ,{   21, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,              0,       21,      21,           GT_TRUE,             21}
        ,{   22, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,              0,       22,      22,           GT_TRUE,             22}
        ,{   23, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,              0,       23,      23,           GT_TRUE,             23}
        ,{   24, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,              0,       24,      24,           GT_FALSE,          GT_NA}
        ,{   25, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,              0,       25,      25,           GT_FALSE,          GT_NA}
        ,{   26, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,              0,       26,      26,           GT_FALSE,          GT_NA}
        ,{   27, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,              0,       27,      27,           GT_FALSE,          GT_NA}
        ,{   28, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,              0,       28,      28,           GT_FALSE,          GT_NA}
        ,{   29, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,              0,       29,      29,           GT_FALSE,          GT_NA}
        ,{   30, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,              0,       30,      30,           GT_FALSE,          GT_NA}
        ,{   31, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,              0,       31,      31,           GT_FALSE,          GT_NA}
        ,{   32, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,              0,       32,      32,           GT_FALSE,          GT_NA}
        ,{   33, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,              0,       33,      33,           GT_FALSE,          GT_NA}
        ,{   34, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,              0,       34,      34,           GT_FALSE,          GT_NA}
        ,{   35, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,              0,       35,      35,           GT_FALSE,          GT_NA}
        ,{   36, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,              0,       36,      36,           GT_FALSE,          GT_NA}
        ,{   37, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,              0,       37,      37,           GT_FALSE,          GT_NA}
        ,{   38, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,              0,       38,      38,           GT_FALSE,          GT_NA}
        ,{   39, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,              0,       39,      39,           GT_FALSE,          GT_NA}
        ,{   40, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,              0,       40,      40,           GT_FALSE,          GT_NA}
        ,{   41, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,              0,       41,      41,           GT_FALSE,          GT_NA}
        ,{   42, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,              0,       42,      42,           GT_FALSE,          GT_NA}
        ,{   43, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,              0,       43,      43,           GT_FALSE,          GT_NA}
        ,{   44, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,              0,       44,      44,           GT_FALSE,          GT_NA}
        ,{   45, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,              0,       45,      45,           GT_FALSE,          GT_NA}
        ,{   46, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,              0,       46,      46,           GT_FALSE,          GT_NA}
  /*      ,{   47, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,              0,       47,      47,           GT_FALSE,          GT_NA}*/

        ,{  56, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       56,   56,            GT_FALSE,             GT_NA}
        ,{  57, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       57,   57,            GT_FALSE,             GT_NA}
        ,{  58, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       58,   58,            GT_FALSE,             GT_NA}
        ,{  59, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       59,   59,            GT_FALSE,             GT_NA}

    ,{           63,     CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,               0,      GT_NA,        63,            GT_FALSE,          GT_NA}

        ,{  64, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       64,   GT_NA,            GT_TRUE,             64}
        ,{  65, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       65,   GT_NA,            GT_TRUE,             65}
        ,{  66, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       66,   GT_NA,            GT_TRUE,             66}
        ,{  67, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       67,   GT_NA,            GT_TRUE,             67}
        ,{  68, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       68,   GT_NA,            GT_TRUE,             68}
        ,{  69, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       69,   GT_NA,            GT_TRUE,             69}
        ,{  70, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       70,   GT_NA,            GT_TRUE,             70}
        ,{  71, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       71,   GT_NA,            GT_TRUE,             71}


    };

    static PortInitList_STC portInitlist_4x10G_4x1G[] = 
    {
         { PORT_LIST_TYPE_INTERVAL,     {0,  46, 1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_1000_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E     }
        ,{ PORT_LIST_TYPE_INTERVAL,     {56, 59, 1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_SR_LR_E      }
        ,{ PORT_LIST_TYPE_INTERVAL,     {64, 71, 1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_SR_LR_E      }
        ,{ PORT_LIST_TYPE_EMPTY,        {           APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E         }
    };


    rc = appDemoDbEntryAdd("initSerdesDefaults", 0);
    if (rc != GT_OK)
    {
        return rc;
    }

    #ifdef ASIC_SIMULATION
        appDemoBobKSimCoreClockSet(dev,365); 
    #endif

    rc = cpssInitSystem(29,2,0);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = cpssDxChPortPhysicalPortMapSet(dev,
                                        sizeof(cetusMap)/sizeof(cetusMap[0]),
                                        &cetusMap[0]);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortPhysicalPortMapSet", rc);
    if (GT_OK != rc)
    {
        return rc;
    }

    rc = appDemoBc2PortListInit(dev,&portInitlist_4x10G_4x1G[0],GT_FALSE);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}


GT_STATUS cetus_88E1690
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U8 dev = 0;
    /*
        GT_U32 groupId;
        PRV_CPSS_DXCH_SPECIAL_TXQ_CLIENT_GROUP_DB_STC *paSpecialTxQClientGroupDB;
    */


    static CPSS_DXCH_PORT_MAP_STC cetusMap[] =
    { /* Port,            mappingType                           portGroupm, intefaceNum, txQPort, trafficManegerEnable , tmPortInd*/

         {  56, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       56,     GT_NA,         GT_FALSE,        GT_NA}
        ,{  58, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       58,     GT_NA,         GT_FALSE,        GT_NA}
        ,{  64, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       64,     GT_NA,         GT_FALSE,        GT_NA}
        ,{  65, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       65,     65,            GT_FALSE,        GT_NA}
        ,{  66, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       66,     66,            GT_FALSE,        GT_NA}
        ,{  67, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       67,     67,            GT_FALSE,        GT_NA}
        ,{  68, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       68,     68,            GT_FALSE,        GT_NA}
        ,{  69, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       69,     69,            GT_FALSE,        GT_NA}
        ,{  70, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       70,     70,            GT_FALSE,        GT_NA}
        ,{  71, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       71,     71,            GT_FALSE,        GT_NA}
        ,{ 100, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       0,       56,      0,            GT_FALSE,        GT_NA}                       
        ,{ 101, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       0,       56,      1,            GT_FALSE,        GT_NA}                       
        ,{ 102, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       0,       56,      2,            GT_FALSE,        GT_NA}                       
        ,{ 103, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       0,       56,      3,            GT_FALSE,        GT_NA}                       
        ,{ 104, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       0,       58,      4,            GT_FALSE,        GT_NA}                       
        ,{ 105, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       0,       58,      5,            GT_FALSE,        GT_NA}                       
        ,{ 106, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       0,       58,      6,            GT_FALSE,        GT_NA}                       
        ,{ 107, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       0,       64,      7,            GT_FALSE,        GT_NA}                       
    };

#if 0
    static PortInitList_STC portInitlist_1x10GxRXAUI_10x10GxSR_LR[] = 
    {
         { PORT_LIST_TYPE_INTERVAL,     {58,   59, 1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_SR_LR_E      }
        ,{ PORT_LIST_TYPE_INTERVAL,     {64,   71, 1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_SR_LR_E      }
        ,{ PORT_LIST_TYPE_EMPTY,        {             APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E         }
    };
#endif



    static PortInitList_STC portInitlist_8xRemotePorts[] = 
    {
         { PORT_LIST_TYPE_INTERVAL,     {100, 105, 1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_1000_E,   CPSS_PORT_INTERFACE_MODE_SGMII_E      } /* remotes QSGMII ??? */
        ,{ PORT_LIST_TYPE_LIST,         {106, 107,    APP_INV_PORT_CNS }, CPSS_PORT_SPEED_2500_E,   CPSS_PORT_INTERFACE_MODE_SGMII_E      } /* remotes QSGMII ??? */
        ,{ PORT_LIST_TYPE_EMPTY,        {             APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E         }
    };

    static PortInitList_STC portInitlist_CascadePort_56[] = 
    {
         { PORT_LIST_TYPE_LIST,         {56,          APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_RXAUI_E      }
        ,{ PORT_LIST_TYPE_EMPTY,        {             APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E         }
    };

    static PortInitList_STC portInitlist_CascadePort_58_64[] = 
    {
         { PORT_LIST_TYPE_LIST,         {58,          APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_RXAUI_E      }
        ,{ PORT_LIST_TYPE_LIST,         {64,          APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_RXAUI_E      }
        ,{ PORT_LIST_TYPE_EMPTY,        {             APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E         }
    };


    rc = appDemoDbEntryAdd("noCpu", 1);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = appDemoDbEntryAdd("initSerdesDefaults", 0);
    if (rc != GT_OK)
    {
        return rc;
    }

    #ifdef ASIC_SIMULATION
        appDemoBobKSimCoreClockSet(dev,365); 
    #endif

    rc = cpssInitSystem(29,1,0);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = cpssDxChPortPhysicalPortMapSet(dev,
                                        sizeof(cetusMap)/sizeof(cetusMap[0]),
                                        &cetusMap[0]);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortPhysicalPortMapSet", rc);
    if (GT_OK != rc)
    {
        return rc;
    }

    rc = gtBobcat2PortMappingDump(dev);
    if (rc != GT_OK)
    {
        return rc;
    }

#if 0
    rc = appDemoBc2PortListInit(dev,&portInitlist_1x10GxRXAUI_10x10GxSR_LR[0], /*skipCheckEnable = */GT_FALSE);
    if (rc != GT_OK)
    {
        return rc;
    }
#endif


    rc = prvCpssDxChPortPizzaArbiterIfCCFCClientConfigure(dev,/*mac*/56,/*txq*/20,CPSS_PORT_SPEED_1000_E);
    if (rc != GT_OK)
    {
        return rc;
    }
    cpssOsPrintf("\nTXQ After CCFC mac 56 txq 20: shall be nothing : no extended-cascade");
    rc = gtBobcat2PortPizzaArbiterIfUnitStateDump(dev,0, CPSS_DXCH_PA_UNIT_TXQ_0_E);
    if (rc != GT_OK)
    {
        return rc;
    }


    rc = appDemoBc2PortListInit(dev,&portInitlist_CascadePort_58_64[0], /*skipCheckEnable = */GT_FALSE);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = gtBobcat2PortPizzaArbiterIfUnitStateDump(dev,0,CPSS_DXCH_PA_UNIT_TXQ_0_E);
    if (rc != GT_OK)
    {
        return rc;
    }


    rc = appDemoBc2PortListInit(dev,&portInitlist_8xRemotePorts[0], /*skipCheckEnable = */GT_FALSE);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = gtBobcat2PortPizzaArbiterIfUnitStateDump(dev,0,CPSS_DXCH_PA_UNIT_TXQ_0_E);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = appDemoBc2PortListInit(dev,&portInitlist_CascadePort_56[0], /*skipCheckEnable = */GT_FALSE);
    if (rc != GT_OK)
    {
        return rc;
    }


    rc = appDemoBobk2PortListResourcesPrint(dev);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = gtBobcat2PortPizzaArbiterIfStateDump(dev,0);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChPortPizzaArbiterIfCCFCClientConfigure(dev,/*mac*/58,/*txq*/21,CPSS_PORT_SPEED_1000_E);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = gtBobcat2PortPizzaArbiterIfUnitStateDump(dev,0,CPSS_DXCH_PA_UNIT_TXQ_0_E);
    if (rc != GT_OK)
    {
        return rc;
    }



#ifdef __CHANGE_TXQ_MANUALLY
    cpssOsPrintf("\n+----------------------------------+");
    cpssOsPrintf("\n| Configure TxQ Pizza              |");
    cpssOsPrintf("\n+----------------------------------+");
    cpssOsPrintf("\n");

    rc = gtBobcat2PortPizzaArbiterIfUnitStateDump(dev,0,CPSS_DXCH_PA_UNIT_TXQ_0_E);
    if (rc != GT_OK)
    {
        return rc;
    }

    paSpecialTxQClientGroupDB = &(PRV_CPSS_PP_MAC(dev)->paPortDB.paSpecialTxQClientGroupDB);
    rc = prvCpssDxChPortDynamicPATxQClientGroupListInit(paSpecialTxQClientGroupDB);
    if (rc != GT_OK)
    {
        return rc;
    }


    for (groupId = 10; groupId <= 20; groupId++)
    {
        rc = prvCpssDxChPortDynamicPATxQClientGroupListAdd(paSpecialTxQClientGroupDB,groupId);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssDxChPortDynamicPATxQClientGroupListAdd(paSpecialTxQClientGroupDB,groupId);
        if (rc == GT_OK)
        {
            return rc;
        }

    }

    rc = prvCpssDxChPortDynamicPATxQClientGroupListDelete(paSpecialTxQClientGroupDB,20);
    if (rc != GT_OK)
    {
        return rc;
    }

    for (groupId = 10; groupId <= 19; groupId++)
    {
        rc = prvCpssDxChPortDynamicPATxQClientGroupListDelete(paSpecialTxQClientGroupDB,groupId);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssDxChPortDynamicPATxQClientGroupListDelete(paSpecialTxQClientGroupDB,groupId);
        if (rc == GT_OK)
        {
            return rc;
        }

    }

#endif


    return GT_OK;
}


GT_U32 bobkStart(GT_U32 cc, GT_U32 sysType)
{
    GT_STATUS rc;

    GT_U8 dev = 0;


    #ifdef ASIC_SIMULATION
        #ifndef GM_USED
            appDemoBobKSimCoreClockSet(dev,cc); 
        #else
            cc = cc;
        #endif
    #endif

    rc = cpssInitSystem(29,sysType,0);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = appDemoPizzaArbiterUnitsDefPrint(dev);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = gtBobcat2PortMappingDump(dev);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = appDemoBobk2PortGobalResourcesPrint(dev);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = appDemoBobk2PortListResourcesPrint(dev);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = gtBobcat2PortPizzaArbiterIfStateDump(dev,0);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}



GT_U32 bobkStartNoPorts(GT_U32 cc)
{
    GT_STATUS rc;

    GT_U8 dev = 0;


    #ifdef ASIC_SIMULATION
        #ifndef GM_USED
            appDemoBobKSimCoreClockSet(dev,cc); 
        #else
            cc = cc;
        #endif
    #endif

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

    rc = appDemoBobk2PortListResourcesPrint(dev);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}




