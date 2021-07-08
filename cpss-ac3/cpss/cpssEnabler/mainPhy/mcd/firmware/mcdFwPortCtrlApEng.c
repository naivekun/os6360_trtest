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
* mcdFwPortCtrlAp.c
*
* DESCRIPTION:
*       Port Control AP Detection Engine
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/
#include <mcdFwPortCtrlInc.h>
#include <mvHwsServiceCpuInt.h> /* from silicone */
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
#include "mcdPortCtrlDoorbellEvents.h"
extern MCD_DEV mcdDevDb;
#endif
extern MCD_PORT_CTRL_AP_CALLBACKS mvPortCtrlApCallbacks;

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

/* These extern requires for MCD_STAT_AP_SM_STATUS_INSERT and MCD_STAT_AP_SM_STATE_INSERT */
void  mcdStatApSmStateInsert(GT_U8 portIndex, GT_U16 state);
void  mcdStatApSmStatusInsert(GT_U8 portIndex, GT_U16 status);

GT_STATUS mvHwsPortEnableSet
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          phyPortNum,
    MV_HWS_PORT_STANDARD            portMode,
    GT_BOOL                         enable
);

GT_STATUS mvHwsPortRetimerModeSet
(
    GT_U32              portNum,
    MCD_OP_MODE         portMode
);
GT_STATUS mvApPortReset
(
    MCD_U32 phyPortNum
);


/* Default values for AP Device and Port Group */
#ifdef MV_SERDES_AVAGO_IP

#include "mv_hws_avago_if.h"
GT_U8  apDevNum    = 0;
/* AVAGO_SERDES_INIT_BYPASS_VCO
** ============================
** BOBK Avago SERDES Init includes VCO calibration,
** this calibration adds 20msec delay for each SERDES
** The portGroup parameter is used to pass bypass VCO
** calibration indication to SERDES Init API in case
** called during AP process
** Only valid for BOBK
*/
#ifndef BV_DEV_SUPPORT
GT_U32 apPortGroup = AVAGO_SERDES_INIT_BYPASS_VCO;
#else
GT_U32 apPortGroup = 0;
#endif
#else
GT_U8  apDevNum    = 0;
GT_U32 apPortGroup = 0;
#endif /* MV_SERDES_AVAGO_IP */

/**
* @internal mvHwsApConvertPortMode function
* @endinternal
*
*/
MV_HWS_PORT_STANDARD mvHwsApConvertPortMode(MV_HWA_AP_PORT_MODE apPortMode)
{
    switch (apPortMode)
    {
    case Port_1000Base_KX:      return MCD_MODE_P1;
    case Port_10GBase_R:        return MCD_MODE_P10S;
    case Port_40GBase_R:        return MCD_MODE_P40K;
    case Port_40GBASE_CR4:      return MCD_MODE_P40C;
    case Port_100GBASE_KP4:     return MCD_MODE_P100S;
    case Port_100GBASE_KR4:     return MCD_MODE_P100S;
    case Port_100GBASE_CR4:     return MCD_MODE_P100C;
    case Port_25GBASE_KR_S:     return MCD_MODE_P25S;
    case Port_25GBASE_KR:       return MCD_MODE_P25S;
#ifndef RMV_CONSORTIUM
    case Port_25GBASE_KR1_CONSORTIUM: return MCD_MODE_P25S;
    case Port_25GBASE_CR1_CONSORTIUM: return MCD_MODE_P25S;
    case Port_50GBASE_KR2_CONSORTIUM: return MCD_MODE_P50R2S;
    case Port_50GBASE_CR2_CONSORTIUM: return MCD_MODE_P50R2S;
#endif /*#ifndef RMV_CONSORTIUM*/

    default:               return NON_SUP_MODE;
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
    AP_PRINT_MCD2_MAC(("mvApArbFsmTrapSet pcsNum %d set fsmState 0x%x enable %d\n",pcsNum, arbState, enable));
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
        AP_PRINT_MCD2_MAC(("mvApArbFsmBpCheck pcsNum %d  fsmState 0x%x bpReached %d\n",pcsNum,
                      (data >> AP_INT_REG_ANEG_CTRL_0_ANEG_STATE_SHIFT),*bpReached));
    }
    return GT_OK;
}


#ifndef MV_BV_PCS
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
#else
GT_STATUS mvApConfig(GT_U8 laneNum, GT_U8 mode)
{
    GT_U32 slice, phyLaneNum;
    /* get slice number */
    MCD_GET_SLICE_NUM_BY_SERDES((GT_U32)laneNum,slice);
    MCD_FW_LOGIC_TO_PHYSICAL_SERDES_MAC((GT_U32)laneNum,phyLaneNum);
    CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, phyLaneNum,
                                    AP_CONTROL_REG,mode,1));
    CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, phyLaneNum,
                                    AP_REG_CFG_1,mode << 3, 8));

    return GT_OK;

}


#endif
/*******************************************************************************
* mvApHcdFecParamsSet
* mvApHcdFcParamsSet
* mvApHcdAdvertiseSet
* mvApFcSet
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
#ifndef RMV_CONSORTIUM
/** mvApHcdFecNpConsortiumParamsSet
*******************************************************************************/
GT_STATUS mvApHcdFecNpConsortiumParamsSet(GT_U8 pcsNum, GT_U8 fecAbility, GT_U8 fecEnable)
{
    GT_U32 data = (fecEnable << 2) + fecAbility;

    CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                    AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31,
                                    (data << AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_FEC_SHIFT),
                                    AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_FEC_MASK));
    AP_PRINT_MCD2_MAC(("mvApHcdFecNpConsortiumParamsSet  pcsNum:%d adr:0x%x data:0x%x, mask:0x%x\n",pcsNum, AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31, (data << AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_FEC_SHIFT),
                  (AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_FEC_MASK)));

    return GT_OK;
}
#endif /*#ifndef RMV_CONSORTIUM*/
GT_STATUS mvApHcdFecParamsGet(GT_U8 portNum, GT_U8 *fecType)
{
    MCD_AP_SM_INFO *apSm = &(mvHwsPortCtrlApPortDetect.apPortSm[portNum].info);

    switch (AP_ST_HCD_FEC_RES_GET(apSm->hcdStatus))
    {
        case AP_ST_HCD_FEC_RES_NONE:
            *fecType = MCD_NO_FEC;
            break;
        case AP_ST_HCD_FEC_RES_FC:
            *fecType =  MCD_FC_FEC;
            break;
        case AP_ST_HCD_FEC_RES_RS:
            *fecType =  MCD_RS_FEC;
            break;
        case AP_ST_HCD_FEC_RES_BOTH:
            if( Port_25GBASE_KR == AP_ST_HCD_TYPE_GET(apSm->hcdStatus))
                *fecType = MCD_RS_FEC;
            else
                *fecType =  MCD_FC_FEC;
            break;
        default:
            mvPcPrintf("Error, wrong FEC config value\n", apSm->hcdStatus);
            return GT_FAIL;
    }
    return GT_OK;
}
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
           (AP_CTRL_1000Base_KX_GET(modeVector) << AP_REG_CFG_0_ADV_1G_KX_SHIFT);
    CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                        AP_REG_CFG_0,data,AP_REG_CFG_0_ADV_MASK));

    /* data to write to CFG1 register*/
    data = (AP_CTRL_40GBase_CR4_GET(modeVector) << AP_REG_CFG_1_ADV_40G_CR4_SHIFT);
    CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                    AP_REG_CFG_1,data, AP_REG_CFG_1_ADV_MASK));
    /* data to write to CFG2 register*/
    data = (AP_CTRL_100GBase_KP4_GET(modeVector) << AP_REG_CFG_2_ADV_100G_KP4_SHIFT) |
           (AP_CTRL_100GBase_KR4_GET(modeVector) << AP_REG_CFG_2_ADV_100G_KR4_SHIFT) |
           (AP_CTRL_100GBase_CR4_GET(modeVector) << AP_REG_CFG_2_ADV_100G_CR4_SHIFT) |
           (AP_CTRL_25GBase_KR1S_GET(modeVector) << AP_REG_CFG_2_ADV_25G_S_SHIFT)  |
           (AP_CTRL_25GBase_KR1_GET(modeVector) << AP_REG_CFG_2_ADV_25G_SHIFT );
    CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                    AP_REG_CFG_2,data, AP_REG_CFG_2_ADV_MASK));
#if 0 /* code below  - for debug only*/
{
        osPrintf("HcdAdvertiseSet mode %x \n",modeVector);

        CHECK_STATUS(genUnitRegisterGet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                        AP_REG_CFG_0,&data, 0));

            osPrintf("HcdAdvertiseSet read cfg0 %x\n",data);
        CHECK_STATUS(genUnitRegisterGet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                        AP_REG_CFG_1,&data, 0));

            osPrintf("HcdAdvertiseSet read cfg1 %x\n",data);
        CHECK_STATUS(genUnitRegisterGet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                        AP_REG_CFG_2,&data, 0));

            osPrintf("HcdAdvertiseSet read cfg2 %x\n",data);
 }
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

