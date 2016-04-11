 /*
 --------------------------------------------------------------------------
 Copyright (C) 2015 SICS Swedish ICT AB

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

#include <math.h>
#ifdef WIN32
#define _USE_MATH_DEFINES
#include <cmath>
#include <FLOAT.H>
#ifndef HUGE_VALF
#define HUGE_VALF FLT_MAX
#endif
#endif
#include "gamma.hh"
#include "hgf.hh"
#include "intfloat.hh"
#include "isc_micromodel.hh"


// Here we use the Gamma approximation to the logarithm of the product of dirichlet
// variables, as opposed to the Gauss approximation which doesn't catch the skewness

class IscMultiDirichletMicroModel : public IscMicroModel {
public:
  IscMultiDirichletMicroModel(int d, int* iv, int* no) { dim = d; indv = new int[d]; outcomes = new int[d]; totcount = new double[d]; counts = new double*[d]; mean = 0.0; var = 0.0; dirty = 1; n = 0.0; prior_n = 0.0; for (int i=0; i<d; i++) { indv[i] = iv[i]; outcomes[i] = no[i]; totcount[i] = 0.0; counts[i] = new double[no[i]]; for (int j=0; j<no[i]; j++) counts[i][j] = 0.0; }};
  virtual ~IscMultiDirichletMicroModel() { delete [] indv; delete [] outcomes; delete [] totcount; for (int i=0; i<dim; i++) delete [] counts[i]; delete [] counts; };

  virtual IscMicroModel* create() {
	  return  new IscMultiDirichletMicroModel(dim, indv, outcomes);
  };

  // Read out anomaly and log predicted prob
  virtual double anomaly(intfloat* vec) { update(); return log(incgammaq(totmean*totmean/totvar, logp(vec)*totmean/totvar)); };
  virtual double logp(intfloat* vec) { int ind; double lp = 0.0; update(); for (int i=0; i<dim; i++) { ind = vec[indv[i]].i; if (ind != -1) lp += log(counts[i][ind]/totcount[i]); } return lp; };

  virtual int ev_logp(intfloat* vec, double& e, double& v) { update(); e = logpmean; v = logpvar; return 1; };
  virtual int logpeak(intfloat* vec, double& p) { double lp = 0.0; update(); for (int i=0; i<dim; i++) { double maxp = 0.0; int ind = -1; for (int j=0; j<outcomes[i]; j++) if (maxp < counts[i][j]) maxp = counts[i][j], ind = j; lp+=log(maxp/totcount[i]); } p = lp; } return 1; };
  virtual int invanomaly(intfloat* vec, double ano, intfloat* minvec, intfloat* maxvec, intfloat* peakvec) { if (peakvec) { for (int i=0; i<dim; i++) { double maxp = 0.0; int ind = -1; for (int j=0; j<outcomes[i]; j++) if (maxp < counts[i][j]) maxp = counts[i][j], ind = j; peakvec[indv[i]].i=ind; }} return 1; };
  virtual int stats(union intfloat* vec, double* expect, double* var) { return 0; };

  // Training
  virtual void add(intfloat* vec) { int val; for (int i=0; i<dim; i++) { val = vec[indv[i]].i; if (val > -1 && val < outcomes[i]) { totcount[i] += 1.0; counts[i][val] += 1.0; }} dirty = 1; };
  virtual void remove(intfloat* vec) { int val; for (int i=0; i<dim; i++) { val = vec[indv[i]].i; if (val > -1 && val < outcomes[i]) { totcount[i] -= 1.0; counts[i][val] -= 1.0; }} dirty = 1; };
  virtual void reset() { for (int i=0; i<dim; i++) { totcount[i] = prior_n; for (int j=0; j<outcomes[i]; j++) counts[i][j] = prior_n/outcomes[i]; } dirty = 1; };
  virtual void set_prior(double nn) { prior_n = nn; };
protected:
  void update() { if (dirty) { logpmean = logpvar = 0.0; for (int i=0; i<dim; i++) { double m=0.0, s=0.0, p, lp; for (int j=0; j<outcomes[i]; j++) { p = counts[i][j]/totcount[i]; lp = log(p); m+=p*lp; s+=p*lp*lp; } logpmean += m; logpvar += (s-m*m); } dirty=0; } };

  int dirty;
  int dim;
  int* indv;
  int* outcomes;
  double prior_n;
  double* totcount;
  double** counts;
  double logpmean;
  double logpvar;
};


