local ffi = require "ffi"

--structs & typedefs
ffi.cdef[[
	struct mtcp_context
	{
		int cpu;
	};
	typedef struct mtcp_context* mctx_t;
	
	struct socaddr_in
	{
		short sin_family;
		unsigned short sin_port;
		struct in_addr sin_addr;
		char sin_zero[8];
	};
	
	struct sockaddr{};

	struct in_addr
	{
		unsigned long s_addr;
	};

	typedef unsigned long in_addr_t;
	
	typedef int socklen_t;
]]

--functions
ffi.cdef[[
	/* not supported *
	int mg_setsock_nonblock(struct mtcp_context *mctx, int sockid);
	int mg_epoll_ctl(struct thread_context *ctx, int epollid, int op, int sockid, struct mtcp_epoll_event *event);
	int mg_epoll_wait(struct thread_context *ctx, int epollid, int maxevents, int timeout);
	int mg_epoll_create(struct thread_context *ctx, int size);
	* not supported */

	void InitMTCP(const char* config_file);

	int mg_tcp_socket(mctx_t mctx, int domain, int type, int protocol);
	int mg_tcp_connect(mctx_t mctx, int sockid, const struct sockaddr_in *addr, socklen_t addrlen);
	int mg_tcp_write(mctx_t mctx, int sockid, char *buf, int len);
	int mg_tcp_read(mctx_t mctx, int sockid, char *buf, int len);
	int mg_tcp_init(char *config_file);
	mctx_h mg_tcp_create_context(int core);
	int mg_init_rss(mctx_t mctx, in_addr_t saddr_base, int num_addr, in_addr_t daddr, in_addr_t dport);
]]

return ffi.C
