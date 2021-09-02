#define STB_IMAGE_IMPLEMENTATION
#define JSMN_STATIC

#define ORQA_IN
#define ORQA_REF
#define ORQA_OUT
#define ORQA_NOARGS

#define BUFSIZE 1024

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "../include/stb_image.h" // using this image-loading library
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>	// inet_addr
#include <unistd.h>	// write
#include <string.h>
#include <netinet/in.h>
#include <sys/types.h> 
#include <cglm/cglm.h>
#include "jsmn.h"

// screen resolution
const GLuint SCR_WIDTH = 1920;
const GLuint SCR_HEIGHT = 1080; 
GLFWwindow* window;

// camera position
vec3 cameraPos = (vec3){0.0f, 0.0f, 0.0f};
vec3 cameraFront = (vec3){0.0f, 0.0f, -1.0f};
vec3 cameraUp = (vec3){0.0f, 1.0f, 0.0f};
float yaw = -90.0f; //-90.0f; // yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch = 0.0f;
float roll = 0.0f;
GLfloat fov = 5.0f;

// mouse state
GLboolean firstMouse = GL_TRUE;
GLfloat lastX = SCR_WIDTH/2.0f; 
GLfloat lastY = SCR_HEIGHT/2.0f;

// sphere attributes
const GLfloat radius = 1.0f;
const GLuint sectors = 100; 
const GLuint stacks = 100;
GLuint numVertices;
GLuint numTriangles;
GLfloat *Vs;
GLuint *Is;

// time
// struct timespec start, end;

const GLchar *vertexShaderSource = "#version 460 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec2 aTexCoord;\n"
    "out vec2 TexCoord;\n"
    "uniform mat4 MVP;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = MVP*vec4(aPos, 1.);\n" // lokal space to clip space
    "   TexCoord = vec2(1. - aTexCoord.x, aTexCoord.y);\n" // mirror textures for inside sphere
    "}\n\0";

const GLchar *fragmentShaderSource = "#version 460 core\n"
    "in vec2 TexCoord;\n"
    "out vec4 FragColor;\n"
    "uniform sampler2D texture1;\n" 
    "void main()\n"
    "{\n"
    "   FragColor = texture(texture1, TexCoord);\n" 
    "}\n\0";

int ORQA_initGLFW(ORQA_NOARGS void);
GLfloat ORQA_radians(ORQA_IN const GLfloat deg);
void ORQA_mouse_callback(ORQA_REF GLFWwindow* window, ORQA_IN const GLdouble xpos, ORQA_IN const GLdouble ypos);
void ORQA_GenSphere(ORQA_IN const float radius, ORQA_IN const unsigned int numLatitudeLines, ORQA_IN const unsigned int numLongitudeLines);
void ORQA_processInput(ORQA_REF GLFWwindow *window);
void ORQA_framebuffer_size_callback(ORQA_REF GLFWwindow* window,ORQA_IN GLint width,ORQA_IN GLint height);
void ORQA_scroll_callback(ORQA_REF GLFWwindow* window,ORQA_IN GLdouble xoffset,ORQA_IN GLdouble yoffset);

void* ORQA_tcp_thread();

void error(char *msg) {
    perror(msg);
    exit(1);
}

