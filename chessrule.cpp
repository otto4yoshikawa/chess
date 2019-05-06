// ---------------------------------------------------------------------------
#include"chessrule.h"
void DoPrintf(char*szFormat, ...);
int drtb[] = {
	1, -1, 0x10, -0x10, 0x11, -0x11, 0x0f, -0x0f,
	0x0E, -0x0E, 0x12, -0x12, 0x1f, -0x1f, 0x21, -0x21};



bool safespot(xPIECE *p1, xPIECE*p2) {
//�Q�_���܂߂Ă��� �Ԃɑ���̌��������邩
	int z1, z, z2;

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
	if (moved(p, &record)) 		return;   //�����������Ƃ���
	xPIECE *rook;
	int t = p->t;

	// king side castling
	int rookz = (t == 1) ? 0x07 : 0x77;
	rook = board[rookz];
	if (moved(rook, &record)) 	goto aa;
	if (!safespot(p, rook))		goto aa;
	generate(rookz, p, rook, kingsidecas);

	// qeen side  castling
aa:
	rookz = (t == 1) ? 0x00 : 0x70;
	rook = board[rookz];
	if (moved(rook, &record))		return;
	if (!safespot(p, rook)) 		return;
	generate(rookz, p, rook, qeensidecas);
}

// --------------------------
// constructor
// --------------------------
chessrule::chessrule() {
	step = 1;
	teban = 1;
	PN = 0;

	for (int z = 0; z < 0x80; z++) {
		if (z & 0x08)
			continue;
		board[z] = 0;
	}

	PlacePiece(0x04, "K");
	PlacePiece(0x00, "RNBQ BNR");
	PlacePiece(0x10, "PPPPPPPP");
	stat = 0;
	teban = 2;
	// showBoard();
}

// ----------------------------------
void chessrule::Play(MOVE m) {
	xPIECE *temp, *p = m.p;     //from
	xPIECE *cap = m.cap;
	int z3;
	int kingy,rookxold,rookxnew,kingnewx,oldking;
	kingy=-1;
	if (m.spe == kingsidecas) {
	rookxold=7,rookxnew=5;kingnewx=6;
     }
	if( m.spe==qeensidecas) {
		 rookxold=0,rookxnew=3; kingnewx=2;
	}
	if(kingy>=0){  // castling
			kingy=p->z&0xf0;
			oldking=p->z;
		temp = board[kingy+rookxold];
		p->z= kingy+kingnewx;
		board[oldking] = 0;
		board[kingy+rookxold] = 0;
		board[p->z]=p;
		board[kingy+rookxnew]=temp;
	}

	else {
		DoPrintf("from %02x ==> %02x",m.oldz,m.newz);
		board[m.newz] = p;
		p->z = m.newz;
		board[m.oldz] = 0;
		if (cap)cap->t = 0;    //	board[cap->z] = 0;
		if(m.spe==2) board[m.cap->z]=0;
		if (m.spe ==queening) p->rank += ('Q' - 'P');

	}
	// p->z=m.newz;
	p->showx = basex + (sz * (p->z & 7));
	p->showy = basey + (p->z) / 16 * sz;
	record.push_back(m);
	step++;
	teban = 3 - teban;
}

