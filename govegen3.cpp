﻿ #include"genmove3.h"


 //
//  グローバル変数
//
 COLORTYPE Player;
BOARDTYPE Board[0x78];
PIECETYPE Pieces[8]  = { rook, knight, bishop, queen,
						 king, bishop, knight, rook};
 PIECETAB PieceTab[6][16];
 MOVETYPE  ZeroMove = { 8, 8, 0, empty, empty };
 MOVETYPE  KeyMove;

 COLORTYPE Opponent;
int Depth;
int MoveNo;
int UseLib;
;

int OfficerNo[2], PawnNo[2]  ;
bool      Running;
bool ComputerThinking;
  extern int MovTab[];
 //prototype
void InsertPiece(PIECETYPE p, COLORTYPE c, SQUARETYPE sq) ;
void ClearBoard( );
void CalcPieceTab();
void  ColorToPlay(COLORTYPE color);
bool MoveCheck(SQUARETYPE startsq, SQUARETYPE endsq);
int abs(int);
void decode(int m,MOVETYPE*M);
int encode( MOVETYPE);
 //----------------------------

 void GenCastSquare(SQUARETYPE new1, SQUARETYPE* castsquare,
					SQUARETYPE* cornersquare);
 //
//  ボードをクリアしてボードモジュールを初期化
//
void
ClearBoard()
{
  SQUARETYPE square;
  for (square = 0; square <= 0x77; square++) {
    Board[square].piece = empty;
    Board[square].color = white;
  }
  UseLib=10;
  MoveNo=0;
}
 void
ClearIndex()
{
 SQUARETYPE square;
  int col;
  INDEXTYPE index;

  for (square = 0; square <= 0x77; square++)
    Board[square].index = 16;
  for (col = 0; col <= 1; col++)
    for (index = 0; index < 16; index++)
	  PieceTab[col][index].ipiece = empty;
  OfficerNo[white] = PawnNo[white] = -1;
  OfficerNo[black] = PawnNo[black] = -1;

}

//
//  スクラッチから駒テーブルを計算
//
void
CalcPieceTab()
{
  SQUARETYPE square;
	int piece1;

	ClearIndex();

	for (piece1 = 1; piece1 <= 6; piece1++) {
		if (piece1 == pawn) {
			OfficerNo[white] = PawnNo[white];
			OfficerNo[black] = PawnNo[black];
		}
		square = 0;
		do {

			if (Board[square].piece == piece1) {     //0 empty,1 king  .. 6 pawn
	   //		 	 DoPrintf("sq=%02x color=%d piece=%d", square, Board[square].color,piece1);
				PawnNo[Board[square].color]++;
				// assert( 	PawnNo[Board[square].color]<16);
				PieceTab[Board[square].color][PawnNo[Board[square].color]]
					.ipiece = piece1;
				PieceTab[Board[square].color][PawnNo[Board[square].color]]
					.isquare = square;
				Board[square].index = short(PawnNo[Board[square].color]);
			}
			square ^= 0x77;
			if (!(square & 4)) {
				if (square >= 0x70)
					square = (square + 0x11) & 0x73;
				else
					square += 0x10;
			}
		}
		while (square);
	}

}

//
// ボード上の駒を新しい位置に移動
//
inline void MovePiece(SQUARETYPE new1, SQUARETYPE old) {
	BOARDTYPE b;
  b = Board[new1];
  Board[new1] = Board[old];
  Board[old] = b;
  PieceTab[Board[new1].color][Board[new1].index].isquare = new1;
}



//
//  この関数は、駒を捕獲する時に使用。insquare は空でないこと
//
inline void
DeletePiece(SQUARETYPE insquare)
{
  Board[insquare].piece = empty;
  PieceTab[Board[insquare].color][Board[insquare].index].ipiece = empty;
}

