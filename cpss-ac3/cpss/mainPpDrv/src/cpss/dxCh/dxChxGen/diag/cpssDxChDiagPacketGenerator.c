/*******************************************************************************
*              (c), Copyright 2006, Marvell International Ltd.                 *
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
* @file cpssDxChDiagPacketGenerator.c
*
* @brief Diag Packet Generator APIs for CPSS DxCh.
*
* Sequence for generating packets on port:
* 1. Connect the port to packet generator and set configurations
* (cpssDxChDiagPacketGeneratorConnectSet)
* 2. Start transmission (cpssDxChDiagPacketGeneratorTransmitEnable)
* 3. If single burst transmit mode used:
* Check transmission done status (cpssDxChDiagPacketGeneratorBurstTransmitStatusGet)
* If continues transmit mode used:
* Stop transmission (cpssDxChDiagPacketGeneratorTransmitEnable)
* 4. Return the port to normal operation, disconnect the packet generator
* (cpssDxChDiagPacketGeneratorConnectSet)
*
* @version   15
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/diag/private/prvCpssDxChDiagLog.h>
#include <cpss/dxCh/dxChxGen/diag/cpssDxChDiagPacketGenerator.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortCtrl.h>
#include <cpss/common/labServices/port/gop/port/private/mvPortModeElements.h>
#include <cpss/generic/private/utils/prvCpssUnitGenDrv.h>



/******************************************************************************
 * specific driver : MAC-TG driver
 ******************************************************************************/
/*
 * enum: struct PRV_CPSS_TG_DRV_FLD_ID_ENT
 *
 * Description: enum that describes fields of Mac-TG
 *
 *    PRV_CPSS_TG_FLD_CONNECT_EN_E         - connect enable field
 *    PRV_CPSS_TG_FLD_CONNECTED_PORT_E     - connected port
 *    PRV_CPSS_TG_FLD_START_TRIGGER_E      - start trasmission (either continues or burst )
 *    PRV_CPSS_TG_FLD_STOP_TRIGGER_E       - stop  trasmission (continues)
 *    PRV_CPSS_TG_FLD_BIRST_TRANSM_DONE_E  - burst trasmission done.
 *    PRV_CPSS_TG_FLD_MAC_DA_0_E           - mac destination address register 0
 *    PRV_CPSS_TG_FLD_MAC_DA_1_E           - mac destination address register 1
 *    PRV_CPSS_TG_FLD_MAC_DA_2_E           - mac destination address register 2
 *    PRV_CPSS_TG_FLD_MAC_DA_INC_EN_E      - mac destination address increment enable
 *    PRV_CPSS_TG_FLD_MAC_DA_INC_LIM_E     - mac destination address increment limit (bobcat2 specific)
 *    PRV_CPSS_TG_FLD_MAC_SA_0_E           - mac source address register 0
 *    PRV_CPSS_TG_FLD_MAC_SA_1_E           - mac source address register 1
 *    PRV_CPSS_TG_FLD_MAC_SA_2_E           - mac source address register 2
 *    PRV_CPSS_TG_FLD_VLAN_TAG_VID_E       - vlan tag VID (vlan id)
 *    PRV_CPSS_TG_FLD_VLAN_TAG_CFI_E       - vlan tag CFI (conjection drop enable)
 *    PRV_CPSS_TG_FLD_VLAN_TAG_VPT_E       - vlan tag VPT (priority)
 *    PRV_CPSS_TG_FLD_ETHER_TYPE_E         - ethrnet type
 *    PRV_CPSS_TG_FLD_TRANS_MODE_E         - transmittion mode (burst/continues)
 *    PRV_CPSS_TG_FLD_PKT_LENGTH_TYPE_E    - packet length type ( constant/random)
 *    PRV_CPSS_TG_FLD_PKT_LENGTH_E         - packet length
 *    PRV_CPSS_TG_FLD_PKT_UNDERSIZE_EN_E   - undersize enable
 *    PRV_CPSS_TG_FLD_PKT_CNT_E            - packet counter
 *    PRV_CPSS_TG_FLD_PKT_CNT_MULTIPLIER_E - packet counter multiplier
 *    PRV_CPSS_TG_FLD_IFG_E                - inter frame gap
 *    PRV_CPSS_TG_FLD_INTERFACE_SIZE_E     - interface size (bobcat2 specific)
 *    PRV_CPSS_TG_FLD_PAYLOAD_TYPE_E       - payload type (cyclic/random/constatnt - (bobcat2 specific))
 *    PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E      - paylload cyclic patter register 0-3  (both lion2 & bobcat2)
 *    PRV_CPSS_TG_FLD_PAYLOAD_REG_31_E = P - paylload cyclic patter register 4-31 (bobcat2 specific)
 *    PRV_CPSS_TG_FLD_MAX
 *
 */


typedef enum PRV_CPSS_TG_DRV_FLD_ID_ENT
{
     PRV_CPSS_TG_FLD_CONNECT_EN_E = 0
    ,PRV_CPSS_TG_FLD_CONNECTED_PORT_E
    ,PRV_CPSS_TG_FLD_START_TRIGGER_E
    ,PRV_CPSS_TG_FLD_STOP_TRIGGER_E
    ,PRV_CPSS_TG_FLD_BIRST_TRANSM_DONE_E
    ,PRV_CPSS_TG_FLD_MAC_DA_0_E
    ,PRV_CPSS_TG_FLD_MAC_DA_1_E
    ,PRV_CPSS_TG_FLD_MAC_DA_2_E
    ,PRV_CPSS_TG_FLD_MAC_DA_INC_EN_E
    ,PRV_CPSS_TG_FLD_MAC_DA_INC_LIM_E
    ,PRV_CPSS_TG_FLD_MAC_SA_0_E
    ,PRV_CPSS_TG_FLD_MAC_SA_1_E
    ,PRV_CPSS_TG_FLD_MAC_SA_2_E
    ,PRV_CPSS_TG_FLD_VLAN_TAG_VID_E
    ,PRV_CPSS_TG_FLD_VLAN_TAG_CFI_E
    ,PRV_CPSS_TG_FLD_VLAN_TAG_VPT_E
    ,PRV_CPSS_TG_FLD_ETHER_TYPE_E
    ,PRV_CPSS_TG_FLD_TRANS_MODE_E
    ,PRV_CPSS_TG_FLD_PKT_LENGTH_TYPE_E
    ,PRV_CPSS_TG_FLD_PKT_LENGTH_E
    ,PRV_CPSS_TG_FLD_PKT_UNDERSIZE_EN_E
    ,PRV_CPSS_TG_FLD_PKT_CNT_E
    ,PRV_CPSS_TG_FLD_PKT_CNT_MULTIPLIER_E
    ,PRV_CPSS_TG_FLD_IFG_E
    ,PRV_CPSS_TG_FLD_INTERFACE_SIZE_E
    ,PRV_CPSS_TG_FLD_PAYLOAD_TYPE_E
    ,PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E
    ,PRV_CPSS_TG_FLD_PAYLOAD_REG_31_E = PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E + 31
    ,PRV_CPSS_TG_FLD_MAX           /* never cross this boundary */
}PRV_CPSS_TG_DRV_FLD_ID_ENT;


typedef struct PRV_CPSS_DXCH_MAC_TG_DRV_STC
{
    PRV_CPSS_DXCH_DRV_STC               genDrv;
    PRV_CPSS_DRV_FLD_DEF_STC            fldDefList[PRV_CPSS_TG_FLD_MAX];
}PRV_CPSS_DXCH_MAC_TG_DRV_STC;


/*------------------------------------------
  where the register address are stored
   PRV_CPSS_DXCH_MAC_PACKET_GEN_CONFIG_STC - Lion2
   PRV_CPSS_DXCH_MAC_PACKET_GEN_VER1_CONFIG_STC - Bobcat2
*/
typedef struct PRV_CPSS_DXCH_MAC_PACKET_GEN_CONFIG_STC       packGenConfig_STC;
typedef struct PRV_CPSS_DXCH_MAC_PACKET_GEN_VER1_CONFIG_STC  packGenConfigVer1_STC;

#define FLD_OFF(STR,fld) (GT_U32)(((GT_CHAR *)(&((STR*)0)->fld))-((GT_CHAR *)0))



