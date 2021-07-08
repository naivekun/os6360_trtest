--******************************************************************************
--*              (c), Copyright 2017, Marvell International Ltd.               *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC. *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT*
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE      *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.   *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,     *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE. *
--******************************************************************************
--* show_pipe_counters.lua
--*
--* DESCRIPTION:
--*       Centralized Counters (CNC) commands for PIPE
--*
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--******************************************************************************

cmdLuaCLI_registerCfunction("wrlCpssGtU64StrGet")

local eq_tdrop_head =   "| Device | Port | Traffic Class | Drop Prcedence | Packets | Bytes |\n"..
                        "--------------------------------------------------------------------\n"
local eq_tdrop_fmt =    "|%-8d|%-6d|%-15d|%-16d|%-9d|%-7d|\n"
local eq_cn_head =      "| Device | Port | Traffic Class | CN | Action | Packets | Bytes |\n"..
                        "-----------------------------------------------------------------\n"
local eq_cn_fmt =       "|%-8d|%-6d|%-15d|%-4s|%-8s|%-9d|%-7d|\n"
local eq_reduced_head = "| Device | Port | Traffic Class | Action | Packets | Bytes |\n"..
                        "------------------------------------------------------------\n"
local eq_reduced_fmt =  "|%-8d|%-6d|%-15d|%-8s|%-9d|%-7d|\n"
local ingress_head =    "| Device | Dst index | Packets | Bytes |\n"..
                        "----------------------------------------\n"
local ingress_fmt =     "|%-8d|%-11d|%-9d|%-7d|\n"
local egress_head =     "| Device | Port | Packet Type | Packets | Bytes |\n"..
                        "-------------------------------------------------\n"
local egress_fmt =      "|%-8d|%-6d|%-13d|%-9d|%-7d|\n"

local mcfc_cntr_head =   "| Device | Counter Type | Paclkets |\n"..
                        "--------------------------------------------------------------------\n"
local mcfc_cntr__fmt =    "|%-8d|%-16d|%-7d|\n"

function counterGet(dev, index)
    local ret
    local result
    local packets
    local bytes

    ret, result = myGenWrapper("cpssPxCncCounterGet",{
        {"IN",  "GT_U8",                         "devNum",   dev},
        {"IN",  "GT_U32",                        "blockNum", 0},
        {"IN",  "GT_U32",                        "index",    index},
        {"IN",  "CPSS_PX_CNC_COUNTER_FORMAT_ENT","client",   "CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E"},
        {"OUT", "CPSS_PX_CNC_COUNTER_STC",       "counterPtr"}
    })
    if ret~=0 then
        print("cpssPxCncCounterGet() failed: "..returnCodes[ret])
        return false,"Error, counter could not be read"
    end

    packets = bit_or(bit_shl(result.counterPtr.packetCount.l[1], 32), result.counterPtr.packetCount.l[0])
    bytes = bit_or(bit_shl(result.counterPtr.byteCount.l[1], 32), result.counterPtr.byteCount.l[0])
    return packets, bytes
end

local function eq_tdrop_indexes(params)
    local indexes = {}
    local idx = 1
    local include
    for i=0,1023 do
        include = true
        if params.tc then
            tclass = bit_and(bit_shr(i, 2), 0x7)
            if tclass ~= params.tc then
                include = false
            end
        end

        local portnum = bit_and(bit_shr(i, 5), 0x1F)
        if portnum > 16 then
            include = false
        end
        if params.ports then
            local inrange = false
            for idx,ports in pairs(params.ports) do
                if type(ports) == "number" then
                    if portnum == ports then
                        inrange = true
                    end
                else
                    for port=ports[1],ports[2] do
                        if portnum == port then
                            inrange = true
                        end
                    end
                end
            end
            if inrange == false then
                include = false
            end
        end

        if params.dp then
            local dp = bit_and(i, 0x3)
            if dp ~= params.dp then
                include = false
            end
        end

        indexes[idx] = include
        idx = idx + 1
    end
    return indexes
end

