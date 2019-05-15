// ---------------------------------------------------------------------------
//V1  (ox00 は左上
//V2 y =7-y
//V3 emptyboard
#include"chessrule.h"
#include<assert.h>
void DoPrintf(char*szFormat, ...);
int drtb[] = {
	1, -1, 0x10, -0x10, 0x11, -0x11, 0x0f, -0x0f,
	0x0E, -0x0E, 0x12, -0x12, 0x1f, -0x1f, 0x21, -0x21};

extern vector<MOVE> record2;

bool chessrule:: safespot(xPIECE *p1, xPIECE*p2) {
//２点とその 間に相手の効きがあるか
	int z1, z, z2 ,dz,n;
	int opponent = 3 - teban;
	int i, start;
	z1=p1->z;z2=p2->z  ;
	n=abs(z1-z2);
	dz= (z2-z1)/n;

	for(z=z1;n>=0;z+=dz,n--){     //target
	if (!(z==p1->z ||z==p2->z) )
	if(board[z]!=emptyboard) return false;
	start = (opponent == 1) ? 0 : 1;
	for (i = start; i < 32; i++, i++) {
		if (piece[i].t == 0)		continue;
		if (Attack(z, &piece[i]))  return false;;

	}
	}//target
	return true;


}

bool moved(xPIECE *p, vector<MOVE> *record) {
	vector<MOVE>::iterator itr;
	for (itr = record->begin(); itr != record->end(); itr++) {
		MOVE m = *itr;
		if (m.p == p)
			return true;
	}
	return false;
}



// -----------------------------------------------------------
void chessrule::castling(xPIECE *p) {
	if (moved(p, &record)) 		return;   //動かしたことあり
	xPIECE *prook;
	int t = p->t;
	// king side castling
	int rookz ;
	int newz;
	rookz = (t ==2) ? 0x07 : 0x77;
	prook = board[rookz];
	newz= (t ==2) ? 0x06 : 0x76;
	if (moved(prook, &record)) 	goto aa;
	if (!safespot(p, prook))		goto aa;
	generate(newz, p, prook, kingsidecas);

	// queen side  castling
aa:
	rookz = (t ==2) ? 0x00 : 0x70;
	newz= (t ==2) ? 0x02 : 0x72;
	prook = board[rookz];
	if (moved(prook, &record))		return;
	if (!safespot(p, prook)) 		return;
	generate(newz, p, prook, queensidecas);
}

