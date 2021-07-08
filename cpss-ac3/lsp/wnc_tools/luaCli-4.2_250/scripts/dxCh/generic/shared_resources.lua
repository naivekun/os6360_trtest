--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* shared_resources.lua
--*
--* DESCRIPTION:
--*       shared resources manager
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

-- prints tabls with such structure [key_string] = {base = base, size = size}
local function print_named_memory_table(tab)
    local key, entry;
    key = next(tab, nil);
    while key do
        entry = tab[key];
        print(
            string.format("0x%04X(%d)    ", entry.base, entry.size) ..
            "<" .. key .. ">");
        key = next(tab, key);
    end
end

-- prints tabls with such structure [rundom_key_number] = {base = base, size = size}
local function print_free_memory_table(tab)
    local key, entry, prt_str;
    prt_str = "free: ";
    key = next(tab, nil);
    while key do
        entry = tab[key];
        prt_str = prt_str .. string.format("0x%04X(%d) ", entry.base, entry.size);
        if string.len(prt_str) > 64 then
            print(prt_str);
            prt_str = "";
        end;
        key = next(tab, key);
    end
    if string.len(prt_str) > 0 then
        print(prt_str);
    end;
end

local function print_named_chain_l2nodes_table(tab)
    local key, entry, k1, e1;
    local pointer_str, node_str;
    key = next(tab, nil);
    while key do
        entry = tab[key];
        pointer_str = string.format("==> 0x%04X(%d)",
             entry.pointer.block, entry.pointer.node);
        print("<" .. key .. ">  " .. pointer_str);
        k1 = next(entry.nodes, nil);
        while k1 do
            e1 = entry.nodes[k1];
            node_str = string.format("0x%04X(%d)", e1.block, e1.node);
            print("<" .. k1 .. ">  " .. node_str);
            k1 = next(entry.nodes, k1);
        end
        key = next(tab, key);
    end
end

local function print_named_chain_l3nodes_table(tab)
    local key, entry, k1, e1;
    local pointer_str, node_str;
    key = next(tab, nil);
    while key do
        entry = tab[key];
        pointer_str = string.format("==> 0x%04X", entry.first_block);
        print("<" .. key .. ">  " .. pointer_str);
        k1 = next(entry.nodes, nil);
        while k1 do
            e1 = entry.nodes[k1];
            node_str = string.format("0x%04X(%d)", e1.block, e1.node);
            print("<" .. k1 .. ">  " .. node_str);
            k1 = next(entry.nodes, k1);
        end
        key = next(tab, key);
    end
end

-- prefix of all global functions: sharedResource or sharedResourceNamed

-- l2Ecmp entries table

-- entries 0-4095 initially free
local l2EcmpFreeMemoDB = {{base = 0, size = 0x2000}};
local l2EcmpNamedMemoryDB = {};

-- returns result or nil
function sharedResourceL2EcmpMemoAlloc(size)
    return DbMemoAlloc(l2EcmpFreeMemoDB, size);
end

-- returns true (OK) or nil (Error)
function sharedResourceL2EcmpMemoFree(base, size)
    return DbMemoFree(l2EcmpFreeMemoDB, base, size);
end

function sharedResourceNamedL2EcmpMemoReset(freeMemoDB)
    l2EcmpFreeMemoDB    = freeMemoDB;
    l2EcmpNamedMemoryDB = {};
end

-- returns result or nil
function sharedResourceNamedL2EcmpMemoAlloc(name, size)
    return DbNamedMemoryAlloc(
        l2EcmpNamedMemoryDB, l2EcmpFreeMemoDB, name,
        1 --[[alignment--]], size, "any" --[[options--]]);
end

-- returns true (OK) or nil (Error)
function sharedResourceNamedL2EcmpMemoFree(name)
    return DbNamedMemoryFree(l2EcmpNamedMemoryDB, l2EcmpFreeMemoDB, name);
end

CLI_addHelp("debug", "show shared l2ecmp-entries-db", "show shared L2ECMP entries DB")

