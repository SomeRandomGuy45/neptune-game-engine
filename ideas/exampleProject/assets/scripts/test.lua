-- main test

--[[

When the game start this function automaticly runs, here you will be able to run setup stuff,
like EventListener, sprite creation, keybinds, and many more..

--]]

local M = {}

local audio = game.Workspace:getObject("audio")
local box = game.Workspace:getDrawObject("superCoolSprite")
local currentPos = Vector2.new(0, 0)
function M.init()
    print("Hi!")
    if (not audio) then
        print("Audio object not found!")
        return
    end
    audio:Play()
    if (not box) then
        print("Box object not found!")
        return
    end
    print("We have our box!")
    print(Enums.Keycodes.w)
end

--[[

This is the code that gets called every frame to be runned
This **should** help prevent the usage of using while true loops

--]]

function M.update()
    --print("update called!")
    if (not box) then return end
    local currentKey = game.InputService:getKeyDown()
    if (currentKey ~= -1) then 
        if (currentKey == Enums.Keycodes.w) then
            currentPos:setY(currentPos:getY() - 2.5)
            box:setPosition(currentPos)
        end
    end
end

return M

-- Don't call the init or update function, it will be ignored!
