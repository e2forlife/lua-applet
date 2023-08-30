-- ===========================================================================
-- ANSI Progress bar
--
local progress = {}
progress.__index = progress
progress.version = 2.0  -- API version
------------------------------------------------------------------------------
progress.size = 50         -- the number of characters the bar is wide
progress.min = 0           -- the minimum value (usually 0)
progress.max = 100         -- the max value for the position
progress.step = 1          -- the step size of the bar
progress.position = 0      -- the present position
-- character set used to render the bar.  This is a table of
-- empty, 25%, 50%, 75%, 100% for EACH step in the bar.
progress.char = {"{c7}=", "{c4}\\", "{c12}|", "{c14}/", "{c10}#" }
progress.bar = "{hide;c4}[${BAR}{c4}]{c7;show}"
------------------------------------------------------------------------------
function progress:pos( pos )
    if pos then
        -- updating progress
        self.position = pos
        return
    end
    -- reading position
    return self.position, self:render()
end
------------------------------------------------------------------------------
-- update to the next position of the bar, and render to a string using the
-- new position.
function progress:next()
    self.position = self.position + (self.step or 1)

    if self.position > self.max then
        self.position = self.max
    end
    return self:render()
end
------------------------------------------------------------------------------
-- render the bar to a string using the current position
function progress:render()
    local bar = ""
    local step_size = (self.max-self.min)/self.size -- each bar step = this much position
    local frac = self.position/step_size -- compute the fractional step
    local fill = math.floor(frac) -- the whole number of steps
    frac = frac - fill -- remove the whole number of steps

    for step = 1,self.size do
        local chr = 1
        if fill >= step then chr = 5 -- full step
        elseif ((fill+1) == step) and (frac > 0) then
            -- fractional conversion
            chr = math.floor( (frac*4)+1 )
        end
        bar = bar..self.char[chr]
    end
    return self.bar:gsub("${BAR}",bar)
end
------------------------------------------------------------------------------
-- generate a new progress bar that is tailored to the conditions of the
-- of the requirements for the progressbar.
--     min : the minimum value for the progressbar, default is 0
--     max : the maximum value of the progressbar, default 100
--     size : the number of characters used for the the rendering of the bar
function progress.new(size, max, min)
    local T = {
        min=min or 0,
        max=max or 100,
        size=size or 50
    }
    return setmetatable(T, progress)
end
-- ===========================================================================
return progress
