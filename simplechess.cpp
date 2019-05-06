// chess のruleを399行で書こう
#include<vector>
#include<iostream>
using namespace std;
typedef struct {int z,t;char rank;} PIECE;
typedef struct {int newz,oldz,spe;PIECE *p,*cap;}MOVE;
class chessrule {
public:
chessrule();
void PlacePiece(int ,char *);
void Play(MOVE);
void Back();
bool CheckAndPlay(int fromto);
bool genmove(int t);
bool Attack(int z,PIECE *);
char * showBoard();
bool Stealmate();
bool Checkmate(int);
bool Repeat3();
void pawnsearch(int,int t);
bool linesearch(int dir1,int dir2,int count,int stattz,int tatget);
void generate(int z,PIECE *p, PIECE *cap, int spe);
void gensearch2(int,int,int,int);
//void empassant();
void castling(PIECE *);
vector<MOVE> record,movelist;
int step,teban,stat,PN;
char *message;
char display[100];
PIECE piece[40];
PIECE *empty;
PIECE*  board[0x90];
};
bool safespot(PIECE *,PIECE*){ return true;}
bool moved(PIECE *){return true;}
int addz(int z1,int dz){
		return (((z1&0xf0)+(dz&0xf0))&0xf0 )+((z1+dz)&0x0f);
	}
void chessrule::castling(PIECE *p){
	if(moved(p)) return;
	PIECE *rook;
	int t=p->t;
//	int kingz=p->z;
//king side castling 
	int rookz=(t==1)?0x07:0x77;
	rook=board[rookz];
	if(moved(rook)) goto aa;
	if(!safespot(p,rook)) goto aa;
	generate(rookz,p,rook,1);
	
// qeen sisw castling
aa:	
	rookz=(t==1)?0x00:0x70;
	rook=board[rookz];
	if(moved(rook) )return;
	if(!safespot(p,rook)) return;
    generate(rookz,p,rook,1);		
}
	
chessrule::chessrule(){
	step=1;teban=1;PN=0;
	piece[32].rank=',';
	piece[32].t=0;
	for(int z=0;z<0x80;z++) 
	{
	if(z&0x08) continue;
	board[z]=piece+32;}

		PlacePiece(0x04,"K");
		PlacePiece(0x00,"RNBQ BNR");
		PlacePiece(0x10,"PPPPPPPP");
	stat=0;teban=2;
	//showBoard();

}
void chessrule::Play(MOVE m) {
	PIECE *p= m.p;
	PIECE *cap= m.cap;
	int x1,x2,y1,y2;
	if(m.spe==1){
	;board[m.newz]=board[m.oldz];
	board[m.newz]=p;}
	else{
	board[m.newz]=board[m.oldz];
	board[m.oldz]=empty;	
	if(cap) cap->t=0;
	if(m.spe==3)p->rank+=('Q'-'P');
	}
	p->z=m.newz;
	record.push_back(m);
	step++;teban=3-teban;
	}

