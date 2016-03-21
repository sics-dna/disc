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
#include "hmatrix.hh"
#include "dynindvector.hh"
#include "intfloat.hh"


// A Markov chain of a cross product of a Discrete and a multivariate Gaussian distribution.
// That is, the state is described by a discrete number (like the cell index) and a
// set of continuous values from a multivariate Gaussian for each discrete number. 
// If the continuous state at two consecutive time steps are v=v_i and w=v_i-1, and the
// discrete state at two steps are h=h_i and k=h_i-1, then we need to calculate a product
// of factors:
//   P(vh|wk) = P(vwh|k)/P(w|k) = P(vw|hk)/P(w|hk) * P(w|hk)P(h|k)/Sum_j=H(P(w|jk)P(j|k))
// So for each hk-transition we need to store Sigma_vw, Mu_vw, and n.
// Here we use a Gaussian approximation to the logarithm of the product of discrete
// variables, since we then can use the multivariate Gaussian anomaly detection equations
// for the whole Markov chain.

struct IscMgdGaussComp {
  IscMgdGaussComp(int dim, double prior_n, double prior_v) {
    count = prior_n;
    mean = new double[dim*2];
    for (int i=0; i<dim*2; i++)
      mean[i] = 0.0;
    var = new HMatrix(dim*2, 2); 
    var->unit();
    var->scale(prior_v);
  };
  double count;
  double* mean;
  HMatrix* var;
};

struct IscMgdVector {
  IscMgdVector() : vector(0) { margin_num = 0.0; };
  DynamicIndexVector<IscMgdGaussComp*> vector;
  double margin_num;
};

struct IscMgdGlobalInfo {
  IscMgdGlobalInfo() { prior_n = prior_v = prior_cellcount = global_num = global_sum = global_sqsum = 0.0; };
  void reset() { global_num = prior_cellcount; global_sum = global_sqsum = 0.0; };
  double prior_n, prior_v;
  double prior_cellcount;
  double global_num;
  double global_sum;
  double global_sqsum;
};

struct IscMgdAccumulator {
  IscMgdAccumulator() { numo = numc = numd = sqdist = logdet = logp = logpmean = logpvar = 0.0; };
  double numo;
  double numc;
  double numd;
  double sqdist;
  double logdet;
  double logp;
  double logpmean;
  double logpvar;
};

class IscMarkovGaussDiscreteMicroModel : public IscMicroModel {
public:
  // iv contains the indices in the vector of the features of this micromodel.
  // The layout is {discr_t1, gauss1_t1, gauss2_t1, ..., discr_t2, gauss1_t2, gauss2_t2}
  // gd is the number of Gaussian dimensions, so the total length of iv is 2*gd+2
  IscMarkovGaussDiscreteMicroModel(int gd, int* iv, int ci) : model(0) {
    gdim = gd;
    gindv = new int[gdim*2];
    dind1 = iv[0];
    dind2 = iv[gdim+1];
    catind = ci;
    for (int i=0; i<gdim; i++) {
      gindv[i] = iv[i+1];
      gindv[i+gdim] = iv[i+gdim+2];
    }
  };
  virtual ~IscMarkovGaussDiscreteMicroModel() { delete [] gindv; };

  virtual void reset_acc(IscMgdAccumulator* acc) {
    acc->numo = 0.0;
    acc->numc = 0.0;
    acc->numd = 0.0;
    acc->sqdist = 0.0;
    acc->logdet = 0.0;
    acc->logp = 0.0;
    acc->logpmean = 0.0;
    acc->logpvar = 0.0;
  };

