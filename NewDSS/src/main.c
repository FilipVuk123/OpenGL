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

const GLchar *fragmentShaderSource = 
    "#version 330 core\n"
    "in vec2 TexCoord;\n"
    "out vec4 FragColor;\n"
    "uniform sampler2D texture1;\n" 
    "void main()\n"
    "{\n"
    "       vec3 color = texture2D(texture1, TexCoord).xyz;\n"
    "       gl_FragColor = vec4(color , 1.0);\n"    
    "}\n\0";

const GLfloat verticesDSS[] = {
    -0.69352,   0.19509,    -0.69352,   0,      1,
    -0.81549,   0.19509,    -0.5449,    0.125,  1,
    -0.90613,   0.19509,    -0.37533,   0.25,   1,
    -0.96194,   0.19509,    -0.19134,   0.375,  1,
    -0.98079,   0.19509,    0      ,    0.5,    1,
    -0.96194,   0.19509,    0.19134,    0.625,  1,
    -0.90613,   0.19509,    0.37533,    0.75,   1,
    -0.81549,   0.19509,    0.5449,     0.825,  1,
    -0.69352,   0.19509,    0.69352,    1,      1,

    -0.69352,   -0.19509,   -0.69352,   0,      0,
    -0.81549,   -0.19509,   -0.5449,    0.125,  0,
    -0.90613,   -0.19509,   -0.37533,   0.25,   0,
    -0.96194,   -0.19509,   -0.19134,   0.375,  0,
    -0.98079,    -0.19509,  0       ,   0.5,    0,
    -0.96194,   -0.19509,   0.19134,    0.625,  0,
    -0.90613,   -0.19509,   0.37533,    0.75,   0,
    -0.81549,   -0.19509,   0.5449,     0.875,  0,
    -0.69352,   -0.19509,   0.69352,    1,      0
};
const GLuint indicesDSS[] = {
    0,1,9, 1, 9, 10,
    1,2,10,2,10,11,
    2,3,11,3,11,12,
    3,4,12,4,12,13,
    4,5,13,5,13,14,
    5,6,14,6,14,15,
    6,7,15,7,15,16,
    7,8,16,8,16,17
};

const GLfloat verticesLR[] = {
    -0.39195, -0.55557, -0.73329, 0, 0,
    -0.58794, -0.55557, -0.58794, 0.5, 0,
    -0.73329, -0.55557, -0.39195, 1, 0,

    -0.4511,  -0.29028, -0.84395, 0,1,
    -0.67666, -0.29028, -0.67666, 0.5, 1,
    -0.84395, -0.29028, -0.4511, 1, 1
    
};
const GLfloat verticesRR[] = {
    -0.39195, -0.55557, 0.73329, 0, 0,
    -0.58794, -0.55557, 0.58794, 0.5, 0,
    -0.73329, -0.55557, 0.39195, 1, 0,

    -0.4511,  -0.29028, 0.84395, 0,1,
    -0.67666, -0.29028, 0.67666, 0.5, 1,
    -0.84395, -0.29028, 0.4511, 1, 1
};
const GLuint indicesR[] = {
    0,1,3, 1,3,4,
    1,2,4,2,4,5,
};

const GLfloat verticesMRSS[] = {
    -0.76818, -0.55557, -0.31819,   0, 0,
    -0.81549, -0.55557, -0.16221,   0.25, 0,
    -0.83147, -0.55557, 0,          0.5, 0,
    -0.81549, -0.55557, 0.16221,    0.75, 0,
    -0.76818, -0.55557, 0.31819,    1, 0,

    -0.8841,  -0.29028, -0.36621,     0, 1,
    -0.93855, -0.29028, -0.18669,    0.25, 1,
    -0.95694, -0.29028, 0,           0.5, 1,
    -0.93855, -0.29028, 0.18669,    0.75, 1,
    -0.8841,  -0.29028, 0.36621,     1, 1,
};
const GLuint indicesMRSS[] = {
    0,1,5,1,5,6,
    1,2,6,2,6,7,
    2,3,7,3,7,8,
    3,4,8,4,7,9
};

const GLfloat verticesBV [] = {
    0.18024, 0.90613, -0.38268,   0, 1,
    0.19134, 0.96194, -0.19509,   0.25, 1,
    0.19509, 0.98078, 0,          0.5, 1,
    0.19134, 0.96194, 0.19509,    0.75, 1,
    0.18024, 0.90613, 0.38268,    1, 1,

    -0.18024, 0.90613, -0.38268,  0, 0,
    -0.19134, 0.96194, -0.19509,  0.25, 0,
    -0.19509, 0.98078, 0,         0.5, 0,
    -0.19134, 0.96194, 0.19509,   0.75, 0,
    -0.18024, 0.90613, 0.38268,   1, 0
};
const GLuint indicesBV[] = {
    0,1,5,1,5,6,
    1,2,6,2,6,7,
    2,3,7,3,7,8,
    3,4,8,4,8,9
};


