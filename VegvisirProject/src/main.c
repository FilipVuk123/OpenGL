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
#include <vendor/glad/glad.h>
#include <vendor/GLFW/glfw3.h>
#include <vendor/cglm/cglm.h> 
#include "vendor/json.h"

#include <stdio.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>	// inet_addr
#include <unistd.h>
#include <netinet/in.h>
#include "orqa_video_reader.h" 
#include "orqa_gen_mash.h" 
#include "orqa_clock.h"
#include "orqa_opengl.h"

// screen resolution
const GLuint SCR_WIDTH = 1920;
const GLuint SCR_HEIGHT = 1080; 
 
// camera_t attributes
typedef struct camera_t{
    vec3 cameraPos;
    GLfloat fov;
    versor resultQuat;
}camera_t;


// fix it!!!
int mode; // 0 => 360, 1 => MRSS, 2 => DSS

pthread_mutex_t mutexLock;

static void orqa_mouse_callback(ORQA_REF GLFWwindow *window, ORQA_IN const GLdouble xpos, ORQA_IN const GLdouble ypos);
static void orqa_process_input(ORQA_REF GLFWwindow *window);
static void orqa_scroll_callback(ORQA_REF GLFWwindow *window,ORQA_IN GLdouble xoffset,ORQA_IN GLdouble yoffset);
static void orqa_framebuffer_size_callback(ORQA_REF GLFWwindow *window,ORQA_IN GLint width,ORQA_IN GLint height);

static void* orqa_udp_thread(ORQA_REF void *c_ptr);
static GLfloat orqa_radians(ORQA_IN const GLfloat deg);

