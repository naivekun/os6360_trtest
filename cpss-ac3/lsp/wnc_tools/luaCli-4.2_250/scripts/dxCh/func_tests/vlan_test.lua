--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* vlan_test.lua
--*
--* DESCRIPTION:
--*
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--[[

---+++ CLI EXECUTION TRACE START +++---

configure

interface vlan device ${dev} vid 2
exit
interface vlan device ${dev} vid 4095

exit
exit

show vlan device ${dev} tag 2

show vlan device ${dev} tag 4095

configure

no interface vlan device ${dev} vid 2

no interface vlan device ${dev} vid 4095

exit




---+++ CLI EXECUTION TRACE END +++---

]]--



