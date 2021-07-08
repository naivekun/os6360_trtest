--[[

    utilities for testing the VSS 'system' .. the system is described below.

--]]

--[[            the 'system'

    -----------                             -----------
    |cc7(vss4)|                             |cc5(vss3)|
    -----------                             -----------
         |                                   |     |
        36                                  36     54
         |                              |----|     |----|
         |                              |               |
         23                             23              23
         |                              |               |
    ------------                    ------------    ------------
    |lc1       |                    |lc1       |    |lc5       |
    ------------                    ------------    ------------
    |    |  |  |                    |    |          |    |   |
    20  18  8  0                    18   8          0    8   18
    |    |  |  |                    |    |          |    |   |
    |    |  |  |                    |    |          |    |   |
    |    |  |  |--------------------|    |          |    |   |
    |    |  |       vss-trunk            |          |    |   |
    |    |  |----------------------------+----------|    |   |
    |    |                               |               |   |
    |    |                               |               |   |
    |    ---------------|   |-------------               |   |
    |           network |   |                            |   |
    |            trunk  |   |                            |   |
station_1           station_2                    station_4  station_5
                    station_3
    
    
--]]

local devFamily, devSubFamily = wrlCpssDeviceFamilyGet(devNum)
if "CPSS_PP_FAMILY_DXCH_AC3X_E" == devFamily then
    global_test_data_env.ePort1008 = 808
    global_test_data_env.ePort1018 = 818
    global_test_data_env.ePort1020 = 820
    global_test_data_env.ePort4500 = 900
    global_test_data_env.ePort5008 = 1008
    global_test_data_env.ePort5018 = 1018
else
    global_test_data_env.ePort1008 = 1008
    global_test_data_env.ePort1018 = 1018
    global_test_data_env.ePort1020 = 1020
    global_test_data_env.ePort4500 = 4500
    global_test_data_env.ePort5008 = 5008
    global_test_data_env.ePort5018 = 5018
end

-- when nil ignored , otherwise test only specific packet_id that match 'current_packet_id'
local debug_specific_test_Id = nil
-- the Id of the packet that is send
local current_packet_id = 1

--'extern' _debug_to_string(...)
local devNum = devEnv.dev-- sw devNum

-- get port for index for LC ports (AC3)
function vss_lc_port(portIndex)
    local devNum = devEnv.dev

    if not is_sip_5(devNum) then
        -- non sip5 devices need to use actual 'tested' ports
        -- only sip5 need to call getTestPortExplicitFamily("XCAT3",portIndex)
        return getTestPort(portIndex)
    end 


    return getTestPortExplicitFamily("XCAT3" , portIndex)
end

-- get port for index for CC ports (BC2)
local function vss_cc_port(portIndex)
    local devNum = devEnv.dev

    if is_sip_5(devNum) then
        -- sip5 devices need to use actual 'tested' ports
        -- only non sip5 need to call getTestPortExplicitFamily("BOBCAT2",portIndex)
        return getTestPort(portIndex)
    end 

    return getTestPortExplicitFamily("BOBCAT2" , portIndex)
end

-- get structure for 'flood' , with src eport
local function get_STC_edsa_frw_src_eport_trg_flood(srcHwDevNum , srcEPort , origSrcPhyPort , srcId , isDsa8Bytes)
    local dsaTagType
    local actual_egrFilterRegistered
    
    if is_sip_5_25() then
        actual_egrFilterRegistered = true
    else
        actual_egrFilterRegistered = false
    end
    
    if isDsa8Bytes then
        dsaTagType="CPSS_DXCH_NET_DSA_2_WORD_TYPE_ENT"
    else
        dsaTagType="CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT"
    end

    return {
          dsaInfo={
            forward={
              source={
                portNum=srcEPort
              },
              srcIsTagged="CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E",
              origSrcPhy={
                portNum=origSrcPhyPort
              },
              srcIsTrunk=false,
              egrFilterRegistered = actual_egrFilterRegistered,
              origSrcPhyIsTrunk=false,
              srcId=srcId,
              dstInterface={
                type="CPSS_INTERFACE_VID_E",
                vlanId=1,
              },
              srcHwDev=srcHwDevNum
            }
          },
          commonParams={
            dsaTagType=dsaTagType,
            vpt=1,
            vid=1
          },
          dsaType="CPSS_DXCH_NET_DSA_CMD_FORWARD_E"
        }
end

local function get_STC_edsa_frw_src_eport_trg_eport(srcHwDevNum , srcEPort , origSrcPhyPort , srcId , trgHwDevNum , trgEport , isTrgPhyPortValid, trgPhyPort, isDsa8Bytes)
    local dsaTagType
    
    if isDsa8Bytes then
        dsaTagType="CPSS_DXCH_NET_DSA_2_WORD_TYPE_ENT"
    else
        dsaTagType="CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT"
    end
    
    local new_isTrgPhyPortValid = (isTrgPhyPortValid and (srcHwDevNum == trgHwDevNum))
    local new_trgPhyPort = trgPhyPort
    if isDsa8Bytes then
        new_trgPhyPort = new_trgPhyPort % 64  -- only 6 bits in the dsa
    else
        new_trgPhyPort = new_trgPhyPort % 256 -- only 8 bits in the dsa
    end
    
    if false == new_isTrgPhyPortValid then
        new_trgPhyPort = 62 -- use null port (62), because BC2 assign it when 'isTrgPhyPortValid = not valid'
    end

    return {
      dsaInfo={
        forward={
          dstEport=trgEport,
          source={
            portNum=srcEPort
          },
          tag0TpidIndex=0,
          phySrcMcFilterEnable=false,
          srcIsTagged="CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E",
          origSrcPhy={
            portNum=origSrcPhyPort
          },
          srcIsTrunk=false,
          egrFilterRegistered=true,
          srcId=srcId,
          isTrgPhyPortValid = new_isTrgPhyPortValid,
          dstInterface={
            type="CPSS_INTERFACE_PORT_E",
            devPort={
              portNum=new_trgPhyPort,
              devNum=trgHwDevNum
            }
          },
          srcHwDev=srcHwDevNum
        }
      },
      commonParams={
        dsaTagType=dsaTagType,
        vpt=1,
        vid=1
      },
      dsaType="CPSS_DXCH_NET_DSA_CMD_FORWARD_E"
    }
end

function get_STC_dsa_frw_src_port_trg_port(srcPort,trgPort)
    local hwDevNum = 0
    return get_STC_edsa_frw_src_eport_trg_eport(hwDevNum , srcPort , srcPort , 0 , hwDevNum , 0 , true , trgPort, true)
end

-- get dsa and return copy with modified srcId
local function modify_SrcId_In_Dsa(devNum,origDsa,newSrcId)
    -- convert DSA to STC
    local rc, dsaSTC = wrlDxChDsaToStruct(devNum , origDsa)
    
    -- updat the srcId
    dsaSTC.dsaInfo.forward.srcId = newSrcId
    -- convert the updated STC to string
    return luaTgfDsaStringFromSTC(devNum,dsaSTC)
end


--generate test packet
local packetPayload = "6666"..-- dummy ethertype
        "000102030405060708090a0b0c0d0e0f"..
        "101112131415161718191a1b1c1d1e1f"..
        "202122232425262728292a2b2c2d2e2f"..
        "303132333435363738393a3b3c3d3e3f"
        
 station_1 = "station_1"
 station_2 = "station_2"
 station_3 = "station_3"
 station_4 = "station_4"
 station_5 = "station_5"

-- describe stations

