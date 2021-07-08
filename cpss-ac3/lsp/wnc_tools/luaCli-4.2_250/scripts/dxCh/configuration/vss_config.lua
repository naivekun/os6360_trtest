--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* vss_config.lua
--*
--* DESCRIPTION:
--*     'config' commands for the 'vss of bc2-ac3' system
--*     according to doc:
--*     http://docil.marvell.com/webtop/drl/objectId/0900dd88801a06b3
--*       cc – Control Card device --  Bc2
--*       lc – line-card device - Ac3
--*	    Native traffic received on AC3 LC are sent with 8B Extended DSA to BC2
--*	    BC2 transmits 16B eDSA packets to AC3 line cards
--*	    Cross platform VSS traffic
--*	    AC3 passes eDSA packets transparently over the VSS interface to remote platform
--*	    AC3 passes eDSA packets received over VSS to BC2
--*	    BC2 device learns MAC address on remote platform based on eDSA <Src ePort>
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

-- global DB to use
-- vss_internal_db_global;

cmdLuaCLI_registerCfunction("wrlDxChPortIsValidEport")
cmdLuaCLI_registerCfunction("wrlCpssSetBmpPort")
-- debug utility
local debug_on = false
local function _debug(...)
    if debug_on == true then
        printLog (...)
    end
end
-- extern function _debug_to_string(...)

-- add error to command_data
local function add_error(command_data,errorStr,...)
    command_data:setFailStatus()
    command_data:setFailLocalStatus()
    command_data:addError(errorStr,...)
end

-- check if error set by 'add_error'
-- return 'true' if 'error'
-- return 'false' if 'no error'
local function is_error(command_data)
    -- command_data["status"] == true  --> meaning 'no error'
    -- command_data["status"] == false --> meaning 'error'
    return not (true == command_data["status"])
end


-- build mask for 'num of bits' from 'start bit'
local function mask_of_bits(startBit,numBits)
    local mask = bit_shl(1,numBits) - 1

    local new_mask = bit_shl(mask , startBit)

    return new_mask
end

local _3k = (3 * 1024)
local on_cc_pcl_id = 0
local on_lc_common_hwDevNum = 0
local vss_dummy_hw_dev_num_for_other_devices = "vss_dummy_hwDevNum_for_other_devices"

-- function to change needed HW devNum in e2phy
local function on_cc_set_e2phy_low_entries(command_data,devNum,hwDevNum,unset)
    local physicalInfo = {}
    physicalInfo.type = "CPSS_INTERFACE_PORT_E"
    physicalInfo.devPort = {}
    local portNum
    local apiName
    local did_error , result , OUT_values
    
    -- get list of physical ports with 'mac'
    local port_list = get_port_list(devNum)

    --_debug_to_string("port_list=",port_list)

    for index, entry in pairs(port_list) do
        portNum = entry
        --***************
        --***************
        apiName = "cpssDxChBrgEportToPhysicalPortTargetMappingTableGet"
        did_error , result , OUT_values = genericCpssApiWithErrorHandler(command_data,
            apiName, {
            { "IN", "GT_U8", "devNum", devNum},
            { "IN", "GT_PORT_NUM", "portNum", portNum},
            { "OUT", "CPSS_INTERFACE_INFO_STC", "physicalInfo"}
        })

        physicalInfo = OUT_values.physicalInfo

        -- modify the hwDevNum in the entries for the lower EPort range
        -- chekc that for this eport the physical interface is 'port type'(not trunk/vidx)
        if physicalInfo.devPort ~= nil then

            physicalInfo.devPort.devNum  = hwDevNum

            apiName = "cpssDxChBrgEportToPhysicalPortTargetMappingTableSet"
            genericCpssApiWithErrorHandler(command_data,
                apiName, {
                { "IN", "GT_U8", "devNum", devNum},
                { "IN", "GT_PORT_NUM", "portNum", portNum},
                { "IN", "CPSS_INTERFACE_INFO_STC", "physicalInfoPtr", physicalInfo }
            })
        end
    end
end
-- save the 'hwDevNum' for the 'restore' function
local prev_hwDevNum = nil
-- function to change needed HW devNum in several key places ... needed after cpssInitSystem(...)
local function changeHwDevNum(command_data,devNum,hwDevNum,unset)
    local dummyPortNum
    if unset == true then
        hwDevNum = prev_hwDevNum
        prev_hwDevNum = nil
    else
        if prev_hwDevNum == nil then
            prev_hwDevNum , dummyPortNum =
                Command_Data:getHWDevicePort(devNum,0)
        end
    end

    local apiName = "cpssDxChCfgHwDevNumSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",     "GT_U8",            "devNum",     devNum },
        { "IN",     "GT_HW_DEV_NUM",    "hwDevNum",    hwDevNum}
    })

    if vss_internal_db_global.my_role == vss_role_cc_full_name then
        on_cc_set_e2phy_low_entries(command_data,devNum,hwDevNum,unset)
    end


end

-- get the max-eport number in device
local function on_cc_getMaxEportNum(devNum)
    local arraySizes = {}
    local size = 128 * 1024 -- 128,64,32,16,8,4
    local eport
    
    for ii = 1,6 do
        arraySizes[ii] = size / 2
        size = size / 2
    end

    for index,entry in pairs(arraySizes) do
        eport = entry-1
        -- find first eport that is supported
         if (true == wrLogWrapper("wrlDxChPortIsValidEport", "(devNum, eport)", devNum, eport)) then
            return eport
         end
    end
end

-- on cc - set the global eport range
local function on_cc_setGlobalEportRange(command_data,devNum,unset)
    local l2EcmpInfo

    local  start_val, end_val, diff, max_eport_num, the_mask, the_pattern, enable

    if unset == true then
        l2EcmpInfo = { enable = false ,
                        pattern = 0,
                        mask = 0}
    else
        start_val = vss_internal_db_global.global_eport_range.start_eport
        end_val    = vss_internal_db_global.global_eport_range.end_eport
        diff = end_val - start_val
        max_eport_num = on_cc_getMaxEportNum(devNum)
        -- calc mask
        the_mask = max_eport_num - diff
        the_pattern = start_val

        l2EcmpInfo = { enable = true ,
                        pattern = the_pattern,
                        mask = the_mask}
    end

    local globalInfo = l2EcmpInfo

    local apiName = "cpssDxChCfgGlobalEportSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",     "GT_U8",            "devNum",     devNum },
        { "IN",     "CPSS_DXCH_CFG_GLOBAL_EPORT_STC",    "globalPtr",    globalInfo},
        { "IN",     "CPSS_DXCH_CFG_GLOBAL_EPORT_STC",    "l2EcmpPtr",    l2EcmpInfo}
    })

    if unset == true then
        enable = false
    else
        enable = true
    end
    -- enable L2ECMP in the device
    apiName = "cpssDxChBrgL2EcmpEnableSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",     "GT_U8",            "devNum",     devNum },
        { "IN",     "GT_BOOL",    "enable",    enable}
    })

end

-- on cc -- the needed tti udbs
local on_cc_tti_udbArr =
{ --byte#   offset type                         offset
    -- metadata section
-- bits  0 .. 7 in the key
    { udbIndex = 0,    offsetType = "CPSS_DXCH_TTI_OFFSET_METADATA_E",  offset = 22  ,name = "pcl_id_low"    }, -- bit 0..4 , 5 --> 5 LSB bits of pclid , bit 5 - Local Device Source is Trunk
-- bits  8..15 in the key
    { udbIndex = 1,    offsetType = "CPSS_DXCH_TTI_OFFSET_METADATA_E",  offset = 23  ,name = "pcl_id_high"    }, -- bit 0..4 --> 5 MSB bits of pclid
-- bits  16 .. 23 in the key
    { udbIndex = 2,    offsetType = "CPSS_DXCH_TTI_OFFSET_METADATA_E",  offset = 26  ,name = "localDevSrcPort"    }, -- Local Device Source ePort/TrunkID[7:0]
-- bits  24 .. 31 in the key
    { udbIndex = 3,    offsetType = "CPSS_DXCH_TTI_OFFSET_METADATA_E",  offset = 30  ,name = "dsa_srcDev"    }, -- DSA Tag SrcDev[7:0]
-- bits  32 .. 39 in the key
    { udbIndex = 4,    offsetType = "CPSS_DXCH_TTI_OFFSET_METADATA_E",  offset = 28  ,name = "dsa_srcPort"    }, -- DSA Tag SrcPort/SRCePort/TrunkID[7:0]
-- bits  40 .. 47 in the key
    { udbIndex = 5,    offsetType = "CPSS_DXCH_TTI_OFFSET_L2_E",  offset = (12+4+2)  ,name = "dsa_useVidx"    } -- do explicit look into the 'extended dsa' to get the 'use_vidx' bit 12 in word1 in dsa --> (byte = (12 + 4 + 2), bit = (12 % 8) = 4 )
}


-- hold the connection between 'logic fields' and their position and mask in the UDBs array
local on_cc_tti_key_info =
{
    pcl_id = {["pcl_id_low"]  = {udb = {startBit = 0, numBits = 5} },  -- low  5 bits , field omitted --> 'start at bit 0 , numBits like 'udb.numBits''
              ["pcl_id_high"] = {udb = {startBit = 0, numBits = 5} , field = {startBit = 5, numBits = 5}} -- high 5 bits
    }

    ,localDevSrcIsTrunk = {["pcl_id_low"]  = {udb = {startBit = 5, numBits = 1}}} -- field omitted --> 'start at bit 0 , numBits like 'udb.numBits''

    ,localDevSrcPort    = {} -- same name as in on_cc_tti_udbArr , 'full byte'
    ,dsa_srcDev         = {} -- same name as in on_cc_tti_udbArr , 'full byte'
    ,dsa_srcPort        = {} -- same name as in on_cc_tti_udbArr , 'full byte'

    ,dsa_useVidx        = {["dsa_useVidx"]  = {udb = {startBit = 4, numBits = 1}}} -- field omitted --> 'start at bit 0 , numBits like 'udb.numBits''
}
-- find the udb info according to it's name
-- return full entry from the on_cc_tti_udbArr
local function on_cc_get_tti_udb_info(udbName)
    for index,entry in pairs(on_cc_tti_udbArr) do
        if entry.name == udbName then
            return entry
        end
    end

    --error

    print ("on_cc_get_tti_udb_info : not found " .. udbName)

end

-- get pattern,mask , value , field info
-- returns updated pattern,mask
local function for_field_buildTtiPatternMask(patternArr,maskArr,fieldValue,fieldName)

    local fieldInfo = on_cc_tti_key_info[fieldName]
    local ii, field_mask, new_mask, new_value, tti_udb
    local startByte, offset_current_byte, current_field, current_udb

    -- format of 'fieldInfo':
    --  {["pcl_id_low"]  = {udb = {startBit = 0, numBits = 5} },
    --   ["pcl_id_high"] = {udb = {startBit = 0, numBits = 5} , field = {startBit = 5, numBits = 5}}

    local count = 0
    for index,entry in pairs(fieldInfo) do
        count = count + 1
    end

    if count == 0 then  -- support 'empty' info : {}
        -- same name as in on_cc_tti_udbArr , 'full byte'
        tti_udb = on_cc_get_tti_udb_info(fieldName)

        -- format of 'tti_udb' -->
        -- { udbIndex = 4,    offsetType = "CPSS_DXCH_TTI_OFFSET_METADATA_E",  offset = 28  ,name = "dsa_srcPort"    }

        ii = tti_udb.udbIndex

        field_mask = 0xFF

        new_value = bit_and(fieldValue , field_mask)
        new_mask = field_mask

        -- update the byte in the pattern,mask
        maskArr[ii]    = new_mask
        patternArr[ii] = new_value

    else
        for index,entry in pairs(fieldInfo) do
            -- format of 'entry'
            -- {udb = {startBit = 0, numBits = 5} , field = {startBit = 5, numBits = 5}}

            --print ("entry = " , to_string(entry))
            --print ("index = " , to_string(index))

            tti_udb = on_cc_get_tti_udb_info(index)

            --print ("tti_udb = " , to_string(tti_udb))

            -- format of 'tti_udb' -->
            -- { udbIndex = 4,    offsetType = "CPSS_DXCH_TTI_OFFSET_METADATA_E",  offset = 28  ,name = "dsa_srcPort"    }

            startByte = tti_udb.udbIndex
            ii = startByte
            offset_current_byte = startByte - ii -- result is 0 !!!

            current_udb = entry.udb

            if entry.field == nil then
                current_field = current_udb
            else
                current_field = entry.field
            end

            -- format of 'current_udb' and of 'current_field' --> field = {startBit = 5, numBits = 5}
            field_mask = mask_of_bits(current_field.startBit,current_field.numBits)

            -- do : new_value = fieldValue & field_mask
            new_value  = bit_and(fieldValue , field_mask)
            -- do : new_value = new_value >> current_field.startBit
            new_value = bit_shr(new_value,current_field.startBit)
            -- do : new_value = new_value << current_udb.startBit
            new_value = bit_shl(new_value,current_udb.startBit)

            -- do : new_mask = 'mask of bits' from current_udb.startBit in length current_udb.numBits
            new_mask = mask_of_bits(current_udb.startBit,current_udb.numBits)

        -- update the byte in the pattern,mask
            maskArr[ii]    = maskArr[ii]    + new_mask
            patternArr[ii] = patternArr[ii] + new_value
        end
    end

    -- return the updated pattern,mask arrays
    return patternArr,maskArr
end

-- convert bool to integer
local function bool_2_int(boolVal)
    if boolVal == true then
        return 1
    else
        return 0
    end
end

-- convert bool to integer , but 'nil' remains 'nil' !!!
local function bool_2_int_keep_nil(boolVal)
    if boolVal == nil then
        return nil -- this to keep indication that variable omitted !!!
    else
        return bool_2_int(boolVal)
    end
end

-- needed on lc only , for ipcl eDSA recognition
-- function return 2 arrays:
-- UDBs pattern array
-- UDBs mask array
function vss_global_func_on_cc_buildTtiPatternMask(dsa_useVidx,localDevSrcIsTrunk,localDevSrcPort,dsa_srcDev,dsa_srcPort,pcl_id)
    local currentPattern_udbArray = {}
    local currentMask_udbArray    = {}

    -- start with empty tables
    for ii = 0,9 do
        currentPattern_udbArray[ii] = 0
        currentMask_udbArray[ii] = 0
    end

    --_debug_to_string(dsa_useVidx,localDevSrcIsTrunk,localDevSrcPort,dsa_srcDev,dsa_srcPort,pcl_id)

    -- ***************************
    -- ***************************
    local fieldsInfo = {
         [1] = {fieldName = "dsa_useVidx"        , fieldValue = bool_2_int(dsa_useVidx)}
        ,[2] = {fieldName = "localDevSrcIsTrunk" , fieldValue = bool_2_int(localDevSrcIsTrunk)}
        ,[3] = {fieldName = "localDevSrcPort"    , fieldValue = localDevSrcPort}
        ,[4] = {fieldName = "dsa_srcDev"         , fieldValue = dsa_srcDev}
        ,[5] = {fieldName = "dsa_srcPort"        , fieldValue = dsa_srcPort}
        ,[6] = {fieldName = "pcl_id"             , fieldValue = pcl_id}
    }

    --_debug_to_string(fieldsInfo)

    -- process all the fields to build the key
    for index,entry in pairs(fieldsInfo) do
        local fieldName = entry.fieldName
        local fieldValue = entry.fieldValue

        -- support 'nil' parameters
        if fieldValue ~= nil then
            currentPattern_udbArray,currentMask_udbArray =
                for_field_buildTtiPatternMask(currentPattern_udbArray,currentMask_udbArray,
                    fieldValue,fieldName)
        end
    end

    --_debug_to_string(currentPattern_udbArray,currentMask_udbArray)

    return currentPattern_udbArray,currentMask_udbArray
end

-- on cc - init the tti unit
local function on_cc_tti_init(command_data,devNum,unset)
    local pclId = on_cc_pcl_id -- assume 0 .. as we not set it explicitly
    
    local enableOwnDevFltr

    -- set the UDBs needed for the tti lookup
    tti_multiple_pciId_and_Udb_config_set(command_data,devNum,pclId,on_cc_tti_udbArr)

    --[[
        When bc2 re-assign srcEPort it also indicate that вЂ�orig srcDevвЂ™ is вЂ�own devвЂ™ .
        In the VSS the bc2 getting packet from AC3 will re-assign srcEPort.(and packet came with dsa)
        such packet subject to drop by  L2i filter according to <DSA Tag Source Device Is Local Filter Disable>
        --> so need to disable the filter
    --]]
    if unset == true then
        enableOwnDevFltr = true
    else
        enableOwnDevFltr = false
    end

    if is_sip_5(devNum) then
        -- the filter must be disabled at all times.
        -- this is the default that the 'phase 1' init of CPSS setting.
        enableOwnDevFltr = false
    end
    
    
    local apiName = "cpssDxChCscdDsaSrcDevFilterSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",     "GT_U8",      "devNum",     devNum },
        { "IN",     "GT_BOOL",    "enableOwnDevFltr",    enableOwnDevFltr}
    })

