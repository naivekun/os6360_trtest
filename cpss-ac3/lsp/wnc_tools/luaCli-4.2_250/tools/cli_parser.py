#!/usr/bin/env python

######################################################################
# cli_parser.py
#
# DESCRIPTION:
#       This script generate CLI description for the most common
#       command line syntax
#       Syntax examples (source strings):
#           [no] bridge aging %devID
#           bridge aging-time %seconds device %devId
#           show bridge address-table %devID [ vlan %vlan [ plus_minus] ] ( ethernet %port-range | port-channel %port-range ) [address %mac-address ] [ static | dynamic | all ] [ count ]
#
#
#       Command modifiers (applied to one command only):
#           *mode=name
#                     Set command mode (folder it is applied to, default "XXXX"
#           *modeSetCmd=command
#                     Use given command to switch to required mode
#                     Can be repeated to switch in more than one step
#                     Please note: this optional attribute will be used
#                     for UT or as sample
#           *help=help string
#                     Set help string for next command
#           *API=cpssApiName
#                     Add CPSS API call to function template
#           *param_help=help string
#                     Set help string for parameter (generic functions to set parameter)
#
#       Values for syntax check:
#           +TYPENAME valid list VALUES
#           +TYPENAME invalid list VALUES
#
#           Example:
#               +devID valid list 0
#               +devID invalid list a 10 -3
#
#           Usage: python cli_parser.py [input_file [ input_file ] ... ]
#           The output will be saved to file with .lua extension
#
#           Options:
#               -A        Scan CPSS sources for APIs"
#               -C path   Scan CPSS sources from different path"
#               -u        Enable UT generation
#               -U        UT generation to stdout
#
# INPUTS:
#       Sourse file:
#
# OUTPUTS:
#
# RETURNS:
#       none
#
# COMMENTS:
#       Requires PLY (Python Lex-Yacc) package
#           http://www.dabeaz.com/ply/
#       Tested with PLY 3.3
#           http://www.dabeaz.com/ply/ply-3.3.tar.gz
#       PLY installation rules:
#           1. unpack PLY:
#               tar xzf ply-3.3.tar.gz
#           2. install PLY (require root privilegies)
#               cd ply-3.3
#               python setup.py build
#               [sudo] python setup.py install
#
# FILE REVISION NUMBER:
#       $Revision: 1 $
#####################################################################

cpssRoot = "x:/cpss/SW/prestera"
cpssRoot = "/home/serg/cpss"

import sys, os
# look for ply in ply-3.3
sys.path.append(os.path.join(sys.path[0],"ply-3.3"))

####################################
### Lexical parsing
####################################

import ply.lex as lex

# token names
tokens = (
    'WORD',
    'VALUE',
    'LISTVALUE',
    'LPARENO',
    'RPARENO',
    'ALTERNATE',
    'LPARENM',
    'RPARENM',
    'CMDATTRIB',
    'AVALUE',
    'ENUMDEF'
)

# token definitions
t_WORD = r'[a-zA-Z_][-:@\w]*'
t_VALUE = r'%[a-zA-Z_][-\w]*'
t_LISTVALUE = r'@[a-zA-Z_][-\w]*'
t_LPARENO = r'\['
t_RPARENO = r'\]'
t_ALTERNATE = r'\|'
t_LPARENM = r'\('
t_RPARENM = r'\)'
t_CMDATTRIB = r'^\*(API|help|param_help|mode|modeSetCmd)=.*'
t_AVALUE = r'[-_0-9:\/][-:@\/\w]*'
t_ENUMDEF = r'^&enum'

# A string containing ignored characters (spaces and tabs)
t_ignore  = ' \t'

def t_COMMENT(t):
    r'\#.*'
    pass
    # no return value, comment discarded

# Error handling rule
def t_error(t):
    #print "Illegal character '%s'" % t.value[0]
    #t.lexer.skip(1)
    raise Exception("Illegal character '%s'" % t.value[0], t.lexpos)

