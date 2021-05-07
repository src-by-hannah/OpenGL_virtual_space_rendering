//
// main.cpp
// GL_Project
//
// ������� ������ �� Navigation
// - ��ǻ�� �׷��Ƚ� 01�й�, �⸻ ������Ʈ
//
// created by Hannah Noh on 17/12/20
//
//
// ** ��� �� ���� ���� **
// - Asset�� ��ġ�� ���� ��θ� Ȯ�����ּ���.
//
//
// ** Reference **
// [1] Lighthouse3d.com, "Keyboard Example: Moving the Camera", 
//     <http://www.lighthouse3d.com/tutorials/glut-tutorial/keyboard-example-moving-around-the-world/>
//

#define _CRT_SECURE_NO_WARNINGS

#include <GL/glut.h>
#include <stdio.h>
#include <math.h>
#include <iostream>

using namespace std;

// Rendering Style
#define	WIRE 0		
#define	FLATSHADE 1		
#define	SMOOTHSHADE 2		

// Texture Mapping Property
#define	POINT 0		
#define	LINEAR 1		

#define	CLAMP 0		
#define	REPEAT 1		

#define	DECAL 1		
#define	REPLACE 2		
#define	MODULATE 3		
#define	BLEND 4	

#define PARALLEL 0
#define PERSP 1


// �Լ� �ʱ�ȭ
void ReadModel();
unsigned char* Read_BmpImage(const char name[], int* width, int* height, int* components);
void DrawWire(void);
void DrawShade(void);
void DrawScene(void);
void MakeGL_Model(void);
void MakeGL_Ground(void);
void Material(int num);
void InitLight();
void MyDisplay(void);
void MyHelp();
void MyKeyboard(unsigned char key, int x, int y);
void MySpecialKeyboard(int key, int x, int y);
void MyMainMenu(int entryID);
void MySubMenuRender(int entryID);
void MySubMenuMapping(int entryID);
void MySubMenuTexture(int entryID);
void MySubMenuProjection(int entryID);


// Texture ���� ����
typedef struct {
	float x;
	float y;
	float z;
} Point;

typedef struct {
	unsigned int ip[3];
} Face;

Point* mpoint = NULL;
Face* mface = NULL;

int pnum;
int fnum;

int filter = 0;
int mode = 1;
int tscale = 1;
int modulate = 1;

string tfname = "./Asset/wall.bmp";

// Rendering ���� �Լ�
int status = 0; // (default) wireframe
int isCull = 0; // (default) Back-Face Culling Off
int isLight = 0; // (default) Ligth Off
int projection = 1; // (default) perspective veiw

// camera ���� �Լ�
float angle = 0.0; // Ref-COP ���� ���� (-z�� ������ 0��)
float lx = 0.0f, lz = -1.0f; // Ref-COP ���� ����
float copX = 0.0f, copZ = 300.0f; // COP X, Z

// �̿� ���� ����
int w, h;
string fname;


/*	cnormal �Լ�
 *
 *	: Normal Vector ���
 */
Point cnormal(Point a, Point b, Point c) {
	Point p, q, r;
	double val;
	// p ���� ���ϱ�
	p.x = a.x - b.x;
	p.y = a.y - b.y;
	p.z = a.z - b.z;

	// q ���� ���ϱ�
	q.x = c.x - b.x;
	q.y = c.y - b.y;
	q.z = c.z - b.z;

	// p X q
	r.x = p.y * q.z - p.z * q.y;
	r.y = p.z * q.x - p.x * q.z;
	r.z = p.x * q.y - p.y * q.x;

	// ����ȭ
	val = sqrt(r.x * r.x + r.y * r.y + r.z * r.z);
	r.x = r.x / val;
	r.y = r.y / val;
	r.z = r.z / val;

	return r; // Normal ���� ��ȯ
}

/*	ReadModel �Լ�
 *
 *	: Model data file �б�
 */
