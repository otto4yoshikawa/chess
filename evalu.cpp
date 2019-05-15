
#include <math.h>
#include"chessrule.h"
#include"board.h"
#include<stdio.h>
extern MAXTYPE RootValue;
extern int  CastMove[2][2][2] ;
int max(int a, int b) {
	return (a>b)?a:b;
}

int min(int a, int b) {
	return (a<b)?a:b;
}
#define    TOLERANCE        8  //  許容範囲
#define    EXCHANGEVALUE    32
// 駒を、前の(ポーン以外の)駒と交換する時の値
#define    ISOLATEDPAWN     20
// 孤立したポーン。２つの孤立したポーンは 3 * 20
#define    DOUBLEPAWN       8  //  ２重ポーン
#define    SIDEPAWN        6   //  端にいるポーン
#define    CHAINPAWN        3  //  ポーンにカバーされている
#define    COVERPAWN        3  //  ポーンをカバーしている
#define    NOTMOVEPAWN      2  //  ポーン移動のペナルティ
#define    BISHOPBLOCKVALUE  20
// d2/e2 ポーンがビショップをブロックしているときのペナルティ
#define    ROOKBEHINDPASSPAWN 16  //  後ろのルークがポーンを通過したときのボーナス

typedef unsigned char FILETYPE; // 縦列番号
typedef unsigned char RANKTYPE; // 横列番号
typedef unsigned char SETOFFILE;
typedef struct {
	SETOFFILE one, dob;
}PAWNBITTYPE[2];



void CalcCastling(COLORTYPE incolor, CASTDIRTYPE * cast);

void GenCastSquare(SQUARETYPE new1, SQUARETYPE * castsquare,
	SQUARETYPE * cornersquare);

char* MoveStr(MOVETYPE*);
// グローバル変数
//

LEVELTYPE Level;
//extern NODEVAL Nodes;

// このモジュールのグローバル変数
//
int PieceValue[7] = {
	0, 0x1000, 0x900, 0x4c0, 0x300, 0x300, 0x100
};
const char distan[8] = {
	3, 2, 1, 0, 0, 1, 2, 3
};
// ポーンの値は横列と縦列の合計。
// 縦列の値は PawnFileFactor * (Rank Number + 2) と等価
const char pawnrank[8] = {
	0, 0, 0, 2, 4, 8, 30, 0
};
const char passpawnrank[8] = {
	0, 0, 10, 20, 40, 60, 70, 0
};
const char pawnfilefactor[8] = {
	0, 0, 2, 5, 6, 2, 0, 0
};
const char castvalue[2] = {
	4, 32
}; // キャスリングの値

const SETOFFILE filebittab[8] = {
	1, 2, 4, 8, 0x10, 0x20, 0x40, 0x80
};
PAWNBITTYPE pawnbitt[MAXPLY + 2];
PAWNBITTYPE *pawnbit = &pawnbitt[1];
int totalmaterial, pawntotalmaterial, material;
// ゲームの物質値レベル
// (序盤、中盤 = 43 - 32、終盤 = 0)
static int materiallevel;
const unsigned char squarerankvalue[8] = {
	0, 0, 0, 0, 1, 2, 4, 4
};
#define  MAXINT  32000
static int tempcolor;
static int temp, temp2; // 一時使用
//-----extern ---------------------------------
extern char *buf;
extern class TInfoWindow* TInfo;
extern MOVETYPE ZeroMove;
extern bool Analysis, Opan;
//void IncNode(NODEVAL * nodes);
short Attacks(COLORTYPE acolor, SQUARETYPE square);
short PieceAttacks(PIECETYPE, COLORTYPE, SQUARETYPE, SQUARETYPE);
FIFTYTYPE FiftyMoveCnt(void);
REPEATTYPE Repetition(short immediate);
void PrintBestMove(MOVETYPE * mainline, MAXTYPE mainevalu);
void DisplayMove(bool*singlestep, DEPTHTYPE maxdepth, MAXTYPE evaluation, bool);

short KillMovGen(MOVETYPE * move);
extern LINETYPE MainLine;
extern bool MultiMove, AutoPlay, SingleStep;

