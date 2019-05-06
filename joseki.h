class joseki{
public :
int LibNo;
int LibDepth;
int OpCount;
joseki();
bool Found;
int findmove();
int UseLib;
int MoveNo;
void PreviousLibNo();
void FirstLibNo();
void NextLibNo(short skip)   ;

unsigned char *Openings;
void FindNode();
 MOVETYPE FindOpeningMove()  ;
 void CalcLibNo();
};
