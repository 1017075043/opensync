#pragma once
#include "opensync_public_include_file.h"

#include <map>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>

#include <boost/bimap.hpp>
#include <boost/noncopyable.hpp>

using namespace std;

/*
	系统用户、用户组类（单例）
	@author 吴南辉
	@time 2020/05/30
*/

namespace opensync
{
	class user_group_info : boost::noncopyable
	{
	private:
		string passwd_file = "/etc/passwd";
		string group_file = "/etc/group";

		typedef boost::bimap<int, string> bimap;
		bimap user; //存放用户信息
		bimap group; //存放用户组信息

		static user_group_info* instance;
		opensync::log4cpp_instance* out = opensync::log4cpp_instance::init_instance();
	private:
		//user_group_info
		user_group_info();
		~user_group_info();
	public:
		//user_group_info
		static user_group_info* get_instance();
		static user_group_info* init_instance();
		static void destory();

	public:
		//user_group_info
		bool init_system_all_user_group_info(); //获取系统所有用户、用户组信息
		bool init_system_all_user_info(); //获取系统所有用户信息
		bool init_system_all_group_info(); //获取系统所有组用户信息

		//user_group_info_get
		string get_user_name(const int& id); //获取用户名
		int get_user_id(const string& name); //获取用户ID
		string get_group_name(const int& id); //获取用户组名
		int get_group_id(const string& name); //获取用户组ID
		
		//user_group_info_is_exist
		bool is_exist_user_name(const string& name); //判断是否存在用户名
		bool is_exist_user_id(const int& id); //判断是否存在用户ID
		bool is_exist_group_name(const string& name); //判断是否存在用户组名
		bool is_exist_group_id(const int& id); //判断是否存在用户组ID

		//user_group_info_show
		void show_all_user_info(); //显示所有用户信息
		void show_all_group_info(); //显示所有用户组信息
	};
}

