require "common/generic/class"
require "common/generic/tables"

--constants
MAXIMUM_RESULT_ARRAY_SIZE = 400
ERROR_AND_WARNING_MESSAGES_COUNT  = 200

--------------------------------------------------------------------------------
--        construct's command execution data class
--------------------------------------------------------------------------------
Command_Data = class(
    function(object)
        object["status"]                        = true
        object["stop_execution"]                = true
        object["warning_array"]                 = {}
        object["error_array"]                   = {}
        object["device_status"]                 = true
        object["port_status"]                   = true
        object["vlan_status"]                   = true
        object["trunk_status"]                  = true
        object["entry_status"]                  = true
        object["vidx_status"]                   = true
        object["local_status"]                  = true
        object["devices_count"]                 = 0
        object["devices_with_success_count"]    = 0
        object["ports_count"]                   = 0
        object["ports_with_success_count"]      = 0
        object["vlans_count"]                   = 0
        object["vlans_with_success_count"]      = 0
        object["trunks_count"]                  = 0
        object["trunks_with_success_count"]     = 0
        object["entries_count"]                 = 0
        object["entries_with_success_count"]    = 0
        object["vidxes_count"]                  = 0
        object["vidxes_with_success_count"]     = 0
        object["items_count"]                   = 0
        object["items_with_success_count"]      = 0
        object["dev_range"]                     = nil
        object["dev_port_range"]                = nil
        object["dev_vlan_range"]                = nil
        object["result"]                        = nil
        object["result_array"]                  = nil
    end)


-- ************************************************************************
---
--  Command_Data:clearStatus
--        @description  clear's global status in command execution data
--                      object
--
--        @return       operation succed
--
function Command_Data:clearStatus()
    self["status"] = true

    return 0
end


-- ************************************************************************
---
--  Command_Data:clearDeviceStatus
--        @description  clear's device status in command execution data
--                      object
--
--        @return       operation succed
--
function Command_Data:clearDeviceStatus()
    self["device_status"] = true

    return 0
end


-- ************************************************************************
---
--  Command_Data:clearPortStatus
--        @description  clear's port status in command execution data
--                      object
--
--        @return       operation succed
--
function Command_Data:clearPortStatus()
    self["port_status"] = true

    return 0
end


-- ************************************************************************
---
--  Command_Data:clearVlanStatus
--        @description  clear's vlan status in command execution data
--                      object
--
--        @return       operation succed
--
function Command_Data:clearVlanStatus()
    self["vlan_status"] = true

    return 0
end


-- ************************************************************************
---
--  Command_Data:clearTrunkStatus
--        @description  clear's trunk status in command execution data
--                      object
--
--        @return       operation succed
--
function Command_Data:clearTrunkStatus()
    self["trunk_status"] = true

    return 0
end


-- ************************************************************************
---
--  Command_Data:clearEntryStatus
--        @description  clear's entry status in command execution data
--                      object
--
--        @return       operation succed
--
function Command_Data:clearEntryStatus()
    self["entry_status"] = true

    return 0
end


-- ************************************************************************
---
--  Command_Data:clearVidxStatus
--        @description  clear's vidx status in command execution data
--                      object
--
--        @return       operation succed
--
function Command_Data:clearVidxStatus()
    self["vidx_status"] = true

    return 0
end


-- ************************************************************************
---
--  Command_Data:clearItemStatus
--        @description  clear's item status in command execution data
--                      object
--
--        @return       operation succed
--
function Command_Data:clearItemStatus()
    self["item_status"] = true

    return 0
end


-- ************************************************************************
---
--  Command_Data:clearPriorutyStatus
--        @description  clear's priority status in command execution data
--                      object
--
--        @return       operation succed
--
function Command_Data:clearPriorutyStatus()
    self["priority_status"] = true

    return 0
end


-- ************************************************************************
---
--  Command_Data:clearLocalStatus
--        @description  clear's command execution status in command
--                      execution data object
--
--        @return       operation succed
--
function Command_Data:clearLocalStatus()
    self["local_status"] = true

    return 0
end


-- ************************************************************************
---
--  Command_Data:clearTrunksCount
--        @description  clear's trunks counts in command execution data
--                      object
--
--        @return       operation succed
--
function Command_Data:clearTrunksCount()
    self["trunks_count"]                 = 0
    self["trunks_with_success_count"]    = 0

    return 0
end


-- ************************************************************************
---
--  Command_Data:clearVlansCount
--        @description  clear's vlans counts in command execution data
--                      object
--
--        @return       operation succed
--
function Command_Data:clearVlansCount()
    self["vlans_count"]                 = 0
    self["vlans_with_success_count"]    = 0

    return 0
end


-- ************************************************************************
---
--  Command_Data:clearEntriesCount
--        @description  clear's entries counts in command execution data
--                      object
--
--        @return       operation succed
--
function Command_Data:clearEntriesCount()
    self["entries_count"]                 = 0
    self["entries_with_success_count"]    = 0

    return 0
end


-- ************************************************************************
---
--  Command_Data:clearVidxCount
--        @description  clear's vidx counts in command execution data
--                      object
--
--        @return       operation succed
--
function Command_Data:clearVidxCount()
    self["vidxes_count"]                  = 0
    self["vidxes_with_success_count"]     = 0

    return 0
end


-- ************************************************************************
---
--  Command_Data:clearPrioritiesCount
--        @description  clear's priorities counts in command execution data
--                      object
--
--        @return       operation succed
--
function Command_Data:clearPrioritiesCount()
    self["priorities_count"]                = 0
    self["priorities_with_success_count"]   = 0

    return 0
end


-- ************************************************************************
---
--  Command_Data:clearDeviceRange
--        @description  clear's device range in command execution data
--                      object
--
--        @return       operation succed
--
function Command_Data:clearDeviceRange()
    self["dev_range"] = {}

    return 0
end


-- ************************************************************************
---
--  Command_Data:clearDevRange
--        @description  clear's devt range in command execution data object
--
--        @return       operation succed
--
function Command_Data:clearDevPortRange()
    self["dev_port_range"] = {}

    return 0
end


-- ************************************************************************
---
--  Command_Data:clearDevTrunkRange
--        @description  clear's device/trunk range in command execution
--                      datao bject
--
--        @return       operation succed
--
function Command_Data:clearDevTrunkRange()
    self["dev_trunk_range"] = {}

    return 0
end


-- ************************************************************************
---
--  Command_Data:clearResultArray
--        @description  clear's result array in command execution data
--                      object
--
--        @return       operation succed
--
function Command_Data:clearResultArray()
    self["result_array"] = {}

    return 0
end

if dxCh_family == true then
    require_safe_dx("generic/command_patterns")
end

if px_family == true then
    require_safe_px("generic/command_patterns")
end

-- ************************************************************************
---
--  Command_Data:setFailStatus
--        @description  set's fail command execution status in command
--                      execution data object
--
--        @return       operation succed
--
function Command_Data:setFailStatus()
    self["status"] = false

    return 0
end


-- ************************************************************************
---
--  Command_Data:setFailStatusOnCondition
--        @description  set's fail command execution status in command
--                      execution data object according to condition
--
--        @param cond           - condition
--
--        @return       operation succed
--
function Command_Data:setFailStatusOnCondition(cond)
    if true == cond     then
        self["status"] = false
    end

    return 0
end


-- ************************************************************************
---
--  Command_Data:setFailDeviceStatus
--        @description  set's fail device processing status in command
--                      execution data object
--
--        @return       operation succed
--
function Command_Data:setFailDeviceStatus()
    if self["dontStateErrorOnCpssFail"] then
        -- ignore registration of the error
        return
    end

    self["device_status"] = false

    return 0
end


-- ************************************************************************
---
--  Command_Data:setFailDeviceStatusOnCondition
--        @description  set's fail device processing status in command
--                      execution data object according to condition
--
--        @param cond           - condition
--
--        @return       operation succed
--
function Command_Data:setFailDeviceStatusOnCondition(cond)
    if true == cond     then
        self["device_status"] = false
    end

    return 0