#ifndef RMV_CONSORTIUM
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
    AP_PRINT_MCD2_MAC(("mvApHcdAdvertiseNpConsortiumSet  pcsNum:%d adr:0x%x data:0x%x, mask:0x%x\n",pcsNum, AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15, data, (AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_25G_MASK | AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_50G_MASK)));
    return GT_OK;

}
#endif /*#ifndef RMV_CONSORTIUM*/
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
    AP_PRINT_MCD2_MAC(("mvApHcdSendNpConsortium  pcsNum:%d adr:0x%x data:0x%x, mask:0x%x\n",pcsNum, AP_INT_REG_802_3_NEXT_PG_TX_REG, data, (AP_INT_REG_802_3_NEXT_PG_TX_REG_OUI_TAGGED_MASK | AP_INT_REG_802_3_NEXT_PG_TX_REG_CTRL_BIT_MASK)));
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
    CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                    AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15,
                                    0 ,
                                    0));
     return mvApHcdSendNpConsortium(pcsNum, AP_INT_REG_802_3_NEXT_PG_TX_REG_OUI_TAGGED_FORM_NULL_CNS, 0, 1, 0, 0);
}

#ifndef RMV_CONSORTIUM
/** mvApHcdSendNpConsortiumStart
*******************************************************************************/
GT_STATUS mvApHcdSendNpConsortiumStart(GT_U8 pcsNum, GT_U8 ack2)
{

    CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                    AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31,
                                    AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_OUI_TAGGED_FORM_CNS,
                                    0));
    CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                    AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15,
                                    AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_OUI_TAGGED_FORM_CNS ,
                                    0));
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
            (AP_INT_REG_802_3_NEXT_PG_TX_REG_OUI_TAGGED_FORM_CNS | AP_INT_REG_802_3_NEXT_PG_TX_REG_MP_MASK | AP_INT_REG_802_3_NEXT_PG_TX_REG_NP_MASK)))
    {
        return GT_TRUE;
    }
    else
    {
        return GT_FALSE;
    }
}
#endif /*#ifndef RMV_CONSORTIUM*/

/** mvApFcResolutionSet
*******************************************************************************/
GT_STATUS mvApFcResolutionSet(GT_U8 portIndex)
{
    GT_U8  localFcEn;
    GT_U8  localFcAsm;
    GT_U8  peerFcEn;
    GT_U8  peerFcAsm;
    GT_U32 data;
    GT_U32  slice, phyPcsNum;
    MCD_AP_SM_INFO *apSm = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info);
    GT_U8 pcsNum = AP_CTRL_PCS_GET(apSm->ifNum);
    slice = portIndex/4;
    MCD_FW_LOGIC_TO_PHYSICAL_SERDES_MAC((GT_U32)pcsNum,phyPcsNum);

    CHECK_STATUS(genUnitRegisterGet(apDevNum, apPortGroup, MMPCS_UNIT, phyPcsNum,
                                    AP_INT_REG_802_3_LP_BASE_ABILITY_REG_1, &data, 0));

    localFcEn  = (GT_U8)AP_CTRL_FC_PAUSE_GET(apSm->options);
    localFcAsm = (GT_U8)AP_CTRL_FC_ASM_GET(apSm->options);

    peerFcEn  = (GT_U8)((data >> AP_INT_REG_802_3_LP_BASE_ABILITY_REG_1_FC_PAUSE_SHIFT)& 0x1);
    peerFcAsm = (GT_U8)((data >> AP_INT_REG_802_3_LP_BASE_ABILITY_REG_1_FC_DIR_SHIFT)& 0x1);

    AP_PRINT_MCD_MAC(("mvApFcResolutionSet port:%d options:0x%x\n", portIndex, apSm->options));
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
    AP_PRINT_MCD_MAC(("mvApFcResolutionSet end port:%d options:0x%x\n", portIndex, apSm->options));

    return GT_OK;
}
#ifndef RMV_CONSORTIUM
/** mvApConsortiumResolutionSet
*******************************************************************************/
MCD_STATUS mvApConsortiumResolutionSet(GT_U8 portIndex, GT_U16 np0, GT_U16 np1, GT_U16 np2)
{

    MCD_AP_SM_INFO *apSm = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info);

    if ((AP_CTRL_50GBase_KR2_CONSORTIUM_GET(apSm->capability) == 1) &&
        ((np1 & AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_50G_KR2_MASK) == AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_50G_KR2_MASK))
    {
        /* 50G is selected */
        AP_ST_HCD_TYPE_SET(apSm->hcdStatus, Port_50GBASE_KR2_CONSORTIUM); /* Set FC Tx */
        AP_PRINT_MCD2_MAC(("Consortium partner resolution portIndex:%d  Port_50GBASE_KR2_CONSORTIUM\n", portIndex));
    }
    else if ((AP_CTRL_50GBase_CR2_CONSORTIUM_GET(apSm->capability) == 1) &&
        ((np1 & AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_50G_CR2_MASK) == AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_50G_CR2_MASK))
    {
        /* 50G is selected */
        AP_ST_HCD_TYPE_SET(apSm->hcdStatus, Port_50GBASE_CR2_CONSORTIUM); /* Set FC Tx */
        AP_PRINT_MCD2_MAC(("Consortium partner resolution portIndex:%d  Port_50GBASE_CR2_CONSORTIUM\n", portIndex));
    }
    else if ((AP_CTRL_25GBase_KR1_CONSORTIUM_GET(apSm->capability) == 1) &&
        ((np1 & AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_25G_KR1_MASK) == AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_25G_KR1_MASK))
    {
        /* 50G is selected */
        AP_ST_HCD_TYPE_SET(apSm->hcdStatus, Port_25GBASE_KR1_CONSORTIUM); /* Set FC Tx */
        AP_PRINT_MCD2_MAC(("Consortium partner resolution portIndex:%d  Port_25GBASE_KR1_CONSORTIUM\n", portIndex));
    }
    else if ((AP_CTRL_25GBase_CR1_CONSORTIUM_GET(apSm->capability) == 1) &&
        ((np1 & AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_25G_CR1_MASK) == AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_25G_CR1_MASK))
    {
        /* 50G is selected */
        AP_ST_HCD_TYPE_SET(apSm->hcdStatus, Port_25GBASE_CR1_CONSORTIUM); /* Set FC Tx */
        AP_PRINT_MCD2_MAC(("Consortium partner resolution portIndex:%d  Port_25GBASE_CR1_CONSORTIUM\n", portIndex));
    }
    else
    {
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
        ((np2 & AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_FEC_RS_ABIL_MASK) == AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_FEC_RS_ABIL_MASK))
    {
        /* RS FEC is selected */
        AP_ST_HCD_FEC_RES_SET(apSm->hcdStatus,AP_ST_HCD_FEC_RES_RS);
        AP_PRINT_MCD2_MAC(("Consortium partner resolution portIndex:%d  AP_ST_HCD_FEC_RES_RS\n", portIndex));
    }
    else if (((AP_CTRL_FC_FEC_REQ_CONSORTIUM_GET(apSm->options) == 1) ||
                ((np2 & AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_FEC_FC_REQ_MASK) == AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_FEC_FC_REQ_MASK))&&
               (AP_CTRL_FC_FEC_ABIL_CONSORTIUM_GET(apSm->options) == 1) &&
               ((np2 & AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_FEC_FC_ABIL_MASK) == AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_FEC_FC_ABIL_MASK))
    {
        /* BASE-R FEC is selected */
        AP_ST_HCD_FEC_RES_SET(apSm->hcdStatus, AP_ST_HCD_FEC_RES_FC);
        AP_PRINT_MCD2_MAC(("Consortium partner resolution portIndex:%d  AP_ST_HCD_FEC_RES_BASE_R\n", portIndex));
    }
    else
    {
        AP_PRINT_MCD2_MAC(("Consortium partner resolution portIndex:%d  NO FEC\n", portIndex));
    }

    return GT_OK;
}
#endif /*#ifndef RMV_CONSORTIUM*/
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
    MCD_AP_SM_INFO *apSm = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info);

    apSm->hcdStatus   = 0;
    apSm->status      = 0;
    MCD_STAT_AP_SM_STATUS_INSERT(portIndex,AP_PORT_START_EXECUTE);
}

/** mvApResetTimer
*******************************************************************************/
void mvApResetTimer(GT_U8 portIndex)
{
    MCD_AP_SM_TIMER *apTimer = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].timer);

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
    MCD_AP_SM_STATS *apStats = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].stats);

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
    MCD_AP_SM_STATS *apStats = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].stats);

    apStats->timestampTime     = 0;
    apStats->hcdResoultionTime = 0;
    apStats->linkUpTime        = 0;
    apStats->portEnableTime    = 0;
    apStats->hcdClbkStartTime = 0;
    apStats->hcdClbkStopTime = 0;
    apStats->trxClbkStartTime = 0;
    apStats->trxClbkStopTime = 0;
    apStats->linkClbkStartTime = 0;
    apStats->linkClbkStopTime = 0;
}

/** mvApPortDelete
*******************************************************************************/
GT_STATUS mvApPortDelete(GT_U8 portIndex)
{
    MV_HWS_PORT_STANDARD portMode;
    MV_HWA_AP_PORT_MODE  apPortMode;
    MCD_AP_SM_INFO    *apSm = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info);
    GT_STATUS rc;
    apPortMode = AP_ST_HCD_TYPE_GET(apSm->hcdStatus);

    portMode = mvHwsApConvertPortMode(apPortMode);
    if(portMode == NON_SUP_MODE)
    {
        return GT_NOT_SUPPORTED;
    }
