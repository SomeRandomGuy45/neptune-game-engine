print("go!")
local object = game.Workspace:getDrawObject("hiiiii")  -- Get sprite by name
local textThing = game.Workspace:getDrawObject("myhappyguy")
local event = game.Workspace:getObject("listener")
local audio = game.Workspace:getObject("audio")
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
    object:SetMouseCallBack(function()
        print("hello!")
        print("color change!")
        local c = Color.new(0, 0, 255, 0)
        object:setColor(c)
    end)
    --object:SetMouseCallBack(h)
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
print("hi!!")
event:Fire("Hello")