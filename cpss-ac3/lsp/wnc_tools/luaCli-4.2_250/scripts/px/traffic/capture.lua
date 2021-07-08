--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* capture.lua
--*
--* DESCRIPTION:
--*       capture traffic functions
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************


do
    
    local capture_pcap_file_fd = nil
    local capture_pcap_params = nil
    cmdLuaCLI_registerCfunction("wraplWriteHexAsBinary")
    
    -- ************************************************************************
    ---
    --  capture_pcap_close
    --        @description  Close PCAP file
    --                      Save it to local FS or put to tftp server if required
    --
    --
    local function capture_pcap_close()
        if capture_pcap_file_fd == nil then
            return true
        end
        local ret = true
    
        fs.close(capture_pcap_file_fd)
        capture_pcap_file_fd = nil
        if capture_pcap_params.destination ~= nil then
            if capture_pcap_params.localFs then
                if (cmdCopyFromRAMFStoDisk(capture_pcap_params.filename) == -1) then
                    print("Unable to copy file from RAMFS to local FS")
                    ret = false
                else
                    print("Saved "..capture_pcap_params.filename.." on local FS")
                end
            elseif capture_pcap_params.tftp ~= nil then
                if capture_pcap_params.tftpport==nil then
                    capture_pcap_params.tftpport=69   --the default port
                end
                local res, e
                res,e = lua_tftp("put",
                        capture_pcap_params.tftp.string,
                        capture_pcap_params.tftpport,
                        capture_pcap_params.filename,
                        capture_pcap_params.filename)
    
                if (res==0) then
                    print("File "..capture_pcap_params.filename.." sent successfuly")
                else
                    print("Sending file failed : " .. e )
                    ret = false
                end
            end
        end
        capture_pcap_params = nil
        return ret
    end
    
    -- ************************************************************************
    ---
    --  capture_pcap_open
    --        @description   open PCAP file for captured packets
    --                       save parameters for copy on close
    --
    --        @param params  Parameters table:
    --                          { filename="xxx.pcap",
    --                            localFS = "localFS", -- optional save to local fs
    --                            tftp.string="10.0.0.1", -- optional tftp server ip
    --                            tftpport=69 -- optional tftp server port
    --                          }
    --
    --
    local function capture_pcap_open(params)
        capture_pcap_close()
        local err
        capture_pcap_file_fd, err = fs.open(params.filename, "w")
        if capture_pcap_file_fd == nil then
            print("Failed to open \""..params.filename.."\": "..err)
            return false
        end
        capture_pcap_params = duplicate(params)
        -- write pcap header
        local pcap_header
        pcap_header = string.format("%08x %04x %04x %08x %08x %08x %08x",
            0xa1b2c3d4, -- magic number
            2, 4, -- version major, minor
            0, -- GMT to local correction
            0, -- accuracy
            1500, -- snaplen
            1) -- data link type == LINKTYPE_ETHERNET
        wraplWriteHexAsBinary(capture_pcap_file_fd, pcap_header)
        return true
    end
    
    local function capture_pcap_packet(rxdata)
        local res, values = cpssGenWrapper("osTimeRT",{
            { "OUT", "GT_U32", "seconds" },
            { "OUT", "GT_U32", "nanoSeconds" }})
        if res ~= 0 then
            values={seconds=0,nanoSeconds=0}
        end
        local pkt_header
        --strip packet to snaplen
        local data = string.sub(rxdata.packet.data,1,1500*2)
        local datalen = string.len(data)/2
    
        pkt_header = string.format ("%08x %08x %08x %08x",
            values.seconds,
            values.nanoSeconds,
            datalen, -- saved length, can be less than actual
            rxdata.packet.len) -- actual length
    
        wraplWriteHexAsBinary(capture_pcap_file_fd, pkt_header .."\n\t" .. data.."\n")
    end
    
    -- ************************************************************************
    ---
    --  capture_from
    --        @description   Start capturing traffic
    --
    --        @param params  CLI parameters
    --
    --
    local function capture_from(params)
        local queue_len = 20
        local dev, port_set, idx, port, capture_type;
        local rxPorts, newRxPorts, newPortsNum;

        capture_type = "capture"
        if params.queue ~= nil then
            queue_len = params.queue
        end

        local pa = luaGlobalGet("rxProtocols")
        if pa == nil then pa = {} end
        if pa["capture"] == nil then
            -- not registered yet
            local rxQ = registerRx("capture", {}, queue_len)
            luaGlobalStore("test_Rx_Queue", rxQ)
        end
    
        local rxPorts = luaGlobalGet("test_Rx_Ports")
        if rxPorts == nil then rxPorts = {} end
        newRxPorts = {}
        for dev, port_set in pairs(params.port) do
            if rxPorts[dev] == nil then rxPorts[dev] = {}; end
            newRxPorts[dev] = {};
            newPortsNum = 0;
            for idx, port in pairs(port_set) do
                if rxPorts[dev][port] == nil then
                    rxPorts[dev][port] = capture_type;
                    newPortsNum = newPortsNum + 1;
                    newRxPorts[dev][newPortsNum] = port;
                end
            end
        end
        luaGlobalStore("test_Rx_Ports", rxPorts)
        setRcv(newRxPorts, true);
        if params.filename ~= nil then
            return capture_pcap_open(params)
        end
        return true
    end
    
    -- ************************************************************************
    ---
    --  capture_stop
    --        @description   stop capturing traffic
    --
    --
    local function capture_stop()
        local rxQ = luaGlobalGet("test_Rx_Queue")
        local rxPorts = luaGlobalGet("test_Rx_Ports")
        local dev, port_set, port, capture_type, port_in_dev_idx;
        local ports_by_types, dev_ports;
        if rxQ == nil then
            return
        end
        luaGlobalStore("test_Rx_Queue", nil)
        luaGlobalStore("test_Rx_Ports", nil)
        -- ports_by_types filling
        ports_by_types = {};
        for dev, port_set in pairs(rxPorts) do
            port_in_dev_idx = 1;
            for port, capture_type in pairs(port_set) do
                if ports_by_types[capture_type] == nil then
                    ports_by_types[capture_type] = {};
                end
                if ports_by_types[capture_type][dev] == nil then
                    ports_by_types[capture_type][dev] = {};
                end
                ports_by_types[capture_type][dev][port_in_dev_idx] = port;
                port_in_dev_idx = port_in_dev_idx + 1;
            end
        end
        -- disabling ports by types
        for capture_type, dev_ports in pairs(ports_by_types) do
            setRcv(dev_ports, false)
        end
        --
        if capture_pcap_file_fd ~= nil then
            -- flush data to file
            while true do
                local status, rxdata
                status, rxdata = luaMsgQRecv(rxQ, 0)
                if status ~= 0 then
                    break
                end
                capture_pcap_packet(rxdata)
            end
        end
        capture_pcap_close()
        unregisterRx("capture",rxQ)
    end
    
    -- ************************************************************************
    ---
    --  capture_show
    --        @description   Display capturing traffic
    --
    --        @param params  CLI parameters
    --
    --
    local function capture_show(params)
        local rxQ = luaGlobalGet("test_Rx_Queue")

        if rxQ == nil then
            return false
        end
        if params.timeout == nil then
            params.timeout = 0
        end
        while true do
            local status, rxdata
            status, rxdata = luaMsgQRecv(rxQ, tonumber(params.timeout))
            if status ~= 0 then
                break
            end
            capture_pcap_packet(rxdata)
            print(string.format("from port %d len=%d",
                    rxdata.portNum,
                    rxdata.packet.len))
            print("    "..rxdata.packet.data.."\n")
        end
    end
    
    CLI_type_dict["capture_queue_len"] = {
        checker = CLI_check_param_number,
        min=10,
        max=40,
        help="capture queue len"
    }
    
    --start capturing traffic
    CLI_addCommand("traffic", "capture from", {
        func=capture_from,
        help="Start packet capturing",
        params={
        { type="named",
            { format="port %port-range",name="port", help="ports to receive message from" },
            { format="queue %capture_queue_len", help="Max packets to be queued then shown by one command" },
            { format="pcap %filename", name="filename", help="Save packet to pcap file" },
            { format="copyToLocalFs", name="localFs", requirements={"filename"}, help = "Copy pcap file to local FS (optional)" },
            { format="to %ipv4", name="tftp", requirements={"filename"}, help = "The ip to upload pcap file to (optional)"},
            { format="port %l4port",name="tftpport", requirements={"tftp"}, help = "The file server port (optional)" },
            mandatory={"port"},
            alt={destination={"localFs","tftp"}}
        }}
    })
    
    --display captured traffic
    CLI_addCommand("traffic", "capture show", {
        func=capture_show,
        help="Show captured packets",
        params={
        { type="named",
            { format="timeout %GT_U32",name="timeout", help="timeout in seconds to wait for packet" }}
        }
    })
    
    --stop capturing traffic
    CLI_addCommand("traffic", "capture stop", {
        func=capture_stop,
        help="Stop packet capturing",
    })
    
    end
    