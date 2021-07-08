local g_standAlone = 0;

local function File_Open(fileName, openOptionStr)
    local fid;
    local errormsg;
    if (g_standAlone == 0) then
        fid,errormsg = fs.open(fileName,openOptionStr);
    else
        fid,errormsg = io.open(fileName,openOptionStr);
    end
    return fid,errormsg;
end

local function File_Close(fid)
    if (g_standAlone == 0) then        
        fs.close(fid);
    else
        io.close(fid);
    end
end


local function File_PrintString(fid,str)
    if (g_standAlone == 0) then
        fs.write(fid,str);
    else
        fid:write(str);
    end
end

local function File_Flush(fid)
    if (g_standAlone == 0) then
        --fs.flush(fid);
    else
        fid:flush();
    end
end


local function LineParser_Init(str)
    local prs = {};
    local list = {};
    local i;
    local ind;

    ind = string.find(str,";");
    if (ind ~= nil) then
        str = string.sub(str,1,ind-1);
    end

    i = 1;
    for w in string.gmatch(str, "[%a%p%d]+") do
        if (w[1] == ";") then
            break;
        end
        list[i] = w;
        i = i+1
    end

    prs.list = list;
    prs.curInd = 1;
    prs.total  = i-1;
    if prs.total > 0 then
        return prs;
    else
        return nil;
    end
end

local function LineParser_Reset(prs)
    prs.curInd = 1
end

local function LineParser_GetToken(prs)
    local token = nil;
    if prs.curInd <= prs.total then
        token = prs.list[prs.curInd];
        prs.curInd = prs.curInd + 1;
    end
    return token;
end

local function LineParser_UnGetToken(prs)
    if prs.curInd > 1 then
        prs.curInd = prs.curInd - 1;
    end
end


local function FileParser_GetLine(prs)
    local line;
    local lineParser;
    while (1) do
        -- line = prs.fid.read(prs.fid); --,"*lines");
        prs.lineN = prs.lineN + 1;      
        line = prs.lineArr[prs.lineN];
        if (line == nil) then -- empty file or eof reached;
            return nil;
        end
        print(string.format(" %3d : %s",prs.lineN,line));
        lineParser = LineParser_Init(line);
        if lineParser ~= nil then
            prs.lineParser = lineParser;
            return line;
        end
    end
end

local function FileParser_UnGetLine(prs)
    if prs.lineN > 1 then
        prs.lineN = prs.lineN - 1;
    end
end


local function FileParser_UnGetToken(prs)
    LineParser_UnGetToken(prs.lineParser);
end


local function FileParser_GetToken(prs)
    return LineParser_GetToken(prs.lineParser);
end

local function FileParser_Init(filename)
    local fid;
    local errormsg;
    local prs;
    local line;
    local lineArr;
    local ind;
    local e;

    fid,errormsg = File_Open(filename,"r");
    if (fid == nil) then
        print(string.format("--> Error : %s : %s",filename,errormsg));
        --print(errormsg);
        return nil;
    end

    prs = {};
    -- prs.fid = fid;
    prs.lineN = 0;
    -- prs.Fun_TokenGet   = FileParser_GetToken;
    -- prs.Fun_TokenUnGet = FileParser_UnGetToken;
    -- prs.Fun_LineGet    = FileParser_GetLine;
    -- prs.Fun_LineUnGet  = FileParser_UnGetLine;
    
    lineArr = {};
    ind = 1;
    while (1) do
        if (g_standAlone == 0) then        
            line,e = fs.gets(fid); --,"*lines");    
        else
            line = fid.read(fid); --,"*lines");
        end
        if (line == nil) then -- empty file or eof reached;
            break;
        end
        line=string.gsub(line,"\n","")               
        lineArr[ind] = line;
        ind = ind + 1;
    end    
    prs.lineArr = lineArr;
    File_Close(fid);
    
    --[[
    lineParser = FileParser_GetLine(prs);
    if (lineParser == nil) then
        return nil;
    end
    prs.lineParser = lineParser;
    ]]
    return prs;
end

--[[
+------------------------------------------------------
|
+------------------------------------------------------
]]

local function Token_String(prs, argument)
    local token;
    token = FileParser_GetToken(prs);        
    if (token == nil) then
        return nil;
    end
    if (token ~= argument) then
       return nil;
    end
    return token;
end


local function Token_Number(prs)
    local token;
    local number;
    token = FileParser_GetToken(prs);        
    if (token == nil) then
        return nil;
    end
    number = tonumber(token);    
    if (number ==nil) then
       return nil;
    end
    return number;
end

local function Token_NumberList(prs)
    local coreList;
    local num;
    local ind;
    local token;
    coreList = {};

    ind = 1;
    while (1) do
        token = FileParser_GetToken(prs);
        if (token == nil) then
            return coreList;
        end
        num = tonumber(token);
        if (num == nil) then
            print("--> Error : " .. token .. " not a number");
            return nil;
        end
        coreList[ind] = num;
        ind = ind + 1;
    end
end

local function Token_Number_TestValue(prs,valueLow, valueHi)
    local token;
    local number;
    token = FileParser_GetToken(prs);        
    if (token == nil) then
        return nil;
    end
    number = tonumber(token);    
    if (number ==nil) then
       return nil;
    end
    if (number < valueLow  or number > valueHi) then
        print("--> Error : " .. token .. " is not a area ",valueLow," : ", valueHi);
        return nil;
    end
    return number;
end


local function Token_NumberList_TestValue(prs, valueLow, valueHi)
    local coreList;
    local num;
    local ind;
    local token;
    coreList = {};

    ind = 1;
    while (1) do
        token = FileParser_GetToken(prs);
        if (token == nil) then
            return coreList;
        end
        num = tonumber(token);
        if (num == nil) then
            print("--> Error : " .. token .. " not a number");
            return nil;
        end
        if (num < valueLow  or num > valueHi) then
            print("--> Error : " .. token .. " is not a area ",valueLow," : ", valueHi);
            return nil;
        end
        coreList[ind] = num;
        ind = ind + 1;
    end
end


--[[
]]


local function Process_Clause_Argument(prs, argument)
    local line;
    local token;

    line = FileParser_GetLine(prs);
    if (line == nil) then
       return nil;
    end
    token = FileParser_GetToken(prs);
    if (token == nil) then
        return nil;
    end
    if token ~= argument then
        print("---> Error: "..argument.." expected");
        return nil;
    end
    token = FileParser_GetToken(prs);
    if (token ~= nil) then
        print("-->Error: Unknown token "..token.." in processing clause : "..argument);
        return nil;
    end
    return argument;
end

local function Process_Clause_Argument_StringValue(prs, argument, testValue)
    local line;
    local token;
    local strValue;
    line = FileParser_GetLine(prs);
    if (line == nil) then
       return nil;
    end
    token = FileParser_GetToken(prs);
    if (token == nil) then
        return nil;
    end
    if token ~= argument then
        print("---> Error: "..argument.." expected");
        return nil;
    end

    strValue = FileParser_GetToken(prs);
    if (token == nil) then
        print("-->Error : value of parameter expected in processing clause : "..argument);
        return nil;
    end

    if (testValue ~= nil) then
        if (testValue ~= strValue) then
            print("-->Error : value of parameter "..strValue..". Expected :"..testValue);  
            return nil;
        end
    end

    token = FileParser_GetToken(prs);
    if (token ~= nil) then
        print("-->Error: Unknown token in processing clause : "..argument);
        return nil;
    end
    return strValue;
end


