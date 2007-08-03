require("cURL")

c = cURL.easy_init()
-- setup url
c:setopt_url("http://localhost")
postdata = {  
   -- post file from filesystem
   name = {file="post.lua",
	   type="text/plain"},
   -- post file from data variable
   name2 = {file="dummy.html",
	    data="<html><bold>bold</bold></html>",
	    type="text/html"}}
c:post(postdata)

    
-- perform, invokes callbacks
c:perform()
-- close output file

print("Done")


