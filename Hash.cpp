#include "hash.h"
#include <string.h>
int *hashlastptr;
class	 Map {
	// entry to map key to value
public:
	Map(Map* n, unsigned char* k, int v);
	int   	getValue() {
    hashlastptr=&val;
      return val;}
	Map*				find(unsigned char* k);
	void*	operator	new(size_t size);
	void	operator	delete(void*);
private:
	Map*				next;
	int					val;
	unsigned char		key[1];

//static
public:
	static	void		setKeyLength(int kLen);
	static	void		setCountEstimate(int c);
private:
	static	int			countEstimate;
	static	size_t		size;
	static	int			keyLength;
	static	int			freeCount;
	static	char*		freeMaps;};

Map::Map(Map* n, unsigned char* k, int v){
	next=n;
	val=v;
    hashlastptr=&val ;
	memcpy(key, k, keyLength);}

Map*	Map::find(unsigned char* k){
	Map* map=this;
	for(; map&&memcmp(map->key, k, keyLength); map=map->next);

	return map;}

void*	Map::operator new(size_t){
	if(!freeCount){
		//なくなるとcountEstimate個をまとめて取るので
		//countEstimateの値は概数でよい
		freeMaps=new char[size*countEstimate];
		freeCount=countEstimate;}
	char* alloc=freeMaps;
	freeMaps=(char*)freeMaps+size;
	--freeCount;
	return alloc;}

void	Map::operator delete(void*){}

int		Map::countEstimate;
size_t	Map::size;
int		Map::keyLength;
int		Map::freeCount=0;
char*	Map::freeMaps;

void	Map::setCountEstimate(int c){
	countEstimate=c;}

void	Map::setKeyLength(int kLen){
	Map* x;
	keyLength=kLen;
	size=((sizeof(x->next)+sizeof(x->val)+kLen+3)/4)*4;
	}

HASH::HASH (int sSize,long countEstimate,int keySize)
{
	Map::setKeyLength(keySize);
	Map::setCountEstimate(countEstimate);
	slotSize=sSize;
	typedef Map* Mapp;
	slots=new Mapp[slotSize];
	for(int i=0;i<slotSize;i++) slots[i]=0;
	}

HASH::~HASH() {
	delete slots;}
 
int HASH::add(unsigned char* key,int val) {
	Map** slot=getSlot(key);
	Map* map=(*slot)->find(key);
	if (map){
		return 0;} // fail, alread existed
	else{
		*slot=new Map(*slot, key, val);	// last come, first found basis
		return 1;}}

int HASH::find(unsigned char* key,int *val) {
	Map** slot=getSlot(key);
	Map* map=(*slot)->find(key);
	if (map) {
		*val = map->getValue();
		return 1;}
	else{
		return 0;}}

Map** HASH::getSlot(unsigned char* key) {
	//ハッシュするのに使うslotSizeの値は
	//大きめの任意の大きさにできるので
	//キーは最初から比較しないといけない
	unsigned short hash= *(unsigned short*) key;
	return &slots[hash%slotSize];}
#ifdef DDD
#define MAXENTRY 10000000
int main(){
	unsigned char rec[10]={1,2,3,4,5,6,7,8,9,10};
  //	set_new_handler(0);
	
	HASH * hash=new HASH (0x40000, MAXENTRY, 8);
	long i;
	for ( i=0; i<MAXENTRY-1; i++){
		long k= i;
		*( (long *) rec)=k;
		assert( hash->add(rec,  i%32 )>0);
	}
	
	for (i=0; i<MAXENTRY-1; i++){
		long k= i;
		*( (long *) rec)=k;
		int v;
		if(hash->find(rec,&v)==0){
				assert( hash->find(rec,&v)>0);}
		if (v!= (i%32)) printf("i=%ld status err",i);
	}
	printf("=============");
	return 0;}
#endif
