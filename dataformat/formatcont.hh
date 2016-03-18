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

class FormatSpecCont : public FormatSpec {
public:
  FormatSpecCont(const char* nm = 0) : FormatSpec(nm) { min = 0.0; max = 0.0; };
  virtual FormatSpec* copy() { FormatSpecCont* fs = new FormatSpecCont(name); fs->min = min; fs->max = max; fs->unset = unset; return fs; };
  virtual void init(struct TokenLink* row);
  virtual void add(const char* str);
  virtual void save(FILE* f);
  virtual intfloat interpret(const char* str);
  virtual const char* represent(intfloat v);
  virtual void usurp(FormatSpecUnknown* ele);
  virtual int type() { return FORMATSPEC_CONT; };
  float min;
  float max;
};
