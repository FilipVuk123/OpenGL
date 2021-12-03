#ifndef orqa_gen_mash_h
#define orqa_gen_mash_h

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

typedef struct orqa_window_t{
    float radius, x, y, z, angleX, angleY; 
    int numVertices, numTriangles; 
    float *Vs;
    int *Is;
    float sectors, stacks;
} orqa_window_t;

/// This function creates UV sphere by generating 2 dynamicaly allocated arrays that describe vertex and index array needed in mordern opengl. 
/// The Vs array contains positions as well as texture coordinates
/// The Is array contains vertex indexes that make triangles (use GL_TRIANGLES)
void orqa_gen_sphere(ORQA_REF orqa_sphere_t *sph);

/// This function creates a spheric window by generating 2 dynamicaly allocated arrays that describe vertex and index array needed in mordern opengl.  
/// Struct window_t has to have radius, x, y, z, angleX and angleY set to desired values.
/// The Vs array contains positions as well as texture coordinates
/// The Is array contains vertex indexes that make triangles (use GL_TRIANGLES)
void orqa_gen_window(ORQA_REF orqa_window_t *w);

/// This function deallocates used memory when calling orqa_gen_sphere()
void orqa_sphere_free(ORQA_REF orqa_sphere_t *sph);

/// This function deallocates used memory when calling orqa_gen_window()
void orqa_window_free(ORQA_REF orqa_window_t *w);

orqa_window_t orqa_create_window(
    const float radius, 
    float angleX, 
    float angleY, 
    float x, float y, float z);

orqa_sphere_t orqa_create_sphere(
    const float radius, 
    float sectors, float stacks);


#endif