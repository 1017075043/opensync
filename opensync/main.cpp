#include "opensync_public_include_file.h"

#include <map>
#include <fstream>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <unordered_set>
#include <mutex>

#include <boost/version.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/noncopyable.hpp>
#include <boost/filesystem.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/detail/sha1.hpp>

#include "instance_garbo.h"
#include "file_system_operation.h"
#include "file_system_listen.h"
#include "process_hidden.h"
#include "system_action.h"
#include "string_operation.h"

using namespace std;
opensync::log4cpp_instance* out = opensync::log4cpp_instance::init_instance();


#include "crow.h"
#define CROW_LOG_CRITICAL out->logs << OUTCRIT
#define CROW_LOG_ERROR out->logs << OUTERROR
#define CROW_LOG_WARNING out->logs << OUTWARN
#define CROW_LOG_INFO out->logs << OUTINFO
#define CROW_LOG_DEBUG out->logs << OUTDEBUG

void opensync_crow_start();
//处理访问静态资源的请求
void opensync_crow_static_resources(const crow::request& req, crow::response& res, const std::string& url, const std::string& filename);
//返回一个文件内容
void opensync_crow_send_file(crow::response& res, const std::string& filename, const std::string& content_type);
//include
void opensync_crow_include(crow::mustache::context& ctx, const std::string& filename, const std::string& label);
//判断是否登录
bool opensync_crow_check_login(const crow::request& req, crow::response& res);

crow::App<crow::CookieParser> opensync_crow_app;
std::string opensync_crow_cookie_user_id;

int main(int argc, char* argv[])
{
	opensync_crow_start();
}

