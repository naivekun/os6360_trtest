-- ************************************************************************
---
--  Console(config-vlan)# bpe-802-1-br flood-etag-info grp <grp> e-cid-base <ecid>
--
--  bpe-802-1-br         bind the vlan to the ETag info of BPE 802.1BR
--
--  flood-etag-info      the 'flood in vlan' associated <GRP> and <E-CID_base>
--
--  grp                  the 2 bits <GRP> in the ETag format (values 1,2,3)
--
--  e-cid-base           the 12 bits <E-CID_base> in the ETag format (values 0..4K-1)

-- global DB - initialized as empty table on registration
-- bpe_802_1_br_my_info_DB = {};

-- temporary here, must be in "lua/lua_cli/scripts/cli_types"

--[[
-- base eport to represent eports of 'pe on me' 
'extern' onPe_getEportOfPcidOnNonCascadePort(...)
--]]


local evidx_for_vlan_prefix = "BPE : eVidx For flooding in VLAN "
local evidx_for_vidx_prefix = "BPE : eVidx For flooding in vidx "


local function vlan_port_member_key_list_get(devNum, vlanId)
    local ret, val, portsMembers, isValid;
    local wordIndex, bitIndex, wordArr, word, port;
    local result_port_list;

    ret, val =
        myGenWrapper("cpssDxChBrgVlanEntryRead",{
            {"IN","GT_U8","devNum",devNum},
            {"IN","GT_U16","vlanId",vlanId},
            {"OUT","CPSS_PORTS_BMP_STC","portsMembers"},
            {"OUT","CPSS_PORTS_BMP_STC","portsTagging"},
            {"OUT","CPSS_DXCH_BRG_VLAN_INFO_STC","vlanInfo"},
            {"OUT","GT_BOOL","isValid"},
            {"OUT","CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC","portsTaggingCmd"}
    });

    if (ret ~= 0) then
        printf("cpssDxChBrgVlanEntryRead failed, rc = " .. ret);
    end

    portsMembers = val["portsMembers"];
    isValid      = val["isValid"];

    wordArr = portsMembers["ports"];
    result_port_list = {};
    -- port bitmap array inexed not as regular LUA table from 1,
    -- but from 0 as C-language array
    for wordIndex = 0, (#wordArr - 1) do
        word = wordArr[wordIndex];
        for bitIndex = 0, 31 do
            if ((word % 2) == 1) then
                port = ((wordIndex * 32) + bitIndex);
                result_port_list[port] = {};
            end
            word = ((word - (word % 2)) / 2);
        end
    end
    return result_port_list;
end

local function  vlan_port_not_cascade_member_key_list_get(devNum, vlanId)
    local port_list, cascade_list, port, dummy, devPortKey;

    port_list    = vlan_port_member_key_list_get(devNum, vlanId);
    cascade_list = bpe_802_1_br_get_cascade_ports_list("all");

    port = next(port_list, nil);
    while (port ~= nil) do
        devPortKey = ((0x1000 * devNum) + port);
        if (cascade_list[devPortKey] ~= nil) then
            port_list[port] = nil; -- removing the port
        end
        port = next(port_list, port);
    end
    return port_list;
end

-- conver physical ports of local 'extended ports' to their 'internal' eports
local function convert_list_physical_ports_to_eports(physical_port_list,vlanId)
    local eport_list = {}
    local port = next(physical_port_list, nil);
    local eport
    while (port ~= nil) do
        -- resources limitations : ports over 128 not needed.
        -- for some reason the vlan 1 in bc2 hold ports 0..82 range and also 128..255
        if port < 128 then
            -- get eport that represent the physical port for L2MLL needed
            eport = onPe_getEportOfPcidOnNonCascadePort(port)
            
            if(eport ~= nil) then
                --set index in 'eport' offset
                eport_list[eport] = port
            else
                -- NOTE: the vlan(s) hold too many members ! the DB of BPE limited to about 74 ports to be used !!!
                print ("ERROR : portNum " .. port ..  "(from vlan[" .. vlanId .."]) can't get eport that represent the physical port for L2MLL ")
                break
            end
        end

        port = next(physical_port_list, port);
    end

    return eport_list
end

-- set the L2 MLL LTT entry
local function bpe_set_l2mll_ltt_entry(command_data,devNum,l2MllIndex,lttEntry)
    --
    local numEntries = NumberOfEntriesIn_L2_MLL_LTT_table
    if(l2MllIndex >= numEntries) then
        local errorStr = "use limited L2 LTT MLL range for full range <GRP><PCI> but limited to max " .. tostring(numEntries) .. "entries"
        command_data:setFailEntryAndLocalStatus()
        command_data:addError(errorStr)
        
        return true -- error
    end
    --
    
    local ret, val = myGenWrapper("cpssDxChL2MllLttEntrySet",{
            {"IN","GT_U8","devNum",devNum},
            {"IN","GT_U32","index", l2MllIndex},
            {"IN","CPSS_DXCH_L2_MLL_LTT_ENTRY_STC","lttEntryPtr", lttEntry}
    });
    if (ret ~= 0) then
        print("cpssDxChL2MllLttEntrySet returned " .. ret .. " for devNum = " .. devNum);
        return true -- error
    end
    
    return false
end

-- when vlanId == nil --> function assume that eVidx created for 'downstreamVidx' and only single mll member needed
function evidx_flood_via_l2mll_chain_config(devNum, vlanId, eVidx, downstreamVidx )
    local port_list, port_array, numOfPorts, numOfMllEntries;
    local mllPairEntry, mllPairIndex, mllPairMember, mllWriteForm;
    local mllEntriesList, index, j, lttEntry, mllEntrySelector;
    local mllEntryInListIdx, mllEntiesChainName;
    local hwDevNum, dummyPortNum;
    local command_data = Command_Data()
    local ret, val
    
    dummyPortNum = 0;
    hwDevNum, dummyPortNum =
        Command_Data:getHWDevicePort(devNum, dummyPortNum);

    if vlanId ~= nil then
        -- emulate vlan members by link list of eport + vidx of cascade ports
        port_list  = vlan_port_not_cascade_member_key_list_get(devNum, vlanId);

        -- conver physical ports of local 'extended ports' to their 'internal' eports
        local eport_list = convert_list_physical_ports_to_eports(port_list,vlanId);

        port_array = convert_key_list_to_array(eport_list);

        numOfPorts = #port_array;
        mllEntiesChainName = evidx_for_vlan_prefix .. tostring(vlanId);
    else
        -- temp implementation that used vidx as single member
        numOfPorts = 0 -- no ports needed as they are members of the 'physical vidx'
        
        mllEntiesChainName = evidx_for_vidx_prefix .. tostring(downstreamVidx)
    end
        

    numOfMllEntries = (numOfPorts + 1);
    mllEntriesList = allocL2MllTableEntriesChain(
        devNum, numOfMllEntries, mllEntiesChainName);
    if (mllEntriesList == nil) then
        print("allocL2MllTableEntriesChain failed");
        return;
    end

    mllEntryInListIdx = 1;
    while (mllEntryInListIdx <= numOfMllEntries) do
        index = mllEntriesList[mllEntryInListIdx];
        mllPairIndex = ((index - (index % 2)) / 2);

        mllPairEntry = {};
        mllPairEntry["firstMllNode"] = {};
        mllPairEntry["secondMllNode"] = {};

        if (mllEntryInListIdx == numOfMllEntries) then
            -- last single
            if (index % 2) == 0 then
                mllPairMember = "firstMllNode";
                mllWriteForm  = 0; --CPSS_DXCH_PAIR_READ_WRITE_FIRST_POINTER_ONLY_E
            else
                mllPairMember = "secondMllNode";
                mllWriteForm  = 1; --CPSS_DXCH_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E
            end;
            mllPairEntry[mllPairMember]["last"] = true;
            mllPairEntry[mllPairMember]["egressInterface"] = {};
            if (mllEntryInListIdx == 1) then
                mllPairEntry[mllPairMember]["egressInterface"]["type"] =
                        "CPSS_INTERFACE_VIDX_E";
                mllPairEntry[mllPairMember]["egressInterface"]["vidx"] = downstreamVidx;
            else
                mllPairEntry[mllPairMember]["egressInterface"]["type"] =
                        "CPSS_INTERFACE_PORT_E";
                mllPairEntry[mllPairMember]["egressInterface"]["devPort"] = {};
                mllPairEntry[mllPairMember]["egressInterface"]
                    ["devPort"]["devNum"] = hwDevNum;
                mllPairEntry[mllPairMember]["egressInterface"]
                    ["devPort"]["portNum"]  = port_array[mllEntryInListIdx - 1];
            end
        else
            -- pair
            if ((index + 1) ~= mllEntriesList[mllEntryInListIdx + 1]) then
                print("Error in mllEntriesList - not consequtive pair ");
            end
            if ((index % 2) ~= 0) then
                print("Error in mllEntriesList - pair beginning odd");
            end

            mllWriteForm  = 2; --CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E

            mllPairEntry["firstMllNode"]["last"] = false;
            mllPairEntry["firstMllNode"]["egressInterface"] = {};
            if (mllEntryInListIdx == 1) then
                mllPairEntry["firstMllNode"]["egressInterface"]["type"] =
                        "CPSS_INTERFACE_VIDX_E";
                mllPairEntry["firstMllNode"]["egressInterface"]["vidx"] = downstreamVidx;
            else
                mllPairEntry["firstMllNode"]["egressInterface"]["type"] =
                        "CPSS_INTERFACE_PORT_E";
                mllPairEntry["firstMllNode"]["egressInterface"]["devPort"] = {};
                mllPairEntry["firstMllNode"]["egressInterface"]
                    ["devPort"]["devNum"] = hwDevNum;
                mllPairEntry["firstMllNode"]["egressInterface"]
                    ["devPort"]["portNum"]  = port_array[mllEntryInListIdx - 1];
            end

            mllPairEntry["secondMllNode"]["last"] = true;
            mllPairEntry["secondMllNode"]["egressInterface"] = {};
            mllPairEntry["secondMllNode"]["egressInterface"]["type"] =
                    "CPSS_INTERFACE_PORT_E";
            mllPairEntry["secondMllNode"]["egressInterface"]["devPort"] = {};
            mllPairEntry["secondMllNode"]["egressInterface"]
                ["devPort"]["devNum"] = hwDevNum;
            mllPairEntry["secondMllNode"]["egressInterface"]
                ["devPort"]["portNum"]  = port_array[mllEntryInListIdx];


            if ((mllEntryInListIdx + 2) <= numOfMllEntries) then
                -- pointer to next
                mllPairEntry["secondMllNode"]["last"] = false;
                j = mllEntriesList[mllEntryInListIdx + 2];
                mllPairEntry["nextPointer"] = (j - (j % 2)) / 2;
                if ((j % 2) == 0) then
                    --CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E
                    mllPairEntry["entrySelector"] = 0;
                else
                    --CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_SECOND_MLL_E
                    mllPairEntry["entrySelector"] = 1;
                end
            end
        end

        ret, val = myGenWrapper("cpssDxChL2MllPairWrite",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_U32","mllPairEntryIndex", mllPairIndex},
                {"IN",TYPE["ENUM"],"mllPairWriteForm", mllWriteForm},
                {"IN","CPSS_DXCH_L2_MLL_PAIR_STC","mllPairEntryPtr", mllPairEntry}
        });
        if (ret ~= 0) then
            print("cpssDxChL2MllPairWrite returned " .. ret .. " for devNum = " .. devNum);
        end

        -- increment idex in L2Mll entries list
        if (mllWriteForm  == 2) then --CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E
            mllEntryInListIdx = mllEntryInListIdx + 2;
        else
            mllEntryInListIdx = mllEntryInListIdx + 1;
        end
    end

    -- Mll Ltt table

    index = mllEntriesList[1];
    mllPairIndex = ((index - (index % 2)) / 2);
    if (index % 2) == 0 then
        mllEntrySelector  = 0; --CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E
    else
        mllEntrySelector  = 1; --CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_SECOND_MLL_E
    end;
    lttEntry = {};
    lttEntry["mllPointer"] = mllPairIndex;
    lttEntry["entrySelector"] = mllEntrySelector;

    local lttEntryIndex = (eVidx - 4096)
    local isError = bpe_set_l2mll_ltt_entry(command_data,devNum,lttEntryIndex,lttEntry)
    if isError then
        return
    end
    

    return {["mllEntriesChainHead"] = mllEntriesList[1]};
