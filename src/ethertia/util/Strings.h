//
// Created by Dreamtowards on 2022/5/4.
//

#ifndef ETHERTIA_STRINGS_H
#define ETHERTIA_STRINGS_H

#include <string>
#include <algorithm>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <functional>
#include <cmath>
#include <cassert>
#include <stdint.h>

class Strings
{
public:

    static std::string& erase(std::string& str, char ch);

    static std::string join(const std::string& delimiter, const std::function<std::string(int)>& fn, int n);

    static std::string join(const std::string& delimiter, const std::vector<std::string>& ls);

    // epoch: -1 -> Now
    static std::string time_fmt(std::time_t epoch_sec = -1, const char* _fmt = "%Y-%m-%d.%H:%M:%S");

    static std::string hex(void* data, std::size_t len = 1, bool uppercase = false);

    inline static const size_t SIZE_KB = 1024,
                               SIZE_MB = SIZE_KB * 1024,
                               SIZE_GB = SIZE_MB * 1024,
                               SIZE_TB = SIZE_GB * 1024;

    static std::string size_str(size_t bytes);


    template<typename... ARGS>
    static std::string fmt(const std::string& pat, ARGS... args) {
        std::stringstream ss;
        _fmt(ss, pat, args...);
        return ss.str();
    }

    template<typename OUT>
    static void _fmt(OUT& out, const std::string& pat) {
        out << pat;
    }

    template<typename OUT, typename A, typename... ARGS>
    static void _fmt(OUT& out, const std::string& pat, A a, ARGS... args) {
        size_t beg = pat.find('{');
        size_t end = pat.find('}', beg);
        int padding = 0;
        if (beg != -1 && beg+1 != end) {
            std::string slen = pat.substr(beg+1, end-(beg+1));
            padding = std::stoi(slen);
        }

        // pre
        out << (beg==-1? pat : pat.substr(0,beg));
        // val
        long vbeg = out.tellp();
        out << a;
        if (padding) {
            int vlen = (long)out.tellp() - vbeg;
            int pad = padding - vlen;
            for (int i = 0; i < pad; ++i) {
                out << ' ';
            }
        }

        // post
        _fmt(out, beg==-1? "" : pat.substr(end+1), args...);
    }


    template<typename T>
    static std::string str(T v) {
        std::stringstream ss;
        ss << v;
        return ss.str();
    }


    static std::string& toupper(std::string& str) {
        for (auto& c : str) c = std::toupper(c);
        return str;
    }
    static std::string toupper(const std::string& str_in) {
        std::string str = str_in;
        for (auto& c : str) c = std::toupper(c);
        return str;
    }

    static std::string& tolower(std::string& str) {
        for (auto& c : str) c = std::tolower(c);
        return str;
    }



    static std::vector<std::string> split(const std::string& str, const std::string& delimiter) {
        std::vector<std::string> ls;
        size_t beg = 0;
        size_t end;
        while ((end = str.find(delimiter, beg)) != std::string::npos) {
            ls.push_back(str.substr(beg, end-beg));
            beg = end + delimiter.length();
        }
        ls.push_back(str.substr(beg));
        return ls;
    }


    static size_t findChar(const std::string_view& str, char ch, int beg) {
        for (int i = beg; i < str.length(); ++i) {
            if (str[i] == ch)
                return i;
        }
        return std::string::npos;
    }
    static size_t findNonChar(const std::string_view& str, char ch, int beg) {
        for (int i = beg; i < str.length(); ++i) {
            if (str[i] != ch)
                return i;
        }
        return std::string::npos;
    }

    static std::vector<std::string> splitConnected(const std::string& str, char ch) {
        std::vector<std::string> ls;
        size_t beg = findNonChar(str, ch, 0);
        if (beg == std::string::npos) return {};

        size_t end;
        while ((end = findChar(str, ch, beg)) != std::string::npos) {
            ls.push_back(str.substr(beg, end-beg));
            beg = findNonChar(str, ch, end);
            if (beg == std::string::npos)
                return ls;
        }
        ls.push_back(str.substr(beg));
        return ls;
    }
    static std::vector<std::string> splitSpaces(const std::string& str) {
        return splitConnected(str, ' ');
    }



    static std::string walkStrings(const char* str) {
        std::stringstream ss;
        while (str && *str != '\0')
        {
            if (ss.tellp() > 0)
                ss << ", ";
            ss << str;
            str += std::strlen(str) + 1;
        }
        return ss.str();
    }

    static int lines(const std::string& str) {
        int line = 1;
        for (int i = 0; i < str.length(); ++i) {
            if (str[i] == '\n') {
                ++line;
            }
        }
        return line;
    }

    static std::string iw2(int i) {
        return i < 10 ? "0"+std::to_string(i) : std::to_string(i);
    }
    // complete 0
    static std::string int_0(int i, int wide) {
        std::stringstream ss;
        ss << i;
        int n = ss.tellp();
        ss.seekp(0);
        for (int j = 0; j < wide - n; ++j) {
            ss << '0';
        }
        return ss.str();
    }

    static std::string daytime(float daytime, bool apm = false) {
        float hr = daytime * 24.0;
        float mn =  (hr - std::floor(hr)) * 60.0;
        float sec = (mn - std::floor(mn)) * 60.0;
        if (apm) {
            bool am = hr < 12;
            return Strings::fmt("{}:{} {}", iw2(int(am ? hr : hr-12)), iw2((int)mn),
                                am ? "AM" : "PM");
        } else {
            return Strings::fmt("{}:{}", iw2((int)hr), iw2((int)mn));
        }
    }

    static float daytime(const std::string& str)
    {
        std::string hr;
        std::string mn;
        size_t apm = 0;
        size_t strlen = str.length();

        if (str.ends_with("pm") || str.ends_with("PM")) {
            apm = 2;
        } else if (str.ends_with("am") || str.ends_with("AM")) {
            apm = 1;
        }

        size_t sp = str.find(':');
        if (sp != std::string::npos) {
            hr = str.substr(0, sp);
            mn = str.substr(sp+1, apm ? strlen-2 : strlen);
        } else {
            hr = apm ? str.substr(0, strlen-2) : str;
        }

        float f = std::stof(hr) / 24.0f;
        if (apm == 2) {  // PM
            f += 0.5;
        }
        if (!mn.empty()) {
            f += std::stof(mn) / 60.0f / 24.0f;;
        }
        return f;
    }

    // split command parts by spaces. quoted by " or ' will be seen as a whole part.
//    static std::vector<std::string> splitCommand(const std::string& str) {
//
//    }
};

#endif //ETHERTIA_STRINGS_H
