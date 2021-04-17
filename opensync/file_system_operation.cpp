#include "file_system_operation.h"

/*####################### file_system_operation #######################*/
namespace opensync
{
	file_system_operation::file_system_operation()
	{
	}
	file_system_operation::~file_system_operation()
	{
	}
	void file_system_operation::init_file_info(const string& file_path) //初始化文件信息
	{
		try
		{
			out->logs << OUTINFO << "create file info:" << file_path;
			boost::filesystem::path p(file_path);
			boost::filesystem::file_status s = boost::filesystem::status(p);

			file_info->data[file_path].file_path = p;
			//out->logs << OUTDEBUG << file_path << ", attribute.file_path=" << file_info->data[file_path].file_path;
			if (!boost::filesystem::is_directory(p))
			{
				file_info->data[file_path].file_size = (long)boost::filesystem::file_size(p);
				out->logs << OUTDEBUG << file_path << ", attribute.file_size=" << file_info->data[file_path].file_size;
				file_info->data[file_path].file_hash = get_file_md5_p(file_path);
				out->logs << OUTDEBUG << file_path << ", attribute.file_hash=" << file_info->data[file_path].file_hash;
			}
			file_info->data[file_path].type = s.type();
			//out->logs << OUTDEBUG << file_path << ", attribute.type=" << file_info->data[file_path].type;
			file_info->data[file_path].type_name = file_info->transfrom_file_type((int)s.type());
			//out->logs << OUTDEBUG << file_path << ", attribute.type_name=" << file_info->data[file_path].type_name;
			file_info->data[file_path].permissions = s.permissions();
			//out->logs << OUTDEBUG << file_path << ", attribute.permissions=" << file_info->data[file_path].permissions;
			file_info->data[file_path].permissions_name = file_info->transfrom_file_permissions((int)s.permissions());
			//out->logs << OUTDEBUG << file_path << ", attribute.permissions_name=" << file_info->data[file_path].permissions_name;
			file_info->data[file_path].last_write_time = boost::filesystem::last_write_time(p);
			//out->logs << OUTDEBUG << file_path << ", attribute.last_write_time=" << file_info->data[file_path].last_write_time;
			file_info->data[file_path].last_write_time_s = boost::posix_time::to_iso_extended_string(boost::posix_time::from_time_t(file_info->data[file_path].last_write_time));
			//out->logs << OUTDEBUG << file_path << ", attribute.last_write_time_s=" << file_info->data[file_path].last_write_time_s;
			file_info->data[file_path].user = get_file_uid(file_path);
			//out->logs << OUTDEBUG << file_path << ", attribute.user=" << file_info->data[file_path].user;
			file_info->data[file_path].user_name = user_group->get_user_name(file_info->data[file_path].user);
			//out->logs << OUTDEBUG << file_path << ", attribute.user_name=" << file_info->data[file_path].user_name;
			file_info->data[file_path].group = get_file_gid(file_path);
			//out->logs << OUTDEBUG << file_path << ", attribute.group=" << file_info->data[file_path].group;
			file_info->data[file_path].group_name = user_group->get_group_name(file_info->data[file_path].group);
			//out->logs << OUTDEBUG << file_path << ", attribute.group_name=" << file_info->data[file_path].group_name;
			file_info->data[file_path].status = true;
			//out->logs << OUTDEBUG << file_path << ", attribute.status=" << file_info->data[file_path].status;
		}
		catch (boost::filesystem::filesystem_error& e)
		{
			file_info->data[file_path].status = false;
			out->logs << OUTERROR << e.what();
		}
		//return file_info->data[file_path];
	}

}