end

-- remove L2MLL and L2LTT for vlan/vidx
function evidx_flood_via_l2mll_chain_remove(devNum, vlanId,vidx)
    local mllEntiesChainName;

    if vlanId ~= nil then
        mllEntiesChainName = evidx_for_vlan_prefix .. tostring(vlanId);
    else
        mllEntiesChainName = evidx_for_vidx_prefix .. tostring(vidx)
    end

    freeL2MllTableEntriesChainByString(devNum, mllEntiesChainName);
end


local function evidx_flood_tti_rule_cfg(devNum, vlanId, grp, ecid_base, flood_vidx, add_rule)
    local command_data = Command_Data()
    local unset = not (add_rule == true)
    local db_key_str = "vlan " .. vlanId

    func_onPe_bpe_802_1_br_mc_etag_info_bind_grp_ecid__tti_set(
        command_data,db_key_str,devNum,vlanId,grp,ecid_base,flood_vidx,
        false--[[not Mc]],
        unset)
end

function bpe_802_1_br_port_extender_flood_etag_cfg(devNum, vlanId, isNoDbError)
    local db_vertex, db_str, flood_vidx, grp, ecid_base, role;

    -- get grp, ecid_base from DB
    db_vertex = bpe_802_1_br_my_info_DB;
    if db_vertex == nil then
        if (isNoDbError == true) then
            print("extender_flood_etag_cfg - NO INFO IN DB");
        end
        return;
    end;
    db_vertex = db_vertex["dev_info"];
    if db_vertex == nil then
        if (isNoDbError == true) then
            print("extender_flood_etag_cfg - NO INFO IN DB");
        end
        return;
    end;

    -- configuration needed only for Port Extender
    role = db_vertex["role"];
    if (role ~= "port_extender") then
        return;
    end

    db_vertex = db_vertex["vlan_info"];
    if db_vertex == nil then
        if (isNoDbError == true) then
            print("extender_flood_etag_cfg - NO INFO IN DB");
        end
        return;
    end;
    db_str = tostring(vlanId);
    db_vertex = db_vertex[db_str];
    if db_vertex == nil then
        if (isNoDbError == true) then
            print("extender_flood_etag_cfg - NO INFO IN DB");
        end
        return;
    end;
    grp       = db_vertex["grp"];
    ecid_base = db_vertex["e-cid-base"];
    if ((grp == nil) or (ecid_base == nil)) then
        if (isNoDbError == true) then
            print("extender_flood_etag_cfg - NO INFO IN DB");
        end
        return;
    end;

    if not bpe_802_1_br_is_eArch_Device() then
        local command_data = Command_Data()
        func_onPe_non_eArch_bpe_802_1_br_mc_etag_info_bind_grp_ecid(command_data,vlanId,nil,devNum,grp,ecid_base)
        return
    end
    
    -- ((grp << 12)) + ecid_base)
    flood_vidx = bpe_802_1_br_mc_eVidx_get(grp,ecid_base);

    evidx_flood_via_l2mll_chain_config(
        devNum, vlanId, flood_vidx,
        bpe_reservedVidxForCascadePortsGet());

    evidx_flood_tti_rule_cfg(
        devNum, vlanId, grp, ecid_base, flood_vidx,
        true --add_rule
        );

