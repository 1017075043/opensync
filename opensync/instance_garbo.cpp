#include "instance_garbo.h"

namespace opensync
{
	instance_garbo::instance_garbo()
	{
	}
	instance_garbo::~instance_garbo()
	{
		opensync::log4cpp_instance::destory();
		opensync::user_group_info::destory();
		opensync::file_info_databases::destory();
		opensync::file_system_listen::destory();
	}
}