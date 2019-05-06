//----------------------------------------------------------------------------
// ObjectWindows - (C) Copyright 1991, 1993 by Borland International
//----------------------------------------------------------------------------
#include <vcl.h>
#include "externs.h"
void DoPrintf(char *szFormat,...);

//
//  �O���[�o���ϐ�
//
static ATTACKTABTYPE attack[240];
ATTACKTABTYPE *AttackTab = &attack[120];
SETOFPIECE BitTab[7] = {0, 1, 2, 4, 8, 0x10, 0x20};
int DirTab[8] = { 1, -1, 0x10, -0x10, 0x11, -0x11, 0x0f, -0x0f};
int KnightDir[8] = {0x0E, -0x0E, 0x12, -0x12, 0x1f, -0x1f, 0x21, -0x21};
int PawnDir[2] = {0x10, -0x10};
MOVETYPE Next;
int BufCount, BufPnt;
MOVETYPE Buffer[81];
CASTMOVETYPE  CastMove[2][2] = { {{2, 4}, {6, 4}}, {{0x72, 0x74}, {0x76, 0x74}} };
 /*
bool convertBoard(PIECE *p) {
     int i,x,y,type,color;
      ClearBoard();
     for (i=0;i<32;i++) {
         if (p[i].type<0) continue;
         color=p[i].color;
         x= p[i].x;
         y=p[i].y;
        board[x+y*8].color=(color==1)?white:black;
        board[x+y*8].piece=type;
     }

}
 */
void
CalcAttackTab()
{
  for (int sq = -0x77; sq <= 0x77; sq++) {
    AttackTab[sq].pieceset = 0;
    AttackTab[sq].direction = 0;
  }

  for (DIRTYPE dir = 7; dir >= 0; dir--) {
    for (unsigned char i = 1; i < 8; i++) {
      if (dir < 4)
        AttackTab[DirTab[dir]*i].pieceset = SETOFPIECE(BitTab[queen]+BitTab[rook]);
      else
        AttackTab[DirTab[dir]*i].pieceset = SETOFPIECE(BitTab[queen]+BitTab[bishop]);
      AttackTab[DirTab[dir]*i].direction = DirTab[dir];
    }
    AttackTab[DirTab[dir]].pieceset += SETOFPIECE(BitTab[king]);
    AttackTab[KnightDir[dir]].pieceset = SETOFPIECE(BitTab[knight]);
    AttackTab[KnightDir[dir]].direction = KnightDir[dir];
  }
}

//
//  asquare �̃}�X��� apiece �̋�Asquare �̃}�X���U���ł��邩�ǂ����v�Z
//
short
PieceAttacks(PIECETYPE apiece, COLORTYPE acolor, SQUARETYPE asquare, SQUARETYPE square)
{
  int x = square - asquare;
  if (apiece == pawn)  //  �|�[���̍U��
    return abs(x - PawnDir[acolor]) == 1;

  //  ���̍U��: ��͂��̃}�X�Ɉړ��\��?
  if (AttackTab[x].pieceset & BitTab[apiece]) {
    if (apiece == king || apiece == knight)
      return 1;

    //  �ԂɃu���b�N���Ă������邩?
    EDGESQUARETYPE sq = asquare;
    do {
      sq += AttackTab[x].direction;
    } while (sq != square && Board[sq].piece == empty);
    return sq == square;
  }
  return 0;
}

//
//  acolor ���� square �̃}�X�̃|�[���ōU���ł��邩�v�Z
//
short
PawnAttacks(COLORTYPE acolor, SQUARETYPE square)
{
  EDGESQUARETYPE sq = square - PawnDir[acolor] - 1;  //  ���̃}�X
  if (!(sq & 0x88))
    if (Board[sq].piece == pawn && Board[sq].color == acolor)
      return 1;

  sq += 2;  //  �E�̃}�X
  if (!(sq & 0x88))
    if (Board[sq].piece == pawn && Board[sq].color == acolor)
      return 1;

  return 0;
}

//
//  acolor ���� square �̃}�X���U���ł��邩�ǂ����v�Z
//
short
Attacks(COLORTYPE acolor, SQUARETYPE square)
{
  if (PawnAttacks(acolor, square))  //  �|�[���̍U��
    return 1;

  //  ���̍U��:  ��������n�߂Ă��ׂĂ̋������
  for (INDEXTYPE i = OfficerNo[acolor]; i >= 0; i--)
    if (PieceTab[acolor][i].ipiece != empty)
      if (PieceAttacks(PieceTab[acolor][i].ipiece, acolor,
          PieceTab[acolor][i].isquare, square))
        return 1;
  return 0;
}

