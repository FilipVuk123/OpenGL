#version 330 core
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D texture1;
void main(){
   float r = texture2D(texture1, TexCoord).x;
   float g = texture2D(texture1, TexCoord).y;
   float b = texture2D(texture1, TexCoord).z;

   FragColor = vec4(r, g,b, 1.0);
};