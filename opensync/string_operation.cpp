#include "string_operation.h"

namespace opensync
{
    string_operation::string_operation()
    {
    }
    string_operation::~string_operation()
    {
    }
    const time_t string_operation::to_date(const string& data_str, const string& format)
    {
       tm tm_time;                                    // 定义tm结构体
       int year, month, day, hour, minute, second;    // 定义时间的各个int临时变量
       // 将string存储的日期时间，转换为int临时变量。
       sscanf((char*)data_str.data(), format.c_str(), &year, &month, &day, &hour, &minute, &second);
       tm_time.tm_year = year - 1900;                 // 年，由于tm结构体存储的是从1900年开始的时间，所以tm_year为int临时变量减去1900
       tm_time.tm_mon = month - 1;                    // 月，由于tm结构体的月份存储范围为0-11，所以tm_mon为int临时变量减去1
       tm_time.tm_mday = day;                         // 日
       tm_time.tm_hour = hour;                        // 时
       tm_time.tm_min = minute;                       // 分
       tm_time.tm_sec = second;                       // 秒
       tm_time.tm_isdst = 0;                          // 非夏令时
       time_t time_temp = mktime(&tm_time);           // 将tm结构体转换成time_t格式
       return time_temp;                       // 返回值
    }
}