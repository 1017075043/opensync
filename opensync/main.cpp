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
opensync::file_system_operation* file_op = opensync::file_system_operation::init_instance();


#include "crow.h"
#include "json/json.h"
#define CROW_LOG_CRITICAL out->logs << OUTCRIT
#define CROW_LOG_ERROR out->logs << OUTERROR
#define CROW_LOG_WARNING out->logs << OUTWARN
#define CROW_LOG_INFO out->logs << OUTINFO
#define CROW_LOG_DEBUG out->logs << OUTDEBUG

//启动http服务
void opensync_crow_start();
//处理访问静态资源的请求
void opensync_crow_static_resources(const crow::request& req, crow::response& res, const std::string& url, const std::string& filename);
//返回一个文件内容
void opensync_crow_send_file(crow::response& res, const std::string& filename, const std::string& content_type);
//include
void opensync_crow_include(crow::mustache::context& ctx, const std::string& filename, const std::string& label);
//判断是否登录
bool opensync_crow_check_login(const crow::request& req, crow::response& res);
//文件上传
bool opensync_crow_upload(const std::string& path, const std::string& data, std::map<std::string, int>& upload_result);

crow::App<crow::CookieParser> opensync_crow_app;
std::string opensync_crow_cookie_user_id;

int main(int argc, char* argv[])
{
	opensync_crow_start();
}

