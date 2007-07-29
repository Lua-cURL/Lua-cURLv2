require("cURL")

-- open output file
f = io.open("example_homepage", "w")

-- setup easy with writecallback handler
c = cURL.easy_init({writefunction = function(str)
				       f:write(str)
				    end})
-- setup url
c.setopt.url("http://www.example.com/")

-- perform, invokes callbacks
c.easy_perform()

-- close output file
f:close()


