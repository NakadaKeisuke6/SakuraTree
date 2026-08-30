#ifdef _WIN32
#include <Windows.h>
#endif // _WIN32
#include <iostream>

#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>
//#include <GL/freeglut.h>
#include <GL/glut.h>
#include <opencv2/opencv.hpp>


#ifdef _MSC_VER
#define _USE_MATH_DEFINES
#endif
#include <math.h>
#include <time.h>


// mqoファイル読み込み用
#include "GLMetaseq.h"


/*
** 光源
*/
static const GLfloat lightpos[] = { -0.5, -1.0, -0.5, 0.0 }; /* 位置　　　 */
static const GLfloat lightcol[] = { 0.2, 0.2, 0.25, 1.0 }; /* 直接光強度 */
static const GLfloat lightamb[] = { 0.2, 0.2, 0.2, 1.0 }; /* 環境光強度 */

/*
** テクスチャ
*/
// 1枚目
int TEXWIDTH1 = 64;                      /* テクスチャの幅　　　 */
int TEXHEIGHT1 = 64;                      /* テクスチャの高さ　　 */
GLuint  texid_1;
static const char texture1[] = ".\\img\\cb.png";  /* テクスチャファイル名 */
// 2枚目
int TEXWIDTH2 = 64;                      /* テクスチャの幅　　　 */
int TEXHEIGHT2 = 64;                      /* テクスチャの高さ　　 */
GLuint  texid_2;
static const char texture2[] = ".\\img\\ground.png";  // テクスチャ（２枚目）
// 3枚目
int TEXWIDTH3 = 512;                      /* テクスチャの幅　　　 */
int TEXHEIGHT3 = 512;                      /* テクスチャの高さ　　 */
GLuint  texid_3;
static const char texture3[] = ".\\img\\sakura2.png";  // テクスチャ（3枚目）

/*
* 視点関連
*/
GLdouble cLoc[3]; // 視点位置
GLdouble dLoc[3]; // 視点方向
GLdouble cUp[3]; // 視点上方向
GLdouble forward[2]; // 進む方向
GLdouble dcLoc[2]; // xz平面上の視点方向と直行する単位ベクトル
GLdouble dTheta[2]; // 視点方向角度
GLdouble move[2] ; // 移動方向制御用
GLdouble vX, vZ; // 視点移動量
double speed; // 移動速度
double sx, sy; // マウスのドラッグ開始位置
double trh, trv; // 視点方向角度の一時計算用変数
int isRotate = 0; // 視点回転操作中か

// 一辺の長さが1の立方体の描画用
GLdouble boxVertex[][3] = {
  { -0.5, -0.5, -0.5 }, /* 0 */
  { 0.5, -0.5, -0.5 }, /* 1 */
  { 0.5, 0.5, -0.5 }, /* 2 */
  { -0.5, 0.5, -0.5 }, /* 3 */
  { -0.5, -0.5, 0.5 }, /* 4 */
  { 0.5, -0.5, 0.5 }, /* 5 */
  { 0.5, 0.5, 0.5 }, /* 6 */
  { -0.5, 0.5, 0.5 }  /* 7 */
};
// 面の情報
int boxFace[][4] = {
  { 0, 1, 2, 3 },
  { 1, 5, 6, 2 },
  { 5, 4, 7, 6 },
  { 4, 0, 3, 7 },
  { 4, 5, 1, 0 },
  { 3, 2, 6, 7 }
};
// 法線の情報
GLdouble boxNormal[][3] = {
  { 0.0, 0.0,-1.0 },
  { 1.0, 0.0, 0.0 },
  { 0.0, 0.0, 1.0 },
  {-1.0, 0.0, 0.0 },
  { 0.0,-1.0, 0.0 },
  { 0.0, 1.0, 0.0 }
};

// 地面の大きさ
const GLdouble fieldSize = 15.0;

// キュー関連
#define QUEUE_OPT 0 // queue.hかqueue.hppのどちらを使うかを設定
#if QUEUE_OPT
#include "queue.h"
#else
#include "queue.hpp" // クラスで実装
#endif

// スプライン曲線
#include "spline.hpp"
ostop *stopOpt;
SmoothMove *autoCamera;
GLdouble c_keypoints[][3] = {{11.0, 1.0, -11.0},
                             {-14.0, 5.0, -14.0},
                             {-11.0, 7.0, 10.0},
                             {12.0, 7.0, 13.0},
                             {14.0, 4.0, -4.0},
                             {11.0, 1.0, -11.0}};