#ifndef BV_DEV_SUPPORT
    /* Disabling MAC before continue with port delete operations in order to provide maximal
       time between MAC disable to the potential MAC enable, in which at that time resources
       of that port are being freed */
    mvHwsPortEnableSet(0/*dev*/, 0/*portGroup*/, (GT_U8)apSm->portNum,
                               portMode, 0/*disable*/);
#endif
    /* Stop RX Adaptive training */
    rc = mvHwsPortAutoTuneSet(0, 0, apSm->portNum,
                                 portMode, RxTrainingStopAdative, NULL);
    AP_PRINT_MCD_MAC((">>>mvHwsPortAutoTuneSet RxTrainingStopAdative port:%d mode %d rc %d\n",apSm->portNum, portMode, rc));

    if (rc != GT_OK)
    {
        return rc;
    }

    /* Send port delete message */
    rc = mvPortCtrlApPortMsgSend(portIndex,
                            (GT_U8)MCD_IPC_PORT_RESET_MSG,
                            (GT_U8)AP_CTRL_QUEUEID_GET(apSm->ifNum)  /*apSm->queueId*/,
                            (GT_U8)apSm->portNum,
                            (GT_U8)portMode,
                            (GT_U8)PORT_POWER_DOWN,
                            0, 0);
    if (rc != GT_OK)
    {
        AP_PRINT_MCD_MAC(("mvApPortDelete portIndex:%d  mvPortCtrlApPortMsgSend fail\n", portIndex));
    }

    return GT_OK;
}

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
    MCD_AP_SM_STATS *apStats = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].stats);

#ifndef BV_DEV_SUPPORT

    MCD_AP_SM_INFO  *apSm    = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info);
    /* set DMA in 10G/40G mode (instead of 1G mode) */
    CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, XLGMAC_UNIT,
                                    AP_CTRL_MAC_GET(apSm->ifNum), 0x84, 0, (1 << 12)));
#endif
    /* reset timestamps */
    mvApResetStatsTimestamp(portIndex);
    apStats->timestampTime = mvPortCtrlCurrentTs();

    return GT_OK;
}

/** mvApSerdesPowerUp
*******************************************************************************/
GT_STATUS mvApSerdesPowerUp(GT_U8 serdesNum, GT_BOOL powerUp)
{
    GT_STATUS status = GT_OK;

    status =  mcdSerdesPowerCtrl(&mcdDevDb,serdesNum,powerUp, _3_125G,GT_FALSE,0 /* primary clock */ ,1 /* no division */);

#ifdef MV_SERDES_AVAGO_IP

    /* set max amplitude */
    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(0, 0, serdesNum, 0x3D, 0x6004, NULL));

    /* set amplitude value for Initialize mode */
    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(0, 0, serdesNum, 0x3D, 0xA007, NULL));

    /* set emph0 value for Initialize mode */
    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(0, 0, serdesNum, 0x3D, 0xB019, NULL));

    /* set emph1 value for Initialize mode */
    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(0, 0, serdesNum, 0x3D, 0x9009, NULL));

    /* Enable the Tx signal, the signal was disabled during Serdes init */

    CHECK_STATUS(mvHwsAvagoSerdesTxEnable(0, 0, serdesNum, GT_TRUE));
#endif /* MV_SERDES_AVAGO_IP */

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
    MCD_AP_SM_INFO  *apSm    = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info);
    MCD_AP_SM_STATS *apStats = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].stats);

    GT_U8 pcsNum = AP_CTRL_PCS_GET(apSm->ifNum);
    GT_U8 laneNum = AP_CTRL_LANE_GET(apSm->ifNum);
    GT_U32 slice,phyPcsNum;
    GT_STATUS rc = GT_OK;

    /* get slice number */
    slice = portIndex/4;
    /* since we don't have remaping inside the AP unit, need to convert
       the logical pcs number to physical pcs number as well */
    MCD_FW_LOGIC_TO_PHYSICAL_SERDES_MAC((GT_U32)pcsNum,phyPcsNum);

    mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));

    mvApResetStatus(portIndex);
    mvApResetTimer(portIndex);

    mvApPortReset(laneNum);

    /* power down AP unit to ensure clean AP initalization process */
    mvApConfig(laneNum, GT_FALSE);

    mcdDevDb.serdesStatus.ctleBiasData[portIndex] = AP_CTRL_CTLE_BIAS_VAL_GET(apSm->options);

    /* Lane power up in 3.125 */
    rc = mvApSerdesPowerUp(laneNum, GT_TRUE);

    /* Check if driver configured for VSR tuning */
    if ((rc == GT_OK) && (mcdDevDb.apCfg[portIndex].enSdTuningApRes))
    {
        /* With some longer cable lenth and due to weak Tx drive strength from peer, AN
           was not not getting completed because of no signal detect, hence to recover
           from this situation we need to trigger VSR.
           Call this API only in this specific case */
        AP_PRINT_MCD2_MAC(("VSR tuning will be triggered on portIndex:%d\n",portIndex));
        rc = mcdSerdesDfeConfig(&mcdDevDb, laneNum, MCD_DFE_ICAL_VSR);
    }

    if (rc != GT_OK)
    {
        /* if Serdes power-up (or) DFE config for VSR-tuning fails, power it down, exit
        ** and next iteration the Serdes will be powered-up again
        */
        apSm->status = AP_PORT_SERDES_INIT_FAILURE;
        MCD_STAT_AP_SM_STATUS_INSERT(portIndex,AP_PORT_SERDES_INIT_FAILURE);
        mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));

        mvApSerdesPowerUp(laneNum, GT_FALSE);
        return GT_OK;
    }

#ifndef MV_BV_PCS
    /* PCS MUX set to AP */
    mvApPcsMuxSet(phyPcsNum, 3);
#else
    mvApConfig(laneNum, GT_TRUE);
