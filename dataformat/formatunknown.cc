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

/*
class FormatSpecUnknown : public FormatSpec {
public:
  FormatSpecUnknown(const char* nm = 0) : FormatSpec(nm) {};
  virtual ~FormatSpecUnknown();
  virtual FormatSpec* copy();
  virtual void init(struct TokenLink* tok);
  virtual void add(const char* str);
  virtual void save(FILE* f);
  virtual intfloat interpret(const char* str);
  virtual int type() { return 0; };
  float min;
  float max;
  int num;
  class IndexTable* table;
};
*/

FormatSpecUnknown::~FormatSpecUnknown()
{
  delete table;
}

FormatSpec* FormatSpecUnknown::copy()
{
  FormatSpecUnknown* fs = new FormatSpecUnknown(name);
  fs->min = min;
  fs->max = max;
  fs->num = num;
  fs->table = table->copy();
  fs->unset = unset;
  return fs; 
}

void FormatSpecUnknown::init(struct TokenLink* tok)
{
  name = stripname(tok);
  if (tok != NULL)
    fprintf(stderr, "Unknown format type: %s\n", tok->token);
  else
    fprintf(stderr, "Unknown format type: %s\n", name);
}

void FormatSpecUnknown::save(FILE* f)
{
  printname(f);
  if (unset)
    fprintf(f, "unknown: \n");
  else
    fprintf(f, "unknown: %g %g %d\n", min, max, num);
  fprintf(stderr, "Unknown format type saved\n");
}

void FormatSpecUnknown::add(const char* str)
{
  float val;
  if (is_unknown(str)) 
    ;
  else if (unset) {
    unset = 0;
    min = max = get_cont(str);
    num = 1;
    table = new IndexTable;
    table->insert(str);
  } else {
    val = get_cont(str);
    if (val < min)
      min = val;
    else if (val > max)
      max = val;
    if (table->get(str) < 0) {
      table->insert(str);
      num += 1;
    }
  }
}
  
intfloat FormatSpecUnknown::interpret(const char* str)
{
  intfloat res;
  res.i = -1;
  return res;
}
