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
* mvHwsPortCtrlAp.c
*
* DESCRIPTION:
*       Port Control AP Detection Engine
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/
#include <mvHwsPortCtrlInc.h>
#include <mvHwsServiceCpuInt.h>
#include <cpss/common/labServices/port/gop/port/private/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#ifdef BV_DEV_SUPPORT
#include "mcdApiTypes.h"
#include "mcdApiRegs.h"
#include "mcdHwCntl.h"
#include "mcdHwSerdesCntl.h"
#include "mcdAPI.h"
#include "mcdDiagnostics.h"
#include "mcdAPIInternal.h"
#include "mcdFwDownload.h"
#include "mcdInitialization.h"
extern MCD_DEV mcdDevDb;
#endif
#include "mvHwsPortCtrlDoorbellEvents.h"
#include "micro_init/NetPort.h"

extern GT_STATUS mvHwsPortFixAlign90Ext
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode
);

extern GT_STATUS mvHwsAvagoSerdesTxEnable
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    GT_BOOL     enable
);

extern GT_STATUS mvHwsPortEnableSet
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          phyPortNum,
    MV_HWS_PORT_STANDARD            portMode,
    GT_BOOL                         enable
);

extern GT_STATUS mvHwsPortCheckGearBox
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *laneLock
);

extern GT_STATUS mvHwsPcsAlignLockGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *lock
);

extern GT_STATUS mvHwsPcsPortLinkStatusGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *linkStatus
);

#ifdef ALDRIN_DEV_SUPPORT
extern GT_U8 refClockSourceDb[ALDRIN_PORTS_NUM];
#endif

#if defined(BC3_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)
extern GT_U8 refClockSourceDb[MV_HWS_MAX_PORT_NUM];
#endif

/* Default values for AP Device and Port Group */
#ifdef MV_SERDES_AVAGO_IP
#include "mv_hws_avago_if.h"
#endif /* MV_SERDES_AVAGO_IP */

/*
   By default the VCO calibration is operated on Avago Serdes.
   To bypass the VCO calibration for AP port set:
   apPortGroup = AVAGO_SERDES_INIT_BYPASS_VCO
*/
GT_U32 apPortGroup = 0;

GT_U8  apDevNum = 0;

/**
* @internal mvHwsApConvertPortMode function
* @endinternal
*
*/
MV_HWS_PORT_STANDARD mvHwsApConvertPortMode(MV_HWA_AP_PORT_MODE apPortMode)
{
    switch (apPortMode)
    {
    case Port_1000Base_KX:   return _1000Base_X;
    case Port_10GBase_KX4:   return _10GBase_KX4;
    case Port_10GBase_R:     return _10GBase_KR;
    case Port_25GBASE_KR_S:  return _25GBase_KR_S;
    case Port_25GBASE_KR:    return _25GBase_KR;
    case Port_40GBase_R:     return _40GBase_KR4;
    case Port_40GBASE_CR4:   return _40GBase_CR4;
    case Port_100GBASE_KR4:  return _100GBase_KR4;
    case Port_100GBASE_CR4:  return _100GBase_CR4;
    case Port_25GBASE_KR_C:  return _25GBase_KR_C;
    case Port_25GBASE_CR_C:  return _25GBase_CR_C;
    case Port_50GBASE_KR2_C: return _50GBase_KR2_C;
    case Port_50GBASE_CR2_C: return _50GBase_CR2_C;
    case Port_25GBASE_CR_S:  return _25GBase_CR_S;
    case Port_25GBASE_CR:    return _25GBase_CR;
    default:                 return NON_SUP_MODE;
    }
}
/*******************************************************************************
* mvApArbFsmGet
* mvApArbFsmSet
*
* DESCRIPTION: List of AP ARM SM API's
*******************************************************************************/

/** mvApArbFsmGet
*******************************************************************************/
MV_HWS_AP_SM_STATE mvApArbFsmGet(GT_U8 pcsNum)
{
    /* Read Arbiter FSM from ANEG Control Register 0
    ** (internal reg 0x80) bits [14:4] to AP port status register
    */
    GT_U32 data;
    MV_HWS_AP_SM_STATE arbState;

    CHECK_STATUS(genUnitRegisterGet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                    AP_INT_REG_ANEG_CTRL_0, &data, 0));

    arbState = (data >> AP_INT_REG_ANEG_CTRL_0_ANEG_STATE_SHIFT) &
                        AP_INT_REG_ANEG_CTRL_0_ANEG_STATE_MASK;

    return arbState;
}

/** mvApArbFsmSet
*******************************************************************************/
GT_STATUS mvApArbFsmSet(GT_U8 pcsNum, MV_HWS_AP_SM_STATE arbState)
{
    GT_U32 data = (GT_U32)arbState;
    AP_PRINT_MAC(("mvApArbFsmSet pcsNum %d set fsmState 0x%x\n",pcsNum, arbState));
    CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                    AP_INT_REG_ANEG_CTRL_0,
                                    (data << AP_INT_REG_ANEG_CTRL_0_ANEG_STATE_SHIFT) + 1,
                                             AP_INT_REG_ANEG_CTRL_0_ANEG_MASK));

    CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                    AP_INT_REG_ANEG_CTRL_0,
                                    (data << AP_INT_REG_ANEG_CTRL_0_ANEG_STATE_SHIFT),
                                             AP_INT_REG_ANEG_CTRL_0_ANEG_MASK));

    return GT_OK;
}


/** mvApArbFsmTrapSet
*******************************************************************************/
GT_STATUS mvApArbFsmTrapSet(GT_U8 pcsNum, MV_HWS_AP_SM_STATE arbState, GT_BOOL enable)
{
    GT_U32 data = (GT_U32)arbState;
    AP_PRINT_MAC(("mvApArbFsmTrapSet pcsNum %d set fsmState 0x%x enable %d\n",pcsNum, arbState, enable));
    if (enable) {
        CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                        AP_INT_REG_ANEG_CTRL_0,
                                        (data << AP_INT_REG_ANEG_CTRL_0_ANEG_STATE_SHIFT) + 0x6,
                                        AP_INT_REG_ANEG_CTRL_0_ANEG_MASK));
    }
    else
    {
        CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                        AP_INT_REG_ANEG_CTRL_0,
                                        0x4,
                                        AP_INT_REG_ANEG_CTRL_0_ANEG_MASK));
    }

    return GT_OK;
}

/** mvApArbFsmBpCheck
*******************************************************************************/
GT_STATUS mvApArbFsmBpCheck(GT_U8 pcsNum, GT_BOOL *bpReached)
{
    GT_U32 data;

    if (bpReached == NULL)
    {
      return GT_BAD_PARAM;
    }

    CHECK_STATUS(genUnitRegisterGet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                    AP_INT_REG_ANEG_CTRL_0, &data, 0));

    *bpReached = ((data >> 3) & 1);
    if (*bpReached) {
        AP_PRINT_MAC(("mvApArbFsmBpCheck pcsNum %d  fsmState 0x%x bpReached %d\n",pcsNum,
                  (data >> AP_INT_REG_ANEG_CTRL_0_ANEG_STATE_SHIFT),*bpReached));
    }
    return GT_OK;
}

/*******************************************************************************
* mvApSetPcsMux
* mvApPcsMuxReset
*
* DESCRIPTION: Set [9:8] bits in PCS40G Common Control 0x088C0414
*              Modes:
*               0x0 = 10G; 10G; PCS working in 10G with lane 0
*               0x1 = 20G; 20G; PCS working in 20G with lane 0,1
*               0x2 = 40G; 40G; PCS working in 40G with lane 0,1,2,3
*               0x3 = AP
*
*              Set to 0 in case of Reset
*******************************************************************************/

/** mvApSetPcsMux
*******************************************************************************/
GT_STATUS mvApPcsMuxSet(GT_U8 pcsNum, GT_U8 pcsMode)
{
    CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                    PCS40G_COMMON_CTRL_REG,
                                    (pcsMode << PCS40G_COMMON_CTRL_REG_PCS_MODE_SHIFT),
                                                PCS40G_COMMON_CTRL_REG_PCS_MODE_MASK));
    return GT_OK;
}

/** mvApPcsMuxReset
*******************************************************************************/
GT_STATUS mvApPcsMuxReset(GT_U8 pcsNum)
{
    GT_U32 data;

    CHECK_STATUS(genUnitRegisterGet(apDevNum, apPortGroup,MMPCS_UNIT, pcsNum,
                                    PCS40G_COMMON_CTRL_REG, &data, (3 << 8)));

    if (data == 0x300)
    {
        mvApPcsMuxSet(pcsNum, 0);
    }

    return GT_OK;
}

#if defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)
GT_STATUS mvApConfig(GT_U8 laneNum, GT_U8 mode)
{
    /* AutoNeg(AP) unit clock enable - Should be activated before unresetting AP. */
    CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, laneNum,
                                    AP_REG_CFG_1, ((3 == mode)<<3), 8));
    return GT_OK;
}
#endif

/*******************************************************************************
* mvApHcdFecParamsSet
* mvApHcdFcParamsSet
* mvApHcdAdvertiseSet
* mvApFcResolutionSet
*
* DESCRIPTION: List of AP HCD API's
*******************************************************************************/

/** mvApHcdFecParamsSet
*******************************************************************************/
GT_STATUS mvApHcdFecParamsSet(GT_U8 pcsNum, GT_U8 fecAbility, GT_U8 fecEnable)
{
    GT_U32 data = (fecEnable << 1) + fecAbility;

    CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                    AP_INT_REG_802_3_AP_ADV_REG_3,
                                    (data << AP_INT_REG_802_3_AP_ADV_REG_3_FEC_SHIFT),
                                             AP_INT_REG_802_3_AP_ADV_REG_3_FEC_MASK));

    return GT_OK;
}

GT_STATUS mvApHcdFecAdvancedParamsSet(GT_U8 pcsNum, GT_U8 fecRequest)
{
    CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                    AP_INT_REG_802_3_AP_ADV_REG_3,
                                    (fecRequest << AP_INT_REG_802_3_AP_ADV_REG_3_FEC_ADV_SHIFT),
                                             AP_INT_REG_802_3_AP_ADV_REG_3_FEC_ADV_MASK));
    return GT_OK;
}

#if defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)
/** mvApHcdFecNpConsortiumParamsSet
*******************************************************************************/
GT_STATUS mvApHcdFecNpConsortiumParamsSet(GT_U8 pcsNum, GT_U8 fecAbility, GT_U8 fecEnable)
{
    GT_U32 data = (fecEnable << 2) + fecAbility;

    CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                    AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31,
                                    (data << AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_FEC_SHIFT),
                                    AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_FEC_MASK));
    AP_PRINT_MAC(("mvApHcdFecNpConsortiumParamsSet  pcsNum:%d adr:0x%x data:0x%x, mask:0x%x\n",pcsNum, AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31, (data << AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_FEC_SHIFT),
                  (AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_FEC_MASK)));

    return GT_OK;
}
#endif
/** mvApHcdFcParamsSet
*******************************************************************************/
GT_STATUS mvApHcdFcParamsSet(GT_U8 pcsNum, GT_U8 fcEnable, GT_U8 fcDir)
{
  GT_U32 data = (fcDir << 1) + fcEnable;

  CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                  AP_INT_REG_802_3_AP_ADV_REG_1,
                                  (data << AP_INT_REG_802_3_AP_ADV_REG_1_FC_PAUSE_SHIFT),
                                           AP_INT_REG_802_3_AP_ADV_REG_1_FC_PAUSE_MASK));

  return GT_OK;
}

/** mvApHcdNpParamsSet
*******************************************************************************/
GT_STATUS mvApHcdParamsNpSet(GT_U8 pcsNum, GT_U8 npEnable)
{
  CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                  AP_INT_REG_802_3_AP_ADV_REG_1,
                                  (npEnable << AP_INT_REG_802_3_AP_ADV_REG_1_NP_SHIFT),
                                  AP_INT_REG_802_3_AP_ADV_REG_1_NP_MASK));

  return GT_OK;
}

/** mvApHcdAdvertiseSet
*******************************************************************************/
GT_STATUS mvApHcdAdvertiseSet(GT_U8 pcsNum, GT_U32 modeVector)
{
    GT_U32 data;

    /* data to write to CFG0 register*/
    data = (AP_CTRL_40GBase_KR4_GET(modeVector) << AP_REG_CFG_0_ADV_40G_KR4_SHIFT) |
           (AP_CTRL_10GBase_KR_GET(modeVector) << AP_REG_CFG_0_ADV_10G_KR_SHIFT) |
           (AP_CTRL_10GBase_KX4_GET(modeVector) << AP_REG_CFG_0_ADV_10G_KX4_SHIFT) |
           (AP_CTRL_1000Base_KX_GET(modeVector) << AP_REG_CFG_0_ADV_1G_KX_SHIFT);
    CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                        AP_REG_CFG_0,data,AP_REG_CFG_0_ADV_MASK));

    /* data to write to CFG1 register*/
    /*if needed need to add 100G_CR10*/
    data = (AP_CTRL_40GBase_CR4_GET(modeVector) << AP_REG_CFG_1_ADV_40G_CR4_SHIFT);
    CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                    AP_REG_CFG_1,data, AP_REG_CFG_1_ADV_MASK));

    /* data to write to CFG2 register*/
    data = (AP_CTRL_100GBase_KR4_GET(modeVector) << AP_REG_CFG_2_ADV_100G_KR4_SHIFT) |
           (AP_CTRL_100GBase_CR4_GET(modeVector) << AP_REG_CFG_2_ADV_100G_CR4_SHIFT) |
           (AP_CTRL_25GBase_KR1S_GET(modeVector) << AP_REG_CFG_2_ADV_25G_S_SHIFT) |
           (AP_CTRL_25GBase_KR1_GET(modeVector) << AP_REG_CFG_2_ADV_25G_SHIFT ) |
           (AP_CTRL_25GBase_CR1S_GET(modeVector) << AP_REG_CFG_2_ADV_25G_S_SHIFT) |
           (AP_CTRL_25GBase_CR1_GET(modeVector) << AP_REG_CFG_2_ADV_25G_SHIFT );
    CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                    AP_REG_CFG_2,data, AP_REG_CFG_2_ADV_MASK));

