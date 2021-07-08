--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* packet-trace.lua
--*
--* DESCRIPTION:  configure and show counters for 2 ports or for vlan
--*       
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************
    
--includes

--constants


local function counters_configure(params)
    local temp= getGlobal("ifRange")  --get table of ports
    local device, ports=pairs(temp)(temp)
    local egressMode, ingressMode,dropMode
    local ingress_counter_enum, egress_counter_num
    local src=false
    local dst=false
    local ret, val

    if #ports~=1 then  --checks that only one port is member in the interface
        print("Only one port can be member in the interface in order to perform this command")
    else  
        if "source-port"==params["mode"] then
            src=true
            ingress_counter_enum="CPSS_DXCH_BRG_CNT_SET_ID_0_E"
            egress_counter_num=0
        else
            if "destination-port"==params["mode"] then
                dst=true
                ingress_counter_enum="CPSS_DXCH_BRG_CNT_SET_ID_1_E"
                egress_counter_num=1
            end  
        end
    
        --check if first counters configured for vlan
    
        ret,val = myGenWrapper("cpssDxChBrgCntBridgeIngressCntrModeGet",{  --get configure:
                {"IN","GT_U8","devNum",device},
                {"IN","CPSS_DXCH_BRIDGE_CNTR_SET_ID_ENT","cntrSetId","CPSS_DXCH_BRG_CNT_SET_ID_0_E"},
                {"OUT","CPSS_BRIDGE_INGR_CNTR_MODES_ENT","setModePtr"},        
                {"OUT","GT_PORT_NUM","portPtr"},
                {"OUT","GT_U16","vlanPtr"}
                })        
                
        if 0~=ret then
            print("Error at getting counters mode: "..returnCodes[ret])
        else
            if "CPSS_BRG_CNT_MODE_2_E"==val["setModePtr"] and true==dst then
                print("Source port must configured first")
            else
                local port=ports[1]
                
                
                    
                egressMode=1  --CPSS_EGRESS_CNT_PORT_E
                ingressMode="CPSS_BRG_CNT_MODE_1_E"
                dropMode="CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_PORT_E"
                
                
                        --configuring ingress counter  
                ret = myGenWrapper("cpssDxChBrgCntBridgeIngressCntrModeSet",{  --get configure:
                        {"IN","GT_U8","devNum",device},
                        {"IN","CPSS_DXCH_BRIDGE_CNTR_SET_ID_ENT","cntrSetId",ingress_counter_enum},
                        {"IN","CPSS_BRIDGE_INGR_CNTR_MODES_ENT","setMode",ingressMode},        
                        {"IN","GT_PORT_NUM","port",port},
                        {"IN","GT_U16","vlan",nil}
                        })        
                        
                if 0~=ret then  
                    print("Error at configuring ingress counter : "..returnCodes[ret])  
                end

                
                if true==src then  --configure drop counter only for source port.
                    --configuring drop counter
                    ret = myGenWrapper("cpssDxChCfgIngressDropCntrModeSet",{  
                                {"IN","GT_U8","devNum",device},
                                {"IN","CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_ENT","mode",dropMode},      
                                {"IN","GT_PORT_NUM","portNum",port},
                                {"IN","GT_U16","vlan",nil}
                                })        

                    if 0~=ret then    
                        print("Error at configuring drop counter: "..returnCodes[ret])
                    end  
                end
                
                        
                    --configuring egress counter
                ret = myGenWrapper("cpssDxChPortEgressCntrModeSet",{  
                        {"IN","GT_U8","devNum",device},
                        {"IN","GT_U8","cntrSetNum",egress_counter_num},
                        {"IN","CPSS_PORT_EGRESS_CNT_MODE_ENT","setModeBmp",egressMode},        
                        {"IN","GT_PHYSICAL_PORT_NUM","portNum",port},
                        {"IN","GT_U16","vlanId",nil},
                        {"IN","GT_U8","tc",nil},
                        {"IN","CPSS_DP_LEVEL_ENT","dpLevel",nil}
                        })    
                if 0~=ret then
                    print("Error at configuring egress counter "..returnCodes[ret])
                end          
                                
                resetCounters(device, egress_counter_num, port)
            end
        end  
    end
    