SmoothMove *autoCamTarget;
GLdouble ct_keypoints[][3] = {{0.0, 2.0, 0.0},
                             {0.0, 10.0, 0.0},
                             {5.0, 10.0, 5.0},
                             {0.0, 10.0, 0.0},
                             {0.0, 4.0, 0.0},
                             {0.0, 2.0, 0.0}};
bool isFlyView; // フライビューか

// 桜の花びらに関するもの
#if QUEUE_OPT
queue *cbs;
#else
Queue* cbs;
#endif
int cbs_timer; // 花びらを生成するタイミングを計る用のタイマー
const float TWO_PI = (float)(2.0 * M_PI);
int cb_numbers; // 花びらの枚数

#ifdef _WIN32 // windowsで実行するとき
const double speedWalk = 0.0025; // 視点の移動速度(低速)
const double speedRun = 0.005; // 視点の移動速度(高速)
float vel[3] = { -0.0006f, -0.0015f, -0.0006f }; // 落ちる速度ベクトル
float rv = 0.3f; // 回転速度
const int cb_spawn_span = 70; // 花びらが生成される間隔
float tailRv[] = { 0.015f, 0.02f }; // しっぽの回転速度
static char tree_model[] = ".\\tree.mqo"; // 木のモデルファイルの相対パス
#else // IEDで実行するとき
const double speedWalk = 0.05; // 視点の移動速度(低速)
const double speedRun = 0.1; // 視点の移動速度(高速)
float vel[3] = { -0.012f, -0.035f, -0.012f }; // 落ちる速度ベクトル
float rv = 20.0f; // 回転速度
const int cb_spawn_span = 2; // 花びらが生成される間隔
float tailRv[] = { 0.7f, 0.9f }; // しっぽの回転速度(2体)
static const char tree_model[] = "./tree.mqo"; // 木のモデルファイルの相対パス
#endif

// 木のモデル
MQO_MODEL tree_mqo;

const GLfloat white[] = { 1.0f, 1.0f, 1.0f, 1.0f }; // 白色
const GLfloat zero4f[] = { 0.0f, 0.0f, 0.0f, 1.0f }; // 黒
const GLfloat gray[] = { 0.2f, 0.2f, 0.2f, 1.0f }; // 灰色
const GLfloat darkGray[] = { 0.05f, 0.05f, 0.05f, 1.0f }; // 暗い灰色

float tailR[2]; // しっぽの回転(2体)
int tailDir[2]; // しっぽの向き(2体)


