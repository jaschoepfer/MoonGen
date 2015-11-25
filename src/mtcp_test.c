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

//GLOBALS
#define MAX_CORES 4

static pthread_t app_thread[MAX_CORES];
static mctx_t g_mctx[MAX_CORES];

struct thread_args
{
	const char* ip;
	int port;
	int core;
};

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

thread_context_t CreateContext(int core)
{
	thread_context_t ctx;

	ctx = (thread_context_t)calloc(1, sizeof(struct thread_context));
	if (!ctx) {
		return NULL;
	}
	ctx->core = core;

	ctx->mctx = mtcp_create_context(core);
	if (!ctx->mctx) {
		return NULL;
	}
	g_mctx[core] = ctx->mctx;

	return ctx;
}

void DestroyContext(thread_context_t ctx) 
{
	mtcp_destroy_context(ctx->mctx);
	free(ctx);
}

int TCPConnect(thread_context_t ctx, const char* ip, int port)
{
	struct mtcp_epoll_event ev;
	struct sockaddr_in addr;
	
	//CREATE SOCKET
	//1: mctx_t, domain, type, protocol
	int sockid = mtcp_socket(ctx->mctx, AF_INET, SOCK_STREAM, 0);
	if (sockid < 0) {
		return -1;
	}
	
	//RESERVE MEMORY FOR STATS
	memset(&ctx->wvars[sockid], 0, sizeof(struct wget_vars));
	int ret = mtcp_setsock_nonblock(ctx->mctx, sockid);
	if (ret < 0) {
		exit(-1);
	}
	
	//SET PORT IPCONFIG
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ip);
	addr.sin_port = port;
	
	//CONNECT
	//mctx, socket_id, struct sockaddr*, struct sockaddr_in/out size
	ret = mtcp_connect(ctx->mctx, sockid, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
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
	mtcp_epoll_ctl(ctx->mctx, ctx->ep, MTCP_EPOLL_CTL_ADD, sockid, &ev);
	
	return sockid;
}

void* SlaveMain(void* args)
{
	struct thread_args th_args = *(struct thread_args*)args;
	thread_context_t ctx = CreateContext(th_args.core);
	int sockid = TCPConnect(ctx, th_args.ip, th_args.port);
	
	//set the payload
	const char* buffer = "2357";
	int len = 4;
	
	//send many packets
	int i = 0;
	int packetCount = 1000;
	while(i++ < packetCount)
	{
		mtcp_write(ctx->mctx, sockid, buffer, len);
	}
	
	DestroyContext(ctx);

	pthread_exit(NULL);
	return NULL;
}

int TCPSend(thread_context_t ctx, int socket, const char* buffer, int len)
{
	return mtcp_write(ctx->mctx, socket, buffer, len);
}

int MTCP_TEST_main()
{
	const char* ip = "10.0.0.1";
	int port = 2200;
	
	int i;
	//Create Threads for every CORE
	for (i = 0; i < MAX_CORES; i++) {

		struct thread_args th_args;
		th_args.ip = ip;
		th_args.port = port;
		th_args.core = i;
		if (pthread_create(&app_thread[i], 
					NULL, SlaveMain, (void *)&th_args)) {
			exit(-1);
		}
	}
	
	//Join all threads
	for (i = 0; i < MAX_CORES; i++) {
		pthread_join(app_thread[i], NULL);
	}

	mtcp_destroy();
	return 0;
}