// =======searck.cpp==================================================

#include"chessrule.h"
#include"board.h"
#include<stdio.h>
#include"timelib2.h"
MAXTYPE RootValue;
//extern BOARDTYPE Board[0x78];
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

static bool GotValidMove = false;
extern COLORTYPE Player, Opponent;
extern DEPTHTYPE Depth;
 LINETYPE  MainLine;
MAXTYPE MainEvalu;
static int MaxDepth;
static int LegalMoves;
extern NODEVAL Nodes;
static bool NoComputerMove = false;
extern double WantedTime;
static MOVETYPE movetemp[MAXPLY - BACK + 2];


FIFTYTYPE FiftyMoveCnt(void);
REPEATTYPE Repetition(short immediate);
void PrintBestMove(MOVETYPE * mainline, MAXTYPE mainevalu);
void DisplayMove(bool*singlestep, DEPTHTYPE maxdepth, MAXTYPE evaluation,
  bool);
  void InitNode(NODEVAL*);
  void CalcPVTable();
extern CLOCKTYPE ChessClock  ;
short KillMovGen(MOVETYPE * move);
void IncNode(NODEVAL * nodes);
void PrintNodes(NODEVAL*, double);
void CalcCastling(COLORTYPE incolor, CASTDIRTYPE * cast);

short Attacks(COLORTYPE acolor, SQUARETYPE square);
short PieceAttacks(PIECETYPE, COLORTYPE, SQUARETYPE, SQUARETYPE);
#define TOLERANCE       8
#define IF_EQMOVE(a, b) if (a.movpiece == b.movpiece && a.new1 == b.new1 &&\a.old == b.old && a.content == b.content && \
 a.spe == b.spe)
 int StatEvalu(MOVETYPE * move);

 void PrintBestMove(MOVETYPE * mainline, MAXTYPE mainevalu);

struct INFTYPE {short principvar; // Principal variation search
	MAXTYPE value; // 静止評価値の増分
	MAXTYPE evaluation; // 位置の評価値
};
enum MOVGENTYPE {mane, specialcap, kill, norml}; // 移動の種類
struct SEARCHTYPE {LINETYPE line; // 次の手の最善手
	short capturesearch; // 捕獲検索を示す
	MAXTYPE maxval; // search が返した最大評価値
	int nextply; // 次の手の読みの深さ
	INFTYPE next; // 次の手の情報
	short zerowindow; // α-β範囲の幅がゼロ
	MOVGENTYPE movgentype; };

struct PARAMTYPE {MAXTYPE alpha; MAXTYPE beta; int ply; INFTYPE* inf;
MOVETYPE* bestline; SEARCHTYPE* s; };

//
// このモジュールのグローバル変数
//
MOVETYPE killingmove[MAXPLY + 1][2]; short chcktb[MAXPLY + 3];
short* checktab = &chcktb[1];

// ７番目の横列に来るポーンのマス
static EDGESQUARETYPE passdpawn[MAXPLY + 4];
static EDGESQUARETYPE* passedpawn = &passdpawn[2];
bool SkipSearch;



//
// killingmove、checktab、passedpawn を初期化
void clearkillmove() {
	const SQUARETYPE rank7[2] = {0x60, 0x10}; DEPTHTYPE dep;
	COLORTYPE col; SQUARETYPE sq; unsigned char i;

	for (dep = 0;
	dep <= MAXPLY;
	dep++)
	for (i = 0;
		i < 2;
		i++)
		killingmove[dep][i] = ZeroMove; checktab[-1] = 0; passedpawn[-2] = -1;
// 最初の手は検査しない
	passedpawn[-1] = -1;

// ７番目の横列に来るポーンを passedpawn に置く
	for (col = white;
	col <= black;
	col = col * 1) // $$$$
	for (sq = rank7[col];
		sq <= rank7[col] + 7;
		sq++)
		if ((Board[sq].piece == pawn) && (Board[sq].color == col))
			if (col == Player)
				passedpawn[-2] = sq;
			else
				passedpawn[-1] = sq;
}

