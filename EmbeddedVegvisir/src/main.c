#include <stdio.h>

#define BUFSIZE     1024
#define ORQA_IN
#define ORQA_REF
#define ORQA_OUT
#define ORQA_NOARGS

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define GLFW_INCLUDE_ES31
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>	// inet_addr
#include <unistd.h>
#include <netinet/in.h>
#include "gen_sphere.h"
#include "json.h"

typedef enum{
    OPENGL_OK           = 0,
    OPENGL_INIT_ERROR   = -1
} OpenGLFlags;

const GLuint SCR_WIDTH = 1920;
const GLuint SCR_HEIGHT = 1080; 

// camera_t attributes
typedef struct camera_t{
    vec3 cameraPos;
    GLfloat fov;
    versor resultQuat;
}camera_t;

const GLchar *vertexShaderSource = 
    "attribute vec3 aPos;\n"
    "attribute vec2 aTexCoord;\n"
    "varying vec2 TexCoord;\n"
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
    "const float oldMinX = 0.05;\n"
    "const float oldMaxX = 0.55;\n"
    "const float oldRange = oldMaxY - oldMinY;\n"
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
    "       newValueX = -1.;\n"
    "       newValueY = -1.;\n"
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
    "const float oldMinX = 0.05;\n"
    "const float oldMaxX = 0.45;\n"
    "const float oldRange = oldMaxY - oldMinY;\n"
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
    "       newValueX = -1.;\n"
    "       newValueY = -1.;\n"
    "   }else{\n"
    "       newValueX = ( ((1. - aTexCoord.x) - oldMinX) * newRange / oldRange ) + newMin;\n"
    "       newValueY = ( (aTexCoord.y - oldMinY) * newRange / oldRange ) + newMin;\n"
    "   }\n"
    "   TexCoord = vec2(newValueX, newValueY);\n" // mirror textures for inside sphere
    "}\n\0";



const GLchar *fragmentShaderSource = 
    "precision mediump float;\n"
    "varying vec2 TexCoord;\n"
    "uniform sampler2D texture1;\n" 
    "void main()\n"
    "{\n"
    "   gl_FragColor = texture2D(texture1, TexCoord);\n" 
    "}\n\0";

pthread_mutex_t mutexLock;


static int orqa_GLFW_init(ORQA_NOARGS void);
static void orqa_framebuffer_size_callback(ORQA_REF GLFWwindow *window,ORQA_IN GLint width,ORQA_IN GLint height);
static  GLfloat orqa_radians(ORQA_IN const GLfloat deg);
static  void orqa_mouse_callback(ORQA_REF GLFWwindow *window, ORQA_IN const double xpos, ORQA_IN const double ypos);
static void orqa_scroll_callback(ORQA_REF GLFWwindow *window,ORQA_IN double xoffset,ORQA_IN double yoffset);
static void* orqa_tcp_thread(ORQA_REF camera_t *c);



int main(int argc, char **argv) {
    if (orqa_GLFW_init()) return OPENGL_INIT_ERROR;
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE); // Full screen
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Vegvisir Project", glfwGetPrimaryMonitor(), NULL); // glfw window object creation
    if (window == NULL){
        fprintf(stderr, "In file: %s, line: %d Failed to create GLFW window\n", __FILE__, __LINE__);
        glfwTerminate();
        return OPENGL_INIT_ERROR;
    }
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, orqa_framebuffer_size_callback); // manipulate view port

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, orqa_mouse_callback); // move camera_t with cursor
    glfwSetScrollCallback(window, orqa_scroll_callback); // zoom in/out using mouse wheel

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
        if (!strcmp(argv[1], "MRSS")){
            glShaderSource(vertexShader, 1, &vertexShaderSource180, NULL);
        } else if (!strcmp(argv[1], "DSS")){
            glShaderSource(vertexShader, 1, &vertexShaderSource150, NULL);
        } else {
            glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        }
    }else {
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
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
    cam.fov = 5.4f;
    glfwSetWindowUserPointer(window, &cam);

    // loading image!
    GLuint width, height, nrChannels;
    unsigned char *data = stbi_load("./data/earth.jpg", &width, &height, &nrChannels, 0); 
    if (data){
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
    } else{
        fprintf(stderr, "In file: %s, line: %d Failed to load texture\n", __FILE__, __LINE__);
        goto loadError;
    } 

    // TCP thread & mutex init
    pthread_t tcp_thread;
    // pthread_create(&tcp_thread, NULL, orqa_tcp_thread, &cam);
    if (pthread_mutex_init(&mutexLock, NULL) != 0) {
        fprintf(stderr, "Mutex init has failed! \n");
        goto threadError;
    }

    // MVP matrices init
    mat4 model, proj, view;
    glm_mat4_identity(model); glm_mat4_identity(view); glm_mat4_identity(proj);

    // get MVP shader indexes
    GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLuint projLoc = glGetUniformLocation(shaderProgram, "proj");

    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glUseProgram(shaderProgram);

    
    const int numElements = sizeof(indices)/sizeof(indices[0]);
    while (1){ // render loop
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

        // build texture  
        glBindTexture(GL_TEXTURE_2D, texture);

        // draw
        glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_INT, 0);

        // glfw: swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // deallocating stuff
    loadError:

    threadError:
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


/// This function initializes GLFW.
/// Returns OPENGL_OK on success and OPENGL_INIT_ERROR on failure.
static int orqa_GLFW_init(ORQA_NOARGS void){ 
    // glfw: we first initialize GLFW with glfwInit, after which we can configure GLFW using glfwWindowHint
    if(!glfwInit()){
        fprintf(stderr, "In file: %s, line: %d Failed to initialize GLFW\n", __FILE__, __LINE__);
        glfwTerminate();
        return OPENGL_INIT_ERROR;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1); 
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);

    return OPENGL_OK;
}
/// This callback function keeps track of window size and updates it when needed.
static void orqa_framebuffer_size_callback(ORQA_REF GLFWwindow *window,ORQA_IN GLint width,ORQA_IN GLint height){
    glViewport(0, 0, width, height); // size of the rendering window
}