#endif

    mvApUnreset(phyPcsNum, GT_TRUE);
    /* AP Enable */
    mvApAnEnable(phyPcsNum, 1);

    /* AP ARB state machine - AN_ENABLE */
    mvApArbFsmSet(phyPcsNum, ST_AN_ENABLE);
    AP_ST_ARB_FSM_SET(apSm->ARMSmStatus, ST_AN_ENABLE);

    /* AP state & status update */
    apSm->status = AP_PORT_INIT_SUCCESS;
    MCD_STAT_AP_SM_STATUS_INSERT(portIndex,AP_PORT_INIT_SUCCESS);
    mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));

    apSm->state = AP_PORT_SM_TX_DISABLE_STATE;
    MCD_STAT_AP_SM_STATE_INSERT(portIndex,AP_PORT_SM_TX_DISABLE_STATE);

    /* AP statistics update */
    apStats->abilityInvalidCnt = 0;

    /* AP register dump */
    mvPortCtrlDbgAllRegsDump(portIndex, phyPcsNum, AP_PORT_SM_INIT_STATE);

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

    MCD_AP_SM_INFO *apSm = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info);
    GT_U8 pcsNum = AP_CTRL_PCS_GET(apSm->ifNum);
    GT_U32 slice,phyPcsNum;
    /* get slice number */
    slice = portIndex/4;
    /* since we don't have remaping inside the AP unit, need to convert
       the logical pcs number to physical pcs number as well */
    MCD_FW_LOGIC_TO_PHYSICAL_SERDES_MAC((GT_U32)pcsNum,phyPcsNum);
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

    CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, phyPcsNum,
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
    MCD_AP_SM_INFO       *apSm     = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info);
    MCD_AP_SM_TIMER      *apTimer  = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].timer);
    MCD_AP_SM_STATS      *apStats  = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].stats);
    MCD_AP_DETECT_ITEROP *apIntrop = &(mvHwsPortCtrlApPortDetect.introp);
    GT_U8 devNum = 0;
    GT_U32 slice,phyPcsNum;
    /* get slice number */
    slice = portIndex/4;

    /* Extract PCS and Lane number */
    pcsNum  = AP_CTRL_PCS_GET(apSm->ifNum);
    laneNum = AP_CTRL_LANE_GET(apSm->ifNum);

    /* since we don't have remaping inside the AP unit, need to convert
       the logical pcs number to physical pcs number as well */
    MCD_FW_LOGIC_TO_PHYSICAL_SERDES_MAC((GT_U32)pcsNum,phyPcsNum);

    if (apSm->status != AP_PORT_TX_DISABLE_IN_PROGRESS)
    {
        /* SERDES Tx Disable */
        mvHwsSerdesTxEnable(apDevNum, apDevNum, laneNum, HWS_DEV_SERDES_TYPE(devNum), GT_FALSE);

        /* AP ARB state machine - TX_DISABLE */
        mvApArbFsmSet(phyPcsNum, ST_TX_DISABLE);
        AP_ST_ARB_FSM_SET(apSm->ARMSmStatus, ST_TX_DISABLE);

        /* Reload advertisement - Configure HW with HCD parameters from AP control */
        mvApHcdAdvertiseSet(phyPcsNum, AP_CTRL_ADV_ALL_GET(apSm->capability));

        mvApParallelDetectModeSet(portIndex);


        mvApHcdFecParamsSet(phyPcsNum, (GT_U8)AP_CTRL_FEC_ABIL_GET(apSm->options),
                                    (GT_U8)AP_CTRL_FEC_REQ_GET(apSm->options));

        mvApHcdFecAdvancedParamsSet(phyPcsNum, (GT_U8)((AP_CTRL_FEC_FC_REQ_GET(apSm->options)<<1)
                                                    | AP_CTRL_FEC_RS_REQ_GET(apSm->options)));


        /* Config FC and FEC */
        mvApHcdFcParamsSet(phyPcsNum, (GT_U8)AP_CTRL_FC_PAUSE_GET(apSm->options),
                                   (GT_U8)AP_CTRL_FC_ASM_GET(apSm->options));
        /*config NP = 1 in case of consortium */
        if (AP_CTRL_ADV_CONSORTIUM_GET(apSm->capability)) {
            AP_PRINT_MCD2_MAC(("set consortium portIndex:%d capability:0x%x\n",portIndex, apSm->capability));
            mvApHcdParamsNpSet(phyPcsNum, 1);
        }
        /* Config loopback if needed */
        if (AP_CTRL_LB_EN_GET(apSm->options))
        {
            mvApPreCfgLoopback(phyPcsNum, 1);
        }
        /* AP state & status update */
        apSm->status = AP_PORT_TX_DISABLE_IN_PROGRESS;
        MCD_STAT_AP_SM_STATUS_INSERT(portIndex,AP_PORT_TX_DISABLE_IN_PROGRESS);
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
            MCD_STAT_AP_SM_STATUS_INSERT(portIndex,AP_PORT_TX_DISABLE_SUCCESS);
            AP_PRINT_MCD_MAC(("TxDisable portIndex:%d status:0x%x\n",portIndex, apSm->status));
            mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));

            mvApArbFsmSet(phyPcsNum, ST_ABILITY_DET);
            AP_ST_ARB_FSM_SET(apSm->ARMSmStatus, ST_ABILITY_DET);
            mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));

            apSm->state = AP_PORT_SM_RESOLUTION_STATE;
            MCD_STAT_AP_SM_STATE_INSERT(portIndex,AP_PORT_SM_RESOLUTION_STATE);

            /* Ability timer start */
            mvPortCtrlThresholdSet(apIntrop->abilityDuration, &(apTimer->abilityThreshold));

            /* AP register dump */
            mvPortCtrlDbgAllRegsDump(portIndex, phyPcsNum , AP_PORT_SM_TX_DISABLE_STATE);
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
    MCD_AP_SM_INFO       *apSm     = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info);
    MCD_AP_SM_TIMER      *apTimer  = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].timer);
    MCD_AP_DETECT_ITEROP *apIntrop = &(mvHwsPortCtrlApPortDetect.introp);
    GT_STATUS rc;
    GT_U8 pcsNum = AP_CTRL_PCS_GET(apSm->ifNum);
 /* PCS MUX set according to HCD resolution */
    apPortMode = AP_ST_HCD_TYPE_GET(apSm->hcdStatus);
    mvApConfig(pcsNum, GT_FALSE);
    mcdDevDb.portConfig[portIndex].portMode = mvHwsApConvertPortMode(apPortMode);

    /* SERDES power down */
    /* AP Port mode parameters update */
    if(mvHwsApSetPortParameters(apSm->portNum, apPortMode) != GT_OK)
    {
        mvPcPrintf("Error, AP , Port-%d Params set Failed\n", apSm->portNum);
        return;
    }

    portMode = mvHwsApConvertPortMode(apPortMode);
    AP_PRINT_MCD_MAC(("mvApPortResolutionSuccess portIndex:%d  apPortMode:0x%x portMode:0x%x pcsNum %x \n",portIndex, apPortMode, portMode, pcsNum));

    /* SERDES Tx Disable - disable signal on lane where managed negotiatio */
    mvHwsSerdesTxEnable(0, 0, AP_CTRL_LANE_GET(apSm->ifNum), HWS_DEV_SERDES_TYPE(0/*devNum*/), GT_FALSE);

    /* Send port init message */
    rc = mvPortCtrlApPortMsgSend(portIndex,
                            (GT_U8)MCD_IPC_PORT_INIT_MSG,
                            (GT_U8)AP_CTRL_QUEUEID_GET(apSm->ifNum)/*apSm->queueId*/,
                            (GT_U8)apSm->portNum,
                            (GT_U8)portMode,
                            (GT_U8)GT_FALSE,
                            (GT_U8)MHz_156,
                            (GT_U8)PRIMARY_LINE_SRC);
    if (rc != GT_OK)
    {
        AP_PRINT_MCD_MAC(("mvApPortResolutionSuccess portIndex:%d  mvPortCtrlApPortMsgSend fail\n", portIndex));
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

    /* AP state & status update */
    apSm->status = AP_PORT_LINK_CHECK_START;
    MCD_STAT_AP_SM_STATUS_INSERT(portIndex,AP_PORT_LINK_CHECK_START);
    mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));

    apSm->state = AP_PORT_SM_ACTIVE_STATE;
    MCD_STAT_AP_SM_STATE_INSERT(portIndex,AP_PORT_SM_ACTIVE_STATE);

    mcdCm3ToHostDoorbell(MCD_AP_DOORBELL_EVENT_PORT_802_3_AP(apSm->portNum));
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
    GT_U32 slice, phyPcsNum;
    MCD_AP_SM_INFO *apSm = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info);
    slice = portIndex/4;
    pcsNum = AP_CTRL_PCS_GET(apSm->ifNum);
    MCD_FW_LOGIC_TO_PHYSICAL_SERDES_MAC((GT_U32)pcsNum,phyPcsNum);

    /* AP ARB state machine - ST_AN_GOOD_CK */
    mvApArbFsmSet(phyPcsNum, ST_AN_GOOD_CK);
    AP_ST_ARB_FSM_SET(apSm->ARMSmStatus, ST_AN_GOOD_CK);

    /* AP state & status update */
    apSm->status = AP_PORT_PD_RESOLUTION_IN_PROGRESS;
    MCD_STAT_AP_SM_STATUS_INSERT(portIndex,AP_PORT_PD_RESOLUTION_IN_PROGRESS);
    mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));

    CHECK_STATUS(genUnitRegisterGet(apDevNum, apPortGroup, MMPCS_UNIT, phyPcsNum,
                                    AP_REG_ST_0, &plData, 0));

    /* Activate the matching symbol-lock bit in AP configuration reg 0/1 */
    switch (plData & AP_REG_ST_0_AP_PWT_UP_MASK)
    {
    case AP_REG_ST_0_AP_PWT_UP_1G_KX_MASK:
        AP_ST_HCD_TYPE_SET(apSm->hcdStatus,Port_1000Base_KX);
        break;

    case AP_REG_ST_0_AP_PWT_UP_10G_KX4_MASK: /* 10G XAUI */
        AP_ST_HCD_TYPE_SET(apSm->hcdStatus, Port_10GBase_KX4);
        break;

    default:
        apSm->status = AP_PORT_PD_RESOLUTION_FAILURE;
        MCD_STAT_AP_SM_STATUS_INSERT(portIndex,AP_PORT_PD_RESOLUTION_FAILURE);
        mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));

        apSm->state = AP_PORT_SM_INIT_STATE;
        MCD_STAT_AP_SM_STATE_INSERT(portIndex,AP_PORT_SM_INIT_STATE);
        return GT_OK;
    }

    AP_ST_HCD_FOUND_SET(apSm->hcdStatus, 1);
    mvPortCtrlLogAdd(AP_PORT_DET_LOG_EXT(apSm->hcdStatus, apSm->portNum));

    /* AP ARB state machine - ST_AN_GOOD_CK */
    mvApArbFsmSet(phyPcsNum, ST_AN_GOOD_CK);
    AP_ST_ARB_FSM_SET(apSm->ARMSmStatus, ST_AN_GOOD_CK);

    /* AP state & status update */
    apSm->status = AP_PORT_RESOLUTION_SUCCESS;
    MCD_STAT_AP_SM_STATUS_INSERT(portIndex,AP_PORT_RESOLUTION_SUCCESS);
    mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));

    /* AP Resolution Success API */
    mvApPortResolutionSuccess(portIndex, PORT_CTRL_PD_LINK_CHECK_MODE);

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
    GT_U32 plData,plData1,plAdv;
    GT_U32 data;
    GT_U32 hcdType = MODE_NOT_SUPPORTED;
    GT_U32 slice,phyPcsNum;
#ifndef RMV_CONSORTIUM
    GT_U32 hcdTypeConsortium = Port_AP_LAST;
    GT_U16 nextPage0, nextPage1, nextPage2;
    GT_U8  consortiumPrio;
#endif /*#ifndef RMV_CONSORTIUM*/
    GT_U8  pcsNum, priority=0, fecType = 0;
    GT_BOOL found = GT_TRUE;
    MCD_AP_SM_INFO       *apSm     = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info);
    MCD_AP_SM_STATS      *apStats  = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].stats);
    MCD_AP_DETECT_ITEROP *apIntrop = &(mvHwsPortCtrlApPortDetect.introp);
    priority = priority;
    pcsNum = AP_CTRL_PCS_GET(apSm->ifNum);
    slice = portIndex/4;
    MCD_FW_LOGIC_TO_PHYSICAL_SERDES_MAC((GT_U32)pcsNum,phyPcsNum);
