--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* string.lua
--*
--* DESCRIPTION:
--*       string conversions functions
--*
--* FILE REVISION NUMBER:
--*       $Revision: 20 $
--*
--********************************************************************************

--includes

--constants
portSpeedSuffix = "speed_"


-- ************************************************************************
---
--  isEmptyStr
--        @description  checks if string is empty
--
--        @param str            - checked string
--
--        @return       true, if string contains non-whitespace characters,
--                      flase, otherwise
--
function isEmptyStr(str)
    if "string" == type(str) then
        return nil == string.find(str, "([^%s]+)")
    else
        return false
    end
end


-- ************************************************************************
---
--  concatHeaderBodyAndFooter
--        @description  concat's body with header and footer;
--                      data could has string or table type
--
--        @param header         - header
--        @param body           - main part
--        @param footer         - footer, could be irrelevant
--
--        @return       concatenated data
--
function concatHeaderBodyAndFooter(header, body, footer)
    if nil ~= footer then
        if "string" == type(body)   then
            body    = header .. body .. footer
        else
            tablePrepend(body, header)
            tableAppend(body, footer)
        end
    else
        if "string" == type(body)   then
            body    = header .. body
        else
            tablePrepend(body, header)
        end
    end

    return body
end


-- ************************************************************************
---
--  isParameterName
--        @description  check's if it is parameter identifier
--
--        @param s                 - checked parameter name
--
--        @return       true if entry name is standart parameter identifier,
--                      otherwise false
--
function isParameterName(s)
    local status
    local i, c

    status = true

    if true == status then
        if (0 >= string.len(s)) or (31 < string.len(s)) then
            status = false
        end
    end

    if true == status then
        c = string.sub(s, 1, 1)
        if not ((c >= 'a' and c <= 'z') or (c >= 'A' and c <= 'Z'))     then
            status = false
        end
    end

    if true == status then
        for i = 2, string.len(s) do
            c = string.sub(s, i, i)
            if not ((c >= 'a' and c <= 'z') or (c >= 'A' and c <= 'Z')  or
                    (c >= '0' and c <= '9') or (c == "_"))              then
                status = false
                break
            end
        end
    end

    if true == status then
        return true, s
    else
        return false
    end
end


-- ************************************************************************
---
--  findFirstStr
--        @description  returns first string matched pattern
--
--        @param str            - entry string
--        @param pattern        - pattern
--
--        @return       matched string or nil if not found
--
function findFirstStr(str, pattern)
    local begin_pos, end_pos = string.find(str, pattern)

    if nil ~= begin_pos then
        return string.sub(str, begin_pos, end_pos)
    else
        return nil
    end
end


-- ************************************************************************
---
--  replaceStr
--        @description  perlaces part of string
--
--        @param str            - processed entry string
--        @param perlaced       - replaced substring
--        @param replace_substring
--                              - substring for replacing
--
--        @return       replaced entry string
--
function replaceStr(str, perlaced, replace_substring)
    return string.gsub(str, perlaced, replace_substring)
end


-- ************************************************************************
---
--  trim
--        @description  trim's the string
--
--        @param str            - processed entry string
--
--        @return       trimmed string
--
function trim(str)
    if "string" == type(str)    then
        return (string.gsub(str, "^%s*(.-)%s*$", "%1"))
    else
        return str
    end
end


-- ************************************************************************
---
--  startsWith
--        @description checks if string str starts with string strStart
--
--        @param  str           - processed entry string
--        @param  strStart      - prefix string
--
--        @return    -  true if string str starts with a string strStart
--
function startsWith(str, strStart)
    return string.sub(str, 1, string.len(strStart))==strStart
end


-- ************************************************************************
---
--  endsWith
--        @description checks if string str ends with string strEnd
--
--        @param  str           - processed entry string
--        @param  strEnd        - suffix string
--
--        @return    -  true if string str ends with a string strEnd
--
function endsWith(str, strEnd)
    return strEnd=='' or string.sub( str, -string.len(strEnd))==strEnd
end


-- ************************************************************************
---
--  deleteLastNewLine
--        @description  deletes last new line of the string
--
--        @param str            - processed entry string
--
--        @return       string without last new line
--
function deleteLastNewLine(str)
    if "string" == type(str)    then
        if "\n" == string.sub(str, -1) then
            return string.sub(str, 1, -2)
         else
            return str
         end
    else
        return str
    end
end


-- ************************************************************************
---
--  arrayToString
--        @description  converts string stored in C-array to lua string
--
--        @param array_value    - entry array
--
--        @return       converted string
--
function arrayToString(array_value)
    local index = 0
    local returned_string = ""

    while 0 ~= array_value[index] do
        returned_string = returned_string .. string.char(array_value[index])
        index = index + 1
    end

    return returned_string
end

-- ************************************************************************
---
--  splitString
--        @description  splits single string to multiple strings separated by
--                      delimiter defined by pattern
--
--        @param str    - string
--               pat    - delimiting pattern
--
--        @return       table of strings
--
function splitString(str, pat)
    local t = {}
    local fpat = "(.-)" .. pat
    local last_end = 1
    local s, e, cap = str:find(fpat, 1)
    while s do
        if s ~= 1 or cap ~= "" then
	        table.insert(t,cap)
        end
        last_end = e+1
        s, e, cap = str:find(fpat, last_end)
    end
    if last_end <= #str then
        cap = str:sub(last_end)
        table.insert(t, cap)
    end
    return t
end

-- ************************************************************************
---
--  dumpTableToConsole
--        @description  dumping of all table item etrings to console
--                      screen
--
--        @param tbl            - The table
--
--        @return       operation succed
--
function dumpTableToConsole(tbl, sp)
    local index, item

    if "table" == type(tbl) then
        for index, item in pairs(tbl) do
            print(tostring(item))
        end
    end

    return 0
end


