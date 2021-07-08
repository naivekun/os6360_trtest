--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* hexstring.lua
--*
--* DESCRIPTION:
--*       hexstring type definition
--*
--* FILE REVISION NUMBER:
--*       $Revision: 4 $
--*
--********************************************************************************

--includes

--constants

--
-- Types defined:
--
-- hexstring  - HEX string
-- hexnum	- HEX number


-- ************************************************************************
---
--  CLI_check_param_hexstring
--        @description  This function validates that the string s is a
--                      valid hex string, the size of the hex string is
--                      given in the desc parameter
--
--        @param s              - The string to validate
--        @param name           - The name of parameter
--        @param desc           - Checked parameter range
--
--        @return       false if failed validation true if successful
--
function CLI_check_param_hexstring(s, name, desc)
    local i, c

	if desc.min ~= nil and desc.min > string.len(s) then
		return false, "String too small"
	end
	if desc.max ~= nil and desc.max < string.len(s) then
		return false, "String too big"
	end
	for i = 1,string.len(s) do
		c = string.sub(s, 1, 1)
		if (not ( (c >= '0' and c <= '9') or (c >= 'a' and c <= 'f') or
           (c >= 'A' and c <= 'F'))  ) then
			return false, "Found a non-numeric character"
		end
	end

	return true, s
end

CLI_type_dict["hexstring"] = {
    checker = CLI_check_param_hexstring,
    help = "HEX string"
}


-- ************************************************************************
---
--  CLI_check_param_hexnum
--        @description  This function validates that the string s is a
--                      valid hex number (including the prefix "0x"), the size of the
--						hex string is given in the desc parameter
--
--        @param s              - The string to validate
--        @param name           - The name of parameter
--        @param desc           - Checked parameter range
--
--        @return       false if failed validation true if successful
--
function CLI_check_param_hexnum(s, name, desc)
    local i, c

	if (string.lower(string.sub(s,1,2))~="0x") then return false, "Hex value must start with \"0x\"" end

	if desc.min ~= nil and desc.min > string.len(s) -2 then
		return false, "String too small"
	end
	if desc.max ~= nil and desc.max < string.len(s) -2  then
		return false, "String too big"
	end
	for i = 3 ,string.len(s) do
		c = string.sub(s, 1, 1)
		if (not ( (c >= '0' and c <= '9') or (c >= 'a' and c <= 'f') or
           (c >= 'A' and c <= 'F'))  ) then
			return false, "Found a non-numeric character"
		end
	end

	return true, tonumber(s,16)
end

CLI_type_dict["hexnum"] = {
    checker = CLI_check_param_hexnum,
    help = "HEX number"
}

CLI_type_dict["GT_U32_hex"] = {
    checker = CLI_check_param_hexnum,
	max =8 ,
	min =0 ,
    help = "HEX number"
}
