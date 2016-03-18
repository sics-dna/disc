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

#ifndef ISC_MICROMODEL_MARKOVGAUSSIAN_HH_
#define ISC_MICROMODEL_MARKOVGAUSSIAN_HH_

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


// A Markov chain of a multivariate Gaussian distribution.
// That is, the state is described by a
// set of continuous values from a multivariate Gaussian for each discrete number. 

struct IscMgdGaussComp {
  IscMgdGaussComp(int dim, double prior_n, double prior_v) {
    count = prior_n;
    mean = new double[dim];
    for (int i=0; i<dim; i++)
      mean[i] = 0.0;
    var = new HMatrix(dim, 2);
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

class IscMarkovGaussMicroModel : public IscMicroModel {

public:
  // predicted_variable_index contains the indices in the vector of the features of this micromodel.
  // that are predicted and conditional_variable_index_length contains the indices in the vector of the features
  // in the condition clause: p(<predicted variables> | <conditional variables>)
  //
  IscMarkovGaussMicroModel(
		  int* predicted_variable_index,
		  int predicted_variable_length,
		  int* conditional_variable_index,
		  int conditional_variable_length,
		  int category_index
		  ) : model(0) {
    gdim_predicted = predicted_variable_length;
    gdim_conditioned = conditional_variable_length;
	gdim_tot = gdim_predicted+gdim_conditioned;


    gindv = new int[gdim_tot];
    catind = category_index;
    for (int i=0; i<gdim_predicted; i++) {
      gindv[i] = predicted_variable_index[i];
    }
    for (int i=gdim_predicted; i<gdim_predicted+gdim_predicted; i++) {
      gindv[i] = conditional_variable_index[i];
    }

  };
  virtual ~IscMarkovGaussMicroModel() { delete [] gindv; };

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

  virtual void update_acc(IscMgdAccumulator* acc, intfloat* vec);

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
  virtual void add(intfloat* vec);

  virtual void remove(intfloat* vec);

  virtual void reset();

  virtual void set_prior(double nn, double vv, double cc) { modelinfo.prior_n = nn; modelinfo.prior_v = vv; modelinfo.prior_cellcount = cc; };

  virtual bool has_matching_link(intfloat* vec) {
    IscMgdVector* mod1;
    IscMgdGaussComp* mod2;
    lookup(vec, 0, mod1, mod2);
    return (mod2 ? true : false);
  }

protected:

  virtual void lookup(intfloat* vec, int insert, IscMgdVector*& mod1, IscMgdGaussComp*& mod2);
  int gdim_tot, gdim_predicted, gdim_conditioned;
  int catind;
  int* gindv;
  IscMgdGlobalInfo modelinfo;
  DynamicIndexVector<IscMgdVector*> model;
};

/*
 En dynindvec, nej hash, med "g1:g2:type" som nyckel, innehåll: count, mean, var.
 Eller en dynindvec med geo1*20+type, av dyninvec geo2, av  count, mean, var.
*/
#endif
// End ISC_MICROMODEL_MARKOVGAUSSIAN_HH_
