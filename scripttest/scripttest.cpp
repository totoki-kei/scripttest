// scripttest.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//


#include "script.h"

ScriptCode Num(float n){
	ScriptCode ret;
	ret.val = n;
	return ret;
}

ScriptCode Op(ScriptOpcode op, short opt){
	ScriptCode ret;
	ret.flag = 0;
	ret.exp = 0xFF;
	ret.opid = (unsigned short)(op & 0x7F);
	ret.option = opt;
	return ret;
}


ScriptCode code[] {
	{10}, { 20 }, Op(OpcodeAdd, 0), Op(OpcodeSto, 0), Op(OpcodeYld, 0)
};


int main(int argc, char* argv[])
{
	ScriptState *st = new ScriptState(8, 8, 8, 8, code);

	st->Run();

	delete st;

	return 0;
}

