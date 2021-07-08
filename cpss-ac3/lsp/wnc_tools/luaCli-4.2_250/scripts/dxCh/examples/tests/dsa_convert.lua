--********************************************************************************
--*              (C), Copyright 2001, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* dsa_convert.lua
--*
--* DESCRIPTION:
--*       test of wrappers wrlDxChDsaToString, wrlDxChDsaToStruct. The logic is:
--*       original DSA string -> wrlDxChDsaToStruct -> DSA struct ->
--*       -> wrlDxChDsaToString -> resulting DSA string. The test expects that
--*       the original DSA string should be equal to the resulting DSA string
--*
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

cmdLuaCLI_registerCfunction("wrlDxChDsaToString")
cmdLuaCLI_registerCfunction("wrlDxChDsaToStruct")

local devNum = devEnv.dev

local devFamily = wrlCpssDeviceFamilyGet(devNum)

local origDsaCh3Strings = {
   "FFF9FFFFFFFFEFFFBFFFDFFF7EFFFFFF", -- FORWARD, useEvidx = 0, srcIsTrunk=0
   "FFFDFFFFFFFFEFFFBFFFC0FF7EFFFFFF", -- FORWARD, useEvidx=0, srcIsTrunk=1
   "FFF9FFFFFFFFFFFF8FFFDFFF7EFFFFC0", -- FORWARD, useEvidx=1, srcIsTrunk=0
   "FFFDFFFFFFFFFFFF8FFFC0FF7EFFFFC0", -- FORWARD, useEvidx=1, srcIsTrunk=1

   -- FROM CPU tags. SrcDev is set to be even (because of Lion2 device numbering rules)
   "7FFBFFFFFFFF6400BFFFC00778FFFFFF", -- FROM CPU, useEvidx=0, excludeIsTrunk=0
   "7FFBFFFFFFFF6C00BFFFC00778FFFFFF", -- FROM CPU, useEvidx=0, excludeIsTrunk=1
   "7FFFFFFFFFFF67FE8FFFC0077EF8007F", -- FROM CPU, useEvidx=1, excludeIsTrunk=0, excludedDev is even (for Lion2)
   "7FFFFFFFFFFF6C7F8FFFC0FF7EF00000", -- FROM CPU, useEvidx=1, excludeIsTrunk=1

   "3FFFFFFFFFFFFFFF80107FFF7800007F", -- TO CPU, SrcTrg=1, OrigIsTrunk=1 (i.e ill-formed)
   "3FFFFFFFF7FFFFFF82107FFF7FFFFFFF", -- TO CPU, SrcTrg=1, OrigIsTrunk=0

   "BFFDFFFFBBFFCCFF801FFFFF78FFFFFF", -- TO ANALYZER, analyzerUseEvidx=0
   "BFFDFFFFB4000C00801FFFFF787FFFFF"  -- TO ANALYZER, analyzerUseEvidx=1
}
-- version for CH1 and CH2 that not support dropOnSource and packetIsLooped
-- FORWARD  Word1 bit26 packetIsLooped bit27 dropOnSource
-- FROM CPU Word1 bit25 packetIsLooped bit26 dropOnSource
-- TO CPU and TO ANALYZER Word1 bit28 packetIsLooped bit29 dropOnSource
-- TO CPU Word1 bit9 - IPFIX timestamp[0] only for xCat_A1 and above
local origDsaCh1Strings = {
   "FFF9FFFFF3FFEFFFBFFFDFFF7EFFFFFF", -- FORWARD, useEvidx = 0, srcIsTrunk=0
   "FFFDFFFFF3FFEFFFBFFFC0FF7EFFFFFF", -- FORWARD, useEvidx=0, srcIsTrunk=1
   "FFF9FFFFF3FFFFFF8FFFDFFF7EFFFFC0", -- FORWARD, useEvidx=1, srcIsTrunk=0
   "FFFDFFFFF3FFFFFF8FFFC0FF7EFFFFC0", -- FORWARD, useEvidx=1, srcIsTrunk=1

   -- FROM CPU tags. SrcDev is set to be even (because of Lion2 device numbering rules)
   "7FFBFFFFF9FF6400BFFFC00778FFFFFF", -- FROM CPU, useEvidx=0, excludeIsTrunk=0
   "7FFBFFFFF9FF6C00BFFFC00778FFFFFF", -- FROM CPU, useEvidx=0, excludeIsTrunk=1
   "7FFFFFFFF9FF67FE8FFFC0077EF8007F", -- FROM CPU, useEvidx=1, excludeIsTrunk=0, excludedDev is even (for Lion2)
   "7FFFFFFFF9FF6C7F8FFFC0FF7EF00000", -- FROM CPU, useEvidx=1, excludeIsTrunk=1

   "3FFFFFFFCFFFFDFF80107FFF7800007F", -- TO CPU, SrcTrg=1, OrigIsTrunk=1 (i.e ill-formed)
   "3FFFFFFFC7FFFDFF82107FFF7FFFFFFF", -- TO CPU, SrcTrg=1, OrigIsTrunk=0

   "BFFDFFFF8BFFCCFF801FFFFF78FFFFFF", -- TO ANALYZER, analyzerUseEvidx=0
   "BFFDFFFF84000C00801FFFFF787FFFFF"  -- TO ANALYZER, analyzerUseEvidx=1
}

local origDsaStrings;

if (devFamily == "CPSS_PP_FAMILY_CHEETAH_E") or
   (devFamily == "CPSS_PP_FAMILY_CHEETAH2_E") then
    origDsaStrings = origDsaCh1Strings;
else
    origDsaStrings = origDsaCh3Strings;
end


for unused, origDsaSt in  pairs(origDsaStrings) do
   printLog("use DSA string " .. origDsaSt)

   local rc, val = wrlDxChDsaToStruct(devNum, origDsaSt)
   if rc ~=0 then
      setFailState()
      printLog(val .. ". Error code is " .. rc)
      return
   end
   local dsaTab = val

   local rc, val = wrlDxChDsaToString(devNum, dsaTab)
   if (rc ~=0) then
      setFailState()
      printLog(val .. ". Error code is " .. rc)
      return
   end

   local newDsaSt = val
   if string.upper(origDsaSt) == string.upper(newDsaSt) then
      printLog("Successful")
   else
      setFailState()
      printLog("NOT EQUAL!")
      printLog("original DSA:  " .. origDsaSt)
      printLog("resulting DSA: " .. newDsaSt)
   end
end
