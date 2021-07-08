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
* @file prvUtfSuitsConfig.h
*
* @brief Internal header which is included only by UTF code
* and provides configuration for all suits.
*
* @version   188
********************************************************************************
*/



#if !(defined CHX_FAMILY)
#ifndef PX_FAMILY
/* WA for compilation script */
  #define PX_FAMILY
#endif
#endif

/* includes */
#include "prvUtfSuitsStart.h"

/* defines */
/* Provides more flexible switching between different configuration of suites */
#define PRV_UTF_SUITES_CONFIG_FILENAME_CNS "prvUtfSuitsConfig.h"

/* Uses macros to declare test-suite functions
 * and then implements utfDeclareAllSuits function
 * that calls test-suite functions.
 * Functions that are defined as arguments of UTF_MAIN_DECLARE_SUIT_MAC macros
 * will be declared and called.
 *
 * If you what add a Suit to the image please paste
 * UTF_MAIN_DECLARE_SUIT_MAC(name of Suit) here.
 */
UTF_MAIN_BEGIN_SUITS_MAC()
#if defined(CHX_FAMILY) || defined(CH3_FAMILY) || defined(CH3P_FAMILY)
    /* declare test type for general suits */
    UTF_MAIN_DECLARE_TEST_TYPE_MAC(UTF_TEST_TYPE_GEN_E);


    /* Common UTs */
#if (defined CHX_FAMILY)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChCatchUp)
#endif
    UTF_MAIN_DECLARE_SUIT_MAC(cpssBuffManagerPool)
    UTF_MAIN_DECLARE_SUIT_MAC(prvCpssMisc)


    /* Generic UTs */
    UTF_MAIN_DECLARE_SUIT_MAC(cpssGenCfg)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssGenPhyVct)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssGenSmi)

    /* Cpss Driver UTs*/
#if (defined CHX_FAMILY) || (defined EXMXPM_FAMILY)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssPpDrvHwPexMbus)
#endif /* CHX_FAMILY || EXMXPM_FAMILY*/

    /* GM does not support TM */
#if (defined CHX_FAMILY) && (!(defined GM_USED))
#if defined (INCLUDE_TM)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssTmSched)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssTmDrop)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssTmNodesRead)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssTmShaping)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssTmNodesTree)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssTmNodesUpdate)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssTmNodesReorder)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssTmNodesCtl)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssTmNodesCreate)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssTmNodesStatus)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssTmEligPrioFunc)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssTmLogicalLayer)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssTmInternal)
#endif /*#if defined (INCLUDE_TM)*/
#endif

    /* finish declaration */
    UTF_MAIN_DECLARE_TEST_TYPE_MAC(UTF_TEST_TYPE_NONE_E);

    /* declare test type for DxCh suits */
    UTF_MAIN_DECLARE_TEST_TYPE_MAC(UTF_TEST_TYPE_CHX_E);
