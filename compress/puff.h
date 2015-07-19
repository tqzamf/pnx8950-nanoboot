/* puff.h
  Copyright (C) 2002, 2003 Mark Adler, all rights reserved
  version 1.7, 3 Mar 2002

  Modified to save space by omitting error handling and using global
  variables.

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the author be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  Mark Adler    madler@alumni.caltech.edu
 */


/*
 * See puff.c for purpose and usage.
 */
void puff_init(void);
int puff(unsigned char *dest,           /* pointer to destination */
         unsigned long *destlen,        /* output parameter: number of bytes written */
         unsigned char *source,         /* pointer to source data */
         unsigned long *srclen);        /* output parameter: number of bytes read */
void puff_tick(void);
