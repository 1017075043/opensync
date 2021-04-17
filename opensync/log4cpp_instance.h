#pragma once
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <log4cpp/Category.hh>
#include <log4cpp/PropertyConfigurator.hh>
#include <log4cpp/CategoryStream.hh>
#include <boost/noncopyable.hpp>

#include "opensync_public_define.h"

using namespace std;

/*
    日志类（单例）
    @author 吴南辉
    @time 2020/05/30
*/

#define OUTEMERG log4cpp::Priority::EMERG << "[" << __FILE__ << "] [" << __func__ << "] [" << __LINE__ << "] "
#define OUTFATAL log4cpp::Priority::FATAL << "[" << __FILE__ << "] [" << __func__ << "] [" << __LINE__ << "] "
#define OUTALERT log4cpp::Priority::ALERT << "[" << __FILE__ << "] [" << __func__ << "] [" << __LINE__ << "] "
#define OUTCRIT log4cpp::Priority::CRIT << "[" << __FILE__ << "] [" << __func__ << "] [" << __LINE__ << "] "
#define OUTERROR log4cpp::Priority::ERROR << "[" << __FILE__ << "] [" << __func__ << "] [" << __LINE__ << "] "
#define OUTWARN log4cpp::Priority::WARN << "[" << __FILE__ << "] [" << __func__ << "] [" << __LINE__ << "] "
#define OUTNOTICE log4cpp::Priority::NOTICE << "[" << __FILE__ << "] [" << __func__ << "] [" << __LINE__ << "] "
#define OUTINFO log4cpp::Priority::INFO << "[" << __FILE__ << "] [" << __func__ << "] [" << __LINE__ << "] "
#define OUTDEBUG log4cpp::Priority::DEBUG << "[" << __FILE__ << "] [" << __func__ << "] [" << __LINE__ << "] "
#define OUTNOTSET log4cpp::Priority::NOTSET << "[" << __FILE__ << "] [" << __func__ << "] [" << __LINE__ << "] "

//#define OUTEMERG log4cpp::Priority::EMERG 
//#define OUTFATAL log4cpp::Priority::FATAL 
//#define OUTALERT log4cpp::Priority::ALERT 
//#define OUTCRIT log4cpp::Priority::CRIT 
//#define OUTERROR log4cpp::Priority::ERROR 
//#define OUTWARN log4cpp::Priority::WARN 
//#define OUTNOTICE log4cpp::Priority::NOTICE 
//#define OUTINFO log4cpp::Priority::INFO 
//#define OUTDEBUG log4cpp::Priority::DEBUG 
//#define OUTNOTSET log4cpp::Priority::NOTSET

#define opensync_log4cpp_conf "/home/wnh/projects/opensync/opensync_log4cpp.conf"

namespace opensync
{
    class log4cpp_instance : boost::noncopyable
    {
    private:
        static log4cpp_instance* instance;
    private:
        log4cpp_instance();
        ~log4cpp_instance();
    public:
        log4cpp::Category& logs = log4cpp::Category::getInstance(std::string("sub1"));
    public:
        static log4cpp_instance* get_instance();
        static log4cpp_instance* init_instance();
        static void destory();
    };
}