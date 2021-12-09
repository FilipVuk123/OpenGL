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
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>	// inet_addr
#include <unistd.h>
#include <netinet/in.h>

#include <vendor/cglm/cglm.h> 
#include "vendor/json.h"
#include "orqa_gen_mash.h" 
#include "orqa_clock.h"
#include "orqa_opengl.h"
#include "orqa_input.h"
#include "orqa_window.h"

// screen resolution
const GLuint SCR_WIDTH = 1920;
const GLuint SCR_HEIGHT = 1080;

int main(){
    if (orqa_init_glfw(3,3)) return OPENGL_INIT_ERROR;
    orqa_GLFW_make_window_full_screen(); // Full screen
    GLFWwindow *window = orqa_create_GLFW_window(SCR_WIDTH, SCR_HEIGHT, "Vegvisir Project", NULL, NULL); // glfw window object creation
    if (window == NULL) return OPENGL_INIT_ERROR;
    orqa_make_window_current(window);

    orqa_set_frame_buffer_cb(window, orqa_framebuffer_size_callback); // manipulate view port
    orqa_set_cursor_position_cb(window, orqa_mouse_callback); // move camera_t with cursor
    orqa_set_input_mode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // use cursor but do not display it
    orqa_set_scroll_cb(window, orqa_scroll_callback); // zoom in/out using mouse wheel

    if (!orqa_load_glad((GLADloadproc)orqa_get_proc_address)){ // glad: load all OpenGL function pointers. GLFW gives us glfwGetProcAddress that defines the correct function based on which OS we're compiling for
        fprintf(stderr, "In file: %s, line: %d Failed to create initialize GLAD\n", __FILE__, __LINE__);
        glfwTerminate();
        return OPENGL_INIT_ERROR;
    }

    // mash generation 
    orqa_window_t myWin = orqa_create_window(1.0, 25,50, 0, 0, 1);
    

    // shader init, compilation and linking
    GLuint *shaders = malloc(sizeof(GLuint) * 2);

    shaders[0] = orqa_load_shader_from_file("./shaders/vertexShader.vert", GL_VERTEX_SHADER);
    shaders[1] = orqa_load_shader_from_file("./shaders/fragmentShader.frag", GL_FRAGMENT_SHADER);
    GLuint shaderProgram = orqa_create_program(shaders, 2);
    orqa_use_program(shaderProgram);

    // get indexes for shader variables
    GLuint posLoc = orqa_get_attrib_location(shaderProgram, "aPos");
    GLuint texLoc = orqa_get_attrib_location(shaderProgram, "aTexCoord");
    
    // init & binding array & buffer objects
    GLuint *VAOs = orqa_generate_VAOs(1);
    GLuint *VBOs = orqa_generate_VBOs(1);
    GLuint *EBOs = orqa_generate_EBOs(1);
    
    orqa_bind_VAOs(VAOs[0]);
    orqa_bind_buffer_set_data(GL_ARRAY_BUFFER, VBOs[0], myWin.numVertices*sizeof(float), myWin.Vs, GL_STATIC_DRAW);
    orqa_bind_buffer_set_data(GL_ELEMENT_ARRAY_BUFFER, EBOs[0], myWin.numTriangles*sizeof(int), myWin.Is, GL_STATIC_DRAW);
    orqa_enable_vertex_attrib_array(posLoc, 3, GL_FLOAT, 5 * sizeof(float), (float*)0);
    orqa_enable_vertex_attrib_array(texLoc, 2, GL_FLOAT,  5 * sizeof(float), (void*)(3* sizeof(float)));
    
    // texture init
    GLuint *textures = orqa_create_textures(1);
    // loading image!
    
    orqa_bind_texture(textures[0]);
    orqa_load_texture_from_file("./data/DSS.png");

    // camera init
    orqa_camera_t cam;
    cam.cameraPos[0] = 0.0f; cam.cameraPos[1] = 0.0f; cam.cameraPos[2] = 0.0f;
    cam.resultQuat[0] = 0.0f; cam.resultQuat[1] = 0.0f; cam.resultQuat[2] = 0.0f; cam.resultQuat[3] = 1.0f;
    cam.fov = 5.4f;
    orqa_set_window_user_pointer(window, &cam); // sent camera object to callback functions

    // MVP matrices init
    mat4 model, proj, view;
    glm_mat4_identity(model); glm_mat4_identity(view); glm_mat4_identity(proj);

    // get MVP shader indexes
    GLuint modelLoc = orqa_get_uniform_location(shaderProgram, "model");
    GLuint viewLoc = orqa_get_uniform_location(shaderProgram, "view");
    GLuint projLoc = orqa_get_uniform_location(shaderProgram, "proj");
    
    while (1){ // render loop
        
        // render
        orqa_clear_color_buffer(0.2f, 0.2f, 0.2f, 1.0f);
        orqa_clear_buffer(GL_COLOR_BUFFER_BIT); // | GL_DEPTH_BUFFER_BIT);

        // generate projection matrix
        glm_perspective(cam.fov, (GLfloat)SCR_WIDTH / (GLfloat)SCR_HEIGHT, 0.01f, 100.0f, proj); // zoom

        // generate view matrix
        glm_quat_look(cam.cameraPos, cam.resultQuat, view);

        // send MVP matrices to vertex shader
        orqa_send_shander_4x4_matrix(modelLoc, 1, &model[0][0]); 
        orqa_send_shander_4x4_matrix(viewLoc, 1, &view[0][0]); 
        orqa_send_shander_4x4_matrix(projLoc, 1, &proj[0][0]);  
            
        orqa_bind_texture(textures[0]); 
        orqa_bind_vertex_object_and_draw_it(VAOs[0], GL_TRIANGLES, myWin.numTriangles);
    
        
        // glfw: swap buffers and poll IO events
        orqa_swap_buffers(window);
        orqa_pool_events();
    }
    
    // deallocating stuff
    orqa_window_free(&myWin);
    orqa_delete_VAOs(1, VAOs);
    orqa_delete_buffers(1, VBOs);
    orqa_delete_buffers(1, EBOs);
    orqa_delete_textures(1, textures);
    orqa_delete_program(shaderProgram);
    glfwTerminate(); // glfw: terminate, clearing all previously allocated GLFW resources.
    return OPENGL_OK;
}
