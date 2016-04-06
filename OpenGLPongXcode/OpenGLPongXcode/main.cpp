#ifdef __APPLE__
#include <GLUT/glut.h>
#include <OpenAL/al.h>
#include <OpenAL/alc.h>

#else

#include <GL/glut.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>

#endif

#include <iostream>
#include <cmath>
#include "imageloader.h"
#include <stdlib.h>
#include <stdio.h>
using namespace std;

string fullPath = __FILE__;
const int TEXTURE_COUNT = 24;
static GLuint texName[TEXTURE_COUNT];

using namespace std;

struct Ball
{
    float x;
    float y;
    float z;
    float rad;
    float r;
    float g;
    float b;
    int stacks;
    int slices;
    float dirx;
    float diry;
    float speed;
};

struct Padel
{
    float x;
    float y;
    float z;
    float sizex;
    float sizey;
    float sizez;
    float r;
    float g;
    float b;
    float speed;
    bool boy;
};

Ball ball;
Padel p1, p2;
bool running = false; //Pause and Unpause
bool game = false; //Start the game
int score1 = 0;
int score2 = 0;
int shrink = 0;
int shrinking = 3;
int gameState = 0; //Game Has not started// Start at right corner
//0 game not started
//1 Right won last
//2 left won last
float orientation = 0;

bool mouseIn = false;

/////AUDIO!
#ifdef __APPLE__ 
#else

// Buffers hold sound data.
ALuint Buffer;
// Sources are points emitting sound.
ALuint Source;
// Position of the source sound.
ALfloat SourcePos[] = { 0.0, 0.0, 0.0 };
// Velocity of the source sound.
ALfloat SourceVel[] = { 0.0, 0.0, 0.0 };
// Position of the listener.
ALfloat ListenerPos[] = { 0.0, 0.0, 0.0 };
// Velocity of the listener.
ALfloat ListenerVel[] = { 0.0, 0.0, 0.0 };
// Orientation of the listener. (first 3 elements are "at", second 3 are "up")
ALfloat ListenerOri[] = { 0.0, 0.0, -1.0, 0.0, 1.0, 0.0 };

ALboolean LoadALData()
{
    // Variables to load into.
    ALenum format;
    ALsizei size;
    ALvoid* data;
    ALsizei freq;
    ALboolean loop;
    signed char *sfile = (signed char*) ("bounce.wav"); //fullPath.c_str() + 
    ALbyte* file = sfile;
    
    // Load wav data into a buffer.
    alGenBuffers(1, &Buffer);
    
    if (alGetError() != AL_NO_ERROR)
        return AL_FALSE;
    
    alutLoadWAVFile(file, &format, &data, &size, &freq, &loop);
    alBufferData(Buffer, format, data, size, freq);
    alutUnloadWAV(format, data, size, freq);
    
    // Bind buffer with a source.
    alGenSources(1, &Source);

    if (alGetError() != AL_NO_ERROR)
        return AL_FALSE;
    
    alSourcei (Source, AL_BUFFER,   Buffer);
    alSourcef (Source, AL_PITCH,    1.0f     );
    alSourcef (Source, AL_GAIN,     1.0f     );
    alSourcefv(Source, AL_POSITION, SourcePos);
    alSourcefv(Source, AL_VELOCITY, SourceVel);
    alSourcei (Source, AL_LOOPING,  AL_FALSE);
    
    // Do another error check and return.
    if (alGetError() == AL_NO_ERROR)
        return AL_TRUE;
    
    return AL_FALSE;
}

void SetListenerValues()
{
    alListenerfv(AL_POSITION,    ListenerPos);
    alListenerfv(AL_VELOCITY,    ListenerVel);
    alListenerfv(AL_ORIENTATION, ListenerOri);
}

void KillALData()
{
    alDeleteBuffers(1, &Buffer);
    alDeleteSources(1, &Source);
    alutExit();
}

#endif