/*
** 初期化
*/
static void init(void)
{
    // 乱数のシード値設定
    srand((unsigned int)time(NULL));

    // キューを作成
#if QUEUE_OPT
    cbs = (queue*)malloc(sizeof(queue));
#else
    cbs = new Queue;
#endif
    if (cbs == NULL) {
        printf("memory error\n"); exit(1);
    }
    //initQueue(cbs);
    cb_numbers = 0; // 花びらの枚数初期化
    cbs_timer = 0;

    /* テクスチャの読み込みに使う配列 */
    /*
    GLubyte texture_buf1[TEXHEIGHT1][TEXWIDTH1][4];
    GLubyte texture_buf2[TEXHEIGHT2][TEXWIDTH2][4];   // ２枚目用に配列を追加
    GLubyte texture_buf3[TEXHEIGHT3][TEXWIDTH3][4];
    FILE* fp;
    */


    // 停止の設定
    stopOpt = new ostop;
    stopOpt->number = 2;
    stopOpt->at = new int[stopOpt->number] {0, 1};
    stopOpt->hl = new int[stopOpt->number] {2, 4};
    // カメラの動きを作成
    autoCamera = new SmoothMove(c_keypoints, (int)(sizeof(c_keypoints)/sizeof(c_keypoints[0])), 0.00005, stopOpt);
    // カメラのターゲットを作成
    autoCamTarget = new SmoothMove(ct_keypoints, (int)(sizeof(ct_keypoints)/sizeof(c_keypoints[0])), 0.00005, stopOpt);

#ifdef _WIN32
    /* テクスチャ画像(1枚目)の読み込み */
    cv::Mat tex1 = cv::imread(texture1, -1);
    std::cout << "[ch.] " << tex1.channels() << std::endl;   // 4ch (αチャネル）が読み込まれているか確認 (ch. = 4ならOK)
    
    TEXWIDTH1 = tex1.size().width;
    TEXHEIGHT1 = tex1.size().height;
    printf("size: %d, %d\n", TEXWIDTH1, TEXHEIGHT1);

    /* テクスチャ画像(2枚目)の読み込み */
    cv::Mat tex2 = cv::imread(texture2, -1);
    std::cout << "[ch.] " << tex2.channels() << std::endl;

    TEXWIDTH2 = tex2.size().width;
    TEXHEIGHT2 = tex2.size().height;
    printf("size: %d, %d\n", TEXWIDTH2, TEXHEIGHT2);

    /* テクスチャ画像(3枚目)の読み込み */
    cv::Mat tex3 = cv::imread(texture3, -1);
    std::cout << "[ch.] " << tex3.channels() << std::endl;

    TEXWIDTH3 = tex3.size().width;
    TEXHEIGHT3 = tex3.size().height;
    printf("size: %d, %d\n", TEXWIDTH3, TEXHEIGHT3);

#else
    if ((fp = fopen(texture1, "rb")) != NULL) {
        fread(texture_buf1, sizeof texture_buf1, 1, fp);
        fclose(fp);
}
    else {
        perror(texture1);
    }

    /* テクスチャ画像(2枚目)の読み込み */
    if ((fp = fopen(texture2, "rb")) != NULL) {
        fread(texture_buf2, sizeof texture_buf2, 1, fp);
        fclose(fp);
    }
    else {
        perror(texture2);
    }

    /* テクスチャ画像(3枚目)の読み込み */
    if ((fp = fopen(texture3, "rb")) != NULL) {
        fread(texture_buf3, sizeof texture_buf3, 1, fp);
        fclose(fp);
    }
    else {
        perror(texture3);
    }
#endif



    ////////// 1枚目のテクスチャの読み込み ////////////

    glGenTextures(1, &texid_1);  // テクスチャIDを生成
    glBindTexture(GL_TEXTURE_2D, texid_1);  // 生成したテクスチャIDに切り替える

    /* テクスチャ画像はバイト単位に詰め込まれている */
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    /* テクスチャの割り当て */
    glTexImage2D(GL_TEXTURE_2D, 0, 4, TEXWIDTH1, TEXHEIGHT1, 0, GL_BGRA, GL_UNSIGNED_BYTE, tex1.data);

    /* テクスチャを拡大・縮小する方法の指定 */
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);   // パラメータ関連の設定
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    /* テクスチャ環境 */
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    glBindTexture(GL_TEXTURE_2D, 0);  // デフォルトにテクスチャIDに切り替える
    ////////////////////////////////////////////////////	


     ////////// 2枚目のテクスチャの読み込み ////////////

    glGenTextures(1, &texid_2);  // テクスチャIDを生成
    glBindTexture(GL_TEXTURE_2D, texid_2);  // 生成したテクスチャIDに切り替える

    /* テクスチャ画像はバイト単位に詰め込まれている */
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    /* テクスチャの割り当て */
    glTexImage2D(GL_TEXTURE_2D, 0, 4, TEXWIDTH2, TEXHEIGHT2, 0, GL_BGRA, GL_UNSIGNED_BYTE, tex2.data);

    /* テクスチャを拡大・縮小する方法の指定 */
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);   // パラメータ関連の設定
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    /* テクスチャ環境 */
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glBindTexture(GL_TEXTURE_2D, 0);  // デフォルトにテクスチャIDに切り替える
    ////////////////////////////////////////////////////	


    ////////// 3枚目のテクスチャの読み込み ////////////

    glGenTextures(1, &texid_3);  // テクスチャIDを生成
    glBindTexture(GL_TEXTURE_2D, texid_3);  // 生成したテクスチャIDに切り替える

    /* テクスチャ画像はバイト単位に詰め込まれている */
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    /* テクスチャの割り当て */
    glTexImage2D(GL_TEXTURE_2D, 0, 4, TEXWIDTH3, TEXHEIGHT3, 0, GL_BGRA, GL_UNSIGNED_BYTE, tex3.data);

    /* テクスチャを拡大・縮小する方法の指定 */
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);   // パラメータ関連の設定
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    /* テクスチャ環境 */
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    glBindTexture(GL_TEXTURE_2D, 0);  // デフォルトにテクスチャIDに切り替える
    ////////////////////////////////////////////////////	


#if 0
/* 混合する色の設定 */
    static const GLfloat blend[] = { 0.0, 1.0, 0.0, 1.0 };
    glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, blend);
