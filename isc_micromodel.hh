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

#ifndef IscMicroModel_HH_
#define IscMicroModel_HH_
#include "intfloat.hh"



class IscMicroModel {
public:
  IscMicroModel() {};
  virtual ~IscMicroModel() {};

  // Should returns a micro model of the same class and with the same creation parameters.
  virtual IscMicroModel* create();

  // Read out anomaly and log predicted prob
  virtual double anomaly(union intfloat* vec) = 0;
  virtual double logp(union intfloat* vec) = 0;

  // Features that may or may not be implemented. Returns 0 if not.
  virtual int ev_logp(union intfloat* vec, double& e, double& v) { return 0; };
  virtual int logpeak(union intfloat* vec, double& p) { return 0; };
  virtual int invanomaly(union intfloat* vec, double ano, union intfloat* minvec, union intfloat* maxvec, union intfloat* peakvec) { return 0; };
  virtual int stats(union intfloat* vec, double* expect, double* var) { return 0; };

  // Training
  virtual void add(union intfloat* vec) = 0;
  virtual void remove(union intfloat* vec) = 0;
  virtual void reset() = 0;
};

#endif
