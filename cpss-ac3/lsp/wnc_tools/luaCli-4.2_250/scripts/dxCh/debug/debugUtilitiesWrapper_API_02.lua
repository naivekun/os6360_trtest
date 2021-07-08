--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* debugUtilitiesWrapper_API_02.lua
--*
--* DESCRIPTION:
--*       displaing of the debug information
--*
--* FILE REVISION NUMBER:
--*       $Revision: 2 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  debugReadReg
--        @description  Service functions using generic lua wrapper
--
--        @param devNum         - The device number
--        @param regAddr        - The register address
--
--        @return       {Return code, Register value}
--
local function debugReadReg(devNum, regAddr)
    local ret, values
    ret, values = myGenWrapper(
        "prvCpssDrvHwPpReadRegister", {
            { "IN", "GT_U8", "deviceId", devNum },
            { "IN", "GT_U32","regAddr", regAddr },
            { "OUT","GT_U32", "regData" }
        })
    return ret, values.regData
end


-- ************************************************************************
---
--  debugReadRegField
--        @description  This function reads a field
--
--        @param devNum         - The device number
--        @param regAddr        - The register address
--        @param fieldOffset    - The offset of the field
--        @param fieldLength    - The field length
--
--        @return       {Return code, Field value}
--
local function debugReadRegField(devNum, regAddr, fieldOffset, fieldLength)
    local ret, values
    ret, values = myGenWrapper(
        "prvCpssDrvHwPpGetRegField",
        {
           { "IN", "GT_U8",  "deviceId", devNum      },
           { "IN", "GT_U32", "regAddr",  regAddr     },
           { "IN", "GT_U32", "fieldOffset", fieldOffset }, 
           { "IN", "GT_U32", "fieldLength", fieldLength }, 
           { "OUT","GT_U32", "fieldData" }
        })
    return ret, values.fieldData
end


-- ************************************************************************
---
--  debugPrintDropCounters
--        @description  This function prints the prints the drop counters
--
--        @param device         - The device ID for which to print the 
--                                counters
--
--        @return       none
--
local function debugPrintDropCounters(device, bmap)
   local str
   local ret, val
   local bitmap
   local function debugPrintReg(device, regAddr, format)
       local ret, value
       ret,value = debugReadReg(device, regAddr)
       print(string.format(format,value))
   end
   local function debugPrintRegField(device, regAddr, fieldOffset, fieldLength, format)
        local ret, value
        ret, value = debugReadRegField(device, regAddr, fieldOffset, fieldLength)
        print(string.format(format, value))
   end
--

    if bmap~=nil then
        bitmap = tonumber(bmap)
    else
        bitmap = 0xffffffff
    end


-- Router Drop Counter value and configuration
   print("\nRouter:")
   ret,val = myGenWrapper("cpssDxChIpPortGroupDropCntGet",{    
                        {"IN","GT_U8","devNum",device},
                        {"IN","GT_PORT_GROUPS_BMP","portGroupsBmp",bitmap},
                        {"OUT","GT_U32","dropPkts"}})
    if ret~=0 then
        print("cpssDxChIpPortGroupDropCntGet() failed: "..to_string(ret))
        return
    end
   print("Drop Counter: "..to_string(val.dropPkts))

    ret,val = myGenWrapper("cpssDxChIpGetDropCntMode",{    
                        {"IN","GT_U8","devNum",device},
                        {"OUT","CPSS_DXCH_IP_DROP_CNT_MODE_ENT","dropCntMode"}})
    if ret~=0 then
        print("cpssDxChIpGetDropCntMode() failed: "..to_string(ret))
        return
    end
    print("RouterDropCntMode= "..to_string(val.dropCntMode))