#endif

    /* アルファテストの比較関数 */
    glAlphaFunc(GL_GREATER, 0.5);

    /* 初期設定 */
    glClearColor(0.25, 0.4, 0.49, 0.0);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    /* 光源の初期設定 */
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightcol);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightcol);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightamb);

    // 視点の位置の初期値
    cLoc[0] = 1.0;  cLoc[2] = 0.0; cLoc[1] = 1.0;
    // 視点の向き関連の初期設定
    dLoc[0] = 1.0; dLoc[1] = 0.0; dLoc[2] = 0.0;
    dcLoc[0] = 0.0; dcLoc[1] = 1.0;
    cUp[0] = 0.0; cUp[1] = 1.0; cUp[2] = 0.0;
    forward[0] = 1.0; forward[1] = 0.0;
    vX = vZ = 0.0;
    dTheta[0] = 0.0; dTheta[1] = M_PI_2;
    move[0] = move[1] = 0.0;
    // 視点の移動速度
    speed = speedWalk;
    // 一時変数の初期化
    trv = trh = 0.0;

    // しっぽの動きに関する初期設定
    tailR[0] = 0.0f;
    tailR[1] = 20.0f;
    tailDir[0] = 1;
    tailDir[1] = 1;
}

/*
* 桜の花びら関連
*/
// 花びらを描く
static void cherryBlossam(void) {
    glNormal3d(0.0, 0.0, 1.0);
    glBegin(GL_QUADS);
    glTexCoord2d(0.0, 1.0);
    glVertex3d(-0.3, -0.3, 0.0);
    glTexCoord2d(1.0, 1.0);
    glVertex3d(0.3, -0.3, 0.0);
    glTexCoord2d(1.0, 0.0);
    glVertex3d(0.3, 0.3, 0.0);
    glTexCoord2d(0.0, 0.0);
    glVertex3d(-0.3, 0.3, 0.0);
    glEnd();
}
// 新しく花びらを追加（キューに加える）
static void addCB(void) {
    float loc[3] = {0};
    loc[0] = (((float)rand() / (float)RAND_MAX) * 30 - 8);
    loc[1] = 20.0f;
    loc[2] = (((float)rand() / (float)RAND_MAX) * 30 - 8);
#if QUEUE_OPT
    enqueue(cbs, loc, 0.0f, vel);
#else
    cbs->enqueue(loc, 0.0f, vel);
#endif
    //printf("%f, %f\n", loc[0], loc[2]);
}
// キューが持っている花びらの情報をすべて描く
static void drawCBs(void) {
#if QUEUE_OPT
    if (cbs == NULL || isEmpty(cbs)) return; // キューが空の場合は描画しない
    if (cbs->head->data->loc[1] < -1.0) dequeue(cbs); // 花びらが見えない位置に来たら消す
    onode *p = cbs->head;
#else
    if (cbs == NULL || cbs->isEmpty()) return; // キューが空の場合は描画しない
    if (cbs->getHeadLocY() < -1.0) cbs->dequeue(); // 花びらが見えない位置に来たら消す
    onode* p = cbs->getData();
#endif
    object* o;
    while (p != NULL) {
        o = p->data; // プログラム省略のため
        p = p->next;
        if (o == NULL) continue; // (念のため)データがない場合は飛ばす
        for (int i = 0; i < 3; i++) {
            o->loc[i] += o->v[i]; // 移動させる
        }
        if ((o->rotate += rv) > 360) o->rotate = 0.0f; // 回転させる
        
        //printf("%f\n", o->rotate);
        //printf("%f, %f, %f\n", o->loc[0], o->loc[1], o->loc[2]);
        
        // 位置と回転の情報をもとに描画する
        glPushMatrix();
            glTranslatef(o->loc[0], o->loc[1], o->loc[2]);
            glRotatef(o->rotate, 1.0f, 1.0f, 1.0f);
            cherryBlossam();
        glPopMatrix();
    }
}

/*
* 地面
*/
static void ground(void) {
    /* １枚の４角形を描く */
    GLdouble gSize = fieldSize + 0.5;
    glNormal3d(0.0, 0.0, 1.0);
    glBegin(GL_QUADS);
    glTexCoord2d(0.0, gSize);
    glVertex3d(-gSize, 0.0, -gSize);
    glTexCoord2d(gSize, gSize);
    glVertex3d(gSize, 0.0, -gSize);
    glTexCoord2d(gSize, 0.0);
    glVertex3d(gSize, 0.0, gSize);
    glTexCoord2d(0.0, 0.0);
    glVertex3d(-gSize, 0.0, gSize);
    glEnd();
}

