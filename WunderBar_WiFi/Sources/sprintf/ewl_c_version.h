/* EWL
 * Copyright © 1995-2009 Freescale Corporation.  All rights reserved.
 *
 * $Date: 2012/06/01 15:39:38 $
 * $Revision: 1.1 $
 */

#ifndef _EWL_C_VERSION_H
#define _EWL_C_VERSION_H

/*
 * The __EWL__ identifier is a hex value tracking, more or less,
 * a decimal identity - that is, for builds 10 - 16, we will use a
 * decimal appearing number.
 *
 * The __EWL__ identifier is broken up into three parts:  the major
 * version number, a zero (for backwards compatibility with older
 * version of the __EWL__ identifier), and the minor version number.
 *
 * The major version number takes the first five positions, the
 * zero is in the sixth position, and the minor version number is in
 * the final two positions; therefore: 0x000008001 means the major
 * MSL version is 8, and the minor version is 1.  This translates to
 * mean that MSL is the 8.1 version.
 *
 * The identifier is monotonically increasing for each build.
 */

/* EWL 15.1 */
#define __EWL__ 0x00015001

#define __EWL_BUILD__ 1

#endif /* _EWL_C_VERSION_H */
