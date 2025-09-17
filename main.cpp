#include <windows.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <cstdlib>
#include <ctime>
#include <vector>
#include "ObjLoader.h"
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>

#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 1600
#define MAX_PARTICLES 130000
#define GROUND_PLATE -0.05f

bool simulationStarted = false;
bool Sat_Rocket = true;
bool MoveCameraMode = false;
float camYaw = 0.0f;
float camPitch = 10.0f;
float camRoll = 0.0f;
float camDistance = 2.5f;
float rocketY = 0.0f;
float rocketVY = 0.0f;
float rocketAY = 0.0005f; //acceleration

float radius = 0.05f;
float height = 0.75f;


void ProcessKeyPress(WPARAM key) {
    const float del_angle = 2.0f;
    const float del_zoom = 0.2f;

    switch (key) {
    case 'S': //Start simulation
        simulationStarted = true;
        break;
    case VK_UP: //Rotate up
        camPitch += del_angle;
        if (camPitch > 89.0f) { 
            camPitch = 89.0f; 
        }
        break;
    case VK_DOWN: //Rotate down
        camPitch -= del_angle;
        if (camPitch < -89.0f) { 
            camPitch = -89.0f; 
        }
        break;
    case VK_LEFT: //Rotate left
        camYaw -= del_angle;
        break;
    case VK_RIGHT: //Rotate right
        camYaw += del_angle;
        break;
    case 'I': //Zoom in
        camDistance -= del_zoom;
        if (camDistance < 1.0f) { 
            camDistance = 1.0f; 
        }
        break;
    case 'O': //Zoom out
        camDistance += del_zoom;
        break;
    case 'L': //Roll left
        camRoll -= del_angle;
        break;
    case 'R': //Roll right
        camRoll += del_angle;
        break;
    case 'G': //Ground Camera Mode
        MoveCameraMode = false;
        break;
    case 'M': //Moving Camera Mode
        MoveCameraMode = true;
        break;
    case 'V': //Saturn V Rocket
        Sat_Rocket = true;
        break;
    case 'N': //Normal Rocket
        Sat_Rocket = false;
        break;
    }
}

struct Particle {
    float x, y, z;
    float vx, vy, vz;
    float r, g, b;
    float life;
};

std::vector<Particle> particles;

void InitParticles() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    particles.clear();
    for (int i = 0; i < MAX_PARTICLES; ++i) {
        Particle p;

        if (Sat_Rocket) {
            static const float nozzleOffsets[] = { 0.00f, 0.04f, -0.04f };
            const int n = sizeof(nozzleOffsets) / sizeof(nozzleOffsets[0]);
            p.x = nozzleOffsets[rand() % n];      
            p.z = nozzleOffsets[rand() % n];
            p.y = 0.0f;                               
        }
        else {
            auto randUnit = []() -> float { return (rand() % 100 - 50) / 50.0f; };
            p.x = 0.6f * radius * randUnit();     
            p.z = 0.6f * radius * randUnit();
            p.y = 0.0f;

        }

        auto randSigned = [](float div) -> float { return (rand() % 100 - 50) / div; };

        p.vx = randSigned(100000.0f);
        p.vy = -(rand() % 100) / 2500.0f;
        p.vz = randSigned(100000.0f);
        p.r = 1.0f;
        p.g = 1.0f;     
        p.b = 0.0f;
        p.life = (rand() % 100) / 100.0f + 0.5f;
        particles.push_back(p);
    }
    glDisable(GL_BLEND);
}

