#include "orqa_input.h"


/// This function converts radians from degrees.
/// Returns radians in float.
static GLfloat orqa_radians(const GLfloat deg){ 
    return (deg*M_PI/180.0f); // calculate radians
}

/// This callback function keeps track of window size and updates it when needed.
void orqa_framebuffer_size_callback(GLFWwindow *window, GLint width, GLint height){
    orqa_viewport(0, 0, width, height); // size of the rendering window
}

/// This callback function updates FieldOfView when using mouse wheel.
void orqa_scroll_callback(GLFWwindow *window, GLdouble xoffset, GLdouble yoffset){
    orqa_camera_t *cam = orqa_get_window_user_pointer(window);	
    cam->fov -= (GLfloat)yoffset/5; // update fov
    if (cam->fov < 4.2f) cam->fov = 4.2f;
    if (cam->fov > 6.2f) cam->fov = 6.2f;   
}

/// This callback function performs motorless gimbal procedure on mouse movement.
void orqa_mouse_callback(GLFWwindow *window, const GLdouble xpos, const GLdouble ypos){
    orqa_camera_t *cam = orqa_get_window_user_pointer(window);	
    versor pitchQuat, yawQuat;
    float yaw, pitch; 

    yaw = orqa_radians(xpos/10); pitch = orqa_radians(ypos/10); 

    // calculate rotations using quaternions 
    glm_quatv(pitchQuat, pitch, (vec3){1.0f, 0.0f, 0.0f});
    glm_quatv(yawQuat, yaw, (vec3){0.0f, 1.0f, 0.0f}); 

    glm_quat_mul(yawQuat, pitchQuat, cam->resultQuat); // get final quat
}


int orqa_get_key(GLFWwindow* window, int key){
    return glfwGetKey(window, key);
}