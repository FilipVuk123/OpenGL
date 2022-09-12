#include "my_input.h"


/// This function converts radians from degrees.
/// Returns radians in float.
static GLfloat my_radians(const GLfloat deg){ 
    return (deg*M_PI/180.0f); // calculate radians
}


void my_framebuffer_size_callback(GLFWwindow *window, GLint width, GLint height){
    (void)window;
    my_set_viewport(0, 0, width, height); // size of the rendering window
}

void my_scroll_callback(GLFWwindow *window, GLdouble xoffset, GLdouble yoffset){
    (void)xoffset;
    my_camera_t *cam = my_get_window_user_pointer(window);	
    cam->fov -= (GLfloat)yoffset/5; // update fov
    if (cam->fov < 4.2f) cam->fov = 4.2f;
    if (cam->fov > 6.2f) cam->fov = 6.2f;   
}

void my_mouse_callback(GLFWwindow *window, const GLdouble xpos, const GLdouble ypos){
    my_camera_t *cam = my_get_window_user_pointer(window);	
    versor pitchQuat, yawQuat;
    float yaw, pitch; 

    yaw = my_radians(xpos/10); pitch = my_radians(ypos/10); 

    // calculate rotations using quaternions 
    glm_quatv(pitchQuat, pitch, (vec3){1.0f, 0.0f, 0.0f});
    glm_quatv(yawQuat, yaw, (vec3){0.0f, 1.0f, 0.0f}); 

    glm_quat_mul(yawQuat, pitchQuat, cam->resultQuat); // get final quat
}


int my_get_key(GLFWwindow* window, int key){
    return glfwGetKey(window, key);
}