bool EqMove(MOVETYPE * a, MOVETYPE * b);
void InitNode(NODEVAL*);
void InitTime(CLOCKTYPE * clock);
void StartTime(CLOCKTYPE*);
void StopTime(CLOCKTYPE*);
void PrintNodes(NODEVAL*, double);

 int StatEvalu(MOVETYPE * move);

 //----------------------------------------------
CLOCKTYPE ChessClock  ;
extern COLORTYPE RunColor;
//
// b のセットされているビットの数を数える
//
static int count(SETOFFILE b) {
	char cnt = 0;
	while (b) {
		if (b % 2 == 1)
			cnt++;
		b >>= 1;
	}
	return cnt;
}

//
// pawnbit[depth][color] のポーン構造の値を計算
//
static int pawnstrval(DEPTHTYPE depth, COLORTYPE color) {
	SETOFFILE iso; // 孤立したポーンを含む縦列

	iso = SETOFFILE(pawnbit[depth][color].one&~
		((pawnbit[depth][color].one << 1) | (pawnbit[depth][color].one >> 1)));
	return-(count(pawnbit[depth][color].dob) * DOUBLEPAWN + count(iso)
		* ISOLATEDPAWN + count(iso & pawnbit[depth][color].dob) * ISOLATEDPAWN * 2);
}

//
// マス上の駒の値を計算
//
inline int PiecePosVal(PIECETYPE piece, COLORTYPE color, SQUARETYPE square) {
	return PieceValue[piece] + PVTable[color][piece][square];
}

short mating; // 詰み評価関数で使われる

