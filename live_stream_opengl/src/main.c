

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "my_opengl.h"
#include "my_window.h"
#include "video_decoder.h"
#include "video_encoder.h"
#include "orqa_clock.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

// screen resolution
const GLuint SCR_WIDTH = 1920;
const GLuint SCR_HEIGHT =  1080;

const GLuint width = 1280;
const GLuint height = 720;

const GLuint indices[] = {  
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
};

const GLfloat verticesYUV[] = {
        // pisitions         // texture coords
         0.00f,  1.00f, -0.1f,  1.0f, 1.0f, // top right vertex
         0.00f, -1.00f, -0.1f,  1.0f, 0.0f, // bottom right vertex
        -1.00f, -1.00f, -0.1f,  0.0f, 0.0f, // bottom left vertex
        -1.00f,  1.00f, -0.1f,  0.0f, 1.0f  // top left vertex
};

static uint8_t* yuv420p_to_rgb3(uint8_t *yuv_buffer, const int width, const int height);
float myCLAMP(float a);
static void my_process_input(GLFWwindow *window);

int main()
{
    my_set_error_cb(my_error_cb);
    
    if (my_init_glfw(3, 3))
        return -1;
    // my_GLFW_make_window_full_screen();                                                                  // Full screen
    GLFWwindow *window = my_create_GLFW_window(SCR_WIDTH, SCR_HEIGHT, "Press ESC to exit!!!", NULL, NULL); // glfw window object creation
    if (window == NULL)
        return -1;
     
    my_make_window_current(window);

    if (!my_load_glad((GLADloadproc)my_get_proc_address))
    { // glad: load all OpenGL function pointers. GLFW gives us glfwGetProcAddress that defines the correct function based on which OS we're compiling for
        fprintf(stderr, "In file: %s, line: %d Failed to create initialize GLAD\n", __FILE__, __LINE__);
        glfwTerminate();
        return -1;
    }

    // shader init, compilation and linking
    GLuint shaders[4];
    shaders[0] = my_load_shader_from_file("./shaders/vertexShader.vert", GL_VERTEX_SHADER);
    shaders[1] = my_load_shader_from_file("./shaders/fragmentShader.frag", GL_FRAGMENT_SHADER);
    shaders[2] = my_load_shader_from_file("./shaders/yuvVertexShader.vert", GL_VERTEX_SHADER);
    shaders[3] = my_load_shader_from_file("./shaders/yuv.frag", GL_FRAGMENT_SHADER);

    GLuint rgbShader = my_create_program(shaders, 0, 1);
    GLuint yuvShader = my_create_program(shaders, 2, 3);

    // get indexes for shader variables
    GLuint YUVposLoc = my_get_attrib_location(yuvShader, "aPos");
    GLuint YUVtexLoc = my_get_attrib_location(yuvShader, "aTexCoord");

    GLuint RGBposLoc = my_get_attrib_location(rgbShader, "aPos");
    GLuint RGBtexLoc = my_get_attrib_location(rgbShader, "aTexCoord");

    // init & binding array & buffer objects
    GLuint *VAOs = my_generate_VAOs(2);
    GLuint *VBOs = my_generate_VBOs(2);
    GLuint *EBOs = my_generate_EBOs(2);

    my_bind_VAOs(VAOs[1]);
    my_bind_buffer_set_data(GL_ARRAY_BUFFER, VBOs[1], sizeof(verticesYUV), verticesYUV, GL_STATIC_DRAW);
    my_bind_buffer_set_data(GL_ELEMENT_ARRAY_BUFFER, EBOs[1], sizeof(indices), indices, GL_STATIC_DRAW);
    my_enable_vertex_attrib_array(YUVposLoc, 3, GL_FLOAT, 5 * sizeof(float), (float *)0);
    my_enable_vertex_attrib_array(YUVtexLoc, 2, GL_FLOAT, 5 * sizeof(float), (void *)(3 * sizeof(float)));

    
    GLuint yLoc = glGetUniformLocation(yuvShader, "textureY");
    GLuint uLoc = glGetUniformLocation(yuvShader, "textureU");
    GLuint vLoc = glGetUniformLocation(yuvShader, "textureV");
    
    FILE *input_file = fopen("data/out720.yuv", "rb");
    if(input_file == NULL){
        return 1;
    }
    const int yuv_size = width*height*3/2;
    uint8_t *inputBuffer = malloc(yuv_size);
    fread(inputBuffer, yuv_size, 1, input_file);
    
    
    // texture init
    // GLuint textureY, textureU, textureV, rgbTex;
    GLuint *textures = my_create_textures(5); 
    
    glBindTexture(GL_TEXTURE_2D, textures[1]); 
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, textures[2]); 
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width/2, height/2, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, textures[3]); 
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width/2, height/2, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
    glGenerateMipmap(GL_TEXTURE_2D);

    my_use_program(yuvShader);
    glUniform1i(yLoc, 0);
    glUniform1i(uLoc, 1);
    glUniform1i(vLoc, 2);

    
    decoder_t dec;
    encoder_t enc;

    decoder_init(&dec, width, height);
    encoder_init(&enc, width, height);

    orqa_clock_t clock = orqa_time_now();
    int frame;

    while (!glfwWindowShouldClose(window))
    { // render loop
        // input
        my_process_input(window);
        encode(&enc, inputBuffer);
        if (enc.encoder_flag){
            decode(&dec, enc.encoded_buffer, enc.size);
            enc.encoder_flag = 0;
        }

        // render
        my_clear_color_buffer(0.2f, 0.2f, 0.2f, 1.0f);
        my_clear_buffer(GL_COLOR_BUFFER_BIT);

        my_use_program(yuvShader);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures[1]);
        if(dec.decoder_flag) {
            my_update_texture_from_buffer(GL_TEXTURE_2D,0,0, width, height, GL_RED, GL_UNSIGNED_BYTE, dec.decoded_buffer);
        }
        
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textures[2]);
        if(dec.decoder_flag){
            my_update_texture_from_buffer(GL_TEXTURE_2D,0,0, width/2, height/2, GL_RED, GL_UNSIGNED_BYTE, dec.decoded_buffer + width*height);
        }
        
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, textures[3]);
        if (dec.decoder_flag) {
            my_update_texture_from_buffer(GL_TEXTURE_2D,0,0, width/2, height/2, GL_RED, GL_UNSIGNED_BYTE, dec.decoded_buffer + width*height + width*height/4);
            frame ++;
            dec.decoder_flag = 0;
        }
        
        my_bind_vertex_object_and_draw_it(VAOs[1], GL_TRIANGLES, 6);

        // glfw: swap buffers and poll IO events
        my_swap_buffers(window);
        my_pool_events();
        orqa_clock_t elapsed = orqa_time_now();
        if (orqa_get_time_diff_msec(clock, elapsed) >= 1000)
        {
            printf("fps %d\n", frame);
            frame = 0;
            clock = elapsed;
        }
    }
    free(inputBuffer);
    fclose(input_file);

    // deallocating stuff
    my_delete_VAOs(2, VAOs);
    my_delete_buffers(2, VBOs);
    my_delete_buffers(2, EBOs);
    my_delete_program(yuvShader);
     
    glfwTerminate(); // glfw: terminate, clearing all previously allocated GLFW resources.
    return 0;
}

/// This function keeps track all the input code. Press ESC to exit
static void my_process_input(GLFWwindow *window)
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

