#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <unistd.h>

#include <sched.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>
#include <sys/fanotify.h>
#include <sys/epoll.h>
#include <list>

#include <sys/inotify.h>
#include <boost/bimap.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

#include "opensync_public_include_file.h"
#include "file_system_operation.h"
#include "system_action.h"

/*
	进程隐藏(单例)
	@author 吴南辉
	@time 2020/06/27
*/

namespace opensync
{
	class process_hidden : boost::noncopyable
	{
	private:
		static process_hidden* instance;
		opensync::log4cpp_instance* out = opensync::log4cpp_instance::init_instance();
		opensync::file_system_operation* file_op = opensync::file_system_operation::init_instance();
		system_action sys_ac;

		int fanotify_fd;
		bool run_flag;
		pid_t pid;

	private:
		//process_hidden
		process_hidden();
		~process_hidden();

	private:
		bool create_fanotify(int& fd); //创建监听fanotify实例
		bool add_watch_unit(const string& file_path);//向inotify中添加一个监控（只监控目录）
		bool open_fanotify(); //开始fanotify监听
		bool fanotify_handle_event(const int& event_count); //处理事件
		bool fanotify_decision_event(const struct fanotify_event_metadata* metadata, const string& path); //决策事件
		
	public:
		//process_hidden
		static process_hidden* get_instance();
		static process_hidden* init_instance();
		static void destory();

		bool start_process_hidden(); //开始隐藏进程
		bool stop_process_hidden(); //停止隐藏进程
	};
}