int main(int argc, char **argv){

    if (!orqa_init_glfw(3,3)) return OPENGL_INIT_ERROR;
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE); // Full screen
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Vegvisir Project", NULL, NULL); // glfw window object creation
    if (window == NULL){
        fprintf(stderr, "In file: %s, line: %d Failed to create GLFW window\n", __FILE__, __LINE__);
        glfwTerminate();
        return OPENGL_INIT_ERROR;
    }
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, orqa_framebuffer_size_callback); // manipulate view port
    glfwSetCursorPosCallback(window, orqa_mouse_callback); // move camera_t with cursor
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // use cursor but do not display it
    glfwSetScrollCallback(window, orqa_scroll_callback); // zoom in/out using mouse wheel

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){ // glad: load all OpenGL function pointers. GLFW gives us glfwGetProcAddress that defines the correct function based on which OS we're compiling for
        fprintf(stderr, "In file: %s, line: %d Failed to create initialize GLAD\n", __FILE__, __LINE__);
        glfwTerminate();
        return OPENGL_INIT_ERROR;
    }    

    orqa_window_t lr;
    lr.radius = 1.0f; lr.angleY = 20; lr.angleX = 40; lr.x = -0.7; lr.y = -0.5; lr.z = 0.55; 
    orqa_gen_window(&lr);
    
    orqa_window_t rr;
    rr.radius= 1.0f; rr.angleY = 20; rr.angleX = 40; rr.x = 0.7; rr.y = -0.5; rr.z = 0.55; 
    orqa_gen_window(&rr);

    orqa_window_t DSS1;
    DSS1.radius = 1.0f; DSS1.angleY = 25; DSS1.angleX = 50; DSS1.x = -0.7; DSS1.y = 0.0; DSS1.z = 0.640;
    orqa_gen_window(&DSS1);

    orqa_window_t DSS2;
    DSS2.radius = 1.0f; DSS2.angleY = 25; DSS2.angleX = 50; DSS2.x = 0.0; DSS2.y = 0.0; DSS2.z = 1.0;
    orqa_gen_window(&DSS2);

    orqa_window_t DSS3;
    DSS3.radius = 1.0f; DSS3.angleY = 25; DSS3.angleX = 50; DSS3.x = 0.7; DSS3.y = 0.0; DSS3.z = 0.640;
    orqa_gen_window(&DSS3);

    orqa_window_t mr;
    mr.radius = 1.0f; mr.angleY = 20; mr.angleX = 35; mr.x = 0.0; mr.y = -0.32; mr.z = 0.5;
    orqa_gen_window(&mr);

    orqa_window_t BW;
    BW.radius = 1.0f; BW.angleY = 20; BW.angleX = 35; BW.x = 0.0; BW.y = 0.50; BW.z = 0.65;
    orqa_gen_window(&BW);
    
    orqa_window_t MRSS;
    MRSS.radius = 1.0f; MRSS.angleY = 60; MRSS.angleX = 130; MRSS.x = 0.0; MRSS.y = 0.0; MRSS.z = 1;
    orqa_gen_window(&MRSS);

    // generating sphere
    orqa_sphere_t sph;
    sph.radius = 1.0f; sph.sectors = 150; sph.stacks = 150;
    orqa_gen_sphere(&sph);

    // shader init, compilation and linking
    GLuint *shaders;
    shaders = (GLuint *) malloc(sizeof(GLuint) * 2);

    shaders[0] = orqa_load_shader_from_file("../shaders/vertexShader", GL_VERTEX_SHADER);
    shaders[1] = orqa_load_shader_from_file("../shaders/fragmentShader", GL_FRAGMENT_SHADER);
    GLuint shaderProgram = orqa_create_program(shaders, 2);
    orqa_use_program(shaderProgram);

    // get indexes for shader variables
    GLuint posLoc = orqa_get_attrib_location(shaderProgram, "aPos");
    GLuint texLoc = orqa_get_attrib_location(shaderProgram, "aTexCoord");

    // init & binding array & buffer objects
    GLuint VBOs[9], VAOs[9], EBOs[9];
    glGenVertexArrays(9, VAOs);
    glGenBuffers(9, VBOs); 
    glGenBuffers(9, EBOs); 

    orqa_bind_VAOs(VAOs[0]);
    glBindBuffer(GL_ARRAY_BUFFER , VBOs[0]);
    glBufferData(GL_ARRAY_BUFFER , rr.numVertices*sizeof(float), rr.Vs, GL_STATIC_DRAW );
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER , EBOs[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER , rr.numTriangles*sizeof(int), rr.Is, GL_STATIC_DRAW );
    orqa_enable_vertex_attrib_array(posLoc, 3, GL_FLOAT, 5 * sizeof(float), (float*)0);
    orqa_enable_vertex_attrib_array(texLoc, 2, GL_FLOAT,  5 * sizeof(float), (void*)(3* sizeof(float)));
    
    orqa_bind_VAOs(VAOs[1]); 
    glBindBuffer(GL_ARRAY_BUFFER , VBOs[1]);
    glBufferData(GL_ARRAY_BUFFER , lr.numVertices*sizeof(float), lr.Vs, GL_STATIC_DRAW );
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER , EBOs[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER , lr.numTriangles*sizeof(int), lr.Is, GL_STATIC_DRAW );
    orqa_enable_vertex_attrib_array(posLoc, 3, GL_FLOAT, 5 * sizeof(float), (float*)0);
    orqa_enable_vertex_attrib_array(texLoc, 2, GL_FLOAT,  5 * sizeof(float), (void*)(3* sizeof(float)));
    
    orqa_bind_VAOs(VAOs[2]);
    glBindBuffer(GL_ARRAY_BUFFER , VBOs[2]);
    glBufferData(GL_ARRAY_BUFFER , DSS1.numVertices*sizeof(float), DSS1.Vs, GL_STATIC_DRAW );
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER , EBOs[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER , DSS1.numTriangles*sizeof(int), DSS1.Is, GL_STATIC_DRAW );
    orqa_enable_vertex_attrib_array(posLoc, 3, GL_FLOAT, 5 * sizeof(float), (float*)0);
    orqa_enable_vertex_attrib_array(texLoc, 2, GL_FLOAT,  5 * sizeof(float), (void*)(3* sizeof(float)));
    
    orqa_bind_VAOs(VAOs[3]);
    glBindBuffer(GL_ARRAY_BUFFER , VBOs[3]);
    glBufferData(GL_ARRAY_BUFFER , DSS2.numVertices* sizeof(float), DSS2.Vs, GL_STATIC_DRAW );
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER , EBOs[3]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER , DSS2.numTriangles* sizeof(int), DSS2.Is, GL_STATIC_DRAW );
    orqa_enable_vertex_attrib_array(posLoc, 3, GL_FLOAT, 5 * sizeof(float), (float*)0);
    orqa_enable_vertex_attrib_array(texLoc, 2, GL_FLOAT,  5 * sizeof(float), (void*)(3* sizeof(float)));
    

    orqa_bind_VAOs(VAOs[4]);
    glBindBuffer(GL_ARRAY_BUFFER , VBOs[4]);
    glBufferData(GL_ARRAY_BUFFER , DSS3.numVertices* sizeof(float), DSS3.Vs, GL_STATIC_DRAW );
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER , EBOs[4]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER , DSS3.numTriangles* sizeof(int), DSS3.Is, GL_STATIC_DRAW );
    orqa_enable_vertex_attrib_array(posLoc, 3, GL_FLOAT, 5 * sizeof(float), (float*)0);
    orqa_enable_vertex_attrib_array(texLoc, 2, GL_FLOAT,  5 * sizeof(float), (void*)(3* sizeof(float)));
    
    
    orqa_bind_VAOs(VAOs[5]);
    glBindBuffer(GL_ARRAY_BUFFER , VBOs[5]);
    glBufferData(GL_ARRAY_BUFFER , BW.numVertices*sizeof(float), BW.Vs, GL_STATIC_DRAW );
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER , EBOs[5]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER , BW.numTriangles*sizeof(int), BW.Is, GL_STATIC_DRAW );
    orqa_enable_vertex_attrib_array(posLoc, 3, GL_FLOAT, 5 * sizeof(float), (float*)0);
    orqa_enable_vertex_attrib_array(texLoc, 2, GL_FLOAT,  5 * sizeof(float), (void*)(3* sizeof(float)));
    
    
    orqa_bind_VAOs(VAOs[6]);
    glBindBuffer(GL_ARRAY_BUFFER , VBOs[6]);
    glBufferData(GL_ARRAY_BUFFER , mr.numVertices*sizeof(float), mr.Vs, GL_STATIC_DRAW );
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER , EBOs[6]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER , mr.numTriangles*sizeof(int), mr.Is, GL_STATIC_DRAW );
    orqa_enable_vertex_attrib_array(posLoc, 3, GL_FLOAT, 5 * sizeof(float), (float*)0);
    orqa_enable_vertex_attrib_array(texLoc, 2, GL_FLOAT,  5 * sizeof(float), (void*)(3* sizeof(float)));
    

    orqa_bind_VAOs(VAOs[7]);
    glBindBuffer(GL_ARRAY_BUFFER , VBOs[7]);
    glBufferData(GL_ARRAY_BUFFER , MRSS.numVertices* sizeof(float), MRSS.Vs, GL_STATIC_DRAW );
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER , EBOs[7]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER , MRSS.numTriangles* sizeof(int), MRSS.Is, GL_STATIC_DRAW );
    orqa_enable_vertex_attrib_array(posLoc, 3, GL_FLOAT, 5 * sizeof(float), (float*)0);
    orqa_enable_vertex_attrib_array(texLoc, 2, GL_FLOAT,  5 * sizeof(float), (void*)(3* sizeof(float)));
    

    orqa_bind_VAOs(VAOs[8]);
    glBindBuffer(GL_ARRAY_BUFFER , VBOs[8]);
    glBufferData(GL_ARRAY_BUFFER , sizeof(float) * sph.numVertices, sph.Vs, GL_STATIC_DRAW );
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER , EBOs[8]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER , sizeof(int) * sph.numTriangles, sph.Is, GL_STATIC_DRAW );
    orqa_enable_vertex_attrib_array(posLoc, 3, GL_FLOAT, 5 * sizeof(float), (float*)0);
    orqa_enable_vertex_attrib_array(texLoc, 2, GL_FLOAT,  5 * sizeof(float), (void*)(3* sizeof(float)));
    

    // texture init
    GLuint *textures = orqa_create_textures(5);

    // loading image!
    
    orqa_bind_texture(0);
    orqa_load_texture_from_file("../data/MRSS.png");
    orqa_bind_texture(1);
    orqa_load_texture_from_file("../data/DSS.png");
    orqa_bind_texture(2);
    orqa_load_texture_from_file("../data/earth.jpg");
    orqa_bind_texture(3);
    orqa_load_texture_from_file("../data/panorama1.bmp");

    // camera init
    camera_t cam;
    cam.cameraPos[0] = 0.0f; cam.cameraPos[1] = 0.0f; cam.cameraPos[2] = 0.0f;
    cam.resultQuat[0] = 0.0f; cam.resultQuat[1] = 0.0f; cam.resultQuat[2] = 0.0f; cam.resultQuat[3] = 1.0f;
    cam.fov = 5.4f;
    glfwSetWindowUserPointer(window, &cam); // sent camera object to callback functions

    // TCP thread & mutex init
    pthread_t udp_thread;
    // pthread_create(&udp_thread, NULL, orqa_udp_thread, &cam);
    if (pthread_mutex_init(&mutexLock, NULL) != 0) {
        fprintf(stderr, "Mutex init has failed! \n");
        goto threadError;
    }

    // loading video file!
    // Before loading generate RGB: $ ffmpeg -y -i input.mp4 -c:v libx264rgb output.mp4
    video_reader_t vr_state;
    /*
    if(orqa_video_reader_open_file(&vr_state, "../data/CartoonRGB.mp4")){
        printf("Could not open file\n");
        goto loadError;
    }
    width = vr_state.width;  height = vr_state.height;
    */

    // MVP matrices init
    mat4 model, proj, view;
    glm_mat4_identity(model); glm_mat4_identity(view); glm_mat4_identity(proj);

    // get MVP shader indexes
    GLuint modelLoc = orqa_get_uniform_location(shaderProgram, "model");
    GLuint viewLoc = orqa_get_uniform_location(shaderProgram, "view");
    GLuint projLoc = orqa_get_uniform_location(shaderProgram, "proj");

    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    while (!glfwWindowShouldClose(window)){ // render loop
        // input
        orqa_process_input(window);

        // render
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT); // | GL_DEPTH_BUFFER_BIT);

        // generate projection matrix
        glm_perspective(cam.fov, (GLfloat)SCR_WIDTH / (GLfloat)SCR_HEIGHT, 0.01f, 100.0f, proj); // zoom

        // generate view matrix
        glm_quat_look(cam.cameraPos, cam.resultQuat, view);

        // send MVP matrices to vertex shader
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]); 
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]); 
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, &proj[0][0]);  
        
        // get video frame -> generate texture
        /*
        uint8_t *frame = orqa_video_reader_read_frame(&vr_state);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, frame); 
        glGenerateMipmap(GL_TEXTURE_2D);
        free (frame);*/

        // build texture && draw
        if(mode == 0){
            // 360 dome
            orqa_bind_texture(3);
            orqa_bind_VAOs(VAOs[8]);
            orqa_draw_elements(GL_TRIANGLES, sph.numTriangles);
        }else if (mode == 1){
            // DSS
            orqa_bind_texture(0); 
            orqa_bind_VAOs(VAOs[1]);
            orqa_draw_elements(GL_TRIANGLES, lr.numTriangles);
            orqa_bind_VAOs(VAOs[0]);
            orqa_draw_elements(GL_TRIANGLES, rr.numTriangles);

            orqa_bind_texture(2);
            orqa_bind_VAOs(VAOs[6]);
            orqa_draw_elements(GL_TRIANGLES, mr.numTriangles);
            orqa_bind_VAOs(VAOs[5]);
            orqa_draw_elements(GL_TRIANGLES, BW.numTriangles);

            orqa_bind_texture(1);
            orqa_bind_VAOs(VAOs[2]);
            orqa_draw_elements(GL_TRIANGLES, DSS1.numTriangles);
            orqa_bind_VAOs(VAOs[3]);
            orqa_draw_elements(GL_TRIANGLES, DSS2.numTriangles);
            orqa_bind_VAOs(VAOs[4]);
            orqa_draw_elements(GL_TRIANGLES, DSS3.numTriangles);
        }else if (mode == 2){
            // MRSS
            orqa_bind_texture(0);
            orqa_bind_VAOs(VAOs[7]);
            orqa_draw_elements(GL_TRIANGLES, MRSS.numTriangles);
        }
        
        // glfw: swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
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
    loadError:
    orqa_video_reader_free(&vr_state);
    threadError:
    pthread_exit(NULL);
    glDeleteVertexArrays(9, VAOs); 
    glDeleteBuffers(9, VBOs);
    glDeleteBuffers(9, EBOs);
    orqa_delete_textures(5, textures);
    linkingError:
    glDeleteProgram(shaderProgram);
    shaderError:
    // glDeleteShader(vertexShader);
    // glDeleteShader(fragmentShader);
    glfwTerminate(); // glfw: terminate, clearing all previously allocated GLFW resources.
    return OPENGL_OK;
}