CLI_addCommand("debug", "show shared l2ecmp-entries-db", {
    func = function(params)
        print ("");
        print ("start the shared L2ECMP entries DB");
        print_named_memory_table(l2EcmpNamedMemoryDB);
        print_free_memory_table(l2EcmpFreeMemoDB);
        print ("end the shared L2ECMP entries DB \n");
    end,
    help="Dump shared L2ECMP entries DB"
})

-- eVidx entries table

-- entries 4096-8191 initially free
local eVidxFreeMemoDB = {{base = 0x1000, size = 0x1000}};
local eVidxNamedMemoryDB = {};

function sharedResourceNamedEVidxMemoReset(freeMemoDB)
    eVidxFreeMemoDB    = freeMemoDB;
    eVidxNamedMemoryDB = {};
end

-- returns result or nil
function sharedResourceNamedEVidxMemoAlloc(name, size)
    return DbNamedMemoryAlloc(
        eVidxNamedMemoryDB, eVidxFreeMemoDB, name,
        1 --[[alignment--]], size, "any" --[[options--]]);
end

-- returns true (OK) or nil (Error)
function sharedResourceNamedEVidxMemoFree(name)
    return DbNamedMemoryFree(eVidxNamedMemoryDB, eVidxFreeMemoDB, name);
end

CLI_addHelp("debug", "show bridge evidx-entries-db", "show Bridge EVidx entries DB")

CLI_addCommand("debug", "show shared evidx-entries-db", {
    func = function(params)
        print ("");
        print ("start the shared EVidx entries DB");
        print_named_memory_table(eVidxNamedMemoryDB);
        print_free_memory_table(eVidxFreeMemoDB);
        print ("end the shared EVidx entries DB \n");
    end,
    help="Dump shared EVidx entries DB"
})

-- VIDX entries table

-- entries 0-4095 initially free
local vidxFreeMemoDB = {{base = 0, size = 0x1000}};
local vidxNamedMemoryDB = {};

function sharedResourceNamedVidxMemoReset(freeMemoDB)
    vidxFreeMemoDB    = freeMemoDB;
    vidxNamedMemoryDB = {};
end

-- returns result or nil
function sharedResourceNamedVidxMemoAlloc(name, size)
    return DbNamedMemoryAlloc(
        vidxNamedMemoryDB, vidxFreeMemoDB, name,
        1 --[[alignment--]], size, "any" --[[options--]]);
end

-- returns true (OK) or nil (Error)
function sharedResourceNamedVidxMemoFree(name)
    return DbNamedMemoryFree(vidxNamedMemoryDB, vidxFreeMemoDB, name);
end

CLI_addHelp("debug", "show bridge vidx-entries-db", "show Bridge Vidx entries DB")

CLI_addCommand("debug", "show shared vidx-entries-db", {
    func = function(params)
        print ("");
        print ("start the shared Vidx entries DB");
        print_named_memory_table(vidxNamedMemoryDB);
        print_free_memory_table(vidxFreeMemoDB);
        print ("end the shared Vidx entries DB \n");
    end,
    help="Dump shared Vidx entries DB"
})

-- eVid entries table

-- entries 4096-8191 initially free
local eVidFreeMemoDB = {{base = 0x1000, size = 0x1000}};
local eVidNamedMemoryDB = {};

function sharedResourceNamedEVidMemoReset(freeMemoDB)
    eVidFreeMemoDB    = freeMemoDB;
    eVidNamedMemoryDB = {};
end

-- returns result or nil
function sharedResourceNamedEVidMemoAlloc(name, size)
    return DbNamedMemoryAlloc(
        eVidNamedMemoryDB, eVidFreeMemoDB, name,
        1 --[[alignment--]], size, "any" --[[options--]]);
end

-- returns true (OK) or nil (Error)
function sharedResourceNamedEVidMemoFree(name)
    return DbNamedMemoryFree(eVidNamedMemoryDB, eVidFreeMemoDB, name);
end

CLI_addHelp("debug", "show shared evid-entries-db", "show Bridge EVid entries DB")

CLI_addCommand("debug", "show shared evid-entries-db", {
    func = function(params)
        print ("");
        print ("start the shared EVid entries DB");
        print_named_memory_table(eVidNamedMemoryDB);
        print_free_memory_table(eVidFreeMemoDB);
        print ("end the shared EVid entries DB \n");
    end,
    help="Dump shared EVid entries DB"
})