end


local dsa_tag_packet_Cmd_forward = 3

-- info about the fields that may be needed for building UDB for eDSA tag.
-- the info is written in format of 'bits and words in the eDSA.
local on_lc_recognized_by_udbs_edsa_tag_format = {
     packetCmd = { [1] = {word = 0 , startBit = 30 , numBits = 2} }
    ,useVidx   = { [1] = {word = 1 , startBit = 12 , numBits = 1} }
    ,trgDev    = {
        -- Trg Dev[4:0]
         [1] = {word = 1 , startBit = 0 , numBits = 5}
        -- Trg Dev[11:5]
        ,[2] = {word = 3 , startBit = 0 , numBits = 7}
    }
    ,trgEPort = {[1] = {word = 3 , startBit = 7 , numBits = 17}}
    ,srcEPort = {
        --Src ePort[4:0]
         [1] = {word = 0 , startBit = 19 , numBits = 5}
        --Src ePort[6:5]
        ,[2] = {word = 1 , startBit = 29 , numBits = 2}
        --Src ePort[16:7]
        ,[3] = {word = 2 , startBit = 3 , numBits = 10}
    }
    ,routed = { [1] = {word = 1 , startBit = 25 , numBits = 1} }

    ,srcDev = {
        --Src Dev[4:0]
         [1] = {word = 0 , startBit = 24 , numBits = 5}
        --Src Dev[11:5]
        ,[2] = {word = 2 , startBit = 14 , numBits = 7}
    }

}
-- convert {word,bit_in_word} to {byte,bit_in_byte}
local function convertWordAndBitToByteAndBit(word,bit_in_word)
    local byte = (word * 4) + (3 - math.floor(bit_in_word / 8))
    local bit_in_byte = bit_in_word % 8

    return byte,bit_in_byte
end

-- get pattern,mask , value , field info
-- returns updated pattern,mask
local function for_field_buildIpclPatternMask(patternArr,maskArr,fieldValue,fieldInfo)
    local debug_function = false
    
    if fieldInfo[1].word == 3 then
        --debug_function = true
    end
    if fieldInfo[3] then
        --debug_function = true
    end

    if debug_function then
        _debug_to_string("fieldInfo",fieldInfo,"fieldValue",fieldValue)
    end

    local numBitsUsedFromField = 0 -- support split fields
    local entry, startByte, startBit, endByte, endBit
    local leftShiftBits, numBitsInCurrByte, currentValue, currentByteValue, currentByteMask
    
    for ii = 1,#fieldInfo do
        entry = fieldInfo[ii]
        -- convert info in db to offset in bytes , bits in byte
        startByte,startBit = convertWordAndBitToByteAndBit(entry.word,entry.startBit)
        endByte,endBit     = convertWordAndBitToByteAndBit(entry.word,entry.startBit + entry.numBits - 1)

        if debug_function then
            _debug_to_string("startByte,startBit,endByte,endBit",startByte,startBit,endByte,endBit)
        end

        --network order is different then little ENDIAN order:
        for ii = startByte,endByte,-1 do
            leftShiftBits = 0
            numBitsInCurrByte = 0
            if ii == endByte then
                if endByte ~= startByte then
                    numBitsInCurrByte = endBit + 1
                    leftShiftBits = 0
                else
                    -- all bits in the same byte
                    numBitsInCurrByte = entry.numBits
                    leftShiftBits = startBit
                end
            elseif ii ~= startByte then
                numBitsInCurrByte = 8
                leftShiftBits = 0
            else
                -- startByte .. that is not equal to endByte
                numBitsInCurrByte = 8 - startBit
                leftShiftBits = startBit
            end

            if debug_function then
                _debug_to_string("ii,leftShiftBits,numBitsInCurrByte,numBitsUsedFromField ", ii,leftShiftBits,numBitsInCurrByte,numBitsUsedFromField)
            end

            -- currentValue = fieldValue >> numBitsUsedFromField
            currentValue = bit_shr(fieldValue,numBitsUsedFromField)
            -- currentValue = currentValue << leftShiftBits
            currentValue = bit_shl(currentValue,leftShiftBits)
            -- mask of current field in current byte
            currentByteMask = mask_of_bits(leftShiftBits,numBitsInCurrByte)
            -- currentByteValue = currentValue & currentByteMask
            currentByteValue = bit_and(currentValue,currentByteMask)

            if debug_function then
                _debug_to_string("currentValue,currentByteMask,currentByteValue ", currentValue,currentByteMask,currentByteValue)
            end

            numBitsUsedFromField = numBitsUsedFromField + numBitsInCurrByte

            -- update the byte in the pattern,mask
            maskArr[ii]    = maskArr[ii]    + currentByteMask
            patternArr[ii] = patternArr[ii] + currentByteValue
        end
    end

    -- return the updated pattern,mask arrays
    return patternArr,maskArr
end

-- needed on lc only , for ipcl eDSA recognition
-- function return 2 arrays:
-- UDBs pattern array
-- UDBs mask array
function vss_global_func_on_lc_buildIpclPatternMask(useVidx,vidx,trgDev,trgEPort,srcEport,routed)
    local currentPattern_udbArray = {}
    local currentMask_udbArray    = {}

    -- start with empty tables
    for ii = 0,15 do
        currentPattern_udbArray[ii] = 0
        currentMask_udbArray[ii] = 0
    end

    local useVidx_value

    -- set packetCmd
    local value = dsa_tag_packet_Cmd_forward
    currentPattern_udbArray,currentMask_udbArray =
        for_field_buildIpclPatternMask(
            currentPattern_udbArray,currentMask_udbArray,
            value,
            on_lc_recognized_by_udbs_edsa_tag_format.packetCmd)

    -- set 'srcEport'
    if srcEport ~= nil then
        value = srcEport

        currentPattern_udbArray,currentMask_udbArray =
            for_field_buildIpclPatternMask(
                currentPattern_udbArray,currentMask_udbArray,
                value,
                on_lc_recognized_by_udbs_edsa_tag_format.srcEPort)

        -- need to associate the eport 'my VSS-id'
        currentPattern_udbArray,currentMask_udbArray =
            for_field_buildIpclPatternMask(
                currentPattern_udbArray,currentMask_udbArray,
                vss_internal_db_global.my_vss_id,
                on_lc_recognized_by_udbs_edsa_tag_format.srcDev)
    end

    --
    -- set 'trg info' : vidx / trgDev,trgEport
    --

    if useVidx == true then
        useVidx_value = 1
    else
        useVidx_value = 0

        -- set trgDev
        value = trgDev
        currentPattern_udbArray,currentMask_udbArray =
            for_field_buildIpclPatternMask(
                currentPattern_udbArray,currentMask_udbArray,
                value,
                on_lc_recognized_by_udbs_edsa_tag_format.trgDev)

        -- set trgEPort
        value = trgEPort
        currentPattern_udbArray,currentMask_udbArray =
            for_field_buildIpclPatternMask(
                currentPattern_udbArray,currentMask_udbArray,
                value,
                on_lc_recognized_by_udbs_edsa_tag_format.trgEPort)
    end

    -- set useVidx field
    currentPattern_udbArray,currentMask_udbArray =
        for_field_buildIpclPatternMask(
            currentPattern_udbArray,currentMask_udbArray,
            useVidx_value,
            on_lc_recognized_by_udbs_edsa_tag_format.useVidx)

    -- set routed
    if routed == true then
        value = 1
        currentPattern_udbArray,currentMask_udbArray =
            for_field_buildIpclPatternMask(
                currentPattern_udbArray,currentMask_udbArray,
                value,
                on_lc_recognized_by_udbs_edsa_tag_format.routed)
    end

    return currentPattern_udbArray,currentMask_udbArray
end


-- needed on lc only , for ipcl eDSA recognition
-- code based on prvLion2FabricUdbsConfigure
-- define the 16 consecutive UDBs of 16 eDSA tag bytes in the packet (after 12 bytes of macDa,macSa)
local function on_lc_define_udbs_for_EDsa_tag_recognition(command_data,devNum,unset)
    --GT_U32                               udbIndex;
    --GT_U32                               udbOffset;
    --CPSS_DXCH_PCL_OFFSET_TYPE_ENT        udbOffsetType;
    --CPSS_DXCH_PCL_PACKET_TYPE_ENT
    local packetTypeArr = {
        "CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E"  ,
        "CPSS_DXCH_PCL_PACKET_TYPE_IPV4_UDP_E"  ,
        "CPSS_DXCH_PCL_PACKET_TYPE_MPLS_E"      ,
        "CPSS_DXCH_PCL_PACKET_TYPE_IPV4_FRAGMENT_E" ,
        "CPSS_DXCH_PCL_PACKET_TYPE_IPV4_OTHER_E"    ,
        "CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E",
        "CPSS_DXCH_PCL_PACKET_TYPE_UDE_E"           ,
        "CPSS_DXCH_PCL_PACKET_TYPE_IPV6_E"
    }

    local udbOffset
    local apiName
    
    local udbOffsetType
    if unset == true then
        udbOffsetType = "CPSS_DXCH_PCL_OFFSET_INVALID_E"
    else
        udbOffsetType = "CPSS_DXCH_PCL_OFFSET_L2_E"
    end

    for index,entry in pairs(packetTypeArr) do
        for udbIndex = 0,15 do
            if unset == true then
                udbOffset = 0
            else
                udbOffset = (12 + udbIndex) --offset after the mac addresses
            end

            --***************
            --***************
            apiName = "cpssDxChPclUserDefinedByteSet"
            genericCpssApiWithErrorHandler(command_data,
                apiName, {
                { "IN",     "GT_U8",            "devNum",     devNum },
                { "IN",     "CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT",    "ruleFormat",    "CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E"},
                { "IN",     "CPSS_DXCH_PCL_PACKET_TYPE_ENT",    "packetType",    entry},
                { "IN",     "CPSS_PCL_DIRECTION_ENT",    "direction",    "CPSS_PCL_DIRECTION_INGRESS_E"},
                { "IN",     "GT_U32",    "udbIndex",    udbIndex},
                { "IN",     "CPSS_DXCH_PCL_OFFSET_TYPE_ENT",    "offsetType",    udbOffsetType},
                { "IN",     "GT_U8",    "offset",    udbOffset}
            })
       end
    end

    return
end

-- set on lc pcl rule
function vss_global_func_on_lc_pcl_rule_set(command_data,devNum,shortRuleFormat,myIndexName,finalPclPattern,finalPclMask,finalPclAction,from_end_of_tcam,unset)
    local internal_convert = {
         ["ing_std_udb"]        = {ruleSize = vss_on_lc_pcl_rule_size_STD_name, ruleFormat = vss_on_lc_pcl_rule_forma_ing_std_udb_name        ,STC_name= vss_on_lc_pcl_rule_STC_ing_std_udb_name}
        ,["egr_std_not_ip"]     = {ruleSize = vss_on_lc_pcl_rule_size_STD_name, ruleFormat = vss_on_lc_pcl_rule_forma_egr_std_not_ip_name     ,STC_name= vss_on_lc_pcl_rule_STC_egr_std_not_ip_name}
        ,["egr_std_ip_l2_qos"]  = {ruleSize = vss_on_lc_pcl_rule_size_STD_name, ruleFormat = vss_on_lc_pcl_rule_forma_egr_std_ip_l2_qos_name  ,STC_name= vss_on_lc_pcl_rule_STC_egr_std_ip_l2_qos_name}
    }

    local apiName
    
    --_debug_to_string("finalPclPattern")
    --_debug_to_string(myIndexName,finalPclPattern)
    --_debug_to_string(finalPclMask)

    local convertEntry = internal_convert[shortRuleFormat]

    if string.match(shortRuleFormat,"egr_") then
        -- make sure that 'egress' is indicated. (not sure it needed... but on the safe side)
        finalPclAction.egressPolicy = true
    end

    -- use STC_name instead of "CPSS_DXCH_PCL_RULE_FORMAT_UNT"
    -- so the 'trace' will less info to print !!!
    local patternAndMaskType = convertEntry.STC_name

    -- concatenate to the name the 'rule format'
    myIndexName = myIndexName .. " " .. shortRuleFormat

    local myRuleSize = vss_on_lc_pcl_rule_size_STD -- 'STD' rule ('standard' rule)
    local pclIndex
    -- if myIndexName not exists in DB it will give 'new index' and saved into DB
    -- if myIndexName exists in DB it will retrieved from DB
    if from_end_of_tcam == true then
        pclIndex = allocPclIndexFromPool_fromEnd(myRuleSize,myIndexName)
    else
        pclIndex = allocPclIndexFromPool        (myRuleSize,myIndexName)
    end

    if unset == true then
        apiName = "cpssDxChPclRuleInvalidate"
        genericCpssApiWithErrorHandler(command_data,
            apiName, {
            { "IN", "GT_U8", "devNum", devNum},
            { "IN", "CPSS_PCL_RULE_SIZE_ENT", "ruleSize", convertEntry.ruleSize},
            { "IN", "GT_U32", "ruleIndex", pclIndex}
        })

        freePclIndexToPool(pclIndex)
    else
        apiName = "cpssDxChPclRuleSet"
        genericCpssApiWithErrorHandler(command_data,
            apiName ,{
            { "IN",     "GT_U8",                           "devNum",     devNum },
            {"IN","CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT","ruleFormat",convertEntry.ruleFormat},
            {"IN","GT_U32","ruleIndex",pclIndex},   -- index of the rule
            {"IN","CPSS_DXCH_PCL_RULE_OPTION_ENT","ruleOptionsBmp",0}, --Rule is valid
            {"IN",patternAndMaskType,"maskPtr",finalPclMask},
            {"IN",patternAndMaskType,"patternPtr",finalPclPattern},
            {"IN","CPSS_DXCH_PCL_ACTION_STC","actionPtr",finalPclAction}
        })
    end

end

-- convert macAddrStr = "00:11:22:33:44:55"
-- to 'GT_ETHERADDR'
function convertMacAddrStringTo_GT_ETHERADDR(macAddrStr)
    local macAddrSTC = {arEther = {}}
    local startNibble, endNibble, curr_byte
    
    for ii = 0,5 do
        startNibble = (3 * ii) + 1
        endNibble = startNibble + 1

        curr_byte = string.sub(macAddrStr,startNibble,endNibble)

        macAddrSTC.arEther[ii] = byteAs2NibblesToNumeric(curr_byte)
    end

    --_debug_to_string(macAddrSTC)

    return macAddrSTC
end

