#pragma once

#include "opensync_public_include_file.h"

#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/filesystem/fstream.hpp>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <openssl/des.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>


#include "file_info_databases.h"
#include "user_group_info.h"
#include "string_operation.h"

/*
	文件系统操作类(单例)
	@author 吴南辉
	@time 2020/05/30
*/

namespace opensync
{
	class file_system_operation : boost::noncopyable
	{
	private:
		static file_system_operation* instance;
		opensync::log4cpp_instance* out = opensync::log4cpp_instance::init_instance();
		opensync::user_group_info* user_group = opensync::user_group_info::init_instance();
		opensync::file_info_databases* file_info = opensync::file_info_databases::init_instance();
	private:
		//file_system_operation
		void init_file_info(const string& file_path); //初始化文件信息

		//file_system_operation_get
		int get_file_uid(const string& file_path); //获取文件的uid
		int get_file_gid(const string& file_path); //获取文件的gid
		string get_file_md5_p(const string& file_path); //获取文件md5值

		//file_system_operation_show
		void show_file_info_p(const string& file_path); //显示文件的属性（私用）

	private:
		//file_system_operation
		file_system_operation();
		~file_system_operation();

	public:
		//file_system_operation
		static file_system_operation* get_instance();
		static file_system_operation* init_instance();
		static void destory();

	public:
		//file_system_operation_get
		const opensync::file_attribute* get_file_info(const string& file_path); //获取一个文件属性信息
		const vector<const opensync::file_attribute*> get_directory_file_list(const string& file_path); //遍历获取一个目录下所有类型文件的属性信息列表（单层)
		const vector<const opensync::file_attribute*> get_file_and_dir_traverse_all_list(const string& file_path); //遍历获取一个目录或目录及其下级所有类型文件的属性信息列表
		const vector<const opensync::file_attribute*> get_file_and_dir_traverse_dir_list(const string& file_path); //遍历获取一个目录或目录及其下级目录类型文件的属性信息列表
		const vector<const opensync::file_attribute*> get_file_and_dir_traverse_file_list(const string& file_path);//遍历获取一个目录或目录及其下级非目录类型文件的属性信息列表
		const path get_file_path(const string& file_path); //获取文件file_path
		const long get_file_size(const string& file_path); //获取文件file_size
		const file_type get_file_type(const string& file_path); //获取文件file_type
		const string get_file_type_name(const string& file_path); //获取文件file_type_name
		const perms get_file_permissions(const string& file_path); //获取文件file_permissions
		const string get_file_permissions_name(const string& file_path); //获取文件file_permissions_name
		const time_t get_file_last_write_time(const string& file_path); //获取文件file_last_write_time
		const string get_file_last_write_time_s(const string& file_path); //获取文件file_last_write_time_s
		const int get_file_user(const string& file_path); //获取文件file_user
		const string get_file_user_name(const string& file_path); //获取文件file_user_name
		const int get_file_group(const string& file_path); //获取文件file_group
		const string get_file_group_name(const string& file_path); //获取文件file_group_name
		const string get_file_hash(const string& file_path); //获取文件file_hash
		const bool get_file_status(const string& file_path); //获取文件file_status

		//file_system_operation_show
		void show_file_info(const string& file_path); //显示文件的属性
		void show_file_info_databases(); //显示目前文件信息库中存储文件的信息

		//file_system_operation_update
		void update_file_info(const string& file_path); //更新文件信息
		bool update_file_size(const string& file_path, const long& file_size); //更新文件file_size
		bool update_file_type(const string& file_path, const file_type& type); //更新文件file_type
		bool update_file_type_name(const string& file_path, const string& type_name); //更新文件file_type_name
		bool update_file_permissions(const string& file_path, const perms& permissions); //更新文件file_permissions
		bool update_file_permissions_name(const string& file_path, const string& permissions_name); //更新文件file_permissions_name
		bool update_file_last_write_time(const string& file_path, const time_t& last_write_time); //更新文件file_last_write_time
		bool update_file_last_write_time_s(const string& file_path, const string& last_write_time_s); //更新文件file_last_write_time_s
		bool update_file_user(const string& file_path, const int& user); //更新文件file_user
		bool update_file_user_name(const string& file_path, const string& user_name); //更新文件file_user_name
		bool update_file_group(const string& file_path, const int& group); //更新文件file_group
		bool update_file_group_name(const string& file_path, const string& group_name); //更新文件file_group_name
		bool update_file_hash(const string& file_path, const string& file_hash); //更新文件file_hash
		bool update_file_status(const string& file_path, const bool& status); //更新文件file_status

		//file_system_operation_set
		bool set_file_permissions(const string& file_path, const perms& permissions);//设置文件file_permissions
		bool set_file_permissions(const string& file_path, const string& permissions_name);//设置文件file_permissions_name
		bool set_file_last_write_time(const string& file_path, const time_t& last_write_time); //设置文件file_last_write_time
		bool set_file_last_write_time_s(const string& file_path, const string& last_write_time_s); //设置文件file_last_write_time_s
		bool set_file_user(const string& file_path, const int& user);//设置文件file_user
		bool set_file_user(const string& file_path, const string& user_name);//设置文件file_user_name
		bool set_file_group(const string& file_path, const int& group);//设置文件file_user
		bool set_file_group(const string& file_path, const string& group_name);//设置文件file_group_name
		bool set_file_user_group(const string& file_path, const int& user, const int& group);//设置文件file_user和file_user_name
		bool set_file_user_group(const string& file_path, const string& user_name, const string& group_name);//设置文件file_user_name和file_user_name

		//file_system_operation_act
		bool act_delete_file(const string& file_path);//删除一个文件
	};
}