-- L2Mll and L3Mll entries table
-- used the same memory but enries have different format

-- L2Mll and L3Mll common
-- entries 0-16K initially free
local l2mllAndL3mllFreeMemoDB = {{base = 0, size = (16*1024)}};

-- L2Mll only
local l2mllNamedMemoryDB = {};
local l2mllBlockedNodesDb = {
    nodes_in_block = 2,
    blocks = {}};
local l2mllBlockedNodesNamesDb = {};

-- L3Mll only
local l3mllNamedMemoryDB = {};
local l3mllBlockedNodesDb = {
    nodes_in_block = 2,
    blocks = {}};
local l3mllBlockedNodesNamesDb = {};

-- reset both L2Mll and L3Mll DB
function sharedResourceNamedL2andL3MllMemoReset(freeMemoDB)
    l2mllAndL3mllFreeMemoDB    = freeMemoDB;
    l2mllNamedMemoryDB = {};
    l2mllBlockedNodesDb = {
        nodes_in_block = 2,
        blocks = {}};
    l3mllBlockedNodesNamesDb = {};
    l3mllNamedMemoryDB = {};
    l3mllBlockedNodesDb = {
        nodes_in_block = 2,
        blocks = {}};
    l3mllBlockedNodesNamesDb = {};
end

-- returns result or nil
function sharedResourceNamedL2MllMemoAlloc(name, size)
    return DbNamedMemoryAlloc(
        l2mllNamedMemoryDB, l2mllAndL3mllFreeMemoDB, name,
        1 --[[alignment--]], size, "any" --[[options--]]);
end

-- returns true (OK) or nil (Error)
function sharedResourceNamedL2MllMemoFree(name)
    return DbNamedMemoryFree(l2mllNamedMemoryDB, l2mllAndL3mllFreeMemoDB, name);
end

function sharedResourceNamedL2MllBlockedNodesAddNode(chainName, nodeName)
    return DbNamedBlockedNodesAddNode(
        l2mllAndL3mllFreeMemoDB, l2mllBlockedNodesDb, l2mllBlockedNodesNamesDb,
        chainName, nodeName);
end

function sharedResourceNamedL2MllBlockedNodesRemoveNode(chainName, nodeName)
    return DbNamedBlockedNodesRemoveNode(
        l2mllAndL3mllFreeMemoDB, l2mllBlockedNodesDb, l2mllBlockedNodesNamesDb,
        chainName, nodeName);
end

function sharedResourceNamedL2MllBlockedNodesGetNodesNames(chainName)
    local chain, k, i, nodes, names;
    chain = l2mllBlockedNodesNamesDb[chainName];
    if not chain then
        return nil;
    end
    nodes = chain.nodes;
    i = 1;
    names = {};
    k = next(nodes, nil)
    while k do
        names[i] = k;
        i = i + 1;
        k = next(nodes, k);
    end
    return names;
end

CLI_addHelp("debug", "show shared l2mll-entries-db", "show shared L2Mll entries DB")

CLI_addCommand("debug", "show shared l2mll-entries-db", {
    func = function(params)
        print ("");
        print ("start the shared L2Mll entries DB");
        print_named_memory_table(l2mllNamedMemoryDB);
        print_free_memory_table(l2mllAndL3mllFreeMemoDB);
        print_named_chain_l2nodes_table(l2mllBlockedNodesNamesDb);
        print(to_string(l2mllBlockedNodesDb));
        print ("end the shared L2Mll entries DB \n");
    end,
    help="Dump shared L2Mll entries DB"
})

-- L3Mll

-- returns result or nil
function sharedResourceNamedL3MllMemoAlloc(name, size)
    return DbNamedMemoryAlloc(
        l3mllNamedMemoryDB, l2mllAndL3mllFreeMemoDB, name,
        1 --[[alignment--]], size, "any" --[[options--]]);
end

-- returns true (OK) or nil (Error)
function sharedResourceNamedL3MllMemoFree(name)
    return DbNamedMemoryFree(l3mllNamedMemoryDB, l2mllAndL3mllFreeMemoDB, name);
end