PRV_CPSS_DRV_FLD_INIT_STC           prv_lion2FldInitStc[] =
{
    /*---------------------------------------------------------------------------------------------------------------------------------*/
    /* fld id                             {          reg off in reg struct,           fld-offs,len }, {min/maxValue},   name           */
    /*---------------------------------------------------------------------------------------------------------------------------------*/
     { PRV_CPSS_TG_FLD_CONNECTED_PORT_E,     { FLD_OFF(packGenConfig_STC,controlReg0),         10,  3 }, { 0,       3 },   "Connected MAC" }
    ,{ PRV_CPSS_TG_FLD_CONNECT_EN_E,         { FLD_OFF(packGenConfig_STC,controlReg0),          0,  1 }, { 0,       1 },   "CONNECT EB"    }

    ,{ PRV_CPSS_TG_FLD_START_TRIGGER_E,      { FLD_OFF(packGenConfig_STC,controlReg0),          4,  1 }, { 1,       1 },   "Start Trigger" }
    ,{ PRV_CPSS_TG_FLD_STOP_TRIGGER_E,       { FLD_OFF(packGenConfig_STC,controlReg0),          5,  1 }, { 1,       1 },   "Stop  Trigger" }
    ,{ PRV_CPSS_TG_FLD_BIRST_TRANSM_DONE_E,  { FLD_OFF(packGenConfig_STC,controlReg0),          6,  1 }, { 0,       1 },   "Birst Transm Done" }

    ,{ PRV_CPSS_TG_FLD_MAC_DA_0_E,           { FLD_OFF(packGenConfig_STC,macDa[0]),             0, 16 }, { 0,  0xFFFF },   "DA 0"          }
    ,{ PRV_CPSS_TG_FLD_MAC_DA_1_E,           { FLD_OFF(packGenConfig_STC,macDa[1]),             0, 16 }, { 0,  0xFFFF },   "DA 1"          }
    ,{ PRV_CPSS_TG_FLD_MAC_DA_2_E,           { FLD_OFF(packGenConfig_STC,macDa[2]),             0, 16 }, { 0,  0xFFFF },   "DA 2"          }

    ,{ PRV_CPSS_TG_FLD_MAC_DA_INC_EN_E,      { FLD_OFF(packGenConfig_STC,controlReg0),          7,  1 }, { 0,       1 },   "DA INC EN"     }

    ,{ PRV_CPSS_TG_FLD_MAC_SA_0_E,           { FLD_OFF(packGenConfig_STC,macSa[0]),             0, 16 }, { 0,  0xFFFF },   "SA 0"          }
    ,{ PRV_CPSS_TG_FLD_MAC_SA_1_E,           { FLD_OFF(packGenConfig_STC,macSa[1]),             0, 16 }, { 0,  0xFFFF },   "SA 1"          }
    ,{ PRV_CPSS_TG_FLD_MAC_SA_2_E,           { FLD_OFF(packGenConfig_STC,macSa[2]),             0, 16 }, { 0,  0xFFFF },   "SA 2"          }

    ,{ PRV_CPSS_TG_FLD_VLAN_TAG_VID_E,       { FLD_OFF(packGenConfig_STC,vlanTag),              0, 12 }, { 0,  0xFFF  },   "VPT"           }
    ,{ PRV_CPSS_TG_FLD_VLAN_TAG_CFI_E,       { FLD_OFF(packGenConfig_STC,vlanTag),             12,  1 }, { 0,      1  },   "CFI"           }
    ,{ PRV_CPSS_TG_FLD_VLAN_TAG_VPT_E,       { FLD_OFF(packGenConfig_STC,vlanTag),             13,  3 }, { 0,      7  },   "VID"           }

    ,{ PRV_CPSS_TG_FLD_ETHER_TYPE_E,         { FLD_OFF(packGenConfig_STC,etherType),            0, 16 }, { 0,  0xFFFF },   "ETHER TYPE"    }
    ,{ PRV_CPSS_TG_FLD_TRANS_MODE_E,         { FLD_OFF(packGenConfig_STC,controlReg0),          1,  1 }, { 0,       1 },   "TRANSM MODE"   }

    ,{ PRV_CPSS_TG_FLD_PKT_LENGTH_TYPE_E,    { FLD_OFF(packGenConfig_STC,controlReg0),          2,  1 }, { 0,       1 },   "PKT LEN TYPE"  }
    ,{ PRV_CPSS_TG_FLD_PKT_LENGTH_E,         { FLD_OFF(packGenConfig_STC,packetLength),         0, 14 }, { 0,  0x3FFF },   "PKT LEN"       }
    ,{ PRV_CPSS_TG_FLD_PKT_UNDERSIZE_EN_E,   { FLD_OFF(packGenConfig_STC,controlReg0),          9,  1 }, { 0,       1 },   "UNDERISZE EN"  }

    ,{ PRV_CPSS_TG_FLD_PKT_CNT_E,            { FLD_OFF(packGenConfig_STC,packetCount),          0, 13 }, { 0,  0x1FFF },   "PKT CNT"       }
    ,{ PRV_CPSS_TG_FLD_PKT_CNT_MULTIPLIER_E, { FLD_OFF(packGenConfig_STC,packetCount),         13,  3 }, { 0,       7 },   "PKT CNT MLT"   }

    ,{ PRV_CPSS_TG_FLD_IFG_E,                { FLD_OFF(packGenConfig_STC,ifg),                 0,  16 }, { 0,  0xFFFF },   "IFG"           }

    ,{ PRV_CPSS_TG_FLD_PAYLOAD_TYPE_E,       { FLD_OFF(packGenConfig_STC,controlReg0),         3,   1 }, { 0,       1 },   "PAYLOAD TYPE"  }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E,      { FLD_OFF(packGenConfig_STC,dataPattern[0]),      0,  16 }, { 0,  0xFFFF },   "DATA PATTERN 0" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+1,    { FLD_OFF(packGenConfig_STC,dataPattern[1]),      0,  16 }, { 0,  0xFFFF },   "DATA PATTERN 1" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+2,    { FLD_OFF(packGenConfig_STC,dataPattern[2]),      0,  16 }, { 0,  0xFFFF },   "DATA PATTERN 2" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+3,    { FLD_OFF(packGenConfig_STC,dataPattern[3]),      0,  16 }, { 0,  0xFFFF },   "DATA PATTERN 3" }

    ,{ PRV_BAD_VAL,                       {                        PRV_BAD_VAL,    PRV_BAD_VAL, PRV_BAD_VAL }, { PRV_BAD_VAL, PRV_BAD_VAL},   NULL  }
};


