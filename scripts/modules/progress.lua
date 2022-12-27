-- ===========================================================================
-- ANSI Progress bar
--
local progress = {}
progress.__index = progress
progress.version = 1.2  -- API version
------------------------------------------------------------------------------
progress.size = 50         -- the number of characters the bar is wide
progress.min = 0           -- the minimum value (usually 0)
progress.max = 100         -- the max value for the position
progress.position = 0      -- the present position
--progress.col= 1            -- the terminal display column where the bar is rendered
--progress.row = 25          -- the terminal display row where the bar is rendered
-- character set used to render the bar.  This is a table of
-- empty, 25%, 50%, 75%, 100% for EACH step in the bar.
progress.char = {" ", "{c4}\254", "{c12}\254", "{c14}\254", "{c10}\254" }
progress.loc = "{mv${ROW},${COL}}"
progress.bar = "{hide;c4}[${BAR}{c4}]{c7;cln;show}"
------------------------------------------------------------------------------
function progress:pos( pos )
    if pos then
        -- updating progress
        self.position = pos
        return
    end
    -- reading position
    return self.position
end
------------------------------------------------------------------------------
function progress:next()
    self.position = self.position + 1

    if self.position > self.max then
        self.position = 0
    end
end
------------------------------------------------------------------------------
function progress:render()
    local function strip_ansi(s)
        return s:gsub("(\x1b)([)([0-9]+)([a-zA-Z])","")
    end
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
    local b = ""
    if self.location then
        b = self.loc:gsub("${ROW}",tostring(self.location.row)):gsub("${COL}",tostring(self.location.col))
    else
        b = "\r" -- just go back to the beginning of the line
    end
    local b = b .. self.bar:gsub("${BAR}",bar)
    ansi(b)
end
------------------------------------------------------------------------------
function progress.new(min,max,size, row, col)
    local T = {min=min,max=max,size=size}
    if row and col then
        T.location = { row = row, col = col }
    end
    return setmetatable(T, progress)
end
-- ===========================================================================
return progress
