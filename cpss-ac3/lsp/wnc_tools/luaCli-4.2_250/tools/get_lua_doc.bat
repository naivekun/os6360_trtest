@Set MyCurDir=%CD%

    @Set ARGS=%*
    @IF ""=="%ARGS%" SET ARGS=*.*
    
    @IF "%VAR%"=="" SET VAR=%1

    @cd ../scripts/
    luaDoc -d ../docs %ARGS%
    
@cd %MyCurDir%