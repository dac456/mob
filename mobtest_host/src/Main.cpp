#include "Host.h"

#include <Canis/Canis.h>
#include <unistd.h>

void initialize(int argc, char* argv[]);
void initWindow(int argc, char* argv[]);
void resize(int width, int height);
void render(void);
void idle(void);

void mouse(int x, int y);
void keyboard(unsigned char key, int x, int y);

Canis::Engine* engine;
Canis::Renderer* renderer;

Canis::ScenePtr scene;
Canis::CameraPtr cam;
Canis::MeshPtr mesh;
Canis::LightPtr light;

glm::mat4 projMatrix;

mob::host host;

bool mouseLock = false;

int main(int argc, char* argv[])
{
     
    initialize(argc, argv);
    glutMainLoop();
    
    //delete renderer;
    delete engine;
    
    return 0;

}

std::vector<mob::float4> result;
std::vector<mob::float4> out;
void initialize(int argc, char* argv[]){
    initWindow(argc, argv);
    
    glewExperimental = GL_TRUE;
    glewInit();
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);    
    
    engine = new Canis::Engine();
    renderer = new Canis::Renderer();
    
    scene = std::make_shared<Canis::Scene>("untitled", glm::mat4(1.0f));
    cam = std::make_shared<Canis::Camera>("default", glm::vec3(0.0, 1.8, 0.0), glm::vec3(0.0, 1.8, -5.0));
    renderer->setScene(scene);
    
    Canis::SceneNodePtr root = std::make_shared<Canis::SceneNode>("Root");
    scene->addSceneNode(root);
    light = std::make_shared<Canis::Light>("Light0", glm::vec3(1.0f, 1.0f, 1.0f), 10000.0f, glm::translate(glm::vec3(0.0f, 200.0f, 0.0f)));
    root->attachLight(light);    
    
    mesh = std::make_shared<Canis::Mesh>("rock", new Canis::AssimpLoader(fs::path("./Media/Models/rock.dae")));
    if(mesh){
        Canis::MeshManager::getSingleton().addMesh(mesh);
    }
    
    engine->setRenderer(renderer);
    
    bool initialized = false;
    
    boost::thread dist([&](){
        
        float avg = 0.0f;
        size_t ns = 0;
        
        for(;;){
     
            std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
            
            host.launch("mobtest", "integrate_forces");
            host.wait("mobtest", "integrate_forces", -1);
            
            host.launch("mobtest", "project_particles");
            host.wait("mobtest", "project_particles", -1);
            
            host.launch("mobtest", "correct_system");
            host.wait("mobtest", "correct_system", -1);                        

            result = host.capture_float4("mobtest", "p", -1); 
            
            
            out.resize(200);
            
            if(!initialized){
                for(size_t i=0; i<200; i++){
                    out[i] = mob::float4(-1.0f,-1.0f,-1.0f,-1.0f);
                    
                    size_t nearest[4] = {-1, -1, -1, -1};
                    float nd[4] = {std::numeric_limits<float>::max(),std::numeric_limits<float>::max(),std::numeric_limits<float>::max(),std::numeric_limits<float>::max()};
                    for(size_t j=0; j<200; j++){
                        mob::float4 delta = result[i] - result[j];
                        float d = delta.length();
                        
                        if(d < nd[0]){
                            nearest[0] = j;
                            nd[0] = d;
                        }
                        else if(d < nd[1]){
                            nearest[1] = j;
                            nd[1] = d;
                        }
                        else if(d < nd[2]){
                            nearest[2] = j;
                            nd[2] = d;
                        }
                        else if(d < nd[3]){
                            nearest[3] = j;
                            nd[3] = d;
                        }                                                
                    }
                    
                    out[i] = mob::float4(float(nearest[0]),float(nearest[1]),float(nearest[2]),float(nearest[3]));
                }
                initialized = true;
                host.set_float4("mobtest", "l", out);
            }
            else{
                for(size_t i=0; i<200; i++){
                    size_t nearest[4] = {size_t(out[i][0]), size_t(out[i][1]), size_t(out[i][2]), size_t(out[i][3])};
                    
                    for(size_t j=0; j<4; j++){
                        if(nearest[j] != -1){
                            mob::float4 delta1 = result[nearest[j]] - result[i];
                            float d1 = delta1.length();
                            
                            size_t nearest_j[4] = {size_t(out[nearest[j]][0]),size_t(out[nearest[j]][1]),size_t(out[nearest[j]][2]),size_t(out[nearest[j]][3])};
                            float o[4] = {-1.0f,-1.0f,-1.0f,-1.0f};
                            for(size_t k=0; k<4; k++){
                                mob::float4 delta2 = result[nearest_j[k]] - result[i];
                                float d2 = delta2.length();
                                
                                if(d2 < d1){
                                    o[k] = float(nearest_j[k]);
                                }
                            }
                            if(o[0] != -1.0f){
                                out[i].x = o[0];
                            }
                            if(o[1] != -1.0f){
                                out[i].x = o[1];
                            }
                            if(o[2] != -1.0f){
                                out[i].x = o[2];
                            }
                            if(o[3] != -1.0f){
                                out[i].x = o[3];
                            }                                                                                    
                          
                        }
                    }
                }
                
                host.set_float4("mobtest", "l", out);
            }
            
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();  
            
            ns++;
            avg += float(millis);
            std::cout << "Avg Time: " << avg/float(ns) << std::endl;
            std::cout << "Total Misses: " << total_misses << std::endl;
        }
        
        
    });    
}

