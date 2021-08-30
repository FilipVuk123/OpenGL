class ObjLoader():
    def __init__(self, fileName):
        self.Vs = []
        self.faces = []
        self.Ts = []
        self.vertices = []
        self.texCoords = []
        self.returnV = []
        self.returnV1 = []
        help = []
        try:
            f = open(fileName, "r")
            for line in f:
                if line[:2] == "v ":
                    index1 = line.find(" ") + 1
                    index2 = line.find(" ", index1 + 1)
                    index3 = line.find(" ", index2 + 1)

                    vertex1 = float(line[index1:index2])
                    vertex2 = float(line[index2:index3])
                    vertex3 = float(line[index3:-1])
                    
                    self.Vs.append(tuple((vertex1, vertex2, vertex3)))
                    self.returnV1.append(vertex1)
                    self.returnV1.append(vertex2)
                    self.returnV1.append(vertex3)
                    
                elif line[:3] == "vt ":
                    index1 = line.find(" ") + 1
                    index2 = line.find(" ", index1 + 1)
                    

                    vertex1 = float(line[index1:index2])
                    vertex2 = float(line[index2:-1])
                    
                    self.Ts.append(tuple((vertex1, vertex2)))
                    
                elif line[:2] == "f ":
                    string = line.replace("//", "/")
                    i = string.find(" ") + 1
                    for item in range(string.count(" ")):
                        self.faces.append(int((string[i:string.find(" ", i)]).split("/")[0])-1)
                        help.append(tuple((int((string[i:string.find(" ", i)]).split("/")[0])-1, int((string[i:string.find(" ", i)]).split("/")[1])-1)))
                        i = string.find(" ", i) + 1
            f.close()
            h = []
            
            for i1, i2 in help:
                self.vertices.append(self.Vs[i1])
                self.vertices.append(self.Ts[i2])

            
            for _tuple in self.vertices:
                for elem in _tuple:
                    self.returnV.append(elem)

            print(len(self.vertices)/5)
            print(len(self.Ts))
            
        except IOError:
            print(".obj file not found.")

    def writeToFiles(self):
        file = open("../data/Vs_Is.txt", "w")
        file.write("// with texCoords\n")
        file.write("const GLfloat vertices[] = {")
        for elem in range(len(self.returnV)):
            file.write(str(self.returnV[elem]))
            if elem != len(self.returnV)-1: file.write(", ")
        file.write("};\n")
        
        file.write("const GLuint indices[] = {")
        for elem in range(len(self.faces)):
            file.write(str(self.faces[elem]))
            if elem != len(self.faces)-1: file.write(", ")
        file.write("};\n")

        file.write("// without texCoords\n")
        file.write("const GLfloat vertices[] = {")
        for elem in range(len(self.returnV1)):
            file.write(str(self.returnV1[elem]))
            if elem != len(self.returnV1)-1: file.write(", ")
        file.write("};\n")

        file.close()


if __name__ == "__main__":
    parser = ObjLoader("../data/TheIcoSphereobj.obj")
    # parser = ObjLoader("../data/testSphere.obj")
    parser.writeToFiles()
    