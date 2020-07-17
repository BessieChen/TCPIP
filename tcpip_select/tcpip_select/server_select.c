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
		//�ͷ�
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
	//���������
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

	//�����Ƿ�������Ҫ�������
	if (2 != HIBYTE(wdSockMsg.wVersion) || 2 != LOBYTE(wdSockMsg.wVersion))
	{
		WSACleanup();
		return 0;
	}
	//����socket
	SOCKET socketServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//�����Ƿ�ɹ�
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

	//�󶨵�ַ�Ͷ˿�
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

	//��ʼ����
	bres = listen(socketServer, SOMAXCONN);
	if (SOCKET_ERROR == bres) {
		int err = WSAGetLastError();
		closesocket(socketServer);
		WSACleanup();
		return 0;
	}

	//�����ǲ�ͬ��c/sģ�͵Ĳ���
	fd_set allSockets;
	FD_ZERO(&allSockets);
	//��serverװ��
	FD_SET(socketServer, &allSockets);

	struct timeval tv;
	tv.tv_sec = 100;
	tv.tv_usec = 0;

	while (1) {
		fd_set tempArray = allSockets;
		fd_set writeArray = allSockets;
		FD_CLR(socketServer, &writeArray);//��Ϊserver���Լ�send�ǲ������߼���,���԰�socketServer��writearrayɾ��
		fd_set errorArray = allSockets;
		int res = select(0, &tempArray, &writeArray, &errorArray, &tv);
		if (res == 0) {
			continue;
		}
		else if (res > 0) {
			//read����Ӧ
			for (u_int i = 0; i < tempArray.fd_count; i++) {
				SOCKET current = tempArray.fd_array[i];
				//if (tempArray.fd_array[i] == socketServer) {
				//	//��Ӧ��������socketServer,��Ҫ������
				//	//Ҳ����ͨ��accept()ȥ����һ��client��socket
				//	SOCKET clientSocket = accept(tempArray.fd_array[i], NULL, NULL);
				//	if (clientSocket == INVALID_SOCKET)
				//	{
				//		printf("clientSocket is wrong");
				//		int a = WSAGetLastError();
				//		//printf(a);
				//		continue;
				//	}

				//	//Ҳ�������Ӻ���һ����client,��Ҫ����socketArray��
				//	FD_SET(clientSocket, &allSockets);
				//	printf("add one client.\n");
				//}
				//else {
				//	char buffer[1500] = { 0 };
				//	int res = recv(tempArray.fd_array[i], buffer, 1499, 0);
				//	if (res == 0) {
				//		//client����
				//		//��Ҫ��array��ɾ��clientSocket���ҹر����socket
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
					//��Ӧ��������socketServer,��Ҫ������
					//Ҳ����ͨ��accept()ȥ����һ��client��socket
					SOCKET clientSocket = accept(current, NULL, NULL);
					if (clientSocket == INVALID_SOCKET)
					{
						printf("clientSocket is wrong");
						int a = WSAGetLastError();
						//printf(a);
						continue;
					}

					//Ҳ�������Ӻ���һ����client,��Ҫ����socketArray��
					FD_SET(clientSocket, &allSockets);
					printf("add one client.\n");
				}
				else {
					char buffer[1500] = { 0 };
					int res = recv(current, buffer, 1499, 0);
					if (res == 0) {
						//client����
						//��Ҫ��array��ɾ��clientSocket���ҹر����socket
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
							SOCKET temp = tempArray.fd_array[i];//��Ҫ��case����ӻ�����,������ʾ"�������ܰ�����ǩ"
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
			break;//���������ʱ���˳�,�����ķ����������˳�
		}

	}
	//����Ϊfd_set()��fd_clr()
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

	//����socket�ر�
	for(u_int i = 0; i < allSockets.fd_count; i++) {
		closesocket(allSockets.fd_array[i]);
	}
	WSACleanup();

	system("pause");
	return 0;
}