function sharedResourceNamedL3MllBlockedNodesAppendNode(chainName, nodeName)
    return DbNamedBlockedL3NodesAppendNode(
        l2mllAndL3mllFreeMemoDB, l3mllBlockedNodesDb, l3mllBlockedNodesNamesDb,
        chainName, nodeName);
end

function sharedResourceNamedL3MllBlockedNodesRemoveChain(chainName)
    return DbNamedBlockedL3NodesRemoveChain(
        l2mllAndL3mllFreeMemoDB, l3mllBlockedNodesDb, l3mllBlockedNodesNamesDb,
        chainName);
end

function sharedResourceNamedL3MllBlockedNodesGetNodesNames(chainName)
    local chain, k, i, nodes, names;
    chain = l3mllBlockedNodesNamesDb[chainName];
    if not chain then
        return nil;
    end
    nodes = chain.nodes;
    i = 1;
    names = {};
    k = next(nodes, nil)
    while k do
        names[i] = k;
        i = i + 1;
        k = next(nodes, k);
    end
    return names;
end

CLI_addHelp("debug", "show shared l3mll-entries-db", "show shared L3Mll entries DB")

CLI_addCommand("debug", "show shared l3mll-entries-db", {
    func = function(params)
        print ("");
        print ("start the shared L3Mll entries DB");
        print_named_memory_table(l3mllNamedMemoryDB);
        print_free_memory_table(l2mllAndL3mllFreeMemoDB);
        print_named_chain_l3nodes_table(l3mllBlockedNodesNamesDb);
        print(to_string(l3mllBlockedNodesDb));
        print ("end the shared L2Mll entries DB \n");
    end,
    help="Dump shared L3Mll entries DB"
})


-- MAC TO ME table

-- entries 0-255 initially free
local mac2meFreeMemoDB = {{base = 0, size = 255}};
local mac2meNamedMemoryDB = {};

function sharedResourceNamedMac2meMemoReset(freeMemoDB)
    mac2meFreeMemoDB    = freeMemoDB;
    mac2meNamedMemoryDB = {};
end

-- returns result or nil
function sharedResourceNamedMac2meMemoAlloc(name, size, fromEnd)
    local options;
    if fromEnd == true then
        options = "high";
    else
        options = "low";
    end
    return DbNamedMemoryAlloc(
        mac2meNamedMemoryDB, mac2meFreeMemoDB, name,
        1 --[[alignment--]], size, options);
end

-- returns true (OK) or nil (Error)
function sharedResourceNamedMac2meMemoFree(name)
    return DbNamedMemoryFree(mac2meNamedMemoryDB, mac2meFreeMemoDB, name);
end

CLI_addHelp("debug", "show shared mac2me-entries-db", "show shared MAC2ME entries DB")

CLI_addCommand("debug", "show shared mac2me-entries-db", {
    func = function(params)
        print ("");
        print ("start the shared MAC2ME entries DB");
        print_named_memory_table(mac2meNamedMemoryDB);
        print_free_memory_table(mac2meFreeMemoDB);
        print ("end the shared MAC2ME entries DB \n");
    end,
    help="Dump shared MAC2ME entries DB"
})

-- Tunnel Start table
-- This memory consists of 16K lines
-- Each line can contain:
-- 8 ARP Entries
-- 2 TS Entries
-- 1 TS Entry and 4 ARP Entries (in any order)
-- 1 IPV6_TS Entry
-- free memory measured in ARP-entry units
-- for ARP entry alignment = 1, size = 1
-- for TS entry alignment = 4, size = 4
-- for IPV6 TS entry alignment = 8, size = 8

-- entries 0-(16K * 8) initially free
local arpTsNatEntriesFreeMemoDB = {{base = 0, size = (16*1024*8)}};
local arpEntriesNamedMemoryDB = {};
local tsEntriesNamedMemoryDB = {};
local ipv6TsEntriesNamedMemoryDB = {};
local natEntriesNamedMemoryDB = {};

function sharedResourceNamedTsMemoReset(freeMemoDB)
    arpTsNatEntriesFreeMemoDB    = freeMemoDB;
    arpEntriesNamedMemoryDB = {};
    tsEntriesNamedMemoryDB = {};
    ipv6TsEntriesNamedMemoryDB = {};
    natEntriesNamedMemoryDB = {};
