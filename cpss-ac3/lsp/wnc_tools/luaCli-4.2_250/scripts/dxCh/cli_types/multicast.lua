--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* multicast.lua
--*
--* DESCRIPTION:
--*       multicast compound types definition
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************


-------------------------------------------------------
-- type registration: multicast
-------------------------------------------------------
CLI_type_dict["multicast"] = 
{
    checker  = CLI_check_param_compound,
    complete = CLI_complete_param_compound,
    compound = { "mac-address-mc", "ipv4_mc", "ipv6_mc" }                    
}  

    
-------------------------------------------------------
-- type registration: multicast_ip
-------------------------------------------------------
CLI_type_dict["multicast_ip"] = 
{
    checker  = CLI_check_param_compound,
    complete = CLI_complete_param_compound,
    compound = { "ipv4_mc", "ipv6_mc" }                                      
}

    
    