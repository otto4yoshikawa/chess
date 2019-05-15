//---------------------------------------------------------------------------

#pragma hdrstop
#include <fstream>
#include <iostream>
#include"genmove3.h"
#define dd
//-----------------------------------------
 using namespace std;
#pragma argsused
void 	NextLibNo(short );
void FirstLibNo() ;
void PreviousLibNo() ;
int LibNo,Depth;
unsigned char *Openings;
//--------------------------------------
void showNode(int dep,int bra,int lib,unsigned char c){
	 int cc=c;
	cout<<"Depth="<<dep<<" bro="<<bra<<" libno="<<lib<< " data="<<hex<<cc<<dec;
	cout<<" choice="<<(c&0x3f) ;
	cout <<" start="<< ((c&0x40)?1:0)<<" end="<<(c&0x80)<<endl;
}
//----------------------------------------
int main(int argc, char* argv[])
{
  int g,i,j,k;



  char *head="       +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F  ";
  char w[100];

 const char *libfilename = "opening.mvs";
  ifstream fin(libfilename, ios::in | ios::binary);
  int choice,DepthMax=4;
  if (!fin) {
   cout<<"opeing.mvs）が見つかりません", "ｴﾗｰ";
	Openings = new unsigned char;
    *Openings = 0;
	return 0;
  }
  Openings = new unsigned char[32000];
  fin.read(Openings, 32000);
  fin.close();
  *Openings = 0xFF;
   LibNo=1;
   //------------------
   for(i=0;i<0x400;i++){
   if((i%0x200)==0) cout<<head<<endl;
   if( (i%0x10)==0) {sprintf(w,"%06x",i);}
	sprintf(w+6+3*(i&0x0f)," %02x",Openings[i]);
   if( (i%0x10)==15) cout<<w<<endl;
	}
  //-----------------------------
	for(Depth=0;Depth<DepthMax;Depth++){
	showNode(Depth,1, LibNo,Openings[LibNo]);
	cout<<"Depth="<<Depth<<"enter your choice(1--15) or 0 ";
	 cin>>choice;
	 if(choice==0) break;
	 choice--;
		for(j=0;j<choice;j++){

			NextLibNo(0);
		}
		g=Openings[LibNo];
			k=(g&0x3f)+1;
	 cout<<" Depth="<<Depth<<" chice="<<(choice+1)<<" libNo="<<
	 LibNo<<" g="<< g<<"("<<hex<<g<<dec<<")"<<" joseki="<<k<<endl;
	 LibNo++;
			g=Openings[LibNo];

			if((g&0x40)==0)  {cout<<"not found"<<endl;  return 1;}
	}
		g=Openings[LibNo];
	 cout<<"joseki="<<hex <<0x34<<g<<endl;
	return 0;
}
//---------------------------------------------------------------------------
 //
//  ブロック内の直前の移動を LibNo に設定
//

void PreviousLibNo()
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
void FirstLibNo()
{
  while (!(Openings[LibNo - 1] & 64))
    PreviousLibNo();
}
//
//  ブロック内の次の移動を LibNo に設定。
//  skip がセットされていれば、動けない移動はスキップ
//
void NextLibNo(short skip)
{
  if (Openings[LibNo] >= 0x80)
	FirstLibNo();
  else {
    int n = 0;
    do {
	  if (Openings[LibNo] & 0x40)
		n++;
	  if (Openings[LibNo] >= 0x80)
        n--;
      LibNo++;
    } while (n);
	if (skip && (Openings[LibNo] == 0x3f))
      FirstLibNo();
  }
}

 #ifdef dd
 //1手目からの棋譜 から」LibNoを求める
//------------------------------------------------
//  ライブラリから定石の指し手を見つける  候補手から乱数 で
//
short  MovTab[16];
 int
FindOpeningMove()
{
  int OpCount;
  const unsigned char weight[7] = {7, 10, 12, 13, 14, 15, 16};

  unsigned char r = (unsigned char)random(16);   //  0..16 の乱数から重みを計算
  unsigned char p = 0;
  while (r >= weight[p]) 	p++;
;
  for (unsigned char countp = 1; countp <= p; countp++)  // 一致するノードを探す
	NextLibNo(1);
  OpCount = Openings[LibNo] & 63;  //  移動を生成


	return OpCount;
 }
 //
//  正しいブロックに一致するノードを見つける
//
 int LibDepth;

 // void 	MovGen();
  //--------------------

bool  FindNode()
{
  bool rc=false;
  int i,OpCount;
int  m,movelist[100];
  LibNo++;
  if (Depth > LibDepth) return true;
  OpCount=-1;
  int n=InitMovGen(movelist) ;
  for(i=0;i<n;i++){
   m= movelist[i];
   if(m==MovTab[Depth]) {OpCount=i;break;}
   }
   if (OpCount>=0) {
	while ((Openings[LibNo] & 0x3f) != OpCount && Openings[LibNo] < 0x80)
	  NextLibNo(0);
	if ((Openings[LibNo] & 0x7f) == 0x40 + OpCount) {
	  MakeMove(MovTab[Depth]); //Depth ++ される
	  rc=FindNode();
	  TakeBackMove();
	}
  }

return rc;
}
 #endif