/*####################### file_system_operation_get #######################*/
namespace opensync
{
	const opensync::file_attribute* file_system_operation::get_file_info(const string& file_path) //获取一个文件属性信息
	{
		if (file_info->data.find(file_path) == file_info->data.end())
		{
			init_file_info(file_path);
		}
		return &file_info->data[file_path];
	}
	int file_system_operation::get_file_uid(const string& file_path) //获取文件的uid
	{
		struct stat st;
		try
		{
			if (stat(file_path.c_str(), &st) == -1)
			{
				throw exception() << err_str(file_path + " " + strerror(errno));
			}
			return (int)st.st_uid;
		}
		catch (exception& e)
		{
			out->logs << OUTERROR << *boost::get_error_info<err_str>(e);
			return -1;
		}
	}
	int file_system_operation::get_file_gid(const string& file_path) //获取文件的gid
	{
		struct stat st;
		try
		{
			if (stat(file_path.c_str(), &st) == -1)
			{
				throw exception() << err_str(file_path + " " + strerror(errno));
			}
			return (int)st.st_gid;
		}
		catch (exception& e)
		{
			out->logs << OUTERROR << *boost::get_error_info<err_str>(e);
			return -1;
		}
	}
	string file_system_operation::get_file_md5_p(const string& file_path) //获取文件md5值
	{
		string md5_value;
		try
		{
			std::ifstream file(file_path.c_str(), std::ifstream::binary);
			if (!file)
			{
				throw exception() << err_str(file_path + " " + strerror(errno));
			}
			MD5_CTX md5Context;
			MD5_Init(&md5Context);

			char buf[1024 * 16];
			while (file.good())
			{
				file.read(buf, sizeof(buf));
				MD5_Update(&md5Context, buf, file.gcount());
			}

			unsigned char result[MD5_DIGEST_LENGTH];
			MD5_Final(result, &md5Context);

			char hex[35];
			memset(hex, 0, sizeof(hex));
			for (int i = 0; i < MD5_DIGEST_LENGTH; ++i)
			{
				sprintf(hex + i * 2, "%02x", result[i]);
			}
			hex[32] = '\0';
			md5_value = string(hex);
		}
		catch (exception& e)
		{
			out->logs << OUTERROR << *boost::get_error_info<err_str>(e);
		}
		return md5_value;
	}
	const vector<const opensync::file_attribute*> file_system_operation::get_file_and_dir_traverse_all_list(const string& file_path)  //遍历获取一个目录或目录及其下级所有类型文件的属性信息列表
	{
		typedef boost::filesystem::recursive_directory_iterator rd_iterator;
		vector<const opensync::file_attribute*> file_and_son_info;
		try
		{
			boost::filesystem::path p(file_path);
			if (!exists(p))
			{
				throw exception() << err_str(file_path + " " + strerror(errno));
			}
			if (file_info->data.find(file_path) == file_info->data.end())
			{
				init_file_info(file_path);
			}
			file_and_son_info.push_back(&file_info->data[file_path]);
			if (boost::filesystem::is_directory(p))
			{
				rd_iterator end;
				for (rd_iterator pos(p); pos != end; ++pos)
				{
					get_file_info((const string&)pos->path());
					file_and_son_info.push_back(&file_info->data[(const string&)pos->path()]);
				}
			}
		}
		catch (exception& e)
		{
			out->logs << OUTERROR << *boost::get_error_info<err_str>(e);
		}
		return file_and_son_info;
	}
	const vector<const opensync::file_attribute*> file_system_operation::get_file_and_dir_traverse_dir_list(const string& file_path)  //遍历获取一个目录或目录及其下级目录类型文件的属性信息列表
	{
		typedef boost::filesystem::recursive_directory_iterator rd_iterator;
		vector<const opensync::file_attribute*> file_and_son_info;
		try
		{
			boost::filesystem::path p(file_path);
			if (!exists(p))
			{
				throw exception() << err_str(file_path + " " + strerror(errno));
			}
			if (file_info->data.find(file_path) == file_info->data.end())
			{
				init_file_info(file_path);
			}
			if (boost::filesystem::is_directory(p))
			{
				file_and_son_info.push_back(&file_info->data[file_path]);
				rd_iterator end;
				for (rd_iterator pos(p); pos != end; ++pos)
				{
					get_file_info((const string&)pos->path());
					if (file_info->data[(const string&)pos->path()].type == 3)
					{
						file_and_son_info.push_back(&file_info->data[(const string&)pos->path()]);
					}
				}
			}
		}
		catch (exception& e)
		{
			out->logs << OUTERROR << *boost::get_error_info<err_str>(e);
		}
		return file_and_son_info;
	}
	const vector<const opensync::file_attribute*> file_system_operation::get_file_and_dir_traverse_file_list(const string& file_path)  //遍历获取一个目录或目录及其下级非目录类型文件的属性信息列表
	{
		typedef boost::filesystem::recursive_directory_iterator rd_iterator;
		vector<const opensync::file_attribute*> file_and_son_info;
		try
		{
			boost::filesystem::path p(file_path);
			if (!exists(p))
			{
				throw exception() << err_str(file_path + " " + strerror(errno));
			}
			if (file_info->data.find(file_path) == file_info->data.end())
			{
				init_file_info(file_path);
			}
			if (boost::filesystem::is_directory(p))
			{
				rd_iterator end;
				for (rd_iterator pos(p); pos != end; ++pos)
				{
					get_file_info((const string&)pos->path());
					if (file_info->data[(const string&)pos->path()].type != 3)
					{
						file_and_son_info.push_back(&file_info->data[(const string&)pos->path()]);
					}
				}
			}
			else
			{
				file_and_son_info.push_back(&file_info->data[file_path]);
			}
		}
		catch (exception& e)
		{
			out->logs << OUTERROR << *boost::get_error_info<err_str>(e);
		}
		return file_and_son_info;
	}
	const path file_system_operation::get_file_path(const string& file_path) //获取文件file_path
	{
		if (file_info->data.find(file_path) == file_info->data.end())
		{
			init_file_info(file_path);
		}
		if (file_info->data[file_path].status)
		{
			return file_info->data[file_path].file_path;
		}
		return (path)"";
	}
	const long file_system_operation::get_file_size(const string& file_path) //获取文件file_size
	{
		if (file_info->data.find(file_path) == file_info->data.end())
		{
			init_file_info(file_path);
		}
		if (file_info->data[file_path].status)
		{
			return file_info->data[file_path].file_size;
		}
		return (long)-1;
	}
	const file_type file_system_operation::get_file_type(const string& file_path) //获取文件file_type
	{
		if (file_info->data.find(file_path) == file_info->data.end())
		{
			init_file_info(file_path);
		}
		if (file_info->data[file_path].status)
		{
			return file_info->data[file_path].type;
		}
		return (file_type)0;
	}
	const string file_system_operation::get_file_type_name(const string& file_path) //获取文件file_type_name
	{
		if (file_info->data.find(file_path) == file_info->data.end())
		{
			init_file_info(file_path);
		}
		if (file_info->data[file_path].status)
		{
			return file_info->data[file_path].type_name;
		}
		return "";
	}
	const perms file_system_operation::get_file_permissions(const string& file_path) //获取文件file_permissions
	{
		if (file_info->data.find(file_path) == file_info->data.end())
		{
			init_file_info(file_path);
		}
		if (file_info->data[file_path].status)
		{
			return file_info->data[file_path].permissions;
		}
		return (perms)-1;
	}
	const string file_system_operation::get_file_permissions_name(const string& file_path) //获取文件file_permissions_name
	{
		if (file_info->data.find(file_path) == file_info->data.end())
		{
			init_file_info(file_path);
		}
		if (file_info->data[file_path].status)
		{
			return file_info->data[file_path].permissions_name;
		}
		return "";
	}
	const time_t file_system_operation::get_file_last_write_time(const string& file_path) //获取文件file_last_write_time
	{
		if (file_info->data.find(file_path) == file_info->data.end())
		{
			init_file_info(file_path);
		}
		if (file_info->data[file_path].status)
		{
			return file_info->data[file_path].last_write_time;
		}
		return (time_t)-1;
	}
	const string file_system_operation::get_file_last_write_time_s(const string& file_path) //获取文件file_last_write_time_s
	{
		if (file_info->data.find(file_path) == file_info->data.end())
		{
			init_file_info(file_path);
		}
		if (file_info->data[file_path].status)
		{
			return file_info->data[file_path].last_write_time_s;
		}
		return "";
	}
	const int file_system_operation::get_file_user(const string& file_path) //获取文件file_user
	{
		if (file_info->data.find(file_path) == file_info->data.end())
		{
			init_file_info(file_path);
		}
		if (file_info->data[file_path].status)
		{
			return file_info->data[file_path].user;
		}
		return (int)-1;
	}
	const string file_system_operation::get_file_user_name(const string& file_path) //获取文件file_user_name
	{
		if (file_info->data.find(file_path) == file_info->data.end())
		{
			init_file_info(file_path);
		}
		if (file_info->data[file_path].status)
		{
			return file_info->data[file_path].user_name;
		}
		return "";
	}
	const int file_system_operation::get_file_group(const string& file_path) //获取文件file_group
	{
		if (file_info->data.find(file_path) == file_info->data.end())
		{
			init_file_info(file_path);
		}
		if (file_info->data[file_path].status)
		{
			return file_info->data[file_path].group;
		}
		return (int)-1;
	}
	const string file_system_operation::get_file_group_name(const string& file_path) //获取文件file_group_name
	{
		if (file_info->data.find(file_path) == file_info->data.end())
		{
			init_file_info(file_path);
		}
		if (file_info->data[file_path].status)
		{
			return file_info->data[file_path].group_name;
		}
		return "";
	}
	const string file_system_operation::get_file_hash(const string& file_path) //获取文件file_hash
	{
		if (file_info->data.find(file_path) == file_info->data.end())
		{
			init_file_info(file_path);
		}
		if (file_info->data[file_path].status)
		{
			return file_info->data[file_path].file_hash;
		}
		return "";
	}
	const bool file_system_operation::get_file_status(const string& file_path) //获取文件file_status
	{
		if (file_info->data.find(file_path) == file_info->data.end())
		{
			init_file_info(file_path);
		}
		return file_info->data[file_path].status;
	}
}