void objInit(){
    //BALL INIT
    ball.x = 0;
    ball.y = 0;
    ball.z = 0;
    ball.rad = 0.25;
    ball.r = 1;
    ball.g = .9;
    ball.b = .9;
    ball.stacks = 20;
    ball.slices = 20;
    ball.dirx = 0;
    ball.diry = 0;
    ball.speed = 0.1;
    
    //P1 INIT
    p1.x = -3.75;
    p1.y = 0;
    p1.z = 0;
    p1.sizex = 0.5;
    p1.sizey = 2;
    p1.sizez = 1;
    p1.r = 0;
    p1.g = 0;
    p1.b = 0;
    p1.speed = 0;
    p1.boy = true;
    
    //P2 INIT
    p2.x = 3.75;
    p2.y = 0;
    p2.z = 0;
    p2.sizex = 0.5;
    p2.sizey = 2;
    p2.sizez = 1;
    p2.r = 0;
    p2.g = 0;
    p2.b = 0;
    p2.speed = 0;
    p2.boy = false;
    
    
}


//Makes the image into a texture, and returns the id of the texture
void loadTexture(Image* image,int k)
{
    
    glBindTexture(GL_TEXTURE_2D, texName[k]); //Tell OpenGL which texture to edit
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    //Filtros de ampliacion y redución con cálculo mas cercano no es tan bueno pero es rápido
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    
    //Filtros de ampliacion y redución con cálculo lineal es mejo pero son más calculos
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    
    //Map the image to the texture
    glTexImage2D(GL_TEXTURE_2D,                //Always GL_TEXTURE_2D
                 0,                            //0 for now
                 GL_RGB,                       //Format OpenGL uses for image
                 image->width, image->height,  //Width and height
                 0,                            //The border of the image
                 GL_RGB, //GL_RGB, because pixels are stored in RGB format
                 GL_UNSIGNED_BYTE, //GL_UNSIGNED_BYTE, because pixels are stored
                 //as unsigned numbers
                 image->pixels);               //The actual pixel data
}


//le borramos el exceso para solo obtener el Path padre
void getParentPath()
{
    for (int i = (int)fullPath.length()-1; i>=0 && fullPath[i] != '/'; i--) {
        fullPath.erase(i,1);
    }
}


