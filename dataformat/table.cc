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

#include <string.h>
#include "table.hh"

/*
struct IndexTableSlot {
  char* key;
  int index;
  IndexTableSlot* next;
  IndexTableSlot* hnext;
};

class IndexTable {
public:
  IndexTable();
  ~IndexTable();
  IndexTable* copy();
  void insert(const char* str);
  void insert_last(const char* str);
  void sort(int (*func)(const char*, const char*), int off = 0);
  int get(const char* str);
  char* nth(int n);
  void reset() { curr = first; };
  char* next() { if (curr) {char* str=curr->key; curr=curr->next; return str;} return 0; };
  int length() { return num; };
protected:
  int hash(const char* str);
  IndexTableSlot* lookup(const char* str, int insert, int resort);
  void resize(int n);
  void inlast(IndexTableSlot* ele);
  void sortin(int (*func)(const char*, const char*), IndexTableSlot* ele, int reindex);
  int size;
  int bsize;
  int num;
  int offset;
  int (*sortfunc)(const char*, const char*);
  IndexTableSlot** table;
  IndexTableSlot* first;
  IndexTableSlot* last;
  IndexTableSlot* curr;
};

#define FORTABLE( tab, key) for((tab).reset(); (key=(tab).next()); )

*/

IndexTable::IndexTable()
{
  int i;
  IndexTableSlot** slot;
  size = 64;
  bsize = 6;
  num = 0;
  offset = 0;
  sortfunc = strcmp;
  table = new IndexTableSlot*[size];
  for (i=0, slot=table; i<size; i++, slot++) *slot = 0;
  first = 0;
  last = 0;
  curr = 0;
}

IndexTable::~IndexTable()
{
  IndexTableSlot *ele, *ele2;
  int i;
  for (i=0; i<size; i++)
    for (ele=table[i]; ele; ele=ele2) {
      ele2 = ele->hnext;
      delete [] ele->key;
      delete ele;
    }
  delete [] table;
}

IndexTable* IndexTable::copy()
{
  IndexTable* tab = new IndexTable();
  char* str;
  FORTABLE(*this, str) 
    tab->insert_last(str);
  return tab;
}

int IndexTable::hash(const char* str)
{
  int work = 0;
  unsigned char *p;
  for (p=(unsigned char*)str; *p; p++) {
    work ^= *p;
    work <<= 1;
    work = (work >> bsize) ^ (work & (size-1));
  }
  return work;
}

IndexTableSlot* IndexTable::lookup(const char* str, int ins, int resort)
{
  int index;
  IndexTableSlot *ele;
  index = hash(str);
  for (ele=table[index]; ele && strcmp(str, ele->key); ele = ele->hnext);
  if (ele)
    return ele;
  else if (ins) {
    ele = new IndexTableSlot;
    ele->key = strcpy(new char[strlen(str)+1], str);
    ele->hnext = table[index];
    if (resort)
      sortin(sortfunc, ele, 1);
    else
      inlast(ele);
    table[index] = ele;
    ++num;
    if (num > size * 4) {
      resize(size * 8);
    }
    return ele;
  }
  else
    return 0;
}

void IndexTable::resize(int n)
{
  IndexTableSlot** oldtable;
  IndexTableSlot *ele, *ele2;
  int index;
  int i;
  int oldsize;
  oldsize = size;
  size = n;
  for (bsize=0; n; bsize++, n>>=1);
  oldtable = table;
  table = new IndexTableSlot*[size];
  num = 0;
  for (i=0; i<size; i++)
    table[i] = 0;
  for (i=0; i<oldsize; i++)
    for (ele=oldtable[i]; ele; ele=ele2) {
      ele2 = ele->hnext;
      index = hash(ele->key);
      ele->hnext = table[index];
      table[index] = ele;
      ++num;
    }
  delete [] oldtable;
}

void IndexTable::insert(const char* str)
{
  if (str)
    lookup(str, 1, 1);
}

void IndexTable::insert_last(const char* str)
{
  if (str)
    lookup(str, 1, 0);
}

char* IndexTable::nth(int n)
{
  IndexTableSlot* ele;
  int i;
  n -= offset;
  if (n<num && n>=0) {
    ele = first;
    for (i=0; i<n; i++, ele=ele->next);
    return ele->key;
  } else
    return 0;
}

int IndexTable::get(const char* str)
{
  IndexTableSlot* ele;
  if (str && (ele = lookup(str, 0, 0)))
    return ele->index;
  else
    return -1;
}

void IndexTable::inlast(IndexTableSlot* ele)
{
  if (!last)
    first = last = ele;
  else {
    last->next = ele;
    last = ele;
  }
  ele->next = 0;
  ele->index = num;
}

void IndexTable::sortin(int (*func)(const char*, const char*), IndexTableSlot* ele, int reindex)
{
  IndexTableSlot* ele2;
  int ind;
  // Bah, away with efficiency
  if (!first || func(ele->key, first->key) <= 0) {
    ele->next = first;
    first = ele;
    if (!last)
      last = first;
    if (reindex)
      ele->index = offset;
  }
  else {
    for (ele2=first; ele2->next && (func(ele->key, ele2->next->key) > 0);
         ele2 = ele2->next);
    ele->next = ele2->next;
    ele2->next = ele;
    if (!ele->next)
      last = ele;
    if (reindex)
      ele->index = ele2->index + 1;
  }
  if (reindex)
    for (ele2=ele->next, ind=ele->index+1; ele2; ele2=ele2->next, ind++)
      ele2->index = ind;
}

void IndexTable::sort(int (*func)(const char*, const char*), int off)
{
  IndexTableSlot *ele;
  int i, ind;
  first = 0;
  offset = off;
  sortfunc = func;
  for (i=0; i<size; i++)
    for (ele=table[i]; ele; ele=ele->hnext)
      sortin(func, ele, 0);
    for (ele=first, ind=off; ele; ele=ele->next, ind++)
      ele->index = ind;
}

