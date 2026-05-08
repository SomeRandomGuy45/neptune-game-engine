local M = {}

local backgroundColor = Color.new(140, 52, 235, 255)

function M.init()
    print("LOADING SCREEN")
    local box = Box.new(0, 0, 10000, 10000, backgroundColor)
    print(box ~= nil)
    box:setZIndex(0)
    local text = Text.new(0, 0, 200, 100, "Loading...")
    text:setZIndex(1)
    text:setBackgroundColor(backgroundColor)
    print(text ~= nil)
    halt(2)
    game:loadNewScene("initScene")
end

function M.update()

end

return M