#ifndef RMV_CONSORTIUM
    if (apSm->status == AP_PORT_AP_RESOLUTION_NO_MORE_PAGES){
        /* read partner next page */
        mvApHcdReadPartnerNp(phyPcsNum, &nextPage0, &nextPage1, &nextPage2);
        AP_PRINT_MCD2_MAC(("mvApPortResolutionAP portIndex:%d  next page:0x%x, 0x%x, 0x%x\n",portIndex, nextPage0, nextPage1, nextPage2));
        /* check partner abilities */
        if (((nextPage0 & AP_INT_REG_802_3_NEXT_PG_TX_REG_OUI_TAGGED_MASK) == AP_INT_REG_802_3_NEXT_PG_TX_REG_OUI_TAGGED_UNFORM_CNS) &&
            ((nextPage0 & AP_INT_REG_802_3_NEXT_PG_TX_REG_MP_MASK) == 0)){
            /* read pertner fec & speed*/
            mvApConsortiumResolutionSet(portIndex, nextPage0, nextPage1, nextPage2);
        }
    }
#endif /*#ifndef RMV_CONSORTIUM*/
    /* AP state & status update */
    apSm->status = AP_PORT_AP_RESOLUTION_IN_PROGRESS;
    MCD_STAT_AP_SM_STATUS_INSERT(portIndex,AP_PORT_AP_RESOLUTION_IN_PROGRESS);
    AP_ST_ARB_FSM_SET(apSm->ARMSmStatus, ST_AN_GOOD_CK);
    mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));
    mvPortCtrlDbgOnDemandRegsDump(portIndex, phyPcsNum, apSm->state);
#if 0 /*code below - trace for debug only*/
    genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_REG_CFG_0 , &plData, 0);
    osPrintf("%d AP_REG_CFG_0  =%x\n",portIndex,plData);
    genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_REG_CFG_1 , &plData, 0);
    osPrintf("%d AP_REG_CFG_1  =%x\n",portIndex,plData);
    genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_REG_CFG_2 , &plData, 0);
    osPrintf("%d AP_REG_CFG_2  =%x\n",portIndex,plData);
    genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_REG_CFG_3 , &plData, 0);
    osPrintf("%d AP_REG_CFG_3  =%x\n",portIndex,plData);
    genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_INT_REG_802_3_AP_ADV_REG_1, &plData, 0);
    osPrintf("%d advReg1 =%x\n",portIndex,plData);
    genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_INT_REG_802_3_AP_ADV_REG_2, &plData, 0);
    osPrintf("%d advReg2 =%x\n",portIndex,plData);
    genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_INT_REG_802_3_AP_ADV_REG_3, &plData, 0);
    osPrintf("%d advReg3 =%x\n",portIndex,plData);
    genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_INT_REG_802_3_LP_BASE_ABILITY_REG_1, &plData, 0);
    osPrintf("%d alpReg11 =%x\n",portIndex,plData);
    genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_INT_REG_802_3_LP_BASE_ABILITY_REG_2, &plData, 0);
    osPrintf("%d alpReg2 =%x\n",portIndex,plData);
    genUnitRegisterGet(0, 0, MMPCS_UNIT, pcsNum, AP_INT_REG_802_3_LP_BASE_ABILITY_REG_3, &plData, 0);
    osPrintf("%d alpReg3 =%x\n",portIndex,plData);
    CHECK_STATUS(genUnitRegisterGet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                    AP_REG_ST_0, &plData, 0));
    osPrintf("%d plData0 =%x\n",portIndex,plData);
    CHECK_STATUS(genUnitRegisterGet(apDevNum, apPortGroup, MMPCS_UNIT, pcsNum,
                                        AP_REG_ST_1, &plData1, 0));
    osPrintf("%d plData1 =%x\n",portIndex,plData1);
#else
    /* Activate the matching symbol-lock bit in AP configuration */
    CHECK_STATUS(genUnitRegisterGet(apDevNum, apPortGroup, MMPCS_UNIT, phyPcsNum,
                                    AP_REG_ST_0, &plData, 0));

    CHECK_STATUS(genUnitRegisterGet(apDevNum, apPortGroup, MMPCS_UNIT, phyPcsNum,
                                        AP_REG_ST_1, &plData1, 0));
    CHECK_STATUS(genUnitRegisterGet(apDevNum, apPortGroup, MMPCS_UNIT, phyPcsNum,
                                        0x250, &plAdv, 0));
#endif
    AP_PRINT_MCD2_MAC(("Found resolution portIndex:%d plData:0x%x plData1:0x%x plAdv: %x hcdStatus:0X%x\n",portIndex, plData, plData1, plAdv,apSm->hcdStatus));


    /* HCD resolved bit[13] validate */
    if ((plData >> AP_REG_ST_0_HCD_RESOLVED_SHIFT) &
                   AP_REG_ST_0_HCD_RESOLVED_MASK)
    {
        fecType = (plData >> AP_REG_ST_0_HCD_BASE_R_FEC_SHIFT) & AP_REG_ST_0_HCD_BASE_R_FEC_MASK;
        if (plData1 & AP_REG_ST_1_AP_PWT_UP_MASK) {
            if((plData1 & AP_REG_ST_1_AP_PWT_UP_MASK) && (plData & AP_REG_ST_0_AP_PWT_UP_MASK))
            {
                mvPcPrintf("Error, AP , Found 2 hcd\n");
            }
            /* Activate the matching symbol-lock bit in AP configuration */
            switch (plData1 & AP_REG_ST_1_AP_PWT_UP_MASK)
            {
            case AP_REG_ST_1_AP_PWT_UP_100G_CR4_MASK:
                CHECK_STATUS(genUnitRegisterGet(apDevNum, apPortGroup, MMPCS_UNIT, phyPcsNum,
                                                AP_REG_CFG_3, &data, 0));
                CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, phyPcsNum,
                                                AP_REG_CFG_3, data | AP_REG_CFG_3_LOCK_100G_CR4_MASK, 0));
                hcdType = Port_100GBASE_CR4;
                priority = 100;
                /* in 100G we must work with RS fec */
                fecType = AP_ST_HCD_FEC_RES_RS;
                    /*(fecType & AP_REG_ST_0_HCD_BASE_R_FEC_MASK) ? AP_ST_HCD_FEC_RES_RS : AP_ST_HCD_FEC_RES_NONE;*/
                break;

            case AP_REG_ST_1_AP_PWT_UP_100G_KR4_MASK:
                CHECK_STATUS(genUnitRegisterGet(apDevNum, apPortGroup, MMPCS_UNIT, phyPcsNum,
                                                AP_REG_CFG_3, &data, 0));
                CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, phyPcsNum,
                                                AP_REG_CFG_3, data | AP_REG_CFG_3_LOCK_100G_KR4_MASK, 0));
                hcdType = Port_100GBASE_KR4;
                /* in 100G we must work with RS fec */
                fecType = AP_ST_HCD_FEC_RES_RS;
                    /*(fecType & AP_REG_ST_0_HCD_BASE_R_FEC_MASK) ? AP_ST_HCD_FEC_RES_RS : AP_ST_HCD_FEC_RES_NONE;*/
                priority = 100;
                break;

            case AP_REG_ST_1_AP_PWT_UP_100G_KP4_MASK:
                CHECK_STATUS(genUnitRegisterGet(apDevNum, apPortGroup, MMPCS_UNIT, phyPcsNum,
                                                AP_REG_CFG_3, &data, 0));
                CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, phyPcsNum,
                                                AP_REG_CFG_3, data | AP_REG_CFG_3_LOCK_100G_KP4_MASK, 0));
                hcdType = Port_100GBASE_KP4;
                priority = 100;
                /* in 100G we must work with RS fec */
                fecType = AP_ST_HCD_FEC_RES_RS;
                    /*(fecType & AP_REG_ST_0_HCD_BASE_R_FEC_MASK) ? AP_ST_HCD_FEC_RES_RS : AP_ST_HCD_FEC_RES_NONE;*/
                break;

            case AP_REG_ST_1_AP_PWT_UP_25G_MASK:
                CHECK_STATUS(genUnitRegisterGet(apDevNum, apPortGroup, MMPCS_UNIT, phyPcsNum,
                                                AP_REG_CFG_3, &data, 0));
                CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, phyPcsNum,
                                                    AP_REG_CFG_3, data | AP_REG_CFG_3_LOCK_25G_MASK, 0));
                hcdType = Port_25GBASE_KR;
                priority = 25;
                /* 25G support also res fec */
                if ((plData1>> AP_REG_ST_1_HCD_RS_FEC_SHIFT) & AP_REG_ST_1_HCD_RS_FEC_MASK)
                {
                    fecType = AP_ST_HCD_FEC_RES_RS;
                }

                break;
            case AP_REG_ST_1_AP_PWT_UP_25G_S_MASK:
                CHECK_STATUS(genUnitRegisterGet(apDevNum, apPortGroup, MMPCS_UNIT, phyPcsNum,
                                                AP_REG_CFG_3, &data, 0));
                CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, phyPcsNum,
                                                AP_REG_CFG_3, data | AP_REG_CFG_3_LOCK_25G_S_MASK, 0));
                hcdType = Port_25GBASE_KR_S;
                priority = 25;
                /* FC FEC uses "old" bits - no need to update advanced */
                break;
            default:
                found = GT_FALSE;
                break;

            }
        } else if (plData & AP_REG_ST_0_AP_PWT_UP_MASK) {

            switch (plData & AP_REG_ST_0_AP_PWT_UP_MASK)
            {
            case AP_REG_ST_0_AP_PWT_UP_40G_CR4_MASK:
                CHECK_STATUS(genUnitRegisterGet(apDevNum, apPortGroup, MMPCS_UNIT, phyPcsNum,
                                                AP_REG_CFG_0, &data, 0));
                CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, phyPcsNum,
                                                AP_REG_CFG_0, data | AP_REG_CFG_0_LOCK_40G_CR4_MASK, 0));
                hcdType = Port_40GBASE_CR4;
                priority = 40;
                break;

            case AP_REG_ST_0_AP_PWT_UP_40G_KR4_MASK:
                CHECK_STATUS(genUnitRegisterGet(apDevNum, apPortGroup, MMPCS_UNIT, phyPcsNum,
                                                AP_REG_CFG_0, &data, 0));
                CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, phyPcsNum,
                                                AP_REG_CFG_0, data | AP_REG_CFG_0_LOCK_40G_KR4_MASK, 0));
                hcdType = Port_40GBase_R;
                priority = 40;
                break;

            case AP_REG_ST_0_AP_PWT_UP_10G_KR_MASK:
                CHECK_STATUS(genUnitRegisterGet(apDevNum, apPortGroup, MMPCS_UNIT, phyPcsNum,
                                                AP_REG_CFG_1, &data, 0));
                CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, phyPcsNum,
                                                AP_REG_CFG_1, data | AP_REG_CFG_1_LOCK_10G_KR_MASK, 0));
                hcdType = Port_10GBase_R;
                priority = 10;
                break;

            case AP_REG_ST_0_AP_PWT_UP_10G_KX4_MASK: /* 10G XAUI */
                CHECK_STATUS(genUnitRegisterGet(apDevNum, apPortGroup, MMPCS_UNIT, phyPcsNum,
                                                AP_REG_CFG_1, &data, 0));
                CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, phyPcsNum,
                                                AP_REG_CFG_1, data | AP_REG_CFG_1_LOCK_10G_KX4_MASK, 0));
                hcdType = Port_10GBase_KX4;
                priority = 10;
                break;

            case AP_REG_ST_0_AP_PWT_UP_1G_KX_MASK:
                CHECK_STATUS(genUnitRegisterGet(apDevNum, apPortGroup, MMPCS_UNIT, phyPcsNum,
                                                AP_REG_CFG_0, &data, 0));
                CHECK_STATUS(genUnitRegisterSet(apDevNum, apPortGroup, MMPCS_UNIT, phyPcsNum,
                                                AP_REG_CFG_0, data | AP_REG_CFG_0_LOCK_1G_KX_MASK, 0));
                hcdType = Port_1000Base_KX;
                priority = 1;
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
        AP_PRINT_MCD2_MAC(("Found NO resolution for IEEE portIndex:%d  \n",portIndex));
    }
