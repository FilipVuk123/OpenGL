#version 330 core
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D textureY;
uniform sampler2D textureU;
uniform sampler2D textureV;

void main()
{
    vec3 rgb;
    vec3 yuv;
    yuv.x=texture2D(textureY, TexCoord).r;
    yuv.y=texture2D(textureU, TexCoord).r - 0.5;
    yuv.z=texture2D(textureV, TexCoord).r - 0.5;
    
    rgb = mat3( 1,           1,        1,
                0,   -0.337633, 1.732446,
                1.370705,   -0.698001, 0)*yuv;
    FragColor = vec4(rgb, 1.0);
}