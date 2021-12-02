#include "orqa_opengl.h"

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
	for (int i = 0; i < shader_num; i++)
	{
		glAttachShader(program, shaders[i]);
	}

	glLinkProgram(program);
    orqa_get_program_status(program);
	for (int i = 0; i < shader_num; i++)
		glDeleteShader(shaders[i]);

	return program;
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

GLint orqa_init_glfw(const int major_version, const int minor_version){
    // glfw: we first initialize GLFW with glfwInit, after which we can configure GLFW using glfwWindowHint
    if(!glfwInit()){
        fprintf(stderr, "In file: %s, line: %d Failed to initialize GLFW\n", __FILE__, __LINE__);
        glfwTerminate();
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major_version); 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor_version); 
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // creating contex profile
    
    return 1;
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
    GLuint textures[number_of_textures];
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

void orqa_bind_VAOs(GLuint VAO[]){
    glBindVertexArray(VAO);
}

void orqa_draw_elements(GLenum type, GLsizei count){
    glDrawElements(type, count, GL_UNSIGNED_INT, 0);
}