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

#ifdef __APPLE__
#include "TargetConditionals.h"
#if TARGET_OS_MAC
/* [PD] off64_t is not defined in BSD, and thus not in darwin (Mac OS X)
        either. We can use off_t instead since it is 64-bit in BSD. */
#define off64_t off_t
/* [PD] O_LARGEFILE is not defined in darwin (Mac OS X), and apparently
        not necessary. */
#define O_LARGEFILE 0
#endif
#endif
#ifdef PLATFORM_MSW // [PD] Tentatively only check for WIN32.
                    //      I don't know what to do (if anything) on WIN64.
                    // [Arndt] _WIN32 doesn't work on cygwin; use our own
/* [PD] O_LARGEFILE is apparently not necessary on MS Windows. */
#define O_LARGEFILE 0
#ifdef _MSC_VER // Microsoft Visual C++
#include <sys/types.h> // off_t
#endif
#define off64_t off_t
#endif

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
