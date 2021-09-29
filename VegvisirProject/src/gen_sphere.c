#include "gen_sphere.h"

void orqa_gen_sphere(orqa_sphere_t *s){
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
    free(textures1); free(textures2);

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
}

void orqa_sphere_free(orqa_sphere_t *sph){
    // deallocate stuff
    free(sph->Vs); free(sph->Is);
}