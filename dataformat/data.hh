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

#ifndef DATA_HH
#define DATA_HH

#include "growable.hh"
#include <stdio.h>

class DataObject {
public:
  DataObject();
  DataObject(class Format* fr, const char* datafile = 0);
  DataObject(class Format* fr, FILE* datafile);
  DataObject(const char* formatfile, const char* datafile = 0);
  virtual ~DataObject();
  virtual void clear();
  virtual void remove(int ind, int n = 1);
  virtual union intfloat* newentry();
  virtual int read(const char* datafile, int append = 0);
  virtual int save(const char* datafile, int standard = 0);
  virtual char* getname(int n);
  virtual int findname(const char* str);
  virtual int hasnames();
  virtual void sort(int (*func)(union intfloat* v1, union intfloat* v2));
  virtual union intfloat* operator[](int i) { if (i<(bl<0 ? num : num-1))
                                        return (*data)[(bl<0 || bl>i ? i : i+1)];
                                      return 0; };
  virtual void block(int i) { bl = i; };
  virtual int size() { return (bl<0 ? num : num-1); };
  virtual int length() { return len; };
  virtual class Format* format() { return form; };
  virtual void setformat(Format* f, int own = 0) { if (!form) { form = f; len = f->length(); ownform = own; }};
protected:
  virtual int read_file(FILE* f, int append = 0);
  virtual int internal_read(FILE* f);
  virtual int internal_save(FILE* f);
  int bl;
  int num;
  int len;
  Format* form;
  int ownform;
  class Growable<union intfloat* > *data;
};

#endif /* DATA_HH */
