#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include <vector>
#include <cmath>
class Sphere{
    public:
        std::vector<GLfloat> vertices;
        std::vector<GLint> indices;
    private:
        float radius;
        unsigned int stacks, sectors;
    public:
    Sphere(float radius, unsigned int stacks, unsigned int sectors): radius(radius), stacks(stacks), sectors(sectors){};
    void generate(){
        for (unsigned int i = 0; i <= stacks; ++i){

            float V   = i / (float) stacks;
            float phi = V * M_PI;

            // Loop Through Sectors
            for (unsigned int j = 0; j <= sectors; ++j){

                float U = j / (float) sectors;
                float theta = U * (M_PI * 2);

                // Calc The Vertex Positions
                float x = cosf (theta) * sinf (phi);
                float y = cosf (phi);
                float z = sinf (theta) * sinf (phi);

                // Push Back Vertex Data
                vertices.push_back (x * radius);
                vertices.push_back (y * radius);
                vertices.push_back (z * radius);
                
            }
        }
        // Calc The Index Positions
        for (unsigned int i = 0; i < sectors * stacks + sectors; ++i){

            indices.push_back(i);
            indices.push_back(i + sectors + 1);
            indices.push_back(i + sectors);

            indices.push_back(i + sectors + 1);
            indices.push_back(i);
            indices.push_back(i + 1);
        }
    }
};
int main(){
    std:: cout << "Hello" << std::endl;
    Sphere s1(1, 2, 3);
    s1.generate();
    long unsigned int a, b;
    a = s1.vertices.size();
    b = s1.indices.size();
    
    std:: cout << "# calcuateed vertices = "<< a << std::endl;
    for(long unsigned int i=0; i < a; i++)
        std::cout << s1.vertices.at(i) << ' ';
    std::cout << std::endl;

    std:: cout << "# calculated indices = "<< b << std::endl;
    for(long unsigned int i=0; i < b; i++)
        std::cout << s1.indices.at(i) << ' ';
    std::cout << std::endl;
    
    return 0;
}