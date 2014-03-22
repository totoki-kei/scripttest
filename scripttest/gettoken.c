/*
 * gettoken.c - 字句解析関数群 Ver 1.3
 *  1996,1998 (C) Hiroshi Masuda
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define STR_MAX 256			/* 1行の最大長 */
#define EUC         1
#define SJIS1		2
#define SJIS2		3
#ifdef UNIX
	#define	KANJI1	EUC
	#define KANJI2	EUC
#else
	#define	KANJI1	SJIS1
	#define KANJI2	SJIS2
#endif

/* グローバル変数 */
char	gt_line[STR_MAX];	/* get_token()用の1行データバッファ */
char	token[STR_MAX];		/* 取得トークン */
/* プロトタイプ宣言 */
char *get_token(void);              /* トークン取りだし */
void unget_token(char *);           /* トークン戻す */
int iskanji(int, unsigned char);	/* 漢字コードの判定 */

char *get_token(void)
{
	char	*p, *ptk, ch;

	p = gt_line;		/* ポインタの初期化 */
	ptk = token;		/* ポインタの初期化 */
	while(*p == ' ' || *p == '\t')	/* 空白を読み飛ばす */
		++p;
	if(*p == '\0'){			/* バッファが空 */
		token[0] = '\0';
		return(token);
	}
	/* ----- 漢字 ----- */
	if(iskanji(KANJI1, *p)){
		do{
			*ptk++ = *p++;			/* 1バイト目 */
			if(!iskanji(KANJI2, *p)){	/* 2バイト目 */
				printf("漢字コードが不正です\n");
				exit(1);
			}
			*ptk++ = *p++;
		}while(iskanji(KANJI1, *p) && *p != '\0');
    /* ----- 英字 英数字 ----- */
	}else if(isalpha(*p)){
		do{
			*ptk++ = *p++;
		}while(isalnum(*p) && *p !='\0');	/* 2文字目以降は英数字 */
	/* ----- 数字 ----- */
    }else if(isdigit(*p) || *p == '+' || *p == '-'){
		do{
			*ptk++ = *p++;
		}while((isdigit(*p) || *p == '.' || toupper(*p) == 'E') && *p !='\0');
	/* ----- 文字・文字列定数 ----- */
	}else if(*p == '\'' || *p == '"'){
		ch = *p;
		*ptk++ = *p++;
		do{
            if(*p == '\\'){         /* エスケープ文字 */
				*ptk++ = *p++;
				*ptk++ = *p++;
            }else if(*p != '\n')    /* 改行コード無視 */
				*ptk++ = *p++;
		}while(*p != ch);
        ++p;
	/* ----- その他の文字 ----- */
	}else
		*ptk++ = *p++;
    *ptk = '\0';        /* 文字列終端付加 */
	strcpy_s(gt_line,STR_MAX, p);	/* 残りの文字列をバッファにコピー */
	return(token);
}
void unget_token(char *t)
{
	char	work[STR_MAX];

    strcpy_s(work,STR_MAX, t);        /* 戻すトークン文字列をworkにコピー */
    if(*t == '\'' || *t == '"'){
        t[1] = '\0';
        strcat_s(work, STR_MAX, t);
    }
	strcat_s(work,STR_MAX, gt_line);	/* workに1行のデータを連結 */
	strcpy_s(gt_line, STR_MAX, work);	/* workを1行データにコピー */
}
int iskanji(int type, unsigned char code)
{
	int	ret = 0;
	switch(type){
		case EUC:
			if((code >= 0xa1 && code <= 0xfe))
				ret = code;
			break;
		case SJIS1:
			if((code >= 0x81 && code <= 0x9F) || (code >= 0xe0 && code <= 0xfc))
				ret = code;
			break;
		case SJIS2:
			if((code >= 0x40 && code <= 0x7e) || (code >= 0x80 && code <= 0xfc))
				ret = code;
			break;
		default:
			printf("iskanji:unknown Kanji code type.\n");
			exit(1);
	}
	return(ret);
}

#ifdef TEST
void main(void)
{
	FILE	*bf;
	char	file[80];		/* ファイル名 */

	printf("ファイル名 : ");	/* プロンプト表示 */
	gets(file);			/* ファイル名入力 */
	if((bf = fopen(file, "r")) == NULL){		/* ファイルオープン */
		printf("ファイルがオープンできない\n");
		exit(1);	/* 強制終了 */
	}
	while(fgets(gt_line, STR_MAX, bf) != NULL){	/* 1行読み込み */
		while(1){
			get_token();
			if(*token == '\0')		/* 1行分終了 */
				break;
			if(*token == '\n')
				printf("(\\n)");	/* 改行コード */
			else
				printf("(%s) ", token);	/* 表示 */
		}
		printf("\n");
	}
	fclose(bf);		/* ファイルクローズ */
}
#endif
