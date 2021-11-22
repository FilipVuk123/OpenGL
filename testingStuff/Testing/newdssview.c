#include <math.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct window_t{
    float radius, x, y, z, angleX, angleY; 
    int numVertices, numTriangles; 
    float *Vs;
    int *Is;
    float stacks, sectors;
}window_t;

static float radians(float a){
    return a*M_PI/180;
}

static void orqa_gen_window(window_t *win){
    const float radius = win->radius;
    float x = win->x;
    float y = win->y;
    float z = win->z;
    win->stacks = floor(win->angleY/10);
    win->sectors = floor(win->angleX/10)-1;


    float *verticesX = calloc((win->stacks+1)*(win->sectors+1), sizeof(float));
    float *verticesY = calloc((win->stacks+1)*(win->sectors+1), sizeof(float));
    float *verticesZ = calloc((win->stacks+1)*(win->sectors+1), sizeof(float));
    float *textures1 = calloc((win->stacks+1)*(win->sectors+1), sizeof(float));
    float *textures2 = calloc((win->stacks+1)*(win->sectors+1), sizeof(float));

    float norm = sqrt(x*x + y*y + z*z);
    x = x / norm; y = y / norm; z = z / norm;

    const float latitudeAngle = asin(y / radius) + radians(win->angleY)/2;
    float longitudeAngle = asin(x / (radius * cos(latitudeAngle))) - radians(win->angleX)/2;
    if (isnan(longitudeAngle)) longitudeAngle = asin(z / (radius * sin(latitudeAngle))) - radians(win->angleX)/2;

    const float latitudeSpacing = 1.0f / win->stacks;
    const float longitudeSpacing =  1.0f / win->sectors;
    
    const float betaSpacing = win->angleX/win->sectors;
    const float alphaSpacing = win->angleY/win->stacks;

    unsigned int k = 0;
    float alpha = latitudeAngle;
    for(unsigned int latitude = 0; latitude <= win->stacks; latitude++){ 
        float beta = longitudeAngle;
        for(unsigned int longitude = 0; longitude <= win->sectors; longitude++){
            *(textures1 + k) = longitude * longitudeSpacing; 
            *(textures2 + k) = 1.0f - latitude * latitudeSpacing;
            const float tmp = cos(alpha);
            *(verticesX + k) = radius * tmp * sin(beta); 
            *(verticesY + k) = radius * sin(alpha);
            *(verticesZ + k) = radius * tmp * cos(beta);
            k++;
            beta+=radians(betaSpacing);
        }
        alpha -= radians(alphaSpacing);
    }
    win->Vs = calloc(5*(win->stacks+1)*(win->sectors+1), sizeof(float));
    unsigned int j = 0;
    for(int i = 0; i < 5*(win->stacks+1)*(win->sectors+1);){
        *(win->Vs + i++) = *(verticesX+j);
        *(win->Vs + i++) = *(verticesY+j);
        *(win->Vs + i++) = *(verticesZ+j);
        *(win->Vs + i++) = *(textures1+j);
        *(win->Vs + i++) = *(textures2+j);
        j++;
    }

    win->Is = calloc(2*win->sectors*win->stacks*3, sizeof(int));
    j = 0;
    unsigned int rowLength = win->sectors + 1;
    for (unsigned int latitude = 0; latitude < win->stacks; latitude++){
        unsigned int rowStart = (latitude * rowLength);
        for (unsigned int longitude = 0; longitude < win->sectors; longitude++){
            unsigned int firstCorner = rowStart + longitude;
            // First triangle of quad: Top-Left, Bottom-Left, Bottom-Right
            *(win->Is + j++) = firstCorner;
            *(win->Is + j++) = firstCorner + rowLength + 1;
            *(win->Is + j++) = firstCorner + rowLength;
            // Second triangle of quad: Top-Left, Bottom-Right, Top-Right
            *(win->Is + j++) = firstCorner;
            *(win->Is + j++) = firstCorner + 1;
            *(win->Is + j++) = firstCorner + rowLength + 1;
        }        
    }

    free(verticesX); 
    free(verticesY); 
    free(verticesZ); 
    free(textures1); 
    free(textures2);
}
int main(){

    window_t MyWin;
    MyWin.radius = 1.0f;
    MyWin.x = -0.55f;
    MyWin.y = -0.4f;
    MyWin.z = -0.5f;
    MyWin.angleX = 100;
    MyWin.angleY = 50;

    orqa_gen_window(&MyWin);
    printf("v ");
    for(int i = 1; i <= 5*(MyWin.stacks+1)*(MyWin.sectors+1); i++){
        printf("%f ", *(MyWin.Vs+i-1));
        if (i % 5 == 0){printf("\nv ");}
    }
    printf("\n");
    /*
    for(int i = 1; i <= 2*3*MyWin.stacks*MyWin.sectors; i++){
        printf("%d ", *(MyWin.Is+i-1));
        if(i % 6 == 0) printf("\n");
    }*/
    printf("\n");

    return 0;
}