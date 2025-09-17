#ifndef OBJLOADER_H
#define OBJLOADER_H

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <gl/gl.h>

class ObjLoader {
public:
    std::vector<float> vertices;
    std::vector<int> faces;

    void load(const std::string& filename) {
        std::ifstream in(filename);
        if (!in) { std::cerr << "Failed to open OBJ file\n"; return; }

        std::string line;
        while (std::getline(in, line)) {
            std::istringstream iss(line);
            std::string type;
            iss >> type;
            if (type == "v") {
                float x, y, z;
                iss >> x >> y >> z;
                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);
            }
            else if (type == "f") {
                int a, b, c;
                iss >> a >> b >> c;
                // OBJ indexing starts at 1
                faces.push_back(a - 1);
                faces.push_back(b - 1);
                faces.push_back(c - 1);
            }
        }
    }

    void draw() const {
        glBegin(GL_TRIANGLES);
        for (size_t i = 0; i < faces.size(); ++i) {
            int idx = faces[i] * 3;
            glVertex3f(vertices[idx], vertices[idx + 1], vertices[idx + 2]);
        }
        glEnd();
    }
};

#endif

