using namespace std;

#include <stdlib.h>  
#include <stdio.h> 
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <errno.h>
#include <sstream>

#include <sched.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>
#include <sys/fanotify.h>
#include <sys/epoll.h>
#include <list>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#include <iostream>
#include <stdio.h>
#include <string.h>

int process_check();
int process_record();
int init_daemon(char* path);
bool create_fanotify(int& fd);
bool add_watch_unit(const string& file_path);
bool open_fanotify();
bool fanotify_handle_event(const int& event_count);
bool fanotify_decision_event(const struct fanotify_event_metadata* metadata, const string& path);
bool start_process_hidden(int argc, char* argv[]);
bool stop_process_hidden();
string get_proc_name(const pid_t& pid);

int fanotify_fd;
bool run_flag;
pid_t pid;

#define process_hidden_pid "/tmp/xxyc.pid"

int main(int argc, char* argv[])
{
	if (argc == 1)
	{
		cout << "./xxyc pid pid ..." << endl;
		cout << "./xxyc stop" << endl;
		exit(1);
	}
	else if (argc == 2)
	{
		string argv_temp = argv[1];
		if (argv_temp == "stop")
		{
			process_check();
			cout << "stop success" << endl;
			exit(1);
		}
	}
	else
	{
		process_check();
	}

	init_daemon("/tmp");
	pid = getpid();
	cout << pid << endl;

	process_record();

	fanotify_fd = 0;
	run_flag = true;
	pid = getpid();
	cout << pid << endl;

	create_fanotify(fanotify_fd);
	start_process_hidden(argc, argv);
	open_fanotify();
}

int process_check()
{
	int fd;
	if ((fd = open(process_hidden_pid, O_CREAT | O_RDONLY, 0600)) < 0) {
		perror("open");
		exit(1);
	}
	char pid_temp[20];
	char file_path_temp[200];
	int n = read(fd, pid_temp, 20);
	pid_t childpid = atoi(pid_temp);
	cout << childpid << endl;
	snprintf(file_path_temp, sizeof(file_path_temp), "/proc/%d", childpid);
	if (access(file_path_temp, F_OK) != 0) //如果文件不存在
	{
		cout << file_path_temp << " is not exists, mesg=" << strerror(errno) << endl;
	}
	else
	{
		int retval = kill(childpid, SIGKILL);
		if (retval)
		{
			perror("kill");
			exit(1);
		}
	}
	close(fd);
}

int process_record()
{
	int fd;
	if ((fd = open(process_hidden_pid, O_CREAT | O_WRONLY | O_TRUNC, 0600)) < 0) {
		perror("open");
		exit(1);
	}
	stringstream pid_ss;
	string pid_str;
	pid_ss << getpid();
	pid_ss >> pid_str;
	write(fd, pid_str.c_str(), pid_str.length());//写入文件
	close(fd);
}

int init_daemon(char* path)
{
	int pid;
	if ((pid = fork()) > 0)
	{
		exit(0); /* 是父进程，结束父进程 */
	}
	else if (pid < 0)
	{
		return -1; /* fork失败 */
	}

	/* 是第一子进程，继续 */
	/* 如果该进程是一个进程组的组长，此函数返回错误。
	为了保证这一点，我们先调用fork()然后exit()，此时只有子进程在运行，
	子进程继承了父进程的进程组ID，但是进程PID却是新分配的，所以不可能
	是新会话的进程组的PID。从而保证了这一点。 */
	setsid();
	/* 创建一个新的会话 */
	/* 第一子进程成为新的会话组长和进程组长 */
	/* 脱离控制终端，登录会话和进程组 */

	/* 现在，第一子进程已经成为无终端的会话组长。但它可以重新申请打开一个
	控制终端。    可以通过使进程不再成为会话组长来禁止进程重新打开控制终端 */
	if ((pid = fork()) > 0)
	{
		exit(0); /* 是第一子进程，结束第一子进程 */
	}
	else if (pid < 0)
	{
		return -1; /* fork失败 */
	}

	/* 是第二子进程，继续 */
	/* 第二子进程不再是会话组长 */

	/* 改变工作目录到/tmp */
	/* 最好是切换到根目录，因为如果它的当前目录是在一个被安装的  文件系统上,那么就会妨碍这个文件系统
	被卸载，非必须,防止mount出错 */
	if (chdir(path ? path : "/tmp") != 0)
		return -2;

	/* parasoft suppress item BD-RES-INVFREE-1 */
	/* 关闭标准输入输出，标准错误的文件描述符 */
	close(STDIN_FILENO); /* parasoft-suppress BD-RES-INVFREE-1 */
	close(STDOUT_FILENO); /* parasoft-suppress BD-RES-INVFREE-1 */
	close(STDERR_FILENO); /* parasoft-suppress BD-RES-INVFREE-1 */
	/* parasoft unsuppress item BD-RES-INVFREE-1 */

	umask(0);
	/* 进程从创建它的父进程那里继承了文件创建掩模。它可能修改守护进程所创建的文件
	的存取位，umask调用把守护进程的umask设置为0，这样取消了父进程的umask，避免了潜在的
	干扰创建文件和目录 */

	return 0;
}