-- Ingress Drop Counter value and configuration 0x0B00003C, 0x0B000040
   print("\nIngress:")
   
   ret,val = myGenWrapper("cpssDxChCfgIngressDropCntrGet",{    
                        {"IN","GT_U8","devNum",device},
                        {"OUT","GT_U32","counter"}})
    if ret~=0 then
        print("cpssDxChCfgIngressDropCntrGet() failed: "..to_string(ret))
        return
    end
    print("Drop Counter: "..to_string(val.counter))

   ret,val = myGenWrapper("cpssDxChCfgIngressDropCntrModeGet",{
                        {"IN","GT_U8","devNum",device},
                        {"OUT","CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_ENT", "mode"},
                        {"OUT","GT_PORT_NUM", "portNum"},
                        {"OUT","GT_U16", "vlan"}})
    if ret~=0 then
        print("cpssDxChCfgIngressDropCntrModeGet() failed: "..to_string(ret))
        return
    end
    if (val.mode == "CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_ALL_E") then
        print("Configuration: "..to_string(val.portNum + val.vlan))
    elseif (val.mode == "CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_VLAN_E") then
        print("Configuration: "..to_string(val.vlan))
    elseif (val.mode == "CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_PORT_E") then
        print("Configuration: "..to_string(val.portNum))
    else
        print("Error: unexcepted value in cpssDxChCfgIngressDropCntrModeGet()")
        return
    end

   print("    IngressDropCntMode="..to_string(val.mode))
   print("    IngressDropCntPort="..to_string(val.portNum))

-- CPU Code Rate Limiter Drop Counter 0x0B000068 and TO CPU Rate LimiterConfiguration 0x0B500000 (31 rate limiters)
   print("\nCPU Code Rate Limiter:")
   ret,val =
         myGenWrapper("cpssDxChNetIfPortGroupCpuCodeRateLimiterDropCntrGet",{    
                        {"IN","GT_U8","devNum",device},
                        {"IN","GT_PORT_GROUPS_BMP","portGroupsBmp",bitmap},
                        {"OUT","GT_U32","dropCntr"}})
    if ret~=0 then
        print("cpssDxChNetIfPortGroupCpuCodeRateLimiterDropCntrGet() failed: "..
                                                               to_string(ret))
        return
    end
    print("Drop Counter: "..to_string(val.dropCntr))

    print("Configuration:")
    for k = 1, 31 do
        ret,val =
          myGenWrapper("cpssDxChNetIfPortGroupCpuCodeRateLimiterTableGet",{
                            {"IN","GT_U8","devNum",device},
                            {"IN","GT_PORT_GROUPS_BMP","portGroupsBmp",bitmap},
                            {"IN","GT_U32","rateLimiterIndex",k},
                            {"OUT","GT_U32","windowSize"},
                            {"OUT","GT_U32","pktLimit"}})
        if ret~=0 then
            print("cpssDxChNetIfPortGroupCpuCodeRateLimiterTableGet() failed: "..
                                                                  to_string(ret))
            return
        end
        print("Limiter: "..to_string(k)..
              "  WindowSize: "..to_string(val.windowSize)..
              "  PktLimit: "..to_string(val.pktLimit))
    end
   
-- Ingress Forwarding Restrictions:  Dropped Packets Counter: 0x0B02000C Configurations: 0x0B020000, 0x0B020004, 0x0B020008
   print("\nIngress Forwarding Restrictions:")
   --debugPrintReg(device, 0x0B02000C, "Drop Counter (0x0B02000C): 0x%X.")
   
   ret,val = myGenWrapper("cpssDxChNstPortGroupIngressFrwFilterDropCntrGet",{    
                        {"IN","GT_U8","devNum",device},
                        {"IN","GT_PORT_GROUPS_BMP","portGroupsBmp",bitmap},
                        {"OUT","GT_U32","ingressCnt"}})
    if ret~=0 then
        print("cpssDxChNstPortGroupIngressFrwFilterDropCntrGet() failed: "..
                                                               to_string(ret))
        return
    end
    print("Drop Counter: "..to_string(val.ingressCnt))
   
   
    -- ret,val = myGenWrapper("cpssDxChNstPortIngressFrwFilterGet",{
              -- {"IN","GT_U8","devNum",device},
              -- {"IN", "GT_PORT_NUM","portNum", 0},
              -- {"IN", "CPSS_NST_INGRESS_FRW_FILTER_ENT",
                 -- "CPSS_NST_INGRESS_FRW_FILTER_TO_CPU_E"},
              -- {"OUT","GT_BOOL", "enable"}})
    -- if ret~=0 then
        -- print("cpssDxChNstPortIngressFrwFilterGet() failed: "..to_string(ret))
        -- return
    -- end
    -- print("TO CPU Configuration Register : "..to_string(val.enable))
   
   -- --debugPrintReg(device, 0x0B020004, "TO NETWORK Configuration Register (0x0B020004): 0x%X.")
    -- ret,val = myGenWrapper("cpssDxChNstPortIngressFrwFilterGet",{
              -- {"IN","GT_U8","devNum",device},
              -- {"IN", "GT_PORT_NUM","portNum", 0},
              -- {"IN", "CPSS_NST_INGRESS_FRW_FILTER_ENT",
                 -- "CPSS_NST_INGRESS_FRW_FILTER_TO_NETWORK_E"},
              -- {"OUT","GT_BOOL", "enable"}})
    -- if ret~=0 then
        -- print("cpssDxChNstPortIngressFrwFilterGet() failed: "..to_string(ret))
        -- return
    -- end
    -- print("TO NETWORK Configuration Register: "..to_string(val.enable))
   
   -- --debugPrintReg(device, 0x0B020008, "TO ANALYZER Configuration Register (0x0B020008): 0x%X.")
    -- ret,val = myGenWrapper("cpssDxChNstPortIngressFrwFilterGet",{
              -- {"IN","GT_U8","devNum",device},
              -- {"IN", "GT_PORT_NUM","portNum", 0},
              -- {"IN", "CPSS_NST_INGRESS_FRW_FILTER_ENT",
                 -- "CPSS_NST_INGRESS_FRW_FILTER_TO_ANALYZER_E"},
              -- {"OUT","GT_BOOL", "enable"}})
    -- if ret~=0 then
        -- print("cpssDxChNstPortIngressFrwFilterGet() failed: "..to_string(ret))
        -- return
    -- end
    -- print("TO ANALYZER Configuration Register: "..to_string(val.enable))

