#include <stdio.h>
#include <time.h>

// 159.335 assignment 3
// This is a working memory allocation program
// but it is slow and uses lots of memory.
// Martin Johnson 2000-2014

// the following is fixed by the OS
// you are not allowed to change it
#define PAGESIZE 4096
// you may want to change the following lines if your
// machine is very faster or very slow to get sensible times
// but when you submit please put them back to these values.
#define NO_OF_POINTERS 2000
#define NO_OF_ITERATIONS 200000
// change the following lines to test the real malloc and free
#define MALLOC mymalloc
#define FREE myfree
// The following ugly stuff is to allow us to measure
// cpu time.
#h
typedef struct { unsigned long l,h; } ti;
typedef struct { unsigned long sz,ml,tp,ap,tpg,apg,tv,av; } ms;
#ifdef __cplusplus
extern "C" {
#endif
unsigned long * _stdcall VirtualAlloc(void *,unsigned long,unsigned long,unsigned long);
int _stdcall VirtualFree(void *,unsigned long,unsigned long);
void _stdcall GlobalMemoryStatus(ms *);
void * _stdcall GetCurrentProcess(void);
unsigned long _stdcall GetVersion(void);
int _stdcall GetProcessTimes(void *, ti *,ti *, ti *, ti *);
void _stdcall Sleep(unsigned long);
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------
struct Node{
	int size;
	struct hearder *prev;
	struct hearder *next;
	}header;


//------------------------------------------------------------	
int cputime(void) { // return cpu time used by current process
   ti ct,et,kt,ut;
   if(GetVersion()<0x80000000) {  // are we running on recent Windows
      GetProcessTimes(GetCurrentProcess(),&ct,&et,&kt,&ut);
      return (ut.l+kt.l)/10000; // include time in kernel
   }
   else return clock(); // for very old Windows
}
int memory(void) { // return memory available to current process
   ms m;
   GlobalMemoryStatus(&m);
   return m.av;
}

// you are not allowed to change the following function
void *allocpages(int n) { // allocate n pages and return start address
   return VirtualAlloc(0,n * PAGESIZE,4096+8192,4);
}

// you are not allowed to change the following function
int freepages(void *p) { // free previously allocated pages.
   return VirtualFree(p,0,32768);
}


//~ void *mymalloc(int n) { // very simple memory allocation
   //~ void *p;
   //~ p=allocpages((n/PAGESIZE)+1);
   //~ if(!p) puts("Failed");
   //~ return p;

//~ }
//------------------------------------------------------------------
void *mymalloc(int n) { 
	if(n==0){
		return null;
	}
	n=n+sizeof(int);
	int ns = find_optimal_memory_size(n);
}
//-------2^k--------------------------------------------------------
static inline int find_optimal_memory_size(int n){
	int suitable_size=PAGESIZE;	
	while(suitable_size < n){
		suitable_size=suitable_size << 1;// 32 << 1 = 64
		}
	return suitable_size;	
}


//-------------------------------------------------------------------
int myfree(void *p) { // very simple free
   int n;
   n=freepages(p);
   if(!n) puts("Failed");
   return n;
}

unsigned seed=7652;

int myrand() { // pick a random number
   seed=(seed*2416+374441)%1771875;
   return seed;
}

int randomsize() { // choose the size of memory to allocate
   int j,k;
   k=myrand();
   j=(k&3)+(k>>2 &3)+(k>>4 &3)+(k>>6 &3)+(k>>8 &3)+(k>>10 &3);
   j=1<<j;
   return (myrand() % j) +1;
}


int main() {
   int i,k;
   unsigned char *n[NO_OF_POINTERS]; // used to store pointers to allocated memory
   int size;

   int s[5000]; // used to store sizes when testing

   int start_time;
   int start_mem;


   for(i=0;i<NO_OF_POINTERS;i++) {
      n[i]=0;     // initially nothing is allocated
   }

   start_time=cputime();
   start_mem=memory();

   for(i=0;i<NO_OF_ITERATIONS;i++) {
      k=myrand()%NO_OF_POINTERS; // pick a pointer
      if(n[k]) { // if it was allocated then free it
         // check that the stuff we wrote has not changed
         if(n[k][0]!=(unsigned char)(int)(n[k]+s[k]+k))
            printf("Error when checking first byte!\n");
         if(s[k]>1 && n[k][s[k]-1]!=(unsigned char)(int)(n[k]-s[k]-k))
            printf("Error when checking last byte!\n");
         FREE(n[k]);
      }
      size=randomsize(); // pick a random size
      n[k]=(unsigned char *)MALLOC(size); // do the allocation
      s[k]=size; // remember the size
      n[k][0]=(unsigned char)(int)(n[k]+s[k]+k);  // put some data in the first and
      if(size>1) n[k][size-1]=(unsigned char)(int)(n[k]-s[k]-k); // last byte
   }

   // print some statistics
   printf("That took %.3f seconds and used %d bytes\n",
         ((float)(cputime()-start_time))/1000,
         start_mem-memory());

   return 1;
}