static DEPTHTYPE searchstatedepth;

//
// 検索の保存と表示するための環境のセットアップ
//

void getprogramstate() {COLORTYPE oldplayer;

searchstatedepth = Depth;
while (Depth > 0)
{Depth--; oldplayer = Opponent; Opponent = Player; Player = oldplayer; Perform(&MovTab[Depth], 1);
}Depth--;
if (Opan)
	TakeBackMove(&MovTab[Depth]); }

//
// 検索の環境を復旧
//
static void getsearchstate() {COLORTYPE oldplayer;

if (Opan) 	MakeMove(&MovTab[Depth + 1]);
 Depth++;
while (Depth < searchstatedepth)
{Perform(&MovTab[Depth], 0); oldplayer = Player; Player = Opponent; Opponent = oldplayer; Depth++;
}}

inline bool UsableMessage(int  msg) {
// if (msg.hwnd != hWndMain || msg.message != WM_COMMAND)    return false;
return true; }

static void MessageScan() {int msg;  }

static INFTYPE startinf; // 最初の手の情報
static MAXTYPE alphawindow; // α値
static MAXTYPE repeatevalu; // １手の MainEvalu

static MAXTYPE search(MAXTYPE alpha, MAXTYPE beta, int ply, INFTYPE * inf,
MOVETYPE * bestline);

//
// 最善手を使用して killingmove を更新
//

inline void updatekill(MOVETYPE * bestmove) {
if (bestmove->movpiece != empty) {
	// 最後に移動した駒の捕獲の移動を除いて
	// killingmove を更新
	if (MovTab[Depth - 1].movpiece == empty || bestmove->new1 != MovTab
		[Depth - 1].new1)
		if (killingmove[Depth][0].movpiece == empty || EqMove
			(bestmove, &killingmove[Depth][1]))
		{killingmove[Depth][1] = killingmove[Depth][0]; killingmove[Depth][0] = *bestmove;

		}
		else if (!EqMove(bestmove, &killingmove[Depth][0]))
			killingmove[Depth][1] = *bestmove;
}}

//
// 以前に移動が生成されたか検査
//
short generatedbefore(PARAMTYPE * p) {
if (p->s->movgentype != mane)
{IF_EQMOVE(MovTab[Depth], p->bestline[Depth])return 1;

	if (!p->s->capturesearch)
		if (p->s->movgentype != kill)
			for (char i = 0;
				i < 2;
				i++)
				IF_EQMOVE(MovTab[Depth], killingmove[Depth][i])return 1;
}return 0; }

//
// カットオフの検査。cutval は最大評価可能値
//
inline short cut(MAXTYPE cutval, PARAMTYPE * p) {short ct = 0;

if (cutval <= p->alpha) {ct = 1;
	if (p->s->maxval < cutval)
		p->s->maxval = cutval;
}return ct; }