-- on lc - init pcl (ipcl,epcl) needed 'global' connfig
local function on_lc_pcl_init(command_data,devNum,unset)
    local enable
    local apiName
    
    --***************
    --***************
    -- enable IPCL on the device
    if unset == true then
        enable = false
    else
        enable = true
    end

    apiName = "cpssDxChPclIngressPolicyEnable"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        {"IN","GT_U8","devNum",devNum},
        {"IN","GT_BOOL","enable",enable}
    })

    -- define the 16 consecutive UDBs of 16 eDSA tag bytes in the packet (after 12 bytes of macDa,macSa)
    on_lc_define_udbs_for_EDsa_tag_recognition(command_data,devNum)

    --***************
    --***************
    -- set IPCL and EPCL configuration tables access modes on the device
    -- CPSS_DXCH_PCL_CFG_TBL_ACCESS_MODE_STC
    local accMode = {
      ipclAccMode        = "CPSS_DXCH_PCL_CFG_TBL_ACCESS_LOCAL_PORT_E"
     ,ipclMaxDevPorts    = "CPSS_DXCH_PCL_CFG_TBL_MAX_DEV_PORTS_32_E"
     ,ipclDevPortBase    = "CPSS_DXCH_PCL_CFG_TBL_DEV_PORT_BASE0_E"
     ,epclAccMode        = "CPSS_DXCH_PCL_CFG_TBL_ACCESS_LOCAL_PORT_E"
     ,epclMaxDevPorts    = "CPSS_DXCH_PCL_CFG_TBL_MAX_DEV_PORTS_32_E"
     ,epclDevPortBase    = "CPSS_DXCH_PCL_CFG_TBL_DEV_PORT_BASE0_E"
    }

    apiName = "cpssDxChPclCfgTblAccessModeSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        {"IN","GT_U8","devNum",devNum},
        {"IN","CPSS_DXCH_PCL_CFG_TBL_ACCESS_MODE_STC","accModePtr",accMode}
    })


    --***************
    --***************
    -- enable EPCL on the device
    apiName = "cpssDxCh2PclEgressPolicyEnable"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        {"IN","GT_U8","devNum",devNum},
        {"IN","GT_BOOL","enable",enable}
    })

    --***************
    --***************
    --[[ IPCL rule :
        a.	purpose : "VSS : 'default rule' set Source ID = 0 for all traffic (that not matched other rules)"
        b.	Key:
            i.	Empty !!!
        c.	Action:
            i.	Source-ID = 0
    --]]

    local myIndexName = "VSS : 'default rule' set Source ID = 0 for ALL traffic (that not matched other rules)"

    -- make the PCL action to modify only the srcId
    local finalPclAction = {sourceId = {assignSourceId = true , sourceIdValue = 0}}

    -- define (almost) empty pattern , for 'STD UDB' key , with pcl-id for 'ipcl' from uplink
    local finalPclPattern = {ruleIngrStdUdb = {commonIngrUdb = {pclId = vss_internal_db_on_lc_used_uplink_ipcl_pcl_id}}}
    -- define (almost) empty mask , for 'STD UDB' key , with mask on pcl-id
    local finalPclMask    = {ruleIngrStdUdb = {commonIngrUdb = {pclId = vss_pcl_id_mask_10_bits}}}

    finalPclPattern = finalPclPattern.ruleIngrStdUdb
    finalPclMask = finalPclMask.ruleIngrStdUdb

    -- call to write the rule to HW
    vss_global_func_on_lc_pcl_rule_set(command_data,devNum,"ing_std_udb",myIndexName,finalPclPattern,finalPclMask,finalPclAction,true,unset)

    --***************
    --***************
    --[[
        5.	set EPCL rules for the used for traffic egress the VSS ports
            a.	NOTE: the next rule need to be added as next 2 keys: (total 2 rules !)
                i.	key_type  = STD_NOT_IP  for non-ip packets
                ii.	key_type  = STD_IP_L2_QOS  for ip packets
            b.	Rule #2
                i.	Purpose : “VSS : EPCL : For traffic egress the VSS ports , filter Routed IPM packets origin from local vss” .. “ key type =” .. ‘key_type’
                ii.	key
                    1.	pcl_id = epcl_pcl_id
                    2.	<Routed>=1
                    3.	<MAC DA[47:24]> = 01-00-5e
                iii.	Action:
                    1.	Drop (hard_drop)
    --]]
    myIndexName = "VSS : EPCL : For traffic egress the VSS ports , filter Routed IPM packets origin from local vss."

    finalPclAction = {pktCmd = "CPSS_PACKET_CMD_DROP_HARD_E"}
    local common_pattern = {
                common={valid = 1 , pclId = vss_internal_db_on_lc_used_for_vss_ports_epcl_pcl_id,
                        fwdData = { isRouted = 0}}
                ,macDa = convertMacAddrStringTo_GT_ETHERADDR("01:00:5e:00:00:00")
                }
    local common_mask = {
                common={valid = 1 , pclId = vss_pcl_id_mask_10_bits,
                        fwdData = { isRouted = 1}}
                ,macDa = convertMacAddrStringTo_GT_ETHERADDR("ff:ff:ff:00:00:00")
                }

    --_debug_to_string("common_pattern = ",common_pattern)

    local shortRuleFormat = "egr_std_not_ip"
    finalPclPattern = {ruleEgrStdNotIp = common_pattern}
    finalPclMask    = {ruleEgrStdNotIp = common_mask}

    finalPclPattern = finalPclPattern.ruleEgrStdNotIp
    finalPclMask = finalPclMask.ruleEgrStdNotIp

    vss_global_func_on_lc_pcl_rule_set(command_data,devNum,shortRuleFormat,myIndexName,finalPclPattern,finalPclMask,finalPclAction,false,unset)

    shortRuleFormat = "egr_std_ip_l2_qos"
    finalPclPattern = {ruleEgrStdIpL2Qos = common_pattern}
    finalPclMask    = {ruleEgrStdIpL2Qos = common_mask}

    finalPclPattern = finalPclPattern.ruleEgrStdIpL2Qos
    finalPclMask = finalPclMask.ruleEgrStdIpL2Qos

    vss_global_func_on_lc_pcl_rule_set(command_data,devNum,shortRuleFormat,myIndexName,finalPclPattern,finalPclMask,finalPclAction,false,unset)

end

-- on lc : set trunk init
local function on_lc_trunk_init(command_data,devNum,unset)
    local mode
    --***************
    --***************
    -- set the designated table more predictable .. without the Vid in the calculations
    if unset == true then
        mode = "CPSS_DXCH_TRUNK_DESIGNATED_TABLE_USE_INGRESS_HASH_AND_VID_E"
    else
        mode = "CPSS_DXCH_TRUNK_DESIGNATED_TABLE_USE_INGRESS_HASH_E"
    end
    local apiName = "cpssDxChTrunkHashDesignatedTableModeSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
           { "IN", "GT_U8",    "dev",  devNum},
           { "IN", "CPSS_DXCH_TRUNK_DESIGNATED_TABLE_MODE_ENT", "mode", mode}
    })
end

-- on lc : set srcId init
local function on_lc_srcid_init(command_data,devNum,unset)
    local portsMembers
    if unset == true then
        -- 28 ports in the device
        portsMembers = {ports = {[0]=0x0fffffff}} -- all ports
    else
        portsMembers = {} -- empty -- no ports !!!
    end

    local source_id = vss_internal_db_global.on_lc_info.uplink.src_id_for_vss_trunk

    local apiName = "cpssDxChBrgSrcIdGroupEntrySet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
           { "IN", "GT_U8",    "dev",  devNum},
           { "IN", "GT_U32", "sourceId", source_id},
           { "IN", "GT_BOOL", "cpuSrcIdMember", true},
           { "IN", "CPSS_PORTS_BMP_STC", "portsMembersPtr", portsMembers}
    })

end

-- set device map table on device to point to 'hw_trgDevNum' by cascade port/trunk
local function setDevMapTable(command_data,devNum,hw_trgDevNum,cscdTrunkId,cscdPort,unset)
    local cascadeLink
    
    if unset == true then
        -- the 'default' is 'null port' (62)
        cascadeLink = {linkNum = 62    , linkType = "CPSS_CSCD_LINK_TYPE_PORT_E"}
    else
        if cscdTrunkId ~= nil then
            cascadeLink = {linkNum = cscdTrunkId , linkType = "CPSS_CSCD_LINK_TYPE_TRUNK_E"}
        else
            cascadeLink = {linkNum = cscdPort    , linkType = "CPSS_CSCD_LINK_TYPE_PORT_E"}
        end
    end

    --***************
    --***************
    local apiName = "cpssDxChCscdDevMapTableSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
           { "IN", "GT_U8",    "dev",  devNum},
           {"IN", "GT_HW_DEV_NUM", "targDev", hw_trgDevNum},
           {"IN", "GT_HW_DEV_NUM", "srcDev", 0},
           {"IN", "GT_PORT_NUM", "portNum", 0},
           {"IN", "CPSS_CSCD_LINK_TYPE_STC", "cascadeLink", cascadeLink},
           {"IN", "CPSS_DXCH_CSCD_TRUNK_LINK_HASH_ENT", "srcPortTrunkHashEn", "CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_INGR_PIPE_HASH_E"},
           {"IN", "GT_BOOL", "egressAttributesLocallyEn", false}
        })

end

-- on lc : Set device map table to point the uplink to 'my cc device'
local function on_lc_point_uplink_to_my_cc_device(command_data,devNum,unset)
    local connection = vss_internal_db_global.on_lc_info.uplink.connection

    local hw_trgDevNum = vss_internal_db_global.my_vss_id
    local cscdTrunkId = connection.trunkId
    local cscdPort = connection.portNum
    -- Set device map table to point the uplink to 'my cc device'
    setDevMapTable(command_data,devNum,hw_trgDevNum,cscdTrunkId,cscdPort,unset)
end

-- 'per device' : function to set 'vss my-info'
local function local_vss_function__per_device__vss_my_info(command_data,devNum,hwDevNum,unset)
    -- change the HW devNum
    changeHwDevNum(command_data,devNum,hwDevNum,unset)

    if vss_internal_db_global.my_role == vss_role_cc_full_name then
        -- set global eport range
        on_cc_setGlobalEportRange(command_data,devNum,unset)

        -- set tti init
        on_cc_tti_init(command_data,devNum,unset)
    else
        -- set pcl/epcl init
        on_lc_pcl_init(command_data,devNum,unset)

        -- set trunk init
        on_lc_trunk_init(command_data,devNum,unset)

       -- set srcId init
        on_lc_srcid_init(command_data,devNum,unset)
    end
end

-- function to set 'vss my-info'
local function local_vss_function__vss_my_info(params)
    local command_data = Command_Data()
    local did_error = false
    local unset = (params.flagNo == true)
    local hwDevNum

    -- Common variables initialization
    command_data:clearResultArray()
    command_data:initAllAvailableDevicesRange()

    if unset == true and vss_internal_db_global.my_role == nil then
        add_error(command_data,"VSS : the device not in VSS")
        did_error = true
    end

    -- do not allow to override existing role
    if unset == true then
        -- no checks regarding 'params'
    elseif did_error == false and
        vss_internal_db_global.my_role and
        vss_internal_db_global.my_role ~= params.role then
        add_error(command_data,"VSS : do not allow to override existing role (first remove role)")
        did_error = true
    end

    if did_error ~= true then
        if unset == true then
            -- info already in DB
            -- no info from 'params'
            hwDevNum = vss_internal_db_global.my_hw_devNum
        else
            vss_internal_db_global.my_role = params.role
            vss_internal_db_global.my_name = params.the_name
            vss_internal_db_global.my_vss_id = params.the_vss_id

            if vss_internal_db_global.my_role == vss_role_cc_full_name then
                hwDevNum = params.the_vss_id
                
                for iterator, devNum in command_data:getDevicesIterator() do
                    setTtiLookups0And1AccordingToAppDemo(devNum)
                end
                -- the trunk range in the device ...
                general_resource_manager_init(general_resource_manager_trunk_table,1,NumberOfEntriesIn_TRUNK_table)
            else
                -- the pcl range in xcat3 device ...
                pcl_manager_range_set(0,_3k-1)
                -- the trunk range in xcat3 device ...
                if NumberOfEntriesIn_TRUNK_table < 127 then
                    general_resource_manager_init(general_resource_manager_trunk_table,1,NumberOfEntriesIn_TRUNK_table)
                else
                    general_resource_manager_init(general_resource_manager_trunk_table,1,127)
                end
                -- manager for dummy device numbers
                general_resource_manager_init(vss_dummy_hw_dev_num_for_other_devices,10,31)

                -- on lc use 0 as hwDevNum
                hwDevNum = on_lc_common_hwDevNum
            end

            vss_internal_db_global.my_hw_devNum = hwDevNum
        end


        for iterator, devNum in command_data:getDevicesIterator() do
            local_vss_function__per_device__vss_my_info(command_data,devNum,hwDevNum,unset)
        end

        if unset == true then
            vss_internal_db_global.my_role = nil
            vss_internal_db_global.my_name = nil
            vss_internal_db_global.my_vss_id = nil
            vss_internal_db_global.my_hw_devNum = nil
        end

    end

    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults()
end


--[[
purpose:
Command to state the ‘local vss’ device role
Needed on CC and LC devices.

format:
vss my-info role <cc/lc> name <the_name> vss-id < the_vss_id >

Parameters:
role – the role of this device
cc – Control Card device --  Bc2
lc – line-card device - Ac3
name – the name of this device
	this parameter needed for command that holds several lc devices ... and current lc
need to ignore ‘not me’ configurations ... for unify config at all LCs and the CCs.
(see connection to VSS)
vss-id  - identify this ‘vss’ the several vss.
	(must not be value 0, must not be used by ‘neighbor vss’)
	this parameter relevant to cc device and to lc device
--]]
CLI_addHelp("config", "vss", "VSS:virtual sub-system. support for multi-vss system.")
CLI_addCommand("config", "vss my-info",
    { func = local_vss_function__vss_my_info,
      help = "State current device role and name and vss scope",
      params =
      {
            { type="named",
               { format = "role %type___vss_role",
                        name = "role",
                        help = description___vss_role },
               { format = "name %s",
                        name = "the_name",
                        help = "the name of current device" },
               { format = "vss-id %type___vss_id",
                        name = "the_vss_id",
                        help = description___vss_id },
               mandatory = {"role" , "the_name" , "the_vss_id"}
            }
      }
    })

CLI_addCommand("config", "no vss my-info",
    { func = function(params)
                params.flagNo = true
                return local_vss_function__vss_my_info(params)
             end,
      help = "State current device NOT relate to VSS any more",
      params = {} -- no parameters
    })
-- create empty cascade trunk
local function on_cc_create_empty_cscd_trunk(command_data,devNum,trunkId,unset)

    local portsMembers
    if unset == true then
        portsMembers = nil --> indication to 'remove trunk from DB'
    else
        portsMembers = {} -- no members
    end

    --***************
    --***************
    local apiName = "cpssDxChTrunkCascadeTrunkPortsSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
            { "IN", "GT_U8",    "dev",  devNum   },
            { "IN", "GT_TRUNK_ID",    "trunkId", trunkId  },
            { "IN", "CPSS_PORTS_BMP_STC",  "portsMembersPtr", portsMembers}
        })
end

-- state cpss to not manage the trunkId on the port
local function portTrunkIdMode_manageByApplicationSet(command_data,devNum,portNum,unset)
    local manageMode
    local apiName   
    --***************
    --***************
    if unset == true then
        manageMode = 0
        --manageMode = "CPSS_TRUNK_PORT_TRUNK_ID_MANAGEMENT_MODE_BY_CPSS_E"
    else
        manageMode = 1
        --manageMode = "CPSS_TRUNK_PORT_TRUNK_ID_MANAGEMENT_MODE_BY_APPLICATION_E"
    end

    apiName = "cpssDxChTrunkDbPortTrunkIdModeSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
            { "IN", "GT_U8", "devNum", devNum},
            { "IN", "GT_TRUNK_ID", "portNum", portNum},
            { "IN", "GT_U32", "manageMode", manageMode}
            --{ "IN", "CPSS_TRUNK_PORT_TRUNK_ID_MANAGEMENT_MODE_ENT", "manageMode", manageMode}
        })