#ifndef RMV_CONSORTIUM
    if (AP_ST_HCD_CONSORTIUM_RES_GET(apSm->hcdStatus)) {
        AP_PRINT_MCD2_MAC(("Found consortium resolution portIndex:%d hcdStatus:0x%x \n",portIndex, apSm->hcdStatus));
        hcdTypeConsortium = AP_ST_HCD_TYPE_GET(apSm->hcdStatus);
        if (found == GT_FALSE){
            hcdType = hcdTypeConsortium;
            fecType = AP_ST_HCD_FEC_RES_GET(apSm->hcdStatus);
            found = GT_TRUE;

        } else {
            consortiumPrio = (hcdTypeConsortium <= Port_25GBASE_CR1_CONSORTIUM)? 25: 50;
            /* if priority is eqal - choose IEEE type */
            if (consortiumPrio > priority) {
                hcdType = hcdTypeConsortium;
                fecType = AP_ST_HCD_FEC_RES_GET(apSm->hcdStatus);
            }
        }
    }
#endif /*#ifndef RMV_CONSORTIUM*/

    if (found)
    {
        /* AP HCD update */
        AP_ST_HCD_FOUND_SET(apSm->hcdStatus, 1);
        AP_ST_HCD_TYPE_SET(apSm->hcdStatus, hcdType);
        AP_ST_HCD_FEC_RES_SET(apSm->hcdStatus, fecType);

        AP_PRINT_MCD_MAC(("Found final resolution portIndex:%d hcdType%x hcdStatus:0x%x\n",portIndex, hcdType, apSm->hcdStatus));
        /* HCD Interrupt update */
        CHECK_STATUS(mvApFcResolutionSet(portIndex));
        mvHwsServCpuIntrSet(portIndex, MV_HWS_PORT_HCD_EVENT | (hcdType << MV_HWS_PORT_HCD_SHIFT));
        AP_ST_HCD_INT_TRIG_SET(apSm->hcdStatus, 1);
        mvPortCtrlLogAdd(AP_PORT_DET_LOG_EXT(apSm->hcdStatus, apSm->portNum));

        /* AP state & status update */
        apSm->status = AP_PORT_RESOLUTION_SUCCESS;
        MCD_STAT_AP_SM_STATUS_INSERT(portIndex,AP_PORT_RESOLUTION_SUCCESS);
        mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));

        /* AP Resolution Success API */
        mvApPortResolutionSuccess(portIndex, PORT_CTRL_AP_LINK_CHECK_MODE);

        /* AP Timing measurment */
        apStats->hcdResoultionTime = mvPortCtrlCurrentTs() - apStats->timestampTime;
        apStats->timestampTime = mvPortCtrlCurrentTs();

        /* AP register dump */
        mvPortCtrlDbgAllRegsDump(portIndex, phyPcsNum, AP_PORT_SM_RESOLUTION_STATE);

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
        MCD_STAT_AP_SM_STATUS_INSERT(portIndex,AP_PORT_AP_NO_RESOLUTION);
        mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));
        mvPortCtrlDbgOnDemandRegsDump(portIndex, phyPcsNum, apSm->state);

        apSm->state = AP_PORT_SM_TX_DISABLE_STATE;
        MCD_STAT_AP_SM_STATE_INSERT(portIndex,AP_PORT_SM_TX_DISABLE_STATE);

        /* AP validate Ability failure case */
        apStats->abilityInvalidCnt++;
        if (apStats->abilityInvalidCnt > apIntrop->abilityFailMaxInterval)
        {
            apSm->status = AP_PORT_AP_RESOLUTION_MAX_LOOP_FAILURE;
            MCD_STAT_AP_SM_STATUS_INSERT(portIndex,AP_PORT_AP_RESOLUTION_MAX_LOOP_FAILURE);
            mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));

            apSm->state = AP_PORT_SM_INIT_STATE;
            MCD_STAT_AP_SM_STATE_INSERT(portIndex,AP_PORT_SM_INIT_STATE);
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
    MCD_AP_SM_INFO       *apSm  = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info);
    GT_U8 pcsNum;
    GT_U32 slice, phyPcsNum;
#ifndef RMV_CONSORTIUM
    GT_U16 nextPage0, nextPage1, nextPage2;
    GT_BOOL supported;
    GT_U8 ack2;
#endif
    /* Extract PCS and Lane number */
    pcsNum  = AP_CTRL_PCS_GET(apSm->ifNum);
    slice = portIndex/4;
    MCD_FW_LOGIC_TO_PHYSICAL_SERDES_MAC((GT_U32)pcsNum,phyPcsNum);
    if (AP_CTRL_ADV_CONSORTIUM_GET(apSm->capability) == 0)
    {
        /* we don't support consortium - need to send NULL next page*/
        mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));
        mvApArbFsmTrapSet(phyPcsNum, ST_COMPLETE_ACK, GT_TRUE);
        mvApHcdSendNpNull(phyPcsNum);
        mvApArbFsmTrapSet(phyPcsNum, ST_COMPLETE_ACK, GT_TRUE);
        return GT_OK;
    }
