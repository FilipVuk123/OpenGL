#include "opengl_api.h"
#include "display.h"
#include <unistd.h>

#define DEBUG

/*  Creates shader from passed file and returns compiled shader*/
GLint create_shader(FILE *fp, GLenum shader_type)
{
	fseek(fp, 0, SEEK_END);
	long file_size = ftell(fp) + 1;
	char *buffer;
	GLint shader;
	buffer = (char *)malloc(file_size * sizeof(char));
	fseek(fp, 0, SEEK_SET);
	fread(buffer, file_size, sizeof(char), fp);
	shader = glCreateShader(shader_type);
	const char *shaders_src = buffer;
	glShaderSource(shader, 1, &shaders_src, NULL);
	glCompileShader(shader);
#ifdef DEBUG
	check_shader_status(shader);
	check_gl_error("create_shader");
#endif
	free(buffer);
	
	printf("hello from create_shader\n");
	return shader;
}

/* Creates program with passed shaders, links program and 
deletes passed shader but does not set it as active*/
GLint create_program(GLint *shaders, unsigned int shader_num)
{
	GLint program = glCreateProgram();
	for (int i = 0; i < shader_num; i++)
	{
		glAttachShader(program, shaders[i]);
	}

	glBindAttribLocation(program, 0, "g_vVertex");
	glBindAttribLocation(program, 1, "g_vColor");

	glLinkProgram(program);

	for (int i = 0; i < shader_num; i++)
		glDeleteShader(shaders[i]);

#ifdef DEBUG
	check_gl_error("create_program");
#endif
	printf("hello from create_program\n");
	return program;
}

void use_program(GLint program)
{
	glUseProgram(program);

#ifdef DEBUG
	//check_program_status(program);
	check_gl_error("use_program");
#endif
	printf("hello from use_program\n");
}

/* returns "vPosition" and "vTexCoord" shader attribute locations */
ATTRIBUTE_LOCATION configure_shader_attributes(GLint shader_program, DISPLAY_INFO info)
{
	ATTRIBUTE_LOCATION attrib;

	glViewport(0, 0, info.width, info.height);

	attrib.vPosition = glGetAttribLocation(shader_program, "vPosition");
	attrib.vTexCoord = glGetAttribLocation(shader_program, "vTexCoord");

	glVertexAttribPointer(attrib.vPosition, 2, GL_FLOAT, GL_FALSE, 0, info.vert_coord_info);
	glVertexAttribPointer(attrib.vTexCoord, 2, GL_SHORT, GL_FALSE, 0, info.tex_coord_info);

	glEnableVertexAttribArray(attrib.vPosition);
	glEnableVertexAttribArray(attrib.vTexCoord);

#ifdef DEBUG
	check_gl_error("configure_shader_attributes");
#endif
	printf("hello from configure_shader_attributes\n");
	return attrib;
}

/* returns enabled position and color uniforms a.k.a. textures from shader */
void get_uniform_locations(char **name_of_uniforms, TEX *texes, int uniform_num, GLint shader_program)
{
	if (name_of_uniforms != NULL)
		for (int i = 0; i < uniform_num; i++)
		{
			texes[i].loc = glGetUniformLocation(shader_program, name_of_uniforms[i]);
		}
	else
	{
		name_of_uniforms = (char **)malloc(sizeof(char *) * uniform_num);
		for (int i = 0; i < uniform_num; i++)
			name_of_uniforms[i] = (char *)malloc(sizeof(char) * 10);

		name_of_uniforms[0] = "yTexture";
		if (uniform_num > 2)
		{
			name_of_uniforms[1] = "uTexture";
			name_of_uniforms[2] = "vTexture";
		}
		else
			name_of_uniforms[1] = "uvTexture";

		for (int i = 0; i < uniform_num; i++)
		{
			texes[i].loc = glGetUniformLocation(shader_program, name_of_uniforms[i]);
			printf("loc %d\n", texes[i].loc);
			check_gl_error("get_uniform_location");
		}
	}

#ifdef DEBUG
	check_gl_error("get_uniform_location");
#endif
printf("hello from get_uniform_locations\n");
}

/* create textures and allocate undelying memory for each texture */
void create_textures(TEX *textures, GLint program, int tex_index,
										 int width, int height, GLenum format)
{
	check_gl_error("b4 for");
	glActiveTexture(GL_TEXTURE0 + tex_index);
	glGenTextures(1, &(textures[tex_index].id));
	glBindTexture(GL_TEXTURE_2D, textures[tex_index].id);

	glUniform1i(textures[tex_index].loc, tex_index);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, NULL);
	printf("create_texes dims: %d %d \n id: %d\n location: %d\n", width, height, textures[tex_index].loc, textures[tex_index].loc);

#ifdef DEBUG
	check_gl_error("tex image");
#endif

	glBindTexture(GL_TEXTURE_2D, 0);
	printf("hello from create_textures\n");
}

