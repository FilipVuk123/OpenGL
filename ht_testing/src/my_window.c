#include "my_window.h"

GLint my_init_glfw(const int major_version, const int minor_version){
    // glfw: we first initialize GLFW with glfwInit, after which we can configure GLFW using glfwWindowHint
    if(!glfwInit()){
        fprintf(stderr, "In file: %s, line: %d Failed to initialize GLFW\n", __FILE__, __LINE__);
        glfwTerminate();
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major_version); 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor_version); 
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // creating contex profile
    
    return 0;
}

GLFWwindow *my_create_GLFW_window(int width, int height, const char *title, GLFWmonitor *monitor, GLFWwindow *share){
    GLFWwindow *window = glfwCreateWindow(width, height, title, monitor, share); // glfw window object creation
    if (window == NULL){
        fprintf(stderr, "In file: %s, line: %d Failed to create GLFW window\n", __FILE__, __LINE__);
        return NULL;
    }
    return window;
}

void my_make_window_current(GLFWwindow *window){
    glfwMakeContextCurrent(window);
}

void my_set_error_cb(GLFWerrorfun cb){
    glfwSetErrorCallback(cb);
}

void my_error_cb(int n, const char *errmsg){
    (void) n;
    printf("GLFW Error: %s\n", errmsg);
}

void my_GLFW_make_window_full_screen(){
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
}

void my_set_frame_buffer_cb(GLFWwindow *window, GLFWframebuffersizefun cb){
    glfwSetFramebufferSizeCallback(window, cb);
}

void my_set_cursor_position_cb(GLFWwindow *window, GLFWcursorposfun cb){
    glfwSetCursorPosCallback(window, cb);
}

void my_set_input_mode(GLFWwindow *window, int mode, int value){
    glfwSetInputMode(window, mode, value);
}

void my_set_scroll_cb(GLFWwindow *window, GLFWscrollfun cb){
    glfwSetScrollCallback(window, cb);
}

void my_set_window_user_pointer(GLFWwindow *window, void *ptr){
    glfwSetWindowUserPointer(window, ptr);
}

GLFWglproc my_get_proc_address(const char *procname){
    return glfwGetProcAddress(procname);
}

void *my_get_window_user_pointer(GLFWwindow *window){
    return glfwGetWindowUserPointer(window);
}

void my_pool_events(){
    glfwPollEvents();
}

void my_swap_buffers(GLFWwindow *window){
    glfwSwapBuffers(window);
}

void my_terminate(){
    glfwTerminate();
}