#ifndef RMV_CONSORTIUM
    AP_PRINT_MCD2_MAC(("ResolutionWaitNP portIndex:%d status:0x%x\n",portIndex, apSm->status));
    switch (apSm->status)
    {
    case AP_PORT_RESOLUTION_IN_PROGRESS:
        /* start of resolution*/
        mvApHcdSendNpConsortiumStart(phyPcsNum, 0);
        apSm->status = AP_PORT_AP_RESOLUTION_CHK_CONSORTIUM;
        MCD_STAT_AP_SM_STATUS_INSERT(portIndex,AP_PORT_AP_RESOLUTION_CHK_CONSORTIUM);
        mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));
        break;

    case AP_PORT_AP_RESOLUTION_CHK_CONSORTIUM:
        /* read partner next page */
        mvApHcdReadPartnerNp(phyPcsNum, &nextPage0, &nextPage1, &nextPage2);
        supported = mvApHcdIsPartnerSupportConsortium(nextPage0, nextPage1, nextPage2);
        if (supported == GT_TRUE)
        {
            ack2 = 1;
        }
        else
        {
            ack2 = 0;
        }
        AP_PRINT_MCD2_MAC(("ResolutionWaitNP portIndex:%d ack2:%d partner next page:0x%x, 0x%x, 0x%x\n",portIndex, ack2, nextPage0, nextPage1, nextPage2));
        mvApHcdSendNpConsortiumStart(phyPcsNum, ack2);
        apSm->status = AP_PORT_AP_RESOLUTION_CHK_ACK2;
        MCD_STAT_AP_SM_STATUS_INSERT(portIndex,AP_PORT_AP_RESOLUTION_CHK_ACK2);
        mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));
        break;

    case AP_PORT_AP_RESOLUTION_CHK_ACK2:
        /* read partner next page */
        mvApHcdReadPartnerNp(phyPcsNum, &nextPage0, &nextPage1, &nextPage2);
        AP_PRINT_MCD2_MAC(("ResolutionWaitNP portIndex:%d  partner next page:0x%x, 0x%x, 0x%x\n",portIndex, nextPage0, nextPage1, nextPage2));
        if (nextPage0 & AP_INT_REG_802_3_NEXT_PG_TX_REG_ACK2_MASK)
        {
            mvApArbFsmTrapSet(phyPcsNum, ST_COMPLETE_ACK, GT_TRUE);

            /* send partner all ports abilities fec/speed */
            mvApHcdFecNpConsortiumParamsSet(phyPcsNum, (GT_U8)AP_CTRL_FEC_ABIL_CONSORTIUM_GET(apSm->options),
                                        (GT_U8)AP_CTRL_FEC_REQ_CONSORTIUM_GET(apSm->options));

            mvApHcdAdvertiseNpConsortiumSet(phyPcsNum, AP_CTRL_ADV_ALL_GET(apSm->capability));

            mvApHcdSendNpConsortium(phyPcsNum, AP_INT_REG_802_3_NEXT_PG_TX_REG_OUI_TAGGED_UNFORM_CNS,
                                    1 /*ack2*/, 0 /*mp*/, 0 /*ack*/, 0 /*np*/);
            mvApArbFsmTrapSet(phyPcsNum, ST_COMPLETE_ACK, GT_TRUE);


            /* check partner abilities */
            if (((nextPage0 & AP_INT_REG_802_3_NEXT_PG_TX_REG_OUI_TAGGED_MASK) == AP_INT_REG_802_3_NEXT_PG_TX_REG_OUI_TAGGED_UNFORM_CNS) &&
                ((nextPage0 & AP_INT_REG_802_3_NEXT_PG_TX_REG_MP_MASK) == 0))
            {
                /* read pertner fec & speed*/
                mvApConsortiumResolutionSet(portIndex, nextPage0, nextPage1, nextPage2);
            }
        }
        else
        {
            /* partner don't support consortium - need to send NULL next page*/
            mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));
            mvApArbFsmTrapSet(phyPcsNum, ST_COMPLETE_ACK, GT_TRUE);
            mvApHcdSendNpNull(phyPcsNum);
            mvApArbFsmTrapSet(phyPcsNum, ST_COMPLETE_ACK, GT_TRUE);
            AP_PRINT_MCD2_MAC(("ResolutionWaitNP portIndex:%d  send null next page\n",portIndex));
        }
        apSm->status = AP_PORT_AP_RESOLUTION_NO_MORE_PAGES;
        MCD_STAT_AP_SM_STATUS_INSERT(portIndex,AP_PORT_AP_RESOLUTION_NO_MORE_PAGES);
        break;

    case AP_PORT_AP_RESOLUTION_NO_MORE_PAGES:
        /* read partner next page */
        mvApHcdReadPartnerNp(phyPcsNum, &nextPage0, &nextPage1, &nextPage2);
        AP_PRINT_MCD2_MAC(("ResolutionWaitNP portIndex:%d  next page:0x%x, 0x%x, 0x%x\n",portIndex, nextPage0, nextPage1, nextPage2));
        /* check partner abilities */
        if (((nextPage0 & AP_INT_REG_802_3_NEXT_PG_TX_REG_OUI_TAGGED_MASK) == AP_INT_REG_802_3_NEXT_PG_TX_REG_OUI_TAGGED_UNFORM_CNS) &&
            ((nextPage0 & AP_INT_REG_802_3_NEXT_PG_TX_REG_MP_MASK) == 0))
        {
            /* read pertner fec & speed*/
            mvApConsortiumResolutionSet(portIndex, nextPage0, nextPage1, nextPage2);
        }
        mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));
        mvApArbFsmTrapSet(phyPcsNum, ST_COMPLETE_ACK, GT_TRUE);
        mvApHcdSendNpNull(phyPcsNum);
        mvApArbFsmTrapSet(phyPcsNum, ST_COMPLETE_ACK, GT_TRUE);
        AP_PRINT_MCD2_MAC(("ResolutionWaitNP portIndex:%d  send null next page\n", portIndex));
        break;
    }
