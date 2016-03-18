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
#include "formatdiscr.hh"

/*
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
*/

void FormatSpecDiscr::init(struct TokenLink* tok)
{
  name = stripname(tok);
  tok = tok->next;
  if (!tok)
    ;
  else if (!tok->next) {
    if (is_posint(tok->token)) {
      unset = 0;
      offset = 0;
      num = get_posint(tok->token);
    } else {
      fprintf(stderr, "Strange discr format argument: %s\n", tok->token);
    }
  }
  else if (!tok->next->next) {
    if ((is_posint(tok->token) || is_negint(tok->token)) &&
        is_posint(tok->next->token)) {
      unset = 0;
      offset = get_posint(tok->token);
      num = get_posint(tok->next->token) - offset + 1;
    } else {
      fprintf(stderr, "Strange discr format arguments: %s %s\n",
              tok->token, tok->next->token);
    }
  }
  else {
    fprintf(stderr, "To many discr format arguments: %s %s %s ...\n",
            tok->token, tok->next->token, tok->next->next->token);
  }
}

void FormatSpecDiscr::save(FILE* f)
{
  printname(f);
  if (unset)
    fprintf(f, "discr:\n");
  else if (offset == 0)
    fprintf(f, "discr:\t%d\n", num);
  else
    fprintf(f, "discr:\t%d\t%d\n", offset,
            num + offset - 1);
}

void FormatSpecDiscr::add(const char* str)
{
  int val;
  if (is_unknown(str)) 
    ;
  else if (is_posint(str) || is_negint(str)) {
    val = get_posint(str);
    if (unset) {
      offset = val;
      num = 1;
      unset = 0;
    }
    else if (val < offset && !dynamic) {
      num += (offset - val);
      offset = val;
    }
    else if (val >= num + offset) {
      num = val - offset + 1;
    }
  }
  else
    fprintf(stderr, "Bad discr value: %s\n", str);
}

intfloat FormatSpecDiscr::interpret(const char* str)
{
  intfloat res;
  if (unset) {
    unset = 0;
    dynamic = 1;
  }
  if (is_unknown(str))
    res.i = -1;
  else if (is_posint(str) || (is_negint(str) && offset < 0)) {
    res.i = get_posint(str) - offset;
    if (dynamic && res.i >= num)
      num = res.i + 1;
  } else {
    res.i = -1;
    fprintf(stderr, "Bad discr value: %s\n", str);
  }
  return res;
}

const char* FormatSpecDiscr::represent(intfloat v)
{
  static char buf[12];
  if (v.i == -1)
    return "?";
  else
    return sprintf(buf, "%lld", (long long)(v.i + offset)), buf;
}

int FormatSpecDiscr::get_discr(int val)
{
  return val + offset;
}

void FormatSpecDiscr::usurp(FormatSpecUnknown* ele)
{
  if (!ele->unset) {
    if (unset) {
      unset = 0;
      offset = (int) ele->min;
      num = (int) ele->max - offset + 1;
    } else {
      if (offset > (int) ele->min)
        num += (offset - (int) ele->min), offset = (int) ele->min;
      if (num + offset <= (int) ele->max)
        num = (int) ele->max - offset + 1;
    }
  }
}



