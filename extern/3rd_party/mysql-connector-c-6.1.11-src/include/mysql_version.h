/* Copyright Abandoned 1996,1999 TCX DataKonsult AB & Monty Program KB
   & Detron HB, 1996, 1999-2004, 2007 MySQL AB.
   This file is public domain and comes with NO WARRANTY of any kind
*/

/* Version numbers for protocol & mysqld */

#ifndef _mysql_version_h
#define _mysql_version_h
#ifdef _CUSTOMCONFIG_
#include <custom_conf.h>
#else
#define PROTOCOL_VERSION            @PROTOCOL_VERSION@
#define MYSQL_SERVER_VERSION       "@SERVER_VERSION@"
#define MYSQL_VERSION_ID            @SERVER_VERSION_ID@
#define MYSQL_PORT                  @MYSQL_TCP_PORT@
#define MYSQL_PORT_DEFAULT          @MYSQL_TCP_PORT_DEFAULT@
#define MYSQL_UNIX_ADDR            "@MYSQL_UNIX_ADDR@"
#define MYSQL_CONFIG_NAME          "my"
#define MYSQL_COMPILATION_COMMENT  "@COMPILATION_COMMENT@"
#define LIBMYSQL_VERSION           "@VERSION@"
#define LIBMYSQL_VERSION_ID         @MYSQL_VERSION_ID@

/* mysqld compile time options */
#endif /* _CUSTOMCONFIG_ */

#ifndef LICENSE
#define LICENSE                     GPL
#endif /* LICENSE */

#endif /* _mysql_version_h */
