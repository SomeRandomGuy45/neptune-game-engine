-- Very messy but its testing grounds

print("go!", Enums.Keycodes.a)
local object = game.Workspace:getDrawObject("hiiiii")  -- Get sprite by name
local box = game.Workspace:getDrawObject("moooo")
local textThing = game.Workspace:getDrawObject("myhappyguy")
local event = game.Workspace:getObject("listener")
local audio = game.Workspace:getObject("audio")
local v_pos = Vector2.new(0, 0)
print(v_pos)
print(textThing)
print(audio)
audio:Play()
halt(2)
textThing:setTextColor(Color.new(0, 0, 255, 0))
audio:Stop()
halt(2)
audio:Play()
textThing:changeText("Hello world guys!")
print(event) 
local myColor = Color.new(255, 255, 255, 0)  -- Create red color
print(object)
if object then
    object:setColor(myColor)
    object:setMouseCallBack(function()
        print("hello!")
        print("color change!")
        object:setColor(Color.new(0, 0, 255, 0))
    end)
    --object:setMouseCallBack(h)
    print("did call back")
end

-- Add a listener that prints the arguments passed to it
event:AddListener(function(...)
    print("Listener called with arguments:", ...)
    halt(2)
    print("color change!")
    object:setColor(Color.new(255, 0, 0, 0))
end)

-- Fire the event with some arguments
halt(2)
game.Workspace:MoveCamera(1, 1)
print("hi!!")
event:Fire("Hello")
--[[

local x, y = 0, 0
while (200 >= x and 200 >= y) do
    halt(0.1)
    game.Workspace:SetCamera(x, y)
    x = x + 0.1
    y = y + 0.1
    print(x, y)
end

]]

local moveMap = {
    [Enums.Keycodes.w] = function() v_pos:setY(v_pos:getY() - 2.5) end,
    [Enums.Keycodes.s] = function() v_pos:setY(v_pos:getY() + 2.5) end,
    [Enums.Keycodes.a] = function() v_pos:setX(v_pos:getX() - 2.5) end,
    [Enums.Keycodes.d] = function() v_pos:setX(v_pos:getX() + 2.5) end,
}

for key, action in pairs(moveMap) do
    game.InputService:addKeybind(key, function()
        action()
        object:setPosition(v_pos)
    end)
end

while true do
    halt(0.1)
    game.Workspace:SetCamera(v_pos:getX(), v_pos:getY()) 
end