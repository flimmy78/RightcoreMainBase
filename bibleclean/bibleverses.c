/* bibleverses */

/* BIBLEVERSES implementation */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGSTART	0		/* debug |isstart()| */
#define	CF_EMPTYTERM	1		/* terminate entry on empty line */
#define	CF_SAFE		0		/* normal safety */


/* revision history:

	- 2008-10-01, David A�D� Morano

	This object module was originally written.


*/

/* Copyright � 2008 David A�D� Morano.  All rights reserved. */

/*******************************************************************************

	This module implements the (actual) interface to the BIBLEVERSES
	datbase.


*******************************************************************************/


#define	BIBLEVERSES_MASTER	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<limits.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<endianstr.h>
#include	<vecobj.h>
#include	<sbuf.h>
#include	<char.h>
#include	<vecstr.h>
#include	<expcook.h>
#include	<dirseen.h>
#include	<localmisc.h>

#include	"bibleverses.h"
#include	"bvi.h"
#include	"bvimk.h"


/* local defines */

#define	BIBLEVERSES_DBDNAME	"share/bibledbs"
#define	BIBLEVERSES_DBTITLE	"Bible"
#define	BIBLEVERSES_DBSUF	"txt"
#define	BIBLEVERSES_ENT		struct bibleverses_e
#define	BIBLEVERSES_EL		struct bibleverses_eline
#define	BIBLEVERSES_NLE		4	/* default number line entries */
#define	BIBLEVERSES_DIRMODE	0777
#define	BIBLEVERSES_IDXMODE	0664

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#ifndef	VARTMPDNAME
#define	VARTMPDNAME	"TMPDIR"
#endif

#ifndef	TMPDNAME
#define	TMPDNAME	"/tmp"
#endif

#ifndef	TMPVARDNAME
#define	TMPVARDNAME	"/var/tmp"
#endif

#ifndef	VCNAME
#define	VCNAME		"var"
#endif

#define	INDDNAME	"bibleverses"

#define	INDNAME		"bibleverses"
#define	INDSUF		"vi"

#define	SUBINFO		struct subinfo
#define	SUBINFO_FL	struct subinfo_flags

#undef	NLINES
#define	NLINES		20

#define	TO_FILEMOD	(60 * 24 * 3600)
#define	TO_MKWAIT	(5 * 50)
#define	TO_CHECK	4


/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	mkfnamesuf2(char *,const char *,const char *,const char *) ;
extern int	sfskipwhite(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	siskipwhite(const char *,int) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	pathclean(char *,const char *,int) ;
extern int	mkdirs(const char *,mode_t) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	isdigitlatin(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* local structures */

struct subinfo_flags {
	uint		dummy:1 ;
} ;

struct subinfo {
	SUBINFO_FL	f ;
	time_t		dt ;
} ;

struct bibleverses_eline {
	uint		loff ;
	uint		llen ;
} ;

struct bibleverses_e {
	BIBLEVERSES_EL	*lines ;
	uint		voff ;
	uint		vlen ;
	int		e, i ;
	uchar		b, c, v ;
} ;


/* forward references */

static int	bibleverses_dbloadbegin(BIBLEVERSES *,SUBINFO *) ;
static int	bibleverses_dbloadend(BIBLEVERSES *) ;
static int	bibleverses_dbmapcreate(BIBLEVERSES *,time_t) ;
static int	bibleverses_dbmapdestroy(BIBLEVERSES *) ;
static int	bibleverses_checkupdate(BIBLEVERSES *,time_t) ;
static int	bibleverses_loadbuf(BIBLEVERSES *,BVI_VERSE *,char *,int) ;
static int	bibleverses_indopen(BIBLEVERSES *,SUBINFO *) ;

static int	bibleverses_indopencheck(BIBLEVERSES *,const char *) ;

static int	bibleverses_indclose(BIBLEVERSES *) ;
static int	bibleverses_indmk(BIBLEVERSES *,const char *,time_t) ;
static int	bibleverses_indmkdata(BIBLEVERSES *,const char *,mode_t) ;
static int	bibleverses_indopenseq(BIBLEVERSES *,SUBINFO *) ;
static int	bibleverses_indopenseqer(BIBLEVERSES *,SUBINFO *,
			DIRSEEN *,vecstr *,EXPCOOK *) ;
static int	bibleverses_loadcooks(BIBLEVERSES *,EXPCOOK *) ;
static int	bibleverses_indopenalt(BIBLEVERSES *,SUBINFO *,DIRSEEN *) ;

static int	subinfo_start(SUBINFO *) ;
static int	subinfo_finish(SUBINFO *) ;

static int	entry_start(BIBLEVERSES_ENT *,BIBLEVERSES_Q *,uint,uint) ;
static int	entry_add(BIBLEVERSES_ENT *,uint,uint) ;
static int	entry_finish(BIBLEVERSES_ENT *) ;

static int	bvemk_start(BVIMK_VERSE *,BIBLEVERSES_ENT *) ;
static int	bvemk_finish(BVIMK_VERSE *) ;

static int	mkdname(cchar *,mode_t) ;
static int	checkdname(const char *) ;

static int	isempty(const char *,int) ;
static int	isstart(const char *,int,BIBLEVERSES_Q *,int *) ;
static int	istermrs(int) ;


/* exported variables */

BIBLEVERSES_OBJ	bibleverses = {
	"bibleverses",
	sizeof(BIBLEVERSES),
	sizeof(BIBLEVERSES_CUR)
} ;


/* local variables */

static const char	*idxdirs[] = {
	"%R/var/%S",
	"/var/tmp/%{PRN}",
	"/var/tmp",
	"/tmp/%{PRN}",
	"/tmp",
	"%T",
	NULL
} ;

static const int	termrs[] = {
	SR_FAULT,
	SR_INVALID,
	SR_NOMEM,
	SR_NOANODE,
	SR_BADFMT,
	SR_NOSPC,
	SR_NOSR,
	SR_NOBUFS,
	SR_BADF,
	SR_OVERFLOW,
	SR_RANGE,
	0
} ;


/* exported subroutines */


int bibleverses_open(BIBLEVERSES *op,cchar pr[],cchar dbname[])
{
	SUBINFO		si ;
	int		rs ;
	int		nverses = 0 ;
	const char	*suf = BIBLEVERSES_DBSUF ;
	char		cname[MAXNAMELEN + 1] ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif

	if (pr == NULL) return SR_FAULT ;

	if (pr[0] == '\0') return SR_INVALID ;

	if ((dbname == NULL) || (dbname[0] == '\0'))
	    dbname = BIBLEVERSES_DBNAME ;

	memset(op,0,sizeof(BIBLEVERSES)) ;
	op->pr = pr ;
	op->dbname = dbname ;

	if ((rs = snsds(cname,MAXNAMELEN,dbname,suf)) >= 0) {
	    cchar	*dn = BIBLEVERSES_DBDNAME ;
	    char	dbfname[MAXPATHLEN + 1] ;
	    if ((rs = mkpath3(dbfname,pr,dn,cname)) >= 0) {
	        const char	*cp = NULL ;
	        if ((rs = uc_mallocstrw(dbfname,-1,&cp)) >= 0) {
	            op->dbfname = cp ;
	            if ((rs = subinfo_start(&si)) >= 0) {
	                if ((rs = bibleverses_dbloadbegin(op,&si)) >= 0) {
	                    nverses = op->nverses ;
	                    op->magic = BIBLEVERSES_MAGIC ;
	                }
	                subinfo_finish(&si) ;
	            } /* end if (subinfo) */
	            if (rs < 0) {
	                uc_free(op->dbfname) ;
	                op->dbfname = NULL ;
	            }
	        } /* end if (m-a) */
	    } /* end if (mkpath) */
	} /* end if (mkpath) */

	return (rs >= 0) ? nverses : rs ;
}
/* end subroutine (bibleverses_open) */


int bibleverses_count(BIBLEVERSES *op)
{
	int		rs ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != BIBLEVERSES_MAGIC) return SR_NOTOPEN ;
#endif

	rs = op->nverses ;

	return rs ;
}
/* end subroutine (bibleverses_count) */


int bibleverses_audit(BIBLEVERSES *op)
{
	int		rs = SR_OK ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != BIBLEVERSES_MAGIC) return SR_NOTOPEN ;
#endif

	if (op->f.vind) {
	    rs = bvi_audit(&op->vind) ;
	}

	return rs ;
}
/* end subroutine (bibleverses_audit) */