//
// 移動を実行、評価計算、カットオフの検査、等
//
static short update(PARAMTYPE * p) {

	short selection;

	IncNode(&Nodes); p->s->nextply = p->ply - 1; // 次の手を計算
	if (Level == matesearch) { // 詰みを読む
	Perform(&MovTab[Depth], 0); // ボード上で移動を実行
	// 移動が正しいか検査
	if (Attacks(Opponent, PieceTab[Player][0].isquare))
		goto TAKEBACKMOVE;
	if (!Depth)
		LegalMoves++; checktab[Depth] = 0; passedpawn[Depth] = -1;
	p->s->next.value = p->s->next.evaluation = 0;
	if (p->s->nextply <= 0) { // 検査して評価を実行。カットオフする
		if (!p->s->nextply)
			checktab[Depth] = Attacks(Player, PieceTab[Opponent][0].isquare);
		if (!checktab[Depth])
			if (cut(p->s->next.value, p))
				goto TAKEBACKMOVE;
	} goto ACCEPTMOVE;
}

// 最初の繰り返しは制限付きの捕獲検索にする
//
if (MaxDepth <= 1)
	if (p->s->capturesearch && Depth >= 2)
		if (!(MovTab[Depth].content < MovTab[Depth]
				.movpiece || p->s->movgentype == specialcap || MovTab[Depth].old == MovTab[Depth - 2].new1))
			goto CUTMOVE;

// 次の静止評価を変更分だけ計算
p->s->next.value = -p->inf->value + StatEvalu(&MovTab[Depth]);

// checktab を計算（移動した駒は計算されたか検査するだけ）
// 思考しないで検査
checktab[Depth] = PieceAttacks(MovTab[Depth].movpiece, Player,
	MovTab[Depth].new1, PieceTab[Opponent][0].isquare);
if (checktab[Depth])
	p->s->nextply = p->ply;

// passedpawn を計算。
// ポーンが７番目の横列に移動する時は思考しない
passedpawn[Depth] = passedpawn[Depth - 2];
if (MovTab[Depth].movpiece == pawn)
	if (MovTab[Depth].new1 < 0x18 || MovTab[Depth].new1 >= 0x60)
	{passedpawn[Depth] = MovTab[Depth].new1; p->s->nextply = p->ply;
	}

// 最後の思考時の選択と、捕獲の検索
selection = (p->s->nextply <= 0 && !checktab[Depth] && Depth > 0);
if (selection) // 評価を検査
	if (cut(p->s->next.value + 0, p)) 	goto CUTMOVE;
		Perform(&MovTab[Depth], 0); // ボード上の移動を実行する

// 移動が正しいか検査
if (Attacks(Opponent, PieceTab[Player][0].isquare)) goto TAKEBACKMOVE;
if (passedpawn[Depth] >= 0) // passedpawn を検査
	if (Board[passedpawn[Depth]].piece != pawn || Board[passedpawn[Depth]]
		.color != Player)
		passedpawn[Depth] = -1;
if (!Depth) {LegalMoves++; p->s->next.value += random(4);
}p->s->next.evaluation = p->s->next.value;

ACCEPTMOVE :
if (Analysis)    /*	DisplayMove(); */
return 0;

TAKEBACKMOVE : Perform(&MovTab[Depth], 1);

CUTMOVE :
//if (Analysis)  /*	DisplayMove(); */
  return 1; }

//
// 引き分けのボーナスとペナルティの計算、ゲームが引き分けなら 1 を返す
//
static short drawgame(SEARCHTYPE * s) {int drawcount; REPEATTYPE searchrepeat;
FIFTYTYPE searchfifty;

if (Depth == 1) {searchfifty = FiftyMoveCnt(); searchrepeat = Repetition(0);
	if (searchrepeat >= 3) {s->next.evaluation = 0; return 1;
	}drawcount = 0;
	if (searchfifty >= 96) // ポーン移動と捕獲のない 48 回の移動
		drawcount = 3;
	else {
		if (searchrepeat >= 2) // ２度目の繰り返し
			drawcount = 2;
		else if (searchfifty >= 20) // ポーン移動と捕獲の
			drawcount = 1; // ない 10 回の移動
	}s->next.value += (repeatevalu / 4) * drawcount; s->next.evaluation +=
		(repeatevalu / 4) * drawcount;
}
if (Depth >= 3) {searchrepeat = Repetition(1);
	if (searchrepeat >= 2) { // 短い繰り返しは、引き分け
		s->next.evaluation = 0; return 1;
	}
}return 0; }