end



local function counters_configure_vlan(params)
    local temp= getGlobal("ifRange")  --get table of ports
    local device, vlan_table=pairs(temp)(temp)
    local egressMode=2
    local ingressMode="CPSS_BRG_CNT_MODE_2_E"
    local dropMode="CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_VLAN_E"
    local ingress_counter_enum="CPSS_DXCH_BRG_CNT_SET_ID_0_E"
    local egress_counter_num=0
    local ret, val
    local vlanId=vlan_table[1]  
    
        --configuring ingress counter  
    ret = myGenWrapper("cpssDxChBrgCntBridgeIngressCntrModeSet",{  --set configure:
            {"IN","GT_U8","devNum",device},
            {"IN","CPSS_DXCH_BRIDGE_CNTR_SET_ID_ENT","cntrSetId",ingress_counter_enum},
            {"IN","CPSS_BRIDGE_INGR_CNTR_MODES_ENT","setMode",ingressMode},        
            {"IN","GT_PORT_NUM","port",nil},
            {"IN","GT_U16","vlan",vlanId}
            })              
            
    if 0~=ret then  
        print("Error at configuring ingress counter : "..returnCodes[ret])  
    end

    
    --configuring drop counter
        ret = myGenWrapper("cpssDxChCfgIngressDropCntrModeSet",{  
                    {"IN","GT_U8","devNum",device},
                    {"IN","CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_ENT","mode",dropMode},      
                    {"IN","GT_PORT_NUM","portNum",nil},
                    {"IN","GT_U16","vlan",vlanId}
                    })        

    if 0~=ret then    
        print("Error at configuring drop counter: "..returnCodes[ret])
    end  

    
            
        --configuring egress counter
    ret = myGenWrapper("cpssDxChPortEgressCntrModeSet",{  
            {"IN","GT_U8","devNum",device},
            {"IN","GT_U8","cntrSetNum",egress_counter_num},
            {"IN","CPSS_PORT_EGRESS_CNT_MODE_ENT","setModeBmp",egressMode},        
            {"IN","GT_PHYSICAL_PORT_NUM","portNum",nil},
            {"IN","GT_U16","vlanId",vlanId},
            {"IN","GT_U8","tc",nil},
            {"IN","CPSS_DP_LEVEL_ENT","dpLevel",nil}
            })  
    if 0~=ret then
        print("Error at configuring egress counter "..returnCodes[ret])
    end          
                    
    resetCountersVlan(device, vlanId)

end










---
--  resetCounters
--        @description  reset the MAC, drop, egress and ingress counters of given device and port (counters set as given)
--
--        @param params             -device: device number
--                  -counterNum: number of counter set to reset (0 or 1)
--                  --port: port number
--
--        @return       none
--
function resetCounters(device, countNum, port)
    local ret, val
    local counterSet
    local errorMsg=""

    if 0==countNum then
        counterSet="CPSS_DXCH_BRG_CNT_SET_ID_0_E"
    elseif 1==countNum then
        counterSet="CPSS_DXCH_BRG_CNT_SET_ID_1_E"
    end
    
    --clear mac counters
    clear_mac_counters_func(
                            {
                              all_interfaces="ethernet",
                              ethernet={
                                [device]={
                                  port
                                }
                              }
                            }
                            )

    
    --reset ingress counters        
    ret,val = myGenWrapper("cpssDxChBrgCntBridgeIngressCntrsGet",{  --get counters:
                {"IN","GT_U8","devNum",device},
                {"IN","CPSS_DXCH_BRIDGE_CNTR_SET_ID_ENT","cntrSetId",counterSet},
                {"OUT","CPSS_BRIDGE_INGRESS_CNTR_STC","ingressCntrPtr"}
                })  
                
    if 0~=ret then    
        errorMsg=errorMsg.."Error at resetting ingress counters: "..returnCodes[ret].."\n"        
    end          

    
    if 0==countNum then  --reset drop counter
        ret = myGenWrapper("cpssDxChCfgIngressDropCntrSet",{  
                    {"IN","GT_U8","devNum",device},
                    {"IN","GT_U32","counter",0}
                    })        
        if 0~=ret then    
            errorMsg=errorMsg.."Error at resetting drop counter: "..returnCodes[ret].."\n"        
        end  
    
    end
    
    
        --reset egress counters              
    ret,val = myGenWrapper("cpssDxChPortEgressCntrsGet",{  
        {"IN","GT_U8","devNum",device},
        {"IN","GT_U8","cntrSetNum",countNum},
        {"OUT","CPSS_PORT_EGRESS_CNTR_STC","egrCntrPtr"}
        })  
    if 0~=ret then    
        errorMsg=errorMsg.."Error at resetting egress counters: "..returnCodes[ret].."\n"        
    end    
    
    
    
