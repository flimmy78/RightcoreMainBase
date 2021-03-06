/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)incima "
#define	SEARCHNAME	"incima"
#define	BANNER		"Include Imagine"

#define	VARPROGRAMROOT1	"INCIMA_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"INCIMA_BANNER"
#define	VARSEARCHNAME	"INCIMA_NAME"
#define	VARFILEROOT	"INCIMA_FILEROOT"
#define	VARLOGTAB	"INCIMA_LOGTAB"
#define	VARERRORFNAME	"INCIMA_ERRORFILE"

#define	VARDEBUGFNAME	"INCIMA_DEBUGFILE"
#define	VARDEBUGFD1	"INCIMA_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARSYSNAME	"SYSNAME"
#define	VARRELEASE	"RELEASE"
#define	VARMACHINE	"MACHINE"
#define	VARARCHITECTURE	"ARCHITECTURE"
#define	VARHZ		"HZ"
#define	VARNODE		"NODE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
#define	VARDOMAIN	"DOMAIN"
#define	VARNISDOMAIN	"NISDOMAIN"
#define	VARTERM		"TERM"
#define	VARPRINTER	"PRINTER"
#define	VARLPDEST	"LPDEST"
#define	VARPAGER	"PAGER"
#define	VARMAIL		"MAIL"
#define	VARORGANIZATION	"ORGANIZATION"
#define	VARLINES	"LINES"
#define	VARCOLUMNS	"COLUMNS"
#define	VARNAME		"NAME"
#define	VARFULLNAME	"FULLNAME"
#define	VARTZ		"TZ"
#define	VARUSERNAME	"USERNAME"
#define	VARLOGNAME	"LOGNAME"
#define	VARPATH		"PATH"
#define	VARMANPATH	"MANPATH"
#define	VARCDPATH	"CDPATH"
#define	VARLIBPATH	"LD_LIBRARY_PATH"

#define	VARHOMEDNAME	"HOME"
#define	VARTMPDNAME	"TMPDIR"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"
#define	FULLFNAME	".fullname"

#define	PIDFNAME	"run/incima"		/* mutex PID file */
#define	LOGFNAME	"var/log/incima"	/* activity log */
#define	LOCKFNAME	"spool/locks/incima"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	TO_FILEMOD	(1 * 60 * 60)		/* IPASSWD timeout */

#define	LINELEN		200
#define	BIBLEN		200

#define	MACRONAME1	".Ig"
#define	MACRONAME2	".Ii"
#define	MACRONAME3	".BG"

#define	IMADIR		".incima"

/* the default picture to include when the user's is not there (whatever ) */

#define	PS_DEFAULT	"ii_default.ps"
#define	PS_NOEXIST	"ii_noexist.ps"
#define	PS_NOREAD	"ii_noread.ps"
#define	PS_UNKNOWN	"ii_unknown.ps"



