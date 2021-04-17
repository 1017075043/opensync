#include "opensync_public_include_file.h"

#include <map>
#include <boost/version.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

#include "instance_garbo.h"
#include "file_system_operation.h"
#include "file_system_listen.h"
#include "process_hidden.h"
#include "system_action.h"

using namespace std;

int main(int argc, char* argv[])
{
	cout << "opensync start" << endl;
	opensync::log4cpp_instance* out = opensync::log4cpp_instance::init_instance();
	out->logs << OUTINFO << "BOOST_VERSION: " << BOOST_VERSION;
	out->logs << OUTINFO << "BOOST_LIB_VERSION: " << BOOST_LIB_VERSION;

	opensync::system_action sys_ac;
	pid_t pid = getpid();
	out->logs << OUTINFO << "pcpu=" << sys_ac.get_proc_cpu(pid);
	out->logs << OUTINFO << "procmem=" << sys_ac.get_proc_mem(pid);
	out->logs << OUTINFO << "virtualmem=" << sys_ac.get_proc_virtualmem(pid);
	//printf("pcpu=%f\n", sys_ac.get_proc_cpu(pid));
	//printf("procmem=%d\n", sys_ac.get_proc_mem(pid));
	//printf("virtualmem=%d\n", sys_ac.get_proc_virtualmem(pid));

	//opensync::process_hidden* proc_hidden = opensync::process_hidden::init_instance(); //隐藏进程
	//boost::thread process_hidden_thread(boost::bind(&opensync::process_hidden::start_process_hidden, proc_hidden));  //隐藏进程

	if (argc != 3)
	{
		out->logs << OUTINFO << "argc=" << argc << "启动参数缺失";
		exit(-1);
	}

	opensync::file_system_listen* file_listen = opensync::file_system_listen::init_instance();
	file_listen->add_watch(argv[1]);
	file_listen->add_ignore_watch(argv[2]);

	boost::thread file_listen_thread(boost::bind(&opensync::file_system_listen::start_watch, file_listen));
	for (int i = 0; i < 1000; i++)
	{
		//out->logs << OUTDEBUG << i;
		sleep(2);
	}
	file_listen->shop_watch();
	file_listen_thread.join(); //线程回收

	//proc_hidden->stop_process_hidden();
	//process_hidden_thread.join(); //线程回收

	opensync::instance_garbo garbo = opensync::instance_garbo();
	cout << "opensync end" << endl;
}