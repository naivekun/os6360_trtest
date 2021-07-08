--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_vlan.lua
--*
--* DESCRIPTION:
--*       showing of bpe_802_1_br information
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants

-- internal functions

local function show_bpe_802_1_br_func_print_role()
    local db_vertex0, db_vertex1;

    db_vertex0 = bpe_802_1_br_my_info_DB;
    if db_vertex0 ~= nil then
        db_vertex0 = db_vertex0["dev_info"];
    end;

    if db_vertex0 ~= nil then
        db_vertex1 = db_vertex0["role"];
        if db_vertex1 ~= nil then
             print("bpe_802_1_br role is " .. db_vertex1);
        end
        
        if db_vertex0.base_ecid then
            print("bpe_802_1_br base_ecid is " .. tostring(db_vertex0.base_ecid))
        end
        if db_vertex0.base_grp then
            print("bpe_802_1_br base_grp is " .. tostring(db_vertex0.base_grp))
        end
       
    end;
end

local function show_bpe_802_1_br_func_print_vlans()
    -- Common variables declaration
    local db_vertex0, db_vertex1, db_vertex2, key;
    local grp, cid_base;

    db_vertex0 = bpe_802_1_br_my_info_DB;
    if db_vertex0 ~= nil then
        db_vertex0 = db_vertex0["dev_info"];
    end;

    if db_vertex0 ~= nil then
        db_vertex1 = db_vertex0["vlan_info"];
    end;
    if db_vertex1 ~= nil then
        print("Vlan Info.");
        print("VlanId\t group\t pcid  ");
        print("------\t -----\t ------");
        for key, db_vertex2 in pairs(db_vertex1) do
            grp      = 0;
            cid_base = 0;
            if db_vertex2 ~= nil then
                if db_vertex2["grp"] ~= nil then
                    grp = db_vertex2["grp"];
                end
                if db_vertex2["e-cid-base"] ~= nil then
                    cid_base = db_vertex2["e-cid-base"];
                end
                print(string.format(
                    "0x%4.4X\t %5d\t 0x%4.4X",
                    key, grp, cid_base));
            end;
        end;
    end;
end

--'extern' function show_bpe_802_1_br_func_print_mc_groups()


-- ************************************************************************
---
--  show_bpe_802_1_br_func
--        @description  shows bpe_802_1_br information
--
--        @param params         - not used;
--
--        @return       true on success, otherwise false and error message
--
local function show_bpe_802_1_br_func(params)
        show_bpe_802_1_br_func_print_role();
        bpe_802_1_br_neighbor_port_neighbor_info_show();
        show_bpe_802_1_br_func_print_vlans();
        show_bpe_802_1_br_func_print_mc_groups();
        if bpe_802_1_br_is_eArch_Device() then
            bpe_802_1_br_pcid_on_pe_eport_info_print_from_db();
        else
            bpe_802_1_br_non_eArch_on_pe_remote_pe_print_from_db();
        end
        bpe_802_1_pcid_on_me_port_info_show();
end

--------------------------------------------------------------------------------
-- command registration: show bpe-802-1-br
--------------------------------------------------------------------------------
CLI_addCommand("exec", "show bpe-802-1-br info", {
    func   = show_bpe_802_1_br_func,
    help   = "bpe-802-1-br info",
    params = {}
})

