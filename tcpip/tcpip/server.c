#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")

int main(void)
{
	WORD wdVersion = MAKEWORD(2, 2);
	WSADATA wdSockMsg;
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

	if (2 != HIBYTE(wdSockMsg.wVersion) || 2 != LOBYTE(wdSockMsg.wVersion))
	{
		WSACleanup();
		return 0;
	}

	SOCKET socketServer = socket();

	system("pause");
	return 0;
}

/*
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





*/