local function eq_cn_indexes(params)
    local indexes = {}
    local idx = 1
    local include
    for i=0,1023 do
        include = true
        if params.tc then
            tclass = bit_and(bit_shr(i, 2), 0x7)
            if tclass ~= params.tc then
                include = false
            end
        end

        local portnum = bit_and(bit_shr(i, 5), 0x1F)
        if portnum > 16 then
            include = false
        end
        if params.ports then
            local inrange = false
            for idx,ports in pairs(params.ports) do
                if type(ports) == "number" then
                    if portnum == ports then
                        inrange = true
                    end
                else
                    for port=ports[1],ports[2] do
                        if portnum == port then
                            inrange = true
                        end
                    end
                end
            end
            if inrange == false then
                include = false
            end
        end

        if params.cn then
            local cn = bit_and(i, 0x1)
            if cn == 0 then
                include = false
            end
        end

        if params.ncn then
            local cn = bit_and(i, 0x1)
            if cn == 1 then
                include = false
            end
        end

        if params.pass then
            local padr = bit_and(bit_shr(i, 1), 0x1)
            if padr == 1 then
                include = false
            end
        end

        if params.drop then
            local padr = bit_and(bit_shr(i, 1), 0x1)
            if padr == 0 then
                include = false
            end
        end

        indexes[idx] = include
        idx = idx + 1
    end
    return indexes
end

local function eq_reduced_indexes(params)
    local indexes = {}
    local idx = 1
    local include
    for i=0,511 do
        include = true
        if params.tc then
            tclass = bit_and(bit_shr(i, 1), 0x7)
            if tclass ~= params.tc then
                include = false
            end
        end

        local portnum = bit_and(bit_shr(i, 4), 0x1F)
        if portnum > 16 then
            include = false
        end
        if params.ports then
            local inrange = false
            for idx,ports in pairs(params.ports) do
                if type(ports) == "number" then
                    if portnum == ports then
                        inrange = true
                    end
                else
                    for port=ports[1],ports[2] do
                        if portnum == port then
                            inrange = true
                        end
                    end
                end
            end
            if inrange == false then
                include = false
            end
        end

        if params.pass then
            local padr = bit_and(i, 0x1)
            if padr == 1 then
                include = false
            end
        end

        if params.drop then
            local padr = bit_and(i, 0x1)
            if padr == 0 then
                include = false
            end
        end

        indexes[idx] = include
        idx = idx + 1
    end
    return indexes
end

local function ingress_indexes(params)
    local indexes = {}
    local idx = 1
    local include
    for i=0,1023 do
        include = true

        if params.indices then
            local inrange = false
            for idx,range in pairs(params.indices) do
                if type(range) == "number" then
                    if range == i then
                        inrange = true
                    end
                else
                    for rng=range[1],range[2] do
                        if rng == i then
                            inrange = true
                        end
                    end
                end
            end
            if inrange == false then
                include = false
            end
        end

        indexes[idx] = include
        idx = idx + 1
    end
    return indexes
end

local function egress_indexes(params)
    local indexes = {}
    local idx = 1
    local include
    for i=0,1023 do
        include = true
        if params.ptype then
            ptype = bit_and(i, 0x1F)
            if ptype ~= params.ptype then
                include = false
            end
        end

        local portnum = bit_and(bit_shr(i, 5), 0x1F)
        if portnum > 16 then
            include = false
        end
        if params.ports then
            local inrange = false
            for idx,ports in pairs(params.ports) do
                if type(ports) == "number" then
                    if portnum == ports then
                        inrange = true
                    end
                else
                    for port=ports[1],ports[2] do
                        if portnum == port then
                            inrange = true
                        end
                    end
                end
            end
            if inrange == false then
                include = false
            end
        end

        indexes[idx] = include
        idx = idx + 1
    end
    return indexes
end

local function eq_tdrop_print(fmt, index, dev, packets, bytes)
    local tclass = bit_and(bit_shr(index, 2), 0x7)
    local portnum = bit_and(bit_shr(index, 5), 0x1F)
    local dp = bit_and(index, 0x3)
    return string.format(fmt, dev, portnum, tclass, dp, packets, bytes)