void ReadModel() {
	// ���� �ʱ�ȭ
	FILE* f1;
	char s[81];
	int i;

	if (mpoint != NULL)
		delete mpoint;

	if (mface != NULL)
		delete mface;

	// ���� ����
	if ((f1 = fopen(fname.c_str(), "rt")) == NULL) {
		printf("No file\n");
		exit(0);
	}

	// Point ���� ���� �� ũ�⸸ŭ �迭 ����
	fscanf(f1, "%s", s);
	fscanf(f1, "%s", s);
	fscanf(f1, "%d", &pnum);
	mpoint = new Point[pnum];

	// �迭 Point ���� ����
	for (i = 0; i < pnum; i++) {
		fscanf(f1, "%f", &mpoint[i].x);
		fscanf(f1, "%f", &mpoint[i].y);
		fscanf(f1, "%f", &mpoint[i].z);
	}

	// Face ���� ���� �� ũ�⸸ŭ �迭 ����
	fscanf(f1, "%s", s);
	fscanf(f1, "%s", s);
	fscanf(f1, "%d", &fnum);
	mface = new Face[fnum];

	// �迭�� Face ���� ����
	for (i = 0; i < fnum; i++) {
		fscanf(f1, "%d", &mface[i].ip[0]);
		fscanf(f1, "%d", &mface[i].ip[1]);
		fscanf(f1, "%d", &mface[i].ip[2]);
	}

	fclose(f1);
}

/*	Read_BmpImage �Լ�
 *
 *	: Texture data file �б�
 */
unsigned char* Read_BmpImage(const char name[], int* width, int* height, int* components) {
	FILE* BMPfile;
	GLubyte garbage;
	long size;
	int start_point, x;
	GLubyte temp[3];
	GLubyte start[4], w[4], h[4];
	unsigned char* read_image;

	BMPfile = fopen(name, "rb");

	for (x = 0; x < 10; x++) {
		fread(&garbage, 1, 1, BMPfile);
	}

	fread(&start[0], 1, 4, BMPfile);

	for (x = 0; x < 4; x++) {
		fread(&garbage, 1, 1, BMPfile);
	}

	fread(&w[0], 1, 4, BMPfile);
	fread(&h[0], 1, 4, BMPfile);

	(*width) = (w[0] + 256 * w[1] + 256 * 256 * w[2] + 256 * 256 * 256 * w[3]);
	(*height) = (h[0] + 256 * h[1] + 256 * 256 * h[2] + 256 * 256 * 256 * h[3]);
	size = (*width) * (*height);
	start_point = (start[0] + 256 * start[1] + 256 * 256 * start[2] + 256 * 1256 * 256 * start[3]);

	read_image = (unsigned char*)malloc(size * 3);

	for (x = 0; x < (start_point - 26); x++) {
		fread(&garbage, 1, 1, BMPfile);
	}

	for (x = 0; x < (size * 3); x = x + 3) {
		fread(&temp[0], 1, 3, BMPfile);
		read_image[x] = temp[2];
		read_image[x + 1] = temp[1];
		read_image[x + 2] = temp[0];
	}
	fclose(BMPfile);
	return (unsigned char*)read_image;
}

/*	DrawWire �Լ�
 *
 *	: Rendering Style - Wireframe
 */
void DrawWire(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);

	// �� on/off
	if (isLight) {
		glEnable(GL_LIGHT0);
		glEnable(GL_LIGHTING);
	}
	else {
		glDisable(GL_LIGHT0);
		glDisable(GL_LIGHTING);
	}

	// Back-Face Culling
	if (isCull) glEnable(GL_CULL_FACE);
	else glDisable(GL_CULL_FACE);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glCallList(1);

	// Ground�� Texture�� ������ ���¸� ����
	glEnable(GL_TEXTURE_2D);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glCallList(2);

	glutSwapBuffers();
}

/*	DrawShade �Լ�
 *
 *	: Rendering Style - Shader
 */