local function Process_Clause_Argument_IntegerValue(prs, argument)
    local line;
    local token;
    local value;
    local strValue;
    line = FileParser_GetLine(prs);
    if (line == nil) then
       return nil;
    end
    token = FileParser_GetToken(prs);
    if (token == nil) then
        return nil;
    end
    if token ~= argument then
        print("---> Error: "..argument.." expected");
        return nil;
    end

    strValue = FileParser_GetToken(prs);
    if (token == nil) then
        print("-->Error : value of parameter expected in processing clause : "..argument);
        return nil;
    end
    value = tonumber(strValue);
    if (value == nil) then
        print("-->Error : value of parameter in clause "..argument);
    end

    token = FileParser_GetToken(prs);
    if (token ~= nil) then
        print("-->Error: Unknown token in processing clause : "..argument);
        return nil;
    end
    return value;
end

local function ProcessProfile_Clause_Argument_IntegerArray(prs,argument)
    local token;
    local line;
    local valueList;

    line = FileParser_GetLine(prs);
    if (line == nil) then
       return nil;
    end
    token = FileParser_GetToken(prs);
    if (token == nil or token ~= argument) then
        print("-->Error : "..argument.." [list of integers] expected");
        return nil;
    end

    valueList = Token_NumberList(prs);
    if (valueList ~= nil) then
        return valueList;
    else
        return nil;
    end
end


--[[
+-----------------------------------------
|  clause system-declaration
+-----------------------------------------]]
local function ProcessProfile_Clause_SystemDeclaration(prs)
    local systemName;

    systemName = Process_Clause_Argument_StringValue(prs,"system");
    if (systemName == nil) then
        print("-->Error : system type [Lion2, Hooper, Puma3] expected");
        return nil;
    end

    if (systemName == "Lion2"  or systemName == "Hooper" or systemName == "Puma3")  then
        return systemName;
    else
        print("-->Error : unknown system name : "..systemName);
        return nil;
    end
end


--[[
+-----------------------------------------
|  clause profile-low-level config
+-----------------------------------------]]
local function ProcessProfile_Clause_CPU(prs)
    local cpu         = {};
    local value;

    value = Process_Clause_Argument(prs,"cpu");
    if (value == nil) then
        return nil;
    end

    value = Process_Clause_Argument_StringValue(prs,"type");
    if (value == nil) then
        return nil;
    end
    if (value ~= "none" and value ~= "external" and value ~= "fixed") then
        print("--> Error : unknown cpu type : ", value, "  [none | external | fixed] expected");
        return nil;
    end
    cpu.type = value;
    cpu.sliceList = {};
    cpu.port = -1;
    if (cpu.type == "external" or cpu.type == "fixed") then
        cpu.port = Process_Clause_Argument_IntegerValue(prs,"port");
        if (cpu.port == nil) then
            return nil;
        end
    end

    if (cpu.type == "none" or cpu.type == "external") then
        cpu.sliceList[1] = -1;
    else
        cpu.sliceList = ProcessProfile_Clause_Argument_IntegerArray(prs,"slices");
        if (cpu.sliceList == nil) then
           return nil;
        end
    end

    value = Process_Clause_Argument(prs,"end");
    if (value == nil) then
        return nil;
    end
    return cpu;
end

local function ProcessProfile_Clause_TxQDef(prs)
    local txqDef = {};
    local cpu    = {};
    local value;

    value = Process_Clause_Argument(prs,"txq-def");
    if (value == nil) then
        return nil;
    end

    value = Process_Clause_Argument_IntegerValue(prs,"TxQ-repetition");
    if (value == nil) then
        return nil;
    end
    txqDef.TxQ_repetition = value;

    cpu = ProcessProfile_Clause_CPU(prs);
    if (cpu == nil) then
        return nil;
    end
    txqDef.cpu = cpu;

    value = Process_Clause_Argument(prs,"end");
    if (value == nil) then
        return nil;
    end
    return txqDef;
end


local function ProcessProfile_Clause_LowLevel_Config(prs)
    local config      = {};
    local txqDef      = {};
    local value;

    value = Process_Clause_Argument(prs,"config");
    if (value == nil) then
        return nil;
    end

    value = Process_Clause_Argument_IntegerValue(prs,"total-ports");
    if (value == nil) then
        return nil;
    end
    config.total_ports = value;
    value = Process_Clause_Argument_IntegerValue(prs,"total-slices");
    if (value == nil) then
        return nil;
    end
    config.total_slices = value;

    txqDef = ProcessProfile_Clause_TxQDef(prs);
    if (txqDef == nil) then
        return nil;
    end
    config.txqDef = txqDef;

    value = Process_Clause_Argument(prs,"end");
    if (value == nil) then
        return nil;
    end
    return config;
end

--[[
+-----------------------------------------
|  clause port policy
+-----------------------------------------]]
local function Test_Clause_Argument(prs,argument)
    local token;
    token = FileParser_GetToken(prs);
    FileParser_UnGetToken(prs);
    if (token == argument) then
        return token;
    else
        return nil;
    end
end

--[[
--------------------------------------------
    Port Policy  : convert string to number of G in port 
    ex :  1G --> 1
         10G --> 10
        100G --> 100
--------------------------------------------]]
local function ProcessProfile_Clause_SpecificPortPolicy_Speed(speed_str)
    if (speed_str == "1G") then  
        return 1;
    end
    if (speed_str == "5G") then  
        return 5;
    end
    if (speed_str == "10G") then
        return 10;
    end
    if (speed_str == "12G") then
        return 12;
    end
    if (speed_str == "15G") then
        return 15;
    end
    if (speed_str == "16G") then
        return 16;
    end
    if (speed_str == "20G") then
        return 20;
    end
    if (speed_str == "40G") then
        return 40;
    end
    if (speed_str == "50G") then
        return 50;
    end
    if (speed_str == "75G") then
        return 75;
    end
    if (speed_str == "100G") then
        return 100;
    end
    return nil;
end

local function ProcessProfile_Clause_SpecificPortPolicy(prs,total_ports, portPolicy)
    local specificPortPolicy;
    local token;
    local line;
    local tableSize;
    local portId;    
    local speedStr;
    local speed;
    local slicesNum;
    local policyInd;
    local policy;    
    local policyEntry;
   
    
    token = FileParser_GetToken(prs);
    if (token ~= "port") then
        print("--> Error : port policy clause [port PortNumber : .... ] expected");
        return nil;
    end
    
    portId = Token_Number(prs);
    if (portId == nil) then
        print("--> Error : port id in policy clause expected");            
        return nil;
    end
    if (portId >= total_ports) then
        print("--> Error : port id "..portId.." shall be in diapason 0:"..total_ports-1);
        return nil;
    end
    
    tableSize = table.getn(portPolicy[portId+1]);
    if (tableSize ~= 0) then
        print("--> Error : port policy for port "..portId.." already exists");
        return nil;
    end
    
    specificPortPolicy = {};
    policy = {};
    
    token = Token_String(prs,":");
    if (token == nil) then
        print("--> Error : port policy clause [port PortNumber : .... ] expected");
        return nil;
    end
    policyInd = 1;
    while (1) do
        token = Test_Clause_Argument(prs,"end");
        if (token ~= nil) then
            break;
        end
        token = Test_Clause_Argument(prs,"port");
        if (token ~= nil) then
            break;
        end
        token = FileParser_GetToken(prs);
        speedStr = token;
        token = Token_String(prs,"-->");        
        if (token == nil) then
            print("--> Error : --> expected in specific port policy [speed <e.g 10G> --> slice ");
            return nil;
        end
        slicesNum = Token_Number(prs);                       
        if (slicesNum == nil) then
            print("--> Error : slice number expected in specific port policy [speed <e.g 10G> --> slice ");
            return nil;
        end
        if (slicesNum  <= 0) then
            print("--> Error : slice number shall be positive in specific port policy");
            return nil;
        end
        speed = ProcessProfile_Clause_SpecificPortPolicy_Speed(speedStr);        
        
        if (speed == nil) then
            print("--> Error : Unknown speed "..speedStr);
            return nil;
        end
        
        policyEntry = {};
        policyEntry.speed     = speed;
        policyEntry.slicesNum = slicesNum;        
        policy[policyInd] = policyEntry;
        policyInd = policyInd + 1;
        line = FileParser_GetLine(prs);
        if (line == nil) then
            print("--> Error : either port policy or end expected");
            return nil;
        end
    end
    specificPortPolicy.portId = portId;  
    specificPortPolicy.policy = policy;
    return specificPortPolicy;