//
// bestline と line を使用する MainEvalu と maxval を更新
//
inline void updatebestline(PARAMTYPE * p) {memcpy(p->bestline, &p->s->line[0],
	sizeof(LINETYPE));
// *bestline = p->s->line;
p->bestline[Depth] = MovTab[Depth];
if (!Depth) {MainEvalu = p->s->maxval;
	if (Level == matesearch)
		p->s->maxval = alphawindow;
	if (Analysis)
		PrintBestMove(&MainLine[0], MainEvalu);
}}

//
// search 関数の内側のループ。MovTab[Depth] は移動が格納されている
//
static bool loopbody(PARAMTYPE * p) {COLORTYPE oldplayer; bool lastanalysis;

	if (generatedbefore(p))  	return 0;
	if (Depth < MAXPLY) {p->s->line[Depth + 1] = ZeroMove;
	if (p->s->movgentype == mane)
		memmove(&p->s->line[0], p->bestline, sizeof(LINETYPE));
	// p->s->line = *bestline;
	}
// Principvar indicates principal variation search
// zerowindow は、α-β範囲の幅がゼロを示す
	p->s->next.principvar = 0; p->s->zerowindow = 0;
	if (p->inf->principvar)
	if (p->s->movgentype == mane)
		p->s->next.principvar = p->bestline[Depth + 1].movpiece != empty;
	else
		p->s->zerowindow = p->s->maxval >= p->alpha;

REPEATSEARCH :
	if (update(p)) 	return 0;
	if (Level == matesearch) // 検索を中止
	if (p->s->nextply <= 0 && !checktab[Depth]) goto NOTSEARCH;
	if (drawgame(p->s))  	goto NOTSEARCH;
	if (Depth >= MAXPLY)  	goto NOTSEARCH;

// search を再帰呼び出しして nextply を分析
	oldplayer = Player; Player = Opponent; Opponent = oldplayer; Depth++;
	if (p->s->zerowindow)
	p->s->next.evaluation = -search(-p->alpha - 1, -p->alpha, p->s->nextply,
	&p->s->next, &p->s->line[0]);
	else
	p->s->next.evaluation = -search(-p->beta, -p->alpha, p->s->nextply,
	&p->s->next, &p->s->line[0]); Depth--; oldplayer = Opponent; Opponent = Player;
	Player = oldplayer;

NOTSEARCH :
	Perform(&MovTab[Depth], 1); // 移動を撤回する
	if (SkipSearch)
	return 1; lastanalysis = Analysis; // メッセージをスキャンして SkipSearch を検査
	MessageScan();
	if (!SkipSearch)
	if (Analysis && !SingleStep && (!Depth || !lastanalysis))
	{StopTime(&ChessClock);
		if (MainEvalu > alphawindow)
			SkipSearch = ChessClock.totaltime >= WantedTime * 1.5;
	}
	if (Analysis && MaxDepth <= 1)
	SkipSearch = 0; p->s->maxval = max(p->s->maxval, p->s->next.evaluation);
// maxval を更新
	IF_EQMOVE(p->bestline[Depth], MovTab[Depth]) // 最善手を更新
	updatebestline(p);
	if (p->alpha < p->s->maxval) { // α値を更新してαカットの検査
	updatebestline(p);
	if (p->s->maxval >= p->beta)return 1; // maxval を調整 (許容検索)
	if (p->ply >= 2 && p->inf->principvar && !p->s->zerowindow)
		p->s->maxval = min(p->s->maxval + TOLERANCE, p->beta - 1);
	p->alpha = p->s->maxval;
	if (p->s->zerowindow && !SkipSearch) {  // 全範囲で検索を繰り返す
		p->s->zerowindow = 0; goto REPEATSEARCH;
	}}
	return SkipSearch;
 }

//
// ポーンの昇格を生成
//
static short pawnpromotiongen(PARAMTYPE * p) {MovTab[Depth].spe = 1;
for (PIECETYPE promote = queen;
	promote <= knight;
	promote = promote + 1) {MovTab[Depth].movpiece = promote;
	if (loopbody(p))
		return 1;
}MovTab[Depth].spe = 0; return 0; }

