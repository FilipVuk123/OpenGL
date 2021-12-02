#version 330 core
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D texture1;
void main()
{
   vec3 color = texture2D(texture1, TexCoord).xyz;
   gl_FragColor = vec4(color , 1.0);
};