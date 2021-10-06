// ssh root@10.220.27.243

#define STB_IMAGE_IMPLEMENTATION
#define BUFSIZE 1024
#define ORQA_IN
#define ORQA_REF
#define ORQA_OUT
#define ORQA_NOARGS

typedef enum{
    OPENGL_OK           = 0,
    OPENGL_INIT_ERROR   = -1
} OpenGLFlags;

#include "glad.h"
#define GLFW_INCLUDE_NONE
#include "glfw3.h"
#include "video_reader.h"

#include "cglm.h"
#include <stdio.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>	
#include <unistd.h>
#include <netinet/in.h>
#include "json.h"
#include "gen_sphere.h" 
#include "stb_image.h" 

const GLuint SCR_WIDTH = 1920;
const GLuint SCR_HEIGHT = 1080; 

typedef struct camera_t{
    vec3 cameraPos;
    float fov;
    versor resultQuat;
}camera_t;

pthread_mutex_t mutexLock;

const GLchar *vertexShaderSource = 
    "#version 300 es\n"
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

const GLchar *fragmentShaderSource = 
    "#version 300 es\n"
    "precision mediump float;\n"
    "in vec2 TexCoord;\n"
    "out vec4 FragColor;\n"
    "uniform sampler2D texture1;\n" 
    "void main()\n"
    "{\n"
    "   FragColor = texture(texture1, TexCoord);\n" 
    "}\n\0";



static int orqa_GLFW_init(ORQA_NOARGS void);
static  GLfloat orqa_radians(ORQA_IN const GLfloat deg);
static void orqa_process_input(ORQA_REF GLFWwindow *window);
static void orqa_framebuffer_size_callback(ORQA_REF GLFWwindow *window,ORQA_IN GLint width,ORQA_IN GLint height);
static void *orqa_tcp_thread(ORQA_REF camera_t *c);

int main(){ 
    
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
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLES2Loader((GLADloadproc)glfwGetProcAddress)){ // glad: load all OpenGL function pointers. GLFW gives us glfwGetProcAddress that defines the correct function based on which OS we're compiling for
        fprintf(stderr, "In file: %s, line: %d Failed to create initialize GLAD\n", __FILE__, __LINE__);
        glfwTerminate();
        return OPENGL_INIT_ERROR;
    }    

    orqa_sphere_t sph;
    sph.radius = 1.0f; sph.sectors = 50; sph.stacks = 50;
    orqa_gen_sphere(&sph);
    float vertices[sph.numVertices*5]; for(int i = 0; i < sph.numVertices*5; i++) vertices[i] = *(sph.Vs + i);
    unsigned int indices[sph.numTriangles*3]; for(int i = 0; i < sph.numTriangles*3; i++) indices[i] = *(sph.Is + i);
    orqa_sphere_free(&sph);
    
    // shader init, compilation and linking
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER); 
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    GLuint shaderProgram = glCreateProgram();
    GLint success;
    GLchar infoLog[BUFSIZE];

    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
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

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    GLuint width, height, nrChannels;
    unsigned char *data = stbi_load("../../VegvisirProject/data/panorama1.bmp", &width, &height, &nrChannels, 0); 
    if (data){
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
    } else{
        fprintf(stderr, "In file: %s, line: %d Failed to load texture\n", __FILE__, __LINE__);
        goto loadError;
    } 

    // camera init
    camera_t cam;
    cam.cameraPos[0] = 0.0f; cam.cameraPos[1] = 0.0f; cam.cameraPos[2] = 0.0f;
    cam.resultQuat[0] = 0.0f; cam.resultQuat[1] = 0.0f; cam.resultQuat[2] = 0.0f; cam.resultQuat[3] = 1.0f;
    cam.fov = 4.8f;

    printf("CAM POS: %f, %f, %f\n", cam.cameraPos[0], cam.cameraPos[1], cam.cameraPos[2]);

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
        glClear(GL_COLOR_BUFFER_BIT);

        glm_perspective(cam.fov, (GLfloat)SCR_WIDTH / (GLfloat)SCR_HEIGHT, 0.01f, 100.0f, proj);

        glm_quat_look(cam.cameraPos, cam.resultQuat, view);

        // send MVP matrices to vertex shader
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]); 
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]); 
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, &proj[0][0]); 

        // build texture  
        glBindTexture(GL_TEXTURE_2D, texture);

        // draw
        glDrawElements(GL_TRIANGLES, sizeof(vertices), GL_UNSIGNED_INT, 0);

        // glfw: swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }   
    loadError:
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
    // using OpenGL ES 3.1
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1); 
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    
    return OPENGL_OK;
}
/// This function converts radians from degrees.
/// Returns radians in float.
static GLfloat orqa_radians(ORQA_IN const GLfloat deg){ 
    return (deg*M_PI/180.0f); // calculate radians
}


/// This function keeps all the input code.
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

/// This function connects to ORQA FPV.One goggles via TCP socket and performs motorless gimbal while goggles are in use.
static void *orqa_tcp_thread(ORQA_REF camera_t *c){
    // inits 
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
    serveraddr.sin_port = htons((unsigned short)portno);

    // binding
    if (bind(parentfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) { perror("ERROR on binding"); exit(1); }

    // listening
    if (listen(parentfd, 5) < 0) { perror("ERROR on listen"); exit(1);}
    unsigned int clientlen = sizeof(clientaddr);

    while (1) {
        childfd = accept(parentfd, (struct sockaddr *) &clientaddr, &clientlen); // accepting
        if (childfd < 0) { perror("ERROR on accept"); exit(1);}

        // reading
        bzero(jsonStr, BUFSIZE);
        int n = read(childfd, jsonStr, BUFSIZE);
        if (n < 0) { perror("ERROR reading from socket"); exit(1); }
        // printf("server received %d bytes: %s", n, jsonStr);

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