# Build the lexer
lexer = lex.lex()

##############################################
# merge_dict
#
#    If p2 or p3 have the item keyed by name
#    then create such item into p1
##############################################
def merge_dict(p1,p2,p3,name):
    if name not in p2 and name not in p3:
        return
    p1[name] = dict()
    if name in p2:
        for k,v in p2[name].items():
            p1[name][k] = v
    if name in p3:
        for k,v in p3[name].items():
            p1[name][k] = v

##############################################
# copy if
#
#    If p2 item keyed by name
#    then copy reate such item into p1
##############################################
def copy_if(p1, p2, name):
    if name in p2:
        p1[name] = p2[name]




####################################
### Syntax rules
####################################

import ply.yacc as yacc

# command sttributes
cmd_attribs = dict()

# Enumerator for alternative group names
alt_counter = 1

def p_all(p):
    '''all : command
           | CMDATTRIB
           | enumdef'''
    if type(p[1]) == str:
        ff=p[1].find('=')
        if ff > 0:
            attrname = p[1][1:ff]
            if attrname == 'modeSetCmd':
                if attrname not in cmd_attribs:
                    cmd_attribs[attrname] = list()
                cmd_attribs[attrname].append(p[1][ff+1:])
            else:
                cmd_attribs[attrname] = p[1][ff+1:]
    else:
        p[0] = p[1]

def p_valueplus(p):
    '''valueplus : AVALUE
                 | WORD
                 | AVALUE valueplus
                 | WORD valueplus'''
    if len(p) > 2:
        p[0] = p[2]
    else:
        p[0] = list()
    p[0].insert(0,p[1])

def p_command(p):
    '''command : command_name
               | command_name params
               | '''
    if len(p) > 1:
        p[0] = { "type": "command", "command" : p[1] }
        if len(p) > 2:
            p[0]["params"] = p[2]

def p_command_name(p):
    'command_name : wordplus'
    p[0] = p[1]

def p_wordplus(p):
    '''wordplus : WORD
                | WORD wordplus'''
    if len(p) > 2:
        p[0] = p[1] + ' ' + p[2]
    else:
        p[0] = p[1]

def p_params(p):
    '''params : mandatory_params
              | named_params
              | mandatory_params named_params'''
    p[0] = [ p[1] ]
    if len(p) > 2:
        p[0].append(p[2])

def p_mandatory_params(p):
    '''mandatory_params : VALUE
                        | VALUE params_plus
                        | LISTVALUE
                        | LISTVALUE params_plus'''
    p[0] = { 'type' : 'values' } 
    if len(p) > 2:
        p[0]['l'] = p[2]
    else:
        p[0]['l'] = list()
    p[0]['l'].insert(0,p[1])

def p_params_plus(p):
    '''params_plus : VALUE params_plus
                   | LISTVALUE params_plus
                   | WORD params_plus
                   | VALUE
                   | LISTVALUE
                   | WORD'''
    if len(p) > 2:
        p[0] = p[2]
    else:
        p[0] = list()
    p[0].insert(0,p[1])

def p_named_params(p):
    '''named_params : LPARENO named_parameter RPARENO named_params
                    | LPARENM named_parameter RPARENM named_params
                    | LPARENO named_parameter RPARENO
                    | LPARENM named_parameter RPARENM'''

    p[0] = { 'type' : 'named' }
    if len(p) > 4:
        p[0]['l'] = p[4]["l"]
        merge_dict(p[0], p[2], p[4], "requirements")
        copy_if(p[0], p[4], "mandatory")
    else:
        p[0]['l'] = list()
        copy_if(p[0], p[2], "requirements")

    for i in range(0, len(p[2]['l'])):
        p[0]['l'].insert(i,p[2]['l'][i])
    if p[1] == '(':
        if "mandatory" not in p[0]:
            p[0]["mandatory"] = list()
        p[0]["mandatory"].append(p[2]["n"])

