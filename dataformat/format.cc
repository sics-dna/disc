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
#include "table.hh"
#include "readtokens.hh"
#include "format.hh"
#include "formatunknown.hh"
#include "formatdispatch.hh"
#include "formattypes.hh"
#include "formattime.hh"

/*
class FormatSpec {
  friend class Format;
public:
  FormatSpec(const char* nm = 0);
  virtual ~FormatSpec() { if (name) delete [] name; };
  virtual FormatSpec* copy() { return 0; };
  virtual void init(struct TokenLink* row) {};
  virtual void setname(const char* nm);
  virtual char* stripname(struct TokenLink*& row);
  virtual void printname(FILE* f);
  virtual void add(const char* str) {};
  virtual void save(FILE* f) {};
  virtual intfloat interpret(const char* str) { intfloat r; r.i = -1; return r; };
  virtual const char* represent(intfloat v) { return "?"; };
  virtual int get_discr(int val) { return -1; };
  virtual const char* get_symbol(int val) { return 0; };
  virtual void usurp(class FormatSpecUnknown* ele) {};
  virtual int type() { return -1; };
  virtual int getnum() { return -1; };
  char* name;
  FormatSpec* next;
  int unset;
};

class Format {
public:
  Format();
  ~Format();
  int read(const char* name);
  int save(const char* name);
  int simple_construct(const char* formcodes);
  int construct(const char* name);
  int check_header(TokenLink* row);
  void extract_header(TokenLink* row);
  int* get_translation(TokenLink* row, int& tl);
  void add(FormatSpec* formspec);
  int length() { return (unset ? -1 : len); };
  FormatSpec* nth(int n) { FormatSpec* f; for(f=list; f&&n; n--,f=f->next); return f; };
  FormatSpec* findcolumn(const char* name) { FormatSpec* f; for (f=list; f && (!f->name || strcmp(name, f->name)); f = f->next); return f; };
  void insert_input(TokenLink* tok, intfloat* vec);
  void insert_input_translated(TokenLink* tok, intfloat* vec, int* trans, int tl);
  void save_output(FILE* f, intfloat* vec);
  void save_header(FILE* f);
  int numsections();
  int getsection(int n);
  void newsection();

protected:
  void internal_construct(TokenLink* row);
  void internal_add(TokenLink* row, int ln);
  void internal_guess();

  int len;
  int unset;
  FormatSpec* list;
  class Growable<int> *section_hints;
};
*/

FormatSpec::FormatSpec(const char* nm)
{
  unset = 1;
  next = 0;
  name = (nm ? strcpy(new char[strlen(nm)+1], nm) : 0);
}

void FormatSpec::setname(const char* nm)
{
  if (name) delete [] name; 
  name = (nm ? strcpy(new char[strlen(nm)+1], nm) : 0);
}

char* FormatSpec::stripname(struct TokenLink*& row)
{
  int len;
  char* res;
  if (row && row->token && (*row->token == '"')) {
    len = strlen(row->token) - 1;
    res = new char[len + 1];
    res = strcpy(res, row->token + 1);
    if (res[len-1] == '"')
      res[len-1] = 0;
    row = row->next;
    return res;
  } else
    return 0;
}

void FormatSpec::printname(FILE* f)
{
  if (name)
    fprintf(f, "\"%s\"\t", name);
}

Format::Format()
{
  len = 0;
  unset = 1;
  list = 0;
  section_hints = new Growable<int>(0, 10);
  (*section_hints)[0] = 0;
}

Format::~Format()
{
  FormatSpec* ele;
  while (list) {
    ele = list;
    list = ele->next;
    delete ele;
  }
  delete section_hints;
}

int Format::read(const char* name)
{
  FILE* f;
  FormatSpec* ele;
  FormatSpec* last;
  TokenLink* row;
  int hint;
  if ((f = fopen(name, "r")) == 0)
    return 0;
  while (list) {
    ele = list;
    list = ele->next;
    delete ele;
  }
  last = 0;
  hint = 1;
  do {
    while ((row = readtokens(f)) && (*row->token != 0)) {
      hint = 0;
      ele = format_create(row);
      ele->init(row);
      if (!list)
        last = list = ele;
      else
        last = last->next = ele;
      len += 1;
      freetokens(row);
    }
    freetokens(row);
    if (!hint) {
      (*section_hints)[0]++;
      (*section_hints) << len;
      hint = 1;
    }
  } while (!feof(f));
  fclose(f);
  unset = 0;
  return 1;
}

