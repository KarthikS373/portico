
#ifndef _PORTICO_COFIG_
#define _PORTICO_COFIG_

#include <string>

class Config
{
public:
    Config();
    ~Config(){};

    void parse_arg(int argc, char *argv[]);
    void loadFromYAML(const std::string &filename);

private:
    int port_;
    int thread_num_;
};

#endif