#define OPENSYNC_SITE "/home/wnh/projects/opensync/site/"
void opensync_crow_start()
{
	crow::mustache::set_base(OPENSYNC_SITE);

	//首页,需要登录
	CROW_ROUTE(opensync_crow_app, "/site/opensync_index.html").methods(crow::HTTPMethod::POST, crow::HTTPMethod::GET)([](const crow::request& req, crow::response& res) {
		if (opensync_crow_check_login(req, res)) //判断是否已经登录
		{
			crow::mustache::context ctx;
			opensync_crow_include(ctx, "opensync_header.html", "opensync_header");
			ctx["opensync_title"] = "首页 - opensync";
			auto text = crow::mustache::load("opensync_index.html").render(ctx);
			res.write(text);
			res.end();
		}
		});
	//登录页面,无需登录
	CROW_ROUTE(opensync_crow_app, "/site/opensync_login.html").methods(crow::HTTPMethod::POST, crow::HTTPMethod::GET)([](const crow::request& req, crow::response& res) {
		res.set_header("Content-Type", "text/html");
		crow::mustache::context ctx;
		ctx["opensync_title"] = "登录 - opensync";
		auto text = crow::mustache::load("opensync_login.html").render(ctx);
		res.write(text);
		res.end();
		});
	CROW_ROUTE(opensync_crow_app, "/site/login.html").methods(crow::HTTPMethod::POST, crow::HTTPMethod::GET)([](const crow::request& req, crow::response& res) {
		res.set_header("Content-Type", "text/html");
		auto text = crow::mustache::load("login.html").render();
		res.write(text);
		res.end();
		});
	//登录动作,无需登录
	CROW_ROUTE(opensync_crow_app, "/site/opensync_login.do").methods(crow::HTTPMethod::POST, crow::HTTPMethod::GET)([](const crow::request& req, crow::response& res) {
		std::map<std::string, std::string> post_params;
		opensync::string_operation::get_post_params(post_params, req.body);
		out->logs << OUTINFO << "username:" << post_params["username"] << ", password:" << post_params["password"];
		if (post_params["username"] == "admin" && post_params["password"] == "123456")
		{
			boost::uuids::random_generator rgen;
			opensync_crow_cookie_user_id = boost::uuids::to_string(rgen());
			res.set_header("SET-COOKIE", "user_id=" + opensync_crow_cookie_user_id);
			res.write("{login_result:1}");
		}
		else
		{
			res.write("{login_result:0}");
		}
		res.end();
		});
	//注销动作,需要登录
	CROW_ROUTE(opensync_crow_app, "/site/opensync_logout.do").methods(crow::HTTPMethod::POST, crow::HTTPMethod::GET)([](const crow::request& req, crow::response& res) {
		if (opensync_crow_check_login(req, res)) //判断是否已经登录
		{
			res.set_header("Content-Type", "text/html");
			res.set_header("SET-COOKIE", "user_id=NULL");
			res.code = 302;
			res.set_header("Location", "opensync_login.html");
			res.end();
		}
		});
	//访问css资源,无需登录
	CROW_ROUTE(opensync_crow_app, "/site/css/<string>").methods(crow::HTTPMethod::POST, crow::HTTPMethod::GET)([](const crow::request& req, crow::response& res, std::string filename) {
		filename = opensync::string_operation::url_decode(filename);
		opensync_crow_static_resources(req, res, "css/" + filename, filename);
		});
	//访问fonts资源,无需登录
	CROW_ROUTE(opensync_crow_app, "/site/fonts/<string>").methods(crow::HTTPMethod::POST, crow::HTTPMethod::GET)([](const crow::request& req, crow::response& res, std::string filename) {
		filename = opensync::string_operation::url_decode(filename);
		opensync_crow_static_resources(req, res, "fonts/" + filename, filename);
		});
	//访问js资源,无需登录
	CROW_ROUTE(opensync_crow_app, "/site/js/<string>").methods(crow::HTTPMethod::POST, crow::HTTPMethod::GET)([](const crow::request& req, crow::response& res, std::string filename) {
		filename = opensync::string_operation::url_decode(filename);
		opensync_crow_static_resources(req, res, "js/" + filename, filename);
		});
	//访问images资源,无需登录
	CROW_ROUTE(opensync_crow_app, "/site/images/<string>").methods(crow::HTTPMethod::POST, crow::HTTPMethod::GET)([](const crow::request& req, crow::response& res, std::string filename) {
		filename = opensync::string_operation::url_decode(filename);
		opensync_crow_static_resources(req, res, "images/" + filename, filename);
		});
	//访问images资源,无需登录
	CROW_ROUTE(opensync_crow_app, "/site/images/<string>/<string>").methods(crow::HTTPMethod::POST, crow::HTTPMethod::GET)([](const crow::request& req, crow::response& res, std::string filename, std::string filename1) {
		filename = opensync::string_operation::url_decode(filename);
		filename1 = opensync::string_operation::url_decode(filename1);
		opensync_crow_static_resources(req, res, "images/" + filename + "/" + filename1, filename1);
		});


	opensync_crow_app.port(18080).multithreaded().run();
}
//处理访问静态资源的请求
void opensync_crow_static_resources(const crow::request& req, crow::response& res, const std::string& url, const std::string& filename)
{
	out->logs << OUTINFO << "access file=" << url;
	string filetype = filename.substr(filename.rfind("."));
	if (filetype == ".css")
	{
		opensync_crow_send_file(res, url, "text/css");
	}
	else if (filetype == ".js")
	{
		opensync_crow_send_file(res, url, "text/javascript");
	}
	else if (filetype == ".jpg")
	{
		opensync_crow_send_file(res, url, "image/jpeg");
	}
	else if (filetype == ".png")
	{
		opensync_crow_send_file(res, url, "image/apng");
	}
	else if (filetype == ".woff2")
	{
		opensync_crow_send_file(res, url, "font/woff2");
	}
	else
	{
		res.code = 403;
		res.write("error file type");
		res.end();
	}
}
//返回一个文件内容
void opensync_crow_send_file(crow::response& res, const std::string& filename, const std::string& content_type)
{
	std::ifstream in(OPENSYNC_SITE + filename, std::ifstream::in);
	if (in)
	{
		std::ostringstream contents;
		contents << in.rdbuf();
		in.close();
		res.set_header("Content-Type", content_type);
		res.write(contents.str());
	}
	else
	{
		res.code = 404;
		res.write("Not found");
	}
	res.end();
}
//include
void opensync_crow_include(crow::mustache::context& ctx, const std::string& filename, const std::string& label)
{
	std::ifstream in(OPENSYNC_SITE + filename, std::ifstream::in);
	if (in)
	{
		std::ostringstream contents;
		contents << in.rdbuf();
		in.close();
		ctx[label] = contents.str();
	}
}
//判断是否登录
bool opensync_crow_check_login(const crow::request& req, crow::response& res) 
{
	auto& cookie_ctx = opensync_crow_app.get_context<crow::CookieParser>(req);
	std::string user_id = cookie_ctx.get_cookie("user_id");
	if (user_id.empty() || user_id != opensync_crow_cookie_user_id)
	{
		out->logs << OUTINFO << "cookie error, denied";
		res.set_header("Content-Type", "text/html");
		res.code = 302;
		res.set_header("Location", "opensync_login.html");
		res.end();
		return false;
	}
	return true;
}
/*
今天链接参数
 g++ -o "/home/wnh/projects/opensync/bin/x64/Debug/opensync.out" -Wl,--no-undefined -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack -Wl,-Bstatic /home/wnh/projects/opensync/obj/x64/Debug/exception.o /home/wnh/projects/opensync/obj/x64/Debug/file_info_databases.o /home/wnh/projects/opensync/obj/x64/Debug/file_system_listen.o /home/wnh/projects/opensync/obj/x64/Debug/file_system_operation.o /home/wnh/projects/opensync/obj/x64/Debug/instance_garbo.o /home/wnh/projects/opensync/obj/x64/Debug/log4cpp_instance.o /home/wnh/projects/opensync/obj/x64/Debug/main.o /home/wnh/projects/opensync/obj/x64/Debug/process_hidden.o /home/wnh/projects/opensync/obj/x64/Debug/string_operation.o /home/wnh/projects/opensync/obj/x64/Debug/system_action.o /home/wnh/projects/opensync/obj/x64/Debug/user_group_info.o -llog4cpp -lboost_date_time -lboost_filesystem -lboost_program_options -lboost_regex -lboost_system -lboost_prg_exec_monitor -lboost_unit_test_framework -lboost_thread -lcrypto -Wl,-Bdynamic  -Wl,--as-needed -lpthread
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

	opensync::opensync_crow* http_server = opensync::opensync_crow::init_instance();
	boost::thread http_server_thread(boost::bind(&opensync::opensync_crow::start, http_server)); //启动http服务

	if (argc != 3)
	{
		out->logs << OUTINFO << "argc=" << argc << "启动参数缺失";
		//exit(-1);
	}

	opensync::file_system_listen* file_listen = opensync::file_system_listen::init_instance();
	//file_listen->add_watch(argv[1]);
	//file_listen->add_ignore_watch(argv[2]);
	file_listen->add_watch("/tmp/opensync/a");
	file_listen->add_ignore_watch("/tmp/opensync/b");

	boost::thread file_listen_thread(boost::bind(&opensync::file_system_listen::start_watch, file_listen));

	for (int i = 0; i < 3000; i++)
	{
		//out->logs << OUTDEBUG << i;
		sleep(2);
	}

	http_server->stop();
	http_server_thread.join();//线程回收

	for (int i = 0; i < 30; i++)
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
*/