void UpdateParticles(float dt, float rocketY) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    for (auto& p : particles) {
        p.x += p.vx;
        p.y += p.vy;
        p.z += p.vz;
        p.g -= dt / 10;
        p.life -= dt;

        // Check if below ground
        if (p.y < GROUND_PLATE) {
            p.y = GROUND_PLATE;
            p.vy = ((rand() % 100) - 50) / 8000.0f;
            p.vx = ((rand() % 100) - 50) / (p.vy * 8000000.0f);
            p.vz = ((rand() % 100) - 50) / (p.vy * 8000000.0f);
            p.life -= 0.25f;
        }

        if (p.life <= 0.0f) {
            // Respawn particle

            if (Sat_Rocket) {
                float numbers[] = { 0, 0.04, -0.04 };
                int size = sizeof(numbers) / sizeof(numbers[0]);
                int randInd1 = rand() % size;
                int randInd2 = rand() % size;
                p.x = numbers[randInd1];
                p.y = 0.0f;
                p.z = numbers[randInd2];
            }
            else {
                p.x = 0.6f * radius * (rand() % 100 - 50) / 50.0f;
                p.z = 0.6f * radius * (rand() % 100 - 50) / 50.0f;
            }
            p.g = 0.2f;
            p.y = rocketY;
            auto randSigned = [](float div) -> float { return (rand() % 100 - 50) / div; };

            p.vx = randSigned(100000.0f);
            p.vy = -(rand() % 100) / 2500.0f;
            p.vz = randSigned(100000.0f);
            p.life = (rand() % 100) / 100.0f + 0.5f;
        }
    }
    glDisable(GL_BLEND);
}

void DrawParticles() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glPointSize(15.0f);
    glBegin(GL_POINTS);
    for (const auto& p : particles) {
        glColor4f(p.r, p.g, p.b, 0.05f);
        glVertex3f(p.x, p.y, p.z);
    }
    glEnd();
    glDisable(GL_BLEND);
}

void DrawSkyCylinder() {
    const float sky_radius = 50.0f;
    const float sky_height = 80.0f;
    const int seg = 100;

    glPushMatrix();
    glDisable(GL_LIGHTING);
    glDisable(GL_BLEND);
    glDepthMask(GL_FALSE);

    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= seg; i++) {
        float theta = 2.0f * 3.14159f * i / seg;
        float x = sky_radius * cos(theta);
        float z = sky_radius * sin(theta);

        glColor3f(0.05f, 0.4f, 0.8f);
        glVertex3f(x, GROUND_PLATE, z);

        glColor3f(0.0f, 0.0f, 0.05f);
        glVertex3f(x, sky_height, z);
    }
    glEnd();

    glDepthMask(GL_TRUE);
    glPopMatrix();
}

struct OBJModel {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
};

OBJModel LoadOBJ(const std::string& path) {

    OBJModel out;
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        std::cerr << "[LoadOBJ] couldn’t open " << path << '\n';
        return out;                            
    }
    std::string token;
    std::vector<float>  verts;                 
    std::vector<unsigned> idx;                 

    for (std::string line; std::getline(in, line); ) {
        std::istringstream s(line);
        s >> token;
        if (token == "v") {
            float x, y, z;  s >> x >> y >> z;
            verts.insert(verts.end(), { x, y, z });
        }
        else if (token == "f") {
            unsigned v[3];  char junk;
            if (line.find('/') != std::string::npos) {
                s.clear(); 
                s.str(line.substr(2));
                for (int k = 0; k < 3; k++) {
                    s >> v[k];    
                    while (s.peek() == '/') {
                        s >> junk;    
                        while (std::isdigit(s.peek())) s >> junk;
                    }
                }
            }
            else {
                s >> v[0] >> v[1] >> v[2];
            }
            idx.push_back(v[0] - 1);
            idx.push_back(v[1] - 1);
            idx.push_back(v[2] - 1);
        }
    }
    out.vertices.swap(verts);
    out.indices.swap(idx);
    return out;
}

