#ifndef chessrule_H
#define chessrule_H
#include<vector>
#include<iostream>
using namespace std;

const MATEVALUE  = 0x7C80;
const DEPTHFACTOR  = 0x80;
const COMMANDLENGTH  = 10;
const MAXLEN         = 64;
const SQUARE_SIZE = 36;
const MAXBDSIZE = 8;
const MYFRAMESIZE = 3;
enum SPE {normal1,kingsidecas,qeensidecas,empassant,queening};
const BACK  =  -104;
const MAXPLY  =  23;
const LOSEVALUE  = 0x7D00;



//
//  �^��`
//
typedef int MAXTYPE;  //  �]���l

typedef int INDEXTYPE;
typedef int SQUARETYPE;
typedef int EDGESQUARETYPE;
typedef short DEPTHTYPE;

enum PIECETYPE {empty, king, queen, rook, bishop, knight, pawn};
enum COLORTYPE {white, black};
typedef struct {int z,t,showx,showy,type;char rank;} xPIECE;
typedef struct {int newz,oldz,spe; xPIECE *p,*cap;}MOVE;
 struct BOARDIDTYPE {
  PIECETYPE piece;
  COLORTYPE color;
};
struct BOARDTYPE {
  PIECETYPE piece;
  COLORTYPE color;
  short index;
  unsigned char attacked;
};

typedef struct MOVETYPE {
  int new1, old;       //  �ړ���ƈړ����̃}�X
  short spe;           //  ����ړ�������
                       //    case movepiece of
                       //      king: �L���X�����O
                       //      pawn: �ʉߕߊl
                       //      else: �|�[�����i
  PIECETYPE movpiece;  // �ړ������ƌ���
  PIECETYPE content;   // �Ƃ��ĕߊl�����
} MOVETYPE;

struct TEXTCOLORTYPE {
  unsigned char background, color;
};

struct SCREENPOSTYPE {
  unsigned char x, y;
  unsigned char background, color;
};
 typedef short SETOFPIECE;
typedef char STR6[6];



typedef struct CLOCKTYPE {
  int hour, minute, second, sec100;
  int totalhours, totmin, totalsecs, tots100;
  int number;
  double totaltime;
} CLOCKTYPE  ;

typedef struct {
  int x;
  STR6 s;
} PICTURETYPE[3];



typedef char MAXSTRING[MAXLEN];

struct PIECETAB {
  SQUARETYPE isquare;
  PIECETYPE ipiece;
};


enum CASTDIRTYPE {zero, lng, shrt};
typedef CASTDIRTYPE CASTTYPE;
typedef unsigned char FIFTYTYPE;
typedef unsigned char REPEATTYPE;

struct NODEVAL {
  unsigned long nodebase, nodeoffset;
};
 typedef struct {int newz,oldz, spe;char PP, QQ;} MMM;
//typedef char STR80[81];

typedef char COMMANDSTRING[COMMANDLENGTH];

enum LEVELTYPE { normal, fullgametime, easygame, infinite, plysearch,
				matesearch, matching };


 typedef unsigned char *LIBTYPE;      //  �I�[�v�j���O���C�u����


struct ATTACKTABTYPE {
	//  �L���O..�|�[���̃Z�b�g�B
	//  ����C���f�b�N�X�ɗ^����ƁA�ǂ��̃}�X�Ɉړ��ł��邩

  SETOFPIECE pieceset;
  int direction;  //  ���i�ރ}�X�̕���
};
#define sz 72
#define basex 72
#define basey 72

class chessrule {
public:
chessrule();
void PlacePiece(int ,char *);
void Play(MOVE);
void Back();
bool CheckAndPlay(int to);
bool genmove(int t);
bool Attack(int z,xPIECE *);
char * showBoard();
bool Stealmate();
bool Checkmate(int);
bool Repeat3();
void pawnsearch(int,int t);
bool linesearch(int dir1,int dir2,int count,int stattz,int tatget);
void generate(int z,xPIECE *p, xPIECE *cap, SPE spe);
void gensearch2(int,int,int,int);
bool getxyrt(int x,int y,int *showx,int *showy, int *col,int *type) ;
bool setCurrentP(int);
void moveCurrent(int x,int y);
bool Translate(char *in,MMM *);
//void empassant();
void castling(xPIECE *);
vector<MOVE> record,movelist;
int step,teban,stat,PN;
char *message;
char display[200];
xPIECE piece[33];

xPIECE*  currentP, *board[0x80];
};
#endif