#pragma once

#include <string>
#include "Core/Enumeration.h"

class CommandType
{
public:
    enum Enumeration
    {
		Init,
        Process,
        Terminate
    };
ENUMERATION(CommandType)
};
