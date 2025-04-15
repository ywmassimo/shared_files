#ifndef __POLY_SYMTABLE_H__
#define __POLY_SYMTABLE_H__

#include <string>

class CSymTable
{
    public:
	CSymTable(int aTok, std::string aStr);
	virtual ~CSymTable();

	double		dVal;
	int		token;
	std::string	strlex;
};

#endif
//martysama0134's b9b96f11f29d7eae7e2d2a9beb08ec8f