local station_1_macAddr = "000123400011" 
local station_2_macAddr = "000123400022" 
local station_3_macAddr = "000123400033" 
local station_4_macAddr = "000123400044" 
local station_5_macAddr = "000123400055" 
local station_5_macAddr_case_4 = "000123400056" -- cause LBH to {4,1018} via vss 3 lc 1
local station_5_macAddr_case_5 = "000123400857" -- cause LBH to {4,1018} via vss 3 lc 5
local station_5_macAddr_case_6 = "000123400067" -- cause LBH to {3,1008} (local vss3 - no need vss trunk)
local station_5_macAddr_case_7 = "000123400068" -- cause LBH to flood vss trunk on vss3 lc1 , flood network trunk on vss 4 lc 1
--local station_5_macAddr_case_8 = "000123400057" -- cause LBH to flood vss trunk on vss3 lc5 , flood network trunk on vss 4 lc 1
local station_2_macAddr_case_9 = "000123400023" -- cause LBH to flood vss trunk on vss3 lc5

devicesName_vss3_cc5 = "vss3_cc5"
devicesName_vss4_cc7 = "vss4_cc7"
devicesName_vss3_lc1 = "vss3_lc1"
devicesName_vss3_lc5 = "vss3_lc5"
devicesName_vss4_lc1 = "vss4_lc1"

local src_id_for_vss_trunk = vss_internal_db_on_lc_used_src_id_for_vss_trunk


-- all 'flood' on this device looks the same , as we can not check egress packet
local function get_test_case_for_vss4_cc7_flood(in_dsa) 
    local sub_test_case =  {
        deviceName      = devicesName_vss4_cc7,
        -- ingress info
        ingressInfo = {
            physicalPort    = vss_cc_port(3),
            dsaTag          = in_dsa
        },
        -- egress info : flooding
        egressInfoArr = {
            {physicalPort = vss_cc_port(3)}, -- can not check egress packet on the port that is also the ingress port
            --+++++++++++++
            {physicalPort = vss_cc_port(1)},-- port not configured .. get flood in vlan 1
            {physicalPort = vss_cc_port(2)},-- port not configured .. get flood in vlan 1
            {physicalPort = vss_cc_port(4)} -- port not configured .. get flood in vlan 1
        }
    }
    
    return sub_test_case
end

-- all 'uc' on this device looks the same , as we can not check egress packet
local function get_test_case_for_vss4_cc7_uc(in_dsa) 
    local sub_test_case =  {
        deviceName      = devicesName_vss4_cc7,
        -- ingress info
        ingressInfo = {
            physicalPort    = vss_cc_port(3),
            dsaTag          = in_dsa
        },
        -- egress info : single destination (ingress port)
        egressInfoArr = {
            {physicalPort = vss_cc_port(3)}, -- can not check egress packet on the port that is also the ingress port
            --+++++++++++++
            {physicalPort = vss_cc_port(1), notEgress = true},
            {physicalPort = vss_cc_port(2), notEgress = true},
            {physicalPort = vss_cc_port(4), notEgress = true}
        }
    }
    
    return sub_test_case
end

-- all 'uc' on this device looks the same , as we can not check egress packet
local function get_test_case_for_vss3_cc5_uc_return_to_src_phy_port(in_port,in_dsa) 
    local sub_test_case =  {
        deviceName      = devicesName_vss3_cc5,
        -- ingress info
        ingressInfo = {
            physicalPort    = in_port,
            dsaTag          = in_dsa
        },
        -- egress info : single destination (ingress port)
        egressInfoArr = {
            -- the 'in_port' will get :
                -- {physicalPort = in_port}, -- can not check egress packet on the port that is also the ingress port
            --+++++++++++++
            {physicalPort = vss_cc_port(1), notEgress = true}, -- one of those will be notEgress = nil
            {physicalPort = vss_cc_port(2), notEgress = true}, -- one of those will be notEgress = nil
            {physicalPort = vss_cc_port(3), notEgress = true}, -- one of those will be notEgress = nil
            {physicalPort = vss_cc_port(4), notEgress = true}  -- one of those will be notEgress = nil
        }
    }
    
    for ii = 1,4 do 
        if sub_test_case.egressInfoArr[ii].physicalPort == in_port then
            sub_test_case.egressInfoArr[ii].notEgress = nil
            break
        end
    end
    
    return sub_test_case
end

-- all 'uc' from vss trunk go to the 'uplink' unmodified
local function get_test_case_for_lc_uc_from_vss_trunk(deviceName,in_port,in_dsa) 
    --local devNum = 0
    local out_dsa = modify_SrcId_In_Dsa(devNum,in_dsa,src_id_for_vss_trunk)

    local sub_test_case = {
        deviceName      = deviceName,
        -- ingress info
        ingressInfo = {
            physicalPort    = in_port,
            dsaTag = in_dsa
        },
        -- egress info
        egressInfoArr = {
            {physicalPort = vss_lc_port(4) , dsaTag = out_dsa},
            {physicalPort = vss_lc_port(1) , notEgress = true},-- one of those lines removed
            {physicalPort = vss_lc_port(2) , notEgress = true},-- one of those lines removed
            {physicalPort = vss_lc_port(3) , notEgress = true},-- one of those lines removed
            {physicalPort = vss_lc_port(5) , notEgress = true},-- one of those lines removed
        }
    }

    local lastIndex = 5
    for ii = 2,lastIndex do 
        if sub_test_case.egressInfoArr[ii].physicalPort == in_port then
            sub_test_case.egressInfoArr[ii].physicalPort = sub_test_case.egressInfoArr[lastIndex].physicalPort
            sub_test_case.egressInfoArr[lastIndex] = nil
            break
        end
    end
    
    --_debug_to_string("from_vss_trunk : sub_test_case = ",sub_test_case)

    return sub_test_case
end

-- all 'uc' from uplink port that need to egress to network port
local function get_test_case_for_lc_uc_uplink_to_network(deviceName,out_port,in_dsa) 
    local sub_test_case = {
        deviceName      = deviceName,
        -- ingress info
        ingressInfo = {
            physicalPort    = vss_lc_port(4),
            dsaTag = in_dsa
        },
        -- egress info
        egressInfoArr = {
            {physicalPort = vss_lc_port(1) , notEgress = true},-- one of those lines change notEgress to nil
            {physicalPort = vss_lc_port(2) , notEgress = true},-- one of those lines change notEgress to nil
            {physicalPort = vss_lc_port(3) , notEgress = true},-- one of those lines change notEgress to nil
            {physicalPort = vss_lc_port(5) , notEgress = true} -- one of those lines change notEgress to nil
        }
    }

    for ii = 1,4 do 
        if sub_test_case.egressInfoArr[ii].physicalPort == out_port then
            sub_test_case.egressInfoArr[ii].notEgress = nil
            break
        end
    end
    
    --_debug_to_string("uplink_to_network : sub_test_case = ",sub_test_case)

    return sub_test_case
end

-- convert "000123400011" to "00:01:23:40:00:11"
local function convertMacStringToMacCommandFormat(macString)
    local cli_mac = 
        string.sub(macString,1 , 2) .. ":" ..
        string.sub(macString,3 , 4) .. ":" ..
        string.sub(macString,5 , 6) .. ":" ..
        string.sub(macString,7 , 8) .. ":" ..
        string.sub(macString,9 ,10) .. ":" ..
        string.sub(macString,11,12)
    
    return cli_mac