/*
* 桜の花
*/
static void sakura(void) {
    glNormal3d(0.0, 0.0, 1.0);
    glBegin(GL_QUADS);
    glTexCoord2d(0.0, 1.0);
    glVertex3d(-40.0, 0.0, -40.0);
    glTexCoord2d(1.0, 1.0);
    glVertex3d(40.0, 0.0, -40.0);
    glTexCoord2d(1.0, 0.0);
    glVertex3d(40.0, 0.0, 40.0);
    glTexCoord2d(0.0, 0.0);
    glVertex3d(-40.0, 0.0, 40.0);
    glEnd();
}
// 桜の花を位置や回転、大きさを変えていくつか複製
static void createSakura(void) {
    glPushMatrix();
        glTranslatef(5.0f, 26.0f, 5.0f);
        sakura();
        
        glPushMatrix();
            glTranslatef(0.6f, 4.0f, 0.6f);
            glRotatef(90.0f, 0.1f, 1.0f, 0.1f);
            sakura();
        glPopMatrix();
        glPushMatrix();
            glTranslatef(-0.6f, 9.0f, -0.6f);
            glRotatef(-90.0f, 0.25f, 1.0f, 0.25f);
            sakura();
        glPopMatrix();
        glPushMatrix();
            glTranslatef(-0.6f, 9.0f, -0.6f);
            glRotatef(90.0f, -0.25f, 1.0f, 0.25f);
            sakura();
        glPopMatrix();
        glPushMatrix();
            glTranslatef(-0.6f, 9.0f, -0.6f);
            glRotatef(90.0f, 0.25f, 1.0f, -0.25f);
            sakura();
        glPopMatrix();
        glPushMatrix();
            glTranslatef(0.0f, 14.0f, -0.6f);
            glRotatef(70.0f, 0.2f, 1.0f, -0.2f);
            glScalef(0.9f, 0.9f, 0.9f);
            sakura();
        glPopMatrix();
        glPushMatrix();
            glTranslatef(0.0f, 20.0f, 0.0f);
            glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
            sakura();
        glPopMatrix();
    glPopMatrix();
}

// 大きさと位置を指定して箱を描画する
static void box(double sx, double sy, double sz, double tx, double ty, double tz) {
    glPushMatrix();
        glTranslated(tx, ty, tz);
        glScaled(sx, sy, sz);

        glBegin(GL_QUADS);
        for (int j = 0; j < 6; j++) {
            glNormal3dv(boxNormal[j]);
            for (int i = 0; i < 4; i++) {
                glVertex3dv(boxVertex[boxFace[j][i]]);
            }
        }
        glEnd();
    glPopMatrix();
    //printf("create at (%.2f, %.2f, %.2f)\n", tx, ty, tz);
}
// 上のboxの中心を変更したもの
static void box2(double sx, double sy, double sz, double tx, double ty, double tz) {
    glPushMatrix();
    glTranslated(tx, ty, tz);
    glScaled(sx, sy, sz);

    glBegin(GL_QUADS);
    for (int j = 0; j < 6; j++) {
        glNormal3dv(boxNormal[j]);
        for (int i = 0; i < 4; i++) {
            glVertex3d(boxVertex[boxFace[j][i]][0]+0.5, boxVertex[boxFace[j][i]][1]-0.5, boxVertex[boxFace[j][i]][2]);
        }
    }
    glEnd();
    glPopMatrix();
    //printf("create at (%.2f, %.2f, %.2f)\n", tx, ty, tz);
}
/*
* 柵関連
*/
// 柵を描画する
static void fence(void) {
    int size = (int)fieldSize;

    // 柵の材質設定
    static const GLfloat colorFence[] = { 0.6f, 0.5f, 0.4f, 1.0f };  // フェンスの色
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, colorFence);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, zero4f);
    glMaterialfv(GL_FRONT, GL_SPECULAR, darkGray);
    glMaterialf(GL_FRONT, GL_SHININESS, 0.05f);

    // 支柱
    for (int j = 0; j < 4; j++) {
        for (int i = -size; i < size; i++) {
            switch (j) {
                case 0:
                    box(0.2, 1.0, 0.2, i, 0.5, -size);
                    break;
                case 1:
                    box(0.2, 1.0, 0.2, -i, 0.5, size);
                    break;
                case 2:
                    box(0.2, 1.0, 0.2, -size, 0.5, -i);
                    break;
                case 3:
                    box(0.2, 1.0, 0.2, size, 0.5, i);
                    break;
                default:
                    break;
            }
        }
    }
    // 横板
    box(size << 1, 0.2, 0.075, 0.0, 0.7, size);
    box(size << 1, 0.2, 0.075, 0.0, 0.7, -size);
    box(0.075, 0.2, size << 1, size, 0.7, 0.0);
    box(0.075, 0.2, size << 1, -size, 0.7, 0.0);
}