#if 0 /* code below  - for debug only*/

    GT_U32 data;

    hwsOsPrintf("HcdAdvertiseSet mode %x \n",modeVector);

    CHECK_STATUS(genUnitRegisterGet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                    AP_REG_CFG_0,&data, 0));

    hwsOsPrintf("HcdAdvertiseSet read cfg0 %x\n",data);
    CHECK_STATUS(genUnitRegisterGet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                    AP_REG_CFG_1,&data, 0));

    hwsOsPrintf("HcdAdvertiseSet read cfg1 %x\n",data);
    CHECK_STATUS(genUnitRegisterGet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                    AP_REG_CFG_2,&data, 0));

    hwsOsPrintf("HcdAdvertiseSet read cfg2 %x\n",data);
#endif
    /* clear symbol_lock bits */
    CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                    AP_REG_CFG_0, 0, AP_REG_CFG_0_LOCK_MASK));
    CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                    AP_REG_CFG_1, 0, AP_REG_CFG_1_LOCK_MASK));
    CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                     AP_REG_CFG_3, 0, AP_REG_CFG_3_LOCK_MASK));

    return GT_OK;
}



#if defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT)|| defined(ALDRIN2_DEV_SUPPORT)
/** mvApHcdAdvertiseNpConsortiumSet
*******************************************************************************/
GT_STATUS mvApHcdAdvertiseNpConsortiumSet(GT_U8 pcsNum, GT_U32 modeVector)
{

    GT_U32 data = ((AP_CTRL_ADV_50G_CONSORTIUM_GET(modeVector) << AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_50G_SHIFT)|
                   (AP_CTRL_ADV_25G_CONSORTIUM_GET(modeVector) << AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_25G_SHIFT));

    CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                    AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15,
                                    data ,
                                    (AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_25G_MASK | AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_50G_MASK)));
    AP_PRINT_MAC(("mvApHcdAdvertiseNpConsortiumSet  pcsNum:%d adr:0x%x data:0x%x, mask:0x%x\n",pcsNum, AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15, data, (AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_25G_MASK | AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_50G_MASK)));
    return GT_OK;

}
#endif
/** mvApHcdSendNpConsortium
*******************************************************************************/
GT_STATUS mvApHcdSendNpConsortium(GT_U8 pcsNum, GT_U32 msg, GT_U32 ack2, GT_U32 mp, GT_U32 ack, GT_U32 np)
{

    GT_U32 data = ((msg & AP_INT_REG_802_3_NEXT_PG_TX_REG_OUI_TAGGED_MASK) |
                   ((ack2 << AP_INT_REG_802_3_NEXT_PG_TX_REG_ACK2_SHIFT) & AP_INT_REG_802_3_NEXT_PG_TX_REG_ACK2_MASK)|
                   ((mp << AP_INT_REG_802_3_NEXT_PG_TX_REG_MP_SHIFT) & AP_INT_REG_802_3_NEXT_PG_TX_REG_MP_MASK)|
                   ((ack << AP_INT_REG_802_3_NEXT_PG_TX_REG_ACK_SHIFT) & AP_INT_REG_802_3_NEXT_PG_TX_REG_ACK_MASK)|
                   ((np << AP_INT_REG_802_3_NEXT_PG_TX_REG_NP_SHIFT) & AP_INT_REG_802_3_NEXT_PG_TX_REG_NP_MASK)) ;

    CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                    AP_INT_REG_802_3_NEXT_PG_TX_REG,
                                    data ,
                                    (AP_INT_REG_802_3_NEXT_PG_TX_REG_OUI_TAGGED_MASK | AP_INT_REG_802_3_NEXT_PG_TX_REG_CTRL_BIT_MASK)));
    AP_PRINT_MAC(("mvApHcdSendNpConsortium  pcsNum:%d adr:0x%x data:0x%x, mask:0x%x\n",pcsNum, AP_INT_REG_802_3_NEXT_PG_TX_REG, data, (AP_INT_REG_802_3_NEXT_PG_TX_REG_OUI_TAGGED_MASK | AP_INT_REG_802_3_NEXT_PG_TX_REG_CTRL_BIT_MASK)));
    return GT_OK;

}

/** mvApHcdSendNpNull
*******************************************************************************/
GT_STATUS mvApHcdSendNpNull(GT_U8 pcsNum)
{

    CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                    AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31,
                                    0,
                                    0));
    AP_PRINT_MAC(("mvApHcdSendNpNull  pcsNum:%d adr:0x%x data:0x%x, mask:0x%x\n",pcsNum, AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31, 0, (0)));

    CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                    AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15,
                                    0 ,
                                    0));
    AP_PRINT_MAC(("mvApHcdSendNpNull  pcsNum:%d adr:0x%x data:0x%x, mask:0x%x\n",pcsNum, AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15, 0, (0)));

    return mvApHcdSendNpConsortium(pcsNum, AP_INT_REG_802_3_NEXT_PG_TX_REG_OUI_TAGGED_FORM_NULL_CNS, 0, 1, 0, 0);
}

/** mvApHcdSendNpConsortiumStart
*******************************************************************************/
#if defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)
GT_STATUS mvApHcdSendNpConsortiumStart(GT_U8 pcsNum, GT_U8 ack2)
{

    CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                    AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31,
                                    AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_OUI_TAGGED_FORM_CNS,
                                    0));
    AP_PRINT_MAC(("mvApHcdSendNpConsortiumStart  pcsNum:%d adr:0x%x data:0x%x, mask:0x%x\n",pcsNum, AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31, AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_OUI_TAGGED_FORM_CNS, (0)));

    CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                    AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15,
                                    AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_OUI_TAGGED_FORM_CNS ,
                                    0));
    AP_PRINT_MAC(("mvApHcdSendNpConsortiumStart  pcsNum:%d adr:0x%x data:0x%x, mask:0x%x\n",pcsNum, AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15, AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_OUI_TAGGED_FORM_CNS, (0)));

    return mvApHcdSendNpConsortium(pcsNum, AP_INT_REG_802_3_NEXT_PG_TX_REG_OUI_TAGGED_FORM_CNS, ack2, 1, 0, 1);
}

/** mvApHcdReadPartnerNp
*******************************************************************************/
GT_STATUS mvApHcdReadPartnerNp(GT_U8 pcsNum, GT_U16 *np0, GT_U16 *np1, GT_U16 *np2)
{

    GT_UREG_DATA    reg0,reg1,reg2;
    CHECK_STATUS(genUnitRegisterGet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                    AP_INT_REG_802_3_LP_NEXT_PG_ABILITY_CODE_16_31,
                                    &reg2, 0));

    CHECK_STATUS(genUnitRegisterGet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                    AP_INT_REG_802_3_LP_NEXT_PG_ABILITY_CODE_0_15,
                                    &reg1, 0));

    CHECK_STATUS(genUnitRegisterGet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                    AP_INT_REG_802_3_LP_NEXT_PG_ABILITY_REG,
                                    &reg0, 0));
    *np0 = reg0;
    *np1 = reg1;
    *np2 = reg2;

    return GT_OK;
}

/** mvApHcdIsPartnerSupportConsortium
*******************************************************************************/
GT_BOOL mvApHcdIsPartnerSupportConsortium(GT_U16 np0, GT_U16 np1, GT_U16 np2)
{

    if ((np2 == AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_OUI_TAGGED_FORM_CNS) &&
        (np1 == AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_OUI_TAGGED_FORM_CNS)  &&
        ((np0 & (AP_INT_REG_802_3_NEXT_PG_TX_REG_MP_MASK | AP_INT_REG_802_3_NEXT_PG_TX_REG_NP_MASK | AP_INT_REG_802_3_NEXT_PG_TX_REG_OUI_TAGGED_MASK)) ==
            (AP_INT_REG_802_3_NEXT_PG_TX_REG_OUI_TAGGED_FORM_CNS | AP_INT_REG_802_3_NEXT_PG_TX_REG_MP_MASK | AP_INT_REG_802_3_NEXT_PG_TX_REG_NP_MASK))){
        return GT_TRUE;
    } else {
        return GT_FALSE;
    }
}
#endif
/** mvApFcResolutionSet
*******************************************************************************/
GT_STATUS mvApFcResolutionSet(GT_U8 portIndex)
{
    GT_U8  localFcEn;
    GT_U8  localFcAsm;
    GT_U8  peerFcEn;
    GT_U8  peerFcAsm;
    GT_U32 data;

    MV_HWS_AP_SM_INFO *apSm = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info);
    GT_U8 pcsNum = AP_CTRL_PCS_GET(apSm->ifNum);

    CHECK_STATUS(genUnitRegisterGet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                    AP_INT_REG_802_3_LP_BASE_ABILITY_REG_1, &data, 0));

    localFcEn  = (GT_U8)AP_CTRL_FC_PAUSE_GET(apSm->options);
    localFcAsm = (GT_U8)AP_CTRL_FC_ASM_GET(apSm->options);

    peerFcEn  = (GT_U8)((data >> AP_INT_REG_802_3_LP_BASE_ABILITY_REG_1_FC_PAUSE_SHIFT)& 0x1);
    peerFcAsm = (GT_U8)((data >> AP_INT_REG_802_3_LP_BASE_ABILITY_REG_1_FC_DIR_SHIFT)& 0x1);

    AP_CTRL_REM_FC_PAUSE_SET(apSm->options, peerFcEn);
    AP_CTRL_REM_FC_ASM_SET(apSm->options, peerFcAsm);

    if ((localFcEn & peerFcEn) ||
        (localFcAsm & peerFcEn & peerFcAsm))
    {
        AP_ST_HCD_FC_TX_RES_SET(apSm->hcdStatus, 1); /* Set FC Tx */
    }

    if ((localFcEn & peerFcEn) ||
        (localFcEn & localFcAsm & peerFcAsm))
    {
        AP_ST_HCD_FC_RX_RES_SET(apSm->hcdStatus, 1); /* Set FC Rx */
    }
    AP_PRINT_MAC(("mvApFcResolutionSet end port:%d options:0x%x\n", portIndex, apSm->options));

    return GT_OK;
}

