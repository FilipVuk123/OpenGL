#ifndef gen_sphere_h
#define gen_sphere_h

#include <stdlib.h>
#include <math.h>

typedef struct sphere{
    float radius; 
    int stacks, sectors, numVertices, numTriangles; 
    float *Vs;
    int *Is;
} sphere;

void ORQA_GenSphere(sphere *sph);
void ORQA_Sphere_free(sphere *sph);
#endif