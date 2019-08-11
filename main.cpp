/*
	コンセプト：敵の攻撃を耐え、耐えた数の攻撃力で敵にダメージを与えて、ボスを倒すゲーム。

	ゲームフロート
	①タイトル画面→②雑魚敵面(2面分)→③ボス(攻撃力初期値　１ * 途中で雑魚的の攻撃を耐えた分の倍率)↓
					↪ここで攻撃力を蓄える(倍率制)
					↓
					④ゲームクリア画面(スコア表示)

	②、③のステージチェンジアニメーション
	FeedOunt→FeedIn

	タイトル画面アニメーション概要
	・タイトル表示の点滅
	・PressEnterの点滅


	ゲーム中の画面UI
	①現在の攻撃倍率
	②耐えた量
	③スコア表示
	④現在時間

	ボス戦での基本概要
	①Fase制度(一ターン毎に現在の攻撃力のダメージをボスに与える)
	②HPが半分辺りまで減れば、ステージチェンジ(背景を禍々しくする)
	(実装できれば)③ボスのスキルにより、プレイヤー周辺の状況しか確認できないようになる
	(透明度により、数フレーム後に明るくなり、更に暗くなる画面処理)


	ボスの攻撃パターンは決められているので、数プレイで攻略可能(ボス２面は鬼畜仕様(時間があれば))




	データ構造

	PlayerX 320
	PlayerY 180




*/


#include<windows.h>
#include<stdio.h>
#include<d3dx9.h>
#include<time.h>

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "winmm.lib")



//プロトタイプ宣言
void LoadTexture(LPDIRECT3DTEXTURE9 *lpTex, const char fname[], int W, int H, D3DCOLOR Color);
//基本図形
void CircleFill(float X, float Y, int SZ, int PN, int CN, D3DCOLOR Color);
void DrawCircle(float X, float Y, int SZ, int PN, int CN, D3DCOLOR Color);
void FillBox(float X, float Y, int Width, int Height, D3DCOLOR Color);
void FillTriangle(float X, float Y, float XX, float YY, float XZ, float YZ, D3DCOLOR Color);
void DrawBox(float X, float Y, int Width, int Height, D3DCOLOR Color);
void DrawTriangle(float X, float Y, float XX, float YY, float XZ, float YZ, D3DCOLOR Color);
void DrawLine(float X, float Y, float XX, float YY, D3DCOLOR Color);
void DrawPixel(float X, float Y, D3DCOLOR Color);
//
void debug(void);
void CreateConsole();
//ゲームフロー
void Title_Init(void);
void Title_Update(void);
void Title_Render(void);
void Game_Init(void);
void Game_Update(void);
void Game_Render(void);
//



void define()
{

	//初期書
#define	SCRW		640	// ウィンドウ幅（Width
#define	SCRH		360	// ウィンドウ高さ（Height

#define	FVF_VERTEX (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1)
////

	//自前制御
#define StageMax		3		//ステージ数(チュートリアル,ボス前中ボスなど,ボス戦)
#define EnemyMax		100		//敵(棒)最大数(仮)
#define EnemyData		4		//敵(棒)のデータ数
#define EnemyDataEnd	99999	//敵(棒)のデータ最終値
////
	//基本図形(守護)
#define BK			FillBox(0, 0, SCRW, SCRH, D3DCOLOR_ARGB(255, 0, 0, 0))	//背景(仮黒)
#define GuardUP		DrawLine(270, 120, 370, 120, D3DCOLOR_ARGB(255, 255, 255, 255))			//プレイヤー守護上
#define	GuardDOWN	DrawLine(270, 240, 370, 240, D3DCOLOR_ARGB(255, 255, 255, 255))			//プレイヤー守護下
#define GuardLEFT	DrawLine(260, 130, 260, 230, D3DCOLOR_ARGB(255, 255, 255, 255))			//プレイヤー守護左
#define GuardRIGHT	DrawLine(380, 130, 380, 230, D3DCOLOR_ARGB(255, 255, 255, 255))			//プレイヤー守護右
////

	//基本図形(敵棒)
#define ENemyUP		DrawBox(290,EnemyUPY , 60, 20, D3DCOLOR_ARGB(255, 255, 255, 255))				//敵棒上
#define ENemyDOWN	DrawBox(290,EnemyDOWNY, 60, 20, D3DCOLOR_ARGB(255, 255, 255, 255))				//敵棒下
#define ENemyLEFT	DrawBox(EnemyLEFTX, 150, 20, 60, D3DCOLOR_ARGB(255, 255, 255, 255))				//敵棒左
#define	ENemyRIGHT	DrawBox(EnemyRIGHTX, 150, 20, 60, D3DCOLOR_ARGB(255, 255, 255, 255))			//敵棒右
////

	//当たり判定フラグ

////

	//フラグの疑似bool
#define T		1		//True
#define F		0		//false
////

	//正規bool
#define Tr		true	//true
#define Fl		false	//false

	//フラグの共用数値変更判別
#define Flg		0		//共用フラグ内容(初期値0)
#define EXIT	999		//フラグぶっ飛ばし用
////
}

