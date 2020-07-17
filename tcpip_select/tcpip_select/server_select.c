#define _CRT_SECURE_NO_WARNINGS
#define FD_SETSIZE 128
#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

int main(void)
{
	WORD wdVersion = MAKEWORD(2, 2);
	WSADATA wdSockMsg;
	//创建网络库
	int nRes = WSAStartup(wdVersion, &wdSockMsg);
	if (0 != nRes) {
		switch (nRes)
		{
		case WSASYSNOTREADY:
			printf("reboot your computer.");
			break;
		case WSAVERNOTSUPPORTED:
			printf("please update your web lib");
			break;
		case WSAEPROCLIM:
			printf("reach the limit");
			break;
		case WSAEINPROGRESS:
			printf("process blocked, please spare enough resources for running this process");
			break;
		case WSAEFAULT:
			printf("lpWSADATA is not a valid pointer, maybe pass a wrong parameter");
			break;
		}
		return 0;
	}

	//检验是否是我们要的网络库
	if (2 != HIBYTE(wdSockMsg.wVersion) || 2 != LOBYTE(wdSockMsg.wVersion))
	{
		WSACleanup();
		return 0;
	}
	//创建socket
	SOCKET socketServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//检验是否成功
	if (INVALID_SOCKET == socketServer)
	{
		int res = WSAGetLastError();
		switch (res)
		{
		case WSANOTINITIALISED:
			break;
		case WSAENETDOWN:
			break;
			//...
		}
		WSACleanup();
		return 0;
	}

	//绑定地址和端口
	struct sockaddr_in sin; //SOCKADDR_IN sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(12345);
	//sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	InetPton(AF_INET, TEXT("127.0.0.1"), &sin.sin_addr.s_addr);
	int bres = bind(socketServer, (const struct sockaddr*)&sin, sizeof(struct sockaddr_in));
	if (SOCKET_ERROR == bres) {
		int err = WSAGetLastError();
		closesocket(socketServer);
		WSACleanup();
		return 0;
	}

	//开始监听
	bres = listen(socketServer, SOMAXCONN);
	if (SOCKET_ERROR == bres) {
		int err = WSAGetLastError();
		closesocket(socketServer);
		WSACleanup();
		return 0;
	}

	//以下是不同于c/s模型的部分
	fd_set allSockets;
	FD_ZERO(&allSockets);
	//将server装入
	FD_SET(socketServer, &allSockets);

	struct timeval tv;
	tv.tv_sec = 100;
	tv.tv_usec = 0;

	while (1) {
		fd_set tempArray = allSockets;
		int res = select(0, &tempArray, NULL, NULL, &tv);
		if (res == 0) {
			continue;
		}
		else if (res > 0) {
			//有响应
			for (u_int i = 0; i < tempArray.fd_count; i++) {
				SOCKET current = tempArray.fd_array[i];
				if (tempArray.fd_array[i] == socketServer) {
					//响应传来的是socketServer,需要链接它
					//也就是通过accept()去生成一个client的socket
					SOCKET clientSocket = accept(tempArray.fd_array[i], NULL, NULL);
					if (clientSocket == INVALID_SOCKET)
					{
						printf("clientSocket is wrong");
						int a = WSAGetLastError();
						//printf(a);
						continue;
					}

					//也就是链接好了一个新client,需要加入socketArray中
					FD_SET(clientSocket, &allSockets);
					printf("add one client.\n");
				}
				else {
					char buffer[1500] = { 0 };
					int res = recv(tempArray.fd_array[i], buffer, 1499, 0);
					if (res == 0) {
						//client下线
						//需要从array中删除clientSocket并且关闭这个socket
						SOCKET temp = tempArray.fd_array[i];
						FD_CLR(tempArray.fd_array[i], &allSockets);
						closesocket(temp);
						printf("remove one client.");
						continue;
					}
					else if (res > 0)
					{
						printf("%s\n",buffer);
						continue;
					}
					else {
						printf("error in recv()");
						int a = WSAGetLastError();
						//printf(a);
					}
				}
			}
		}
		else
		{

		}

	}
	//以下为fd_set()和fd_clr()
	/*
	FD_SET(socketServer, &clientSockets);
	FD_CLR(socketServer, &clientSockets);
	closesocket(socketServer);
	int res = FD_ISSET(socketServer, &clientSockets);
	if (res == 0)
	{
		printf("server socket is not in set");
	}
	*/



	//用完socket关闭
	closesocket(socketServer);
	WSACleanup();

	system("pause");
	return 0;
}
