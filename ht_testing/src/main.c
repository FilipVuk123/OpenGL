#define STB_IMAGE_IMPLEMENTATION

#define ORQA_IN
#define ORQA_REF
#define ORQA_OUT
#define ORQA_NOARGS

typedef enum{
    OPENGL_OK           = 0,
    OPENGL_INIT_ERROR   = -1
} OpenGLFlags;

#define BUFSIZE 1024
#define PORT 8000

#include <stdio.h>
#include <vendor/cglm/cglm.h> 
#include "my_gen_mash.h" 
#include "my_opengl.h"
#include "my_input.h"
#include "my_window.h"
#include <signal.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

static volatile int EXIT = 0;

void intHandler(int dummy) {
    EXIT = 1;
}
float my_radians(const float deg)
{
    return (deg * M_PI / 180.0f); // calculate radians
}

void *fake_headtracking(void *c_ptr){
    my_camera_t *c = c_ptr;
    versor rollQuat, pitchQuat, yawQuat;

    glm_quat_identity(rollQuat);
    glm_quat_identity(yawQuat);
    glm_quat_identity(pitchQuat);
    float yaw, pitch, roll;
    yaw = 180;
    pitch = 0;
    roll = 0;
    float yawd = 1;
    float pitchd = 1;
    float rolld = -1;
    while(!EXIT){
        // yaw += yawd;
        // pitch += pitchd;
        roll += rolld;
        if (roll >= 50 || roll <=-50) rolld = -rolld;
        if (pitch >= 50 || pitch <=-50) pitchd = -pitchd;
        if (yaw >= 50 || yaw <=-50) yawd = -yawd;
        glm_quatv(pitchQuat, my_radians(pitch), (vec3){1.0f, 0.0f, 0.0f});
        glm_quatv(yawQuat, my_radians(yaw), (vec3){0.0f, 1.0f, 0.0f});
        glm_quatv(rollQuat, my_radians(roll), (vec3){0.0f, 0.0f, 1.0f});
        glm_quat_mul(yawQuat, pitchQuat, c->resultQuat);
        glm_quat_mul(c->resultQuat, rollQuat, c->resultQuat);
        usleep(15000);
    }

    return NULL;
}

void processInput(GLFWwindow* window){
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS){
        sleep(3);
    }
}
// screen resolution
const GLuint SCR_WIDTH = 1280;
const GLuint SCR_HEIGHT = 720;

