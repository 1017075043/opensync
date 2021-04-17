#pragma once
#include <iostream>

#include <log4cpp/Category.hh>
#include <log4cpp/PropertyConfigurator.hh>

using namespace std;

class log4cpp_test_demo
{
private:
	string initFileName = "/home/wnh/projects/opensync/log4cpp.properties";
	log4cpp::Category& logs = log4cpp::Category::getInstance(std::string("sub1"));
public:
	log4cpp_test_demo();
	~log4cpp_test_demo();
	void action();
	
};