int Format::save(const char* name)
{
  FILE* f;
  FormatSpec* ele;
  int i, sec;
  if ((f = fopen(name, "w")) == 0)
    return 0;
  ele = list;
  i = 0;
  sec = ((*section_hints)[0] ? 1 : 0);
  while (ele) {
    if (sec && (*section_hints)[sec] == i) {
      fprintf(f, "\n");
      sec++;
    }
    ele->save(f);
    ele = ele->next;
    i++;
  }
  fclose(f);
  return 1;
}

void Format::insert_input(TokenLink* tok, intfloat* vec)
{
  FormatSpec* ele;
  ele = list;
  while (tok && ele) {
    *vec = ele->interpret(tok->token);
    tok = tok->next;
    ele = ele->next;
    vec++;
  }
  if (ele)
    fprintf(stderr, "Too few values in input\n");
  else if (tok)
    fprintf(stderr, "Too many values in input\n");
}

void Format::insert_input_translated(TokenLink* tok, intfloat* vec, int* trans, int tl)
{
  int i = 0;
  for (i=1; i<len; i++)
    vec[i].i = -1;
  i = 0;
  while (tok && i < tl) {
    if (trans[i] > -1)
      vec[trans[i]] = nth(trans[i])->interpret(tok->token);
    tok = tok->next;
    i++;
  }
  if (i < tl)
    fprintf(stderr, "Too few values in input\n");
  else if (tok)
    fprintf(stderr, "Too many values in input\n");
}

void Format::save_output(FILE* f, intfloat* vec)
{
  int i;
  intfloat* p=vec;
  FormatSpec* ele = list;
  if (ele) {
    fprintf(f, "%s", ele->represent(*p));
    ele = ele->next;
    p++;
  }
  for (i=1; i<len && ele; i++, p++, ele=ele->next)
    fprintf(f, "\t%s", ele->represent(*p));
  fprintf(f, "\n");
}

void Format::save_header(FILE* f)
{
  int i;
  FormatSpec* ele = list;
  if (ele) {
    fprintf(f, "%s", (ele->name ? ele->name : ""));
    ele = ele->next;
  }
  for (i=1; i<len && ele; i++, ele=ele->next)
    fprintf(f, "\t%s", (ele->name ? ele->name : ""));
  fprintf(f, "\n");
}

void Format::add(FormatSpec* fs)
{
  FormatSpec* last;
  if (!list)
    list = fs;
  else {
    for (last=list; last->next; last=last->next);
    last->next = fs;
  }
  unset = 0;
  len += 1;
}

int Format::numsections()
{
  return (*section_hints)[0];
}

int Format::getsection(int n)
{
  if (n<(*section_hints)[0])
    return (*section_hints)[n+1];
  else
    return -1;
}

void Format::newsection()
{
  if ((*section_hints)[(*section_hints)[0]] != len) {
    (*section_hints)[0]++;
    (*section_hints) << len;
  }
}

void Format::internal_construct(TokenLink* row)
{
  FormatSpec* ele;
  FormatSpec* last;
  if (!row || (!*row->token && !row->next))
    return;
  last = 0;
  while (row) {
    ele = format_guess(row->token, 1);
    ele->add(row->token);
    if (!list)
      last = list = ele;
    else
      last = last->next = ele;
    len += 1;
    row = row->next;
  }
}

void Format::internal_add(TokenLink* row, int ln)
{
  FormatSpec* ele;
  FormatSpec* last;
  FormatSpec* ele2;
  if (!row || (!*row->token && !row->next))
    return;
  ele = list;
  last = 0;
  while (row && ele) {
    if (IsType(ele, FormatSpecUnknown))
      if ((ele2 = format_guess(row->token, 0))) {
        ele2->usurp((FormatSpecUnknown*) ele);
        ele2->next = ele->next;
        if (last)
          last->next = ele2;
        else
          list = ele2;
        delete ele;
        ele = ele2;
      }
    ele->add(row->token);
    row = row->next;
    last = ele;
    ele = ele->next;
  }
  if (ele)
    fprintf(stderr, "Too few values in input, on line %d\n", ln);
  else if (row)
    fprintf(stderr, "Too many values in input, on line %d\n", ln);
}

