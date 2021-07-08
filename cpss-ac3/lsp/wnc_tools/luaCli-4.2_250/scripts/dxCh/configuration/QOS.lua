--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* QOS.lua
--*
--* DESCRIPTION:
--*       QOS related commands
--*
--* FILE REVISION NUMBER:
--*       $Revision: 10 $
--*
--[[
---+++ CLI EXECUTION TRACE START +++---
show qos map dscp-queue
---+++ CLI EXECUTION TRACE END +++---
]]--
--********************************************************************************

--includes


--constants



-- ************************************************************************
--  qosMapPolicedDSCP
--
--  @description invalidates a policy rule
--
--  @param params - params["devID"]                - the device ID number
--                    params["from"]                - a list of dscp values to change
--                    params["to"]                - the new value
--                    params["flagNo"]            - returns values to default
--
--  @return  true on success, otherwise false and error message
--
-- ************************************************************************

local function qosMapPolicedDSCP(params)
    local i,ret,devices,j,val


    if (params["devID"]=="all") then devices=wrLogWrapper("wrlDevList") else devices={params["devID"]} end

    for j=1,#devices do
        for i=1,table.getn(params["from"]) do

            if params.flagNo==true then params["to"]=params["from"][i] end

            ret = myGenWrapper("cpssDxCh3PolicerQosRemarkingEntrySet",{   --Always ingress and port mode
                {"IN","GT_U8","devNum",devices[j]},
                {"IN",TYPE["ENUM"],"stage",0},   --stage 0 ingress
                {"IN","GT_U32","qosProfileIndex",params["from"][i]},
                {"IN","GT_U32","greenQosTableRemarkIndex",0},
                {"IN","GT_U32","yellowQosTableRemarkIndex",params["from"][i]},
                {"IN","GT_U32","redQosTableRemarkIndex",params["to"]}
            })

            if (ret~=0) then
                print("Could not set value".. params["from"][i].." to value"..params["to"])
                return false,"Could not set value".. params["from"][i].." to value"..params["to"]
            end

            ret,val = myGenWrapper("cpssDxChCosProfileEntryGet",{
                {"IN","GT_U8", "devNum",devices[j]},
                {"IN","GT_U32", "profileIndex", params["to"]},
                {"OUT","CPSS_DXCH_COS_PROFILE_STC", "cosPtr"}
            })

            if (ret==0) then
                val["cosPtr"]["dscp"] = params["to"]

                ret = myGenWrapper("cpssDxChCosProfileEntrySet",{
                    {"IN","GT_U8","devNum",devices[j]},
                    {"IN","GT_U32","profileIndex",params["to"]},
                    {"IN","CPSS_DXCH_COS_PROFILE_STC","cosPtr",val["cosPtr"]}
                })
            else
                return false,"Could not set profile number "..params["from"][i]
            end

            if (ret==0) then
                ret = myGenWrapper("cpssDxChCosDscpToProfileMapSet", {
                    {"IN","GT_U8","devNum",devices[j]},
                    {"IN","GT_U32","mappingTableIndex",0},
                    {"IN","GT_U8","dscp",params["from"][i]},
                    {"IN","GT_U32","profileIndex",params["from"][i]}
                })
            else
                print("Error while calling cpssDxChCosDscpToProfileMapSet")
                return false,"Error while calling cpssDxChCosDscpToProfileMapSet"
            end

        end
    end
    return true
end

--------------------------------------------
-- command registration: qos map policed-dscp
--------------------------------------------

CLI_addHelp("config", "qos", "QOS related commands")
CLI_addHelp("config", "qos map", "QOS map related commands")

CLI_addCommand("config", "qos map policed-dscp ", {
  func=qosMapPolicedDSCP,
  help="Maps the entered values to new values",
   params={
    { type="named",
      { format="device %devID_all", name="devID", help="The device number" },
      { format="dscp-list %dscpMap", name="from", help="List of values to map" },
      { format="to %dscp", name="to", help="The mapped values" },
          requirements={["from"] = {"devID"}, ["to"] = {"from"}},
        mandatory = {"to"}
    }
  }
})

--------------------------------------------
-- command registration: no qos map policed-dscp
--------------------------------------------

CLI_addHelp("config", "no qos", "No-commands for qos")
CLI_addHelp("config", "no qos map", "No-commands for qos map commands")

CLI_addCommand("config", "no qos map policed-dscp ", {
 func=function(params)
      params.flagNo=true
      return qosMapPolicedDSCP(params)
  end,
 help="returns list values to default",
   params={
    { type="named",
      { format="device %devID_all", name="devID", help="The device number" },
      { format="dscp-list %dscpMap", name="from", help="List of values to map" },
          requirements={from = {"devID"}},
        mandatory = {"from"}
    }
  }
})




-- ************************************************************************
--  qosMapDSCPQueue
--
--  @description maps dscp values to a new value
--
--  @param params - params["devID"]                - the device ID number
--                    params["from"]                - a list of dscp values to change
--                    params["to"]                - a list of the new values
--                    params["flagNo"]            - returns values to default
--
--  @return  true on success, otherwise false and error message
--
-- ************************************************************************
local function qosMapDSCPQueue(params)
    local i,ret,val,to,devices,j

    to={}

    for i=1,table.getn(params["from"]) do
        if (params.flagNo==true) then
            table.insert(to,math.floor(params["from"][i]/8) +1)
        else
            table.insert(to,params["to"])
        end
    end


    if (params["devID"]=="all") then devices=wrLogWrapper("wrlDevList") else devices={params["devID"]} end

    for j=1,#devices do

        for i=1,table.getn(params["from"]) do

            ret,val = myGenWrapper("cpssDxChCosProfileEntryGet",{
                {"IN","GT_U8","devNum",devices[j]},
                {"IN","GT_U32","profileIndex",params["from"][i]},
                {"OUT","CPSS_DXCH_COS_PROFILE_STC","cosPtr"}
            })

            if (ret==0) then
                val["cosPtr"]["trafficClass"]=to[i]-1

                ret,val = myGenWrapper("cpssDxChCosProfileEntrySet",{
                    {"IN","GT_U8","devNum",devices[j]},
                    {"IN","GT_U32","profileIndex",params["from"][i]},
                    {"IN","CPSS_DXCH_COS_PROFILE_STC","cosPtr",val["cosPtr"]}
                })
            else
                return false,"Could not set profile number "..params["from"][i]
            end


            ret  = cpssAPIcall("cpssDxChCosDscpToProfileMapSet",{
                devNum = devices[j],
                dscp = params["from"][i],
                profileIndex = params["from"][i]
            })


        end
    end


    return true
end
--------------------------------------------
-- command registration: qos map dscp-queue
--------------------------------------------


