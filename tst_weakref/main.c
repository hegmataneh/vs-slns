#include <stdio.h>

void * _malloc_dbg2(
	       size_t      _Size ,
	       int         _BlockUse ,
	 char const * _FileName ,
	       int         _LineNumber
) __attribute__((weakref("_malloc_dbg")));

int main()
{
    


    return 0;
}