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

#ifndef FORMAT_HH
#define FORMAT_HH

#include <stdio.h>
#include <string.h>
#include "intfloat.hh"
#include "growable.hh"

enum {FORMATSPEC_NYI = -1,
      FORMATSPEC_UNKNOWN = 0,
      FORMATSPEC_BINARY = 1,
      FORMATSPEC_DISCR = 2,
      FORMATSPEC_CONT = 3,
      FORMATSPEC_SYMBOL = 4};

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
  virtual int type() { return FORMATSPEC_NYI; };
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

#endif