int main(){
    if (ORQA_initGLFW() == -1) return 0;

    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Learning OpenGL", NULL, NULL); // glfw window object creation
    if (window == NULL){
        fprintf(stderr, "In file: %s, line: %d Failed to create GLFW window\n", __FILE__, __LINE__);
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, ORQA_framebuffer_size_callback);
    glfwSetCursorPosCallback(window, ORQA_mouse_callback);
    glfwSetScrollCallback(window, ORQA_scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // use cursor but do not display it

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){ // glad: load all OpenGL function pointers. GLFW gives us glfwGetProcAddress that defines the correct function based on which OS we're compiling for
        fprintf(stderr, "In file: %s, line: %d Failed to create initialize GLAD\n", __FILE__, __LINE__);
        glfwTerminate();
        return -1;
    }    

    // generating sphere
    ORQA_GenSphere(radius, sectors, stacks);
    GLfloat vertices[numVertices*5];
    GLuint indices[numTriangles*3];
    for(unsigned int i = 0; i < numVertices*5; i++) vertices[i] = *(Vs + i);
    for(unsigned int i = 0; i < numTriangles*3; i++) indices[i] = *(Is + i);

    // shader stuff
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER); 
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success){
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        fprintf(stderr, "In file: %s, line: %d ERROR::SHADER::VERTEX::COMPILATION_FAILED\nError:\n%s\n", __FILE__, __LINE__, infoLog);
        goto shaderError; 
    }
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success){
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        fprintf(stderr, "In file: %s, line: %d ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\nError:\n%s\n", __FILE__, __LINE__, infoLog);
        goto shaderError;
    }
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        fprintf(stderr, "In file: %s, line: %d ERROR::SHADER::PROGRAM::LINKING_FAILED\nError:\n%s\n", __FILE__, __LINE__, infoLog);
        goto linkingError; 
    }

    GLuint VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO); 
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO); 
    glBindBuffer(GL_ARRAY_BUFFER , VBO);
    glBufferData(GL_ARRAY_BUFFER , sizeof(vertices), vertices, GL_STATIC_DRAW );
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER , EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER , sizeof(indices), indices, GL_STATIC_DRAW );

    GLuint positionLocation = glGetAttribLocation(shaderProgram, "aPos");
    GLuint texCoordLocation = glGetAttribLocation(shaderProgram, "aTexCoord");

    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (float*)0);
    glEnableVertexAttribArray(positionLocation);
    glVertexAttribPointer(texCoordLocation, 2, GL_FLOAT, GL_FALSE,  5 * sizeof(float), (void*)(3* sizeof(float)));
    glEnableVertexAttribArray(texCoordLocation);

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // load and create a texture 
    GLint width, height, nrChannels;
    unsigned char *data = stbi_load("../data/panorama1.bmp", &width, &height, &nrChannels, 0); 
    // unsigned char *data = stbi_load("../data/result2.jpg", &width, &height, &nrChannels, 0); 
    // unsigned char *data = stbi_load("../data/earth.jpg", &width, &height, &nrChannels, 0); 
    
    if (data){
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else fprintf(stderr, "In file: %s, line: %d Failed to load texture\n", __FILE__, __LINE__);
    stbi_image_free(data);

    // MVP matrices
    mat4 model, projection, view, temp, MVP;
    vec3 cameraCentar;
    GLuint MVPLoc = glGetUniformLocation(shaderProgram, "MVP");

    glm_mat4_identity(model);
    glm_mat4_identity(view);
    glm_mat4_identity(projection);

    glm_vec3_add(cameraPos, cameraFront, cameraCentar);
    glm_lookat(cameraPos, cameraCentar, cameraUp, view);
    
    glm_mat4_mul(view, model, temp);
    glm_mat4_mul(projection, temp, MVP);

    // pthread_t tcp_thread;
    // pthread_create(&tcp_thread, NULL, ORQA_tcp_thread, NULL);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glEnable(GL_DEPTH_TEST);
    
    while (!glfwWindowShouldClose(window)){ // render loop
        // input
        ORQA_processInput(window);

        // render
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shaderProgram);

        // zoom and rotate
        glm_perspective(fov, (GLfloat)SCR_WIDTH / (GLfloat)SCR_HEIGHT, 0.01f, 100.0f, projection);
        
        glm_mat4_mul(view, model, MVP);
        glm_mat4_mul(projection, MVP, MVP);

        glm_vec3_add(cameraPos, cameraFront, cameraCentar);
        glm_lookat(cameraPos, cameraCentar, cameraUp, view);

        // send MVP matrix to vertex shader
        glUniformMatrix4fv(MVPLoc, 1, GL_FALSE, &MVP[0][0]); 

        // build texture
        glBindTexture(GL_TEXTURE_2D, texture);
        
        // draw
        glDrawElements(GL_TRIANGLES, sizeof(vertices), GL_UNSIGNED_INT, 0);
        
        // glfw: swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // deallocating stuff
    // pthread_exit(NULL);
    free(Vs);
    free(Is);
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
    return 0;
}

