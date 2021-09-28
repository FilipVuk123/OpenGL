#ifndef gen_sphere_h
#define gen_sphere_h

#include <stdlib.h>
#include <math.h>

typedef struct orqa_sphere_t{
    float radius; 
    int stacks, sectors, numVertices, numTriangles; 
    float *Vs;
    int *Is;
} orqa_sphere_t;

void orqa_gen_sphere(orqa_sphere_t *sph);
void orqa_sphere_free(orqa_sphere_t *sph);
#endif