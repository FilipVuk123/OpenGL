// gcc SphereProject.c glad.c -ldl -lm -lglfw -pipe -Wall -Wextra

#define STB_IMAGE_IMPLEMENTATION

#define ORQA_IN
#define ORQA_REF
#define ORQA_OUT
#define ORQA_NOARGS

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdbool.h> 
#include "stb_image.h" // using this image-loading library
#include <math.h>

#include <cglm/cglm.h>
#include <cglm/common.h>

const vec3 cameraPos = (vec3){2.0f, 0.0f, 0.0f};
const vec3 cameraCentar = (vec3){2.0f, 0.0f, -5.0f};
const vec3 cameraUp = (vec3){0.0f, 1.0f, 0.0f};

GLfloat fov = 5.0f;
const GLdouble rotation = 0.001;
const GLuint SCR_WIDTH = 1920;
const GLuint SCR_HEIGHT = 1080;

const GLfloat radius = 0.7f;
const GLuint sectors = 100; 
const GLuint stacks = 100; 

GLfloat *Vs;
GLuint *Is;

const GLchar *vertexShaderSource = "#version 460 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 aColor;\n"
    "layout (location = 2) in vec2 aTexCoord;\n"
    "out vec3 ourColor;\n"
    "out vec2 TexCoord;\n"
    "uniform mat4 MVP;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = MVP*vec4(aPos, 1.0f);\n"
    "   ourColor = aColor;\n"
    "   TexCoord = vec2(aTexCoord.x, aTexCoord.y);\n"
    "}\n\0";
const GLchar *fragmentShaderSource = "#version 460 core\n"
    "out vec4 FragColor;\n"
    "in vec3 ourColor;\n"
    "in vec2 TexCoord;\n"
    "uniform sampler2D texture1;\n" 
    "void main()\n"
    "{\n"
    "   FragColor = texture(texture1, TexCoord);\n" 
    "}\n\0";

