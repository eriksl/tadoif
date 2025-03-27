#include "tadoif.h"

TadoIf::Exception::Exception(const std::string &what) : what_string(what)
{
}

TadoIf::Exception::Exception(const char *what) : what_string(what)
{
}

TadoIf::Exception::Exception(const boost::format &what) : what_string(what.str())
{
}

const char *TadoIf::Exception::what() const noexcept
{
	return(what_string.c_str());
}

TadoIf::InternalException::InternalException(const std::string &what) : what_string(what)
{
}

TadoIf::InternalException::InternalException(const char *what) : what_string(what)
{
}

TadoIf::InternalException::InternalException(const boost::format &what) : what_string(what.str())
{
}

const char *TadoIf::InternalException::what() const noexcept
{
	return(what_string.c_str());
}
