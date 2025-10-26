#include <stdio.h>


typedef struct
{
    int i;
} A;


void fxn( void * (*arrr)[5] )
{
    int ggg2 = sizeof( arrr );
    int ggg = sizeof( *arrr );

    int uu = 1 + 2;
}

typedef A * pA;

int main()
{
    pA arr[5];
    fxn( &arr );


    return 0;
}