void initRendering()
{
    //Declaración del objeto Image
    Image* image;
    GLuint i=0;
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glGenTextures(TEXTURE_COUNT, texName); //Make room for our texture
    getParentPath();
    
    char  ruta[200];
    sprintf(ruta,"%s%s", fullPath.c_str() , "Texturas/burger.bmp");
    image = loadBMP(ruta);loadTexture(image,i++);
    
    sprintf(ruta,"%s%s", fullPath.c_str() , "Texturas/escuela.bmp");
    image = loadBMP(ruta);loadTexture(image,i++);
    
    sprintf(ruta,"%s%s", fullPath.c_str() , "Texturas/Fondo.bmp");
    image = loadBMP(ruta);loadTexture(image,i++);
    
    sprintf(ruta,"%s%s", fullPath.c_str() , "Texturas/ganador.bmp");
    image = loadBMP(ruta);loadTexture(image,i++);
    
    sprintf(ruta,"%s%s", fullPath.c_str() , "Texturas/i1-1.bmp");
    image = loadBMP(ruta);loadTexture(image,i++);
    
    sprintf(ruta,"%s%s", fullPath.c_str() , "Texturas/i3-1.bmp");
    image = loadBMP(ruta);loadTexture(image,i++);
    
    sprintf(ruta,"%s%s", fullPath.c_str() , "Texturas/i4-1.bmp");
    image = loadBMP(ruta);loadTexture(image,i++);
    
    sprintf(ruta,"%s%s", fullPath.c_str() , "Texturas/i5-1.bmp");
    image = loadBMP(ruta);loadTexture(image,i++);
    
    sprintf(ruta,"%s%s", fullPath.c_str() , "Texturas/inicio1.bmp");
    image = loadBMP(ruta);loadTexture(image,i++);
    
    sprintf(ruta,"%s%s", fullPath.c_str() , "Texturas/inicio2.bmp");
    image = loadBMP(ruta);loadTexture(image,i++);
    
    sprintf(ruta,"%s%s", fullPath.c_str() , "Texturas/instrucciones1.bmp");
    image = loadBMP(ruta);loadTexture(image,i++);
    
    sprintf(ruta,"%s%s", fullPath.c_str() , "Texturas/instrucciones2.bmp");
    image = loadBMP(ruta);loadTexture(image,i++);
    
    sprintf(ruta,"%s%s", fullPath.c_str() , "Texturas/materia.bmp");
    image = loadBMP(ruta);loadTexture(image,i++);
    
    sprintf(ruta,"%s%s", fullPath.c_str() , "Texturas/nina.bmp");
    image = loadBMP(ruta);loadTexture(image,i++);
    
    sprintf(ruta,"%s%s", fullPath.c_str() , "Texturas/nino.bmp");
    image = loadBMP(ruta);loadTexture(image,i++);
    
    sprintf(ruta,"%s%s", fullPath.c_str() , "Texturas/nuevojuego1.bmp");
    image = loadBMP(ruta);loadTexture(image,i++);
    
    sprintf(ruta,"%s%s", fullPath.c_str() , "Texturas/nuevojuego2.bmp");
    image = loadBMP(ruta);loadTexture(image,i++);
    
    sprintf(ruta,"%s%s", fullPath.c_str() , "Texturas/objetivo.bmp");
    image = loadBMP(ruta);loadTexture(image,i++);
    
    sprintf(ruta,"%s%s", fullPath.c_str() , "Texturas/pasto.bmp");
    image = loadBMP(ruta);loadTexture(image,i++);
    
    sprintf(ruta,"%s%s", fullPath.c_str() , "Texturas/perdedor.bmp");
    image = loadBMP(ruta);loadTexture(image,i++);
    
    sprintf(ruta,"%s%s", fullPath.c_str() , "Texturas/RaquetaNina.bmp");
    image = loadBMP(ruta);loadTexture(image,i++);
    
    sprintf(ruta,"%s%s", fullPath.c_str() , "Texturas/RaquetaNino.bmp");
    image = loadBMP(ruta);loadTexture(image,i++);
    
    sprintf(ruta,"%s%s", fullPath.c_str() , "Texturas/salir1.bmp");
    image = loadBMP(ruta);loadTexture(image,i++);
    
    sprintf(ruta,"%s%s", fullPath.c_str() , "Texturas/salir2.bmp");
    image = loadBMP(ruta);loadTexture(image,i++);
    
    delete image;
}



void resetGameValues(){

    //Score
    score1 = 0;
    score2 = 0;
    
    //GameState
    gameState = 0;
    
}

struct Rectangle
{
    float x;
    float y;
    float w;
    float h;
};

bool checkCollision(Rectangle rect1, Rectangle rect2){
    return (
        rect1.w + rect2.w > abs(rect1.x - rect2.x) and
        rect1.h + rect2.h > abs(rect1.y - rect2.y)
        );
}

Rectangle getBallRect(Ball b){
    Rectangle r;
    r.x = b.x;
    r.w = b.rad;
    r.y = b.y;
    r.h = b.rad;
    return r;
}

Rectangle getPadelRect(Padel p){
    Rectangle r;
    r.x = p.x;
    r.w = p.sizex/2;
    r.y = p.y;
    r.h = p.sizey/2;
    return r;
}

void playAudioLinux(){
    ///AUDIO
    #ifdef __APPLE__ 
    #else
    alSourceStop(Source);
    alSourcePlay(Source);
    #endif
    
}

void startGame(){
    int side = 1;
    int upDown = 1;
    switch(gameState){
        case 1  :
        side = -1;
        upDown = (1 - 2 * (rand() % 2));
        break;
        case 2  :
        side = 1;
        upDown = (1 - 2 * (rand() % 2));
        break;

        default :
        side = 1;
        upDown = 1;
        break;
    }
    
    double angle = rand() % 45 + 22.5;
    ball.diry = ball.speed * sin ( angle * 3.1416 / 180.0 ) * upDown;
    ball.dirx = ball.speed * cos ( angle * 3.1416 / 180.0 ) * side;
    
}

