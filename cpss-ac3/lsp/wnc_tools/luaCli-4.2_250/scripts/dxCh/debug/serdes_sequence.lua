--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* serdes_sequence.lua
--*
--* DESCRIPTION:
--*       get and set serdes sequence configuration
--*
--*
--********************************************************************************


-- ************************************************************************
---
--  DEC_HEX_String
--        @description  get serdes sequence configuration
--
--        @param params         - inserted number                   
--
--        @return        result number(string)
--
local function DEC_HEX_String(IN)
    local IN_length   = string.len(IN)
    for i = IN_length , 7 do
        i   =   i+1
        IN  = "0"   ..  IN
    end
    IN  = "0x"  .. IN
    return IN
end


-- ************************************************************************
---
--  DEC_HEX
--        @description  get serdes sequence configuration
--
--        @param params         - inserted number                   
--
--        @return        result number
--
local function DEC_HEX(IN)
    local B,K,OUT,I,D=16,"0123456789abcdef","",0
    while IN>0 do
        I=I+1
        IN,D=math.floor(IN/B),math.mod(IN,B)+1
        OUT=string.sub(K,D,D)..OUT
    end
    return DEC_HEX_String(OUT)
end


-- ************************************************************************
---
--  get_serdes_sequence_configuration
--        @description  get serdes sequence configuration
--
--        @param params         - params["all_device"]: all or given 
--                                devices iterating property, could be 
--                                irrelevant
--        @param command_data   - command execution data object                     
--
--        @return        true on success, otherwise false and error message
--
local function get_serdes_sequence_configuration(params, command_data)
    local GT_OK       = 0
    local GT_NO_MORE  = 12
    local rc          = GT_OK
    local inner_rc    = GT_OK
    local baseAddr
    
    local devNum      = params["devID"]
    local portGroup   = params["portGroup"]
    local seqType     = params["seqType"]
    
    local lineNum    = 0
    local operationType
    local unitIndex     = 0
    local dataToWrite   = 0
    local regAddress    = 0
    local mask          = 0
    local delay         = 0
    local numOfLoops    = 0
    local indexOffset
    local regOffset
    local data
    local waitTime      = 0
    local print_header  = true
    
    local values
    
    local header_string =   "\nline operationType baseAddress regAddress dataToWrite    mask    delay numOfLoops\n" ..
                              "---- ------------- ----------- ---------- ----------- ---------- ----- ----------"
    while rc ~= GT_NO_MORE do
        baseAddr        = ''
        regOffset       = ''
        data            = ''
        mask            = ''
        waitTime        = 0
        numOfLoops      = 0
    
        rc, values = myGenWrapper(
    "cpssDxChPortSerdesSequenceGet", {
      { "IN", "GT_U8"  , "dev", devNum},                                          -- devNum
      { "IN", "GT_U32" , "portGroupId", portGroup},                               -- portGroup
      { "IN", "CPSS_DXCH_PORT_COM_PHY_H_SUB_SEQ_ENT" , "GT_U32", seqType},        -- seqType
      { "IN", "GT_U32" , "GT_U32", lineNum},                                      -- lineNum
      { "OUT","CPSS_DXCH_PORT_SERDES_OPERATION_CFG_STC" , "cfgEl"}                -- cfgEl
        });
        if(rc == GT_NO_MORE) then
            return;
        end
        
        if(rc ~= GT_OK and rc ~= GT_NO_MORE) then
            local index = string.find(returnCodes[rc],"=")
            print("Error at Serdes Sequence Get:", string.sub(returnCodes[rc],index + 1))
            break
        end
        
        if(print_header == true) then
            print(header_string)
            print_header = false
        end
        
        local OutValues       = values["cfgEl"]
        operationType   = OutValues.op
        
        if(operationType == "CPSS_DXCH_PORT_EL_DB_WRITE_OP_E") then
            indexOffset    = OutValues.indexOffset
            regOffset      = DEC_HEX(OutValues.regOffset)
            data           = DEC_HEX(OutValues.data)
            mask           = DEC_HEX(OutValues.mask)
            
            inner_rc, values = myGenWrapper(
                "cpssDxChPortUnitInfoGet", {
                    { "IN", "GT_U8"  , "dev", devNum},                                      -- devNum
                    { "IN", "CPSS_DXCH_PORT_UNITS_ID_ENT" , "indexOffset", indexOffset},    -- indexOffset
                    { "OUT","GT_U32" , "baseAddr"},                                         -- baseAddr
                    { "OUT","GT_U32" , "unitIndex"}                                         -- unitIndex
            });
            baseAddr     = DEC_HEX(values.baseAddr)
            unitIndex    = DEC_HEX(values.unitIndex)
            if(inner_rc ~= GT_OK) then
                print("Error at Unit Info Get.")
            end 
            
        elseif  (operationType == "CPSS_DXCH_PORT_EL_DB_DELAY_OP_E")    then
            delay          = OutValues.delay
        elseif  (operationType == "CPSS_DXCH_PORT_EL_DB_POLLING_OP_E")  then
            indexOffset   = OutValues.indexOffset
            regOffset     = DEC_HEX(OutValues.regOffset)
            data          = DEC_HEX(OutValues.data)
            mask          = DEC_HEX(OutValues.mask)
            delay         = OutValues.waitTime
            numOfLoops    = OutValues.numOfLoops

            inner_rc, values = myGenWrapper(
                "cpssDxChPortUnitInfoGet", {
                    { "IN", "GT_U8"  , "dev", devNum},                                      -- devNum
                    { "IN", "CPSS_DXCH_PORT_UNITS_ID_ENT" , "indexOffset", indexOffset},    -- indexOffset
                    { "OUT","GT_U32" , "baseAddr"},	                                    -- baseAddr
                    { "OUT","GT_U32" , "unitIndex"}                                         -- unitIndex
            });
            baseAddr     = DEC_HEX(values.baseAddr)
            unitIndex    = DEC_HEX(values.unitIndex)
            if(inner_rc ~= GT_OK) then
                print("Error at Unit Info Get.")
            end 
        end
        
        operationType = string.sub(operationType,22,-6)
        local currentLine =
                    string.format("%-4s", alignLeftToCenterStr(to_string(lineNum), 4))          .. " " ..
                    string.format("%-13s", alignLeftToCenterStr(operationType, 13))             .. " " ..
                    string.format("%-11s", alignLeftToCenterStr(baseAddr, 11))	                .. " " ..
                    string.format("%-10s", alignLeftToCenterStr(regOffset, 10)) 	        .. " " ..
                    string.format("%-11s", alignLeftToCenterStr(data, 11))                      .. " " ..
                    string.format("%-10s", alignLeftToCenterStr(mask, 10))	                .. " " ..
                    string.format("%-5s", alignLeftToCenterStr(to_string(waitTime), 5))	        .. " " ..
                    string.format("%-10s", alignLeftToCenterStr(to_string(numOfLoops), 10))    
        print(currentLine)
        lineNum = lineNum + 1
    end
