 /*
 --------------------------------------------------------------------------
 Copyright (C) 1996, 2012, 2015 Anders Holst <aho@sics.se> 

 This code is free software: you can redistribute it and/or modify it
 under the terms of the GNU Lesser General Public License as published
 by the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This code is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License
 along with this code.  If not, see <http://www.gnu.org/licenses/>.
 --------------------------------------------------------------------------
 */

#ifndef INTFLOAT

#define INTFLOAT

#ifdef IF_FLOAT
#error "Don't use IF_FLOAT, use INTFLOAT_LONG"
#endif

#ifdef INTFLOAT_LONG
#define IF_FLOAT double
#define IF_INT long long
#else
#define IF_FLOAT float
#define IF_INT int
#endif

union intfloat {
  IF_INT i;
  IF_FLOAT f;
  intfloat() {};
  intfloat(IF_FLOAT ff) { f = ff; };
  intfloat(IF_INT ii) { i = ii; };
  operator IF_FLOAT() { return f; };
  operator IF_INT() { return i; };
};

#endif
