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

int main(int argc, char* argv[])
{
     
    initialize(argc, argv);
    glutMainLoop();
    
    //delete renderer;
    delete engine;
    
    return 0;

}

std::vector<mob::float4> result;
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
    
    boost::thread dist([&](){
        
        for(;;){
            host.launch("mobtest", "integrate_forces");
            //host.wait("mobtest", "integrate_forces");

            result = host.capture_float4("mobtest", "p"); 
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

void render(){
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

    //if(mouseLock){
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
    //}
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
        //mouseLock = !mouseLock;
    }
    glutPostRedisplay();

}