void DrawOBJModel(const OBJModel& model) {
    glBegin(GL_TRIANGLES);
    for (size_t i = 0; i < model.indices.size(); i++) {
        int idx = model.indices[i] * 3;
        glVertex3f(model.vertices[idx], model.vertices[idx + 1], model.vertices[idx + 2]);
    }
    glEnd();
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_KEYDOWN:
        ProcessKeyPress(wParam);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    AllocConsole();
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    printf("===== Rocket Simulation Controls =====\n");
    printf("Press 'S' to start the rocket simulation.\n");
    printf("Camera Controls:\n");
    printf("  Arrow Keys:\n");
    printf("    Up    - Rotate camera up\n");
    printf("    Down  - Rotate camera down\n");
    printf("    Left  - Rotate camera left (yaw)\n");
    printf("    Right - Rotate camera right (yaw)\n");
    printf("  'L' - Roll camera left\n");
    printf("  'R' - Roll camera right\n");
    printf("  'I' - Zoom in\n");
    printf("  'O' - Zoom out\n");
    printf("Camera Modes:\n");
    printf("  'G' - Switch to Ground Camera Mode\n");
    printf("  'M' - Switch to Moving Camera Mode\n");
    printf("Rocket Modes:\n");
    printf("  'V' - Use Saturn V Rocket\n");
    printf("  'N' - Use Normal Rocket\n");
    printf("======================================\n");

    srand((unsigned int)time(0));
    WNDCLASSW wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"RocketPlumeWindow";
    RegisterClassW(&wc);

    HWND hwnd = CreateWindowW(L"RocketPlumeWindow", L"Rocket Plume Simulation",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT,
        nullptr, nullptr, hInstance, nullptr);

    HDC hdc = GetDC(hwnd);
    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.iLayerType = PFD_MAIN_PLANE;

    int pf = ChoosePixelFormat(hdc, &pfd);
    SetPixelFormat(hdc, pf, &pfd);

    HGLRC hglrc = wglCreateContext(hdc);
    wglMakeCurrent(hdc, hglrc);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        MessageBoxA(NULL, "GLEW Init Failed", "Error", MB_OK);
        return -1;
    }

    ShowWindow(hwnd, nCmdShow);

    SetForegroundWindow(hwnd);
    SetFocus(hwnd);

    glEnable(GL_POINT_SMOOTH);


    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    double ratio_per = (double)WINDOW_WIDTH / WINDOW_HEIGHT;
    gluPerspective(45.0, ratio_per, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0, 1.0, 4.0, 0, 0, 0, 0, 1, 0);

    InitParticles();

    // IF NEEDED CHANGE THE DIRECTORY 
    OBJModel rocketModel = LoadOBJ("..\\Saturn V.obj");
    // "C:\\Users\\arnob\\OneDrive\\Desktop\\CS 334\\RocketPlume_Simulation\\Saturn V.obj"


    MSG msg = {};
    DWORD lastTime = GetTickCount64();
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            DWORD currentTime = GetTickCount64();
            float deltaTime = (currentTime - lastTime) / 100.0f;
            lastTime = currentTime;

            if (simulationStarted) {
                UpdateParticles(deltaTime, rocketY);
            }

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glLoadIdentity();

            const float RAD = 3.14159265f / 180.0f;     // degrees -> radians
            float yawRad = camYaw * RAD;
            float pitchRad = camPitch * RAD;
            float sinYaw = sinf(yawRad), cosYaw = cosf(yawRad);
            float sinPitch = sinf(pitchRad), cosPitch = cosf(pitchRad);

            float camX = camDistance * cosPitch * sinYaw;
            float camY = camDistance * sinPitch;
            float camZ = camDistance * cosPitch * cosYaw;

            // Calculate up vector from roll
            float upX = sinf(camRoll * 3.14159f / 180.0f);
            float upY = cosf(camRoll * 3.14159f / 180.0f);
            float upZ = 0.0f;

            if (simulationStarted) {
                rocketVY += rocketAY * deltaTime;
                rocketY += rocketVY * deltaTime;
            }
            if (MoveCameraMode) {
                gluLookAt(camX, camY + rocketY, camZ, 0, rocketY, 0, upX, upY, upZ);
            }
            else {
                gluLookAt(camX, camY, camZ, 0, rocketY, 0, upX, upY, upZ);
            }

            // Draw Ground
            glColor4f(0.2f, 0.4f, 0.2f, 1.0f); // Dark green
            glBegin(GL_QUADS);
            glVertex3f(-10000.0f, GROUND_PLATE, -10000.0f);
            glVertex3f(-10000.0f, GROUND_PLATE, 10000.0f);
            glVertex3f(10000.0f, GROUND_PLATE, 10000.0f);
            glVertex3f(10000.0f, GROUND_PLATE, -10000.0f);

            glEnd();
            glPopMatrix();

            DrawSkyCylinder();

            // Draw rocket body based on the Condition

            if (Sat_Rocket == false) {
                // Normal Rocket
                // Draw four fins

                const float PI = 3.14159265f;
                const float RAD_45 = PI * 0.25f;          // 45
                const float finFootY = GROUND_PLATE + rocketY;
                const float finTipY = rocketY + 0.2f;
                struct Fin {
                    float ang, dx, dz;
                };

                const Fin fins[4] = {
                    {  RAD_45,    +0.10f, +0.10f },
                    { -RAD_45,    +0.10f, -0.10f },
                    {  3 * RAD_45,  -0.10f, +0.10f },
                    { -3 * RAD_45,  -0.10f, -0.10f }
                };

                glColor4f(1.0f, 0.0f, 0.0f, 1.0f);           // red fins

                for (const Fin& f : fins) {
                    float baseX = radius * cosf(f.ang);
                    float baseZ = radius * sinf(f.ang);
                    glBegin(GL_TRIANGLES);
                    glVertex3f(baseX + f.dx, finFootY, baseZ + f.dz);
                    glVertex3f(baseX, rocketY, baseZ);
                    glVertex3f(baseX, finTipY, baseZ);
                    glEnd();

                }

                // Draw cylindrical body + nose cap

                const float RAD = PI / 180.0f;
                const float SLICE = 10.0f;

                glColor4f(0.0f, 0.0f, 0.30f, 1.0f);           // blue body

                for (float aDeg = 0; aDeg < 360.0f; aDeg += SLICE) {
                    float a0 = aDeg * RAD;
                    float a1 = (aDeg + SLICE) * RAD;
                    float x0 = radius * cosf(a0), z0 = radius * sinf(a0);
                    float x1 = radius * cosf(a1), z1 = radius * sinf(a1);

                    /* body strip */
                    glBegin(GL_QUADS);
                    glVertex3f(x0, rocketY, z0);
                    glVertex3f(x0, rocketY + height, z0);
                    glVertex3f(x1, rocketY + height, z1);
                    glVertex3f(x1, rocketY, z1);
                    glEnd();


                    /* nose-cap triangle (keeps original red colour) */
                    glColor4f(1.0f, 0.0f, 0.0f, 1.0f);        // red
                    glBegin(GL_TRIANGLES);
                    glVertex3f(x0, rocketY + height, z0);
                    glVertex3f(x1, rocketY + height, z1);
                    glVertex3f(0.0f, rocketY + height + 0.2f, 0.0f);
                    glEnd();

                    glColor4f(0.0f, 0.0f, 0.30f, 1.0f);
                }
            }
            else {
                // Saturn V Rocket

                glPushMatrix();
                glTranslatef(0.0f, rocketY, 0.0f);
                glRotatef(-90, 1, 0, 0);
                glScalef(0.01f, 0.01f, 0.01f);
                glColor4f(0.6f, 0.6f, 0.6f, 1.0f);

                DrawOBJModel(rocketModel);
                glPopMatrix();
            }


            if (simulationStarted) {
                DrawParticles();
            }

            SwapBuffers(hdc);
        }
    }

    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hglrc);
    ReleaseDC(hwnd, hdc);
    return 0;
}