--********************************************************************************
--*              (C), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* common.lua
--*
--* DESCRIPTION:
--*       Any functions, variables etc that are required in several tests.
--*
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************


--************************************************************************
--  vlanMappingTest
--        @description  test a VLAN Translation feature with appropriate
--                      CLI commands. This function is used by several
--                      Lua-tests with different configurations.
--
--        @strConfigFileName   - a name of file with configuration commands
--        @strDeconfigFileName - a name of file with deconfiguration commands
--        strPacketFileName    - a name of file with a packet to be sent
--        expected             - a table. Every item is a table of two fields:
--                      1) portNum - an egress port number,
--                      2) packetFields - a table with same fiedls as an argument
--                         of function genPktL2. it is a list of fields that
--                         changed in outgoing packet comparing with incoming
--                         packet. If a field should be deleted it is "false".
--                         An example:
--                         {{portNum=2, packetFields={ieee802_1q_Tag="81000007"}},
--                          {portNum=3, packetFields={ieee802_1q_Tag=false}}}.
--                         It means we expect outgoing packets on ports 2,3.
--                         This packets are the same as incoming packet except
--                         a vlan tag id. Packet on port 2 should be received
--                         with vlan id=7, and the packet on the port 3 should
--                         be received without vlan tag at all.
--
function vlanMappingTest(strConfigFileName, strDeconfigFileName,
                         strPacketFileName, expected)
   local rxPort  = devEnv.port[1];

   -- arguments to  luaTgfTransmitPacketsWithExpectedEgressInfo() functon
   local rxPacketInfo, rxPacket

   -- read a packet
   ret, rxPacketInfo, rxPacket = pcall(dofile, strPacketFileName)
   if not ret then
      error('Error in the network packet')
   end

   -- run a configuration
   if not executeLocalConfig(strConfigFileName) then
      error("executeLocalConfig " .. strConfigFileName)
   end

   -- fill egressInfoTable with expected ports/packets
   local egressInfoTable = {}
   if expected then
      for unused, egress in pairs(expected) do
         local txPacketInfo = deepcopy(rxPacketInfo)
         if egress.packetFields then
            for key, value in pairs(egress.packetFields) do
               if value then
                  txPacketInfo[key] = value
               else --value=false or value=nil
                  txPacketInfo[key] = nil
               end
            end
         end
         table.insert(egressInfoTable,
                      {portNum = egress.portNum,
                       pktInfo = {fullPacket = genPktL2(txPacketInfo)},
                       packetCount = 1})
      end
   end

   -- send and capture the packets
   local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(
      {
         devNum = devEnv.dev,
         portNum = rxPort,
         pktInfo = {fullPacket = rxPacket}
      }, -- transmitInfo
      egressInfoTable
   )
   if rc ~= 0 then
      error(string.format("luaTgfTransmitPacketsWithExpectedEgressInfo failed. Error:"..
                             "#%d, \" %s \" ", rc, returnCodes[rc]))
   end

   -- clear configuration
   if not executeLocalConfig(strDeconfigFileName) then
      error("executeLocalConfig " .. strDeconfigFileName)
   end
   return 0
end

-- ************************************************************************

function gen_Test_Result_Def_V01(TestResultTable)
    return TestResultTable;
end

-- ************************************************************************

function AddFileFromLocalFS_v01(FromLocalFS)

   if wraplLoadFileFromLocalFS(FromLocalFS) then
       --printMsgLvlLog("debug", "Ok add : file FromLocalFS")                                   
       return true
   else
        rc=pcall(require,FromLocalFS)
        if not rc then
            printMsgLvlLog("debug", "can't add :"..FromLocalFS.." file not exists")
            return false
        end
   end
   return true
end

-- ************************************************************************

function Set_Test_Results_v01(Test_Results_File)

    local res
    local ret

    res = AddFileFromLocalFS_v01(Test_Results_File)
    if res == false then
        printMsgLvlLog("debug", "Failed to open  file " .. Test_Results_File)
        return false, nil;
    end
    ret, TestResultTable = pcall(dofile,Test_Results_File)
    return ret, TestResultTable;
end

-- ************************************************************************
--************************************************************************
--  getPclRange
--        @description  return first and last TCAM indices relevant to
--                      PCL clients.
--
--        @devNum   - a device number
--
function getPclRange(devNum)
    -- assume that TTI takes upper floors of TCAM.
    -- return two parameters: first index, last index
    local ttiBase = myGenWrapper("appDemoDxChTcamTtiBaseIndexGet",{
         {"IN","GT_U8","devNum",devNum},
         {"IN","GT_U32","hitNum",0}
    })
    return 0, ttiBase - 1
end


cmdLuaCLI_registerCfunction("prvLuaTgfIngressTaggingForCapturedPortsSet")
local ingress_tpid_profile_for_empty_bmp_index = 6
function xcat3x_start(start)
    if (is_xCat3x_in_system()) then
        -- the configuration not use tpid 6  --> use it as 'empty' bmp
        -- just what we need.
        prvLuaTgfIngressTaggingForCapturedPortsSet(devNum,start--[[start]],1--[[CPSS_VLAN_ETHERTYPE0_E]],ingress_tpid_profile_for_empty_bmp_index)
        prvLuaTgfIngressTaggingForCapturedPortsSet(devNum,start--[[start]],2--[[CPSS_VLAN_ETHERTYPE1_E]],ingress_tpid_profile_for_empty_bmp_index)
    end
end
-- function to set default UP0 on the port
function xcat3x_set_ingress_port_vpt(portNum,newUp)
    if(is_xCat3x_in_system()) then
        -- the UNI command set TTI rules.
        -- in regular system : those rules implicit set the UP0 (vpt) to value 0
        -- but in AC3X , for 88e1690 ports: the TTI action NOT modify the UP0 (vpt) to value 0
        -- because as ingress with 'DSA Tag' ... so the for those the TTI not modify !!!
        local command_tpidProfileForPort = 
        [[
        end
        config
        interface ethernet ${dev}/]]..portNum..[[

        qos default-up ]] .. newUp .. [[
        
        exit 
        end
        ]]
        executeStringCliCommands(command_tpidProfileForPort)
    end
end

function globalLogGetLineFormatted(line, values)
    -- If line doesn't start with "@" it will be return as is.
    -- If line looks like "@name@<text>" the string.format(<text>, values[name])
    -- will be return.
    -- If log line looks like "@@<text>" the line "@<text>" will be return.
    -- EXAMPLES:
    -- 1)
    --     values : {rc="GT_OK"}
    --     line   : "@rc@    EXIT: %s"
    --     result : "    EXIT: GT_OK"
    -- 2)
    --     values : any
    --     line   : "@@foo bar"
    --     result : "@foo bar"
    -- 3)
    --     values : any
    --     line   : "@foo bar"
    --     result : FORMAT ERROR! closing "@" is not found.
    -- 4)
    --     values : nil
    --     line   : "@foo@ bar"
    --     result : ERROR! value["foo"] doesn't exist!

    if line and string.sub(line, 1, 1)=="@" then
        local key = string.match(line, "@(.-)@")
        if key == nil then
            error("bad log line format: ", line)
        elseif key == "" then
            line = string.sub(line, 2)
        else
            local val = values[key]
            if values == nil or values[key] == nil then
                error(string.format("Substitition key \"%s\" is not found:", key))
            else
                line = string.format(string.sub(line,#key+3), values[key])
            end
        end
    end
    return line
end