end

local function ProcessProfile_Clause_PortPolicy(prs,total_ports)
    local portPolicy;
    local token;
    local line;
    local specificPortPolicy;

    token = Process_Clause_Argument(prs,"port-policy");
    if (token == nil) then
        return nil;
    end
    portPolicy = {};
    line = FileParser_GetLine(prs);
    if (line == nil) then
        print("--> Error : either 'port policy' clause or 'end' clause expected");
        return nil;
    end
    
    for portId=1,total_ports do
        portPolicy[portId] = {};
    end
    
    while (1) do
        token = Test_Clause_Argument(prs,"end");
        if (token ~= nil) then
            break;
        end
        specificPortPolicy = ProcessProfile_Clause_SpecificPortPolicy(prs,total_ports, portPolicy);       
        if (specificPortPolicy == nil) then
           return nil;
        end
        portPolicy[specificPortPolicy.portId+1] = specificPortPolicy.policy;
    end
    -- end already got , check that after end there is nothing
    token = FileParser_GetToken(prs);    -- "end" got
    token = FileParser_GetToken(prs);    
    if (token ~= nil) then
       print("--> Error : unknown token "..token.." after 'end'");
       return nil;
    end
    return portPolicy;
end

--[[
------------------------------------------------------------
  port priority
------------------------------------------------------------]]
local function ProcessProfile_Clause_SpecificPortPriority(prs,total_ports, portPriority)
    local specificPortPriority;
    local token;
    local portId;   
    local priorityList;
   
    
    token = FileParser_GetToken(prs);
    if (token ~= "port") then
        print("--> Error : port policy clause [port PortNumber : .... ] expected");
        return nil;
    end
    
    portId = Token_Number(prs);
    if (portId == nil) then
        print("--> Error : port id in policy clause expected");            
        return nil;
    end
    if (portId >= total_ports) then
        print("--> Error : port id "..portId.." shall be in diapason 0:"..total_ports-1);
        return nil;
    end
    if (portPriority[portId+1] ~= nil) then
        print("--> Error : port priority for port "..portId.." already exists");
        return nil;
    end
    
    specificPortPriority = {};
    priorityList = {};
    
    token = Token_String(prs,":");
    if (token == nil) then
        print("--> Error : port priority clause [port PortNumber : .... ] expected");
        return nil;
    end
    
    priorityList = Token_NumberList(prs);    
    
    specificPortPriority.portId = portId;  
    specificPortPriority.priorityList = priorityList;
    return specificPortPriority;
end


local function ProcessProfile_Clause_PortPriority(prs, total_ports)
    local line;  
    local token;
    local portPriority;
    local specificPortPriority;
   
   
    token = Process_Clause_Argument(prs,"port-priority");
    if (token == nil) then
        return nil;
    end
    portPriority = {};
   
    line = FileParser_GetLine(prs);
    if (line == nil) then
        print("--> Error : either 'port priority body' clause or 'end' clause expected");
        return 0,nil;
    end
    token = Test_Clause_Argument(prs,"none");
    if (token ~= nil) then
        token = FileParser_GetToken(prs);    -- "none" got
        token = FileParser_GetToken(prs);    
        if (token ~= nil) then
           print("--> Error : unknown token "..token.." after 'none'");
           return 0,nil;
        end
        token = Process_Clause_Argument(prs,"end");
        if (token == nil) then
            return 0,nil;
        end
        return 1,portPriority;
    end
    
    while (1) do
        token = Test_Clause_Argument(prs,"end");
        if (token ~= nil) then
            break;
        end
        specificPortPriority = ProcessProfile_Clause_SpecificPortPriority(prs,total_ports, portPriority);       
        if (specificPortPriority == nil) then
           return 0,nil;
        end
        portPriority[specificPortPriority.portId+1] = specificPortPriority.priorityList;
        line = FileParser_GetLine(prs);
        if (line == nil) then
            print("--> Error : either 'port priority body' clause or 'end' clause expected");
            return 0,nil;
        end
    end
    -- end already got , check that after end there is nothing
    token = FileParser_GetToken(prs);    -- "end" got
    token = FileParser_GetToken(prs);    
    if (token ~= nil) then
       print("--> Error : unknown token "..token.." after 'end'");
       return 0,nil;
    end
    return 1,portPriority;
end


--[[
+-----------------------------------------
|  clause profile-body
+-----------------------------------------]]
local function Process_Clause_Argument_Delimiter_Number(prs,argument,delimiter)
    local token;
    local number;
    
    token = FileParser_GetToken(prs);
    if (token ~= argument) then
        print("--> Error : "..argument.." expected");
        return nil;        
    end
    token = FileParser_GetToken(prs);
    if (token ~= delimiter) then
        print("--> Error : unknown delimiter "..token..", "..delimiter.." expected");      
        return nil;        
    end
    token = FileParser_GetToken(prs);
    if (token == nil) then
        print("--> Error : unknown token "..token..", number expected");            
        return nil;        
    end
    
    number = tonumber(token);
    if (number == nil) then
        return nil;
    end
    return number;
end

local function ProcessProfile_Clause_SpecificAllocation_FindPort(allocationList,portId)
    local listSize;
    listSize = table.getn(allocationList);
    if (listSize == 0) then
       return 0;
    end
    
    for idx = 1,listSize,1 do
        if (allocationList[idx].portId == portId) then
            return 1;
        end
    end
    return 0;
end

local function ProcessProfile_Clause_SpecificAllocation(prs,total_ports, total_slices)
    local line;
    local token;
    local specificAllocation;
    local sliceN;
    local idx;
    local portId;
    local slicesList;
    local allocationList;    
    local allocationListItem;
    local listSize;
    local rc;
    
    specificAllocation = {};
    
    sliceN = Process_Clause_Argument_Delimiter_Number(prs,"sliceN",":");    
    if (sliceN == nil) then
        print("--> Error : [sliceN : number] expected");
        return nil;
    end
    specificAllocation.sliceN = sliceN;
    specificAllocation.allocationList = {};
    
    allocationList = {};
    idx = 1;
    while (1) do
        line = FileParser_GetLine(prs);        
        if (line == nil) then
           print("--> Error : slice list for specific port expected");
           return nil;
        end
        token = FileParser_GetToken(prs);
        if (token ~= "port") then
            FileParser_UnGetToken(prs);          
            break;
        end
        
        portId = Token_Number_TestValue(prs,0,total_ports-1);
        if (portId == nil) then
            print("--> Error : port id expected");            
            return nil;
        end
        token = Token_String(prs,":");        
        if (token == nil) then
            print("--> Error : ':' expected in port allocatoin");            
            return nil;
        end
        slicesList = Token_NumberList_TestValue(prs,0,total_slices-1);
        if (slicesList == nil) then
            print("--> Error : Proper slice list expected");                      
            return nil;
        end
        listSize = table.getn(slicesList);
        if (listSize ~= sliceN) then
            print("--> Error : slice list size expected ",sliceN,"got ",listSize);                      
            return nil;
        end
        rc = ProcessProfile_Clause_SpecificAllocation_FindPort(allocationList,portId);
        if (rc == 1) then
            print("--> Error : port id "..portId.." already defined");                      
            return nil;
        end
        allocationListItem = {};
        allocationListItem.portId     = portId;
        allocationListItem.slicesList = slicesList;        
        
        
        allocationList[idx] = allocationListItem;
        idx = idx + 1;
    end
    
    specificAllocation.sliceN = sliceN;
    specificAllocation.allocationList = allocationList;
    
    return specificAllocation;
