


const COMMANDLENGTH  = 10;
const MAXLEN         = 64;
const SQUARE_SIZE = 36;
const MAXBDSIZE = 8;
const MYFRAMESIZE = 3;
enum SPE {normal,kingsidecas,queensidecas,
empassant,queening,take,check};
const BACK  =  -104;
const MAXPLY  =  23;
const LOSEVALUE  = 0x7D00;

 enum PIECETYPE {empty, king, queen, rook, bishop, knight, pawn};
enum COLORTYPE {white, black};

//
//  �^��`
//
typedef int MAXTYPE;  //  �]���l
typedef int DIRTYPE;
typedef int INDEXTYPE;
typedef int SQUARETYPE;
typedef int EDGESQUARETYPE;
typedef short 	DEPTHTYPE;
typedef struct {int z,t,showx,showy,type;char rank;} xPIECE;
typedef struct {int newz,oldz; SPE spe; xPIECE *p,*cap;}MOVE;
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

enum LEVELTYPE { normalz, fullgametime, easygame, infinite, plysearch,
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


  void MakeMove( short);
  void TakeBackMove();
  int  InitMovGen(int *);
  void initBoardandPiece();