CLI_addCommand("config", "qos map dscp-queue", {
  func=qosMapDSCPQueue,
  help="Maps the entered dscp values to a specific queue",
   params={
    { type="named",
      { format="device %devID_all", name="devID", help="The device number" },
      { format="dscp-list %dscpMap", name="from", help="List of values to map" },
      { format="to %vptQueue", name="to", help="The queue number" },
          requirements={["from"] = {"devID"}, ["to"] = {"from"}},
        mandatory = {"to"}
    }
  }
})



--------------------------------------------
-- command registration: no qos map dscp-queue
--------------------------------------------

CLI_addCommand("config", "no qos map dscp-queue ", {
 func=function(params)
      params.flagNo=true
      return qosMapDSCPQueue(params)
  end,
 help="returns list values to default",
   params={
    { type="named",
      { format="device %devID_all", name="devID", help="The device number" },
      { format="dscp-list %dscpMap", name="from", help="List of values to map back to default" },
          requirements={["from"] = {"devID"}},
        mandatory = {"from"}
    }
  }
})


-- ************************************************************************
--  qosMapVptQueue
--
--  @description maps vpt to queue
--
--  @param params - params["devID"]                - the device ID number
--                    params["queueId"]            - the queue number
--                    params["vptlist"]            - a list of vpt values
--
--  @return  true on success, otherwise false and error message
--
-- ************************************************************************

local function qosMapVptQueue(params)
    local j,i,ret,val,devices,k

    if (params["devID"]=="all") then devices=wrLogWrapper("wrlDevList") else devices={params["devID"]} end

    for k=1,#devices do
        for i=1,table.getn(params["vptList"]) do
            for j=0,1 do  --cfi bit 0 and 1
                    ret = cpssAPIcall("cpssDxChCosUpCfiDeiToProfileMapSet",{
                        devNum = devices[k],
                        upProfileIndex = 0,
                        up = params["vptList"][i],
                        cfiDeiBit = j,
                        profileIndex = params["queueId"]-1+64
                    })
                    if (ret~=0) then return false,"Could not set vpt "..params["vptList"][i].." to queue-id "..params["queueId"] end
            end
        end

            ret,val = myGenWrapper("cpssDxChCosProfileEntryGet",{
                {"IN","GT_U8","devNum",devices[j]},
                {"IN","GT_U32","profileIndex",params["queueId"]-1+64},
                {"OUT","CPSS_DXCH_COS_PROFILE_STC","cosPtr"}
            })

            if (ret==0) then
                val["cosPtr"]["trafficClass"]=params["queueId"]-1

                ret = myGenWrapper("cpssDxChCosProfileEntrySet",{
                    {"IN","GT_U8","devNum",devices[j]},
                    {"IN","GT_U32","profileIndex",params["queueId"]-1+64},
                    {"IN","CPSS_DXCH_COS_PROFILE_STC","cosPtr",val["cosPtr"]}
                })
            else
                return false,"Could not set profile number "..(params["queueId"]-1+64)
            end

    end
    return true
end


-- ************************************************************************
--  noQosMapVptQueue
--
--  @description no command for vpt-queue mapping
--
--  @param params - params["devID"]                - the device ID number
--                    params["queueId"]            - the queue number
--
--  @return  true on success, otherwise false and error message
--
-- ************************************************************************

local function noQosMapVptQueue(params)
    local default,j,i,ret,val,devices,k

    default={3,1,2,4,5,6,7,8}

    if (params["devID"]=="all") then devices=wrLogWrapper("wrlDevList") else devices={params["devID"]} end

    for k=1,#devices do
        for i=1,8 do
            ret,val = cpssAPIcall("cpssDxChCosUpCfiDeiToProfileMapGet",{
                devNum = devices[k],
                upProfileIndex = 0,
                up = i-1,
                cfiDeiBit = 0
            })

            if (ret~=0) then return false,"Could not get profile index for device "..devices[k].." up "..(i-1) end
            if (params["queueId"]==nil) or (tonumber(val["profileIndexPtr"])== params["queueId"]-1+64) then
                for j=0,1 do  --cfi bit 0 and 1
                    ret = cpssAPIcall("cpssDxChCosUpCfiDeiToProfileMapSet",{
                        devNum = devices[k],
                        upProfileIndex = 0,
                        up = i-1,
                        cfiDeiBit = j,
                        profileIndex = default[i]-1+64
                    })
                    if (ret~=0) then return false,"Could not set vpt "..params["vptList"][i].." to queue-id "..(default[i]-1) end
                end

                ret,val = myGenWrapper("cpssDxChCosProfileEntryGet",{
                    {"IN","GT_U8","devNum",devices[k]},
                    {"IN","GT_U32","profileIndex",default[i]-1+64},
                    {"OUT","CPSS_DXCH_COS_PROFILE_STC","cosPtr"}
                })

                if (ret==0) then val["cosPtr"]["trafficClass"]=default[i]-1
                    ret = myGenWrapper("cpssDxChCosProfileEntrySet",{
                        {"IN","GT_U8","devNum",devices[k]},
                        {"IN","GT_U32","profileIndex",default[i]-1+64},
                        {"IN","CPSS_DXCH_COS_PROFILE_STC","cosPtr",val["cosPtr"]}
                    })
                else
                    return false,"Could not set profile number "..(default[i]-1+64)
                end
            end
        end
    end
    return true
end


--------------------------------------------
-- command registration: qos map vpt-queue
--------------------------------------------


CLI_addCommand("config", "qos map vpt-queue", {
  func=qosMapVptQueue,
  help="Maps the entered vpt values to a specific egress queue",
   params={
    { type="named",
      { format="device %devID_all", name="devID", help="The device number" },
      { format="queue-id %vptQueue", name="queueId", help="Queue number (1-8)" },
      { format="vpt %vptList", name="vptList", help="The list of vpt values" },
          requirements={["vptList"] = {"queueId"}, ["queueId"] = {"devID"}},
        mandatory = {"vptList"}
    }
  }
})



--------------------------------------------
-- command registration: no qos map vpt-queue
--------------------------------------------

CLI_addCommand("config", "no qos map vpt-queue ", {
 func=noQosMapVptQueue,
 help="returns list of vpt values to default",
   params={
     { type="named",
      { format="device %devID_all", name="devID", help="The device number" },
      { format="queue-id %vptQueue", name="queueId", help="Queue number (1-8)" },
          requirements={["queueId"] = {"devID"}},
        mandatory = {"devID"}
    }
  }
})




-- ************************************************************************
--  mapDscpDp
--
--  @description maps dscp list to drop precedence
--
--  @param params - params["devID"]                - the device ID number
--                    params["dscpList"]            - the policy id number
--                    params["dropPrecedence"]    - the drop precedence
--
--  @return  true on success, otherwise false and error message
--
-- ************************************************************************