end

 
-- dsa tag that state 'ingress from dev=0 , port=18'
--"c0903001".."100002e0"
local dsa_tag_info_src_lc_port_18 = luaTgfDsaStringFromSTC(devNum, get_STC_dsa_frw_src_port_trg_port(vss_lc_port(3),vss_lc_port(4)))
--"c0403001".."100002e0"
local dsa_tag_info_src_lc_port_8  = luaTgfDsaStringFromSTC(devNum, get_STC_dsa_frw_src_port_trg_port(vss_lc_port(2),vss_lc_port(4)))

-- ingress vss3_lc5 port 18 .. egress port 23
local case1_vss3_lc5_upstream = {
    deviceName      = devicesName_vss3_lc5,
    -- ingress info
    ingressInfo = {
        physicalPort    = vss_lc_port(3),
        dsaTag          = nil
    },
    -- egress info
    egressInfoArr = {
        {physicalPort = vss_lc_port(4) , dsaTag = dsa_tag_info_src_lc_port_18},
        {physicalPort = vss_lc_port(1), notEgress = true},
        {physicalPort = vss_lc_port(2), notEgress = true}
    }
}
-- eDsa tag that state 'ingress from dev=3 , port=5018' , trg = 'flood'
--"c3d03001".."80001fff".."80000138".."00000900"
local dsa_tag_cc5_src_eport_5018_trg_flood = luaTgfDsaStringFromSTC(devNum, get_STC_edsa_frw_src_eport_trg_flood(3 , global_test_data_env.ePort5018 , vss_lc_port(3) , 0))

local case1_vss3_cc5 = {
    deviceName      = devicesName_vss3_cc5,
    -- ingress info
    ingressInfo = {
        physicalPort    = vss_cc_port(4),
        dsaTag          = dsa_tag_info_src_lc_port_18
    },
    -- egress info : flooding
    egressInfoArr = {
        {physicalPort = vss_cc_port(3) , dsaTag = dsa_tag_cc5_src_eport_5018_trg_flood},
        {physicalPort = vss_cc_port(4) , dsaTag = dsa_tag_cc5_src_eport_5018_trg_flood},
        {physicalPort = vss_cc_port(1) , dsaTag = nil} ,
        {physicalPort = vss_cc_port(2) , dsaTag = nil} 
    }
}

-- different SRC-ID from the ingress DSA (srcid = 19 for 'orig' ingress port 18)
--"c3d03001".."81301fff".."80000138".."00000900"
local port18SrcId = vss_cc_port(2) + 1
local dsa_tag_cc5_src_eport_5018_trg_flood_to_vss_trunk = modify_SrcId_In_Dsa(devNum,dsa_tag_cc5_src_eport_5018_trg_flood,port18SrcId)
--luaTgfDsaStringFromSTC(devNum, get_STC_edsa_frw_src_eport_trg_flood(3 , 5018 , 18 , 19))

-- ingress vss3_lc5 port 23 .. flood , filter port 18
local case1_vss3_lc5_downstream = {
    deviceName      = devicesName_vss3_lc5,
    -- ingress info
    ingressInfo = {
        physicalPort    = vss_lc_port(4),
        dsaTag          = dsa_tag_cc5_src_eport_5018_trg_flood
    },
    -- egress info
    egressInfoArr = {
        {physicalPort = vss_lc_port(3) , notEgress = true},-- check that filtered
        {physicalPort = vss_lc_port(2) , dsaTag = nil},
        {physicalPort = vss_lc_port(1) , dsaTag = dsa_tag_cc5_src_eport_5018_trg_flood_to_vss_trunk} 
    }
}


-- ingress vss3_lc1 port 23 .. flood 
local case1_vss3_lc1_downstream = {
    deviceName      = devicesName_vss3_lc1,
    -- ingress info
    ingressInfo = {
        physicalPort    = vss_lc_port(4),
        dsaTag          = dsa_tag_cc5_src_eport_5018_trg_flood
    },
    -- egress info
    egressInfoArr = {
        {physicalPort = vss_lc_port(3) , notEgress = true},-- vss trunk port - flood sent from lc5 to this trunk
        {physicalPort = vss_lc_port(1) , dsaTag = nil} , -- port not configured ... get 'flood' in default vlan 1
        {physicalPort = vss_lc_port(2) , dsaTag = nil} -- trunk port -- may want to egress port in other device ('vss4')
    }
}

-- very similar to dsa_tag_cc5_src_eport_5018_trg_flood_to_vss_trunk but with different src-id (31) as the LC device modify it.
local dsa_tag_cc5_src_eport_5018_trg_flood_from_vss_trunk_on_vss4_lc1 = modify_SrcId_In_Dsa(devNum,dsa_tag_cc5_src_eport_5018_trg_flood_to_vss_trunk,src_id_for_vss_trunk)
--luaTgfDsaStringFromSTC(devNum, get_STC_edsa_frw_src_eport_trg_flood(3 , 5018 , 18 , src_id_for_vss_trunk))

-- ingress vss4_lc1 port 8 (from vss3_lc5).. to uplink 23
local case1_vss4_lc1_upstream = get_test_case_for_lc_uc_from_vss_trunk(devicesName_vss4_lc1,8,dsa_tag_cc5_src_eport_5018_trg_flood_to_vss_trunk) 

-- vss4 : in cc7 port 36 , egress ports 36
local case1_vss4_cc7 = get_test_case_for_vss4_cc7_flood(dsa_tag_cc5_src_eport_5018_trg_flood_from_vss_trunk_on_vss4_lc1)

-- vss4 : in lc1 port 23 , .. flood , filter : 0,8 - vss trunk , 18 - trunk member (flood in vss3-lc1) . egress ports 20
local case1_vss4_lc1_downstream = {
    deviceName      = devicesName_vss4_lc1,
    -- ingress info
    ingressInfo = {
        physicalPort    = vss_lc_port(4),
        dsaTag = dsa_tag_cc5_src_eport_5018_trg_flood_from_vss_trunk_on_vss4_lc1
    },
    -- egress info
    egressInfoArr = {
        {physicalPort = vss_lc_port(1) , notEgress = true},
        {physicalPort = vss_lc_port(2) , notEgress = true},
        {physicalPort = vss_lc_port(3) , notEgress = true},
        {physicalPort = vss_lc_port(5) , dsaTag = nil}
    }
}

-- this case called "Use Case #2c: L2 MC" in the VSS presentation
local test_case_1 = {
    name = "vss : station_5 flooding (5018 to flood) , use vss trunk on lc5 , use network trunk port on lc1 vss 3",
    macSa = station_5_macAddr,
    macDa = broadcast_macAddr,
    steps_of_packet = 
    {
        -- vss3 : in lc5 port 18 , egress port 23
        case1_vss3_lc5_upstream,
        -- vss3 : in cc5 port 54 , egress ports 36,54
        case1_vss3_cc5,
        -- ingress vss3_lc5 port 23 .. flood , filter port 18
        case1_vss3_lc5_downstream,
        -- ingress vss3_lc1 port 23 .. flood
        case1_vss3_lc1_downstream,
        -- ingress vss4_lc1 port 8 (from vss3_lc5).. to uplink 23
        case1_vss4_lc1_upstream,
        -- vss4 : in cc7 port 36 , egress ports 36
        case1_vss4_cc7,
        -- vss4 : in lc1 port 23 , .. flood , filter : 0,8 - vss trunk , 18 - trunk member (flood in vss3-lc1) . egress ports 20
        case1_vss4_lc1_downstream
    }
}

-- ingress vss3_lc5 port 8 .. egress port 23
local case2_vss3_lc1_upstream = {
    deviceName      = devicesName_vss3_lc1,
    -- ingress info
    ingressInfo = {
        physicalPort    = vss_lc_port(2),
        dsaTag          = nil
    },
    -- egress info
    egressInfoArr = {
        {physicalPort = vss_lc_port(4) , dsaTag = dsa_tag_info_src_lc_port_8},
        {physicalPort = vss_lc_port(1) , notEgress = true},
        {physicalPort = vss_lc_port(3) , notEgress = true}
    }
}


