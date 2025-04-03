print("go!")
local object = game.Workspace:getDrawObject("hiiiii")  -- Get sprite by name
local event = game.Workspace:getObject("listener")
print(event) --
local myColor = Color.new(255, 0, 255, 0)  -- Create red color
print(object)
function h()
    print("hello! 2")
end
if object then
    object:setColor(myColor)
    object:SetMouseCallBack(function ()
        print("hello!")
    end)
    object:SetMouseCallBack(h)
    print("did call back")
end

-- Add a listener that prints the arguments passed to it
event:AddListener(function(...)
    print("Listener called with arguments:", ...)
end)

-- Fire the event with some arguments
event:Fire("Hello")