void  chessrule::Back(){
	MOVE m= record[1];
	record.pop_back();
	PIECE *p= m.p;
	PIECE *cap= m.cap;
	if(m.spe==1){
	board[m.newz]=board[m.oldz];
	board[m.oldz]=p;}

	else{
	board[m.oldz]=p;
	board[m.newz]=empty;	
	if(cap) {cap->t=3-teban;	board[m.newz]=cap;
		if(m.spe==2) board[cap->z]=cap;}
	if(m.spe==3)p->rank-=('Q'-'P');	
	}
	p->z=m.oldz;
	step--;teban=3-teban;
}
bool chessrule::CheckAndPlay(int xyxy){
	int i,x1,x2,y1,y2,z1,z2;
	unsigned int ii;
	PIECE *p1,*p2,*cap;
	x1=xyxy/1000; xyxy=xyxy%1000;
	y1=xyxy/100;xyxy=xyxy%100;
	x2=xyxy/10;y2=xyxy%10;
	x1--;x2--;y1--;y2--;
	z1=y1*16+x1;	z2=y2*16+x2;
	if((z1&0x88)|(z2&0x88)) {
		message="systax error";return false;}
	movelist.clear();
	message="no move";
	p1=board[z1];
	cout<<hex<<z1<<" teban="<<teban<<" "<<p1->rank<<" "<<p1->t<<endl;
	
//	p2=board[y2][x2];
	if(p1->t!=teban) {
		message="不正な駒";return false;
	}
	if(!genmove(teban))return false; 
cout<<" movelist="<<movelist.size()<<endl;
	for(ii=0;ii<movelist.size();ii++) 
	if((movelist[ii].oldz==z1) && (movelist[ii].newz==z2)) break;
	if(ii==movelist.size()) {message="不正な移動";return false;}
	cout<<"OK";
		stat=0;
	Play(movelist[ii]);
	if(Checkmate(teban)) {Back();return false;}

	if(Stealmate()) {stat=2;message="steaal mate draw";}
	else if(Checkmate(teban)) {stat=1;message= "check mate";}
	else if(Repeat3()) {stat=2;message="repeat threedraw";}
	return true;
	

}

char * chessrule::showBoard(){
	char *p=display;
	int x,y;
  	for(y=0;y<8;y++){
		for(x=0;x<8;x++) {*p++=board[x+16*y]->rank;	}
		*p++='\n';
		
	}*p=0;
	return display;
}
bool chessrule::Checkmate(int t){
	int opponent=3-t;
	int targetz=piece[t-1].z;
	int i,start;
	start=(opponent==1)?0:16;
	for(i=start;i<start+16;i++){
		
		if(piece[i].t==0) continue;
		
		if(Attack(targetz,&piece[i])) return true;
		
	}
	return false;
} 
void chessrule::PlacePiece(int startz,char *p){
	int t,z,len=strlen(p);
 	int y=startz&0xf0;
    int dz=0x70-y*2;
 //   cout <<len<< " "<<startz<<" s="<<p<<endl;
	for(int i=0;i<len;i++,startz++){
		
		char c=*p++;
		for(t=1;t<=2;t++){
		z=startz;
		if(t==2) z=z+dz;	
		if(t==2) c|=0x20;
	//	cout<<"t="<<t<<" PN="<<PN<<" c="<<c;
		if(c==' ') continue;
		piece[PN].rank=c;
		piece[PN].z=z;
		piece[PN].t=t;
		board[z]=piece+PN++;
		}
		}
	
}
bool chessrule:: linesearch(int dir1,int dir2,int count,int startz,int target){
	int dirtable[]={0x10,0xf0,0x01,0x0f,0x11,0xf1,0x1f,0xff};
	
	for (int i=dir1;i<dir2;i++){
	int z=startz;
	for(int j=0;j<count;j++){
		z=addz(z,dirtable[i]);
		if(z==target) return true;
		if(z&0x88) return false;
		if(board[z]->t) return false;
	}}
	
	return false;}
bool chessrule::Attack(int targetz, PIECE *p){
	char rank =p->rank |0x20;
	int knightdir[]={0x12};
	int startz= p->z;
	int z,t=p->t;
	int difforwad=(t==1) ?-0x10:0x10;
	switch  (rank) {
	case 'p':
		if((startz+1+difforwad)==targetz) return true;
		if((startz-1+difforwad)==targetz) return true;
	case 'n':
		for(int i=0;i<8;i++){
		z= startz+knightdir[i];
		if(z==targetz) return true;
		} break;	
	case 'k' : return linesearch(0,7,1,startz,targetz);	
	case 'r' : return linesearch(0,3,7,startz,targetz);
	case 'b' : return linesearch(0,3,7,startz,targetz);
	case 'q' : return linesearch(0,3,7,startz,targetz);
	}
	return false;
	};
