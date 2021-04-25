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
    // url encode 和url decode
    unsigned char string_operation::to_hex(unsigned char x)
    {
        return  x > 9 ? x + 55 : x + 48;
    }
    // url encode 和url decode
    unsigned char string_operation::from_hex(unsigned char x)
    {
        unsigned char y;
        if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
        else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
        else if (x >= '0' && x <= '9') y = x - '0';
        else assert(0);
        return y;
    }
    // url encode 和url decode
    string string_operation::url_encode(const std::string& str)
    {
        std::string str_temp = "";
        size_t length = str.length();
        for (size_t i = 0; i < length; i++)
        {
            if (isalnum((unsigned char)str[i]) ||
                (str[i] == '-') ||
                (str[i] == '_') ||
                (str[i] == '.') ||
                (str[i] == '~'))
                str_temp += str[i];
            else if (str[i] == ' ')
                str_temp += "+";
            else
            {
                str_temp += '%';
                str_temp += to_hex((unsigned char)str[i] >> 4);
                str_temp += to_hex((unsigned char)str[i] % 16);
            }
        }
        return str_temp;
    }
    // url encode 和url decode
    string string_operation::url_decode(const std::string& str)
    {
        string str_temp = "";
        size_t length = str.length();
        for (size_t i = 0; i < length; i++)
        {
            if (str[i] == '+') str_temp += ' ';
            else if (str[i] == '%')
            {
                assert(i + 2 < length);
                unsigned char high = from_hex((unsigned char)str[++i]);
                unsigned char low = from_hex((unsigned char)str[++i]);
                str_temp += high * 16 + low;
            }
            else str_temp += str[i];
        }
        return str_temp;
    }
    //获取post参数
    void string_operation::get_post_params(std::map<std::string, std::string>& params, const std::string& str)
    {
        //std::cout << str << std::endl;
        if (str.find("&") != std::string::npos)
        {
            std::string str1 = str.substr(0, str.find("&"));
            std::string str2 = str.substr(str.find("&") + 1);
            //std::cout << str << ", " << str1 << ", " << str2 << ", " << std::endl;
            params.insert(std::pair <std::string, std::string>(str1.substr(0, str1.find("=")), str1.substr(str1.find("=") + 1)));
            //std::cout << str1 << ", " << str1.substr(0, str1.find("=")) << ", " << str1.substr(str1.find("=") + 1) << ", " << std::endl;
            opensync::string_operation::get_post_params(params, str2);
        }
        else if (str.find("=") != std::string::npos)
        {
            params.insert(std::pair <std::string, std::string>(str.substr(0, str.find("=")), str.substr(str.find("=") + 1)));
            //std::cout << str << ", " << str.substr(0, str.find("=")) << ", " << str.substr(str.find("=") + 1) << ", " << std::endl;
        }
        return;
    }
}