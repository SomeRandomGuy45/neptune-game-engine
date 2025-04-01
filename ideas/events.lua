--[[
    Events...
--]]

local event = game.Workspace:getObject("MyTestEvent") -- gets the event

if event then
    event:Fire("data_1", 34324)
end

-- in a different script

local event = game.Workspace:getObject("MyTestEvent")
event:AddListener(function(arg1, arg2)
    print("WOW", arg1, arg2)
end)