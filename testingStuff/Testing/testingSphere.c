#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define ORQA_IN
#define ORQA_REF
#define ORQA_OUT
#define ORQA_NOARGS

const double radius = 0.7f;
const unsigned int longitude = 5; 
const unsigned int latitude = 5; 

double *Vs;
unsigned int *Is;
double *Ns;
double *Ts;

int numTriangles;
int numVertices;
unsigned int k;
/*
void ORQA_GenSphere(ORQA_IN GLfloat radius, ORQA_IN GLuint longitude,ORQA_IN GLuint stacks){
    GLfloat *verticesX = calloc(longitude*stacks, sizeof(GLfloat));
    GLfloat *verticesY = calloc(longitude*stacks, sizeof(GLfloat));
    GLfloat *verticesZ = calloc(longitude*stacks, sizeof(GLfloat));
    for (GLuint i = 0; i < stacks; i++){
        GLfloat V   = i / (GLfloat) stacks;
        GLfloat phi = V * M_PI;

        for (GLuint j = 0; j < longitude; j++){
            GLfloat U = j / (GLfloat) longitude;
            GLfloat theta = U * (M_PI * 2);

            GLfloat x = cosf (theta) * sinf (phi);
            GLfloat y = cosf (phi);
            GLfloat z = sinf (theta) * sinf (phi);

            *(verticesX + stacks*i + j) = (x * radius);
            *(verticesY + stacks*i + j) = (y * radius);
            *(verticesZ + stacks*i + j) = (z * radius);
        }
    }
    Vs = calloc(longitude*stacks*3, sizeof(GLfloat));
    GLuint j = 0;
    for(GLuint i = 0; i < 3*longitude*stacks; i=i+3){
        *(Vs + i) = *(verticesX+j);
        *(Vs + i+1) = *(verticesY+j);
        *(Vs + i+2) = *(verticesZ+j);
        j++;
    }
    free(verticesX);
    free(verticesY);
    free(verticesZ);
    

    Is = calloc((longitude * stacks + longitude)*6, sizeof(GLint));
    j = 0;
    for (GLuint i = 0; i < longitude * stacks + longitude; ++i){
        *(Is + j++) = i;
        *(Is + j++) = i + longitude + 1;
        *(Is + j++) = i + longitude;
        
        *(Is + j++) = i + longitude + 1;
        *(Is + j++) = i;
        *(Is + j++) = i + 1;
    }
}*/
/*
void ORQA_GenSphere(const float radius, const unsigned int stacks, const unsigned int longitude){
    GLfloat *verticesX = calloc((longitude+1)*stacks, sizeof(GLfloat));
    GLfloat *verticesY = calloc((longitude+1)*stacks, sizeof(GLfloat));
    GLfloat *verticesZ = calloc((longitude+1)*stacks, sizeof(GLfloat));
    GLfloat drho = M_PI / (GLfloat)stacks;
    GLfloat dtheta = 2*M_PI / (GLfloat)longitude;

    for (GLuint i = 0; i < stacks; i++){
        const GLfloat rho = (GLfloat)i * drho;
        const GLfloat srhodrho = (GLfloat)(sinf(rho + drho));
        const GLfloat crhodrho = (GLfloat)(cosf(rho + drho));

        for (GLuint j = 0; j <= longitude; j++){
            const GLfloat theta = (j == longitude) ? 0.0f : j * dtheta;
            const GLfloat stheta = (GLfloat)(-sinf(theta));
            const GLfloat ctheta = (GLfloat)(cosf(theta));

            GLfloat x = stheta * srhodrho;
            GLfloat y = ctheta * srhodrho;
            GLfloat z = crhodrho;

            *(verticesX + stacks*i + j) = x * radius;
            *(verticesY + stacks*i + j) = y * radius;
            *(verticesZ + stacks*i + j) = z * radius;
        }
    }
    Vs = calloc(longitude*stacks*3, sizeof(GLfloat));
    GLuint j = 0;
    for(GLuint i = 0; i < 3*longitude*stacks; i=i+3){
        *(Vs + i) = *(verticesX+j);
        *(Vs + i+1) = *(verticesY+j);
        *(Vs + i+2) = *(verticesZ+j);
        j++;
    }
    free(verticesX);
    free(verticesY);
    free(verticesZ);
    Is = calloc((longitude * stacks + longitude)*6, sizeof(GLint));
    j = 0;
    for (GLuint i = 0; i < longitude * stacks + longitude; ++i){
        *(Is + j++) = i;
        *(Is + j++) = i + longitude + 1;
        *(Is + j++) = i + longitude;
        
        *(Is + j++) = i + longitude + 1;
        *(Is + j++) = i;
        *(Is + j++) = i + 1;
    }
}*/
/*void ORQA_GenSphere(ORQA_IN float radius, ORQA_IN unsigned int latitude, ORQA_IN unsigned int longitude){
    double const R = 1./(double)(latitude-1);
    double const S = 1./(double)(longitude-1);
    double *verticesX = calloc(longitude*latitude, sizeof(double));
    double *verticesY = calloc(longitude*latitude, sizeof(double));
    double *verticesZ = calloc(longitude*latitude, sizeof(double));
    for (unsigned int r = 0; r < latitude; r++){
        for (unsigned int s = 0; s < longitude; s++){
            double x = cos(2*M_PI * s * S) * sin( M_PI * r * R );
            *(verticesX + latitude*r + s) = x * radius;
            double y = sin( -M_PI_2 + M_PI * r * R );
            *(verticesY + latitude*r + s) = y * radius;
            double z = sin(2*M_PI * s * S) * sin( M_PI * r * R );
            *(verticesZ + latitude*r + s) = z * radius;
        }
    }
    
    Vs = calloc(longitude*latitude*3, sizeof(double));
    unsigned int j = 0;
    for(unsigned int i = 0; i < 3*longitude*latitude; i=i+3){
        *(Vs + i) = *(verticesX+j);
        *(Vs + i+1) = *(verticesY+j);
        *(Vs + i+2) = *(verticesZ+j);
        j++;
    }
    free(verticesX);
    free(verticesY);
    free(verticesZ);
    
    
    Is = calloc((longitude * latitude)*4, sizeof(int));
    j = 0;
    for(unsigned int r = 0; r < latitude; r++) for(unsigned int s = 0; s < longitude; s++) {
        *(Is + j++) = r * longitude + s;
        *(Is + j++) = r * longitude + (s+1);
        *(Is + j++) = (r+1) * longitude + (s+1);
        *(Is + j++) = (r+1) * longitude + s;
        
    }
};*/
/*void ORQA_GenSphere(ORQA_IN float radius, ORQA_IN unsigned int latitude, ORQA_IN unsigned int longitude){
    double angleLat = M_PI/latitude;
    double angleLong = M_PI*2/longitude;


    // vertices, poles and textures
    numVertices = (latitude + 1)*(longitude - 1) + 2;
    Vs = calloc(numVertices*3, sizeof(double));
    Ns = calloc(numVertices*3, sizeof(double));
    Ts = calloc(numVertices*2, sizeof(double));
    
    
    // first pole
    *(Ns) = 0; *(Ns+ 1) = 0, *(Ns + 2) = 1;
    *(Vs) = 0; *(Vs + 1) = 0; *(Vs + 2) = radius; 
    *(Ts) = 0.5f; *(Ts + 1) = 1.0f;

    // sphere body
    unsigned int k = 1;
    for (unsigned int i = 1; i < latitude; i++) for (unsigned int j = 0; j <= longitude; j++){
        *(Ns+ 3*k) = sin(angleLat * i) * cos(angleLong * j);
        *(Ns+ 3*k+1) = sin(angleLat * i) * sin(angleLong * j);
        *(Ns+ 3*k+2) = cos(angleLat * i);
        *(Vs + 3*k) = radius * *(Ns+ 3*k);
        *(Vs + 3*k+1) = radius * *(Ns+ 3*k+1);
        *(Vs + 3*k+2) = radius * *(Ns+ 3*k+2);
        *(Ts + 2*k) = (double) j / longitude;
        *(Ts + 2*k + 1) = (double) i / latitude;
        k++;
    }
    // second pole
    *(Ns + 3*k) = 0; *(Ns+ 3*k + 1) = 0, *(Ns+ 3*k + 2) = 1;
    *(Vs + 3*k ) = 0; *(Vs + 3*k + 1) = 0; *(Vs +3*k + 2) = -radius; 
    *(Ts + 3*k) = 0.5f; *(Ts + 3*k + 1) = 1.0f;

    // indices
    numTriangles = (latitude - 1) * longitude * 6;
    Is = calloc(numTriangles, sizeof(int));
    k = 0;

    // for first pole
    for (int j = 1; j<= longitude; j++){
        *(Is + k++) = 0;
        *(Is + k++) = j; 
        *(Is + k++) = j+1;
    }
    
    // for second pole
    int temp = numVertices  - 1;
    for (int j  = temp-1; j > temp - longitude - 1; j--){
    *(Is + k++) = temp;
    *(Is + k++) = j; 
    *(Is + k++) = j - 1;
    }
    for (unsigned int i = 1; i < latitude - 1; i++) {
        for (unsigned int j = 1; j <= longitude; j++) {
            // triangle one
            *(Is + k++) = (i - 1) * longitude + j;
            *(Is + k++) = i * longitude + j;
            *(Is + k++) = i * longitude + j + 1;
            // triangle two
            *(Is + k++) = (i - 1) * longitude + j;
            *(Is + k++) = i * longitude + j + 1;
            *(Is + k++) = (i - 1) * longitude + j + 1;
        }
    }
}*/
/*
void ORQA_GenSphere(const float radius, const unsigned int latitude, const unsigned int longitude){
    GLfloat *verticesX = calloc((longitude+1)*latitude, sizeof(GLfloat));
    GLfloat *verticesY = calloc((longitude+1)*latitude, sizeof(GLfloat));
    GLfloat *verticesZ = calloc((longitude+1)*latitude, sizeof(GLfloat));
    GLfloat drho = M_PI / (GLfloat)latitude;
    GLfloat dtheta = 2*M_PI / (GLfloat)longitude;

    for (GLuint i = 0; i < latitude; i++){
        const GLfloat rho = (GLfloat)i * drho;
        const GLfloat srhodrho = (GLfloat)(sinf(rho + drho));
        const GLfloat crhodrho = (GLfloat)(cosf(rho + drho));

        for (GLuint j = 0; j <= longitude; j++){
            const GLfloat theta = (j == longitude) ? 0.0f : j * dtheta;
            const GLfloat stheta = (GLfloat)(-sinf(theta));
            const GLfloat ctheta = (GLfloat)(cosf(theta));

            GLfloat x = stheta * srhodrho;
            GLfloat y = ctheta * srhodrho;
            GLfloat z = crhodrho;

            *(verticesX + latitude*i + j) = x * radius;
            *(verticesY + latitude*i + j) = y * radius;
            *(verticesZ + latitude*i + j) = z * radius;
        }
    }
    Vs = calloc(longitude*latitude*3, sizeof(GLfloat));
    GLuint j = 0;
    for(GLuint i = 0; i < 3*longitude*latitude; i=i+3){
        *(Vs + i) = *(verticesX+j);
        *(Vs + i+1) = *(verticesY+j);
        *(Vs + i+2) = *(verticesZ+j);
        j++;
    }
    free(verticesX);
    free(verticesY);
    free(verticesZ);
    Is = calloc((longitude * latitude + longitude)*6, sizeof(GLint));
    j = 0;
    for (GLuint i = 0; i < longitude * latitude + longitude; ++i){
        *(Is + j++) = i;
        *(Is + j++) = i + longitude + 1;
        *(Is + j++) = i + longitude;
        
        *(Is + j++) = i + longitude + 1;
        *(Is + j++) = i;
        *(Is + j++) = i + 1;
    }
}
*/
/*
void ORQA_GenSphere(const float radius, const unsigned int latitude, const unsigned int longitude){
    numVertices = latitude*(longitude+1)+2;
    
    Vs = calloc(numVertices*3, sizeof(GLfloat));
    printf("numVertices: %d\n", numVertices*3);
    *(Vs) = 0; *(Vs + 1) = radius; *(Vs + 2) = 0; 
    *(Vs+ numVertices*3-2) = 0; *(Vs + numVertices*3 -1) = -radius; *(Vs + numVertices*3) = 0; 

    double *verticesX = calloc(numVertices, sizeof(double));
    double *verticesY = calloc(numVertices, sizeof(double));
    double *verticesZ = calloc(numVertices, sizeof(double));
    
    GLfloat latitudeSpacing = 1.0f / (latitude + 1.0f);
    GLfloat longitudeSpacing = 1.0f / (longitude);

    for(unsigned int i = 0; i < latitude; i++){
        for(unsigned int j = 0; j <longitude + 1; j++){
            GLfloat theta = j*longitudeSpacing* 2.0f * M_PI;
            GLfloat phi = (1.0f - (i + 1) * latitudeSpacing  - 0.5f) * M_PI;
            GLfloat c = cos(phi);
            
            *(verticesX + i*latitude + j) = radius * c * cos(theta);
            *(verticesY + i*latitude + j) = radius * sin(phi);
            *(verticesZ + i*latitude + j) = radius * c * sin(theta);   
            
            printf("%d ", i*j);
        }
    } 
    Vs = calloc(longitude*latitude*3, sizeof(double));
    unsigned int j = 0;
    for(unsigned int i = 0; i < 3*longitude*latitude; i=i+3){
        *(Vs + i) = *(verticesX+j);
        *(Vs + i+1) = *(verticesY+j);
        *(Vs + i+2) = *(verticesZ+j);
        j++;
    }
    free(verticesX);
    free(verticesY);
    free(verticesZ);

    printf("\nk= %d\n", k);
    k = 0;
    
    numTriangles = latitude * longitude * 2;
    Is = calloc(numTriangles*3, sizeof(GLuint));
    printf("numTriangles: %d\n", numTriangles*3);
    for (int j = 0; j<= longitude; j++){
        *(Is + k++) = 0;
        *(Is + k++) = j+1; 
        *(Is + k++) = j+2;
    }

    int rowLength = longitude +1;
    for (int i = 0; i < latitude - 1; i++) {
        int rowStart = i * rowLength + 1;
        for (int j = 0; j < longitude; j++) {        
            int firstCorner = rowStart + j;

            // First triangle of quad: Top-Left, Bottom-Left, Bottom-Right
            *(Is + k++) = firstCorner;
            *(Is + k++) = firstCorner + rowLength + 1;
            *(Is + k++) = firstCorner + rowLength;

            // Second triangle of quad: Top-Left, Bottom-Right, Top-Right
            *(Is + k++) = firstCorner;
            *(Is + k++) = firstCorner + 1;
            *(Is + k++) = firstCorner + rowLength + 1;
        }
    }
    int pole = numVertices - 1;
    int bottomRow = (latitude - 1) * rowLength + 1;

    for (int i = 0; i < longitude; i++) {
        *(Is + k++) = pole;
        *(Is + k++) = bottomRow + i;
        *(Is + k++) = bottomRow + i + 1;
    }
    printf("k= %d\n", k);
}*/