/* creates frame buffers and render buffers for each frame buffer */
DISPLAY_BUFFER get_display_buffers(int num_of_render_buffers, int num_of_frame_buffers, int width, int height)
{
	DISPLAY_BUFFER buffer;
	buffer.frame_buffers = (GLint *)malloc(sizeof(GLint) * num_of_frame_buffers);
	buffer.render_buffers = (GLint **)malloc(sizeof(GLint *) * num_of_frame_buffers);

	glGenFramebuffers(num_of_frame_buffers, buffer.frame_buffers);

	if (buffer.frame_buffers != NULL)
		for (int i = 0; i < num_of_frame_buffers; i++)
		{
			buffer.render_buffers[i] = (GLint *)malloc(sizeof(GLint) * num_of_render_buffers);

			glBindFramebuffer(GL_FRAMEBUFFER, buffer.frame_buffers[i]);
			glGenRenderbuffers(num_of_render_buffers, buffer.render_buffers[i]);

			if (buffer.render_buffers[i] != NULL)
			{
				for (int j = 0; j < num_of_render_buffers; j++)
				{
					glBindRenderbuffer(GL_RENDERBUFFER, buffer.render_buffers[i][j]);
					glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA4, width, height);
					glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, buffer.render_buffers[i][j]);
				}
			}
			else
			{
				printf("no renders\n");
			}
		}
	else
	{
		printf("no frame buffers\n");
	}

#ifdef DEBUG
	check_gl_error("create_frame_buffer");
#endif
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	printf("hello from get_display_buffers\n");
	return buffer;
}

void prepare_for_drawing(DISPLAY_BUFFER buffer, int frame_buffer_index, int render_buffer_index)
{
	display_dispatch();
	// Clear the backbuffer and depth-buffer
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindFramebuffer(GL_FRAMEBUFFER, buffer.frame_buffers[frame_buffer_index]);
	glBindRenderbuffer(GL_RENDERBUFFER, buffer.render_buffers[frame_buffer_index][render_buffer_index]);
	printf("render buffer bound to frame buffer\n");
}

/* update previously allocated textures */
void update_textures(void *data, int width, int height, GLenum format, int tex_index)
{
	glActiveTexture(GL_TEXTURE0 + tex_index);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, data);
	//printf("update dims: %d %d format: %x\n", width, height, format);
#ifdef DEBUG
	check_gl_error("update_textures");
#endif
	printf("hello from update_textures\n");
}

/* draws to egl buffer and swaps egl buffers */
void display_textures(DISPLAY_INFO *info)
{
	glDrawElements(GL_TRIANGLE_STRIP, 6, GL_UNSIGNED_SHORT, info->index_order);
	RefreshWindow();
	printf("hello from display_textures\n");
}

/* create coordinates for drawing the frame */
void get_draw_info(int width, int height, DISPLAY_INFO *disp_info)
{
	GLfloat kVertexInformation[8] =
			{
					-1, 1,	// TexCoord 0 top left
					-1, -1, // TexCoord 1 bottom left
					1, -1,	// TexCoord 2 bottom right
					1, 1		// TexCoord 2 top right
			};
	GLshort kTextureCoordinateInformation[8] =
			{
					0, 0, // TexCoord 0 top left
					0, 1, // TexCoord 1 bottom left
					1, 1, // TexCoord 2 bottom right
					1, 0	// TexCoord 2 bottom right
			};

	GLshort kIndicesInformation[6] =
			{
					0, 1, 2,
					0, 2, 3};

	disp_info->stride = 0;

	/* disp_info->tex_coord_info = (GLshort *)calloc(sizeof(GLshort), 8);
	disp_info->vert_coord_info = (GLfloat *)calloc(sizeof(GLfloat), 8);
	disp_info->index_order = (GLshort *)calloc(sizeof(GLshort), 6); */

	for (int i = 0; i < 8; i++)
	{
		disp_info->vert_coord_info[i] = kVertexInformation[i];
		disp_info->tex_coord_info[i] = kTextureCoordinateInformation[i];

		if (i < 6)
		{
			disp_info->index_order[i] = kIndicesInformation[i];
			//printf("index %d\n", disp_info->index_order[i]);
		}

		//printf("tex %d,\nvert %f\n", disp_info->tex_coord_info[i], disp_info->vert_coord_info[i]);
	}

	disp_info->height = height;
	disp_info->width = width;
	printf("hello from get_draw_info\n");
}

/* bind textures to frame_buffer */
void bind_texture_to_buffer(GLint frame_buffer, TEX *textures, int texture_num)
{
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
	for (int i = 0; i < texture_num; i++)
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[i].id, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

#ifdef DEBUG
	check_gl_error("bind_texture_to_buffer");
#endif
printf("hello from get_draw_info\n");	
}

static void check_program_status(GLint program)
{
	GLint log_size = 0;
	glGetShaderiv(program, GL_INFO_LOG_LENGTH, &log_size);
	check_gl_error("get shader iv");
	if (log_size > 0)
	{
		char *log;
		log = (char *)malloc(log_size);
		glGetProgramInfoLog(program, log_size, NULL, log);
		check_gl_error("program info log");
		printf("%s\n", log);
		free(log);
	}
}
static void check_shader_status(GLint shader)
{
	GLint log_size = 0;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_size);

	if (log_size > 0)
	{
		char *log;
		log = (char *)malloc(log_size);
		glGetShaderInfoLog(shader, log_size, NULL, log);
		printf("%s\n", log);
		free(log);
	}
	check_gl_error("check_shader_status");
}

static void check_gl_error(const char *func_name)
{
	for (GLint error = glGetError(); error; error = glGetError())
	{
		printf("after %s() glError (0x%x)\n", func_name, error);
	}
	//free(func_name);
}