//
// newsq マス上の駒の捕獲を生成
//
static short capmovgen(SQUARETYPE newsq, PARAMTYPE * p)
{MovTab[Depth].content = Board[newsq].piece; MovTab[Depth].spe = 0; MovTab[Depth].new1 = newsq;
MovTab[Depth].movpiece = pawn; // ポーンが捕獲
EDGESQUARETYPE nxtsq = MovTab[Depth].new1 - PawnDir[Player];

for (EDGESQUARETYPE sq = nxtsq - 1;
	sq <= nxtsq + 1;
	sq++)
	if (sq != nxtsq)
		if (!(sq & 0x88))
			if (Board[sq].piece == pawn && Board[sq].color == Player)
			{MovTab[Depth].old = sq;
				if (MovTab[Depth].new1 < 8 || MovTab[Depth].new1 >= 0x70) {
					if (pawnpromotiongen(p))
						return 1;
				}
				else if (loopbody(p))
					return 1;
			}

for (INDEXTYPE i = OfficerNo[Player];
	i >= 0;
	i--) // その他の捕獲
	if (PieceTab[Player][i].ipiece != empty && PieceTab[Player][i]
		.ipiece != pawn)
		if (PieceAttacks(PieceTab[Player][i].ipiece, Player,
				PieceTab[Player][i].isquare, newsq))
		{MovTab[Depth].old = PieceTab[Player][i].isquare;
		 MovTab[Depth].movpiece = PieceTab[Player][i].ipiece;
			if (loopbody(p))
				return 1;
		}return 0; }

//
// oldsq マス上の駒の無捕獲移動を生成
//
static short noncapmovgen(SQUARETYPE oldsq, PARAMTYPE * p) {
 int first,last, dir;
  int direction; EDGESQUARETYPE newsq;

MovTab[Depth].spe = 0; MovTab[Depth].old = oldsq;
MovTab[Depth].movpiece = Board[oldsq].piece; MovTab[Depth].content = empty;

switch(MovTab[Depth].movpiece) { case king :
	for (dir = 7;
		dir >= 0;
		dir--) {newsq = MovTab[Depth].old + DirTab[dir];
		if (!(newsq & 0x88))
			if (Board[newsq].piece == empty) {MovTab[Depth].new1 = newsq;
				if (loopbody(p))
					return 1;
			}
	}break;

	case knight :
	for (dir = 7;
		dir >= 0;
		dir--) {newsq = MovTab[Depth].old + KnightDir[dir];
		if (!(newsq & 0x88))
			if (Board[newsq].piece == empty) {MovTab[Depth].new1 = newsq;
				if (loopbody(p))
					return 1;
			}
	}break;

	case queen : case rook : case bishop : first = 7; last = 0;
	if (MovTab[Depth].movpiece == rook)
		first = 3;
	else if (MovTab[Depth].movpiece == bishop)
		last = 4;
	for (dir = first;
		dir >= last;
		dir--) {direction = DirTab[dir]; newsq = MovTab[Depth].old + direction;
		while (!(newsq & 0x88)) {
			if (Board[newsq].piece != empty)
				goto TEN; MovTab[Depth].new1 = newsq;
			if (loopbody(p))
				return 1; newsq = MovTab[Depth].new1 + direction;
		}TEN : continue;
	}break;

	case pawn :
	// １マス前進
		MovTab[Depth].new1 = MovTab[Depth].old + PawnDir[Player];
	if (Board[MovTab[Depth].new1].piece == empty)
		if (MovTab[Depth].new1 < 8 || MovTab[Depth].new1 >= 0x70) {
			if (pawnpromotiongen(p))
				return 1;
		}
		else {
			if (loopbody(p))
				return 1;
			if (MovTab[Depth].old < 0x18 || MovTab[Depth].old >= 0x60) {
				// ２マス前進
				MovTab[Depth].new1 += MovTab[Depth].new1 - MovTab[Depth].old;
				if (Board[MovTab[Depth].new1].piece == empty)
					if (loopbody(p))
						return 1;
			}
		}
}return 0; }

