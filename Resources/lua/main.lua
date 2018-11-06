
-- StrandedLog
sLog = function(...)
    print( string.format(...) )
end

-- for SLuaEngine traceback
function __G__TRACKBACK__(msg)
    sLog("---------------------------")
    sLog("LUA ERROR: " .. tostring(msg) .. "\n")
    sLog(debug.traceback())
    sLog("---------------------------")

    -- 保存到文件
    local file = assert(io.open("error.log", "w"))
    if file ~= nil then
        file:write("---------------------------")
        -- 换行
        file:write(string.char(10))
        file:write("LUA ERROR: " .. tostring(msg) .. "\n")
        file:write(debug.traceback())
        file:write(string.char(10))
        file:write("---------------------------")
        io.close(file)
    end

    return msg
end

local function main()
    collectgarbage("collect")
    -- avoid memory leak
    collectgarbage("setpause", 100)
    collectgarbage("setstepmul", 5000)
    
    math.randomseed( os.time() )
end

local status, msg = xpcall(main, __G__TRACKBACK__)
if not status then
    error(msg)
end