#if defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)
/** mvApConsortiumResolutionSet
*******************************************************************************/
GT_STATUS mvApConsortiumResolutionSet(GT_U8 portIndex, GT_U16 np0, GT_U16 np1, GT_U16 np2)
{

    MV_HWS_AP_SM_INFO *apSm = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info);

    if ((AP_CTRL_50GBase_KR2_CONSORTIUM_GET(apSm->capability) == 1) &&
        ((np1 & AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_50G_KR2_MASK) == AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_50G_KR2_MASK)){
        /* 50G is selected */
        AP_ST_HCD_TYPE_SET(apSm->hcdStatus, Port_50GBASE_KR2_C); /* Set FC Tx */
        AP_PRINT_MAC(("Consortium partner resolution portIndex:%d  Port_50GBASE_KR2_CONSORTIUM\n", portIndex));

    }else if ((AP_CTRL_50GBase_CR2_CONSORTIUM_GET(apSm->capability) == 1) &&
        ((np1 & AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_50G_CR2_MASK) == AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_50G_CR2_MASK)){
        /* 50G is selected */
        AP_ST_HCD_TYPE_SET(apSm->hcdStatus, Port_50GBASE_CR2_C); /* Set FC Tx */
        AP_PRINT_MAC(("Consortium partner resolution portIndex:%d  Port_50GBASE_CR2_CONSORTIUM\n", portIndex));

    }else if ((AP_CTRL_25GBase_KR_CONSORTIUM_GET(apSm->capability) == 1) &&
        ((np1 & AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_25G_KR1_MASK) == AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_25G_KR1_MASK)){
        /* 50G is selected */
        AP_ST_HCD_TYPE_SET(apSm->hcdStatus, Port_25GBASE_KR_C); /* Set FC Tx */
        AP_PRINT_MAC(("Consortium partner resolution portIndex:%d  Port_25GBASE_KR1_CONSORTIUM\n", portIndex));

    }else if ((AP_CTRL_25GBase_CR_CONSORTIUM_GET(apSm->capability) == 1) &&
        ((np1 & AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_25G_CR1_MASK) == AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_25G_CR1_MASK)){
        /* 50G is selected */
        AP_ST_HCD_TYPE_SET(apSm->hcdStatus, Port_25GBASE_CR_C); /* Set FC Tx */
        AP_PRINT_MAC(("Consortium partner resolution portIndex:%d  Port_25GBASE_CR1_CONSORTIUM\n", portIndex));

    }else {
        /* there is no match in consortium */
        return GT_OK;
    }
    AP_ST_HCD_CONSORTIUM_RES_SET(apSm->hcdStatus, 1);
    /* either link partner requests RS-FEC  & both link partners support RS-FEC
      If ((LD.F3 | RD.F3 ) & (LD.F1 & RD.F1)) {
        Use RS-FEC ;
    } either link partner requests BASE-R FEC & both link partners support BASE-R-FEC
    Else if ((LD.F4 | RD.F4) & (LD.F2 & RD.F2)) {
        Use BASE-R-FEC
    }
    Else NO FEC;
    */

    if (((AP_CTRL_RS_FEC_REQ_CONSORTIUM_GET(apSm->options) == 1) ||
         ((np2 & AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_FEC_RS_REQ_MASK) == AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_FEC_RS_REQ_MASK))&&
        (AP_CTRL_RS_FEC_ABIL_CONSORTIUM_GET(apSm->options) == 1) &&
        ((np2 & AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_FEC_RS_ABIL_MASK) == AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_FEC_RS_ABIL_MASK)){
        /* RS FEC is selected */
        AP_ST_HCD_FEC_RES_SET(apSm->hcdStatus,AP_ST_HCD_FEC_RES_RS);
        AP_PRINT_MAC(("Consortium partner resolution portIndex:%d  AP_ST_HCD_FEC_RES_RS\n", portIndex));

    } else if (((AP_CTRL_FC_FEC_REQ_CONSORTIUM_GET(apSm->options) == 1) ||
                ((np2 & AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_FEC_FC_REQ_MASK) == AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_FEC_FC_REQ_MASK))&&
               (AP_CTRL_FC_FEC_ABIL_CONSORTIUM_GET(apSm->options) == 1) &&
               ((np2 & AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_FEC_FC_ABIL_MASK) == AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_FEC_FC_ABIL_MASK)){
        /* BASE-R FEC is selected */
        AP_ST_HCD_FEC_RES_SET(apSm->hcdStatus, AP_ST_HCD_FEC_RES_FC);
        AP_PRINT_MAC(("Consortium partner resolution portIndex:%d  AP_ST_HCD_FEC_RES_BASE_R\n", portIndex));

    } else {
        AP_PRINT_MAC(("Consortium partner resolution portIndex:%d  NO FEC\n", portIndex));
    }

    return GT_OK;
}
#endif
/*******************************************************************************
* mvApReset
* mvApAnEnable
* mvApAnRestart
* mvApStop
* mvApResetStatus
* mvApResetTimer
* mvApPortDelete
*
* DESCRIPTION: List of AP HW mechanisem API's
*******************************************************************************/

/** mvApReset
*******************************************************************************/
GT_STATUS mvApReset(GT_U8 pcsNum, GT_U8 enable)
{
    CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                    AP_INT_REG_802_3_AP_CTRL,
                                    (enable << AP_INT_REG_802_3_AP_CTRL_RST_SHIFT),
                                               AP_INT_REG_802_3_AP_CTRL_RST_MASK));

    return GT_OK;
}

/** mvApAnEnable
*******************************************************************************/
GT_STATUS mvApAnEnable(GT_U8 pcsNum, GT_U8 anEn)
{

    CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                    AP_INT_REG_802_3_AP_CTRL,
                                    (anEn << AP_INT_REG_802_3_AP_CTRL_AN_ENA_SHIFT),
                                             AP_INT_REG_802_3_AP_CTRL_AN_ENA_MASK));

    return GT_OK;
}


/** mvApAnRestart
*******************************************************************************/
GT_STATUS mvApAnRestart(GT_U8 pcsNum, GT_U8 reset)
{
    CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                    AP_INT_REG_802_3_AP_CTRL,
                                    (reset << AP_INT_REG_802_3_AP_CTRL_AN_RST_SHIFT),
                                              AP_INT_REG_802_3_AP_CTRL_AN_RST_MASK));

    return GT_OK;
}

/** mvApUnreset
*******************************************************************************/
GT_STATUS mvApUnreset(GT_U8 pcsNum, GT_BOOL unreset)
{
    CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                    PCS_CLOCK_RESET_REG,
                                    (unreset << PCS_CLOCK_RESET_REG_AP_RESET_SHIFT),
                                                PCS_CLOCK_RESET_REG_AP_RESET_MASK));

    return GT_OK;
}

/** mvApStop
*******************************************************************************/
void mvApStop(GT_U8 pcsNum)
{
    mvApAnEnable(pcsNum, 0);
    mvApArbFsmSet(pcsNum, ST_AN_ENABLE);
    mvApHcdAdvertiseSet(pcsNum, 0);
}

/** mvApResetStatus
*******************************************************************************/
void mvApResetStatus(GT_U8 portIndex)
{
    MV_HWS_AP_SM_INFO *apSm = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info);

    apSm->hcdStatus   = 0;
    apSm->status      = 0;
}

/** mvApResetTimer
*******************************************************************************/
void mvApResetTimer(GT_U8 portIndex)
{
    MV_HWS_AP_SM_TIMER *apTimer = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].timer);

    apTimer->txDisThreshold   = 0;
    apTimer->abilityThreshold = 0;
    apTimer->abilityCount     = 0;
    apTimer->linkThreshold    = 0;
    apTimer->linkCount        = 0;
}

/** mvApResetStats
*******************************************************************************/
void mvApResetStats(GT_U8 portIndex)
{
    MV_HWS_AP_SM_STATS *apStats = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].stats);

    apStats->txDisCnt          = 0;
    apStats->abilityCnt        = 0;
    apStats->abilitySuccessCnt = 0;
    apStats->linkFailCnt       = 0;
    apStats->linkSuccessCnt    = 0;
}

/** mvApResetStatsTimestamp
*******************************************************************************/
void mvApResetStatsTimestamp(GT_U8 portIndex)
{
    MV_HWS_AP_SM_STATS *apStats = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].stats);

    apStats->timestampTime     = 0;
    apStats->hcdResoultionTime = 0;
    apStats->linkUpTime        = 0;
}

/** mvApPortDelete
*******************************************************************************/
GT_STATUS mvApPortDelete(GT_U8 portIndex)
{
    GT_STATUS            rc;
    MV_HWS_PORT_STANDARD portMode;
    MV_HWA_AP_PORT_MODE  apPortMode;
    MV_HWS_AP_SM_INFO    *apSm = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info);

    apPortMode = AP_ST_HCD_TYPE_GET(apSm->hcdStatus);

    portMode = mvHwsApConvertPortMode(apPortMode);
    if(portMode == NON_SUP_MODE)
    {
        return GT_NOT_SUPPORTED;
    }

    /* Disabling MAC before continue with port delete operations in order to provide maximal
       time between MAC disable to the potential MAC enable, in which at that time resources
       of that port are being freed */
    rc = mvHwsPortEnableSet(0/*dev*/, 0/*portGroup*/, apSm->portNum,
                               portMode, GT_FALSE);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* TBD - move StopAdative to Trainng task (potential bug - ectivate stopAdaptive while training)*/
#if !defined(BC2_DEV_SUPPORT) && !defined(AC3_DEV_SUPPORT)
    /* Stop RX Adaptive training */
    rc = mvHwsPortAutoTuneSet(0/*dev*/, 0/*portGroup*/, apSm->portNum,
                                 portMode, RxTrainingStopAdative, NULL);
    if (rc != GT_OK)
    {
        return rc;
    }
#endif

    /* Send port delete message */
    rc = mvPortCtrlApPortMsgSend(portIndex,
                                (GT_U8)MV_HWS_IPC_PORT_RESET_MSG,
                                (GT_U8)AP_CTRL_QUEUEID_GET(apSm->ifNum)  /*apSm->queueId*/,
                                (GT_U8)apSm->portNum,
                                (GT_U8)portMode,
                                (GT_U8)PORT_POWER_DOWN,
                                0, 0);
    if (rc != GT_OK)
    {
        mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state,AP_PORT_MSG_SEND_FAILURE,apSm->portNum,MV_HWS_IPC_PORT_RESET_MSG)); /*trace only - AP status is not changed*/
        AP_PRINT_MAC(("mvApPortDelete port:%d  mvPortCtrlApPortMsgSend fail\n", apSm->portNum));
    }

    return GT_OK;
}

#if defined(BC3_DEV_SUPPORT) || defined(BOBK_DEV_SUPPORT) || defined(ALDRIN_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)
/** mvApCtleBiasDbUpdate
*******************************************************************************/
void mvApCtleBiasDbUpdate(GT_U8 portIndex)
{
    MV_HWS_AP_SM_INFO *apSm = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info);
    GT_U8 ctleBitMapIndex;

#ifdef BOBK_DEV_SUPPORT
    if (portIndex < 48)
    {
        portIndex /= 4;
    }
#endif

    ctleBitMapIndex = portIndex/HWS_MAX_CTLE_DB_SIZE;

    if (AP_CTRL_CTLE_BIAS_VAL_GET(apSm->options))
    {
        hwsDeviceSpecInfo[0].serdesInfo.ctleBiasValue[ctleBitMapIndex] |= (0x1 << (portIndex%HWS_MAX_CTLE_DB_SIZE));
    }
    else
    {
        hwsDeviceSpecInfo[0].serdesInfo.ctleBiasValue[ctleBitMapIndex] &= ~(0x1 << (portIndex%HWS_MAX_CTLE_DB_SIZE));
    }
}
#endif

/*******************************************************************************
*                            AP Port Init State                                *
* ============================================================================ *
*******************************************************************************/

/**
* @internal mvHwsInitialConfiguration function
* @endinternal
*
*/
GT_STATUS mvHwsInitialConfiguration(GT_U8 portIndex)
{
    MV_HWS_AP_SM_STATS *apStats = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].stats);
    MV_HWS_AP_SM_INFO  *apSm    = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info);

    /* This bit selects between the GbE MAC to the other optional MACmodes.
       Default is GbE MAC. */
    CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, XLGMAC_UNIT,
                                    AP_CTRL_MAC_GET(apSm->ifNum), 0x84, 0, (1 << 12)));

#if defined(BC3_DEV_SUPPORT) || defined(BOBK_DEV_SUPPORT) || defined(ALDRIN_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)
    if(AP_CTRL_PCS_GET(apSm->ifNum) != AP_CTRL_MAC_GET(apSm->ifNum))
    {/* if for negotiation defined not first lane of port this configuration required
        for its MAC too */
        CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, XLGMAC_UNIT,
                                        AP_CTRL_PCS_GET(apSm->ifNum), 0x84, 0, (1 << 12)));
    }
#endif

    /* reset timestamps */
    mvApResetStatsTimestamp(portIndex);
    apStats->timestampTime = mvPortCtrlCurrentTs();

    return GT_OK;
}

#ifdef ALDRIN_DEV_SUPPORT
/** mvApRefClockUpdate
************************************************/
GT_STATUS mvApRefClockUpdate
(
    GT_U8                       refClockSrcFromDb,
    GT_U8                       validFromDb,
    GT_U8                       cpllOutFreqFromDb,
    MV_HWS_REF_CLOCK_SUP_VAL    *refClockPtr
)
{
    MV_HWS_CPLL_OUTPUT_FREQUENCY    cpllOutFreq;
    MV_HWS_REF_CLOCK_SOURCE         refClockSource;


    cpllOutFreq = cpllOutFreqFromDb;
    refClockSource = refClockSrcFromDb;

    if (Aldrin == hwsDeviceSpecInfo[0].devType)
    {
        if((refClockSource == SECONDARY_LINE_SRC) && (validFromDb == GT_TRUE))
        {   /* CPLL as ref clock source */
            switch(cpllOutFreq)
            {
                case MV_HWS_156MHz_OUT:
                    *refClockPtr = MHz_156;
                    break;

                case MV_HWS_78MHz_OUT:
                    *refClockPtr = MHz_78;
                    break;

                default:
                    mvPcPrintf("GT_BAD_STATE\n");
                    return GT_BAD_STATE;
            }
        }
        else if (refClockSource == PRIMARY_LINE_SRC)
        {
            *refClockPtr = MHz_156;
        }
    }

    return GT_OK;
}
#endif

