#include "file_system_listen.h"

/*####################### file_system_listen #######################*/
namespace opensync
{
	file_system_listen* file_system_listen::instance = nullptr;
	file_system_listen::file_system_listen()
	{
		run_flag = true;
		create_fanotify(fanotify_fd);
		create_inotify(inotify_fd);
	}
	file_system_listen::~file_system_listen()
	{
	}
	file_system_listen* file_system_listen::get_instance()
	{
		return instance;
	}
	file_system_listen* file_system_listen::init_instance()
	{
		if (instance == nullptr) {
			instance = new file_system_listen();
		}
		return instance;
	}
	void file_system_listen::destory()
	{
		if (instance != nullptr)
		{
			delete instance;
			instance = nullptr;
		}
	}
	inline const string file_system_listen::inotify_transform_event_mask(const uint32_t& mask) //事件转换
	{
		string event_list;
		if (mask & IN_ACCESS) event_list = event_list + "IN_ACCESS|"; /* 文件被访问.  */
		if (mask & IN_MODIFY) event_list = event_list + "IN_MODIFY|"; /* 文件被修改.  */
		if (mask & IN_ATTRIB) event_list = event_list + "IN_ATTRIB|"; /* 元数据发生了变化.  */
		if (mask & IN_CLOSE_WRITE) event_list = event_list + "IN_CLOSE_WRITE|"; /* 可写入文件关闭.  */
		if (mask & IN_CLOSE_NOWRITE) event_list = event_list + "IN_CLOSE_NOWRITE|"; /* 不可写文件关闭.  */
		if (mask & IN_CLOSE_NOWRITE && mask & IN_CLOSE_NOWRITE) event_list = event_list + "IN_CLOSE|"; /* 文件已关闭.  */
		if (mask & IN_OPEN) event_list = event_list + "IN_OPEN|"; /* 文件被打开.  */
		if (mask & IN_MOVED_FROM) event_list = event_list + "IN_MOVED_FROM|"; /* 文件从X移动.  */
		if (mask & IN_MOVED_TO) event_list = event_list + "IN_MOVED_TO|"; /* 文件被移动到Y.  */
		if (mask & IN_MOVED_FROM && mask & IN_MOVED_TO) event_list = event_list + "IN_MOVE|"; /* 文件被移动.  */
		if (mask & IN_CREATE) event_list = event_list + "IN_CREATE|"; /* 子文件创建.  */
		if (mask & IN_DELETE) event_list = event_list + "IN_DELETE|"; /* 子文件被删除.  */
		if (mask & IN_DELETE_SELF) event_list = event_list + "IN_DELETE_SELF|"; /* 自我被删除.  */
		if (mask & IN_MOVE_SELF) event_list = event_list + "IN_MOVE_SELF|"; /* 自我被移动.  */
		if (mask & IN_UNMOUNT) event_list = event_list + "IN_UNMOUNT|"; /* 已卸载备份fs.  */
		if (mask & IN_Q_OVERFLOW) event_list = event_list + "IN_Q_OVERFLOW|"; /* 事件队列溢出.  */
		if (mask & IN_IGNORED) event_list = event_list + "IN_IGNORED|"; /* 文件被忽视.  */
		if (mask & IN_ONLYDIR) event_list = event_list + "IN_ONLYDIR|"; /* 只有在路径是目录的情况下才查看路径.  */
		if (mask & IN_DONT_FOLLOW) event_list = event_list + "IN_DONT_FOLLOW|"; /* 不遵循一个sym链接.  */
		if (mask & IN_EXCL_UNLINK) event_list = event_list + "IN_EXCL_UNLINK|"; /* 排除未链接对象上的事件sym链接.  */
		if (mask & IN_MASK_ADD) event_list = event_list + "IN_MASK_ADD|"; /* 添加到一个已经存在的监视的掩码.  */
		if (mask & IN_ISDIR) event_list = event_list + "IN_ISDIR|"; /* 针对dir发生的事件.  */
		if (mask & IN_ONESHOT) event_list = event_list + "IN_ONESHOT|"; /* 只发送事件一次.  */
		return event_list;
	}
	inline const string file_system_listen::inotify_transform_event_mask_chinese(const uint32_t& mask) //事件转换(中文)
	{
		string event_list;
		if (mask & IN_ACCESS) event_list = event_list + "文件被访问|"; /* 文件被访问.  */
		if (mask & IN_MODIFY) event_list = event_list + "文件被修改|"; /* 文件被修改.  */
		if (mask & IN_ATTRIB) event_list = event_list + "元数据发生了变化|"; /* 元数据发生了变化.  */
		if (mask & IN_CLOSE_WRITE) event_list = event_list + "可写入文件关闭|"; /* 可写入文件关闭.  */
		if (mask & IN_CLOSE_NOWRITE) event_list = event_list + "不可写文件关闭|"; /* 不可写文件关闭.  */
		if (mask & IN_CLOSE_NOWRITE && mask & IN_CLOSE_NOWRITE) event_list = event_list + "文件已关闭|"; /* 文件已关闭.  */
		if (mask & IN_OPEN) event_list = event_list + "文件被打开|"; /* 文件被打开.  */
		if (mask & IN_MOVED_FROM) event_list = event_list + "文件从X移动|"; /* 文件从X移动.  */
		if (mask & IN_MOVED_TO) event_list = event_list + "文件被移动到Y|"; /* 文件被移动到Y.  */
		if (mask & IN_MOVED_FROM && mask & IN_MOVED_TO) event_list = event_list + "文件被移动|"; /* 文件被移动.  */
		if (mask & IN_CREATE) event_list = event_list + "子文件创建|"; /* 子文件创建.  */
		if (mask & IN_DELETE) event_list = event_list + "子文件被删除|"; /* 子文件被删除.  */
		if (mask & IN_DELETE_SELF) event_list = event_list + "自我被删除|"; /* 自我被删除.  */
		if (mask & IN_MOVE_SELF) event_list = event_list + "自我被移动|"; /* 自我被移动.  */
		if (mask & IN_UNMOUNT) event_list = event_list + "已卸载备份fs|"; /* 已卸载备份fs.  */
		if (mask & IN_Q_OVERFLOW) event_list = event_list + "事件队列溢出|"; /* 事件队列溢出.  */
		if (mask & IN_IGNORED) event_list = event_list + "文件被忽视|"; /* 文件被忽视.  */
		if (mask & IN_ONLYDIR) event_list = event_list + "只有在路径是目录的情况下才查看路径|"; /* 只有在路径是目录的情况下才查看路径.  */
		if (mask & IN_DONT_FOLLOW) event_list = event_list + "不遵循一个sym链接|"; /* 不遵循一个sym链接.  */
		if (mask & IN_EXCL_UNLINK) event_list = event_list + "排除未链接对象上的事件sym链接|"; /* 排除未链接对象上的事件sym链接.  */
		if (mask & IN_MASK_ADD) event_list = event_list + "添加到一个已经存在的监视的掩码|"; /* 添加到一个已经存在的监视的掩码.  */
		if (mask & IN_ISDIR) event_list = event_list + "针对dir发生的事件|"; /* 针对dir发生的事件.  */
		if (mask & IN_ONESHOT) event_list = event_list + "只发送事件一次|"; /* 只发送事件一次.  */
		return event_list;
	}