//
//  捕獲した駒を戻す
//
inline void
InsertPTabPiece(PIECETYPE inpiece, COLORTYPE incolor,
   SQUARETYPE insquare)
{
    Board[insquare].piece = PieceTab[incolor][Board[insquare].index].ipiece
            = inpiece;
    Board[insquare].color = incolor;
    PieceTab[incolor][Board[insquare].index].isquare = insquare;
}

//
//  ポーンが昇格するときに使用
//
inline void
ChangeType(PIECETYPE newtype, SQUARETYPE insquare)
{
  Board[insquare].piece
	= PieceTab[Board[insquare].color][Board[insquare].index].ipiece = newtype;
  if (OfficerNo[Board[insquare].color] < Board[insquare].index)
      OfficerNo[Board[insquare].color] = Board[insquare].index;
}


//
//  移動を行うか、元に戻し(resetmove が true のときは元に戻し)、
//  ボードと駒テーブルを更新する。Player は移動するプレーヤー
//  の色が、Opponent は相手の色が格納されていなければならない。
//
//  MovePiece、DeletePiece、InsertPTabPiece、ChangeType は
//  ボードモジュールの更新に使用される。
//
void
Perform(MOVETYPE* move, bool resetmove)
{
  SQUARETYPE castsquare, cornersquare, epsquare;

  if (resetmove){        // back is true
	MovePiece(move->old, move->new1);
    if (move->content != empty)
      InsertPTabPiece(move->content, Opponent, move->new1);

  } else {                         //play
    if (move->content != empty)
      DeletePiece(move->new1);
    MovePiece(move->new1, move->old);
  }

  if (move->spe) {
    if (move->movpiece == king) {
	  GenCastSquare(move->new1, &castsquare, &cornersquare);
      if (resetmove)
        MovePiece(cornersquare, castsquare);
      else
        MovePiece(castsquare, cornersquare);

    } else {
      if (move->movpiece == pawn) {
        epsquare = (move->new1 & 7) + (move->old & 0x70); // 通過捕獲
		if (resetmove)
          InsertPTabPiece(pawn, Opponent, epsquare);
        else
          DeletePiece(epsquare);
      } else {
        if (resetmove)
          ChangeType(pawn, move->old);
        else
          ChangeType(move->movpiece,move->new1);
      }
    }
  }

}

//
// ２つの移動内容の比較
//
bool
EqMove(MOVETYPE* a, MOVETYPE* b)
{
  return a->movpiece == b->movpiece && a->new1 == b->new1 &&
	a->old == b->old && a->content == b->content && a->spe == b->spe;
  }

static  void
InsertPiece(PIECETYPE p, COLORTYPE c, SQUARETYPE sq)
{

  Board[sq].piece = p;
  Board[sq].color = c;
}
//--------------------------------------------------------
void initBoardandPiece()

{
  ClearBoard();
 Running = false;


   Depth =0;

   //DoPrintf("dep -1 %d",Depth);

  MovTab[0] = 0;
  //ZeroMove;    //DEAD!!!!!!!!!!!!!!!!!!!!
 //  DoPrintf("dep -1 %d  %08x ",Depth, &Depth);


  for (int i = 0; i < 8; i++) {
	InsertPiece(Pieces[i], white, i);
	InsertPiece(pawn, white, i + 0x10);
	InsertPiece(pawn, black, i + 0x60);
	InsertPiece(Pieces[i], black, i + 0x70);
  }

    MoveNo = 0;
 CalcPieceTab();


  Player = white;

 // ClearDisplay();
//  InitDisplay();
 // ColorToPlay(Player);        display.cpp
  Opponent = black;
}
//----------------------------------------
   // ----------------------------------------------------------------------------
// ObjectWindows - (C) Copyright 1991, 1993 by Borland International
// ----------------------------------------------------------------------------



void DoPrintf(char*szFormat, ...);
;
NODEVAL Nodes;