//======================================
//bool型制御(コンソール管理、デバッグモード管理)
//======================================
bool gameFullScreen;	// フルスクリーン（true,false)
bool Con = Tr;		// コンソールモード(true,false)
bool Debug = Tr;		// デバッグモード(true,false)



LPDIRECT3D9				lpD3D;		// Direct3Dインターフェイス

LPDIRECT3DDEVICE9		lpD3DDevice;	// Direct3DDeviceインターフェイス

D3DPRESENT_PARAMETERS d3dpp;


/*=====================================================================*/
//構造体

// 頂点フォーマットの定義
struct VERTEX
{
	D3DXVECTOR3 Pos;
	D3DCOLOR Color;
	D3DXVECTOR2 Tex;
};

struct Vertex2D
{
	float x;
	float y;
	float z;
	float rhw;
	D3DCOLOR Color;
};

typedef struct EnemyUP		//敵上
{
	float X;
	float Y;
	float XX;
	float YY;
	float Color;
	int	  Type;
	int   EUFlg;
}EnemyUP;

typedef struct EnemyDOWN		//敵下
{
	float X;
	float Y;
	float XX;
	float YY;
	float Color;
	int   Type;
	int   EDFlg;
}EnemyDOWN;

typedef struct EnemyLEFT		//敵左
{
	float X;
	float Y;
	float XX;
	float YY;
	float Color;
	int   Type;
	int   ELFlg;
}EnemyLEFT;

typedef struct EnemyRIGHT		//敵右
{
	float X;
	float Y;
	float XX;
	float YY;
	float Color;
	int   Type;
	int   ERFlg;
}EnemyRIGHT;

typedef struct EnemyAllData
{
	DWORD time;		//敵の出現時間
	int type;		//敵のタイプ(1.上　2.下　3.左　4.右)
	int EFlg;		//出現時のフラグ管理
}EAD;


/*=====================================================================*/



////  グローバル変数宣言

//プレイヤー
float PlayerX;		//マウスのX値収納
float PlayerY;		//マウスのY値収納
int	  PlayerRemain;	//プレイヤー残機

//守護対象危険信号変化
static int GDB = 255;
static int GDR = 0;

//守護盾フラグ
int GuardUPFlg;		//ガード上フラグ
int GuardDOWNFlg;	//ガード下フラグ
int GuardLREFTFlg;	//ガード左フラグ
int GuardRIGHTFlg;	//ガード右フラグ

//敵(敵棒)

float EnemyUPY;
float EnemyDOWNY;
float EnemyLEFTX;
float EnemyRIGHTX;

static DWORD EnemyTime;
static EAD	 EnemyAllData[EnemyDataEnd];
static int	 EnemyCnt;


////マウス座標
POINT Mouse;		//マウスの座標を保持
HWND hwnd;			//WinMainから切り取り
float MousePosX;	//マウスの現在Xポジションを保持
float MousePosY;	//マウスの現在Yポジションを保持


/*====================================*/
//Alfa値
int Ti;
int AlfaFlg;
int Flame;
/*====================================*/

/*=============================*/
//場面管理
int Scene;
/*(0 = タイトル 
   1 = ゲーム本編)*/
//プレイ中場面制御
static int GameNumBer;
/*(1 = チュートリアル
   2 = 本編
   3 = ボス戦)*/