//
//  square �̃}�X��� inpiece �̋�����āA��x�����������Ƃ��Ȃ����ǂ�������
//
short
Check(SQUARETYPE square, PIECETYPE inpiece, COLORTYPE incolor)
{
  if(Board[square].piece == inpiece && Board[square].color == incolor) {
    DEPTHTYPE dep = DEPTHTYPE(Depth - 1);
    while (MovTab[dep].movpiece != empty) {
      if (MovTab[dep].new1 == square)
        return 0;
      dep--;
    }
    return 1;
  }
  return 0;
}

//
//  incolor �̐F���L���X�����O�ł��邩�ǂ�������
//
void
CalcCastling(COLORTYPE incolor,  CASTDIRTYPE *cast)
{
  SQUARETYPE square = 0;
  int k;
  if (incolor == black)
    square = 0x70;
  k = zero;
  if (Check(square + 4, king, incolor)) { //  �L���O������
    if (Check(square, rook, incolor))
      k += lng;                //  A ��̃��[�N������
    if (Check(square + 7, rook, incolor))
      k+= shrt;               //  H ��̃��[�N������
  }
  *cast=  (CASTDIRTYPE)k;
}

//
//  �ړ����A�|�[���ړ��܂��͕ߊl�ړ�������
//
inline short
RepeatMove(MOVETYPE* move)
{
  return move->movpiece != empty && move->movpiece != pawn &&
         move->content == empty && !move->spe;
}

//----------------------------------------------------------------------------

//
//  �Ō�ɕߊl���邩�A�|�[�����ړ����Ă���̈ړ����𐔂���
//  fiftymovecnt = 100 �Ȃ�A�Q�[���͈�������
//
FIFTYTYPE
FiftyMoveCnt()
{
  FIFTYTYPE cnt = 0;
  while (RepeatMove(&MovTab[Depth - cnt]))
    cnt++;
  return cnt;
}

//
//  �ȑO�ɂ��̈ʒu�����񔭐����������v�Z�B
//  Repetition ���R��Ԃ����Ƃ��A�Q�[���͈��������B
//  MovTab[back..Depth] �ɍŌ�Ɏ��s�����ړ����i�[����Ă���
//  immediate ���Z�b�g����Ă���΁A���O�̌J��Ԃ��̂݌���
//
REPEATTYPE
Repetition(short immediate)
{
  DEPTHTYPE lastdep, compdep, tracedep, checkdep, samedepth;
  SQUARETYPE tracesq, checksq;
  REPEATTYPE repeatcount;

  repeatcount = 1;
  lastdep = samedepth = DEPTHTYPE(Depth + 1);  //  ���݂̈ʒu
  compdep = DEPTHTYPE(samedepth - 4);      //  ��r����ŏ��̈ʒu

  //  MovTab[lastdep..Depth] �ɂ͈ȑO�̊֘A�����ړ����i�[����Ă���
  while (RepeatMove(&MovTab[lastdep - 1]) && (compdep < lastdep || !immediate))
    lastdep--;
  if (compdep < lastdep)
    return repeatcount;
  checkdep = samedepth;
  for (;;) {
    checkdep--;
    checksq = MovTab[checkdep].new1;
    for (tracedep = DEPTHTYPE(checkdep + 2); tracedep < samedepth; tracedep += DEPTHTYPE(2))
      if (MovTab[tracedep].old == checksq)
        goto TEN;

    //  �ȑO�̈ړ������������ƂɂȂ邩�A�ړ��������̂ڂ��Č���
    tracedep = checkdep;
    tracesq = MovTab[tracedep].old;
    do {
      if (tracedep-2 < lastdep)
        return repeatcount;
      tracedep -= (DEPTHTYPE)2;
      //  �ȑO�Ɉړ����������
      if (tracesq == MovTab[tracedep].new1)
        tracesq = MovTab[tracedep].old;
    } while (tracesq != checksq || tracedep > compdep + 1);
    if (tracedep < compdep) {  //  compdep �𒲐�
      compdep = tracedep;
      if ((samedepth - compdep) % 2 == 1) {
        if (compdep == lastdep) return repeatcount;
        compdep --;
      }
      checkdep = samedepth;
    }
    //  samedep �� compdep �Ԃ̂��ׂĂ̈ړ��͌�������A
    //  �J��Ԃ������o���ꂽ
TEN:
    if (checkdep <= compdep) {
      repeatcount++;
      if (compdep - 2 < lastdep) return repeatcount;
      checkdep = samedepth = compdep;
      compdep -= (DEPTHTYPE)2;
    }
  }
}