	bool file_system_listen::create_inotify(int& fd)  //创建监听inotify实例
	{
		try
		{
			fd = inotify_init();
			if (fd < 0)
			{
				throw exception() << err_str("cteate inotify fail, fd:" + to_string(fd) + ", mesg=" + strerror(errno));
			}
			else
			{
				out->logs << OUTINFO << "cteate inotify success, fd:" << fd;
			}
		}
		catch (exception& e)
		{
			out->logs << OUTERROR << *boost::get_error_info<err_str>(e);
			return false;
		}
		return true;
	}
	bool file_system_listen::create_fanotify(int& fd) //创建监听inotify实例
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

	bool file_system_listen::add_watch_unit(const string& file_path) //向inotify中添加一个监控（只监控目录）
	{
		if (is_ignore_watch_unit(file_path)) //忽略监听
		{
			return true;
		}
		if (file_op->get_file_status(file_path) == false) //如果文件不存在
		{
			return false;
		}
		string dir_path = file_path;
		if (file_op->get_file_type(dir_path) != 3) //如果文件不是目录
		{
			dir_path = file_op->get_file_path(dir_path).parent_path().string();
			out->logs << OUTDEBUG << file_path << " is file, get it parent_path=" << dir_path;
		}
		if (is_exists_inotifyed_file(file_path)) //是否已经处于监听状态
		{
			out->logs << OUTINFO << file_path << " is inotifyed status";
			return true;
		}
		try
		{
			//int ret = fanotify_mark(fanotify_fd, FAN_MARK_ADD, FAN_ALL_PERM_EVENTS | FAN_EVENT_ON_CHILD, AT_FDCWD, file_path.c_str());
			int ret = fanotify_mark(fanotify_fd, FAN_MARK_ADD, FAN_OPEN_PERM | FAN_ACCESS_PERM | FAN_EVENT_ON_CHILD | FAN_ONDIR, AT_FDCWD, file_path.c_str());
			if (ret < 0)
			{
				throw exception() << err_str(file_path + " add fanotify fail, mesg=" + strerror(errno));
			}
			else
			{
				out->logs << OUTINFO << file_path << " add fanotify success, ret:" << ret;
			}
			int wd = inotify_add_watch(inotify_fd, file_path.c_str(), IN_ALL_EVENTS); //添加监控
			if (wd < 0)
			{
				throw exception() << err_str(file_path + " add inotifyed fail, mesg=" + strerror(errno));
			}
			else
			{
				inotify_list.left.insert(make_pair(wd, file_path)); //加入监控列表
				out->logs << OUTINFO << file_path << " add inotifyed success, wd=" << wd;
			}
		}
		catch (exception& e)
		{
			out->logs << OUTERROR << *boost::get_error_info<err_str>(e);
			return false;
		}
		return true;
	}
	bool file_system_listen::add_watch_dir(const string& dir_path)//向inotify中添加一个目录监控,会监控目录下（包括子目录）的所有动作（只监控目录）
	{
		vector<const opensync::file_attribute*> file_attr_list;
		file_attr_list = file_op->get_file_and_dir_traverse_dir_list(dir_path);
		BOOST_FOREACH(const opensync::file_attribute * file_attr, file_attr_list)
		{
			if (add_watch_unit(file_attr->file_path.string()) == false)
			{
				return false;
			}
		}
		return true;
	}
	bool file_system_listen::del_watch_unit(const int& wd, const string& file_path) //向inotify中去除一个监控
	{
	}
	bool file_system_listen::is_ignore_watch_unit(const string& file_path) //判断是否为忽略监听的单元
	{
		BOOST_FOREACH(string ignore_file_path, ignore_watch_list) //检查是否忽略监听
		{
			if (ignore_file_path == file_path.substr(0, ignore_file_path.length()))
			{
				out->logs << OUTDEBUG << "ignore watch: " << file_path;
				return true;
			}
		}
		return false;
	}