local function mapDscpDp(params)
    local i,ret,val,devices,j


    if (params.flagNo==true) then
        params["dropPrecedence"]=0

        if params["dscpList"]==nil then
            params["dscpList"]={}
            for i=0,63 do table.insert(params["dscpList"],i) end
        end
    end

       if (params["devID"]=="all") then devices=wrLogWrapper("wrlDevList") else devices={params["devID"]} end

    for j=1,#devices do
        for i=1,table.getn(params["dscpList"]) do

            ret,val = myGenWrapper("cpssDxChCosProfileEntryGet",{
                {"IN","GT_U8","devNum",devices[j]},
                {"IN","GT_U32","profileIndex",params["dscpList"][i]},
                {"OUT","CPSS_DXCH_COS_PROFILE_STC","cosPtr"}
            })

            if (ret==0) then
                val["cosPtr"]["dropPrecedence"]=params["dropPrecedence"]

                ret = myGenWrapper("cpssDxChCosProfileEntrySet",{
                    {"IN","GT_U8","devNum",devices[j]},
                    {"IN","GT_U32","profileIndex",params["dscpList"][i]},
                    {"IN","CPSS_DXCH_COS_PROFILE_STC","cosPtr",val["cosPtr"]}
                })
            else
                return false,"Could not set profile number "..params["dscpList"][i]
            end
        end
    end
end

--------------------------------------------
-- command registration: qos map dscp-dp
--------------------------------------------

CLI_addCommand("config", "qos map dscp-dp", {
  func=mapDscpDp,
  help="Shows a policy rule list",
   params={
     { type="named",
      { format="device %devID_all", name="devID", help="The device number" },
      { format="dscp-list %dscpMap", name="dscpList", help="A list of dscp values" },
      { format="to %dropPrecedence", name="dropPrecedence", help="The drop precedence, a value equal to 0 or 2" },
      requirements={["dscpList"] = {"devID"},["dropPrecedence"] = {"dscpList"}},
      mandatory = {"dropPrecedence"}
    }
  }
})

--------------------------------------------
-- command registration: no qos map dscp-dp
--------------------------------------------


CLI_addCommand("config", "no qos map dscp-dp", {
 func=function(params)
      params.flagNo=true
      return mapDscpDp(params)
  end,
  help="Shows a policy rule list",
   params={
     { type="named",
      { format="device %devID_all", name="devID", help="The device number" },
      { format="dscp-list %dscpMap", name="dscpList", help="A list of dscp values" },
      requirements={["dscpList"] = {"devID"}},
      mandatory = {"devID"}
    }
  }
})


-- ************************************************************************
--  setQosTrust
--
--  @description sets qos trust mode (vpt,dscp or vpt-dscp)
--
--  @param params - params["trustMode"]                - the device ID number
--
--  @return  execution results from command_data
--
-- ************************************************************************

local function setQosTrust(params)
    local ret,iterator, devNum, portNum
    local command_data = Command_Data()

    if (params["flagNo"]==true) then params["trustMode"]=3 end

    if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in command_data:getInterfacePortIterator() do
            command_data:clearPortStatus()
            command_data:clearLocalStatus()

            ret = myGenWrapper("cpssDxChCosPortQosTrustModeSet",{   --Always ingress and port mode
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_U32","portNum",portNum},
                {"IN",TYPE["ENUM"],"portQosTrustMode",params["trustMode"]}
            })


            if ret~=0 then
                command_data:setFailPortAndLocalStatus()
                command_data:addError("Error set trust mode at device %d port %d: ", devNum, portNum)
            end

            command_data:updateStatus()
            command_data:updatePorts()
        end

        command_data:addWarningIfNoSuccessPorts("Can not set trust mode for all processed ports.")
    end

    -- Command data postprocessing
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()

   return command_data:getCommandExecutionResults()

end

--------------------------------------------
-- command registration: qos trust
--------------------------------------------

CLI_addCommand({"interface", "interface_eport"}, "qos trust", {
 func=setQosTrust,
  help="Set an interface trust mode",
   params={
       {type="values", "%trustMode"},
      mandatory = {"trustMode"}
    }
})

--------------------------------------------
-- command registration: no qos trust
--------------------------------------------

CLI_addHelp({"interface", "interface_eport"}, "no qos", "No-commands for qos")
CLI_addCommand({"interface", "interface_eport"}, "no qos trust", {
 func=function(params)
      params.flagNo=true
      return setQosTrust(params)
  end,
  help="Set an interface trust mode to default (VPT-DSCP)",

})


-- ************************************************************************
--  qosTrafficShape
--
--  @description sets qos traffic shaper or disables it
--
--  @param params - params["commitedRate"]        - The commited rate in Kbps
--                    params["commitedBurst"]        - Thecommited burst size in 4Kbyte units
--
--  @return  execution results from command_data
--
-- ************************************************************************


local function qosTrafficShape(params)
    local ret,val,iterator, devNum, portNum,enable
    local command_data = Command_Data()

    if (params["flagNo"]==true) then enable=false params["commitedBurst"]=0 params["commitedRate"]=0 else enable=true end

    if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in command_data:getInterfacePortIterator() do
            command_data:clearPortStatus()
            command_data:clearLocalStatus()

            ret, val = myGenWrapper("cpssDxChPortTxShaperProfileSet",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_U32","portNum",portNum},
                {"IN","GT_U16","burstSize",params["commitedBurst"]},
                {"INOUT","GT_U32","maxRatePtr",params["commitedRate"]}
            })

            if ret~=0 then
                command_data:setFailPortAndLocalStatus()
                command_data:addError("Error setting shaper profile at device %d port %d: ", devNum, portNum)
            end

            ret = myGenWrapper("cpssDxChPortTxShaperEnableSet",{
                    {"IN","GT_U8","devNum",devNum},
                    {"IN","GT_U32","portNum",portNum},
                    {"IN","GT_BOOL","enable",enable}
            })


            if ret~=0 then
                command_data:setFailPortAndLocalStatus()
                command_data:addError("Error enabling/disabling shaper at device %d port %d: ", devNum, portNum)
            end

            command_data:updateStatus()
            command_data:updatePorts()
        end

        command_data:addWarningIfNoSuccessPorts("Can not set shaper for all processed ports.")
    end

    -- Command data postprocessing
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults()
end


--------------------------------------------
-- command registration: qos traffic shape
--------------------------------------------
CLI_addHelp("interface", "qos traffic", "Traffic commands for qos")
CLI_addCommand("interface", "qos traffic shape", {
 func=qosTrafficShape,
  help="Set an interface shaper",
   params={
     { type="named",
      { format="commited-rate %commitedRateKbpsShape", name="commitedRate" , help="The commited rate in Kbps (64Kbps-Max port speed)" },
      { format="commited-burst %commitedBurst4KShape", name="commitedBurst" },
        requirements={["commitedBurst"] = {"commitedRate"}},
         mandatory = {"commitedBurst"}
    }
    }
})


