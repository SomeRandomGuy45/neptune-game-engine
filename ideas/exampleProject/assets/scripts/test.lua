-- main test

--[[

When the game start this function automaticly runs, here you will be able to run setup stuff,
like EventListener, sprite creation, keybinds, and many more..

--]]

local M = {}

local audio = game.Workspace:getObject("audio")

function M.init()
    print("Hi!")
    if (not audio) then
        print("Audio object not found!")
        return
    end
    audio:Play()
end

--[[

This is the code that gets called every frame to be runned
This **should** help prevent the usage of using while true loops

--]]

function M.update()
    --print("update called!")
end

return M

-- Don't call the init or update function, it will be ignored!
