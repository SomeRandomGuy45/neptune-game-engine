function wait(seconds)
    local start = os.time()
    while os.time() - start < seconds do end
end


wait(10)
print("go!")
local object = game.Workspace:getObject("moooo")  -- Get sprite by name
local myColor = Color.new(255, 0, 255, 0)  -- Create red color
print(object)
if object then
    object:setName("23")  -- Rename it
    object:setColor(myColor)
end