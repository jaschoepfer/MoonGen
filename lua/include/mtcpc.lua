local ffi = require "ffi"

--structs, typedefs & constants
ffi.cdef[[
	struct mtcp_context
	{
		int cpu;
	};
	typedef struct mtcp_context* mctx_t;

	struct in_addr
	{
		unsigned long int s_addr;
	};


	struct sockaddr_in
	{
		int sin_family;
		unsigned int sin_port;
		struct in_addr sin_addr;
		char sin_zero[8];
	};
	
	struct sockaddr{};

	typedef unsigned long int in_addr_t;

	typedef unsigned long int socklen_t;

]]

--functions
ffi.cdef[[
	int mg_tcp_socket(mctx_t mctx, int domain, int type, int protocol);
	int mg_tcp_connect(mctx_t mctx, int sockid, const struct sockaddr *addr, socklen_t addrlen);
	int mg_tcp_close(mctx_t mctx, int sockid);
	struct sockaddr *mg_alloc_sockaddr(unsigned int family, unsigned int port, const char *ip_addr);
	char *mg_alloc_buffer(const char *src, int offset, int size);
	socklen_t size_sockaddr_in();
	int mg_tcp_write(mctx_t mctx, int sockid, const char *buf, int len);
	int mg_tcp_read(mctx_t mctx, int sockid, char *buf, int len);
	int mg_mtcp_init(const char *config_file);
	mctx_t mg_tcp_create_context(int core);
	int mg_init_rss(mctx_t mctx, in_addr_t saddr_base, int num_addr, in_addr_t daddr, in_addr_t dport);
	in_addr_t mg_ipaddr_hton(const char* ip_addr);
	in_addr_t mg_port_hton(int port);
	void mg_tcp_destroy_context(mctx_t mctx);
]]

return ffi.C
