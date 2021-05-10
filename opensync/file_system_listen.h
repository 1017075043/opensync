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
#include <boost/date_time/posix_time/posix_time.hpp>

#include "opensync_public_include_file.h"
#include "file_system_operation.h"
#include "system_action.h"

/*
	文件系统监控类(单例)
	@author 吴南辉
	@time 2020/06/27
*/

namespace opensync
{
	class file_system_listen : boost::noncopyable
	{
	private:
		static file_system_listen* instance;
		opensync::log4cpp_instance* out = opensync::log4cpp_instance::init_instance();
		opensync::file_system_operation* file_op = opensync::file_system_operation::init_instance();
		system_action sys_ac;

		bool run_flag;
		int fanotify_fd; //保存fanotify_init返回的文件描述符
		int inotify_fd;  //保存inotify_init返回的文件描述符
		typedef boost::bimap<int, string> bimap;
		bimap inotify_list;					//监控列表
		list<string> watch_list;			//监听文件列表
		list<string> ignore_watch_list;		//忽略监听文件列表
		map<string, uint32_t> event_list;	//事件列表
		map<string, uint32_t> action_list;	//动作列表
		map<string, uint32_t> decision_list;//决策列表

	private:
		//file_system_listen
		file_system_listen();
		~file_system_listen();
		bool create_inotify(int& fd); //创建监听inotify实例
		bool create_fanotify(int& fd); //创建监听fanotify实例
		bool add_watch_unit(const string& file_path);//向inotify中添加一个监控（只监控目录）
		bool add_watch_dir(const string& dir_path);//向inotify中添加一个目录监控,会监控目录下（包括子目录）的所有动作（只监控目录）
		bool del_watch_unit(const int& wd, const string& file_path);//向inotify中去除一个监控
		bool is_ignore_watch_unit(const string& file_path); //判断是否为忽略监听的单元

		bool open_inotify(); //开始inotify监控
		bool open_inotify_stake(); //开始进行监控(桩，只监听，不读取)
		void inotify_handle_event(const int& wd, const string& name, const uint32_t& mask); //处理一个事件
		inline const string inotify_transform_event_mask(const uint32_t& mask); //事件转换
		inline const string inotify_transform_event_mask_chinese(const uint32_t& mask); //事件转换(中文)
		inline void inotify_add_event_into_event_list(const string& file_path, const uint32_t& mask); //向事件列表中添加或者更新事件

		bool open_fanotify(); //开始fanotify监听
		bool fanotify_handle_event(const int& event_count); //处理事件
		bool fanotify_decision_event(const struct fanotify_event_metadata* metadata, const string& path); //决策事件

		//file_system_listen_list_operation
		bool is_exists_inotifyed_file(const string& file_path); //判断一个文件是否处于监听（只监控目录）
		string get_inotifyed_file(const int& wd); //根据wd从监控列表中获取文件名
		int get_inotifyed_file(const string& file_path); //根据文件路径从监控列表中获取wd

		//file_system_listen_fifo 设置监控函数 多线程启动，CPU亲和度配置和线程优先级
		void inotify_thread_set(const int& thread_num); //设置CPU亲和度配置和线程优先级
		const int get_inotify_thread_id(const int& thread_num); //获取线程执行的CPU号

	public:
		//file_system_listen
		static file_system_listen* get_instance();
		static file_system_listen* init_instance();
		static void destory();

	public:
		//file_system_listen_list_operation
		void add_watch(const string& file_path); //添加监听
		void add_ignore_watch(const string& file_path); //添加忽略监听

		//file_system_listen_fifo
		void start_watch(); //开始监听
		void shop_watch();//停止监听
	};
}

