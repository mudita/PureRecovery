-- @module recovery

local recovery = {}
local sys = require("lsys")
local keyboard = require("lkeyboard")
local gui = require("lgui")

function recovery.version()
  return "0.0.0"
end

function recovery.branch()
  return "master"
end

function recovery.revision()
  return "12344321"
end

recovery = {sys,keyboard,gui}
return recovery