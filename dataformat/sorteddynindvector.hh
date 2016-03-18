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

#ifndef TEMPLATE_SORTEDDYNINDVECTOR

#define TEMPLATE_SORTEDDYNINDVECTOR

template<class T> class SDIVecEle {
public:
  int ind;
  T val;
  SDIVecEle<T>* next;
  SDIVecEle<T>* hnext;
};

template<class T> class SortedDynamicIndexVector {
public:
  SortedDynamicIndexVector() {
    num = 0;
    _size = 0;
    _blockresize = 0;
    vec = 0;
    first = 0;
    currele = 0;
//    def = 0;
    resize(32);
  }; 
  SortedDynamicIndexVector(const T d) {
    num = 0;
    _size = 0;
    _blockresize = 0;
    vec = 0;
    first = 0;
    currele = 0;
    def = d;
    resize(32);
  }; 
  ~SortedDynamicIndexVector() {
    SDIVecEle<T> *ele, *ele2;
    int i;
    for (i=0; i<_size; i++)
      for (ele=vec[i]; ele; ele=ele2) {
        ele2 = ele->hnext;
        delete ele;
      }
    delete [] vec;
  };
  T& operator[](int ind) {
    SDIVecEle<T>* ele;
    int h = hash(ind);
    for (ele=vec[h]; ele && ele->ind != ind; ele=ele->hnext);
    if (!ele)
      ele = add(h, ind);
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
  int currind() { return currele->ind; }
  int size() { return num; }
  int operator!() { return (num == 0); }
  operator int() { return (num > 0); }
  void setdefault(T d) { def = d; };
private:
  int hash(int ind) {
    register int h;
    if (ind<0) ind &= 0x7fffffff;
    for (h=0; ind; h^=ind, ind>>=_bits);
    return h & (_size-1);
  }
  SDIVecEle<T>* add(int h, int ind) {
    SDIVecEle<T>* ele = new SDIVecEle<T>();
    SDIVecEle<T> *p, *p2;
    ele->ind = ind;
    ele->val = def;
    ele->hnext = vec[h];
    vec[h] = ele;
    for (p=first, p2=0; p && p->ind<ind; p2=p, p=p->next); // linear search...
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
    SDIVecEle<T>** oldvec;
    SDIVecEle<T> *ele, *ele2;
    int hind;
    int i;
    int oldsize;
    oldsize = _size;
    oldvec = vec;
    _size = sz;
    for (_bits=-1; sz; _bits++, sz>>=1);
    vec = new SDIVecEle<T>*[_size];
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
  SDIVecEle<T> *currele;
  SDIVecEle<T> *first;
  SDIVecEle<T>** vec;
};

#ifndef FORDIV
#define FORDIV( div, var, ind) for( (div).reset(); !((div).end())&&((var=(div).currval()),(ind=(div).currind()),1); )
#endif

#endif
