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
   -- why isn't there a continue built in
   -- rahhhhh
   for i, v in pairs(handler.root.buttons) do
      if i ~= "button" then
         goto continue
      end
      --print(i, "Text:", v.text)
      local btnText = v.text
      local colorTag = v.color
      local btnColor = Color.new(tonumber(colorTag._attr.r),tonumber(colorTag._attr.g),tonumber(colorTag._attr.b),tonumber(colorTag._attr.a))
      local btnId = v._attr.buttonId
      local btnX = tonumber(v._attr.x)
      local btnY = tonumber(v._attr.y)
      local btnW = tonumber(v._attr.w)
      local btnH = tonumber(v._attr.h)
      print(btnText, btnX, btnY, btnW, btnH)
      buttons[btnId] = buttonLib:newButton(btnText, btnX, btnY, btnW, btnH, btnColor)
      ::continue::
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