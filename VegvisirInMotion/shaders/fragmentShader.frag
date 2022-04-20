#version 330 core
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D texture1;
void main(){
   float r = texture2D(texture1, TexCoord).x;
   float g = texture2D(texture1, TexCoord).y; // uvijek 0
   float b = texture2D(texture1, TexCoord).z;
   float a = texture2D(texture1, TexCoord).w; // uvijek 1
   FragColor = vec4(r, g,b, 1.0);
};