	bool file_system_listen::open_inotify() //开始进行监控
	{
		if (watch_list.empty())
		{
			out->logs << OUTWARN << "watch list is empty";
			return true;
		}
		BOOST_FOREACH(string file_path, watch_list) //添加要监控的文件
		{
			out->logs << OUTINFO << file_path << ", 开始添加监视";
			add_watch_dir(file_path);
		}
		struct epoll_event epev;  //记录套接字相关信息
		epev.events = EPOLLIN;    //监视有数据可读事件
		epev.data.fd = inotify_fd;//文件描述符数据，其实这里可以放任何数据。
		int epollfd = epoll_create(1024); //创建 epoll 文件描述符，出错返回-1
		try
		{
			string error_str;
			//加入监听列表，当 listenfd 上有对应事件产生时， epoll_wait 会将 epoll_event 填充到 events_in 数组里, 出错返回 -1
			if (epoll_ctl(epollfd, EPOLL_CTL_ADD, inotify_fd, &epev) == -1)
			{
				throw exception() << err_str(error_str + "inotify_fd add epoll listen fail, mesg=" + strerror(errno));
			}
			int run_cpu_core = get_inotify_thread_id(0);
			struct epoll_event events_in[16];
			struct inotify_event* event;
			while (run_flag)
			{
				int event_count = epoll_wait(epollfd, events_in, sizeof(events_in) - 1, 10);
				//out->logs << OUTDEBUG << "event_count:" << event_count;
				//等待事件， epoll_wait 会将事件填充至 events_in 内
				//返回 获得的事件数量，若超时且没有任何事件返回 0 ，出错返回 -1 。 timeout设置为-1表示无限等待。毫秒级，1秒等于1000毫秒
				if (event_count == -1)
				{
					throw exception() << err_str(error_str + "epoll_wait get event fail, mesg=" + strerror(errno));
				}
				for (int i = 0; i < event_count; i++)      //遍历所有事件
				{
					//out->logs << OUTDEBUG << "run_cpu_core=" << run_cpu_core << ", epoll listen event count: " << event_count;
					char buf[1024];
					int len = read(inotify_fd, buf, sizeof(buf) - 1); //从缓冲池中取出事件
					int nread = 0;
					while (len > 0)
					{
						event = (struct inotify_event*) & buf[nread];
						if (event->len > 0)
						{
							inotify_handle_event(event->wd, event->name, event->mask);
						}
						nread = nread + sizeof(struct inotify_event) + event->len;
						len = len - sizeof(struct inotify_event) - event->len;
					}
				}
			}
		}
		catch (exception& e)
		{
			out->logs << OUTERROR << *boost::get_error_info<err_str>(e);
			return false;
		}
		return true;
	}
	bool file_system_listen::open_inotify_stake() //开始进行监控(桩，只监听，不读取)
	{
		if (watch_list.empty())
		{
			out->logs << OUTWARN << "watch list is empty";
			return true;
		}
		struct epoll_event epev;  //记录套接字相关信息
		epev.events = EPOLLIN;    //监视有数据可读事件
		epev.data.fd = inotify_fd;//文件描述符数据，其实这里可以放任何数据。
		int epollfd = epoll_create(1024); //创建 epoll 文件描述符，出错返回-1
		try
		{
			//加入监听列表，当 listenfd 上有对应事件产生时， epoll_wait 会将 epoll_event 填充到 events_in 数组里, 出错返回 -1
			if (epoll_ctl(epollfd, EPOLL_CTL_ADD, inotify_fd, &epev) == -1)
			{
				throw exception() << err_str("inotify_fd add epoll listen fail");
			}
		}
		catch (exception& e)
		{
			out->logs << OUTERROR << *boost::get_error_info<err_str>(e);
			return false;
		}

		int run_cpu_core = get_inotify_thread_id(0);
		struct epoll_event events_in[16];
		while (run_flag)
		{
			int event_count = epoll_wait(epollfd, events_in, sizeof(events_in) - 1, 10);
			//if (event_count)
			//{
			//	out->logs << OUTDEBUG << "run_cpu_core=" << run_cpu_core << ", epoll listen event count: " << event_count;
			//}
			//out->logs << OUTDEBUG << "event_count:" << event_count;
			//等待事件， epoll_wait 会将事件填充至 events_in 内
			//返回 获得的事件数量，若超时且没有任何事件返回 0 ，出错返回 -1 。 timeout设置为-1表示无限等待。毫秒级，1秒等于1000毫秒
		}
		return true;
	}
	void file_system_listen::inotify_handle_event(const int& wd, const string& name, const uint32_t& mask) //处理一个事件
	{
		string parent_name = get_inotifyed_file(wd);
		if (parent_name.empty() == false)
		{
			string file_path = parent_name + "/" + name;
			inotify_add_event_into_event_list(file_path, mask);

			//file_op->show_file_info(parent_name + "/" + name);
			if ((mask & IN_ISDIR) && ((mask & IN_CREATE) || (mask & IN_MOVED_TO)) && is_exists_inotifyed_file(file_path) == false) //如果新增加或者移入了一个目录，将其添加到监控列表
			{
				add_watch_unit(file_path);
			}
			else if ((mask & IN_ISDIR) && ((mask & IN_DELETE) || (mask & IN_MOVED_FROM))) //如果删除或者移去了一个目录，将其从监控列表中删除
			{
				int file_wd = get_inotifyed_file(file_path);
				if (file_wd != -1)
				{
					inotify_rm_watch(inotify_fd, file_wd); //调用inotify_rm_watch移除监听
					inotify_list.left.erase(file_wd);
					out->logs << OUTINFO << file_path << " del inotifyed success, wd=" << file_wd;
				}
			}
		}
	}
	inline void file_system_listen::inotify_add_event_into_event_list(const string& file_path, const uint32_t& mask) //向事件列表中添加或者更新事件
	{
		event_list[file_path] = event_list[file_path] | mask;
		if (mask & IN_MODIFY || mask & IN_MOVE || mask & IN_CREATE || mask & IN_DELETE || mask & IN_DELETE_SELF || mask & IN_MOVE_SELF)
		{
			action_list[file_path] = mask;
			out->logs << OUTINFO << "event->name=" << file_path << ", event->mask=" << inotify_transform_event_mask(mask) << inotify_transform_event_mask_chinese(mask);
		}
		else if (mask & IN_ATTRIB)
		{
			action_list[file_path] = action_list[file_path] | mask;
			out->logs << OUTINFO << "event->name=" << file_path << ", event->mask=" << inotify_transform_event_mask(action_list[file_path]) << inotify_transform_event_mask_chinese(action_list[file_path]);
		}
		else
		{
			out->logs << OUTINFO << "event->name=" << file_path << ", event->mask=" << inotify_transform_event_mask(event_list[file_path]) << inotify_transform_event_mask_chinese(event_list[file_path]);
		}
	}