local cc5HwDevNum = 3
local cc7HwDevNum = 4
local eport_represent_trunk = global_test_data_env.ePort4500

local srcHwDevNum = cc5HwDevNum
local srcEPort = eport_represent_trunk
local origSrcPhyPort = 0 --??
local srcId = 0
local trgHwDevNum = cc5HwDevNum
local trgEport = global_test_data_env.ePort5018
local isTrgPhyPortValid  = true
local trgPhyPort = vss_cc_port(4) -- physical port on cc5
local isDsa8Bytes = false


-- eDsa tag that state 'ingress from dev=3 , port=4500' , trg = 'dev=3 , port=5018'
--"c3803001".."90000483".."a0000138".."0009c800"
local dsa_tag_cc5_src_eport_4500_trg_eport_5018 = luaTgfDsaStringFromSTC(devNum, 
        get_STC_edsa_frw_src_eport_trg_eport(srcHwDevNum , srcEPort , origSrcPhyPort , srcId , trgHwDevNum , trgEport , isTrgPhyPortValid , trgPhyPort, isDsa8Bytes))
                                            

local case2_vss3_cc5 = {
    deviceName      = devicesName_vss3_cc5,
    -- ingress info
    ingressInfo = {
        physicalPort    = vss_cc_port(3), dsaTag = dsa_tag_info_src_lc_port_8
    },
    -- egress info : uc
    egressInfoArr = {
        {physicalPort = vss_cc_port(4)  , dsaTag = dsa_tag_cc5_src_eport_4500_trg_eport_5018} 
    }
}
-- ingress vss3_lc5 port 23 .. egress single port 
local case2_vss3_lc5 = {
    deviceName      = devicesName_vss3_lc5,
    -- ingress info
    ingressInfo = {
        physicalPort    = vss_lc_port(4),
        dsaTag          = dsa_tag_cc5_src_eport_4500_trg_eport_5018
    },
    -- egress info
    egressInfoArr = {
        {physicalPort = vss_lc_port(3) , dsaTag = nil},
        {physicalPort = vss_lc_port(1)  , notEgress = true},
        {physicalPort = vss_lc_port(2)  , notEgress = true}
    }
}

local test_case_2 = {
    name = "vss : station_2 uc to station_5 (4500 to 5018) (IN vss3 lc1)",
    macSa = station_2_macAddr,
    macDa = station_5_macAddr,
    steps_of_packet = 
    {
        -- vss3 : in lc1 port 8 , egress port 23
        case2_vss3_lc1_upstream,
        -- vss3 : in cc5 port 36 , egress port 54
        case2_vss3_cc5,
        -- vss3 : in lc5 port 23 , egress port 18
        case2_vss3_lc5
    }
}

-- vss4 : in lc1 port 18 , egress port 23
local case3_vss4_lc1_upstream = {
    deviceName      = devicesName_vss4_lc1,
    -- ingress info
    ingressInfo = {
        physicalPort    = vss_lc_port(3),
        dsaTag          = nil
    },
    -- egress info
    egressInfoArr = {
        {physicalPort = vss_lc_port(4) , dsaTag = dsa_tag_info_src_lc_port_18},
        {physicalPort = vss_lc_port(1) , notEgress = true},
        {physicalPort = vss_lc_port(2) , notEgress = true},
        {physicalPort = vss_lc_port(5) , notEgress = true}
    }
}

-- vss4 : in cc7 port 36 , egress port 36
local case3_vss4_cc7 = get_test_case_for_vss4_cc7_uc(dsa_tag_info_src_lc_port_18)

local srcHwDevNum = cc7HwDevNum
local srcEPort = eport_represent_trunk
local origSrcPhyPort = 0 --??
local srcId = 0
local trgHwDevNum = cc5HwDevNum
local trgEport = global_test_data_env.ePort5018
local isTrgPhyPortValid  = true
local trgPhyPort = vss_cc_port(3) -- physical port on cc7
local isDsa8Bytes = false

-- eDsa tag that state 'ingress from dev=3 , port=4500' , trg = 'dev=3 , port=5018'
--"c3803001".."90000483".."a0000138".."0009c800"
local dsa_tag_cc7_src_eport_4500_trg_eport_5018_on_cc5 = luaTgfDsaStringFromSTC(devNum, 
        get_STC_edsa_frw_src_eport_trg_eport(srcHwDevNum , srcEPort , origSrcPhyPort , srcId , trgHwDevNum , trgEport , isTrgPhyPortValid , trgPhyPort, isDsa8Bytes))


-- vss4 : in lc1 port 23 , egress port 8 (vss trunk member)
local case3_vss4_lc1_downstream = {
    deviceName      = devicesName_vss4_lc1,
    -- ingress info
    ingressInfo = {
        physicalPort    = vss_lc_port(4),
        dsaTag          = dsa_tag_cc7_src_eport_4500_trg_eport_5018_on_cc5
    },
    -- egress info
    egressInfoArr = {
        {physicalPort = vss_lc_port(2)  , dsaTag = dsa_tag_cc7_src_eport_4500_trg_eport_5018_on_cc5},--unmodified DSA
        {physicalPort = vss_lc_port(1)  , notEgress = true},
        {physicalPort = vss_lc_port(3)  , notEgress = true},
        {physicalPort = vss_lc_port(5)  , notEgress = true}
    }
}

-- like dsa_tag_cc7_src_eport_4500_trg_eport_5018_on_cc5 , with different srcid
local dsa_tag_cc7_src_eport_4500_trg_eport_5018_on_cc5_srcId_of_vss_trunk = 
            modify_SrcId_In_Dsa(devNum,dsa_tag_cc7_src_eport_4500_trg_eport_5018_on_cc5,src_id_for_vss_trunk)

-- vss3 : in lc5 port 0 , egress port 23
local case3_vss3_lc5_upstream = get_test_case_for_lc_uc_from_vss_trunk(devicesName_vss3_lc5,vss_lc_port(1),dsa_tag_cc7_src_eport_4500_trg_eport_5018_on_cc5) 

-- vss3 : in cc7 port 54 , egress port 54
local case3_vss3_cc5 = get_test_case_for_vss3_cc5_uc_return_to_src_phy_port(vss_cc_port(4),dsa_tag_cc7_src_eport_4500_trg_eport_5018_on_cc5_srcId_of_vss_trunk) 

-- vss3 : in lc5 port 23 , egress port 18
local case3_vss3_lc5_downstream = {
    deviceName      = devicesName_vss3_lc5,
    -- ingress info
    ingressInfo = {
        physicalPort    = vss_lc_port(4),
        dsaTag = dsa_tag_cc7_src_eport_4500_trg_eport_5018_on_cc5_srcId_of_vss_trunk
    },
    -- egress info
    egressInfoArr = {
        {physicalPort = vss_lc_port(3) , dsaTag = nil},
        {physicalPort = vss_lc_port(1) , notEgress = true},
        {physicalPort = vss_lc_port(2) , notEgress = true}
    }
}

