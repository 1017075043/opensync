#pragma once
#include "opensync_public_include_file.h"

#include <vector>
#include <map>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/bimap.hpp>
#include <boost/noncopyable.hpp>
#include <boost/format.hpp>

/*
	文件信息类（单例）
	@author 吴南辉
	@time 2020/05/30
*/

using namespace std;

namespace opensync
{
	using namespace boost;
	using namespace boost::filesystem;

	typedef
		struct node {
			path file_path;
			long file_size;
			file_type type;
			string type_name;
			perms permissions;
			string permissions_name;
			time_t last_write_time;
			string last_write_time_s;
			int user;
			string user_name;
			int group;
			string group_name;
			string file_hash;
			bool status;
		} 
	file_attribute;

	class file_info_databases : boost::noncopyable
	{
	private:
		static file_info_databases* instance;
		opensync::log4cpp_instance* out = opensync::log4cpp_instance::init_instance();
		typedef boost::bimap<int, string> bimap;
		bimap file_type_dictionary; //存放文件类型字典
		bimap file_permissions_dictionary; //存放文件权限字典
	private:
		//file_info_databases
		file_info_databases();
		~file_info_databases();
		void init_file_type_dictionary(); //初始化文件类型字典
		void init_file_permissions_dictionary(); //初始化文件权限字典
	public:
		//file_info_databases
		static file_info_databases* get_instance();
		static file_info_databases* init_instance();
		static void destory();
	public:
		map<string, file_attribute> data; //存放文件信息
	public:
		//file_info_databases_transfrom
		string transfrom_file_type(const int& type); //转换文件类型
		int transfrom_file_type(const string& type); //转换文件类型
		string transfrom_file_permissions(const int& permissions); //转换文件权限
		int transfrom_file_permissions(const string& permissions); //转换文件权限

		//file_info_databases_is_exist
		bool is_exist_file_type(const int& type); //判断转换文件类型是否存在
		bool is_exist_file_type(const string& type); //判断转换文件类型是否存在
		bool is_exist_file_permissions(const int& permissions); //判断文件权限是否存在
		bool is_exist_file_permissions(const string& permissions); //判断文件权限是否存在
	};
}