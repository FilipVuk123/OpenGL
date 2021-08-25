#define STB_IMAGE_IMPLEMENTATION

#define ORQA_IN
#define ORQA_REF
#define ORQA_OUT
#define ORQA_NOARGS

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "../include/stb_image.h" // using this image-loading library
#include <vector>
#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Sphere{
    public:
        std::vector<GLfloat> vertices;
        std::vector<GLint> indices;
    private:
        float radius;
        unsigned int stacks, sectors;
    public:
    Sphere(float radius, unsigned int stacks, unsigned int sectors): radius(radius), stacks(stacks), sectors(sectors){};
    void generate(){
        for (unsigned int i = 0; i <= stacks; ++i){

            float V   = i / (float) stacks;
            float phi = V * M_PI;

            for (unsigned int j = 0; j <= sectors; ++j){

                float U = j / (float) sectors;
                float theta = U * (M_PI * 2);

                float x = cosf (theta) * sinf (phi);
                float y = cosf (phi);
                float z = sinf (theta) * sinf (phi);

                vertices.push_back (x * radius);
                vertices.push_back (y * radius);
                vertices.push_back (z * radius);                
            }
        }
        for (unsigned int i = 0; i < sectors * stacks + sectors; ++i){
            indices.push_back(i);
            indices.push_back(i + sectors + 1);
            indices.push_back(i + sectors);
            
            indices.push_back(i + sectors + 1);
            indices.push_back(i);
            indices.push_back(i + 1);
        }
    }
};

void ORQA_processInput(ORQA_REF GLFWwindow *window){ // keeps all the input code
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) // closes window on ESC
        glfwSetWindowShouldClose(window, GL_TRUE);
}

void ORQA_framebuffer_size_callback(ORQA_REF GLFWwindow* window,ORQA_IN GLint width,ORQA_IN GLint height){
    glViewport(0, 0, width, height); // size of the rendering window
}

GLfloat fov = 50.0f;
const GLuint SCR_WIDTH = 800;
const GLuint SCR_HEIGHT = 600;

// camera
glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset){
    fov -= (float)yoffset;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 100.0f)
        fov = 100.0f;
}

const GLchar *vertexShaderSource = "#version 460 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 aColor;\n"
    "layout (location = 2) in vec2 aTexCoord;\n"
    "out vec3 ourColor;\n"
    "out vec2 TexCoord;\n"
    "uniform mat4 modelMatrix;\n"
    "uniform mat4 projMatrix;\n"
    "uniform mat4 viewMatrix;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = projMatrix*viewMatrix*modelMatrix*vec4(aPos, 1.0f);\n" // projMatrix*viewMatrix*modelMatrix*vec4()
    "   ourColor = aColor;\n"
    "   TexCoord = aTexCoord;\n"
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
    glfwSetScrollCallback(window, scroll_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){ // glad: load all OpenGL function pointers. GLFW gives us glfwGetProcAddress that defines the correct function based on which OS we're compiling for
        fprintf(stderr, "In file: %s, line: %d Failed to create initialize GLAD\n", __FILE__, __LINE__);
        return -1;
    }    

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
        glfwTerminate();
        return 0;
    }
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success){
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        fprintf(stderr, "In file: %s, line: %d ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\nError:\n%s\n", __FILE__, __LINE__, infoLog);
        glfwTerminate();
        return 0;
    }
    // creating shaderProgram
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        fprintf(stderr, "In file: %s, line: %d ERROR::SHADER::PROGRAM::LINKING_FAILED\nError:\n%s\n", __FILE__, __LINE__, infoLog);
        glfwTerminate();
        return 0;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // generating Sphere vertices 
    Sphere s1(1.0, 25, 25);
    s1.generate();

    GLfloat vertices[s1.vertices.size()];
    std::copy(s1.vertices.begin(), s1.vertices.end(), vertices);
    GLint indices[s1.indices.size()];
    std::copy(s1.indices.begin(), s1.indices.end(), indices);

    GLuint VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO); 
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO); 

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    GLint positionLocation = glGetAttribLocation(shaderProgram, "aPos");
    GLint colorLocation = glGetAttribLocation(shaderProgram, "aColor");
    GLint texCoordLocation = glGetAttribLocation(shaderProgram, "aTexCoord");

    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(positionLocation);

    glVertexAttribPointer(colorLocation, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(colorLocation);

    glVertexAttribPointer(texCoordLocation, 2, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(texCoordLocation);
    
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_BUFFER, texture);
    
    glTexParameteri(GL_TEXTURE_BUFFER, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_BUFFER, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

    glTexParameteri(GL_TEXTURE_BUFFER, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_BUFFER, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLint width, height, nrChannels;
    unsigned char *data = stbi_load("../data/image", &width, &height, &nrChannels, 0);
    if (data){
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else fprintf(stderr, "In file: %s, line: %d Failed to load texture\n", __FILE__, __LINE__);
    stbi_image_free(data);

    const float rotation = 0.1f;
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 proj = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);

    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    glEnable(GL_DEPTH_TEST); // tells opengl which triangles to draw


    while (!glfwWindowShouldClose(window)){ // render loop
        
        // input
        ORQA_processInput(window);

        // render
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shaderProgram);
        
        // zoom
        int projLoc = glGetUniformLocation(shaderProgram, "projMatrix");
        proj = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

        // view
        int viewLoc = glGetUniformLocation(shaderProgram, "viewMatrix");
        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        // rotation
        int modelLoc = glGetUniformLocation(shaderProgram, "modelMatrix");
        model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        
        if(glfwGetKey(window, GLFW_KEY_A)){
            model = glm::rotate(model, glm::radians(rotation), glm::vec3(-1.0f, 0.0f, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        }
        if(glfwGetKey(window, GLFW_KEY_D)){
            model = glm::rotate(model, glm::radians(rotation), glm::vec3(1.0f, 0.0f, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        }
        if(glfwGetKey(window, GLFW_KEY_LEFT)){
            model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        }
        if(glfwGetKey(window, GLFW_KEY_RIGHT)){
            model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, -1.0f, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        }
        if(glfwGetKey(window, GLFW_KEY_UP)){
            model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        }
        if(glfwGetKey(window, GLFW_KEY_DOWN)){
            model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, -1.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        }

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
    glDeleteProgram(shaderProgram);
    glfwTerminate(); // glfw: terminate, clearing all previously allocated GLFW resources.

    return 0;
}