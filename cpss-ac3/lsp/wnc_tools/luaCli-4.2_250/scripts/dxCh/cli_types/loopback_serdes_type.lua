--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* loopback_serdes_type.lua
--*
--* DESCRIPTION:
--*       descriptor type definition
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants

--
-- Types defined:
--
-- serial deserial types,
--
-- Note: in case the ENUM CPSS_DXCH_DIAG_DESCRIPTOR_TYPE_ENT in the file: mainPpDrv\h\cpss\dxCh\dxChxGen\diag\cpssDxChDiagDescriptor.h
-- changes this file must be updated as well
--
CLI_type_dict["LoopbackSerdeType"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "descriptor types",
    enum = {
        ["rx2tx"] = { value="CPSS_DXCH_PORT_SERDES_LOOPBACK_DIGITAL_RX2TX_E", help="Loopback receive to transmit" },
        ["analogTx2rx"] = { value="CPSS_DXCH_PORT_SERDES_LOOPBACK_ANALOG_TX2RX_E", help="Analog loopback transmit to receive" },
        ["digitalTx2rx"] = { value="CPSS_DXCH_PORT_SERDES_LOOPBACK_DIGITAL_TX2RX_E", help="Digital loopback transmit to receive" }
    }
}












