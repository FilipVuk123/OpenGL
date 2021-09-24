#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
float *Vs;
int *Is;

int numVertices;
int numTriangles;
void ORQA_GenCupola(const float radius, const unsigned int numLatitudeLines, const unsigned int numLongitudeLines){
    
    numVertices = numLatitudeLines * (numLongitudeLines + 1) + 1; 
    float *verticesX = calloc(numVertices, sizeof(float));
    float *verticesY = calloc(numVertices, sizeof(float));
    float *verticesZ = calloc(numVertices, sizeof(float));
    float *textures1 = calloc(numVertices, sizeof(float));
    float *textures2 = calloc(numVertices, sizeof(float));

    unsigned int k = 0;
    const float latitudeSpacing = 1.0f / (float)(numLatitudeLines + 1);
    const float longitudeSpacing = 1.0f / (float)(numLongitudeLines);
    const float new_latitudeSpacing = 1.0f / (float)(numLatitudeLines - 2);
    for(unsigned int latitude = 0; latitude < numLatitudeLines; latitude++) {
        for(unsigned int longitude = 0; longitude <= numLongitudeLines; longitude++){
            const float theta = (float)(longitude * longitudeSpacing * 2.0f * M_PI);
            const float phi = (float)(((1.0f - (latitude + 1) * latitudeSpacing) - 0.5f) * M_PI);
            const float c = (float)cos(phi);
            if(latitude > 0){
                *(verticesX + k) = c * cos(theta) * radius; 
                *(verticesY + k) = sin(phi) * radius; 
                *(verticesZ + k) = c * sin(theta) * radius;
                *(textures1 + k) = (longitude) * longitudeSpacing; 
                *(textures2 + k) = (latitude-1) * new_latitudeSpacing;
                k++;
            }            
        }
    }
    
    for (int i = 0; i < numVertices; i++){
        printf("u: %f, v:%f\n", *(textures1+i), *(textures2+i));
    }

    Vs = calloc(numVertices*5, sizeof(float));
    unsigned int j = 0;
    for(unsigned int i = 0; i < 5*numVertices;){
        *(Vs + i++) = *(verticesX+j);
        *(Vs + i++) = *(verticesY+j);
        *(Vs + i++) = *(verticesZ+j);
        *(Vs + i++) = *(textures1+j);
        *(Vs + i++) = *(textures2+j);
        j++;
    }
    
    numTriangles = numLatitudeLines * numLongitudeLines * 2;
    Is = calloc((numTriangles)*3, sizeof(int));
    j = 0;
    /*
    for (unsigned int i = 0; i < numLongitudeLines; i++){
        *(Is + j++) = 0;
        *(Is + j++) = i + 2;
        *(Is + j++) = i + 1;
    }
    */
    unsigned int rowLength = numLongitudeLines + 1;
    for (unsigned int latitude = 0; latitude < numLatitudeLines - 1; latitude++){
        unsigned int rowStart = (latitude * rowLength);
        for (unsigned int longitude = 0; longitude < numLongitudeLines; longitude++){
            unsigned int firstCorner = rowStart + longitude;
    
            *(Is + j++) = firstCorner;
            *(Is + j++) = firstCorner + rowLength + 1;
            *(Is + j++) = firstCorner + rowLength;
    
            *(Is + j++) = firstCorner;
            *(Is + j++) = firstCorner + 1;
            *(Is + j++) = firstCorner + rowLength + 1;
        }        
    }
    /*
    unsigned int pole = numVertices-1;
    unsigned int bottomRow = ((numLatitudeLines - 1) * rowLength) + 1;
    for (unsigned int i = 0; i < numLongitudeLines; i++){
        *(Is + j++) = pole;
        *(Is + j++) = bottomRow + i;
        *(Is + j++) = bottomRow + i + 1;
    }*/
}

int main(){
    float radius = 1.0f;
    int lat = 30;
    int lon = 30;
    ORQA_GenCupola(radius, lat, lon);

    float vertices[numVertices*5];
    int indices[numTriangles*3];
    
    for(unsigned int i = 0; i < numVertices*5; i++) vertices[i] = *(Vs + i);
    for(unsigned int i = 0; i < numTriangles*3; i++) indices[i] = *(Is + i);
    printf("GLfloat vertices[]={\n");
    for(unsigned int i = 0; i < numVertices*5*4/5+1; i++) {
        printf(" %f,",vertices[i]);
        // if(i%5==4) printf("\nv");
    }
    printf("};\n");
    printf("GLuint indices[]={\n");
    for(unsigned int i = 0; i < numTriangles*3; i++) {
        printf(" %d,",indices[i]);
        // if(i%3==2) printf("\nf");
        if(indices[i] == 194) break;
    }
    printf("};\n");
    
    return 0;
}