void checkCollisions(){
    //Vertical borders
    if(abs(ball.y) + ball.rad > 4){
        ball.diry *= -1;
        playAudioLinux();
        shrink = shrinking;
    }
    
    Rectangle rb;
    Rectangle rp1;
    Rectangle rp2;
    rb = getBallRect(ball);
    rp1 = getPadelRect(p1);
    rp2 = getPadelRect(p2);
    
    //Collision with left panel
    if(checkCollision(rb, rp1)){
        (ball.dirx < 0) ? ball.dirx = -ball.dirx : ball.dirx;
        shrink = shrinking;
        
        //AUDIO
        playAudioLinux();
        
        
        //Collision with right panel
    } else if(checkCollision(rb, rp2)){
        (ball.dirx > 0) ? ball.dirx = -ball.dirx : ball.dirx;
        shrink = shrinking;
        
        //AUDIO
        playAudioLinux();
        
        
        //Goal on right side
    } else if(ball.x > 4){
        objInit();
        
        //running = false;
        //game = false;
        score1++;
        gameState = 1;
        
        startGame();
        
        //Goal on left side
    }else if(ball.x < -4){
        objInit();
        
        //running = false;
        //game = false;
        score2++;
        gameState = 2;
        
        startGame();
        
    }
}

void moveBall(){
    ball.x += ball.dirx;
    ball.y += ball.diry;
}

void movePadel(Padel &p){
    if(abs(p.y + p.speed) + p.sizey/2 < 4){
        p.y += p.speed;
    }
}

void update(){
    checkCollisions();
    moveBall();
    movePadel(p1);
    movePadel(p2);
}

void timer(int v) {
    if(running){
        update();
    }
    glutTimerFunc(20, timer, v);
    glutPostRedisplay();

}

void text_paintBoy(){
    //14
    

}
void text_paintGirl(){
    //13
    
}

void paintCube(float x, float y, float z, float sizex, float sizey, float sizez, float r, float g, float b, bool boy, int lineW = 1){
   
    
   
    glPushMatrix();
    
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
    if(boy){
        glColor3f(0.0, 1.0, 1.0);
        glBindTexture(GL_TEXTURE_2D, texName[14]);

    }
    else{
        glColor3f(0.0, 1.0, 0.0);
        glBindTexture(GL_TEXTURE_2D, texName[13]);

    }
    
    glPushMatrix();
    glTranslated(x,y,z);
    glScalef(sizex, sizey, sizez);
    //glColor3f(1.0, 1.0, 1.0);
    glutSolidCube(1);
    //glColor3f(1.0, 1.0, 1.0);
    glPopMatrix();
    
    
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    

    glPopMatrix();
    
    
}
void paintCube2(float x, float y, float z, float sizex, float sizey, float sizez, float r, float g, float b, bool solid, int lineW = 1){
    glPushMatrix();
    glTranslated(x,y,z);
    glScalef(sizex, sizey, sizez);
    glColor3f(r,g,b);
    if(solid){
        glutSolidCube(1);
    } else {
        glLineWidth(lineW);
        glutWireCube(1);
    }
    glPopMatrix();
}

void paintPadel(Padel p){
    
    paintCube(p.x, p.y, p.z, p.sizex, p.sizey, p.sizez, p.r, p.g, p.b, p.boy);
    paintCube2(p.x, p.y, p.z, p.sizex, p.sizey, p.sizez, 255, 0, 0, false, 5);
}

