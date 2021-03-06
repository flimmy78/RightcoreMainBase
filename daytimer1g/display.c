/* display */

/* handle display functions */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_WRITE	1		/* actually do the 'write(2)' ? */


/* revision history:

	= 88/02/01, David A�D� Morano

	This subroutine was originally written.


	= 98/05/01, David A�D� Morano

	This subroutine was modified to not write out anything
	to standard output if the access time of the associated
	terminal has not been changed in 10 minutes.


	= 98/12/01, David A�D� Morano

	I modified this file to only handle pure display functions.


*/


/************************************************************************

	This module performs all of the display functions for
	the 'daytimer' program.


*************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<sys/wait.h>
#include	<stropts.h>
#include	<poll.h>
#include	<time.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<termstr.h>

#include	"localmisc.h"
#include	"display.h"



/* local defines */

#define	DISPLAY_MAGIC	0x985426
#define	CTIMELEN	19		/* used string length from ctime() */



/* external subroutines */

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_log(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */

static const char	blanks[] = "                                              " ;







int display_init(dp,fd,f_where,t_refresh,t_offset)
DISPLAY		*dp ;
int		fd ;
int		f_where ;
time_t		t_refresh, t_offset ;
{
	struct ustat	sb ;

	int	rs ;


	if (dp == NULL)
	    return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("display_init: entered fd=%d\n",fd) ;
#endif

	rs = u_fstat(fd,&sb) ;

	if (rs >= 0) {

	    dp->f.status = (f_where) ? TRUE : FALSE ;

	    dp->last_str[0] = '\0' ;
	    dp->fd = fd ;
	    dp->t_full = 0 ;
	    dp->t_refresh = t_refresh ;
	    dp->t_offset = t_offset ;
	    dp->magic = DISPLAY_MAGIC ;
	}

#if	CF_DEBUGS
	debugprintf("display_init: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (display_init) */


int display_finish(dp)
DISPLAY		*dp ;
{

	if (dp == NULL)
	    return SR_FAULT ;

	if (dp->magic != DISPLAY_MAGIC)
		return SR_NOTOPEN ;

	dp->magic = 0 ;
	return SR_OK ;
}
/* end subroutine (display_finish) */


/* show the time string on the screen */
int display_show(dp,daytime,f_mail)
DISPLAY		*dp ;
time_t		daytime ;
int		f_mail ;
{
	time_t	t_display ;

	int	rs = SR_OK, i, len ;
	int	ti, tlen ;
	int	si ;
	int	dlen ;
	int	f_mailchange, f_timechange ;
	int	f_change, f_refresh ;

	char	new_str[DISPLAY_STRLEN + 1] ;
	char	dbuf[DISPLAY_STRLEN + 30] ;
#if	CF_DEBUGS
	char	timebuf1[100], timebuf2[100] ;
#endif


	if (dp == NULL)
		return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("display_show: entered, f_mail=%d dp->f_mail=%d\n",
	    f_mail,dp->f.mail) ;
	debugprintf("display_show: daytime=%s f_blank=%d\n",
	    timestr_log(daytime,timebuf1),dp->f.blank) ;
#endif

	f_mailchange = LXOR(f_mail,dp->f.mail) ;

	f_timechange = (dp->t_last != daytime) ;

#if	CF_DEBUGS
	debugprintf("display_show: f_mailchange=%d f_timechange=%d\n",
	    f_mailchange,f_timechange) ;
#endif

	f_refresh = FALSE ;
	if (((daytime - dp->t_full) > dp->t_refresh) || dp->f.blank) {

#if	CF_DEBUGS
	debugprintf("display_show: full refresh\n") ;
#endif

	    f_timechange = TRUE ;
	    f_mailchange = TRUE ;
	    f_refresh = TRUE ;

	} /* end if (perform a full refresh) */

	f_change = f_mailchange || f_timechange ;

	if ((! f_change) && (! f_refresh)) {

#if	CF_DEBUGS
	    debugprintf("display_show: returning no change\n") ;
#endif

	    goto ret0 ;
	}

	dp->f.blank = FALSE ;

#if	CF_DEBUGS
	debugprintf("display_show: f_timechange=%d f_refresh=%d\n",
	    f_timechange,f_refresh) ;
#endif

	ti = 0 ;
	tlen = 0 ;
	if (f_timechange) {

#if	CF_DEBUGS
	    debugprintf("display_show: timechange code, mailchange=%d\n",
	        f_mailchange) ;
#endif

	    t_display = daytime + dp->t_offset ;
	    (void) ctime_r(&t_display,new_str) ;

	    tlen = CTIMELEN ;
#ifdef	COMMENT
	    new_str[tlen] = '\0' ;
#endif /* COMMENT */

	    if ((! f_mailchange) && (! f_refresh)) {

	        for (ti = 0 ; ti < CTIMELEN ; ti += 1)
	            if (dp->last_str[ti] != new_str[ti]) 
			break ;

	        tlen = CTIMELEN - ti ;

#if	CF_DEBUGS
	        debugprintf("display_show: updating len=%d\n",tlen) ;
	        debugprintf("display_show: old> %s\n",dp->last_str) ;
	        debugprintf("display_show: new> %s",new_str) ;
#endif

	    } /* end if */

	    strwcpy(dp->last_str,new_str,CTIMELEN) ;

	} /* end if (time changed) */

	if (f_mailchange || f_refresh) {

	    si = 59 ;
	    dp->t_full = daytime ;

	} else
	    si = 59 + 3 + CTIMELEN - tlen ;

	dlen = 0 ;
	dlen += bufprintf(dbuf + dlen,DISPLAY_STRLEN,
	    "%s%s%s%s\033[1;%dH",
	    TERMSTR_SAVE,TERMSTR_R_CUR,
	    ((dp->f.status) ? TERMSTR_S_SD : ""),
	    TERMSTR_NORM,
	    si) ;

	if (f_mailchange || f_refresh)
	    dlen += bufprintf(dbuf + dlen,DISPLAY_STRLEN,
		" %s%c%s ",
	        TERMSTR_BLINK,((f_mail) ? 'M' : ' '), 
	        TERMSTR_NORM) ;

	else
	    dlen += bufprintf(dbuf + dlen,DISPLAY_STRLEN,
		"%s",
	        TERMSTR_NORM) ;

	dlen += bufprintf(dbuf + dlen,DISPLAY_STRLEN,
		"%t%s%s%s",
	    (dp->last_str + ti),tlen,
	    ((dp->f.status) ? TERMSTR_R_SD : ""),
	    TERMSTR_S_CUR,TERMSTR_RESTORE) ;

/* update the remaining object state */

#if	CF_DEBUGS
	debugprintf("display_show: mail=%d\n",f_mail) ;
#endif

	dp->t_last = daytime ;
	dp->f.mail = (f_mail) ? TRUE : FALSE ;

/* write it to the actual screen */

#if	CF_DEBUGS
	debugprintf("display_show: old_mail=%d\n",dp->f.mail) ;
	debugprintf("display_show: write() fd=%d\n",dp->fd) ;
#endif

#if	CF_WRITE
	rs = u_write(dp->fd,dbuf,dlen) ;
#else
	rs = 1 ;
#endif

	dp->f.blank = FALSE ;

ret0:

#if	CF_DEBUGS
	debugprintf("display_show: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (display_show) */


/* blank out the display */
int display_blank(dp)
DISPLAY		*dp ;
{
	int	rs ;
	int	i, len ;
	int	ti, tlen ;
	int	si ;
	int	dlen ;

	char	dbuf[DISPLAY_STRLEN + 30] ;


	if (dp == NULL)
		return SR_FAULT ;

	if (dp->magic != DISPLAY_MAGIC)
		return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("display_blank: entered\n") ;
#endif

	si = 59 ;
	tlen = 22 ;

	dlen = 0 ;
	dlen += bufprintf(dbuf + dlen,DISPLAY_STRLEN,
	    "%s%s%s%s\033[1;%dH",
	    TERMSTR_SAVE,TERMSTR_R_CUR,
	    ((dp->f.status) ? TERMSTR_S_SD : ""),
	    TERMSTR_NORM,
	    si) ;

	dlen += bufprintf(dbuf + dlen,DISPLAY_STRLEN,
		"%s",
	    TERMSTR_NORM) ;

	dlen += bufprintf((dbuf + dlen),DISPLAY_STRLEN,
		"%t%s%s%s",
	    blanks,tlen,
	    ((dp->f.status) ? TERMSTR_R_SD : ""),
	    TERMSTR_S_CUR,TERMSTR_RESTORE) ;

/* write it to the actual screen */

#if	CF_DEBUGS
	debugprintf("display_blank: final write\n") ;
#endif

	rs = u_write(dp->fd,dbuf,dlen) ;

#if	CF_DEBUGS
	debugprintf("display_blank: ret rs=%d\n",rs) ;
#endif

	dp->f.blank = TRUE ;
	dp->t_full = 0 ;
	return rs ;
}
/* end subroutine (display_blank) */


int display_check(dp,daytime)
DISPLAY	*dp ;
time_t	daytime ;
{


	if (dp == NULL)
		return SR_FAULT ;

	if (dp->magic != DISPLAY_MAGIC)
		return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("display_check: f_blank=%d\n",dp->f.blank) ;
#endif

	return SR_OK ;
}
/* end subroutine (display_blank) */



/* PRIVATE SUBROUTINES */