/** mvApSerdesPowerUp
*******************************************************************************/
GT_STATUS mvApSerdesPowerUp(GT_U8 serdesNum, GT_BOOL powerUp, GT_U8 portNum)
{
    GT_STATUS status;
    MV_HWS_SERDES_CONFIG_STR    serdesConfig;

#ifdef ALDRIN_DEV_SUPPORT
    MV_HWS_REF_CLOCK_SUP_VAL    refClock;
    GT_U8 isValid;
    GT_U8 cpllOutFreq;
    GT_U8 refClockSrc;
    isValid = refClockSourceDb[portNum] & 0x1;
    cpllOutFreq = (refClockSourceDb[portNum] >> 1) & 0x7;
    refClockSrc = (refClockSourceDb[portNum] >> 4) & 0xF;
    serdesConfig.refClockSource = refClockSrc;
#elif defined(BC3_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)
    portNum = portNum; /* avoid warning */
    serdesConfig.refClock = (refClockSourceDb[portNum] >> 5) & 0x7;
    serdesConfig.refClockSource = (refClockSourceDb[portNum] >> 4) & 0x1;
#else
    portNum = portNum; /* avoid warning */
    serdesConfig.refClock = _156dot25Mhz;
    serdesConfig.refClockSource = PRIMARY;
#endif

    serdesConfig.baudRate = _3_125G;
    serdesConfig.media = XAUI_MEDIA;
    serdesConfig.busWidth = _10BIT_OFF;
    serdesConfig.rxEncoding = SERDES_ENCODING_NA;
    serdesConfig.serdesType = (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(apDevNum));
    serdesConfig.txEncoding = SERDES_ENCODING_NA;

#ifdef ALDRIN_DEV_SUPPORT
    status = mvApRefClockUpdate(refClockSrc,isValid,cpllOutFreq,&refClock);
    if (status != GT_OK)
    {
        mvPcPrintf("Error, mvApRefClockUpdate failed, status : %d ", status);
        return status;
    }
    serdesConfig.refClock = (MV_HWS_REF_CLOCK)refClock;
#endif

    status = mvHwsSerdesPowerCtrl(apDevNum,             /* devNum         */
                                  apPortGroup,          /* portGroup      */
                                  serdesNum,            /* serdesNum      */
                                  powerUp,              /* powerUp        */
                                  &serdesConfig);

    return status;
}

/**
* @internal mvApPortInit function
* @endinternal
*
* @brief   AP Port init execution sequence
*/
GT_STATUS mvApPortInit(GT_U8 portIndex)
{
    MV_HWS_AP_SM_INFO  *apSm    = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info);
    MV_HWS_AP_SM_STATS *apStats = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].stats);

    GT_U8 pcsNum = AP_CTRL_PCS_GET(apSm->ifNum);
    GT_U8 laneNum = AP_CTRL_LANE_GET(apSm->ifNum);

    mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));

    mvApResetStatus(portIndex);
    mvApResetTimer(portIndex);
    mvApSerdesPowerUp(laneNum, GT_FALSE, apSm->portNum);
    mvApPcsMuxReset(pcsNum);
#if defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)
    /*hwsOsTimerWkFuncPtr(5);*/
    mvApConfig(pcsNum, 0);
#endif

#if defined(BC3_DEV_SUPPORT) || defined(BOBK_DEV_SUPPORT) || defined(ALDRIN_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)
        mvApCtleBiasDbUpdate(portIndex);
#endif
    /* Lane power up in 3.125 */
    if (mvApSerdesPowerUp(laneNum, GT_TRUE, apSm->portNum) != GT_OK)
    {
        /* if Serdes power-up failed, power it down, exit and next iteration
        ** the Serdes will be powered-up again
        */
        apSm->status = AP_PORT_SERDES_INIT_FAILURE;
        mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));

        mvApSerdesPowerUp(laneNum, GT_FALSE, apSm->portNum);
        return GT_OK;
    }

    /* PCS MUX set to AP */
    mvApPcsMuxSet(pcsNum, 3);

#if defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)
    /* after talking with design team we agreed that delay is needed */
    hwsOsTimerWkFuncPtr(5);
    mvApConfig(pcsNum, 3);
#endif

    mvApUnreset(pcsNum, GT_TRUE);
    /* AP Enable */
    mvApAnEnable(pcsNum, 1);

    /* AP ARB state machine - AN_ENABLE */
    mvApArbFsmSet(pcsNum, ST_AN_ENABLE);
    AP_ST_ARB_FSM_SET(apSm->ARMSmStatus, ST_AN_ENABLE);

    /* AP state & status update */
    apSm->status = AP_PORT_INIT_SUCCESS;
    mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));

    apSm->state = AP_PORT_SM_TX_DISABLE_STATE;

    /* AP statistics update */
    apStats->abilityInvalidCnt = 0;

    /* AP register dump */
    mvPortCtrlDbgAllRegsDump(portIndex, pcsNum, AP_PORT_SM_INIT_STATE);

    return GT_OK;
}

/*******************************************************************************
*                            AP Port Tx Disable State                          *
* ============================================================================ *
*******************************************************************************/

/**
* @internal mvApParallelDetectModeSet function
* @endinternal
*
* @brief   Check if KX4(XAUI)/1G are advertised
*         if not, close parallel detect for this port.
*         This is done by enabling overwrite bit
*         (bits [6] or [8] and closing parallel detect (bits [7] or [9])
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvApParallelDetectModeSet(GT_U8 portIndex)
{
    GT_U32 data = 0;

    MV_HWS_AP_SM_INFO *apSm = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info);
    GT_U8 pcsNum = AP_CTRL_PCS_GET(apSm->ifNum);

    /* Enable Overwrite and disable parallel detect for XAUI */
    if(AP_CTRL_10GBase_KX4_GET(apSm->capability) == 0)
    {
        data |= (1 << 6);
    }

    /* Enable Overwrite and disable parallel detect for 1G */
    if(AP_CTRL_1000Base_KX_GET(apSm->capability) == 0)
    {
        data |= (1 << 8);
    }

    CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                    AP_INT_REG_ANEG_CTRL_1, data,
                                    AP_INT_REG_ANEG_CTRL_1_PD_CAP_MASK));

    return GT_OK;
}

/** mvApPreCfgLoopback
*******************************************************************************/
GT_STATUS mvApPreCfgLoopback(GT_U8 pcsNum, GT_U8 lbEn)
{
    CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                    AP_INT_REG_ANEG_CTRL_1,
                                    (lbEn << AP_INT_REG_ANEG_CTRL_1_LB_NONCE_MATCH_SHIFT),
                                             AP_INT_REG_ANEG_CTRL_1_LB_NONCE_MATCH_MASK));

    return GT_OK;
}

/**
* @internal mvApPortTxDisable function
* @endinternal
*
* @brief   AP Port Tx Disable execution sequence
*/
GT_STATUS mvApPortTxDisable(GT_U8 portIndex)
{
    GT_U8 laneNum;
    GT_U8 pcsNum;
    MV_HWS_AP_SM_INFO       *apSm     = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info);
    MV_HWS_AP_SM_TIMER      *apTimer  = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].timer);
    MV_HWS_AP_SM_STATS      *apStats  = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].stats);
    MV_HWS_AP_DETECT_ITEROP *apIntrop = &(mvHwsPortCtrlApPortDetect.introp);
    GT_U8 devNum = 0;

    /* Extract PCS and Lane number */
    pcsNum  = AP_CTRL_PCS_GET(apSm->ifNum);
    laneNum = AP_CTRL_LANE_GET(apSm->ifNum);

    if (apSm->status != AP_PORT_TX_DISABLE_IN_PROGRESS)
    {
        /* SERDES Tx Disable */
        mvHwsSerdesTxEnable(apDevNum, apDevNum, laneNum, HWS_DEV_SERDES_TYPE(devNum), GT_FALSE);

        /* AP ARB state machine - TX_DISABLE */
        mvApArbFsmSet(pcsNum, ST_TX_DISABLE);
        AP_ST_ARB_FSM_SET(apSm->ARMSmStatus, ST_TX_DISABLE);

        /* Reload advertisement - Configure HW with HCD parameters from AP control */
        mvApHcdAdvertiseSet(pcsNum, AP_CTRL_ADV_ALL_GET(apSm->capability));

        mvApParallelDetectModeSet(portIndex);

        /* Config FC and FEC */
        mvApHcdFcParamsSet(pcsNum, (GT_U8)AP_CTRL_FC_PAUSE_GET(apSm->options),
                                   (GT_U8)AP_CTRL_FC_ASM_GET(apSm->options));

        mvApHcdFecParamsSet(pcsNum, (GT_U8)AP_CTRL_FEC_ABIL_GET(apSm->options),
                                    (GT_U8)AP_CTRL_FEC_REQ_GET(apSm->options));

        mvApHcdFecAdvancedParamsSet(pcsNum, (GT_U8)((AP_CTRL_FEC_FC_REQ_GET(apSm->options)<<1)
                                                    | AP_CTRL_FEC_RS_REQ_GET(apSm->options)));

        if (AP_CTRL_ADV_CONSORTIUM_GET(apSm->capability)) {
            AP_PRINT_MAC(("set consortium portIndex:%d advertise capability:0x%x\n",portIndex, apSm->capability));
            mvApHcdParamsNpSet(pcsNum, 1);
        }
        /* Config loopback if needed */
        if (AP_CTRL_LB_EN_GET(apSm->options))
        {
            mvApPreCfgLoopback(pcsNum, 1);
        }

        /* AP state & status update */
        apSm->status = AP_PORT_TX_DISABLE_IN_PROGRESS;
        mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));

        /* AP statistics update */
        apStats->txDisCnt++;

        /* Tx Disable timer start */
        mvPortCtrlThresholdSet(apIntrop->txDisDuration, &(apTimer->txDisThreshold));
    }
    else if (apSm->status == AP_PORT_TX_DISABLE_IN_PROGRESS)
    {
        /* Check Tx Disable threshold */
        if (mvPortCtrlThresholdCheck(apTimer->txDisThreshold))
        {
            /* clear Tx Disable */
            mvHwsSerdesTxEnable(apDevNum, apDevNum, laneNum, HWS_DEV_SERDES_TYPE(devNum), GT_TRUE);

            /* AP state & status update */
            apSm->status = AP_PORT_TX_DISABLE_SUCCESS;
            AP_PRINT_MAC(("TxDisable portIndex:%d status:0x%x\n",portIndex, apSm->status));
            mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));

            mvApArbFsmSet(pcsNum, ST_ABILITY_DET);
            AP_ST_ARB_FSM_SET(apSm->ARMSmStatus, ST_ABILITY_DET);
            mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));

            apSm->state = AP_PORT_SM_RESOLUTION_STATE;

            /* Ability timer start */
            mvPortCtrlThresholdSet(apIntrop->abilityDuration, &(apTimer->abilityThreshold));

            /* AP register dump */
            mvPortCtrlDbgAllRegsDump(portIndex, pcsNum, AP_PORT_SM_TX_DISABLE_STATE);
        }
    }

    return GT_OK;
}

/*******************************************************************************
*                            AP Port Resolution State                          *
* ============================================================================ *
*******************************************************************************/

/**
* @internal mvApPortResolutionSuccess function
* @endinternal
*
* @brief   AP Port Resolution - AP execution sequence
*         Read the power-up bits in AP status reg 0
*         As certain that exactly one of them is active
*/
void mvApPortResolutionSuccess(GT_U8 portIndex, GT_U8 mode)
{
    MV_HWS_PORT_STANDARD portMode;
    MV_HWA_AP_PORT_MODE apPortMode;
    MV_HWS_AP_SM_INFO       *apSm     = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info);
    MV_HWS_AP_SM_TIMER      *apTimer  = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].timer);
    MV_HWS_AP_DETECT_ITEROP *apIntrop = &(mvHwsPortCtrlApPortDetect.introp);
    GT_U8 pcsNum = AP_CTRL_PCS_GET(apSm->ifNum);
    GT_STATUS rc;
#ifdef ALDRIN_DEV_SUPPORT
    MV_HWS_REF_CLOCK_SUP_VAL    refClock;
    GT_U8 isValid;
    GT_U8 cpllOutFreq;
    GT_U8 refClockSrc;

    isValid = refClockSourceDb[apSm->portNum] & 0x1;
    cpllOutFreq = (refClockSourceDb[apSm->portNum] >> 1) & 0x7;
    refClockSrc = (refClockSourceDb[apSm->portNum] >> 4) & 0xF;
#elif defined(BC3_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)
    GT_U8 refClockSrc = (refClockSourceDb[apSm->portNum] >> 4) & 0x1;
    MV_HWS_REF_CLOCK_SUP_VAL    refClock = (refClockSourceDb[apSm->portNum] >> 5) & 0x7;
#endif


    /* PCS MUX set according to HCD resolution */
    apPortMode = AP_ST_HCD_TYPE_GET(apSm->hcdStatus);

    /* here no matter which mode really needed, it will be configured farther
        during port init (also in some scenarios configuration of port mode here
        causes troubles) - here important to move port from AP mode single lane mode */
    mvApPcsMuxSet(pcsNum, 0);

#if defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)
    mvApConfig(pcsNum, 0);
#endif

    /* AP Port mode parameters update */
    if(mvHwsApSetPortParameters(apSm->portNum, apPortMode) != GT_OK)
    {
        mvPcPrintf("Error, AP , Port-%d Params set Failed\n", apSm->portNum);
        return;
    }

    portMode = mvHwsApConvertPortMode(apPortMode);
    AP_PRINT_MAC(("mvApPortResolutionSuccess portIndex:%d  apPortMode:0x%x portMode:0x%x\n",portIndex, apPortMode, portMode));