void text_bg_grass(){
    
    glEnable(GL_TEXTURE_2D);
    
    //Elegir la textura del Quads: angulo cambia con el timer
    glBindTexture(GL_TEXTURE_2D, texName[18]);
    
    glBegin(GL_QUADS);
    //Asignar la coordenada de textura 0,0 al vertice
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-4.0f, -4.0f, -1);
    //Asignar la coordenada de textura 1,0 al vertice
    glTexCoord2f(1.0f, 0.0f); ///-
    glVertex3f(4.0f, -4.0f, -1);
    //Asignar la coordenada de textura 1,1 al vertice
    glTexCoord2f(1.0f,1.0f); //-
    //glTexCoord2f(1.0f,1.0f);
    //glTexCoord2f(2.0f,5.0f);
    glVertex3f(4.0f, 4.0f, -1);
    //Asignar la coordenada de textura 0,1 al vertice
    glTexCoord2f(0.0f, 1.0f);
    //glTexCoord2f(0.0f, 5.0f);
    
    glVertex3f(-4.0f, 4.0f, -1);
    glEnd();
    
    
}

void paintTable(){
    
    glColor3f(1.0, 1.0, 1.0);

    text_bg_grass();

    glPushMatrix();
    
    
    glScalef(8,8,1);
    
    glColor3f(1.0, 1.0, 1.0);
    glColor3ub(255,140,0);
    glLineWidth(5);
    glutWireCube(1);
    //glColor3f(1.0, 1.0, 1.0);

    glPopMatrix();
    
    glColor3f(1.0, 1.0, 1.0);

    //LINEAS DE
    glBegin(GL_LINES);
    glVertex2d(0, -4);
    glVertex2d(0, 4);
    glVertex2d(-4, 0);
    glVertex2d(4, 0);
    glVertex2d(4, -4);
    glVertex2d(4, 4);
    glVertex2d(-4, 4);
    glVertex2d(4, 4);
    glVertex2d(-4, -4);
    glVertex2d(-4, 4);
    glVertex2d(-4, -4);
    glVertex2d(4, -4);
    glEnd();

    glColor3b(1.0, 1.0, 1.0);

}

void paintSphere(float x, float y, float z, float rad, int slices, int stacks, float r, float g, float b, bool solid,bool change, int lineW = 1 ){
    
    glPushMatrix();
    glTranslated(x,y,z);
    //glColor3f(r,g,b);
    
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
    
    glColor3f(1.0, 1.0, 1.0);
    glEnable(GL_TEXTURE_2D);

    GLUquadricObj *qobj;
    
    if (!change) {
        //cout<<"notchanged"<<endl;
        glBindTexture(GL_TEXTURE_2D, texName[0]);

    }else{
        glBindTexture(GL_TEXTURE_2D, texName[1]);

    }
    
    glPushMatrix();
    qobj = gluNewQuadric();
    //glTranslatef(x, y, z);
    gluQuadricDrawStyle(qobj, GLU_FILL); /* smooth shaded */
    gluSphere(qobj, rad, slices, stacks);
    glutSolidSphere(rad,slices,stacks);
    glPopMatrix();
    
    
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    
    glPopMatrix();
    
}

void paintSphere2(float x, float y, float z, float rad, int slices, int stacks, float r, float g, float b, bool solid, int lineW = 1){
    glPushMatrix();
    glTranslated(x,y,z);
    glColor3f(r,g,b);
    if(solid){
        glutSolidSphere(rad,slices,stacks);
    } else {
        glLineWidth(lineW);
        glutWireSphere(rad,slices,stacks);
    }
    glPopMatrix();
}
void paintBall(){

    if(shrink){
        glPushMatrix();
        {
            float size = 0.75;
            glScalef(size, size, size);
            paintSphere(ball.x / size, ball.y / size, ball.z / size, ball.rad, ball.slices, ball.stacks, ball.r, ball.g, ball.b, true, true);
            paintSphere2(ball.x / size, ball.y / size, ball.z / size, ball.rad, ball.slices/2, ball.stacks/2, ball.r, 0, 0, false, 3);
            shrink--;
        }
        glPopMatrix();
    }
    else {
        paintSphere(ball.x, ball.y, ball.z, ball.rad, ball.slices, ball.stacks, ball.r, ball.g, ball.b, false, false);
        paintSphere2(ball.x, ball.y, ball.z, ball.rad, ball.slices/2, ball.stacks/2, ball.r, 0, 0, false, 3);
    }
}

