// gcc learning.c glad.c -ldl -lm -lglfw -pipe -Wall -Wextra

#define STB_IMAGE_IMPLEMENTATION
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdbool.h>
#include "stb_image.h" // image-loading library
#include "glext.h" // extensions

void framebuffer_size_callback(GLFWwindow* window, GLint width, GLint height);
void processInput(GLFWwindow *window);

const GLuint SCR_WIDTH = 800;
const GLuint SCR_HEIGHT = 600;

const  GLchar *vertexShaderSource = "#version 460 core\n"
    "#extension GL_NV_gpu_shader5 : enable\n"
    "layout (location = 0) in vec3 aPos;\n" // the position variable has attribute position 0
    "layout (location = 1) in vec3 aColor;\n"  // the color variable has attribute position 1
    "layout (location = 2) in vec2 aTexCoord;\n" // the texture coord variable has attribute position 2
    "out vec3 ourColor;\n" // specify a color output to the fragment shader
    "out vec2 TexCoord;\n" // specify a texture coord output to the fragment shader
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos, 1.0);\n"
    "   ourColor = aColor;\n"
    "   TexCoord = vec2(aTexCoord.x, aTexCoord.y);\n"
    "}\0";

const  GLchar *fragmentShaderSource = "#version 460 core\n"
    "#extension GL_ARB_draw_buffers : enable\n"
    "#extension GL_NV_fragment_shader_barycentric : enable\n"
    "out vec4 FragColor;\n"
    "in vec3 ourColor;\n" // the aColor variable from the vertex shader
    "in vec2 TexCoord;\n" // the aTexCoord variable from the vertex shader
    "uniform sampler2D texture1;\n" // in order to pass the texture object we need built-in data-type "sample" so we can later assign our texture
    "void main()\n"
    "{\n"
    "   FragColor = texture(texture1, TexCoord);\n"
    "}\n\0";

int main(){
    // glfw: we first initialize GLFW with glfwInit, after which we can configure GLFW using glfwWindowHint
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // Specify API version 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // Specify API version 3.3
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // creating contex profile


    // glfw window creation
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Learning OpenGL", NULL, NULL); // "window" object holds all the windowing data
    if (window == NULL){
        printf("Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window); // making the context of our window the main context on the current thread
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); // sets the framebuffer resize callback for the specified window.

    // glad: load all OpenGL (OS specific) function pointers 
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){ 
        printf("Failed to create initialize GLAD\n");
        return -1;
    }    
    
    // vertex shader - processes as much vertices from its memory
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER_ARB); // create vertex shader
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL); // replaces the source code of vertex shader
    glCompileShader(vertexShader); // compileing vertex shader

    // fragment shader - calculates the color output of your pixels
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER_ARB); // create fragment shader
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL); // replaces the source code of fragment shader
    glCompileShader(fragmentShader); // compileing fragment shader

    // checking for compile-time errors 
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success){
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n");
    }
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success){
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n");
    }
    // creating shaderProgram - shaderProgram object should be the final linked version of multiple shaders combined
    GLuint shaderProgram = glCreateProgram(); // creates a program and returns the ID reference
    glAttachShader(shaderProgram, vertexShader); 
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram); // linking shaders
    
    // checking for errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n");
    }
    // delete the shader objects once we've linked them into the program object
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    

    // drawing rectangle using two triagles
    GLfloat vertices[] = {
        // pisitions         // colors            // texture coords
        0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right vertex
        0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right vertex
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left vertex
        -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left vertex
    };
    GLuint indices[] = {  
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    GLuint VBO; // vertex buffer object - send large batches of data all at once to the graphics card
    GLuint VAO; // contains one or more Vertex Buffer Objects (stores the information for a complete rendered object)
    GLuint EBO; // EBO is a buffer that decides what vertices to draw
    glGenVertexArrays(1, &VAO); // generates vertex array objects
    glGenBuffers(1, &VBO); // generates vertex buffer objects 
    glGenBuffers(1, &EBO); // generates element buffer objects

    // bind the Vertex Array Object first, then bind and set vertex buffers, and then configure vertex attributes
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER_ARB, VBO); // bind the newly created buffer to the GL_ARRAY_BUFFER_ARB target (extension)
    // copies the previously defined vertex data into the buffer's memory, GL_STATIC_DRAW means that the data is set only once and used many times
    glBufferData(GL_ARRAY_BUFFER_ARB, sizeof(vertices), vertices, GL_STATIC_DRAW); 
    // Similar to the VBO we bind the EBO and copy the indices into the buffer, this time we specify GL_ELEMENT_ARRAY_BUFFER_ARB as the buffer type.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER_ARB, sizeof(indices), indices, GL_STATIC_DRAW);

    // getting attrib locations
    GLint positionLocation = glGetAttribLocation(shaderProgram, "aPos");
    GLint colorLocation = glGetAttribLocation(shaderProgram, "aColor");
    GLint texCoordLocation = glGetAttribLocation(shaderProgram, "aTexCoord");
    
    // interpreting vertex data 
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0); 
    glVertexAttribPointer(colorLocation, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glVertexAttribPointer(texCoordLocation, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    // enabling generic vertex attributes
    glEnableVertexAttribArray(positionLocation);
    glEnableVertexAttribArray(colorLocation);
    glEnableVertexAttribArray(texCoordLocation);


    // load and create a texture 
    GLuint texture;
    glGenTextures(1, &texture); // setting referenced IDs that are later genereted into textures using glTexImage2D
    glBindTexture(GL_TEXTURE_BUFFER_ARB, texture); // sets the given texture object as the currently active texture object

    // set the texture wrapping and filtering params, GL_REPEAT is default behavior for textures
    // GL_LINEAR takes an interpolated value - approximating a color between the texels
    glTexParameteri(GL_TEXTURE_BUFFER_ARB, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_BUFFER_ARB, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_BUFFER_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_BUFFER_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // loading image
    GLint width, height, nrChannels;
    unsigned  char *data = stbi_load("image", &width, &height, &nrChannels, 0);
    if (data){
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data); // genereting texture
        glGenerateMipmap(GL_TEXTURE_2D); // generate mipmaps for a specified texture object
    } else printf("Failed to load texture\n");
    stbi_image_free(data); // free the image memory

    glUseProgram(shaderProgram); // sets the given program object as the current active shader

    while (!glfwWindowShouldClose(window)){ // render loop
        // inputs
        processInput(window); // check for specific key presses and react accordingly every frame

        // rendering
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // sets the color value that OpenGL uses to reset the color buffer (background color)
        glClear(GL_COLOR_BUFFER_BIT); // clears the entire buffer. That color buffer will be filled with the color as configured by glClearColor()

        // drawing
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); // renders the triangles from EBO

        // swaping buffers and poll I/O events
        glfwSwapBuffers(window); // will swap the color buffer
        glfwPollEvents(); // checks if any events are triggered
    }
    // deallocating stuff
    glDeleteVertexArrays(1, &VAO); 
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
    glfwTerminate(); // delete all of GLFW's resources that were allocated
    return 0;
}

void processInput(GLFWwindow *window){ // keeps all the input code
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) // closes window on ESC
        glfwSetWindowShouldClose(window, true);
}

// whenever the window size changed this callback function executes!
void framebuffer_size_callback(GLFWwindow* window, GLint width, GLint height){
    glViewport(0, 0, width, height); // size of the rendering window
}