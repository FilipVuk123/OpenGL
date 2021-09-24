#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// sphere attributes
const float radius = 1.0f;
const int sectors = 25; 
const int stacks = 25;
int numVertices, numTriangles;
float *Vs;
int *Is;

void ORQA_GenSphere(const float radius, const int numLatitudeLines, const int numLongitudeLines);

int main(){

    ORQA_GenSphere(radius, sectors, stacks);
    const unsigned int verticesSize = numVertices*5;
    const unsigned int indicesSize = numTriangles*3;
    float vertices[verticesSize];
    int indices[indicesSize];
    for(unsigned int i = 0; i < verticesSize; i++) vertices[i] = *(Vs + i);
    for(unsigned int i = 0; i < indicesSize; i++) indices[i] = *(Is + i);

    FILE *f1= fopen("vertices.txt", "w");
    if(f1) for(int j=0;j<verticesSize;j++) fprintf(f1,"%f, ",vertices[j]);
    fclose(f1);
    
    FILE *f2 = fopen("indices.txt", "w");
    if(f2) for(int j=0;j<indicesSize;j++) fprintf(f1,"%d, ",indices[j]);
    fclose(f2);
}



void ORQA_GenSphere(const float radius, const int numLatitudeLines, const int numLongitudeLines){
    // One vertex at every latitude-longitude intersection, plus one for the north pole and one for the south.
    numVertices = numLatitudeLines * (numLongitudeLines + 1) + 2; 
    float *verticesX = calloc(numVertices, sizeof(float));
    float *verticesY = calloc(numVertices, sizeof(float));
    float *verticesZ = calloc(numVertices, sizeof(float));
    float *textures1 = calloc(numVertices, sizeof(float));
    float *textures2 = calloc(numVertices, sizeof(float));

    // poles
    *(verticesX) = 0; *(verticesY) = radius; *(verticesZ) = 0; *(textures1) = 0; *(textures2) = 1;
    *(verticesX+numVertices-1)=0; *(verticesY+numVertices-1)=-radius; *(verticesZ+numVertices-1)=0; *(textures1+numVertices-1)=0; *(textures2+numVertices-1)=0;

    int k = 1;
    const float latitudeSpacing = 1.0f / (numLatitudeLines + 1.0f);
    const float longitudeSpacing = 1.0f / (numLongitudeLines);
    // vertices
    for(int latitude = 0; latitude < numLatitudeLines; latitude++) {
        for(int longitude = 0; longitude <= numLongitudeLines; longitude++){
            *(textures1 + k) = longitude * longitudeSpacing; 
            *(textures2 + k) = 1.0f - (latitude + 1) * latitudeSpacing;
            const float theta = (float)(*(textures1 + k) * 2.0f * M_PI);
            const float phi = (float)((*(textures2 + k) - 0.5f) * M_PI);
            const float c = (float)cos(phi);
            *(verticesX + k) = c * cos(theta) * radius; 
            *(verticesY + k) = sin(phi) * radius; 
            *(verticesZ + k) = c * sin(theta) * radius;
            k++;
        }
    }

    Vs = calloc(numVertices*5, sizeof(float));
    int j = 0;
    for(int i = 0; i < 5*numVertices;){
        *(Vs + i++) = *(verticesX+j);
        *(Vs + i++) = *(verticesY+j);
        *(Vs + i++) = *(verticesZ+j);
        *(Vs + i++) = *(textures1+j);
        *(Vs + i++) = *(textures2+j);
        j++;
    }
    free(verticesX); free(verticesY); free(verticesZ); 
    free(textures1); free(textures2);
    
    // indices
    numTriangles = numLatitudeLines * numLongitudeLines * 2;
    Is = calloc((numTriangles)*3, sizeof(int));
    j = 0;
    // pole one indices
    for (int i = 0; i < numLongitudeLines; i++){
        *(Is + j++) = 0;
        *(Is + j++) = i + 2;
        *(Is + j++) = i + 1;
    }
    // no pole indices
    int rowLength = numLongitudeLines + 1;
    for (int latitude = 0; latitude < numLatitudeLines - 1; latitude++){
        int rowStart = (latitude * rowLength) + 1;
        for (int longitude = 0; longitude < numLongitudeLines; longitude++){
            int firstCorner = rowStart + longitude;
            // First triangle of quad: Top-Left, Bottom-Left, Bottom-Right
            *(Is + j++) = firstCorner;
            *(Is + j++) = firstCorner + rowLength + 1;
            *(Is + j++) = firstCorner + rowLength;
            // Second triangle of quad: Top-Left, Bottom-Right, Top-Right
            *(Is + j++) = firstCorner;
            *(Is + j++) = firstCorner + 1;
            *(Is + j++) = firstCorner + rowLength + 1;
        }        
    }
    // pole two indices
    int pole = numVertices-1;
    int bottomRow = ((numLatitudeLines - 1) * rowLength) + 1;
    for (int i = 0; i < numLongitudeLines; i++){
        *(Is + j++) = pole;
        *(Is + j++) = bottomRow + i;
        *(Is + j++) = bottomRow + i + 1;
    }
}