#if (defined CHX_FAMILY)
    /* the tests under cpssDxChNetIf_packetSend are tests that may send packets ,
       and need a 'correct' tables configurations.
       so those should run as 'first tests' after cpssInitSystem */
    /* UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChInitSystemOnDiffCC) */ /* shall be removed when problem with dead-lock is served */
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChNetIf_packetSend)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChBobcat2PortPizzaArbiter)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChBrgCount)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChBrgE2Phy)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChBrgEgrFlt)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChBrgFdb)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChBrgFdbRouting)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChBrgGen)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChBrgL2Ecmp)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChBrgMc)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChBrgNestVlan)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChBrgPrvEdgeVlan)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChBrgSecurityBreach)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChBrgSrcId)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChBrgStp)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChBrgVlan)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChBrgPe)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChCnc)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChCfgInit)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChCos)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChCscd)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChCutThrough)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChDiag)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChDiagDescriptor)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChDiagDataIntegrity)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChDiagDataIntegrityTables)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChDiagPacketGenerator)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChFabricHGLink)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChIp)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChIpCtrl)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChIpFix)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChIpLpm)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChHwInit)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChHwInitLedCtrl)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChLatencyMonitoring)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChLogicalTargetMapping)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChMirror)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChStc)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChNetIf)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChNetIfMii)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChNst)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChNstPortIsolation)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChPcl)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChPha)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChPhySmi)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChPhySmiPreInit)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChPolicer)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChPortInterlaken)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChPortBufMg)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChPortCn)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChPortEcn)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChPortPfc)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChPortSyncEther)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChPortCtrl)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChPortStat)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChPortTx)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChPortIfModeCfg)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChPortPip)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChPortEee)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChLion2PortPizzaArbiter)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChPtp)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChTcamManager)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChTrunkHighLevel)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChTrunkLowLevel)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChTrunkHighLevel_1)/*second run*/
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChTrunkLowLevel_1) /*second run*/
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChTti)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChTtiRule)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChTunnel)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChVersion)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChVnt)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChOam)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChL2Mll)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChHsu)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChPortAp)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChTcam)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChExactMatch)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChVirtualTcam)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChProtection)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChLpm)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChPortLoopback)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChFalconPortPizzaArbiter)
    UTF_MAIN_DECLARE_SUIT_MAC(prvCpssDxChPortPacketBuffer)

#if defined (INCLUDE_TM)
#ifndef GM_USED
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChTmGlueAgingAndDelay)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChTmGlueDram)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChTmGluePfc)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChTmGlueFlowControl)
#endif
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChTmGlueQueueMap)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChTmGlueDrop)
#endif /*#if defined (INCLUDE_TM)*/

    UTF_MAIN_DECLARE_SUIT_MAC(mvHws)

    /* should be last - mapping DB is corrupted - therefore APIs will not work */
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChPortMapping)

#endif /* (defined CHX_FAMILY) */

#if (defined CH3_FAMILY)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxCh3Policer)
#endif /* (defined CH3_FAMILY) */

#if (defined CH3P_FAMILY)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxCh3pBrgCapwap)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxCh3pBrgMc)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxCh3pBrgVlan)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxCh3pCapwapGen)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxCh3pIp)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxCh3pTti)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxCh3pTunnel)
#endif /* (defined CH3P_FAMILY) */

#if (defined CHX_FAMILY)
    /* cpssDxChMultiPortGroup :
       those tests MUST come last because they change topology of the
       cpssInitSystem , and not restore them ...  */
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChMultiPortGroup)
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChDiagBist)

    /* cpssDxChHwInitDeviceMatrix MUST be last test for DxCh families*/
    UTF_MAIN_DECLARE_SUIT_MAC(cpssDxChHwInitDeviceMatrix)
#endif /* (defined CHX_FAMILY) */

    /* finish declaration */
    UTF_MAIN_DECLARE_TEST_TYPE_MAC(UTF_TEST_TYPE_NONE_E);

    /* declare test type for Enhanced UT suits */
    UTF_MAIN_DECLARE_TEST_TYPE_MAC(UTF_TEST_TYPE_TRAFFIC_E);