end 

local function ProcessProfile_Clause_AllocationByMode(prs,total_ports, total_slices)
    local line;
    local token;
    local allocation;
    local allocationId;
    local specificAllocation;
    allocation = {};
   
    line = FileParser_GetLine(prs);
    if (line == nil) then
        print("--> Error : either 'sliceN : integer ' clause expected");
        return 0,nil;
    end
    
    allocationId = 1;
    while (1) do
        token = Test_Clause_Argument(prs,"end");
        if (token ~= nil) then
            break;
        end
        
        specificAllocation = ProcessProfile_Clause_SpecificAllocation(prs,total_ports, total_slices);       
        if (specificAllocation == nil) then
           return nil;
        end
        allocation[allocationId] = specificAllocation;
        allocationId = allocationId + 1;        
    end
    -- end already got , check that after end there is nothing
    token = FileParser_GetToken(prs);    -- "end" got
    token = FileParser_GetToken(prs);    
    if (token ~= nil) then
       print("--> Error : unknown token "..token.." after 'end'");
       return 0,nil;
    end
    return allocation;
end

local function ProcessProfile_Clause_Allocation(prs,total_ports, total_slices)
    local token;
    local allocation_reg;
    local allocation_ext;  
    local allocation;
    
    token = Process_Clause_Argument(prs,"allocation");
    if (token == nil) then
        return nil;
    end
    token = Process_Clause_Argument_StringValue(prs,"mode","regular");    
    if (token == nil) then
        return nil;
    end
    allocation_reg = ProcessProfile_Clause_AllocationByMode(prs,total_ports, total_slices);
    if (allocation_reg == nil) then
       return nil;
    end
    token = Process_Clause_Argument_StringValue(prs,"mode","extended");    
    if (token == nil) then
        return nil;
    end
    allocation_ext = ProcessProfile_Clause_AllocationByMode(prs,total_ports, total_slices);
    if (allocation_ext == nil) then
       return nil;
    end
    
    token = Process_Clause_Argument(prs,"end");
    if (token == nil) then
        return nil;
    end
    allocation = {};
    allocation.reg = allocation_reg;
    allocation.ext = allocation_ext;
    return allocation;
end

--[[
+-----------------------------------------
|  clause profile-body
+-----------------------------------------]]
local function ProcessProfile_Clause_LowLevelProfileBody(prs)
    local profileBody   = {};
    local config        = {};
    local port_policy   = {};
    local port_priority = {};
    local allocation    = {};
    local rc;

    profileBody.type = "low-level";
    config = ProcessProfile_Clause_LowLevel_Config(prs);
    if (config == nil) then
        return nil;
    end

    port_policy = ProcessProfile_Clause_PortPolicy(prs,config.total_ports);
    if (port_policy == nil) then
        return nil;
    end
    
    rc, port_priority = ProcessProfile_Clause_PortPriority(prs,config.total_ports);
    if (rc == 0) then
        return nil;
    end
    
    allocation = ProcessProfile_Clause_Allocation(prs,config.total_ports, config.total_slices);
    if (allocation == nil) then
        return nil;
    end

    profileBody.config      = config;
    profileBody.port_policy = port_policy;
    profileBody.port_priority = port_priority;
    profileBody.allocation    = allocation;
    return profileBody;
end


local function ProcessProfile_Clause_Body(prs)
    local profileBody;
    local profileType;

    profileBody = {};

    profileType = Process_Clause_Argument_StringValue(prs,"type");
    if (profileType == nil) then
        print("--> Error : type [generation | low-level] expected");
        return nil;
    end

    if (profileType ~= "low-level" and profileType ~= "generation") then
        print("--> Error : unknown profile type : ", profileType, "  [generation | low-level] expected");
        return nil;
    end

    if (profileType == "generation") then
        print("--> Error : profile type 'generation' is not supported currently");
        return nil;
    end

    -- profile type is low-level
    profileBody = ProcessProfile_Clause_LowLevelProfileBody(prs);
    if (profileBody == nil) then
        return nil;
    end
    return profileBody;
end

--[[
+-----------------------------------------
|  profile
+-----------------------------------------]]
local function isEmptyIntersection(coreList1,coreList2)
    local N;
    local M;
    local coreid1;
    local coreid2;
    
    N = table.getn(coreList1);
    M = table.getn(coreList2);   
    for i = 1,N do
        coreid1 = coreList1[i];
        for j = 1,M do
            coreid2 = coreList2[j];            
            if (coreid1 == coreid2) then
                return 0;
            end
        end
    end
    return 1;
end

local function checkCoreList(coreList, profileList)
    local rc;
    local N;
    N = table.getn(profileList);
    if (N == 0) then
        return 1;
    end
    for n=1,N do
        rc = isEmptyIntersection(coreList,profileList[n].coreList)
        if (rc == 0) then
             print("--> Error : non empty intersection of core list");
             return 0;
        end
    end
    return 1;
end

local function ProcessProfile(prs, profileList)
    local token;
    local profile;
    local systemName;
    local coreList;
    local profileBody;
    local rc;
    local dev;

    token = Process_Clause_Argument(prs,"profile");
    if (token == nil) then
        return 1,nil;
    end
    profile = {};
    systemName = ProcessProfile_Clause_SystemDeclaration(prs);
    if (systemName == nil) then
       return 0,nil;
    end
    profile.systemName = systemName;

    dev = Process_Clause_Argument_IntegerValue(prs,"device");
    if (dev == nil) then
       print("--> Error : device id expected");
       return 0,nil;
    end

    coreList = ProcessProfile_Clause_Argument_IntegerArray(prs,"cores");
    if (coreList == nil) then
       print("--> Error : core list is not defined");
       return 0,nil;
    end
    rc = checkCoreList(coreList,profileList);
    if (rc == 0) then
        return 0,nil;
    end
      
    profileBody = ProcessProfile_Clause_Body(prs);
    if (profileBody == nil) then
        return 0,nil;
    end
    
    profile.dev      = dev;
    profile.coreList = coreList;          
    profile.config        = profileBody.config;
    profile.port_policy   = profileBody.port_policy;
    profile.port_priority = profileBody.port_priority;
    profile.allocation    = profileBody.allocation;


    token = Process_Clause_Argument(prs,"end");
    if (token == nil) then
        return 0,nil;
    end
    return 1,profile;
end

local function FileParser_BuildProfileList(filename)
    local profile;
    local prs;
    local profileList;
    local idx;
    local rc;

    prs = FileParser_Init(filename);
    if (prs == nil) then
        return nil;
    end

    profileList = {};
    
    idx = 1;
    while 1
    do
       profile = {};
       rc,profile = ProcessProfile(prs,profileList);
       if (rc == 0) then
           return nil;
       end
       -- rc = 1 there is either full profile or no profile
       if (profile == nil) then
           break;
       end;
       profileList[idx] = profile;
       idx = idx + 1;
    end
    return profileList;
end

-- ************************************************************************
-- Print CH files  
-- ************************************************************************

local function buildCFileanameByProfileFilename(filename)
    local ind;
    local lastInd = 0;
    local len;
    local fileNameWoExt = filename;
    local cFileName;    
    local hFileName;    
    local str;
    local profileName;
    
    str = filename;
    ind = string.find(str,"%.");
    if (ind ~= nil) then
        while ind ~= nil  do
            len = string.len(str);
            str = string.sub(str,ind+1,len);
            lastInd = lastInd + ind;       
            ind = string.find(str,"%.");        
        end 
        fileNameWoExt = string.sub(filename,1,lastInd-1);        
    end
    cFileName = fileNameWoExt .. ".c";
    hFileName = fileNameWoExt .. ".h";

    profileName = string.gsub(fileNameWoExt, "%.", "_");
    profileName = string.gsub(profileName, "-", "_");
    return profileName,cFileName,hFileName;
