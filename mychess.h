


#define	ENQ(org, q)	(q)->next=org,	\
			(q)->prev=(org)->prev,	\
			(org)->prev->next=q,	\
			(org)->prev=q 
#define	DEQ(p)		(p)->prev->next=(p)->next,	\
			(p)->next->prev=(p)->prev
#define	INITQ(p)	(p)->next=(p)->prev=p


typedef struct PIECE
{struct PIECE* next,*prev;int x,y,showx,showy,color,type;} PIECE;
typedef struct MOVEPACK { struct MOVEPACK *prev,*next;int oldx,oldy,
type,color,newx,newy,esp; PIECE *p,*cappiece;} MOVEPACK;
typedef char DIRTYPE;
typedef short SETOFPIECE;



/*/  �L���O..�|�[���̃Z�b�g�B
	//  ����C���f�b�N�X�ɗ^����ƁA�ǂ��̃}�X�Ɉړ��ł��邩

  SETOFPIECE pieceset;
  int direction;  //  ���i�ރ}�X�̕���
};
 */

class mychess {

public:
mychess();
int teban,step;
 PIECE piece[32];
 PIECE * boardmap[8][8] ;
 PIECE *currentP;
 MOVEPACK moveTable[100];
void initializeBoard();
void Play(MOVEPACK*);
void Back();
void PrintBoard( );
void setmove(int,int);
bool setCurrent(int n) ;
bool mouseup(int n);
void clearBoard();
 void addPiece(int x,int y,char c) ;
 
};