end

local function bpe_802_1_br_port_extender_flood_etag_remove(devNum, vlanId)
    local db_vertex, db_str, flood_vidx, grp, ecid_base, role;
    local apiName
    
    -- get grp, ecid_base from DB
    db_vertex = bpe_802_1_br_my_info_DB;
    if db_vertex == nil then
        return;
    end;
    db_vertex = db_vertex["dev_info"];
    if db_vertex == nil then
        return;
    end;

    role = db_vertex["role"];

    db_vertex = db_vertex["vlan_info"];
    if db_vertex == nil then
        return;
    end;
    db_str = tostring(vlanId);
    db_vertex = db_vertex[db_str];
    if db_vertex == nil then
        return;
    end;
    grp       = db_vertex["grp"];
    ecid_base = db_vertex["e-cid-base"];
    if ((grp == nil) or (ecid_base == nil)) then
        db_vertex["grp"]          = nil;
        db_vertex["e-cid-base"]   = nil;
        return;
    end;

    if (role == "control_bridge") then
        local result, values;

        result, values  = myGenWrapper(
            "cpssDxChBrgVlanFloodVidxModeSet",{
            {"IN","GT_U8","devNum",devNum},
            {"IN","GT_U16","vlanId",vlanId},
            {"IN","GT_U16","floodVidx",0xFFF},
            {"IN",TYPE["ENUM"],"floodVidxMode",
                1 --[["CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_ALL_FLOODED_TRAFFIC_E"--]]},
            });
        if (result ~= 0) then
            print("cpssDxChBrgVlanFloodVidxModeSet error");
        end
    end

    if (role == "port_extender") then
        if not bpe_802_1_br_is_eArch_Device() then
            local command_data = Command_Data()
            func_onPe_non_eArch_bpe_802_1_br_mc_etag_info_bind_grp_ecid(command_data,vlanId,nil,devNum,nil,nil,true)
        else
            --[[ ((grp << 12)) + ecid_base) --]]
            flood_vidx = bpe_802_1_br_mc_eVidx_get(grp,ecid_base);

            evidx_flood_via_l2mll_chain_remove(
                devNum, vlanId);

            evidx_flood_tti_rule_cfg(
                devNum, vlanId, grp, ecid_base, flood_vidx,
                false --[[add_rule--]]);
        end
    end

    db_vertex["grp"]          = nil;
    db_vertex["e-cid-base"]   = nil;

    --remove Vlan vertex in DB if empty
    db_vertex = bpe_802_1_br_my_info_DB["dev_info"]["vlan_info"];
    db_str = tostring(vlanId);
    if (next(db_vertex[db_str], nil) == nil) then
        db_vertex[db_str] = nil;
    end
