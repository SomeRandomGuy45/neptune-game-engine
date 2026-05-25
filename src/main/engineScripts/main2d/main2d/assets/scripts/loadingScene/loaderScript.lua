local M = {}

local backgroundColor = Color.new(140, 52, 235, 255)
local start = 0
local did = false

function M.init()
    start = os.clock()
    print("LOADING SCREEN")
    warn("hi")
    fault("bye")
    local box = Box.new(0, 0, 10000, 10000, backgroundColor)
    print(box ~= nil)
    box:setZIndex(0)
    local text = Text.new(0, 0, 200, 100, "Loading...")
    text:setZIndex(1)
    print(text ~= nil)
end

function M.update()
    if os.clock() - start >= 2 and not did then
        did = true
        print("MOVING TO NEXT SCREEN")
        game:loadNewScene("initScene")
    end
end

return M