bool MultiMove, AutoPlay, SingleStep;
//
// グローバル変数
//
struct ATTACKTABTYPE attack[240];
struct ATTACKTABTYPE *AttackTab = &attack[120];
char  BitTab[7] = {
	0, 1, 2, 4, 8, 0x10, 0x20
};
int DirTab[8] = {
	1, -1, 0x10, -0x10, 0x11, -0x11, 0x0f, -0x0f
};
int KnightDir[8] = {
	0x0E, -0x0E, 0x12, -0x12, 0x1f, -0x1f, 0x21, -0x21
};
int PawnDir[2] = {
	0x10, -0x10
};
MOVETYPE Next;
extern PIECETYPE *piece;
int BufCount, BufPnt;
MOVETYPE Buffer[81];
extern  int Depth;
extern BOARDTYPE Board[0x78];
int PVTable[2][7][0x78];
extern  MOVETYPE ZeroMove;

 COLORTYPE ProgramColor;
//^-------------------------------------------------
int  CastMove[2][2][2] = {   2, 4,  6, 4,
0x72, 0x74, 0x76, 0x74  };

extern PIECETAB PieceTab[6][16];

static MOVETYPE movetemp[MAXPLY - BACK + 2];



void CalcAttackTab() {
	for (int sq = -0x77; sq <= 0x77; sq++) {
		AttackTab[sq].pieceset = 0;
		AttackTab[sq].direction = 0;
	}

	for (DIRTYPE dir = 7; dir >= 0; dir--) {
		for (unsigned char i = 1; i < 8; i++) {
			if (dir < 4)
				AttackTab[DirTab[dir] * i].pieceset = SETOFPIECE
					(BitTab[queen] + BitTab[rook]);
			else
				AttackTab[DirTab[dir] * i].pieceset = SETOFPIECE
					(BitTab[queen] + BitTab[bishop]);
			AttackTab[DirTab[dir] * i].direction = DirTab[dir];
		}
		AttackTab[DirTab[dir]].pieceset += SETOFPIECE(BitTab[king]);
		AttackTab[KnightDir[dir]].pieceset = SETOFPIECE(BitTab[knight]);
		AttackTab[KnightDir[dir]].direction = KnightDir[dir];
	}
}

//
// asquare のマス上の apiece の駒が、square のマスを攻撃できるかどうか計算
//
short PieceAttacks(PIECETYPE apiece, COLORTYPE acolor, SQUARETYPE asquare,
	SQUARETYPE square) {
	int x = square - asquare;
	if (apiece == pawn) // ポーンの攻撃
		return abs(x - PawnDir[acolor]) == 1;

	// 他の攻撃: 駒はそのマスに移動可能か?
	if (AttackTab[x].pieceset & BitTab[apiece]) {
		if (apiece == king || apiece == knight)
			return 1;

		// 間にブロックしている駒があるか?
		EDGESQUARETYPE sq = asquare;
		do {
			sq += AttackTab[x].direction;
		}
		while (sq != square && Board[sq].piece == 0);
		return sq == square;
	}
	return 0;
}

//
// acolor 側が square のマスのポーンで攻撃できるか計算
//
short PawnAttacks(COLORTYPE acolor, SQUARETYPE square) {
	EDGESQUARETYPE sq = square - PawnDir[acolor] - 1; // 左のマス
	if (!(sq & 0x88))
		if (Board[sq].piece == pawn && Board[sq].color == acolor)
			return 1;

	sq += 2; // 右のマス
	if (!(sq & 0x88))
		if (Board[sq].piece == pawn && Board[sq].color == acolor)
			return 1;

	return 0;
}

//
// acolor 側が square のマスを攻撃できるかどうか計算
//
short Attacks(COLORTYPE acolor, SQUARETYPE square) {
	if (PawnAttacks(acolor, square)) // ポーンの攻撃
		return 1;

	// 他の攻撃:  小さい駒始めてすべての駒を検査
	for (INDEXTYPE i = OfficerNo[acolor]; i >= 0; i--)
		if (PieceTab[acolor][i].ipiece != 0)
			if (PieceAttacks(PieceTab[acolor][i].ipiece, acolor,
					PieceTab[acolor][i].isquare, square))
				return 1;
	return 0;
}

