/*
 * Copyright (c) 1995, 1994, 1993, 1992, 1991, 1990  
 * Open Software Foundation, Inc. 
 *  
 * Permission to use, copy, modify, and distribute this software and 
 * its documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation, and that the name of ("OSF") or Open Software 
 * Foundation not be used in advertising or publicity pertaining to 
 * distribution of the software without specific, written prior permission. 
 *  
 * OSF DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE 
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL OSF BE LIABLE FOR ANY 
 * SPECIAL, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES 
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN 
 * ACTION OF CONTRACT, NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING 
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE 
 */
/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 * 
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 * OSF Research Institute MK6.1 (unencumbered) 1/31/1995
 */

#include <mach/boolean.h>
#include <mach/error.h>
#include <mach_error.h>
#include <errorlib.h>

static
do_compat( org_err )
	mach_error_t		* org_err;
{
	mach_error_t		err = *org_err;

	/* 
	 * map old error numbers to 
	 * to new error sys & subsystem 
	 */

	if ((-200 < err) && (err <= -100))
		err = -(err + 100) | IPC_SEND_MOD;
	else if ((-300 < err) && (err <= -200))
		err = -(err + 200) | IPC_RCV_MOD;
	else if ((-400 < err) && (err <= -300))
		err = -(err + 300) | MACH_IPC_MIG_MOD;
	else if ((1000 <= err) && (err < 1100))
		err = (err - 1000) | SERV_NETNAME_MOD;
	else if ((1600 <= err) && (err < 1700))
		err = (err - 1600) | SERV_ENV_MOD;
	else if ((27600 <= err) && (err < 27700))
		err = (err - 27600) | SERV_EXECD_MOD;

	*org_err = err;
}

char *
mach_error_type( err )
	mach_error_t		err;
{
	int sub, system;

	do_compat( &err );

	sub = err_get_sub(err);
	system = err_get_system(err);

	if (system > err_max_system
	||  sub >= errors[system].max_sub ) return( "(?/?)" );
	return( errors[system].subsystem[sub].subsys_name );
}

boolean_t mach_error_full_diag = FALSE;

char *
mach_error_string_int( err, diag )
	mach_error_t		err;
	boolean_t		* diag;
{
	int sub, system, code;
	char * err_str;

	do_compat( &err );

	sub = err_get_sub(err);
	system = err_get_system(err);
	code = err_get_code(err);

	*diag = TRUE;

	if (system > err_max_system) return( "(?/?) unknown error system" );
	if (sub >= errors[system].max_sub) return( errors[system].bad_sub );
	if (code >= errors[system].subsystem[sub].max_code) return ( NO_SUCH_ERROR );

	*diag = mach_error_full_diag;
	return( errors[system].subsystem[sub].codes[code] );
}

char *
mach_error_string( err )
	mach_error_t		err;
{
	boolean_t diag;

	return mach_error_string_int( err, &diag );

}
