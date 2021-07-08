--********************************************************************************
--*              (c), Copyright 2013, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* dsa_tag_calculator.lua
--*
--* DESCRIPTION:
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes
cmdLuaCLI_registerCfunction("wrlDxChDsaToStruct") 
--constants

local dsa_format_string = "must be 1 or 2 or 4 word (8/16/32 HEX digits) , for example 012345678 or 0x0123456789abcdef"

-- ************************************************************************
---
--  check_param_dsa_tag_hex
--        @description  Check parameter that it is in the correct form, 
--                      DSA tag hex value
--
--        @param param          - Parameter string
--        @param name           - parameter name
--        @param desc           - type description reference
--        @param varray         - previous values array
--
--        @return       boolean check status
--        @return       true:string or false:error_string if failed
--        @return       true if no more subunits
--
local function check_param_dsa_tag_hex(param, name, desc)
    if prefix_match("0x", param) then
        param = string.sub(param,3)
    end

    local currentHex
    local ret = {}
    local d
    
    while string.len(param) > 7  do
        d = tonumber(string.sub(param,1,8),16)
        if d == nil then
            return false, "bad hex digit(s): "..string.sub(param,1,4)
        end
        if #ret == 4 then
            return false, "tag too long, " .. dsa_format_string
        end
        
        -- convert to HEX
        currentHex =  string.format("%8.8x",d)
        
        table.insert(ret,currentHex)
        param = string.sub(param,9)
    end
    if param ~= "" or #ret == 0 or #ret == 3 then
        return false, "bad dsa tag, " .. dsa_format_string
    end
    return true, ret
end

CLI_type_dict["dsa_tag_hex_type"] = {
    checker = check_param_dsa_tag_hex,
    help = "DSA TAG hex " .. dsa_format_string
}
CLI_type_dict["dsa_word2"] = {
    checker = CLI_check_param_hexstring,
    min=8,
    max=8,
    help = "DSA TAG second word"
} 
CLI_type_dict["dsa_word3"] = {
    checker = CLI_check_param_hexstring,
    min=8,
    max=8,
    help = "DSA TAG third word"
} 
CLI_type_dict["dsa_word4"] = {
    checker = CLI_check_param_hexstring,
    min=8,
    max=8,
    help = "DSA TAG forth word"
} 

-- ************************************************************************
---
--  dsa_tag_decode
--        @description  Decode dsa tag and display it
--
--        @param params            - data description table
--        @param words          - bitsteam words (start with index 1)
--
--        @return       table
--
local function dsa_tag_decode(params)
    local numOfWords

    -- read TagCommand bits
    local devNum = devEnv.dev
    local dsaString = params.dsa_tag_hex[1]
    
    numOfWords = 1
    
    if params.dsa_tag_hex[2] then 
        dsaString = dsaString .. tostring(params.dsa_tag_hex[2])
        numOfWords = numOfWords + 1
        
        if params.dsa_tag_hex[3] then 
            dsaString = dsaString .. tostring(params.dsa_tag_hex[3])
            numOfWords = numOfWords + 1
        end
        
        if params.dsa_tag_hex[4] then 
            dsaString = dsaString .. tostring(params.dsa_tag_hex[4])
            numOfWords = numOfWords + 1
        end

    else
        if params.word2 then 
            dsaString = dsaString .. tostring(params.word2)
            numOfWords = numOfWords + 1
        end
        if params.word3 then 
            dsaString = dsaString .. tostring(params.word3)
            numOfWords = numOfWords + 1
        end
        if params.word4 then 
            dsaString = dsaString .. tostring(params.word4)
            numOfWords = numOfWords + 1
        end
    end
    
    print("input DSA string : " , to_string(dsaString))
    
    local rc, dsaStruct = wrLogWrapper("wrlDxChDsaToStruct", "(devNum , dsaString)", devNum , dsaString)

    if rc ~= 0 then
        local errorCode = returnCodes[rc]
        
        if (errorCode) then
            print ("Error parsing the DSA : " .. errorCode)
        else
            print ("Error parsing the DSA")
        end
        
        return
    end

    -- do the printing regardless to the below check of possible error
    -- because we may want to allow those errors ...
    print("DSA structure : " , to_string(dsaStruct))
    print("Successful DSA parsing")

    if(dsaStruct.commonParams.dsaTagType ~= "CPSS_DXCH_NET_DSA_" ..numOfWords.. "_WORD_TYPE_ENT") then
        print("")-- force new line
        print("WARNING : input was " .. numOfWords .. " DSA tag words , but recognized as " .. dsaStruct.commonParams.dsaTagType)
    end
    
end

CLI_addHelp("debug", "dsa-tag", "DSA tag calculator")
CLI_addCommand("debug", "dsa-tag decode", {
    help = "decode dsa tag",
    func = dsa_tag_decode,
    first_optional = "word2", -- word2 and all other after it are 'optional'
    params = {
    { type="values",
          {format="%dsa_tag_hex_type", name="dsa_tag_hex", help=CLI_type_dict["dsa_tag_hex_type"].help}, 
          {optional = true,format="%dsa_word2", name="word2", help="second word"}, 
          {optional = true,format="%dsa_word3", name="word3", help="third word"}, 
          {optional = true,format="%dsa_word4", name="word4", help="forth word"}
    }}
})