end

local function eq_cn_print(fmt, index, dev, packets, bytes)
    local tclass = bit_and(bit_shr(index, 2), 0x7)
    local portnum = bit_and(bit_shr(index, 5), 0x1F)
    local cn = bit_and(index, 0x1)
    local padr = bit_and(bit_shr(index, 1), 0x1)
    local cnotify
    local passdrop
    if cn == 1 then
        cnotify = "yes"
    else
        cnotify = "no"
    end
    if padr == 1 then
        passdrop = "drop"
    else
        passdrop = "pass"
    end
    return string.format(fmt, dev, portnum, tclass, cnotify, passdrop, packets, bytes)
end

local function eq_reduced_print(fmt, index, dev, packets, bytes)
    local tclass = bit_and(bit_shr(index, 1), 0x7)
    local portnum = bit_and(bit_shr(index, 4), 0x1F)
    local padr = bit_and(index, 0x1)
    local passdrop
    if padr == 1 then
        passdrop = "drop"
    else
        passdrop = "pass"
    end
    return string.format(fmt, dev, portnum, tclass, passdrop, packets, bytes)
end

local function ingress_processing_print(fmt, index, dev, packets, bytes)
    return string.format(fmt, dev, index, packets, bytes)
end

local function egress_processing_print(fmt, index, dev, packets, bytes)
    local portnum = bit_and(bit_shr(index, 5), 0x1F)
    local ptype = bit_and(index, 0x1F)
    return string.format(fmt, dev, portnum, ptype, packets, bytes)
end


local function mcfc_cntr_print(fmt, dev, counter, packets)
    return string.format(fmt, dev, counter, packets)
end

function mcfcCounterGet(dev, index)
    local ret
    local result

    ret, result = myGenWrapper("cpssPxCfgCntrGet",{
        {"IN",  "GT_SW_DEV_NUM",                 "devNum",   dev},
        {"IN",  "CPSS_PX_CFG_CNTR_ENT",          "counterType",  index},
        {"OUT", "GT_U32",                        "cntPtr"}
    })
    if ret~=0 then
        print("cpssPxCfgCntrGet() failed: "..returnCodes[ret])
        return false,"Error, counter could not be read"
    end

    return result.cntPtr
end

local function show_mcfc_counters(params)
    local table_header 
    local table_format
    local output = ""
    local devNum = params["devID"]
    local cntrType
    local counters = {}
    local devices
    local idx = 1

    table_header = mcfc_cntr_head
    table_format = mcfc_cntr_fmt
    print_func   = mcfc_cntr_print

    if (devNum=="all") then 
        devices=wrLogWrapper("wrlDevList") 
    else 
        devices={devNum} 
    end
    if params["cntrType"] then
        counters[idx] = params["cntrType"]
    else
        counters = mcfc_counter_type_enum
    end

    for dev=1,#devices do
        for ii=1,#counters do
            packets = mcfcCounterGet(devices[dev], counters[ii])
            if packets == false then
                return packets
            end
            output = output..print_func(table_format, ii-1, devices[dev], packets)
        end
    end

    local lines = splitString(output, "\n")
    for ii=1,#lines do
        print(lines[ii])
    end
end