void writeBigStringWide(GLdouble x, GLdouble y, string s, float size, int r, int g, int b){

    unsigned int i;
    glMatrixMode(GL_MODELVIEW);
    
    glPushMatrix();
    glTranslatef(x, y, 0);
    
    glScaled( size, size, 0.2);
    
    glColor3b(r, g, b);
    
    for (i = 0; i < s.size(); i++){
        glutStrokeCharacter(GLUT_STROKE_ROMAN, s[i]);

    }
    glPopMatrix();
    glColor3f(1.0, 1.0, 1.0);

}

void displayPoints(){

    
    writeBigStringWide(-2, -2.6, "Enrique Hernandez A01185423", 0.003, 0, 0, 0);
    writeBigStringWide(-2, -2.3, "Luis Juan Sanchez A01183634", 0.003, 0, 0, 0);
    writeBigStringWide(-2, 2, to_string(score1), 0.009, 0, 0, 0);
    writeBigStringWide(2, 2, to_string(score2), 0.009, 0, 0, 0);
    
}

//
//void display() {
//    
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
//    
//    glColor3f(1.0, 1.0, 1.0);
//
//    glPushMatrix();
//    {
//
//        glRotatef(orientation, 0, 0, 1);
//        glRotatef(ball.x * 10, 0, 1, 0);
//        
//        paintTable();
//        
//        paintPadel(p1);
//        paintPadel(p2);
//        
//        paintBall();
//        
//        displayPoints();
//
//    }
//    glPopMatrix();
//    
//    glutSwapBuffers();
//}

void boton1(){
    

    //glPushMatrix()
    //Habilitar el uso de texturas
    glEnable(GL_TEXTURE_2D);
    
    //Elegir la textura del Quads: angulo cambia con el timer
    if (!mouseIn) {
        glBindTexture(GL_TEXTURE_2D, texName[8]);

        
    }else{
        glBindTexture(GL_TEXTURE_2D, texName[9]);

        
    }
    
    glBegin(GL_QUADS);
    
    float x = 2.0;
    float y = 1.0;
    
    //Asignar la coordenada de textura 0,0 al vertice
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-x, -y, 0);
    //Asignar la coordenada de textura 1,0 al vertice
    glTexCoord2f(1.0f, 0.0f); ///-
    glVertex3f(x, -y, 0);
    //Asignar la coordenada de textura 1,1 al vertice
    glTexCoord2f(1.0f,1.0f); //-
    //glTexCoord2f(1.0f,1.0f);
    //glTexCoord2f(2.0f,5.0f);
    glVertex3f(x, y, 0);
    //Asignar la coordenada de textura 0,1 al vertice
    glTexCoord2f(0.0f, 1.0f);
    //glTexCoord2f(0.0f, 5.0f);
    
    glVertex3f(-x, y, 0);
    glEnd();
    
}
void menu(){
    
    
    //Habilitar el uso de texturas
    glEnable(GL_TEXTURE_2D);
    
    //Elegir la textura del Quads: angulo cambia con el timer
    glBindTexture(GL_TEXTURE_2D, texName[2]);
    
    glBegin(GL_QUADS);
    //Asignar la coordenada de textura 0,0 al vertice
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-4.0f, -4.0f, 0);
    //Asignar la coordenada de textura 1,0 al vertice
    glTexCoord2f(1.0f, 0.0f); ///-
    glVertex3f(4.0f, -4.0f, 0);
    //Asignar la coordenada de textura 1,1 al vertice
    glTexCoord2f(1.0f,1.0f); //-
    //glTexCoord2f(1.0f,1.0f);
    //glTexCoord2f(2.0f,5.0f);
    glVertex3f(4.0f, 4.0f, 0);
    //Asignar la coordenada de textura 0,1 al vertice
    glTexCoord2f(0.0f, 1.0f);
    //glTexCoord2f(0.0f, 5.0f);
    
    glVertex3f(-4.0f, 4.0f, 0);
    glEnd();
}



