#ifndef HASHHH
#define HASHHH


#include <stdio.h>
#include <assert.h>
//#include <alloc.h>
//#include <new.h>

class	Map;
class	HASH{
public:
	HASH(int sSize, long countEstimate, int keySize);
	~HASH();
	
	int		add(unsigned char *rec ,int );
	int		find(unsigned char *rec,int * );
private:
	Map**	getSlot(unsigned char *rec );
	Map**	slots;
	int		slotSize;
};
#endif

