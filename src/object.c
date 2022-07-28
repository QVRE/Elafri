#ifndef _OBJECT_PARSER //reading object files
#define _OBJECT_PARSER
#include <stdlib.h>
#include <stdio.h>
#include "3D.c"

// .obj file reading

obj ReadOBJ(char *filename) {
	FILE *fp = fopen(filename, "rb");
	u32 v_alloc=128, f_alloc=64; //currently allocated amounts of vertices & faces
	u32 v_off=0, f_off=0; //how many we actually have used
	vec3 *vert = malloc(v_alloc * sizeof(vec3)), *norm = malloc(f_alloc * sizeof(vec3));
	color_face *faces = malloc(f_alloc * sizeof(color_face));
	char ch;
	while ((ch=getc(fp)) != EOF) {
		switch (ch) {
			case 'v':
				if (getc(fp) == ' ') {
					F32 va, vb, vc;
					fscanf(fp, "%f %f %f", &va, &vb, &vc);
					vert[v_off] = (vec3){va, vb, vc};
					v_off++;
					if (v_off >= v_alloc) {
						v_alloc *= 2;
						vert = realloc(vert, v_alloc * sizeof(vec3));
					}
				}
				fscanf(fp, "%*[^\n]\n");
				break;
			case 'f':
				u32 a=1, b=1, c=1, d=1; //the face might be a quad
				//read each element and skip anything right after it until a space or newline
				int n = fscanf(fp, " %u%*[^ ] %u%*[^ ] %u%*[^\n ] %u%*[^\n]\n", &a, &b, &c, &d);
				a--, b--, c--, d--; //make offset start from 0
				faces[f_off] = (color_face){a, b, c, WHITE};
				norm[f_off] = GetTriangleNormal(vert[c], vert[b], vert[a]);
				f_off++;
				if (f_off >= f_alloc-1) { //-1 in case we need to process a second face below
					f_alloc *= 2;
					norm = realloc(norm, f_alloc * sizeof(vec3));
					faces = realloc(faces, f_alloc * sizeof(color_face));
				}
				if (n == 4) { //process the second triangle
					faces[f_off] = (color_face){a, c, d, WHITE};
					norm[f_off] = GetTriangleNormal(vert[d], vert[c], vert[a]);
					f_off++;
				}
			case '\n':
				break;
			default:
				fscanf(fp, "%*[^\n]\n");
				break;
		}
	}
	vert = realloc(vert, v_off * sizeof(vec3));
	norm = realloc(norm, f_off * sizeof(vec3));
	faces = realloc(faces, f_off * sizeof(color_face));
	return (obj){{0},{0},{1,1,1}, MallocMat4x4(), f_off,v_off, vert,faces,norm, NULL};
}
#endif