// --------------------------
// constructor
// --------------------------
chessrule::chessrule() {
	step = 1;
	teban = 1;
	PN = 0;

	for (int z = 0; z < 0x80; z++) {

			board[z] = emptyboard;
	}
   emptyboard->t=0;
   emptyboard->z=0;

   emptyboard->type=0;
   if(*VERSION=='1'){
	PlacePiece(0x04, "K");
	PlacePiece(0x00, "RNBQ BNR");
	PlacePiece(0x10, "PPPPPPPP");
	}
	else{
	PlacePiece(0x74, "K");
	PlacePiece(0x70, "RNBQ BNR");
	PlacePiece(0x60, "PPPPPPPP") ;

	}
	stat = 0;
	teban = 2;
	// showBoard();
}
//-----------------------------------
// Play                      m.cap can be omitted
// ----------------------------------
void chessrule::Play(MOVE m) {
	xPIECE *temp, *p = m.p;     //from
	xPIECE *cap = m.cap;
	 int newz=m.newz;
	int y2,y,z3;
	int capa='$' ;
	int oldz,rooknewz,oldkingz,rookoldz,rookxnew;
	rookxnew=-1;
	if (m.spe == kingsidecas) {
	rookxnew=5;   rookoldz=7;
	 }
	if( m.spe==queensidecas) {
		 rookxnew=3;  	rookoldz=0;
	}
	if( m.spe==empassant) {
		int epsquare = (m.newz & 7) + (m.oldz & 0x70); // 通過捕獲
		temp=board[epsquare];temp->t=0;
		board[epsquare] = emptyboard;
	}
	if(rookxnew>0){  // castling
			 oldz= 4 + newz&0x70;

  //	DoPrintf("m.new=%03x m.old=%02x p->z=%02x",m.newz,m.oldz,m.p->z);
			p=m.p ;
			rookoldz=    m.cap->z;    // +=newz&0x70;

			rooknewz = (rookoldz&0x70)+ rookxnew;


		board[m.newz] = p;
		p->z = m.newz;
		board[m.oldz]=emptyboard;

		temp=m.cap;
	//	DoPrintf("cast rookz=%02x rooknew=%02x rook=%c",
	//	rookoldz,rooknewz,temp->rank);

		board[temp->z] = emptyboard;   //old rook
		temp->z=	rooknewz;
		board[rooknewz]=temp;         //


		//rook display
	   temp->showx = basex + (sz * (temp->z & 7));
	   y=(temp->z)/16;
	   y=7-y;

		temp->showy = basey + y  * sz;
	}

	else {
	DoPrintf("from %02x ==> %02x ",m.oldz,m.newz);
	temp=	board[m.newz];
		board[m.oldz] = emptyboard;
		board[m.newz] = p;
		p->z = m.newz;
	 if(temp->t>0){

	  DoPrintf("%02x",m.newz);
		 capa= temp->rank;
		 temp->t=0;

		 }

		if (m.spe ==queening) p->rank += ('Q' - 'P');
	}
		;

		//	assert(capa!='u');

		DoPrintf("from %02x ==> %02x P=%c cap=%c "
			,m.oldz,m.newz,p->rank,capa);


	// p->z=m.newz;
	p->showx = basex + (sz * (p->z & 7));
    	y2=(p->z) / 16;
		y2=7-y2;
	p->showy = basey + y2 * sz;
	record.push_back(m);
  //	record2.push_back(m);

	assert( record.size()==step);
	step++;
	teban = 3 - teban;
}
//--------------------------
//    Back
//-------------------------
void chessrule::Back() {

	int rookz,rooknew,kingy,rookxold,rookxnew,kingnewx,oldking;
	int y2,newking;
	rookxnew=-1;
	if(step==78)
	step=step;
	step--;
	teban = 3 - teban;
	MOVE m = record[step-1];
	if (m.spe == kingsidecas) {
		rookxold=7,rookxnew=5;kingnewx=6;
	 }
	if( m.spe==queensidecas) {
		 rookxold=0,rookxnew=3;kingnewx=2;
	}
	record.pop_back();
	xPIECE *temp,*p = m.p;
	xPIECE *cap = m.cap;

	DoPrintf("Back step=%d%02x %02x",step,m.newz,m.oldz);

	if(rookxnew>=0){
	  //castling

		oldking =(teban==2)?0x04:0x74;  //depend on  vwesion
		kingy=oldking&0x70;
		newking= p->z;
		rookz=kingy+rookxnew;
		temp= board[rookz];
		rooknew=kingy+rookxnew;
		p->z= oldking;
		temp->z=kingy+rookxold;
		board[newking] = emptyboard;
		board[rookz] = emptyboard;
		board[oldking]=p;
		rooknew= rookxold+kingy;
		board[rooknew]= temp;
	 	DoPrintf("back cas %02x %02x %02x",rookz,rooknew,newking);
		temp->showx = basex + (sz * (temp->z & 7));
		y2=(temp->z) / 16;
		if(V2) y2=7-y2;
		temp->showy = basey +y2  * sz;

	}

	else {
	   //	 temp= 	board[m.oldz]:

		board[m.oldz] = p;
		board[m.newz] = emptyboard;
		if (cap) {
			cap->t = 3 - teban;
			board[m.newz] = cap;
			if (m.spe == empassant)
				board[cap->z] = cap;
		}
		if (m.spe == queening)	p->rank -= ('Q' - 'P');
	}
	p->z = m.oldz;
	p->showx = basex + (sz * (p->z & 7));
		y2=(p->z) / 16;
		y2=7-y2;
	p->showy = basey + y2 * sz;

}