/* free up the entire vector string data structure object */
int bibleverses_close(BIBLEVERSES *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != BIBLEVERSES_MAGIC) return SR_NOTOPEN ;
#endif

#if	CF_DEBUGS
	if (op->f.vind) {
	    rs1 = bvi_audit(&op->vind) ;
	    if (rs >= 0) rs = rs1 ;
	}
#endif

#if	CF_DEBUGS
	debugprintf("bibleverses_close: 1 rs=%d\n",rs) ;
#endif

	rs1 = bibleverses_dbloadend(op) ;
	if (rs >= 0) rs = rs1 ;

	if (op->dbfname != NULL) {
	    rs1 = uc_free(op->dbfname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->dbfname = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("bibleverses_close: ret rs=%d\n",rs) ;
#endif

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (bibleverses_close) */


/* get a string by its index */
int bibleverses_get(BIBLEVERSES *op,BIBLEVERSES_Q *qp,char vbuf[],int vlen)
{
	BVI_QUERY	viq ;
	BVI_VERSE	viv ;
	BVI_LINE	lines[NLINES + 1] ;
	time_t		dt = 0 ;
	int		rs = SR_OK ;
	int		linesize = ((NLINES + 1) * sizeof(BVI_LINE)) ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != BIBLEVERSES_MAGIC) return SR_NOTOPEN ;
#endif

	if (qp == NULL) return SR_FAULT ;

#ifdef	COMMENT
	if (buf == NULL)
	    return SR_FAULT ;
#endif

/* check for update */

	if (op->ncursors == 0) {
	    rs = bibleverses_checkupdate(op,dt) ;
	}

	if (rs >= 0) {
	    viq.b = qp->b ;
	    viq.c = qp->c ;
	    viq.v = qp->v ;
	    rs = bvi_get(&op->vind,&viq,&viv,(char *) lines,linesize) ;
	    if ((rs >= 0) && (vbuf != NULL)) {
	        rs = bibleverses_loadbuf(op,&viv,vbuf,vlen) ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("bibleverses_get: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bibleverses_get) */


int bibleverses_curbegin(BIBLEVERSES *op,BIBLEVERSES_CUR *curp)
{
	int		rs ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != BIBLEVERSES_MAGIC) return SR_NOTOPEN ;
#endif

	if (curp == NULL) return SR_FAULT ;

	if ((rs = bvi_curbegin(&op->vind,&curp->vicur)) >= 0) {
	    op->ncursors += 1 ;
	}

	return rs ;
}
/* end subroutine (bibleverses_curbegin) */


int bibleverses_curend(BIBLEVERSES *op,BIBLEVERSES_CUR *curp)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != BIBLEVERSES_MAGIC) return SR_NOTOPEN ;
#endif

	if (curp == NULL) return SR_FAULT ;

	rs1 = bvi_curend(&op->vind,&curp->vicur) ;
	if (rs >= 0) rs = rs1 ;

	if (op->ncursors > 0)
	    op->ncursors -= 1 ;

	return rs ;
}
/* end subroutine (bibleverses_curend) */


int bibleverses_enum(op,curp,qp,vbuf,vlen)
BIBLEVERSES	*op ;
BIBLEVERSES_CUR	*curp ;
BIBLEVERSES_Q	*qp ;
char		vbuf[] ;
int		vlen ;
{
	time_t		dt = 0 ;
	int		rs = SR_OK ;
	int		ls = ((NLINES + 1) * sizeof(BVI_LINE)) ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != BIBLEVERSES_MAGIC) return SR_NOTOPEN ;
#endif

	if (curp == NULL) return SR_FAULT ;
	if (qp == NULL) return SR_FAULT ;

#ifdef	COMMENT
	if (vbuf == NULL) return SR_FAULT ;
#endif

/* check for update */

	if (op->ncursors == 0) {
	    rs = bibleverses_checkupdate(op,dt) ;
	}

	if (rs >= 0) {
	    BVI_CUR	*bcurp = &curp->vicur ;
	    BVI_VERSE	viv ;
	    BVI_LINE	lines[NLINES + 1] ;
	    if ((rs = bvi_enum(&op->vind,bcurp,&viv,(char *) lines,ls)) >= 0) {
	        if (vbuf != NULL)
	            rs = bibleverses_loadbuf(op,&viv,vbuf,vlen) ;
	        if ((rs >= 0) && (qp != NULL)) {
		    qp->b = viv.b ;
		    qp->c = viv.c ;
		    qp->v = viv.v ;
	        }
	    } /* end if (bvi_enum) */
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("bibleverses_enum: ret rs=%d\n", rs) ;
#endif

	return rs ;
}
/* end subroutine (bibleverses_enum) */


int bibleverses_info(BIBLEVERSES *op,BIBLEVERSES_INFO *ip)
{
	BVI_INFO	bi ;
	int		rs ;
	int		nverses = 0 ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != BIBLEVERSES_MAGIC) return SR_NOTOPEN ;
#endif

	if ((rs = bvi_info(&op->vind,&bi)) >= 0) {
	    nverses = bi.count ;
	    if (ip != NULL) {
	        memset(ip,0,sizeof(BIBLEVERSES_INFO)) ;
	        ip->dbtime = op->ti_db ;
	        ip->vitime = op->ti_vind ;
	        ip->maxbook = bi.maxbook ;
	        ip->maxchapter = bi.maxchapter ;
	        ip->nverses = bi.count ;
	        ip->nzverses = bi.nzverses ;
	    }
	} /* end if (bvi_info) */

	return (rs >= 0) ? nverses : rs ;
}
/* end subroutine (bibleverses_info) */


/* retrieve a table w/ number of verses for each chapter of a book */
int bibleverses_chapters(BIBLEVERSES *op,int book,uchar *ap,int al)
{
	int		rs = SR_OK ;
	int		n = 0 ;

#if	CF_DEBUGS
	debugprintf("bibleverses_chapters: ent\n") ;
#endif

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != BIBLEVERSES_MAGIC) return SR_NOTOPEN ;
#endif

	if (book < 0)
	    return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("bibleverses_chapters: b=%u al=%u\n",book,al) ;
#endif

	if (ap != NULL) {
	    if (op->ncursors == 0) {
	        rs = bibleverses_checkupdate(op,0L) ;
	    }
	    if (rs >= 0) {
	       rs = bvi_chapters(&op->vind,book,ap,al) ;
	       n = rs ;
	    } /* end if */
	} /* end if (non-null) */

#if	CF_DEBUGS
	debugprintf("bibleverses_chapters: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (bibleverses_chapters) */


/* private subroutines */


static int bibleverses_dbloadbegin(BIBLEVERSES *op,SUBINFO *sip)
{
	int		rs ;

#if	CF_DEBUGS
	debugprintf("bibleverses_dbloadbegin: ent\n") ;
#endif

	if ((rs = bibleverses_dbmapcreate(op,sip->dt)) >= 0) {
	    rs = bibleverses_indopen(op,sip) ;
	    if (rs < 0)
	        bibleverses_dbmapdestroy(op) ;
	}

#if	CF_DEBUGS
	debugprintf("bibleverses_dbloadbegin: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bibleverses_dbloadbegin) */


static int bibleverses_dbloadend(BIBLEVERSES *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = bibleverses_indclose(op) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("bibleverses_dbloadend: _indclose() rs=%d\n",rs) ;
#endif

	rs1 = bibleverses_dbmapdestroy(op) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("bibleverses_dbloadend: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bibleverses_dbloadend) */


static int bibleverses_dbmapcreate(BIBLEVERSES *op,time_t dt)
{
	int		rs ;

#if	CF_DEBUGS
	debugprintf("bibleverses_dbmapcreate: dbfname=%s\n",op->dbfname) ;
#endif

	if ((rs = u_open(op->dbfname,O_RDONLY,0666)) >= 0) {
	    struct ustat	sb ;
	    const int		fd = rs ;
	    if ((rs = u_fstat(fd,&sb)) >= 0) {
		if (S_ISREG(sb.st_mode)) {
		    if ((sb.st_size <= INT_MAX) && (sb.st_size > 0)) {
	    	        size_t	ms = (size_t) (sb.st_size & UINT_MAX) ;
			int	mp = PROT_READ ;
	    	        int	mf = MAP_SHARED ;
	    	        void	*md ;
			op->filesize = ms ;
			op->ti_db = sb.st_mtime ;
#if	CF_DEBUGS
	debugprintf("bibleverses_dbmapcreate: ms=%ld\n",ms) ;
#endif
	    		if ((rs = u_mmap(NULL,ms,mp,mf,fd,0L,&md)) >= 0) {
			    const int		madv = MADV_RANDOM ;
			    const caddr_t	ma = md ;
	        	    if ((rs = uc_madvise(ma,ms,madv)) >= 0) {
	            	        op->mapdata = md ;
	            	        op->mapsize = ms ;
	            	        op->ti_map = dt ;
	            	        op->ti_lastcheck = dt ;
			    }
#if	CF_DEBUGS
	debugprintf("bibleverses_dbmapcreate: uc_madvise() rs=%d\n",rs) ;
#endif
	                    if (rs < 0) {
		                u_munmap(md,ms) ;
	                        op->mapdata = NULL ;
	                        op->mapsize = 0 ;
		            }
	                } /* end if (u_mmap) */
#if	CF_DEBUGS
	debugprintf("bibleverses_dbmapcreate: u_mmap-out rs=%d\n",rs) ;
#endif
	            } else
	                rs = SR_TOOBIG ;
	        } else
	            rs = SR_NOTSUP ;
	    } /* end if (u_fstat) */
	    u_close(fd) ;
	} /* end if (file) */

#if	CF_DEBUGS
	debugprintf("bibleverses_dbmapcreate: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bibleverses_dbmapcreate) */


static int bibleverses_dbmapdestroy(BIBLEVERSES *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->mapdata != NULL) {
	    rs1 = u_munmap(op->mapdata,op->mapsize) ;
	    if (rs >= 0) rs = rs1 ;
	    op->mapdata = NULL ;
	    op->mapsize = 0 ;
	}

#if	CF_DEBUGS
	debugprintf("bibleverses_dbmapdestroy: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bibleverses_dbmapdestroy) */


static int bibleverses_checkupdate(BIBLEVERSES *op,time_t dt)
{
	SUBINFO		si ;
	struct ustat	sb ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		f = FALSE ;

	if (op->ncursors > 0)
	    goto ret0 ;

	if (dt <= 0)
	    dt = time(NULL) ;

	if ((dt - op->ti_lastcheck) < TO_CHECK)
	    goto ret0 ;

	op->ti_lastcheck = dt ;
	if ((rs1 = u_stat(op->dbfname,&sb)) >= 0) {
	if ((sb.st_mtime > op->ti_db) || (sb.st_mtime > op->ti_map)) {

	    f = TRUE ;
	    bibleverses_dbloadend(op) ;

	    if ((rs = subinfo_start(&si)) >= 0) {

	        rs = bibleverses_dbloadbegin(op,&si) ;

		subinfo_finish(&si) ;
	    } /* end if */

	} /* end if (update) */
	} /* end if (stat) */

ret0:
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (bibleverses_checkupdate) */


static int bibleverses_loadbuf(BIBLEVERSES *op,BVI_VERSE *vivp,
		char *rbuf,int rlen)
{
	BVI_LINE	*lines ;
	SBUF		b ;
	int		rs ;
	int		i ;
	int		nlines ;
	int		ll ;
	int		len = 0 ;
	const char	*lp ;

#if	CF_DEBUGS
	debugprintf("bibleverses_loadbuf: voff=%u vlen=%u\n",
		vivp->voff,vivp->vlen) ;
#endif

	lines = vivp->lines ;
	nlines = vivp->nlines ;

	if ((rs = sbuf_start(&b,rbuf,rlen)) >= 0) {

	    for (i = 0 ; i < nlines ; i += 1) {

	        if (i > 0)
		    sbuf_char(&b,' ') ;

	        lp = (op->mapdata + lines[i].loff) ;
	        ll = lines[i].llen ;
	        rs = sbuf_strw(&b,lp,ll) ;

	        if (rs < 0) break ;
	    } /* end for */

	    len = sbuf_finish(&b) ;
	    if (rs >= 0) rs = len ;
	} /* end if (sbuf) */

	return (rs >= 0) ? len : rs ;
} 
/* end subroutine (bibleverses_loadbuf) */


static int bibleverses_indopen(BIBLEVERSES *op,SUBINFO *sip)
{
	int		rs ;

#if	CF_DEBUGS
	debugprintf("bibleverses_indopen: ent\n") ;
#endif

	if ((rs = bibleverses_indopenseq(op,sip)) >= 0) {
	    if (op->f.vind) {
	        rs = bvi_count(&op->vind) ;
	        op->nverses = rs ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("bibleverses_indopen: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bibleverses_indopen) */


static int bibleverses_indopenseq(BIBLEVERSES *op,SUBINFO *sip)
{
	DIRSEEN		ds ;
	int		rs ;
	int		rs1 ;

	if ((rs = dirseen_start(&ds)) >= 0) {
	    vecstr	spnames, *spp = &spnames ;
	    const int	vopts = VECSTR_OCOMPACT ;
	    if ((rs = vecstr_start(spp,6,vopts)) >= 0) {
		EXPCOOK		cooks, *ckp = &cooks ;
		if ((rs = expcook_start(ckp)) >= 0) {
		    if ((rs = bibleverses_loadcooks(op,ckp)) >= 0) {
			rs = bibleverses_indopenseqer(op,sip,&ds,spp,ckp) ;
		    }
		    rs1 = expcook_finish(ckp) ;
	            if (rs >= 0) rs = rs1 ;
		} /* end if (cooks) */
		rs1 = vecstr_finish(spp) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (spnames) */
	    rs1 = dirseen_finish(&ds) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ds) */

	return rs ;
}
/* end subroutines (bibleverses_indopenseq) */


static int bibleverses_indopenseqer(BIBLEVERSES *op,SUBINFO *sip,
		DIRSEEN *dsp,vecstr *spp,EXPCOOK *ckp)
{
	const int	elen = MAXPATHLEN ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		plen ;
	int		el ;
	int		i ;
	char		ebuf[MAXPATHLEN + 1] ;
	char		pbuf[MAXPATHLEN + 1] ;

/* first phase: expand possible directory paths */

	for (i = 0 ; (rs >= 0) && (idxdirs[i] != NULL) ; i += 1) {

	    rs = expcook_exp(ckp,'\0',ebuf,elen,idxdirs[i],-1) ;
	    el = rs ;
	    if ((rs >= 0) && (el > 0)) {
		rs = pathclean(pbuf,ebuf,el) ;
		plen = rs ;
		if ((rs >= 0) && (plen > 0)) {
		    rs1 = dirseen_havename(dsp,pbuf,plen) ;
		    if (rs1 == SR_NOTFOUND) {
			rs = dirseen_add(dsp,pbuf,plen,NULL) ;
		    }
		}
	    }

	} /* end for */

/* next phase: create DB file-paths from directories */

	if (rs >= 0) {
	    DIRSEEN_CUR	cur ;
	    if ((rs = dirseen_curbegin(dsp,&cur)) >= 0) {

	        while ((el = dirseen_enum(dsp,&cur,ebuf,elen)) >= 0) {
	            if (el == 0) continue ;

		    if ((rs = mkpath2(pbuf,ebuf,op->dbname)) >= 0) {
		        plen = rs ;
		        rs = vecstr_add(spp,pbuf,plen) ;
		    }

		    if (rs < 0) break ;
	        } /* end while */

	        dirseen_curend(dsp,&cur) ;
	    } /* end if (cursor) */
	} /* end if (ok) */

/* final phase: try to open one of them trying them in-sequence */

	if (rs >= 0) {
	    cchar	**dv ;
	    if ((rs = vecstr_getvec(spp,&dv)) >= 0) {
		for (i = 0 ; dv[i] != NULL ; i += 1) {
		    rs = bibleverses_indopencheck(op,dv[i]) ;

#if	CF_DEBUGS
	debugprintf("bibleverses_indopenseq: _indopencheck() rs=%d\n",rs) ;
	debugprintf("bibleverses_indopenseq: dv[%u]=%s\n",i,dv[i]) ;
#endif

		    if ((rs >= 0) || istermrs(rs)) break ;
		} /* end for */
	    } /* end if */

	    if ((rs < 0) && (! istermrs(rs))) {

	        rs = bibleverses_indopenalt(op,sip,dsp) ;

#if	CF_DEBUGS
	debugprintf("bibleverses_indopenseq: _indopenalt() rs=%d\n",rs) ;
#endif

	    }

	} /* end if */

	return rs ;
}
/* end subroutines (bibleverses_indopenseqer) */


static int bibleverses_loadcooks(BIBLEVERSES *op,EXPCOOK *ecp)
{
	int		rs = SR_OK ;
	    const int	hlen = MAXHOSTNAMELEN ;
	    int		i ;
	    int		kch ;
	    int		vl ;
	cchar		*tmpdname = getenv(VARTMPDNAME) ;
	    cchar	*ks = "RST" ;
	    cchar	*vp ;
	    char	hbuf[MAXHOSTNAMELEN+1] ;
	    char	kbuf[2] ;

	if (tmpdname == NULL) tmpdname = TMPDNAME ;

	    kbuf[1] = '\0' ;
	    for (i = 0 ; (rs >= 0) && (ks[i] != '\0') ; i += 1) {
	        kch = MKCHAR(ks[i]) ;
	        vp = NULL ;
	        vl = -1 ;
	        switch (kch) {
	        case 'R':
	            vp = op->pr ;
	            break ;
	        case 'S':
	            vp = INDDNAME ;
	            break ;
	        case 'T':
	            vp = tmpdname ;
	            break ;
	        } /* end switch */
	        if ((rs >= 0) && (vp != NULL)) {
	            kbuf[0] = kch ;
	            rs = expcook_add(ecp,kbuf,vp,vl) ;
	        }
	    } /* end for */

	if (rs >= 0) {
	    cchar	*prname ;
	    if ((rs = sfbasename(op->pr,-1,&prname)) >= 0) {
	        rs = SR_NOENT ;
	        if (prname != NULL) {
	            rs = expcook_add(ecp,"PRN",prname,-1) ;
		}
	    }
	}

	return rs ;
}
/* end subroutines (bibleverses_loadcooks) */


static int bibleverses_indopenalt(BIBLEVERSES *op,SUBINFO *sip,DIRSEEN *dsp)
{
	DIRSEEN_CUR	cur ;
	int		rs ;
	int		rs1 ;

	if ((rs = dirseen_curbegin(dsp,&cur)) >= 0) {
	    const int	elen = MAXPATHLEN ;
	    int		el ;
	    char	ebuf[MAXPATHLEN + 1] ;
	    char	indname[MAXPATHLEN + 1] ;
	    while ((el = dirseen_enum(dsp,&cur,ebuf,elen)) >= 0) {
	        if (el > 0) {
	            if ((rs = bibleverses_indmk(op,ebuf,sip->dt)) >= 0) {
		        cchar	*db = op->dbname ;
		        if ((rs = mkpath2(indname,ebuf,db)) >= 0) {
	                    if ((rs = bvi_open(&op->vind,indname)) >= 0) {
	                        op->f.vind = TRUE ;
			    }
		        }
	            }
	        }
	        if ((rs >= 0) || istermrs(rs)) break ;
	    } /* end while */
	    rs1 = dirseen_curend(dsp,&cur) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (cursor) */

	return rs ;
}
/* end subroutines (bibleverses_indopenalt) */


static int bibleverses_indopencheck(BIBLEVERSES *op,cchar dbname[])
{
	int		rs ;
	int		nverses = 0 ;

#if	CF_DEBUGS
	debugprintf("bibleverses_indopencheck: ent dbname=%s\n",dbname) ;
#endif

	if ((rs = bvi_open(&op->vind,dbname)) >= 0) {
	    BVI_INFO	binfo ;
	    nverses = rs ;

#if	CF_DEBUGS
	debugprintf("bibleverses_indopencheck: bvi_open() rs=%d\n",rs) ;
#endif

	    if ((rs = bvi_info(&op->vind,&binfo)) >= 0) {
		if (binfo.ctime < op->ti_db) {
		    rs = SR_STALE ;
		} else {
		    op->f.vind = TRUE ;
		}
	        if (rs < 0)
	             bvi_close(&op->vind) ;
	    } /* end if (bvi_open) */
	} /* end if (bvi_open) */

#if	CF_DEBUGS
	debugprintf("bibleverses_indopencheck: ret rs=%d nv=%u\n",
		rs,nverses) ;
#endif

	return (rs >= 0) ? nverses : rs ;
}
/* end subroutine (bibleverses_indopencheck) */


static int bibleverses_indmk(BIBLEVERSES *op,cchar dname[],time_t dt)
{
	const mode_t	dm = BIBLEVERSES_DIRMODE ;
	int		rs ;
	int		c = 0 ;

	if ((rs = mkdname(dname,dm)) >= 0) {
	    char	indname[MAXPATHLEN + 1] ;
	    if ((rs = mkpath2(indname,dname,op->dbname)) >= 0) {
		const mode_t	om = BIBLEVERSES_IDXMODE ;
	        if ((rs = bibleverses_indmkdata(op,indname,om)) >= 0) {
	            c += rs ;
	            op->ti_vind = dt ;
	        }
	    } /* end if (mkpath) */
	} /* end if (mkdname) */

#if	CF_DEBUGS
	debugprintf("bibleverses_indmk: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (bibleverses_indmk) */


static int bibleverses_indmkdata(BIBLEVERSES *op,cchar indname[],mode_t om)
{
	BVIMK		bvind ;
	BVIMK_VERSE	bve ;
	int		rs ;
	int		rs1 ;
	int		of = 0 ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("bibleverses_indmkdata: ent indname=%s\n",indname) ;
#endif

	if ((rs = bvimk_open(&bvind,indname,of,om)) >= 0) {
	    c = rs ;
	    if (rs == 0) {
		BIBLEVERSES_ENT	e ;
		BIBLEVERSES_Q	q ;
		int		foff = 0 ;
		int		len ;
		int		ml = (op->filesize & INT_MAX) ;
		int		ll ;
		int		si ;
		int		f_ent = FALSE ;
		cchar		*mp = op->mapdata ;
		cchar		*tp, *lp ;

	while ((tp = strnchr(mp,ml,'\n')) != NULL) {

	    len = ((tp + 1) - mp) ;
	    lp = mp ;
	    ll = (len - 1) ;

	    if ((ll > 0) && (! isempty(lp,ll))) {

#if	CF_DEBUGS
	debugprintf("bibleverses_indmkdata: line>%t<\n",
		lp,strnlen(lp,MIN(ll,40))) ;
#endif

	        if ((tp = strnchr(lp,ll,'#')) != NULL)
	            ll = (tp - lp) ;

	        if ((rs = isstart(lp,ll,&q,&si)) > 0) {

	            if (f_ent) {
	                c += 1 ;
	                if ((rs = bvemk_start(&bve,&e)) >= 0) {
	                    rs = bvimk_add(&bvind,&bve) ;
			    bvemk_finish(&bve) ;
		        }
			f_ent = FALSE ;
	                entry_finish(&e) ;
	            }

	            if (rs >= 0) {
	                rs = entry_start(&e,&q,(foff + si),(ll - si)) ;
			if (rs >= 0) {
	                    f_ent = TRUE ;
			}
		    }

	        } else if (rs >= 0) {

		    if (f_ent)
	                rs = entry_add(&e,foff,ll) ;

	        } /* end if (entry start of add) */

	    } else {

#if	CF_EMPTYTERM
	        if (f_ent) {
	            c += 1 ;
	            if ((rs = bvemk_start(&bve,&e)) >= 0) {
	                rs = bvimk_add(&bvind,&bve) ;
		        bvemk_finish(&bve) ;
		    }
	            f_ent = FALSE ;
	            entry_finish(&e) ;
	        }
#else
		rs = SR_OK ;
#endif /* CF_EMPTYTERM */

	    } /* end if (not empty) */

	    foff += len ;
	    ml -= len ;
	    mp += len ;

	    if (rs < 0) break ;
	} /* end while (readling lines) */

	if ((rs >= 0) && f_ent) {
	    c += 1 ;
	    if ((rs = bvemk_start(&bve,&e)) >= 0) {
	        rs = bvimk_add(&bvind,&bve) ;
		bvemk_finish(&bve) ;
	    }
	    f_ent = FALSE ;
	    entry_finish(&e) ;
	}

	if (f_ent) {
	    f_ent = FALSE ;
	    entry_finish(&e) ;
	}

#if	CF_DEBUGS
	{
	    BVIMK_INFO	bi ;
	    rs1 = bvimk_info(&bvind,&bi) ;
	    debugprintf("bibleverses_indmkdata: maxbook=%u\n",
		bi.maxbook) ;
	    debugprintf("bibleverses_indmkdata: maxchapter=%u\n",
		bi.maxchapter) ;
	    debugprintf("bibleverses_indmkdata: maxverse=%u\n",
		bi.maxverse) ;
	    debugprintf("bibleverses_indmkdata: nverses=%u\n",
		bi.nverses) ;
	    debugprintf("bibleverses_indmkdata: nzverses=%u\n",
		bi.nzverses) ;
	}
#endif /* CF_DEBUGS */

	    } /* end if (creation needed) */
	    rs1 = bvimk_close(&bvind) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (bvimk) */

#if	CF_DEBUGS
	debugprintf("bibleverses_indmkdata: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (bibleverses_indmkdata) */


static int bibleverses_indclose(BIBLEVERSES *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->f.vind) {
	    op->f.vind = FALSE ;
	    rs1 = bvi_close(&op->vind) ;
	    if (rs >= 0) rs = rs1 ;
	}

#if	CF_DEBUGS
	debugprintf("bibleverses_indclose: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bibleverses_indclose) */


static int subinfo_start(SUBINFO *sip)
{
	int		rs = SR_OK ;

	memset(sip,0,sizeof(SUBINFO)) ;
	sip->dt = time(NULL) ;

	return rs ;
}
/* end subroutine (subinfo_start) */


static int subinfo_finish(SUBINFO *sip)
{
	if (sip == NULL) return SR_FAULT ;
	return SR_OK ;
}
/* end subroutine (subinfo_finish) */


static int entry_start(ep,qp,loff,llen)
BIBLEVERSES_ENT	*ep ;
BIBLEVERSES_Q	*qp ;
uint		loff, llen ;
{
	const int	ne = BIBLEVERSES_NLE ;
	int		rs = SR_OK ;
	int		size ;
	void		*p ;

	if (ep == NULL) return SR_FAULT ;

	memset(ep,0,sizeof(BIBLEVERSES_ENT)) ;
	ep->b = qp->b ;
	ep->c = qp->c ;
	ep->v = qp->v ;
	ep->voff = loff ;
	ep->vlen = llen ;

	size = ne * sizeof(BIBLEVERSES_EL) ;
	if ((rs = uc_malloc(size,&p)) >= 0) {
	    BIBLEVERSES_EL *elp = p ;
	    ep->lines = p ;
	    ep->e = ne ;
	    ep->i += 1 ;
	    elp->loff = loff ;
	    elp->llen = llen ;
	}

	return rs ;
}
/* end subroutine (entry_start) */


static int entry_add(ep,loff,llen)
BIBLEVERSES_ENT	*ep ;
uint		loff, llen ;
{
	BIBLEVERSES_EL	*elp ;
	int		rs = SR_OK ;
	int		ne ;
	int		size ;

	if (ep == NULL) return SR_FAULT ;

	if (ep->e <= 0) return SR_NOTOPEN ;

	if ((ep->i < 0) || (ep->i > ep->e)) return SR_BADFMT ;

	if (ep->i == ep->e) {
	    ne = ep->e + BIBLEVERSES_NLE ;
	    size = (ne * sizeof(BIBLEVERSES_EL)) ;
	    if ((rs = uc_realloc(ep->lines,size,&elp)) >= 0) {
	        ep->e = ne ;
	        ep->lines = elp ;
	    }
	}

	if (rs >= 0) {
	    ep->vlen = ((loff + llen) - ep->voff) ;
	    elp = (ep->lines + ep->i) ;
	    elp->loff = loff ;
	    elp->llen = llen ;
	    ep->i += 1 ;
	}

	return rs ;
}
/* end subroutine (entry_add) */


static int entry_finish(ep)
BIBLEVERSES_ENT	*ep ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (ep == NULL) return SR_FAULT ;

	if (ep->e <= 0) return SR_NOTOPEN ;

	if ((ep->i < 0) || (ep->i > ep->e)) return SR_BADFMT ;

	if (ep->lines != NULL) {
	    rs1 = uc_free(ep->lines) ;
	    if (rs >= 0) rs = rs1 ;
	    ep->lines = NULL ;
	}

	ep->i = 0 ;
	ep->e = 0 ;
	return rs ;
}
/* end subroutine (entry_finish) */


static int bvemk_start(bvep,ep)
BVIMK_VERSE	*bvep ;
BIBLEVERSES_ENT	*ep ;
{
	uint		nlines = 0 ;
	int		rs = SR_OK ;

	if (ep == NULL) return SR_FAULT ;

	bvep->b = ep->b ;
	bvep->c = ep->c ;
	bvep->v = ep->v ;
	bvep->voff = ep->voff ;
	bvep->vlen = ep->vlen ;
	bvep->lines = NULL ;

	nlines = ep->i ;
	if (nlines <= UCHAR_MAX) {
	    BVIMK_LINE	*lines ;
	    int		size ;
	    bvep->nlines = nlines ;
	    size = (nlines + 1) * sizeof(BVIMK_LINE) ;
	    if ((rs = uc_malloc(size,&lines)) >= 0) {
		int	i ;
		bvep->lines = lines ;
	        for (i = 0 ; i < nlines ; i += 1) {
		    lines[i].loff = ep->lines[i].loff ;
		    lines[i].llen = ep->lines[i].llen ;
	        }
		lines[i].loff = 0 ;
		lines[i].llen = 0 ;
	    } /* end if (memory-allocation) */
	} else
	    rs = SR_TOOBIG ;

	return (rs >= 0) ? nlines : rs ;
}
/* end subroutine (bvemk_start) */


static int bvemk_finish(bvep)
BVIMK_VERSE	*bvep ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (bvep == NULL) return SR_FAULT ;

	if (bvep->lines != NULL) {
	    rs1 = uc_free(bvep->lines) ;
	    if (rs >= 0) rs = rs1 ;
	    bvep->lines = NULL ;
	}

	return rs ;
}
/* end subroutine (bvemk_finish) */


static int mkdname(cchar *dname,mode_t dm)
{
	const int	nrs = SR_NOENT ;
	int		rs ;
	if ((rs = checkdname(dname)) == nrs) {
	    rs = mkdirs(dname,dm) ;
	}
	return rs ;
}
/* end subroutine (mkdname) */


static int checkdname(cchar *dname)
{
	int		rs = SR_OK ;

	if (dname[0] == '/') {
	    struct ustat	sb ;
	    if ((rs = u_stat(dname,&sb)) >= 0) {
		if (! S_ISDIR(sb.st_mode)) rs = SR_NOTDIR ;
		if (rs >= 0) {
		    rs = perm(dname,-1,-1,NULL,W_OK) ;
		}
	    } /* end if (stat) */
	} else
	    rs = SR_INVALID ;

	return rs ;
}
/* end subroutine (checkdname) */


static int isempty(cchar *lp,int ll)
{
	int		cl ;
	int		f = FALSE ;
	const char	*cp ;

	f = f || (ll == 0) ;
	f = f || (lp[0] == '#') ;
	if ((! f) && CHAR_ISWHITE(*lp)) {
	    cl = sfskipwhite(lp,ll,&cp) ;
	    f = f || (cl == 0) ;
	    f = f || (cp[0] == '#') ;
	}

	return f ;
}
/* end subroutine (isempty) */


static int isstart(lp,ll,qp,sip)
const char	*lp ;
int		ll ;
BIBLEVERSES_Q	*qp ;
int		*sip ;
{
	int		rs1 ;
	int		sl = ll ;
	int		ch ;
	int		si = 0 ;
	int		f = FALSE ;
	const char	*sp = lp ;

#if	CF_DEBUGS && CF_DEBUGSTART
	    debugprintf("bibleqs/isstart: ent l=>%t<\n",lp,
		strlinelen(lp,ll,40)) ;
#endif

	if (CHAR_ISWHITE(sp[0]) && ((si = siskipwhite(lp,ll)) > 0)) {
	    sp += si ;
	    sl -= si ;
	}

	ch = MKCHAR(sp[0]) ;
	if ((sl >= 5) && isdigitlatin(ch)) {
	    int		i, v ;
	    int		cl ;
	    const char	*tp, *cp ;

	    for (i = 0 ; i < 3 ; i += 1) {

	    cp = sp ;
	    cl = sl ;
	    if ((tp = strnpbrk(sp,sl,": \t\n")) != NULL) {
	        cl = (tp - sp) ;
	        sl -= ((tp + 1) - sp) ;
	        sp = (tp + 1) ;
	    } else {
	        cl = sl ;
	        sp += sl ;
	        sl = 0 ;
	    }

	    if (cl == 0)
	        break ;

	    si = ((cp + cl) - lp) ;
	    rs1 = cfdeci(cp,cl,&v) ;

#if	CF_DEBUGS && CF_DEBUGSTART
	    debugprintf("bibleqs/isstart: cfdeci() rs=%d\n",rs1) ;
#endif

	    if (rs1 < 0)
	        break ;

	    switch (i) {
	    case 0:
	        qp->b = v ;
	        break ;
	    case 1:
	        qp->c = v ;
	        break ;
	    case 2:
	        qp->v = v ;
	        break ;
	    } /* end switch */

	} /* end for */

	f = (i == 3) ;
	if (f)
	    si += siskipwhite(sp,sl) ;

	} /* end if (have a start) */

	if (sip != NULL)
	    *sip = (f) ? si : 0 ;

#if	CF_DEBUGS && CF_DEBUGSTART
	debugprintf("bibleqs/isstart: f=%u si=%u\n",f,si) ;
#endif

	return (f) ? si : 0  ;
}
/* end subroutine (isstart) */


static int istermrs(int rs)
{
	int		i ;
	int		f = FALSE ;
	for (i = 0 ; termrs[i] != 0 ; i += 1) {
	    f = (rs == termrs[i]) ;
	    if (f) break ;
	} /* end if */
	return f ;
}
/* end subroutine (istermrs) */