def p_named_parameter(p):
    '''named_parameter : parameter_noalt ALTERNATE named_parameter
                       | parameter_noalt'''
    p[0] = p[1].copy()
    if len(p) > 2:
        if "altname" in p[3]:
            altname = p[3]["altname"]
        else:
            global alt_counter
            altname = "alt%d"%alt_counter
            alt_counter += 1
        p[0]["altname"] = altname
        p[0]["n"] = altname
        p[0]["l"][0]["altname"] = altname
        first = True
        for i in p[3]["l"]:
            p[0]["l"].append(i)
            if first:
                p[0]["l"][-1]["altname"] = altname
            first = False
        merge_dict(p[0], p[1], p[3], "requirements")

def p_parameter_noalt(p):
    '''parameter_noalt : sentence
                       | sentence LPARENO named_parameter RPARENO'''
    if len(p) > 2:
        p[0] = dict()
        for k,v in p[3].items():
            if k != "l":
                p[0][k] = v
            else:
                p[0][k] = v[:]
    else:
        p[0] = dict()
        p[0]['l'] = list()

    p[0]['l'].insert(0, p[1])
    p[0]['n'] = p[1]["name"]
    if len(p) > 2:
        if "requirements" not in p[0]:
            p[0]["requirements"] = dict()
        for i in p[3]["l"]:
            p[0]["requirements"][i["name"]] = p[1]["name"]

def p_sentence(p):
    '''sentence : WORD sentence_plus
                | WORD
                | VALUE
                | LISTVALUE'''
    p[0] = { "format" : p[1] }
    if p[1][0] == "%" or p[1][0] == "@":
        p[0]["name"] = p[1][1:]
    else:
        p[0]["name"] = p[1]
    if len(p) > 2:
        p[0]["format"] += ' ' + p[2]

def p_sentence_plus(p):
    '''sentence_plus : VALUE sentence_plus
                     | LISTVALUE sentence_plus
                     | WORD sentence_plus
                     | VALUE
                     | LISTVALUE
                     | WORD'''
    if len(p) > 2:
        p[0] = p[1] + ' ' + p[2]
    else:
        p[0] = p[1]

def p_enumdef(p):
    '''enumdef : ENUMDEF WORD valueplus'''
    p[0] = { "type": "enumdef", "name" : p[2], "values": p[3] }



# Error rule for syntax errors
def p_error(p):
    tok = yacc.token()
    # tok.type: token type
    # tok.value: string
    # tok.lexpos: position in line
    #print "Syntax error in input! Token=",tok
    if tok == None:
        raise Exception("Syntax error at eol", -1)
    raise Exception("Syntax error", tok.lexpos)

# Build the parser
parser = yacc.yacc(write_tables=0, debug=0)



generate_function_stub = True
generate_UT = False
UT_TOOL = False
UT_seq_number = 1
function_descriptions = dict()

