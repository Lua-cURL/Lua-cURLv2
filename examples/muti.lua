require("cURL")

-- setup easy 
c = cURL.easy_init()
c2 = cURL.easy_init()
-- setup url
c:setopt_url("http://www.example.com/")
c2:setopt_url("http://www.hoetzel.info/")
-- c.perform()
-- perform, invokes callbacks
m = cURL.multi_init()
m:add_handle(c)
m:add_handle(c2)
-- it = m:perform()

for data,type,easy in m:perform() do 
   if (type == "data") then print(data) end
end