end

local function bpe_802_1_br__flood_etag_info(params)
    local db_vertex, db_str;
    local role = nil;
    local apiName
    
    db_vertex = bpe_802_1_br_my_info_DB;
    if (db_vertex ~= nil) then
         db_vertex = db_vertex["dev_info"];
    end
    if (db_vertex ~= nil) then
         role = db_vertex["role"];
    end
    if (role == nil) then
            print("The BPE 802.1BR role of board not defined\n");
            return;
    end
    
    local command_data;
    local grp, ecid_base, flood_vidx;
    local iterator, devNum, vlanId, lttEntry , lttEntryIndex;
    local result, values;
    
    grp       = tonumber(params["grp"]);
    ecid_base = tonumber(params["e-cid-base"]);

    if ((grp < 1) or (grp > 3)) then
        print("Wrong grp: should be in range 1..3 \n");
        return;
    end
    if (ecid_base > 0xFFF) then
        print("Wrong e-cid-base: should be in range 0..4095 \n");
        return;
    end

    -- Common variables initialization
    command_data = Command_Data();

    local max_grp = bpe_802_1_br_max_grp_get() 
    if grp > max_grp then
        command_data:setFailStatus()
        command_data:addError("ERROR : the device not support <GRP> = " .. tostring(grp) .. "only 1.. " .. tostring(max_grp))
        return
    end
    
    if (role == "port_extender") then
        ---[[ BPE 802.1BR Port Extender support --]]

        -- Check vlan context
        command_data:initInterfaceDevVlanRange();
        if (true ~= command_data["status"]) then
            print("Wrong context information status \n");
            return;
        end

        for iterator, devNum, vlanId in
                            command_data:getInterfaceVlanIterator() do
            --[ save info to DB --]
            db_vertex = bpe_802_1_br_my_info_DB["dev_info"];
            if db_vertex["vlan_info"] == nil then db_vertex["vlan_info"] = {}; end;
            db_vertex = db_vertex["vlan_info"];
            db_str = tostring(vlanId);
            if db_vertex[db_str] == nil then db_vertex[db_str] = {}; end;
            db_vertex = db_vertex[db_str];
            db_vertex["grp"]        = grp;
            db_vertex["e-cid-base"] = ecid_base;

            bpe_802_1_br_port_extender_flood_etag_cfg(devNum, vlanId, true);

            command_data:updateStatus();
            command_data:updateVlans();
        end
        command_data:analyzeCommandExecution();
        --[[ command_data:printCommandExecutionResults(); --]]

    elseif (role == "control_bridge") then
        --[[BPE 802.1BR  control_bridge support --]]
        --[[ ((grp << 12)) + ecid_base) --]]
        flood_vidx = bpe_802_1_br_mc_eVidx_get(grp,ecid_base);

        lttEntry = {};
        --[[ last L2Mll table emtry --]]
        lttEntry["mllPointer"] = 0x3FFF;
        --[[CPSS_DXCH_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E--]]
        lttEntry["entrySelector"] = 1;

        -- Check vlan context
        command_data:initInterfaceDevVlanRange();
        if (true ~= command_data["status"]) then
            print("Wrong context information status \n");
            return;
        end
        for iterator, devNum, vlanId in
                            command_data:getInterfaceVlanIterator() do

            apiName = "cpssDxChCfgTableNumEntriesGet"
            local isError , result, values = genericCpssApiWithErrorHandler(command_data,
                apiName, {
                {"IN","GT_U8","devNum",devNum},
                {"IN","CPSS_DXCH_CFG_TABLES_ENT","table","CPSS_DXCH_CFG_TABLE_MLL_PAIR_E"},
                {"OUT","GT_U32","numEntries"}
            })

            if isError then
                return
            end
            
            local lastMllPairIndex = values.numEntries - 1
            
            lttEntry["mllPointer"] = lastMllPairIndex -- not all devices supports 0x3FFF
                            
                            
            result, values  = myGenWrapper("cpssDxChBrgVlanFloodVidxModeSet",{
                        {"IN","GT_U8","devNum",devNum},
                        {"IN","GT_U16","vlanId",vlanId},
                        {"IN","GT_U16","floodVidx",flood_vidx},
                        {"IN",TYPE["ENUM"],"floodVidxMode",
                            1 --[["CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_ALL_FLOODED_TRAFFIC_E"--]]},
                });

            if (0 ~= result) then
                command_data:setFailVlanAndLocalStatus();
                command_data:addError("Error at Vlan Flood Vidx setting " ..
                                      "of device %d vlan %d: %s", devNum,
                                      vlanId, returnCodes[result]);
            end

            -- the ltt by default is not accessed for the first 4K entries
            lttEntryIndex = flood_vidx - 4096;
            local isError = bpe_set_l2mll_ltt_entry(command_data,devNum,lttEntryIndex,lttEntry)
            if isError then
                return
            end

            --[ save info to DB --]
            db_vertex = bpe_802_1_br_my_info_DB["dev_info"];
            if db_vertex["vlan_info"] == nil then db_vertex["vlan_info"] = {}; end;
            db_vertex = db_vertex["vlan_info"];
            db_str = tostring(vlanId);
            if db_vertex[db_str] == nil then db_vertex[db_str] = {}; end;
            db_vertex = db_vertex[db_str];
            db_vertex["grp"]        = grp;
            db_vertex["e-cid-base"] = ecid_base;

            command_data:updateStatus();
            command_data:updateVlans();
        end
        command_data:analyzeCommandExecution();
        --[[ command_data:printCommandExecutionResults(); --]]

    else
            print("The BPE 802.1BR role is wrong\n");
            return;
    end
