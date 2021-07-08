--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_ccfc.lua
--*
--* DESCRIPTION:
--*       show dump of ccfc
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************
local function booleanToStatus (bool)
    if bool then return "Enable" end
    return "Disable"

end

local function ccfcDump(params)
    local devNum=params["device"]
    local notApplicableMsg="N/A"
    local errorMsg="ERR"
    local NOT_APPLICABLE=30
    local dataToPrint=""
    local errors=""
    local chunk
    local enum
    local maxPort
    local ret, val
    local temp    
    local cnProfile, cnSpeed, cnPause, cnFc, cnMode, cnFrameQueue, cnEthertype
    
    local temp2
    local flag
    
    dataToPrint=dataToPrint..("\n")
    --getting max port for device
    ret, maxPort = wrLogWrapper("wrlCpssDevicePortNumberGet", "(devNum)", devNum)
    if 0 ~= ret then
        errors=errors.."Couldn't get number of ports for device "..devNum..": "..returnCodes[ret].." \n"
        maxPort=-1
    end


--1
    ret,val = myGenWrapper("cpssDxChPortCnModeEnableGet",{
                {"IN","GT_U8","devNum",devNum},
                {"OUT","CPSS_DXCH_PORT_CN_MODE_ENT","enablePtr"}
                })
    if  0 == ret then
        cnMode=val["enablePtr"]
    else
        if NOT_APPLICABLE == ret then
            cnMode=notApplicableMsg 
        else
            cnMode=errorMsg
            errors=errors.."cpssDxChPortCnModeEnableGet failed: "..returnCodes[ret].."\n"
        end
    end             

    dataToPrint=dataToPrint..(string.format("CN status: [%s]\n",booleanToStatus(cnMode)))
                

                
    --6         
    ret,val = myGenWrapper("cpssDxChPortCnFrameQueueGet",{
                {"IN","GT_U8","devNum",devNum},
                {"OUT","GT_U8","tcQueuePtr"}
                })
                
    if  0 == ret then
        cnFrameQueue=val["tcQueuePtr"]
    else
        if NOT_APPLICABLE == ret then
            cnFrameQueue=notApplicableMsg 
        else
            cnFrameQueue=errorMsg
            errors=errors.."cpssDxChPortCnFrameQueueGet failed: "..returnCodes[ret].."\n"
        end
    end             
    dataToPrint=dataToPrint..(string.format("Traffic class queue associated with CN frames: [%s]\n",tostring(cnFrameQueue)))

                
    --8   
    ret,val = myGenWrapper("cpssDxChPortCnEtherTypeGet",{
                {"IN","GT_U8","devNum",devNum},
                {"OUT","GT_U32","etherTypePtr"}
                })

    if  0 == ret then
        cnEthertype=val["etherTypePtr"]
    else
        if NOT_APPLICABLE == ret then
            cnEthertype=notApplicableMsg 
        else
            cnEthertype=errorMsg
            errors=errors.."cpssDxChPortCnEtherTypeGet failed: "..returnCodes[ret].."\n"
        end
    end             

                
    dataToPrint=dataToPrint..(string.format("EtherType for CN frames: [%s]\n",tostring(cnEthertype)))



    
 
    
    dataToPrint=dataToPrint..("\n")
    flag=false
    chunk="Buffer limit threshold for triggering CN frame:\n"..
          "-----------------------------------------------\n"
    chunk=chunk..
            " profile | profile | profile | profile | profile | profile | profile | profile \n"..
            "    0    |    1    |    2    |    3    |    4    |    5    |    6    |    7    \n"..
            "---------|---------|---------|---------|---------|---------|---------|---------\n"
    
    temp={}
    for profileIndex=0,7 do
        --2      
        ret,val = myGenWrapper("cpssDxChPortCnProfileThresholdGet",{
                    {"IN","GT_U8","devNum",devNum},
                    {"IN","GT_U32","profileIndex",profileIndex},
                    {"OUT","GT_U32","thresholdPtr"}
                    })
        if  0 == ret then
            temp[profileIndex]=val["thresholdPtr"]
        else
            if NOT_APPLICABLE == ret then
                flag=true
                break
            else
                temp[profileIndex]=errorMsg
                errors=errors.."cpssDxChPortCnProfileThresholdGet failed: "..returnCodes[ret].."\n"
            end
        end   
    end
    if flag then
        dataToPrint=dataToPrint..("Buffer limit threshold for triggering CN frame:\n"..
          "-----------------------------------------------\n")
        dataToPrint=dataToPrint..(notApplicableMsg.."\n\n\n")
    else
        chunk=chunk..string.format("%-9s|%-9s|%-9s|%-9s|%-9s|%-9s|%-9s|%-9s",
                                alignLeftToCenterStr(tostring(temp[0]),9), 
                                alignLeftToCenterStr(tostring(temp[1]),9), 
                                alignLeftToCenterStr(tostring(temp[2]),9), 
                                alignLeftToCenterStr(tostring(temp[3]),9), 
                                alignLeftToCenterStr(tostring(temp[4]),9), 
                                alignLeftToCenterStr(tostring(temp[5]),9), 
                                alignLeftToCenterStr(tostring(temp[6]),9), 
                                alignLeftToCenterStr(tostring(temp[7]),9)).."\n"
                 
        dataToPrint=dataToPrint..(chunk.."\n\n")
    end
    
    

    flag=false
    chunk="Buffer limit threshold for triggering CN frame on a given tc queue:\n"..
          "-------------------------------------------------------------------\n"
    chunk=chunk..
            " tc  |profile |profile |profile |profile |profile |profile |profile |profile |\n"..
            "queue|   0    |   1    |   2    |   3    |   4    |   5    |   6    |   7    |\n"..
            "-----|--------|--------|--------|--------|--------|--------|--------|--------|\n"
    
    for  tcQueue= 0,7 do
        temp={}
        for profileIndex=0,7 do
            --2      
            ret,val = myGenWrapper("cpssDxChPortCnProfileQueueThresholdGet",{
                        {"IN","GT_U8","devNum",devNum},
                        {"IN","GT_U32","profileIndex",profileIndex},
                        {"IN","GT_U8","tcQueue",tcQueue},
                        {"OUT","GT_U32","thresholdPtr"}
                        })
            if  0 == ret then
                temp[profileIndex]=val["thresholdPtr"]
            else
                if NOT_APPLICABLE == ret then
                    flag=true
                    break 
                else
                    temp[profileIndex]=errorMsg
                    errors=errors.."cpssDxChPortCnProfileQueueThresholdGet failed: "..returnCodes[ret].."\n"
                end
            end   
        end
        if flag then
            break
        end
        chunk=chunk..string.format("%-5s|%-8s|%-8s|%-8s|%-8s|%-8s|%-8s|%-8s|%-8s|",
                                tcQueue,
                                alignLeftToCenterStr(tostring(temp[0]),8), 
                                alignLeftToCenterStr(tostring(temp[1]),8), 
                                alignLeftToCenterStr(tostring(temp[2]),8), 
                                alignLeftToCenterStr(tostring(temp[3]),8), 
                                alignLeftToCenterStr(tostring(temp[4]),8), 
                                alignLeftToCenterStr(tostring(temp[5]),8), 
                                alignLeftToCenterStr(tostring(temp[6]),8), 
                                alignLeftToCenterStr(tostring(temp[7]),8)).."\n" 
    end
    if flag then
        dataToPrint=dataToPrint..("Buffer limit threshold for triggering CN frame on a given tc queue:\n"..
          "-------------------------------------------------------------------\n")
        dataToPrint=dataToPrint..(notApplicableMsg.."\n\n\n")
    else
        dataToPrint=dataToPrint..(chunk.."\n\n")
    end

    
    flag=false
    chunk="Status of Congestion Notification awareness:\n"..
          "--------------------------------------------\n"
    chunk=chunk..
            "tc queue |tc queue |tc queue |tc queue |tc queue |tc queue |tc queue |tc queue \n"..
            "    0    |    1    |    2    |    3    |    4    |    5    |    6    |    7    \n"..
            "---------|---------|---------|---------|---------|---------|---------|---------\n"
    
    temp={}
    for tcQueue=0,7 do
        --2      
        ret,val = myGenWrapper("cpssDxChPortCnQueueAwareEnableGet",{
                    {"IN","GT_U8","devNum",devNum},
                    {"IN","GT_U8","tcQueue",tcQueue},
                    {"OUT","GT_BOOL","enablePtr"}
                    })
        if  0 == ret then
            temp[tcQueue]=booleanToStatus(val["enablePtr"])
        else
            if NOT_APPLICABLE == ret then
                flag=true 
                break
            else
                temp[tcQueue]=errorMsg
                errors=errors.."enablePtr failed: "..returnCodes[ret].."\n"
            end
        end   
    end
    
    if flag then
        dataToPrint=dataToPrint..("Status of Congestion Notification awareness:\n"..
          "--------------------------------------------\n")
        dataToPrint=dataToPrint..(notApplicableMsg.."\n\n\n")
    else
        chunk=chunk..string.format("%-9s|%-9s|%-9s|%-9s|%-9s|%-9s|%-9s|%-9s",
                                alignLeftToCenterStr(tostring(temp[0]),9), 
                                alignLeftToCenterStr(tostring(temp[1]),9), 
                                alignLeftToCenterStr(tostring(temp[2]),9), 
                                alignLeftToCenterStr(tostring(temp[3]),9), 
                                alignLeftToCenterStr(tostring(temp[4]),9), 
                                alignLeftToCenterStr(tostring(temp[5]),9), 
                                alignLeftToCenterStr(tostring(temp[6]),9), 
                                alignLeftToCenterStr(tostring(temp[7]),9)).."\n"
                 
        dataToPrint=dataToPrint..(chunk.."\n\n")
    end



    chunk=""
    chunk=  "Port|   CN    |     CN     |  CN pause |  CN FC  |\n"..
            "    | profile |speed index |  trigger  |         |\n"..
            "----|---------|------------|-----------|---------|\n"
            
    for portNum = 0,maxPort do
        --[[ skip not existed ports ]]
        if does_port_exist(devNum, portNum) then
   
            --cnProfile
            ret,val = myGenWrapper("cpssDxChPortCnProfileGet",{
                        {"IN","GT_U8","devNum",devNum},
                        {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                        {"OUT","GT_U32","profileIndexPtr"}
                        })    
            if  0 == ret then
                cnProfile=val["profileIndexPtr"]
            else
                if NOT_APPLICABLE == ret then
                    cnProfile=notApplicableMsg 
                else
                    cnProfile=errorMsg
                    errors=errors.."cpssDxChPortCnProfileGet failed: "..returnCodes[ret].."\n"
                end
            end     





            --cnSpeed
            ret,val = myGenWrapper("cpssDxChPortCnSpeedIndexGet",{
                        {"IN","GT_U8","devNum",devNum},
                        {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                        {"OUT","GT_U32","portSpeedIndexPtr"}
                        })  
            if  0 == ret then
                cnSpeed=val["portSpeedIndexPtr"]
            else
                if NOT_APPLICABLE == ret then
                    cnSpeed=notApplicableMsg 
                else
                    cnSpeed=errorMsg
                    errors=errors.."cpssDxChPortCnSpeedIndexGet failed: "..returnCodes[ret].."\n"
                end
            end   


            --cnPause
            ret,val = myGenWrapper("cpssDxChPortCnPauseTriggerEnableGet",{
                        {"IN","GT_U8","devNum",devNum},
                        {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                        {"OUT","GT_BOOL","enablePtr"}
                        }) 
            if  0 == ret then
                cnPause=booleanToStatus(val["enablePtr"])
            else
                if NOT_APPLICABLE == ret then
                    cnPause=notApplicableMsg 
                else
                    cnPause=errorMsg
                    errors=errors.."cpssDxChPortCnPauseTriggerEnableGet failed: "..returnCodes[ret].."\n"
                end
            end               
        
        
        
        
            --cnFc
            ret,val = myGenWrapper("cpssDxChPortCnFcEnableGet",{
                        {"IN","GT_U8","devNum",devNum},
                        {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                        {"OUT","GT_BOOL","enablePtr"}
                        })
            if  0 == ret then
                cnFc=booleanToStatus(val["enablePtr"])
            else
                if NOT_APPLICABLE == ret then
                    cnFc=notApplicableMsg 
                else
                    cnFc=errorMsg
                    errors=errors.."cpssDxChPortCnFcEnableGet failed: "..returnCodes[ret].."\n"
                end
            end               
        

            chunk=chunk..string.format("%-4s|%-9s|%-12s|%-11s|%-9s|",
                                        portNum,
                                        alignLeftToCenterStr(tostring(cnProfile),9), 
                                        alignLeftToCenterStr(tostring(cnSpeed),12), 
                                        alignLeftToCenterStr(tostring(cnPause),11), 
                                        alignLeftToCenterStr(tostring(cnFc),9)).."\n"

            
        end    
    end
    dataToPrint=dataToPrint..(chunk.."\n\n")
    
    
    enum={
        CPSS_DXCH_PORT_FC_MODE_802_3X_E="FC",
        CPSS_DXCH_PORT_FC_MODE_PFC_E   ="PFC",
        CPSS_DXCH_PORT_FC_MODE_LL_FC_E ="LL_FC"
        }
    
    chunk="FC/PFC Pause time:\n"..
          "------------------\n"
    chunk=chunk..
            "Port|  FC   |Index |Index |Index |Index |Index |Index |Index |Index |\n"..
            "    | Mode  |  0   |  1   |  2   |  3   |  4   |  5   |  6   |  7   |\n"..
            "----|-------|------|------|------|------|------|------|------|------|\n"
    
    for portNum = 0,maxPort do
        --[[ skip not existed ports ]]
        if does_port_exist(devNum, portNum) then
            temp={}
            for index=0,7 do
                --cnProfile
                ret,val = myGenWrapper("cpssDxChPortCnFcTimerGet",{
                            {"IN","GT_U8","devNum",devNum},
                            {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                            {"IN","GT_U32","index",index},
                            {"OUT","GT_U32","timerPtr"}
                            }) 
                if  0 == ret then
                    temp[index]=val["timerPtr"]
                else
                    if NOT_APPLICABLE == ret then
                        temp[index]=notApplicableMsg 
                    else
                        temp[index]=errorMsg
                        errors=errors.."cpssDxChPortCnProfileGet failed: "..returnCodes[ret].."\n"
                    end
                end     
            end
            
            
            ret,val = myGenWrapper("cpssDxChPortFlowControlModeGet",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                {"OUT","CPSS_DXCH_PORT_FC_MODE_ENT","fcModePtr"}
                })
            if  0 == ret then  
                temp2=enum[val["fcModePtr"]]

            else

                if NOT_APPLICABLE==ret then
                    temp2=notApplicableMsg
                else
                
                    errors=errors.."cpssDxChPortFlowControlModeGet failed: "..returnCodes[ret].."\n"                
                end
            end
            
            
            
            
            
            
            ret,val = myGenWrapper("cpssDxChPortFlowControlModeGet",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                {"OUT","CPSS_DXCH_PORT_FC_MODE_ENT","fcModePtr"}
                })
            if  0 == ret then 
                temp2=enum[val["fcModePtr"]]
            else
                if NOT_APPLICABLE==ret then
                    temp2=notApplicableMsg
                else
                    temp2="ERR"
                    errors=errors.."cpssDxChPortFlowControlModeGet failed: "..returnCodes[ret].."\n"                
                end    
            end
            
            
            
            
            
            
            
            

            chunk=chunk..string.format("%-4s|%-7s|%-6s|%-6s|%-6s|%-6s|%-6s|%-6s|%-6s|%-6s|",
                                        portNum,
                                        alignLeftToCenterStr(tostring(temp2),7),
                                        alignLeftToCenterStr(tostring(temp[0]),6), 
                                        alignLeftToCenterStr(tostring(temp[1]),6), 
                                        alignLeftToCenterStr(tostring(temp[2]),6), 
                                        alignLeftToCenterStr(tostring(temp[3]),6), 
                                        alignLeftToCenterStr(tostring(temp[4]),6), 
                                        alignLeftToCenterStr(tostring(temp[5]),6), 
                                        alignLeftToCenterStr(tostring(temp[6]),6), 
                                        alignLeftToCenterStr(tostring(temp[7]),6)).."\n"
        end    
    end
    dataToPrint=dataToPrint..(chunk.."\n\n")
    

    dataToPrint=dataToPrint..("\n")
    if ""~=errors then
        dataToPrint=dataToPrint..("Errors:\n-------\n"..errors.."\n\n")
    end
    print(dataToPrint)
end






--------------------------------------------------------------------------------
-- command registration: counters show
--------------------------------------------------------------------------------
--CLI_addHelp("exec", "show counters", "Display counters")
CLI_addCommand("exec", "show ccfc ", {
  func   = ccfcDump,
  help   = "The dump of all CCFC related information",
  params={{type= "named", {format="device %devID", name="device", help="ID of the device"},
                          mandatory = {"device"}}
  }
})
