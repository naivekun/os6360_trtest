--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* upgradeFirmware.lua
--*
--* DESCRIPTION:
--*       Starts remote upgrade firmware of the internal CPU
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1$
--*
--********************************************************************************

--includes

--constants

  local updateStatus = nil

-- ************************************************************************
---
--  upgradeFW
--        @description    Upgrade FW of the internal CPU using 
--                        image located in RAMFS
--
--        @param params - The parameters
--
--        @return         true if there was no error otherwise false
--
local function upgradeFW(params)
  local result, values, devNum
  local status, err

  local str, e, fd

  local inBufSize       = 0
  local fileOfs         = 0
  local bufSize         = 0
  local dataLen         = 0
  local bufBmp          = 3
  local miStatus
  local imageSrc

  devNum = 0
  status = true

  if ((params.imageNum == nil) or (params.imageNum<1) or (params.imageNum>3))then
    print("Image number error")
    return false
  end
  bufBmp = params.imageNum

  if params.filename == nil then
    print("Image file name error.")
    return false
  end

  if ((params.sizeNum == nil) or (params.sizeNum<1) or (params.sizeNum>240))then
    print("Input buffer size is invalid")
    return false
  end
  inBufSize = params.sizeNum

  print("Loading " .. params.filename .. " .. \n")
  
  local fstat = fs.stat(params.filename)
  if fstat == nil then
    print("Error: Input file " .. tostring(params.filename) .. " not found.")
    return false
  end
  
  local fsize = fstat.st_size

  fd, e = fs.open(params["filename"], 'r')
  if fd == nil then
    print("failed to open file " .. params["filename"] .. ": ".. to_string(e))
    return false, e
  end

  -- iterate thru file
  while (fileOfs<fsize)  do
    bufSize=fsize-fileOfs;
    if (bufSize<=inBufSize) then
      dataLen = bufSize
      bufSize = bufSize+0x100 -- last block of file
    else
      dataLen = inBufSize
      bufSize = inBufSize
    end

    imageSrc = fs.read(fd, dataLen )
    
    if imageSrc == nil or imageSrc == "" then  
      print("error reading file: " .. params["filename"])  
      return false
    end

    -- upgrade firmware
    result, values = myGenWrapper("prvCpssGenericSrvCpuFWUpgrade", 
                                  {
                                    { "IN", "GT_U8",  "devNum",   devNum  },
                                    { "IN", "GT_U8",  "bufBmp",   bufBmp  },
                                    { "IN", "GT_U32", "fileOfs",  fileOfs/inBufSize },
                                    { "IN", "GT_U32", "bufSize",  bufSize },
                                    { "IN", "string", "fwBufPtr", imageSrc},
                                    { "OUT","GT_U32", "miStatus", miStatus}
                                  })

    if (result ~= 0) then
      print("Error: " .. to_string(returnCodes[result]) .. "\n")
      print("microInit code := " .. to_string(values.miStatus) .. "\n")
      return false
    end

    fileOfs = fileOfs + inBufSize

  end
  fs.close(fd)
    
  -- save update results
  updateStatus = 0
  return true
end

CLI_type_dict["imageFW"] = {
    checker = CLI_check_param_number,
    min=1,
    max=3,
    help = "Enter number of upgraded FW image: 1,2 or 3 for both images"
}

CLI_type_dict["bufSize"] = {
    checker = CLI_check_param_number,
    min=1,
    max=240,
    help = "Enter number of upgraded FW image: 1,2 or 3 for both images"
}

CLI_addCommand("debug", "load firmware", {
    help = "Transfer FirmWare located in RAMFS to internalCPU and burn it",
    func = upgradeFW,
    params={
        { type="values",
            { format="%imageFW",
               name="imageNum",
               help = "Number of upgraded FW image 1,2 or 3 for both images"}
        },
        { type="named",
            { format="imageName %filename",
               name = "filename",
               help  = "booton.bin or extendedImage.bin file name"}
        },
        { type="values",
            { format="%bufSize",
               name="sizeNum",
               help = "Size of buffer - (Min:32 Bytes, Max: 240 Bytes)"}
        }
    }
})

