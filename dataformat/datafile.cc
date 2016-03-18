 /*
 --------------------------------------------------------------------------
 Copyright (C) 2012, 2015 Anders Holst <aho@sics.se> 

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

#ifndef _MSC_VER // Microsoft Visual C++
#include <unistd.h>
#endif
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include "readtokens.hh"
#include "format.hh"
#include "data.hh"
#include "datafile.hh"

#ifdef _MSC_VER // Microsoft Visual C++
#include <io.h>
#define open _open
#define fdopen _fdopen
#endif

/*
class DataFileObject : public DataObject {
public:
  DataFileObject(class Format* fr, const char* datafile) : DataObject(fr) { init(datafile); };
  DataFileObject(const char* formatfile, const char* datafile) : DataObject(formatfile) { init(datafile); };
  void init(const char* datafile);
  virtual ~DataFileObject();
  virtual void clear();
  virtual void remove(int ind, int n = 1) {};             // Finns ej
  virtual union intfloat* newentry() { return 0; };       // Finns ej
  virtual int read(const char* datafile, int append = 0);
  virtual int save(const char* datafile, int standard = 0) { return -1; };  // Finns ej   
  virtual void sort(int (*func)(union intfloat* v1, union intfloat* v2)) {};  // Inte än...
  virtual union intfloat* operator[](int i);
protected:
  virtual int read_start(const char* datafile);
  virtual void read_stop();
  virtual int read_one(off64_t pos, intfloat* vec);
  class Growable<off64_t> *datapos;
  off64_t lastpos;
  int cachesize;
  int* cacheind;
  union intfloat** cachevec;
  int currind;
  int* trans;
  int translen;
  int fd;
  FILE* fp;
  int complete;
};
*/

void DataFileObject::init(const char* datafile)
{
  int i;
  datapos = new Growable<off64_t>(0, 1048576);
  cachesize = 8;
  cacheind = new int[cachesize];
  cachevec = new intfloat*[cachesize];
  for (i=0; i<cachesize; i++) {
    cacheind[i] = -1;
    cachevec[i] = new intfloat[len];
  }
  currind = -1;
  trans = 0;
  translen = 0;
  fd = -1;
  fp = 0;
  complete = 1;
  read_start(datafile);
}

DataFileObject::~DataFileObject()
{
  int i;
  if (!complete)
    read_stop();
  delete datapos;
  delete [] cacheind;
  for (i=0; i<cachesize; i++)
    delete [] cachevec[i];
  delete [] cachevec;
  if (trans)
    delete [] trans;
}

void DataFileObject::clear()
{
  int i;
  if (num) {
    delete datapos;
    datapos = new Growable<off64_t>(0, 1048576);
  }
  for (i=0; i<cachesize; i++) {
    if (cachevec[i])
      delete [] cachevec[i];
    cachevec[i] = new intfloat[len];
    cacheind[i] = -1;
  }
  currind = -1;
}

int DataFileObject::read(const char* datafile, int append)
{
  int status;
  clear(); // Ignore append - always clear
  if (!complete)
    read_stop();
  status = read_start(datafile);
  return (status ? num : -1);
}

int DataFileObject::read_start(const char* datafile)
{
  TokenLink* row;
  if ((fd = open(datafile, O_RDONLY | O_LARGEFILE)) == -1) {
    fprintf(stderr, "Failed to open data file \"%s\"\n", datafile);
    return -1;
  }
  fp = fdopen(fd, "r"); // Nu har vi samma fil både som en fd och en FILE*, båda behövs
  row = readtokens(fp);
  if (form->check_header(row)) {
    if (hasnames()) {
      trans = form->get_translation(row, translen);
    } else {
      form->extract_header(row);
    }
    freetokens(row);
  } else { // Ta hand om den oavsiktligt lästa dataraden
    currind = (currind+1)%cachesize;
    form->insert_input(row, cachevec[currind]);
    (*datapos)[0] = 0;
    num++;
    freetokens(row);
  }
  lastpos = ftell(fp);
  num++;
  complete = 0;
  return 1;
}

void DataFileObject::read_stop()
{
  if (!complete) {
    num -= 1;
    fclose(fp);
    complete = 1;
  }
}

int DataFileObject::read_one(off64_t pos, intfloat* vec)
{
  TokenLink* row;
  fseek(fp, pos, SEEK_SET);
  row = readtokens(fp);
  if (!row || (!*row->token && !row->next)) {
    if (row)
      freetokens(row);
    if (feof(fp)) {
      clearerr(fp);
      return -1;
    } else
      return 0;
  }
  if (ferror(fp)) {
    freetokens(row);
    return -1;
  }
  if (trans)
    form->insert_input_translated(row, vec, trans, translen);
  else
    form->insert_input(row, vec);
  freetokens(row);
  return 1;
}

intfloat* DataFileObject::operator[](int i)
{
  TokenLink* row;
  int j;
  if (i<0)
    return 0;
  if (bl >= 0 && bl <= i)
    i += 1;
  if (complete && i >= num)
    return 0;
  if (!complete && i >= num-1) {
    fseek(fp, lastpos, SEEK_SET);
    row = 0;
    for (j=num-1; j<=i; j++) {
      if (row)
        freetokens(row);
      row = readtokens(fp);
      while (!row || (!*row->token && !row->next)) {
        if (row)
          freetokens(row);
        if (feof(fp) || ferror(fp)) {
          clearerr(fp);
          num = j;
          complete = 1;
          return 0;
        }
        lastpos = ftell(fp);
        row = readtokens(fp);
      }
      (*datapos)[j] = lastpos;
      lastpos = ftell(fp);
    }
    currind = (currind+1)%cachesize;
    if (trans)
      form->insert_input_translated(row, cachevec[currind], trans, translen);
    else
      form->insert_input(row, cachevec[currind]);
    cacheind[currind] = i;
    num = i+2;
    freetokens(row);
    return cachevec[currind];
  } else {
    for (j=currind; j>=0; j--)
      if (cacheind[j] == i)
        return cachevec[j];
    for (j=cachesize-1; j>currind; j--)
      if (cacheind[j] == i)
        return cachevec[j];
    currind = (currind+1)%cachesize;
    read_one((*datapos)[i], cachevec[currind]);
    cacheind[currind] = i;
    return cachevec[currind];
  }
}


// Repetition
//   När filen öppnas, kolla headrar och sätt trans, om en rad läst stoppa in i cache
//   När man accessar en rad:
//     om den finns i cache, returnera
//     om den finns i tabellen, använd read_one som hanterar cachen
//     om inte complete, läs fram till rätt index, ha koll på filslut och sätt complete
//     annars skicka tillbaka 0

