#ifndef ORQA_VEGVISIR_COMMON_H
#define ORQA_VEGVISIR_COMMON_H

#include <vendor/cglm/cglm.h>
#include "vendor/json.h"
#include "orqa_gen_mash.h"
#include "orqa_clock.h"
#include "orqa_opengl.h"
#include "orqa_input.h"
#include "orqa_window.h"
#include <sys/socket.h>
#include <arpa/inet.h> // inet_addr
#include <unistd.h>
#include <netinet/in.h>

#include <fcntl.h>   // Contains file controls like O_RDWR
#include <errno.h>   // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h>  // write(), read(), close()


#define BUFSIZE 1024
#define HEADTRACKING_BUFFER_SIZE 64
#define PORT 8000

int EXIT;
int mode1;
int mode2;

void orqa_process_input(GLFWwindow *window);
float orqa_radians(const float deg);
void *orqa_udp_thread(void *c_ptr);
void *orqa_read_from_serial();

#endif