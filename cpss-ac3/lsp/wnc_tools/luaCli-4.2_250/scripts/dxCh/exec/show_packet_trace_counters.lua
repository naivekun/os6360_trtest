--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* counters.lua
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


---
--  printLocalHeader
--        @description  prints the counters name and their description
--
--        @param params             - none
--
--        @return       none
--
local function printLocalHeader()
  local header="\n"
  header=header.."       Counter                                Description       \n"
  header=header.."---------------------- ---------------------------------------------------------\n"




  --goodOctetsRcv
  header=header..string.format("%-23s%-55s\n","goodOctetsRcv",
                            "Number of ethernet frames received that are not bad")
  header=header..string.format("%-23s%-55s\n","",
                            "ethernet frames or MAC Control pkts")


  --badOctetsRcv
  header=header..string.format("%-23s%-55s\n","badOctetsRcv",
                            "Sum of lengths of all bad ethernet frames received")

  --gtBrgInFrames
  header=header..string.format("%-23s%-55s\n","gtBrgInFrames",
                            "Number of packets received")
    
  --gtBrgVlanIngFilterDisc
  header=header..string.format("%-23s%-55s\n","gtBrgVlanIngFilterDisc",
                            "Number of packets discarded due to VLAN Ingress Filtering")

  --gtBrgSecFilterDisc    
  header=header..string.format("%-23s%-55s\n","gtBrgSecFilterDisc",
                            "Number of packets discarded due to")
  header=header..string.format("%-23s%-55s\n","",
                            "Security Filtering measures")            

  --gtBrgLocalPropDisc    
  header=header..string.format("%-23s%-55s\n","gtBrgLocalPropDisc",
                            "Number of packets discarded due to reasons other than ")
  header=header..string.format("%-23s%-55s\n","",
                            "VLAN ingress and Security filtering")

  --dropCounter    
  header=header..string.format("%-23s%-55s\n","dropCounter",
                            "Ingress Drop Counter")

  --outUcFrames    
  header=header..string.format("%-23s%-55s\n","outUcFrames",
                            "Number of unicast packets transmitted")
                            
  --outMcFrames    
  header=header..string.format("%-23s%-55s\n","outMcFrames",
                            "Number of multicast packets transmitted. This includes")
  header=header..string.format("%-23s%-55s\n","",
                            "registered multicasts, unregistered multicasts")
  header=header..string.format("%-23s%-55s\n","",
                            "and unknown unicast packets")              

  --outBcFrames    
  header=header..string.format("%-23s%-55s\n","outBcFrames",
                            "Number of broadcast packets transmitted")

  --brgEgrFilterDisc    
  header=header..string.format("%-23s%-55s\n","brgEgrFilterDisc",
                            "Number of IN packets that were Bridge Egress filtered")

  --txqFilterDisc    
  header=header..string.format("%-23s%-55s\n","txqFilterDisc",
                            "Number of IN packets that were filtered")
  header=header..string.format("%-23s%-55s\n","",
                            "due to TxQ congestion")
                            
                            
  --outCtrlFrames    
  header=header..string.format("%-23s%-55s\n","outCtrlFrames",
                            "Number of out control packets ")
  header=header..string.format("%-23s%-55s\n","",
                            "(to cpu, from cpu and to  analyzer)")

  --egrFrwDropFrames    
  header=header..string.format("%-23s%-55s\n","egrFrwDropFrames",
                            "Number of packets dropped due to egress")
  header=header..string.format("%-23s%-55s\n","",
                            "forwarding restrictions")

  --goodOctetsSent    
  header=header..string.format("%-23s%-55s\n","goodOctetsSent",
                            "Sum of lengths of all good ethernet")
  header=header..string.format("%-23s%-55s\n","",
                            "frames sent from this MAC")
    
  print(header)
end


