void ResetGame();
void Perform(MOVETYPE *move, bool resetmove);
void InitMovGen();
void MovGen() ;
 void TakeBackMove(MOVETYPE*);
 bool EqMove(MOVETYPE*,MOVETYPE*);
 void  MakeMove ( MOVETYPE *);
 #define MaxLevel 23
 extern bool ComputerThinking;
extern int PVTable[2][7][0x78];
extern PIECETAB PieceTab[][16];
extern BOARDTYPE Board[];
extern int DirTab[];

extern int OfficerNo[2], PawnNo[2];
extern MAXTYPE MainEvalu;
extern int KnightDir[];
extern int PawnDir[];
extern short INFOXSIZE, INFOYSIZE;
extern int OfficerNo[];
extern COLORTYPE Player;
extern COLORTYPE Opponent;
extern COLORTYPE ProgramColor;
extern int MoveNo;
extern LEVELTYPE Level;
extern DEPTHTYPE Depth;
extern  MOVETYPE ZeroMove;
extern int PVTable[2][7][0x78];
extern bool MultiMove, AutoPlay, SingleStep;

extern bool Analysis, Opan;
extern MOVETYPE  MovTab[]; // = &movetemp[-BACK];

