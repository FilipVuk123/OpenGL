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
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>	// inet_addr
#include <unistd.h>
#include <netinet/in.h>
#include <cglm/cglm.h> 
#include "json.h"
#include "video_reader.h" 
#include "stb_image.h" // using this image-loading library
#include "gen_sphere.h" 
#include "orqa_clock.h"

// screen resolution
const GLuint SCR_WIDTH = 1920;
const GLuint SCR_HEIGHT = 1080; 
 
// camera_t attributes
typedef struct camera_t{
    vec3 cameraPos;
    GLfloat fov;
    versor resultQuat;
}camera_t;

pthread_mutex_t mutexLock;

const GLchar *vertexShaderSource360 = 
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec2 aTexCoord;\n"
    "out vec2 TexCoord;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 proj;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = proj*view*model*vec4(aPos.x, aPos.y, aPos.z , 1.);\n" // local space to clip space
    "   TexCoord = vec2(1. - aTexCoord.x, aTexCoord.y);\n" // mirror textures for inside sphere
    "}\n\0";

// MRSS shader
const GLchar *vertexShaderSource180 = 
    "attribute vec3 aPos;\n"
    "attribute vec2 aTexCoord;\n"
    "varying vec2 TexCoord;\n"
    "const float newMin = 0.;\n"
    "const float newMax = 1.;\n"
    "const float oldMinY = 0.25;\n"
    "const float oldMaxY = 0.75;\n"
    "const float oldRange = oldMaxY - oldMinY;\n"
    "const float oldMinX = 0.05;\n"
    "const float oldMaxX = oldMinX + oldRange;\n"
    "const float newRange = newMax - newMin;\n"
    "float newValueX;\n"
    "float newValueY;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 proj;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = proj*view*model*vec4(aPos.x, aPos.y, aPos.z , 1.);\n" // local space to clip space
    "   if ((1. - aTexCoord.x) < oldMinX || (1. - aTexCoord.x) > oldMaxX || aTexCoord.y < oldMinY || aTexCoord.y > oldMaxY){\n"
    "       newValueX = -2.;\n"
    "       newValueY = -2.;\n"
    "   }else{\n"
    "       newValueX = ( ((1. - aTexCoord.x) - oldMinX) * newRange / oldRange ) + newMin;\n"
    "       newValueY = ( (aTexCoord.y - oldMinY) * newRange / oldRange ) + newMin;\n"
    "   }\n"
    "   TexCoord = vec2(newValueX, newValueY);\n" // mirror textures for inside sphere
    "}\n\0";

// DSS shader
const char *vertexShaderSource150 =
    "attribute vec3 aPos;\n"
    "attribute vec2 aTexCoord;\n"
    "varying vec2 TexCoord;\n"
    "const float newMin = 0.;\n"
    "const float newMax = 1.;\n"
    "const float oldMinY = 0.3;\n"
    "const float oldMaxY = 0.7;\n"
    "const float oldRange = oldMaxY - oldMinY;\n"
    "const float oldMinX = 0.05;\n"
    "const float oldMaxX = oldMinX + oldRange;\n"
    "const float newRange = newMax- newMin;\n"
    "float newValueX;\n"
    "float newValueY;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 proj;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = proj*view*model*vec4(aPos.x, aPos.y, aPos.z , 1.);\n" // local space to clip space
    "   if ((1. - aTexCoord.x) < oldMinX || (1. - aTexCoord.x) > oldMaxX || aTexCoord.y < oldMinY || aTexCoord.y > oldMaxY){\n"
    "       newValueX = -2.;\n"
    "       newValueY = -2.;\n"
    "   }else{\n"
    "       newValueX = ( ((1. - aTexCoord.x) - oldMinX) * newRange / oldRange ) + newMin;\n"
    "       newValueY = ( (aTexCoord.y - oldMinY) * newRange / oldRange ) + newMin;\n"
    "   }\n"
    "   TexCoord = vec2(newValueX, newValueY);\n" // mirror textures for inside sphere
    "}\n\0";

const GLchar *fragmentShaderSource = 
    "#version 330 core\n"
    "in vec2 TexCoord;\n"
    "out vec4 FragColor;\n"
    "uniform sampler2D texture1;\n" 
    "void main()\n"
    "{\n"
    "   if (TexCoord.x < -1.0 && TexCoord.y < -1.0){\n"
    "       gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);\n"
    "   }else{\n"
    "       vec3 color = texture2D(texture1, TexCoord).xyz;\n"
    "       gl_FragColor = vec4(color , 1.0);\n"    
    "   }\n"
    "}\n\0";

