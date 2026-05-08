local buttonBase = {}
buttonBase.__index = buttonBase

function buttonBase:newButton(_text ,_x, _y, _w, _h, _color)
    local self = setmetatable({
        x = _x,
        y = _y,
        w = _w,
        h = _h,
    }, buttonBase)
    self.button = Box.new(_x, _y, _w, _h, _color)
    local sizeText = game:getTextSize("FreeSans", _text)
    local btnText = Text.new(_x, _y, sizeText["w"], sizeText["h"], _text)
    btnText:setBackgroundColor(_color)
    self.buttonText = btnText
    return self
end

function buttonBase:setMouseCallback(func)
    print("called")
    self.button:setMouseCallBack(func)
end

return buttonBase