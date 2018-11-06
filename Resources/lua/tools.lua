function echo(...)
    local arr = {}
    for i, a in ipairs({...}) do
        arr[#arr + 1] = tostring(a)
    end

    print( table.concat(arr, "\t") )
end

--[Comment]
-- Dumps information about a variable.
-- @param mixed object
-- @param string label
-- @param bool isReturnContents
-- @param int nesting
-- @return nil | string
function dump(object, label, isReturnContents, nesting)
    if type(nesting) ~= "number" then nesting = 99 end

    local lookupTable = {}
    local result = {}
    local spc = "";

    local function _v()
        if type(v) == "string" then
            v = "\"" .. v .. "\""
        end
        return tostring(v);
    end

    local traceback = string.split(debug.traceback("", 2), "\n")
    echo("dump from: " .. string.trim(traceback[3]))

    local function _dump(object, label, indent, nest, keylen)
        label = label or "<var>"
        spc = ""
        if type(keylen) == "number" then
            spc = string.rep(" ", keylen - string.len(_v(label)))
        end
        if type(object) ~= "table" then
            result[#result + 1] = string.format("%s%s%s = %s", indent, _v(label), spc, _v(object))
        elseif lookupTable[object] then
            result[#result + 1] = string.format("%s%s%s = *REF*", indent, label, spc)
        else
            lookupTable[object] = true
            if nest > nesting then
                result[#result + 1] = string.format("%s%s = *MAX NESTING*", indent, lable)
            else
                result[#result + 1] = string.format("%s%s = {", indent, _v(label))
                local indent2 = indent .. "    "
                local keys = {}
                local keylen = 0
                local values = {}
                for k, v in pairs(object) do
                    keys[#keys + 1] = k
                    local vk = _v(k)
                    local vkl = string.len(vk)
                    if vkl > keylen then keylen = vkl end
                    values[k] = v
                end
                table.sort(keys, function(a, b)
                    if type(a) == "number" and type(b) == "number" then
                        return a < b
                    else
                        return tostring(a) < tostring(b)
                    end
                end)
                for i, k in ipairs(keys) do
                    _dump(values[k], k, indent2, nest + 1, keylen)
                end
                result[#result + 1] = string.format("%s}", indent)
            end
        end
    end
    _dump(object, label, "- ", 1)

    if isReturnContents then
        return table.concat(result, "\n")
    end

    for i, line in ipairs(result) do 
        echo(line)
    end
end

local code, ret = pcall(loadstring( string.format("do local _=%s return _ end", str) ))
if code then
    return ret
else
    -- 异常
    error(msg)
end