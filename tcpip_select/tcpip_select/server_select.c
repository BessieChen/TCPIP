#define _CRT_SECURE_NO_WARNINGS
#define FD_SETSIZE 128
#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

fd_set allSockets;
BOOL WINAPI func(DWORD dwCtrlType) {
	switch (dwCtrlType) {
	case CTRL_CLOSE_EVENT: 
		//释放
		for (u_int i = 0; i < allSockets.fd_count; i++) {
			closesocket(allSockets.fd_array[i]);
		}
		WSACleanup();
	}
	return TRUE;
}
int main(void)
{
	SetConsoleCtrlHandler(func, TRUE);
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
		fd_set writeArray = allSockets;
		FD_CLR(socketServer, &writeArray);//因为server给自己send是不符合逻辑的,所以把socketServer从writearray删除
		fd_set errorArray = allSockets;
		int res = select(0, &tempArray, &writeArray, &errorArray, &tv);
		if (res == 0) {
			continue;
		}
		else if (res > 0) {
			//read有响应
			for (u_int i = 0; i < tempArray.fd_count; i++) {
				SOCKET current = tempArray.fd_array[i];
				//if (tempArray.fd_array[i] == socketServer) {
				//	//响应传来的是socketServer,需要链接它
				//	//也就是通过accept()去生成一个client的socket
				//	SOCKET clientSocket = accept(tempArray.fd_array[i], NULL, NULL);
				//	if (clientSocket == INVALID_SOCKET)
				//	{
				//		printf("clientSocket is wrong");
				//		int a = WSAGetLastError();
				//		//printf(a);
				//		continue;
				//	}

				//	//也就是链接好了一个新client,需要加入socketArray中
				//	FD_SET(clientSocket, &allSockets);
				//	printf("add one client.\n");
				//}
				//else {
				//	char buffer[1500] = { 0 };
				//	int res = recv(tempArray.fd_array[i], buffer, 1499, 0);
				//	if (res == 0) {
				//		//client下线
				//		//需要从array中删除clientSocket并且关闭这个socket
				//		SOCKET temp = tempArray.fd_array[i];
				//		FD_CLR(tempArray.fd_array[i], &allSockets);
				//		closesocket(temp);
				//		printf("remove one client.\n");
				//		continue;
				//	}
				//	else if (res > 0)
				//	{
				//		printf("%s\n",buffer);
				//		continue;
				//	}
				//	else {
				//		printf("error in recv()");
				//		int a = WSAGetLastError();
				//		//printf(a);
				//	}
				//}

				if (current == socketServer) {
					//响应传来的是socketServer,需要链接它
					//也就是通过accept()去生成一个client的socket
					SOCKET clientSocket = accept(current, NULL, NULL);
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
					int res = recv(current, buffer, 1499, 0);
					if (res == 0) {
						//client下线
						//需要从array中删除clientSocket并且关闭这个socket
						SOCKET temp = current;
						FD_CLR(current, &allSockets);
						closesocket(temp);
						printf("remove one client.\n");
						continue;
					}
					else if (res > 0)
					{
						printf("%s\n", buffer);
						continue;
					}
					else {
						printf("error in recv()\n");
						int a = WSAGetLastError();
						switch (a)
						{
						case 10054:
						{
							SOCKET temp = tempArray.fd_array[i];//需要在case里面加花括号,否则显示"声明不能包含标签"
							FD_CLR(tempArray.fd_array[i], &allSockets);
							closesocket(temp);
						}
						}
					}
				}
			}
			for (u_int i = 0; i < writeArray.fd_count; i++) {
				if (send(writeArray.fd_array[i], "hi", 2, 0) == SOCKET_ERROR) {
					int a = WSAGetLastError();
				}
			}
			for (u_int i = 0; i < errorArray.fd_count; i++) {
				char buf[100] = { 0 };
				int buflen = 99;
				if (getsockopt(errorArray.fd_array[i], SOL_SOCKET, SO_ERROR, buf, &buflen) == SOCKET_ERROR)
					printf("no error info\n");
			}
		}
		else
		{
			break;//遇到错误的时候退出,正常的服务器不能退出
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
	for(u_int i = 0; i < allSockets.fd_count; i++) {
		closesocket(allSockets.fd_array[i]);
	}
	WSACleanup();

	system("pause");
	return 0;
}
