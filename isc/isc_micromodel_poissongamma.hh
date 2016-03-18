 /*
 --------------------------------------------------------------------------
 Copyright (C) 2010, 2015 SICS Swedish ICT AB

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

#ifndef isc_micromodel_poissongamma_HH_
#define isc_micromodel_poissongamma_HH_

#ifdef WIN32
#include <FLOAT.H>
#define HUGE_VALF FLT_MAX
#endif





// The one doing all the calculations. The rest are just wrappers.
// Note that in the prior P(lambda)=lambda^-c, c is set to 0 below,
// otherwise r=0 produces infinities
class IscRawExponentialMicroModel {
public:
  IscRawExponentialMicroModel() {};
  virtual ~IscRawExponentialMicroModel() {};

  double raw_anomaly(double r, double t, double rr, double tt);
  double raw_logp(double r, double t, double rr, double tt);

  void raw_ev_logp(double rr, double tt, double& e, double& v);
  double raw_maxlogp(double rr, double tt, double& peakr, double& peakt);
  void raw_invanomaly(double rr, double tt, double ano, double& minr, double& maxr, double& mint, double& maxt);
  void raw_stats_r(double rr, double tt, double& expect, double& var) {}; // Not implemented in the general case
  void raw_stats_t(double rr, double tt, double& expect, double& var) {}; // Not implemented in the general case

  void raw_add(double r, double t) { sumr += r; sumt += t; };
  void raw_remove(double r, double t) { sumr -= r; sumt -= t; };
  void raw_reset() { sumr = 0.0; sumt = 0.0; };

  double sumr, sumt;
};

class IscExponentialMicroModel : public IscMicroModel, public IscRawExponentialMicroModel {
public:
  IscExponentialMicroModel(int ir, int it, int irr, int itt) { indr=ir; indt=it; indrr=irr; indtt=itt; };
  virtual ~IscExponentialMicroModel() {};

  // Read out anomaly and log predicted prob
  virtual double anomaly(intfloat* vec) { return raw_anomaly(vec[indr].f, vec[indt].f, vec[indrr].f, vec[indtt].f); };
  virtual double logp(intfloat* vec) { return raw_logp(vec[indr].f, vec[indt].f, vec[indrr].f, vec[indtt].f); };

  virtual int ev_logp(intfloat* vec, double& e, double& v) { raw_ev_logp(vec[indrr].f, vec[indtt].f, e, v); return 1; };
  virtual int logpeak(intfloat* vec, double& p) { double dr, dt; p = raw_maxlogp(vec[indrr].f, vec[indtt].f, dr, dt); return 1; };
  virtual int invanomaly(intfloat* vec, double ano, intfloat* minvec, intfloat* maxvec, intfloat* peakvec) { double r1, r2, t1, t2; if (minvec && maxvec) { raw_invanomaly(vec[indrr].f, vec[indtt].f, ano, r1, r2, t1, t2); minvec[indr].f=r1, maxvec[indr].f=r2, minvec[indt].f=t1, maxvec[indt].f=t2; } if (peakvec) { raw_maxlogp(vec[indrr].f, vec[indtt].f, r1, t1); peakvec[indr].f=r1, peakvec[indt].f=t1; } return 1; };

  // Training
  virtual void add(intfloat* vec) { raw_add(vec[indr].f, vec[indt].f); };
  virtual void remove(intfloat* vec) { raw_remove(vec[indr].f, vec[indt].f); };
  virtual void reset() { raw_reset(); };

  int indr, indt, indrr, indtt;
};

class IscPoissonMicroModel : public IscMicroModel, public IscRawExponentialMicroModel {
public:
  IscPoissonMicroModel(int ir, int it) { indr=ir; indt=it; };
  virtual ~IscPoissonMicroModel() {};

  // Read out anomaly and log predicted prob
  virtual double anomaly(intfloat* vec) { return raw_anomaly(vec[indr].f, vec[indt].f, HUGE_VALF, vec[indt].f); };
  virtual double logp(intfloat* vec) { return raw_logp(vec[indr].f, vec[indt].f, HUGE_VALF, vec[indt].f); };

  virtual int ev_logp(intfloat* vec, double& e, double& v) { raw_ev_logp(HUGE_VALF, vec[indt].f, e, v); return 1; };
  virtual int logpeak(intfloat* vec, double& p) { double dr, dt; p = raw_maxlogp(HUGE_VALF, vec[indt].f, dr, dt); return 1; };
  virtual int invanomaly(intfloat* vec, double ano, intfloat* minvec, intfloat* maxvec, intfloat* peakvec) { double r1, r2, t1, t2; if (minvec && maxvec) { raw_invanomaly(HUGE_VALF, vec[indt].f, ano, r1, r2, t1, t2); minvec[indr].f=r1, maxvec[indr].f=r2, minvec[indt].f=t1, maxvec[indt].f=t2; } if (peakvec) { raw_maxlogp(HUGE_VALF, vec[indt].f, r1, t1); peakvec[indr].f=r1, peakvec[indt].f=t1; } return 1; };
  virtual int stats(union intfloat* vec, double* expect, double* var) { double e; e=vec[indt].f*sumr/sumt; if (expect) expect[indr]=e; if (var) var[indr]=e; return 1; }; // This assumes pure Poisson, not the predictive distribution neg binomial


  // Training
  virtual void add(intfloat* vec) { raw_add(vec[indr].f, vec[indt].f); };
  virtual void remove(intfloat* vec) { raw_remove(vec[indr].f, vec[indt].f); };
  virtual void reset() { raw_reset(); };

  int indr, indt;
};

class IscGammaMicroModel  : public IscMicroModel, public IscRawExponentialMicroModel {
public:
  IscGammaMicroModel(int ir, int it) { indr=ir; indt=it; };
  virtual ~IscGammaMicroModel() {};

  // Read out anomaly and log predicted prob
  virtual double anomaly(intfloat* vec) { return raw_anomaly(vec[indr].f, vec[indt].f, vec[indr].f, vec[indt].f); };
  virtual double logp(intfloat* vec) { return raw_logp(vec[indr].f, vec[indt].f, vec[indr].f, vec[indt].f); };

  virtual int ev_logp(intfloat* vec, double& e, double& v) { raw_ev_logp(vec[indr].f, vec[indt].f, e, v); return 1; };
  virtual int logpeak(intfloat* vec, double& p) { double dr, dt; p = raw_maxlogp(vec[indr].f, vec[indt].f, dr, dt); return 1; };
  virtual int invanomaly(intfloat* vec, double ano, intfloat* minvec, intfloat* maxvec, intfloat* peakvec) { double r1, r2, t1, t2; if (minvec && maxvec) { raw_invanomaly(vec[indr].f, vec[indt].f, ano, r1, r2, t1, t2); minvec[indr].f=r1, maxvec[indr].f=r2, minvec[indt].f=t1, maxvec[indt].f=t2; } if (peakvec) { raw_maxlogp(vec[indr].f, vec[indt].f, r1, t1); peakvec[indr].f=r1, peakvec[indt].f=t1; } return 1; };

  // Training
  virtual void add(intfloat* vec) { raw_add(vec[indr].f, vec[indt].f); };
  virtual void remove(intfloat* vec) { raw_remove(vec[indr].f, vec[indt].f); };
  virtual void reset() { raw_reset(); };

  int indr, indt;
};

#endif //isc_micromodel_poissongamma.hh
