#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <iostream>
#include <cmath>

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
};

Ball ball;
Padel p1, p2;
bool running = false; //Pause and Unpause
bool game = false; //Start the game
int score1 = 0;
int score2 = 0;

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
	p1.sizex = 0.25;
	p1.sizey = 2;
	p1.sizez = 1;
	p1.r = 0;
	p1.g = 0;
	p1.b = 0;
	p1.speed = 0;

	//P2 INIT
	p2.x = 3.75;
	p2.y = 0;
	p2.z = 0;
	p2.sizex = 0.25;
	p2.sizey = 2;
	p2.sizez = 1;
	p2.r = 0;
	p2.g = 0;
	p2.b = 0;
	p2.speed = 0;
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

void checkCollisions(){
	//Vertical borders
	if(abs(ball.y) + ball.rad > 4){
		ball.diry *= -1;
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
	//Collision with right panel
	} else if(checkCollision(rb, rp2)){
		(ball.dirx > 0) ? ball.dirx = -ball.dirx : ball.dirx;
	//Goal on right side
	} else if(ball.x > 4){
		objInit();
		running = false;
		game = false;
		score1++;
	//Goal on left side
	}else if(ball.x < -4){
		objInit();
		running = false;
		game = false;
		score2++;
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
		glutPostRedisplay();
	}
	glutTimerFunc(50, timer, v);
}

void paintCube(float x, float y, float z, float sizex, float sizey, float sizez, float r, float g, float b, bool solid, int lineW = 1){
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
	paintCube(p.x, p.y, p.z, p.sizex, p.sizey, p.sizez, p.r, p.g, p.b, true);
	paintCube(p.x, p.y, p.z, p.sizex, p.sizey, p.sizez, 255, 0, 0, false, 5);
}

void paintTable(){
	glPushMatrix();
	glScalef(8,8,1);
	glColor3ub(255,140,0);
	glLineWidth(5);
	glutWireCube(1);
	glPopMatrix();

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
}

void paintSphere(float x, float y, float z, float rad, int slices, int stacks, float r, float g, float b, bool solid, int lineW = 1){
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
	paintSphere(ball.x, ball.y, ball.z, ball.rad, ball.slices, ball.stacks, ball.r, ball.g, ball.b, true);
	paintSphere(ball.x, ball.y, ball.z, ball.rad, ball.slices/2, ball.stacks/2, ball.r, 0, 0, false, 3);
}


void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glPushMatrix();
	{
		glRotatef(ball.x * 10, 0, 1, 0);

		paintTable();

		paintPadel(p1);
		paintPadel(p2);

		paintBall();
	}
	glPopMatrix();

	glutSwapBuffers();
}

void reshape(int w, int h) {
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, ((double)w)/h, 1, 10000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0, 0, 7.5, 0, 0, 0, 0, 1, 0);
}

void startGame(){
	double angle = rand() % 45 + 22.5;
	ball.diry = ball.speed * sin ( angle * 3.1416 / 180.0 ) * (1 - 2 * (rand() % 2));
	ball.dirx = ball.speed * cos ( angle * 3.1416 / 180.0 ) * (1 - 2 * (rand() % 2));
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

		case 'R':
		case 'r':
		// reset
		objInit();
		timer(0);
		running = false;
		game = false;
		break;

		case 'w':
		p1.speed = 0.1;
		break;

		case 's':
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
		p2.speed = 0.1;
		break;

		case GLUT_KEY_DOWN:
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

int main(int argc, char** argv)
{
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(500,500);
	glutInitWindowPosition(100,100);
	glutCreateWindow("Pong");
	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	init();
	glutKeyboardFunc(myKeyboard);
	glutSpecialFunc(SpecialInput);
	glutTimerFunc(50, timer, 0);
	glutMainLoop();
}