bool chessrule::Repeat3(){return false;}	
bool chessrule::Stealmate(){return false;}
void chessrule::generate(int z,PIECE *p, PIECE *cap, int spe){
	MOVE m;
	cout<<"generate";
	m.newz=z;m.oldz=p->z;m.p=p;m.cap=cap;m.spe=spe;
//	movelist.push_back(m);
}
bool  chessrule::genmove(int t){
	int x,y,i,ia;
	PIECE *p;
	ia=(t==1)?0:1;
	movelist.clear();
	for (i=ia;i<32;i++,i++){
	
		p=piece+i;
		char rank=p->rank;
		rank |=0x20;
			cout<<"movegen i="<<dec<<i<<" rank="<<p->rank<<" z="<<hex<<p->z<<endl;
		switch (rank){
		case 'p':pawnsearch(p->z,p->t);
		//empassant();
		break;
		case 'r':gensearch2(0,4,7,p->z);break;
		case 'n':gensearch2(8,16,1,p->z);break;
		case 'b':gensearch2(4,8,7,p->z);break;
		case 'q':gensearch2(0,8,7,p->z);break;
		case 'k':gensearch2(0,8,1,p->z);castling(p);break;
		
		}
		
	}	
	return movelist.size()>0;
	
	}

bool isMyColor(int z,int teban,PIECE *p){
	return (p->t==teban);}
void chessrule::gensearch2(int dirstart,int dirend,int count ,int startz)
{ 
	cout<<"start="<<dirstart<<" end="<<dirend<<endl;
		int t,j,i,z;
		PIECE *p,*q;
		
		int drtb[]={0x01,0x0f,0x10,0xf0,0x11,0xff,0xf1,0x1f,
		0x12,0x21,0x2f,0x1e, 0xfe,0xef,0xf2,0xe1};
		for(i=dirstart;i<dirend;i++){
			z=startz;
		count=0;	
		cout<<dec<<"i="<<i<<" z="<<hex<<z<< " dz="<<drtb[i]<<endl;
			for(j=0;j<count;j++){
				z=addz(z,drtb[i]);
				t=board[z]->t;
				if(z&0x88) break;
				cout<<dec<<" i="<<i<<" j="<<j<<" t="<<t<<" z="<<hex<<z<<endl;
				
				if(isMyColor(z,teban,board[z])) break;
				cout<<"$"<<endl;
				if(board[z]==empty) q=0;
				else q=board[z];
				generate(z,p,q,0);
				if(isMyColor(z,3-teban,board[z])) break;
			}
		}	

}
void chessrule::pawnsearch(int oldz,int t){
PIECE *p,*q,*cap;
// １マス前進
	int newz;
	int dz=(t==1)?0x10:0xf0;
	int originaly=(t==1)?0x10:0x60;
	newz=addz(oldz,dz);
	if(newz&0x80) return;
		p=board[oldz];
	if(board[newz]==empty){
			generate(newz,p,0,0);
			if((oldz&0xf0)==originaly){
				newz=newz+dz;
				if(board[newz]==empty){
				generate(newz,p,0,0);}
	}}
// capture	
	for(int i=0;i<2;i++){
		int dx=(i==0)?1:-0x0f;		
		newz=oldz+dz+dx;
		if(newz&0x88) continue;
		q=board[newz];
		if(q->t==(3-teban)) 	generate(newz,p,q,0);
	}
//empassant
	int midy=(t==0)?0x40: 0x30;
	if((oldz&0xf0)!=midy) return;
	MOVE *mm =&record[step-1];
	q=mm->p;
	if(!(p->rank=='p'||q->rank=='P')) return;
	if(abs((q->z/16)-  mm->newz/16)!=2) return;
	if(abs((q->z&0x07)-(oldz&0x0f))!=1) return;
	generate(newz,p,q,2);

}	
int main(){
	int xyxy;
	chessrule mychess;
	while(mychess.stat==0){
		cout <<mychess.showBoard();
		cout<<"your move teban="<<mychess.teban<<" ";
		cin >>xyxy;
		if(!mychess.CheckAndPlay(xyxy)) cout<<mychess.message<<endl;
	}
	cout <<mychess.showBoard()<<endl;
	cout<<mychess.message;
	return 1;
}
