#ifndef _OPENGL_API_H
#define _OPENGL_API_H

#include <stdio.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

GLuint create_shader(FILE *fp, GLenum shader_type);
GLuint create_program(GLuint *shaders, unsigned int shader_num);
void use_program(GLuint program);
GLuint getUniformLocation(GLuint shaderProgram, char *name);
GLuint getAtribLocation(GLuint shaderProgram, char *name);


#endif