end

local function buildStringListOfCores(coreList)
    local coreListStr;
    local str;
    local coreN;
    
    coreN = table.getn(coreList);
    coreListStr = string.format("%d",coreList[1]);    
    
    for coreIdx = 2,coreN do
        str = string.format("_%d",coreList[coreIdx]);
        coreListStr = coreListStr .. str;
    end
    return coreListStr;
end


local function printExternalVariables(fid,profileList,profileName)
    local profile;
    local structName;
    local coreListStr;
    local profileN;
    
    File_PrintString(fid,string.format("\n"));
    
    profileN = table.getn(profileList);
    for n=1,profileN do
        profile = profileList[n];
        coreListStr = buildStringListOfCores(profile.coreList);        
        File_PrintString(fid,string.format("\n/*----------------------------------------------------------------"));
        File_PrintString(fid,string.format("\n * external variables core %s",coreListStr));    
        File_PrintString(fid,string.format("\n *----------------------------------------------------------------"));        
        File_PrintString(fid,string.format("\n */"));        
        structName = string.format("portGroupSpeed2SliceNumList_core_%s",coreListStr);    
        File_PrintString(fid,string.format("\nextern CPSS_DXCH_PORT_PIZZA_PORTGROUP_SPEED_TO_SLICENUM_STC %s;",structName));    
        structName = string.format("portConfig_core_%s",coreListStr);    
        File_PrintString(fid,string.format("\nextern CPSS_DXCH_PIZZA_PROFILE_STC                          %s;",structName));
    end
    
    File_PrintString(fid,string.format("\n                                                           "));
    File_PrintString(fid,string.format("\n                                                           "));
    File_PrintString(fid,string.format("\n                                                           "));            
    File_Flush(fid);
end


local function printHeader(fid,profileName)
    File_PrintString(fid,string.format("\n/*******************************************************************************"));
    File_PrintString(fid,string.format("\n*              (c), Copyright 2001, Marvell International Ltd.                 *"));
    File_PrintString(fid,string.format("\n* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *"));
    File_PrintString(fid,string.format("\n* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *"));
    File_PrintString(fid,string.format("\n* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *"));
    File_PrintString(fid,string.format("\n* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *"));
    File_PrintString(fid,string.format("\n* THIS CODE IS PROVIDED \"AS IS\". MARVELL MAKES NO WARRANTIES, EXPRESSED,     *"));
    File_PrintString(fid,string.format("\n* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *"));
    File_PrintString(fid,string.format("\n********************************************************************************"));
    File_PrintString(fid,string.format("\n* user defined profile genrated by profile %s                                   ",profileName));
    File_PrintString(fid,string.format("\n*                                                                               "));
    File_PrintString(fid,string.format("\n* DESCRIPTION:                                                                  "));
    File_PrintString(fid,string.format("\n*       User defined Pizza arbiter profiles generated from file %s              ",profileName));
    File_PrintString(fid,string.format("\n*                                                                               "));
    File_PrintString(fid,string.format("\n* FILE REVISION NUMBER:                                                         "));
    File_PrintString(fid,string.format("\n*       $Revision: 1 $                                                          "));
    File_PrintString(fid,string.format("\n*******************************************************************************/"));
    File_PrintString(fid,string.format("\n                                                                                 "));
    File_PrintString(fid,string.format("\n#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/cpssDxChPortPizzaArbiter.h>       "));
    File_PrintString(fid,string.format("\n#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/cpssDxChPortPizzaArbiterProfile.h>"));
    File_PrintString(fid,string.format("\n                                                                                    "));
    File_PrintString(fid,string.format("\n                                                                                    "));
end


--[[typedef enum
{
    CPSS_PORT_SPEED_10_E,       /* 0 */ 
    CPSS_PORT_SPEED_100_E,      /* 1 */ 
    CPSS_PORT_SPEED_1000_E,     /* 2 */ 
    CPSS_PORT_SPEED_10000_E,    /* 3 */ 
    CPSS_PORT_SPEED_12000_E,    /* 4 */ 
    CPSS_PORT_SPEED_2500_E,     /* 5 */ 
    CPSS_PORT_SPEED_5000_E,     /* 6 */ 
    CPSS_PORT_SPEED_13600_E,    /* 7 */ 
    CPSS_PORT_SPEED_20000_E,    /* 8 */ 
    CPSS_PORT_SPEED_40000_E,    /* 9 */ 
    CPSS_PORT_SPEED_16000_E,    /* 10 */
    CPSS_PORT_SPEED_15000_E,    /* 11 */
    CPSS_PORT_SPEED_75000_E,    /* 12 */
    CPSS_PORT_SPEED_100G_E,     /* 13 */
    CPSS_PORT_SPEED_50000_E,    /* 14 */
    CPSS_PORT_SPEED_140G_E,     /* 15 */
                                
    CPSS_PORT_SPEED_NA_E        /* 16 */

}CPSS_PORT_SPEED_ENT;
--]]
--[[
--------------------------------------------
    convert port speed in number of G to CPSS const 
    ex :  1 <--> 1G   --> CPSS_PORT_SPEED_1000_E
         10 <--> 10G  --> CPSS_PORT_SPEED_10000_E
        100 <--> 100G --> CPSS_PORT_SPEED_100G_E
--------------------------------------------]]

local function getPortSpeedString(portSpeed)
    if (portSpeed == 1) then  -- see CPSS_PORT_SPEED_ENT
        return "CPSS_PORT_SPEED_1000_E";
    end
    if (portSpeed == 5) then  -- see CPSS_PORT_SPEED_ENT
        return "CPSS_PORT_SPEED_5000_E";
    end
    if (portSpeed == 10) then
        return "CPSS_PORT_SPEED_10000_E";
    end
    if (portSpeed == 12) then
        return "CPSS_PORT_SPEED_12000_E";
    end
    if (portSpeed == 15) then
        return "CPSS_PORT_SPEED_15000_E";
    end
    if (portSpeed == 16) then
        return "CPSS_PORT_SPEED_16000_E";
    end
    if (portSpeed == 20) then
        return "CPSS_PORT_SPEED_20000_E";
    end
    if (portSpeed == 40) then
        return "CPSS_PORT_SPEED_40000_E";
    end
    if (portSpeed == 50) then
        return "CPSS_PORT_SPEED_50000_E";
    end
    if (portSpeed == 75) then
        return "CPSS_PORT_SPEED_75000_E";
    end
    if (portSpeed == 100) then
        return "CPSS_PORT_SPEED_100G_E";
    end
    return nil;
end

local function printProfileComment(fid,profile)
    local coreListStr;
    coreListStr = buildStringListOfCores(profile.coreList);    
    
    File_PrintString(fid,string.format("\n/*-----------------------------------------------------------------------------*"));
    File_PrintString(fid,string.format("\n * cores %s : speed 2 slice conversion table",coreListStr));
    File_PrintString(fid,string.format("\n *-----------------------------------------------------------------------------*/"));    
    File_Flush(fid);
end


