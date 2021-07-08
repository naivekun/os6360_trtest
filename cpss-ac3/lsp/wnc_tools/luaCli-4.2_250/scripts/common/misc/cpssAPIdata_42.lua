if (dxCh_family == true) then
  if fs.exists("dxCh/misc/cpssAPIdata_42.lua") then
    require("dxCh/misc/cpssAPIdata_42")
  end
end

if (px_family == true) then
  if fs.exists("px/misc/cpssAPIdata_42.lua") then
    require("px/misc/cpssAPIdata_42")
  end
end
