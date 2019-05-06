#include  <stdlib.h>
#include "hash.h"
#include "wcdefs.h"
#include "dfpnmid.h"

#define max(a, b)  (((a) > (b)) ? (a) : (b))
#define min(a, b)  (((a) < (b)) ? (a) : (b))
#define F_UNCERTAIN 4
#define F_LIVE 1
#define F_KILL 2
 bool Play(MOVEPACK *);
 void Back();
int getNextMoves(int color,MOVEPACK *);
extern  __int64 hashcode;

 extern MOVEPACK mpack[64];
extern int GCount2;
extern int TraceAt2;
extern int teban;
extern HASH *myhash;
extern int Attacker,Target    ;
extern int levelMax;
extern int DeepestLevel;
extern int  *hashlastptr;

static bool kill;


static bool timeOut;

static int nodeID;


//prototype-----------------
int  checkMyTime(int t);
bool updatePnDn(NODE *p);
bool expand(NODE *p) ;
 bool getaExpand(NODE *p);
 int getaJudge();
bool MID(NODE *p) ;
void deletesons(NODE *);

NODE * SelectChild(NODE *p ,int *v1,int *v2,int *v3,int *v4);
void sgfcreate(int ,char *filename)  ;
//-----------------------------------
bool DSX(int) {return true;;}
void DoPrintf(char *szFormat,...);

void trace(NODE *p) {

}

//-----------------------------------
//    Multiple Iterative Deepening
//-----------------------------------


bool MID(NODE *p) {
    bool brc;
    int rc;
    int n,repn,thredn,vsum,vpn,vdn,vmin2,pn1,dn1,status;
    int savestep;

 // if (p->zeye && ((goban->EEmap+(p->zeye->point))->type&TYPE_EYE)==0) goban->addEElist(p->zeye,3);
  if (p->level>levelMax){

 // while(p->level>0) SZ(p->z),p=p->father;   assert(0);
 //  p->dn=INFINITY; p->pn=0;
  p->pn=INFINITY; p->dn=0;
  return false; }

 //NODE p がfrontieerならその下の子供のNodeを展開する
 //展開中に解をみつけたら帰る。

  if (p->son==0) {
   if (expand(p)) {
      *(p->hashStatusPtr) = (p->status&31)+(p->ID*32);
    deletesons(p->son);p->son=0;
    return true;
   } }

 while(1){
 if (checkMyTime(0)<0) {timeOut=true;return false; }
    if (updatePnDn(p)){
        *(p->hashStatusPtr) = (p->status&31)+(p->ID*32);

       if (p->level>1 ) {
         deletesons(p->son);p->son=0;
        return true;}
      if (( p->level==1)&& (p->pn==0 && kill) ||(!kill&& p->dn==0))   {

      rc=p->pn;p->pn=p->dn;p->dn=rc;

     }
        return true;
   }
 //閾値を 超えたらやめる
 if (DSX(p->level))trace(p);
    if (p->pn>=p->thpn||p->dn>= p->thdn)  return false;
     NODE *q= SelectChild(p,&vsum,&vpn,&vdn,&vmin2);
 //最良の子供qの閾値を計算 ２番目の子供 も使う

    if (q->mode) {
    q->thpn= min(p->thpn, vmin2+1);
    q->thdn= p->thdn+ vdn-vsum;
    }
    else {
      q->thdn= min(p->thdn, vmin2+1);
      q->thpn= p->thpn+ vpn -vsum;
    }
    q->thpn =min(INFINITY,q->thpn);
    q->thdn =min(INFINITY,q->thdn);


 if (!Play(q->z))
    {
    q->pn=0;q->dn=INFINITY; return false;   }
    brc= MID(q);
  Back();

    if (timeOut) break;
    }         //while
   return false;
 }


 bool updatePnDn(NODE *p) {
   int sumval=0;
   int minval=INFINITY;
   int oldpn=p->pn;
   int olddn=p->dn;
   char kc=0;
   char kcmax=0;
   bool canPass=true;
     NODE *q;

     p->status= (p->status &16)+F_UNCERTAIN;
    if (p->mode) {    //OR node
    for (q= p->son; q;q=q->brother){
         if (q->z==0)  canPass=false;
         sumval = min(INFINITY,q->pn+sumval) ;
         minval=  min(minval,q->dn);
         kcmax= max(kcmax,q->kocount);
         if (p->level==0 && q->dn==0) {p->z=q->z; }
         if (minval==0) {kc=q->kocount;break;}
    }
    p->pn=sumval;
    p->dn=minval;
    }
    else {    //AND node
     for (q= p->son; q;q=q->brother){
         if (q->z==0) canPass=false;
         sumval= min(INFINITY,sumval+q->dn) ;
         minval= min(minval,q->pn);
          kcmax= max(kcmax,q->kocount);
         if (p->level==0 && q->pn==0) {p->z=q->z; }
         if (minval==0) {kc=q->kocount;break;}
    }
    p->dn=sumval;
    p->pn=minval;
    }

    if (minval==0)
      {
      if (DSX(p->level)) DoPrintf("id=%d stat=%d",p->ID,p->status);

      p->kocount=kc;
      p->status= p->mode?F_LIVE:F_KILL; trace(p); return true;}
    if (p->pn==0 || p->dn==0)
     {
        if (DSX(p->level)) DoPrintf("id=%d stat=%d",p->ID,p->status);
      p->kocount=kcmax;
     p->status= !p->mode?F_LIVE:F_KILL; trace(p);return true;}
    if(p->pn==oldpn && p->dn==olddn ) p->pn++, p->dn++;
    return  false ;

 }
 NODE *  SelectChild(NODE *p,int *sumv,int *pnc,int *dnc, int *minval2){

     int minv=INFINITY;
     int u,v;
     int sum=0;
     int minv2=INFINITY;
    NODE *q,*r,*s;
      s=0;
    for (q= p->son; q;q=q->brother){

       if (q->mode) {u= q->dn; v=q->pn;  }
       else{    u= q->pn;v=q->dn  ;}
       sum+=u;
       if (sum> INFINITY) sum=INFINITY;
      if (v <minv) {
          minv2=minv;
          minv=v;
           r=q;
       }
      else if (v<minv2) minv2=v;
     }
     *sumv=sum;
     *pnc=*dnc=minv;
     *minval2=minv2;
       return r;
    }

