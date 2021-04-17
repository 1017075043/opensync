#include <log4cplus/logger.h>
#include <log4cplus/configurator.h> 
#include <log4cplus/layout.h> 
#include <log4cplus/loggingmacros.h> 
#include <log4cplus/helpers/stringhelper.h> 

#define MY_LOG_FILE_PATH "/home/wnh/projects/opensync/log4cplus.properties"

class log4cplus_test_demo
{
private:
	log4cplus_test_demo();
	~log4cplus_test_demo();

	static log4cplus_test_demo* m_logger;

public:
	static log4cplus_test_demo* getInstance();

	log4cplus::Logger m_rootLog;

	log4cplus::Logger m_loggerYW;
	log4cplus::Logger m_loggerWJ;
	log4cplus::Logger m_loggerUI;
};