void Format::extract_header(TokenLink* row)
{
  int l;
  FormatSpec* ele = list;
  while (ele && row) {
    if (row->token && (*row->token == '"')) {
      l = strlen(row->token) - 1;
      ele->name = new char[l + 1];
      strcpy(ele->name, row->token + 1);
      if (ele->name[l-1] == '"')
        ele->name[l-1] = 0;
    } else if (row->token) {
      l = strlen(row->token);
      ele->name = new char[l + 1];
      strcpy(ele->name, row->token);
    }
    ele = ele->next;
    row = row->next;
  }
  if (ele)
    fprintf(stderr, "Too few header names in input\n");
  else if (row)
    fprintf(stderr, "Too many header names in input\n");
}

int Format::check_header(TokenLink* row)
{
  int c_match = 0, c_miss = 0;
  FormatSpec* ele = list;
  while (ele && row) {
    if (!is_unknown(row->token) && ele->type() != FormatSpecSymbolType) {
      if (ele->type() == FormatSpecDatetimeType) {
        if (check_datetime(row->token))
          c_match++;
        else
          c_miss++;
      } else {
        if (is_cont(row->token)) // Aproximation.
          c_match++;
        else
          c_miss++;
      }
    }
    ele = ele->next;
    row = row->next;
  }
  return (c_miss > c_match);
}

int* Format::get_translation(TokenLink* row, int& tl)
{
  int* trans;
  int i, j, leng;
  FormatSpec* f;
  TokenLink* tok;
  for (leng=0, tok=row; tok; leng++, tok=tok->next); 
  trans = new int[leng];
  for (i=0, tok=row; tok; i++, tok=tok->next) {
    for(f=list, j=0; f && (!f->name || strcmp(tok->token, f->name)); f=f->next, j++);
    trans[i] = (f ? j : -1);
  }
  tl = leng;
  return trans;
}

void Format::internal_guess()
{
  FormatSpec* ele;
  FormatSpec* last;
  FormatSpec* ele2;
  ele = list;
  last = 0;
  while (ele) {
    if (IsType(ele, FormatSpecUnknown)) {
      if ((ele2 = format_guess_unknown((FormatSpecUnknown*) ele))) {
        ele2->usurp((FormatSpecUnknown*) ele);
        ele2->next = ele->next;
        if (last)
          last->next = ele2;
        else
          list = ele2;
        delete ele;
        ele = ele2;
      }
    }
    last = ele;
    ele = ele->next;
  }
}

int Format::simple_construct(const char* formcodes)
{
  FormatSpec* ele;
  FormatSpec* last;
  const char* c;
  if (!formcodes) {
    fprintf(stderr, "No format string provided.\n");
    return 0;
  }
  last = 0;
  for (c=formcodes; *c; c++) {
    ele = format_create(*c);
    if (!list)
      last = list = ele;
    else
      last = last->next = ele;
    len += 1;
  }
  unset = 0;
  return 1;
}

int Format::construct(const char* name)
{
  FILE* f;
  int linenum = 0;
  TokenLink* row;
  TokenLink* frow = 0;
  char* tmp;
  if ((f = fopen(name, "r")) == 0)
    return 0;
  if (unset) {
    frow = readtokens(f);
    linenum +=1;
    tmp = freezebuf();
    row = readtokens(f);
    linenum +=1;
    internal_construct(row);
    freetokens(row);
    unset = 0;
    row = readtokens(f);
    linenum +=1;
  } else {
    row = readtokens(f);
    linenum +=1;
    if (check_header(row)) {
      freetokens(row);
      row = readtokens(f);
      linenum +=1;
    }
  }
  while (row) {
    internal_add(row, linenum);
    freetokens(row);
    row = readtokens(f);
    linenum +=1;
  }
  internal_guess();
  if (frow) {
    if (check_header(frow))
      extract_header(frow);
    else
      internal_add(frow, 1);
    freetokens(frow);
    delete [] tmp;
  }
  fclose(f);
  return 1;
}