//
// square のマス上に inpiece の駒があって、一度も動いたことがないかどうか検査
//
short Check(SQUARETYPE square, PIECETYPE inpiece, COLORTYPE incolor) {
	if (Board[square].piece == inpiece && Board[square].color == incolor) {
		DEPTHTYPE dep = DEPTHTYPE(Depth - 1);
		while (MovTab[dep] != 0) {
			if (MovTab[dep]&0x70 == square*16)
				return 0;
			dep--;
		}
		return 1;
	}
	return 0;
}

//
// incolor の色がキャスリングできるかどうか検査
//
void CalcCastling(COLORTYPE incolor, CASTDIRTYPE *cast) {
	SQUARETYPE square = 0;

	if (incolor == black)
		square = 0x70;
	*cast = zero;
	if (Check(square + 4, king, incolor)) { // キングを検査
		if (Check(square, rook, incolor))
			(*cast) = (*cast) + lng; // A 列のルークを検査
		if (Check(square + 7, rook, incolor))
			(*cast) = (*cast) + shrt; // H 列のルークを検査
	}
}

//
// 移動が、ポーン移動または捕獲移動か検査
//
inline short RepeatMove(MOVETYPE* move) {
	return move->movpiece != 0 && move->movpiece != pawn &&
		move->content == 0 && !move->spe;
}

// ----------------------------------------------------------------------------

//
// 最後に捕獲するか、ポーンが移動してからの移動数を数える
// fiftymovecnt = 100 なら、ゲームは引き分け
//
FIFTYTYPE FiftyMoveCnt() {
	FIFTYTYPE cnt = 0;
   //	while (RepeatMove(&MovTab[Depth - cnt]))
		cnt++;
	return cnt;
}

//
// 以前にその位置が何回発生したかを計算。
// Repetition が３を返したとき、ゲームは引き分け。
// MovTab[back..Depth] に最後に実行した移動が格納されている
// immediate がセットされていれば、直前の繰り返しのみ検査
//
#ifdef hhhh
REPEATTYPE Repetition(short immediate) {
	DEPTHTYPE lastdep, compdep, tracedep, checkdep, samedepth;
	SQUARETYPE tracesq, checksq;
	REPEATTYPE repeatcount;

	repeatcount = 1;
	lastdep = samedepth = DEPTHTYPE(Depth + 1); // 現在の位置
	compdep = DEPTHTYPE(samedepth - 4); // 比較する最初の位置

	// MovTab[lastdep..Depth] には以前の関連した移動が格納されている
	while (RepeatMove(&MovTab[lastdep - 1]) && (compdep < lastdep || !immediate)
		)
		lastdep--;
	if (compdep < lastdep)
		return repeatcount;
	checkdep = samedepth;
	for (; ; ) {
		checkdep--;
		checksq = MovTab[checkdep].new1;
		for (tracedep = DEPTHTYPE(checkdep + 2); tracedep < samedepth;
			tracedep += DEPTHTYPE(2))
			if (MovTab[tracedep].old == checksq)
				goto TEN;

		// 以前の移動を取り消すことになるか、移動をさかのぼって検索
		tracedep = checkdep;
		tracesq = MovTab[tracedep].old;
		do {
			if (tracedep - 2 < lastdep)
				return repeatcount;
			tracedep -= (DEPTHTYPE)2;
			// 以前に移動した駒か検査
			if (tracesq == MovTab[tracedep].new1)
				tracesq = MovTab[tracedep].old;
		}
		while (tracesq != checksq || tracedep > compdep + 1);
		if (tracedep < compdep) { // compdep を調整
			compdep = tracedep;
			if ((samedepth - compdep) % 2 == 1) {
				if (compdep == lastdep)
					return repeatcount;
				compdep--;
			}
			checkdep = samedepth;
		}
		// samedep と compdep 間のすべての移動は検査され、
		// 繰り返しが検出された
	TEN:
		if (checkdep <= compdep) {
			repeatcount++;
			if (compdep - 2 < lastdep)
				return repeatcount;
			checkdep = samedepth = compdep;
			compdep -= (DEPTHTYPE)2;
		}
	}
}
 #endif
