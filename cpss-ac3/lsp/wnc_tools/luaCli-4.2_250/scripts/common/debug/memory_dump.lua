--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* memory_dump.lua
--*
--* DESCRIPTION:
--*       dumping tables memory
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants

-- read nemory
-- return on fail: return code, on success: GT_OK, read data
-- prvCpssDrvHwPpReadRam function generated Lua wrapper does not give to use it
-- it does not understand the size of array being returned
-- cpssDrvPpHwRegisterRead used instead
-- returns table of word values.
local function generic_pp_memory_read(dev_num, port_group, address, num_of_words)
    local data = {};
    local i;
    for i = 0,(num_of_words - 1) do
        local ret,val = myGenWrapper("cpssDrvPpHwRegisterRead",{
            {"IN","GT_U8",   "devNum",       dev_num},
            {"IN","GT_U32",  "portGroupId",  port_group},
            {"IN","GT_U32",  "regAddr", (address + (4 * i))},
            {"OUT","GT_U32", "data"}});
         if ret~=0 then
             print("cpssDrvPpHwRegisterRead() failed: ".. tostring(ret))
             return ret;
         end
         data[i + 1] = val.data;
    end
    return 0, data;
end

local function field_from_data(data, low_bit, high_bit)
    local res, word_num, num_of_bits, start_bit;
    local max_len, offset, w;
    if type(low_bit) ~= "number" then
        print("field_from_data - low_bit error");
        return 0;
    end
    if type(high_bit) ~= "number" then
        print("field_from_data - high_bit error");
        return 0;
    end
    if (low_bit > high_bit) then
        print("field_from_data - low_bit > high_bit");
        return 0;
    end
    res = 0;
    offset = 0;
    while low_bit <= high_bit do
        word_num = math.floor(low_bit / 32); -- 0-based word number
        start_bit = low_bit - (word_num * 32); -- 0-based bit number
        max_len = 32 - start_bit;
        num_of_bits = high_bit - low_bit + 1;
        if num_of_bits > max_len then
            num_of_bits = max_len;
        end
        w = data[word_num + 1];
        w = bit_shr(w, start_bit);
        w = bit_and(w, bit_shr(0xFFFFFFFF, (32 - num_of_bits)));
        res = res + bit_shl(w, offset);
        offset  = offset  + num_of_bits;
        low_bit = low_bit + num_of_bits;
    end
    return res;
end

-- returns table of vield values with keys derived from field definition
-- fields - table of field definitions
--    each field is field_name = {low_bit,high_bit} or field_name = {{low_bit1,high_bit1}....}
-- data - memory representes as table of word values.
local function generic_row_data_to_fields(fields, data)
    local res = {};
    local name, bits;
    for name, bits in pairs(fields) do
        if type(bits) ~= "table" then
            print("generic_data_to_fields - error1");
            return nil;
        end
        if type(bits[1]) == "number" then
            res[name] = field_from_data(
                data, bits[1] --[[low_bit--]], bits[2] --[[high_bit--]]);
        elseif type(bits[1]) == "table" then
            local i, n, n1, offset;
            offset = 0;
            n = 0;
            for i, bits1 in pairs(bits) do
                n1 = field_from_data(
                    data, bits1[1] --[[low_bit--]], bits1[2] --[[high_bit--]]);
                n = n + bit_shl(n1, offset);
                offset = offset + (bits1[2] - bits1[1] + 1);
            end
            res[name] = n;
        else
            print("generic_data_to_fields - error2");
        end
    end
    return res;
end

-- data for debugging
--[[
lua
_data = {0xF0F0F0F0, 0x50000005, 0x33333333};
_fields = {b0_7 = {0,7}, b28_31 = {28,31}, b28_40={28,40}, b4_7__32_39={{4,7},{32,39}}};
_res = generic_row_data_to_fields(_fields, _data);
print(to_string(_res));
_data = nil;
_fields = nil;
_res = nil;
.
--]]

local function generic_pp_memory_row_data_to_fields(
    dev_num, port_group, address, num_of_words, fields)
    local ret, data = generic_pp_memory_read(
        dev_num, port_group, address, num_of_words);
    if ret ~= 0 then
        return nil;
    end
    return generic_row_data_to_fields(fields, data);
end

-- returns "commented" field values:
-- replaced field_name = value by field_name = {value, value_name}
local function generic_fields_to_enum(fields_value_names, fields_values)
    local field_name, field_value, value_name, res;
    res = {};
    for field_name, field_value in pairs(fields_values) do
        if fields_value_names[field_name] then
            value_name = fields_value_names[field_name][field_value];
            if not value_name then
                value_name = "***unknown***";
            end
            res[field_name] = {field_value,value_name};
        else
            res[field_name] = field_value;
        end
    end
    return res;
end

function generic_pp_memory_row_data_to_fields_values_named(
    dev_num, port_group, address, num_of_words, fields, value_names)
    return generic_fields_to_enum(
        value_names,
        generic_pp_memory_row_data_to_fields(
            dev_num, port_group, address, num_of_words, fields));
end


--function generic_pp_memory_read(dev_num, port_group, address, num_of_words)
local function dump_pp_memory(params)
    local num = params.num;
    local step = num;
    local port_group = params.portGroup;
    if not port_group then
        port_group = 0;
    end
    local addr = params.address;
    local data, devices, j,i, rc;
    if num > 8 then
        step = 8;
    end
    local bound = num + step;

    if (params["devID"]=="all") then
        devices=wrLogWrapper("wrlDevList")
    else
        devices={params["devID"]}
    end

    for j = 1, #devices do
        local dev_num = devices[j];
        print("dump on device: " .. tostring(dev_num));
        while bound > step do
            rc, data = generic_pp_memory_read(dev_num, port_group, addr, step);
            if rc ~= 0 then
                print("read error");
                return false;
            end
            local s = string.format("0x%08X: ", addr);
            for i = 1,step do
                s = s .. string.format("0x%08X ", data[i]);
            end
            print(s);
            bound = bound - step;
            addr = addr + (step * 4);
        end
    end
    return true;
end

CLI_addCommand("debug", "dump memory", {
    func = dump_pp_memory,
    help = "Dump memory",
    params={
         { type="named",
         { format="device %devID_all",name="devID", help="The device number" },
         { format="port-group %portGroup",name="portGroup", help="The port-group (optional, default 0)" },
         { format="address %register_address", help="The address of the data to read (hex)" },
         { format="num %GT_U32",  help="Number of words"},
           requirements = {
            ["portGroup"] = {"devID"},
            ["address"] = {"devID"},
            ["num"] =  {"address"},
         },
         mandatory = {"num"}
        }
    }
})
