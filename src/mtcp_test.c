#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/queue.h>

#include <mtcp_api.h>
#include <mtcp_epoll.h>

/* OBSOLETE *
struct wget_stat
{
	uint64_t waits;
	uint64_t events;
	uint64_t connects;
	uint64_t reads;
	uint64_t writes;
	uint64_t completes;

	uint64_t errors;
	uint64_t timedout;

	uint64_t sum_resp_time;
	uint64_t max_resp_time;
};

struct thread_context
{
	int core;

	mctx_t mctx;
	int ep;
	int wvar_size;
	struct wget_vars *wvars;

	int target;
	int started;
	int errors;
	int incompletes;
	int done;
	int pending;

	//struct wget_stat stat;
};
typedef struct thread_context* thread_context_t;

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

	//struct timeval t_start;
	//struct timeval t_end;
	
	int fd;
};
 * OBSOLETE */

/* <STRUCTS AND TYPEDEFS> * ALREADY INCLUDED *
struct mtcp_context
{
	int cpu;
};
typedef struct mtcp_context* mctx_t;

typedef struct in_addr* in_addr_t;
 * </STRUCTS AND TYPEDEFS> * ALREADY INCLUDED */

/* <EPOLL FUNCTION WRAPPERS> * UNSUPPORTED *
int mg_epoll_create(mctx_t mctx, int size)
{
	return mtcp_epoll_create(mctx, size);
}

int mg_epoll_wait(mctx_t mctx, int epollid, int maxevents, int timeout)
{
	struct mtcp_epoll_event *events = (struct mtcp_epoll_event *)calloc(maxevents, sizeof(struct mtcp_epoll_event));
	return mtcp_epoll_wait(mctx, epollid, events, maxevents, timeout);
}

int mg_epoll_ctl(mctx_t mctx, int epollid, int op, int sockid, struct mtcp_epoll_event *event)
{
	return mtcp_epoll_ctl(ctx->mctx, epollid, op, sockid, event);
}
 * </EPOLL FUNCTION WRAPPERS> * UNSUPPORTED */

/* <SOCKET FLAG SETTERS> * UNSUPPORTED *
int mg_setsock_nonblock(mctx_t mctx, int sockid)
{
	return mtcp_setsock_nonblock(mctx, sockid);
}
 * </SOCKET FLAG SETTERS> * UNSUPPORTED */

/* <BSD SOCKET INTERFACE> */
int mg_tcp_socket(mctx_t mctx, int domain, int type, int protocol)
{
	return mtcp_socket(mctx, domain, type, protocol);
}

int mg_tcp_connect(mctx_t mctx, int sockid, const struct sockaddr_in *addr, socklen_t addrlen)
{
	return mtcp_connect(mctx, sockid,(struct sockaddr *)addr, addrlen);
}

int mg_tcp_write(mctx_t mctx, int sockid, char *buf, int len)
{
	return mtcp_write(mctx, sockid, buf, len);
}

int mg_tcp_read(mctx_t mctx, int sockid, char *buf, int len)
{
	return mtcp_read(mctx, sockid, buf, len);
}
/* </BSD SOCKET INTERFACE> */

/* <MTCP INITIALIZATION> */
int mg_tcp_init(char *config_file)
{
	return mtcp_init(config_file);
}

mctx_t mg_tcp_create_context(int core)
{
	    return mtcp_create_context(core);
}

int mg_init_rss(mctx_t mctx, in_addr_t saddr_base, int num_addr, in_addr_t daddr, in_addr_t dport)
{
	return mtcp_init_rss(mctx, saddr_base, num_addr, daddr, dport);
}
/* </MTCP INITIALIZATION> */

/*complex wrappers obsolete*
void DestroyContext(thread_context_t ctx) 
{
	mtcp_destroy_context(ctx->mctx);
	free(ctx);
}

void InitMTCP(const char* config_file)
{
	printf("! Initializing MTCP...");
	int max_concurrency = 300;
	int core_limit = GetNumCPUs();
	
	//Set Core Limit & max fds
	struct mtcp_conf mcfg;
	mtcp_getconf(&mcfg);
	mcfg.num_cores = core_limit;
	mcfg.max_concurrency = max_concurrency;
	mcfg.max_num_buffers = max_concurrency;
	mtcp_setconf(&mcfg);
	
	
	mtcp_init(config_file);
}

int SetSocketNonBlock(thread_context_t ctx, int sockId)
{
	int ret = mtcp_setsock_nonblock(ctx->mctx, sockId);
	if (ret < 0) {
		printf("ERROR: Unable to set socket to \"nonblock\"\n"); 
	}
	return ret;
}



int TCPConnect(thread_context_t ctx, const char* src_ip, const char* dst_ip, int port)
{
	int flows = 1;
	int maxevents = 900;
	
	//int ep;
	if(!ctx)
		printf("ERROR: thread_context is NULL\n");
	printf("core=%i", ctx->core);
	//CREATE SOCKET
	//1: mctx_t, domain, type, protocol
	printf("LOADING SOCKET\n");
	int sockid = mtcp_socket(ctx->mctx, AF_INET, SOCK_STREAM, 0);
	if (sockid < 0) {
		return -1;
	} 

	//set non-blocking
	//mtcp_setsock_nonblock(ctx->mctx, sockid);

	//RESERVE MEMORY FOR STATS
	printf("RESERVE MEMORY\n");
	printf("sockid=%i\n", sockid);
	if(ctx->wvar_size <= sockid)
	{
		struct wget_vars *old_vars = ctx->wvars;
		ctx->wvars = (struct wget_vars*)calloc((ctx->wvar_size*2), sizeof(struct wget_vars));
		memcpy(old_vars, ctx->wvars, ctx->wvar_size*sizeof(struct wget_vars));
		ctx->wvar_size*=2;
	}
	
	//Init mtcp stuff
	mtcp_init_rss(ctx->mctx, inet_addr(src_ip), flows, inet_addr(dst_ip), port);
	ctx->target = flows;
	
	//ep = mtcp_epoll_create(ctx->mctx, maxevents);
	//ctx->ep = ep;
	
	ctx->started = ctx->done = ctx->pending = 0;
	ctx->errors = ctx->incompletes = 0;
	
	// ==== Create Connection ====
	struct mtcp_epoll_event ev;
	struct sockaddr_in *addr = calloc(1,sizeof(struct sockaddr_in));
	
	memset(&ctx->wvars[sockid], 0, sizeof(struct wget_vars));
	
	//SET PORT IPCONFIG
	addr->sin_family = AF_INET;
	addr->sin_addr.s_addr = inet_addr(dst_ip);
	addr->sin_port = port;


	//CONNECT
	//mctx, socket_id, struct sockaddr*, struct sockaddr_in/out size
	printf("CONNECTING...\n");
	int ret = mtcp_connect(ctx->mctx, sockid, (struct sockaddr *)addr, sizeof(struct sockaddr_in));
	printf("CONNECTED: %i\n", ret);
	if (ret < 0) {
		mtcp_close(ctx->mctx, sockid);
		return -1;
	}
	
	//UPDATE STATS
	ctx->started++;
	ctx->pending++;
	//ctx->stat.connects++;

	//POLL EVENTS
	ev.events = MTCP_EPOLLOUT;
	ev.data.sockid = sockid;
	//mtcp_epoll_ctl(ctx->mctx, ctx->ep, MTCP_EPOLL_CTL_ADD, sockid, &ev);
	
	return sockid;
}

int TCPSend(thread_context_t ctx, int socket, const char* buffer, int len)
{
	return mtcp_write(ctx->mctx, socket, buffer, len);
}
*complex wrappers obsolete*/
