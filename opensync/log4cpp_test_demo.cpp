#include "log4cpp_test_demo.h"

log4cpp_test_demo::log4cpp_test_demo()
{
	log4cpp::PropertyConfigurator::configure(initFileName);
}

log4cpp_test_demo::~log4cpp_test_demo()
{
	log4cpp::Category::shutdown();
}

void log4cpp_test_demo::action()
{
	logs.debug("Received storm warning");
	logs.info("Closing all hatches");

	logs.debug("Hiding solar panels");
	logs.error("Solar panels are blocked");
}