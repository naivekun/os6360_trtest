--*******************************************************************************
--*              (c), Copyright 2017, Marvell International Ltd.                *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.  *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE       *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.    *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,      *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.  *
--*******************************************************************************
--* qos.lua
--*
--* DESCRIPTION:
--*       The file defines types needed for the qos commands
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--*******************************************************************************


CLI_type_dict["qos_queue_id"] = {
    checker = CLI_check_param_number,
    complete = CLI_complete_param_number,
    min = 1,
    max = 8,
    help = "Specified queue number to which the tail-drop threshold is assigned"
}

CLI_type_dict["qos_threshold_percentage"] = {
    checker = CLI_check_param_number,
    complete = CLI_complete_param_number,
    min = 0,
    max = 100,
    help = "Specifies queue threshold percentage value"
}
