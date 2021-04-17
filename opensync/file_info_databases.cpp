#include "file_info_databases.h"

/*####################### file_info_databases #######################*/
namespace opensync
{
	file_info_databases* file_info_databases::instance = nullptr;
	file_info_databases::file_info_databases()
	{
		init_file_type_dictionary();
		init_file_permissions_dictionary();
	}
	file_info_databases::~file_info_databases()
	{

	}
	file_info_databases* file_info_databases::get_instance()
	{
		return instance;
	}
	file_info_databases* file_info_databases::init_instance()
	{
		if (instance == nullptr) {
			instance = new file_info_databases();
		}
		return instance;
	}
	void file_info_databases::destory()
	{
		if (instance != nullptr)
		{
			delete instance;
			instance = nullptr;
		}
	}
	void file_info_databases::init_file_type_dictionary() //初始化文件类型字典
	{
		file_type_dictionary.left.insert(make_pair(0, "status_error"));
		file_type_dictionary.left.insert(make_pair(1, "file_not_found"));
		file_type_dictionary.left.insert(make_pair(2, "regular_file"));
		file_type_dictionary.left.insert(make_pair(3, "directory_file"));
		file_type_dictionary.left.insert(make_pair(4, "symlink_file"));
		file_type_dictionary.left.insert(make_pair(5, "block_file"));
		file_type_dictionary.left.insert(make_pair(6, "character_file"));
		file_type_dictionary.left.insert(make_pair(7, "fifo_file"));
		file_type_dictionary.left.insert(make_pair(8, "socket_file"));
		file_type_dictionary.left.insert(make_pair(9, "reparse_file"));
		file_type_dictionary.left.insert(make_pair(10, "type_unknown"));
		file_type_dictionary.left.insert(make_pair(11, "_detail_directory_symlink"));
	}
	void file_info_databases::init_file_permissions_dictionary() //初始化文件权限字典
	{
		boost::format fmt("%03d");
		for (int user = 0; user <= 7; ++user)
		{
			for (int group = 0; group <= 7; ++group)
			{
				for (int other = 0; other <= 7; ++other)
				{
					file_permissions_dictionary.left.insert(make_pair(user * 8 * 8 + group * 8 + other, (fmt % to_string(user * 100 + group * 10 + other)).str()));
					//out->logs << OUTDEBUG << user * 8 * 8 + group * 8 + other << " = " << (fmt % to_string(user * 100 + group * 10 + other)).str();
				}
			}
		}
	}
}

/*####################### file_info_databases_is_exist #######################*/
namespace opensync
{
	bool file_info_databases::is_exist_file_type(const int& type) //判断转换文件类型是否存在
	{
		if (file_type_dictionary.left.find(type) != file_type_dictionary.left.end())
		{
			return true;
		}
		out->logs << OUTWARN << type << " is not exist";
		return false;
	}
	bool file_info_databases::is_exist_file_type(const string& type) //判断转换文件类型是否存在
	{
		if (file_type_dictionary.right.find(type) != file_type_dictionary.right.end())
		{
			return true;
		}
		out->logs << OUTWARN << type << " is not exist";
		return false;
	}
	bool file_info_databases::is_exist_file_permissions(const int& permissions) //判断文件权限是否存在
	{
		if (file_permissions_dictionary.left.find(permissions) != file_permissions_dictionary.left.end())
		{
			return true;
		}
		out->logs << OUTWARN << permissions << " is not exist";
		return false;
	}
	bool file_info_databases::is_exist_file_permissions(const string& permissions) //判断文件权限是否存在
	{
		if (file_permissions_dictionary.right.find(permissions) != file_permissions_dictionary.right.end())
		{
			return true;
		}
		out->logs << OUTWARN << permissions << " is not exist";
		return false;
	}
}

/*####################### file_info_databases_transfrom #######################*/
namespace opensync
{
	string file_info_databases::transfrom_file_type(const int& type)
	{
		bimap::left_const_iterator it;
		it = file_type_dictionary.left.find(type);
		if (it != file_type_dictionary.left.end())
		{
			//out->logs << OUTDEBUG << type << ":" << it->second;
			return it->second;
		}
		out->logs << OUTWARN << type << " is not exist";
		return "";
	}
	int file_info_databases::transfrom_file_type(const string& type)
	{
		bimap::right_const_iterator it;
		it = file_type_dictionary.right.find(type);
		if (it != file_type_dictionary.right.end())
		{
			//out->logs << OUTDEBUG << type << ":" << it->second;
			return it->second;
		}
		out->logs << OUTWARN << type << " is not exist";
		return -1;
	}
	string file_info_databases::transfrom_file_permissions(const int& permissions) //转换文件权限
	{
		bimap::left_const_iterator it;
		it = file_permissions_dictionary.left.find(permissions);
		if (it != file_permissions_dictionary.left.end())
		{
			//out->logs << OUTDEBUG << permissions << ":" << it->second;
			return it->second;
		}
		out->logs << OUTWARN << permissions << " is not exist";
		return "";
	}
	int file_info_databases::transfrom_file_permissions(const string& permissions) //转换文件权限
	{
		bimap::right_const_iterator it;
		it = file_permissions_dictionary.right.find(permissions);
		if (it != file_permissions_dictionary.right.end())
		{
			//out->logs << OUTDEBUG << permissions << ":" << it->second;
			return it->second;
		}
		out->logs << OUTWARN << permissions << " is not exist";
		return -1;
	}
}