end

  
-- ************************************************************************
---
--  set_serdes_sequence_configuration
--        @description  set serdes sequence configuration
--
--        @param params         - params["all_device"]: all or given 
--                                devices iterating property, could be 
--                                irrelevant
--        @param command_data   - command execution data object                     
--
--        @return        true on success, otherwise false and error message
--
local function set_serdes_sequence_configuration(params, command_data)
    -- Common variables declaration
    local GT_OK       = 0
    local GT_NO_MORE  = 12
    local rc          = GT_OK
    local inner_rc    = GT_OK
    local  devNum       = params["devID"]
    local  portGroup    = params["portGroup"]
    local  seqType      = params["seqType"]
    local  lineNum      = params["lineNum"]
    local  operationType= params["operationType"]
    local  baseAddress  = params["baseAddress"]
    local  regAddress   = params["regAddress"]
    local  dataToWrite  = params["dataToWrite"]
    local  mask         = params["mask"]
    local  delay        = params["delay"]
    local  numOfLoops   = params["numOfLoops"]
    local unitId
    
    local values
    
    local unitIndex = baseAddress
    local cfgEl = {}
    cfgEl.op            = operationType;
    if(operationType == "CPSS_DXCH_PORT_EL_DB_WRITE_OP_E")    then
        inner_rc, values = myGenWrapper(
            "cpssDxChPortUnitInfoGetByAddr", {
                { "IN", "GT_U8"  , "dev", devNum},                                      -- devNum
                { "IN", "GT_U32" , "baseAddress", baseAddress},	                        -- baseAddr
                { "OUT","CPSS_DXCH_PORT_UNITS_ID_ENT" , "unitIdPtr"}                    -- unitIndex
        });
        if(inner_rc ~= GT_OK and inner_rc ~= GT_NO_MORE) then
            local index = string.find(returnCodes[inner_rc],"=")
            print("Error at Serdes Sequence Set:", string.sub(returnCodes[inner_rc],index + 1))
            return;
        end
        unitId   = values.unitIdPtr
        cfgEl.regOffset     = regAddress
        cfgEl.data          = dataToWrite
        cfgEl.mask          = mask
    elseif  (operationType == "CPSS_DXCH_PORT_EL_DB_DELAY_OP_E")    then
        cfgEl.delay         = delay
    elseif  (operationType == "CPSS_DXCH_PORT_EL_DB_POLLING_OP_E")  then
        inner_rc, values = myGenWrapper(
            "cpssDxChPortUnitInfoGetByAddr", {
                { "IN", "GT_U8"  , "dev", devNum},                                      -- devNum
                { "IN", "GT_U32" , "baseAddress", baseAddress},	                        -- baseAddr
                { "OUT","CPSS_DXCH_PORT_UNITS_ID_ENT" , "unitIdPtr"}                    -- unitIndex
        });
        if(inner_rc ~= GT_OK and inner_rc ~= GT_NO_MORE) then
            local index = string.find(returnCodes[inner_rc],"=")
            print("Error at Serdes Sequence Set:", string.sub(returnCodes[inner_rc],index + 1))
            return;
        end
        unitId   = values.unitIdPtr
        
        cfgEl.regOffset     = regAddress
        cfgEl.data          = dataToWrite
        cfgEl.mask          = mask
        cfgEl.waitTime      = delay
        cfgEl.numOfLoops    = numOfLoops
    end
    
    local firstLine
    if(lineNum == 0) then
        firstLine = "GT_TRUE"
    else
        firstLine = "GT_FALSE"
    end
    local numOfOp = lineNum
    
    rc, values = myGenWrapper(
        "cpssDxChPortSerdesSequenceSet", {
            { "IN", "GT_U8"  , "dev", devNum},                                          -- devNum
            { "IN", "GT_U32" , "portGroup", portGroup},                                 -- portGroup
            { "IN", "GT_BOOL" , "firstLine", firstLine},                                -- firstLine
            { "IN", "CPSS_DXCH_PORT_COM_PHY_H_SUB_SEQ_ENT" , "seqType", seqType},       -- seqType
            { "IN", "CPSS_DXCH_PORT_UNITS_ID_ENT" , "unitId", unitId},                  -- unitId
            { "IN", "CPSS_DXCH_PORT_SERDES_OPERATION_CFG_STC" , "seqLinePtr", cfgEl},   -- seqLinePtr
            { "IN", "GT_U16" , "numOfOp", numOfOp}                                      -- numOfOp
    });
    if(rc ~= GT_OK and rc ~= GT_NO_MORE) then
        local index = string.find(returnCodes[rc],"=")
        print("Error at Serdes Sequence Set:", string.sub(returnCodes[rc],index + 1))
        return;
    end