end

-- function to add/remove port to/from cascade trunk
local function on_cc_cscd_trunk_add_port(command_data,devNum,trunkId,portNum,unset)
    local apiName = "cpssDxChTrunkCascadeTrunkPortsGet"
    local isFailed, rc, out_param = genericCpssApiWithErrorHandler(command_data,
        apiName, {
            { "IN", "GT_U8",    "dev",  devNum   },
            { "IN", "GT_TRUNK_ID",    "trunkId", trunkId  },
            { "OUT", "CPSS_PORTS_BMP_STC",  "portsMembers"}
        })

    local portsMembers = out_param.portsMembers
    local desigMcMode
    if unset == true then
        -- remove the port from bmp
        portsMembers = wrlCpssUnsetBmpPort(portsMembers, portNum)
        desigMcMode = 1 -- "CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_NATIVE_WEIGHTED_E"
    else
        -- add the port to bmp
        portsMembers = wrLogWrapper("wrlCpssSetBmpPort", "(portsMembers, portNum)", portsMembers, portNum)
        desigMcMode = 2 --"CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_FORCE_ALL_E"
    end

    apiName = "cpssDxChTrunkPortMcEnableSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
            { "IN", "GT_U8",    "dev",  devNum   },
            { "IN", "GT_PHYSICAL_PORT_NUM",    "portNum", portNum  },
            { "IN", "GT_U32",  "mode", desigMcMode}
--            { "IN", "CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_ENT",  "mode", desigMcMode}
        })

    --***************
    --***************
    -- state cpss to not manage the trunkId on the port
    portTrunkIdMode_manageByApplicationSet(command_data,devNum,portNum,unset)

    --***************
    --***************
    apiName = "cpssDxChTrunkCascadeTrunkPortsSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
            { "IN", "GT_U8",    "dev",  devNum   },
            { "IN", "GT_TRUNK_ID",    "trunkId", trunkId  },
            { "IN", "CPSS_PORTS_BMP_STC",  "portsMembersPtr", portsMembers}
        })

end

-- set on cc tti rule
function vss_global_func_on_cc_tti_rule_set(command_data,devNum,myIndexName,finalTtiPattern,finalTtiMask,finalTtiAction,ttiLookupName,from_end_of_tcam,unset)


    local apiName
    local myRuleSize
    local ruleType
    local ttiIndex
    
    --_debug_to_string(command_data,devNum,myIndexName,finalTtiPattern,finalTtiMask,finalTtiAction,ttiLookupName,from_end_of_tcam,unset)

    myRuleSize = 1 -- 10 udbs
    ruleType = "CPSS_DXCH_TTI_RULE_UDB_10_E"

    -- if myIndexName not exists in DB it will give 'new index' and saved into DB
    -- if myIndexName exists in DB it will retrieved from DB
    if from_end_of_tcam == true then
        ttiIndex = allocTtiIndexFromPool_fromEnd(myRuleSize,myIndexName,ttiLookupName)
    else
        ttiIndex = allocTtiIndexFromPool        (myRuleSize,myIndexName,ttiLookupName)
    end

    if unset == true then
        apiName = "cpssDxChTtiRuleValidStatusSet"
        genericCpssApiWithErrorHandler(command_data,
            apiName, {
            { "IN", "GT_U8", "devNum", devNum},
            { "IN", "GT_U32", "index", ttiIndex},
            { "IN", "GT_BOOL", "valid", GT_FALSE}
        })

        freeTtiIndexToPool(ttiIndex)
    else
        apiName = "cpssDxChTtiRuleSet"
        genericCpssApiWithErrorHandler(command_data,
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

--[[
    Create TTI rule to classify VSS UC packet that cames from the neighbor vss:
        a.	purpose : “VSS : classify VSS Packets from neighbor vss = ” .. the_neighbor_vss _name
        b.	Key :
            i.	DSA <srcDev> = vss_hw_dev_num
            ii.	DSA<useVidx> = 0
        c.	Action :
            i.	Bypass_bridge = true
            ii.
--]]
local function on_cc_tti_rule_for_vss_uc_packets(command_data,devNum,neighbor_vss_name,unset)
    local ttiAction = {}
    local finalTtiAction = {}
    local finalTtiPattern = {}
    local finalTtiMask = {}

    local other_vssEntry = vss_internal_db_global.other_vss[neighbor_vss_name]
    local neighbor_vss_id = other_vssEntry.vss_id

    local dsa_useVidx         = nil --don't care --- fixed to nil (from false) since needed for useVidx too !
    local localDevSrcIsTrunk  = nil --don't care
    local localDevSrcPort     = nil --don't care
    local dsa_srcDev          = neighbor_vss_id
    local dsa_srcPort         = nil --don't care
    local pcl_id              = on_cc_pcl_id

    -- build the pattern,mask udbArray
    finalTtiPattern.udbArray,finalTtiMask.udbArray =
        vss_global_func_on_cc_buildTtiPatternMask(dsa_useVidx,localDevSrcIsTrunk,localDevSrcPort,dsa_srcDev,dsa_srcPort,pcl_id)

    -- build the action
    ttiAction.tag1VlanCmd    = "CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E"
    ttiAction.bridgeBypass   = true

    finalTtiAction.type2 = ttiAction

    local myIndexName = "VSS : classify VSS Packets from neighbor vss = " .. neighbor_vss_name .. " to 'bypass bridge'"


    local ttiLookupName = nil
    -- write the entry to HW
    vss_global_func_on_cc_tti_rule_set(command_data,devNum,myIndexName,finalTtiPattern,finalTtiMask,finalTtiAction,ttiLookupName,false,unset)

end

-- create empty (regular) trunk
local function on_lc_create_empty_trunk(command_data,devNum,vss_trunk_id,unset)

    if unset == true then
        -- invalidate the entry by setting '0' enabled members and '0' disabled members
    else
        -- actually since the trunk is 'empty' , we can not make it 'ours'
        -- so wait for actual ports that need to be added to it.
        return
    end

    --***************
    --***************
    local apiName = "cpssDxChTrunkMembersSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName ,{
        { "IN",     "GT_U8",                    "devNum",               devNum },
        { "IN",     "GT_TRUNK_ID",              "trunkId",              vss_trunk_id },
        { "IN",     "GT_U32",                   "numOfEnabledMembers",  0},
        { "IN",     "CPSS_TRUNK_MEMBER_STC",    "enabledMembersArray",  nil},
        { "IN",     "GT_U32",                   "numOfDisabledMembers", 0},
        { "IN",     "CPSS_TRUNK_MEMBER_STC",    "disabledMembersArray", nil}
    })


end
--[[
    set EPCL rules for the used for traffic egress the VSS ports
        a.	NOTE: the next rule need to be added as next 2 keys: (total 2 rules !)
            i.	key_type  = STD_NOT_IP  for non-ip packets
            ii.	key_type  = STD_IP_L2_QOS  for ip packets
        b.	rule #1
            i.	purpose : “VSS : EPCL : For traffic egress the VSS ports , filter those originated on the neighbor vss” .. the_neighbor_vss _name .. “ key type =” .. key_type
            ii.	key
                1.	pcl_id = epcl_pcl_id
                2.	<SrcDev> = vss_hw_dev_num
            iii.	Action:
                1.	Drop (hard_drop)
--]]
local function on_lc_epcl_rule_for_vss_filtering(command_data,devNum,neighbor_vss_name,unset)
    local other_vssEntry = vss_internal_db_global.other_vss[neighbor_vss_name]
    local neighbor_vss_id = other_vssEntry.vss_id

    local myIndexName = "VSS : EPCL : For traffic egress the VSS ports , filter those originated on the neighbor vss" .. neighbor_vss_name

    local finalPclAction = {pktCmd = "CPSS_PACKET_CMD_DROP_HARD_E"}
    local common_pattern = {
                common={valid = 1 , pclId = vss_internal_db_on_lc_used_for_vss_ports_epcl_pcl_id,
                        srcDev = neighbor_vss_id}
                }
    local common_mask = {
                common={valid = 1 , pclId = vss_pcl_id_mask_10_bits,
                        srcDev = vss_on_lc_src_dev_mask_5_bits}
                }

    local shortRuleFormat = "egr_std_not_ip"
    local finalPclPattern = {ruleEgrStdNotIp = common_pattern}
    local finalPclMask    = {ruleEgrStdNotIp = common_mask}

    finalPclPattern = finalPclPattern.ruleEgrStdNotIp
    finalPclMask = finalPclMask.ruleEgrStdNotIp

    vss_global_func_on_lc_pcl_rule_set(command_data,devNum,shortRuleFormat,myIndexName,finalPclPattern,finalPclMask,finalPclAction,false,unset)

    shortRuleFormat = "egr_std_ip_l2_qos"
    finalPclPattern = {ruleEgrStdIpL2Qos = common_pattern}
    finalPclMask    = {ruleEgrStdIpL2Qos = common_mask}

    finalPclPattern = finalPclPattern.ruleEgrStdIpL2Qos
    finalPclMask = finalPclMask.ruleEgrStdIpL2Qos

    vss_global_func_on_lc_pcl_rule_set(command_data,devNum,shortRuleFormat,myIndexName,finalPclPattern,finalPclMask,finalPclAction,false,unset)

end
-- function to set 'per device' the 'vss neighbor-vss-info'
local function local_vss_function__per_device__neighbor_vss_info(command_data,devNum,neighbor_vss_name,unset)
    local other_vssEntry = vss_internal_db_global.other_vss[neighbor_vss_name]
    local vss_trunk_id = other_vssEntry.vss_trunk_id
    local neighbor_vss_id = other_vssEntry.vss_id

    -- Set device map table to point vss_trunk_Id to ‘trunk’ = vss_trunk_Id
    setDevMapTable(command_data,devNum,neighbor_vss_id,vss_trunk_id,nil,unset)

    if vss_internal_db_global.my_role == vss_role_cc_full_name then
        --Create ‘cascade trunk’ with vss_trunk_Id with no ports
        on_cc_create_empty_cscd_trunk(command_data,devNum,vss_trunk_id,unset)
        --set tti rule for vss traffic to 'bypass bridge'
        on_cc_tti_rule_for_vss_uc_packets(command_data,devNum,neighbor_vss_name,unset)
    else
        --Create ‘regular trunk’ with vss_trunk_Id with no ports
        on_lc_create_empty_trunk(command_data,devNum,vss_trunk_id,unset)
        -- set epcl rule for vss loops prevention.
        on_lc_epcl_rule_for_vss_filtering(command_data,devNum,neighbor_vss_name,unset)
    end

end

-- function to set 'vss neighbor-vss-info'
local function local_vss_function__neighbor_vss_info(params)
    local command_data = Command_Data()
    local did_error = false
    local unset = (params.flagNo == true)

    -- Common variables initialization
    command_data:clearResultArray()
    command_data:initAllAvailableDevicesRange()

    -- do not allow to config without proper initialization

    if vss_internal_db_global.my_role == nil then
        add_error(command_data,"VSS : command not valid before calling command 'vss my-info' ")
        did_error = true
    end

    if unset == true then
        if vss_internal_db_global.other_vss[params.the_neighbor_vss_name] == nil then
            add_error(command_data,"VSS : unknown VSS " .. params.the_neighbor_vss_name)
            did_error = true
        end
    end

    if did_error ~= true then

        local maxTrunkId = 127
        if (maxTrunkId > NumberOfEntriesIn_TRUNK_table) then
        	maxTrunkId = NumberOfEntriesIn_TRUNK_table
        end
        if vss_internal_db_global.my_role == vss_role_cc_full_name then
            trunkId = maxTrunkId
        else
            trunkId = ((maxTrunkId+1) / 2) - 1
        end

        general_resource_manager_use_specific_index(general_resource_manager_trunk_table,trunkId)

        if unset == true then
            -- already checked
        else
            vss_internal_db_global.other_vss[params.the_neighbor_vss_name] =
                {vss_id         = params.the_vss_id ,
                 vss_trunk_id   = trunkId}
        end

        for iterator, devNum in command_data:getDevicesIterator() do
            local_vss_function__per_device__neighbor_vss_info(command_data,devNum,params.the_neighbor_vss_name,unset)
        end

        if unset == true then
            -- release the trunkId from the manager
            general_resource_manager_free_specific_index(general_resource_manager_trunk_table,trunkId)
            -- release the DB about this neighbor
            vss_internal_db_global.other_vss[params.the_neighbor_vss_name] = nil
        end

    end

    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults()
end

--[[
purpose:
Command to state the ‘remote vss’ info.
Needed on CC and LC devices.

Context – config

Format:
vss neighbor-vss-info vss-name <the_neighbor_vss_name> vss-id < the_vss_id >

Parameters:
vss-name – the name of the neighbor vss
vss-id  - identify the vss devices in multi-vss system.
		(must not be value 0 , must not be used by ‘local vss’)
--]]
CLI_addCommand("config", "vss neighbor-vss-info",
    { func = local_vss_function__neighbor_vss_info,
      help = "State the 'neighbor vss' info",
      params =
      {
            { type="named",
               { format = "vss-name %s",
                        name = "the_neighbor_vss_name",
                        help = description___neighbor_vss_name },
               { format = "vss-id %type___vss_id",
                        name = "the_vss_id",
                        help = description___vss_id },
               mandatory = {"the_neighbor_vss_name" , "the_vss_id"}
            }
      }
    })

CLI_addCommand("config", "no vss neighbor-vss-info",
    { func =    function(params)
            params.flagNo = true
            return local_vss_function__neighbor_vss_info(params)
        end,
      help = "State that the 'neighbor vss' valid no more",
      params =
      {
            { type="named",
               { format = "vss-name %s",
                        name = "the_neighbor_vss_name",
                        help = description___neighbor_vss_name },
               mandatory = {"the_neighbor_vss_name"}
            }
      }
    })

-- set port as cascade
local function cscd_port_set(command_data,devNum, portNum, unset)
    local portType
    if unset == true then
        portType = "CPSS_CSCD_PORT_NETWORK_E"
    elseif vss_internal_db_global.my_role == vss_role_cc_full_name then
        portType = "CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E"
    else
        portType = "CPSS_CSCD_PORT_DSA_MODE_2_WORDS_E"
    end

    local apiName = "cpssDxChCscdPortTypeSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN", "GT_U8", "devNum", devNum},
        { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
        { "IN", "CPSS_PORT_DIRECTION_ENT", "portDirection", "CPSS_PORT_DIRECTION_BOTH_E"},
        { "IN", "CPSS_CSCD_PORT_TYPE_ENT", "portType", portType}
    })

    -- no need to 'restore' next config when change mode from 'cscd port' to 'network port'
    -- because this is the 'default' state of 'network port' (set by cpssDxChCscdPortTypeSet(...))
    if vss_internal_db_global.my_role == vss_role_cc_full_name and
        unset ~= true
    then
        local ignoreFilter = false

        -- override setting done by cpssDxChCscdPortTypeSet(...)
        -- allow the cascade port to return 'flood' traffic back to it.
        -- regardless to 'per src eports' settings of 'SrcMcFilter'
        -- done by cpssDxChBrgMcPhysicalSourceFilteringEnableSet()
        apiName = "cpssDxChBrgMcPhysicalSourceFilteringIgnoreSet"
        genericCpssApiWithErrorHandler(command_data,
            apiName, {
            { "IN", "GT_U8", "devNum", devNum},
            { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
            { "IN", "GT_BOOL", "ignoreFilter", ignoreFilter}
        })
    end
end

-- on cc : set bypass bridge from cscd port
local function on_cscd_port_bridge_bypass_disable(command_data,devNum, portNum, unset)
    local enable
    if unset == true then
        enable = true
    else
        enable = false -- disable the bypass !!!
    end

    local apiName = "cpssDxChCscdPortBridgeBypassEnableSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN", "GT_U8", "devNum", devNum},
        { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
        { "IN", "GT_BOOL", "enable", enable}
    })

