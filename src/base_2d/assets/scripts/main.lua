-- main test

--[[

When the game start this function automaticly runs, here you will be able to run setup stuff,
like EventListener, sprite creation, keybinds, and many more..

--]]

local M = {}

local v_pos = Vector2.new(0, 0)

local moveMap = {
    [Enums.Keycodes.w] = function() v_pos:setY(v_pos:getY() - 2.5) end,
    [Enums.Keycodes.s] = function() v_pos:setY(v_pos:getY() + 2.5) end,
    [Enums.Keycodes.a] = function() v_pos:setX(v_pos:getX() - 2.5) end,
    [Enums.Keycodes.d] = function() v_pos:setX(v_pos:getX() + 2.5) end,
}

local object = game.Workspace:getDrawObject("hiiiii")
local greenBox = game.Workspace:getDrawObject("moooo")
local textThing = game.Workspace:getDrawObject("myhappyguy")
local audio = game.Workspace:getObject("audio")

function Follow(cam_X, cam_Y, x, y)
    return Vector2.new(cam_X - x, cam_Y - y)
end

function M.init()
    print("Init!")
    print("Audio nil:", audio == nil)
    print("textThing nil:", textThing == nil)
    print("object nil:", object == nil)
    print("start")
    audio:Play()
    halt(2)
    print("halt 1")
    textThing:setTextColor(Color.new(0, 0, 255, 0))
    audio:Stop()
    print("halt 2")
    halt(2)
    print("text change")
    audio:Play()
    textThing:changeText("Hello world guys!")
    local obj_pos_X = -greenBox:getX()
    local obj_pos_Y = greenBox:getY()
    for key, action in pairs(moveMap) do
        game.InputService:addKeybind(key, function()
            local ok, err = pcall(function()
                action()
                object:setPosition(v_pos)

                local cam_X = game.Workspace:getCameraX()
                local cam_Y = game.Workspace:getCameraY()
                print(cam_X - obj_pos_X, cam_Y - obj_pos_Y)
                greenBox:setPosition(Follow(cam_X, cam_Y, obj_pos_X, obj_pos_Y))
            end)
            if not ok then
                print("Error during keybind:", err)
            end
        end)
    end
end

--[[

This is the code that gets called every frame to be runned
This **should** help prevent the usage of using while true loops

--]]

function M.update()
    --print("update called!")
    game.Workspace:SetCamera(v_pos:getX(), v_pos:getY()) 
end

return M

-- Don't call the init or update function, it will be ignored!