local function show_counters(params)
    local table_header
    local table_format
    local index_func
    local devNum = params["devID"]
    local devices
    local dev
    local output = ""
    local indexes
    local ii
    local mode
    local gmode
    local clearEnabled
    local activeDevices
    local counterEngine

    activeDevices = globalGet("cncActiveDevices")
    if activeDevices == nil then
        activeDevices = {}
    end

    if params.counter_type == "eq_tdrop" then
        table_header = eq_tdrop_head
        table_format = eq_tdrop_fmt
        index_func   = eq_tdrop_indexes
        print_func   = eq_tdrop_print
        mode         = 0
        counterEngine = "egressQueue"
    elseif params.counter_type == "eq_cn" then
        table_header = eq_cn_head
        table_format = eq_cn_fmt
        index_func   = eq_cn_indexes
        print_func   = eq_cn_print
        mode         = 1
        counterEngine = "egressQueue"
    elseif params.counter_type == "eq_reduced" then
        table_header = eq_reduced_head
        table_format = eq_reduced_fmt
        index_func   = eq_reduced_indexes
        print_func   = eq_reduced_print
        mode         = 2
        counterEngine = "egressQueue"
    elseif params.counter_type == "ingress_processing" then
        table_header = ingress_head
        table_format = ingress_fmt
        index_func   = ingress_indexes
        print_func   = ingress_processing_print
        counterEngine = "ingressProcessing"
    elseif params.counter_type == "egress_processing" then
        table_header = egress_head
        table_format = egress_fmt
        index_func   = egress_indexes
        print_func   = egress_processing_print
        counterEngine = "egressProcessing"
    end

    output = table_header
    indexes = index_func(params)
    if (devNum=="all") then devices=wrLogWrapper("wrlDevList") else devices={devNum} end

    for dev=1,#devices do
        if activeDevices[devices[dev]] ~= counterEngine then
            print("The client is not enabled for the device")
            return
        end

        if mode ~= nil then
            gmode = globalGet("cncEgressQueueCnMode")
            if mode ~= gmode[devices[dev]] then
                print("You have selected wrong egress queue client mode")
                return
            end
        end
    end

    for dev=1,#devices do
        ret, clearEnabled = cncCounterClearByReadEnableGet(devices[dev], 0)
        if ret == false then
            return false, clearEnabled
        end

        ret, val = cncCounterClearByReadEnableSet(devices[dev], 0, false)
        if ret == false then
            return false, val
        end
        for ii=1,#indexes do
            packets, bytes = counterGet(devices[dev], ii-1)
            if packets == false then
                return packets, bytes
            end
            if indexes[ii] == true then
                output = output..print_func(table_format, ii-1, devices[dev], packets, bytes)
            end
        end
        ret, val = cncCounterClearByReadEnableSet(devices[dev], 0, clearEnabled)
        if ret == false then
            return false, val
        end
    end

    local lines = splitString(output, "\n")
    for ii=1,#lines do
        print(lines[ii])
    end
end

CLI_type_dict["traffic_class"] = {
    checker = function(s, name, desc)
        local value = tonumber(s)
        if value then
            if value > desc["max"] then
                return false, "Out of range"
            end

            if value < desc["min"] then
                return false, "Out of range"
            end
            return true, value
        else
            return false, "Not a number"
        end
    end,
	max = 7, -- Maximal class
	min = 0,
    help = "Traffic class"
}


local function check_port_range_wo_check(param)
    local err = "Index is out of designated port table range";
    for num in string.gmatch(param, "%d+") do
        if(tonumber(num) > 16) then --16 is maximal port number
            return false, err
        end
    end
    return check_param_number_range(param)
end

CLI_type_dict["port_range_wo_check"] = {
    checker = check_port_range_wo_check,
    help = "The range of port numbers. Example: 2-4,6 means 2,3,4,6 (0..16)"
}

CLI_type_dict["drop_precedence"] = {
    checker = function(s, name, desc)
        local value = tonumber(s)
        if value then
            if value > desc["max"] then
                return false, "Out of range"
            end

            if value < desc["min"] then
                return false, "Out of range"
            end
            return true, value
        else
            return false, "Not a number"
        end
    end,
	max = 2, -- Maximal precedence
	min = 0,
    help = "Drop precedence"
}

CLI_type_dict["px_packet_type"] = {
    checker = function(s, name, desc)
        local value = tonumber(s)
        if value then
            if value > desc["max"] then
                return false, "Out of range"
            end

            if value < desc["min"] then
                return false, "Out of range"
            end
            return true, value
        else
            return false, "Not a number"
        end
    end,
	max = 31, -- Maximal precedence
	min = 0,
    help = "Packet type"
}