	bool file_system_listen::open_fanotify() //开始监听
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
				for (int i = 0; i < event_count; i++)
				{
					fanotify_handle_event(event_count);
				}
			}
		}
		catch (exception& e)
		{
			out->logs << OUTERROR << *boost::get_error_info<err_str>(e);
			return false;
		}
	}
	bool file_system_listen::fanotify_handle_event(const int& event_count) //处理事件
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
	bool file_system_listen::fanotify_decision_event(const struct fanotify_event_metadata* metadata, const string& path) //决策事件
	{
		static boost::posix_time::ptime event_time_temp = boost::posix_time::second_clock::local_time();
		static pid_t pid_temp = 0;
		static string path_temp;

		string proc_name = sys_ac.get_proc_name(metadata->pid);
		//处理matadata
		if (metadata->mask & FAN_ALL_PERM_EVENTS)
		{
			if (pid_temp != metadata->pid || path_temp != path || event_time_temp != boost::posix_time::second_clock::local_time()) //去重
			{
				pid_temp = metadata->pid;
				path_temp = path;
				event_time_temp = boost::posix_time::second_clock::local_time();
				out->logs << OUTINFO << path << ", 给内核发送消息, proc_name: " << proc_name << ", metadata: " << metadata;
			}
			struct fanotify_response response_struct;
			response_struct.fd = metadata->fd;
			//if (proc_name == "/usr/bin/bash"
			if (0)
			{
				response_struct.response = FAN_ALLOW;
			}
			else
			{
				response_struct.response = FAN_DENY;
			}
			for (int a = 0; a < 3; ++a)
			{
				out->logs << OUTINFO << "等待fanotify进行决策，" << response_struct.response;
				sleep(1);
			}
			int ret = write(fanotify_fd, &response_struct, sizeof(response_struct));
			if (ret < 0)
			{
				return false;
			}
		}
		return true;
	}

}

