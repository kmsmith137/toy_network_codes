#include <map>
#include <vector>
#include <string>
#include <functional>

#include "lexical_cast.hpp"

#ifndef _ARGUMENT_PARSER_HPP
#define _ARGUMENT_PARSER_HPP


class argument_parser {
public:
    std::vector<std::string> args;
    int nargs = 0;

    void add_boolean_flag(const char *s, bool &flag)
    {
	flag = false;
	this->_add_bflag(s, [&flag]() { flag = true; return true; });
    }

    template<typename T> 
    void add_flag_with_parameter(const char *s, T &val)
    {
	this->_add_pflag(s, [&val](const std::string &v) { return lexical_cast<T> (v,val); });
    }

    template<typename T> 
    void add_flag_with_parameter(const char *s, T &val, bool &flag)
    {
	flag = false;
	this->_add_pflag(s, [&val,&flag](const std::string &v) { flag = true; return lexical_cast<T> (v,val); });
    }

    bool parse_args(int argc, char **argv);

protected:
    std::map<char, std::function<bool()>> _bflags;
    std::map<char, std::function<bool(const std::string &)>> _pflags;
    bool _parsed = false;

    char _get_char(const char *s);
    void _add_bflag(const char *s, std::function<bool()> action);
    void _add_pflag(const char *s, std::function<bool(const std::string &)> action);
    bool _parse_args(int argc, char **argv);
};


#endif  // _ARGUMENT_PARSER_HPP