//
// 静止評価関数のために駒の値テーブル（Piece-Value Table）を計算
//
void CalcPVTable() {
	typedef SETOFFILE PAWNTABTYPE[8];

	PAWNTABTYPE pawntab[2];
	// 静止ポーン構造評価のためのビットテーブル
	SETOFFILE pawnfiletab, bit, oppasstab, behindoppass, leftsidetab,
	rightsidetab, sidetab, leftchaintab, rightchaintab, chaintab, leftcovertab,
	rightcovertab;

	// マスの攻撃重要性
	char attackvalue[2][0x78];
	// マスで制御されるマスの値
	int pvcontrol[2][5][0x78];

	COLORTYPE losingcolor; // 詰まれる色
	int posval; // 駒の位置値
	int attval; // マスの攻撃値
	FILETYPE line; // 駒の縦列
	RANKTYPE rank; // 駒の横列
	char dist, kingdist; // 中心までの距離、相手のキングまでの距離
	CASTTYPE cast; // キャスリング可能
	short direct; // 直接/間接攻撃
	int cnt; // 攻撃値用カウンタ
	int strval; // ポーン構造値
	int color, oppcolor; // 自分の色、相手の色
	int piececount; // 駒用カウンタ
	SQUARETYPE square; // マス用カウンタ
	int dir;
	// 方向用カウンタ
	EDGESQUARETYPE sq; // マス用カウンタ
	int temp, temp2; // 一時使用
	int PVTable[2][7][0x78];
	// 物質値合計、ポーンの物質値の合計、物質値の計算
	material = pawntotalmaterial = totalmaterial = mating = 0;

	for (square = 0; square < 0x78; square++)
		if (!(square & 0x88))
			if (Board[square].piece != empty)
				if (Board[square].piece != king) {
					temp = PieceValue[Board[square].piece];
					totalmaterial += temp;
					if (Board[square].piece == pawn)
						pawntotalmaterial += PieceValue[pawn];
					if (Board[square].color == white)
						temp = -temp;
					material -= temp;
				}
	materiallevel = max(0, totalmaterial - 0x2000) / 0x100;
	// 弱いプレイヤーが２つのビショップの価値よりも低く、
	// 少なくともビショップよりもルークが有利なら mating をセットする
	if (material < 0)
		losingcolor = white;
	else
		losingcolor = black;
	mating = ((totalmaterial - abs(material)) / 2 <= PieceValue[bishop] * 2) &&
		(abs(material) >= PieceValue[rook] - PieceValue[bishop]);
	// 攻撃値の計算（各マスの重要性）
	for (rank = 0; rank < 8; rank++)
		for (line = 0; line < 8; line++) {
			square = (rank << 4) + line;
			attval = max(0, 8 - 3 * (distan[rank] + distan[line]));
			// 中央の重要性
			// 横列の重要性
			for (color = white; color <= black; color++) {
				attackvalue[color][square] = char
					(((squarerankvalue[rank] * 3 * (materiallevel + 8)) >> 5) + attval);
				square ^= 0x70;
			}
		}
	for (color = white; color <= black; color++) {
		oppcolor = (COLORTYPE)(1 - color);
		CalcCastling(oppcolor, &cast);
		if (cast != shrt && materiallevel > 0)
		// 相手のキングの回りの８つのマスは重要
			for (dir = 0; dir < 8; dir++) {
				sq = PieceTab[oppcolor][0].isquare + DirTab[dir];
				if (!(sq & 0x88))
					attackvalue[color][sq] += char
						(((12 * (materiallevel + 8)) >> 5));
			}
	}

	// PVControl を計算
	for (square = 0x77; square >= 0; square--)
		if (!(square & 0x88))
			for (color = white; color <= black; ((int)color)++)
				for (piececount = rook; piececount <= bishop; ((int)piececount)
					++)
					pvcontrol[color][piececount][square] = 0;
	for (square = 0x77; square >= 0; square--)
		if (!(square & 0x88))
			for (color = white; color <= black; ((int)color)++) {
				for (dir = 7; dir >= 0; dir--) {
					if (dir < 4)
						piececount = rook;
					else
						piececount = bishop;
					// マスのすべての方向からの攻撃を数える。
					// マスの攻撃の値は ATTACKVAL にある。
					// 間接的な攻撃（例えば、他のルークを通
					// したルークの攻撃）は、普通の攻撃とし
					// て数え、他の駒を通した攻撃は、半分に
					// 数える。
					cnt = 0;
					sq = square;
					direct = 1;
					do {
						sq += DirTab[dir];
						if (sq & 0x88)
							goto TEN;
						temp = attackvalue[color][sq];
						if (direct)
							cnt += temp;
						else
							cnt += (temp >> 1);
						if (Board[sq].piece != empty)
							if
								(Board[sq].piece != piececount && Board[sq].piece != queen)
								direct = 0;
					}
					while (Board[sq].piece != pawn);
				TEN:
					pvcontrol[color][piececount][square] += (cnt >> 2);
				}
			}

	// PVTable を計算。値による値
	for (square = 0x77; square >= 0; square--)
		if (!(square & 0x88)) {
			for (color = white; color <= black; ((int)color)++) {
				oppcolor = (COLORTYPE)(1 - color);
				line = FILETYPE(square & 7);
				rank = RANKTYPE(square >> 4);
				if (color == black)
					rank = RANKTYPE(7 - rank);
				dist = char(distan[rank] + distan[line]);
				kingdist = char
					(abs((square >> 4) - (PieceTab[oppcolor][0].isquare >> 4)) + ((square - PieceTab[oppcolor][0].isquare) & 7));
				for (piececount = king; piececount <= pawn; ((int)piececount)
					++) {
					posval = 0; // マス上の駒の位置
					// の値を計算
					if (mating && piececount != pawn) {
						if (piececount == king)
							if (color == losingcolor) { // 詰み評価
								posval = 128 - 16 * distan[rank] - 12 * distan
									[line];
								if (distan[rank] == 3)
									posval -= 16;

							}
							else {
								posval = 128 - 4 * kingdist;
								if (distan[rank] >= 2 || distan[line] == 3)
									posval -= 16;
							}
					}
					else {
						temp = pvcontrol[color][rook][square];
						temp2 = pvcontrol[color][bishop][square];
						// 通常の評価関数
						switch(piececount) {
						case king:
							if (materiallevel <= 0)
								posval = -2 * dist;
							break;
						case queen:
							posval = (temp + temp2) >> 2;
							break;
						case rook:
							posval = temp;
							break;
						case bishop:
							posval = temp2;
							break;
						case knight:
							cnt = 0;
							for (dir = 0; dir < 8; dir++) {
								sq = square + KnightDir[dir];
								if (!(sq & 0x88))
									cnt += attackvalue[color][sq];
							}
							posval = (cnt >> 1) - dist * 3;
							break;
						case pawn:
							if (rank != 0 && rank != 7)
								posval = pawnrank[rank] + pawnfilefactor[line]
									* (rank + 2) - 12;
						}
					}
					PVTable[color][piececount][square] = posval;
				}
			}
		}

	// pawntab の計算 (どのマスにポーンがいるかを示す)

	for (color = white; color <= black; ((int)color)++)
		for (rank = 0; rank < 8; rank++)
			pawntab[color][rank] = 0;
	for (square = 0x77; square >= 0; square--)
		if (!(square & 0x88))
			if (Board[square].piece == pawn) {
				rank = RANKTYPE(square >> 4);
				if (Board[square].color == black)
					rank = RANKTYPE(7 - rank);
				pawntab[Board[square].color][rank] |= filebittab[square & 7];
			}
	for (color = white; color <= black; ((int)color)++) { // pawnbit の初期化
		pawnbit[-1][color].one = pawnbit[-1][color].dob = 0;
		for (rank = 1; rank < 7; rank++) {
			temp = pawntab[color][rank];
			pawnbit[-1][color].dob |= SETOFFILE
				((pawnbit[-1][color].one & temp));
			pawnbit[-1][color].one |= SETOFFILE(temp);
		}
	}

	// ポーンの構造の値を計算
	RootValue = pawnstrval(-1, Player) - pawnstrval(-1, Opponent);

	// ポーンの構造による静止値の計算
	for (color = white; color <= black; ((int)color)++) {
		oppcolor = (1 - color);
		pawnfiletab = leftsidetab = rightsidetab = behindoppass = 0;
		oppasstab = 0xff;
		for (rank = 1; rank < 7; rank++) {
			// ポーンに通過される相手のポーンのマス
			oppasstab &= SETOFFILE((~(pawnfiletab | leftsidetab | rightsidetab))
				);
			// 相手のポーンがポーンを通過した後ろのマス
			behindoppass |= (oppasstab & pawntab[oppcolor][7 - rank]);
			// ポーンによってカバーされているマス
			leftchaintab = leftsidetab;
			rightchaintab = rightsidetab;
			pawnfiletab = pawntab[color][rank]; // ポーンのいるマス
			// それらに並んでいるポーンのマス
			leftsidetab = SETOFFILE((pawnfiletab << 1) & 0xff);
			rightsidetab = SETOFFILE((pawnfiletab >> 1) & 0xff);
			sidetab = leftsidetab | rightsidetab;
			chaintab = leftchaintab | rightchaintab;
			// ポーンをカバーするマス
			temp = pawntab[color][rank + 1];
			leftcovertab = SETOFFILE((temp << 1) & 0xff);
			rightcovertab = SETOFFILE((temp >> 1) & 0xff);
			sq = rank << 4;
			if (color == black)
				sq ^= 0x70;
			bit = 1;
			while (bit) {
				strval = 0;
				if (bit & sidetab)
					strval = SIDEPAWN;
				else if (bit & chaintab)
					strval = CHAINPAWN;
				if (bit & leftcovertab)
					strval += COVERPAWN;
				if (bit & rightcovertab)
					strval += COVERPAWN;
				if (bit & pawnfiletab)
					strval += NOTMOVEPAWN;
				PVTable[color][pawn][sq] += strval;
				if ((materiallevel <= 0) || (oppcolor != ProgramColor)) {
					if (bit & oppasstab)
						PVTable[oppcolor][pawn][sq] += passpawnrank[7 - rank];
					if (bit & behindoppass) {
						temp = sq ^ 0x10;
						for (tempcolor = black; tempcolor >= white;
							tempcolor--) {
							PVTable[tempcolor][rook][sq] += ROOKBEHINDPASSPAWN;
							if (rank == 6)
								PVTable[tempcolor][rook][temp]
									+= ROOKBEHINDPASSPAWN;
						}
					}
				}
				sq++;
				bit = SETOFFILE((bit << 1) & 0xff);
			}
		}
	}

	// 中央のポーンをビショップがブロックしているペナルティの計算
	for (sq = 3; sq < 5; sq++) {
		if (Board[sq + 0x10].piece == pawn && Board[sq + 0x10].color == white)
			PVTable[white][bishop][sq + 0x20] -= BISHOPBLOCKVALUE;
		if (Board[sq + 0x60].piece == pawn && Board[sq + 0x60].color == black)
			PVTable[black][bishop][sq + 0x50] -= BISHOPBLOCKVALUE;
	}
	for (square = 0x77; square >= 0; square--) // RootValue を計算
		if (!(square & 0x88))
			if (Board[square].piece != empty)
				if (Board[square].color == Player)
					RootValue += PiecePosVal
						(Board[square].piece, Player, square);
				else
					RootValue -= PiecePosVal
						(Board[square].piece, Opponent, square);
}