#define OPENSYNC_SITE "/home/wnh/projects/opensync/site/"
//启动crow http框架
void opensync_crow_start()
{
	crow::mustache::set_base(OPENSYNC_SITE);

	//首页,需要登录
	CROW_ROUTE(opensync_crow_app, "/site/opensync_index.html")
		.methods(crow::HTTPMethod::POST, crow::HTTPMethod::GET)([](const crow::request& req, crow::response& res) {
		if (opensync_crow_check_login(req, res)) //判断是否已经登录
		{
			res.set_header("Content-Type", "text/html");
			crow::mustache::context ctx;
			opensync_crow_include(ctx, "opensync_header.html", "opensync_header");
			ctx["opensync_title"] = "首页 - opensync";
			auto text = crow::mustache::load("opensync_index.html").render(ctx);
			res.write(text);
			res.end();
		}
		});
	//登录页面,无需登录
	CROW_ROUTE(opensync_crow_app, "/site/opensync_login.html")
		.methods(crow::HTTPMethod::POST, crow::HTTPMethod::GET)([](const crow::request& req, crow::response& res) {
		res.set_header("Content-Type", "text/html");
		crow::mustache::context ctx;
		ctx["opensync_title"] = "登录 - opensync";
		auto text = crow::mustache::load("opensync_login.html").render(ctx);
		res.write(text);
		res.end();
		});
	CROW_ROUTE(opensync_crow_app, "/site/login.html")
		.methods(crow::HTTPMethod::POST, crow::HTTPMethod::GET)([](const crow::request& req, crow::response& res) {
		res.set_header("Content-Type", "text/html");
		auto text = crow::mustache::load("login.html").render();
		res.write(text);
		res.end();
		});
	//登录动作,无需登录
	CROW_ROUTE(opensync_crow_app, "/site/opensync_login.do")
		.methods(crow::HTTPMethod::POST, crow::HTTPMethod::GET)([](const crow::request& req, crow::response& res) {
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
	CROW_ROUTE(opensync_crow_app, "/site/opensync_logout.do")
		.methods(crow::HTTPMethod::POST, crow::HTTPMethod::GET)([](const crow::request& req, crow::response& res) {
		if (opensync_crow_check_login(req, res)) //判断是否已经登录
		{
			res.set_header("Content-Type", "text/html");
			res.set_header("SET-COOKIE", "user_id=NULL");
			res.code = 302;
			res.set_header("Location", "opensync_login.html");
			res.end();
		}
		});
	//文件上传接口,需要登录
	CROW_ROUTE(opensync_crow_app, "/site/opensync_upload.do")
		.methods(crow::HTTPMethod::POST, crow::HTTPMethod::GET)([](const crow::request& req, crow::response& res) {
		if (opensync_crow_check_login(req, res)) //判断是否已经登录
		{
			std::string upload_path = req.url_params.get("path") != nullptr && req.url_params.get("path") != NULL ? req.url_params.get("path") : "/home";
			out->logs << OUTDEBUG << "upload_path=" << upload_path;
			out->logs << OUTDEBUG << "Content-Length=" << req.get_header_value("Content-Length");
			res.set_header("Content-Type", "text/html");
			std::map<std::string, int> upload_result;
			opensync_crow_upload(upload_path + "/", req.body, upload_result);
			std::map<std::string, int>::iterator it;
			it = upload_result.begin();
			Json::Value result;
			while (it != upload_result.end())
			{
				Json::Value file_result;

				file_result["filename"] = it->first;
				file_result["result"] = it->second;
				//out->logs << OUTINFO << it->first << "=" << it->second;
				result["info"].append(file_result);
				it++;
			}
			res.write(result.toStyledString());
			res.end();
		}
		});
	//文件系统,,需要登录
	CROW_ROUTE(opensync_crow_app, "/site/opensync_file_system.html")
		.methods(crow::HTTPMethod::POST, crow::HTTPMethod::GET)([](const crow::request& req, crow::response& res) {
		if (opensync_crow_check_login(req, res)) //判断是否已经登录
		{
			res.set_header("Content-Type", "text/html");
			crow::mustache::context ctx;
			opensync_crow_include(ctx, "opensync_header.html", "opensync_header");
			ctx["opensync_title"] = "文件系统 - opensync";
			auto text = crow::mustache::load("opensync_file_system.html").render(ctx);
			res.write(text);
			res.end();
		}
		});
	//目录资源接口,需要登录
	CROW_ROUTE(opensync_crow_app, "/site/opensync_dir_info.do")
		.methods(crow::HTTPMethod::POST, crow::HTTPMethod::GET)([](const crow::request& req, crow::response& res) {
		if (opensync_crow_check_login(req, res)) //判断是否已经登录
		{
			res.set_header("Content-Type", "text/html");
			//从get请求的path参数中获取访问的目录路径
			std::string local_path = req.url_params.get("path") != nullptr && req.url_params.get("path") != NULL ? req.url_params.get("path") : "/home";

			//获取目录内容，构建html代码
			vector<const opensync::file_attribute*> file_attr_list;
			file_attr_list = file_op->get_directory_file_list(local_path);
			out->logs << OUTDEBUG << file_attr_list.size();
			int i = 0;
			std::string file_context;
			Json::Value file_info_list;
			file_info_list["path"] = local_path;
			file_info_list["parents_path"] = local_path.rfind("/") != string::npos && local_path.rfind("/") != 0 ? local_path.substr(0, local_path.rfind("/")) : "/";
			BOOST_FOREACH(const opensync::file_attribute* file_attr, file_attr_list)
			{
				if (boost::filesystem::is_directory(file_attr->file_path) == true) 
				{
					Json::Value file_attribute;
					file_attribute["num"] = Json::Value(++i);
					file_attribute["filename"] = Json::Value(file_attr->file_path.filename().c_str());
					file_attribute["permissions"] = Json::Value(file_attr->permissions_name);
					file_attribute["user"] = Json::Value(file_attr->user_name);
					file_attribute["group"] = Json::Value(file_attr->group_name);
					file_attribute["size"] = Json::Value(std::to_string(file_attr->file_size));
					file_attribute["last_write_time"] = Json::Value(file_attr->last_write_time_s);
					file_attribute["type"] = Json::Value("dir");
					file_info_list["info"].append(file_attribute);
				}
			}
			BOOST_FOREACH(const opensync::file_attribute* file_attr, file_attr_list)
			{
				if (boost::filesystem::is_directory(file_attr->file_path) == false)
				{
					Json::Value file_attribute;
					file_attribute["num"] = Json::Value(++i);
					file_attribute["filename"] = Json::Value(file_attr->file_path.filename().c_str());
					file_attribute["permissions"] = Json::Value(file_attr->permissions_name);
					file_attribute["user"] = Json::Value(file_attr->user_name);
					file_attribute["group"] = Json::Value(file_attr->group_name);
					file_attribute["size"] = Json::Value(std::to_string(file_attr->file_size));
					file_attribute["last_write_time"] = Json::Value(file_attr->last_write_time_s);
					file_attribute["type"] = Json::Value("file");
					file_info_list["info"].append(file_attribute);
				}
			}
			file_info_list["count"] = i;
			//out->logs << OUTINFO << file_info_list.toStyledString();
			out->logs << OUTINFO << "目录资源, 目录=" << local_path;
			res.write(file_info_list.toStyledString());
			res.end();
		}
		});
	//文件下载接口,需要登录
	CROW_ROUTE(opensync_crow_app, "/site/opensync_download.do")
		.methods(crow::HTTPMethod::POST, crow::HTTPMethod::GET)([](const crow::request& req, crow::response& res) {
		if (opensync_crow_check_login(req, res)) //判断是否已经登录
		{
			//out->logs << OUTDEBUG << "Content-Length=" << req.get_header_value("Content-Length");
			res.set_header("Content-Type", "text/html");
			//从get请求的file参数中获取要下载文件的路径
			std::string download_file = req.url_params.get("file") != nullptr && req.url_params.get("file") != NULL ? req.url_params.get("file") : "";
			if (download_file.empty())
			{
				res.code = 503;
				res.write("Not found");
				res.end();
			}
			else
			{
				std::ifstream in(download_file, std::ifstream::in);
				if (in)
				{
					std::ostringstream contents;
					contents << in.rdbuf();
					in.close();
					res.set_header("Content-Type", "text/javascript");
					res.write(contents.str());
					out->logs << OUTINFO << "下载资源，文件=" << download_file;
				}
				else
				{
					res.code = 503;
					res.write("Not found");
				}
				res.end();
			}
		}
		});
	//文件删除接口,需要登录
	CROW_ROUTE(opensync_crow_app, "/site/opensync_delete.do")
		.methods(crow::HTTPMethod::POST, crow::HTTPMethod::GET)([](const crow::request& req, crow::response& res) {
		//out->logs << OUTDEBUG << "Content-Length=" << req.get_header_value("Content-Length");
		res.set_header("Content-Type", "text/html");
		//从get请求的file参数中获取要下载文件的路径
		std::string delete_file = req.url_params.get("file") != nullptr && req.url_params.get("file") != NULL ? req.url_params.get("file") : "";
		if (delete_file.empty())
		{
			res.code = 503;
			res.write("Not found");
			res.end();
		}
		Json::Value result;
		if (file_op->act_delete_file(delete_file))
		{
			out->logs << OUTINFO << "删除文件，文件=" << delete_file;
			result["result"] = 1;
		}
		else 
		{
			out->logs << OUTWARN << "删除文件失败，文件=" << delete_file;
			result["result"] = 0;
		}
		res.write(result.toStyledString());
		res.end();
		});
	//访问css资源,无需登录
	CROW_ROUTE(opensync_crow_app, "/site/css/<string>")
		.methods(crow::HTTPMethod::POST, crow::HTTPMethod::GET)([](const crow::request& req, crow::response& res, std::string filename) {
		filename = opensync::string_operation::url_decode(filename);
		opensync_crow_static_resources(req, res, "css/" + filename, filename);
		});
	//访问fonts资源,无需登录
	CROW_ROUTE(opensync_crow_app, "/site/fonts/<string>")
		.methods(crow::HTTPMethod::POST, crow::HTTPMethod::GET)([](const crow::request& req, crow::response& res, std::string filename) {
		filename = opensync::string_operation::url_decode(filename);
		opensync_crow_static_resources(req, res, "fonts/" + filename, filename);
		});
	//访问js资源,无需登录
	CROW_ROUTE(opensync_crow_app, "/site/js/<string>")
		.methods(crow::HTTPMethod::POST, crow::HTTPMethod::GET)([](const crow::request& req, crow::response& res, std::string filename) {
		filename = opensync::string_operation::url_decode(filename);
		opensync_crow_static_resources(req, res, "js/" + filename, filename);
		});
	//访问images资源,无需登录
	CROW_ROUTE(opensync_crow_app, "/site/images/<string>")
		.methods(crow::HTTPMethod::POST, crow::HTTPMethod::GET)([](const crow::request& req, crow::response& res, std::string filename) {
		filename = opensync::string_operation::url_decode(filename);
		opensync_crow_static_resources(req, res, "images/" + filename, filename);
		});
	//访问images资源,无需登录
	CROW_ROUTE(opensync_crow_app, "/site/images/<string>/<string>")
		.methods(crow::HTTPMethod::POST, crow::HTTPMethod::GET)([](const crow::request& req, crow::response& res, std::string filename, std::string filename1) {
		filename = opensync::string_operation::url_decode(filename);
		filename1 = opensync::string_operation::url_decode(filename1);
		opensync_crow_static_resources(req, res, "images/" + filename + "/" + filename1, filename1);
		});


	opensync_crow_app.port(18080).multithreaded().run();
}
//处理访问静态资源的请求
void opensync_crow_static_resources(const crow::request& req, crow::response& res, const std::string& url, const std::string& filename)
{
	out->logs << OUTDEBUG << "访问静态资源，文件=" << url;
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
		//out->logs << OUTINFO << "访问HTML资源，文件=" << OPENSYNC_SITE + filename;
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
		out->logs << OUTDEBUG << "导入文件，文件=" << OPENSYNC_SITE + filename;
	}
}
//判断是否登录
bool opensync_crow_check_login(const crow::request& req, crow::response& res)
{
	auto& cookie_ctx = opensync_crow_app.get_context<crow::CookieParser>(req);
	std::string user_id = cookie_ctx.get_cookie("user_id");
	if (user_id.empty() || user_id != opensync_crow_cookie_user_id)
	{
		out->logs << OUTINFO << "未登录，拒绝访问";
		res.set_header("Content-Type", "text/html");
		res.code = 302;
		res.set_header("Location", "opensync_login.html");
		res.end();
		return false;
	}
	return true;
}
//文件上传
bool opensync_crow_upload(const std::string& path, const std::string& data, std::map<std::string, int>& upload_result)
{
	std::string boundary = data.substr(0, data.find("\n"));
	std::string other = data.substr(boundary.length() + 1);
	std::string file1;
	if (other.find(boundary) == string::npos)
	{
		file1 = other.substr(0, other.length() - boundary.length() - 5);
		out->logs << OUTDEBUG << "上传的文件解析完成";
	}
	else
	{
		file1 = other.substr(0, other.find(boundary)-2);
		out->logs << OUTDEBUG << "继续解析下一个上传的文件";
	}

	std::string content_disposition = file1.substr(file1.find("Content-Disposition: "), file1.find("\n"));
	out->logs << OUTDEBUG << content_disposition;

	file1 = file1.substr(file1.find("\n") + 1);
	std::string content_type = file1.substr(file1.find("Content-Type: "), file1.find("\n"));
	out->logs << OUTDEBUG << content_type;

	std::string filename = content_disposition.substr(content_disposition.find("filename") + 10);
	std::string filename_s = filename.substr(0, filename.length() - 2);
	filename = path + filename_s;
	out->logs << OUTDEBUG << "filename=" << filename;

	file1 = file1.substr(file1.find("\n") + 3);
	//out->logs << OUTINFO << "file1=" << file1;
	if (filename != path)
	{
		try
		{
			ofstream fout(filename);
			if (fout.is_open())
			{
				fout << file1;
				fout.close();
				upload_result[filename_s] = 1;
				out->logs << OUTINFO << "上传成功，保存路径=" << filename;
			}
			else
			{
				upload_result[filename_s] = 0;
				throw opensync::exception() << opensync::err_str("保存异常 " + filename + ", mesg=" + strerror(errno));
			}
		}
		catch (exception& e)
		{
			out->logs << OUTERROR << *boost::get_error_info<opensync::err_str>(e);
		}
	}

	if (other.find(boundary) != string::npos)
	{
		other = other.substr(other.find(boundary));
		opensync_crow_upload(path, other, upload_result);
	}
	return true;
}


/*
#include <iostream>
#include <string>
#include "json/json.h"   
using namespace std;
int main() {
	Json::Value json_temp;
	json_temp["name"] = Json::Value("sharexu");
	json_temp["age"] = Json::Value(18);
	Json::Value json_temp1;
	json_temp1["name"] = Json::Value("sharexu");
	json_temp1["age"] = Json::Value(18);
	json_temp["wnh"].append(json_temp1);
	json_temp["wnh"].append(json_temp1);
	json_temp["wnh"].append(json_temp1);
	cout << json_temp["name"].asString() << endl;
	cout << json_temp.toStyledString() << endl;
	//Json::Value root;
	//root["key_string"] = Json::Value("value_string");
	//root["key_number"] = Json::Value(12345);
	//root["key_boolean"] = Json::Value(false);
	//root["key_double"] = Json::Value(12.345);
	//root["key_object"] = json_temp;
	//root["key_array"].append("array_string");
	//root["key_array"].append(1234);
	//
	//Json::FastWriter fast_writer;
	//std::cout << fast_writer.write(root);
	//
	//Json::StyledWriter styled_writer;
	//std::cout << styled_writer.write(root);
	//
	//string str_test = "{\"id\":1,\"name\":\"pacozhong\"}";
	//Json::Reader reader;
	//Json::Value value;
	//if (!reader.parse(str_test, value))
	//	return 0;
	//string value_name = value["name"].asString();
	//cout << value_name << endl;
	//cout << value["name"];
	//if (!value["id"].isInt()) {
	//	cout << "id is not int" << endl;
	//}
	//else {
	//	int value_id = value["id"].asInt();
	//	cout << value_id << endl;
	//}
	return 0;
}
*/
/*
要安装jsoncpp,首先要下载好scons,再去安装jsoncpp
scons下载地址：wget http://prdownloads.sourceforge.NET/scons/scons-2.2.0.tar.gz
Jsoncpp 下载地址 http://sourceforge.net/projects/jsoncpp/files/latest/download?_test=goal
 以上的下载地址未用过，但是下面的命令确实可以实现JsonCPP安装成功（下面命令都是小写，不知道为什么提交后部分变成大写）
安装步骤如下：
#tar -zxvf scons-2.1.0.tar.gz #cd scons-2.1.0
#python setup.py install
#tar -zxvf jsoncpp-src-0.5.0.tar.gz
#cd jsoncpp-src-0.5.0
#scons platform=linux-gcc
//这里操作的动态库文件（.so）
#mv libs/linux-gcc-4.4.7/libjson_linux-gcc-4.4.7_libmt.so  /lib
#ln /lib/libjson_linux-gcc-4.4.7_libmt.so   /lib/libjson.so
#mv include/json/ /usr/include/
#ldconfig
#./bin/linux-gcc-4.4.7/test_lib_json
【注意这里的版本号可能不同】先通过cd libs进入libs目录，再用“ls”命令查看自己的版本号，然后在上述步骤中修改成自己的版本号即可
Testing ValueTest/size: OK
Testing ValueTest/isObject: OK
Testing ValueTest/isArray: OK
Testing ValueTest/isBool: OK
Testing ValueTest/isInt: OK
Testing ValueTest/isUInt: OK
Testing ValueTest/isDouble: OK
Testing ValueTest/isString: OK
Testing ValueTest/isNull: OK
All 9 tests passed
上面操作动态库文件的命令行其实也可对静态库进行操作(当前目录jsoncpp-src-0.5.0)
#mv libs/linux-gcc-4.4.7/libjson_linux-gcc-4.47_libmt.a  /usr/local/lib
#ln  /usr/local/lib/libjson_linux-gcc-4.47_libmt.a   /usr/local/lib/libjson.a
 如果只对静态库转移的话，那.bin/linux-gcc-4.4.7/test_lib_json会报错
*/
/*
静态链接参数
g++ -o "/home/wnh/projects/opensync/bin/x64/Debug/opensync.out" -Wl,--no-undefined -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack -Wl,-Bstatic /home/wnh/projects/opensync/obj/x64/Debug/exception.o /home/wnh/projects/opensync/obj/x64/Debug/file_info_databases.o /home/wnh/projects/opensync/obj/x64/Debug/file_system_listen.o /home/wnh/projects/opensync/obj/x64/Debug/file_system_operation.o /home/wnh/projects/opensync/obj/x64/Debug/instance_garbo.o /home/wnh/projects/opensync/obj/x64/Debug/log4cpp_instance.o /home/wnh/projects/opensync/obj/x64/Debug/main.o /home/wnh/projects/opensync/obj/x64/Debug/process_hidden.o /home/wnh/projects/opensync/obj/x64/Debug/string_operation.o /home/wnh/projects/opensync/obj/x64/Debug/system_action.o /home/wnh/projects/opensync/obj/x64/Debug/user_group_info.o -llog4cpp -lboost_date_time -lboost_filesystem -lboost_program_options -lboost_regex -lboost_system -lboost_prg_exec_monitor -lboost_unit_test_framework -lboost_thread -ljson -lcrypto -Wl,-Bdynamic  -Wl,--as-needed -lpthread
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