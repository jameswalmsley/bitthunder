
#include <bitthunder.h>
#include "lwip/sockets.h"

BT_DEF_MODULE_NAME			("BitThunder Socket Layer")
BT_DEF_MODULE_DESCRIPTION	("Provides a berkeley sockets api for bitthunder")
BT_DEF_MODULE_AUTHOR		("Robert Steinbauer")
BT_DEF_MODULE_EMAIL			("rsteinbauer@riegl.com")

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER	h;				///< All handles must include a handle header.
	int					socket;
};

static const BT_IF_HANDLE oHandleInterface;

static BT_ERROR socket_cleanup(BT_HANDLE hSocket) {
	lwip_close(hSocket->socket);
	return BT_ERR_NONE;
}


int accept(int s, struct sockaddr *addr, socklen_t *addrlen) {
	BT_HANDLE hSocket = (BT_HANDLE)s;

	int new_socket = lwip_accept(hSocket->socket, addr, addrlen);
	if(new_socket) {
		BT_ERROR Error;
		BT_HANDLE h = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), &Error);
		if(!h) {
			lwip_close(new_socket);
			return 0;
		}
		h->socket = new_socket;
		return (int)h;
	}

	return 0;
}

int bind(int s, const struct sockaddr *name, socklen_t namelen) {
	BT_HANDLE hSocket = (BT_HANDLE)s;

	return lwip_bind(hSocket->socket, name, namelen);
}

int shutdown(int s, int how) {
	BT_HANDLE hSocket = (BT_HANDLE)s;

	return lwip_shutdown(hSocket->socket, how);

}

int getpeername (int s, struct sockaddr *name, socklen_t *namelen) {
	BT_HANDLE hSocket = (BT_HANDLE)s;

	return lwip_getpeername(hSocket->socket, name, namelen);
}

int getsockname (int s, struct sockaddr *name, socklen_t *namelen) {
	BT_HANDLE hSocket = (BT_HANDLE)s;

	return lwip_getsockname(hSocket->socket, name, namelen);
}

int getsockopt (int s, int level, int optname, void *optval, socklen_t *optlen) {
	BT_HANDLE hSocket = (BT_HANDLE)s;

	return lwip_getsockopt(hSocket->socket, level, optname, optval, optlen);
}

int setsockopt (int s, int level, int optname, const void *optval, socklen_t optlen) {
	BT_HANDLE hSocket = (BT_HANDLE)s;

	return lwip_setsockopt(hSocket->socket, level, optname, optval, optlen);
}

int closesocket(int s) {
	BT_CloseHandle((BT_HANDLE)s);

	return 0;
}

int connect(int s, const struct sockaddr *name, socklen_t namelen) {
	BT_HANDLE hSocket = (BT_HANDLE)s;

	return lwip_connect(hSocket->socket, name, namelen);
}

int listen(int s, int backlog) {
	BT_HANDLE hSocket = (BT_HANDLE)s;

	return lwip_listen(hSocket->socket, backlog);
}

int recv(int s, void *mem, size_t len, int flags) {
	BT_HANDLE hSocket = (BT_HANDLE)s;

	return lwip_recv(hSocket->socket, mem, len, flags);
}

int read(int s, void *mem, size_t len) {
	BT_HANDLE hSocket = (BT_HANDLE)s;

	return lwip_read(hSocket->socket, mem, len);
}

int recvfrom(int s, void *mem, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen) {
	BT_HANDLE hSocket = (BT_HANDLE)s;

	return lwip_recvfrom(hSocket->socket, mem, len, flags, from, fromlen);
}

int send(int s, const void *dataptr, size_t size, int flags) {
	BT_HANDLE hSocket = (BT_HANDLE)s;

	return lwip_send(hSocket->socket, dataptr, size, flags);
}

int sendto(int s, const void *dataptr, size_t size, int flags, const struct sockaddr *to, socklen_t tolen) {
	BT_HANDLE hSocket = (BT_HANDLE)s;

	return lwip_sendto(hSocket->socket, dataptr, size, flags, to, tolen);
}

int socket(int domain, int type, int protocol) {

	if(!BT_isNetworkingReady()) {
		return -1;
	}

	BT_ERROR Error;
	BT_HANDLE hSocket = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), &Error);
	if(!hSocket) {
		return 0;
	}

	hSocket->socket = lwip_socket(domain, type, protocol);

	return (int)hSocket;
}

int write(int s, const void *dataptr, size_t size) {
	BT_HANDLE hSocket = (BT_HANDLE)s;

	return lwip_write(hSocket->socket, dataptr, size);
}

int select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset, struct timeval *timeout) {

	return lwip_select(maxfdp1, readset, writeset, exceptset, timeout);
}

int ioctl(int s, long cmd, void *argp)  {
	BT_HANDLE hSocket = (BT_HANDLE)s;

	return lwip_ioctl(hSocket->socket, cmd, argp);
}

int fcntl(int s, int cmd, int val) {
	BT_HANDLE hSocket = (BT_HANDLE)s;

	return lwip_fcntl(hSocket->socket, cmd, val);
}


static BT_s32 socket_read(BT_HANDLE hSocket, BT_u32 ulFlags, BT_u32 ulSize, void *pBuffer) {
	return BT_ERR_GENERIC;
}

static BT_s32 socket_write(BT_HANDLE hSocket, BT_u32 ulFlags, BT_u32 ulSize, const void *pBuffer) {
	return BT_ERR_GENERIC;
}

/**
 *	Here we allow socket handles to be passed into BT_Read and BT_Write apis.
 *
 *
 **/
static const BT_IF_FILE oFileOperations = {
	.pfnRead 	= socket_read,
	.pfnWrite	= socket_write,
};

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.eType = BT_HANDLE_T_SYSTEM,
	.pfnCleanup = socket_cleanup,
	.pFileIF = &oFileOperations,
};