//
// 移動可能か検査
//
// 入力:
// move は、違う位置で生成された正しい移動の完全な記述が格納されている
// MovTab[Depth-1] に最後に実行した移動が格納されている
//
// 出力:
// KillMovGen は、移動可能かどうかを返す
//
short KillMovGen(MOVETYPE* move) {
	SQUARETYPE castsq;
	PIECETYPE promote;
	CASTDIRTYPE castdir;
	CASTTYPE cast;
	short killmov;

	killmov = 0;
	if (move->spe && move->movpiece == king) {
		CalcCastling(Player, &cast); // キャスリング
		if (move->new1 > move->old)
			castdir = shrt;
		else
			castdir = lng;

		if (cast & castdir) { // キングとルークは以前に移動したことがあるか
			castsq = (int)((move->new1 + move->old) / 2);
			// マスは空か?
			if (Board[move->new1].piece == 0)
				if (Board[castsq].piece == 0)
					if (move->new1 > move->old || Board[move->new1 - 1]
						.piece == 0)
					// マスは攻撃されていないか
						if (!Attacks(Opponent, move->old))
							if (!Attacks(Opponent, move->new1))
								if (!Attacks(Opponent, castsq))
									killmov = 1;
		}

	}
	else {
		if (move->spe && move->movpiece == pawn) {
			// 捕獲移動
			// 相手は、２マス移動�
			MOVETYPE mmmm;//�たのか?
			decode(MovTab[Depth - 1],&mmmm);
			if (mmmm.movpiece == pawn)
				if (abs(mmmm.new1 - mmmm.old) >= 0x20)
					if (Board[move->old].piece == pawn && Board[move->old]
						.color == Player)
						killmov = move->new1 ==
							(mmmm.new1 + mmmm.old) / 2;
		}
		else {
			if (move->spe) { // 通常のテスト
				promote = move->movpiece; // ポーン昇格
				move->movpiece = pawn;
			}

			// Old と New1 のマスの内容は正しいか?
			if (Board[move->old].piece == move->movpiece)
				if (Board[move->old].color == Player)
					if (Board[move->new1].piece == move->content)
						if (move->content == 0 || Board[move->new1]
							.color == Opponent) {
							if (move->movpiece == pawn) { // 移動可能か?
								if (abs(move->new1 - move->old) < 0x20)
									killmov = 1;
								else
									killmov = Board[(move->new1 + move->old)
								/ 2].piece == 0;
							}
							else
								killmov = PieceAttacks
									(move->movpiece, Player, move->old, move->new1);
						}
			if (move->spe)
				move->movpiece = promote;
		}
	}
	return killmov;
}

//
// 移動をバッファに格納
//
static void Generate() {
	BufCount++;
	Buffer[BufCount] = Next;
   //	 DoPrintf("gener=%02x %02x",Next.new1,Next.old);
}

//
// ポーンの昇格を生成
//
static void PawnPromotionGen() {
	Next.spe = 1;
	for (PIECETYPE promote = queen; promote <= knight; promote = promote + 1) {
		Next.movpiece = promote;
		Generate();
	}
	Next.spe = 0;
}