#ifdef ALDRIN_DEV_SUPPORT
    rc = mvApRefClockUpdate(refClockSrc,isValid,cpllOutFreq, &refClock);
    if (rc != GT_OK)
    {
        mvPcPrintf("Error, mvApRefClockUpdate failed, rc: %d ", rc);
        return;
    }
#endif
    /* Send port init message */
    rc = mvPortCtrlApPortMsgSend(portIndex,
                                (GT_U8)MV_HWS_IPC_PORT_INIT_MSG,
                                (GT_U8)AP_CTRL_QUEUEID_GET(apSm->ifNum)/*apSm->queueId*/,
                                (GT_U8)apSm->portNum,
                                (GT_U8)portMode,
                                (GT_U8)GT_FALSE,
#if defined(BC3_DEV_SUPPORT) || defined(ALDRIN_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)
                                (GT_U8)refClock,
                                (GT_U8)refClockSrc);
#else
                                (GT_U8)MHz_156,
                                (GT_U8)PRIMARY_LINE_SRC);
#endif

    if (rc != GT_OK)
    {
        mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state,AP_PORT_MSG_SEND_FAILURE,apSm->portNum,MV_HWS_IPC_PORT_INIT_MSG)); /*trace only - AP status is not changed*/
        AP_PRINT_MAC(("mvApPortResolutionSuccess port:%d  mvPortCtrlApPortMsgSend fail\n", apSm->portNum));
    }

    if (mode == PORT_CTRL_AP_LINK_CHECK_MODE)
    {
        /* Link check timer start - AP */
        mvPortCtrlThresholdSet(apIntrop->apLinkDuration, &(apTimer->linkThreshold));
    }
    else
    {
        /* Link check timer start - Parallel Detect */
        mvPortCtrlThresholdSet(apIntrop->pdLinkDuration, &(apTimer->linkThreshold));
    }
    /*disable port MAC to prevent traffic during link establish*/
    mvHwsPortEnableSet(0/*dev*/, 0/*portGroup*/, (GT_U8)apSm->portNum, portMode, GT_FALSE);

    /* AP state & status update */
    apSm->status = AP_PORT_LINK_CHECK_START;
    mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));

    apSm->state = AP_PORT_SM_ACTIVE_STATE;

#if defined (MICRO_INIT) && defined (PIPE_DEV_SUPPORT)
    {
        extern MV_U32 FW_SDK_stack_type;
        extern void unq_check_assert_int(void);

        if (FW_SDK_stack_type == 0 /* STACK_SA */)
            apDoorbellIntrAddEvent(AP_DOORBELL_EVENT_PORT_802_3_AP(apSm->portNum));
        else        /* FW SDK has it's own interrupt mechanism */
            unq_check_assert_int(); /* Interrupt is triggered: doorbell - in case device is FW SDK master
                                                               remote   - on case device is FW SDK slave */
    }
#else
    apDoorbellIntrAddEvent(AP_DOORBELL_EVENT_PORT_802_3_AP(apSm->portNum));
#endif
}

/**
* @internal mvApPortResolutionParallelDetect function
* @endinternal
*
* @brief   AP Port Resolution - Parallel Detect execution sequence
*         Read the power-up bits in AP status reg 0 (MPCS regfile).
*         As certain that exactly one of them is active.
*         resolution is done in ST_AN_GOOD_CK state so we need to
*         update state machine
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvApPortResolutionParallelDetect(GT_U8 portIndex)
{
    GT_U8 pcsNum;
    GT_U32 plData;

    MV_HWS_AP_SM_INFO *apSm = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info);
    MV_HWS_AP_SM_STATS *apStats  = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].stats);

    pcsNum = AP_CTRL_PCS_GET(apSm->ifNum);

    /* AP ARB state machine - ST_AN_GOOD_CK */
    mvApArbFsmSet(pcsNum, ST_AN_GOOD_CK);
    AP_ST_ARB_FSM_SET(apSm->ARMSmStatus, ST_AN_GOOD_CK);

    /* AP state & status update */
    apSm->status = AP_PORT_PD_RESOLUTION_IN_PROGRESS;
    mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));

    CHECK_STATUS(genUnitRegisterGet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                    AP_REG_ST_0, &plData, 0));

    /* Activate the matching symbol-lock bit in AP configuration reg 0/1 */
    switch (plData & AP_REG_ST_0_AP_PWT_UP_MASK)
    {
    case AP_REG_ST_0_AP_PWT_UP_1G_KX_MASK:
        AP_ST_HCD_TYPE_SET(apSm->hcdStatus, Port_1000Base_KX);
        break;

    case AP_REG_ST_0_AP_PWT_UP_10G_KX4_MASK: /* 10G XAUI */
        AP_ST_HCD_TYPE_SET(apSm->hcdStatus, Port_10GBase_KX4);
        break;

    default:
        apSm->status = AP_PORT_PD_RESOLUTION_FAILURE;
        mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));

        apSm->state = AP_PORT_SM_INIT_STATE;
        return GT_OK;
    }

    AP_ST_HCD_FOUND_SET(apSm->hcdStatus, 1);
    mvPortCtrlLogAdd(AP_PORT_DET_LOG_EXT(apSm->hcdStatus, apSm->portNum));

    /* AP ARB state machine - ST_AN_GOOD_CK */
    mvApArbFsmSet(pcsNum, ST_AN_GOOD_CK);
    AP_ST_ARB_FSM_SET(apSm->ARMSmStatus, ST_AN_GOOD_CK);

    /* AP state & status update */
    apSm->status = AP_PORT_RESOLUTION_SUCCESS;
    mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));

    /* AP Resolution Success API */
    mvApPortResolutionSuccess(portIndex, PORT_CTRL_PD_LINK_CHECK_MODE);

    /* AP Timing measurment */
    apStats->hcdResoultionTime = mvPortCtrlCurrentTs() - apStats->timestampTime;
    apStats->timestampTime = mvPortCtrlCurrentTs();

    /* AP statistics update */
    apStats->abilitySuccessCnt++;

    return GT_OK;
}

/**
* @internal mvApPortResolutionAP function
* @endinternal
*
* @brief   AP Port Resolution - AP execution sequence
*         Read the power-up bits in AP status reg 0
*         As certain that exactly one of them is active
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvApPortResolutionAP(GT_U8 portIndex)
{
    GT_U32 plData, plData1;
    GT_U32 data;
    GT_U32 hcdType = MODE_NOT_SUPPORTED;
    GT_U8  pcsNum;
    GT_U8  priority = 0;
    GT_U8  fecType = AP_ST_HCD_FEC_RES_NONE;
    GT_BOOL found = GT_TRUE;
    MV_HWS_AP_SM_INFO       *apSm     = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info);
    MV_HWS_AP_SM_STATS      *apStats  = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].stats);
    MV_HWS_AP_DETECT_ITEROP *apIntrop = &(mvHwsPortCtrlApPortDetect.introp);
#if defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)
    GT_U32 hcdTypeConsortium = Port_AP_LAST;
    GT_U16 nextPage0, nextPage1, nextPage2;
    GT_U8  consortiumPrio;
#endif

    pcsNum = AP_CTRL_PCS_GET(apSm->ifNum);

#if defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)
    if (apSm->status == AP_PORT_AP_RESOLUTION_NO_MORE_PAGES){
        /* read partner next page */
        mvApHcdReadPartnerNp(pcsNum, &nextPage0, &nextPage1, &nextPage2);
        AP_PRINT_MAC(("mvApPortResolutionAP portIndex:%d  next page:0x%x, 0x%x, 0x%x\n",portIndex, nextPage0, nextPage1, nextPage2));
        /* check partner abilities */
        if (((nextPage0 & AP_INT_REG_802_3_NEXT_PG_TX_REG_OUI_TAGGED_MASK) == AP_INT_REG_802_3_NEXT_PG_TX_REG_OUI_TAGGED_UNFORM_CNS) &&
            ((nextPage0 & AP_INT_REG_802_3_NEXT_PG_TX_REG_MP_MASK) == 0)){
            /* read pertner fec & speed*/
            mvApConsortiumResolutionSet(portIndex, nextPage0, nextPage1, nextPage2);
        }
    }
#endif

    /* AP state & status update */
    apSm->status = AP_PORT_AP_RESOLUTION_IN_PROGRESS;
    AP_ST_ARB_FSM_SET(apSm->ARMSmStatus, ST_AN_GOOD_CK);
    mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));
    mvPortCtrlDbgOnDemandRegsDump(portIndex, pcsNum, apSm->state);

#if 0 /*code below - trace for debug only*/
    genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_REG_CFG_0 , &plData, 0);
    hwsOsPrintf("%d AP_REG_CFG_0  =0x%x\n",portIndex,plData);
    genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_REG_CFG_1 , &plData, 0);
    hwsOsPrintf("%d AP_REG_CFG_1  =0x%x\n",portIndex,plData);
    genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_REG_CFG_2 , &plData, 0);
    hwsOsPrintf("%d AP_REG_CFG_2  =0x%x\n",portIndex,plData);
    genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_REG_CFG_3 , &plData, 0);
    hwsOsPrintf("%d AP_REG_CFG_3  =0x%x\n",portIndex,plData);
    genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_INT_REG_802_3_AP_ADV_REG_1, &plData, 0);
    hwsOsPrintf("%d advReg1 =0x%x\n",portIndex,plData);
    genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_INT_REG_802_3_AP_ADV_REG_2, &plData, 0);
    hwsOsPrintf("%d advReg2 =0x%x\n",portIndex,plData);
    genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_INT_REG_802_3_AP_ADV_REG_3, &plData, 0);
    hwsOsPrintf("%d advReg3 =0x%x\n",portIndex,plData);
    genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_INT_REG_802_3_LP_BASE_ABILITY_REG_1, &plData, 0);
    hwsOsPrintf("%d alpReg1 =0x%x\n",portIndex,plData);
    genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_INT_REG_802_3_LP_BASE_ABILITY_REG_2, &plData, 0);
    hwsOsPrintf("%d alpReg2 =0x%x\n",portIndex,plData);
    genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_INT_REG_802_3_LP_BASE_ABILITY_REG_3, &plData, 0);
    hwsOsPrintf("%d alpReg3 =0x%x\n",portIndex,plData);

    /* Activate the matching symbol-lock bit in AP configuration */
    CHECK_STATUS(genUnitRegisterGet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                    AP_REG_ST_0, &plData, 0));
    hwsOsPrintf("%d plData0 =0x%x\n",portIndex,plData);
    CHECK_STATUS(genUnitRegisterGet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                        AP_REG_ST_1, &plData1, 0));
    hwsOsPrintf("%d plData1 =0x%x\n",portIndex,plData1);

#else

    /* Activate the matching symbol-lock bit in AP configuration */
    CHECK_STATUS(genUnitRegisterGet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                    AP_REG_ST_0, &plData, 0));

    CHECK_STATUS(genUnitRegisterGet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                        AP_REG_ST_1, &plData1, 0));