void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    glColor3f(1.0, 1.0, 1.0);

    
    if(game){
        glPushMatrix();
        {
            
            glRotatef(orientation, 0, 0, 1);
            glRotatef(ball.x * 10, 0, 1, 0);
            
            paintTable();
            
            paintPadel(p1);
            paintPadel(p2);
            
            paintBall();
            
            displayPoints();
            
        }
        glPopMatrix();
    } else {
        
        glPushMatrix();
            boton1();
            menu();
        glPopMatrix();
    }
    
    glutSwapBuffers();
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, 1, 1, 100);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0, 0, 7.5, 0, 0, 0, 0, 1, 0);
}

void motionfunc (int x, int y){
    float xx = (x - 250) / 62.5;
    float yy = -(y - 250) / 62.5;
    //cout<<xx<<" : "<<yy<<endl;
    if(abs(xx) < 1.75 && abs(yy) < 0.85){
        mouseIn = true;
    }else {
        mouseIn = false;
    }
}
void mousefunc (int button, int state, int x, int y){
    float xx = (x - 250) / 62.5;
    float yy = -(y - 250) / 62.5;
    //cout<<xx<<" : "<<yy<<endl;
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        if(abs(xx) < 1.75 && abs(yy) < 0.85){
            if(!game){
                startGame();
                game = true;
            }
            running = true;
        }
    }
}

void myKeyboard(unsigned char theKey, int mouseX, int mouseY){
    switch (theKey){
        case 'I':
        case 'i':
            // iniciar
        if(!game){
            startGame();
            game = true;
        }
        running = true;
        break;

        case 'P':
        case 'p':
            //stop;
        running = false;
        break;

        case 'O':
        case 'o':
        (orientation == 0) ? orientation = 90 : orientation = 0;
        break;

        case 'R':
        case 'r':
            // reset
        objInit();
        //timer(0);
        running = false;
        game = false;
        resetGameValues();
        break;

        case 'W':
        case 'w':
        case 'A':
        case 'a':
        p1.speed = 0.1;
        break;

        case 'S':
        case 's':
        case 'D':
        case 'd':
        p1.speed = -0.1;
        break;

        case 27:
        exit(0);
        break;
    }
}


void SpecialInput(int key, int x, int y)
{
    switch(key)
    {
        case GLUT_KEY_UP:
        case GLUT_KEY_LEFT:
        p2.speed = 0.1;
        break;

        case GLUT_KEY_DOWN:
        case GLUT_KEY_RIGHT:
        p2.speed = -0.1;
        break;
    }
}

void init() {
    glClearColor(0.1, 0.5, 0.1, 1);
    // IMPORTANTE PARA QUE SE VEA LA PROFUNDIDAD
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    objInit();
}


//MARK: MAIN
int main(int argc, char** argv)
{
    glutInit(&argc,argv);
    
    ///AUDIO
    #ifdef __APPLE__
    #else
    alutInit(&argc,argv);
    #endif
    
    
    alGetError();
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(500,500);
    glutInitWindowPosition(100,100);
    glutCreateWindow("Pong");
    
    initRendering();
    glutReshapeFunc(reshape);
    glutDisplayFunc(display);
    getParentPath();
    init();
    
    //AUDIO
    #ifdef __APPLE__ 
    #else
    if (LoadALData() == AL_FALSE)
        return -1;
    SetListenerValues();
    atexit(KillALData);
    #endif
    //AUDIO
    
    
    glutKeyboardFunc(myKeyboard);
    glutMouseFunc(mousefunc);
    glutSpecialFunc(SpecialInput);
    glutPassiveMotionFunc(motionfunc);
    glutTimerFunc(10, timer, 0);
    glutMainLoop();
}





#include <assert.h>
#include <fstream>

#include "imageloader.h"

using namespace std;

Image::Image(char* ps, int w, int h) : pixels(ps), width(w), height(h)
{
    
}

Image::~Image()
{
    delete[] pixels;
}

namespace
{
    //Converts a four-character array to an integer, using little-endian form
    int toInt(const char* bytes)
    {
        return (int)(((unsigned char)bytes[3] << 24) |
                     ((unsigned char)bytes[2] << 16) |
                     ((unsigned char)bytes[1] << 8) |
                     (unsigned char)bytes[0]);
    }
    