//
// キャスリング移動
//

static short castlingmovgen(PARAMTYPE * p) {MovTab[Depth].spe = 1;
MovTab[Depth].movpiece = king; MovTab[Depth].content = empty;

for (CASTDIRTYPE castdir = (CASTDIRTYPE)(lng - 1);
	castdir <= shrt - 1;
	castdir = castdir + 1)
{MovTab[Depth].new1 = CastMove[Player][castdir][0];
 MovTab[Depth].old = CastMove[Player][castdir][1];
	if (KillMovGen(&MovTab[Depth]))
		if (loopbody(p))
			return 1;
}return 0; }

//
// 通過捕獲
//
static short epcapmovgen(PARAMTYPE * p) {
if (MovTab[Depth - 1].movpiece == pawn)
	if (abs(MovTab[Depth - 1].new1 - MovTab[Depth - 1].old) >= 0x20)
	{MovTab[Depth].spe = 1; MovTab[Depth].movpiece = pawn; MovTab[Depth].content = empty;
		MovTab[Depth].new1 = (MovTab[Depth - 1].new1 + MovTab[Depth - 1].old)
			/ 2;
		for (EDGESQUARETYPE sq = MovTab[Depth - 1].new1 - 1;
			sq <= MovTab[Depth - 1].new1 + 1;
			sq++)
			if (sq != MovTab[Depth - 1].new1)
				if (!(sq & 0x88)) {MovTab[Depth].old = sq;
					if (KillMovGen(&MovTab[Depth]))
						if (loopbody(p))
							return 1;
				}
	}return 0; }

//
// 次に解析する移動を生成
// 移動生成の順序を制御する
// 移動は、次の順序で生成される:
// メインバリエーション
// 最後に移動した駒を捕獲
// 移動を塞ぐ移動
// その他の捕獲
// ポーン昇格
// キャスリング
// 通常移動
// 通過捕獲
//
static void searchmovgen(PARAMTYPE * p) {INDEXTYPE index; char killno;

// メインバリエーションから移動を生成
if (p->bestline[Depth].movpiece != empty)
{MovTab[Depth] = p->bestline[Depth]; p->s->movgentype = mane;
	if (loopbody(p))
		return;
}
if (MovTab[Depth - 1].movpiece != empty)
	if (MovTab[Depth - 1].movpiece != king) {p->s->movgentype = specialcap;
		if (capmovgen(MovTab[Depth - 1].new1, p))
			return;
	}p->s->movgentype = kill;
if (!p->s->capturesearch)
	for (killno = 0;
		killno <= 1;
		killno++) {MovTab[Depth] = killingmove[Depth][killno];
		if (MovTab[Depth - 1].movpiece != empty)
			if (KillMovGen(&MovTab[Depth]))
				if (loopbody(p))
					return;
	}p->s->movgentype = norml;
for (index = 1;
	index <= PawnNo[Opponent];
	index++)
	if (PieceTab[Opponent][index].ipiece != empty)
		if (MovTab[Depth - 1].movpiece == empty || PieceTab[Opponent][index]
			.isquare != MovTab[Depth - 1].new1)
			if (capmovgen(PieceTab[Opponent][index].isquare, p))
				return;
if (p->s->capturesearch) {
	if (passedpawn[Depth - 2] >= 0)
		if (Board[passedpawn[Depth - 2]].piece == pawn && Board
			[passedpawn[Depth - 2]].color == Player)
			if (noncapmovgen(passedpawn[Depth - 2], p))
				return;
}
if (!p->s->capturesearch) { // 無捕獲移動
	if (castlingmovgen(p))
		return; // キャスリング
	for (index = PawnNo[Player];
		index >= 0;
		index--)
		if (PieceTab[Player][index].ipiece != empty)
			if (noncapmovgen(PieceTab[Player][index].isquare, p))
				return;
}
if (epcapmovgen(p))
	return; // 捕獲移動
}