end

-- returns result or nil
function sharedResourceNamedArpMemoAlloc(name, size)
    return DbNamedMemoryAlloc(
        arpEntriesNamedMemoryDB, arpTsNatEntriesFreeMemoDB, name,
        1 --[[alignment--]], size, "any" --[[options--]]);
end

-- returns true (OK) or nil (Error)
function sharedResourceNamedArpMemoFree(name)
    return DbNamedMemoryFree(arpEntriesNamedMemoryDB, arpTsNatEntriesFreeMemoDB, name);
end

-- returns result or nil
function sharedResourceNamedTsMemoAlloc(name, size)
    local res = DbNamedMemoryAlloc(
        tsEntriesNamedMemoryDB, arpTsNatEntriesFreeMemoDB, name,
        4 --[[alignment--]], (size * 4), "any" --[[options--]]);
    if res then
        res = res / 4;
    end
    return res;
end

-- returns true (OK) or nil (Error)
function sharedResourceNamedTsMemoFree(name)
    return DbNamedMemoryFree(tsEntriesNamedMemoryDB, arpTsNatEntriesFreeMemoDB, name);
end

-- returns result or nil
function sharedResourceNamedIpv6TsMemoAlloc(name, size)
    local res = DbNamedMemoryAlloc(
        ipv6TsEntriesNamedMemoryDB, arpTsNatEntriesFreeMemoDB, name,
        8 --[[alignment--]], (size * 8), "any" --[[options--]]);
    if res then
        res = res / 8;
    end
    return res;
end

-- returns true (OK) or nil (Error)
function sharedResourceNamedIpv6TsMemoFree(name)
    return DbNamedMemoryFree(ipv6TsEntriesNamedMemoryDB, arpTsNatEntriesFreeMemoDB, name);
end

-- returns result or nil
function sharedResourceNamedNatMemoAlloc(name, size)
    local res = DbNamedMemoryAlloc(
        natEntriesNamedMemoryDB, arpTsNatEntriesFreeMemoDB, name,
        4 --[[alignment--]], (size * 4), "any" --[[options--]]);
    if res then
        res = res / 4;
    end
    return res;
end

-- returns true (OK) or nil (Error)
function sharedResourceNamedNatMemoFree(name)
    return DbNamedMemoryFree(natEntriesNamedMemoryDB, arpTsNatEntriesFreeMemoDB, name);
end

CLI_addHelp("debug", "show tunnel-start-entries-db",
            "show ARP, TS, IPV6-TS and NAT entries DB")

CLI_addCommand("debug", "show shared tunnel-start-entries-db", {
    func = function(params)
        print ("");
        print ("start the shared ARP, TS, IPV6-TS and NAT entries DB");
        print_named_memory_table(arpEntriesNamedMemoryDB);
        print_named_memory_table(tsEntriesNamedMemoryDB);
        print_named_memory_table(ipv6TsEntriesNamedMemoryDB);
        print_named_memory_table(natEntriesNamedMemoryDB);
        print_free_memory_table(arpTsNatEntriesFreeMemoDB);
        print ("end the ARP, TS, IPV6-TS and NAT entries DB \n");
    end,
    help="Dump shared ARP, TS, IPV6-TS and NAT entries DB"
})

-- PCL Rules table

-- entries 0-18K initially free
local pclRulesFreeMemoDB = {{base = 0, size = (18*1024)}};
local pclRulesNamedMemoryDB = {};

local ipcl0 = "ipcl0"
local ipcl1 = "ipcl1"
local ipcl2 = "ipcl2"
local epcl  = "epcl"

local CPSS_DXCH_TCAM_IPCL_0_E = "CPSS_DXCH_TCAM_IPCL_0_E"
local CPSS_DXCH_TCAM_IPCL_1_E = "CPSS_DXCH_TCAM_IPCL_1_E"
local CPSS_DXCH_TCAM_IPCL_2_E = "CPSS_DXCH_TCAM_IPCL_2_E"
local CPSS_DXCH_TCAM_EPCL_E   = "CPSS_DXCH_TCAM_EPCL_E"