end   
  

--------------------------------------------------------------------------------
-- command registration: set serdes sequence configuration
--------------------------------------------------------------------------------
CLI_addCommand("debug", "serdes configuration set", {
    func = set_serdes_sequence_configuration,
    help = "Set serdes configuration",
    params = {
    	{  type= "named",   { format="device %devID_all"                ,name="devID",          help="The device number"                            },
                            { format="port-group %portGroup"            ,name="portGroup",      help="The port-group (optional, default is all)"    },  
                            { format="seqType %serdes_sequence_type"    ,name="seqType",        help="The sequence Type"                            },
                            { format="lineNum %GT_U16"                  ,name="lineNum",        help="Line Number"                                  },
                            { format="operationType %serdes_operation"  ,name="operationType",  help="Operation Type"                               },  
                            { format="baseAddress %GT_U32_hex"          ,name="baseAddress",    help="Base Address"                                 },
                            { format="regAddress %GT_U32_hex"           ,name="regAddress",     help="Reg Address"                                  },
                            { format="dataToWrite %GT_U32_hex"          ,name="dataToWrite",    help="Date to Write"                                },  
                            { format="mask %GT_U32_hex"                 ,name="mask",           help="Mask"                                         },
                            { format="delay %GT_U16"                    ,name="delay",          help="Delay"                                        },
                            { format="numOfLoops %GT_U16"               ,name="numOfLoops",     help="Number of Loops"                              },  
                            requirements = {
                                ["portGroup"]       = {"devID"},
                                ["seqType"]         = {"portGroup"},
                                ["lineNum"]         = {"seqType"},
                                ["operationType"]   = {"lineNum"},
                                ["baseAddress"]     = {"operationType"},
                                ["regAddress"]      = {"baseAddress"},
                                ["dataToWrite"]     = {"regAddress"},
                                ["mask"]            = {"dataToWrite"},
                                ["delay"]           = {"mask"},
                                ["numOfLoops"]      = {"delay"}
                                },
              mandatory={"devID", "portGroup", "seqType", "lineNum", "operationType", "baseAddress", "regAddress", "dataToWrite", "mask", "delay", "numOfLoops"}
    }
  }  
})


