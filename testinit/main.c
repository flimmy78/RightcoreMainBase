/* main */
/* lang=C99 */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_INIT		0		/* use pragma init (only Solaris�?) */


/* revision history:

	= 2017-09-07, David A�D� Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright � 2017 David A�D� Morano.  All rights reserved. */

/*******************************************************************************

	We test the GCC pragma 'init'.

	Does this work only for the Solaris� compilers?
	It appears to be the case, that this only works for the Solaris�
	compilers). But the GNU compiler (GCC) does have some similar
	mechanism, I just forgot what it is!


*******************************************************************************/


#include	<envstandards.h>
#include	<stdio.h>


/* local defines */


/* pragmas */

#if	CF_INIT
#pragma		init(maininit)
#endif


/* name-spaces */


/* local structures (and methods) */


/* global variables */


/* exported subroutines */

void maininit()
{
	printf("main-init!\n") ;
}

int main(int argc,const char **argv,const char **envv)
{
	printf("hello world!\n") ;
	return 0 ;
}
/* end subroutine (main) */