LPD3DXSPRITE lpSprite;	// スプライト
LPD3DXFONT lpFont;		// フォント



#define	FVF_VERTEX (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1)

//指定座標に点を書く関数
void DrawPixel(float X, float Y, D3DCOLOR Color)
{
	struct Vertex2D vertex[] =
	{ X,Y,0,1,Color };
	lpD3DDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);

	lpD3DDevice->DrawPrimitiveUP(D3DPT_POINTLIST, 3, vertex, sizeof(Vertex2D));

	lpD3DDevice->SetFVF(FVF_VERTEX);

}

void DrawLine(float X, float Y, float XX, float YY, D3DCOLOR Color)
{
	struct Vertex2D vertex[] =
	{
	{ X,Y,0,1,Color },
	{XX, YY, 0, 1, Color } };
	lpD3DDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);

	lpD3DDevice->DrawPrimitiveUP(D3DPT_LINELIST, 3, vertex, sizeof(Vertex2D));

	lpD3DDevice->SetFVF(FVF_VERTEX);

}

void DrawTriangle(float X, float Y, float XX, float YY, float XZ, float YZ, D3DCOLOR Color)
{
	struct Vertex2D vertex[] =
	{
	{ X,Y,0,1,Color },
	{XX, YY, 0, 1, Color },
	{XZ,YZ,0,1,Color} };
	lpD3DDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);

	lpD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 3, vertex, sizeof(Vertex2D));

	lpD3DDevice->SetFVF(FVF_VERTEX);
}

void DrawBox(float X, float Y, int Width, int Height, D3DCOLOR Color)
{
	struct Vertex2D vertex[] =
	{
	{ X,Y,0,1,Color },
	{X + Width, Y, 0, 1, Color },
	{X + Width,Y + Height,0,1,Color},
	{X,Y + Height,0,1,Color},
	{X,Y,0,1,Color} };
	lpD3DDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);

	lpD3DDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, vertex, sizeof(Vertex2D));

	lpD3DDevice->SetFVF(FVF_VERTEX);
}

void FillTriangle(float X, float Y, float XX, float YY, float XZ, float YZ, D3DCOLOR Color)
{
	struct Vertex2D vertex[] =
	{
	{ X,Y,0,1,Color },
	{XX, YY, 0, 1, Color },
	{XZ,YZ,0,1,Color} };
	lpD3DDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);

	lpD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 1, vertex, sizeof(Vertex2D));

	lpD3DDevice->SetFVF(FVF_VERTEX);
}

void FillBox(float X, float Y, int Width, int Height, D3DCOLOR Color)
{
	struct Vertex2D vertex[] =
	{
	{ X,Y,0,1,Color },
	{X + Width, Y, 0, 1, Color },
	{X,Y + Height,0,1,Color},
	{X + Width,Y + Height,0,1,Color} };
	lpD3DDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);

	lpD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertex, sizeof(Vertex2D));

	lpD3DDevice->SetFVF(FVF_VERTEX);
}

void DrawCircle(float X,float Y,int SZ,int PN, int CN, D3DCOLOR Color)
{
	struct Vertex2D vertex[360 + 1];

	float DeG;
	float RaD;

	for (int i = 0; i < PN + 1; i++)
	{
		DeG = i * CN;
		RaD = DeG * 3.14 / 180;
		vertex[i].x = X + cos(RaD) * SZ;
		vertex[i].y = Y + sin(RaD) * SZ;
		vertex[i].z = 0;
		vertex[i].rhw = 1;
		vertex[i].Color = Color;
	}

	lpD3DDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);

	lpD3DDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, PN, vertex, sizeof(Vertex2D));

	lpD3DDevice->SetFVF(FVF_VERTEX);
}

void CircleFill(float X, float Y, int SZ, int PN, int CN, D3DCOLOR Color)
{
	struct Vertex2D vertex[360 + 1];

	float DeG;
	float RaD;

	for (int i = 0; i < PN + 1; i++)
	{
		DeG = i * CN;
		RaD = DeG * 3.14 / 180;
		vertex[i].x = X + cos(RaD) * SZ;
		vertex[i].y = Y + sin(RaD) * SZ;
		vertex[i].z = 0;
		vertex[i].rhw = 1;
		vertex[i].Color = Color;
	}

	lpD3DDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);

	lpD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, PN - 2, vertex, sizeof(Vertex2D));

	lpD3DDevice->SetFVF(FVF_VERTEX);
}

