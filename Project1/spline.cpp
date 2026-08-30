#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <iostream>
#include "spline.hpp"
/*
* 3次スプライン曲線の生成
* 参考：https://www5d.biglobe.ne.jp/stssk/maze/spline.html
*/
Spline::Spline(double *p, int num) {

	a = new double[num];
	b = new double[num];
	c = new double[num];
	d = new double[num];
	//std::cout << "num = " << num << std::endl;

	this->n = num - 1;

	int i; // ループ用変数
	// a(i) = y(i)
	for (i=0; i<num; i++) {
		a[i] = p[i];
	}

	// c(i-1) + 4c(i) + c(i-1) = 3(a(i+1) - 2a(i) + a(i-1)) (1 <= i <= n-2)
	// 右辺の計算
	c[0] = c[n] = 0.0;
	for (i=1; i<n; i++) {
		c[i] = 3.0 * (a[i+1] - 2*a[i] + a[i-1]);
	}

	// c(n-1)を求める
	double temp;
	double *w = new double[n]; // tempの逆数
	w[0] = 0.0; // i=0のときは0
	for (i=1; i<n; i++) {
		temp = 4.0 - w[i-1]; // c(i)について解くときのc(i)の係数
		c[i] = (c[i] -c[i-1]) / temp;
		w[i] = 1.0 / temp;
	}
	
	// c(n-1)からc(i)をすべて求める
	for (i=n-1; i>0; i--) {
		c[i] = c[i] - c[i-1]*w[i];
	}

	// d(i), b(i)を求める
	b[n] = d[n] = 0.0;
	for (i=0; i<n; i++) {
		d[i] = (c[i] - c[i+1]) / 3.0;
		b[i] = a[i+1] - a[i] - c[i] - d[i];
	}
}

Spline::~Spline() {
	delete [] a;
	delete [] b;
	delete [] c;
	delete [] d;
}

double Spline::f(double t) {
	int i = (int)floor(t);
	return a[i] + b[i]*(t-i) + c[i]*pow(t-i, 2) + d[i]*pow(t-i, 3);
}


SmoothMove::SmoothMove(double p[][3], int num, double speed, ostop *stop) {
	this->n = num;
	int i;
	double *d = new double[n];

	// xyzそれぞれの座標について、tに対するスプライン曲線を生成
	for (i=0; i<n; i++) d[i] = p[i][0];
	this->x = new Spline(d, n);

	for (i=0; i<n; i++) d[i] = p[i][1];
	this->y = new Spline(d, n);

	for (i=0; i<n; i++) d[i] = p[i][2];
	this->z = new Spline(d, n);

	delete[] d;

	// 各値の初期化
	this->speed = speed;
	this->t = 0.0; // パラメータtの初期値
	this->stop = stop;

	this->now = 0;
	this->prev = 0;

	this->isStop = false;
	this->canStop = stop==NULL ? false : true;
	this->stopNum = -1;
	this->stopTime = 0;
	this->stopAt = NULL;

	this->isEnabled = false;
}

/*
SmoothMove::SmoothMove(double *x, double *y, double *z, int num, double speed) {
	this->n = num;
	this->x = new Spline(x, n);
	this->y = new Spline(y, n);
	this->z = new Spline(z, n);
	this->speed = speed;
	this->t = 0.0;
}
*/

SmoothMove::~SmoothMove() {
	delete x;
	delete y;
	delete z;

	if (stop != NULL) {
		//delete[] stop->at;
		//delete[] stop->hl;
		delete stop;
	}
	std::cout << "Delete SmoothMove instance" << std::endl;
}
void SmoothMove::update() {
	if (!isEnabled) return;

	if (isStop) {
		stopTime = (long)(time(NULL)) - stopAt;
		//std::cout << "hl : " << stop->hl[stopNum] << std::endl;

		if (stopTime > stop->hl[stopNum]) { // 時間が過ぎたら動かす
			std::cout << "stopTime: " << stopTime << std::endl;
			isStop = false;
			canStop = false;
			stopTime = 0;
			this->t = stop->at[stopNum];
		}
	} else {
		this->t += speed;
		if (t > n-1) t -= n-1; // ループするように
		this->now = (int)floor(t);

		// 区画が変わったら止まれるようにする
		if (stop!=NULL && prev!=now) canStop = true;
		this->prev = now;

		// 止まれる場合は止まる
		if (canStop) stopIfNeed();
	}
	//this->t += speed * weight[(int)round(t)]
}
void SmoothMove::stopIfNeed() {
	for (int i=0; i<stop->number; i++) {
		if (now == stop->at[i]) {
			stopNum = i;
			isStop = true;
			stopAt = (long)(time(NULL));
			std::cout << "Stop " << stopNum << std::endl;
			break;
		}
	}
}
double SmoothMove::getX() {
	return x->f(t);
}
double SmoothMove::getY() {
	return y->f(t);
}
double SmoothMove::getZ() {
	return z->f(t);
}

void SmoothMove::switchEnabled() {
	this->isEnabled = !isEnabled;
	this->t = 0.0;
}