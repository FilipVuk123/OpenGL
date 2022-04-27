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
    float yaw, pitch, roll;
    versor resultQuat;
}orqa_camera_t;



/// This function moves camera (orqa_camera_t) while moving mouse
void orqa_mouse_callback( 
    GLFWwindow *window, 
    const GLdouble xpos, 
    const GLdouble ypos);

/// This function adjusts orqa_camera_t fov while scrolling mouse wheel
void orqa_scroll_callback(
    GLFWwindow *window, 
    GLdouble xoffset, 
    GLdouble yoffset);

/// This callback function keeps track of window size and updates it when needed.
void orqa_framebuffer_size_callback(
    GLFWwindow *window, 
    GLint width, 
    GLint height);

/// This function returns ID of a pressed key
int orqa_get_key(
    GLFWwindow* window, 
    int key);


#endif