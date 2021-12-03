#include "orqa_input.h"


/// This function converts radians from degrees.
/// Returns radians in float.
static GLfloat orqa_radians(const GLfloat deg){ 
    return (deg*M_PI/180.0f); // calculate radians
}


void orqa_framebuffer_size_callback(GLFWwindow *window, GLint width, GLint height){
    orqa_set_viewport(0, 0, width, height); // size of the rendering window
}

void orqa_scroll_callback(GLFWwindow *window, GLdouble xoffset, GLdouble yoffset){
    orqa_camera_t *cam = orqa_get_window_user_pointer(window);	
    cam->fov -= (GLfloat)yoffset/5; // update fov
    if (cam->fov < 4.2f) cam->fov = 4.2f;
    if (cam->fov > 6.2f) cam->fov = 6.2f;   
}

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