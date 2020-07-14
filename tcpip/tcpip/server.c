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

	bres = listen(socketServer, SOMAXCONN);
	if (SOCKET_ERROR == bres) {
		int err = WSAGetLastError();
		closesocket(socketServer);
		WSACleanup();
		return 0;
	}

	//�����ͻ��˵�socket
	struct sockaddr_in clientsin;
	int len = sizeof(clientsin);
	SOCKET socketClient = accept(socketServer, (struct sockaddr*)&clientsin, &len);
	if (INVALID_SOCKET == socketClient) {
		printf("�ͻ�������ʧ��.");
		int err = WSAGetLastError();
		closesocket(socketServer);
		WSACleanup();
		return 0;
	}
	printf("�ͻ������ӳɹ�.");

	//����socket�ر�
	closesocket(socketClient);
	closesocket(socketServer);
	WSACleanup();

	system("pause");
	return 0;
}

/*
0. ����
	�����С�İ�����insert��,�ͻ���ֻ��ɾ������
1. ����
	WinSock2.h:
		windows socket �׽���
			ͨ�����ļ�,���Կ�����ǰ������֧�ֵ����°汾��,�ҵ���2.2
			���ִ�Сд,����winSock2.hҲ����,���ǻ������Ҫ�淶:WinSock2.h

	#pragma comment(lib, "ws2_32.lib")
		��̬���ӿ�
		��ʽ:
			#pragma comment(lib, "xxx")
		��:
			ws: windows socket
			2: �ڶ��� (��һ�����wsock32.lib)
			32: 32λ(ע��,û��64λ��,32λ���Ѿ���������)
			��̬��ĺ�׺:lib
			lib��cpp����ú�Ķ����ƴ���,�����ǿ�������
			����ʹ�õ�ʱ��,����Ҫ�������,���Խ�ʡ�����ı���ʱ��
			������ֲ��ִ�Сд,����wS2_32.libҲ����
2. �������
	1. int WSAStartUP
		1. ����:��ʼ��winsock dll,���������
			w windows
			s socket
			a asynchronous �첽(��������,����������Ҫ)
				ͬ��:
					һ��������Ҫ�ȴ���һ������
				�첽:
					�������ͬʱ����
			startup ����
		2. ����
			����int
			#1:
				��Ҫ�ĸ��汾�������, WORD����
					WORD��typedef unsigned short, 2���ֽ�, �պ�һ���ֽڷ�������汾,һ���ֽڷ�����Ӱ汾
					WORD wdVersion = MAKEWORD(2, 1); //����д��2.1,�������float��
				makeword()��һ����
					#define MAKEWORD(a, b)   ((WORD)(((BYTE)(((DWORD_PTR)(a)) & 0xff)) | ((WORD)((BYTE)(((DWORD_PTR)(b)) & 0xff))) << 8))
						��ߵ�ֵ,����1111 1111
						�ұߵ�ֵ,����1111 1111,Ȼ������8λ
						����ٻ���������
				������������뷽����2.2����汾���ݽ�ȥ
			#2:
				��λ�ֽ������汾,��λ���Ӱ汾��
					����markword(2,1)֮��Ľ����258
						0000 0001 0000 0010
					���Ը�λ 0000 0001�����Ӱ汾
					���Ե�λ 0000 0010�������汾
					һ��������С��,���Ե͵�ַ���λ
						��������ǵ͵�ַ,������ʾ���ǵ�λ����,Ҳ�������汾
						��ʾ��16����,����ǵ͵�ַ: 2 1
				**��ʦ��һ������ķ���:�ҵ��ڴ��д�ŵĶ���:
					int a = *((char*) &wdVersion);
						����Ľ��: wdVersion�ĵ�һ���ֽڵ�ֵ,Ҳ����2
					int b = *((char*) &wdVersion + 1);
						����Ľ��: wdVersion�ĵڶ����ֽڵ�ֵ,Ҳ����1
			#3:
				LPWSADATA
					LPWSADATA �� WSADATA*, Ҳ����LPWSADATA��WSADATA*��������
						LP:ָ����ָ��,Ҳ����Ҫ��ַ. Ҫwsadata�ĵ�ַ
					������Ҫ����һ���ṹ�����wsadata,Ȼ������ĵ�ַ�����������
					����:
						��ʦ�õ���ջ�ռ�
							WSADATA wdSockMsg;
							WSAStartUp(wdVersion, &wdSockMsg);
						�����Ҫ�öѵĻ�,��Ҫmalloc,֮����Ҫ�ͷ�,̫�鷳
							LPWSADATA lpw = malloc(sizeof(WSADATA)); //Ҳ����WSADATA* lpw =  malloc(sizeof(WSADATA));
							WSAStartUp(wdVersion, lpw);
							free(xxx);
					֮������������һЩ��Ϣ,���ݸ����Ǵ����Ľṹ��.
						���������ַӦ��
		3. ����ֵint
			0
				�ɹ�
			����ֵ:
				���ش������е�һ��
				����
					WSAEPROCLIM
						�Ѿ��ﵽ��win �׽���ʵ����֧�ֵ���������������
							Ҳ���ǲ������޿������
						������Ӳ�����ò���,��������ͽṹ��ĳ�ʼ��
						�����Ǵ�65536��,������Ϊ�˿ڵĴ�С��Χ
							ÿ�δ���Դ,��Ҫ��һ���˿�,�˿���һ��unsigned short
							16bit, 2^16 = 65536
						�����㲻�ܴ�7����˿�
		4. �ɹ�֮��
			���ǿ��Բ鿴����Ϊ���ǵĽṹ��(WSADATA wdSockMsg)������ʲô��Ϣ:
			1. wVersion
				������Ҫʹ�õİ汾
			2. wHighVersion
				ϵͳ���ṩ����߰汾
			3. iMaxSockets(����)
				���ؿ��õ�socket������
			4. iMaxUdpDg(����)
				udp���ݱ���Ϣ�Ĵ�С
			5. ...
		5. ����ʧ��
			1. �������Ҫ1.3
				����û��1.3,ֻ��1.2��2.0
				��ô�ʹ����汾1֧�ֵİ汾,Ҳ����1.2
			2. �������3.0
				����һ���ܴ�İ汾��,�Ǿͷ�������ϵͳ����2.2�汾
			3. ����0.0
				���ش�����10092: ������ʧ��,��֧�������socket�汾
	2. У��汾
		if (2 != HIBYTE(wdSockMsg.wVersion) || 2 != LOBYTE(wdSockMsg.wVersion))
			����Ļ�,˵���汾����, hibyte()��ָ�Ӱ汾,lobyte�����汾
			��������,����ͦ��Ҫ��
		���������: WSACleanUp();

2. ����socket
	����
		���ײ㸴�ӵ�Э����ϵ,ִ������,��װ��,api����socket
		socket�����ǵ���Э�����ͨ�ŵĲ����ӿ�
	����
		���ǲ���Ҫ֪���ײ�Э��ľ���ϸ��,������ʹ��
		�����̾�����Э��ĸ�����,���Ǽ򵥾�����Ϊ��socket
	����
		��һ����������
			typedef UINT_PTR  SOCKET;
			����SOCKET��һ���޷��ŵ�int
		�൱��һ�ű�
			һ�����򴴽���һ��socket,������23,���23����Ψһ��
				23��Ӧ����:������Э��(ͨ������ָ����ͬ��Э��,����tcpЭ��,�������������͵�,ָ��ip��ַ,�˿ں�)
				�Ժ�ͨ�ŵ�ʱ��,ֱ����23ͨ��,ϵͳ���ҵ�23��Э��,������֮ǰָ���ķ�ʽ��ͨ��
			�������򴴽���socket,ֵ������������
				����ֵ��24,Ȼ��ָ������udpЭ��
	Ӧ��
		����ͨ��,����Ҫsocket
			�����client����Ϣ,��Ҫ֪������socket
			����һ��client����Ϣ,Ҳ��Ҫ֪�����client��socket
		ÿһ��client����socket,ÿһ��server����������socket
	����
		1. ��ַ������
			���������΢����ϵ��ʦ,����ʹ��xxx΢�ź�,΢�źž��ǵ�ַ������
			����
				AD_INET
					ipv4
						4�ֽ�,32bit,�޷������� 0 - 4294967295
						��0.0.0.0 - 255.255.255.255
					���ǲ�������
				AF_INET6
					ipv6
						16�ֽ�,128bits,2^128 = 3*10^38
						�����˵����ÿһ��ip
					�Ժ��Խ��Խ�ձ�
				AF_BTH
					������ַ
				AF_IRDA
					������
			ע��
				Ӳ��ҲҪ֧��,������ʹ��af_irda,�����Ҫ�к����߶˿�

		2. socket����
			��������ʲôЭ��
			����
				SOCK_STREAM
					tcp
					�ɿ���:��Ϣ��һ�㶼���ܶ���
						����qq����,����
					˳���:
						��˳��ĵ���
				SOCK_DGRAM
					udp
					���ɿ���:��Ϣ���Զ�
						��Ӱ�Ῠһ��,�еĵط�������
						�������ݴ����յ��˸���
						�����ڹ������ߵ�ʱ����Ҫ�˸���
					����˳���:
						���翴��Ӱ,��������,�Ǽ�ϵĻ����
				SOCK_RAM
				SOCK_RDM
				SOCK_SEQPACKET

		3. Э������
			�����0,ϵͳ�������Ĭ��ѡһ�����ʵ�ֵ.��Ҳ�����Լ�������ֵ
			����
				IPPROTO_TCP
				IPPROTO_UDP
				IPPROTO_ICMP
				IPPROTO_IGMP
				IPPROTO_RM
	����ֵ
		���ʧ����,����Ҫwsacleanup()�����������
		��ʲô����:
			int res = WSAGetLastError(); //һ���ǻ�������error
4. ���ϵ�ַ�Ͷ˿�
	��ַ
		ÿһ������,����һ����ַ
		����ʹ��ipv4,��ô��ַ����ip��ַ
		���:	
			��ַ������,qq��
			��ô��Ӧ�ĵ�ַ��������10348712
	�˿�
		ÿ�����,����һ���˿ں�
			������Ϣ�Ǵ��ݵ�a���,���Ǵ��ݵ�b���
		����qq��һ���˿ں�,�������һ���˿ں�
			ע��:
				ͬһ����������ж���˿ں�
					qq������Ϣ:��tcp��Ӧ�Ķ˿ں�
					qq����Ƶ:��udp��Ӧ�Ķ˿ں�
	ʵ��
		int bind()
			����1
				�󶨵�ַ�Ͷ˿ں�
			����2
				����sockaddr*
					��������Ҫ����һ��sockaddr�Ľṹ��
					�������������һЩ:
						struct sockaddr_in{
							short sin_family;
							u_short sin_prot;
							struct in_addr sin_addr;
							char sin_zero[8];
							}
						��������,��Ҫ����sockaddr,���������������sockaddr_in����,������
							����,��Ϊ
								sockaddr��ushort �� char xx[14]
									����һ����2+14=16���ֽ�
									��Ϊ�ڴ����,�ǰ�������������,�����ڴ����ռ16�ֽ�
								sockaddr_in
									��Ϊshort��2�ֽ�,u_short��2�ֽ�,struct��Ϊ��32λip��ַ������4�ֽ�
									Ȼ��char xx[8]Ҳ��8�ֽ�
									����Ҳ��һ��16�ֽ�
							��Ȼ�ڴ�һ����,��������ǿ��ת����ok��
					������������Ǵ���һ��sockaddr_in
						ע��:
							���ֶԵ���д��ʽ:
								struct sockaddr_in xx;
								SOCKADDR_IN xxx;
							��Ϊc������,����������. ����cpp���Բ���Ҫstruct,����ֱ��дsockaddr_in xxx;
							Ȼ���㿴���Ķ���
								typedef struct sockaddr_in{...} SOCKADDR_IN
								��˵��SOCKADDR_IN �� struct sockaddr_in ���ض���
						����
							xx.sin_family = AF_INET;
								һ��Ҫ��Ӧsocket(AF_INET,xxx),��Ϊ������Ҫ��
							xx.sin_port = htons(27015);
								ʹ�ú궨��htons(),���������������ת���ɶ˿ں�
								**�˿ں�
									����:
										unsigned short: 16bits,2�ֽ�
											����һ������ 0 - 65535
									������д��ֵ
										1. ������0-65535������
										2. ʵ����,����д����1024-65535,���д��һ���ֵ
											1. ϵͳ��ʹ��0-1023�Ķ˿ں�
												21�˿ں�:�����FTP(�ļ�����Э��)����
												25:�����SMTP(���ʼ�����Э��)����
												80:�����HTTP����
									��Ϊ�˿ں���Ψһ��,�����������һ���Ѿ�ռ�õĶ˿ں�,�ͻ��ʧ��
										����ж϶˿ں��Ƿ�ʹ��?
											��command,����
												netstat -ano
													�鿴����ʹ�õĶ˿�,���зǳ���
													����, ���ص�ַ����д�� 162.32.42.53.50134
														�������һ��50134���Ƕ˿ں�
												netstat -ano|findstr "12345"
													��ѯĳһ��
													�����ռ��:����ʾ�ó���
													���û�б�ռ��,��ʲô������ʾ
							xx.sin_addr
								���Ҳ��һ���ṹ��struct in_addr sin_addr;
									��仰��:��Աsin_addr��������struct in_addr
									����������֮ǰ˵��typedef struct in_addr{}IN_ADDR;
								��������һ��union,�����������struct
									union:�ռ��ǳ�Ա��,�����Ǹ�,Ȼ���Ҷ����ÿռ�
									���������ǿ�һ����ʲô���ӵ�:
										struct in_addr{
											union{
												struct{
													u_char s_b1;
													u_char s_b2;
													u_char s_b3;
													u_char s_b4;
												}S_un_b;

												struct{
													u_short s_W1;
													u_short s_W2;
												}S_un_w;

												u_long S_addr;
											}S_un;
										};
									����:
										�൱�����ǿ��Դ�Union S_un����ѡһ:
											1. ѡ��һ��struct S_un_b;
												һ��4��u_char, ÿ��1�ֽ�
												һ��4�ֽ�
												�÷�: 193.23.43.53
												����:
													xx.sin_addr.S_un.S_un_b.s_b1 = 199;
													xx.sin_addr.S_un.S_un_b.s_b2 = 99;
													xx.sin_addr.S_un.S_un_b.s_b3 = 99;
													xx.sin_addr.S_un.S_un_b.s_b4 = 19;
												�ǳ��鷳,��ʾunion S_un,���ǵ�һ��struct,��������Ԫ��
											2. ѡ�ڶ���struct S_un_w;
												һ��2��u_short,ÿ��ռ2�ֽ�
												һ��4�ֽ�
											3, ѡ������ u_long S_addr;
												��windowsϵͳ��,������32λ����64λ,u_long����4�ֽ�
												����linux��64λϵͳ,u_long��8�ֽ�
												����:
													xx.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
													inet_addr()Ҳ��һ����,����ת����һ��u_long
									��ַ����
										ʹ��127.0.0.1
											�������ػ��͵�ַ
											���ڱ����������,��������ǵ����ݷ��ͳ�ȥ
												���������Լ��ĵ���,дserver��client,���������ն˽���
											����:���Լ�û������,����û����������wifi�ĵ���
										ʹ��192.168.xx.xx
											��ο����Լ���ip��ַ:
												�����ڿ���̨����ָ��ipconfig �Ϳ�����
											����
												һ������,����һ��������,�ñ��˵�pc��server�ҵĵ�client
									����
										InetPton(AF_INET, TEXT("127.0.0.1"), &sin.sin_addr.s_addr);
													


									
			����3
				��sockaddr�Ĵ�С
					bind(socketServer, (const struct sockaddr*)&sin, sizeof(struct sockaddr_in));
						ע��:
							1. ����ǿ������ת����,��Ϊsockaddr��struct,��Ҫ�ǵ�д��struct
							2. sizeof()���Ը�������,Ҳ����������
								����sizeof(sin)Ҳ����
			����ֵ
				0
					�ɹ�
				SOCKET_ERROR
					����һ����,����-1,�Ǵ���
					�������ͨ��WSAGetLastError()���
					���ʧ��,���ǵ�closesocket(); WSACleanUp();
				����֪ʶ:
					#define INVALID_SOCKET (SOCKET)(~0)
						��ΪSOCKET����Ҳ��һ����
							typedef UINT_PTR SOCKET;
						�����ٿ�UINT_PTR��ʲô
							typedef _w64 unsigned int UINT_PTR;
							����UINT_PTR��һ��unsigned int,��32bit
						����~0��0000 0000 0000 0000 ��ȡ��
							1111 1111 1111 1111
							���������з��ŵ�,��ô����-1
								�ǵ���,���Ǵ�0��2...7,Ȼ���ٴ�-2...8��-1
							��������ǿ��ΪSOCKET,���޷��ŵ�,���Դ��������ֵ4....5
								����Բ���: unsigned int a = ~0; ȥdebug�ᷢ��a�Ĵ�С����4...5
								�����int a = ~0; ȥdebug�ᷢ��a��ֵ��-1
					#define SOCKET_ERROR (-1)
5. ��ʼ����listen()
	����
		�����������WSAAPI()֮��,client�Ϳ���ͨ��server��socket������server
		����֮ǰ��Ȼ���ú���Э��,�˿�,��������������������,server�ͱ����������״̬
		��������
			��һ������,�Ͱ�������ӵ���Ϣ��һ��
			���Լ����������
	����
		ǰ���WSAAPI
			��һ������Լ��,���ǿ��Ժ���,��Ϊ�Ǹ�ϵͳ����
			����
				�����������ֵı��뷽ʽ
				������������ջ˳��
				��¼�����ĵ���ʱ��
		#1
			��Ϊ���������server.c,�����������ڴ���������,����������Ҫ���뵱ǰ��������socket
			Ҳ��������֮ǰ��socket()������SOCKET xxx;
		#2
			�������Ӷ��е���󳤶�(��Ϣ���ĳ���)
			��Դ:
				��ΪӲ��������,�������������ֻ�ܴ���20������,���Ǵ�����client������100������
				���Զ�������80������,���ǿ����������е�20���������ǵ���Ϣ��
					��Ϣ����������������:��������(�������Ӷ��е���󳤶�)
				�����60��,��������ʾ:���Ժ�����.
			����
				�Լ�����
					��Ҫ��:2-10��(�������Ǹ��˵���)
					��Ϊ���Ǵ���һ������,��ҪΪ������һ���ռ�,����������Ϣ(������������,���Ķ˿ں�..)
				ϵͳ����
					SOMAXCONN
						����һ����.����0x7fffffff
						ϵͳ�������ֵ�ܴ�,���Բ�����,����ϵͳѡ���ʺ��Լ��ĸ���
	����ֵ
		0
			�ɹ�
		SOCKET_ERROR
			����һ����,����-1,�Ǵ���
			�������ͨ��WSAGetLastError()���
			���ʧ��,���ǵ�closesocket(); WSACleanUp();
6. �����ͻ��˵�socket(accept())
	����
		�����ͻ��˵�socket,���ݿͻ��˵���Ϣ
		Ȼ�����clientSocket��Ϊ����ֵ����
			�������ֵ����Ҫ!!server����ͨ�����clientSocket����ֵ��client����
	***ע��accept��ȱ��(��Ҫ!!):
		1. ֻ�ܴ���һ������
			�м����ͻ�������,����Ҫ���ü������accept()
			�Ա�:
				listen()һ�ο��Լ����������,����ÿ������,ϵͳ���ᱣ��������Ϣ
				���Ƕ���accept(),ÿһ�����Ӷ���Ҫһ��accept()������������ӵ�socket
			��Ҫ�ż�
				����ֻ���������c/sģ��,����һ��client��һ��server
				�����ǲ�����,���client��
				��������accept()ֻ������һ��socket,û�й�ϵ
			����
				�������2��terminal,һ����client��,һ����server��
				���������ȥdebug�ļ���,��Ҫ��client.exe����,���ִ򲻿�
				Ϊʲô?��Ϊaccept()ֻ��һ��,��֧�ֶ��client
		2. ����/ͬ��
			���û�пͻ�������,��ô�ͻ�һֱ����
			�������Ҳ��ͬ��,һ��Ҫ�ȵ���,�������̲��ܼ���
			����
				accept()������������Ҫ���õĸոպ�!���Բ�����while��дaccept()
				���accept()��������client������,client.exe�򲻿�
				accept()��������,�ͻ�����,һֱ������һ��
		3. ���ܵĽ������
			1. ���߳�
				1. ��accept()�ŵ�һ���߳�,�������
				2. �շ���Ϣ,�ŵ���һ���߳�
				����ϲ�������,����΢���ϻ���������.
					�����߳̽�������Ǻܺ�
			2. ������ģ��
				�ڵײ�,��������⴦����
	����
		#1
			���Ǵ�����server��socket
		#2
			��һ���ṹ��struct sockaddr*
				����,����֮ǰ��bind()������Ҫ����const struct sockaddr*
					��Ϊ���������ú������ֵ,���Բ����޸�
					����ǿ������ת��Ҳ��(const struct sockaddr* xxx)
				��������,������Ҫ�����޸���,����û��const,cast��ʱ��Ҳ����const
			�����Ǵ���ַ,�����Ϊ�˺�������Ϣд��sockaddr*, Ȼ�󷵻ظ�����
		#3
			����2�Ĵ�С:sizeof(xxx)
			���Ժ���Ҳ��ѷ��صĲ���2,�Ѳ���2�Ĵ�С���ظ�����3.
		ע��
			���#2��#3Ҳ����ͬʱ����ΪNULL
				������
					�㲻��Ҫʹ��client��socket
				��������Ժ�������client��socket��ô��
					1. ��getpeername(newSocket, (struct sockaddr*) &sockClient, &len)
						����newSocketָ��������֮ǰnewSocket = accept(socketListen, NULL, NULL);
						Ȼ���������sockClient��len����ס��Ҫ��client��socket
					2. ��getsockname(xxSocket, (struct sockaddr*) &addr, &len)
						������������ڵõ��Լ�����Ϣ,������client�˵���,�Ϳ���֪��client��addr
						ע��,����˵��client�˵���,Ӧ����ָ��client.c��.
						����
							������server.c�е���,�������Ƿ�client��socket����server��,���صĶ���server��addr��len
								getsockname(sockServer, (struct sockaddr*) &addr, &len)
								getsockname(sockClient, (struct sockaddr*) &addr, &len)
								���Ϸ��صĶ���sockServer�Ķ˿�,���Ե�һ������û��ʲô��!
	����ֵ
		socket����
			�ɹ�
		INVALID_SOCKET
			ʧ��


7. ����ͨ��
	recv()��send()
		����
			����server,��Ҫ�õ�ָ��Ψһһ��client��������Ϣ
		����:�Ǹ���!����Ҫ
			0. ��client�������ݵĽ���,��ͨ��Э�鱾������,Ҳ����ϵͳ�ڵײ���Զ������ǽ���client����Ϣ,Ȼ�����ϵͳ��һ����Ϣ������.
				����recv()�����޸�ϵͳ����Ϣ������������,���ǿ��Ը��Ƶ�����,Ҳ���ǳ�����ڴ���.
			1. ֻҪclient������Ϣ,����recv()��û�б�����,ϵͳ�����Զ����Ӵ���,ȥ���������Ϣ
				recv()�޷��ǰ�ϵͳ�е���Ϣ����������Ϣ���Ƶ��˳�����ڴ���,Ҳ����char*��
				��Ϊ�����recv()������Ϣ,������ϵͳ������Ϣ,���п��ܵ������ǵ���recv()��ʱ������
				����recv()��������,��������
				����ϵͳ�������Զ����Ӻͽ�����Ϣ,����ʲôʱ����Ҫ,��ʲôʱ�����recv()
				recv()����ͨ��socket�ҵ���ϵͳ����һ�黺����
		ȱ��
			ֻ�ܽ���һ��client����Ϣ

		����
			#1 SOCKET��
				��Ҫ����client��socket
			#2 char* buffer(�ַ�����:char����)
				����client��Ϣ�Ŀռ�
				ע����Ϣ��һ���ֽ�һ���ֽڴ洢��
			#3 int len
				Ҫ��ȡ���ֽ���
				ͨ����char*�ĳ���-1,��Ϊ��β�п��ַ�\0
			#4 int flags
				���ݶ�ȡ�ķ�ʽ


	
		
					

4. ʹ����socket
	�����˾�һ��Ҫ����socket
		closesocket()
		server��client��socket��Ҫɾ��
	��֮��,��Ҫ�ر������WSACleanUp()
		ע����Ϊclosesocket�����������,������Ҫ�ڹر�֮ǰʹ��closesocket,����closesocket�͵��ò�����
	


*/