local test_case_3 = {
    name = "vss : station_2 uc to station_5 (4500 to 5018)(IN vss4 lc1)",
    macSa = station_2_macAddr,
    macDa = station_5_macAddr,
    steps_of_packet = 
    {
        -- vss4 : in lc1 port 18 , egress port 23
        case3_vss4_lc1_upstream,
        -- vss4 : in cc7 port 36 , egress port 36
        case3_vss4_cc7,
        -- vss4 : in lc1 port 23 , egress port 8 (vss trunk member)
        case3_vss4_lc1_downstream,
        
        -- vss3 : in lc5 port 0 , egress port 23
        case3_vss3_lc5_upstream,
        -- vss3 : in cc7 port 54 , egress port 54
        case3_vss3_cc5,
        -- vss3 : in lc5 port 23 , egress port 18
        case3_vss3_lc5_downstream
    }
}
-- vss3 : in lc5 port 18 , egress port 23
local case4_vss3_lc5_upstream = case1_vss3_lc5_upstream

-- mac address in format of 'cli command'
local station_2_macAddr_for_cli_command = convertMacStringToMacCommandFormat(station_2_macAddr)

local srcHwDevNum = cc5HwDevNum
local srcEPort = global_test_data_env.ePort5018
local origSrcPhyPort = 0 --??
local srcId = 0
local trgHwDevNum = cc7HwDevNum
local trgEport = global_test_data_env.ePort1018-- one of the eports of 4500 : {3,1008} , {4,1018}
local isTrgPhyPortValid  = false -- we egress 'remote device'
local trgPhyPort = vss_lc_port(4) -- still this field exists in the dsa
local isDsa8Bytes = false

local dsa_tag_cc5_src_eport_5018_trg_4_1018_represent_4500_STC = get_STC_edsa_frw_src_eport_trg_eport(srcHwDevNum , srcEPort , origSrcPhyPort , srcId , trgHwDevNum , trgEport , isTrgPhyPortValid , trgPhyPort, isDsa8Bytes)
local dsa_tag_cc5_src_eport_5018_trg_4_1018_represent_4500 = luaTgfDsaStringFromSTC(devNum,dsa_tag_cc5_src_eport_5018_trg_4_1018_represent_4500_STC)

local trgEport = global_test_data_env.ePort1008-- one of the eports of 4500 : {3,1008} , {4,1018}
local trgHwDevNum = cc5HwDevNum
local isTrgPhyPortValid  = true -- we egress 'local device'
local trgPhyPort = vss_cc_port(3) 
local dsa_tag_cc5_src_eport_5018_trg_3_1008_represent_4500_STC = get_STC_edsa_frw_src_eport_trg_eport(srcHwDevNum , srcEPort , origSrcPhyPort , srcId , trgHwDevNum , trgEport , isTrgPhyPortValid , trgPhyPort, isDsa8Bytes)
local dsa_tag_cc5_src_eport_5018_trg_3_1008_represent_4500 = luaTgfDsaStringFromSTC(devNum,dsa_tag_cc5_src_eport_5018_trg_3_1008_represent_4500_STC)

local mac_addr_static_line = "mac address-table static " .. station_2_macAddr_for_cli_command .. " device ${dev} vlan 1 eport ${dev}/" .. eport_represent_trunk
-- commands that allow to add fdb entry about 'station_2' on eport of the trunk
local cli_commands_add_fdb_entry_station_2_on_tunk_eport = 
[[

    do configure
    
]] ..
    mac_addr_static_line
   ..
[[

    exit
]]

        
-- vss3 : in cc5 port 54 , egress ports 36
local case4_vss3_cc5 = {
    deviceName      = devicesName_vss3_cc5,
    -- add the mac DA to FDB prior to running the test
    runtime_commands_arr = cli_commands_add_fdb_entry_station_2_on_tunk_eport,
    
    -- ingress info
    ingressInfo = {
        physicalPort    = vss_cc_port(4),
        dsaTag          = dsa_tag_info_src_lc_port_18
    },
    -- egress info : single port
    egressInfoArr = {
        {physicalPort = vss_cc_port(3) , dsaTag = dsa_tag_cc5_src_eport_5018_trg_4_1018_represent_4500},
        {physicalPort = vss_cc_port(1)  , notEgress = true} ,
        {physicalPort = vss_cc_port(2) , notEgress = true} 
    }
}

-- ingress vss3_lc1 port 23 , egress port 18 (direct to device 4 on vss trunk)
local case4_vss3_lc1_downstream = {
    deviceName      = devicesName_vss3_lc1,
    -- ingress info
    ingressInfo = {
        physicalPort    = vss_lc_port(4),
        dsaTag = dsa_tag_cc5_src_eport_5018_trg_4_1018_represent_4500
    },
    -- egress info
    egressInfoArr = {
        {physicalPort = vss_lc_port(3) , dsaTag = dsa_tag_cc5_src_eport_5018_trg_4_1018_represent_4500},
        {physicalPort = vss_lc_port(1) , notEgress = true},
        {physicalPort = vss_lc_port(2) , notEgress = true}
    }
}

-- very similar to dsa_tag_cc5_src_eport_5018_trg_flood_to_vss_trunk but with different src-id (31) as the LC device modify it.
local dsa_tag_cc5_src_eport_5018_trg_4_1018_represent_4500_from_vss_trunk_on_vss4_lc1 = modify_SrcId_In_Dsa(devNum,dsa_tag_cc5_src_eport_5018_trg_4_1018_represent_4500,src_id_for_vss_trunk)

-- ingress vss4_lc1 port 0 , egress port 23
local case4_vss4_lc1_upstream = get_test_case_for_lc_uc_from_vss_trunk(devicesName_vss4_lc1,0,dsa_tag_cc5_src_eport_5018_trg_4_1018_represent_4500)

-- vss4 : in cc7 port 36 , egress ports 36
local case4_vss4_cc7 = get_test_case_for_vss4_cc7_uc(dsa_tag_cc5_src_eport_5018_trg_4_1018_represent_4500_from_vss_trunk_on_vss4_lc1)

-- vss4 : in lc1 port 23 , egress port 18 (the eport {4,1018} , member of trunk {4500} )
local case4_vss4_lc1_downstream = get_test_case_for_lc_uc_uplink_to_network(devicesName_vss4_lc1,vss_lc_port(3),dsa_tag_cc5_src_eport_5018_trg_4_1018_represent_4500_from_vss_trunk_on_vss4_lc1) 

-- this case called "Use Case #1a: L2/3 UC to LAG" in the VSS presentation
local test_case_4 = {
    name = "vss : station_5 uc to station_2 (5018 to 4500) choose {4,1008} via vss trunk lc1 port 18",
    macSa = station_5_macAddr_case_4,
    macDa = station_2_macAddr,
    steps_of_packet = 
    {
        -- vss3 : in lc5 port 18 , egress port 23
        case4_vss3_lc5_upstream,
        -- vss3 : in cc5 port 54 , egress ports 36
        case4_vss3_cc5,
        -- ingress vss3_lc1 port 23 , egress port 18 (direct to device 4 on vss trunk)
        case4_vss3_lc1_downstream,
        -- ingress vss4_lc1 port 0 , egress port 23
        case4_vss4_lc1_upstream,
        -- vss4 : in cc7 port 36 , egress ports 36
        case4_vss4_cc7,
        -- vss4 : in lc1 port 23 , egress port 18 (the eport {4,1018} , member of trunk {4500} )
        case4_vss4_lc1_downstream
    }
}

-- vss3 : in lc5 port 18 , egress port 23
local case5_vss3_lc5_upstream = case4_vss3_lc5_upstream

-- vss3 : in cc5 port 54 , egress ports 54
local case5_vss3_cc5 = get_test_case_for_vss3_cc5_uc_return_to_src_phy_port(vss_cc_port(4),dsa_tag_info_src_lc_port_18) 
-- add the mac DA to FDB prior to running the test
case5_vss3_cc5.runtime_commands_arr = cli_commands_add_fdb_entry_station_2_on_tunk_eport

