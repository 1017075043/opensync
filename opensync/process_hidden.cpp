#include "process_hidden.h"

/*####################### process_hidden #######################*/
namespace opensync
{
	process_hidden* process_hidden::instance = nullptr;
	process_hidden::process_hidden()
	{
		run_flag = true;
		pid = getpid();
		create_fanotify(fanotify_fd);
	}
	process_hidden::~process_hidden()
	{
	}
	process_hidden* process_hidden::get_instance()
	{
		return instance;
	}
	process_hidden* process_hidden::init_instance()
	{
		if (instance == nullptr) {
			instance = new process_hidden();
		}
		return instance;
	}
	void process_hidden::destory()
	{
		if (instance != nullptr)
		{
			delete instance;
			instance = nullptr;
		}
	}

	bool process_hidden::create_fanotify(int& fd) //创建监听inotify实例
	{
		//初始化，第一个参数为FAN_CLASS_CONTENT（表示允许接收通知文件事件）另一个值为FAN_CLASS_NOTIF 为默认值。只允许接收事件通知，在一般的监听中，使用FAN_CLASS_CONTENT
		//第二个参数，表示接收到操作文件事件后，本程序对文件的操作为可读可写，因为metadata中的fd为程序操作发生事件的文件的fd，可以直接操作那个文件，操作完后，要关掉。
		try
		{
			fd = fanotify_init(FAN_CLASS_CONTENT, O_RDWR);
			if (fd < 0) {
				throw exception() << err_str("cteate fanotify fail, fd:" + to_string(fd) + ", mesg=" + strerror(errno));
			}
			else
			{
				out->logs << OUTINFO << "cteate fanotify success, fd:" << fd;
			}
		}
		catch (exception& e)
		{
			out->logs << OUTERROR << *boost::get_error_info<err_str>(e);
			return false;
		}
		return true;
	}
	bool process_hidden::add_watch_unit(const string& file_path) //向inotify中添加一个监控（只监控目录）
	{
		try
		{
			int ret = fanotify_mark(fanotify_fd, FAN_MARK_ADD, FAN_OPEN_PERM | FAN_ACCESS_PERM | FAN_EVENT_ON_CHILD | FAN_ONDIR, AT_FDCWD, file_path.c_str());
			//int ret = fanotify_mark(fanotify_fd, FAN_MARK_ADD, FAN_ALL_PERM_EVENTS | FAN_EVENT_ON_CHILD, AT_FDCWD, file_path.c_str());
			if (ret < 0)
			{
				throw exception() << err_str(file_path + " add fanotify fail, mesg=" + strerror(errno));
			}
			else
			{
				out->logs << OUTINFO << file_path << " add fanotify success, ret:" << ret;
			}
		}
		catch (exception& e)
		{
			out->logs << OUTERROR << *boost::get_error_info<err_str>(e);
			return false;
		}
		return true;
	}
	bool process_hidden::open_fanotify() //开始监听
	{
		struct epoll_event epev;  //记录套接字相关信息
		epev.events = EPOLLIN;    //监视有数据可读事件
		epev.data.fd = fanotify_fd;//文件描述符数据，其实这里可以放任何数据。
		int epollfd = epoll_create(1024); //创建 epoll 文件描述符，出错返回-1
		try
		{
			string error_str;
			//加入监听列表，当 listenfd 上有对应事件产生时， epoll_wait 会将 epoll_event 填充到 events_in 数组里, 出错返回 -1
			if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fanotify_fd, &epev) == -1)
			{
				throw exception() << err_str(error_str + "fanotify_fd add epoll listen fail, mesg=" + strerror(errno));
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
					throw exception() << err_str(error_str + "epoll_wait get event fail, mesg=" + strerror(errno));
				}
				fanotify_handle_event(event_count);
			}
		}
		catch (exception& e)
		{
			out->logs << OUTERROR << *boost::get_error_info<err_str>(e);
			return false;
		}
	}
	bool process_hidden::fanotify_handle_event(const int& event_count) //处理事件
	{
		for (int i = 0; i < event_count; i++)      //遍历所有事件
		{
			try
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
					throw exception() << err_str(error_str + "from fanotify_fd read event fail, mesg=" + strerror(errno));
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
						throw exception() << err_str(error_str + "fanotify元数据版本不匹配, mesg=" + strerror(errno));
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
							throw exception() << err_str(error_str + "readlink 失败, mesg=" + strerror(errno));
						}
						path[path_len] = '\0';
						//printf("File %s\n", path);
						close(metadata->fd); //关闭事件的文件描述符
						fanotify_decision_event(metadata, path);
					}
					metadata = FAN_EVENT_NEXT(metadata, len); //获取下一个事件
				}
			}
			catch (exception& e)
			{
				out->logs << OUTERROR << *boost::get_error_info<err_str>(e);
				return false;
			}
		}
		return true;
	}
	bool process_hidden::fanotify_decision_event(const struct fanotify_event_metadata* metadata, const string& path) //决策事件
	{
		string proc_name = sys_ac.get_proc_name(metadata->pid);
		//处理matadata
		if (metadata->mask & FAN_ALL_PERM_EVENTS)
		{
			//out->logs << OUTINFO << path << ", 给内核发送消息";
			struct fanotify_response response_struct;
			response_struct.fd = metadata->fd;
			if (pid == metadata->pid)
			{
				response_struct.response = FAN_ALLOW;
				out->logs << OUTINFO << "允许" << proc_name << "访问" << path;
			}
			else
			{
				response_struct.response = FAN_DENY;
				out->logs << OUTINFO << "拒绝" << proc_name << "访问" << path;
			}

			int ret = write(fanotify_fd, &response_struct, sizeof(response_struct));
			if (ret < 0)
			{
				return false;
			}
		}
		return true;
	}
	bool process_hidden::start_process_hidden() //开始隐藏指定进程
	{
		char file_path_temp[32] = { 0 };
		snprintf(file_path_temp, sizeof(file_path_temp), "/proc/%d", pid);
		string file_path = file_path_temp;
		try
		{
			if (file_op.get_file_status(file_path) == false) //如果文件不存在
			{
				throw exception() << err_str(file_path + " is not exists, mesg=" + strerror(errno));
			}
			if (add_watch_unit(file_path))
			{
				boost::thread process_hidden_thread = boost::thread(boost::bind(&process_hidden::open_fanotify, this)); //创建fanotify线程
				process_hidden_thread.join();
			}
		}
		catch (exception& e)
		{
			out->logs << OUTERROR << *boost::get_error_info<err_str>(e);
			return false;
		}
	}
	bool process_hidden::stop_process_hidden() //停止隐藏指定进程
	{
		run_flag = false;
	}
}