local function printSlice2SpeedTable(fid,profile,profileName)
    local portN;
    local policy;
    local policyEntriesN;
    local policyEntry;
    local structName;
    local coreListStr;
    local speedStr;
    local sliceN;
    local listStructNames = {};

    coreListStr = buildStringListOfCores(profile.coreList);
    portN = table.getn(profile.port_policy);

    for port=1,portN do
        policy = profile.port_policy[port];
        policyEntriesN = table.getn(policy);
        if (policyEntriesN ~= nil and policyEntriesN > 0) 
        then
            structName = string.format("portSpeed2SliceNumList_core_%s_port_%d",coreListStr,port-1);
            listStructNames[port] = structName;
            
            File_PrintString(fid,string.format("\nstatic CPSS_DXCH_PORT_SPEED_TO_SLICE_NUM_STC %s[] = ",structName));
            File_PrintString(fid,string.format("\n{     /* port speed        ,     slices N */                                        "));
            
            policyEntry = policy[1];
            speedStr = getPortSpeedString(policyEntry.speed);
            sliceN   = policyEntry.slicesNum;
            File_PrintString(fid,string.format("\n      {   %30s,       %4d }",speedStr,sliceN));
            File_Flush(fid);
            
            for policyId=2,policyEntriesN do
                policyEntry = policy[policyId];
                speedStr = getPortSpeedString(policyEntry.speed);
                sliceN   = policyEntry.slicesNum;
                File_PrintString(fid,string.format("\n     ,{   %30s,       %4d }",speedStr,sliceN));
                File_Flush(fid);
            end    
            File_PrintString(fid,string.format("\n     ,{   %30s,       CPSS_INVALID_SLICE_NUM }     ","CPSS_PORT_SPEED_NA_E"));
            File_PrintString(fid,string.format("\n};                                                 "));
            File_PrintString(fid,string.format("\n                                                   "));
        else
            listStructNames[port] = "portSpeed2SliceNumLIst_NoALLOC";
        end
    end
    
    structName = string.format("portGroupSpeed2SliceNumList_core_%s",coreListStr);
    File_PrintString(fid,string.format("\n                                                           "));
    File_PrintString(fid,string.format("\n                                                           "));
    File_PrintString(fid,string.format("\nCPSS_DXCH_PORT_PIZZA_PORTGROUP_SPEED_TO_SLICENUM_STC %s =  ",structName));
    File_PrintString(fid,string.format("\n{                                                          "));
    File_PrintString(fid,string.format("\n     /* %d  */                                             ",portN));
    File_PrintString(fid,string.format("\n     {                                                     "));
    File_PrintString(fid,string.format("\n          { %3d, &%-40s[0] }",0,listStructNames[1]));
    for port=2,portN do
        File_PrintString(fid,string.format("\n         ,{ %3d, &%-40s[0] }",port-1,listStructNames[port]));
    end
    File_PrintString(fid,string.format("\n         ,{ CPSS_PA_INVALID_PORT  , NULL     }"));
    File_PrintString(fid,string.format("\n     }                                                     "));
    File_PrintString(fid,string.format("\n};                                                         "));
    File_PrintString(fid,string.format("\n                                                           "));
    File_Flush(fid);
end

local function PortPriorityList2String(portPriorityList)
    local portListStr;
    local str;
    local portN;
    
    portN = table.getn(portPriorityList);
    if (portN == 0) then
        portListStr = "CPSS_PA_INVALID_PORT";
        return portListStr;
    end
    
    portListStr = "";
    for portIdx = 1,portN do
        str = string.format(" %2d,",portPriorityList[portIdx]);
        portListStr = portListStr .. str;
    end
    portListStr = portListStr ..  " CPSS_PA_INVALID_PORT";
    return portListStr;
end

local function printPortPriorityMatrix(fid,profile,profileName)
    local portN;
    local structName;
    local coreListStr;
    local isTableEmpty = 0;
    local strPortList = {};

    coreListStr = buildStringListOfCores(profile.coreList);
    portN = table.getn(profile.port_priority);
    if (portN == nil or portN == 0) then
        isTableEmpty = 1;
    end

    File_PrintString(fid,string.format("\n                                                         "));    
    File_PrintString(fid,string.format("\n                                                         "));    
    File_PrintString(fid,string.format("\n/*-------------------------------------------------------"));
    File_PrintString(fid,string.format("\n * core %s : port priority Matrix",coreListStr));
    File_PrintString(fid,string.format("\n *-------------------------------------------------------"));
    File_PrintString(fid,string.format("\n */"));

    structName = "portPriority_core_"..coreListStr;
    
    portN = table.getn(profile.port_priority);
    File_PrintString(fid,string.format("\n                                                           "));    
    File_PrintString(fid,string.format("\nstatic CPSS_DXCH_PORT_PRIORITY_MATR_STC %s = ",structName));                
    File_PrintString(fid,string.format("\n{                                                              "));    
    File_PrintString(fid,string.format("\n/* GT_U32 portN;     */  %d                                    " ,profile.config.total_ports));    
    File_PrintString(fid,string.format("\n/* portPriorityList  */  ,{                                    "));
    File_Flush(fid);

    portN = profile.config.total_ports;
    for port=1,portN do
        File_PrintString(fid,string.format("\n    /* port %2d  */            ",port-1));
        if (port > 1) then
            File_PrintString(fid,string.format(","));
        else
            File_PrintString(fid,string.format(" "));
        end
        if (isTableEmpty == 1) then
            strPortList = " CPSS_PA_INVALID_PORT";                    
        else
            strPortList = PortPriorityList2String(profile.port_priority[port]);
        end
        File_PrintString(fid,string.format("{ %s }",strPortList));
        File_Flush(fid);
    end
    File_PrintString(fid,string.format("\n                          }                                    "));
    File_PrintString(fid,string.format("\n};    "));    
    File_Flush(fid);
    return     structName;
end


local function SliceList2String(sliceList)
    local sliceListStr;
    local str;
    local sliceN;
    
    if (sliceList == nil) then
       return "CPSS_PA_INVALID_SLICE";
    end    
    
    sliceN = table.getn(sliceList);
    if (sliceN == 0) then
        sliceListStr = "CPSS_PA_INVALID_SLICE";
        return sliceListStr;
    end
    
    sliceListStr = "";
    for sliceIdx = 1,sliceN do
        str = string.format(" %2d,",sliceList[sliceIdx]);
        sliceListStr = sliceListStr .. str;
    end
    sliceListStr = sliceListStr ..  " CPSS_PA_INVALID_SLICE";
    return sliceListStr;
end

local function printSliceAllocationByModeBySliceNum(fid,allocationBySlice,modeStr,profile,profileName)
    local coreListStr;
    local structName;
    local sliceN;
    local tblAllocList;
    local portNum;
    local portId;
        
    coreListStr = buildStringListOfCores(profile.coreList);    
    
    structName = string.format("portConfig_cores_%s_mode_%s_by_%dSlice",coreListStr,modeStr,allocationBySlice.sliceN);
    sliceN = table.getn(allocationBySlice.allocationList);
    
    
    tblAllocList = {};
    for i=1,profile.config.total_ports do
        tblAllocList[i] = {};
    end

    portNum = table.getn(allocationBySlice.allocationList);
    for i = 1,portNum do
        portId = allocationBySlice.allocationList[i].portId;
        tblAllocList[portId+1] = allocationBySlice.allocationList[i].slicesList;
    end
    
    File_PrintString(fid,string.format("\nstatic CPSS_DXCH_PORT_PIZZA_CFG_PORT_SLICE_LIST_STC %s = ",structName));            
    File_PrintString(fid,string.format("\n{ "));                
    File_PrintString(fid,string.format("\n/* maxPortNum*/ %d",profile.config.total_ports));                    
    File_PrintString(fid,string.format("\n/* portArr   */,{"));
    File_PrintString(fid,string.format("\n/* PORT %2d  */      { %s }",0,SliceList2String(tblAllocList[1])));    
    portNum = table.getn(tblAllocList);
    for port= 2,portNum do
        File_PrintString(fid,string.format("\n/* PORT %2d  */     ,{ %s }",port-1,SliceList2String(tblAllocList[port])));    
        File_Flush(fid);        
    end
    File_PrintString(fid,string.format("\n                }"));    
    File_PrintString(fid,string.format("\n};"));                    
    File_PrintString(fid,string.format("\n"));
    File_PrintString(fid,string.format("\n"));    
    File_Flush(fid);
    return structName;
    
