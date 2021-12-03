#include "orqa_opengl.h"

GLint orqa_load_glad(GLADloadproc load){
	return gladLoadGLLoader(load);
}

GLuint orqa_load_shader_from_file(const char *filename, GLenum type){
    FILE *fp = 0;
	fp = fopen(filename, "rb");
	if (!fp)
	{
		printf("Can not open shader \n");
		return 0;
	}
	return orqa_create_shader(fp, type);
}

GLuint orqa_create_shader(FILE *fp, GLenum shader_type){
    fseek(fp, 0, SEEK_END);
	long file_size = ftell(fp) + 1;
	char *buffer;
	GLuint shader;
	buffer = (char *)malloc(file_size * sizeof(char));
	fseek(fp, 0, SEEK_SET);
	fread(buffer, file_size, sizeof(char), fp);
	shader = glCreateShader(shader_type);
	const char *shaders_src = buffer;
	glShaderSource(shader, 1, &shaders_src, NULL);
	glCompileShader(shader);

    orqa_get_shader_status(shader);
    free(buffer);
    return shader;
}

void orqa_get_shader_status(const GLuint shader){
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
}

GLuint orqa_create_program(GLuint *shaders, unsigned int shader_num)
{
	GLuint program = glCreateProgram();
	for (unsigned int i = 0; i < shader_num; i++)
	{
		glAttachShader(program, shaders[i]);
	}

	glLinkProgram(program);
    orqa_get_program_status(program);
	for (unsigned int i = 0; i < shader_num; i++)
		glDeleteShader(shaders[i]);

	return program;
}

void orqa_delete_program(GLuint program){
	glDeleteProgram(program);
}

void orqa_use_program(GLuint program){
    glUseProgram(program);
}

GLuint orqa_get_attrib_location(GLuint shader, char *name){
    return glGetAttribLocation(shader, name);
}   

GLuint orqa_get_uniform_location(GLuint shader, char *name){
    return glGetUniformLocation(shader, name);
}   

void orqa_get_program_status(const GLuint program){
    GLint log_size = 0;
	glGetShaderiv(program, GL_INFO_LOG_LENGTH, &log_size);
	if (log_size > 0)
	{
		char *log;
		log = (char *)malloc(log_size);
		glGetProgramInfoLog(program, log_size, NULL, log);
		printf("%s\n", log);
		free(log);
	}
}

void orqa_bind_texture(GLuint texture_id){
    glBindTexture(GL_TEXTURE_2D, texture_id);
}

void orqa_load_texture_from_file(const char *filename){
    orqa_texture_t tex;
    tex.data = stbi_load(filename, &tex.width, &tex.height, &tex.nrChannels, 0);
    if (tex.data){
        if ((int) tex.nrChannels == 3)  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex.width, tex.height, 0, GL_RGB, GL_UNSIGNED_BYTE, tex.data);
        else if ((int) tex.nrChannels == 4) glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex.width, tex.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex.data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(tex.data);
    }else{
        fprintf(stderr, "In file: %s, line: %d Failed to load texture\n", __FILE__, __LINE__);
    } 
}

GLuint *orqa_create_textures(const int number_of_textures){
    GLuint *textures = calloc(number_of_textures, sizeof(GLuint));;
    glGenTextures(number_of_textures, textures);
    return textures;
}

void orqa_delete_textures(const int number_of_textures, GLuint *textures){
    glDeleteTextures(number_of_textures, textures);
}

void orqa_enable_vertex_attrib_array(GLuint location, GLuint size, GLenum type, GLsizei stride, const GLvoid *ptr){
    glVertexAttribPointer(location, size, type, GL_FALSE, stride, ptr);
    glEnableVertexAttribArray(location);
}

void orqa_bind_VAOs(GLuint VAO){
    glBindVertexArray(VAO);
}

void orqa_draw_elements(GLenum type, GLsizei count){
    glDrawElements(type, count, GL_UNSIGNED_INT, 0);
}

void orqa_bind_buffer(GLenum type, GLuint buffer){
    glBindBuffer(type, buffer);
}

void orqa_set_buffer_data(GLenum type,GLsizeiptr size,const GLvoid *data, GLenum usage){
	glBufferData(type, size, data, usage);
}

void orqa_generate_buffers(const int count, GLuint *buffers){
	glGenBuffers(count, buffers);
}

GLuint *orqa_generate_VBOs(GLsizei count){
	GLuint * buffer = calloc(count, sizeof(GLuint));
	orqa_generate_buffers(count, buffer);
	return buffer;
}

GLuint *orqa_generate_VAOs(GLsizei count){
	GLuint * arrays = calloc(count, sizeof(GLuint));
	glGenVertexArrays(count, arrays);
	return arrays;
}

GLuint *orqa_generate_EBOs(GLsizei count){
	GLuint * buffer = calloc(count, sizeof(GLuint));
	orqa_generate_buffers(count, buffer);
	return buffer;
}

void orqa_delete_buffers(GLsizei n, const GLuint *buffer){
	glDeleteBuffers(n, buffer);
}

void orqa_delete_VAOs(GLsizei n, const GLuint *arrays){
	glDeleteVertexArrays(n, arrays);
}

void orqa_send_shander_4x4_matrix(GLint location,GLsizei count,GLfloat *matrix){
	glUniformMatrix4fv(location, count, GL_FALSE, matrix);
}

void orqa_clear_color_buffer(GLfloat r, GLfloat g, GLfloat b, GLfloat a){
	glClearColor(r,g,b,a);
}
void orqa_clear_buffer(GLbitfield mask){
	glClear(mask);
}
void orqa_clear_depth_buffer(GLclampd depth){
	glClearDepth(depth);
}

void orqa_bind_buffer_set_data(GLenum type, GLuint buffer, GLsizeiptr size, const GLvoid *data, GLenum usage){
	orqa_bind_buffer(type , buffer);
    orqa_set_buffer_data(type , size, data, usage );
}

void orqa_bind_vertex_object_and_draw_it(GLuint vao, GLenum type, GLsizei count){
	orqa_bind_VAOs(vao);
	orqa_draw_elements(type, count);
}


void orqa_viewport(GLint x, GLint y, GLsizei witdh, GLsizei height){
    glViewport(x, y, witdh, height);
}