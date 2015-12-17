local ffi = require "ffi"
--structs
ffi.cdef[[
	struct mtcp_context
	{
		int cpu;
	};
	
	struct thread_context
	{
		int core;

		struct mtcp_context *mctx;
		int ep;
		struct wget_vars *wvars;

		int target;
		int started;
		int errors;
		int incompletes;
		int done;
		int pending;
	};
	
	struct wget_vars
	{
		int request_sent;
	
		char response[30];
		int resp_len;
		int headerset;
		uint32_t header_len;
		uint64_t file_len;
		uint64_t recv;
		uint64_t write;
		
		int fd;
	};
]]

--functions
ffi.cdef[[
	
	struct thread_context* CreateContext(int core);
	void DestroyContext(struct thread_context *ctx);
	int TCPConnect(struct thread_context *ctx, const char* src_ip,  const char* dst_ip, int port);
	int TCPSend(struct thread_context *ctx, int socket, const char* buffer, int len);
	void WriteCoreLimit();
	void InitMTCP(const char* config_file);
]]

return ffi.C