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

    u64 loc = 2*sizeof(u32)+max_files*sizeof(finfo); //offset into start of data
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
    u32 totalf, freef, filecount;
    fread(&totalf, 1, sizeof(u32), fp); //table size
    fread(&freef, 1, sizeof(u32), fp); //free space
    filecount = totalf - freef; //get taken file count

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

void AddFile(char *package, pfile file) {
    FILE *fp = fopen(package,"r+b");
    u32 totalf, freef, filecount;
    fread(&totalf, 1, sizeof(u32), fp);
    fread(&freef, 1, sizeof(u32), fp);
    filecount = totalf - freef; //preserve offset
    freef--; //since we're adding a file
    fseek(fp, 4, SEEK_SET);
    fwrite(&freef, 1, sizeof(u32), fp);
    finfo *table = malloc(filecount*sizeof(finfo));
    fread(table, filecount, sizeof(finfo), fp); //get table

    for (u32 i=0; i<filecount; i++) { //find available space
        const u64 end = table[i].location + table[i].size; //where file ends
        u64 space = ~0; //how much free space after file ends
        for (u32 j=0; j<filecount && space; j++)
            if (table[j].location >= end)
                space = table[j].location - end;
        if (space)
            if (space >= file.size) {
                const void *data = file.dat;
                finfo *header = (finfo*)&file;
                header->location = end;
                fwrite(header, 1, sizeof(finfo), fp);
                fseek(fp, end, SEEK_SET);
                fwrite(data, file.size, 1, fp);
                break;
            }
    }
    fclose(fp);
}

void FreePkg(pkg p) {
    for (u32 i=0; i<p.filecount; i++)
        free(p.files[i].dat);
    free(p.files);
}
#endif
