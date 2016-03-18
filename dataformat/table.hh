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





