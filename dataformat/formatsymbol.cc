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

#include <stdio.h>
#include <string.h>
#include "readtokens.hh"
#include "table.hh"
#include "format.hh"
#include "formatunknown.hh"
#include "formatsymbol.hh"

/*
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
*/

FormatSpecSymbol::FormatSpecSymbol(const char* nm)
  : FormatSpec(nm)
{
  num = 0;
  table = new IndexTable;
  dynamic = 0;
}

FormatSpecSymbol::~FormatSpecSymbol()
{
  delete table;
}

FormatSpec* FormatSpecSymbol::copy()
{
  FormatSpecSymbol* fs = new FormatSpecSymbol(name);
  fs->num = num;
  delete fs->table;
  fs->table = table->copy();
  fs->dynamic = dynamic;
  fs->unset = unset;
  return fs;
}

void FormatSpecSymbol::init(struct TokenLink* tok)
{
  name = stripname(tok);
  tok = tok->next;
  if (tok) {
    unset = 0;
    num = 0;
    while (tok) {
      table->insert_last(tok->token);
      num++;
      tok = tok->next;
    }
  }
}

void FormatSpecSymbol::save(FILE* f)
{
  printname(f);
  if (unset)
    fprintf(f, "symbol:\n");
  else {
    char* str;
    fprintf(f, "symbol:");
    table->reset();
    while ((str = table->next()))
      fprintf(f, "\t%s", str);
    fprintf(f, "\n");
  }
}

void FormatSpecSymbol::add(const char* str)
{
  if (unset)
    unset = 0;
  if (is_unknown(str)) 
    ;
  else if (table->get(str) < 0) {
    table->insert(str);
    num += 1;
  }
}

intfloat FormatSpecSymbol::interpret(const char* str)
{
  intfloat res;
  if (unset) {
    unset = 0;
    dynamic = 1;
  }
  if (is_unknown(str))
    res.i = -1;
  else {
    res.i = table->get(str);
    if (res.i == -1) {
      if (dynamic) {
        table->insert_last(str);
        res.i = num++;
      } else
        fprintf(stderr, "Unknown symbol: %s\n", str);
    }
  }
  return res;
}

const char* FormatSpecSymbol::represent(intfloat v)
{
  if (v.i == -1)
    return "?";
  else
    return table->nth(v.i);
}

const char* FormatSpecSymbol::get_symbol(int val)
{
  return table->nth(val);
}
  
void FormatSpecSymbol::usurp(FormatSpecUnknown* ele)
{
  char* sym;
  if (!ele->unset) {
    if (unset) {
      unset = 0;
      num = 0;
      table = new IndexTable;
    }
    ele->table->reset();
    while ((sym = ele->table->next()))
      if (table->get(sym) < 0) {
        table->insert(sym);
        num += 1;
      }
  }
}
