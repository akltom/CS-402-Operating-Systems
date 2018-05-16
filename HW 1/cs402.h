/******************************************************************************/
/* Important CSCI 402 usage information:                                      */
/*                                                                            */
/* This fils is part of CSCI 402 warmup programming assignments at USC.       */
/* Please understand that you are NOT permitted to distribute or publically   */
/*         display a copy of this file (or ANY PART of it) for any reason.    */
/* If anyone (including your prospective employer) asks you to post the code, */
/*         you must inform them that you do NOT have permissions to do so.    */
/* You are also NOT permitted to remove or alter this comment block.          */
/******************************************************************************/

/*
 * Author:      William Chia-Wei Cheng (bill.cheng@acm.org)
 *:
 * @(#)$Id: cs402.h,v 1.1 2012/08/24 00:42:42 william Exp $
 */

#ifndef _CS402_H_
#define _CS402_H_

#ifndef NULL
#define NULL 0L
#endif /* ~NULL */

#ifndef TRUE
#define FALSE 0
#define TRUE 1
#endif /* ~TRUE */

#ifdef WIN32
#define DIR_SEP '\\'
#else /* ~WIN32 */
#define DIR_SEP '/'
#endif /* WIN32 */

#ifndef max
#define max(A,B) (((A)>(B)) ? (A) : (B))
#define min(A,B) (((A)>(B)) ? (B) : (A))
#endif /* ~max */

#ifndef round
#define round(X) (((X) >= 0) ? (int)((X)+0.5) : (int)((X)-0.5))
#endif /* ~round */

#ifndef MAXPATHLENGTH
#define MAXPATHLENGTH 256
#endif /* ~MAXPATHLENGTH */

#endif /*_CS402_H_*/
