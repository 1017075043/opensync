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
	};
}