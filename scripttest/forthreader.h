#ifndef SCRIPT_FORTHREADER_H
#define SCRIPT_FORTHREADER_H

#include "script.h"
#include <stdio.h>

class ForthReader : ScriptCodeProvider{
public:
	ForthReader(FILE* source);
	ForthReader(char* source);

private:
	FILE* srcFile;
	char* srcText;
	int srcTextPtr;

	ScriptCode nextCode;
	bool hasNext;

	bool readOne();

	int wsSize;
	int csSize;
	int waSize;
	int caSize;

public:
	ScriptCode GetCode();
	bool HasCode();
	int GetWorkStackSize();	
	int GetCallStackSize();	
	int GetWorkAreaSize();	
	int GetCodeAreaSize();	

}

#endif