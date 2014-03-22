/*
 * gettoken.c - �����͊֐��Q Ver 1.3
 *  1996,1998 (C) Hiroshi Masuda
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define STR_MAX 256			/* 1�s�̍ő咷 */
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

/* �O���[�o���ϐ� */
char	gt_line[STR_MAX];	/* get_token()�p��1�s�f�[�^�o�b�t�@ */
char	token[STR_MAX];		/* �擾�g�[�N�� */
/* �v���g�^�C�v�錾 */
char *get_token(void);              /* �g�[�N����肾�� */
void unget_token(char *);           /* �g�[�N���߂� */
int iskanji(int, unsigned char);	/* �����R�[�h�̔��� */

char *get_token(void)
{
	char	*p, *ptk, ch;

	p = gt_line;		/* �|�C���^�̏����� */
	ptk = token;		/* �|�C���^�̏����� */
	while(*p == ' ' || *p == '\t')	/* �󔒂�ǂݔ�΂� */
		++p;
	if(*p == '\0'){			/* �o�b�t�@���� */
		token[0] = '\0';
		return(token);
	}
	/* ----- ���� ----- */
	if(iskanji(KANJI1, *p)){
		do{
			*ptk++ = *p++;			/* 1�o�C�g�� */
			if(!iskanji(KANJI2, *p)){	/* 2�o�C�g�� */
				printf("�����R�[�h���s���ł�\n");
				exit(1);
			}
			*ptk++ = *p++;
		}while(iskanji(KANJI1, *p) && *p != '\0');
    /* ----- �p�� �p���� ----- */
	}else if(isalpha(*p)){
		do{
			*ptk++ = *p++;
		}while(isalnum(*p) && *p !='\0');	/* 2�����ڈȍ~�͉p���� */
	/* ----- ���� ----- */
    }else if(isdigit(*p) || *p == '+' || *p == '-'){
		do{
			*ptk++ = *p++;
		}while((isdigit(*p) || *p == '.' || toupper(*p) == 'E') && *p !='\0');
	/* ----- �����E������萔 ----- */
	}else if(*p == '\'' || *p == '"'){
		ch = *p;
		*ptk++ = *p++;
		do{
            if(*p == '\\'){         /* �G�X�P�[�v���� */
				*ptk++ = *p++;
				*ptk++ = *p++;
            }else if(*p != '\n')    /* ���s�R�[�h���� */
				*ptk++ = *p++;
		}while(*p != ch);
        ++p;
	/* ----- ���̑��̕��� ----- */
	}else
		*ptk++ = *p++;
    *ptk = '\0';        /* ������I�[�t�� */
	strcpy_s(gt_line,STR_MAX, p);	/* �c��̕�������o�b�t�@�ɃR�s�[ */
	return(token);
}
void unget_token(char *t)
{
	char	work[STR_MAX];

    strcpy_s(work,STR_MAX, t);        /* �߂��g�[�N���������work�ɃR�s�[ */
    if(*t == '\'' || *t == '"'){
        t[1] = '\0';
        strcat_s(work, STR_MAX, t);
    }
	strcat_s(work,STR_MAX, gt_line);	/* work��1�s�̃f�[�^��A�� */
	strcpy_s(gt_line, STR_MAX, work);	/* work��1�s�f�[�^�ɃR�s�[ */
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
	char	file[80];		/* �t�@�C���� */

	printf("�t�@�C���� : ");	/* �v�����v�g�\�� */
	gets(file);			/* �t�@�C�������� */
	if((bf = fopen(file, "r")) == NULL){		/* �t�@�C���I�[�v�� */
		printf("�t�@�C�����I�[�v���ł��Ȃ�\n");
		exit(1);	/* �����I�� */
	}
	while(fgets(gt_line, STR_MAX, bf) != NULL){	/* 1�s�ǂݍ��� */
		while(1){
			get_token();
			if(*token == '\0')		/* 1�s���I�� */
				break;
			if(*token == '\n')
				printf("(\\n)");	/* ���s�R�[�h */
			else
				printf("(%s) ", token);	/* �\�� */
		}
		printf("\n");
	}
	fclose(bf);		/* �t�@�C���N���[�Y */
}
#endif