--------------------------------------------
-- command registration: no qos traffic shape
--------------------------------------------

CLI_addHelp("interface", "no qos traffic", "No-commands for qos traffic")
CLI_addCommand("interface", "no qos traffic shape", {
 func=function(params)
      params.flagNo=true
      return qosTrafficShape(params)
  end,
  help="Disable the shaper on an interface",

})


-- ************************************************************************
--  qosTrafficQueueShape
--
--  @description sets qos traffic shaper or disables it
--
--  @param params - params["commitedRate"]        - The commited rate in Kbps
--                    params["commitedBurst"]        - Thecommited burst size in 4Kbyte units
--                    params["vptQueue"]            - The queue number
--  @return  execution results from command_data
--
-- ************************************************************************


local function qosTrafficQueueShape(params)
    local ret,val,iterator, devNum, portNum,enable,i,startFor,endFor
    local command_data = Command_Data()

    if (params["flagNo"]==true) then
        enable=false
        startFor=0
        endFor=7
    else
        startFor=(params["vptQueue"]-1)
        endFor=startFor
        enable=true
    end

    if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in command_data:getInterfacePortIterator() do
            command_data:clearPortStatus()
            command_data:clearLocalStatus()

            if (enable==true) then
                ret, val = myGenWrapper("cpssDxChPortTxQShaperProfileSet",{
                    {"IN","GT_U8","devNum",devNum},
                    {"IN","GT_U32","portNum",portNum},
                    {"IN","GT_U8","tcQueue",(params["vptQueue"]-1)},
                    {"IN","GT_U16","burstSize",params["commitedBurst"]},
                    {"INOUT","GT_U32","maxRatePtr",params["commitedRate"]}
                })

                if ret~=0 then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error setting queue shaper profile at device %d port %d: ", devNum, portNum)
                end
            end

            for i=startFor,endFor do
                ret = myGenWrapper("cpssDxChPortTxQShaperEnableSet",{
                        {"IN","GT_U8","devNum",devNum},
                        {"IN","GT_U32","portNum",portNum},
                        {"IN","GT_U8","tcQueue",i},
                        {"IN","GT_BOOL","enable",enable}
                })

                if ret~=0 then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error enabling/disabling queue shaper at device %d port %d queue: ", devNum, portNum,i+1)
                end
            end



            command_data:updateStatus()
            command_data:updatePorts()
        end

        command_data:addWarningIfNoSuccessPorts("Can not set shaper for all processed ports.")
    end

    -- Command data postprocessing
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults()
end


--------------------------------------------
-- command registration: qos traffic shape queue
--------------------------------------------
CLI_addCommand("interface", "qos traffic shape queue", {
 func=qosTrafficQueueShape,
  help="Set a queue traffic shaper for an interface",
   params={
   {type="values", "%vptQueue"},
     { type="named",
      { format="commited-rate %commitedRateKbpsShape", name="commitedRate", help="The commited rate in Kbps (64Kbps-Max port speed)" },
      { format="commited-burst %commitedBurst4KShape", name="commitedBurst" },
        requirements={["commitedRate"] = {"vptQueue"},["commitedBurst"] = {"commitedRate"}},
         mandatory = {"commitedBurst"}
    }
    }
})


--------------------------------------------
-- command registration: no qos traffic shape queue
--------------------------------------------

CLI_addCommand("interface", "no qos traffic shape queue", {
 func=function(params)
      params.flagNo=true
      return qosTrafficQueueShape(params)
  end,
  help="Disable the queue traffic shaper on an interface",

})





-- ************************************************************************
--  qosWrrQueueBandwidth
--
--  @description sets qos traffic shaper or disables it
--
--  @param params - params["devID"]        - The device number
--                    params["weight"]    - The weight of all the queues in a list
--  @return  execution results from command_data
--
-- ************************************************************************


local function qosWrrQueueBandwidth(params)
    local ret,devices,j,i,k,arbGroup,portList,index,portNum

    if (params["flagNo"]==true) then params["weight"]={1,1,1,1,1,1,1,1} end

    if (params["devID"]=="all") then devices=wrLogWrapper("wrlDevList") else devices={params["devID"]} end


    for j=1,#devices do
        portList=get_port_list(devices[j])

        for index,portNum in pairs(portList) do
            ret = myGenWrapper("cpssDxChPortTxBindPortToSchedulerProfileSet",{
                {"IN","GT_U8","devNum",devices[j]},
                {"IN","GT_U32","portNum",portNum},
                {"IN",TYPE["ENUM"],"profileSet",1}  --profile 2
            })
        end

        for i=1,8 do
            if params["weight"][i]=="sp" then arbGroup=2 else arbGroup=0 end

            ret = myGenWrapper("cpssDxChPortTxQArbGroupSet",{
                {"IN","GT_U8","devNum",devices[j]},
                {"IN","GT_U8","tcQueue",i-1},
                {"IN",TYPE["ENUM"],"arbGroup",arbGroup},
                {"IN",TYPE["ENUM"],"profileSet",1}    --profile 2
            })

            if (ret~=0) then return false,"Could not set device ".. devices[j] .. " queue number "..i.."'s mode" end

            if arbGroup~=2 then
                ret = myGenWrapper("cpssDxChPortTxQWrrProfileSet",{
                    {"IN","GT_U8","devNum",devices[j]},
                    {"IN","GT_U8","tcQueue",i-1},
                    {"IN","GT_U8","wrrWeight",params["weight"][i]},
                    {"IN",TYPE["ENUM"],"profileSet",1}    --profile 2
                })
            end

            if (ret~=0) then return false,"Could not set device ".. devices[j] .. " queue number "..i.."'s weight" end
        end
    end

    return true

end


--------------------------------------------
-- command registration: qos wrr-queue bandwidth
--------------------------------------------
CLI_addHelp("config", "qos wrr-queue", "QOS weighted round robin related commands")
CLI_addCommand("config", "qos wrr-queue bandwidth", {
 func=qosWrrQueueBandwidth,
  help="Set a queue traffic shaper for an interface",
   params={
     { type="named",
      { format="device %devID_all", name="devID", help="The device number" },
     { format="weight %queueWeightOrSp", name="weight"},
        requirements={["weight"] = {"devID"}},
         mandatory = {"weight"}
    }
    }
})


