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

#ifndef TEMPLATE_HASHTABLE

#define TEMPLATE_HASHTABLE

template<class T> class HTEle {
public:
  HTEle(const char* str) { ind = strcpy(new char[strlen(str)+1], str); };
  ~HTEle() { delete [] ind; };
  char* ind;
  T val;
  HTEle<T>* next;
};

template<class T> class HashTable {
public:
  HashTable() {
    num = 0;
    _size = 0;
    _blockresize = 0;
    vec = 0;
    currele = 0;
/*    def = 0; */
    resize(32);
  }; 
  HashTable(T d) {
    num = 0;
    _size = 0;
    _blockresize = 0;
    vec = 0;
    currele = 0;
    def = d;
    resize(32);
  }; 
  ~HashTable() {
    HTEle<T> *ele, *ele2;
    int i;
    for (i=0; i<_size; i++)
      for (ele=vec[i]; ele; ele=ele2) {
        ele2 = ele->next;
        delete ele;
      }
    delete [] vec;
  };
  T& operator()(const char* str) {
    HTEle<T>* ele;
    int h = hash(str);
    for (ele=vec[h]; ele && strcmp(ele->ind, str); ele=ele->next);
    if (!ele)
      ele = add(h, str);
    return ele->val;
  };
  void reset() {
    currele = 0;
    currhind = -1;
    _blockresize = 1;
  };
  int end() {
    if (currele && currele->next)
      currele = currele->next;
    else
      for (currele=0; currhind<_size-1 && !(currele=vec[++currhind]); );
    if (currele)
      return 0;
    else {
      _blockresize = 0;
      return 1;
    }
  };
  T& currval() { return currele->val; }
  char* currind() { return currele->ind; }
  int size() { return num; }
  int operator!() { return (num == 0); }
  operator int() { return (num > 0); }
  void setdefault(T d) { def = d; };
private:
  int hash(const char* str) {
    register int h = 0;
    const unsigned char *p;
    for (p=(const unsigned char*)str; *p; p++) {
      h ^= *p;
      h <<= 1;
      h = (h >> _bits) ^ (h & (_size-1));
    }
    return h;
  }
  HTEle<T>* add(int h, const char* str) {
    HTEle<T>* ele = new HTEle<T>(str);
    ele->val = def;
    ele->next = vec[h];
    vec[h] = ele;
    num++;
    if (!_blockresize && num > _size*4) 
      resize(_size*8);
    return ele;
  };
  void resize(int sz) {
    HTEle<T>** oldvec;
    HTEle<T> *ele, *ele2;
    int hind;
    int i;
    int oldsize;
    oldsize = _size;
    oldvec = vec;
    _size = sz;
    for (_bits=-1; sz; _bits++, sz>>=1);
    vec = new HTEle<T>*[_size];
    num = 0;
    for (i=0; i<_size; i++)
      vec[i] = 0;
    if (oldvec) {
      for (i=0; i<oldsize; i++)
        for (ele=oldvec[i]; ele; ele=ele2) {
          ele2 = ele->next;
          hind = hash(ele->ind);
          ele->next = vec[hind];
          vec[hind] = ele;
          ++num;
        }
      delete [] oldvec;
    }
  };
  int _size;
  int _bits;
  int _blockresize;
  int num;
  T def;
  HTEle<T> *currele;
  int currhind;
  HTEle<T>** vec;
};

#ifndef FORHASH
#define FORHASH( ht, var, str) for( (ht).reset(); !((ht).end())&&((var=(ht).currval()),(str=(ht).currind()),1); )
#endif

#endif
