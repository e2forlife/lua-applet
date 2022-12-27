-- General Utilities for processing files, and generating other 
-- ===========================================================================
local utils = {}

------------------------------------------------------------------------------
-- This function will convert a block of RAW binary data into a intel hex
-- output format that can be used with JTAG programmers.  The arguments
-- rawdata is a string that contains the data to be converted, and the
-- base_address is the starting address for the data to be located.
function utils.bin2hex( rawdata, base_address )
    local fdata = ""
    local address = base_address or 0x8000000
    local segment = 0
    local indx = 1

    local function makeline( raw )
        local str = ":"
        local cs = 0
        for _,v in ipairs(raw) do
            str = str .. string.format("%02X",v)
            cs = cs + v 
        end
        cs = ((~cs) + 1)&0xFF
        return str .. string.format("%02X",cs).."\n"
    end

    -- loop through the raw data until it is all processed
    while indx < #rawdata do
        -- check for segment write
        if address > 0xFFFF then
            -- calculate new segment address from the offset
            segment = segment + ((address>>16)&0xFFFF)
            -- generate the segment write line
            fdata = fdata .. makeline({2,0,0,4,(segment>>8)&0xFF, segment&0xFF})
            -- adjust the address to be within the segment window
            address = address&0xFFFF
        end
        -- grab the substring of the binary image data from the present index to
        -- the line length of the hexfile
        local linesize = (16 < #rawdata-indx+1) and 16 or (#rawdata-indx+1)
        local data = {
            linesize&0xFF,                                    -- line size
            (address>>8)&0xFF, address&0xFF,       -- offset
            0,                                     -- record type
            rawdata:sub(indx,indx+(linesize-1)):byte(1,linesize)   -- data
        }
        -- get the next index of the file and set the next offset
        indx = indx + linesize
        address = address + linesize
        fdata = fdata .. makeline(data)
    end
    -- add termination record and return the converted file data
    return fdata .. ":00000001FF\n"

end
------------------------------------------------------------------------------
-- The parse_arg function will substiture placeholder named arguments in a
-- string with values from a list.  The index of the value set is the name
-- of the argument used, and the value of the value set is the value that
-- is used.  This allow for strings to contain a shorthand arguments to build
-- formulas with valeus pulled from the value list.  string variables that
-- are filled in are of the format %<name>%.
function utils.parse_arg( txt, value_set )

    for name,value in pairs(value_set) do
        txt = txt:gsub("%%"..name.."%%", value)
    end
    return txt
end
------------------------------------------------------------------------------
-- convert an AXF file to a binary file for processing.
function utils.load_axf( filename )
    local command = "c:\\keil_v5\\arm\\armcc\\bin\\fromelf.exe --bin "..filename.." --output temp.bin"
    os.execute(command)
    local fil = io.open("temp.bin","rb")
    if fil == nil then return nil end
    local data = fil:read("a")
    fil:close()
    os.execute("DEL /F /Q temp.bin")
    return data
end
------------------------------------------------------------------------------
-- function to return the filename and file extension as seperate strings
function utils.split_file(path)
    local filepath = {}
    path:gsub("[^/\\]+", function(x) filepath[#filepath+1]=x end)
    local fname = {}
    filepath[#filepath]:gsub("[^%.]+", function(x) fname[#fname+1]=x end)
    table.remove(filepath,#filepath)
    return table.concat(filepath,"\\")..'\\'..fname[1],fname[2]
end
------------------------------------------------------------------------------
-- this function returns a checksum on a block of data
function utils.checksum(block)
    local sum = 0
    for _,v in ipairs({string:byte(1,#block)}) do
        sum = sum + v
    end
    return sum
end

-- ===========================================================================
return utils