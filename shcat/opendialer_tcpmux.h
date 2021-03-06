/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)opendialertcpmux "
#define	BANNER		"Open-Dialer TCPMUX"
#define	SEARCHNAME	"opendialertcpmux"
#define	VARPRNAME	"EXTRA"

#define	VARPROGRAMROOT1	"OPENDIALERTCPMUX_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"OPENDIALERTCPMUX_BANNER"
#define	VARSEARCHNAME	"OPENDIALERTCPMUX_NAME"
#define	VARFILEROOT	"OPENDIALERTCPMUX_FILEROOT"
#define	VARLOGTAB	"OPENDIALERTCPMUX_LOGTAB"
#define	VARMSFNAME	"OPENDIALERTCPMUX_MSFILE"
#define	VARUTFNAME	"OPENDIALERTCPMUX_UTFILE"
#define	VARERRORFNAME	"OPENDIALERTCPMUX_ERRORFILE"

#define	VARDEBUGFNAME	"OPENDIALERTCPMUX_DEBUGFILE"
#define	VARDEBUGFD1	"OPENDIALERTCPMUX_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARNODE		"NODE"
#define	VARSYSNAME	"SYSNAME"
#define	VARRELEASE	"RELEASE"
#define	VARMACHINE	"MACHINE"
#define	VARARCHITECTURE	"ARCHITECTURE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
#define	VARNISDOMAIN	"NISDOMAIN"
#define	VARPRINTER	"PRINTER"

#define	VARTMPDNAME	"TMPDIR"

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

#define	PIDFNAME	"run/opendialertcpmux"
#define	LOGFNAME	"var/log/opendialertcpmux"
#define	LOCKFNAME	"spool/locks/opendialertcpmux"
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50

#define	TO_CACHE	2