//The best
void ORQA_GenSphere(const float radius, const unsigned int stacks, const unsigned int sectors){
    GLfloat *verticesX = calloc((sectors+1)*stacks, sizeof(GLfloat));
    GLfloat *verticesY = calloc((sectors+1)*stacks, sizeof(GLfloat));
    GLfloat *verticesZ = calloc((sectors+1)*stacks, sizeof(GLfloat));
    GLfloat *textures1 = calloc((sectors+1)*stacks, sizeof(GLfloat));
    GLfloat *textures2 = calloc((sectors+1)*stacks, sizeof(GLfloat));
    
    GLfloat drho = M_PI / (GLfloat)stacks;
    GLfloat dtheta = 2*M_PI / (GLfloat)sectors;

    for (GLuint i = 0; i < stacks; i++){
        const GLfloat rho = (GLfloat)i * drho;
        const GLfloat srhodrho = (GLfloat)(sinf(rho + drho));
        const GLfloat crhodrho = (GLfloat)(cosf(rho + drho));

        for (GLuint j = 0; j <= sectors; j++){
            const GLfloat theta = (j == sectors) ? 0.0f : j * dtheta;
            const GLfloat stheta = (GLfloat)(-sinf(theta));
            const GLfloat ctheta = (GLfloat)(cosf(theta));

            GLfloat x = stheta * srhodrho;
            GLfloat y = ctheta * srhodrho;
            GLfloat z = crhodrho;

            *(verticesX + stacks*i + j) = x * radius;
            *(verticesY + stacks*i + j) = y * radius;
            *(verticesZ + stacks*i + j) = z * radius;
            *(textures1 + stacks*i + j) = (GLfloat) j / sectors;
            *(textures2 + stacks*i + j) = (GLfloat) i / stacks;
        }
    }
    numVertices = sectors*stacks*5;
    Vs = calloc(sectors*stacks*5, sizeof(GLfloat));
    GLuint j = 0;
    for(GLuint i = 0; i < 5*sectors*stacks; ){
        *(Vs + i++) = *(verticesX+j);
        *(Vs + i++) = *(verticesY+j);
        *(Vs + i++) = *(verticesZ+j);
        *(Vs + i++) = *(textures1+j);
        *(Vs + i++) = *(textures2+j);
        j++;
    }

    free(verticesX);
    free(verticesY);
    free(verticesZ);
    numTriangles =     (sectors * stacks + sectors)*6;
    Is = calloc((sectors * stacks + sectors)*6, sizeof(GLint));
    j = 0;
    for (GLuint i = 0; i < sectors * stacks + sectors; ++i){
        *(Is + j++) = i;
        *(Is + j++) = i + sectors + 1;
        *(Is + j++) = i + sectors;
        
        *(Is + j++) = i + sectors + 1;
        *(Is + j++) = i;
        *(Is + j++) = i + 1;
    }
}

