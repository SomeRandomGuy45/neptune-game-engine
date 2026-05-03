-- main test

--[[

When the game start this function automaticly runs, here you will be able to run setup stuff,
like EventListener, sprite creation, keybinds, and many more..

--]]

local M = {}

local audio = game.Workspace:getObject("audio")
local color = Color.new(255, 255, 0, 255)
local box = nil
local currentPos = Vector2.new(0, 0)
function M.init()
    print("Hi!")
    print("added new")
    print("doing test!")
    Box.new(0, 0, 100, 100, color)
    box = game.Workspace:getDrawObject("boxTest")
    print("boom")
    local result = game.PlatformService:popUpWindow("Hello!", "This is a pop-up window test!")
    print("Pop-up result: " .. tostring(result))
    local fileChoosen = game.PlatformService:getFileFromPicker()
    print("File chosen: " .. fileChoosen)
    print(game.PlatformService:getExecutableDir())
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
    game.InputService:bindKeybind(Enums.Keycodes.w, function()
        currentPos:setY(currentPos:getY() - 2.5)
        box:setPosition(currentPos)
    end)
    game.InputService:bindKeybind(Enums.Keycodes.s, function()
        currentPos:setY(currentPos:getY() + 2.5)
        box:setPosition(currentPos)
    end)
    game.InputService:bindKeybind(Enums.Keycodes.a, function()
        currentPos:setX(currentPos:getX() - 2.5)
        box:setPosition(currentPos)
    end)
    game.InputService:bindKeybind(Enums.Keycodes.d, function()
        currentPos:setX(currentPos:getX() + 2.5)
        box:setPosition(currentPos)
    end)
    print("Init function is done!")
end

--[[

This is the code that gets called every frame to be runned
This **should** help prevent the usage of using while true loops

--]]

function M.update()
    --print("update called!")
    if (not box) then return end
    --[[
    local currentKey = game.InputService:getKeyDown()
    if (currentKey ~= -1) then 
        if (currentKey == Enums.Keycodes.w) then
            currentPos:setY(currentPos:getY() - 2.5)
            box:setPosition(currentPos)
        elseif (currentKey == Enums.Keycodes.s) then
            currentPos:setY(currentPos:getY() + 2.5)
            box:setPosition(currentPos)
        elseif (currentKey == Enums.Keycodes.a) then
            currentPos:setX(currentPos:getX() - 2.5)
            box:setPosition(currentPos)
        elseif (currentKey == Enums.Keycodes.d) then
            currentPos:setX(currentPos:getX() + 2.5)
            box:setPosition(currentPos)
        end
    end
    ]]
    game.Workspace:setCamera(currentPos:getX(), currentPos:getY()) 
end

return M

-- Don't call the init or update function, it will be ignored!
