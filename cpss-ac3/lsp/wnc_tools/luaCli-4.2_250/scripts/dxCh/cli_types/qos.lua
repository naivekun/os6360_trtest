--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* qos.lua
--*
--* DESCRIPTION:
--*       the file defines types needed for the qos commands
--*
--* FILE REVISION NUMBER:
--*       $Revision: 3 $
--*
--********************************************************************************

cmdLuaCLI_registerCfunction("wrlPrvCpssDxChPortIsRemote")

-- get speed of port in Kbps value
function portSpeedKbpsGet(devNum , portNum)
    local portSpeed = 0

    local result, values = cpssPerPortParamGet(
            "cpssDxChPortSpeedGet",
            devNum, portNum,
            "speed", "CPSS_PORT_SPEED_ENT")
    if result==0 then
        local portSpeedStr
        portSpeedStr,portSpeed = speedStrGet(values.speed)
        if portSpeed == nil then
            portSpeed = 0
        end
    end

    -- 10 means '10K bps'
    portSpeed = portSpeed * 1000

    return portSpeed
end

-- convert pps (packets per second) to Kbps (Kilo bits per second)
-- note : packetSize must include 4 byte CRC
function convertPpsToKbps(pps,packetSize)
    if pps == nil then
        pringLog("Error : convertPpsToKbps : the pps is nil \n")
        pps = 0
    end

    local preamble = 20
    return (pps * (packetSize + preamble) * 8--[[bits]])/1000--[[k]]
end

-- support call from the config file : ${@@committedRateGet(3,80)}
-- return value in Kbps units
function committedRateGet(portIndex , rate_percent , packet_size , calledFromTest)
    local devNum  = devEnv.dev
    local portNum = devEnv.port[portIndex]
    local portSpeed = portSpeedKbpsGet(devNum,portNum)
    local actualL2BitsRate

    if packet_size == nil then
        packet_size = 64
    end

    if calledFromTest then
        -- when called from the test we need to get value of the L1+L2 (not only L2)
        actualL2BitsRate = portSpeed
    else
        local preamble = 20
        -- note : although the rate is 'bits'  and {packet_size , preamble} are 'BYTES'
        -- I do not add the '8' multiplier because the 'BYTEs' are on multiplier AND on divider
        actualL2BitsRate = (portSpeed / (packet_size + preamble)) * packet_size
    end

    --[[
    printLog("portNum",to_string(portNum))
    printLog("rate_percent",to_string(rate_percent))
    printLog("portSpeed",to_string(portSpeed))
    printLog("actualL2BitsRate",to_string(actualL2BitsRate))
    printLog("calledFromTest",to_string(calledFromTest))
    --]]

    --calculate rate_percent from portSpeed
    if isSimulationUsed() and calledFromTest then
        -- rate not depends on speed of port
        local pps = get_wirespeed_rate_on_simulation(devNum,portNum)
        local kbps = convertPpsToKbps(pps,packet_size)
        return kbps
    end

    return math.floor((rate_percent / 100) * actualL2BitsRate)
end

local function CLI_check_param_dscp_map(param, data)
    local s,e,ret,val

    s=1
    e=1
    ret={}
    param=param..","

    while (e<string.len(param)) do
        e= string.find(param,",",s)
        if e==nil then break end

        val=string.sub(param,s,e-1)
        s=e+1
        val=tonumber(val)
        if (val~=nil) and (val<=63) and (val>=0) then
            table.insert(ret,val)
        else
            return false, "The list must only contain numbers in the range 0-63"
        end
    end

    return true,ret
end


local function CLI_check_param_vpt_list(param, data)
    local s,e,ret,val

    s=1
    e=1
    ret={}
    param=param..","

    while (e<string.len(param)) do
        e= string.find(param,",",s)
        if e==nil then break end

        val=string.sub(param,s,e-1)
        s=e+1
        val=tonumber(val)
        if (val~=nil) and (val<=7) and (val>=0) then
            table.insert(ret,val)
        else
            return false, "The list must only contain numbers in the range 0-7"
        end
    end

    return true,ret
end

local function CLI_check_param_dropPrecedence(param,data)
    if tonumber(param)==0 or tonumber(param)==2 then
        return true,tonumber(param)
    else
        return false,"Drop precedence must be a value equal to 0 or 2"
    end
end