end


-- ************************************************************************
---
--  ommand_Data:setFailPortStatus
--        @description  set's fail port processing status in command
--                      execution data object
--
--        @return       operation succed
--
function Command_Data:setFailPortStatus()
    self["port_status"] = false

    return 0
end


-- ************************************************************************
---
--  ommand_Data:setFailPortStatusOnCondition
--        @description  set's fail port processing status in command
--                      execution data object according to condition
--
--        @param cond           - condition
--
--        @return       operation succed
--
function Command_Data:setFailPortStatusOnCondition(cond)
    if true == cond then
        self["port_status"] = false
    end

    return 0
end


-- ************************************************************************
---
--  Command_Data:setFailVlanStatus
--        @description  set's fail vlan processing status in command
--                      execution data object
--
--        @return       operation succed
--
function Command_Data:setFailVlanStatus()
    self["vlan_status"] = false

    return 0
end


-- ************************************************************************
---
--  Command_Data:setFailVlanStatusOnCondition
--        @description  set's fail vlan processing status in command
--                      execution data object according to condition
--
--        @param cond           - condition
--
--        @return       operation succed
--
function Command_Data:setFailVlanStatusOnCondition(cond)
    if true == cond then
        self["vlan_status"] = false
    end

    return 0
end


-- ************************************************************************
---
--  Command_Data:setFailTrunkStatus
--        @description  set's fail trunk processing status in command
--                      execution data object
--
--        @return       operation succed
--
function Command_Data:setFailTrunkStatus()
    self["trunk_status"] = false

    return 0
end


-- ************************************************************************
---
--  Command_Data:setFailEntryStatus
--        @description  set's fail entry processing status in command
--                      execution data object
--
--        @return       operation succed
--
function Command_Data:setFailEntryStatus()
    self["entry_status"] = false

    return 0
end


-- ************************************************************************
---
--  Command_Data:setFailVidxStatus
--        @description  set's fail vidx processing status in command
--                      execution data object
--
--        @return       operation succed
--
function Command_Data:setFailVidxStatus()
    self["vidx_status"] = false

    return 0
end


-- ************************************************************************
---
--  Command_Data:setFailLocalStatus
--        @description  set's fail local code block status in command
--                      execution data object
--
--        @return       operation succed
--
function Command_Data:setFailLocalStatus()
    self["local_status"] = false

    return 0
end


-- ************************************************************************
---
--  Command_Data:setFailLocalStatusOnCondition
--        @description  set's fail local code block status in command
--                      execution data object according to condition
--
--        @param cond           - condition
--
--        @return       operation succed
--
function Command_Data:setFailLocalStatusOnCondition(cond)
    if true == cond         then
        self["local_status"] = false
    end

    return 0
end


-- ************************************************************************
---
--  Command_Data:setFailGeneralAndLocalStatus
--        @description  set's fail command execution status and its local
--                      status in command
--                      execution data object
--
--        @return       operation succed
--
function Command_Data:setFailGeneralAndLocalStatus()
    self["status"] = false
    self["local_status"] = false

    return 0
end


-- ************************************************************************
---
--  Command_Data:setFailDeviceAndLocalStatus
--        @description  set's fail device processing and local code block
--                      status in command execution data object
--
--        @return       operation succed
--
function Command_Data:setFailDeviceAndLocalStatus()
    if self["dontStateErrorOnCpssFail"] then
        -- ignore registration of the error
        return
    end
    self["device_status"] = false
    self["local_status"] = false

    return 0
end

-- ************************************************************************
---
--  Command_Data:addWarning
--        @description  add's new warning message to command warning array
--                      in command execution data object
--
--        @param formatString   - The warning message
--        @param ...            - The warning message parameters
--
--        @return       operation succed
--
function Command_Data:addWarning(formatString, ...)
    if self["dontStateErrorOnCpssFail"] then
        -- ignore registration of the error
        return
    end
    if ERROR_AND_WARNING_MESSAGES_COUNT >= #self["warning_array"] then
        tableAppend(self["warning_array"], string.format(formatString, ...))
    end

    return 0
end

-- ************************************************************************
---
--  Command_Data:addError
--        @description  add's new error message to command error array in
--                      command execution data object
--
--        @param formatString   - The error message
--
--        @return       operation succed
--
function Command_Data:addError(formatString, ...)
    if self["dontStateErrorOnCpssFail"] then
        -- ignore registration of the error
        return
    end
    if ERROR_AND_WARNING_MESSAGES_COUNT >= #self["error_array"] then
        --printCallStack(5)
        tableAppend(self["error_array"], string.format(formatString, ...))
    end

    return 0
end
-- ************************************************************************
---
--  Command_Data:addConditionalWarning
--        @description  add's new warning message to command warning array
--                      in command execution data object if condition is
--                      true and object local status is ok
--
--        @param cond           - Boolean condition
--        @param formatString   - The warning message
--        @param ...            - The warning message parameters
--
--        @return       operation succed
--
function Command_Data:addConditionalWarning(cond, formatString, ...)
    if true == cond then
        self:addWarning(formatString, ...)
    end

    return 0
end


-- ************************************************************************
---
--  Command_Data:addWarningIfFalseCondition
--        @description  add's new warning message to command warning array
--                      in command execution data object if condition is
--                      not true
--
--        @param cond           - Boolean condition
--        @param formatString   - The warning message
--        @param ...            - The warning message parameters
--
--        @return       operation succed
--
function Command_Data:addWarningIfFalseCondition(cond, formatString, ...)
    if true ~= cond     then
        self:addWarning(formatString, ...)
    end

    return 0
end


-- ************************************************************************
---
--  Command_Data:addWarningOnceOnKey
--        @description  add's new warning message to command warning array
--                      in command execution data object
--
--        @param formatString   - The warning message
--        @param ...            - The warning message parameters
--
--        @return       operation succed
--
function Command_Data:addWarningOnceOnKey(key, formatString, ...)
    if false == isItemInTable(key, self["warnings_keys"]) then
        self["warnings_keys"] = tableInsert(self["warnings_keys"], key)
        return self:addWarning(formatString, ...)
    end

    return 0
end


-- ************************************************************************
---
--  Command_Data:addWarningIfFailedStatus
--        @description  add's new warning message to command warning array
--                      in command execution data object, if its status is
--                      false
--
--        @param formatString   - The warning message
--        @param ...            - The warning message parameters
--
--        @return       operation succed
--
function Command_Data:addWarningIfFailedStatus(formatString, ...)
    if false == self["status"] then
        self:addWarning(formatString, ...)
    end

    return 0
end


-- ************************************************************************
---
--  Command_Data:addWarningIfNoSuccessDevices
--        @description  add's new warning message to command warning array
--                      in command execution data object, if there is no
--                      successfully processed devices
--
--        @param formatString   - The warning message
--        @param ...            - The warning message parameters
--
--        @return       operation succed
--
function Command_Data:addWarningIfNoSuccessDevices(formatString, ...)
    if 0 == self["devices_with_success_count"] then
        self:addWarning(formatString, ...)
    end

    return 0
end


-- ************************************************************************
---
--  Command_Data:addWarningIfNoSuccessPorts
--        @description  add's new warning message to command warning array
--                      in command execution data object, if there is no
--                      successfully processed ports
--        @param formatString   - The warning message
--        @param ...            - The warning message parameters
--
--        @return       operation succed
--
function Command_Data:addWarningIfNoSuccessPorts(formatString, ...)
    if 0 == self["ports_with_success_count"] then
        self:addWarning(formatString, ...)
    end

    return 0
end

-- ************************************************************************
---
--  Command_Data:getDevPortRangeBitmap
--        @description  get's port bitmap related to device form dev/port
--                      range of command execution data object applied to
--                      given device
--
--        @param devId              - applied device number
--
--        @return       port-range
--
function Command_Data:getDevPortRangeBitmap(devId)
    local index, portNum
    local portBmp   = {["ports"] = {[0] = 0, 0, 0, 0, 0, 0, 0}}

    if "table" == type(self["dev_port_range"][devId])   then
        for index, portNum in pairs(self["dev_port_range"][devId]) do
            portBmp = add_port_to_port_bmp(portBmp, portNum)
        end
    end

    return portBmp
