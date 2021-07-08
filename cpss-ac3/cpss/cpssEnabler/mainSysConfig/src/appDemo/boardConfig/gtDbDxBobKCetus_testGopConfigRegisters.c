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

extern APPDEMO_UNIT_x_REG_x_VALUE_STC cetus_wo_TM_RegList_M4[];

#ifdef __cplusplus
}
#endif /* __cplusplus */

#ifdef WIN32
    #include <stdio.h>
    #define cpssOsPrintf printf
#endif



typedef struct 
{
    GT_U32                  coreClock;
    GT_U32                  initSystemType;
    GT_BOOL                 tmEnable;
    PortInitList_STC       *portInitlistPtr;
    CPSS_DXCH_PORT_MAP_STC *mapPtr;
    APPDEMO_UNIT_x_REG_x_VALUE_STC        *regList2Print;
}APPDEMO_BOBK_SCENARIO_DATA_STC;

typedef struct 
{
    GT_U32                          scenarioId;
    APPDEMO_BOBK_SCENARIO_DATA_STC  scenarioData;
}APPDEMO_BOBK_SCENARIO_STC;

#ifdef WIN32
    #define cpssOsPrintf printf
#endif

#define BAD_VALUE (GT_U32)(~0)



static CPSS_DXCH_PORT_MAP_STC bobkCetus_withTM_Map_M3[] =
{ /* Port,            mappingType                                     portGroupm, intefaceNum, txQPort, trafficManegerEnable, tmPortInd*/
     {           56, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,         56,        56,            GT_FALSE,          GT_NA}
    ,{           57, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,         57,     GT_NA,             GT_TRUE,             57}
    ,{           58, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,         58,     GT_NA,             GT_TRUE,             58}
    ,{           59, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,         59,     GT_NA,             GT_TRUE,             59}
    ,{           64, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,         64,     GT_NA,             GT_TRUE,             64}
    ,{           65, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,         65,     GT_NA,             GT_TRUE,             65}
    ,{           66, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,         66,     GT_NA,             GT_TRUE,             66}
    ,{           67, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,         67,     GT_NA,             GT_TRUE,             67}
    ,{           68, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,         68,     GT_NA,             GT_TRUE,             68}
    ,{           63,     CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,               0,      GT_NA,        63,            GT_FALSE,          GT_NA}
    ,{    BAD_VALUE,      CPSS_DXCH_PORT_MAPPING_TYPE_INVALID_E,       BAD_VALUE,  BAD_VALUE, BAD_VALUE,            GT_FALSE,      BAD_VALUE}
};

static PortInitList_STC bobkCetus_withTM_portInitlist_M3[] = 
{
     { PORT_LIST_TYPE_LIST,      { 56,57,58,59, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
    ,{ PORT_LIST_TYPE_LIST,      { 64,65,       APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
    ,{ PORT_LIST_TYPE_INTERVAL,  { 66,71,1,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_1000_E,   CPSS_PORT_INTERFACE_MODE_SGMII_E  }
    ,{ PORT_LIST_TYPE_EMPTY,     {              APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E      }
};



static CPSS_DXCH_PORT_MAP_STC bobkCetus_woTM_Map_M4[] =
{ /* Port,            mappingType                                     portGroupm, intefaceNum, txQPort, trafficManegerEnable, tmPortInd*/
     {           56, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,         56,        56,            GT_FALSE,          GT_NA}
    ,{           64, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,         64,        64,            GT_FALSE,          GT_NA}
    ,{           65, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,         65,        65,            GT_FALSE,          GT_NA}
    ,{           66, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,         66,        66,            GT_FALSE,          GT_NA}
    ,{           67, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,         67,        67,            GT_FALSE,          GT_NA}
    ,{           68, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,         68,        68,            GT_FALSE,          GT_NA}
    ,{           69, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,         69,        69,            GT_FALSE,          GT_NA}
    ,{           70, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,         70,        70,            GT_FALSE,          GT_NA}
    ,{           71, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,         71,        71,            GT_FALSE,          GT_NA}
    ,{           63,     CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,               0,      GT_NA,        63,            GT_FALSE,          GT_NA}
    ,{    BAD_VALUE,      CPSS_DXCH_PORT_MAPPING_TYPE_INVALID_E,       BAD_VALUE,  BAD_VALUE, BAD_VALUE,            GT_FALSE,      BAD_VALUE}
};

static PortInitList_STC bobkCetus_woTM_portInitlist_M4[] = 
{
     { PORT_LIST_TYPE_LIST,      { 56,          APP_INV_PORT_CNS }, CPSS_PORT_SPEED_40000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
    ,{ PORT_LIST_TYPE_INTERVAL,  { 64,71,1,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
    ,{ PORT_LIST_TYPE_EMPTY,     {              APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,     CPSS_PORT_INTERFACE_MODE_NA_E      }
};

extern APPDEMO_UNIT_x_REG_x_VALUE_STC cetus_wo_TM_RegList_M4[];

static APPDEMO_BOBK_SCENARIO_STC scenarioList[] = 
{  /* scenario Id,     coreClock,   InitSys,   tmEnable,   port list to init,                       mapping,  regList2Print               */
     {          3,   {       167,         2,    GT_TRUE,   &bobkCetus_withTM_portInitlist_M3[0],  &bobkCetus_withTM_Map_M3[0],   (APPDEMO_UNIT_x_REG_x_VALUE_STC*)NULL     } }
    ,{          4,   {       365,         1,   GT_FALSE,   &bobkCetus_woTM_portInitlist_M4[0],    &bobkCetus_woTM_Map_M4[0],     &cetus_wo_TM_RegList_M4[0] } }
    ,{ BAD_VALUE,    { BAD_VALUE, BAD_VALUE,   GT_FALSE,   (PortInitList_STC*)NULL,               (CPSS_DXCH_PORT_MAP_STC*)NULL, (APPDEMO_UNIT_x_REG_x_VALUE_STC*)NULL     } }
};

APPDEMO_BOBK_SCENARIO_DATA_STC * scenarioDataPtr = (APPDEMO_BOBK_SCENARIO_DATA_STC*)NULL;




GT_STATUS bobkCetusUserScen
(
    GT_U32 scenario,
    GT_U32 printReg
)
{
    GT_STATUS rc;
    GT_U8 dev = 0;

    GT_U32 i;
    GT_U32 mapSize = 0;

    for (i = 0 ; scenarioList[i].scenarioId != BAD_VALUE; i++)
    {
        if (scenarioList[i].scenarioId == scenario)
        {
            scenarioDataPtr = &scenarioList[i].scenarioData;
            break;
        }
    }

    if (scenarioDataPtr == NULL)  /* not found */
    {
        cpssOsPrintf("\n-->ERROR BOBK CETUS scenario %d : : Undefined scenario",scenario );
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

    if (scenarioDataPtr->tmEnable == GT_TRUE)
    {
        appDemoBc2SetCustomPortsMapping(GT_TRUE);
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

    if (scenarioDataPtr->tmEnable == GT_TRUE)
    {
        rc = appDemoBc2TmPortsInit(dev);
        if (rc != GT_OK)
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

#if 0
        rc = bobkCetusUserScenPrintReg(scenarioDataPtr->regList2Print);
        if (rc != GT_OK)
        {
            return rc;
        }
#endif
        cpssOsPrintf("\n\n");
    }
    return GT_OK;
}