end

local function bpe_802_1_br__flood_etag_info_remove(params)
    local command_data;
    local iterator, devNum;

    -- Common variables initialization
    command_data = Command_Data();
    -- Check vlan context
    command_data:initInterfaceDevVlanRange();
    if (true ~= command_data["status"]) then
        print("Wrong context information status \n");
        return;
    end
    for iterator, devNum, vlanId in
            command_data:getInterfaceVlanIterator() do
        bpe_802_1_br_port_extender_flood_etag_remove(devNum, vlanId);
    end
end

CLI_addCommand("vlan_configuration", "bpe-802-1-br flood-etag-info",
        { func = bpe_802_1_br__flood_etag_info,
          help = "bpe-802-1-br flood-etag-info manipulation",
          params =
          {
                { type="named",
                   { format = "grp %grp", name = "grp",
                   help = "the 2 bits <GRP> in the ETag format (values 1,2,3)" },
                   { format = "e-cid-base %ecid", name = "e-cid-base",
                   help = "the 12 bits <E-CID_base> in the ETag format (values 0..4K-1)" },
                   mandatory = {"grp", "e-cid-base"}
                }
          }
        });

CLI_addCommand("vlan_configuration", "no bpe-802-1-br flood-etag-info",
        { func = bpe_802_1_br__flood_etag_info_remove,
          help = "bpe-802-1-br flood-etag-info remove",
          params = {}
        });


