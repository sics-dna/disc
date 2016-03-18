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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "readtokens.hh"
#include "table.hh"
#include "format.hh"
#include "formattime.hh"
#include "formatunknown.hh"
#include "formattime.hh"

/*

 // date: dateform  -- days from 2000-01-01 inklusive
 // time: timeform  -- seconds since midnight
 // datetime: datetimeform firstdatetime [interval]  -- intervals since firstdatetime
 //  typical timeforms  h:m:s.c  h:m  [AM/PM] (sommartid?)
 //  typical dateforms  Yy-m-d  m/d/Yy  d-m-Y  d M Yy
 // Förenklad variant utan argument, endast de två första av varje, och endast
 // från 2000-01-01
 // date/time/datetime: first last

class FormatSpecDatetime : public FormatSpec {
public:
  FormatSpecDatetime(const char* nm = 0) : FormatSpec(nm) { dtform = 0; first.i = last.i = -1; };
  virtual FormatSpec* copy() { FormatSpecDatetime* fs = new FormatSpecDatetime(name); fs->dtform = dtform; fs->first = first; fs->last = last; fs->unset = unset; return fs; };
  virtual void init(struct TokenLink* tok);
  virtual void add(const char* str);
  virtual void save(FILE* f);
  virtual intfloat interpret(const char* str);
  virtual const char* represent(intfloat v);
  virtual void get_time(int val, int& h, int& mi, int& s);
  virtual void get_date(int val, int& y, int& mo, int& d);
  virtual void usurp(FormatSpecUnknown* ele);
  virtual int type() { return 5; };
//  char* timeform;
//  int offset;
  int dtform;
  intfloat first, last;
};

extern int check_datetime(const char* str);

*/

static int chunk_svdate(const char* str, int& y, int& m, int& d, const char*& q)
{
  const char* p = str;
  q = str;
  if (*p == '2' && *(p+1) == '0' && *(p+2) >= '0' && *(p+2) <= '9') p += 2;
  if (*p < '0' || *p > '9' || *(p+1) < '0' || *(p+1) > '9')
    return 0;
  y = (*p - '0')*10 + (*(p+1) - '0');
  p +=2;
  if (*p == '-') p++;
  if (*p < '0' || *p > '9' || *(p+1) < '0' || *(p+1) > '9')
    return 0;
  m = (*p - '0')*10 + (*(p+1) - '0');
  if (m == 0 || m > 12)
    return 0;
  p +=2;
  if (*p == '-') p++;
  if (*p < '0' || *p > '9' || *(p+1) < '0' || *(p+1) > '9')
    return 0;
  d = (*p - '0')*10 + (*(p+1) - '0');
  if (d == 0 || d > 31)
    return 0;
  p +=2;
  if (*p && *p != ' ')
    return 0;
  while (*p == ' ') p++;
  q = p;
  return 1;
}

static int chunk_usdate(const char* str, int& y, int& m, int& d, const char*& q)
{
  const char* p = str;
  q = str;
  if (*p < '0' || *p > '9')
    return 0;
  if (*(p+1) < '0' || *(p+1) > '9') {
    m = (*p - '0');
    p += 1;
  } else {
    m = (*p - '0')*10 + (*(p+1) - '0');
    p += 2;
  }
  if (m == 0 || m > 12)
    return 0;
  if (*p != '/')
    return 0;
  p++;
  if (*p < '0' || *p > '9')
    return 0;
  if (*(p+1) < '0' || *(p+1) > '9') {
    d = (*p - '0');
    p += 1;
  } else {
    d = (*p - '0')*10 + (*(p+1) - '0');
    p += 2;
  }
  if (d == 0 || d > 31)
    return 0;
  if (*p != '/')
    return 0;
  p++;
  if (*p < '0' || *p > '9')
    return 0;
  if (*(p+1) < '0' || *(p+1) > '9') {
    y = (*p - '0');
    p += 1;
  } else {
    if (*p == '2' && *(p+1) == '0' && *(p+2) >= '0' && *(p+2) <= '9') p += 2;
    y = (*p - '0')*10 + (*(p+1) - '0');
    p += 2;
  }
  if (*p && *p != ' ')
    return 0;
  while (*p == ' ') p++;
  q = p;
  return 1;
}

