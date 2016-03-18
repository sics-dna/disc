 /*
 --------------------------------------------------------------------------
 Copyright (C) 2011, 2015 SICS Swedish ICT AB

 Main author: Anders Holst <aho@sics.se>

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

#include "isc_micromodel.hh"
#include "isc_component.hh"

/*
typedef class IscMicroModel* (*IscCreateFunc)(const void* co, int ind);

enum IscCombinationRule { IscMax, IscPlus, IscBoth };

class IscComponent {
public:
  IscComponent(int cla, int clu, int ll, IscCombinationRule cr, IscCreateFunc cf, void* co);
  virtual ~IscComponent();

  // Read out anomaly and log predicted prob
  virtual double anomaly(union intfloat* vec);
  virtual double logp(union intfloat* vec);

  // Features that may or may not be implemented. Returns 0 if not.
  virtual int ev_logp(union intfloat* vec, double& e, double& v);
  virtual int logpeak(union intfloat* vec, double& p);
  virtual int invanomaly(union intfloat* vec, double ano, union intfloat* minvec, union intfloat* maxvec, union intfloat* peakvec);
  virtual int partsanomaly(union intfloat* vec, double* avec);
  virtual int stats(union intfloat* vec, double* expect, double* var);

  // Training
  virtual void add(union intfloat* vec);
  virtual void remove(union intfloat* vec);
  virtual void reset();

  virtual IscMicroModel* micromodel(int ind) { if (ind >= 0 && ind < len) return micro[ind]; else return 0; };
  virtual int nummicromodels() { return len; };
  int class_id;
  int cluster_id;
  IscComponent* next;
protected:
  int len;
  class IscMicroModel** micro;
  IscCombinationRule comb;
};
*/


IscComponent::IscComponent(int cla, int clu, int ll, IscCombinationRule cr, IscCreateFunc cf, void* co)
{
  int i;
  class_id = cla;
  cluster_id = clu;
  len = ll;
  comb = cr;
  micro = new IscMicroModel*[len];
  for (i=0; i<len; i++)
    micro[i] = cf(co, i);
  next = 0;
}

IscComponent::~IscComponent()
{
  int i;
  for (i=0; i<len; i++)
    delete micro[i];
  delete [] micro;
}

double IscComponent::anomaly(intfloat* vec)
{
  int i;
  double as, am, a;
  as = am = 0.0;
  for (i=0; i<len; i++) {
    a = micro[i]->anomaly(vec);
    as += a;
    if (a > am) am = a;
  }
  if (comb == IscMax)  // The below is wrong - both numbers should be adjusted
    return am;
  else
    return as;
}

double IscComponent::logp(intfloat* vec)
{
  int i;
  double lp = 0.0;
  for (i=0; i<len; i++)  // Here we make an independence assumption
    lp += micro[i]->logp(vec);
  return lp;
}

int IscComponent::ev_logp(intfloat* vec, double& e, double& v)
{
  int i;
  double ee, vv;
  e = v = 0.0;
  for (i=0; i<len; i++) {
    if (!micro[i]->ev_logp(vec, ee, vv))
      return 0;
    e += ee;
    v += vv;
  }
  return 1;
}

int IscComponent::logpeak(intfloat* vec, double& p)
{
  int i;
  double pp;
  p = 0.0;
  for (i=0; i<len; i++) {
    if (!micro[i]->logpeak(vec, pp))
      return 0;
    p += pp;
  }
  return 1;
}

int IscComponent::invanomaly(intfloat* vec, double ano, intfloat* minvec, intfloat* maxvec, intfloat* peakvec)
{
  int i;
  for (i=0; i<len; i++) {
    if (!micro[i]->invanomaly(vec, ano, minvec, maxvec, peakvec))
      return 0;
  }
  return 1;
}

int IscComponent::partsanomaly(intfloat* vec, double* avec)
{
  int i;
  for (i=0; i<len; i++) {
    avec[i] = micro[i]->anomaly(vec);
  }
  return 1;
}

int IscComponent::stats(intfloat* vec, double* expect, double* var)
{
  int i;
  for (i=0; i<len; i++) {
    if (!micro[i]->stats(vec, expect, var))
      return 0;
  }
  return 1;
}

void IscComponent::add(intfloat* vec)
{
  int i;
  for (i=0; i<len; i++)
    micro[i]->add(vec);
}

void IscComponent::remove(intfloat* vec)
{
  int i;
  for (i=0; i<len; i++)
    micro[i]->remove(vec);
}

void IscComponent::reset()
{
  int i;
  for (i=0; i<len; i++)
    micro[i]->reset();
}

