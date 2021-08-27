#ifndef _EPKG
#define _EPKG
#include "evar.c"

typedef struct PackageFileInfo {
    u64 ID; //8 byte unique identifier
    u64 location; //in file
    u32 size, elsize; //true size in bytes & how many elements there are
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

void WritePkg(char *filename, pkg p, u32 max_files) {
    FILE *fp = fopen(filename,"wb");
    finfo *file_info = (finfo*)p.files;
    u32 filecount = p.filecount;
    u32 freefl = max_files - filecount;
    void **data = malloc(filecount*sizeof(void*)); //backup pointers to data

    u64 loc = 2*sizeof(u32)+max_files*sizeof(finfo); //header + table size
    for (u32 i=0; i<filecount; i++) {
        data[i] = (void*)file_info[i].location; //save pointer
        file_info[i].location = loc; //set location
        loc += file_info[i].size; //move on
    }

    fwrite(&max_files, 1, sizeof(u32), fp); //write table size
    fwrite(&freefl, 1, sizeof(u32), fp); //how many files are free
    fwrite(file_info, filecount, sizeof(finfo), fp); //write table
    finfo tmp = {0};
    for (u32 i=0; i<freefl; i++)
        fwrite(&tmp, 1, sizeof(finfo), fp); //fill remaining space
    for (u32 i=0; i<filecount; i++)
        fwrite(data[i], file_info[i].size, 1, fp);
    free(data);
    fclose(fp);
}

pkg ReadPkg(char *filename) {
    FILE *fp = fopen(filename,"rb");
    u32 totalf, freef, filecount; //get filecount
    fread(&totalf, 1, sizeof(u32), fp); //read header
    fread(&freef, 1, sizeof(u32), fp);
    filecount = totalf - freef;

    pfile *files = malloc(filecount*sizeof(pfile)); //get files
    fread(files, filecount, sizeof(pfile), fp); //read taken table
    fseek(fp, freef, SEEK_CUR);
    u64 loc;
    for (u32 i=0; i<filecount; i++) {
        loc = (u64)files[i].dat;
        files[i].dat = malloc(files[i].size);
        fseek(fp, loc, SEEK_SET);
        fread(files[i].dat, files[i].size, 1, fp);
    }

    fclose(fp);
    return (pkg){files, filecount}; //assemble
}

void FreePkg(pkg p) {
    for (u32 i=0; i<p.filecount; i++)
        free(p.files[i].dat);
    free(p.files);
}
#endif