-- ingress vss3_lc5 port 23 , egress port 0 (direct to device 4 on vss trunk)
local case5_vss3_lc5_downstream = {
    deviceName      = devicesName_vss3_lc5,
    -- ingress info
    ingressInfo = {
        physicalPort    = vss_lc_port(4),
        dsaTag = dsa_tag_cc5_src_eport_5018_trg_4_1018_represent_4500
    },
    -- egress info
    egressInfoArr = {
        {physicalPort =  vss_lc_port(1), dsaTag = dsa_tag_cc5_src_eport_5018_trg_4_1018_represent_4500},
        {physicalPort =  vss_lc_port(3), notEgress = true},
        {physicalPort =  vss_lc_port(2), notEgress = true}
    }
}

-- ingress vss4_lc1 port 8 , egress port 23
local case5_vss4_lc1_upstream = get_test_case_for_lc_uc_from_vss_trunk(devicesName_vss4_lc1,vss_lc_port(2),dsa_tag_cc5_src_eport_5018_trg_4_1018_represent_4500)

-- vss4 : in cc7 port 36 , egress ports 36
local case5_vss4_cc7 = case4_vss4_cc7

-- vss4 : in lc1 port 23 , egress port 18 (the eport {4,1018} , member of trunk {4500} )
local case5_vss4_lc1_downstream = case4_vss4_lc1_downstream

-- this case called "Use Case #1b: L2/3 UC to LAG" in the VSS presentation
local test_case_5 = {
    name = "vss : station_5 uc to station_2 (5018 to 4500) choose {4,1008} via vss trunk lc5 port 0",
    macSa = station_5_macAddr_case_5,
    macDa = station_2_macAddr,
    steps_of_packet = 
    {
        -- vss3 : in lc5 port 18 , egress port 23
        case5_vss3_lc5_upstream,
        -- vss3 : in cc5 port 54 , egress ports 54
        case5_vss3_cc5,
        -- ingress vss3_lc5 port 23 , egress port 0 (direct to device 4 on vss trunk)
        case5_vss3_lc5_downstream,
        -- ingress vss4_lc1 port 8 , egress port 23
        case5_vss4_lc1_upstream,
        -- vss4 : in cc7 port 36 , egress ports 36
        case5_vss4_cc7,
        -- vss4 : in lc1 port 23 , egress port 18 (the eport {4,1018} , member of trunk {4500} )
        case5_vss4_lc1_downstream
    }
}

-- vss3 : in lc5 port 18 , egress port 23
local case6_vss3_lc5_upstream = case5_vss3_lc5_upstream

-- vss3 : in cc5 port 54 , egress ports 36
local case6_vss3_cc5 = {
    deviceName      = devicesName_vss3_cc5,
    -- add the mac DA to FDB prior to running the test
    runtime_commands_arr = cli_commands_add_fdb_entry_station_2_on_tunk_eport,
    
    -- ingress info
    ingressInfo = {
        physicalPort    = vss_cc_port(4),
        dsaTag          = dsa_tag_info_src_lc_port_18
    },
    -- egress info : single port
    egressInfoArr = {
        {physicalPort = vss_cc_port(3) , dsaTag = dsa_tag_cc5_src_eport_5018_trg_3_1008_represent_4500},
        {physicalPort = vss_cc_port(1) , notEgress = true} ,
        {physicalPort = vss_cc_port(2) , notEgress = true} 
    }
}
-- vss3 : in lc1 port 23 , egress port 8 (the eport {3,1008} , member of trunk {4500} )
local case6_vss3_lc1_downstream = get_test_case_for_lc_uc_uplink_to_network(devicesName_vss3_lc1,vss_lc_port(2),dsa_tag_cc5_src_eport_5018_trg_3_1008_represent_4500) 

-- this case called "Use Case #1c: L2/3 UC to LAG" in the VSS presentation  
local test_case_6 = {
    name = "vss : station_5 uc to station_2 (5018 to 4500) choose {3,1008}",
    macSa = station_5_macAddr_case_6,
    macDa = station_2_macAddr,
    steps_of_packet = 
    {
        -- vss3 : in lc5 port 18 , egress port 23
        case6_vss3_lc5_upstream,
        -- vss3 : in cc5 port 54 , egress ports 36
        case6_vss3_cc5,
        -- vss3 : in lc1 port 23 , egress port 8 (the eport {3,1008} , member of trunk {4500} )
        case6_vss3_lc1_downstream
    }
}

-- vss3 : in lc5 port 18 , egress port 23
local case7_vss3_lc5_upstream = case1_vss3_lc5_upstream
-- vss3 : in cc5 port 54 , egress ports 36,54
local case7_vss3_cc5 = case1_vss3_cc5
-- ingress vss3_lc5 port 23 .. flood , filter port 18 , filter port 0 (vss trunk port)
local case7_vss3_lc5_downstream = {
    deviceName      = devicesName_vss3_lc5,
    -- ingress info
    ingressInfo = {
        physicalPort    = vss_lc_port(4),
        dsaTag          = dsa_tag_cc5_src_eport_5018_trg_flood
    },
    -- egress info
    egressInfoArr = {
        {physicalPort = vss_lc_port(3) , notEgress = true},-- check that filtered
        {physicalPort = vss_lc_port(2) , dsaTag = nil},
        {physicalPort = vss_lc_port(1) , notEgress = true} -- choose vss trunk on other device
    }
}

-- ingress vss3_lc1 port 23 .. flood , filter network trunk
local case7_vss3_lc1_downstream = {
    deviceName      = devicesName_vss3_lc1,
    -- ingress info
    ingressInfo = {
        physicalPort    = vss_lc_port(4),
        dsaTag          = dsa_tag_cc5_src_eport_5018_trg_flood
    },
    -- egress info
    egressInfoArr = {
        {physicalPort = vss_lc_port(3) , dsaTag = dsa_tag_cc5_src_eport_5018_trg_flood},-- vss trunk port - flood sent from lc5 to this trunk
        {physicalPort = vss_lc_port(1) , dsaTag = nil} , -- port not configured ... get 'flood' in default vlan 1
        {physicalPort = vss_lc_port(2) , notEgress = true} -- trunk port -- use other device on vss4
    }
}

-- ingress vss4_lc1 port 0 (from vss3_lc1).. to uplink 23
local case7_vss4_lc1_upstream = get_test_case_for_lc_uc_from_vss_trunk(devicesName_vss4_lc1,0,dsa_tag_cc5_src_eport_5018_trg_flood) 

-- vss4 : in cc7 port 36 , egress ports 36
local case7_vss4_cc7 = case1_vss4_cc7

-- vss4 : in lc1 port 23 , .. flood , filter : 0,8 - vss trunk , 18 - trunk member (flood in vss3-lc1) . egress ports 20
local case7_vss4_lc1_downstream = {
    deviceName      = devicesName_vss4_lc1,
    -- ingress info
    ingressInfo = {
        physicalPort    = vss_lc_port(4),
        dsaTag = dsa_tag_cc5_src_eport_5018_trg_flood_from_vss_trunk_on_vss4_lc1
    },
    -- egress info
    egressInfoArr = {
        {physicalPort = vss_lc_port(1) , notEgress = true},
        {physicalPort = vss_lc_port(2) , notEgress = true},
        {physicalPort = vss_lc_port(3) , dsaTag = nil},-- trunk port -- flood from here
        {physicalPort = vss_lc_port(5) , dsaTag = nil}
    }
}