end

-- on cc - set 'default' tti rule for hard-drop for unknown src port from lc
local function on_cc_tti_rule_for_hard_drop_unknown_src_port_from_lc(command_data,devNum, portNum,interface_trunkId,isFirst,unset)
    local ttiAction = {}
    local finalTtiAction = {}
    local finalTtiPattern = {}
    local finalTtiMask = {}


    local localDevSrcIsTrunk
    local localDevSrcPort


    local dsa_useVidx         = nil
    if interface_trunkId ~= nil then
        if isFirst ~= true then
            -- no need to do more then one 'per trunk' rule
            return
        end

        localDevSrcIsTrunk  = true
        localDevSrcPort     = interface_trunkId
    else
        localDevSrcIsTrunk  = false
        localDevSrcPort     = portNum
    end

    local dsa_srcDev          = on_lc_common_hwDevNum
    local dsa_srcPort         = nil --don't care
    local pcl_id              = on_cc_pcl_id

    -- build the pattern,mask udbArray
    finalTtiPattern.udbArray,finalTtiMask.udbArray =
        vss_global_func_on_cc_buildTtiPatternMask(dsa_useVidx,localDevSrcIsTrunk,localDevSrcPort,dsa_srcDev,dsa_srcPort,pcl_id)

    -- build the action
    ttiAction.tag1VlanCmd    = "CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E"
    ttiAction.command        = "CPSS_PACKET_CMD_DROP_HARD_E"

    finalTtiAction.type2 = ttiAction

    vss_the_neighbor_device_name = command_data.vss_the_neighbor_device_name

    local myIndexName = "VSS: (default rule) hard-drop for unknown src port from lc " .. vss_the_neighbor_device_name


    local ttiLookupName = nil
    -- write the entry to HW
    vss_global_func_on_cc_tti_rule_set(command_data,devNum,myIndexName,finalTtiPattern,finalTtiMask,finalTtiAction,ttiLookupName,true,unset)

end

-- set port enabled for ipcl lookup
local function on_lc_on_port_ipcl_configuration_table_set(command_data, devNum, portNum , unset)
    local enable
    if unset == true then
        enable = false
    else
        enable = true
    end

    local apiName = "cpssDxChPclPortIngressPolicyEnable"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN", "GT_U8", "devNum", devNum},
        { "IN", "GT_PORT_NUM", "portNum", portNum},
        { "IN", "GT_BOOL", "enable", enable}
    })


    if enable == true then
        apiName = "cpssDxChPclPortLookupCfgTabAccessModeSet"
        genericCpssApiWithErrorHandler(command_data,
            apiName, {
            { "IN", "GT_U8", "devNum", devNum},
            { "IN", "GT_PORT_NUM", "portNum", portNum},
            { "IN", "CPSS_PCL_DIRECTION_ENT", "direction", "CPSS_PCL_DIRECTION_INGRESS_E"},
            { "IN", "CPSS_PCL_LOOKUP_NUMBER_ENT", "lookupNum", "CPSS_PCL_LOOKUP_0_E"},
            { "IN", "GT_U32", "subLookupNum", 0},
            { "IN", "CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT", "mode", "CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E"}
        })
    end

    local lookupCfg = {groupKeyTypes={}}
    lookupCfg.enableLookup = enable
    lookupCfg.pclId = vss_internal_db_on_lc_used_uplink_ipcl_pcl_id
    lookupCfg.groupKeyTypes.nonIpKey = vss_on_lc_pcl_rule_forma_ing_std_udb_name
    lookupCfg.groupKeyTypes.ipv4Key  = vss_on_lc_pcl_rule_forma_ing_std_udb_name
    lookupCfg.groupKeyTypes.ipv6Key  = vss_on_lc_pcl_rule_forma_ing_std_udb_name

    local interfaceInfo = {devPort = {devNum = 0,portNum = portNum}}-- devNum --> don't care !!!

    apiName = "cpssDxChPclCfgTblSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN", "GT_U8", "devNum", devNum},
        { "IN", "CPSS_INTERFACE_INFO_STC", "interfaceInfoPtr", interfaceInfo},
        { "IN", "CPSS_PCL_DIRECTION_ENT", "direction", "CPSS_PCL_DIRECTION_INGRESS_E"},
        { "IN", "CPSS_PCL_LOOKUP_NUMBER_ENT", "lookupNum", "CPSS_PCL_LOOKUP_0_E"},
        { "IN", "CPSS_DXCH_PCL_LOOKUP_CFG_STC", "lookupCfgPtr", lookupCfg}
    })

end

--iterate on ports and set this 'uplink' as PVE
local function on_lc_set_pve_to_uplink(command_data, devNum,unset)
    local enable
    local apiName
    local dstTrunk
    local dstPort
  
    if unset == true then
        enable = false
    else
        enable = true
    end

    local dstHwDev = vss_internal_db_global.my_hw_devNum

    local connection = vss_internal_db_global.on_lc_info.uplink.connection

    if connection.trunkId ~= nil then
        dstTrunk = true
        dstPort = connection.trunkId
    else
        dstTrunk = false
        dstPort = connection.portNum
    end

    -- set all lc ports as PVE to uplink
    -- the assumption is that the 'cascade ports' set as 'bypass bridge'
    -- so the PVE will not be relevant to traffic ingress cascade ports.
    for portNum = 0,27 do
        apiName = "cpssDxChBrgPrvEdgeVlanPortEnable"
        genericCpssApiWithErrorHandler(command_data,
            apiName, {
            { "IN", "GT_U8", "devNum", devNum},
            { "IN", "GT_PORT_NUM", "portNum", portNum},
            { "IN", "GT_BOOL", "enable", enable},
            { "IN", "GT_PORT_NUM", "dstPort", dstPort},
            { "IN", "GT_HW_DEV_NUM", "dstHwDev", dstHwDev},
            { "IN", "GT_BOOL", "dstTrunk", dstTrunk}
        })
    end

end

-- update 'srcId' entry to add/remove a port from entry
local function updateSrcIdEntry(command_data,devNum,srcId,portNum,addPort)
    local apiName
    if addPort == true then
        apiName = "cpssDxChBrgSrcIdGroupPortAdd"
    else
        apiName = "cpssDxChBrgSrcIdGroupPortDelete"
    end

    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN", "GT_U8", "devNum", devNum},
        { "IN", "GT_U32", "sourceId", srcId},
        { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum}
    })
end

-- on lc : manage the flood from VSS-trunk port to the uplink trunk
-- state that the uplink port can get 'flood' traffic , from the VSS trunk ports.
local function on_lc_uplink_port_src_id_for_flood_from_vss_trunk_set(command_data,devNum, portNum, unset)

    if unset == true then
        -- no more to do .. the port already member of the group
        return
    end

    local source_id = vss_internal_db_global.on_lc_info.uplink.src_id_for_vss_trunk

    -- add the port to the srcId group
    updateSrcIdEntry(command_data,devNum,source_id,portNum,true)

end

-- per port : state the neighbor device that is connected to it.
-- on 'trunk' case : this function called for all ports of the trunk.
local function local_vss_function__per_port_on_port_trunk_neighbor_device_info(command_data, devNum, portNum , interface_trunkId , isFirst , unset)

    local enableLookup
  
    -- set port as cascade
    cscd_port_set(command_data,devNum, portNum, unset)

    if vss_internal_db_global.my_role == vss_role_cc_full_name then
        -- unset bypass bridge from cscd port
        on_cscd_port_bridge_bypass_disable(command_data,devNum, portNum, unset)

        if unset == true then
            enableLookup = false
        else
            enableLookup = true
        end
        -- enable tti lookup on the cascade ports
        tti_multiple_cpssDxChTtiPortLookupEnableSet(command_data,devNum,portNum,enableLookup)

        --set 'default' tti rule for hard-drop for unknown src port from lc
        on_cc_tti_rule_for_hard_drop_unknown_src_port_from_lc(command_data,devNum, portNum,interface_trunkId,isFirst,unset)

    else
        -- set port enabled for ipcl lookup
        on_lc_on_port_ipcl_configuration_table_set(command_data, devNum, portNum , unset)

        -- manage the flood from VSS-trunk port to the uplink port/trunk
        on_lc_uplink_port_src_id_for_flood_from_vss_trunk_set(command_data,devNum, portNum, unset)

    end

end

-- command on 'port/trunk' interface to state the neighbor device that is connected to it.
local function local_vss_function__on_port_trunk_neighbor_device_info(params)
    local command_data = Command_Data()
    local did_error = false
    local unset = (params.flagNo == true)
    local connection
    local neighbor_name_from_db

    -- Common variables initialization.
    command_data:initInterfaceDevPortRange()

    -- in case that we are under 'trunk' the interface_trunkId will hold the 'trunkId'
    -- otherwise it is nil
    -- NOTE: when this is 'trunk' the 'getPortIterator' will iterate on ALL the members of the trunk.
    local interface_trunkId  = getGlobal("ifPortChannel")

    -- do not allow to config without proper initialization
    if vss_internal_db_global.my_role == nil then
        add_error(command_data,"VSS : command not valid before calling command 'vss my-info' ")
        did_error = true
    end

    if unset == true then
        -- can not check any 'params' values
        -- need to find the neighbor that is connected to this interface
        local found_interface = false
        connection = nil
        if vss_internal_db_global.my_role == vss_role_cc_full_name then
            for index,entry in pairs(vss_internal_db_global.neighbors) do
                connection = entry.on_cc_info.connection
                if interface_trunkId ~= nil and connection.trunkId == interface_trunkId then
                    -- the interface is trunk
                    found_interface = true
                    neighbor_name_from_db = index -- the name of the neighbor
                    break
                end

                if interface_trunkId == nil and connection.trunkId == nil then
                    for iterator, devNum, portNum in command_data:getPortIterator() do
                        if portNum == connection.portNum then
                            -- the interface is port
                            found_interface = true
                            break
                        end
                    end

                    if found_interface == true then
                        neighbor_name_from_db = index -- the name of the neighbor
                        break
                    end
                end
            end
        else -- on lc device
            connection = vss_internal_db_global.on_lc_info.uplink.connection
            neighbor_name_from_db = connection.the_neighbor_device_name
            if interface_trunkId ~= nil and connection.trunkId == interface_trunkId then
                -- the interface is trunk
                found_interface = true
            elseif interface_trunkId == nil and connection.trunkId == nil then
                for iterator, devNum, portNum in command_data:getPortIterator() do
                    if portNum == connection.portNum then
                        -- the interface is port
                        found_interface = true
                        break
                    end
                end
            end
        end

        if found_interface == false then
            add_error(command_data,"VSS : current interface is not connected to neighbor device ")
            did_error = true
        end
    else
        -- do not allow same role as own
        if vss_internal_db_global.my_role == params.role then
            add_error(command_data,"VSS : the role of neighbor device is never like own role " .. to_string(params.role))
            did_error = true
        end
    end

    if did_error ~= true then
        if unset == true then

            command_data.vss_the_neighbor_device_name = neighbor_name_from_db
        else
            if vss_internal_db_global.my_role == vss_role_cc_full_name then
                vss_internal_db_global.neighbors[params.the_name] = {on_cc_info = {role = params.role , connection = {}}}
                connection = vss_internal_db_global.neighbors[params.the_name].on_cc_info.connection
            else
                connection = vss_internal_db_global.on_lc_info.uplink.connection
                connection.the_neighbor_device_name = params.the_name
            end

            if interface_trunkId ~= nil then
                connection.trunkId = interface_trunkId
                connection.portNum = nil
            else
                connection.trunkId = nil
            end

            command_data.vss_the_neighbor_device_name = params.the_name
        end


        local isFirst = true
        local savedDevNum = 0

        for iterator, devNum, portNum in command_data:getPortIterator() do
            command_data:clearPortStatus()
            command_data:clearLocalStatus()

            if unset ~= true then
                -- 'connection' variable already set
                if interface_trunkId == nil then
                    if connection.portNum == nil then
                        connection.portNum = portNum
                    else
                        add_error(command_data,"VSS : not support 'ethernet range' ")
                        break
                    end
                end
            end

            -- call configuration per port ...
            local_vss_function__per_port_on_port_trunk_neighbor_device_info(command_data, devNum, portNum , interface_trunkId , isFirst , unset)

            isFirst = false

            savedDevNum = devNum
        end

        if vss_internal_db_global.my_role == vss_role_cc_full_name then
        else
            -- we are done with the ports , so now we can iterate on ports and set this 'uplink' as PVE
            on_lc_set_pve_to_uplink(command_data, savedDevNum,unset)

            -- Set device map table to point the uplink to 'my cc device'
            on_lc_point_uplink_to_my_cc_device(command_data,devNum,unset)
        end

        if unset == true then
            -- need to remove DB info
            if vss_internal_db_global.my_role == vss_role_cc_full_name then
                vss_internal_db_global.neighbors[neighbor_name_from_db] = nil
            else
                -- restore default values as set in file vss_internal_db_manager.lua
                connection.portNum = nil
                connection.trunkId = nil
                connection.the_neighbor_device_name = nil
            end
        end
    end

    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults()
end

--[[
purpose:
Command to state that ‘physical port’/’physical trunk’ connecting current device to neighbor device.

Format:
vss neighbor-info role < cc/lc > name <the_name>

Parameters:
Role :
On Control Card :
role = lc  meaning lc connected on the physical port/trunk
		name  the name of the ‘lc’
role = cc  not valid.

On Line-card :
role = lc  not valid
role = cc  defines the physical interface connecting to the CC device (should be
trunk when multi-cc devices in current VSS).
--]]
CLI_addHelp("interface", "vss", "VSS: state interface related info.")
CLI_addCommand("interface", "vss neighbor-info", {
    func   = local_vss_function__on_port_trunk_neighbor_device_info,
    help   = "Info about the direct connected neighbor to 'my device' in VSS",
    params = {
        { type = "named",
               { format = "role %type___vss_role",
                        name = "role",
                        help = description___vss_role },
               { format = "name %s",
                        name = "the_name",
                        help = "the name of neighbor device" },
            mandatory = { "role", "the_name"}
        }
    }
})

CLI_addCommand("interface", "no vss neighbor-info", {
    func   = function(params)
                params.flagNo = true
                return local_vss_function__on_port_trunk_neighbor_device_info(params)
             end,
    help   = "State this interface no longer directly to other device in VSS",
    params = {} -- no parameters
})

