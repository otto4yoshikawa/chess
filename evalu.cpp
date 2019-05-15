
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
#define    TOLERANCE        8  //  ���e�͈�
#define    EXCHANGEVALUE    32
// ����A�O��(�|�[���ȊO��)��ƌ������鎞�̒l
#define    ISOLATEDPAWN     20
// �Ǘ������|�[���B�Q�̌Ǘ������|�[���� 3 * 20
#define    DOUBLEPAWN       8  //  �Q�d�|�[��
#define    SIDEPAWN        6   //  �[�ɂ���|�[��
#define    CHAINPAWN        3  //  �|�[���ɃJ�o�[����Ă���
#define    COVERPAWN        3  //  �|�[�����J�o�[���Ă���
#define    NOTMOVEPAWN      2  //  �|�[���ړ��̃y�i���e�B
#define    BISHOPBLOCKVALUE  20
// d2/e2 �|�[�����r�V���b�v���u���b�N���Ă���Ƃ��̃y�i���e�B
#define    ROOKBEHINDPASSPAWN 16  //  ���̃��[�N���|�[����ʉ߂����Ƃ��̃{�[�i�X

typedef unsigned char FILETYPE; // �c��ԍ�
typedef unsigned char RANKTYPE; // ����ԍ�
typedef unsigned char SETOFFILE;
typedef struct {
	SETOFFILE one, dob;
}PAWNBITTYPE[2];



void CalcCastling(COLORTYPE incolor, CASTDIRTYPE * cast);

void GenCastSquare(SQUARETYPE new1, SQUARETYPE * castsquare,
	SQUARETYPE * cornersquare);

char* MoveStr(MOVETYPE*);
// �O���[�o���ϐ�
//

LEVELTYPE Level;
//extern NODEVAL Nodes;

// ���̃��W���[���̃O���[�o���ϐ�
//
int PieceValue[7] = {
	0, 0x1000, 0x900, 0x4c0, 0x300, 0x300, 0x100
};
const char distan[8] = {
	3, 2, 1, 0, 0, 1, 2, 3
};
// �|�[���̒l�͉���Əc��̍��v�B
// �c��̒l�� PawnFileFactor * (Rank Number + 2) �Ɠ���
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
}; // �L���X�����O�̒l

const SETOFFILE filebittab[8] = {
	1, 2, 4, 8, 0x10, 0x20, 0x40, 0x80
};
PAWNBITTYPE pawnbitt[MAXPLY + 2];
PAWNBITTYPE *pawnbit = &pawnbitt[1];
int totalmaterial, pawntotalmaterial, material;
// �Q�[���̕����l���x��
// (���ՁA���� = 43 - 32�A�I�� = 0)
static int materiallevel;
const unsigned char squarerankvalue[8] = {
	0, 0, 0, 0, 1, 2, 4, 4
};
#define  MAXINT  32000
static int tempcolor;
static int temp, temp2; // �ꎞ�g�p
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
// b �̃Z�b�g����Ă���r�b�g�̐��𐔂���
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
// pawnbit[depth][color] �̃|�[���\���̒l���v�Z
//
static int pawnstrval(DEPTHTYPE depth, COLORTYPE color) {
	SETOFFILE iso; // �Ǘ������|�[�����܂ޏc��

	iso = SETOFFILE(pawnbit[depth][color].one&~
		((pawnbit[depth][color].one << 1) | (pawnbit[depth][color].one >> 1)));
	return-(count(pawnbit[depth][color].dob) * DOUBLEPAWN + count(iso)
		* ISOLATEDPAWN + count(iso & pawnbit[depth][color].dob) * ISOLATEDPAWN * 2);
}

//
// �}�X��̋�̒l���v�Z
//
inline int PiecePosVal(PIECETYPE piece, COLORTYPE color, SQUARETYPE square) {
	return PieceValue[piece] + PVTable[color][piece][square];
}

short mating; // �l�ݕ]���֐��Ŏg����

//
// �Î~�]���֐��̂��߂ɋ�̒l�e�[�u���iPiece-Value Table�j���v�Z
//
void CalcPVTable() {
	typedef SETOFFILE PAWNTABTYPE[8];

	PAWNTABTYPE pawntab[2];
	// �Î~�|�[���\���]���̂��߂̃r�b�g�e�[�u��
	SETOFFILE pawnfiletab, bit, oppasstab, behindoppass, leftsidetab,
	rightsidetab, sidetab, leftchaintab, rightchaintab, chaintab, leftcovertab,
	rightcovertab;

	// �}�X�̍U���d�v��
	char attackvalue[2][0x78];
	// �}�X�Ő��䂳���}�X�̒l
	int pvcontrol[2][5][0x78];

	COLORTYPE losingcolor; // �l�܂��F
	int posval; // ��̈ʒu�l
	int attval; // �}�X�̍U���l
	FILETYPE line; // ��̏c��
	RANKTYPE rank; // ��̉���
	char dist, kingdist; // ���S�܂ł̋����A����̃L���O�܂ł̋���
	CASTTYPE cast; // �L���X�����O�\
	short direct; // ����/�ԐڍU��
	int cnt; // �U���l�p�J�E���^
	int strval; // �|�[���\���l
	int color, oppcolor; // �����̐F�A����̐F
	int piececount; // ��p�J�E���^
	SQUARETYPE square; // �}�X�p�J�E���^
	int dir;
	// �����p�J�E���^
	EDGESQUARETYPE sq; // �}�X�p�J�E���^
	int temp, temp2; // �ꎞ�g�p
	int PVTable[2][7][0x78];
	// �����l���v�A�|�[���̕����l�̍��v�A�����l�̌v�Z
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
	// �ア�v���C���[���Q�̃r�V���b�v�̉��l�����Ⴍ�A
	// ���Ȃ��Ƃ��r�V���b�v�������[�N���L���Ȃ� mating ���Z�b�g����
	if (material < 0)
		losingcolor = white;
	else
		losingcolor = black;
	mating = ((totalmaterial - abs(material)) / 2 <= PieceValue[bishop] * 2) &&
		(abs(material) >= PieceValue[rook] - PieceValue[bishop]);
	// �U���l�̌v�Z�i�e�}�X�̏d�v���j
	for (rank = 0; rank < 8; rank++)
		for (line = 0; line < 8; line++) {
			square = (rank << 4) + line;
			attval = max(0, 8 - 3 * (distan[rank] + distan[line]));
			// �����̏d�v��
			// ����̏d�v��
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
		// ����̃L���O�̉��̂W�̃}�X�͏d�v
			for (dir = 0; dir < 8; dir++) {
				sq = PieceTab[oppcolor][0].isquare + DirTab[dir];
				if (!(sq & 0x88))
					attackvalue[color][sq] += char
						(((12 * (materiallevel + 8)) >> 5));
			}
	}

	// PVControl ���v�Z
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
					// �}�X�̂��ׂĂ̕�������̍U���𐔂���B
					// �}�X�̍U���̒l�� ATTACKVAL �ɂ���B
					// �ԐړI�ȍU���i�Ⴆ�΁A���̃��[�N���
					// �������[�N�̍U���j�́A���ʂ̍U���Ƃ�
					// �Đ����A���̋��ʂ����U���́A������
					// ������B
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

	// PVTable ���v�Z�B�l�ɂ��l
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
					posval = 0; // �}�X��̋�̈ʒu
					// �̒l���v�Z
					if (mating && piececount != pawn) {
						if (piececount == king)
							if (color == losingcolor) { // �l�ݕ]��
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
						// �ʏ�̕]���֐�
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

	// pawntab �̌v�Z (�ǂ̃}�X�Ƀ|�[�������邩������)

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
	for (color = white; color <= black; ((int)color)++) { // pawnbit �̏�����
		pawnbit[-1][color].one = pawnbit[-1][color].dob = 0;
		for (rank = 1; rank < 7; rank++) {
			temp = pawntab[color][rank];
			pawnbit[-1][color].dob |= SETOFFILE
				((pawnbit[-1][color].one & temp));
			pawnbit[-1][color].one |= SETOFFILE(temp);
		}
	}

	// �|�[���̍\���̒l���v�Z
	RootValue = pawnstrval(-1, Player) - pawnstrval(-1, Opponent);

	// �|�[���̍\���ɂ��Î~�l�̌v�Z
	for (color = white; color <= black; ((int)color)++) {
		oppcolor = (1 - color);
		pawnfiletab = leftsidetab = rightsidetab = behindoppass = 0;
		oppasstab = 0xff;
		for (rank = 1; rank < 7; rank++) {
			// �|�[���ɒʉ߂���鑊��̃|�[���̃}�X
			oppasstab &= SETOFFILE((~(pawnfiletab | leftsidetab | rightsidetab))
				);
			// ����̃|�[�����|�[����ʉ߂������̃}�X
			behindoppass |= (oppasstab & pawntab[oppcolor][7 - rank]);
			// �|�[���ɂ���ăJ�o�[����Ă���}�X
			leftchaintab = leftsidetab;
			rightchaintab = rightsidetab;
			pawnfiletab = pawntab[color][rank]; // �|�[���̂���}�X
			// �����ɕ���ł���|�[���̃}�X
			leftsidetab = SETOFFILE((pawnfiletab << 1) & 0xff);
			rightsidetab = SETOFFILE((pawnfiletab >> 1) & 0xff);
			sidetab = leftsidetab | rightsidetab;
			chaintab = leftchaintab | rightchaintab;
			// �|�[�����J�o�[����}�X
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

	// �����̃|�[�����r�V���b�v���u���b�N���Ă���y�i���e�B�̌v�Z
	for (sq = 3; sq < 5; sq++) {
		if (Board[sq + 0x10].piece == pawn && Board[sq + 0x10].color == white)
			PVTable[white][bishop][sq + 0x20] -= BISHOPBLOCKVALUE;
		if (Board[sq + 0x60].piece == pawn && Board[sq + 0x60].color == black)
			PVTable[black][bishop][sq + 0x50] -= BISHOPBLOCKVALUE;
	}
	for (square = 0x77; square >= 0; square--) // RootValue ���v�Z
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
// pawnbit ���X�V���A�|�[���� line �����苎�����Ƃ��̒l���v�Z
//
inline int decpawnstrval(COLORTYPE color, FILETYPE line) {
	temp = ~filebittab[line];
	pawnbit[Depth][color].one = SETOFFILE((pawnbit[Depth][color].one & temp)
		| pawnbit[Depth][color].dob);
	pawnbit[Depth][color].dob &= SETOFFILE(temp);
	return int(pawnstrval(Depth, color) - pawnstrval(DEPTHTYPE(Depth - 1),
			color));
}

// pawnbit ���X�V���A�|�[���� old ���� new1 �̏c���
// �ړ�����Ƃ��̒l���v�Z
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
// �ړ��̐Î~�]�����v�Z
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
			epsquare = move->new1 - PawnDir[Player]; // �ʉߕߊl
			value = PiecePosVal(pawn, Opponent, epsquare);

		}
		else // �|�[���̏��i
			value = PiecePosVal(move->movpiece, Player, move->old) - PiecePosVal
				(pawn, Player, move->old) + decpawnstrval
				(Player, FILETYPE(move->old & 7));

	if (move->content != empty) { // �ʏ�ړ�
		value += PiecePosVal(move->content, Opponent, move->new1);
		// ���̋�ƌ�������Ƃ��̕����l�y�i���e�B
		if (abs(MainEvalu) >= 0x100)
			if (move->content != pawn)
				if ((ProgramColor == Opponent) == (MainEvalu >= 0))
					value -= EXCHANGEVALUE;
	}
	pawnbit[Depth][black] = pawnbit[Depth - 1][black]; // pawnbit���v�Z
	pawnbit[Depth][white] = pawnbit[Depth - 1][white];
	if (move->movpiece == pawn && (move->content != empty || move->spe))
		value += movepawnstrval(Player, FILETYPE(move->new1 & 7), FILETYPE
		(move->old & 7));
	if (move->content == pawn || move->spe && move->movpiece == pawn)
		value -= decpawnstrval(Opponent, FILETYPE(move->new1 & 7));
	// �ړ��̒l���v�Z
	return value + PiecePosVal(move->movpiece, Player, move->new1) - PiecePosVal
		(move->movpiece, Player, move->old);
}