#endif /*#ifndef RMV_CONSORTIUM*/
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
    GT_U32 slice, phyPcsNum;

    MCD_AP_SM_INFO       *apSm     = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info);
    MCD_AP_SM_TIMER      *apTimer  = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].timer);
    MCD_AP_SM_STATS      *apStats  = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].stats);
    MCD_AP_DETECT_ITEROP *apIntrop = &(mvHwsPortCtrlApPortDetect.introp);
    GT_BOOL              bpReached;
    pcsNum = AP_CTRL_PCS_GET(apSm->ifNum);
    slice = portIndex/4;
    /* since we don't have remaping inside the AP unit, need to convert
     the logical pcs number to physical pcs number as well */
    MCD_FW_LOGIC_TO_PHYSICAL_SERDES_MAC((GT_U32)pcsNum,phyPcsNum);

    /* Check Ability detect threshold */
    if (!(mvPortCtrlThresholdCheck(apTimer->abilityThreshold)))
    {
        return GT_OK;
    }

    /* Extract AP ARB state */
    fsmState = mvApArbFsmGet(phyPcsNum);
    if (apSm->ARMSmStatus != fsmState) {
        AP_ST_ARB_FSM_SET(apSm->ARMSmStatus, fsmState);
        apTimer->abilityCount = 0;
    }


    /* Validate Ability detect max interval, in case crossed update state to INIT */
    apTimer->abilityCount++;
    if(apTimer->abilityCount >= apIntrop->abilityMaxInterval)
    {
        /* AP ARB state machine - AN_ENABLE */
        /* set at init state
        mvApArbFsmSet(pcsNum, ST_AN_ENABLE);
        AP_ST_ARB_FSM_SET(apSm->ARMSmStatus, ST_AN_ENABLE);*/

        /* AP state & status update */
        apSm->status = AP_PORT_AP_RESOLUTION_TIMER_FAILURE;
        MCD_STAT_AP_SM_STATUS_INSERT(portIndex,AP_PORT_AP_RESOLUTION_TIMER_FAILURE);
        AP_PRINT_MCD2_MAC(("Resolution portIndex:%d failed status:0x%x\n",portIndex, apSm->status));
        mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));
        apSm->state = AP_PORT_SM_INIT_STATE;
        MCD_STAT_AP_SM_STATE_INSERT(portIndex,AP_PORT_SM_INIT_STATE);
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
        MCD_STAT_AP_SM_STATUS_INSERT(portIndex,AP_PORT_RESOLUTION_IN_PROGRESS);
        AP_PRINT_MCD2_MAC(("Resolution portIndex:%d status:0x%x\n",portIndex, apSm->status));
        mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));
        mvPortCtrlDbgOnDemandRegsDump(portIndex, phyPcsNum, apSm->state);
    }

    AP_PRINT_MCD2_MAC(("Resolution portIndex:%d fsmState:0x%x\n",portIndex, fsmState));
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
        mvApArbFsmBpCheck(phyPcsNum, &bpReached);
        if (bpReached)
        {
            /* trap ST_COMPLETE_ACK state */
            mvApArbFsmTrapSet(phyPcsNum, 0, GT_FALSE);
        }
        CHECK_STATUS(mvApPortResolutionWaitNP(portIndex));
        break;

    case ST_TX_DISABLE:
        AP_PRINT_MCD_MAC(("Resolution portIndex:%d fsmState:0x%x reset port*****\n",portIndex, fsmState));
        apSm->status = AP_PORT_AP_RESOLUTION_TIMER_FAILURE;
        MCD_STAT_AP_SM_STATUS_INSERT(portIndex,AP_PORT_AP_RESOLUTION_TIMER_FAILURE);
        mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));
        apSm->state = AP_PORT_SM_INIT_STATE;
        MCD_STAT_AP_SM_STATE_INSERT(portIndex,AP_PORT_SM_INIT_STATE);
        mvApArbFsmTrapSet(phyPcsNum, 0, GT_FALSE);

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

    MCD_AP_SM_INFO  *apSm    = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info);
    MCD_AP_SM_STATS *apStats = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].stats);

    GT_U8 pcsNum = AP_CTRL_PCS_GET(apSm->ifNum);

    /* This point means 1 of 2:
    ** 1) during port power up, Max link check was exceeded so the port should
    **    be shut down and the AP process will start over.
    ** 2) link was up but now it's down so AP process should start over (after
    **    the port will be shut down).
    */

    /* Link Interrupt status reset */
    mvHwsServCpuIntrStatusReset(portIndex);

    AP_PRINT_MCD_MAC(("mvApPortLinkFailure portIndex:%d \n",portIndex));

    /* Delete the port */
    apSm->status = AP_PORT_DELETE_IN_PROGRESS;
    MCD_STAT_AP_SM_STATUS_INSERT(portIndex,AP_PORT_DELETE_IN_PROGRESS);
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
    MCD_STAT_AP_SM_STATE_INSERT(portIndex,AP_PORT_SM_DELETE_STATE);

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
    MCD_PORT_CTRL_AP_INIT *portInit = &(mvHwsPortCtrlApPortMng.apInitStatus);
    MCD_AP_SM_INFO        *apSm     = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info);
    MCD_AP_SM_TIMER       *apTimer  = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].timer);

    /* AP state & status update */
    if (apSm->status != AP_PORT_LINK_CHECK_VALIDATE)
    {
        apSm->status = AP_PORT_LINK_CHECK_VALIDATE;
        MCD_STAT_AP_SM_STATUS_INSERT(portIndex,AP_PORT_LINK_CHECK_VALIDATE);
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
        AP_PRINT_MCD2_MAC(("mvApPortLinkValidate portIndex:%d reach max retrys %d\n",portIndex, apTimer->linkCount));

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
* @internal mvApPortLinkUp function
* @endinternal
*
* @brief   AP Port Link Up execution sequence
*/
GT_STATUS mvApPortLinkUp(GT_U8 portIndex)
{
    MV_HWS_PORT_STANDARD portMode;
    MV_HWA_AP_PORT_MODE  apPortMode;
    GT_BOOL              linkUp;
    GT_U16               linkCheckDuration;
    GT_U16               linkCheckInterval;
    GT_U8                pcsNum;

    MCD_PORT_CTRL_PORT_SM *portSm   = &(mvHwsPortCtrlApPortMng.apPortSm[portIndex]);
    MCD_AP_SM_INFO        *apSm     = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info);
    MCD_AP_SM_TIMER       *apTimer  = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].timer);
    MCD_AP_SM_STATS       *apStats  = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].stats);
    MCD_AP_DETECT_ITEROP  *apIntrop = &(mvHwsPortCtrlApPortDetect.introp);

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
    }
    else
    {
        linkCheckDuration = apIntrop->apLinkDuration;
        linkCheckInterval = apIntrop->apLinkMaxInterval;
    }


    if (portSm->state == PORT_SM_ACTIVE_STATE)
    {
        /* Extarct Link status */
        mvHwsPortLinkStatusGet(apDevNum, apDevNum, apSm->portNum, portMode, &linkUp);
        /* Port is Active & Link_UP detected */

        if (linkUp)
        {
            /* Link up process - only at first occurance */
            if (apSm->status != AP_PORT_LINK_CHECK_SUCCESS)
            {
                /* AP state & status update */
                apSm->status = AP_PORT_LINK_CHECK_SUCCESS;
                MCD_STAT_AP_SM_STATUS_INSERT(portIndex,AP_PORT_LINK_CHECK_SUCCESS);
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
                mcdCm3ToHostDoorbell(MCD_AP_DOORBELL_EVENT_PORT_STATUS_CHANGE(apSm->portNum));

                /* AP register dump */
                mvPortCtrlDbgCtrlRegsDump(portIndex, pcsNum, AP_PORT_SM_ACTIVE_STATE);
            }

            /* Link check timer restart */
            mvPortCtrlThresholdSet(linkCheckDuration, &(apTimer->linkThreshold));
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
                /* AP state & status update */
                apSm->status = AP_PORT_LINK_CHECK_FAILURE;
                MCD_STAT_AP_SM_STATUS_INSERT(portIndex,AP_PORT_LINK_CHECK_FAILURE);
                mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));
                AP_PRINT_MCD_MAC(("mvApPortLinkUp port:%d was up retimerMode %d\n",portIndex, mcdDevDb.apCfg[portIndex].retimerMode));

                /* Link failure */
                mvApPortLinkFailure(portIndex);
                /*Doorbell interrupt*/
                mcdCm3ToHostDoorbell(MCD_AP_DOORBELL_EVENT_PORT_STATUS_CHANGE(apSm->portNum));

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
    MCD_PORT_CTRL_PORT_SM *portSm  = &(mvHwsPortCtrlApPortMng.apPortSm[portIndex]);
    MCD_AP_SM_INFO        *apSm    = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info);
    MCD_AP_SM_STATS       *apStats = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].stats);

    if (portSm->state == PORT_SM_IDLE_STATE)
    {
        /* AP state & status update */
        apSm->status = AP_PORT_DELETE_SUCCESS;
        MCD_STAT_AP_SM_STATUS_INSERT(portIndex,AP_PORT_DELETE_SUCCESS);
        mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));

        /* AP Timing reset */
        mvApResetStatsTimestamp(portIndex);

        /* AP Timing measurment */
        apStats->timestampTime = mvPortCtrlCurrentTs();

        /* AP state & status update */
        apSm->state = AP_PORT_SM_INIT_STATE;
        MCD_STAT_AP_SM_STATE_INSERT(portIndex,AP_PORT_SM_INIT_STATE);
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
    MCD_AP_SM_INFO *apSm = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info);
    MCD_PORT_CTRL_PORT_SM       *portSm    = &(mvHwsPortCtrlApPortMng.apPortSm[portIndex]);
    GT_U8 pcsNum = AP_CTRL_PCS_GET(apSm->ifNum);
    /*MV_HWS_PORT_STANDARD portMode;
    MV_HWA_AP_PORT_MODE  apPortMode;*/
    /*MCD_AP_SM_STATS      *apStats  = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].stats);*/

    /* Validate AP port state & status */
    if (apSm->state != AP_PORT_SM_IDLE_STATE)
    {
        if ((apSm->state == AP_PORT_SM_ACTIVE_STATE) &&
            (apSm->status != AP_PORT_DELETE_IN_PROGRESS))
        {
            mcdCm3ToHostDoorbell(MCD_AP_DOORBELL_EVENT_PORT_STATUS_CHANGE(apSm->portNum));
            /* AP Port Delete */
            apSm->status = AP_PORT_DELETE_IN_PROGRESS;
            MCD_STAT_AP_SM_STATUS_INSERT(portIndex,AP_PORT_DELETE_IN_PROGRESS);

            /* move to portSm task */
            /* call project callback for additional operations */

            CHECK_STATUS(mvApPortDelete(portIndex));

            AP_PRINT_MCD_MAC(("mvApPortDeleteMsg port:%d \n",portIndex));

        }

        if (apSm->state != AP_PORT_SM_AP_DISABLE_STATE)
        {
            #ifndef MV_BV_PCS
                /* PCS MUX to non AP mode */
                 mvApPcsMuxReset(pcsNum);
             #else
                mvApConfig(pcsNum, GT_FALSE);
            #endif
                /* AP Reset All */
                mvApResetStatus(portIndex);
                mvApResetTimer(portIndex);
                mvApResetStats(portIndex);
                mvApResetStatsTimestamp(portIndex);

                /* AP ARB state machine - AN_ENABLE */
                AP_ST_ARB_FSM_SET(apSm->ARMSmStatus, ST_AN_ENABLE);

                /* AP state & status update */
                mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));
                apSm->state = AP_PORT_SM_AP_DISABLE_STATE;
                MCD_STAT_AP_SM_STATE_INSERT(portIndex,AP_PORT_SM_AP_DISABLE_STATE);
                mvPortCtrlLogAdd(AP_PORT_SM_LOG(apSm->state, apSm->status, apSm->portNum, apSm->ARMSmStatus));
        }

        if (portSm->state == PORT_SM_IDLE_STATE)
        {
            apSm->state  = AP_PORT_SM_IDLE_STATE;
            MCD_STAT_AP_SM_STATE_INSERT(portIndex,AP_PORT_SM_IDLE_STATE);
        }

    }

    return GT_OK;
}

/*******************************************************************************
*                            AP Port Log State/Status                          *
* ============================================================================ *
*******************************************************************************/

/**
* @internal mcdStatApSmStateInsert function
* @endinternal
*
* @brief   AP Port Log State/Status Insert
*          Keeping the AP_SM state information coded in the argument
*          Binary 0000 0001 etc ... - means according to the MV_AP_PORT_SM_STATE above
*          This function must be updated if the State # will raise above 15 (4 bits coded)
*/
void  mcdStatApSmStateInsert(GT_U8 portIndex, GT_U16 state)
{
#ifdef  MCD_AP_STATE_STATUS_LOG
    MCD_AP_SM_DECODE(portIndex)      = ((MCD_AP_SM_DECODE(portIndex) << 4) | ((MCD_U16)state & 0xF));
    MCD_AP_SM_DECODE_TYPE(portIndex) = ((MCD_AP_SM_DECODE_TYPE(portIndex) << 1) | ((MCD_U16)1));
#endif  /*MCD_AP_STATE_STATUS_LOG*/
}


/**
* @internal mcdStatApSmStatusInsert function
* @endinternal
*
* @brief   AP Port Log Status Insert
*          Keeping the AP_SM state information coded in the argument
*          Binary 00000 00001 00010 00011... - means according to the MV_AP_PORT_SM_STATUS above
*          This function must be updated if the Status # will raise above 31 (5 bits coded)
*/
void  mcdStatApSmStatusInsert(GT_U8 portIndex, GT_U16 status)
{
#ifdef  MCD_AP_STATE_STATUS_LOG
    MCD_AP_SM_DECODE(portIndex) = ((MCD_AP_SM_DECODE(portIndex) << 5) | ((MCD_U16)status & 0x1F));
    MCD_AP_SM_DECODE_TYPE(portIndex) = ((MCD_AP_SM_DECODE_TYPE(portIndex) << 1));
#endif  /*MCD_AP_STATE_STATUS_LOG*/
}

