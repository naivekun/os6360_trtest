--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* descriptor_types.lua
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
-- Descriptor types,
--
-- Note: in case the ENUM CPSS_DXCH_DIAG_DESCRIPTOR_TYPE_ENT in the file: mainPpDrv\h\cpss\dxCh\dxChxGen\diag\cpssDxChDiagDescriptor.h
-- changes this file must be updated as well
--
CLI_type_dict["descType"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "descriptor types",
    enum = {
        ["WR_DMA_TO_TTI"]                         = { value=0, help="DMA to TTI descriptor" },
        ["TTI_TO_PCL"]                            = { value=1, help="TTI to PCL descriptor" },
        ["PCL_TO_BRIDGE"]                         = { value=2, help="PCL to bridge descriptor" },
        ["BRIDGE_TO_ROUTER"]                      = { value=3, help="Bridge to router descriptor" },
        ["ROUTER_TO_INGRESS_POLICER"]             = { value=4, help="Bridge to router descriptor" },
        ["INGRESS_POLICER0_TO_INGRESS_POLICER1"]  = { value=5, help="Ingress policer0 to ingress policer1 descriptor", appl={"LION", "LION2"}},
        ["INGRESS_POLICER1_TO_PRE_EGRESS"]        = { value=6, help="Ingress policer1 to pre egress descriptor" },
        ["PRE_EGRESS_TO_TXQ"]                     = { value=7, help="Pre egress to TXQ descriptor" },
        ["TXQ_TO_HEADER_ALTERATION"]              = { value=8, help="TXQ to header alternation descriptor", appl={"XCAT2", "LION", "LION2"}},
        ["EGRESS_POLICER_TO_EGRESS_PCL"]          = { value=9, help="Egress policer to egress PCL descriptor" }
    }
}

-- Extension to show descriptor all command

-- Especial enum "ALL" with value = -1 added to descType
local tempType = CLI_type_dict["descType"]["enum"]
tempType["ALL"]={value=-1, help="ALL descriptors"}

-- The descType with "ALL" enum
CLI_type_dict["descTypeAll"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "descriptor types",
    enum = tempType
}