-- MLL MC FIFO Drop Counter 0x0D800984
   print("\nMLL MC FIFO:")
   ret,val = myGenWrapper("cpssDxChIpPortGroupMultiTargetQueueFullDropCntGet",{    
                        {"IN","GT_U8","devNum",device},
                        {"IN","GT_PORT_GROUPS_BMP","portGroupsBmp",bitmap},
                        {"OUT","GT_U32","dropPkts"}})
    if ret~=0 then
        print("cpssDxChIpPortGroupMultiTargetQueueFullDropCntGet() failed: "..
                                                               to_string(ret))
        return
    end
    print("Drop Counter: "..to_string(val.dropPkts))

-- NAT Drop Counter value 
   ret,val = myGenWrapper("cpssDxChIpNatDroppedPacketsCntGet",{    
                        {"IN","GT_U8","devNum",device},                        
                        {"OUT","GT_U32","natDropPktsPtr"}})
   -- NAT is only supported for Bobcat2 B0 and above 30="GT_NOT_APPLICABLE_DEVICE"
    if ((ret~=0)and(ret~=30))then
        print("cpssDxChIpNatDroppedPacketsCntGet() failed: "..to_string(ret))
        return
    else
        if(ret==0)then
            print("\nNAT:")
            print("Drop Counter: "..to_string(val.natDropPktsPtr))
        end
    end

    if (DeviceCpuPortMode ~= "CPSS_NET_CPU_PORT_MODE_SDMA_E") then
 -- CPU Port Configuration Register and MIB Counters: counters: 0x00000060-0x0000007C, configuration:0x000000A0
        print("\nCPU Port Configuration Register and MIB Counters:")
           ret,val = myGenWrapper("cpssDxChPortMacCountersClearOnReadGet",{
                  {"IN","GT_U8","devNum",device},
                  {"IN", "GT_PHYSICAL_PORT_NUM","portNum",
                                    cpssConst.enum.CPSS_CPU_PORT_NUM_CNS.value},
                  {"OUT","GT_BOOL", "enable"}})
        if ret~=0 then
            print("cpssDxChPortMacCountersClearOnReadGet() failed: "..to_string(ret))
            return
        end
        print("configuration: "..to_string(val.enable))

        print("\nMIB Counters:")
        local cntrNames ={
            "CPSS_GOOD_PKTS_SENT_E",
            "CPSS_MAC_TRANSMIT_ERR_E",
            "CPSS_GOOD_OCTETS_SENT_E",
            "CPSS_DROP_EVENTS_E",
            "CPSS_GOOD_PKTS_RCV_E",
            "CPSS_BAD_PKTS_RCV_E",
            "CPSS_GOOD_OCTETS_RCV_E",
            "CPSS_BAD_OCTETS_RCV_E"}

        for k = 1, 8 do
            ret,val = myGenWrapper("cpssDxChMacCounterGet",{
                      {"IN","GT_U8","devNum",device},
                      {"IN","GT_PHYSICAL_PORT_NUM", "portNum",
                                    cpssConst.enum.CPSS_CPU_PORT_NUM_CNS.value},
                      {"IN","CPSS_PORT_MAC_COUNTERS_ENT", "cntrName", cntrNames[k]},
                      {"OUT","GT_U64","cntrValue"}})
            if ret~=0 then
                print("cpssDxChMacCounterGet() failed: "..to_string(ret))
                return
            end
            print(cntrNames[k].."   \t"..to_string(val.cntrValue.l[0]))
        end
    else
        local rxerrs = nil
        ret, val = myGenWrapper("cpssDxChNetIfSdmaRxErrorCountGet", {
            { "IN",  "GT_U8",  "devNum", device },
            { "OUT", "CPSS_NET_SDMA_RX_ERROR_COUNTERS_STC", "rxErrCount" }
        })
        if ret == 0 then
            rxerrs = { l={[0] = 0, [1] = 0 }}
            for i=0,7 do
                local e = val.rxErrCount.counterArray[i]
                rxerrs = wrlCpssGtU64MathAdd(rxerrs, {l={[0]=e,[1]=0}})
            end
            print("Rx error  :"..to_string(rxerrs.l[0]))
        else
            print("cpssDxChNetIfSdmaRxErrorCountGet() failed: "..to_string(ret))
        end
    end

   return