static int orqa_GLFW_init(ORQA_NOARGS void);
static  GLfloat orqa_radians(ORQA_IN const GLfloat deg);
static  void orqa_mouse_callback(ORQA_REF GLFWwindow *window, ORQA_IN const GLdouble xpos, ORQA_IN const GLdouble ypos);
static void orqa_process_input(ORQA_REF GLFWwindow *window);
static void orqa_framebuffer_size_callback(ORQA_REF GLFWwindow *window,ORQA_IN GLint width,ORQA_IN GLint height);
static void orqa_scroll_callback(ORQA_REF GLFWwindow *window,ORQA_IN GLdouble xoffset,ORQA_IN GLdouble yoffset);
static void* orqa_udp_thread(ORQA_REF void *c_ptr);

int main(int argc, char **argv){
    if (orqa_GLFW_init()) return OPENGL_INIT_ERROR;
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE); // Full screen
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Vegvisir Project", NULL, NULL); // glfw window object creation
    if (window == NULL){
        fprintf(stderr, "In file: %s, line: %d Failed to create GLFW window\n", __FILE__, __LINE__);
        glfwTerminate();
        return OPENGL_INIT_ERROR;
    }
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, orqa_framebuffer_size_callback); // manipulate view port
    // glfwSetCursorPosCallback(window, orqa_mouse_callback); // move camera_t with cursor
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // use cursor but do not display it
    glfwSetScrollCallback(window, orqa_scroll_callback); // zoom in/out using mouse wheel

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){ // glad: load all OpenGL function pointers. GLFW gives us glfwGetProcAddress that defines the correct function based on which OS we're compiling for
        fprintf(stderr, "In file: %s, line: %d Failed to create initialize GLAD\n", __FILE__, __LINE__);
        glfwTerminate();
        return OPENGL_INIT_ERROR;
    }    

    // generating sphere
    orqa_sphere_t sph;
    sph.radius = 1.0f; sph.sectors = 300; sph.stacks = 300;
    orqa_gen_sphere(&sph);
    GLfloat vertices[sph.numVertices*5]; for(int i = 0; i < sph.numVertices*5; i++) vertices[i] = *(sph.Vs + i);
    GLuint indices[sph.numTriangles*3]; for(int i = 0; i < sph.numTriangles*3; i++) indices[i] = *(sph.Is + i);
    orqa_sphere_free(&sph);

    // shader init, compilation and linking
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER); 
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    GLuint shaderProgram = glCreateProgram();
    GLint success;
    GLchar infoLog[BUFSIZE];

    if(argc > 1){
        if (!strcasecmp(argv[1], "MRSS")){
            glShaderSource(vertexShader, 1, &vertexShaderSource180, NULL);
        } else if (!strcasecmp(argv[1], "DSS")){
            glShaderSource(vertexShader, 1, &vertexShaderSource150, NULL);
        } else {
            glShaderSource(vertexShader, 1, &vertexShaderSource360, NULL);
        }
    }else {
        glShaderSource(vertexShader, 1, &vertexShaderSource360, NULL);
    }
    
    glCompileShader(vertexShader);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success){
        glGetShaderInfoLog(vertexShader, BUFSIZE, NULL, infoLog);
        fprintf(stderr, "In file: %s, line: %d ERROR::SHADER::VERTEX::COMPILATION_FAILED\nError:\n%s\n", __FILE__, __LINE__, infoLog);
        goto shaderError; 
    }
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success){
        glGetShaderInfoLog(fragmentShader, BUFSIZE, NULL, infoLog);
        fprintf(stderr, "In file: %s, line: %d ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\nError:\n%s\n", __FILE__, __LINE__, infoLog);
        goto shaderError;
    }
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, BUFSIZE, NULL, infoLog);
        fprintf(stderr, "In file: %s, line: %d ERROR::SHADER::PROGRAM::LINKING_FAILED\nError:\n%s\n", __FILE__, __LINE__, infoLog);
        goto linkingError; 
    } 

    // init & binding array & buffer objects
    GLuint VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO); 
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO); 
    glBindBuffer(GL_ARRAY_BUFFER , VBO);
    glBufferData(GL_ARRAY_BUFFER , sizeof(vertices), vertices, GL_STATIC_DRAW );
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER , EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER , sizeof(indices), indices, GL_STATIC_DRAW );

    // get indexes for shader variables
    GLuint posLoc = glGetAttribLocation(shaderProgram, "aPos");
    GLuint texLoc = glGetAttribLocation(shaderProgram, "aTexCoord");
    glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (float*)0);
    glEnableVertexAttribArray(posLoc);
    glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE,  5 * sizeof(float), (void*)(3* sizeof(float)));
    glEnableVertexAttribArray(texLoc);

    // texture init
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // camera init
    camera_t cam;
    cam.cameraPos[0] = 0.0f; cam.cameraPos[1] = 0.0f; cam.cameraPos[2] = 0.0f;
    cam.resultQuat[0] = 0.0f; cam.resultQuat[1] = 0.0f; cam.resultQuat[2] = 0.0f; cam.resultQuat[3] = 1.0f;
    cam.fov = 5.2f;
    glfwSetWindowUserPointer(window, &cam); // sent camera object to callback functions

    // TCP thread & mutex init
    pthread_t tcp_thread;
    pthread_create(&tcp_thread, NULL, orqa_udp_thread, &cam);
    if (pthread_mutex_init(&mutexLock, NULL) != 0) {
        fprintf(stderr, "Mutex init has failed! \n");
        goto threadError;
    }

    // loading video file!
    // Before loading generate RGB: $ ffmpeg -y -i input.mp4 -c:v libx264rgb output.mp4
    
    video_reader_t vr_state;
    
    if(orqa_video_reader_open_file(&vr_state, "../data/360videoRGB.mp4")){
        printf("Could not open file\n");
        goto loadError;
    }
    const GLuint width = vr_state.width;  const GLuint height = vr_state.height;
    
    /*
    // loading image!
    int width, height, nrChannels;
    unsigned char *data;
    if(argc > 1){
        if (!strcasecmp(argv[1], "mrss")){
            data = stbi_load("../data/MRSS.png", &width, &height, &nrChannels, 0); 
        } else if (!strcasecmp(argv[1], "dss")){
            data = stbi_load("../data/DSS.png", &width, &height, &nrChannels, 0); 
        }
    }else {
        data = stbi_load("../data/panorama1.bmp", &width, &height, &nrChannels, 0); 
    }
    
    if (data){
        if ((int) nrChannels == 3)  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        else if ((int) nrChannels == 4)glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
    }else{
        fprintf(stderr, "In file: %s, line: %d Failed to load texture\n", __FILE__, __LINE__);
        goto loadError;
    } 
    */

    // MVP matrices init
    mat4 model, proj, view;
    glm_mat4_identity(model); glm_mat4_identity(view); glm_mat4_identity(proj);

    // get MVP shader indexes
    GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLuint projLoc = glGetUniformLocation(shaderProgram, "proj");

    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glUseProgram(shaderProgram);

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
        uint8_t *frame = orqa_video_reader_read_frame(&vr_state);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, frame); 
        glGenerateMipmap(GL_TEXTURE_2D);
        free (frame);

        // build texture  
        glBindTexture(GL_TEXTURE_2D, texture);

        // draw
        glDrawElements(GL_TRIANGLES, sizeof(indices)/sizeof(indices[0]), GL_UNSIGNED_INT, 0);

        // glfw: swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // deallocating stuff
    loadError:
    orqa_video_reader_free(&vr_state);
    threadError:
    pthread_exit(NULL);
    glDeleteVertexArrays(1, &VAO); 
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteTextures(1, &texture);
    linkingError:
    glDeleteProgram(shaderProgram);
    shaderError:
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glfwTerminate(); // glfw: terminate, clearing all previously allocated GLFW resources.
    return OPENGL_OK;
}

