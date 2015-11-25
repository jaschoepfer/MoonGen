local mtcp	= require "mtcpc"
local dpdk	= require "dpdk"
local log	= require "log"
local memory= require "memory"
local ffi   = require "ffi"

function master(...)
	
	local txPort, rxPort, rate, flows, size = tonumberall(...)
	--if not txPort or not rxPort then
	--	return log:info("usage: txPort rxPort [rate [flows [pktSize]]]")
	--end
	
	ip = "10.0.0.1"
	port = 6112
	
	flows = flows or 4
	rate = rate or 2000
	size = (size or 124)

	log:info("launching slave...")
	dpdk.launchLuaOnCore(1, "loadSlave", 1, size)
	log:info("thread launched, waiting...")
	dpdk.waitForSlaves()
	log:info("waiting finished")
end

function loadSlave(core, size)
	local mem = memory.createMemPool(function(buf)
		local data = ffi.cast("uint8_t*", buf.pkt.data)
		for i = 0, size do
			data[i] = 0x09
		end
	end)
	local payload = mem:bufArray()
	log:info("creating context...")
	local context = mtcp:CreateContext(core)
	log:info("establishing socket...")
	local socket = mtcp:TCPConnect(context, ip, port)
	local counter = 0
	log:info("start sending 0x09 spam...")
	while counter < 100 do
		counter = counter + 1
		mtcp:TCPSend(context, socket, payload, size)
	end
	log:info("done sending, destroying context...")
	mtcp.DestroyContext(context)
	log:info("context destroyed")
	
end