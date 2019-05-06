#include"chessrule.h"
#include <fstream.h>
#include"board.h"
#include "wcdefs.h"
const UNPLAYMARK = 0x3f;

#include "joseki.h"
 extern int Depth;
 extern  MOVETYPE Next;

 extern  MOVETYPE MovTab[];
 void DoPrintf(char *szFormat, ...);
//
//  �������u���b�N�Ɉ�v����m�[�h��������
//
void joseki::
FindNode()
{
  LibNo++;
  if (Depth > LibDepth) {
	Found = true;
    return;
  }
  OpCount = -1;
  InitMovGen();
  do {
	OpCount++;
	MovGen();
  } while (Next.movpiece != empty && !EqMove(&Next, &MovTab[Depth]));
  if (Next.movpiece != empty) {
    while ((Openings[LibNo] & 63) != OpCount && Openings[LibNo] < 128)
      NextLibNo(0);
    if ((Openings[LibNo] & 127) == 64 + OpCount) {
	  MakeMove(&MovTab[Depth]);
      FindNode();
	  TakeBackMove(&MovTab[Depth-1]);
    }
  }
}

//
//  �u���b�N���̒��O�̈ړ��� LibNo �ɐݒ�
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
//  �u���b�N���̍ŏ��̈ړ��� LibNo �ɐݒ�
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
   // MessageBox(0,"�����ݸ�̧�فiOPENING.LIB�j��������܂���", "�װ", MB_ICONHAND | MB_OK);
    Openings = new unsigned char;
    *Openings = 0;
    return;
  }
  Openings = new unsigned char[32000];
  fin.read(Openings, 32000);
  fin.close();
  *Openings = 0xFF;
   LibNo=1;
}
//
//  �u���b�N���̎��̈ړ��� LibNo �ɐݒ�B
//  skip ���Z�b�g����Ă���΁A�����Ȃ��ړ��̓X�L�b�v
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
  LibNo = 0;
  if (MoveNo < UseLib) {
    LibDepth = Depth;
    while (MovTab[Depth].movpiece != empty)
	  TakeBackMove(&MovTab[Depth]);
	Found = false;
    if (MovTab[Depth].content == king) {
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
}

//
//  ���C�u���������΂̈ړ���������
//
 MOVETYPE  joseki::
FindOpeningMove()
{
DoPrintf("LibNo=%d  G=%02x",LibNo,Openings[LibNo] );
  const unsigned char weight[7] = {7, 10, 12, 13, 14, 15, 16};

  unsigned char r = (unsigned char)random(16);   //  0..16 �̗�������d�݂��v�Z
  unsigned char p = 0;
  while (r >= weight[p]) 	p++;
	DoPrintf("p=%d",p);
  for (unsigned char countp = 1; countp <= p; countp++)  // ��v����m�[�h��T��
	NextLibNo(1);
  OpCount = Openings[LibNo] & 63;  //  �ړ��𐶐�

  InitMovGen();
  for (unsigned char cnt = 0; cnt <= OpCount; cnt++)
    MovGen();
	return Next;
  // MainLine �Ɉړ����i�[
 // MainLine[0] = Next;
 // MainLine[1] = ZeroMove;
 // MainEvalu = 0;
 // MaxDepth = 0;
 // LegalMoves = 0;
 // InitNode(&Nodes);
}