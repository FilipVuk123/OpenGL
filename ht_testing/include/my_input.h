#ifndef ORQA_INPUT_H
#define ORQA_INPUT_H

#include "my_opengl.h"
#include "my_window.h"
#include <vendor/GLFW/glfw3.h>
#include <vendor/cglm/cglm.h>
#include <math.h>

typedef struct my_camera_t{
    vec3 cameraPos;
    GLfloat fov;
    versor resultQuat;
}my_camera_t;

/// This function moves camera (my_camera_t) while moving mouse
void my_mouse_callback( 
    GLFWwindow *window, 
    const GLdouble xpos, 
    const GLdouble ypos);

/// This function adjusts my_camera_t fov while scrolling mouse wheel
void my_scroll_callback(
    GLFWwindow *window, 
    GLdouble xoffset, 
    GLdouble yoffset);

/// This callback function keeps track of window size and updates it when needed.
void my_framebuffer_size_callback(
    GLFWwindow *window, 
    GLint width, 
    GLint height);

/// This function returns ID of a pressed key
int my_get_key(
    GLFWwindow* window, 
    int key);


#endif