#endif

    /* HCD resolved bit[13] validate */
    if ((plData >> AP_REG_ST_0_HCD_RESOLVED_SHIFT) &
                   AP_REG_ST_0_HCD_RESOLVED_MASK)
    {
        fecType = (plData >> AP_REG_ST_0_HCD_BASE_R_FEC_SHIFT) & AP_REG_ST_0_HCD_BASE_R_FEC_MASK;
        if (plData1 & AP_REG_ST_1_AP_PWT_UP_MASK)
        {
            if((plData1 & AP_REG_ST_1_AP_PWT_UP_MASK) && (plData & AP_REG_ST_0_AP_PWT_UP_MASK))
            {
                /*Add error handle*/
                hwsOsPrintf("Found 2 hcd\n");
            }
            /* Activate the matching symbol-lock bit in AP configuration */
            switch (plData1 & AP_REG_ST_1_AP_PWT_UP_MASK)
            {
            case AP_REG_ST_1_AP_PWT_UP_100G_CR4_MASK:
                CHECK_STATUS(genUnitRegisterGet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                                AP_REG_CFG_3, &data, 0));
                CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                                AP_REG_CFG_3, data | AP_REG_CFG_3_LOCK_100G_CR4_MASK, 0));
                hcdType = Port_100GBASE_CR4;
                fecType = (fecType & AP_REG_ST_0_HCD_BASE_R_FEC_MASK) ?
                                    AP_ST_HCD_FEC_RES_RS : AP_ST_HCD_FEC_RES_NONE;
                priority = 100;
                break;

            case AP_REG_ST_1_AP_PWT_UP_100G_KR4_MASK:
                CHECK_STATUS(genUnitRegisterGet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                                AP_REG_CFG_3, &data, 0));
                CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                                AP_REG_CFG_3, data | AP_REG_CFG_3_LOCK_100G_KR4_MASK, 0));
                hcdType = Port_100GBASE_KR4;
                fecType = (fecType & AP_REG_ST_0_HCD_BASE_R_FEC_MASK) ?
                                    AP_ST_HCD_FEC_RES_RS : AP_ST_HCD_FEC_RES_NONE;
                priority = 100;
                break;

            case AP_REG_ST_1_AP_PWT_UP_100G_KP4_MASK:
                CHECK_STATUS(genUnitRegisterGet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                                AP_REG_CFG_3, &data, 0));
                CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                                AP_REG_CFG_3, data | AP_REG_CFG_3_LOCK_100G_KP4_MASK, 0));
                hcdType = Port_100GBASE_KP4;
                fecType = (fecType & AP_REG_ST_0_HCD_BASE_R_FEC_MASK) ?
                                    AP_ST_HCD_FEC_RES_RS : AP_ST_HCD_FEC_RES_NONE;
                priority = 100;
                break;

            case AP_REG_ST_1_AP_PWT_UP_25G_MASK:
                CHECK_STATUS(genUnitRegisterGet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                                AP_REG_CFG_3, &data, 0));
                CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                                    AP_REG_CFG_3, data | AP_REG_CFG_3_LOCK_25G_MASK, 0));
                if(AP_CTRL_25GBase_KR1_GET(apSm->capability) == 1)
                {
                    hcdType = Port_25GBASE_KR;
                }
                else
                {
                    hcdType = Port_25GBASE_CR;
                }
                priority = 25;
                if((plData1 >> AP_REG_ST_1_HCD_RS_FEC_SHIFT) & AP_REG_ST_1_HCD_RS_FEC_MASK)
                {
                    fecType = AP_ST_HCD_FEC_RES_RS;
                }
                break;

            case AP_REG_ST_1_AP_PWT_UP_25G_S_MASK:
                CHECK_STATUS(genUnitRegisterGet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                                AP_REG_CFG_3, &data, 0));
                CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                                AP_REG_CFG_3, data | AP_REG_CFG_3_LOCK_25G_S_MASK, 0));
                if(AP_CTRL_25GBase_KR1S_GET(apSm->capability) == 1)
                {
                    hcdType = Port_25GBASE_KR_S;
                }
                else
                {
                    hcdType = Port_25GBASE_CR_S;
                }
                break;
            default:
                    found = GT_FALSE;
                    break;
            }
        } else if (plData & AP_REG_ST_0_AP_PWT_UP_MASK)
        {
            switch (plData & AP_REG_ST_0_AP_PWT_UP_MASK)
            {
            case AP_REG_ST_0_AP_PWT_UP_40G_KR4_MASK:
                CHECK_STATUS(genUnitRegisterGet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                                AP_REG_CFG_0, &data, 0));
                CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                                AP_REG_CFG_0, data | AP_REG_CFG_0_LOCK_40G_KR4_MASK, 0));
                hcdType = Port_40GBase_R;
                priority = 40;
                break;

            case AP_REG_ST_0_AP_PWT_UP_10G_KR_MASK:
                CHECK_STATUS(genUnitRegisterGet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                                AP_REG_CFG_1, &data, 0));
                CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                                AP_REG_CFG_1, data | AP_REG_CFG_1_LOCK_10G_KR_MASK, 0));
                hcdType = Port_10GBase_R;
                priority = 10;
                break;

            case AP_REG_ST_0_AP_PWT_UP_10G_KX4_MASK: /* 10G XAUI */
                CHECK_STATUS(genUnitRegisterGet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                                AP_REG_CFG_1, &data, 0));
                CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                                AP_REG_CFG_1, data | AP_REG_CFG_1_LOCK_10G_KX4_MASK, 0));
                hcdType = Port_10GBase_KX4;
                priority = 10;
                break;

            case AP_REG_ST_0_AP_PWT_UP_1G_KX_MASK:
                CHECK_STATUS(genUnitRegisterGet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                                AP_REG_CFG_0, &data, 0));
                CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                                AP_REG_CFG_0, data | AP_REG_CFG_0_LOCK_1G_KX_MASK, 0));
                hcdType = Port_1000Base_KX;
                priority = 1;
                break;

            case AP_REG_ST_0_AP_PWT_UP_40G_CR4_MASK:
                CHECK_STATUS(genUnitRegisterGet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                                AP_REG_CFG_1, &data, 0));
                CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                                AP_REG_CFG_1, data | AP_REG_CFG_1_LOCK_40G_CR4_MASK, 0));
                hcdType = Port_40GBASE_CR4;
                priority = 40;
                break;

            default:
                    found = GT_FALSE;
                    break;
            }
        } else {
            found = GT_FALSE;
        }
    } else {
        found = GT_FALSE;
        AP_PRINT_MAC(("Found NO resolution for IEEE portIndex:%d  \n",portIndex));
    }
#if defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)
    if (AP_ST_HCD_CONSORTIUM_RES_GET(apSm->hcdStatus)) {
        AP_PRINT_MAC(("Found consortium resolution portIndex:%d hcdStatus:0x%x \n",portIndex, apSm->hcdStatus));
        hcdTypeConsortium = AP_ST_HCD_TYPE_GET(apSm->hcdStatus);
        if (found == GT_FALSE){
            hcdType = hcdTypeConsortium;
            fecType = AP_ST_HCD_FEC_RES_GET(apSm->hcdStatus);
            found = GT_TRUE;

        } else {
            consortiumPrio = (hcdTypeConsortium <= Port_25GBASE_CR_C)? 25: 50;
            /* if priority is eqal - choose IEEE type */
            if (consortiumPrio > priority) {
                hcdType = hcdTypeConsortium;
                fecType = AP_ST_HCD_FEC_RES_GET(apSm->hcdStatus);
            }
        }
    }
#else
    /* to avoid error: variable 'priority' set but not used [-Werror=unused-but-set-variable] */
    (GT_VOID)priority;
#endif
#if 0
    genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_REG_CFG_0 , &plData, 0);
    hwsOsPrintf("%d AP_REG_CFG_0  =0x%x\n",portIndex,plData);
    genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_REG_CFG_1 , &plData, 0);
    hwsOsPrintf("%d AP_REG_CFG_1  =0x%x\n",portIndex,plData);
    genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_REG_CFG_2 , &plData, 0);
    hwsOsPrintf("%d AP_REG_CFG_2  =0x%x\n",portIndex,plData);
    genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_REG_CFG_3 , &plData, 0);
    hwsOsPrintf("%d AP_REG_CFG_3  =0x%x\n",portIndex,plData);
#endif

    if (found)
    {
        /* AP HCD update */
        AP_ST_HCD_FOUND_SET(apSm->hcdStatus, 1);
        AP_ST_HCD_TYPE_SET(apSm->hcdStatus, hcdType);
        AP_ST_HCD_FEC_RES_SET(apSm->hcdStatus, fecType);

        AP_PRINT_MAC(("Found final resolution portIndex:%d  hcdStatus:0x%x\n",portIndex, apSm->hcdStatus));
        /* HCD Interrupt update */
        CHECK_STATUS(mvApFcResolutionSet(portIndex));
        mvHwsServCpuIntrSet(portIndex, MV_HWS_PORT_HCD_EVENT | (hcdType << MV_HWS_PORT_HCD_SHIFT));
        AP_ST_HCD_INT_TRIG_SET(apSm->hcdStatus, 1);
        mvPortCtrlLogAdd(AP_PORT_DET_LOG_EXT(apSm->hcdStatus, apSm->portNum));

        /* AP state & status update */
        apSm->status = AP_PORT_RESOLUTION_SUCCESS;
        mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));

        /* AP Resolution Success API */
        mvApPortResolutionSuccess(portIndex, PORT_CTRL_AP_LINK_CHECK_MODE);

        /* AP Timing measurment */
        apStats->hcdResoultionTime = mvPortCtrlCurrentTs() - apStats->timestampTime;
        apStats->timestampTime = mvPortCtrlCurrentTs();

        /* AP register dump */
        mvPortCtrlDbgAllRegsDump(portIndex, pcsNum, AP_PORT_SM_RESOLUTION_STATE);

        /* AP statistics update */
        apStats->abilitySuccessCnt++;
    }
    else /* NO Resolution */
    {
         /* AP Reset statistics and timers */
        mvApResetStatus(portIndex);
        mvApResetTimer(portIndex);

        /* AP state & status update */
        apSm->status = AP_PORT_AP_NO_RESOLUTION;
        mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));
        mvPortCtrlDbgOnDemandRegsDump(portIndex, pcsNum, apSm->state);

        apSm->state = AP_PORT_SM_TX_DISABLE_STATE;

        /* AP validate Ability failure case */
        apStats->abilityInvalidCnt++;
        if (apStats->abilityInvalidCnt > apIntrop->abilityFailMaxInterval)
        {
            apSm->status = AP_PORT_AP_RESOLUTION_MAX_LOOP_FAILURE;
            mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));

            apSm->state = AP_PORT_SM_INIT_STATE;
        }
    }

    return GT_OK;
}

/**
* @internal mvApPortResolutionWaitNP function
* @endinternal
*
* @brief   AP Port Resolution wait for next page
*
* @retval 0                        - on success
* @retval 1                        - on error
* @retval in order to prevent CDC bug in AP (MSPEEDPCS -653) we trap the ST_COMPLETE_ACK state
*                                       in the AP machine and then free the state when reaching ST_COMPLETE_ACK state again
*/
GT_STATUS mvApPortResolutionWaitNP(GT_U8 portIndex)
{
    MV_HWS_AP_SM_INFO       *apSm  = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info);
    GT_U8 pcsNum;
#if defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)
    GT_U8 ack2;
    GT_U16 nextPage0, nextPage1, nextPage2;
    GT_BOOL supported;
#endif

    /* Extract PCS and Lane number */
    pcsNum  = AP_CTRL_PCS_GET(apSm->ifNum);

    if (AP_CTRL_ADV_CONSORTIUM_GET(apSm->capability) == 0) {
        /* we don't support consortium - need to send NULL next page*/
        AP_PRINT_MAC(("ResolutionWaitNP portIndex:%d consortium not supported\n",portIndex));
        mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));
        mvApArbFsmTrapSet(pcsNum, ST_COMPLETE_ACK, GT_TRUE);
        mvApHcdSendNpNull(pcsNum);
        mvApArbFsmTrapSet(pcsNum, ST_COMPLETE_ACK, GT_TRUE);
        return GT_OK;
    }
#if defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)
    AP_PRINT_MAC(("ResolutionWaitNP portIndex:%d status:0x%x\n",portIndex, apSm->status));
    switch (apSm->status) {
    case AP_PORT_RESOLUTION_IN_PROGRESS:
        /* start of resolution*/
        mvApHcdSendNpConsortiumStart(pcsNum, 0);
        apSm->status = AP_PORT_AP_RESOLUTION_CHK_CONSORTIUM;
        mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));
        break;

    case AP_PORT_AP_RESOLUTION_CHK_CONSORTIUM:
        /* read partner next page */
        mvApHcdReadPartnerNp(pcsNum, &nextPage0, &nextPage1, &nextPage2);
        supported = mvApHcdIsPartnerSupportConsortium(nextPage0, nextPage1, nextPage2);
        if (supported == GT_TRUE) {
            ack2 = 1;
        } else {
            ack2 = 0;
        }
        AP_PRINT_MAC(("ResolutionWaitNP portIndex:%d ack2:%d partner next page:0x%x, 0x%x, 0x%x\n",portIndex, ack2, nextPage0, nextPage1, nextPage2));
        mvApHcdSendNpConsortiumStart(pcsNum, ack2);
        apSm->status = AP_PORT_AP_RESOLUTION_CHK_ACK2;
        mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));
        break;

    case AP_PORT_AP_RESOLUTION_CHK_ACK2:
        /* read partner next page */
        mvApHcdReadPartnerNp(pcsNum, &nextPage0, &nextPage1, &nextPage2);
        AP_PRINT_MAC(("ResolutionWaitNP portIndex:%d  partner next page:0x%x, 0x%x, 0x%x\n",portIndex, nextPage0, nextPage1, nextPage2));
        if (nextPage0 & AP_INT_REG_802_3_NEXT_PG_TX_REG_ACK2_MASK) {
            mvApArbFsmTrapSet(pcsNum, ST_COMPLETE_ACK, GT_TRUE);


            /* send partner all ports abilities fec/speed */
            mvApHcdFecNpConsortiumParamsSet(pcsNum, (GT_U8)AP_CTRL_FEC_ABIL_CONSORTIUM_GET(apSm->options),
                                        (GT_U8)AP_CTRL_FEC_REQ_CONSORTIUM_GET(apSm->options));

            mvApHcdAdvertiseNpConsortiumSet(pcsNum, AP_CTRL_ADV_ALL_GET(apSm->capability));

            mvApHcdSendNpConsortium(pcsNum, AP_INT_REG_802_3_NEXT_PG_TX_REG_OUI_TAGGED_UNFORM_CNS,
                                    1 /*ack2*/, 0 /*mp*/, 0 /*ack*/, 0 /*np*/);
            mvApArbFsmTrapSet(pcsNum, ST_COMPLETE_ACK, GT_TRUE);

            /* check partner abilities */
            if (((nextPage0 & AP_INT_REG_802_3_NEXT_PG_TX_REG_OUI_TAGGED_MASK) == AP_INT_REG_802_3_NEXT_PG_TX_REG_OUI_TAGGED_UNFORM_CNS) &&
                ((nextPage0 & AP_INT_REG_802_3_NEXT_PG_TX_REG_MP_MASK) == 0)){
                /* read pertner fec & speed*/
                mvApConsortiumResolutionSet(portIndex, nextPage0, nextPage1, nextPage2);
            }
        } else {
            /* partner don't support consortium - need to send NULL next page*/
            mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));
            mvApArbFsmTrapSet(pcsNum, ST_COMPLETE_ACK, GT_TRUE);
            mvApHcdSendNpNull(pcsNum);
            mvApArbFsmTrapSet(pcsNum, ST_COMPLETE_ACK, GT_TRUE);
            AP_PRINT_MAC(("ResolutionWaitNP portIndex:%d  send null next page\n",portIndex));
        }
        apSm->status = AP_PORT_AP_RESOLUTION_NO_MORE_PAGES;
        break;

    case AP_PORT_AP_RESOLUTION_NO_MORE_PAGES:
        /* read partner next page */
        mvApHcdReadPartnerNp(pcsNum, &nextPage0, &nextPage1, &nextPage2);
        AP_PRINT_MAC(("ResolutionWaitNP portIndex:%d  next page:0x%x, 0x%x, 0x%x\n",portIndex, nextPage0, nextPage1, nextPage2));
        /* check partner abilities */
        if (((nextPage0 & AP_INT_REG_802_3_NEXT_PG_TX_REG_OUI_TAGGED_MASK) == AP_INT_REG_802_3_NEXT_PG_TX_REG_OUI_TAGGED_UNFORM_CNS) &&
            ((nextPage0 & AP_INT_REG_802_3_NEXT_PG_TX_REG_MP_MASK) == 0)){
            /* read pertner fec & speed*/
            mvApConsortiumResolutionSet(portIndex, nextPage0, nextPage1, nextPage2);
        }
        mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));
        mvApArbFsmTrapSet(pcsNum, ST_COMPLETE_ACK, GT_TRUE);
        mvApHcdSendNpNull(pcsNum);
        mvApArbFsmTrapSet(pcsNum, ST_COMPLETE_ACK, GT_TRUE);
        AP_PRINT_MAC(("ResolutionWaitNP portIndex:%d  send null next page\n", portIndex));
        break;
    }
