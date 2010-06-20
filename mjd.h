
/* $Id: mjd.h,v 1.3 2005/01/18 14:00:46 purbanec Exp $ */

/*

  Copyright (C) 2004 Peter Urbanec <toppy at urbanec.net>

  This file is part of puppy.

  puppy is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  puppy is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with puppy; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#ifndef _MJD_H
#define _MJD_H 1

#include <time.h>
#include "types.h"

/* Encapsulation of MJD date and h:m:s timestamp */
#pragma pack(push,1)
struct tf_datetime
{
    __u16 mjd;
    __u8 hour;
    __u8 minute;
    __u8 second;
};
#pragma pack(pop)


time_t tfdt_to_time(struct tf_datetime *dt);
void time_to_tfdt(time_t t, struct tf_datetime *dt);
void time_to_tfdt64(__time64_t t, struct tf_datetime *dt);
#endif /* _MJD_H */