######################################################################
# try_generic_implementation
#
# DESCRIPTION:
#       try to use generic implementation
#
# INPUTS:
#       Sourse string
#       Parsed data
#
# RETURNS:
#       status(bool), CLI description string (Lua code) or None
#
# COMMENTS:
#
#####################################################################
def try_generic_implementation(str, result):
    if "API" not in cmd_attribs:
        return False, ""
    API = cmd_attribs["API"]
    if API not in function_descriptions:
        return False, "-- wrong API: "+API+"\n"

    descr = function_descriptions[API]
    if "params" not in descr:
        return False, ""
    # check if generic case applicable

    def atomic_type(t):
        return not t.endswith("_STC")
    def basic_int(t):
        return t in ['GT_8','GT_U8','GT_16','GT_U16','GT_32','GT_U32']

    def generic_implementation(type, defparams, param_name, param_type):
        out  = "-- "+result["command"]+defparams+"\n"
        out += 'print("Warning: The command : '+result["command"]+'")\n'
        out += 'print("    is implemented using generic implementation.")\n'
        out += 'print("    Remove this text if you sure its applicable")\n'
        command_mode = "nil"
        if 'mode' in cmd_attribs:
            command_mode = '"'+cmd_attribs['mode']+'"'
        out += 'CLI_add_'+type+'('+command_mode+', "'+result["command"]+'",\n'
        out += '    "'+API+'",\n'
        out += '    "'+param_name+'",\n'
        out += '    "'+param_type+'",\n'
        if "help" in cmd_attribs:
            out += '    "'+cmd_attribs["help"]+'",\n'
        else:
            out += '    nil,\n'
        if "param_help" in cmd_attribs:
            out += '    "'+cmd_attribs["param_help"]+'"\n'
        else:
            out += '    nil\n'
        out += ')\n'
        return out
    p = descr["params"]
    if len(p) == 1:
        if API.endswith("Get") and p[0][0] == 'OUT' and atomic_type(p[0][1]):
            return True, generic_implementation(
                    "Show_parameter_global",
                    "", p[0][2], p[0][1])
        if API.endswith("Set") and p[0][0] == 'IN' and atomic_type(p[0][1]):
            return True, generic_implementation(
                    "Set_parameter_global",
                    "%"+p[0][1],
                    p[0][2], p[0][1])
    if len(p) == 2:
        if p[0][0] == "IN" and basic_int(p[0][1]) and p[0][2] in ['dev','devNum','devId','devID','deviceId','deviceID']:
            if API.endswith("Get") and p[1][0] == 'OUT' and atomic_type(p[1][1]):
                return True, generic_implementation(
                        "Show_parameter_per_device",
                        " [ device %devID ]",
                        p[1][2], p[1][1])
            if API.endswith("Set") and p[1][0] == 'IN' and atomic_type(p[1][1]):
                return True, generic_implementation(
                        "Set_parameter_per_device",
                        " [ device %devID ] %"+p[1][1],
                        p[1][2], p[1][1])
    if len(p) == 3:
        if p[0][0] == "IN" and basic_int(p[0][1]) and p[0][2] in ['dev','devNum','devId','devID','deviceId','deviceID'] and \
           p[1][0] == "IN" and basic_int(p[1][1]) and p[1][2] in ['port','portNum']:
            if API.endswith("Get") and p[2][0] == 'OUT' and atomic_type(p[2][1]):
                return True, generic_implementation(
                        "Show_parameter_per_port",
                        " [ device %devID ] [ port %number ]",
                        p[2][2], p[2][1])
            if API.endswith("Set") and p[2][0] == 'IN' and atomic_type(p[2][1]):
                return True, generic_implementation(
                        "Set_parameter_per_port",
                        " [ device %devID ] [ port %number ] %"+p[2][1],
                        p[2][2], p[2][1])
    return False, ""