//
//  �ړ��\������
//
//  ����:
//   move �́A�Ⴄ�ʒu�Ő������ꂽ�������ړ��̊��S�ȋL�q���i�[����Ă���
//   MovTab[Depth-1] �ɍŌ�Ɏ��s�����ړ����i�[����Ă���
//
//  �o��:
//   KillMovGen �́A�ړ��\���ǂ�����Ԃ�
//
short
KillMovGen(MOVETYPE* move)
{
  SQUARETYPE castsq;
  PIECETYPE promote;
  CASTDIRTYPE castdir;
  CASTTYPE cast;
  short killmov;

  killmov = 0;
  if (move->spe && move->movpiece == king) {
    CalcCastling(Player, &cast);   //  �L���X�����O
    if (move->new1 > move->old)
      castdir = shrt;
    else
      castdir = lng;

    if (cast & castdir) {  //  �L���O�ƃ��[�N�͈ȑO�Ɉړ��������Ƃ����邩
      castsq = (int)((move->new1 + move->old) / 2);
      //  �}�X�͋�?
      if  (Board[move->new1].piece == empty)
        if (Board[castsq].piece == empty)
          if (move->new1 > move->old || Board[move->new1-1].piece == empty)
            //  �}�X�͍U������Ă��Ȃ���
            if (!Attacks(Opponent, move->old))
              if (!Attacks(Opponent, move->new1))
                if (!Attacks(Opponent, castsq))
                  killmov = 1;
    }

  } else {
    if (move->spe && move->movpiece == pawn) {
      //  �ߊl�ړ�
      //  ����́A�Q�}�X�ړ������̂�?
      if (MovTab[Depth-1].movpiece == pawn)
        if (abs(MovTab[Depth-1].new1 - MovTab[Depth-1].old) >= 0x20)
          if (Board[move->old].piece == pawn && Board[move->old].color == Player)
            killmov = move->new1 == (MovTab[Depth-1].new1+MovTab[Depth-1].old) / 2;
    } else {
      if (move->spe) {            // �ʏ�̃e�X�g
        promote = move->movpiece; // �|�[�����i
        move->movpiece = pawn;
      }

      //  Old �� New1 �̃}�X�̓��e�͐�������?
      if (Board[move->old].piece == move->movpiece)
        if (Board[move->old].color == Player)
          if (Board[move->new1].piece == move->content)
            if (move->content == empty || Board[move->new1].color == Opponent) {
              if (move->movpiece == pawn) {  //  �ړ��\��?
                if (abs(move->new1 - move->old) < 0x20)
                  killmov = 1;
                else
                  killmov = Board[(move->new1+move->old) / 2].piece == empty;
              } else
                killmov = PieceAttacks(move->movpiece, Player,
              move->old, move->new1);
            }
      if (move->spe)
        move->movpiece = promote;
    }
  }
  return killmov;
}

//
//  �ړ����o�b�t�@�Ɋi�[
//
static void
Generate()
{
  BufCount++;
  Buffer[BufCount] = Next;
}

//
//  �|�[���̏��i�𐶐�
//
static void
PawnPromotionGen()
{
  Next.spe = 1;
  int promote;
  for ( promote = queen; promote <= knight; promote++) {
    Next.movpiece = promote;
    Generate();
  }
  Next.spe = 0;
}

//
//  PieceTab ���g���� new1 �̋�̕ߊl�ړ��𐶐�
//
static void
CapMovGen()
{
  Next.spe = 0;
  Next.content = Board[Next.new1].piece;
  Next.movpiece = pawn;
  EDGESQUARETYPE nextsq = Next.new1 - PawnDir[Player];
  for (EDGESQUARETYPE sq = nextsq-1; sq <= nextsq+1; sq++)
    if (sq != nextsq)
      if ((sq & 0x88) == 0)
        if (Board[sq].piece == pawn && Board[sq].color == Player) {
          Next.old = sq;
          if (Next.new1 < 8 || Next.new1 >= 0x70)
            PawnPromotionGen();
          else
            Generate();
        }

  //  ���̑��̕ߊl�A���������J�n
  for (INDEXTYPE i = OfficerNo[Player]; i >= 0; i--)
    if (PieceTab[Player][i].ipiece != empty && PieceTab[Player][i].ipiece != pawn)
      if (PieceAttacks(PieceTab[Player][i].ipiece, Player,
          PieceTab[Player][i].isquare, Next.new1)) {
        Next.old = PieceTab[Player][i].isquare;
        Next.movpiece = PieceTab[Player][i].ipiece;
        Generate();
      }
}

