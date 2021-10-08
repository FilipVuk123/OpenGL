#ifndef _OPENGL_API_H_
#define _OPENGL_API_H_

#include <GLES2/gl2.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct TEXTURE
{
	GLint loc;
	GLint id;
} TEX;

typedef struct ATTRIBUTE_LOCATION
{
	GLint vPosition;
	GLint vTexCoord;
} ATTRIBUTE_LOCATION;

typedef struct DISPLAY_INFO
{
	GLshort tex_coord_info[8];
	GLfloat vert_coord_info[8];
	GLshort index_order[6];
	GLint stride;
	int height;
	int width;
} DISPLAY_INFO;

typedef struct DISPLAY_BUFFER
{
	GLint **render_buffers;
	GLint *frame_buffers;
} DISPLAY_BUFFER;

typedef struct DISPLAY_DATA
{
	void **data;
	int data_planes; //number of data planes to render to texture
	int size;
	ATTRIBUTE_LOCATION attribs;
	DISPLAY_INFO info;
	DISPLAY_BUFFER buf;
	TEX *texes;
	GLint program;
} DISPLAY_CONTEXT;

void initialize_display(int screen_width, int screen_height);
GLint create_shader(FILE *fp, GLenum shader_type);
GLint create_program(GLint *shaders, unsigned int shader_num);
void use_program(GLint program);
ATTRIBUTE_LOCATION configure_shader_attributes(GLint shader_program, DISPLAY_INFO info);
void get_uniform_locations(char **name_of_uniforms, TEX *texes, int uniform_num, GLint shader_program);
void create_textures(TEX *textures, GLint program, int tex_index,
										 int width, int height, GLenum format);
DISPLAY_BUFFER get_display_buffers(int num_of_render_buffers, int num_of_frame_buffers, int width, int height);
void prepare_for_drawing(DISPLAY_BUFFER buffer, int frame_buffer_index, int render_buffer_index);
void update_textures(void *data, int width, int height, GLenum type, int tex_index);
void display_textures(DISPLAY_INFO *info);
void get_draw_info(int height, int width, DISPLAY_INFO *disp_info);
void bind_texture_to_buffer(GLint frame_buffer, TEX *textures, int texture_num);
static void check_gl_error(const char *func_name);
static void check_shader_status(GLint shader_program);
static void check_program_status(GLint shader_program);
#endif