#include <bitthunder.h>
#include <shell/bt_env.h>
#include <sys/socket.h>
#include <netdb.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define TFTP_RRQ 	0x0001
#define TFTP_WRP	0x0002
#define TFTP_DATA	0x0003
#define TFTP_ACK 	0x0004
#define TFTP_ERROR 	0x0005

struct ack_packet {
	BT_u16 	opcode;
	BT_u16	block_nr;
};

struct error_packet {
	BT_u16 	opcode;
	BT_u16 	error_code;
	char   	message[1];
};

struct data_packet {
	BT_u16	opcode;
	BT_u16	block_nr;
	char	data[1];
};

struct tftp_packet {
	BT_u16 	opcode;
	char 	info[514];
};

static int bt_tftp_command(int argc, char **argv) {

	BT_ENV_VARIABLE *server_host = BT_ShellGetEnv("server-host");
	if(!server_host) {
		bt_printf("Error: env: server-host variable not defined.\n");
		return -1;
	}

	if(argc != 3) {
		bt_printf("Usage: %s 0x[address] [remote-path]\n", argv[0]);
		return -1;
	}

	int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	struct sockaddr_in sad;

	bt_printf("sockfd = %08x\n", sockfd);

	memset(&sad, 0, sizeof(sad));
	sad.sin_family = AF_INET;
	sad.sin_port = htons(69);

	int timeout = 3;

	struct hostent *ptrh = gethostbyname(server_host->o.string->s);
	while(!ptrh) {
		ptrh = gethostbyname(server_host->o.string->s);
		if(!timeout--) {
			break;
		}
	}

	if(ptrh) {
		bt_printf("ptrh->h_addr: %d.%d.%d.%d\n", ptrh->h_addr[0], ptrh->h_addr[1], ptrh->h_addr[2], ptrh->h_addr[3]);
	} else {
		bt_printf("Could not resolve hostname: %s\n", server_host->o.string->s);
		goto cleanup_socket;
	}

	memcpy(&sad.sin_addr, ptrh->h_addr, ptrh->h_length);

	void *pBuffer = BT_kMalloc(1024);
	struct tftp_packet *packet = (struct tftp_packet *) pBuffer;
	if(!packet) {
		goto cleanup_socket;
	}

	packet->opcode = htons(TFTP_RRQ);
	int n = sprintf(packet->info, "%s%c%s%c", argv[2], '\0', "octet", '\0');

	n = sendto(sockfd, (void *) packet, n+2, 0, (struct sockaddr *) &sad, sizeof(sad));

	struct sockaddr rxaddr;
	socklen_t rxaddr_len = sizeof(rxaddr);

	int sock_opt = 2500;
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &sock_opt, sizeof(sock_opt));

	n = recvfrom(sockfd, pBuffer, 1024, 0, &rxaddr, &rxaddr_len);
	if(n) {
		bt_printf("Received %d bytes\n");
	} else {
		bt_printf("No response from server\n");
		goto err_free_buffer;
	}

	packet = (struct tftp_packet *) pBuffer;
	switch(ntohs(packet->opcode)) {
	case TFTP_ERROR: {
		struct error_packet *err_packet = (struct error_packet *) packet;
		bt_printf("tftp error: %d - %s\n", ntohs(err_packet->error_code), err_packet->message);
		goto err_free_buffer;
		break;
	}

	case TFTP_DATA: {
		struct data_packet *data_packet = (struct data_packet *) packet;
		BT_u32 blocksize = 512;
		BT_u32 length;
		BT_u32 address = strtoul(argv[1], NULL, 16);
		BT_u8 *dest = (BT_u8 *) address;
		BT_u32 total_length = 0;
		BT_u32 last_block = 0;

		do {
			BT_u16 block = ntohs(data_packet->block_nr);
			length = n - 4;
			memcpy(dest + ((block-1) * blocksize), data_packet->data, length);

			// Send ack
			struct ack_packet *ack_packet = (struct ack_packet *) packet;
			ack_packet->opcode 		= htons(TFTP_ACK);
			ack_packet->block_nr 	= htons(block);

			struct sockaddr_in *server_addr = (struct sockaddr_in *) &rxaddr;
			sad.sin_port = server_addr->sin_port;

			sendto(sockfd, (void *) ack_packet, 4, 0, (struct sockaddr *) &sad, sizeof(sad));
			if(block > last_block) {
				total_length += length;
			}

			last_block = block;

			n = recvfrom(sockfd, pBuffer, 1024, 0, &rxaddr, &rxaddr_len);
		} while(length == 512);

		bt_printf("received %d bytes\n", total_length);

		sprintf(pBuffer, "%d", (int) total_length);

		BT_ShellSetEnv("tftp-length", pBuffer, BT_ENV_T_STRING);

		break;

	}

	default:
		goto err_free_buffer;
	}


err_free_buffer:
	BT_kFree(pBuffer);

cleanup_socket:
	if(sockfd) {
		closesocket(sockfd);
	}
	return 0;
}

BT_SHELL_COMMAND_DEF oCommand = {
	.szpName = "tftp",
	.eType = BT_SHELL_NORMAL_COMMAND,
	.pfnCommand = bt_tftp_command,
};
