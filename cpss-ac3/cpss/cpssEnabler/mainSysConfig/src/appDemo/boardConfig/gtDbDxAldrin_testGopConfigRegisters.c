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


static CPSS_DXCH_PORT_MAP_STC aldrinDefaultMap[] =
{ /* Port,            mappingType                              portGroup, intefaceNum, txQPort,       TM Enable ,     tmPortInd*/
    {         0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        0,       0,           GT_FALSE,          GT_NA}
   ,{         1, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        1,       1,           GT_FALSE,          GT_NA}
   ,{         2, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        2,       2,           GT_FALSE,          GT_NA}
   ,{         3, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        3,       3,           GT_FALSE,          GT_NA}
   ,{         4, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        4,       4,           GT_FALSE,          GT_NA}
   ,{         5, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        5,       5,           GT_FALSE,          GT_NA}
   ,{         6, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        6,       6,           GT_FALSE,          GT_NA}
   ,{         7, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        7,       7,           GT_FALSE,          GT_NA}
   ,{         8, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        8,       8,           GT_FALSE,          GT_NA}
   ,{         9, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,        9,       9,           GT_FALSE,          GT_NA}
   ,{        10, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       10,      10,           GT_FALSE,          GT_NA}
   ,{        11, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       11,      11,           GT_FALSE,          GT_NA}
   ,{        12, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       12,      12,           GT_FALSE,          GT_NA}
   ,{        13, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       13,      13,           GT_FALSE,          GT_NA}
   ,{        14, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       14,      14,           GT_FALSE,          GT_NA}
   ,{        15, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       15,      15,           GT_FALSE,          GT_NA}
   ,{        16, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       16,      16,           GT_FALSE,          GT_NA}
   ,{        17, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       17,      17,           GT_FALSE,          GT_NA}
   ,{        18, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       18,      18,           GT_FALSE,          GT_NA}
   ,{        19, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       19,      19,           GT_FALSE,          GT_NA}
   ,{        20, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       20,      20,           GT_FALSE,          GT_NA}
   ,{        21, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       21,      21,           GT_FALSE,          GT_NA}
   ,{        22, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       22,      22,           GT_FALSE,          GT_NA}
   ,{        23, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       23,      23,           GT_FALSE,          GT_NA}
   ,{        24, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       24,      24,           GT_FALSE,          GT_NA}
   ,{        25, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       25,      25,           GT_FALSE,          GT_NA}
   ,{        26, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       26,      26,           GT_FALSE,          GT_NA}
   ,{        27, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       27,      27,           GT_FALSE,          GT_NA}
   ,{        28, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       28,      28,           GT_FALSE,          GT_NA}
   ,{        29, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       29,      29,           GT_FALSE,          GT_NA}
   ,{        30, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       30,      30,           GT_FALSE,          GT_NA}
   ,{        31, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       31,      31,           GT_FALSE,          GT_NA}
   ,{        63, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,                   0,    GT_NA,      63,           GT_FALSE,          GT_NA}
   ,{ BAD_VALUE,      CPSS_DXCH_PORT_MAPPING_TYPE_INVALID_E,       BAD_VALUE,  BAD_VALUE, BAD_VALUE,       GT_FALSE,      BAD_VALUE}
};


/*   Phys     globalDma,       dp-index,  localDma */
/*    0,   {      0,       {         0,        56 }    }     */
/*    1,  ,{      1,       {         0,        57 }    }     */
/*    2,  ,{      2,       {         0,        58 }    }     */
/*    3,  ,{      3,       {         0,        59 }    }     */
/*    4,  ,{      4,       {         0,        64 }    }     */
/*    5,  ,{      5,       {         0,        65 }    }     */
/*    6,  ,{      6,       {         0,        66 }    }     */
/*    7,  ,{      7,       {         0,        67 }    }     */
/*    8,  ,{      8,       {         0,        68 }    }     */
/*    9,  ,{      9,       {         0,        69 }    }     */
/*   10,  ,{     10,       {         0,        70 }    }     */
/*   11,  ,{     11,       {         0,        71 }    }     */
/*   ------------------------------------------------------- */
/*   12,  ,{     12,       {         1,        56 }    }     */
/*   13,  ,{     13,       {         1,        57 }    }     */
/*   14,  ,{     14,       {         1,        58 }    }     */
/*   15,  ,{     15,       {         1,        59 }    }     */
/*   16,  ,{     16,       {         1,        64 }    }     */
/*   17,  ,{     17,       {         1,        65 }    }     */
/*   18,  ,{     18,       {         1,        66 }    }     */
/*   19,  ,{     19,       {         1,        67 }    }     */
/*   20,  ,{     20,       {         1,        68 }    }     */
/*   21,  ,{     21,       {         1,        69 }    }     */
/*   22,  ,{     22,       {         1,        70 }    }     */
/*   23,  ,{     23,       {         1,        71 }    }     */
/*   ------------------------------------------------------- */
/*   24,  ,{     24,       {         2,        56 }    }     */
/*   25,  ,{     25,       {         2,        57 }    }     */
/*   26,  ,{     26,       {         2,        58 }    }     */
/*   27,  ,{     27,       {         2,        59 }    }     */
/*   28,  ,{     28,       {         2,        64 }    }     */
/*   29,  ,{     29,       {         2,        65 }    }     */
/*   30,  ,{     30,       {         2,        66 }    }     */
/*   31,  ,{     31,       {         2,        67 }    }     */
/*   63,  ,{     72,       {         2,        72 }    }     */