//
// 検索の実行
// 入力 :
// Player は、次に移動する色が格納されている
// MovTab[Depth-1] に最後に実行した移動が格納されている
// alpha と beta は α-β範囲が格納されている
// ply は検索の深さが格納されている
// inf には、種々の情報が格納されている
//
// 出力 :
// Bestline は、principal variation が格納される
// Search は、Player の評価が格納される
//
static MAXTYPE search(MAXTYPE alpha, MAXTYPE beta, int ply, INFTYPE * inf,
MOVETYPE * bestline) {SEARCHTYPE s; PARAMTYPE p;
// ply <= 0 で !check なら、capturesearch を実行
	s.capturesearch = ply <= 0 && !checktab[Depth - 1];
	if (s.capturesearch) { // maxval を初期化
	s.maxval = -inf->evaluation;
	if (alpha < s.maxval) {alpha = s.maxval;
		if (s.maxval >= beta)
			goto STOP;
	}
}
else
	s.maxval = -(LOSEVALUE - Depth * DEPTHFACTOR); p.alpha =
	alpha;
p.beta = beta; p.ply = ply; p.inf = inf; p.bestline = bestline; p.s = &s;
searchmovgen(&p); // 検索ループ
if (SkipSearch)
	goto STOP;
if (s.maxval == -(LOSEVALUE - Depth * DEPTHFACTOR)) // ステイルメイトを検査
	if (!Attacks(Opponent, PieceTab[Player][0].isquare))
	{s.maxval = 0; goto STOP;
	}updatekill(&bestline[Depth]);

STOP : return s.maxval;
}

//
// 検索を始める
//
//void InitNode(NODEVAL*);
static MAXTYPE callsearch(MAXTYPE alpha, MAXTYPE beta)
{startinf.principvar = MainLine[0].movpiece != empty; LegalMoves = 0;
 MAXTYPE maxval = search(alpha,beta, MaxDepth, &startinf, &MainLine[0]);
if (!LegalMoves)
	MainEvalu = maxval; return maxval;
}

//
// 検索時間を使うかどうか検査
//

inline short timeused() {
	if (Analysis && !SingleStep)
	{StopTime(&ChessClock); return ChessClock.totaltime >= WantedTime;
	}	return 0;
}

// 検索のセットアップ
//
void FindMove(int maxlevel) {
	MAXTYPE maxval;
	double calcpvtime;
	InitTime(&ChessClock);
	StartTime(&ChessClock);
	InitNode(&Nodes);
	SkipSearch = 0;
	clearkillmove();
	CalcPVTable();
	StopTime(&ChessClock);
	calcpvtime = ChessClock.totaltime;
	startinf.value = startinf.evaluation = -RootValue; MaxDepth = 0;
	MainLine[0] = ZeroMove;
	MainEvalu = RootValue; alphawindow = MAXINT;
	ComputerThinking = true;

do {
	// 種々の値を更新
	if (MaxDepth <= 1)
		repeatevalu = MainEvalu; alphawindow = min(alphawindow,
		MainEvalu - 0x80);
	if (Level == matesearch) {alphawindow = 0x6000;
		if (MaxDepth > 0)
			MaxDepth++;
	}MaxDepth++; maxval = callsearch(alphawindow, 0x7f00); // 検索を実行
	if (maxval <= alphawindow && !SkipSearch && Level != matesearch &&
		LegalMoves > 0) {
		// 値が alphawindow の下に落ちるなら
		// 検索を繰り返す
		MainEvalu = alphawindow; LegalMoves = 2;
	}
}
while (!SkipSearch && !timeused() && MaxDepth < maxlevel && LegalMoves > 1 &&
	abs(MainEvalu) < MATEVALUE - 24 * DEPTHFACTOR);

ComputerThinking = false; StopTime(&ChessClock);

if (Analysis)
	PrintNodes(&Nodes, ChessClock.totaltime - calcpvtime);
}