/*
* 動物関連
*/
const GLfloat colorFur[] = { 0.55f, 0.48f, 0.4f, 1.0f };  // 毛の色
// 頭
static void headA(void) {
    static const GLfloat colorEye[] = { 0.05f, 0.05f, 0.0f, 1.0f };  // 目の色
    
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, colorFur);
    glMaterialfv(GL_FRONT, GL_SPECULAR, darkGray);
    glMaterialf(GL_FRONT, GL_SHININESS, 0.1f);
    box(1.1, 1.0, 0.9, 0.0, 0.0, 0.0);
    box(0.75, 0.6, 0.55, 0.7, -0.15, 0.0);

    // 耳
    glPushMatrix();
        glRotated(15.0, 1.0, 1.0, 1.0);
        box(0.8, 0.3, 0.15, -0.8, 0.4, 0.4);
    glPopMatrix();
    glPushMatrix();
        glRotated(-15.0, 1.0, 1.0, -1.0);
        box(0.8, 0.3, 0.15, -0.8, 0.4, -0.4);
    glPopMatrix();

    // 目
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, colorEye);
    glMaterialfv(GL_FRONT, GL_SPECULAR, gray);
    glMaterialf(GL_FRONT, GL_SHININESS, 1.0f);
    box(0.2, 0.2, 0.2, 0.5, 0.05, 0.4);
    box(0.2, 0.2, 0.2, 0.5, 0.05, -0.4);

    // 鼻
    glMaterialf(GL_FRONT, GL_SHININESS, 0.3f);
    box(0.2, 0.2, 0.25, 1.05, 0.1, 0.0);

    // ハイライト
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
    glMaterialfv(GL_FRONT, GL_EMISSION, white);
    box(0.1, 0.1, 0.1, 0.58, 0.08, 0.35);
    box(0.1, 0.1, 0.1, 0.58, 0.08, -0.35);
    glMaterialfv(GL_FRONT, GL_EMISSION, zero4f);
}
// 体
static void bodyA(void) {
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, colorFur);
    glMaterialfv(GL_FRONT, GL_SPECULAR, darkGray);
    glMaterialf(GL_FRONT, GL_SHININESS, 0.1f);

    box(2.0, 1.0, 0.9, 0.0, 0.5, 0.0);
    box(0.4, 0.7, 0.35, 0.78, -0.2, 0.25);
    box(0.4, 0.7, 0.35, 0.78, -0.2, -0.25);
    box(0.4, 0.7, 0.35, -0.78, -0.2, 0.25);
    box(0.4, 0.7, 0.35, -0.78, -0.2, -0.25);


    static const GLfloat colorClaw[] = { 0.3f, 0.3f, 0.3f, 1.0f };  // 爪の色
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, colorClaw);
    glMaterialfv(GL_FRONT, GL_SPECULAR, gray);
    glMaterialf(GL_FRONT, GL_SHININESS, 0.8f);
    box(0.45, 0.1, 0.32, 0.83, -0.5, 0.25);
    box(0.45, 0.1, 0.32, 0.83, -0.5, -0.25);
    box(0.45, 0.1, 0.32, -0.73, -0.5, 0.25);
    box(0.45, 0.1, 0.32, -0.73, -0.5, -0.25);
}
// しっぽ
static void tailA(void) {
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, colorFur);
    glMaterialfv(GL_FRONT, GL_SPECULAR, darkGray);
    glMaterialf(GL_FRONT, GL_SHININESS, 0.1f);
    glPushMatrix();
        glRotated(-30.0, 0.0, 0.0, 1.0);
        box2(0.25, 0.8, 0.25, -1.25, 0.0, 0.0);
    glPopMatrix();
}
// 全身
static void animal(int i) {
    glPushMatrix();
        glTranslated(0.7, 1.0, 0.0);
        glScaled(0.45, 0.45, 0.45);
        headA();
    glPopMatrix();
    glPushMatrix();
        glTranslated(0.0, 0.4, 0.0);
        glScaled(0.6, 0.6, 0.6);
        bodyA();
    glPopMatrix();
    glPushMatrix();
        glTranslated(0.0, 0.6, 0.0);
        glRotated(tailR[i], 1.0,-0.2, 0.0);
        glScaled(0.6, 0.6, 0.6);
        tailA();
    glPopMatrix();
    // しっぽを動かす
    if (tailDir[i] == 1) tailR[i] += tailRv[i];
    else              tailR[i] -= tailRv[i];
    if (tailR[i] > 25) {
        tailDir[i] = -1;
    } else if (tailR[i] < -25) {
        tailDir[i] = 1;
    }
}

/*
* 木の幹
*/
static void tree(void) {
    glPushMatrix();
        glTranslated(5.0, 0.0, 5.0);
        mqoCallModel(tree_mqo);
    glPopMatrix();
}