local pclClientsAware = {
    [ipcl0]=nil, --{RulesFreeMemoDB , RulesNamedMemoryDB}
    [ipcl1]=nil,
    [ipcl2]=nil,
    [epcl] =nil
    }

-- return pclRulesFreeMemoDB , pclRulesNamedMemoryDB of the pclClient
local function pclGetDbInfo(pclClient)
    if pclClient == nil then
        pclClient = "ipcl0"
    end

    if pclClientsAware[pclClient] == nil then
        return pclRulesFreeMemoDB , pclRulesNamedMemoryDB
    end
    
    return pclRulesFreeMemoDB[pclClient] , pclRulesNamedMemoryDB[pclClient]
end
    
-- ************************************************************************
--  sharedResourceNamedPclRulesMemoInfoGet
--
--  @description get PCL TCAM base index and size by specified clients.
--
--  @param devNum - a device number
--  @param pclClient - can be a single value of
--                   'ipcl0', 'ipcl1', 'ipcl2', 'epcl'
--                   
--  @return 'base' and 'size' of the client
--
-- ************************************************************************
function sharedResourceNamedPclRulesMemoInfoGet(pclClient)
    local RulesFreeMemoDB , RulesNamedMemoryDB = pclGetDbInfo(pclClient)
    
    --print("pclClient " .. to_string(pclClient) .. "RulesFreeMemoDB" .. to_string(RulesFreeMemoDB))
    
    return RulesFreeMemoDB-- in format of : {{base = 0, size = (18*1024)}}
end

function sharedResourceNamedPclRulesMemoReset(freeMemoDB)
     pclRulesFreeMemoDB    = freeMemoDB;
     pclRulesNamedMemoryDB = {};
     
     --print("freeMemoDB ", to_string(freeMemoDB))
    
    if freeMemoDB[CPSS_DXCH_TCAM_IPCL_0_E] then
        pclRulesFreeMemoDB[ipcl0]    = freeMemoDB[CPSS_DXCH_TCAM_IPCL_0_E]
        pclRulesNamedMemoryDB[ipcl0] = {}
        pclClientsAware[ipcl0] = {pclRulesFreeMemoDB[ipcl0],pclRulesNamedMemoryDB[ipcl0]}
    end

    if freeMemoDB[CPSS_DXCH_TCAM_IPCL_1_E] then
        pclRulesFreeMemoDB[ipcl1]    = freeMemoDB[CPSS_DXCH_TCAM_IPCL_1_E]
        pclRulesNamedMemoryDB[ipcl1] = {}
        pclClientsAware[ipcl1] = {pclRulesFreeMemoDB[ipcl1],pclRulesNamedMemoryDB[ipcl1]}
    end
    
    if freeMemoDB[CPSS_DXCH_TCAM_IPCL_2_E] then
        pclRulesFreeMemoDB[ipcl2]    = freeMemoDB[CPSS_DXCH_TCAM_IPCL_2_E]
        pclRulesNamedMemoryDB[ipcl2] = {}
        pclClientsAware[ipcl2] = {pclRulesFreeMemoDB[ipcl2],pclRulesNamedMemoryDB[ipcl2]}
    end
    
    if freeMemoDB[CPSS_DXCH_TCAM_EPCL_E] then
        pclRulesFreeMemoDB[epcl]    = freeMemoDB[CPSS_DXCH_TCAM_EPCL_E]
        pclRulesNamedMemoryDB[epcl] = {}
        pclClientsAware[epcl] = {pclRulesFreeMemoDB[epcl],pclRulesNamedMemoryDB[epcl]}
    end
    
    freeMemoDB[CPSS_DXCH_TCAM_IPCL_0_E] = nil -- replaced by short name 'ipcl0' if existed
    freeMemoDB[CPSS_DXCH_TCAM_IPCL_1_E] = nil -- replaced by short name 'ipcl1' if existed
    freeMemoDB[CPSS_DXCH_TCAM_IPCL_2_E] = nil -- replaced by short name 'ipcl2' if existed
    freeMemoDB[CPSS_DXCH_TCAM_EPCL_E]   = nil -- replaced by short name 'epcl'  if existed

    sharedResourceNamedPclRulesMemoInfoGet(epcl)
end