end

local function printSliceAllocationByMode(fid,modeStr,profile,profileName)
    local coreListStr;
    local sliceConfigStructName;
    local allocation;
    local allocN;
    local structName;
    local allocListNames;
    local allocationItem;

    if (modeStr == "regular") then
        allocation = profile.allocation.reg;
    else
        allocation = profile.allocation.ext;
    end
   
    allocN = table.getn(allocation);
    if (allocN == 0) then -- no allocation 
        return nil;
    end

    allocListNames = {};
    for i = 1,allocN do
        structName = printSliceAllocationByModeBySliceNum(fid,allocation[i],modeStr,profile,profileName);
        allocationItem = {};
        allocationItem.sliceN     = allocation[i].sliceN;
        allocationItem.structName = structName;
        allocListNames[i]  = allocationItem;
    end
    
    coreListStr = buildStringListOfCores(profile.coreList);    
    sliceConfigStructName = string.format("portConfig_core_%s_mode_%s",coreListStr,modeStr);
    File_PrintString(fid,string.format("\nstatic CPSS_DXCH_PORT_PIZZA_SLICENUM_TO_SLICE_LIST_STC %s = ",sliceConfigStructName));            
    File_PrintString(fid,string.format("\n{                                 "));                
    File_PrintString(fid,string.format("\n/* portNum */       %d",profile.config.total_ports));
    File_PrintString(fid,string.format("\n/* maxSliceNum */  ,%d",profile.config.total_slices));    
    File_PrintString(fid,string.format("\n/* pizzaCfg */     ,{             "));        
    for i = 1,allocN do
        File_PrintString(fid,string.format("\n                      "));
        if (i > 1) then
            File_PrintString(fid,string.format(","));
        else
            File_PrintString(fid,string.format(" "));           
        end
        File_PrintString(fid,string.format("{ %2d , &%-40s }",allocListNames[i].sliceN,allocListNames[i].structName));                   
        File_Flush(fid);                                    
    end
    File_PrintString(fid,string.format("\n                      ,{  0 , NULL                                       }"));    
    File_PrintString(fid,string.format("\n                    }             "));            
    File_PrintString(fid,string.format("\n};                                "));                    
    File_PrintString(fid,string.format("\n                                  "));                        
    File_PrintString(fid,string.format("\n                                  "));
    File_Flush(fid);                            
    return sliceConfigStructName;
end

local function printSliceAllocation(fid,profile,profileName)
    local coreListStr;
    local extSliceAllocationName;
    local regSliceAllocationName;    

    coreListStr = buildStringListOfCores(profile.coreList);

    File_PrintString(fid,string.format("\n\n"));
    File_PrintString(fid,string.format("\n/*-------------------------------------------------------"));
    File_PrintString(fid,string.format("\n * core %s : port configuration",coreListStr));
    File_PrintString(fid,string.format("\n *-------------------------------------------------------"));
    File_PrintString(fid,string.format("\n */"));
    File_Flush(fid);     
    regSliceAllocationName = printSliceAllocationByMode(fid,"regular" ,profile,profileName);
    extSliceAllocationName = printSliceAllocationByMode(fid,"extended",profile,profileName);    
    return regSliceAllocationName,extSliceAllocationName;
end


local function printProfile(fid,profile,profileName)
    local portPriorityStructName;
    local extSliceAllocationName;
    local regSliceAllocationName;    
    local sliceConfigStructName;
    local cpuTypeStr;
    local cpuPortStr;
    local cpuSliceListStr;
    local coreListStr;


    printProfileComment(fid,profile);
    printSlice2SpeedTable(fid,profile,profileName);
    portPriorityStructName                        = printPortPriorityMatrix(fid,profile,profileName);
    regSliceAllocationName,extSliceAllocationName = printSliceAllocation(fid,profile,profileName);


    coreListStr = buildStringListOfCores(profile.coreList);    
    sliceConfigStructName = string.format("portConfig_core_%s",coreListStr);
    

    if  (profile.config.txqDef.cpu.type == "fixed") then
        cpuTypeStr      = "CPSS_DXCH_CPUPORT_TYPE_FIXED_E";
        cpuPortStr      = string.format("%d",profile.config.txqDef.cpu.port);
        cpuSliceListStr = SliceList2String(profile.config.txqDef.cpu.sliceList);
    elseif (profile.config.txqDef.cpu.type == "external") then
        cpuTypeStr      = "CPSS_DXCH_CPUPORT_TYPE_EXTERNAL_E";
        cpuPortStr      = string.format("%d",profile.config.txqDef.cpu.port);
        cpuSliceListStr = "CPSS_PA_INVALID_SLICE";
    else
        cpuTypeStr      = "CPSS_DXCH_CPUPORT_TYPE_UNDEFINED_E";
        cpuPortStr      = "CPSS_PA_INVALID_PORT";
        cpuSliceListStr = "CPSS_PA_INVALID_SLICE";
    end    
    
    File_PrintString(fid,string.format("\n\n"));
    File_PrintString(fid,string.format("\nCPSS_DXCH_PIZZA_PROFILE_STC  %s = ",sliceConfigStructName));
    File_PrintString(fid,string.format("\n{                           "));
    File_PrintString(fid,string.format("\n    /* portNum        */  %d",profile.config.total_ports));
    File_PrintString(fid,string.format("\n    /* maxSliceNum    */ ,%d",profile.config.total_slices));
    File_PrintString(fid,string.format("\n    /* txQDef         */ ,{       "));
    File_PrintString(fid,string.format("\n    /* TXQ_repetition */     %d   ",profile.config.txqDef.TxQ_repetition));
    File_PrintString(fid,string.format("\n    /*    TxQDef      */    ,{    "));
    File_PrintString(fid,string.format("\n    /*       CPU TYPE */         %s",cpuTypeStr));
    File_PrintString(fid,string.format("\n    /*       CPU Port */        ,%s",cpuPortStr));
    File_PrintString(fid,string.format("\n    /*       Slices   */        ,{ %s }",cpuSliceListStr));
    File_PrintString(fid,string.format("\n    /*                */     }     "));
    File_PrintString(fid,string.format("\n    /*                */  }        "));
    File_PrintString(fid,string.format("\n    /* reg mode       */ ,(CPSS_DXCH_PORT_PIZZA_SLICENUM_TO_SLICE_LIST_STC *)"));
    if (regSliceAllocationName ~= nil) then
        File_PrintString(fid,string.format("&%s",regSliceAllocationName));
    else
        File_PrintString(fid,string.format("NULL"));
    end
    File_PrintString(fid,string.format("\n    /* ext mode       */ ,(CPSS_DXCH_PORT_PIZZA_SLICENUM_TO_SLICE_LIST_STC *)"));    
    if (extSliceAllocationName ~= nil) then
        File_PrintString(fid,string.format("&%s",extSliceAllocationName));
    else
        File_PrintString(fid,string.format("NULL"));
    end
    File_PrintString(fid,string.format("\n    /* port priority  */ ,(CPSS_DXCH_PORT_PRIORITY_MATR_STC *)&%s",portPriorityStructName));
    File_PrintString(fid,string.format("\n};                           "));
    File_PrintString(fid,string.format("\n                             "));
    File_PrintString(fid,string.format("\n                             "));
    File_Flush(fid);     
end

local function WhetherNoALLOCNeed4Profile(profile)
    local answer4profile = 0;
    local port;
    local portN;
    local policy;
    local policyEntriesN;

    portN = table.getn(profile.port_policy);

    for port=1,portN do
        policy = profile.port_policy[port];
        policyEntriesN = table.getn(policy);
        if (policyEntriesN == nil or policyEntriesN == 0) then
            return 1; -- need
        end
    end
    return 0;
end


