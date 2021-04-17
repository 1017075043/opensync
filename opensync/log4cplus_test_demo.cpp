#include "log4cplus_test_demo.h"

log4cplus_test_demo* log4cplus_test_demo::m_logger = NULL;

log4cplus_test_demo::log4cplus_test_demo()
{
	log4cplus::initialize();
	log4cplus::PropertyConfigurator::doConfigure(LOG4CPLUS_TEXT(MY_LOG_FILE_PATH));

	m_rootLog = log4cplus::Logger::getRoot();

	m_loggerYW = log4cplus::Logger::getInstance("loggerYW");
	m_loggerWJ = log4cplus::Logger::getInstance("loggerWJ");
	m_loggerUI = log4cplus::Logger::getInstance("loggerUI");
}
log4cplus_test_demo::~log4cplus_test_demo()
{
	if (m_logger)
	{
		delete m_logger;
	}
}
log4cplus_test_demo* log4cplus_test_demo::getInstance()
{
	if (m_logger == NULL)
	{
		m_logger = new log4cplus_test_demo();
	}
	return m_logger;
}