//
//  old �̋�̖��ߊl�ړ��𐶐�
//
static void
NonCapMovGen()
{
  DIRTYPE  first, last, dir;
  int direction;
  EDGESQUARETYPE  newsq;

  Next.spe = 0;
  Next.movpiece = Board[Next.old].piece;
  Next.content = empty;
  switch (Next.movpiece) {
    case king:
      for (dir = 7; dir >= 0; dir--) {
        newsq = Next.old + DirTab[dir];
        if (!(newsq & 0x88))
        if (Board[newsq].piece == empty) {
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
        //  ���̕����Ŗ��ߊl�ړ��𐶐�
        while (!(newsq & 0x88)) {
          if (Board[newsq].piece != empty) goto TEN;
          Next.new1 = newsq;
          Generate();
          newsq = Next.new1 + direction;
        }
TEN:    continue;
      }
      break;

    case pawn:
      Next.new1 = Next.old + PawnDir[Player];  //  �P�}�X�O�i
      if (Board[Next.new1].piece == empty) {
        if (Next.new1 < 8 || Next.new1 >= 0x70)
          PawnPromotionGen();
        else {
          Generate();
          if (Next.old < 0x18 || Next.old >= 0x60) {
            Next.new1 += (Next.new1 - Next.old); // �Q�}�X�O�i
            if (Board[Next.new1].piece == empty)
              Generate();
          }
        }
      }
  }
}

//
//  �ړ�����
//  InitMovGen �́A���ׂẲ\�Ȉړ��𐶐����āABuffer �Ɋi�[����B
//  MovGen �́A�P�P�ړ��𐶐����Ă����� Next �Ɋi�[����B
//
//  ����:
//    Player �ɂ͈ړ�����F���i�[����Ă���
//    MovTab[Depth-1] �ɍŌ�Ɏ��s�����ړ����i�[����Ă���
//
//  �o��:
//    Buffer �ɐ������ꂽ�ړ����i�[����
//
//    �ړ��͎��̏����Ő��������:
//      �ߊl�ړ�
//      �L���X�����O�ړ�
//      ���ߊl�ړ�
//      �ʉߕߊl
//

void
InitMovGen()
{
  int castdir;
  EDGESQUARETYPE sq;
  int index;

  BufCount = BufPnt = 0;
  //  �傫����̕ߊl����J�n���Ă��ׂĂ̕ߊl�𐶐�
  for (index = 1; index <= PawnNo[Opponent]; index++)
    if (PieceTab[Opponent][index].ipiece != empty) {
      Next.new1 = PieceTab[Opponent][index].isquare;
      CapMovGen();
    }

  Next.spe = 1;
  Next.movpiece = king;
  Next.content = empty;
  for (castdir = lng-1; castdir <= shrt-1; castdir++) {
    Next.new1 = CastMove[Player][castdir].castnew;
    Next.old = CastMove[Player][castdir].castold;
    if (KillMovGen(&Next)) Generate();
  }

  //  ���ߊl�ړ��𐶐��A�|�[������J�n
  for (index = PawnNo[Player]; index >= 0; index--)
    if (PieceTab[Player][index].ipiece != empty) {
      Next.old = PieceTab[Player][index].isquare;
      NonCapMovGen();
    }
  if (MovTab[Depth-1].movpiece == pawn)  //  �ʉߕߊl
    if (abs(MovTab[Depth-1].new1 - MovTab[Depth-1].old) >= 0x20) {
      Next.spe = 1;
      Next.movpiece = pawn;
      Next.content = empty;
      Next.new1 = (MovTab[Depth-1].new1 + MovTab[Depth-1].old) / 2;
      for (sq = MovTab[Depth-1].new1-1; sq <= MovTab[Depth-1].new1+1;
              sq++)
        if (sq != MovTab[Depth-1].new1)
          if (!(sq & 0x88)) {
            Next.old = sq;
            if (KillMovGen(&Next)) Generate();
          }
    }
}

//
//  �o�b�t�@����擾�������̈ړ��� Next �ɒu���B�ړ���������΁A
//  ZeroMove �𐶐��B
//
void MovGen()
{
  if (BufPnt >= BufCount)
    Next = ZeroMove;
  else {
    BufPnt++;
    Next = Buffer[BufPnt];
  }
}