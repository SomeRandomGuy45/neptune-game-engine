print("go!")
local object = game.Workspace:getObject("moooo")  -- Get sprite by name
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