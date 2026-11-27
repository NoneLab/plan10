#ifndef __9PP_FILE_H
#define __9PP_FILE_H

#include "9c/dir.h"
#include "9c/ref.h"
#include "9c/sync.h"

#include <sys/types.h>

struct File;
struct FilList;
struct Tree;

/*
 * File trees.
 */
struct File {
	P9Ref ref;
	P9Dir dir;
	struct File *parent;
	void *aux;

/* below is implementation-specific; don't use */
	P9RWLock rwlock;
	struct Filelist *filelist;
	struct Tree *tree;
	int nchild;
	int allocd;
	int nxchild;
	P9Ref readers;
};

struct Filelist
{
	struct File *f;
	struct Filelist *link;
};


struct Tree {
	struct File *root;
	void	(*destroy)(struct File *file);

/* below is implementation-specific; don't use */
	P9Lock genlock;
	ulong qidgen;
	ulong dirqidgen;
};


#endif