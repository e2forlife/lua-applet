if ansi == nil then
    print("\x1b[34m[\x1b[91mERROR\x1b[34m]\x1b[37m: Remapping \x1b[96mansi()\x1b[37m to io.write().\n\x1b[92mPlease use eXtended Lua (\x1b[93mxLua\x1b[92m)\x1b[37m\n")  
    function ansi(s)  io.write(s) end
end
-- ---------------------------------------------------------------------------
local sep  = package.config:sub(1,1) -- extract the separator
package.path = ("..{SEP}modules{SEP}?.lua;.{SEP}modules{SEP}?.lua;"):gsub("{SEP}",sep) .. package.path
------------------------------------------------------------------------------
local app = require "app"
local progress = require "progress"
local base64 = require "base64"
------------------------------------------------------------------------------
local encbin = app.new(false)  -- create the application framework object
------------------------------------------------------------------------------
encbin.name = "encbin"
encbin.brief = "convert binary file to encoded ASCII for packing within Lua app"
encbin.version = "1.0.0"  -- requires xLua w/ "ansi()" function
encbin.detail = [[
    {c4}[{c7}Encode Binary file as source{c4}]

    --encode : Encode binary files into output encoded text
    --decode : Decode binary data from encoded text.

    --ofile=<> : Set the name of the output file for encoded results (default is stdout)
    --max_line=## : Set the max number bytes per encoded line.
]]
------------------------------------------------------------------------------
-- Default options 
function encbin:set_defaults( )
    -- the maximum number of bytes per encoded line
    if not self.opts.max_line then
        self.opts.max_line = 189
    elseif type(self.opts.max_line) == "string" then
        self.opts.max_line = tonumber(self.opts.max_line)
    end
end
------------------------------------------------------------------------------
function encbin:encode( filename, ofile )
    local b64 = base64.new("bcrypt")

    self:message("info","Encoding {c10}"..filename.."{c7}.")
    local fil = io.open(filename,"rb")
    if fil ~= nil then
        -- bulk read file data into the buffer
        local data = fil:read("a")
        fil:close()
        local pb = progress.new(0,#data)
        self:message("info","Estimating {c11}"..tostring(#data//self.opts.max_line + (((#data%self.opts.max_line) > 0) and 1 or 0)) .. "{c7} lines.")
        ofile:write("    {\n        filename = \""..filename.."\",\n")
        ofile:write("        data = {\n")
        local indx = 1
        repeat
            -- encode each "line" of data in the file
            -- as encoded base-64 ASCII
            local line_length = (#data >= self.opts.max_line) and self.opts.max_line or #data
            local value = b64:encode(data:sub(1, line_length))
            data = data:sub(line_length+1,#data)
            ofile:write("            \""..value.."\",\n")
            pb:pos( pb:pos() + line_length )
            pb:render()
            ansi("{C13}"..tostring(indx).."{c7}")
            indx = indx + 1
        until #data == 0
        ofile:write("        }\n    },\n")
        ansi("\n")
        self:message("info","Successfully encoded {c10}"..tostring(indx).."{c7} lines.")
    else
        self:message("error","Could not open input file.")
    end -- end File opened successfully
end
------------------------------------------------------------------------------
function encbin:decode( pack_data )
    local b64 = base64.new("bcrypt")

    self:message("info","Decoding {c10}"..pack_data.filename.."{c7}")
    local pb = progress.new(0,#pack_data.data)
    local fil = io.open(pack_data.filename, "w+b")
    if fil ~= nil then
        for i,line in pairs(pack_data.data) do
            pb:next()
            local data = b64:decode(line)
            fil:write(data)
            pb:render()
            ansi("{c13}"..tostring(i).."{c7}")
        end
        fil:close()
        ansi("\n")
    end
end
-- Framework callbacks =======================================================
------------------------------------------------------------------------------
function encbin:init()
    -- tool initialization
    self:set_defaults()
    self:message("info", "Binary Encode/Decode v"..self.version)
    self:message("info", "Application Framework v"..app.version)
    self:message("info", "Progress bar v"..progress.version)
    self:message("info", "Base64 Encoding v"..base64.version)
end
------------------------------------------------------------------------------
function encbin:main( ifile, ofile )
    if self.opts.decode then
        for _,filename in pairs(ifile) do
            local f_data = dofile(filename) -- load the packfile
            for indx,pack in pairs( f_data ) do
                local process = true
                if self.opts.filename ~= nil and self.opts.filename ~= pack.filename then
                    process = false
                end
                if process then
                    -- decode the file
                    self:decode(pack)
                end
            end
        end
    elseif self.opts.encode then
        ofile:write("return {\n")
        for _,filename in pairs(ifile) do
            self:encode(filename,ofile)
        end
        ofile:write("}\n")
    else
        self:help()
    end
end
------------------------------------------------------------------------------
-- Execute Framework =========================================================
encbin:go( arg )
-- return the application table (module)
return encbin
