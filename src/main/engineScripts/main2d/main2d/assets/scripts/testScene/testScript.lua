local M = {}

local testModule = require("testScene.testModule")
local backgroundColor = Color.new(140, 52, 235, 255)

function M.init()
    print("NEW SCREEN")
    local box = Box.new(0, 0, 10000, 10000, backgroundColor)
    print(box ~= nil)
    print(package.path)
    print("okay 1")
    print(testModule.functionTest())
    print("okay 2")
    box:setZIndex(0)
    local text = Text.new(0, 0, 200, 100, "HEY IM STILL HERE!")
    text:setZIndex(1)
    text:setBackgroundColor(backgroundColor)
    print(text ~= nil)
end

function M.update()

end

return M