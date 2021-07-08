--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_buffer_management.lua
--*
--* DESCRIPTION:
--*       show dump of bf
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
--  buffer_management_dump
--        @description  show bf dump file 
--
--        @param params             -params["device"]: specific device number 
--
--
--        @return       true on success, otherwise false and error message
--

local function buffer_management_dump(params)
    local devNum=params["device"]
    local notApplicableMsg="N/A"
    local errorMsg="ERR"
    local NOT_APPLICABLE=30
    local GT_OK=0
    local dataToPrint=""
    local errors=""
    local chunk, chunk2
    local enum
    local maxPort
    local ret, val
    local temp, temp2    
    local enumState, enumPeriodic, fcEnable, fcAutoNegState, fcAutoNegPause, fcMode, pariodicFcEnable, enable802_3
    local temp1, temp3    

    --getting max port for device
    ret, maxPort = wrLogWrapper("wrlCpssDevicePortNumberGet","(devNum)",devNum)
    if 0 ~= ret then
        errors=errors.."Couldn't get number of ports for device "..devNum..": "..returnCodes[ret].." \n"
        maxPort=-1
    end

    local portDigits=2
    if maxPort>99 then
        portDigits=3
    end

    
    --[[
    Global Config:
        xoff=1208, xon=1088
    ]]
    ret,val = myGenWrapper("cpssDxChGlobalXoffLimitGet",{
        {"IN","GT_U8","devNum",devNum},
        {"OUT","GT_U32","xoffLimitPtr"}
        })     
    if ret ~= 0 then
       errors=errors.."cpssDxChGlobalXoffLimitGet failed: "..returnCodes[ret].."\n"
    end
    temp1=val["xoffLimitPtr"]
    
    ret,val = myGenWrapper("cpssDxChGlobalXonLimitGet",{
        {"IN","GT_U8","devNum",devNum},
        {"OUT","GT_U32","xonLimitPtr"}
        })     
    if ret ~= 0 then
       errors=errors.."cpssDxChGlobalXonLimitGet failed: "..returnCodes[ret].."\n"
    end
    temp2=val["xonLimitPtr"]
    
    
    if nil ~= temp1 and nil ~= temp2 then
        dataToPrint=dataToPrint..("\nGlobal FC Config:\nxOff="..temp1 ..", xOn="..temp2.."\n\n\n")
    end    
    
    
     --[[
    Port<->rx Profile Binding:
    [00:00] [01:00] [02:00] [03:00] [04:00] [05:00] [06:00] [07:00] [08:00] [09:00] [10:00] [11:00] [12:-1] [13:-1] [14:-1] [15:-1] 
    [16:00] [17:00] [18:00] [19:00] [20:00] [21:00] [22:00] [23:00] [24:00] [25:00] [26:00] [27:00] [28:-1] [29:-1] [30:-1] [31:-1] 
    [32:00] [33:00] [34:00] [35:00] [36:00] [37:00] [38:00] [39:00] [40:00] [41:00] [42:00] [43:00] [44:-1] [45:-1] [46:-1] [47:-1] 
    [48:00] [49:00] [50:00] [51:00] [52:00] [53:00] [54:00] [55:00] [56:00] [57:00] [58:00] [59:00] [60:-1] [61:-1] [62:-1] [63:02] 
    ]] 
    chunk=""
    local counter=0
    dataToPrint=dataToPrint..("Port<->FC Profile Binding: [port:Profile]\n\n")
    for portNum = 0,maxPort do
        --[[ skip not existed ports ]]
        if does_port_exist(devNum, portNum) then
            ret,val = myGenWrapper("cpssDxChPortRxFcProfileGet",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                {"OUT","GT_PHYSICAL_PORT_NUM","profileSetPtr"}
                })       
            if ret ~= 0 then
                if NOT_APPLICABLE==ret then
                    dataToPrint=dataToPrint..(notApplicableMsg.."\n")       
                    break
                end
                errors=errors.."cpssDxChPortRxFcProfileGet port"..portNum.." failed: "..returnCodes[ret].."\n"            
            else  
                if 0 == counter % 9 and 0~=portNum then
                    chunk=chunk.."\n"
                end   
                chunk=chunk..string.format("[%"..portDigits.."d:%1d] ",
                                portNum,
                                val["profileSetPtr"])
                counter=counter+1 
            end
        end
    end
    if ""~=chunk then

        dataToPrint=dataToPrint..(chunk.."\n\n")
    end
    
    
    
    
        --[[
    Port Profile Config:
    Port Profile limit  xoff   xon
              0  2048  2048  2048
              1  2048   192   144
              2    80    64    56
              3     0     0     0
    ]]
    dataToPrint=dataToPrint..("\nPort Profile Config:\n\n") ;
    dataToPrint=dataToPrint..("Profile | Buffer limit |   xOff  |  xOn  |\n")
    dataToPrint=dataToPrint..("--------|--------------|---------|-------|\n")
    for fcProfileNum =0,3 do 
        local flag=true
           

        ret,val = myGenWrapper("cpssDxChPortRxBufLimitGet",{
            {"IN","GT_U8","devNum",devNum},
            {"IN","CPSS_PORT_RX_FC_PROFILE_SET_ENT","profileSet",fcProfileNum},
            {"OUT","GT_U32","rxBufLimitPtr"}
            })           
        if 0==ret then
            temp1=val
        else
            flag=false
            errors=errors.."cpssDxChPortRxBufLimitGet profile "..fcProfileNum.." failed: "..returnCodes[ret].."\n"

        end
        
        
        ret,val = myGenWrapper("cpssDxChPortXoffLimitGet",{
            {"IN","GT_U8","devNum",devNum},
            {"IN","CPSS_PORT_RX_FC_PROFILE_SET_ENT","profileSet",fcProfileNum},
            {"OUT","GT_U32","xoffLimitPtr"}
            })         
        if 0==ret then
            temp2=val
        else
            flag=false
            errors=errors.."cpssDxChPortXoffLimitGet profile "..fcProfileNum.." failed: "..returnCodes[ret].."\n"            
        end


        
        
        ret,val = myGenWrapper("cpssDxChPortXonLimitGet",{
            {"IN","GT_U8","devNum",devNum},
            {"IN","CPSS_PORT_RX_FC_PROFILE_SET_ENT","profileSet",fcProfileNum},
            {"OUT","GT_U32","xonLimitPtr"}
            })                 
        if 0==ret then
            temp3=val
        else
            flag=false
            errors=errors.."cpssDxChPortXonLimitGet profile "..fcProfileNum.." failed: "..returnCodes[ret].."\n"            
        end
    
    
        if true==flag then
            dataToPrint=dataToPrint..(string.format("%-8s|%-14s|%-9s|%-7s|\n",
                                alignLeftToCenterStr(tostring(fcProfileNum),8),
                                alignLeftToCenterStr(tostring(temp1["rxBufLimitPtr"]),14),
                                alignLeftToCenterStr(tostring(temp2["xoffLimitPtr"]),9),
                                alignLeftToCenterStr(tostring(temp3["xonLimitPtr"]),7)))
        end
    end
    
    
    
        dataToPrint=dataToPrint..("\n\n")
    
    
    ret,val = myGenWrapper("cpssDxChPortCpuRxBufCountGet",{
        {"IN","GT_U8","devNum",devNum},
        {"OUT","GT_U16","cpuRxBufCntPtr"}
        })
    if GT_OK == ret then
        dataToPrint=dataToPrint..(string.format("Number of buffers allocated for the CPU:[%s]\n",val["cpuRxBufCntPtr"]))
    else
        errors=errors.."cpssDxChPortCpuRxBufCountGet failed: "..returnCodes[ret].."\n"
    end

    enum={
            CPSS_DXCH_PORT_BUFFERS_MODE_SHARED_E="Shared",
            CPSS_DXCH_PORT_BUFFERS_MODE_DIVIDED_E="Divided"
         }
 
            
    ret,val = myGenWrapper("cpssDxChPortCrossChipFcPacketRelayEnableGet",{
            {"IN","GT_U8","devNum",devNum},
            {"OUT","GT_BOOL","enablePtr"}
            })
    if GT_OK == ret then
        dataToPrint=dataToPrint..(string.format("SOHO Cross Chip Flow control packet relay staus:[%s]\n",booleanToStatus(val["enablePtr"])))
    else
        errors=errors.."cpssDxChPortCrossChipFcPacketRelayEnableGet failed: "..returnCodes[ret].."\n"
    end
            
            
    ret,val = myGenWrapper("cpssDxChPortClearByBufferEnableGet",{
            {"IN","GT_U8","devNum",devNum},
            {"IN","GT_PHYSICAL_PORT_NUM","portNum",port},
            {"OUT","GT_BOOL","enablePtr"}
            })          
    if NOT_APPLICABLE == ret then
        dataToPrint=dataToPrint..("port Buffer Memory Clear type:"..notApplicableMsg.."\n")
    elseif GT_OK ~= ret then
        errors=errors.."cpssDxChPortClearByBufferEnableGet failed: "..returnCodes[ret].."\n"
    else
        if true == val["enablePtr"] then
            dataToPrint=dataToPrint..("port Buffer Memory Clear type: Clear By Buffer mode\n")
        else
            dataToPrint=dataToPrint..("port Buffer Memory Clear type: Regular Clear mode\n")
        end
    end    

            
            
    ret,val = myGenWrapper("cpssDxChPortGlobalRxBufNumberGet",{
            {"IN","GT_U8","devNum",devNum},
            {"OUT","GT_U32","numOfBuffersPtr"}
            })
    if GT_OK == ret then
        dataToPrint=dataToPrint..(string.format("Number of buffers:[%s]\n",val["numOfBuffersPtr"]))
    else
        errors=errors.."cpssDxChPortGlobalRxBufNumberGet failed: "..returnCodes[ret].."\n"
    end

    
