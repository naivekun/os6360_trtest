--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_flow_control.lua
--*
--* DESCRIPTION:
--*       show dump of fc
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local function booleanToStatus (bool)
    if bool then return "Enable" end
    return "Disable"
end


---
--  buffer_management_fc
--        @description  show fc dump file
--
--        @param params             -params["device"]: specific device number
--
--        @return       true on success, otherwise false and error message
--

local function flow_control_dump(params)
    local devNum=params["device"]
    local notApplicableMsg="N/A"
    local errorMsg="ERR"
    local NOT_APPLICABLE=30
    local dataToPrint="\n"
    local errors=""
    local chunk
    local enum
    local maxPort
    local ret, val
    local temp
    local enumState, enumPeriodic, fcEnable, fcAutoNeg, fcAutoNegPause, fcMode, pariodicFcEnable, enable802_3

    --getting max port for device
    ret, maxPort = wrLogWrapper("wrlCpssDevicePortNumberGet","(devNum)",devNum)
    if 0 ~= ret then
        errors=errors.."Couldn't get number of ports for device "..devNum..": "..returnCodes[ret].." \n"
        maxPort=-1
    end

    enum={
            CPSS_PORT_FC_MODE_802_3X_E  = "802.3X",
            CPSS_PORT_FC_MODE_PFC_E     = "PFC",
            CPSS_PORT_FC_MODE_LL_FC_E   = "LL FC"
        }
    enumState=  {
                    CPSS_PORT_FLOW_CONTROL_DISABLE_E = "Disable",
                    CPSS_PORT_FLOW_CONTROL_RX_TX_E =   "Enable",
                    CPSS_PORT_FLOW_CONTROL_RX_ONLY_E=  "RX",
                    CPSS_PORT_FLOW_CONTROL_TX_ONLY_E = "TX"
                }

    enumPeriodic={
                CPSS_PORT_PERIODIC_FLOW_CONTROL_DISABLE_E =  "Disable",
                CPSS_PORT_PERIODIC_FLOW_CONTROL_XON_XOFF_E = "XOn_XOff",
                CPSS_PORT_PERIODIC_FLOW_CONTROL_XON_ONLY_E=  "XOn",
                CPSS_PORT_PERIODIC_FLOW_CONTROL_XOFF_ONLY_E= "XOff"
                }
    chunk=""
    temp=5  --number of enteries in a row
    chunk=  "Port|   FC    |  FC Auto-   |  Pause    |  FC   | Periodic | 802.3x frames |\n"..
            "    |  State  | Negotiation | Advertise | Mode  |    Fc    |   forwarding  |\n"..
            "----|---------|-------------|---------- |-------|----------|---------------|\n"
    for portNum = 0,maxPort do
        --[[ skip not existed ports and CPU port]]
        if does_port_exist(devNum, portNum) and portNum ~= 16 then

            --status
            ret,val = myGenWrapper("cpssPxPortFlowControlEnableGet",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                {"OUT","CPSS_PORT_FLOW_CONTROL_ENT","statePtr"}
                })
            if  0 == ret then
                fcEnable=enumState[val["statePtr"]]
            else
                if NOT_APPLICABLE == ret then
                    fcEnable=notApplicableMsg
                else
                    fcEnable=errorMsg
                    errors=errors.."cpssPxPortFlowControlEnableGet failed: "..returnCodes[ret].."\n"
                end
            end

            --Auto neg status and pause advertise
            ret,val = myGenWrapper("cpssPxPortFlowCntrlAutoNegEnableGet",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                {"OUT","GT_BOOL","statePtr"},
                {"OUT","GT_BOOL","pauseAdvertisePtr"}
                })
            if  0 == ret then
                fcAutoNeg=booleanToStatus(val["statePtr"])
                fcAutoNegPause=booleanToStatus(val["pauseAdvertisePtr"])
            else
                if NOT_APPLICABLE == ret then
                    fcAutoNeg=notApplicableMsg
                    fcAutoNegPause=notApplicableMsg
                else
                    fcAutoNeg=errorMsg
                    fcAutoNegPause=errorMsg
                    errors=errors.."cpssPxPortFlowCntrlAutoNegEnableGet failed: "..returnCodes[ret].."\n"
                end
            end


            --Mode
            ret,val = myGenWrapper("cpssPxPortFlowControlModeGet",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                {"OUT","CPSS_PX_PORT_FC_MODE_ENT","fcModePtr"}
                })
            if  0 == ret then
                fcMode=enum[val["fcModePtr"]]
            else
                if NOT_APPLICABLE == ret then
                    fcMode=notApplicableMsg
                else
                    fcMode=errorMsg
                    errors=errors.."cpssPxPortFlowControlModeGet failed: "..returnCodes[ret].."\n"
                end
            end

            --Periodic Fc Status
            ret,val = myGenWrapper("cpssPxPortPeriodicFcEnableGet",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                {"OUT","CPSS_PORT_PERIODIC_FLOW_CONTROL_TYPE_ENT","enablePtr"}
                })
            if  0 == ret then
                pariodicFcEnable=enumPeriodic[val["enablePtr"]]
            else
                if NOT_APPLICABLE == ret then
                    pariodicFcEnable=notApplicableMsg
                else
                    pariodicFcEnable=errorMsg
                    errors=errors.."cpssPxPortPeriodicFcEnableGet failed: "..returnCodes[ret].."\n"
                end
            end

            --802.3x frames forwarding
            ret,val = myGenWrapper("cpssPxPortForward802_3xEnableGet",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                {"OUT","GT_BOOL","enablePtr"}
                })
            if  0 == ret then
                enable802_3=booleanToStatus(val["enablePtr"])
            else
                if NOT_APPLICABLE == ret then
                    enable802_3=notApplicableMsg
                else
                    enable802_3=errorMsg
                    errors=errors.."cpssPxPortPeriodicFcEnableGet failed: "..returnCodes[ret].."\n"
                end
            end

            chunk=chunk..string.format("%-4s|%-9s|%-13s|%-11s|%-7s|%-10s|%-15s|",
                                        portNum,
                                        alignLeftToCenterStr(tostring(fcEnable),9),
                                        alignLeftToCenterStr(tostring(fcAutoNeg),13),
                                        alignLeftToCenterStr(tostring(fcAutoNegPause),11),
                                        alignLeftToCenterStr(tostring(fcMode),7),
                                        alignLeftToCenterStr(tostring(pariodicFcEnable),10),
                                        alignLeftToCenterStr(tostring(enable802_3),15)).."\n"

        end
    end
    dataToPrint=dataToPrint..(chunk.."\n\n")

    if ""~=errors then
        dataToPrint=dataToPrint..("Errors:\n-------\n"..errors.."\n\n")
    end

    print(dataToPrint)

end


--------------------------------------------------------------------------------
-- command registration: counters show
--------------------------------------------------------------------------------
--CLI_addHelp("exec", "show counters", "Display counters")
CLI_addCommand("exec", "show flow-control ", {
  func   = flow_control_dump,
  help   = "The dump of all Flow Control related information",
  params={{type= "named", {format="device %devID", name="device", help="ID of the device"},
                          mandatory = {"device"}}
  }
})