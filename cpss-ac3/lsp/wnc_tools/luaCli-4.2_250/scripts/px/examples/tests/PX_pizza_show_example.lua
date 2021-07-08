--* PX_pizza_show_example.lua
--*
--* DESCRIPTION:
--*       test for PIPE device : egress-processing configuration.
--*                    done in 'DX system' but only as sanity since actual commands
--*                    currently only manipulate 'DB' (in the LUA)
--*
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--just run the config file
executeLocalConfig(luaTgfBuildConfigFileName("PX_pizza_arbiter_show"))