function on_pe_evidx_flood_tti_default_rule(devNum, upstream_port, add_rule)
    local myIndexName;
    local ttiIndex, ttiAction;
    local myIndexName = 1;
    local finalTtiAction = {};
    local finalTtiMask = {};
    local finalTtiPattern = {};
    local ruleType = "CPSS_DXCH_TTI_RULE_UDB_10_E";
    local myRuleSize = 1;
    local apiName, result

    --***************
    --***************
    -- rule for 'downstream' Lookup 1:
    --
    myIndexName =
        "BPE: downstream(tti1) Default Rule" ..
        ", drop Unknown target packets";
    -- if myIndexName not exists in DB it will give 'new index' and saved into DB
    -- if myIndexName exists in DB it will retrieved from DB
    ttiIndex = allocTtiIndexFromPool(myRuleSize, myIndexName, "tti1", true --[[fromEnd--]]);
    if (ttiIndex == nil) then
        print("TTI1 rule allocation failed");
        print(myIndexName);
    end
    ttiAction = {}

    ---- force all packets on Upstream physical port that did not matched
    ---- the Group and ECid_base to assign vid0=0 --> cause drop !
    ttiAction['tag0VlanCmd']           = "CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E"
    ttiAction['tag1VlanCmd']           = "CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E"
    ttiAction['tag0VlanId']            = 0
    ttiAction['tag1VlanId']            = 0

    finalTtiAction["type2"] = ttiAction;

    finalTtiPattern["udbArray"], finalTtiMask["udbArray"] =
        bpe_802_1_br_buildTtiPatternMask(
            upstream_port,
            true,--ETag_exists
            nil, -- grp
            nil, -- ecid_base
            nil); -- ingress_ecid_base

    if add_rule == false then
        apiName = "cpssDxChTtiRuleValidStatusSet"
        result = myGenWrapper(
            apiName, {
            { "IN", "GT_U8", "devNum", devNum},
            { "IN", "GT_U32", "index", ttiIndex},
            { "IN", "GT_BOOL", "valid", GT_FALSE}
        })

        freeTtiIndexToPool(ttiIndex)
    else
        apiName = "cpssDxChTtiRuleSet"
        result = myGenWrapper(
            apiName ,{
            { "IN",     "GT_U8",                           "devNum",     devNum },
            { "IN",     "GT_U32",                          "index",      ttiIndex },
            { "IN",     "CPSS_DXCH_TTI_RULE_TYPE_ENT",     "ruleType",   ruleType},
            { "IN",     "CPSS_DXCH_TTI_RULE_UNT_udbArray", "patternPtr", finalTtiPattern},
            { "IN",     "CPSS_DXCH_TTI_RULE_UNT_udbArray", "maskPtr",    finalTtiMask},
            { "IN",     "CPSS_DXCH_TTI_ACTION_TYPE_ENT",   "actionType", "CPSS_DXCH_TTI_ACTION_TYPE2_ENT"},
            { "IN",     "CPSS_DXCH_TTI_ACTION_UNT_type2",  "actionPtr",  finalTtiAction},
        })
    end
end


