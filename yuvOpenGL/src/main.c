
typedef enum
{
    OPENGL_OK = 0,
    OPENGL_INIT_ERROR = -1
} OpenGLFlags;


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "orqa_clock.h"
#include "orqa_opengl.h"
#include "orqa_window.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

// screen resolution
const GLuint SCR_WIDTH = 1920;
const GLuint SCR_HEIGHT =  1080;

const GLuint width = 1280;
const GLuint height = 720;

const GLfloat vertices[] = {
        // pisitions         // texture coords
         0.5f,  0.5f, 0.0f,  1.0f, 1.0f, // top right vertex
         0.5f, -0.5f, 0.0f,  1.0f, 0.0f, // bottom right vertex
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f, // bottom left vertex
        -0.5f,  0.5f, 0.0f,  0.0f, 1.0f  // top left vertex
};
const GLuint indices[] = {  
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
};


const GLfloat verticesRGB[] = {
        // pisitions         // texture coords
         1.00f,  1.00f, -0.1f,  1.0f, 1.0f, // top right vertex
         1.00f, -1.00f, -0.1f,  1.0f, 0.0f, // bottom right vertex
        -1.00f, -1.00f, -0.1f,  0.0f, 0.0f, // bottom left vertex
        -1.00f,  1.00f, -0.1f,  0.0f, 1.0f  // top left vertex
};

/// This function keeps track all the input code. Press ESC to exit
static void orqa_process_input(GLFWwindow *window)
{
    // keeps all the input code
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, 1);
    }
}


float myCLAMP(float a)
{
    return (float)MIN(MAX(a, 0), 255);
}

static uint8_t* yuv420p_to_rgb3(uint8_t *yuv_buffer, const int width, const int height)
{
    uint8_t *y = yuv_buffer;
    uint8_t *u = yuv_buffer + (width * height);
    uint8_t *v = yuv_buffer + (width * height) + (width * height / 4);
    uint8_t *rgb = calloc((width * height * 3), sizeof(uint8_t));
    float b, g, r;
    uint8_t *ptr = rgb;
    for (int j = 0; j < height; j++)
    {
        for (int i = 0; i < width; i++)
        {
            int yy = y[(j * width) + i];
            int uu = u[((j / 2) * (width / 2)) + (i / 2)];
            int vv = v[((j / 2) * (width / 2)) + (i / 2)];
            r = 1.164 * (yy - 16) + 1.596 * (vv - 128);
            g = 1.164 * (yy - 16) - 0.813 * (vv - 128) - 0.391 * (uu - 128);
            b = 1.164 * (yy - 16) + 2.018 * (uu - 128);
            *ptr++ = myCLAMP(r);
            *ptr++ = myCLAMP(g);
            *ptr++ = myCLAMP(b);
        }
    }
    return rgb;
}



