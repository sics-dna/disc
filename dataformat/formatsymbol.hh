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

class FormatSpecSymbol : public FormatSpec {
public:
  FormatSpecSymbol(const char* nm = 0);
  virtual ~FormatSpecSymbol();
  virtual FormatSpec* copy();
  virtual void init(struct TokenLink* tok);
  virtual void add(const char* str);
  virtual void save(FILE* f);
  virtual intfloat interpret(const char* str);
  virtual const char* represent(intfloat v);
  virtual const char* get_symbol(int val);
  virtual void usurp(class FormatSpecUnknown* ele);
  virtual int type() { return FORMATSPEC_SYMBOL; };
  virtual int getnum() { return num; };
  int num;
  class IndexTable* table;
  int dynamic;
};
