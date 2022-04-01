#define STB_IMAGE_IMPLEMENTATION
#define _IN
#define _REF
#define _OUT
#define _NOARGS

#include <stdio.h> // fprintf
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb_image.h" // image-loading library, used funcktions: stbi_load, stbi_image_free
#include "glext.h" // extensions for gpu optimization
#include "orqa_clock.h"

const GLuint SCR_WIDTH = 800;
const GLuint SCR_HEIGHT = 600;

const GLfloat vertices[] = {
        // pisitions         // texture coords
         0.9f,  0.9f, 0.0f,  1.0f, 1.0f, // top right vertex
         0.9f, -0.9f, 0.0f,  1.0f, 0.0f, // bottom right vertex
        -0.9f, -0.9f, 0.0f,  0.0f, 0.0f, // bottom left vertex
        -0.9f,  0.9f, 0.0f,  0.0f, 1.0f  // top left vertex
};
const GLuint indices[] = {  
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
};

const  GLchar *vertexShaderSource = "#version 460 core\n"
    "#extension GL_NV_gpu_shader5 : enable\n"
    "layout (location = 0) in vec3 aPos;\n" // the position variable has attribute position 0
    "layout (location = 1) in vec2 aTexCoord;\n" // the texture coord variable has attribute position 2
    "out vec2 TexCoord;\n" // specify a texture coord output to the fragment shader
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos, 1.0);\n"
    "   TexCoord = vec2(aTexCoord.x, 1 - aTexCoord.y);\n"
    "}\0";

const  GLchar *fragmentShaderSource = "#version 460 core\n"
    "#extension GL_ARB_draw_buffers : enable\n"
    "#extension GL_NV_fragment_shader_interlock : enable\n"
    "out vec4 FragColor;\n"
    "in vec2 TexCoord;\n" // the aTexCoord variable from the vertex shader
    "uniform sampler2D texture1;\n" // in order to pass the texture object we need built-in data-type "sample" so we can later assign our texture
    "void main()\n"
    "{\n"
    "   FragColor = texture(texture1, TexCoord);\n"
    "}\n\0";

int _initGLFW(_NOARGS void){ // glfw: we first initialize GLFW with glfwInit, after which we can configure GLFW using glfwWindowHint
    if(!glfwInit()){
        fprintf(stderr, "In file: %s, line: %d Failed to initialize GLFW\n", __FILE__, __LINE__);
        glfwTerminate();
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // Specify API version 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6); // Specify API version 3.3
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // creating contex profile
    
    return 0;
}

void _processInput(_REF GLFWwindow *window){ // keeps all the input code
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) // closes window on ESC
        glfwSetWindowShouldClose(window, GL_TRUE);
}

// whenever the window size changed this callback function executes!
void _framebuffer_size_callback(_REF GLFWwindow* window,_IN GLint width,_IN GLint height){
    glViewport(0, 0, width, height); // size of the rendering window
}

int main(){
    if (_initGLFW() == -1) return 0;
    
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Learning OpenGL", NULL, NULL); // "window" object holds all the windowing data
    if (window == NULL){
        fprintf(stderr, "In file: %s, line: %d Failed to create GLFW window\n", __FILE__, __LINE__);
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window); // making the context of our window the main context on the current thread
    glfwSetFramebufferSizeCallback(window, _framebuffer_size_callback); // sets the framebuffer resize callback for the specified window.

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){ // glad: load all OpenGL (OS specific) function pointers 
        fprintf(stderr, "In file: %s, line: %d Failed to create initialize GLAD\n", __FILE__, __LINE__);
        glfwTerminate();
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
        fprintf(stderr, "In file: %s, line: %d ERROR::SHADER::VERTEX::COMPILATION_FAILED\nError:\n%s\n", __FILE__, __LINE__, infoLog);
        goto shaderError; // error handling
    }
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success){
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        fprintf(stderr, "In file: %s, line: %d ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\nError:\n%s\n", __FILE__, __LINE__, infoLog);
        goto shaderError; // error handling
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
        fprintf(stderr, "In file: %s, line: %d ERROR::SHADER::PROGRAM::LINKING_FAILED\nError:\n%s\n", __FILE__, __LINE__, infoLog);
        goto linkingError; // error handling
    }

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
    glBufferData(GL_ARRAY_BUFFER_ARB, sizeof(vertices), vertices, GL_STATIC_DRAW_ARB); 
    // Similar to the VBO we bind the EBO and copy the indices into the buffer, this time we specify GL_ELEMENT_ARRAY_BUFFER_ARB as the buffer type.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER_ARB, sizeof(indices), indices, GL_STATIC_DRAW_ARB);

    // getting attrib locations
    GLint positionLocation = glGetAttribLocation(shaderProgram, "aPos");
    GLint texCoordLocation = glGetAttribLocation(shaderProgram, "aTexCoord");
    
    // interpreting vertex data 
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0); 
    glVertexAttribPointer(texCoordLocation, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    
    // enabling generic vertex attributes
    glEnableVertexAttribArray(positionLocation);
    glEnableVertexAttribArray(texCoordLocation);

    // load and create a texture 
    GLuint texture;
    glGenTextures(1, &texture); 
    glBindTexture(GL_TEXTURE_BUFFER_ARB, texture); // sets the given texture object as the currently active texture object

    

    // loading image
    GLint width, height, nrChannels;
    unsigned  char *data = stbi_load("./data/pic.bmp", &width, &height, &nrChannels, 0);
    
    GLuint pbo;
    glGenBuffers(1, &pbo); 
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, width*height*nrChannels, NULL, GL_STREAM_DRAW);

    void *mappedBuffer = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
    memcpy(mappedBuffer, data, width*height*nrChannels);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, (void *)0);
    glGenerateMipmap(GL_TEXTURE_2D); 
    

    stbi_image_free(data); // free the image memory
    glUseProgram(shaderProgram); // sets the given program object as the current active shader

    while (!glfwWindowShouldClose(window)){ // render loop
        orqa_clock_t clock = orqa_time_now();
        // inputs
        _processInput(window); // check for specific key presses and react accordingly every frame

        // rendering
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // sets the color value that OpenGL uses to reset the color buffer (background color)
        glClear(GL_COLOR_BUFFER_BIT); // clears the entire buffer. That color buffer will be filled with the color as configured by glClearColor()

        // drawing
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); // renders the triangles from EBO

        // swaping buffers and poll I/O events
        glfwSwapBuffers(window); // will swap the color buffer
        glfwPollEvents(); // checks if any events are triggered
        printf("\r render time = %f", 1000/orqa_get_time_diff_msec(clock, orqa_time_now()));
    }
    printf("\n");
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
    glfwTerminate(); // delete all of GLFW's resources that were allocated
    return 0;
}