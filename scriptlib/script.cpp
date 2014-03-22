#include "script.h"
#include <stdio.h>
#include <stdlib.h>

ScriptState::ScriptState(
						 int wstacksize,
						 int cstacksize,
						 int wareasize,
						 int codesize,
						 ScriptCode* code)
{
	provider = NULL;

	workstacksize = wstacksize;
	callstacksize = cstacksize;
	workareasize = wareasize;
	codeareasize = codesize;

	if((workstacksize | callstacksize | workareasize | codeareasize) < 0){
		errorCode = InvalidFileHeader;
		return;
 	}

	workstack = new float[workstacksize];
	callstack = new int[callstacksize];
	workarea = new float[workareasize];
	codearea = (float*)code;
	owncode = false;

	for(int i = 0;i < workareasize;i++){
		((int*)workarea)[i] = 0x7FBFFFFF;
	}

	if (!(workstack && callstack && workarea && codearea)){
		errorCode = MemAllocationError;
		return;
	}

	Reset();
};

ScriptState::ScriptState(ScriptCodeProvider *prov){

	extOpTable = NULL;

	this->provider = prov;

	workstacksize = prov->GetWorkAreaSize();
	callstacksize = prov->GetCallStackSize();
	workareasize = prov->GetWorkAreaSize();
	codeareasize = prov->GetCodeAreaSize();

	if((workstacksize | callstacksize | workareasize | codeareasize) < 0){
		errorCode = InvalidFileHeader;
		goto finish;
	}

	workstack = new float[workstacksize];
	callstack = new int[callstacksize];
	workarea = new float[workareasize];
	codearea = prov->GetCodes();
	owncode = false;

	if(!(workstack && callstack && workarea && codearea)){
		errorCode = MemAllocationError;
		goto finish;
	}

	for (int i = 0; i < workareasize; i++){
		((int*)workarea)[i] = 0x7FBFFFFF;
	}

	Reset();

finish:
	return;
};

ScriptState::ScriptState(char* filename){
	FILE* fp;

	short header[4];

	extOpTable = NULL;
	provider = NULL;

	if(fopen_s(&fp,filename,"rb")){
		errorCode = FileCannotOpen;
		return;
	}

	if(fread(header,sizeof(int),4,fp) == (size_t)0){
		errorCode = FileCannotRead;
		goto finish;
	}

	workstacksize = header[0];
	callstacksize = header[1];
	workareasize = header[2];
	codeareasize = header[3];

	if((workstacksize | callstacksize | workareasize | codeareasize) < 0){
		errorCode = InvalidFileHeader;
		goto finish;
	}

	workstack = new float[workstacksize];
	callstack = new int[callstacksize];
	workarea = new float[workareasize];
	codearea = new float[codeareasize];
	owncode = true;

	if(!(workstack && callstack && workarea && codearea)){
		errorCode = MemAllocationError;
		goto finish;
	}

	for(int i = 0;i < workareasize;i++){
		((int*)workarea)[i] = 0x7FBFFFFF;
	}

	if(fread(codearea,sizeof(int),codeareasize,fp) < (size_t)codeareasize){
		errorCode = InvalidFileHeader;
		goto finish;
	}

	Reset();

finish:
	fclose(fp);
	return;
};

ScriptState::~ScriptState(){
	delete[] workstack;
	delete[] callstack;
	delete[] workarea;
	if (owncode) delete[] codearea;
};

ScriptReturnState ScriptState::Run(){
	if(errorCode)
		return Error;
	
	if(waitcount){
		waitcount--;
		return Wait;
	}

	ScriptCode code;
	int id;
	short opt;
	ScriptReturnState rs;
	bool flg;
	while(codeindex < codeareasize){
		code.val = codearea[codeindex];
		if(code.exp == 0xFF){	//	仮
			// オペコード
			id = (unsigned int)code.opid;
			// オプション値
			opt = (short)code.option;
			// 拡張コードフラグ
			flg = (0 || code.flag);

			//	命令実行
			
			switch(rs = (flg ? extOpTable ? extOpTable[id](this,opt) : Error : (this->*optable[id])(opt))){
				case Error:
				case Yield:
				case Finished:
					return rs;
					break;
			}
			
		}
		else{
			// float
			workstack[workstacktop++] = code.val;
			if(workstacktop == workstacksize){
				errorCode = WorkstackOverflow;
				return Error;
			}
		}
		codeindex++;
	};
	
	return Finished;
};

ScriptReturnState ScriptState::NullOp(ScriptState* stat,short opt){
	stat->errorCode = InvalidOpcode;
	return Error;
};

void ScriptState::SetExternalOpTable(ScriptExternOp* opTable){
	this->extOpTable = opTable;
};

void ScriptState::Reset(){
	errorCode = OK;
	codeindex = 0;
	callstacktop = 0;
	workstacktop = 0;
	waitcount = 0;
};