-- ************************************************************************
---
--  alignLeftToCenterStr
--        @description  adds necessary count of backspaces at beginning
--                      of the sting for alinged outputing from left to
--                      right
--
--        @param str            - The port speed string
--        @param size           - Displaing symbols count
--
--        @return       normilized string
--
function alignLeftToCenterStr(str, size)
    if str == nil then
        return string.rep(" ", size)
    end
    local correction = math.floor((size - #str) / 2)
    if 0 < correction then
        return string.rep(" ", correction) .. str
    else
        return str
    end
end


-- ************************************************************************
---
--  normilizePortSpeedStr
--        @description  normilizes port speed string
--
--        @param speed          - The port speed string
--
--        @return       normilized string
--
function normilizePortSpeedStr(speed)
    if portSpeedSuffix == string.sub(speed, 1, string.len(portSpeedSuffix)) then
        return string.sub(speed, string.len(portSpeedSuffix) + 1)
    else
        return speed
    end
end


-- ************************************************************************
---
--  boolStrGet
--        @description  converts bool value to string
--
--        @param bool_value     - The entry boolean value
--
--        @return       "True" if bool_value == true, otherwise "False"
--
function boolStrGet(bool_value)
    if true == bool_value then
        return "True"
    else
        return "False"
    end
end


-- ************************************************************************
---
--  boolEnabingStrGet
--        @description  converts bool value to Enable/Disable
--
--        @param bool_value     - The entry boolean value
--
--        @return       "Enable" if bool_value == true, otherwise "Disable"
--
function boolEnabingStrGet(bool_value)
    if true == bool_value then
        return "Enable"
    else
        return "Disable"
    end
end


-- ************************************************************************
---
--  boolEnableLowerStrGet
--        @description  converts bool value to enable/disable (in lover
--                      case)
--
--        @param bool_value     - The entry boolean value
--
--        @return       "enable" if bool_value == true, otherwise "disable"
--
function boolEnableLowerStrGet(bool_value)
    if true == bool_value then
        return "enable"
    else
        return "disable"
    end
end


-- ************************************************************************
---
--  boolEnabledLowerStrGet
--        @description  converts bool value to enabled/disabled (in lover
--                      case)
--
--        @param bool_value     - The entry boolean value
--
--        @return       "enabled" if bool_value == true, otherwise
--                      "disabled"
--
function boolEnabledLowerStrGet(bool_value)
    if true == bool_value then
        return "enabled"
    else
        return "disabled"
    end
end


-- ************************************************************************
---
--  boolEnabledUpperStrGet
--        @description  converts bool value to enabled/disabled (in upper
--                      case)
--
--        @param bool_value     - The entry boolean value
--
--        @return       "Enabled" if bool_value == true, otherwise
--                      "Disabled"
--
function boolEnabledUpperStrGet(bool_value)
    if true == bool_value then
        return "Enabled"
    else
        return "Disabled"
    end
end


-- ************************************************************************
---
--  taggedStrGet
--        @description  converts bool value to tagged/untaged (in lover
--                      case)
--
--        @param bool_value     - The entry boolean value
--
--        @return       "tagged"   if bool_value == true,
--                      "untagged" if bool_value == false,
--                      otherwice empty string
--
function taggedStrGet(bool_value)
    if      true == bool_value   then
        return "tagged"
    elseif false == bool_value  then
        return "untagged"
    else
        return ""
    end
end

-- ************************************************************************
---
--  boolNegativeEnableLowerStrGet
--        @description  converts bool value to enable/disable (in lower
--                      case)
--
--        @param bool_value     - The entry boolean value
--
--        @return       "enable" if bool_value == false, otherwise "disable"
--
function boolNegativeEnableLowerStrGet(bool_value)
    if true ~= bool_value then
        return "enable"
    else
        return "disable"
    end
end


-- ************************************************************************
---
--  boolEnablingLowerStrGet
--        @description  converts bool value to enabling/disabling (in lover
--                      case)
--
--        @param bool_value     - The entry boolean value
--
--        @return       "enabling" if bool_value == true, otherwise
--                      "disabling"
--
function boolEnablingLowerStrGet(bool_value)
    if true == bool_value then
        return "enabling"
    else
        return "disabling"
    end
end


-- ************************************************************************
---
--  boolEnablingLowerOnTrunkL4LbhModeStrGet
--        @description  converts trunk L4 LBH mode value to enabling/
--                      disabling (in lover case)
--
--        @param trunk_l4_lbh_mode     - The entry trunk L4 LBH mode value
--
--        @return       "enabling" if
--                          trunk_l4_lbh_mode !=
--                              "CPSS_DXCH_TRUNK_L4_LBH_DISABLED_E",
--                      otherwise "disabling"
--
function boolEnablingLowerOnTrunkL4LbhModeStrGet(trunk_l4_lbh_mode)
    if "CPSS_DXCH_TRUNK_L4_LBH_DISABLED_E" ~= trunk_l4_lbh_mode then
        return "enabling"
    else
        return "disabling"
    end
end


-- ************************************************************************
---
--  boolEnablingUpperOnTrunkL4LbhModeStrGet
--        @description  converts trunk L4 LBH mode value to enabling/
--                      disabling (in upper case)
--
--        @param trunk_l4_lbh_mode     - The entry trunk L4 LBH mode value
--
--        @return       "Enabling" if
--                          trunk_l4_lbh_mode !=
--                              "CPSS_DXCH_TRUNK_L4_LBH_DISABLED_E",
--                      otherwise "Disabling"
--
function boolEnablingUpperOnTrunkL4LbhModeStrGet(trunk_l4_lbh_mode)
    if "CPSS_DXCH_TRUNK_L4_LBH_DISABLED_E" ~= trunk_l4_lbh_mode then
        return "Enabling"
    else
        return "Disabling"
    end
end


-- ************************************************************************
---
--  boolEnablingUpperStrGet
--        @description  converts bool value to enabling/disabling (in upper
--                      case)
--
--        @param bool_value     - The entry boolean value
--
--        @return       "Enabling" if bool_value == true, otherwise
--                      "Disabling"
--
function boolEnablingUpperStrGet(bool_value)
    if true == bool_value then
        return "Enabling"
    else
        return "Disabling"
    end
end


-- ************************************************************************
---
--  boolYesNoStrGet
--        @description  converts bool value to Yes/No
--
--        @param bool_value     - The entry boolean value
--
--        @return       "Yes" if bool_value == true, otherwise "No"
--
function boolYesNoStrGet(bool_value)
    if true == bool_value then
        return "Yes"
    else
        return "No"
    end
end


-- ************************************************************************
---
--  boolUpDownStrGet
--        @description  converts bool value to Up/Down
--
--        @param bool_value     - The entry boolean value
--
--        @return       "Up" if bool_value == true, otherwise "Down"
--
function boolUpDownStrGet(bool_value)
    if true == bool_value then
        return "Up"
    else
        return "Down"
    end
end


-- ************************************************************************
---
--  boolPredicatStrGet
--        @description  converts bool value to verb
--
--        @param bool_value     - The entry boolean value
--
--        @return       "set" if bool_value == true, otherwise "false"
--
function boolPredicatStrGet(bool_value)
    if true == bool_value then
        return "set"
    else
        return "unset"
    end
end


-- ************************************************************************
---
--  boolNounPredicatStrGet
--        @description  converts bool value to substantivization
--
--        @param bool_value     - The entry boolean value
--
--        @return       "setting" if bool_value == true,
--                      otherwise "unsetting"
--
function boolNounPredicatStrGet(bool_value)
    if true == bool_value then
        return "setting"
    else
        return "unsetting"
    end
end


-- ************************************************************************
---
--  boolStaticDynamicStrGet
--        @description  converts bool value to static/dynamic string
--
--        @param bool_value     - The entry boolean value
--
--        @return       "static" if bool_value == true,
--                      otherwise "dynamic"
--
function boolStaticDynamicStrGet(bool_value)
    if true == bool_value then
        return "static"
    else
        return "dynamic"
    end
end


-- ************************************************************************
---
--  boolForwardDropGet
--        @description  converts bool value to Forward/Drop
--
--        @param bool_value     - The entry boolean value
--
--        @return       "Drop" if bool_value == true, otherwise "Forward"
--
function boolForwardDropGet(bool_value)
    if true == bool_value then
        return "Drop"
    else
        return "Forward"
    end
end


-- ************************************************************************
---
--  strIfNotNil
--        @description  value string or "n/a" if it is nil
--
--        @param v              - value
--
--        @return       value string or "n/a"
--
function strIfNotNil(v)
    if nil ~= v then
        return tostring(v)
    else
        return "n/a"
    end
end


-- ************************************************************************
---
--  stringValueIfConditionTrue
--        @description  getting of string value or its default according to
--                      boolean condition
--
--        @param cond               - condition
--        @param value              - given value
--        @param default            - default value, could be irrelevant
--
--        @return       value if condition is true, otherwise default
--
function stringValueIfConditionTrue(cond, value, default)
    if    true == cond      then
        return tostring(value)
    elseif nil ~= default   then
        return tostring(default)
    else
        return ""
    end
end


-- ************************************************************************
---
--  separatedNumberStrGet
--        @description  updates entry srting to separated string
--                          CPSS_4.0_059 => 4.0.59
--
--        @param str            - entry string
--        @param sep            - separator
--
--        @return       port speed string
--
function separatedNumberStrGet(str, sep)
    local pos, cur_pos, end_pos
    local number, number_order
    local ret = ""

    if nil == sep then
        sep = '.'
    end

    if "string" == type(str) then
        cur_pos         = 1
        end_pos         = string.len(str)
        number_order    = 0

        while cur_pos ~= nil    do
            cur_pos, pos, number = string.find(str, "%d+", cur_pos)

            if nil ~= cur_pos   then
                number = string.sub(str, cur_pos, pos)

                number_order = number_order + 1
                if 1 < number_order then
                    ret = ret .. sep .. tostring(number)
                else
                    ret = tostring(number)
                end

                cur_pos = pos + 1
                if cur_pos > end_pos then
                    cur_pos = nil
                end
            end
        end
    end

    return ret
end


-- ************************************************************************
---
--  speedStrGet
--        @description  convert's CPSS_PORT_SPEED_ENT member string
--
--        @param port_speed    - The entry CPSS_PORT_SPEED_ENT member
--                               string
--
--        @return       port speed string , and it's number (Kbps)
--
function speedStrGet(port_speed)
    if     "CPSS_PORT_SPEED_10_E"    == port_speed      then
        return "10" , 10
    elseif "CPSS_PORT_SPEED_100_E"   == port_speed      then
        return "100", 100
    elseif "CPSS_PORT_SPEED_1000_E"  == port_speed      then
        return "1G" , 1000
    elseif "CPSS_PORT_SPEED_10000_E" == port_speed      then
        return "10G" , 10000
    elseif "CPSS_PORT_SPEED_12000_E" == port_speed      then
        return "12G" , 12000
    elseif "CPSS_PORT_SPEED_12500_E" == port_speed      then
        return "12.5G" , 12500
    elseif "CPSS_PORT_SPEED_2500_E"  == port_speed      then
        return "2.5G" , 2500
    elseif "CPSS_PORT_SPEED_5000_E"  == port_speed      then
        return "5G" , 5000
    elseif "CPSS_PORT_SPEED_13600_E" == port_speed      then
        return "13.6G" , 13600
    elseif "CPSS_PORT_SPEED_20000_E" == port_speed      then
        return "20G" , 20000
    elseif "CPSS_PORT_SPEED_40000_E" == port_speed      then
        return "40G" , 40000
    elseif "CPSS_PORT_SPEED_16000_E" == port_speed      then
        return "16G" , 16000
    elseif "CPSS_PORT_SPEED_15000_E" == port_speed      then
        return "15G" , 15000
    elseif "CPSS_PORT_SPEED_75000_E" == port_speed      then
        return "75G" , 75000
    elseif "CPSS_PORT_SPEED_100G_E" == port_speed      then
        return "100G" , 100000
    elseif "CPSS_PORT_SPEED_50000_E" == port_speed      then
        return "50G" , 50000
    elseif "CPSS_PORT_SPEED_140G_E" == port_speed      then
        return "140G" , 140000
    elseif "CPSS_PORT_SPEED_11800_E" == port_speed      then
        return "11.8G" , 11800
    elseif "CPSS_PORT_SPEED_47200_E" == port_speed      then
        return "47.2G" , 47200
    elseif "CPSS_PORT_SPEED_22000_E" == port_speed      then
        return "22G"  , 22000
    elseif "CPSS_PORT_SPEED_25000_E" == port_speed      then
        return "25G"  , 25000
    elseif "CPSS_PORT_SPEED_107G_E" == port_speed      then
        return "107"  , 107000
    else
        return "n/a" , 0
    end
end

-- ************************************************************************
---
--  classIndicationStrGet
--        @description  convert's CPSS_DXCH_LED_INDICATION_ENT member
--                      string
--
--        @param port_interface
--                             - The entry CPSS_DXCH_LED_INDICATION_ENT
--                               member string
--
--        @return       port interface string
--
function classIndicationStrGet(indication)
    if     "CPSS_DXCH_LED_INDICATION_PORT_ENABLED_E" == indication then
        return "Port En"
    elseif "CPSS_DXCH_LED_INDICATION_LINK_E"         == indication then
        return "Link OK"
    elseif "CPSS_DXCH_LED_INDICATION_RX_TX_ACT_E"    == indication then
        return "Activity"
    elseif "CPSS_DXCH_LED_INDICATION_RX_ERROR_E"     == indication then
        return "Rx Error"
    elseif "CPSS_DXCH_LED_INDICATION_FC_TX_E"        == indication then
        return "Tx FC"
    elseif "CPSS_DXCH_LED_INDICATION_FC_RX_E"        == indication then
        return "Rx FC"
    elseif "CPSS_DXCH_LED_INDICATION_FULL_DUPLEX_E"  == indication then
        return "Full Duplex"
    elseif "CPSS_DXCH_LED_INDICATION_GMII_SPEED_E"   == indication then
        return "GMII Speed"
    elseif "CPSS_DXCH_LED_INDICATION_MII_SPEED_E"    == indication then
        return "MII Speed"

    else
        return "n/a"
    end
end


-- ************************************************************************
---
--  interfaceStrGet
--        @description  convert's CPSS_PORT_INTERFACE_MODE_ENT member
--                      string
--
--        @param port_interface
--                             - The entry CPSS_PORT_INTERFACE_MODE_ENT
--                               member string
--
--        @return       port interface string
--
function interfaceStrGet(port_interface)
    if     "CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E" == port_interface     then
        return "10 bit"
    elseif "CPSS_PORT_INTERFACE_MODE_REDUCED_GMII_E"  == port_interface     then
        return "GMII"
    elseif "CPSS_PORT_INTERFACE_MODE_MII_E"           == port_interface     then
        return "MII"
    elseif "CPSS_PORT_INTERFACE_MODE_SGMII_E"         == port_interface     then
        return "SGMII"
    elseif "CPSS_PORT_INTERFACE_MODE_XGMII_E"         == port_interface     then
        return "XGMII"
    elseif "CPSS_PORT_INTERFACE_MODE_MGMII_E"         == port_interface     then
        return "MGMII"
    elseif "CPSS_PORT_INTERFACE_MODE_1000BASE_X_E"    == port_interface     then
        return "1000_BaseX"
    elseif "CPSS_PORT_INTERFACE_MODE_GMII_E"          == port_interface     then
        return "GMII"
    elseif "CPSS_PORT_INTERFACE_MODE_MII_PHY_E"       == port_interface     then
        return "MII Phy"
    elseif "CPSS_PORT_INTERFACE_MODE_QX_E"            == port_interface     then
        return "QX"
    elseif "CPSS_PORT_INTERFACE_MODE_HX_E"            == port_interface     then
        return "HX"
    elseif "CPSS_PORT_INTERFACE_MODE_RXAUI_E"         == port_interface     then
        return "RXAUI"
    elseif "CPSS_PORT_INTERFACE_MODE_100BASE_FX_E"    == port_interface     then
        return "100_BaseFX"
    elseif "CPSS_PORT_INTERFACE_MODE_QSGMII_E"        == port_interface     then
        return "QSGMII"
    elseif "CPSS_PORT_INTERFACE_MODE_XLG_E"           == port_interface     then
        return "XLG"
    elseif "CPSS_PORT_INTERFACE_MODE_LOCAL_XGMII_E"   == port_interface     then
        return "Local SCMII"
    elseif "CPSS_PORT_INTERFACE_MODE_KR_E"            == port_interface     then
        return "KR"
    elseif "CPSS_PORT_INTERFACE_MODE_KR2_E"           == port_interface     then
        return "KR2"
    elseif "CPSS_PORT_INTERFACE_MODE_KR4_E"           == port_interface     then
        return "KR4"
    elseif "CPSS_PORT_INTERFACE_MODE_HGL_E"           == port_interface     then
        return "HGL"
    elseif "CPSS_PORT_INTERFACE_MODE_CHGL_12_E"       == port_interface     then
        return "CHGL"
    elseif "CPSS_PORT_INTERFACE_MODE_ILKN12_E"        == port_interface     then
        return "ILKN12"
    elseif "CPSS_PORT_INTERFACE_MODE_SR_LR_E"         == port_interface     then
        return "SR_LR"
    elseif "CPSS_PORT_INTERFACE_MODE_SR_LR2_E"         == port_interface     then
        return "SR_LR2"
    elseif "CPSS_PORT_INTERFACE_MODE_SR_LR4_E"         == port_interface     then
        return "SR_LR4"
    elseif "CPSS_PORT_INTERFACE_MODE_ILKN16_E"        == port_interface     then
        return "ILKN16"
    elseif "CPSS_PORT_INTERFACE_MODE_ILKN24_E"        == port_interface     then
        return "ILKN24"
    elseif "CPSS_PORT_INTERFACE_MODE_ILKN4_E"         == port_interface     then
        return "ILKN4"
    elseif "CPSS_PORT_INTERFACE_MODE_ILKN8_E"         == port_interface     then
        return "ILKN8"
    elseif "CPSS_PORT_INTERFACE_MODE_XHGS_E"          == port_interface     then
        return "XHGS"
    elseif "CPSS_PORT_INTERFACE_MODE_XHGS_SR_E"       == port_interface     then
        return "XHGS_SR"
    elseif "CPSS_PORT_INTERFACE_MODE_KR_C_E"          == port_interface     then
        return "KR_CONS"
    elseif "CPSS_PORT_INTERFACE_MODE_CR_C_E"          == port_interface     then
        return "CR_CONS"
    elseif "CPSS_PORT_INTERFACE_MODE_KR2_C_E"         == port_interface     then
        return "KR2_CONS"
    elseif "CPSS_PORT_INTERFACE_MODE_CR2_C_E"         == port_interface     then
        return "CR2_CONS"
    elseif "CPSS_PORT_INTERFACE_MODE_CR_E"            == port_interface     then
        return "CR"
    elseif "CPSS_PORT_INTERFACE_MODE_CR2_E"            == port_interface     then
        return "CR2"
    elseif "CPSS_PORT_INTERFACE_MODE_CR4_E"            == port_interface     then
        return "CR4"
    elseif "CPSS_PORT_INTERFACE_MODE_KR_S_E"            == port_interface     then
        return "KR_S"
    elseif "CPSS_PORT_INTERFACE_MODE_CR_S_E"            == port_interface     then
        return "CR_S"

    else
        return "n/a"
    end
end


-- ************************************************************************
---
--  cascadePortTypeStrGet
--        @description  convert's CPSS_CSCD_PORT_TYPE_ENT member string
--
--        @param cascade_port_type
--                             - The entry CPSS_CSCD_PORT_TYPE_ENT member
--                               string
--
--        @return       cascade_port_type string
--
function cascadePortTypeStrGet(cascade_port_type)
    if     "CPSS_CSCD_PORT_DSA_MODE_1_WORD_E"  == cascade_port_type then
        return "Reg"
    elseif "CPSS_CSCD_PORT_DSA_MODE_2_WORDS_E" == cascade_port_type then
        return "Ext"
    elseif "CPSS_CSCD_PORT_DSA_MODE_3_WORDS_E" == cascade_port_type then
        return "3"
    elseif "CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E" == cascade_port_type then
        return "4"
    elseif          "CPSS_CSCD_PORT_NETWORK_E" == cascade_port_type then
        return "Net"
    else
        return "n/a"
    end
end

local function ipv6_sip_dip_to_string(xip)
    if xip[0] == 0 and xip[1] == 0 and xip[2] == 0 and xip[3] == 0 then
        return "0::0"
    end
    return string.format("*:%02x%02x:%02x%02x",xip[0],xip[1],xip[2],xip[3])
end

local function fdb_type_to_legacy(entry_type)
    if entry_type == "CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E" then
        return "CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E";
    end
    if entry_type == "CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_FID_VID1_E" then
        return "CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E";
    end
    if entry_type == "CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E" then
        return "CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E";
    end
    if entry_type == "CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_FID_VID1_E" then
        return "CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E";
    end
    if entry_type == "CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_E" then
        return "CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_E";
    end
    if entry_type == "CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_FID_VID1_E" then
        return "CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_E";
    end
    return entry_type;
end

-- ************************************************************************
---
--  sipAddressStrGet
--        @description  gets mac of source ip v4 or v6 address according
--                      to given entry type
--
--        @param entry_type     - entry data type, affects the choice of
--                                address string
--        @param sip            - source ip address, could be irrelevant
--                                (depends on entry_type)
--
--        @return       empty string,   if entry_type ==
--                                    "CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E";
--                      sip string,     if entry_type ==
--                                  "CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E";
--                      sip string,     if entry_type ==
--                                  "CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_E"
--
function sipAddressStrGet(entry_type, sip)
    entry_type = fdb_type_to_legacy(entry_type);
    if     "CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E" == entry_type then
        return ""
    elseif "CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E" == entry_type then
        return sip
    elseif "CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_E" == entry_type then
        return ipv6_sip_dip_to_string(sip)
    else
        return "n/a"
    end
end

-- ************************************************************************
---
--  macOrDipAddressStrGet
--        @description  gets source ip v4/v6 address or n/a according
--                      to given entry type
--
--        @param entry_type     - entry data type, affects the choice of
--                                address string
--        @param mac_address    - mac-address, could be irrelevant
--                                (depends on entry_type)
--        @param dip            - destination ip addresscould be irrelevant
--                                (depends on entry_type)
--
--        @return       mac-address string, if entry_type ==
--                                    "CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E";
--                      dip string,         if entry_type ==
--                                  "CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E";
--                      dip string,         if entry_type ==
--                                  "CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_E"
--
--[[function macOrDipAddressStrGet(entry_type, mac_address, dip)
    entry_type = fdb_type_to_legacy(entry_type);
    if     "CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E" == entry_type then
        return mac_address
    elseif "CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E" == entry_type then
        return dip
    elseif "CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_E" == entry_type then
        return ipv6_sip_dip_to_string(dip)
    else
        return "n/a"
    end
end
]]--

function macOrDipAddressStrGet(entry_type, macItem, ipItem)
    entry_type = fdb_type_to_legacy(entry_type);
    if     "CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E" == entry_type then
        return macItem.macAddr
    elseif "CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E" == entry_type then
        return ipItem.dip
    elseif "CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_E" == entry_type then
        return ipv6_sip_dip_to_string(ipItem.dip)
    else
        return "n/a"
    end
end


-- ************************************************************************
---
--  macOrIpAddressFirstStrGet
--        @description  gets first part of mac/ipv4/ipv6 address according
--                      to given entry data
--
--        @param entry_type     - entry data type, affects the choice of
--                                address string
--        @param mac_address    - mac-address, could be irrelevant
--                                (depends on entry_type)
--        @param sip            - source ip address, could be irrelevant
--                                (depends on entry_type)
--        @param dip            - destination ip addresscould be irrelevant
--                                (depends on entry_type)
--
--        @return       mac-address string, if entry_type ==
--                                    "CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E";
--                      sip string,         if entry_type ==
--                                  "CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E";
--                      sip string,         if entry_type ==
--                                  "CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_E"
--
function macOrIpAddressFirstStrGet(entry_type, mac_address, sip, dip)
    entry_type = fdb_type_to_legacy(entry_type);
    if        "CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E" == entry_type then
        return mac_address
    elseif ("CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E" == entry_type) or
           ("CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_E" == entry_type) then
        return "sip=" .. sip
    else
        return "n/a"
    end
end

-- ************************************************************************
---
--  macOrIpAddressSecondStrGet
--        @description  gets second part of mac/ipv4/ipv6 address according
--                      to given entry data
--
--        @param entry_type     - entry data type, affects the choice of
--                                address string
--        @param mac_address    - mac-address, could be irrelevant
--                                (depends on entry_type)
--        @param sip            - source ip address, could be irrelevant
--                                (depends on entry_type)
--        @param dip            - destination ip addresscould be irrelevant
--                                (depends on entry_type)
--
--        @return       nil,                if entry_type ==
--                                    "CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E";
--                      dip string,         if entry_type ==
--                                  "CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E";
--                      dip string,         if entry_type ==
--                                  "CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_E"
--
function macOrIpAddressSecondStrGet(entry_type, mac_address, sip, dip)
    entry_type = fdb_type_to_legacy(entry_type);
    if        "CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E" == entry_type then
        return ""
    elseif ("CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E" == entry_type) or
           ("CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_E" == entry_type) then
        return "dip=" .. dip
    else
        return ""
    end
end


-- ************************************************************************
---
--  duplexStrGet
--        @description  convert CPSS_PORT_DUPLEX_ENT member string to
--                      Half/Full
--
--        @param port_duplex    - The entry CPSS_PORT_DUPLEX_ENT member
--                                string
--
--        @return       "Half" if port_duplex == "CPSS_PORT_HALF_DUPLEX_E",
--                      "Full" if port_duplex == "CPSS_PORT_FULL_DUPLEX_E",
--                      otherwise "n/a"
--
function duplexStrGet(port_duplex)
    if     "CPSS_PORT_HALF_DUPLEX_E" == port_duplex then
        return "Half"
    elseif "CPSS_PORT_FULL_DUPLEX_E" == port_duplex then
        return "Full"
    else
        return "n/a"
    end
end


-- ************************************************************************
---
--  bridgingIpv4ModeStrGet
--        @description  convert CPSS_BRG_IPM_MODE_ENT member string to
--                      MAC-GROUP/IP-GROUP/IP-SRC-GROUP
--
--        @param bridging_mode_enabling
--                              - Enabling property of bringing mode
--        @param bridging_mode  - The entry CPSS_BRG_IPM_MODE_ENT member
--                                string
--
--        @return       "MAC-GROUP"      if bridging_mode_enabling
--                                                                == false;
--                      "IP-GROUP"       if bridging_mode_enabling == true,
--                                          bridging_mode
--                                                  == "CPSS_BRG_IPM_GV_E";
--                      "IP-SRC-GROUP"   if bridging_mode_enabling == true,
--                                          bridging_mode
--                                                 == "CPSS_BRG_IPM_SGV_E";
--                      otherwise "n/a"
--
function bridgingIpv4ModeStrGet(bridging_mode_enabling, bridging_mode)
    if    false == bridging_mode_enabling   then
        return "MAC-GROUP"
    else
        if      "CPSS_BRG_IPM_GV_E" == bridging_mode    then
            return "IP-GROUP"
        elseif "CPSS_BRG_IPM_SGV_E" == bridging_mode    then
            return "IP-SRC-GROUP"
        else
            return "n/a"
        end
    end
end


-- ************************************************************************
---
--  bridgingIpv6ModeStrGet
--        @description  convert CPSS_BRG_IPM_MODE_ENT member string to
--                      MAC-GROUP/IPv6-GROUP/IPv6-SRC-GROUP
--
--        @param bridging_mode_enabling
--                              - Enabling property of bringing mode
--        @param bridging_mode  - The entry CPSS_BRG_IPM_MODE_ENT member
--                                string
--
--        @return       "MAC-GROUP"      if bridging_mode_enabling
--                                                                == false;
--                      "IPv6-GROUP"     if bridging_mode_enabling == true,
--                                          bridging_mode
--                                                  == "CPSS_BRG_IPM_GV_E";
--                      "IPv6-SRC-GROUP" if bridging_mode_enabling == true,
--                                      bridging_mode
--                                                 == "CPSS_BRG_IPM_SGV_E";
--                      otherwise "n/a"
--
function bridgingIpv6ModeStrGet(bridging_mode_enabling, bridging_mode)
    if    false == bridging_mode_enabling   then
        return "MAC-GROUP"
    else
        if      "CPSS_BRG_IPM_GV_E" == bridging_mode    then
            return "IPv6-GROUP"
        elseif "CPSS_BRG_IPM_SGV_E" == bridging_mode    then
            return "IPv6-SRC-GROUP"
        else
            return "n/a"
        end
    end
end


-- ************************************************************************
---
--  fdbActionModeGet
--        @description  converts aging mode string to fdb-action mode
--
--        @param agingModeStr       - aging mode string
--
--        @return       if aginf mode is    "auto_removal" then return
--                          "CPSS_FDB_ACTION_AGE_WITH_REMOVAL_E"
--                      if aginf mode is    "no_removal" then return
--                          "CPSS_FDB_ACTION_AGE_WITHOUT_REMOVAL_E"
--                      if aginf mode is    "controlled_removal" then
--                          return "CPSS_FDB_ACTION_DELETING_E"
--
function fdbActionModeGet(agingModeStr)
    if           ("auto_removal" == agingModeStr)   or
                 ("auto-removal" == agingModeStr)   then
        return "CPSS_FDB_ACTION_AGE_WITH_REMOVAL_E"
    elseif         ("no_removal" == agingModeStr)   or
                   ("no-removal" == agingModeStr)   then
        return "CPSS_FDB_ACTION_AGE_WITHOUT_REMOVAL_E"
    elseif ("controlled_removal" == agingModeStr)   or
           ("controlled-removal" == agingModeStr)   then
        return "CPSS_FDB_ACTION_DELETING_E"
    else
        return "CPSS_FDB_ACTION_AGE_WITH_REMOVAL_E"
    end
end


-- ************************************************************************
---
--  actionModeStrGet
--        @description  get's aging mode string
--
--        @param fdbActionMode      - fdb action mode getting according to
--                                    string vcalues of type
--                                    "CPSS_FDB_ACTION_MODE_ENT"
--
--        @return       GT_OK and mac address adding condition, if check
--                      was successful
--                      GT_OK, mac address adding condition and checking
--                      warning message, if check has failed
--                      error code and error message
--
function actionModeStrGet(fdbActionMode)
    if        "CPSS_FDB_ACTION_AGE_WITH_REMOVAL_E" == fdbActionMode then
        return "auto-removal"
    elseif "CPSS_FDB_ACTION_AGE_WITHOUT_REMOVAL_E" == fdbActionMode then
        return "no-removal"
    elseif            "CPSS_FDB_ACTION_DELETING_E" == fdbActionMode then
        return "controlled-removal"
    elseif       "CPSS_FDB_ACTION_TRANSPLANTING_E" == fdbActionMode then
        return "transplanting"
    else
        return "n/a"
    end
end


-- ************************************************************************
---
--  addressStrGet
--        @description  converts address string
--
--        @param comp1          - first address component
--        @param comp2          - second address component
--        @param comp3          - therd address component
--
--        @return       concatenation of defined components
--
function addressStrGet(comp1, comp2, comp3)
    local returned_str = ""

    function add_comp(comp)
        if nil ~= comp      then
            returned_str = returned_str .. " " .. comp
        end
    end

    add_comp(comp1)
    add_comp(comp2)
    add_comp(comp3)

    return returned_str
end


-- ************************************************************************
---
--  ipPrefixStrGet
--        @description  converts ip v4 or v6 prefix
--
--        @param ip_protocol    - ip protocol (not used)
--        @param ip_address     - ip v4 or v6 address
--        @param prefix_length  - prefix length
--
--        @return       concatenation of defined components
--
function ipPrefixStrGet(ip_protocol, ip_address, prefix_length)
    if "string" ~= type(ip_address) then
        return tostring(ip_address["string"]) .. "/" .. tostring(prefix_length)
    else
        return ip_address .. "/" .. tostring(prefix_length)
    end
end


-- ************************************************************************
---
--  itemsStrGet
--        @description  getting of range string from item array
--                      {1, 2, 3, 4} => 1-4
--
--        @param items          - items range
--
--        @return       items range string
--
function itemsStrGet(items)
    table.sort(items)
    local nxt = nil
    local i,port
    local last
    local result = ""

    for i,port in pairs(items) do
        if port ~= nxt then
            if result ~= "" then
                if nxt ~= nil and last + 1 ~= nxt then
                    result = result.."-"..tostring(nxt-1)
                 end
                 result = result..","
            end

            result  = result..to_string(port)
            last    = port
        end

        nxt = port + 1
    end

        if last ~= nil and last + 1 ~= nxt then
            result = result.."-"..tostring(nxt-1)
        end

    return result
end


-- ************************************************************************
---
--  devRangeStrGet
--        @description  getting of range string from dev/item array
--                      {[0] = {1, 2, 3, 4}} => 0/1-4
--
--        @param dev_ports      - device range
--
--        @return       device range string
--
function devRangeStrGet(dev_ports)
    local dev, ports
    local result = ""
    for dev, ports in pairs(dev_ports) do
        if 0 < #ports then
            if result ~= "" then
                result = result .. " "
            end
            result = result .. tostring(dev).."/"..itemsStrGet(ports)
        end
    end
    return result
end


-- ************************************************************************
---
--  interfaceInfoStrGet
--        @description  get CPSS_INTERFACE_INFO_STC string in user frendly
--                      form
--
--        @param interface_info - interface info structure
--
--        @return       CPSS_INTERFACE_INFO_STC string
--
function interfaceInfoStrGet(interface_info)
    local interface_info_type = interface_info['type']
    local ret

    if     "CPSS_INTERFACE_PORT_E"  == interface_info_type then
        local result, devNum, portNum =
            device_port_from_hardware_format_convert(
                interface_info['devPort']['devNum'],
                interface_info['devPort']['portNum'])

        if 0 == result  then
            ret = "Eth " .. tostring(devNum) .. "/" .. tostring(portNum)
        else
            ret = "n/a"
        end
    elseif "CPSS_INTERFACE_TRUNK_E" == interface_info_type  then
        ret = "Ch " .. tostring(interface_info['trunkId'])
    elseif "CPSS_INTERFACE_VIDX_E"  == interface_info_type  then
        ret = "Vidx " .. tostring(interface_info['vidx'])
    else
        ret = "n/a"
    end

    return ret
end


-- ************************************************************************
---
--  trunkL4LbhModeStringGet
--        @description  get CPSS_DXCH_TRUNK_L4_LBH_MODE_ENT string
--                      according to entry load balance
--
--        @param load_balance - entry load ballance
--
--        @return       if load ballance is  "src_dst_ip" then
--                          return "CPSS_DXCH_TRUNK_L4_LBH_DISABLED_E";
--                      if load ballance is  "src_dst_mac_ip" then
--                          return "CPSS_DXCH_TRUNK_L4_LBH_DISABLED_E";
--                      if load ballance is  "src_dst_mac_ip_port" then
--                          return "CPSS_DXCH_TRUNK_L4_LBH_LONG_E";
--                      otherwise
--                          return "CPSS_DXCH_TRUNK_L4_LBH_DISABLED_E"
--
function trunkL4LbhModeStringGet(load_balance)
    if     ("src-dst-ip"  == load_balance)          or
           ("src_dst_ip"  == load_balance)          then
        return "CPSS_DXCH_TRUNK_L4_LBH_DISABLED_E"
    elseif ("src-dst-mac-ip" == load_balance)       or
           ("src_dst_mac_ip" == load_balance)       then
        return "CPSS_DXCH_TRUNK_L4_LBH_DISABLED_E"
    elseif ("src-dst-mac-ip-port" == load_balance)  or
           ("src_dst_mac_ip_port" == load_balance)  then
        return "CPSS_DXCH_TRUNK_L4_LBH_LONG_E"
    else
        return "CPSS_DXCH_TRUNK_L4_LBH_DISABLED_E"
    end
end



-- ************************************************************************
---
--  get_full_representation_of_ipv6_addr
--        @description  - the function get an ipv6 address and return a
--                        full representation of the ipv6 address.
--                        meaning if you have the following address it will be translated as follows:
--                        ff:1234:05:0:789::abc --> 00ff:1234:0005:0000:0789:0000:0000:0abc
--
--        @param ipv6Addr     - ip v6 address
--
--        @return             - full representation of ipv6
--
function get_full_representation_of_ipv6_addr(ipv6Addr)
    local ncolumns, i
    ncolumns = 0
    for i = 0,string.len(ipv6Addr) do
        if string.sub(ipv6Addr,i,i) == ":" then
            ncolumns = ncolumns+1
        end
    end
    if ncolumns > 7 then
        return false, "too many columns in IPv6 address"
    end
    local ipv6Addr_c
    i = string.find(ipv6Addr, "::")
    if i ~= nil then
        ipv6Addr_c = string.sub(ipv6Addr,1,i) .. "0000"
        while ncolumns < 7 do
            ipv6Addr_c = ipv6Addr_c .. ":0000"
            ncolumns = ncolumns+1
        end
        ipv6Addr_c = ipv6Addr_c .. string.sub(ipv6Addr, i+1)
        if string.find(ipv6Addr_c, "::") ~= nil then
            return false, "Double columns must appear only once in IPv6 address"
        end
        if string.sub(ipv6Addr_c,1,1) == ":" then
            ipv6Addr_c = "0000" .. ipv6Addr_c
        end
        if string.sub(ipv6Addr_c,-1) == ":" then
            ipv6Addr_c = ipv6Addr_c .. "0000"
        end
    else
        if ncolumns < 7 then
            return false, "not enough columns in IPv6 address"
        end
        ipv6Addr_c = ipv6Addr
    end
    -- now go over all ipv6 address and check that each column element is of length 4
    local new_ipv6Addr_c = ipv6Addr_c
    -- if the length of the ipv6 Address is 39 then we have a full representation-->nothing to do
    if(string.len(ipv6Addr_c)~=39)then
        -- i is the location of ":"
        i = string.find(ipv6Addr_c, ":")
        local j=1 -- j is used to find the locations of ":", it will be located
                  -- in places 5, 10, 15, 20, 25, 30, 35 when you have a full representation
        local ipv6Addr_elem --an element taken form the ipv6 address,
                            --it can be from one character up to 4 characters
                            -- for example in ffff:2:55:001:0000::abcd
                            -- then ffff: , 2: , 55:, 001:, 0000:, abcd are all elements
        local ipv6Addr_elem_orig_size -- the original size of an element (this includes the ":") can be from 2 to 5
        while(i ~= nil) do -- as long as we have ":" in the ipv6 address representation we continue parsing
            ipv6Addr_elem = string.sub(new_ipv6Addr_c,(j*5-4),i)
            ipv6Addr_elem_orig_size = string.len(ipv6Addr_elem)
            if(string.len(ipv6Addr_elem)==2)then ipv6Addr_elem = "000"..ipv6Addr_elem
                else
                    if(string.len(ipv6Addr_elem)==3)then ipv6Addr_elem = "00"..ipv6Addr_elem
                        else
                            if(string.len(ipv6Addr_elem)==4)then ipv6Addr_elem = "0"..ipv6Addr_elem
                            end
                    end
            end
            if(ipv6Addr_elem_orig_size<5 and string.len(ipv6Addr_elem)==5)then
                new_ipv6Addr_c = string.sub(new_ipv6Addr_c,1,i-ipv6Addr_elem_orig_size)..ipv6Addr_elem..string.sub(new_ipv6Addr_c,(j*5-4)+ipv6Addr_elem_orig_size,-1)
            end
            i = string.find(new_ipv6Addr_c, ":",j*5+1) -- find next location of ":"
            j=j+1
        end
        -- if we still do not have a full representation this means we need to take care of last element in the ipv6 (that do not have ":" after it)
        if(string.len(ipv6Addr_c)~=39)then
            -- check last ipv6 element
            ipv6Addr_elem = string.sub(new_ipv6Addr_c,36)
            ipv6Addr_elem_orig_size = string.len(ipv6Addr_elem) -- the size can be from 1 to 3
            if(string.len(ipv6Addr_elem)==1)then
                ipv6Addr_elem = "000"..ipv6Addr_elem
            else
                if(string.len(ipv6Addr_elem)==2)then
                    ipv6Addr_elem = "00"..ipv6Addr_elem
                else
                    if(string.len(ipv6Addr_elem)==3)then
                        ipv6Addr_elem = "0"..ipv6Addr_elem
                    end
                end
            end
            if(ipv6Addr_elem_orig_size<4 and string.len(ipv6Addr_elem)==4)then
                new_ipv6Addr_c = string.sub(new_ipv6Addr_c,1,35)..ipv6Addr_elem
            end
        end
    end
    return new_ipv6Addr_c
end

-- ************************************************************************
---
--  get_sub_strings_according_to_delimiter
--        @description        - function returns an array of sub strings
--                              according to the delimiter parameter
--
--        @param string     - the string we want to parse
--        @param delimiter  - the delimiter to look for in the string
--
--        @return             - array of sub strings
--
function get_sub_strings_according_to_delimiter(string,delimiter)
  if delimiter == '' then return false end
  local pos, arr = 0, {}
  --for each delimiter found
  for st, sp in function() return string.find( string, delimiter, pos, true ) end do
    table.insert( arr, string.sub( string, pos, st - 1 ) ) --attach chars left of current delimiter
    pos = sp + 1 --jump past current delimiter
  end
  table.insert( arr, string.sub( string, pos ) ) -- Attach chars right of last delimiter
  return arr
end

-- ************************************************************************
---
--  calcIpv6AddressMasked
--        @description        - The function return a masked ipv6 according to the mask parameter
--                              Example:
--                               IP: 2001:4444:1234:5678::abcd
--                              Mask in: /48
--                              => Mask Wildcard: ffff:ffff:ffff:0000:0000:0000:0000:0000
--                              output :
--                              Masked IP: 2001:4444:1234:0000:0000:0000:0000:0000
--
--        @param ipv6Addr     - ip v6 address
--        @param mask         - ip v6 mask
--
--        @return             - the maksed ipv6 address
--
function calcIpv6AddressMasked(ipv6_addr, mask)
    -- get the full representation of an IP address (32 characters)
    if((mask~=nil)and(tonumber(mask,10)>128))then
        print("Error: ipv6 mask can not be bigger then 128")
        return nil
    end

    local full_ip_addr_rep = get_full_representation_of_ipv6_addr(ipv6_addr)
    if((mask==nil)or(tonumber(mask,10)==128))then
        return full_ip_addr_rep
    else
        -- mask the relevant bits and return the masked IP
        local ip_addr_masked_string
        local mask_representation
        --get ip elements
        local ip_elements = get_sub_strings_according_to_delimiter( full_ip_addr_rep, ':' )

        --generate wildcard from mask
        local number_of_full_chars = mask / 4
        local remainbits = mask % 4
        local wildcardbits = {}
        for i = 0, number_of_full_chars - 1 do
            table.insert( wildcardbits, 'f' )
        end
        if(remainbits~=0)then
            if(remainbits==1)then
                table.insert( wildcardbits, '8' )
            else
                if(remainbits==2)then
                    table.insert( wildcardbits, 'c' )
                else
                    if(remainbits==3)then
                        table.insert( wildcardbits, 'e' )
                    end
                end
            end
            number_of_full_chars=number_of_full_chars+1
        end
        for i = 0, 31 - number_of_full_chars do
            table.insert( wildcardbits, '0' )
        end

        --convert into 8 string array each w/ 4 chars
        local count, index, wildcard = 1, 1, {}
        for k, v in pairs( wildcardbits ) do
            if count > 4 then
                count = 1
                index = index + 1
            end
            if not wildcard[index] then wildcard[index] = '' end
            wildcard[index] = wildcard[index] .. v
            count = count + 1
        end

        --loop each character in each ip_elements group
        local bitVal
        local masked_ip = {}
        for k, v in pairs(ip_elements) do
            local masked_characters = ''
            for i = 1, 4 do
                local wild = wildcard[k]:sub( i, i )
                local character = v:sub( i, i )
                if wild == 'f' then
                    masked_characters = masked_characters .. character
                else
                    if(wild == 'e')then
                        bitVal = bit_and(tonumber(character,16),14)
                        masked_characters = masked_characters .. string.format("%x",bitVal)
                    else
                        if(wild == 'c')then
                            bitVal = bit_and(tonumber(character,16),12)
                            masked_characters = masked_characters .. string.format("%x",bitVal)
                        else
                            if(wild == '8')then
                                bitVal = bit_and(tonumber(character,16),8)
                                masked_characters = masked_characters .. string.format("%x",bitVal)
                            else
                                masked_characters = masked_characters .. '0'
                            end
                        end
                    end
                end
            end
            masked_ip[k] = masked_characters
        end

        ip_addr_masked_string = masked_ip[1] .. ':' .. masked_ip[2] .. ':' .. masked_ip[3] .. ':' .. masked_ip[4] .. ':' .. masked_ip[5] .. ':' .. masked_ip[6] .. ':' .. masked_ip[7] .. ':' .. masked_ip[8]
        mask_representation = wildcard[1] .. ':' .. wildcard[2] .. ':' .. wildcard[3] .. ':' .. wildcard[4] .. ':' .. wildcard[5] .. ':' .. wildcard[6] .. ':' .. wildcard[7] .. ':' .. wildcard[8]
        print("full_ip_addr_rep="..full_ip_addr_rep)
        print( 'Mask in: /' .. mask )
        print( 'Mask: ' .. mask_representation )
        print("ip_addr_masked_string="..ip_addr_masked_string)

        return ip_addr_masked_string
    end
end

-- ************************************************************************
---
--  stripHexDataSequence
--        @description        - removes 0x prefix or "" '' brackets from given string,
--                              checks it to contain only 0..9,a..f,A..F symbols.
--                              pads 0 at the end in amount of symbols is odd.
--                              returns result string
--                              if string contains other symbols - returns nil
--                              Example:
--                               0x12345 => 123450
--                               "5566"  => 5566
--                               '3'     => 30
--                               123q    => nil
--
--        @param data        - source string
--        @return            - stripped string or nil
--
function stripHexDataSequence(data)
    if (type(data) ~= "string") then
        return nil;
    end
    local res = data;
    -- strippings
    if (string.len(res) >= 3) then
        if (string.sub(res,1,1) == "\"")
            and (string.sub(res,-1,-1) == "\"") then
            res = string.sub(res,2,-2);
        elseif (string.sub(res,1,1) == "\'")
            and (string.sub(res,-1,-1) == "\'") then
            res = string.sub(res,2,-2);
        elseif (string.sub(res,1,2) == "0x") then
            res = string.sub(res,3,-1);
        end
    end
    -- padding
    if ((string.len(res) % 2) ~= 0) then
        res = res .. "0";
    end
    -- check
    if string.find(res, "[^0-9a-fA-F]") then
        return nil;
    end
    -- OK
    return res;
end

