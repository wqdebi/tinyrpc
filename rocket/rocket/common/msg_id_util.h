#ifndef ROCKET_COMMOM_MSG_UTIL_H
#define ROCKET_COMMOM_MSG_UTIL_H

#include<string>

namespace rocket{

class MsgIDUtil{
public:
    static std::string GenMsgID();
};

}

#endif