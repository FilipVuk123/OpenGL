#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
int CLAMP(float a)
{
    if (a < 0)
        return 0;
    else if (a > 255)
        return 255;
    else
        return a;
}
static void yuv420p_to_rgb3(uint8_t *yuv_buffer, const int width, const int height, uint8_t *rgb_out)
{
    uint8_t *y = yuv_buffer;
    uint8_t *u = yuv_buffer + (width * height);
    uint8_t *v = yuv_buffer + (width * height) + (width * height / 4);
    // uint8_t *rgb = calloc((width * height * 3), sizeof(uint8_t));
    float b, g, r;
    uint8_t *ptr = rgb_out;
    for (int j = 0; j < height; j++)
    {
        for (int i = 0; i < width; i++)
        {
            int yy = y[(j * width) + i];
            int uu = u[((j / 2) * (width / 2)) + (i / 2)];
            int vv = v[((j / 2) * (width / 2)) + (i / 2)];
            r = 1.164 * (yy - 16) + 1.596 * (vv - 128);
            g = 1.164 * (yy - 16) - 0.813 * (vv - 128) - 0.391 * (uu - 128);
            b = 1.164 * (yy - 16) + 2.018 * (uu - 128);
            *ptr++ = CLAMP(r);
            *ptr++ = CLAMP(g);
            *ptr++ = CLAMP(b);
        }
    }
}
int main()
{
    const int w = 1280;
    const int h = 720;
    const int yuv_size = w * h * 3 / 2;
    FILE *input_file = fopen("out720.yuv", "rb");
    uint8_t *yuv_buffer = malloc(yuv_size);
    fread(yuv_buffer, yuv_size, 1, input_file);
    fclose(input_file);
    uint8_t *rgb = malloc(w*h*3);
    yuv420p_to_rgb3(yuv_buffer, w, h, rgb);
    FILE *output_file = fopen("out720.rgb", "wb");
    fwrite(rgb, w * h * 3, 1, output_file);
    fclose(output_file);
    free(yuv_buffer);
    free(rgb);
    return 0;
}