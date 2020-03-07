#include <iostream>
#include <opencv2/opencv.hpp> //<cv.h>
#include <opencv2/highgui.hpp> //<highgui.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <GL/glut.h>
#include <math.h>

#define FOLDER "./images/"
#define SIZE 512
#define START 1501
#define STOP 1541
#define JUMP 5
#define EXTRACTVALUE 600 // 600 Skin, 1300 Bones

using namespace std;
using namespace cv;

GLuint listHaut;
map<int,Mat> dat;
vector< vector<Vec3i> > skinC[STOP - START +1];
unsigned int images;
int zeile;
unsigned int konturRunner;

vector< vector<Vec3i> > pointExtract(Mat & bild, int z) {
    vector< vector<Vec3i> > result;
    int x,y;
    Vec3i dummy;

    vector< vector<Point> > cont;
    Mat temp;
    bild.convertTo(temp, CV_8U);
    findContours(temp, cont, RETR_EXTERNAL, CHAIN_APPROX_NONE);

    for(unsigned int i = 0; i<cont.size(); ++i) {
        
        if (cont[i].size()>40) {
            vector<Vec3i> resulti;
            for(unsigned int j = 0; j<cont[i].size(); j+=1){
                // z ist ebene, i ist index der gefundenen kontur, j ist der index des Punkts
                x = cont[i][j].x;
                y = cont[i][j].y;
                dummy[0] = x;
                dummy[1] = y;
                dummy[2] = z;
                //cout<<x<<","<<y<<","<<z<<""<<endl;
                resulti.push_back(dummy);
            }
            result.push_back(resulti);
        }
    }
    return result;
}

void readDcm(const string &fullPath, Mat &img){
    long fileSize, pixelSize = img.rows * img.cols *2;
    ifstream f(fullPath.c_str(), ifstream::binary);
    f.seekg(0, ios::end);
    fileSize = f.tellg();
    f.seekg(fileSize - pixelSize);
    f.read((char *) img.data, pixelSize);
    f.close();
}

void toGray(Mat &img){
    double mini, maxi, scale, shift;
    minMaxLoc(img, &mini, &maxi);
    scale = 255.0/(maxi-mini);
    shift = -mini*scale;
    Mat imgGray = Mat::zeros(img.rows, img.cols, CV_8U);
    convertScaleAbs(img, imgGray, scale, shift);
    img = imgGray.clone();
}

void extractIt(Mat &img, const double &level){
    GaussianBlur(img, img, Size(5,5), 0);
    compare(img, level, img, CMP_GT);
}

float lengthCalc(const float & x, const float & y, const float & z) {
    return sqrt(pow(x,2) + pow(y,2) + pow(z,2));
}

Vec3i nearTo(Vec3i & pos, vector< vector<Vec3i> > & cont) {
    Vec3i result;
    float d;
    float mini = 100000;
    for(unsigned int i = 0; i<cont.size(); i+=1){
        for(unsigned int j = 0; j<cont[i].size(); j+=1) {
            d = lengthCalc(
                pos[0] - cont[i][j][0],
                pos[1] - cont[i][j][1],
                pos[2] - cont[i][j][2]
            );
            if(d < mini) {
                mini = d;
                result[0] = cont[i][j][0];
                result[1] = cont[i][j][1];
                result[2] = cont[i][j][2];
            }
        }
    }
    return result;
}

Vec3f normaleCalc(Vec3i & pos, Mat & pBef, Mat & pAct, Mat & pNex) {
    Vec3f n;
    float x = pos[0];
    float y = pos[1];

    n[0] = pAct.at<int16_t>(y, x-1) - pAct.at<int16_t>(y, x+1);
    n[1] = pAct.at<int16_t>(y-1, x) - pAct.at<int16_t>(y+1, x);
    n[2] = pBef.at<int16_t>(y, x)   - pNex.at<int16_t>(y, x);
    float d = lengthCalc(n[0], n[1], n[2]);
    // normierung auf 1:
    n[0] = n[0]/d;
    n[1] = n[1]/d;
    n[2] = n[2]/d;
    return n;
}

void create3D(
    vector< vector<Vec3i> > & conturAct,
    vector< vector<Vec3i> > & conturNext,
    Mat & pBef,
    Mat & pAct,
    Mat & pNext,
    bool conturRun = false
) {
    // speicher, um alle zu zeichnende Punkte ab zu legen
    vector<Vec3i> aktuell;
    vector<Vec3i> naechste;
    vector<Vec3f> normale;
                
    for(unsigned int i = 0; i<conturAct.size(); i+=1) {
        for(unsigned int j = 0; j<conturAct[i].size(); j+=JUMP) {
            Vec3i p1 = conturAct[i][j];
            Vec3f norm = normaleCalc(p1, pBef, pAct, pNext);
            Vec3i p2 = nearTo(p1, conturNext);

            aktuell.push_back(p1);
            naechste.push_back(p2);
            normale.push_back(norm);
        }
        //cout<<"contur "<<(i+1)<<" has "<<conturAct[i].size()<<"dots"<<endl;
    }

    if(konturRunner>=aktuell.size()) konturRunner = 0;
    unsigned int endPoint = konturRunner;

    // nur in der Konturzeile, die die "letzte" ist, soll durch die punkte
    // bis zu einem ende iteriert werden!
    if(conturRun == false) endPoint = aktuell.size();

    glBegin(GL_TRIANGLE_STRIP);
    for(unsigned int i = 0; i<endPoint; i++) {
        glColor4ub(255, 0, 0, 128);
        glNormal3f(normale[i][0], normale[i][1], normale[i][2]);
        glVertex3f(aktuell[i][0], aktuell[i][1], aktuell[i][2]);
        glVertex3f(naechste[i][0], naechste[i][1], naechste[i][2]);
        if(conturRun == true && (i+1)==endPoint) {
            cout<<aktuell[i][0]<<","<<aktuell[i][1]<<","<<aktuell[i][2]<<endl;
        }
    }
    glEnd();

    glutPostRedisplay();
    glutSwapBuffers();
}