local function check_dst_index_range(param)
    local err = "Index is out of maximal counter index";
    for num in string.gmatch(param, "%d+") do
        if(tonumber(num) > 1023) then --1023 is maximal index in designated port table
            return false, err
        end
    end
    return check_param_number_range(param)
end
CLI_type_dict["dst_index_range"] = {
    checker = check_dst_index_range,
    help = "The range of numbers. Example: 2-4,6 means 2,3,4,6 (0..1023)"
}

mcfc_counter_type_help = "MCFC Counter Type"
local mcfc_counter_type_enum = {
    ["CPSS_PX_CFG_CNTR_RECEIVED_PFC_E"]         = { value =  0, help = "Received priority flow control packets, whether they are dropped or not" },
    ["CPSS_PX_CFG_CNTR_RECEIVED_MC_E"]          = { value =  1, help = "Received MC packets before replication which are not QCN" },
    ["CPSS_PX_CFG_CNTR_RECEIVED_UC_E"]          = { value =  2, help = "Received UC packets which are not QCN." },
    ["CPSS_PX_CFG_CNTR_RECEIVED_QCN_E"]         = { value =  3, help = "QCN received and QCN triggered packets which are not MC" },
    ["CPSS_PX_CFG_CNTR_RECEIVED_MC_QCN_E"]      = { value =  4, help = "Multicast QCN received packets and QCN triggered packets (before replication)" },
    ["CPSS_PX_CFG_CNTR_INGRESS_DROP_E"]         = { value =  5, help = "Received packets which arrived with an empty port map" },
    ["CPSS_PX_CFG_CNTR_OUT_MC_PACKETS_E"]       = { value =  6, help = "Outgoing MC replications which are not QCN" },
    ["CPSS_PX_CFG_CNTR_OUT_QCN_TO_PFC_MSG_E"]   = { value =  7, help = "Outgoing QCN2 Priority Flow Control messages due to response or Trigger QCN packets" },
    ["CPSS_PX_CFG_CNTR_OUT_QCN_PACKETS_E"]      = { value =  8, help = "Outgoing QCN Packets which are not multicast (both QCN generated in the local device and QCN which were received on the local device)" },
    ["CPSS_PX_CFG_CNTR_OUT_PFC_PACKETS_E"]      = { value =  9, help = "Outgoing PFC messages" },
    ["CPSS_PX_CFG_CNTR_OUT_UC_PACKETS_E"]       = { value = 10, help = "Outgoing UC packets which are not QCN" },
    ["CPSS_PX_CFG_CNTR_OUT_MC_QCN_PACKETS_E"]   = { value = 11, help = "Outgoing MC QCN packets (both QCN generated in the local device and QCN which were received on the device)" }
}
CLI_addParamDictAndType_enum("mcfc_counter_type_enum","Counter type", mcfc_counter_type_help, mcfc_counter_type_enum)


CLI_addHelp("exec", "show counters", "Show CNC counters")
CLI_addHelp("exec", "show counters egress-queue", "Show egress queue / CN counters")

--------------------------------------------
-- show counters egress-queue tail-drop
--------------------------------------------
help_show_counter_egress_queue_tdrop = "Tail drop mode"
CLI_addHelp("exec", "show counters egress-queue tail-drop", help_show_counter_egress_queue_tdrop)
CLI_addCommand("exec", "show counters egress-queue tail-drop", {
  func=function(params)
    params.counter_type = "eq_tdrop"
    do_command_with_print_nice("show_counters", show_counters, params)
  end,
  help=help_show_counter_egress_queue_tdrop,
   params={
    { type="named",
      { format="device %devID_all",         name="devID"   ,help="The device number"},
      { format="port %port_range_wo_check", name="ports"   ,help="TxQ ports"},
      { format="tc %traffic_class",         name="tc"      ,help="Traffic class"},
      { format="dp %drop_precedence",       name="dp"      ,help="Drop precedence"},
        mandatory = {"devID"}
    }
  }
})