void ESUP(void)
{
	DrawBox(EUP.X, EUP.Y, EUP.XX, EUP.YY, EUP.Color);				//敵棒上
}

void ESDOWN(void)
{
	DrawBox(EDW.X, EDW.Y, EDW.XX, EDW.YY, EDW.Color);				//敵棒上
}

void ESLEFT(void)
{
	DrawBox(ELF.X, ELF.Y, ELF.XX, ELF.YY, ELF.Color);				//敵棒上
}

void ESRIGHT(void)
{
	DrawBox(ERI.X, ERI.Y, ERI.XX, ERI.YY, ERI.Color);				//敵棒上
}

void GlobalInit(void)
{
	printf("\nInitialize Start\n\n");
	if (Con)
	{
		//==============
		CreateConsole();	//コンソール作成
		//==============
		printf("\nConsole Make Success\n");
	}

	/*======================================*/
	//グローバル座表＆初期値初期化
	PlayerRemain = 3;
	/*======================================*/


	/*======================================*/
	//グローバルフラグ初期化
	AlfaFlg = T;
	GuardUPFlg = F;
	GuardDOWNFlg = F;
	GuardLREFTFlg = F;
	GuardRIGHTFlg = F;

	//ゲーム内完全制御初期化
	Scene = F;
	printf("\nScene Initialize %d\n", Scene);
	GameNumBer = 1;
	printf("\nGameNumBer Initialize %d\n", GameNumBer);
	//==========================//
	printf("\nInitialize End\n\n");
}

void Title_Init(void)
{
	Scene = F;
}

void Title_Update(void)
{
	if (GetAsyncKeyState(VK_RETURN) & 0x8000)
	{
		Game_Init();
	}
}

void Title_Render(void)
{

}

void Game_Init(void)
{
	Scene = 1;
	PlayerX = SCRW / 2;
	PlayerY = SCRH / 2;
}

void Game_Update(void)
{
	if (GetAsyncKeyState(VK_SPACE) & 0x8000)
	{
		Scene = 0;
	}

	/*=======================================================*/
		//ガード判定
	if (Mouse.y < 40)					//上ガード判定
	{
		GuardUPFlg = T;

		if (Mouse.x > SCRW - 40)		//上右ガード判定
		{
			GuardRIGHTFlg = T;
		}

		if (Mouse.x < 40)				//上左ガード判定
		{
			GuardLREFTFlg = T;
		}
	}

	else if (Mouse.y > SCRH - 40)		//下ガード判定
	{
		GuardDOWNFlg = T;

		if (Mouse.x < 40)				//下左ガード判定
		{
			GuardLREFTFlg = T;
		}

		if (Mouse.x > SCRW - 40)		//下右ガード判定
		{
			GuardRIGHTFlg = T;
		}
	}
	else if (Mouse.x < 40)				//左ガード判定
	{
		GuardLREFTFlg = T;
	}
	else if (Mouse.x > SCRW - 40)		//右ガード判定
	{
		GuardRIGHTFlg = T;
	}
	else
	{
		GuardUPFlg = F;
		GuardDOWNFlg = F;
		GuardLREFTFlg = F;
		GuardRIGHTFlg = F;
	}
	/*=====================================================*/

	/*-------------*/
	//自機死亡制御
	if (PlayerRemain <= 0)
	{
		Title_Init();		//(本来はリザルト画面に移行)仮
	}
	/*-------------*/

	/*=====================================================*/
	//敵攻撃ガード当たり判定
	if (GuardUPFlg == T && EUP.EUFlg == T)
	{
		if (EUP.Y > 100)
		{
			EUP.EUFlg = F;
			EUP.Y = -40;
		}
	}
	/*=====================================================*/
	/*if (EUP.EUFlg == F)
	{
		EUP.EUFlg = T;
	}*/
	//敵表示合致消去(α)
	if (EUP.EUFlg == T)
	{
		EUP.Y += 0.5;
		if ((GuardUPFlg == F) && (EUP.Y > 100))
		{
			GDB -= 85;
			GDR += 85;
			PlayerRemain--;
			EUP.EUFlg = F;
		}
	}



	/*=============================*/

	/*==============================================*/
	//ステージデータ
	/*switch ()
	{
	default:
		break;
	}*/

	/*==============================================*/
}

