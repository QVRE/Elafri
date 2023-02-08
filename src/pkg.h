#pragma once
#include "evar.h"

typedef struct PackageFileInfo {
	u64 ID; //8 byte unique identifier
	u64 location; //in file
	u32 size, elsize; //total size in bytes & how many elements there are
} finfo;

typedef struct PackageFile {
	u64 ID;
	void *dat;
	u32 size, elsize;
} pfile;

typedef struct ElafriPackage {
	pfile *files;
	u32 filecount;
} pkg;

void FreePkg(pkg p);

void WritePkg(char *filename, pkg p, u32 max_files);

pkg ReadPkg(char *filename);

void AddFile(char *package, pfile file);

void RmFile(char *package, u64 ID);
