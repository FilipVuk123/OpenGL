#define STB_IMAGE_IMPLEMENTATION

#define ORQA_IN
#define ORQA_REF
#define ORQA_OUT
#define ORQA_NOARGS

typedef enum
{
    OPENGL_OK = 0,
    OPENGL_INIT_ERROR = -1
} OpenGLFlags;

#define BUFSIZE 1024
#define HEADTRACKING_BUFFER_SIZE 64
#define PORT 8000

#include <stdio.h>
#include <pthread.h>
#include <orqa_vegvisir_common.h>

// screen resolution
const GLuint SCR_WIDTH = 1920;
const GLuint SCR_HEIGHT = 1080;


int main()
{
    orqa_set_error_cb(orqa_error_cb);

    if (orqa_init_glfw(3, 3))
        return OPENGL_INIT_ERROR;
    orqa_GLFW_make_window_full_screen();                                                                 // Full screen
    GLFWwindow *window = orqa_create_GLFW_window(SCR_WIDTH, SCR_HEIGHT, "Vegvisir Project", NULL, NULL); // glfw window object creation
    orqa_set_input_mode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);   // use cursor but do not display it
    if (window == NULL)
        return OPENGL_INIT_ERROR;
    orqa_make_window_current(window);

    orqa_set_frame_buffer_cb(window, orqa_framebuffer_size_callback); // manipulate view port
    // orqa_set_cursor_position_cb(window, orqa_mouse_callback);         // move camera_t with cursor
    orqa_set_scroll_cb(window, orqa_scroll_callback);                 // zoom in/out using mouse wheel

    if (!orqa_load_glad((GLADloadproc)orqa_get_proc_address))
    { // glad: load all OpenGL function pointers. GLFW gives us glfwGetProcAddress that defines the correct function based on which OS we're compiling for
        fprintf(stderr, "In file: %s, line: %d Failed to create initialize GLAD\n", __FILE__, __LINE__);
        glfwTerminate();
        return OPENGL_INIT_ERROR;
    }

    // mash generation
    orqa_window_t lr = orqa_create_window(1.0, 40, 20, -0.7, -0.5, 0.55);
    orqa_window_t rr = orqa_create_window(1.0, 40, 20, 0.7, -0.5, 0.55);
    orqa_window_t DSS1 = orqa_create_window(1.0, 50, 25, -0.7, 0, 0.64);
    orqa_window_t DSS2 = orqa_create_window(1.0, 50, 25, 0, 0, 1);
    orqa_window_t DSS3 = orqa_create_window(1.0, 50, 25, 0.7, 0, 0.64);
    orqa_window_t mr = orqa_create_window(1.0, 35, 20, 0, -0.32, 0.5);
    orqa_window_t BW = orqa_create_window(1.0, 35, 20, 0, 0.6, 0.65);
    orqa_window_t MRSS = orqa_create_window(1.0, 130, 60, 0, 0, 1);
    orqa_sphere_t sph = orqa_create_sphere(1.0, 150, 150);

    // shader init, compilation and linking
    GLuint shaders[2];

    shaders[0] = orqa_load_shader_from_file("./shaders/vertexShader.vert", GL_VERTEX_SHADER);
    shaders[1] = orqa_load_shader_from_file("./shaders/fragmentShader.frag", GL_FRAGMENT_SHADER);
    GLuint shaderProgram = orqa_create_program(shaders, 2);
    orqa_use_program(shaderProgram);

    // get indexes for shader variables
    GLuint posLoc = orqa_get_attrib_location(shaderProgram, "aPos");
    GLuint texLoc = orqa_get_attrib_location(shaderProgram, "aTexCoord");

    // init & binding array & buffer objects
    GLuint *VAOs = orqa_generate_VAOs(10);
    GLuint *VBOs = orqa_generate_VBOs(10);
    GLuint *EBOs = orqa_generate_EBOs(10);

    orqa_bind_VAOs(VAOs[0]);
    orqa_bind_buffer_set_data(GL_ARRAY_BUFFER, VBOs[0], rr.numVertices * sizeof(float), rr.Vs, GL_STATIC_DRAW);
    orqa_bind_buffer_set_data(GL_ELEMENT_ARRAY_BUFFER, EBOs[0], rr.numTriangles * sizeof(int), rr.Is, GL_STATIC_DRAW);
    orqa_enable_vertex_attrib_array(posLoc, 3, GL_FLOAT, 5 * sizeof(float), (float *)0);
    orqa_enable_vertex_attrib_array(texLoc, 2, GL_FLOAT, 5 * sizeof(float), (void *)(3 * sizeof(float)));

    orqa_bind_VAOs(VAOs[1]);
    orqa_bind_buffer_set_data(GL_ARRAY_BUFFER, VBOs[1], lr.numVertices * sizeof(float), lr.Vs, GL_STATIC_DRAW);
    orqa_bind_buffer_set_data(GL_ELEMENT_ARRAY_BUFFER, EBOs[1], lr.numTriangles * sizeof(int), lr.Is, GL_STATIC_DRAW);
    orqa_enable_vertex_attrib_array(posLoc, 3, GL_FLOAT, 5 * sizeof(float), (float *)0);
    orqa_enable_vertex_attrib_array(texLoc, 2, GL_FLOAT, 5 * sizeof(float), (void *)(3 * sizeof(float)));

    orqa_bind_VAOs(VAOs[2]);
    orqa_bind_buffer_set_data(GL_ARRAY_BUFFER, VBOs[2], DSS1.numVertices * sizeof(float), DSS1.Vs, GL_STATIC_DRAW);
    orqa_bind_buffer_set_data(GL_ELEMENT_ARRAY_BUFFER, EBOs[2], DSS1.numTriangles * sizeof(int), DSS1.Is, GL_STATIC_DRAW);
    orqa_enable_vertex_attrib_array(posLoc, 3, GL_FLOAT, 5 * sizeof(float), (float *)0);
    orqa_enable_vertex_attrib_array(texLoc, 2, GL_FLOAT, 5 * sizeof(float), (void *)(3 * sizeof(float)));

    orqa_bind_VAOs(VAOs[3]);
    orqa_bind_buffer_set_data(GL_ARRAY_BUFFER, VBOs[3], DSS2.numVertices * sizeof(float), DSS2.Vs, GL_STATIC_DRAW);
    orqa_bind_buffer_set_data(GL_ELEMENT_ARRAY_BUFFER, EBOs[3], DSS2.numTriangles * sizeof(int), DSS2.Is, GL_STATIC_DRAW);
    orqa_enable_vertex_attrib_array(posLoc, 3, GL_FLOAT, 5 * sizeof(float), (float *)0);
    orqa_enable_vertex_attrib_array(texLoc, 2, GL_FLOAT, 5 * sizeof(float), (void *)(3 * sizeof(float)));

    orqa_bind_VAOs(VAOs[4]);
    orqa_bind_buffer_set_data(GL_ARRAY_BUFFER, VBOs[4], DSS3.numVertices * sizeof(float), DSS3.Vs, GL_STATIC_DRAW);
    orqa_bind_buffer_set_data(GL_ELEMENT_ARRAY_BUFFER, EBOs[4], DSS3.numTriangles * sizeof(int), DSS3.Is, GL_STATIC_DRAW);
    orqa_enable_vertex_attrib_array(posLoc, 3, GL_FLOAT, 5 * sizeof(float), (float *)0);
    orqa_enable_vertex_attrib_array(texLoc, 2, GL_FLOAT, 5 * sizeof(float), (void *)(3 * sizeof(float)));

    orqa_bind_VAOs(VAOs[5]);
    orqa_bind_buffer_set_data(GL_ARRAY_BUFFER, VBOs[5], BW.numVertices * sizeof(float), BW.Vs, GL_STATIC_DRAW);
    orqa_bind_buffer_set_data(GL_ELEMENT_ARRAY_BUFFER, EBOs[5], BW.numTriangles * sizeof(int), BW.Is, GL_STATIC_DRAW);
    orqa_enable_vertex_attrib_array(posLoc, 3, GL_FLOAT, 5 * sizeof(float), (float *)0);
    orqa_enable_vertex_attrib_array(texLoc, 2, GL_FLOAT, 5 * sizeof(float), (void *)(3 * sizeof(float)));

    orqa_bind_VAOs(VAOs[6]);
    orqa_bind_buffer_set_data(GL_ARRAY_BUFFER, VBOs[6], mr.numVertices * sizeof(float), mr.Vs, GL_STATIC_DRAW);
    orqa_bind_buffer_set_data(GL_ELEMENT_ARRAY_BUFFER, EBOs[6], mr.numTriangles * sizeof(int), mr.Is, GL_STATIC_DRAW);
    orqa_enable_vertex_attrib_array(posLoc, 3, GL_FLOAT, 5 * sizeof(float), (float *)0);
    orqa_enable_vertex_attrib_array(texLoc, 2, GL_FLOAT, 5 * sizeof(float), (void *)(3 * sizeof(float)));

    orqa_bind_VAOs(VAOs[7]);
    orqa_bind_buffer_set_data(GL_ARRAY_BUFFER, VBOs[7], MRSS.numVertices * sizeof(float), MRSS.Vs, GL_STATIC_DRAW);
    orqa_bind_buffer_set_data(GL_ELEMENT_ARRAY_BUFFER, EBOs[7], MRSS.numTriangles * sizeof(int), MRSS.Is, GL_STATIC_DRAW);
    orqa_enable_vertex_attrib_array(posLoc, 3, GL_FLOAT, 5 * sizeof(float), (float *)0);
    orqa_enable_vertex_attrib_array(texLoc, 2, GL_FLOAT, 5 * sizeof(float), (void *)(3 * sizeof(float)));

    orqa_bind_VAOs(VAOs[8]);
    orqa_bind_buffer_set_data(GL_ARRAY_BUFFER, VBOs[8], sph.numVertices * sizeof(float), sph.Vs, GL_STATIC_DRAW);
    orqa_bind_buffer_set_data(GL_ELEMENT_ARRAY_BUFFER, EBOs[8], sph.numTriangles * sizeof(int), sph.Is, GL_STATIC_DRAW);
    orqa_enable_vertex_attrib_array(posLoc, 3, GL_FLOAT, 5 * sizeof(float), (float *)0);
    orqa_enable_vertex_attrib_array(texLoc, 2, GL_FLOAT, 5 * sizeof(float), (void *)(3 * sizeof(float)));

    // texture init
    GLuint *textures = orqa_create_textures(5);
    // loading image!

    orqa_bind_texture(textures[0]);
    orqa_load_texture_from_file("./data/MRSS.png");
    orqa_bind_texture(textures[1]);
    orqa_load_texture_from_file("./data/DSS.png");
    orqa_bind_texture(textures[2]);
    orqa_load_texture_from_file("./data/pic.bmp");
    orqa_bind_texture(textures[3]);
    orqa_load_texture_from_file("./data/panorama1.bmp");

    // camera init
    orqa_camera_t cam;
    cam.cameraPos[0] = 0.0f;
    cam.cameraPos[1] = 0.0f;
    cam.cameraPos[2] = 0.0f;
    cam.resultQuat[0] = 0.0f;
    cam.resultQuat[1] = 0.0f;
    cam.resultQuat[2] = 0.0f;
    cam.resultQuat[3] = 1.0f;
    cam.fov = 5.4f;
    orqa_set_window_user_pointer(window, &cam); // sent camera object to callback functions

    // UDP thread & mutex init
    pthread_t readFromSerial, readFromUDP;
    pthread_create(&readFromUDP, NULL, orqa_udp_thread, &cam);
    // pthread_create(&readFromSerial, NULL, orqa_read_from_serial, &cam);


    // MVP matrices init
    mat4 model, proj, view;
    glm_mat4_identity(model);
    glm_mat4_identity(view);
    glm_mat4_identity(proj);

    // get MVP shader indexes
    GLuint modelLoc = orqa_get_uniform_location(shaderProgram, "model");
    GLuint viewLoc = orqa_get_uniform_location(shaderProgram, "view");
    GLuint projLoc = orqa_get_uniform_location(shaderProgram, "proj");

    EXIT = 0;

    while (!glfwWindowShouldClose(window))
    { // render loop
        // input
        orqa_clock_t clock = orqa_time_now();
        orqa_process_input(window);

        // render
        orqa_clear_color_buffer(0.2f, 0.2f, 0.2f, 1.0f);
        orqa_clear_buffer(GL_COLOR_BUFFER_BIT); // | GL_DEPTH_BUFFER_BIT);

        // generate projection matrix
        glm_perspective(cam.fov, (GLfloat)SCR_WIDTH / (GLfloat)SCR_HEIGHT, 0.01f, 100.0f, proj); // zoom

        // generate view matrix
        glm_quat_look(cam.cameraPos, cam.resultQuat, view);
        // printf("%f, %f, %f, %f\n", cam.resultQuat[0], cam.resultQuat[1], cam.resultQuat[2], cam.resultQuat[3]);

        // send MVP matrices to vertex shader
        orqa_send_shander_4x4_matrix(modelLoc, 1, &model[0][0]);
        orqa_send_shander_4x4_matrix(viewLoc, 1, &view[0][0]);
        orqa_send_shander_4x4_matrix(projLoc, 1, &proj[0][0]);

        // build texture && draw
        if (mode == 0)
        {
            // 360 dome
            orqa_bind_texture(textures[3]);
            orqa_bind_vertex_object_and_draw_it(VAOs[8], GL_TRIANGLES, sph.numTriangles);
        }
        else if (mode == 1)
        {
            // DSS

            orqa_bind_texture(textures[0]);
            orqa_bind_vertex_object_and_draw_it(VAOs[0], GL_TRIANGLES, rr.numTriangles);
            orqa_bind_vertex_object_and_draw_it(VAOs[1], GL_TRIANGLES, lr.numTriangles);

            orqa_bind_texture(textures[2]);
            orqa_bind_vertex_object_and_draw_it(VAOs[5], GL_TRIANGLES, BW.numTriangles);
            orqa_bind_vertex_object_and_draw_it(VAOs[6], GL_TRIANGLES, mr.numTriangles);

            orqa_bind_texture(textures[1]);
            orqa_bind_vertex_object_and_draw_it(VAOs[2], GL_TRIANGLES, DSS1.numTriangles);
            orqa_bind_vertex_object_and_draw_it(VAOs[3], GL_TRIANGLES, DSS2.numTriangles);
            orqa_bind_vertex_object_and_draw_it(VAOs[4], GL_TRIANGLES, DSS3.numTriangles);
        }
        else if (mode == 2)
        {
            // MRSS

            orqa_bind_texture(textures[0]);
            orqa_bind_vertex_object_and_draw_it(VAOs[7], GL_TRIANGLES, MRSS.numTriangles);
        }

        // printf("\r Render FPS: %f", 1000/orqa_get_time_diff_msec(clock, orqa_time_now()));

        // glfw: swap buffers and poll IO events
        orqa_swap_buffers(window);
        orqa_pool_events();
    }

    // deallocating stuff
    orqa_window_free(&lr);
    orqa_window_free(&rr);
    orqa_window_free(&DSS1);
    orqa_window_free(&DSS2);
    orqa_window_free(&DSS3);
    orqa_window_free(&mr);
    orqa_window_free(&BW);
    orqa_window_free(&MRSS);
    orqa_sphere_free(&sph);
    orqa_delete_VAOs(10, VAOs);
    orqa_delete_buffers(10, VBOs);
    orqa_delete_buffers(10, EBOs);
    orqa_delete_textures(5, textures);
    orqa_delete_program(shaderProgram);
threadError:
    glfwTerminate(); // glfw: terminate, clearing all previously allocated GLFW resources.
    fprintf(stdout, "\nExit OK!\n");
    return OPENGL_OK;
}
