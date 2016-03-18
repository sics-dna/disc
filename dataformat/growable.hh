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

#ifndef TEMPLATE_GROWABLE

#define TEMPLATE_GROWABLE

template<class T> class Growable {
public:
  Growable(int sz = 0, int chunk = 64){
    num = 0;
    _size = 0;
    _chunk = chunk;
    vec = curr = 0;
    resize(sz);
    num = sz;
  }; 
  Growable(const Growable<T> &grow, int chunk = 64){
    T *vec1, *vec2;
    int i;
    
    num = 0;
    _size = 0;
    vec = curr = 0;
    resize(grow.num);
    num = grow.num;
    for (i=0, vec1=grow.vec, vec2=vec; i<num; i++, vec1++, vec2++)
      *vec2 = *vec1;
  }; 
  ~Growable() {delete [] vec;}
  Growable<T>& operator=(Growable<T> &grow){
    T *vec1, *vec2;
    int i;
    
    if (grow.num > _size) {
      num = 0;
      delete [] vec;
      vec = 0;
      resize(grow.num);
    }
    curr = vec;
    num = grow.num;
    for (i=0, vec1=grow.vec, vec2=vec; i<num; i++, vec1++, vec2++)
      *vec2 = *vec1;
    return *this;
  };
  T* vect() {return vec;}
  Growable<T>& operator<<(T val) { int i=add(1); vec[i]=val; return *this;};
  T& operator[](int i) {if (i>=num) add(i-num+1);
                        return *((curr = vec + i + 1) - 1);}
  T& operator()() {return *curr++;}
  Growable<T>& reset() {curr = vec; return *this;}
  int end() {return (curr >= vec + num);}
  int current() { return curr - vec; }
  int size() {return num;}
  int operator!() {return (num == 0);}
  operator int() {return (num > 0);}
  int add(int n = 1){
    int tn = num;
    if (num + n > _size) 
      resize(num + n);
    num += n;
    return tn;
  };
  int remove(int n = 1){
    num -= n;
    if (num < 0)
      num = 0;
    if (num < _size - 2*_chunk)
      resize(num);
    return num;
  };
private:
  void resize(int sz){
    T *newvec, *vec1, *vec2;
    int i, tcurr;
    
    sz = ((sz - 1) / _chunk + 1) * _chunk;
    if (_size != sz) {
      _size = sz;
      if (_size > 10 * _chunk) _chunk *= 2;
      newvec = new T[_size];
      num = (num > _size ? _size : num);
      tcurr = (curr - vec);
      tcurr = (tcurr > _size ? _size : tcurr);
      for (i=0, vec1=newvec, vec2=vec; i<num; i++, vec1++, vec2++)
        *vec1 = *vec2;
      delete [] vec;
      vec = newvec;
      curr = vec + tcurr;
    }
  };
  int _size;
  int _chunk;
  int num;
  T *curr;
  T *vec;
};

#define FORVEC( grow, var) for((grow).reset(); !((grow).end())&&((var=(grow)()),1); )

// #define FORGROW( grow, var, gop) for( gop=(grow).current(),(grow).reset(); ((grow).end()?((grow)[gop-1],0):((var=(grow)()),1)); )

#define FORGROW( grow, var, gop) for( gop=0; gop<(grow).size() && (var=(grow)[gop], 1); gop++ )

#endif
