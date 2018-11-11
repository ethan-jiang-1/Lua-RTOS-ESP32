--import machine, network, utime, uos, _thread, ujson, urandom
--machine = require('machine')
--network = require('network')
--utime = require('utime')
--_thread = require('_thread')
--urandom = require('urandom')

cjson = require("cjson")




print("")
print("mount windows in vfd ...")
fs.mount("/window", "window")
fs.mount("/data", "fat")



--def window1_py():
local function window1_py()

    --for i in range(1000):
    for i = 1, 1000, 1 do 
        print("task a "..tostring(i))
        local localwindow_config = [[
        {
           "action": "create",
           "components" : [
             {
                 "type" : "image",
                 "name" : "im1",
                 "xpos" : 0,
                 "ypos" : 0,
                 "width" : 240,
                 "height" : 240,
                 "src": "/data/water00.zip"
             },
             {
                 "type" : "button",
                 "name" : "bt1",
                 "xpos" : 100,
                 "ypos" : 100,
                 "width" : 50,
                 "height" : 50,
                 "res" : 0
             }
           ]
          }

		]]

		local window_config01 = [[
        {
         "action": "change",
         "components" : [
           {
                "type" : "button",
                "name" : "bt1",
        ]]

        window_config02 = [[
            "width" : 50,
            "height" : 50,
            "res" : 0
           }
         ]
        }
		]]

        fa = lvgl.open("/window/create")
        if fa > 0 then 
			lvgl.write(fa, localwindow_config, string.len(localwindow_config))
			for j = 1, 1000, 1 do
				thread.sleepms(1000)
				msg = lvgl.read(fa, 80)
				if msg ~= nil then
					jmsg = cjson.decode(msg)
					obj = jmsg['value']
					if obj == 'bt1' then
						x = math.random(0, 190)
						y = math.random(0, 190)
						change_config = window_config01 .. '\t\"xpos\" :'..tostring(x)..',\n\t\t"ypos" : '..tostring(y) ..","
						change_config = change_config .. window_config02
						lvgl.write(fa, change_config, string.len(change_config))
					end
				end
			end
			lvgl.close(fa)
        else
          print("failed to open /windows/create")
        end
	end
end 



local function window2_py()
    for k = 1, 1000, 1 do 
        print("task b "..tostring(k))
        window_config2 = [[
        {
         "action": "create",
         "components" : [
           {
               "type" : "animation",
               "name" : "an1",
               "xpos" :  0,
               "ypos" :  0,
               "width" : 240,
               "height" : 240,
               "src" : "/data/water00.zip",
               "length" : 6
           }
         ]
        }
        ]]

        --utime.sleep_ms(10000)
        thread.sleepms(10000)
        fb = lvgl.open("/window/create")
        if fb > 0 then 
          lvgl.write(fb, window_config2, string.len(window_config2))
	  print("task b")
          --utime.sleep_ms(20000)
          thread.sleepms(20000)
          lvgl.close(fb)
        else
          print("failed to open /windows/create")
        end
    end
end

--_thread.start_new_thread("window1_py", window1_py, ())
--_thread.start_new_thread("window2_py", window2_py, ())

thread.start(window1_py)
thread.start(window2_py)
