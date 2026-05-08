local M = {}

-- MODULES
local buttonLib = require("modules.buttons.button")

-- VARIABLES
local buttons = {}

function M.init()
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
end

function M.update()

end

return M