void ORQA_GenSphere(const float radius, const unsigned int stacks, const unsigned int sectors){
    GLfloat *verticesX = calloc((sectors+1)*stacks, sizeof(GLfloat));
    GLfloat *verticesY = calloc((sectors+1)*stacks, sizeof(GLfloat));
    GLfloat *verticesZ = calloc((sectors+1)*stacks, sizeof(GLfloat));
    GLfloat drho = M_PI / (GLfloat)stacks;
    GLfloat dtheta = 2*M_PI / (GLfloat)sectors;

    for (GLuint i = 0; i < stacks; i++){
        const GLfloat rho = (GLfloat)i * drho;
        const GLfloat srhodrho = (GLfloat)(sinf(rho + drho));
        const GLfloat crhodrho = (GLfloat)(cosf(rho + drho));

        for (GLuint j = 0; j <= sectors; j++){
            const GLfloat theta = (j == sectors) ? 0.0f : j * dtheta;
            const GLfloat stheta = (GLfloat)(-sinf(theta));
            const GLfloat ctheta = (GLfloat)(cosf(theta));

            GLfloat x = stheta * srhodrho;
            GLfloat y = ctheta * srhodrho;
            GLfloat z = crhodrho;

            *(verticesX + stacks*i + j) = x * radius;
            *(verticesY + stacks*i + j) = y * radius;
            *(verticesZ + stacks*i + j) = z * radius;
        }
    }
    Vs = calloc(sectors*stacks*3, sizeof(GLfloat));
    GLuint j = 0;
    for(GLuint i = 0; i < 3*sectors*stacks; i=i+3){
        *(Vs + i) = *(verticesX+j);
        *(Vs + i+1) = *(verticesY+j);
        *(Vs + i+2) = *(verticesZ+j);
        j++;
    }
    free(verticesX);
    free(verticesY);
    free(verticesZ);
    Is = calloc((sectors * stacks + sectors)*6, sizeof(GLint));
    j = 0;
    for (GLuint i = 0; i < sectors * stacks + sectors; ++i){
        *(Is + j++) = i;
        *(Is + j++) = i + sectors + 1;
        *(Is + j++) = i + sectors;
        
        *(Is + j++) = i + sectors + 1;
        *(Is + j++) = i;
        *(Is + j++) = i + 1;
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
    fov -= (GLfloat)yoffset/10;
    if (fov < 4.0f) fov = 4.0f;
    if (fov > 6.0f) fov = 6.0f;
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
    
    ORQA_GenSphere(radius, sectors, stacks);
    GLfloat vertices[sectors*stacks*3];
    GLuint indices[(sectors * stacks + sectors)*6];
    for(unsigned int i = 0; i < sectors*stacks*3; i++) vertices[i] = *(Vs + i);
    for(unsigned int i = 0; i < (sectors * stacks + sectors)*6; i++) indices[i] = *(Is + i);
    free(Vs);
    free(Is);
    
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
        goto shaderError; // error handling
    }
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success){
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        fprintf(stderr, "In file: %s, line: %d ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\nError:\n%s\n", __FILE__, __LINE__, infoLog);
        goto shaderError; // error handling
    }

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        fprintf(stderr, "In file: %s, line: %d ERROR::SHADER::PROGRAM::LINKING_FAILED\nError:\n%s\n", __FILE__, __LINE__, infoLog);
        goto linkingError; // error handling
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
    GLuint colorLocation = glGetAttribLocation(shaderProgram, "aColor");
    GLuint texCoordLocation = glGetAttribLocation(shaderProgram, "aTexCoord");

    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(positionLocation);

    glVertexAttribPointer(colorLocation, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(colorLocation);

    glVertexAttribPointer(texCoordLocation, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(texCoordLocation);

    GLint width, height, nrChannels;
    unsigned char *data = stbi_load("../data/earth.jpg", &width, &height, &nrChannels, 0); 
    if (data){
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else fprintf(stderr, "In file: %s, line: %d Failed to load texture\n", __FILE__, __LINE__);
    stbi_image_free(data);

    // glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0); 
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0); 
    
    mat4 model, projection, view, temp, MVP;
    GLuint MVPLoc = glGetUniformLocation(shaderProgram, "MVP");

    glm_mat4_identity(model);
    glm_mat4_identity(view);

    glm_translate(view, (vec3){0,0,-5});

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
        glm_perspective(fov, (GLfloat)SCR_WIDTH / (GLfloat)SCR_HEIGHT, 0.01f, 10.0f, projection);
        
        glm_rotate(model, 0.0f, (vec3){1, 0, 0});

        if(glfwGetKey(window, GLFW_KEY_A)) glm_rotate(model, rotation, (vec3){-0.1f, 0.0f, 0.0f});
        if(glfwGetKey(window, GLFW_KEY_D)) glm_rotate(model, rotation, (vec3){0.1f, 0.0f, 0.0f});
        if(glfwGetKey(window, GLFW_KEY_LEFT)) glm_rotate(model, rotation, (vec3){0.0f, 0.1f, 0.0f});
        if(glfwGetKey(window, GLFW_KEY_RIGHT)) glm_rotate(model, rotation, (vec3){0.0f,-0.1f, 0.0f});
        if(glfwGetKey(window, GLFW_KEY_UP)) glm_rotate(model, rotation, (vec3){0.0f, 0.0f, 0.1f});
        if(glfwGetKey(window, GLFW_KEY_DOWN)) glm_rotate(model, rotation, (vec3){0.0f, 0.0f, -0.1f});

        glm_mat4_mul(view, model, MVP);
        glm_mat4_mul(projection, MVP, MVP);

        glUniformMatrix4fv(MVPLoc,1, GL_FALSE, &MVP[0][0]); 

        // build texture
        glBindTexture(GL_TEXTURE_2D, texture);

        // draw our first triangle
        glDrawElements(GL_TRIANGLES, sizeof(vertices), GL_UNSIGNED_INT, 0);
    
        // glfw: swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

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