/*
clock_gettime(CLOCK_REALTIME, &start);
clock_gettime(CLOCK_REALTIME, &end);

double time = (end.tv_nsec - start.tv_nsec);
printf("%f\n", time);
*/


void ORQA_GenCupola(const float radius, const unsigned int latitude, const unsigned int longitude){
    numVertices = (longitude+1)*latitude;
    GLfloat *verticesX = calloc(numVertices, sizeof(GLfloat));
    GLfloat *verticesY = calloc(numVertices, sizeof(GLfloat));
    GLfloat *verticesZ = calloc(numVertices, sizeof(GLfloat));
    GLfloat *textures1 = calloc(numVertices, sizeof(GLfloat));
    GLfloat *textures2 = calloc(numVertices, sizeof(GLfloat));
    GLfloat drho = M_PI / (GLfloat)latitude;
    GLfloat dtheta = 2*M_PI / (GLfloat)longitude;
    const GLfloat latitudeSpacing = 1.0f / (latitude + 1.0f);
    const GLfloat longitudeSpacing = 1.0f / (longitude);

    for (GLuint i = 0; i < latitude; i++){
        const GLfloat rho = (GLfloat)i * drho;
        const GLfloat srhodrho = (GLfloat)(sinf(rho + drho));
        const GLfloat crhodrho = (GLfloat)(cosf(rho + drho));

        for (GLuint j = 0; j <= longitude; j++){
            const GLfloat theta = (j == longitude) ? 0.0f : j * dtheta;
            const GLfloat stheta = (GLfloat)(-sinf(theta));
            const GLfloat ctheta = (GLfloat)(cosf(theta));

            GLfloat x = stheta * srhodrho;
            GLfloat y = ctheta * srhodrho;
            GLfloat z = crhodrho;

            *(verticesX + latitude*i + j) = x * radius;
            *(verticesY + latitude*i + j) = y * radius;
            *(verticesZ + latitude*i + j) = z * radius;

            *(textures1 + latitude*i + j) = i * longitudeSpacing; 
            *(textures2 + latitude*i + j) = 1.0f - (i + 1) * latitudeSpacing;
        }
    }
    Vs = calloc(numVertices*5, sizeof(GLfloat));
    GLuint j = 0;
    for(GLuint i = 0; i < 5*numVertices; i=i+5){
        *(Vs + i) = *(verticesX+j);
        *(Vs + i+1) = *(verticesY+j);
        *(Vs + i+2) = *(verticesZ+j);
        *(Vs + i+3) = *(textures1+j);
        *(Vs + i+4) = *(textures2+j);
        j++;
    }
    free(verticesX);
    free(verticesY);
    free(verticesZ);
    numTriangles = (longitude * latitude + longitude)*2;
    Is = calloc(numTriangles*3, sizeof(GLint));
    j = 0;
    for (GLuint i = 0; i < longitude * latitude + longitude; ++i){
        *(Is + j++) = i;
        *(Is + j++) = i + longitude + 1;
        *(Is + j++) = i + longitude;
        
        *(Is + j++) = i + longitude + 1;
        *(Is + j++) = i;
        *(Is + j++) = i + 1;
    }
}

