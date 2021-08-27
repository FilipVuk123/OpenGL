#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

const float radius = 0.7f;
const unsigned int sectors = 5; 
const unsigned int stacks = 5; 


GLfloat *vertices;
GLuint *indices;

void GenSphere(GLfloat radius, GLuint sectors, GLuint stacks){
    GLfloat *verticesX = calloc(sectors*stacks, sizeof(float));
    GLfloat *verticesY = calloc(sectors*stacks, sizeof(float));
    GLfloat *verticesZ = calloc(sectors*stacks, sizeof(float));
    for (unsigned int i = 0; i < stacks; i++){
        GLfloat V   = i / (GLfloat) stacks;
        GLfloat phi = V * M_PI;

        for (unsigned int j = 0; j < sectors; j++){
            GLfloat U = j / (GLfloat) sectors;
            GLfloat theta = U * (M_PI * 2);

            GLfloat x = cosf (theta) * sinf (phi);
            GLfloat y = cosf (phi);
            GLfloat z = sinf (theta) * sinf (phi);

            *(verticesX + stacks*i + j) = (x * radius);
            *(verticesY + stacks*i + j) = (y * radius);
            *(verticesZ + stacks*i + j) = (z * radius);
        }
    }
    vertices = calloc(sectors*stacks*3, sizeof(GLfloat));
    unsigned int j = 0;
    for(unsigned int i = 0; i < 3*sectors*stacks; i=i+3){
        *(vertices + i) = *(verticesX+j);
        *(vertices + i+1) = *(verticesY+j);
        *(vertices + i+2) = *(verticesZ+j);
        j++;
    }
    indices = calloc((sectors * stacks + sectors)*6, sizeof(GLint));
    j = 0;
    for (unsigned int i = 0; i < sectors * stacks + sectors; ++i){
        *(indices + j++) = i;
        *(indices + j++) = i + sectors + 1;
        *(indices + j++) = i + sectors;
        
        *(indices + j++) = i + sectors + 1;
        *(indices + j++) = i;
        *(indices + j++) = i + 1;
    }
}

int main(){
    GenSphere(radius, sectors, stacks);
    // for (unsigned int i = 0; i < 3*sectors*stacks; i++ )
   	    //printf("*(vertices + %d) = %lf\n", i, *(vertices+i) );
    for (unsigned int i = 0; i < (sectors * stacks + sectors)*6; i++)
        printf("*(indices + %d) = %d\n", i, *(indices+i) );
    return 0;
}