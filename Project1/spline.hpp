#include <iostream>

/*
*  スプライン曲線
*/
class Spline {
private:
	// y = a + b(x - xi) + c(x - xi)^2 + d(x - xi)^3
	double *a, *b, *c, *d;
	int n; // 関数の数（制御点の個数-1）
	//double *weight;
public:
	Spline(double *p, int n); // コンストラクタ
	~Spline(); // デストラクタ
	double f(double t); // 値を取得
};

typedef struct {
	int *at; // 止まる区間
	int *hl; // 何秒止まるか
	int number; // 止まる区間数
} ostop;

class SmoothMove {
private:
	Spline *x, *y, *z; // xyzのスプライン曲線
	ostop *stop; // 停止の動作が保持されている
	double speed; // 動く速さ
	double t; // xyzのパラメータt
	int now, prev; // 現在とひとつ前の区間
	int n; // 制御点の数
	bool isStop, canStop; // 止まってるか、止められるか
	int stopNum; // 止まっている区間
	long stopAt; // 止まった時点
	long stopTime; // 止まっている時間（秒）
	bool isEnabled; // 有効のときにtrue

	void stopIfNeed(); // 必要なら止める
public:
	// コンストラクタ
	SmoothMove(double p[][3], int num, double speed, ostop *stop);
	//SmoothMove(double *x, double *y, double *z, int num, double speed);
	~SmoothMove(); // デストラクタ
	void update(); // オブジェクトの位置を更新
	double getX(); // 座標を取得
	double getY();
	double getZ();
	void switchEnabled();
};