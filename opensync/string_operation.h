#pragma once
#include "opensync_public_include_file.h"

#include <vector>
#include <map>
#include <time.h>
#include <ctime>
#include <sstream>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/bimap.hpp>
#include <boost/noncopyable.hpp>
#include <boost/format.hpp>
#include "boost/assign.hpp"

/*
	字符串操作类
	@author 吴南辉
	@time 2020/06/06
*/

using namespace std;

namespace opensync
{
	class string_operation
	{
	private:
		opensync::log4cpp_instance* out = opensync::log4cpp_instance::init_instance();
	private:
		string_operation();
		~string_operation();

	public:
		static const time_t to_date(const string& data_str, const string& format); //将字符串按照指定的格式转化为时间戳
		static unsigned char to_hex(unsigned char x); //url解码与编码
		static unsigned char from_hex(unsigned char x); //url解码与编码
		static string url_encode(const std::string& str);  //url编码
		static string url_decode(const std::string& str);  //url解码
		static void get_post_params(std::map<std::string, std::string>& params, const std::string& str);  //获取post参数
	};
}