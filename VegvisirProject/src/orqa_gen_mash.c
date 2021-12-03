#include "orqa_gen_mash.h"


static float radians(const float angle){
    return angle*M_PI /180;
}

static void orqa_gen_sphere(orqa_sphere_t *s){
    unsigned int numLatitudeLines = s->stacks; unsigned int numLongitudeLines = s->sectors;
    s->numVertices = numLatitudeLines * (numLongitudeLines + 1) + 2; // 2 poles
    float *verticesX = calloc(s->numVertices, sizeof(float));
    float *verticesY = calloc(s->numVertices, sizeof(float));
    float *verticesZ = calloc(s->numVertices, sizeof(float));
    float *textures1 = calloc(s->numVertices, sizeof(float));
    float *textures2 = calloc(s->numVertices, sizeof(float));

    // poles
    *(verticesX) = 0; *(verticesY) = s->radius; *(verticesZ) = 0; *(textures1) = 0; *(textures2) = 1;
    *(verticesX+s->numVertices-1)=0; *(verticesY+s->numVertices-1)=-s->radius; *(verticesZ+s->numVertices-1)=0; *(textures1+s->numVertices-1)=0; *(textures2+s->numVertices-1)=0;

    unsigned int k = 1;
    // calculate spacint in between longitude and latitute lines
    const float latitudeSpacing = 1.0f / (numLatitudeLines + 1.0f);
    const float longitudeSpacing = 1.0f / (numLongitudeLines);
    // vertices and textures
    for(unsigned int latitude = 0; latitude < numLatitudeLines; latitude++) {
        for(unsigned int longitude = 0; longitude <= numLongitudeLines; longitude++){
            *(textures1 + k) = longitude * longitudeSpacing; 
            *(textures2 + k) = 1.0f - (latitude + 1) * latitudeSpacing;
            const float theta = (float)(*(textures1 + k) * 2.0f * M_PI);
            const float phi = (float)((*(textures2 + k) - 0.5f) * M_PI);
            const float c = (float)cos(phi);
            *(verticesX + k) = c * cos(theta) * s->radius; 
            *(verticesY + k) = sin(phi) * s->radius; 
            *(verticesZ + k) = c * sin(theta) * s->radius;
            k++;
        }
    }

    s->Vs = calloc(s->numVertices*5, sizeof(float));
    unsigned int j = 0;
    for(int i = 0; i < 5*s->numVertices;){
        *(s->Vs + i++) = *(verticesX+j);
        *(s->Vs + i++) = *(verticesY+j);
        *(s->Vs + i++) = *(verticesZ+j);
        *(s->Vs + i++) = *(textures1+j);
        *(s->Vs + i++) = *(textures2+j);
        j++;
    }
    free(verticesX); 
    free(verticesY); 
    free(verticesZ); 
    free(textures1); 
    free(textures2);

    // indices
    s->numTriangles = numLatitudeLines * numLongitudeLines * 2;
    s->Is = calloc((s->numTriangles)*3, sizeof(int));
    j = 0;
    // pole one indices
    for (unsigned int i = 0; i < numLongitudeLines; i++){
        *(s->Is + j++) = 0;
        *(s->Is + j++) = i + 2;
        *(s->Is + j++) = i + 1;
    }
    // no pole indices
    unsigned int rowLength = numLongitudeLines + 1;
    for (unsigned int latitude = 0; latitude < numLatitudeLines - 1; latitude++){
        unsigned int rowStart = (latitude * rowLength) + 1;
        for (unsigned int longitude = 0; longitude < numLongitudeLines; longitude++){
            unsigned int firstCorner = rowStart + longitude;
            // First triangle of quad: Top-Left, Bottom-Left, Bottom-Right
            *(s->Is + j++) = firstCorner;
            *(s->Is + j++) = firstCorner + rowLength + 1;
            *(s->Is + j++) = firstCorner + rowLength;
            // Second triangle of quad: Top-Left, Bottom-Right, Top-Right
            *(s->Is + j++) = firstCorner;
            *(s->Is + j++) = firstCorner + 1;
            *(s->Is + j++) = firstCorner + rowLength + 1;
        }        
    }
    // pole two indices
    unsigned int pole = s->numVertices-1;
    unsigned int bottomRow = ((numLatitudeLines - 1) * rowLength) + 1;
    for (unsigned int i = 0; i < numLongitudeLines; i++){
        *(s->Is + j++) = pole;
        *(s->Is + j++) = bottomRow + i;
        *(s->Is + j++) = bottomRow + i + 1;
    }
    s->numTriangles *=3;
    s->numVertices *=5;
}

static void orqa_gen_window(orqa_window_t *win){
    const float radius = win->radius;
    float x = win->x;
    float y = win->y;
    float z = win->z;
    win->stacks = floor(win->angleY/10);
    win->sectors = floor(win->angleX/10);


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
    win->numVertices = 5*(win->stacks+1)*(win->sectors+1);
    win->numTriangles = 6*win->sectors*win->stacks;
    free(verticesX); 
    free(verticesY); 
    free(verticesZ); 
    free(textures1); 
    free(textures2);
}

void orqa_sphere_free(orqa_sphere_t *sph){
    free(sph->Vs); free(sph->Is);
}

void orqa_window_free(orqa_window_t *win){
    free(win->Vs); free(win->Is);
}

orqa_window_t orqa_create_window(const float radius, float angleX, float angleY, float x, float y, float z){
    orqa_window_t window;
    window.radius = radius; window.angleY = angleX; window.angleX = angleY; window.x = x; window.y = y; window.z = z;
    orqa_gen_window(&window);
    return window;
}

orqa_sphere_t orqa_create_sphere(const float radius, float sectors, float stacks){
    orqa_sphere_t sphere;
    sphere.radius = radius; sphere.sectors = sectors; sphere.stacks = stacks;
    orqa_gen_sphere(&sphere);
    return sphere;
}
