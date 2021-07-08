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
* @file cpssDxChVirtualTcamUT.c
*
* @brief The mainUt of CPSS DXCH High Level Virtual TCAM Manager
*
* @version   1
********************************************************************************
*/
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <cpss/dxCh/dxChxGen/virtualTcam/cpssDxChVirtualTcam.h>
#include <cpss/dxCh/dxChxGen/virtualTcam/private/prvCpssDxChVirtualTcamDb.h>
#include <cpss/dxCh/dxChxGen/virtualTcam/private/prvCpssDxChVirtualTcamDb.h>
#include <gtUtil/gtMemoryUsageTrace.h>
#include <gtUtil/gtStringUtil.h>
#include <cpss/dxCh/dxChxGen/virtualTcam/cpssDxChVirtualTcamUT.h>

/* memory leak diagnostics  */
static GT_MEMORY_USAGE_TRACE_ANCHOR_DEFINE_MAC(leak1, "cpssDxChVirtualTcamUT_any_api");
static GT_MEMORY_USAGE_TRACE_ANCHOR_DEFINE_MAC(leak2, "cpssDxChVirtualTcamUT_summary");

static GT_BOOL cpssDxChVirtualTcamUtTraceCpssDxChVirtualTcamResizeEnable = GT_FALSE;

GT_VOID cpssDxChVirtualTcamUtTraceCpssDxChVirtualTcamResizeEnableSet(GT_BOOL enable)
{
    cpssDxChVirtualTcamUtTraceCpssDxChVirtualTcamResizeEnable = enable;
}

static GT_VOID  trace_cpssDxChVirtualTcamResize
(
    IN  GT_U32                                vTcamMngId,
    IN  GT_U32                                vTcamId,
    IN  GT_U32                                rulePlace,
    IN  GT_BOOL                               toInsert,
    IN  GT_U32                                sizeInRules
)
{
    GT_STATUS                          rc;
    GT_U32                             oldRulesAmount;
    GT_U32                             newRulesAmount;
    CPSS_DXCH_VIRTUAL_TCAM_USAGE_STC   vTcamUsage;

    if (cpssDxChVirtualTcamUtTraceCpssDxChVirtualTcamResizeEnable == GT_FALSE)
    {
        return;
    }
    oldRulesAmount = (GT_U32)-1;
    newRulesAmount = (GT_U32)-1;

    rc = cpssDxChVirtualTcamUsageGet(
        vTcamMngId, vTcamId, &vTcamUsage);
    if (rc == GT_OK)
    {
        oldRulesAmount = (vTcamUsage.rulesFree + vTcamUsage.rulesUsed);
        if (toInsert == GT_FALSE)
        {
            newRulesAmount = oldRulesAmount - sizeInRules;
        }
        else
        {
            newRulesAmount = oldRulesAmount + sizeInRules;
        }
    }
    cpssOsPrintf(
        "call cpssDxChVirtualTcamResize vTcamId %d rulePlace %d toInsert %d sizeInRules %d\n",
        vTcamId, rulePlace, toInsert, sizeInRules);
    cpssOsPrintf(
        "oldRulesAmount %d newRulesAmount %d\n",
        oldRulesAmount, newRulesAmount);

}

static GT_BOOL      debug_writeRulePosition_enable = GT_FALSE;
static GT_BOOL priorityMode = GT_FALSE;
static GT_U32  priorityArr[5] = {555,666,777,888,999};
/* indication to validate rules content by direct PCL CPSS APIs (not via tcam manager API) */
static GT_BOOL readByPclCpssApi = GT_FALSE;
static GT_U8   devListArr[PRV_CPSS_MAX_PP_DEVICES_CNS];
static CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT prvRuleSize = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E;
static CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_ENT prvRuleType = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;

static GT_U32   existingVtcamsBmp[(CPSS_DXCH_VIRTUAL_TCAM_VTCAM_MAX_ID_CNS+32)/32] = {0};
static void clearVtcamBmp(void)
{
    cpssOsMemSet(existingVtcamsBmp,0,sizeof(existingVtcamsBmp));
}
static void addVtcamToBmp(IN GT_U32 vTcamId)
{
    existingVtcamsBmp[vTcamId>>5] |= (1 << (vTcamId & 0x1f));
}
static void removeVtcamFromBmp(IN GT_U32 vTcamId)
{
    existingVtcamsBmp[vTcamId>>5] &= ~ (1 << (vTcamId & 0x1f));
}
/* return 1 if vtcam in BMP m 0 if not */
static GT_U32 isVtcamInBmp(IN GT_U32 vTcamId)
{
    return (existingVtcamsBmp[vTcamId>>5] & (1 << (vTcamId & 0x1f))) ? 1 : 0;
}

/* forward declaration */
GT_VOID cpssDxChVirtualTcamUt_vTcamAllocStatusPrint(
    GT_U32 vTcamMngId, GT_U32 vTcamId);

/* set rule with unique indication for PCL type rule*/
#define UNIQUE_PCL_STD_RULE_SET_MAC(_patternPtr,_uniqueValue_ruleId) \
    (_patternPtr)->ruleStdNotIp.udb15_17[0] = (GT_U8)(((_uniqueValue_ruleId) >>  0) & 0xFF);/* lower 8 bits */ \
    (_patternPtr)->ruleStdNotIp.udb15_17[1] = (GT_U8)(((_uniqueValue_ruleId) >>  8) & 0xFF);/* next 8 bits */ \
    (_patternPtr)->ruleStdNotIp.udb15_17[2] = (GT_U8)(((_uniqueValue_ruleId) >> 16) & 0xFF) /* next 8 bits */
#define UNIQUE_PCL_EXT_RULE_SET_MAC(_patternPtr,_uniqueValue_ruleId) \
    (_patternPtr)->ruleExtNotIpv6.udb0_5[0] = (GT_U8)(((_uniqueValue_ruleId) >>  0) & 0xFF);/* lower 8 bits */ \
    (_patternPtr)->ruleExtNotIpv6.udb0_5[1] = (GT_U8)(((_uniqueValue_ruleId) >>  8) & 0xFF);/* next 8 bits */ \
    (_patternPtr)->ruleExtNotIpv6.udb0_5[2] = (GT_U8)(((_uniqueValue_ruleId) >> 16) & 0xFF) /* next 8 bits */
#define UNIQUE_PCL_ULTRA_RULE_SET_MAC(_patternPtr,_uniqueValue_ruleId) \
    (_patternPtr)->ruleUltraIpv6PortVlanQos.udb0_11[0] = (GT_U8)(((_uniqueValue_ruleId) >>  0) & 0xFF);/* lower 8 bits */ \
    (_patternPtr)->ruleUltraIpv6PortVlanQos.udb0_11[1] = (GT_U8)(((_uniqueValue_ruleId) >>  8) & 0xFF);/* next 8 bits */ \
    (_patternPtr)->ruleUltraIpv6PortVlanQos.udb0_11[2] = (GT_U8)(((_uniqueValue_ruleId) >> 16) & 0xFF) /* next 8 bits */
#define UNIQUE_PCL_UDB_RULE_SET_MAC(_patternPtr,_uniqueValue_ruleId) \
    (_patternPtr)->ruleIngrUdbOnly.udb[5] = (GT_U8)(((_uniqueValue_ruleId) >>  0) & 0xFF);/* lower 8 bits */ \
    (_patternPtr)->ruleIngrUdbOnly.udb[6] = (GT_U8)(((_uniqueValue_ruleId) >>  8) & 0xFF);/* next 8 bits */ \
    (_patternPtr)->ruleIngrUdbOnly.udb[7] = (GT_U8)(((_uniqueValue_ruleId) >> 16) & 0xFF) /* next 8 bits */
#define UNIQUE_PCL_RULE_SET_MAC(_tcamRuleTypePtr,_patternPtr,_uniqueValue_ruleId) \
    if ((_tcamRuleTypePtr)->rule.pcl.ruleFormat == CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E) \
    {   \
        UNIQUE_PCL_STD_RULE_SET_MAC(_patternPtr,_uniqueValue_ruleId);   \
    }   \
    else    \
    if ((_tcamRuleTypePtr)->rule.pcl.ruleFormat == CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E) \
    {   \
        UNIQUE_PCL_EXT_RULE_SET_MAC(_patternPtr,_uniqueValue_ruleId);   \
    }   \
    else    \
    if ((_tcamRuleTypePtr)->rule.pcl.ruleFormat == CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E) \
    {   \
        UNIQUE_PCL_ULTRA_RULE_SET_MAC(_patternPtr,_uniqueValue_ruleId);   \
    }   \
    else    \
    {   \
        UNIQUE_PCL_UDB_RULE_SET_MAC(_patternPtr,_uniqueValue_ruleId);   \
    }

/* get the unique indication from PCL type rule */
#define UNIQUE_PCL_STD_RULE_GET_MAC(_patternPtr,_uniqueValue_ruleId) \
    _uniqueValue_ruleId = (_patternPtr)->ruleStdNotIp.udb15_17[0] |     /* lower 8 bits */ \
                          (_patternPtr)->ruleStdNotIp.udb15_17[1] << 8 |/* next 8 bits */  \
                          (_patternPtr)->ruleStdNotIp.udb15_17[2] << 16/* next 8 bits */
#define UNIQUE_PCL_EXT_RULE_GET_MAC(_patternPtr,_uniqueValue_ruleId) \
    _uniqueValue_ruleId = (_patternPtr)->ruleExtNotIpv6.udb0_5[0] |     /* lower 8 bits */ \
                          (_patternPtr)->ruleExtNotIpv6.udb0_5[1] << 8 |/* next 8 bits */  \
                          (_patternPtr)->ruleExtNotIpv6.udb0_5[2] << 16/* next 8 bits */
#define UNIQUE_PCL_ULTRA_RULE_GET_MAC(_patternPtr,_uniqueValue_ruleId) \
    _uniqueValue_ruleId = (_patternPtr)->ruleUltraIpv6PortVlanQos.udb0_11[0] |     /* lower 8 bits */ \
                          (_patternPtr)->ruleUltraIpv6PortVlanQos.udb0_11[1] << 8 |/* next 8 bits */  \
                          (_patternPtr)->ruleUltraIpv6PortVlanQos.udb0_11[2] << 16/* next 8 bits */
#define UNIQUE_PCL_UDB_RULE_GET_MAC(_patternPtr,_uniqueValue_ruleId) \
    _uniqueValue_ruleId = (_patternPtr)->ruleIngrUdbOnly.udb[5] |     /* lower 8 bits */ \
                          (_patternPtr)->ruleIngrUdbOnly.udb[6] << 8 |/* next 8 bits */  \
                          (_patternPtr)->ruleIngrUdbOnly.udb[7] << 16/* next 8 bits */
#define UNIQUE_PCL_RULE_GET_MAC(_tcamRuleTypePtr,_patternPtr,_uniqueValue_ruleId) \
    if ((_tcamRuleTypePtr)->rule.pcl.ruleFormat == CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E) \
    {   \
        UNIQUE_PCL_STD_RULE_GET_MAC(_patternPtr,_uniqueValue_ruleId);   \
    }   \
    else    \
    if ((_tcamRuleTypePtr)->rule.pcl.ruleFormat == CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E) \
    {   \
        UNIQUE_PCL_EXT_RULE_GET_MAC(_patternPtr,_uniqueValue_ruleId);   \
    }   \
    else    \
    if ((_tcamRuleTypePtr)->rule.pcl.ruleFormat == CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E) \
    {   \
        UNIQUE_PCL_ULTRA_RULE_GET_MAC(_patternPtr,_uniqueValue_ruleId);   \
    }   \
    else    \
    {   \
        UNIQUE_PCL_UDB_RULE_GET_MAC(_patternPtr,_uniqueValue_ruleId);   \
    }

/* set rule with unique indication for TTI type rule*/
#define UNIQUE_TTI_RULE_SET_MAC(_tcamRuleTypePtr,_patternPtr,_uniqueValue_ruleId) \
    if ((_tcamRuleTypePtr)->rule.tti.ruleFormat == CPSS_DXCH_TTI_RULE_IPV4_E) \
    {   \
        (_patternPtr)->ipv4.destIp.arIP[1] = (GT_U8)(((_uniqueValue_ruleId) >>  0) & 0xFF);/* lower 8 bits */ \
        (_patternPtr)->ipv4.destIp.arIP[2] = (GT_U8)(((_uniqueValue_ruleId) >>  8) & 0xFF);/* next 8 bits */ \
        (_patternPtr)->ipv4.destIp.arIP[3] = (GT_U8)(((_uniqueValue_ruleId) >> 16) & 0xFF);/* next 8 bits */   \
    }   \
    else    \
    {   \
        (_patternPtr)->udbArray[7] = (GT_U8)(((_uniqueValue_ruleId) >>  0) & 0xFF);/* lower 8 bits */ \
        (_patternPtr)->udbArray[8] = (GT_U8)(((_uniqueValue_ruleId) >>  8) & 0xFF);/* next 8 bits */ \
        (_patternPtr)->udbArray[9] = (GT_U8)(((_uniqueValue_ruleId) >> 16) & 0xFF);/* next 8 bits */   \
    }

/* get the unique indication from TTI type rule */
#define UNIQUE_TTI_RULE_GET_MAC(_tcamRuleTypePtr,_patternPtr,_uniqueValue_ruleId) \
    if ((_tcamRuleTypePtr)->rule.tti.ruleFormat == CPSS_DXCH_TTI_RULE_IPV4_E) \
    {   \
        _uniqueValue_ruleId = (_patternPtr)->ipv4.destIp.arIP[1] |          /* lower 8 bits */ \
                              (_patternPtr)->ipv4.destIp.arIP[2] << 8 |     /* next 8 bits */ \
                              (_patternPtr)->ipv4.destIp.arIP[3] << 16;     /* next 8 bits */   \
    }   \
    else    \
    {   \
        _uniqueValue_ruleId = (_patternPtr)->udbArray[7] |     /* lower 8 bits */ \
                              (_patternPtr)->udbArray[8] << 8 |/* next 8 bits */  \
                              (_patternPtr)->udbArray[9] << 16;/* next 8 bits */   \
    }

extern GT_VOID prvCpssDxChVirtualTcamDbVTcamDumpByField
(
    IN  GT_U32                                      vTcamMngId,
    IN  GT_U32                                      vTcamId,
    IN  GT_CHAR_PTR                                 fieldNamePtr
);

/* function to force (or not to force) the 'ruleWrite' API to use 'first' position.
   (by default it uses the 'last' position)
 debug mode to allow quick testing of the 'position first' that
   the 'rule Update' API allow to use. */
extern void  prvCpssDxChVirtualTcamDbVTcam_debug_writeRulePositionForceFirstSet(
    GT_BOOL forceFirstPosition
);

/* the com port swallow printings when given enough time */
#define ALLOW_TIME_FOR_COM_PORT_MAC cpssOsTimerWkAfter(50)

/* used debug DB dump of CPSS internal DB */
#define DEBUG_PRINT(vTcamMngId,vTcamId) \
    {                                \
        cpssOsPrintf("-------------DEBUG_PRINT start----------\n");                            \
        prvCpssDxChVirtualTcamDbVTcamDumpByField(vTcamMngId,vTcamId,"segmentsTree");           \
        ALLOW_TIME_FOR_COM_PORT_MAC;                                                           \
        prvCpssDxChVirtualTcamDbVTcamDumpByField(vTcamMngId,vTcamId,"priorityPriTree");        \
        ALLOW_TIME_FOR_COM_PORT_MAC;                                                           \
        prvCpssDxChVirtualTcamDbVTcamDumpByField(vTcamMngId,vTcamId,"prvCpssDxChVirtualTcamDbResizeHwIndexDbArr");     \
        ALLOW_TIME_FOR_COM_PORT_MAC;                                                           \
        prvCpssDxChVirtualTcamDbVTcamDumpByField(vTcamMngId,vTcamId,"usedRulesBitmapArr");     \
        prvCpssDxChVirtualTcamDbVTcamDumpByField(vTcamMngId,vTcamId,"rulesAmount");            \
        prvCpssDxChVirtualTcamDbVTcamDumpByField(vTcamMngId,vTcamId,"usedRulesAmount");        \
        ALLOW_TIME_FOR_COM_PORT_MAC;                                                           \
        prvCpssDxChVirtualTcamDbVTcamDumpByField(vTcamMngId,vTcamId,"ruleIdIdTree");           \
        ALLOW_TIME_FOR_COM_PORT_MAC;                                                           \
        prvCpssDxChVirtualTcamDbVTcamDumpByField(vTcamMngId,vTcamId,"rulePhysicalIndexArr");   \
        ALLOW_TIME_FOR_COM_PORT_MAC;                                                           \
        prvCpssDxChVirtualTcamDbVTcamDumpByField(vTcamMngId,vTcamId,"usedRulesBitmapArr");     \
        ALLOW_TIME_FOR_COM_PORT_MAC;                                                           \
        prvCpssDxChVirtualTcamDbVTcamDumpByField(vTcamMngId,vTcamId,"rulesAmount");            \
        prvCpssDxChVirtualTcamDbVTcamDumpByField(vTcamMngId,vTcamId,"usedRulesAmount");        \
        ALLOW_TIME_FOR_COM_PORT_MAC;                                                           \
        cpssOsPrintf("-------------DEBUG_PRINT end----------\n");                              \
    }

/* used debug DB dump of CPSS internal DB */
#define DEBUG_PRINT2(vTcamMngId,vTcamId) \
    {                                \
        prvCpssDxChVirtualTcamDbVTcamDumpByField(vTcamMngId,vTcamId,"segmentsTree");           \
        prvCpssDxChVirtualTcamDbVTcamDumpByField(vTcamMngId,vTcamId,"rulePhysicalIndexArr");   \
        ALLOW_TIME_FOR_COM_PORT_MAC;                                                           \
        /*prvCpssDxChVirtualTcamDbVTcamDumpByField(vTcamMngId,vTcamId,"ruleIdIdTree");     */  \
        /*prvCpssDxChVirtualTcamDbVTcamDumpByField(vTcamMngId,vTcamId,"priorityPriTree");  */  \
        prvCpssDxChVirtualTcamDbVTcamDumpByField(vTcamMngId,vTcamId,"usedRulesBitmapArr");     \
        prvCpssDxChVirtualTcamDbVTcamDumpByField(vTcamMngId,vTcamId,"rulesAmount");            \
        prvCpssDxChVirtualTcamDbVTcamDumpByField(vTcamMngId,vTcamId,"usedRulesAmount");        \
        /*prvCpssDxChVirtualTcamDbVTcamDumpByField(vTcamMngId,vTcamId,"prvCpssDxChVirtualTcamDbResizeHwIndexDbArr");*/ \
        ALLOW_TIME_FOR_COM_PORT_MAC;                                                           \
    }
/*start stop CPSS trace for the vTcam lib */
#define DO_CPSS_TRACE(enable) \
    {                         \
        cpssLogEnableSet(enable);\
        cpssLogLibEnableSet(CPSS_LOG_LIB_VIRTUAL_TCAM_E,CPSS_LOG_TYPE_ALL_E,enable);\
        cpssLogLibEnableSet(CPSS_LOG_LIB_VIRTUAL_TCAM_E,CPSS_LOG_TYPE_NON_ENTRY_LEVEL_FUNCTION_E,GT_FALSE);\
    }

#define NUM_60_RULES_PER_BLOCK 256

static GT_BOOL prvCpssDxChVirtualTcamDbCheckIsOkDumpOnErrorEnable = GT_FALSE;

GT_VOID prvCpssDxChVirtualTcamDbCheckIsOkDumpOnErrorEnableSet(GT_BOOL enable)
{
    prvCpssDxChVirtualTcamDbCheckIsOkDumpOnErrorEnable = enable;
}

static GT_STATUS checkCpssDbIsOkWithReturnCode(
    IN GT_U32   vTcamMngId,
    IN GT_U32   vTcamId
)
{
    return prvCpssDxChVirtualTcamDbVTcamRulesLayoutVerify(
        vTcamMngId,vTcamId, GT_TRUE /*stopAfterFirstError*/,
        GT_TRUE);
}

static void checkCpssDbIsOk(
    IN GT_U32   vTcamMngId,
    IN GT_U32   vTcamId
)
{
    GT_STATUS   st;

    st = checkCpssDbIsOkWithReturnCode(
        vTcamMngId,vTcamId);
    if ((st != GT_OK) && (prvCpssDxChVirtualTcamDbCheckIsOkDumpOnErrorEnable != GT_FALSE))
    {
        prvCpssDxChVirtualTcamDbVTcamDumpFullTcamLayout(vTcamMngId);
        prvCpssDxChVirtualTcamDbVTcamDumpByField(
            vTcamMngId, vTcamId, "segmentsTree");
        prvCpssDxChVirtualTcamDbVTcamDumpByField(
            vTcamMngId, vTcamId, "rulePhysicalIndexArr");
        prvCpssDxChVirtualTcamDbVTcamDumpByField(
            vTcamMngId, vTcamId, "usedRulesBitmapArr");
    }

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvCpssDxChVirtualTcamDbVTcamRulesLayoutVerify failed\n");
}

static void checkCpssSegmentsDbIsOk(
    IN GT_U32   vTcamMngId
)
{
    GT_STATUS   st;
    /*GT_BOOL            printErrorMessage;

    printErrorMessage = prvUtfLogPrintEnableGet();*/

    st = prvCpssDxChVirtualTcamDbVerifyFullTcamLayout(
        vTcamMngId, GT_TRUE /*stopAfterFirstError*/,
        GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvCpssDxChVirtualTcamDbVerifyFullTcamLayout failed\n");
}

static GT_STATUS checkCpssTreesDbIsOk(
    IN GT_U32   vTcamMngId
)
{
    GT_BOOL            printErrorMessage;

    printErrorMessage = prvUtfLogPrintEnableGet();

    return prvCpssDxChVirtualTcamDbVerifyAllTrees(
        vTcamMngId, GT_FALSE /*stopAfterFirstError*/,
        printErrorMessage);
}

/* function to delete all the vtcams */
static void VT_emptyAllVtcams
(
    IN GT_U32                               vTcamMngId,
    IN GT_U32                               numOfDevs
)
{
    GT_STATUS st, devListRemoveSt;

    clearVtcamBmp();

    devListRemoveSt = cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);

    st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, st,
        "cpssDxChVirtualTcamManagerDevListAdd failed \n");
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, devListRemoveSt,
        "cpssDxChVirtualTcamManagerDevListRemove failed \n");
    return;
}

/* do resize but check that the 'Availability Check' predict the proper
   success/fail of the 'resize' action */
static void internal_ut_cpssDxChVirtualTcamResize(
    IN  GT_U32                                vTcamMngId,
    IN  GT_U32                                vTcamId,
    IN  GT_U32                                rulePlace,
    IN  GT_BOOL                               toInsert,
    IN  GT_U32                                sizeInRules,
    IN  GT_STATUS                             *returnCodePtr
)
{
    GT_STATUS st;
    CPSS_DXCH_VIRTUAL_TCAM_AVAILABILTY_ENT     tcamAvailability;
    CPSS_DXCH_VIRTUAL_TCAM_CHECK_INFO_STC   vTcamCheckInfoArr[1];
    GT_U32                                  vTCAMAmount = 1;
    CPSS_DXCH_VIRTUAL_TCAM_USAGE_STC        vTcamUsage;
    GT_BOOL EXPECTED_BAD_PARAM = GT_FALSE;

    *returnCodePtr = GT_NOT_READY;

    vTcamCheckInfoArr[0].isExist = GT_TRUE;
    vTcamCheckInfoArr[0].vTcamId = vTcamId;

    /* get tcam info */
    st = cpssDxChVirtualTcamInfoGet(vTcamMngId,vTcamId,&vTcamCheckInfoArr[0].vTcamInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamInfoGet failed\n");

    /* get actual number of rules used+free in the vtcam */
    st = cpssDxChVirtualTcamUsageGet(vTcamMngId,vTcamId,&vTcamUsage);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamUsageGet failed\n");

    vTcamCheckInfoArr[0].vTcamInfo.guaranteedNumOfRules =
        vTcamUsage.rulesUsed + vTcamUsage.rulesFree;
    if(toInsert == GT_TRUE)
    {
        vTcamCheckInfoArr[0].vTcamInfo.guaranteedNumOfRules +=
            sizeInRules;
    }
    else
    {
        if(sizeInRules > vTcamCheckInfoArr[0].vTcamInfo.guaranteedNumOfRules)
        {
            EXPECTED_BAD_PARAM = GT_TRUE;
        }
        else
        {
            if(vTcamUsage.rulesFree < sizeInRules)
            {
                if(priorityMode == GT_TRUE || (sizeInRules > (vTcamUsage.rulesUsed + vTcamUsage.rulesFree)))
                {
                    EXPECTED_BAD_PARAM = GT_TRUE;
                }
            }
            else
            {
                vTcamCheckInfoArr[0].vTcamInfo.guaranteedNumOfRules -=
                    sizeInRules;
            }
        }
    }

    /* predict the success/fail of the next resize operation */
    GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "before cpssDxChVirtualTcamMemoryAvailabilityCheck");
    st = cpssDxChVirtualTcamMemoryAvailabilityCheck(vTcamMngId, vTCAMAmount,
                                               vTcamCheckInfoArr, &tcamAvailability);
    GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "after cpssDxChVirtualTcamMemoryAvailabilityCheck");
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamMemoryAvailabilityCheck failed\n");

    /* do actual resize operation */
    trace_cpssDxChVirtualTcamResize(vTcamMngId,vTcamId,rulePlace,toInsert,sizeInRules);
    GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "before cpssDxChVirtualTcamResize");
    st = cpssDxChVirtualTcamResize(vTcamMngId,vTcamId,rulePlace,toInsert,sizeInRules);
    GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "after cpssDxChVirtualTcamResize");
    *returnCodePtr = st;

    if(EXPECTED_BAD_PARAM == GT_TRUE)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChVirtualTcamResize expected BAD_PARAM");
    }
    else
    if(tcamAvailability == CPSS_DXCH_VIRTUAL_TCAM_AVAILABILTY_AVAILABLE_E)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamResize expected GT_OK");
    }
    else
    {
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamResize expected NOT GT_OK");
    }
}

/* do resize but check that the 'Availability Check' predict the proper
   success/fail of the 'resize' action */
static GT_STATUS ut_cpssDxChVirtualTcamResize(
    IN  GT_U32                                vTcamMngId,
    IN  GT_U32                                vTcamId,
    IN  GT_U32                                rulePlace,
    IN  GT_BOOL                               toInsert,
    IN  GT_U32                                sizeInRules
)
{
    GT_STATUS st;
    internal_ut_cpssDxChVirtualTcamResize(vTcamMngId,vTcamId,
        rulePlace,toInsert,sizeInRules,&st);
    return st;
}


/* do vtcam create but check that the 'Availability Check' predict the proper
   success/fail of the 'vtcam create' action */
static void internal_ut_cpssDxChVirtualTcamCreate(
    IN  GT_U32                                vTcamMngId,
    IN  GT_U32                                vTcamId,
    IN  CPSS_DXCH_VIRTUAL_TCAM_INFO_STC       *vTcamInfoPtr,
    IN  GT_STATUS                             *returnCodePtr
)
{
    GT_STATUS st;
    CPSS_DXCH_VIRTUAL_TCAM_AVAILABILTY_ENT     tcamAvailability;
    CPSS_DXCH_VIRTUAL_TCAM_CHECK_INFO_STC   vTcamCheckInfoArr[1];
    GT_U32                                  vTCAMAmount = 1;

    *returnCodePtr = GT_NOT_READY;

    vTcamCheckInfoArr[0].isExist = GT_FALSE;
    vTcamCheckInfoArr[0].vTcamId = vTcamId;

    /* set vtcam info */
    vTcamCheckInfoArr[0].vTcamInfo = *vTcamInfoPtr;

    /* predict the success/fail of the next resize operation */
    st = cpssDxChVirtualTcamMemoryAvailabilityCheck(vTcamMngId, vTCAMAmount,
                                               vTcamCheckInfoArr, &tcamAvailability);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamMemoryAvailabilityCheck failed\n");

    /* do actual vtcam create operation */
    st = cpssDxChVirtualTcamCreate(vTcamMngId, vTcamId, vTcamInfoPtr);
    if(st == GT_OK)
    {
        addVtcamToBmp(vTcamId);
    }
    *returnCodePtr = st;
    if(tcamAvailability == CPSS_DXCH_VIRTUAL_TCAM_AVAILABILTY_AVAILABLE_E)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamCreate expected GT_OK");
    }
    else
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_FULL, st, "cpssDxChVirtualTcamCreate expected GT_FULL");
    }
}

/* do vtcam create but check that the 'Availability Check' predict the proper
   success/fail of the 'vtcam create' action */
static GT_STATUS ut_cpssDxChVirtualTcamCreate(
    IN  GT_U32                                vTcamMngId,
    IN  GT_U32                                vTcamId,
    IN  CPSS_DXCH_VIRTUAL_TCAM_INFO_STC     *vTcamInfoPtr
)
{
    GT_STATUS st;
    internal_ut_cpssDxChVirtualTcamCreate(vTcamMngId,vTcamId,
        vTcamInfoPtr,&st);
    return st;
}

/* forward declaration */
static void deleteAllVtcamRules(
    IN GT_U32   vTcamMngId,
    IN GT_U32   vTcamId
);


/* do vtcam create but check that the 'Availability Check' predict the proper
   success/fail of the 'vtcam create' action */
static void internal_ut_cpssDxChVirtualTcamRemove(
    IN  GT_U32                                vTcamMngId,
    IN  GT_U32                                vTcamId,
    IN  GT_STATUS                             *returnCodePtr
)
{
    GT_STATUS st;

    deleteAllVtcamRules(vTcamMngId, vTcamId);

    st = cpssDxChVirtualTcamRemove(vTcamMngId, vTcamId);
    if(st == GT_OK)
    {
        removeVtcamFromBmp(vTcamId);
    }
    *returnCodePtr = st;
    /*
        do not check error here because caller may expect the error
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamRemove failed\n");*/
}

/* wrapper for cpssDxChVirtualTcamRemove */
static GT_STATUS ut_cpssDxChVirtualTcamRemove(
    IN  GT_U32                                vTcamMngId,
    IN  GT_U32                                vTcamId
)
{
    GT_STATUS st;
    internal_ut_cpssDxChVirtualTcamRemove(vTcamMngId,vTcamId,&st);
    return st;
}



/* check that the HW hold proper values for all valid rules in the vtcam */
/* NOTE: assumption that the rule entry in HW should hold 'ruleId' indication */
/* assumption is that : PCL udb 30 */
static void internal_checkHwContentAllRules_ByDevNum(
    IN GT_U8    devNum,/*relevant when (readByPclCpssApi == GT_TRUE)*/
    IN GT_U32   vTcamMngId,
    IN GT_U32   vTcamId
)
{
    GT_STATUS   st;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC            vTcamInfo;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC       RuleData_forRead;
    CPSS_DXCH_PCL_ACTION_STC                   pclAction_forRead;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              mask_forRead;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              pattern_forRead;
    CPSS_DXCH_TTI_ACTION_UNT                   ttiAction_forRead;
    CPSS_DXCH_TTI_RULE_UNT                     ttiMask_forRead;
    CPSS_DXCH_TTI_RULE_UNT                     ttiPattern_forRead;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC       TcamRuleType;
    GT_U32  RuleId;
    GT_U32  tmpValue;
    GT_U32  logicalIndex,prev_logicalIndex = 0xFFFFFFFF;
    GT_U32  hwIndex,prev_hwIndex = 0xFFFFFFFF;

    cpssOsMemSet(&vTcamInfo, 0, sizeof(vTcamInfo));
    cpssOsMemSet(&RuleData_forRead, 0, sizeof(RuleData_forRead));

    st = cpssDxChVirtualTcamInfoGet(vTcamMngId,vTcamId,&vTcamInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,
                            "cpssDxChVirtualTcamInfoGet failed");

    if (vTcamInfo.clientGroup == CG_PCL)
    {
        /*get PCL rule format */
        TcamRuleType.ruleType            = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;
        switch(vTcamInfo.ruleSize)
        {
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_10_B_E:
            default:
                TcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E;
                break;
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_20_B_E:
                TcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E;
                break;
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E:
                TcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
                break;
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_40_B_E:
                TcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_40_E;
                break;
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_50_B_E:
                TcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_50_E;
                break;
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_60_B_E:
                TcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E;
                break;
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E:
                TcamRuleType.rule.pcl.ruleFormat =
                    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E;
                break;
        }

        RuleData_forRead.rule.pcl.actionPtr     = &pclAction_forRead;
        RuleData_forRead.rule.pcl.maskPtr       = &mask_forRead;
        RuleData_forRead.rule.pcl.patternPtr    = &pattern_forRead;
    }
    else
    {
        /*get TTI rule format */
        TcamRuleType.ruleType            = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_TTI_E;
        switch(vTcamInfo.ruleSize)
        {
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_10_B_E:
                TcamRuleType.rule.tti.ruleFormat = CPSS_DXCH_TTI_RULE_UDB_10_E;
                break;
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_20_B_E:
                TcamRuleType.rule.tti.ruleFormat = CPSS_DXCH_TTI_RULE_UDB_20_E;
                break;
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E:
            default:
                TcamRuleType.rule.tti.ruleFormat = CPSS_DXCH_TTI_RULE_IPV4_E;
                break;
        }

        RuleData_forRead.rule.tti.actionPtr     = &ttiAction_forRead;
        RuleData_forRead.rule.tti.maskPtr       = &ttiMask_forRead;
        RuleData_forRead.rule.tti.patternPtr    = &ttiPattern_forRead;
    }

    RuleId = CPSS_DXCH_VIRTUAL_TCAM_START_RULE_ID_CNS;

    while (GT_OK == cpssDxChVirtualTcamNextRuleIdGet(
        vTcamMngId, vTcamId, RuleId, &RuleId))
    {
        /* convert rule ID to HW index */
        st = cpssDxChVirtualTcamDbRuleIdToHwIndexConvert(vTcamMngId, vTcamId, RuleId, &logicalIndex, &hwIndex);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamDbRuleIdToHwIndexConvert failed\n");

        if(prev_logicalIndex != 0xFFFFFFFF)
        {
            /* check that the logical index is higher that previous one */
            if(prev_logicalIndex >= logicalIndex)
            {
                UTF_VERIFY_EQUAL3_STRING_MAC(0, 1,
                    "RuleId[%d] hold logical index[%d] that is not higher than logical index[%d] of previous rule \n",
                    RuleId,logicalIndex,prev_logicalIndex);
            }
            /* check that the HW index is higher that previous one */
            if(prev_hwIndex >= hwIndex)
            {
                UTF_VERIFY_EQUAL3_STRING_MAC(0, 1,
                    "RuleId[%d] hold HW index that is not higher than HW index[%d] of previous rule \n",
                    RuleId,hwIndex,prev_hwIndex);
            }
        }
        /* update for next time*/
        prev_logicalIndex = logicalIndex;
        prev_hwIndex = hwIndex;

        cpssOsMemSet(&pclAction_forRead, 0, sizeof(pclAction_forRead));
        cpssOsMemSet(&mask_forRead, 0, sizeof(mask_forRead));
        cpssOsMemSet(&pattern_forRead, 0, sizeof(pattern_forRead));
        cpssOsMemSet(&ttiAction_forRead, 0, sizeof(ttiAction_forRead));
        cpssOsMemSet(&ttiMask_forRead, 0, sizeof(ttiMask_forRead));
        cpssOsMemSet(&ttiPattern_forRead, 0, sizeof(ttiPattern_forRead));
        if(readByPclCpssApi == GT_TRUE)
        {
            if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_FALSE)
            {
                switch(vTcamInfo.ruleSize)
                {
                    case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E:
                        break;
                    case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_60_B_E:
                        hwIndex /= 2;
                        break;
                    case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E:
                        hwIndex /= 4;
                        break;
                    default:
                        /* ignore */
                        break;
                }
            }
            if (vTcamInfo.clientGroup == CG_PCL)
            {
                /* read PCL rule entry */
                st = cpssDxChPclRuleParsedGet(devNum, TcamRuleType.rule.pcl.ruleFormat, hwIndex, 0/*ruleOptionsBmp*/,
                                             &RuleData_forRead.valid, &mask_forRead, &pattern_forRead, &pclAction_forRead);
            }
            else
            {
                /* read TTI rule entry */
                st = cpssDxChTtiRuleGet(devNum,
                            hwIndex,
                            TcamRuleType.rule.tti.ruleFormat,
                            &ttiPattern_forRead,
                            &ttiMask_forRead,
                            CPSS_DXCH_TTI_ACTION_TYPE2_ENT,
                            &ttiAction_forRead);
                if (st == GT_OK)
                {
                    st = cpssDxChTtiRuleValidStatusGet(devNum,
                                hwIndex,
                                &RuleData_forRead.valid);
                }
            }
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChPclRuleParsedGet failed on hwIndex[%d]",hwIndex);
        }
        else
        {
            st = cpssDxChVirtualTcamRuleRead(
                vTcamMngId, vTcamId, RuleId,
                &TcamRuleType, &RuleData_forRead);

            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamRuleRead failed on ruleId[%d]",RuleId);
        }

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, RuleData_forRead.valid,
            "cpssDxChVirtualTcamRuleRead : expected valid entry RuleId[%d] \n",RuleId);

        /* get the unique value that each rule got */
        if (vTcamInfo.clientGroup == CG_PCL)
        {
            UNIQUE_PCL_RULE_GET_MAC(&TcamRuleType,RuleData_forRead.rule.pcl.patternPtr,tmpValue);
        }
        else
        {
            UNIQUE_TTI_RULE_GET_MAC(&TcamRuleType,RuleData_forRead.rule.tti.patternPtr,tmpValue);
        }

        UTF_VERIFY_EQUAL0_STRING_MAC(RuleId, tmpValue, "wrong rule content");
    }

    checkCpssDbIsOk(vTcamMngId,vTcamId);
}

/* check HW rules from specific device , using CPSS APIs */
static void checkHwContentAllRules_fromCpssSpecificDevNum(
    IN GT_U8    devNum,
    IN GT_U32   vTcamMngId,
    IN GT_U32   vTcamId
)
{
    GT_BOOL orig_readByPclCpssApi = readByPclCpssApi;
    readByPclCpssApi = GT_TRUE;

    internal_checkHwContentAllRules_ByDevNum(devNum,vTcamMngId,vTcamId);

    readByPclCpssApi = orig_readByPclCpssApi;
}

static void checkHwContentAllRules(
    IN GT_U32   vTcamMngId,
    IN GT_U32   vTcamId
)
{
    internal_checkHwContentAllRules_ByDevNum(devListArr[0],vTcamMngId,vTcamId);
}

/* function to delete Consecutive Rules from specific ruleId */
void vTcamDeleteConsecutiveRules(
    IN GT_U32   vTcamMngId,
    IN GT_U32   vTcamId,
    IN GT_U32   startRuleId,
    IN GT_U32   numRulesToRemove
)
{
    GT_STATUS   st;
    GT_U32  dummyRuleId;
    GT_U32  RuleId;
    GT_U32  countdown = numRulesToRemove;
    GT_U32 lastRuleId;
    CPSS_DXCH_VIRTUAL_TCAM_USAGE_STC           vTcamUsage;
    GT_U32  startIndex;

    st = cpssDxChVirtualTcamUsageGet(vTcamMngId, vTcamId, &vTcamUsage);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamUsageGet failed \n");

    if(numRulesToRemove != DELETE_ALL_CNS)
    {
        if(vTcamUsage.rulesUsed < numRulesToRemove)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_BAD_PARAM, "vTcamDeleteConsecutiveRules not enough rules to delete needed number of them \n");
            return;
        }
    }

    if(startRuleId == DELETE_RULE_ID_FROM_END_OF_TABLE_CNS)
    {
        /*indication to delete entries from the end*/
        while(1)
        {
            lastRuleId = 0xFFFFFFFF;
            /* loop from the start to find the last one */
            RuleId = CPSS_DXCH_VIRTUAL_TCAM_START_RULE_ID_CNS;
            while (GT_OK == cpssDxChVirtualTcamNextRuleIdGet(
                vTcamMngId, vTcamId, RuleId, &RuleId))
            {
                lastRuleId = RuleId;
            }

            if(lastRuleId == 0xFFFFFFFF)
            {
                break;
            }

            /* delete last ruleId */
            st = cpssDxChVirtualTcamRuleDelete(
                vTcamMngId, vTcamId, lastRuleId);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, st, "cpssDxChVirtualTcamRuleDelete failed\n");
            if (st != GT_OK)
            {
                /* needed when continuing after error */
                break;
            }

            countdown --;
            if(countdown == 0)
            {
                break;
            }
        }
    }
    else
    if(startRuleId == DELETE_RULE_ID_FROM_MIDDLE_OF_TABLE_CNS)
    {
        /*indication to delete entries from the middle */
        if(numRulesToRemove == DELETE_ALL_CNS)
        {
            startIndex = (vTcamUsage.rulesUsed + 1) / 2;
        }
        else
        {
            startIndex = (vTcamUsage.rulesUsed - numRulesToRemove) / 2;
            if(startIndex)
            {
                startIndex--;
            }
        }

        dummyRuleId = 0xFFFF0000;
        RuleId = CPSS_DXCH_VIRTUAL_TCAM_START_RULE_ID_CNS;
        while (GT_OK == cpssDxChVirtualTcamNextRuleIdGet(
            vTcamMngId, vTcamId, RuleId, &RuleId))
        {
            if(startIndex != 0)
            {
                /* skip this index from the start */
                startIndex--;
                /* update the dummyRuleId */
                dummyRuleId = RuleId;

                continue;
            }

            st = cpssDxChVirtualTcamRuleDelete(
                vTcamMngId, vTcamId, RuleId);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, st, "cpssDxChVirtualTcamRuleDelete failed\n");
            if (st != GT_OK)
            {
                /* needed when continuing after error */
                break;
            }

            countdown --;
            if(countdown == 0)
            {
                break;
            }

            /* delete again the rule after 'dummyRuleId' */
            RuleId = dummyRuleId;
        }
    }
    else
    {
        RuleId = startRuleId;/*cause to delete from the start entries */
        while (GT_OK == cpssDxChVirtualTcamNextRuleIdGet(
            vTcamMngId, vTcamId, RuleId, &dummyRuleId))
        {
            st = cpssDxChVirtualTcamRuleDelete(
                vTcamMngId, vTcamId, dummyRuleId);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, st, "cpssDxChVirtualTcamRuleDelete failed\n");
            if (st != GT_OK)
            {
                /* needed when continuing after error */
                break;
            }

            countdown --;
            if(countdown == 0)
            {
                break;
            }
        }
    }

    checkCpssDbIsOk(vTcamMngId,vTcamId);

    if(numRulesToRemove == DELETE_ALL_CNS)
    {
        /* no need to check countdown */
        return;
    }

    UTF_VERIFY_EQUAL0_STRING_MAC(0, countdown, "countdown not ZERO");

}
/* function to check that there are expected used rules */
static  void checkNumUsed(
    IN GT_U32   vTcamMngId,
    IN GT_U32   vTcamId,
    IN GT_U32   expectedUsede
)
{
    GT_STATUS   st;
    CPSS_DXCH_VIRTUAL_TCAM_USAGE_STC           vTcamUsage;

    st = cpssDxChVirtualTcamUsageGet(vTcamMngId, vTcamId, &vTcamUsage);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamUsageGet failed \n");

    UTF_VERIFY_EQUAL0_STRING_MAC(expectedUsede, vTcamUsage.rulesUsed, "checkNumUsed : rulesUsed not as expected \n");
}

/* function to check that there are expected free rules */
static  void checkNumFree(
    IN GT_U32   vTcamMngId,
    IN GT_U32   vTcamId,
    IN GT_U32   expectedFree
)
{
    GT_STATUS   st;
    CPSS_DXCH_VIRTUAL_TCAM_USAGE_STC           vTcamUsage;

    st = cpssDxChVirtualTcamUsageGet(vTcamMngId, vTcamId, &vTcamUsage);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamUsageGet failed \n");

    UTF_VERIFY_EQUAL0_STRING_MAC(expectedFree, vTcamUsage.rulesFree, "checkNumFree : expectedFree not as expected \n");
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChVirtualTcamManagerCreate
(
    IN  GT_U32                              vTcamMngId
)
*/
/* func makeRuleIngrUdbOnly*/
static void setRuleIngrUdbOnly(CPSS_DXCH_PCL_RULE_FORMAT_UNT *mask,
                               CPSS_DXCH_PCL_RULE_FORMAT_UNT *pattern,
                               CPSS_DXCH_PCL_ACTION_STC      *pclAction)
{
    GT_U16 flowId = 2;

    pclAction->pktCmd = CPSS_PACKET_CMD_DROP_HARD_E;
    pclAction->mirror.cpuCode = CPSS_NET_FIRST_USER_DEFINED_E;
    pclAction->qos.ingress.profileAssignIndex = GT_TRUE;
    pclAction->qos.ingress.profileIndex       = 0x13;

    /* Set Mask */
    mask->ruleIngrUdbOnly.replacedFld.pclId = 0x3FF;
    mask->ruleIngrUdbOnly.udb[2] = 0xFF; /* destination MAC */
    mask->ruleIngrUdbOnly.udb[3] = 0xFF;
    mask->ruleIngrUdbOnly.udb[4] = 0xFF;
    mask->ruleIngrUdbOnly.udb[5] = 0xFF;
    mask->ruleIngrUdbOnly.udb[6] = 0xFF;
    mask->ruleIngrUdbOnly.udb[7] = 0xFF;
    mask->ruleIngrUdbOnly.udb[8] = 0xFF;/* Source MAC */
    mask->ruleIngrUdbOnly.udb[9] = 0xFF;
    mask->ruleIngrUdbOnly.udb[10] = 0xFF;
    mask->ruleIngrUdbOnly.udb[11] = 0xFF;
    mask->ruleIngrUdbOnly.udb[12] = 0xFF;
    mask->ruleIngrUdbOnly.udb[13] = 0xFF;
    mask->ruleIngrUdbOnly.udb[14] = 0xFF;/* VLAN ID */
    mask->ruleIngrUdbOnly.udb[15] = 0x0F;
    mask->ruleIngrUdbOnly.udb[16] = 0xFF;/* Source port */
    mask->ruleIngrUdbOnly.udb[17] = 0xFF;/* Ethertype */
    mask->ruleIngrUdbOnly.udb[18] = 0xFF;
    /* Set Pattern */
    pattern->ruleIngrUdbOnly.replacedFld.pclId = 0x000;
    pattern->ruleIngrUdbOnly.udb[2] = 0x00; /* destination MAC */
    pattern->ruleIngrUdbOnly.udb[3] = 0x00;
    pattern->ruleIngrUdbOnly.udb[4] = 0x00;
    pattern->ruleIngrUdbOnly.udb[5] = 0x00;
    pattern->ruleIngrUdbOnly.udb[6] = 0x00;
    pattern->ruleIngrUdbOnly.udb[7] = 0x02;
    pattern->ruleIngrUdbOnly.udb[8] = 0x00; /* Source MAC */
    pattern->ruleIngrUdbOnly.udb[9] = 0x00;
    pattern->ruleIngrUdbOnly.udb[10] = 0x00;
    pattern->ruleIngrUdbOnly.udb[11] = 0x00;
    pattern->ruleIngrUdbOnly.udb[12] = 0x00;
    pattern->ruleIngrUdbOnly.udb[13] = 0x01; /* Source MAC */
    pattern->ruleIngrUdbOnly.udb[14] = (GT_U8)flowId; /* VLAN ID */
    pattern->ruleIngrUdbOnly.udb[15] = (GT_U8)((flowId >> 8)& 0x0F);
    pattern->ruleIngrUdbOnly.udb[16] = 0x20; /* Source port*/
    pattern->ruleIngrUdbOnly.udb[17] = 0x02; /* Ethertype */
    pattern->ruleIngrUdbOnly.udb[18] = 0x89;

}

/* func makeRuleStdNotIp*/
static void setRuleStdNotIp(CPSS_DXCH_PCL_RULE_FORMAT_UNT *mask,
                               CPSS_DXCH_PCL_RULE_FORMAT_UNT *pattern,
                               CPSS_DXCH_PCL_ACTION_STC      *pclAction)
{
    pclAction->pktCmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    pclAction->mirror.cpuCode = CPSS_NET_FIRST_USER_DEFINED_E;
    pclAction->qos.ingress.profileAssignIndex = GT_TRUE;
    pclAction->qos.ingress.profileIndex       = 0x13;

    /* Set Mask */
    mask->ruleStdNotIp.udb15_17[0] = 0xFF;      /* For test only - for case rule is STD */
    mask->ruleStdNotIp.udb15_17[1] = 0xFF;
    mask->ruleStdNotIp.udb15_17[2] = 0xFF;
    /* Set Pattern */
    pattern = pattern;
}

/* func makeRuleExtNotIpv6*/
static void setRuleExtNotIpv6(CPSS_DXCH_PCL_RULE_FORMAT_UNT *mask,
                               CPSS_DXCH_PCL_RULE_FORMAT_UNT *pattern,
                               CPSS_DXCH_PCL_ACTION_STC      *pclAction)
{
    pclAction->pktCmd = CPSS_PACKET_CMD_DROP_HARD_E;
    pclAction->mirror.cpuCode = CPSS_NET_FIRST_USER_DEFINED_E;
    pclAction->qos.ingress.profileAssignIndex = GT_TRUE;
    pclAction->qos.ingress.profileIndex       = 0x13;

    /* Set Mask */
    mask->ruleExtNotIpv6.udb0_5[0] = 0xFF;      /* For test only - for case rule is EXT */
    mask->ruleExtNotIpv6.udb0_5[1] = 0xFF;
    mask->ruleExtNotIpv6.udb0_5[2] = 0xFF;
    /* Set Pattern */
    pattern = pattern;
}

static void setRuleUltraIpv6PortVlanQos(
    CPSS_DXCH_PCL_RULE_FORMAT_UNT *mask,
    CPSS_DXCH_PCL_RULE_FORMAT_UNT *pattern,
    CPSS_DXCH_PCL_ACTION_STC      *pclAction)
{
    pclAction->pktCmd = CPSS_PACKET_CMD_DROP_HARD_E;
    pclAction->mirror.cpuCode = CPSS_NET_FIRST_USER_DEFINED_E;
    pclAction->qos.ingress.profileAssignIndex = GT_TRUE;
    pclAction->qos.ingress.profileIndex       = 0x13;

    /* Set Mask */
    mask->ruleUltraIpv6PortVlanQos.udb0_11[0] = 0xFF;
    mask->ruleUltraIpv6PortVlanQos.udb0_11[1] = 0xFF;
    mask->ruleUltraIpv6PortVlanQos.udb0_11[2] = 0xFF;
    /* Set Pattern */
    pattern = pattern;
}

static void setTtiRuleUdb(CPSS_DXCH_TTI_RULE_UNT *mask,
                                     CPSS_DXCH_TTI_RULE_UNT *pattern,
                                     CPSS_DXCH_TTI_ACTION_UNT *action)
{
    /*Set Pattern */
    pattern->udbArray[7] = 0xAA;
    pattern->udbArray[8] = 0x55;
    pattern->udbArray[9] = 0x33;

    /* Set Mask */
    mask->udbArray[7] = 0xFF;
    mask->udbArray[8] = 0xFF;
    mask->udbArray[9] = 0xFF;

    /* Set Action */
    /* This field must be set with a value different from default */
    action->type2.tag1VlanCmd = CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E;
}

static void setTtiRuleIpv4(CPSS_DXCH_TTI_RULE_UNT *mask,
                                     CPSS_DXCH_TTI_RULE_UNT *pattern,
                                     CPSS_DXCH_TTI_ACTION_UNT *action)
{
    /*Set Pattern */
    pattern->ipv4.common.vid = 0x005;
    pattern->ipv4.destIp.u32Ip = 0x01010101;
    /* Set Mask */
    mask->ipv4.common.vid = 0xfff;
    mask->ipv4.destIp.u32Ip = 0xffffffff;
    /* Set Action */
    action->type2.command = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    action->type2.userDefinedCpuCode = CPSS_NET_FIRST_USER_DEFINED_E;
    /* This field must be set with a value different from default */
    action->type2.ttPassengerPacketType = CPSS_DXCH_TTI_PASSENGER_IPV4V6_E;
    action->type2.tag1VlanCmd = CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E;
}

/*add and write some STD_NOT_IP_E rule by ruleID and priority */
static GT_STATUS addRuleNoHitByIndexPriorityEx(
    GT_U32                                      vTcamMngId,
    GT_U32                                      vTcamId,
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ID              ruleId,
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC  *ruleAttributesPtr)
{
    GT_STATUS                                  rc;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              mask;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              pattern;
    CPSS_DXCH_PCL_ACTION_STC                   pclAction;
    CPSS_DXCH_TTI_RULE_UNT                     ttiMask;
    CPSS_DXCH_TTI_RULE_UNT                     ttiPattern;
    CPSS_DXCH_TTI_ACTION_UNT                   ttiAction;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC       ruleData;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC       tcamRuleType;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC            vTcamInfo;

    ruleData.valid                   = GT_TRUE;
    tcamRuleType.ruleType            = prvRuleType;
    if (tcamRuleType.ruleType == CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E)
    {
        cpssOsMemSet(&vTcamInfo, 0, sizeof(vTcamInfo));
        rc = cpssDxChVirtualTcamInfoGet(vTcamMngId, vTcamId, &vTcamInfo);
        if (rc != GT_OK)
        {
            return rc;
        }
        ruleData.rule.pcl.actionPtr      = &pclAction;
        ruleData.rule.pcl.maskPtr        = &mask;
        ruleData.rule.pcl.patternPtr     = &pattern;
        tcamRuleType.rule.pcl.ruleFormat = (vTcamInfo.ruleSize == CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E) ?
                                            CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E :
                                            CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;

        cpssOsMemSet(&mask, '\0', sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
        cpssOsMemSet(&pattern, '\0', sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
        cpssOsMemSet(&pclAction, '\0', sizeof(CPSS_DXCH_PCL_ACTION_STC));
        /* Set Mask */

        if (vTcamInfo.ruleSize == CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E)
        {
            setRuleUltraIpv6PortVlanQos(&mask, &pattern, &pclAction);
        }
        else
        {
            setRuleStdNotIp(&mask, &pattern, &pclAction);
        }
    }
    else
    {
        ruleData.rule.tti.actionPtr      = &ttiAction;
        ruleData.rule.tti.maskPtr        = &ttiMask;
        ruleData.rule.tti.patternPtr     = &ttiPattern;
        tcamRuleType.rule.tti.ruleFormat = CPSS_DXCH_TTI_RULE_IPV4_E;

        cpssOsMemSet(&ttiMask, 0, sizeof(ttiMask));
        cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));
        cpssOsMemSet(&ttiAction, 0, sizeof(ttiAction));

        setTtiRuleIpv4(&ttiMask, &ttiPattern, &ttiAction);
    }

    rc = cpssDxChVirtualTcamRuleWrite(vTcamMngId, vTcamId, ruleId,
                                          ruleAttributesPtr, &tcamRuleType, &ruleData);
    return rc;
}

static void addRuleNoHitByIndexPriority(
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ID              ruleId,
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC  *ruleAttributesPtr)
{
    GT_STATUS                                  rc;
    rc = addRuleNoHitByIndexPriorityEx(
        1 /*vTcamMngId*/, 1 /*vTcamId*/,
        ruleId, ruleAttributesPtr);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "cpssDxChVirtualTcamRuleWrite: vTCAM = %d, RuleID = %d",
        1 /*vTcamId*/, ruleId);
}

/* fill vtcam with max number of rules */
static void fillVtcamWithRules(
    IN GT_U32   vTcamMngId,
    IN GT_U32   vTcamId,
    IN GT_U32  maxRulesToFill
)
{
    GT_STATUS   st;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC            vTcamInfo;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC RuleAttributes;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC       RuleData;
    CPSS_DXCH_PCL_ACTION_STC                   pclAction;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              mask;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              pattern;
    CPSS_DXCH_VIRTUAL_TCAM_USAGE_STC           vTcamUsage;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC       TcamRuleType;
    GT_U32  minRulesToFill;
    GT_U32  RuleId;

    cpssOsMemSet(&vTcamInfo, '\0', sizeof(vTcamInfo));
    st = cpssDxChVirtualTcamInfoGet(vTcamMngId,vTcamId,&vTcamInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,
                                "cpssDxChVirtualTcamInfoGet failed");

    RuleAttributes.priority          = 0;
    TcamRuleType.ruleType            = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;
    switch(vTcamInfo.ruleSize)
    {
        case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_10_B_E:
        default:
            TcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E;
            break;
        case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_20_B_E:
            TcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E;
            break;
        case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E:
            TcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
            break;
        case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_40_B_E:
            TcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_40_E;
            break;
        case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_50_B_E:
            TcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_50_E;
            break;
        case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_60_B_E:
            TcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E;
            break;
        case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E:
            TcamRuleType.rule.pcl.ruleFormat =
                CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E;
            break;
    }

    RuleData.valid                   = GT_TRUE;
    /* set new rule */
    cpssOsMemSet(&mask, '\0', sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
    cpssOsMemSet(&pattern, '\0', sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
    cpssOsMemSet(&pclAction, '\0', sizeof(CPSS_DXCH_PCL_ACTION_STC));
    /* Set Mask */
    switch (TcamRuleType.rule.pcl.ruleFormat)
    {
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E:

            setRuleStdNotIp(&mask, &pattern, &pclAction);
            break;

        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E:

            setRuleExtNotIpv6(&mask, &pattern, &pclAction);
            break;

        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E:
            setRuleUltraIpv6PortVlanQos(&mask, &pattern, &pclAction);
            break;

        default:

            setRuleIngrUdbOnly(&mask, &pattern, &pclAction);
            break;

    }

    RuleData.rule.pcl.actionPtr     = &pclAction;
    RuleData.rule.pcl.maskPtr       = &mask;
    RuleData.rule.pcl.patternPtr    = &pattern;

    cpssOsMemSet(&vTcamUsage, '\0', sizeof(vTcamUsage));
    st = cpssDxChVirtualTcamUsageGet(vTcamMngId,vTcamId,&vTcamUsage);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,
                                "cpssDxChVirtualTcamUsageGet failed");

    if (maxRulesToFill == 0)
    {
        minRulesToFill = 0;
        maxRulesToFill = vTcamUsage.rulesFree + vTcamUsage.rulesUsed;
    }
    else
    if ((maxRulesToFill == ((GT_U32)(-1))) &&
            (vTcamId == 1) &&
            (vTcamMngId == 1) &&
            (priorityMode))
    {
        /* Special case: we want to auto resize (increase) a vTcam */
        minRulesToFill = vTcamUsage.rulesUsed;
        maxRulesToFill = vTcamUsage.rulesUsed + (256 - (256%12));   /* Allocation is in 12 granularity */
    }
    else
    {
        minRulesToFill = 0;
        maxRulesToFill = MIN(maxRulesToFill , (vTcamUsage.rulesFree + vTcamUsage.rulesUsed));
    }

    for(RuleId = minRulesToFill; RuleId < maxRulesToFill ; RuleId++)
    {
        UNIQUE_PCL_RULE_SET_MAC(&TcamRuleType,&pattern,RuleId);

        RuleAttributes.priority = priorityArr[RuleId % 5];

        st = cpssDxChVirtualTcamRuleWrite(vTcamMngId , vTcamId, RuleId, &RuleAttributes,
                                         &TcamRuleType, &RuleData);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,
                                    "cpssDxChVirtualTcamRuleWrite failed");
    }
}

static void checkVtcamRuleRangeStamps
(
    IN GT_U32   vTcamMngId,
    IN GT_U32   vTcamId,
    IN GT_U32   ruleIdBase,
    IN GT_U32   ruleStampBase,
    IN GT_U32   rulesAmount
)
{
    GT_STATUS   st;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC            vTcamInfo;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC       RuleData_forRead;
    CPSS_DXCH_PCL_ACTION_STC                   pclAction_forRead;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              mask_forRead;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              pattern_forRead;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC       TcamRuleType;
    GT_U32  RuleId;
    GT_U32  tmpValue;

    TcamRuleType.ruleType            = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;

    /*get PCL rule format */
    st = cpssDxChVirtualTcamInfoGet(vTcamMngId,vTcamId,&vTcamInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,
                            "cpssDxChVirtualTcamInfoGet failed");
    switch(vTcamInfo.ruleSize)
    {
        case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_10_B_E:
        default:
            TcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E;
            break;
        case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_20_B_E:
            TcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E;
            break;
        case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E:
            TcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
            break;
        case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_40_B_E:
            TcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_40_E;
            break;
        case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_50_B_E:
            TcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_50_E;
            break;
        case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_60_B_E:
            TcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E;
            break;
        case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E:
            TcamRuleType.rule.pcl.ruleFormat =
                CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E;
            break;
    }

    RuleData_forRead.rule.pcl.actionPtr     = &pclAction_forRead;
    RuleData_forRead.rule.pcl.maskPtr       = &mask_forRead;
    RuleData_forRead.rule.pcl.patternPtr    = &pattern_forRead;

    RuleId = CPSS_DXCH_VIRTUAL_TCAM_START_RULE_ID_CNS;

    for (RuleId = ruleIdBase; (RuleId < (ruleIdBase + rulesAmount)); RuleId++)
    {
        /* update for next time*/
        st = cpssDxChVirtualTcamRuleRead(
            vTcamMngId, vTcamId, RuleId,
            &TcamRuleType, &RuleData_forRead);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamRuleRead failed on ruleId[%d]",RuleId);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, RuleData_forRead.valid,
            "cpssDxChVirtualTcamRuleRead : expected valid entry RuleId[%d] \n",RuleId);

        /* get the unique value that each rule got */
        UNIQUE_PCL_RULE_GET_MAC(&TcamRuleType,RuleData_forRead.rule.pcl.patternPtr,tmpValue);

        UTF_VERIFY_EQUAL0_STRING_MAC(
            (ruleStampBase + RuleId - ruleIdBase), tmpValue, "wrong rule content");
    }
}

static void checkVtcamWithRules(
    IN GT_U32   vTcamMngId,
    IN GT_U32   vTcamId,
    IN GT_U32  maxRulesToCheck
)
{
    GT_STATUS   st;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC            vTcamInfo;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC       RuleData_forRead;
    CPSS_DXCH_PCL_ACTION_STC                   pclAction_forRead;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              mask_forRead;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              pattern_forRead;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC       TcamRuleType;
    GT_U32  RuleId;
    GT_U32  tmpValue;
    GT_U32  logicalIndex,prev_logicalIndex = 0xFFFFFFFF;
    GT_U32  hwIndex,prev_hwIndex = 0xFFFFFFFF;

    TcamRuleType.ruleType            = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;

    /*get PCL rule format */
    {
        cpssOsMemSet(&vTcamInfo, '\0', sizeof(vTcamInfo));
        st = cpssDxChVirtualTcamInfoGet(vTcamMngId,vTcamId,&vTcamInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,
                                "cpssDxChVirtualTcamInfoGet failed");
        switch(vTcamInfo.ruleSize)
        {
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_10_B_E:
            default:
                TcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E;
                break;
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_20_B_E:
                TcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E;
                break;
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E:
                TcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
                break;
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_40_B_E:
                TcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_40_E;
                break;
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_50_B_E:
                TcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_50_E;
                break;
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_60_B_E:
                TcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E;
                break;
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E:
                TcamRuleType.rule.pcl.ruleFormat =
                    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E;
                break;
        }
    }

    RuleData_forRead.rule.pcl.actionPtr     = &pclAction_forRead;
    RuleData_forRead.rule.pcl.maskPtr       = &mask_forRead;
    RuleData_forRead.rule.pcl.patternPtr    = &pattern_forRead;

    RuleId = CPSS_DXCH_VIRTUAL_TCAM_START_RULE_ID_CNS;

    while (GT_OK == cpssDxChVirtualTcamNextRuleIdGet(
        vTcamMngId, vTcamId, RuleId, &RuleId))
    {
        /* convert rule ID to HW index */
        st = cpssDxChVirtualTcamDbRuleIdToHwIndexConvert(vTcamMngId, vTcamId, RuleId, &logicalIndex, &hwIndex);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamDbRuleIdToHwIndexConvert failed\n");

        if(prev_logicalIndex != 0xFFFFFFFF)
        {
            /* check that the logical index is higher that previous one */
            if(prev_logicalIndex >= logicalIndex)
            {
                UTF_VERIFY_EQUAL3_STRING_MAC(0, 1,
                    "RuleId[%d] hold logical index[%d] that is not higher than logical index[%d] of previous rule \n",
                    RuleId,logicalIndex,prev_logicalIndex);
            }
            /* check that the HW index is higher that previous one */
            if(prev_hwIndex >= hwIndex)
            {
                UTF_VERIFY_EQUAL3_STRING_MAC(0, 1,
                    "RuleId[%d] hold HW index that is not higher than HW index[%d] of previous rule \n",
                    RuleId,hwIndex,prev_hwIndex);
            }
        }
        /* update for next time*/
        prev_logicalIndex = logicalIndex;
        prev_hwIndex = hwIndex;

        st = cpssDxChVirtualTcamRuleRead(
            vTcamMngId, vTcamId, RuleId,
            &TcamRuleType, &RuleData_forRead);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamRuleRead failed on ruleId[%d]",RuleId);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, RuleData_forRead.valid,
            "cpssDxChVirtualTcamRuleRead : expected valid entry RuleId[%d] \n",RuleId);

        /* get the unique value that each rule got */
        UNIQUE_PCL_RULE_GET_MAC(&TcamRuleType,RuleData_forRead.rule.pcl.patternPtr,tmpValue);

        /* After a resize in logical index mode, ruleIds are shifted from their unique stamps.
            * this is fixing it, only in a very specific case. */
        if ((maxRulesToCheck == ((GT_U32)-1)) &&
            (vTcamId == 1) &&
            (vTcamMngId == 1) &&
            (RuleId >= 256) &&
            (priorityMode == GT_FALSE))
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(RuleId - 256, tmpValue, "wrong rule content");
        }
        else
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(RuleId, tmpValue, "wrong rule content");
        }
    }
}

static void fillVtcamWithTtiRules(
    IN GT_U32   vTcamMngId,
    IN GT_U32   vTcamId,
    IN GT_U32  maxRulesToFill
)
{
    GT_STATUS   st;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC            vTcamInfo;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC RuleAttributes;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC       RuleData;
    CPSS_DXCH_TTI_ACTION_UNT                   ttiAction;
    CPSS_DXCH_TTI_RULE_UNT                     mask;
    CPSS_DXCH_TTI_RULE_UNT                     pattern;
    CPSS_DXCH_VIRTUAL_TCAM_USAGE_STC           vTcamUsage;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC       TcamRuleType;
    GT_U32  minRulesToFill;
    GT_U32  RuleId;

    cpssOsMemSet(&vTcamInfo, '\0', sizeof(vTcamInfo));
    st = cpssDxChVirtualTcamInfoGet(vTcamMngId,vTcamId,&vTcamInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,
                                "cpssDxChVirtualTcamInfoGet failed");

    RuleAttributes.priority          = 0;
    TcamRuleType.ruleType            = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_TTI_E;
    /* set new rule */
    cpssOsMemSet(&mask, '\0', sizeof(mask));
    cpssOsMemSet(&pattern, '\0', sizeof(pattern));
    cpssOsMemSet(&ttiAction, '\0', sizeof(ttiAction));

    switch(vTcamInfo.ruleSize)
    {
        case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_10_B_E:
            TcamRuleType.rule.tti.ruleFormat = CPSS_DXCH_TTI_RULE_UDB_10_E;
            setTtiRuleUdb(&mask, &pattern, &ttiAction);
            break;
        case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_20_B_E:
            TcamRuleType.rule.tti.ruleFormat = CPSS_DXCH_TTI_RULE_UDB_20_E;
            setTtiRuleUdb(&mask, &pattern, &ttiAction);
            break;
        case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E:
        default:
            TcamRuleType.rule.tti.ruleFormat = CPSS_DXCH_TTI_RULE_IPV4_E;
            setTtiRuleIpv4(&mask, &pattern, &ttiAction);
            break;
    }

    RuleData.valid                   = GT_TRUE;
    RuleData.rule.tti.actionPtr     = &ttiAction;
    RuleData.rule.tti.maskPtr       = &mask;
    RuleData.rule.tti.patternPtr    = &pattern;

    cpssOsMemSet(&vTcamUsage, '\0', sizeof(vTcamUsage));
    st = cpssDxChVirtualTcamUsageGet(vTcamMngId,vTcamId,&vTcamUsage);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,
                                "cpssDxChVirtualTcamUsageGet failed");

    if (maxRulesToFill == 0)
    {
        minRulesToFill = 0;
        maxRulesToFill = vTcamUsage.rulesFree + vTcamUsage.rulesUsed;
    }
    else
    if ((maxRulesToFill == ((GT_U32)(-1))) &&
            (vTcamId == 1) &&
            (vTcamMngId == 1) &&
            (priorityMode))
    {
        /* Special case: we want to auto resize (increase) a vTcam */
        minRulesToFill = vTcamUsage.rulesUsed;
        maxRulesToFill = vTcamUsage.rulesUsed + 256;
    }
    else
    {
        minRulesToFill = 0;
        maxRulesToFill = MIN(maxRulesToFill , (vTcamUsage.rulesFree + vTcamUsage.rulesUsed));
    }


    for(RuleId = minRulesToFill; RuleId < maxRulesToFill ; RuleId++)
    {
        UNIQUE_TTI_RULE_SET_MAC(&TcamRuleType,&pattern,RuleId);

        RuleAttributes.priority = priorityArr[RuleId % 5];

        st = cpssDxChVirtualTcamRuleWrite(vTcamMngId , vTcamId, RuleId, &RuleAttributes,
                                         &TcamRuleType, &RuleData);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,
                                    "cpssDxChVirtualTcamRuleWrite failed");
    }
}

static void checkVtcamWithTtiRules(
    IN GT_U32   vTcamMngId,
    IN GT_U32   vTcamId,
    IN GT_U32  maxRulesToCheck
)
{
    GT_STATUS   st;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC            vTcamInfo;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC       RuleData_forRead;
    CPSS_DXCH_TTI_ACTION_UNT                   ttiAction_forRead;
    CPSS_DXCH_TTI_RULE_UNT                     mask_forRead;
    CPSS_DXCH_TTI_RULE_UNT                     pattern_forRead;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC       TcamRuleType;
    GT_U32  RuleId;
    GT_U32  tmpValue;
    GT_U32  logicalIndex,prev_logicalIndex = 0xFFFFFFFF;
    GT_U32  hwIndex,prev_hwIndex = 0xFFFFFFFF;

    TcamRuleType.ruleType            = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_TTI_E;

    /*get TTI rule format */
    {
        cpssOsMemSet(&vTcamInfo, '\0', sizeof(vTcamInfo));
        st = cpssDxChVirtualTcamInfoGet(vTcamMngId,vTcamId,&vTcamInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,
                                "cpssDxChVirtualTcamInfoGet failed");
        switch(vTcamInfo.ruleSize)
        {
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_10_B_E:
            default:
                TcamRuleType.rule.tti.ruleFormat = CPSS_DXCH_TTI_RULE_UDB_10_E;
                break;
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_20_B_E:
                TcamRuleType.rule.tti.ruleFormat = CPSS_DXCH_TTI_RULE_UDB_20_E;
                break;
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E:
                TcamRuleType.rule.tti.ruleFormat = CPSS_DXCH_TTI_RULE_IPV4_E;
                break;
        }
    }

    RuleData_forRead.rule.tti.actionPtr     = &ttiAction_forRead;
    RuleData_forRead.rule.tti.maskPtr       = &mask_forRead;
    RuleData_forRead.rule.tti.patternPtr    = &pattern_forRead;

    RuleId = CPSS_DXCH_VIRTUAL_TCAM_START_RULE_ID_CNS;

    while (GT_OK == cpssDxChVirtualTcamNextRuleIdGet(
        vTcamMngId, vTcamId, RuleId, &RuleId))
    {
        /* convert rule ID to HW index */
        st = cpssDxChVirtualTcamDbRuleIdToHwIndexConvert(vTcamMngId, vTcamId, RuleId, &logicalIndex, &hwIndex);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamDbRuleIdToHwIndexConvert failed\n");

        if(prev_logicalIndex != 0xFFFFFFFF)
        {
            /* check that the logical index is higher that previous one */
            if(prev_logicalIndex >= logicalIndex)
            {
                UTF_VERIFY_EQUAL3_STRING_MAC(0, 1,
                    "RuleId[%d] hold logical index[%d] that is not higher than logical index[%d] of previous rule \n",
                    RuleId,logicalIndex,prev_logicalIndex);
            }
            /* check that the HW index is higher that previous one */
            if(prev_hwIndex >= hwIndex)
            {
                UTF_VERIFY_EQUAL3_STRING_MAC(0, 1,
                    "RuleId[%d] hold HW index that is not higher than HW index[%d] of previous rule \n",
                    RuleId,hwIndex,prev_hwIndex);
            }
        }
        /* update for next time*/
        prev_logicalIndex = logicalIndex;
        prev_hwIndex = hwIndex;

        st = cpssDxChVirtualTcamRuleRead(
            vTcamMngId, vTcamId, RuleId,
            &TcamRuleType, &RuleData_forRead);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamRuleRead failed on ruleId[%d]",RuleId);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, RuleData_forRead.valid,
            "cpssDxChVirtualTcamRuleRead : expected valid entry RuleId[%d] \n",RuleId);

        /* get the unique value that each rule got */
        UNIQUE_TTI_RULE_GET_MAC(&TcamRuleType,RuleData_forRead.rule.tti.patternPtr,tmpValue);

        /* After a resize in logical index mode, ruleIds are shifted from their unique stamps.
            * this is fixing it, only in a very specific case. */
        if ((maxRulesToCheck == ((GT_U32)-1)) &&
            (vTcamId == 1) &&
            (vTcamMngId == 1) &&
            (RuleId >= 256) &&
            (priorityMode == GT_FALSE))
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(RuleId - 256, tmpValue, "wrong rule content");
        }
        else
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(RuleId, tmpValue, "wrong rule content");
        }
    }
}

static void deleteAllVtcamRules(
    IN GT_U32   vTcamMngId,
    IN GT_U32   vTcamId
)
{
    GT_STATUS   st;
    GT_STATUS   stNext;
    GT_U32  RuleId;
    GT_U32  RuleIdNext;

    stNext = cpssDxChVirtualTcamNextRuleIdGet(
        vTcamMngId, vTcamId, CPSS_DXCH_VIRTUAL_TCAM_START_RULE_ID_CNS, &RuleId);
    while (GT_OK == stNext)
    {
        stNext = cpssDxChVirtualTcamNextRuleIdGet(
            vTcamMngId, vTcamId, RuleId, &RuleIdNext);
        st = cpssDxChVirtualTcamRuleDelete(
            vTcamMngId, vTcamId, RuleId);

        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, st, "pssDxChVirtualTcamRuleDelete failed on ruleId[%d:%d]",vTcamId,RuleId);
        RuleId = RuleIdNext;
    }
}

/* valid values for tcamPattern and tcamMaskPtr */
#define TCAM_PATTERN    0xFEAB
#define TCAM_MASK       0xFCEB
#if 0
static GT_STATUS getFreeVtcamId(
    IN  CPSS_DXCH_VIRTUAL_TCAM_CHECK_INFO_STC vTcamCheckInfoArr[],
    IN  GT_U8 vTCAMAmount,
    OUT GT_U32 *vTcamId)
{
    GT_U32 ii;

    vTcamId = 0;

    for (ii = 0; ii < vTCAMAmount; ii++ )
    {
        if(vTcamCheckInfoArr[ii].vTcamId == 0)
        {
            *vTcamId = ii + 1;
            break;
        }
    }

    return GT_OK;
}
#endif

UTF_TEST_CASE_MAC(cpssDxChVirtualTcam_checkValidInvalidLogicalIndex)
{
    GT_STATUS                                  st   = GT_OK;
    GT_U8                                      dev;
    GT_U32                                     vTcamMngId;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC            vTcamInfo;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC            vTcamInfoGet;
    GT_U32                                     vTcamId;
    CPSS_DXCH_VIRTUAL_TCAM_USAGE_STC           vTcamUsage;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ID             RuleId;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC RuleAttributes;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC       TcamRuleType;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC       RuleData;
    CPSS_DXCH_PCL_ACTION_STC                   pclAction;
    CPSS_DXCH_PCL_ACTION_STC                   pclActionGet;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              mask;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              pattern;
    CPSS_DXCH_TTI_ACTION_UNT                   ttiAction;
    CPSS_DXCH_TTI_ACTION_UNT                   ttiActionGet;
    CPSS_DXCH_TTI_RULE_UNT                     ttiMask;
    CPSS_DXCH_TTI_RULE_UNT                     ttiPattern;
    GT_U32                                     numOfDevs = 0;
    GT_U32                                     notAppFamilyBmp = 0;
    GT_U32                                     utfFamilyBit = 0;
    GT_BOOL                                    isTestSkipped = GT_TRUE;
    GT_BOOL                                    validStatus;
    GT_BOOL                                    isEqual;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_TYPE_STC   actionType;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_DATA_STC   actionData;
    GT_U32                                     logicalIndex;
    GT_U32                                     ruleIndex;
    GT_BOOL                                    isRuleValid;
    GT_U32                                     profileIndexMask = (GT_U32) -1;

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    cpssOsMemSet(&vTcamInfo, 0, sizeof(vTcamInfo));
    cpssOsMemSet(&vTcamInfoGet, 0, sizeof(vTcamInfoGet));
    cpssOsMemSet(&actionType, 0, sizeof(actionType));
    cpssOsMemSet(&actionData, 0, sizeof(actionData));
    cpssOsMemSet(&pclActionGet, 0, sizeof(pclActionGet));
    cpssOsMemSet(&ttiActionGet, 0, sizeof(ttiActionGet));

    vTcamMngId                     = 1;
    vTcamId                        = 1;
    if (prvRuleType == CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E)
    {
        vTcamInfo.clientGroup          = CG_PCL; /* IPCL0 */
    }
    else
    {
        vTcamInfo.clientGroup          = CG_TTI; /* TTI */
        profileIndexMask = 0x7F;
    }
    vTcamInfo.hitNumber            = 0;
    vTcamInfo.ruleSize             = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E;
    vTcamInfo.autoResize           = GT_FALSE;
    vTcamInfo.guaranteedNumOfRules = 1000;
    vTcamInfo.ruleAdditionMethod =
            CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* this feature is on eArch devices, xCat3 and Lion2 */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    notAppFamilyBmp &= (~UTF_XCAT3_E) & (~UTF_LION2_E);

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* 1. Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
              devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }
        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;

        /* xCat3 checks profileIndex at Action to be 7 bits */
        if (PRV_CPSS_SIP_5_CHECK_MAC(devListArr[0]) == GT_FALSE)
        {
            profileIndexMask = 0x7F;
        }

        /* Create vTCAM manager */
        st = cpssDxChVirtualTcamManagerCreate(vTcamMngId);
        if (st == GT_ALREADY_EXIST)
            st = GT_OK;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerCreate failed\n");


        st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        if (st != GT_OK)
        {
            cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
            st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        }
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListAdd failed\n");

        /* 1. Create vTCAM(1) */
        st = ut_cpssDxChVirtualTcamCreate(vTcamMngId, vTcamId, &vTcamInfo);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamCreate failed for vTcamMngId = %d, st = %d\n", vTcamMngId, st);

        if(GT_OK == st)
        {
            st = cpssDxChVirtualTcamInfoGet(vTcamMngId, vTcamId, &vTcamInfoGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamInfoGet failed hitNumber = %d, clientGroup = %d\n",
                                         vTcamInfo.hitNumber, vTcamInfo.clientGroup );

            isEqual = (0 == cpssOsMemCmp((GT_VOID*) &vTcamInfo,
                                   (GT_VOID*) &vTcamInfoGet,
                                    sizeof(vTcamInfoGet))) ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another vTcamInfo than was set: %d", vTcamId);


            /* 3. Get vTCAM (1) usage. Call cpssDxChVirtualTcamUsageGet
               Expected: GT_OK and the same params.     */

            st = cpssDxChVirtualTcamUsageGet(vTcamMngId, vTcamId, &vTcamUsage);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamUsageGet failed rulesUsed = %d / rulesFrees = %d\n",
                                        vTcamUsage.rulesUsed, vTcamUsage.rulesFree );
            UTF_VERIFY_EQUAL0_STRING_MAC(0, vTcamUsage.rulesUsed,
                                        "not correct rulesUsed\n");
            UTF_VERIFY_EQUAL0_STRING_MAC( 1000, vTcamUsage.rulesFree,
                                        "not correct rulesFree\n");


           /*   Add additional rule to vTCAM (1) with logical
           index = 1000 */
           /* Add rule #0 */
           RuleAttributes.priority          = 0; /* Not be used */
           RuleId                           = 1000;
           RuleData.valid                   = GT_TRUE;
           /* set new rule */
           TcamRuleType.ruleType            = prvRuleType;
           if (prvRuleType == CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E)
           {
               TcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
               cpssOsMemSet(&mask, '\0', sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
               cpssOsMemSet(&pattern, '\0', sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
               cpssOsMemSet(&pclAction, '\0', sizeof(CPSS_DXCH_PCL_ACTION_STC));
               /* Set Mask */
               setRuleStdNotIp(&mask, &pattern, &pclAction);

               RuleData.rule.pcl.actionPtr     = &pclAction;
               RuleData.rule.pcl.maskPtr       = &mask;
               RuleData.rule.pcl.patternPtr    = &pattern;
           }
           else
           {
               TcamRuleType.rule.tti.ruleFormat = CPSS_DXCH_TTI_RULE_IPV4_E;
               cpssOsMemSet(&ttiMask, '\0', sizeof(CPSS_DXCH_TTI_RULE_UNT));
               cpssOsMemSet(&ttiPattern, '\0', sizeof(CPSS_DXCH_TTI_RULE_UNT));
               cpssOsMemSet(&ttiAction, '\0', sizeof(CPSS_DXCH_TTI_ACTION_UNT));
               /* Set Mask */
               setTtiRuleIpv4(&ttiMask, &ttiPattern, &ttiAction);

               RuleData.rule.tti.actionPtr     = &ttiAction;
               RuleData.rule.tti.maskPtr       = &ttiMask;
               RuleData.rule.tti.patternPtr    = &ttiPattern;
           }

           st = cpssDxChVirtualTcamRuleWrite(vTcamMngId , vTcamId, RuleId, &RuleAttributes,
                                             &TcamRuleType, &RuleData); /* Should Failed*/
           UTF_VERIFY_NOT_EQUAL0_STRING_MAC(GT_OK, st,
                                            "Add additional rule to vTCAM (1) with logical index = 1000 should failed");

           for(RuleId = 0; RuleId < 1000; RuleId++ )
           {
               /* mark each action by rule id */
               pclAction.qos.ingress.profileIndex = RuleId & profileIndexMask;
               ttiAction.type2.qosProfile = RuleId & profileIndexMask;

               /* get status and get error because rule does not exist yet */
               st = cpssDxChVirtualTcamRuleValidStatusGet(vTcamMngId,vTcamId,RuleId, &validStatus);
               UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st,
                                       "cpssDxChVirtualTcamRuleValidStatusGet on RuleId(%d)failed\n",RuleId);

               st = cpssDxChVirtualTcamRuleWrite(vTcamMngId, vTcamId, RuleId, &RuleAttributes,
               &TcamRuleType, &RuleData);
               UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                           "cpssDxChVirtualTcamRuleWrite on RuleId(%d)failed\n",RuleId);

               /* get and check status */
               st = cpssDxChVirtualTcamRuleValidStatusGet(vTcamMngId,vTcamId,RuleId, &validStatus);
               UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                       "cpssDxChVirtualTcamRuleValidStatusGet on RuleId(%d)failed\n",RuleId);
               UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, validStatus,
                                       "cpssDxChVirtualTcamRuleValidStatusGet result on RuleId(%d)failed\n",RuleId);

           }

           actionType.ruleType = prvRuleType;
           if (prvRuleType == CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E)
           {
               /* test use ingress PCL rules */
               actionType.action.pcl.direction = CPSS_PCL_DIRECTION_INGRESS_E;
               actionData.action.pcl.actionPtr = &pclActionGet;
           }
           else
           {
               /* test use TTI rules */
               actionData.action.tti.actionPtr = &ttiActionGet;
           }

           for(RuleId = 0; RuleId < 1000; RuleId++ )
           {
               /* mark each action by rule id */
               pclAction.qos.ingress.profileIndex = RuleId & profileIndexMask;
               ttiAction.type2.qosProfile = RuleId & profileIndexMask;
               cpssOsMemSet(&pclActionGet, 0, sizeof(pclActionGet));
               cpssOsMemSet(&ttiActionGet, 0, sizeof(ttiActionGet));

               /* get status and get error because rule does not exist yet */
               st = cpssDxChVirtualTcamRuleActionGet(vTcamMngId,vTcamId,RuleId, &actionType, &actionData);
               UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                       "cpssDxChVirtualTcamRuleActionGet on RuleId(%d)failed\n",RuleId);

               if (prvRuleType == CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E)
               {
                   isEqual = (0 == cpssOsMemCmp((GT_VOID*) actionData.action.pcl.actionPtr,
                                          (GT_VOID*) &pclAction,
                                           sizeof(pclActionGet))) ? GT_TRUE : GT_FALSE;
               }
               else
               {
                   isEqual = (0 == cpssOsMemCmp(&ttiActionGet,
                                                &ttiAction,
                                                sizeof(ttiActionGet)));
               }
               UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                          "get another action than was set for rule %d", RuleId);

           }

           st = cpssDxChVirtualTcamUsageGet(vTcamMngId, vTcamId, &vTcamUsage);
           UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamUsageGet failed rulesUsed = %d / rulesFrees = %d\n",
                                       vTcamUsage.rulesUsed, vTcamUsage.rulesFree );
           UTF_VERIFY_EQUAL0_STRING_MAC(1000, vTcamUsage.rulesUsed,
                                       "not correct rulesUsed\n");
           UTF_VERIFY_EQUAL0_STRING_MAC( 0, vTcamUsage.rulesFree,
                                       "not correct rulesFree\n");

           pclAction.mirror.cpuCode = CPSS_NET_CONTROL_E;
           ttiAction.type2.userDefinedCpuCode = CPSS_NET_LAST_USER_DEFINED_E;
           if (prvRuleType == CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E)
           {
               actionData.action.pcl.actionPtr = &pclAction;
           }
           else
           {
               actionData.action.tti.actionPtr = &ttiAction;
           }

           for(RuleId = 0; RuleId < 1000; RuleId++ )
           {
              /* mark each action by rule id */
               pclAction.qos.ingress.profileIndex = (1000 - RuleId) & profileIndexMask;
               ttiAction.type2.qosProfile = (1000 - RuleId) & profileIndexMask;
               /* update vTcam PCL action with different action data */
               st = cpssDxChVirtualTcamRuleActionUpdate(vTcamMngId,vTcamId,RuleId, &actionType, &actionData);
               UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                       "cpssDxChVirtualTcamRuleActionUpdate on RuleId(%d)failed\n",RuleId);
           }

           if (prvRuleType == CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E)
           {
               actionData.action.pcl.actionPtr = &pclActionGet;
           }
           else
           {
               actionData.action.tti.actionPtr = &ttiActionGet;
           }
           /* check PCL actions were updated */
           for(RuleId = 0; RuleId < 1000; RuleId++ )
           {
              /* mark each action by rule id */
               pclAction.qos.ingress.profileIndex = (1000 - RuleId) & profileIndexMask;
               ttiAction.type2.qosProfile = (1000 - RuleId) & profileIndexMask;

               cpssOsMemSet(&pclActionGet, 0, sizeof(pclActionGet));
               cpssOsMemSet(&ttiActionGet, 0, sizeof(ttiActionGet));

               /* get vTcam action */
               st = cpssDxChVirtualTcamRuleActionGet(vTcamMngId,vTcamId,RuleId, &actionType, &actionData);
               UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                       "cpssDxChVirtualTcamRuleActionGet on RuleId(%d)failed\n",RuleId);

               if (prvRuleType == CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E)
               {
                   isEqual = (0 == cpssOsMemCmp((GT_VOID*) actionData.action.pcl.actionPtr,
                                          (GT_VOID*) &pclAction,
                                           sizeof(pclActionGet))) ? GT_TRUE : GT_FALSE;
               }
               else
               {
                   isEqual = (0 == cpssOsMemCmp(&ttiActionGet,
                                                &ttiAction,
                                                sizeof(ttiActionGet)));
               }
               UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                          "get another action than was set for rule %d", RuleId);

           }

           if (prvRuleType == CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E)
           {
               actionData.action.pcl.actionPtr = &pclAction;
           }
           else
           {
               actionData.action.tti.actionPtr = &ttiAction;
           }

           for(RuleId = 0; RuleId < 10; RuleId++ )
           {
               /* set QOS profile index */
               pclAction.qos.ingress.profileIndex = RuleId % 3;
               ttiAction.type2.qosProfile = RuleId % 3;

               /* update vTcam PCL action with different action data */
               st = cpssDxChVirtualTcamRuleActionUpdate(vTcamMngId,vTcamId,RuleId, &actionType, &actionData);
               UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                       "cpssDxChVirtualTcamRuleActionUpdate on RuleId(%d)failed\n",RuleId);
           }

           if (prvRuleType == CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E)
           {
               actionData.action.pcl.actionPtr = &pclActionGet;
           }
           else
           {
               actionData.action.tti.actionPtr = &ttiActionGet;
           }
           /* Read PCL actions from hardware */
           for(RuleId = 0; RuleId < 10; RuleId++ )
           {
              /* mark each action by rule id */
               pclAction.qos.ingress.profileIndex = RuleId % 3;
               ttiAction.type2.qosProfile = RuleId % 3;

               /* convert rule ID to HW index */
               st = cpssDxChVirtualTcamDbRuleIdToHwIndexConvert(vTcamMngId, vTcamId, RuleId, &logicalIndex, &ruleIndex);
               UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamDbRuleIdToHwIndexConvert failed\n");
               /* ruleIndex for Std. rule does not need adjustment */
               if (prvRuleType == CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E)
               {
                   /* read PCL rule entry                              */
                   cpssOsMemSet(&pclActionGet, 0, sizeof(pclActionGet));
                   st = cpssDxChPclRuleParsedGet(devListArr[0],
                                                TcamRuleType.rule.pcl.ruleFormat, ruleIndex, 0,
                                                 &isRuleValid, &mask, &pattern, &pclActionGet);
               }
               else
               {
                   /* read TTI rule entry                              */
                   cpssOsMemSet(&ttiActionGet, 0, sizeof(ttiActionGet));
                   st = cpssDxChTtiRuleGet(devListArr[0], ruleIndex,
                                                TcamRuleType.rule.tti.ruleFormat,
                                                 &ttiPattern, &ttiMask,
                                                 CPSS_DXCH_TTI_ACTION_TYPE2_ENT,
                                                 &ttiActionGet);
               }
               UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, prvRuleType, ruleIndex);

               if (prvRuleType == CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E)
               {
                   isEqual = (0 == cpssOsMemCmp((GT_VOID*) actionData.action.pcl.actionPtr,
                                          (GT_VOID*) &pclAction,
                                           sizeof(pclActionGet))) ? GT_TRUE : GT_FALSE;
               }
               else
               {
                   isEqual = (0 == cpssOsMemCmp(&ttiActionGet,
                                                &ttiAction,
                                                sizeof(ttiActionGet)));
               }
               UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                          "get another action than was set for rule %d", RuleId);
           }

           for(RuleId = 0; RuleId < 1000; RuleId++ )
           {
               st = cpssDxChVirtualTcamRuleValidStatusSet(vTcamMngId, vTcamId, RuleId, GT_FALSE);
               UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                           "cpssDxChVirtualTcamRuleValidStatusSet on RuleId(%d) failed \n",RuleId);
           }

           /* try to remove and get error because rules are still exist */
           st = cpssDxChVirtualTcamRemove(vTcamMngId, vTcamId);
           UTF_VERIFY_NOT_EQUAL0_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamRemove failed\n");

           for(RuleId = 0; RuleId < 1000; RuleId++ )
           {
               /* get and check status */
               st = cpssDxChVirtualTcamRuleValidStatusGet(vTcamMngId,vTcamId,RuleId, &validStatus);
               UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                       "cpssDxChVirtualTcamRuleValidStatusGet on RuleId(%d)failed\n",RuleId);
               UTF_VERIFY_EQUAL1_STRING_MAC(GT_FALSE, validStatus,
                                       "cpssDxChVirtualTcamRuleValidStatusGet result on RuleId(%d)failed\n",RuleId);

           }

           /* try to remove and get error because rules are still exist */
           st = cpssDxChVirtualTcamRemove(vTcamMngId, vTcamId);
           UTF_VERIFY_NOT_EQUAL0_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamRemove failed\n");

           for(RuleId = 0; RuleId < 1000; RuleId++ )
           {
               st = cpssDxChVirtualTcamRuleDelete(vTcamMngId, vTcamId, RuleId);
               UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                           "cpssDxChVirtualTcamRuleDelete on RuleId(%d) failed \n",RuleId);
           }
        }

        st = ut_cpssDxChVirtualTcamRemove(vTcamMngId, vTcamId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamRemove failed\n");

        st = cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListRemove failed for vTcamMngId = %d\n",
                                     vTcamMngId);

        st = cpssDxChVirtualTcamManagerDelete(vTcamMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDelete failed for vTcamMngId = %d\n",
                                     vTcamMngId);
    }

    if (isTestSkipped)
    {
        SKIP_TEST_MAC;
    }
}

UTF_TEST_CASE_MAC(cpssDxChVirtualTcam_checkValidInvalidLogicalIndex_tti)
{
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_ENT prvRuleType_save;

    prvRuleType_save = prvRuleType;
    prvRuleType = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_TTI_E;

    UTF_TEST_CALL_MAC(cpssDxChVirtualTcam_checkValidInvalidLogicalIndex);

    prvRuleType = prvRuleType_save;
}

UTF_TEST_CASE_MAC(cpssDxChVirtualTcam_increaseVtcamSizebyResizing)
{
    GT_STATUS                                  st   = GT_OK;
    GT_STATUS                                  tempSt = GT_OK;

    GT_U8                                      dev;
    GT_U32                                     numOfDevs = 0;
    GT_U32                                     notAppFamilyBmp = 0;
    GT_U32                                     utfFamilyBit = 0;
    GT_BOOL                                    isTestSkipped = GT_TRUE;
    GT_U32                                     vTcamMngId;
    CPSS_DXCH_VIRTUAL_TCAM_USAGE_STC           vTcamUsage;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ID             RuleId, destRule;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC RuleAttributes;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC       RuleData;
    GT_U8                                      vTCAMAmount;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC       TcamRuleType;
    CPSS_DXCH_VIRTUAL_TCAM_CHECK_INFO_STC      vTcamCheckInfoArr[2];
    CPSS_DXCH_PCL_ACTION_STC                   pclAction;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              mask;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              pattern;
    GT_U32                                     i;
    GT_U32                                     writeErrorCounter = 0;

    GT_U32                                     tmpValue;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC       RuleData_forRead;
    CPSS_DXCH_PCL_ACTION_STC                   pclAction_forRead;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              mask_forRead;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              pattern_forRead;
    GT_U32                                     totalRulesAllowed = 0;
    GT_U32                                     totalRulesTested;
    GT_U32                                     totalRulesBase;
    GT_U32                                     totalRulesShift;
    GT_U32                                     totalRulesExtra;
    GT_U32                                     totalRulesUsed;

    /* use seed for randomize according to 'stream number' */
    cpssOsSrand(prvUtfSeedFromStreamNameGet());

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* this feature is on eArch devices, xCat3 and Lion2 */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    notAppFamilyBmp &= (~UTF_XCAT3_E) & (~UTF_LION2_E);

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* 1. Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
              devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }
        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;

        internal_ut_cpssDxChVirtualTcamTotalRulesAllowedPerDevGet(
                    devListArr[0],
                    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E,
                    &totalRulesAllowed);
        totalRulesTested = ((totalRulesAllowed / 12) / 4) * 4;
        totalRulesBase = totalRulesTested / 3;
        totalRulesShift = totalRulesTested / 2;
        totalRulesExtra = totalRulesTested / 8;

        vTCAMAmount = 2;
        vTcamMngId = 1;
        vTcamCheckInfoArr[0].isExist = GT_TRUE;
        vTcamCheckInfoArr[0].vTcamId = 1;
        vTcamCheckInfoArr[0].vTcamInfo.clientGroup = 0; /* TTI and IPCL0 */
        vTcamCheckInfoArr[0].vTcamInfo.guaranteedNumOfRules = totalRulesTested;
        vTcamCheckInfoArr[0].vTcamInfo.hitNumber = 0;
        vTcamCheckInfoArr[0].vTcamInfo.ruleSize =
        CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E;
        vTcamCheckInfoArr[0].vTcamInfo.autoResize = GT_FALSE;
        if(priorityMode == GT_FALSE)
        {
            vTcamCheckInfoArr[0].vTcamInfo.ruleAdditionMethod =
                    CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E;
        }
        else
        {
            vTcamCheckInfoArr[0].vTcamInfo.ruleAdditionMethod =
                    CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E;
        }

        vTcamCheckInfoArr[1].isExist = GT_TRUE;
        vTcamCheckInfoArr[1].vTcamId = 2;
        vTcamCheckInfoArr[1].vTcamInfo.clientGroup = 0; /* TTI and IPCL0 */
        vTcamCheckInfoArr[1].vTcamInfo.guaranteedNumOfRules = totalRulesTested + 100;
        vTcamCheckInfoArr[1].vTcamInfo.hitNumber = 0;
        vTcamCheckInfoArr[1].vTcamInfo.ruleSize =
        CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E;
        vTcamCheckInfoArr[1].vTcamInfo.autoResize = GT_FALSE;
        if(priorityMode == GT_FALSE)
        {
            vTcamCheckInfoArr[1].vTcamInfo.ruleAdditionMethod =
                    CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E;
        }
        else
        {
            vTcamCheckInfoArr[1].vTcamInfo.ruleAdditionMethod =
                    CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E;
        }

        GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "before TcamManagerCreate");
        GT_MEMORY_USAGE_TRACE_NODE_MAC(leak2, "before TcamManagerCreate (leak2)");
        /* Create vTCAM manager */
        st = cpssDxChVirtualTcamManagerCreate(vTcamMngId);
            if (st == GT_ALREADY_EXIST)
                st = GT_OK;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerCreate failed\n");

        st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        if (st != GT_OK)
        {
            cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
            st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        }
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListAdd failed\n numOfDevs - %d", numOfDevs);

        /* 1. Create vTCAM(1) */

        GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "after DevListAdd");
        for (i = 0 ; i < vTCAMAmount; i++)
        {
            st = ut_cpssDxChVirtualTcamCreate(vTcamMngId, vTcamCheckInfoArr[i].vTcamId, &vTcamCheckInfoArr[i].vTcamInfo);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                                        "ut_cpssDxChVirtualTcamCreate failed for vTcamMngId/vTcamId = %d/%d, st = %d \n",
                                        vTcamMngId, vTcamCheckInfoArr[i].vTcamId, st);
        }
        GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "after Creating vTcams");

        /*   Add additional rule to vTCAM (1) with logical
        index = 1000 */
        /* Add rule #0 */
        RuleAttributes.priority          = 0; /* Not be used */
        TcamRuleType.ruleType            = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;
        TcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
        RuleData.valid                   = GT_TRUE;
        /* set new rule */
        cpssOsMemSet(&mask, '\0', sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
        cpssOsMemSet(&pattern, '\0', sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
        cpssOsMemSet(&pclAction, '\0', sizeof(CPSS_DXCH_PCL_ACTION_STC));
        /* Set Mask */
        setRuleStdNotIp(&mask, &pattern, &pclAction);

        RuleData.rule.pcl.actionPtr     = &pclAction;
        RuleData.rule.pcl.maskPtr       = &mask;
        RuleData.rule.pcl.patternPtr    = &pattern;

        RuleData_forRead.rule.pcl.actionPtr     = &pclAction_forRead;
        RuleData_forRead.rule.pcl.maskPtr       = &mask_forRead;
        RuleData_forRead.rule.pcl.patternPtr    = &pattern_forRead;

        writeErrorCounter = 0;
        tempSt = GT_OK;

        for(RuleId = 0; RuleId < totalRulesTested; RuleId++ )
        {
            UNIQUE_PCL_RULE_SET_MAC(&TcamRuleType,&pattern,RuleId);

            if(priorityMode == GT_TRUE)
            {
                RuleAttributes.priority = (RuleId < 400) ? priorityArr[0] :
                                          (RuleId < 800) ? priorityArr[1] :
                                          priorityArr[2];
            }

            GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "before cpssDxChVirtualTcamRuleWrite");
            st = cpssDxChVirtualTcamRuleWrite(vTcamMngId, vTcamCheckInfoArr[0].vTcamId, RuleId, &RuleAttributes,
            &TcamRuleType, &RuleData);
            GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "after cpssDxChVirtualTcamRuleWrite");

            if(st != GT_OK)
            {
                writeErrorCounter++;
                tempSt = st;
            }

            if(RuleId == 0)
            {
                GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "before cpssDxChVirtualTcamRuleDelete");
                st = cpssDxChVirtualTcamRuleDelete(vTcamMngId, vTcamCheckInfoArr[0].vTcamId, RuleId);
                GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "after cpssDxChVirtualTcamRuleDelete");
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamRuleDelete failed on ruleId[%d]",RuleId);
                st = cpssDxChVirtualTcamRuleWrite(vTcamMngId, vTcamCheckInfoArr[0].vTcamId, RuleId, &RuleAttributes,
                    &TcamRuleType, &RuleData);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamRuleWrite failed on ruleId[%d]",RuleId);
            }

        }
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, tempSt,
                                    "cpssDxChVirtualTcamRuleWrite on RuleId(%d)failed for %d amount of Rules \n",RuleId, writeErrorCounter);

        st = cpssDxChVirtualTcamUsageGet(vTcamMngId, vTcamCheckInfoArr[0].vTcamId, &vTcamUsage);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamUsageGet failed rulesUsed = %d / rulesFrees = %d\n",
                                    vTcamUsage.rulesUsed, vTcamUsage.rulesFree );
        UTF_VERIFY_EQUAL0_STRING_MAC(totalRulesTested, vTcamUsage.rulesUsed,
                                    "not correct rulesUsed\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(vTcamCheckInfoArr[0].vTcamInfo.guaranteedNumOfRules-vTcamUsage.rulesUsed, vTcamUsage.rulesFree,
                                    "not correct rulesFree\n");

        GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "after Writing 1000 rules");

        /*
           7. Resize vTCAM(1) with
           additional 100 rules at the end.
        */
        {
            CPSS_DXCH_VIRTUAL_TCAM_AVAILABILTY_ENT     tcamAvailability;
            GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "before cpssDxChVirtualTcamMemoryAvailabilityCheck");
            st = cpssDxChVirtualTcamMemoryAvailabilityCheck(vTcamMngId, vTCAMAmount,
                                                       vTcamCheckInfoArr, &tcamAvailability);
            GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "after cpssDxChVirtualTcamMemoryAvailabilityCheck");

            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamMemoryAvailabilityCheck failed\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_DXCH_VIRTUAL_TCAM_AVAILABILTY_AVAILABLE_E, tcamAvailability,
                                        "Virtual Tcam is not available  failed\n");
        }

        GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "after MemoryAvailabilityCheck");

        /*
            Call ut_cpssDxChVirtualTcamResize.
            Expected: GT_OK.
        */
        if(priorityMode == GT_FALSE)
        {
            GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "before ut_cpssDxChVirtualTcamResize");
            st = ut_cpssDxChVirtualTcamResize(vTcamMngId, vTcamCheckInfoArr[0].vTcamId,
                                   CPSS_DXCH_VIRTUAL_TCAM_APPEND_CNS, GT_TRUE, totalRulesExtra);
            GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "after ut_cpssDxChVirtualTcamResize");
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamResize failed\n");
        }
        else
        {
            GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "before ut_cpssDxChVirtualTcamResize");
            st = ut_cpssDxChVirtualTcamResize(vTcamMngId, vTcamCheckInfoArr[0].vTcamId,
                                   priorityArr[2]/* after priority[2] */, GT_TRUE, totalRulesExtra);
            GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "after ut_cpssDxChVirtualTcamResize");
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamResize failed\n");
        }

        st = cpssDxChVirtualTcamUsageGet(vTcamMngId, vTcamCheckInfoArr[0].vTcamId, &vTcamUsage);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamUsageGet failed rulesUsed = %d / rulesFrees = %d\n",
                                    vTcamUsage.rulesUsed, vTcamUsage.rulesFree );
        UTF_VERIFY_EQUAL0_STRING_MAC(totalRulesTested, vTcamUsage.rulesUsed,
                                    "not correct rulesUsed\n");
        totalRulesUsed = ((totalRulesExtra+vTcamCheckInfoArr[0].vTcamInfo.guaranteedNumOfRules + 3) / 4) * 4;
        UTF_VERIFY_EQUAL0_STRING_MAC( totalRulesUsed-vTcamUsage.rulesUsed, vTcamUsage.rulesFree,
                                    "not correct rulesFree\n");

        /*
           Add additional rule to
           vTCAM (1) with logical
           index = 1000,...,1099
        */

        writeErrorCounter = 0;
        tempSt = GT_OK;

        for(RuleId = totalRulesTested; RuleId < (totalRulesTested+totalRulesExtra); RuleId++ )
        {
            UNIQUE_PCL_RULE_SET_MAC(&TcamRuleType,&pattern,RuleId);

            if(priorityMode == GT_TRUE)
            {
                RuleAttributes.priority = priorityArr[2];
            }

            GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "before cpssDxChVirtualTcamRuleWrite");
            st = cpssDxChVirtualTcamRuleWrite(vTcamMngId, vTcamCheckInfoArr[0].vTcamId, RuleId, &RuleAttributes,
            &TcamRuleType, &RuleData);
            GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "after cpssDxChVirtualTcamRuleWrite");

            if(st != GT_OK)
            {
                writeErrorCounter++;
                tempSt = st;
            }
        }
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, tempSt,
                                    "cpssDxChVirtualTcamRuleWrite failed for %d amount of Rules. Last RuleID(%d) \n",
                                     RuleId, writeErrorCounter);

        st = cpssDxChVirtualTcamUsageGet(vTcamMngId, vTcamCheckInfoArr[0].vTcamId, &vTcamUsage);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamUsageGet failed rulesUsed = %d / rulesFrees = %d\n",
                                    vTcamUsage.rulesUsed, vTcamUsage.rulesFree );
        UTF_VERIFY_EQUAL0_STRING_MAC(totalRulesTested+totalRulesExtra, vTcamUsage.rulesUsed,
                                    "not correct rulesUsed\n");
        totalRulesUsed = ((totalRulesExtra+vTcamCheckInfoArr[0].vTcamInfo.guaranteedNumOfRules + 3) / 4) * 4;
        UTF_VERIFY_EQUAL0_STRING_MAC(totalRulesUsed-vTcamUsage.rulesUsed, vTcamUsage.rulesFree,
                                    "not correct rulesFree");
        GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "after Writing 1000 rules from 1000 to 0");

        for(RuleId = 0; RuleId < (totalRulesTested+totalRulesExtra); RuleId++ )
        {
            GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "before cpssDxChVirtualTcamRuleRead");
            st = cpssDxChVirtualTcamRuleRead(vTcamMngId, vTcamCheckInfoArr[0].vTcamId, RuleId,
            &TcamRuleType, &RuleData_forRead);
            GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "after cpssDxChVirtualTcamRuleRead");

            if(st != GT_OK)
            {
                writeErrorCounter++;
                tempSt = st;
            }

            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamRuleRead failed on ruleId[%d]",RuleId);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, RuleData_forRead.valid,
                "cpssDxChVirtualTcamRuleRead : expected valid entry RuleId[%d] \n",RuleId);

            /* get the unique value that each rule got */
            UNIQUE_PCL_RULE_GET_MAC(&TcamRuleType,RuleData_forRead.rule.pcl.patternPtr,tmpValue);

            UTF_VERIFY_EQUAL0_STRING_MAC(RuleId, tmpValue, "wrong rule content");
        }

        GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "after Reading 1000 rules");

        /*
            Call ut_cpssDxChVirtualTcamResize.
            Expected: GT_OK.
        */
        if(priorityMode == GT_FALSE)
        {
            GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "before ut_cpssDxChVirtualTcamResize");
            st = ut_cpssDxChVirtualTcamResize(vTcamMngId, vTcamCheckInfoArr[0].vTcamId,
                                   totalRulesBase, GT_TRUE, totalRulesShift);
            GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "after ut_cpssDxChVirtualTcamResize");
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamResize failed\n");
        }
        else
        {
            GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "before ut_cpssDxChVirtualTcamResize");
            st = ut_cpssDxChVirtualTcamResize(vTcamMngId, vTcamCheckInfoArr[0].vTcamId,
                                   priorityArr[1]/*pushing priorityArr[2]*/, GT_TRUE, totalRulesShift);
            GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "after ut_cpssDxChVirtualTcamResize");
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamResize failed\n");
        }

        GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "after second ut_TcamResize");

    /*    prvCpssDxChVirtualTcamDbVTcamDump(vTcamMngId, vTcamCheckInfoArr[0].vTcamId);*/

        for(RuleId = 0; RuleId < (totalRulesTested+totalRulesExtra+totalRulesShift); RuleId++ )
        {
            GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "before cpssDxChVirtualTcamRuleRead");
            st = cpssDxChVirtualTcamRuleRead(vTcamMngId, vTcamCheckInfoArr[0].vTcamId, RuleId,
            &TcamRuleType, &RuleData_forRead);
            GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "after cpssDxChVirtualTcamRuleRead");

            if(st != GT_OK)
            {
                writeErrorCounter++;
                tempSt = st;
            }

            if(priorityMode == GT_FALSE)
            {
                if((RuleId >= totalRulesBase) && (RuleId < (totalRulesBase+totalRulesShift)))
                {
                    UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamRuleRead succeeded on non valid ruleId[%d]",RuleId);
                }
                else
                {
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamRuleRead failed on ruleId[%d]",RuleId);
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, RuleData_forRead.valid,
                        "cpssDxChVirtualTcamRuleRead : expected valid entry RuleId[%d] \n",RuleId);

                    /* get the unique value that each rule got */
                    UNIQUE_PCL_RULE_GET_MAC(&TcamRuleType,RuleData_forRead.rule.pcl.patternPtr,tmpValue);

                    if(RuleId > 0)
                    {
                        UTF_VERIFY_NOT_EQUAL0_STRING_MAC(0, tmpValue, "error getting unique value that each rule got");
                    }


                    if(RuleId < totalRulesBase)
                    {
                        UTF_VERIFY_EQUAL0_STRING_MAC(RuleId, tmpValue, "wrong rule content");
                    }
                    else
                    {
                        UTF_VERIFY_EQUAL0_STRING_MAC((RuleId - totalRulesShift/*the shift*/), tmpValue, "wrong rule content");
                    }
                }
            }
            else
            {
                if(RuleId >= totalRulesTested+totalRulesExtra)
                {
                    UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamRuleRead succeeded on non valid ruleId[%d]",RuleId);
                }
                else
                {
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamRuleRead failed on ruleId[%d]",RuleId);
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, RuleData_forRead.valid,
                        "cpssDxChVirtualTcamRuleRead : expected valid entry RuleId[%d] \n",RuleId);

                    /* get the unique value that each rule got */
                    UNIQUE_PCL_RULE_GET_MAC(&TcamRuleType,RuleData_forRead.rule.pcl.patternPtr,tmpValue);

                    UTF_VERIFY_EQUAL0_STRING_MAC(RuleId, tmpValue, "wrong rule content");
                }
            }
        }

        GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "after 100+100+500 rules reading");


        if(priorityMode == GT_FALSE)
        {
            /* move 100 rules down */
            for(RuleId = totalRulesBase+totalRulesShift; RuleId < (totalRulesBase+totalRulesShift+totalRulesExtra); RuleId++ )
            {
                destRule = RuleId-totalRulesShift;
                GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "before cpssDxChVirtualTcamRuleMove");
                st = cpssDxChVirtualTcamRuleMove(vTcamMngId, vTcamCheckInfoArr[0].vTcamId, RuleId, destRule);
                GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "after cpssDxChVirtualTcamRuleMove");
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamRuleMove failed source %d dest %d\n",
                                        RuleId, destRule);
            }

            /* check rules after move */
            for(RuleId = 0; RuleId < (totalRulesTested+totalRulesExtra+totalRulesShift); RuleId++ )
            {
                GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "before cpssDxChVirtualTcamRuleRead");
                st = cpssDxChVirtualTcamRuleRead(
                    vTcamMngId, vTcamCheckInfoArr[0].vTcamId, RuleId,
                    &TcamRuleType, &RuleData_forRead);
                GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "after cpssDxChVirtualTcamRuleRead");

                if(st != GT_OK)
                {
                    writeErrorCounter++;
                    tempSt = st;
                }

                if((RuleId >= (totalRulesBase+totalRulesExtra)) && (RuleId < (totalRulesBase+totalRulesExtra+totalRulesShift)))
                {
                    UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamRuleRead succeeded on non valid ruleId[%d]",RuleId);
                }
                else
                {
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamRuleRead failed on ruleId[%d]",RuleId);
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, RuleData_forRead.valid,
                        "cpssDxChVirtualTcamRuleRead : expected valid entry RuleId[%d] \n",RuleId);

                    /* get the unique value that each rule got */
                    UNIQUE_PCL_RULE_GET_MAC(&TcamRuleType,RuleData_forRead.rule.pcl.patternPtr,tmpValue);

                    if(RuleId > 0)
                    {
                        UTF_VERIFY_NOT_EQUAL0_STRING_MAC(0, tmpValue, "error getting unique value that each rule got");
                    }

                    if(RuleId < totalRulesBase)
                    {
                        UTF_VERIFY_EQUAL0_STRING_MAC(RuleId, tmpValue, "wrong rule content");
                    }
                    else if (RuleId < totalRulesBase+totalRulesExtra)
                    {
                        /* 100 move rules should be here */
                        UTF_VERIFY_EQUAL0_STRING_MAC((RuleId), tmpValue, "wrong rule content");
                    }
                    else
                    {
                        UTF_VERIFY_EQUAL0_STRING_MAC((RuleId - totalRulesShift/*the shift*/), tmpValue, "wrong rule content");
                    }
                }
            }

            GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "after first rules moving and reading");


            /* move 100 rules up */
            for(RuleId = totalRulesBase; RuleId < (totalRulesBase+totalRulesExtra); RuleId++)
            {
                destRule = RuleId+totalRulesShift;
                GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "before cpssDxChVirtualTcamRuleMove");
                st = cpssDxChVirtualTcamRuleMove(vTcamMngId, vTcamCheckInfoArr[0].vTcamId, RuleId, destRule);
                GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "after cpssDxChVirtualTcamRuleMove");
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamRuleMove failed source %d dest %d\n",
                                        RuleId, destRule);
            }

            /* check content and status of rules after move */
            for(RuleId = 0; RuleId < (totalRulesTested+totalRulesExtra+totalRulesShift); RuleId++ )
            {
                GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "before cpssDxChVirtualTcamRuleRead");
                st = cpssDxChVirtualTcamRuleRead(
                    vTcamMngId, vTcamCheckInfoArr[0].vTcamId, RuleId,
                    &TcamRuleType, &RuleData_forRead);
                GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "after cpssDxChVirtualTcamRuleRead");

                if(st != GT_OK)
                {
                    writeErrorCounter++;
                    tempSt = st;
                }

                if((RuleId >= totalRulesBase) && (RuleId < (totalRulesBase+totalRulesShift)))
                {
                    UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamRuleRead succeeded on non valid ruleId[%d]",RuleId);
                }
                else
                {
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamRuleRead failed on ruleId[%d]",RuleId);
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, RuleData_forRead.valid,
                        "cpssDxChVirtualTcamRuleRead : expected valid entry RuleId[%d] \n",RuleId);

                    /* get the unique value that each rule got */
                    UNIQUE_PCL_RULE_GET_MAC(&TcamRuleType,RuleData_forRead.rule.pcl.patternPtr,tmpValue);

                    if(RuleId > 0)
                    {
                        UTF_VERIFY_NOT_EQUAL0_STRING_MAC(0, tmpValue, "error getting unique value that each rule got");
                    }


                    if(RuleId < totalRulesBase)
                    {
                        UTF_VERIFY_EQUAL0_STRING_MAC(RuleId, tmpValue, "wrong rule content");
                    }
                    else
                    {
                        UTF_VERIFY_EQUAL0_STRING_MAC((RuleId - totalRulesShift/*the shift*/), tmpValue, "wrong rule content");
                    }
                }
            }
        }

        GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "after first rules moving and reading");

        for(RuleId = 0; RuleId < (totalRulesTested+totalRulesExtra+totalRulesShift); RuleId++ )
        {
            GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "before cpssDxChVirtualTcamRuleDelete");
            st = cpssDxChVirtualTcamRuleDelete(vTcamMngId, vTcamCheckInfoArr[0].vTcamId, RuleId);
            GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "after cpssDxChVirtualTcamRuleDelete");

            if(st != GT_OK)
            {
                writeErrorCounter++;
                tempSt = st;
            }

            if(priorityMode == GT_FALSE)
            {
                if((RuleId >= totalRulesBase) && (RuleId < (totalRulesBase+totalRulesShift)))
                {
                    UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamRuleDelete succeeded on non valid ruleId[%d]",RuleId);
                }
                else
                {
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamRuleDelete failed on ruleId[%d]",RuleId);
                }
            }
            else
            {
                if(RuleId >= totalRulesTested+totalRulesExtra)
                {
                    UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamRuleDelete succeeded on non valid ruleId[%d]",RuleId);
                }
                else
                {
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamRuleDelete failed on ruleId[%d]",RuleId);
                }
            }
        }

        GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "after vTcams Deleting");

        if(priorityMode == GT_TRUE)
        {
            GT_U32   ii,jj;
            GT_U32   vTcamId = vTcamCheckInfoArr[0].vTcamId;
            GT_U32  startRuleId = 5000,endRuleId;
            GT_U32  numAllocatedRules;
            GT_BOOL  forceSinglePriority;
            GT_BOOL checkHwContent;
            #define RANDOM_SIZE_CNS 0xFFFFFFFF
            struct{
                GT_U32 numIterations;
                GT_U32 resizeNumOfRules;
                GT_BOOL forceSinglePriority;
                GT_BOOL checkHwContent;
            }testedCasesArr[] = {
                /*forceSinglePriority --> heavy run so do minimal iterations only */
                 {15  ,     17                  ,GT_FALSE       ,GT_TRUE}
                /*************************************/
                ,{1000,     317                 ,GT_TRUE        ,GT_FALSE}
                ,{1000,     17                  ,GT_TRUE        ,GT_FALSE}
                ,{1000,     (_1K/2)+1           ,GT_TRUE        ,GT_TRUE}
                ,{1000,     (_1K/2)-1           ,GT_TRUE        ,GT_TRUE}
                ,{1000,     _1K+1               ,GT_TRUE        ,GT_TRUE}
                ,{1000,     _1K-1               ,GT_TRUE        ,GT_TRUE}
                ,{1000,     (3*_1K/2)+1         ,GT_TRUE        ,GT_TRUE}
                ,{1000,     (3*_1K/2)-1         ,GT_TRUE        ,GT_TRUE}
                ,{1000,     RANDOM_SIZE_CNS     ,GT_TRUE        ,GT_TRUE}
            };
            GT_U32  jjStart = 0 , jjEnd = sizeof(testedCasesArr)/sizeof(testedCasesArr[0]);
            GT_U32  iiMax;
            GT_U32  resizeNum,tmpResizeNum;
            GT_U32  priority;

            st = cpssDxChVirtualTcamUsageGet(vTcamMngId, vTcamCheckInfoArr[1].vTcamId, &vTcamUsage);
            UTF_VERIFY_EQUAL3_STRING_MAC(
                GT_OK, st, "cpssDxChVirtualTcamUsageGet failed on vtcam %d rulesUsed = %d / rulesFrees = %d\n",
                vTcamCheckInfoArr[1].vTcamId,vTcamUsage.rulesUsed, vTcamUsage.rulesFree );

            totalRulesAllowed -= (vTcamUsage.rulesUsed + vTcamUsage.rulesFree);

            for(jj = jjStart ; jj < jjEnd ; jj++)
            {
                iiMax     = testedCasesArr[jj].numIterations;
                resizeNum = testedCasesArr[jj].resizeNumOfRules;

                forceSinglePriority = testedCasesArr[jj].forceSinglePriority;
                checkHwContent      = testedCasesArr[jj].checkHwContent;

                if(resizeNum == RANDOM_SIZE_CNS)
                {
                    PRV_UTF_LOG3_MAC("start iteration[%d] with numIterations[%d] 'Random resize sizes' \n",
                        jj,iiMax,resizeNum);
                }
                else
                {
                    PRV_UTF_LOG3_MAC("start iteration[%d] with numIterations[%d] resizeNumOfRules[%d] \n",
                        jj,iiMax,resizeNum);
                }

                st = cpssDxChVirtualTcamUsageGet(vTcamMngId, vTcamId, &vTcamUsage);
                UTF_VERIFY_EQUAL2_STRING_MAC(
                    GT_OK, st, "cpssDxChVirtualTcamUsageGet failed rulesUsed = %d / rulesFrees = %d\n",
                    vTcamUsage.rulesUsed, vTcamUsage.rulesFree );

                /*decrease size to 30*/
                GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "before ut_cpssDxChVirtualTcamResize");
                st = ut_cpssDxChVirtualTcamResize(vTcamMngId, vTcamId,
                                       0/* don't care */, GT_FALSE,
                                       (vTcamUsage.rulesUsed + vTcamUsage.rulesFree - 30) );
                GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "after ut_cpssDxChVirtualTcamResize");
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamResize decrease [%d]  failed\n",
                    (vTcamUsage.rulesUsed + vTcamUsage.rulesFree - 30));

                checkCpssDbIsOk(vTcamMngId,vTcamId);

                for(ii = 0 ; ii < iiMax; ii++)
                {
                    st = cpssDxChVirtualTcamUsageGet(vTcamMngId, vTcamId, &vTcamUsage);
                    UTF_VERIFY_EQUAL2_STRING_MAC(
                        GT_OK, st, "cpssDxChVirtualTcamUsageGet failed rulesUsed = %d / rulesFrees = %d\n",
                        vTcamUsage.rulesUsed, vTcamUsage.rulesFree );

                    numAllocatedRules = (vTcamUsage.rulesUsed + vTcamUsage.rulesFree);

    /*                if(ii >= 103)
                    {
                        cpssOsPrintf("======= ii[%d] ======= ",
                            ii);
                        DEBUG_PRINT2(vTcamMngId,vTcamId);
                    }*/

                    if(resizeNum == RANDOM_SIZE_CNS)
                    {
                        tmpResizeNum = (59 + cpssOsRand()%(_1K/2));
                        PRV_UTF_LOG1_MAC("start resize with random size[%d] \n",
                            tmpResizeNum);
                    }
                    else
                    {
                        tmpResizeNum = resizeNum;
                    }

                    if(debug_writeRulePosition_enable == GT_TRUE)
                    {
                        priority =
                            forceSinglePriority == GT_TRUE ?
                                0:/* since entries comes at first position do resize before our priority */
                                (priorityArr[ii%5]-1);
                    }
                    else
                    {
                        priority =
                            forceSinglePriority == GT_TRUE ?
                                priorityArr[0]:
                                (priorityArr[ii%5]-1);
                    }

                    GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "before ut_cpssDxChVirtualTcamResize");
                    st = ut_cpssDxChVirtualTcamResize(vTcamMngId, vTcamId,
                                           priority, GT_TRUE,
                                           tmpResizeNum);
                    GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "after ut_cpssDxChVirtualTcamResize");
                    if(totalRulesAllowed < (numAllocatedRules + tmpResizeNum))
                    {
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_FULL, st, "ut_cpssDxChVirtualTcamResize should return 'GT_FULL' \n");

                        PRV_UTF_LOG2_MAC("ended iteration[%d] after [%d] resize times \n",
                            jj,ii);
                        break;
                    }
                    else
                    {
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamResize failed\n");
                    }

                    st = cpssDxChVirtualTcamUsageGet(vTcamMngId, vTcamId, &vTcamUsage);
                    UTF_VERIFY_EQUAL2_STRING_MAC(
                        GT_OK, st, "cpssDxChVirtualTcamUsageGet failed rulesUsed = %d / rulesFrees = %d\n",
                        vTcamUsage.rulesUsed, vTcamUsage.rulesFree );

                    endRuleId = (startRuleId + vTcamUsage.rulesFree);
                    if((ii & 0xf) == 0xf)
                    {
                        /* !!! show that we are keep on going !!! */
                        cpssOsPrintf(".");
                        if((ii & 0x1f) == 0x1f)
                        {
                            /* !!! show that we are keep on going !!! */
                            cpssOsPrintf("[%d] ",ii);
                        }
                    }
                    checkCpssDbIsOk(vTcamMngId,vTcamId);

                    if(ii == 81)
                    {
                        /* full print */
                        /*DEBUG_PRINT(vTcamMngId,vTcamId);*/
                    }

                    for(RuleId = startRuleId; RuleId < endRuleId; RuleId++ )
                    {
                        UNIQUE_PCL_RULE_SET_MAC(&TcamRuleType,&pattern,RuleId);

                        RuleAttributes.priority = forceSinglePriority == GT_TRUE ? priorityArr[0]:
                                                  (RuleId % 3) == 0 ? priorityArr[0] :
                                                  (RuleId % 3) == 1 ? priorityArr[1] :
                                                  priorityArr[2];

                        GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "before cpssDxChVirtualTcamRuleWrite");
                        st = cpssDxChVirtualTcamRuleWrite(
                            vTcamMngId, vTcamId, RuleId, &RuleAttributes,
                            &TcamRuleType, &RuleData);
                        GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "after cpssDxChVirtualTcamRuleWrite");
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamRuleWrite failed\n");
                    }

                    if(checkHwContent == GT_TRUE)
                    {
                        checkHwContentAllRules(vTcamMngId,vTcamId);
                    }

                    startRuleId = RuleId;
                }

                GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "before vTcamDeleteConsecutiveRules");

                vTcamDeleteConsecutiveRules(vTcamMngId,
                    vTcamCheckInfoArr[0].vTcamId,
                    CPSS_DXCH_VIRTUAL_TCAM_START_RULE_ID_CNS,
                    DELETE_ALL_CNS);

                GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "after  vTcamDeleteConsecutiveRules");
            }
        }


        for (i = 0 ; i < vTCAMAmount; i++)
        {
            GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "before ut_cpssDxChVirtualTcamRemove");
            st = ut_cpssDxChVirtualTcamRemove(vTcamMngId, vTcamCheckInfoArr[i].vTcamId);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamRemove failed\n");
            GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "after  ut_cpssDxChVirtualTcamRemove");
        }

        GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "after removing vTcams");
        GT_MEMORY_USAGE_TRACE_ISOLATED_NODE_MAC("vtcam_isolated", "ISOLATED: after removing vTcams");

        st = cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListRemove failed for vTcamMngId = %d\n",
                                     vTcamMngId);
        st = cpssDxChVirtualTcamManagerDelete(vTcamMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDelete failed for vTcamMngId = %d\n",
                                     vTcamMngId);

        GT_MEMORY_USAGE_TRACE_NODE_MAC(leak1, "after deleting devices and Tcam Manager");
        GT_MEMORY_USAGE_TRACE_NODE_MAC(leak2, "after deleting devices and Tcam Manager(leak2)");
        GT_MEMORY_USAGE_TRACE_ISOLATED_NODE_MAC("vtcam_isolated", "ISOLATED: after deleting devices and Tcam Manager");
    }

    if (isTestSkipped)
    {
        SKIP_TEST_MAC;
    }
}

UTF_TEST_CASE_MAC(cpssDxChVirtualTcam_decreaseVtcamSizeByResizing)
{
    GT_STATUS                                  st   = GT_OK;
    GT_U8                                      dev;
    GT_U32                                     numOfDevs = 0;
    GT_U32                                     notAppFamilyBmp = 0;
    GT_U32                                     utfFamilyBit = 0;
    GT_BOOL                                    isTestSkipped = GT_TRUE;
    GT_U32                                     vTcamMngId;
    CPSS_DXCH_VIRTUAL_TCAM_USAGE_STC           vTcamUsage;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ID             RuleId;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ID             NextRuleId;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC RuleAttributes;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC       RuleData;
    /*GT_U8                                      vTCAMAmount = 2;*/
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC       TcamRuleType;
    CPSS_DXCH_VIRTUAL_TCAM_CHECK_INFO_STC      vTcamCheckInfoArr[2];
    CPSS_DXCH_PCL_ACTION_STC                   pclAction;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              mask;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              pattern;
    GT_U8                                      RuleID_list[5] = {0, 4, 7, 8, 10};
    GT_STATUS                                  tempSt = GT_OK;
    GT_U32                                     writeErrorCounter = 0;

    GT_U32                                     tmpValue;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC       RuleData_forRead;
    CPSS_DXCH_PCL_ACTION_STC                   pclAction_forRead;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              mask_forRead;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              pattern_forRead;
    GT_BOOL                                     createdVtcam2 = GT_FALSE;
    GT_U32                                     expectedNumUsed = 0;

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* this feature is on eArch devices, xCat3 and Lion2 */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    notAppFamilyBmp &= (~UTF_XCAT3_E) & (~UTF_LION2_E);

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* 1. Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
              devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }
        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;

        createdVtcam2 = GT_FALSE;
        expectedNumUsed = 0;
        vTcamMngId = 1;
        vTcamCheckInfoArr[0].isExist = GT_TRUE;
        vTcamCheckInfoArr[0].vTcamId = 1;
        vTcamCheckInfoArr[0].vTcamInfo.clientGroup = 0; /* TTI and IPCL0 */
        vTcamCheckInfoArr[0].vTcamInfo.guaranteedNumOfRules = 1000;
        vTcamCheckInfoArr[0].vTcamInfo.hitNumber = 0;
        vTcamCheckInfoArr[0].vTcamInfo.ruleSize =
        CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E;
        vTcamCheckInfoArr[0].vTcamInfo.autoResize = GT_FALSE;
        if(priorityMode == GT_FALSE)
        {
            vTcamCheckInfoArr[0].vTcamInfo.ruleAdditionMethod =
                    CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E;
        }
        else
        {
            vTcamCheckInfoArr[0].vTcamInfo.ruleAdditionMethod =
                    CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E;
        }

        /* Create vTCAM manager */
        st = cpssDxChVirtualTcamManagerCreate(vTcamMngId);
            if (st == GT_ALREADY_EXIST)
                st = GT_OK;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerCreate failed\n");

        st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        if (st != GT_OK)
        {
            cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
            st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        }
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListAdd failed\n");

        /* 1. Create vTCAM(1) */
        st = ut_cpssDxChVirtualTcamCreate(vTcamMngId, vTcamCheckInfoArr[0].vTcamId, &vTcamCheckInfoArr[0].vTcamInfo);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                                    "ut_cpssDxChVirtualTcamCreate failed for vTcamMngId/vTcamMngId = %d/%d, st = %d\n",
                                    vTcamMngId, vTcamCheckInfoArr[0].vTcamId, st);


    CPSS_TBD_BOOKMARK
#if 0 /* wait until function will be implemented. no need to fail the test on it */
        {
            CPSS_DXCH_VIRTUAL_TCAM_AVAILABILTY_ENT     tcamAvailability;
            st = cpssDxChVirtualTcamMemoryAvailabilityCheck(vTcamMngId, vTCAMAmount,
                                                       vTcamCheckInfoArr, &tcamAvailability);

            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamMemoryAvailabilityCheck failed\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_DXCH_VIRTUAL_TCAM_AVAILABILTY_AVAILABLE_E, tcamAvailability,
                                        "Virtual Tcam is not available  failed\n");
        }
#endif

        /* Add rule #0 */
        RuleAttributes.priority          = 0; /* Not be used */
        TcamRuleType.ruleType            = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;
        TcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
        RuleData.valid                   = GT_TRUE;
        /* set new rule */
        cpssOsMemSet(&mask, '\0', sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
        cpssOsMemSet(&pattern, '\0', sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
        cpssOsMemSet(&pclAction, '\0', sizeof(CPSS_DXCH_PCL_ACTION_STC));
        /* Set Mask */
        setRuleStdNotIp(&mask, &pattern, &pclAction);

        RuleData.rule.pcl.actionPtr     = &pclAction;
        RuleData.rule.pcl.maskPtr       = &mask;
        RuleData.rule.pcl.patternPtr    = &pattern;

        RuleData_forRead.rule.pcl.actionPtr     = &pclAction_forRead;
        RuleData_forRead.rule.pcl.maskPtr       = &mask_forRead;
        RuleData_forRead.rule.pcl.patternPtr    = &pattern_forRead;

        writeErrorCounter = 0;
        tempSt = GT_OK;

        for(RuleId = 0; RuleId < 1000; RuleId++ )
        {
            UNIQUE_PCL_RULE_SET_MAC(&TcamRuleType,&pattern,RuleId);

            if(priorityMode == GT_TRUE)
            {
                RuleAttributes.priority = (RuleId < 400) ? priorityArr[0] :
                                          (RuleId < 800) ? priorityArr[1] :
                                          priorityArr[2];
            }

            st = cpssDxChVirtualTcamRuleWrite(vTcamMngId, vTcamCheckInfoArr[0].vTcamId, RuleId, &RuleAttributes,
            &TcamRuleType, &RuleData);

            if(st != GT_OK)
            {
                writeErrorCounter++;
                tempSt = st;
            }

            expectedNumUsed++;
        }
        /*checkCpssDbIsOk(vTcamMngId,vTcamCheckInfoArr[0].vTcamId);*/
        /* check that the HW hold proper values */
        checkHwContentAllRules(vTcamMngId, vTcamCheckInfoArr[0].vTcamId);

        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, tempSt,
                                    "cpssDxChVirtualTcamRuleWrite on RuleId(%d)failed for %d amount of Rules \n",RuleId, writeErrorCounter);
        st = cpssDxChVirtualTcamUsageGet(vTcamMngId, vTcamCheckInfoArr[0].vTcamId, &vTcamUsage);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamUsageGet failed rulesUsed = %d / rulesFrees = %d\n",
                                     vTcamUsage.rulesUsed, vTcamUsage.rulesFree );
        /*
           7. decrize vTCAM(1) on 300 rules at the end.
            Call ut_cpssDxChVirtualTcamResize.
            Expected: GT_OK.
        */
        if(priorityMode == GT_FALSE)
        {
            st = ut_cpssDxChVirtualTcamResize(vTcamMngId, vTcamCheckInfoArr[0].vTcamId,
                                   150, GT_FALSE, 300);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamResize failed\n");

            checkCpssDbIsOk(vTcamMngId,vTcamCheckInfoArr[0].vTcamId);
        }
        else
        {
            checkNumUsed(vTcamMngId, vTcamCheckInfoArr[0].vTcamId,expectedNumUsed);
            /* expect fail because no empty 100 rules */
            st = ut_cpssDxChVirtualTcamResize(vTcamMngId, vTcamCheckInfoArr[0].vTcamId,
                                   priorityArr[0]/* after priority[0] */, GT_FALSE, 100);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PARAM, st, "ut_cpssDxChVirtualTcamResize failed\n");
            checkNumUsed(vTcamMngId, vTcamCheckInfoArr[0].vTcamId,expectedNumUsed);

            /* check that the HW hold proper values */
            checkHwContentAllRules(vTcamMngId, vTcamCheckInfoArr[0].vTcamId);

            for(RuleId = 300; RuleId < (300+100); RuleId++ )
            {
                st = cpssDxChVirtualTcamRuleDelete(vTcamMngId, vTcamCheckInfoArr[0].vTcamId, RuleId);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamRuleDelete failed\n");
                expectedNumUsed--;
            }

            /* expect GT_OK */
            st = ut_cpssDxChVirtualTcamResize(vTcamMngId, vTcamCheckInfoArr[0].vTcamId,
                                   priorityArr[0]/* after priority[0] */, GT_FALSE, 100);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamResize failed\n");
            checkNumUsed(vTcamMngId, vTcamCheckInfoArr[0].vTcamId,expectedNumUsed);

            /* check that the HW hold proper values */
            checkHwContentAllRules(vTcamMngId, vTcamCheckInfoArr[0].vTcamId);

            /* make 'non-consecutive hole' */
            for(RuleId = 0; RuleId < 1000; RuleId += 3 )
            {
                if(RuleId >= 300 && RuleId < 400)
                {
                    /* we already removed this range */
                    RuleId = 400;
                }

                st = cpssDxChVirtualTcamRuleDelete(vTcamMngId, vTcamCheckInfoArr[0].vTcamId, RuleId);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamRuleDelete failed\n");

                expectedNumUsed--;
            }

            /* check that the HW hold proper values */
            checkHwContentAllRules(vTcamMngId, vTcamCheckInfoArr[0].vTcamId);

            /* expect fail because no empty 304 rules - there are only 300 free rule places */
            /* but CPSS API aligns it to granularity and must get more then 300.            */
            /*  granularity is (STD Rule on Legacy - 4, 30-bytes on SIP5 - 2)               */
            st = ut_cpssDxChVirtualTcamResize(vTcamMngId, vTcamCheckInfoArr[0].vTcamId,
                                   priorityArr[0]/* after priority[0] */, GT_FALSE, 304);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PARAM, st, "ut_cpssDxChVirtualTcamResize failed\n");
            checkNumUsed(vTcamMngId, vTcamCheckInfoArr[0].vTcamId,expectedNumUsed);

            /* expect GT_OK */
            st = ut_cpssDxChVirtualTcamResize(vTcamMngId, vTcamCheckInfoArr[0].vTcamId,
                                   priorityArr[0]/* after priority[0] */, GT_FALSE, 300);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamResize failed\n");

            checkNumUsed(vTcamMngId, vTcamCheckInfoArr[0].vTcamId,expectedNumUsed);

            /* make hole at the end of the last priority  */
            /* make ' consecutive hole' from ruleId 860 --> 51 rules */
            vTcamDeleteConsecutiveRules(vTcamMngId, vTcamCheckInfoArr[0].vTcamId,860,51);
            expectedNumUsed-=51;

            checkNumUsed(vTcamMngId, vTcamCheckInfoArr[0].vTcamId,expectedNumUsed);

            /* expect GT_OK */
            st = ut_cpssDxChVirtualTcamResize(vTcamMngId, vTcamCheckInfoArr[0].vTcamId,
                                   priorityArr[2]/* after priority[2] */, GT_FALSE, 50);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamResize failed\n");

            checkNumUsed(vTcamMngId, vTcamCheckInfoArr[0].vTcamId,expectedNumUsed);

            /* check that the HW hold proper values */
            checkHwContentAllRules(vTcamMngId, vTcamCheckInfoArr[0].vTcamId);

            /* make hole at the start of the last priority  */
            /* make ' consecutive hole' from start --> 49 rules */
            /* we have additional 1 free at the end from last time */
            vTcamDeleteConsecutiveRules(vTcamMngId, vTcamCheckInfoArr[0].vTcamId,CPSS_DXCH_VIRTUAL_TCAM_START_RULE_ID_CNS,49);
            expectedNumUsed-=49;

            checkNumUsed(vTcamMngId, vTcamCheckInfoArr[0].vTcamId,expectedNumUsed);

            /* 50 = (49 from current deletes) + (1 from previous case) */
            /* expect GT_OK */
            st = ut_cpssDxChVirtualTcamResize(vTcamMngId, vTcamCheckInfoArr[0].vTcamId,
                                   priorityArr[1]/* after priority[1] */, GT_FALSE, 50);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamResize failed\n");

            checkNumUsed(vTcamMngId, vTcamCheckInfoArr[0].vTcamId,expectedNumUsed);

            /* check that the HW hold proper values */
            readByPclCpssApi = GT_TRUE;
            checkHwContentAllRules(vTcamMngId, vTcamCheckInfoArr[0].vTcamId);
            readByPclCpssApi = GT_FALSE;


            /* make hole at the start of the first priority  */
            /* make ' consecutive hole' from start --> 20 rules */
            vTcamDeleteConsecutiveRules(vTcamMngId, vTcamCheckInfoArr[0].vTcamId,CPSS_DXCH_VIRTUAL_TCAM_START_RULE_ID_CNS,20);
            expectedNumUsed-=20;

            checkNumUsed(vTcamMngId, vTcamCheckInfoArr[0].vTcamId,expectedNumUsed);

            /* make hole at the end of the last priority  */
            /* make ' consecutive hole' at end --> 15 rules */
            vTcamDeleteConsecutiveRules(vTcamMngId, vTcamCheckInfoArr[0].vTcamId,DELETE_RULE_ID_FROM_END_OF_TABLE_CNS,15);
            expectedNumUsed-=15;

            checkNumUsed(vTcamMngId, vTcamCheckInfoArr[0].vTcamId,expectedNumUsed);

            /* 35 will pass  */
            st = ut_cpssDxChVirtualTcamResize(vTcamMngId, vTcamCheckInfoArr[0].vTcamId,
                                   0/*don't care*/, GT_FALSE, 35);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamResize failed\n");

            checkNumUsed(vTcamMngId, vTcamCheckInfoArr[0].vTcamId,expectedNumUsed);

            /* check that the HW hold proper values */
            checkHwContentAllRules(vTcamMngId, vTcamCheckInfoArr[0].vTcamId);

            /* make hole at the start of the range  */
            /* make ' consecutive hole' from start --> 11 rules */
            vTcamDeleteConsecutiveRules(vTcamMngId, vTcamCheckInfoArr[0].vTcamId,CPSS_DXCH_VIRTUAL_TCAM_START_RULE_ID_CNS,11);
            expectedNumUsed-=11;
            checkNumUsed(vTcamMngId, vTcamCheckInfoArr[0].vTcamId,expectedNumUsed);
            /* make hole at the end of the range  */
            /* make ' consecutive hole' at end --> 13 rules */
            vTcamDeleteConsecutiveRules(vTcamMngId, vTcamCheckInfoArr[0].vTcamId,DELETE_RULE_ID_FROM_END_OF_TABLE_CNS,13);
            expectedNumUsed-=13;
            checkNumUsed(vTcamMngId, vTcamCheckInfoArr[0].vTcamId,expectedNumUsed);
            /* make hole at the midle of the range */
            /* make ' consecutive hole' at end --> 55 rules */
            vTcamDeleteConsecutiveRules(vTcamMngId, vTcamCheckInfoArr[0].vTcamId,DELETE_RULE_ID_FROM_MIDDLE_OF_TABLE_CNS,55);
            expectedNumUsed-=55;
            checkNumUsed(vTcamMngId, vTcamCheckInfoArr[0].vTcamId,expectedNumUsed);

            /* 79 will pass  */
            st = ut_cpssDxChVirtualTcamResize(vTcamMngId, vTcamCheckInfoArr[0].vTcamId,
                                   0/*don't care*/, GT_FALSE, 79);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamResize failed\n");
            checkNumUsed(vTcamMngId, vTcamCheckInfoArr[0].vTcamId,expectedNumUsed);


            st = cpssDxChVirtualTcamUsageGet(vTcamMngId, vTcamCheckInfoArr[0].vTcamId, &vTcamUsage);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamUsageGet failed \n");

            for(RuleId = 5000; RuleId < (5000+vTcamUsage.rulesFree); RuleId++ )
            {
                UNIQUE_PCL_RULE_SET_MAC(&TcamRuleType,&pattern,RuleId);

                if(priorityMode == GT_TRUE)
                {
                    RuleAttributes.priority = (RuleId < 5400) ? priorityArr[0] :
                                              (RuleId < 5800) ? priorityArr[1] :
                                              priorityArr[2];
                }

                st = cpssDxChVirtualTcamRuleWrite(vTcamMngId, vTcamCheckInfoArr[0].vTcamId, RuleId, &RuleAttributes,
                &TcamRuleType, &RuleData);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamRuleWrite failed\n");

                expectedNumUsed++;
            }

            /* check that the HW hold proper values */
            checkHwContentAllRules(vTcamMngId, vTcamCheckInfoArr[0].vTcamId);

            checkNumUsed(vTcamMngId, vTcamCheckInfoArr[0].vTcamId,expectedNumUsed);
        }

        st = cpssDxChVirtualTcamUsageGet(vTcamMngId, vTcamCheckInfoArr[0].vTcamId, &vTcamUsage);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamUsageGet failed\n",
                                    vTcamUsage.rulesUsed, vTcamUsage.rulesFree);

        if(priorityMode == GT_TRUE)
        {
            goto cleanExit_lbl;
        }

        checkCpssDbIsOk(vTcamMngId,vTcamCheckInfoArr[0].vTcamId);

        for(RuleId = 0; RuleId < (1000 - 300); RuleId++ )
        {
            st = cpssDxChVirtualTcamRuleRead(vTcamMngId, vTcamCheckInfoArr[0].vTcamId, RuleId,
            &TcamRuleType, &RuleData_forRead);

            if(st != GT_OK)
            {
                writeErrorCounter++;
                tempSt = st;
            }

            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamRuleRead failed on ruleId[%d]",RuleId);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, RuleData_forRead.valid,
                "cpssDxChVirtualTcamRuleRead : expected valid entry RuleId[%d] \n",RuleId);

            /* get the unique value that each rule got */
            UNIQUE_PCL_RULE_GET_MAC(&TcamRuleType,RuleData_forRead.rule.pcl.patternPtr,tmpValue);

            if(RuleId < 150)
            {
                UTF_VERIFY_EQUAL0_STRING_MAC(RuleId, tmpValue, "wrong rule content");
            }
            else
            {
                UTF_VERIFY_EQUAL0_STRING_MAC((RuleId + 300/*the shift*/), tmpValue, "wrong rule content");
            }
        }

        checkCpssDbIsOk(vTcamMngId,vTcamCheckInfoArr[0].vTcamId);

        RuleId += 6;/* skip granularity issue */
        st = cpssDxChVirtualTcamRuleRead(vTcamMngId, vTcamCheckInfoArr[0].vTcamId, RuleId,
            &TcamRuleType, &RuleData_forRead);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamRuleRead should fail on ruleId[%d]",RuleId);

        checkCpssDbIsOk(vTcamMngId,vTcamCheckInfoArr[0].vTcamId);

        for(RuleId = 0; RuleId < (1000 - 300); RuleId++ )
        {
            st = cpssDxChVirtualTcamRuleDelete(vTcamMngId, vTcamCheckInfoArr[0].vTcamId, RuleId);

            if(st != GT_OK)
            {
                writeErrorCounter++;
                tempSt = st;
            }

            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamRuleDelete failed on ruleId[%d]",RuleId);
        }

        checkCpssDbIsOk(vTcamMngId,vTcamCheckInfoArr[0].vTcamId);

        /* 13. Create vTCAM(2) */
        vTcamCheckInfoArr[1].isExist = GT_TRUE;
        vTcamCheckInfoArr[1].vTcamId = 2;
        vTcamCheckInfoArr[1].vTcamInfo.clientGroup = 0; /* TTI and IPCL0 */
        vTcamCheckInfoArr[1].vTcamInfo.guaranteedNumOfRules = 500;
        vTcamCheckInfoArr[1].vTcamInfo.hitNumber = 0;
        vTcamCheckInfoArr[1].vTcamInfo.ruleSize =
        CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_60_B_E;
        vTcamCheckInfoArr[1].vTcamInfo.autoResize = GT_FALSE;
        vTcamCheckInfoArr[1].vTcamInfo.ruleAdditionMethod =
                CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E;

        st = ut_cpssDxChVirtualTcamCreate(vTcamMngId, vTcamCheckInfoArr[1].vTcamId, &vTcamCheckInfoArr[1].vTcamInfo);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamCreate failed for vTcamMngId/vTcamMngId = %d/%d, st = %d \n",
                                    vTcamMngId, vTcamCheckInfoArr[1].vTcamId, st);
        createdVtcam2 = GT_TRUE;

        /* 14. Get vTCAM (2) attribute*/
        st = cpssDxChVirtualTcamInfoGet(vTcamMngId, vTcamCheckInfoArr[1].vTcamId, &vTcamCheckInfoArr[1].vTcamInfo);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamInfoGet failed hitNumber = %d, clientGroup = %d \n",
                                     vTcamCheckInfoArr[1].vTcamInfo.hitNumber, vTcamCheckInfoArr[1].vTcamInfo.clientGroup );

        /* 15. Get vTCAM (2) usage */
        st = cpssDxChVirtualTcamUsageGet(vTcamMngId, vTcamCheckInfoArr[1].vTcamId, &vTcamUsage);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamUsageGet failed",
                                    vTcamUsage.rulesUsed, vTcamUsage.rulesFree);
        /*
            Add 5 PCL rules to
            vTCAM (2) with logical
            indexes: 0,4,7,8,10
        */
        TcamRuleType.ruleType = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;
        TcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E;
        RuleData.valid = GT_TRUE;
        /* set new rule */
        cpssOsMemSet(&mask, '\0', sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
        cpssOsMemSet(&pattern, '\0', sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
        cpssOsMemSet(&pclAction, '\0', sizeof(CPSS_DXCH_PCL_ACTION_STC));
        /* Set Mask, Pattern, pclAction */
        setRuleExtNotIpv6(&mask, &pattern, &pclAction);

        RuleData.rule.pcl.actionPtr     = &pclAction;
        RuleData.rule.pcl.maskPtr       = &mask;
        RuleData.rule.pcl.patternPtr    = &pattern;

        for(RuleId = 0; RuleId < 5; RuleId++ )
        {
            st = cpssDxChVirtualTcamRuleWrite(vTcamMngId, vTcamCheckInfoArr[1].vTcamId, RuleID_list[RuleId],
                                              &RuleAttributes,
                                              &TcamRuleType, &RuleData);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamRuleWrite failed on RuleID(%d) \n", RuleID_list[RuleId]);
        }

        /* 17. Get vTCAM (2) usage  */
        st = cpssDxChVirtualTcamUsageGet(vTcamMngId, vTcamCheckInfoArr[1].vTcamId, &vTcamUsage);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamUsageGet failed rulesUsed = %d / rulesFrees = %d \n",
                                     vTcamUsage.rulesUsed, vTcamUsage.rulesFree );

        /* 7.1.5 Get Next Rule ID in vTCAM.
                 Call: cpssDxChVirtualTcamNextRuleIdGet
                 Expected: NextRuleID = 4    */

        RuleId = 0;
        st = cpssDxChVirtualTcamNextRuleIdGet(vTcamMngId,vTcamCheckInfoArr[1].vTcamId,
                                         RuleId, &NextRuleId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamNextRuleIdGet failed. NextRuleId(%d) \n", NextRuleId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(4, NextRuleId, "cpssDxChVirtualTcamRuleWrite failed on RuleID(%d) \n", RuleID_list[RuleId]);


        for(RuleId = 0; RuleId < 5; RuleId++ )
        {
            st = cpssDxChVirtualTcamRuleDelete(vTcamMngId, vTcamCheckInfoArr[1].vTcamId, RuleID_list[RuleId]);

            if(st != GT_OK)
            {
                writeErrorCounter++;
                tempSt = st;
            }

            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamRuleDelete failed on ruleId[%d]",RuleId);
        }

cleanExit_lbl:

        vTcamDeleteConsecutiveRules(vTcamMngId,
            vTcamCheckInfoArr[0].vTcamId,
            CPSS_DXCH_VIRTUAL_TCAM_START_RULE_ID_CNS,
            DELETE_ALL_CNS);

        st = ut_cpssDxChVirtualTcamRemove(vTcamMngId, vTcamCheckInfoArr[0].vTcamId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamRemove failed to remove vTCAM(%d)\n", vTcamCheckInfoArr[0].vTcamId);

        if(createdVtcam2 == GT_TRUE)
        {
            st = ut_cpssDxChVirtualTcamRemove(vTcamMngId, vTcamCheckInfoArr[1].vTcamId);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamRemove failed to remove vTCAM(%d)\n", vTcamCheckInfoArr[1].vTcamId);
        }

        st = cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListRemove failed for vTcamMngId = %d\n",
                                     vTcamMngId);
        st = cpssDxChVirtualTcamManagerDelete(vTcamMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDelete failed for vTcamMngId = %d\n",
                                     vTcamMngId);
    }

    if (isTestSkipped)
    {
        SKIP_TEST_MAC;
    }
}

UTF_TEST_CASE_MAC(cpssDxChVirtualTcam_increaseVtcamSizebyResizing_priorityMode)
{
    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);
    priorityMode = GT_TRUE;
    UTF_TEST_CALL_MAC(cpssDxChVirtualTcam_increaseVtcamSizebyResizing);
    priorityMode = GT_FALSE;
}

UTF_TEST_CASE_MAC(cpssDxChVirtualTcam_decreaseVtcamSizeByResizing_priorityMode)
{
    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);
    priorityMode = GT_TRUE;
    UTF_TEST_CALL_MAC(cpssDxChVirtualTcam_decreaseVtcamSizeByResizing);
    priorityMode = GT_FALSE;
}

UTF_TEST_CASE_MAC(cpssDxChVirtualTcam_deleteVtcam)
{
    GT_STATUS                                  st = GT_OK;
    GT_U8                                      dev;
    GT_U32                                     numOfDevs = 0;
    GT_U32                                     notAppFamilyBmp = 0;
    GT_U32                                     utfFamilyBit = 0;
    GT_BOOL                                    isTestSkipped = GT_TRUE;
    GT_U32                                     vTcamMngId;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC            vTcamInfo;
    GT_U32                                     vTcamId;

    vTcamMngId                     = 1;
    vTcamId                        = 1;
    vTcamInfo.clientGroup          = 0; /* TTI and IPCL0 */
    vTcamInfo.hitNumber            = 0;
    vTcamInfo.ruleSize             = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E;
    vTcamInfo.autoResize           = GT_FALSE;
    vTcamInfo.guaranteedNumOfRules = 500;
    vTcamInfo.ruleAdditionMethod =
            CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E;

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* this feature is on eArch devices, xCat3 and Lion2 */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    notAppFamilyBmp &= (~UTF_XCAT3_E) & (~UTF_LION2_E);

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* 1. Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
              devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }
        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;

        /* Create vTCAM manager */
        st = cpssDxChVirtualTcamManagerCreate(vTcamMngId);
            if (st == GT_ALREADY_EXIST)
                st = GT_OK;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerCreate failed\n");

        st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        if (st != GT_OK)
        {
            cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
            st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        }
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListAdd failed\n");

        /* Create vTCAM(1) */
        st = ut_cpssDxChVirtualTcamCreate(vTcamMngId, vTcamId, &vTcamInfo);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamCreate failed for vTcamMngId = %d, st = %d\n", vTcamMngId, st);

        /* 18. Delete vTCAM(1) */
        st = ut_cpssDxChVirtualTcamRemove(vTcamMngId, vTcamId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamRemove failed\n");

        /*19. Get vTCAM (1) attribute.
         * Expected:  Should Failed  */
        st = cpssDxChVirtualTcamInfoGet(vTcamMngId, vTcamId, &vTcamInfo);
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC(GT_OK, st, "Expected:  Should Failed");

        st = cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListRemove failed for vTcamMngId = %d\n",
                                     vTcamMngId);
        st = cpssDxChVirtualTcamManagerDelete(vTcamMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDelete failed for vTcamMngId = %d\n",
                                     vTcamMngId);
    }

    if (isTestSkipped)
    {
        SKIP_TEST_MAC;
    }
}

#define NON_VALID_RULE_ID_CNS   0xFFFFFFFF

enum {
    PRIORITY_2000_INDEX_E,
    PRIORITY_10000_INDEX_E,
    PRIORITY_19000_INDEX_E,
    PRIORITY_20000_INDEX_E,

    /* must be last */
    PRIORITY_LAST_INDEX_E
};

static struct {
    GT_U32  firstRuleId;
    GT_U32  secondRuleId;
    GT_U32  lastRuleId;
}priority_Arr_Info[PRIORITY_LAST_INDEX_E];

/*
    RuleIds : there are can be rule in priority 2000,10000,19000,20000
    and we move around : 0xA002
*/
static void checkMovingRule0xA002BeteenPriorities(
    IN GT_U32   vTcamMngId,
    IN GT_U32   vTcamId,
    IN GT_BOOL tcamFull
)
{
    GT_STATUS   st;
    GT_U32  priority_Arr[PRIORITY_LAST_INDEX_E] = {2000,10000,19000,20000};
    struct {
        GT_U32          priority;
        CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_ENT position;
    }priorityUpdateArr[]=
    {
         { 5     , CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_FIRST_E }
        ,{  3000 , CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_FIRST_E }
        ,{ 15000 , CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_FIRST_E }
        ,{ 25000 , CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_FIRST_E }
        ,{ 10000 , CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_FIRST_E }
        ,{ 20000 , CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_FIRST_E }

        ,{ 5     , CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_LAST_E  }
        ,{  3000 , CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_LAST_E  }
        ,{ 15000 , CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_LAST_E  }
        ,{ 25000 , CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_LAST_E  }
        ,{ 10000 , CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_LAST_E  }
        ,{ 20000 , CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_LAST_E  }

        /* move from lart to first in the same priority */
        ,{ 5     , CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_LAST_E  }
        ,{ 5     , CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_FIRST_E }

        ,{ 15000 , CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_LAST_E  }
        ,{ 15000 , CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_FIRST_E }

        ,{ 25000 , CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_LAST_E  }
        ,{ 25000 , CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_FIRST_E }

        ,{ 10000 , CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_LAST_E  }
        ,{ 10000 , CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_FIRST_E }

        ,{ 20000 , CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_LAST_E  }
        ,{ 20000 , CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_FIRST_E }

        ,{  3000 , CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_LAST_E  }
        ,{  3000 , CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_FIRST_E }

        /* restore it to be at first of priority 20000 */
        ,{ 20000 , CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_FIRST_E }
    };

    GT_U32  iiMax = sizeof(priorityUpdateArr)/sizeof(priorityUpdateArr[0]);
    GT_U32  RuleId = 0xA002;
    GT_U32  actualNextRuleId,expectedNextRuleId,preOperationActualNextRuleId;
    GT_U32  ii,jj;
    GT_BOOL ruleCanKeepHwPosition; /* indication that the rule (when TCAM FULL)
            can keep it's HW index and operation can succeed */

    for(ii = 0 ; ii < iiMax ; ii++)
    {
        expectedNextRuleId = NON_VALID_RULE_ID_CNS;
        for(jj = 0 ; jj < PRIORITY_LAST_INDEX_E; jj++)
        {

            if(priorityUpdateArr[ii].position == CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_FIRST_E)
            {
                /* find the first ruleId of 'current priority' */
                if(priorityUpdateArr[ii].priority <= priority_Arr[jj])
                {
                    while(priority_Arr_Info[jj].firstRuleId == NON_VALID_RULE_ID_CNS)
                    {
                        jj++;
                        if(jj == PRIORITY_LAST_INDEX_E)
                        {
                            jj--;
                            break;
                        }
                    }

                    if(priority_Arr_Info[jj].firstRuleId != RuleId)
                    {
                        /* this is the RuleId before we changed the current RuleId
                           to be first in the priority */
                        expectedNextRuleId = priority_Arr_Info[jj].firstRuleId;
                    }
                    else
                    {
                        expectedNextRuleId = priority_Arr_Info[jj].secondRuleId;
                    }

                    break;
                }
            }
            else  /*position 'LAST'*/
            {
                /* find the first ruleId of 'next priority' */
                if(priorityUpdateArr[ii].priority < priority_Arr[jj])
                {
                    jj--;/* allow '0' to become '-1' */

                    while(priority_Arr_Info[jj+1].firstRuleId == NON_VALID_RULE_ID_CNS)
                    {
                        jj++;
                        if(jj == (PRIORITY_LAST_INDEX_E-1))
                        {
                            jj--;
                            break;
                        }
                    }

                    expectedNextRuleId = priority_Arr_Info[jj+1].firstRuleId;

                    if(priority_Arr_Info[jj+1].firstRuleId == RuleId)
                    {
                        expectedNextRuleId =
                            priority_Arr_Info[jj+1].secondRuleId != NON_VALID_RULE_ID_CNS ? priority_Arr_Info[jj+1].secondRuleId :
                            priority_Arr_Info[jj+1].lastRuleId   != NON_VALID_RULE_ID_CNS ? priority_Arr_Info[jj+1].lastRuleId   :
                                (jj+2) < (PRIORITY_LAST_INDEX_E) ? priority_Arr_Info[jj+1].firstRuleId : NON_VALID_RULE_ID_CNS;

                        if(expectedNextRuleId == NON_VALID_RULE_ID_CNS)
                        {
                            jj += 2;
                            do
                            {
                                expectedNextRuleId = priority_Arr_Info[jj+1].firstRuleId;
                                jj++;
                                if(jj == PRIORITY_LAST_INDEX_E)
                                {
                                    jj--;
                                    break;
                                }
                            } while(expectedNextRuleId == NON_VALID_RULE_ID_CNS);
                        }
                    }
                    break;
                }
                else
                if (priorityUpdateArr[ii].priority == priority_Arr[jj])
                {
                    if((jj+1) >= PRIORITY_LAST_INDEX_E)
                    {
                        /* no more */
                        expectedNextRuleId = NON_VALID_RULE_ID_CNS;
                        break;
                    }

                    expectedNextRuleId = priority_Arr_Info[jj+1].lastRuleId;
                    while(expectedNextRuleId == NON_VALID_RULE_ID_CNS)
                    {
                        expectedNextRuleId = priority_Arr_Info[jj+1].firstRuleId;
                        jj++;
                        if(jj == (PRIORITY_LAST_INDEX_E-1))
                        {
                            jj--;
                            break;
                        }
                    }
                }
            }
        }


        if(tcamFull == GT_TRUE)
        {
            ruleCanKeepHwPosition = GT_FALSE;

            if(priorityUpdateArr[ii].position == CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_FIRST_E)
            {
                if((jj < PRIORITY_LAST_INDEX_E) && priority_Arr_Info[jj].firstRuleId == RuleId)
                {
                    /* the rule already first in the needed priority */
                    ruleCanKeepHwPosition = GT_TRUE;
                }
                else
                if((jj > 0) && priority_Arr_Info[jj-1].lastRuleId == RuleId)
                {
                    /* the rule changed from last in previous priority to first in current.
                       so no HW movements needed , so operation can succeed.
                     */
                    ruleCanKeepHwPosition = GT_TRUE;
                }
            }
            else
            {
                if((jj < PRIORITY_LAST_INDEX_E) && priority_Arr_Info[jj].lastRuleId == RuleId)
                {
                    /* the rule already last in the needed priority */
                    ruleCanKeepHwPosition = GT_TRUE;
                }
                else
                if((jj < (PRIORITY_LAST_INDEX_E-1)) && priority_Arr_Info[jj+1].firstRuleId == RuleId)
                {
                    /* the rule changed from first in next priority to last in current.
                       so no HW movements needed , so operation can succeed.
                     */
                    ruleCanKeepHwPosition = GT_TRUE;
                }
            }
        }
        else
        {
            /* not relevant */
            ruleCanKeepHwPosition = GT_FALSE;
        }

        /* save the 'pre operation' next of the ruleId */
        st = cpssDxChVirtualTcamNextRuleIdGet(vTcamMngId, vTcamId, RuleId, &preOperationActualNextRuleId);
        if(st != GT_OK)
        {
            preOperationActualNextRuleId = NON_VALID_RULE_ID_CNS;
        }

        if(ii == 5000)
        {
            cpssOsPrintf("======= before update ii[%d] ======= ",
                RuleId);
            DEBUG_PRINT(vTcamMngId,vTcamId);
            checkCpssDbIsOk(vTcamMngId,vTcamId);
        }

        /* reposition the rule at the new priority */
        st = cpssDxChVirtualTcamRulePriorityUpdate(vTcamMngId, vTcamId,
                                                   RuleId,
                                                   priorityUpdateArr[ii].priority,
                                                   priorityUpdateArr[ii].position);
        if(ii == 5000)
        {
            cpssOsPrintf("======= after update ii[%d] ======= ",
                RuleId);
            DEBUG_PRINT(vTcamMngId,vTcamId);
            checkCpssDbIsOk(vTcamMngId,vTcamId);
        }

        if(tcamFull == GT_FALSE)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,
                                        "cpssDxChVirtualTcamRulePriorityUpdate failed\n");
        }
        else
        {
            if(ruleCanKeepHwPosition == GT_TRUE)
            {
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,
                                            "cpssDxChVirtualTcamRulePriorityUpdate table is full but no HW movements needed !!! \n");
            }
            else
            {
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_FULL, st,
                                            "cpssDxChVirtualTcamRulePriorityUpdate should not support 'tcam full' when HW moves needed \n");

                /* we expect the next ruleId to not be changed ! */
                expectedNextRuleId = preOperationActualNextRuleId;
            }
        }

        checkCpssDbIsOk(vTcamMngId,vTcamId);


        st = cpssDxChVirtualTcamNextRuleIdGet(vTcamMngId, vTcamId, RuleId, &actualNextRuleId);
        if(expectedNextRuleId != NON_VALID_RULE_ID_CNS)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,
                                        "cpssDxChVirtualTcamNextRuleIdGet failed\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(expectedNextRuleId, actualNextRuleId,
                                        "cpssDxChVirtualTcamNextRuleIdGet: not correct NextRuleId \n");
        }
        else
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_NO_MORE, st,
                                        "cpssDxChVirtualTcamNextRuleIdGet expected to 'no more' after RuleId[%d] in priority[%d] \n",
                                        RuleId,
                                        priorityUpdateArr[ii].priority);
        }
    }
}


/*
 * 7.2 Use-Case #2
 * Create dynamic vTCAMs with key size 30 with <autoResize = TRUE>
 */
UTF_TEST_CASE_MAC(cpssDxChVirtualTcam_addRulesWithDifferentPriority)
{
    GT_STATUS                                  st   = GT_OK;
    GT_U8                                      dev;
    GT_U32                                     numOfDevs = 0;
    GT_U32                                     notAppFamilyBmp = 0;
    GT_U32                                     utfFamilyBit = 0;
    GT_BOOL                                    isTestSkipped = GT_TRUE;
    GT_U32                                     vTcamMngId;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC            vTcamInfo;
    GT_U32                                     vTcamId;
    CPSS_DXCH_VIRTUAL_TCAM_USAGE_STC           vTcamUsage;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ID             RuleId;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ID             NextRuleId;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC RuleAttributes;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC       TcamRuleType;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC       RuleData;
    CPSS_DXCH_PCL_ACTION_STC                   pclAction;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              mask;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              pattern;
    GT_U32                                     priorityIndex;
    GT_U32                                     totalRulesAllowed = 0;
    GT_U32                                     priorityGet;

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* this feature is on eArch devices, xCat3 and Lion2 */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    notAppFamilyBmp &= (~UTF_XCAT3_E) & (~UTF_LION2_E);

    TcamRuleType.ruleType            = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;
    TcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* 1. Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
              devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }
        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;

        /* set invalid values */
        for(priorityIndex = 0 ; priorityIndex < PRIORITY_LAST_INDEX_E; priorityIndex++)
        {
            priority_Arr_Info[priorityIndex].firstRuleId   = NON_VALID_RULE_ID_CNS;
            priority_Arr_Info[priorityIndex].secondRuleId  = NON_VALID_RULE_ID_CNS;
            priority_Arr_Info[priorityIndex].lastRuleId    = NON_VALID_RULE_ID_CNS;
        }

        vTcamMngId                     = 1;
        vTcamId                        = 1;
        vTcamInfo.clientGroup          = 0; /* TTI and IPCL0 */
        vTcamInfo.hitNumber            = 0;
        vTcamInfo.ruleSize             = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E;
        vTcamInfo.autoResize           = GT_TRUE;
        vTcamInfo.guaranteedNumOfRules = 0;
        vTcamInfo.ruleAdditionMethod =
                CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E;

        /* Create vTCAM manager */
        st = cpssDxChVirtualTcamManagerCreate(vTcamMngId);
            if (st == GT_ALREADY_EXIST)
                st = GT_OK;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerCreate failed\n");

        st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        if (st != GT_OK)
        {
            cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
            st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        }
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListAdd failed\n");

        /* 1. Create vTCAM(1) */
        st = ut_cpssDxChVirtualTcamCreate(vTcamMngId, vTcamId, &vTcamInfo);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamCreate failed for vTcamMngId = %d, st = %d\n", vTcamMngId, st);

        if(GT_OK == st)
        {
            st = cpssDxChVirtualTcamInfoGet(vTcamMngId, vTcamId, &vTcamInfo);

            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamInfoGet failed hitNumber = %d, clientGroup = %d\n",
                                         vTcamInfo.hitNumber, vTcamInfo.clientGroup );
            UTF_VERIFY_EQUAL0_STRING_MAC(0, vTcamInfo.hitNumber,
                                         "not correct hit number");
            UTF_VERIFY_EQUAL0_STRING_MAC(0, vTcamInfo.clientGroup,
                                         "not correct client Group");

            /* 3. Get vTCAM (1) usage. Call cpssDxChVirtualTcamUsageGet
               Expected: GT_OK and the same params.     */

            st = cpssDxChVirtualTcamUsageGet(vTcamMngId, vTcamId, &vTcamUsage);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamUsageGet failed rulesUsed = %d / rulesFrees = %d\n",
                                        vTcamUsage.rulesUsed, vTcamUsage.rulesFree );
            UTF_VERIFY_EQUAL1_STRING_MAC(0, vTcamUsage.rulesUsed,
                                        "not correct rulesUsed(%d), expected '0'\n",
                                         vTcamUsage.rulesUsed);
            UTF_VERIFY_EQUAL1_STRING_MAC(0, vTcamUsage.rulesFree,
                                        "not correct rulesFree(%d), expected '0'\n",
                                         vTcamUsage.rulesFree);

            /* 4. Add PCL rule to vTCAM (1) with priority 10000 */
            /* Add rule #0 */
            RuleAttributes.priority          = 10000;
            TcamRuleType.ruleType            = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;
            TcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
            RuleId                           = 0xA000;
            RuleData.valid = GT_TRUE;
            /* set new rule */
            cpssOsMemSet(&mask, '\0', sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
            cpssOsMemSet(&pattern, '\0', sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
            cpssOsMemSet(&pclAction, '\0', sizeof(CPSS_DXCH_PCL_ACTION_STC));
            /* Set Mask */
            setRuleStdNotIp(&mask, &pattern, &pclAction);

            RuleData.rule.pcl.actionPtr     = &pclAction;
            RuleData.rule.pcl.maskPtr       = &mask;
            RuleData.rule.pcl.patternPtr    = &pattern;

            UNIQUE_PCL_RULE_SET_MAC(&TcamRuleType,&pattern,RuleId);
            st = cpssDxChVirtualTcamRuleWrite(vTcamMngId, vTcamId, RuleId,
                                             &RuleAttributes, &TcamRuleType,
                                             &RuleData);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,
                                        "cpssDxChVirtualTcamRuleWrite failed\n");


            /*5. Get vTCAM (1) usage */
            st = cpssDxChVirtualTcamUsageGet(vTcamMngId, vTcamId, &vTcamUsage);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamUsageGet failed rulesUsed = %d / rulesFrees = %d\n",
                                        vTcamUsage.rulesUsed, vTcamUsage.rulesFree );
            UTF_VERIFY_EQUAL1_STRING_MAC(1, vTcamUsage.rulesUsed,
                                        "not correct rulesUsed(%d), expected '0'\n",
                                         vTcamUsage.rulesUsed);


            /* 6. Add PCL rule to vTCAM(1) with priority 20000
                  with existing rule-id */
            RuleId                  = 0xA000;
            RuleAttributes.priority = 20000;
            UNIQUE_PCL_RULE_SET_MAC(&TcamRuleType,&pattern,RuleId);
            st = cpssDxChVirtualTcamRuleWrite(vTcamMngId, vTcamId, RuleId,
                                             &RuleAttributes, &TcamRuleType,
                                             &RuleData);
            /*Should Failed. Rule ID already exists */
            UTF_VERIFY_NOT_EQUAL0_STRING_MAC(GT_OK, st,
                                        "cpssDxChVirtualTcamRuleWrite failed. Rule ID already exists\n");

            /* 7. Add PCL rule to vTCAM(1) with priority 20000
                  with new rule-id */
            RuleId                  = 0xA001;
            RuleAttributes.priority = 20000;
            UNIQUE_PCL_RULE_SET_MAC(&TcamRuleType,&pattern,RuleId);
            st = cpssDxChVirtualTcamRuleWrite(vTcamMngId, vTcamId, RuleId,
                                             &RuleAttributes, &TcamRuleType,
                                             &RuleData);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,
                                        "cpssDxChVirtualTcamRuleWrite failed\n");

            st = cpssDxChVirtualTcamRulePriorityGet(
                vTcamMngId, vTcamId, RuleId, &priorityGet);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, st, "cpssDxChVirtualTcamRulePriorityGet failed.\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(
                RuleAttributes.priority, priorityGet, "diffrent priority retrieved.\n");


            /* 8. Get vTCAM (1) usage. Call cpssDxChVirtualTcamUsageGet
               Expected: GT_OK and the same params.     */
            st = cpssDxChVirtualTcamUsageGet(vTcamMngId, vTcamId, &vTcamUsage);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamUsageGet failed rulesUsed = %d / rulesFrees = %d\n",
                                        vTcamUsage.rulesUsed, vTcamUsage.rulesFree );
            UTF_VERIFY_EQUAL2_PARAM_MAC(2, vTcamUsage.rulesUsed,
                                        "not correct rulesUsed(%d), expected '0'\n",
                                         vTcamUsage.rulesUsed);

            /* 7.2.2 Add Rules with the Same Priority*/

            RuleId = 0xA002;
            RuleAttributes.priority = 20000;

            /* 7. Add PCL rule to vTCAM(1) with priority 20000
                  with new rule-id and Rule-ID 0xA001 is placed before rule-id 0xA002 */
            UNIQUE_PCL_RULE_SET_MAC(&TcamRuleType,&pattern,RuleId);
            st = cpssDxChVirtualTcamRuleWrite(vTcamMngId, vTcamId, RuleId,
                                             &RuleAttributes, &TcamRuleType,
                                             &RuleData);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,
                                        "cpssDxChVirtualTcamManagerCreate failed\n");

            st = cpssDxChVirtualTcamRulePriorityGet(
                vTcamMngId, vTcamId, RuleId, &priorityGet);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, st, "cpssDxChVirtualTcamRulePriorityGet failed.\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(
                RuleAttributes.priority, priorityGet, "diffrent priority retrieved.\n");

            /* 8. Get vTCAM (1) usage. Call cpssDxChVirtualTcamUsageGet
                  Expected: GT_OK and Used = 3*/
            st = cpssDxChVirtualTcamUsageGet(vTcamMngId, vTcamId, &vTcamUsage);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamUsageGet failed rulesUsed = %d / rulesFrees = %d\n",
                                        vTcamUsage.rulesUsed, vTcamUsage.rulesFree );
            UTF_VERIFY_EQUAL1_STRING_MAC(3, vTcamUsage.rulesUsed,
                                        "not correct rulesUsed(%d), expected '0'\n",
                                         vTcamUsage.rulesUsed);
        }
        checkHwContentAllRules(vTcamMngId,vTcamId);


        /*7.2.3 Update Priority of Existing Rule */

        RuleAttributes.priority = 19000;
        RuleId                  = 0xA002;
        /*DEBUG_PRINT(vTcamMngId,vTcamId);*/

        st = cpssDxChVirtualTcamRulePriorityUpdate(vTcamMngId, vTcamId,
                                                   RuleId, RuleAttributes.priority,
                                                   CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_LAST_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,
                                    "cpssDxChVirtualTcamRulePriorityUpdate failed\n");

        st = cpssDxChVirtualTcamRulePriorityGet(
            vTcamMngId, vTcamId, RuleId, &priorityGet);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, st, "cpssDxChVirtualTcamRulePriorityGet failed.\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(
            RuleAttributes.priority, priorityGet, "diffrent priority retrieved.\n");

        checkCpssDbIsOk(vTcamMngId,vTcamId);

        RuleAttributes.priority = 20000;
        st = cpssDxChVirtualTcamRulePriorityUpdate(vTcamMngId, vTcamId,
                                                   RuleId, RuleAttributes.priority,
                                                   CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_LAST_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,
                                    "cpssDxChVirtualTcamRulePriorityUpdate failed\n");

        st = cpssDxChVirtualTcamRulePriorityGet(
            vTcamMngId, vTcamId, RuleId, &priorityGet);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, st, "cpssDxChVirtualTcamRulePriorityGet failed.\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(
            RuleAttributes.priority, priorityGet, "diffrent priority retrieved.\n");

       checkCpssDbIsOk(vTcamMngId,vTcamId);

        st = cpssDxChVirtualTcamNextRuleIdGet(vTcamMngId, vTcamId, RuleId, &NextRuleId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, st,
                                    "cpssDxChVirtualTcamNextRuleIdGet failed\n");

        /* reposition the rule at the start of the priority */
        st = cpssDxChVirtualTcamRulePriorityUpdate(vTcamMngId, vTcamId,
                                                   RuleId, RuleAttributes.priority,
                                                   CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_FIRST_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,
                                    "cpssDxChVirtualTcamRulePriorityUpdate failed\n");

        st = cpssDxChVirtualTcamRulePriorityGet(
            vTcamMngId, vTcamId, RuleId, &priorityGet);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, st, "cpssDxChVirtualTcamRulePriorityGet failed.\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(
            RuleAttributes.priority, priorityGet, "diffrent priority retrieved.\n");

        checkCpssDbIsOk(vTcamMngId,vTcamId);

        st = cpssDxChVirtualTcamNextRuleIdGet(vTcamMngId, vTcamId, RuleId, &NextRuleId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,
                                    "cpssDxChVirtualTcamNextRuleIdGet failed\n");
        UTF_VERIFY_EQUAL1_STRING_MAC(0xA001, NextRuleId,
                                    "cpssDxChVirtualTcamNextRuleIdGet: not correct NextRuleId(%d)\n", NextRuleId);

        /* fill info for function checkMovingRule0xA002BeteenPriorities */
        priority_Arr_Info[PRIORITY_10000_INDEX_E].firstRuleId  = 0xA000;
        priority_Arr_Info[PRIORITY_20000_INDEX_E].lastRuleId   = 0xA000;

        priority_Arr_Info[PRIORITY_20000_INDEX_E].firstRuleId  = 0xA002;
        priority_Arr_Info[PRIORITY_20000_INDEX_E].secondRuleId = 0xA001;
        priority_Arr_Info[PRIORITY_20000_INDEX_E].lastRuleId   = 0xA001;
        checkMovingRule0xA002BeteenPriorities(vTcamMngId, vTcamId,GT_FALSE/* TCAM not FULL*/);

        /* 8. Get vTCAM (1) usage. Call cpssDxChVirtualTcamUsageGet
              Expected: GT_OK and Used = 3*/
        st = cpssDxChVirtualTcamUsageGet(vTcamMngId, vTcamId, &vTcamUsage);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamUsageGet failed rulesUsed = %d / rulesFrees = %d\n",
                                    vTcamUsage.rulesUsed, vTcamUsage.rulesFree );

        checkHwContentAllRules(vTcamMngId,vTcamId);

        /* fill the table to be 'full' */
        for(RuleId = 5000; RuleId < (5000+vTcamUsage.rulesFree); RuleId++ )
        {
            UNIQUE_PCL_RULE_SET_MAC(&TcamRuleType,&pattern,RuleId);

            RuleAttributes.priority = (RuleId % 3) == 0 ? 10000 :
                                      (RuleId % 3) == 1 ? 19000 :
                                      2000;
            priorityIndex =
                RuleAttributes.priority == 10000 ? PRIORITY_10000_INDEX_E :
                RuleAttributes.priority == 19000 ? PRIORITY_19000_INDEX_E :
                                                   PRIORITY_2000_INDEX_E;
            if(priority_Arr_Info[priorityIndex].firstRuleId == NON_VALID_RULE_ID_CNS)
            {
                /* update the first only one time */
                priority_Arr_Info[priorityIndex].firstRuleId = RuleId;
            }
            else
            if(priority_Arr_Info[priorityIndex].secondRuleId == NON_VALID_RULE_ID_CNS)
            {
                /* update the second only one time */
                priority_Arr_Info[priorityIndex].secondRuleId = RuleId;
            }

            /* update the 'last' Rule */
            priority_Arr_Info[priorityIndex].lastRuleId   = RuleId;


                /*cpssOsPrintf("======= RuleId[%d] ======= ",
                    RuleId);
                DEBUG_PRINT(vTcamMngId,vTcamId);
                checkCpssDbIsOk(vTcamMngId,vTcamId);*/

            st = cpssDxChVirtualTcamRuleWrite(vTcamMngId, vTcamId, RuleId, &RuleAttributes,
            &TcamRuleType, &RuleData);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamRuleWrite failed\n");

            st = cpssDxChVirtualTcamRulePriorityGet(
                vTcamMngId, vTcamId, RuleId, &priorityGet);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, st, "cpssDxChVirtualTcamRulePriorityGet failed.\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(
                RuleAttributes.priority, priorityGet, "diffrent priority retrieved.\n");

        }
        checkHwContentAllRules(vTcamMngId,vTcamId);

        /* check that table is full */
        checkNumFree(vTcamMngId,vTcamId,0);

        /* check that the operation success */
        /*7.2.3 Update Priority of Existing Rule */

        RuleAttributes.priority = 19000;
        RuleId                  = 0xA002;

        st = cpssDxChVirtualTcamRulePriorityUpdate(vTcamMngId, vTcamId,
                                                   RuleId, RuleAttributes.priority,
                                                   CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_LAST_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,
                                    "cpssDxChVirtualTcamRulePriorityUpdate failed\n");

        st = cpssDxChVirtualTcamRulePriorityGet(
            vTcamMngId, vTcamId, RuleId, &priorityGet);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, st, "cpssDxChVirtualTcamRulePriorityGet failed.\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(
            RuleAttributes.priority, priorityGet, "diffrent priority retrieved.\n");

        /* check that table is full */
        checkNumFree(vTcamMngId,vTcamId,0);

        RuleAttributes.priority = 20000;
        st = cpssDxChVirtualTcamRulePriorityUpdate(vTcamMngId, vTcamId,
                                                   RuleId, RuleAttributes.priority,
                                                   CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_LAST_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,
                                    "cpssDxChVirtualTcamRulePriorityUpdate failed\n");

        st = cpssDxChVirtualTcamRulePriorityGet(
            vTcamMngId, vTcamId, RuleId, &priorityGet);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, st, "cpssDxChVirtualTcamRulePriorityGet failed.\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(
            RuleAttributes.priority, priorityGet, "diffrent priority retrieved.\n");

        st = cpssDxChVirtualTcamNextRuleIdGet(vTcamMngId, vTcamId, RuleId, &NextRuleId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, st,
                                    "cpssDxChVirtualTcamNextRuleIdGet failed\n");

        /* reposition the rule at the start of the priority */
        st = cpssDxChVirtualTcamRulePriorityUpdate(vTcamMngId, vTcamId,
                                                   RuleId, RuleAttributes.priority,
                                                   CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_FIRST_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,
                                    "cpssDxChVirtualTcamRulePriorityUpdate failed\n");

        st = cpssDxChVirtualTcamRulePriorityGet(
            vTcamMngId, vTcamId, RuleId, &priorityGet);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, st, "cpssDxChVirtualTcamRulePriorityGet failed.\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(
            RuleAttributes.priority, priorityGet, "diffrent priority retrieved.\n");

        /* check that table is full */
        checkNumFree(vTcamMngId,vTcamId,0);

        checkHwContentAllRules(vTcamMngId,vTcamId);

        /* fill info for function checkMovingRule0xA002BeteenPriorities */
        priority_Arr_Info[PRIORITY_20000_INDEX_E].firstRuleId  = 0xA002;
        priority_Arr_Info[PRIORITY_20000_INDEX_E].secondRuleId = 0xA001;
        priority_Arr_Info[PRIORITY_20000_INDEX_E].lastRuleId   = 0xA001;
        checkMovingRule0xA002BeteenPriorities(vTcamMngId, vTcamId,GT_FALSE/* TCAM not FULL*/);


        /* create additional vtcam with HUGE size to use the rest of the TCAM manager */
        {
            internal_ut_cpssDxChVirtualTcamTotalRulesAllowedPerDevGet(
                        devListArr[0],
                        vTcamInfo.ruleSize,
                        &totalRulesAllowed);

            st = cpssDxChVirtualTcamUsageGet(vTcamMngId, vTcamId, &vTcamUsage);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamUsageGet failed rulesUsed = %d / rulesFrees = %d\n",
                                        vTcamUsage.rulesUsed, vTcamUsage.rulesFree );

            vTcamInfo.guaranteedNumOfRules =  totalRulesAllowed - vTcamUsage.rulesUsed;

            /* 1. Create vTCAM(2) */
            st = ut_cpssDxChVirtualTcamCreate(vTcamMngId, vTcamId+1, &vTcamInfo);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamCreate failed for vTcamMngId = %d, st = %d\n",
                vTcamMngId+1, st);

            /* 1. Create vTCAM(3) --- should FAIL --> FULL */
            vTcamInfo.guaranteedNumOfRules = 1;
            st = ut_cpssDxChVirtualTcamCreate(vTcamMngId, vTcamId+2, &vTcamInfo);
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamCreate expected to failed for vTcamMngId = %d, st = %d\n",
                vTcamMngId+2, st);
        }


        /* check that the operation success */
        /*7.2.3 Update Priority of Existing Rule */

        RuleAttributes.priority = 19000;
        RuleId                  = 0xA002;

        st = cpssDxChVirtualTcamRulePriorityUpdate(vTcamMngId, vTcamId,
                                                   RuleId, RuleAttributes.priority,
                                                   CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_LAST_E);
        /* NOTE: although TCAM is full this operation should succeed because the Rule was 'first rule' in priority 2000
           and we asked to make it 'last rule'  in priority 19000 .
           meaning no HW moves needed !!! */
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,
                                    "cpssDxChVirtualTcamRulePriorityUpdate should fail on 'table full' \n");
        st = cpssDxChVirtualTcamRulePriorityGet(
            vTcamMngId, vTcamId, RuleId, &priorityGet);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, st, "cpssDxChVirtualTcamRulePriorityGet failed.\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(
            RuleAttributes.priority, priorityGet, "diffrent priority retrieved.\n");

        checkCpssDbIsOk(vTcamMngId,vTcamId);
        /* check that table is full */
        checkNumFree(vTcamMngId,vTcamId,0);

        RuleAttributes.priority = 20000;
        st = cpssDxChVirtualTcamRulePriorityUpdate(vTcamMngId, vTcamId,
                                                   RuleId, RuleAttributes.priority,
                                                   CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_LAST_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_FULL, st,
                                    "cpssDxChVirtualTcamRulePriorityUpdate should fail on 'table full' \n");

        checkCpssDbIsOk(vTcamMngId,vTcamId);

        st = cpssDxChVirtualTcamNextRuleIdGet(vTcamMngId, vTcamId, RuleId, &NextRuleId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,
                                    "cpssDxChVirtualTcamNextRuleIdGet failed\n");

        /* reposition the rule at the start of the priority */
        st = cpssDxChVirtualTcamRulePriorityUpdate(vTcamMngId, vTcamId,
                                                   RuleId, RuleAttributes.priority,
                                                   CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_FIRST_E);
        /* NOTE: although TCAM is full this operation should succeed because the Rule 'last rule'  in priority 19000
           and we asked to make it 'first rule' in priority 2000.
           meaning no HW moves needed !!! */
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,
                                    "cpssDxChVirtualTcamRulePriorityUpdate should fail on 'table full' \n");
        st = cpssDxChVirtualTcamRulePriorityGet(
            vTcamMngId, vTcamId, RuleId, &priorityGet);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, st, "cpssDxChVirtualTcamRulePriorityGet failed.\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(
            RuleAttributes.priority, priorityGet, "diffrent priority retrieved.\n");

        checkCpssDbIsOk(vTcamMngId,vTcamId);

        /* check that table is full */
        checkNumFree(vTcamMngId,vTcamId,0);

        checkHwContentAllRules(vTcamMngId,vTcamId);


        checkMovingRule0xA002BeteenPriorities(vTcamMngId, vTcamId,GT_TRUE/* TCAM is FULL*/);

        {
            GT_U32  dummyRuleId = 5005;
            struct {
                GT_BOOL         needHwMove;
                GT_U32          priority;
                CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_ENT position;
            }priorityUpdateArr_forTableFull[]=
            {
                /* HW move needed for this case */
                 { GT_TRUE ,5     , CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_FIRST_E }
                ,{ GT_FALSE,6     , CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_LAST_E }
                ,{ GT_FALSE,2000 , CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_FIRST_E }
                ,{ GT_FALSE,6     , CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_LAST_E }
                /* HW move needed for this case */
                ,{ GT_TRUE , 2000 , CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_LAST_E }
                ,{ GT_FALSE, 3000 , CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_FIRST_E }
                ,{ GT_FALSE, 3000 , CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_LAST_E }
                ,{ GT_FALSE,10000 , CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_FIRST_E }
                ,{ GT_FALSE, 3000 , CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_LAST_E }
                /* HW move needed for this case */
                ,{ GT_TRUE ,12000 , CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_FIRST_E }
                ,{ GT_FALSE,15000 , CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_FIRST_E }
                ,{ GT_FALSE,16000 , CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_LAST_E }
                ,{ GT_FALSE,17500 , CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_FIRST_E }
                ,{ GT_FALSE,12000 , CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_LAST_E }
                ,{ GT_FALSE,19000 , CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_FIRST_E }
                /* HW move needed for this case */
                ,{ GT_TRUE ,20000 , CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_LAST_E }
                ,{ GT_FALSE,20001 , CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_FIRST_E }
                ,{ GT_FALSE,20000 , CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_LAST_E }
                ,{ GT_FALSE,25000 , CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_FIRST_E }

            };
            GT_U32  iiMax = sizeof(priorityUpdateArr_forTableFull)/sizeof(priorityUpdateArr_forTableFull[0]);
            GT_U32  ii;

            for(ii = 0 ; ii < iiMax; ii ++)
            {
                if(priorityUpdateArr_forTableFull[ii].needHwMove == GT_TRUE)
                {
                    /* delete dummyRuleId to allow move the RuleId to needed position */
                    st = cpssDxChVirtualTcamRuleDelete(
                        vTcamMngId, vTcamId, dummyRuleId);
                    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamRuleDelete failed\n");
                }

                /*cpssOsPrintf("======= before update ii[%d] ======= ",
                    ii);
                DEBUG_PRINT(vTcamMngId,vTcamId);
                checkCpssDbIsOk(vTcamMngId,vTcamId);*/

                /* move RuleId to position that allows it to keep HW on some cases */
                st = cpssDxChVirtualTcamRulePriorityUpdate(vTcamMngId, vTcamId,
                                                           RuleId,
                                                           priorityUpdateArr_forTableFull[ii].priority,
                                                           priorityUpdateArr_forTableFull[ii].position);
                if(priorityUpdateArr_forTableFull[ii].needHwMove == GT_TRUE)
                {
                    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,
                                                "cpssDxChVirtualTcamRulePriorityUpdate failed");
                }
                else
                {
                    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,
                                                "cpssDxChVirtualTcamRulePriorityUpdate table is full but no HW movements needed !!!");
                }

                /*cpssOsPrintf("======= after update ii[%d] ======= ",
                    ii);
                DEBUG_PRINT(vTcamMngId,vTcamId);
                checkCpssDbIsOk(vTcamMngId,vTcamId);*/

                /* restore the dummyRuleId back to it's priority */
                if(priorityUpdateArr_forTableFull[ii].needHwMove == GT_TRUE)
                {
                    RuleAttributes.priority = (dummyRuleId % 3) == 0 ? 10000 :
                                              (dummyRuleId % 3) == 1 ? 19000 :
                                              2000;
                    priorityIndex =
                        RuleAttributes.priority == 10000 ? PRIORITY_10000_INDEX_E :
                                                   19000 ? PRIORITY_19000_INDEX_E :
                                                           PRIORITY_2000_INDEX_E;
                    /* update the 'last' Rule */
                    priority_Arr_Info[priorityIndex].lastRuleId   = dummyRuleId;

                    /* restore the Rule back to it's priority */
                    st = cpssDxChVirtualTcamRuleWrite(vTcamMngId, vTcamId, dummyRuleId, &RuleAttributes,
                    &TcamRuleType, &RuleData);
                    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamRuleWrite failed\n");
                }

                checkCpssDbIsOk(vTcamMngId,vTcamId);
            }
        }


        vTcamDeleteConsecutiveRules(vTcamMngId,
            vTcamId,
            CPSS_DXCH_VIRTUAL_TCAM_START_RULE_ID_CNS,
            DELETE_ALL_CNS);

        st = ut_cpssDxChVirtualTcamRemove(vTcamMngId, vTcamId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamRemove failed\n");

        st = ut_cpssDxChVirtualTcamRemove(vTcamMngId, vTcamId+1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamRemove failed\n");


        st = cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListRemove failed for vTcamMngId = %d\n",
                                     vTcamMngId);
        st = cpssDxChVirtualTcamManagerDelete(vTcamMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDelete failed for vTcamMngId = %d\n",
                                     vTcamMngId);
    }

    if (isTestSkipped)
    {
        SKIP_TEST_MAC;
    }
}

/*7.3.1 Create vTCAMs based key size ordering (long key size to short)*/
UTF_TEST_CASE_MAC(cpssDxChVirtualTcam_createVtcamsLtoSkeySize)
{
    GT_STATUS                                  st   = GT_OK;
    GT_U8                                      dev;
    GT_U32                                     numOfDevs = 0;
    GT_U32                                     notAppFamilyBmp = 0;
    GT_U32                                     utfFamilyBit = 0;
    GT_BOOL                                    isTestSkipped = GT_TRUE;
    GT_U32                                     vTcamMngId;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC            vTcamInfo;
    GT_U32                                     vTcamId;
    CPSS_DXCH_VIRTUAL_TCAM_USAGE_STC           vTcamUsage;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ID             RuleId;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC RuleAttributes;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC       TcamRuleType;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC       RuleData;
    CPSS_DXCH_PCL_ACTION_STC                   pclAction;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              mask;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              pattern;
    GT_STATUS                                  tempSt = GT_OK;
    GT_U32                                     writeErrorCounter = 0;
    GT_U32                                     totalRulesAllowed = 0;

    vTcamMngId                     = 1;
    vTcamId                        = 1;
    vTcamInfo.clientGroup          = 0; /* TTI and IPCL0 */
    vTcamInfo.hitNumber            = 0;
    vTcamInfo.ruleSize             = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_60_B_E;
    vTcamInfo.autoResize           = GT_FALSE;
    vTcamInfo.guaranteedNumOfRules = 3072;
    vTcamInfo.ruleAdditionMethod =
            CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E;

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* 1. Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
              devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }
        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;

        internal_ut_cpssDxChVirtualTcamTotalRulesAllowedPerDevGet(
                   devListArr[0],
                   CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_60_B_E,
                   &totalRulesAllowed);

        vTcamInfo.guaranteedNumOfRules = totalRulesAllowed / 2;

        /* Create vTCAM manager */
        st = cpssDxChVirtualTcamManagerCreate(vTcamMngId);
            if (st == GT_ALREADY_EXIST)
                st = GT_OK;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerCreate failed\n");

        st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        if (st != GT_OK)
        {
            cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
            st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        }
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListAdd failed\n");

        /* 1. Create vTCAM(1) */
        st = ut_cpssDxChVirtualTcamCreate(vTcamMngId, vTcamId, &vTcamInfo);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamCreate failed for vTcamMngId = %d, st = %d\n", vTcamMngId, st);

        if(GT_OK == st)
        {
            st = cpssDxChVirtualTcamInfoGet(vTcamMngId, vTcamId, &vTcamInfo);

            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamInfoGet failed hitNumber = %d, clientGroup = %d\n",
                                         vTcamInfo.hitNumber, vTcamInfo.clientGroup );

           /*   Add additional 3072 rules to vTCAM (1) */
           /* Add rule #0 */
           RuleAttributes.priority     = 0; /* Not be used */
           TcamRuleType.ruleType = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;
           TcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_E;
           RuleData.valid = GT_TRUE;
           /* set new rule */
           cpssOsMemSet(&mask, '\0', sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
           cpssOsMemSet(&pattern, '\0', sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
           cpssOsMemSet(&pclAction, '\0', sizeof(CPSS_DXCH_PCL_ACTION_STC));
           /* Set Mask */
           setRuleIngrUdbOnly(&mask, &pattern, &pclAction);

           RuleData.rule.pcl.actionPtr     = &pclAction;
           RuleData.rule.pcl.maskPtr       = &mask;
           RuleData.rule.pcl.patternPtr    = &pattern;

           /* 0,1,2,3,...17 HW Bloks# occupied */
           writeErrorCounter = 0;
           tempSt = GT_OK;

           for(RuleId = 0; RuleId < vTcamInfo.guaranteedNumOfRules; RuleId++ )
           {
               st = cpssDxChVirtualTcamRuleWrite(vTcamMngId, vTcamId, RuleId, &RuleAttributes,
               &TcamRuleType, &RuleData);

               if(st != GT_OK)
               {
                   writeErrorCounter++;
                   tempSt = st;
               }
           }
           UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, tempSt,
                                       "cpssDxChVirtualTcamRuleWrite on RuleId(%d)failed for %d amount of Rules \n",RuleId, writeErrorCounter);

           st = cpssDxChVirtualTcamUsageGet(vTcamMngId, vTcamId, &vTcamUsage);
           UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamUsageGet failed rulesUsed = %d / rulesFrees = %d\n",
                                        vTcamUsage.rulesUsed, vTcamUsage.rulesFree );
           for(RuleId = 0; RuleId < vTcamInfo.guaranteedNumOfRules; RuleId++ )
           {
               st = cpssDxChVirtualTcamRuleDelete(vTcamMngId, vTcamId, RuleId);
               UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                           "cpssDxChVirtualTcamRuleDelete on RuleId(%d) failed \n",RuleId);
           }
        }

        vTcamId                        = 2;
        vTcamInfo.ruleSize             = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_50_B_E;
       /* vTcamInfo.guaranteedNumOfRules = totalRulesAllowed / 2;*/

        /* 1. Create vTCAM(2) */
        st = ut_cpssDxChVirtualTcamCreate(vTcamMngId, vTcamId, &vTcamInfo);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamCreate failed for vTcamMngId = %d, st = %d\n", vTcamMngId, st);

        /*   Add additional 3072 rules to vTCAM (1) */
        /* Add rule #0 */
        RuleAttributes.priority     = 0; /* Not be used */
        TcamRuleType.ruleType = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;
        TcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_50_E;
        RuleData.valid = GT_TRUE;
        /* set new rule */
        cpssOsMemSet(&mask, '\0', sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
        cpssOsMemSet(&pattern, '\0', sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
        cpssOsMemSet(&pclAction, '\0', sizeof(CPSS_DXCH_PCL_ACTION_STC));
        /* Set Mask */
        setRuleIngrUdbOnly(&mask, &pattern, &pclAction);

        RuleData.rule.pcl.actionPtr     = &pclAction;
        RuleData.rule.pcl.maskPtr       = &mask;
        RuleData.rule.pcl.patternPtr    = &pattern;

        /* 18,...,35 HW Bloks# occupied */
        writeErrorCounter = 0;
        tempSt = GT_OK;

        for(RuleId = 0; RuleId < vTcamInfo.guaranteedNumOfRules; RuleId++ )
        {
            st = cpssDxChVirtualTcamRuleWrite(vTcamMngId, vTcamId, RuleId, &RuleAttributes,
            &TcamRuleType, &RuleData);

            if(st != GT_OK)
            {
                writeErrorCounter++;
                tempSt = st;
            }
        }
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, tempSt,
                                 "cpssDxChVirtualTcamRuleWrite on RuleId(%d)failed for %d amount of Rules \n",RuleId, writeErrorCounter);

        st = cpssDxChVirtualTcamUsageGet(vTcamMngId, vTcamId, &vTcamUsage);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamUsageGet failed rulesUsed = %d / rulesFrees = %d\n",
                                  vTcamUsage.rulesUsed, vTcamUsage.rulesFree );

        for(RuleId = 0; RuleId < vTcamInfo.guaranteedNumOfRules; RuleId++ )
        {
            st = cpssDxChVirtualTcamRuleDelete(vTcamMngId, vTcamId, RuleId);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                        "cpssDxChVirtualTcamRuleDelete on RuleId(%d) failed \n",RuleId);
        }

        vTcamId                        = 3;
        vTcamInfo.ruleSize             = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_10_B_E;
       /* vTcamInfo.guaranteedNumOfRules = vTcamInfo.guaranteedNumOfRules;*/

        /* Create vTCAM manager */
        st = cpssDxChVirtualTcamManagerCreate(vTcamMngId);
         if (st == GT_ALREADY_EXIST)
             st = GT_OK;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerCreate failed\n");

        /* 1. Create vTCAM(3) */
        st = ut_cpssDxChVirtualTcamCreate(vTcamMngId, vTcamId, &vTcamInfo);
        if (st != GT_OK)
        {
            cpssDxChVirtualTcamUt_vTcamAllocStatusPrint(
                vTcamMngId, vTcamId);
        }
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamCreate failed for vTcamMngId = %d, st = %d\n", vTcamMngId, st);

        /*   Add additional 36864 rules to vTCAM (1) */
        /* Add rule #0 */
        RuleAttributes.priority     = 0; /* Not be used */
        TcamRuleType.ruleType = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;
        TcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E;
        RuleData.valid = GT_TRUE;
        /* set new rule */
        cpssOsMemSet(&mask, '\0', sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
        cpssOsMemSet(&pattern, '\0', sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
        cpssOsMemSet(&pclAction, '\0', sizeof(CPSS_DXCH_PCL_ACTION_STC));
        /* Set Mask */
        setRuleIngrUdbOnly(&mask, &pattern, &pclAction);

        RuleData.rule.pcl.actionPtr     = &pclAction;
        RuleData.rule.pcl.maskPtr       = &mask;
        RuleData.rule.pcl.patternPtr    = &pattern;

        /* 0,...,35 HW Bloks# occupied */
        writeErrorCounter = 0;
        tempSt = GT_OK;

        for(RuleId = 0; RuleId < vTcamInfo.guaranteedNumOfRules; RuleId++ )
        {
            st = cpssDxChVirtualTcamRuleWrite(vTcamMngId, vTcamId, RuleId, &RuleAttributes,
            &TcamRuleType, &RuleData);
            if(st != GT_OK)
            {
                writeErrorCounter++;
                tempSt = st;
            }
        }
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, tempSt,
                                 "cpssDxChVirtualTcamRuleWrite on RuleId(%d)failed for %d amount of Rules \n",RuleId, writeErrorCounter);

        st = cpssDxChVirtualTcamUsageGet(vTcamMngId, vTcamId, &vTcamUsage);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamUsageGet failed rulesUsed = %d / rulesFrees = %d\n",
                                  vTcamUsage.rulesUsed, vTcamUsage.rulesFree );

        for(RuleId = 0; RuleId < vTcamInfo.guaranteedNumOfRules; RuleId++ )
        {
            st = cpssDxChVirtualTcamRuleDelete(vTcamMngId, vTcamId, RuleId);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                        "cpssDxChVirtualTcamRuleDelete on RuleId(%d) failed \n",RuleId);
        }

        for(vTcamId = 1; vTcamId < 4; vTcamId++)
        {
            st = ut_cpssDxChVirtualTcamRemove(vTcamMngId, vTcamId);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamRemove failed for vTcamId = %d \n", vTcamId);
        }

        st = cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListRemove failed for vTcamMngId = %d\n",
                                     vTcamMngId);
        st = cpssDxChVirtualTcamManagerDelete(vTcamMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDelete failed for vTcamMngId = %d\n",
                                     vTcamMngId);

    }

    if (isTestSkipped)
    {
        SKIP_TEST_MAC;
    }
}

/*7.3.2 Create vTCAMs regardless key size ordering*/
UTF_TEST_CASE_MAC(cpssDxChVirtualTcam_createVtcamsRegardlesskeySizeOrdering)
{
    GT_STATUS                                  st   = GT_OK;
    GT_U8                                      dev;
    GT_U32                                     numOfDevs = 0;
    GT_U32                                     notAppFamilyBmp = 0;
    GT_U32                                     utfFamilyBit = 0;
    GT_BOOL                                    isTestSkipped = GT_TRUE;
    GT_U32                                     vTcamMngId;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC            vTcamInfo;
    GT_U32                                     vTcamId;
    CPSS_DXCH_VIRTUAL_TCAM_USAGE_STC           vTcamUsage;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ID             RuleId;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC RuleAttributes;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC       TcamRuleType;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC       RuleData;
    CPSS_DXCH_PCL_ACTION_STC                   pclAction;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              mask;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              pattern;
    GT_STATUS                                  tempSt = GT_OK;
    GT_U32                                     writeErrorCounter = 0;
    GT_U32                                     totalRulesAllowed = 0;

    vTcamMngId                     = 1;
    vTcamId                        = 3;
    vTcamInfo.clientGroup          = 0; /* TTI and IPCL0 */
    vTcamInfo.hitNumber            = 0;
    vTcamInfo.ruleSize             = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_10_B_E;
    vTcamInfo.autoResize           = GT_FALSE;
    vTcamInfo.guaranteedNumOfRules = 3072;
    vTcamInfo.ruleAdditionMethod =
            CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E;

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* 1. Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
              devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }
        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;

        internal_ut_cpssDxChVirtualTcamTotalRulesAllowedPerDevGet(
                    devListArr[0],
                    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_60_B_E,
                    &totalRulesAllowed);

        vTcamInfo.guaranteedNumOfRules = totalRulesAllowed / 2;

        /* Create vTCAM manager */
        st = cpssDxChVirtualTcamManagerCreate(vTcamMngId);
            if (st == GT_ALREADY_EXIST)
                st = GT_OK;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerCreate failed\n");

        st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        if (st != GT_OK)
        {
            cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
            st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        }
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListAdd failed\n");

        /* 1. Create vTCAM(3) */
        st = ut_cpssDxChVirtualTcamCreate(vTcamMngId, vTcamId, &vTcamInfo);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamCreate failed for vTcamMngId = %d, st = %d\n", vTcamMngId, st);

        if(GT_OK == st)
        {
            st = cpssDxChVirtualTcamInfoGet(vTcamMngId, vTcamId, &vTcamInfo);

            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamInfoGet failed hitNumber = %d, clientGroup = %d\n",
                                         vTcamInfo.hitNumber, vTcamInfo.clientGroup );

            /* 3. Get vTCAM (3) usage. Call cpssDxChVirtualTcamUsageGet
               Expected: GT_OK and the same params.     */

           st = cpssDxChVirtualTcamUsageGet(vTcamMngId, vTcamId, &vTcamUsage);
           UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamUsageGet failed rulesUsed = %d / rulesFrees = %d\n",
                                        vTcamUsage.rulesUsed, vTcamUsage.rulesFree );

           /*   Add additional 36864 rules to vTCAM (3) */
           /* Add rule #0 */
           RuleAttributes.priority     = 0; /* Not be used */
           TcamRuleType.ruleType = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;
           TcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E;
           RuleData.valid = GT_TRUE;
           /* set new rule */
           cpssOsMemSet(&mask, '\0', sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
           cpssOsMemSet(&pattern, '\0', sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
           cpssOsMemSet(&pclAction, '\0', sizeof(CPSS_DXCH_PCL_ACTION_STC));
           /* Set Mask */
           setRuleIngrUdbOnly(&mask, &pattern, &pclAction);

           RuleData.rule.pcl.actionPtr     = &pclAction;
           RuleData.rule.pcl.maskPtr       = &mask;
           RuleData.rule.pcl.patternPtr    = &pattern;

           /* 0,...,35 HW Bloks# occupied */
           writeErrorCounter = 0;
           tempSt = GT_OK;

           for(RuleId = 0; RuleId < vTcamInfo.guaranteedNumOfRules; RuleId++ )
           {
               st = cpssDxChVirtualTcamRuleWrite(vTcamMngId, vTcamId, RuleId, &RuleAttributes,
               &TcamRuleType, &RuleData);

               if(st != GT_OK)
               {
                   writeErrorCounter++;
                   tempSt = st;
               }
           }
           UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, tempSt,
                                       "cpssDxChVirtualTcamRuleWrite on RuleId(%d)failed for %d amount of Rules \n",RuleId, writeErrorCounter);

           st = cpssDxChVirtualTcamUsageGet(vTcamMngId, vTcamId, &vTcamUsage);
           UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamUsageGet failed rulesUsed = %d / rulesFrees = %d\n",
                                        vTcamUsage.rulesUsed, vTcamUsage.rulesFree );

           for(RuleId = 0; RuleId < vTcamInfo.guaranteedNumOfRules; RuleId++ )
           {
               st = cpssDxChVirtualTcamRuleDelete(vTcamMngId, vTcamId, RuleId);
               UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                           "cpssDxChVirtualTcamRuleDelete on RuleId(%d) failed \n",RuleId);
           }
        }

        vTcamId                        = 1;
        vTcamInfo.ruleSize             = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_60_B_E;

        /* Create vTCAM manager */
        st = cpssDxChVirtualTcamManagerCreate(vTcamMngId);
            if (st == GT_ALREADY_EXIST)
                st = GT_OK;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerCreate failed\n");

        /* 1. Create vTCAM(1) */
        st = ut_cpssDxChVirtualTcamCreate(vTcamMngId, vTcamId, &vTcamInfo);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamCreate failed for vTcamMngId = %d, st = %d\n", vTcamMngId, st);

        /*   Add additional 3072 rules to vTCAM (1) */
        /* Add rule #0 */
        RuleAttributes.priority     = 0; /* Not be used */
        TcamRuleType.ruleType = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;
        TcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_E;
        RuleData.valid = GT_TRUE;
        /* set new rule */
        cpssOsMemSet(&mask, '\0', sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
        cpssOsMemSet(&pattern, '\0', sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
        cpssOsMemSet(&pclAction, '\0', sizeof(CPSS_DXCH_PCL_ACTION_STC));
        /* Set Mask */
        setRuleIngrUdbOnly(&mask, &pattern, &pclAction);

        RuleData.rule.pcl.actionPtr     = &pclAction;
        RuleData.rule.pcl.maskPtr       = &mask;
        RuleData.rule.pcl.patternPtr    = &pattern;

        /* 0,1,2,3,...17 HW Bloks# occupied */
        writeErrorCounter = 0;
        tempSt = GT_OK;

        for(RuleId = 0; RuleId < vTcamInfo.guaranteedNumOfRules; RuleId++ )
        {
            st = cpssDxChVirtualTcamRuleWrite(vTcamMngId, vTcamId, RuleId, &RuleAttributes,
            &TcamRuleType, &RuleData);

            if(st != GT_OK)
            {
                writeErrorCounter++;
                tempSt = st;
            }
        }
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, tempSt,
                                    "cpssDxChVirtualTcamRuleWrite on RuleId(%d)failed for %d amount of Rules \n",RuleId, writeErrorCounter);

        st = cpssDxChVirtualTcamUsageGet(vTcamMngId, vTcamId, &vTcamUsage);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamUsageGet failed rulesUsed = %d / rulesFrees = %d\n",
                                     vTcamUsage.rulesUsed, vTcamUsage.rulesFree );

        for(RuleId = 0; RuleId < vTcamInfo.guaranteedNumOfRules; RuleId++ )
        {
            st = cpssDxChVirtualTcamRuleDelete(vTcamMngId, vTcamId, RuleId);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                        "cpssDxChVirtualTcamRuleDelete on RuleId(%d) failed \n",RuleId);
        }

        vTcamId                        = 2;
        vTcamInfo.ruleSize             = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_50_B_E;

        /* 1. Create vTCAM(2) */
        st = ut_cpssDxChVirtualTcamCreate(vTcamMngId, vTcamId, &vTcamInfo);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamCreate failed for vTcamMngId = %d, st = %d\n", vTcamMngId, st);

        st = ut_cpssDxChVirtualTcamRemove(vTcamMngId, 1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamRemove failed for vTcamId = %d \n", vTcamId);

    st = ut_cpssDxChVirtualTcamRemove(vTcamMngId, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamRemove failed for vTcamId = %d \n", vTcamId);

        st = ut_cpssDxChVirtualTcamRemove(vTcamMngId, 3);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamRemove failed for vTcamId = %d \n", vTcamId);

        st = cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListRemove failed for vTcamMngId = %d\n",
                                     vTcamMngId);
        st = cpssDxChVirtualTcamManagerDelete(vTcamMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDelete failed for vTcamMngId = %d\n",
                                     vTcamMngId);
    }

    if (isTestSkipped)
    {
        SKIP_TEST_MAC;
    }
}

/* 7.3.3 Create vTCAMs in Different Groups (Total Rules relevant for Bobcat2 case)
 *Group#, Hit#| vTCAM#| Key Size| Total Rules| HW Block #| Expected Result
 *0,0         |   1   |    30   |     256    |   0,1,2   |     Success
 *0,0         |   1   |    60   |     256    |0,1,2,3,4,5|     Success
 *1,0         |   1   |    30   |    1536    |6,7,..15   |     Success
 *1,0         |   1   |    60   |    2560    |16,...35   |     Success
 */
UTF_TEST_CASE_MAC(cpssDxChVirtualTcam_createVtcamsInDifferentGroups)
{
    GT_STATUS                                  st   = GT_OK;
    GT_U8                                      dev;
    GT_U32                                     numOfDevs = 0;
    GT_U32                                     notAppFamilyBmp = 0;
    GT_U32                                     utfFamilyBit = 0;
    GT_BOOL                                    isTestSkipped = GT_TRUE;
    GT_U32                                     vTcamMngId;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC            vTcamInfo;
    GT_U32                                     vTcamId;
    CPSS_DXCH_VIRTUAL_TCAM_USAGE_STC           vTcamUsage;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ID             RuleId;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC RuleAttributes;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC       TcamRuleType;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC       RuleData;
    CPSS_DXCH_PCL_ACTION_STC                   pclAction;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              mask;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              pattern;
    GT_STATUS                                  tempSt = GT_OK;
    GT_U32                                     writeErrorCounter = 0;
    GT_U32                                     totalRulesAllowed;
    GT_U32                                     divisor;

    vTcamMngId                     = 1;
    vTcamId                        = 1;
    vTcamInfo.clientGroup          = 0; /* TTI and IPCL0 */
    vTcamInfo.hitNumber            = 0;
    vTcamInfo.ruleSize             = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E;
    vTcamInfo.autoResize           = GT_FALSE;
    vTcamInfo.ruleAdditionMethod =
            CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E;

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* 1. Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
              devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }
        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;

        internal_ut_cpssDxChVirtualTcamTotalRulesAllowedPerDevGet(
                    devListArr[0],
                    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_10_B_E,
                    &totalRulesAllowed);

        divisor = 48;
        vTcamInfo.guaranteedNumOfRules = totalRulesAllowed / (vTcamInfo.ruleSize + 1) / divisor;

        /* Create vTCAM manager */
        st = cpssDxChVirtualTcamManagerCreate(vTcamMngId);
            if (st == GT_ALREADY_EXIST)
                st = GT_OK;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerCreate failed\n");

        st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        if (st != GT_OK)
        {
            cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
            st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        }
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListAdd failed\n");

        /* 1. Create vTCAM(1) */
        st = ut_cpssDxChVirtualTcamCreate(vTcamMngId, vTcamId, &vTcamInfo);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamCreate failed for vTcamMngId = %d, st = %d\n", vTcamMngId, st);

        if(GT_OK == st)
        {
            st = cpssDxChVirtualTcamInfoGet(vTcamMngId, vTcamId, &vTcamInfo);

            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamInfoGet failed hitNumber = %d, clientGroup = %d\n",
                                         vTcamInfo.hitNumber, vTcamInfo.clientGroup );

            /* 3. Get vTCAM (1) usage. Call cpssDxChVirtualTcamUsageGet
               Expected: GT_OK and the same params.     */

           st = cpssDxChVirtualTcamUsageGet(vTcamMngId, vTcamId, &vTcamUsage);
           UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamUsageGet failed rulesUsed = %d / rulesFrees = %d\n",
                                        vTcamUsage.rulesUsed, vTcamUsage.rulesFree );

           /*   Add additional 256 rules to vTCAM (1) */
           /* Add rule #0 */
           RuleAttributes.priority     = 0; /* Not be used */
           TcamRuleType.ruleType = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;
           TcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E;
           RuleData.valid = GT_TRUE;
           /* set new rule */
           cpssOsMemSet(&mask, '\0', sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
           cpssOsMemSet(&pattern, '\0', sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
           cpssOsMemSet(&pclAction, '\0', sizeof(CPSS_DXCH_PCL_ACTION_STC));
           /* Set Mask */
           setRuleIngrUdbOnly(&mask, &pattern, &pclAction);

           RuleData.rule.pcl.actionPtr     = &pclAction;
           RuleData.rule.pcl.maskPtr       = &mask;
           RuleData.rule.pcl.patternPtr    = &pattern;

           /* 0,1,2 HW Bloks# occupied */
           writeErrorCounter = 0;
           tempSt = GT_OK;

           for(RuleId = 0; RuleId < vTcamInfo.guaranteedNumOfRules; RuleId++ )
           {
               st = cpssDxChVirtualTcamRuleWrite(vTcamMngId, vTcamId, RuleId, &RuleAttributes,
               &TcamRuleType, &RuleData);

               if(st != GT_OK)
               {
                   writeErrorCounter++;
                   tempSt = st;
               }
           }
           UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, tempSt,
                                       "cpssDxChVirtualTcamRuleWrite on RuleId(%d)failed for %d amount of Rules \n",RuleId, writeErrorCounter);

           st = cpssDxChVirtualTcamUsageGet(vTcamMngId, vTcamId, &vTcamUsage);
           UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamUsageGet failed rulesUsed = %d / rulesFrees = %d\n",
                                        vTcamUsage.rulesUsed, vTcamUsage.rulesFree );

           for(RuleId = 0; RuleId < vTcamInfo.guaranteedNumOfRules; RuleId++ )
           {
               st = cpssDxChVirtualTcamRuleDelete(vTcamMngId, vTcamId, RuleId);
               UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                           "cpssDxChVirtualTcamRuleDelete on RuleId(%d) failed \n",RuleId);
           }
        }

        /* delete vTCAM for 1 case */
        st = ut_cpssDxChVirtualTcamRemove(vTcamMngId, vTcamId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamRemove failed for vTcamId = %d \n", vTcamId);

        divisor = 24;
        vTcamInfo.ruleSize             = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_60_B_E;
        vTcamInfo.guaranteedNumOfRules = totalRulesAllowed / (vTcamInfo.ruleSize + 1) / divisor;

        /* 1. Create vTCAM(1.0/0) for case 2 */
        st = ut_cpssDxChVirtualTcamCreate(vTcamMngId, vTcamId, &vTcamInfo);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamCreate failed for vTcamMngId = %d, st = %d\n", vTcamMngId, st);

        /*   Add additional 256 rules to vTCAM (1) */
        /* Add rule #0 */
        RuleAttributes.priority     = 0; /* Not be used */
        TcamRuleType.ruleType = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;
        TcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_E;
        RuleData.valid = GT_TRUE;
        /* set new rule */
        cpssOsMemSet(&mask, '\0', sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
        cpssOsMemSet(&pattern, '\0', sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
        cpssOsMemSet(&pclAction, '\0', sizeof(CPSS_DXCH_PCL_ACTION_STC));
        /* Set Mask */
        setRuleIngrUdbOnly(&mask, &pattern, &pclAction);

        RuleData.rule.pcl.actionPtr     = &pclAction;
        RuleData.rule.pcl.maskPtr       = &mask;
        RuleData.rule.pcl.patternPtr    = &pattern;

        /* 0,1,2,3,4,5 HW Bloks# occupied */
        writeErrorCounter = 0;
        tempSt = GT_OK;

        for(RuleId = 0; RuleId < vTcamInfo.guaranteedNumOfRules; RuleId++ )
        {
            st = cpssDxChVirtualTcamRuleWrite(vTcamMngId, vTcamId, RuleId, &RuleAttributes,
            &TcamRuleType, &RuleData);

            if(st != GT_OK)
            {
                writeErrorCounter++;
                tempSt = st;
            }
        }
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, tempSt,
                                    "cpssDxChVirtualTcamRuleWrite on RuleId(%d)failed for %d amount of Rules \n",RuleId, writeErrorCounter);

        st = cpssDxChVirtualTcamUsageGet(vTcamMngId, vTcamId, &vTcamUsage);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamUsageGet failed rulesUsed = %d / rulesFrees = %d\n",
                                     vTcamUsage.rulesUsed, vTcamUsage.rulesFree );

        for(RuleId = 0; RuleId < vTcamInfo.guaranteedNumOfRules; RuleId++ )
        {
            st = cpssDxChVirtualTcamRuleDelete(vTcamMngId, vTcamId, RuleId);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                        "cpssDxChVirtualTcamRuleDelete on RuleId(%d) failed \n",RuleId);
        }

        /* delete vTCAM for 2 case */
        st = ut_cpssDxChVirtualTcamRemove(vTcamMngId, vTcamId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamRemove failed for vTcamId = %d \n", vTcamId);

        vTcamInfo.ruleSize             = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E;
        vTcamInfo.clientGroup          = 1; /* TTI and IPCL0 */
        divisor = 8;
        vTcamInfo.guaranteedNumOfRules = totalRulesAllowed / (vTcamInfo.ruleSize + 1) / divisor;

        /*create vTCAM(1 1/0 ) for 3 case */
        st = ut_cpssDxChVirtualTcamCreate(vTcamMngId, vTcamId, &vTcamInfo);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamCreate failed for vTcamMngId = %d, st = %d\n", vTcamMngId, st);

        /*   Add additional 1536 rules to vTCAM (1) */
        RuleAttributes.priority     = 0; /* Not be used */
        TcamRuleType.ruleType = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;
        TcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E;
        RuleData.valid = GT_TRUE;
        /* set new rule */
        cpssOsMemSet(&mask, '\0', sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
        cpssOsMemSet(&pattern, '\0', sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
        cpssOsMemSet(&pclAction, '\0', sizeof(CPSS_DXCH_PCL_ACTION_STC));
        /* Set Mask */
        setRuleIngrUdbOnly(&mask, &pattern, &pclAction);

        RuleData.rule.pcl.actionPtr     = &pclAction;
        RuleData.rule.pcl.maskPtr       = &mask;
        RuleData.rule.pcl.patternPtr    = &pattern;

        /* 6,7,..15 HW Bloks# occupied */
        writeErrorCounter = 0;
        tempSt = GT_OK;

        for(RuleId = 0; RuleId < vTcamInfo.guaranteedNumOfRules; RuleId++ )
        {
            st = cpssDxChVirtualTcamRuleWrite(vTcamMngId, vTcamId, RuleId, &RuleAttributes,
            &TcamRuleType, &RuleData);

            if(st != GT_OK)
            {
                writeErrorCounter++;
                tempSt = st;
            }
        }
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, tempSt,
                                    "cpssDxChVirtualTcamRuleWrite on RuleId(%d)failed for %d amount of Rules \n",RuleId, writeErrorCounter);

        for(RuleId = 0; RuleId < vTcamInfo.guaranteedNumOfRules; RuleId++ )
        {
            st = cpssDxChVirtualTcamRuleDelete(vTcamMngId, vTcamId, RuleId);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                        "cpssDxChVirtualTcamRuleDelete on RuleId(%d) failed \n",RuleId);
        }

        /* delete vTCAM for 3 case */
        st = ut_cpssDxChVirtualTcamRemove(vTcamMngId, vTcamId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamRemove failed for vTcamId = %d \n", vTcamId);

        divisor = 2;
        vTcamInfo.ruleSize             = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_60_B_E;
        vTcamInfo.guaranteedNumOfRules = totalRulesAllowed / (vTcamInfo.ruleSize + 1) / divisor;

        /*create vTCAM(1 1/0 ) for 4 case */
        st = ut_cpssDxChVirtualTcamCreate(vTcamMngId, vTcamId, &vTcamInfo);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamCreate failed for vTcamMngId = %d, st = %d\n", vTcamMngId, st);

        /*   Add additional 2560 rules to vTCAM (1) */
        RuleAttributes.priority     = 0; /* Not be used */
        TcamRuleType.ruleType = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;
        TcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_E;
        RuleData.valid = GT_TRUE;
        /* set new rule */
        cpssOsMemSet(&mask, '\0', sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
        cpssOsMemSet(&pattern, '\0', sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
        cpssOsMemSet(&pclAction, '\0', sizeof(CPSS_DXCH_PCL_ACTION_STC));
        /* Set Mask */
        setRuleIngrUdbOnly(&mask, &pattern, &pclAction);

        RuleData.rule.pcl.actionPtr     = &pclAction;
        RuleData.rule.pcl.maskPtr       = &mask;
        RuleData.rule.pcl.patternPtr    = &pattern;

        /* 16,...35 HW Bloks# occupied */
        writeErrorCounter = 0;
        tempSt = GT_OK;

        for(RuleId = 0; RuleId < vTcamInfo.guaranteedNumOfRules; RuleId++ )
        {
            st = cpssDxChVirtualTcamRuleWrite(vTcamMngId, vTcamId, RuleId, &RuleAttributes,
            &TcamRuleType, &RuleData);

            if(st != GT_OK)
            {
                writeErrorCounter++;
                tempSt = st;
            }
        }
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, tempSt,
                                    "cpssDxChVirtualTcamRuleWrite on RuleId(%d)failed for %d amount of Rules \n",RuleId, writeErrorCounter);

        for(RuleId = 0; RuleId < vTcamInfo.guaranteedNumOfRules; RuleId++ )
        {
            st = cpssDxChVirtualTcamRuleDelete(vTcamMngId, vTcamId, RuleId);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                        "cpssDxChVirtualTcamRuleDelete on RuleId(%d) failed \n",RuleId);
        }

        /* delete vTCAM for 4 case */
        st = ut_cpssDxChVirtualTcamRemove(vTcamMngId, vTcamId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamRemove failed for vTcamId = %d \n", vTcamId);

        st = cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListRemove failed for vTcamMngId = %d\n",
                                     vTcamMngId);
        st = cpssDxChVirtualTcamManagerDelete(vTcamMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDelete failed for vTcamMngId = %d\n",
                                     vTcamMngId);
    }

    if (isTestSkipped)
    {
        SKIP_TEST_MAC;
    }
}

static GT_BOOL cpssDxChVirtualTcamUt_vTcamAllocStatusPrintEnable = GT_FALSE;

GT_VOID cpssDxChVirtualTcamUt_vTcamAllocStatusPrintEnableSet(GT_BOOL enable)
{
    cpssDxChVirtualTcamUt_vTcamAllocStatusPrintEnable = enable;
}

GT_VOID cpssDxChVirtualTcamUt_vTcamAllocStatusPrint(
    GT_U32 vTcamMngId, GT_U32 vTcamId)
{
    GT_STATUS st;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC vTcamInfo;
    CPSS_DXCH_VIRTUAL_TCAM_USAGE_STC usage;

    if (cpssDxChVirtualTcamUt_vTcamAllocStatusPrintEnable == GT_FALSE)
    {
        return;
    }
    /*
    prvCpssDxChVirtualTcamDbVTcamDumpByField(
        vTcamMngId, vTcamId, "segFreeLuTree");
    */
    prvCpssDxChVirtualTcamDbVTcamDumpFullTcamLayout(vTcamMngId);
    prvCpssDxChVirtualTcamDbVTcamDumpBlockFreeSpace(vTcamMngId, GT_TRUE);
    st = cpssDxChVirtualTcamInfoGet(
        vTcamMngId, vTcamId, &vTcamInfo);
    cpssOsPrintf("vTcamId %d \n", vTcamId);
    if (st == GT_OK)
    {
        cpssOsPrintf(
            "clientGroup %d ruleSize %d  guaranteedNumOfRules %d\n",
            vTcamInfo.clientGroup, (vTcamInfo.ruleSize + 1),
            vTcamInfo.guaranteedNumOfRules);
        prvCpssDxChVirtualTcamDbVTcamDumpByField(
            vTcamMngId, vTcamId, "segmentsTree");
        st = cpssDxChVirtualTcamUsageGet(
            vTcamMngId, vTcamId, &usage);
        if (st != GT_OK)
        {
            cpssOsPrintf(
                "cpssDxChVirtualTcamUsageGet failed\n");
        }
        else
        {
            cpssOsPrintf(
                "vTcamId %d, usage: Used %d Free %d ruleSize %d totalSpace %d\n",
                vTcamId, usage.rulesUsed, usage.rulesFree, ((vTcamInfo.ruleSize + 1) * 10),
                ((usage.rulesUsed + usage.rulesFree) * (vTcamInfo.ruleSize + 1)));
        }
        prvCpssDxChVirtualTcamDbVTcamDumpByField(
            vTcamMngId, vTcamId, "usedRulesBitmapArr");
    }
}

static VT_SEQUENCE bc2_sequence[] =
{
    {VT_CREATE,    5, CG_TTI,  RS_20, 1002, GT_OK},
    {VT_CREATE,    6, CG_TTI,  RS_30, 1000, GT_OK},
    {VT_COMPLETE,  7, CG_TTI,  RS_NONE,  0, GT_OK},
    {VT_CREATE,    8, CG_PCL,  RS_20, 1002, GT_OK},
    {VT_CREATE,    9, CG_PCL,  RS_40, 2000, GT_OK},
    {VT_CREATE,   10, CG_PCL,  RS_50, 1002, GT_OK},
    {VT_CREATE,   11, CG_PCL,  RS_10, 7002, GT_OK},
    {VT_CREATE,   12, CG_PCL,  RS_60,  500, GT_OK},
    {VT_CREATE,   13, CG_PCL,  RS_30,  100, GT_OK},
    {VT_CREATE,   14, CG_PCL,  RS_50,  814, GT_OK},
    {VT_CREATE,   15, CG_PCL,  RS_10,  814, GT_OK},
    {VT_COMPLETE, 16, CG_PCL,  RS_NONE,  0, GT_OK},
    {VT_CREATE,   50, CG_TTI,  RS_10,    1, GT_FULL},
    {VT_CREATE,   51, CG_PCL,  RS_10,    1, GT_FULL},
    {VT_REMOVE,    6, CG_NONE, RS_NONE,  0, GT_OK},
    {VT_REMOVE,    8, CG_NONE, RS_NONE,  0, GT_OK},
    {VT_REMOVE,   10, CG_NONE, RS_NONE,  0, GT_OK},
    {VT_REMOVE,   13, CG_NONE, RS_NONE,  0, GT_OK},
    {VT_REMOVE,   15, CG_NONE, RS_NONE,  0, GT_OK},
    {VT_CREATE,   40, CG_TTI,  RS_60,  500, GT_OK},
    {VT_CREATE,   31, CG_PCL,  RS_50,  900, GT_OK}, /*RS_40*/ /* no defrag */
    {VT_CREATE,   32, CG_PCL,  RS_50,  102, GT_OK}, /* amount was 100 *//* no defrag */
    /*{VT_CREATE,   33, CG_PCL,  RS_10,  900, GT_OK},*/ /* no defrag */
    {VT_CREATE,   34, CG_PCL,  RS_30,  600, GT_OK},
    {VT_COMPLETE, 41, CG_TTI,  RS_NONE,  0, GT_OK}, /* (47 * (6 + 6)) */
    {VT_COMPLETE, 35, CG_PCL,  RS_NONE,  0, GT_OK},
    {VT_CREATE,   50, CG_TTI,  RS_10,    1, GT_FULL},
    {VT_CREATE,   51, CG_PCL,  RS_10,    1, GT_FULL},
};

static VT_SEQUENCE bobk_sequence[] =
{
    {VT_CREATE,    5, CG_TTI,  RS_20,  501, GT_OK}, /* (167 * 3) */
    {VT_CREATE,    6, CG_TTI,  RS_30,  502, GT_OK}, /* ((167 * 2) + (42 * (2 + 2))) */
    {VT_COMPLETE,  7, CG_TTI,  RS_NONE,  0, GT_OK}, /* (47 * (6 + 6)) */
    /* TTI filled 2 blocks */
    {VT_CREATE,    8, CG_PCL,  RS_20,  501, GT_OK},
    {VT_CREATE,    9, CG_PCL,  RS_40, 1000, GT_OK},
    {VT_CREATE,    10, CG_PCL,  RS_50,  501, GT_OK},
    {VT_CREATE,    11, CG_PCL,  RS_10, 3501, GT_OK},
    {VT_CREATE,    12, CG_PCL,  RS_60,  250, GT_OK},
    {VT_CREATE,    13, CG_PCL,  RS_30,   50, GT_OK},
    {VT_CREATE,    14, CG_PCL,  RS_50,  407, GT_OK},
    {VT_CREATE,    15, CG_PCL,  RS_10,  407, GT_OK},
    {VT_COMPLETE,  16, CG_PCL,  RS_NONE,  0, GT_OK},  /*252*/
    {VT_CREATE,    50, CG_TTI,  RS_10,    1, GT_FULL},
    {VT_CREATE,    51, CG_PCL,  RS_10,    1, GT_FULL},
    {VT_REMOVE,     6, CG_NONE, RS_NONE,  0, GT_OK},
    {VT_REMOVE,     8, CG_NONE, RS_NONE,  0, GT_OK},
    {VT_REMOVE,    10, CG_NONE, RS_NONE,  0, GT_OK},
    {VT_REMOVE,    13, CG_NONE, RS_NONE,  0, GT_OK},
    {VT_REMOVE,    15, CG_NONE, RS_NONE,  0, GT_OK},
    {VT_CREATE,    40, CG_TTI,  RS_60,  250, GT_OK},
    {VT_CREATE,    31, CG_PCL,  RS_50,  450, GT_OK}, /* RS_40 *//* no defrag */
    {VT_CREATE,    32, CG_PCL,  RS_50,   51, GT_OK}, /* amount was 501 *//* no defrag */
    {VT_CREATE,    34, CG_PCL,  RS_30,  450, GT_OK},
    {VT_CREATE,    33, CG_PCL,  RS_10,  200, GT_OK},
    {VT_COMPLETE,  41, CG_TTI,  RS_NONE,  0, GT_OK},
    {VT_COMPLETE,  35, CG_PCL,  RS_NONE,  0, GT_OK},
    {VT_CREATE,    50, CG_TTI,  RS_10,    1, GT_FULL},
    {VT_CREATE,    51, CG_PCL,  RS_10,    1, GT_FULL},
};

static VT_SEQUENCE aldrin_sequence[] =
{
    {VT_CREATE,    5, CG_TTI,  RS_20,  252, GT_OK},
    {VT_CREATE,    6, CG_TTI,  RS_30,  250, GT_OK},
    {VT_COMPLETE,  7, CG_TTI,  RS_NONE,  0, GT_OK}, /* 342 */
    {VT_CREATE,    8, CG_PCL,  RS_20,  252, GT_OK},
    {VT_CREATE,    9, CG_PCL,  RS_40,  500, GT_OK},
    {VT_CREATE,   10, CG_PCL,  RS_50,  250, GT_OK},
    {VT_CREATE,   11, CG_PCL,  RS_10, 1752, GT_OK},
    {VT_CREATE,   12, CG_PCL,  RS_60,  125, GT_OK},
    {VT_CREATE,   13, CG_PCL,  RS_30,   24, GT_OK},
    {VT_CREATE,   14, CG_PCL,  RS_50,  205, GT_OK},
    {VT_CREATE,   15, CG_PCL,  RS_10,  207, GT_OK},
    {VT_COMPLETE, 16, CG_PCL,  RS_NONE,  0, GT_OK},
    {VT_CREATE,   50, CG_TTI,  RS_10,    1, GT_FULL},
    {VT_CREATE,   51, CG_PCL,  RS_10,    1, GT_FULL},
    {VT_REMOVE,    6, CG_NONE, RS_NONE,  0, GT_OK},
    {VT_REMOVE,    8, CG_NONE, RS_NONE,  0, GT_OK},
    {VT_REMOVE,   10, CG_NONE, RS_NONE,  0, GT_OK},
    {VT_REMOVE,   13, CG_NONE, RS_NONE,  0, GT_OK},
    {VT_REMOVE,   15, CG_NONE, RS_NONE,  0, GT_OK},
    {VT_CREATE,   40, CG_TTI,  RS_60,  125, GT_OK},
    {VT_CREATE,   31, CG_PCL,  RS_50,  225, GT_OK}, /*RS_40*/ /* no defrag */
    {VT_CREATE,   32, CG_PCL,  RS_50,   25, GT_OK}, /* amount was 100 *//* no defrag */
    {VT_CREATE,   34, CG_PCL,  RS_30,  150, GT_OK},
    {VT_COMPLETE, 41, CG_TTI,  RS_NONE,  0, GT_OK}, /* (47 * (6 + 6)) */
    {VT_COMPLETE, 35, CG_PCL,  RS_NONE,  0, GT_OK},
    {VT_CREATE,   50, CG_TTI,  RS_10,    1, GT_FULL},
    {VT_CREATE,   51, CG_PCL,  RS_10,    1, GT_FULL},
};

static VT_SEQUENCE xcat3_sequence[] =
{
    {VT_CREATE,    5, CG_TTI,  RS_30,  252, GT_OK},
    {VT_CREATE,    6, CG_TTI,  RS_30,  250, GT_OK},
    {VT_CREATE,    40, CG_TTI,  RS_60,   20, GT_BAD_PARAM},    /* Bad rule size */
    {VT_COMPLETE,  7, CG_TTI,  RS_NONE,  0, GT_OK}, /* 342 */
    {VT_CREATE,    8,  CG_PCL,  RS_60,  200, GT_OK},
    {VT_CREATE,    9,  CG_PCL,  RS_30,  500, GT_OK},
    {VT_CREATE,    10, CG_PCL,  RS_60,  501, GT_OK}, /*502*/
    {VT_CREATE,    11, CG_PCL,  RS_30,  500, GT_OK},
    {VT_CREATE,    12, CG_PCL,  RS_60,  150, GT_OK},
    {VT_CREATE,    13, CG_PCL,  RS_30,   50, GT_OK}, /*52*/
    {VT_CREATE,    14, CG_PCL,  RS_30,  100, GT_OK},
    {VT_CREATE,    15, CG_PCL,  RS_60,   40, GT_OK},
    {VT_CREATE,    41, CG_PCL,  RS_10,   20, GT_BAD_PARAM},    /* Bad rule size */
    {VT_CREATE,    42, CG_PCL,  RS_20,   20, GT_BAD_PARAM},    /* Bad rule size */
    {VT_CREATE,    44, CG_PCL,  RS_40,   20, GT_BAD_PARAM},    /* Bad rule size */
    {VT_CREATE,    45, CG_PCL,  RS_50,   20, GT_BAD_PARAM},    /* Bad rule size */
    {VT_COMPLETE,    16, CG_PCL,  RS_30,   (768*4)-(200*2)-500-(502*2)-500-(150*2)-52-100-(40*2) /* 136 */, GT_OK},
    {VT_CREATE,   50, CG_TTI,  RS_30,    1, GT_FULL},
    {VT_CREATE,    51, CG_PCL,  RS_30,    1, GT_FULL},
    {VT_REMOVE,     5, CG_NONE, RS_NONE,  0, GT_OK},
    {VT_REMOVE,     6, CG_NONE, RS_NONE,  0, GT_OK},
    {VT_REMOVE,     7, CG_NONE, RS_NONE,  0, GT_OK},
    {VT_REMOVE,     8, CG_NONE, RS_NONE,  0, GT_OK},
    {VT_REMOVE,    10, CG_NONE, RS_NONE,  0, GT_OK},
    {VT_REMOVE,    13, CG_NONE, RS_NONE,  0, GT_OK},
    {VT_REMOVE,    15, CG_NONE, RS_NONE,  0, GT_OK},
};

static VT_SEQUENCE lion2_sequence[] =
{
    {VT_CREATE,    5, CG_TTI,  RS_30,  252, GT_OK},
    {VT_CREATE,    6, CG_TTI,  RS_30,  250, GT_OK},
    {VT_CREATE,    40, CG_TTI,  RS_60,   20, GT_BAD_PARAM},    /* Bad rule size */
    {VT_COMPLETE,  7, CG_TTI,  RS_NONE,  0, GT_OK}, /* 342 */
    {VT_CREATE,    8,  CG_PCL,  RS_60,  166, GT_OK},
    {VT_CREATE,    9,  CG_PCL,  RS_30,  332, GT_OK},
    {VT_CREATE,    10, CG_PCL,  RS_60,  334, GT_OK},
    {VT_CREATE,    11, CG_PCL,  RS_30,  332, GT_OK},
    {VT_CREATE,    12, CG_PCL,  RS_60,  100, GT_OK},
    {VT_CREATE,    13, CG_PCL,  RS_30,   32, GT_OK},
    {VT_CREATE,    14, CG_PCL,  RS_30,   68, GT_OK},
    {VT_CREATE,    15, CG_PCL,  RS_60,   26, GT_OK},
    {VT_CREATE,    41, CG_PCL,  RS_10,   20, GT_BAD_PARAM},    /* Bad rule size */
    {VT_CREATE,    42, CG_PCL,  RS_20,   20, GT_BAD_PARAM},    /* Bad rule size */
    {VT_CREATE,    44, CG_PCL,  RS_40,   20, GT_BAD_PARAM},    /* Bad rule size */
    {VT_CREATE,    45, CG_PCL,  RS_50,   20, GT_BAD_PARAM},    /* Bad rule size */
    {VT_COMPLETE,    16, CG_PCL,  RS_30,   (512*4)-(166*2)-332-(334*2)-332-(100*2)-32-68-(26*2) /*  32 */, GT_OK},
    {VT_CREATE,   50, CG_TTI,  RS_30,    1, GT_FULL},
    {VT_CREATE,    51, CG_PCL,  RS_30,    1, GT_FULL},
    {VT_REMOVE,     8, CG_NONE, RS_NONE,  0, GT_OK},
    {VT_REMOVE,    10, CG_NONE, RS_NONE,  0, GT_OK},
    {VT_REMOVE,    13, CG_NONE, RS_NONE,  0, GT_OK},
    {VT_REMOVE,    15, CG_NONE, RS_NONE,  0, GT_OK},
};

/*Get total num of rules with proper size of TCAM per device for SIP_5 and xCat3 dev*/
void internal_ut_cpssDxChVirtualTcamTotalRulesAllowedPerDevGet(
    IN  GT_U32                               devNum,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT ruleSize,
    OUT GT_U32                               *ptrTotalRulesAllowed
)
{
    GT_U32                               rulesPerRow;   /* rules per TCAM row */

    rulesPerRow = 12 / (ruleSize+1);
    if (PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        *ptrTotalRulesAllowed =
            (PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.tunnelTerm * rulesPerRow) / 12;
    }
    else
    {
        *ptrTotalRulesAllowed =
            PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.policyTcamRaws * rulesPerRow;
    }
}

static GT_U32  getRulesAmountFor_VT_COMPLETE(
    IN  GT_U32                               devNum,
    IN GT_U32 vTcamMngId,
    IN GT_U32   clientGroup,
    OUT CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT            *ruleSizePtr
)
{
    GT_STATUS   st;
    GT_U32 blockSize = 6 * 256;
    GT_U32 rulesInRow = 6;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT            ruleSize;
    GT_U32 maxVTcamId = 100;
    GT_U32 vTcamId1;
    GT_U32 spentSize = 0;
    GT_U32 granulatedSpentSize = 0;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC info;
    CPSS_DXCH_VIRTUAL_TCAM_USAGE_STC usage;
    GT_U32 result = 0;

    if (PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        ruleSize = RS_10;
        rulesInRow = 6;
    }
    else
    {
        ruleSize = RS_30;
        switch (clientGroup)
        {
            case CG_PCL:
                blockSize = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.policyTcamRaws * 4 *
                                (ruleSize + 1);
                rulesInRow = 4;
                break;
            case CG_TTI:
                blockSize = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.tunnelTerm *
                                (ruleSize + 1);
                rulesInRow = 1;
                break;
            default:
                return 0;
        }
    }
    for (vTcamId1 = 0; (vTcamId1 < maxVTcamId); vTcamId1++)
    {
        if(0 == isVtcamInBmp(vTcamId1))
        {
            /* stop generating errors in CPSS :
            ERROR GT_NOT_FOUND in function: internal_cpssDxChVirtualTcamInfoGet, file cpssdxchvirtualtcam.c, line[????].
            */
            continue;
        }

        st = cpssDxChVirtualTcamInfoGet(
            vTcamMngId, vTcamId1, &info);
        if (st != GT_OK) continue;
        if (info.clientGroup != clientGroup) continue;
        st = cpssDxChVirtualTcamUsageGet(
            vTcamMngId, vTcamId1, &usage);
        if (st != GT_OK)
        {
            break;
        }
        spentSize +=
            ((usage.rulesUsed + usage.rulesFree) * (info.ruleSize + 1));
        PRV_UTF_LOG4_MAC(
            "vTcam %d, rulesUsed %d, rulesFree %d, ruleSize %d\n",
            vTcamId1, usage.rulesUsed, usage.rulesFree,
            (info.ruleSize + 1));
    }

    granulatedSpentSize = (spentSize + (rulesInRow - 1));
    granulatedSpentSize -= (granulatedSpentSize % rulesInRow);

    /* get vTCAM size */
    if ((granulatedSpentSize % blockSize) != 0)
    {
        result = (blockSize - (granulatedSpentSize % blockSize)) / (ruleSize + 1);
    }
    else
    {
        result = 0;
    }
    PRV_UTF_LOG3_MAC(
        "spentSize %d, granulatedSpentSize %d, rulesAmount_for_complete %d \n",
        spentSize, granulatedSpentSize, result);
    *ruleSizePtr = ruleSize;
    return result;
}

static GT_STATUS vtcamSequenceEntryProcessLastResult = GT_OK;

GT_STATUS vtcamSequenceEntryProcess(
    INOUT VT_SEQUENCE *sequencePtr,
    IN GT_U32 idx,
    IN GT_U32 vTcamMngId,
    INOUT CPSS_DXCH_VIRTUAL_TCAM_INFO_STC  *vTcamInfoPtr
)
{
    GT_U32                                vTcamId;
    GT_STATUS                             st, expectedRc;
    GT_U32                                rulePlace;
    GT_BOOL                               toInsert;
    GT_U32                                sizeInRules;
    GT_U32                                ruleIdBase;
    GT_U32                                ruleStampBase;
    GT_U32                                rulesAmount;

    vTcamId                            = sequencePtr[idx].vTcamId;

    vTcamInfoPtr->clientGroup          = (sequencePtr[idx].clientGroup & 0x0F); /* TTI and IPCL0 */
    vTcamInfoPtr->hitNumber         = ((sequencePtr[idx].clientGroup >> 4) & 0x0F);
    vTcamInfoPtr->ruleSize             = sequencePtr[idx].ruleSize;
    vTcamInfoPtr->guaranteedNumOfRules = sequencePtr[idx].rulesAmount;

    expectedRc                     = sequencePtr[idx].rc;

    switch (sequencePtr[idx].operation)
    {
        case VT_CREATE:
            /* Create vTCAM */
            /* do not call ut_cpssDxChVirtualTcamCreate to avoid any relation to
               'check availability' */
            PRV_UTF_LOG4_MAC(
                "Create vTcamId %d client_group %d rule_size %d rules_amount %d\n",
                sequencePtr[idx].vTcamId,
                sequencePtr[idx].clientGroup,
                (sequencePtr[idx].ruleSize + 1),
                sequencePtr[idx].rulesAmount);
            st = cpssDxChVirtualTcamCreate(vTcamMngId, vTcamId, vTcamInfoPtr);
            if(st == GT_OK)
            {
                addVtcamToBmp(vTcamId);
            }
            break;
        case VT_REMOVE:
            PRV_UTF_LOG1_MAC(
                "Remove vTcamId %d \n",
                sequencePtr[idx].vTcamId);
            st = ut_cpssDxChVirtualTcamRemove(vTcamMngId, vTcamId);
            if(st == GT_OK)
            {
                removeVtcamFromBmp(vTcamId);
            }
            break;
        case VT_COMPLETE:
            PRV_UTF_LOG2_MAC(
                "Complete vTcamId %d client_group %d\n",
                sequencePtr[idx].vTcamId,
                sequencePtr[idx].clientGroup);
            vTcamInfoPtr->guaranteedNumOfRules =
                getRulesAmountFor_VT_COMPLETE(devListArr[0],
                    vTcamMngId,
                    vTcamInfoPtr->clientGroup,
                    &vTcamInfoPtr->ruleSize);
            PRV_UTF_LOG3_MAC(
                "Complete vTcamId %d ruleSize %d rulesAmount %d\n",
                vTcamId, ((vTcamInfoPtr->ruleSize + 1) * 10),
                vTcamInfoPtr->guaranteedNumOfRules);
            sequencePtr[idx].ruleSize    = vTcamInfoPtr->ruleSize;
            sequencePtr[idx].rulesAmount = vTcamInfoPtr->guaranteedNumOfRules;
            /* do not call ut_cpssDxChVirtualTcamCreate to avoid any relation to
               'check availability' */
            st = cpssDxChVirtualTcamCreate(vTcamMngId, vTcamId, vTcamInfoPtr);
            if(st == GT_OK)
            {
                addVtcamToBmp(vTcamId);
            }
            break;

        case VT_INCREASE:/* increase number of rules (add to start) */
        case VT_DECREASE:/* decrease number of rules (remove from start) */
            if(sequencePtr[idx].operation == VT_INCREASE)
            {
                rulePlace = (priorityMode == GT_TRUE) ?
                    0 /*first priority*/ :
                    CPSS_DXCH_VIRTUAL_TCAM_BEFORE_START_CNS;
                toInsert = GT_TRUE;
            }
            else
            {
                rulePlace = 0;/* first priority / index 0 */
                toInsert = GT_FALSE;
            }
            sizeInRules = sequencePtr[idx].rulesAmount;

            PRV_UTF_LOG4_MAC(
                "%s vTcamId %d rulePlace %d sizeInRules %d\n",
                ((sequencePtr[idx].operation == VT_INCREASE) ? "INCREASE" : "DECREASE"),
                sequencePtr[idx].vTcamId,
                rulePlace, sizeInRules);
            /* do not call ut_cpssDxChVirtualTcamResize to avoid any relation to
               'check availability' */
            st = cpssDxChVirtualTcamResize(vTcamMngId,vTcamId,rulePlace,toInsert,sizeInRules);
            break;
        case VT_STAMP_FILL:
            /* rules amount will be taken from vTcam info */
            PRV_UTF_LOG1_MAC(
                "STAMP_FILL vTcamId %d\n",
                sequencePtr[idx].vTcamId);
            if ((sequencePtr[idx].clientGroup & 0xF) != CG_TTI)
            {
                fillVtcamWithRules(
                    vTcamMngId, vTcamId, sequencePtr[idx].rulesAmount/*maxRulesToFill*/);
            }
            else
            {
                fillVtcamWithTtiRules(
                    vTcamMngId, vTcamId, sequencePtr[idx].rulesAmount/*maxRulesToFill*/);
            }
            st = GT_OK;
            break;
        case VT_STAMP_CHECK:
            PRV_UTF_LOG1_MAC(
                "STAMP_CHECK vTcamId %d\n",
                sequencePtr[idx].vTcamId);
            if ((sequencePtr[idx].clientGroup & 0xF)!= CG_TTI)
            {
                checkVtcamWithRules(vTcamMngId, vTcamId, sequencePtr[idx].rulesAmount);
            }
            else
            {
                checkVtcamWithTtiRules(vTcamMngId, vTcamId, sequencePtr[idx].rulesAmount);
            }
            st = GT_OK;
            break;
        case VT_STAMP_RANGE_CHECK:
            ruleIdBase = gtStringUtilDecimalFromLine(
                sequencePtr[idx].title, "ruleIdBase");
            ruleStampBase = gtStringUtilDecimalFromLine(
                sequencePtr[idx].title, "ruleStampBase");
            rulesAmount = gtStringUtilDecimalFromLine(
                sequencePtr[idx].title, "rulesAmount");
            PRV_UTF_LOG4_MAC(
                "VT_STAMP_RANGE_CHECK vTcamId %d ruleIdBase %d ruleStampBase %d rulesAmount %d\n",
                sequencePtr[idx].vTcamId, ruleIdBase, ruleStampBase, rulesAmount);
            checkVtcamRuleRangeStamps(
                vTcamMngId, vTcamId,
                ruleIdBase, ruleStampBase, rulesAmount);
            st = GT_OK;
            break;
        case VT_TITLE:
            cpssOsPrintf(
                "vtcamSequenceEntryProcess Title: %s \n", sequencePtr[idx].title);
            return GT_OK;
            break;

        default:
            st = GT_FAIL;
            break;
    }
    if (st != GT_OK)
    {
        PRV_UTF_LOG1_MAC("Returned code %d\n", st);
    }
    else if (sequencePtr[idx].operation != VT_REMOVE)
    {
        CPSS_DXCH_VIRTUAL_TCAM_USAGE_STC   vTcamUsage;
        GT_STATUS rc1;
        rc1 = cpssDxChVirtualTcamUsageGet(
            vTcamMngId, vTcamId, &vTcamUsage);
        if (rc1 != GT_OK)
        {
            PRV_UTF_LOG1_MAC("cpssDxChVirtualTcamUsageGet failed, rc == %d\n", rc1);
        }
        else
        {
            PRV_UTF_LOG4_MAC(
                "vTcam(%d) rules total %d used  %d free %d\n",
                vTcamId, (vTcamUsage.rulesUsed + vTcamUsage.rulesFree),
                vTcamUsage.rulesUsed, vTcamUsage.rulesFree);
        }
    }
    cpssDxChVirtualTcamUt_vTcamAllocStatusPrint(vTcamMngId, vTcamId);
    checkCpssSegmentsDbIsOk(vTcamMngId);

    vtcamSequenceEntryProcessLastResult = st;
    return (st == expectedRc) ? st : GT_FAIL;
}

static GT_STATUS vtcamSequenceIncreaseDecreaseProcess(
    INOUT VT_INCREASE_DECREASE_SEQUENCE *sequencePtr,
    IN GT_U32 idx,
    IN GT_U32 vTcamMngId
)
{
    GT_U32             vTcamId;
    GT_STATUS          st, expectedRc;
    GT_U32             rulePlace;
    GT_BOOL            toInsert;
    GT_U32             sizeInRules;

    vTcamId            = sequencePtr[idx].vTcamId;
    expectedRc         = sequencePtr[idx].rc;

    switch (sequencePtr[idx].operation)
    {
        case VT_INCREASE:/* increase number of rules (add to start) */
        case VT_DECREASE:/* decrease number of rules (remove from start) */
            if(sequencePtr[idx].operation == VT_INCREASE)
            {
                toInsert = GT_TRUE;
            }
            else
            {
                toInsert = GT_FALSE;
            }
            sizeInRules = sequencePtr[idx].rulesAmount;
            rulePlace = sequencePtr[idx].rulesPlace;

            PRV_UTF_LOG4_MAC(
                "%s vTcamId %d rulePlace %d sizeInRules %d\n",
                ((sequencePtr[idx].operation == VT_INCREASE) ? "INCREASE" : "DECREASE"),
                sequencePtr[idx].vTcamId,
                rulePlace, sizeInRules);
            /* do not call ut_cpssDxChVirtualTcamResize to avoid any relation to
               'check availability' */
            st = cpssDxChVirtualTcamResize(vTcamMngId,vTcamId,rulePlace,toInsert,sizeInRules);

            break;
        default:
            st = GT_FAIL;
            break;
    }
    if (st != GT_OK)
    {
        PRV_UTF_LOG1_MAC("Returned code %d\n", st);
    }
    cpssDxChVirtualTcamUt_vTcamAllocStatusPrint(vTcamMngId, vTcamId);
    checkCpssSegmentsDbIsOk(vTcamMngId);

    return (st == expectedRc) ? st : GT_FAIL;
}

static void autoresizeGetIds
(
    IN GT_U32                          iterationIndex,
    IN GT_U32                          ruleIndex,
    OUT CPSS_DXCH_VIRTUAL_TCAM_RULE_ID *ruleIdPtr,
    OUT GT_U32                         *priorityPtr
)
{
    switch (iterationIndex)
    {
        default:
        case 0:
            *priorityPtr   = (ruleIndex / 2);
            *ruleIdPtr     = (10000 + ruleIndex);
            break;
        case 1:
            *priorityPtr   = (ruleIndex % 10);
            *ruleIdPtr     = (10000 - ruleIndex);
            break;
        case 2:
            *priorityPtr   = 200 - ruleIndex;
            *ruleIdPtr     = (10000 + ((ruleIndex % 10) * 10) + (ruleIndex / 10));
            break;
        case 3:
            *priorityPtr   = ((ruleIndex % 10) * 10) + (ruleIndex / 10);
            *ruleIdPtr     = 100100 - ruleIndex;
            break;
    }
}

UTF_TEST_CASE_MAC(cpssDxChVirtualTcam_autoresize)
{
    GT_STATUS                                  st   = GT_OK;
    GT_STATUS                                  st1  = GT_OK;
    GT_U8                                      dev;
    GT_U32                                     numOfDevs = 0;
    GT_U32                                     notAppFamilyBmp = 0;
    GT_U32                                     utfFamilyBit = 0;
    GT_BOOL                                    isTestSkipped = GT_TRUE;
    GT_U32                                     vTcamMngId;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC            vTcamInfo;
    GT_U32                                     vTcamId;
    CPSS_DXCH_VIRTUAL_TCAM_USAGE_STC           vTcamUsage;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ID             RuleId;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ID             RuleIdGet;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC RuleAttributes;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC       TcamRuleType;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC       RuleData;
    CPSS_DXCH_PCL_ACTION_STC                   pclAction;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              mask;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              pattern;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC       RuleDataGet;
    CPSS_DXCH_PCL_ACTION_STC                   pclActionGet;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              maskGet;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              patternGet;
    GT_U32                                     ruleIdx;
    GT_U32                                     priority;
    GT_U32                                     iterationIdx;

    vTcamMngId                     = 1;
    vTcamId                        = 1;
    vTcamInfo.clientGroup          = 0; /* TTI and IPCL0 */
    vTcamInfo.hitNumber            = 0;
    vTcamInfo.ruleSize             = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E;
    vTcamInfo.autoResize           = GT_TRUE;
    vTcamInfo.guaranteedNumOfRules = 0;
    vTcamInfo.ruleAdditionMethod =
            CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E;

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* this feature is on eArch devices, xCat3 and Lion2 */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    notAppFamilyBmp &= (~UTF_XCAT3_E) & (~UTF_LION2_E);

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* 1. Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
              devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }
        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;

        /* Create vTCAM manager */
        st = cpssDxChVirtualTcamManagerCreate(vTcamMngId);
            if (st == GT_ALREADY_EXIST)
                st = GT_OK;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerCreate failed\n");

        st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        if (st != GT_OK)
        {
            cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
            st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        }
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListAdd failed\n");

        /* 1. Create vTCAM(1) */
        st = ut_cpssDxChVirtualTcamCreate(vTcamMngId, vTcamId, &vTcamInfo);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamCreate failed for vTcamMngId = %d, st = %d\n", vTcamMngId, st);

        if(GT_OK == st)
        {
            /* Add PCL 100 rules to vTCAM (1) with priority (0x10000 + (ruleIdx/2)) */
            RuleAttributes.priority          = 0xFFFFFFF;
            TcamRuleType.ruleType            = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;
            TcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
            RuleId                           = 0xFFFFFFF;
            RuleData.valid = GT_TRUE;
            /* set new rule */
            cpssOsMemSet(&mask, '\0', sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
            cpssOsMemSet(&pattern, '\0', sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
            cpssOsMemSet(&pclAction, '\0', sizeof(CPSS_DXCH_PCL_ACTION_STC));
            /* Set Mask */
            setRuleStdNotIp(&mask, &pattern, &pclAction);

            RuleData.rule.pcl.actionPtr     = &pclAction;
            RuleData.rule.pcl.maskPtr       = &mask;
            RuleData.rule.pcl.patternPtr    = &pattern;

            RuleDataGet.rule.pcl.actionPtr     = &pclActionGet;
            RuleDataGet.rule.pcl.maskPtr       = &maskGet;
            RuleDataGet.rule.pcl.patternPtr    = &patternGet;

            for (iterationIdx = 0; (iterationIdx < 4); iterationIdx++)
            {
                for (ruleIdx = 0; (ruleIdx < 100); ruleIdx ++)
                {
                    autoresizeGetIds(
                        iterationIdx, ruleIdx, &RuleId, &priority);
                    RuleAttributes.priority = priority;
                    UNIQUE_PCL_RULE_SET_MAC(
                        &TcamRuleType,RuleData.rule.pcl.patternPtr, RuleId);

                    /*DEBUG_PRINT(vTcamMngId,vTcamId);*/

                    st = cpssDxChVirtualTcamRuleWrite(
                        vTcamMngId, vTcamId, RuleId,
                        &RuleAttributes, &TcamRuleType, &RuleData);
                    UTF_VERIFY_EQUAL3_STRING_MAC(
                        GT_OK, st,
                        "cpssDxChVirtualTcamRuleWrite failed, iterationIdx: %d ruleIdx: %d RuleId: 0x%X\n",
                        iterationIdx, ruleIdx, RuleId);

                    checkCpssDbIsOk(vTcamMngId, vTcamId);
                }

                st1 = checkCpssTreesDbIsOk(vTcamMngId);
                UTF_VERIFY_EQUAL1_STRING_MAC(
                    GT_OK, st1,
                    "checkCpssTreesDbIsOk after Write failed, iterationIdx: %d\n",
                    iterationIdx);

                for (ruleIdx = 0; (ruleIdx < 100); ruleIdx ++)
                {
                    autoresizeGetIds(
                        iterationIdx, ruleIdx, &RuleId, &priority);
                    st = cpssDxChVirtualTcamRuleRead(
                        vTcamMngId, vTcamId, RuleId,
                        &TcamRuleType, &RuleDataGet);
                    UTF_VERIFY_EQUAL3_STRING_MAC(
                        GT_OK, st,
                        "cpssDxChVirtualTcamRuleRead failed, iterationIdx: %d ruleIdx: %d RuleId: 0x%X\n",
                        iterationIdx, ruleIdx, RuleId);
                    UNIQUE_PCL_RULE_GET_MAC(
                        &TcamRuleType,RuleDataGet.rule.pcl.patternPtr, RuleIdGet);
                    UTF_VERIFY_EQUAL0_STRING_MAC(
                        RuleId, RuleIdGet,
                        "cpssDxChVirtualTcamRuleRead got not expected rule contents\n");
                }

                /* Get vTCAM usage */
                st = cpssDxChVirtualTcamUsageGet(vTcamMngId, vTcamId, &vTcamUsage);
                UTF_VERIFY_EQUAL3_STRING_MAC(
                    GT_OK, st,
                    "cpssDxChVirtualTcamUsageGet failed, iterationIdx: %d ruleIdx: %d RuleId: 0x%X\n",
                    iterationIdx, ruleIdx, RuleId);
                UTF_VERIFY_EQUAL0_STRING_MAC(
                    100, vTcamUsage.rulesUsed,
                    "not correct rulesUsed\n");

                for (ruleIdx = 0; (ruleIdx < 100); ruleIdx ++)
                {
                    autoresizeGetIds(
                        iterationIdx, ruleIdx, &RuleId, &priority);

                    st = cpssDxChVirtualTcamRuleRead(
                        vTcamMngId, vTcamId, RuleId,
                        &TcamRuleType, &RuleDataGet);
                    UTF_VERIFY_EQUAL3_STRING_MAC(
                        GT_OK, st,
                        "cpssDxChVirtualTcamRuleRead failed, iterationIdx: %d ruleIdx: %d RuleId: 0x%X\n",
                        iterationIdx, ruleIdx, RuleId);
                    UNIQUE_PCL_RULE_GET_MAC(
                        &TcamRuleType,RuleDataGet.rule.pcl.patternPtr, RuleIdGet);
                    UTF_VERIFY_EQUAL0_STRING_MAC(
                        RuleId, RuleIdGet,
                        "cpssDxChVirtualTcamRuleRead got not expected rule contents\n");

                    st = cpssDxChVirtualTcamRuleDelete(
                        vTcamMngId, vTcamId, RuleId);
                    st1 = checkCpssDbIsOkWithReturnCode(vTcamMngId, vTcamId);
                    UTF_VERIFY_EQUAL3_STRING_MAC(
                        GT_OK, st1,
                        "checkCpssDbIsOkWithReturnCode after Delete failed, iterationIdx: %d Write ruleIdx: %d RuleId: 0x%X\n",
                        iterationIdx, ruleIdx, RuleId);
                    UTF_VERIFY_EQUAL3_STRING_MAC(
                        GT_OK, st,
                        "cpssDxChVirtualTcamRuleDelete failed, iterationIdx: %d ruleIdx: %d RuleId: 0x%X\n",
                        iterationIdx, ruleIdx, RuleId);

                    /* Get vTCAM usage */
                    st = cpssDxChVirtualTcamUsageGet(vTcamMngId, vTcamId, &vTcamUsage);
                    UTF_VERIFY_EQUAL2_STRING_MAC(
                        GT_OK, st, "cpssDxChVirtualTcamUsageGet failed rulesUsed = %d / rulesFrees = %d\n",
                        vTcamUsage.rulesUsed, vTcamUsage.rulesFree );

                    UTF_VERIFY_EQUAL1_STRING_MAC(
                        (99 - ruleIdx), vTcamUsage.rulesUsed,
                        "not correct rulesUsed, ruleIdx: %d \n", ruleIdx);
                    if (vTcamUsage.rulesUsed == 0)
                    {
                        UTF_VERIFY_EQUAL1_STRING_MAC(
                            0, vTcamUsage.rulesFree,
                            "not correct rulesFree, ruleIdx: %d \n", ruleIdx);
                    }
                }
                st1 = checkCpssTreesDbIsOk(vTcamMngId);
                UTF_VERIFY_EQUAL1_STRING_MAC(
                    GT_OK, st1,
                    "checkCpssTreesDbIsOk after Write failed, iterationIdx: %d\n",
                    iterationIdx);

            }
        }

        st = ut_cpssDxChVirtualTcamRemove(vTcamMngId, vTcamId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamRemove failed\n");

        st = cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st, "cpssDxChVirtualTcamManagerDevListRemove failed for vTcamMngId = %d\n",
            vTcamMngId);
        st = cpssDxChVirtualTcamManagerDelete(vTcamMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDelete failed for vTcamMngId = %d\n",
                                     vTcamMngId);
    }

    if (isTestSkipped)
    {
        SKIP_TEST_MAC;
    }
}

/* priority mode :
   force the 'ruleWrite' API to use 'first' position.
   (by default it uses the 'last' position)
   debug mode to allow quick testing of the 'position first' that
   the 'rule Update' API allow to use.

   ===============================
   aggregate several tests that need to run under this mode.
*/
#if 0
UTF_TEST_CASE_MAC(cpssDxChVirtualTcam_priorityMode_writeRulePositionForceFirst)
{
    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    debug_writeRulePosition_enable = GT_TRUE;
    prvCpssDxChVirtualTcamDbVTcam_debug_writeRulePositionForceFirstSet(debug_writeRulePosition_enable);
    UTF_TEST_CALL_MAC(cpssDxChVirtualTcam_increaseVtcamSizebyResizing_priorityMode);
    UTF_TEST_CALL_MAC(cpssDxChVirtualTcam_autoresize);
    debug_writeRulePosition_enable = GT_FALSE;
    prvCpssDxChVirtualTcamDbVTcam_debug_writeRulePositionForceFirstSet(debug_writeRulePosition_enable);
}
#endif /*0*/
UTF_TEST_CASE_MAC(cpssDxChVirtualTcam_fragmentation)
{
    GT_STATUS                                  st   = GT_OK;
    GT_U8                                      dev;
    GT_U32                                     numOfDevs = 0;
    GT_U32                                     notAppFamilyBmp = 0;
    GT_U32                                     utfFamilyBit = 0;
    GT_BOOL                                    isTestSkipped = GT_TRUE;
    GT_U32                                     vTcamMngId;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC            vTcamInfo;
    VT_SEQUENCE                                *sequencePtr;
    GT_U32                                     sequenceSize;
    GT_U32                                     idx;

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* this feature is on eArch devices, xCat3 and Lion2 */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    notAppFamilyBmp &= (~UTF_XCAT3_E) & (~UTF_LION2_E);

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* 1. Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
              devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }
        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;

        switch (utfFamilyBit)
        {
            case UTF_XCAT3_E:
                sequencePtr = xcat3_sequence;
                sequenceSize = sizeof(xcat3_sequence) / sizeof(xcat3_sequence[0]);
                break;

            case UTF_LION2_E:
                sequencePtr = lion2_sequence;
                sequenceSize = sizeof(lion2_sequence) / sizeof(lion2_sequence[0]);
                break;

            case UTF_ALDRIN2_E:
            case UTF_FALCON_E:
            case UTF_CAELUM_E:
                sequencePtr = bobk_sequence;
                sequenceSize = sizeof(bobk_sequence) / sizeof(bobk_sequence[0]);
                break;

            case UTF_ALDRIN_E:
            case UTF_AC3X_E:
                sequencePtr = aldrin_sequence;
                sequenceSize = sizeof(aldrin_sequence) / sizeof(aldrin_sequence[0]);
                break;

            case UTF_BOBCAT2_E:
            case UTF_BOBCAT3_E:
            default:
                sequencePtr = bc2_sequence;
                sequenceSize = sizeof(bc2_sequence) / sizeof(bc2_sequence[0]);
                break;
        }

        vTcamMngId = 1;
        cpssOsMemSet(&vTcamInfo, 0, sizeof(vTcamInfo));
        vTcamInfo.hitNumber     = 0;
        vTcamInfo.autoResize    = GT_FALSE;
        vTcamInfo.ruleAdditionMethod =
            CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E;

        /* Create vTCAM manager */
        st = cpssDxChVirtualTcamManagerCreate(vTcamMngId);
            if (st == GT_ALREADY_EXIST)
                st = GT_OK;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerCreate failed\n");

        st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        if (st != GT_OK)
        {
            cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
            st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        }
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListAdd failed\n");


        for (idx = 0; (idx < sequenceSize); idx++)
        {
            st = vtcamSequenceEntryProcess(sequencePtr, idx, vTcamMngId, &vTcamInfo);
            UTF_VERIFY_NOT_EQUAL1_STRING_MAC(
                GT_FAIL, st,
                "vtcamSequenceEntryProcess failed, index: %d\n", idx);
        }

        st = cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st, "cpssDxChVirtualTcamManagerDevListRemove failed for vTcamMngId = %d\n",
            vTcamMngId);
        st = cpssDxChVirtualTcamManagerDelete(vTcamMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDelete failed for vTcamMngId = %d\n",
                                     vTcamMngId);
    }

    if (isTestSkipped)
    {
        SKIP_TEST_MAC;
    }
}

/*******************************************************************************
* cpssDxChVirtualTcam_AutoResizeCheck
*
* DESCRIPTION:
*       Test auto-resize on write rule priority mode
* PURPOSE:
*       Check that the auto-resize allow full vtcam to add
*       new rule if tcam manager still have place.
* LOGIC:
*       1. Create vtcam with auto-resize = GT_TRUE, guarantied 10 entries (30B rules)
*       2. (priority 5) Write 10 rules
*       3. (priority 5) Write another rule expect to success.
*
*       1. Create other vtcam with auto-resize = GT_FALSE,
*          guarantied 10 entries (30B rules)
*       2. (priority 5) Write 10 rules
*       3. (priority 5) Write another rule expect to FAIL.
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*
*******************************************************************************/
UTF_TEST_CASE_MAC(cpssDxChVirtualTcam_AutoResizeCheck)
{
    GT_BOOL                                    isAutoResized = GT_TRUE;
    GT_STATUS                                  rc;
    GT_U32                                     notAppFamilyBmp = 0;
    GT_U32                                     utfFamilyBit = 0;
    GT_BOOL                                    isTestSkipped = GT_TRUE;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC            vTcamInfo;
    GT_U32                                     vTcamId;
    GT_U8                                      vTcamMngId;
    GT_U8                                      dev;
    GT_U16                                     jj;
    GT_U16                                     ii;

    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC ruleAttributes;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ID             ruleId;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              mask;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              pattern;
    CPSS_DXCH_PCL_ACTION_STC                   pclAction;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC       ruleData;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC       tcamRuleType;
    GT_U8                                      numOfRulesPerVtcam;
    GT_U32                                     numOfDevs = 0;

    vTcamMngId                     = 1;
    vTcamId                        = 1;
    /* due to granularity == 4 on Legcy device the real size will be 12 */
    numOfRulesPerVtcam             = 12; /*was 10*/

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* this feature is on eArch devices, xCat3 and Lion2 */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    notAppFamilyBmp &= (~UTF_XCAT3_E) & (~UTF_LION2_E);

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* 1. Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
              devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }
        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;

        isAutoResized = GT_TRUE;
        for( ii = 0; ii < 2; ii++)
        {
            /* AUTODOC: SETUP CONFIGURATION: */
            /* AUTODOC: Create first 30 Bytes logical index based vTCAMs with auto resize*/
            vTcamInfo.ruleSize             = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E;
            vTcamInfo.autoResize           = isAutoResized;
            vTcamInfo.guaranteedNumOfRules = numOfRulesPerVtcam;
            vTcamInfo.ruleAdditionMethod   = CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E;

            /* Create vTCAM manager */
            rc = cpssDxChVirtualTcamManagerCreate(vTcamMngId);
            if (rc == GT_ALREADY_EXIST)
                rc = GT_OK;
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamManagerCreate failed\n");

            rc = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId, devListArr, numOfDevs);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamManagerDevListAdd failed\n");

            /* AUTODOC: create vTCAM */
            vTcamInfo.hitNumber   = 0;
            vTcamInfo.clientGroup = 0;

            rc = cpssDxChVirtualTcamCreate(vTcamMngId, vTcamId, &vTcamInfo);
            UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, rc, "cpssDxChVirtualTcamCreate FAILED for vTCAM = %d",
                vTcamId);

            /*AUTODOC: 2.  Priority 5 - Make rules #0..10 */
            ruleAttributes.priority      = 5;
            for (ruleId = 0; ruleId < numOfRulesPerVtcam; ruleId++)
            {
                addRuleNoHitByIndexPriority(ruleId, &ruleAttributes);
            }

            ruleData.valid                   = GT_TRUE;
            ruleData.rule.pcl.actionPtr      = &pclAction;
            ruleData.rule.pcl.maskPtr        = &mask;
            ruleData.rule.pcl.patternPtr     = &pattern;
            tcamRuleType.ruleType            = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;
            tcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;

            cpssOsMemSet(&mask, '\0', sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
            cpssOsMemSet(&pattern, '\0', sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
            cpssOsMemSet(&pclAction, '\0', sizeof(CPSS_DXCH_PCL_ACTION_STC));
            /* Set Mask */

            setRuleStdNotIp(&mask, &pattern, &pclAction);

            /*3. (priority 5) Write another rule */
            rc = cpssDxChVirtualTcamRuleWrite(vTcamMngId, vTcamId, ruleId,
                                                  &ruleAttributes, &tcamRuleType, &ruleData);
            /* expect to success when isAutoResized == GT_TRUE*/
            if(isAutoResized)
            {
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                             "cpssDxChVirtualTcamRuleWrite: vTCAM = %d, RuleID = %d, isAutoResized = %d",
                                                 vTcamId, ruleId, isAutoResized);
            }
            /* expect to FAIL when isAutoResized == GT_FALSE.*/
            else
            {
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_FULL, rc,
                                             "cpssDxChVirtualTcamRuleWrite: vTCAM = %d, RuleID = %d, isAutoResized = %d",
                                                 vTcamId, ruleId, isAutoResized);
            }

            /* AUTODOC: Remove vTCAMs with PCL rules  */
            for(jj = 0; jj < numOfRulesPerVtcam; jj++ )
            {
                rc = cpssDxChVirtualTcamRuleDelete(vTcamMngId, vTcamId, jj);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleDelete(vTCAM:%d) for ruleId[%d] failed\n",
                                             vTcamId, jj);
            }
            if(isAutoResized)
            {
                rc = cpssDxChVirtualTcamRuleDelete(vTcamMngId, vTcamId, jj);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleDelete(vTCAM:%d) for ruleId[%d] failed\n",
                                             vTcamId, jj);
            }

            rc = cpssDxChVirtualTcamRemove(vTcamMngId, vTcamId);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRemove(vTCAM:%d) failed\n", vTcamId);

            rc = cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamManagerDevListRemove failed for vTcamMngId = %d\n",
                                         vTcamMngId);

            rc = cpssDxChVirtualTcamManagerDelete(vTcamMngId);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamManagerDelete failed for vTcamMngId = %d\n",
                                         vTcamMngId);
            isAutoResized = GT_FALSE;
        }
    }

    if (isTestSkipped)
    {
        SKIP_TEST_MAC;
    }
}

UTF_TEST_CASE_MAC(cpssDxChVirtualTcam_column_fragmentation_one_block)
{
    GT_STATUS                                  st   = GT_OK;
    GT_U8                                      dev;
    GT_U32                                     numOfDevs = 0;
    GT_U32                                     notAppFamilyBmp = 0;
    GT_U32                                     utfFamilyBit = 0;
    GT_BOOL                                    isTestSkipped = GT_TRUE;
    GT_U32                                     vTcamMngId;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC            vTcamInfo;
    GT_U32                                     sequenceSize;
    GT_U32                                     idx;
    GT_U32                                     totalRulesAllowed;

    static VT_SEQUENCE sequence[] =
    {
        /* Fill: column 5 busy, columns 0..4 free */
        {VT_CREATE,      1, CG_PCL,  RS_50,    256, GT_OK},
        {VT_CREATE,      2, CG_PCL,  RS_50,      1, GT_FULL},
        {VT_CREATE,     15, CG_PCL,  RS_10,    252, GT_OK}, /*256 - (256 % 6) */
        {VT_STAMP_FILL, 15, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_STAMP_CHECK,15, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_CREATE,      2, CG_PCL,  RS_10,      1, GT_FULL},
        {VT_REMOVE,      1, CG_NONE, RS_NONE,    0, GT_OK},
        /* reuse free space */
        /* allocate RS_50 again and remove */
        {VT_CREATE,      1, CG_PCL,  RS_50,    256, GT_OK},
        {VT_REMOVE,      1, CG_NONE, RS_NONE,    0, GT_OK},
        /* 6 * (50 free + 10 used) => (1 * (6 * 10 used)) + (5 * 60 free) */
        /* (42 bundles) - 210 full free rows */
        {VT_CREATE,      1, CG_PCL,  RS_60,    210, GT_OK},
        {VT_STAMP_CHECK,15, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      1, CG_NONE, RS_NONE,    0, GT_OK},
        /* (1 * (6 * 10 used) + 5 free_rows) => 6 * (50 free + 10 used) */
        /* allocate RS_50 again and remove */
        {VT_CREATE,      1, CG_PCL,  RS_50,    256, GT_OK},
        {VT_STAMP_CHECK,15, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      1, CG_NONE, RS_NONE,    0, GT_OK},
        /* 2 * (50 free + 10 used) => (40 free + (2 * 10 used)) + (1 * free_row) */
        /* (128 bundles) and full reminder by 20-bytes */
        {VT_CREATE,      1, CG_PCL,  RS_40,     256, GT_OK},
        {VT_STAMP_CHECK,15, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_CREATE,      2, CG_PCL,  RS_20,     129, GT_OK}, /* (129 % 3) == 0 */
        {VT_STAMP_CHECK,15, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      1, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      2, CG_NONE, RS_NONE,    0, GT_OK},
        /* free all TCAM */
        {VT_REMOVE,      15, CG_NONE, RS_NONE,    0, GT_OK},

        /* Fill: columns 4..5 busy (20-byte), columns 0..3 free */
        {VT_CREATE,      1, CG_PCL,  RS_40,    256, GT_OK},
        {VT_CREATE,      2, CG_PCL,  RS_40,      1, GT_FULL},
        {VT_CREATE,     15, CG_PCL,  RS_20,    255, GT_OK}, /* (255 % 3) == 0 */
        {VT_CREATE,      2, CG_PCL,  RS_10,      1, GT_FULL},
        {VT_REMOVE,      1, CG_NONE, RS_NONE,    0, GT_OK},
        /* reuse free space */
        /* allocate RS_40 again and remove */
        {VT_CREATE,      1, CG_PCL,  RS_40,    256, GT_OK},
        {VT_STAMP_CHECK,15, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      1, CG_NONE, RS_NONE,    0, GT_OK},
        /* 3 * (40 free + 20 used) => (1 * (3 * 20 used)) + (2 * 60 free) */
        /* 85 bundles - 170 full free rows */
        {VT_CREATE,      1, CG_PCL,  RS_60,    170, GT_OK},
        {VT_STAMP_CHECK,15, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      1, CG_NONE, RS_NONE,    0, GT_OK},
        /* (1 * (3 * 20 used) + 2 free_rows) => 3 * (40 free + (1 * 20 used)) */
        /* allocate RS_40 again and remove */
        {VT_CREATE,      1, CG_PCL,  RS_40,    256, GT_OK},
        {VT_STAMP_CHECK,15, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      1, CG_NONE, RS_NONE,    0, GT_OK},
        /* free all TCAM */
        {VT_REMOVE,      15, CG_NONE, RS_NONE,    0, GT_OK},

        /* Fill: columns 4..5 busy (2 * 10-byte), columns 0..3 free */
        {VT_CREATE,      1, CG_PCL,  RS_40,    256, GT_OK},
        {VT_CREATE,      2, CG_PCL,  RS_40,      1, GT_FULL},
        {VT_CREATE,     15, CG_PCL,  RS_10,    510, GT_OK},
        {VT_STAMP_FILL, 15, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_CREATE,      2, CG_PCL,  RS_10,      1, GT_FULL},
        {VT_REMOVE,      1, CG_NONE, RS_NONE,    0, GT_OK},
        /* reuse free space */
        /* allocate RS_40 again and remove */
        {VT_CREATE,      1, CG_PCL,  RS_40,    256, GT_OK},
        {VT_REMOVE,      1, CG_NONE, RS_NONE,    0, GT_OK},
        /* 6 * (40 free + (2 * 10 used)) => (1 * (6 * 10)) + (5 * (50 free + 10 used)) */
        {VT_CREATE,      1, CG_PCL,  RS_50,    204, GT_OK},
        {VT_STAMP_CHECK,15, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      1, CG_NONE, RS_NONE,    0, GT_OK},
        /* 2 * (50 free + 10 used) => (40 free + (2 * 10 used)) + (1 * free_row) */
        /* allocate RS_40 again and remove */
        {VT_CREATE,      1, CG_PCL,  RS_40,    256, GT_OK},
        {VT_STAMP_CHECK,15, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      1, CG_NONE, RS_NONE,    0, GT_OK},
        /* 3 * (40 free + (2 * 10 used)) => (1 * (6 * 10 used)) + (2 * 60 free) */
        {VT_CREATE,      1, CG_PCL,  RS_60,    170, GT_OK},
        {VT_STAMP_CHECK,15, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      1, CG_NONE, RS_NONE,    0, GT_OK},
        /* (1 * (6 * 10 used) + 2 free_rows) => 3 * (40 free + (2 * 10 used)) */
        /* allocate RS_40 again and remove */
        {VT_CREATE,      1, CG_PCL,  RS_40,    256, GT_OK},
        {VT_STAMP_CHECK,15, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      1, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,     15, CG_NONE, RS_NONE,    0, GT_OK},
        /*----------------------------------------------------*/
        /* text using mixed holes 2*40 + 2*50 ==> 3*60 */
        {VT_CREATE,      1, CG_PCL,  RS_60,    252, GT_OK},
        {VT_CREATE,      2, CG_PCL,  RS_50,      2, GT_OK},
        {VT_CREATE,      3, CG_PCL,  RS_40,      2, GT_OK},
        {VT_CREATE,     15, CG_PCL,  RS_10,      6, GT_OK},
        {VT_STAMP_FILL, 15, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_STAMP_CHECK,15, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      2, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      3, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_CREATE,      4, CG_PCL,  RS_60,      3, GT_OK},
        {VT_STAMP_CHECK,15, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      4, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,     15, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      1, CG_NONE, RS_NONE,    0, GT_OK},
        /* text using mixed holes 1*40 + 4*50 ==> 4*60 */
        {VT_CREATE,      1, CG_PCL,  RS_60,    251, GT_OK},
        {VT_CREATE,      2, CG_PCL,  RS_50,      4, GT_OK},
        {VT_CREATE,      3, CG_PCL,  RS_40,      1, GT_OK},
        {VT_CREATE,     15, CG_PCL,  RS_10,      6, GT_OK},
        {VT_STAMP_FILL, 15, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_STAMP_CHECK,15, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      2, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      3, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_CREATE,      4, CG_PCL,  RS_60,      4, GT_OK},
        {VT_STAMP_CHECK,15, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      4, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,     15, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      1, CG_NONE, RS_NONE,    0, GT_OK},
        /*-----------------------------------------------*/
        /* pulling narrow vTcam to narrow free column    */
        /* drugging 20-byte rules vTcam (4)              */
        {VT_CREATE,      1, CG_PCL,  RS_40,   120, GT_OK}, /* fill rows 0-119   columns 0-3 */
        {VT_CREATE,      2, CG_PCL,  RS_60,   136, GT_OK}, /* fill rows 120-255 columns all */
        {VT_CREATE,      3, CG_PCL,  RS_20,   120, GT_OK}, /* fill rows 0-119   columns 4-5 */
        {VT_REMOVE,      2, CG_NONE, RS_NONE,    0, GT_OK},/* free rows 120-255 columns all */
        {VT_CREATE,      4, CG_PCL,  RS_20,   120, GT_OK}, /* fill rows 120-179 columns all */
        {VT_STAMP_FILL,  4, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      3, CG_NONE, RS_NONE,    0, GT_OK},/* free rows 0-119   columns 4-5 */
        {VT_CREATE,      5, CG_PCL,  RS_60,   136, GT_OK}, /* fill rows 120-255 columns all */
        {VT_STAMP_CHECK, 4, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      1, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      4, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      5, CG_NONE, RS_NONE,    0, GT_OK},
        /* drugging 10-byte rules vTcam (4) to 2-columns space */
        {VT_CREATE,      1, CG_PCL,  RS_40,   120, GT_OK}, /* fill rows 0-119   columns 0-3 */
        {VT_CREATE,      2, CG_PCL,  RS_60,   136, GT_OK}, /* fill rows 120-255 columns all */
        {VT_CREATE,      3, CG_PCL,  RS_20,   120, GT_OK}, /* fill rows 0-119   columns 4-5 */
        {VT_REMOVE,      2, CG_NONE, RS_NONE,    0, GT_OK},/* free rows 120-255 columns all */
        {VT_CREATE,      4, CG_PCL,  RS_10,   240, GT_OK}, /* fill rows 120-179 columns all */
        {VT_STAMP_FILL,  4, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      3, CG_NONE, RS_NONE,    0, GT_OK},/* free rows 0-119   columns 4-5 */
        {VT_CREATE,      5, CG_PCL,  RS_60,   136, GT_OK}, /* fill rows 120-255 columns all */
        {VT_STAMP_CHECK, 4, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      1, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      4, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      5, CG_NONE, RS_NONE,    0, GT_OK},
        /* drugging 10-byte rules vTcam (4) to 1-columns space */
        {VT_CREATE,      1, CG_PCL,  RS_50,   120, GT_OK}, /* fill rows 0-119   columns 0-4 */
        {VT_CREATE,      2, CG_PCL,  RS_60,   136, GT_OK}, /* fill rows 120-255 columns all */
        {VT_CREATE,      3, CG_PCL,  RS_10,   120, GT_OK}, /* fill rows 0-119   columns 5  */
        {VT_REMOVE,      2, CG_NONE, RS_NONE,    0, GT_OK},/* free rows 120-255 columns all */
        {VT_CREATE,      4, CG_PCL,  RS_10,   120, GT_OK}, /* fill rows 120-149 columns all */
        {VT_STAMP_FILL,  4, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      3, CG_NONE, RS_NONE,    0, GT_OK},/* free rows 0-119   columns 5   */
        {VT_CREATE,      5, CG_PCL,  RS_60,   136, GT_OK}, /* fill rows 120-255 columns all */
        {VT_STAMP_CHECK, 4, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      1, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      4, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      5, CG_NONE, RS_NONE,    0, GT_OK},
    };


    /* skip any case, prevent compiler warnings */
     /*numOfDevs = 0; if (numOfDevs == 0) SKIP_TEST_MAC;*/

    sequenceSize = (sizeof(sequence) / sizeof(sequence[0]));

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* 1. Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
              devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }
        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;

        vTcamMngId = 1;
        cpssOsMemSet(&vTcamInfo, 0, sizeof(vTcamInfo));
        vTcamInfo.hitNumber     = 0;
        vTcamInfo.autoResize    = GT_FALSE;
        vTcamInfo.ruleAdditionMethod =
            CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E;

        /* Create vTCAM manager */
        st = cpssDxChVirtualTcamManagerCreate(vTcamMngId);
            if (st == GT_ALREADY_EXIST)
                st = GT_OK;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerCreate failed\n");

        st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        if (st != GT_OK)
        {
            cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
            st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        }
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListAdd failed\n");

        /* create vTcam 100 occuping all blocks beside one to TTI */
        vTcamInfo.clientGroup = CG_TTI;
        vTcamInfo.ruleSize    = RS_10;

        internal_ut_cpssDxChVirtualTcamTotalRulesAllowedPerDevGet(
                           devListArr[0],
                           CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_60_B_E,
                           &totalRulesAllowed);

        if (totalRulesAllowed > NUM_60_RULES_PER_BLOCK) /*shrink the TCAM to the 2 blocks */
        {
            vTcamInfo.guaranteedNumOfRules = (totalRulesAllowed - NUM_60_RULES_PER_BLOCK)*6;
        }

        st = ut_cpssDxChVirtualTcamCreate(vTcamMngId, 100 /*vTcamId*/, &vTcamInfo);
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC(
            GT_FAIL, st, "ut_cpssDxChVirtualTcamCreate failed\n");

        for (idx = 0; (idx < sequenceSize); idx++)
        {
            st = vtcamSequenceEntryProcess(sequence, idx, vTcamMngId, &vTcamInfo);
            if (sequence[idx].operation == VT_CREATE)
            {
                PRV_UTF_LOG4_MAC(
                    "Created vTcam %d ruleSize %d rulesAmount %d clientGroup %d\n",
                    sequence[idx].vTcamId, (sequence[idx].ruleSize + 1),
                    sequence[idx].rulesAmount, sequence[idx].clientGroup);
            }
            if (sequence[idx].operation == VT_REMOVE)
            {
                PRV_UTF_LOG1_MAC("Removed vTcam %d\n", sequence[idx].vTcamId);
            }
            if (st != GT_OK) PRV_UTF_LOG0_MAC("Operation Failed\n");

            UTF_VERIFY_EQUAL1_STRING_MAC(
                sequence[idx].rc, st,
                "vtcamSequenceEntryProcess failed, index: %d\n", idx);
        }

        st = cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st, "cpssDxChVirtualTcamManagerDevListRemove failed for vTcamMngId = %d\n",
            vTcamMngId);
        st = cpssDxChVirtualTcamManagerDelete(vTcamMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDelete failed for vTcamMngId = %d\n",
                                     vTcamMngId);
    }

    if (isTestSkipped)
    {
        SKIP_TEST_MAC;
    }
}

UTF_TEST_CASE_MAC(cpssDxChVirtualTcam_column_fragmentation_two_blocks)
{
    GT_STATUS                                  st   = GT_OK;
    GT_U8                                      dev;
    GT_U32                                     numOfDevs = 0;
    GT_U32                                     notAppFamilyBmp = 0;
    GT_U32                                     utfFamilyBit = 0;
    GT_BOOL                                    isTestSkipped = GT_TRUE;
    GT_U32                                     vTcamMngId;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC            vTcamInfo;
    GT_U32                                     sequenceSize;
    GT_U32                                     idx;
    GT_U32                                     totalRulesAllowed;

    static VT_SEQUENCE sequence[] =
    {
        /* Fill: column 5 busy, columns 0..4 free */
        {VT_TITLE,       0, CG_NONE, RS_NONE,    0, GT_OK, "Primary 512 * 50 + 510 * 10"},
        {VT_CREATE,      1, CG_PCL,  RS_50,    512, GT_OK},
        {VT_CREATE,      2, CG_PCL,  RS_50,      1, GT_FULL},
        {VT_CREATE,     15, CG_PCL,  RS_10,    510, GT_OK}, /*510 - (510 % 6) */
        {VT_STAMP_FILL, 15, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_STAMP_CHECK,15, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_CREATE,      2, CG_PCL,  RS_10,      1, GT_FULL},
        {VT_REMOVE,      1, CG_NONE, RS_NONE,    0, GT_OK},
        /* reuse free space */
        /* allocate RS_50 again and remove */
        {VT_TITLE,       0, CG_NONE, RS_NONE,    0, GT_OK, "Keep 510 * 10 Remove an Add again 512 * 50"},
        {VT_CREATE,      1, CG_PCL,  RS_50,    512, GT_OK},
        {VT_REMOVE,      1, CG_NONE, RS_NONE,    0, GT_OK},
        /* 6 * (50 free + 10 used) => (1 * (6 * 10 used)) + (5 * 60 free) */
        /* (85 bundles) - 425 full free rows */

        {VT_TITLE,       0, CG_NONE, RS_NONE,    0, GT_OK, "Convert 512 * 50 to 425 * 60"},
        {VT_CREATE,      1, CG_PCL,  RS_60,    425, GT_OK},
        {VT_STAMP_CHECK,15, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      1, CG_NONE, RS_NONE,    0, GT_OK},
        /* (1 * (6 * 10 used) + 5 free_rows) => 6 * (50 free + 10 used) */
        /* allocate RS_50 again and remove */
        {VT_TITLE,       0, CG_NONE, RS_NONE,    0, GT_OK, "Convert 425 * 60 to 512 * 50"},
        {VT_CREATE,      1, CG_PCL,  RS_50,    512, GT_OK},
        {VT_STAMP_CHECK,15, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      1, CG_NONE, RS_NONE,    0, GT_OK},
        /* 2 * (50 free + 10 used) => (40 free + (2 * 10 used)) + (1 * free_row) */
        /* (128 bundles) and full reminder by 20-bytes */
        {VT_TITLE,       0, CG_NONE, RS_NONE,    0, GT_OK, "Convert 512 * 50 to (512 * 40 + 255 * 20)"},
        {VT_CREATE,      1, CG_PCL,  RS_40,     512, GT_OK},
        {VT_STAMP_CHECK,15, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_CREATE,      2, CG_PCL,  RS_20,     255, GT_OK}, /* (255 % 3) == 0 */
        {VT_STAMP_CHECK,15, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      1, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      2, CG_NONE, RS_NONE,    0, GT_OK},
        /* free all TCAM */
        {VT_REMOVE,      15, CG_NONE, RS_NONE,    0, GT_OK},

        /* Fill: columns 4..5 busy (20-byte), columns 0..3 free */
        {VT_TITLE,       0, CG_NONE, RS_NONE,    0, GT_OK, "Primary 512 * 40 + 510 * 20"},
        {VT_CREATE,      1, CG_PCL,  RS_40,    512, GT_OK},
        {VT_CREATE,      2, CG_PCL,  RS_40,      1, GT_FULL},
        {VT_CREATE,     15, CG_PCL,  RS_20,    510, GT_OK}, /* (510 % 3) == 0 */
        {VT_CREATE,      2, CG_PCL,  RS_10,      1, GT_FULL},
        {VT_REMOVE,      1, CG_NONE, RS_NONE,    0, GT_OK},
        /* reuse free space */
        /* allocate RS_40 again and remove */
        {VT_TITLE,       0, CG_NONE, RS_NONE,    0, GT_OK, "Keep 510 * 20 Remove an Add again 512 * 40"},
        {VT_CREATE,      1, CG_PCL,  RS_40,    512, GT_OK},
        {VT_STAMP_CHECK,15, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      1, CG_NONE, RS_NONE,    0, GT_OK},
        /* 3 * (40 free + 20 used) => (1 * (3 * 20 used)) + (2 * 60 free) */
        /* 85 bundles - 340 full free rows */
        {VT_TITLE,       0, CG_NONE, RS_NONE,    0, GT_OK, "Convert 512 * 40 to 342 * 60"},
        {VT_CREATE,      1, CG_PCL,  RS_60,    342, GT_OK},
        {VT_STAMP_CHECK,15, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      1, CG_NONE, RS_NONE,    0, GT_OK},
        /* (1 * (3 * 20 used) + 2 free_rows) => 3 * (40 free + (1 * 20 used)) */
        /* allocate RS_40 again and remove */
        {VT_TITLE,       0, CG_NONE, RS_NONE,    0, GT_OK, "Convert 342 * 60 to 512 * 40"},
        {VT_CREATE,      1, CG_PCL,  RS_40,    510, GT_OK},
        {VT_STAMP_CHECK,15, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      1, CG_NONE, RS_NONE,    0, GT_OK},
        /* free all TCAM */
        {VT_REMOVE,      15, CG_NONE, RS_NONE,    0, GT_OK},

        /* Fill: columns 4..5 busy (2 * 10-byte), columns 0..3 free */
        {VT_TITLE,       0, CG_NONE, RS_NONE,    0, GT_OK, "Primary 512 * 40 + 1020 * 10"},
        {VT_CREATE,      1, CG_PCL,  RS_40,    512, GT_OK},
        {VT_CREATE,      2, CG_PCL,  RS_40,      1, GT_FULL},
        {VT_CREATE,     15, CG_PCL,  RS_10,   1020, GT_OK},
        {VT_STAMP_FILL, 15, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_CREATE,      2, CG_PCL,  RS_10,      1, GT_FULL},
        {VT_REMOVE,      1, CG_NONE, RS_NONE,    0, GT_OK},
        /* reuse free space */
        /* allocate RS_40 again and remove */
        {VT_TITLE,       0, CG_NONE, RS_NONE,    0, GT_OK, "Keep 1020 * 10 Remove an Add again 512 * 40"},
        {VT_CREATE,      1, CG_PCL,  RS_40,    512, GT_OK},
        {VT_REMOVE,      1, CG_NONE, RS_NONE,    0, GT_OK},
        /* 6 * (40 free + (2 * 10 used)) => (1 * (6 * 10)) + (5 * (50 free + 10 used)) */
        {VT_TITLE,       0, CG_NONE, RS_NONE,    0, GT_OK, "Convert 512 * 40 to 410 * 50"},
        {VT_CREATE,      1, CG_PCL,  RS_50,    410, GT_OK},
        {VT_STAMP_CHECK,15, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      1, CG_NONE, RS_NONE,    0, GT_OK},
        /* 2 * (50 free + 10 used) => (40 free + (2 * 10 used)) + (1 * free_row) */
        /* allocate RS_40 again and remove */
        {VT_TITLE,       0, CG_NONE, RS_NONE,    0, GT_OK, "Convert 410 * 50 to 512 * 40"},
        {VT_CREATE,      1, CG_PCL,  RS_40,    512, GT_OK},
        {VT_STAMP_CHECK,15, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      1, CG_NONE, RS_NONE,    0, GT_OK},
        /* 3 * (40 free + (2 * 10 used)) => (1 * (6 * 10 used)) + (2 * 60 free) */
        {VT_TITLE,       0, CG_NONE, RS_NONE,    0, GT_OK, "Convert 512 * 40 to 342 * 60"},
        {VT_CREATE,      1, CG_PCL,  RS_60,    342, GT_OK},
        {VT_STAMP_CHECK,15, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      1, CG_NONE, RS_NONE,    0, GT_OK},
        /* (1 * (6 * 10 used) + 2 free_rows) => 3 * (40 free + (2 * 10 used)) */
        /* allocate RS_40 again and remove */
        {VT_TITLE,       0, CG_NONE, RS_NONE,    0, GT_OK, "Convert 342 * 60 to 512 * 40"},
        {VT_CREATE,      1, CG_PCL,  RS_40,    512, GT_OK},
        {VT_STAMP_CHECK,15, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      1, CG_NONE, RS_NONE,    0, GT_OK}
    };


    /* skip any case, prevent compiler warnings */
     /*numOfDevs = 0; if (numOfDevs == 0) SKIP_TEST_MAC;*/

    sequenceSize = (sizeof(sequence) / sizeof(sequence[0]));

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* 1. Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
              devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }
        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;

        vTcamMngId = 1;
        cpssOsMemSet(&vTcamInfo, 0, sizeof(vTcamInfo));
        vTcamInfo.hitNumber     = 0;
        vTcamInfo.autoResize    = GT_FALSE;
        vTcamInfo.ruleAdditionMethod =
            CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E;

        /* Create vTCAM manager */
        st = cpssDxChVirtualTcamManagerCreate(vTcamMngId);
            if (st == GT_ALREADY_EXIST)
                st = GT_OK;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerCreate failed\n");

        st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        if (st != GT_OK)
        {
            cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
            st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        }
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListAdd failed\n");

        /* create vTcam 100 occuping all blocks beside one to TTI */
        vTcamInfo.clientGroup = CG_TTI;
        vTcamInfo.ruleSize    = RS_10;

        internal_ut_cpssDxChVirtualTcamTotalRulesAllowedPerDevGet(
                           devListArr[0],
                           CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_60_B_E,
                           &totalRulesAllowed);

        if (totalRulesAllowed > NUM_60_RULES_PER_BLOCK) /*shrink the TCAM to the 2 blocks */
        {
            vTcamInfo.guaranteedNumOfRules =
                    (totalRulesAllowed - NUM_60_RULES_PER_BLOCK *2 ) * 6;
        }

        st = ut_cpssDxChVirtualTcamCreate(vTcamMngId, 100 /*vTcamId*/, &vTcamInfo);
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC(
            GT_FAIL, st, "ut_cpssDxChVirtualTcamCreate failed\n");

        for (idx = 0; (idx < sequenceSize); idx++)
        {
            st = vtcamSequenceEntryProcess(sequence, idx, vTcamMngId, &vTcamInfo);
            if (sequence[idx].operation == VT_CREATE)
            {
                PRV_UTF_LOG4_MAC(
                    "Created vTcam %d ruleSize %d rulesAmount %d clientGroup %d\n",
                    sequence[idx].vTcamId, (sequence[idx].ruleSize + 1),
                    sequence[idx].rulesAmount, sequence[idx].clientGroup);
            }
            if (sequence[idx].operation == VT_REMOVE)
            {
                PRV_UTF_LOG1_MAC("Removed vTcam %d\n", sequence[idx].vTcamId);
            }
            if (st != GT_OK) PRV_UTF_LOG0_MAC("Operation Failed\n");

            UTF_VERIFY_EQUAL1_STRING_MAC(
                sequence[idx].rc, st,
                "vtcamSequenceEntryProcess failed, index: %d\n", idx);
        }

        st = cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st, "cpssDxChVirtualTcamManagerDevListRemove failed for vTcamMngId = %d\n",
            vTcamMngId);
        st = cpssDxChVirtualTcamManagerDelete(vTcamMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDelete failed for vTcamMngId = %d\n",
                                     vTcamMngId);
    }

    if (isTestSkipped)
    {
        SKIP_TEST_MAC;
    }
}

UTF_TEST_CASE_MAC(cpssDxChVirtualTcam_block_fragmentation)
{
    GT_STATUS                                  st   = GT_OK;
    GT_U8                                      dev;
    GT_U32                                     numOfDevs = 0;
    GT_U32                                     notAppFamilyBmp = 0;
    GT_U32                                     utfFamilyBit = 0;
    GT_BOOL                                    isTestSkipped = GT_TRUE;
    GT_U32                                     vTcamMngId;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC            vTcamInfo;
    GT_U32                                     sequenceSize;
    GT_U32                                     idx;
    GT_U32                                     totalRulesAllowed;

    static VT_SEQUENCE sequence_block_frag[] =
    {
        /* Fill all blocks except 2 on client PCL */
        {VT_CREATE,      1, CG_PCL,  RS_60,    256*22, GT_OK},
        {VT_STAMP_FILL,  1, CG_PCL,  RS_60,    256*22, GT_OK},
        /* Fill remaining 2 blocks on client TTI */
        {VT_CREATE,      2, CG_TTI,  RS_30,      2, GT_OK},
        {VT_STAMP_FILL,  2, CG_TTI,  RS_30,      2, GT_OK},
        {VT_CREATE,      3, CG_TTI,  RS_30,    256*2, GT_OK},
        {VT_CREATE,      4, CG_TTI,  RS_30,    256*2-2, GT_OK},
        {VT_STAMP_FILL,  4, CG_TTI,  RS_30,    256*2-2, GT_OK},
        /* Check the TCAM is full */
        {VT_CREATE,     52, CG_PCL,  RS_10,      1, GT_FULL},
        {VT_CREATE,     53, CG_TTI,  RS_10,      1, GT_FULL},
        /* Remove space equivalent to 1 block - but removed space is actually spanning both blocks */
        {VT_REMOVE,      3, CG_NONE, RS_NONE,    0, GT_OK},
        /* 1. Check that both blocks could be compressed and reused on the other client PCL */
        {VT_CREATE,      5, CG_PCL,  RS_60,    256, GT_OK},
        {VT_STAMP_FILL,  5, CG_PCL,  RS_60,    256, GT_OK},
        /* Check the TCAM is full again */
        {VT_CREATE,     54, CG_PCL,  RS_10,      1, GT_FULL},
        {VT_CREATE,     55, CG_TTI,  RS_10,      1, GT_FULL},
        /* Check every rule has moved to its correct place */
        {VT_STAMP_CHECK,  1, CG_PCL,  RS_60,    256*22, GT_OK},
        {VT_STAMP_CHECK,  2, CG_TTI,  RS_30,      2, GT_OK},
        {VT_STAMP_CHECK,  4, CG_TTI,  RS_30,    256*2-2, GT_OK},
        {VT_STAMP_CHECK,  5, CG_PCL,  RS_60,    256, GT_OK},

        /* Cleanup stuff in order to prepare next test stage */
        {VT_REMOVE,      2, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      4, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      5, CG_NONE, RS_NONE,    0, GT_OK},
        /* Fill remaining 2 blocks on client TTI */
        {VT_CREATE,      2, CG_TTI,  RS_30,      2, GT_OK},
        {VT_STAMP_FILL,  2, CG_TTI,  RS_30,      2, GT_OK},
        {VT_CREATE,      3, CG_TTI,  RS_30,    256*2, GT_OK},
        {VT_CREATE,      4, CG_TTI,  RS_30,    256*2-2, GT_OK},
        {VT_STAMP_FILL,  4, CG_TTI,  RS_30,    256*2-2, GT_OK},
        /* Check the TCAM is full */
        {VT_CREATE,     52, CG_PCL,  RS_10,      1, GT_FULL},
        {VT_CREATE,     53, CG_TTI,  RS_10,      1, GT_FULL},
        /* Remove space equivalent to 1 block - but removed space is actually spanning both blocks */
        {VT_REMOVE,      3, CG_NONE, RS_NONE,    0, GT_OK},
        /* 2. Increase existing TCAM, check that both blocks could be compressed and reused on the other client PCL */
        {VT_INCREASE,    1, CG_PCL,  RS_60,    256, GT_OK},
        {VT_STAMP_FILL,  1, CG_PCL,  RS_60,    256, GT_OK},
        /* Check the TCAM is full again */
        {VT_CREATE,     54, CG_PCL,  RS_10,      1, GT_FULL},
        {VT_CREATE,     55, CG_TTI,  RS_10,      1, GT_FULL},
        /* Check every rule has moved to its correct place */
        {VT_STAMP_CHECK,  1, CG_PCL,  RS_60,    (GT_U32)-1, GT_OK}, /* Special handling*/
        {VT_STAMP_CHECK,  2, CG_TTI,  RS_30,      2, GT_OK},
        {VT_STAMP_CHECK,  4, CG_TTI,  RS_30,    256*2-2, GT_OK},

        /* Cleanup everything */
        {VT_REMOVE,      1, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      2, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      4, CG_NONE, RS_NONE,    0, GT_OK},
    };


    sequenceSize = (sizeof(sequence_block_frag) / sizeof(sequence_block_frag[0]));

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* 1. Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
              devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }
        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;


        /* Create vTCAM manager */
        vTcamMngId = 1;
        st = cpssDxChVirtualTcamManagerCreate(vTcamMngId);
            if (st == GT_ALREADY_EXIST)
                st = GT_OK;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerCreate failed\n");

        st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        if (st != GT_OK)
        {
            cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
            st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        }
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListAdd failed\n");

        /* Adjust allocation size according to device architecture */
        internal_ut_cpssDxChVirtualTcamTotalRulesAllowedPerDevGet(
                           devListArr[0],
                           CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_60_B_E,
                           &totalRulesAllowed);

        if (totalRulesAllowed > NUM_60_RULES_PER_BLOCK) /*shrink the TCAM to the 2 blocks */
        {
            sequence_block_frag[0].rulesAmount =
                    totalRulesAllowed - NUM_60_RULES_PER_BLOCK*2;
        }

        cpssOsMemSet(&vTcamInfo, 0, sizeof(vTcamInfo));
        vTcamInfo.hitNumber     = 0;
        vTcamInfo.autoResize    = GT_FALSE;
        vTcamInfo.ruleAdditionMethod =
            CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E;

        for (idx = 0; (idx < sequenceSize); idx++)
        {
            st = vtcamSequenceEntryProcess(sequence_block_frag, idx, vTcamMngId, &vTcamInfo);
            UTF_VERIFY_EQUAL1_STRING_MAC(
                sequence_block_frag[idx].rc, st,
                "vtcamSequenceEntryProcess failed, index: %d\n", idx);
        }

        st = cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st, "cpssDxChVirtualTcamManagerDevListRemove failed for vTcamMngId = %d\n",
            vTcamMngId);
        st = cpssDxChVirtualTcamManagerDelete(vTcamMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDelete failed for vTcamMngId = %d\n",
                                     vTcamMngId);
    }

    if (isTestSkipped)
    {
        SKIP_TEST_MAC;
    }
}

UTF_TEST_CASE_MAC(cpssDxChVirtualTcam_block_fragmentation_autoresize)
{
    GT_STATUS                                  st   = GT_OK;
    GT_U8                                      dev;
    GT_U32                                     numOfDevs = 0;
    GT_U32                                     notAppFamilyBmp = 0;
    GT_U32                                     utfFamilyBit = 0;
    GT_BOOL                                    isTestSkipped = GT_TRUE;
    GT_U32                                     vTcamMngId;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC            vTcamInfo;
    GT_U32                                     sequenceSize;
    GT_U32                                     idx;
    GT_U32                                     totalRulesAllowed;
    GT_BOOL                                    savePriorityMode;

    static VT_SEQUENCE sequence_block_frag[] =
    {
        /* Fill all blocks except 2 on client PCL */
        {VT_CREATE,      9, CG_PCL,  RS_60,    256*21, GT_OK},
        {VT_CREATE,      1, CG_PCL,  RS_60,       256, GT_OK},
        {VT_STAMP_FILL,  1, CG_PCL,  RS_60,       256, GT_OK},
        /* Fill remaining 2 blocks on client TTI */
        {VT_CREATE,      2, CG_TTI,  RS_30,      2, GT_OK},
        {VT_STAMP_FILL,  2, CG_TTI,  RS_30,      2, GT_OK},
        {VT_CREATE,      3, CG_TTI,  RS_30,    256*2, GT_OK},
        {VT_CREATE,      4, CG_TTI,  RS_30,    256*2-2, GT_OK},
        {VT_STAMP_FILL,  4, CG_TTI,  RS_30,    256*2-2, GT_OK},
        /* Check the TCAM is full */
        {VT_CREATE,     52, CG_PCL,  RS_10,      1, GT_FULL},
        {VT_CREATE,     53, CG_TTI,  RS_10,      1, GT_FULL},
        /* Remove space equivalent to 1 block - but removed space is actually spanning both blocks */
        {VT_REMOVE,      3, CG_NONE, RS_NONE,    0, GT_OK},
        /* 2. Increase (by auto resize) existing TCAM, check that both blocks could be compressed and reused on the other client PCL */
        {VT_STAMP_FILL,  1, CG_PCL,  RS_60,    (GT_U32)-1, GT_OK}, /* Special handling*/
        /* Check the TCAM is full again */
        {VT_CREATE,     54, CG_PCL,  RS_10,      (6*(256%12))+1, GT_FULL},
        {VT_CREATE,     55, CG_TTI,  RS_10,      1, GT_FULL},
        /* Check every rule has moved to its correct place */
        {VT_STAMP_CHECK,  1, CG_PCL,  RS_60,      0, GT_OK},
        {VT_STAMP_CHECK,  2, CG_TTI,  RS_30,      2, GT_OK},
        {VT_STAMP_CHECK,  4, CG_TTI,  RS_30,    256*2-2, GT_OK},

        /* Cleanup everything */
        {VT_REMOVE,      1, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      2, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      4, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      9, CG_NONE, RS_NONE,    0, GT_OK},
    };


    sequenceSize = (sizeof(sequence_block_frag) / sizeof(sequence_block_frag[0]));

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* 1. Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
              devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }
        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;

        /* This test is priority mode test */
        savePriorityMode = priorityMode;
        priorityMode = GT_TRUE;

        /* Create vTCAM manager */
        vTcamMngId = 1;
        st = cpssDxChVirtualTcamManagerCreate(vTcamMngId);
            if (st == GT_ALREADY_EXIST)
                st = GT_OK;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerCreate failed\n");

        st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        if (st != GT_OK)
        {
            cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
            st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        }
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListAdd failed\n");

        /* Adjust allocation size according to device architecture */
        internal_ut_cpssDxChVirtualTcamTotalRulesAllowedPerDevGet(
                           devListArr[0],
                           CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_60_B_E,
                           &totalRulesAllowed);

        if (totalRulesAllowed > NUM_60_RULES_PER_BLOCK * 3) /*shrink the TCAM to the 3 blocks */
        {
            sequence_block_frag[0].rulesAmount =
                    totalRulesAllowed - ( NUM_60_RULES_PER_BLOCK * 3);
        }
        cpssOsMemSet(&vTcamInfo, 0, sizeof(vTcamInfo));
        vTcamInfo.hitNumber     = 0;
        vTcamInfo.autoResize    = GT_TRUE;
        vTcamInfo.ruleAdditionMethod =
            CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E;

        for (idx = 0; (idx < sequenceSize); idx++)
        {
            st = vtcamSequenceEntryProcess(sequence_block_frag, idx, vTcamMngId, &vTcamInfo);
            UTF_VERIFY_EQUAL1_STRING_MAC(
                sequence_block_frag[idx].rc, st,
                "vtcamSequenceEntryProcess failed, index: %d\n", idx);
        }

        st = cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st, "cpssDxChVirtualTcamManagerDevListRemove failed for vTcamMngId = %d\n",
            vTcamMngId);
        st = cpssDxChVirtualTcamManagerDelete(vTcamMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDelete failed for vTcamMngId = %d\n",
                                     vTcamMngId);

        priorityMode = savePriorityMode;
    }

    if (isTestSkipped)
    {
        SKIP_TEST_MAC;
    }
}

UTF_TEST_CASE_MAC(cpssDxChVirtualTcam_block_fragmentation_columnTypes)
{
    GT_STATUS                                  st   = GT_OK;
    GT_U8                                      dev;
    GT_U32                                     numOfDevs = 0;
    GT_U32                                     notAppFamilyBmp = 0;
    GT_U32                                     utfFamilyBit = 0;
    GT_BOOL                                    isTestSkipped = GT_TRUE;
    GT_U32                                     vTcamMngId;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC            vTcamInfo;
    VT_SEQUENCE                                *sequencePtr;
    GT_U32                                     sequenceSize;
    GT_U32                                     idx;

    static VT_SEQUENCE bc2_create_sequence[] =
    {
        /* Create a number of vTcams - bc2/3 */
        {VT_CREATE,    5, CG_TTI,  RS_20,    1002, GT_OK},
        {VT_CREATE,    6, CG_TTI,  RS_30,    1000, GT_OK},
        {VT_COMPLETE,  7, CG_TTI,  RS_NONE,  1140, GT_OK},
        {VT_CREATE,    8, CG_PCL,  RS_20,    1002, GT_OK},
        {VT_CREATE,    9, CG_PCL,  RS_40,    2000, GT_OK},
        {VT_CREATE,   10, CG_PCL,  RS_50,    1000, GT_OK},
        {VT_CREATE,   11, CG_PCL,  RS_10,    7002, GT_OK},
        {VT_CREATE,   12, CG_PCL,  RS_60,     500, GT_OK},
        {VT_CREATE,   13, CG_PCL,  RS_30,     100, GT_OK},
        {VT_CREATE,   14, CG_PCL,  RS_50,     814, GT_OK},
        {VT_CREATE,   15, CG_PCL,  RS_10,     814, GT_OK},
        {VT_COMPLETE, 16, CG_PCL,  RS_NONE,   528, GT_OK},
        /* Check there is no more space - bc2/3 */
        {VT_CREATE,   50, CG_TTI,  RS_10,       1, GT_FULL},
        {VT_CREATE,   51, CG_PCL,  RS_10,       1, GT_FULL},
        /* Remove some vTcams in order to make space - bc2/3 */
        {VT_REMOVE,    6, CG_NONE, RS_NONE,  1000, GT_OK},
        {VT_REMOVE,    8, CG_NONE, RS_NONE,  1002, GT_OK},
        {VT_REMOVE,   10, CG_NONE, RS_NONE,  1000, GT_OK},
        {VT_REMOVE,   13, CG_NONE, RS_NONE,   100, GT_OK},
        {VT_REMOVE,   15, CG_NONE, RS_NONE,   814, GT_OK},
        /* Sequence of increase operations - bc2/3 */
        {VT_INCREASE,  5, CG_TTI,  RS_20,  996, GT_OK},
        {VT_INCREASE,  7, CG_TTI,  RS_10, 1008, GT_OK},
        {VT_INCREASE,  5, CG_TTI,  RS_20, 3072, GT_OK},     /* block defrag expected here */
        /* Check there is really no more space - bc2/3 */
        {VT_INCREASE,  5, CG_TTI,  RS_20,    1, GT_FULL},
        /* Remove all vTcams in order to finish test - bc2/3 */
        {VT_REMOVE,    5, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,    7, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,    9, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,   11, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,   12, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,   14, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,   16, CG_NONE, RS_NONE,     0, GT_OK},
    };

    /* Sequence below is a subset of bobk_sequence: only part 1 + part 2 up to remove vTcams step */
    static VT_SEQUENCE bobk_create_sequence[] =
    {
        /* Create a number of vTcams - bobk */
        {VT_CREATE,    5, CG_TTI,  RS_20,     501, GT_OK},
        {VT_CREATE,    6, CG_TTI,  RS_30,     500, GT_OK},
        {VT_COMPLETE,  7, CG_TTI,  RS_NONE,   570, GT_OK},
        {VT_CREATE,    8, CG_PCL,  RS_20,     501, GT_OK},
        {VT_CREATE,    9, CG_PCL,  RS_40,    1000, GT_OK},
        {VT_CREATE,   10, CG_PCL,  RS_50,     500, GT_OK},
        {VT_CREATE,   11, CG_PCL,  RS_10,    3501, GT_OK},
        {VT_CREATE,   12, CG_PCL,  RS_60,     250, GT_OK},
        {VT_CREATE,   13, CG_PCL,  RS_30,      50, GT_OK},
        {VT_CREATE,   14, CG_PCL,  RS_50,     407, GT_OK},
        {VT_CREATE,   15, CG_PCL,  RS_10,     407, GT_OK},
        {VT_COMPLETE, 16, CG_PCL,  RS_NONE,   258, GT_OK},
        /* Check there is no more space - bobk */
        {VT_CREATE,   50, CG_TTI,  RS_10,       1, GT_FULL},
        {VT_CREATE,   51, CG_PCL,  RS_10,       1, GT_FULL},
        /* Remove some vTcams in order to make space - bobk */
        {VT_REMOVE,    6, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,    8, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,   10, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,   13, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,   15, CG_NONE, RS_NONE,     0, GT_OK},
        /* Sequence of increase operations - bobk */
        {VT_INCREASE,  5, CG_TTI,  RS_20,  498, GT_OK},
        {VT_INCREASE,  7, CG_TTI,  RS_10,  504, GT_OK},
        {VT_INCREASE,  5, CG_TTI,  RS_20, 1536, GT_OK},     /* block defrag expected here */
        /* Check there is really no more space - bobk */
        {VT_INCREASE,  5, CG_TTI,  RS_20,    1, GT_FULL},
        /* Remove all vTcams in order to finish test - bobk */
        {VT_REMOVE,    5, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,    7, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,    9, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,   11, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,   12, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,   14, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,   16, CG_NONE, RS_NONE,     0, GT_OK},
    };

    static VT_SEQUENCE aldrin_create_sequence[] =
    {
        /* Create a number of vTcams - aldrin */
        {VT_CREATE,    5, CG_TTI,  RS_20,     252, GT_OK},
        {VT_CREATE,    6, CG_TTI,  RS_30,     250, GT_OK},
        {VT_COMPLETE,  7, CG_TTI,  RS_10,     282, GT_OK},
        {VT_CREATE,    8, CG_PCL,  RS_20,     342, GT_OK},
        {VT_CREATE,    9, CG_PCL,  RS_40,     500, GT_OK},
        {VT_CREATE,   10, CG_PCL,  RS_50,     202, GT_OK},
        {VT_CREATE,   11, CG_PCL,  RS_10,    1116, GT_OK},
        {VT_CREATE,   12, CG_PCL,  RS_60,     117, GT_OK},
        {VT_CREATE,   13, CG_PCL,  RS_30,     194, GT_OK},
        {VT_CREATE,   14, CG_PCL,  RS_50,     103, GT_OK},
        {VT_CREATE,   15, CG_PCL,  RS_10,     198, GT_OK},
        {VT_COMPLETE, 16, CG_PCL,  RS_NONE,   870, GT_OK},
        /* Check there is no more space - aldrin */
        {VT_CREATE,   50, CG_TTI,  RS_10,       1, GT_FULL},
        {VT_CREATE,   51, CG_PCL,  RS_10,       1, GT_FULL},
        /* Remove some vTcams in order to make space - aldrin */
        {VT_REMOVE,    6, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,    8, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,   10, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,   13, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,   15, CG_NONE, RS_NONE,     0, GT_OK},
        /* Sequence of increase operations - aldrin */
        {VT_INCREASE,  5, CG_TTI,  RS_20,  258, GT_OK},
        {VT_INCREASE,  7, CG_TTI,  RS_10,  234, GT_OK},
        {VT_INCREASE,  5, CG_TTI,  RS_20,  768, GT_OK},     /* block defrag expected here */
        /* Check there is really no more space - aldrin */
        {VT_INCREASE,  5, CG_TTI,  RS_20,    1, GT_FULL},
        /* Remove all vTcams in order to finish test - aldrin */
        {VT_REMOVE,    5, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,    7, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,    9, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,   11, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,   12, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,   14, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,   16, CG_NONE, RS_NONE,     0, GT_OK},
    };


    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* 1. Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
              devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }
        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;


        /* Create vTCAM manager */
        vTcamMngId = 1;
        st = cpssDxChVirtualTcamManagerCreate(vTcamMngId);
            if (st == GT_ALREADY_EXIST)
                st = GT_OK;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerCreate failed\n");

        st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        if (st != GT_OK)
        {
            cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
            st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        }
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListAdd failed\n");

        /* Adjust allocation size according to device architecture */
        switch (utfFamilyBit)
        {
            case UTF_ALDRIN2_E:
            case UTF_FALCON_E:
            case UTF_CAELUM_E:
                sequencePtr = bobk_create_sequence;
                sequenceSize = (sizeof(bobk_create_sequence) / sizeof(bobk_create_sequence[0]));
                break;

            case UTF_ALDRIN_E:
            case UTF_AC3X_E:
                sequencePtr = aldrin_create_sequence;
                sequenceSize = (sizeof(aldrin_create_sequence) / sizeof(aldrin_create_sequence[0]));
                break;

            case UTF_BOBCAT2_E:
            case UTF_BOBCAT3_E:
            default:
                sequencePtr = bc2_create_sequence;
                sequenceSize = (sizeof(bc2_create_sequence) / sizeof(bc2_create_sequence[0]));
                break;
        }
        cpssOsMemSet(&vTcamInfo, 0, sizeof(vTcamInfo));
        vTcamInfo.hitNumber     = 0;
        vTcamInfo.autoResize    = GT_FALSE;
        vTcamInfo.ruleAdditionMethod =
            CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E;

        for (idx = 0; (idx < sequenceSize); idx++)
        {
            st = vtcamSequenceEntryProcess(sequencePtr, idx, vTcamMngId, &vTcamInfo);
            UTF_VERIFY_EQUAL1_STRING_MAC(
                sequencePtr[idx].rc, st,
                "vtcamSequenceEntryProcess failed, index: %d\n", idx);
        }

        st = cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st, "cpssDxChVirtualTcamManagerDevListRemove failed for vTcamMngId = %d\n",
            vTcamMngId);
        st = cpssDxChVirtualTcamManagerDelete(vTcamMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDelete failed for vTcamMngId = %d\n",
                                     vTcamMngId);
    }

    if (isTestSkipped)
    {
        SKIP_TEST_MAC;
    }
}

UTF_TEST_CASE_MAC(cpssDxChVirtualTcam_block_hole_fragmentation)
{
    GT_STATUS                                  st   = GT_OK;
    GT_U8                                      dev;
    GT_U32                                     numOfDevs = 0;
    GT_U32                                     notAppFamilyBmp = 0;
    GT_U32                                     utfFamilyBit = 0;
    GT_BOOL                                    isTestSkipped = GT_TRUE;
    GT_U32                                     vTcamMngId;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC            vTcamInfo;
    GT_U32                                     sequenceSize;
    GT_U32                                     idx;
    GT_U32                                     totalRulesAllowed;

    static VT_SEQUENCE sequence_block_frag[] =
    {
        /* Fill all blocks except 3 on client PCL */
        {VT_CREATE,      9, CG_PCL,  RS_60,    256*21, GT_OK},
        {VT_STAMP_FILL,  9, CG_PCL,  RS_60,    0, GT_OK},

        /* Fill another block with -10, -40 and -50 rules on client PCL */
        {VT_CREATE,      8, CG_PCL,  RS_50,    126, GT_OK},
        {VT_CREATE,      7, CG_PCL,  RS_40,    130, GT_OK},
        {VT_CREATE,      1, CG_PCL,  RS_10,    (126*1)+(129*2), GT_OK},
        {VT_STAMP_FILL,  1, CG_PCL,  RS_10,    0, GT_OK},
        /* Fill remaining 2 blocks on client TTI */
        {VT_CREATE,      2, CG_TTI,  RS_30,      2, GT_OK},
        {VT_STAMP_FILL,  2, CG_TTI,  RS_30,      2, GT_OK},
        {VT_CREATE,      3, CG_TTI,  RS_30,    256*2, GT_OK},
        {VT_CREATE,      4, CG_TTI,  RS_30,    256*2-2, GT_OK},
        {VT_STAMP_FILL,  4, CG_TTI,  RS_30,    0, GT_OK},
        /* Check the TCAM is full */
        {VT_CREATE,     52, CG_PCL,  RS_10,      1, GT_FULL},
        {VT_CREATE,     53, CG_TTI,  RS_10,      1, GT_FULL},
        /* Remove space equivalent to (126*5)/6+(129*4)/6+1 rows */
        {VT_REMOVE,      7, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      8, CG_NONE, RS_NONE,    0, GT_OK},
        /* Remove space equivalent to 1 block - but removed space is actually spanning both blocks */
        {VT_REMOVE,      3, CG_NONE, RS_NONE,    0, GT_OK},
        /* 1. Check that both blocks and holes could be compressed and reused on the other client PCL */
        {VT_CREATE,      5, CG_PCL,  RS_60,    256+(126*5)/6+(129*4)/6+1, GT_OK},
        {VT_STAMP_FILL,  5, CG_PCL,  RS_60,    0, GT_OK},
        /* Check the TCAM is full again */
        {VT_CREATE,     54, CG_PCL,  RS_10,      1, GT_FULL},
        {VT_CREATE,     55, CG_TTI,  RS_10,      1, GT_FULL},
        /* Check every rule has moved to its correct place */
        {VT_STAMP_CHECK,  1, CG_PCL,  RS_60,    0, GT_OK},
        {VT_STAMP_CHECK,  2, CG_TTI,  RS_30,    0, GT_OK},
        {VT_STAMP_CHECK,  4, CG_TTI,  RS_30,    0, GT_OK},
        {VT_STAMP_CHECK,  5, CG_PCL,  RS_60,    0, GT_OK},
        {VT_STAMP_CHECK,  9, CG_PCL,  RS_60,    0, GT_OK},

        /* Cleanup stuff in order to prepare next test stage */
        {VT_REMOVE,      1, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      2, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      4, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      5, CG_NONE, RS_NONE,    0, GT_OK},

        /* Partially fill another block with -10 rules on client PCL */
        {VT_CREATE,      1, CG_PCL,  RS_10,    (256/6)*6, GT_OK},
        {VT_STAMP_FILL,  1, CG_PCL,  RS_10,    0, GT_OK},
        /* Fill remaining 2 blocks on client TTI */
        {VT_CREATE,      2, CG_TTI,  RS_30,      2, GT_OK},
        {VT_STAMP_FILL,  2, CG_TTI,  RS_30,      2, GT_OK},
        {VT_CREATE,      3, CG_TTI,  RS_30,    256*2, GT_OK},
        {VT_CREATE,      4, CG_TTI,  RS_30,    256*2-2, GT_OK},
        {VT_STAMP_FILL,  4, CG_TTI,  RS_30,    0, GT_OK},
        {VT_CREATE,     53, CG_TTI,  RS_10,      1, GT_FULL},
        /* Remove space equivalent to 1 block - but removed space is actually spanning both blocks */
        {VT_REMOVE,      3, CG_NONE, RS_NONE,    0, GT_OK},
        /* 2. Check that both blocks and holes could be compressed and reused on the other client PCL */
        {VT_CREATE,      5, CG_PCL,  RS_50,    256*2, GT_OK},
        {VT_STAMP_FILL,  5, CG_PCL,  RS_50,    0, GT_OK},
        /* Check the TCAM is full again */
        {VT_CREATE,     54, CG_PCL,  RS_50,      1, GT_FULL},
        {VT_CREATE,     55, CG_TTI,  RS_10,      1, GT_FULL},
        /* Check every rule has moved to its correct place */
        {VT_STAMP_CHECK,  1, CG_PCL,  RS_10,    0, GT_OK},
        {VT_STAMP_CHECK,  2, CG_TTI,  RS_30,    0, GT_OK},
        {VT_STAMP_CHECK,  4, CG_TTI,  RS_30,    0, GT_OK},
        {VT_STAMP_CHECK,  5, CG_PCL,  RS_50,    0, GT_OK},
        {VT_STAMP_CHECK,  9, CG_PCL,  RS_60,    0, GT_OK},

        /* Cleanup stuff in order to prepare next test stage */
        {VT_REMOVE,      1, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      2, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      4, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      5, CG_NONE, RS_NONE,    0, GT_OK},

        /* Partially fill another block with -10 rules on client PCL */
        {VT_CREATE,      1, CG_PCL,  RS_10,    ((2*256)/6)*6, GT_OK},
        {VT_STAMP_FILL,  1, CG_PCL,  RS_10,    0, GT_OK},
        /* Fill remaining 2 blocks on client TTI */
        {VT_CREATE,      2, CG_TTI,  RS_30,      2, GT_OK},
        {VT_STAMP_FILL,  2, CG_TTI,  RS_30,      2, GT_OK},
        {VT_CREATE,      3, CG_TTI,  RS_30,    256*2, GT_OK},
        {VT_CREATE,      4, CG_TTI,  RS_30,    256*2-2, GT_OK},
        {VT_STAMP_FILL,  4, CG_TTI,  RS_30,    0, GT_OK},
        {VT_CREATE,     53, CG_TTI,  RS_10,      1, GT_FULL},
        /* Remove space equivalent to 1 block - but removed space is actually spanning both blocks */
        {VT_REMOVE,      3, CG_NONE, RS_NONE,    0, GT_OK},
        /* 3. Check that both blocks and holes could be compressed and reused on the other client PCL */
        {VT_CREATE,      5, CG_PCL,  RS_40,    256*2, GT_OK},
        {VT_STAMP_FILL,  5, CG_PCL,  RS_40,    0, GT_OK},
        /* Check the TCAM is full again */
        {VT_CREATE,     54, CG_PCL,  RS_40,      1, GT_FULL},
        {VT_CREATE,     55, CG_TTI,  RS_10,      1, GT_FULL},
        /* Check every rule has moved to its correct place */
        {VT_STAMP_CHECK,  1, CG_PCL,  RS_10,    0, GT_OK},
        {VT_STAMP_CHECK,  2, CG_TTI,  RS_30,    0, GT_OK},
        {VT_STAMP_CHECK,  4, CG_TTI,  RS_30,    0, GT_OK},
        {VT_STAMP_CHECK,  5, CG_PCL,  RS_40,    0, GT_OK},
        {VT_STAMP_CHECK,  9, CG_PCL,  RS_60,    0, GT_OK},

        /* Cleanup everything */
        {VT_REMOVE,      1, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      2, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      4, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      5, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      9, CG_NONE, RS_NONE,    0, GT_OK},
    };


    sequenceSize = (sizeof(sequence_block_frag) / sizeof(sequence_block_frag[0]));

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* 1. Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
              devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }
        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;


        /* Create vTCAM manager */
        vTcamMngId = 1;
        st = cpssDxChVirtualTcamManagerCreate(vTcamMngId);
            if (st == GT_ALREADY_EXIST)
                st = GT_OK;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerCreate failed\n");

        st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        if (st != GT_OK)
        {
            cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
            st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        }
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListAdd failed\n");

        /* Adjust allocation size according to device architecture */
        internal_ut_cpssDxChVirtualTcamTotalRulesAllowedPerDevGet(
                           devListArr[0],
                           CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_60_B_E,
                           &totalRulesAllowed);

        if (totalRulesAllowed > NUM_60_RULES_PER_BLOCK) /*shrink the TCAM to the 2 blocks */
        {
            sequence_block_frag[0].rulesAmount =
                    totalRulesAllowed - ( NUM_60_RULES_PER_BLOCK * 3);
        }

        cpssOsMemSet(&vTcamInfo, 0, sizeof(vTcamInfo));
        vTcamInfo.hitNumber     = 0;
        vTcamInfo.autoResize    = GT_FALSE;
        vTcamInfo.ruleAdditionMethod =
            CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E;

        for (idx = 0; (idx < sequenceSize); idx++)
        {
            st = vtcamSequenceEntryProcess(sequence_block_frag, idx, vTcamMngId, &vTcamInfo);
            UTF_VERIFY_EQUAL1_STRING_MAC(
                sequence_block_frag[idx].rc, st,
                "vtcamSequenceEntryProcess failed, index: %d\n", idx);
        }

        st = cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st, "cpssDxChVirtualTcamManagerDevListRemove failed for vTcamMngId = %d\n",
            vTcamMngId);
        st = cpssDxChVirtualTcamManagerDelete(vTcamMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDelete failed for vTcamMngId = %d\n",
                                     vTcamMngId);
    }

    if (isTestSkipped)
    {
        SKIP_TEST_MAC;
    }
}

UTF_TEST_CASE_MAC(cpssDxChVirtualTcam_block_fragmentation_80B)
{
    GT_STATUS                                  st   = GT_OK;
    GT_U8                                      dev;
    GT_U32                                     numOfDevs = 0;
    GT_U32                                     notAppFamilyBmp = 0;
    GT_U32                                     utfFamilyBit = 0;
    GT_BOOL                                    isTestSkipped = GT_TRUE;
    GT_U32                                     vTcamMngId;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC            vTcamInfo;
    GT_U32                                     sequenceSize;
    GT_U32                                     idx;
    GT_U32                                     totalRulesAllowed;

    static VT_SEQUENCE sequence_block_frag[] =
    {
        /* Fill all blocks except 4 on client PCL */
        {VT_CREATE,      1, CG_PCL,  RS_60,    256*20, GT_OK},
        {VT_STAMP_FILL,  1, CG_PCL,  RS_60,    256*20, GT_OK},
        /* Fill remaining 4 blocks on client TTI */
        {VT_CREATE,      2, CG_TTI,  RS_30,  2+2*256, GT_OK},
        {VT_STAMP_FILL,  2, CG_TTI,  RS_30,      0, GT_OK},
        {VT_CREATE,      3, CG_TTI,  RS_30,  256*4, GT_OK},
        {VT_CREATE,      4, CG_TTI,  RS_30,  256*2-2, GT_OK},
        {VT_STAMP_FILL,  4, CG_TTI,  RS_30,      0, GT_OK},
        /* Check the TCAM is full */
        {VT_CREATE,     52, CG_PCL,  RS_10,      1, GT_FULL},
        {VT_CREATE,     53, CG_TTI,  RS_10,      1, GT_FULL},
        /* Remove space equivalent to 2 blocks - but removed space is actually spanning both blocks */
        {VT_REMOVE,      3, CG_NONE, RS_NONE,    0, GT_OK},
        /* 1. Check that both blocks could be compressed and reused on the other client PCL */
        {VT_CREATE,      5, CG_PCL,  RS_80,    256, GT_OK},
        {VT_STAMP_FILL,  5, CG_PCL,  RS_80,      0, GT_OK},
        {VT_CREATE,      6, CG_PCL,  RS_10,  (256*4)/6*6, GT_OK},
        {VT_STAMP_FILL,  6, CG_PCL,  RS_10,      0, GT_OK},
        /* Check the TCAM is full again */
        {VT_CREATE,     54, CG_PCL,  RS_10,      1, GT_FULL},
        {VT_CREATE,     55, CG_TTI,  RS_10,      1, GT_FULL},
        /* Check every rule has moved to its correct place */
        {VT_STAMP_CHECK,  1, CG_PCL,  RS_60,     0, GT_OK},
        {VT_STAMP_CHECK,  2, CG_TTI,  RS_30,     0, GT_OK},
        {VT_STAMP_CHECK,  4, CG_TTI,  RS_30,     0, GT_OK},
        {VT_STAMP_CHECK,  5, CG_PCL,  RS_80,     0, GT_OK},
        {VT_STAMP_CHECK,  6, CG_PCL,  RS_10,     0, GT_OK},

        /* Cleanup everything */
        {VT_REMOVE,      1, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      2, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      4, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      5, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      6, CG_NONE, RS_NONE,    0, GT_OK},
    };


    sequenceSize = (sizeof(sequence_block_frag) / sizeof(sequence_block_frag[0]));

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* 1. Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
              devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }
        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;


        /* Create vTCAM manager */
        vTcamMngId = 1;
        st = cpssDxChVirtualTcamManagerCreate(vTcamMngId);
            if (st == GT_ALREADY_EXIST)
                st = GT_OK;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerCreate failed\n");

        st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        if (st != GT_OK)
        {
            cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
            st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        }
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListAdd failed\n");

        /* Adjust allocation size according to device architecture */
        internal_ut_cpssDxChVirtualTcamTotalRulesAllowedPerDevGet(
                           devListArr[0],
                           CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_60_B_E,
                           &totalRulesAllowed);

        if (totalRulesAllowed > NUM_60_RULES_PER_BLOCK) /*shrink the TCAM to the 2 blocks */
        {
            sequence_block_frag[0].rulesAmount =
                    totalRulesAllowed - ( NUM_60_RULES_PER_BLOCK * 4);
        }

        cpssOsMemSet(&vTcamInfo, 0, sizeof(vTcamInfo));
        vTcamInfo.hitNumber     = 0;
        vTcamInfo.autoResize    = GT_FALSE;
        vTcamInfo.ruleAdditionMethod =
            CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E;

        for (idx = 0; (idx < sequenceSize); idx++)
        {
            st = vtcamSequenceEntryProcess(sequence_block_frag, idx, vTcamMngId, &vTcamInfo);
            UTF_VERIFY_EQUAL1_STRING_MAC(
                sequence_block_frag[idx].rc, st,
                "vtcamSequenceEntryProcess failed, index: %d\n", idx);
        }

        st = cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st, "cpssDxChVirtualTcamManagerDevListRemove failed for vTcamMngId = %d\n",
            vTcamMngId);
        st = cpssDxChVirtualTcamManagerDelete(vTcamMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDelete failed for vTcamMngId = %d\n",
                                     vTcamMngId);
    }

    if (isTestSkipped)
    {
        SKIP_TEST_MAC;
    }
}

UTF_TEST_CASE_MAC(cpssDxChVirtualTcam_block_fragmentation_80B_columnTypes)
{
    GT_STATUS                                  st   = GT_OK;
    GT_U8                                      dev;
    GT_U32                                     numOfDevs = 0;
    GT_U32                                     notAppFamilyBmp = 0;
    GT_U32                                     utfFamilyBit = 0;
    GT_BOOL                                    isTestSkipped = GT_TRUE;
    GT_U32                                     vTcamMngId;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC            vTcamInfo;
    VT_SEQUENCE                                *sequencePtr;
    GT_U32                                     sequenceSize;
    GT_U32                                     idx;

    static VT_SEQUENCE bc2_create_sequence[] =
    {
        /* Create a number of vTcams - bc2/3 */
        {VT_CREATE,    5, CG_TTI,  RS_20,    1002, GT_OK},
        {VT_CREATE,    6, CG_TTI,  RS_30,    1000, GT_OK},
        {VT_COMPLETE,  7, CG_TTI,  RS_NONE,  1140, GT_OK},
        {VT_CREATE,    8, CG_PCL,  RS_20,    1002, GT_OK},
        {VT_CREATE,    9, CG_PCL,  RS_40,    2000, GT_OK},
        {VT_CREATE,   10, CG_PCL,  RS_50,    1000, GT_OK},
        {VT_CREATE,   11, CG_PCL,  RS_10,    7002, GT_OK},
        {VT_CREATE,   12, CG_PCL,  RS_80,     250, GT_OK},
        {VT_CREATE,   13, CG_PCL,  RS_30,     100, GT_OK},
        {VT_CREATE,   14, CG_PCL,  RS_50,     814, GT_OK},
        {VT_CREATE,   15, CG_PCL,  RS_10,     814, GT_OK},
        {VT_COMPLETE, 16, CG_PCL,  RS_NONE,   528, GT_OK},
        /* Check there is no more space - bc2/3 */
        {VT_CREATE,   50, CG_TTI,  RS_10,       1, GT_FULL},
        {VT_CREATE,   51, CG_PCL,  RS_10,       1, GT_FULL},
        /* Remove some vTcams in order to make space - bc2/3 */
        {VT_REMOVE,    6, CG_NONE, RS_NONE,  1000, GT_OK},
        {VT_REMOVE,    8, CG_NONE, RS_NONE,  1002, GT_OK},
        {VT_REMOVE,   10, CG_NONE, RS_NONE,  1000, GT_OK},
        {VT_REMOVE,   13, CG_NONE, RS_NONE,   100, GT_OK},
        {VT_REMOVE,   15, CG_NONE, RS_NONE,   814, GT_OK},
        /* Sequence of increase operations - bc2/3 */
        {VT_INCREASE,  5, CG_TTI,  RS_20,  996, GT_OK},
        {VT_INCREASE,  7, CG_TTI,  RS_10, 1008, GT_OK},
        {VT_INCREASE,  5, CG_TTI,  RS_20, 2304, GT_OK},     /* block defrag expected here */
        /* Check there is really no more space - bc2/3 */
        {VT_INCREASE,  5, CG_TTI,  RS_20,    1, GT_FULL},
        /* Remove all vTcams in order to finish test - bc2/3 */
        {VT_REMOVE,    5, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,    7, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,    9, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,   11, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,   12, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,   14, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,   16, CG_NONE, RS_NONE,     0, GT_OK},
    };

    /* Sequence below is a subset of bobk_sequence: only part 1 + part 2 up to remove vTcams step */
    static VT_SEQUENCE bobk_create_sequence[] =
    {
        /* Create a number of vTcams - bobk */
        {VT_CREATE,    5, CG_TTI,  RS_20,     501, GT_OK},
        {VT_CREATE,    6, CG_TTI,  RS_30,     500, GT_OK},
        {VT_COMPLETE,  7, CG_TTI,  RS_NONE,   570, GT_OK},
        {VT_CREATE,    8, CG_PCL,  RS_20,     501, GT_OK},
        {VT_CREATE,    9, CG_PCL,  RS_40,    1000, GT_OK},
        {VT_CREATE,   10, CG_PCL,  RS_50,     500, GT_OK},
        {VT_CREATE,   11, CG_PCL,  RS_10,    3501, GT_OK},
        {VT_CREATE,   12, CG_PCL,  RS_80,     100, GT_OK},
        {VT_CREATE,   13, CG_PCL,  RS_30,      50, GT_OK},
        {VT_CREATE,   14, CG_PCL,  RS_50,     407, GT_OK},
        {VT_CREATE,   15, CG_PCL,  RS_10,     407, GT_OK},
        {VT_CREATE,   16, CG_PCL,  RS_10,     912, GT_OK},
        /* Check there is no more space - bobk */
        {VT_CREATE,   50, CG_TTI,  RS_10,       1, GT_FULL},
        {VT_CREATE,   51, CG_PCL,  RS_10,       1, GT_FULL},
        /* Remove some vTcams in order to make space - bobk */
        {VT_REMOVE,    6, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,    8, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,   10, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,   13, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,   15, CG_NONE, RS_NONE,     0, GT_OK},
        /* Sequence of increase operations - bobk */
        {VT_INCREASE,  5, CG_TTI,  RS_20,  498, GT_OK},
        {VT_INCREASE,  7, CG_TTI,  RS_10,  504, GT_OK},
        {VT_INCREASE,  5, CG_TTI,  RS_20,  768, GT_OK},     /* block defrag expected here */
        /* Check there is really no more space - bobk */
        {VT_INCREASE,  5, CG_TTI,  RS_20,    1, GT_FULL},
        /* Remove all vTcams in order to finish test - bobk */
        {VT_REMOVE,    5, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,    7, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,    9, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,   11, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,   12, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,   14, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,   16, CG_NONE, RS_NONE,     0, GT_OK},
    };

    static VT_SEQUENCE aldrin_create_sequence[] =
    {
        /* Create a number of vTcams - aldrin */
        {VT_CREATE,    5, CG_TTI,  RS_20,     252, GT_OK},
        {VT_CREATE,    6, CG_TTI,  RS_30,     250, GT_OK},
        {VT_COMPLETE,  7, CG_TTI,  RS_NONE,   282, GT_OK},
        {VT_CREATE,    8, CG_PCL,  RS_20,     252, GT_OK},
        {VT_CREATE,    9, CG_PCL,  RS_40,     500, GT_OK},
        {VT_CREATE,   10, CG_PCL,  RS_80,      49, GT_OK},
        {VT_CREATE,   11, CG_PCL,  RS_10,    1752, GT_OK},
        {VT_CREATE,   12, CG_PCL,  RS_50,     250, GT_OK},
        {VT_CREATE,   13, CG_PCL,  RS_30,      77, GT_OK},
        {VT_CREATE,   14, CG_PCL,  RS_50,     203, GT_OK},
        {VT_CREATE,   15, CG_PCL,  RS_10,     204, GT_OK},
        {VT_COMPLETE, 16, CG_PCL,  RS_NONE,   132, GT_OK},
        /* Check there is no more space - aldrin */
        {VT_CREATE,   50, CG_TTI,  RS_10,       1, GT_FULL},
        {VT_CREATE,   51, CG_PCL,  RS_10,       1, GT_FULL},
        /* Remove some vTcams in order to make space - bc2/3 */
        {VT_REMOVE,    6, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,    8, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,   12, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,   13, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,   15, CG_NONE, RS_NONE,     0, GT_OK},
        /* Sequence of increase operations - aldrin */
        {VT_INCREASE,  5, CG_TTI,  RS_20,  249, GT_OK},
        {VT_INCREASE,  7, CG_TTI,  RS_10,  252, GT_OK},
        {VT_INCREASE,  5, CG_TTI,  RS_20,  768, GT_OK},     /* block defrag expected here */
        /* Check there is really no more space - aldrin */
        {VT_INCREASE,  5, CG_TTI,  RS_20,    1, GT_FULL},
        /* Remove all vTcams in order to finish test - aldrin */
        {VT_REMOVE,    5, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,    7, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,    9, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,   10, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,   11, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,   14, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,   16, CG_NONE, RS_NONE,     0, GT_OK},
    };


    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* 1. Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
              devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }
        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;


        /* Create vTCAM manager */
        vTcamMngId = 1;
        st = cpssDxChVirtualTcamManagerCreate(vTcamMngId);
            if (st == GT_ALREADY_EXIST)
                st = GT_OK;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerCreate failed\n");

        st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        if (st != GT_OK)
        {
            cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
            st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        }
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListAdd failed\n");

        /* Adjust allocation size according to device architecture */
        switch (utfFamilyBit)
        {
            case UTF_ALDRIN2_E:
            case UTF_FALCON_E:
            case UTF_CAELUM_E:
                sequencePtr = bobk_create_sequence;
                sequenceSize = (sizeof(bobk_create_sequence) / sizeof(bobk_create_sequence[0]));
                break;

            case UTF_ALDRIN_E:
            case UTF_AC3X_E:
                sequencePtr = aldrin_create_sequence;
                sequenceSize = (sizeof(aldrin_create_sequence) / sizeof(aldrin_create_sequence[0]));
                break;

            case UTF_BOBCAT2_E:
            case UTF_BOBCAT3_E:
            default:
                sequencePtr = bc2_create_sequence;
                sequenceSize = (sizeof(bc2_create_sequence) / sizeof(bc2_create_sequence[0]));
                break;
        }
        cpssOsMemSet(&vTcamInfo, 0, sizeof(vTcamInfo));
        vTcamInfo.hitNumber     = 0;
        vTcamInfo.autoResize    = GT_FALSE;
        vTcamInfo.ruleAdditionMethod =
            CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E;

        for (idx = 0; (idx < sequenceSize); idx++)
        {
            st = vtcamSequenceEntryProcess(sequencePtr, idx, vTcamMngId, &vTcamInfo);
            UTF_VERIFY_EQUAL1_STRING_MAC(
                sequencePtr[idx].rc, st,
                "vtcamSequenceEntryProcess failed, index: %d\n", idx);
        }

        st = cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st, "cpssDxChVirtualTcamManagerDevListRemove failed for vTcamMngId = %d\n",
            vTcamMngId);
        st = cpssDxChVirtualTcamManagerDelete(vTcamMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDelete failed for vTcamMngId = %d\n",
                                     vTcamMngId);
    }

    if (isTestSkipped)
    {
        SKIP_TEST_MAC;
    }
}

UTF_TEST_CASE_MAC(cpssDxChVirtualTcam_basicRuleWrite_priorityMode)
{
    GT_STATUS                                  st   = GT_OK;
    GT_U8                                      dev;
    GT_U32                                     numOfDevs = 0;
    GT_U32                                     notAppFamilyBmp = 0;
    GT_U32                                     utfFamilyBit = 0;
    GT_BOOL                                    isTestSkipped = GT_TRUE;
    GT_U32                                     vTcamMngId;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC RuleAttributes;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC       RuleData;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC       TcamRuleType;
    CPSS_DXCH_VIRTUAL_TCAM_CHECK_INFO_STC      vTcamCheckInfoArr[1];
    CPSS_DXCH_PCL_ACTION_STC                   pclAction;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              mask;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              pattern;
    GT_U32                                     expectedNumUsed = 0;
    GT_U32                                     RuleId,prevRuleId,ii;

    vTcamMngId = 1;
    vTcamCheckInfoArr[0].isExist = GT_TRUE;
    vTcamCheckInfoArr[0].vTcamId = 1;
    vTcamCheckInfoArr[0].vTcamInfo.clientGroup = 0; /* TTI and IPCL0 */
    vTcamCheckInfoArr[0].vTcamInfo.guaranteedNumOfRules = 9;
    vTcamCheckInfoArr[0].vTcamInfo.hitNumber = 0;
    vTcamCheckInfoArr[0].vTcamInfo.ruleSize =
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E;
    vTcamCheckInfoArr[0].vTcamInfo.autoResize = GT_FALSE;
    vTcamCheckInfoArr[0].vTcamInfo.ruleAdditionMethod =
            CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E;

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* this feature is on eArch devices, xCat3 and Lion2 */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    notAppFamilyBmp &= (~UTF_XCAT3_E) & (~UTF_LION2_E);

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* 1. Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
              devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }
        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;

        expectedNumUsed = 0;
        /* Create vTCAM manager */
        st = cpssDxChVirtualTcamManagerCreate(vTcamMngId);
        if (st == GT_ALREADY_EXIST)
            st = GT_OK;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerCreate failed\n");

        st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        if (st != GT_OK)
        {
            cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
            st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        }
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListAdd failed\n");

        /* 1. Create vTCAM(1) */
        st = ut_cpssDxChVirtualTcamCreate(vTcamMngId, vTcamCheckInfoArr[0].vTcamId, &vTcamCheckInfoArr[0].vTcamInfo);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                                    "ut_cpssDxChVirtualTcamCreate failed for vTcamMngId/vTcamMngId = %d/%d, st = %d\n",
                                    vTcamMngId, vTcamCheckInfoArr[0].vTcamId, st);


        /* Add rule #0 */
        RuleAttributes.priority          = 0; /* Not be used */
        TcamRuleType.ruleType            = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;
        TcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
        RuleData.valid                   = GT_TRUE;
        /* set new rule */
        cpssOsMemSet(&mask, '\0', sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
        cpssOsMemSet(&pattern, '\0', sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
        cpssOsMemSet(&pclAction, '\0', sizeof(CPSS_DXCH_PCL_ACTION_STC));
        /* Set Mask */
        setRuleStdNotIp(&mask, &pattern, &pclAction);

        RuleData.rule.pcl.actionPtr     = &pclAction;
        RuleData.rule.pcl.maskPtr       = &mask;
        RuleData.rule.pcl.patternPtr    = &pattern;

        /* fill the table */
        for(RuleId = 0; RuleId < 12/*more than 9*/; RuleId++)
        {
            UNIQUE_PCL_RULE_SET_MAC(&TcamRuleType,&pattern,RuleId);

            RuleAttributes.priority = (RuleId < 3) ? priorityArr[0] :
                                      (RuleId < 6) ? priorityArr[1] :
                                      priorityArr[2];

            st = cpssDxChVirtualTcamRuleWrite(vTcamMngId, vTcamCheckInfoArr[0].vTcamId, RuleId, &RuleAttributes,
                &TcamRuleType, &RuleData);
            if(st == GT_FULL)
            {
                /* no more */
                break;
            }

            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamRuleWrite failed\n");
            expectedNumUsed++;
        }

        checkNumUsed(vTcamMngId, vTcamCheckInfoArr[0].vTcamId,expectedNumUsed);
        /* check that the HW hold proper values */
        checkHwContentAllRules(vTcamMngId,vTcamCheckInfoArr[0].vTcamId);

        for(prevRuleId = expectedNumUsed ; prevRuleId > 0; prevRuleId--)
        {
            /* make hole in each priority, on each position */
            RuleId = prevRuleId - 1;
            st = cpssDxChVirtualTcamRuleDelete(vTcamMngId, vTcamCheckInfoArr[0].vTcamId, RuleId);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamRuleDelete failed\n");
            expectedNumUsed--;

            checkNumUsed(vTcamMngId, vTcamCheckInfoArr[0].vTcamId,expectedNumUsed);
            /* check that the HW hold proper values */
            checkHwContentAllRules(vTcamMngId,vTcamCheckInfoArr[0].vTcamId);

            for(ii = 0 ; ii < 5; ii++)
            {
                RuleId = (ii+1)*10000;
                UNIQUE_PCL_RULE_SET_MAC(&TcamRuleType,&pattern,RuleId);
                RuleAttributes.priority = priorityArr[(ii+1)%5];

                st = cpssDxChVirtualTcamRuleWrite(vTcamMngId, vTcamCheckInfoArr[0].vTcamId, RuleId, &RuleAttributes,
                    &TcamRuleType, &RuleData);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamRuleWrite failed\n");

                expectedNumUsed++;
                checkNumUsed(vTcamMngId, vTcamCheckInfoArr[0].vTcamId,expectedNumUsed);
                /* check that the HW hold proper values */
                checkHwContentAllRules(vTcamMngId,vTcamCheckInfoArr[0].vTcamId);

                st = cpssDxChVirtualTcamRuleDelete(vTcamMngId, vTcamCheckInfoArr[0].vTcamId, RuleId);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamRuleDelete failed\n");
                expectedNumUsed--;

                checkNumUsed(vTcamMngId, vTcamCheckInfoArr[0].vTcamId,expectedNumUsed);
                /* check that the HW hold proper values */
                checkHwContentAllRules(vTcamMngId,vTcamCheckInfoArr[0].vTcamId);
            }

            RuleId = prevRuleId - 1;
            UNIQUE_PCL_RULE_SET_MAC(&TcamRuleType,&pattern,RuleId);
            RuleAttributes.priority = (RuleId < 3) ? priorityArr[0] :
                                      (RuleId < 6) ? priorityArr[1] :
                                      priorityArr[2];

            st = cpssDxChVirtualTcamRuleWrite(vTcamMngId, vTcamCheckInfoArr[0].vTcamId, RuleId, &RuleAttributes,
                &TcamRuleType, &RuleData);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamRuleWrite failed\n");

            expectedNumUsed++;
            checkNumUsed(vTcamMngId, vTcamCheckInfoArr[0].vTcamId,expectedNumUsed);
            /* check that the HW hold proper values */
            checkHwContentAllRules(vTcamMngId,vTcamCheckInfoArr[0].vTcamId);

            /* TABLE is FULL now */
        }

        prevRuleId = 1;/*first ruleId to remove*/
        /* make hole in priorityArr[0] */
        for(ii = 0 ; ii < 5; ii++)
        {
            RuleId = prevRuleId;
            st = cpssDxChVirtualTcamRuleDelete(vTcamMngId, vTcamCheckInfoArr[0].vTcamId, RuleId);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamRuleDelete failed\n");
            expectedNumUsed--;

            checkNumUsed(vTcamMngId, vTcamCheckInfoArr[0].vTcamId,expectedNumUsed);
            /* check that the HW hold proper values */
            checkHwContentAllRules(vTcamMngId,vTcamCheckInfoArr[0].vTcamId);

            RuleId = (ii+1)*10000;
            prevRuleId = RuleId;
            UNIQUE_PCL_RULE_SET_MAC(&TcamRuleType,&pattern,RuleId);
            RuleAttributes.priority = priorityArr[(ii+1)%5];

            /*DEBUG_PRINT(vTcamMngId,vTcamCheckInfoArr[0].vTcamId);*/

            st = cpssDxChVirtualTcamRuleWrite(vTcamMngId, vTcamCheckInfoArr[0].vTcamId, RuleId, &RuleAttributes,
                &TcamRuleType, &RuleData);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamRuleWrite failed\n");

            expectedNumUsed++;
            checkNumUsed(vTcamMngId, vTcamCheckInfoArr[0].vTcamId,expectedNumUsed);
            /* check that the HW hold proper values */
            checkHwContentAllRules(vTcamMngId,vTcamCheckInfoArr[0].vTcamId);

            /* TABLE is FULL now */
        }

        /* delete the first rules (high priority) */
        vTcamDeleteConsecutiveRules(vTcamMngId,
            vTcamCheckInfoArr[0].vTcamId,
            CPSS_DXCH_VIRTUAL_TCAM_START_RULE_ID_CNS,
            4);
        expectedNumUsed-=4;

        /* insert low priority rules */
        for(ii = 0 ; ii < 4; ii++)
        {
            RuleId = (ii+1)*1000;
            UNIQUE_PCL_RULE_SET_MAC(&TcamRuleType,&pattern,RuleId);
            RuleAttributes.priority = priorityArr[(ii+1)%5] + 10000;

            st = cpssDxChVirtualTcamRuleWrite(vTcamMngId, vTcamCheckInfoArr[0].vTcamId, RuleId, &RuleAttributes,
                &TcamRuleType, &RuleData);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamRuleWrite failed\n");

            expectedNumUsed++;
            checkNumUsed(vTcamMngId, vTcamCheckInfoArr[0].vTcamId,expectedNumUsed);
            /* check that the HW hold proper values */
            checkHwContentAllRules(vTcamMngId,vTcamCheckInfoArr[0].vTcamId);
        }

        /* delete the first rules (high priority) */
        vTcamDeleteConsecutiveRules(vTcamMngId,
            vTcamCheckInfoArr[0].vTcamId,
            CPSS_DXCH_VIRTUAL_TCAM_START_RULE_ID_CNS,
            4);
        expectedNumUsed-=4;

        /* insert high priority rules */
        for(ii = 0 ; ii < 4; ii++)
        {
            RuleId = (ii+1)*100000;
            UNIQUE_PCL_RULE_SET_MAC(&TcamRuleType,&pattern,RuleId);
            RuleAttributes.priority = priorityArr[(ii+1)%5] / 10;

            st = cpssDxChVirtualTcamRuleWrite(vTcamMngId, vTcamCheckInfoArr[0].vTcamId, RuleId, &RuleAttributes,
                &TcamRuleType, &RuleData);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamRuleWrite failed\n");

            expectedNumUsed++;
            checkNumUsed(vTcamMngId, vTcamCheckInfoArr[0].vTcamId,expectedNumUsed);
            /* check that the HW hold proper values */
            checkHwContentAllRules(vTcamMngId,vTcamCheckInfoArr[0].vTcamId);
        }



        vTcamDeleteConsecutiveRules(vTcamMngId,
            vTcamCheckInfoArr[0].vTcamId,
            CPSS_DXCH_VIRTUAL_TCAM_START_RULE_ID_CNS,
            DELETE_ALL_CNS);

        st = ut_cpssDxChVirtualTcamRemove(vTcamMngId, vTcamCheckInfoArr[0].vTcamId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamRemove failed\n");

        st = cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListRemove failed for vTcamMngId = %d\n",
                                     vTcamMngId);
        st = cpssDxChVirtualTcamManagerDelete(vTcamMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDelete failed for vTcamMngId = %d\n",
                                     vTcamMngId);
    }

    if (isTestSkipped)
    {
        SKIP_TEST_MAC;
    }
}

typedef struct {
    GT_U32  vTcamSeqId;
    GT_U32  ruleId;
    enum  {VT_RULE_NONE, VT_RULE_MOVED, VT_RULE_DELETED} operation;
}VT_RULE_ID;

static GT_STATUS vtcamSequenceRuleUpdate
(
    IN GT_U32                                  vTcamMngId,
    IN VT_SEQUENCE                             *sequencePtr,
    IN GT_U32                                  idx,
    IN CPSS_DXCH_VIRTUAL_TCAM_RULE_ID          ruleId,
    IN CPSS_DXCH_PCL_ACTION_STC                *pclActionPtr,
    IN CPSS_DXCH_PCL_RULE_FORMAT_UNT           *pclMaskPtr,
    IN CPSS_DXCH_PCL_RULE_FORMAT_UNT           *pclPatternPtr,
    IN CPSS_DXCH_TTI_ACTION_UNT                *ttiActionPtr,
    IN CPSS_DXCH_TTI_RULE_UNT                  *ttiMaskPtr,
    IN CPSS_DXCH_TTI_RULE_UNT                  *ttiPatternPtr
)
{
    GT_STATUS                                   st = GT_OK, expectedRc;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC  ruleAttributes;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC        tcamRuleType;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC        ruleData;

    CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT          pclRuleFormat[] =
    {
        CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E,
        CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E,
        CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E  /* Was: CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E */,  /* TBD: Check it in BC2 */
        CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_40_E,
        CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_50_E,
        CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E    /* Was: CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_E */   /* TBD: Check it in BC2 */
    };

    CPSS_DXCH_TTI_RULE_TYPE_ENT                 ttiRuleFormat[] =
    {
        CPSS_DXCH_TTI_RULE_UDB_10_E,
        CPSS_DXCH_TTI_RULE_UDB_20_E,
        CPSS_DXCH_TTI_RULE_IPV4_E
    };

#define PCL_RULE_SIZE(vtcam_rule_size) (((vtcam_rule_size) == RS_NONE) ? CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E : pclRuleFormat[(vtcam_rule_size)])
#define TTI_RULE_SIZE(vtcam_rule_size) (((vtcam_rule_size) == RS_NONE) ? CPSS_DXCH_TTI_RULE_UDB_10_E : (ttiRuleFormat[(vtcam_rule_size) % 3]))

    cpssOsMemSet(&ruleAttributes, 0, sizeof(ruleAttributes));
    cpssOsMemSet(&ruleData, 0, sizeof(ruleData));

    expectedRc = sequencePtr[idx].rc;

    switch (sequencePtr[idx].clientGroup & 0xF)
    {
        case CG_PCL:
            tcamRuleType.ruleType = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;
            tcamRuleType.rule.pcl.ruleFormat = PCL_RULE_SIZE(sequencePtr[idx].ruleSize);
            UNIQUE_PCL_RULE_SET_MAC(&tcamRuleType, pclPatternPtr, ruleId);
            ruleData.rule.pcl.actionPtr     = pclActionPtr;
            ruleData.rule.pcl.maskPtr       = pclMaskPtr;
            ruleData.rule.pcl.patternPtr    = pclPatternPtr;
            break;
        case CG_TTI:
            tcamRuleType.ruleType = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_TTI_E;
            tcamRuleType.rule.tti.ruleFormat = TTI_RULE_SIZE(sequencePtr[idx].ruleSize);
            UNIQUE_TTI_RULE_SET_MAC(&tcamRuleType, ttiPatternPtr, ruleId);
            ruleData.rule.tti.actionPtr     = ttiActionPtr;
            ruleData.rule.tti.maskPtr       = ttiMaskPtr;
            ruleData.rule.tti.patternPtr    = ttiPatternPtr;
            break;
        case CG_NONE:
            st = GT_FAIL;
    }

    ruleData.valid = GT_TRUE;

    st = cpssDxChVirtualTcamRuleWrite(vTcamMngId, sequencePtr[idx].vTcamId, ruleId, &ruleAttributes,
                                      &tcamRuleType, &ruleData);
    if (st != GT_OK)
    {
        PRV_UTF_LOG1_MAC("Returned %d\n", st);
    }
    return (st == expectedRc) ? GT_OK : GT_FAIL;
}

static GT_BOOL checkDuplicateRules
(
    GT_U32                                      ruleId,
    VT_RULE_ID                                  * srcTcamRulePtr,
    VT_RULE_ID                                  * dstTcamRulePtr,
    GT_U32                                      checkSize
)
{
    GT_U32 idx;

    for (idx = 0; idx < checkSize; idx++)
    {
        if (srcTcamRulePtr[idx].ruleId == ruleId ||
            dstTcamRulePtr[idx].ruleId == ruleId)
        {
            return GT_TRUE;
        }
    }
    return GT_FALSE;
}

UTF_TEST_CASE_MAC(cpssDxChVirtualTcam_createRemove)
{
    GT_STATUS                                   st   = GT_OK;
    GT_U8                                       dev;
    GT_U32                                      vTcamMngId;
    GT_U32                                      idx;
    GT_U32                                      numOfDevs = 0;
    GT_U32                                      notAppFamilyBmp = 0;
    GT_U32                                     utfFamilyBit = 0;
    GT_BOOL                                    isTestSkipped = GT_TRUE;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC             vTcamInfo;
    VT_SEQUENCE                                 *sequencePtr;
    GT_U32                                      sequenceSize;
    GT_U32                                      vTcamAdded = 0;
    CPSS_DXCH_VIRTUAL_TCAM_USAGE_STC            vTcamUsage;

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);


    cpssOsMemSet(&vTcamInfo, 0, sizeof(vTcamInfo));

    vTcamMngId = 7;

    /* this feature is on eArch devices, xCat3 and Lion2 */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    notAppFamilyBmp &= (~UTF_XCAT3_E) & (~UTF_LION2_E);

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* 1. Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
            devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }
        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;

        /* Create vTCAM manager */
        st = cpssDxChVirtualTcamManagerCreate(vTcamMngId);
        if (st == GT_ALREADY_EXIST)
        {
            st = GT_OK;
        }
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerCreate failed\n");


        st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        if (st != GT_OK)
        {
            cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
            st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        }
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListAdd failed\n numOfDevs - %d", numOfDevs);

        switch (utfFamilyBit)
        {
            case UTF_XCAT3_E:
                sequencePtr = xcat3_sequence;
                sequenceSize = sizeof(xcat3_sequence) / sizeof(xcat3_sequence[0]);
                break;

            case UTF_LION2_E:
                sequencePtr = lion2_sequence;
                sequenceSize = sizeof(lion2_sequence) / sizeof(lion2_sequence[0]);
                break;

            case UTF_ALDRIN2_E:
            case UTF_FALCON_E:
            case UTF_CAELUM_E:
                sequencePtr = bobk_sequence;
                sequenceSize = sizeof(bobk_sequence) / sizeof(bobk_sequence[0]);
                break;

            case UTF_ALDRIN_E:
            case UTF_AC3X_E:
                sequencePtr = aldrin_sequence;
                sequenceSize = sizeof(aldrin_sequence) / sizeof(aldrin_sequence[0]);
                break;

            case UTF_BOBCAT2_E:
            case UTF_BOBCAT3_E:
            default:
                sequencePtr = bc2_sequence;
                sequenceSize = sizeof(bc2_sequence) / sizeof(bc2_sequence[0]);
                break;
        }

        vTcamInfo.hitNumber     = 0;
        vTcamInfo.autoResize    = GT_FALSE;
        vTcamInfo.ruleAdditionMethod =
            CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E;

        /* create VTCAMs and fill in PCL/TTI rules */
        for (idx = 0; idx < sequenceSize; idx++)
        {
            if (sequencePtr[idx].operation == VT_CREATE ||
                sequencePtr[idx].operation == VT_COMPLETE)
            {
                st = vtcamSequenceEntryProcess(sequencePtr, idx, vTcamMngId, &vTcamInfo);
                /* no free memory for new vTCAM */
                if  ((sequencePtr[idx].rc != vtcamSequenceEntryProcessLastResult)
                    && (vtcamSequenceEntryProcessLastResult == GT_FULL))
                {
                    break;
                }

                if (sequencePtr[idx].rc == GT_OK)
                {
                    /* new vTCAM created */
                    vTcamAdded = idx + 1;
                }
            }
        }

        UTF_VERIFY_NOT_EQUAL0_STRING_MAC(0, vTcamAdded, "failed adding vTcams");

        for (idx = 0; idx < vTcamAdded; idx++)
        {
            if ((sequencePtr[idx].operation == VT_CREATE ||
                sequencePtr[idx].operation == VT_COMPLETE) &&
                (sequencePtr[idx].rc == GT_OK))
            {
                st = cpssDxChVirtualTcamUsageGet(vTcamMngId, sequencePtr[idx].vTcamId, &vTcamUsage);
                UTF_VERIFY_EQUAL3_STRING_MAC(
                    GT_OK, st,
                    "cpssDxChVirtualTcamUsageGet failed vTcamId = %d, rulesUsed = %d, rulesFree = %d\n",
                     sequencePtr[idx].vTcamId, vTcamUsage.rulesUsed, vTcamUsage.rulesFree);

                st = ut_cpssDxChVirtualTcamRemove(vTcamMngId, sequencePtr[idx].vTcamId);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamRemove failed\n");
            }
        }

        /* Check for special case:
         *  - Logical index mode.
         *  - guaranteed size is 0.
         *  - No rules are added.
         *  - tCam mng. is removed from device */
        {
            static VT_SEQUENCE                                 smallSequence[] =
            {
                {VT_CREATE,    1, CG_PCL,  RS_10,    0, GT_OK},
                {VT_REMOVE,    1, CG_NONE, RS_NONE,  0, GT_OK},
            };

            for (idx = 0; idx < (sizeof(smallSequence)/sizeof(smallSequence[0])); idx++)
            {
                st = vtcamSequenceEntryProcess(smallSequence, idx, vTcamMngId, &vTcamInfo);
            }
        }

        st = cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListRemove failed for vTcamMngId = %d\n",
                                     vTcamMngId);
        st = cpssDxChVirtualTcamManagerDelete(vTcamMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDelete failed for vTcamMngId = %d\n",
                                     vTcamMngId);

    }

    if (isTestSkipped)
    {
        SKIP_TEST_MAC;
    }
}

UTF_TEST_CASE_MAC(cpssDxChVirtualTcam_moveRulesLogicalIndex)
{
    GT_STATUS                                   st   = GT_OK;
    GT_U8                                       dev;
    GT_U32                                      vTcamMngId;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ID              ruleId = 0, ruleId1 = 0;
    GT_U32                                      idx, vTcamIdx, iteration;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC  ruleAttributes;
    CPSS_DXCH_PCL_ACTION_STC                    pclAction;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT               pclMask;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT               pclPattern;
    CPSS_DXCH_TTI_ACTION_UNT                    ttiAction;
    CPSS_DXCH_TTI_RULE_UNT                      ttiMask;
    CPSS_DXCH_TTI_RULE_UNT                      ttiPattern;
    GT_U32                                      numOfDevs = 0;
    GT_U32                                      notAppFamilyBmp = 0;
    GT_U32                                     utfFamilyBit = 0;
    GT_BOOL                                    isTestSkipped = GT_TRUE;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_TYPE_STC   actionType;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_DATA_STC   actionData;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC             vTcamInfo;
    VT_RULE_ID                                  srcTcamRule[50];
    VT_RULE_ID                                  dstTcamRule[50];
    VT_SEQUENCE                                 *sequencePtr;
    GT_U32                                      sequenceSize;
    GT_U32                                      ruleIdx;
    GT_U32                                      vTcamAdded = 0;
    CPSS_DXCH_VIRTUAL_TCAM_USAGE_STC            vTcamUsage;
    GT_BOOL                                     duplicated;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC        tcamRuleType;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC        tcamRuleDataGet;
    CPSS_DXCH_PCL_ACTION_STC                    pclActionGet;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT               pclMaskGet;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT               pclPatternGet;
    CPSS_DXCH_TTI_ACTION_UNT                    ttiActionGet;
    CPSS_DXCH_TTI_RULE_UNT                      ttiMaskGet;
    CPSS_DXCH_TTI_RULE_UNT                      ttiPatternGet;
    GT_U32                                      tmpValue;

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);


    cpssOsMemSet(&vTcamInfo, 0, sizeof(vTcamInfo));
    cpssOsMemSet(&actionType, 0, sizeof(actionType));
    cpssOsMemSet(&actionData, 0, sizeof(actionData));

    cpssOsMemSet(&tcamRuleDataGet, 0, sizeof(tcamRuleDataGet));

    vTcamMngId = 7;

    /* this feature is on eArch devices, xCat3 and Lion2 */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    notAppFamilyBmp &= (~UTF_XCAT3_E) & (~UTF_LION2_E);

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* 1. Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
            devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }
        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;

        /* Create vTCAM manager */
        st = cpssDxChVirtualTcamManagerCreate(vTcamMngId);
        if (st == GT_ALREADY_EXIST)
        {
            st = GT_OK;
        }
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerCreate failed\n");


        st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        if (st != GT_OK)
        {
            cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
            st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        }
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListAdd failed\n numOfDevs - %d", numOfDevs);

        switch (utfFamilyBit)
        {
            case UTF_XCAT3_E:
                sequencePtr = xcat3_sequence;
                sequenceSize = sizeof(xcat3_sequence) / sizeof(xcat3_sequence[0]);
                break;

            case UTF_LION2_E:
                sequencePtr = lion2_sequence;
                sequenceSize = sizeof(lion2_sequence) / sizeof(lion2_sequence[0]);
                break;

            case UTF_ALDRIN2_E:
            case UTF_CAELUM_E:
                sequencePtr = bobk_sequence;
                sequenceSize = sizeof(bobk_sequence) / sizeof(bobk_sequence[0]);
                break;

            case UTF_ALDRIN_E:
            case UTF_FALCON_E:
            case UTF_AC3X_E:
                sequencePtr = aldrin_sequence;
                sequenceSize = sizeof(aldrin_sequence) / sizeof(aldrin_sequence[0]);
                break;

            case UTF_BOBCAT2_E:
            case UTF_BOBCAT3_E:
            default:
                sequencePtr = bc2_sequence;
                sequenceSize = sizeof(bc2_sequence) / sizeof(bc2_sequence[0]);
                break;
        }

        vTcamInfo.hitNumber     = 0;
        vTcamInfo.autoResize    = GT_FALSE;
        vTcamInfo.ruleAdditionMethod =
            CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E;

        /* set new PCL rule */
        cpssOsMemSet(&pclMask, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
        cpssOsMemSet(&pclPattern, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
        cpssOsMemSet(&pclAction, 0, sizeof(CPSS_DXCH_PCL_ACTION_STC));

        /* set mask, pattern and action data */
        setRuleIngrUdbOnly(&pclMask, &pclPattern, &pclAction);

        /* set new TTI rule */
        cpssOsMemSet(&ttiMask, 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
        cpssOsMemSet(&ttiPattern, 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
        cpssOsMemSet(&ttiAction, 0, sizeof(CPSS_DXCH_TTI_ACTION_UNT));
        /* set mask, pattern and action data */
        setTtiRuleIpv4(&ttiMask, &ttiPattern, &ttiAction);

        cpssOsMemSet(&ruleAttributes, 0, sizeof(ruleAttributes));

        /* create VTCAMs and fill in PCL/TTI rules */
        for (idx = 0; idx < sequenceSize; idx++)
        {
            if (sequencePtr[idx].operation == VT_CREATE ||
                sequencePtr[idx].operation == VT_COMPLETE)
            {
                st = vtcamSequenceEntryProcess(sequencePtr, idx, vTcamMngId, &vTcamInfo);
                /* no free memory for new vTCAM */
                if  ((sequencePtr[idx].rc != vtcamSequenceEntryProcessLastResult)
                    && (vtcamSequenceEntryProcessLastResult == GT_FULL))
                {
                    break;
                }


                if (sequencePtr[idx].rc == GT_OK)
                {
                    /* new vTCAM created */
                    vTcamAdded = idx + 1;

                    /*  add rules to vTCAM */
                    if (sequencePtr[idx].clientGroup == CG_PCL)
                    {
                        /* set new PCL rule */
                        cpssOsMemSet(&pclMask, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
                        cpssOsMemSet(&pclPattern, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
                        cpssOsMemSet(&pclAction, 0, sizeof(CPSS_DXCH_PCL_ACTION_STC));

                        switch (sequencePtr[idx].ruleSize)
                        {
                            case RS_30:

                                setRuleStdNotIp(&pclMask, &pclPattern, &pclAction);
                                break;

                            case RS_60:

                                setRuleExtNotIpv6(&pclMask, &pclPattern, &pclAction);
                                break;

                            case RS_10:
                            case RS_20:
                            case RS_40:
                            case RS_50:
                            default:

                                setRuleIngrUdbOnly(&pclMask, &pclPattern, &pclAction);
                                break;

                        }
                    }
                    else
                    {
                        /* set new TTI rule */
                        cpssOsMemSet(&ttiMask, 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
                        cpssOsMemSet(&ttiPattern, 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
                        cpssOsMemSet(&ttiAction, 0, sizeof(CPSS_DXCH_TTI_ACTION_UNT));

                        switch (sequencePtr[idx].ruleSize)
                        {
                            case RS_10:
                            case RS_20:

                                setTtiRuleUdb(&ttiMask, &ttiPattern, &ttiAction);
                                break;

                            default:
                            case RS_30:

                                setTtiRuleIpv4(&ttiMask, &ttiPattern, &ttiAction);
                                break;
                        }
                    }
                    for (ruleIdx = 0; ruleIdx < vTcamInfo.guaranteedNumOfRules; ruleIdx++)
                    {
                        ruleId = ruleIdx;
                        st = vtcamSequenceRuleUpdate(vTcamMngId, sequencePtr, idx, ruleId,
                                                     &pclAction, &pclMask, &pclPattern,
                                                     &ttiAction, &ttiMask, &ttiPattern);
                        UTF_VERIFY_EQUAL2_STRING_MAC(
                            sequencePtr[idx].rc, st,
                            "vtcamSequenceRuleUpdate on vTcam(%d) sequence index(%d) failed",
                            sequencePtr[idx].vTcamId,idx);
                    }
                }
            }
        }

        UTF_VERIFY_NOT_EQUAL0_STRING_MAC(0, vTcamAdded, "failed adding vTcams");

        vTcamIdx = 6;
        if ((vTcamIdx >= sequenceSize) ||
            (sequencePtr[vTcamIdx].rc != GT_OK))
        {
            vTcamIdx = 0;
        }

        for (iteration = 0; iteration < 100; iteration++)
        {
            /* iterate 100 random rules */
            for (idx = 0; idx < 50; idx++)
            {
                dstTcamRule[idx].vTcamSeqId = vTcamIdx;
                dstTcamRule[idx].ruleId = 0xFFFFFFFF;
                dstTcamRule[idx].operation = VT_RULE_NONE;

                srcTcamRule[idx].vTcamSeqId = vTcamIdx;
                srcTcamRule[idx].ruleId = 0xFFFFFFFF;
                srcTcamRule[idx].operation = VT_RULE_NONE;


                ruleId  = cpssOsRand() % sequencePtr[vTcamIdx].rulesAmount;
                ruleId1 = cpssOsRand() % sequencePtr[vTcamIdx].rulesAmount;

                if (ruleId == ruleId1)
                {
                    continue;
                }
                duplicated = checkDuplicateRules(ruleId, srcTcamRule, dstTcamRule, idx + 1);
                if (duplicated == GT_TRUE)
                {
                    continue;
                }

                duplicated = checkDuplicateRules(ruleId1, srcTcamRule, dstTcamRule, idx + 1);
                if (duplicated == GT_TRUE)
                {
                    continue;
                }

                /* delete rule entry from vTcam */
                st = cpssDxChVirtualTcamRuleDelete(vTcamMngId,
                                                   sequencePtr[vTcamIdx].vTcamId, ruleId);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                             "cpssDxChVirtualTcamRuleDelete on RuleId(%d) failed", ruleId);

                /* get random rule from random vTcam*/
                dstTcamRule[idx].ruleId = ruleId;
                dstTcamRule[idx].operation = VT_RULE_DELETED;

                /* no duplicated deleted rule */
                /* get random rule from random vTcam*/
                srcTcamRule[idx].ruleId = ruleId1;
            }

            vTcamIdx = srcTcamRule[0].vTcamSeqId;
            if (sequencePtr[vTcamIdx].clientGroup == CG_PCL)
            {
                tcamRuleType.ruleType = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;
                tcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E;

                cpssOsMemSet(&pclMaskGet, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
                cpssOsMemSet(&pclPatternGet, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
                cpssOsMemSet(&pclActionGet, 0, sizeof(CPSS_DXCH_PCL_ACTION_STC));

                tcamRuleDataGet.rule.pcl.actionPtr      = &pclActionGet;
                tcamRuleDataGet.rule.pcl.maskPtr        = &pclMaskGet;
                tcamRuleDataGet.rule.pcl.patternPtr     = &pclPatternGet;

                switch (sequencePtr[vTcamIdx].ruleSize)
                {
                    case RS_10:
                            tcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E;
                            break;
                    case RS_20:
                            tcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E;
                            break;
                    case RS_30:
                            tcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
                            break;
                    case RS_40:
                            tcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_40_E;
                            break;
                    case RS_50:
                            tcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_50_E;
                            break;
                    case RS_60:
                            tcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E;
                            break;
                    default:
                            tcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E;
                            break;

                }
            }
            else
            {
                tcamRuleType.ruleType = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_TTI_E;
                tcamRuleType.rule.tti.ruleFormat = CPSS_DXCH_TTI_RULE_UDB_10_E;

                cpssOsMemSet(&ttiMaskGet, 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
                cpssOsMemSet(&ttiPatternGet, 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
                cpssOsMemSet(&ttiActionGet, 0, sizeof(CPSS_DXCH_TTI_ACTION_UNT));

                tcamRuleDataGet.rule.tti.actionPtr      = &ttiActionGet;
                tcamRuleDataGet.rule.tti.maskPtr        = &ttiMaskGet;
                tcamRuleDataGet.rule.tti.patternPtr     = &ttiPatternGet;

                switch (sequencePtr[vTcamIdx].ruleSize)
                {
                    case RS_10:
                        tcamRuleType.rule.tti.ruleFormat = CPSS_DXCH_TTI_RULE_UDB_10_E;
                        break;
                    case RS_20:
                        tcamRuleType.rule.tti.ruleFormat = CPSS_DXCH_TTI_RULE_UDB_20_E;
                        break;
                    default:
                    case RS_30:
                        tcamRuleType.rule.tti.ruleFormat = CPSS_DXCH_TTI_RULE_IPV4_E;
                        break;

                }
            }

            /* move 50 rules to free places */
            for (idx = 0; idx < 50; idx++)
            {
                if (srcTcamRule[idx].ruleId != 0xFFFFFFFF)
                {
                    st = cpssDxChVirtualTcamRuleMove(vTcamMngId, sequencePtr[vTcamIdx].vTcamId,
                                                     srcTcamRule[idx].ruleId, dstTcamRule[idx].ruleId);
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                                 "cpssDxChVirtualTcamRuleMove on ruleId(%d) failed \n", srcTcamRule[idx].ruleId);

                    /* read vTcam rule after move */
                    st = cpssDxChVirtualTcamRuleRead(vTcamMngId, sequencePtr[vTcamIdx].vTcamId,
                                                     dstTcamRule[idx].ruleId, &tcamRuleType, &tcamRuleDataGet);
                    UTF_VERIFY_EQUAL0_STRING_MAC(
                        GT_OK, st, "cpssDxChVirtualTcamRuleRead failed\n");

                    /* get the unique value that each rule got */
                    if (sequencePtr[vTcamIdx].clientGroup == CG_PCL)
                    {
                        UNIQUE_PCL_RULE_GET_MAC(&tcamRuleType,tcamRuleDataGet.rule.pcl.patternPtr, tmpValue);
                    }
                    else
                    {
                        UNIQUE_TTI_RULE_GET_MAC(&tcamRuleType,tcamRuleDataGet.rule.tti.patternPtr, tmpValue);
                    }

                    UTF_VERIFY_EQUAL0_STRING_MAC(srcTcamRule[idx].ruleId, tmpValue, "Wrong rule content");

                    /* update rule data after move */
                    st = vtcamSequenceRuleUpdate(vTcamMngId, sequencePtr, vTcamIdx,
                                                 dstTcamRule[idx].ruleId,
                                                 &pclAction, &pclMask, &pclPattern,
                                                 &ttiAction, &ttiMask, &ttiPattern);

                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                                 "vtcamSequenceRuleUpdate on vTcam sequence index(%d) failed", idx);

                    srcTcamRule[idx].operation = VT_RULE_MOVED;
                }
            }

            /*  restore "empty" rules to vTCAM */
            for (idx = 0; idx < 50; idx++)
            {
                /* rule was moved*/
                if (srcTcamRule[idx].operation == VT_RULE_MOVED)
                {
                    vTcamIdx = srcTcamRule[idx].vTcamSeqId;
                    st = vtcamSequenceRuleUpdate(vTcamMngId, sequencePtr, vTcamIdx, srcTcamRule[idx].ruleId,
                                                 &pclAction, &pclMask, &pclPattern,
                                                 &ttiAction, &ttiMask, &ttiPattern);
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                                 "vtcamSequenceRuleUpdate on vTcam sequence index(%d) failed", vTcamIdx);

                }
            }
        }

        for (idx = 0; idx < vTcamAdded; idx++)
        {
            if ((sequencePtr[idx].operation == VT_CREATE ||
                sequencePtr[idx].operation == VT_COMPLETE) &&
                (sequencePtr[idx].rc == GT_OK))
            {
                st = cpssDxChVirtualTcamUsageGet(vTcamMngId, sequencePtr[idx].vTcamId, &vTcamUsage);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamUsageGet failed rulesUsed = %d / rulesFrees = %d\n",
                                             vTcamUsage.rulesUsed, vTcamUsage.rulesFree);
                /*  remove rules from vTCAM */
                for (ruleIdx = 0; ruleIdx < vTcamUsage.rulesUsed; ruleIdx++)
                {
                    ruleId = ruleIdx;
                    /* delete rule entry from vTcam */
                    st = cpssDxChVirtualTcamRuleDelete(vTcamMngId,
                                                       sequencePtr[idx].vTcamId, ruleId);
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                                 "cpssDxChVirtualTcamRuleDelete on RuleId(%d) failed \n", ruleId);
                }

                st = ut_cpssDxChVirtualTcamRemove(vTcamMngId, sequencePtr[idx].vTcamId);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamRemove failed\n");
            }
        }

        st = cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListRemove failed for vTcamMngId = %d\n",
                                     vTcamMngId);
        st = cpssDxChVirtualTcamManagerDelete(vTcamMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDelete failed for vTcamMngId = %d\n",
                                     vTcamMngId);

    }

    if (isTestSkipped)
    {
        SKIP_TEST_MAC;
    }
}


/*******************************************************************************
* cpssDxChVirtualTcam_ConvertRuleIdToHwIndex
*
* DESCRIPTION:
*       Test debug function ‘convert ruleId to HW index’(logical index and priority)
* PURPOSE:
*       Check that the API cpssDxChVirtualTcamDbRuleIdToHwIndexConvert return
*       proper index that can be used with the PCL/TTI CPSS APIs to retrieve
*       entry from the HW .
* LOGIC:
*       1. Create many vtcams in tcam see table 1 (part 1 , without remove vtcams)
*       2. Fill all 36K entries with valid proper PCL/TTI rules (cpssDxChVirtualTcamRuleWrite)
*       3. Loop on 100 Randomize ruleId (from TTI and PCL)
*          a. Convert the ruleId to ‘HW index’ (cpssDxChVirtualTcamDbRuleIdToHwIndexConvert)
*          b. Read from HW the rule using the ‘HW index’
*             via CPSS TTI/PCL APIs ( cpssDxChTtiRuleGet  / cpssDxChPclRuleParsedGet )
*          c. Check that the content is as expected.(as written by cpssDxChVirtualTcamRuleWrite)
*
*******************************************************************************/
UTF_TEST_CASE_MAC(cpssDxChVirtualTcam_ConvertRuleIdToHwIndex)
{
    GT_STATUS                                  st   = GT_OK;
    GT_U8                                      dev;
    GT_U32                                     vTcamMngId;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ID             ruleId = 0;
    GT_U32                                     idx, vTcamIdx;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC ruleAttributes;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC       ruleData;
    CPSS_DXCH_PCL_ACTION_STC                   pclAction;
    CPSS_DXCH_PCL_ACTION_STC                   pclActionGet;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              pclMask;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              pclPattern;
    CPSS_DXCH_TTI_ACTION_UNT                   ttiAction;
    CPSS_DXCH_TTI_RULE_UNT                     ttiMask;
    CPSS_DXCH_TTI_RULE_UNT                     ttiPattern;
    GT_U32                                     numOfDevs = 0;
    GT_U32                                     notAppFamilyBmp = 0;
    GT_U32                                     utfFamilyBit = 0;
    GT_BOOL                                    isTestSkipped = GT_TRUE;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_TYPE_STC   actionType;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_DATA_STC   actionData;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC            vTcamInfo;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC            vTcamInfoGet;
    VT_SEQUENCE                               *sequencePtr;
    GT_U32                                     sequenceSize;
    GT_U32                                     ruleIdx;
    GT_U32                                     vTcamAdded = 0;
    CPSS_DXCH_VIRTUAL_TCAM_USAGE_STC           vTcamUsage;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC       TcamRuleType;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC       pclRuleData_forRead;
    CPSS_DXCH_PCL_ACTION_STC                   pclAction_forRead;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              pclMask_forRead;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              pclPattern_forRead;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC       ttiRuleData_forRead;
    CPSS_DXCH_TTI_ACTION_UNT                   ttiAction_forRead;
    CPSS_DXCH_TTI_RULE_UNT                     ttiMask_forRead;
    CPSS_DXCH_TTI_RULE_UNT                     ttiPattern_forRead;
    GT_U8                                      devNum;
    GT_U8                                      loopCounter;
    GT_U32                                     logicalIndex;
    GT_U32                                     hwIndex;
    GT_U16                                     tmpValue;
    GT_U8                                      firstDevNum;

    /* set new TTI rule for read */
    cpssOsMemSet(&ttiMask_forRead, 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
    cpssOsMemSet(&ttiPattern_forRead, 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
    cpssOsMemSet(&ttiAction_forRead, 0, sizeof(CPSS_DXCH_TTI_ACTION_UNT));

    ttiRuleData_forRead.rule.tti.actionPtr     = &ttiAction_forRead;
    ttiRuleData_forRead.rule.tti.maskPtr       = &ttiMask_forRead;
    ttiRuleData_forRead.rule.tti.patternPtr    = &ttiPattern_forRead;

    /* set new PCL rule for read */
    cpssOsMemSet(&pclMask_forRead, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
    cpssOsMemSet(&pclPattern_forRead, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
    cpssOsMemSet(&pclAction_forRead, 0, sizeof(CPSS_DXCH_PCL_ACTION_STC));

    pclRuleData_forRead.rule.pcl.actionPtr     = &pclAction_forRead;
    pclRuleData_forRead.rule.pcl.maskPtr       = &pclMask_forRead;
    pclRuleData_forRead.rule.pcl.patternPtr    = &pclPattern_forRead;


    cpssOsMemSet(&vTcamInfo, 0, sizeof(vTcamInfo));
    cpssOsMemSet(&vTcamInfoGet, 0, sizeof(vTcamInfoGet));
    cpssOsMemSet(&actionType, 0, sizeof(actionType));
    cpssOsMemSet(&actionData, 0, sizeof(actionData));
    cpssOsMemSet(&pclActionGet, 0, sizeof(pclActionGet));
    cpssOsMemSet(&TcamRuleType, 0,  sizeof(TcamRuleType));

    vTcamMngId = 7;

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* this feature is on eArch devices, xCat3 and Lion2 */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    notAppFamilyBmp &= (~UTF_XCAT3_E) & (~UTF_LION2_E);

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* 1. Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
            devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }
        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;

        firstDevNum = devListArr[0];

        /* Create vTCAM manager */
        st = cpssDxChVirtualTcamManagerCreate(vTcamMngId);
        if (st == GT_ALREADY_EXIST)
        {
            st = GT_OK;
        }
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerCreate failed\n");


        st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId, devListArr, numOfDevs);
        if (st != GT_OK)
        {
            cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
            st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        }
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListAdd failed\n");

        switch (utfFamilyBit)
        {
            case UTF_XCAT3_E:
                sequencePtr = xcat3_sequence;
                sequenceSize = sizeof(xcat3_sequence) / sizeof(xcat3_sequence[0]);
                break;

            case UTF_LION2_E:
                sequencePtr = lion2_sequence;
                sequenceSize = sizeof(lion2_sequence) / sizeof(lion2_sequence[0]);
                break;

            case UTF_ALDRIN2_E:
            case UTF_FALCON_E:
            case UTF_CAELUM_E:
                sequencePtr = bobk_sequence;
                sequenceSize = sizeof(bobk_sequence) / sizeof(bobk_sequence[0]);
                break;

            case UTF_ALDRIN_E:
            case UTF_AC3X_E:
                sequencePtr = aldrin_sequence;
                sequenceSize = sizeof(aldrin_sequence) / sizeof(aldrin_sequence[0]);
                break;

            case UTF_BOBCAT2_E:
            case UTF_BOBCAT3_E:
            default:
                sequencePtr = bc2_sequence;
                sequenceSize = sizeof(bc2_sequence) / sizeof(bc2_sequence[0]);
                break;
        }

        vTcamInfo.hitNumber     = 0;
        vTcamInfo.autoResize    = GT_FALSE;
        vTcamInfo.ruleAdditionMethod =
            CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E;

        /* set new PCL rule */
        cpssOsMemSet(&pclMask, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
        cpssOsMemSet(&pclPattern, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
        cpssOsMemSet(&pclAction, 0, sizeof(CPSS_DXCH_PCL_ACTION_STC));
        /* set mask, pattern and action data */
        setRuleIngrUdbOnly(&pclMask, &pclPattern, &pclAction);

        /* set new TTI rule */
        cpssOsMemSet(&ttiMask, 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
        cpssOsMemSet(&ttiPattern, 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
        cpssOsMemSet(&ttiAction, 0, sizeof(CPSS_DXCH_TTI_ACTION_UNT));
        /* set mask, pattern and action data */
        setTtiRuleIpv4(&ttiMask, &ttiPattern, &ttiAction);

        cpssOsMemSet(&ruleAttributes, 0, sizeof(ruleAttributes));
        cpssOsMemSet(&ruleData, 0, sizeof(ruleData));

        ruleData.valid = GT_TRUE;

        /*AUTODOC: create VTCAMs and fill in PCL/TTI rules */
        for (idx = 0; idx < sequenceSize; idx++)
        {
            if (sequencePtr[idx].operation == VT_CREATE ||
                sequencePtr[idx].operation == VT_COMPLETE)
            {
                st = vtcamSequenceEntryProcess(sequencePtr, idx, vTcamMngId, &vTcamInfo);
                /* no free memory for new vTCAM */
                if  ((sequencePtr[idx].rc != vtcamSequenceEntryProcessLastResult)
                    && (vtcamSequenceEntryProcessLastResult == GT_FULL))
                {
                    break;
                }

                if (sequencePtr[idx].rc == GT_OK)
                {
                    /* new vTCAM created */
                    vTcamAdded = idx + 1;

                    /*  add rules to vTCAM */
                    if (sequencePtr[idx].clientGroup == CG_PCL)
                    {
                        /* set new PCL rule */
                        cpssOsMemSet(&pclMask, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
                        cpssOsMemSet(&pclPattern, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
                        cpssOsMemSet(&pclAction, 0, sizeof(CPSS_DXCH_PCL_ACTION_STC));

                        switch (sequencePtr[idx].ruleSize)
                        {
                            case RS_30:

                                setRuleStdNotIp(&pclMask, &pclPattern, &pclAction);
                                break;

                            case RS_60:

                                setRuleExtNotIpv6(&pclMask, &pclPattern, &pclAction);
                                break;

                            case RS_10:
                            case RS_20:
                            case RS_40:
                            case RS_50:
                            default:

                                setRuleIngrUdbOnly(&pclMask, &pclPattern, &pclAction);
                                break;
                        }
                    }
                    else
                    {
                        /* set new TTI rule */
                        cpssOsMemSet(&ttiMask, 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
                        cpssOsMemSet(&ttiPattern, 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
                        cpssOsMemSet(&ttiAction, 0, sizeof(CPSS_DXCH_TTI_ACTION_UNT));

                        switch (sequencePtr[idx].ruleSize)
                        {
                            case RS_10:
                            case RS_20:

                                setTtiRuleUdb(&ttiMask, &ttiPattern, &ttiAction);
                                break;

                            case RS_30:
                            default:

                                setTtiRuleIpv4(&ttiMask, &ttiPattern, &ttiAction);
                                break;
                        }
                    }
                    for (ruleIdx = 0; ruleIdx < vTcamInfo.guaranteedNumOfRules; ruleIdx++)
                    {
                        ruleId = ruleIdx;

                        st = vtcamSequenceRuleUpdate(vTcamMngId, sequencePtr, idx, ruleId,
                                                     &pclAction, &pclMask, &pclPattern,
                                                     &ttiAction, &ttiMask, &ttiPattern);
                        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                                     "vtcamSequenceRuleUpdate on vTcam sequence index(%d) failed", idx);
                    }
                }
            }
        }

        if (vTcamAdded)
        {
            /*AUTODOC: 3. Loop on 100 Randomize ruleId (from TTI and PCL). Loop counter in case if we can not increase ruleIdx */
            for (ruleIdx = 0, loopCounter = 0; ruleIdx < 100 && loopCounter < 200; loopCounter++)
            {
                /* vTcam was created */
                vTcamIdx = cpssOsRand() % vTcamAdded;
                if ((sequencePtr[vTcamIdx].operation == VT_CREATE ||
                    sequencePtr[vTcamIdx].operation == VT_COMPLETE) &&
                    (sequencePtr[vTcamIdx].rc == GT_OK))
                {
                    /* get random rule from random vTcam*/
                    ruleId = cpssOsRand() % sequencePtr[vTcamIdx].rulesAmount;

                    /*AUTODOC: 3a. Convert the ruleId to ‘HW index’ */
                    st = cpssDxChVirtualTcamDbRuleIdToHwIndexConvert(vTcamMngId, sequencePtr[vTcamIdx].vTcamId,
                                                                     ruleId,
                                                                     &logicalIndex, &hwIndex);
                    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamDbRuleIdToHwIndexConvert failed\n");

                    st = cpssDxChVirtualTcamInfoGet(vTcamMngId,sequencePtr[vTcamIdx].vTcamId,&vTcamInfo);
                    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,
                                            "cpssDxChVirtualTcamInfoGet failed");

                    /* get first devNum iterator */
                    devNum = firstDevNum;

                    /*AUTODOC: 3b.read from HW the rule using the ‘HW index’ via CPSS TTI/PCL APIs*/
                    switch(vTcamInfo.clientGroup)
                    {

                    }
                    if(vTcamInfo.clientGroup == CG_TTI)
                    {
                        TcamRuleType.ruleType            = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_TTI_E;

                        /*get TTI rule format */
                        st = cpssDxChVirtualTcamInfoGet(vTcamMngId,sequencePtr[vTcamIdx].vTcamId,&vTcamInfo);
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,
                                                "cpssDxChVirtualTcamInfoGet failed");
                        switch(vTcamInfo.ruleSize)
                        {
                            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_10_B_E:
                            default:
                                TcamRuleType.rule.tti.ruleFormat = CPSS_DXCH_TTI_RULE_UDB_10_E;
                                break;
                            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_20_B_E:
                                TcamRuleType.rule.tti.ruleFormat = CPSS_DXCH_TTI_RULE_UDB_20_E;
                                break;
                            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E:
                                TcamRuleType.rule.tti.ruleFormat = CPSS_DXCH_TTI_RULE_IPV4_E;
                                break;
                        }

                        st = cpssDxChTtiRuleGet(devNum,
                                        hwIndex,
                                        TcamRuleType.rule.tti.ruleFormat,
                                        ttiRuleData_forRead.rule.tti.patternPtr,
                                        ttiRuleData_forRead.rule.tti.maskPtr,
                                        CPSS_DXCH_TTI_ACTION_TYPE2_ENT,
                                        ttiRuleData_forRead.rule.tti.actionPtr
                                        );
                        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                "cpssDxChTtiRuleGet failed on hwIndex[%d]",hwIndex);

                        st = cpssDxChTtiRuleValidStatusGet(devNum,
                                      hwIndex,
                                      &ttiRuleData_forRead.valid);
                        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                "cpssDxChTtiRuleValidStatusGet failed on hwIndex[%d]",hwIndex);

                        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, ttiRuleData_forRead.valid,
                            "cpssDxChTtiRuleGet : expected valid entry hwIndex[%d] \n",hwIndex);

                        /* get the unique value that each rule got */
                        UNIQUE_TTI_RULE_GET_MAC(&TcamRuleType,ttiRuleData_forRead.rule.tti.patternPtr,tmpValue);

                        UTF_VERIFY_EQUAL0_STRING_MAC(ruleId, tmpValue, "wrong rule content");
                    }
                    else if (vTcamInfo.clientGroup == CG_PCL)
                    {
                        TcamRuleType.ruleType            = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;

                        /*get PCL rule format */

                        st = cpssDxChVirtualTcamInfoGet(vTcamMngId,sequencePtr[vTcamIdx].vTcamId,&vTcamInfo);
                        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,
                                                "cpssDxChVirtualTcamInfoGet failed");
                        switch(vTcamInfo.ruleSize)
                        {
                            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_10_B_E:
                            default:
                                TcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E;
                                break;
                            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_20_B_E:
                                TcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E;
                                break;
                            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E:
                                TcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
                                break;
                            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_40_B_E:
                                TcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_40_E;
                                break;
                            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_50_B_E:
                                TcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_50_E;
                                break;
                            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_60_B_E:
                                TcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E;
                                break;
                            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E:
                                TcamRuleType.rule.pcl.ruleFormat =
                                    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E;
                                break;
                        }

                        if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_FALSE)
                        {
                            switch(vTcamInfo.ruleSize)
                            {
                                case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E:
                                    break;
                                case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_60_B_E:
                                    hwIndex /= 2;
                                    break;
                                case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E:
                                    hwIndex /= 4;
                                    break;
                                default:
                                    /* ignore */
                                    break;
                            }
                        }

                        st = cpssDxChPclRuleParsedGet(devNum,
                            TcamRuleType.rule.pcl.ruleFormat,
                            hwIndex,
                            0,/*ruleOptionsBmp -- unused parameter */
                            &pclRuleData_forRead.valid,
                            pclRuleData_forRead.rule.pcl.maskPtr,
                            pclRuleData_forRead.rule.pcl.patternPtr,
                            pclRuleData_forRead.rule.pcl.actionPtr
                            );

                        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, pclRuleData_forRead.valid,
                            "cpssDxChPclRuleParsedGet : expected valid entry hwIndex[%d] \n",hwIndex);

                        /* get the unique value that each rule got */
                        UNIQUE_PCL_RULE_GET_MAC(&TcamRuleType, pclRuleData_forRead.rule.pcl.patternPtr,tmpValue);

                        UTF_VERIFY_EQUAL0_STRING_MAC(ruleId, tmpValue, "wrong rule content");
                    }
                    else
                    {
                       continue;
                    }

                    checkCpssDbIsOk(vTcamMngId,sequencePtr[vTcamIdx].vTcamId);
                    ruleIdx++;
                }
            }
        }

        for (idx = 0; idx < vTcamAdded; idx++)
        {
            if ((sequencePtr[idx].operation == VT_CREATE ||
                sequencePtr[idx].operation == VT_COMPLETE) &&
                (sequencePtr[idx].rc == GT_OK))
            {
                st = cpssDxChVirtualTcamUsageGet(vTcamMngId, sequencePtr[idx].vTcamId, &vTcamUsage);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamUsageGet failed rulesUsed = %d / rulesFrees = %d\n",
                                             vTcamUsage.rulesUsed, vTcamUsage.rulesFree);
                /*  remove rules from vTCAM */
                for (ruleIdx = 0; ruleIdx < vTcamUsage.rulesUsed; ruleIdx++)
                {
                    ruleId = ruleIdx;
                    /* delete rule entry from vTcam */
                    st = cpssDxChVirtualTcamRuleDelete(vTcamMngId,
                                                       sequencePtr[idx].vTcamId, ruleId);
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                                 "cpssDxChVirtualTcamRuleDelete on RuleId(%d) failed \n", ruleId);
                }

                st = ut_cpssDxChVirtualTcamRemove(vTcamMngId, sequencePtr[idx].vTcamId);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamRemove failed\n");
            }
        }

        st = cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListRemove failed for vTcamMngId = %d\n",
                                     vTcamMngId);
        st = cpssDxChVirtualTcamManagerDelete(vTcamMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDelete failed for vTcamMngId = %d\n",
                                     vTcamMngId);

    }

    if (isTestSkipped)
    {
        SKIP_TEST_MAC;
    }
}


UTF_TEST_CASE_MAC(cpssDxChVirtualTcam_writeDeleteRulesLogicalIndex)
{
    GT_STATUS                                  st   = GT_OK;
    GT_U8                                      dev;
    GT_U32                                     vTcamMngId;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ID             ruleId = 0;
    GT_U32                                     idx, vTcamIdx;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC ruleAttributes;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC       ruleData;
    CPSS_DXCH_PCL_ACTION_STC                   pclAction;
    CPSS_DXCH_PCL_ACTION_STC                   pclActionGet;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              pclMask;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              pclPattern;
    CPSS_DXCH_TTI_ACTION_UNT                   ttiAction;
    CPSS_DXCH_TTI_RULE_UNT                     ttiMask;
    CPSS_DXCH_TTI_RULE_UNT                     ttiPattern;
    GT_U32                                     numOfDevs = 0;
    GT_U32                                     notAppFamilyBmp = 0;
    GT_U32                                     utfFamilyBit = 0;
    GT_BOOL                                    isTestSkipped = GT_TRUE;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_TYPE_STC   actionType;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_DATA_STC   actionData;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC            vTcamInfo;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC            vTcamInfoGet;
    VT_RULE_ID                                 deletedTcamRule[100];
    VT_SEQUENCE                               *sequencePtr;
    GT_U32                                     sequenceSize;
    GT_U32                                     ruleIdx;
    GT_U32                                     vTcamAdded = 0;
    CPSS_DXCH_VIRTUAL_TCAM_USAGE_STC           vTcamUsage;

    cpssOsMemSet(&vTcamInfo, 0, sizeof(vTcamInfo));
    cpssOsMemSet(&vTcamInfoGet, 0, sizeof(vTcamInfoGet));
    cpssOsMemSet(&actionType, 0, sizeof(actionType));
    cpssOsMemSet(&actionData, 0, sizeof(actionData));
    cpssOsMemSet(&pclActionGet, 0, sizeof(pclActionGet));

    vTcamMngId = 7;

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* this feature is on eArch devices, xCat3 and Lion2 */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    notAppFamilyBmp &= (~UTF_XCAT3_E) & (~UTF_LION2_E);

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* 1. Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
            devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }
        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;

        /* Create vTCAM manager */
        st = cpssDxChVirtualTcamManagerCreate(vTcamMngId);
        if (st == GT_ALREADY_EXIST)
        {
            st = GT_OK;
        }
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerCreate failed\n");


        st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId, devListArr, numOfDevs);
        if (st != GT_OK)
        {
            cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
            st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        }
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListAdd failed\n");

        switch (utfFamilyBit)
        {
            case UTF_XCAT3_E:
                sequencePtr = xcat3_sequence;
                sequenceSize = sizeof(xcat3_sequence) / sizeof(xcat3_sequence[0]);
                break;

            case UTF_LION2_E:
                sequencePtr = lion2_sequence;
                sequenceSize = sizeof(lion2_sequence) / sizeof(lion2_sequence[0]);
                break;

            case UTF_ALDRIN2_E:
            case UTF_FALCON_E:
            case UTF_CAELUM_E:
                sequencePtr = bobk_sequence;
                sequenceSize = sizeof(bobk_sequence) / sizeof(bobk_sequence[0]);
                break;

            case UTF_ALDRIN_E:
            case UTF_AC3X_E:
                sequencePtr = aldrin_sequence;
                sequenceSize = sizeof(aldrin_sequence) / sizeof(aldrin_sequence[0]);
                break;

            case UTF_BOBCAT2_E:
            case UTF_BOBCAT3_E:
            default:
                sequencePtr = bc2_sequence;
                sequenceSize = sizeof(bc2_sequence) / sizeof(bc2_sequence[0]);
                break;
        }

        vTcamInfo.hitNumber     = 0;
        vTcamInfo.autoResize    = GT_FALSE;
        vTcamInfo.ruleAdditionMethod =
            CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E;

        /* set new PCL rule */
        cpssOsMemSet(&pclMask, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
        cpssOsMemSet(&pclPattern, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
        cpssOsMemSet(&pclAction, 0, sizeof(CPSS_DXCH_PCL_ACTION_STC));
        /* aet mask, pattern and action data */
        setRuleIngrUdbOnly(&pclMask, &pclPattern, &pclAction);

        /* set new TTI rule */
        cpssOsMemSet(&ttiMask, 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
        cpssOsMemSet(&ttiPattern, 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
        cpssOsMemSet(&ttiAction, 0, sizeof(CPSS_DXCH_TTI_ACTION_UNT));
        /* set mask, pattern and action data */
        setTtiRuleIpv4(&ttiMask, &ttiPattern, &ttiAction);

        cpssOsMemSet(&ruleAttributes, 0, sizeof(ruleAttributes));
        cpssOsMemSet(&ruleData, 0, sizeof(ruleData));

        ruleData.valid = GT_TRUE;

        /* create VTCAMs and fill in PCL/TTI rules */
        for (idx = 0; idx < sequenceSize; idx++)
        {
            if (sequencePtr[idx].operation == VT_CREATE ||
                sequencePtr[idx].operation == VT_COMPLETE)
            {
                st = vtcamSequenceEntryProcess(sequencePtr, idx, vTcamMngId, &vTcamInfo);
                /* no free memory for new vTCAM */
                if  ((sequencePtr[idx].rc != vtcamSequenceEntryProcessLastResult)
                    && (vtcamSequenceEntryProcessLastResult == GT_FULL))
                {
                    break;
                }

                if (sequencePtr[idx].rc == GT_OK)
                {
                    /* new vTCAM created */
                    vTcamAdded = idx + 1;

                    /*  add rules to vTCAM */
                    if (sequencePtr[idx].clientGroup == CG_PCL)
                    {
                        /* set new PCL rule */
                        cpssOsMemSet(&pclMask, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
                        cpssOsMemSet(&pclPattern, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
                        cpssOsMemSet(&pclAction, 0, sizeof(CPSS_DXCH_PCL_ACTION_STC));

                        switch (sequencePtr[idx].ruleSize)
                        {
                            case RS_30:

                                setRuleStdNotIp(&pclMask, &pclPattern, &pclAction);
                                break;

                            case RS_60:

                                setRuleExtNotIpv6(&pclMask, &pclPattern, &pclAction);
                                break;

                            case RS_10:
                            case RS_20:
                            case RS_40:
                            case RS_50:
                            default:

                                setRuleIngrUdbOnly(&pclMask, &pclPattern, &pclAction);
                                break;

                        }
                    }
                    else
                    {
                        /* set new TTI rule */
                        cpssOsMemSet(&ttiMask, 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
                        cpssOsMemSet(&ttiPattern, 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
                        cpssOsMemSet(&ttiAction, 0, sizeof(CPSS_DXCH_TTI_ACTION_UNT));

                        switch (sequencePtr[idx].ruleSize)
                        {
                            case RS_10:
                            case RS_20:

                                setTtiRuleUdb(&ttiMask, &ttiPattern, &ttiAction);
                                break;

                            case RS_30:
                            default:

                                setTtiRuleIpv4(&ttiMask, &ttiPattern, &ttiAction);
                                break;
                        }
                    }
                    for (ruleIdx = 0; ruleIdx < vTcamInfo.guaranteedNumOfRules; ruleIdx++)
                    {
                        ruleId = ruleIdx;
                        st = vtcamSequenceRuleUpdate(vTcamMngId, sequencePtr, idx, ruleId,
                                                     &pclAction, &pclMask, &pclPattern,
                                                     &ttiAction, &ttiMask, &ttiPattern);
                        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                                     "vtcamSequenceRuleUpdate on vTcam sequence index(%d) failed", idx);
                    }
                }
            }
        }

        if (vTcamAdded)
        {
            /* loop 10K times on remove/restore random number of 100 rules in random virtual TCAMs */
            for (idx = 0; idx < 1000; idx++)
            {
                /* delete 100 random rules */
                for (ruleIdx = 0; ruleIdx < 100; ruleIdx++)
                {
                    /* clear array of deleted rules */
                    deletedTcamRule[ruleIdx].vTcamSeqId = 0xFFFFFFFF;
                    deletedTcamRule[ruleIdx].ruleId = 0xFFFFFFFF;

                    /* vTcam was created */
                    vTcamIdx = cpssOsRand() % vTcamAdded;
                    if ((sequencePtr[vTcamIdx].operation == VT_CREATE ||
                        sequencePtr[vTcamIdx].operation == VT_COMPLETE) &&
                        (sequencePtr[vTcamIdx].rc == GT_OK))
                    {
                        deletedTcamRule[ruleIdx].vTcamSeqId = vTcamIdx;
                        /* get random rule from random vTcam*/
                        deletedTcamRule[ruleIdx].ruleId =
                            cpssOsRand() % sequencePtr[vTcamIdx].rulesAmount;
                        /* delete rule entry from vTcam */
                        st = cpssDxChVirtualTcamRuleDelete(vTcamMngId,
                                                           sequencePtr[vTcamIdx].vTcamId,
                                                           deletedTcamRule[ruleIdx].ruleId);
                        if (st != GT_NOT_FOUND)
                        {
                            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                                         "cpssDxChVirtualTcamRuleDelete on RuleId(%d) failed \n", deletedTcamRule[ruleIdx].ruleId);
                        }
                    }
                }
                /* restore 100 random rules */
                for (ruleIdx = 0; ruleIdx < 100; ruleIdx++)
                {
                    /* rule was deleted */
                    if (deletedTcamRule[ruleIdx].ruleId != 0xFFFFFFFF)
                    {
                        vTcamIdx = deletedTcamRule[ruleIdx].vTcamSeqId;
                        if (sequencePtr[vTcamIdx].clientGroup == CG_PCL)
                        {
                            /* set new PCL rule */
                            cpssOsMemSet(&pclMask, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
                            cpssOsMemSet(&pclPattern, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
                            cpssOsMemSet(&pclAction, 0, sizeof(CPSS_DXCH_PCL_ACTION_STC));

                            switch (sequencePtr[vTcamIdx].ruleSize)
                            {
                                case RS_30:

                                    setRuleStdNotIp(&pclMask, &pclPattern, &pclAction);
                                    break;

                                case RS_60:

                                    setRuleExtNotIpv6(&pclMask, &pclPattern, &pclAction);
                                    break;

                                case RS_10:
                                case RS_20:
                                case RS_40:
                                case RS_50:
                                default:

                                    setRuleIngrUdbOnly(&pclMask, &pclPattern, &pclAction);
                                    break;

                            }
                        }
                        else
                        {
                            /* set new TTI rule */
                            cpssOsMemSet(&ttiMask, 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
                            cpssOsMemSet(&ttiPattern, 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
                            cpssOsMemSet(&ttiAction, 0, sizeof(CPSS_DXCH_TTI_ACTION_UNT));

                            switch (sequencePtr[vTcamIdx].ruleSize)
                            {
                                case RS_10:
                                case RS_20:

                                    setTtiRuleUdb(&ttiMask, &ttiPattern, &ttiAction);
                                    break;

                                case RS_30:
                                default:

                                    setTtiRuleIpv4(&ttiMask, &ttiPattern, &ttiAction);
                                    break;
                            }
                        }
                        st = vtcamSequenceRuleUpdate(vTcamMngId, sequencePtr, vTcamIdx,
                                                     deletedTcamRule[ruleIdx].ruleId,
                                                     &pclAction, &pclMask, &pclPattern,
                                                     &ttiAction, &ttiMask, &ttiPattern);
                        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                                     "vtcamSequenceRuleUpdate on vTcam sequence index(%d) failed", vTcamIdx);

                    }
                }
            }
        }

        for (idx = 0; idx < vTcamAdded; idx++)
        {
            if ((sequencePtr[idx].operation == VT_CREATE ||
                sequencePtr[idx].operation == VT_COMPLETE) &&
                (sequencePtr[idx].rc == GT_OK))
            {
                st = cpssDxChVirtualTcamUsageGet(vTcamMngId, sequencePtr[idx].vTcamId, &vTcamUsage);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamUsageGet failed rulesUsed = %d / rulesFrees = %d\n",
                                             vTcamUsage.rulesUsed, vTcamUsage.rulesFree);
                /*  remove rules from vTCAM */
                for (ruleIdx = 0; ruleIdx < vTcamUsage.rulesUsed; ruleIdx++)
                {
                    ruleId = ruleIdx;
                    /* delete rule entry from vTcam */
                    st = cpssDxChVirtualTcamRuleDelete(vTcamMngId,
                                                       sequencePtr[idx].vTcamId, ruleId);
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                                 "cpssDxChVirtualTcamRuleDelete on RuleId(%d) failed \n", ruleId);
                }

                st = ut_cpssDxChVirtualTcamRemove(vTcamMngId, sequencePtr[idx].vTcamId);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamRemove failed\n");
            }
        }

        st = cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListRemove failed for vTcamMngId = %d\n",
                                     vTcamMngId);
        st = cpssDxChVirtualTcamManagerDelete(vTcamMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDelete failed for vTcamMngId = %d\n",
                                     vTcamMngId);
    }

    if (isTestSkipped)
    {
        SKIP_TEST_MAC;
    }
}

/* Test Case #8.    Test resize in logical index mode */
UTF_TEST_CASE_MAC(cpssDxChVirtualTcam_resizeRulesLogicalIndex)
{
    GT_STATUS                                  st   = GT_OK;
    GT_U8                                      dev;
    GT_U32                                     vTcamMngId;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ID             ruleId = 0;
    GT_U32                                     idx;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC ruleAttributes;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC       ruleData;
    CPSS_DXCH_PCL_ACTION_STC                   pclAction;
    CPSS_DXCH_PCL_ACTION_STC                   pclActionGet;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              pclMask;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              pclPattern;
    CPSS_DXCH_TTI_ACTION_UNT                   ttiAction;
    CPSS_DXCH_TTI_ACTION_UNT                   ttiActionGet;
    CPSS_DXCH_TTI_RULE_UNT                     ttiMask;
    CPSS_DXCH_TTI_RULE_UNT                     ttiPattern;
    GT_U32                                     numOfDevs = 0;
    GT_U32                                     notAppFamilyBmp = 0;
    GT_U32                                     utfFamilyBit = 0;
    GT_BOOL                                    isTestSkipped = GT_TRUE;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_TYPE_STC   actionType;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_DATA_STC   actionData;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC            vTcamInfo;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC            vTcamInfoGet;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC        tcamRuleType;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC        tcamRuleDataGet;
    VT_SEQUENCE                               *sequencePtr;
    VT_INCREASE_DECREASE_SEQUENCE             *incDecSequencePtr;
    VT_RULE_ID                                *srcSequencePtr;
    VT_RULE_ID                                *dstSequencePtr;
    GT_U32                                     sequenceSize;
    GT_U32                                     ruleIdx;
    GT_U32                                     vTcamAdded = 0;
    GT_U32                                      tmpValue;

    /* Create, Increase, CheckRule1, Decrease, CheckRule2 */
    /* We don't test the Full state - the correct rules drugging only */

    /*------------------- BC2/Bobk Data -------------------*/

    /* Create */
    static VT_SEQUENCE bc2_create_sequence[] =
    {
        {VT_CREATE,    5, CG_TTI,  RS_20,    1002, GT_OK},
        {VT_CREATE,    6, CG_TTI,  RS_30,    1000, GT_OK},
        {VT_COMPLETE,  7, CG_TTI,  RS_NONE,  1140, GT_OK},
        {VT_CREATE,    8, CG_PCL,  RS_20,    1002, GT_OK},
        {VT_CREATE,    9, CG_PCL,  RS_40,    2000, GT_OK},
        {VT_CREATE,   10, CG_PCL,  RS_50,    1000, GT_OK},
        {VT_CREATE,   11, CG_PCL,  RS_10,    7002, GT_OK},
        {VT_CREATE,   12, CG_PCL,  RS_60,     500, GT_OK},
        {VT_CREATE,   13, CG_PCL,  RS_30,     100, GT_OK},
        {VT_CREATE,   14, CG_PCL,  RS_50,     814, GT_OK},
        {VT_CREATE,   15, CG_PCL,  RS_10,     814, GT_OK},
        {VT_COMPLETE, 16, CG_PCL,  RS_NONE,   528, GT_OK},
        {VT_CREATE,   50, CG_TTI,  RS_10,       1, GT_FULL},
        {VT_CREATE,   51, CG_PCL,  RS_10,       1, GT_FULL},
        {VT_REMOVE,    6, CG_NONE, RS_NONE,  1000, GT_OK},
        {VT_REMOVE,    8, CG_NONE, RS_NONE,  1002, GT_OK},
        {VT_REMOVE,   10, CG_NONE, RS_NONE,  1000, GT_OK},
        {VT_REMOVE,   13, CG_NONE, RS_NONE,   100, GT_OK},
        {VT_REMOVE,   15, CG_NONE, RS_NONE,   814, GT_OK},
    };

    /* Sequence below is a subset of bobk_sequence: only part 1 + part 2 up to remove vTcams step */
    static VT_SEQUENCE bobk_create_sequence[] =
    {
        {VT_CREATE,    5, CG_TTI,  RS_20,     501, GT_OK},
        {VT_CREATE,    6, CG_TTI,  RS_30,     500, GT_OK},
        {VT_COMPLETE,  7, CG_TTI,  RS_NONE,   570, GT_OK},
        {VT_CREATE,    8, CG_PCL,  RS_20,     501, GT_OK},
        {VT_CREATE,    9, CG_PCL,  RS_40,    1000, GT_OK},
        {VT_CREATE,   10, CG_PCL,  RS_50,     500, GT_OK},
        {VT_CREATE,   11, CG_PCL,  RS_10,    3501, GT_OK},
        {VT_CREATE,   12, CG_PCL,  RS_60,     250, GT_OK},
        {VT_CREATE,   13, CG_PCL,  RS_30,      50, GT_OK},
        {VT_CREATE,   14, CG_PCL,  RS_50,     407, GT_OK},
        {VT_CREATE,   15, CG_PCL,  RS_10,     407, GT_OK},
        {VT_COMPLETE, 16, CG_PCL,  RS_NONE,   258, GT_OK},
        {VT_CREATE,   50, CG_TTI,  RS_10,       1, GT_FULL},
        {VT_CREATE,   51, CG_PCL,  RS_10,       1, GT_FULL},
        {VT_REMOVE,    6, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,    8, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,   10, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,   13, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,   15, CG_NONE, RS_NONE,     0, GT_OK},
    };

    /* Sequence below is a subset of aldrin_sequence: only part 1 + part 2 up to remove vTcams step */
    static VT_SEQUENCE aldrin_create_sequence[] =
    {
        {VT_CREATE,    5, CG_TTI,  RS_20,     252, GT_OK},
        {VT_CREATE,    6, CG_TTI,  RS_30,     250, GT_OK},
        {VT_COMPLETE,  7, CG_TTI,  RS_NONE,   282, GT_OK},
        {VT_CREATE,    8, CG_PCL,  RS_20,     252, GT_OK},
        {VT_CREATE,    9, CG_PCL,  RS_40,     500, GT_OK},
        {VT_CREATE,   10, CG_PCL,  RS_50,     250, GT_OK},
        {VT_CREATE,   11, CG_PCL,  RS_10,    1752, GT_OK},
        {VT_CREATE,   12, CG_PCL,  RS_60,     125, GT_OK},
        {VT_CREATE,   13, CG_PCL,  RS_30,      24, GT_OK},
        {VT_CREATE,   14, CG_PCL,  RS_50,     205, GT_OK},
        {VT_CREATE,   15, CG_PCL,  RS_10,     207, GT_OK},
        {VT_COMPLETE, 16, CG_PCL,  RS_NONE,   114, GT_OK},
        {VT_CREATE,   50, CG_TTI,  RS_10,       1, GT_FULL},
        {VT_CREATE,   51, CG_PCL,  RS_10,       1, GT_FULL},
        {VT_REMOVE,    6, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,    8, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,   10, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,   13, CG_NONE, RS_NONE,     0, GT_OK},
        {VT_REMOVE,   15, CG_NONE, RS_NONE,     0, GT_OK},
    };

    /* Sequence of increase operations - xcat3 */
    static VT_INCREASE_DECREASE_SEQUENCE xcat3_increase_sequence[] =
    {
        {VT_INCREASE,  9,   5,  500,  CPSS_DXCH_VIRTUAL_TCAM_BEFORE_START_CNS, GT_OK},
        {VT_INCREASE, 12,   8,  150,  100, GT_OK},
        {VT_INCREASE, 14,  10,   42,   15, GT_OK},
        {VT_INCREASE, 16,  16,   10,   50, GT_OK},
        {VT_INCREASE, 11,   7,  680,  CPSS_DXCH_VIRTUAL_TCAM_APPEND_CNS, GT_OK},
        {VT_INCREASE, 11,   7,    1,  CPSS_DXCH_VIRTUAL_TCAM_BEFORE_START_CNS, GT_FULL},
    };

    /* Sequence of increase operations - lion2 */
    static VT_INCREASE_DECREASE_SEQUENCE lion2_increase_sequence[] =
    {
        {VT_INCREASE,  9,   5,  334,  CPSS_DXCH_VIRTUAL_TCAM_BEFORE_START_CNS, GT_OK},
        {VT_INCREASE, 12,   8,  100,   66, GT_OK},
        {VT_INCREASE, 14,  10,   28,   10, GT_OK},
        {VT_INCREASE, 16,  16,    6,   30, GT_OK},
        {VT_INCREASE, 11,   7,  452,  CPSS_DXCH_VIRTUAL_TCAM_APPEND_CNS, GT_OK},
        {VT_INCREASE, 11,   7,    1,  CPSS_DXCH_VIRTUAL_TCAM_BEFORE_START_CNS, GT_FULL},
    };

    /* Sequence of increase operations - bc2/3 */
    static VT_INCREASE_DECREASE_SEQUENCE bc2_increase_sequence[] =
    {
        {VT_INCREASE,  5,   0,  996,  100, GT_OK},
        {VT_INCREASE,  7,   2, 1008,  500, GT_OK},
        {VT_INCREASE,  9,   4, 1000,  CPSS_DXCH_VIRTUAL_TCAM_BEFORE_START_CNS, GT_OK},
        {VT_INCREASE, 12,   7,  300,  200, GT_OK},
        {VT_INCREASE, 14,   9,  /*240*/84,  300, GT_OK},
        {VT_INCREASE, 16,  11,   18,  500, GT_OK},
        {VT_INCREASE, 11,   6,  /*1880*/1878,  CPSS_DXCH_VIRTUAL_TCAM_APPEND_CNS, GT_OK},
        {VT_INCREASE,  5,   0,    1,    1, GT_FULL},
        {VT_INCREASE, 11,   6,    1,  CPSS_DXCH_VIRTUAL_TCAM_BEFORE_START_CNS, GT_FULL},
    };

    /* Sequence of increase operations - bobk */
    static VT_INCREASE_DECREASE_SEQUENCE bobk_increase_sequence[] =
    {
        {VT_INCREASE,  5,   0,  498,   50, GT_OK},
        {VT_INCREASE,  7,   2,  504,  250, GT_OK},
        {VT_INCREASE,  9,   4,  500,  CPSS_DXCH_VIRTUAL_TCAM_BEFORE_START_CNS, GT_OK},
        {VT_INCREASE, 12,   7,  150,  100, GT_OK},
        {VT_INCREASE, 14,   9,   42,  150, GT_OK},
        {VT_INCREASE, 16,  11,    9,  250, GT_OK},
        {VT_INCREASE, 11,   6,  935,  CPSS_DXCH_VIRTUAL_TCAM_APPEND_CNS, GT_OK},
        {VT_INCREASE,  5,   0,    1,    1, GT_FULL},
        /* rules amount increased due to new resize alignment algorithm */
        {VT_INCREASE, 11,   6,    6,  CPSS_DXCH_VIRTUAL_TCAM_BEFORE_START_CNS, GT_FULL},
    };

    /* Sequence of increase operations - aldrin */
    static VT_INCREASE_DECREASE_SEQUENCE aldrin_increase_sequence[] =
    {
        {VT_INCREASE,  5,   0,  249,   25, GT_OK},
        {VT_INCREASE,  7,   2,  252,  125, GT_OK},
        {VT_INCREASE,  9,   4,  250,  CPSS_DXCH_VIRTUAL_TCAM_BEFORE_START_CNS, GT_OK},
        {VT_INCREASE, 12,   7,   75,   50, GT_OK},
        {VT_INCREASE, 14,   9,   21,   75, GT_OK},
        {VT_INCREASE, 16,  11,    6,  110, GT_OK},
        {VT_INCREASE, 11,   6,  474,  CPSS_DXCH_VIRTUAL_TCAM_APPEND_CNS, GT_OK},
        {VT_INCREASE,  5,   0,    1,    1, GT_FULL},
        {VT_INCREASE, 11,   6,    1,  CPSS_DXCH_VIRTUAL_TCAM_BEFORE_START_CNS, GT_FULL},
    };

    /* A couple of sequences to check correct moving of rules on increase operation - xcat3 */
    static VT_RULE_ID xcat3_srcTcamRule[] =
    {
        {   9,      0, VT_RULE_NONE},
        {  12,    149, VT_RULE_NONE},
        {  14,     15, VT_RULE_NONE},
        {  16,     50, VT_RULE_NONE},
        {  11,    499, VT_RULE_NONE},
    };

    static VT_RULE_ID xcat3_dstTcamRule[] =
    {
        {   9,     0 +  500, VT_RULE_NONE},
        {  12,   149 +  150, VT_RULE_NONE},
        {  14,    15 +   42, VT_RULE_NONE},
        {  16,    50 +   10, VT_RULE_NONE},
        {  11,   499 +    0, VT_RULE_NONE},
    };

    /* A couple of sequences to check correct moving of rules on increase operation - lion2 */
    static VT_RULE_ID lion2_srcTcamRule[] =
    {
        {   9,      0, VT_RULE_NONE},
        {  12,     99, VT_RULE_NONE},
        {  14,     10, VT_RULE_NONE},
        {  16,     30, VT_RULE_NONE},
        {  11,    332, VT_RULE_NONE},
    };

    static VT_RULE_ID lion2_dstTcamRule[] =
    {
        {   9,     0 +  334, VT_RULE_NONE},
        {  12,    99 +  100, VT_RULE_NONE},
        {  14,    10 +   28, VT_RULE_NONE},
        {  16,    30 +    6, VT_RULE_NONE},
        {  11,   332 +    0, VT_RULE_NONE},
    };

    /* A couple of sequences to check correct moving of rules on increase operation - bc2/3 */
    static VT_RULE_ID bc2_srcTcamRule[] =
    {
        {   9,      0, VT_RULE_NONE},
        {  12,    499, VT_RULE_NONE},
        {  14,    300, VT_RULE_NONE},
        {  16,    500, VT_RULE_NONE},
        {  11,   7001, VT_RULE_NONE},
        {   5,    101, VT_RULE_NONE},
        {   7,    501, VT_RULE_NONE},
    };

    static VT_RULE_ID bc2_dstTcamRule[] =
    {
        {   9,     0 + 1000, VT_RULE_NONE},
        {  12,   499 +  300, VT_RULE_NONE},
        {  14,   300 +   84, VT_RULE_NONE},
        {  16,   500 +   18, VT_RULE_NONE},
        {  11,  7001 +    0, VT_RULE_NONE},
        {   5,   101 +  996, VT_RULE_NONE},
        {   7,   501 + 1008, VT_RULE_NONE},
    };

    /* A couple of sequences to check correct moving of rules on increase operation - bobk */
    static VT_RULE_ID bobk_srcTcamRule[] =
    {
        {   9,      0, VT_RULE_NONE},
        {  12,    249, VT_RULE_NONE},
        {  14,    150, VT_RULE_NONE},
        {  16,    250, VT_RULE_NONE},
        {  11,   3500, VT_RULE_NONE},
        {   5,     51, VT_RULE_NONE},
        {   7,    250, VT_RULE_NONE},
    };

    static VT_RULE_ID bobk_dstTcamRule[] =
    {
        {   9,     0 +  500, VT_RULE_NONE},
        {  12,   249 +  150, VT_RULE_NONE},
        {  14,   150 +   42, VT_RULE_NONE},
        {  16,   250 +    9, VT_RULE_NONE},
        {  11,  3500 +    0, VT_RULE_NONE},
        {   5,    51 +  498, VT_RULE_NONE},
        {   7,   250 +  504, VT_RULE_NONE},
    };

    /* A couple of sequences to check correct moving of rules on increase operation - aldrin */
    static VT_RULE_ID aldrin_srcTcamRule[] =
    {
        {   9,      0, VT_RULE_NONE},
        {  12,    124, VT_RULE_NONE},
        {  14,     75, VT_RULE_NONE},
        {  16,    110, VT_RULE_NONE},
        {  11,   1750, VT_RULE_NONE},
        {   5,     25, VT_RULE_NONE},
        {   7,    125, VT_RULE_NONE},
    };

    static VT_RULE_ID aldrin_dstTcamRule[] =
    {
        {   9,     0 +  250, VT_RULE_NONE},
        {  12,   124 +   75, VT_RULE_NONE},
        {  14,    75 +   21, VT_RULE_NONE},
        {  16,   110 +    6, VT_RULE_NONE},
        {  11,  1750 +    0, VT_RULE_NONE},
        {   5,    25 +  249, VT_RULE_NONE},
        {   7,   125 +  252, VT_RULE_NONE},
    };


    /* Sequence to check correct increase operation by writing to the end - xcat3 */
    static VT_SEQUENCE xcat3_check_increase_sequence[] =
    {
        {VT_NO_MORE,    9, CG_PCL,  RS_30,     500 +  500 - 1, GT_OK},
        {VT_NO_MORE,   12, CG_PCL,  RS_60,     150 +  150 - 1, GT_OK},
        {VT_NO_MORE,   14, CG_PCL,  RS_30,     100 +   42 - 1, GT_OK},
        {VT_NO_MORE,   16, CG_PCL,  RS_30,     138 +   10 - 1, GT_OK},
        {VT_NO_MORE,   11, CG_PCL,  RS_30,     500 +  680 - 1, GT_OK},
    };

    /* Sequence to check correct increase operation by writing to the end - lion2 */
    static VT_SEQUENCE lion2_check_increase_sequence[] =
    {
        {VT_NO_MORE,    9, CG_PCL,  RS_30,     332 +  334 - 1, GT_OK},
        {VT_NO_MORE,   12, CG_PCL,  RS_60,     100 +  100 - 1, GT_OK},
        {VT_NO_MORE,   14, CG_PCL,  RS_30,      68 +   28 - 1, GT_OK},
        {VT_NO_MORE,   16, CG_PCL,  RS_30,      32 +    6 - 1, GT_OK},
        {VT_NO_MORE,   11, CG_PCL,  RS_30,     332 +  452 - 1, GT_OK},
    };

    /* Sequence to check correct increase operation by writing to the end - bc2/3 */
    static VT_SEQUENCE bc2_check_increase_sequence[] =
    {
        {VT_NO_MORE,    9, CG_PCL,  RS_40,    2000 + 1000 - 1, GT_OK},
        {VT_NO_MORE,   12, CG_PCL,  RS_60,     500 +  300 - 1, GT_OK},
        {VT_NO_MORE,   14, CG_PCL,  RS_50,     814 +   84 - 1, GT_OK},
        {VT_NO_MORE,   16, CG_PCL,  RS_10,     528 +   18 - 1, GT_OK},
        {VT_NO_MORE,   11, CG_PCL,  RS_10,    7002 + 1878 - 1, GT_OK},
        {VT_NO_MORE,    5, CG_TTI,  RS_20,    1002 +  996 - 1, GT_OK},
        {VT_NO_MORE,    7, CG_TTI,  RS_10,    1140 + 1008 - 1, GT_OK},
    };

    /* Sequence to check correct increase operation by writing to the end - bobk */
    static VT_SEQUENCE bobk_check_increase_sequence[] =
    {
        {VT_NO_MORE,    9, CG_PCL,  RS_40,    1000 +  500 - 1, GT_OK},
        {VT_NO_MORE,   12, CG_PCL,  RS_60,     250 +  150 - 1, GT_OK},
        {VT_NO_MORE,   14, CG_PCL,  RS_50,     407 +   42 - 1, GT_OK},
        {VT_NO_MORE,   16, CG_PCL,  RS_10,     250 +    9 - 1, GT_OK},
        {VT_NO_MORE,   11, CG_PCL,  RS_10,    3501 +  935 - 1, GT_OK},
        {VT_NO_MORE,    5, CG_TTI,  RS_20,     501 +  498 - 1, GT_OK},
        {VT_NO_MORE,    7, CG_TTI,  RS_10,     570 +  504 - 1, GT_OK},
    };

    /* Sequence to check correct increase operation by writing to the end - aldrin */
    static VT_SEQUENCE aldrin_check_increase_sequence[] =
    {
        {VT_NO_MORE,    9, CG_PCL,  RS_40,     500 +  250 - 1, GT_OK},
        {VT_NO_MORE,   12, CG_PCL,  RS_60,     125 +   75 - 1, GT_OK},
        {VT_NO_MORE,   14, CG_PCL,  RS_50,     203 +   21 - 1, GT_OK},
        {VT_NO_MORE,   16, CG_PCL,  RS_10,     114 +    6 - 1, GT_OK},
        {VT_NO_MORE,   11, CG_PCL,  RS_10,    1750 +  474 - 1, GT_OK},
        {VT_NO_MORE,    5, CG_TTI,  RS_20,     250 +  249 - 1, GT_OK},
        {VT_NO_MORE,    7, CG_TTI,  RS_10,     282 +  252 - 1, GT_OK},
    };

    /* Sequence of decrease operations - xcat3 */
    static VT_INCREASE_DECREASE_SEQUENCE xcat3_decrease_sequence[] =
    {
        {VT_DECREASE,  9,   5,  500 + 10,    0, GT_OK},
        {VT_DECREASE, 12,   8,  150 + 10,    0, GT_OK},
        {VT_DECREASE, 14,  10,   42 + 10,    0, GT_OK},
        {VT_DECREASE, 16,  16,   10 + 10,    0, GT_OK},
        {VT_DECREASE, 11,   7,  680 + 10,    0, GT_OK},
    };

    /* Sequence of decrease operations - lion2 */
    static VT_INCREASE_DECREASE_SEQUENCE lion2_decrease_sequence[] =
    {
        {VT_DECREASE,  9,   5,  334 + 10,    0, GT_OK},
        {VT_DECREASE, 12,   8,  100 + 10,    0, GT_OK},
        {VT_DECREASE, 14,  10,   28 + 10,    0, GT_OK},
        {VT_DECREASE, 16,  16,    6 + 10,    0, GT_OK},
        {VT_DECREASE, 11,   7,  452 + 10,    0, GT_OK},
    };

    /* Sequence of decrease operations - bc2/3 */
    static VT_INCREASE_DECREASE_SEQUENCE bc2_decrease_sequence[] =
    {
        {VT_DECREASE,  5,   0,  996 + 100,    0, GT_OK},
        {VT_DECREASE,  7,   2, 1008 + 100,    0, GT_OK},
        {VT_DECREASE,  9,   4, 1000 + 100,    0, GT_OK},
        {VT_DECREASE, 12,   7,  300 + 100,    0, GT_OK},
        {VT_DECREASE, 14,   9,   84 + 100,    0, GT_OK},
        {VT_DECREASE, 16,  11,   18 + 100,    0, GT_OK},
        {VT_DECREASE, 11,   6, 1878 + 100,    0, GT_OK},
    };

    /* Sequence of decrease operations - bobk */
    static VT_INCREASE_DECREASE_SEQUENCE bobk_decrease_sequence[] =
    {
        {VT_DECREASE,  5,   0,  498 + 50,    0, GT_OK},
        {VT_DECREASE,  7,   2,  504 + 50,    0, GT_OK},
        {VT_DECREASE,  9,   4,  500 + 50,    0, GT_OK},
        {VT_DECREASE, 12,   7,  150 + 50,    0, GT_OK},
        {VT_DECREASE, 14,   9,   42 + 50,    0, GT_OK},
        {VT_DECREASE, 16,  11,    9 + 50,    0, GT_OK},
        {VT_DECREASE, 11,   6,  935 + 50,    0, GT_OK},
    };

    /* Sequence of decrease operations - aldrin */
    static VT_INCREASE_DECREASE_SEQUENCE aldrin_decrease_sequence[] =
    {
        {VT_DECREASE,  5,   0,  249 + 25,    0, GT_OK},
        {VT_DECREASE,  7,   2,  252 + 25,    0, GT_OK},
        {VT_DECREASE,  9,   4,  250 + 25,    0, GT_OK},
        {VT_DECREASE, 12,   7,   75 + 25,    0, GT_OK},
        {VT_DECREASE, 14,   9,   21 + 25,    0, GT_OK},
        {VT_DECREASE, 16,  11,    6 + 25,    0, GT_OK},
        {VT_DECREASE, 11,   6,  474 + 25,    0, GT_OK},
    };

    /* Sequence to check correct decrease operation by writing to the end - xcat3 */
    static VT_SEQUENCE xcat3_check_decrease_sequence[] =
    {
        {VT_NO_MORE,    9, CG_PCL,  RS_30,     500 - 10 - 1, GT_OK},
        {VT_NO_MORE,   12, CG_PCL,  RS_60,     150 - 10 - 1, GT_OK},
        {VT_NO_MORE,   14, CG_PCL,  RS_30,     100 - 10 - 1, GT_OK},
        {VT_NO_MORE,   16, CG_PCL,  RS_30,     138 - 10 - 1, GT_OK},
        {VT_NO_MORE,   11, CG_PCL,  RS_30,     500 - 10 - 1, GT_OK},
    };

    /* Sequence to check correct decrease operation by writing to the end - lion2 */
    static VT_SEQUENCE lion2_check_decrease_sequence[] =
    {
        {VT_NO_MORE,    9, CG_PCL,  RS_30,     332 - 10 - 1, GT_OK},
        {VT_NO_MORE,   12, CG_PCL,  RS_60,     100 - 10 - 1, GT_OK},
        {VT_NO_MORE,   14, CG_PCL,  RS_30,      68 - 10 - 1, GT_OK},
        {VT_NO_MORE,   16, CG_PCL,  RS_30,      32 - 10 - 1, GT_OK},
        {VT_NO_MORE,   11, CG_PCL,  RS_30,     332 - 10 - 1, GT_OK},
    };

    /* Sequence to check correct decrease operation by writing to the end - bc2/3 */
    static VT_SEQUENCE bc2_check_decrease_sequence[] =
    {
        {VT_NO_MORE,    9, CG_PCL,  RS_40,    2000 - 100 - 1, GT_OK},
        {VT_NO_MORE,   12, CG_PCL,  RS_60,     500 - 100 - 1, GT_OK},
        {VT_NO_MORE,   14, CG_PCL,  RS_50,     814 - 100 - 1, GT_OK},
        {VT_NO_MORE,   16, CG_PCL,  RS_10,     528 - 100 - 1, GT_OK},
        {VT_NO_MORE,   11, CG_PCL,  RS_10,    7002 - 100 - 1, GT_OK},
        {VT_NO_MORE,    5, CG_TTI,  RS_20,    ((((1002 - 100) + (3-1))/3)*3) - 1, GT_OK},
        {VT_NO_MORE,    7, CG_TTI,  RS_10,    ((((1140 - 100) + (6-1))/6)*6) - 1, GT_OK},
    };

    /* Sequence to check correct decrease operation by writing to the end - bobk */
    static VT_SEQUENCE bobk_check_decrease_sequence[] =
    {
        {VT_NO_MORE,    9, CG_PCL,  RS_40,    1000 - 50 - 1, GT_OK},
        {VT_NO_MORE,   12, CG_PCL,  RS_60,     250 - 50 - 1, GT_OK},
        {VT_NO_MORE,   14, CG_PCL,  RS_50,     407 - 50 - 1, GT_OK},
        {VT_NO_MORE,   16, CG_PCL,  RS_10,     250 - 50 - 1, GT_OK},
        {VT_NO_MORE,   11, CG_PCL,  RS_10,    3501 - 50 - 1, GT_OK},
        {VT_NO_MORE,    5, CG_TTI,  RS_20,     ((((501 - 50) + (3-1))/3)*3)-1, GT_OK},
        {VT_NO_MORE,    7, CG_TTI,  RS_10,    ((((570 - 50) + (6-1))/6)*6)-1, GT_OK},
    };

    /* Sequence to check correct decrease operation by writing to the end - aldrin */
    static VT_SEQUENCE aldrin_check_decrease_sequence[] =
    {
        {VT_NO_MORE,    9, CG_PCL,  RS_40,     250 - 25 - 1, GT_OK},
        {VT_NO_MORE,   12, CG_PCL,  RS_60,     125 - 25 - 1, GT_OK},
        {VT_NO_MORE,   14, CG_PCL,  RS_50,     203 - 25 - 1, GT_OK},
        {VT_NO_MORE,   16, CG_PCL,  RS_10,     114 - 25 - 1, GT_OK},
        {VT_NO_MORE,   11, CG_PCL,  RS_10,    1750 - 25 - 1, GT_OK},
        {VT_NO_MORE,    5, CG_TTI,  RS_20,     ((((250 - 25) + (3-1))/3)*3)-1, GT_OK},
        {VT_NO_MORE,    7, CG_TTI,  RS_10,    ((((282 - 25) + (6-1))/6)*6)-1, GT_OK},
    };

    /* Sequence to check correct decrease operation by creating new vTcam on deallocated space */
    static VT_SEQUENCE xcat3_create_after_decrease_sequence[] =
    {
        {VT_CREATE,    50, CG_PCL,  RS_30,     150, GT_OK},
    };
    static VT_SEQUENCE lion2_create_after_decrease_sequence[] =
    {
        {VT_CREATE,    50, CG_PCL,  RS_30,     100, GT_OK},
    };
    static VT_SEQUENCE bc2_create_after_decrease_sequence[] =
    {
        {VT_CREATE,    50, CG_TTI,  RS_10,    3000, GT_OK},
    };
    static VT_SEQUENCE bobk_create_after_decrease_sequence[] =
    {
        {VT_CREATE,    50, CG_TTI,  RS_10,    1500, GT_OK},
    };

    static VT_SEQUENCE aldrin_create_after_decrease_sequence[] =
    {
        {VT_CREATE,    50, CG_TTI,  RS_10,    750, GT_OK},
    };

    cpssOsMemSet(&vTcamInfo, 0, sizeof(vTcamInfo));
    cpssOsMemSet(&vTcamInfoGet, 0, sizeof(vTcamInfoGet));
    cpssOsMemSet(&actionType, 0, sizeof(actionType));
    cpssOsMemSet(&actionData, 0, sizeof(actionData));
    cpssOsMemSet(&pclActionGet, 0, sizeof(pclActionGet));

    cpssOsMemSet(&tcamRuleDataGet, 0, sizeof(tcamRuleDataGet));

    vTcamMngId = 7;

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* this feature is on eArch devices, xCat3 and Lion2 */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    notAppFamilyBmp &= (~UTF_XCAT3_E) & (~UTF_LION2_E);

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* 1. Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
            devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }
        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;

        /* Create vTCAM manager */
        st = cpssDxChVirtualTcamManagerCreate(vTcamMngId);
        if (st == GT_ALREADY_EXIST)
        {
            st = GT_OK;
        }
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerCreate failed\n");


        st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId, devListArr, numOfDevs);
        if (st != GT_OK)
        {
            cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
            st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        }
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListAdd failed\n");

        switch (utfFamilyBit)
        {
            case UTF_XCAT3_E:
                sequencePtr = xcat3_sequence;
                sequenceSize = sizeof(xcat3_sequence) / sizeof(xcat3_sequence[0]);
                break;

            case UTF_LION2_E:
                sequencePtr = lion2_sequence;
                sequenceSize = sizeof(lion2_sequence) / sizeof(lion2_sequence[0]);
                break;

            case UTF_ALDRIN2_E:
            case UTF_FALCON_E:
            case UTF_CAELUM_E:
                sequencePtr = bobk_create_sequence;
                sequenceSize = sizeof(bobk_create_sequence) / sizeof(bobk_create_sequence[0]);
                break;

            case UTF_ALDRIN_E:
            case UTF_AC3X_E:
                sequencePtr = aldrin_create_sequence;
                sequenceSize = sizeof(aldrin_create_sequence) / sizeof(aldrin_create_sequence[0]);
                break;

            case UTF_BOBCAT2_E:
            case UTF_BOBCAT3_E:
            default:
                sequencePtr = bc2_create_sequence;
                sequenceSize = sizeof(bc2_create_sequence) / sizeof(bc2_create_sequence[0]);
                break;
        }

        vTcamInfo.hitNumber     = 0;
        vTcamInfo.autoResize    = GT_FALSE;
        vTcamInfo.ruleAdditionMethod =
            CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E;

        /* set new PCL rule */
        cpssOsMemSet(&pclMask, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
        cpssOsMemSet(&pclPattern, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
        cpssOsMemSet(&pclAction, 0, sizeof(CPSS_DXCH_PCL_ACTION_STC));
        /* set mask, pattern and action data */
        setRuleIngrUdbOnly(&pclMask, &pclPattern, &pclAction);

        /* set new TTI rule */
        cpssOsMemSet(&ttiMask, 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
        cpssOsMemSet(&ttiPattern, 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
        cpssOsMemSet(&ttiAction, 0, sizeof(CPSS_DXCH_TTI_ACTION_UNT));
        /* set mask, pattern and action data */
        setTtiRuleIpv4(&ttiMask, &ttiPattern, &ttiAction);

        cpssOsMemSet(&ruleAttributes, 0, sizeof(ruleAttributes));
        cpssOsMemSet(&ruleData, 0, sizeof(ruleData));

        ruleData.valid = GT_TRUE;

        /* 1.   Create many vtcams in tcam see table 1 ' part 1 + part  2 (with the remove of vtcams: 6,8,10,13,15) */
        for (idx = 0; idx < sequenceSize; idx++)
        {
            if ((sequencePtr[idx].operation == VT_CREATE) ||
                (sequencePtr[idx].operation == VT_COMPLETE) ||
                (sequencePtr[idx].operation == VT_REMOVE))
            {
                if ((sequencePtr[idx].operation == VT_REMOVE))
                {
                    /*  Delete all rules before removing vTCAM */
                    vTcamDeleteConsecutiveRules(vTcamMngId,
                                            sequencePtr[idx].vTcamId,
                                            CPSS_DXCH_VIRTUAL_TCAM_START_RULE_ID_CNS,
                                            DELETE_ALL_CNS);
                }

                /* Apply the sequenced operation */
                st = vtcamSequenceEntryProcess(sequencePtr, idx, vTcamMngId, &vTcamInfo);
                /* no free memory for new vTCAM */
                if (st == GT_FAIL)
                {
                    PRV_UTF_LOG2_MAC(
                        "vtcamSequenceEntryProcess calls loop broken due to FAIL idx %d vTcamId %d\n",
                        idx, sequencePtr[idx].vTcamId);
                    break;
                }

                if (sequencePtr[idx].rc == GT_OK)
                {

                    if ((sequencePtr[idx].operation == VT_CREATE) ||
                        (sequencePtr[idx].operation == VT_COMPLETE))
                    {
                        /* new vTCAM created */
                        vTcamAdded = idx + 1;

                        /*  add rules to vTCAM */
                        if (sequencePtr[idx].clientGroup == CG_PCL)
                        {
                            /* set new PCL rule */
                            cpssOsMemSet(&pclMask, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
                            cpssOsMemSet(&pclPattern, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
                            cpssOsMemSet(&pclAction, 0, sizeof(CPSS_DXCH_PCL_ACTION_STC));

                            switch (sequencePtr[idx].ruleSize)
                            {
                                case RS_30:

                                    setRuleStdNotIp(&pclMask, &pclPattern, &pclAction);
                                    break;

                                case RS_60:

                                    setRuleExtNotIpv6(&pclMask, &pclPattern, &pclAction);
                                    break;

                                case RS_10:
                                case RS_20:
                                case RS_40:
                                case RS_50:
                                default:

                                    setRuleIngrUdbOnly(&pclMask, &pclPattern, &pclAction);
                                    break;

                            }
                        }
                        else
                        {
                            /* set new TTI rule */
                            cpssOsMemSet(&ttiMask, 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
                            cpssOsMemSet(&ttiPattern, 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
                            cpssOsMemSet(&ttiAction, 0, sizeof(CPSS_DXCH_TTI_ACTION_UNT));

                            switch (sequencePtr[idx].ruleSize)
                            {
                                case RS_10:
                                case RS_20:

                                    setTtiRuleUdb(&ttiMask, &ttiPattern, &ttiAction);
                                    break;

                                case RS_30:
                                default:

                                    setTtiRuleIpv4(&ttiMask, &ttiPattern, &ttiAction);
                                    break;
                            }
                        }
                        for (ruleIdx = 0; ruleIdx < vTcamInfo.guaranteedNumOfRules; ruleIdx++)
                        {
                            ruleId = ruleIdx;
                            st = vtcamSequenceRuleUpdate(vTcamMngId, sequencePtr, idx, ruleId,
                                                         &pclAction, &pclMask, &pclPattern,
                                                         &ttiAction, &ttiMask, &ttiPattern);
                            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                                         "vtcamSequenceRuleUpdate on vTcam sequence index(%d), ruleId:%d failed", idx, ruleId);
                        }
                    }
                }
            }
        }

        if (vTcamAdded)
        {
            /* 2.   Start to resize to increase number of guaranteed rules (cpssDxChVirtualTcamResize) */

            switch (utfFamilyBit)
            {
                case UTF_XCAT3_E:
                    incDecSequencePtr = xcat3_increase_sequence;
                    sequenceSize = sizeof(xcat3_increase_sequence) / sizeof(xcat3_increase_sequence[0]);
                    break;

                case UTF_LION2_E:
                    incDecSequencePtr = lion2_increase_sequence;
                    sequenceSize = sizeof(lion2_increase_sequence) / sizeof(lion2_increase_sequence[0]);
                    break;

                case UTF_ALDRIN2_E:
                case UTF_FALCON_E:
                case UTF_CAELUM_E:
                    incDecSequencePtr = bobk_increase_sequence;
                    sequenceSize = sizeof(bobk_increase_sequence) / sizeof(bobk_increase_sequence[0]);
                    break;

                case UTF_ALDRIN_E:
                case UTF_AC3X_E:
                    incDecSequencePtr = aldrin_increase_sequence;
                    sequenceSize = sizeof(aldrin_increase_sequence) / sizeof(aldrin_increase_sequence[0]);
                    break;

                case UTF_BOBCAT2_E:
                case UTF_BOBCAT3_E:
                default:
                    incDecSequencePtr = bc2_increase_sequence;
                    sequenceSize = sizeof(bc2_increase_sequence) / sizeof(bc2_increase_sequence[0]);
                    break;
            }

            for (idx = 0; idx < sequenceSize; idx++)
            {
                if ((incDecSequencePtr[idx].operation == VT_INCREASE) ||
                    (incDecSequencePtr[idx].operation == VT_DECREASE))
                {
                    st = vtcamSequenceIncreaseDecreaseProcess(incDecSequencePtr, idx, vTcamMngId);
                    /* no free memory for new vTCAM */
                    if (st == GT_FAIL)
                    {
                        PRV_UTF_LOG2_MAC(
                            "vtcamSequenceIncreaseDecreaseProcess calls loop broken due to FAIL idx %d vTcamId %d\n",
                            idx, incDecSequencePtr[idx].vTcamId);
                        break;
                    }

                    /*  add rules to vTCAM */
                    if (sequencePtr[incDecSequencePtr[idx].vTcamIdIdx].clientGroup == CG_PCL)
                    {
                        /* set new PCL rule */
                        cpssOsMemSet(&pclMask, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
                        cpssOsMemSet(&pclPattern, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
                        cpssOsMemSet(&pclAction, 0, sizeof(CPSS_DXCH_PCL_ACTION_STC));

                        switch (sequencePtr[incDecSequencePtr[idx].vTcamIdIdx].ruleSize)
                        {
                            case RS_30:

                                setRuleStdNotIp(&pclMask, &pclPattern, &pclAction);
                                break;

                            case RS_60:

                                setRuleExtNotIpv6(&pclMask, &pclPattern, &pclAction);
                                break;

                            case RS_10:
                            case RS_20:
                            case RS_40:
                            case RS_50:
                            default:

                                setRuleIngrUdbOnly(&pclMask, &pclPattern, &pclAction);
                                break;

                        }
                    }
                    for (ruleIdx = 0; ruleIdx < incDecSequencePtr[idx].rulesAmount; ruleIdx++)
                    {
                        switch (incDecSequencePtr[idx].rulesPlace)
                        {
                            case CPSS_DXCH_VIRTUAL_TCAM_BEFORE_START_CNS:
                                ruleId = ruleIdx;
                                break;
                            case CPSS_DXCH_VIRTUAL_TCAM_APPEND_CNS:
                                ruleId = sequencePtr[incDecSequencePtr[idx].vTcamIdIdx].rulesAmount + ruleIdx;
                                break;
                            default:
                                ruleId = incDecSequencePtr[idx].rulesPlace + ruleIdx;
                                break;
                        }
                        st = vtcamSequenceRuleUpdate(vTcamMngId, sequencePtr, incDecSequencePtr[idx].vTcamIdIdx, ruleId,
                                                     &pclAction, &pclMask, &pclPattern,
                                                     &ttiAction, &ttiMask, &ttiPattern);
                        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                                     "vtcamSequenceRuleUpdate on vTcam sequence index(%d), ruleId:%d failed", idx, ruleId);
                    }
                }
            }

            /* 3.   Check content of logical index in the tcam (cpssDxChVirtualTcamRuleRead) */

            switch (utfFamilyBit)
            {
                case UTF_XCAT3_E:
                    srcSequencePtr = xcat3_srcTcamRule;
                    dstSequencePtr = xcat3_dstTcamRule;
                    sequenceSize = sizeof(xcat3_srcTcamRule) / sizeof(xcat3_srcTcamRule[0]);
                    break;

                case UTF_LION2_E:
                    srcSequencePtr = lion2_srcTcamRule;
                    dstSequencePtr = lion2_dstTcamRule;
                    sequenceSize = sizeof(lion2_srcTcamRule) / sizeof(lion2_srcTcamRule[0]);
                    break;

                case UTF_ALDRIN2_E:
                case UTF_FALCON_E:
                case UTF_CAELUM_E:
                    srcSequencePtr = bobk_srcTcamRule;
                    dstSequencePtr = bobk_dstTcamRule;
                    sequenceSize = sizeof(bobk_srcTcamRule) / sizeof(bobk_srcTcamRule[0]);
                    break;

                case UTF_ALDRIN_E:
                case UTF_AC3X_E:
                    srcSequencePtr = aldrin_srcTcamRule;
                    dstSequencePtr = aldrin_dstTcamRule;
                    sequenceSize = sizeof(aldrin_srcTcamRule) / sizeof(aldrin_srcTcamRule[0]);
                    break;

                case UTF_BOBCAT2_E:
                case UTF_BOBCAT3_E:
                default:
                    srcSequencePtr = bc2_srcTcamRule;
                    dstSequencePtr = bc2_dstTcamRule;
                    sequenceSize = sizeof(bc2_srcTcamRule) / sizeof(bc2_srcTcamRule[0]);
                    break;
            }

            for (idx = 0; idx < sequenceSize; idx++)
            {
                cpssOsMemSet(&pclMask, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
                cpssOsMemSet(&pclPattern, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
                cpssOsMemSet(&pclActionGet, 0, sizeof(CPSS_DXCH_PCL_ACTION_STC));

                cpssOsMemSet(&ttiMask, 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
                cpssOsMemSet(&ttiPattern, 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
                cpssOsMemSet(&ttiActionGet, 0, sizeof(CPSS_DXCH_TTI_ACTION_UNT));

                if ((utfFamilyBit == UTF_XCAT3_E) ||
                    (utfFamilyBit == UTF_LION2_E))
                {
                    switch (dstSequencePtr[idx].vTcamSeqId)
                    {
                    case 5:
                    case 7:
                        tcamRuleType.ruleType = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_TTI_E;
                        tcamRuleType.rule.tti.ruleFormat = CPSS_DXCH_TTI_RULE_IPV4_E;
                        tcamRuleDataGet.rule.tti.actionPtr      = &ttiActionGet;
                        tcamRuleDataGet.rule.tti.maskPtr        = &ttiMask;
                        tcamRuleDataGet.rule.tti.patternPtr     = &ttiPattern;
                        break;

                    case 8:
                    case 10:
                    case 12:
                    case 15:
                        tcamRuleType.ruleType = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;
                        tcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E;
                        tcamRuleDataGet.rule.pcl.actionPtr      = &pclActionGet;
                        tcamRuleDataGet.rule.pcl.maskPtr        = &pclMask;
                        tcamRuleDataGet.rule.pcl.patternPtr     = &pclPattern;
                        break;

                    case 9:
                    case 11:
                    case 13:
                    case 14:
                    case 16:
                    default:

                        tcamRuleType.ruleType = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;
                        tcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
                        tcamRuleDataGet.rule.pcl.actionPtr      = &pclActionGet;
                        tcamRuleDataGet.rule.pcl.maskPtr        = &pclMask;
                        tcamRuleDataGet.rule.pcl.patternPtr     = &pclPattern;
                        break;
                    }
                }
                else
                {
                    switch (dstSequencePtr[idx].vTcamSeqId)
                    {
                    case 5:
                    case 7:

                        tcamRuleType.ruleType = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_TTI_E;
                        tcamRuleType.rule.tti.ruleFormat = (dstSequencePtr[idx].vTcamSeqId == 5) ?
                                                            CPSS_DXCH_TTI_RULE_UDB_20_E :
                                                            CPSS_DXCH_TTI_RULE_UDB_10_E;
                        tcamRuleDataGet.rule.tti.actionPtr      = &ttiActionGet;
                        tcamRuleDataGet.rule.tti.maskPtr        = &ttiMask;
                        tcamRuleDataGet.rule.tti.patternPtr     = &ttiPattern;

                        break;

                    case 9:
                    case 11:
                    case 12:
                    case 14:
                    case 16:
                    default:

                        tcamRuleType.ruleType = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;
                        tcamRuleType.rule.pcl.ruleFormat = (dstSequencePtr[idx].vTcamSeqId == 9) ?
                                                                CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_40_E :
                                                            (dstSequencePtr[idx].vTcamSeqId == 12) ?
                                                                CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E :
                                                            (dstSequencePtr[idx].vTcamSeqId == 14) ?
                                                                CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_50_E :
                                                                CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E;
                        tcamRuleDataGet.rule.pcl.actionPtr      = &pclActionGet;
                        tcamRuleDataGet.rule.pcl.maskPtr        = &pclMask;
                        tcamRuleDataGet.rule.pcl.patternPtr     = &pclPattern;

                        break;
                    }
                }

                st = cpssDxChVirtualTcamRuleRead(
                    vTcamMngId, dstSequencePtr[idx].vTcamSeqId, dstSequencePtr[idx].ruleId,
                    &tcamRuleType, &tcamRuleDataGet);
                if (st != GT_OK)
                {
                    cpssOsPrintf("\n vtcam %d\n", dstSequencePtr[idx].vTcamSeqId);
                    prvCpssDxChVirtualTcamDbVTcamDumpByField(
                        vTcamMngId, dstSequencePtr[idx].vTcamSeqId, "usedRulesBitmapArr");
                }

                UTF_VERIFY_EQUAL3_STRING_MAC(
                    GT_OK, st,
                    "cpssDxChVirtualTcamRuleRead failed, vtcam(%d) Idx(%d) RuleId: %d\n",
                    dstSequencePtr[idx].vTcamSeqId, idx, dstSequencePtr[idx].ruleId);

                if (st == GT_OK)
                {

                    /* get the unique value that each rule got */
                    if (tcamRuleType.ruleType == CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E)
                    {
                        UNIQUE_PCL_RULE_GET_MAC(&tcamRuleType,tcamRuleDataGet.rule.pcl.patternPtr, tmpValue);
                    }
                    else
                    {
                        UNIQUE_TTI_RULE_GET_MAC(&tcamRuleType,tcamRuleDataGet.rule.tti.patternPtr, tmpValue);
                    }

                    if (srcSequencePtr[idx].ruleId != tmpValue)
                    {
                        cpssOsPrintf("\n vtcam %d\n", dstSequencePtr[idx].vTcamSeqId);
                        prvCpssDxChVirtualTcamDbVTcamDumpByField(
                            vTcamMngId, dstSequencePtr[idx].vTcamSeqId, "usedRulesBitmapArr");
                    }
                    UTF_VERIFY_EQUAL0_STRING_MAC(srcSequencePtr[idx].ruleId, tmpValue, "Wrong rule content");

                }
            }

            /* 4.   Check that we have more rules (write to last index) */

            switch (utfFamilyBit)
            {
                case UTF_XCAT3_E:
                    sequencePtr = xcat3_check_increase_sequence;
                    sequenceSize = sizeof(xcat3_check_increase_sequence) / sizeof(xcat3_check_increase_sequence[0]);
                    break;

                case UTF_LION2_E:
                    sequencePtr = lion2_check_increase_sequence;
                    sequenceSize = sizeof(lion2_check_increase_sequence) / sizeof(lion2_check_increase_sequence[0]);
                    break;

                case UTF_ALDRIN2_E:
                case UTF_FALCON_E:
                case UTF_CAELUM_E:
                    sequencePtr = bobk_check_increase_sequence;
                    sequenceSize = sizeof(bobk_check_increase_sequence) / sizeof(bobk_check_increase_sequence[0]);
                    break;

                case UTF_ALDRIN_E:
                case UTF_AC3X_E:
                    sequencePtr = aldrin_check_increase_sequence;
                    sequenceSize = sizeof(aldrin_check_increase_sequence) / sizeof(aldrin_check_increase_sequence[0]);
                    break;

                case UTF_BOBCAT2_E:
                case UTF_BOBCAT3_E:
                default:
                    sequencePtr = bc2_check_increase_sequence;
                    sequenceSize = sizeof(bc2_check_increase_sequence) / sizeof(bc2_check_increase_sequence[0]);
                    break;
            }

            cpssOsMemSet(&ttiMask, 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
            cpssOsMemSet(&ttiPattern, 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
            cpssOsMemSet(&pclMask, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
            cpssOsMemSet(&pclPattern, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));

            for (idx = 0; idx < sequenceSize; idx++)
            {
                ruleId = sequencePtr[idx].rulesAmount;
                if (sequencePtr[idx].clientGroup == CG_PCL)
                {
                    /* set new PCL rule */
                    cpssOsMemSet(&pclMask, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
                    cpssOsMemSet(&pclPattern, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
                    cpssOsMemSet(&pclAction, 0, sizeof(CPSS_DXCH_PCL_ACTION_STC));

                    switch (sequencePtr[idx].ruleSize)
                    {
                        case RS_30:

                            setRuleStdNotIp(&pclMask, &pclPattern, &pclAction);
                            break;

                        case RS_60:

                            setRuleExtNotIpv6(&pclMask, &pclPattern, &pclAction);
                            break;

                        case RS_10:
                        case RS_20:
                        case RS_40:
                        case RS_50:
                        default:

                            setRuleIngrUdbOnly(&pclMask, &pclPattern, &pclAction);
                            break;

                    }
                }
                st = vtcamSequenceRuleUpdate(vTcamMngId, sequencePtr, idx, ruleId,
                                             &pclAction, &pclMask, &pclPattern,
                                             &ttiAction, &ttiMask, &ttiPattern);
                UTF_VERIFY_EQUAL3_STRING_MAC(
                    sequencePtr[idx].rc, st,
                    "vtcamSequenceRuleUpdate on vTcam(%d) sequence index(%d), ruleId:%d failed",
                    sequencePtr[idx].vTcamId, idx, ruleId);
            }

            /* 5.   Start to resize to decrease number of guaranteed rules (cpssDxChVirtualTcamResize) */

            switch (utfFamilyBit)
            {
                case UTF_XCAT3_E:
                    incDecSequencePtr = xcat3_decrease_sequence;
                    sequenceSize = sizeof(xcat3_decrease_sequence) / sizeof(xcat3_decrease_sequence[0]);
                    break;

                case UTF_LION2_E:
                    incDecSequencePtr = lion2_decrease_sequence;
                    sequenceSize = sizeof(lion2_decrease_sequence) / sizeof(lion2_decrease_sequence[0]);
                    break;

                case UTF_ALDRIN2_E:
                case UTF_FALCON_E:
                case UTF_CAELUM_E:
                    incDecSequencePtr = bobk_decrease_sequence;
                    sequenceSize = sizeof(bobk_decrease_sequence) / sizeof(bobk_decrease_sequence[0]);
                    break;

                case UTF_ALDRIN_E:
                case UTF_AC3X_E:
                    incDecSequencePtr = aldrin_decrease_sequence;
                    sequenceSize = sizeof(aldrin_decrease_sequence) / sizeof(aldrin_decrease_sequence[0]);
                    break;

                case UTF_BOBCAT2_E:
                case UTF_BOBCAT3_E:
                default:
                    incDecSequencePtr = bc2_decrease_sequence;
                    sequenceSize = sizeof(bc2_decrease_sequence) / sizeof(bc2_decrease_sequence[0]);
                    break;
            }

            for (idx = 0; idx < sequenceSize; idx++)
            {
                if ((incDecSequencePtr[idx].operation == VT_INCREASE) ||
                    (incDecSequencePtr[idx].operation == VT_DECREASE))
                {
                    st = vtcamSequenceIncreaseDecreaseProcess(incDecSequencePtr, idx, vTcamMngId);
                    /* no free memory for new vTCAM */
                    if (st == GT_FAIL)
                    {
                        PRV_UTF_LOG2_MAC(
                            "vtcamSequenceIncreaseDecreaseProcess calls loop broken due to FAIL idx %d vTcamId %d\n",
                            idx, incDecSequencePtr[idx].vTcamId);
                        break;
                    }

                }
            }

            /* 6.   Check that we have less rules (write to last index and to next 'not valid') */

            switch (utfFamilyBit)
            {
                case UTF_XCAT3_E:
                    sequencePtr = xcat3_check_decrease_sequence;
                    sequenceSize = sizeof(xcat3_check_decrease_sequence) / sizeof(xcat3_check_decrease_sequence[0]);
                    break;

                case UTF_LION2_E:
                    sequencePtr = lion2_check_decrease_sequence;
                    sequenceSize = sizeof(lion2_check_decrease_sequence) / sizeof(lion2_check_decrease_sequence[0]);
                    break;

                case UTF_ALDRIN2_E:
                case UTF_FALCON_E:
                case UTF_CAELUM_E:
                    sequencePtr = bobk_check_decrease_sequence;
                    sequenceSize = sizeof(bobk_check_decrease_sequence) / sizeof(bobk_check_decrease_sequence[0]);
                    break;

                case UTF_ALDRIN_E:
                case UTF_AC3X_E:
                    sequencePtr = aldrin_check_decrease_sequence;
                    sequenceSize = sizeof(aldrin_check_decrease_sequence) / sizeof(aldrin_check_decrease_sequence[0]);
                    break;

                case UTF_BOBCAT2_E:
                case UTF_BOBCAT3_E:
                default:
                    sequencePtr = bc2_check_decrease_sequence;
                    sequenceSize = sizeof(bc2_check_decrease_sequence) / sizeof(bc2_check_decrease_sequence[0]);
                    break;
            }

            for (idx = 0; idx < sequenceSize; idx++)
            {
                ruleId = sequencePtr[idx].rulesAmount;
                if (sequencePtr[idx].clientGroup == CG_PCL)
                {
                    /* set new PCL rule */
                    cpssOsMemSet(&pclMask, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
                    cpssOsMemSet(&pclPattern, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
                    cpssOsMemSet(&pclAction, 0, sizeof(CPSS_DXCH_PCL_ACTION_STC));

                    switch (sequencePtr[idx].ruleSize)
                    {
                        case RS_30:

                            setRuleStdNotIp(&pclMask, &pclPattern, &pclAction);
                            break;

                        case RS_60:

                            setRuleExtNotIpv6(&pclMask, &pclPattern, &pclAction);
                            break;

                        case RS_10:
                        case RS_20:
                        case RS_40:
                        case RS_50:
                        default:

                            setRuleIngrUdbOnly(&pclMask, &pclPattern, &pclAction);
                            break;

                    }
                }
                sequencePtr[idx].rc = GT_OK;
                st = vtcamSequenceRuleUpdate(vTcamMngId, sequencePtr, idx, ruleId,
                                             &pclAction, &pclMask, &pclPattern,
                                             &ttiAction, &ttiMask, &ttiPattern);
                UTF_VERIFY_EQUAL3_STRING_MAC(
                    sequencePtr[idx].rc, st,
                    "vtcamSequenceRuleUpdate on vTcam(%d) sequence index(%d), ruleId:%d failed",
                    sequencePtr[idx].vTcamId, idx, ruleId);
            }

            /* 7.   Check that resize decrease allow to free area of rules, that can be allocated by other vtcam */

            switch (utfFamilyBit)
            {
                case UTF_XCAT3_E:
                    sequencePtr = xcat3_create_after_decrease_sequence;
                    sequenceSize = sizeof(xcat3_create_after_decrease_sequence) / sizeof(xcat3_create_after_decrease_sequence[0]);
                    break;

                case UTF_LION2_E:
                    sequencePtr = lion2_create_after_decrease_sequence;
                    sequenceSize = sizeof(lion2_create_after_decrease_sequence) / sizeof(lion2_create_after_decrease_sequence[0]);
                    break;

                case UTF_ALDRIN2_E:
                case UTF_FALCON_E:
                case UTF_CAELUM_E:
                    sequencePtr = bobk_create_after_decrease_sequence;
                    sequenceSize = sizeof(bobk_create_after_decrease_sequence) / sizeof(bobk_create_after_decrease_sequence[0]);
                    break;

                case UTF_ALDRIN_E:
                case UTF_AC3X_E:
                    sequencePtr = aldrin_create_after_decrease_sequence;
                    sequenceSize = sizeof(aldrin_create_after_decrease_sequence) / sizeof(aldrin_create_after_decrease_sequence[0]);
                    break;

                case UTF_BOBCAT2_E:
                case UTF_BOBCAT3_E:
                default:
                    sequencePtr = bc2_create_after_decrease_sequence;
                    sequenceSize = sizeof(bc2_create_after_decrease_sequence) / sizeof(bc2_create_after_decrease_sequence[0]);
                    break;
            }

            for (idx = 0; idx < sequenceSize; idx++)
            {
                if ((sequencePtr[idx].operation == VT_CREATE))
                {
                    st = vtcamSequenceEntryProcess(sequencePtr, idx, vTcamMngId, &vTcamInfo);
                    /* no free memory for new vTCAM */
                    if (st == GT_FAIL)
                    {
                        PRV_UTF_LOG2_MAC(
                            "vtcamSequenceEntryProcess calls loop broken due to FAIL idx %d vTcamId %d\n",
                            idx, sequencePtr[idx].vTcamId);
                        break;
                    }

                    if (sequencePtr[idx].rc == GT_OK)
                    {

                        /*  add rules to vTCAM */
                        if (sequencePtr[idx].clientGroup == CG_PCL)
                        {
                            /* set new PCL rule */
                            cpssOsMemSet(&pclMask, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
                            cpssOsMemSet(&pclPattern, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
                            cpssOsMemSet(&pclAction, 0, sizeof(CPSS_DXCH_PCL_ACTION_STC));

                            switch (sequencePtr[idx].ruleSize)
                            {
                                case RS_30:

                                    setRuleStdNotIp(&pclMask, &pclPattern, &pclAction);
                                    break;

                                case RS_60:

                                    setRuleExtNotIpv6(&pclMask, &pclPattern, &pclAction);
                                    break;

                                case RS_10:
                                case RS_20:
                                case RS_40:
                                case RS_50:
                                default:

                                    setRuleIngrUdbOnly(&pclMask, &pclPattern, &pclAction);
                                    break;

                            }
                        }
                        for (ruleIdx = 0; ruleIdx < vTcamInfo.guaranteedNumOfRules; ruleIdx++)
                        {
                            ruleId = ruleIdx;
                            st = vtcamSequenceRuleUpdate(vTcamMngId, sequencePtr, idx, ruleId,
                                                         &pclAction, &pclMask, &pclPattern,
                                                         &ttiAction, &ttiMask, &ttiPattern);
                            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                                         "vtcamSequenceRuleUpdate on vTcam sequence index(%d), ruleId:%d failed", idx, ruleId);
                        }
                    }
                }
            }

        }

        /*  Delete all rules before removing vTCAM */
        if ((utfFamilyBit != UTF_XCAT3_E) &&
            (utfFamilyBit != UTF_LION2_E))
        {
            vTcamDeleteConsecutiveRules(vTcamMngId,
                                    5,
                                    CPSS_DXCH_VIRTUAL_TCAM_START_RULE_ID_CNS,
                                    DELETE_ALL_CNS);
            vTcamDeleteConsecutiveRules(vTcamMngId,
                                    7,
                                    CPSS_DXCH_VIRTUAL_TCAM_START_RULE_ID_CNS,
                                    DELETE_ALL_CNS);
        }
        vTcamDeleteConsecutiveRules(vTcamMngId,
                                9,
                                CPSS_DXCH_VIRTUAL_TCAM_START_RULE_ID_CNS,
                                DELETE_ALL_CNS);
        vTcamDeleteConsecutiveRules(vTcamMngId,
                                11,
                                CPSS_DXCH_VIRTUAL_TCAM_START_RULE_ID_CNS,
                                DELETE_ALL_CNS);
        vTcamDeleteConsecutiveRules(vTcamMngId,
                                12,
                                CPSS_DXCH_VIRTUAL_TCAM_START_RULE_ID_CNS,
                                DELETE_ALL_CNS);
        vTcamDeleteConsecutiveRules(vTcamMngId,
                                14,
                                CPSS_DXCH_VIRTUAL_TCAM_START_RULE_ID_CNS,
                                DELETE_ALL_CNS);
        vTcamDeleteConsecutiveRules(vTcamMngId,
                                16,
                                CPSS_DXCH_VIRTUAL_TCAM_START_RULE_ID_CNS,
                                DELETE_ALL_CNS);
        vTcamDeleteConsecutiveRules(vTcamMngId,
                                50,
                                CPSS_DXCH_VIRTUAL_TCAM_START_RULE_ID_CNS,
                                DELETE_ALL_CNS);

        /*  Removing vTCAM manager */
        st = cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListRemove failed for vTcamMngId = %d\n",
                                     vTcamMngId);
        st = cpssDxChVirtualTcamManagerDelete(vTcamMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDelete failed for vTcamMngId = %d\n",
                                     vTcamMngId);
    }

    if (isTestSkipped)
    {
        SKIP_TEST_MAC;
    }
}

/* Test Case #9.    Test resize priority mode */
UTF_TEST_CASE_MAC(cpssDxChVirtualTcam_resizeRulesPriorityMode)
{
    GT_STATUS                                  st   = GT_OK;
    GT_U8                                      dev;
    GT_U32                                     vTcamMngId;
    GT_U32                                     vTcamId;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ID             ruleId = 0;
    GT_U32                                     idx = 0;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC ruleAttributes;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC       ruleData;
    CPSS_DXCH_PCL_ACTION_STC                   pclAction;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              pclMask;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              pclPattern;
    GT_U32                                     numOfDevs = 0;
    GT_U32                                     notAppFamilyBmp = 0;
    GT_U32                                     utfFamilyBit = 0;
    GT_BOOL                                    isTestSkipped = GT_TRUE;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC            vTcamInfo;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC       tcamRuleType;
    CPSS_DXCH_VIRTUAL_TCAM_USAGE_STC           vTcamUsage;

    cpssOsMemSet(&vTcamInfo, 0, sizeof(vTcamInfo));

    vTcamMngId = 7;
    vTcamId = 1;

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* this feature is on eArch devices, xCat3 and Lion2 */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    notAppFamilyBmp &= (~UTF_XCAT3_E) & (~UTF_LION2_E);

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* 1. Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
            devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }
        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;

        /* Create vTCAM manager */
        st = cpssDxChVirtualTcamManagerCreate(vTcamMngId);
        if (st == GT_ALREADY_EXIST)
        {
            st = GT_OK;
        }
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerCreate failed\n");


        st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId, devListArr, numOfDevs);
        if (st != GT_OK)
        {
            cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
            st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        }
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListAdd failed\n");

        /*
         1.  Create tcam manager with vtcam (100 guaranteed) with 100 rules
         */
        vTcamInfo.clientGroup = CG_PCL;
        vTcamInfo.hitNumber = 0;
        vTcamInfo.ruleSize = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E;
        vTcamInfo.autoResize = GT_FALSE;
        vTcamInfo.guaranteedNumOfRules = 100;
        vTcamInfo.ruleAdditionMethod = CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E;

        st = cpssDxChVirtualTcamCreate(vTcamMngId, vTcamId, &vTcamInfo);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                                    "ut_cpssDxChVirtualTcamCreate failed for vTcamMngId/vTcamId = %d/%d, st = %d \n",
                                    vTcamMngId, 1, st);
        /*
         2.  Priority 1..priority 1001 with steps of 100 (10 priorities)
                a.  Add to each of them 10 rules
         */
        /* Build rule */
        tcamRuleType.ruleType            = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;
        tcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
        ruleData.valid                   = GT_TRUE;
        /* set new rule */
        cpssOsMemSet(&pclMask, '\0', sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
        cpssOsMemSet(&pclPattern, '\0', sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
        cpssOsMemSet(&pclAction, '\0', sizeof(CPSS_DXCH_PCL_ACTION_STC));
        /* Set Mask */
        setRuleStdNotIp(&pclMask, &pclPattern, &pclAction);

        ruleData.rule.pcl.actionPtr     = &pclAction;
        ruleData.rule.pcl.maskPtr       = &pclMask;
        ruleData.rule.pcl.patternPtr    = &pclPattern;

        for (ruleAttributes.priority = 1; ruleAttributes.priority < 1001; ruleAttributes.priority += 100)
        {
            for (idx = 0; idx < 10; idx++)
            {
                /* Add rule */
                ruleId = (10 * ruleAttributes.priority) + idx;
                UNIQUE_PCL_RULE_SET_MAC(&tcamRuleType,&pclPattern,ruleId);
                st = cpssDxChVirtualTcamRuleWrite(vTcamMngId, vTcamId, ruleId, &ruleAttributes,
                                                    &tcamRuleType, &ruleData);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamRuleWrite failed\n");
            }
        }

        /*
         3.  Add new rule to priority 501
                a.  Check that 'GT_FULL'
         */
        {
            /* Try to add rule */
            ruleAttributes.priority = 501;
            ruleId = (10 * ruleAttributes.priority) + idx;
            UNIQUE_PCL_RULE_SET_MAC(&tcamRuleType,&pclPattern,ruleId);
            st = cpssDxChVirtualTcamRuleWrite(vTcamMngId, vTcamId, ruleId, &ruleAttributes,
                                                &tcamRuleType, &ruleData);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_FULL, st, "cpssDxChVirtualTcamRuleWrite wrong failure code\n");
        }

        /*
         4.  Resize to add 10 rules after priority 501 (cpssDxChVirtualTcamResize)
             Amount of added rules is 12 due to granularity.
         */
        {
            st = cpssDxChVirtualTcamResize(vTcamMngId,vTcamId, 501, GT_TRUE, 10);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamResize (increase) failed\n");
        }

        /*
         5.  Check that you can add total of 10 new rules of any priority
         */

        for (ruleAttributes.priority = 1; ruleAttributes.priority < 1001; ruleAttributes.priority += 100)
        {
            /* Add rule */
            ruleId = (10 * ruleAttributes.priority) + idx;
            UNIQUE_PCL_RULE_SET_MAC(&tcamRuleType,&pclPattern,ruleId);
            st = cpssDxChVirtualTcamRuleWrite(vTcamMngId, vTcamId, ruleId, &ruleAttributes,
                                                &tcamRuleType, &ruleData);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamRuleWrite failed\n");
        }

        /*
         6.  Void
         7.  Delete 50 rules from priority : 101,201,301,401,501 (need to empty 101 before going to 201..)
         */

        for (ruleAttributes.priority = 101; ruleAttributes.priority <= 501; ruleAttributes.priority += 100)
        {
            for (idx = 0; idx < 10; idx++)
            {
                /* Delete rule */
                ruleId = (10 * ruleAttributes.priority) + idx;
                st = cpssDxChVirtualTcamRuleDelete(vTcamMngId, vTcamId, ruleId);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamRuleDelete failed\n");
            }
        }

        /*
         8.  Resize to remove 50 rules from the start of tcam
             Amount of deleted ruleplaces is 48 - granulated to 4-aligment.
         9.  Void
         */

        {
            st = cpssDxChVirtualTcamResize(vTcamMngId,vTcamId, 0, GT_FALSE, 50);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamResize (decrese) failed\n");

            /* occuipate all available ruleplaces */
            st = cpssDxChVirtualTcamUsageGet(
                vTcamMngId, vTcamId, &vTcamUsage);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamUsageGet failed\n");

            for (idx = 0; (idx < vTcamUsage.rulesFree); idx++)
            {
                /* Add rule */
                ruleId = 10000 + idx;
                ruleAttributes.priority = 10000;
                UNIQUE_PCL_RULE_SET_MAC(&tcamRuleType,&pclPattern,ruleId);
                st = cpssDxChVirtualTcamRuleWrite(vTcamMngId, vTcamId, ruleId, &ruleAttributes,
                                                    &tcamRuleType, &ruleData);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamRuleWrite failed\n");
            }
        }

        /*
         10. Check that adding new rule fail in any priority. (cpssDxChVirtualTcamRuleWrite)
        */

        idx = 20;
        for (ruleAttributes.priority = 1; ruleAttributes.priority < 1001; ruleAttributes.priority += 100)
        {
            /* Add rule */
            ruleId = (10 * ruleAttributes.priority) + idx;
            UNIQUE_PCL_RULE_SET_MAC(&tcamRuleType,&pclPattern,ruleId);
            st = cpssDxChVirtualTcamRuleWrite(vTcamMngId, vTcamId, ruleId, &ruleAttributes,
                                                &tcamRuleType, &ruleData);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_FULL, st, "cpssDxChVirtualTcamRuleWrite failed\n");
        }

        /*  Removing vTCAM manager */
        vTcamDeleteConsecutiveRules(vTcamMngId,
                                vTcamId,
                                CPSS_DXCH_VIRTUAL_TCAM_START_RULE_ID_CNS,
                                DELETE_ALL_CNS);
        st = cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListRemove failed for vTcamMngId = %d\n",
                                     vTcamMngId);
        st = cpssDxChVirtualTcamManagerDelete(vTcamMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDelete failed for vTcamMngId = %d\n",
                                     vTcamMngId);
    }

    if (isTestSkipped)
    {
        SKIP_TEST_MAC;
    }
}

/*
    for new/existing vtcam :
    fill element of CPSS_DXCH_VIRTUAL_TCAM_CHECK_INFO_STC with info from VT_SEQUENCE
*/
#define FILL_MEMORY_AVAILABILITY_INFO_MAC(vTcamMngId,memCheckPtr,sequencePtr)  \
    (memCheckPtr)->isExist = (isVtcamInBmp((sequencePtr)->vTcamId) ? GT_TRUE :  GT_FALSE); \
    (memCheckPtr)->vTcamId = (sequencePtr)->vTcamId;            \
    if( isVtcamInBmp((sequencePtr)->vTcamId))                   \
    {                                                           \
        CPSS_DXCH_VIRTUAL_TCAM_USAGE_STC   vTcamUsage = {0,0};  \
        GT_U32                             rulesAmount;         \
        cpssDxChVirtualTcamUsageGet(vTcamMngId, (sequencePtr)->vTcamId, &vTcamUsage);               \
        rulesAmount = vTcamUsage.rulesUsed + vTcamUsage.rulesFree;                                  \
        cpssDxChVirtualTcamInfoGet(vTcamMngId, (sequencePtr)->vTcamId, &(memCheckPtr)->vTcamInfo);  \
        if ((sequencePtr)->operation == VT_DECREASE) rulesAmount -= (sequencePtr)->rulesAmount;     \
        if ((sequencePtr)->operation == VT_INCREASE) rulesAmount += (sequencePtr)->rulesAmount;     \
        (memCheckPtr)->vTcamInfo.guaranteedNumOfRules = rulesAmount;                                \
    }                                                           \
    else                                                        \
    {                                                           \
        (memCheckPtr)->vTcamInfo.clientGroup = (sequencePtr)->clientGroup;/* TTI and IPCL */ \
        (memCheckPtr)->vTcamInfo.hitNumber = 0;                     \
        (memCheckPtr)->vTcamInfo.guaranteedNumOfRules = (sequencePtr)->rulesAmount;   \
        (memCheckPtr)->vTcamInfo.ruleSize = (sequencePtr)->ruleSize;\
        (memCheckPtr)->vTcamInfo.autoResize = GT_FALSE;              \
        (memCheckPtr)->vTcamInfo.ruleAdditionMethod =                          \
            priorityMode == GT_TRUE ?                                          \
                CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E:        \
                CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E;   \
    }


/* check that the given array should be able to fit into the TCAM */
static void memoryAvailabilityCheckReady(
    IN GT_U32   vTcamMngId,
    IN CPSS_DXCH_VIRTUAL_TCAM_CHECK_INFO_STC * vTcamCheckInfoArr,
    IN GT_U32                                  vTcamAmount
)
{
    GT_STATUS   st;
    CPSS_DXCH_VIRTUAL_TCAM_AVAILABILTY_ENT          tcamAvailability;

    st = cpssDxChVirtualTcamMemoryAvailabilityCheck(vTcamMngId,
        vTcamAmount,vTcamCheckInfoArr,&tcamAvailability);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamMemoryAvailabilityCheck failed \n");

    UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_DXCH_VIRTUAL_TCAM_AVAILABILTY_AVAILABLE_E,
                                 tcamAvailability,
                                 "cpssDxChVirtualTcamMemoryAvailabilityCheck expected 'ready now' \n");

    return;
}

/* check that the given array CAN not fit into the current TCAM */
static void memoryAvailabilityCheckNotReadyNow(
    IN GT_U32   vTcamMngId,
    IN CPSS_DXCH_VIRTUAL_TCAM_CHECK_INFO_STC * vTcamCheckInfoArr,
    IN GT_U32                                  vTcamAmount
)
{
    GT_STATUS   st;
    CPSS_DXCH_VIRTUAL_TCAM_AVAILABILTY_ENT          tcamAvailability;

    st = cpssDxChVirtualTcamMemoryAvailabilityCheck(vTcamMngId,
        vTcamAmount,vTcamCheckInfoArr,&tcamAvailability);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamMemoryAvailabilityCheck failed \n");

    UTF_VERIFY_NOT_EQUAL0_STRING_MAC(CPSS_DXCH_VIRTUAL_TCAM_AVAILABILTY_AVAILABLE_E,
                                 tcamAvailability,
                                 "cpssDxChVirtualTcamMemoryAvailabilityCheck not expected 'ready now' \n");

}

/* check that the given array contains bad parameters */
static void memoryAvailabilityCheckBadParam(
    IN GT_U32   vTcamMngId,
    IN CPSS_DXCH_VIRTUAL_TCAM_CHECK_INFO_STC * vTcamCheckInfoArr,
    IN GT_U32                                  vTcamAmount
)
{
    GT_STATUS   st;
    CPSS_DXCH_VIRTUAL_TCAM_AVAILABILTY_ENT          tcamAvailability;

    st = cpssDxChVirtualTcamMemoryAvailabilityCheck(vTcamMngId,
        vTcamAmount,vTcamCheckInfoArr,&tcamAvailability);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChVirtualTcamMemoryAvailabilityCheck unexpected rc \n");

}

/*
GT_STATUS cpssDxChVirtualTcamMemoryAvailabilityCheck
(
    IN   GT_U32                                 vTcamMngId,
    IN   GT_U32                                 vTcamAmount,
    IN   CPSS_DXCH_VIRTUAL_TCAM_CHECK_INFO_STC  vTcamCheckInfoArr[],
    OUT  CPSS_DXCH_VIRTUAL_TCAM_AVAILABILTY_ENT *tcamAvailabilityPtr
)
*/
/*  from test design doc :
    http://docil.marvell.com/webtop/drl/objectId/0900dd88801b5ab6

1.        Check from empty tcam .. and grow:
a.        Call api with array of 'all new' many vtcams in tcam see table 1
i.        The api should indicate 'ready'
b.        Start to add the vtcams , one by one :
i.        the api for same array still should indicate 'ready' (the 'already added' should be as 'exists = GT_TRUE')
ii.        check with array with another element 'size = 10B , guarantee = 1'
1.        the api should indicate 'never'
*/
UTF_TEST_CASE_MAC(cpssDxChVirtualTcam_memoryAvailabilityCheck_fromEmptyAndGrow)
{
    GT_STATUS                                  st   = GT_OK;
    GT_U8                                      dev;
    GT_U32                                     numOfDevs = 0;
    GT_U32                                     notAppFamilyBmp = 0;
    GT_U32                                     utfFamilyBit = 0;
    GT_BOOL                                    isTestSkipped = GT_TRUE;
    GT_U32                                     vTcamMngId;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC            vTcamInfo;
    VT_SEQUENCE                                *origSequencePtr;
    VT_SEQUENCE                                *sequencePtr;
    GT_U32                                     sequenceSize;
    GT_U32                                     idx;
    GT_U32                                          vTcamAmount;
    static CPSS_DXCH_VIRTUAL_TCAM_CHECK_INFO_STC    vTcamCheckInfoArr_full[100];
    static CPSS_DXCH_VIRTUAL_TCAM_CHECK_INFO_STC    vTcamCheckInfoArr_after_full[5];
    static CPSS_DXCH_VIRTUAL_TCAM_CHECK_INFO_STC    vTcamCheckInfoArr_bad_param[5];
    GT_U32  globalIndex;
    GT_U32  globalIndex_afterFull;
    GT_U32  globalIndex_bad_param;
    GT_U32  localIndex;
    GT_BOOL wasFull = GT_FALSE;

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* this feature is on eArch devices, xCat3 and Lion2 */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    notAppFamilyBmp &= (~UTF_XCAT3_E) & (~UTF_LION2_E);

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* 1. Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
            devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }
        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;

        switch (utfFamilyBit)
        {
            case UTF_XCAT3_E:
                origSequencePtr = xcat3_sequence;
                sequenceSize = sizeof(xcat3_sequence) / sizeof(xcat3_sequence[0]);
                break;

            case UTF_LION2_E:
                origSequencePtr = lion2_sequence;
                sequenceSize = sizeof(lion2_sequence) / sizeof(lion2_sequence[0]);
                break;

            case UTF_ALDRIN2_E:
            case UTF_FALCON_E:
            case UTF_CAELUM_E:
                origSequencePtr = bobk_sequence;
                sequenceSize = sizeof(bobk_sequence) / sizeof(bobk_sequence[0]);
                break;

            case UTF_ALDRIN_E:
            case UTF_AC3X_E:
                origSequencePtr = aldrin_sequence;
                sequenceSize = sizeof(aldrin_sequence) / sizeof(aldrin_sequence[0]);
                break;

            case UTF_BOBCAT2_E:
            case UTF_BOBCAT3_E:
            default:
                origSequencePtr = bc2_sequence;
                sequenceSize = sizeof(bc2_sequence) / sizeof(bc2_sequence[0]);
                break;
        }

        vTcamMngId = 1;
        cpssOsMemSet(&vTcamInfo, 0, sizeof(vTcamInfo));
        vTcamInfo.hitNumber     = 0;
        vTcamInfo.autoResize    = GT_FALSE;
        vTcamInfo.ruleAdditionMethod =
            priorityMode == GT_TRUE ?
                CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E:
                CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E;


        /* Create vTCAM manager */
        st = cpssDxChVirtualTcamManagerCreate(vTcamMngId);
            if (st == GT_ALREADY_EXIST)
                st = GT_OK;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerCreate failed\n");

        st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        if (st != GT_OK)
        {
            cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
            st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        }
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListAdd failed\n");

        /* fill <rulesAmount> for entries with <operation> = VT_COMPLETE */
        {
            /* do actual creation of the vtcams */
            sequencePtr = origSequencePtr;
            for (idx = 0; (idx < sequenceSize); idx++,sequencePtr++)
            {
                if(sequencePtr->rc == GT_FULL)
                {
                    break;
                }
                /* do the actual adding of the vtcam to the TCAM manager */
                /* this will fill <rulesAmount> for entries with <operation> = VT_COMPLETE */
                st = vtcamSequenceEntryProcess(origSequencePtr, idx, vTcamMngId, &vTcamInfo);
                UTF_VERIFY_NOT_EQUAL1_STRING_MAC(
                    GT_FAIL, st,
                    "vtcamSequenceEntryProcess failed, index: %d\n", idx);

            }

            /* delete all the created vtcams */
            sequencePtr = origSequencePtr;
            for (idx = 0; (idx < sequenceSize); idx++,sequencePtr++)
            {
                if(sequencePtr->rc == GT_FULL)
                {
                    break;
                }
                if(sequencePtr->rc == GT_OK)
                {
                    /* remove all the vtcams from the DB */
                    st = ut_cpssDxChVirtualTcamRemove(vTcamMngId,sequencePtr->vTcamId);
                    UTF_VERIFY_EQUAL1_STRING_MAC(
                        GT_OK, st,
                        "ut_cpssDxChVirtualTcamRemove failed, idx: %d\n", idx);
                }

            }
        }

        /* due to bug that test cpssDxChVirtualTcam_releaseBlocksCheck shows we need
           to remove devices , in order to really clean the DB of free segments */
        VT_emptyAllVtcams(vTcamMngId, numOfDevs);

        globalIndex = 0;
        globalIndex_afterFull = 0;
        globalIndex_bad_param = 0;
        /* add all the creating vtcams till table should be full */
        sequencePtr = origSequencePtr;
        wasFull = GT_FALSE;
        for (idx = 0; (idx < sequenceSize); idx++,sequencePtr++)
        {
            if(sequencePtr->rc == GT_FULL)
            {
                wasFull = GT_TRUE;
            }
            else if (wasFull == GT_TRUE)
            {
                /* starting new section after checking that table should be full */

                /* add no more elements to the array */
                break;
            }

            if(sequencePtr->rc == GT_OK)
            {
                FILL_MEMORY_AVAILABILITY_INFO_MAC(vTcamMngId,&vTcamCheckInfoArr_full[globalIndex],sequencePtr);
                globalIndex++;
            }
            else
            if(sequencePtr->rc == GT_FULL)
            {
                FILL_MEMORY_AVAILABILITY_INFO_MAC(vTcamMngId,&vTcamCheckInfoArr_after_full[globalIndex_afterFull],sequencePtr);
                globalIndex_afterFull++;
            }
            else
            if(sequencePtr->rc == GT_BAD_PARAM)
            {
                if (globalIndex_bad_param < (sizeof(vTcamCheckInfoArr_bad_param)/sizeof(vTcamCheckInfoArr_bad_param[0])))
                {
                    FILL_MEMORY_AVAILABILITY_INFO_MAC(vTcamMngId,&vTcamCheckInfoArr_bad_param[globalIndex_bad_param],sequencePtr);
                    globalIndex_bad_param++;
                }
            }
        }

        wasFull = GT_FALSE;


        localIndex = 0;
        sequencePtr = origSequencePtr;
        for (idx = 0; (idx < sequenceSize); idx++,sequencePtr++)
        {
            if(sequencePtr->rc == GT_FULL)
            {
                wasFull = GT_TRUE;
                localIndex = 0;
            }
            else if (wasFull == GT_TRUE)
            {
                /* starting new section after checking that table should be full */

                /* add no more elements to the array */
                break;
            }

            if(sequencePtr->rc == GT_OK)
            {
                vTcamAmount = globalIndex - localIndex;
                memoryAvailabilityCheckReady(vTcamMngId,
                    &vTcamCheckInfoArr_full[localIndex],/* skip those who already added */
                    vTcamAmount);

                /*check with array with another element 'size = 10B , guarantee = 1'*/
                vTcamCheckInfoArr_full[globalIndex] = vTcamCheckInfoArr_after_full[0];
                memoryAvailabilityCheckNotReadyNow(vTcamMngId,
                    &vTcamCheckInfoArr_full[localIndex],/* skip those who already added */
                    vTcamAmount+1);

                if(globalIndex_afterFull > 1)
                {
                    /*check with array with another element 'size = 10B , guarantee = 1'*/
                    vTcamCheckInfoArr_full[globalIndex] = vTcamCheckInfoArr_after_full[1];
                    memoryAvailabilityCheckNotReadyNow(vTcamMngId,
                        &vTcamCheckInfoArr_full[localIndex],/* skip those who already added */
                        vTcamAmount+1);
                }

                if(globalIndex_bad_param > 0)
                {
                    /*check with array with another element 'size = 10B , guarantee = 1'*/
                    vTcamCheckInfoArr_full[globalIndex] = vTcamCheckInfoArr_bad_param[0];
                    memoryAvailabilityCheckBadParam(vTcamMngId,
                        &vTcamCheckInfoArr_full[localIndex],/* skip those who already added */
                        vTcamAmount+1);
                }

                localIndex++;
            }
            else
            if(sequencePtr->rc == GT_FULL)
            {
                vTcamAmount = 1;
                memoryAvailabilityCheckNotReadyNow(vTcamMngId,
                    &vTcamCheckInfoArr_after_full[localIndex],
                    vTcamAmount);
                localIndex++;
            }

            /* do the actual adding of the vtcam to the TCAM manager */
            st = vtcamSequenceEntryProcess(origSequencePtr, idx, vTcamMngId, &vTcamInfo);
            UTF_VERIFY_NOT_EQUAL1_STRING_MAC(
                GT_FAIL, st,
                "vtcamSequenceEntryProcess failed, index: %d\n", idx);
        }

        /************************/
        /* the TCAM is FULL now */
        /************************/

        /************************/
        /* remove some vtcams in order to make space for new availability checks */
        /************************/


        st = cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st, "cpssDxChVirtualTcamManagerDevListRemove failed for vTcamMngId = %d\n",
            vTcamMngId);
        st = cpssDxChVirtualTcamManagerDelete(vTcamMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDelete failed for vTcamMngId = %d\n",
                                     vTcamMngId);
    }

    if (isTestSkipped)
    {
        SKIP_TEST_MAC;
    }
}

/* sum the number of actual rules allocated to the vtcams */
static void calcTotal10BRulesInAllVtcams(
    IN GT_U32                               vTcamMngId,
    OUT GT_U32                              *total10BAmountPtr /* can be NULL --> just for print info */
)
{
    GT_STATUS st;
    GT_U32  total10BRulesAllowed;
    GT_U32   total10BAmount = 0;/* total number of rules in term of 10B rules */
    CPSS_DXCH_VIRTUAL_TCAM_USAGE_STC   vTcamUsage;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC    vTcamInfo;
    GT_U32  ii;

    /* sum the number of actual rules allocated to the vtcams */
    for (ii = 0; ii <= CPSS_DXCH_VIRTUAL_TCAM_VTCAM_MAX_ID_CNS; ii++)
    {
        if(0==isVtcamInBmp(ii))
        {
            continue;
        }

        st = cpssDxChVirtualTcamUsageGet(vTcamMngId,ii,&vTcamUsage);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st,
            "cpssDxChVirtualTcamUsageGet failed, vtcamId[%d] \n", ii);

        st = cpssDxChVirtualTcamInfoGet(vTcamMngId,ii,&vTcamInfo);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChVirtualTcamInfoGet failed, vtcamId[%d] \n", ii);

        total10BAmount += (vTcamInfo.ruleSize+1) * (vTcamUsage.rulesUsed + vTcamUsage.rulesFree);
    }

    if(total10BAmountPtr)
    {
        *total10BAmountPtr = total10BAmount;
    }

    internal_ut_cpssDxChVirtualTcamTotalRulesAllowedPerDevGet(
                devListArr[0],
                CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_10_B_E,
                &total10BRulesAllowed);

    cpssOsPrintf("calcTotal10BRulesInAllVtcams : filled with [%d] of 10B rules , not allocated[%d] of 10B rules \n",
        total10BAmount , total10BRulesAllowed - total10BAmount);
    prvCpssDxChVirtualTcamDbVTcamDumpBlockFreeSpace(
        vTcamMngId, GT_TRUE /*perLookupOnly*/);
}

/*  from test design doc :
    http://docil.marvell.com/webtop/drl/objectId/0900dd88801b5ab6

2.        check from 'full table' with 'decrease resize of existing' and adding new vtcams on that memory.
a.        The tcam is full see table 1
b.        Check next array with the api
 (expected 'GT_OK' ' 'ready now')

 Table 2 - check memory availability of vtcams in same tcam manager
New / updated        vtcamId        clientGroup        hitNumber  ruleSize   guaranteedNumOfRules   Converted to     Total free
                                                                                                    '10bB' rules     '10bB' rules
Updated               5             TTI                   0        20        1002-500                   500*2        1000
Updated               6             TTI                   0        30        1000-355                   355*3        2065
Updated               7             TTI                   0        10        1140-143                   143*1        2208
Updated               8             PCL                   0        20        1002-399                   399*2        598
Updated               9             PCL                   0        40        2000-25                    25*4         698
Updated               10            PCL                   0        50        1000-443                   443*5        2913
Updated               11            PCL                   0        10        7002-666                   666*1        3579
Updated               12            PCL                   0        60        500-5                      5*6          3609
Updated               13            PCL                   0        30        100-9                      9*3          3636
Updated               14            PCL                   0        50        814-2                      2*5          3646
Updated               15            PCL                   0        10        814-8                      8*1          3654
Updated               16            PCL                   0        10        530+1505                   1505*1       2149
New                   50            TTI                   0        50        400                        2000         208
New                   52            TTI                   0        10        208                        208          0
New                   51            PCL                   0        30        150                        150*3        1849
New                   53            PCL                   0        20        220                        220*3        1189
New                   54            PCL                   0        40        275                        275*4        89
New                   55            PCL                   0        10        89                         89*1         0

?

Check it        with additional element : (expected 'GT_FULL')
New                   56        TTI                10        1        1        GT_FULL (never available)

Check it with additional element: (expected 'GT_FULL')
New                   57        PCL        0        10        1        1        0

Check it with updated entry element: (expected 'GT_FULL')
Updated               12        PCL        0        60        500-4        -->was -5

Check it with updated entry element: (expected 'GT_FULL')
Updated               16        PCL        0        10        530+1506     -->was +1505

*/
static VT_SEQUENCE bc2_sequence_memoryAvailabilityCheck_fromFullWithUpdates[] =
{
    {VT_DECREASE,    5, CG_TTI,  RS_20,   /*minus*/  560, GT_OK},
    {VT_DECREASE,    6, CG_TTI,  RS_30,   /*minus*/  394, GT_OK},
    {VT_DECREASE,    7, CG_TTI,  RS_10,   /*minus*/  142, GT_OK},

    {VT_DECREASE,    8, CG_PCL,  RS_20,   /*minus*/  398, GT_OK},
    {VT_DECREASE,    9, CG_PCL,  RS_40,   /*minus*/   24, GT_OK},
    {VT_DECREASE,   10, CG_PCL,  RS_50,   /*minus*/  442, GT_OK},
    {VT_DECREASE,   11, CG_PCL,  RS_10,   /*minus*/  666, GT_OK},
    {VT_DECREASE,   12, CG_PCL,  RS_60,   /*minus*/    4, GT_OK},
    {VT_DECREASE,   13, CG_PCL,  RS_30,   /*minus*/    8, GT_OK},
    {VT_DECREASE,   14, CG_PCL,  RS_50,   /*minus*/    2, GT_OK},
    {VT_DECREASE,   15, CG_PCL,  RS_10,   /*minus*/    8, GT_OK},
    {VT_INCREASE,   16, CG_PCL,  RS_10,   /*plus*/  1506, GT_OK},/*1504,1505 took 1506*/

    {VT_CREATE,    50, CG_TTI,  RS_50,              402, GT_OK},
    {VT_CREATE,    52, CG_TTI,  RS_10,              426, GT_OK},

    {VT_CREATE,    51, CG_PCL,  RS_30,              150, GT_OK},
    {VT_CREATE,    53, CG_PCL,  RS_20,              220, GT_OK},
    {VT_CREATE,    54, CG_PCL,  RS_40,              186, GT_OK},
    {VT_CREATE,    55, CG_PCL,  RS_10,              /*688*/ 684, GT_OK},

    {VT_CREATE,    56, CG_TTI,  RS_10,                1, GT_FULL},

    {VT_CREATE,    57, CG_PCL,  RS_10,                1, GT_FULL},

    /* must be last */
    {VT_NO_MORE}

};

static VT_SEQUENCE bobk_sequence_memoryAvailabilityCheck_fromFullWithUpdates[] =
{
    {VT_DECREASE,    5, CG_TTI,  RS_20,   /*minus*/  280, GT_OK},
    {VT_DECREASE,    6, CG_TTI,  RS_30,   /*minus*/  197, GT_OK},
    {VT_DECREASE,    7, CG_TTI,  RS_10,   /*minus*/   71, GT_OK},

    {VT_DECREASE,    8, CG_PCL,  RS_20,   /*minus*/  199, GT_OK},
    {VT_DECREASE,    9, CG_PCL,  RS_40,   /*minus*/   12, GT_OK},
    {VT_DECREASE,   10, CG_PCL,  RS_50,   /*minus*/  221, GT_OK},
    {VT_DECREASE,   11, CG_PCL,  RS_10,   /*minus*/  333, GT_OK},
    {VT_DECREASE,   12, CG_PCL,  RS_60,   /*minus*/    2, GT_OK},
    {VT_DECREASE,   13, CG_PCL,  RS_30,   /*minus*/    4, GT_OK},
    {VT_DECREASE,   14, CG_PCL,  RS_50,   /*minus*/    1, GT_OK},
    {VT_DECREASE,   15, CG_PCL,  RS_10,   /*minus*/    4, GT_OK},
    {VT_INCREASE,   16, CG_PCL,  RS_10,   /*plus*/   752, GT_OK},

    {VT_CREATE,    50, CG_TTI,  RS_50,              201, GT_OK},
    {VT_CREATE,    52, CG_TTI,  RS_10,              /*207*/204, GT_OK},

    {VT_CREATE,    51, CG_PCL,  RS_30,               75, GT_OK},
    {VT_CREATE,    53, CG_PCL,  RS_20,              110, GT_OK},
    {VT_CREATE,    54, CG_PCL,  RS_40,               97, GT_OK},
    {VT_CREATE,    55, CG_PCL,  RS_10,              /*328*/312, GT_OK},

    {VT_CREATE,    56, CG_TTI,  RS_10,                1, GT_FULL},

    {VT_CREATE,    57, CG_PCL,  RS_10,                1, GT_FULL},

    /* must be last */
    {VT_NO_MORE}

};

static VT_SEQUENCE aldrin_sequence_memoryAvailabilityCheck_fromFullWithUpdates[] =
{
    {VT_DECREASE,    5, CG_TTI,  RS_20,   /*minus*/  140, GT_OK},
    {VT_DECREASE,    6, CG_TTI,  RS_30,   /*minus*/   94, GT_OK},
    {VT_DECREASE,    7, CG_TTI,  RS_10,   /*minus*/   33, GT_OK},

    {VT_DECREASE,    8, CG_PCL,  RS_20,   /*minus*/   99, GT_OK},
    {VT_DECREASE,    9, CG_PCL,  RS_40,   /*minus*/    6, GT_OK},
    {VT_DECREASE,   10, CG_PCL,  RS_50,   /*minus*/  110, GT_OK},
    {VT_DECREASE,   11, CG_PCL,  RS_10,   /*minus*/  141, GT_OK},
    {VT_DECREASE,   12, CG_PCL,  RS_60,   /*minus*/    1, GT_OK},
    {VT_DECREASE,   13, CG_PCL,  RS_30,   /*minus*/    2, GT_OK},
    {VT_DECREASE,   14, CG_PCL,  RS_50,   /*minus*/    1, GT_OK},
    {VT_DECREASE,   15, CG_PCL,  RS_10,   /*minus*/    2, GT_OK},
    {VT_INCREASE,   16, CG_PCL,  RS_10,   /*plus*/   378, GT_OK},

    {VT_CREATE,    50, CG_TTI,  RS_50,              100, GT_OK},
    {VT_CREATE,    52, CG_TTI,  RS_10,               84, GT_OK},

    {VT_CREATE,    51, CG_PCL,  RS_30,               38, GT_OK},
    {VT_CREATE,    53, CG_PCL,  RS_20,               56, GT_OK},
    {VT_CREATE,    54, CG_PCL,  RS_40,               48, GT_OK},
    {VT_CREATE,    55, CG_PCL,  RS_10,              132, GT_OK},

    {VT_CREATE,    56, CG_TTI,  RS_10,                1, GT_FULL},

    {VT_CREATE,    57, CG_PCL,  RS_10,                1, GT_FULL},

    /* must be last */
    {VT_NO_MORE}

};

static VT_SEQUENCE xcat3_sequence_memoryAvailabilityCheck_fromFullWithUpdates[] =
{
    {VT_DECREASE,   12, CG_PCL,  RS_60,   /*minus*/   50, GT_OK},
    {VT_DECREASE,   13, CG_PCL,  RS_30,   /*minus*/    4, GT_OK},
    {VT_INCREASE,   16, CG_PCL,  RS_30,   /*plus*/    24, GT_OK},

    {VT_CREATE,    51, CG_PCL,  RS_30,                80, GT_OK},

    {VT_CREATE,    57, CG_PCL,  RS_30,                 1, GT_FULL},

    /* must be last */
    {VT_NO_MORE}

};

static VT_SEQUENCE lion2_sequence_memoryAvailabilityCheck_fromFullWithUpdates[] =
{
    {VT_DECREASE,   12, CG_PCL,  RS_60,   /*minus*/   32, GT_OK},
    {VT_DECREASE,   13, CG_PCL,  RS_30,   /*minus*/    4, GT_OK},
    {VT_INCREASE,   16, CG_PCL,  RS_30,   /*plus*/    16, GT_OK},

    {VT_CREATE,    51, CG_PCL,  RS_30,                52, GT_OK},

    {VT_CREATE,    57, CG_PCL,  RS_30,                 1, GT_FULL},

    /* must be last */
    {VT_NO_MORE}

};

/*  struct to allow override specific row in DB of type VT_SEQUENCE

    vTcamId - the vTcamId to find in the overridden DB of type VT_SEQUENCE
    updatedRulesAmount - the new rules amount to override in DB of type VT_SEQUENCE
    firstGT_FULL_vTcamId - the first vtcam that will issue GT_FULL
*/
typedef struct{
    GT_U32    vTcamId;
    GT_U32    updatedRulesAmount;
    GT_U32    firstGT_FULL_vTcamId;
}VT_OVERRIDE_RULES_AMOUNT_STC;

static VT_OVERRIDE_RULES_AMOUNT_STC bc2_sequence_memoryAvailabilityCheck_fromFullWithUpdates_permutations[] =
{
/* vTcamId *//*updatedRulesAmount*/
     {11        ,  100 , 55 /*54*/} /*  decrease by  100 instead of    666 */
    ,{16        , 1524 , 55}     /*  increase by 1518 instead of 1506 */ /*1518*/
};

static VT_OVERRIDE_RULES_AMOUNT_STC bobk_sequence_memoryAvailabilityCheck_fromFullWithUpdates_permutations[] =
{
/* vTcamId *//*updatedRulesAmount*/
     {10        ,  180 , 55}     /*  decrease by 180 instead of  221 */
    ,{16        ,  780 , 55}     /*  increase by 780 instead of 752 */
};

static VT_OVERRIDE_RULES_AMOUNT_STC aldrin_sequence_memoryAvailabilityCheck_fromFullWithUpdates_permutations[] =
{
/* vTcamId *//*updatedRulesAmount*/
     {11        ,   25 , 55 /*54*/} /*  decrease by  25 instead of   141 */
    ,{16        ,  381 , 55}     /*  increase by 381 instead of 378 */
};

static VT_OVERRIDE_RULES_AMOUNT_STC xcat3_sequence_memoryAvailabilityCheck_fromFullWithUpdates_permutations[] =
{
/* vTcamId *//*updatedRulesAmount*/
     {12        ,   49 , 51}     /*  decrease by  49 instead of  50 */
    ,{16        ,   28 , 51}     /*  increase by  28 instead of  26 */
};

static VT_OVERRIDE_RULES_AMOUNT_STC lion2_sequence_memoryAvailabilityCheck_fromFullWithUpdates_permutations[] =
{
/* vTcamId *//*updatedRulesAmount*/
     {12        ,   30 , 51}     /*  decrease by  30 instead of  32 */
    ,{16        ,   20 , 51}     /*  increase by  20 instead of  16 */
};

/* at start the tcam expected to be full , and can not add any new vtcam .
    this function will start to decrease/delete vtcams to make place for new
    added vtcams

*/
static void memoryAvailabilityCheck_fromFullWithUpdates
(
    IN GT_U32                               vTcamMngId,
    IN VT_SEQUENCE                          *sequencePassedArrPtr,
    IN VT_OVERRIDE_RULES_AMOUNT_STC         *permutationPtr,
    IN CPSS_DXCH_VIRTUAL_TCAM_INFO_STC         *dummy_vTcamInfoPtr
)
{
    static CPSS_DXCH_VIRTUAL_TCAM_CHECK_INFO_STC    vTcamCheckInfoArr_full[60];
    static VT_SEQUENCE                              sequenceArr[60];
    VT_SEQUENCE                                     *sequenceArrPtr;
    GT_U32       globalIndex;
    GT_U32       globalAmount_afterFull;
    GT_STATUS    st;
    VT_SEQUENCE  *sequencePtr;
    GT_U32       idx;
    GT_U32       vTcamAmount;

    /* copy sequencePassedArrPtr array to sequenceArr */
    for(idx = 0;
        sequencePassedArrPtr[idx].operation != VT_NO_MORE;
        idx++)
    {
        sequenceArr[idx] = sequencePassedArrPtr[idx];
    }
    sequenceArr[idx] = sequencePassedArrPtr[idx]; /* entry with VT_NO_MORE */
    sequenceArrPtr = &(sequenceArr[0]);

    if (permutationPtr != NULL)
    {
        for (idx = 0, sequencePtr = &sequenceArrPtr[0];
            (sequencePtr->operation != VT_NO_MORE);
            sequencePtr++, idx++)
        {
            if (permutationPtr->vTcamId == sequencePtr->vTcamId)
            {
                /* update the entry */
                sequencePtr->rulesAmount = permutationPtr->updatedRulesAmount;
            }
            if (permutationPtr->firstGT_FULL_vTcamId == sequencePtr->vTcamId)
            {
                /* update the entry */
                sequencePtr->rc = GT_FULL;
            }
        }
    }

    globalIndex = 0;
    globalAmount_afterFull = 0;
    /* add all the creating vtcams till table should be full */
    for (idx = 0, sequencePtr = &sequenceArrPtr[0];
        (sequencePtr->operation != VT_NO_MORE);
        sequencePtr++, idx++)
    {
        FILL_MEMORY_AVAILABILITY_INFO_MAC(
            vTcamMngId,&vTcamCheckInfoArr_full[globalIndex],sequencePtr);
        if (sequencePtr->rc != GT_FULL)
        {
            globalIndex++;
        }
        else
        {
            globalAmount_afterFull = 1;
            break;
        }
    }

    for (idx = 0, sequencePtr = &sequenceArrPtr[0];
        (idx < globalIndex);
        sequencePtr++, idx++)
    {
        vTcamAmount = globalIndex - idx;

        memoryAvailabilityCheckReady(vTcamMngId,
            &vTcamCheckInfoArr_full[idx],/* skip those who already added */
            vTcamAmount);

        if (globalAmount_afterFull > 0)
        {
            memoryAvailabilityCheckNotReadyNow(vTcamMngId,
                &vTcamCheckInfoArr_full[idx],/* skip those who already added */
                (vTcamAmount + 1));
        }

        /* do the actual adding of the vtcam to the TCAM manager */
        /* this will fill <rulesAmount> for entries with <operation> = VT_COMPLETE */
        st = vtcamSequenceEntryProcess(sequenceArrPtr, idx, vTcamMngId, dummy_vTcamInfoPtr);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st,
            "vtcamSequenceEntryProcess failed, index: %d\n", idx);

        calcTotal10BRulesInAllVtcams(vTcamMngId,NULL);
    }

    if (globalAmount_afterFull > 0)
    {
        memoryAvailabilityCheckNotReadyNow(vTcamMngId,
            &vTcamCheckInfoArr_full[globalIndex],/* skip those who already added */
            1 /*vTcamAmount*/);
    }
}

static void createAndUpdate_fromFullWithUpdates
(
    IN GT_U32                               vTcamMngId,
    IN VT_SEQUENCE                          *sequenceArrPtr,
    IN VT_OVERRIDE_RULES_AMOUNT_STC         *permutationPtr,
    IN CPSS_DXCH_VIRTUAL_TCAM_INFO_STC         *dummy_vTcamInfoPtr
)
{
    GT_STATUS    st;
    VT_SEQUENCE  *sequencePtr;
    GT_U32       idx;

    permutationPtr = permutationPtr;

    idx = 0;

    for(sequencePtr = &sequenceArrPtr[0];
        sequencePtr->operation != VT_NO_MORE ;
        sequencePtr++,idx++)
    {
        /* do the actual update of the vtcam to the TCAM manager */
        st = vtcamSequenceEntryProcess(sequenceArrPtr, idx, vTcamMngId, dummy_vTcamInfoPtr);

        /* sum the number of actual rules allocated to the vtcams */
        calcTotal10BRulesInAllVtcams(vTcamMngId,NULL);

        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(
            GT_FAIL, st,
            "vtcamSequenceEntryProcess failed, index: %d vTcamId: %d requredRc: %d\n",
            idx, sequenceArrPtr[idx].vTcamId, sequenceArrPtr[idx].rc);
    }
}

static void doSequenceTill_GT_FULL
(
    IN GT_U32                               vTcamMngId,
    IN VT_SEQUENCE                          *sequenceArrPtr,
    OUT CPSS_DXCH_VIRTUAL_TCAM_INFO_STC     *vTcamInfoPtr
)
{
    GT_STATUS                                  st;
    VT_SEQUENCE                                *sequencePtr;
    GT_U32                                     idx;
    GT_U32  total10BRulesAllowed;
    GT_U32   total10BAmount;/* total number of rules in term of 10B rules */

    cpssOsMemSet(vTcamInfoPtr, 0, sizeof(*vTcamInfoPtr));
    vTcamInfoPtr->hitNumber     = 0;
    vTcamInfoPtr->autoResize    = GT_FALSE;
    vTcamInfoPtr->ruleAdditionMethod =
        priorityMode == GT_TRUE ?
            CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E:
            CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E;

    /* do actual creation of the vtcams */
    sequencePtr = sequenceArrPtr;
    for (idx = 0; /* no check */; idx++,sequencePtr++)
    {
        if(sequencePtr->rc == GT_FULL)
        {
            break;
        }
        /* do the actual adding of the vtcam to the TCAM manager */
        /* this will fill <rulesAmount> for entries with <operation> = VT_COMPLETE */
        st = vtcamSequenceEntryProcess(sequenceArrPtr, idx, vTcamMngId, vTcamInfoPtr);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(
            GT_FAIL, st,
            "vtcamSequenceEntryProcess failed, index: %d\n", idx);
    }

    /* sum the number of actual rules allocated to the vtcams */
    calcTotal10BRulesInAllVtcams(vTcamMngId,&total10BAmount);

    internal_ut_cpssDxChVirtualTcamTotalRulesAllowedPerDevGet(
                devListArr[0],
                CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_10_B_E,
                &total10BRulesAllowed);

    /* allocation granularity */
    if (total10BRulesAllowed > (total10BAmount + 5))
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(
            total10BRulesAllowed, total10BAmount,
            "totalAmount should be as TCAM size \n");
    }

    return;
}

/* create TCAM manager with it's devices */
static void createTcamManagerWithDevices
(
    IN GT_U32                               vTcamMngId,
    IN GT_U32                               numOfDevs
)
{
    GT_STATUS                                  st;

    /* clear the BMP */
    clearVtcamBmp();

    /* Create vTCAM manager */
    st = cpssDxChVirtualTcamManagerCreate(vTcamMngId);
    if (st == GT_ALREADY_EXIST)
        st = GT_OK;
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerCreate failed\n");

    st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
    if (st != GT_OK)
    {
        cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
        st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
    }
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListAdd failed\n");
}
/* create/resize-increase/resize-decrease from state of 'full TCAM' */
UTF_TEST_CASE_MAC(cpssDxChVirtualTcam_createAndUpdate_fromFullWithUpdates)
{
    GT_STATUS   st;
    GT_U8                                      dev;
    GT_U32                                     numOfDevs = 0;
    GT_U32                                     notAppFamilyBmp = 0;
    GT_U32                                     utfFamilyBit = 0;
    GT_BOOL                                    isTestSkipped = GT_TRUE;
    VT_SEQUENCE                                 *sequenceArrPtr;
    VT_SEQUENCE                                 *fillTable_sequenceArrPtr;
    GT_U32                                     vTcamMngId = 1;
    /*GT_U32                                     ii;*/
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC             dummy_vTcamInfo;

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* 1. Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
            devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }
        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;

        switch (utfFamilyBit)
        {
            case UTF_ALDRIN2_E:
            case UTF_FALCON_E:
            case UTF_CAELUM_E:
                sequenceArrPtr = &bobk_sequence_memoryAvailabilityCheck_fromFullWithUpdates[0];
                fillTable_sequenceArrPtr = bobk_sequence;
                break;

            case UTF_ALDRIN_E:
            case UTF_AC3X_E:
                sequenceArrPtr = &aldrin_sequence_memoryAvailabilityCheck_fromFullWithUpdates[0];
                fillTable_sequenceArrPtr = aldrin_sequence;
                break;

            case UTF_BOBCAT2_E:
            case UTF_BOBCAT3_E:
            default:
                sequenceArrPtr = &bc2_sequence_memoryAvailabilityCheck_fromFullWithUpdates[0];
                fillTable_sequenceArrPtr = bc2_sequence;
                break;
        }

        createTcamManagerWithDevices(vTcamMngId,numOfDevs);

        /* fill the table */
        doSequenceTill_GT_FULL(vTcamMngId,fillTable_sequenceArrPtr,&dummy_vTcamInfo);

        /* first test - 'no permutation' */
        createAndUpdate_fromFullWithUpdates(vTcamMngId,sequenceArrPtr,
            NULL,
            &dummy_vTcamInfo);

        calcTotal10BRulesInAllVtcams(vTcamMngId,NULL);

        st = cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st, "cpssDxChVirtualTcamManagerDevListRemove failed for vTcamMngId = %d\n",
            vTcamMngId);
        st = cpssDxChVirtualTcamManagerDelete(vTcamMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDelete failed for vTcamMngId = %d\n",
                                     vTcamMngId);


    }

    if (isTestSkipped)
    {
        SKIP_TEST_MAC;
    }
}


UTF_TEST_CASE_MAC(cpssDxChVirtualTcam_memoryAvailabilityCheck_fromFullWithUpdates)
{
    GT_STATUS   st;
    GT_U8                                      dev;
    GT_U32                                     numOfDevs = 0;
    GT_U32                                     notAppFamilyBmp = 0;
    GT_U32                                     utfFamilyBit = 0;
    GT_BOOL                                    isTestSkipped = GT_TRUE;
    GT_U32                                      numOfPermutations;
    VT_OVERRIDE_RULES_AMOUNT_STC                *permutationArrPtr;
    VT_SEQUENCE                                 *sequenceArrPtr;
    VT_SEQUENCE                                 *fillTable_sequenceArrPtr;
    GT_U32                                     vTcamMngId = 1;
    GT_U32                                     ii;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC             dummy_vTcamInfo;

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* this feature is on eArch devices, xCat3 and Lion2 */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    notAppFamilyBmp &= (~UTF_XCAT3_E) & (~UTF_LION2_E);

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* 1. Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
            devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }
        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;

        switch (utfFamilyBit)
        {
            case UTF_XCAT3_E:
                numOfPermutations =
                        sizeof(xcat3_sequence_memoryAvailabilityCheck_fromFullWithUpdates_permutations)/
                        sizeof(xcat3_sequence_memoryAvailabilityCheck_fromFullWithUpdates_permutations[0]);
                permutationArrPtr = &xcat3_sequence_memoryAvailabilityCheck_fromFullWithUpdates_permutations[0];

                sequenceArrPtr = &xcat3_sequence_memoryAvailabilityCheck_fromFullWithUpdates[0];
                fillTable_sequenceArrPtr = xcat3_sequence;
                break;

            case UTF_LION2_E:
                numOfPermutations =
                        sizeof(lion2_sequence_memoryAvailabilityCheck_fromFullWithUpdates_permutations)/
                        sizeof(lion2_sequence_memoryAvailabilityCheck_fromFullWithUpdates_permutations[0]);
                permutationArrPtr = &lion2_sequence_memoryAvailabilityCheck_fromFullWithUpdates_permutations[0];

                sequenceArrPtr = &lion2_sequence_memoryAvailabilityCheck_fromFullWithUpdates[0];
                fillTable_sequenceArrPtr = lion2_sequence;
                break;

            case UTF_ALDRIN2_E:
            case UTF_FALCON_E:
            case UTF_CAELUM_E:
                numOfPermutations =
                        sizeof(bobk_sequence_memoryAvailabilityCheck_fromFullWithUpdates_permutations)/
                        sizeof(bobk_sequence_memoryAvailabilityCheck_fromFullWithUpdates_permutations[0]);
                permutationArrPtr = &bobk_sequence_memoryAvailabilityCheck_fromFullWithUpdates_permutations[0];

                sequenceArrPtr = &bobk_sequence_memoryAvailabilityCheck_fromFullWithUpdates[0];
                fillTable_sequenceArrPtr = bobk_sequence;
                break;

            case UTF_ALDRIN_E:
            case UTF_AC3X_E:
                numOfPermutations =
                        sizeof(aldrin_sequence_memoryAvailabilityCheck_fromFullWithUpdates_permutations)/
                        sizeof(aldrin_sequence_memoryAvailabilityCheck_fromFullWithUpdates_permutations[0]);
                permutationArrPtr = &aldrin_sequence_memoryAvailabilityCheck_fromFullWithUpdates_permutations[0];

                sequenceArrPtr = &aldrin_sequence_memoryAvailabilityCheck_fromFullWithUpdates[0];
                fillTable_sequenceArrPtr = aldrin_sequence;
                break;

            case UTF_BOBCAT2_E:
            case UTF_BOBCAT3_E:
            default:
                numOfPermutations =
                        sizeof(bc2_sequence_memoryAvailabilityCheck_fromFullWithUpdates_permutations)/
                        sizeof(bc2_sequence_memoryAvailabilityCheck_fromFullWithUpdates_permutations[0]);
                permutationArrPtr = &bc2_sequence_memoryAvailabilityCheck_fromFullWithUpdates_permutations[0];
                sequenceArrPtr = &bc2_sequence_memoryAvailabilityCheck_fromFullWithUpdates[0];
                fillTable_sequenceArrPtr = bc2_sequence;
                break;
        }

        /* clear the BMP */
        clearVtcamBmp();

        createTcamManagerWithDevices(vTcamMngId,numOfDevs);

        /* fill the table */
        doSequenceTill_GT_FULL(vTcamMngId,fillTable_sequenceArrPtr,&dummy_vTcamInfo);

        /* first test - 'no permutation' */
        memoryAvailabilityCheck_fromFullWithUpdates(vTcamMngId,sequenceArrPtr,
            NULL,
            &dummy_vTcamInfo);

        /* empty the table */
        VT_emptyAllVtcams(vTcamMngId, numOfDevs);

        /* next tests with minor change every time */
        for(ii = 0 ; ii < numOfPermutations ; ii++)
        {
            /* fill the table */
            doSequenceTill_GT_FULL(vTcamMngId,fillTable_sequenceArrPtr,&dummy_vTcamInfo);

            memoryAvailabilityCheck_fromFullWithUpdates(
                vTcamMngId,
                sequenceArrPtr,
                &permutationArrPtr[ii],
                &dummy_vTcamInfo);

            /* empty the table */
            VT_emptyAllVtcams(vTcamMngId, numOfDevs);

        }

        st = cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st, "cpssDxChVirtualTcamManagerDevListRemove failed for vTcamMngId = %d\n",
            vTcamMngId);
        st = cpssDxChVirtualTcamManagerDelete(vTcamMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDelete failed for vTcamMngId = %d\n",
                                     vTcamMngId);
    }

    if (isTestSkipped)
    {
        SKIP_TEST_MAC;
    }
}

/*
    check that the TCAM manager free the blocks properly after releasing vtcams
*/
UTF_TEST_CASE_MAC(cpssDxChVirtualTcam_releaseBlocksCheck)
{
    GT_STATUS                                  st;
    GT_U8                                      dev;
    GT_U32                                     numOfDevs = 0;
    GT_U32                                     notAppFamilyBmp = 0;
    GT_U32                                     utfFamilyBit = 0;
    GT_BOOL                                    isTestSkipped = GT_TRUE;
    GT_U32                                     vTcamMngId;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC            vTcamInfo;
    VT_SEQUENCE                                *origSequencePtr;
    VT_SEQUENCE                                *sequencePtr;
    GT_U32                                     sequenceSize;
    GT_U32                                     idx;
    GT_U32                                     vTcamId;

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* 1. Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
            devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }
        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;

        switch (utfFamilyBit)
        {
            case UTF_ALDRIN2_E:
            case UTF_FALCON_E:
            case UTF_CAELUM_E:
                origSequencePtr = bobk_sequence;
                sequenceSize = sizeof(bobk_sequence) / sizeof(bobk_sequence[0]);
                break;

            case UTF_ALDRIN_E:
            case UTF_AC3X_E:
                origSequencePtr = aldrin_sequence;
                sequenceSize = sizeof(aldrin_sequence) / sizeof(aldrin_sequence[0]);
                break;

            case UTF_BOBCAT2_E:
            case UTF_BOBCAT3_E:
            default:
                origSequencePtr = bc2_sequence;
                sequenceSize = sizeof(bc2_sequence) / sizeof(bc2_sequence[0]);
                break;
        }

        vTcamMngId = 1;
        cpssOsMemSet(&vTcamInfo, 0, sizeof(vTcamInfo));
        vTcamInfo.hitNumber     = 0;
        vTcamInfo.autoResize    = GT_FALSE;
        vTcamInfo.ruleAdditionMethod =
            priorityMode == GT_TRUE ?
                CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E:
                CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E;


        /* Create vTCAM manager */
        st = cpssDxChVirtualTcamManagerCreate(vTcamMngId);
            if (st == GT_ALREADY_EXIST)
                st = GT_OK;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerCreate failed\n");

        st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        if (st != GT_OK)
        {
            cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
            st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        }
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListAdd failed\n");

        /* do actual creation of the vtcams */
        sequencePtr = origSequencePtr;
        for (idx = 0; (idx < sequenceSize); idx++,sequencePtr++)
        {
            if(sequencePtr->rc == GT_FULL)
            {
                break;
            }
            /* do the actual adding of the vtcam to the TCAM manager */
            /* this will fill <rulesAmount> for entries with <operation> = VT_COMPLETE */
            st = vtcamSequenceEntryProcess(origSequencePtr, idx, vTcamMngId, &vTcamInfo);
            UTF_VERIFY_NOT_EQUAL1_STRING_MAC(
                GT_FAIL, st,
                "vtcamSequenceEntryProcess failed, index: %d\n", idx);

        }

        /* delete all the created vtcams */
        sequencePtr = origSequencePtr;
        for (idx = 0; (idx < sequenceSize); idx++,sequencePtr++)
        {
            if(sequencePtr->rc == GT_FULL)
            {
                break;
            }
            /* remove all the vtcams from the DB */
            st = ut_cpssDxChVirtualTcamRemove(vTcamMngId,sequencePtr->vTcamId);
            UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, st,
                "ut_cpssDxChVirtualTcamRemove failed, idx: %d\n", idx);
        }

        /* there are no vtcams now !!! (we deleted all) */
        /* lets add vtcam from another segment */
        vTcamInfo.clientGroup = 2;
        vTcamInfo.hitNumber = 2;
        vTcamInfo.guaranteedNumOfRules = (_3K/4);
        vTcamInfo.ruleSize = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_50_B_E;
        vTcamId = 151;

        /* do not call ut_cpssDxChVirtualTcamCreate to avoid any relation to
           'check availability' */
        st = cpssDxChVirtualTcamCreate(vTcamMngId, vTcamId, &vTcamInfo);
        /* check it as last line to allow the test fail but only after full cleanup */
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ut_cpssDxChVirtualTcamCreate failed for vTcamId = %d even though TCAM is empty !!! \n",
                                     vTcamId);

        st = cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st, "cpssDxChVirtualTcamManagerDevListRemove failed for vTcamMngId = %d\n",
            vTcamMngId);
        st = cpssDxChVirtualTcamManagerDelete(vTcamMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDelete failed for vTcamMngId = %d\n",
                                     vTcamMngId);

    }

    if (isTestSkipped)
    {
        SKIP_TEST_MAC;
    }
}

static VT_SEQUENCE sequence_causeError_60B[] =
{
    {VT_CREATE,    5, CG_TTI,  RS_20,  501, GT_OK}, /* (167 * 3) */
    {VT_CREATE,    6, CG_TTI,  RS_30,  502, GT_OK}, /* ((167 * 2) + (42 * (2 + 2))) */
    {VT_COMPLETE,  7, CG_TTI,  RS_NONE,  0, GT_OK}, /* (47 * (6 + 6)) */
    /* TTI filled 2 blocks */
    {VT_CREATE,    8, CG_PCL,  RS_20,  501, GT_OK},

    {VT_CREATE,    12, CG_PCL,  RS_60,  250, GT_OK}/* the CPSS fail on this case ! */
};
/*
    test to show error in adding 60 B that cause the CPSS to fail
    or to give more rules than asked for.
*/
UTF_TEST_CASE_MAC(cpssDxChVirtualTcam_causeError_60B)
{
    GT_STATUS                                  st;
    GT_U8                                      dev;
    GT_U32                                     numOfDevs = 0;
    GT_U32                                     notAppFamilyBmp = 0;
    GT_U32                                     utfFamilyBit = 0;
    GT_BOOL                                    isTestSkipped = GT_TRUE;
    GT_U32                                     vTcamMngId;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC            vTcamInfo;
    VT_SEQUENCE                                *origSequencePtr;
    VT_SEQUENCE                                *sequencePtr;
    GT_U32                                     sequenceSize;
    GT_U32                                     idx;
    CPSS_DXCH_VIRTUAL_TCAM_USAGE_STC           vTcamUsage;

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* 1. Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
            devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }
        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;

        origSequencePtr = sequence_causeError_60B;
        sequenceSize = sizeof(sequence_causeError_60B) / sizeof(sequence_causeError_60B[0]);

        vTcamMngId = 1;
        cpssOsMemSet(&vTcamInfo, 0, sizeof(vTcamInfo));
        vTcamInfo.hitNumber     = 0;
        vTcamInfo.autoResize    = GT_FALSE;
        vTcamInfo.ruleAdditionMethod =
            priorityMode == GT_TRUE ?
                CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E:
                CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E;

        /* Create vTCAM manager */
        st = cpssDxChVirtualTcamManagerCreate(vTcamMngId);
            if (st == GT_ALREADY_EXIST)
                st = GT_OK;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerCreate failed\n");

        st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        if (st != GT_OK)
        {
            cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
            st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        }
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListAdd failed\n");

        sequencePtr = origSequencePtr;
        for (idx = 0; (idx < sequenceSize); idx++,sequencePtr++)
        {
            /* do the actual adding of the vtcam to the TCAM manager */
            st = vtcamSequenceEntryProcess(origSequencePtr, idx, vTcamMngId, &vTcamInfo);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "vtcamSequenceEntryProcess failed\n");

            if(st == GT_OK && sequencePtr->ruleSize >= RS_40)
            {
                /* check that we got exactly the number of rules that we asked for */
                st = cpssDxChVirtualTcamUsageGet(
                    vTcamMngId, sequencePtr->vTcamId, &vTcamUsage);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamUsageGet failed\n");

                UTF_VERIFY_EQUAL0_STRING_MAC(0,vTcamUsage.rulesUsed, "used rules must be 0 \n");
                UTF_VERIFY_EQUAL0_STRING_MAC(sequencePtr->rulesAmount,vTcamUsage.rulesFree, "got different number of rules \n");
            }
        }

        st = cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st, "cpssDxChVirtualTcamManagerDevListRemove failed for vTcamMngId = %d\n",
            vTcamMngId);
        st = cpssDxChVirtualTcamManagerDelete(vTcamMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDelete failed for vTcamMngId = %d\n",
                                     vTcamMngId);

    }

    if (isTestSkipped)
    {
        SKIP_TEST_MAC;
    }
}
/* creation order that fail on bc2 */
/*
    NOTE: the table is NOT ordered but sizes just like in bc2_sequence_creation_order_error
*/
static VT_SEQUENCE bc2_sequence_creation_OK[] =
{
    {VT_CREATE,    5, CG_TTI,  RS_20, 1002, GT_OK},  /* 1002 % 3 = 0*/
    {VT_CREATE,    6, CG_TTI,  RS_30, 1000, GT_OK},  /* 1000 % 2 = 0*/
    {VT_CREATE,    7, CG_TTI,  RS_10, 1140, GT_OK},/* fill the second block */

    {VT_CREATE,    8, CG_PCL,  RS_20, 1002, GT_OK},  /* 1002 % 3 = 0*/
    {VT_CREATE,    9, CG_PCL,  RS_40, 2000, GT_OK},  /* 2000 % 1 = 0*//*free 4000 of 10B */
    {VT_CREATE,   10, CG_PCL,  RS_50, 1000, GT_OK},  /* 1000 % 1 = 0*//*free 1000 of 10B */
    {VT_CREATE,   11, CG_PCL,  RS_10, 7002, GT_OK},  /* 7002 % 6 = 0*//*fill (4000 + 1000) , others 2002%6 = 4 ??? */
    {VT_CREATE,   12, CG_PCL,  RS_60,  500, GT_OK},  /*  500 % 1 = 0*/
    {VT_CREATE,   13, CG_PCL,  RS_30,  100, GT_OK},  /*  100 % 2 = 0*/
    {VT_CREATE,   14, CG_PCL,  RS_50,  814, GT_OK},  /*  814 % 1 = 0*//*free 814 of 10B */
    {VT_CREATE,   15, CG_PCL,  RS_10,  810, GT_OK},  /* was 814 *//*  810 --> can feet 814 of 50'th */
    {VT_CREATE,   16, CG_PCL,  RS_10,  534, GT_OK},  /* was 530 *//* 534 % 6 = 0   fill the 12'th block (in holes) */

    {VT_CREATE,   50, CG_TTI,  RS_10,    1, GT_FULL},
    {VT_CREATE,   51, CG_PCL,  RS_10,    1, GT_FULL},
};

/* NOTE: the table is ordered
1.by 'client'
2.by rule size
3.by number of rules

    NOTE: the table is like in bc2_sequence_creation_OK
*/
static VT_SEQUENCE bc2_sequence_creation_order_error[] =
{
    {VT_CREATE,    6, CG_TTI,  RS_30, 1000, GT_OK}, /* 1000 % 2 = 0*/
    {VT_CREATE,    5, CG_TTI,  RS_20, 1002, GT_OK}, /* 1002 % 3 = 0*/
    {VT_CREATE,    7, CG_TTI,  RS_10, 1140, GT_OK},/* fill the second block */

    {VT_CREATE,   12, CG_PCL,  RS_60,  500, GT_OK}, /*  500 % 1 = 0*/
    {VT_CREATE,   10, CG_PCL,  RS_50, 1000, GT_OK}, /* 1000 % 1 = 0*//*free 1000 of 10B */
    {VT_CREATE,   14, CG_PCL,  RS_50,  814, GT_OK}, /*  814 % 1 = 0*//*free  814 of 10B */
    {VT_CREATE,    9, CG_PCL,  RS_40, 2000, GT_OK}, /* 2000 % 1 = 0*//*free 4000 of 10B */
    {VT_CREATE,   13, CG_PCL,  RS_30,  100, GT_OK}, /*  100 % 2 = 0*/
    {VT_CREATE,    8, CG_PCL,  RS_20, 1002, GT_OK}, /* 1002 % 3 = 0*/
    {VT_CREATE,   11, CG_PCL,  RS_10, 7002, GT_OK}, /* 7002 --> fill holes (1000+814+4000) = 5814 , (7002-5814)= 1188 % 6 = 0 !!! */
    {VT_CREATE,   15, CG_PCL,  RS_10,  814, GT_OK}, /* 814 % 12 = 10 --> take full rows with extra 2 */
    {VT_CREATE,   16, CG_PCL,  RS_10,  528, GT_OK}, /*was 530*//* --> the extra 2 not allow to succeed !!! to fill the 12'th block */

    {VT_CREATE,   50, CG_TTI,  RS_10,    1, GT_FULL},
    {VT_CREATE,   51, CG_PCL,  RS_10,    1, GT_FULL},
};

/*
    test to show error in adding 60 B that cause the CPSS to fail
    or to give more rules than asked for.
*/
UTF_TEST_CASE_MAC(cpssDxChVirtualTcam_bc2_sequence_creation_order_ok)
{
    GT_STATUS                                  st;
    GT_U8                                      dev;
    GT_U32                                     numOfDevs = 0;
    GT_U32                                     notAppFamilyBmp = 0;
    GT_U32                                     utfFamilyBit = 0;
    GT_BOOL                                    isTestSkipped = GT_TRUE;
    GT_U32                                     vTcamMngId;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC            vTcamInfo;
    VT_SEQUENCE                                *origSequencePtr;
    GT_U32                                     sequenceSize;
    GT_U32                                     idx;

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    notAppFamilyBmp |= UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E;
    notAppFamilyBmp |= UTF_FALCON_E;

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* 1. Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
            devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }
        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;


        origSequencePtr = bc2_sequence_creation_OK;
        sequenceSize = sizeof(bc2_sequence_creation_OK) / sizeof(bc2_sequence_creation_OK[0]);

        vTcamMngId = 1;
        cpssOsMemSet(&vTcamInfo, 0, sizeof(vTcamInfo));
        vTcamInfo.hitNumber     = 0;
        vTcamInfo.autoResize    = GT_FALSE;
        vTcamInfo.ruleAdditionMethod =
            priorityMode == GT_TRUE ?
                CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E:
                CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E;

        /* Create vTCAM manager */
        st = cpssDxChVirtualTcamManagerCreate(vTcamMngId);
            if (st == GT_ALREADY_EXIST)
                st = GT_OK;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerCreate failed\n");

        st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        if (st != GT_OK)
        {
            cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
            st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        }
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListAdd failed\n");

        for (idx = 0; (idx < sequenceSize); idx++)
        {
            /* do the actual adding of the vtcam to the TCAM manager */
            st = vtcamSequenceEntryProcess(origSequencePtr, idx, vTcamMngId, &vTcamInfo);
            UTF_VERIFY_NOT_EQUAL1_STRING_MAC(
                GT_FAIL, st,
                "vtcamSequenceEntryProcess failed, index: %d\n", idx);
        }

        st = cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st, "cpssDxChVirtualTcamManagerDevListRemove failed for vTcamMngId = %d\n",
            vTcamMngId);
        st = cpssDxChVirtualTcamManagerDelete(vTcamMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDelete failed for vTcamMngId = %d\n",
                                     vTcamMngId);

    }

    if (isTestSkipped)
    {
        SKIP_TEST_MAC;
    }
}

/*
    test to show error in adding 60 B that cause the CPSS to fail
    or to give more rules than asked for.
*/
UTF_TEST_CASE_MAC(cpssDxChVirtualTcam_bc2_sequence_creation_order_error)
{
    GT_STATUS                                  st;
    GT_U8                                      dev;
    GT_U32                                     numOfDevs = 0;
    GT_U32                                     notAppFamilyBmp = 0;
    GT_U32                                     utfFamilyBit = 0;
    GT_BOOL                                    isTestSkipped = GT_TRUE;
    GT_U32                                     vTcamMngId;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC            vTcamInfo;
    VT_SEQUENCE                                *origSequencePtr;
    GT_U32                                     sequenceSize;
    GT_U32                                     idx;

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    notAppFamilyBmp |= UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E;
    notAppFamilyBmp |= UTF_FALCON_E;

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* 1. Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
            devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }
        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;

        origSequencePtr = bc2_sequence_creation_order_error;
        sequenceSize = sizeof(bc2_sequence_creation_order_error) / sizeof(bc2_sequence_creation_order_error[0]);

        vTcamMngId = 1;
        cpssOsMemSet(&vTcamInfo, 0, sizeof(vTcamInfo));
        vTcamInfo.hitNumber     = 0;
        vTcamInfo.autoResize    = GT_FALSE;
        vTcamInfo.ruleAdditionMethod =
            priorityMode == GT_TRUE ?
                CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E:
                CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E;

        /* Create vTCAM manager */
        st = cpssDxChVirtualTcamManagerCreate(vTcamMngId);
            if (st == GT_ALREADY_EXIST)
                st = GT_OK;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerCreate failed\n");

        st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        if (st != GT_OK)
        {
            cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
            st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        }
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListAdd failed\n");

        for (idx = 0; (idx < sequenceSize); idx++)
        {
            /* do the actual adding of the vtcam to the TCAM manager */
            st = vtcamSequenceEntryProcess(origSequencePtr, idx, vTcamMngId, &vTcamInfo);

            if(st == GT_FAIL)
            {
                /* explain that there are 530 places in the table*/
                calcTotal10BRulesInAllVtcams(vTcamMngId,NULL);
            }

            UTF_VERIFY_NOT_EQUAL1_STRING_MAC(
                GT_FAIL, st,
                "vtcamSequenceEntryProcess failed, index: %d\n", idx);
        }

        st = cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st, "cpssDxChVirtualTcamManagerDevListRemove failed for vTcamMngId = %d\n",
            vTcamMngId);
        st = cpssDxChVirtualTcamManagerDelete(vTcamMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDelete failed for vTcamMngId = %d\n",
                                     vTcamMngId);

    }

    if (isTestSkipped)
    {
        SKIP_TEST_MAC;
    }
}
/* Create vTCAM manager */
/* make sure no device in the tcam from previous test */
/* add the first device */
/* create vtcams and fill them with rules */
/* add the other devices to the TCAM manager */
/* check that the new devices hold the Rules in the HW */
/* --- cleaning config --- */
/* delete all the rules */
/* remove the vtcam */
/* delete all devices from the tcam from previous test */
/* Delete vTCAM manager */
UTF_TEST_CASE_MAC(cpssDxChVirtualTcam_DevListAdd_toExistingDeviceWithRules)
{
    GT_STATUS                                  st;
    GT_U8                                      dev;
    GT_U32                                     numOfDevs = 0;
    GT_U32                                     notAppFamilyBmp = 0;
    GT_U32                                     utfFamilyBit = 0;
    GT_BOOL                                    isTestSkipped = GT_TRUE;
    GT_U32                                     vTcamMngId;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC            vTcamInfo;
    VT_SEQUENCE                                *sequencePtr;
    VT_SEQUENCE                                *origSequencePtr;
    GT_U32                                     sequenceSize;
    GT_U32                                     idx;
    GT_U8                                       single_devListArr[1];
    GT_U32                                      ii;
    GT_U8                                       currDevNum;

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* this feature is on eArch devices, xCat3 and Lion2 */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    notAppFamilyBmp &= (~UTF_XCAT3_E) & (~UTF_LION2_E);

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);


        /* 1. Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
            devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs < 2)
        {
            /* the test is only for multi devices */
            continue;
        }
        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;

        single_devListArr[0] = devListArr[0];


        switch (utfFamilyBit)
        {
            case UTF_XCAT3_E:
                origSequencePtr = xcat3_sequence;
                sequenceSize = sizeof(xcat3_sequence) / sizeof(xcat3_sequence[0]);
                break;

            case UTF_LION2_E:
                origSequencePtr = lion2_sequence;
                sequenceSize = sizeof(lion2_sequence) / sizeof(lion2_sequence[0]);
                break;

            case UTF_ALDRIN2_E:
            case UTF_FALCON_E:
            case UTF_CAELUM_E:
                origSequencePtr = bobk_sequence;
                sequenceSize = sizeof(bobk_sequence) / sizeof(bobk_sequence[0]);
                break;

            case UTF_ALDRIN_E:
            case UTF_AC3X_E:
                origSequencePtr = aldrin_sequence;
                sequenceSize = sizeof(aldrin_sequence) / sizeof(aldrin_sequence[0]);
                break;

            case UTF_BOBCAT2_E:
            case UTF_BOBCAT3_E:
            default:
                origSequencePtr = bc2_sequence;
                sequenceSize = sizeof(bc2_sequence) / sizeof(bc2_sequence[0]);
                break;
        }

        vTcamMngId = 1;
        cpssOsMemSet(&vTcamInfo, 0, sizeof(vTcamInfo));
        vTcamInfo.hitNumber     = 0;
        vTcamInfo.autoResize    = GT_FALSE;
        vTcamInfo.ruleAdditionMethod =
            priorityMode == GT_TRUE ?
                CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E:
                CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E;

        /* Create vTCAM manager */
        st = cpssDxChVirtualTcamManagerCreate(vTcamMngId);
        if (st == GT_ALREADY_EXIST)
            st = GT_OK;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerCreate failed\n");

        /* make sure no device in the tcam from previous test */
        (void)cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);

        /* add the first device */
        st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,single_devListArr,1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListAdd failed\n");

        clearVtcamBmp();

        /* create vtcams and fill them with rules */
        sequencePtr = origSequencePtr;
        for (idx = 0; (idx < sequenceSize); idx++,sequencePtr++)
        {
            if(sequencePtr->rc == GT_FULL)
            {
                break;
            }
            st = vtcamSequenceEntryProcess(origSequencePtr, idx, vTcamMngId, &vTcamInfo);
            UTF_VERIFY_NOT_EQUAL1_STRING_MAC(
                GT_FAIL, st,
                "vtcamSequenceEntryProcess failed, index: %d\n", idx);

            if(sequencePtr->rc == GT_OK)
            {
                if(sequencePtr->clientGroup == CG_PCL)
                {
                    fillVtcamWithRules(vTcamMngId,sequencePtr->vTcamId , 50/* max rules per vtcam */);
                }
                else
                {
                    fillVtcamWithTtiRules(vTcamMngId,sequencePtr->vTcamId , 50/* max rules per vtcam */);
                }
            }
        }

        /* add the other devices to the TCAM manager */
        st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,&devListArr[1],numOfDevs-1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListAdd failed\n");

        /* check that the new devices hold the Rules in the HW */
        for(ii = 1 ; ii < numOfDevs ; ii++)
        {
            currDevNum = devListArr[ii];

            sequencePtr = origSequencePtr;

            for (idx = 0; (idx < sequenceSize); idx++,sequencePtr++)
            {
                if(sequencePtr->rc == GT_FULL)
                {
                    break;
                }

                if(sequencePtr->rc == GT_OK)
                {
                    checkHwContentAllRules_fromCpssSpecificDevNum(currDevNum,vTcamMngId,sequencePtr->vTcamId);
                }
            }
        }


        /* delete all the rules */
        sequencePtr = origSequencePtr;
        for (idx = 0; (idx < sequenceSize); idx++,sequencePtr++)
        {
            if(sequencePtr->rc == GT_FULL)
            {
                break;
            }

            if(sequencePtr->rc == GT_OK)
            {
                vTcamDeleteConsecutiveRules(vTcamMngId,
                    sequencePtr->vTcamId,
                    CPSS_DXCH_VIRTUAL_TCAM_START_RULE_ID_CNS,
                    DELETE_ALL_CNS);

                /* remove the vtcam */
                ut_cpssDxChVirtualTcamRemove(vTcamMngId,sequencePtr->vTcamId);
            }
        }

        st = cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st, "cpssDxChVirtualTcamManagerDevListRemove failed for vTcamMngId = %d\n",
            vTcamMngId);
        st = cpssDxChVirtualTcamManagerDelete(vTcamMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDelete failed for vTcamMngId = %d\n",
                                     vTcamMngId);

    }

    if (isTestSkipped)
    {
        SKIP_TEST_MAC;
    }
}

UTF_TEST_CASE_MAC(cpssDxChVirtualTcam_DevListAdd_toExistingDeviceWithRules_priorityMode)
{
    priorityMode = GT_TRUE;
    UTF_TEST_CALL_MAC(cpssDxChVirtualTcam_DevListAdd_toExistingDeviceWithRules);
    priorityMode = GT_FALSE;
}

UTF_TEST_CASE_MAC(cpssDxChVirtualTcam_rule80_space)
{
    GT_STATUS                                  st   = GT_OK;
    GT_U8                                      dev;
    GT_U32                                     numOfDevs = 0;
    GT_U32                                     notAppFamilyBmp = 0;
    GT_U32                                     utfFamilyBit = 0;
    GT_BOOL                                    isTestSkipped = GT_TRUE;
    GT_U32                                     vTcamMngId;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC            vTcamInfo;
    GT_U32                                     sequenceSize;
    GT_U32                                     idx;
    GT_U32                                     totalRulesAllowed;

    static VT_SEQUENCE sequence[] =
    {
        {VT_TITLE,       0, CG_NONE, RS_NONE,    0, GT_OK, "Create 15 (120 rules of 80 bytes"},
        {VT_CREATE,     15, CG_PCL,  RS_80,    120, GT_OK},
        {VT_STAMP_FILL, 15, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_STAMP_CHECK,15, CG_NONE, RS_NONE,    0, GT_OK},

        {VT_TITLE,       0, CG_NONE, RS_NONE,    0, GT_OK, "Create 16 (272 rules of 60 bytes"},
        {VT_CREATE,     16, CG_PCL,  RS_60,    272, GT_OK}, /* 512 - (2 * 120) */
        {VT_STAMP_FILL, 16, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_STAMP_CHECK,16, CG_NONE, RS_NONE,    0, GT_OK},

        {VT_TITLE,       0, CG_NONE, RS_NONE,    0, GT_OK, "Create and remove 1 (480 rules of 10 bytes"},
        {VT_CREATE,      1, CG_PCL,  RS_10,    480, GT_OK},
        {VT_STAMP_FILL,  1, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_STAMP_CHECK, 1, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      1, CG_NONE, RS_NONE,    0, GT_OK},

        {VT_TITLE,       0, CG_NONE, RS_NONE,    0, GT_OK, "Create and remove 2 (240 rules of 20 bytes"},
        {VT_CREATE,      2, CG_PCL,  RS_20,    240, GT_OK},
        {VT_STAMP_FILL,  2, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_STAMP_CHECK, 2, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      2, CG_NONE, RS_NONE,    0, GT_OK},

        {VT_TITLE,       0, CG_NONE, RS_NONE,    0, GT_OK, "Create and remove 3 (120 rules of 30 bytes"},
        {VT_CREATE,      3, CG_PCL,  RS_30,    120, GT_OK},
        {VT_STAMP_FILL,  3, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_STAMP_CHECK, 3, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      3, CG_NONE, RS_NONE,    0, GT_OK},

        {VT_TITLE,       0, CG_NONE, RS_NONE,    0, GT_OK, "insert 24 80-rules"},
        {VT_REMOVE,     16, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_INCREASE,   15, CG_NONE, RS_NONE,    24, GT_OK},
        {VT_STAMP_RANGE_CHECK,   15, CG_NONE,RS_NONE,0,GT_OK,
            "ruleIdBase = 24, ruleStampBase = 0, rulesAmount = 120"},

        {VT_TITLE,       0, CG_NONE, RS_NONE,    0, GT_OK, "remove 36 80-rules"},
        {VT_STAMP_FILL, 15, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_STAMP_CHECK,15, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_DECREASE,   15, CG_NONE, RS_NONE,    36, GT_OK},
        {VT_STAMP_RANGE_CHECK,   15, CG_NONE,RS_NONE,0,GT_OK,
            "ruleIdBase = 0, ruleStampBase = 36, rulesAmount = 108"},

        {VT_TITLE,       0,CG_NONE,RS_NONE,0,GT_OK, "alloc, decrease, increase 30-rules near 80-rules"},
        {VT_INCREASE,   15, CG_NONE, RS_NONE,    12, GT_OK},
        {VT_CREATE,      3, CG_PCL,  RS_30,    240, GT_OK},
        {VT_STAMP_FILL,  3, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_STAMP_CHECK, 3, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_DECREASE,   3, CG_NONE, RS_NONE,    40, GT_OK},
        {VT_STAMP_RANGE_CHECK,   3, CG_NONE,RS_NONE,0,GT_OK,
            "ruleIdBase = 0, ruleStampBase = 40, rulesAmount = 200"},
        {VT_INCREASE,   3, CG_NONE, RS_NONE,    20, GT_OK},
        {VT_STAMP_RANGE_CHECK,   3, CG_NONE,RS_NONE,0,GT_OK,
            "ruleIdBase = 20, ruleStampBase = 40, rulesAmount = 200"},
        {VT_REMOVE,     3, CG_NONE, RS_NONE,     0, GT_OK},

        {VT_TITLE,       0, CG_NONE, RS_NONE,    0, GT_OK, "Remove all"},
        {VT_REMOVE,     15, CG_NONE, RS_NONE,    0, GT_OK},
    };

    /* skip any case, prevent compiler warnings */
     /*numOfDevs = 0; if (numOfDevs == 0) SKIP_TEST_MAC;*/

    sequenceSize = (sizeof(sequence) / sizeof(sequence[0]));

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* 1. Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
              devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }
        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;

        vTcamMngId = 1;
        cpssOsMemSet(&vTcamInfo, 0, sizeof(vTcamInfo));
        vTcamInfo.hitNumber     = 0;
        vTcamInfo.autoResize    = GT_FALSE;
        vTcamInfo.ruleAdditionMethod =
            CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E;

        /* Create vTCAM manager */
        st = cpssDxChVirtualTcamManagerCreate(vTcamMngId);
            if (st == GT_ALREADY_EXIST)
                st = GT_OK;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerCreate failed\n");

        st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        if (st != GT_OK)
        {
            cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
            st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        }
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListAdd failed\n");

        /* create vTcam 100 occupying all blocks beside two to TTI */
        vTcamInfo.clientGroup = CG_TTI;
        vTcamInfo.ruleSize    = RS_10;
        internal_ut_cpssDxChVirtualTcamTotalRulesAllowedPerDevGet(
                    devListArr[0],
                    vTcamInfo.ruleSize,
                    &totalRulesAllowed);
        vTcamInfo.guaranteedNumOfRules = totalRulesAllowed - (2 * NUM_60_RULES_PER_BLOCK * 6);
        st = ut_cpssDxChVirtualTcamCreate(vTcamMngId, 100 /*vTcamId*/, &vTcamInfo);
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC(
            GT_FAIL, st, "ut_cpssDxChVirtualTcamCreate failed\n");

        for (idx = 0; (idx < sequenceSize); idx++)
        {
            st = vtcamSequenceEntryProcess(sequence, idx, vTcamMngId, &vTcamInfo);
            if (sequence[idx].operation == VT_CREATE)
            {
                PRV_UTF_LOG4_MAC(
                    "Created vTcam %d ruleSize %d rulesAmount %d clientGroup %d\n",
                    sequence[idx].vTcamId, (sequence[idx].ruleSize + 1),
                    sequence[idx].rulesAmount, sequence[idx].clientGroup);
            }
            if (sequence[idx].operation == VT_REMOVE)
            {
                PRV_UTF_LOG1_MAC("Removed vTcam %d\n", sequence[idx].vTcamId);
            }
            if (st != GT_OK) PRV_UTF_LOG0_MAC("Operation Failed\n");

            UTF_VERIFY_EQUAL1_STRING_MAC(
                sequence[idx].rc, st,
                "vtcamSequenceEntryProcess failed, index: %d\n", idx);
        }

        st = cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st, "cpssDxChVirtualTcamManagerDevListRemove failed for vTcamMngId = %d\n",
            vTcamMngId);
        st = cpssDxChVirtualTcamManagerDelete(vTcamMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDelete failed for vTcamMngId = %d\n",
                                     vTcamMngId);
    }

    if (isTestSkipped)
    {
        SKIP_TEST_MAC;
    }
}

UTF_TEST_CASE_MAC(cpssDxChVirtualTcam_rule80_xCat3_space)
{
    GT_STATUS                                  st   = GT_OK;
    GT_U8                                      dev;
    GT_U32                                     numOfDevs = 0;
    GT_U32                                     vTcamMngId;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC            vTcamInfo;
    GT_U32                                     sequenceSize;
    GT_U32                                     idx;
    GT_U32                                     tcamRowsNum;

    static VT_SEQUENCE sequence[] =
    {
        {VT_TITLE,       0, CG_NONE, RS_NONE,    0, GT_OK, "Create 15 (120 rules of 80 bytes"},
        {VT_CREATE,     15, CG_PCL,  RS_80,    120, GT_OK},
        {VT_STAMP_FILL, 15, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_STAMP_CHECK,15, CG_NONE, RS_NONE,    0, GT_OK},

        {VT_TITLE,       0, CG_NONE, RS_NONE,    0, GT_OK, "Create 16 (272 rules of 60 bytes"},
        {VT_CREATE,     16, CG_PCL,  RS_60,    272, GT_OK}, /* 512 - (2 * 120) */
        {VT_STAMP_FILL, 16, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_STAMP_CHECK,16, CG_NONE, RS_NONE,    0, GT_OK},

        {VT_TITLE,       0, CG_NONE, RS_NONE,    0, GT_OK, "Create and remove 3 (120 rules of 30 bytes"},
        {VT_CREATE,      3, CG_PCL,  RS_30,    120, GT_OK},
        {VT_STAMP_FILL,  3, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_STAMP_CHECK, 3, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_REMOVE,      3, CG_NONE, RS_NONE,    0, GT_OK},

        {VT_TITLE,       0, CG_NONE, RS_NONE,    0, GT_OK, "insert 24 80-rules"},
        {VT_REMOVE,     16, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_INCREASE,   15, CG_NONE, RS_NONE,    24, GT_OK},
        {VT_STAMP_RANGE_CHECK,   15, CG_NONE,RS_NONE,0,GT_OK,
            "ruleIdBase = 24, ruleStampBase = 0, rulesAmount = 120"},

        {VT_TITLE,       0, CG_NONE, RS_NONE,    0, GT_OK, "remove 36 80-rules"},
        {VT_STAMP_FILL, 15, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_STAMP_CHECK,15, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_DECREASE,   15, CG_NONE, RS_NONE,    36, GT_OK},
        {VT_STAMP_RANGE_CHECK,   15, CG_NONE,RS_NONE,0,GT_OK,
            "ruleIdBase = 0, ruleStampBase = 36, rulesAmount = 108"},

        {VT_TITLE,       0,CG_NONE,RS_NONE,0,GT_OK, "alloc, decrease, increase 30-rules near 80-rules"},
        {VT_INCREASE,   15, CG_NONE, RS_NONE,    12, GT_OK},
        {VT_CREATE,      3, CG_PCL,  RS_30,    240, GT_OK},
        {VT_STAMP_FILL,  3, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_STAMP_CHECK, 3, CG_NONE, RS_NONE,    0, GT_OK},
        {VT_DECREASE,   3, CG_NONE, RS_NONE,    40, GT_OK},
        {VT_STAMP_RANGE_CHECK,   3, CG_NONE,RS_NONE,0,GT_OK,
            "ruleIdBase = 0, ruleStampBase = 40, rulesAmount = 200"},
        {VT_INCREASE,   3, CG_NONE, RS_NONE,    20, GT_OK},
        {VT_STAMP_RANGE_CHECK,   3, CG_NONE,RS_NONE,0,GT_OK,
            "ruleIdBase = 20, ruleStampBase = 40, rulesAmount = 200"},
        {VT_REMOVE,     3, CG_NONE, RS_NONE,     0, GT_OK},

        {VT_TITLE,       0, CG_NONE, RS_NONE,    0, GT_OK, "Remove all"},
        {VT_REMOVE,     15, CG_NONE, RS_NONE,    0, GT_OK},
    };

    /* skip any case, prevent compiler warnings */
     /*numOfDevs = 0; if (numOfDevs == 0) SKIP_TEST_MAC;*/

    sequenceSize = (sizeof(sequence) / sizeof(sequence[0]));

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* prepare iterator for go over xCat3 devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, (~UTF_XCAT3_E) & (~UTF_LION2_E));


    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
          devListArr[numOfDevs++] = dev;
    }

    if (numOfDevs == 0)
    {
        SKIP_TEST_MAC;
    }

    tcamRowsNum = PRV_CPSS_DXCH_PP_MAC(devListArr[0])->fineTuning.tableSize.policyTcamRaws;

    vTcamMngId = 1;

    /* Create vTCAM manager */
    st = cpssDxChVirtualTcamManagerCreate(vTcamMngId);
        if (st == GT_ALREADY_EXIST)
            st = GT_OK;
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerCreate failed\n");

    st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
    if (st != GT_OK)
    {
        cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
        st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
    }
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListAdd failed\n");

    /* create vTcam 100 occupying all TCAM beside 256 rows (4 * STD Rule) */
    cpssOsMemSet(&vTcamInfo, 0, sizeof(vTcamInfo));
    vTcamInfo.clientGroup = CG_PCL;
    vTcamInfo.ruleSize    = RS_30;
    vTcamInfo.hitNumber     = 0;
    vTcamInfo.autoResize    = GT_FALSE;
    vTcamInfo.ruleAdditionMethod =
        CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E;
    vTcamInfo.guaranteedNumOfRules = ((tcamRowsNum - 256) * 4);

    st = ut_cpssDxChVirtualTcamCreate(vTcamMngId, 100 /*vTcamId*/, &vTcamInfo);
    UTF_VERIFY_NOT_EQUAL0_STRING_MAC(
        GT_FAIL, st, "ut_cpssDxChVirtualTcamCreate failed\n");

    for (idx = 0; (idx < sequenceSize); idx++)
    {
        st = vtcamSequenceEntryProcess(sequence, idx, vTcamMngId, &vTcamInfo);
        if (sequence[idx].operation == VT_CREATE)
        {
            PRV_UTF_LOG4_MAC(
                "Created vTcam %d ruleSize %d rulesAmount %d clientGroup %d\n",
                sequence[idx].vTcamId, (sequence[idx].ruleSize + 1),
                sequence[idx].rulesAmount, sequence[idx].clientGroup);
        }
        if (sequence[idx].operation == VT_REMOVE)
        {
            PRV_UTF_LOG1_MAC("Removed vTcam %d\n", sequence[idx].vTcamId);
        }
        if (st != GT_OK) PRV_UTF_LOG0_MAC("Operation Failed\n");

        UTF_VERIFY_EQUAL1_STRING_MAC(
            sequence[idx].rc, st,
            "vtcamSequenceEntryProcess failed, index: %d\n", idx);
    }

    st = cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, st, "cpssDxChVirtualTcamManagerDevListRemove failed for vTcamMngId = %d\n",
        vTcamMngId);
    st = cpssDxChVirtualTcamManagerDelete(vTcamMngId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDelete failed for vTcamMngId = %d\n",
                                 vTcamMngId);
}

UTF_TEST_CASE_MAC(cpssDxChVirtualTcam_priority_autoresize_full_tcam)
{
    GT_STATUS                                  st   = GT_OK;
    GT_U8                                      dev;
    GT_U32                                     numOfDevs = 0;
    GT_U32                                     notAppFamilyBmp = 0;
    GT_U32                                     utfFamilyBit = 0;
    GT_BOOL                                    isTestSkipped = GT_TRUE;
    GT_U32          startSeconds;
    GT_U32          startNanoSeconds;
    GT_U32          stopSeconds;
    GT_U32          stopNanoSeconds;
    GT_U32          elapsedSeconds;
    GT_U32          elapsedNanoSeconds;
    GT_U32                                     vTcamMngId;
    GT_U32                                     vTcamId;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC            vTcamInfo;
    GT_U32                                     idx;
    GT_U32                                     totalRulesAllowed;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC ruleAttributes;
    CPSS_DXCH_VIRTUAL_TCAM_USAGE_STC           vTcamUsage;

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    if (prvRuleType == CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_TTI_E)
    {
        /* No need to repeat the test on TTI for eArch devices */
        notAppFamilyBmp = UTF_ALL_FAMILIES_SET_CNS;
    }
    else
    {
        /* this feature is on eArch devices */
        UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    }
    notAppFamilyBmp &= (~UTF_XCAT3_E) & (~UTF_LION2_E);

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* 1. Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
              devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }
        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;

        vTcamMngId = 1;
        vTcamId    = 100;
        cpssOsMemSet(&vTcamInfo, 0, sizeof(vTcamInfo));
        vTcamInfo.clientGroup   = 0;
        vTcamInfo.hitNumber     = 0;
        vTcamInfo.autoResize    = GT_TRUE;
        vTcamInfo.ruleAdditionMethod =
            CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E;
        vTcamInfo.guaranteedNumOfRules = 0;
        vTcamInfo.ruleSize = prvRuleSize;

        /* Create vTCAM manager */
        st = cpssDxChVirtualTcamManagerCreate(vTcamMngId);
            if (st == GT_ALREADY_EXIST)
                st = GT_OK;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerCreate failed\n");

        cpssDxChVirtualTcamAutoResizeGranularitySet(vTcamMngId,12);
        cpssOsPrintf("*** AutoResize Threshold set to 12 ***\n");

        st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        if (st != GT_OK)
        {
            cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
            st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        }
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListAdd failed\n");

        if (PRV_CPSS_SIP_5_CHECK_MAC(devListArr[0]) ||
            (prvRuleType == CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E))
        {
            internal_ut_cpssDxChVirtualTcamTotalRulesAllowedPerDevGet(
                               devListArr[0],
                               prvRuleSize,
                               &totalRulesAllowed);
        }
        else
        {
            totalRulesAllowed = PRV_CPSS_DXCH_PP_MAC(devListArr[0])->fineTuning.tableSize.tunnelTerm;
            vTcamInfo.clientGroup   = CG_TTI;
        }

        st = cpssDxChVirtualTcamCreate(vTcamMngId, vTcamId, &vTcamInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, st, "cpssDxChVirtualTcamCreate failed\n");

        cpssOsTimeRT(&startSeconds, &startNanoSeconds);
        for (idx = 0; (idx < totalRulesAllowed); idx++)
        {
            ruleAttributes.priority = idx;
            st = addRuleNoHitByIndexPriorityEx(
                vTcamMngId, vTcamId, idx /*ruleId*/, &ruleAttributes);
            UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, st, "addRuleNoHitByIndexPriorityEx failed for idx = %d\n",
                idx);
        }
        cpssOsTimeRT(&stopSeconds, &stopNanoSeconds);
        if (stopNanoSeconds >= startNanoSeconds)
        {
            elapsedNanoSeconds = (stopNanoSeconds - startNanoSeconds);
            elapsedSeconds = (stopSeconds - startSeconds);
        }
        else
        {
            elapsedNanoSeconds = ((1000*1000*1000) + stopNanoSeconds) - startNanoSeconds;
            elapsedSeconds = (stopSeconds - startSeconds) - 1;
        }
        cpssOsPrintf("Create forward time: [%d.%06d] secs ([%d] TCAM size) \n",
            elapsedSeconds, elapsedNanoSeconds / (1000),
            totalRulesAllowed);

        cpssOsTimeRT(&startSeconds, &startNanoSeconds);
        for (idx = 0; (idx < totalRulesAllowed); idx++)
        {
            st = cpssDxChVirtualTcamRuleDelete(
                vTcamMngId, vTcamId, idx /*ruleId*/);
            UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, st, "cpssDxChVirtualTcamRuleDelete failed for idx = %d\n",
                idx);
        }
        cpssOsTimeRT(&stopSeconds, &stopNanoSeconds);
        if (stopNanoSeconds >= startNanoSeconds)
        {
            elapsedNanoSeconds = (stopNanoSeconds - startNanoSeconds);
            elapsedSeconds = (stopSeconds - startSeconds);
        }
        else
        {
            elapsedNanoSeconds = ((1000*1000*1000) + stopNanoSeconds) - startNanoSeconds;
            elapsedSeconds = (stopSeconds - startSeconds) - 1;
        }
        cpssOsPrintf("Delete forward time: [%d.%06d] secs ([%d] TCAM size) \n",
            elapsedSeconds, elapsedNanoSeconds / (1000),
            totalRulesAllowed);

        /* get actual number of rules used+free in the vtcam */
        st = cpssDxChVirtualTcamUsageGet(vTcamMngId,vTcamId,&vTcamUsage);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamUsageGet failed\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, vTcamUsage.rulesFree, "vTcamUsage.rulesFree != 0\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, vTcamUsage.rulesUsed, "vTcamUsage.rulesUsed != 0\n");

        st = cpssDxChVirtualTcamRemove(
            vTcamMngId, vTcamId);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st, "cpssDxChVirtualTcamRemove failed for vTcamId = %d\n",
            vTcamId);

        st = cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st, "cpssDxChVirtualTcamManagerDevListRemove failed for vTcamMngId = %d\n",
            vTcamMngId);
        cpssDxChVirtualTcamAutoResizeGranularitySet(vTcamMngId,48);

        /* !!! From now on, all tests are done on 1024 entries only !!! */
        if (totalRulesAllowed > _1K)
        {
            /* Set it to 1024 in order to avoid long running time*/
            totalRulesAllowed = _1K + (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_DEFAULT_AUTO_RESIZE_GRANULARITY_CNS - 1);
            totalRulesAllowed -= totalRulesAllowed % PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_DEFAULT_AUTO_RESIZE_GRANULARITY_CNS;
        }
        /* !!! From now on, all tests are done on 1024 entries only !!! */

        st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListAdd failed\n");
        st = cpssDxChVirtualTcamCreate(vTcamMngId, vTcamId, &vTcamInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, st, "cpssDxChVirtualTcamCreate failed\n");

        cpssOsPrintf("*** AutoResize Threshold set to 48 ***\n");

        cpssOsTimeRT(&startSeconds, &startNanoSeconds);
        for (idx = 0; (idx < totalRulesAllowed); idx++)
        {
            ruleAttributes.priority = idx;
            st = addRuleNoHitByIndexPriorityEx(
                vTcamMngId, vTcamId, idx /*ruleId*/, &ruleAttributes);
            UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, st, "addRuleNoHitByIndexPriorityEx failed for idx = %d\n",
                idx);
        }
        cpssOsTimeRT(&stopSeconds, &stopNanoSeconds);
        if (stopNanoSeconds >= startNanoSeconds)
        {
            elapsedNanoSeconds = (stopNanoSeconds - startNanoSeconds);
            elapsedSeconds = (stopSeconds - startSeconds);
        }
        else
        {
            elapsedNanoSeconds = ((1000*1000*1000) + stopNanoSeconds) - startNanoSeconds;
            elapsedSeconds = (stopSeconds - startSeconds) - 1;
        }
        cpssOsPrintf("Create forward time: [%d.%06d] secs ([%d] TCAM size) \n",
            elapsedSeconds, elapsedNanoSeconds / (1000),
            totalRulesAllowed);

        cpssOsTimeRT(&startSeconds, &startNanoSeconds);
        for (idx = 0; (idx < totalRulesAllowed); idx++)
        {
            st = cpssDxChVirtualTcamRuleDelete(
                vTcamMngId, vTcamId, idx /*ruleId*/);
            UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, st, "cpssDxChVirtualTcamRuleDelete failed for idx = %d\n",
                idx);
        }
        cpssOsTimeRT(&stopSeconds, &stopNanoSeconds);
        if (stopNanoSeconds >= startNanoSeconds)
        {
            elapsedNanoSeconds = (stopNanoSeconds - startNanoSeconds);
            elapsedSeconds = (stopSeconds - startSeconds);
        }
        else
        {
            elapsedNanoSeconds = ((1000*1000*1000) + stopNanoSeconds) - startNanoSeconds;
            elapsedSeconds = (stopSeconds - startSeconds) - 1;
        }
        cpssOsPrintf("Delete forward time: [%d.%06d] secs ([%d] TCAM size) \n",
            elapsedSeconds, elapsedNanoSeconds / (1000),
            totalRulesAllowed);

        /* get actual number of rules used+free in the vtcam */
        st = cpssDxChVirtualTcamUsageGet(vTcamMngId,vTcamId,&vTcamUsage);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamUsageGet failed\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, vTcamUsage.rulesFree, "vTcamUsage.rulesFree != 0\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, vTcamUsage.rulesUsed, "vTcamUsage.rulesUsed != 0\n");

        st = cpssDxChVirtualTcamRemove(
            vTcamMngId, vTcamId);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st, "cpssDxChVirtualTcamRemove failed for vTcamId = %d\n",
            vTcamId);

        st = cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st, "cpssDxChVirtualTcamManagerDevListRemove failed for vTcamMngId = %d\n",
            vTcamMngId);
        cpssDxChVirtualTcamAutoResizeGranularitySet(vTcamMngId,144);
        st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListAdd failed\n");
        st = cpssDxChVirtualTcamCreate(vTcamMngId, vTcamId, &vTcamInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, st, "cpssDxChVirtualTcamCreate failed\n");

        cpssOsPrintf("*** AutoResize Threshold set to 144 ***\n");

        cpssOsTimeRT(&startSeconds, &startNanoSeconds);
        for (idx = 0; (idx < totalRulesAllowed); idx++)
        {
            ruleAttributes.priority = idx;
            st = addRuleNoHitByIndexPriorityEx(
                vTcamMngId, vTcamId, idx /*ruleId*/, &ruleAttributes);
            UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, st, "addRuleNoHitByIndexPriorityEx failed for idx = %d\n",
                idx);
        }
        cpssOsTimeRT(&stopSeconds, &stopNanoSeconds);
        if (stopNanoSeconds >= startNanoSeconds)
        {
            elapsedNanoSeconds = (stopNanoSeconds - startNanoSeconds);
            elapsedSeconds = (stopSeconds - startSeconds);
        }
        else
        {
            elapsedNanoSeconds = ((1000*1000*1000) + stopNanoSeconds) - startNanoSeconds;
            elapsedSeconds = (stopSeconds - startSeconds) - 1;
        }
        cpssOsPrintf("Create forward time: [%d.%06d] secs ([%d] TCAM size) \n",
            elapsedSeconds, elapsedNanoSeconds / (1000),
            totalRulesAllowed);

        cpssOsTimeRT(&startSeconds, &startNanoSeconds);
        for (idx = 0; (idx < totalRulesAllowed); idx++)
        {
            st = cpssDxChVirtualTcamRuleDelete(
                vTcamMngId, vTcamId, idx /*ruleId*/);
            UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, st, "cpssDxChVirtualTcamRuleDelete failed for idx = %d\n",
                idx);
        }
        cpssOsTimeRT(&stopSeconds, &stopNanoSeconds);
        if (stopNanoSeconds >= startNanoSeconds)
        {
            elapsedNanoSeconds = (stopNanoSeconds - startNanoSeconds);
            elapsedSeconds = (stopSeconds - startSeconds);
        }
        else
        {
            elapsedNanoSeconds = ((1000*1000*1000) + stopNanoSeconds) - startNanoSeconds;
            elapsedSeconds = (stopSeconds - startSeconds) - 1;
        }
        cpssOsPrintf("Delete forward time: [%d.%06d] secs ([%d] TCAM size) \n",
            elapsedSeconds, elapsedNanoSeconds / (1000),
            totalRulesAllowed);

        /* get actual number of rules used+free in the vtcam */
        st = cpssDxChVirtualTcamUsageGet(vTcamMngId,vTcamId,&vTcamUsage);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamUsageGet failed\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, vTcamUsage.rulesFree, "vTcamUsage.rulesFree != 0\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, vTcamUsage.rulesUsed, "vTcamUsage.rulesUsed != 0\n");

        st = cpssDxChVirtualTcamRemove(
            vTcamMngId, vTcamId);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st, "cpssDxChVirtualTcamRemove failed for vTcamId = %d\n",
            vTcamId);

        st = cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st, "cpssDxChVirtualTcamManagerDevListRemove failed for vTcamMngId = %d\n",
            vTcamMngId);
        cpssDxChVirtualTcamAutoResizeGranularitySet(vTcamMngId,240);
        st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListAdd failed\n");
        st = cpssDxChVirtualTcamCreate(vTcamMngId, vTcamId, &vTcamInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, st, "cpssDxChVirtualTcamCreate failed\n");

        cpssOsPrintf("*** AutoResize Threshold set to 240 ***\n");

        cpssOsTimeRT(&startSeconds, &startNanoSeconds);
        for (idx = 0; (idx < totalRulesAllowed); idx++)
        {
            ruleAttributes.priority = idx;
            st = addRuleNoHitByIndexPriorityEx(
                vTcamMngId, vTcamId, idx /*ruleId*/, &ruleAttributes);
            UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, st, "addRuleNoHitByIndexPriorityEx failed for idx = %d\n",
                idx);
        }
        cpssOsTimeRT(&stopSeconds, &stopNanoSeconds);
        if (stopNanoSeconds >= startNanoSeconds)
        {
            elapsedNanoSeconds = (stopNanoSeconds - startNanoSeconds);
            elapsedSeconds = (stopSeconds - startSeconds);
        }
        else
        {
            elapsedNanoSeconds = ((1000*1000*1000) + stopNanoSeconds) - startNanoSeconds;
            elapsedSeconds = (stopSeconds - startSeconds) - 1;
        }
        cpssOsPrintf("Create forward time: [%d.%06d] secs ([%d] TCAM size) \n",
            elapsedSeconds, elapsedNanoSeconds / (1000),
            totalRulesAllowed);

        cpssOsTimeRT(&startSeconds, &startNanoSeconds);
        for (idx = 0; (idx < totalRulesAllowed); idx++)
        {
            st = cpssDxChVirtualTcamRuleDelete(
                vTcamMngId, vTcamId, idx /*ruleId*/);
            UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, st, "cpssDxChVirtualTcamRuleDelete failed for idx = %d\n",
                idx);
        }
        cpssOsTimeRT(&stopSeconds, &stopNanoSeconds);
        if (stopNanoSeconds >= startNanoSeconds)
        {
            elapsedNanoSeconds = (stopNanoSeconds - startNanoSeconds);
            elapsedSeconds = (stopSeconds - startSeconds);
        }
        else
        {
            elapsedNanoSeconds = ((1000*1000*1000) + stopNanoSeconds) - startNanoSeconds;
            elapsedSeconds = (stopSeconds - startSeconds) - 1;
        }
        cpssOsPrintf("Delete forward time: [%d.%06d] secs ([%d] TCAM size) \n",
            elapsedSeconds, elapsedNanoSeconds / (1000),
            totalRulesAllowed);

        /* get actual number of rules used+free in the vtcam */
        st = cpssDxChVirtualTcamUsageGet(vTcamMngId,vTcamId,&vTcamUsage);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamUsageGet failed\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, vTcamUsage.rulesFree, "vTcamUsage.rulesFree != 0\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, vTcamUsage.rulesUsed, "vTcamUsage.rulesUsed != 0\n");

        st = cpssDxChVirtualTcamRemove(
            vTcamMngId, vTcamId);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st, "cpssDxChVirtualTcamRemove failed for vTcamId = %d\n",
            vTcamId);

        st = cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st, "cpssDxChVirtualTcamManagerDevListRemove failed for vTcamMngId = %d\n",
            vTcamMngId);
        cpssDxChVirtualTcamAutoResizeGranularitySet(vTcamMngId,12);
        st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListAdd failed\n");
        st = cpssDxChVirtualTcamCreate(vTcamMngId, vTcamId, &vTcamInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, st, "cpssDxChVirtualTcamCreate failed\n");

        cpssOsPrintf("*** AutoResize Threshold set to 12 ***\n");

        cpssOsTimeRT(&startSeconds, &startNanoSeconds);
        for (idx = totalRulesAllowed-1; (idx < totalRulesAllowed); idx--)
        {
            ruleAttributes.priority = idx;
            st = addRuleNoHitByIndexPriorityEx(
                vTcamMngId, vTcamId, idx /*ruleId*/, &ruleAttributes);
            UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, st, "addRuleNoHitByIndexPriorityEx failed for idx = %d\n",
                idx);
        }
        cpssOsTimeRT(&stopSeconds, &stopNanoSeconds);
        if (stopNanoSeconds >= startNanoSeconds)
        {
            elapsedNanoSeconds = (stopNanoSeconds - startNanoSeconds);
            elapsedSeconds = (stopSeconds - startSeconds);
        }
        else
        {
            elapsedNanoSeconds = ((1000*1000*1000) + stopNanoSeconds) - startNanoSeconds;
            elapsedSeconds = (stopSeconds - startSeconds) - 1;
        }
        cpssOsPrintf("Create backwards time: [%d.%06d] secs ([%d] TCAM size) \n",
            elapsedSeconds, elapsedNanoSeconds / (1000),
            totalRulesAllowed);

        cpssOsTimeRT(&startSeconds, &startNanoSeconds);
        for (idx = totalRulesAllowed-1; idx < totalRulesAllowed; idx--)
        {
            st = cpssDxChVirtualTcamRuleDelete(
                vTcamMngId, vTcamId, idx /*ruleId*/);
            UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, st, "cpssDxChVirtualTcamRuleDelete failed for idx = %d\n",
                idx);
        }
        cpssOsTimeRT(&stopSeconds, &stopNanoSeconds);
        if (stopNanoSeconds >= startNanoSeconds)
        {
            elapsedNanoSeconds = (stopNanoSeconds - startNanoSeconds);
            elapsedSeconds = (stopSeconds - startSeconds);
        }
        else
        {
            elapsedNanoSeconds = ((1000*1000*1000) + stopNanoSeconds) - startNanoSeconds;
            elapsedSeconds = (stopSeconds - startSeconds) - 1;
        }
        cpssOsPrintf("Delete backwards time: [%d.%06d] secs ([%d] TCAM size) \n",
            elapsedSeconds, elapsedNanoSeconds / (1000),
            totalRulesAllowed);

        /* get actual number of rules used+free in the vtcam */
        st = cpssDxChVirtualTcamUsageGet(vTcamMngId,vTcamId,&vTcamUsage);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamUsageGet failed\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, vTcamUsage.rulesFree, "vTcamUsage.rulesFree != 0\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, vTcamUsage.rulesUsed, "vTcamUsage.rulesUsed != 0\n");

        cpssOsTimeRT(&startSeconds, &startNanoSeconds);
        for (idx = 0; (idx < totalRulesAllowed); idx++)
        {
            ruleAttributes.priority = idx;
            st = addRuleNoHitByIndexPriorityEx(
                vTcamMngId, vTcamId, idx /*ruleId*/, &ruleAttributes);
            UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, st, "addRuleNoHitByIndexPriorityEx failed for idx = %d\n",
                idx);
        }
        cpssOsTimeRT(&stopSeconds, &stopNanoSeconds);
        if (stopNanoSeconds >= startNanoSeconds)
        {
            elapsedNanoSeconds = (stopNanoSeconds - startNanoSeconds);
            elapsedSeconds = (stopSeconds - startSeconds);
        }
        else
        {
            elapsedNanoSeconds = ((1000*1000*1000) + stopNanoSeconds) - startNanoSeconds;
            elapsedSeconds = (stopSeconds - startSeconds) - 1;
        }
        cpssOsPrintf("Create forward time: [%d.%06d] secs ([%d] TCAM size) \n",
            elapsedSeconds, elapsedNanoSeconds / (1000),
            totalRulesAllowed);

        cpssDxChVirtualTcamAutoResizeEnable(vTcamMngId,vTcamId,GT_FALSE);
        cpssOsPrintf("*** AutoResize Enable set to FALSE ***\n");

        cpssOsTimeRT(&startSeconds, &startNanoSeconds);
        for (idx = 0; (idx < totalRulesAllowed); idx++)
        {
            st = cpssDxChVirtualTcamRuleDelete(
                vTcamMngId, vTcamId, idx /*ruleId*/);
            UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, st, "cpssDxChVirtualTcamRuleDelete failed for idx = %d\n",
                idx);
        }

        /* get actual number of rules used+free in the vtcam */
        st = cpssDxChVirtualTcamUsageGet(vTcamMngId,vTcamId,&vTcamUsage);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamUsageGet failed\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(totalRulesAllowed, vTcamUsage.rulesFree, "vTcamUsage.rulesFree != 0\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, vTcamUsage.rulesUsed, "vTcamUsage.rulesUsed != 0\n");

        cpssDxChVirtualTcamAutoResizeEnable(vTcamMngId,vTcamId,GT_TRUE);
        cpssOsPrintf("*** AutoResize Enable set to TRUE ***\n");

        cpssOsTimeRT(&stopSeconds, &stopNanoSeconds);
        if (stopNanoSeconds >= startNanoSeconds)
        {
            elapsedNanoSeconds = (stopNanoSeconds - startNanoSeconds);
            elapsedSeconds = (stopSeconds - startSeconds);
        }
        else
        {
            elapsedNanoSeconds = ((1000*1000*1000) + stopNanoSeconds) - startNanoSeconds;
            elapsedSeconds = (stopSeconds - startSeconds) - 1;
        }
        cpssOsPrintf("Delete forward time: [%d.%06d] secs ([%d] TCAM size) \n",
            elapsedSeconds, elapsedNanoSeconds / (1000),
            totalRulesAllowed);

        /* get actual number of rules used+free in the vtcam */
        st = cpssDxChVirtualTcamUsageGet(vTcamMngId,vTcamId,&vTcamUsage);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamUsageGet failed\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, vTcamUsage.rulesFree, "vTcamUsage.rulesFree != 0\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, vTcamUsage.rulesUsed, "vTcamUsage.rulesUsed != 0\n");

        cpssOsTimeRT(&startSeconds, &startNanoSeconds);
        for (idx = 0; (idx < totalRulesAllowed); idx++)
        {
            ruleAttributes.priority = idx;
            st = addRuleNoHitByIndexPriorityEx(
                vTcamMngId, vTcamId, idx /*ruleId*/, &ruleAttributes);
            UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, st, "addRuleNoHitByIndexPriorityEx failed for idx = %d\n",
                idx);
        }
        cpssOsTimeRT(&stopSeconds, &stopNanoSeconds);
        if (stopNanoSeconds >= startNanoSeconds)
        {
            elapsedNanoSeconds = (stopNanoSeconds - startNanoSeconds);
            elapsedSeconds = (stopSeconds - startSeconds);
        }
        else
        {
            elapsedNanoSeconds = ((1000*1000*1000) + stopNanoSeconds) - startNanoSeconds;
            elapsedSeconds = (stopSeconds - startSeconds) - 1;
        }
        cpssOsPrintf("Create forward time: [%d.%06d] secs ([%d] TCAM size) \n",
            elapsedSeconds, elapsedNanoSeconds / (1000),
            totalRulesAllowed);

        cpssDxChVirtualTcamAutoResizeEnable(vTcamMngId,vTcamId,GT_FALSE);
        cpssOsPrintf("*** AutoResize Enable set to FALSE ***\n");

        cpssOsTimeRT(&startSeconds, &startNanoSeconds);
        for (idx = totalRulesAllowed-1; (idx < totalRulesAllowed); idx--)
        {
            st = cpssDxChVirtualTcamRuleDelete(
                vTcamMngId, vTcamId, idx /*ruleId*/);
            UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, st, "cpssDxChVirtualTcamRuleDelete failed for idx = %d\n",
                idx);
        }

        /* get actual number of rules used+free in the vtcam */
        st = cpssDxChVirtualTcamUsageGet(vTcamMngId,vTcamId,&vTcamUsage);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamUsageGet failed\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(totalRulesAllowed, vTcamUsage.rulesFree, "vTcamUsage.rulesFree != 0\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, vTcamUsage.rulesUsed, "vTcamUsage.rulesUsed != 0\n");

        cpssDxChVirtualTcamAutoResizeEnable(vTcamMngId,vTcamId,GT_TRUE);
        cpssOsPrintf("*** AutoResize Enable set to TRUE ***\n");

        cpssOsTimeRT(&stopSeconds, &stopNanoSeconds);
        if (stopNanoSeconds >= startNanoSeconds)
        {
            elapsedNanoSeconds = (stopNanoSeconds - startNanoSeconds);
            elapsedSeconds = (stopSeconds - startSeconds);
        }
        else
        {
            elapsedNanoSeconds = ((1000*1000*1000) + stopNanoSeconds) - startNanoSeconds;
            elapsedSeconds = (stopSeconds - startSeconds) - 1;
        }
        cpssOsPrintf("Delete backwards time: [%d.%06d] secs ([%d] TCAM size) \n",
            elapsedSeconds, elapsedNanoSeconds / (1000),
            totalRulesAllowed);

        /* get actual number of rules used+free in the vtcam */
        st = cpssDxChVirtualTcamUsageGet(vTcamMngId,vTcamId,&vTcamUsage);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamUsageGet failed\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, vTcamUsage.rulesFree, "vTcamUsage.rulesFree != 0\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, vTcamUsage.rulesUsed, "vTcamUsage.rulesUsed != 0\n");

        st = cpssDxChVirtualTcamRemove(
            vTcamMngId, vTcamId);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st, "cpssDxChVirtualTcamRemove failed for vTcamId = %d\n",
            vTcamId);

        st = cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st, "cpssDxChVirtualTcamManagerDevListRemove failed for vTcamMngId = %d\n",
            vTcamMngId);
        st = cpssDxChVirtualTcamManagerDelete(vTcamMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDelete failed for vTcamMngId = %d\n",
                                     vTcamMngId);
    }

    if (isTestSkipped)
    {
        SKIP_TEST_MAC;
    }
}

UTF_TEST_CASE_MAC(cpssDxChVirtualTcam_priority_autoresize_full_tcam_80B)
{
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT prvRuleSize_save;

    prvRuleSize_save = prvRuleSize;
    prvRuleSize = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E;

    UTF_TEST_CALL_MAC(cpssDxChVirtualTcam_priority_autoresize_full_tcam);

    prvRuleSize = prvRuleSize_save;
}

UTF_TEST_CASE_MAC(cpssDxChVirtualTcam_priority_autoresize_full_tcam_tti)
{
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_ENT prvRuleType_save;

    prvRuleType_save = prvRuleType;
    prvRuleType = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_TTI_E;

    UTF_TEST_CALL_MAC(cpssDxChVirtualTcam_priority_autoresize_full_tcam);

    prvRuleType = prvRuleType_save;
}

static void typicalVtcamSpaceTest
(
    IN GT_U8       devListArr[],
    IN GT_U32      numOfDevs,
    IN GT_U32      fullingVTcamID,
    IN GT_U32      fullingVTcamHitNumber,
    IN GT_U32      fullingVTcamClientGroup,
    IN GT_U32      leaveFreeBlocks,
    IN VT_SEQUENCE sequence[],
    IN GT_U32      sequenceSize
)
{
    GT_STATUS                                  st   = GT_OK;
    GT_U32                                     vTcamMngId;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC            vTcamInfo;
    GT_U32                                     idx;
    GT_U32                                     totalRulesAllowed;

    vTcamMngId = 1;
    cpssOsMemSet(&vTcamInfo, 0, sizeof(vTcamInfo));
    vTcamInfo.hitNumber     = fullingVTcamHitNumber;
    vTcamInfo.clientGroup   = fullingVTcamClientGroup;
    vTcamInfo.autoResize    = GT_FALSE;
    vTcamInfo.ruleAdditionMethod =
        CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E;

    /* Create vTCAM manager */
    st = cpssDxChVirtualTcamManagerCreate(vTcamMngId);
        if (st == GT_ALREADY_EXIST)
            st = GT_OK;
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerCreate failed\n");

    st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
    if (st != GT_OK)
    {
        cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
        st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,devListArr,numOfDevs);
    }
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListAdd failed\n");

    /* create vTcam 100 occupying all blocks beside two to TTI */
    vTcamInfo.ruleSize    = RS_60;
    internal_ut_cpssDxChVirtualTcamTotalRulesAllowedPerDevGet(
                devListArr[0], vTcamInfo.ruleSize, &totalRulesAllowed);
    vTcamInfo.guaranteedNumOfRules =
        totalRulesAllowed - (leaveFreeBlocks * NUM_60_RULES_PER_BLOCK);
    st = ut_cpssDxChVirtualTcamCreate(vTcamMngId, fullingVTcamID, &vTcamInfo);
    UTF_VERIFY_NOT_EQUAL0_STRING_MAC(
        GT_FAIL, st, "ut_cpssDxChVirtualTcamCreate failed\n");

    vTcamInfo.hitNumber     = 0;
    for (idx = 0; (idx < sequenceSize); idx++)
    {
        st = vtcamSequenceEntryProcess(sequence, idx, vTcamMngId, &vTcamInfo);
        if (sequence[idx].operation == VT_CREATE)
        {
            PRV_UTF_LOG4_MAC(
                "Created vTcam %d ruleSize %d rulesAmount %d clientGroup %d\n",
                sequence[idx].vTcamId, (sequence[idx].ruleSize + 1),
                sequence[idx].rulesAmount, sequence[idx].clientGroup);
        }
        if (sequence[idx].operation == VT_REMOVE)
        {
            PRV_UTF_LOG1_MAC("Removed vTcam %d\n", sequence[idx].vTcamId);
        }
        if (st != GT_OK) PRV_UTF_LOG0_MAC("Operation Failed\n");

        UTF_VERIFY_EQUAL1_STRING_MAC(
            sequence[idx].rc, st,
            "vtcamSequenceEntryProcess failed, index: %d\n", idx);
    }

    ut_cpssDxChVirtualTcamRemove(
        vTcamMngId, fullingVTcamID);

    st = cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, st, "cpssDxChVirtualTcamManagerDevListRemove failed for vTcamMngId = %d\n",
        vTcamMngId);
    st = cpssDxChVirtualTcamManagerDelete(vTcamMngId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDelete failed for vTcamMngId = %d\n",
                                 vTcamMngId);
}

UTF_TEST_CASE_MAC(cpssDxChVirtualTcam_make_space_for_80_bytes)
{
    GT_U8                                      dev;
    GT_U32                                     numOfDevs = 0;
    GT_U32                                     notAppFamilyBmp = 0;
    GT_U32                                     utfFamilyBit = 0;
    GT_BOOL                                    isTestSkipped = GT_TRUE;

    /* skip any case, prevent compiler warnings */
     /*numOfDevs = 0; if (numOfDevs == 0) SKIP_TEST_MAC;*/

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* 1. Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
              devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }
        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;

        {
            /* splitting VTCAM from one full block to vertical half of pair */
            static VT_SEQUENCE sequence[] =
            {
                {VT_TITLE,       0, CG_NONE, RS_NONE,    0, GT_OK,
                    "Create 15 (256 rules of 60 bytes) ClientGroup 0, Hit 0 "},
                {VT_CREATE,     15, CG_PCL,  RS_60,    256, GT_OK},
                {VT_STAMP_FILL, 15, CG_NONE, RS_NONE,    0, GT_OK},
                {VT_STAMP_CHECK,15, CG_NONE, RS_NONE,    0, GT_OK},
                {VT_TITLE,       0, CG_NONE, RS_NONE,    0, GT_OK,
                    "Create and remove 16 (256 rules of 60 bytes) ClientGroup 0, Hit 2 "},
                {VT_CREATE,     16, 0x20,  RS_60,    256, GT_OK},
                {VT_REMOVE,     16, 0x20,  RS_60,    256, GT_OK},
                {VT_TITLE,       0, CG_NONE, RS_NONE,    0, GT_OK,
                    "Create 1 (128 rules of 80 bytes) ClientGroup 0, Hit 0 "},
                {VT_CREATE,      1, CG_PCL,  RS_80,    128, GT_OK},
                {VT_STAMP_FILL,  1, CG_NONE, RS_NONE,    0, GT_OK},
                {VT_STAMP_CHECK, 1, CG_NONE, RS_NONE,    0, GT_OK},
                {VT_TITLE,       0, CG_NONE, RS_NONE,    0, GT_OK,
                    "Check 15 (256 rules of 60 bytes) ClientGroup 0, Hit 1 "},
                {VT_STAMP_CHECK,15, CG_NONE, RS_NONE,    0, GT_OK},
            };

            typicalVtcamSpaceTest(
                devListArr, numOfDevs,
                100      /*fullingVTcamID*/,
                2      /*fullingVTcamHitNumber*/,
                1      /*fullingVTcamClientGroup*/,
                2      /*leaveFreeBlocks*/,
                sequence,
                (sizeof(sequence) / sizeof(sequence[0])));
        }

        {
            /* moving full block */
            static VT_SEQUENCE sequence[] =
            {
                {VT_TITLE,       0, CG_NONE, RS_NONE,    0, GT_OK,
                    "Create 15 (256 rules of 60 bytes) ClientGroup 0, Hit 2 "},
                {VT_CREATE,     15, 0x20,  RS_60,    256, GT_OK},
                {VT_TITLE,       0, CG_NONE, RS_NONE,    0, GT_OK,
                    "Create 16 and remove 15 (256 rules of 60 bytes) ClientGroup 0, Hit 1 "},
                {VT_CREATE,     16, 0x10,  RS_60,    256, GT_OK},
                {VT_STAMP_FILL, 16, CG_NONE, RS_NONE,    0, GT_OK},
                {VT_STAMP_CHECK,16, CG_NONE, RS_NONE,    0, GT_OK},
                {VT_REMOVE,     15, 0x20,  RS_60,    256, GT_OK},
                {VT_TITLE,       0, CG_NONE, RS_NONE,    0, GT_OK,
                    "Create 1 (256 rules of 80 bytes) ClientGroup 0, Hit 0 "},
                {VT_CREATE,      1, CG_PCL,  RS_80,    256, GT_OK},
                {VT_STAMP_FILL,  1, CG_NONE, RS_NONE,    0, GT_OK},
                {VT_STAMP_CHECK, 1, CG_NONE, RS_NONE,    0, GT_OK},
                {VT_TITLE,       0, CG_NONE, RS_NONE,    0, GT_OK,
                    "Check 16 (256 rules of 60 bytes) ClientGroup 0, Hit 1 "},
                {VT_STAMP_CHECK,16, CG_NONE, RS_NONE,    0, GT_OK},
            };

            typicalVtcamSpaceTest(
                devListArr, numOfDevs,
                100      /*fullingVTcamID*/,
                2      /*fullingVTcamHitNumber*/,
                1      /*fullingVTcamClientGroup*/,
                3      /*leaveFreeBlocks*/,
                sequence,
                (sizeof(sequence) / sizeof(sequence[0])));
        }

        {
            /* 256 * 30-bytes move to reminder of 256 * 80-bytes */
            static VT_SEQUENCE sequence[] =
            {
                {VT_TITLE,       0, CG_NONE, RS_NONE,    0, GT_OK,
                    "Create 15 (256 rules of 30 bytes"},
                {VT_CREATE,     15, CG_PCL,  RS_30,    256, GT_OK},
                {VT_STAMP_FILL, 15, CG_NONE, RS_NONE,    0, GT_OK},
                {VT_STAMP_CHECK,15, CG_NONE, RS_NONE,    0, GT_OK},
                {VT_TITLE,       0, CG_NONE, RS_NONE,    0, GT_OK,
                    "Create 1 (256 rules of 80 bytes)"},
                {VT_CREATE,      1, CG_PCL,  RS_80,    256, GT_OK},
                {VT_STAMP_FILL,  1, CG_NONE, RS_NONE,    0, GT_OK},
                {VT_STAMP_CHECK, 1, CG_NONE, RS_NONE,    0, GT_OK},
                {VT_TITLE,       0, CG_NONE, RS_NONE,    0, GT_OK,
                    "Check 15 (256 rules of 60 bytes)"},
                {VT_STAMP_CHECK,15, CG_NONE, RS_NONE,    0, GT_OK},
            };

            typicalVtcamSpaceTest(
                devListArr, numOfDevs,
                100      /*fullingVTcamID*/,
                2      /*fullingVTcamHitNumber*/,
                1      /*fullingVTcamClientGroup*/,
                2      /*leaveFreeBlocks*/,
                sequence,
                (sizeof(sequence) / sizeof(sequence[0])));
        }
    }

    if (isTestSkipped)
    {
        SKIP_TEST_MAC;
    }
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChVirtualTcam suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChVirtualTcam)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVirtualTcam_createRemove)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVirtualTcam_checkValidInvalidLogicalIndex)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVirtualTcam_checkValidInvalidLogicalIndex_tti)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVirtualTcam_increaseVtcamSizebyResizing_priorityMode)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVirtualTcam_increaseVtcamSizebyResizing)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVirtualTcam_decreaseVtcamSizeByResizing)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVirtualTcam_deleteVtcam)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVirtualTcam_createVtcamsLtoSkeySize)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVirtualTcam_createVtcamsRegardlesskeySizeOrdering)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVirtualTcam_createVtcamsInDifferentGroups)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVirtualTcam_fragmentation)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVirtualTcam_column_fragmentation_one_block)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVirtualTcam_column_fragmentation_two_blocks)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVirtualTcam_block_fragmentation)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVirtualTcam_block_fragmentation_autoresize)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVirtualTcam_block_fragmentation_columnTypes)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVirtualTcam_block_hole_fragmentation)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVirtualTcam_block_fragmentation_80B)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVirtualTcam_block_fragmentation_80B_columnTypes)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVirtualTcam_writeDeleteRulesLogicalIndex)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVirtualTcam_moveRulesLogicalIndex)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVirtualTcam_resizeRulesLogicalIndex)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVirtualTcam_resizeRulesPriorityMode)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVirtualTcam_basicRuleWrite_priorityMode)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVirtualTcam_addRulesWithDifferentPriority)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVirtualTcam_decreaseVtcamSizeByResizing_priorityMode)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVirtualTcam_autoresize)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVirtualTcam_releaseBlocksCheck)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVirtualTcam_causeError_60B)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVirtualTcam_bc2_sequence_creation_order_error)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVirtualTcam_bc2_sequence_creation_order_ok)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVirtualTcam_memoryAvailabilityCheck_fromEmptyAndGrow)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVirtualTcam_createAndUpdate_fromFullWithUpdates)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVirtualTcam_memoryAvailabilityCheck_fromFullWithUpdates)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVirtualTcam_DevListAdd_toExistingDeviceWithRules)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVirtualTcam_DevListAdd_toExistingDeviceWithRules_priorityMode)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVirtualTcam_AutoResizeCheck)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVirtualTcam_ConvertRuleIdToHwIndex)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVirtualTcam_rule80_space)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVirtualTcam_rule80_xCat3_space)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVirtualTcam_priority_autoresize_full_tcam)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVirtualTcam_priority_autoresize_full_tcam_80B)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVirtualTcam_priority_autoresize_full_tcam_tti)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVirtualTcam_make_space_for_80_bytes)


/*    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChVirtualTcam_priorityMode_writeRulePositionForceFirst)*/
UTF_SUIT_END_TESTS_MAC(cpssDxChVirtualTcam)


