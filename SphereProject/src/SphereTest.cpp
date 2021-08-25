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
    Sphere s1(0.7, 2, 3);
    s1.generate();


    // "Generating" obj file that one can import into blender
    
    std::cout << "# vertices: " << s1.vertices.size()/3 << "\n";
    std::cout << "# indicies: " << s1.indices.size() << "\n";
    std::cout << "v ";
    for(int i = 1; i <= s1.vertices.size(); i++) {
        std::cout<< s1.vertices[i-1] << " ";
        if(i % 3 == 0 && i!=s1.vertices.size()) std::cout << std::endl << "v ";
    }
    std::cout << "\nf ";
    
    for(int i = 1; i <= s1.indices.size(); i++){
        std::cout<< s1.indices[i-1]+1; // obj starts from 1
        if(i == s1.indices.size()) std::cout << std::endl;
        else if (i % 3 == 0)  std::cout << std::endl << "f ";
        else std::cout<< "/";
    } 
    std::cout << std::endl;
    
    return 0;
}