  virtual void update_acc(IscMgdAccumulator* acc, intfloat* vec) {
    double sum, sqsum, totcount, p, lp;
    int geocell1 = static_cast<int>(vec[dind1].i);
    int geocell2 = static_cast<int>(vec[dind2].i);
    int category = static_cast<int>(vec[catind].i);
    int index2;
    IscMgdVector* mod1;
    IscMgdGaussComp* mod2;
    IscMgdGaussComp* mod2tmp;
    lookup(vec, 0, mod1, mod2);
    if (mod1) {
      totcount = 1.0;
      sum = modelinfo.global_sum / modelinfo.global_num;
      sqsum = modelinfo.global_sqsum / modelinfo.global_num;
      if (mod2) {
        double targetcount = 0.0;
        double *tmp = new double[gdim*2];
        for (int i=0; i<gdim*2; i++)
          tmp[i] = vec[gindv[i]].f - mod2->mean[i];
        FORDIV(mod1->vector, mod2tmp, index2)
          if (mod2tmp) {
/**/
            double dist = mod2tmp->var->sqprodinv_part(tmp, gdim);
            double det = mod2tmp->var->det_part(gdim);
            double weightedcount = (det ? exp(-0.5*dist)/sqrt(det) : dist ? 0.0 : 1.0) * mod2tmp->count;
/**/
//            double weightedcount = mod2tmp->count;
            double lc = log(weightedcount);
            if (mod2 == mod2tmp) targetcount = weightedcount;
            totcount += weightedcount;
            sum += weightedcount * lc;
            sqsum += weightedcount * lc * lc;
          }
        acc->numo += mod2->count;
        acc->numc += 1.0;
        acc->numd += 1.0;
        acc->logp += log(targetcount/totcount);
        acc->logpmean += sum/totcount - log(totcount);
        acc->logpvar += sqsum/totcount - sum*sum/(totcount*totcount);
        acc->sqdist += mod2->var->sqprodinv_cond(tmp, gdim);
        acc->logdet += log(mod2->var->det_cond(gdim));
        delete [] tmp;
      } else {
        // What to do if mod1 exists but we never jumped to mod2 before
        FORDIV(mod1->vector, mod2tmp, index2)
          if (mod2tmp) {
            double weightedcount = mod2tmp->count;
            double lc = log(weightedcount);
            totcount += weightedcount;
            sum += weightedcount * lc;
            sqsum += weightedcount * lc * lc;
          }
        acc->numd += 1.0;
        acc->logp += log(1.0/totcount);
        acc->logpmean += sum/totcount - log(totcount);
        acc->logpvar += sqsum/totcount - sum*sum/(totcount*totcount);
      }
    } else {
      // What if we never were in mod1 before, and thus never jumped anywhere from there
      int geocell2 = static_cast<int>(vec[dind2].i);
      int category = static_cast<int>(vec[catind].i);
      double margnum;
      if (geocell2 != -1 && category != -1)
        mod1 = model[geocell1*20+category];
      if (mod1)
        margnum = mod1->margin_num;
      else
        margnum = 1.0;
      acc->numd += 1.0;
      acc->logp += log(margnum/modelinfo.global_num);
      acc->logpmean += modelinfo.global_sum / modelinfo.global_num - log(modelinfo.global_num);
      acc->logpvar += modelinfo.global_sqsum / modelinfo.global_num - modelinfo.global_sum*modelinfo.global_sum / (modelinfo.global_num*modelinfo.global_num);
    }
  };

  // Read out anomaly and log predicted prob
  virtual double anomaly_acc(IscMgdAccumulator* acc) {
    double ddist = (acc->numd == 0 ? 0.0 : (acc->logp - acc->logpmean)*(acc->logp - acc->logpmean)/acc->logpvar);
    //return -logintmultistudent(0.5*(5*(acc->numo+acc->numd)/(acc->numc+1.0)), sqrt((acc->sqdist*0.5 + ddist*0.67)/(5*(acc->numo+acc->numd)/(acc->numc+1.0))), acc->numc/* +1.0-1.0*/); 
    return -log(0.5*incgammaq(0.5, (acc->sqdist*0.5 + ddist*0.67)));
    //return (acc->logp < acc->logpmean ? -log(0.5*incgammaq(0.5, ddist*0.5)) + 3.0 : 0.0);
  };

  virtual double logp_acc(IscMgdAccumulator* acc) {
// just a gaussian for now, should be multi-student-t
    return acc->logp - 0.5*acc->sqdist - 0.5*acc->logdet;
  };

  virtual double anomaly(intfloat* vec) { return 0.0; };
  virtual double logp(intfloat* vec) { return 0.0; };

  virtual int ev_logp(intfloat* vec, double& e, double& v) { return 0; };
  virtual int logpeak(intfloat* vec, double& p) { return 0; };
  virtual int invanomaly(intfloat* vec, double ano, intfloat* minvec, intfloat* maxvec, intfloat* peakvec) { return 0; };
  virtual int stats(union intfloat* vec, double* expect, double* var) { return 0; };

