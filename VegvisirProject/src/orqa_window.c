#include "orqa_window.h"

GLint orqa_init_glfw(const int major_version, const int minor_version){
    // glfw: we first initialize GLFW with glfwInit, after which we can configure GLFW using glfwWindowHint
    if(!glfwInit()){
        fprintf(stderr, "In file: %s, line: %d Failed to initialize GLFW\n", __FILE__, __LINE__);
        glfwTerminate();
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major_version); 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor_version); 
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // creating contex profile
    
    return 1;
}

GLFWwindow *orqa_create_GLFW_window(int width, int height, const char *title, GLFWmonitor *monitor, GLFWwindow *share){
    GLFWwindow *window = glfwCreateWindow(width, height, title, monitor, share); // glfw window object creation
    if (window == NULL){
        fprintf(stderr, "In file: %s, line: %d Failed to create GLFW window\n", __FILE__, __LINE__);
        return NULL;
    }
    return window;
}

void orqa_make_window_current(GLFWwindow *window){
    glfwMakeContextCurrent(window);
}


void orqa_GLFW_make_window_full_screen(){
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
}

void orqa_set_frame_buffer_cb(GLFWwindow *window, GLFWframebuffersizefun cb){
    glfwSetFramebufferSizeCallback(window, cb);
}

void orqa_set_cursor_position_cb(GLFWwindow *window, GLFWcursorposfun cb){
    glfwSetCursorPosCallback(window, cb);
}

void orqa_set_input_mode(GLFWwindow *window, int mode, int value){
    glfwSetInputMode(window, mode, value);
}

void orqa_set_scroll_cb(GLFWwindow *window, GLFWscrollfun cb){
    glfwSetScrollCallback(window, cb);
}

void orqa_set_window_user_pointer(GLFWwindow *window, void *ptr){
    glfwSetWindowUserPointer(window, ptr);
}

GLFWglproc orqa_get_proc_address(const char *procname){
    return glfwGetProcAddress(procname);
}

void *orqa_get_window_user_pointer(GLFWwindow *window){
    return glfwGetWindowUserPointer(window);
}
