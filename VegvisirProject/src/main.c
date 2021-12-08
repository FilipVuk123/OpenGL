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

// fix it!!! 
int mode; // 0 => 360, 1 => MRSS, 2 => DSS

pthread_mutex_t mutexLock;
static float orqa_radians(const float deg);
static void* orqa_udp_thread(ORQA_REF void *c_ptr);
static void orqa_process_input(ORQA_REF GLFWwindow *window); 

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
    orqa_window_t lr = orqa_create_window(1.0, 20,40,-0.7, -0.5, 0.55);
    orqa_window_t rr = orqa_create_window(1.0, 20, 40, 0.7, -0.5, 0.55);
    orqa_window_t DSS1 = orqa_create_window(1.0, 25,50,-0.7, 0, 0.64);
    orqa_window_t DSS2 = orqa_create_window(1.0, 25,50, 0, 0, 1);
    orqa_window_t DSS3 = orqa_create_window(1.0, 25,50,0.7, 0, 0.64);
    orqa_window_t mr = orqa_create_window(1.0, 20, 35, 0, -0.32, 0.5);
    orqa_window_t BW = orqa_create_window(1, 20, 35, 0, 0.6, 0.65);    
    orqa_window_t MRSS = orqa_create_window(1, 60, 130, 0, 0, 1);
    orqa_sphere_t sph = orqa_create_sphere(1, 150, 150);

    // shader init, compilation and linking
    GLuint *shaders;
    shaders = malloc(sizeof(GLuint) * 2);

    shaders[0] = orqa_load_shader_from_file("./shaders/vertexShader.vert", GL_VERTEX_SHADER);
    shaders[1] = orqa_load_shader_from_file("./shaders/fragmentShader.frag", GL_FRAGMENT_SHADER);
    GLuint shaderProgram = orqa_create_program(shaders, 2);
    orqa_use_program(shaderProgram);

    // get indexes for shader variables
    GLuint posLoc = orqa_get_attrib_location(shaderProgram, "aPos");
    GLuint texLoc = orqa_get_attrib_location(shaderProgram, "aTexCoord");
    
    // init & binding array & buffer objects
    GLuint *VAOs = orqa_generate_VAOs(9);
    GLuint *VBOs = orqa_generate_VBOs(9);
    GLuint *EBOs = orqa_generate_EBOs(9);
    
    orqa_bind_VAOs(VAOs[0]);
    orqa_bind_buffer_set_data(GL_ARRAY_BUFFER, VBOs[0], rr.numVertices*sizeof(float), rr.Vs, GL_STATIC_DRAW);
    orqa_bind_buffer_set_data(GL_ELEMENT_ARRAY_BUFFER, EBOs[0], rr.numTriangles*sizeof(int), rr.Is, GL_STATIC_DRAW);
    orqa_enable_vertex_attrib_array(posLoc, 3, GL_FLOAT, 5 * sizeof(float), (float*)0);
    orqa_enable_vertex_attrib_array(texLoc, 2, GL_FLOAT,  5 * sizeof(float), (void*)(3* sizeof(float)));
    
    orqa_bind_VAOs(VAOs[1]); 
    orqa_bind_buffer_set_data(GL_ARRAY_BUFFER, VBOs[1], lr.numVertices*sizeof(float), lr.Vs, GL_STATIC_DRAW);
    orqa_bind_buffer_set_data(GL_ELEMENT_ARRAY_BUFFER, EBOs[1], lr.numTriangles*sizeof(int), lr.Is, GL_STATIC_DRAW);
    orqa_enable_vertex_attrib_array(posLoc, 3, GL_FLOAT, 5 * sizeof(float), (float*)0);
    orqa_enable_vertex_attrib_array(texLoc, 2, GL_FLOAT,  5 * sizeof(float), (void*)(3* sizeof(float)));
    
    orqa_bind_VAOs(VAOs[2]);
    orqa_bind_buffer_set_data(GL_ARRAY_BUFFER, VBOs[2], DSS1.numVertices*sizeof(float),DSS1.Vs, GL_STATIC_DRAW);
    orqa_bind_buffer_set_data(GL_ELEMENT_ARRAY_BUFFER, EBOs[2], DSS1.numTriangles*sizeof(int), DSS1.Is, GL_STATIC_DRAW);
    orqa_enable_vertex_attrib_array(posLoc, 3, GL_FLOAT, 5 * sizeof(float), (float*)0);
    orqa_enable_vertex_attrib_array(texLoc, 2, GL_FLOAT,  5 * sizeof(float), (void*)(3* sizeof(float)));
    
    orqa_bind_VAOs(VAOs[3]);
    orqa_bind_buffer_set_data(GL_ARRAY_BUFFER, VBOs[3], DSS2.numVertices*sizeof(float), DSS2.Vs, GL_STATIC_DRAW);
    orqa_bind_buffer_set_data(GL_ELEMENT_ARRAY_BUFFER, EBOs[3], DSS2.numTriangles*sizeof(int), DSS2.Is, GL_STATIC_DRAW);
    orqa_enable_vertex_attrib_array(posLoc, 3, GL_FLOAT, 5 * sizeof(float), (float*)0);
    orqa_enable_vertex_attrib_array(texLoc, 2, GL_FLOAT,  5 * sizeof(float), (void*)(3* sizeof(float)));
    

    orqa_bind_VAOs(VAOs[4]);
    orqa_bind_buffer_set_data(GL_ARRAY_BUFFER, VBOs[4], DSS3.numVertices*sizeof(float),DSS3.Vs, GL_STATIC_DRAW);
    orqa_bind_buffer_set_data(GL_ELEMENT_ARRAY_BUFFER, EBOs[4], DSS3.numTriangles*sizeof(int), DSS3.Is, GL_STATIC_DRAW);
    orqa_enable_vertex_attrib_array(posLoc, 3, GL_FLOAT, 5 * sizeof(float), (float*)0);
    orqa_enable_vertex_attrib_array(texLoc, 2, GL_FLOAT,  5 * sizeof(float), (void*)(3* sizeof(float)));
    
    
    orqa_bind_VAOs(VAOs[5]);
    orqa_bind_buffer_set_data(GL_ARRAY_BUFFER, VBOs[5], BW.numVertices*sizeof(float),BW.Vs, GL_STATIC_DRAW);
    orqa_bind_buffer_set_data(GL_ELEMENT_ARRAY_BUFFER, EBOs[5], BW.numTriangles*sizeof(int), BW.Is, GL_STATIC_DRAW);
    orqa_enable_vertex_attrib_array(posLoc, 3, GL_FLOAT, 5 * sizeof(float), (float*)0);
    orqa_enable_vertex_attrib_array(texLoc, 2, GL_FLOAT,  5 * sizeof(float), (void*)(3* sizeof(float)));
    
    
    orqa_bind_VAOs(VAOs[6]);
    orqa_bind_buffer_set_data(GL_ARRAY_BUFFER, VBOs[6], mr.numVertices*sizeof(float),mr.Vs, GL_STATIC_DRAW);
    orqa_bind_buffer_set_data(GL_ELEMENT_ARRAY_BUFFER, EBOs[6], mr.numTriangles*sizeof(int), mr.Is, GL_STATIC_DRAW);
    orqa_enable_vertex_attrib_array(posLoc, 3, GL_FLOAT, 5 * sizeof(float), (float*)0);
    orqa_enable_vertex_attrib_array(texLoc, 2, GL_FLOAT,  5 * sizeof(float), (void*)(3* sizeof(float)));
    

    orqa_bind_VAOs(VAOs[7]);
    orqa_bind_buffer_set_data(GL_ARRAY_BUFFER, VBOs[7], MRSS.numVertices*sizeof(float), MRSS.Vs, GL_STATIC_DRAW);
    orqa_bind_buffer_set_data(GL_ELEMENT_ARRAY_BUFFER, EBOs[7], MRSS.numTriangles*sizeof(int), MRSS.Is, GL_STATIC_DRAW);
    orqa_enable_vertex_attrib_array(posLoc, 3, GL_FLOAT, 5 * sizeof(float), (float*)0);
    orqa_enable_vertex_attrib_array(texLoc, 2, GL_FLOAT,  5 * sizeof(float), (void*)(3* sizeof(float)));
    

    orqa_bind_VAOs(VAOs[8]);
    orqa_bind_buffer_set_data(GL_ARRAY_BUFFER, VBOs[8], sph.numVertices*sizeof(float), sph.Vs, GL_STATIC_DRAW);
    orqa_bind_buffer_set_data(GL_ELEMENT_ARRAY_BUFFER, EBOs[8], sph.numTriangles*sizeof(int), sph.Is, GL_STATIC_DRAW);
    orqa_enable_vertex_attrib_array(posLoc, 3, GL_FLOAT, 5 * sizeof(float), (float*)0);
    orqa_enable_vertex_attrib_array(texLoc, 2, GL_FLOAT,  5 * sizeof(float), (void*)(3* sizeof(float)));
    

    // texture init
    GLuint *textures = orqa_create_textures(5);
    // loading image!
    
    orqa_bind_texture(textures[0]);
    orqa_load_texture_from_file("./data/MRSS.png");
    orqa_bind_texture(textures[1]);
    orqa_load_texture_from_file("./data/DSS.png");
    orqa_bind_texture(textures[2]);
    orqa_load_texture_from_file("./data/pic.bmp");
    orqa_bind_texture(textures[3]);
    orqa_load_texture_from_file("./data/panorama1.bmp");

    // camera init
    orqa_camera_t cam;
    cam.cameraPos[0] = 0.0f; cam.cameraPos[1] = 0.0f; cam.cameraPos[2] = 0.0f;
    cam.resultQuat[0] = 0.0f; cam.resultQuat[1] = 0.0f; cam.resultQuat[2] = 0.0f; cam.resultQuat[3] = 1.0f;
    cam.fov = 5.4f;
    orqa_set_window_user_pointer(window, &cam); // sent camera object to callback functions

    // TCP thread & mutex init
    pthread_t udp_thread;
    // pthread_create(&udp_thread, NULL, orqa_udp_thread, &cam);
    if (pthread_mutex_init(&mutexLock, NULL) != 0) {
        fprintf(stderr, "Mutex init has failed! \n");
        goto threadError;
    }

    // MVP matrices init
    mat4 model, proj, view;
    glm_mat4_identity(model); glm_mat4_identity(view); glm_mat4_identity(proj);

    // get MVP shader indexes
    GLuint modelLoc = orqa_get_uniform_location(shaderProgram, "model");
    GLuint viewLoc = orqa_get_uniform_location(shaderProgram, "view");
    GLuint projLoc = orqa_get_uniform_location(shaderProgram, "proj");
    
    while (1){ // render loop
        // input
        orqa_process_input(window);
        
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
        
        // build texture && draw
        if(mode == 0){
            // 360 dome
            orqa_bind_texture(textures[3]);
            orqa_bind_vertex_object_and_draw_it(VAOs[8], GL_TRIANGLES, sph.numTriangles);
        }else if (mode == 1){
            // DSS
            orqa_bind_texture(textures[0]); 
            orqa_bind_vertex_object_and_draw_it(VAOs[1], GL_TRIANGLES, lr.numTriangles);
            orqa_bind_vertex_object_and_draw_it(VAOs[0], GL_TRIANGLES, rr.numTriangles);
            

            orqa_bind_texture(textures[2]);
            orqa_bind_vertex_object_and_draw_it(VAOs[5], GL_TRIANGLES, BW.numTriangles);
            orqa_bind_vertex_object_and_draw_it(VAOs[6], GL_TRIANGLES, mr.numTriangles);
            
            orqa_bind_texture(textures[1]);
            orqa_bind_vertex_object_and_draw_it(VAOs[2], GL_TRIANGLES, DSS1.numTriangles);
            orqa_bind_vertex_object_and_draw_it(VAOs[3], GL_TRIANGLES, DSS2.numTriangles);
            orqa_bind_vertex_object_and_draw_it(VAOs[4], GL_TRIANGLES, DSS3.numTriangles);
        }else if (mode == 2){
            // MRSS
            orqa_bind_texture(textures[0]);
            orqa_bind_vertex_object_and_draw_it(VAOs[7], GL_TRIANGLES, MRSS.numTriangles);
        }
        
        // glfw: swap buffers and poll IO events
        orqa_swap_buffers(window);
        orqa_pool_events();
    }
    
    // deallocating stuff
    orqa_window_free(&lr);
    orqa_window_free(&rr);
    orqa_window_free(&DSS1);
    orqa_window_free(&DSS2);
    orqa_window_free(&DSS3);
    orqa_window_free(&mr);
    orqa_window_free(&BW);
    orqa_window_free(&MRSS);
    orqa_sphere_free(&sph);
    orqa_delete_VAOs(9, VAOs);
    orqa_delete_buffers(9, VBOs);
    orqa_delete_buffers(9, EBOs);
    orqa_delete_textures(5, textures);
    orqa_delete_program(shaderProgram);
    threadError:
    pthread_exit(NULL);
    glfwTerminate(); // glfw: terminate, clearing all previously allocated GLFW resources.
    return OPENGL_OK;
}