// 3D Zeichen
void show3D(void) {
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glTranslatef(-256, -256, -100);
    glCallList(listHaut);
    glTranslatef(256, 256, 100);
    glutPostRedisplay();
    glutSwapBuffers();
}

// zum drehen und zoomen der szene
void mouseRotate(int x, int y) {
    static int lastx = 0;
    static int lasty = 0;
    if(x-lastx > 0) glRotatef(-3, 1, 0, 0);
    if(x-lastx < 0) glRotatef( 3, 1, 0, 0);
    if(y-lasty > 0) glRotatef(-3, 0, 0, 1);
    if(y-lasty < 0) glRotatef( 3, 0, 0, 1);
    glutSwapBuffers();
    lastx = x; lasty = y;
}

void keyAction(unsigned char key, int x, int y) {
    if(key == '+') {
        glScalef(1.1, 1.1, 1.1);
    } else if(key == '-') {
        glScalef(0.9, 0.9, 0.9);
    } else if(key == 'c') {
        // in einer Konturebene durch die Punkte laufen
        glNewList(listHaut, GL_COMPILE);
            glEnable(GL_BLEND);
            glDepthMask(GL_FALSE);
            glBlendEquation(GL_FUNC_ADD);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            unsigned int stopZeile = zeile;
            for(unsigned int z=JUMP; z<stopZeile; z+=JUMP) {
                if( (z+JUMP) >= stopZeile)
                    create3D(skinC[z], skinC[z+JUMP], dat[z-JUMP], dat[z], dat[z+JUMP], true);
                else
                    create3D(skinC[z], skinC[z+JUMP], dat[z-JUMP], dat[z], dat[z+JUMP]);
            }
            glDisable(GL_BLEND);
            glDepthMask(GL_TRUE);
        glEndList();
        konturRunner++;
    } else if(key == ' ') {
        // weitere Konturebene zeichnen
        konturRunner = 0;
        glNewList(listHaut, GL_COMPILE);
            glEnable(GL_BLEND);
            glDepthMask(GL_FALSE);
            glBlendEquation(GL_FUNC_ADD);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            unsigned int stopZeile = zeile;
            for(unsigned int z=JUMP; z<stopZeile; z+=JUMP) {
                if( (z+JUMP) >= stopZeile)
                    create3D(skinC[z], skinC[z+JUMP], dat[z-JUMP], dat[z], dat[z+JUMP], true);
                else
                    create3D(skinC[z], skinC[z+JUMP], dat[z-JUMP], dat[z], dat[z+JUMP]);
            }
            glDisable(GL_BLEND);
            glDepthMask(GL_TRUE);
        glEndList();
        zeile+=JUMP;
        if(zeile>=images-JUMP) zeile=1;
    }

    glutPostRedisplay();
}

int main(int argc, char *argv[]) {
    images = STOP - START +1;
    zeile = 1;
    konturRunner = 0;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH);
    glutInitWindowSize(500, 500);
    glutCreateWindow("MRT-Aufnahme");
    listHaut = glGenLists(1);

    // Grauwert-Bilder einlesen
    cout<<"lade Daten"<<endl;
    for(unsigned int z=0; z<images; z++) {
        Mat cpy(SIZE, SIZE, CV_16U);
        stringstream path;
        path << FOLDER << "vhf." << (START + z) << ".dcm";
        //cout<<z<<endl;
        readDcm(path.str(), cpy);
        dat[z] = cpy.clone();
        extractIt(cpy, EXTRACTVALUE);
        // randpixel extrahieren und in vector/array ablegen
        skinC[z] = pointExtract(cpy, z);
    }


    // kameraausschnitt
    glMatrixMode(GL_PROJECTION);
    glOrtho(-300,300, -300,300, -300, 300);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glClearColor(0.5, 0.5, 0.5, 1);
    glEnable(GL_DEPTH_TEST);

    GLfloat white[]= {1,1,1, 1};
    GLfloat posit[]= {-250,250,250, 1};
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
    glLightfv(GL_LIGHT0, GL_POSITION, posit);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glMatrixMode(GL_MODELVIEW);

    glutDisplayFunc(show3D);
    glutMotionFunc(mouseRotate);
    glutKeyboardFunc(keyAction);
    glutMainLoop();
    return 0;
}
