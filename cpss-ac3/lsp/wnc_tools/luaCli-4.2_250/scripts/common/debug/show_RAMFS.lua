--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_RAMFS.lua
--*
--* DESCRIPTION:
--*       showing of the file on the RAMFS
--*
--* FILE REVISION NUMBER:
--*       $Revision: 4 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  show_RAMFS
--        @description  show's of RAMFS
--
--        @param params         - The parameters
--
local function show_RAMFS(params)
    local fd,i,err,totalMem
    totalMem=0
    i=0

    fd,err = fs.opendir(" ")

    if (fd==nil) then
        print (err)
    else
        local a = fs.readdir(fd)
        print("\nIndex File name                                                    Size (Bytes)\n")

        while (a~=nil) and (a~=0) do
            i=i+1
            print(string.format("%-6d%-60s %s"  ,i,string.sub(to_string(a.d_name),2,-2),to_string(a.st_size)))
            totalMem=totalMem+a.st_size
            a = fs.readdir(fd)
        end

        print("\nTotal memory in use:"..totalMem.." Bytes\n")
        fs.closedir(fd)
    end


end

CLI_addCommand("debug", "show RAMFS", {
    func=show_RAMFS,
    help="Shows the file on the RAMFS",

})

-- ************************************************************************
---
--  printFile
--        @description  shows the contents of a file
--
--        @param params         - The parameters
--

local function printFile(params)
    local str,e,fd

    fd,e=fs.open(params["filename"])

    if fd == nil then
        print("failed to open file " .. params["filename"] .. ": "..e)
        return false
    end


    str, e = fs.gets(fd)
    while (str~=nil) do
        str=string.gsub(str,"\n","")
        print(str)
        str, e = fs.gets(fd)
    end
    fs.close(fd)
    if e ~= "<EOF>" then  print("error reading file: " .. e)  end
end

CLI_addCommand("debug", "show file", {
    func=printFile,
    help="Show the contents of a file in the RAMFS",
    params={
        { type="values", "%filename"},
    }
})
