#include "forthreader.h"
#include <iostream>

using namespace std;

ForthReader::ForthReader(FILE* src){
	srcFile = src;
	srcText = NULL;
	srcTextPtr = -1;

	hasNext = readOne();
}

ForthReader::ForthReader(char* src){
	srcFile = NULL;
	srcText = src;
	srcTextPtr = 0;

	hasNext = readOne();
}

ScriptCode ForthReader::GetCode(){
	ScriptCode ret = nextCode;
	hasNext = readOne();
	return ret;
}

bool ForthReader::HasCode(){
	return hasNext;
}

int ForthReader::GetWorkStackSize(){
}

int ForthReader::GetCallStackSize(){
}

int ForthReader::GetWorkAreaSize(){
}

int ForthReader::GetCodeAreaSize(){
}

bool ForthReader::readOne(){
	char buf[64];
	int l = 0;
	ScriptCode code;

top:
	if(srcFile){
		char c;
		while(isspace(c = getc(srcFile)));

		do{
			buf[l++] = c;
		}while(!isspace(c = getc(srcFile)) && l < 64);

		buf[l++] = '\0';
	}
	else{
		char c;
		while(isspace(c = srcText[srcTextPtr++]));

		do{
			buf[l++] = c;
		}while(!isspace(c = srcText[srcTextPtr++]) && l < 64);

		buf[l++] = '\0';
	}

	if(buf[0] == '#'){
		// 特殊命令を実行
	}
	else{
		// 命令をコード変換
	}
}