void DrawShade(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	
	// �� on/off
	if (isLight) {
		glEnable(GL_LIGHT0);
		glEnable(GL_LIGHTING);
	}
	else {
		glDisable(GL_LIGHT0);
		glDisable(GL_LIGHTING);
	}

	// Back-Face Culling
	if (isCull) glEnable(GL_CULL_FACE);
	else glDisable(GL_CULL_FACE);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glCallList(1);

	// Ground�� Texture�� ������ ���¸� ����
	glEnable(GL_TEXTURE_2D);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glCallList(2);

	glutSwapBuffers();
}

/*	DrawScene �Լ�
 *
 *	: Rendering Style�� �°� Scene �׸���
 */
void DrawScene(void) {
	// �׸���
	if (status == WIRE) {
		DrawWire();
	}

	else if (status == FLATSHADE) {
		glShadeModel(GL_FLAT);
		DrawShade();
	}

	else if (status == SMOOTHSHADE) {
		glShadeModel(GL_SMOOTH);
		DrawShade();
	}
}

/*	MakeGL_Model �Լ�
 *
 *	: point�� face ������ �������� �� ����
 */
void MakeGL_Model(void) {
	int i;

	if (!glIsList(1)) {
		glNewList(1, GL_COMPILE);

		/* [Object 1] Chair */
		fname = "./Asset/chair.dat";
		ReadModel();

		glPushMatrix();

		// Modeling Transformation
		glScalef(0.5, 0.5, 0.5);
		glColor3f(1.0f, 0.0, 0.0);

		for (i = 0; i < fnum; i++) {
			Point norm = cnormal(mpoint[mface[i].ip[2]], mpoint[mface[i].ip[1]], mpoint[mface[i].ip[0]]); // �������� ���

			glBegin(GL_TRIANGLES);
			glNormal3f(norm.x, norm.y, norm.z);
			glVertex3f(mpoint[mface[i].ip[0]].x, mpoint[mface[i].ip[0]].y, mpoint[mface[i].ip[0]].z);
			glNormal3f(norm.x, norm.y, norm.z);
			glVertex3f(mpoint[mface[i].ip[1]].x, mpoint[mface[i].ip[1]].y, mpoint[mface[i].ip[1]].z);
			glNormal3f(norm.x, norm.y, norm.z);
			glVertex3f(mpoint[mface[i].ip[2]].x, mpoint[mface[i].ip[2]].y, mpoint[mface[i].ip[2]].z);
			glEnd();
		}

		glPopMatrix();

		/* [Object 2] mysphere */
		fname = "./Asset/mysphere.dat";
		ReadModel();

		glPushMatrix();

		// Modeling Transformation
		glScalef(0.5, 0.5, 0.5);
		glTranslatef(-150, 100, 100);
		glColor3f(0, 0, 1);

		for (i = 0; i < fnum; i++) {
			Point norm = cnormal(mpoint[mface[i].ip[2]], mpoint[mface[i].ip[1]], mpoint[mface[i].ip[0]]); // �������� ���

			glBegin(GL_TRIANGLES);
			glNormal3f(norm.x, norm.y, norm.z);
			glVertex3f(mpoint[mface[i].ip[0]].x, mpoint[mface[i].ip[0]].y, mpoint[mface[i].ip[0]].z);
			glNormal3f(norm.x, norm.y, norm.z);
			glVertex3f(mpoint[mface[i].ip[1]].x, mpoint[mface[i].ip[1]].y, mpoint[mface[i].ip[1]].z);
			glNormal3f(norm.x, norm.y, norm.z);
			glVertex3f(mpoint[mface[i].ip[2]].x, mpoint[mface[i].ip[2]].y, mpoint[mface[i].ip[2]].z);
			glEnd();
		}

		glPopMatrix();

		/* [Object 3] Color Triangle */
		glPushMatrix();
		glBegin(GL_TRIANGLES);
		glColor3f(1, 0, 0);
		glVertex3f(0, 100, -150);
		glColor3f(0, 1, 0);
		glVertex3f(-100, 0, -150);
		glColor3f(0, 0, 1);
		glVertex3f(100, 0, -150);
		glEnd();
		glPopMatrix();

		/* [Object 4] torus */
		fname = "./Asset/mytorus.dat";
		ReadModel();

		glPushMatrix();

		// Modeling Transformation
		glScalef(0.3, 0.3, 0.3);
		glTranslatef(-230, 100, -230);
		glColor3f(0, 1, 0);

		for (i = 0; i < fnum; i++) {
			Point norm = cnormal(mpoint[mface[i].ip[2]], mpoint[mface[i].ip[1]], mpoint[mface[i].ip[0]]); // �������� ���

			glBegin(GL_TRIANGLES);
			glNormal3f(norm.x, norm.y, norm.z);
			glVertex3f(mpoint[mface[i].ip[0]].x, mpoint[mface[i].ip[0]].y, mpoint[mface[i].ip[0]].z);
			glNormal3f(norm.x, norm.y, norm.z);
			glVertex3f(mpoint[mface[i].ip[1]].x, mpoint[mface[i].ip[1]].y, mpoint[mface[i].ip[1]].z);
			glNormal3f(norm.x, norm.y, norm.z);
			glVertex3f(mpoint[mface[i].ip[2]].x, mpoint[mface[i].ip[2]].y, mpoint[mface[i].ip[2]].z);
			glEnd();
		}

		glPopMatrix();

		glEndList();
	}
}

