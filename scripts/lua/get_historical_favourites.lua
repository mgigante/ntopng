--
-- (C) 2013-15 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path
require "lua_utils"
require "top_talkers"
require "db_utils"
local json = require ("dkjson")

sendHTTPHeader('text/html; charset=iso-8859-1')

ifid = getInterfaceId(ifname)

-- use this two params to see statistics of a single host
-- or for a pair of them
local host = _GET["host"]
local peer = _GET["peer"]
if peer and not host then
   host = peer
   peer = nil
end

-- this is to retrieve L7 appliation data
local l7_proto_id = _GET["l7_proto_id"]

-- specify the type of stats
local action = _GET["action"]
if action == nil or (action ~= "set" and stats_type ~= "get") then
   -- default to get
   stats_type = "get"
end

local stats_type = _GET["stats_type"]
if stats_type == nil or (stats_type ~= "top_talkers" and stats_type ~= "top_applications") then
   -- default to top traffic
   stats_type = "top_talkers"
end

local favourite_type = _GET["favourite_type"]
if favourite_type == nil or (favourite_type ~= "talker" and favourite_type ~= "apps_per_host_pair") then
   -- default to talkers
   -- infer the favourite type by looking at peers
   favourite_type = "talker"
end

-- start building the response
local res = {["status"] = "unable to parse the request, please check input parameters."}

-- prepare the redis key
local k = getRedisPrefix("ntopng.prefs")..'.historical_favourites.'..stats_type..'.'..favourite_type
if action == "get" then
   -- retrieve all the elements set for this kind of preference
   res = ntop.getHashKeysCache(k)

   if res == nil then res = {} end
   -- now it's time to retrieve has values that contain resolved addresses
   -- and are a more-user friendly way to represent hosts
   for h, _ in pairs(res) do
      res[h] = ntop.getHashCache(k, h)
      if res[h] == "" or res[h] == nil then res[h] = h end
   end
elseif action == "set" then
   local entry = ""
   local resolved = ""
   if host ~= "" and host ~= nil then
      entry = host
      resolved = ntop.getResolvedAddress(host)
   end
   if peer ~= "" and peer ~= nil then
      entry = entry..','..peer
      resolved = resolved..','..ntop.getResolvedAddress(peer)
   end
   if entry ~= "" then
      ntop.setHashCache(k, entry, resolved)
   end
   res = {}
else
   -- should never be reached

end

print(json.encode(res, nil))