--[[
    Set tti-0 rule (from start-of tcam):
        i.	‘porpose’ = “VSS: re-assign srcEPort “ .. current_eport .. ”for lc” .. < neighbor-name > .. ” lc-port = ” .. lc_port_id
        ii.	Key =
            1.	localDevSrcPort = current_port_in_loop
            2.	DSA <SrcDev> = 0
            3.	DSA <SrcPort> = lc_port_id
        iii.	Action :
            1.	Re-assign srcEPort = current_eport
--]]
function vss_on_cc_tti_rule_for_reassign_srcEPort(
    command_data, devNum, my_localDevSrcPort, assignedEPort, primaryEPort, unset)
    local ttiAction = {}
    local finalTtiAction = {}
    local finalTtiPattern = {}
    local finalTtiMask = {}

    local eportInfo = vss_internal_db_global.on_cc_info.eports[primaryEPort]

    local lc_device_name  = eportInfo.lc_name
    local lc_portNum      = eportInfo.lc_portNum

    local dsa_useVidx         = nil --don't care
    local localDevSrcIsTrunk  = false
    local localDevSrcPort     = my_localDevSrcPort
    local dsa_srcDev          = on_lc_common_hwDevNum
    local dsa_srcPort         = lc_portNum
    local pcl_id              = on_cc_pcl_id

    -- build the pattern,mask udbArray
    finalTtiPattern.udbArray,finalTtiMask.udbArray =
        vss_global_func_on_cc_buildTtiPatternMask(dsa_useVidx,localDevSrcIsTrunk,localDevSrcPort,dsa_srcDev,dsa_srcPort,pcl_id)

    --_debug_to_string("pattern",finalTtiPattern.udbArray,"mask",finalTtiMask.udbArray)

    -- build the action
    ttiAction.tag1VlanCmd    = "CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E"
    ttiAction.sourceEPortAssignmentEnable = true
    ttiAction.sourceEPort = assignedEPort

    finalTtiAction.type2 = ttiAction

    local myIndexName = "VSS : re-assign srcEPort " .. assignedEPort .. " for lc " .. lc_device_name .. " lc-port = " .. lc_portNum

    local ttiLookupName = nil
    -- write the entry to HW
    vss_global_func_on_cc_tti_rule_set(command_data,devNum,myIndexName,finalTtiPattern,finalTtiMask,finalTtiAction,ttiLookupName,false,unset)

end

local function on_cc_tti_rule_for_reassign_srcEPort(
    command_data, devNum, my_localDevSrcPort, eportNum, unset)
    return vss_on_cc_tti_rule_for_reassign_srcEPort(
        command_data, devNum, my_localDevSrcPort, eportNum, eportNum, unset)
end

-- on cc : common config on eport in VSS
-- set controlled learning , NA to cpu , 'disable' McPhysicalSourceFiltering (allow flood back to physical port - cscd port)
function vss_global_func_on_cc_set_common_config_eport(command_data, devNum,eportNum,unset)
    local apiName
    local enable, learn_status, learn_cmd
  
    if unset == true then
        enable = false
        learn_status = true
    else
        enable = true
        learn_status = false
    end
    learn_cmd = "CPSS_LOCK_FRWRD_E"
    --
    -- enable 'controlled learning' + NA to CPU
    --***************
    --***************
    apiName = "cpssDxChBrgFdbNaToCpuPerPortSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN", "GT_U8", "devNum", devNum},
        { "IN", "GT_PORT_NUM", "portNum", eportNum},
        { "IN", "GT_BOOL", "enable", enable}
    })

    --***************
    --***************
    apiName = "cpssDxChBrgFdbPortLearnStatusSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN", "GT_U8", "devNum", devNum},
        { "IN", "GT_PORT_NUM", "portNum", eportNum},
        { "IN", "GT_BOOL", "status", learn_status},
        { "IN", "CPSS_PORT_LOCK_CMD_ENT", "cmd", learn_cmd }
    })


    if unset == true then
        enable = true
    else
        enable = false
    end
    --***************
    --***************
    -- allow src eport flood back to it's physical port (cascade port)
    -- note : this  will have indication also in the egress eDSA .
    apiName = "cpssDxChBrgMcPhysicalSourceFilteringEnableSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN", "GT_U8", "devNum", devNum},
        { "IN", "GT_PORT_NUM", "portNum", eportNum},
        { "IN", "GT_BOOL", "enable", enable }
    })

end

-- on cc state the 'lc port' that eport represents
local function on_cc_on_eport_of_port_on_lc_info(command_data,devNum,eportNum,unset)
    local eportInfo = vss_internal_db_global.on_cc_info.eports[eportNum]

    local lc_device_name  = eportInfo.lc_name
    local lc_portNum      = eportInfo.lc_portNum

    local connection = vss_internal_db_global.neighbors[lc_device_name].on_cc_info.connection

    if connection.portNum == nil then
        add_error(command_data,"VSS : cc currently need to be connected by single port to lc (not by trunk)")
        return
    end

    local connection_port_num = connection.portNum

    --set tti to re-assign srcEPort
    on_cc_tti_rule_for_reassign_srcEPort(command_data,devNum,connection_port_num,eportNum,unset)

    -- set e2phy from the eport to the 'connection port'
    e2phySet(command_data,devNum,eportNum,connection_port_num,unset)

    -- set common eport config
    vss_global_func_on_cc_set_common_config_eport(command_data, devNum,eportNum,unset)

end


-- Command to state on CC device the eports representing the ‘physical ports’ on lc device (on local vss)
local function local_vss_function__on_eport_of_port_on_lc_info(params)
    local command_data = Command_Data()
    local did_error = false
    local unset = (params.flagNo == true)
    local eportInfo
    
    -- prepare dev,port iterators
    command_data:initInterfaceDevPortRange()

    -- do not allow to config without proper initialization
    if vss_internal_db_global.my_role ~= vss_role_cc_full_name then
        add_error(command_data,"VSS : command not valid when 'vss my-info' is not 'cc device'")
        did_error = true
    end

    if did_error ~= true then
        if unset == true then
            -- we need to remove configurations
            -- get the info about the 'lc neighbor device' and the 'lc port'

            -- check that the 'lc device' is known neighbor
        elseif vss_internal_db_global.neighbors[params.the_name] == nil then
            add_error(command_data,"VSS : the 'lc neighbor device' " .. params.the_name .. " was not defined by command 'vss neighbor-info'")
            did_error = true
        end
    end

    if did_error ~= true then

        -- configure the port with generic iterator
        for dummy, devNum, eportNum in command_data:getPortIterator() do
            command_data:clearPortStatus()
            command_data:clearLocalStatus()

            if eportNum >= vss_global_eport_range_start_eport and
               eportNum <= vss_global_eport_range_end_eport then
                add_error(command_data,"VSS : current command not valid for 'global eports' , in range [" .. vss_global_eport_range_start_eport .. ".." .. vss_global_eport_range_end_eport .. "]")
                did_error = true
                break
            end


            eportInfo = vss_internal_db_global.on_cc_info.eports[eportNum]

            if unset == true then
                if eportInfo == nil then
                    add_error(command_data,"VSS : the eport " .. eportNum .. " not bound to lc device and port")
                    did_error = true
                    break
                end
            else
                if eportInfo and eportInfo.lc_name and eportInfo.lc_portNum then
                    add_error(command_data,"VSS : the eport " .. eportNum .. " already in use by " .. eportInfo.lc_name .. " port " .. eportInfo.lc_portNum)
                    did_error = true
                    break
                end

                if eportInfo == nil then
                    eportInfo = {}
                end

                -- set the eport with the info
                eportInfo.lc_name = params.the_name
                eportInfo.lc_portNum = params.lc_port_id

                vss_internal_db_global.on_cc_info.eports[eportNum] = eportInfo
            end

            -- act on the eport
            on_cc_on_eport_of_port_on_lc_info(command_data,devNum,eportNum,unset)

            if unset == true then
                -- remove the eport from the DB
                vss_internal_db_global.on_cc_info.eports[eportNum] = nil
            end

            command_data:updateStatus()
            command_data:updatePorts()
        end
    end

    -- handle all the errors that may come from calling the function
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
 end

--[[
purpose:
Command to state on CC device the eports representing the ‘physical ports’ on lc device (on local vss)
Needed only on CC (and not on LC devices).

Format:
vss port-on-lc lc-name <the_name> lc-port <lc_port_id>

Notes:
1.	For support of  ‘network trunks’ with lc ports , see command aggregate-eports

--]]
CLI_addHelp({"interface", "interface_eport"}, "vss", "VSS: state interface related info.")
CLI_addCommand({"interface", "interface_eport"}, "vss port-on-lc", {
    func   = local_vss_function__on_eport_of_port_on_lc_info,
    help   = "(on cc device) The eport represent a 'physical port' on lc device (on local vss)",
    params = {
        { type = "named",
               { format = "lc-name %s",
                        name = "the_name",
                        help = "the name of the neighbor lc device" },
               { format = "lc-port %type___vss_lc_port_range",
                        name = "lc_port_id",
                        help = description___vss_on_lc_port_range },
            mandatory = { "the_name", "lc_port_id"}
        }
    }
})
CLI_addCommand({"interface", "interface_eport"}, "no vss port-on-lc", {
    func   = function(params)
                params.flagNo = true
                return local_vss_function__on_eport_of_port_on_lc_info(params)
            end
        ,
    help   = "(on cc device) delete the eport represent a 'physical port' on lc device (on local vss)",
    params = {} -- no parameters
})


-- Assignment Source ID for MC Source Filtering
local function on_lc_pcl_rule_for_src_id_for_mc_source_filtering(command_data,devNum,eport,srcId,unset)
    --***************
    --***************
    --[[ a.	set IPCL rule :
            i.	purpose : “VSS : Assignment Source ID for MC Source Filtering for eport = ” .. the_eport
            ii.	Key:
                1.	pcl_id = vss_internal_db_global. on_lc_info.uplink.pcl_id
                2.	eDSA<Use eVIDX>=1
                3.	eDSA<Src ePort> = the_eport
                4.	 eDSA<Routed>=0
            iii.	Action:
                1.	Source-ID = source_id
    --]]

    local myIndexName = "VSS : Assignment Source ID for MC Source Filtering for eport = " .. eport

    -- make the PCL action to modify only the srcId
    local finalPclAction = {sourceId = {assignSourceId = true , sourceIdValue = srcId}}

    local pattern_udb0_15,mask_udb0_15 =
        vss_global_func_on_lc_buildIpclPatternMask(
            true,--useVidx
            nil,--vidx
            nil,--trgDev
            nil,--trgEPort
            eport,--srcEport
            false)--routed

    -- define pattern , for 'STD UDB' key
    local common_Ingr_Udb_pattern = {}
    common_Ingr_Udb_pattern.pclId = vss_internal_db_on_lc_used_uplink_ipcl_pcl_id

    -- define mask , for 'STD UDB' key
    local common_Ingr_Udb_mask = {}
    common_Ingr_Udb_mask.pclId = vss_pcl_id_mask_10_bits

    local finalPclPattern = {ruleIngrStdUdb = {commonIngrUdb = common_Ingr_Udb_pattern , udb0_15 = pattern_udb0_15}}
    local finalPclMask    = {ruleIngrStdUdb = {commonIngrUdb = common_Ingr_Udb_mask , udb0_15 = mask_udb0_15}}

    finalPclPattern = finalPclPattern.ruleIngrStdUdb
    finalPclMask = finalPclMask.ruleIngrStdUdb

    -- call to write the rule to HW
    vss_global_func_on_lc_pcl_rule_set(command_data,devNum,"ing_std_udb",myIndexName,finalPclPattern,finalPclMask,finalPclAction,false,unset)


end

-- on lc : check if port already in aggregation
local function on_lc_eport_aggregator_is_member_already_exists(command_data,params,devNum,current_member,unset)
    for index,entry in pairs(vss_internal_db_global.on_lc_info.members_in_aggregations) do

        local member = entry.member

        if member.neighbor_vss_name == current_member.neighbor_vss_name and
           member.lc_name == current_member.lc_name and
           member.lc_port == current_member.lc_port
        then
            if entry.eport_aggregator == params.the_global_eport then
                if unset ~= true then
                    -- port already member in current aggregation .. no more to do
                    add_error(command_data,"VSS : port already member in current aggregation")
                end

                return true
            else
                -- port already member in aggregation .. not allowed
                add_error(command_data,"VSS : port is member in another aggregation [" .. entry.eport_aggregator.."] (allowed only in single aggregation)")
                return true
            end
        end
    end

    if unset == true then
        add_error(command_data,"VSS : port is not member in current aggregation")
    end

    return false
end

-- on lc : update the aggregation DB
local function on_lc_eport_aggregator_update_db(command_data,params,current_member, current_aggregator)
    vss_internal_db_global.on_lc_info.eports_aggregator[params.the_global_eport] = current_aggregator
    local count
    local new_entry
    
    if params.the_action == vss_add_port_full_name then
        -- add new member
        count = #vss_internal_db_global.on_lc_info.members_in_aggregations
        new_entry = {member = current_member , eport_aggregator = params.the_global_eport}
        vss_internal_db_global.on_lc_info.members_in_aggregations[count+1] = new_entry
     else
        -- remove the member
        for index,entry in pairs(vss_internal_db_global.on_lc_info.members_in_aggregations) do
            if entry.member.neighbor_vss_name == current_member.neighbor_vss_name and
               entry.member.lc_name == current_member.lc_name and
               entry.member.lc_port == current_member.lc_port
            then
                -- entry found
                -- remove the member
                vss_internal_db_global.on_lc_info.members_in_aggregations[index] = nil
            end
        end
        list_table_inplace_compress(
            vss_internal_db_global.on_lc_info.members_in_aggregations)
    end
end

-- set member in the eport aggregation in the trunk
-- return the trunkId
local function on_lc_vss_eport_aggregator_member_trunk(command_data,devNum,current_member,eport,unset)
  local apiName
  local is_local_port
  local hwDevNum
  
    if current_member.neighbor_vss_name == nil and
       current_member.lc_name == vss_internal_db_global.my_name then
        is_local_port = true
    else
        is_local_port = false
    end

    --***************
    --***************
    if is_local_port == true then
        -- state cpss to not manage the trunkId on the port
        portTrunkIdMode_manageByApplicationSet(command_data,devNum,current_member.lc_port,unset)
    end

    local table_name = general_resource_manager_trunk_table
    local key_name = "VSS : trunk for vss_eport_aggregator , eport = " .. eport
    -- get trunk id from the general manager
    trunkId = general_resource_manager_entry_get_in_limited_range(table_name,key_name,5)-- start trunks from 5 (not from 1)
    local neighbor_vss_name
    if current_member.neighbor_vss_name == nil then
        neighbor_vss_name = ""
    else
        neighbor_vss_name = current_member.neighbor_vss_name .. " "
    end

    if is_local_port == true then
        -- member from my device
        hwDevNum = vss_internal_db_global.my_hw_devNum
    else
        --member from  other device
        table_name = vss_dummy_hw_dev_num_for_other_devices
        key_name = neighbor_vss_name .. current_member.lc_name
        hwDevNum = general_resource_manager_entry_get(table_name,key_name)-- start devices from 10
    end

    local trunk_member = {device = hwDevNum, port = current_member.lc_port}


    --***************
    --***************
    if unset == true then
        apiName = "cpssDxChTrunkMemberRemove"
    else
        apiName = "cpssDxChTrunkMemberAdd"
    end

    genericCpssApiWithErrorHandler(command_data,
        apiName, {
            { "IN", "GT_U8", "devNum", devNum},
            { "IN", "GT_TRUNK_ID", "trunkId", trunkId},
            { "IN", "CPSS_TRUNK_MEMBER_STC", "memberPtr", trunk_member }
        })

    return trunkId
end

-- remove portNum from the cpssPortsBmp
function wrlCpssUnsetBmpPort(cpssPortsBmp, portNum)
    local wordArr = cpssPortsBmp["ports"];

    local wordIndex = math.floor(portNum / 32)
    local bitIndex = portNum % 32
    local mask = 0xFFFFFFFF - bit_shl(1,bitIndex)

    wordArr[wordIndex] = bit_and(wordArr[wordIndex],mask)

    return {ports = wordArr};
end

