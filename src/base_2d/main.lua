local object = game.Workspace:getObject("hiiiii")  -- Get sprite by name
local myColor = Color.new(255, 0, 0, 0)  -- Create red color
if object then
    object:setName("hj")  -- Rename it
    local newObject = game.Workspace:getObject("hj")
    if newObject then
        print(object)
        newObject:setColor(myColor) 
    end
end