void deletesons(NODE *p) {

     if (!p) return ;
    deletesons(p->son); deletesons(p->brother);  delete p;
}

bool  expand(NODE *p) {

       int stat,rc,rc2,i, n,PN,DN,v,where,rcx,NN,arm;


       NODE *q,*r;
       NODE *qq=0;
       MOVEPACK *z;
       MOVEPACK movepack[64];
       if (p->level>levelMax) {
        p->pn=0,p->dn= INFINITY ;  p->status=F_KILL;
       return false;
       }

       DeepestLevel=max(DeepestLevel, p->level);
     n= getNextMoves(teban,movepack);
     if (n==0){
       p->pn=0,p->dn= INFINITY ;  p->status=F_KILL;
       return false;
     }
        for (i=0;i<n;i++) {
        z=  &mpack[i];
           if (!Play(z))  continue;

         __int64  GH= hashcode;
         rcx=myhash->find((unsigned char *)(&GH),&stat);

         if (rcx==0){

           stat=0;



           myhash->add((unsigned char *)(&GH), nodeID*32+stat);

           }
         else {      stat &=15;


         }

         NODE *q= new NODE();
           q->mode=! p->mode;   //mode 反転
           q->z=&mpack[i];
           q->ID=nodeID++;
           q->father=p;   //??
           q->brother=0;    q->son=0;
           q->ko= (rc==-3);

           q->hash=hashcode;
           q->level=p->level+1;
           q->pn=q->dn=0;

           q->hashStatusPtr=hashlastptr;
           q->seq='a'+i;
           q->kocount=p->kocount;

       if (stat== F_UNCERTAIN )
       stat=(!q->mode)? F_KILL: F_LIVE;

           v= -rc*20;

          if (v>120) v=120;
          if (v<-120) v=-120;
         //  v=0;

           q->status=stat;       //??
            switch(stat){
            case F_KILL : q->dn=INFINITY;  break;
            case F_LIVE:  q->pn=INFINITY;  break;
            case F_UNCERTAIN:NN++;
            q->pn= 1000+v;
            q->dn= 1000-v;

            break;
           }
         if (qq) qq->brother=q;   else p->son=q;
         qq=q;
         Back();
          trace(q);
          if (!p->mode && q->pn==0)  {
          p->pn=0;p->dn=INFINITY; p->status=F_KILL;
          trace(p);  p->kocount=q->kocount;
          if (p->level==0)p->z=z;
          return true;}
          if (p->mode &&  q->dn==0)  {
          p->dn=0;p->pn=INFINITY; p->status=F_LIVE;
           p->kocount=q->kocount;
           if (p->level==0)p->z=z;
          trace(p);
          return true;}

         }
     return false ;
   }