void Game_Render(void)
{	
	/*==============================*/
	//透明描画

	if (AlfaFlg == T)
	{
		Ti = (sin(3 * 3.1415 / 260 * Flame) + 1) * 127;
		if (Ti <= 80)
		{
			Ti = 80;
		}
	}

	/*==============================*/

	/*==============================*/
	//ゲーム内描画
	if (Scene == 1)
	{
		static int GDB = 255;
		static int GDR = 0;
		if ((GDB <= 0) && (GDR >= 255))
		{
			GDB <= 0;
			GDR >= 255;
		}


		////描画
		
		BK;		//背景(仮黒)
		DrawBox(180, -40, 300, 80, D3DCOLOR_ARGB(Ti, 255, 0, 0));					//上当たり判定Box
		DrawBox(180, SCRH - 40, 300, SCRH + 80, D3DCOLOR_ARGB(Ti, 255, 0, 0));		//下当たり判定Box
		DrawBox(-40, 80, 80, 210, D3DCOLOR_ARGB(Ti, 255, 0, 0));					//左当たり判定Box
		DrawBox(SCRW - 40, 80, SCRW + 80, 210, D3DCOLOR_ARGB(Ti, 255, 0, 0));		//右当たり判定Box
		/*EnemyDOWN;
		EnemyLEFT;
		EnemyRIGHT;*/
		DrawCircle(PlayerX, PlayerY, 50, 60, 6, D3DCOLOR_ARGB(255, GDR, 0, GDB));				//自機(守護対象)
		CircleFill(Mouse.x, Mouse.y, 15, 60, 6, D3DCOLOR_ARGB(255, 255, 0, 0));					//プレイヤー操作アイコン①(赤●)
		DrawLine(Mouse.x - 10, Mouse.y, Mouse.x + 10, Mouse.y, D3DCOLOR_ARGB(255, 0, 0, 255));	//プレイヤー操作アイコン②(青十字)
		DrawLine(Mouse.x, Mouse.y - 10, Mouse.x, Mouse.y + 10, D3DCOLOR_ARGB(255, 0, 0, 255));	//プレイヤー操作アイコン③(青十字)
		//敵(棒)----------------------//
		if (EUP.EUFlg == T)
		{
			ESUP();
		}
		if (EDW.EDFlg == T)
		{
			ESDOWN();
		}
		if (ELF.ELFlg == T)
		{
			ESLEFT();
		}
		if (ERI.ERFlg == T)
		{
			ESRIGHT();
		}
		//ガーディアン----------------//
		if (GuardUPFlg == T)
		{
			GuardUP;
		}
		if (GuardDOWNFlg == T)
		{
			GuardDOWN;
		}
		if (GuardLREFTFlg == T)
		{
			GuardLEFT;
		}
		if (GuardRIGHTFlg == T)
		{
			GuardRIGHT;
		}
		/*----------------------------*/

	}
}


void GlobalEnd(void)
{
	//============
	//コンソールの解放
	FreeConsole();
	printf("Console　END");
	//============
}

// 更新処理
void Update(void)
{
	/*------------------------------*/
	//マウスの座標を取得
	GetCursorPos(&Mouse);
	ScreenToClient(hwnd, &Mouse);
	/*------------------------------*/
	MousePosX = Mouse.x;
	MousePosY = Mouse.y;

	switch (Scene)
	{
	case 0: Title_Update();
		break;
	case 1: Game_Update();
		break;
	}
}

// 3D描画
void Render3D(void)
{

}


// 2D描画
void Render2D(void)
{
	//////////////////////////////////////////////////
	///	スプライトの描画処理
	//////////////////////////////////////////////////
	// 描画開始
	lpSprite->Begin(D3DXSPRITE_ALPHABLEND);

	switch (Scene)
	{
	case 0:Title_Render();
		break;
	case 1:Game_Render();
		break;
	}

	

	// 描画終了
	lpSprite->End();

	if (Con)
	{
		debug();
	}

}