int main()
{
    orqa_set_error_cb(orqa_error_cb);
    
    if (orqa_init_glfw(3, 3))
        return OPENGL_INIT_ERROR;
    // orqa_GLFW_make_window_full_screen();                                                                 // Full screen
    GLFWwindow *window = orqa_create_GLFW_window(SCR_WIDTH, SCR_HEIGHT, "Press ESC to exit!!!", NULL, NULL); // glfw window object creation
    if (window == NULL)
        return OPENGL_INIT_ERROR;
     
    orqa_make_window_current(window);

    if (!orqa_load_glad((GLADloadproc)orqa_get_proc_address))
    { // glad: load all OpenGL function pointers. GLFW gives us glfwGetProcAddress that defines the correct function based on which OS we're compiling for
        fprintf(stderr, "In file: %s, line: %d Failed to create initialize GLAD\n", __FILE__, __LINE__);
        glfwTerminate();
        return OPENGL_INIT_ERROR;
    }

    // shader init, compilation and linking
    GLuint shaders[3];

    shaders[0] = orqa_load_shader_from_file("./shaders/vertexShader.vert", GL_VERTEX_SHADER);
    shaders[1] = orqa_load_shader_from_file("./shaders/fragmentShader.frag", GL_FRAGMENT_SHADER);
    shaders[2] = orqa_load_shader_from_file("./shaders/yuv.frag", GL_FRAGMENT_SHADER);
    GLuint rgbShader = orqa_create_program(shaders, 0, 1);
     

    GLuint yuvShader = orqa_create_program(shaders, 0, 2);
     
    // get indexes for shader variables
    GLuint RGBposLoc = orqa_get_attrib_location(rgbShader, "aPos");
    GLuint RGBtexLoc = orqa_get_attrib_location(rgbShader, "aTexCoord");

    // get indexes for shader variables
    GLuint YUVposLoc = orqa_get_attrib_location(yuvShader, "aPos");
    GLuint YUVtexLoc = orqa_get_attrib_location(yuvShader, "aTexCoord");

    // init & binding array & buffer objects
    GLuint *VAOs = orqa_generate_VAOs(2);
    GLuint *VBOs = orqa_generate_VBOs(2);
    GLuint *EBOs = orqa_generate_EBOs(2);

    orqa_bind_VAOs(VAOs[0]);
    orqa_bind_buffer_set_data(GL_ARRAY_BUFFER, VBOs[0], sizeof(vertices), vertices, GL_STATIC_DRAW);
    orqa_bind_buffer_set_data(GL_ELEMENT_ARRAY_BUFFER, EBOs[0], sizeof(indices), indices, GL_STATIC_DRAW);
    orqa_enable_vertex_attrib_array(RGBposLoc, 3, GL_FLOAT, 5 * sizeof(float), (float *)0);
    orqa_enable_vertex_attrib_array(RGBtexLoc, 2, GL_FLOAT, 5 * sizeof(float), (void *)(3 * sizeof(float)));

    orqa_bind_VAOs(VAOs[1]);
    orqa_bind_buffer_set_data(GL_ARRAY_BUFFER, VBOs[1], sizeof(verticesRGB), verticesRGB, GL_STATIC_DRAW);
    orqa_bind_buffer_set_data(GL_ELEMENT_ARRAY_BUFFER, EBOs[1], sizeof(indices), indices, GL_STATIC_DRAW);
    orqa_enable_vertex_attrib_array(YUVposLoc, 3, GL_FLOAT, 5 * sizeof(float), (float *)0);
    orqa_enable_vertex_attrib_array(YUVtexLoc, 2, GL_FLOAT, 5 * sizeof(float), (void *)(3 * sizeof(float)));

     
    GLuint yLoc = orqa_get_uniform_location(yuvShader, "textureY");
    GLuint uLoc = orqa_get_uniform_location(yuvShader, "textureU");
    GLuint vLoc = orqa_get_uniform_location(yuvShader, "textureV");

    const unsigned int numOfFrames =  100;
    FILE *input_file = fopen("frames.yuv", "rb");
    const int yuv_size = width*height*3/2;
    uint8_t *inputBuffer = malloc(yuv_size *  numOfFrames);
    fread(inputBuffer, yuv_size *  numOfFrames, 1, input_file);
    
    uint8_t *rgb; // = malloc(width*height*3);
    

    uint8_t *yBuffer = malloc(width*height);
    uint8_t *uBuffer = malloc(width*height/4);
    uint8_t *vBuffer = malloc(width*height/4);

    // memcpy(yBuffer, yuv_buffer, width*height);
    // memcpy(uBuffer, yuv_buffer + width*height, width*height/4);
    // memcpy(vBuffer, yuv_buffer + width*height + width*height/4, width*height/4);
    
    // texture init
    GLuint *textures = orqa_create_textures(5);

    
    // orqa_bind_texture(textures[0]);
    // orqa_generate_texture_from_buffer(GL_TEXTURE_2D, GL_LUMINANCE, width, height, GL_LUMINANCE, GL_UNSIGNED_BYTE, yBuffer);
     
    // orqa_bind_texture(textures[1]);
    // orqa_generate_texture_from_buffer(GL_TEXTURE_2D, GL_LUMINANCE, width/2, height/2, GL_LUMINANCE, GL_UNSIGNED_BYTE, vBuffer);
     
    // orqa_bind_texture(textures[2]);
    // orqa_generate_texture_from_buffer(GL_TEXTURE_2D, GL_LUMINANCE, width/2, height/2, GL_LUMINANCE, GL_UNSIGNED_BYTE, uBuffer);
    
    orqa_bind_texture(textures[3]);
    orqa_generate_texture_from_buffer(GL_TEXTURE_2D, GL_RGB, width, height, GL_RGB, GL_UNSIGNED_BYTE, NULL);
     

    glUniform1i(yLoc, textures[0]);
    glUniform1i(uLoc, textures[1]);
    glUniform1i(vLoc, textures[2]);
     

    int i = 0;
    while (!glfwWindowShouldClose(window))
    { // render loop
        // input
        orqa_process_input(window);

        uint8_t *yuv_buffer = malloc(yuv_size);
        memcpy(yuv_buffer, inputBuffer + (i++ * yuv_size), yuv_size);
        rgb = yuv420p_to_rgb3(yuv_buffer, width, height);


        // render
        orqa_clear_color_buffer(0.2f, 0.2f, 0.2f, 1.0f);
        orqa_clear_buffer(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        orqa_use_program(rgbShader);
        orqa_bind_texture(textures[3]);
        orqa_generate_texture_from_buffer(GL_TEXTURE_2D, GL_RGB, width, height, GL_RGB, GL_UNSIGNED_BYTE, rgb);
        orqa_bind_vertex_object_and_draw_it(VAOs[1], GL_TRIANGLES, 6);
        
        // orqa_sleep(ORQA_SLEEP_SEC, 2);
        /*
        orqa_use_program(yuvShader);
        orqa_bind_texture(textures[0]);
        orqa_generate_texture_from_buffer(GL_TEXTURE_2D, GL_LUMINANCE, width, height, GL_LUMINANCE, GL_UNSIGNED_BYTE, yBuffer);
        orqa_bind_texture(textures[1]);
        orqa_generate_texture_from_buffer(GL_TEXTURE_2D, GL_LUMINANCE, width/2, height/2, GL_LUMINANCE, GL_UNSIGNED_BYTE, vBuffer);
        orqa_bind_texture(textures[2]);
        orqa_generate_texture_from_buffer(GL_TEXTURE_2D, GL_LUMINANCE, width/2, height/2, GL_LUMINANCE, GL_UNSIGNED_BYTE, uBuffer);
        orqa_bind_vertex_object_and_draw_it(VAOs[0], GL_TRIANGLES, 6);
        */

        // glfw: swap buffers and poll IO events
        orqa_swap_buffers(window);
        orqa_pool_events();
        if (i ==  numOfFrames) break;
    }
    free(rgb);
    free(yBuffer);
    free(uBuffer);
    free(vBuffer);
    // free(yuv_buffer);
    fclose(input_file);

    // deallocating stuff
    orqa_delete_VAOs(2, VAOs);
    orqa_delete_buffers(2, VBOs);
    orqa_delete_buffers(2, EBOs);
    orqa_delete_textures(5, textures);
    orqa_delete_program(rgbShader);
    orqa_delete_program(yuvShader);
     
    glfwTerminate(); // glfw: terminate, clearing all previously allocated GLFW resources.
    return OPENGL_OK;
}

