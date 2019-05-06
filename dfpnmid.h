
class NODE   {
   public:
   NODE *father,*son,*brother,*prev,*next;
   int ID ,status,pn,dn,level,step,thpn,thdn;
   bool mode,transfer,ko,passAllow;
   char seq,kocount;
   MOVEPACK *z;
   int *hashStatusPtr;
   __int64 hash;
  NODE (){
  father=0;
  son=0;
  ko=false;
  passAllow=false;
  brother=0;
  pn=1;dn=1;
  prev=next=this;
  };
  ~NODE(){};

}  ;
 
#define INFINITY 10000000
 