static PortInitList_STC aldrin_M1_portInitlist_321G_20x10G_3x40G_CPU[] =
{
    /* core 0 */
     { PORT_LIST_TYPE_INTERVAL,  {  0,  3,  1,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
    ,{ PORT_LIST_TYPE_LIST,      {  4,             APP_INV_PORT_CNS }, CPSS_PORT_SPEED_40000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
    ,{ PORT_LIST_TYPE_INTERVAL,  {  8, 11,  1,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }

    /* core 1 */
    ,{ PORT_LIST_TYPE_INTERVAL,  { 12, 15,  1,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
    ,{ PORT_LIST_TYPE_LIST,      { 16,             APP_INV_PORT_CNS }, CPSS_PORT_SPEED_40000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
    ,{ PORT_LIST_TYPE_INTERVAL,  { 20, 23,  1,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
    /* core 2 */
    ,{ PORT_LIST_TYPE_INTERVAL,  { 24, 27,  1,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
    ,{ PORT_LIST_TYPE_LIST,      { 28,             APP_INV_PORT_CNS }, CPSS_PORT_SPEED_40000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
    ,{ PORT_LIST_TYPE_EMPTY,     {                 APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E      }
};


static PortInitList_STC aldrin_M3_portInitlist_321G_32x10G_CPU[] =                                                                      
{
     { PORT_LIST_TYPE_INTERVAL,  {  0,  31, 1,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
    ,{ PORT_LIST_TYPE_EMPTY,     {                 APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E      }
};

#ifdef __cplusplus                               
extern "C" {                                     
#endif /* __cplusplus */                         

extern APPDEMO_UNIT_x_REG_x_VALUE_STC aldrin_M1_regList_321G_20x10G_3x40G_CPU[]; 
extern APPDEMO_UNIT_x_REG_x_VALUE_STC aldrin_M3_regList_321G_32x10G_CPU[]; 


#ifdef __cplusplus                               
}                                                
#endif /* __cplusplus */                         


static APPDEMO_ASIC_woTM_GOP_COMPARISON_SCENARIO_STC aldrin_scenarioList[] = 
{  /* scenario Id,     coreClock,   InitSys,   port list to init,                                      mapping,                   regList2Print               */
     {          1,   {       365,         1,   &aldrin_M1_portInitlist_321G_20x10G_3x40G_CPU[0],       &aldrinDefaultMap[0],     &aldrin_M1_regList_321G_20x10G_3x40G_CPU[0] } }
    ,{          3,   {       480,         1,   &aldrin_M3_portInitlist_321G_32x10G_CPU[0],             &aldrinDefaultMap[0],     &aldrin_M3_regList_321G_32x10G_CPU[0]       } }
    ,{ BAD_VALUE,    { BAD_VALUE, BAD_VALUE,   (PortInitList_STC*)NULL,               (CPSS_DXCH_PORT_MAP_STC*)NULL, (APPDEMO_UNIT_x_REG_x_VALUE_STC*)NULL     } }
};



GT_STATUS aldrinUserScen
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

    for (i = 0 ; aldrin_scenarioList[i].scenarioId != BAD_VALUE; i++)
    {
        if (aldrin_scenarioList[i].scenarioId == scenario)
        {
            scenarioDataPtr = &aldrin_scenarioList[i].scenarioData;
            break;
        }
    }

    if (scenarioDataPtr == NULL)  /* not found */
    {
        cpssOsPrintf("\n-->ERROR Aldrin scenario %d : : Undefined scenario",scenario );
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

    if (scenarioDataPtr->mapPtr != NULL)
    {
        /*------------------------------------------------------------------------*
         * count number of valid lines till CPSS_DXCH_PORT_MAPPING_TYPE_INVALID_E *
         *------------------------------------------------------------------------*/
        for (i = 0 ; scenarioDataPtr->mapPtr[i].mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_INVALID_E; i++);
        mapSize = i;
        
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

    rc = appDemoBc2PortListInit(dev,scenarioDataPtr->portInitlistPtr,GT_FALSE);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (printReg)
    {

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

        if (scenarioDataPtr->regList2Print != NULL)
        {
            rc = asicUserScenRegListPrint(scenarioDataPtr->regList2Print);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        cpssOsPrintf("\n\n");
    }
    return GT_OK;
}