static int chunk_time(const char* str, int& h, int& m, int& s, const char*& q)
{
  const char* p = str;
  q = str;
  if (*p < '0' || *p > '9')
    return 0;
  if (*(p+1) < '0' || *(p+1) > '9') {
    h = (*p - '0');
    p += 1;
  } else {
    h = (*p - '0')*10 + (*(p+1) - '0');
    p += 2;
  }
  if (h > 23)
    return 0;
  if (*p != ':')
    return 0;
  p++;
  if (*p < '0' || *p > '9')
    return 0;
  if (*(p+1) < '0' || *(p+1) > '9') {
    m = (*p - '0');
    p += 1;
  } else {
    m = (*p - '0')*10 + (*(p+1) - '0');
    p += 2;
  }
  if (m > 59)
    return 0;
  if (!*p || *p == ' ') {
    s = 0;
    while (*p == ' ') p++;
    q = p;
    return 1;
  }
  if (*p != ':')
    return 0;
  p++;
  if (*p < '0' || *p > '9')
    return 0;
  if (*(p+1) < '0' || *(p+1) > '9') {
    s = (*p - '0');
    p += 1;
  } else {
    s = (*p - '0')*10 + (*(p+1) - '0');
    p += 2;
  }
  if (*p == '.') {
    p += 1;
    while (*p >= '0' && *p <= '9') p++;
  }
  if (*p && *p != ' ')
    return 0;
  while (*p == ' ') p++;
  q = p;
  return 1;
}

static int chunk_seconds(const char* str, int& has_ms, int& sec, int& ms)
{
  int i;
  long s;
  const char* p = str;
  if (*p != '1')
    return 0;
  for (i=0; i<15 && *p >= '0' && *p <= '9'; i++, p++);
  if (*p || (i != 10 && i != 13))
    return 0;
  s = atol(str);
  if (i == 13) {
    has_ms = 1;
    ms = s%1000;
    sec = s/1000;
  } else {
    has_ms = 0;
    ms = 0;
    sec = s;
  }
  return 1;
}

static char* string_svdate(char* buf, int y, int m, int d)
{
  int l;
  l = sprintf(buf, "20%2.2d-%2.2d-%2.2d", y, m, d);
  if (l>0)
    return buf+l;
  else
    return 0;
}

static char* string_usdate(char* buf, int y, int m, int d)
{
  int l;
  l = sprintf(buf, "%2.2d/%2.2d/20%2.2d", m, d, y);
  if (l>0)
    return buf+l;
  else
    return 0;
}

static char* string_time(char* buf, int h, int m, int s)
{
  int l;
  l = sprintf(buf, "%d:%2.2d:%2.2d", h, m, s);
  if (l>0)
    return buf+l;
  else
    return 0;
}

int check_datetime(const char* str)
{
  const char* p = str;
  int d1, d2, d3, fmt=0;
  if (chunk_usdate(p, d1, d2, d3, p))
    fmt |= 4;
  else if (chunk_svdate(p, d1, d2, d3, p))
    fmt |= 2;
  if (chunk_time(p, d1, d2, d3, p))
    fmt |= 1;
  if (!fmt && chunk_seconds(p, d1, d2, d3))
    fmt |= (d1 ? 16 : 8);
  return fmt;
}

static int check_datetime(const char*str, int fmt)
{
  const char* p = str;
  int d1, d2, d3, fmt2=0;
  if ((fmt&4) && chunk_usdate(p, d1, d2, d3, p))
    fmt2 |= 4;
  else if ((fmt&2) && chunk_svdate(p, d1, d2, d3, p))
    fmt2 |= 2;
  if ((fmt&1) && chunk_time(p, d1, d2, d3, p))
    fmt2 |= 1;
  if (!fmt2 && (fmt&24) && chunk_seconds(p, d1, d2, d3))
    fmt2 = fmt & (d1 ? 16 : 8);
  return fmt2;
}