/*
** シーンの描画
*/
static void scene(void)
{
    static const GLfloat emission1[] = { 0.8f, 0.6f, 0.6f, 1.0f }; // 桜を発光させる
    /* 材質の設定 */
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);

    /* アルファテスト開始 */
    glEnable(GL_ALPHA_TEST);

    /* テクスチャマッピング開始 */
    glEnable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, texid_2);  // テクスチャID=2を指定

    ground();

    glBindTexture(GL_TEXTURE_2D, texid_1);  // テクスチャIDをデフォルトに戻す

    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission1);

    glBindTexture(GL_TEXTURE_2D, texid_1);  // テクスチャID=1(桜)を指定

    drawCBs();

    glBindTexture(GL_TEXTURE_2D, texid_1);  // テクスチャIDをデフォルトに戻す

    glBindTexture(GL_TEXTURE_2D, texid_3);  // テクスチャID=3を指定

    createSakura();
    glPushMatrix();
        glScalef(0.4f, 0.4f, 0.4f);
        glTranslatef(0.0f, 0.005f, 0.0f);
        sakura();
    glPopMatrix();

    glBindTexture(GL_TEXTURE_2D, texid_1);  // テクスチャIDをデフォルトに戻す


    /* テクスチャマッピング終了 */
    glDisable(GL_TEXTURE_2D);


    /* アルファテスト終了 */
    glDisable(GL_ALPHA_TEST);

    // 柵作成
    fence();

    // 動物(1体目)
    glPushMatrix();
        glTranslated(-1.0, 0.0, 0.5);
        glRotated(-30.0, 0.0, 1.0, 0.0);
        glScaled(1.3, 1.3, 1.3);
        animal(0);
    glPopMatrix();
    // 動物(2体目)
    glPushMatrix();
        glTranslated(-1.9, 0.0, -1.5);
        glRotated(-75.0, 0.0, 1.0, 0.0);
        glScaled(0.5, 0.5, 0.5);
        animal(1);
    glPopMatrix();

    // 木を描画
    tree();
    
    GLdouble cx = autoCamera->getX();
    GLdouble cy = autoCamera->getY();
    GLdouble cz = autoCamera->getZ();
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
    //box(1.0, 1.0, 1.0, cx, cy, cz);
    autoCamera->update();
    autoCamTarget->update();
}

/*
* 視点関連
*/
// 視点回転操作後の視点方向を計算
static void eyeRotate(double rh, double rv) {
    trh = dTheta[0] + rh / 300.0;
    trv = dTheta[1] + rv / 300.0;

    // 垂直方向の視点が範囲内に収まるようにする
    if (trv < 0) trv = 0;
    else if (trv > M_PI) trv = M_PI;

    // 視点のターゲット位置を計算(3次元極座標)
    dLoc[0] = sin(trv) * cos(trh);
    dLoc[1] = cos(trv);
    dLoc[2] = sin(trv) * sin(trh);
    // 視線と直交するベクトルの計算
    dcLoc[0] = -sin(trh);
    dcLoc[1] = cos(trh);
    // 進む方向
    forward[0] = cos(trh);
    forward[1] = sin(trh);
    // 視点上方向
    cUp[0] = sin(trv-M_PI_2) * cos(trh);
    cUp[1] = cos(trv-M_PI_2);
    cUp[2] = sin(trv-M_PI_2) * sin(trh);
}
// 左マウスボタンでドラッグしているとき
static void lMouseMotion(int x, int y) {
    if (isRotate) {
        eyeRotate(x - sx, y - sy);
    }
}
// 左マウスボタンを押したとき
static void lMouseDown(int x, int y) {
    // ドラッグ開始位置を記録
    sx = x;
    sy = y;
    isRotate = 1;
}
// 左マウスボタンを離したとき
static void lMouseUp() {
    isRotate = 0;
    dTheta[0] = trh;
    dTheta[1] = trv;
}

/****************************
** GLUT のコールバック関数 **
****************************/