---
--  clearMacCountersAndReturnClearOnReadToTheSameStatus
--        @description  varify clear on read status, enables it if was disable,
--            clear the counters by reading, return to the previous status
--
--        @param params             - devNum: device number
--                  -portNum: port number
--
--        @return       success(0/1), values of mac counters and errors messages
--
local function clearMacCountersAndReturnClearOnReadToTheSameStatus(devNum, portNum)
    local clearOnReadStatus, result, values
    local errMsg=""
    result, values= myGenWrapper("cpssDxChPortMacCountersClearOnReadGet",{  
    {"IN","GT_U8","devNum",devNum},
    {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
    {"OUT","GT_BOOL","enablePtr"}
    })

    if 0~=result then  --couldn't get mac counters clear on read status
        return 1,nil,"Error at clearing mac counters"..devNum.."/"..portNum..": "..returnCodes[result]
    else
        clearOnReadStatus=values["enablePtr"]
        if false==clearOnReadStatus then
            result = 
                cpssPerPortParamSet("cpssDxChPortMacCountersClearOnReadSet",
                                    devNum, portNum, true, "enable", 
                                    "GT_BOOL")
            if 0~=result then  --couldn't set counters as clear on read
                return 1,nil,"Error at clearing mac counters"..devNum.."/"..portNum..": "..returnCodes[result]
            end
        end
        
        --counters set as clear on read- clear by getting traffic.
        result, values =  myGenWrapper("cpssDxChPortMacCountersOnPortGet",{  --getting mac counters for current port
                            {"IN","GT_U8","devNum",device},
                            {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                            {"OUT","CPSS_PORT_MAC_COUNTER_SET_STC","portMacCounterSetArrayPtr"}
                            })
        
        if 0~=result then
            errMsg="Error at clearing mac counters"..devNum.."/"..portNum..": "..returnCodes[result]
        end
        
        if false==clearOnReadStatus then
            result = 
                cpssPerPortParamSet("cpssDxChPortMacCountersClearOnReadSet",
                                    devNum, portNum, false, "enable", 
                                    "GT_BOOL")
            if 0~=result then  --couldn't disable clear on read
                errMsg=errMsg.."Error at disabling clear on read mac counters"..devNum.."/"..portNum..": "..returnCodes[result]
            end
        end
        
        if ""~=errMsg then
            return 1,values, errMsg
        end
        return 0, values, ""
    
    end
end

-- ************************************************************************
---
--  counters_show
--        @description  show packet trace counters 
--
--        @param params             -params["device"]: device number
--									  			  
--
--        @return       true on success, otherwise false and error message
--
local function counters_show(params)
    local device=params["device"]
    local mode, vlan
    local srcPort, dstPort
    local data=""
    local bottom=""
    local errorMsg=""
    local errTmp
    local ret, val, val1
    local tmp_good_octets_received,
          tmp_bad_octets_received,
          tmp_good_octets_sent
    local flag=true  --flag that indicates if getting mac counters for any port from the vlan didn't succeed
    
    --mac counters variables
    local     src_good_octets_received, dst_good_octets_received,
                src_bad_octets_received, dst_bad_octets_received,
                src_good_octets_sent, dst_good_octets_sent
    --ingress counters variables  
    local src_gtBrgInFrames,      dst_gtBrgInFrames,    
          src_gtBrgVlanIngFilterDisc,   dst_gtBrgVlanIngFilterDisc,
          src_gtBrgSecFilterDisc,       dst_gtBrgSecFilterDisc,
          src_gtBrgLocalPropDisc,       dst_gtBrgLocalPropDisc

    --egress counters variables  
    local src_outUcFrames,        dst_outUcFrames,
          src_outMcFrames,        dst_outMcFrames,
          src_outBcFrames,        dst_outBcFrames,
          src_brgEgrFilterDisc,   dst_brgEgrFilterDisc,
          src_txqFilterDisc,      dst_txqFilterDisc,
          src_outCtrlFrames,      dst_outCtrlFrames,
          src_egrFrwDropFrames,   dst_egrFrwDropFrames  
          
    local spaceBeforeTitles="                                       "
    local spaceBeforeTitlesVLAN="                             "	
    local errorCount="---"	  
--getting mode
    local count0="CPSS_DXCH_BRG_CNT_SET_ID_0_E"
    local count1="CPSS_DXCH_BRG_CNT_SET_ID_1_E"
    ret,val = myGenWrapper("cpssDxChBrgCntBridgeIngressCntrModeGet",{	--get configure:
                {"IN","GT_U8","devNum",device},
                {"IN","CPSS_DXCH_BRIDGE_CNTR_SET_ID_ENT","cntrSetId",count0},
                {"OUT","CPSS_BRIDGE_INGR_CNTR_MODES_ENT","setModePtr"},				
                {"OUT","GT_PORT_NUM","portPtr"},
                {"OUT","GT_U16","vlanPtr"}
                })				

    if 0~=ret then
        errorMsg=errorMsg.."Error at getting counters mode: "..returnCodes[ret].."\n"
    else
        if "CPSS_BRG_CNT_MODE_2_E"==val["setModePtr"] then
            mode="vlan"
        end
        
        if "vlan"==mode then
        --**************************************************************************
        --***********************vlan mode******************************************
        --**************************************************************************

            --get vlan id
            vlan= val["vlanPtr"]
            data=data.."\n"
            data=data.."        Counter                 VLAN "..vlan.."\n"
            data=data.."-----------------------  --------------------- \n"	
            
            
            --getting vlan members
            ret,val=vlan_info_get(device,vlan)
            
            if 0==ret then
                local portMembers=val["portsMembers"]
            
                --getting mac counters
                src_good_octets_received=0
                src_bad_octets_received=0
                src_good_octets_sent=0
                for key,port in pairs(portMembers) do  --iterate over all the ports related to the vlan

                    
                    --[[ret,val = myGenWrapper("cpssDxChPortMacCountersOnPortGet",{  --getting mac counters for current port
                        {"IN","GT_U8","devNum",device},
                        {"IN","GT_PHYSICAL_PORT_NUM","portNum",port},
                        {"OUT","CPSS_PORT_MAC_COUNTER_SET_STC","portMacCounterSetArrayPtr"}
                        })]]
                    ret,val,errTmp=clearMacCountersAndReturnClearOnReadToTheSameStatus(device, port)  

                            
                    if        0 == ret then   --got mac counters for current port
                        --proccess data
                        tmp_good_octets_received            = 
                            val["portMacCounterSetArrayPtr"]["goodOctetsRcv"]
                        tmp_bad_octets_received             = 
                            val["portMacCounterSetArrayPtr"]["badOctetsRcv"]
                        tmp_good_octets_sent                = 
                            val["portMacCounterSetArrayPtr"]["goodOctetsSent"]

                        
                        tmp_good_octets_received=tmp_good_octets_received["l"][0]+tmp_good_octets_received["l"][1]*2^32
                        tmp_bad_octets_received=tmp_bad_octets_received["l"][0]+tmp_bad_octets_received["l"][1]*2^32
                        tmp_good_octets_sent=tmp_good_octets_sent["l"][0]+tmp_good_octets_sent["l"][1]*2^32
                
                        src_good_octets_received=src_good_octets_received+tmp_good_octets_received--sum the counters
                        src_bad_octets_received=src_bad_octets_received+tmp_bad_octets_received
                        src_good_octets_sent=src_good_octets_sent+tmp_good_octets_sent
            
                    else
                        flag=false  --can't get mac counters for all the members in the vlan
                        break
                    
                    
                    end

                end
            
                if true==flag then
                    -- mac counters strings formatting and adding  
                    data=data..string.format("%-24s%-23s","goodOctetsRcv",
                                            alignLeftToCenterStr(tostring(src_good_octets_received),23)).."\n"  
                    data=data..string.format("%-24s%-23s","badOctetsRcv",
                                            alignLeftToCenterStr(tostring(src_bad_octets_received),23)).."\n"  
                    bottom=bottom..string.format("%-24s%-23s","goodOctetsSent",
                                            alignLeftToCenterStr(tostring(src_good_octets_sent),23))                        
                        
                
                else  --error occure             
                    errorMsg=errTmp
                    data=data..string.format("%-23s%-23s","goodOctetsRcv",
                                            alignLeftToCenterStr(tostring(errorCount),23)).."\n"  
                    data=data..string.format("%-24s%-23s","badOctetsRcv",
                                            alignLeftToCenterStr(tostring(errorCount),23)).."\n"  
                    bottom=bottom..string.format("%-24s%-23s","goodOctetsSent",
                                            alignLeftToCenterStr(tostring(errorCount),23))                        
                                        
                end        

            else 
            
                errorMsg=errorMsg.."Error at getting MAC counters: "..returnCodes[ret].."\n"

            
            
            end
            
            
            data=data.."\n"            
            --***********************getting ingress counters-************************
            data=data..spaceBeforeTitlesVLAN.."Ingress counters \n"
        
            ret,val = myGenWrapper("cpssDxChBrgCntBridgeIngressCntrsGet",{  --getting ingress counters:
                        {"IN","GT_U8","devNum",device},
                        {"IN","CPSS_DXCH_BRIDGE_CNTR_SET_ID_ENT","cntrSetId","CPSS_DXCH_BRG_CNT_SET_ID_0_E"},
                        {"OUT","CPSS_BRIDGE_INGRESS_CNTR_STC","ingressCntrPtr"}
                        })    
            
            if        0 == ret then   --got ingress counters for source port        
                --proccess data
                src_gtBrgInFrames= val["ingressCntrPtr"]["gtBrgInFrames"]
                src_gtBrgVlanIngFilterDisc= val["ingressCntrPtr"]["gtBrgVlanIngFilterDisc"]
                src_gtBrgSecFilterDisc= val["ingressCntrPtr"]["gtBrgSecFilterDisc"]
                src_gtBrgLocalPropDisc= val["ingressCntrPtr"]["gtBrgLocalPropDisc"]

                
                -- ingress counters strings formatting and adding  
                data=data..string.format("%-24s%-23s","gtBrgInFrames",
                    alignLeftToCenterStr(tostring(src_gtBrgInFrames),23)).."\n"  
                data=data..string.format("%-24s%-23s","gtBrgVlanIngFilterDisc",
                    alignLeftToCenterStr(tostring(src_gtBrgVlanIngFilterDisc),23)).."\n"  
                data=data..string.format("%-24s%-23s","gtBrgSecFilterDisc",
                    alignLeftToCenterStr(tostring(src_gtBrgSecFilterDisc),23)).."\n"  
                data=data..string.format("%-24s%-23s","gtBrgLocalPropDisc",
                    alignLeftToCenterStr(tostring(src_gtBrgLocalPropDisc),23)).."\n"  

            else
                errorMsg=errorMsg.."Error at getting ingress counter: "..returnCodes[ret].."\n"
            
                data=data..string.format("%-24s%-23s","gtBrgInFrames",
                    alignLeftToCenterStr(tostring(errorCount),23)).."\n"  
                data=data..string.format("%-24s%-23s","gtBrgVlanIngFilterDisc",
                    alignLeftToCenterStr(tostring(errorCount),23)).."\n"  
                data=data..string.format("%-24s%-23s","gtBrgSecFilterDisc",
                    alignLeftToCenterStr(tostring(errorCount),23)).."\n"  
                data=data..string.format("%-24s%-23s","gtBrgLocalPropDisc",
                    alignLeftToCenterStr(tostring(errorCount),23)).."\n"  
            end
    
    

    
    
    
    
    
                    --***********************getting drop counter (src port)--*****************
            ret,val = myGenWrapper("cpssDxChCfgIngressDropCntrGet",{  --getting drop counter
                                    {"IN","GT_U8","devNum",device},
                                    {"OUT","GT_U32","counterPtr"}
                                    })        
            
            if        0 == ret then 
                -- drop counter strings formatting and adding  
                data=data..string.format("%-24s%-23s","dropCounter",
                    alignLeftToCenterStr(tostring(val["counterPtr"]),23)).."\n"  
                --resetting drop counter (not clear on read)
                ret = myGenWrapper("cpssDxChCfgIngressDropCntrSet",{  
                            {"IN","GT_U8","devNum",device},
                            {"IN","GT_U32","counter",0}
                            })        
                if 0~=ret then    
                    errorMsg=errorMsg.."Error at resetting vlan drop counter: "..returnCodes[ret].."\n"        
                end
                    
            
            else
                errorMsg=errorMsg.."Error at getting drop counter: "..returnCodes[ret].."\n"
                data=data..string.format("%-24s%-23s","dropCounter",
                    alignLeftToCenterStr(tostring(errorCount),23)).."\n"  
                                
                
            end
            data=data.."\n"  --seperate ingress & drop counters from egress counters      

    
    
    
    

                    --***********************getting egress counters--***********************
            data=data..spaceBeforeTitlesVLAN.."Egress counters \n"  --title for egress counter
            --source counter:
            ret,val = myGenWrapper("cpssDxChPortEgressCntrsGet",{  
            {"IN","GT_U8","devNum",device},
            {"IN","GT_U8","cntrSetNum",0},
            {"OUT","CPSS_PORT_EGRESS_CNTR_STC","egrCntrPtr"}
            })    

            if        0 == ret then   --got egress counters for source port
                --proccess data
                src_outUcFrames= val["egrCntrPtr"]["outUcFrames"]
                src_outMcFrames= val["egrCntrPtr"]["outMcFrames"]
                src_outBcFrames= val["egrCntrPtr"]["outBcFrames"]
                src_brgEgrFilterDisc= val["egrCntrPtr"]["brgEgrFilterDisc"]
                src_txqFilterDisc= val["egrCntrPtr"]["txqFilterDisc"]
                src_outCtrlFrames= val["egrCntrPtr"]["outCtrlFrames"]
                src_egrFrwDropFrames= val["egrCntrPtr"]["egrFrwDropFrames"]

                -- egress counters strings formatting and adding  
                data=data..string.format("%-24s%-23s","outUcFrames",
                    alignLeftToCenterStr(tostring(src_outUcFrames),23)).."\n"  
                data=data..string.format("%-24s%-23s","outMcFrames",
                    alignLeftToCenterStr(tostring(src_outMcFrames),23)).."\n"  
                data=data..string.format("%-24s%-23s","outBcFrames",
                    alignLeftToCenterStr(tostring(src_outBcFrames),23)).."\n"  
                data=data..string.format("%-24s%-23s","brgEgrFilterDisc",
                    alignLeftToCenterStr(tostring(src_brgEgrFilterDisc),23)).."\n"  
                data=data..string.format("%-24s%-23s","txqFilterDisc",
                    alignLeftToCenterStr(tostring(src_txqFilterDisc),23)).."\n"  
                data=data..string.format("%-24s%-23s","outCtrlFrames",
                    alignLeftToCenterStr(tostring(src_outCtrlFrames),23)).."\n"  
                data=data..string.format("%-24s%-23s","egrFrwDropFrames",
                    alignLeftToCenterStr(tostring(src_egrFrwDropFrames),23)).."\n"    
            else
                errorMsg=errorMsg.."Error at getting egress counters: "..returnCodes[ret].."\n"

                data=data..string.format("%-24s%-23s","outUcFrames",
                    alignLeftToCenterStr(tostring(errorCount),23),
                    alignLeftToCenterStr(tostring(errorCount),23)).."\n"  
                data=data..string.format("%-24s%-23s","outMcFrames",
                    alignLeftToCenterStr(tostring(errorCount),23),
                    alignLeftToCenterStr(tostring(errorCount),23)).."\n"  
                data=data..string.format("%-24s%-23s","outBcFrames",
                    alignLeftToCenterStr(tostring(errorCount),23),
                    alignLeftToCenterStr(tostring(errorCount),23)).."\n"  
                data=data..string.format("%-24s%-23s","brgEgrFilterDisc",
                    alignLeftToCenterStr(tostring(errorCount),23),
                    alignLeftToCenterStr(tostring(errorCount),23)).."\n"  
                data=data..string.format("%-24s%-23s","txqFilterDisc",
                    alignLeftToCenterStr(tostring(errorCount),23),
                    alignLeftToCenterStr(tostring(errorCount),23)).."\n"  
                data=data..string.format("%-24s%-23s","outCtrlFrames",
                    alignLeftToCenterStr(tostring(errorCount),23),
                    alignLeftToCenterStr(tostring(errorCount),23)).."\n"  
                data=data..string.format("%-24s%-23s","egrFrwDropFrames",
                    alignLeftToCenterStr(tostring(errorCount),23),
                    alignLeftToCenterStr(tostring(errorCount),23)).."\n"  

            end
    
            data=data.."\n"..bottom
            printLocalHeader()
            print(data.."\n")
            if ""~=errorMsg then
                print(errorMsg.."\n")
            end    
    
    

    
    
    
        
        else  --source and destination port
        --****************************************************************************
        --***********************2 ports mode*****************************************
        --****************************************************************************

            --***********************get ports**********************************************  
            srcPort=val["portPtr"]
            ret,val = myGenWrapper("cpssDxChBrgCntBridgeIngressCntrModeGet",{  --get configure:
                        {"IN","GT_U8","devNum",device},
                        {"IN","CPSS_DXCH_BRIDGE_CNTR_SET_ID_ENT","cntrSetId",count1},
                        {"OUT","CPSS_BRIDGE_INGR_CNTR_MODES_ENT","setModePtr"},        
                        {"OUT","GT_PORT_NUM","portPtr"},
                        {"OUT","GT_U16","vlanPtr"}
                        })        
                        
            if 0~=ret then
                errorMsg=errorMsg.."Error at getting counters mode: "..returnCodes[ret].."\n"
            else  
            
                dstPort=val["portPtr"]

                local devPort=device.."/"..srcPort
                local devPort2=device.."/"..dstPort
                --header for the counters data
                data=data.."\n"
                data=data.."        Counter            Source port- "..devPort.."    Destination port- "..devPort2.."\n"
                data=data.."-----------------------  ---------------------  ---------------------\n"  
        
                --***********************getting mac counters*******************************

                ret,val,errTmp=clearMacCountersAndReturnClearOnReadToTheSameStatus(device, srcPort)  
                            
                --************************************reset      
                if        0 == ret then   --got mac counters for source port

                    --------getting mac counter for destination port  
                    ret,val1,errTmp=clearMacCountersAndReturnClearOnReadToTheSameStatus(device, dstPort)                  
                
                    if        0 == ret then 
                        --proccess data
                        src_good_octets_received            = 
                            val["portMacCounterSetArrayPtr"]["goodOctetsRcv"]
                        src_bad_octets_received             = 
                            val["portMacCounterSetArrayPtr"]["badOctetsRcv"]
                        src_good_octets_sent                = 
                            val["portMacCounterSetArrayPtr"]["goodOctetsSent"]
                        dst_good_octets_received            = 
                            val1["portMacCounterSetArrayPtr"]["goodOctetsRcv"]
                        dst_bad_octets_received             = 
                            val1["portMacCounterSetArrayPtr"]["badOctetsRcv"]
                        dst_good_octets_sent                = 
                            val1["portMacCounterSetArrayPtr"]["goodOctetsSent"]
                        
                        
                        src_good_octets_received=src_good_octets_received["l"][0]+src_good_octets_received["l"][1]*2^32
                        src_bad_octets_received=src_bad_octets_received["l"][0]+src_bad_octets_received["l"][1]*2^32
                        src_good_octets_sent=src_good_octets_sent["l"][0]+src_good_octets_sent["l"][1]*2^32
                        dst_good_octets_received=dst_good_octets_received["l"][0]+dst_good_octets_received["l"][1]*2^32
                        dst_bad_octets_received=dst_bad_octets_received["l"][0]+dst_bad_octets_received["l"][1]*2^32
                        dst_good_octets_sent=dst_good_octets_sent["l"][0]+dst_good_octets_sent["l"][1]*2^32

                        
                        -- mac counters strings formatting and adding  
                        data=data..string.format("%-24s%-23s%-23s","goodOctetsRcv",
                                                alignLeftToCenterStr(tostring(src_good_octets_received),23),
                                                alignLeftToCenterStr(tostring(dst_good_octets_received),23)).."\n"  
                        data=data..string.format("%-24s%-23s%-23s","badOctetsRcv",
                                                alignLeftToCenterStr(tostring(src_bad_octets_received),23),
                                                alignLeftToCenterStr(tostring(dst_bad_octets_received),23)).."\n"  
                        bottom=bottom..string.format("%-24s%-23s%-23s","goodOctetsSent",
                                                alignLeftToCenterStr(tostring(src_good_octets_sent),23),
                                                alignLeftToCenterStr(tostring(dst_good_octets_sent),23))                        
                            
                    else  -- error at getting destination counters
                        errorMsg=errorMsg..errTmp
                        
                        data=data..string.format("%-23s%-23s%-23s","goodOctetsRcv",
                                                alignLeftToCenterStr(tostring(errorCount),23),
                                                alignLeftToCenterStr(tostring(errorCount),23)).."\n"  
                        data=data..string.format("%-24s%-23s%-23s","badOctetsRcv",
                                                alignLeftToCenterStr(tostring(errorCount),23),
                                                alignLeftToCenterStr(tostring(errorCount),23)).."\n"  
                        bottom=bottom..string.format("%-24s%-23s%-23s","goodOctetsSent",
                                                alignLeftToCenterStr(tostring(errorCount),23),
                                                alignLeftToCenterStr(tostring(errorCount),23))                        
                        
                        
                    end    
                else  -- error at getting source counters
                    errorMsg=errorMsg..errTmp
                    
                    data=data..string.format("%-23s%-23s%-23s","goodOctetsRcv",
                                            alignLeftToCenterStr(tostring(errorCount),23),
                                            alignLeftToCenterStr(tostring(errorCount),23)).."\n"  
                    data=data..string.format("%-24s%-23s%-23s","badOctetsRcv",
                                            alignLeftToCenterStr(tostring(errorCount),23),
                                            alignLeftToCenterStr(tostring(errorCount),23)).."\n"  
                    bottom=bottom..string.format("%-24s%-23s%-23s","goodOctetsSent",
                                            alignLeftToCenterStr(tostring(errorCount),23),
                                            alignLeftToCenterStr(tostring(errorCount),23))                    
                end
                data=data.."\n"            
                --***********************getting ingress counters-************************
                --source counter:
                data=data..spaceBeforeTitles.."Ingress counters \n"
            
                ret,val = myGenWrapper("cpssDxChBrgCntBridgeIngressCntrsGet",{  --get counters:
                            {"IN","GT_U8","devNum",device},
                            {"IN","CPSS_DXCH_BRIDGE_CNTR_SET_ID_ENT","cntrSetId","CPSS_DXCH_BRG_CNT_SET_ID_0_E"},
                            {"OUT","CPSS_BRIDGE_INGRESS_CNTR_STC","ingressCntrPtr"}
                            })    
                
                if        0 == ret then   --got ingress counters for source port
                    
                    --------getting ingress counter for destination port
                    
                    ret,val1 = myGenWrapper("cpssDxChBrgCntBridgeIngressCntrsGet",{  --get counters:
                            {"IN","GT_U8","devNum",device},
                            {"IN","CPSS_DXCH_BRIDGE_CNTR_SET_ID_ENT","cntrSetId","CPSS_DXCH_BRG_CNT_SET_ID_1_E"},
                            {"OUT","CPSS_BRIDGE_INGRESS_CNTR_STC","ingressCntrPtr"}
                            })

                        
                    
                    if        0 == ret then 
                        --proccess data
                        src_gtBrgInFrames= val["ingressCntrPtr"]["gtBrgInFrames"]
                        src_gtBrgVlanIngFilterDisc= val["ingressCntrPtr"]["gtBrgVlanIngFilterDisc"]
                        src_gtBrgSecFilterDisc= val["ingressCntrPtr"]["gtBrgSecFilterDisc"]
                        src_gtBrgLocalPropDisc= val["ingressCntrPtr"]["gtBrgLocalPropDisc"]
                        dst_gtBrgInFrames= val1["ingressCntrPtr"]["gtBrgInFrames"]
                        dst_gtBrgVlanIngFilterDisc= val1["ingressCntrPtr"]["gtBrgVlanIngFilterDisc"]
                        dst_gtBrgSecFilterDisc= val1["ingressCntrPtr"]["gtBrgSecFilterDisc"]
                        dst_gtBrgLocalPropDisc= val1["ingressCntrPtr"]["gtBrgLocalPropDisc"]
                        
                        
                        -- ingress counters strings formatting and adding  
                        data=data..string.format("%-24s%-23s%-23s","gtBrgInFrames",
                            alignLeftToCenterStr(tostring(src_gtBrgInFrames),23),
                            alignLeftToCenterStr(tostring(dst_gtBrgInFrames),23)).."\n"  
                        data=data..string.format("%-24s%-23s%-23s","gtBrgVlanIngFilterDisc",
                            alignLeftToCenterStr(tostring(src_gtBrgVlanIngFilterDisc),23),
                            alignLeftToCenterStr(tostring(dst_gtBrgVlanIngFilterDisc),23)).."\n"  
                        data=data..string.format("%-24s%-23s%-23s","gtBrgSecFilterDisc",
                            alignLeftToCenterStr(tostring(src_gtBrgSecFilterDisc),23),
                            alignLeftToCenterStr(tostring(dst_gtBrgSecFilterDisc),23)).."\n"  
                        data=data..string.format("%-24s%-23s%-23s","gtBrgLocalPropDisc",
                            alignLeftToCenterStr(tostring(src_gtBrgSecFilterDisc),23),
                            alignLeftToCenterStr(tostring(dst_gtBrgLocalPropDisc),23)).."\n"  

                        
                        
                    else
                        errorMsg=errorMsg.."Error at getting ingress counters: "..returnCodes[ret].."\n"
                    
                        data=data..string.format("%-24s%-23s%-23s","gtBrgInFrames",
                            alignLeftToCenterStr(tostring(errorCount),23),
                            alignLeftToCenterStr(tostring(errorCount),23)).."\n"  
                        data=data..string.format("%-24s%-23s%-23s","gtBrgVlanIngFilterDisc",
                            alignLeftToCenterStr(tostring(errorCount),23),
                            alignLeftToCenterStr(tostring(errorCount),23)).."\n"  
                        data=data..string.format("%-24s%-23s%-23s","gtBrgSecFilterDisc",
                            alignLeftToCenterStr(tostring(errorCount),23),
                            alignLeftToCenterStr(tostring(errorCount),23)).."\n"  
                        data=data..string.format("%-24s%-23s%-23s","gtBrgLocalPropDisc",
                            alignLeftToCenterStr(tostring(errorCount),23),
                            alignLeftToCenterStr(tostring(errorCount),23)).."\n"  
                    
                    
                    end  
                    
                else
                    errorMsg=errorMsg.."Error at getting ingress counters: "..returnCodes[ret].."\n"
                
                    data=data..string.format("%-24s%-23s%-23s","gtBrgInFrames",
                        alignLeftToCenterStr(tostring(errorCount),23),
                        alignLeftToCenterStr(tostring(errorCount),23)).."\n"  
                    data=data..string.format("%-24s%-23s%-23s","gtBrgVlanIngFilterDisc",
                        alignLeftToCenterStr(tostring(errorCount),23),
                        alignLeftToCenterStr(tostring(errorCount),23)).."\n"  
                    data=data..string.format("%-24s%-23s%-23s","gtBrgSecFilterDisc",
                        alignLeftToCenterStr(tostring(errorCount),23),
                        alignLeftToCenterStr(tostring(errorCount),23)).."\n"  
                    data=data..string.format("%-24s%-23s%-23s","gtBrgLocalPropDisc",
                        alignLeftToCenterStr(tostring(errorCount),23),
                        alignLeftToCenterStr(tostring(errorCount),23)).."\n"  
                


                end
                        
                    --***********************getting drop counter (src port)--*****************
                    ret,val = myGenWrapper("cpssDxChCfgIngressDropCntrGet",{  
                                            {"IN","GT_U8","devNum",device},
                                            {"OUT","GT_U32","counterPtr"}
                                            })        
                    
                    if        0 == ret then 
                        -- drop counter strings formatting and adding  
                        data=data..string.format("%-24s%-23s%-23s","dropCounter",
                            alignLeftToCenterStr(tostring(val["counterPtr"]),23),
                            alignLeftToCenterStr(tostring("Only for source-port"),23)).."\n"  
              
                            --eArch devices hold the counter as read only and clear so can't set to it
                            if is_sip_5(device) == false then 
                                --resetting drop counter (not clear on read)
                                ret = myGenWrapper("cpssDxChCfgIngressDropCntrSet",{  
                                        {"IN","GT_U8","devNum",device},
                                        {"IN","GT_U32","counter",0}
                                        })
                                if 0~=ret then    
                                    errorMsg=errorMsg.."Error at resetting source port drop counter: "..returnCodes[ret].."\n"        
                                end
                            end    
                    
                    else
                        errorMsg=errorMsg.."Error at getting drop counter: "..returnCodes[ret].."\n"
                        data=data..string.format("%-24s%-23s%-23s","dropCounter",
                            alignLeftToCenterStr(tostring(errorCount),23),
                            alignLeftToCenterStr(tostring("Only for source-port"),23)).."\n"  
                                        
                        
                    end
                    data=data.."\n"  --seperate drop counter from other counters
                    
                    
                --***********************getting egress counters--***********************
                data=data..spaceBeforeTitles.."Egress counters \n"  --title for egress counter
                --source counter:
                ret,val = myGenWrapper("cpssDxChPortEgressCntrsGet",{  
                {"IN","GT_U8","devNum",device},
                {"IN","GT_U8","cntrSetNum",0},
                {"OUT","CPSS_PORT_EGRESS_CNTR_STC","egrCntrPtr"}
                })    
                
                    
                if        0 == ret then   --got egress counters for source port
                    
                    --------getting egress counter for destination port
                    ret,val1 = myGenWrapper("cpssDxChPortEgressCntrsGet",{  
                    {"IN","GT_U8","devNum",device},
                    {"IN","GT_U8","cntrSetNum",1},
                    {"OUT","CPSS_PORT_EGRESS_CNTR_STC","egrCntrPtr"}
                    })    

                    if        0 == ret then 
                        --proccess data
                        src_outUcFrames= val["egrCntrPtr"]["outUcFrames"]
                        src_outMcFrames= val["egrCntrPtr"]["outMcFrames"]
                        src_outBcFrames= val["egrCntrPtr"]["outBcFrames"]
                        src_brgEgrFilterDisc= val["egrCntrPtr"]["brgEgrFilterDisc"]
                        src_txqFilterDisc= val["egrCntrPtr"]["txqFilterDisc"]
                        src_outCtrlFrames= val["egrCntrPtr"]["outCtrlFrames"]
                        src_egrFrwDropFrames= val["egrCntrPtr"]["egrFrwDropFrames"]
                        
                        dst_outUcFrames= val1["egrCntrPtr"]["outUcFrames"]
                        dst_outMcFrames= val1["egrCntrPtr"]["outMcFrames"]
                        dst_outBcFrames= val1["egrCntrPtr"]["outBcFrames"]
                        dst_brgEgrFilterDisc= val1["egrCntrPtr"]["brgEgrFilterDisc"]
                        dst_txqFilterDisc= val1["egrCntrPtr"]["txqFilterDisc"]
                        dst_outCtrlFrames= val1["egrCntrPtr"]["outCtrlFrames"]
                        dst_egrFrwDropFrames= val1["egrCntrPtr"]["egrFrwDropFrames"]





                        
                        -- egress counters strings formatting and adding  
                        data=data..string.format("%-24s%-23s%-23s","outUcFrames",
                            alignLeftToCenterStr(tostring(src_outUcFrames),23),
                            alignLeftToCenterStr(tostring(dst_outUcFrames),23)).."\n"  
                        data=data..string.format("%-24s%-23s%-23s","outMcFrames",
                            alignLeftToCenterStr(tostring(src_outMcFrames),23),
                            alignLeftToCenterStr(tostring(dst_outMcFrames),23)).."\n"  
                        data=data..string.format("%-24s%-23s%-23s","outBcFrames",
                            alignLeftToCenterStr(tostring(src_outBcFrames),23),
                            alignLeftToCenterStr(tostring(dst_outBcFrames),23)).."\n"  
                        data=data..string.format("%-24s%-23s%-23s","brgEgrFilterDisc",
                            alignLeftToCenterStr(tostring(src_brgEgrFilterDisc),23),
                            alignLeftToCenterStr(tostring(dst_brgEgrFilterDisc),23)).."\n"  
                        data=data..string.format("%-24s%-23s%-23s","txqFilterDisc",
                            alignLeftToCenterStr(tostring(src_txqFilterDisc),23),
                            alignLeftToCenterStr(tostring(dst_txqFilterDisc),23)).."\n"  
                        data=data..string.format("%-24s%-23s%-23s","outCtrlFrames",
                            alignLeftToCenterStr(tostring(src_outCtrlFrames),23),
                            alignLeftToCenterStr(tostring(dst_outCtrlFrames),23)).."\n"  
                        data=data..string.format("%-24s%-23s%-23s","egrFrwDropFrames",
                            alignLeftToCenterStr(tostring(src_egrFrwDropFrames),23),
                            alignLeftToCenterStr(tostring(dst_egrFrwDropFrames),23)).."\n"  
            
                    else
                        errorMsg=errorMsg.."Error at getting egress counters: "..returnCodes[ret].."\n"

                        data=data..string.format("%-24s%-23s%-23s","outUcFrames",
                            alignLeftToCenterStr(tostring(errorCount),23),
                            alignLeftToCenterStr(tostring(errorCount),23)).."\n"  
                        data=data..string.format("%-24s%-23s%-23s","outMcFrames",
                            alignLeftToCenterStr(tostring(errorCount),23),
                            alignLeftToCenterStr(tostring(errorCount),23)).."\n"  
                        data=data..string.format("%-24s%-23s%-23s","outBcFrames",
                            alignLeftToCenterStr(tostring(errorCount),23),
                            alignLeftToCenterStr(tostring(errorCount),23)).."\n"  
                        data=data..string.format("%-24s%-23s%-23s","brgEgrFilterDisc",
                            alignLeftToCenterStr(tostring(errorCount),23),
                            alignLeftToCenterStr(tostring(errorCount),23)).."\n"  
                        data=data..string.format("%-24s%-23s%-23s","txqFilterDisc",
                            alignLeftToCenterStr(tostring(errorCount),23),
                            alignLeftToCenterStr(tostring(errorCount),23)).."\n"  
                        data=data..string.format("%-24s%-23s%-23s","outCtrlFrames",
                            alignLeftToCenterStr(tostring(errorCount),23),
                            alignLeftToCenterStr(tostring(errorCount),23)).."\n"  
                        data=data..string.format("%-24s%-23s%-23s","egrFrwDropFrames",
                            alignLeftToCenterStr(tostring(errorCount),23),
                            alignLeftToCenterStr(tostring(errorCount),23)).."\n"  
                                
                    
                    
                    end  
                    
                            
                else
                    errorMsg=errorMsg.."Error at getting egress counters: "..returnCodes[ret].."\n"

                    data=data..string.format("%-24s%-23s%-23s","outUcFrames",
                        alignLeftToCenterStr(tostring(errorCount),23),
                        alignLeftToCenterStr(tostring(errorCount),23)).."\n"  
                    data=data..string.format("%-24s%-23s%-23s","outMcFrames",
                        alignLeftToCenterStr(tostring(errorCount),23),
                        alignLeftToCenterStr(tostring(errorCount),23)).."\n"  
                    data=data..string.format("%-24s%-23s%-23s","outBcFrames",
                        alignLeftToCenterStr(tostring(errorCount),23),
                        alignLeftToCenterStr(tostring(errorCount),23)).."\n"  
                    data=data..string.format("%-24s%-23s%-23s","brgEgrFilterDisc",
                        alignLeftToCenterStr(tostring(errorCount),23),
                        alignLeftToCenterStr(tostring(errorCount),23)).."\n"  
                    data=data..string.format("%-24s%-23s%-23s","txqFilterDisc",
                        alignLeftToCenterStr(tostring(errorCount),23),
                        alignLeftToCenterStr(tostring(errorCount),23)).."\n"  
                    data=data..string.format("%-24s%-23s%-23s","outCtrlFrames",
                        alignLeftToCenterStr(tostring(errorCount),23),
                        alignLeftToCenterStr(tostring(errorCount),23)).."\n"  
                    data=data..string.format("%-24s%-23s%-23s","egrFrwDropFrames",
                        alignLeftToCenterStr(tostring(errorCount),23),
                        alignLeftToCenterStr(tostring(errorCount),23)).."\n"  

                end
                    
                    
                    
                data=data.."\n"..bottom
                printLocalHeader()
                print(data.."\n")
                if ""~=errorMsg then
                    print(errorMsg.."\n")
                end    
            end

        end

    end




end


--------------------------------------------------------------------------------
-- command registration: counters show
--------------------------------------------------------------------------------
CLI_addCommand("exec", "show counters packet-trace", {
  func   = counters_show,
  help   = 'Show the counters configured by "counters packet-trace"',
  params={{type= "named", {format="device %devID", name="device", help="ID of the device needed to count"},
                    mandatory = {"device"}}
  }
})