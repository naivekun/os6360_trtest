--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* TCP_support.lua
--*
--* DESCRIPTION:
--*       TCP support functions
--*
--* FILE REVISION NUMBER:
--*       $Revision: 2 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  send_file
--        @description  sending of a file over ip-protocol 
--
--        @param filename       - name of file
--        @param ip             - ip-address
--        @param port           - port name
--
--        @return       true 
--
function send_file(filename, ip, port)
    local descr,e,fd,tempStat,ack

    --open the file we want to send
    fd, e = fs.open(filename,"r")
    if fd == nil then
        return false,e
    end

    --connect to the server
    descr,e=tcp.connect(ip,port)

    if (descr==nil) then
        fs.close(fd)
        return false,e
    end

    tempStat=fs.stat(filename)
    tcp.send(descr,"push " .. filename .. " "..tempStat["st_size"].."\n")

    --wait for ack that the server is ready to receive
    ack,e=tcp.waitline(descr,1000)

    if (ack==0) then
        tcp.close(descr)
        fs.close(fd)
        return false,"Connection timed out"
    elseif (ack==nil) or (string.find(ack, "200 cmdFileTransfer ready") == nil) then
        tcp.close(descr)
        fs.close(fd)
        return false, "Connection error with ack: " .. ack
    end

    --wait for ack from server
    ack=tcp.waitline(descr,1000)
    if (ack==nil) or (string.find(ack, "201 Upload file now") == nil) then
        tcp.close(descr)
        fs.close(fd)
        return false, "Connection error with code: " .. ack
    end

    --send the file
    repeat
        local str
        str, e = fs.gets(fd)
        if str == nil then
            if e == "<EOF>" then
                break
            end
            tcp.close(descr)
            fs.close(fd)
            return false,("Error reading file: " .. e)
        end
    until	(tcp.send(descr,str)==false)


    --wait for file receiving ack from server
    ack=tcp.waitline(descr,1000)
    if (ack==nil) or (string.find(ack, "202 File upload successful") == nil) then
        tcp.close(descr)
        fs.close(fd)
        return false, "File transfer error: " .. ack
    end

    tcp.close(descr)
    fs.close(fd)

    return true
end


-- ************************************************************************
---
--  receive_file
--        @description  receiving of a file  
--
--        @param filename       - name of file
--        @param ip             - ip-address
--        @param port           - port name
--
--        @return       true on success, otherwise false and error message
--
local function receive_file(filename, ip, port)
    local descr,e,fd,ack,retVal
    retVal=true

    --if the filename we want to use already exists we create a backup
    if fs.fileexists(filename) then
        copy_file(filename, filename .. ".bak")
    end
    fs.unlink(filename)

    --open file on ramfs
    fd, e = fs.open(filename, "w")
    if fd == nil then
        fs.close(fd)
        return false,("Failed to open file " .. filename .. ": "..e)
    end

    --connect to the server
    descr,e=tcp.connect(ip,port)

    if (descr==nil) then 	retVal=false 	end

    if retVal==true then 	retVal,e=prv_receive_file(descr,fd,filename) end

    tcp.close(descr)
    fs.close(fd)
    return retVal,e
end


-- ************************************************************************
---
--  receive_file_tftp
--        @description  receiving of a file  via tftp
--
--        @param filename       - name of file
--        @param ip             - ip-address
--        @param port           - port name
--
--        @return       true on success, otherwise false and error message
--
local function receive_file_tftp(filename, ip, port)
    local rc, err = lua_tftp("get", ip, port, filename, filename)
    if rc == 0 then
        return true
    end
    return false, err
end

-- ************************************************************************
---
--  prv_receive_file
--        @description  receiving of a file
--
--        @param descr          - destination/source
--        @param fd             - file descriptor
--        @param filename       - name of file
--
--        @return       true on success, otherwise false and error message
--
function prv_receive_file(descr, fd, filename)
    local ack,filesize,strToWrite

    tcp.send(descr,"get " .. filename.."\n") --file request

    --wait for ack that the server is ready to receive
    ack=tcp.waitline(descr,1000)
    if (ack==nil) then return false, "Connection timed out"
    elseif (string.sub(ack,1,14)=="file not found") then
        return false, "File does not exist on the server"
    elseif (#ack>=16) and (string.sub(ack,1,14)~="get file acked") then
        return false,"Connection error"
    end

    filesize=tonumber(string.sub(ack,16))

    if (filesize==nil) then
        return false, "Connection error"
    end

    --receive file
    repeat
        if (tcp.wait(descr,1000)==1) then
            strToWrite=tcp.recv(descr)
            if (strToWrite~=nil) then
                fs.write(fd,strToWrite)
                filesize=filesize-#strToWrite
            else
                return false,"Connection error"
            end
        else
            return false,"Connection error"
        end

    until (filesize==0)

    --send ack when finished receiving
    tcp.send(descr,"get file complete\n")
    return true
end
