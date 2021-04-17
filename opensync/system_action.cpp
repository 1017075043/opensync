#include "system_action.h"

/*####################### system_action #######################*/
namespace opensync
{
	system_action::system_action()
	{
	}
	system_action::~system_action()
	{
	}
	string system_action::get_proc_name(const pid_t& pid)
	{
		static pid_t static_pid = -1;
		static string static_proc_name = "";
		try
		{
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
					throw exception() << err_str(to_string(pid) + ", Current System not Surport Proc, mesg=" + strerror(errno));
				}
				else
				{
					p = basename(chPath);
					if (p == NULL)
					{
						throw exception() << err_str(to_string(pid) + ", is not exists, mesg=" + strerror(errno));
					}
				}
				static_pid = pid;
				//static_proc_name = basename(chPath);
				static_proc_name = chPath;
			}
		}
		catch (exception& e)
		{
			out->logs << OUTERROR << *boost::get_error_info<err_str>(e);
			return "";
		}
		//out->logs << OUTDEBUG << "pid: " << pid << "的进程名为: " << static_proc_name;
		return static_proc_name;
	}

	const char* system_action::get_items(const char* buffer, unsigned int item) //获取第N项开始的指针
	{
		const char* p = buffer;
		int len = strlen(buffer);
		int count = 0;
		for (int i = 0; i < len; i++) {
			if (' ' == *p) {
				count++;
				if (count == item - 1) {
					p++;
					break;
				}
			}
			p++;
		}
		return p;
	}
	unsigned long system_action::get_cpu_total_occupy()  //获取总的CPU时间
	{
		FILE* fd;
		char buff[1024] = { 0 };
		Total_Cpu_Occupy_t t;

		fd = fopen("/proc/stat", "r");
		if (nullptr == fd) {
			return 0;
		}

		fgets(buff, sizeof(buff), fd);
		char name[64] = { 0 };
		sscanf(buff, "%s %ld %ld %ld %ld", name, &t.user, &t.nice, &t.system, &t.idle);
		fclose(fd);

		return (t.user + t.nice + t.system + t.idle);
	}
	unsigned long system_action::get_cpu_proc_occupy(unsigned int pid)  //获取进程的CPU时间
	{
		char file_name[64] = { 0 };
		Proc_Cpu_Occupy_t t;
		FILE* fd;
		char line_buff[1024] = { 0 };
		sprintf(file_name, "/proc/%d/stat", pid);

		fd = fopen(file_name, "r");
		if (nullptr == fd) {
			return 0;
		}

		fgets(line_buff, sizeof(line_buff), fd);

		sscanf(line_buff, "%u", &t.pid);
		const char* q = get_items(line_buff, PROCESS_ITEM);
		sscanf(q, "%ld %ld %ld %ld", &t.utime, &t.stime, &t.cutime, &t.cstime);
		fclose(fd);

		return (t.utime + t.stime + t.cutime + t.cstime);
	}
	float system_action::get_proc_cpu(unsigned int pid)  //获取CPU占用率
	{
		unsigned long totalcputime1, totalcputime2;
		unsigned long procputime1, procputime2;

		totalcputime1 = get_cpu_total_occupy();
		procputime1 = get_cpu_proc_occupy(pid);

		usleep(200000);

		totalcputime2 = get_cpu_total_occupy();
		procputime2 = get_cpu_proc_occupy(pid);

		float pcpu = 0.0;
		if (0 != totalcputime2 - totalcputime1) {
			pcpu = 100.0 * (procputime2 - procputime1) / (totalcputime2 - totalcputime1);
		}

		return pcpu;
	}
	unsigned int system_action::get_proc_mem(unsigned int pid) //获取进程占用内存
	{
		char file_name[64] = { 0 };
		FILE* fd;
		char line_buff[512] = { 0 };
		sprintf(file_name, "/proc/%d/status", pid);

		fd = fopen(file_name, "r");
		if (nullptr == fd) {
			return 0;
		}

		char name[64];
		int vmrss;
		for (int i = 0; i < VMRSS_LINE - 1; i++) {
			fgets(line_buff, sizeof(line_buff), fd);
		}

		fgets(line_buff, sizeof(line_buff), fd);
		sscanf(line_buff, "%s %d", name, &vmrss);
		fclose(fd);

		return vmrss;
	}
	unsigned int system_action::get_proc_virtualmem(unsigned int pid)  //获取进程占用虚拟内存
	{
		char file_name[64] = { 0 };
		FILE* fd;
		char line_buff[512] = { 0 };
		sprintf(file_name, "/proc/%d/status", pid);

		fd = fopen(file_name, "r");
		if (nullptr == fd) {
			return 0;
		}

		char name[64];
		int vmsize;
		for (int i = 0; i < VMSIZE_LINE - 1; i++) {
			fgets(line_buff, sizeof(line_buff), fd);
		}

		fgets(line_buff, sizeof(line_buff), fd);
		sscanf(line_buff, "%s %d", name, &vmsize);
		fclose(fd);

		return vmsize;
	}

}