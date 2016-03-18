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

class FormatSpecDiscr : public FormatSpec {
public:
  FormatSpecDiscr(const char* nm = 0) : FormatSpec(nm) { offset = 0; num = 0; dynamic = 0; };
  virtual FormatSpec* copy() { FormatSpecDiscr* fs = new FormatSpecDiscr(name); fs->offset = offset; fs->num = num; fs->dynamic = dynamic; fs->unset = unset; return fs; };
  virtual void init(struct TokenLink* tok);
  virtual void add(const char* str);
  virtual void save(FILE* f);
  virtual intfloat interpret(const char* str);
  virtual const char* represent(intfloat v);
  virtual int get_discr(int val);
  virtual void usurp(FormatSpecUnknown* ele);
  virtual int type() { return FORMATSPEC_DISCR; };
  virtual int getnum() { return num; };
  int offset;
  int num;
  int dynamic;
};
