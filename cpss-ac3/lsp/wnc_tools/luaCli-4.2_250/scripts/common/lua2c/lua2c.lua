-- lua2c.lua - Driver for lua2c - converts Lua 5.1 source to C code.
--
-- STATUS:
--   WARNING: This code passes much of the Lua 5.1 test suite,
--   but there could still be errors.  In particular, a few
--   language features (e.g. coroutines) are not implemented.
--
--   Unimplemented Lua language features:
--    - deprecated old style vararg (arg) table inside vararg functions
--      (LUA_COMPAT_VARARG)
--    - debug.getinfo(f, 'n').name for C-based functions
--    - setfenv does not permit C-based functions
--    - how well do tail call optimizations work?
--    - how to handle coroutines? (see README)
--    Note: A few things (coroutines) might remain
--      unimplemented--see README file file for details.
--
--   Possible improvements:
--    - Fix: large numerical literals can give gcc warnings such as
--      'warning: integer constant is too large for "long" type').
--      Literal numbers are rendered as C integers literals (e.g. 123)
--      rather than C double literals (eg. 123.0).  
--    - improved debug tracebacks on exceptions
--    - See items marked FIX in below code.
--
-- SOURCE:
--
--   http://lua-users.org/wiki/LuaToCee
--
--   (c) 2008 David Manura.  Licensed in the same terms as Lua (MIT license).
--   See included LICENSE file for full licensing details.
--   Please post any patches/improvements.
--

local _G           = _G
local assert       = _G.assert
local error        = _G.error
local io           = _G.io
local ipairs       = _G.ipairs
local os           = _G.os
local package      = _G.package
local require      = _G.require
local string       = _G.string
local table        = _G.table

package.path = './common/lua2c/lib/?.lua;' .. package.path

-- note: includes gg/mlp Lua parsing Libraries taken from Metalua.
require "lexer"
require "gg"
require "mlp_lexer"
require "mlp_misc"
require "mlp_table"
require "mlp_meta"
require "mlp_expr"
require "mlp_stat"
require "mlp_ext"
_G.mlc = {} -- make gg happy
local mlp = assert(_G.mlp)
local A2C = require "lua2c.ast2cast"
local C2S = require "lua2c.cast2string"

local function NOTIMPL(s)
  error('FIX - NOT IMPLEMENTED: ' .. s, 2)
end

local function DEBUG(...)
  local ts = {...}
  for i,v in ipairs(ts) do
    ts[i] = table.tostring(v,'nohash',60)
  end
  -- io.stderr:write(table.concat(ts, ' ') .. '\n')
  print( tostring(table.concat(ts, ' ')) .. '\n')
end

-- Converts Lua source string to Lua AST (via mlp/gg)
local function string_to_ast(src)
  local  lx  = mlp.lexer:newstream (src)
  local  ast = mlp.chunk (lx)
  return ast
end


function lua2c(luaFileName, outFileName)
  local fstat = fs.stat(luaFileName)
  if fstat == nil then
    print("Error: Input file " .. tostring(luaFileName) .. " not found.")
    return false
  end
  
  local fsize = fstat.st_size
  local inFile = assert(fs.open (luaFileName, 'r'))
  local luaSrc = fs.read(inFile, fsize)
  fs.close(inFile)

  luaSrc = luaSrc:gsub('^#[^\r\n]*', '') -- remove any shebang

  local ast = string_to_ast(luaSrc)

  local cast = A2C.ast_to_cast(luaSrc, ast)
  
  -- DEBUG(cast)
  -- create outFile
  local outFile, e = fs.open("traceCpssApi.tmp", "w")
  assert(fs.write(outFile, C2S.cast_to_string(cast)))
  fs.close(outFile)
  return true
end
