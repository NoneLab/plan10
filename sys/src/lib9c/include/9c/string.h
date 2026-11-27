#ifndef P9_LIB9C_STRING_H
#define P9_LIB9C_STRING_H


#ifdef __cplusplus
extern "C" {
#endif

/* Plan 9 quoted-token splitting (single quotes, doubled quotes). */
int p9Tokenize(char *s, char **args, int maxargs);
char *p9Strecpy(char *dst, char *end, const char *src);
int p9Cistrcmp(const char *a, const char *b);
int p9Cistrncmp(const char *a, const char *b, int n);
char *p9Cistrstr(const char *s, const char *needle);

/* In-place path normalization. For an empty input, supply room for ".". */
char *p9Cleanname(char *path);
int p9Getfields(char *s, char **args, int maxargs, int merge, const char *sep);
int p9Gettokens(char *s, char **args, int maxargs, const char *sep);
/* Returned strings use lib9c storage. */
char *p9Quotestrdup(const char *s);
char *p9Unquotestrdup(const char *s);

#ifdef __cplusplus
}
#endif

#endif
