
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
BT_EXPORT_SYMBOL(accept);

int bind(int s, const struct sockaddr *name, socklen_t namelen) {
	BT_HANDLE hSocket = (BT_HANDLE)s;

	return lwip_bind(hSocket->socket, name, namelen);
}
BT_EXPORT_SYMBOL(bind);

int shutdown(int s, int how) {
	BT_HANDLE hSocket = (BT_HANDLE)s;

	return lwip_shutdown(hSocket->socket, how);
}
BT_EXPORT_SYMBOL(shutdown);

int getpeername(int s, struct sockaddr *name, socklen_t *namelen) {
	BT_HANDLE hSocket = (BT_HANDLE)s;

	return lwip_getpeername(hSocket->socket, name, namelen);
}
BT_EXPORT_SYMBOL(getpeername);

int getsockname(int s, struct sockaddr *name, socklen_t *namelen) {
	BT_HANDLE hSocket = (BT_HANDLE)s;

	return lwip_getsockname(hSocket->socket, name, namelen);
}
BT_EXPORT_SYMBOL(getsockname);

int getsockopt(int s, int level, int optname, void *optval, socklen_t *optlen) {
	BT_HANDLE hSocket = (BT_HANDLE)s;

	return lwip_getsockopt(hSocket->socket, level, optname, optval, optlen);
}
BT_EXPORT_SYMBOL(getsockopt);

int setsockopt(int s, int level, int optname, const void *optval, socklen_t optlen) {
	BT_HANDLE hSocket = (BT_HANDLE)s;

	return lwip_setsockopt(hSocket->socket, level, optname, optval, optlen);
}
BT_EXPORT_SYMBOL(setsockopt);

int closesocket(int s) {
	BT_CloseHandle((BT_HANDLE)s);

	return 0;
}
BT_EXPORT_SYMBOL(closesocket);

int connect(int s, const struct sockaddr *name, socklen_t namelen) {
	BT_HANDLE hSocket = (BT_HANDLE)s;

	return lwip_connect(hSocket->socket, name, namelen);
}
BT_EXPORT_SYMBOL(connect);

int listen(int s, int backlog) {
	BT_HANDLE hSocket = (BT_HANDLE)s;

	return lwip_listen(hSocket->socket, backlog);
}
BT_EXPORT_SYMBOL(listen);

int recv(int s, void *mem, size_t len, int flags) {
	BT_HANDLE hSocket = (BT_HANDLE)s;

	return lwip_recv(hSocket->socket, mem, len, flags);
}
BT_EXPORT_SYMBOL(recv);

int read(int s, void *mem, size_t len) {
	BT_HANDLE hSocket = (BT_HANDLE)s;

	return lwip_read(hSocket->socket, mem, len);
}
BT_EXPORT_SYMBOL(read);

int recvfrom(int s, void *mem, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen) {
	BT_HANDLE hSocket = (BT_HANDLE)s;

	return lwip_recvfrom(hSocket->socket, mem, len, flags, from, fromlen);
}
BT_EXPORT_SYMBOL(recvfrom);

int send(int s, const void *dataptr, size_t size, int flags) {
	BT_HANDLE hSocket = (BT_HANDLE)s;

	return lwip_send(hSocket->socket, dataptr, size, flags);
}
BT_EXPORT_SYMBOL(send);

int sendto(int s, const void *dataptr, size_t size, int flags, const struct sockaddr *to, socklen_t tolen) {
	BT_HANDLE hSocket = (BT_HANDLE)s;

	return lwip_sendto(hSocket->socket, dataptr, size, flags, to, tolen);
}
BT_EXPORT_SYMBOL(sendto);

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
BT_EXPORT_SYMBOL(socket);

int write(int s, const void *dataptr, size_t size) {
	BT_HANDLE hSocket = (BT_HANDLE)s;

	return lwip_write(hSocket->socket, dataptr, size);
}
BT_EXPORT_SYMBOL(write);

int select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset, struct timeval *timeout) {
	return lwip_select(maxfdp1, readset, writeset, exceptset, timeout);
}
BT_EXPORT_SYMBOL(select);

int ioctl(int s, long cmd, void *argp)  {
	BT_HANDLE hSocket = (BT_HANDLE)s;

	return lwip_ioctl(hSocket->socket, cmd, argp);
}
BT_EXPORT_SYMBOL(ioctl);

int fcntl(int s, int cmd, int val) {
	BT_HANDLE hSocket = (BT_HANDLE)s;

	return lwip_fcntl(hSocket->socket, cmd, val);
}
BT_EXPORT_SYMBOL(fcntl);


static BT_s32 socket_read(BT_HANDLE hSocket, BT_u32 ulFlags, BT_u32 ulSize, void *pBuffer) {

	return lwip_recv(hSocket->socket, pBuffer, ulSize, ulFlags);
	//return BT_ERR_GENERIC;
}

static BT_s32 socket_write(BT_HANDLE hSocket, BT_u32 ulFlags, BT_u32 ulSize, const void *pBuffer) {

	return lwip_send(hSocket->socket, pBuffer, ulSize, ulFlags);
	//return BT_ERR_GENERIC;
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