void FormatSpecDatetime::init(struct TokenLink* tok)
{
  int fmt1, fmt2;
  name = stripname(tok);
  if (!strcmp(tok->token, "date:"))
    dtform = 6;
  else if (!strcmp(tok->token, "time:"))
    dtform = 1;
  else if (!strcmp(tok->token, "seconds:"))
    dtform = 8;
  else if (!strcmp(tok->token, "millisec:"))
    dtform = 16;
  else 
    dtform = 7;
  tok = tok->next;
  if (!tok)
    ;
  else if (!tok->next) {
    fprintf(stderr, "Too few %s format arguments: %s\n",
              (dtform&6 ? (dtform&1 ? "datetime" : "date") : dtform&24 ? (dtform&8 ? "seconds" : "millisec") : "time"),
            tok->token);
  }
  else if (!tok->next->next) {
    fmt1 = check_datetime(tok->token, dtform);
    fmt2 = check_datetime(tok->next->token, dtform);
    if (fmt1 == fmt2 && (!(dtform&1) || (fmt1&1)) && (!(dtform&6) || (fmt1&6))) {
      dtform = fmt1;
      unset = 0;
      first = interpret(tok->token);
      last = interpret(tok->next->token);
    } else
      fprintf(stderr, "Strange %s format arguments: %s %s\n",
              (dtform&6 ? (dtform&1 ? "datetime" : "date") : dtform&24 ? (dtform&8 ? "seconds" : "millisec") : "time"),
              tok->token, tok->next->token);
  }
  else {
    fprintf(stderr, "Too many %s format arguments: %s %s %s ...\n",
              (dtform&6 ? (dtform&1 ? "datetime" : "date") : dtform&24 ? (dtform&8 ? "seconds" : "millisec") : "time"),
            tok->token, tok->next->token, tok->next->next->token);
  }
}

void FormatSpecDatetime::save(FILE* f)
{
  const char* tp = (dtform&6 ? (dtform&1 ? "datetime" : "date") : dtform&24 ? (dtform&8 ? "seconds" : "millisec") : "time");
  printname(f);
  if (unset || first.i == -1 || last.i == -1)
    fprintf(f, "%s:\n", tp);
  else {
    fprintf(f, "%s:\t%s", tp, represent(first));
    fprintf(f, "\t%s\n", represent(last));
  }
}

void FormatSpecDatetime::add(const char* str)
{
  intfloat val;
  if (is_unknown(str)) 
    return;
  val = interpret(str);
  if (val.i != -1) {
    if (val.i < first.i || first.i == -1)
      first = val;
    if (val.i > last.i)
      last = val;
  }
}

