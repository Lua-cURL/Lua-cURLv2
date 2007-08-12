-- use LuaExpat and Lua-CuRL together for On-The-Fly XML parsing
require("lxp")
require("cURL")

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
      --we are parsing a item, get rid of trailing whitespace
      items[#items][tags[#tags]] = string.gsub(str, "%s*$", "")
   end
end
function callback.EndElement(parser, tagname)
   tags[#tags] = nil		--assuming well formed xml
end


p = lxp.new(callback)

-- create and setup easy handle
c = cURL.easy_init()
c:setopt_url("http://www.lua.org/news.rss")

m = cURL.multi_init()
m:add_handle(c)

for data,type in m:perform() do 
   -- ign "header"
   if (type == "data") then 
      assert(p:parse(data))
   end
end

assert(p:parse())		--finish document
p:close()

for i, item in ipairs(items) do 
   for k, v in pairs(item) do 
      print(k,v)
   end
   print()			--newline
end