end

-- ************************************************************************
---
--  Command_Data:getInterfacePortIterator
--        @description  iterating over the dev/port pairs in ifRange
--                      for iterator, devNum, portNum in
--                                  command_data:getInterfacePortIterator()
--                      do
--                          print(devNum, portNum)
--                      end
--
--        @usage __global       - __global["ifRange"]: iterface range
--                                dev/port table
--
--        @return       interator over the dev/port pairs
--
 function Command_Data:getInterfacePortIterator()
    self:initInterfaceRangeIterator()

    return self:getPortIterator()
end

-- ************************************************************************
---
--  Command_Data:isOnePortInPortRange
--        @description  checks if there is one port in range
--
--        @return       true if there is one port, otherwise false
--
function Command_Data:isOnePortInPortRange()
    if "table" == type(self["dev_port_range"]) then
        local count = 0
        local dev, ports, port

        for dev, ports in pairs(self["dev_port_range"]) do
            for port = 1, #ports do
                if does_port_exist(dev,ports[port]) then
                    count = count + 1
                end

                if 1 < count then
                    break
                end
            end

            if 1 < count then
                break
            end
        end

        return 1 == count
    else
        return false
    end
end

-- ************************************************************************
---
--  Command_Data:getFirstPort
--        @description  getting of first valid port in the dev/port pairs
--                      in self["dev_port_range"] list
--
--        @return       dev/port pair
--
function Command_Data:getFirstPort()
    local dev, ports, index, port

    for dev, ports in pairs(self["dev_port_range"]) do
        for index, port in pairs(ports) do
            if does_port_exist(dev, port) then
                return dev, port
            end
        end
    end

    return nil, nil
end


-- ************************************************************************
---
--  Command_Data:getTrunksIterator
--        @description  iterating over the dev/trunks pairs in command
--                      execution data object
--                      command_data:initDevTrunkWithPortsRange()
--                      for iterator, devNum, trunkId in
--                                          command_data:getTrunksIterator()
--                      do
--                          print(devNum, trunkId)
--                      end
--
--        @return       interator over the dev/trunk pairs
--
function Command_Data:getTrunksIterator()
    function iterator(v, i)
        local dev, trunks
        if nil == i then i = 0 end
        if true ~= self["stop_execution"] then  return nil, nil, nil end

        i = i + 1
        v = i

        for dev, trunks in pairs(self["dev_trunk_range"]) do
            if v > #trunks then
                v = v - #trunks
            else
                return i, dev, trunks[v]
            end
        end
        return nil, nil, nil
    end

    return iterator
end


-- ************************************************************************
---
--  Command_Data:getInterfaceDevPortVlanIterator
--        @description  iterating over the dev/port/vlanId triples
--                      accorging to merged dev/port range and vlan range
--                      of command execution data object
--                      command_data:initInterfaceDevPortRange()
--                      command_data:initVlanRange()
--                      for iterator, devNum, portNum, vlanId in
--                          command_data:getInterfaceDevPortVlanIterator()
--                      do
--                          print(devNum, portNum, vlanId)
--                      end
--
--        @return       interator over the dev/port/vlanId triples
--
function Command_Data:getInterfaceDevPortVlanIterator()
    function iterator(v, i)
        local dev, ports
        local vlans, vi
        if nil == i then i = 0 end
        if true ~= self["stop_execution"] then  return nil, nil, nil end

        i = i + 1

        vlans = self["vlan_range"]

        v = i / #vlans
        vi = i % #vlans + 1

        for dev, ports in pairs(self["dev_port_range"]) do
            if v > #ports then
                v = v - #ports
            else
                if does_port_exist(dev,ports[v]) then
                    if does_vlan_exists(dev, vlans[vi]) then
                        return i, dev, ports[v], vlans[vi]
                    else
                        if i < #vlans then
                            self:addNotExistendPort(dev, ports[v])
                        end

                        i = i + 1
                        vi = vi + 1

                        while vi <= #vlans do
                            if does_vlan_exists(dev, vlans[vi]) then
                                return i, dev, ports[v], vlans[vi]
                            elseif i < #vlans then
                                self:addNotExistendVlan(dev, vlans[v])
                            end
                            i = i + 1
                            vi = vi + 1
                        end
                        vi = 0
                    end
                else
                    i = i + #vlans
                    self:addNotExistendPort(dev, ports[v])
                end
                v = v + 1

                while v <= #ports do
                    if does_port_exist(dev,ports[v]) then
                        if does_vlan_exists(dev, vlans[vi]) then
                            return i, dev, ports[v], vlans[vi]
                        else
                            if i < #vlans then
                                self:addNotExistendPort(dev, ports[v])
                            end

                            i = i + 1
                            vi = vi + 1

                            while vi <= #self["vlan_range"] do
                                if does_vlan_exists(dev, vlans[vi]) then
                                    return i, dev, ports[v], vlans[vi]
                                elseif i < #vlans then
                                    self:addNotExistendVlan(dev, vlans[v])
                                end
                                i = i + 1
                                vi = vi + 1
                            end
                            vi = 0
                        end
                    else
                        i = i + #vlans
                        self:addNotExistendPort(dev, ports[v])
                    end
                    v = v + 1
                end
                v = v - #ports
            end
        end
        return nil, nil, nil
    end

    return iterator
end


-- ************************************************************************
---
--  Command_Data:getPortChannelPortIterator
--        @description  iterating over the dev/port pairs in current port
--                      channel
--                      for iterator, devNum, portNum in
--                              command_data:getPortChannelPortIterator()
--                      do
--                          print(devNum, portNum)
--                      end
--
--        @return       interator over the dev/port pairs
--
function Command_Data:getPortChannelPortIterator()
    function iterator(v, i)
        -- to do later
        return nil, nil, nil
    end

    return iterator
end


-- ************************************************************************
---
--  Command_Data:analyzeCommandExecution
--        @description  analyzes result of command execution according to
--                      command execution data object
--
function Command_Data:analyzeCommandExecution()
    if 0 < #self["warning_array"] then
        self["warning_string"] = tbltostr(self["warning_array"] , '\n')
    end

    if false == self["status"] then
        self["error_string"] = tbltostr(self["error_array"], '\n')
    end
end

