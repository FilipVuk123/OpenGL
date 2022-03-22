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
#include <sys/socket.h>
#include <arpa/inet.h> // inet_addr
#include <unistd.h>
#include <netinet/in.h>

#include <fcntl.h>   // Contains file controls like O_RDWR
#include <errno.h>   // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h>  // write(), read(), close()

#include <vendor/cglm/cglm.h>
#include "vendor/json.h"
#include "orqa_gen_mash.h"
#include "orqa_clock.h"
#include "orqa_opengl.h"
#include "orqa_input.h"
#include "orqa_window.h"

// screen resolution
const GLuint SCR_WIDTH = 1920;
const GLuint SCR_HEIGHT = 1080;

char serialBuffer[HEADTRACKING_BUFFER_SIZE] = "\0";
int FLAG = 0;
int EXIT = 0;

// fix it!!!
int mode; // 0 => 360, 1 => MRSS, 2 => DSS

pthread_mutex_t mutexLock;
static float orqa_radians(const float deg);
static void *orqa_udp_thread(ORQA_REF void *c_ptr);
static void orqa_process_input(ORQA_REF GLFWwindow *window);
static void *orqa_move_camera(ORQA_REF void *c_ptr);
static void *orqa_read_from_serial();
int main()
{
    orqa_set_error_cb(orqa_error_cb);

    if (orqa_init_glfw(3, 3))
        return OPENGL_INIT_ERROR;
    orqa_GLFW_make_window_full_screen();                                                                 // Full screen
    GLFWwindow *window = orqa_create_GLFW_window(SCR_WIDTH, SCR_HEIGHT, "Vegvisir Project", NULL, NULL); // glfw window object creation
    if (window == NULL)
        return OPENGL_INIT_ERROR;
    orqa_make_window_current(window);

    orqa_set_frame_buffer_cb(window, orqa_framebuffer_size_callback); // manipulate view port
    orqa_set_cursor_position_cb(window, orqa_mouse_callback);         // move camera_t with cursor
    orqa_set_input_mode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);   // use cursor but do not display it
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
    pthread_t readFromSerial, readFromUDP, moveCamera;
    // pthread_create(&readFromUDP, NULL, orqa_udp_thread, &cam);
    pthread_create(&readFromSerial, NULL, orqa_read_from_serial, NULL);
    pthread_create(&moveCamera, NULL, orqa_move_camera, &cam);

    if (pthread_mutex_init(&mutexLock, NULL) != 0)
    {
        fprintf(stderr, "Mutex init has failed! \n");
        goto threadError;
    }

    // MVP matrices init
    mat4 model, proj, view;
    glm_mat4_identity(model);
    glm_mat4_identity(view);
    glm_mat4_identity(proj);

    // get MVP shader indexes
    GLuint modelLoc = orqa_get_uniform_location(shaderProgram, "model");
    GLuint viewLoc = orqa_get_uniform_location(shaderProgram, "view");
    GLuint projLoc = orqa_get_uniform_location(shaderProgram, "proj");

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

/// This function keeps track all the input code.
/// Moves between DSS, MRSS and 360 modules using 'D', 'M' or '3' keys
static void orqa_process_input(GLFWwindow *window)
{
    // keeps all the input code
    if (orqa_get_key(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        EXIT = 1;
        orqa_sleep(ORQA_SLEEP_SEC, 1);
        glfwSetWindowShouldClose(window, TRUE);
    }
    if (orqa_get_key(window, GLFW_KEY_3) == GLFW_PRESS)
    {
        mode = 0;
    }
    if (orqa_get_key(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        mode = 1;
    }
    if (orqa_get_key(window, GLFW_KEY_M) == GLFW_PRESS)
    {
        mode = 2;
    }
}

/// This function converts radians from degrees.
/// Returns radians in float.
static float orqa_radians(const float deg)
{
    return (deg * M_PI / 180.0f); // calculate radians
}

/// This function connects to ORQA FPV.One goggles via UDP socket and performs motorless gimbal while goggles are in use.
static void *orqa_udp_thread(ORQA_REF void *c_ptr)
{
    // inits
    fprintf(stderr, "In thread\n");
    orqa_camera_t *c = c_ptr;
    char buf[BUFSIZE];
    float yaw, pitch, roll;
    versor rollQuat, pitchQuat, yawQuat;
    glm_quat_identity(rollQuat);
    glm_quat_identity(yawQuat);
    glm_quat_identity(pitchQuat);
    struct sockaddr_in serveraddr;
    int s, recv_len, optval = 1;

    // create a UDP socket
    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < -1)
    {
        printf("socket failed init\n");
        return NULL;
    }
    printf("Socket created!\n");
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));
    memset((char *)&serveraddr, 0, sizeof(serveraddr));

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(PORT);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    // bind socket to port
    if (bind(s, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1)
    {
        printf("Binding error!\n");
        goto exitUDP;
    }
    printf("W8ing for packets!!!\n");
    while (1)
    {
        bzero(buf, BUFSIZE);

        if (EXIT)
            goto exitUDP;

        if ((recv_len = recv(s, buf, BUFSIZE, 0)) < 0)
        {
            printf("Recieving error!\n");
            break;
        }
        
        // parse JSON
        JSONObject *json = parseJSON(buf);
        yaw = atof(json->pairs[0].value->stringValue);
        pitch = -atof(json->pairs[1].value->stringValue);
        roll = -atof(json->pairs[2].value->stringValue);
        free(json);

        // Using quaternions to calculate camera rotations
        glm_quatv(pitchQuat, orqa_radians(pitch), (vec3){1.0f, 0.0f, 0.0f});
        glm_quatv(yawQuat, orqa_radians(yaw), (vec3){0.0f, 1.0f, 0.0f});
        glm_quatv(rollQuat, orqa_radians(roll), (vec3){0.0f, 0.0f, 1.0f});

        pthread_mutex_lock(&mutexLock);
        glm_quat_mul(yawQuat, pitchQuat, c->resultQuat);
        glm_quat_mul(c->resultQuat, rollQuat, c->resultQuat);
        pthread_mutex_unlock(&mutexLock);
    }
exitUDP:
    close(s);
    printf("UDO socket closed!\n\n");
    return NULL;
}