/// This function converts radians from degrees.
/// Returns radians in float.
static GLfloat orqa_radians(ORQA_IN const GLfloat deg){ 
    return (deg*M_PI/180.0f); // calculate radians
}


/// This function keeps track all the input code.
/// Closes GLFW windows when Esc key is pressed.
static void orqa_process_input(ORQA_REF GLFWwindow *window){ 
    // keeps all the input code
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){ // closes window on ESC
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    if(glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS){
        mode = 0;
    }
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
        mode = 1;
    }
    if(glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS){
        mode = 2;
    }
}

/// This callback function keeps track of window size and updates it when needed.
static void orqa_framebuffer_size_callback(ORQA_REF GLFWwindow *window,ORQA_IN GLint width,ORQA_IN GLint height){
    glViewport(0, 0, width, height); // size of the rendering window
}

/// This callback function updates FieldOfView when using mouse wheel.
static void orqa_scroll_callback(ORQA_REF GLFWwindow *window, ORQA_IN GLdouble xoffset, ORQA_IN GLdouble yoffset){
    camera_t *cam = glfwGetWindowUserPointer(window);	
    cam->fov -= (GLfloat)yoffset/5; // update fov
    if (cam->fov < 4.2f) cam->fov = 4.2f;
    if (cam->fov > 6.2f) cam->fov = 6.2f;   
}