-- returns result or nil
-- pclClient - "ipcl0"/"ipcl1"/"ipcl1"/"epcl"/nil
--              if nil considered as "ipcl0"
function sharedResourceNamedPclRulesMemoAlloc(name, alignment, size, fromEnd , pclClient)
    local options;
    if fromEnd == true then
        options = "high";
    else
        options = "low";
    end
    
    local RulesFreeMemoDB , RulesNamedMemoryDB = pclGetDbInfo(pclClient)
    
    return DbNamedMemoryAlloc(
         RulesNamedMemoryDB,  RulesFreeMemoDB, name,
         alignment, size, options);
end

-- returns true (OK) or nil (Error)
function sharedResourceNamedPclRulesMemoFree(name, pclClient)
    local RulesFreeMemoDB , RulesNamedMemoryDB = pclGetDbInfo(pclClient)

    return DbNamedMemoryFree( RulesNamedMemoryDB,  RulesFreeMemoDB, name);
end

CLI_addHelp("debug", "show shared pcl-tcam-entries-db", "show shared Pcl Rules DB")

CLI_addCommand("debug", "show shared pcl-tcam-entries-db", {
    func = function(params)
        print ("");
        print ("start the shared Pcl Rules DB");
        
        local perClient = false
        
        if(pclRulesFreeMemoDB[ipcl0]) then
            perClient = true
            print ("start the shared Pcl Rules DB : IPCL0 ");
            print_named_memory_table(pclRulesNamedMemoryDB[ipcl0]);
            print_free_memory_table(pclRulesFreeMemoDB[ipcl0]);
            print ("end the shared Pcl Rules DB : IPCL0 ");
        end

        if(pclRulesFreeMemoDB[ipcl1]) then
            perClient = true
            print ("start the shared Pcl Rules DB : IPCL1 ");
            print_named_memory_table(pclRulesNamedMemoryDB[ipcl1]);
            print_free_memory_table(pclRulesFreeMemoDB[ipcl1]);
            print ("end the shared Pcl Rules DB : IPCL1 ");
        end

        if(pclRulesFreeMemoDB[ipcl2]) then
            perClient = true
            print ("start the shared Pcl Rules DB : IPCL2 ");
            print_named_memory_table(pclRulesNamedMemoryDB[ipcl2]);
            print_free_memory_table(pclRulesFreeMemoDB[ipcl2]);
            print ("end the shared Pcl Rules DB : IPCL2 ");
        end

        if(pclRulesFreeMemoDB[epcl]) then
            perClient = true
            print ("start the shared Pcl Rules DB : EPCL ");
            print_named_memory_table(pclRulesNamedMemoryDB[epcl]);
            print_free_memory_table(pclRulesFreeMemoDB[epcl]);
            print ("end the shared Pcl Rules DB : EPCL ");
        end
        
        if(perClient) then
            print_named_memory_table(pclRulesNamedMemoryDB);
            print_free_memory_table(pclRulesFreeMemoDB);
        end
        
        print ("end the shared Pcl Rules DB \n");
    end,
    help="Dump Pcl Rules DB"
})

-- TTI0 Rules table

-- entries 18K-30K initially free
local tti0RulesFreeMemoDB = {{base = (18*1024), size = ((30 - 18)*1024)}};
local tti0RulesNamedMemoryDB = {};

function sharedResourceNamedTti0RulesMemoReset(freeMemoDB)
     tti0RulesFreeMemoDB    = freeMemoDB;
     tti0RulesNamedMemoryDB = {};
end

-- returns result or nil
function sharedResourceNamedTti0RulesMemoAlloc(name, alignment, size, fromEnd)
    local options;
    if fromEnd == true then
        options = "high";
    else
        options = "low";
    end
    return DbNamedMemoryAlloc(
         tti0RulesNamedMemoryDB,  tti0RulesFreeMemoDB, name,
         alignment, size, options);
end

-- returns true (OK) or nil (Error)
function sharedResourceNamedTti0RulesMemoFree(name)
    return DbNamedMemoryFree( tti0RulesNamedMemoryDB,  tti0RulesFreeMemoDB, name);
end

CLI_addHelp("debug", "show tti0 tcam-entries-db", "show TTI0 Rules DB")

