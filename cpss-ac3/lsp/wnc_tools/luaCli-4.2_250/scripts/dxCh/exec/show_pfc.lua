--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_pfc.lua
--*
--* DESCRIPTION:  show pfc dump 
--*       
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants
---
--  getPFCConfiguration
--        @description  show pfc dump file 
--
--        @param params             -params["device"]: specific device number 
--
--
--        @return       true on success, otherwise false and error message
--

local function getPFCConfiguration(params)
    local devNum=params["device"]
    local notApplicableMsg="N/A"
    local NOT_APPLICABLE=30
    local temp1, temp2 ,temp3, flag, chunk, chunk2, counter
    local dataToPrint=""
    local errors=""
    local ret, val
    
    --local portgroups=luaCLI_getPortGroups(devNum)
    local maxPort
    --getting max port for device
    ret, maxPort = wrLogWrapper("wrlCpssDevicePortNumberGet","(devNum)",devNum)
    if 0 ~= ret then
        errors=errors.."Couldn't get number of ports for device"..devNum..": "..returnCodes[ret]" \n"
        maxPort=-1
    end
    
    local portDigits=2
    if maxPort>99 then
        portDigits=3
    end


    
    ret,val = myGenWrapper("cpssDxChPortBuffersModeGet",{
            {"IN","GT_U8","devNum",devNum},
            {"OUT","CPSS_DXCH_PORT_BUFFERS_MODE_ENT","bufferModePtr"}
            })
    if 0 == ret then
        if ("CPSS_DXCH_PORT_BUFFERS_MODE_DIVIDED_E" == val["bufferModePtr"]) then
            dataToPrint=dataToPrint..("BuffersMode[Divided]\n")
        else
            dataToPrint=dataToPrint..("BuffersMode[Shared]\n")
        end
    else 
        errors=errors.."cpssDxChPortBuffersModeGet failed: "..returnCodes[ret].."\n"
    end

    
    --[[TailDropUcEnable=1]]
    ret,val = myGenWrapper("cpssDxChPortTxTailDropUcEnableGet",{
        {"IN","GT_U8","devNum",devNum},
        {"OUT","GT_BOOL","enablePtr"}
        })

    if 0 == ret then
            dataToPrint=dataToPrint..("TailDropUcEnable="..tostring(val["enablePtr"]).."\n")
    else
        errors=errors.."cpssDxChPortTxTailDropUcEnableGet failed: "..returnCodes[ret].."\n"
    end


    
      
    --[[TxShareingGlobalEnable=1 ]]
    ret,val = myGenWrapper("cpssDxChPortTxSharingGlobalResourceEnableGet",{
        {"IN","GT_U8","devNum",devNum},
        {"OUT","GT_BOOL","enablePtr"}
        })
    if 0 == ret then
        dataToPrint=dataToPrint..("TxShareingGlobalEnable="..tostring(val["enablePtr"]).."\n")
    else
        if NOT_APPLICABLE==ret then
            dataToPrint=dataToPrint..("TxShareingGlobalEnable="..notApplicableMsg.."\n")
        else   
            errors=errors.."cpssDxChPortTxSharingGlobalResourceEnableGet failed: "..returnCodes[ret].."\n"
        end    
    end
   
    
    
    
    --[[TxDp1SharedEnable=1]]
    ret,val = myGenWrapper("cpssDxChPortTxDp1SharedEnableGet",{
        {"IN","GT_U8","devNum",devNum},
        {"OUT","GT_BOOL","enablePtr"}
        })  
    if 0 == ret then
        dataToPrint=dataToPrint..("TxDp1SharedEnable="..tostring(val["enablePtr"]).."\n")
    else
        if NOT_APPLICABLE==ret then
            dataToPrint=dataToPrint..("TxDp1SharedEnable="..notApplicableMsg.."\n")
        else   
            errors=errors.."cpssDxChPortTxDp1SharedEnableGet failed: "..returnCodes[ret].."\n"
        end
    end
      
      
      
    --[[TxSharedPolicy=Constrained]]
    ret,val = myGenWrapper("cpssDxChPortTxSharedPolicyGet",{
        {"IN","GT_U8","devNum",devNum},
        {"OUT","CPSS_DXCH_PORT_TX_SHARED_POLICY_ENT","policyPtr"}
        })  
    if 0 == ret then
        if "CPSS_DXCH_PORT_TX_SHARED_POLICY_UNCONSTRAINED_E" == val["policyPtr"] then
            dataToPrint=dataToPrint..("TxSharedPolicy=UnConstrained".."\n")
        else
            dataToPrint=dataToPrint..("TxSharedPolicy=Constrained".."\n")
        end
    else
        if NOT_APPLICABLE==ret then
            dataToPrint=dataToPrint..("TxSharedPolicy="..notApplicableMsg.."\n")
        else   
            errors=errors.."cpssDxChPortTxSharedPolicyGet failed: "..returnCodes[ret].."\n"
        end
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
        dataToPrint=dataToPrint..("\nGlobal FC Config:\nxOff="..temp1 ..", xOn="..temp2.."\n")
    end

    
    local enum={
            "CPSS_DXCH_PORT_GROUP_GIGA_E",
            "CPSS_DXCH_PORT_GROUP_HGS_E"
         }
    --[[ 
    PortGroup Config:
    PortGroup[0]: limit=2016, off=1208, xon=1088
    PortGroup[1]: limit=2032, off=2040, xon=2040
    ]]
    dataToPrint=dataToPrint..("\nPortGroup Config:\n")  --[[ assuming all devices supports these APIs ]]
    for portgroup=1,2 do
        local portgroupType=enum[portgroup]
        ret,val = myGenWrapper("cpssDxChPortGroupRxBufLimitGet",{
            {"IN","GT_U8","devNum",devNum},
            {"IN","CPSS_DXCH_PORT_GROUP_ENT","portGroup",portgroupType},
            {"OUT","GT_U32","rxBufLimitPtr"}
            })
        if 0 == ret then
            temp1=val
            ret,val = myGenWrapper("cpssDxChPortGroupXoffLimitGet",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","CPSS_DXCH_PORT_GROUP_ENT","portGroup",portgroupType},
                {"OUT","GT_U32","xoffLimitPtr"}
                })
            if 0 == ret then  
                temp2=val
                ret,val = myGenWrapper("cpssDxChPortGroupXonLimitGet",{
                    {"IN","GT_U8","devNum",devNum},
                    {"IN","CPSS_DXCH_PORT_GROUP_ENT","portGroup",portgroupType},
                    {"OUT","GT_U32","xonLimitPtr"}
                    })
                if 0 == ret then                 
                    dataToPrint=dataToPrint..(string.format("PortGroup[%-27s]: limit=[%4s], off=[%4s], xon=[%4s]\n" ,portgroupType,temp1["rxBufLimitPtr"],temp2["xoffLimitPtr"],val["xonLimitPtr"]))
                else
                    errors=errors.."cpssDxChPortGroupXonLimitGet portgroup "..portgroupType.." failed: "..returnCodes[ret].."\n"
                end
            else
                errors=errors.."cpssDxChPortGroupXoffLimitGet portgroup "..portgroupType.." failed: "..returnCodes[ret].."\n"
            end
        else
            errors=errors.."cpssDxChPortGroupRxBufLimitGet portgroup "..portgroupType.." failed: "..returnCodes[ret].."\n"
        end
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
        flag=true
           

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
    
    
    
    
    dataToPrint=dataToPrint..("\n=========================================================================\n\n");

    --[[
    Port<->rx Profile Binding:
    [00:00] [01:00] [02:00] [03:00] [04:00] [05:00] [06:00] [07:00] [08:00] [09:00] [10:00] [11:00] [12:-1] [13:-1] [14:-1] [15:-1] 
    [16:00] [17:00] [18:00] [19:00] [20:00] [21:00] [22:00] [23:00] [24:00] [25:00] [26:00] [27:00] [28:-1] [29:-1] [30:-1] [31:-1] 
    [32:00] [33:00] [34:00] [35:00] [36:00] [37:00] [38:00] [39:00] [40:00] [41:00] [42:00] [43:00] [44:-1] [45:-1] [46:-1] [47:-1] 
    [48:00] [49:00] [50:00] [51:00] [52:00] [53:00] [54:00] [55:00] [56:00] [57:00] [58:00] [59:00] [60:-1] [61:-1] [62:-1] [63:02] 
    ]] 
    chunk=""
    counter=0
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
    CPSS_PORT_FLOW_CONTROL_DISABLE_E = GT_FALSE,
    CPSS_PORT_FLOW_CONTROL_RX_TX_E = GT_TRUE,
    CPSS_PORT_FLOW_CONTROL_RX_ONLY_E,  
    CPSS_PORT_FLOW_CONTROL_TX_ONLY_E
]]
    enum={      CPSS_PORT_FLOW_CONTROL_DISABLE_E = 0,
                CPSS_PORT_FLOW_CONTROL_RX_TX_E=1,
                CPSS_PORT_FLOW_CONTROL_RX_ONLY_E=2,  
                CPSS_PORT_FLOW_CONTROL_TX_ONLY_E=3}
    chunk=""
    counter=0        
    dataToPrint=dataToPrint..("\nPort FC state [DISABLE,RX_TX,RX,TX]\n")
    for portNum = 0,maxPort do

        --[[ skip not existed ports ]]
        if does_port_exist(devNum, portNum) then

            ret,val = myGenWrapper("cpssDxChPortFlowControlEnableGet",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                {"OUT","CPSS_PORT_FLOW_CONTROL_ENT","statePtr"}
                })          
        
        
        
        
            --ret = cpssDxChPortFlowControlEnableGet(devNum, portNum, &FCstate);
            if(ret ~= 0) then
                if NOT_APPLICABLE==ret then
                    dataToPrint=dataToPrint..(notApplicableMsg).."\n"         
                    break
                end
                errors=errors.."cpssDxChPortFlowControlEnableGet port"..portNum.." failed: "..returnCodes[ret].."\n"            
            else  
                if counter %9 == 0 and 0~=portnum then
                    chunk=chunk.."\n"
                end    
                chunk=chunk..string.format("[%"..portDigits.."d:%1d] ",
                                portNum,
                                enum[val["statePtr"]])
                counter=counter+1
            end
        end    
    end
    if ""~=chunk then

        dataToPrint=dataToPrint..(chunk.."\n\n")
    end
    

    
    ret,val = myGenWrapper("cpssDxChPortPfcEnableGet",{--[[ 0=triger 1=triger and response ]]
        {"IN","GT_U8","devNum",devNum},
        {"OUT","CPSS_DXCH_PORT_PFC_ENABLE_ENT","pfcEnablePtr"}
        })          
    if 0==ret then 
        dataToPrint=dataToPrint..("\n\npfcEnable [trigger, trigger and response]= "..val["pfcEnablePtr"].."\n")
    else
        if NOT_APPLICABLE==ret then
            dataToPrint=dataToPrint..("\n\npfcEnable [trigger, trigger and response]= "..notApplicableMsg.."\n")
        else        
            errors=errors.."cpssDxChPortPfcEnableGet failed: "..returnCodes[ret].."\n"             
        end
    end

 

 
    ret,val = myGenWrapper("cpssDxChPortPfcCountingModeGet",{--[[ 0=buffer 1=packet ]]
        {"IN","GT_U8","devNum",devNum},
        {"OUT","CPSS_DXCH_PORT_PFC_COUNT_MODE_ENT","pfcCountModePtr"}
        })           
    if 0 == ret then 
        if "CPSS_DXCH_PORT_PFC_COUNT_BUFFERS_MODE_E" == val["pfcCountModePtr"] then
            dataToPrint=dataToPrint..("pfcCountMode: [Buffer]\n")
        else
            dataToPrint=dataToPrint..("pfcCountMode: [Packet]\n")
        end
    else
        if NOT_APPLICABLE==ret then
            dataToPrint=dataToPrint..("pfcCountMode: "..notApplicableMsg.."\n")
        else   
            errors=errors.."cpssDxChPortPfcCountingModeGet failed: "..returnCodes[ret].."\n"
        end               
    end
    

    
    ret,val = myGenWrapper("cpssDxChPortPfcGlobalDropEnableGet",{
        {"IN","GT_U8","devNum",devNum},
        {"OUT","GT_BOOL","enablePtr"}
        })            
    if 0 == ret then 
        dataToPrint=dataToPrint..("enablePFCGlobalDrop = "..tostring(val["enablePtr"]).."\n")
    else
        if NOT_APPLICABLE==ret then
            dataToPrint=dataToPrint..("enablePFCGlobalDrop = "..notApplicableMsg.."\n")
        else   
            errors=errors.."cpssDxChPortPfcGlobalDropEnableGet failed: "..returnCodes[ret].."\n"
        end              
    end

    --[[
    GlobalQueue Config:
       Tc limit  xoff
        0  1984  2047
        1   828  2047
        2   828  2047
        3   828  2047
        4   828  2047
        5   828  2047
        6   828  2047
        7   828  2047
    ]]
    dataToPrint=dataToPrint..("\nPFC GlobalQueue Config:\n")
    for tcQueue = 0,7 do
        ret,val = myGenWrapper("cpssDxChPortPfcGlobalQueueConfigGet",{
            {"IN","GT_U8","devNum",devNum},
            {"IN","GT_U8","tcQueue",tcQueue},
            {"OUT","GT_U32","xoffThresholdPtr"},
            {"OUT","GT_U32","dropThresholdPtr"}
            })          
    
        if 0 == ret then
            if 0 == tcQueue then
                dataToPrint=dataToPrint..("Tc |  limit |  xOff  |\n")
                dataToPrint=dataToPrint..("---|--------|--------|\n")
            end
            dataToPrint=dataToPrint..(string.format("%-3d|%6d  |%6d  |\n",tcQueue,val["dropThresholdPtr"],val["xoffThresholdPtr"]))
        else 
            if NOT_APPLICABLE==ret then
                dataToPrint=dataToPrint..(notApplicableMsg).."\n"
                break  --if device not applicable break
            end
            errors=errors.."cpssDxChPortPfcGlobalQueueConfigGet failed: "..returnCodes[ret].."\n"
            
        end
    end

    --[[
    Port enableFWD PFC:
    [00:00] [01:00] [02:00] [03:00] [04:00] [05:00] [06:00] [07:00] [08:00] [09:00] [10:00] [11:00] [12:-1] [13:-1] [14:-1] [15:-1] 
    ]]
    chunk=""
    counter=0
    if 2==portDigits then
        temp1=7
    else
        temp1=6
    end
    dataToPrint=dataToPrint..("\nPort enableFWD PFC:\n")
    for portNum = 0,maxPort do
        --[[ skip not existed ports ]]
        if does_port_exist(devNum, portNum) then
            ret,val = myGenWrapper("cpssDxChPortPfcForwardEnableGet",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                {"OUT","GT_BOOL","enablePtr"}
                })            
            if  0 == ret then
                if 0 == counter % temp1 and 0~=portNum then
                    chunk=chunk.."\n"
                end    
                chunk=chunk..(string.format("[%"..portDigits.."d:%-5s] ",portNum,tostring(val["enablePtr"])))
                counter=counter+1
            else  
                if NOT_APPLICABLE==ret then
                    dataToPrint=dataToPrint..(notApplicableMsg).."\n"
                    break  --if device not applicable break
                end
                errors=errors.."cpssDxChPortPfcForwardEnableGet failed: "..returnCodes[ret].."\n"                
            end
        end    
    end
    if ""~=chunk then
        
        dataToPrint=dataToPrint..(chunk.."\n\n")
    end
    
    
    

    --[[
    Port FC mode:
    [00:00] [01:00] [02:00] [03:00] [04:00] [05:00] [06:00] [07:00] [08:00] [09:00] [10:00] [11:00] [12:-1] [13:-1] [14:-1] [15:-1] 
    ]]
    chunk=""
    counter=0
    enum={      CPSS_DXCH_PORT_FC_MODE_802_3X_E=0,
                CPSS_DXCH_PORT_FC_MODE_PFC_E=1,
                CPSS_DXCH_PORT_FC_MODE_LL_FC_E=2}
    dataToPrint=dataToPrint..("\nPort FC mode: [802.3x,PFC,LLC]\n")
    for portNum = 0,maxPort do
        --[[ skip not existed ports ]]
        if does_port_exist(devNum, portNum) then
            ret,val = myGenWrapper("cpssDxChPortFlowControlModeGet",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                {"OUT","CPSS_DXCH_PORT_FC_MODE_ENT","fcModePtr"}
                })
            if  0 == ret then 
                if 0 == counter % 9 and 0~=portNum then
                    chunk=chunk.."\n"
                end    
                chunk=chunk..string.format("[%"..portDigits.."d:%1d] ",portNum,enum[val["fcModePtr"]])
                counter=counter+1
            else

                if NOT_APPLICABLE==ret then
                    dataToPrint=dataToPrint..(notApplicableMsg).."\n"
                    break  --if device not applicable break
                end
                errors=errors.."cpssDxChPortFlowControlModeGet failed: "..returnCodes[ret].."\n"                
            end
        end
    end
    if ""~=chunk then
        
        dataToPrint=dataToPrint..(chunk.."\n\n")
    end

    --[[
    Port<->PfcProfile Binding:
    [00:00] [01:00] [02:00] [03:00] [04:00] [05:00] [06:00] [07:00] [08:00] [09:00] [10:00] [11:00] [12:-1] [13:-1] [14:-1] [15:-1] 
    ]]
    chunk=""
    counter=0
    dataToPrint=dataToPrint..("\nPort<->PfcProfile Binding: [port:Profile]\n")
    for portNum = 0,maxPort do
        --[[ skip not existed ports ]]
        if does_port_exist(devNum, portNum) then
            ret,val = myGenWrapper("cpssDxChPortPfcProfileIndexGet",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                {"OUT","GT_U32","profileIndexPtr"}
                })        
        

            if  0 == ret then
                if 0 == counter % 9 and 0~=portNum then
                    chunk=chunk.."\n"
                end    
                chunk=chunk..string.format("[%"..portDigits.."d:%1d] ",portNum,val["profileIndexPtr"])
                counter=counter+1                
            else 
                if NOT_APPLICABLE==ret then
                    dataToPrint=dataToPrint..(notApplicableMsg).."\n"
                    break  --if device not applicable break
                end
                errors=errors.."cpssDxChPortPfcProfileIndexGet failed: "..returnCodes[ret].."\n"

            end
        end
    end
    if ""~=chunk then
        dataToPrint=dataToPrint..(chunk.."\n\n")
    end    
    
    
   --[[for(portNum = 0; portNum < PRV_CPSS_PP_MAC(devNum)->numOfPorts; portNum++)
    {
        /* skip not existed ports */
        if (! PRV_CPSS_PHY_PORT_IS_EXIST_MAC(devNum, portNum))
            continue;
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
        if(portNum == 0)
        {
            cpssOsPrintf("\n\nPort FC DSA TAG [port: MACTYPE [NOT_EXISTS_E,FE,GE,XG,XLG,HGL,CG,ILKN,NA] [FC_DSATag2|FC_DSATag3]\n");
            cpssOsPrintf("[Port:MAC_Type][FC_DSATag2|FC_DSATag3]\n");
        }
        for(portMacType = PRV_CPSS_PORT_GE_E; portMacType < PRV_CPSS_PORT_NOT_APPLICABLE_E; portMacType++)
        {
            regAddr_FC0 = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                          macRegs.perPortRegs[portNum].macRegsPerType[portMacType].fcDsaTag2;
    
            if(regAddr_FC0 != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                rc =  prvCpssHwPpPortGroupReadRegBitMask(devNum,portGroupId, regAddr_FC0, 0xffffffff,&FC_DSA0);
                if (rc != GT_OK)
                {
                    cpssOsPrintf("prvCpssHwPpPortGroupReadRegBitMask fails rc=%x ",rc);
                }
            }
    
            regAddr_FC1 = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                          macRegs.perPortRegs[portNum].macRegsPerType[portMacType].fcDsaTag3;

            if(regAddr_FC1 != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                rc =  prvCpssHwPpPortGroupReadRegBitMask(devNum,portGroupId, regAddr_FC1, 0xffffffff,&FC_DSA1);
                if (rc != GT_OK)
                {
                    cpssOsPrintf("prvCpssHwPpPortGroupReadRegBitMask fails rc=%x ",rc);
                }
            }
            cpssOsPrintf("[%3d:%2d] [0x%4x | 0x%4x]",portNum,portMacType,FC_DSA0,FC_DSA1);
        }
        cpssOsPrintf("\n");
    }
    cpssOsPrintf("\n");]]
    
    

