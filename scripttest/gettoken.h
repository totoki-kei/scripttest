/* gettoken.h - head file for gettoken library */
#define STR_MAX 256		/* 1行の最大長 */
extern char	gt_line[STR_MAX];	/* get_token()用の1行データバッファ */
extern char	token[STR_MAX];		/* 取得トークン */
/* プロトタイプ宣言 */
extern char *get_token(void);		/* トークン取りだし */
extern void unget_token(char *);	/* トークン戻す */
int iskanji(int, unsigned char);	/* 漢字コードの判定 */
