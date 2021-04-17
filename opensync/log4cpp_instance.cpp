#include "log4cpp_instance.h"

namespace opensync
{
    log4cpp_instance* log4cpp_instance::instance = nullptr;

    log4cpp_instance::log4cpp_instance() {
		//cout << "create out project" << endl;
        try
        {
            log4cpp::PropertyConfigurator::configure(opensync_log4cpp_conf);
        }
        catch (log4cpp::ConfigureFailure& f)
        {
            cout << f.what() << endl;
        }
	}
    log4cpp_instance::~log4cpp_instance()
    {
    }

    log4cpp_instance* log4cpp_instance::get_instance() 
    {
        return instance;
    }
    log4cpp_instance* log4cpp_instance::init_instance() 
    {
        if (instance == nullptr) {
            instance = new log4cpp_instance();
        }
        return instance;
    }
    void log4cpp_instance::destory() 
    {
        if (instance != nullptr) 
        {
            delete instance;
            instance = nullptr;
            log4cpp::Category::shutdown();
        }
    }
}