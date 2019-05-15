#include"chessrule.h"
#include <fstream.h>
#include"board.h"
//#include "wcdefs.h"
const UNPLAYMARK = 0x3f;

#include "joseki.h"
extern int UseLib;
 extern DEPTHTYPE Depth;
 extern  MOVETYPE Next;
 extern int MoveNo;

 void DoPrintf(char *szFormat, ...);
//
//  正しいブロックに一致するノードを見つける
//
void joseki::
FindNode()
{
DoPrintf("enter findNode Depth=%d JibDeprg=%d LibNo=%d MoveNo=%d",
Depth,LibDepth,LibNo,MoveNo);
  LibNo++;
  if (Depth > LibDepth) {
  DoPrintf("found");
	Found = true;
	return;
  }
  OpCount = -1;
  InitMovGen();
  do {
	OpCount++;
	MovGen();
	DoPrintf("opcount=%d",OpCount);
  } while (Next.movpiece != empty && !EqMove(&Next, &MovTab[Depth]));
  if (Next.movpiece != empty) {
	 DoPrintf("JibNo=%d",LibNo);
	while ((Openings[LibNo] & 0x3f) != OpCount && Openings[LibNo] < 0x80)
	  NextLibNo(0);
	  DoPrintf("LibNo=%d Openings=%02x", LibNo,Openings[LibNo]);
	if ((Openings[LibNo] & 0x7f) == 0x40 + OpCount) {

	  MakeMove(&MovTab[Depth]);
      FindNode();
	  TakeBackMove(&MovTab[Depth-1]);
    }
  }
}

//
//  ブロック内の直前の移動を LibNo に設定
//
void joseki::PreviousLibNo()
{
  int n = 0;
  do {
   LibNo--;
   if (Openings[LibNo] >= 128)
     n++;
   if (Openings[LibNo] & 64)
     n--;
  } while (n);
}
//
//  ブロック内の最初の移動を LibNo に設定
//
void joseki::FirstLibNo()
{
  while (!(Openings[LibNo - 1] & 64))
    PreviousLibNo();
}



joseki::joseki(){

 const char *libfilename = "opening.mvs";
  ifstream fin(libfilename, ios::in | ios::binary);

  if (!fin) {
   // MessageBox(0,"ｵｰﾌﾟﾆﾝｸﾞﾌｧｲﾙ（OPENING.LIB）が見つかりません", "ｴﾗｰ", MB_ICONHAND | MB_OK);
    Openings = new unsigned char;
    *Openings = 0;
    return;
  }
  Openings = new unsigned char[32000];
  fin.read(Openings, 32000);
  fin.close();
  *Openings = 0xFF;
   LibNo=1;
  CalcLibNo();

}
//
//  ブロック内の次の移動を LibNo に設定。
//  skip がセットされていれば、動けない移動はスキップ
//
void joseki::NextLibNo(short skip)
{
  if (Openings[LibNo] >= 128)
    FirstLibNo();
  else {
    int n = 0;
    do {
	  if (Openings[LibNo] & 64)
		n++;
      if (Openings[LibNo] >= 128)
        n--;
      LibNo++;
    } while (n);
    if (skip && (Openings[LibNo] == UNPLAYMARK))
      FirstLibNo();
  }
}

void  joseki::
CalcLibNo()
{
DoPrintf("calclib Depth=%d MN=%d",Depth,MoveNo);
  LibNo = 0;
  if (MoveNo < UseLib) {
    LibDepth = Depth;
	while (Depth>0) {
	DoPrintf("takeback $1 depth=%d from=%02x",Depth,MovTab[Depth].old);
	  TakeBackMove(&MovTab[Depth]);  }
	Found = false;
    if (1) {
      Depth++;
      FindNode();
	  Depth--;
    }
    while(Depth < LibDepth)
      MakeMove(&MovTab[Depth + 1]);
    if (Found)
	  UseLib = 200;
    else {
      UseLib = MoveNo;
      LibNo = 0;
	}
  }
 DoPrintf("clac libno foun=%d libno=%d",Found,LibNo);
}

//
//  ライブラリから定石の移動を見つける
//
 MOVETYPE  joseki::
FindOpeningMove()
{
DoPrintf("LibNo=%d  G=%02x",LibNo,Openings[LibNo] );
  const unsigned char weight[7] = {7, 10, 12, 13, 14, 15, 16};

  unsigned char r = (unsigned char)random(16);   //  0..16 の乱数から重みを計算
  unsigned char p = 0;
  while (r >= weight[p]) 	p++;
	DoPrintf("p=%d",p);
  for (unsigned char countp = 1; countp <= p; countp++)  // 一致するノードを探す
	NextLibNo(1);
  OpCount = Openings[LibNo] & 63;  //  移動を生成

  InitMovGen();
  for (unsigned char cnt = 0; cnt <= OpCount; cnt++)
    MovGen();
	return Next;
  // MainLine に移動を格納
 // MainLine[0] = Next;
 // MainLine[1] = ZeroMove;
 // MainEvalu = 0;
 // MaxDepth = 0;
 // LegalMoves = 0;
 // InitNode(&Nodes);
}