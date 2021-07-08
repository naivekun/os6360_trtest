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


extern GT_STATUS appDemoAldrinPortGobalResourcesPrint
(
    IN GT_U8 dev
);




extern void appDemoBc2SetCustomPortsMapping
(
    IN GT_BOOL enable
);



extern GT_STATUS appDemoAldrinPortMappingDump
(
    IN  GT_U8  dev
);


extern GT_STATUS appDemoAldrinPortListResourcesPrint
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




GT_U32 aldrinStart(GT_U32 cc)
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

    rc = appDemoAldrinPortMappingDump(dev);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = appDemoAldrinPortGobalResourcesPrint(dev);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = appDemoAldrinPortListResourcesPrint(dev);
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




#define BAD_VALUE (GT_U32)(~0)

#define MAC_PORT_TO_88E1690_0_CNS   12  /* SERDES 12,13 */
#define MAC_PORT_TO_88E1690_1_CNS   14  /* SERDES 14,15 */
#define MAC_PORT_TO_88E1690_2_CNS   16  /* SERDES 16,17 */
#define MAC_PORT_TO_88E1690_3_CNS   18  /* SERDES 18,19 */
#define MAC_PORT_TO_88E1690_4_CNS   20  /* SERDES 20,21 */
#define MAC_PORT_TO_88E1690_5_CNS   22  /* SERDES 22,23 */

#define NA_PG                       0
#define CASCADE_PORT_NOT_NEED_TXQ_PORT_CNS          GT_NA

static CPSS_DXCH_PORT_MAP_STC xcat3x_Map_24_remote_ports[] =
{ /* Port,            mappingType                                portGroup,                     intefaceNum,                       txQPort,      */
    /* DP 0 */
     {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,                               0,                             0  }
    ,{   1, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,                               1,                             1  }
    ,{   2, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,                               2,                             2  }
    ,{   3, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,                               3,                             3  }
    ,{   4, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,                               4,                             4  }
    ,{   5, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,                               5,                             5  }
    ,{   6, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,                               6,                             6  }
    ,{   7, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,                               7,                             7  }
    ,{   8, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,                               8,                             8  }
    ,{   9, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,                               9,                             9  }
    ,{  10, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,                              10,                            10  }
    ,{  11, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,                              11,                            11  }
    /* DP 1 */
    ,{  12, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       MAC_PORT_TO_88E1690_0_CNS,     CASCADE_PORT_NOT_NEED_TXQ_PORT_CNS}
    ,{  14, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       MAC_PORT_TO_88E1690_1_CNS,     CASCADE_PORT_NOT_NEED_TXQ_PORT_CNS}
    ,{  16, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       MAC_PORT_TO_88E1690_2_CNS,     CASCADE_PORT_NOT_NEED_TXQ_PORT_CNS}
    ,{  18, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,                              18,                            18  }
    ,{  19, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,                              19,                            19  }
    ,{  20, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,                              20,                            20  }
    ,{  21, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,                              21,                            21  }
    ,{  22, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,                              22,                            22  }
    ,{  23, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,                              23,                            23  }

    /* DP 2 */
    ,{  24, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,                              24,                            24  }
    ,{  25, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,                              25,                            25  }
    ,{  26, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,                              26,                            26  }
    ,{  27, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,                              27,                            27  }
    ,{  28, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,                              28,                            28  }
    ,{  29, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,                              29,                            29  }
    ,{  30, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,                              30,                            30  }
    ,{  31, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,                              31,                            31  }

    /* connection to 88E1690 #0 */
    ,{  80, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_0_CNS,                            32  }
    ,{  81, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_0_CNS,                            33  }
    ,{  82, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_0_CNS,                            34  }
    ,{  83, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_0_CNS,                            35  }
    ,{  84, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_0_CNS,                            36  }
    ,{  85, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_0_CNS,                            37  }
    ,{  86, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_0_CNS,                            38  }
    ,{  87, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_0_CNS,                            39  }
                                                                                                                                            
    /* connection to 88E1690 #1 */                                                                                                       
    ,{  88, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_1_CNS,                            40  }
    ,{  89, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_1_CNS,                            41  }
    ,{  90, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_1_CNS,                            42  }
    ,{  91, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_1_CNS,                            43  }
    ,{  92, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_1_CNS,                            44  }
    ,{  93, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_1_CNS,                            45  }
    ,{  94, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_1_CNS,                            46  }
    ,{  95, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_1_CNS,                            47  }
                                                                                                                                                
    /* connection to 88E1690 #2 */                                                                                                           
    ,{  96, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_2_CNS,                            48  }
    ,{  97, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_2_CNS,                            49  }
    ,{  98, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_2_CNS,                            50  }
    ,{  99, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_2_CNS,                            51  }
    ,{ 100, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_2_CNS,                            52  }
    ,{ 101, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_2_CNS,                            53  }
    ,{ 102, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_2_CNS,                            54  }
    ,{ 103, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_2_CNS,                            55  }
                                                                                                                                                
    ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,                   NA_PG,                           GT_NA,                            63  }
    ,{ BAD_VALUE,        CPSS_DXCH_PORT_MAPPING_TYPE_INVALID_E,    BAD_VALUE,                      BAD_VALUE,                     BAD_VALUE  }
};


