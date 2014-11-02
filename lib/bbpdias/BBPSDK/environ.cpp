/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2006-2007. All rights reserved.

        Authors: Juan Hernando Vieites

*/

#include <cstring>
#include <boost/thread/mutex.hpp>
#include "BBP/Common/System/environ.h"


#ifndef __APPLE__
#ifndef WIN32
extern char ** environ;
#endif
#else
#include <cstdlib>
#endif

namespace bbp
{

// ----------------------------------------------------------------------------

static boost::mutex  s_environ_mutex;

// ----------------------------------------------------------------------------

bool getenv(const char * name, std::string & value)
{
    boost::mutex::scoped_lock lock(s_environ_mutex);
#ifndef __APPLE__
    for (char ** variable = environ; *variable != 0; ++variable)
    {
        const char * value_str = strchr(*variable, '=');
        if (strncmp(*variable, name, value_str - *variable) == 0)
        {
            value = *value_str == 0 ? "" : value_str + 1;
            return true;
        }
    }
    return false;

#else
    const char *env = ::getenv(name);
    if (env != 0)
        value = env;
    return env != 0;
#endif
}

// ----------------------------------------------------------------------------

} // namespace bbp