void debug(void)
{
	printf("\rMouseX %.2f\tMouseY %.2f\tScene %d\tGameNumBer %d　enemyUPY %.2f", MousePosX, MousePosY,Scene,GameNumBer,EUP.Y);

}

void GameFrame(void)
{
	// バックバッファと Z バッファをクリア
	lpD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(64, 64, 255), 1.0f, 0);


	// 更新処理
	Update();


	// 描画開始
	lpD3DDevice->BeginScene();

	D3DXMATRIX mView, mProj;

	// 視点行列の設定
	D3DXMatrixLookAtLH(&mView,
		&D3DXVECTOR3(0.0f, 0.0f, -10.0f),	// カメラの位置
		&D3DXVECTOR3(0.0f, 0.0f, 0.0f),	// カメラの視点
		&D3DXVECTOR3(0.0f, 1.0f, 0.0f)	// カメラの頭の方向
	);

	// 投影行列の設定
	D3DXMatrixPerspectiveFovLH(&mProj, D3DXToRadian(60), (float)SCRW / (float)SCRH, 1.0f, 1000.0f);

	//行列設定
	lpD3DDevice->SetTransform(D3DTS_VIEW, &mView);
	lpD3DDevice->SetTransform(D3DTS_PROJECTION, &mProj);



	// 3D描画
	Render3D();

	// 2D描画
	Render2D();



	// 描画終了
	lpD3DDevice->EndScene();

	// バックバッファをプライマリバッファにコピー
	lpD3DDevice->Present(NULL, NULL, NULL, NULL);

	Flame++;
}

LRESULT APIENTRY WndFunc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_KEYDOWN:
		switch (wParam) {
		case VK_ESCAPE:
			PostQuitMessage(0);
			return 0;
		}
		return 0;

	}
	return DefWindowProc(hwnd, msg, wParam, lParam);

}