/*####################### file_system_operation_set #######################*/
namespace opensync
{
	bool file_system_operation::set_file_permissions(const string& file_path, const perms& permissions)//设置文件file_permissions
	{
		if (!file_info->is_exist_file_permissions(permissions))
		{
			out->logs << OUTERROR << permissions << "is not exist file permissions";
			return false;
		}
		if (file_info->data[file_path].permissions == permissions)
		{
			return true;
		}
		if (file_info->data.find(file_path) == file_info->data.end())
		{
			init_file_info(file_path);
		}
		if (file_info->data[file_path].status)
		{
			try
			{
				if (chmod(file_path.c_str(), permissions) == -1)
				{
					throw exception() << err_str(file_path + " " + strerror(errno));
				}
				file_info->data[file_path].permissions = permissions;
				file_info->data[file_path].permissions_name = file_info->transfrom_file_permissions(file_info->data[file_path].permissions);
			}
			catch (exception& e)
			{
				out->logs << OUTERROR << *boost::get_error_info<err_str>(e);
			}
		}
		out->logs << OUTDEBUG << "set_file_permissions " << file_path << ":" << file_info->data[file_path].permissions;
		return file_info->data[file_path].status;
	}
	bool file_system_operation::set_file_permissions(const string& file_path, const string& permissions_name)//设置文件file_permissions_name
	{
		if (!file_info->is_exist_file_permissions(permissions_name))
		{
			out->logs << OUTERROR << permissions_name << "is not exist file permissions";
			return false;
		}
		if (file_info->data[file_path].permissions_name == permissions_name)
		{
			return true;
		}
		if (file_info->data.find(file_path) == file_info->data.end())
		{
			init_file_info(file_path);
		}
		if (file_info->data[file_path].status)
		{
			try
			{
				if (chmod(file_path.c_str(), (perms)file_info->transfrom_file_permissions(permissions_name)) == -1)
				{
					throw exception() << err_str(file_path + " " + strerror(errno));
				}
				file_info->data[file_path].permissions = (perms)file_info->transfrom_file_permissions(permissions_name);
				file_info->data[file_path].permissions_name = permissions_name;
			}
			catch (exception& e)
			{
				out->logs << OUTERROR << *boost::get_error_info<err_str>(e);
			}
		}
		out->logs << OUTDEBUG << "set_file_permissions " << file_path << ":" << file_info->data[file_path].permissions_name;
		return file_info->data[file_path].status;
	}
	bool file_system_operation::set_file_last_write_time(const string& file_path, const time_t& last_write_time) //设置文件file_last_write_time
	{
		if (file_info->data.find(file_path) == file_info->data.end())
		{
			init_file_info(file_path);
		}
		if (file_info->data[file_path].status)
		{
			boost::filesystem::last_write_time(file_info->data[file_path].file_path, last_write_time);
			file_info->data[file_path].last_write_time = last_write_time;
			file_info->data[file_path].last_write_time_s = boost::posix_time::to_iso_extended_string(boost::posix_time::from_time_t(file_info->data[file_path].last_write_time));
		}
		out->logs << OUTDEBUG << "set_file_last_write_time " << file_path << ":" << file_info->data[file_path].last_write_time;
		return file_info->data[file_path].status;
	}
	bool file_system_operation::set_file_last_write_time_s(const string& file_path, const string& last_write_time_s) //设置文件file_last_write_time_s
	{
		return set_file_last_write_time(file_path, opensync::string_operation::to_date(last_write_time_s, "%d-%d-%dT%d:%d:%d"));
	}
	bool file_system_operation::set_file_user(const string& file_path, const int& user)//设置文件file_permissions
	{
		if (!user_group->is_exist_user_id(user))
		{
			out->logs << OUTERROR << user << "is not exist user id";
			return false;
		}
		if (file_info->data[file_path].user == user)
		{
			return true;
		}
		if (file_info->data.find(file_path) == file_info->data.end())
		{
			init_file_info(file_path);
		}
		if (file_info->data[file_path].status)
		{
			try
			{
				if (chown(file_path.c_str(), (uid_t)user, (uid_t)file_info->data[file_path].group) == -1)
				{
					throw exception() << err_str(file_path + " " + strerror(errno));
				}
				file_info->data[file_path].user = user;
				file_info->data[file_path].user_name = user_group->get_user_name(file_info->data[file_path].user);
			}
			catch (exception& e)
			{
				out->logs << OUTERROR << *boost::get_error_info<err_str>(e);
			}
		}
		out->logs << OUTDEBUG << "set_file_user " << file_path << ":" << file_info->data[file_path].user;
		return file_info->data[file_path].status;
	}
	bool file_system_operation::set_file_user(const string& file_path, const string& user_name)//设置文件file_permissions_name
	{
		if (!user_group->is_exist_user_name(user_name))
		{
			out->logs << OUTERROR << user_name << "is not exist user name";
			return false;
		}
		if (file_info->data[file_path].user_name == user_name)
		{
			return true;
		}
		if (file_info->data.find(file_path) == file_info->data.end())
		{
			init_file_info(file_path);
		}
		if (file_info->data[file_path].status)
		{
			try
			{
				if (chown(file_path.c_str(), (uid_t)user_group->get_user_id(user_name), (uid_t)file_info->data[file_path].group) == -1)
				{
					throw exception() << err_str(file_path + " " + strerror(errno));
				}
				file_info->data[file_path].user = user_group->get_user_id(user_name);
				file_info->data[file_path].user_name = user_name;
			}
			catch (exception& e)
			{
				out->logs << OUTERROR << *boost::get_error_info<err_str>(e);
			}
		}
		out->logs << OUTDEBUG << "set_file_user " << file_path << ":" << file_info->data[file_path].user_name;
		return file_info->data[file_path].status;
	}
	bool file_system_operation::set_file_group(const string& file_path, const int& group)//设置文件file_user
	{
		if (!user_group->is_exist_group_id(group))
		{
			out->logs << OUTERROR << group << "is not exist group id";
			return false;
		}
		if (file_info->data[file_path].group == group)
		{
			return true;
		}
		if (file_info->data.find(file_path) == file_info->data.end())
		{
			init_file_info(file_path);
		}
		if (file_info->data[file_path].status)
		{
			try
			{
				if (chown(file_path.c_str(), (uid_t)file_info->data[file_path].user, (uid_t)group) == -1)
				{
					throw exception() << err_str(file_path + " " + strerror(errno));
				}
				file_info->data[file_path].group = group;
				file_info->data[file_path].group_name = user_group->get_user_name(file_info->data[file_path].group);
			}
			catch (exception& e)
			{
				out->logs << OUTERROR << *boost::get_error_info<err_str>(e);
			}
		}
		out->logs << OUTDEBUG << "set_file_group " << file_path << ":" << file_info->data[file_path].group;
		return file_info->data[file_path].status;
	}
	bool file_system_operation::set_file_group(const string& file_path, const string& group_name)//设置文件file_group_name
	{
		if (!user_group->is_exist_group_name(group_name))
		{
			out->logs << OUTERROR << group_name << "is not exist group name";
			return false;
		}
		if (file_info->data[file_path].group_name == group_name)
		{
			return true;
		}
		if (file_info->data.find(file_path) == file_info->data.end())
		{
			init_file_info(file_path);
		}
		if (file_info->data[file_path].status)
		{
			try
			{
				if (chown(file_path.c_str(), (uid_t)file_info->data[file_path].user, (uid_t)user_group->get_group_id(group_name)) == -1)
				{
					throw exception() << err_str(file_path + " " + strerror(errno));
				}
				file_info->data[file_path].group = user_group->get_group_id(group_name);
				file_info->data[file_path].group_name = group_name;
			}
			catch (exception& e)
			{
				out->logs << OUTERROR << *boost::get_error_info<err_str>(e);
			}
		}
		out->logs << OUTDEBUG << "set_file_group " << file_path << ":" << file_info->data[file_path].group_name;
		return file_info->data[file_path].status;
	}
	bool file_system_operation::set_file_user_group(const string& file_path, const int& user, const int& group)//设置文件file_user和file_user_name
	{
		if (!user_group->is_exist_user_id(user))
		{
			out->logs << OUTERROR << user << "is not exist user id";
			return false;
		}
		if (!user_group->is_exist_group_id(group))
		{
			out->logs << OUTERROR << group << "is not exist group id";
			return false;
		}
		if (file_info->data[file_path].user == user && file_info->data[file_path].group == group)
		{
			return true;
		}
		if (file_info->data.find(file_path) == file_info->data.end())
		{
			init_file_info(file_path);
		}
		if (file_info->data[file_path].status)
		{
			try
			{
				if (chown(file_path.c_str(), (uid_t)user, (uid_t)group) == -1)
				{
					throw exception() << err_str(file_path + " " + strerror(errno));
				}
				file_info->data[file_path].user = user;
				file_info->data[file_path].user_name = user_group->get_user_name(user);
				file_info->data[file_path].group = group;
				file_info->data[file_path].group_name = user_group->get_user_name(group);
			}
			catch (exception& e)
			{
				out->logs << OUTERROR << *boost::get_error_info<err_str>(e);
			}
		}
		out->logs << OUTDEBUG << "set_file_user " << file_path << ":" << file_info->data[file_path].user << "," << file_info->data[file_path].group;
		return file_info->data[file_path].status;
	}
	bool file_system_operation::set_file_user_group(const string& file_path, const string& user_name, const string& group_name)//设置文件file_user_name和file_user_name
	{
		if (!user_group->is_exist_user_name(user_name))
		{
			out->logs << OUTERROR << user_name << "is not exist user name";
			return false;
		}
		if (!user_group->is_exist_group_name(group_name))
		{
			out->logs << OUTERROR << group_name << "is not exist group name";
			return false;
		}
		if (file_info->data[file_path].user_name == user_name && file_info->data[file_path].group_name == group_name)
		{
			return true;
		}
		if (file_info->data.find(file_path) == file_info->data.end())
		{
			init_file_info(file_path);
		}
		if (file_info->data[file_path].status)
		{
			try
			{
				if (chown(file_path.c_str(), (uid_t)user_group->get_group_id(user_name), (uid_t)user_group->get_group_id(group_name)) == -1)
				{
					throw exception() << err_str(file_path + " " + strerror(errno));
				}
				file_info->data[file_path].user = user_group->get_group_id(user_name);
				file_info->data[file_path].user_name = user_name;
				file_info->data[file_path].group = user_group->get_group_id(group_name);
				file_info->data[file_path].group_name = group_name;
			}
			catch (exception& e)
			{
				out->logs << OUTERROR << *boost::get_error_info<err_str>(e);
			}
		}
		out->logs << OUTDEBUG << "set_file_user " << file_path << ":" << file_info->data[file_path].user_name << "," << file_info->data[file_path].group_name;
		return file_info->data[file_path].status;
	}

}

