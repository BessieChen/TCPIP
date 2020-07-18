#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

int main(void) {
	//创建网络库
	WORD version = MAKEWORD(2, 2);
	WSADATA wdSocketMsg;
	int ret = WSAStartup(version, &wdSocketMsg);
	if (ret != 0) {
		//失败
		switch (ret) {
		case WSASYSNOTREADY:
			printf("system not ready, reboot your computer.\n");
			break;
		case WSAVERNOTSUPPORTED:
			printf("version not support, update your web lib.\n");
			break;
		case WSAEPROCLIM:
			printf("reach limit.\n");
			break;
		case WSAEFAULT:
			printf("process blocked.\n");
			break;
		}
	}
	//校验网络库版本
	int main_version = 2;
	int sub_version = 2;
	if (main_version != HIBYTE(wdSocketMsg.wVersion) || sub_version != LOBYTE(wdSocketMsg.wVersion))
	{
		WSACleanup();
		return 0;
	}
	//创建socket
	SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSocket == INVALID_SOCKET) {
		int ret = WSAGetLastError();
		switch (ret) {
		case WSANOTINITIALISED:
			printf("not initialized.\n");
			break;
		case WSAENETDOWN:
			printf("net down.\n");
			break;
		}
		WSACleanup();
		return 0;
	}

	//为socket绑定地址和端口
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(12345);
	InetPton(AF_INET, TEXT("127.0.0.1"), &sin.sin_addr.s_addr);
	int sret = bind(serverSocket, (const struct sockaddr*) &sin, sizeof(struct sockaddr));
	if (sret == SOCKET_ERROR) {
		closesocket(serverSocket);
		WSACleanup();
		return 0;
	}

	//开始监听
	int lret = listen(serverSocket, SOMAXCONN);
	if (lret == SOCKET_ERROR) {
		closesocket(serverSocket);
		WSACleanup();
		return 0;
	}

	//events select


	//关闭网络库
	closesocket(serverSocket);
	WSACleanup();
	return 0;

}