// ----------------------------------------------------------------------------

static int value;
SQUARETYPE castsquare, cornersquare, epsquare;

//
// pawnbit を更新し、ポーンを line から取り去ったときの値を計算
//
inline int decpawnstrval(COLORTYPE color, FILETYPE line) {
	temp = ~filebittab[line];
	pawnbit[Depth][color].one = SETOFFILE((pawnbit[Depth][color].one & temp)
		| pawnbit[Depth][color].dob);
	pawnbit[Depth][color].dob &= SETOFFILE(temp);
	return int(pawnstrval(Depth, color) - pawnstrval(DEPTHTYPE(Depth - 1),
			color));
}

// pawnbit を更新し、ポーンが old から new1 の縦列に
// 移動するときの値を計算
//
static int movepawnstrval(COLORTYPE color, FILETYPE new1, FILETYPE old) {
	temp = filebittab[new1];
	temp2 = ~filebittab[old];
	pawnbit[Depth][color].dob |= SETOFFILE((pawnbit[Depth][color].one & temp));
	pawnbit[Depth][color].one = SETOFFILE((pawnbit[Depth][color].one & temp2)
		| pawnbit[Depth][color].dob | temp);
	pawnbit[Depth][color].dob &= SETOFFILE(temp2);
	return int(pawnstrval(Depth, color) - pawnstrval(DEPTHTYPE(Depth - 1),
			color));
}