void LoadTexture(LPDIRECT3DTEXTURE9 *lpTex, const char fname[], int W, int H, D3DCOLOR Color)
{
	if (W == 0)W = D3DX_DEFAULT;
	if (H == 0)H = D3DX_DEFAULT;
	D3DXCreateTextureFromFileEx(lpD3DDevice, fname, W, H, 1, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_DEFAULT, Color, NULL, NULL, lpTex);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev,
	LPSTR lpszCmdParam, int nCmdshow)
{
	MSG msg;

	hwnd;
	WNDCLASS wc;
	char szAppName[] = "Generic Game SDK Window";

	wc.style = CS_DBLCLKS;
	wc.lpfnWndProc = WndFunc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInst;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = szAppName;

	RegisterClass(&wc);

	hwnd = CreateWindowEx(
		0,
		szAppName,
		"Gurd ",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		SCRW, SCRH,
		NULL, NULL, hInst,
		NULL);

	if (!hwnd)return FALSE;

	ShowWindow(hwnd, nCmdshow);
	UpdateWindow(hwnd);
	SetFocus(hwnd);

	gameFullScreen = false;	// ウィンドウモード

	if (gameFullScreen) {
		ShowCursor(FALSE);
	}
	else {
		RECT rc = { 0,0,SCRW,SCRH };
		AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
		SetWindowPos(hwnd, NULL, 50, 50, rc.right - rc.left, rc.bottom - rc.top, SWP_SHOWWINDOW | SWP_NOZORDER);
	}

	//---------------------DirectX Graphics関連-----------------------

	// Direct3D オブジェクトを作成
	lpD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (!lpD3D)
	{
		// オブジェクト作成失敗
		MessageBox(NULL, "Direct3D の作成に失敗しました。", "ERROR", MB_OK | MB_ICONSTOP);
		// 終了する
		return 0;
	}
	int adapter;

	// 使用するアダプタ番号
	adapter = D3DADAPTER_DEFAULT;

	// ウインドウの作成が完了したので、Direct3D を初期化する
	ZeroMemory(&d3dpp, sizeof(D3DPRESENT_PARAMETERS));
	// Direct3D 初期化パラメータの設定
	if (gameFullScreen)
	{
		// フルスクリーン : ほとんどのアダプタでサポートされているフォーマットを使用
		d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
	}
	else
	{
		// ウインドウ : 現在の画面モードを使用
		D3DDISPLAYMODE disp;
		// 現在の画面モードを取得
		lpD3D->GetAdapterDisplayMode(adapter, &disp);
		d3dpp.BackBufferFormat = disp.Format;
	}
	// 表示領域サイズの設定
	d3dpp.BackBufferWidth = SCRW;
	d3dpp.BackBufferHeight = SCRH;
	d3dpp.SwapEffect = D3DSWAPEFFECT_FLIP;

	if (!gameFullScreen)
	{
		// ウインドウモード
		d3dpp.Windowed = 1;
	}

	// Z バッファの自動作成
	d3dpp.EnableAutoDepthStencil = 1;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

	//ﾊﾞｯｸﾊﾞｯﾌｧをﾛｯｸ可能にする(GetDCも可能になる)
	d3dpp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;

	// デバイスの作成 - T&L HAL
	if (FAILED(lpD3D->CreateDevice(adapter, D3DDEVTYPE_HAL, hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &lpD3DDevice)))
	{
		// 失敗したので HAL で試行
		if (FAILED(lpD3D->CreateDevice(adapter, D3DDEVTYPE_HAL, hwnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &lpD3DDevice)))
		{
			// 失敗したので REF で試行
			if (FAILED(lpD3D->CreateDevice(adapter, D3DDEVTYPE_REF, hwnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &lpD3DDevice)))
			{
				// 結局失敗した
				MessageBox(NULL, "DirectX9が初期化できません。\n未対応のパソコンと思われます。", "ERROR", MB_OK | MB_ICONSTOP);
				lpD3D->Release();
				// 終了する
				return 0;
			}
		}
	}

	// レンダリング・ステートを設定
	// Z バッファ有効化->前後関係の計算を正確にしてくれる
	lpD3DDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
	lpD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);

	// アルファブレンディング有効化
	lpD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

	// アルファブレンディング方法を設定
	lpD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	lpD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	// レンダリング時のアルファ値の計算方法の設定
	lpD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	// テクスチャの色を使用
	lpD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	// 頂点の色を使用
	lpD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	// レンダリング時の色の計算方法の設定
	lpD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);

	//裏面カリング
	lpD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	// フィルタ設定
	lpD3DDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
	lpD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	lpD3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

	// ライト
	lpD3DDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

	lpD3DDevice->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL);
	// 頂点フォーマットの設定
	lpD3DDevice->SetFVF(FVF_VERTEX);

	timeBeginPeriod(1);

	// ゲームに関する初期化処理 ---------------------------

	GlobalInit();

	switch (Scene)
	{
	case 0: Title_Init();
		break;
	case 1: Game_Init();
		break;
	}
	


	// スプライト作成
	D3DXCreateSprite(lpD3DDevice, &lpSprite);
	lpSprite->OnResetDevice();

	// フォント作成
	D3DXCreateFont(lpD3DDevice, 30, 30, FW_REGULAR, NULL, FALSE, SHIFTJIS_CHARSET,
		OUT_DEFAULT_PRECIS, PROOF_QUALITY, FIXED_PITCH | FF_MODERN, "ＭＳ ゴシック", &lpFont);

	lpFont->OnResetDevice();

	while (1) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			if (!GetMessage(&msg, NULL, 0, 0))
				break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			GameFrame();
		}
	}

	// ゲームに関する終了処理 ---------------------------

	GlobalEnd();

	lpSprite->Release();	// スプライト
	lpFont->Release();		// フォント


	timeEndPeriod(1);

	// Direct3D オブジェクトを解放
	lpD3DDevice->Release();
	lpD3D->Release();

	return (int)msg.wParam;
}

void CreateConsole()
{
	if (Con)
	{
		//コンソール作成
		AllocConsole();
		FILE *fp_c;
		freopen_s(&fp_c, "CONOUT$", "w", stdout);
		freopen_s(&fp_c, "CONOUT$", "r", stdin);
	}
}