/*####################### file_system_operation_show #######################*/
namespace opensync
{
	void file_system_operation::show_file_info_p(const string& file_path) //显示文件的属性（私用）
	{
		out->logs << OUTDEBUG << file_path << ", attribute.file_path=" << file_info->data[file_path].file_path;
		out->logs << OUTDEBUG << file_path << ", attribute.file_size=" << file_info->data[file_path].file_size;
		out->logs << OUTDEBUG << file_path << ", attribute.type=" << file_info->data[file_path].type;
		out->logs << OUTDEBUG << file_path << ", attribute.type_name=" << file_info->data[file_path].type_name;
		out->logs << OUTDEBUG << file_path << ", attribute.permissions=" << file_info->data[file_path].permissions;
		out->logs << OUTDEBUG << file_path << ", attribute.permissions_name=" << file_info->data[file_path].permissions_name;
		out->logs << OUTDEBUG << file_path << ", attribute.last_write_time=" << file_info->data[file_path].last_write_time;
		out->logs << OUTDEBUG << file_path << ", attribute.last_write_time_s=" << file_info->data[file_path].last_write_time_s;
		out->logs << OUTDEBUG << file_path << ", attribute.user=" << file_info->data[file_path].user;
		out->logs << OUTDEBUG << file_path << ", attribute.user_name=" << file_info->data[file_path].user_name;
		out->logs << OUTDEBUG << file_path << ", attribute.group=" << file_info->data[file_path].group;
		out->logs << OUTDEBUG << file_path << ", attribute.group_name=" << file_info->data[file_path].group_name;
		out->logs << OUTDEBUG << file_path << ", attribute.file_hash=" << file_info->data[file_path].file_hash;
		out->logs << OUTDEBUG << file_path << ", attribute.status=" << file_info->data[file_path].status;
	}
	void file_system_operation::show_file_info(const string& file_path) //显示文件的属性
	{
		if (file_info->data.find(file_path) == file_info->data.end())
		{
			init_file_info(file_path);
		}
		if (file_info->data[file_path].status)
		{
			show_file_info_p(file_path);
		}
	}
	void file_system_operation::show_file_info_databases() //显示目前文件信息库中存储文件的信息
	{
		map<string, file_attribute>::iterator it;
		for (it = file_info->data.begin(); it != file_info->data.end(); it++)
		{
			show_file_info_p(it->first);
		}
	}
}