#endif
    return GT_OK;
}
/**
* @internal mvApPortResolution function
* @endinternal
*
* @brief   AP Port Resolution execution sequence
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvApPortResolution(GT_U8 portIndex)
{
    MV_HWS_AP_SM_STATE fsmState;
    GT_U8 pcsNum;

    MV_HWS_AP_SM_INFO       *apSm     = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info);
    MV_HWS_AP_SM_TIMER      *apTimer  = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].timer);
    MV_HWS_AP_SM_STATS      *apStats  = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].stats);
    MV_HWS_AP_DETECT_ITEROP *apIntrop = &(mvHwsPortCtrlApPortDetect.introp);
    GT_BOOL                 bpReached;
    pcsNum = AP_CTRL_PCS_GET(apSm->ifNum);

    /* Check Ability detect threshold */
    if (!(mvPortCtrlThresholdCheck(apTimer->abilityThreshold)))
    {
        return GT_OK;
    }

    /* Extract AP ARB state */
    fsmState = mvApArbFsmGet(pcsNum);
    if (apSm->ARMSmStatus != fsmState) {
        AP_ST_ARB_FSM_SET(apSm->ARMSmStatus, fsmState);
        apTimer->abilityCount = 0;
    }

    apTimer->abilityCount++;
    /* Validate Ability detect max interval, in case crossed update state to INIT */
    if(apTimer->abilityCount >= apIntrop->abilityMaxInterval)
    {

        /* AP state & status update */
        apSm->status = AP_PORT_AP_RESOLUTION_TIMER_FAILURE;
        AP_PRINT_MAC(("Resolution portIndex:%d failed status:0x%x\n",portIndex, apSm->status));
        mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));
        apSm->state = AP_PORT_SM_INIT_STATE;
        return GT_OK;
    }
    /* Ability timer restart */
    mvPortCtrlThresholdSet(apIntrop->abilityDuration, &(apTimer->abilityThreshold));

    /* AP statistics update */
    apStats->abilityCnt++;


    if (apSm->status == AP_PORT_TX_DISABLE_SUCCESS)
    {
        /* AP state & status update */
        apSm->status = AP_PORT_RESOLUTION_IN_PROGRESS;
        AP_PRINT_MAC(("Resolution portIndex:%d set to status:0x%x fsmState:0x%x \n",portIndex, apSm->status, fsmState));
        mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));
        mvPortCtrlDbgOnDemandRegsDump(portIndex, pcsNum, apSm->state);
    }
    switch (fsmState)
    {
    case ST_LINK_STAT_CK:
        CHECK_STATUS(mvApPortResolutionParallelDetect(portIndex));
        break;

    case ST_AN_GOOD_CK:
        CHECK_STATUS(mvApPortResolutionAP(portIndex));
        break;

/* need to send next page for consortium support */
    case ST_COMPLETE_ACK:
        /* in order to prevent CDC bug in AP (MSPEEDPCS-653) we trap the ST_COMPLETE_ACK state
         in the AP machine and then free the state when reaching ST_COMPLETE_ACK state again */
        mvApArbFsmBpCheck(pcsNum, &bpReached);
        if (bpReached)
        {
            /* trap ST_COMPLETE_ACK state */
            mvApArbFsmTrapSet(pcsNum, 0, GT_FALSE);
        }
        CHECK_STATUS(mvApPortResolutionWaitNP(portIndex));
        break;

    case ST_TX_DISABLE:
        AP_PRINT_MAC(("Resolution portIndex:%d fsmState:0x%x reset port*****\n",portIndex, fsmState));
        apSm->status = AP_PORT_AP_RESOLUTION_TIMER_FAILURE;
        mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));
        apSm->state = AP_PORT_SM_INIT_STATE;
        mvApArbFsmTrapSet(pcsNum, 0, GT_FALSE);

        break;

    default:
        return GT_OK;/*Do Nothing*/
    }

    return GT_OK;
}

/*******************************************************************************
*                            AP Port Link Check State                          *
* ============================================================================ *
*******************************************************************************/

/**
* @internal mvApPortLinkFailure function
* @endinternal
*
* @brief   AP Port Link failure execution sequence
*/
GT_STATUS mvApPortLinkFailure(GT_U8 portIndex)
{
    MV_HWS_AP_SM_INFO  *apSm    = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info);
    MV_HWS_AP_SM_STATS *apStats = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].stats);

    GT_U8 pcsNum = AP_CTRL_PCS_GET(apSm->ifNum);

    /* This point means 1 of 2:
    ** 1) during port power up, Max link check was exceeded so the port should
    **    be shut down and the AP process will start over.
    ** 2) link was up but now it's down so AP process should start over (after
    **    the port will be shut down).
    */

    AP_PRINT_MAC(("mvApPortLinkFailure portIndex:%d\n",portIndex));

    /* Link Interrupt status reset */
    mvHwsServCpuIntrStatusReset(portIndex);

    /* Delete the port */
    apSm->status = AP_PORT_DELETE_IN_PROGRESS;
    mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));

    if (mvApPortDelete(portIndex) != GT_OK)
    {
        return GT_OK;
    }

    /* AP statistics update */
    apStats->linkFailCnt++;

    /* clear symbol-lock (using Advertise set) */
    mvApHcdAdvertiseSet(pcsNum, AP_CTRL_ADV_ALL_GET(apSm->capability));

    /* AP state & status update */
    apSm->state = AP_PORT_SM_DELETE_STATE;
    mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));

    return GT_OK;
}

/**
* @internal mvApPortLinkValidate function
* @endinternal
*
* @brief   AP Port Link validate execution sequence
*/
GT_STATUS mvApPortLinkValidate(GT_U8 portIndex, GT_U16 linkCheckInterval, GT_U16 linkCheckDuration)
{
    MV_HWS_PORT_CTRL_AP_INIT *portInit = &(mvHwsPortCtrlApPortMng.apInitStatus);
    MV_HWS_AP_SM_INFO        *apSm     = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info);
    MV_HWS_AP_SM_TIMER       *apTimer  = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].timer);

    /* AP state & status update */
    if (apSm->status != AP_PORT_LINK_CHECK_VALIDATE)
    {
        apSm->status = AP_PORT_LINK_CHECK_VALIDATE;
        mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));
    }

    apTimer->linkCount++;

    /*
    ** In case port was port init Failure
    ** There is no need to wait and check link status and port delete is executed
    */
    if ((apTimer->linkCount >= linkCheckInterval) ||
        (*portInit & MV_PORT_CTRL_NUM(portIndex)))
    {
        /* Link check failure */
        mvApPortLinkFailure(portIndex);
    }
    else
    {
        /* Link check timer restart */
        mvPortCtrlThresholdSet(linkCheckDuration, &(apTimer->linkThreshold));
    }

    return GT_OK;
}

/**
* @internal prvPortCtrlApEngLinkUpCheck function
* @endinternal
*
* @brief   Check link/sync status, which means in AP terms - connection established
*         succesfully or not
* @param[in] portNum                  - port number
* @param[in] portMode                 - interface found by negotiation
* @param[in] fecMode                  - FEC mode found by negotiation(affects which PCS used)
*
* @param[out] linkUpPtr                - connection status
*                                       None.
*/
static GT_STATUS prvPortCtrlApEngLinkUpCheck
(
    GT_U8                   portNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32                  fecMode,
    GT_BOOL                 *linkUpPtr
)
{
    GT_STATUS               rc;

    *linkUpPtr = GT_FALSE;
    switch (portMode)
    {
        case _10GBase_KX4:
            rc = mvHwsPcsPortLinkStatusGet(apDevNum, apDevNum, portNum, portMode, linkUpPtr);
            if (rc != GT_OK)
            {
                mvPcPrintf("Error, AP Detect, portNum-%d portMode=%d mvHwsPcsPortLinkStatusGet %d!!!\n", portNum, portMode, rc);
                return rc;
            }
            break;

        case _1000Base_X:
            rc = mvHwsPortLinkStatusGet(apDevNum, apDevNum, portNum, portMode, linkUpPtr);
            if (rc != GT_OK)
            {
                mvPcPrintf("Error, AP Detect, portNum-%d portMode=%d mvHwsPortLinkStatusGet %d!!!\n", portNum, portMode, rc);
                return rc;
            }
            break;

        case _10GBase_KR:
        case _40GBase_KR:
        case _40GBase_KR4:
        case _40GBase_CR4:
        case _25GBase_KR:
        case _25GBase_CR:
        case _25GBase_KR_C:
        case _50GBase_KR2_C:
        case _25GBase_CR_C:
        case _50GBase_CR2_C:
        case _25GBase_KR_S:
        case _25GBase_CR_S:
        case _100GBase_KR4:
        case _100GBase_CR4:
            {
                MV_HWS_PORT_INIT_PARAMS curPortParams;

                if ((GT_OK != hwsPortModeParamsGetToBuffer(apDevNum, apDevNum, portNum, portMode, &curPortParams))|| (0 == curPortParams.numOfActLanes))
                {
                    mvPcPrintf("Error, AP Detect, portNum-%d,portMode=%d,hwsPortModeParamsGet:GT_NOT_SUPPORTED\n",
                                portNum, portMode);
                    return GT_NOT_SUPPORTED;
                }

                rc = mvHwsPcsAlignLockGet(apDevNum, apDevNum, portNum, portMode, linkUpPtr);
                if (rc != GT_OK)
                {
                    mvPcPrintf("Error, AP Detect, portNum-%d,portMode=%d,pcsNum=%d,pcsType=%d,mvHwsPcsAlignLockGet %d!!!\n",
                                portNum, portMode, curPortParams.portPcsNumber, curPortParams.portPcsType, rc);
                    return rc;
                }
            }
            break;

        default:
            mvPcPrintf("Error, AP Detect, portNum-%d Invalid port mode %d!!!\n", portNum, portMode);
            return GT_NOT_SUPPORTED;
    }

#if 0
            if (GT_FALSE == *linkUpPtr)
            {
                GT_UREG_DATA    data;

                if ((_100GBase_KR4 == portMode) || (AP_ST_HCD_FEC_RES_RS == fecMode))
                {
/* if CG MAC powered down can cause system stuck */
                    genUnitRegisterGet(0, 0, CG_UNIT, portNum, 0x28, &data, 0);
                    hwsOsPrintf("ip_status=0x%x,", data);
                    genUnitRegisterGet(0, 0, CG_UNIT, portNum, 0x28 + 4, &data, 0);
                    hwsOsPrintf("ip_status_2=0x%x\n", data);
                    genUnitRegisterGet(0, 0, CG_UNIT, portNum, 0x440, &data, 0);
                    hwsOsPrintf("cg_status=0x%x\n", data);
                    genUnitRegisterGet(0, 0, CGPCS_UNIT, portNum, 0x4, &data, 0);
                    hwsOsPrintf("cgpcs_status1=0x%x\n", data);
                    genUnitRegisterGet(0, 0, CGPCS_UNIT, portNum, 0x20, &data, 0);
                    hwsOsPrintf("cgpcs_status2=0x%x\n", data);
                    genUnitRegisterGet(0, 0, CGPCS_UNIT, portNum, 0x80, &data, 0);
                    hwsOsPrintf("cgpcs_baseRstatus1=0x%x\n", data);
                    genUnitRegisterGet(0, 0, CGPCS_UNIT, portNum, 0x84, &data, 0);
                    hwsOsPrintf("cgpcs_baseRstatus2=0x%x\n", data);
                    genUnitRegisterGet(0, 0, CGPCS_UNIT, portNum, 0xc8, &data, 0);
                    hwsOsPrintf("cgpcs_multiLalignSt1=0x%x\n", data);
                    genUnitRegisterGet(0, 0, CG_UNIT, portNum, 0x10, &data, 0);
                    hwsOsPrintf("cg_resets=0x%x\n", data);
                    genUnitRegisterGet(0, 0, CG_UNIT, portNum, 0xC,  &data, 0);
                    hwsOsPrintf("cg_ctrl2=0x%x\n", data);
                }

                if (portMode != _100GBase_KR4)
                {
                    genUnitRegisterGet(0, 0, XLGMAC_UNIT, portNum, 0xc, &data, 0);
                    hwsOsPrintf("xlgStatus=0x%x\n", data);
                    genUnitRegisterGet(0, 0, MMPCS_UNIT, portNum, 0x30, &data, 0);
                    hwsOsPrintf("mpcsStatus=0x%x\n", data);
                    genUnitRegisterGet(0, 0, MMPCS_UNIT, portNum, 0x14, &data, 0);
                    hwsOsPrintf("mpcsControl=0x%x\n", data);
                }
            }
#endif

    return GT_OK;
}

