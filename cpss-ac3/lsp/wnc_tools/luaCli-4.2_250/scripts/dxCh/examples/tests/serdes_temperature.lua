--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* serdes_temperature.lua
--*
--* DESCRIPTION:
--*       show ethernet interfaces serdes temperature on specific port 
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************


local isSuccess   = true                   
local testStillOk = true   

Test_Case = "serdes_temperature_port_1"
executeLocalConfig("dxCh/examples/configurations/serdes_temperature.txt")
ret, parm_NN, parm_enable_WM, parm_enable_BM, parm_CoverageLvl, parm_DevNum, parm_PortNum, parm_LaneNum,parm_mode1, parm_mode2,parm_feature = get_Test_Parameter_Def(Test_Case) 
Test_Case_Number = parm_NN
DevNum, PortNum, LaneNum, mode1,mode2=Set_Test_Case_Parameters(parm_MsgLevel, parm_DevNum, parm_PortNum, parm_LaneNum, parm_mode1, parm_mode2)
isSuccess =  Serdes_Run_Test_case(Test_Case)
testStillOk  = error_handle("********** "..to_string(Test_Case_Number).." Test_Case #"..to_string(parm_NN).." "..Test_Case, isSuccess, testStillOk )

--=========================================================================================

if CoverageLvl > 1 then

    Test_Case = "serdes_temperature_port_all"
    ret, parm_NN, parm_enable_WM, parm_enable_BM, parm_CoverageLvl, parm_DevNum, parm_PortNum, parm_LaneNum,parm_mode1, parm_mode2,parm_feature = get_Test_Parameter_Def(Test_Case) 
    Test_Case_Number = parm_NN
    DevNum, PortNum, LaneNum, mode1,mode2=Set_Test_Case_Parameters(parm_MsgLevel, parm_DevNum, parm_PortNum, parm_LaneNum, parm_mode1, parm_mode2)
    isSuccess =  Serdes_Run_Test_case(Test_Case)
    executeLocalConfig("dxCh/examples/configurations/serdes_temperature_deconfig.txt")
    testStillOk  = error_handle("********** "..to_string(Test_Case_Number).." Test_Case #"..to_string(parm_NN).." "..Test_Case, isSuccess, testStillOk )

end --if CoverageLvl > 1 then


return testStillOk;  