void initWindow(int argc, char* argv[]){
    glutInit(&argc, argv);
    glutInitContextVersion(3,3);
    glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
    glutInitContextProfile(GLUT_CORE_PROFILE);

    glutInitWindowPosition(200, 32);
    glutInitWindowSize(1024,768);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    projMatrix = glm::perspective(glm::radians(45.0f), 1024.0f/768.0f, 0.1f, 10000.0f);

    int wHandle = glutCreateWindow("-");

    if(wHandle < 1){
        fprintf(stderr, "Could not create rendering window.");
        exit(EXIT_FAILURE);
    }

    //projMatrix = glm::perspective(45.0f, (float)(800/600), 0.9f, 10000.0f);
    //fprintf(stdout, "%i\n", sizeof(char));

    glutReshapeFunc(resize);
    glutDisplayFunc(render);
    glutIdleFunc(idle);
    glutPassiveMotionFunc(mouse);
    glutKeyboardFunc(keyboard);    
}

void resize(int width, int height){
    glViewport(0,0,width,height);
    projMatrix = glm::perspective(glm::radians(45.0f), (float)width/(float)height, 0.1f, 10000.0f);
}

int frame, currentTime, lastTime = 0;
void render(){
    frame++;
    currentTime=glutGet(GLUT_ELAPSED_TIME);
    if (currentTime - lastTime > 1000) {
        char s[20];
        sprintf(s,"FPS:%4.2f",
            frame*1000.0/(currentTime-lastTime));
        glutSetWindowTitle(s);
        lastTime = currentTime;
        frame = 0;
    }    
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.5f, 0.5f, 0.5, 1.0f);    
    
    if(mesh && result.size()){
        for(auto v : result){
            scene->drawMesh(mesh, glm::translate(glm::vec3(v.x, v.y, v.z)));
        }
    }
    
    renderer->getActiveScene()->render(cam.get(), projMatrix);        
    
    glutSwapBuffers();
    glutPostRedisplay();
}

void idle(){
    glutPostRedisplay();
}

void mouse(int x, int y){
    //POINT mousePos;
    //GetCursorPos(&mousePos);

    if(mouseLock){
        int mX = glutGet(GLUT_WINDOW_X)+(glutGet(GLUT_WINDOW_WIDTH))/2;
        int mY = glutGet(GLUT_WINDOW_Y)+(glutGet(GLUT_WINDOW_HEIGHT))/2;

        if((x == mX) && (y == mY))
            return;

        float dX = ((float)mX - (float)x)/100.0f;
        float dY = ((float)mY - (float)y)/100.0f;

        glm::vec3 angles;
        angles.x += dX;
        angles.y += dY;

        if(angles.x < -M_PI)
          angles.x += M_PI * 2;
        else if(angles.x > M_PI)
          angles.x -= M_PI * 2;

        if(angles.y < -M_PI / 2)
          angles.y = -M_PI / 2;
        if(angles.y > M_PI / 2)
          angles.y = M_PI / 2;

        cam->rotate(glm::radians(angles.y*4.0f), glm::radians(angles.x*4.0f), 0.0f);

        //SetCursorPos(mX, mY);
        glutWarpPointer(mX, mY);
    }
}

void keyboard(unsigned char key, int x, int y){
    if(key == 119){
        cam->move(glm::vec3(-1.5f, -1.5f, -1.5f));
    }
    if(key == 115){
        cam->move(glm::vec3(1.5f, 1.5f, 1.5f));
    }
    if(key == 100){
        cam->strafe(-1.5f);
    }
    if(key == 97){
        cam->strafe(1.5f);
    }
    if(key == 27){
        exit(0);
    }
    if(key == 109){
        mouseLock = !mouseLock;
    }
    glutPostRedisplay();

}