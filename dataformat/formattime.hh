 /*
 --------------------------------------------------------------------------
 Copyright (C) 2012, 2015 Anders Holst <aho@sics.se> 

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

#ifndef FORMAT_TIME_HH
#define FORMAT_TIME_HH

class FormatSpecDatetime : public FormatSpec {
public:
  FormatSpecDatetime(const char* nm = 0) : FormatSpec(nm) { dtform = 0; first.i = last.i = -1; };
  virtual FormatSpec* copy() { FormatSpecDatetime* fs = new FormatSpecDatetime(name); fs->dtform = dtform; fs->first = first; fs->last = last; fs->unset = unset; return fs; };
  virtual void init(struct TokenLink* tok);
  virtual void add(const char* str);
  virtual void save(FILE* f);
  virtual intfloat interpret(const char* str);
  virtual const char* represent(intfloat v);
  virtual void get_time(int val, int& h, int& mi, int& s);
  virtual void get_date(int val, int& y, int& mo, int& d);
  virtual void usurp(FormatSpecUnknown* ele);
  virtual int type() { return 5; };
//  char* timeform;
//  int offset;
  int dtform;
  intfloat first, last;
};

extern int check_datetime(const char* str);

#endif
