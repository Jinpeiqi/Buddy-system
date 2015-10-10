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
void *recursive_mymalloc(int m);  
#ifdef __cplusplus
}
#endif

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

typedef struct _head{//use double link list for allocation   
   struct _head *next;   
   struct _head *prev;   
	int mem_size;   
}head;   
   
head *FreeSpaceList=NULL; // initialise FreeSpaceList   
void *mymalloc(int n) { // very simple memory allocation
  // mymalloc use linked allocation method   
   head *tmp=FreeSpaceList,   
        *tmpPrev=NULL;   
   void *p;   
   int blockSize=1,   
       m2=n+sizeof(head);   
   bool empty=true; //when true: not allocated, when false: already allocated   
      
   while(m2>blockSize) blockSize=blockSize*2;   
      
   while(tmp){ // find whether empty or not   
       if (tmp->next){   
           empty=false;   
           break;   
           }   
        tmp=tmp->next;   
       }   
      
   if(empty) {//first time allocate memory   
       p=allocpages(PAGESIZE); // to allocate 4096 pages   
       if(!p){ puts("allocate Failed"); return p; }   
        
        FreeSpaceList=(head*)p;   
        FreeSpaceList->next=NULL;   
        FreeSpaceList->prev=NULL;   
        FreeSpaceList->mem_size=PAGESIZE*PAGESIZE;   
        tmp=(head*)recursive_mymalloc(blockSize);      
        return (void *)((char*)tmp+sizeof(head));//end function   
      }   
       
    else {   
        
        tmp=FreeSpaceList;  
        for (;tmp!=NULL;tmp=tmp->next){ 
            if((tmp->mem_size==blockSize)){  
                if(tmp->prev) { 
                    tmpPrev=tmp->prev;   
                    tmpPrev->next=tmp->next;   
                    if(tmp->next!=NULL)  (tmp->next)->prev=tmpPrev;           
                   }  
                else {   
                    if(tmp->next)    (tmp->next)->prev=NULL;   
                    FreeSpaceList=tmp->next;   
                }   
                return (void*)((char*)tmp+sizeof(head));// will end function   
            }  
        }  
       
    
    do {   
       tmp=(head*)recursive_mymalloc(blockSize*2);   
           tmpPrev=(head *)((char *)tmp+blockSize);   
           FreeSpaceList->prev=tmpPrev;  
            } while(!FreeSpaceList);   
               
           tmpPrev->next=FreeSpaceList;   
           tmpPrev->mem_size=blockSize;   
           tmp->mem_size=blockSize;   
           tmpPrev->prev=NULL;   
           FreeSpaceList=tmpPrev;   
        return (void*)((char*)tmp+sizeof(head)); 
    } 
	}
void *recursive_mymalloc(int m) {   
    head *tmp=FreeSpaceList,   
        *tmpPrev=NULL;   
           
    for (;tmp!=NULL;tmp=tmp->next){   
        if((tmp->mem_size==m)){   
            if(tmp->prev!=NULL) {   
                tmpPrev=tmp->prev;   
                tmpPrev->next=tmp->next;   
                if(tmp->next!=NULL)  (tmp->next)->prev=tmpPrev;   
            }   
           else {   
                FreeSpaceList=tmp->next;   
              if(FreeSpaceList != NULL) (tmp->next)->prev=NULL;   
            }   
            return (void*)tmp;//end of function   
        }   
    }   
    //else, recursive, size times 2   
   tmp=(head*)recursive_mymalloc(m*2);   
    tmpPrev=(head *)((char *)tmp+m);   
    tmpPrev->mem_size=m;   
    tmpPrev->next=FreeSpaceList;    
    if(FreeSpaceList!=NULL) FreeSpaceList->prev=tmpPrev;   
    FreeSpaceList=tmpPrev;   
       
    return (void*)tmp;//end function   
}   

int myfree(void *p) { // very simple free
   head *tmp=(head*)((char*)p-sizeof(head));   
   if (tmp) { //add tmp to the front of the list   
   tmp->prev=NULL;   
    tmp->next=FreeSpaceList;   
    FreeSpaceList->prev=tmp;   
    FreeSpaceList=FreeSpaceList->prev;   
    }   
    else tmp=freepages(p);   
    return (tmp->mem_size-sizeof(head));   
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
         if(n[k][0]!=(unsigned char)(n[k]+s[k]+k))
            printf("Error when checking first byte!\n");
         if(s[k]>1 && n[k][s[k]-1]!=(unsigned char)(n[k]-s[k]-k))
            printf("Error when checking last byte!\n");
         FREE(n[k]);
      }
      size=randomsize(); // pick a random size
      n[k]=(unsigned char *)MALLOC(size); // do the allocation
      s[k]=size; // remember the size
      n[k][0]=(unsigned char)(n[k]+s[k]+k);  // put some data in the first and
      if(size>1) n[k][size-1]=(unsigned char)(n[k]-s[k]-k); // last byte
   }

   // print some statistics
   printf("That took %.3f seconds and used %d bytes\n",
         ((float)(cputime()-start_time))/1000,
         start_mem-memory());

   return 1;
}