-- set member in the eport aggregation in the 'designated table' -- to avoid flooding to it on ALL cases !!!
-- this is done when there are more then 8 members in the aggregation
local function on_lc_vss_eport_aggregator_member_not_in_trunk(command_data,devNum,current_member,eport,unset)
    local is_local_port
    local apiName
    
    if current_member.neighbor_vss_name == nil and
       current_member.lc_name == vss_internal_db_global.my_name then
        is_local_port = true
    else
        is_local_port = false
    end

    if is_local_port == false then
        -- not relevant to current device
        return
    end

    for entryIndex = 0 , vss_on_lc_designated_table_size do
        --***************
        --***************
        apiName = "cpssDxChTrunkDesignatedPortsEntryGet"
        local isFailed,rc,out_param = genericCpssApiWithErrorHandler(command_data,
            apiName, {
                { "IN", "GT_U8", "devNum", devNum},
                { "IN", "GT_U32", "entryIndex", entryIndex},
                { "OUT", "CPSS_PORTS_BMP_STC", "designatedPorts"}
            })

        local designatedPorts = out_param.designatedPorts

        if unset == true then
            -- add the port to bmp
            local lc_port_ = current_member.lc_port
            designatedPorts = wrLogWrapper("wrlCpssSetBmpPort", "(designatedPorts, lc_port_)", designatedPorts, lc_port_)
        else
            -- remove the port from bmp
            designatedPorts = wrlCpssUnsetBmpPort(designatedPorts, current_member.lc_port)

--[[
            if entryIndex == 0 then
                _debug_to_string("designatedPorts = " , designatedPorts)
            end
--]]
        end

        --***************
        --***************
        apiName = "cpssDxChTrunkDesignatedPortsEntrySet"
        genericCpssApiWithErrorHandler(command_data,
            apiName, {
                { "IN", "GT_U8", "devNum", devNum},
                { "IN", "GT_U32", "entryIndex", entryIndex},
                { "IN", "CPSS_PORTS_BMP_STC", "designatedPortsPtr", designatedPorts}
            })
    end

end

-- on lc : 'per device' set add/remove port to/from global eport .
local function on_lc_per_device_vss_eport_aggregator(command_data,params,devNum)
    local unset = not (params.the_action == vss_add_port_full_name)
    local is_local_port
    local source_id, is_first_local_port, is_last_local_port, is_last_port_removed, is_first_local_port_removed 
    
    if params.the_neighbor_vss_name == nil and params.the_lc_name == vss_internal_db_global.my_name then
        is_local_port = true
    else
        is_local_port = false
    end

    local first_local_port = nil
    local second_local_port = nil
    local current_member_exist_in_db = false
    local count_local_ports = 0
    local member_exist_in_index = 0 -- index 0 is not valid !!!

    local current_member = {  neighbor_vss_name = params.the_neighbor_vss_name ,
                    lc_name = params.the_lc_name,
                    lc_port = params.lc_port_id}


    -- check if port already in aggregation
    on_lc_eport_aggregator_is_member_already_exists(command_data,params,devNum,current_member,unset)
    if is_error(command_data) then
        -- check if on_lc_eport_aggregator_is_member_already_exists raised error
        return
    end

    local current_aggregator = vss_internal_db_global.on_lc_info.eports_aggregator[params.the_global_eport]

    if current_aggregator == nil then
        current_aggregator = {members = {}}
    end

    -- loop on already existing members of the aggregation
    local members = current_aggregator.members
    for index,entry in pairs(members) do
        if entry.neighbor_vss_name == params.the_neighbor_vss_name and
           entry.lc_name == params.the_lc_name
        then
            if entry.lc_port == params.lc_port_id then
                current_member_exist_in_db = true
                -- save the index
                member_exist_in_index = index
            end

            if entry.neighbor_vss_name == nil and -- local vss
               entry.lc_name == vss_internal_db_global.my_name -- my name
            then
                if first_local_port == nil then -- not assigned first port already
                    first_local_port = entry.lc_port
                elseif second_local_port == nil then -- not assigned second port already
                    second_local_port = entry.lc_port
                end
                -- increment number of 'local ports'
                count_local_ports = count_local_ports + 1
            end
        end
    end

    local current_num_of_members = #members


    if params.the_action == vss_add_port_full_name then

        if current_num_of_members < vss_on_lc_designated_table_size then
            -- add the member to the trunk to be as member in the 'flood' load balance
            current_aggregator.trunkId =
                on_lc_vss_eport_aggregator_member_trunk(command_data,devNum,current_member,params.the_global_eport)
        else
            -- set the member to not be part of flooding in vlan (the first 8 members will get flood)
            on_lc_vss_eport_aggregator_member_not_in_trunk(command_data,devNum,current_member,params.the_global_eport)
        end

        
        if (first_local_port == nil and is_local_port == true) then
            is_first_local_port =  true
        else
            is_first_local_port =  false
        end

        -- add new member as 'last'
        -- in lua index start from 1
        current_aggregator.members[current_num_of_members + 1] = current_member

        if is_first_local_port == true then
            source_id = vss_global_func_convert_port_to_src_id(params.lc_port_id)
            current_aggregator.source_id = source_id
        else
            -- not the first 'local port' on 'own device'
            source_id = current_aggregator.source_id
        end

        if is_local_port == true and is_first_local_port == false then
            -- remove the port from the srcId
            updateSrcIdEntry(command_data,devNum,source_id,params.lc_port_id,false)
        end

        if is_first_local_port == true then
            -- Assignment Source ID for MC Source Filtering
            on_lc_pcl_rule_for_src_id_for_mc_source_filtering(
                command_data,devNum,params.the_global_eport,source_id)
        end

    else -- remove port
        if current_member_exist_in_db == false then
            -- the port is not in the aggregation
            add_error(command_data,"VSS : can't remove non-exist member from the aggregation")
            return
        end

        if count_local_ports == 1 and is_local_port == true then
            is_last_local_port = true
        else
            is_last_local_port = false
        end

        if current_num_of_members == 1 then
            is_last_port_removed = true
        else
            is_last_port_removed = false
        end

        if is_local_port == true and params.lc_port_id == first_local_port then
            is_first_local_port_removed = true
        else
            is_first_local_port_removed = false
        end

        -- update the members about the removed member
        current_aggregator.members[member_exist_in_index] = nil
        list_table_inplace_compress(current_aggregator.members)

        if member_exist_in_index <= vss_on_lc_designated_table_size then
            -- remove the member from the trunk  -- for 'flood' load balance
            on_lc_vss_eport_aggregator_member_trunk(command_data,devNum,current_member,params.the_global_eport,true)
            -- add another member to replace it
            if current_num_of_members > vss_on_lc_designated_table_size then
                local member = current_aggregator.members[vss_on_lc_designated_table_size]
                -- add the member to the trunk to be as member in the 'flood' load balance
                on_lc_vss_eport_aggregator_member_trunk(command_data,devNum,current_member,params.the_global_eport)
            end
        else
            -- restore  the member to be part of flooding in vlan
            on_lc_vss_eport_aggregator_member_not_in_trunk(command_data,devNum,current_member,params.the_global_eport,true)
        end

        if is_local_port == true and is_first_local_port_removed == false then
            -- add the removed port to the 'current_aggregator.source_id'
            updateSrcIdEntry(command_data,devNum,current_aggregator.source_id,params.lc_port_id,true)
        end

        if second_local_port ~= nil and is_first_local_port_removed == true then
            -- we need to replace src-id since source-id was associated to the first port , that we remove
            local new_source_id = vss_global_func_convert_port_to_src_id(second_local_port)
            local old_source_id = current_aggregator.source_id
            current_aggregator.source_id = new_source_id
            -- update src_id groups membership of all local ports
            for index,entry in pairs(current_aggregator.members) do
                if entry.neighbor_vss_name == params.the_neighbor_vss_name and
                    entry.lc_name == params.the_lc_name and
                    entry.neighbor_vss_name == nil and -- local vss
                    entry.lc_port ~= first_local_port
                then
                    -- add the port to the old srcId group
                    updateSrcIdEntry(command_data,devNum,old_source_id,entry.lc_port,true)
                    if entry.lc_port ~= second_local_port then
                        -- remove the port from the new srcId group
                        updateSrcIdEntry(command_data,devNum,new_source_id,entry.lc_port,false)
                    end
                end
            end

            -- update the Assignment Source ID for MC Source Filtering
            on_lc_pcl_rule_for_src_id_for_mc_source_filtering(
                command_data,devNum,params.the_global_eport, new_source_id)
        end

        if is_last_local_port == true then
            -- remove the pcl rule
            on_lc_pcl_rule_for_src_id_for_mc_source_filtering(
                command_data,devNum,params.the_global_eport,0,true)
            -- remove sourece id from DB
            current_aggregator.source_id = nil
        end

        if is_last_port_removed == true then
            current_aggregator = nil
        end
    end

    -- update the db
    on_lc_eport_aggregator_update_db(command_data,params,current_member, current_aggregator)

end

-- on lc : set add/remove port to/from global eport .
local function local_vss_function__vss_eport_aggregator(params)
    local command_data = Command_Data()
    local did_error = false

    -- Common variables initialization
    command_data:clearResultArray()
    command_data:initAllAvailableDevicesRange()

    -- do not allow to config without proper initialization
    if vss_internal_db_global.my_role ~= vss_role_lc_full_name then
        add_error(command_data,"VSS : command not valid when 'vss my-info' is not 'lc device'")
        did_error = true
    end

    if did_error ~= true then
        for iterator, devNum in command_data:getDevicesIterator() do
            on_lc_per_device_vss_eport_aggregator(command_data,params,devNum)
        end
    end

    -- handle all the errors that may come from calling the function
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end

--[[
command to state for ‘lc-device’ the eport that used as ‘eports aggregator’ for relevant eports that reside on all ‘lc-devices’ across the VSS .
This ‘eports aggregator’ is similar to ‘channel-group’ but it ports from different devices.
Important : the ‘members’ should be ONLY those that should participate in ‘mc traffic load balance’ !

Note :
1.	Port aggregator must be in range : ‘0x1100-0x1111’ (as ‘global eports on CC device)
2.	The eports of ‘current’ lc must be ‘known’ prior to this call.
3.	The eports from  ‘non-current’ lc are not checked for ‘existence’

Needed only on LC (and not on CC devices … for CC see command aggregate-eports).

Format:
vss eport-aggregator eport <the_eport> action <add/remove> lc-name <the_lc_name>
lc-port <the_lc_port> [neighbor-vss-name < the_neighbor_vss _name >]

Parameters:
the_eport – the ‘eport-aggregator’  (‘global-eport’ is the eport of the ‘context’)
action – add/remove a port from the aggregator , options are
	add  - add the port to be member in traffic load balance , sent towards the
		‘the_eport’ .
	remove – remove the port from traffic load balance , sent towards the ‘the_eport’ .

lc-name – the name of the lc (can be ‘my name’)
lc-port – the port number on the ‘lc-name’ device
Optional parameter :
neighbor-vss-name – the name of the neighbor vss  . needed when adding port from
lc-device from neighbor vss to the ‘eport aggregator’.
(when parameter  omitted  ‘lc-name’ device considered as lc of
 ‘local-vss’)

--]]
CLI_addCommand("config", "vss eport-aggregator",
    { func = local_vss_function__vss_eport_aggregator,
      help = "(on lc) State the eport that aggregate ports on lc devices (like port-channel)",
      params =
      {
            { type="named",
               { format = "eport %type___vss_global_eport",
                        name = "the_global_eport",
                        help = description___vss_global_eport_range },
               { format = "action %type___vss_add_remove_port",
                        name = "the_action",
                        help = description___vss_add_remove_port },
               { format = "lc-name %s",
                        name = "the_lc_name",
                        help = "the name of the neighbor lc device (or 'mine')" },
               { format = "lc-port %type___vss_lc_port_range",
                        name = "lc_port_id",
                        help = description___vss_on_lc_port_range },
               { format = "vss-name %s",
                        name = "the_neighbor_vss_name",
                        help = "Optional parameter: " .. description___neighbor_vss_name },
               mandatory = {"the_global_eport" , "the_action", "the_lc_name", "lc_port_id" }
            }
      }
    })


-- set IPCL rule to redirect packet intented to {trgDevNum, ePort}
-- on {local devNum, physical portNum}
local function on_lc_pcl_rule_for_uc_redirect_to_local_port (command_data, trgDevNum, trgEPort, devNum, portNum, unset)
      -- set IPCL rule :
      --    a.	Purpose = "VSS: UC Forwarding Rule to local network port = " .. current_port .. " trg ePort = " .. trgEPort
      --    b.	Key:
      --       1.  pcl_id = vss_internal_db_global.on_lc_info.uplink.pcl_id
      --       2.  eDSA <UseVIDX>=0
      --       3.	eDSA <TrgDev>= trgHwDev
      --       4.	eDSA <Trg ePort> = trgEPort
      --    c.	action :
      --       1.	Redirect to TrgDev=0 TrgPort= current_port
      local myIndexName = "VSS: UC Forwarding Rule to local network port = " ..
         portNum .. " trg ePort = " .. trgEPort

      -- action: redirect packets to current physical port
      local action = {
         redirect = {
            redirectCmd = "CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E",
            data = { -- union
                outIf = {
                   outInterface = {
                      type = "CPSS_INTERFACE_PORT_E",
                       devPort = { devNum = vss_internal_db_global.my_hw_devNum , portNum = portNum }
      }}}}}
      local pattern_udb0_15, mask_udb0_15 =
         vss_global_func_on_lc_buildIpclPatternMask(
            false,--useVidx
            nil,--vidx
            trgDevNum, --trgDev
            trgEPort,--trgEPort
            nil,--srcEport
            false)--routed

      local pattern = {ruleIngrStdUdb = {
                          commonIngrUdb = {
                             pclId = vss_internal_db_on_lc_used_uplink_ipcl_pcl_id
                          },
                          udb0_15 = pattern_udb0_15 }}

      local mask = { ruleIngrStdUdb = {
                        commonIngrUdb = {
                           pclId      = vss_pcl_id_mask_10_bits
                        },
                        udb0_15 = mask_udb0_15 }}

      pattern = pattern.ruleIngrStdUdb
      mask = mask.ruleIngrStdUdb

      -- write rule to HW
      vss_global_func_on_lc_pcl_rule_set(command_data, devNum,
                                         "ing_std_udb", myIndexName,
                                         pattern, mask, action,
                                         false, unset)
end


--------------------------------------------------------------------------------
-- on lc : map eport used by the 'cc device' as TrgEPort to local network port
local function local_vss_function__vss_my_eport_on_cc(params)
   local command_data = Command_Data()

   --------------callback used in the cycle through devices/ports---------------
   local func = function(command_data, devNum, portNum, params)
      local eport_on_cc
      local unset = (params.flagNo == true)
      if unset == true then
        if vss_internal_db_global.on_lc_info.physical_port_info[portNum] == nil then
            add_error(command_data,"VSS : eport is not associated with current physical interface")
            return
        end

        eport_on_cc = vss_internal_db_global.on_lc_info.physical_port_info[portNum].eport_on_cc
      else
        eport_on_cc = params.eport
      end
      local source_id = vss_global_func_convert_port_to_src_id(portNum)

      -- set DB fields
      vss_internal_db_global.on_lc_info.physical_port_info[portNum] = {
         eport_on_cc = eport_on_cc, source_id = source_id}

      -- set IPCL rule to redirect packet to local port.
      on_lc_pcl_rule_for_uc_redirect_to_local_port (
         command_data,
         vss_internal_db_global.my_vss_id, -- trgDevNum
         eport_on_cc,                      -- trgEPort
         devNum,
         portNum,
         unset
      )

      -- exclude current port from sourceId = portNum + 1
      updateSrcIdEntry(command_data, devNum, source_id, portNum, unset)

      -- add IPCL rule for MC source-id filtering
      on_lc_pcl_rule_for_src_id_for_mc_source_filtering(
         command_data, devNum, eport_on_cc, source_id, unset)

      if unset == true then
        vss_internal_db_global.on_lc_info.physical_port_info[portNum] = nil
      end
   end -- func

----------------------- A FUNCTION BODY ----------------------------------------
   -- do not allow to config without proper initialization
    if vss_internal_db_global.my_role ~= vss_role_lc_full_name then
        add_error(command_data,"VSS : command not valid when 'vss my-info' is not 'lc device'")
    else
       command_data:initInterfaceDevPortRange()
       -- command can be run only for single port not for range of ports
       if command_data:getPortCounts() ~= 1 then
          add_error(command_data, "command can't be applied to the range of ports!")
       else
          command_data:iterateOverPorts(func, params)
       end
    end

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()

