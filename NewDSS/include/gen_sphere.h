#ifndef gen_sphere_h
#define gen_sphere_h

#define ORQA_IN
#define ORQA_REF
#define ORQA_OUT
#define ORQA_NOARGS

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

typedef struct orqa_sphere_t{
    float radius; 
    int stacks, sectors, numVertices, numTriangles; 
    float *Vs;
    int *Is;
} orqa_sphere_t;

typedef struct window_t{
    float radius, x, y, z, angleX, angleY; 
    int numVertices, numTriangles; 
    float *Vs;
    int *Is;
    float sectors, stacks;
}window_t;

void orqa_gen_sphere(ORQA_REF orqa_sphere_t *sph);
void orqa_gen_window(ORQA_REF window_t *w);
void orqa_sphere_free(ORQA_REF orqa_sphere_t *sph);
void orqa_window_free(ORQA_REF window_t *w);
#endif