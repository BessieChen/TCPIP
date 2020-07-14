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

	bres = listen(socketServer, SOMAXCONN);
	if (SOCKET_ERROR == bres) {
		int err = WSAGetLastError();
		closesocket(socketServer);
		WSACleanup();
		return 0;
	}

	//创建客户端的socket
	struct sockaddr_in clientsin;
	int len = sizeof(clientsin);
	SOCKET socketClient = accept(socketServer, (struct sockaddr*)&clientsin, &len);
	if (INVALID_SOCKET == socketClient) {
		printf("客户端连接失败.");
		int err = WSAGetLastError();
		closesocket(socketServer);
		WSACleanup();
		return 0;
	}
	printf("客户端连接成功.");

	//用完socket关闭
	closesocket(socketClient);
	closesocket(socketServer);
	WSACleanup();

	system("pause");
	return 0;
}

/*
0. 其他
	如果不小心按到了insert键,就会变成只能删除文字
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
	返回值
		如果失败了,就需要wsacleanup()将网络库清理
		是什么错误:
			int res = WSAGetLastError(); //一定是获得最近的error
4. 绑定上地址和端口
	地址
		每一个电脑,都有一个地址
		例如使用ipv4,那么地址就是ip地址
		类比:	
			地址的类型,qq号
			那么对应的地址就是例如10348712
	端口
		每个软件,都有一个端口号
			所以信息是传递到a软件,不是传递到b软件
		例如qq有一个端口号,浏览器有一个端口号
			注意:
				同一个软件可能有多个端口号
					qq传递消息:用tcp对应的端口号
					qq看视频:用udp对应的端口号
	实现
		int bind()
			参数1
				绑定地址和端口号
			参数2
				传入sockaddr*
					所以我们要构造一个sockaddr的结构体
					可是这个更好填一些:
						struct sockaddr_in{
							short sin_family;
							u_short sin_prot;
							struct in_addr sin_addr;
							char sin_zero[8];
							}
						疑问来了,需要的是sockaddr,但是我们如果创建sockaddr_in传入,可以吗
							可以,因为
								sockaddr是ushort 和 char xx[14]
									所以一共是2+14=16个字节
									因为内存对齐,是按照最大的整数倍,所以内存就是占16字节
								sockaddr_in
									因为short是2字节,u_short是2字节,struct因为是32位ip地址所以是4字节
									然后char xx[8]也是8字节
									所以也是一共16字节
							既然内存一样大,所以我们强制转换就ok了
					所以我们最后是创建一个sockaddr_in
						注意:
							两种对的书写方式:
								struct sockaddr_in xx;
								SOCKADDR_IN xxx;
							因为c语言中,就是这样的. 但是cpp可以不需要struct,可以直接写sockaddr_in xxx;
							然后你看他的定义
								typedef struct sockaddr_in{...} SOCKADDR_IN
								就说明SOCKADDR_IN 是 struct sockaddr_in 的重定义
						参数
							xx.sin_family = AF_INET;
								一定要对应socket(AF_INET,xxx),因为我们是要绑定
							xx.sin_port = htons(27015);
								使用宏定义htons(),将我们输入的数字转化成端口号
								**端口号
									本质:
										unsigned short: 16bits,2字节
											就是一个整数 0 - 65535
									我们填写的值
										1. 理论上0-65535都可以
										2. 实际中,我们写的是1024-65535,最好写大一点的值
											1. 系统会使用0-1023的端口号
												21端口号:分配给FTP(文件传输协议)服务
												25:分配给SMTP(简单邮件传输协议)服务
												80:分配给HTTP服务
									因为端口号是唯一的,如果我们设置一个已经占用的端口号,就会绑定失败
										如何判断端口号是否被使用?
											打开command,输入
												netstat -ano
													查看所有使用的端口,会有非常多
													例如, 本地地址下面写的 162.32.42.53.50134
														其中最后一个50134就是端口号
												netstat -ano|findstr "12345"
													查询某一个
													如果被占用:会显示该程序
													如果没有被占用,就什么都不显示
							xx.sin_addr
								这个也是一个结构体struct in_addr sin_addr;
									这句话是:成员sin_addr的类型是struct in_addr
									就像是我们之前说的typedef struct in_addr{}IN_ADDR;
								它里面是一个union,里面包含两个struct
									union:空间是成员中,最大的那个,然后大家都共用空间
									首先让我们看一下是什么样子的:
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
									解释:
										相当于我们可以从Union S_un中三选一:
											1. 选第一个struct S_un_b;
												一共4个u_char, 每个1字节
												一共4字节
												用法: 193.23.43.53
												举例:
													xx.sin_addr.S_un.S_un_b.s_b1 = 199;
													xx.sin_addr.S_un.S_un_b.s_b2 = 99;
													xx.sin_addr.S_un.S_un_b.s_b3 = 99;
													xx.sin_addr.S_un.S_un_b.s_b4 = 19;
												非常麻烦,显示union S_un,再是第一个struct,再是里面元素
											2. 选第二个struct S_un_w;
												一共2个u_short,每个占2字节
												一共4字节
											3, 选第三个 u_long S_addr;
												在windows系统中,无论是32位还是64位,u_long都是4字节
												但是linux的64位系统,u_long是8字节
												举例:
													xx.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
													inet_addr()也是一个宏,就是转换成一个u_long
									地址设置
										使用127.0.0.1
											叫做本地回送地址
											用于本地网络测试,不会把我们的数据发送出去
												例如我用自己的电脑,写server和client,就是两个终端交流
											用于:我自己没有网络,或者没有两部共用wifi的电脑
										使用192.168.xx.xx
											如何看到自己的ip地址:
												可以在控制台输入指令ipconfig 就可以了
											举例
												一个寝室,共用一个局域网,用别人的pc当server我的当client
									更新
										InetPton(AF_INET, TEXT("127.0.0.1"), &sin.sin_addr.s_addr);
													


									
			参数3
				是sockaddr的大小
					bind(socketServer, (const struct sockaddr*)&sin, sizeof(struct sockaddr_in));
						注意:
							1. 首先强制类型转换中,因为sockaddr是struct,你要记得写上struct
							2. sizeof()可以跟变量名,也可以是类型
								所以sizeof(sin)也可以
			返回值
				0
					成功
				SOCKET_ERROR
					这是一个宏,代表-1,是错误
					具体错误通过WSAGetLastError()获得
					如果失败,最后记得closesocket(); WSACleanUp();
				补充知识:
					#define INVALID_SOCKET (SOCKET)(~0)
						因为SOCKET类型也是一个宏
							typedef UINT_PTR SOCKET;
						我们再看UINT_PTR是什么
							typedef _w64 unsigned int UINT_PTR;
							所以UINT_PTR是一个unsigned int,是32bit
						所以~0是0000 0000 0000 0000 的取反
							1111 1111 1111 1111
							如果这个是有符号的,那么代表-1
								记得吗,我们从0到2...7,然后再从-2...8到-1
							可是我们强制为SOCKET,是无符号的,所以代表的最大的值4....5
								你可以测试: unsigned int a = ~0; 去debug会发现a的大小就是4...5
								如果是int a = ~0; 去debug会发现a的值是-1
					#define SOCKET_ERROR (-1)
5. 开始监听listen()
	作用
		调用这个函数WSAAPI()之后,client就可以通过server的socket来链接server
		就是之前虽然设置好了协议,端口,但是现在是终于启动了,server就变成了侦听的状态
		监听过程
			来一个连接,就把这个连接的信息存一次
			可以监听多个连接
	参数
		前面的WSAAPI
			是一个调用约定,我们可以忽略,因为是给系统看的
			作用
				决定函数名字的编译方式
				决定参数的入栈顺序
				记录函数的调用时间
		#1
			因为我们这个是server.c,所以我们是在创建服务器,所以我们需要传入当前服务器的socket
			也就是我们之前用socket()创建的SOCKET xxx;
		#2
			挂起链接队列的最大长度(休息区的长度)
			来源:
				因为硬件的限制,服务器可能最多只能处理20个请求,但是从所有client处来了100个请求
				所以对于其中80个请求,我们可以邀请其中的20个进入我们的休息区
					休息区允许的最大连接数:就是我们(挂起链接队列的最大长度)
				其余的60个,对它们显示:请稍后再试.
			设置
				自己设置
					不要大:2-10个(对于我们个人电脑)
					因为我们处理一个请求,都要为它设置一个空间,保留它的信息(例如它的请求,它的端口号..)
				系统设置
					SOMAXCONN
						这是一个宏.代表0x7fffffff
						系统发现这个值很大,所以不会用,所以系统选择适合自己的个数
	返回值
		0
			成功
		SOCKET_ERROR
			这是一个宏,代表-1,是错误
			具体错误通过WSAGetLastError()获得
			如果失败,最后记得closesocket(); WSACleanUp();
6. 创建客户端的socket(accept())
	作用
		创建客户端的socket,根据客户端的信息
		然后将这个clientSocket作为返回值返回
			这个返回值很重要!!server就是通过这个clientSocket返回值和client交互
	***注意accept的缺点(重要!!):
		1. 只能创建一个连接
			有几个客户端连接,就需要调用几次这个accept()
			对比:
				listen()一次可以监听多个连接,对于每个连接,系统都会保存它的信息
				但是对于accept(),每一个连接都需要一个accept()来生成这个连接的socket
			不要着急
				这里只是最基本的c/s模型,就是一个client和一个server
				而不是并发的,多个client的
				所以这里accept()只能生成一个socket,没有关系
			体现
				如果打开了2个terminal,一个是client的,一个是server的
				如果你现在去debug文件夹,想要打开client.exe程序,发现打不开
				为什么?因为accept()只有一个,不支持多个client
		2. 阻塞/同步
			如果没有客户端请求,那么就会一直卡着
			所以这个也叫同步,一定要等到你,其他进程才能继续
			所以
				accept()函数的数量需要设置的刚刚好!所以不能用while来写accept()
				如果accept()数量少于client的数量,client.exe打不开
				accept()数量过多,就会阻塞,一直卡在这一句
		3. 可能的解决方法
			1. 用线程
				1. 把accept()放到一个线程,处理接收
				2. 收发信息,放到另一个线程
				宏观上不会阻塞,但是微观上还是阻塞了.
					所以线程解决并不是很好
			2. 用网络模型
				在底层,把这个问题处理了
	参数
		#1
			我们创建的server的socket
		#2
			是一个结构体struct sockaddr*
				回忆,我们之前的bind()里面需要的是const struct sockaddr*
					因为我们是设置好了这个值,所以不能修改
					所以强制类型转换也是(const struct sockaddr* xxx)
				但是这里,我们需要函数修改它,所以没有const,cast的时候也不用const
			这里是传地址,这个是为了函数把信息写入sockaddr*, 然后返回给我们
		#3
			参数2的大小:sizeof(xxx)
			所以函数也会把返回的参数2,把参数2的大小返回给参数3.
		注意
			你的#2和#3也可以同时设置为NULL
				适用于
					你不需要使用client的socket
				但是如果以后你想用client的socket怎么办
					1. 用getpeername(newSocket, (struct sockaddr*) &sockClient, &len)
						其中newSocket指的是我们之前newSocket = accept(socketListen, NULL, NULL);
						然后你可以用sockClient和len来接住你要的client的socket
					2. 用getsockname(xxSocket, (struct sockaddr*) &addr, &len)
						这个函数是用于得到自己的信息,例如在client端调用,就可以知道client的addr
						注意,这里说在client端调用,应该是指在client.c中.
						举例
							假如在server.c中调用,无论你是放client的socket还是server的,返回的都是server的addr和len
								getsockname(sockServer, (struct sockaddr*) &addr, &len)
								getsockname(sockClient, (struct sockaddr*) &addr, &len)
								以上返回的都是sockServer的端口,所以第一个参数没有什么用!
	返回值
		socket类型
			成功
		INVALID_SOCKET
			失败


7. 进行通信
	recv()和send()
		作用
			我是server,我要得到指定唯一一个client发来的消息
		本质:是复制!很重要
			0. 从client来的数据的接受,是通过协议本身做的,也就是系统在底层会自动帮我们接受client的消息,然后存在系统的一个消息缓存区.
				我们recv()不能修改系统的消息缓存区的内容,但是可以复制到本地,也就是程序的内存中.
			1. 只要client发来消息,不管recv()有没有被调用,系统都会自动监视窗口,去接受这个消息
				recv()无非是把系统中的消息缓存区的消息复制到了程序的内存中,也就是char*中
				因为如果是recv()接受消息,而不是系统接受消息,很有可能导致我们调用recv()的时机不对
				例如recv()调用早了,或者晚了
				所以系统帮我们自动监视和接受消息,我们什么时候需要,就什么时候调用recv()
				recv()可以通过socket找到是系统的哪一块缓冲区
		缺点
			只能接受一个client的消息

		参数
			#1 SOCKET类
				我要听的client的socket
			#2 char* buffer(字符数组:char数组)
				储存client消息的空间
				注意消息是一个字节一个字节存储的
			#3 int len
				要读取的字节数
				通常是char*的长度-1,因为结尾有空字符\0
			#4 int flags
				数据读取的方式


	
		
					

4. 使用完socket
	不用了就一定要销毁socket
		closesocket()
		server和client的socket都要删除
	在之后,需要关闭网络库WSACleanUp()
		注意因为closesocket就在网络库中,所以你要在关闭之前使用closesocket,否则closesocket就调用不了了
	


*/