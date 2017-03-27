#include <cstring>
#include "argument_parser.hpp"

using namespace std;


char argument_parser::_get_char(const char *s)
{
    if ((s[0] != '-') || (s[1] == 0) || (s[2] != 0))
	throw runtime_error("argument_parser: expected argument \"" + string(s) + "\" to have form \"-X\"");

    char c = s[1];

    if ((this->_bflags.find(c) != this->_bflags.end()) || (this->_pflags.find(c) != this->_pflags.end()))
	throw runtime_error("argument_parser: multiple \"" + string(s) + "\"entries");

    return c;
}


void argument_parser::_add_bflag(const char *s, std::function<bool()> action)
{
    char c = _get_char(s);
    _bflags[c] = action;
}


void argument_parser::_add_pflag(const char *s, std::function<bool(const string &)> action)
{
    char c = _get_char(s);
    _pflags[c] = action;
}


bool argument_parser::parse_args(int argc, char **argv)
{
    bool ret = _parse_args(argc, argv);
    
    if (ret)
	this->nargs = args.size();
    else {
	this->nargs = 0;
	this->args.clear();
    }

    return ret;
}


bool argument_parser::_parse_args(int argc, char **argv)
{
    if (_parsed)
	throw runtime_error("double call to argument_parser::parse_args()");
	    
    this->_parsed = true;

    // Note loop starts at i=1
    for (int i = 1; i < argc; i++) {
	if (argv[i][0] != '-') {
	    this->args.push_back(argv[i]);
	    continue;
	}

	if (!strcmp(argv[i], "--")) {
	    for (int j = i; j < argc; j++)
		this->args.push_back(argv[j]);
	    return true;
	}

	int n = strlen(argv[i]);

	if (n <= 1) {
	    this->args.clear();
	    return false;
	}

	if ((n == 2) && (i+1 < argc)) {
	    auto p = this->_pflags.find(argv[i][1]);

	    if (p != this->_pflags.end()) {
		if (!p->second(argv[i+1]))
		    return false;
		i++;  // extra advance
		continue;
	    }
	    
	    // fallthrough here intentional...
	}

	for (int j = 1; j < n; j++) {
	    auto p = this->_bflags.find(argv[i][j]);
	    
	    if (p == this->_bflags.end())
		return false;
	    if (!p->second())
		return false;
	}
    }

    return true;
}