--------------------------------------------
-- show counters egress-queue cn
--------------------------------------------
help_show_counter_egress_queue_cn = "CN/non-CN mode"
CLI_addHelp("exec", "show counters egress-queue cn", help_show_counter_egress_queue_cn)
CLI_addCommand("exec", "show counters egress-queue cn", {
  func=function(params)
    params.counter_type = "eq_cn"
    do_command_with_print_nice("show_counters", show_counters, params)
  end,
  help=help_show_counter_egress_queue_cn,
   params={
    { type="named",
      { format="device %devID_all",         name="devID"   ,help="The device number"},
      { format="port %port_range_wo_check", name="ports"   ,help="TxQ ports"},
      { format="tc %traffic_class",         name="tc"      ,help="Traffic class"},
      { format="pass",                      name="pass"    ,help="Pass"},
      { format="drop",                      name="drop"    ,help="Drop"},
      { format="cn",                        name="cn"      ,help="CN"},
      { format="non-cn",                    name="ncn"     ,help="non-CN"},
        mandatory = {"devID"}
    }
  }
})

--------------------------------------------
-- show counters egress-queue reduced
--------------------------------------------
help_show_counter_egress_queue_reduced = "pass/drop reduced mode"
CLI_addHelp("exec", "show counters egress-queue reduced", help_show_counter_egress_queue_reduced)
CLI_addCommand("exec", "show counters egress-queue reduced", {
  func=function(params)
    params.counter_type = "eq_reduced"
    do_command_with_print_nice("show_counters", show_counters, params)
  end,
  help=help_show_counter_egress_queue_reduced,
   params={
    { type="named",
      { format="device %devID_all",         name="devID"   ,help="The device number"},
      { format="port %port_range_wo_check", name="ports"   ,help="TxQ ports"},
      { format="tc %traffic_class",         name="tc"      ,help="Traffic class"},
      { format="pass",                      name="pass"    ,help="Pass"},
      { format="drop",                      name="drop"    ,help="Drop"},
        mandatory = {"devID"}
    }
  }
})

--------------------------------------------
-- show counters ingress-processing
--------------------------------------------
help_show_counter_ingress_processing = "Show egress processing counters"
CLI_addHelp("exec", "show counters ingress-processing", help_show_counter_ingress_processing)
CLI_addCommand("exec", "show counters ingress-processing", {
  func=function(params)
    params.counter_type = "ingress_processing"
    do_command_with_print_nice("show_counters", show_counters, params)
  end,
  help=help_show_counter_ingress_processing,
   params={
    { type="named",
      { format="device %devID_all",      name="devID"   ,help="The device number"},
      { format="index %dst_index_range", name="indices" ,help="Index range"},
        mandatory = {"devID"}
    }
  }
})

--------------------------------------------
-- show counters egress-processing
--------------------------------------------
help_show_counter_egress_processing = "Show egress processing counters"
CLI_addHelp("exec", "show counters egress-processing", help_show_counter_egress_processing)
CLI_addCommand("exec", "show counters egress-processing", {
  func=function(params)
    params.counter_type = "egress_processing"
    do_command_with_print_nice("show_counters", show_counters, params)
  end,
  help=help_show_counter_egress_processing,
   params={
    { type="named",
      { format="device %devID_all",           name="devID", help="The device number"},
      { format="port %port_range_wo_check",   name="ports", help="Target ports"},
      { format="packet-type %px_packet_type", name="ptype", help="Packet type"},
        mandatory = {"devID"}
    }
  }
})

--------------------------------------------
-- show counters mcfc
--------------------------------------------
help_show_counter_mcfc = "Show MCFC counters"
CLI_addHelp("exec", "show counters mcfc", help_show_counter_mcfc)
CLI_addCommand("exec", "show counters mcfc", {
  func=function(params)
    do_command_with_print_nice("show_counters", show_mcfc_counters, params)
  end,
  help=help_show_counter_mcfc,
   params={
    { type="named",
      { format="device %devID_all",            name="devID", help="The device number"},
      { format="counter-type %mcfc_counter_type_enum", name="cntrType", help="MCFC counter type"},
        mandatory = {"devID"}
    }
  }
})
