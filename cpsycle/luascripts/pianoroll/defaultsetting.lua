local node = require("psycle.node") 
local orderedtable = require("psycle.orderedtable")
local stock = require("psycle.stock")
local property = require("psycle.property")

local setting = node:new():setname("setting")

setting.meta = {
  name = "pianoroll",
  version = 0.1
}

local general = node:new(setting):settext("General"):setname("general")
general.properties = orderedtable.new()
general.properties.exampleproperty = property:new(100, "Example Property")

return setting