--------------------------------------------
-- command registration: no qos wrr-queue bandwidth
--------------------------------------------
CLI_addHelp("config", "no qos wrr-queue", "No-commands for QOS weighted round robin")
CLI_addCommand("config", "no qos wrr-queue bandwidth", {
 func=function(params)
      params.flagNo=true
      return qosWrrQueueBandwidth(params)
  end,
  help="Disable the queue traffic shaper on an interface",
  params={
      { type="named",
       { format="device %devID_all", name="devID", help="The device number" },
        mandatory = { "devID" }}
    }
})


-- ************************************************************************
--  qosWrrQueueThreshold
--
--  @description sets qos queue threshold
--
--  @param params - params["queueId"]        - Queue number
--                    params["threshold"]        - Threshold of dropping dp2 packets
--                    params["flagNo"]        - Flags the no command
--  @return  execution results from command_data
--
-- ************************************************************************

local function qosWrrQueueThreshold(params)
    local ret,val1,val2,devices,i,tailDropProfile,k

    if params.flagNo==true then params["threshold"]=80 end

    devices=wrLogWrapper("wrlDevList")

    if nil == params["thresholdPercent"] then
        params["thresholdPercent"]  = 80
    end

    for i=1,#devices do
        for k=0,3 do
            ret,val1 = myGenWrapper("cpssDxChPortTxTailDropProfileGet",{
                {"IN","GT_U8","devNum",devices[i]},
                {"IN","GT_U32","profileSet",k},
                {"OUT","CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT","portAlphaPtr"},
                {"OUT","GT_U32","portMaxBuffLimitPtr"},
                {"OUT","GT_U32","portMaxDescrLimitPtr"}
            })

            if (ret~=0) then return false,"Could not get tail drop profile for device ".. devices[i] end

            ret,val2 = myGenWrapper("cpssDxChPortTx4TcTailDropProfileGet",{
                {"IN","GT_U8","devNum",devices[i]},
                {"IN","GT_U32","profileSet",k},
                {"IN","GT_U8","trafficClass",(params["vptQueue"]-1)},
                {"OUT","CPSS_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS","tailDropProfileParamsPtr"}
            })

            if (ret~=0) then return false,"Could not get tail drop profile for device ".. devices[i].. " queue "..(params["vptQueue"]-1) end

            tailDropProfile=val2["tailDropProfileParamsPtr"]
            tailDropProfile["dp0MaxBuffNum"]=val1["portMaxBuffLimitPtr"]
            tailDropProfile["dp0MaxDescrNum"]=val1["portMaxDescrLimitPtr"]
            tailDropProfile["dp2MaxBuffNum"]=math.floor((val1["portMaxBuffLimitPtr"]*params["thresholdPercent"])/100)
            tailDropProfile["dp2MaxDescrNum"]=math.floor((val1["portMaxDescrLimitPtr"]*params["thresholdPercent"])/100)

            ret = myGenWrapper("cpssDxChPortTx4TcTailDropProfileSet",{
                {"IN","GT_U8","devNum",devices[i]},
                {"IN","GT_U32","profileSet",k},
                {"IN","GT_U8","trafficClass",(params["vptQueue"]-1)},
                {"IN","CPSS_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS","tailDropProfileParamsPtr",tailDropProfile}
            })
        end
    end
end


--------------------------------------------
-- command registration: qos wrr-queue threshold
--------------------------------------------

CLI_addCommand("config", "qos wrr-queue threshold", {
 func=qosWrrQueueThreshold,
  help="Set a queue traffic shaper for an interface",
   params={
     { type="values", "%vptQueue", "%thresholdPercent"}
  }
})

--------------------------------------------
-- command registration: no qos wrr-queue threshold
--------------------------------------------

CLI_addCommand("config", "no qos wrr-queue threshold", {
 func=function(params)
      params.flagNo=true
      return qosWrrQueueThreshold(params)
  end,
  help="Set a queue traffic shaper for an interface",
   params={
     { type="values", "%vptQueue" }
  }
})




-- ************************************************************************
--  qosMapShow
--
--  @description shows qos map parameters
--
--  @param params - params["qosMapOptions"]        - The option to display
--
--  @return  true on success, otherwise false and error message
--
-- ************************************************************************


