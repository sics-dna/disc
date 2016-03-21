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

#include "hmatrix.hh"

// The one doing all the calculations. The rest are just wrappers.
class IscRawMultiGaussianMicroModel {
public:
  IscRawMultiGaussianMicroModel() {};
  virtual ~IscRawMultiGaussianMicroModel() {};

  double raw_anomaly(double r, double n, int d);
  double raw_logp(double r, double dt, double n, int d);
  void raw_ev_logp(double dt, double n, int d, double& e, double& v);
  double raw_maxlogp(double dt, double n, int d);
  void raw_invanomaly(double c, double mn, double sc, double ano, double& minx, double& maxx);
  void raw_stats(double c, double mn, double sc, double& expect, double& var);
};

class IscMultiGaussianMicroModel : public IscMicroModel, public IscRawMultiGaussianMicroModel {
public:
  IscMultiGaussianMicroModel(int d, int* iv) { int i; dim = d; indv = new int[d]; sumx = new double[d]; sumxx = new HMatrix(d,2); mean = new double[d]; var = new HMatrix(d,2); dirty = 1; n = 0.0; prior_n = 0.0; prior_var = 0.0; for (i=0; i<d; i++) indv[i] = iv[i], sumx[i] = 0.0; };
  virtual ~IscMultiGaussianMicroModel() { delete [] indv; delete [] sumx; delete sumxx; delete [] mean; delete var; };

  // Read out anomaly and log predicted prob
  virtual double anomaly(intfloat* vec) { update(); return raw_anomaly(dist(vec), n, dim); };
  virtual double logp(intfloat* vec) { update(); return raw_logp(dist(vec), var->det(), n, dim); };

  virtual int ev_logp(intfloat* vec, double& e, double& v) { update(); raw_ev_logp(var->det(), n, dim, e, v); return 1; };
  virtual int logpeak(intfloat* vec, double& p) { update(); p = raw_maxlogp(var->det(), n, dim); return 1; };
//  virtual int invanomaly(intfloat* vec, double ano, intfloat* minvec, intfloat* maxvec, intfloat* peakvec) { double x1, x2; update(); if (minvec && maxvec) { raw_invanomaly(0.5*n, sumx/n, (n+1.0)/n*(sumxx-sumx*sumx/n), ano, x1, x2); minvec[indx].f=x1, maxvec[indx].f=x2; } if (peakvec) { raw_maxlogp(0.5*n, sumx/n, (n+1.0)/n*(sumxx-sumx*sumx/n), x1); peakvec[indx].f=x1; } return 1; };
//  virtual int stats(union intfloat* vec, double* expect, double* var) { double e, v; update(); raw_stats(0.5*n, sumx/n, (n+1.0)/n*(sumxx-sumx*sumx/n), e, v); if (expect) expect[indx]=e; if (var) var[indx]=v; return 1; };
  virtual int invanomaly(intfloat* vec, double ano, intfloat* minvec, intfloat* maxvec, intfloat* peakvec) { if (peakvec) { for (int i=0; i<dim;i++) peakvec[indv[i]].f=mean[i]; } return 1; };
  virtual int stats(union intfloat* vec, double* expect, double* var) { return 0; };

  // Training
  virtual void add(intfloat* vec) { int i; double *tmp = new double[dim]; n+=1.0; for (i=0; i<dim; i++) sumx[i] += tmp[i] = vec[indv[i]].f; sumxx->accum(tmp, 1.0); dirty = 1; delete[] tmp; };
  virtual void remove(intfloat* vec) { int i; double *tmp = new double[dim]; n-=1.0; for (i=0; i<dim; i++) sumx[i] -= tmp[i] = vec[indv[i]].f; sumxx->accum(tmp, -1.0); dirty = 1; delete[] tmp; };
// !!! TEMPORARY HACK TO TEST PRIOR : n =  3.0; sumxx->unit(); sumxx->scale(n*6.0); 
  virtual void reset() { int i; n=prior_n; for (i=0; i<dim; i++) sumx[i]=0.0; sumxx->unit(); sumxx->scale(prior_n * prior_var); dirty = 1; };
  virtual void set_prior(double nn, double std) { prior_n = nn; prior_var = std*std; };
protected:
  void update() { if (dirty) { int i; for (i=0; i<dim; i++) mean[i]=sumx[i]/n; var->set(*sumxx); var->accum(mean, -n); var->scale((n+1.0)/n); dirty=0; } };
  double dist(intfloat* vec) { double *v = new double[dim]; int i; for (i=0; i<dim; i++) v[i] = vec[indv[i]].f - mean[i]; double res = var->sqprodinv(v); delete[] v; return res; };

  int dirty;
  int dim;
  int* indv;
  double n;
  double prior_n, prior_var;
  double* sumx;
  HMatrix* sumxx;
  double* mean;
  HMatrix* var;
};