//
// PieceTab を使って new1 の駒の捕獲移動を生成
//
static void CapMovGen() {
	Next.spe = 0;
	Next.content = Board[Next.new1].piece;
	Next.movpiece = pawn;
	EDGESQUARETYPE nextsq = Next.new1 - PawnDir[Player];
	for (EDGESQUARETYPE sq = nextsq - 1; sq <= nextsq + 1; sq++)
		if (sq != nextsq)
			if ((sq & 0x88) == 0)
				if (Board[sq].piece == pawn && Board[sq].color == Player) {
					Next.old = sq;
					if (Next.new1 < 8 || Next.new1 >= 0x70)
						PawnPromotionGen();
					else
						Generate();
				}

	// その他の捕獲、小さい駒から開始
	for (INDEXTYPE i = OfficerNo[Player]; i >= 0; i--)
		if (PieceTab[Player][i].ipiece != 0 && PieceTab[Player][i]
			.ipiece != pawn)
			if (PieceAttacks(PieceTab[Player][i].ipiece, Player,
					PieceTab[Player][i].isquare, Next.new1)) {
				Next.old = PieceTab[Player][i].isquare;
				Next.movpiece = PieceTab[Player][i].ipiece;
				Generate();
			}
}

//
// old の駒の無捕獲移動を生成
//
static void NonCapMovGen() {
	DIRTYPE first, last, dir;
	int direction;
	EDGESQUARETYPE newsq;

	Next.spe = 0;
	Next.movpiece = Board[Next.old].piece;
	Next.content = 0;
	// DoPrintf("non cap piece=%d", Next.movpiece);
	switch(Next.movpiece) {
	case king:
		for (dir = 7; dir >= 0; dir--) {
			newsq = Next.old + DirTab[dir];
			if (!(newsq & 0x88))
				if (Board[newsq].piece == 0) {
					Next.new1 = newsq;
					Generate();
				}
		}
		break;

	case knight:
		for (dir = 7; dir >= 0; dir--) {
			newsq = Next.old + KnightDir[dir];
			if (!(newsq & 0x88))
				if (Board[newsq].piece == empty) {
					Next.new1 = newsq;
					// DoPrintf("knight sq=%02x pirce=%d ",newsq,Board[newsq].piece);
					Generate();
				}
		}
		break;

	case queen:
	case rook:
	case bishop:
		first = 7;
		last = 0;
		if (Next.movpiece == rook)
			first = 3;
		if (Next.movpiece == bishop)
			last = 4;
		for (dir = first; dir >= last; dir--) {
			direction = DirTab[dir];
			newsq = Next.old + direction;
			// その方向で無捕獲移動を生成
			while (!(newsq & 0x88)) {
				if (Board[newsq].piece != 0)
					goto TEN;
				Next.new1 = newsq;
				Generate(); ;
				newsq = Next.new1 + direction;
			}
		TEN:
			continue;
		}
		break;

	case pawn:
		Next.new1 = Next.old + PawnDir[Player]; // １マス前進
		if (Board[Next.new1].piece == 0) {
			if (Next.new1 < 8 || Next.new1 >= 0x70)
				PawnPromotionGen();
			else {
				Generate();
				if (Next.old < 0x18 || Next.old >= 0x60) {
					Next.new1 += (Next.new1 - Next.old); // ２マス前進
					if (Board[Next.new1].piece == 0)
						Generate();
				}
			}

		}
	}
}

//
// 移動生成
// InitMovGen は、すべての可能な移動を生成して、Buffer に格納する。
// MovGen は、１つ１つ移動を生成してそれらを Next に格納する。
//
// 入力:
// Player には移動する色が格納されている
// MovTab[Depth-1] に最後に実行した移動が格納されている
//
// 出力:
// Buffer に生成された移動を格納する
//
// 移動は次の順序で生成される:
// 捕獲移動
// キャスリング移動
// 無捕獲移動
// 通過捕獲
//