void ORQA_GenSphere(ORQA_IN const GLfloat radius, ORQA_IN const GLuint numLatitudeLines, ORQA_IN const GLuint numLongitudeLines){
    // One vertex at every latitude-longitude intersection, plus one for the north pole and one for the south.
    numVertices = numLatitudeLines * (numLongitudeLines + 1) + 2; 
    GLfloat *verticesX = calloc(numVertices, sizeof(GLfloat));
    GLfloat *verticesY = calloc(numVertices, sizeof(GLfloat));
    GLfloat *verticesZ = calloc(numVertices, sizeof(GLfloat));
    GLfloat *textures1 = calloc(numVertices, sizeof(GLfloat));
    GLfloat *textures2 = calloc(numVertices, sizeof(GLfloat));

    // poles
    *(verticesX) = 0; *(verticesY) = radius; *(verticesZ) = 0;
    *(textures1) = 0; *(textures2) = 1;

    *(verticesX + numVertices-1) = 0; *(verticesY+ numVertices-1) = -radius; *(verticesZ+ numVertices-1) = 0;
    *(textures1 + numVertices-1) = 0; *(textures2+ numVertices-1) = 0;

    GLuint k = 1;
    const GLfloat latitudeSpacing = 1.0f / (numLatitudeLines + 1.0f);
    const GLfloat longitudeSpacing = 1.0f / (numLongitudeLines);
    // vertices
    for(GLuint latitude = 0; latitude < numLatitudeLines; latitude++) {
        for(GLuint longitude = 0; longitude <= numLongitudeLines; longitude++){
            *(textures1 + k) = longitude * longitudeSpacing; 
            *(textures2 + k) = 1.0f - (latitude + 1) * latitudeSpacing;
            const GLfloat theta = (GLfloat)(*(textures1 + k) * 2.0f * M_PI);
            const GLfloat phi = (GLfloat)((*(textures2 + k) - 0.5f) * M_PI);
            const GLfloat c = (GLfloat)cos(phi);
            *(verticesX + k) = c * cos(theta) * radius; 
            *(verticesY + k) = sin(phi) * radius; 
            *(verticesZ + k) = c * sin(theta) * radius;
            k++;
        }
    }

    Vs = calloc(numVertices*5, sizeof(GLfloat));
    GLuint j = 0;
    for(GLuint i = 0; i < 5*numVertices;){
        *(Vs + i++) = *(verticesX+j);
        *(Vs + i++) = *(verticesY+j);
        *(Vs + i++) = *(verticesZ+j);
        *(Vs + i++) = *(textures1+j);
        *(Vs + i++) = *(textures2+j);
        j++;
    }
    
    // indices
    numTriangles = numLatitudeLines * numLongitudeLines * 2;
    Is = calloc((numTriangles)*3, sizeof(GLint));
    j = 0;
    // pole one indices
    for (GLuint i = 0; i < numLongitudeLines; i++){
        *(Is + j++) = 0;
        *(Is + j++) = i + 2;
        *(Is + j++) = i + 1;
    }
    // no pole indices
    GLuint rowLength = numLongitudeLines + 1;
    for (GLuint latitude = 0; latitude < numLatitudeLines - 1; latitude++){
        GLuint rowStart = (latitude * rowLength) + 1;
        for (GLuint longitude = 0; longitude < numLongitudeLines; longitude++){
            GLuint firstCorner = rowStart + longitude;
            // First triangle of quad: Top-Left, Bottom-Left, Bottom-Right
            *(Is + j++) = firstCorner;
            *(Is + j++) = firstCorner + rowLength + 1;
            *(Is + j++) = firstCorner + rowLength;
            // Second triangle of quad: Top-Left, Bottom-Right, Top-Right
            *(Is + j++) = firstCorner;
            *(Is + j++) = firstCorner + 1;
            *(Is + j++) = firstCorner + rowLength + 1;
        }        
    }
    // pole two indices
    GLuint pole = numVertices-1;
    GLuint bottomRow = ((numLatitudeLines - 1) * rowLength) + 1;
    for (GLuint i = 0; i < numLongitudeLines; i++){
        *(Is + j++) = pole;
        *(Is + j++) = bottomRow + i;
        *(Is + j++) = bottomRow + i + 1;
    }
}

GLfloat ORQA_radians(ORQA_IN const GLfloat deg){ // calculate radians
    return (deg*M_PI/180.0f); 
}

