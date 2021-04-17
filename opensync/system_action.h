#pragma once
#include "opensync_public_include_file.h"

#include <stdio.h> 
#include <unistd.h>
#include <string.h>
#include <map>

#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>

#include <boost/noncopyable.hpp>

#define VMRSS_LINE 17
#define VMSIZE_LINE 13
#define PROCESS_ITEM 14

using namespace std;

/*
	linux系统操作类（单例）
	@author 吴南辉
	@time 2020/05/30
*/

namespace opensync
{
	class system_action : boost::noncopyable
	{
	private:
		opensync::log4cpp_instance* out = opensync::log4cpp_instance::init_instance();

	private:
		typedef struct {
			unsigned long user;
			unsigned long nice;
			unsigned long system;
			unsigned long idle;
		} Total_Cpu_Occupy_t;
		typedef struct {
			unsigned int pid;
			unsigned long utime;  //user time
			unsigned long stime;  //kernel time
			unsigned long cutime; //all user time
			unsigned long cstime; //all dead time
		} Proc_Cpu_Occupy_t;

	private:
		const char* get_items(const char* buffer, unsigned int item); //获取第N项开始的指针
		unsigned long get_cpu_total_occupy();  //获取总的CPU时间

	public:
		system_action();
		~system_action();
		string get_proc_name(const pid_t& pid);

		unsigned long get_cpu_proc_occupy(unsigned int pid);  //获取进程的CPU时间
		float get_proc_cpu(unsigned int pid);  //获取CPU占用率
		unsigned int get_proc_mem(unsigned int pid); //获取进程占用内存
		unsigned int get_proc_virtualmem(unsigned int pid);  //获取进程占用虚拟内存
	};
}

