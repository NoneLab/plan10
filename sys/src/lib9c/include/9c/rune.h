#ifndef P9_LIB9C_RUNE_H
#define P9_LIB9C_RUNE_H
#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int P9Rune;
enum { P9_UTFmax = 4, P9_Runeself = 0x80, P9_Runeerror = 0xfffd,
       P9_Runemax = 0x10ffff };
int p9Chartorune(P9Rune *r, const char *s);
int p9Runetochar(char *s, const P9Rune *r);
int p9Runelen(long r);
int p9Runenlen(const P9Rune *r, int n);
int p9Fullrune(const char *s, int n);
int p9Utflen(const char *s);
int p9Utfnlen(const char *s, long n);
char *p9Utfrune(const char *s, long r);
char *p9Utfrrune(const char *s, long r);
char *p9Utfutf(const char *s, const char *needle);
char *p9Utfecpy(char *dst, char *end, const char *src);
P9Rune *p9Runestrcat(P9Rune *dst, const P9Rune *src);
P9Rune *p9Runestrchr(const P9Rune *s, P9Rune r);
int p9Runestrcmp(const P9Rune *a, const P9Rune *b);
P9Rune *p9Runestrcpy(P9Rune *dst, const P9Rune *src);
P9Rune *p9Runestrncpy(P9Rune *dst, const P9Rune *src, long n);
P9Rune *p9Runestrecpy(P9Rune *dst, P9Rune *end, const P9Rune *src);
P9Rune *p9Runestrdup(const P9Rune *s);
P9Rune *p9Runestrncat(P9Rune *dst, const P9Rune *src, long n);
int p9Runestrncmp(const P9Rune *a, const P9Rune *b, long n);
P9Rune *p9Runestrrchr(const P9Rune *s, P9Rune r);
long p9Runestrlen(const P9Rune *s);
P9Rune *p9Runestrstr(const P9Rune *s, const P9Rune *needle);

#ifdef __cplusplus
}
#endif
#endif
