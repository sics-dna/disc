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

#ifndef TEMPLATE_SORTEDHASHTABLE

#define TEMPLATE_SORTEDHASHTABLE

template<class T> class SHTEle {
public:
  SHTEle(const char* str) { ind = strcpy(new char[strlen(str)+1], str); };
  ~SHTEle() { delete [] ind; };
  char* ind;
  T val;
  SHTEle<T>* next;
  SHTEle<T>* hnext;
};

template<class T> class SortedHashTable {
public:
  SortedHashTable() {
    num = 0;
    _size = 0;
    _blockresize = 0;
    vec = 0;
    first = 0;
    currele = 0;
/*    def = 0; */
    resize(32);
  }; 
  SortedHashTable(T d) {
    num = 0;
    _size = 0;
    _blockresize = 0;
    vec = 0;
    first = 0;
    currele = 0;
    def = d;
    resize(32);
  };
  ~SortedHashTable() {
    SHTEle<T> *ele, *ele2;
    int i;
    for (i=0; i<_size; i++)
      for (ele=vec[i]; ele; ele=ele2) {
        ele2 = ele->hnext;
        delete ele;
      }
    delete [] vec;
  };
  T& operator()(const char* str) {
    SHTEle<T>* ele;
    int h = hash(str);
    for (ele=vec[h]; ele && strcmp(ele->ind, str); ele=ele->hnext);
    if (!ele)
      ele = add(h, str);
    return ele->val;
  };
  void reset() {
    currele = 0;
    _blockresize = 1;
  };
  int end() {
    if (currele)
      currele = currele->next;
    else
      currele = first;
    if (currele) {
      return 0;
    } else {
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
  SHTEle<T>* add(int h, const char* str) {
    SHTEle<T>* ele = new SHTEle<T>(str);
    SHTEle<T> *p, *p2;
    ele->val = def;
    ele->hnext = vec[h];
    vec[h] = ele;
    for (p=first, p2=0; p && strcmp(str,p->ind)>0; p2=p, p=p->next); // linear search...
    if (!p2)
      ele->next = first, first = ele;
    else
      ele->next = p, p2->next = ele;
    num++;
    if (!_blockresize && num > _size*4) 
      resize(_size*8);
    return ele;
  };
  void resize(int sz) {
    SHTEle<T>** oldvec;
    SHTEle<T> *ele, *ele2;
    int hind;
    int i;
    int oldsize;
    oldsize = _size;
    oldvec = vec;
    _size = sz;
    for (_bits=-1; sz; _bits++, sz>>=1);
    vec = new SHTEle<T>*[_size];
    num = 0;
    for (i=0; i<_size; i++)
      vec[i] = 0;
    if (oldvec) {
      for (i=0; i<oldsize; i++)
        for (ele=oldvec[i]; ele; ele=ele2) {
          ele2 = ele->hnext;
          hind = hash(ele->ind);
          ele->hnext = vec[hind];
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
  SHTEle<T> *currele;
  SHTEle<T> *first;
  SHTEle<T>** vec;
};

#ifndef FORHASH
#define FORHASH( ht, var, str) for( (ht).reset(); !((ht).end())&&((var=(ht).currval()),(str=(ht).currind()),1); )
#endif

#endif