local function WhetherNoALLOCNeed(profileList)
    local profileId;
    local profileN;
    local profile;
    local answer4profile;
    local answer = 0;

    profileN = table.getn(profileList);
    for profileId = 1,profileN do
        profile = profileList[profileId];
        answer4profile = WhetherNoALLOCNeed4Profile(profile);
        answer = answer + answer4profile;
    end
    return answer;
end

local function printCFile(profileList,cFileName, profileName)
    local profileN;
    local profile;
    local fid;
    local errormsg;
    local whetherNoALLOCNeed;
    
    fid,errormsg = File_Open(cFileName,"w");
    if (fid == nil) then
        print(string.format("--> Error : %s : %s",cFileName,errormsg));
        --print(errormsg);
        return nil;
    end

    printHeader(fid,profileName);
    
    printExternalVariables(fid,profileList,profileName);
    
    whetherNoALLOCNeed = WhetherNoALLOCNeed(profileList);
    if (whetherNoALLOCNeed ~= 0) then
        File_PrintString(fid,string.format("\n                                                                                "));
        File_PrintString(fid,string.format("\nstatic CPSS_DXCH_PORT_SPEED_TO_SLICE_NUM_STC portSpeed2SliceNumLIst_NoALLOC[] = "));
        File_PrintString(fid,string.format("\n{     /* port speed        ,     slices N */                                    "));
        File_PrintString(fid,string.format("\n     {   CPSS_INVALID_SLICE_NUM , CPSS_PORT_SPEED_NA_E }                        "));
        File_PrintString(fid,string.format("\n};                                                                              "));
        File_PrintString(fid,string.format("\n                                                                                "));
        File_Flush(fid);
    end

    profileN = table.getn(profileList);
        
    for n=1,profileN do
        profile = profileList[n];
        printProfile(fid,profile,profileName);
    end

    File_Close(fid);
    return 0;    
end

local function printHFile(profileList,hFileName, profileName)
    local fid;
    local errormsg;
    
    fid,errormsg = File_Open(hFileName,"w");
    if (fid == nil) then
        print(string.format("--> Error : %s : %s",hFileName,errormsg));
        --print(errormsg);
        return nil;
    end

    printHeader(fid,profileName);
    File_PrintString(fid,string.format("\n#ifndef __USER_DEFINED_PROFILE_%s_H",profileName));
    File_PrintString(fid,string.format("\n#define __USER_DEFINED_PROFILE_%s_H",profileName));    
    File_PrintString(fid,string.format("\n"));    
    File_PrintString(fid,string.format("\n#ifdef __cplusplus           "));
    File_PrintString(fid,string.format("\nextern \"C\" {               "));
    File_PrintString(fid,string.format("\n#endif /* __cplusplus */     "));
    File_PrintString(fid,string.format("\n"));        
    
    printExternalVariables(fid,profileList,profileName);

    File_PrintString(fid,string.format("\n"));    
    File_PrintString(fid,string.format("\n#endif"));
    File_PrintString(fid,string.format("\n"));    
    
    File_Close(fid);
    return 0;
end


local function configurePizzaArbiterProfile(filename)
    local profileList;
    local rc;
    local cFileName;
    local hFileName;
    local profileName;
    
    profileList = FileParser_BuildProfileList(filename);
    if (profileList == nil) 
    then
        return nil,nil,nil;
    end
    
    if (g_standAlone == 0) then
        rc = wrLogWrapper("wrlDxChPortPizzaArbiterIfUserTableSet","(profileList)", profileList);
        if (rc ~= 0) then
            if (rc == 27) then
                print("--> Error : profile already exists");
            end
            return nil,nil,nil;
        end
    end
    
    profileName,cFileName,hFileName = buildCFileanameByProfileFilename(filename);
    print("Print C-file...");    
    rc = printCFile(profileList,cFileName,profileName);
    if (rc == nil) then
        return nil,nil,nil;
    end
    print("done");
    print("Print H-file...");   
    rc = printHFile(profileList,hFileName,profileName);
    if (rc == nil) then
        return nil,nil,nil;
    end
    print("done");
    return profileList,cFileName,hFileName;      
end

local function executeConfigurePizzaArbiterProfile(params)
    local profileList;
    local fileName;
    local tftpServer;
    local tftpPort;
    local ind;
    local len;
    local rc,err;
    local cFileName,hFileName;
    
    if (params["filename"] == nil) then  --load from file server
        return false, "file name absent"
    end

    
    len = string.len(params["filename"]);
    ind = string.find(params["filename"],":");
    
    tftpPort   = 69;            
    if (ind ~= nil) then
        tftpServer = string.sub(params["filename"],1,ind-1);
        fileName   = string.sub(params["filename"],ind+1,len);
        tftpPort   = 69;        
    else
        print("Params : "..to_string(params));    
        if (params["tftp-server"] ~= nil) then
            tftpServer = params["tftp-server"].string;
            if (params["port"] ~= nil) then
                tftpPort   = params["port"];
            end
        end
        fileName   = params["filename"];
        if (tftpServer ~= nil) then
            print("Server : "..to_string(tftpServer));
            print("Port   : "..to_string(tftpPort));
        end
        print("File   : "..fileName);
    end
    
    --tftpServer = params["tftp-server"];

    
    if (tftpServer ~= nil) then
        --print("Params : "..fileName.." : "..tftpServer);    
        rc, err = lua_tftp("get", tftpServer, tftpPort, fileName, fileName);
        if (err ~= nil) then
            print("tftp Error : ".. err);
            return;
        end
    end

    profileList,cFileName,hFileName = configurePizzaArbiterProfile(fileName);
    if (profileList == nil) then
        print("--> Error : failed to configure user defined pizza arbiter profile");
        return nil;
    else
        print("--> Success : profile configured");
    end
    -- save reults back 
    if (tftpServer ~= nil) then
        print("tftp put : "..cFileName.." : "..tftpServer);
        rc, err = lua_tftp("put", tftpServer, tftpPort, cFileName, cFileName);
        if (err ~= nil) then
            print("tftp Error : ".. err);
            return;
        end
        print("tftp put : "..hFileName.." : "..tftpServer);
        rc, err = lua_tftp("put", tftpServer, tftpPort, hFileName, hFileName);
        if (err ~= nil) then
            print("tftp Error : ".. err);
            return;
        end
    end
end

local function configureDefaultProfile()
    local params;

    params = {};
    params["filename"]    = "127.0.0.1:profile.txt";
    params["tftp-server"] = "127.0.0.1";
    
    executeConfigurePizzaArbiterProfile(params);
end

local function registerLuaCLICommand_configurePizzaArbiterProfile()
  CLI_addCommand( "debug"
                  ,"configure-pizza"
                  ,{
                      func = executeConfigurePizzaArbiterProfile,
                      help = "configure pizza arbiter profile from text file in RAMFS, [tftp-server:]filename",
                      params=
                      {
                         { type="values",     "%filename" }
                        ,{ type="named",
                             { format="from %ipv4",  name="tftp-server" ,help = "The ip to download the file from (optional)"}
                            ,{ format="port %l4port",name="port", help = "The file server port" }
                            ,requirements={port={"tftp-server"}} 
                         }
                      }
                   }
                );
end

if (g_standAlone == 0) then        
    cmdLuaCLI_registerCfunction("lua_tftp")
    cmdLuaCLI_registerCfunction("wrlDxChPortPizzaArbiterIfUserTableSet");
    registerLuaCLICommand_configurePizzaArbiterProfile();
    --configureDefaultProfile();    
else
    local profileList;
    --profileList = configurePizzaArbiterProfile("c:\\temp\\dtemp\\Dx_cpss_4.0_Lion2PALuaTablePrint_grigoryk_59\\application\\work\\VC10\\profile.txt");
    --profileList = configurePizzaArbiterProfile("profile.txt");
end