int main(){
    signal(SIGINT, intHandler);
    my_set_error_cb(my_error_cb);

    if (my_init_glfw(3,3)) return OPENGL_INIT_ERROR;
    my_GLFW_make_window_full_screen(); // Full screen
    GLFWwindow *window = my_create_GLFW_window(SCR_WIDTH, SCR_HEIGHT, "Vegvisir Project", NULL, NULL); // glfw window object creation
    if (window == NULL) return OPENGL_INIT_ERROR;
    my_make_window_current(window);

    // my_set_frame_buffer_cb(window, my_framebuffer_size_callback); // manipulate view port
    // my_set_cursor_position_cb(window, my_mouse_callback); // move camera_t with cursor
    // my_set_input_mode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // use cursor but do not display it
    // my_set_scroll_cb(window, my_scroll_callback); // zoom in/out using mouse wheel

    if (!my_load_glad((GLADloadproc)my_get_proc_address)){ // glad: load all OpenGL function pointers. GLFW gives us glfwGetProcAddress that defines the correct function based on which OS we're compiling for
        fprintf(stderr, "In file: %s, line: %d Failed to create initialize GLAD\n", __FILE__, __LINE__);
        my_terminate();
        return OPENGL_INIT_ERROR;
    }

    // mash generation 
    my_window_t myWin = my_create_window(1.0, 50, 25, 0, 0, -1);
    

    // shader init, compilation and linking
    GLuint *shaders = malloc(sizeof(GLuint) * 2);

    shaders[0] = my_load_shader_from_file("./shaders/vertexShader.vert", GL_VERTEX_SHADER);
    shaders[1] = my_load_shader_from_file("./shaders/fragmentShader.frag", GL_FRAGMENT_SHADER);

    GLuint shaderProgram = my_create_program(shaders, 2);
    my_use_program(shaderProgram);

    // get indexes for shader variables
    GLuint posLoc = my_get_attrib_location(shaderProgram, "aPos");
    GLuint texLoc = my_get_attrib_location(shaderProgram, "aTexCoord");
    
    // init & binding array & buffer objects
    GLuint *VAOs = my_generate_VAOs(1);
    GLuint *VBOs = my_generate_VBOs(1);
    GLuint *EBOs = my_generate_EBOs(1);
    
    my_bind_VAOs(VAOs[0]);
    my_bind_buffer_set_data(GL_ARRAY_BUFFER, VBOs[0], myWin.numVertices*sizeof(float), myWin.Vs, GL_STATIC_DRAW);
    my_bind_buffer_set_data(GL_ELEMENT_ARRAY_BUFFER, EBOs[0], myWin.numTriangles*sizeof(int), myWin.Is, GL_STATIC_DRAW);
    my_enable_vertex_attrib_array(posLoc, 3, GL_FLOAT, 5 * sizeof(float), (float*)0);
    my_enable_vertex_attrib_array(texLoc, 2, GL_FLOAT,  5 * sizeof(float), (void*)(3* sizeof(float)));
    
    // texture init
    GLuint *textures = my_create_textures(1);
    // loading image!
    
    my_bind_texture(textures[0]);
    my_load_texture_from_file("./data/DSS.png");

    // camera init
    my_camera_t cam;
    cam.cameraPos[0] = 0.0f; cam.cameraPos[1] = 0.0f; cam.cameraPos[2] = 0.0f;
    cam.resultQuat[0] = 0.0f; cam.resultQuat[1] = 1.0f; cam.resultQuat[2] = 0.0f; cam.resultQuat[3] = 0.0f;
    cam.fov = 5.4f;
    my_set_window_user_pointer(window, &cam); // sent camera object to callback functions

    // MVP matrices init
    mat4 model, proj, view;
    glm_mat4_identity(model); glm_mat4_identity(view); glm_mat4_identity(proj);

    // get MVP shader indexes
    GLuint modelLoc = my_get_uniform_location(shaderProgram, "model");
    GLuint viewLoc = my_get_uniform_location(shaderProgram, "view");
    GLuint projLoc = my_get_uniform_location(shaderProgram, "proj");

    pthread_t FakeHeadtracking;
    pthread_create(&FakeHeadtracking, NULL, fake_headtracking, &cam);


    versor rollQuat, pitchQuat, yawQuat;
    glm_quat_identity(rollQuat);
    glm_quat_identity(yawQuat);
    glm_quat_identity(pitchQuat);
    float yaw, pitch, roll;
    yaw = 180;
    pitch = 0;
    roll = 0;
    float yawd = 1;
    float pitchd = 1;
    float rolld = -1;

    while (!EXIT){ // render loop
        processInput(window);
        roll += rolld;
        if (roll >= 50 || roll <=-50) rolld = -rolld;
        if (pitch >= 50 || pitch <=-50) pitchd = -pitchd;
        if (yaw >= 50 || yaw <=-50) yawd = -yawd;
        glm_quatv(pitchQuat, my_radians(pitch), (vec3){1.0f, 0.0f, 0.0f});
        glm_quatv(yawQuat, my_radians(yaw), (vec3){0.0f, 1.0f, 0.0f});
        glm_quatv(rollQuat, my_radians(roll), (vec3){0.0f, 0.0f, 1.0f});
        glm_quat_mul(yawQuat, pitchQuat, cam.resultQuat);
        glm_quat_mul(cam.resultQuat, rollQuat, cam.resultQuat);
        usleep(15000);
        // render
        my_clear_color_buffer(0.2f, 0.2f, 0.2f, 1.0f);
        my_clear_buffer(GL_COLOR_BUFFER_BIT); // | GL_DEPTH_BUFFER_BIT);

        // generate projection matrix
        glm_perspective(cam.fov, (GLfloat)SCR_WIDTH / (GLfloat)SCR_HEIGHT, 0.01f, 100.0f, proj); // zoom

        // generate view matrix
        glm_quat_look(cam.cameraPos, cam.resultQuat, view);

        // send MVP matrices to vertex shader
        my_send_shander_4x4_matrix(modelLoc, 1, &model[0][0]); 
        my_send_shander_4x4_matrix(viewLoc, 1, &view[0][0]); 
        my_send_shander_4x4_matrix(projLoc, 1, &proj[0][0]);  
            
        my_bind_texture(textures[0]); 
        my_bind_vertex_object_and_draw_it(VAOs[0], GL_TRIANGLES, myWin.numTriangles);
    
        
        // glfw: swap buffers and poll IO events
        my_swap_buffers(window);
        my_pool_events();
    }
    
    // deallocating stuff
    my_window_free(&myWin);
    my_delete_VAOs(1, VAOs);
    my_delete_buffers(1, VBOs);
    my_delete_buffers(1, EBOs);
    my_delete_textures(1, textures);
    my_delete_program(shaderProgram);
    my_terminate(); // glfw: terminate, clearing all previously allocated GLFW resources.
    return OPENGL_OK;
}
