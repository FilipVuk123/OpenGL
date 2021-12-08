#ifndef ORQA_WINDOW_H
#define ORQA_WINDOW_H

#include <vendor/GLFW/glfw3.h>
#include <stdio.h>

/// This function initializes GLFW and returns 0 on success.
/// IMPORTANT: Use beafore initializing glad!!!
/// @param major_version sets GL major version
/// @param minor_version sets GL minor version
int orqa_init_glfw(
    const int major_version, 
    const int minor_verion);

// This function returns glproc need for initializing glad loader.
GLFWglproc orqa_get_proc_address(
    const char *procname);

// This function creates GLFW window and returns a pointer.
// Returns NULL on failure.
GLFWwindow *orqa_create_GLFW_window(
    int width, int height, 
    const char *title, 
    GLFWmonitor *monitor, 
    GLFWwindow *share);

// This function makes a window current -> makes context current for window
void orqa_make_window_current(
    GLFWwindow *window);

// This function sets current window to full screen
void orqa_GLFW_make_window_full_screen();

// This function sets framebuffer callback
void orqa_set_frame_buffer_cb(
    GLFWwindow *window, 
    GLFWframebuffersizefun cb);

// This function sets cursor position callback
void orqa_set_cursor_position_cb(
    GLFWwindow *window, 
    GLFWcursorposfun cb);

// This function sets input mode
void orqa_set_input_mode(
    GLFWwindow *window, 
    int mode, 
    int value);

// This function sets wheel scroll callback
void orqa_set_scroll_cb(
    GLFWwindow *window, 
    GLFWscrollfun cb);

// This function sets the user-defined pointer of the specified window
void orqa_set_window_user_pointer(
    GLFWwindow *window, 
    void *ptr);

// This function returns the current value of the user-defined pointer of the specified window. Initial is NULL
void *orqa_get_window_user_pointer(
    GLFWwindow *window);

void orqa_pool_events();

void orqa_swap_buffers(
    GLFWwindow *window
);

#endif