static PortInitList_STC ac3x_portInitlist_26x10G_Cascade_3x10G_Remotes_24x1_CPU[] =
{
    /* core 0 */
/* DP 0 */  { PORT_LIST_TYPE_INTERVAL,  {  0,   11,  1,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_SR_LR_E   } 
/* DP 1 */ ,{ PORT_LIST_TYPE_LIST,      { 12,   14, 16,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_RXAUI_E   } 
/* DP 1 */ ,{ PORT_LIST_TYPE_INTERVAL,  { 18,   23,  1,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_SR_LR_E   } 
/* DP 2 */ ,{ PORT_LIST_TYPE_INTERVAL,  { 24,   31,  1,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_SR_LR_E   } 
/*         ,{ PORT_LIST_TYPE_INTERVAL,  { 80,  103,  1,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_1000_E,   CPSS_PORT_INTERFACE_MODE_SGMII_E   } */
           ,{ PORT_LIST_TYPE_EMPTY,     {                   APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E      }
};


static CPSS_DXCH_PORT_MAP_STC xcat3x_Map_48_remote_ports[] =
{ /* Port,            mappingType                                portGroup,                     intefaceNum,                       txQPort,      */
    /* DP 0 */
     {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,                               0,                             0  }
    ,{   1, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,                               1,                             1  }
    ,{   2, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,                               2,                             2  }
    ,{   3, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,                               3,                             3  }
    ,{   4, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,                               4,                             4  }
    ,{   5, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,                               5,                             5  }
    ,{   6, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,                               6,                             6  }
    ,{   7, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,                               7,                             7  }
    ,{   8, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,                               8,                             8  }
    ,{   9, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,                               9,                             9  }
    ,{  10, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,                              10,                            10  }
    ,{  11, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,                              11,                            11  }
    /* DP 1 */
    ,{  12, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       MAC_PORT_TO_88E1690_0_CNS,     CASCADE_PORT_NOT_NEED_TXQ_PORT_CNS}
    ,{  14, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       MAC_PORT_TO_88E1690_1_CNS,     CASCADE_PORT_NOT_NEED_TXQ_PORT_CNS}
    ,{  16, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       MAC_PORT_TO_88E1690_2_CNS,     CASCADE_PORT_NOT_NEED_TXQ_PORT_CNS}
    ,{  18, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       MAC_PORT_TO_88E1690_3_CNS,     CASCADE_PORT_NOT_NEED_TXQ_PORT_CNS}
    ,{  20, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       MAC_PORT_TO_88E1690_4_CNS,     CASCADE_PORT_NOT_NEED_TXQ_PORT_CNS}
    ,{  22, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,       MAC_PORT_TO_88E1690_5_CNS,     CASCADE_PORT_NOT_NEED_TXQ_PORT_CNS}
    /* DP 2 */
    ,{  24, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,                              24,                            24  }
    ,{  25, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,                              25,                            25  }
    ,{  26, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,                              26,                            26  }
    ,{  27, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,                              27,                            27  }
    ,{  28, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,                              28,                            28  }
    ,{  29, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,                              29,                            29  }
    ,{  30, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,                              30,                            30  }
    ,{  31, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,                              31,                            31  }
    ,{  32, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               NA_PG,                              32,                            32  }

    /* connection to 88E1690 #0 */
    ,{  80, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_0_CNS,                            33  }
    ,{  81, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_0_CNS,                            34  }
    ,{  82, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_0_CNS,                            35  }
    ,{  83, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_0_CNS,                            36  }
    ,{  84, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_0_CNS,                            37  }
    ,{  85, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_0_CNS,                            38  }
    ,{  86, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_0_CNS,                            39  }
    ,{  87, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_0_CNS,                            40  }
                                                                                                                                            
    /* connection to 88E1690 #1 */                                                                                                       
    ,{  88, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_1_CNS,                            41  }
    ,{  89, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_1_CNS,                            42  }
    ,{  90, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_1_CNS,                            43  }
    ,{  91, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_1_CNS,                            44  }
    ,{  92, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_1_CNS,                            45  }
    ,{  93, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_1_CNS,                            46  }
    ,{  94, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_1_CNS,                            47  }
    ,{  95, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_1_CNS,                            48  }
                                                                                                                                                
    /* connection to 88E1690 #2 */                                                                                                           
    ,{  96, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_2_CNS,                            49  }
    ,{  97, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_2_CNS,                            50  }
    ,{  98, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_2_CNS,                            51  }
    ,{  99, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_2_CNS,                            52  }
    ,{ 100, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_2_CNS,                            53  }
    ,{ 101, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_2_CNS,                            54  }
    ,{ 102, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_2_CNS,                            55  }
    ,{ 103, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_2_CNS,                            56  }

    /* connection to 88E1690 #3 */
    ,{ 104, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_3_CNS,                            57  }
    ,{ 105, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_3_CNS,                            58  }
    ,{ 106, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_3_CNS,                            59  }
    ,{ 107, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_3_CNS,                            60  }
    ,{ 108, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_3_CNS,                            61  }
    ,{ 109, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_3_CNS,                            62  }
    ,{ 110, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_3_CNS,                            64  }
    ,{ 111, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_3_CNS,                            65  }

    /* connection to 88E1690 #4 */                                                                                                       
    ,{ 112, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_4_CNS,                            66  }
    ,{ 113, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_4_CNS,                            67  }
    ,{ 114, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_4_CNS,                            68  }
    ,{ 115, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_4_CNS,                            69  }
    ,{ 116, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_4_CNS,                            70  }
    ,{ 117, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_4_CNS,                            71  }
    ,{ 118, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_4_CNS,                            12  }
    ,{ 119, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_4_CNS,                            13  }

    /* connection to 88E1690 #5 */                                                                                                           
    ,{ 120, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_5_CNS,                            14  }
    ,{ 121, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_5_CNS,                            15  }
    ,{ 122, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_5_CNS,                            16  }
    ,{ 123, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_5_CNS,                            17  }
    ,{ 124, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_5_CNS,                            18  }
    ,{ 125, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_5_CNS,                            19  }
    ,{ 126, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_5_CNS,                            20  }
    ,{ 127, CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,       NA_PG,       MAC_PORT_TO_88E1690_5_CNS,                            21  }

    ,{  63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,                   NA_PG,                           GT_NA,                            63  }
    ,{ BAD_VALUE,      CPSS_DXCH_PORT_MAPPING_TYPE_INVALID_E,       BAD_VALUE,                     BAD_VALUE,                     BAD_VALUE  }
};

static PortInitList_STC ac3x_portInitlist_20x10G_Cascade_6x10G_Remotes_48x1G_CPU[] =
{
    /* core 0 */
/* DP 0 */  { PORT_LIST_TYPE_INTERVAL,  {  0,   11,  1,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_SR_LR_E   } 
/* DP 1 */ ,{ PORT_LIST_TYPE_LIST,      { 12,   14, 16,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_RXAUI_E   } 
/* DP 1 */ ,{ PORT_LIST_TYPE_LIST,      { 18,   20, 22,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_RXAUI_E   } 
/* DP 2 */ ,{ PORT_LIST_TYPE_INTERVAL,  { 24,   31,  1,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_SR_LR_E   } 
           ,{ PORT_LIST_TYPE_EMPTY,     {                   APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E      }
};

static PortInitList_STC ac3x_portInitlist_18x12_5G_2x10G_Cascade_6x10G_Remotes_48x1G_CPU[] =
{
/* DP 0 */  { PORT_LIST_TYPE_INTERVAL,  {  0,    8,  1,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_12500_E,  CPSS_PORT_INTERFACE_MODE_KR_E      } 
/* DP 0 */ ,{ PORT_LIST_TYPE_LIST,      {  9,   10,         APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_SR_LR_E   } 

/* DP 1 */ ,{ PORT_LIST_TYPE_LIST,      { 12,   14, 16,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_RXAUI_E   } 
/* DP 1 */ ,{ PORT_LIST_TYPE_LIST,      { 18,   20, 22,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_RXAUI_E   } 

/* DP 2 */ ,{ PORT_LIST_TYPE_INTERVAL,  { 24,   30,  1,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_12500_E,  CPSS_PORT_INTERFACE_MODE_KR_E      } 
/* DP 2 */ ,{ PORT_LIST_TYPE_LIST,      { 32,               APP_INV_PORT_CNS }, CPSS_PORT_SPEED_2500_E,   CPSS_PORT_INTERFACE_MODE_SGMII_E   } 
           ,{ PORT_LIST_TYPE_EMPTY,     {                   APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E      }
};

/*
 * +-----------------------------------+-----------+----------+----------+
 * |   Conf                            | BW per DP | Port N   | Serdes N |
 * +-----------------------------------+-----------+----------+----------+
 * |                                   | Giv   Bnd | Giv Bnd  |  Giv Bnd |
 * +-----------------------------------+-----------+----------+----------+
 * | DP-0 : 2x10+9*12.5                | 137   147 | 11   12  |   11  12 |
 * | DP-1 : 6x10                       | 60    147 |  6   12  |   12  12 |
 * | DP-2 : 1x2.5+7x12.5               | 94    147 |  8    8  |    8   8 |
 * +-----------------------------------+-----------+----------+----------+
 * 
 */

typedef struct 
{
    GT_U32                      confIdx;
    CPSS_DXCH_PORT_MAP_STC     *portMapArrayPtr;
    PortInitList_STC           *portInitListPtr;
    GT_PHYSICAL_PORT_NUM        firstRemotePort;
    GT_PHYSICAL_PORT_NUM        lastRemotePort;
}ac3xOnAldrin_CONF_STC;

static ac3xOnAldrin_CONF_STC ac3xOnAldrin_ConfArr[] = 
{
     {         1, &xcat3x_Map_24_remote_ports[0], &ac3x_portInitlist_26x10G_Cascade_3x10G_Remotes_24x1_CPU[0],                80,       103 }
    ,{         2, &xcat3x_Map_48_remote_ports[0], &ac3x_portInitlist_20x10G_Cascade_6x10G_Remotes_48x1G_CPU[0],               80,       103 }
    ,{         3, &xcat3x_Map_48_remote_ports[0], &ac3x_portInitlist_18x12_5G_2x10G_Cascade_6x10G_Remotes_48x1G_CPU[0],       80,       103 }
    ,{ BAD_VALUE, (CPSS_DXCH_PORT_MAP_STC*)NULL,  (PortInitList_STC *)NULL,                                            BAD_VALUE, BAD_VALUE }
};

GT_STATUS InitCPLL4ports_12p5G_KR(GT_U8 dev, PortInitList_STC *portInitListPtr)
{
    GT_STATUS rc;
    GT_U32 i,j;
    GT_BOOL found = GT_FALSE;
    GT_PHYSICAL_PORT_NUM portNum;
    PortInitList_STC * entryPtr;
    /*-----------------------------------*
     * find whether we really need it    *
     *-----------------------------------*/
    for (i = 0 ; portInitListPtr[i].entryType != PORT_LIST_TYPE_EMPTY; i++)
    {
        if (portInitListPtr[i].speed == CPSS_PORT_SPEED_12500_E && portInitListPtr[i].interfaceMode == CPSS_PORT_INTERFACE_MODE_KR_E)
        {
            found = GT_TRUE;
            break;
        }
    }
    if (found == GT_TRUE)
    {
        cpssOsPrintf("\nCPLL Init() ...");
        rc = cpssDxChPortCpllCfgInit(dev,/* cpllNum */0, CPSS_DXCH_PORT_CPLL_INPUT_FREQUENCY_25M_E,CPSS_DXCH_PORT_CPLL_OUTPUT_FREQUENCY_78M_E);
        if (rc != GT_OK)
        {
            return rc;
        }
        cpssOsPrintf("Done");

        cpssOsPrintf("\nRefClockSource overrifde : ");
        for (i = 0 ; portInitListPtr[i].entryType != PORT_LIST_TYPE_EMPTY; i++)
        {
            entryPtr = &portInitListPtr[i];
            if (entryPtr->speed == CPSS_PORT_SPEED_12500_E && entryPtr->interfaceMode == CPSS_PORT_INTERFACE_MODE_KR_E)
            {
                switch (entryPtr->entryType)
                {
                    case PORT_LIST_TYPE_INTERVAL:
                        cpssOsPrintf("\n    ");
                        for (portNum = entryPtr->portList[0] ; portNum <= entryPtr->portList[1]; portNum += entryPtr->portList[2])
                        {
                            cpssOsPrintf(" %3d",portNum);
                            rc = cpssDxChPortRefClockSourceOverrideEnableSet(dev,portNum,GT_TRUE,CPSS_PORT_REF_CLOCK_SOURCE_SECONDARY_E);
                            if (rc != GT_OK)
                            {
                                return rc;
                            }
                        }
                    break;
                    case PORT_LIST_TYPE_LIST:
                        cpssOsPrintf("\n    ");
                        for (j = 0 ; entryPtr->portList[j] != APP_INV_PORT_CNS; j++)
                        {
                            portNum = entryPtr->portList[j];
                            cpssOsPrintf(" %3d",portNum);
                            rc = cpssDxChPortRefClockSourceOverrideEnableSet(dev,portNum,GT_TRUE,CPSS_PORT_REF_CLOCK_SOURCE_SECONDARY_E);
                            if (rc != GT_OK)
                            {
                                return rc;
                            }
                        }
                    break;
                    default:
                    {
                        cpssOsPrintf("\n-->ERROR : entry %d : undefined entry type",i);
                        return GT_BAD_PARAM;
                    }
                }
            }
        }
        cpssOsPrintf("\nDone");
    }
    return GT_OK;
}


GT_STATUS aldrin_23400(GT_VOID)
{
    GT_STATUS rc;

    GT_U8 dev = 0;

    static PortInitList_STC portInitlist[] =
    {
        /* core 0 */
    /* DP 0 */  { PORT_LIST_TYPE_LIST,      {  0,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_23600_E,  CPSS_PORT_INTERFACE_MODE_XHGS_E    } 
    /* DP 1 */ ,{ PORT_LIST_TYPE_LIST,      { 12,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_23600_E,  CPSS_PORT_INTERFACE_MODE_XHGS_E    } 
    /* DP 2 */ ,{ PORT_LIST_TYPE_LIST,      { 24,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_23600_E,  CPSS_PORT_INTERFACE_MODE_XHGS_E    } 
               ,{ PORT_LIST_TYPE_EMPTY,     {      APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E      }
    };


    #ifdef ASIC_SIMULATION
        #ifndef GM_USED
            appDemoBobKSimCoreClockSet(dev,480); 
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

    rc = appDemoBc2PortListInit(dev,&portInitlist[0],GT_FALSE);
    if (rc != GT_OK)
    {
        return rc;
    }

    cpssOsPrintf("Done");

    rc = appDemoAldrinPortGobalResourcesPrint(dev);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = appDemoAldrinPortListResourcesPrint(dev);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}



GT_STATUS aldrin_480Mhz_DP_PA_156Gbps(GT_VOID)
{
    GT_STATUS rc;

    GT_U8 dev = 0;

    static PortInitList_STC portInitlist[] =
    {
        /* core 0 */
    /* DP 0 */  { PORT_LIST_TYPE_INTERVAL,  {   0, 11, 1,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_12500_E,  CPSS_PORT_INTERFACE_MODE_KR_E      } 
    /* DP 1 */ ,{ PORT_LIST_TYPE_INTERVAL,  {  12, 15, 1,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_12500_E,  CPSS_PORT_INTERFACE_MODE_KR_E      } 
    /* DP 1 */ ,{ PORT_LIST_TYPE_INTERVAL,  {  16, 19, 1,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      } 
    /* DP 2 */ ,{ PORT_LIST_TYPE_INTERVAL,  {  24, 31, 1,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_12500_E,  CPSS_PORT_INTERFACE_MODE_KR_E      } 
               ,{ PORT_LIST_TYPE_EMPTY,     {      APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E      }
    };


    #ifdef ASIC_SIMULATION
        #ifndef GM_USED
            appDemoBobKSimCoreClockSet(dev,480); 
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

    rc = appDemoBc2PortListInit(dev,&portInitlist[0],GT_FALSE);
    if (rc != GT_OK)
    {
        return rc;
    }

    cpssOsPrintf("Done");
    cpssOsTimerWkAfter(1000);

    rc = appDemoAldrinPortGobalResourcesPrint(dev);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = appDemoAldrinPortListResourcesPrint(dev);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}





GT_STATUS ac3xOnAldrin(GT_U32 confId)
{
    GT_STATUS rc;

    GT_U8 dev = 0;
    GT_U32                      i;
    GT_U32                      portMapArraySize;
    CPSS_DXCH_PORT_MAP_STC     *portMapArrayPtr;
    PortInitList_STC           *portInitListPtr;
    GT_PHYSICAL_PORT_NUM        firstRemotePort;
    GT_PHYSICAL_PORT_NUM        lastRemotePort;
    GT_PHYSICAL_PORT_NUM        portNum;
    ac3xOnAldrin_CONF_STC      *configPtr;


    #ifdef ASIC_SIMULATION
        #ifndef GM_USED
            appDemoBobKSimCoreClockSet(dev,480); 
        #else
            cc = cc;
        #endif
    #endif

    configPtr = (ac3xOnAldrin_CONF_STC *)NULL;
    for (i = 0 ; ac3xOnAldrin_ConfArr[i].confIdx != BAD_VALUE; i++)
    {
        if (ac3xOnAldrin_ConfArr[i].confIdx == confId)
        {
            configPtr = &ac3xOnAldrin_ConfArr[i];
            break;
        }
    }
    if (configPtr == NULL)
    {
        cpssOsPrintf("\n-->ERROR : %d : unknown config",confId);
        return GT_BAD_PARAM;
    }
    
    portMapArrayPtr  = configPtr->portMapArrayPtr;
    portInitListPtr  = configPtr->portInitListPtr;
    firstRemotePort  = configPtr->firstRemotePort;
    lastRemotePort   = configPtr->lastRemotePort;

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

    cpssOsPrintf("\nConfig mapping....");
    if (portMapArrayPtr != NULL)
    {
        /* count number of lines on mapping */
        for (i = 0 ; portMapArrayPtr[i].mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_INVALID_E; i++);
        portMapArraySize = i;

        rc = cpssDxChPortPhysicalPortMapSet(dev,  portMapArraySize, portMapArrayPtr);
        if (GT_OK != rc)
        {
            return rc;
        }
        cpssOsPrintf("Done");
    }
    else
    {
        cpssOsPrintf("No Mapping");
    }

    rc = InitCPLL4ports_12p5G_KR(dev,portInitListPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* init pizza arbiter once more */
    rc = prvCpssDxChPortPizzaArbiterIfInit(dev);
    if (rc != GT_OK)
    {
        return rc;
    }

    cpssOsPrintf("\nConfig ports....");
    rc = appDemoBc2PortListInit(dev, portInitListPtr,GT_FALSE);
    if (rc != GT_OK)
    {
        return rc;
    }
    cpssOsPrintf("Done");

    cpssOsPrintf("\nConfig slices for remote ports : %3d : %3d....",firstRemotePort,lastRemotePort);
    for (portNum =  firstRemotePort; portNum <= lastRemotePort; portNum++)
    {
        rc = prvCpssDxChPortPizzaArbiterIfConfigure(dev, portNum,  CPSS_PORT_SPEED_1000_E);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChPortCaelumModeSpeedSet: error in prvCpssDxChPortPizzaArbiterIfConfigure, portNum = %d\n", portNum);
        }
    }
    cpssOsPrintf("Done");

    rc = appDemoAldrinPortMappingDump(dev);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = appDemoAldrinPortGobalResourcesPrint(dev);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = appDemoAldrinPortListResourcesPrint(dev);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}


