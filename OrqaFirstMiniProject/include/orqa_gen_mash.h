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

/// This function creates UV sphere by generating 2 dynamicaly allocated arrays that describe vertex and index array needed for describing mashes in mordern opengl. 
/// Returns orqa_sphere_t struct containing Vs and Is.
/// The Vs array contains positions as well as texture coordinates
/// The Is array contains vertex indexes that make triangles (use GL_TRIANGLES)
/// @param radius sets sphere radius
/// @param sectors sets # of horizontal lines
/// @param stacks sets # of vertical lines
orqa_sphere_t orqa_create_sphere(
    const float radius, 
    const float sectors, 
    const float stacks);

/// This function creates a spheric window by generating 2 dynamicaly allocated arrays that describe vertex and index array needed for describing mashes in mordern opengl.  
/// Returns orqa_window_t struct containing Vs and Is.
/// The Vs array contains positions as well as texture coordinates
/// The Is array contains vertex indexes that make triangles (use GL_TRIANGLES)
/// @param radius sets spheric window radius
/// @param angleX sets horizontal angle
/// @param angleY sets vertical angle
/// @param xyz set center coorinates
orqa_window_t orqa_create_window(
    const float radius, 
    const float angleX, 
    const float angleY, 
    float x, float y, float z);

/// This function deallocates used memory when calling orqa_create_sphere()
void orqa_sphere_free(ORQA_REF orqa_sphere_t *sph);

/// This function deallocates used memory when calling orqa_create_window()
void orqa_window_free(ORQA_REF orqa_window_t *w);


#endif