local function qosMapShow(params)
    local ret,val,i,j,devices,res,k,val1,val2

    devices=wrLogWrapper("wrlDevList")

    for j=1,#devices do
        print("\nDevice:"..devices[j])

        if params["qosMapOptions"]=="buffers-threshold" then
            print("QID   Threshold   Weight   Priority")
            for i=0,7 do
                local threshold,weight,priority

                ret,val1 = myGenWrapper("cpssDxChPortTxTailDropProfileGet",{
                    {"IN","GT_U8","devNum",devices[i]},
                    {"IN","GT_U32","profileSet",0},
                    {"OUT","CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT","portAlphaPtr"},
                    {"OUT","GT_U32","portMaxBuffLimitPtr"},
                    {"OUT","GT_U32","portMaxDescrLimitPtr"}
                })

                if (ret~=0) then return false,"Could not get tail drop profile for device ".. devices[i] end


                ret,val2 = myGenWrapper("cpssDxChPortTx4TcTailDropProfileGet",{
                    {"IN","GT_U8","devNum",devices[i]},
                    {"IN","GT_U32","profileSet",0},
                    {"IN","GT_U8","trafficClass",i},
                    {"OUT","CPSS_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS","tailDropProfileParamsPtr"}
                })

                if (ret~=0) then return false,"Could not get tail drop profile for device ".. devices[i].. " queue "..(params["queueId"]-1) end

                threshold=math.floor((val2["tailDropProfileParamsPtr"]["dp2MaxBuffNum"]*100)/val1["portMaxBuffLimitPtr"])


                ret,val = myGenWrapper("cpssDxChPortTxQArbGroupGet",{
                    {"IN","GT_U8","devNum",devices[j]},
                    {"IN","GT_U8","tcQueue",i},
                    {"IN",TYPE["ENUM"],"profileSet",1},   --using only profile 2
                    {"OUT",TYPE["ENUM"],"arbGroupPtr"}
                })

                if (ret~=0) then return false,"Could not get device ".. devices[j] .. " queue number "..(i+1).."'s arbitrary group" end

                if val["arbGroupPtr"]~=2 then
                    ret,val = myGenWrapper("cpssDxChPortTxQWrrProfileGet",{
                        {"IN","GT_U8","devNum",devices[j]},
                        {"IN","GT_U8","tcQueue",i},
                        {"IN",TYPE["ENUM"],"profileSet",1},  --using profile 2
                        {"OUT","GT_U8","wrrWeight"}
                    })

                    if (ret~=0) then return false,"Could not set device ".. devices[j] .. " queue number "..(i+1).."'s weight" end

                    weight=val["wrrWeight"]
                    priority=""
                else
                    weight="sp"
                    priority=i+1
                end

                print(string.format(" %d %10d %7s %10s",(i+1),threshold,weight,priority))
            end

        elseif params["qosMapOptions"]=="vpt-queue" then
            print("VPT\tQID")
            for i=0,7 do
                ret,val = cpssAPIcall("cpssDxChCosUpCfiDeiToProfileMapGet",{
                    devNum = devices[j],
                    upProfileIndex = 0,
                    up = i,
                    cfiDeiBit = 0
                })

                    if (ret~=0) then return false,"Could not get profile index for device "..devices[j].." up "..i end
                    if tonumber(val["profileIndexPtr"])-63>0 then print(i.."\t"..(tonumber(val["profileIndexPtr"])-64+1)) else print(i.."\tN/A") end
            end


        elseif params["qosMapOptions"]=="dscp-queue" then
            for i=0,7 do
                res=""
                for k=0,7 do
                    ret,val = myGenWrapper("cpssDxChCosProfileEntryGet",{   --Always ingress and port mode
                        {"IN","GT_U8","devNum",devices[j]},
                        {"IN","GT_U32","profileIndex",k*8+i},
                        {"OUT","CPSS_DXCH_COS_PROFILE_STC","cosPtr"}
                    })

                    if (ret~=0) then return false,"Could not get profile index for device "..devices[j].." dscp "..(k*8+i) end
                    res=res..(k*8+i)..":"..(val["cosPtr"]["trafficClass"]+1).."\t"
                end
                print(res)
            end

        elseif params["qosMapOptions"]=="dscp-dp" then
            for i=0,7 do
                res=""
                for k=0,7 do
                    ret,val = myGenWrapper("cpssDxChCosProfileEntryGet",{   --Always ingress and port mode
                        {"IN","GT_U8","devNum",devices[j]},
                        {"IN","GT_U32","profileIndex",k*8+i},
                        {"OUT","CPSS_DXCH_COS_PROFILE_STC","cosPtr"}
                    })

                    if val["cosPtr"]["dropPrecedence"]=="CPSS_DP_GREEN_E" then val=0 elseif val["cosPtr"]["dropPrecedence"]=="CPSS_DP_RED_E" then val=2 end
                    if (ret~=0) then return false,"Could not get profile index for device "..devices[j].." dscp "..(k*8+i) end
                    res=res..(k*8+i)..":"..val.."\t"
                end
                print(res)
            end


        elseif params["qosMapOptions"]=="policed-dscp" then
            for i=0,7 do
                res=""
                for k=0,7 do
                    ret,val = myGenWrapper("cpssDxCh3PolicerQosRemarkingEntryGet",{   --Always ingress and port mode
                        {"IN","GT_U8","devNum",devices[j]},
                        {"IN",TYPE["ENUM"],"stage",0},   --stage 0 ingress
                        {"IN","GT_U32","qosProfileIndex",(k*8+i)},
                        {"OUT","GT_U32","yellowQosTableRemarkIndex"},
                        {"OUT","GT_U32","redQosTableRemarkIndex"}
                    })

                    if (ret~=0) then return false,"Could not get dscp remarking entry for device "..devices[j].." dscp "..(k*8+i) end
                    res=res..(k*8+i)..":"..val["redQosTableRemarkIndex"].."\t"
                end
                print(res)
            end
        end
    end
end


--------------------------------------------
-- command registration: show qos map
--------------------------------------------
CLI_addHelp("exec", "show qos", "Show qos commands")
CLI_addCommand("exec", "show qos map", {
 func=qosMapShow,
  help="Show qos map parameters",
   params={
   {type="values", "%qosMapOptions"}

    }
})


-- ************************************************************************
--  qosInterfaceShow
--
--  @description shows qos map parameters
--
--  @param params - params["qosMapOptions"]        - The option to display
--
--  @return  true on success, otherwise false and error message
--
-- ************************************************************************


local function qosInterfaceShow(params)
    local ret,val,i,j,devices,res,k,devNum,portNum


    if params["interface"]=="ethernet" then
        devices=params["ethernet"]
    else
        ret,devices= get_trunk_device_port_list(params["port-channel"], wrLogWrapper("wrlDevList"))
        if (ret~=0) or devices==nil then print("Invalid port channel") return false,"Port channel null" end
    end


    for devNum,portNum in pairs(devices) do
        print("\nDevice:"..devNum)

        if params["qosInterfaceOptions"]=="buffers" then
            for i=1,#portNum do
                res=""
                for j=0,7 do

                    ret,val = myGenWrapper("cpssDxChPortTxQShaperProfileGet",{
                        {"IN","GT_U8","devNum",devNum},
                        {"IN","GT_U32","portNum",portNum[i]},
                        {"IN","GT_U8","tcQueue",j},
                        {"OUT","GT_U16","burstSizePtr"},
                        {"OUT","GT_U32","maxRatePtr"},
                        {"OUT","GT_BOOL","enablePtr"}
                    })

                    if ret~=0 then
                        return false, "Could not get shaper profile for device ".. devnum .." port " ..portNum[i]
                    else
                        if val["enablePtr"]==true then val["enablePtr"]="Enabled " else val["enablePtr"]="Disabled" end
                        res = res.. string.format("% 2d% 12s% 12d% 12d",j,val["enablePtr"],val["maxRatePtr"],(4096*val["burstSizePtr"])) .. "\n"
                    end
                end

                print("Port:"..portNum[i])
                print("QID\tStatus\tCommitted rate\tCommitted burst\n"..res)
            end

        elseif params["qosInterfaceOptions"]=="trust" then
            for i=1,#portNum do
                ret,val = myGenWrapper("cpssDxChCosPortQosTrustModeGet",{
                    {"IN","GT_U8","devNum",devNum},
                    {"IN","GT_U32","portNum",portNum[i]},
                    {"OUT",TYPE["ENUM"],"portQosTrustModePtr"}
                })

                if (ret==0) then
                    if val["portQosTrustModePtr"]==0 then res="No trust"
                    elseif val["portQosTrustModePtr"]==1 then res="VPT"
                    elseif val["portQosTrustModePtr"]==2 then res="DSCP"
                    elseif val["portQosTrustModePtr"]==3 then res="DSCP-VPT"
                    end

                    print("Port "..portNum[i].." trust:"..res)
                else
                        return false,"Could not get device "..devNum.. " port "..portNum[i].." trust mode"
                end
            end
        elseif params["qosInterfaceOptions"]=="shapers" then
            for i=1,#portNum do
                ret,val = myGenWrapper("cpssDxChPortTxShaperProfileGet",{
                    {"IN","GT_U8","devNum",devNum},
                    {"IN","GT_U32","portNum",portNum[i]},
                    {"OUT","GT_U16","burstSizePtr"},
                    {"OUT","GT_U32","maxRatePtr"},
                    {"OUT","GT_BOOL","enablePtr"}
                })
                if (ret==0) then
                    if (val["enablePtr"]==true) then res="Enabled" else res="Disabled" end
                    print("Port "..portNum[i].."\tStatus:"..res.."\tCommitted rate:"..val["maxRatePtr"].."Kbps\tCommitted burst:"..(val["burstSizePtr"]*4).."KBps"  )
                else
                    return false,"Could not get device "..devNum.. " port "..portNum[i].." profile"
                end
            end
        end
    end
