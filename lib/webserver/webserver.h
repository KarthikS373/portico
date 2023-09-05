#ifndef _PORTICO_WEBSERVER_
#define _PORTICO_WEBSERVER_

#include <string>

class WebServer
{
public:
    WebServer();
    ~WebServer();

    void init(
        int port,
        std::string user,
        std::string password,
        std::string database_name,
        int sql_num,
        int thread_num);

private:
};

#endif