static void display(void)
{
    /* モデルビュー変換行列の初期化 */
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    /* 光源の位置を設定 */
    glLightfv(GL_LIGHT0, GL_POSITION, lightpos);

    if (isFlyView) {
        gluLookAt(autoCamera->getX(), autoCamera->getY(), autoCamera->getZ(), autoCamTarget->getX(), autoCamTarget->getY(), autoCamTarget->getZ(), 0.0, 1.0, 0.0);
    }
    else {
        // 視点の移動
        //前後
        cLoc[0] += move[0] * forward[0];
        cLoc[2] += move[0] * forward[1];
        //左右
        cLoc[0] += move[1] * dcLoc[0];
        cLoc[2] += move[1] * dcLoc[1];

        // 移動を制限
        GLdouble mr = fieldSize - 0.7;
        if (cLoc[0] > mr) cLoc[0] = mr;
        else if (cLoc[0] < -mr) cLoc[0] = -mr;
        if (cLoc[2] > mr) cLoc[2] = mr;
        else if (cLoc[2] < -mr) cLoc[2] = -mr;

        // 視点の設定
        gluLookAt(cLoc[0], cLoc[1], cLoc[2], cLoc[0] + dLoc[0], cLoc[1] + dLoc[1], cLoc[2] + dLoc[2], cUp[0], cUp[1], cUp[2]);
    }

    /* 画面クリア */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 一定時間経ったら花びらを追加
    if ((++cbs_timer) > cb_spawn_span) {
        cbs_timer = 0;
        addCB();
        //printf("timer\n");
    }

    /* シーンの描画 */
    scene();

    /* ダブルバッファリング */
    glutSwapBuffers();
}

static void resize(int w, int h) {

    /* ウィンドウ全体をビューポートにする */
    glViewport(0, 0, w, h);

    /* 透視変換行列の指定 */
    glMatrixMode(GL_PROJECTION);

    /* 透視変換行列の初期化 */
    glLoadIdentity();
    gluPerspective(60.0, (double)w / (double)h, 0.5, 100.0);
}

static void idle(void)
{
    /* 画面の描き替え */
    glutPostRedisplay();
}

// 終了時の処理
static void exitScene(void) {
    printf("Exit scene\n");
    autoCamera->switchEnabled();
    mqoDeleteModel(tree_mqo);
    mqoCleanup();
#if QUEUE_OPT
    freeQueue(cbs); // キューで確保したメモリをすべて解放
#else
    delete cbs;
    printf("step 1\n");
    delete autoCamera;
#endif
    exit(0);
}

static void mouse(int button, int state, int x, int y)
{
    switch (button) {
        case GLUT_LEFT_BUTTON:
            switch (state) {
            case GLUT_DOWN:
                lMouseDown(x, y);
                break;
            case GLUT_UP:
                lMouseUp();
                break;
            default:
                break;
            }
            break;
        default:
            break;
    }
}

static void motion(int x, int y)
{
    lMouseMotion(x, y);
}

static void keyboard(unsigned char key, int x, int y)
{
    // SHIFTが押されているかどうかで移動速度を調整
    if (glutGetModifiers() & GLUT_ACTIVE_SHIFT) {
        speed = speedRun;
    }
    else {
        speed = speedWalk;
    }

    // 各キーが押された時の操作
    switch (key) {
        case 'w':
        case 'W':
            move[0] = speed; // 前
            break;
        case 's':
        case 'S':
            move[0] = -speed; // 後ろ
            break;
        case 'a':
        case 'A':
            move[1] = -speed; // 左
            break;
        case 'd':
        case 'D':
            move[1] = speed; // 右
            break;
        case 'l':
        case 'L':
            printf("Position: (%.2f, %.2f, %.2f)\n", cLoc[0], cLoc[1], cLoc[2]);
            break;
        case 'v':
        case 'V':
            autoCamTarget->switchEnabled();
            autoCamera->switchEnabled();
            isFlyView = !isFlyView;
            break;
        case 'q':
        case 'Q':
        case '\033':
            /* ESC か q か Q をタイプしたら終了 */
            exit(0);
        default:
            break;
    }
}

static void keyboardUp(unsigned char key, int x, int y) {
    switch (key) {
        case 'w':
        case 'W':
        case 's':
        case 'S':
            // 前後の移動をやめる
            vX = vZ = 0.0;
            move[0] = 0;
            break;
        case 'a':
        case 'A':
        case 'd':
        case 'D':
            // 左右の移動をやめる
            vX = vZ = 0.0;
            move[1] = 0;
            break;
        default:
            break;
    }

}

/*
** メインプログラム
*/
int main(int argc, char* argv[])
{
    glutInitWindowSize(640, 640);
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    glutCreateWindow("The Rain of Sakura");

    GLenum err = glewInit();

    // mqoファイル読み込み
    mqoInit();
    tree_mqo = mqoCreateModel(tree_model, 0.7);

    // 終了時の処理関数の指定
    atexit(exitScene);

    // コールバック関数の設定
    glutDisplayFunc(display);
    glutReshapeFunc(resize);
    glutIdleFunc(idle);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);

    init();
    glutMainLoop();
    return 0;
}