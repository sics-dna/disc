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
#include <stdlib.h>
#include <string.h>

#ifdef GZLIB
#include <zlib.h>
#endif

#include "readtokens.hh"
#include "format.hh"
#include "data.hh"

/*
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
 */

DataObject::DataObject()
{
  num = 0;
  len = 0;
  bl = -1;
  form = 0;
  ownform = 0;
  data = new Growable<intfloat*>(0, 128);
}

DataObject::DataObject(class Format* fr, const char* datafile)
{
  num = 0;
  bl = -1;
  form = fr;
  ownform = 0;
  len = form->length();
  data = new Growable<intfloat*>(0, 128);
  if (datafile)
    read(datafile);
}

DataObject::DataObject(class Format* fr, FILE* datafile)
{
  num = 0;
  bl = -1;
  form = fr;
  ownform = 0;
  len = form->length();
  data = new Growable<intfloat*>(0, 128);
  if (datafile)
    read_file(datafile);
}


DataObject::DataObject(const char* formatfile, const char* datafile)
{
  num = 0;
  bl = -1;
  data = new Growable<intfloat*>(0, 128);
  form = new Format();
  ownform = 1;
  form->read(formatfile);
  len = form->length();
  if (len == -1) {
    fprintf(stderr, "Failed to read format file \"%s\"\n", formatfile);
    return;
  }
  if (datafile)
    read(datafile);
}

DataObject::~DataObject()
{
  clear();
  delete data;
  if (ownform)
    delete form;
}

void DataObject::clear()
{
  intfloat* vec;
  if (data) {
    FORVEC(*data, vec)
      delete [] vec;
    data->remove(num);
    num = 0;
  }
}

void DataObject::remove(int ind, int n)
{
  int i;
  if (data) {
    if (ind + n > num)
      n = num - ind;
    for (i=ind; i<n+ind; i++) {
      delete [] (*data)[i];
      (*data)[i] = (*data)[i+n];
    }
    for (; i<num-n; i++)
      (*data)[i] = (*data)[i+n];
    data->remove(n);
    num -= n;
  }
}

union intfloat* DataObject::newentry()
{
  intfloat* vec;
  vec = new intfloat[len];
  (*data)[num++] = vec;
  return vec;
}

int DataObject::read(const char* datafile, int append)
{
  FILE* f;

  if ((f = fopen(datafile, "r")) == 0) {
    fprintf(stderr, "Failed to read data file \"%s\"\n", datafile);
    return -1;
  }
  int status = read_file(f, append);
  fclose(f);
  return status;
}

int DataObject::read_file(FILE* f, int append) {
  int status;
  if (!append) {
    clear();
  }
  status = internal_read(f);

  return (status ? num : -1);
}

#ifdef GZLIB
extern FILE *gz_file;
extern gzFile gz_gzfile;

void getfirst(FILE *f)
{
  int c = fgetc(f);
  //printf("char1 = %d\n", c);
  rewind(f);
  if (c == 31) {
    gz_gzfile = gzdopen(fileno(f), "rb");
    //printf("gzf = %p\n", gz_gzfile);
    gz_file = f;
  } else {
    gz_gzfile = NULL;
    gz_file = NULL;
  }
}
#endif

int DataObject::internal_read(FILE* f)
{
  intfloat* vec;
  TokenLink* row;
  int* trans = 0;
  int translen;

#ifdef GZLIB
  getfirst(f);
#endif

  row = readtokens(f);
  if (form->check_header(row)) {
    if (hasnames()) {
      trans = form->get_translation(row, translen);
    } else {
      form->extract_header(row);
    }
    freetokens(row);
    row = readtokens(f);
  }
  while (row) {
    if (!*row->token && !row->next) {
      freetokens(row);
      row = readtokens(f);
      continue;
    }
    vec = new intfloat[len];
    if (trans)
      form->insert_input_translated(row, vec, trans, translen);
    else
      form->insert_input(row, vec);
    (*data)[num++] = vec;
    freetokens(row);
    row = readtokens(f);
  }
  if (trans) delete [] trans;
  return 1;
}

int DataObject::save(const char* datafile, int standard)
{
  FILE* f;
  int status;
  if (!data) {
    fprintf(stderr, "No data to save\n");
    return -1;
  }
  if ((f = fopen(datafile, "w")) == 0) {
    fprintf(stderr, "Failed to save data to file \"%s\"\n", datafile);
    return -1;
  }
  if (standard)
    status = DataObject::internal_save(f);
  else
    status = internal_save(f);
  fclose(f);
  return (status ? 0 : -1);
}

int DataObject::internal_save(FILE* f)
{
  intfloat* vec;
  if (hasnames())
    form->save_header(f);
  FORVEC(*data, vec)
    form->save_output(f, vec);
  return 1;
}

char* DataObject::getname(int n)
{
  FormatSpec* f;
  f = form->nth(n);
  if (f)
    return f->name;
  else
    return 0;
}

int DataObject::findname(const char* str)
{
  FormatSpec* f = form->nth(0);
  int i = 0;
  while (f && (!f->name || strcmp(str, f->name)))
    f = f->next, i++;
  return (f ? i : -1);
}

int DataObject::hasnames()
{
  FormatSpec* f;
  f = form->nth(0);
  while (f && !f->name)
    f = f->next;
  return (f ? 1 : 0);
}

static int (*intern_sfunc)(union intfloat* v1, union intfloat* v2);

static int hfunc(const void* p1, const void* p2)
{
  return intern_sfunc(*(intfloat**)p1, *(intfloat**)p2);
}

void DataObject::sort(int (*func)(union intfloat* v1, union intfloat* v2))
{
  intern_sfunc = func;
  qsort(data->vect(), num, sizeof(intfloat*), hfunc);
}