enum={
            "CPSS_DXCH_PORT_GROUP_GIGA_E",
            "CPSS_DXCH_PORT_GROUP_HGS_E"
     }
    --[[ 
    PortGroup Config:
    PortGroup[0]: numOfBuffersPtr=2016
    PortGroup[1]: numOfBuffersPtr=2032
    ]]
    for portgroup=1,2 do
        local portgroupType=enum[portgroup]
        ret,val = myGenWrapper("cpssDxChPortGroupRxBufNumberGet",{
            {"IN","GT_U8","devNum",devNum},
            {"IN","CPSS_DXCH_PORT_GROUP_ENT","portGroup",portgroupType},
            {"OUT","GT_U32","numOfBuffersPtr"}
            })
        if 0 == ret then               
            dataToPrint=dataToPrint..("PortGroup["..portgroupType.."]: Number of buffers="..val["numOfBuffersPtr"] .."\n")
        else
            errors=errors.."cpssDxChPortGroupRxBufNumberGet portgroup "..portgroupType.." failed: "..returnCodes[ret].."\n"
        end
    end
            

            
          
    chunk=""
    counter=0
    if 2==portDigits then
        temp=7
    else
        temp=6
    end
    chunk=chunk.."\n\nTotal number of buffers allocated for specified port: [Port: Number of buffers]\n"
    for portNum = 0,maxPort do
        --[[ skip not existed ports ]]
        if does_port_exist(devNum, portNum) then
            ret,val = myGenWrapper("cpssDxChPortRxBufNumberGet",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                {"OUT","GT_U32","numOfBuffersPtr"}
                })       
            if  0 == ret then
                if 0 == counter % temp and 0~=portNum then
                    chunk=chunk.."\n"
                end    
                chunk=chunk..(string.format("[%"..portDigits.."d:%-5s] ",portNum,val["numOfBuffersPtr"]))
                counter=counter+1
            else  
                errors=errors.."cpssDxChPortPfcForwardEnableGet failed: "..returnCodes[ret].."\n"                
            end
        end    
    end
    if ""~=chunk then
        dataToPrint=dataToPrint..(chunk.."\n\n\n")
    end
    

    ret,val = myGenWrapper("cpssDxChPortGlobalPacketNumberGet",{
            {"IN","GT_U8","devNum",devNum},
            {"OUT","GT_U32","numOfPacketsPtr"}
            })

   
    if NOT_APPLICABLE == ret then
        dataToPrint=dataToPrint..string.format("Total number of unique packets:[%s]\n",notApplicableMsg)
    elseif GT_OK ~= ret then
        errors=errors.."cpssDxChPortGlobalPacketNumberGet failed: "..returnCodes[ret].."\n"
    else
        dataToPrint=dataToPrint..(string.format("Total number of unique packets:[%s]\n",val["numOfPacketsPtr"]))
    end   

           
    ret,val = myGenWrapper("cpssDxChPortRxNumOfAgedBuffersGet",{
            {"IN","GT_U8","devNum",devNum},
            {"OUT","GT_U32","agedBuffersPtr"}
            })
    if GT_OK ~= ret then
        errors=errors.."cpssDxChPortRxNumOfAgedBuffersGet failed: "..returnCodes[ret].."\n"
    else
        dataToPrint=dataToPrint..(string.format("Number of aged buffers:[%s]\n",val["agedBuffersPtr"]))
    end                   
              
              

             

    enum={
        CPSS_DXCH_PORT_BUF_MEM_FIFO_TYPE_GIGA_E ="GIGA",
        CPSS_DXCH_PORT_BUF_MEM_FIFO_TYPE_XG_E ="XG",
        CPSS_DXCH_PORT_BUF_MEM_FIFO_TYPE_HGL_E ="HGL",
        CPSS_DXCH_PORT_BUF_MEM_FIFO_TYPE_XLG_E ="CLG",
        CPSS_DXCH_PORT_BUF_MEM_FIFO_TYPE_CPU_E ="CPU"
     }

    dataToPrint=dataToPrint..("\nbuffer memory FIFOs Thresholds:\n")
    dataToPrint=dataToPrint..("===============================\n")
    chunk=""
    for fifoType,name in pairs(enum) do
        ret,val = myGenWrapper("cpssDxChPortBufMemFifosThresholdGet",{
            {"IN","GT_U8","devNum",devNum},
            {"IN","CPSS_DXCH_PORT_BUF_MEM_FIFO_TYPE_ENT","fifoType",fifoType},
            {"OUT","GT_U32","descFifoThresholdPtr"},
            {"OUT","GT_U32","txFifoThresholdPtr"},
            {"OUT","GT_U32","minXFifoReadThresholdPtr"}
            })
        if NOT_APPLICABLE == ret then
            break
        elseif 0~=ret then
            errors=errors.."cpssDxChPortBufMemFifosThresholdGet fifoType"..name.." failed: "..returnCodes[ret].."\n"
        else
            chunk=chunk..string.format("   %-7s|%-13s|%-18s|%-29s|",
                                        name,
                                        alignLeftToCenterStr(tostring(val["descFifoThresholdPtr"]),13),
                                        alignLeftToCenterStr(tostring(val["txFifoThresholdPtr"]),18),
                                        alignLeftToCenterStr(tostring(val["minXFifoReadThresholdPtr"]),29)).."\n"
        end
    end
    if NOT_APPLICABLE == ret then
        dataToPrint=dataToPrint..("N/A\n")        
    elseif ""~=chunk then
        dataToPrint=dataToPrint..(" FifoType | Descriptors | lines in Tx FIFO | Minimal descriptors to read |\n")
        dataToPrint=dataToPrint..("----------|-------------|------------------|-----------------------------|\n")
        dataToPrint=dataToPrint..(chunk.."\n")        
    end
            


    chunk=""
    counter=0
    temp=5  --number of enteries in a row

    chunk=chunk.."\nStatus of TXDMA burst limit thresholds use: [Port: Status]\n"
    for portNum = 0,maxPort do
        --[[ skip not existed ports ]]
        if does_port_exist(devNum, portNum) then
            ret,val = myGenWrapper("cpssDxChPortTxdmaBurstLimitEnableGet",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                {"OUT","GT_BOOL","enablePtr"}
                })   
            if  0 == ret then
                if 0 == counter % temp and 0~=portNum then
                    chunk=chunk.."\n"
                end    
                chunk=chunk..(string.format("[%"..portDigits.."d:%-8s] ",portNum,booleanToStatus(val["enablePtr"])))
                counter=counter+1
            else
                if NOT_APPLICABLE == ret then
                    chunk=chunk.."N/A" 
                    break
                else
                    errors=errors.."cpssDxChPortTxdmaBurstLimitEnableGet failed: "..returnCodes[ret].."\n"
                end
            end
        end    
    end
    if ""~=chunk then
        
        dataToPrint=dataToPrint..(chunk.."\n\n")
    end
    
    
    chunk=""
    counter=0
    temp=4  --number of enteries in a row
    chunk=chunk.."TXDMA burst limit thresholds: \n[Port: Almost full threshold in Bytes , Full threshold in Bytes]\n"
    for portNum = 0,maxPort do
        --[[ skip not existed ports ]]
        if does_port_exist(devNum, portNum) then
            ret,val = myGenWrapper("cpssDxChPortTxdmaBurstLimitThresholdsGet",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                {"OUT","GT_U32","almostFullThresholdPtr"},
                {"OUT","GT_U32","fullThresholdPtr"}
                })  
            if  0 == ret then
                if 0 == counter % temp and 0~=portNum and 2 == portDigits then
                    chunk=chunk
                end    
                chunk=chunk..(string.format("[%"..portDigits.."d:%-6d,%-6d] ",portNum,val["almostFullThresholdPtr"],val["fullThresholdPtr"]))
                counter=counter+1
            else
                if NOT_APPLICABLE == ret then
                    chunk=chunk.."N/A" 
                    break
                else
                    errors=errors.."cpssDxChPortTxdmaBurstLimitEnableGet failed: "..returnCodes[ret].."\n"
                end
            end
        end    
    end
    if ""~=chunk then
        dataToPrint=dataToPrint..(chunk.."\n\n")
    end
    
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
CLI_addCommand("exec", "show buffer-management ", {
  func   = buffer_management_dump,
  help   = "The dump of all Buffer Management related information",
  params={{type= "named", {format="device %devID", name="device", help="ID of the device"},
                          mandatory = {"device"}}
  }
})