static int orqa_GLFW_init(ORQA_NOARGS void);
static  GLfloat orqa_radians(ORQA_IN const GLfloat deg);
static  void orqa_mouse_callback(ORQA_REF GLFWwindow *window, ORQA_IN const GLdouble xpos, ORQA_IN const GLdouble ypos);
static void orqa_process_input(ORQA_REF GLFWwindow *window);
static void orqa_framebuffer_size_callback(ORQA_REF GLFWwindow *window,ORQA_IN GLint width,ORQA_IN GLint height);
static void orqa_scroll_callback(ORQA_REF GLFWwindow *window,ORQA_IN GLdouble xoffset,ORQA_IN GLdouble yoffset);
static void* orqa_tcp_thread(ORQA_REF void *c_ptr);

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
    glfwSetCursorPosCallback(window, orqa_mouse_callback); // move camera_t with cursor
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // use cursor but do not display it
    glfwSetScrollCallback(window, orqa_scroll_callback); // zoom in/out using mouse wheel

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){ // glad: load all OpenGL function pointers. GLFW gives us glfwGetProcAddress that defines the correct function based on which OS we're compiling for
        fprintf(stderr, "In file: %s, line: %d Failed to create initialize GLAD\n", __FILE__, __LINE__);
        glfwTerminate();
        return OPENGL_INIT_ERROR;
    }    

    // shader init, compilation and linking
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER); 
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    GLuint shaderProgram = glCreateProgram();
    GLint success;
    GLchar infoLog[BUFSIZE];

    glShaderSource(vertexShader, 1, &vertexShaderSource360, NULL);
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

    // get indexes for shader variables
    GLuint posLoc = glGetAttribLocation(shaderProgram, "aPos");
    GLuint texLoc = glGetAttribLocation(shaderProgram, "aTexCoord");

    // init & binding array & buffer objects
    GLuint VBOs[5], VAOs[5], EBOs[5];
    glGenVertexArrays(5, VAOs);
    glGenBuffers(5, VBOs); 
    glGenBuffers(5, EBOs); 

    glBindVertexArray(VAOs[0]);
    glBindBuffer(GL_ARRAY_BUFFER , VBOs[0]);
    glBufferData(GL_ARRAY_BUFFER , sizeof(verticesRR), verticesRR, GL_STATIC_DRAW );
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER , EBOs[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER , sizeof(indicesR), indicesR, GL_STATIC_DRAW );
    glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (float*)0);
    glEnableVertexAttribArray(posLoc);
    glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE,  5 * sizeof(float), (void*)(3* sizeof(float)));
    glEnableVertexAttribArray(texLoc);
    
    glBindVertexArray(VAOs[1]);
    glBindBuffer(GL_ARRAY_BUFFER , VBOs[1]);
    glBufferData(GL_ARRAY_BUFFER , sizeof(verticesLR), verticesLR, GL_STATIC_DRAW );
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER , EBOs[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER , sizeof(indicesR), indicesR, GL_STATIC_DRAW );
    glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (float*)0);
    glEnableVertexAttribArray(posLoc);
    glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE,  5 * sizeof(float), (void*)(3* sizeof(float)));
    glEnableVertexAttribArray(texLoc);
    
    glBindVertexArray(VAOs[2]);
    glBindBuffer(GL_ARRAY_BUFFER , VBOs[2]);
    glBufferData(GL_ARRAY_BUFFER , sizeof(verticesMRSS), verticesMRSS, GL_STATIC_DRAW );
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER , EBOs[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER , sizeof(indicesMRSS), indicesMRSS, GL_STATIC_DRAW );
    glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (float*)0);
    glEnableVertexAttribArray(posLoc);
    glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE,  5 * sizeof(float), (void*)(3* sizeof(float)));
    glEnableVertexAttribArray(texLoc);
    
    glBindVertexArray(VAOs[3]);
    glBindBuffer(GL_ARRAY_BUFFER , VBOs[3]);
    glBufferData(GL_ARRAY_BUFFER , sizeof(verticesDSS), verticesDSS, GL_STATIC_DRAW );
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER , EBOs[3]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER , sizeof(indicesDSS), indicesDSS, GL_STATIC_DRAW );
    glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (float*)0);
    glEnableVertexAttribArray(posLoc);
    glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE,  5 * sizeof(float), (void*)(3* sizeof(float)));
    glEnableVertexAttribArray(texLoc);
    
    glBindVertexArray(VAOs[4]);
    glBindBuffer(GL_ARRAY_BUFFER , VBOs[4]);
    glBufferData(GL_ARRAY_BUFFER , sizeof(verticesBV), verticesBV, GL_STATIC_DRAW );
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER , EBOs[4]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER , sizeof(indicesBV), indicesBV, GL_STATIC_DRAW );
    glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (float*)0);
    glEnableVertexAttribArray(posLoc);
    glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE,  5 * sizeof(float), (void*)(3* sizeof(float)));
    glEnableVertexAttribArray(texLoc);

    // camera init
    camera_t cam;
    cam.cameraPos[0] = 0.0f; cam.cameraPos[1] = 0.0f; cam.cameraPos[2] = 0.0f;
    cam.resultQuat[0] = 0.0f; cam.resultQuat[1] = 0.0f; cam.resultQuat[2] = 0.0f; cam.resultQuat[3] = 1.0f;
    cam.fov = 5.6f;
    glfwSetWindowUserPointer(window, &cam); // sent camera object to callback functions

    // TCP thread & mutex init
    pthread_t tcp_thread;
    pthread_create(&tcp_thread, NULL, orqa_tcp_thread, &cam);
    if (pthread_mutex_init(&mutexLock, NULL) != 0) {
        fprintf(stderr, "Mutex init has failed! \n");
        goto threadError;
    }
    // texture init
    GLuint textures[3];
    glGenTextures(3, textures);


    // loading image!
    int width, height, nrChannels;
    unsigned char *data;
    data = stbi_load("../data/MRSS.png", &width, &height, &nrChannels, 0); 
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    if (data){
        if ((int) nrChannels == 3)  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        else if ((int) nrChannels == 4)glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
    }else{
        fprintf(stderr, "In file: %s, line: %d Failed to load texture\n", __FILE__, __LINE__);
        goto loadError;
    } 

    data = stbi_load("../data/DSS.png", &width, &height, &nrChannels, 0); 
    glBindTexture(GL_TEXTURE_2D, textures[1]);
    if (data){
        if ((int) nrChannels == 3)  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        else if ((int) nrChannels == 4)glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
    }else{
        fprintf(stderr, "In file: %s, line: %d Failed to load texture\n", __FILE__, __LINE__);
        goto loadError;
    } 

    data = stbi_load("../data/result2.jpg", &width, &height, &nrChannels, 0); 
    glBindTexture(GL_TEXTURE_2D, textures[2]);
    if (data){
        if ((int) nrChannels == 3)  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        else if ((int) nrChannels == 4)glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
    }else{
        fprintf(stderr, "In file: %s, line: %d Failed to load texture\n", __FILE__, __LINE__);
        goto loadError;
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
    int c = 0;
    while (1){ // render loop
        c++;
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

        

        // build texture  
        glBindTexture(GL_TEXTURE_2D, textures[0]);

        // draw stu
        glBindVertexArray(VAOs[2]);
        glDrawElements(GL_TRIANGLES, sizeof(indicesMRSS)/sizeof(indicesMRSS[0]), GL_UNSIGNED_INT, 0);

        glBindTexture(GL_TEXTURE_2D, textures[2]);
        glBindVertexArray(VAOs[0]);
        glDrawElements(GL_TRIANGLES, sizeof(indicesR)/sizeof(indicesR[0]), GL_UNSIGNED_INT, 0);
        
        glBindVertexArray(VAOs[1]);
        glDrawElements(GL_TRIANGLES, sizeof(indicesR)/sizeof(indicesR[0]), GL_UNSIGNED_INT, 0);

        
        glBindVertexArray(VAOs[4]); 
        glDrawElements(GL_TRIANGLES, sizeof(indicesBV)/sizeof(indicesBV[0]), GL_UNSIGNED_INT, 0);

        glBindTexture(GL_TEXTURE_2D, textures[1]);
        glBindVertexArray(VAOs[3]);
        glDrawElements(GL_TRIANGLES, sizeof(indicesDSS)/sizeof(indicesDSS[0]), GL_UNSIGNED_INT, 0);
        
        // glfw: swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // deallocating stuff
    loadError:
    threadError:
    pthread_exit(NULL);
    glDeleteVertexArrays(5, VAOs); 
    glDeleteBuffers(5, VBOs);
    glDeleteBuffers(5, EBOs);
    glDeleteTextures(3, textures);
    
    
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

/// This function connects to ORQA FPV.One goggles via TCP socket and performs motorless gimbal while goggles are in use.
static void *orqa_tcp_thread(ORQA_REF void *c_ptr){
    // inits 
    fprintf(stderr, "In thread");
    camera_t *c = c_ptr;
    float yaw, pitch, roll;
    mat4 rollMat; 
    glm_mat4_identity(rollMat);
    versor rollQuat, pitchQuat, yawQuat;
    glm_quat_identity(rollQuat); glm_quat_identity(yawQuat); glm_quat_identity(pitchQuat);
    int optval = 1;
    int portno = 8000;
    struct sockaddr_in serveraddr, clientaddr; 
    int childfd;
    char jsonStr[BUFSIZE];

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
        // orqa_clock_t clock = orqa_time_now();
        childfd = accept(parentfd, (struct sockaddr *) &clientaddr, &clientlen); // accepting
        if (childfd < 0) { perror("ERROR on accept"); exit(1);}

        // reading
        bzero(jsonStr, BUFSIZE);
        int n = read(childfd, jsonStr, BUFSIZE);
        if (n < 0) { perror("ERROR reading from socket"); exit(1); }

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
        // printf("%.2lf\n", orqa_get_time_diff_msec(clock, orqa_time_now()));
    }
    return 0;
}