-- this case called "Use Case #2a: L2 MC" in the VSS presentation
local test_case_7 = {
    name = "vss : station_5 flooding (5018 to flood) , use vss trunk on lc1 , use network trunk port on lc1 vss 4",
    macSa = station_5_macAddr_case_7,
    macDa = broadcast_macAddr,
    steps_of_packet = 
    {
        -- vss3 : in lc5 port 18 , egress port 23
        case7_vss3_lc5_upstream,
        -- vss3 : in cc5 port 54 , egress ports 36,54
        case7_vss3_cc5,
        -- ingress vss3_lc5 port 23 .. flood , filter port 18 , filter port 0 (vss trunk port)
        case7_vss3_lc5_downstream,
        -- ingress vss3_lc1 port 23 .. flood , filter network trunk
        case7_vss3_lc1_downstream,
        -- ingress vss4_lc1 port 0 (from vss3_lc1).. to uplink 23
        case7_vss4_lc1_upstream,
        -- vss4 : in cc7 port 36 , egress ports 36
        case7_vss4_cc7,
        -- vss4 : in lc1 port 23 , .. flood , filter : 0,8 - vss trunk . egress ports 20 , flood network trunk here
        case7_vss4_lc1_downstream
    }
}
--[[
-- vss3 : in lc5 port 18 , egress port 23
local case8_vss3_lc5_upstream = case7_vss3_lc5_upstream
-- vss3 : in cc5 port 54 , egress ports 36,54
local case8_vss3_cc5 = case7_vss3_cc5
-- ingress vss3_lc5 port 23 .. flood , filter port 18 , filter port 0 (vss trunk port)
local case8_vss3_lc5_downstream = case7_vss3_lc5_downstream
-- ingress vss3_lc1 port 23 .. flood - flood network trunk here
local case8_vss3_lc1_downstream = {
    deviceName      = devicesName_vss3_lc1,
    -- ingress info
    ingressInfo = {
        physicalPort    = 23,
        dsaTag          = dsa_tag_cc5_src_eport_5018_trg_flood
    },
    -- egress info
    egressInfoArr = {
        {physicalPort = 18 , dsaTag = dsa_tag_cc5_src_eport_5018_trg_flood},-- vss trunk port - flood sent from lc5 to this trunk
        {physicalPort = 0  , dsaTag = nil} , -- port not configured ... get 'flood' in default vlan 1
        {physicalPort = 8  , dsaTag = nil} -- trunk port -- flood from here
    }
}

-- ingress vss4_lc1 port 0 (from vss3_lc1).. to uplink 23
local case8_vss4_lc1_upstream = case7_vss4_lc1_upstream

-- vss4 : in cc7 port 36 , egress ports 36
local case8_vss4_cc7 = case7_vss4_cc7
-- vss4 : in lc1 port 23 , .. flood , filter : 0,8 - vss trunk , 18 - trunk member (flood in vss3-lc1) . egress ports 20
local case8_vss4_lc1_downstream = case1_vss4_lc1_downstream

-- this case called "Use Case #2b: L2 MC" in the VSS presentation
local test_case_8 = {
    name = "vss : station_5 flooding (5018 to flood) , use vss trunk on lc1 , use network trunk port on lc1 vss 3",
    macSa = station_5_macAddr_case_8,
    macDa = broadcast_macAddr,
    steps_of_packet = 
    {
        -- vss3 : in lc5 port 18 , egress port 23
        case8_vss3_lc5_upstream,
        -- vss3 : in cc5 port 54 , egress ports 36,54
        case8_vss3_cc5,
        -- ingress vss3_lc5 port 23 .. flood , filter port 18 , filter port 0 (vss trunk port)
        case8_vss3_lc5_downstream,
        -- ingress vss3_lc1 port 23 .. flood - flood network trunk here
        case8_vss3_lc1_downstream,
        -- ingress vss4_lc1 port 0 (from vss3_lc1).. to uplink 23
        case8_vss4_lc1_upstream,
        -- vss4 : in cc7 port 36 , egress ports 36
        case8_vss4_cc7,
        -- vss4 : in lc1 port 23 , .. flood , filter : 0,8 - vss trunk , 18 - trunk member (flood in vss3-lc1) . egress ports 20 
        case8_vss4_lc1_downstream
    }
}
--]]

-- vss3 : in lc1 port 8 , egress port 23
local case9_vss3_lc1_upstream = case2_vss3_lc1_upstream

-- eDsa tag that state 'ingress from dev=3 , port=4500' , trg = 'flood'
local dsa_tag_cc5_src_eport_4500_trg_flood = luaTgfDsaStringFromSTC(devNum, get_STC_edsa_frw_src_eport_trg_flood(3 , global_test_data_env.ePort4500 , vss_lc_port(2) , 0))

-- vss3 : in cc5 port 36 , egress ports 36,54
local case9_vss3_cc5 = {
    deviceName      = devicesName_vss3_cc5,
    -- ingress info
    ingressInfo = {
        physicalPort    = vss_cc_port(3),
        dsaTag          = dsa_tag_info_src_lc_port_8
    },
    -- egress info : flooding
    egressInfoArr = {
        {physicalPort = vss_cc_port(3) , dsaTag = dsa_tag_cc5_src_eport_4500_trg_flood},
        {physicalPort = vss_cc_port(4) , dsaTag = dsa_tag_cc5_src_eport_4500_trg_flood},
        {physicalPort = vss_cc_port(1) , dsaTag = nil} ,
        {physicalPort = vss_cc_port(2) , dsaTag = nil} 
    }
}

-- different SRC-ID from the ingress DSA (srcid = 0 because no member of the network trunk in this device)
local dsa_tag_cc5_src_eport_4500_trg_flood_to_vss_trunk = modify_SrcId_In_Dsa(devNum,dsa_tag_cc5_src_eport_4500_trg_flood,0)

-- ingress vss3_lc5 port 23 .. flood , flood vss trunk here (port 0)
local case9_vss3_lc5_downstream = {
    deviceName      = devicesName_vss3_lc5,
    -- ingress info
    ingressInfo = {
        physicalPort    = vss_lc_port(4),
        dsaTag          = dsa_tag_cc5_src_eport_4500_trg_flood
    },
    -- egress info
    egressInfoArr = {
        {physicalPort = vss_lc_port(3) , dsaTag = nil},
        {physicalPort = vss_lc_port(2) , dsaTag = nil},
        {physicalPort = vss_lc_port(1) , dsaTag = dsa_tag_cc5_src_eport_4500_trg_flood_to_vss_trunk} -- flood vss trunk here
    }
}

-- ingress vss3_lc1 port 23 .. flood , filter network trunk 
local case9_vss3_lc1_downstream = {
    deviceName      = devicesName_vss3_lc1,
    -- ingress info
    ingressInfo = {
        physicalPort    = vss_lc_port(4),
        dsaTag          = dsa_tag_cc5_src_eport_4500_trg_flood
    },
    -- egress info
    egressInfoArr = {
        {physicalPort = vss_lc_port(3) , notEgress = true},-- vss trunk port - flood sent from lc5 to this trunk
        {physicalPort = vss_lc_port(1) , dsaTag = nil} , -- port not configured ... get 'flood' in default vlan 1
        {physicalPort = vss_lc_port(2) , notEgress = true} -- filter network trunk
    }
}

-- ingress vss4_lc1 port 8 (from vss3_lc5).. to uplink 23
local case9_vss4_lc1_upstream = get_test_case_for_lc_uc_from_vss_trunk(devicesName_vss4_lc1,vss_lc_port(2),dsa_tag_cc5_src_eport_4500_trg_flood_to_vss_trunk) 

