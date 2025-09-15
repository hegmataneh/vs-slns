#include <stdio.h>

//int iiii = 0;
//
//__attribute__( ( constructor ) )
//static void my_init( void )
//{
//	iiii = 1;
//}
//
//__attribute__( ( constructor ) )
//void my_init2( void )
//{
//	iiii = 2;
//}

extern int __iii;


int main()
{
	__iii = 1;

    printf("hello from %s!\n", "tst_compiler_attrib");
    return 0;
}