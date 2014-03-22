#ifndef _SCRIPT_PARSE_H
#define _SCRIPT_PARSE_H

#include "script.h"
#include <string>
#include <vector>

using namespace std;

class ScriptForceParser{
public:
	ScriptForceParser();

	ScriptState* Parse(char* src);

	void SetAlias(char* name,float value);
	void SetOperand(char* name, int index);
	void SetOperandTable(char names[128]);
};

#endif