/*####################### file_system_operation_update #######################*/
namespace opensync
{
	void file_system_operation::update_file_info(const string& file_path) //更新文件信息
	{
		init_file_info(file_path);
	}
	bool file_system_operation::update_file_size(const string& file_path, const long& file_size) //更新文件file_size
	{
		if (file_info->data.find(file_path) == file_info->data.end())
		{
			init_file_info(file_path);
		}
		else if (file_info->data[file_path].status)
		{
			file_info->data[file_path].file_size = file_size;
		}
		out->logs << OUTDEBUG << "update_file_size " << file_path << ":" << file_info->data[file_path].file_size;
		return file_info->data[file_path].status;
	}
	bool file_system_operation::update_file_type(const string& file_path, const file_type& type) //更新文件file_type
	{
		if (!file_info->is_exist_file_type(type))
		{
			return false;
		}
		if (file_info->data.find(file_path) == file_info->data.end())
		{
			init_file_info(file_path);
		}
		else if (file_info->data[file_path].status)
		{
			file_info->data[file_path].type = type;
			file_info->data[file_path].type_name = file_info->transfrom_file_type(file_info->data[file_path].type);
		}
		out->logs << OUTDEBUG << "update_file_type " << file_path << ":" << file_info->data[file_path].type;
		return file_info->data[file_path].status;
	}
	bool file_system_operation::update_file_type_name(const string& file_path, const string& type_name) //更新文件file_type_name
	{
		if (!file_info->is_exist_file_type(type_name))
		{
			return false;
		}
		if (file_info->data.find(file_path) == file_info->data.end())
		{
			init_file_info(file_path);
		}
		else if (file_info->data[file_path].status)
		{
			file_info->data[file_path].type = (file_type)file_info->transfrom_file_type(file_info->data[file_path].type_name);
			file_info->data[file_path].type_name = type_name;
		}
		out->logs << OUTDEBUG << "update_file_type_name " << file_path << ":" << file_info->data[file_path].type_name;
		return file_info->data[file_path].status;
	}
	bool file_system_operation::update_file_permissions(const string& file_path, const perms& permissions) //更新文件file_permissions
	{
		if (!file_info->is_exist_file_permissions(permissions))
		{
			return false;
		}
		if (file_info->data.find(file_path) == file_info->data.end())
		{
			init_file_info(file_path);
		}
		else if (file_info->data[file_path].status)
		{
			file_info->data[file_path].permissions = permissions;
			file_info->data[file_path].type_name = file_info->transfrom_file_permissions(file_info->data[file_path].permissions);
		}
		out->logs << OUTDEBUG << "update_file_permissions " << file_path << ":" << file_info->data[file_path].permissions;
		return file_info->data[file_path].status;
	}
	bool file_system_operation::update_file_permissions_name(const string& file_path, const string& permissions_name) //更新文件file_permissions_name
	{
		if (!file_info->is_exist_file_permissions(permissions_name))
		{
			return false;
		}
		if (file_info->data.find(file_path) == file_info->data.end())
		{
			init_file_info(file_path);
		}
		else if (file_info->data[file_path].status)
		{
			file_info->data[file_path].permissions = (perms)file_info->transfrom_file_type(permissions_name);
			file_info->data[file_path].type_name = permissions_name;
		}
		out->logs << OUTDEBUG << "update_file_permissions_name " << file_path << ":" << file_info->data[file_path].permissions_name;
		return file_info->data[file_path].status;
	}
	bool file_system_operation::update_file_last_write_time(const string& file_path, const time_t& last_write_time) //更新文件file_last_write_time
	{
		if (file_info->data.find(file_path) == file_info->data.end())
		{
			init_file_info(file_path);
		}
		else if (file_info->data[file_path].status)
		{
			file_info->data[file_path].last_write_time = last_write_time;
			file_info->data[file_path].last_write_time_s = boost::posix_time::to_iso_extended_string(boost::posix_time::from_time_t(file_info->data[file_path].last_write_time));
		}
		out->logs << OUTDEBUG << "update_file_last_write_time " << file_path << ":" << file_info->data[file_path].last_write_time;
		return file_info->data[file_path].status;
	}
	bool file_system_operation::update_file_last_write_time_s(const string& file_path, const string& last_write_time_s) //更新文件file_last_write_time_s
	{
		return update_file_last_write_time(file_path, opensync::string_operation::to_date(last_write_time_s, "%d-%d-%dT%d:%d:%d"));
	}
	bool file_system_operation::update_file_user(const string& file_path, const int& user) //更新文件file_user
	{
		if (!user_group->is_exist_group_id(user))
		{
			return false;
		}
		if (file_info->data.find(file_path) == file_info->data.end())
		{
			init_file_info(file_path);
		}
		else if (file_info->data[file_path].status)
		{
			file_info->data[file_path].user = user;
			file_info->data[file_path].user_name = user_group->get_user_name(file_info->data[file_path].user);
		}
		out->logs << OUTDEBUG << "update_file_user " << file_path << ":" << file_info->data[file_path].user;
		return file_info->data[file_path].status;
	}
	bool file_system_operation::update_file_user_name(const string& file_path, const string& user_name) //更新文件file_user_name
	{
		if (!user_group->is_exist_group_name(user_name))
		{
			return false;
		}
		if (file_info->data.find(file_path) == file_info->data.end())
		{
			init_file_info(file_path);
		}
		else if (file_info->data[file_path].status)
		{
			file_info->data[file_path].user = user_group->get_user_id(user_name);
			file_info->data[file_path].user_name = user_name;
		}
		out->logs << OUTDEBUG << "update_file_user_name " << file_path << ":" << file_info->data[file_path].user_name;
		return file_info->data[file_path].status;
	}
	bool file_system_operation::update_file_group(const string& file_path, const int& group) //更新文件file_group
	{
		if (!user_group->is_exist_group_id(group))
		{
			return false;
		}
		if (file_info->data.find(file_path) == file_info->data.end())
		{
			init_file_info(file_path);
		}
		else if (file_info->data[file_path].status)
		{
			file_info->data[file_path].group = group;
			file_info->data[file_path].group_name = user_group->get_group_name(file_info->data[file_path].group);
		}
		out->logs << OUTDEBUG << "update_file_group " << file_path << ":" << file_info->data[file_path].group;
		return file_info->data[file_path].status;
	}
	bool file_system_operation::update_file_group_name(const string& file_path, const string& group_name) //更新文件file_group_name
	{
		if (!user_group->is_exist_group_name(group_name))
		{
			return false;
		}
		if (file_info->data.find(file_path) == file_info->data.end())
		{
			init_file_info(file_path);
		}
		else if (file_info->data[file_path].status)
		{
			file_info->data[file_path].group = user_group->get_group_id(group_name);
			file_info->data[file_path].group_name = group_name;
		}
		out->logs << OUTDEBUG << "update_file_group_name " << file_path << ":" << file_info->data[file_path].group_name;
		return file_info->data[file_path].status;
	}
	bool file_system_operation::update_file_hash(const string& file_path, const string& file_hash) //更新文件file_hash
	{
		if (file_info->data.find(file_path) == file_info->data.end())
		{
			init_file_info(file_path);
		}
		else if (file_info->data[file_path].status)
		{
			file_info->data[file_path].file_hash = file_hash;
		}
		out->logs << OUTDEBUG << "update_file_hash " << file_path << ":" << file_info->data[file_path].file_hash;
		return file_info->data[file_path].status;
	}
	bool file_system_operation::update_file_status(const string& file_path, const bool& status) //更新文件file_status
	{
		if (file_info->data.find(file_path) == file_info->data.end())
		{
			init_file_info(file_path);
		}
		else if (file_info->data[file_path].status)
		{
			file_info->data[file_path].status = status;
		}
		out->logs << OUTDEBUG << "update_file_status " << file_path << ":" << file_info->data[file_path].status;
		return file_info->data[file_path].status;
	}
}