void chessrule::Back() {
	MOVE m = record[1];
	record.pop_back();
	xPIECE *p = m.p;
	xPIECE *cap = m.cap;
	if (m.spe == 1) {
		board[m.newz] = board[m.oldz];
		board[m.oldz] = p;
	}

	else {
		board[m.oldz] = p;
		board[m.newz] = 0;
		if (cap) {
			cap->t = 3 - teban;
			board[m.newz] = cap;
			if (m.spe == 2)
				board[cap->z] = cap;
		}
		if (m.spe == 3)
			p->rank -= ('Q' - 'P');
	}
	p->z = m.oldz;
	step--;
	teban = 3 - teban;
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
	p1 = board[z1];
	DoPrintf("rank=%c z=%02x", p1->rank, p1->z);
	 currentP->showx=  basex + sz * (z1 % 8);
	  currentP->showy=  basey + sz * (z1 /16);
	// p2=board[y2][x2];
	if (p1->t != teban) {
		message = "�s���ȋ�";
		return false;
	}
	if (!genmove(teban)) 	return false;

	DoPrintf("movelist= %d", movelist.size());
	for (ii = 0; ii < movelist.size(); ii++) {
	DoPrintf("%02x %03x==>%02x %02x",movelist[ii].oldz ,z1, movelist[ii].newz ,z2);
		if ((movelist[ii].oldz == z1) && (movelist[ii].newz == z2))
			break;
		}
	if (ii == movelist.size()) {
		message = "�s���Ȉړ�";
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
	int x, y;
	for (y = 0; y < 8; y++) {
		*p++ = ' ';
		*p++ = ' ';
		for (x = 0; x < 8; x++) {
			*p++ = board[x + 16 * y]->rank;
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
	start = (opponent == 1) ? 0 : 1;
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
	int k, dz = 0x70 - y * 2;
	// cout <<len<< " "<<startz<<" s="<<p<<endl;
	for (int i = 0; i < len; i++, startz++) {

		char c = *p++;
		for (t = 1; t <= 2; t++) {
			z = startz;
			if (t == 2)
				z = z + dz;
			if (t == 2)
				c |= 0x20;
			// cout<<"t="<<t<<" PN="<<PN<<" c="<<c;
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
			piece[PN].showy = basey + sz * (z / 16);
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

 if(z==0x33) DoPrintf("bz =%02x %c %02x",p->z,p->rank,z);
	m.newz = z;
	m.oldz = p->z;
	m.p = p;
	m.cap = cap;
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
		char rank = p->rank;
		rank |= 0x20;
		// cout<<"movegen i="<<dec<<i<<" rank="<<p->rank<<" z="<<hex<<p->z<<endl;
		switch(rank) {
		case 'p':
			pawnsearch(p->z, p->t); // empassant();
			break;
		case 'r':
			gensearch2(0, 4, 7, p->z);
			break;
		case 'n':
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
		  //	castling(p);
			break;

		}

	}
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

			generate(z, p,board[z] , 0);
			if (isMyColor(z, 3 - teban, board[z]))
				break;
		}
	}

}

void chessrule::pawnsearch(int oldz, int t) {
	xPIECE *p, *q, *cap;
	// �P�}�X�O�i
	// cout<<"pawn";]\]\\\

   //	DoPrintf("enter parn search");
	int newz;
	int dz = (t == 1) ? 0x10 :- 0x10;
	int originaly = (t == 1) ? 0x10 : 0x60;
	newz =oldz+ dz;

	if (newz & 0x80)  		return;
	// cout<<"pawn "<<hex<<newz<<endl;
	p = board[oldz];
	// DoPrintf("dz=%02x newz=%02x old=%02x",dz,newz,p->z);
	if (board[newz] == 0) {
		generate(newz, p, 0, 0);
		if ((oldz & 0xf0) == originaly) {
			newz = newz+ dz;
			if (board[newz] == 0) {
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
	  //	DoPrintf("newz=%02x",newz);
		if (q  && ( q->t) == (3 - teban)) {
			DoPrintf ("Pawn tkw%02x %02x q=%02x ", newz,p->z, q->z);

			generate(newz, p, q, 0);   }
	}
	// empassant
	int midy = (t == 0) ? 0x40 : 0x30;
	if ((oldz & 0xf0) != midy)
		return;
	if(step<3) return;
	MOVE *mm = &record[step - 2];
	q = mm->p;
	DoPrintf("mm %02x %02x rank=%c",mm->newz,mm->oldz,mm->p->rank);

	if (!(q->rank == 'p' || q->rank =='P'))return;  //pawn �Ɍ���
	if (abs(mm->oldz/16 - mm->newz / 16) != 2)	return;
	if (abs((mm->oldz & 0x07)-(oldz&0x0f)) != 1) return;
	newz=(mm->newz+mm->oldz)/2;//���_
	DoPrintf ("emp %02x %02x q=%02x ", newz,p->z, q->z);


	generate(newz, p, q, empassant);

}



bool chessrule::getxyrt(int x, int y, int *showx, int *showy, int *col,
	int *type) {
	int z = y * 16 + x;
	xPIECE *p = board[z];
	if (p == 0)	return false;
	*showx = p->showx;
	*showy = p->showy;
	*type = p->type;
	*col = p->t; ;
	return true;
}

// ---------------------------------------------------------------------------
bool chessrule::setCurrentP(int z) {
	currentP = board[z];
	if (board[z] == 0)
		return false;
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