######################################################################
# generate_descr
#
# DESCRIPTION:
#       Generate CLI description for passed string
#
# INPUTS:
#       Sourse string
#
# RETURNS:
#       CLI description string (Lua code) or None
#
# COMMENTS:
#
#####################################################################
def generate_descr(str):
    global parser, cmd_attribs
    global function_descriptions
    if str.startswith('+'):
        # valid/invalid values
        p = str[1:].split()
        if p[1] not in ['valid', 'invalid'] or p[2] not in ['list','range']:
            # raise an error
            print "wrong keyword"
            return None, None
        if "values" not in cmd_attribs:
            cmd_attribs["values"] = dict()
        if p[0] not in cmd_attribs["values"]:
            cmd_attribs["values"][p[0]] = { 'valid' : [], 'invalid' : [] }
        if p[2] == 'list':
            for v in p[3:]:
                cmd_attribs["values"][p[0]][p[1]].append(v)
        if p[2] == 'range':
            # raise an error
            print "TBD"
        return None,None
    result = parser.parse(str, tracking=True)
    if result == None:
        return None, None
    def indent(i):
        return '                      '[:i*2]
    if result["type"] == "enumdef":
        #print "*** result=",result
        out  = '-- '+str+'\n'
        out += 'CLI_type_dict["'+result["name"]+'"] = {\n'
        out += '    checker = CLI_check_param_enum,\n'
        out += '    complete = CLI_complete_param_enum,\n'
        out += '    enum {'
        first = True
        for v in result["values"]:
            if not first:
                out += ','
            first = False
            out += '\n        ["'+v+'"] = { value="'+v+'"}'
        out += '\n    }\n'
        out += '}\n'
        return out,None
    if result["type"] != "command":
        return '',None
    status, out = try_generic_implementation(str, result)
    if status:
        return out,None
    out += "-- "+str+"\n"
    if generate_function_stub:
        funcname = result["command"].replace(" ", "_")+"_func"
        funcname = funcname.replace("-", "_")
        out += "function "+funcname+"(params)\n"
        out += "    -- add implementation here\n"
        if "API" in cmd_attribs and cmd_attribs["API"] in function_descriptions:
            descr = function_descriptions[cmd_attribs["API"]]
            out += '    -- cpssGenWrapper("'+cmd_attribs["API"]+'", {\n'
            if "params" in descr:
                first = True
                for param in descr["params"]:
                    if not first:
                        out += ',\n'
                    first = False
                    out += '    --     { "'+param[0]+'", "'+param[1]+'", "'+param[2]+'"'
                    if param[0] == "IN" or param[0] == "INOUT":
                        out += ', '+param[2]
                    out += ' }'
            out += '\n    -- })\n'
        out += "end\n"
    else:
        funcname = "XXXX"
    command_mode = "XXXX"
    if 'mode' in cmd_attribs:
        command_mode = cmd_attribs['mode']
    out += 'CLI_addCommand("'+command_mode+'", "'+result["command"]+'", {\n'
    out += indent(1)+'func='+funcname
    if "help" in cmd_attribs:
        out += ",\n"+indent(1)+'help="'+cmd_attribs["help"]+'"'
    if generate_UT:
        UT_params_errors = list()
        if "values" not in cmd_attribs:
            cmd_attribs["values"] = dict()
        UT_params = list()
    if "params" in result:
        out += ',\n'
        out += indent(1)+'params={\n'
        first_section = True
        for p in result["params"]:
            if not first_section:
                out += ',\n'
            first_section = False
            out += indent(2)+'{ type="'+p["type"]+'"'
            if p["type"] == "values":
                for i in p["l"]:
                    out += ',\n'+indent(3)+'"'+i+'"'
                    # generate UT_params list
                    if generate_UT:
                        UT_param = list()
                        UT_param.append({"str" : "", "valid" : False })
                        if i[0] == '%':
                            if i[1:] not in cmd_attribs["values"]:
                                UT_params_errors.append("No values specified for parameter "+i[1:])
                                continue
                            for v in cmd_attribs["values"][i[1:]]["valid"]:
                                UT_param.append({"str" : " "+v, "valid" : True })
                            for v in cmd_attribs["values"][i[1:]]["invalid"]:
                                UT_param.append({"str" : " "+v, "valid" : False })
                        else:
                            UT_param.append({"str" : ' '+i, "valid" : True})
                        UT_params.append({"name":i, "values" : UT_param})
            elif p["type"] == "named":
                for i in p["l"]:
                    out += ',\n'+indent(3)+'{ format="'+i["format"]+'"'
                    if "altname" in i:
                        out += ', name="'+i["altname"]+'"'
                    else:
                        if i["format"][0] == "%" or i["format"][0] == "@":
                            out += ', name="'+i["name"]+'"'
                    # flag multiplevalues
                    nvalues = 0
                    for v in i["format"].split(" "):
                        if v[0] == "%":
                            nvalues = nvalues + 1
                    if nvalues > 1:
                        out += ', multiplevalues=true'
                    out += ' }'
                    # generate UT param list
                    if generate_UT:
                        UT_param = list()
                        UT_param.append({"str" : "", "valid" : True })

                        def do_val(name, fmt, idx, str, valid):
                            str += " "
                            if idx >= len(fmt):
                                UT_param.append({"str": str, "valid": valid })
                                return True
                            if fmt[idx][0] == "%":
                                name
                                nm = name+"_"+fmt[idx][1:]
                                if nm not in cmd_attribs["values"]:
                                    nm = name
                                if nm not in cmd_attribs["values"]:
                                    UT_params_errors.append("No values specified for parameter "+nm)
                                    return False
                                for v in cmd_attribs["values"][nm]["valid"]:
                                    if not do_val(name,fmt,idx+1, str+v, valid):
                                        return False
                                for v in cmd_attribs["values"][nm]["invalid"]:
                                    if not do_val(name,fmt,idx+1,str+v,False):
                                        return False
                                return True
                            else:
                                str += fmt[idx]
                                return do_val(name, fmt, idx+1, str, valid)
                        do_val(i["name"],i["format"].split(" "),0,"",True)
                        UT_params.append({"name":i["name"], "values" : UT_param})
                        if "altname" in i:
                            UT_params[-1]["altname"] = i["altname"]
                        if "mandatory" in p:
                            UT_params[-1]["mandatory"] = p["mandatory"]
                        if "requirements" in p:
                            UT_params[-1]["requirements"] = p["requirements"]


                if "mandatory" in p:
                    out += ',\n'+indent(3)+'mandatory = { "'+'", "'.join(p["mandatory"])+'" }'
                if "requirements" in p:
                    out += ',\n'+indent(3)+'requirements = {\n'
                    first = True
                    for k,v in p["requirements"].items():
                        if not first:
                            out += ',\n'
                        first = False
                        out += indent(4)+'["'+k+'"] = { "'+v+'" }'
                    out += '\n'+indent(3)+'}'
            else:
                raise "internal error: section_type="+p["type"]
            out += '\n'
            out += indent(2)+'}'
        out += '\n'
        out += indent(1)+'}'
    out += '\n'
    out += '})'
    if not generate_UT:
        return out, None
    #print "***UT_params=",UT_params
    def UT_iterate(lvl, s, flg, names):
        r = ''
        name = UT_params[lvl]["name"]
        altname = name
        if "altname" in UT_params[lvl]:
            altname = UT_params[lvl]["altname"]
        #print "** lvl=",lvl,"names=",names,"p=",UT_params[lvl]
        for p in UT_params[lvl]["values"]:
            nflg = flg and p["valid"]
            nms = names[:]
            # apply mandatory, alt rules, requirements
            if p["str"] == "":
                # apply mandatory rule
                #if "mandatory" in UT_params[lvl]:
                #    if altname in UT_params[lvl]["mandatory"]:
                #        if altname not in nms:
                #            nflg = False
                #            print "p1: name="+name+" m=",UT_params[lvl],"p=",p
                pass
            else:
                # apply alt rule
                if name in nms:
                    nflg = False
                    #print "a: lvl=",lvl,"name=",name,"nms=",nms
                else:
                    if name in nms:
                        nflg = False
                    else:
                        nms.append(name)
                    if altname != name:
                        if altname in nms:
                            nflg = False
                        else:
                            nms.append(altname)
                # apply requirements rule
                if "requirements" in UT_params[lvl]:
                    if name in UT_params[lvl]["requirements"]:
                        if UT_params[lvl]["requirements"][name] not in names:
                            nflg = False
            if lvl+1 < len(UT_params):
                r += UT_iterate(lvl+1, s+p["str"], nflg, nms)
            else:
                if "mandatory" in UT_params[lvl]:
                    for m in UT_params[lvl]["mandatory"]:
                        if m not in nms:
                            nflg = False
                if nflg:
                    r += '    UT_syntax_expect_ok'
                else:
                    r += '    UT_syntax_expect_fail'
                r += ' "'+result["command"]+s+p["str"]+'"\n'
        return r
    UT_out = "# "+result["command"]+"\n"
    global UT_seq_number
    if UT_TOOL:
        if "modeSetCmd" in cmd_attribs:
            for msetcmd in cmd_attribs["modeSetCmd"]:
                UT_out += 'UT_do_command "'+msetcmd+'"\n'
            UT_out += 'UT_test_P'
        elif "mode" in cmd_attribs:
            UT_out += 'UT_test_in_mode_P "'+cmd_attribs["mode"]+'"'
        else:
            UT_out += 'UT_test_P'
        UT_out += ' n%d'%UT_seq_number+' suite_syntax_n%d'%UT_seq_number
        UT_out += ' "'+result["command"]+'"\n'
    else:
        UT_out += 'proc suite_syntax_n%d {} {\n'%UT_seq_number
    UT_out += '    send_user "*** test syntax: '+result["command"]+'\\n"\n'
    if len(UT_params_errors) != 0:
        if not UT_TOOL:
            print "***UT generation error for '"+result["command"]+"':\n   "+"\n   ".join(UT_params_errors)
            return out, None
        UT_out += '    send_log "*** Test generation error:"\n'
        UT_out += '    send_log "    '+ '"\n    send_log "    '.join(UT_params_errors)+'"\n'
        UT_out += '    UT_event FAILED "generation error, skipped"\n'
    else:
        if len(UT_params) > 0:
            UT_out += UT_iterate(0, "", True, [])
        else:
            UT_out += '    UT_syntax_expect_ok "'+result["command"]+'"\n'
        UT_out += '    return 1\n'
    if UT_TOOL:
        UT_out += "#UT_END_TEST\n"
        if "modeSetCmd" in cmd_attribs:
            for msetcmd in cmd_attribs["modeSetCmd"]:
                UT_out += 'UT_do_command "exit"\n'
    else:
        UT_out += '}\n'
        if "modeSetCmd" in cmd_attribs:
            for msetcmd in cmd_attribs["modeSetCmd"]:
                UT_out += 'UT_do_command "'+msetcmd+'"\n'
            UT_out += 'UT_test'
        elif "mode" in cmd_attribs:
            UT_out += 'UT_test_in_mode "'+cmd_attribs["mode"]+'"'
        else:
            UT_out += 'UT_test'
        UT_out += ' n%d'%UT_seq_number+' suite_syntax_n%d'%UT_seq_number
        UT_out += ' "'+result["command"]+'"\n'
        if "modeSetCmd" in cmd_attribs:
            for msetcmd in cmd_attribs["modeSetCmd"]:
                UT_out += 'UT_do_command "exit"\n'
    UT_seq_number = UT_seq_number + 1
    #print "***UT_out=",UT_out
    return out,UT_out