/*####################### file_system_listen_fifo #######################*/
namespace opensync
{
	void file_system_listen::inotify_thread_set(const int& thread_num)  //获取线程执行的CPU号
	{
		try
		{
			//设置线程与CPU亲和力
			cpu_set_t mask;  //CPU核的集合
			//cpu_set_t get;   //获取在集合中的CPU
			CPU_ZERO(&mask);    //置空
			CPU_SET(thread_num, &mask);   //设置亲和力值
			if (sched_setaffinity(0, sizeof(mask), &mask) == -1)//设置线程CPU亲和力
			{
				throw exception() << err_str("thread_num=" + to_string(thread_num) + ", could not set CPU affinity, " + strerror(errno));
			}
			//get_inotify_thread_id(thread_num);

			//CPU_ZERO(&get);
			//if (sched_getaffinity(0, sizeof(get), &get) == -1)//获取线程CPU亲和力
			//{
			//	throw exception() << err_str("thread=" + to_string(thread_num) + ", cound not get thread affinity, " + strerror(errno));
			//}
			//if (!CPU_ISSET(thread_num, &get))//判断线程与哪个CPU有亲和力
			//{
			//	throw exception() << err_str("thread=" + to_string(thread_num) + ", this thread " + to_string(thread_num) + " is not running processor :" + to_string(thread_num) + ", " + strerror(errno));
			//}
			//out->logs << OUTINFO << "thread=" << thread_num << ", this thread" << thread_num << " is running processor:" << thread_num;

			//设置线程优先级为99
			struct sched_param param;
			int sched_method = 1;
			int sched_priority = 99;
			param.sched_priority = sched_priority;
			if (sched_setscheduler(0, sched_method, &param))
			{
				throw exception() << err_str("thread_num=" + to_string(thread_num) + ", set scheduler to 1 99 failed, " + strerror(errno));
			}
			int scheduler = sched_getscheduler(0);
			out->logs << OUTDEBUG << "thread_num=" << thread_num << ", the scheduler of " << thread_num << " is " << scheduler << ", priority " << sched_priority;
			if (thread_num != 0)
			{
				boost::this_thread::sleep(boost::posix_time::seconds(3));
				out->logs << OUTINFO << "thread_num=" << thread_num << ", inotify thread start.";
				open_inotify_stake();
			}
			else
			{
				out->logs << OUTINFO << "thread_num=" << thread_num << ", inotify thread start.";
				open_inotify();
			}
			out->logs << OUTINFO << "thread_num=" << thread_num << ", inotify thread stop.";
		}
		catch (exception& e)
		{
			out->logs << OUTERROR << *boost::get_error_info<err_str>(e);
		}
	}
	const int file_system_listen::get_inotify_thread_id(const int& thread_num) //获取线程执行的CPU号
	{
		try
		{
			int cpu_num = sysconf(_SC_NPROCESSORS_CONF);  //获取CPU核数
			cpu_set_t get;   //获取在集合中的CPU
			CPU_ZERO(&get);
			if (sched_getaffinity(0, sizeof(get), &get) == -1)//获取线程CPU亲和力
			{
				throw exception() << err_str("thread_num=" + to_string(thread_num) + ", cound not get thread affinity, " + strerror(errno));
			}
			for (int i = 0; i < cpu_num; ++i)
			{
				if (CPU_ISSET(i, &get))//判断线程与哪个CPU有亲和力
				{
					out->logs << OUTDEBUG << "thread_num=" << thread_num << ", is running processor:" << i;
					return i;
				}
			}
			throw exception() << err_str("thread_num=" + to_string(thread_num) + ", is not running processor :" + strerror(errno));
		}
		catch (exception& e)
		{
			out->logs << OUTERROR << *boost::get_error_info<err_str>(e);
		}
		return -1;
	}
	void file_system_listen::start_watch() //创建和CPU相同的线程数，
	{
		boost::thread fanotify_thread = boost::thread(boost::bind(&file_system_listen::open_fanotify, this)); //创建fanotify线程

		int cpu_num = sysconf(_SC_NPROCESSORS_CONF);  //获取CPU核数
		boost::thread inotify_thread[cpu_num];
		out->logs << OUTINFO << "system has " << cpu_num << " processor(s).";
		for (int i = 0; i < cpu_num; i++)
		{
			out->logs << OUTINFO << "i=" << i << ", 开始尝试创建线程.";
			inotify_thread[i] = boost::thread(boost::bind(&file_system_listen::inotify_thread_set, this, i)); //创建inotify线程
		}
		fanotify_thread.join();
		for (int i = 0; i < cpu_num; ++i)
		{
			inotify_thread[i].join();
		}
		return;
	}
	void file_system_listen::shop_watch() //停止监听
	{
		run_flag = false;
	}

}

/*####################### file_system_listen_list_operation #######################*/
namespace opensync
{
	bool file_system_listen::is_exists_inotifyed_file(const string& file_path) //判断一个文件是否处于监听
	{
		bimap::right_const_iterator it;
		it = inotify_list.right.find(file_path);
		if (it != inotify_list.right.end())
		{
			return true;
		}
		return false;
	}
	string file_system_listen::get_inotifyed_file(const int& wd) //根据wd从监控列表中获取文件名
	{
		bimap::left_const_iterator it;
		it = inotify_list.left.find(wd);
		if (it != inotify_list.left.end())
		{
			return it->second;
		}
		return "";
	}
	int file_system_listen::get_inotifyed_file(const string& file_path) //根据文件路径从监控列表中获取wd
	{
		bimap::right_const_iterator it;
		it = inotify_list.right.find(file_path);
		if (it != inotify_list.right.end())
		{
			return it->second;
		}
		return -1;
	}
	void file_system_listen::add_watch(const string& file_path) //添加监听
	{
		watch_list.push_back(file_path);
	}
	void file_system_listen::add_ignore_watch(const string& file_path) //添加忽略监听
	{
		ignore_watch_list.push_back(file_path);
	}
}