  // Training
  virtual void add(intfloat* vec) {
    double *tmp = new double[gdim*2];
    double lc1, lc0;
    IscMgdVector* mod1;
    IscMgdGaussComp* mod2;
    lookup(vec, 1, mod1, mod2);
    if (mod2) {
      for (int i=0; i<gdim*2; i++)
        tmp[i] = vec[gindv[i]].f - mod2->mean[i];
      mod2->count += 1.0;
      for (int i=0; i<gdim*2; i++)
        mod2->mean[i] += tmp[i]/mod2->count;
      mod2->var->accum(tmp, 1.0/mod2->count);
      mod2->var->scale((mod2->count - 1.0)/mod2->count);
      lc0 = (mod1->margin_num ? log(mod1->margin_num) : 0.0);
      mod1->margin_num += 1.0;
      lc1 = (mod1->margin_num ? log(mod1->margin_num) : 0.0);
      modelinfo.global_num += 1.0;
      modelinfo.global_sum += mod1->margin_num*(lc1-lc0) + lc0;
      modelinfo.global_sqsum += mod1->margin_num*(lc1*lc1-lc0*lc0) + lc0*lc0;
    }
    delete [] tmp;
  };

  virtual void remove(intfloat* vec) {
    double *tmp = new double[gdim*2];
    double lc0, lc1;
    IscMgdVector* mod1;
    IscMgdGaussComp* mod2;
    lookup(vec, 1, mod1, mod2);
    if (mod2) {
      for (int i=0; i<gdim*2; i++)
        tmp[i] = vec[gindv[i]].f - mod2->mean[i];
      mod2->count -= 1.0;
      for (int i=0; i<gdim*2; i++)
        mod2->mean[i] -= tmp[i]/mod2->count;
      mod2->var->accum(tmp, -1.0/mod2->count);
      mod2->var->scale((mod2->count + 1.0)/mod2->count);
      lc0 = (mod1->margin_num ? log(mod1->margin_num) : 0.0);
      mod1->margin_num -= 1.0;
      lc1 = (mod1->margin_num ? log(mod1->margin_num) : 0.0);
      modelinfo.global_num -= 1.0;
      modelinfo.global_sum += mod1->margin_num*(lc1-lc0) - lc0;
      modelinfo.global_sqsum += mod1->margin_num*(lc1*lc1-lc0*lc0) - lc0*lc0;
    }
    delete [] tmp;
  };

  virtual void reset() {
    IscMgdVector* mod1;
    IscMgdGaussComp* mod2;
    int index1, index2;
    FORDIV(model, mod1, index1)
      if (mod1) {
        mod1->margin_num = 0.0;
        FORDIV(mod1->vector, mod2, index2)
          if (mod2) {
            mod2->count = modelinfo.prior_n;
            for (int i=0; i<gdim*2; i++)
              mod2->mean[i] = 0.0;
            mod2->var->unit();
            mod2->var->scale(modelinfo.prior_v);
          }
      }
    modelinfo.reset();
  }

  virtual void set_prior(double nn, double vv, double cc) { modelinfo.prior_n = nn; modelinfo.prior_v = vv; modelinfo.prior_cellcount = cc; };

  virtual bool has_matching_link(intfloat* vec) {
    IscMgdVector* mod1;
    IscMgdGaussComp* mod2;
    lookup(vec, 0, mod1, mod2);
    return (mod2 ? true : false);
  }

protected:

  void lookup(intfloat* vec, int insert, IscMgdVector*& mod1, IscMgdGaussComp*& mod2) {
    int geocell1 = static_cast<int>(vec[dind1].i);
    int geocell2 = static_cast<int>(vec[dind2].i);
    int category = static_cast<int>(vec[catind].i);
    if (geocell1 != -1 && geocell2 != -1 && category != -1) {
      mod1 = model[geocell1*20+category];
      if (mod1) {
        mod2 = mod1->vector[geocell2];
        if (!mod2 && insert)
          mod2 = mod1->vector[geocell2] = new IscMgdGaussComp(gdim, modelinfo.prior_n, modelinfo.prior_v);
      } else if (insert) {
        mod1 = model[geocell1*20+category] = new IscMgdVector();
        mod2 = mod1->vector[geocell2] = new IscMgdGaussComp(gdim, modelinfo.prior_n, modelinfo.prior_v);
      } else {
        mod1 = 0;
        mod2 = 0;
      }
      return;
    } else {
      mod1 = 0;
      mod2 = 0;
      return;
    }
  };

  int gdim;
  int dind1, dind2, catind;
  int* gindv;
  IscMgdGlobalInfo modelinfo;
  DynamicIndexVector<IscMgdVector*> model;
};

/*
 En dynindvec, nej hash, med "g1:g2:type" som nyckel, innehåll: count, mean, var.
 Eller en dynindvec med geo1*20+type, av dyninvec geo2, av  count, mean, var.
*/
