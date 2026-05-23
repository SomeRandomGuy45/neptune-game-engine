local M = {}

-- MODULES
local buttonLib = require("modules.buttons.button")
local xml2lua = require("modules.xml.xml2lua")
local handler = require("modules.xml.xmlhandler.tree")

-- VARIABLES
local buttons = {}

function M.init()
   local xml = xml2lua.loadFile(game:getExecDir() .. "/assets/scripts/init/buttonData.xml")
   local parser = xml2lua.parser(handler)
   parser:parse(xml)
   for i, v in pairs(handler.root.buttons) do
      print(i, "Text:", v.text)
   end
   --[[
   buttons["test1"] = buttonLib:newButton("Test! How are you!", 0, 0, 250, 100, Color.new(255, 0, 0, 255))
   -- Okay ig
   buttons["test1"]:setMouseCallback(function() 
      print("hi")
      local event = game.Workspace:getObject("buttonFinishEvent")
      print(event == nil)
      event:Fire("hey")
      print("done")
   end)
   print("button logic... DONE!")
   --]]

end

function M.update()
   
end

return M