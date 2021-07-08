--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* serdesConf.lua
--*
--* DESCRIPTION:
--*       serdes configuration commands
--*
--* FILE REVISION NUMBER:
--*       $Revision: 12 $
--*
--********************************************************************************
cmdLuaCLI_registerCfunction("luaCLI_getTraceDBLine")
cmdLuaCLI_registerCfunction("wrlCpssCommonCpssVersionGet")

-- ************************************************************************
--  prvGetCpssVersion
--        @description  local function which retruns CPSS baseline version
--
--        @return       CPSS baseline version
--

local function prvGetCpssVersion()
local ret,val,cpss_version
    -- CPSS version data getting
    ret, val = wrLogWrapper("wrlCpssCommonCpssVersionGet")
    if (ret == LGT_OK) then
        cpss_version = separatedNumberStrGet(val)
    else
        cpss_version = MSG_NOT_APPLICABLE
    end
    return cpss_version

end

-------------------------------------------------------
-- command registration: execute configuration
-------------------------------------------------------
CLI_addCommand("debug", "load configuration", {
    func=load_running_config,
    help="Load configuration from file",
    params={
        { type="values", "%filename"},
        { type="named",
            "#loadableFile"
        }
    }
})


-- ************************************************************************
--  traceSave
--        @description  saves the dump into a file
--
--        @param params  - params[filename] - name of the file to save the dump to.
--                       - params[apiEeprom] - format of the output file, either apis or eeprom.
--                       - params[ip] - ip to send the file to.
--                       - params[port] - port to send the file to.
--
--        @return       true if there was no error otherwise false
--
local function traceSave(params)
    local data,mask,address,paramsBMP,dev,portGroup,fd, e,i,j,res,searchRes,cider_string,k
    local accessType -- type of acess (read or write) of hw access db entry read from table
    local addrSpace -- address space for register
    local mode -- which hw access mode to dump
    local dfx=false
    local firstLine = true
    i=0
    
    if params["dumpMode"] == nil
    then
    mode = 1 --write
    else
    mode = tonumber(params["dumpMode"])
    end
    
    --print("Mode is "..mode)

    if (params["devID"]=="all") then params["devID"]=nil end

    if fs.fileexists(params["filename"]) then
        copy_file(params["filename"], params["filename"] .. ".bak")
    end
    fs.unlink(params["filename"])

    address,mask,data,paramsBMP = luaCLI_getTraceDBLine(i)

    if (address==nil) and (data==nil) then print("No trace is stored") return false end

    fd, e = fs.open(params["filename"], "w")

    if fd == nil then
        print("failed to open file " .. params["filename"] .. ": "..e)
        return false
    end

   
    local result, values
    result, values = myGenWrapper(
                "appDemoTraceHwAccessDbIsCorrupted", {
                    { "OUT","GT_BOOL", "corruptedPtr"}
                }
            )
    
    if(values["corruptedPtr"] == true) then
    print("\nWARNING : DB is corrupted .The HW  access data base  has reached the limit and the recording has been stopped!\nUse trace clean in order to clear db")
    fs.write(fd,"WARNING : DB is corrupted .The HW  access data base  has reached the limit and the recording has been stopped!\n")
    end

    while (address~=nil and data~=nill) do
        dev=paramsBMP%256
        portGroup=math.floor((paramsBMP%0x10000)/256)
        accessType = math.floor((paramsBMP%0x8000000)/0x4000000)
        addrSpace = math.floor((paramsBMP%0x800000)/0x20000)
        

        if (((params["devID"]==nil) or (dev==params["devID"]))  and ((params["portGroup"]==nil) or (portGroup==params["portGroup"]))) then  --filter by port-group and device number

            --Write as APIs
            if params["apiEeprom"]=="api" then
                if (address~=0) then
                    if (accessType == mode)or (mode == 2) then
                        if accessType == 0 then accessType = "cpssDrvPpHwRegisterRead " 
                        else accessType = "cpssDrvPpHwRegisterWrite "
                        end
                    fs.write(fd, accessType..string.format(" 0x%x, 0x%x, 0x%08x, 0x%08x\n",dev,portGroup,address,data))
                    end
                else
                    fs.write(fd, "cpssOsDelay "..(data*1000).."\n")
                end

            --Write as EEPROM 
            elseif params["apiEeprom"]=="eeprom" then  --write as eeprom
                if (address~=0) then
                    if (accessType == mode)or (mode == 2) then
                    fs.write(fd,"offset "..string.format("%08x",address).." 0x"..string.format("%08x",data).."\n")
                    end
                else
                    fs.write(fd, "//----Delay "..data.." mSec----\n")
                    for j=0,(data-1) do
                        fs.write(fd, "offset 000000f0 0x"..string.format("%08x",j).."\n")
                    end
                    fs.write(fd, "//------End of delay------\n")
                end

            --Write as CIDER
            elseif params["apiEeprom"]=="cider" then
                if (address~=0) then
                    searchRes={}
                    searchRes["address"]=address
                    --print(string.format("-- Address: 0x%08X  Value 0x%08X",address,data))
                    if (do_search_unit_by_address(nil,searchRes,{},0,cider_RootNode())==true) then
                        cider_string="set unit ".. searchRes.foundpath

                        local k, idx
                        if type(searchRes.i_indices) == "table" then
                            for k = 1,#searchRes.i_indices do
                                if searchRes.i_indices[k].do_print then
                                    local index_string = " index "
                                    if k ~= 1 then
                                        index_string = string.format(" index%d ",k)
                                    end
                                    cider_string= cider_string .. index_string .. tostring(searchRes.i_indices[k].current_value)
                                end
                            end
                        end

                        cider_string = cider_string..string.format(" value 0x%08X\n",data)
                        fs.write(fd,cider_string)
                    else
                    -- fs.write(fd, string.format("-- Address: 0x%08X  Value 0x%08X\n",address,data))
                        if (accessType == mode)or (mode == 2) then
                            if accessType == 0 then 
                            fs.write(fd, string.format("register read device %d port-group %d offset 0x%08X data 0x%08X\n",dev,portGroup,address,data))
                            else
                            fs.write(fd, string.format("register write device %d port-group %d offset 0x%08X data 0x%08X\n",dev,portGroup,address,data))
                            end
                        end 
                        
                    end
                else --delay
                    fs.write(fd,"delay "..(data).."\n")
                end
            --Write as Micro-Init sequence
            elseif params["apiEeprom"]=="micInit" then
                -- check DFX address
                if addrSpace == 8 then
                    dfx=true
                else
                    dfx=false
                end

                if firstLine == true then
                    local version = prvGetCpssVersion();
                    fs.write(fd, "-- CPSS baseline version: "..version.."\n")
                    firstLine = false
                end

                if (address~=0) then
                    if (accessType == mode) or (mode == 2) then
                        if dfx == false then
                            fs.write(fd, string.format("wrd %x %x 0x%08x 0x%08x 0x%08x\n",dev,portGroup,address,data,mask))
                        else
                            fs.write(fd, string.format("wrd %x %x 0x%08x 0x%08x 0x%08x DFX\n",dev,portGroup,address,data,mask))
                        end
                    end
				else
					fs.write(fd,string.format("delay %d\n",data) )
				end


            --Write as CLI write register commands
            else
                if (address~=0) then
                    if (accessType == mode)or (mode == 2) then
                        if accessType == 0 then 
                        fs.write(fd, string.format("register read device %d port-group %d offset 0x%08X data 0x%08X\n",dev,portGroup,address,data))
                        else
                        fs.write(fd, string.format("register write device %d port-group %d offset 0x%08X data 0x%08X\n",dev,portGroup,address,data))
                        end
                    end 
                else --delay
                    fs.write(fd,"delay "..(data).."\n")
                end


            end
        end

        i=i+1
        address,mask,data,paramsBMP = luaCLI_getTraceDBLine(i)
    end

    fs.close(fd)

    if params["saveToHd"]~=nil then
      if (cmdCopyFromRAMFStoDisk(params["filename"]) == -1) then
        print("Error: Unable to save file ".. params["filename"] .. " to disk\n")
        return false
      end
    end

    if params["ip"]~=nil then

        if params["port"]==nil then
            params["port"]=12346   --the default port
        end
        res,e = send_file(params["filename"],params.ip["string"],params["port"])

        if (res==true) then
            print("File sent successfuly")
        else
            print("Sending file failed : " .. e )
            return false
        end
    end

    return true
end
-------------------------------------------------------
-- command registration: trace save
-------------------------------------------------------

CLI_addCommand("debug", "trace save", {
    func=traceSave,
    help="Trace commands",
    params={
        { type="named",
            { format="file %filename",name="filename",help="File name to output trace to"},
            { format="as %cpssApi_eeprom",name="apiEeprom",help="Type of file to create"},
            { format="device %devID_all",name="devID",help="Filter by device number (optional)"},
            { format="port-group %GT_U8",name="portGroup",help="Filter by port-group (optional)"},
            { format="to %ipv4", name="ip" ,help = "The ip to upload the file to (optional)"},
            { format="saveToHd", name="saveToHd" ,help = "Save file in CWD (optional)"},
            { format="port %l4port",name="port", help = "The file server port" },
            { format="mode %dumpMode",name="dumpMode",help=" write (default) - write hw access , read - read hw access, both   - read and write hw access"},
            requirements={["apiEeprom"]={"filename"}, ["devID"]={"apiEeprom"}, ["saveToHd"]={"apiEeprom"}, ["portGroup"]={"apiEeprom"}, ["ip"]={"apiEeprom"},["port"]={"ip"}},
            mandatory={"filename","apiEeprom"}
        },

}})