/// This function keeps track all the input code.
/// Moves between DSS, MRSS and 360 modules using 'D', 'M' or '3' keys
static void orqa_process_input(GLFWwindow *window){ 
    // keeps all the input code
    if(orqa_get_key(window, GLFW_KEY_3) == GLFW_PRESS){
        mode = 0;
    }
    if(orqa_get_key(window, GLFW_KEY_D) == GLFW_PRESS){
        mode = 1;
    }
    if(orqa_get_key(window, GLFW_KEY_M) == GLFW_PRESS){
        mode = 2;
    }
}

/// This function connects to ORQA FPV.One goggles via UDP socket and performs motorless gimbal while goggles are in use.
static void *orqa_udp_thread(ORQA_REF void *c_ptr){
    // inits 
    fprintf(stderr, "In thread\n");
    orqa_camera_t *c = c_ptr;
    char buf[BUFSIZE];
    float yaw, pitch, roll;
    mat4 rollMat; 
    glm_mat4_identity(rollMat);
    versor rollQuat, pitchQuat, yawQuat;
    glm_quat_identity(rollQuat); glm_quat_identity(yawQuat); glm_quat_identity(pitchQuat);
    struct sockaddr_in serveraddr;
	int s, recv_len, optval = 1;
	
	//create a UDP socket
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < -1){
		printf("socket failed init\n");
        return NULL;
	}
	printf("Socket created!\n");
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));
	memset((char *) &serveraddr, 0, sizeof(serveraddr));
	
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(PORT);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	//bind socket to port
	if( bind(s , (struct sockaddr*)&serveraddr, sizeof(serveraddr) ) == -1){
		printf("Binding error!\n");
        goto exit;
	}
	printf("Bind done!\n");
	while(1){
		bzero(buf, BUFSIZE);

        if ((recv_len = recv(s, buf, BUFSIZE, 0)) < 0){
			printf("Recieving error!\n");
            break;
		}
        
        // parse JSON
        JSONObject *json = parseJSON(buf);
        yaw = atof(json->pairs[0].value->stringValue);
        pitch = -atof(json->pairs[1].value->stringValue);
        roll = -atof(json->pairs[2].value->stringValue);
        free(json);

        // Using quaternions to calculate camera rotations
        glm_quatv(pitchQuat, orqa_radians(pitch), (vec3){1.0f, 0.0f, 0.0f}); 
        glm_quatv(yawQuat, orqa_radians(yaw), (vec3){0.0f, 1.0f, 0.0f});  
        glm_quatv(rollQuat, orqa_radians(roll), (vec3){0.0f, 0.0f, 1.0f}); 
        
        pthread_mutex_lock(&mutexLock);
        glm_quat_mul(yawQuat, pitchQuat, c->resultQuat);
        glm_quat_mul(c->resultQuat, rollQuat, c->resultQuat);
        pthread_mutex_unlock(&mutexLock);
    }
    exit:
    close(s);
    return NULL;
}

/// This function converts radians from degrees.
/// Returns radians in float.
static float orqa_radians(const float deg){ 
    return (deg*M_PI/180.0f); // calculate radians
}
