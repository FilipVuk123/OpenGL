#ifndef ORQA_INPUT_H
#define ORQA_INPUT_H

#include "orqa_opengl.h"
#include "orqa_window.h"
#include <vendor/GLFW/glfw3.h>
#include <vendor/cglm/cglm.h>
#include <math.h>

typedef struct orqa_camera_t{
    vec3 cameraPos;
    GLfloat fov;
    versor resultQuat;
}orqa_camera_t;

void orqa_mouse_callback( 
    GLFWwindow *window, 
    const GLdouble xpos, 
    const GLdouble ypos);

void orqa_scroll_callback(
    GLFWwindow *window, 
    GLdouble xoffset, 
    GLdouble yoffset);

void orqa_framebuffer_size_callback(
    GLFWwindow *window, 
    GLint width, 
    GLint height);

int orqa_get_key(
    GLFWwindow* window, 
    int key);


#endif