end



---
--  resetCountersVlan
--        @description  reset the MAC, drop, egress and ingress counters of given device and ports
--
--        @param params             -device: device number
--                  --vlan: vlan number
--
--        @return       none
--
function resetCountersVlan(device, vlan)

    local ret, val
    local errorMsg=""

    --reset all the vlan members counters  
    ret,val=vlan_info_get(device,vlan)
                
    if 0==ret then
        local portMembers=val["portsMembers"]

        --clearing mac counters for each port in the vlan
        for key,port in pairs(portMembers) do

            clear_mac_counters_func(
                                    {
                                      all_interfaces="ethernet",
                                      ethernet={
                                        [device]={
                                          port
                                        }
                                      }
                                    }
                                    )
        end  
    else 

        errorMsg=errorMsg.."Error at resetting vlan MAC counters: "..returnCodes[ret].."\n"
    end  



    
    
                        
                                    
--reset ingress counters        
    ret,val = myGenWrapper("cpssDxChBrgCntBridgeIngressCntrsGet",{  --get counters:
                {"IN","GT_U8","devNum",device},
                {"IN","CPSS_DXCH_BRIDGE_CNTR_SET_ID_ENT","cntrSetId","CPSS_DXCH_BRG_CNT_SET_ID_0_E"},
                {"OUT","CPSS_BRIDGE_INGRESS_CNTR_STC","ingressCntrPtr"}
                })  
                
    if 0~=ret then    
        errorMsg=errorMsg.."Error at resetting ingress counters: "..returnCodes[ret].."\n"        
    end          

    
    
    --reset drop counter:
    ret = myGenWrapper("cpssDxChCfgIngressDropCntrSet",{  
                {"IN","GT_U8","devNum",device},
                {"IN","GT_U32","counter",0}
                })        
    if 0~=ret then    
        errorMsg=errorMsg.."Error at resetting drop counter: "..returnCodes[ret].."\n"        
    end

    --reset egress counters              
    ret,val = myGenWrapper("cpssDxChPortEgressCntrsGet",{  
        {"IN","GT_U8","devNum",device},
        {"IN","GT_U8","cntrSetNum",0},
        {"OUT","CPSS_PORT_EGRESS_CNTR_STC","egrCntrPtr"}
        })  
    if 0~=ret then    
        errorMsg=errorMsg.."Error at resetting egress counters: "..returnCodes[ret].."\n"        
    end      
        
        
    if ""~=errorMsg then
        print(errorMsg)
    end  
end


































































--------------------------------------------------------------------------------
-- command registration: counters packet-trace
--------------------------------------------------------------------------------
CLI_addHelp({"interface", "interface_eport"}, "counters", "Configure counters")
CLI_addCommand({"interface", "interface_eport"}, "counters packet-trace",   {
  func   = counters_configure,
  help   = "Configure port for packet trace counters",
  params=  {   {type= "named", 
                   "#destOrSrcPort",
                   mandatory = { "mode" }
      }
            }})
            
            
            
--------------------------------------------------------------------------------
-- command registration: counters packet-trace
--------------------------------------------------------------------------------
CLI_addHelp("vlan_configuration", "counters", "Configure counters")
CLI_addCommand("vlan_configuration", "counters packet-trace",   {
  func   = counters_configure_vlan,
  help   = "Configure port for packet trace counters",
  params=  {
            }})
                        
            
            
            