/// This function converts radians from degrees.
/// Returns radians in float.
static GLfloat orqa_radians(ORQA_IN const GLfloat deg){ 
    return (deg*M_PI/180.0f); // calculate radians
}

/// This function initializes GLFW.
/// Returns OPENGL_OK on success and OPENGL_INIT_ERROR on failure.
static int orqa_GLFW_init(ORQA_NOARGS void){ 
    // glfw: we first initialize GLFW with glfwInit, after which we can configure GLFW using glfwWindowHint
    if(!glfwInit()){
        fprintf(stderr, "In file: %s, line: %d Failed to initialize GLFW\n", __FILE__, __LINE__);
        glfwTerminate();
        return OPENGL_INIT_ERROR;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // Specify API version 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // Specify API version 3.3
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // creating contex profile
    
    return OPENGL_OK;
}

/// This function keeps track all the input code.
/// Closes GLFW windows when Esc key is pressed.
static void orqa_process_input(ORQA_REF GLFWwindow *window){ 
    // keeps all the input code
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){ // closes window on ESC
        glfwSetWindowShouldClose(window, GL_TRUE);
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
	if ((s=socket(AF_INET, SOCK_DGRAM, 0)) < -1){
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
	while(1)
	{
        // orqa_clock_t clock = orqa_time_now();
		bzero(buf, BUFSIZE);
		
		if ((recv_len = recv(s, buf, BUFSIZE, 0)) < 0){
			printf("Recieving error!\n");
            break;
		}
        printf("%s\n", buf);
        // printf("%.2lf\n", orqa_get_time_diff_msec(clock, orqa_time_now()));

        // parse JSON
        JSONObject *json = parseJSON(buf);
        yaw = atof(json->pairs[0].value->stringValue);
        pitch = -atof(json->pairs[1].value->stringValue);
        roll = -atof(json->pairs[2].value->stringValue);
        // printf("%f, %f, %f\n", yaw, pitch, roll);
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