static void *orqa_read_from_serial()
{
    // Open the serial port. Change device path as needed (currently set to an standard FTDI USB-UART cable type device)
    int serial_port = open("/dev/ttyUSB0", O_RDONLY); // Create new termios struc, we call it 'tty' for convention
    struct termios tty;                             // Read in existing settings, and handle any error
    if (tcgetattr(serial_port, &tty) != 0)
    {
        printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
        goto exitSerial;
    }
    tty.c_cflag &= ~PARENB;        // Clear parity bit, disabling parity (most common)
    tty.c_cflag &= ~CSTOPB;        // Clear stop field, only one stop bit used in communication (most common)
    tty.c_cflag &= ~CSIZE;         // Clear all bits that set the data size
    tty.c_cflag |= CS8;            // 8 bits per byte (most common)
    tty.c_cflag &= ~CRTSCTS;       // Disable RTS/CTS hardware flow control (most common)
    tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)
    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO;                                                        // Disable echo
    tty.c_lflag &= ~ECHOE;                                                       // Disable erasure
    tty.c_lflag &= ~ECHONL;                                                      // Disable new-line echo
    tty.c_lflag &= ~ISIG;                                                        // Disable interpretation of INTR, QUIT and SUSP
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);                                      // Turn off s/w flow ctrl
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL); // Disable any special handling of received bytes
    tty.c_oflag &= ~OPOST;                                                       // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR;                                                       // Prevent conversion of newline to carriage return/line feed

    tty.c_cc[VTIME] = 1;
    tty.c_cc[VMIN] = 0;

    // Set in/out baud rate to be 115200
    cfsetispeed(&tty, B115200);
    cfsetospeed(&tty, B115200);

    // Save tty settings, also checking for errors
    if (tcsetattr(serial_port, TCSANOW, &tty) != 0)
    {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
        goto exitSerial;
    }
    // Allocate memory for read buffer, set size according to your needs
    printf("SERIAL = OK!\n");

    char ch;
    do
    {
        read(serial_port, &ch, sizeof(ch));
    } while (ch != ';');
    
    while (1)
    {
        if (EXIT)
            goto exitSerial;
        int index = 0;
        char headTrackingBuffer[40] = "\0";
        do
        {
            if (EXIT)
                goto exitSerial;
            read(serial_port, &ch, sizeof(ch));
            headTrackingBuffer[index++] = ch;
        } while (ch != ';');
        memcpy(serialBuffer, headTrackingBuffer, sizeof(headTrackingBuffer));
        FLAG = 1;
    }
exitSerial:
    close(serial_port);
    printf("\nSerial port closed!\n");
    return NULL; // success
}

static void *orqa_move_camera(ORQA_REF void *c_ptr)
{
    orqa_camera_t *c = c_ptr;
    float yaw, pitch, roll;
    versor rollQuat, pitchQuat, yawQuat;
    glm_quat_identity(rollQuat);
    glm_quat_identity(yawQuat);
    glm_quat_identity(pitchQuat);

    printf("IN MOVE CAMERA THREAD!");

    while (1)
    {
        if (EXIT)
            goto exitThread;
        if (FLAG)
        {
            int b = 0, count = 0;
            char yawBuf[12] = "\0";
            char pitchBuf[12] = "\0";
            char rollBuf[12] = "\0";
            for (int i = 0; i < HEADTRACKING_BUFFER_SIZE - 1; i++)
            {
                const char ch = serialBuffer[i];

                if (ch == ';')
                    break;

                if (ch == ',')
                {
                    b = 0;
                    count++;
                    continue;
                }
                if (count == 0)
                    yawBuf[b++] = ch;
                else if (count == 1)
                    pitchBuf[b++] = ch;
                else
                    rollBuf[b++] = ch;
                if (EXIT)
                    goto exitThread;
            }
            yaw = atof(yawBuf);
            pitch = -atof(pitchBuf);
            roll = atof(rollBuf);
            glm_quatv(pitchQuat, orqa_radians(pitch), (vec3){1.0f, 0.0f, 0.0f});
            glm_quatv(yawQuat, orqa_radians(yaw), (vec3){0.0f, 1.0f, 0.0f});
            glm_quatv(rollQuat, orqa_radians(roll), (vec3){0.0f, 0.0f, 1.0f});

            pthread_mutex_lock(&mutexLock);
            glm_quat_mul(yawQuat, pitchQuat, c->resultQuat);
            glm_quat_mul(c->resultQuat, rollQuat, c->resultQuat);
            pthread_mutex_unlock(&mutexLock);
            FLAG = 0;
        }
    }
exitThread:
    return NULL; // success
}