int ORQA_initGLFW(ORQA_NOARGS void){ // glfw: we first initialize GLFW with glfwInit, after which we can configure GLFW using glfwWindowHint


    if(!glfwInit()){
        fprintf(stderr, "In file: %s, line: %d Failed to initialize GLFW\n", __FILE__, __LINE__);
        glfwTerminate();
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // Specify API version 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // Specify API version 3.3
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // creating contex profile
    
    return 0;
}

void ORQA_processInput(ORQA_REF GLFWwindow *window){ // keeps all the input code
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){ // closes window on ESC
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}

void ORQA_framebuffer_size_callback(ORQA_REF GLFWwindow* window,ORQA_IN GLint width,ORQA_IN GLint height){
    glViewport(0, 0, width, height); // size of the rendering window
}

void ORQA_scroll_callback(ORQA_REF GLFWwindow* window,ORQA_IN GLdouble xoffset,ORQA_IN GLdouble yoffset){
    fov -= (GLfloat)yoffset/5;
    if (fov < 5.0f) fov = 5.0f;
    if (fov > 6.2f) fov = 6.2f;
}

void ORQA_mouse_callback(ORQA_REF GLFWwindow* window, ORQA_IN const GLdouble xpos, ORQA_IN const GLdouble ypos){
    if(firstMouse){ // x/yoffset needs to be 0 at first call
        lastX = xpos;
        lastY = ypos;
        firstMouse = GL_FALSE;
    }
    GLfloat xoffset = xpos - lastX;
    GLfloat yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    GLfloat sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // when pitch is out of bounds, screen doesn't get flipped
    if(pitch > 89.0f) pitch = 89.0f;
    if(pitch < -89.0f) pitch = -89.0f;

    vec3 front;
    front[0] = cos(ORQA_radians(yaw)) * cos(ORQA_radians(pitch));
    front[1] = sin(ORQA_radians(pitch));
    front[2] = sin(ORQA_radians(yaw)) * cos(ORQA_radians(pitch));

    glm_vec3_normalize(front);

    cameraFront[0] = front[0];
    cameraFront[1] = front[1];
    cameraFront[2] = front[2];
}

void* ORQA_tcp_thread(){

    int parentfd, childfd, portno, clientlen, optval, n, err, exit; 
    struct sockaddr_in serveraddr, clientaddr; 
    char buf[BUFSIZE];

    jsmn_parser p;
    jsmntok_t t[3];
    jsmn_init(&p);

    portno = 8000;

    parentfd = socket(AF_INET, SOCK_STREAM, 0);
    if (parentfd < 0) error("ERROR opening socket");

    optval = 1;
    setsockopt(parentfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));
    bzero((char *) &serveraddr, sizeof(serveraddr));

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)portno);

    if (bind(parentfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) error("ERROR on binding");

    if (listen(parentfd, 5) < 0) error("ERROR on listen");

    clientlen = sizeof(clientaddr);
    exit = 0;
    while (exit) {
        childfd = accept(parentfd, (struct sockaddr *) &clientaddr, &clientlen);
        if (childfd < 0) error("ERROR on accept");
        
         
        bzero(buf, BUFSIZE);
        n = read(childfd, buf, BUFSIZE);
        if (n < 0) error("ERROR reading from socket");
        printf("server received.. %d bytes: %s", n, buf);

        err = jsmn_parse(&p, buf, strlen(buf), t, 3);
        if (err < 0) {
            printf("Failed to parse JSON: %d\n", err);
            // exit = 1;
        }
        
        // isparsirati yaw i pitch
        
        vec3 front;
        front[0] = cos(ORQA_radians(yaw)) * cos(ORQA_radians(pitch));
        front[1] = sin(ORQA_radians(pitch));
        front[2] = sin(ORQA_radians(yaw)) * cos(ORQA_radians(pitch));

        glm_vec3_normalize(front);

        cameraFront[0] = front[0];
        cameraFront[1] = front[1];
        cameraFront[2] = front[2];

        close(childfd);
    }
    return 0;
}