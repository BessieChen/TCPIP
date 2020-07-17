#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

int main(void)
{
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
	//����server��socket
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

	//��server��socket��server�ĵ�ַ�˿�
	struct sockaddr_in sin; //SOCKADDR_IN sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(12345);//��Ҫ��server�Ķ˿�һ��
	InetPton(AF_INET, TEXT("127.0.0.1"), &sin.sin_addr.s_addr);//��Ҫ��server�ĵ�ַһ��
	int bres = connect(socketServer, (const struct sockaddr*)&sin, sizeof(struct sockaddr_in));
	if (SOCKET_ERROR == bres) {
		int err = WSAGetLastError();
		closesocket(socketServer);
		WSACleanup();
		return 0;
	}

	//��ʼͨ��
	while (1) {
		char buf[1500] = { 0 };
		
		scanf("%s", buf);
		if ('0' == buf[0]) break;
		int sres = send(socketServer, buf, strlen(buf), 0);
		if (sres == SOCKET_ERROR) {
			int a = WSAGetLastError();
		}

		//int res = recv(socketServer, buf, 1499, 0);
		//if (res == 0)
		//{
		//	printf("���ӶϿ�");
		//	return 0;
		//}
		//else if (res == SOCKET_ERROR)
		//{
		//	//error
		//}
		//else
		//{
		//	//success
		//	printf("%s\n", buf);
		//}

	}


	//����socket�ر�
	closesocket(socketServer);
	WSACleanup();

	system("pause");
	return 0;
}