/*	MakeGL_Ground �Լ�
 *
 *	: Ground ����
 */
void MakeGL_Ground(void)
{
	unsigned char* image = NULL;
	int iwidth, iheight, idepth;

	if (mode == CLAMP) {
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	}
	else {
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	if (filter == POINT) {
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	else {
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	if (modulate == DECAL) {
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	}
	else if (modulate == REPLACE) {
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	}
	else if (modulate == MODULATE) {
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	}
	else {
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
	}

	if (!glIsList(2)) {
		glNewList(2, GL_COMPILE);
		glPushMatrix();

		image = Read_BmpImage(tfname.c_str(), &iwidth, &iheight, &idepth);

		glTexImage2D(GL_TEXTURE_2D, 0, 3, iwidth, iheight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

		glColor3f(0.0f, 0.0f, 1.0f);
		glBegin(GL_TRIANGLES);
		glNormal3d(0, 0, -1);
		glTexCoord2d(0, 0);
		glVertex3d(-300, 0, -300);

		glTexCoord2d(1 * tscale, 0);
		glVertex3d(300, 0, -300);

		glTexCoord2d(0, 1 * tscale);
		glVertex3d(-300, 0, 300);
		glEnd();

		glColor3f(0.0f, 1.0f, 0.0f);
		glBegin(GL_TRIANGLES);
		glNormal3d(0, 0, -1);
		glTexCoord2d(1 * tscale, 0);
		glVertex3d(300, 0, -300);

		glTexCoord2d(1 * tscale, 1 * tscale);
		glVertex3d(300, 0, 300);

		glTexCoord2d(0, 1 * tscale);
		glVertex3d(-300, 0, 300);
		glEnd();

		glPopMatrix();
		glEndList();
	}
}

/*	Materiel �Լ�
 *
 *	: Object ���� ����
 */
void Material(int num) {
	GLfloat mat_diffuse[4];
	GLfloat mat_specular[4];
	GLfloat mat_ambient[4];
	GLfloat mat_shininess[4];

	// ����
	// gold
	if (num == 1) {
		mat_diffuse[0] = 0.7516f; mat_diffuse[1] = 0.6065f; mat_diffuse[2] = 0.2265f; mat_diffuse[3] = 1.0f;
		mat_specular[0] = 0.6283f; mat_specular[1] = 0.5558f; mat_specular[2] = 0.3661f; mat_specular[3] = 1.0f;
		mat_ambient[0] = 0.2473f; mat_ambient[1] = 0.1995f; mat_ambient[2] = 0.0745f; mat_ambient[3] = 1.0f;
		mat_shininess[0] = 51.2f;
	}
	// bronze
	else if (num == 2) {
		mat_diffuse[0] = 0.2125f; mat_diffuse[1] = 0.1275f; mat_diffuse[2] = 0.0054f; mat_diffuse[3] = 1.0f;
		mat_specular[0] = 0.714f; mat_specular[1] = 0.4284f; mat_specular[2] = 0.18144f; mat_specular[3] = 1.0f;
		mat_ambient[0] = 0.3935f; mat_ambient[1] = 0.2719f; mat_ambient[2] = 0.1667f; mat_ambient[3] = 1.0f;
		mat_shininess[0] = 25.6f;
	}
	// chrome
	else if (num == 3) {
		mat_diffuse[0] = 0.25f; mat_diffuse[1] = 0.25f; mat_diffuse[2] = 0.25f; mat_diffuse[3] = 1.0f;
		mat_specular[0] = 0.4f; mat_specular[1] = 0.4f; mat_specular[2] = 0.4f; mat_specular[3] = 1.0f;
		mat_ambient[0] = 0.7746f; mat_ambient[1] = 0.7746f; mat_ambient[2] = 0.7746f; mat_ambient[3] = 1.0f;
		mat_shininess[0] = 76.8f;
	}
	// copper
	else if (num == 4) {
		mat_diffuse[0] = 0.1913f; mat_diffuse[1] = 0.0735f; mat_diffuse[2] = 0.225f; mat_diffuse[3] = 1.0f;
		mat_specular[0] = 0.7038f; mat_specular[1] = 0.2705f; mat_specular[2] = 0.0828f; mat_specular[3] = 1.0f;
		mat_ambient[0] = 0.2568f; mat_ambient[1] = 0.1376f; mat_ambient[2] = 0.0860f; mat_ambient[3] = 1.0f;
		mat_shininess[0] = 12.8f;
	}

	// ���� ����
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
}

/*	InitLight �Լ�
 *
 *	: ������ ���� ���� �� Ȱ��ȭ
 */
void InitLight() {
	// �� ����
	GLfloat light_specular[] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat light_diffuse[] = { 0.8, 0.8, 0.8, 1.0 };
	GLfloat light_ambient[] = { 0.3, 0.3, 0.3, 1.0 };
	GLfloat light_position[] = { 100, 100.0, 400.0, 0.0 };

	glEnable(GL_DEPTH_TEST);

	// ���� ���� ���� ON
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	// ���� ���� ����
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
}

/*	MyDisplay �Լ�
 *
 *	: display �ݹ� �Լ�
 */
void MyDisplay(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// perspective �� ���
	if (projection) {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(40.0, 1.0, 1.0, 2000.0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(copX, 100.0f, copZ,
			copX + lx, 100.0f, copZ + lz,
			0.0f, 1.0f, 0.0f);

		DrawScene();
		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();

		glutPostRedisplay();
	}

	// parallel �� ���
	else {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(-100, 100, -100, 100, 0.5, 500.0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(copX, 100.0f, copZ,
			copX + lx, 100.0f, copZ + lz,
			0.0f, 1.0f, 0.0f);
		DrawScene();
		glPopMatrix();

		glMatrixMode(GL_PROJECTION);
		glPopMatrix();

		glutPostRedisplay();
	}
}

/*	Myhelp �Լ�
 *
 *	: ����
 */
void MyHelp() {
	printf("<< KeyBoard Event �ȳ� >>\n");
	printf("* Material ����\n");
	printf("=> 1. Gold\n");
	printf("=> 2. Bronze\n");
	printf("=> 3. Chrome\n");
	printf("=> 4. Copper\n\n");
}

/*	MyKeyboard �Լ�
 *
 *	: Material ���� (1/2/3/4)
 */
void MyKeyboard(unsigned char key, int x, int y) {
	switch (key)
	{
	case '1':
		Material(1);
		printf("Material : Gold\n");
		break;

	case '2':
		Material(2);
		printf("Material : Bronze\n");
		break;

	case '3':
		Material(3);
		printf("Material : Chrome\n");
		break;

	case '4':
		Material(4);
		printf("Material : Copper\n");
		break;

	case 27:
		exit(0);
		break;
	}
}

/*	MySpecialKeyboard �Լ�
 *
 *	: ����Ű ����
 *	: �ٴ� �浹 üũ
 */
void MySpecialKeyboard(int key, int x, int y) {
	// ������ ����
	float moveFource = 10.0f;

	// �ٴ� �浹 üũ�� ���� ����
	float updateX;
	float updateZ;

	switch (key) {
	case GLUT_KEY_LEFT:
		angle -= 0.1f;
		lx = sin(angle);
		lz = -cos(angle);
		break;

	case GLUT_KEY_RIGHT:
		angle += 0.1f;
		lx = sin(angle);
		lz = -cos(angle);
		break;

	case GLUT_KEY_UP:
		updateX = copX + lx * moveFource;
		updateZ = copZ + lz * moveFource;
		if (-300 <= updateX && updateX <= 300)
			copX = updateX;
		else updateX = copX;

		if (-300 <= updateZ && updateZ <= 300)
			copZ = updateZ;
		else updateZ = copZ;

		break;

	case GLUT_KEY_DOWN:
		updateX = copX - lx * moveFource;
		updateZ = copZ - lz * moveFource;
		if (-300.0 <= updateX && updateX <= 300.0)
			copX = updateX;
		else updateX = copX;

		if (-300.0 <= updateZ && updateZ <= 300.0)
			copZ = updateZ;
		else updateZ = copZ;
		break;
	}
}

/*
 *	MyMainMenu(int entryID)
 *
 *	: entryID�� 1�� ��� = Exit
 */
void MyMainMenu(int entryID) {
	if (entryID == 1) {
		if (isLight) {
			isLight = 0;
			printf("Light Off");
		}
		else {
			isLight = 1;
			printf("Light Off");
		}
	}
	else if (entryID == 2) {
		exit(0);
	}
}

/*
 *	MySubMenuRender �Լ�
 *
 *	: entryID�� 1�� ��� = Wireframe
 *	: entryID�� 2�� ��� = Flat Shading
 *	: entryID�� 3�� ��� = Smooth Shading
 *	: entryID�� 4�� ��� = Back-Face Culling(On/Off)
 */
void MySubMenuRender(int entryID) {
	if (entryID == 1) {
		status = WIRE;
		printf("Rendering Style : Wireframe\n");
	}
	else if (entryID == 2) {
		status = FLATSHADE;
		printf("Rendering Style : Flat Shading\n");
	}
	else if (entryID == 3) {
		status = SMOOTHSHADE;
		printf("Rendering Style : Smooth Shading\n");
	}
	else if (entryID == 4) {
		if (isCull) {
			isCull = 0;
			printf("Back-Face Culling Off\n");
		}
		else {
			isCull = 1;
			printf("Back-Face Culling On\n");
		}
	}

	glutPostRedisplay();
}

/*
 *	MySubMenuMapping �Լ�
 *
 *	: entryID�� 1�� ��� = Decal
 *	: entryID�� 2�� ��� = Replace
 *	: entryID�� 3�� ��� = Modulate
 *	: entryID�� 4�� ��� = Blend
 */
void MySubMenuMapping(int entryID) {
	if (entryID == 1) {
		modulate = DECAL;
		printf("Texture Mapping: Decal\n");
	}
	else if (entryID == 2) {
		modulate = REPLACE;
		printf("Texture Mapping: Replace\n");
	}
	else if (entryID == 3) {
		modulate = MODULATE;
		printf("Texture Mapping: Modulate\n");
	}
	else if (entryID == 4) {
		modulate = BLEND;
		printf("Texture Mapping: Blend\n");
	}

	glutPostRedisplay();
}

/*
 *	MySubMenuTexture �Լ�
 *
 *	: entryID�� 1�� ��� = Samping
 *	: entryID�� 2�� ��� = Clamp / Repeat
 */
void MySubMenuTexture(int entryID) {
	if (entryID == 1) {
		if (filter) {
			filter = POINT;
			printf("Point Samping\n");
		}
		else {
			filter = LINEAR;
			printf("Linear Samping\n");
		}
	}
	else if (entryID == 2) {
		if (mode) {
			mode = CLAMP;
			printf("Texture Clamp\n");
		}
		else {
			mode = REPEAT;
			printf("Texture Repeat\n");
		}
	}

	glutPostRedisplay();
}

/*
 *	MySubMenuProjection �Լ�
 *
 *	: entryID�� 1�� ��� = Parallel
 *	: entryID�� 2�� ��� = Perspective
 */
void MySubMenuProjection(int entryID) {
	if (entryID == 1) {
		projection = 0;
		printf("Parallel Projection\n");
	}
	else if (entryID == 2) {
		projection = 1;
		printf("Perspective Projection\n");
	}

	glutPostRedisplay();
}

/*
 *	main �Լ�
 *
 *	: ����
 */
int main(int argc, char** argv) {
	glutInit(&argc, argv);
	w = 500; h = 500;
	glutInitWindowSize(w, h);
	glutInitWindowPosition(100, 100);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutCreateWindow("20190149_HannahNoh");

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // ���� (���)

	/* Menu ���� */
	GLint MySubMenuIDRender = glutCreateMenu(MySubMenuRender);
	glutAddMenuEntry("Wireframe", 1);
	glutAddMenuEntry("Flat Shading", 2);
	glutAddMenuEntry("Smooth Shading", 3);
	glutAddMenuEntry("Back-Face Culling(On/Off)", 4);

	GLint MySubMenuIDMapping = glutCreateMenu(MySubMenuMapping);
	glutAddMenuEntry("Decal", 1);
	glutAddMenuEntry("Replace", 2);
	glutAddMenuEntry("Modulate", 3);
	glutAddMenuEntry("Blend", 4);

	GLint MySubMenuIDTexture = glutCreateMenu(MySubMenuTexture);
	glutAddMenuEntry("[Point / Linear] Sampling", 1);
	glutAddMenuEntry("[Clamp / Repeat]", 2);
	glutAddSubMenu("Mapping", MySubMenuIDMapping);

	GLint MySubMenuIDProjection = glutCreateMenu(MySubMenuProjection);
	glutAddMenuEntry("Parallel", 1);
	glutAddMenuEntry("Perspective", 2);

	GLint MyMainMenuID = glutCreateMenu(MyMainMenu);
	glutAddSubMenu("Model Rendering", MySubMenuIDRender);
	glutAddSubMenu("Ground Texturing", MySubMenuIDTexture);
	glutAddSubMenu("Projection", MySubMenuIDProjection);
	glutAddMenuEntry("Lighting", 1);
	glutAddMenuEntry("Exit", 2);

	glutAttachMenu(GLUT_RIGHT_BUTTON);

	/* OpenGL Loop */
	// GLUT �ݹ��Լ�
	glutDisplayFunc(MyDisplay);
	glutKeyboardFunc(MyKeyboard);
	glutSpecialFunc(MySpecialKeyboard);

	// �� �ҷ�����
	MakeGL_Model();
	MakeGL_Ground();

	InitLight();
	MyHelp();
	glutMainLoop();

	return 0;
}