int InitMovGen(int movelist[]) {
	CASTDIRTYPE castdir;
	EDGESQUARETYPE sq;
	INDEXTYPE index;
	int i;

	BufCount = BufPnt = 0;
	// 大きい駒の捕獲から開始してすべての捕獲を生成
	for (index = 1; index <= PawnNo[Opponent]; index++)
		if (PieceTab[Opponent][index].ipiece != 0) {
			Next.new1 = PieceTab[Opponent][index].isquare;
			CapMovGen();
		}

	Next.spe = 1;
	Next.movpiece = king;
	Next.content = 0;
	for (int castdir = CASTDIRTYPE(lng - 1); castdir <= shrt - 1;
		castdir = castdir + 1) {
		Next.new1 = CastMove[Player][castdir][0];
		Next.old = CastMove[Player][castdir][1];
		if (KillMovGen(&Next))
			Generate();
	}

	// 無捕獲移動を生成、ポーンから開始
	for (index = PawnNo[Player]; index >= 0; index--) {
	   //	DoPrintf("index=%d ipece=%d", index, PieceTab[Player][index].ipiece);
		if (PieceTab[Player][index].ipiece != empty) {
			Next.old = PieceTab[Player][index].isquare;
			NonCapMovGen();
		   //	 DoPrintf("noncap z=%02x player =%d index=%d zbuf=%d",
		  //	 Next.old,Player,index,BufCount);
		}
	}
	MOVETYPE m;decode( MovTab[Depth - 1],&m);
	if (m.movpiece == pawn) // 通過捕獲
		if (abs(m.new1 - m.old) >= 0x20) {
			Next.spe = 1;
			Next.movpiece = pawn;
			Next.content = 0;
			Next.new1 = (m.new1 + m.old) / 2;
			for (sq = m.new1 - 1;
				sq <= m.new1 + 1; sq++)
				if (sq != m.new1)
					if (!(sq & 0x88)) {
						Next.old = sq;
						if (KillMovGen(&Next))
							Generate();
					}
		}
  for(int i=0;i<BufCount;i++){
	  movelist[i]=encode(Buffer[i]);
  }
  return BufCount;
 //	DoPrintf("buf=%d", BufCount);
}

//
// バッファから取得した次の移動を Next に置く。移動が無ければ、
// ZeroMove を生成。
//
void MovGen() {
	if (BufPnt >= BufCount)
		Next = ZeroMove;
	else {
		BufPnt++;
		Next = Buffer[BufPnt];
	}
   //Perform(&Next,1);
}
void GenCastSquare(SQUARETYPE new1, SQUARETYPE* castsquare,
					SQUARETYPE* cornersquare)
{
  if ((new1 & 7) >= 4) {        // ショートキャッスル
	*castsquare = new1 - 1;
	*cornersquare = new1 + 1;

  } else {                      // ロングキャッスル
	*castsquare = new1 + 1;
	*cornersquare = new1 - 2;
  }
}


//  移動を元に戻し、変数を更新する
//
void
TakeBackMove()
{
MOVETYPE move;
 int sm=MovTab[Depth-1];
 decode(sm,&move);
  ProgramColor = Opponent;
  Opponent = Player;
  Player = ProgramColor;
  Perform(&move, 1);

  MoveNo--;
  Depth--;

}
// makeMove()
//                    talk.cpp
void MakeMove(int move) {

Depth++;
MoveNo++;
MovTab[Depth]=move;
MOVETYPE m;decode(move,&m);
Perform(&m, 0);

ProgramColor = Opponent;
Opponent = Player; Player = ProgramColor;
//DoPrintf("MakeMove Deth=%d %d newz=%02x newz=%02x ",Depth,Player,
//move->new1,move->old);





}

int encode( MOVETYPE m){
   int xy;
   xy= m.new1*256+m.old;
   xy|=m.movpiece<<16;
   xy|=m.content<<24;
   if(m.spe==2) xy|=0x20000000;
   if(m.spe==1) xy|=0x10000000;
   if(m.spe==3) xy|=0x30000000;

	return xy;
}
void decode(int m,MOVETYPE*M){
	M->old= m&0xff;
	M->old= (m>>18)&0xff ;
	M->movpiece= (m>>16)&0xff ;
	M->content= (m>>24)&0x0f ;
	M->spe= (m>>28)&0x03    ;
}
