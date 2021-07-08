--********************************************************************************
--                    { "0",        "1",       "2",     "3",     "4",       "5",        "6",          "7",       "8",      "9" } 
--TestLogMsgLevelList={"silent","emergency","alerts","critical","errors","warnings","notification","information","debug","verbose"};
--********************************************************************************
Test_Parameter_Def=({
["serdes_prbs_set_show_port_all_mode_all"]     ={NN= 1,enable_WM="Y", enable_BM="Y", feature="SERDES_PRBS"        ,CoverageLvl=3, MsgLevel=4, DevNum  ="Default", PortNum ="all",     LaneNum ="Default", mode1= { "all" }, mode2= {}},
["serdes_prbs_set_show_port_1_mode_1"]         ={NN= 2,enable_WM="Y", enable_BM="Y", feature="SERDES_PRBS"        ,CoverageLvl=1, MsgLevel=8, DevNum  ="Default", PortNum ="Default", LaneNum ="Default", mode1= { "7" }, mode2= {}},
["serdes_prbs_set_show_port_1_mode_all"]       ={NN= 3,enable_WM="Y", enable_BM="Y", feature="SERDES_PRBS"        ,CoverageLvl=2, MsgLevel=9, DevNum  ="Default", PortNum ="Default", LaneNum ="Default", mode1= { "all" }, mode2= {}},
["serdes_prbs_set_show_port_all_mode_1"]       ={NN= 4,enable_WM="Y", enable_BM="Y", feature="SERDES_PRBS"        ,CoverageLvl=2, MsgLevel=9, DevNum  ="Default", PortNum ="all",     LaneNum ="Default", mode1= { "7"   }, mode2= {}},
["serdes_prbs_set_no_enable_port_all_lane_all"]={NN= 5,enable_WM="Y", enable_BM="Y", feature="SERDES_PRBS"        ,CoverageLvl=3, MsgLevel=9, DevNum  ="Default", PortNum ="all",     LaneNum ="Default", mode1= {"all" }, mode2= {}},
["serdes_prbs_set_no_enable_port_1_lane_all"]  ={NN= 6,enable_WM="Y", enable_BM="Y", feature="SERDES_PRBS"        ,CoverageLvl=2, MsgLevel=9, DevNum  ="Default", PortNum ="Default", LaneNum ="Default", mode1= {"no" }, mode2= {}},
["serdes_auto_tune_show_port_all"]             ={NN= 7,enable_WM="Y", enable_BM="Y", feature="SERDES_AUTO_TUNE"   ,CoverageLvl=3, MsgLevel=4, DevNum  ="Default", PortNum ="all",     LaneNum ="Default", mode1= {}, mode2= {}},
["serdes_auto_tune_show_port_1"]               ={NN= 8,enable_WM="Y", enable_BM="Y", feature="SERDES_AUTO_TUNE"   ,CoverageLvl=2, MsgLevel=4, DevNum  ="Default", PortNum ="Default", LaneNum ="Default", mode1= {}, mode2= {}},
["serdes_no_reset_port_all"]                   ={NN= 9,enable_WM="Y", enable_BM="Y", feature="SERDES_RESET"       ,CoverageLvl=2, MsgLevel=9, DevNum  ="Default", PortNum ="all",     LaneNum ="Default", mode1= {}, mode2= {}},
["serdes_no_reset_port_1"]                     ={NN=10,enable_WM="Y", enable_BM="Y", feature="SERDES_RESET"       ,CoverageLvl=1, MsgLevel=9, DevNum  ="Default", PortNum ="Default", LaneNum ="Default", mode1= {}, mode2= {}},
["serdes_reset_port_all"]                      ={NN=11,enable_WM="Y", enable_BM="Y", feature="SERDES_RESET"       ,CoverageLvl=2, MsgLevel=9, DevNum  ="Default", PortNum ="all",     LaneNum ="Default", mode1= {}, mode2= {}},
["serdes_reset_port_1"]                        ={NN=12,enable_WM="Y", enable_BM="Y", feature="SERDES_RESET"       ,CoverageLvl=1, MsgLevel=9, DevNum  ="Default", PortNum ="Default", LaneNum ="Default", mode1= {}, mode2= {}},
["serdes_voltage_port_all"]                    ={NN=13,enable_WM="Y", enable_BM="Y", feature="SERDES_VOLTAGE"     ,CoverageLvl=1, MsgLevel=9, DevNum  ="Default", PortNum ="all",     LaneNum ="Default", mode1= {}, mode2= {}}, 
["serdes_voltage_port_1"]                      ={NN=14,enable_WM="Y", enable_BM="Y", feature="SERDES_VOLTAGE"     ,CoverageLvl=2, MsgLevel=9, DevNum  ="Default", PortNum ="Default", LaneNum ="Default", mode1= {}, mode2= {}}, 
["serdes_temperature_port_all"]                ={NN=15,enable_WM="Y", enable_BM="Y", feature="SERDES_TEMPERATURE" ,CoverageLvl=1, MsgLevel=9, DevNum  ="Default", PortNum ="all",     LaneNum ="Default", mode1= {}, mode2= {}}, 
["serdes_temperature_port_1"]                  ={NN=16,enable_WM="Y", enable_BM="Y", feature="SERDES_TEMPERATURE" ,CoverageLvl=1, MsgLevel=9, DevNum  ="Default", PortNum ="Default", LaneNum ="Default", mode1= {}, mode2= {}}, 
["serdes_polarity_trigger_true_port_all"]      ={NN=17,enable_WM="Y", enable_BM="Y", feature="SERDES_POLARITY"    ,CoverageLvl=2, MsgLevel=9, DevNum  ="Default", PortNum ="all",     LaneNum ="Default", mode1= { true  }, mode2= {}},
["serdes_polarity_trigger_true_port_1"]        ={NN=18,enable_WM="Y", enable_BM="Y", feature="SERDES_POLARITY"    ,CoverageLvl=1, MsgLevel=9, DevNum  ="Default", PortNum ="Default", LaneNum ="Default", mode1= { true  }, mode2= {}},
["serdes_polarity_trigger_false_port_all"]     ={NN=20,enable_WM="Y", enable_BM="Y", feature="SERDES_POLARITY"    ,CoverageLvl=2, MsgLevel=9, DevNum  ="Default", PortNum ="all",     LaneNum ="Default", mode1= { false }, mode2= {}},
["serdes_polarity_trigger_false_port_1"]       ={NN=21,enable_WM="Y", enable_BM="Y", feature="SERDES_POLARITY"    ,CoverageLvl=1, MsgLevel=9, DevNum  ="Default", PortNum ="Default", LaneNum ="Default", mode1= { false }, mode2= {}},
["serdes_polarity_Tx_false_Rx_true_port_all"]  ={NN=22,enable_WM="Y", enable_BM="Y", feature="SERDES_POLARITY"    ,CoverageLvl=2, MsgLevel=9, DevNum  ="Default", PortNum ="all",     LaneNum ="Default", mode1= { false }, mode2= { true }},
["serdes_polarity_Tx_false_Rx_true_port_1"]    ={NN=23,enable_WM="Y", enable_BM="Y", feature="SERDES_POLARITY"    ,CoverageLvl=1, MsgLevel=9, DevNum  ="Default", PortNum ="Default", LaneNum ="Default", mode1= { false }, mode2= { true }},
["serdes_polarity_Tx_true_Rx_false_port_all"]  ={NN=24,enable_WM="Y", enable_BM="Y", feature="SERDES_POLARITY"    ,CoverageLvl=2, MsgLevel=9, DevNum  ="Default", PortNum ="all",     LaneNum ="Default", mode1= { true },  mode2= {false }},
["serdes_polarity_Tx_true_Rx_false_port_1"]    ={NN=25,enable_WM="Y", enable_BM="Y", feature="SERDES_POLARITY"    ,CoverageLvl=1, MsgLevel=9, DevNum  ="Default", PortNum ="Default", LaneNum ="Default", mode1= { true },  mode2= {false }},
["serdes_polarity_Tx_true_Rx_true_port_all"]   ={NN=26,enable_WM="Y", enable_BM="Y", feature="SERDES_POLARITY"    ,CoverageLvl=2, MsgLevel=9, DevNum  ="Default", PortNum ="all",     LaneNum ="Default", mode1= { true },  mode2= {true }},
["serdes_polarity_Tx_true_Rx_true_port_1"]     ={NN=27,enable_WM="Y", enable_BM="Y", feature="SERDES_POLARITY"    ,CoverageLvl=1, MsgLevel=9, DevNum  ="Default", PortNum ="Default", LaneNum ="Default", mode1= { true },  mode2= {true }},
["serdes_polarity_Tx_false_Rx_false_port_all"] ={NN=28,enable_WM="Y", enable_BM="Y", feature="SERDES_POLARITY"    ,CoverageLvl=2, MsgLevel=9, DevNum  ="Default", PortNum ="all",     LaneNum ="Default", mode1= { false },  mode2= {false }},
["serdes_polarity_Tx_false_Rx_false_port_1"]   ={NN=29,enable_WM="Y", enable_BM="Y", feature="SERDES_POLARITY"    ,CoverageLvl=1, MsgLevel=9, DevNum  ="Default", PortNum ="Default", LaneNum ="Default", mode1= { false },  mode2= {false }},
})                                                                           
--********************************************************************************
return Test_Parameter_Def                                                    
--********************************************************************************
                                                                  





                                                                    
                                                                             
                                                                             
                                                                             
                                                                             
                                                                             
                                                                             
                                                                             
                                                                             
                                                                             




















