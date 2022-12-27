-- ===========================================================================
-- Base 64 utilities
--
local base64 = {}
base64.version = 1.0
base64.__index = base64
base64.dialect = {
-- encode/decode dialects for supported standards.
    standard = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/', -- RFC 4648, ss4
    url      = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+_', -- RFC 4648, ss5
    imap     = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+,', -- RFC 3501
    bash     = '0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ@_', -- BASH term encode
    bcrypt   = './ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789',
    uuencode = ' !"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_', -- default UUencode
    hqx      = '!"#$%&\'()*+,-012345689@ABCDEFGHIJKLMNPQRSTUVXYZ[`abcdefhijklmpqr',
    passwd   = './0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz',
    xxencode = '+-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz',
    pgp      = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/', -- standard encoding+ 24-bit CRC
}
base64.def_mode = "standard" -- setup the default mode as standard
------------------------------------------------------------------------------
function base64:mode( mode )
    if mode then
        self.def_mode = mode or "standard"
        -- handle bad dialect selection
        if not self.dialect[mode] then
            self.def_mode = "standard"
        end
    else
        return self.def_mode
    end
end
------------------------------------------------------------------------------
-- @brief encode a string as base 64
function base64:encode(data,mode)
    -- load the dialect of Base64 used
    local btype = mode or self.def_mode
    if btype == true then btype = self.def_mode end
    local dialect = self.dialect[btype]

    return ((data:gsub('.', function(x)
        local r,b='',x:byte()
        for i=8,1,-1 do r=r..(b%2^i-b%2^(i-1)>0 and '1' or '0') end
        return r;
    end)..'0000'):gsub('%d%d%d?%d?%d?%d?', function(x)
        if (#x < 6) then return '' end
        local c=0
        for i=1,6 do c=c+(x:sub(i,i)=='1' and 2^(6-i) or 0) end
        return dialect:sub(c+1,c+1)
    end)..({ '', '==', '=' })[#data%3+1])
end
------------------------------------------------------------------------------
-- @brief decode a base 64 string and return data
function base64:decode(data, mode)
    -- load the dialect of Base64 used
    local btype = mode or self.def_mode
    if btype == true then btype = self.def_mode end
    local dialect = self.dialect[btype]

    data = string.gsub(data, '[^'..dialect..'=]', '')
    return (data:gsub('.', function(x)
        if (x == '=') then return '' end
        local r,f='',(dialect:find(x)-1)
        for i=6,1,-1 do r=r..(f%2^i-f%2^(i-1)>0 and '1' or '0') end
        return r;
    end):gsub('%d%d%d?%d?%d?%d?%d?%d?', function(x)
        if (#x ~= 8) then return '' end
        local c=0
        for i=1,8 do c=c+(x:sub(i,i)=='1' and 2^(8-i) or 0) end
            return string.char(c)
    end))
end
-- ===========================================================================
function base64.new(mode)
    return setmetatable({def_mode = mode}, base64)
end
-- ===========================================================================
return base64
-- END OF FILE ---------------------------------------------------------------