--[[
Port PfcProfile Tc Config:
PfcProfile[0]: |PfcProfile[1]: |PfcProfile[2]: |PfcProfile[3]: |PfcProfile[4]: |PfcProfile[5]: |PfcProfile[6]: |PfcProfile[7]: |
  Tc xoff  xon |  Tc xoff  xon |  Tc xoff  xon |  Tc xoff  xon |  Tc xoff  xon |  Tc xoff  xon |  Tc xoff  xon |  Tc xoff  xon |
   0   42   24 |   0    0    0 |   0    0    0 |   0    0    0 |   0    0    0 |   0    0    0 |   0    0    0 |   0    0    0 |
   1 2047 2047 |   1    0    0 |   1    0    0 |   1    0    0 |   1    0    0 |   1    0    0 |   1    0    0 |   1    0    0 |
   2 2047 2047 |   2    0    0 |   2    0    0 |   2    0    0 |   2    0    0 |   2    0    0 |   2    0    0 |   2    0    0 |
   3 2047 2047 |   3    0    0 |   3    0    0 |   3    0    0 |   3    0    0 |   3    0    0 |   3    0    0 |   3    0    0 |
   4 2047 2047 |   4    0    0 |   4    0    0 |   4    0    0 |   4    0    0 |   4    0    0 |   4    0    0 |   4    0    0 |
   5 2047 2047 |   5    0    0 |   5    0    0 |   5    0    0 |   5    0    0 |   5    0    0 |   5    0    0 |   5    0    0 |
   6 2047 2047 |   6    0    0 |   6    0    0 |   6    0    0 |   6    0    0 |   6    0    0 |   6    0    0 |   6    0    0 |
   7 2047 2047 |   7    0    0 |   7    0    0 |   7    0    0 |   7    0    0 |   7    0    0 |   7    0    0 |   7    0    0 |
===========================================================
    ]]

    chunk=""
    chunk2=""
    dataToPrint=dataToPrint..("\nPort PfcProfile Tc Config:\n");

    flag=true
    for tcQueue = 0,7 do

        for profileIndex = 0,7 do
            ret,val = myGenWrapper("cpssDxChPortPfcProfileQueueConfigGet",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_U32","profileIndex",profileIndex},
                {"IN","GT_U8","tcQueue",tcQueue},
                {"OUT","CPSS_DXCH_PORT_PFC_PROFILE_CONFIG_STC","pfcProfileCfgPtr"}
                })        
            if  0 == ret then 
                if 4>profileIndex then
                    chunk=chunk..string.format("%3d %4d %4d |",tcQueue,val["pfcProfileCfgPtr"]["xoffThreshold"],val["pfcProfileCfgPtr"]["xonThreshold"])
                else
                    chunk2=chunk2..string.format("%3d %4d %4d |",tcQueue,val["pfcProfileCfgPtr"]["xoffThreshold"],val["pfcProfileCfgPtr"]["xonThreshold"])
                end
            else  
                if NOT_APPLICABLE == ret then
                    dataToPrint=dataToPrint..(notApplicableMsg).."\n"
                    flag=false
                    break
                end
                errors=errors.."cpssDxChPortPfcProfileQueueConfigGet failed: "..returnCodes[ret].."\n"
            end
        end
        if false==flag then
            break
        end        
        chunk=chunk.."\n"
        chunk2=chunk2.."\n"        
    end
    
    if ""~=chunk and ""~=chunk2 then
        dataToPrint=dataToPrint..("PfcProfile[0] |PfcProfile[1] |PfcProfile[2] |PfcProfile[3] \n")
        dataToPrint=dataToPrint..(" Tc xoff  xon | Tc xoff  xon | Tc xoff  xon | Tc xoff  xon \n")
        dataToPrint=dataToPrint..(chunk.."\n\n")
        dataToPrint=dataToPrint..("PfcProfile[4] |PfcProfile[5] |PfcProfile[6] |PfcProfile[7] |\n")
        dataToPrint=dataToPrint..(" Tc xoff  xon | Tc xoff  xon | Tc xoff  xon | Tc xoff  xon |\n")
        dataToPrint=dataToPrint..(chunk2.."\n\n")
    
        
    end    
    
    dataToPrint=dataToPrint..("\n=========================================================================\n\n")
 
 
 

    --[[
    Port schedulerProfileSet Config:
    PfcProfile[0]: |PfcProfile[1]: |PfcProfile[2]: |PfcProfile[3]: |PfcProfile[4]: |PfcProfile[5]: |PfcProfile[6]: |PfcProfile[7]: |
        Tc   Ratio |    Tc   Ratio |    Tc   Ratio |    Tc   Ratio |    Tc   Ratio |    Tc   Ratio |    Tc   Ratio |    Tc   Ratio |
         0   200   |     0    xxxx |     0         |     0         |     0         |     0         |     0         |     0         |
    ]]
    chunk=""
    chunk2=""
    flag=true
    dataToPrint=dataToPrint..("\nPort schedulerProfileSet Config:\n");

    for tcQueue = 0,7 do
        for schedulerProfileSet = 0,7 do
            ret,val = myGenWrapper("cpssDxChPortPfcShaperToPortRateRatioGet",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT","profileSet",schedulerProfileSet},
                {"IN","GT_U8","tcQueue",tcQueue},
                {"OUT","GT_U32","shaperToPortRateRatioPtr"}
                })                
            if  0 == ret then 
                if 4>schedulerProfileSet then
                    chunk=chunk..string.format("%-6d %-6d|",tcQueue,val["shaperToPortRateRatioPtr"])
                else
                    chunk2=chunk2..string.format("%-6d %-6d|",tcQueue,val["shaperToPortRateRatioPtr"])
                end
            else  
                if NOT_APPLICABLE == ret then
                    dataToPrint=dataToPrint..(notApplicableMsg).."\n"
                    flag=false
                    break
                end
                errors=errors.."cpssDxChPortPfcShaperToPortRateRatioGet failed: "..returnCodes[ret].."\n"
            end
        end
        if false==flag then
            break
        end        
        chunk=chunk.."\n"
        chunk2=chunk2.."\n"      
    end
    
    if ""~=chunk and ""~=chunk2 then
        dataToPrint=dataToPrint..("PfcProfile[0]|PfcProfile[1]|PfcProfile[2]|PfcProfile[3]|\n")
        dataToPrint=dataToPrint..("Tc     Ratio |Tc     Ratio |Tc     Ratio |Tc     Ratio |\n")
        dataToPrint=dataToPrint..("-------------|-------------|-------------|-------------|\n")
        dataToPrint=dataToPrint..(chunk.."\n\n")
        dataToPrint=dataToPrint..("PfcProfile[4]|PfcProfile[5]|PfcProfile[6]|PfcProfile[7]|\n")
        dataToPrint=dataToPrint..("Tc     Ratio |Tc     Ratio |Tc     Ratio |Tc     Ratio |\n")
        dataToPrint=dataToPrint..("-------------|-------------|-------------|-------------|\n")
        dataToPrint=dataToPrint..(chunk2.."\n\n")
    
        
    end    
    
    
    
    dataToPrint=dataToPrint..("\n=========================================================================\n\n")
 



    dataToPrint=dataToPrint..("PFC Timer Map status:\n")

    for schedulerProfileSet = 0,7 do
        ret,val = myGenWrapper("cpssDxChPortPfcTimerMapEnableGet",{
            {"IN","GT_U8","devNum",devNum},
            {"IN","CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT","profileSet",schedulerProfileSet},
            {"OUT","GT_U32","shaperToPortRateRatioPtr"}
            })      

        if  0 == ret then 
            if(schedulerProfileSet==0) then
                dataToPrint=dataToPrint..("Timer | Map enable/disabled|\n")
                dataToPrint=dataToPrint..("------|--------------------|\n")
            end
            if true==val["enablePtr"] then
                temp1="enabled"
            else
                temp1="disabled"
            end
            dataToPrint=dataToPrint..(string.format("%3d   |%-20s|\n",
                                    schedulerProfileSet,
                                    alignLeftToCenterStr(tostring(temp1),20)))
        else  
            if NOT_APPLICABLE == ret then
                dataToPrint=dataToPrint..(notApplicableMsg).."\n"
                break
            end
            errors=errors.."cpssDxChPortPfcTimerMapEnableGet failed: "..returnCodes[ret].."\n"
        end
    end
    dataToPrint=dataToPrint..("\n=========================================================================\n\n")


    chunk=""
    dataToPrint=dataToPrint..("PFC Timer to Queue Mapping: [Timer:TC]\n")
    for pfcTimer = 0,7 do
        ret,val = myGenWrapper("cpssDxChPortPfcTimerToQueueMapGet",{
            {"IN","GT_U8","devNum",devNum},
            {"IN","GT_U32","pfcTimer",pfcTimer},
            {"OUT","GT_U32","tcQueuePtr"}
            })      
    
        if  0 == ret then 
            chunk=chunk..(string.format("[%1d:%1d]",pfcTimer,val["tcQueuePtr"]))
            if 7~=pfcTimer then
                chunk=chunk.." | "
            end
        else  
            if NOT_APPLICABLE == ret then
                dataToPrint=dataToPrint..(notApplicableMsg).."\n"
                break
            end
            errors=errors.."cpssDxChPortPfcTimerToQueueMapGet failed: "..returnCodes[ret].."\n"
        end
    end
    if ""~=chunk then
        dataToPrint=dataToPrint..(chunk).."\n"
    end
    
    dataToPrint=dataToPrint..("\n=========================================================================\n\n")

    --[[GT_BOOL toCpuLossyDropEnable;
    GT_BOOL toTargetSnifferLossyDropEnable;
    GT_BOOL fromCpuLossyDropEnable;]]
    
    
    ret,val = myGenWrapper("cpssDxChPortPfcLossyDropConfigGet",{
        {"IN","GT_U8","devNum",devNum},
        {"OUT","CPSS_DXCH_PORT_PFC_LOSSY_DROP_CONFIG_STC","lossyDropConfigPtr"}
        })      
    
    if  0 == ret then 
        dataToPrint=dataToPrint..(string.format("toCpuLossyDropEnable [%s] \ntoTargetSnifferLossyDropEnable [%s] \nfromCpuLossyDropEnable [%s]\n",
                     tostring(val["lossyDropConfigPtr"]["fromCpuLossyDropEnable"]), tostring(val["lossyDropConfigPtr"]["toCpuLossyDropEnable"]), tostring(val["lossyDropConfigPtr"]["toTargetSnifferLossyDropEnable"])))
    else  
        if NOT_APPLICABLE == ret then
            dataToPrint=dataToPrint..(string.format("toCpuLossyDropEnable [%s] \ntoTargetSnifferLossyDropEnable [%s] \nfromCpuLossyDropEnable [%s]\n",
                 notApplicableMsg, notApplicableMsg, notApplicableMsg))
        end
    end

    dataToPrint=dataToPrint.."\n"
    for tcQueue = 0,7 do
        ret,val = myGenWrapper("cpssDxChPortPfcLossyDropQueueEnableGet",{
            {"IN","GT_U8","devNum",devNum},
            {"IN","GT_U8","tcQueue",tcQueue},
            {"OUT","GT_BOOL","enablePtr"}
            })      
    
        if  0 == ret then 
            if(tcQueue == 0) then
                dataToPrint=dataToPrint..("tcQueue |  Status  |\n")
                dataToPrint=dataToPrint..("--------|----------|\n")
            end
            if true==val["enablePtr"] then
                temp1="enabled"
            else
                temp1="disabled"
            end
            
            
            dataToPrint=dataToPrint..(string.format("%-8s|%-10s|\n",
                                alignLeftToCenterStr(tostring(tcQueue),8),
                                alignLeftToCenterStr(tostring(temp1),10)))
        else  
            if NOT_APPLICABLE == ret then
                dataToPrint=dataToPrint..("tcQueue | enabled/disabled: "..notApplicableMsg.."\n")
                break
            end
            errors=errors.."cpssDxChPortPfcLossyDropQueueEnableGet failed: "..returnCodes[ret].."\n"
        end
    end
    dataToPrint=dataToPrint..("\n=========================================================================\n\n");

    --[[
    0x11003008 + n*0x4: where n (0-7) represents profile [23:0] Profile <%n> LLFC_XOFF_Value
    The value written to the Shaper Token Bucket counter when a LL-FC XOFF indication is received.
    The configured value must be lower than the Shaper's <Token Bucket Base Line> threshold.
    When triggered, the queue is paused for <Token Bucket Base Line> - <LLFC_XOFF_Value> Byte times, 
        where a Byte time is defined by the queue shaper configured rate.
    Note: The LL_FC XON indication sets the the shaper token bucket counter to <Token Bucket Base Line>, 
    so transmission can immediately be resumed.
    TxQ differentiates between LL-FC and PFC indications according to a command associated with every 
    transaction from TTI. The TTI differentiates between PFC and LL-FC according to different OpCode.
    ]]
 --[[
    print("PFC Timer Map enable:\n");
    print("Scheduler Profile | LLF_XOFF_Value:\n");
    for(schedulerProfileSet=0 ; schedulerProfileSet<8;schedulerProfileSet++) 
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                    txqVer1.dq.flowControl.schedulerProfileLlfcMtu[schedulerProfileSet];
        --print("Scheduler Profile | LLF_XOFF_Value: addres = %8x\n",regAddr);
        ret =  prvCpssHwPpGetRegField(devNum, regAddr, 0, 23,&LLF_XOFF_Value);
        if (ret ~= 0)
        {
            return ret;
        }

        print("%10d        |%10d\n",schedulerProfileSet,LLF_XOFF_Value);
    }
    return 0;
}]]

    if ""~=errors then
        dataToPrint=dataToPrint..("Errors:\n-------\n"..errors.."\n\n")
    end
    print(dataToPrint)
end


--------------------------------------------------------------------------------
-- command registration: counters show
--------------------------------------------------------------------------------
--CLI_addHelp("exec", "show counters", "Display counters")
CLI_addCommand("exec", "show pfc ", {
  func   = getPFCConfiguration,
  help   = 'The dump of all PFC related information',
  params={{type= "named", {format="device %devID", name="device", help="ID of the device"},
                          mandatory = {"device"}}
  }
})