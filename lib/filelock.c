/*
 * Copyright 2007 (c) SAEIL Systems Inc. All Rights Reserved.
 *
 * MODULE NAME : filelock.c
 *
 * PRODUCT(S)  : LSIS DCU FEP Development
 *
 * DESCRIPTION : file locking libraries.
 *
 * CAUTION     :
 *
 * GLOBAL FUNCTIONS DEFINED IN THIS MODULE :
 *
 * REVISION HISTORY
 *   Date      Who    Rev                       Comments
 * ---------- ------  ----  -------------------------------------------
 * 2006.11.03 ChoiBC   01   create
 */

/*
 * include files
 */
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include "filelock.h"
//#include "logs.h"
#include <stdio.h>

/*
 *	lock_reg
 *		lock pdb
 *	RETURN : result status
 *		NOK -1 : failure or error
 *		OK	0  : success
 */
int lock_reg( int fd, int cmd, int type, off_t offset, off_t len )
{
	struct flock lock;

	lock.l_type = type;			/* F_RDLCK, F_WRLCK, F_UNLCK */
	lock.l_start = offset;		/* byte offset, relative to l_whence */
	lock.l_whence = SEEK_SET;	/* SEEK_SET, SEEK_CUR, SEEK_END */
	lock.l_len = len;			/* #bytes (0 means until EOF) */

	if( fcntl( fd, cmd, &lock ) == -1 ) {
//		logtW( "fcntl error, <%d,%s>\n", errno, strerror(errno) );
		printf( "lock_reg : fcntl error, <%d,%s>\n", errno, strerror(errno) );
		return( -1 );
	}
	else
		return( 0 );
}

/*  End of filelock.c  */