intfloat FormatSpecDatetime::interpret(const char* str)
{
  intfloat res;
  int fmt, ok=1;
  int y, mo, d, h, mi, s, dnum, sec, ms, has_ms;
  int mv1[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
  int mv2[] = {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335};
  if (is_unknown(str)) {
    res.i = -1;
    return res;
  }
  if (unset) {
    fmt = check_datetime(str, dtform);
    if (fmt && (!(dtform&1) || (fmt&1)) && (!(dtform&6) || (fmt&6))) {
      dtform = fmt;
      unset = 0;
    }
  }
  if (dtform&24) {
    ok = chunk_seconds(str, has_ms, sec, ms);
    if (ok)
      res.i = sec + (has_ms && ms>500 ? 1 : 0);
  } else {
    if (dtform&6) {
      if (dtform&4)
        ok = chunk_usdate(str, y, mo, d, str);
      else
        ok = chunk_svdate(str, y, mo, d, str);
      if (ok)
        dnum = d + (y % 4 ? mv1[mo-1] : mv2[mo-1]) + y*365 + (y+3)/4 - 1;
    }
    if (ok && (dtform&1)) {
      ok = chunk_time(str, h, mi, s, str);
      if( ok)
        sec = h*3600 + mi*60 + s;
    }
    if (ok)
      res.i = (dtform&6 ? (dtform&1 ? dnum*86400 + sec : dnum) : sec);
  }
  if (!ok) {
    res.i = -1;
    fprintf(stderr, "Bad %s value: %s\n",
            (dtform&6 ? (dtform&1 ? "datetime" : "date") : dtform&24 ? (dtform&8 ? "seconds" : "millisec") : "time"),
            str);
  }
  return res;
}

const char* FormatSpecDatetime::represent(intfloat v)
{
  static char buf[20];
  char* p = buf;
  int y, mo, d, dnum, sec;
  int mv1[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
  int mv2[] = {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335};
  if (v.i == -1)
    return "?";
  if (dtform&24) {
    if (dtform&16)
      sprintf(buf, "%lld000", (long long)v.i); // Not really milliseconds yet, fix it!!!
    else
      sprintf(buf, "%d", (int)v.i);
  } else {
    if (dtform&6) {
      if (dtform&1) {
        dnum = v.i/86400;
      } else
        dnum = v.i;
      for (y=0; dnum >= (y%4 ? 365 : 366); dnum-=(y%4 ? 365 : 366), y++);
      for (mo=12; dnum < (y%4 ? mv1 : mv2)[mo-1]; mo--);
      dnum -= (y%4 ? mv1 : mv2)[mo-1];
      d = dnum+1;
      if (dtform&2)
        p = string_svdate(p, y, mo, d);
      else
        p = string_usdate(p, y, mo, d);
    }
    if (dtform&1) {
      if (dtform&6)
        *p++ = ' ';
      sec = v.i % 86400;
      string_time(p, sec/3600, (sec/60)%60, sec%60);
    }
  }
  return buf;
}

void FormatSpecDatetime::get_time(int val, int& h, int& mi, int& s)
{
  int sec;
  if (!(dtform&1)) {
    h=mi=s=0;
  } else {
    sec = val % 86400;
    h = sec/3600, mi = (sec/60)%60, s = sec%60;
  }
}

void FormatSpecDatetime::get_date(int val, int& y, int& mo, int& d)
{
  int dnum;
  int mv1[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
  int mv2[] = {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335};
  if (!(dtform&6)) {
    y=mo=d=0;
  } else {
    if (dtform&1)
      dnum = val/86400;
    else
      dnum = val;
    for (y=0; dnum >= (y%4 ? 365 : 366); dnum-=(y%4 ? 365 : 366), y++);
    for (mo=12; dnum < (y%4 ? mv1 : mv2)[mo-1]; mo--);
    dnum -= (y%4 ? mv1 : mv2)[mo-1];
    d = dnum+1;
  }
}

void FormatSpecDatetime::usurp(FormatSpecUnknown* ele)
{
  char* str;
  int fmt;
  if (!ele->unset) {
    ele->table->reset();
    if (!dtform) {
      str = ele->table->next();
      if (str) {
        fmt = check_datetime(str);
        if (fmt) {
          dtform = fmt;
          add(str);
        }
      }
    }
    while ((str = ele->table->next()))
      add(str);
  }
}

//  Läs in formatfil:
//    ele = format_create(row);
//    ele->init(row);
//  dtform kan vara 6 (och unset), men inte 0

//  simple_construct:
//    ele = format_create(*c);
//  dtform är 0...

//  construct:
//  internal_construct
//    ele = format_guess(row->token, 1);
//    ele->add(row->token);
//  internal_add
//    if (IsType(ele, FormatSpecUnknown))
//      if ((ele2 = format_guess(row->token, 0))) {
//        ele2->usurp((FormatSpecUnknown*) ele);
//    ele2->add(row->token);
//  internal_guess
//    if (IsType(ele, FormatSpecUnknown)) {
//      if ((ele2 = format_guess_unknown((FormatSpecUnknown*) ele))) {
//        ele2->usurp((FormatSpecUnknown*) ele);
//  
