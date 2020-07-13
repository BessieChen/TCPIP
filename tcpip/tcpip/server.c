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
1. 基础
WinSock2.h:
windows socket 套接字
通过打开文件,可以看到当前编译器支持的最新版本号,我的是2.2
不分大小写,例如winSock2.h也可以,但是还是最好要规范:WinSock2.h

#pragma comment(lib, "ws2_32.lib")
动态链接库
格式:
#pragma comment(lib, "xxx")
库:
ws: windows socket
2: 第二版 (第一版的是wsock32.lib)
32: 32位(注意,没有64位的,32位就已经可以用了)
静态库的后缀:lib
lib是cpp编译好后的二进制代码,我们是看不懂的
我们使用的时候,不需要编译好了,可以节省大量的编译时间
库的名字不分大小写,例如wS2_32.lib也可以
2. 打开网络库
1. int WSAStartUP
1. 功能:初始化winsock dll,启动网络库
w windows
s socket
a asynchronous 异步(在网络中,这个概念很重要)
同步:
一个进程需要等待另一个进程
异步:
多个进程同时进行
startup 启动
2. 参数
返回int
#1:
需要哪个版本的网络库, WORD类型
WORD是typedef unsigned short, 2个字节, 刚好一个字节分配给主版本,一个字节分配给子版本
WORD wdVersion = MAKEWORD(2, 1); //不能写成2.1,否则就是float了
makeword()是一个宏
#define MAKEWORD(a, b)   ((WORD)(((BYTE)(((DWORD_PTR)(a)) & 0xff)) | ((WORD)((BYTE)(((DWORD_PTR)(b)) & 0xff))) << 8))
左边的值,与上1111 1111
右边的值,与上1111 1111,然后左移8位
最后再或两个部分
我们这里就是想方法把2.2这个版本传递进去
#2:
低位字节是主版本,高位是子版本号
例如markword(2,1)之后的结果是258
0000 0001 0000 0010
所以高位 0000 0001就是子版本
所以低位 0000 0010就是主版本
一般计算机是小端,所以低地址存低位
假设左边是低地址,所以显示的是低位数字,也就是主版本
显示成16进制,左侧是低地址: 2 1
**老师有一个神奇的方法:找到内存中存放的东西:
int a = *((char*) &wdVersion);
这个的结果: wdVersion的第一个字节的值,也就是2
int b = *((char*) &wdVersion + 1);
这个的结果: wdVersion的第二个字节的值,也就是1
#3:
LPWSADATA
LPWSADATA 是 WSADATA*, 也就是LPWSADATA是WSADATA*的重命名
LP:指的是指针,也就是要地址. 要wsadata的地址
我们需要创建一个结构体叫做wsadata,然后把它的地址传给这个参数
创建:
老师用的是栈空间
WSADATA wdSockMsg;
WSAStartUp(wdVersion, &wdSockMsg);
如果你要用堆的话,需要malloc,之后还需要释放,太麻烦
LPWSADATA lpw = malloc(sizeof(WSADATA)); //也就是WSADATA* lpw =  malloc(sizeof(WSADATA));
WSAStartUp(wdVersion, lpw);
free(xxx);
之后这个函数会把一些信息,传递给我们创建的结构体.
这个叫做传址应用
3. 返回值int
0
成功
其他值:
返回错误码中的一个
举例
WSAEPROCLIM
已经达到了win 套接字实现所支持的任务数量的上限
也就是不能无限开网络库
可能是硬件配置不够,例如变量和结构体的初始化
理论是打开65536次,这是因为端口的大小范围
每次打开资源,都要开一个端口,端口是一个unsigned short
16bit, 2^16 = 65536
所以你不能打开7万个端口
4. 成功之后
我们可以查看函数为我们的结构体(WSADATA wdSockMsg)传递了什么信息:
1. wVersion
我们需要使用的版本
2. wHighVersion
系统能提供的最高版本
3. iMaxSockets(少用)
返回可用的socket的数量
4. iMaxUdpDg(少用)
udp数据报信息的大小
5. ...
5. 假如失败
1. 如果我们要1.3
可是没有1.3,只有1.2和2.0
那么就打开主版本1支持的版本,也就是1.2
2. 如果输入3.0
这是一个很大的版本号,那就返回我们系统最大的2.2版本
3. 输入0.0
返回错误码10092: 网络库打开失败,不支持请求的socket版本
2. 校验版本
if (2 != HIBYTE(wdSockMsg.wVersion) || 2 != LOBYTE(wdSockMsg.wVersion))
进入的话,说明版本不对, hibyte()是指子版本,lobyte是主版本
这两个宏,还是挺重要的
清理网络库: WSACleanUp();

2. 创建socket
定义
将底层复杂的协议体系,执行流程,封装后,api就是socket
socket是我们调用协议进行通信的操作接口
意义
我们不需要知道底层协议的具体细节,方便了使用
网络编程就难在协议的复杂性,但是简单就是因为有socket
本质
是一种数据类型
typedef UINT_PTR  SOCKET;
所以SOCKET是一个无符号的int
相当于一张表
一个程序创建了一个socket,假设是23,这个23就是唯一的
23对应的是:创建的协议(通过参数指定不同的协议,例如tcp协议,例如数据是流型的,指定ip地址,端口号)
以后通信的时候,直接用23通信,系统会找到23的协议,就是用之前指定的方式来通信
其他程序创建的socket,值就是其他的了
例如值是24,然后指定的是udp协议
应用
网络通信,都需要socket
例如给client发信息,需要知道它的socket
给另一个client发信息,也需要知道这个client的socket
每一个client都有socket,每一个server服务器都有socket
参数
1. 地址的类型
例如如果用微信联系老师,就是使用xxx微信号,微信号就是地址的类型
举例
AD_INET
ipv4
4字节,32bit,无符号整型 0 - 4294967295
从0.0.0.0 - 255.255.255.255
但是不够用了
AF_INET6
ipv6
16字节,128bits,2^128 = 3*10^38
包括了地球的每一个ip
以后会越来越普遍
AF_BTH
蓝牙地址
AF_IRDA
红外线
注意
硬件也要支持,例如你使用af_irda,你就需要有红外线端口

2. socket类型
我们是用什么协议
举例
SOCK_STREAM
tcp
可靠的:信息是一点都不能丢的
例如qq聊天,传真
顺序的:
按顺序的到达
SOCK_DGRAM
udp
不可靠的:信息可以丢
电影会卡一下,有的地方花屏了
例如数据传输收到了干扰
例如在光缆上走的时候需要了干扰
不是顺序的:
例如看电影,看缓冲条,是间断的缓冲的
SOCK_RAM
SOCK_RDM
SOCK_SEQPACKET

3. 协议类型
如果填0,系统会给我们默认选一个合适的值.你也可以自己填具体的值
举例
IPPROTO_TCP
IPPROTO_UDP
IPPROTO_ICMP
IPPROTO_IGMP
IPPROTO_RM





*/