bool chessrule::CheckAndPlay(int to) {
	int i, x1, x2, y1, y2, z1, z2;
	unsigned int ii;
	xPIECE *p1, *p2;

	z1 = currentP->z;
	z2 = to;
	if ((z1 & 0x88) | (z2 & 0x88)) {
		message = "systax error";
		return false;
	}
	movelist.clear();
	message = "no move";
	p1 = board[z1]; y1=z1/16;
	if(*VERSION>'1') y1=7-y1;
	DoPrintf("rank=%c z=%02x", p1->rank, p1->z);
	 currentP->showx=  basex + sz * (z1 % 8);

	 currentP->showy=  basey + sz * y1;    //$$$$$$
	// p2=board[y2][x2];
	if (p1->t != teban) {
		message = "不正な駒";
		return false;
	}
	if (!genmove(teban)) 	return false;

	DoPrintf("movelist= %d", movelist.size());
	for (ii = 0; ii < movelist.size(); ii++) {
  //	DoPrintf("%c %02x %02x==>%02x %02x",movelist[ii].p->rank,
  //	movelist[ii].oldz ,z1, movelist[ii].newz ,z2);
		if ((movelist[ii].oldz == z1) && (movelist[ii].newz == z2))
			break;
		}
	if (ii == movelist.size()) {
		message = "不正な移動";
		return false;
	}
	stat = 0;
	Play(movelist[ii]);
	if (Checkmate(3 - teban)) {
		Back();
		return false;
	}

	if (Stealmate()) {
		stat = 2;
		message = "steaal mate draw";
	}
	else if (Checkmate(teban)) {
		stat = 1;
		message = "check mate";
	}
	else if (Repeat3()) {
		stat = 2;
		message = "repeat threedraw";
	}

	return true;

}

char * chessrule::showBoard() {
	char *p = display;
	int x, y,y2;
	for (y = 0; y < 8; y++) {
		*p++ = ' ';
		*p++ = ' ';
		y2=y;
		if(V2) y2=7-y2;
		for (x = 0; x < 8; x++) {
		if( board[x + 16 * y2]==0) *p++='#';
		 else *p++ = board[x + 16 * y2]->rank;
		}
		*p++ = '\n';

	}
	*p = 0;
	return display;
}

bool chessrule::Checkmate(int t) {
	int opponent = 3 - t;
	int targetz = piece[t - 1].z;
	int i, start;
	start = (opponent == 1) ? 0 : 32;
	for (i = start; i < 32; i++, i++) {

		if (piece[i].t == 0)		continue;
	//	DoPrintf("target =%02x i=%d %d %02x",
	//	targetz, piece[i].type, piece[i].z);
		if (Attack(targetz, &piece[i])) 		return true;

	}
	return false;
}

void chessrule::PlacePiece(int startz, char *p) {
	int t, z, len = strlen(p);
	int y = startz & 0xf0;
	int k, dz;

	dz = 0x70 - y * 2;
	if(V2)
	if(y==0x70)	dz=-(0x70); else dz=-(0x50);
  //	DoPrintf ("vr=%c %02x %02x %s ",*VERSION,startz,dz,p);
	for (int i = 0; i < len; i++, startz++) {

		char c = *p++;
		for (t = 1; t <= 2; t++) {
			z = startz;
			if (t == 2)    //$$$$$$$$$$$$
				{z = z + dz; z&=0x77;    }
			if (t == 1)
				c |= 0x20;

			if (c == ' ')
				continue;
			for (k = 1; k < 8; k++)
				if ((c | ' ') == *(".prnbqk" + k))
					break;

			piece[PN].rank = c;
			piece[PN].z = z;
			piece[PN].t = t;
			piece[PN].type = k;
			piece[PN].showx = basex + sz * (z % 8);
			y=z/16;   	if(V2)y=7-y;
			piece[PN].showy = basey + sz * y;

			board[z] = piece + PN;
			PN++;
		}
	}

}

