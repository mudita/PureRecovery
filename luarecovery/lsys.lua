-- @module recovery.sys

local sys = {}

-- Load dummy boot reason from a file. Its content can be easily filled during unit testing using automatic tools.
local fd = assert(io.open("sysboot.in"))
local reason = fd:read("*a")
fd:close()

local reason_as_string = { [0] = "update",
                           [1] = "recovery",
                           [2] = "factory",
                           [3] = "pgm_keys"
}
local reason_unknown = "unknown"

function sys.sleep(seconds)
    os.execute("sleep " .. tonumber(seconds) / 1000)
end

function sys.uptime()
    return tonumber(os.execute("date +%s"))
end

function sys.boot_reason()
    return tonumber(reason)
end

function sys.boot_reason_str()
    local r = tonumber(reason)

    if (reason_as_string[r]) then
        return reason_as_string[r]
    else
        return reason_unknown
    end
end

function sys.set_boot_reason(code)
end

function sys.flash_bootloader()
    return 0
end

function sys.swap_slots()
    return 0
end

function sys.select_slot(slot)
    return 0
end

return sys