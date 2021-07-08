Manual, that describes how to run and analyse CLI command syntax UT.

To run UT:
1) Unpack CPSS with bult-in lua-scripts and lua UT on Linux.
2) Compile CPSS in simulation mode on Linux (for instance, in lua folder).
3) Open folder with appDemoSim or copy it in home folder.
4) Start expect script:
	expect <CPSS path>/cpss/SW/prestera/tools/expectScripts/run_LuaCLI_UT.exp <lua folder path>/lua  <board name>
as board name "XCAT" could be specified
5) Run UT's.
6) Analyse results.

To Analyse results:
1) Open logfile_LuaCLI_XCAT2_report.txt and see UT running summary.
2) Open logfile_LuaCLI_xcat2.txt to see detailed failure reasons (just try to search "FAILED" in upper case).
 

UT's do not working on Windows platform.