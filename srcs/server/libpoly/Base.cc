#include "Base.h"

CBase::CBase()
{
    id = 0;
}

CBase::~CBase()
{
}

bool CBase::isNumber()
{
    return (id & MID_NUMBER);
}

bool CBase::isVar()
{
    return (id & MID_VARIABLE);
}

bool CBase::isSymbol()
{
    return (id & MID_SYMBOL);
}
//martysama0134's b9b96f11f29d7eae7e2d2a9beb08ec8f
