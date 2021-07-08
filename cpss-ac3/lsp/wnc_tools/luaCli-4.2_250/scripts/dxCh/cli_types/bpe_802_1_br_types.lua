--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* bpe_802_1_br.lua
--*
--* DESCRIPTION:
--*       BPE IEEE 802.1 br types
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

-- support call from the config file : ${@@bpe_grp(3)}
function bpe_grp(grp)
    max_grp = bpe_802_1_br_max_grp_get()
    if grp > max_grp then
        grp = 1 + (grp % max_grp) -- grp is always 1 or more
    end
    
    return grp
end

CLI_type_dict["bpe_802_1_br___role"] =
{
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "The role (cb - Control Bridge , pe - Port Extender)",
    enum =
    {
        ["cb"] = { value="control_bridge", help="Control Bridge" },
        ["pe"] = { value="port_extender" , help="Port Extender"  }
    }
}

CLI_type_dict["bpe_802_1_br___connection_type"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "The role (CB - Control Bridge , PE - port extender)",
    enum = {
        ["direct"] = { value="direct", help="directly connected (to 'my' physical port)" },
        ["indirect"] = { value="indirect" , help="indirectly connected (to 'my' physical port via other directly connected device)"  }
    }
}

CLI_type_dict["bpe_802_1_br__ETag_GRP"] = {
    checker = CLI_check_param_number,
    min=1,
    max=3,
    complete = CLI_complete_param_number,
    help="the 2 bits <GRP> in the ETag format (values 1,2,3)"
}

CLI_type_dict["bpe_802_1_br__ETag_ecid"] = {
    checker = CLI_check_param_number,
    min=0,
    max=4095,
    complete = CLI_complete_param_number,
    help="the 12 bits <E-CID_base> in the ETag format (values 0..4K-1)"
}

-- function without parameter to check if the single device is eArch
function bpe_802_1_br_is_eArch_Device() 
    return is_device_eArch_enbled(devEnv.dev)
end

bpe_802_1_br_error_system_is_not_802_1br_string = "ERROR : the device was not configured with proper 'role' ('cb'/'pe') "
-- define index for the tpid of 'pushed tags'
bpe_802_1_br_egress_pushed_tag_tpid_reserved_index = 7
--a reserved VLAN-ID in the IEEE 802.1Q standard
bpe_802_1_br_not_valid_vid = 4095

-- get max GRP supported by the device
-- eArch function
function bpe_802_1_br_max_grp_get()
    if bpe_802_1_br_is_eArch_Device() then
        local value = math.floor(NumberOfEntriesIn_L2_MLL_LTT_table / 4096) 
        if value > 3 then 
            value = 3
        end
        
        return value
    else
        return 3
    end
end

-- get evidx number that should represent the {grp,ecid}
-- note : grp - should be 1,2,3 (not 0,and not more then 3)
function bpe_802_1_br_mc_eVidx_get(grp,ecid)
    if bpe_802_1_br_is_eArch_Device() then
        -- grp is never 0 .. only 1,2,3
        -- 12 LSbits ecid , 2 MSBits grp
        return  (grp * 0x1000) + ecid
    else
        -- grp is never 0 .. only 1,2,3
        return ((grp - 1) * 0x1000) + ecid
    end
end
 
-- check if system is 'PE in 802.1br' 
function bpe_802_1_br_is_system()
    if not bpe_802_1_br_my_info_DB then 
        return false 
    end
    
    if not bpe_802_1_br_my_info_DB["dev_info"] then 
        return false 
    end

    if (bpe_802_1_br_my_info_DB["dev_info"]["role"] ~= nil) then
        return true
    end
    
    return false
end

-- check if device is 'PE in 802.1br' and 'non-eArch'
function bpe_802_1_br_is_system_with_non_eArch_Device()
    if not bpe_802_1_br_is_system() then 
        return false 
    end
    
    return not bpe_802_1_br_is_eArch_Device()
end

-- check if this is control bridge
function bpe_802_1_br_is_control_bridge()
    return (bpe_802_1_br_my_info_DB["dev_info"]["role"] == "control_bridge")
end

-----------------------------
-- support for 6-bytes-tag --
-----------------------------
-- used bit 10 for lBit (meaning src-id limited to 10 bits)
bpe_802_1_br_6_bytes_tag_lBitInSrcId = 10
-- used bit 11 for lBit
bpe_802_1_br_6_bytes_tag_dBitInSrcId = 11

-- types of 'reserved VIDX for cascade port' -- the cascade ports are 'E-TAG' ports and also 'DSA-tag' ports.
-- consider number of VIDX available
function bpe_reservedVidxForCascadePortsGet()
    if(NumberOfEntriesIn_VIDX_table <= 0x1000) then
        return NumberOfEntriesIn_VIDX_table - 2 -- usually 0xffe (4094) , but when there are less vidx ...
    else
        -- the xCat3 may hold 12K (0x3000) entries
        return 0x1000 - 2 -- 0xffe (4094) , do not exceed the 0x1000 range.
    end
end



