#ifndef ORQA_WINDOW_H
#define ORQA_WINDOW_H

#include <vendor/GLFW/glfw3.h>
#include <stdio.h>

int orqa_init_glfw(
    const int major_version, 
    const int minor_verion);

GLFWglproc orqa_get_proc_address(
    const char *procname);

GLFWwindow *orqa_create_GLFW_window(
    int width, int height, 
    const char *title, 
    GLFWmonitor *monitor, 
    GLFWwindow *share);

void orqa_make_window_current(
    GLFWwindow *window);

void orqa_GLFW_make_window_full_screen();

void orqa_set_frame_buffer_cb(
    GLFWwindow *window, 
    GLFWframebuffersizefun cb);

void orqa_set_cursor_position_cb(
    GLFWwindow *window, 
    GLFWcursorposfun cb);

void orqa_set_input_mode(
    GLFWwindow *window, 
    int mode, 
    int value);

void orqa_set_scroll_cb(
    GLFWwindow *window, 
    GLFWscrollfun cb);

void orqa_set_window_user_pointer(
    GLFWwindow *window, 
    void *ptr);

void *orqa_get_window_user_pointer(
    GLFWwindow *window);

#endif