--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* filename.lua
--*
--* DESCRIPTION:
--*       filename type definition
--*
--* FILE REVISION NUMBER:
--*       $Revision: 6 $
--*
--********************************************************************************

--includes

--constants

--
-- Types defined:
--
-- filename  - File name
--


-- ************************************************************************
---
--  CLI_check_param_filename
--        @description  This function validates that the string s is a
--                      valid filename, it accepts lower/upper case
--                      letters, numbers, underscore (_) and a dot (.) when
--                      it is not the first char
--
--        @param s             - The string to validate
--
--        @return       false and an error description if failed validation
--                      true if successful
--
function CLI_check_param_filename(s)
	local i, c

	for i = 1,string.len(s) do
		c = string.sub(s, 1, 1)
		if (not ( (c >= '0' and c <= '9') or (c >= 'a' and c <= 'z') or
                  (c >= 'A' and c <= 'Z') or (c=="_") or (c=='.' and i>1)  )) then
			return false, "The file name is invalid"
		end
	end

	return true,s
end


-- ************************************************************************
---
--  CLI_complete_param_filename
--        @description  complete name of file
--
--        @param param          - checked parameter value
--        @param name           - checked parameter name
--        @param desc           - checked parameter range
--
--        @return       complete and help lists
--
local function CLI_complete_param_filename(param, name, desc)
    local values = { }
    local compl, help, index

    compl   = { }
    help    = { }

    for index = 1, #values do
        if prefix_match(param, tostring(values[index][1])) then
            table.insert(compl, tostring(values[index][1]))
            help[#compl] = values[index][2]
        end
    end

    return compl, help
end


CLI_type_dict["filename"] = {
    checker     = CLI_check_param_filename,
    complete    = CLI_complete_param_filename,
    help        = "File name"
}

CLI_type_dict["cpssApi_eeprom"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Type of file\n",
    enum = {
        ["api"] = { value="api", help="File contents is written as cpss apis" },
        ["eeprom"] = { value="eeprom", help="File contents is written as eeprom file" },
        ["cider"] = { value="cider", help="File contents is written as cider units" },
        ["cli"] = { value="cli", help="File contents is written as luaCLI write commands" },
        ["micro-init"] = { value="micInit", help="File contents is written as micro-init sequence" }
   }
}

CLI_type_dict["cpssApi_eeprom__without_cider"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Type of file",
    enum = {
        ["api"] = {
            value = "api",
            help  = "File contents is written as cpss API"
        },
        ["eeprom"] = {
            value = "eeprom",
            help  = "File contents is written as EEPROM file"
        },
        ["cli"] = {
            value = "cli",
            help  = "File contents is written as luaCLI write commands"
        },
        ["micro-init"] = {
            value = "micInit",
            help  = "File contents is written as micro-init sequence"
        }
   }
}