bool chessrule::linesearch(int dir1, int dir2, int count, int startz,
	int target) {
	for (int i = dir1; i < dir2; i++) {
		int z = startz;
		for (int j = 0; j < count; j++) {
			z = z+drtb[i];
			if (z == target)
				return true;
			if (z & 0x88)
				return false;
			if (board[z] != 0)
				return false;
		}
	}
	return false;
}

bool chessrule::Attack(int targetz, xPIECE *p) {
	char rank = p->rank | 0x20;
	int startz = p->z;
	int z, t = p->t;
	int difforwad = (t == 1) ? -0x10 : 0x10;
	switch(rank) {
	case 'p':
		if ((startz + 1 + difforwad) == targetz)
			return true;
		if ((startz - 1 + difforwad) == targetz)
			return true;
	case 'n':
		return linesearch(8, 16, 1, startz, targetz);
	case 'k':
		return linesearch(0, 7, 1, startz, targetz);
	case 'r':
		return linesearch(0, 3, 7, startz, targetz);
	case 'b':
		return linesearch(0, 3, 7, startz, targetz);
	case 'q':
		return linesearch(0, 3, 7, startz, targetz);
	}
	return false;
};

bool chessrule::Repeat3() {
	return false;
}

bool chessrule::Stealmate() {
	return false;
}

void chessrule::generate(int z, xPIECE *p, xPIECE *cap, SPE spe) {
	MOVE m;

//if(step>=30) DoPrintf("old =%02x %c new=%02x",p->z,p->rank,z);
	m.newz = z;
	m.oldz = p->z;
	m.p = p;
	m.cap = cap;
	if(cap)DoPrintf("z=%02x p->z=%02x %02x", z,p->z,cap->z);
	m.spe = spe;
	movelist.push_back(m);
}

bool chessrule::genmove(int t) {
	int x, y, i, ia;
	xPIECE *p;
	ia = (t == 1) ? 0 : 1;
	movelist.clear();
	for (i = ia; i < 32; i++, i++) {
		p = piece + i;
		if(p->t==0) continue;        //$2019-5-7
		char rank = p->rank;
		rank |= 0x20;
		switch(rank) {
		case 'p':
			pawnsearch(p->z, p->t); // empassant();
			break;
		case 'r':
			gensearch2(0, 4, 7, p->z);
			break;
		case 'n':
	 //	if(step>17) DoPrintf("knight %02x",p->z);
			gensearch2(8, 16, 1, p->z);
			break;
		case 'b':
			gensearch2(4, 8, 7, p->z);
			break;
		case 'q':
			gensearch2(0, 8, 7, p->z);
			break;
		case 'k':
			gensearch2(0, 8, 1, p->z);
			castling(p);
			break;

		}

	}
	DoPrintf("gensize=%d",movelist.size());
	return movelist.size() > 0;

}

bool isMyColor(int z, int teban, xPIECE *p) {
	if (p == 0) 	return false;
	return(p->t == teban);
}

void chessrule::gensearch2(int dirstart, int dirend, int count, int startz) {
	int t, j, i, z;
	xPIECE *p, *q;

	for (i = dirstart; i < dirend; i++) {
		z = startz;
		p= board[z];
		for (j = 0; j < count; j++) {
			z = z+ drtb[i];
				if (z & 0x88) 				break;
	  //		DoPrintf("i=%d j=%d z=%02x", i, j, z);

			// t=board[z]->t;
			// cout<<dec<<" i="<<i<<" j="<<j<<" t="<<t<<" z="<<hex<<z<<endl;

			if (isMyColor(z, teban, board[z])) 		break;
			   q=board[z];if(q->t==0  ) q=0;
			generate(z, p,q, 0);
			if (isMyColor(z, 3 - teban, board[z]))
				break;
		}
	}

}