// ----------------------------------------------------------------------------

//
// 移動の静止評価を計算
//
int StatEvalu(MOVETYPE *move) {
	value = 0;
	if (move->spe)
		if (move->movpiece == king) {
			GenCastSquare(move->new1, &castsquare, &cornersquare);
			value = PiecePosVal(rook, Player, castsquare) - PiecePosVal
				(rook, Player, cornersquare);
			if (move->new1 > move->old)
				value += castvalue[shrt - 1];
			else
				value += castvalue[lng - 1];

		}
		else if (move->movpiece == pawn) {
			epsquare = move->new1 - PawnDir[Player]; // 通過捕獲
			value = PiecePosVal(pawn, Opponent, epsquare);

		}
		else // ポーンの昇格
			value = PiecePosVal(move->movpiece, Player, move->old) - PiecePosVal
				(pawn, Player, move->old) + decpawnstrval
				(Player, FILETYPE(move->old & 7));

	if (move->content != empty) { // 通常移動
		value += PiecePosVal(move->content, Opponent, move->new1);
		// 後ろの駒と交換するときの物質値ペナルティ
		if (abs(MainEvalu) >= 0x100)
			if (move->content != pawn)
				if ((ProgramColor == Opponent) == (MainEvalu >= 0))
					value -= EXCHANGEVALUE;
	}
	pawnbit[Depth][black] = pawnbit[Depth - 1][black]; // pawnbitを計算
	pawnbit[Depth][white] = pawnbit[Depth - 1][white];
	if (move->movpiece == pawn && (move->content != empty || move->spe))
		value += movepawnstrval(Player, FILETYPE(move->new1 & 7), FILETYPE
		(move->old & 7));
	if (move->content == pawn || move->spe && move->movpiece == pawn)
		value -= decpawnstrval(Opponent, FILETYPE(move->new1 & 7));
	// 移動の値を計算
	return value + PiecePosVal(move->movpiece, Player, move->new1) - PiecePosVal
		(move->movpiece, Player, move->old);
}

