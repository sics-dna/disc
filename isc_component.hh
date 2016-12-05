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

#ifndef IscComponent_HH_
#define IscComponent_HH_

#include "isc_micromodel.hh"

typedef class IscMicroModel* (*IscCreateFunc)(const void* co, int ind);

enum IscCombinationRule { IscMin, IscMax, IscPlus, IscBoth };

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

#endif