/// This callback function performs motorless gimbal procedure on mouse movement.
static void orqa_mouse_callback(ORQA_REF GLFWwindow *window, ORQA_IN const double xpos, ORQA_IN const double ypos){
    camera_t *cam = glfwGetWindowUserPointer(window);	
    if (!cam) {
        fprintf(stderr, "No camera in orqa_mouse_callback()\n");
        return;
    }
    versor pitchQuat, yawQuat;
    float yaw, pitch;

    yaw = orqa_radians(xpos/10); pitch = orqa_radians(ypos/10); 

    // calculate rotations using quaternions 
    glm_quatv(pitchQuat, pitch, (vec3){1.0f, 0.0f, 0.0f});
    glm_quatv(yawQuat, yaw, (vec3){0.0f, 1.0f, 0.0f}); 
    
    glm_quat_mul(yawQuat, pitchQuat, cam->resultQuat); // get final quat
}

/// This function converts radians from degrees.
/// Returns radians in float.
static GLfloat orqa_radians(ORQA_IN const GLfloat deg){ 
    return (deg*M_PI/180.0f); // calculate radians
}

/// This callback function updates FieldOfView when using mouse wheel.
static void orqa_scroll_callback(ORQA_REF GLFWwindow *window, ORQA_IN double xoffset, ORQA_IN double yoffset){
    camera_t *cam = glfwGetWindowUserPointer(window);	
    cam->fov -= (GLfloat)yoffset/5; // update fov
    if (cam->fov < 5.4f) cam->fov = 5.4f;
    if (cam->fov > 6.2f) cam->fov = 6.2f;   
}

/// This function connects to ORQA FPV.One goggles via TCP socket and performs motorless gimbal while goggles are in use.
static void *orqa_tcp_thread(ORQA_REF camera_t *c){
    // "sudo wpa_supplicant -c /etc/wpa_supplicant.conf -i wlan0" na pločici

    fprintf(stderr, "In thread!\n");
    struct sockaddr_in serveraddr, clientaddr; 
    int childfd;
    char jsonStr[BUFSIZE];
    float yaw, pitch, roll;
    mat4 rollMat; 
    glm_mat4_identity(rollMat);
    versor rollQuat, pitchQuat, yawQuat;
    glm_quat_identity(rollQuat); glm_quat_identity(yawQuat); glm_quat_identity(pitchQuat);
    int optval = 1;
    int portno = 8000;

    // create socket
    int parentfd = socket(AF_INET, SOCK_STREAM, 0);
    if (parentfd < 0) { perror("ERROR opening socket"); exit(1);}
    
    // socket attributes
    setsockopt(parentfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));
    bzero((char *) &serveraddr, sizeof(serveraddr));

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short) portno);

    // binding
    if (bind(parentfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) { perror("ERROR on binding"); exit(1); }

    // listening
    if (listen(parentfd, 5) < 0) { perror("ERROR on listen"); exit(1);}
    unsigned int clientlen = sizeof(clientaddr);

    while (1) {
        fprintf(stderr, "In while\n");
        childfd = accept(parentfd, (struct sockaddr *) &clientaddr, &clientlen); // accepting
        if (childfd < 0) { perror("ERROR on accept"); exit(1);}
        fprintf(stderr, "Accepted!\n");
        // reading
        bzero(jsonStr, BUFSIZE);
        int n = read(childfd, jsonStr, BUFSIZE);
        if (n < 0) { perror("ERROR reading from socket"); exit(1); }
        fprintf(stderr, "server received %d bytes: %s", n, jsonStr);

        // parse JSON
        JSONObject *json = parseJSON(jsonStr);
        yaw = atof(json->pairs[0].value->stringValue);
        pitch = -atof(json->pairs[1].value->stringValue);
        roll = -atof(json->pairs[2].value->stringValue);
        free(json);
        // Using quaternions to calculate camera rotations
        yaw = orqa_radians(yaw); pitch = orqa_radians(pitch); roll = orqa_radians(roll);

        pthread_mutex_lock(&mutexLock);
        glm_quatv(pitchQuat, pitch, (vec3){1.0f, 0.0f, 0.0f}); 
        glm_quatv(yawQuat, yaw, (vec3){0.0f, 1.0f, 0.0f});  
        glm_quatv(rollQuat,roll, (vec3){0.0f, 0.0f, 1.0f}); 
        
        glm_quat_mul(yawQuat, pitchQuat, c->resultQuat);
        glm_quat_mul(c->resultQuat, rollQuat, c->resultQuat);
        glm_quat_normalize(c->resultQuat);
        pthread_mutex_unlock(&mutexLock);
    
        close(childfd);
    }
    return 0;
}