local function CLI_check_param_queueWeightOrSp(param,data)
    local s,e,ret,val,spFound

    spFound=false
    s=1
    e=1
    ret={}
    param=param..","

    while (e<string.len(param)) do
        e= string.find(param,",",s)
        if e==nil then break end

        val=string.sub(param,s,e-1)
        s=e+1

        if (val=="sp") then
            spFound=true
            table.insert(ret,val)
        else
            val=tonumber(val)
            if (val~=nil) and (val<=255) and (val>=0) then
                if spFound==false then table.insert(ret,val) else return false,"sp cannot appear before weight"  end
            else
                return false, "The list must only contain numbers in the range 0-255 or \"sp\""
            end
        end
    end

    if (#ret~=8) then return false,"The list must contain 8 values" end
    return true,ret
end

local function CLI_check_param_commitedBurst4KShape(param,data)
    local res

    res=tonumber(param)

    if res==nil then return false, "The value must be a number" end

    res= math.floor((res+2048)/4096)

    if res<=0 or tonumber(param)<4096 then return false, "The value is too low, minimum is 4096" end
    if tonumber(param)>4096*4096 then return false, "value is too high, max value is ".. (4096*4096) end
    return true,res

end


local function CLI_check_param_commitedRateKbpsShape(param,data)
    local minSpeed,err,devNum, portArr, portIdx, portNum
    local port_is_remote;

    if tonumber(param)<64 then return false,"The value must be equal to 64 or higher" end

    ports=getGlobal("ifRange")
    port_is_remote = false;
    for devNum, portArr in pairs(ports) do
        for portIdx, portNum in pairs(portArr) do
            if wrlPrvCpssDxChPortIsRemote(devNum, portNum) then
                port_is_remote = true;
            end
        end
    end

    if not port_is_remote then
        minSpeed=getInterfaceSpeed()

        if tonumber(param)>minSpeed then
            return false,"Rate is higher than port configuration ("..(minSpeed).."Kbps)"
        end
    end
    return true,(tonumber(param))
end

function getInterfaceSpeed()
    local ports,minSpeed,found
    
    minSpeed = 0xffffffff

    ports=getGlobal("ifRange")

    for devNum, portArr in pairs(ports) do
        for portIdx, portNum in pairs(portArr) do
            local portSpeedKbps = portSpeedKbpsGet(devNum , portNum)
            minSpeed = math.min(portSpeedKbps,minSpeed)
            found = true
        end
    end

    if (not found) then return 0 end
    
    return minSpeed
end



CLI_type_dict["commitedRateKbps"] = {
    checker = CLI_check_param_number,
    complete = CLI_complete_param_number,
    min=0,
    help = "Average traffic rate (Kbps)"
}


CLI_type_dict["commitedBurstByte"] = {
    checker = CLI_check_param_number,
    complete = CLI_complete_param_number,
    min=0,
    help = "The normal burst size (Bytes)"
}

CLI_type_dict["peakRateKbps"] = {
    checker = CLI_check_param_number,
    complete = CLI_complete_param_number,
    min=0,
    help = "Peak Information Rate (Kbps)"
}


CLI_type_dict["peakBurstByte"] = {
    checker = CLI_check_param_number,
    complete = CLI_complete_param_number,
    min=0,
    help = "Peak Burst Size (Bytes)"
}

CLI_type_dict["commitedRateKbpsShape"] = {
    checker = CLI_check_param_commitedRateKbpsShape,
    help = "Average traffic rate (Kbps)"
}


CLI_type_dict["commitedBurst4KShape"] = {
    checker = CLI_check_param_commitedBurst4KShape,
    help = "The burst size rounded to 4KB units (4096-16777216 bytes)"
}

CLI_type_dict["exceedAction"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Action performed when rate exceeded\n",
    enum = {
        ["drop"] = { value="drop", help="Drop packet" },
        ["policed-dscp-transmit"] = { value="transmit", help="Policed DSCP transmit" },
        ["policed-drop-precedence"] = { value="precedence", help="policed drop precedence" }
   }
}


CLI_type_dict["trustMode"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "QOS trust mode\n",
    enum = {
        ["vpt"] = { value=1, help="VPT trust mode" },
        ["dscp"] = { value=2, help="DSCP trust mode" },
        ["dscp-vpt"] = { value=3, help="DSCP-VPT trust mode" }
   }
}

CLI_type_dict["qosMapOptions"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "QOS map display options\n",
    enum = {
        ["buffers-threshold"] = { value="buffers-threshold", help="Display buffers-threshold" },
        ["vpt-queue"] = { value="vpt-queue", help="Display vpt-queue" },
        ["dscp-queue"] = { value="dscp-queue", help="Display dscp-queue" },
        ["dscp-dp"] = { value="dscp-dp", help="Display dscp-dp" },
        ["policed-dscp"] = { value="policed-dscp", help="Display policed-dscp" }
    }
}

CLI_type_dict["qosInterfaceOptions"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "QOS interface display options\n",
    enum = {
        ["buffers"] = { value="buffers", help="Display buffers" },
        ["trust"] = { value="trust", help="Display trust values" },
        ["shapers"] = { value="shapers", help="Display shapers" }
    }
}


CLI_type_dict["dscpMap"] = {
    checker = CLI_check_param_dscp_map,
    help = "A list of dscp values (0-63)"
}

CLI_type_dict["dscp"] = {
    checker = CLI_check_param_number,
    min=0,
    max=63,
    complete = CLI_complete_param_number,
    help="Differentiated Services Code Point (0-63)"
}

CLI_type_dict["qos_profile"] = {
    checker = CLI_check_param_number,
    min=0,
    max=255,
    complete = CLI_complete_param_number,
    help="QOS profile (0-255)"
}

CLI_type_dict["thresholdPercent"] = {
    checker = CLI_check_param_number,
    min=0,
    max=100,
    complete = CLI_complete_param_number,
    help="The threshold in percents (0-100)"
}

CLI_type_dict["vptList"] = {
    checker = CLI_check_param_vpt_list,
    help = "A list of vpt values (0-7)"
}


CLI_type_dict["vptQueue"] = {
    checker = CLI_check_param_number,
    complete = CLI_complete_param_number,
    min=1,
    max=8,
    help = "The queue number (1-8)"
}

CLI_type_dict["dropPrecedence"] =
{
    checker  = CLI_check_param_dropPrecedence,
    help = "The drop precedence, a value equal to 0 or 2"
}

CLI_type_dict["queueWeightOrSp"] = {
    checker = CLI_check_param_queueWeightOrSp,
    help = "A weighted round robin weight (0-255) or \"sp\" for strict-priority"
}

CLI_type_dict["dropPrecedenceEnum"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Drop Pecedence Enum",
    enum = {
        ["green"]  = { value="CPSS_DP_GREEN_E",  help="green" },
        ["yellow"] = { value="CPSS_DP_YELLOW_E", help="yellow" },
        ["red"]    = { value="CPSS_DP_RED_E",    help="red" }
    }
}

