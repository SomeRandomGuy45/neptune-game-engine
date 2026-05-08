local M = {}

local event = game.Workspace:getObject("buttonFinishEvent")

function M.init()
    if not event then
        print("nooo")
        return
    end
    print("exists")
    event:addListener(function(str)
        print("hiiii working " .. str)
    end)
    print("added listener")
end

function M.update()

end

return M