    //Converts a two-character array to a short, using little-endian form
    short toShort(const char* bytes)
    {
        return (short)(((unsigned char)bytes[1] << 8) |
                       (unsigned char)bytes[0]);
    }
    
    //Reads the next four bytes as an integer, using little-endian form
    int readInt(ifstream &input)
    {
        char buffer[4];
        input.read(buffer, 4);
        return toInt(buffer);
    }
    
    //Reads the next two bytes as a short, using little-endian form
    short readShort(ifstream &input)
    {
        char buffer[2];
        input.read(buffer, 2);
        return toShort(buffer);
    }
    
    //Just like auto_ptr, but for arrays
    template<class T>
    class auto_array
    {
    private:
        T* array;
        mutable bool isReleased;
    public:
        explicit auto_array(T* array_ = NULL) :
        array(array_), isReleased(false)
        {
        }
        
        auto_array(const auto_array<T> &aarray)
        {
            array = aarray.array;
            isReleased = aarray.isReleased;
            aarray.isReleased = true;
        }
        
        ~auto_array()
        {
            if (!isReleased && array != NULL)
            {
                delete[] array;
            }
        }
        
        T* get() const
        {
            return array;
        }
        
        T &operator*() const
        {
            return *array;
        }
        
        void operator=(const auto_array<T> &aarray)
        {
            if (!isReleased && array != NULL)
            {
                delete[] array;
            }
            array = aarray.array;
            isReleased = aarray.isReleased;
            aarray.isReleased = true;
        }
        
        T* operator->() const
        {
            return array;
        }
        
        T* release()
        {
            isReleased = true;
            return array;
        }
        
        void reset(T* array_ = NULL)
        {
            if (!isReleased && array != NULL)
            {
                delete[] array;
            }
            array = array_;
        }
        
        T* operator+(int i)
        {
            return array + i;
        }
        
        T &operator[](int i)
        {
            return array[i];
        }
    };
}

Image* loadBMP(const char* filename)
{
    ifstream input;
    input.open(filename, ifstream::binary);
    assert(!input.fail() || !"Could not find file");
    char buffer[2];
    input.read(buffer, 2);
    assert(buffer[0] == 'B' && buffer[1] == 'M' || !"Not a bitmap file");
    input.ignore(8);
    int dataOffset = readInt(input);
    
    //Read the header
    int headerSize = readInt(input);
    int width;
    int height;
    switch (headerSize)
    {
        case 40:
            //V3
            width = readInt(input);
            height = readInt(input);
            input.ignore(2);
            assert(readShort(input) == 24 || !"Image is not 24 bits per pixel");
            assert(readShort(input) == 0 || !"Image is compressed");
            break;
        case 12:
            //OS/2 V1
            width = readShort(input);
            height = readShort(input);
            input.ignore(2);
            assert(readShort(input) == 24 || !"Image is not 24 bits per pixel");
            break;
        case 64:
            //OS/2 V2
            assert(!"Can't load OS/2 V2 bitmaps");
            break;
        case 108:
            //Windows V4
            assert(!"Can't load Windows V4 bitmaps");
            break;
        case 124:
            //Windows V5
            assert(!"Can't load Windows V5 bitmaps");
            break;
        default:
            assert(!"Unknown bitmap format");
    }
    
    //Read the data
    int bytesPerRow = ((width * 3 + 3) / 4) * 4 - (width * 3 % 4);
    int size = bytesPerRow * height;
    auto_array<char> pixels(new char[size]);
    input.seekg(dataOffset, ios_base::beg);
    input.read(pixels.get(), size);
    
    //Get the data into the right format
    auto_array<char> pixels2(new char[width * height * 3]);
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            for (int c = 0; c < 3; c++)
            {
                pixels2[3 * (width * y + x) + c] =
                pixels[bytesPerRow * y + 3 * x + (2 - c)];
            }
        }
    }
    
    input.close();
    return new Image(pixels2.release(), width, height);
}