PRV_CPSS_DRV_FLD_INIT_STC           prv_bobcat2FldInitStc[] =
{
    /*---------------------------------------------------------------------------------------------------------------------------------*/
    /* fld id                             {          reg off in reg struct,           fld-offs,len }, {min/maxValue},   name           */
    /*---------------------------------------------------------------------------------------------------------------------------------*/
     { PRV_CPSS_TG_FLD_CONNECTED_PORT_E,     { FLD_OFF(packGenConfigVer1_STC,controlReg0),         10,  3 }, { 0,       3 },   "Connected MAC" }
    ,{ PRV_CPSS_TG_FLD_CONNECT_EN_E,         { FLD_OFF(packGenConfigVer1_STC,controlReg0),          0,  1 }, { 0,       1 },   "CONNECT EB"    }

    ,{ PRV_CPSS_TG_FLD_START_TRIGGER_E,      { FLD_OFF(packGenConfigVer1_STC,controlReg0),          4,  1 }, { 1,       1 },   "Start Trigger" }
    ,{ PRV_CPSS_TG_FLD_STOP_TRIGGER_E,       { FLD_OFF(packGenConfigVer1_STC,controlReg0),          5,  1 }, { 1,       1 },   "Stop  Trigger" }
    ,{ PRV_CPSS_TG_FLD_BIRST_TRANSM_DONE_E,  { FLD_OFF(packGenConfigVer1_STC,controlReg0),          6,  1 }, { 0,       1 },   "Birst Transm Done" }

    ,{ PRV_CPSS_TG_FLD_MAC_DA_0_E,           { FLD_OFF(packGenConfigVer1_STC,macDa[0]),             0, 16 }, { 0,  0xFFFF },   "DA 0"          }
    ,{ PRV_CPSS_TG_FLD_MAC_DA_1_E,           { FLD_OFF(packGenConfigVer1_STC,macDa[1]),             0, 16 }, { 0,  0xFFFF },   "DA 1"          }
    ,{ PRV_CPSS_TG_FLD_MAC_DA_2_E,           { FLD_OFF(packGenConfigVer1_STC,macDa[2]),             0, 16 }, { 0,  0xFFFF },   "DA 2"          }

    ,{ PRV_CPSS_TG_FLD_MAC_DA_INC_EN_E,      { FLD_OFF(packGenConfigVer1_STC,controlReg0),          7,  1 }, { 0,       1 },   "DA INC EN"     }
    ,{ PRV_CPSS_TG_FLD_MAC_DA_INC_LIM_E,     { FLD_OFF(packGenConfigVer1_STC,macDaIncrementLimit),  0, 16 }, { 0,  0xFFFF },   "DA INC LIM"    }

    ,{ PRV_CPSS_TG_FLD_MAC_SA_0_E,           { FLD_OFF(packGenConfigVer1_STC,macSa[0]),             0, 16 }, { 0,  0xFFFF },   "SA 0"          }
    ,{ PRV_CPSS_TG_FLD_MAC_SA_1_E,           { FLD_OFF(packGenConfigVer1_STC,macSa[1]),             0, 16 }, { 0,  0xFFFF },   "SA 1"          }
    ,{ PRV_CPSS_TG_FLD_MAC_SA_2_E,           { FLD_OFF(packGenConfigVer1_STC,macSa[2]),             0, 16 }, { 0,  0xFFFF },   "SA 2"          }

    ,{ PRV_CPSS_TG_FLD_VLAN_TAG_VID_E,       { FLD_OFF(packGenConfigVer1_STC,vlanTag),              0, 12 }, { 0,  0xFFF  },   "VPT"           }
    ,{ PRV_CPSS_TG_FLD_VLAN_TAG_CFI_E,       { FLD_OFF(packGenConfigVer1_STC,vlanTag),             12,  1 }, { 0,      1  },   "CFI"           }
    ,{ PRV_CPSS_TG_FLD_VLAN_TAG_VPT_E,       { FLD_OFF(packGenConfigVer1_STC,vlanTag),             13,  3 }, { 0,      7  },   "VID"           }

    ,{ PRV_CPSS_TG_FLD_ETHER_TYPE_E,         { FLD_OFF(packGenConfigVer1_STC,etherType),            0, 16 }, { 0,  0xFFFF },   "ETHER TYPE"    }
    ,{ PRV_CPSS_TG_FLD_TRANS_MODE_E,         { FLD_OFF(packGenConfigVer1_STC,controlReg0),          1,  1 }, { 0,       1 },   "TRANSM MODE"   }

    ,{ PRV_CPSS_TG_FLD_PKT_LENGTH_TYPE_E,    { FLD_OFF(packGenConfigVer1_STC,controlReg0),          2,  1 }, { 0,       1 },   "PKT LEN TYPE"  }
    ,{ PRV_CPSS_TG_FLD_PKT_LENGTH_E,         { FLD_OFF(packGenConfigVer1_STC,packetLength),         0, 14 }, { 0,  0x3FFF },   "PKT LEN"       }
    ,{ PRV_CPSS_TG_FLD_PKT_UNDERSIZE_EN_E,   { FLD_OFF(packGenConfigVer1_STC,controlReg0),          9,  1 }, { 0,       1 },   "UNDERISZE EN"  }

    ,{ PRV_CPSS_TG_FLD_PKT_CNT_E,            { FLD_OFF(packGenConfigVer1_STC,packetCount),          0, 13 }, { 0,  0x1FFF },   "PKT CNT"       }
    ,{ PRV_CPSS_TG_FLD_PKT_CNT_MULTIPLIER_E, { FLD_OFF(packGenConfigVer1_STC,packetCount),         13,  3 }, { 0,       7 },   "PKT CNT MLT"   }

    ,{ PRV_CPSS_TG_FLD_IFG_E,                { FLD_OFF(packGenConfigVer1_STC,ifg),                  0, 16 }, { 0,  0xFFFF },   "IFG"           }
    ,{ PRV_CPSS_TG_FLD_INTERFACE_SIZE_E,     { FLD_OFF(packGenConfigVer1_STC,controlReg0),         13,  3 }, { 0,       7 },   "IF WIDTH"      }

    ,{ PRV_CPSS_TG_FLD_PAYLOAD_TYPE_E,       { FLD_OFF(packGenConfigVer1_STC,packetLength),        14,  2 }, { 0,       2 },   "PAYLOAD TYPE"  }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+ 0,   { FLD_OFF(packGenConfigVer1_STC,dataPattern[ 0]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN  0" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+ 1,   { FLD_OFF(packGenConfigVer1_STC,dataPattern[ 1]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN  1" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+ 2,   { FLD_OFF(packGenConfigVer1_STC,dataPattern[ 2]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN  2" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+ 3,   { FLD_OFF(packGenConfigVer1_STC,dataPattern[ 3]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN  3" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+ 4,   { FLD_OFF(packGenConfigVer1_STC,dataPattern[ 4]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN  4" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+ 5,   { FLD_OFF(packGenConfigVer1_STC,dataPattern[ 5]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN  5" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+ 6,   { FLD_OFF(packGenConfigVer1_STC,dataPattern[ 6]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN  6" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+ 7,   { FLD_OFF(packGenConfigVer1_STC,dataPattern[ 7]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN  7" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+ 8,   { FLD_OFF(packGenConfigVer1_STC,dataPattern[ 8]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN  8" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+ 9,   { FLD_OFF(packGenConfigVer1_STC,dataPattern[ 9]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN  9" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+10,   { FLD_OFF(packGenConfigVer1_STC,dataPattern[10]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN 10" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+11,   { FLD_OFF(packGenConfigVer1_STC,dataPattern[11]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN 11" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+12,   { FLD_OFF(packGenConfigVer1_STC,dataPattern[12]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN 12" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+13,   { FLD_OFF(packGenConfigVer1_STC,dataPattern[13]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN 13" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+14,   { FLD_OFF(packGenConfigVer1_STC,dataPattern[14]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN 14" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+15,   { FLD_OFF(packGenConfigVer1_STC,dataPattern[15]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN 15" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+16,   { FLD_OFF(packGenConfigVer1_STC,dataPattern[16]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN 16" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+17,   { FLD_OFF(packGenConfigVer1_STC,dataPattern[17]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN 17" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+18,   { FLD_OFF(packGenConfigVer1_STC,dataPattern[18]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN 18" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+19,   { FLD_OFF(packGenConfigVer1_STC,dataPattern[19]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN 19" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+20,   { FLD_OFF(packGenConfigVer1_STC,dataPattern[20]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN 20" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+21,   { FLD_OFF(packGenConfigVer1_STC,dataPattern[21]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN 21" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+22,   { FLD_OFF(packGenConfigVer1_STC,dataPattern[22]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN 22" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+23,   { FLD_OFF(packGenConfigVer1_STC,dataPattern[23]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN 23" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+24,   { FLD_OFF(packGenConfigVer1_STC,dataPattern[24]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN 24" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+25,   { FLD_OFF(packGenConfigVer1_STC,dataPattern[25]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN 25" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+26,   { FLD_OFF(packGenConfigVer1_STC,dataPattern[26]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN 26" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+27,   { FLD_OFF(packGenConfigVer1_STC,dataPattern[27]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN 27" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+28,   { FLD_OFF(packGenConfigVer1_STC,dataPattern[28]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN 28" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+29,   { FLD_OFF(packGenConfigVer1_STC,dataPattern[29]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN 29" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+30,   { FLD_OFF(packGenConfigVer1_STC,dataPattern[30]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN 30" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+31,   { FLD_OFF(packGenConfigVer1_STC,dataPattern[31]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN 31" }

    ,{ PRV_BAD_VAL,                       {                        PRV_BAD_VAL,    PRV_BAD_VAL, PRV_BAD_VAL }, { PRV_BAD_VAL, PRV_BAD_VAL},   NULL  }
};

PRV_CPSS_DRV_FLD_INIT_STC           prv_xcat3FldInitStc[] =
{
    /*---------------------------------------------------------------------------------------------------------------------------------*/
    /* fld id                             {          reg off in reg struct,           fld-offs,len }, {min/maxValue},   name           */
    /*---------------------------------------------------------------------------------------------------------------------------------*/
     { PRV_CPSS_TG_FLD_CONNECTED_PORT_E,     { FLD_OFF(packGenConfig_STC,controlReg0),         10,  3 }, { 0,       3 },   "Connected MAC" }
    ,{ PRV_CPSS_TG_FLD_CONNECT_EN_E,         { FLD_OFF(packGenConfig_STC,controlReg0),          0,  1 }, { 0,       1 },   "CONNECT EB"    }

    ,{ PRV_CPSS_TG_FLD_START_TRIGGER_E,      { FLD_OFF(packGenConfig_STC,controlReg0),          4,  1 }, { 1,       1 },   "Start Trigger" }
    ,{ PRV_CPSS_TG_FLD_STOP_TRIGGER_E,       { FLD_OFF(packGenConfig_STC,controlReg0),          5,  1 }, { 1,       1 },   "Stop  Trigger" }
    ,{ PRV_CPSS_TG_FLD_BIRST_TRANSM_DONE_E,  { FLD_OFF(packGenConfig_STC,controlReg0),          6,  1 }, { 0,       1 },   "Birst Transm Done" }

    ,{ PRV_CPSS_TG_FLD_MAC_DA_0_E,           { FLD_OFF(packGenConfig_STC,macDa[0]),             0, 16 }, { 0,  0xFFFF },   "DA 0"          }
    ,{ PRV_CPSS_TG_FLD_MAC_DA_1_E,           { FLD_OFF(packGenConfig_STC,macDa[1]),             0, 16 }, { 0,  0xFFFF },   "DA 1"          }
    ,{ PRV_CPSS_TG_FLD_MAC_DA_2_E,           { FLD_OFF(packGenConfig_STC,macDa[2]),             0, 16 }, { 0,  0xFFFF },   "DA 2"          }

    ,{ PRV_CPSS_TG_FLD_MAC_DA_INC_EN_E,      { FLD_OFF(packGenConfig_STC,controlReg0),          7,  1 }, { 0,       1 },   "DA INC EN"     }
    ,{ PRV_CPSS_TG_FLD_MAC_DA_INC_LIM_E,     { FLD_OFF(packGenConfig_STC,macDaIncrementLimit),  0, 16 }, { 0,  0xFFFF },   "DA INC LIM"    }

    ,{ PRV_CPSS_TG_FLD_MAC_SA_0_E,           { FLD_OFF(packGenConfig_STC,macSa[0]),             0, 16 }, { 0,  0xFFFF },   "SA 0"          }
    ,{ PRV_CPSS_TG_FLD_MAC_SA_1_E,           { FLD_OFF(packGenConfig_STC,macSa[1]),             0, 16 }, { 0,  0xFFFF },   "SA 1"          }
    ,{ PRV_CPSS_TG_FLD_MAC_SA_2_E,           { FLD_OFF(packGenConfig_STC,macSa[2]),             0, 16 }, { 0,  0xFFFF },   "SA 2"          }

    ,{ PRV_CPSS_TG_FLD_VLAN_TAG_VID_E,       { FLD_OFF(packGenConfig_STC,vlanTag),              0, 12 }, { 0,  0xFFF  },   "VPT"           }
    ,{ PRV_CPSS_TG_FLD_VLAN_TAG_CFI_E,       { FLD_OFF(packGenConfig_STC,vlanTag),             12,  1 }, { 0,      1  },   "CFI"           }
    ,{ PRV_CPSS_TG_FLD_VLAN_TAG_VPT_E,       { FLD_OFF(packGenConfig_STC,vlanTag),             13,  3 }, { 0,      7  },   "VID"           }

    ,{ PRV_CPSS_TG_FLD_ETHER_TYPE_E,         { FLD_OFF(packGenConfig_STC,etherType),            0, 16 }, { 0,  0xFFFF },   "ETHER TYPE"    }
    ,{ PRV_CPSS_TG_FLD_TRANS_MODE_E,         { FLD_OFF(packGenConfig_STC,controlReg0),          1,  1 }, { 0,       1 },   "TRANSM MODE"   }

    ,{ PRV_CPSS_TG_FLD_PKT_LENGTH_TYPE_E,    { FLD_OFF(packGenConfig_STC,controlReg0),          2,  1 }, { 0,       1 },   "PKT LEN TYPE"  }
    ,{ PRV_CPSS_TG_FLD_PKT_LENGTH_E,         { FLD_OFF(packGenConfig_STC,packetLength),         0, 14 }, { 0,  0x3FFF },   "PKT LEN"       }
    ,{ PRV_CPSS_TG_FLD_PKT_UNDERSIZE_EN_E,   { FLD_OFF(packGenConfig_STC,controlReg0),          9,  1 }, { 0,       1 },   "UNDERISZE EN"  }

    ,{ PRV_CPSS_TG_FLD_PKT_CNT_E,            { FLD_OFF(packGenConfig_STC,packetCount),          0, 13 }, { 0,  0x1FFF },   "PKT CNT"       }
    ,{ PRV_CPSS_TG_FLD_PKT_CNT_MULTIPLIER_E, { FLD_OFF(packGenConfig_STC,packetCount),         13,  3 }, { 0,       7 },   "PKT CNT MLT"   }

    ,{ PRV_CPSS_TG_FLD_IFG_E,                { FLD_OFF(packGenConfig_STC,ifg),                  0, 16 }, { 0,  0xFFFF },   "IFG"           }
    ,{ PRV_CPSS_TG_FLD_INTERFACE_SIZE_E,     { FLD_OFF(packGenConfig_STC,controlReg0),         13,  3 }, { 0,       7 },   "IF WIDTH"      }

    ,{ PRV_CPSS_TG_FLD_PAYLOAD_TYPE_E,       { FLD_OFF(packGenConfig_STC,packetLength),        14,  2 }, { 0,       2 },   "PAYLOAD TYPE"  }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+ 0,   { FLD_OFF(packGenConfig_STC,dataPattern[ 0]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN  0" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+ 1,   { FLD_OFF(packGenConfig_STC,dataPattern[ 1]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN  1" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+ 2,   { FLD_OFF(packGenConfig_STC,dataPattern[ 2]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN  2" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+ 3,   { FLD_OFF(packGenConfig_STC,dataPattern[ 3]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN  3" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+ 4,   { FLD_OFF(packGenConfig_STC,dataPattern[ 4]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN  4" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+ 5,   { FLD_OFF(packGenConfig_STC,dataPattern[ 5]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN  5" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+ 6,   { FLD_OFF(packGenConfig_STC,dataPattern[ 6]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN  6" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+ 7,   { FLD_OFF(packGenConfig_STC,dataPattern[ 7]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN  7" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+ 8,   { FLD_OFF(packGenConfig_STC,dataPattern[ 8]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN  8" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+ 9,   { FLD_OFF(packGenConfig_STC,dataPattern[ 9]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN  9" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+10,   { FLD_OFF(packGenConfig_STC,dataPattern[10]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN 10" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+11,   { FLD_OFF(packGenConfig_STC,dataPattern[11]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN 11" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+12,   { FLD_OFF(packGenConfig_STC,dataPattern[12]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN 12" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+13,   { FLD_OFF(packGenConfig_STC,dataPattern[13]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN 13" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+14,   { FLD_OFF(packGenConfig_STC,dataPattern[14]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN 14" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+15,   { FLD_OFF(packGenConfig_STC,dataPattern[15]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN 15" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+16,   { FLD_OFF(packGenConfig_STC,dataPattern[16]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN 16" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+17,   { FLD_OFF(packGenConfig_STC,dataPattern[17]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN 17" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+18,   { FLD_OFF(packGenConfig_STC,dataPattern[18]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN 18" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+19,   { FLD_OFF(packGenConfig_STC,dataPattern[19]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN 19" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+20,   { FLD_OFF(packGenConfig_STC,dataPattern[20]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN 20" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+21,   { FLD_OFF(packGenConfig_STC,dataPattern[21]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN 21" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+22,   { FLD_OFF(packGenConfig_STC,dataPattern[22]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN 22" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+23,   { FLD_OFF(packGenConfig_STC,dataPattern[23]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN 23" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+24,   { FLD_OFF(packGenConfig_STC,dataPattern[24]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN 24" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+25,   { FLD_OFF(packGenConfig_STC,dataPattern[25]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN 25" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+26,   { FLD_OFF(packGenConfig_STC,dataPattern[26]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN 26" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+27,   { FLD_OFF(packGenConfig_STC,dataPattern[27]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN 27" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+28,   { FLD_OFF(packGenConfig_STC,dataPattern[28]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN 28" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+29,   { FLD_OFF(packGenConfig_STC,dataPattern[29]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN 29" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+30,   { FLD_OFF(packGenConfig_STC,dataPattern[30]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN 30" }
    ,{ PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+31,   { FLD_OFF(packGenConfig_STC,dataPattern[31]),      0, 16 }, { 0,  0xFFFF },   "DATA PATTERN 31" }

    ,{ PRV_BAD_VAL,                       {                        PRV_BAD_VAL,    PRV_BAD_VAL, PRV_BAD_VAL }, { PRV_BAD_VAL, PRV_BAD_VAL},   NULL  }
};

PRV_CPSS_DXCH_MAC_TG_DRV_STC      prv_dxChMacTGLion2;
PRV_CPSS_DXCH_MAC_TG_DRV_STC      prv_dxChMacTGBc2;
PRV_CPSS_DXCH_MAC_TG_DRV_STC      prv_dxChMacTGXCat3;


/**
* @internal prvCpssDxChDiagPacketGeneratorNumberGet function
* @endinternal
*
* @brief   Get number of packet generator for current device and macNum
*
* @note   APPLICABLE DEVICES:      Lion2; xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] macNum                   - number of MAC
*                                       Number of packet generator for current devNum and macNum
*
* @note For SIP_5_15 devices used 1 packet generator per 1 MACs. For other
*       device packGenNum = macNum / 4. See packet generator init function
*       gopMacTGInit(devNum) to get better understanding about packet
*       generator config initialization.
*
*/
static GT_U32 prvCpssDxChDiagPacketGeneratorNumberGet
(
    IN    GT_U8                  devNum,
    IN    GT_U32                 macNum
)
{
    return (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum) && (!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))) ? (macNum) : (macNum / 4);
}


/**
* @internal prvCpssDxChMacTGDrvInit function
* @endinternal
*
* @brief   Mac TG driver init
*
* @note   APPLICABLE DEVICES:      Lion2; xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Falcon.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note it uses prv_dxChMacTG as single-tone
*
*/
GT_STATUS prvCpssDxChMacTGDrvInit
(
    IN    GT_U8                   devNum
)
{
    GT_STATUS rc;
    PRV_CPSS_DRV_FLD_INIT_STC * intSeqPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E |
        CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_FALCON_E);

    PRV_CPSS_DXCH_SIP6_NOT_SUPPORT_SIP5_PACKET_GENERATOR_CHECK_MAC(devNum);

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) ||
       PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E ||
       PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
    {
        intSeqPtr = &prv_bobcat2FldInitStc[0];
        rc = prvCpssDrvInit(/*INOUT*/&prv_dxChMacTGBc2.genDrv
                                        ,&prv_dxChMacTGBc2.fldDefList[0]
                                        ,intSeqPtr
                                        ,PRV_CPSS_TG_FLD_MAX);
        if (rc != GT_OK)
        {
            return rc;
        }

        intSeqPtr = &prv_lion2FldInitStc[0];
        rc = prvCpssDrvInit(/*INOUT*/&prv_dxChMacTGLion2.genDrv
                                        ,&prv_dxChMacTGLion2.fldDefList[0]
                                        ,intSeqPtr
                                        ,PRV_CPSS_TG_FLD_MAX);
        if (rc != GT_OK)
        {
            return rc;
        }

        intSeqPtr = &prv_xcat3FldInitStc[0];
        rc = prvCpssDrvInit(/*INOUT*/&prv_dxChMacTGXCat3.genDrv
                                        ,&prv_dxChMacTGXCat3.fldDefList[0]
                                        ,intSeqPtr
                                        ,PRV_CPSS_TG_FLD_MAX);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

#define NUM_OF_ELEMENTS_MAC(arrayPtr) (sizeof(arrayPtr)/sizeof(arrayPtr[0]))
/**
* @internal prvCpssDxChMacTGDrvQeueryInit function
* @endinternal
*
* @brief   Mac-TG driver Init qeuery
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
*
* @retval GT_OK                    - on success
*
* @note it uses prv_dxChMacTG as single-tone
*
*/
GT_STATUS prvCpssDxChMacTGDrvQeueryInit
(
    INOUT PRV_CPSS_DRV_FLD_QEUERY_STC *queryPtr,
    IN    GT_U8 devNum,
    IN    GT_U32 portGroupId,
    IN    GT_U32 packGenNum
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_DRV_STC *drvPtr;
    GT_U32  numElements;

    GT_U32 * regStructBaseAdr;

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        regStructBaseAdr = (GT_U32*)&(PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->packGenConfig[packGenNum]);
        drvPtr = &prv_dxChMacTGBc2.genDrv;


        numElements = NUM_OF_ELEMENTS_MAC(PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->packGenConfig);
    }
    else  if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
    {
        regStructBaseAdr = (GT_U32*)&(PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->packGenConfig[packGenNum]);
        drvPtr = &prv_dxChMacTGLion2.genDrv;
        numElements = NUM_OF_ELEMENTS_MAC(PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->packGenConfig);
    }
    else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
    {
        regStructBaseAdr = (GT_U32*)&(PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->packGenConfig[packGenNum]);
        drvPtr = &prv_dxChMacTGXCat3.genDrv;
        numElements = NUM_OF_ELEMENTS_MAC(PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->packGenConfig);
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    if (packGenNum >= numElements)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    rc = prvCpssDrvQeueryInit(queryPtr, /*IN*/drvPtr,devNum,portGroupId,regStructBaseAdr);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChMacTGDrvQeueryFldSet function
* @endinternal
*
* @brief   Mac TG : Set field
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] fldId                    - field id
* @param[in] fldValue                 - field value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on field id is out of scope of total field
* @retval GT_NOT_SUPPORTED         - on field is not spuurted on specific implementation of driver
* @retval GT_OUT_OF_RANGE          - id is valid, field is suppprted, but value is put of range
*
* @note it uses prv_dxChMacTG as single-tone
*
*/
GT_STATUS prvCpssDxChMacTGDrvQeueryFldSet
(
    INOUT PRV_CPSS_DRV_FLD_QEUERY_STC *queryPtr,
    IN    PRV_CPSS_TG_DRV_FLD_ID_ENT   fldId,
    IN    GT_U32                       fldValue
)
{
    return prvCpssDrvQeueryFldSet(queryPtr,fldId,fldValue);
}

/**
* @internal prvCpssDxChMacTGDrvQeueryFldGet function
* @endinternal
*
* @brief   MAc TG : Get the field
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on field id is out of scope of total field
* @retval GT_NOT_SUPPORTED         - on field is not spuurted on specific implementation of driver
*
* @note it uses prv_dxChMacTG as single-tone
*
*/
GT_STATUS prvCpssDxChMacTGDrvQeueryFldGet
(
    INOUT PRV_CPSS_DRV_FLD_QEUERY_STC *queryPtr,
    IN    PRV_CPSS_TG_DRV_FLD_ID_ENT   fldId,
    OUT   GT_U32                      *fldValuePtr
)
{
    return prvCpssDrvQeueryFldGet(queryPtr,/*IN*/fldId,/*OUT*/fldValuePtr);
}

/**
* @internal prvCpssDxChMacTGDrvQeueryFldCheck function
* @endinternal
*
* @brief   Set field
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on field id is out of scope of total field
* @retval GT_NOT_SUPPORTED         - on field is not spuurted on specific implementation of driver
*
* @note it uses prv_dxChMacTG as single-tone
*
*/
GT_STATUS prvCpssDxChMacTGDrvQeueryFldCheck
(
    INOUT PRV_CPSS_DRV_FLD_QEUERY_STC *queryPtr,
    IN PRV_CPSS_TG_DRV_FLD_ID_ENT      fldId,
    IN GT_U32                          fldValue
)
{
    return prvCpssDrvQeueryFldCheck(queryPtr,fldId,fldValue);
}

/*-----------------------------------------------------------------------------------*/
/* API and logical layer of Mac-TG driver                                            */
/*-----------------------------------------------------------------------------------*/


/**
* @internal prvCpssDxChDiagPacketGeneratorGetMacNum function
* @endinternal
*
* @brief   Get the MAC number for given port
*
* @note   APPLICABLE DEVICES:      Lion2; xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number to connect
*                                      its MAC to packet generator.
*
* @param[out] macNumPtr                - (pointer to) MAC number
*
* @retval GT_OK                    - on success
* @retval GT_NOT_INITIALIZED       - port is not initialized
*/
static GT_STATUS prvCpssDxChDiagPacketGeneratorGetMacNum
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT GT_U32               *macNumPtr
)
{
    GT_STATUS               rc;             /* return status */
    MV_HWS_PORT_STANDARD    portMode;       /* port i/f mode and speed translated to BlackBox enum */
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    CPSS_PORT_INTERFACE_MODE_ENT ifMode;    /* current port i/f mode */
    CPSS_PORT_SPEED_ENT     speed;          /* current port's speed */
    GT_U32                  localPort;      /* local port - support multi-port-groups device */
    GT_U32                  portMacNum;
    GT_U32                  portGroup;      /* local core number */

    CPSS_NULL_PTR_CHECK_MAC(macNumPtr);

    /* get MAC num */
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    ifMode = PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum,portMacNum);
    speed = PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum,portMacNum);

    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,ifMode, speed, &portMode);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,
                                      "prvCpssCommonPortIfModeToHwsTranslate FAIL: port %d, ifMode %d, speed %d\n",
                                      portNum, ifMode, speed);
    }

    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portMacNum);

    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

    rc = hwsPortModeParamsGetToBuffer(devNum, portGroup, localPort, portMode, &curPortParams);
    if (GT_OK != rc)
    {
        return rc;
    }

    *macNumPtr = curPortParams.portMacNumber;
    return GT_OK;
}

/**
* @internal cpssDxChDiagPacketGeneratorConnectParamsCheck function
* @endinternal
*
* @brief   Check parameters of Mac-TG
*
* @note   APPLICABLE DEVICES:      Lion2; xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] configPtr                - parames
*
* @retval GT_OK                    - on success
* @retval GT_NOT_SUPPORTED         - param is not supported
* @retval GT_BAD_PARAM             - on bad params
* @retval GT_OUT_OF_RANGE          - on param is out of range
*/
static GT_STATUS cpssDxChDiagPacketGeneratorConnectParamsCheck
(
    IN GT_U8                                 devNum,
    IN CPSS_DXCH_DIAG_PG_CONFIGURATIONS_STC *configPtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E |
        CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_FALCON_E);
    PRV_CPSS_DXCH_SIP6_NOT_SUPPORT_SIP5_PACKET_GENERATOR_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(configPtr);

    if (configPtr->transmitMode != CPSS_DIAG_PG_TRANSMIT_CONTINUES_E && configPtr->transmitMode != CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (configPtr->packetLengthType != CPSS_DIAG_PG_PACKET_LENGTH_CONSTANT_E && configPtr->packetLengthType != CPSS_DIAG_PG_PACKET_LENGTH_RANDOM_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) ||
       PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
    {
        if (   configPtr->payloadType != CPSS_DIAG_PG_PACKET_PAYLOAD_CYCLIC_E
            && configPtr->payloadType != CPSS_DIAG_PG_PACKET_PAYLOAD_RANDOM_E
            && configPtr->payloadType != CPSS_DIAG_PG_PACKET_PAYLOAD_CONSTANT_E)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
    {
        if (   configPtr->payloadType != CPSS_DIAG_PG_PACKET_PAYLOAD_CYCLIC_E
            && configPtr->payloadType != CPSS_DIAG_PG_PACKET_PAYLOAD_RANDOM_E)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    /* check values */
    if (configPtr->vlanTagEnable == GT_TRUE)
    {
        if (configPtr->vid < 1 || configPtr->vid > 4095)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
        if (configPtr->vpt > 7)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
        if (configPtr->cfi > 1)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
    }
    if (configPtr->packetLengthType == CPSS_DIAG_PG_PACKET_LENGTH_CONSTANT_E)
    {
        if (configPtr->packetLength < 20 || configPtr->packetLength > 16383)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
    }
    if (configPtr->transmitMode == CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E)
    {
        if (configPtr->packetCount < 1 || configPtr->packetCount > 8191)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
        if (configPtr->packetCountMultiplier < CPSS_DIAG_PG_PACKET_COUNT_MULTIPLIER_1_E ||
            configPtr->packetCountMultiplier > CPSS_DIAG_PG_PACKET_COUNT_MULTIPLIER_16M_E)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    if (configPtr->ipg > 0xFFFF)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}


/**
* @internal prvCpssDxChDiagPacketGenConnectedStateGet function
* @endinternal
*
* @brief   get connect State (enable to port/disable)
*
* @note   APPLICABLE DEVICES:      Lion2; xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Falcon.
*
*
* @param[out] currEnablePtr            - enable/disable
* @param[out] currMacNumPtr            - curr mac (from GOP (0--3 on case of Lion-2)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on bad pointer
*/
static GT_STATUS prvCpssDxChDiagPacketGenConnectedStateGet
(
    INOUT PRV_CPSS_DRV_FLD_QEUERY_STC *queryPtr,
    OUT   GT_BOOL                     *currEnablePtr,
    OUT   GT_U32                      *currMacNumPtr
)
{
    GT_STATUS rc;
    GT_U32 val;

    CPSS_NULL_PTR_CHECK_MAC(queryPtr);
    CPSS_NULL_PTR_CHECK_MAC(currEnablePtr);
    CPSS_NULL_PTR_CHECK_MAC(currMacNumPtr);

    rc = prvCpssDxChMacTGDrvQeueryFldGet(queryPtr,PRV_CPSS_TG_FLD_CONNECTED_PORT_E, currMacNumPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChMacTGDrvQeueryFldGet(queryPtr,PRV_CPSS_TG_FLD_CONNECT_EN_E,  &val);
    if (rc != GT_OK)
    {
        return rc;
    }
    *currEnablePtr = (GT_BOOL)val;

    return GT_OK;
}


typedef enum PRV_CPSS_TG_ADDR_TYPE_ENM
{
    PRV_CPSS_TG_ADDR_TYPE_SRC_E = 0,
    PRV_CPSS_TG_ADDR_TYPE_DST_E
}PRV_CPSS_TG_ADDR_TYPE_ENM;

/**
* @internal prvCpssDxChDiagPacketGenEthAddrSet function
* @endinternal
*
* @brief   set Mac DA/Sa ethernet address
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] type                     - address type
* @param[in] ethAddrPtr               - etherent address
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on bad pointer
*/
static GT_STATUS prvCpssDxChDiagPacketGenEthAddrSet
(
    INOUT PRV_CPSS_DRV_FLD_QEUERY_STC *queryPtr,
    IN    PRV_CPSS_TG_ADDR_TYPE_ENM    type,
    IN    GT_ETHERADDR                *ethAddrPtr

)
{
    GT_U32      value;
    GT_STATUS   rc;          /* function return value */
    PRV_CPSS_TG_DRV_FLD_ID_ENT fldId0;
    PRV_CPSS_TG_DRV_FLD_ID_ENT fldId1;
    PRV_CPSS_TG_DRV_FLD_ID_ENT fldId2;

    CPSS_NULL_PTR_CHECK_MAC(ethAddrPtr);
    switch(type)
    {
        case PRV_CPSS_TG_ADDR_TYPE_SRC_E:
            fldId0 = PRV_CPSS_TG_FLD_MAC_SA_0_E;
            fldId1 = PRV_CPSS_TG_FLD_MAC_SA_1_E;
            fldId2 = PRV_CPSS_TG_FLD_MAC_SA_2_E;
        break;
        case PRV_CPSS_TG_ADDR_TYPE_DST_E:
            fldId0 = PRV_CPSS_TG_FLD_MAC_DA_0_E;
            fldId1 = PRV_CPSS_TG_FLD_MAC_DA_1_E;
            fldId2 = PRV_CPSS_TG_FLD_MAC_DA_2_E;
        break;
        default:
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    value = ethAddrPtr->arEther[5] | ethAddrPtr->arEther[4] << 8;
    rc = prvCpssDxChMacTGDrvQeueryFldSet(queryPtr,fldId0, value);
    if (rc != GT_OK)
    {
        return rc;
    }
    value = ethAddrPtr->arEther[3] | ethAddrPtr->arEther[2] << 8;
    rc = prvCpssDxChMacTGDrvQeueryFldSet(queryPtr,fldId1, value);
    if (rc != GT_OK)
    {
        return rc;
    }

    value = ethAddrPtr->arEther[1] | ethAddrPtr->arEther[0] << 8;
    rc = prvCpssDxChMacTGDrvQeueryFldSet(queryPtr,fldId2, value);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChDiagPacketGenEthAddrGet function
* @endinternal
*
* @brief   set Mac DA/Sa ethernet address
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] type                     - address type
*
* @param[out] ethAddrPtr               - etherent address
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on bad pointer
*/
static GT_STATUS prvCpssDxChDiagPacketGenEthAddrGet
(
    INOUT PRV_CPSS_DRV_FLD_QEUERY_STC *queryPtr,
    IN    PRV_CPSS_TG_ADDR_TYPE_ENM    type,
    OUT   GT_ETHERADDR                *ethAddrPtr
)
{
    GT_U32      value;
    GT_STATUS   rc;          /* function return value */
    PRV_CPSS_TG_DRV_FLD_ID_ENT fldId0;
    PRV_CPSS_TG_DRV_FLD_ID_ENT fldId1;
    PRV_CPSS_TG_DRV_FLD_ID_ENT fldId2;

    switch(type)
    {
        case PRV_CPSS_TG_ADDR_TYPE_SRC_E:
            fldId0 = PRV_CPSS_TG_FLD_MAC_SA_0_E;
            fldId1 = PRV_CPSS_TG_FLD_MAC_SA_1_E;
            fldId2 = PRV_CPSS_TG_FLD_MAC_SA_2_E;
        break;
        case PRV_CPSS_TG_ADDR_TYPE_DST_E:
            fldId0 = PRV_CPSS_TG_FLD_MAC_DA_0_E;
            fldId1 = PRV_CPSS_TG_FLD_MAC_DA_1_E;
            fldId2 = PRV_CPSS_TG_FLD_MAC_DA_2_E;
        break;
        default:
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    rc = prvCpssDxChMacTGDrvQeueryFldGet(queryPtr,fldId0, &value);
    if (rc != GT_OK)
    {
        return rc;
    }
    ethAddrPtr->arEther[5] = (GT_U8)(value & 0xff);
    ethAddrPtr->arEther[4] = (GT_U8)((value >> 8 ) & 0xff);

    /*  MAC DA 1 */
    rc = prvCpssDxChMacTGDrvQeueryFldGet(queryPtr,fldId1, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    ethAddrPtr->arEther[3] = (GT_U8)(value & 0xff);
    ethAddrPtr->arEther[2] = (GT_U8)((value >> 8 ) & 0xff);

    /*  MAC DA 2 */
    rc = prvCpssDxChMacTGDrvQeueryFldGet(queryPtr,fldId2, &value);
    if (rc != GT_OK)
    {
        return rc;
    }
    ethAddrPtr->arEther[1] = (GT_U8)(value & 0xff);
    ethAddrPtr->arEther[0] = (GT_U8)((value >> 8 ) & 0xff);
    return GT_OK;
}


/**
* @internal prvCpssDxChDiagPacketGenVlanTagSet function
* @endinternal
*
* @brief   set Vlan tag
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] vlanTagEnable            - whether vlan tag is used
* @param[in] vpt                      - priority
* @param[in] cfi                      =
* @param[in] vid                      - VID
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on bad pointer
*/
static GT_STATUS prvCpssDxChDiagPacketGenVlanTagSet
(
    INOUT PRV_CPSS_DRV_FLD_QEUERY_STC *queryPtr,
    IN GT_BOOL   vlanTagEnable,
    IN GT_U8     vpt,
    IN GT_U8     cfi,
    IN GT_U16    vid
)
{
    GT_STATUS rc;

    CPSS_NULL_PTR_CHECK_MAC(queryPtr);

    if (vlanTagEnable == GT_FALSE)
    {
        vpt = 0;
        cfi = 0;
        vid = 0;
    }

    rc = prvCpssDxChMacTGDrvQeueryFldSet(queryPtr,PRV_CPSS_TG_FLD_VLAN_TAG_VPT_E, vpt);
    if( rc != GT_OK )
    {
        return rc;
    }
    rc = prvCpssDxChMacTGDrvQeueryFldSet(queryPtr,PRV_CPSS_TG_FLD_VLAN_TAG_CFI_E, cfi);
    if( rc != GT_OK )
    {
        return rc;
    }
    rc = prvCpssDxChMacTGDrvQeueryFldSet(queryPtr,PRV_CPSS_TG_FLD_VLAN_TAG_VID_E, vid);
    if( rc != GT_OK )
    {
        return rc;
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChDiagPacketGenVlanTagGet function
* @endinternal
*
* @brief   get Vlan tag
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
*
* @param[out] vlanTagEnablePtr         - whether vlan tag is used
* @param[out] vptPtr                   - priority
* @param[out] cfiPtr                   = cfi
* @param[out] vidPtr                   - VID
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on bad pointer
*/
static GT_STATUS prvCpssDxChDiagPacketGenVlanTagGet
(
    INOUT PRV_CPSS_DRV_FLD_QEUERY_STC *queryPtr,
    OUT GT_BOOL                  *vlanTagEnablePtr,
    OUT GT_U8                    *vptPtr,
    OUT GT_U8                    *cfiPtr,
    OUT GT_U16                   *vidPtr
)
{
    GT_STATUS rc;
    OUT GT_U32  vptVal;
    OUT GT_U32  cfiVal;
    OUT GT_U32  vidVal;


    CPSS_NULL_PTR_CHECK_MAC(queryPtr);
    CPSS_NULL_PTR_CHECK_MAC(vlanTagEnablePtr);
    CPSS_NULL_PTR_CHECK_MAC(vptPtr);
    CPSS_NULL_PTR_CHECK_MAC(cfiPtr);
    CPSS_NULL_PTR_CHECK_MAC(vidPtr);


    rc = prvCpssDxChMacTGDrvQeueryFldGet(queryPtr,PRV_CPSS_TG_FLD_VLAN_TAG_VPT_E, &vptVal);
    if( rc != GT_OK )
    {
        return rc;
    }
    rc = prvCpssDxChMacTGDrvQeueryFldGet(queryPtr,PRV_CPSS_TG_FLD_VLAN_TAG_CFI_E, &cfiVal);
    if( rc != GT_OK )
    {
        return rc;
    }
    rc = prvCpssDxChMacTGDrvQeueryFldGet(queryPtr,PRV_CPSS_TG_FLD_VLAN_TAG_VID_E, &vidVal);
    if( rc != GT_OK )
    {
        return rc;
    }
    *vptPtr = (GT_U8 )  vptVal;
    *cfiPtr = (GT_U8 )  cfiVal;
    *vidPtr = (GT_U16)  vidVal;


    if ( 0 == vptVal && 0 == cfiVal && 0 == vidVal)
    {
        *vlanTagEnablePtr = GT_FALSE;
    }
    else
    {
        *vlanTagEnablePtr = GT_TRUE;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChDiagPacketGenInterfaceSizeSet function
* @endinternal
*
* @brief   onterface size set
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
*
* @param[out] interfaceSize            - interface size
* @param[out] portType                 - port type
* @param[out] speed                    = port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on bad pointer
*/
static GT_STATUS prvCpssDxChDiagPacketGenInterfaceSizeSet
(
    INOUT PRV_CPSS_DRV_FLD_QEUERY_STC *queryPtr,
    IN    CPSS_DIAG_PG_IF_SIZE_ENT interfaceSize,
    IN    PRV_CPSS_PORT_TYPE_ENT   portType,
    IN    CPSS_PORT_SPEED_ENT      speed
)
{
    GT_STATUS rc;
    GT_U32 value;

    CPSS_NULL_PTR_CHECK_MAC(queryPtr);
    value = interfaceSize;
    if (interfaceSize == CPSS_DIAG_PG_IF_SIZE_DEFAULT_E)
    {
        #if (PRV_CPSS_GEB_DRV_DEBUG == 1)
            GT_CHAR * macTypeStrArr[] =
            {
                "PORT_NOT_EXISTS_E   "
                ,"PORT_FE_E          "
                ,"PORT_GE_E          "
                ,"PORT_XG_E          "
                ,"PORT_XLG_E         "
                ,"PORT_HGL_E         "
                ,"PORT_CG_E          "
                ,"PORT_ILKN_E        "
                ,"PORT_NOT_APPLICABLE"
            };
        #endif
        switch (portType)
        {
            case PRV_CPSS_PORT_FE_E:
            case PRV_CPSS_PORT_GE_E:
                value = CPSS_DIAG_PG_IF_SIZE_1_BYTE_E;             /* 1 bytes */
            break;
            case PRV_CPSS_PORT_XG_E:
            case PRV_CPSS_PORT_HGL_E:
                value = CPSS_DIAG_PG_IF_SIZE_8_BYTES_E;             /* 8 bytes */
            break;
            case PRV_CPSS_PORT_XLG_E:
                if (speed == CPSS_PORT_SPEED_10000_E || speed == CPSS_PORT_SPEED_20000_E)
                {
                    value = CPSS_DIAG_PG_IF_SIZE_8_BYTES_E;            /* 8 bytes */
                }
                else
                {
                    value = CPSS_DIAG_PG_IF_SIZE_16_BYTES_E;             /* 16 bytes */
                }
            break;

            case PRV_CPSS_PORT_CG_E:
            case PRV_CPSS_PORT_ILKN_E:
                value = CPSS_DIAG_PG_IF_SIZE_64_BYTES_E;             /* 64 bytes */
            break;
            default:
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
            }
        }
        #if (PRV_CPSS_GEB_DRV_DEBUG == 1)
            cpssOsPrintf("\n port %2d MAC type = %s value=%d",portNum,macTypeStrArr[portType],value);
        #endif
    }
    rc = prvCpssDxChMacTGDrvQeueryFldSet(queryPtr,PRV_CPSS_TG_FLD_INTERFACE_SIZE_E, value);
    if( rc != GT_OK )
    {
        return rc;
    }
    return GT_OK;
}

/**
* @internal internal_cpssDxChDiagPacketGeneratorConnectSet function
* @endinternal
*
* @brief   Connect/Disconnect port to packet generator.
*         Set packet generator's configurations.
*
* @note   APPLICABLE DEVICES:      Lion2; xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number to connect
*                                      its MAC to packet generator.
* @param[in] connect                  - GT_TRUE:  the given port's MAC to packet generator.
*                                      GT_FALSE: disconnect the given port's MAC from
*                                      packet generator; return the port's MAC
*                                      to normal egress pipe.
* @param[in] configPtr                - (pointer to) packet generator configurations.
*                                      Relevant only if enable==GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - on port is not initialized (interface/speed)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported port
*
* @note 1. In case packet generator connected to other port the function perfroms
*       the following:
*       - stops traffic.
*       - connects packet generator to new port
*       - overrides packet generator's configurations
*       2. Before enabling the packet generator, port must be initialized,
*       i.e. port interface and speed must be set.
*
*/
static GT_STATUS internal_cpssDxChDiagPacketGeneratorConnectSet
(
    IN GT_U8                                 devNum,
    IN GT_PHYSICAL_PORT_NUM                  portNum,
    IN GT_BOOL                               connect,
    IN CPSS_DXCH_DIAG_PG_CONFIGURATIONS_STC *configPtr
)
{
    GT_U32      portGroupId; /*the port group Id - support multi-port-groups device */
    GT_U32      macNum;      /* number of MAC used by given port */
    GT_U32      value;
    GT_U32      i;           /* iterator */
    GT_STATUS   rc;          /* function return value */

    GT_U32   currMacNum;     /* current MAC number connected to packet generator*/
    GT_BOOL  currEnable;     /* is packet generator enabled */
    GT_U32   packGenNum;     /* packet generator number */
    PRV_CPSS_DRV_FLD_QEUERY_STC query;
    GT_U32   portMacNum;      /* MAC number */


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E |
        CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_FALCON_E);
    PRV_CPSS_DXCH_SIP6_NOT_SUPPORT_SIP5_PACKET_GENERATOR_CHECK_MAC(devNum);


    if (PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
                     PRV_CPSS_DXCH_BOBCAT2_PACKET_GENERATOR_SPECIFIC_PORTS_WA_E))
    {
        if(PRV_CPSS_GE_PORT_GE_ONLY_E == PRV_CPSS_DXCH_PORT_TYPE_OPTIONS_MAC(devNum, portMacNum))
        {
            if ((portMacNum % 4) == 0)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
    }

    /* get the port's MAC number */
    rc = prvCpssDxChDiagPacketGeneratorGetMacNum(devNum,portNum,&macNum);
    if( rc != GT_OK )
    {
        return rc;
    }

    packGenNum = prvCpssDxChDiagPacketGeneratorNumberGet(devNum, macNum);
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);

    rc = prvCpssDxChMacTGDrvQeueryInit(&query,devNum,portGroupId,packGenNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* get the current status */
    rc = prvCpssDxChDiagPacketGenConnectedStateGet(&query,/*OUT*/&currEnable,/*OU*/&currMacNum);
    if (rc != GT_OK)
    {
        return rc;
    }


    if (connect == GT_FALSE)
    {
        if ((currEnable == GT_TRUE) && (currMacNum == (macNum%4)))
        {
            /* disable PG */
            rc = prvCpssDxChMacTGDrvQeueryFldSet(&query,PRV_CPSS_TG_FLD_CONNECT_EN_E, 0);
            if (rc != GT_OK)
            {
                return rc;
            }
                        return GT_OK;
        }
        else
        {
            /* do nothing */
            return GT_OK;
        }

    }

    rc = cpssDxChDiagPacketGeneratorConnectParamsCheck(devNum,configPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* For Lion2- if packet length type is RANDOM then packet length should be set to 0x3FF.
           In this case packet size is variate between 64 and 1518 (not including CRC). */
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
    {
        if (configPtr->packetLengthType == CPSS_DIAG_PG_PACKET_LENGTH_RANDOM_E)
        {
            configPtr->packetLength = 0x3FF;
        }
    }



    /* other MAC is connected to packet generator */
    if ((currEnable == GT_TRUE) && (currMacNum != (macNum%4)))
    {
        /* stop traffic */
        rc = prvCpssDxChMacTGDrvQeueryFldSet(&query,PRV_CPSS_TG_FLD_STOP_TRIGGER_E, 1);
        if( rc != GT_OK )
        {
            return rc;
        }
    }


    /*  MAC DA MAC, DA increment enable & increment limit (BC2)*/
    rc = prvCpssDxChDiagPacketGenEthAddrSet(&query,PRV_CPSS_TG_ADDR_TYPE_DST_E,&configPtr->macDa);
    if( rc != GT_OK )
    {
        return rc;
    }


    rc = prvCpssDxChMacTGDrvQeueryFldSet(&query,PRV_CPSS_TG_FLD_MAC_DA_INC_EN_E,configPtr->macDaIncrementEnable);
    if( rc != GT_OK )
    {
        return rc;
    }

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) ||
       PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
    {
        if (configPtr->macDaIncrementEnable == GT_TRUE)
        {
            rc = prvCpssDxChMacTGDrvQeueryFldSet(&query,PRV_CPSS_TG_FLD_MAC_DA_INC_LIM_E,configPtr->macDaIncrementLimit);
            if( rc != GT_OK )
            {
                return rc;
            }
        }
    }


    /*  MAC SA 0 */
    rc = prvCpssDxChDiagPacketGenEthAddrSet(&query,PRV_CPSS_TG_ADDR_TYPE_SRC_E,&configPtr->macSa);
    if( rc != GT_OK )
    {
        return rc;
    }

    /* EtherType */
    rc = prvCpssDxChMacTGDrvQeueryFldSet(&query,PRV_CPSS_TG_FLD_ETHER_TYPE_E, configPtr->etherType);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Vlan Tag */
    rc = prvCpssDxChDiagPacketGenVlanTagSet(&query,configPtr->vlanTagEnable,configPtr->vpt,configPtr->cfi,configPtr->vid);
    if( rc != GT_OK )
    {
        return rc;
    }


    /* Packet length & packet length type * undersize enable */
    rc = prvCpssDxChMacTGDrvQeueryFldSet(&query,PRV_CPSS_TG_FLD_PKT_LENGTH_TYPE_E, configPtr->packetLengthType);
    if( rc != GT_OK )
    {
        return rc;
    }

    rc = prvCpssDxChMacTGDrvQeueryFldSet(&query,PRV_CPSS_TG_FLD_PKT_LENGTH_E, configPtr->packetLength);
    if( rc != GT_OK )
    {
        return rc;
    }

    rc = prvCpssDxChMacTGDrvQeueryFldSet(&query,PRV_CPSS_TG_FLD_PKT_UNDERSIZE_EN_E, configPtr->undersizeEnable);
    if( rc != GT_OK )
    {
        return rc;
    }

    rc = prvCpssDxChMacTGDrvQeueryFldSet(&query,PRV_CPSS_TG_FLD_TRANS_MODE_E, configPtr->transmitMode);
    if( rc != GT_OK )
    {
        return rc;
    }

    /* pcket count */
    if (configPtr->transmitMode == CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E)
    {
        rc = prvCpssDxChMacTGDrvQeueryFldSet(&query,PRV_CPSS_TG_FLD_PKT_CNT_E, configPtr->packetCount);
        if( rc != GT_OK )
        {
            return rc;
        }
        rc = prvCpssDxChMacTGDrvQeueryFldSet(&query,PRV_CPSS_TG_FLD_PKT_CNT_MULTIPLIER_E, configPtr->packetCountMultiplier);
        if( rc != GT_OK )
        {
            return rc;
        }
    }

    /* Inter frame gap */
    rc = prvCpssDxChMacTGDrvQeueryFldSet(&query,PRV_CPSS_TG_FLD_IFG_E, configPtr->ipg);
    if( rc != GT_OK )
    {
        return rc;
    }

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) ||
       PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
    {
        rc = prvCpssDxChDiagPacketGenInterfaceSizeSet(&query,configPtr->interfaceSize
                                       ,PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portNum)
                                       ,PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum,portNum));
        if( rc != GT_OK )
        {
            return rc;
        }
    }

    /* PAYLOAD */
    rc = prvCpssDxChMacTGDrvQeueryFldSet(&query,PRV_CPSS_TG_FLD_PAYLOAD_TYPE_E, configPtr->payloadType);
    if( rc != GT_OK )
    {
        return rc;
    }

    /* Data pattern */
    for (i=0; i < 4; i++)
    {
        value = configPtr->cyclicPatternArr[7 - i*2] | (configPtr->cyclicPatternArr[7 - i*2 - 1]) << 8;
        rc = prvCpssDxChMacTGDrvQeueryFldSet(&query,(PRV_CPSS_TG_DRV_FLD_ID_ENT)(PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+i), value);
        if( rc != GT_OK )
        {
            return rc;
        }
    }


    /* mac num */
    rc = prvCpssDxChMacTGDrvQeueryFldSet(&query,PRV_CPSS_TG_FLD_CONNECTED_PORT_E, macNum %4);
    if( rc != GT_OK )
    {
        return rc;
    }

    /* connect enable */
    rc = prvCpssDxChMacTGDrvQeueryFldSet(&query,PRV_CPSS_TG_FLD_CONNECT_EN_E, 1);
    if( rc != GT_OK )
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChDiagPacketGeneratorConnectSet function
* @endinternal
*
* @brief   Connect/Disconnect port to packet generator.
*         Set packet generator's configurations.
*
* @note   APPLICABLE DEVICES:      Lion2; xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number to connect
*                                      its MAC to packet generator.
* @param[in] connect                  - GT_TRUE:  the given port's MAC to packet generator.
*                                      GT_FALSE: disconnect the given port's MAC from
*                                      packet generator; return the port's MAC
*                                      to normal egress pipe.
* @param[in] configPtr                - (pointer to) packet generator configurations.
*                                      Relevant only if enable==GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - on port is not initialized (interface/speed)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported port
*
* @note 1. In case packet generator connected to other port the function perfroms
*       the following:
*       - stops traffic.
*       - connects packet generator to new port
*       - overrides packet generator's configurations
*       2. Before enabling the packet generator, port must be initialized,
*       i.e. port interface and speed must be set.
*
*/
GT_STATUS cpssDxChDiagPacketGeneratorConnectSet
(
    IN GT_U8                                 devNum,
    IN GT_PHYSICAL_PORT_NUM                  portNum,
    IN GT_BOOL                               connect,
    IN CPSS_DXCH_DIAG_PG_CONFIGURATIONS_STC *configPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagPacketGeneratorConnectSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, connect, configPtr));

    rc = internal_cpssDxChDiagPacketGeneratorConnectSet(devNum, portNum, connect, configPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, connect, configPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChDiagPacketGeneratorConnectGet function
* @endinternal
*
* @brief   Get the connect status of specified port.
*         Get packet generator's configurations.
*
* @note   APPLICABLE DEVICES:      Lion2; xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number to connect
*                                      its MAC to packet generator.
*
* @param[out] connectPtr               - (pointer to)
*                                      GT_TRUE: port connected to packet generator.
*                                      GT_FALSE: port not connected to packet generator.
* @param[out] configPtr                - (pointer to) packet generator configurations.
*                                      Relevant only if enable==GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - on port is not initialized (interface/speed)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChDiagPacketGeneratorConnectGet
(
    IN  GT_U8                                 devNum,
    IN  GT_PHYSICAL_PORT_NUM                  portNum,
    OUT GT_BOOL                              *connectPtr,
    OUT CPSS_DXCH_DIAG_PG_CONFIGURATIONS_STC *configPtr
)
{
    GT_U32      portGroupId; /*the port group Id - support multi-port-groups device */
    GT_U32      macNum;      /* number of MAC used by given port */
    GT_U32      value;
    GT_U32      i;           /* iterator */
    GT_STATUS   rc;          /* function return value */

    GT_U32   currMacNum;     /* current MAC number connected to packet generator*/
    GT_BOOL  currEnable;     /* is packet generator enabled */
    GT_U32   packGenNum;     /* packet generator number*/
    GT_U32   portMacNum;     /* MAC number */

    PRV_CPSS_DRV_FLD_QEUERY_STC query;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E |
        CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_FALCON_E);
    PRV_CPSS_DXCH_SIP6_NOT_SUPPORT_SIP5_PACKET_GENERATOR_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(connectPtr);
    CPSS_NULL_PTR_CHECK_MAC(configPtr);

    /* get the port's MAC number */
    rc = prvCpssDxChDiagPacketGeneratorGetMacNum(devNum,portNum,&macNum);
    if( rc != GT_OK )
        return rc;

    packGenNum = prvCpssDxChDiagPacketGeneratorNumberGet(devNum, macNum);
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);

    rc = prvCpssDxChMacTGDrvQeueryInit(&query,devNum,portGroupId,packGenNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* get the current status */
    rc = prvCpssDxChDiagPacketGenConnectedStateGet(&query,/*OUT*/&currEnable,/*OU*/&currMacNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    if ((currEnable == GT_FALSE) || (currMacNum != (macNum%4)))
    {
        *connectPtr = GT_FALSE;
    }
    else
    {
        *connectPtr = GT_TRUE;
    }


    /*  MAC DA */
    rc = prvCpssDxChDiagPacketGenEthAddrGet(&query,PRV_CPSS_TG_ADDR_TYPE_DST_E, /*OUT*/&configPtr->macDa);
    if( rc != GT_OK )
    {
        return rc;
    }
    /*  MAC DA increment enable & increment limit (BC2)*/
    rc = prvCpssDxChMacTGDrvQeueryFldGet(&query,PRV_CPSS_TG_FLD_MAC_DA_INC_EN_E,&value);
    if( rc != GT_OK )
    {
        return rc;
    }
    configPtr->macDaIncrementEnable = (GT_BOOL)value;

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) ||
       PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
    {
        rc = prvCpssDxChMacTGDrvQeueryFldGet(&query,PRV_CPSS_TG_FLD_MAC_DA_INC_LIM_E,&value);
        if( rc != GT_OK )
        {
            return rc;
        }
        configPtr->macDaIncrementLimit = value;
    }


    rc = prvCpssDxChDiagPacketGenEthAddrGet(&query,PRV_CPSS_TG_ADDR_TYPE_SRC_E, /*OUT*/&configPtr->macSa);
    if( rc != GT_OK )
    {
        return rc;
    }

    /* EtherType */
    rc = prvCpssDxChMacTGDrvQeueryFldGet(&query,PRV_CPSS_TG_FLD_ETHER_TYPE_E, &value);
    if (rc != GT_OK)
    {
        return rc;
    }
    configPtr->etherType = (GT_U16)value;

    /* Vlan Tag */
    rc = prvCpssDxChDiagPacketGenVlanTagGet(&query,&configPtr->vlanTagEnable,&configPtr->vpt,&configPtr->cfi,&configPtr->vid);
    if( rc != GT_OK )
    {
        return rc;
    }


    /* Packet length & packet length type * undersize enable */
    rc = prvCpssDxChMacTGDrvQeueryFldGet(&query,PRV_CPSS_TG_FLD_PKT_LENGTH_TYPE_E, &value);
    if( rc != GT_OK )
    {
        return rc;
    }
    configPtr->packetLengthType = (CPSS_DIAG_PG_PACKET_LENGTH_TYPE_ENT)value;

    rc = prvCpssDxChMacTGDrvQeueryFldGet(&query,PRV_CPSS_TG_FLD_PKT_LENGTH_E, &configPtr->packetLength);
    if( rc != GT_OK )
    {
        return rc;
    }

    rc = prvCpssDxChMacTGDrvQeueryFldSet(&query,PRV_CPSS_TG_FLD_PKT_UNDERSIZE_EN_E, value);
    if( rc != GT_OK )
    {
        return rc;
    }
    configPtr->undersizeEnable = (GT_BOOL)value;
    /* pcket count */

    rc = prvCpssDxChMacTGDrvQeueryFldGet(&query,PRV_CPSS_TG_FLD_TRANS_MODE_E,&value);
    if( rc != GT_OK )
    {
        return rc;
    }
    configPtr->transmitMode = (CPSS_DIAG_PG_TRANSMIT_MODE_ENT)value;

    if (configPtr->transmitMode == CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E)
    {
        rc = prvCpssDxChMacTGDrvQeueryFldGet(&query,PRV_CPSS_TG_FLD_PKT_CNT_E, &configPtr->packetCount);
        if( rc != GT_OK )
        {
            return rc;
        }
        rc = prvCpssDxChMacTGDrvQeueryFldGet(&query,PRV_CPSS_TG_FLD_PKT_CNT_MULTIPLIER_E, &value);
        if( rc != GT_OK )
        {
            return rc;
        }
        configPtr->packetCountMultiplier = (CPSS_DIAG_PG_PACKET_COUNT_MULTIPLIER_ENT)value;
    }

    /* Inter frame gap */
    rc = prvCpssDxChMacTGDrvQeueryFldGet(&query,PRV_CPSS_TG_FLD_IFG_E, &configPtr->ipg);
    if( rc != GT_OK )
    {
        return rc;
    }

    configPtr->interfaceSize = CPSS_DIAG_PG_IF_SIZE_NOT_APPLICABLE_E;
    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) ||
       PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
    {
        rc = prvCpssDxChMacTGDrvQeueryFldGet(&query,PRV_CPSS_TG_FLD_INTERFACE_SIZE_E, &value);
        if( rc != GT_OK )
        {
            return rc;
        }
        configPtr->interfaceSize = (CPSS_DIAG_PG_IF_SIZE_ENT)value;
    }


    /* PAYLOAD */
    rc = prvCpssDxChMacTGDrvQeueryFldGet(&query,PRV_CPSS_TG_FLD_PAYLOAD_TYPE_E,&value );
    if( rc != GT_OK )
    {
        return rc;
    }
    configPtr->payloadType = (CPSS_DIAG_PG_PACKET_PAYLOAD_TYPE_ENT)value;

    /* Data pattern */
    for (i=0; i < 4; i++)
    {
        rc = prvCpssDxChMacTGDrvQeueryFldGet(&query,PRV_CPSS_TG_FLD_PAYLOAD_REG_0_E+i, &value);
        if( rc != GT_OK )
        {
            return rc;
        }
        configPtr->cyclicPatternArr[7 - i*2    ] = value & 0xFF;
        configPtr->cyclicPatternArr[7 - i*2 - 1] = (value >> 8) & 0xFF;
    }

    return GT_OK;
}

/**
* @internal cpssDxChDiagPacketGeneratorConnectGet function
* @endinternal
*
* @brief   Get the connect status of specified port.
*         Get packet generator's configurations.
*
* @note   APPLICABLE DEVICES:      Lion2; xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number to connect
*                                      its MAC to packet generator.
*
* @param[out] connectPtr               - (pointer to)
*                                      GT_TRUE: port connected to packet generator.
*                                      GT_FALSE: port not connected to packet generator.
* @param[out] configPtr                - (pointer to) packet generator configurations.
*                                      Relevant only if enable==GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - on port is not initialized (interface/speed)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChDiagPacketGeneratorConnectGet
(
    IN  GT_U8                                 devNum,
    IN  GT_PHYSICAL_PORT_NUM                  portNum,
    OUT GT_BOOL                              *connectPtr,
    OUT CPSS_DXCH_DIAG_PG_CONFIGURATIONS_STC *configPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagPacketGeneratorConnectGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, connectPtr, configPtr));

    rc = internal_cpssDxChDiagPacketGeneratorConnectGet(devNum, portNum, connectPtr, configPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, connectPtr, configPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChDiagPacketGeneratorTransmitEnable function
* @endinternal
*
* @brief   Enable/Disable(Start/Stop) transmission on specified port.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] enable                   - GT_TRUE: enable(start) transmission
*                                      GT_FALSE: disable(stop) transmission
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_STATE             - on port not connected to packet generator
* @retval GT_NOT_INITIALIZED       - on port is not initialized (interface/speed)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported port
*
* @note 1. Before calling this function the port must be connected to packet
*       generator (cpssDxChDiagPacketGeneratorConnectSet),
*       otherwise GT_BAD_STATE is returned.
*
*/
static GT_STATUS internal_cpssDxChDiagPacketGeneratorTransmitEnable
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL              enable
)
{
    GT_U32      portGroupId; /*the port group Id - support multi-port-groups device */
    GT_U32      macNum;      /* number of MAC used by given port */
    GT_STATUS   rc;          /* function return value */
    GT_U32      currMacNum;     /* current MAC number connected to packet generator*/
    GT_U32      packGenNum;     /* packet generator number */
    GT_BOOL     currEnable;     /* is packet generator enabled */
    PRV_CPSS_DRV_FLD_QEUERY_STC query;
    GT_U32      portMacNum;     /* MAC number */


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E |
        CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_FALCON_E);
    PRV_CPSS_DXCH_SIP6_NOT_SUPPORT_SIP5_PACKET_GENERATOR_CHECK_MAC(devNum);

    if (PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
                     PRV_CPSS_DXCH_BOBCAT2_PACKET_GENERATOR_SPECIFIC_PORTS_WA_E))
    {
        if(PRV_CPSS_GE_PORT_GE_ONLY_E == PRV_CPSS_DXCH_PORT_TYPE_OPTIONS_MAC(devNum, portMacNum))
        {
            if ((portMacNum % 4) == 0)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
    }

    /* get the port's MAC number */
    rc = prvCpssDxChDiagPacketGeneratorGetMacNum(devNum,portNum,&macNum);
    if( rc != GT_OK )
    {
        return rc;
    }

    packGenNum = prvCpssDxChDiagPacketGeneratorNumberGet(devNum, macNum);
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);

    rc = prvCpssDxChMacTGDrvQeueryInit(&query,devNum,portGroupId,packGenNum);
    if( rc != GT_OK )
    {
        return rc;
    }

    /* get the current status */
    rc = prvCpssDxChDiagPacketGenConnectedStateGet(&query,/*OUT*/&currEnable,/*OU*/&currMacNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* other MAC is connected to packet generator */
    if (currMacNum != (macNum%4) || currEnable == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    if (enable == GT_TRUE)
    {
        /* stop traffic */
        rc = prvCpssDxChMacTGDrvQeueryFldSet(&query,PRV_CPSS_TG_FLD_START_TRIGGER_E, 1);
        if( rc != GT_OK )
        {
            return rc;
        }
    }
    else
    {
        rc = prvCpssDxChMacTGDrvQeueryFldSet(&query,PRV_CPSS_TG_FLD_STOP_TRIGGER_E, 1);
        if( rc != GT_OK )
        {
            return rc;
        }
    }
    return GT_OK;
}

/**
* @internal cpssDxChDiagPacketGeneratorTransmitEnable function
* @endinternal
*
* @brief   Enable/Disable(Start/Stop) transmission on specified port.
*
* @note   APPLICABLE DEVICES:      Lion2; xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] enable                   - GT_TRUE: enable(start) transmission
*                                      GT_FALSE: disable(stop) transmission
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_STATE             - on port not connected to packet generator
* @retval GT_NOT_INITIALIZED       - on port is not initialized (interface/speed)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported port
*
* @note 1. Before calling this function the port must be connected to packet
*       generator (cpssDxChDiagPacketGeneratorConnectSet),
*       otherwise GT_BAD_STATE is returned.
*
*/
GT_STATUS cpssDxChDiagPacketGeneratorTransmitEnable
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL              enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagPacketGeneratorTransmitEnable);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChDiagPacketGeneratorTransmitEnable(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChDiagPacketGeneratorBurstTransmitStatusGet function
* @endinternal
*
* @brief   Get burst transmission status on specified port.
*
* @note   APPLICABLE DEVICES:      Lion2; xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] burstTransmitDonePtr     - (pointer to) burst transmit done status
*                                      GT_TRUE: burst transmission done
*                                      GT_FALSE: burst transmission not done
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on port not connected to packet generator
* @retval GT_NOT_INITIALIZED       - on port is not initialized (interface/speed)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. Burst transmission status is clear on read.
*       2. Before calling this function the port must be connected to packet
*       generator (cpssDxChDiagPacketGeneratorConnectSet),
*       otherwise GT_BAD_STATE is returned.
*
*/
static GT_STATUS internal_cpssDxChDiagPacketGeneratorBurstTransmitStatusGet
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT GT_BOOL              *burstTransmitDonePtr
)
{
    GT_U32      portGroupId; /*the port group Id - support multi-port-groups device */
    GT_U32      macNum;      /* number of MAC used by given port */
    GT_U32      value;
    GT_STATUS   rc;          /* function return value */
    GT_U32      currMacNum;     /* current MAC number connected to packet generator*/
    GT_U32      packGenNum;     /* packet generator number */
    GT_BOOL     currEnable;     /* is packet generator enabled */
    PRV_CPSS_DRV_FLD_QEUERY_STC query;
    GT_U32      portMacNum;     /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E |
        CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_FALCON_E);
    PRV_CPSS_DXCH_SIP6_NOT_SUPPORT_SIP5_PACKET_GENERATOR_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(burstTransmitDonePtr);


    /* get the port's MAC number */
    rc = prvCpssDxChDiagPacketGeneratorGetMacNum(devNum,portNum,&macNum);
    if( rc != GT_OK )
    {
        return rc;
    }

    packGenNum = prvCpssDxChDiagPacketGeneratorNumberGet(devNum, macNum);
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);

    rc = prvCpssDxChMacTGDrvQeueryInit(&query,devNum,portGroupId,packGenNum);
    if( rc != GT_OK )
    {
        return rc;
    }

    /* get status of transmission */
    rc = prvCpssDxChMacTGDrvQeueryFldGet(&query,PRV_CPSS_TG_FLD_BIRST_TRANSM_DONE_E, &value);
    if( rc != GT_OK )
    {
        return rc;
    }

    /* get the current status */
    rc = prvCpssDxChDiagPacketGenConnectedStateGet(&query,/*OUT*/&currEnable,/*OU*/&currMacNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* other MAC is connected to packet generator */
    if (currMacNum != (macNum%4) || currEnable == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    *burstTransmitDonePtr = (GT_BOOL)value;

    return GT_OK;
}

/**
* @internal cpssDxChDiagPacketGeneratorBurstTransmitStatusGet function
* @endinternal
*
* @brief   Get burst transmission status on specified port.
*
* @note   APPLICABLE DEVICES:      Lion2; xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] burstTransmitDonePtr     - (pointer to) burst transmit done status
*                                      GT_TRUE: burst transmission done
*                                      GT_FALSE: burst transmission not done
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on port not connected to packet generator
* @retval GT_NOT_INITIALIZED       - on port is not initialized (interface/speed)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. Burst transmission status is clear on read.
*       2. Before calling this function the port must be connected to packet
*       generator (cpssDxChDiagPacketGeneratorConnectSet),
*       otherwise GT_BAD_STATE is returned.
*
*/
GT_STATUS cpssDxChDiagPacketGeneratorBurstTransmitStatusGet
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT GT_BOOL              *burstTransmitDonePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagPacketGeneratorBurstTransmitStatusGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, burstTransmitDonePtr));

    rc = internal_cpssDxChDiagPacketGeneratorBurstTransmitStatusGet(devNum, portNum, burstTransmitDonePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, burstTransmitDonePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