end


-- ************************************************************************
---
--  show_drop_counters
--        @description  CLI for Call print debug inoformation of various 
--                      cases. Prints: output values with names.
--                      Example:
--                          cpssDebugPrint device 0 drop_counters;
--                          cpssDebugPrint sdma_descriptor device 1.
--
--        @param params         - params["info"]: name of specifc debug 
--                                print. 
--                                "drop_counters",
--                                "sdma_descriptor",
--                                "port_attribute",
--                                "interrupt_tree",
--                                "Design_Units_Global_Configurations",
--                                "Table_of_port_attributes".
--                                params["device"]: device number
--
--        @return       true, nil
--
local function show_drop_counters(params)
	debugPrintDropCounters(params.devID, params["portgroup"])
	return true, nil
end

-- *debug*
-- show drop counters %devID
CLI_addHelp("debug", "show", "Display debug information")
CLI_addHelp("debug", "show drop", "Display drop related debug information")
CLI_addCommand("debug", "show drop counters", {
    func=show_drop_counters,
    help="Display drop counters",
    params={
        {type="values","%devID"},
        {type="named",
            { format="port-group %bitmap", name="portgroup",
                                   help = "Set port group bitmap for multicore devices."}}
        }
})

--Router Drop Counter  value and configuration: 0x02800950, 0x02800954
--Ingress Drop Counter value and configuration 0x0B00003C, 0x0B000040
--CPU Code Rate Limiter Drop Counter 0x0B000068 and TO CPU Rate LimiterConfiguration 0x0B500000 (31 rate limiters)
--Ingress Forwarding Restrictions:  Dropped Packets Counter: 0x0B02000C Configurations: 0x0B020000, 0x0B020004, 0x0B020008
--MLL MC FIFO Drop Counter 0x0D800984
--CPU Port Configuration Register and MIB Counters: counters: 0x00000060-0x0000007C, configuration:0x000000A0
--Transmit Queue (Tx) and Egress MIB Counters: Set<%n>  n=0,1
--Bridge egress filtered egress Counter: 0x01B40150 and 0x01B40170
--Tail Dropped Packet Counter: 0x01B40154 and 0x01B40174
--Egress Forwarding Restrictions dropped: 0x01B4015C and 0x01B4017C
--Multicast FIFO Dropped Packets counter: 0x01B4015C and 0x01B4017C
--(internal?)Dropped BCN Descriptors Counter: 0x0A40101C
--Bridge Drop Counter and Security Breach Drop Counters
--Bridge Global Configuration1: 0x02040004 (bits 7-18,19 used for "Port/VLAN Security Breach Drop Counter", bits 20-24,0 used for Bridge Drop Counter Mode[4:0],[5])
--Global Security Breach Filter Counter : 0x02040104
--Port/VLAN Security Breach Drop Counter: 0x02040108
--Bridge Filter Counter: 0x02040150
--Ingress Ports Rate Limit Configuration
--Ingress Rate limit Drop Counter[31:0]: 0x02040148
--Ingress Rate limit Drop Counter[39:32]: 0x0204014C
--
--
--
--
--
--