--------------------------------------------------------------------------------
-- command registration: get the serdes sequence configuration
--------------------------------------------------------------------------------
CLI_addCommand("debug", "serdes configuration get", {
    func = get_serdes_sequence_configuration,
    help = "Get serdes configuration",
    params = {
    	{  type= "named",   { format="device %devID_all"            ,name="devID"       , help="The device number"                          },
                            { format="port-group %portGroup"        ,name="portGroup"   , help="The port-group (optional, default is all)"  },  
                            { format="seqType %serdes_sequence_type",name="seqType"    , help="Serdes type"                                 },
                            requirements = {
                                ["portGroup"]       = {"devID"},
                                ["seqType"]         = {"portGroup"},
                                },
              mandatory={"devID", "portGroup", "seqType"}
    }
  }
})
    
   
--------------------------------------------------------------------------------
-- CLI type dict: serdes sequence type
--------------------------------------------------------------------------------
CLI_type_dict["serdes_sequence_type"] = {
    checker = CLI_check_param_number,
    min=0,
    max=38,
    help=   "  0                    SERDES_SD_RESET_SEQ\n"                 ..
            "  1                    SERDES_SD_UNRESET_SEQ\n"               ..
            "  2                    SERDES_RF_RESET_SEQ\n"                 ..
            "  3                    SERDES_RF_UNRESET_SEQ\n"               ..
            "  4                    SERDES_SYNCE_RESET_SEQ\n"              ..
            "  5                    SERDES_SYNCE_UNRESET_SEQ\n"            ..
            "  6                    SERDES_SERDES_POWER_UP_CTRL_SEQ\n"     ..
            "  7                    SERDES_SERDES_POWER_DOWN_CTRL_SEQ\n"   ..
            "  8                    SERDES_SERDES_RXINT_UP_SEQ\n"          ..
            "  9                    SERDES_SERDES_RXINT_DOWN_SEQ\n"        ..
            "  10                   SERDES_SERDES_WAIT_PLL_SEQ\n"         ..
            "  11                   SERDES_SPEED_1_25G_SEQ\n"             ..
            "  12                   SERDES_SPEED_3_125G_SEQ\n"            ..
            "  13                   SERDES_SPEED_3_75G_SEQ\n"             ..
            "  14                   SERDES_SPEED_4_25G_SEQ\n"             ..
            "  15                   SERDES_SPEED_5G_SEQ\n"                ..
            "  16                   SERDES_SPEED_6_25G_SEQ\n"             ..
            "  17                   SERDES_SPEED_7_5G_SEQ\n"              ..
            "  18                   SERDES_SPEED_10_3125G_SEQ\n"          ..
            "  19                   SERDES_SD_LPBK_NORMAL_SEQ\n"          ..
            "  20                   SERDES_SD_ANA_TX_2_RX_SEQ\n"          ..
            "  21                   SERDES_SD_DIG_TX_2_RX_SEQ\n"          ..
            "  22                   SERDES_SD_DIG_RX_2_TX_SEQ\n"          ..
            "  23                   SERDES_PT_AFTER_PATTERN_NORMAL_SEQ\n" ..
            "  24                   SERDES_PT_AFTER_PATTERN_TEST_SEQ\n"   ..
            "  25                   SERDES_RX_TRAINING_ENABLE_SEQ\n"      ..
            "  26                   SERDES_RX_TRAINING_DISABLE_SEQ\n"     ..
            "  27                   SERDES_TX_TRAINING_ENABLE_SEQ\n"      ..
            "  28                   SERDES_TX_TRAINING_DISABLE_SEQ\n"     ..
            "  29                   SERDES_SPEED_12_5G_SEQ\n"             ..
            "  30                   SERDES_SPEED_3_3G_SEQ\n"              ..
            "  31                   SERDES_SPEED_11_5625G_SEQ\n"          ..
            "  32                   SERDES_SERDES_PARTIAL_POWER_DOWN_SEQ\n"..
            "  33                   SERDES_SERDES_PARTIAL_POWER_UP_SEQ\n" ..
            "  34                   SERDES_SPEED_11_25G_SEQ\n"            ..
            "  35                   SERDES_CORE_RESET_SEQ\n"              ..
            "  36                   SERDES_CORE_UNRESET_SEQ\n"            ..
            "  37                   SERDES_FFE_TABLE_LR_SEQ\n"            ..
            "  38                   SERDES_FFE_TABLE_SR_SEQ"
}