-- very similar to dsa_tag_cc5_src_eport_4500_trg_flood_to_vss_trunk but with different src-id (31) as the LC device modify it.
local dsa_tag_cc5_src_eport_4500_trg_flood_from_vss_trunk_on_vss4_lc1 = modify_SrcId_In_Dsa(devNum,dsa_tag_cc5_src_eport_4500_trg_flood_to_vss_trunk,src_id_for_vss_trunk)

-- vss4 : in cc7 port 36 , egress ports 36
local case9_vss4_cc7 = get_test_case_for_vss4_cc7_flood(dsa_tag_cc5_src_eport_4500_trg_flood_from_vss_trunk_on_vss4_lc1)

-- vss4 : in lc1 port 23 , .. flood , filter : 0,8 - vss trunk , 18 - trunk member (src trunk member) . egress ports 20 
local case9_vss4_lc1_downstream = {
    deviceName      = devicesName_vss4_lc1,
    -- ingress info
    ingressInfo = {
        physicalPort    = vss_lc_port(4),
        dsaTag = dsa_tag_cc5_src_eport_4500_trg_flood_from_vss_trunk_on_vss4_lc1
    },
    -- egress info
    egressInfoArr = {
        {physicalPort = vss_lc_port(1) , notEgress = true},
        {physicalPort = vss_lc_port(2) , notEgress = true},
        {physicalPort = vss_lc_port(3) , notEgress = true},
        {physicalPort = vss_lc_port(5) , dsaTag = nil}
    }
}

-- this case called "Use Case #2d: L2 MC (from LAG)" in the VSS presentation
local test_case_9 = {
    name = "vss : station_2 flooding (4500 to flood) , use vss trunk on lc5",
    macSa = station_2_macAddr_case_9,
    macDa = broadcast_macAddr,
    steps_of_packet = 
    {
        -- vss3 : in lc1 port 8 , egress port 23
        case9_vss3_lc1_upstream,
        -- vss3 : in cc5 port 36 , egress ports 36,54
        case9_vss3_cc5,
        -- ingress vss3_lc5 port 23 .. flood , flood vss trunk here (port 0)
        case9_vss3_lc5_downstream,
        -- ingress vss3_lc1 port 23 .. flood , filter network trunk 
        case9_vss3_lc1_downstream,
        -- ingress vss4_lc1 port 8 (from vss3_lc5).. to uplink 23
        case9_vss4_lc1_upstream,
        -- vss4 : in cc7 port 36 , egress ports 36
        case9_vss4_cc7,
        -- vss4 : in lc1 port 23 , .. flood , filter : 0,8 - vss trunk , 18 - trunk member (flood in vss3-lc1) . egress ports 20 
        case9_vss4_lc1_downstream
    }
}


-- build packet with dsa
local function build_full_packet(macDa,macSa,dsa)
    if dsa then
        return macDa..macSa..dsa..packetPayload
    else
        return macDa..macSa..packetPayload
    end
end

-- run vss single step of test 
local function run_test_step(currentStep,macDa,macSa,testName)
    local ingressInfo = currentStep.ingressInfo
    local egressInfoArr = currentStep.egressInfoArr

    local ingrPacket = build_full_packet(macDa,macSa,currentStep.ingressInfo.dsaTag)
    local transmitInfo = {portNum = ingressInfo.physicalPort , pktInfo = {fullPacket = ingrPacket} }

    local egressInfoTable = {}-- filled in the next loop
    local egressPortsNumbersString = ""
    local notEgressPortsNumbersString = ""
    
    for ii=1,#egressInfoArr do
        local currEgrInfo = egressInfoArr[ii]
        
        local newEgressEntry = {portNum = currEgrInfo.physicalPort}
        
        if currEgrInfo.notEgress == true then
            -- port stated to not egress the packet
            newEgressEntry.packetCount = 0
            notEgressPortsNumbersString = notEgressPortsNumbersString .. to_string(currEgrInfo.physicalPort) .. " "
        else
            -- port need to egress next packet
            if newEgressEntry.portNum == transmitInfo.portNum then
                -- expect loopback storming (at least 2 packets)
                transmitInfo.loopbackStorming = true
            else
                local egrPacket = build_full_packet(macDa,macSa,currEgrInfo.dsaTag)
            
                newEgressEntry.pktInfo = {fullPacket = egrPacket}
            end
            
            egressPortsNumbersString = egressPortsNumbersString .. to_string(currEgrInfo.physicalPort) .. " "
            
            
        end
        
        -- add new entry to the table (as last)
        egressInfoTable[ii]  = newEgressEntry
    end

    local traffic_check
    if debug_specific_test_Id ~= nil and debug_specific_test_Id ~= current_packet_id then
        -- debug stated specific packet to check ... and it is not current packet
        traffic_check = false
    else
        traffic_check = true
    end
    
    -- check if the device need the mac DA to be in the FDB ... prior to sending the traffic
    if currentStep.runtime_commands_arr then
        executeStringCliCommands(currentStep.runtime_commands_arr)
    end
    
    
    if traffic_check then
        rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
        luaTgfCheckTestResults(rc,  "(packet#" ..current_packet_id..")" .. testName .. " in device " .. currentStep.deviceName .. " packet from port " .. transmitInfo.portNum .. 
                " should egress ports : " .. egressPortsNumbersString .. ". should not egress : " .. notEgressPortsNumbersString)
    end

    -- increment the Id of the 'tested case'
    current_packet_id = current_packet_id + 1
            
end

-- run vss tests related to the tested device -- check if relevant
local function run_test(test_case,testedDeviceName)
    local steps_of_packet = test_case.steps_of_packet
    local found = false
    -- check if test relevant to tested device
    for ii = 1,#steps_of_packet do
        if steps_of_packet[ii].deviceName == testedDeviceName then
            found = true
            break
        end
    end
    
    if found == false then
        _debug_to_string("test: " .. test_case.name .. " not relevant for device: " .. testedDeviceName)
        return
    end
    
    _debug_to_string("start test: " .. test_case.name .. " for device: " , testedDeviceName)

    for ii = 1,#steps_of_packet do
        if steps_of_packet[ii].deviceName == testedDeviceName then
            run_test_step(steps_of_packet[ii],test_case.macDa,test_case.macSa,test_case.name)
        end
    end
    
    _debug_to_string("end test: " .. test_case.name .. " for device: " , testedDeviceName)
end

local test_cases = {

    -- this case called "Use Case #2c: L2 MC" in the VSS presentation
    test_case_1,
    test_case_2,
    test_case_3,
    -- this case called "Use Case #1a: L2/3 UC to LAG" in the VSS presentation
    test_case_4,
    -- this case called "Use Case #1b: L2/3 UC to LAG" in the VSS presentation  
    test_case_5,
    -- this case called "Use Case #1c: L2/3 UC to LAG" in the VSS presentation  
    test_case_6,
    -- this case called "Use Case #2a: L2 MC" in the VSS presentation
    test_case_7,

    -- this case called "Use Case #2b: L2 MC" in the VSS presentation
    -- !!! test_case_8, -- can not achieve this in CPSS with 2 members in the 2 trunks !!!!

    -- this case called "Use Case #2d: L2 MC (from LAG)" in the VSS presentation
    test_case_9,
}

-- run vss tests related to the tested device
function util_vss_run_tests(testedDeviceName)
    -- initialize the counter
    current_packet_id = 1
    -- loop on test cases , select those that relevant
    for ii = 1,#test_cases do
        run_test(test_cases[ii],testedDeviceName)
    end
end