#if (defined CHX_FAMILY) && (defined IMPL_TGF)

    UTF_MAIN_DECLARE_SUIT_MAC(tgfExactMatch)
    UTF_MAIN_DECLARE_SUIT_MAC(tgfOam)
    UTF_MAIN_DECLARE_SUIT_MAC(tgfStc)
    UTF_MAIN_DECLARE_SUIT_MAC(tgfConfig)
    UTF_MAIN_DECLARE_SUIT_MAC(tgfDiag)
    UTF_MAIN_DECLARE_SUIT_MAC(tgfTailDrop)
    UTF_MAIN_DECLARE_SUIT_MAC(tgfCscd)
    UTF_MAIN_DECLARE_SUIT_MAC(tgfCnc)
    UTF_MAIN_DECLARE_SUIT_MAC(tgfCutThrough)
    UTF_MAIN_DECLARE_SUIT_MAC(tgfPort)
    UTF_MAIN_DECLARE_SUIT_MAC(tgfLogicalTarget)
    UTF_MAIN_DECLARE_SUIT_MAC(tgfTunnel)
    UTF_MAIN_DECLARE_SUIT_MAC(tgfBridge)
    UTF_MAIN_DECLARE_SUIT_MAC(tgfIp)
    UTF_MAIN_DECLARE_SUIT_MAC(tgfNst)
    UTF_MAIN_DECLARE_SUIT_MAC(tgfPcl)
    UTF_MAIN_DECLARE_SUIT_MAC(tgfVnt)
    UTF_MAIN_DECLARE_SUIT_MAC(tgfPolicer)
    UTF_MAIN_DECLARE_SUIT_MAC(tgfMirror)
    UTF_MAIN_DECLARE_SUIT_MAC(tgfTrunk)
    UTF_MAIN_DECLARE_SUIT_MAC(tgfIpfix)
    UTF_MAIN_DECLARE_SUIT_MAC(tgfCos)
    UTF_MAIN_DECLARE_SUIT_MAC(tgfL2Mll)
    UTF_MAIN_DECLARE_SUIT_MAC(tgfMpls)
    UTF_MAIN_DECLARE_SUIT_MAC(tgfFcoe)
    UTF_MAIN_DECLARE_SUIT_MAC(tgfPtp)
    UTF_MAIN_DECLARE_SUIT_MAC(tgfProtection)
    UTF_MAIN_DECLARE_SUIT_MAC(tgfNetIf)
    UTF_MAIN_DECLARE_SUIT_MAC(tgfLion2Fabric)
    UTF_MAIN_DECLARE_SUIT_MAC(tgfEvent)
    UTF_MAIN_DECLARE_SUIT_MAC(tgfVirtualTcam)
    UTF_MAIN_DECLARE_SUIT_MAC(tgfQcn)
#endif /* IMPL_TGF */
    /* finish declaration */
    UTF_MAIN_DECLARE_TEST_TYPE_MAC(UTF_TEST_TYPE_NONE_E);

    /* declare test type for Enhanced UT suits */
    UTF_MAIN_DECLARE_TEST_TYPE_MAC(UTF_TEST_TYPE_TRAFFIC_FDB_NON_UNIFIED_E);
#if (defined CHX_FAMILY) && (defined IMPL_TGF)
    /* enhanced UT */
    UTF_MAIN_DECLARE_SUIT_MAC(tgfBridge_ForMultiCoreFdbLookup)
#endif /* IMPL_TGF */
    /* finish declaration */
    UTF_MAIN_DECLARE_TEST_TYPE_MAC(UTF_TEST_TYPE_NONE_E);

#endif /*defined(CHX_FAMILY) || defined(CH3_FAMILY) || defined(CH3P_FAMILY)*/

#ifdef PX_FAMILY
    UTF_MAIN_DECLARE_TEST_TYPE_MAC(UTF_TEST_TYPE_PX_E);

#ifndef DXPX
    /* Generic suits */
    UTF_MAIN_DECLARE_SUIT_MAC(cpssGenSmi)