void chessrule::pawnsearch(int oldz, int t) {
	xPIECE *p, *q, *cap;
		xPIECE *temp;
	// １マス前進
	int col,newz;

	int dz ,newz1;
	int originaly;

	 dz = (t == 2) ? 0x10 :- 0x10;
	 originaly = (t == 2) ? 0x10 : 0x60;
	newz =oldz+ dz;

	temp=emptyboard;

	if (newz & 0x80)  		return;
	// cout<<"pawn "<<hex<<newz<<endl;
	p = board[oldz];
	temp = board[newz] ;
			 col=temp->t;


	if (col == 0) {
		generate(newz, p, 0, 0);
		if ((oldz & 0xf0) == originaly) {
			// 2マス前進
			newz = newz+ dz;


	 temp = board[newz] ;
			 col=temp->t;


			if (col== 0) {
				generate(newz, p, 0, 0);
			}
		}
	}
   //		DoPrintf("enter parn search old=%02x ",oldz) ;
	// capture
	for (int i = 0; i < 2; i++) {
		int dx = (i == 0) ? 1 : -1;
		newz = oldz+ dz + dx;

		if (newz & 0x88)	continue;
		q = board[newz];
	if(q!=emptyboard){
	  //	DoPrintf("newz=%02x",newz);
		if (q  && ( q->t) == (3 - teban)) {
  //			DoPrintf ("Pawn tkw%02x %02x q=%02x ", newz,p->z, q->z);

			generate(newz, p, q, 0);   }
		// if(q->t) continue;
		 }
   else{

	// empassant
	int midy = (t == 1) ? 0x50 : 0x40;
	if ((oldz & 0x70) != midy) 		continue;

   //	  assert(p->z==oldz);
	if(step<3) return;
	MOVE *mm = &record[step - 2];
	q = mm->p;
   //	DoPrintf("i=%d oldz= %02x %02x mm %02x %02x rank=%c",i, oldz,newz,
	//mm->newz,mm->oldz,mm->p->rank);

	if (!(q->rank == 'p' || q->rank =='P'))return;  //pawn に限る
	if (abs(mm->oldz/16 - mm->newz / 16) != 2)	continue;
	if (abs((mm->oldz & 0x07)-(oldz&0x0f)) != 1) continue;;
	newz=(mm->newz+mm->oldz)/2;//中点
  //	DoPrintf ("emp %02x %02x q=%02x ", newz,p->z, q->z);


	generate(newz, p, q, empassant);
	 }
	 }
}



bool chessrule::getxyrt(int x, int y, int *showx, int *showy, int *col,
	int *type) {
	int z = y * 16 + x;
	xPIECE *p = board[z];
	if(V3){p->t=p->t;}
	else if (p == 0)	return false;
	*showx = p->showx;
	*showy = p->showy;
	*type = p->type;
	*col = p->t; ;
	return true;
}

// ---------------------------------------------------------------------------
bool chessrule::setCurrentP(int z) {
	currentP = board[z];
	if(V3){
		if (board[z] == emptyboard) 		return false ;
	}
	else 	if (board[z] == 0) 		return false;
	return(currentP->t == teban);
};

void chessrule::moveCurrent(int x, int y) {
	currentP->showx = x;
	currentP->showy = y;
}

bool chessrule:: Translate(char *str,MMM *mmm){
	   if(!genmove(teban)) return false;



	   int newz,oldz,move_p,cap_p,len;
	   int ii;
		len=strlen(str);

		for (ii = 0; ii < movelist.size(); ii++) {
		DoPrintf("%02x %03x==>%02x %02x",movelist[ii].oldz ,oldz,
		 movelist[ii].newz ,newz);

		if(movelist[ii].newz != newz) continue;
		if(move_p>0 && (move_p != movelist[ii].p->rank)) continue;
		if(cap_p>0 && (move_p != movelist[ii].p->rank)) continue;

		mmm->newz=newz;
		mmm->oldz=oldz;
		mmm ->PP=move_p;
		mmm->QQ=cap_p;
		mmm->spe= movelist[ii].spe;
		return true;
	 }
	return false;


}
// ---------------------------------------------------------------------------