CLI_addCommand("debug", "show shared tti0-tcam-entries-db", {
    func = function(params)
        print ("");
        print ("start the shared TTI0 Rules DB");
        print_named_memory_table(tti0RulesNamedMemoryDB);
        print_free_memory_table(tti0RulesFreeMemoDB);
        print ("end the shared TTI0 Rules DB \n");
    end,
    help="Dump shared TTI0 Rules DB"
})

-- TTI1 Rules table

-- entries 30K-33K initially free
local tti1RulesFreeMemoDB = {{base = (30*1024), size = ((33 - 30)*1024)}};
local tti1RulesNamedMemoryDB = {};

function sharedResourceNamedTti1RulesMemoReset(freeMemoDB)
     tti1RulesFreeMemoDB    = freeMemoDB;
     tti1RulesNamedMemoryDB = {};
end

-- returns result or nil
function sharedResourceNamedTti1RulesMemoAlloc(name, alignment, size, fromEnd)
    local options;
    if fromEnd == true then
        options = "high";
    else
        options = "low";
    end
    return DbNamedMemoryAlloc(
         tti1RulesNamedMemoryDB,  tti1RulesFreeMemoDB, name,
         alignment, size, options);
end

-- returns true (OK) or nil (Error)
function sharedResourceNamedTti1RulesMemoFree(name)
    return DbNamedMemoryFree( tti1RulesNamedMemoryDB,  tti1RulesFreeMemoDB, name);
end

CLI_addHelp("debug", "show shared tti1-tcam-entries-db", "show shared TTI1 Rules DB")

CLI_addCommand("debug", "show shared tti1-tcam-entries-db", {
    func = function(params)
        print ("");
        print ("start the shared TTI1 Rules DB");
        print_named_memory_table(tti1RulesNamedMemoryDB);
        print_free_memory_table(tti1RulesFreeMemoDB);
        print ("end the shared TTI1 Rules DB \n");
    end,
    help="Dump TTI1 Rules DB"
})

local list_of_named_memory_dbs = {
    {db = l2EcmpNamedMemoryDB, name = "l2Ecmp"},
    {db = eVidxNamedMemoryDB, name = "eVidx"},
    {db = vidxNamedMemoryDB, name = "vidx"},
    {db = eVidNamedMemoryDB, name = "eVid"},
    {db = l2mllNamedMemoryDB, name = "l2mll"},
    {db = mac2meNamedMemoryDB, name = "mac2me"},
    {db = l2mllBlockedNodesNamesDb, name = "l2mllBlockedNodesNamesDb",
        print_function =  print_named_chain_l2nodes_table},
    {db = l3mllBlockedNodesNamesDb, name = "l3mllBlockedNodesNamesDb",
        print_function =  print_named_chain_l3nodes_table},
    {db = arpEntriesNamedMemoryDB, name = "arpEntries"},
    {db = tsEntriesNamedMemoryDB, name = "tunnelStart(index * 4)"},
    {db = ipv6TsEntriesNamedMemoryDB, name = "ipv6TunnelStart(index * 8)"},
    {db = natEntriesNamedMemoryDB, name = "NAT enties (index * 4)"},
    {db = pclRulesNamedMemoryDB, name = "pclRules"},
    {db = tti0RulesNamedMemoryDB, name = "tti0Rules"},
    {db = tti1RulesNamedMemoryDB, name = "tti1Rules"}
}

CLI_addHelp("debug", "show shared not-empty-table-dbs", "show all shared not empty DBs")

CLI_addCommand("debug", "show shared not-empty-table-dbs", {
    func = function(params)
        local key, entry, print_function;
        print ("");
        print ("start the shared not empty DB");
        key = next(list_of_named_memory_dbs, nil);
        while key do
            entry = list_of_named_memory_dbs[key];
            print_function = entry.print_function;
            if print_function == nil then
                print_function = print_named_memory_table;
            end
            if next(entry.db, nil) ~= nul then
                print("table: " .. entry.name);
                print_function(entry.db);
            end
            key = next(list_of_named_memory_dbs, key);
        end
        print ("end the shared not empty DB \n");
    end,
    help="Dump all not-empty named memories DBs"
})