/// This callback function performs motorless gimbal procedure on mouse movement.
static void orqa_mouse_callback(ORQA_REF GLFWwindow *window, ORQA_IN const GLdouble xpos, ORQA_IN const GLdouble ypos){
    camera_t *cam = glfwGetWindowUserPointer(window);	
    versor pitchQuat, yawQuat;
    float yaw, pitch;

    yaw = orqa_radians(xpos/10); pitch = orqa_radians(ypos/10); 

    // calculate rotations using quaternions 
    glm_quatv(pitchQuat, pitch, (vec3){1.0f, 0.0f, 0.0f});
    glm_quatv(yawQuat, yaw, (vec3){0.0f, 1.0f, 0.0f}); 

    glm_quat_mul(yawQuat, pitchQuat, cam->resultQuat); // get final quat
}

/// This function connects to ORQA FPV.One goggles via UDP socket and performs motorless gimbal while goggles are in use.
static void *orqa_udp_thread(ORQA_REF void *c_ptr){
    // inits 
    fprintf(stderr, "In thread\n");
    camera_t *c = c_ptr;
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
        orqa_clock_t clock = orqa_time_now();
		
        if ((recv_len = recv(s, buf, BUFSIZE, 0)) < 0){
			printf("Recieving error!\n");
            break;
		}
        
        const double timeit = orqa_get_time_diff_msec(clock, orqa_time_now());
        fprintf(stderr, "%.4lf\n", timeit);
        
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