end


--------------------------------------------
-- command registration: show qos interface
--------------------------------------------
CLI_addCommand("exec", "show qos interface", {
 func=qosInterfaceShow,
  help="Show qos inteface parameters",
 params={
   {type="values", "%qosInterfaceOptions"},

    { type="named","#interface",
        requirements = {["interface"] = { "qosInterfaceOptions" }},
         mandatory = {"interface"}
    }
  }
})

--------------------------------------------
-- command : qos profile update
--------------------------------------------
local function qosProfileUpdate(params)
    local devices,j, entry, iterator, index;
    local did_error, result, OUT_values;
    local command_data = Command_Data();

    if (params["devID"]=="all") then
        devices=wrLogWrapper("wrlDevList");
    else
        devices={params["devID"]};
    end

    for j=1,#devices do
        for iterator, index in number_range_iterator(params.index) do
            did_error, result, OUT_values =
                genericCpssApiWithErrorHandler(
                    command_data, "cpssDxChCosProfileEntryGet",{
                    {"IN","GT_U8","devNum",devices[j]},
                    {"IN","GT_U32","profileIndex",index},
                    {"OUT","CPSS_DXCH_COS_PROFILE_STC","cosPtr"}
                });
            if did_error then
                return false;
            end

            entry = deepcopy(OUT_values.cosPtr);
            if params.dp then
                entry.dropPrecedence = params.dp;
            end
            if params.dscp then
                entry.dscp = params.dscp;
            end
            if params.exp then
                entry.exp = params.exp;
            end
            if params.tc then
                entry.trafficClass = params.tc;
            end
            if params.up then
                entry.userPriority = params.up;
            end

            did_error, result, OUT_values =
                genericCpssApiWithErrorHandler(
                    command_data, "cpssDxChCosProfileEntrySet",{
                    {"IN","GT_U8","devNum",devices[j]},
                    {"IN","GT_U32","profileIndex",index},
                    {"IN","CPSS_DXCH_COS_PROFILE_STC","cosPtr", entry}
                });
            if did_error then
                return false;
            end
        end
    end
    return true;
end

CLI_addCommand("config", "qos profile", {
 func=qosProfileUpdate,
  help="configure qos profile",
    params={
     { type="named",
         { format="device %devID_all",      name="devID", help="The device number" },
         { format="index %number_range",    name="index", help="range of QOS profile indexes" },
         { format="dp %dropPrecedenceEnum", name="dp", help="drop precedence" },
         { format="dscp %GT_U32",           name="dscp", help="DSCP" },
         { format="exp %GT_U32",            name="exp", help="EXP" },
         { format="tc %GT_U32",             name="tc", help="traffic class" },
         { format="up %GT_U32",             name="up", help="user priority" },
         mandatory = {"devID", "index"},
         requirements={
             ["index"] = {"devID"},
         },
     }
   }
})

--------------------------------------------
-- command : show qos profile
--------------------------------------------
local function qosProfileShow(params)
    local devices,j, entry, iterator, index;
    local did_error, result, OUT_values;
    local command_data = Command_Data();

    if (params["devID"]=="all") then
        devices=wrLogWrapper("wrlDevList")
    else
        devices={params["devID"]};
    end

    for j=1,#devices do
        for iterator, index in number_range_iterator(params.index) do
            did_error, result, OUT_values =
                genericCpssApiWithErrorHandler(
                    command_data, "cpssDxChCosProfileEntryGet",{
                    {"IN","GT_U8","devNum",devices[j]},
                    {"IN","GT_U32","profileIndex",index},
                    {"OUT","CPSS_DXCH_COS_PROFILE_STC","cosPtr"}
                });
            if did_error then
                return false;
            end

            entry = deepcopy(OUT_values.cosPtr);
            print(
                "dev "    .. tostring(devices[j]) ..
                " index " .. tostring(index) ..
                " dp "    .. tostring(entry.dropPrecedence) ..
                " dscp "  .. tostring(entry.dscp) ..
                " exp "   .. tostring(entry.exp) ..
                " tc "    .. tostring(entry.trafficClass) ..
                " up "    .. tostring(entry.userPriority));

        end
    end
    return true;
end

CLI_addCommand("exec", "show qos profile", {
 func=qosProfileShow,
  help="show qos profile",
    params={
     { type="named",
         { format="device %devID_all",      name="devID", help="The device number" },
         { format="index %number_range",    name="index", help="range of QOS profile indexes" },
         mandatory = {"devID", "index"},
         requirements={
             ["index"] = {"devID"},
         },
     }
   }
})


-- ************************************************************************
---
--  qos_counters_set
--        @description  Set egress counter set
--
--        @param        params["devNum"]: device number (mandatory)
--                      params["counterSetNum"]: counter set 0/1 (mandatory)
--                      params["vlanId"]: vlan ID (optional)
--                      params["portNum"]: port number (optional)
--                      params["queue"]: queue number (optional)
--                      params["dp"]: drop-precedence (optional)
--
--        @return       true if successful otherwise false
--
-- ************************************************************************
local function qos_counters_set(params)
    local ret
    local dev
    local port
    local egress_counter_num
    local vlan
    local queue
    local dp
    local egressMode = 0
    
    dev = params["devNum"]
    egress_counter_num = params["counterSetNum"] 
    port = params["portNum"]
    vlan = params["vlanId"]
    queue = params["queue"]
    dp = params["dp"]

--[[
    CPSS_EGRESS_CNT_PORT_E = 1,
    CPSS_EGRESS_CNT_VLAN_E = 2,
    CPSS_EGRESS_CNT_TC_E   = 4,
    CPSS_EGRESS_CNT_DP_E   = 8
   ]]

    if port ~= nil then
        egressMode = egressMode + 1
    end
    if vlan ~= nil then
        egressMode = egressMode + 2
    end
    if queue ~= nil then
        egressMode = egressMode + 4
    end
    if dp ~= nil then
        egressMode = egressMode + 8
    end

    --configuring egress counter
    ret = myGenWrapper("cpssDxChPortEgressCntrModeSet",{  
            {"IN","GT_U8","devNum",dev},
            {"IN","GT_U8","cntrSetNum",egress_counter_num},
            {"IN","CPSS_PORT_EGRESS_CNT_MODE_ENT","setModeBmp",egressMode},        
            {"IN","GT_PHYSICAL_PORT_NUM","portNum",port},
            {"IN","GT_U16","vlanId",vlan},
            {"IN","GT_U8","tc",queue},
            {"IN","CPSS_DP_LEVEL_ENT","dpLevel",dp}
            })  
    if 0~=ret then
        print("Error at configuring egress counter "..returnCodes[ret])
        return false
    end         
    
        --reset egress counters              
    ret,val = myGenWrapper("cpssDxChPortEgressCntrsGet",{  
        {"IN","GT_U8","devNum",device},
        {"IN","GT_U8","cntrSetNum",egress_counter_num},
        {"OUT","CPSS_PORT_EGRESS_CNTR_STC","egrCntrPtr"}
        })  
    if 0~=ret then    
        print("Error resetting egress counters: "..returnCodes[ret])
    end    
 
    return true
