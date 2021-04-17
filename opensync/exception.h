#pragma once

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <boost/exception/all.hpp>

using namespace std;

/*
	“Ï≥£¿‡
	@author Œ‚ƒœª‘
	@time 2020/05/30
*/

namespace opensync
{
	class exception : virtual public std::exception, virtual public boost::exception
	{
	};
	typedef boost::error_info<struct tag_err_no, int> err_no;
	typedef boost::error_info<struct tag_err_str, std::string> err_str;
}