-- ************************************************************************
---
--  Command_Data:printCommandExecutionResults
--        @description  print's result of command execution according to
--                      command execution data object
--
function Command_Data:printCommandExecutionResults()
    if nil ~= self["result"] then
        if "string" == type(self["result"]) then
            print(self["result"])
        elseif "table" == type(self["result"]) then
            dumpTableToConsole(self["result"])
        end
    end

    if false == self["status"] then
        if ('\n' ~= self["error_string"][#self["error_string"]]) and
           (nil == self["warning_string"]) then
            self["error_string"] = self["error_string"] .. '\n'
        end

        print(self["error_string"])
    end

    if nil ~= self["warning_string"] then
        if '\n' ~= self["warning_string"][#self["warning_string"]] then
            self["warning_string"] = self["warning_string"] .. '\n'
        end

        print(self["warning_string"])
    end

    self:disablePausedPrinting()
end

-- ************************************************************************
---
--  Command_Data:getCommandExecutionResults
--        @description  return's result of command execution
--
--        @return       status and error message, if error occures
--
function Command_Data:getCommandExecutionResults()
    return self["status"], self["error_string"]
end

-- ************************************************************************
---
--  Command_Data:printAndFreeResultArray
--        @description  show's and release result array of command
--                      execution data object at chash situation
--
function Command_Data:printAndFreeResultArray()
    if nil ~= self["emergency_header"]  then
        print(deleteLastNewLine(tostring(self["emergency_header"])))
    end

    dumpTableToConsole(self["result_array"])

    if nil ~= self["emergency_footer"]  then
        print(deleteLastNewLine(tostring(self["emergency_footer"])))
    end

    self["result_array"] = nil

    self["result_array"] = {}
end

-- ************************************************************************
---
--  Command_Data:setFailDeviceAndLocalStatusOnCondition
--        @description  set's fail device processing and local code block
--                      status in command execution data object according
--                      to condition
--
--        @param cond           - condition
--
--        @return       operation succed
--
function Command_Data:setFailDeviceAndLocalStatusOnCondition(cond)
    if true == cond         then
        self["device_status"] = false
        self["local_status"] = false
    end

    return 0
end


-- ************************************************************************
---
--  Command_Data:setFailDeviceAndLocalStatusIfFalseCondition
--        @description  set's fail device processing and local code block
--                      status in command execution data object if
--                      condition is not true
--
--        @param cond           - condition
--
--        @return       operation succed
--
function Command_Data:setFailDeviceAndLocalStatusIfFalseCondition(cond)
    if true ~= cond         then
        self["device_status"] = false
        self["local_status"] = false
    end

    return 0
end


-- ************************************************************************
---
--  Command_Data:setFailPortAndLocalStatus
--        @description  set's fail port processing and local code block
--                      status in command execution data object
--
--        @return       operation succed
--
function Command_Data:setFailPortAndLocalStatus()
    self["port_status"] = false
    self["local_status"] = false

    return 0
end


-- ************************************************************************
---
--  Command_Data:setFailVlanAndLocalStatus
--        @description  set's fail vlan processing and local code block
--                      execution status in command execution data object
--
--        @return       operation succed
--
function Command_Data:setFailVlanAndLocalStatus()
    self["vlan_status"] = false
    self["local_status"] = false

    return 0
end


-- ************************************************************************
---
--  Command_Data:setFailTrunkAndLocalStatus
--        @description  set's fail trunk processing and local code block
--                      execution status in command execution data object
--
--        @return       operation succed
--
function Command_Data:setFailTrunkAndLocalStatus()
    self["trunk_status"] = false
    self["local_status"] = false

    return 0
end


-- ************************************************************************
---
--  Command_Data:setFailEntryAndLocalStatus
--        @description  set's fail entry processing and local code block
--                      status in command execution data object
--
--        @return       operation succed
--
function Command_Data:setFailEntryAndLocalStatus()
    self["entry_status"] = false
    self["local_status"] = false

    return 0
end

-- ************************************************************************
---
--  Command_Data:setFailVidxAndLocalStatus
--        @description  set's fail vidx processing and local code block
--                      status in command execution data object
--
--        @return       operation succed
--
function Command_Data:setFailVidxAndLocalStatus()
    self["vidx_status"] = false
    self["local_status"] = false

    return 0
end


-- ************************************************************************
---
--  Command_Data:setEmergencyPrintingHeaderAndFooter
--        @description  setting of emergency printing header and footer
--                      value
--
--        @param header         - emergency header
--        @param footer         - emergency footer
--
function Command_Data:setEmergencyPrintingHeaderAndFooter(header, footer)
    self["emergency_header"]   = header
    self["emergency_footer"]   = footer
end


-- ************************************************************************
---
--  Command_Data:enablePausedPrinting
--        @description  enables paused printing with message
--                      "Type <CR> to continue, Q<CR> to stop:"
--
function Command_Data:enablePausedPrinting()
    reset_paused_printing()
    set_breaking_function(stopCommandExecution, self)
    push_and_replace_print_function()
end


-- ************************************************************************
---
--  Command_Data:disablePausedPrinting
--        @description  disanles paused printing with message
--                      "Type <CR> to continue, Q<CR> to stop:"
--
function Command_Data:disablePausedPrinting()
    if true == is_paused_printing_activated()   then
        restore_and_pop_print_function()
    end
end


-- ************************************************************************
---
--  Command_Data:setDevRange
--        @description  initialize dev range to given range
--
--        @param range          - given range
--
--        @return       operation succed
--
function Command_Data:setDevRange(range)
    self["dev_range"] = range

    return 0
end


-- ************************************************************************
---
--  Command_Data:setAllDevRange
--        @description  initialize dev range to all avaible devices
--
--        @param range          - given range
--
--        @return       operation succed
--
function Command_Data:setAllDevRange(range)
    self["dev_range"] = wrlDevList()

    return 0
end

-- ************************************************************************
---
--  Command_Data:getDevicePortRange
--        @description  get's of device port range of command execution
--                      data object
--
--        @return       device port range
--
function Command_Data:getDevicePortRange()
    return self["dev_port_range"]
end

-- ************************************************************************
---
--  Command_Data:setDevPortRange
--        @description  setting of the dev/ports pairs to given range
--
--        @param range         - given range
--
--        @return       operation succeeded
--
function Command_Data:setDevPortRange(range)
    self["dev_port_range"] = range
    return 0
end

-- ************************************************************************
---
--  Command_Data:addNotExistendPort
--        @description  processes not existend port
--
--        @param devId          - device number
--        @param portNum        - port number
--
--        @return       operation succed
--
function Command_Data:addNotExistendPort(devId, portNum)
    self:addWarning("Port %d of device %d does not exist.", portNum, devId)

    return 0
end

-- ************************************************************************
---
--  Command_Data:iterateOverDevices
--        @description  iterating over devices in self["dev_range"]
--                      handle device and local status
--
--                      command_data:initAllDeviceRange(params) or
--                      command_data:initInterfaceDeviceRange() or
--                      command_data:initAllAvailableDevicesRange()
--                      -- now iterate
--                      command_data:iterateOverDevices(
--                          function(command_data, devNum, message)
--                              print(message, devNum)
--                          end,
--                          "hello")
--
--        @return       interator over all available devices
--
function Command_Data:iterateOverDevices(func, ...)
    -- Common variables initialization.
    local iterator,devNum

    for iterator, devNum in  self:getDevicesIterator() do
        self:clearDeviceStatus()
        self:clearLocalStatus()

        if true == self["local_status"] then
            func(self, devNum, ...)
        end

        self:updateStatus()
        self:updateDevices()
    end
end
-- ************************************************************************
---
--  Command_Data:initInterfaceRange
--        @description  alias to initInterfaceRange
--
function Command_Data:initInterfaceRangeIterator()
    return self:initInterfaceDevPortRange()
end

-- ************************************************************************
---
--  Command_Data:iterateOverPorts
--        @description  iterating over devices in self["dev_range"]
--                      handle port and local status
--
--                      command_data:initInterfaceRangeIterator() or
--                      command_data:initInterfaceDevPortRange()
--                      -- now iterate
--                      command_data:iterateOverPorts(
--                          function(command_data, devNum, portNum, message)
--                              print(message, devNum, portNum)
--                          end,
--                          "hello")
--
--        @return       interator over all available devices
--
function Command_Data:iterateOverPorts(func, ...)
    -- Common variables initialization.
    local iterator,devNum,portNum

    for iterator, devNum, portNum in  self:getPortIterator() do
        self:clearPortStatus()
        self:clearLocalStatus()

        if true == self["local_status"] then
            func(self, devNum, portNum, ...)
        end

        self:updateStatus()
        self:updatePorts()
    end
end

-- ************************************************************************
---
--  Command_Data:getPortIterator
--        @description  iterating over the dev/port pairs in
--                      self["dev_port_range"] list
--                      command_data:initInterfaceRangeIterator() or
--                      command_data:initInterfaceDevPortRange()
--                      for iterator, devNum, portNum in
--                                          command_data:getPortIterator()
--                      do
--                          print(devNum, portNum)
--                      end
--
--        @return       interator over the dev/port pairs
--
function Command_Data:getPortIterator()
    function iterator(v, i)
        local dev, ports
        if nil == i then i = 0 end
        if true ~= self["stop_execution"] then  return nil, nil, nil end

        i = i + 1
        v = i

        for dev, ports in pairs(self["dev_port_range"]) do
            if v > #ports then
                v = v - #ports
            else
                if does_port_exist(dev,ports[v]) then
                    self["current_dev_num"] = dev
                    self["current_port_num"] = ports[v]
                    return i, dev, ports[v]
                else
                    self:addNotExistendPort(dev, ports[v])
                end
                v = v + 1
                i = i + 1
                while v <= #ports do
                    if does_port_exist(dev,ports[v]) then
                        self["current_dev_num"] = dev
                        self["current_port_num"] = ports[v]
                        return i, dev, ports[v]
                    else
                        self:addNotExistendPort(dev, ports[v])
                    end
                    v = v + 1
                    i = i + 1
                end
                v = v - #ports
            end
        end
        return nil, nil, nil
    end

    return iterator
end

-- ************************************************************************
---
--  Command_Data:setPortRangeIterator
--        @description  initialize iterator over the dev/port pairs to go
--                      through given port range
--
--        @param trunkId        - The trunk Id
--
--        @return       operation succed or operation error code and error
--                      message
--
function Command_Data:setPortRangeIterator(trunkId)
    return self:setPortChannelDevPortRange(trunkId)
end

-- ************************************************************************
---
--  Command_Data:setPortChannelDevPortRange
--        @description  setting of the dev/ports range of command execution
--                      data object according to dev/port range of given
--                      port-channel
--
--        @param trunkId       - given trunk id
--        @param dev_range     - device range
--
--        @return       operation succeeded
--
function Command_Data:setPortChannelDevPortRange(trunkId, dev_range)
    local result, values, error_message
    local all_ports = {}

    result = 0

    if nil == dev_range then
        dev_range = wrlDevList()
    end

    if 0 == result then
        result, values = get_trunk_device_port_list(trunkId, dev_range)
        if 0 == result then
            all_ports = values
        else
            error_message = values
        end
    end

    if 0 == result then
        self["dev_port_range"] = all_ports
    end

    return result, error_message
end
-- ************************************************************************
---
--  Command_Data:getAllAvailableDevicesIterator
--        @description  iterating over all available devices
--                      for iterator, devNum in
--                          command_data:getAllAvailableDevicesIterator()
--                      do
--                          print(devNum)
--                      end
--
--        @return       interator over all available devices
--
function Command_Data:getAllAvailableDevicesIterator()
    self:setAllDevRange()

    return self:getDevicesIterator()
end

-- ************************************************************************
---
--  Command_Data:initAllInterfacesPortIterator
--        @description  sets port iterator according to all_interfaces
--                      params
--
--        @param params         - params["all"]: all devices port or all
--                                ports of given device cheking switch,
--                                could be irrelevant;
--                                params["devID"]: checked device number,
--                                relevant if params["all"] is not nil;
--                                params["ethernet"]: checked interface
--                                name, relevant if params["all"] is not
--                                nil;
--                                params["port-channel"]: checked interface
--                                name, relevant if params["all"] is not
--                                nil
--        @param alternative    - alternative interface behavior checking
--                                flag
--
--        @return       true is success or alternative != true otherwise
--                      false
--
function Command_Data:initAllInterfacesPortIterator(params, alternative)
    local result, error_message
    local no_need_alternative
    local allDevsInSystem = (nil ~= params["all"]) or ("all" == params["devID"]) or ("all" == params["all_device"])

    if allDevsInSystem then
        -- all ports on all devices
        result, error_message = self:setSystemIterator()
        no_need_alternative   = true
    elseif nil ~= params["devID"] then
        -- all ports on specific device
        result, error_message = self:setDeviceIterator(params["devID"])
        no_need_alternative   = true
    elseif nil ~= params["ethernet"] then
        result, error_message     = self:setRangeIterator(params["ethernet"])
        no_need_alternative       = true
    elseif nil ~= params["port-channel"] then
        result, error_message     = self:setPortRangeIterator(params["port-channel"])
        no_need_alternative       = true
    elseif true == alternative then
        result                = 0
        no_need_alternative   = false
    else
        result                = 1
        error_message         = "Such kind of interfaces is not supported."
        no_need_alternative   = true
    end

    if 0 ~= result then
        self:setFailStatus()
        self:addError(error_message)
    end

    return no_need_alternative
end

-- ************************************************************************
---
--  Command_Data:addToResultStrOnPortCount
--        @description  add's to result string ports count more than 0
--
--        @param ...            - Added strings
--
--        @return       operation succed
--
function Command_Data:addToResultStrOnPortCount(...)
    if 0 < self["ports_count"] then
        local items = {...}
        local index, item

        if "table" ~= type(self["result"])  then
            for index, item in ipairs(items) do
                self["result"] = self["result"] .. item
            end
        else
            for index, item in ipairs(items) do
                if "table" ~= type(item)    then
                    tableAppend(self["result"], item)
                else
                    tableAppendToTable(self["result"], item)
                end
            end
        end
    end

    return 0
end

-- ************************************************************************
---
--  functions
--        @description  initialize iterator over the dev/port pairs to go
--                      through all ports of given device
--
--        @param devId          - device
--
--        @return       operation succed
--
function Command_Data:setDeviceIterator(devId)
    self:clearDevPortRange()
    self["dev_port_range"][devId] = get_port_list(devId)

    return 0
end


-- ************************************************************************
---
--  Command_Data:setSystemIterator
--        @description  initialize iterator over the dev/port pairs to go
--                      through all ports of all devices
--
--        @param all_devices    - device range
--
--        @return       operation succed
--
function Command_Data:setSystemIterator(all_devices)
    local devIndex, devId
    local i

    if nil == all_devices   then
        all_devices = wrlDevList()
    end

    self:clearDevPortRange()
    for devIndex, devId in pairs(all_devices) do
        self["dev_port_range"][devId] = get_port_list(devId)
    end

    return 0
end



-- ************************************************************************
---
--  Command_Data:initAllAvaiblesPortIterator
--        @description  sets iterator over all avaible ports
--                      Requires:
--                          initAllAvailableDevicesRange()
--
--        @usage self           - self["dev_range"]: device range
--
--        @return       operation successed
--
function Command_Data:initAllAvaiblesPortIterator()
    self:setSystemIterator(self["dev_range"])

    return 0
end

-- ************************************************************************
---
--  Command_Data:initAllDeviceRange
--        @description  sets device range in command execution data object
--                      according to all_deveice params
--
--        @param params         - params["all_device"]: all or given
--                                devices iterating property
--        @param alternative    - alternative interface behavior
--                                checking flag
--
--        @return       true is success or alternative != true otherwise
--                      false
--
function Command_Data:initAllDeviceRange(params, alternative)
    local result, values, error_message
    local all_devices = wrlDevList()
    local no_need_alternative = true

    if   "all" == params["all_device"] then
        result = self:setAllDevRange()
    elseif nil ~= params["all_device"] then
        result = self:setDevRange({params["all_device"]})
    elseif true == alternative then
        result                = 0
        no_need_alternative   = false
    else
        result                = 1
        error_message         = "Such kind of device range is not supported."
        no_need_alternative   = true
    end

    return no_need_alternative
end


-- ************************************************************************
---
--  Command_Data:initAllAvailableDevicesRange
--        @description  sets device range in command execution data object
--                      according to all avaible devices
--
--
--        @return       0 on success, otherwise error code and error
--                      message
--
function Command_Data:initAllAvailableDevicesRange()
    local result

    result = self:setAllDevRange()

    return result
end

-- ************************************************************************
---
--  Command_Data:addResultToResultArray
--        @description  add's result of command execution data object to
--                      result array in command execution data object
--
--        @return       operation succed
--
function Command_Data:addResultToResultArray()
    self:addToResultArray(self["result"])

    return 0
end


-- ************************************************************************
---
--  Command_Data:addResultToResultArrayOnCondition
--        @description  add's result of command execution data object to
--                      result array in command execution data object on
--                      condition
--
--        @param cond           - condition
--
--        @return       operation succed
--
function Command_Data:addResultToResultArrayOnCondition(cond)
    if true == cond     then
        self:addResultToResultArray()
    end

    return 0
end

-- ************************************************************************
---
--  Command_Data:addToResultArray
--        @description  add's new items to result array in command
--                      execution data object
--
--        @param ...            - The warning message
--
--        @return       operation succed
--
function Command_Data:addToResultArray(...)
    local items = {...}
    local index, item

    for index, item in ipairs(items) do
        tableAppend(self["result_array"], item)
    end

    self:printAndFreeResultArrayIfItsSizeOverflow()

    return 0
end

-- ************************************************************************
---
--  Command_Data:printAndFreeResultArrayIfItsSizeOverflow
--        @description  show's and release result array of command
--                      execution data object if its member count more than
--                      its limit
--
function Command_Data:printAndFreeResultArrayIfItsSizeOverflow()
    if MAXIMUM_RESULT_ARRAY_SIZE < #self["result_array"] then
        self:printAndFreeResultArray()
    end
end

-- ************************************************************************
---
--  Command_Data:updateEntries
--        @description  updates entries count statistic in command
--                      execution data object
--
function Command_Data:updateEntries()
    self["entries_count"] = self["entries_count"] + 1
    if true == self["entry_status"] then
        self["entries_with_success_count"] =
            self["entries_with_success_count"] + 1
    end
end

-- ************************************************************************
---
--  Command_Data:setResultStr
--        @description  sets result in command execution data object to
--                      concatenation of string header, body and footer, if
--                      it is relevant
--
--        @return       operation succed
--
function Command_Data:setResultStr(header, body, footer)
    body    = concatHeaderBodyAndFooter(header, body, footer)

    self["result"] = body
end

 -- ************************************************************************
---
--  Command_Data:setResultStrOnPortCount
--        @description  sets result in command execution data object to
--                      concatenation of string header, body and footer, of
--                      ports count more than 0 otherwise to alternative,
--                      if it is relevant footer could be irrelevant
--
--        @return       operation succed
--
function Command_Data:setResultStrOnPortCount(header, body, footer, alternative)
    body    = concatHeaderBodyAndFooter(header, body, footer)

    self:setResultOnPortCount(body, alternative)

    return 0
end

-- ************************************************************************
---
--  Command_Data:setResultOnEntriesCount
--        @description  set's result in command execution data object to
--                      given value, if entries count more than 0 otherwise
--                      to alternative, if it is relevant
--
--        @return       operation succed
--
function Command_Data:setResultOnEntriesCount(value, alternative)
    if     0 < self["entries_count"] then
        self["result"] = value
    elseif nil ~= alternative then
        self["result"] = alternative
    end

    return 0
end

-- ************************************************************************
---
--  Command_Data:setResultStrOnEntriesCount
--        @description  sets result in command execution data object to
--                      concatenation of string header, body and footer, of
--                      entries count more than 0 otherwise to alternative,
--                      if it is relevant footer could be irrelevant
--
--        @return       operation succeed
--
function Command_Data:setResultStrOnEntriesCount(header, body, footer, alternative)
    body    = concatHeaderBodyAndFooter(header, body, footer)

    self:setResultOnEntriesCount(body, alternative)

    return 0
end

-- ************************************************************************
---
--  Command_Data:getTrueIfFlagNoIsNil
--        @description  get's of bollean value (true or false) according
--                      to command no-flag
--
--        @param params             - params["flagNo"]: command no-flag
--
--        @return       true if no-flag is nil, otherwise false
--
function Command_Data:getTrueIfFlagNoIsNil(params)
    if nil == params["flagNo"] then
        return true
    else
        return false
    end
end

-- ************************************************************************
---
--  Command_Data:initInterfaceDeviceRange
--        @description  initialization of device range on interface
--                      dev/port range of command execution data object
--                      Requires:
--                          initAllInterfacesPortIterator(params)
--                          initInterfaceDevPortRange() or
--                          initInterfaceDevVlanRange()
--
--        @param range_name     - name of range in ommand execution data
--                                object (default value is
--                                "dev_port_range")
--
--        @return       operation succed
--
function Command_Data:initInterfaceDeviceRange(range_name)
    if nil == range_name then
        range_name = "dev_port_range"
    end

    self["dev_range"] = getTableKeys(self[range_name])

    return 0
end

-- ************************************************************************
---
--  Command_Data:getDevicesIterator
--        @description  iterating over devices in self["dev_range"]
--                      command_data:initAllDeviceRange(params) or
--                      command_data:initInterfaceDeviceRange() or
--                      command_data:initAllAvailableDevicesRange()
--                      for iterator, devNum in
--                                      command_data:getDevicesIterator()
--                      do
--                          print(devNum)
--                      end
--
--        @return       interator over all available devices
--
function Command_Data:getDevicesIterator()
    function iterator(v, i)
        local index
        if nil == i then i = 0 end
        if true ~= self["stop_execution"] then  return nil, nil end

        i = i + 1
        v = 0

        for index, dev in pairs(self["dev_range"]) do
            v = v + 1

            if v == i then
                self["current_dev_num"] = dev
                return i, dev
            end
        end

        return nil, nil
    end

    return iterator
end


-- ************************************************************************
---
--  Command_Data:setIpPrefixEntries
--        @description  ip prefix entries data setting
--
--        @param ip_protocol    - ip v4 or v6 protocol
--
--        @return       operation succes
--
function Command_Data:setIpPrefixEntries(ip_protocol)
    self["ip_prefix_entries"]   = { ["ip_protocol"] = ip_protocol }

    return 0
end


-- ************************************************************************
---
--  Command_Data:setEmptyMirrorAnalyzerEntries
--        @description  setting of mirror analyzer entries indexes of
--                      command execution data object according to given
--                      device range;
--
--        @param dev_range     - device tange
--
--        @return       operation succeeded
--
function Command_Data:setEmptyMirrorAnalyzerEntries(dev_range)
    local index, devNum

    self["mirror_analyzer_entries"] = {}

    for index, devNum in pairs(dev_range) do
        self["mirror_analyzer_entries"][devNum] = {}
    end

    return 0
end

-- ************************************************************************
---
--  Command_Data:setDevicesEmptyVidxRange
--        @description  setting of dev/vidx rangem where vidx is first
--                      available empty vidx of given device
--
--        @param dev_range     - device tange
--        @return       operation succeeded
--
function Command_Data:setDevicesEmptyVidxRange(dev_range)
    local index, devNum

    self["dev_vidx_range"] = {}

    for index, devNum in pairs(dev_range) do
        self["dev_vidx_range"][devNum] = {}
    end

    return 0
end


-- ************************************************************************
---
--  Command_Data:setRangeIterator
--        @description  initialize iterator over the dev/port pairs to go
--                      through given dev/port pairs
--
--        @param range          - given range
--
--        @return       operation succed
--
function Command_Data:setRangeIterator(range)
    self["dev_port_range"] = range

    return 0
end

-- ************************************************************************
---
--  Command_Data:setDevRangePorts
--        @description  setting of dev/port pairs of command execution data
--                      object according to given device range
--
--        @return       operation succed
--
function Command_Data:setDevRangePorts(dev_range)
    local devIndex, devId
    local i

    self:clearDevPortRange()
    for devIndex, devId in pairs(dev_range) do
        self["dev_port_range"][devId] = get_port_list(devId)
    end

    return 0
end

-- ************************************************************************
---
--  Command_Data:setDevicesPortCounts
--        @description  setting of the device port counts of command
--                      execution data object according to given device
--                      range
--
--        @param dev_range     - device tange
--
--        @return       operation succeeded
--
function Command_Data:setDevicesPortCounts(dev_range)
    local result, values
    local index, devNum

    self["dev_port_counts"] = {}

    for index, devNum in pairs(dev_range) do
        result, values = wrlCpssDevicePortNumberGet(devNum)
        if     0 == result then
            self["dev_port_counts"][devNum] = values
        elseif 0 ~= result then
            command_data:setFailStatus()
            command_data:addError("Error at port number getting of device %d: %s.",
                                  devNum, values)
        end
    end

    return 0
end

-- ************************************************************************
---
--  Command_Data:initInterfaceDevPortRange
--        @description  dev/port range of command execution data
--                      object according to initializes interface range
--
--        @usage __global           - __global["ifRange"]: iterface range
--                                    dev/port table
--
--        @return       operation succed
--
function Command_Data:initInterfaceDevPortRange()
    self["dev_port_range"] = getGlobal("ifRange")

    self:checkIterfaceType({"ethernet", "eport"})

    return 0
end

-- ************************************************************************
---
--  Command_Data:checkIterfaceType
--        @description  checking of entry params presence in
--                      __global["ifType"], and error, when it is absent
--
--        @param ...                - allowed types list
--
--        @usage __global           - __global["ifType"]: name of interface
--
--        @return       true, if it is present, otherwise false
--
function Command_Data:checkIterfaceType(...)
    local returned_value = isItemInTable(getGlobal("ifType"), ...)

    if false == returned_value then
        self:setFailStatus()
        self:addError("Such type of interface is not supported")
    end

    return returned_value
end

-- ************************************************************************
---
--  Command_Data:setResultArrayToResultStr
--        @description  sets result in command execution data object to
--                      result array converted to string
--
--        @param first          - First item array offset (optional)
--        @param step           - Items step in array (optional)
--
--        @return       operation succeed
--
function Command_Data:setResultArrayToResultStr(first, step)
    self["result"] = get_interlaced_tbl(self["result_array"], first, step)

    return 0
end

-- ************************************************************************
---
--  Command_Data:getLastResultArrayItem
--        @description  get's last item of command execution data object
--                      result array
--
--        @return       last array item or nil, if it is empty array
--
function Command_Data:getLastResultArrayItem()
    return getLastTableItem(self["result_array"])
end


-- ************************************************************************
---
--  Command_Data:getLastResultArrayItemIndex
--        @description  get's last item index of command execution data
--                      object result array
--
--        @return       last array item index or nil, if it is empty array
--
function Command_Data:getLastResultArrayItemIndex()
    return getLastTableItemIndex(self["result_array"])
end


-- ************************************************************************
---
--  Command_Data:getInterlacedResultArray
--        @description  gets array which is interlaced toward result array
--                      of command execution data object
--
--        @param first          - First item array offset (optional)
--        @param step           - Items step in array (optional)
--
--        @return       interlaced result array
--
function Command_Data:getInterlacedResultArray(first, step)
    return get_interlaced_tbl(self["result_array"], first, step)
end

-- ************************************************************************
---
--  Command_Data:getResultArrayStr
--        @description  gets result array of command execution data object
--                      converted to string
--
--        @param first          - First item array offset (optional)
--        @param step           - Items step in array (optional)
--
--        @return       array converted to string
--
function Command_Data:getResultArrayStr(first, step)
    return tbltostr(self["result_array"], '\n', first, step)
end

-- ************************************************************************
---
--  Command_Data:setStrOnDevicesCount
--        @description  get's given value, if devices count of command
--                      execution data object more than 0 otherwise
--                      get's alternative, if it is relevant
--
--        @return       operation succed
--
function Command_Data:setStrOnDevicesCount(value, alternative)
    if     0 < self["devices_count"] then
        return value
    elseif nil ~= alternative then
        return alternative
    end
end

-- ************************************************************************
---
--  Command_Data:setStrOnPortsCount
--        @description  get's given value, if ports count of command
--                      execution data object more than 0 otherwise
--                      get's alternative, if it is relevant
--
--        @return       operation succed
--
function Command_Data:setStrOnPortsCount(value, alternative)
    if     0 < self["ports_count"] then
        return value
    elseif nil ~= alternative then
        return alternative
    end
end

-- ************************************************************************
---
--  Command_Data:setStrOnPrioritiesCount
--        @description  get's given value, if priorities count of command
--                      execution data object more than 0 otherwise
--                      get's alternative, if it is relevant
--
--        @return       operation succed
--
function Command_Data:setStrOnPrioritiesCount(value, alternative)
    if     0 < self["priorities_count"] then
        return value
    elseif nil ~= alternative then
        return alternative
    end
end

-- ************************************************************************
---
--  Command_Data:setStrOnEntriesCount
--        @description  get's given value, if entries count of command
--                      execution data object more than 0 otherwise
--                      get's alternative, if it is relevant
--
--        @return       operation succed
--
function Command_Data:setStrOnEntriesCount(value, alternative)
    if     0 < self["entries_count"] then
        return value
    elseif nil ~= alternative then
        return alternative
    end
end

-- ************************************************************************
---
--  Command_Data:initDevPortRange
--        @description  dev/port range of command execution data
--                      object according to entry parameters
--
--        @param params         - params["ethernet"]: entry ethernet
--                                interface dev/port pair or range, could
--                                be irrelevant;
--                                params["port-channel"]: entry trunk,
--                                could be irrelevant;
--                                params["devId"]: device number, all ports
--                                of which should be added to dev/port range
--                                of command execution data object;
--                                given dev/port range (relevant, if others
--                                entry cases are irrelevant);
--                                alternative interface behavior
--                                checking flag
--        @param alternative    - alternative interface behavior
--                                checking flag
--        @param empty_by_default
--                              - set empty dev/port range when entry options
--                                are not defined
--
--        @return       true is success or alternative != true otherwise
--                      false
--
function Command_Data:initDevPortRange(params, alternative, empty_by_default)
    local result, error_message
    local no_need_alternative = true

    result = 0

    if nil ~= params["eport"] then
      if (nil ~= params["eport"]["devId"]) and
         (nil ~= params["eport"]["portNum"])  then
         self:setDevPortRange({[params["eport"]["devId"]] =
                                            {params["eport"]["portNum"]}})
      else
        result                = 1
        error_message         = "Illegal dev/ePort range is not supported."
        no_need_alternative   = true
      end
    elseif nil ~= params["ethernet"] then
      if true == is_dev_port_range(params["ethernet"]) then
        if nil ~= params["devId"] then
          self:setDevPortRange({[params["devId"]] = get_port_list(params["devId"])})
        else
          self:setDevPortRange(params["ethernet"])
        end
      elseif (nil ~= params["ethernet"]["devId"]) and
             (nil ~= params["ethernet"]["portNum"])  then
          self:setDevPortRange({[params["ethernet"]["devId"]] =
                                            {params["ethernet"]["portNum"]}})
      end
    elseif nil ~= params["port-channel"] then
        result, error_message   =
            self:setPortChannelDevPortRange(params["port-channel"])
        if (   0 == result)                                     and
           (true == isEmptyTable(self["dev_port_range"]))       then
            self:setFailStatus()
            self:addWarning("Trunk " .. params["port-channel"] ..
                            " does no contain any ports.")
        end
    elseif true == is_dev_port_range(params)            then
        self:setDevPortRange(params)
    elseif (nil ~= self["dev_range"])                   and
           (nil ~= params["ethernet"])                  then
        if    (nil ~= params["ethernet"]["devId"])          and
              (nil ~= params["ethernet"]["portNum"])        then
            self:setDevPortRange({[params["ethernet"]["devId"]] =
                                            {params["ethernet"]["portNum"]}})
        else
            self:setDevPortRange(makeNestedTable(self["dev_range"],
                                                 params["ethernet"]))
        end
    elseif nil ~= self["dev_range"]                     then
        if true== empty_by_default                  then
            self:setDevPortRange(self:getEmptyDeviceNestedTable())
        else
        self:setDevRangePorts(self["dev_range"])
        end
    elseif true == alternative                          then
        result                = 0
        no_need_alternative   = false
        else
        result                = 1
        error_message         = "Such kind of dev/port range is not supported."
        no_need_alternative   = true
    end

    if 0 ~= result then
        self:setFailStatus()
        self:addError(error_message)
    end

    return no_need_alternative
end

-- ************************************************************************
---
--  Command_Data:updateStatus
--        @description  updates cammand execution status according to its
--                      local status in command execution data object
--
--        @return       operation succed
--
function Command_Data:updateStatus()
    if true ~= self["local_status"] then
        self["status"] = false
    end

    return 0
end

-- ************************************************************************
---
--  Command_Data:updatePorts
--        @description  updates ports count statistic in command execution
--                      data object
--
function Command_Data:updatePorts()
    self["ports_count"] = self["ports_count"] + 1
    if true == self["port_status"] then
        self["ports_with_success_count"] = self["ports_with_success_count"] + 1
    end
end

-- ************************************************************************
---
--  Command_Data:updateDevices
--        @description  updates devices count statistic in command
--                      execution data object
--
function Command_Data:updateDevices()
    self["devices_count"] = self["devices_count"] + 1
    if true == self["device_status"] then
        self["devices_with_success_count"] =
            self["devices_with_success_count"] + 1
    end
end

-- ************************************************************************
---
--  Command_Data:handleCpssErrorDevPort
--        @description  check cpss return code
--                      set's warning/error message
--
--        @param status    - cpss function return status
--        @param message   - function description for error/warning msg
--        @param devNum    - (optional, default self["current_dev_num"])
--        @param portNum   - (optional, default self["current_port_num"])
--
--        @return       operation succed
--
-- Example:
--        result = cpssPerPortParamSet("cpssDxChIpPortRouterMacSaLsbModeSet",
--                                devNum, portNum,
--                                "CPSS_SA_LSB_PER_PORT_E", "saLsbMode",
--                                "CPSS_MAC_SA_LSB_MODE_ENT")
--        command_data:handleCpssErrorDevPort(result,
--                    "setting mode for MAC SA least significant byte",
--                    devNum, portNum)
--
function Command_Data:handleCpssErrorDevPort(status, message, devNum, portNum)
    if status == 0 then
        return
    end

    if devNum == nil then
        devNum = self["current_dev_num"]
    end
    if portNum == nil then
        portNum = self["current_port_num"]
    end
    if 0x10 == status then      -- GT_NOT_SUPPORTED
        self:setFailDeviceStatus()
        self:addWarning("port doesn't support "..message.." on device %d port %d",
                    devNum, portNum)
    elseif 0x1e == status then  -- GT_NOT_APPLICABLE_DEVICE
        self:setFailDeviceStatus()
        self:addWarning(message .. " is not applicable for device %d", devNum)
    elseif 0 ~= status then
        self:setFailPortAndLocalStatus()
        self:addError("Error at " .. message .. "on device %d port %d, result: %s",
                    devNum, portNum, returnCodes[status])
    end

end

-- ************************************************************************
---
--  Command_Data:handleCpssErrorDevice
--        @description  check cpss return code
--                      set's warning/error message
--
--        @param status    - cpss function return status
--        @param message   - function description for error/warning msg
--        @param devNum    - (optional, default self["current_dev_num"])
--
--        @return       operation succed
--
-- Example:
--        result = myGenWrapper("cpssDxChIpRouterMacSaBaseSet", {
--                    { "IN", "GT_U8",        "devNum", devNum  },
--                    { "IN", "GT_ETHERADDR", "mac",    mac_address }})
--        command_data:handleCpssErrorDevice(result,
--                    "setting of Router MAC SA Base address", devNum)
--
function Command_Data:handleCpssErrorDevice(status, message, devNum)
    if status == 0 then
        return
    end

    if devNum == nil then
        devNum = self["current_dev_num"]
    end
    if 0x10 == status then      -- GT_NOT_SUPPORTED
        self:setFailDeviceStatus()
        self:addWarning("Device doesn't support "..message.." on device %d",
                    devNum)
    elseif 0x1e == status then  -- GT_NOT_APPLICABLE_DEVICE
        self:setFailDeviceStatus()
        self:addWarning(message .. " is not applicable for device %d", devNum)
    elseif 0 ~= status then
        self:setFailDeviceAndLocalStatus()
        self:addError("Error at " .. message .. " on device %d, result: %s",
                    devNum, returnCodes[status])
    end

end

-- ************************************************************************
---
--  Command_Data:setResultOnPortCount
--        @description  set's result in command execution data object to
--                      given value, if ports count more than 0 otherwise
--                      to alternative, if it is relevant
--
--        @return       operation succed
--
function Command_Data:setResultOnPortCount(value, alternative)
    if     0 < self["ports_count"] then
        self["result"] = value
    elseif nil ~= alternative then
        self["result"] = alternative
    end

    return 0
end


-- function dedicated to such use in high level command implementing functions:
-- if <some error> then
--     return Command_Data:addErrorAndPrint("some error", ...);
-- end
-- ************************************************************************
---
--  Command_Data:addErrorAndPrint
--        @description  function dedicated to such use in high level command
--                      implementing functions:
--                      if <some error> then
--                      return Command_Data:addErrorAndPrint(fmt_string, ...);
--        @param formatString - format string. Is the same as first
--                      argument of function string.format.
--        @param ...    format values substituted instead of %-specifiers
--                      inside formatString, ...
--
--
--        @return       operation succed
--
function Command_Data:addErrorAndPrint(formatString, ...)
    self:setFailStatus()
    self:setFailLocalStatus()
    self:addError(formatString, ...)
    self:analyzeCommandExecution()
    self:printCommandExecutionResults()
    -- array already analyzed (copied to prined string) and printed
    -- clear it to not being printed again
    self["error_array"] = {};
    return self:getCommandExecutionResults()
end

--  Command_Data:initStringItemsRange
--        @description
--
--        @param params         - not used
--        @param item_table     - items container
--        @param alternative    - alternative interface behavior
--                                checking flag
--
--        @return       true is success or alternative != true otherwise
--                      false
--
function Command_Data:initStringItemsRange(params, item_table, alternative)
    local result, error_message
    local index, item
    local no_need_alternative = true

    result              = 0
    self["item_array"]  = {}

    if "table" == type(item_table)                      then
        for index, item in pairs(item_table) do
            tableAppend(self["item_array"], tostring(item))
        end
    elseif true == alternative                          then
        result                = 0
        no_need_alternative   = false
    else
        result                = 1
        error_message         = "Such kind of utems range is not supported"
        no_need_alternative   = true
    end

    if 0 ~= result then
        self:setFailStatus()
        self:addError(error_message)
    end

    return no_need_alternative
end

-- ************************************************************************
---
--  Command_Data:getStringItemsIterator
--        @description  iterating over string items in items range of
--                      command execution data object
--                      command_data:initStringItemsRange(params, table)
--                      for iterator, stringItem in
--                                  command_data:getStringItemsIterator()
--                      do
--                          print(stringItem)
--                      end
--
--        @return       interator over enabled mirrored ports
--
function Command_Data:getStringItemsIterator()
    function iterator(v, i)
        local index = 1
        local item_index, item
        if nil == i then i = 0 end
        if true ~= self["stop_execution"] then  return nil, nil, nil end

        i = i + 1

        for item_index, item in pairs(self["item_array"]) do
            if "string" == type(item)   then
                if i <= index   then
                    return i, item
                else
                    index = index + 1
                end
            end
        end

        return nil, nil
    end

    return iterator
end

-- ************************************************************************
---
--  Command_Data:updateItems
--        @description  updates items count statistic in command execution
--                      data object
--
function Command_Data:updateItems()
    self["items_count"] = self["items_count"] + 1
    if true == self["items_status"] then
        self["items_with_success_count"] =
            self["items_with_success_count"] + 1
    end
end

-- ************************************************************************
---
--  Command_Data:setResultStrOnItemsCount
--        @description  sets result in command execution data object to
--                      concatenation of string header, body and footer, of
--                      items count more than 0 otherwise to alternative,
--                      if it is relevant footer could be irrelevant
--
--        @return       operation succeed
--
function Command_Data:setResultStrOnItemsCount(header, body, footer, alternative)
    body    = concatHeaderBodyAndFooter(header, body, footer)

    self:setResultOnItemsCount(body, alternative)

    return 0
end

-- ************************************************************************
---
--  Command_Data:setResultOnItemsCount
--        @description  set's result in command execution data object to
--                      given value, if items count more than 0 otherwise
--                      to alternative, if it is relevant
--
--        @return       operation succed
--
function Command_Data:setResultOnItemsCount(value, alternative)
    if     0 < self["items_count"] then
        self["result"] = value
    elseif nil ~= alternative then
        self["result"] = alternative
    end

    return 0
end
