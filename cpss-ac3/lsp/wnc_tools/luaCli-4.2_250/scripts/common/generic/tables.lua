--********************************************************************************
--*              (c), Copyright 2014, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* tables.lua
--*
--* DESCRIPTION:
--*       Table manipulation functions
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************



-- ************************************************************************
---
--  Nice_Table class(desc)
--        @description  Table printing class
--
--        @param  desc  - table columns description
--
--        @return       operation succed
--

--[[
Here:
    head  - header string
    len   - column length
    align - alignment ("l", "r", "c"), default="l"
    path  - item path
    type  - "number","hex","bool","boolYN","string",function; default=tostring(value)
    rmprefix - prefix to remove
    rmsuffix - suffix to remove
    tolower  - lowercase if true
    toupper  - lowercase if true
{
  { head="Index",len=5,align="c",path="index",type="number" },
  { head="Address",len=22,align="c",path="addr_s"},
  { head="Vlan",len=5,align="c",path="mac_entry.key.vlanId" },
  { head="Skip",len=4,align="c",path="skip_valid_aged.skip",type="boolYN" },
  { head="Interface",len=14,path="mac_entry.dstInterface",type=interfaceInfoStrGet },
  { head="Static",len=7,path="mac_entry.isStatic",type="bool" },
  { head="DA Route",len=8,path="mac_entry.daRoute",type="boolYN" },
  { head="saCommand",len=9,path="mac_entry.saCommand",type=mac_table_cmd_view },
  { head="daCommand",len=9,path="mac_entry.daCommand",type=mac_table_cmd_view }
}
]]--
do

Nice_Table = class(
    function(object, desc, add)
        object:initTable(desc, add)
    end
)

local function column_align_and_fill(str,len,alignment,last)
    local s
    if alignment == "r" then
        s = string.format("%"..tostring(len).."s",str)
    elseif alignment == "c" then
        s = alignLeftToCenterStr(str,len)
        if last then return s end
        return string.format("%-"..tostring(len+2).."s",s)
    else -- alignment == "l" or alignment == nil then
        if last then return str end
        return string.format("%-"..tostring(len+2).."s",str)
    end
    if last ~= nil then
        s = s .. "  "
    end
    return s
end

local function string_splitl(str)
    local t = {}
    while str ~= "" do
        local p = str.find(str,"\n",1,true)
        if p == nil then break end
        table.insert(t,string.sub(str,1,p-1))
        str = string.sub(str,p+1)
    end
    if str ~= "" then
        table.insert(t,str)
    end
    return t
end

function Nice_Table:initTable(pdesc, add)
    self.table_desc = pdesc
    self.rows = {}
    local i
    local header = "\n"
    local hl

    if type(add) == "table" then
        self.table_desc = {}
        for i=1,#pdesc do
            table.insert(self.table_desc,pdesc[i])
        end
        for i=1,#add do
            table.insert(self.table_desc,add[i])
        end
    end

    local desc = self.table_desc
    for hl = 1, 10 do
        local f = false
        local s = ""
        for i = 1, #desc do
            local ch = string_splitl(desc[i].head)[hl]
            if ch == nil then
                ch = ""
            else
                f = true
            end
            s = s .. column_align_and_fill(
                ch,desc[i].len,
                desc[i].align, i==#desc)
        end
        if f == false then break end
        header = header .. s .."\n"
    end
    for i = 1, #desc do
        header = header .. column_align_and_fill(
            string.rep("-",desc[i].len),desc[i].len,
            desc[i].align, i==#desc)
    end
    self.header_string = header
    self.footer_string = ""
end

function Nice_Table:convertCell(col,tbl)
    local val
    val = get_item_by_path(tbl, col.path)
    if val == nil then val = "n/a" end
    if type(col.type) == "function" then
        val = col.type(val)
    elseif col.type == "hex" then
        val = string.format("0x%x",val)
    elseif col.type == "bool" then
        if val then
            val = "True"
        else
            val = "False"
        end
    elseif col.type == "boolYN" then
        if val then
            val = "Yes"
        else
            val = "No"
        end
    else -- number, string, etc
        val = tostring(val)
    end
    if col.rmprefix ~= nil then
        val = prefix_del(col.rmprefix, val)
    end
    if col.rmsuffix ~= nil then
        if string.sub(val,-string.len(col.rmsuffix)) == col.rmsuffix then
            val = string.sub(val,1,-1-string.len(col.rmsuffix))
        end
    end
    if col.tolower then
        val = string.lower(val)
    end
    if col.toupper then
        val = string.upper(val)
    end
    return val
end
function Nice_Table:formatRow(tbl)
    local s = ""
    local i, col, val
    for i = 1, #self.table_desc do
        col = self.table_desc[i]
        val = self:convertCell(col, tbl)
        s = s .. column_align_and_fill(
            val, col.len, col.align,
            i==#self.table_desc)
    end
    return s
end

function Nice_Table:addRow(tbl)
    table.insert(self.rows, self:formatRow(tbl))
end

function Nice_Table:formatTableRows(tbl)
    local s = ""
    local i, col, val
    local max_sep = 0
    for i = 1, #self.table_desc do
        col = self.table_desc[i]
        if #col.head > max_sep then
            max_sep = #col.head
        end
    end

    for i = 1, #self.table_desc do
        col = self.table_desc[i]
        val = self:convertCell(col, tbl)
        if s ~= "" then
            s = s .. "\n"
        end
        s = s .. string.format("%-"..max_sep.."s : %s", col.head, val)
    end

    return s
end

function Nice_Table:print()
    print(self.header_string)
    local i
    for i=1, #self.rows do
        print(self.rows[i])
    end
    print(self.footer_string)
end

end

--[[
--Example of use:


table_info = {
    { head="Index",len=5,align="c",path="router_arp_index" },
    { head="Address",len=22,align="c",path="mac-address"}
}


tbl = Nice_Table(table_info)
tbl:addRow({router_arp_index=1,["mac-address"]="00:00:00:00:00:01"})
tbl:addRow({router_arp_index=5,["mac-address"]="00:00:00:00:00:08"})

do
    print(tbl.header_string)
    local i
    for i=1, #tbl.rows do
        print(tbl.rows[i])
    end
    print(tbl.footer_string)
end

]]--
