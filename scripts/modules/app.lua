-- Lua Shell tool Template
-- V0.4.0 :  Moved applciation initialization to after the arguments are
--           processed by the startup code.
---------------------------------------------------------------------------
-- utitility to split a line on a delimiter
-- and return a table of results.  when there
-- are no delimiters found, the function will
-- just return a table with 1 entry.
if string.split == nil then
    function string:split(pat)
        local tbl = {}
        if self ~= nil then 
            self:gsub(pat, function(x) tbl[#tbl+1]=x end)
        end
        -- a very complicated way to say return a table for more
        -- than 1 thing, return a string for one thing, or
        -- when we are at the end of the file, return nil
        return ((#tbl > 0) and tbl) or nil
    end
end
------------------------------------------------------------------------------
local app = {}
app.__index = app
app.name = "default"
app.brief = "This is a quick applciation to do something."
app.version = "0.5.0"
app.detail = "\nThere is no detail yet..."
app.license = "CC-BY-NC-SA 4.0 (https://creativecommons.org/licenses/by-nc-sa/4.0/)"
app.copyright = "(c)2021 -"
------------------------------------------------------------------------------
function app.framework_version()
    return app.version
end
------------------------------------------------------------------------------
function app.framework_license()
    return app.copyright .. " " .. app.license
end
------------------------------------------------------------------------------
function app:help()
    local str = "{c15}${NAME}{c7}: ${DESC}\n"
    local o = str:gsub("${NAME}",self.name):gsub("${DESC}",self.brief)
    ansi(o)
    ansi(self.detail)
end
------------------------------------------------------------------------------
function app:show_version()
    local str = "{c15}${NAME}{c7}: v${VER}\n"
    local o = str:gsub("${NAME}",self.name):gsub("${VER}",tostring(self.version))
    ansi(o)
    
    ansi("\n\nGenerated with {c3}lua-app framework{c7} v"..app.version.."\n")
    ansi("The lua-app framework is {c15}"..app.copyright .." {c5}".. app.license.."{c7}\n")
end
------------------------------------------------------------------------------
function app:get_args( cmd_line )
    local options = {}
    local other = {}
    for _,v in ipairs(cmd_line) do
        if (v:sub(1,1) == '-') and (v:sub(2,2) == '-') then
            -- option starts with a --
            -- get index of =
            local asn = v:find("=")
            local opt_value
            if asn ~= nil and asn < #v then
                opt_value = v:sub(asn+1)
                local xv = opt_value:split("[^,]+")
                xv = (#xv == 1) and xv[1] or xv
                local idx = v:sub(3,asn-1)
                if options[idx] then
                    -- when the option exists on the command line,
                    -- check to see if it is a multiple option
                    -- (for example when multple args are already
                    -- present).
                    if type(options[idx]) == "table" then
                        -- add the new option to the option list
                        options[idx][#options[idx]+1] = xv
                    else
                        -- this is the second option, create a list
                        -- and add the new option to the list
                        options[idx] = { options[idx], xv}
                    end
                else
                    -- the option did not exist already, so, create a new option
                    options[idx] = xv
                end
            else
                -- option was a binary true/false option, so just set the option
                -- to truse since it was present.
                options[v:sub(3)] = true
            end
        else
            other[#other+1] = v
        end
    end

    self.opts = options
    self.files = other

    return options, other
end
------------------------------------------------------------------------------
function app:go( cmd_line, no_ansi )

    self.plain = no_ansi or self.plain
    if self.plain then ansi_enable(false) end

    -- process command line arguments
    self.opts = {}   -- no options
    self.files = {}  -- no files
    self:get_args(cmd_line or arg)

    -- initialize the applciation default values, and other setupt
    -- for the tool.

    if self.init then self.ofile = self:init() end
    -- when operating in plain mode, disable ANSI interpretation
    if self.opts.plain then ansi_enable(false) end

    if self.opts.help then
        app.help(self)
        return
    elseif self.opts.version then
        app.show_version(self)
        return
    end
    -- divert output stdout
    if not self.ofile then self.ofile = io.stdout end
    if self.opts.ofile then
        -- output file was specified, so open file
        self.ofile = io.open(self.opts.ofile,"w+"..((self.bin_mode and "b") or ""))
    end
    -- process application tool functions.
    if self.main then
        self:main(self.files, self.ofile)
    else
        ansi("{c1}This application does not do anything yet.\nWe suggest that you implement {c15}app:loop(){c1} to assign operations.{c7}\n\n")
    end

    -- close the file
    if self.opts.ofile then self.ofile:close() end
end
------------------------------------------------------------------------------
function app:message(state, message, ...)
    local st = state or "info"
    local msg = "{c4}[{c${COLOR}}${STATE}{c4}]{c7}: "
    if st:lower() == "warn" then
        msg = msg:gsub("${COLOR}","11")
    elseif st:lower() == "error" then
        msg = msg:gsub("${COLOR}","9")
    else
        msg = msg:gsub("${COLOR}","10")
    end
    msg = msg:gsub("${STATE}",st)
    msg = msg .. message:format( ... )
    if not self.opts.quiet then
        -- quiet output when not in verbose mode
        ansi(msg.."\n")
    end
    return msg  -- return the message to the caller
end
------------------------------------------------------------------------------
-- this helper function takes a binary string and creates a dump of the binary
-- data, and returns the dump string.
function app:show_bin(pkt, line_length, bin_only)
    local len = line_length or 16
	local str = ""
    local bin_data = ""
    local pos = 0
	for _,val in ipairs({pkt:byte(1,#pkt)}) do
        pos = pos + 1
        if pos > len then
            if not bin_only then str = str .. "  "..bin_data end
            bin_data = ""
            str = str .. "\n"
            pos = 1
        end
        bin_data = bin_data .. ( (((val >=32) and (val < 127)) and string.char(val)) or ".")
		str = str .. string.format("[%02X]",val)
	end
    if pos < len then
        -- there was not a full line (pad with spaces)
        str = str..string.rep("    ", (len-pos))
    end
	return str  .. "  "..bin_data .. "\n"
end
------------------------------------------------------------------------------
-- add some color to a string based on some simple built-in rules:
--   Brackets ([]) are color blue, value inside Brackets is fg[1], bg[1]
--   after closing bracket color is set to fg[2],bg[2]
function app:prettify(s, fg,bg)
	local function get_color(c,bg)
		local color = c or 7
		local offset = bg and {40,100} or {30,90}
		return ((color<=7) and (color+offset[1])) or ((color-8)+offset[2])
	end
	local fgc = fg or {15,7}
	local bgc = bg or {0, 0}
    -- make sure that the colors are tables, even if the app passes an integer
    if type(fgc) ~= "table" then fgc = { fgc, 7} end
    if type(bgc) ~= "table" then bgc = { bgc, 0} end

	if s then
		local fmt = {
			string.format("{c4}[{c%d;b%d}",get_color(fgc[1]),get_color(bgc[1],true)),
			string.format("{c4}]{c%d;b%d}",get_color(fgc[2]),get_color(bgc[2],true)),
		}
		s = s:gsub("%[",fmt[1])
		s = s:gsub("%]",fmt[2])
	end
	return s
end
------------------------------------------------------------------------------
function app.new( bin_output , plain)
    return setmetatable({bin_mode = bin_output, plain = plain}, app)
end
------------------------------------------------------------------------------
return app
