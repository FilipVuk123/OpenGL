
#include <stdio.h>
#include <GLES2/gl2.h>

#include <stdio.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>	// inet_addr
#include <unistd.h>
#include <netinet/in.h>
#include <cglm/cglm.h> 
#include "json.h"
#include "gen_sphere.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080; 

const char *vertexShaderSource = 
    "#version 200 es\n"
    "precision mediump float;\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec2 aTexCoord;\n"
    "out vec2 TexCoord;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 proj;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = proj*view*model*vec4(aPos.x, aPos.y, aPos.z , 1.);\n" 
    "   TexCoord = vec2(1. - aTexCoord.x, aTexCoord.y);\n"
    "}\n\0";

const char *fragmentShaderSource = 
    "#version 200 es\n"
    "precision mediump float;\n"
    "in vec2 TexCoord;\n"
    "out vec4 FragColor;\n"
    "uniform sampler2D texture1;\n" 
    "void main()\n"
    "{\n"
    "   FragColor = texture(texture1, TexCoord);\n" 
    "}\n\0";

typedef struct camera_t{
    vec3 cameraPos;
    GLfloat fov;
    versor resultQuat;
}camera_t;

int main(){
    
    // generating sphere
    orqa_sphere_t sph;
    sph.radius = 1.0f; sph.sectors = 50; sph.stacks = 50;
    orqa_gen_sphere(&sph);
    GLfloat vertices[sph.numVertices*5]; for(int i = 0; i < sph.numVertices*5; i++) vertices[i] = *(sph.Vs + i);
    GLuint indices[sph.numTriangles*3]; for(int i = 0; i < sph.numTriangles*3; i++) indices[i] = *(sph.Is + i);
    orqa_sphere_free(&sph);

    camera_t cam;
    cam.cameraPos[0] = 0.0f; cam.cameraPos[1] = 0.0f; cam.cameraPos[2] = 0.0f;
    cam.resultQuat[0] = 0.0f; cam.resultQuat[1] = 0.0f; cam.resultQuat[2] = 0.0f; cam.resultQuat[3] = 1.0f;
    cam.fov = 4.8f;

    printf("Hello world!\n");
    
    return 0;
}