bool create_fanotify(int& fd) //创建监听inotify实例
{
	//初始化，第一个参数为FAN_CLASS_CONTENT（表示允许接收通知文件事件）另一个值为FAN_CLASS_NOTIF 为默认值。只允许接收事件通知，在一般的监听中，使用FAN_CLASS_CONTENT
	//第二个参数，表示接收到操作文件事件后，本程序对文件的操作为可读可写，因为metadata中的fd为程序操作发生事件的文件的fd，可以直接操作那个文件，操作完后，要关掉。
	fd = fanotify_init(FAN_CLASS_CONTENT, O_RDWR);
	if (fd < 0)
	{
		cout << "cteate fanotify fail, fd:" << fd << ", mesg=" << strerror(errno) << endl;
	}
	else
	{
		cout << "cteate fanotify success, fd:" << fd << endl;
	}
	return true;
}
bool add_watch_unit(const string& file_path) //向inotify中添加一个监控（只监控目录）
{
	int ret = fanotify_mark(fanotify_fd, FAN_MARK_ADD, FAN_OPEN_PERM | FAN_ACCESS_PERM | FAN_EVENT_ON_CHILD | FAN_ONDIR, AT_FDCWD, file_path.c_str());
	if (ret < 0)
	{
		cout << file_path << " add fanotify fail, mesg=" << strerror(errno) << endl;
	}
	else
	{
		cout << file_path << " add fanotify success, ret:" << ret << endl;
	}
	return true;
}
bool open_fanotify() //开始监听
{
	struct epoll_event epev;  //记录套接字相关信息
	epev.events = EPOLLIN;    //监视有数据可读事件
	epev.data.fd = fanotify_fd;//文件描述符数据，其实这里可以放任何数据。
	int epollfd = epoll_create(1024); //创建 epoll 文件描述符，出错返回-1
	//加入监听列表，当 listenfd 上有对应事件产生时， epoll_wait 会将 epoll_event 填充到 events_in 数组里, 出错返回 -1
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fanotify_fd, &epev) == -1)
	{
		cout << "fanotify_fd add epoll listen fail, mesg=" << strerror(errno) << endl;
	}
	struct epoll_event events_in[16];
	struct inotify_event* event;
	while (run_flag)
	{
		//等待事件， epoll_wait 会将事件填充至 events_in 内
		//返回 获得的事件数量，若超时且没有任何事件返回 0 ，出错返回 -1 。 timeout设置为-1表示无限等待。毫秒级，1秒等于1000毫秒
		int event_count = epoll_wait(epollfd, events_in, sizeof(events_in) - 1, 10);
		if (event_count == -1)
		{
			cout << "epoll_wait get event fail, mesg=" << strerror(errno) << endl;
		}
		fanotify_handle_event(event_count);
	}
}
bool fanotify_handle_event(const int& event_count) //处理事件
{
	for (int i = 0; i < event_count; i++)      //遍历所有事件
	{
		string error_str;
		//out->logs << OUTDEBUG << "event_count:" << event_count;
		//循环，而事件可以从fanotify文件描述符读取
		const struct fanotify_event_metadata* metadata;
		struct fanotify_event_metadata buf[4096];
		ssize_t len;
		char path[PATH_MAX];
		ssize_t path_len;
		char procfd_path[PATH_MAX];
		struct fanotify_response response;

		len = read(fanotify_fd, (void*)&buf, sizeof(buf)); //读一些事件
		if (len == -1 && errno != EAGAIN)
		{
			cout << "from fanotify_fd read event fail, mesg=" << strerror(errno) << endl;
		}
		if (len <= 0) //检查是否已到达可用数据
		{
			break;
		}
		metadata = buf; //指向缓冲区中的第一个事件

		while (FAN_EVENT_OK(metadata, len))  //循环缓冲区中的所有事件
		{
			if (metadata->vers != FANOTIFY_METADATA_VERSION)  //检查运行时结构和编译时结构是否匹配
			{
				cout << "fanotify metadata version does not match, mesg=" << strerror(errno) << endl;
			}
			if (metadata->fd >= 0)  //metadata->fd包含FAN_NOFD(指示队列溢出)或文件描述符(非负整数)。这里，我们简单地忽略队列溢出。
			{
				snprintf(procfd_path, sizeof(procfd_path), "/proc/self/fd/%d", metadata->fd); //检索和打印被访问文件的路径名
				//readlink()会将参数path的符号链接内容存储到参数buf所指的内存空间，返回的内容不是以\000作字符串结尾，
				//但会将字符串的字符数返回，这使得添加\000变得简单。若参数bufsiz小于符号连接的内容长度，过长的内容会被截断，
				//如果 readlink 第一个参数指向一个文件而不是符号链接时，readlink 设 置errno 为 EINVAL 并返回 -1。 
				//readlink()函数组合了open()、read()和close()的所有操作。
				path_len = readlink(procfd_path, path, sizeof(path) - 1);
				if (path_len == -1)
				{
					cout << "readlink fail, mesg=" << strerror(errno) << endl;
				}
				path[path_len] = '\0';
				//printf("File %s\n", path);
				close(metadata->fd); //关闭事件的文件描述符
				fanotify_decision_event(metadata, path);
			}
			metadata = FAN_EVENT_NEXT(metadata, len); //获取下一个事件
		}
	}
	return true;
}
bool fanotify_decision_event(const struct fanotify_event_metadata* metadata, const string& path) //决策事件
{
	string proc_name = get_proc_name(metadata->pid);
	//处理matadata
	if (metadata->mask & FAN_ALL_PERM_EVENTS)
	{
		//out->logs << OUTINFO << path << ", 给内核发送消息";
		struct fanotify_response response_struct;
		response_struct.fd = metadata->fd;
		if (pid == metadata->pid)
		{
			response_struct.response = FAN_ALLOW;
			cout << "允许" << proc_name << "访问" << path << endl;
		}
		else
		{
			response_struct.response = FAN_DENY;
			cout << "拒绝" << proc_name << "访问" << path << endl;
		}

		int ret = write(fanotify_fd, &response_struct, sizeof(response_struct));
		if (ret < 0)
		{
			return false;
		}
	}
	return true;
}
bool start_process_hidden(int argc, char* argv[]) //开始隐藏指定进程
{
	char file_path_temp[32] = { 0 };
	snprintf(file_path_temp, sizeof(file_path_temp), "/proc/%d", pid);
	string file_path = file_path_temp;
	add_watch_unit(file_path_temp);
	for (int i = 1; i < argc; i++)
	{
		char file_path_temp[32] = { 0 };
		snprintf(file_path_temp, sizeof(file_path_temp), "/proc/%d", atoi(argv[i]));
		string file_path = file_path_temp;
		if (access(file_path_temp, F_OK) != 0) //如果文件不存在
		{
			cout << file_path_temp << " is not exists, mesg=" << strerror(errno) << endl;
		}
		else
		{
			add_watch_unit(file_path);
			cout << argv[i] << " process hidden success" << endl;
		}
	}
}
bool stop_process_hidden() //停止隐藏指定进程
{
	run_flag = false;
}

string get_proc_name(const pid_t& pid)
{
	static pid_t static_pid = -1;
	static string static_proc_name = "";
	if (static_pid != pid)
	{
		char* p = NULL;
		int count = 0;
		char chPath[4096] = { 0 };
		char cParam[4096] = { 0 };

		sprintf(cParam, "/proc/%d/exe", pid);
		count = readlink(cParam, chPath, 4096);
		if (count < 0 || count >= 4096)
		{
			cout << pid << ", Current System not Surport Proc, mesg=" << strerror(errno) << endl;
		}
		else
		{
			p = basename(chPath);
			if (p == NULL)
			{
				cout << pid << ", is not exists, mesg=" << strerror(errno) << endl;
			}
		}
		static_pid = pid;
		static_proc_name = chPath;
	}
	return static_proc_name;
}