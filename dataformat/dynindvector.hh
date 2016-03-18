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

#ifndef TEMPLATE_DYNINDVECTOR

#define TEMPLATE_DYNINDVECTOR

template<class T> class DIVecEle {
public:
  int ind;
  T val;
  DIVecEle<T>* next;
};

template<class T> class DynamicIndexVector {
public:
  DynamicIndexVector() {
    num = 0;
    _size = 0;
    _blockresize = 0;
    vec = 0;
    currele = 0;
    resize(32);
  };
  DynamicIndexVector(const T d) {
    num = 0;
    _size = 0;
    _blockresize = 0;
    vec = 0;
    currele = 0;
    def = d;
    resize(32);
  };
  ~DynamicIndexVector() {
    DIVecEle<T> *ele, *ele2;
    int i;
    for (i=0; i<_size; i++)
      for (ele=vec[i]; ele; ele=ele2) {
        ele2 = ele->next;
        delete ele;
      }
    delete [] vec;
  };
  T& operator[](int ind) {
    DIVecEle<T>* ele;
    int h = hash(ind);
    for (ele=vec[h]; ele && ele->ind != ind; ele=ele->next);
    if (!ele)
      ele = add(h, ind);
    return ele->val;
  };
  void clear() {
    DIVecEle<T> *ele, *ele2;
    int i;
    for (i=0; i<_size; i++){
      for (ele=vec[i]; ele; ele=ele2) {
        ele2 = ele->next;
        delete ele;
      }
      vec[i]=0;
    }
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
  DIVecEle<T>* add(int h, int ind) {
    DIVecEle<T>* ele = new DIVecEle<T>();
    ele->ind = ind;
    ele->val = def;
    ele->next = vec[h];
    vec[h] = ele;
    num++;
    if (!_blockresize && num > _size*4)
      resize(_size*8);
    return ele;
  };
  void resize(int sz) {
    DIVecEle<T>** oldvec;
    DIVecEle<T> *ele, *ele2;
    int hind;
    int i;
    int oldsize;
    oldsize = _size;
    oldvec = vec;
    _size = sz;
    for (_bits=-1; sz; _bits++, sz>>=1);
    vec = new DIVecEle<T>*[_size];
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
  DIVecEle<T> *currele;
  int currhind;
  DIVecEle<T>** vec;
};

#ifndef FORDIV
#define FORDIV( div, var, ind) for( (div).reset(); !((div).end())&&((var=(div).currval()),(ind=(div).currind()),1); )
#endif

#endif
