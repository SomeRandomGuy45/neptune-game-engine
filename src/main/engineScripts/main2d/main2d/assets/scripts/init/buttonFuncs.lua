local M = {}

M.functions = {
    ["testButton"] = function ()
        print("function test hello!")
    end,
    ["testButton2"] = function ()
        local event = game.Workspace:getObject("buttonFinishEvent")
        if not event then
            print("Can't find event object!")
        end
        event:Fire("Hello! This is from testButton2!")
    end
}

return M