end

--[[
Command to state for 'lc-device' the eport that used by the 'cc device'
to represent the current physical port (needed also for ports members in user-trunk).
NOTE: the 'eport' are those that stated on the CC device .
   1.	the CC assigned eport in command port-on-lc.
   2.	NOTE: NOT needed for trunk that CC assigned eport in command aggregate-eports
Needed only on LC (and not on CC devices).
Context - interface of 'ethernet'
Format:
   vss my-eport-on-cc <the_eport>
Parameters:
   The_eport - the eport on CC that represents current physical port
--]]
CLI_addCommand({"interface", "ethernet"}, "vss my-eport-on-cc", {
  func   = local_vss_function__vss_my_eport_on_cc,
  help   = "state for 'lc-device' the ePort that is used by the 'cc device' to"..
           " represent the current physical port",
  params = {
     {type="values", {format="%ePort", name = "eport"}}
     }
  }
)

CLI_addCommand({"interface", "ethernet"}, "no vss my-eport-on-cc", {
  func   =  function(params)
                params.flagNo = true
                return local_vss_function__vss_my_eport_on_cc(params)
            end,
  help   = "remove from 'lc-device' the ePort that is used by the 'cc device' to"..
           " represent the current physical port",
  params = {}-- no parameters
})

-- on lc : add/remove trunk member from the vss trunk
-- trunk_member - hold 2 fields :
--  port    --> port number
--  device --> hwDevNum
local function on_lc_trunk_add_port(command_data,devNum,is_local_port,trunk_member,unset)

    local apiName
    
    --***************
    --***************
    if is_local_port == true then
        -- state cpss to not manage the trunkId on the port
        portTrunkIdMode_manageByApplicationSet(command_data,devNum,trunk_member.port,unset)
    end

    --***************
    --***************
    if unset == true then
        apiName = "cpssDxChTrunkMemberRemove"
    else
        apiName = "cpssDxChTrunkMemberAdd"
    end

    --_debug_to_string("is_local_port",is_local_port)
    --_debug_to_string("trunk_member",trunk_member)

    genericCpssApiWithErrorHandler(command_data,
        apiName, {
            { "IN", "GT_U8", "devNum", devNum},
            { "IN", "GT_TRUNK_ID", "trunkId", trunkId},
            { "IN", "CPSS_TRUNK_MEMBER_STC", "memberPtr", trunk_member }
        })
end

-- on lc : set epcl relate to the egress port
local function on_lc_on_port_epcl_configuration_table_set(command_data, devNum, portNum , unset)
    local enable
    if unset == true then
        enable = false
    else
        enable = true
    end
    -- configuration relevant to EPCL
    -- set egress port 'access to PCL Configuration table' = 'port_mode'
    genericCpssApiWithErrorHandler(
        command_data,
        "cpssDxChPclPortLookupCfgTabAccessModeSet",
        {
            {"IN", "GT_U8", "devNum", devNum},
            {"IN", "GT_PORT_NUM", "portNum", portNum},
            {"IN", "CPSS_PCL_DIRECTION_ENT", "direction", "CPSS_PCL_DIRECTION_EGRESS_E"},
            {"IN", "CPSS_PCL_LOOKUP_NUMBER_ENT", "lookupNum", "CPSS_PCL_LOOKUP_0_E"},
            {"IN", "GT_U32", "subLookupNum", 0},
            {"IN", "CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT", "mode", "CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E"}
        })

    -- allow EPCL lookup for ALL packet types on the port.
    for key, pktType in pairs({ "CPSS_DXCH_PCL_EGRESS_PKT_FROM_CPU_CONTROL_E",
                              "CPSS_DXCH_PCL_EGRESS_PKT_FROM_CPU_DATA_E",
                              "CPSS_DXCH_PCL_EGRESS_PKT_TO_CPU_E",
                              "CPSS_DXCH_PCL_EGRESS_PKT_TO_ANALYZER_E",
                              "CPSS_DXCH_PCL_EGRESS_PKT_TS_E",
                              "CPSS_DXCH_PCL_EGRESS_PKT_NON_TS_E" })
    do
        genericCpssApiWithErrorHandler(
            command_data,
            "cpssDxCh2EgressPclPacketTypesSet",
            {
                {"IN", "GT_U8", "devNum", devNum},
                {"IN", "GT_PHYSICAL_PORT_NUM", "port", portNum},
                {"IN", "CPSS_DXCH_PCL_EGRESS_PKT_TYPE_ENT", "pktType", pktType},
                {"IN",  "GT_BOOL", "enable", enable}
            })
    end

    local epcl_pcl_id    = vss_internal_db_global.on_lc_info.epcl_pcl_id

    -- set pcl configuration entry
    local iface = { type = "CPSS_INTERFACE_PORT_E",
                  devPort = { devNum = vss_internal_db_global.my_hw_devNum ,
                              portNum = portNum }}

    local lookupCfg = {groupKeyTypes={}}
    lookupCfg.enableLookup = enable
    lookupCfg.pclId = epcl_pcl_id
    lookupCfg.groupKeyTypes.nonIpKey = vss_on_lc_pcl_rule_forma_egr_std_not_ip_name
    lookupCfg.groupKeyTypes.ipv4Key  = vss_on_lc_pcl_rule_forma_egr_std_ip_l2_qos_name
    lookupCfg.groupKeyTypes.ipv6Key  = vss_on_lc_pcl_rule_forma_egr_std_ip_l2_qos_name

    genericCpssApiWithErrorHandler(
        command_data,
        "cpssDxChPclCfgTblSet",
        {
            {"IN", "GT_U8", "devNum", devNum},
            {"IN", "CPSS_INTERFACE_INFO_STC", "interfaceInfoPtr", iface},
            {"IN", "CPSS_PCL_DIRECTION_ENT", "direction", "CPSS_PCL_DIRECTION_EGRESS_E"},
            {"IN", "CPSS_PCL_LOOKUP_NUMBER_ENT", "lookupNum", "CPSS_PCL_LOOKUP_0_E"},
            {"IN", "CPSS_DXCH_PCL_LOOKUP_CFG_STC", "lookupCfgPtr", lookupCfg }
        })

end

-- force src-id for the traffic comes from the vss trunk port
local function on_lc_on_vss_trunk_port_force_src_id_set(command_data, devNum, portNum , unset)

    local apiName
    local enable
    local source_id
    
    if unset == true then
        enable = false
        source_id = 0
    else
        enable = true
        source_id = vss_internal_db_global.on_lc_info.uplink.src_id_for_vss_trunk
    end

    --***************
    --***************
    -- force the port to use srcId (and not from the DSA)
    apiName = "cpssDxChBrgSrcIdPortSrcIdForceEnableSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
            { "IN", "GT_U8", "devNum", devNum},
            { "IN", "GT_PORT_NUM", "portNum", portNum},
            { "IN", "GT_BOOL", "enable", enable }
        })

    --***************
    --***************
    -- set the srcId of the port
    apiName = "cpssDxChBrgSrcIdPortDefaultSrcIdSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
            { "IN", "GT_U8", "devNum", devNum},
            { "IN", "GT_PORT_NUM", "portNum", portNum},
            { "IN", "GT_U32", "defaultSrcId", source_id }
        })

end

-- add a port of one of the local-LC that is connected to the neighbor vss (per device).
local function vss_per_device__neighbor_vss_connection_set_port(command_data, devNum, params,unset)

    local the_dev_name_in_local_vss    = params.the_dev_name_in_local_vss
    local the_port_on_dev_in_local_vss = params.the_port_on_dev_in_local_vss
    local the_vss_name                 = params.the_vss_name

    local isCc         = (vss_internal_db_global.my_role == vss_role_cc_full_name)
    local otherVssInfo = vss_internal_db_global.other_vss[the_vss_name]
    local vssTrunkId   = otherVssInfo.vss_trunk_id
    local hwDevNum, portNum
  
    ---------------------------------------------------------------------------------------
    if isCc then
        -- config on CC
        local ccInfo = vss_internal_db_global.neighbors[the_dev_name_in_local_vss].on_cc_info
        -- use single port ... ignore the option that it may be trunk ...
        -- because cpss will not allow a port in 2 trunks ...
        portNum = ccInfo.connection.portNum
        on_cc_cscd_trunk_add_port(command_data,devNum,vssTrunkId,portNum,unset)

   else
        -- config on LC
        local port           = the_port_on_dev_in_local_vss
        local vss_hw_dev_num = otherVssInfo.vss_id
        local is_local_port = (the_dev_name_in_local_vss == vss_internal_db_global.my_name)

        if is_local_port == true then
            hwDevNum = vss_internal_db_global.my_hw_devNum
        else -- port is on other LC
            local neighbor_vss_name = ""
            local table_name = vss_dummy_hw_dev_num_for_other_devices
            local key_name = neighbor_vss_name .. the_dev_name_in_local_vss
            hwDevNum = general_resource_manager_entry_get(table_name,key_name)-- start devices from 10
            -- save to the DB
            otherVssInfo.connections[the_dev_name_in_local_vss].on_lc_dummy_hw_dev_num_for_vss_trunk = hwDevNum
        end

        -----------------------------------------------------------------------------
        -- config on LC, specific for 'add-port' command

        local trunk_member = {port = port, device = hwDevNum}
        -- add port to regular trunk
        on_lc_trunk_add_port(command_data,devNum,is_local_port,trunk_member,unset)

        if is_local_port == true then
            -- set port as cascade port 'extended dsa'
            cscd_port_set(command_data,devNum, port, unset)

            -- set port with EPCL configurations
            on_lc_on_port_epcl_configuration_table_set(command_data, devNum, port , unset)

            -- force src-id for the traffic comes from the vss trunk port
            on_lc_on_vss_trunk_port_force_src_id_set(command_data, devNum, port , unset)

        end
    end
end

local function internal__vss_neighbor_vss_connection_set_port(command_data,params)
    local unset = (params.flagNo == true)

    local the_dev_name_in_local_vss = params.the_dev_name_in_local_vss
    local the_port_on_dev_in_local_vss = params.the_port_on_dev_in_local_vss
    local the_vss_name = params.the_vss_name

    -- do not allow to config without proper initialization
    if vss_internal_db_global.my_role == nil then
       local msg =  "VSS : command can't be run before the command 'vss my-info"
       add_error(command_data,msg)
       return nil, msg
    end

    local isCc = (vss_internal_db_global.my_role == vss_role_cc_full_name)

    -- the command can't be run on CC before 'vss neighbor-info'
    if isCc and vss_internal_db_global.neighbors[the_dev_name_in_local_vss] == nil then
       local msg =
          "VSS : the neighbor device " .. the_dev_name_in_local_vss
          .. " should be configured first (use 'vss neighbor-info')"
       add_error(command_data,msg)
       return nil, msg
    end
    ----------------------------------------------------------------------------
    -- set DB fields relevant to both LC and CC devices
    local otherVssInfo = vss_internal_db_global.other_vss[the_vss_name]

    if otherVssInfo == nil then
        if unset == true then
            add_error(command_data,"VSS : unknown VSS " .. the_vss_name)
            return
        end

       otherVssInfo = {}
    end
    if otherVssInfo.connections == nil then
        if unset == true then
            add_error(command_data,"VSS : no connections to VSS " .. the_vss_name)
            return
        end
       otherVssInfo.connections = {}
    end

    if otherVssInfo.connections[the_dev_name_in_local_vss] == nil then
        if unset == true then
            add_error(command_data,"VSS : local device " ..the_dev_name_in_local_vss .. " not connected to VSS " .. the_vss_name)
            return
        end

        otherVssInfo.connections[the_dev_name_in_local_vss] = {}
    end

    local conn = otherVssInfo.connections[the_dev_name_in_local_vss]
    if conn.ports == nil then
        if unset == true then
            add_error(command_data,"VSS : local device " ..the_dev_name_in_local_vss .. " with no ports connected to VSS " .. the_vss_name)
            return
        end
       conn.ports = {}
    end

    if conn.ports[the_port_on_dev_in_local_vss] == nil and unset == true then
        add_error(command_data,"VSS : port " .. the_port_on_dev_in_local_vss .. " on local device " ..the_dev_name_in_local_vss .. " not connected to VSS " .. the_vss_name)
        return
    end

    conn.ports[the_port_on_dev_in_local_vss] = the_port_on_dev_in_local_vss

    -- set DB fields relevant to LC devices only
    if (not isCc) and conn.on_lc_dummy_hw_dev_num_for_vss_trunk == nil then
       local tbl = vss_dummy_hw_dev_num_for_other_devices
       local key = the_vss_name .. the_dev_name_in_local_vss

       conn.on_lc_dummy_hw_dev_num_for_vss_trunk = general_resource_manager_entry_get(tbl, key)
    end

    vss_internal_db_global.other_vss[the_vss_name] = otherVssInfo

    ----------------------------------------------------------------------------
    -- setup configuration
    local vssTrunkId = otherVssInfo.vss_trunk_id

    for iterator, devNum in command_data:getDevicesIterator() do
       vss_per_device__neighbor_vss_connection_set_port(command_data, devNum, params,unset)
    end

    if unset == true then
        conn.ports[the_port_on_dev_in_local_vss] = nil

        local empty = true
        -- check if empty list
        for index,entry in pairs(conn.ports) do
            empty = false
            break
        end

        if empty == true then
            -- no more ports in the connection

            if (not isCc) then
                local tbl = vss_dummy_hw_dev_num_for_other_devices
                local key = the_vss_name .. the_dev_name_in_local_vss
                -- free on_lc_dummy_hw_dev_num_for_vss_trunk
                -- free the resource as nobody use it any more
                general_resource_manager_release_entry(tbl, key)
            end

            otherVssInfo.connections[the_dev_name_in_local_vss] = nil
        end

    end


end

-- add a port of one of the local-lc that is connected to the neighbor vss.
local function vss_neighbor_vss_connection_set_port(params)
    local command_data = Command_Data()

    command_data:clearResultArray()
    command_data:initAllAvailableDevicesRange()

    -- Common variables initialization
    command_data:clearResultArray()
    command_data:initAllAvailableDevicesRange()

    internal__vss_neighbor_vss_connection_set_port(command_data,params)

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end

--[[
Command to add a port of one of the local-lc that is connection to the neighbor vss.
Needed on CC and LC devices.
NOTE:
1.	ALL LCs must get calls about ALL connection ports regardless if ‘currently’ hold ports in the connection (ports on LCs of local vss)
(also correct for CC device … but it seems obvious)
2.	On CC this command can’t be called before stating ‘dev-name-in-local-vss’ (see eport context command neighbor-info)
--]]
local vss_neighbor_vss_connection___params = {
     {  type="named",
            { format = "vss-name %s", name = "the_vss_name",
                help = description___neighbor_vss_name},

            {format = "dev-name-in-local-vss %s", name = "the_dev_name_in_local_vss",
                help = "the name of the LC device owning by a port connected to a neighbor VSS"},

            {format = "port %type___vss_lc_port_range",
                name = "the_port_on_dev_in_local_vss",
                help = description___vss_on_lc_port_range},

        mandatory = {"the_vss_name", "the_dev_name_in_local_vss", "the_port_on_dev_in_local_vss"}
     }
  }

CLI_addCommand("config", "vss neighbor-vss-connection add-port", {
  func   = vss_neighbor_vss_connection_set_port,
  help   = "add port of one of the local-LC that is connection to the neighbor VSS",
  params = vss_neighbor_vss_connection___params
})

CLI_addCommand("config", "vss neighbor-vss-connection remove-port", {
  func   =  function(params)
                params.flagNo = true
                return vss_neighbor_vss_connection_set_port(params)
            end,
  help   = "remove port of one of the local-LC that is not connection any more to the neighbor VSS",
  params = vss_neighbor_vss_connection___params
})



