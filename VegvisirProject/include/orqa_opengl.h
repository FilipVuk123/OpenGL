#ifndef ORQA_OPENGL_H
#define ORQA_OPENGL_H

#include <stdio.h>
#include <stdlib.h>
#include <vendor/glad/glad.h>
#include <vendor/GLFW/glfw3.h>
#include "vendor/stb_image.h"

typedef struct orqa_texture_t{
    int width, height, nrChannels;
    uint8_t* data;
}orqa_texture_t;

GLint orqa_init_glfw(const int major_version, const int minor_verion);

GLuint orqa_load_shader_from_file(
    const char *file_name, 
    GLenum type);

GLuint orqa_create_shader(
    FILE *fp, 
    GLenum shader_type);
void orqa_get_shader_status(
    const GLuint shader);

GLuint orqa_create_program(
    GLuint *shaders, 
    GLuint shader_num);

void orqa_get_program_status(
    const GLuint program);

void orqa_use_program(
    GLuint shader);

GLuint orqa_get_attrib_location(
    GLuint shaderProgram, 
    char *name);
GLuint orqa_get_uniform_location(
    GLuint shaderProgram, 
    char *name);

void orqa_use_program(
    GLuint program);

GLuint *orqa_create_textures(
    const int number_of_textures);

void orqa_bind_texture(
    GLuint texture_id);

void orqa_load_texture_from_file(
    const char *filename);

void orqa_delete_textures(
    const int number_of_textures, 
    GLuint *textures);

void orqa_enable_vertex_attrib_array(
    GLuint location, 
    GLuint size, 
    GLenum type, 
    GLsizei stride, 
    const GLvoid *ptr);

void orqa_bind_VAOs(
    GLuint VAO[]);

void orqa_draw_elements(
    GLenum type, 
    GLsizei count);


#endif