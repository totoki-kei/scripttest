/* gettoken.h - head file for gettoken library */
#define STR_MAX 256		/* 1�s�̍ő咷 */
extern char	gt_line[STR_MAX];	/* get_token()�p��1�s�f�[�^�o�b�t�@ */
extern char	token[STR_MAX];		/* �擾�g�[�N�� */
/* �v���g�^�C�v�錾 */
extern char *get_token(void);		/* �g�[�N����肾�� */
extern void unget_token(char *);	/* �g�[�N���߂� */
int iskanji(int, unsigned char);	/* �����R�[�h�̔��� */