#endif

    /* the tests under cpssPxNetIf_packetSend are tests that may send packets ,
       and need a 'correct' tables configurations.
       so those should run as 'first tests' after cpssInitSystem */
    UTF_MAIN_DECLARE_SUIT_MAC(cpssPxNetIf_packetSend);
    UTF_MAIN_DECLARE_SUIT_MAC(cpssPxNetIf);
    UTF_MAIN_DECLARE_SUIT_MAC(cpssPxDiag);
    UTF_MAIN_DECLARE_SUIT_MAC(cpssPxDiagPacketGenerator);
    UTF_MAIN_DECLARE_SUIT_MAC(cpssPxCfgInit);
    UTF_MAIN_DECLARE_SUIT_MAC(cpssPxPortStat);
    UTF_MAIN_DECLARE_SUIT_MAC(cpssPxPortTxScheduler);
    UTF_MAIN_DECLARE_SUIT_MAC(cpssPxPortTxTailDrop);
    UTF_MAIN_DECLARE_SUIT_MAC(cpssPxPortCtrl);
    UTF_MAIN_DECLARE_SUIT_MAC(cpssPxPortTxDebug);
    UTF_MAIN_DECLARE_SUIT_MAC(cpssPxPortTxShaper);
    UTF_MAIN_DECLARE_SUIT_MAC(cpssPxIngress);
    UTF_MAIN_DECLARE_SUIT_MAC(cpssPxIngressHash);
    UTF_MAIN_DECLARE_SUIT_MAC(cpssPxEgress);
    UTF_MAIN_DECLARE_SUIT_MAC(cpssPxCos);
    UTF_MAIN_DECLARE_SUIT_MAC(cpssPxPortBufMg);
    UTF_MAIN_DECLARE_SUIT_MAC(cpssPxCutThrough);
    UTF_MAIN_DECLARE_SUIT_MAC(cpssPxCnc);
    UTF_MAIN_DECLARE_SUIT_MAC(cpssPxHwInitLedCtrl);
    UTF_MAIN_DECLARE_SUIT_MAC(cpssPxPtp);
    UTF_MAIN_DECLARE_SUIT_MAC(cpssPxPortCn);
    UTF_MAIN_DECLARE_SUIT_MAC(cpssPxPortEcn);
    UTF_MAIN_DECLARE_SUIT_MAC(cpssPxPhySmi);
    UTF_MAIN_DECLARE_SUIT_MAC(cpssPxPortPfc);
    UTF_MAIN_DECLARE_SUIT_MAC(cpssPxPortSyncEther);
    UTF_MAIN_DECLARE_SUIT_MAC(cpssPxDiagDataIntegrity);
    UTF_MAIN_DECLARE_SUIT_MAC(cpssPxDiagDataIntegrityTables);
    /* BIST test damages whole memories - must run after all other */
    UTF_MAIN_DECLARE_SUIT_MAC(cpssPxDiagBist);

    /* cpssPxHwInitDeviceMatrix MUST be last test for Px family */
    UTF_MAIN_DECLARE_SUIT_MAC(cpssPxHwInitDeviceMatrix);

    /**************************/
    /* add here more PX tests */
    /**************************/

    /* finish declaration */
    UTF_MAIN_DECLARE_TEST_TYPE_MAC(UTF_TEST_TYPE_NONE_E);

    /***********************************/
    UTF_MAIN_DECLARE_TEST_TYPE_MAC(UTF_TEST_TYPE_PX_TRAFFIC_E);

    UTF_MAIN_DECLARE_SUIT_MAC(cpssPxTgfBasic);
    UTF_MAIN_DECLARE_SUIT_MAC(cpssPxTgfEgress802_1br);
    UTF_MAIN_DECLARE_SUIT_MAC(cpssPxTgfEgressDsa);
    UTF_MAIN_DECLARE_SUIT_MAC(cpssPxTgfPxPfc);
    UTF_MAIN_DECLARE_SUIT_MAC(cpssPxTgfDataIntegrity);
    UTF_MAIN_DECLARE_SUIT_MAC(cpssPxTgfIngress);
    UTF_MAIN_DECLARE_SUIT_MAC(cpssPxTgfIngressHash);
    UTF_MAIN_DECLARE_SUIT_MAC(cpssPxTgfPxCnc);
    UTF_MAIN_DECLARE_SUIT_MAC(cpssPxTgfEvent);
    UTF_MAIN_DECLARE_SUIT_MAC(cpssPxTgfPortTxTailDrop);

    /* finish declaration */
    UTF_MAIN_DECLARE_TEST_TYPE_MAC(UTF_TEST_TYPE_NONE_E);
    /***********************************/

#endif /*PX_FAMILY*/

UTF_MAIN_END_SUITS_MAC()

/* includes */
#include "prvUtfSuitsEnd.h"