##test
#
#str = '[no] bridge aging %devID'
#print "+++"
#print generate_descr(str)
#
#str = 'bridge aging-time %seconds device %devId'
#print "+++"
#print generate_descr(str)
#
#
#str = 'show bridge address-table %devID [ vlan %vlan [ plus_minus] ] ( ethernet %port-range | port-channel %port-range ) [address %mac-address ] [ static | dynamic | all ] [ count ]'
##print str
##result = parser.parse(str)
##print result
#
#print "+++"
#print generate_descr(str)

def do_file(input, output, ut_output):
    global cmd_attribs
    line_number = 0
    while True:
        line = input.readline()
        if line == "":
            return
        line_number += 1
        if line.endswith("\n"):
            line = line[:-1]
        if line.endswith("\r"):
            line = line[:-1]
        if line == "":
            continue
        # contignous lines
        while line.endswith("\\") and not line.rstrip().startswith("#"):
            lnext = input.readline()
            # if lnext == "":
            #     print "Error: unsinished contignous line"
            if lnext.endswith("\n"):
                lnext = lnext[:-1]
            if lnext.endswith("\r"):
                lnext = lnext[:-1]
            line_number += 1
            line = line[:-1].rstrip()+' '+lnext.lstrip()

        try:
            descr,UT_out = generate_descr(line)
            if descr != None:
                if output != None:
                    print >>output, descr
                    print >>output, ""
                if generate_UT and UT_out != None:
                    print >>ut_output, UT_out
                    print >>ut_output, ""
                # clean attributes
                cmd_attribs = dict()
        #except Exception, (strerror,strpos):
        except Exception, err:
            if len(err.args) == 2:
                strerror = err.args[0]
                strpos = err.args[1]
                print err
                if output != None:
                    print >>output, "-- Wrong line:"
                    print >>output, "-- "+line
                    print >>output, ""
                print line
                print strerror+" at line %d position %d"%(line_number, strpos)+" at '"+line[strpos:strpos+10]+"':"
            else:
                raise
            