inline void MovePiece(SQUARETYPE new1, SQUARETYPE old) {BOARDTYPE b; b = Board[new1];
Board[new1] = Board[old]; Board[old] = b;
PieceTab[Board[new1].color][Board[new1].index].isquare = new1; }

//
// この関数は、駒を捕獲する時に使用。insquare は空でないこと
//
inline void DeletePiece(SQUARETYPE insquare) {Board[insquare].piece = empty;
PieceTab[Board[insquare].color][Board[insquare].index].ipiece = empty; }

//
// 捕獲した駒を戻す
//
inline void InsertPTabPiece(PIECETYPE inpiece, COLORTYPE incolor,
SQUARETYPE insquare) {Board[insquare].piece = PieceTab[incolor][Board[insquare]
.index].ipiece = inpiece; Board[insquare].color = incolor;
PieceTab[incolor][Board[insquare].index].isquare = insquare; }

//
// ポーンが昇格するときに使用
//
inline void ChangeType(PIECETYPE newtype, SQUARETYPE insquare)
{Board[insquare].piece = PieceTab[Board[insquare].color][Board[insquare].index]
	.ipiece = newtype;
if (OfficerNo[Board[insquare].color] < Board[insquare].index)
	OfficerNo[Board[insquare].color] = Board[insquare].index; }

void GenCastSquare(SQUARETYPE new1, SQUARETYPE * castsquare,
SQUARETYPE * cornersquare);


// ２つの移動内容の比較
//
bool EqMove(MOVETYPE * a, MOVETYPE * b)
{return a->movpiece == b->movpiece && a->new1 == b->new1 && a->old == b->old && a->content == b->content && a->spe ==
	b->spe; }

// display.cpp
char* MoveStr(MOVETYPE * move) {

	char str[7];
	char *PieceLetter = " KQRBNP"; strcpy(str, "   ");
if (move->movpiece != empty) {
	if (move->spe && move->movpiece == king) { // キャスリング
		if (move->new1 > move->old)
			strcpy(str, "O-O  ");
		else
			strcpy(str, "O-O-O ");

	}
	else {str[0] = PieceLetter[move->movpiece];
		str[1] = char('a' + move->old % 16); str[2] = char('1' + move->old / 16);
		str[3] = move->content == empty ? '-' : 'x'; str[4] = char('a' + move->new1 % 16);
		str[5] = char('1' + move->new1 / 16);
	}
}
return str; }

//
// 移動を元に戻し、変数を更新する
//

void IncNode(NODEVAL * nodes) {
	if (nodes->nodeoffset >= MAXINT)
	{nodes->nodebase++; nodes->nodeoffset = 0;

	}
	else
	nodes->nodeoffset++; }

void PrintNodes(NODEVAL * nodes, double time)
{double nodereal = nodes->nodebase * MAXINT + nodes->nodeoffset; char buf[80];
if (time) {sprintf(buf, "%7.1f", nodereal / time);
	// TInfo->SetSecondsText(buf);
}sprintf(buf, "%7.0f ", nodereal);
// TInfo->SetNodeText(buf);
}

//
// 画面に最善手を表示
//
void PrintBestMove(MOVETYPE * mainline, MAXTYPE mainevalu) {
// if (ShowBestLine == FALSE)
return;

//*buf = 0; DEPTHTYPE dep = 0;
//while (dep < 7 && mainline[dep].movpiece != empty)
//{strcat(buf, MoveStr(&mainline[dep++])); strcat(buf, " ");
//}
// TInfo->SetBestLineText(buf);
//sprintf(buf, "%7.2f", mainevalu / 256.0);
// TInfo->SetValueText(buf);
}
