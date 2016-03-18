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
#ifdef GZLIB
#include <zlib.h>
#endif
#include <string.h>
#include "readtokens.hh"

/*
struct TokenLink {
  char* token;
  TokenLink* next;
};

TokenLink* readtokens(FILE* f);
void freetokens(TokenLink* list);
int length(TokenLink* list);
int is_posint(const char* str);
int is_cont(const char* str);
int is_unknown(const char* str);
int get_posint(const char* str);
float get_cont(const char* str);
*/

static int buflen = 0;

static char* buf = 0;

static char* pos;

static char curr = 0;

static TokenLink* worklist;

const char* global_unknown_tag = 0;

char global_separator_char = 0;

char selected_separator = '\t';

#ifdef GZLIB
FILE *gz_file;
gzFile gz_gzfile;

static int xgetc(FILE *f)
{
  int c;

  if (f == gz_file)
    return gzgetc(gz_gzfile);
  else {
    c = getc(f);
    return c;
  }
}

static char *xgets(char *buf1, int siz, FILE *f)
{
  int c;

  if (f == gz_file)
    return gzgets(gz_gzfile, buf1, siz);
  else {
    return fgets(buf1, siz, f);
  }
}

static int xfeof(FILE *f)
{
  if (f == gz_file)
    return gzeof(gz_gzfile);
  else
    return feof(f);
}
#else
#define xgetc fgetc
#define xgets fgets
#define xfeof feof
#endif

static void resetbuf(FILE* f)
{
  if (!buf) {
    buflen = 1024;
    buf = new char[buflen];
  }
  pos = buf;
  curr = xgetc(f);
  if (xfeof(f)) curr = 0;
}

char* freezebuf()
{
  char* tmp = buf;
  buf = 0;
  buflen = 0;
  return tmp;
}

static void growbuf(char*& s1, char*& s2)
{
  int i;
  char *p1, *p2;
  TokenLink* l;
  int nlen = buflen*2;
  char* nbuf = new char[nlen];
  for (i=0, p1=buf, p2=nbuf; i<buflen; *p2++=*p1++, i++);
  for (l=worklist; l; l=l->next)
    l->token = nbuf + (l->token-buf);
  s1 = nbuf + (s1-buf);
  s2 = nbuf + (s2-buf);
  delete [] buf;
  buf = nbuf;
  buflen = nlen;
}

void set_selected_separator(char sep) {
	 selected_separator = sep;
}

static char get_selected_separator()
{
    return selected_separator;
}

static int readsep(FILE* f, int first)
{
  int tres = first;

  if (first) {
    // kludge fix: if first character is a blank, we use
    // global_separator_char = 0, otherwise Tab.
    // space-separated fields where the line doesn't begin with blank
    // don't work.
    if (curr != ' ')
      global_separator_char = get_selected_separator();
    else
      global_separator_char = 0;
  }

  while (global_separator_char ? 
         (!tres && curr == global_separator_char) : 
         (curr == ' ' || (!tres && (curr == ',' || curr == ';' || curr == '\t')))) {
    if (curr != ' ')
      tres = 1;
    curr = xgetc(f);
    if (xfeof(f)) curr = 0;
  }
  if (curr == '\r') {
    curr = xgetc(f);
    if (xfeof(f)) curr = 0;
  }
  if (curr == 0 && (first || !tres))
    return 1;
  else if (curr == '\n' && !tres)
    return 1;
  else
    return 0;
}

static char* readtoken(FILE* f)
{
  char* str;
  str = pos;

#if 0
  // Comments start with # and make up the rest of the line
  while (curr == '#') {
    for (;;) {
      curr = getc(f);
      if (curr == '\n') {
	curr = getc(f);
	if (feof(f))
	  curr = 0;
	break;
      }
    }
  }
#endif

  while (global_separator_char ? 
         (curr != 0 && curr != '\n' && curr != '\r' && curr != global_separator_char) :
         (curr != 0 && curr != ' ' && curr != ',' && curr != ';' && curr != '\t' && curr != '\n' && curr != '\r')) {
    *pos++ = curr;
    if (pos >= buf+buflen)
      growbuf(pos, str);
    curr = xgetc(f);
    if (xfeof(f)) curr = 0;
  }
  *pos++ = 0;
  if (pos >= buf+buflen)
    growbuf(pos, str);
  return str;
}

TokenLink* readtokens(FILE* f)
{
  char* tok;
  TokenLink* link;
  TokenLink* list = 0;
  TokenLink* last = 0;
  worklist = 0;
  resetbuf(f);
  if (readsep(f, 1))
    return 0;
  do {
    tok = readtoken(f);
    link = new TokenLink;
    link->token = tok;
    link->next = 0;
    if (!last)
      worklist = last = list = link;
    else
      last = last->next = link;
  } while (!readsep(f, 0));
  return list;
}

void freetokens(TokenLink* list)
{
  TokenLink* link = list;
  while (link) {
    list = link->next;
    delete link;
    link = list;
  }
}
  
int length(TokenLink* list)
{
  int len = 0;
  while (list) {
    list = list->next;
    len++;
  }
  return len;
}

int is_posint(const char* str)
{
  if (!*str) return 0;
  while (*str >= '0' && *str <= '9') str++;
  return (!*str);
}

int is_negint(const char* str)
{
  if (!*str) return 0;
  if (*str == '-') str++;
  else return 0;
  if (!*str) return 0;
  while (*str >= '0' && *str <= '9') str++;
  return (!*str);
}

int is_cont(const char* str)
{
  if (!*str) return 0;
  if (*str == '-') str++;
  if (!*str) return 0;
  while (*str >= '0' && *str <= '9') str++;
  if (*str == '.') str++;
  while (*str >= '0' && *str <= '9') str++;
  if (*str == 'e' || *str == 'E') {
    str++;
    if (*str == '-' || *str == '+') str++;
    while (*str >= '0' && *str <= '9') str++;
  }
  return (!*str);
}

int is_unknown(const char* str)
{
  if (!*str || 
      (global_unknown_tag ? !strcmp(str, global_unknown_tag) :
       ((*str == '*' || *str == '-' || *str == '?') && !*(str+1))))
    return 1;
  else
    return 0;
}

IF_INT get_posint(const char* str)
{
  long long res;
  if (sscanf(str, "%lld", &res) == 1)
    return res;
  else
    return -1;
}

IF_FLOAT get_cont(const char* str)
{
  static IF_INT i = -1;
  double res;
  if (sscanf(str, "%lf", &res) == 1)
    return res;
  else
    return *(IF_FLOAT*)&i;
}
