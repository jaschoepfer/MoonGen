local mtcp	= require "mtcpc"
local dpdk	= require "dpdk"
local log	= require "log"
local memory= require "memory"
local ffi   = require "ffi"

function master(...)
	
	local port = tonumberall(...)
	if not port then
		port = 6112
	end
	
	src_ip = "10.0.0.3"
	dst_ip = "10.0.0.4"
	
	flows = flows or 4
	rate = rate or 2000

	local conf = "mtcp.conf"
	log:info("reading mtcp config file from " .. conf)
	local ret = mtcp.mg_mtcp_init(conf)
	log:info("mtcp_init finished: " .. ret)

	log:info("creating context...")
	local context = mtcp.mg_tcp_create_context(1)
	
	log:info("launching slave...")
	local task = dpdk.launchLuaOnCore(2, "loadSlave", src_ip, dst_ip, port, context)
	log:info("TASKID:" .. task.id)
	log:info("thread launched, waiting...")
	--WARNING: Do NOT use dpdk.waitForSlaves() here. create_context() creates
	--(cont.): background threads, that can only be closed by the master core.
	task:wait()
	log:info("waiting finished")

	log:info("destroying context(killing background thread)...")
	mtcp.mg_tcp_destroy_context(context)
	log:info("context destroyed")

	dpdk.waitForSlaves()
end

function loadSlave(src_ip, dst_ip, port, context)
	log:info("slave launched")
	
	local protocol_fam = 2 --AF_INET
	local sock_type = 1 --SOCK_STREAM

	local payload = "\nDoom for you, doom for me, DOOM FOR EVERYONE! I think we need more content, oh dear. Is this 64 chars yet? I think so."
	local size = 64
	local counter, sockaddr

	log:info("establishing socket...")
	local socket = mtcp.mg_tcp_socket(context, protocol_fam , sock_type , 0)
	log:info("socket() finished: " .. socket)

	log:info("initializing adress pool...")
	local saddr = mtcp.mg_ipaddr_hton(src_ip)
	local daddr = mtcp.mg_ipaddr_hton(dst_ip)
	local dport = mtcp.mg_port_hton(port)
	local ret = mtcp.mg_init_rss(context, saddr, 1, daddr, dport)
	log:info("init_rss() finished: " .. ret)

	log:info("establishing socket...")
	local socket = mtcp.mg_tcp_socket(context, protocol_fam , sock_type , 0)
	log:info("socket() finished: " .. socket)
	if ret < 0 then
		log:info("failed to create socket, skipping to mtcp-cleanup")
		goto cleanup_mtcp
	end

	log:info("establishing connection...")
	sockaddr = mtcp.mg_alloc_sockaddr(protocol_fam, port, dst_ip)
	ret = mtcp.mg_tcp_connect(context, socket, sockaddr, mtcp.size_sockaddr_in())
	log:info("connect() finished: " .. ret)
	if ret < 0 then
		log:info("failed to connect, skipping to socket-cleanup")
		goto cleanup_socket
	end
	
	log:info("start sending DOOM spam...")
	counter = 0
	while counter < 10 do
		local buffer = mtcp.mg_alloc_buffer(payload, 0, size)
		counter = counter + 1
		ret = mtcp.mg_tcp_write(context, socket, buffer, size)
		log:info("write()#" .. counter .. " finished: " .. ret)
	end
	log:info("sent 10 payloads")

	::cleanup_socket::
	log:info("closing connection...")
	ret = mtcp.mg_tcp_close(context, socket)
	log:info("close() finished: " .. ret)

	::cleanup_mtcp::
end