import getopt

#####################
### Main
#####################
if __name__ == '__main__':
    # parse cmdline args
    load_function_decls = False
    opts, args = getopt.getopt(sys.argv[1:], "C:SAuU?")
    for (opt, val) in opts:
        if opt == "-S":
            generate_function_stub = False
        elif opt == "-A":
            load_function_decls = True
        elif opt == "-C":
            load_function_decls = True
            cpssRoot = val
        elif opt == "-u":
            generate_UT = True
        elif opt == "-U":
            generate_UT = True
            UT_TOOL = True

        elif opt == '-?':
            print "Usage: "+sys.argv[0]+"[-S] [-C cpssRoot ] [-A] [ input_file [ input_file ] ... ]"
            print "      -C path  Scan CPSS sources from different path"
            print "      -A       Scan CPSS sources for APIs"
            print "      -S       don't generate implementation stub"
            print "      -u       Enable UT generation"
            print "      -U       UT generation to stdout"
            print "The output will be saved to file with .lua extension"
            sys.exit(0)

    if load_function_decls:
        from scan_utils import *
        _reArrayParameter = re.compile(r"(\w+)\[\w*\]")

        for srcpath in all_files(cpssRoot,"*.h"):
            source = readfile(srcpath)
            for decl in scan_c_src(source):
                name = decl['name']
                descr = dict()
                descr["params"] = list()
                if "params" in decl:
                    for i in range(len(decl['params'])):
                        param = decl["params"][i]
                        pdir = param[0]
                        ptype = param[1]
                        pname = param[2]
                        # handle pointers
                        if ptype.endswith("*"):
                            ptype = ptype[:-1].strip()
                        if pname.startswith("*"):
                            pname = pname[1:].strip()
                        if pname.endswith("Ptr"):
                            pname = pname[:-3]
                        # handle arrays
                        m = _reArrayParameter.match(pname)
                        if m != None:
                            pname = m.group(1).strip()
                        descr["params"].append([pdir,ptype,pname])
                function_descriptions[name] = descr

    if len(args) > 0:
        for file_name in args:
            try:
                input = open(file_name, "r")
            except IOError, (errno, strerror):
                print "Failed to open input file "+file_name+": "+strerror
                continue
            if not UT_TOOL:
                (nm, ext) = os.path.splitext(file_name)
                output_name = nm + ".lua"
                try:
                    output = open(output_name, "w")
                except IOError, (errno, strerror):
                    print "Failed to open output file "+output_name+": "+strerror
                    input.close()
                    continue
                if generate_UT:
                    ut_output_name = nm + "_UT.exp"
                    try:
                        ut_output = open(ut_output_name, "w")
                    except IOError, (errno, strerror):
                        print "Failed to open UT output file "+ut_output_name+": "+strerror
                        input.close()
                        output.close()
                        continue
                else:
                    ut_output = None
                msg = "do "+file_name+" => "+output_name
                if generate_UT:
                    msg += ","+ut_output_name
                print msg
                do_file(input, output, ut_output)
                output.close()
                if generate_UT:
                    ut_output.close()
            else:
                do_file(input, None, sys.stdout)
            input.close()
    else: # len(args) == 0
        print "No input files specified, STDIN used as input, STDOUT is output"
        do_file(sys.stdin, sys.stdout, sys.stdout)