/*
void ORQA_GenSphere(ORQA_IN float radius, ORQA_IN unsigned int latitude, ORQA_IN unsigned int longitude){
    double angleLat = M_PI/latitude;
    double angleLong = M_PI*2/longitude;


    // vertices, poles and textures
    numVertices = (latitude)*(longitude - 1) + 2;
    Vs = calloc(numVertices*3, sizeof(double));
    Ns = calloc(numVertices*3, sizeof(double));
    Ts = calloc(numVertices*2, sizeof(double));
    
    
    // first pole
    *(Ns) = 0; *(Ns+ 1) = 0, *(Ns + 2) = 1;
    *(Vs) = 0; *(Vs + 1) = 0; *(Vs + 2) = radius; 
    *(Ts) = 0.5f; *(Ts + 1) = 1.0f;

    // sphere body
    unsigned int k = 1;
    for (unsigned int i = 1; i < latitude; i++) for (unsigned int j = 0; j <= longitude; j++){
        *(Ns+ 3*k) = sin(angleLat * i) * cos(angleLong * j);
        *(Ns+ 3*k+1) = sin(angleLat * i) * sin(angleLong * j);
        *(Ns+ 3*k+2) = cos(angleLat * i);
        *(Vs + 3*k) = radius * *(Ns+ 3*k);
        *(Vs + 3*k+1) = radius * *(Ns+ 3*k+1);
        *(Vs + 3*k+2) = radius * *(Ns+ 3*k+2);
        *(Ts + 2*k) = (double) j / longitude;
        *(Ts + 2*k + 1) = (double) i / latitude;
        k++;
    }
    // second pole
    *(Ns + 3*k) = 0; *(Ns+ 3*k + 1) = 0, *(Ns+ 3*k + 2) = 1;
    *(Vs + 3*k ) = 0; *(Vs + 3*k + 1) = 0; *(Vs +3*k + 2) = -radius; 
    *(Ts + 3*k) = 0.5f; *(Ts + 3*k + 1) = 1.0f;

    // indices
    k = 0;
    
    numTriangles = (latitude-1) * longitude * 2;
    Is = calloc(numTriangles*3, sizeof(GLuint));
    printf("numTriangles: %d\n", numTriangles*3);
    for (int j = 0; j<= longitude; j++){
        *(Is + k++) = 0;
        *(Is + k++) = j+1; 
        *(Is + k++) = j+2;
    }

    int rowLength = longitude +1;
    for (int i = 0; i < latitude - 1; i++) {
        int rowStart = i * rowLength + 1;
        for (int j = 0; j < longitude; j++) {        
            int firstCorner = rowStart + j;

            // First triangle of quad: Top-Left, Bottom-Left, Bottom-Right
            *(Is + k++) = firstCorner;
            *(Is + k++) = firstCorner + rowLength + 1;
            *(Is + k++) = firstCorner + rowLength;

            // Second triangle of quad: Top-Left, Bottom-Right, Top-Right
            *(Is + k++) = firstCorner;
            *(Is + k++) = firstCorner + 1;
            *(Is + k++) = firstCorner + rowLength + 1;
        }
    }
    int pole = numVertices - 1;
    int bottomRow = (latitude - 1) * rowLength + 1;

    for (int i = 0; i < longitude; i++) {
        *(Is + k++) = pole;
        *(Is + k++) = bottomRow + i;
        *(Is + k++) = bottomRow + i + 1;
    }
}*/

int main(){
    ORQA_GenSphere(radius, latitude, longitude);
    /*
    printf("v ");
    for (unsigned int i = 1; i <= numVertices*3; i++ ){
        printf("%lf ", *(Vs+i-1) );
        if(i % 3 == 0) printf("\nv ");
    }
    printf("\nf");
    
    for (unsigned int i = 1; i <= numTriangles*3; i++){
        printf(" %d ",*(Is+i-1)+1);
        if(i %3 == 0) printf("\nf"); else printf("/");
    }
    */
    free(Vs);
    free(Ns);
    free(Ts);
    free(Is);
    
    printf("\n");
    return 0;
}