/**
* @internal mvApPortLinkUp function
* @endinternal
*
* @brief   AP Port Link Up execution sequence
*/
GT_STATUS mvApPortLinkUp(GT_U8 portIndex)
{
    GT_STATUS            rc;
    MV_HWS_PORT_STANDARD portMode;
    MV_HWA_AP_PORT_MODE  apPortMode;
    GT_BOOL              linkUp;
    GT_U16               linkCheckDuration;
    GT_U16               linkCheckInterval;
    GT_U16               linkCheckIdleDuration;
    GT_U8                pcsNum;

    MV_HWS_PORT_CTRL_PORT_SM *portSm   = &(mvHwsPortCtrlApPortMng.apPortSm[portIndex]);
    MV_HWS_AP_SM_INFO        *apSm     = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info);
    MV_HWS_AP_SM_TIMER       *apTimer  = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].timer);
    MV_HWS_AP_SM_STATS       *apStats  = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].stats);
    MV_HWS_AP_DETECT_ITEROP  *apIntrop = &(mvHwsPortCtrlApPortDetect.introp);

    pcsNum = AP_CTRL_PCS_GET(apSm->ifNum);

    /* Check Link check threshold */
    if (!(mvPortCtrlThresholdCheck(apTimer->linkThreshold)))
    {
        return GT_OK;
    }

    if (apSm->status == AP_PORT_LINK_CHECK_START)
    {
        mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));
    }

    apPortMode = AP_ST_HCD_TYPE_GET(apSm->hcdStatus);
    portMode   = mvHwsApConvertPortMode(apPortMode);

    /* Check link on MAC according to HCD resolution */
    if ((apPortMode == Port_1000Base_KX) || (apPortMode == Port_10GBase_KX4))
    {

        linkCheckDuration = apIntrop->pdLinkDuration;
        linkCheckInterval = apIntrop->pdLinkMaxInterval;
        linkCheckIdleDuration = apIntrop->pdLinkDuration*2;
    }
    else
    {
        linkCheckDuration = apIntrop->apLinkDuration;
        linkCheckInterval = apIntrop->apLinkMaxInterval;
        linkCheckIdleDuration = apIntrop->apLinkDuration*2;
    }

    if (portSm->state == PORT_SM_ACTIVE_STATE)
    {
        rc = prvPortCtrlApEngLinkUpCheck(apSm->portNum, portMode,
                                    AP_ST_HCD_FEC_RES_GET(apSm->hcdStatus), &linkUp);
        if (rc != GT_OK)
        {
            /* AP state & status update */
            apSm->status = AP_PORT_LINK_CHECK_FAILURE;
            mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));
            return rc;
        }
        /* Port is Active & Link_UP detected */
        if (linkUp)
        {
            /* Link up process - only at first occurance */
            if (apSm->status != AP_PORT_LINK_CHECK_SUCCESS)
            {
#if ( defined (PIPE_DEV_SUPPORT) || defined (BC3_DEV_SUPPORT) || defined (ALDRIN2_DEV_SUPPORT) ) && defined (MICRO_INIT)
                printf("\nAP FSM: Try to handle AP port %d link UP",apSm->portNum);
                mvNetPortEnableCfg(apSm->portNum, GT_TRUE);
#endif
                /* AP state & status update */
                apSm->status = AP_PORT_LINK_CHECK_SUCCESS;
                AP_ST_ARB_FSM_SET(apSm->ARMSmStatus, ST_AN_GOOD);
                mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));

                /* AP HCD update */
                AP_ST_HCD_LINK_SET(apSm->hcdStatus, 1);

                /* Link Interrupt update */
                mvHwsServCpuIntrSet(portIndex, MV_HWS_PORT_LINK_EVENT);
                AP_ST_LINK_INT_TRIG_SET(apSm->hcdStatus, 1);

                /* AP Timing measurment */
                apStats->linkUpTime = mvPortCtrlCurrentTs() - apStats->timestampTime;
                apStats->timestampTime = mvPortCtrlCurrentTs();

                /* AP statistics update */
                apStats->linkSuccessCnt++;

                /*Doorbell interrupt*/
                apDoorbellIntrAddEvent(AP_DOORBELL_EVENT_PORT_STATUS_CHANGE(apSm->portNum));
                /* AP register dump */
                mvPortCtrlDbgCtrlRegsDump(portIndex, pcsNum, AP_PORT_SM_ACTIVE_STATE);
            }

            /* Link check timer restart */
            mvPortCtrlThresholdSet(linkCheckIdleDuration, &(apTimer->linkThreshold));
            apTimer->linkCount = 0;
        }
        /* Port is Active NO Link_UP detected */
        else
        {
            /* Port is Active and No link detected,
            ** The previous status was link success, therefore it means link down
            ** is detected first time
            ** In this case execute Link failure
            */
            if (apSm->status == AP_PORT_LINK_CHECK_SUCCESS)
            {
#if ( defined (PIPE_DEV_SUPPORT) || defined (BC3_DEV_SUPPORT) || defined (ALDRIN2_DEV_SUPPORT) ) && defined (MICRO_INIT)
                printf("\nAP FSM: Try to handle AP port %d link Down",apSm->portNum);
                mvNetPortEnableCfg(apSm->portNum, GT_FALSE);
#endif
                /* AP state & status update */
                apSm->status = AP_PORT_LINK_CHECK_FAILURE;
                mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));

                /* Link failure */
                mvApPortLinkFailure(portIndex);
                /*Doorbell interrupt*/
                apDoorbellIntrAddEvent(AP_DOORBELL_EVENT_PORT_STATUS_CHANGE(apSm->portNum));
                return GT_OK;
            }

            /* Port is Active and No link detected,
            ** The previous status was NOT link success
            ** It means that the port is in the process of bring up
            ** this case execute Link validate
            */
            mvApPortLinkValidate(portIndex, linkCheckInterval, linkCheckDuration);
        }
    }
    else /* portSm->state != PORT_SM_ACTIVE_STATE */
    {
        /* Port is Not Active
        ** It means that the port is in the process of bring up
        ** this case execute Link validate
        */
        mvApPortLinkValidate(portIndex, linkCheckInterval, linkCheckDuration);
    }

    return GT_OK;
}

/**
* @internal mvApPortDeleteValidate function
* @endinternal
*
* @brief   AP Port Link Up execution sequence
*/
GT_STATUS mvApPortDeleteValidate(GT_U8 portIndex)
{
    MV_HWS_PORT_CTRL_PORT_SM *portSm  = &(mvHwsPortCtrlApPortMng.apPortSm[portIndex]);
    MV_HWS_AP_SM_INFO        *apSm    = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info);
    MV_HWS_AP_SM_STATS       *apStats = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].stats);
    MV_HWS_PORT_CTRL_AP_PORT_TIMER *portTimer = &(mvHwsPortCtrlApPortMng.apPortTimer[portIndex]);
    GT_U8 pcsNum = AP_CTRL_PCS_GET(apSm->ifNum);

    if (portSm->state == PORT_SM_IDLE_STATE && portSm->status == PORT_SM_DELETE_SUCCESS)
    {
        /* AP state & status update */
        if(apSm->state == AP_PORT_SM_DELETE_STATE)
        {
            apSm->status = AP_PORT_DELETE_SUCCESS;
            mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));

            /* AP Timing reset */
            mvApResetStatsTimestamp(portIndex);

            /* AP Timing measurment */
            apStats->timestampTime = mvPortCtrlCurrentTs();

            /* AP state & status update */
            apSm->state = AP_PORT_SM_INIT_STATE;
            apSm->status = AP_PORT_START_EXECUTE;
            portSm->status = PORT_SM_NOT_RUNNING;
        }
        else if(apSm->state == AP_PORT_SM_DISABLE_STATE)
        {
            /* PCS MUX to non AP mode */
            mvApPcsMuxReset(pcsNum);

#if defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)
            mvApConfig(pcsNum, 0);
#endif

            /* AP Reset All */
            mvApResetStatus(portIndex);
            mvApResetTimer(portIndex);
            mvApResetStats(portIndex);
            mvApResetStatsTimestamp(portIndex);

            /*Reset Pizza ack parameters*/
            portTimer->sysCfStatus = PORT_SM_SERDES_SYSTEM_NOT_VALID;
            portTimer->sysCfMode = NON_SUP_MODE;

            /* AP ARB state machine - AN_ENABLE */
            AP_ST_ARB_FSM_SET(apSm->ARMSmStatus, ST_AN_ENABLE);

            apSm->status = AP_PORT_DELETE_SUCCESS;
            mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));

            /* AP state & status update */
            apSm->state  = AP_PORT_SM_IDLE_STATE;
            portSm->status = PORT_SM_NOT_RUNNING;
        }
    }

    return GT_OK;
}

/*******************************************************************************
*                            AP Port Delete State                              *
* ============================================================================ *
*******************************************************************************/

/**
* @internal mvApPortDeleteMsg function
* @endinternal
*
* @brief   AP Port Delete Msg execution sequence
*/
GT_STATUS mvApPortDeleteMsg(GT_U8 portIndex)
{
    MV_HWS_AP_SM_INFO *apSm = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info);
    MV_HWS_PORT_CTRL_AP_PORT_TIMER *portTimer = &(mvHwsPortCtrlApPortMng.apPortTimer[portIndex]);
    GT_U8 pcsNum = AP_CTRL_PCS_GET(apSm->ifNum);
    /* Validate AP port state & status */
    if (apSm->state != AP_PORT_SM_IDLE_STATE)
    {

        if ((apSm->state == AP_PORT_SM_ACTIVE_STATE) &&
            (apSm->status != AP_PORT_DELETE_IN_PROGRESS))
        {
            apDoorbellIntrAddEvent(AP_DOORBELL_EVENT_PORT_STATUS_CHANGE(apSm->portNum));

            /* AP Port Delete */
            if(mvApPortDelete(portIndex) != GT_OK)
            {
                AP_PRINT_MAC(("Port %d - Delete AP training task failed\n",apSm->portNum));
                return GT_FAIL;
            }

            /* AP state & status update */
            apSm->state = AP_PORT_SM_DISABLE_STATE;
            apSm->status = AP_PORT_DELETE_IN_PROGRESS;
            mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));

            /*other reset operation will be done after AP training task delete*/
            return GT_OK;

        }

        /*we get to this point if resulotion was not found till the point user ask to disable AP*/

        /* AP state & status update */
        apSm->state = AP_PORT_SM_DISABLE_STATE;
        apSm->status = AP_PORT_DELETE_IN_PROGRESS;
        mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));

        /* PCS MUX to non AP mode */
        mvApPcsMuxReset(pcsNum);

#if defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)
        mvApConfig(pcsNum, 0);
#endif

        /* AP Reset All */
        mvApResetStatus(portIndex);
        mvApResetTimer(portIndex);
        mvApResetStats(portIndex);
        mvApResetStatsTimestamp(portIndex);

        /*Reset Pizza ack parameters*/
        portTimer->sysCfStatus = PORT_SM_SERDES_SYSTEM_NOT_VALID;
        portTimer->sysCfMode = NON_SUP_MODE;

        /* AP ARB state machine - AN_ENABLE */
        AP_ST_ARB_FSM_SET(apSm->ARMSmStatus, ST_AN_ENABLE);

        apSm->status = AP_PORT_DELETE_SUCCESS;
        mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));

        /* AP state & status update */
        apSm->state = AP_PORT_SM_IDLE_STATE;
    }

    return GT_OK;
}

