-- use LuaExpat and Lua-CuRL together for On-The-Fly XML parsing
require("lxp")

callback = {}
tags = {}			
items = {}

function callback.StartElement(parser, tagname)
   tags[#tags + 1] = tagname
   if (tagname == "item") then
      items[#items + 1] = {}
   end
end

function callback.CharacterData(parser, str) 
   if (tags[#tags -1] == "item") then
      --we are parsing a item
      items[#items][tags[#tags]] = str
   end
end
function callback.EndElement(parser, tagname)
   tags[#tags] = nil		--assuming well formed xml
end


p = lxp.new(callback)

f = io.open("slashdot", "r")

for l in f:lines() do 
   assert(p:parse(l))
--    assert(p:parse("\n"))
   
end

assert(p:parse())		--finish document
p:close()

for i, item in ipairs(items) do 
   for k, v in pairs(item) do 
      print(k,v)
   end
   print()			--newline
end
