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
#include "format.hh"
#include "formatunknown.hh"
#include "formatcont.hh"

/*
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
*/

void FormatSpecCont::init(struct TokenLink* tok)
{
  name = stripname(tok);
  tok = tok->next;
  if (!tok)
    ;
  else if (tok->next && !tok->next->next) {
    if (is_cont(tok->token) &&
        is_cont(tok->next->token)) {
      unset = 0;
      min = get_cont(tok->token);
      max = get_cont(tok->next->token);
    } else {
      fprintf(stderr, "Strange cont format arguments: %s %s\n",
              tok->token, tok->next->token);
    }
  }
  else {
    if (!tok->next)
      fprintf(stderr, "Bad number of cont format arguments: %s\n",
              tok->token);
    else
      fprintf(stderr, "Bad number of cont format arguments: %s %s %s ...\n",
              tok->token, tok->next->token, tok->next->next->token);
  }
}

void FormatSpecCont::save(FILE* f)
{
  printname(f);
  if (unset)
    fprintf(f, "cont:\n");
  else
    fprintf(f, "cont:\t%g\t%g\n", min, max);
}

void FormatSpecCont::add(const char* str)
{
  float val;
  if (is_unknown(str)) 
    ;
  else if (is_cont(str)) {
    val = get_cont(str);
    if (unset) {
      min = val;
      max = val;
      unset = 0;
    }
    else if (val < min) {
      min = val;
    }
    else if (val > max) {
      max = val;
    }
  }
  else
    fprintf(stderr, "Bad cont value: %s\n", str);
}

intfloat FormatSpecCont::interpret(const char* str)
{
  intfloat res;
  if (unset) {
    unset = 0;
  }
  if (is_unknown(str))
    res.i = -1;
  else if (is_cont(str))
    res.f = get_cont(str);
  else {
    res.i = -1;
    fprintf(stderr, "Bad cont value: %s\n", str);
  }
  return res;
}

const char* FormatSpecCont::represent(intfloat v)
{
  static char buf[20];
  if (v.i == -1)
    return "?";
  else
    return sprintf(buf, "%f", v.f), buf;
}

void FormatSpecCont::usurp(FormatSpecUnknown* ele)
{
  if (!ele->unset) {
    if (unset) {
      unset = 0;
      min = ele->min;
      max = ele->max;
    } else {
      if (min > ele->min)
        min = ele->min;
      if (max < ele->max)
        max = ele->max;
    }
  }
}
