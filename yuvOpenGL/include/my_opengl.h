#ifndef my_OPENGL_H
#define my_OPENGL_H

#include <stdio.h>
#include <stdlib.h>
#include <vendor/glad/glad.h>
#include "vendor/stb_image.h"

typedef struct my_texture_t{
    int width, height, nrChannels;
    uint8_t* data;
}my_texture_t;

// This function loads glad. Use (GLADloadproc)my_get_proc_address as its argument.
// Returns 0 on failure
GLint my_load_glad(
    GLADloadproc load);

// This function loads shader from .txt files and returns shader's ID
GLuint my_load_shader_from_file(
    const char *file_name, 
    GLenum type);

// This function prints shader's info log
void my_get_shader_status(
    const GLuint shader);

// This function attaches a shader object to a program object and combines all provided shader programs into one.
// Then deletes shaders
GLuint my_create_program(
    GLuint *shaders, 
    GLuint shader_num1,
    GLuint shader_num2);

// This function deletes created shader program
void my_delete_program(
    GLuint program);

// This function prints programs's info log
void my_get_program_status(
    const GLuint program);

// This function installs a program object as part of current rendering state
void my_use_program(
    GLuint shader);

// This function returns the location of an attribute variable
GLuint my_get_attrib_location(
    GLuint shaderProgram, 
    char *name);

// This function returns the location of a uniform variable
GLuint my_get_uniform_location(
    GLuint shaderProgram, 
    char *name);

// This function creates textures and returns an array of texture indices
GLuint *my_create_textures(
    GLsizei number_of_textures);

// This function binds a named texture to a texturing target
void my_bind_texture(
    GLuint texture_id);

// This function loads a texture from file to previously bound texture
void my_load_texture_from_file(
    const char *filename);

// This function deletes previously allocated textures
void my_delete_textures(
    GLsizei number_of_textures, 
    GLuint *textures);

// This function defines an array of generic vertex attribute data and enables it
void my_enable_vertex_attrib_array(
    GLuint location, 
    GLuint size, 
    GLenum type, 
    GLsizei stride, 
    const GLvoid *ptr);

// This function binds a vertex array object
void my_bind_VAOs(
    GLuint VAO);

// This function renders primitives from bound array data
void my_draw_elements(
    GLenum type, 
    GLsizei n);

// This function binds named buffer object
void my_bind_buffer(
    GLenum type, 
    GLuint buffer);

// This function creates and initializes a buffer object's data store
void my_set_buffer_data(
    GLenum	type,
    GLsizeiptr	size,
    const GLvoid *data,
    GLenum	usage);

// This function generates an array of n VBO-s 
GLuint *my_generate_VBOs(
	GLsizei n);

// This function generates an array of n VAO-s 
GLuint *my_generate_VAOs(
	GLsizei n);

// This function generates an array of n EBO-s 
GLuint *my_generate_EBOs(
	GLsizei n);

// This function generates n buffers
void my_generate_buffers(
    GLsizei n, 
    GLuint *buffers);

// This function generates n generated buffers
void my_delete_buffers(
    GLsizei n, 
    const GLuint *buffer);

// This function deletes generated VAOs
void my_delete_VAOs(
    GLsizei n, 
    const GLuint *arrays);

// This function sends a matrix to shader on speciffied location
void my_send_shander_4x4_matrix(
    GLint location,
    GLsizei n,
    GLfloat *matrix
);

// This function specify clears values for color buffers
void my_clear_color_buffer(
    GLfloat r, GLfloat g, GLfloat b, GLfloat a);
    
// This function clears buffers to preset values
void my_clear_buffer(
    GLbitfield mask);

// This function specify clears values for depth buffers
void my_clear_depth_buffer(
    GLclampd depth);


// This function binds buffer and then sets it's data
void my_bind_buffer_set_data(
    GLenum type, 
    GLuint buffer, 
    GLsizeiptr size, 
    const GLvoid *data, 
    GLenum usage);

// This function binds VAO and then renders it's elements
void my_bind_vertex_object_and_draw_it(
    GLuint vao, 
    GLenum type, 
    GLsizei n);

// This function sets viewport
void my_set_viewport(
    GLint x, GLint y, 
    GLsizei witdh, GLsizei height);


/// This function generates 2D texture from buffer
void my_generate_texture_from_buffer(
    GLenum target,
    GLint internal_format,
    GLsizei width,
    GLsizei height,
    GLenum format,
    GLenum type,
    const GLvoid * buffer);

GLuint my_create_shader_from_source(
    GLenum type,
    GLsizei size, 
    const GLchar **source);

void my_update_texture_from_buffer(
    GLenum target,
    GLuint xoffset,
    GLuint yoffset,
    GLsizei width,
    GLsizei height,
    GLenum format,
    GLenum type,
    const GLvoid * buffer);

#endif