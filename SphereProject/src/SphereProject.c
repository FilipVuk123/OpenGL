#define STB_IMAGE_IMPLEMENTATION

#define ORQA_IN
#define ORQA_REF
#define ORQA_OUT
#define ORQA_NOARGS

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "../include/stb_image.h" // using this image-loading library
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <cglm/cglm.h>
#include <cglm/common.h>

// screen resolution
const GLuint SCR_WIDTH = 1920;
const GLuint SCR_HEIGHT = 1080;

// camera position
const vec3 cameraPos = (vec3){0.0f, 0.0f, 0.0f};
const vec3 cameraCentar = (vec3){-1.0f, 0.0f, -1.0f};
const vec3 cameraUp = (vec3){0.0f, 1.0f, 0.0f};

// field of view and rotation speed
GLfloat fov = 5.0f;
const GLdouble rotation = 0.001;

// sphere attributes
const GLfloat radius = 1.0f;
const GLuint sectors = 50; 
const GLuint stacks = 50; 
GLuint numVertices;
GLuint numTriangles;
GLfloat *Vs;
GLuint *Is;

void ORQA_GenSphere(ORQA_IN const float radius, ORQA_IN const unsigned int numLatitudeLines, ORQA_IN const unsigned int numLongitudeLines);
void ORQA_processInput(ORQA_REF GLFWwindow *window);
void ORQA_framebuffer_size_callback(ORQA_REF GLFWwindow* window,ORQA_IN GLint width,ORQA_IN GLint height);
void ORQA_scroll_callback(ORQA_REF GLFWwindow* window,ORQA_IN GLdouble xoffset,ORQA_IN GLdouble yoffset);
int ORQA_initGLFW(ORQA_NOARGS void);

const GLchar *vertexShaderSource = "#version 460 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec2 aTexCoord;\n"
    "out vec2 TexCoord;\n"
    "uniform mat4 MVP;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = MVP*vec4(aPos, 1.0f);\n"
    "   TexCoord = vec2(1.0f - aTexCoord.x, aTexCoord.y);\n" // mirror textures for inside sphere
    "}\n\0";

const GLchar *fragmentShaderSource = "#version 460 core\n"
    "out vec4 FragColor;\n"
    "in vec2 TexCoord;\n"
    "uniform sampler2D texture1;\n" 
    "void main()\n"
    "{\n"
    "   FragColor = texture(texture1, TexCoord);\n" 
    "}\n\0";

int main(){
    if (ORQA_initGLFW() == -1) return 0;

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Learning OpenGL", NULL, NULL); // glfw window object creation
    if (window == NULL){
        fprintf(stderr, "In file: %s, line: %d Failed to create GLFW window\n", __FILE__, __LINE__);
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, ORQA_framebuffer_size_callback);
    glfwSetScrollCallback(window, ORQA_scroll_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){ // glad: load all OpenGL function pointers. GLFW gives us glfwGetProcAddress that defines the correct function based on which OS we're compiling for
        fprintf(stderr, "In file: %s, line: %d Failed to create initialize GLAD\n", __FILE__, __LINE__);
        glfwTerminate();
        return -1;
    }    
    
    // generating sphere
    ORQA_GenSphere(radius, sectors, stacks);
    GLfloat vertices[numVertices*5];
    for(unsigned int i = 0; i < numVertices*5; i++) vertices[i] = *(Vs + i);
    GLuint indices[numTriangles*3];
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

    // load and create a texture 
    GLint width, height, nrChannels;
    unsigned char *data = stbi_load("../data/earth.jpg", &width, &height, &nrChannels, 0); 
    if (data){
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else fprintf(stderr, "In file: %s, line: %d Failed to load texture\n", __FILE__, __LINE__);
    stbi_image_free(data);

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
    
    mat4 model, projection, view, temp, MVP;
    GLuint MVPLoc = glGetUniformLocation(shaderProgram, "MVP");

    glm_mat4_identity(model);
    glm_mat4_identity(view);
    glm_mat4_identity(projection);

    glm_lookat(cameraPos, cameraCentar, cameraUp, view);

    glm_mat4_mul(view, model, temp);
    glm_mat4_mul(projection, temp, MVP);
    
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
        
        glm_rotate(model, 0.0f, (vec3){1, 0, 0});

        if(glfwGetKey(window, GLFW_KEY_A)) glm_rotate(model, rotation, (vec3){-0.1f, 0.0f, 0.0f});
        if(glfwGetKey(window, GLFW_KEY_D)) glm_rotate(model, rotation, (vec3){0.1f, 0.0f, 0.0f});
        if(glfwGetKey(window, GLFW_KEY_LEFT)) glm_rotate(model, rotation, (vec3){0.0f, 0.1f, 0.0f});
        if(glfwGetKey(window, GLFW_KEY_RIGHT)) glm_rotate(model, rotation, (vec3){0.0f,-0.1f, 0.0f});
        if(glfwGetKey(window, GLFW_KEY_UP)) glm_rotate(model, rotation, (vec3){0.0f, 0.0f, -0.1f});
        if(glfwGetKey(window, GLFW_KEY_DOWN)) glm_rotate(model, rotation, (vec3){0.0f, 0.0f, 0.1f});
        glm_mat4_mul(view, model, MVP);
        glm_mat4_mul(projection, MVP, MVP);

        glUniformMatrix4fv(MVPLoc,1, GL_FALSE, &MVP[0][0]); 

        // build texture
        glBindTexture(GL_TEXTURE_2D, texture);

        // draw
        glDrawElements(GL_TRIANGLES, sizeof(vertices), GL_UNSIGNED_INT, 0);
    
        // glfw: swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // deallocating stuff
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

    free(Vs);
    free(Is);

    return 0;
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

void ORQA_processInput(ORQA_REF GLFWwindow *window){ // keeps all the input code
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) // closes window on ESC
        glfwSetWindowShouldClose(window, GL_TRUE);
}

void ORQA_framebuffer_size_callback(ORQA_REF GLFWwindow* window,ORQA_IN GLint width,ORQA_IN GLint height){
    glViewport(0, 0, width, height); // size of the rendering window
}

void ORQA_scroll_callback(ORQA_REF GLFWwindow* window,ORQA_IN GLdouble xoffset,ORQA_IN GLdouble yoffset){
    fov -= (GLfloat)yoffset/5;
    if (fov < 3.8f) fov = 3.8f;
    if (fov > 6.2f) fov = 6.2f;
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