end

-- ************************************************************************
---
--  qos_counters_show
--        @description  show egress counter sets
--
--        @param        params["devNum"]: device number (mandatory)
--
--        @return       true if successful otherwise false
--
-- ************************************************************************
local function qos_counters_show(params)
    local ret
    local dev
    local egrCntrPtr
    local egress_counter_num
    local val
    
    --[[
    GT_U32 outUcFrames;
    GT_U32 outMcFrames;
    GT_U32 outBcFrames;
    GT_U32 brgEgrFilterDisc;
    GT_U32 txqFilterDisc;
    GT_U32 outCtrlFrames;
    GT_U32 egrFrwDropFrames;
    GT_U32 mcFifo3_0DropPkts;
    GT_U32 mcFifo7_4DropPkts;
    ]]


    dev = params["devNum"]
    
    --get egress counters  
    egress_counter_num = 0             
    ret,val = myGenWrapper("cpssDxChPortEgressCntrsGet",{  
        {"IN","GT_U8","devNum",dev},
        {"IN","GT_U8","cntrSetNum",egress_counter_num},
        {"OUT","CPSS_PORT_EGRESS_CNTR_STC","egrCntrPtr"}
        })  
    if 0~=ret then    
        print("Error reading egress counter set 0: "..returnCodes[ret])
    else
        egrCntrPtr = val["egrCntrPtr"]
        print(string.format("Counter set %d :",egress_counter_num))
        print(string.format("  outUcFrames %-4s",egrCntrPtr["outUcFrames"]))
        print(string.format("  outMcFrames %-4s",egrCntrPtr["outMcFrames"]))
        print(string.format("  outBcFrames %-4s",egrCntrPtr["outBcFrames"]))
        print(string.format("  brgEgrFilterDisc %-4s",egrCntrPtr["brgEgrFilterDisc"]))
        print(string.format("  txqFilterDisc %-4s",egrCntrPtr["txqFilterDisc"]))
        print(string.format("  outCtrlFrames %-4s",egrCntrPtr["outCtrlFrames"]))
        print(string.format("  egrFrwDropFrames %-4s",egrCntrPtr["egrFrwDropFrames"]))
    end    

    --get egress counters  
    egress_counter_num = 1             
    ret,val = myGenWrapper("cpssDxChPortEgressCntrsGet",{  
        {"IN","GT_U8","devNum",dev},
        {"IN","GT_U8","cntrSetNum",egress_counter_num},
        {"OUT","CPSS_PORT_EGRESS_CNTR_STC","egrCntrPtr"}
        })  
    if 0~=ret then    
        print("Error reading egress counter set 1: "..returnCodes[ret])
    else
        egrCntrPtr = val["egrCntrPtr"]
        print(string.format("Counter set %d :",egress_counter_num))
        print(string.format("  outUcFrames %-4s",egrCntrPtr["outUcFrames"]))
        print(string.format("  outMcFrames %-4s",egrCntrPtr["outMcFrames"]))
        print(string.format("  outBcFrames %-4s",egrCntrPtr["outBcFrames"]))
        print(string.format("  brgEgrFilterDisc %-4s",egrCntrPtr["brgEgrFilterDisc"]))
        print(string.format("  txqFilterDisc %-4s",egrCntrPtr["txqFilterDisc"]))
        print(string.format("  outCtrlFrames %-4s",egrCntrPtr["outCtrlFrames"]))
        print(string.format("  egrFrwDropFrames %-4s",egrCntrPtr["egrFrwDropFrames"]))
    end    
 

    return true
end

-- ************************************************************************
---
--  qos_counters_clear
--        @description  Clear egress counter sets
--
--        @param        params["devNum"]: device number (mandatory)
--
--        @return       true if successful otherwise false
--
-- ************************************************************************
local function qos_counters_clear(params)
    local ret
    local dev
    local egress_counter_num
    local val
    
    dev = params["devNum"]
    
    for egress_counter_num = 0,1 do 
        --get egress counters  
        ret,val = myGenWrapper("cpssDxChPortEgressCntrsGet",{  
            {"IN","GT_U8","devNum",dev},
            {"IN","GT_U8","cntrSetNum",egress_counter_num},
            {"OUT","CPSS_PORT_EGRESS_CNTR_STC","egrCntrPtr"}
            })  
        if 0~=ret then    
            print("Error reading egress counter set: "..returnCodes[ret])
        end    
    end
    return true
end

--------------------------------------------
-- command registration: 
--      qos counters set
--------------------------------------------
CLI_addHelp("config", "qos counters", "egress counters commands") 
CLI_addCommand("config", "qos counters set", {
    func   = qos_counters_set,
    help   = "Set egress counters",
    params = {
        { type = "named",
            { format = "device %devID", name="devNum", help="The device number" },
            { format = "set %egressCounterSet", name="counterSetNum", help="Counter set number 0/1" },
            { format = "port %portNum", name="portNum", help="The port number" },
            { format = "vlan %vlan", name="vlanId", help="VLAN ID" },
            { format = "queue %queue_id_no_all", name="queue", help="Traffic class queue" },
            { format = "dp %drop_precedence_no_all", name="dp", help="Drop precedence" },
          mandatory = {"devNum", "counterSetNum"}
        }
    }
}) 

CLI_addCommand("config", "qos counters show", {
    func   = qos_counters_show,
    help   = "Show egress counters",
    params = {
        { type = "named",
            { format = "device %devID", name="devNum", help="The device number" },
          mandatory = {"devNum"}
        }
    }
}) 

CLI_addCommand("config", "qos counters clear", {
    func   = qos_counters_clear,
    help   = "Clear egress counters",
    params = {
        { type = "named",
            { format = "device %devID", name="devNum", help="The device number" },
          mandatory = {"devNum"}
        }
    }
}) 

CLI_type_dict["egressCounterSet"] = {
    checker = CLI_check_param_number,
    min=0,
    max=1,
    complete = CLI_complete_param_number,
    help="The counter set 0..1"
} 


