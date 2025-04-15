/* Copyright Abandoned 1996, 1999, 2001 MySQL AB
   This file is public domain and comes with NO WARRANTY of any kind */

/* Version numbers for protocol & mysqld */

#ifndef _mariadb_version_h_
#define _mariadb_version_h_

#ifdef _CUSTOMCONFIG_
#include <custom_conf.h>
#else
#define PROTOCOL_VERSION		10
#define MARIADB_CLIENT_VERSION_STR	"10.11.4"
#define MARIADB_BASE_VERSION		"mariadb-10.11"
#define MARIADB_VERSION_ID		101104
#define MARIADB_PORT	        	3306
#define MARIADB_UNIX_ADDR               "/var/run/mysql/mysql.sock"
#ifndef MYSQL_UNIX_ADDR
#define MYSQL_UNIX_ADDR MARIADB_UNIX_ADDR
#endif
#ifndef MYSQL_PORT
#define MYSQL_PORT MARIADB_PORT
#endif

#define MYSQL_CONFIG_NAME               "my"
#define MYSQL_VERSION_ID                101104
#define MYSQL_SERVER_VERSION            "10.11.4-MariaDB"

#define MARIADB_PACKAGE_VERSION "3.3.5"
#define MARIADB_PACKAGE_VERSION_ID 30305
#define MARIADB_SYSTEM_TYPE "FreeBSD"
#define MARIADB_MACHINE_TYPE "i386"
#define MARIADB_PLUGINDIR "/usr/local/lib/mysql/plugin"

/* mysqld compile time options */
#ifndef MYSQL_CHARSET
#define MYSQL_CHARSET			""
#endif
#endif

/* Source information */
#define CC_SOURCE_REVISION ""

#endif /* _mariadb_version_h_ */
