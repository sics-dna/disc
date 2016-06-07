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
#include "isc_micromodel.hh"

// A Markov model of a multivariate Gaussian distribution.
// That is, the state is described by a
// set of continuous values from a multivariate Gaussian

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


struct IscMgdGlobalInfo {
	IscMgdGlobalInfo() { prior_n = prior_v =  0.0; };
	void reset() { };
	double prior_n, prior_v;
};

class IscMgdAccumulator {
public:
	IscMgdAccumulator() {sqdist = logdet = 0.0; };
	virtual ~IscMgdAccumulator(){};

	// Read out anomaly and log predicted prob
	virtual double anomaly_acc() {
		//double ddist = (numd == 0 ? 0.0 : (logp - logpmean)*(logp - logpmean)/logpvar);
		//return -logintmultistudent(0.5*(5*(numo+numd)/(numc+1.0)), sqrt((sqdist*0.5 + ddist*0.67)/(5*(numo+numd)/(numc+1.0))), numc/* +1.0-1.0*/);
		return -log(0.5*incgammaq(0.5, (sqdist*0.5))); // + ddist*0.67)));
		//return (logp < logpmean ? -log(0.5*incgammaq(0.5, ddist*0.5)) + 3.0 : 0.0);
	};

	virtual double logp_acc() {
		// just a gaussian for now, should be multi-student-t
		return - 0.5*sqdist - 0.5*logdet;
	};


	virtual void reset_acc() {
		sqdist = 0.0;
		logdet = 0.0;
	};

	double sqdist;
	double logdet;

	IscMgdGlobalInfo modelinfo;
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
			int conditional_variable_length
	)  {
		gdim_predicted = predicted_variable_length;
		gdim_conditioned = conditional_variable_length;
		gdim_tot = gdim_predicted+gdim_conditioned;


		gindv = new int[gdim_tot];
		for (int i=0; i<gdim_predicted; i++) {
			gindv[i] = predicted_variable_index[i];
		}
		for (int i=gdim_predicted; i<gdim_tot; i++) {
			gindv[i] = conditional_variable_index[i];
		}
	};
	virtual ~IscMarkovGaussMicroModel() { delete [] gindv; };

	virtual IscMicroModel* create() {
		return new IscMarkovGaussMicroModel(
				gindv,
				gdim_predicted,
				(gindv+gdim_predicted*sizeof(int)),
				gdim_conditioned);
	};


	virtual void add_acc(IscMgdAccumulator *acc, intfloat* vec);

	virtual double anomaly(intfloat* vec) {
		IscMgdAccumulator* acc = new IscMgdAccumulator();
		add_acc(acc,vec);
		return acc->anomaly_acc();
	};
	virtual double logp(intfloat* vec) {
		IscMgdAccumulator* acc = new IscMgdAccumulator();
		add_acc(acc,vec);
		return acc->logp_acc();
	};

	// NOT IMPLEMENTED
	virtual int ev_logp(intfloat* vec, double& e, double& v) { return 0; };
	virtual int logpeak(intfloat* vec, double& p) { return 0; };
	virtual int invanomaly(intfloat* vec, double ano, intfloat* minvec, intfloat* maxvec, intfloat* peakvec) { return 0; };
	virtual int stats(union intfloat* vec, double* expect, double* var) { return 0; };

	// Training
	virtual void add(intfloat* vec);

	virtual void remove(intfloat* vec);

	virtual void reset();

	virtual void set_prior(double nn, double vv) {
		modelinfo.prior_n = nn;
		modelinfo.prior_v = vv;
	};
	virtual IscMgdGaussComp* getGaussComponent();

protected:

	int gdim_tot, gdim_predicted, gdim_conditioned;
	int* gindv;
	IscMgdGlobalInfo modelinfo;
	IscMgdGaussComp* gaussian_component;
};

//
// IscMarkovGaussCombinerMicroModel takes as input an array of IscMarkovGaussMicroModels
// and combines them into one model.It only works if the IscMarkovGaussMicroModels are
// (conditional) independent. This is equivalent to a directed graph model, e.g.,
// a Bayesian Network.
//
class IscMarkovGaussCombinerMicroModel : public IscMicroModel {
public:
	// New components are created internally, so the given components are not kept.
	IscMarkovGaussCombinerMicroModel(IscMarkovGaussMicroModel** gaussian_components, int num_of_components) {
		IscMarkovGaussMicroModel** comps = new IscMarkovGaussMicroModel*[num_of_components];
		for(int i=0; i < num_of_components; i++) {
			comps[i] = (IscMarkovGaussMicroModel*) gaussian_components[i]->create();
		}
		this->gaussian_components=comps;
		this->num_of_components=num_of_components;
		this->accumulator = new IscMgdAccumulator();
	};
	~IscMarkovGaussCombinerMicroModel() {
		if(accumulator) {
			delete accumulator;
		}
		if(gaussian_components) {
			for(int i=0; i < num_of_components; i++) {
				if(gaussian_components[i]) {
					delete gaussian_components[i];
				}
			}
			delete [] gaussian_components;

		}
	}

	virtual IscMicroModel* create() {
		return new IscMarkovGaussCombinerMicroModel(gaussian_components,num_of_components);
	}

	// Read out anomaly and log predicted prob
	virtual double anomaly(union intfloat* vec) {
		accumulator->reset_acc();
		for(int i=0; i < num_of_components; i++) {
			gaussian_components[i]->add_acc(accumulator,vec);
		}
		return accumulator->anomaly_acc();
	};
	virtual double logp(union intfloat* vec) {
		accumulator->reset_acc();
		for(int i=0; i < num_of_components; i++) {
			gaussian_components[i]->add_acc(accumulator,vec);
		}
		return accumulator->logp_acc();
	}

	// Training
	virtual void add(union intfloat* vec) {
		for(int i=0; i < num_of_components; i++) {
			gaussian_components[i]->add(vec);
		}
	};

	virtual void remove(union intfloat* vec) {
		for(int i=0; i < num_of_components; i++) {
			gaussian_components[i]->remove(vec);
		}
	}
	virtual void reset() {
		for(int i=0; i < num_of_components; i++) {
			gaussian_components[i]->reset();
		}
	}
protected:
	IscMarkovGaussMicroModel** gaussian_components;
	int num_of_components;
	IscMgdAccumulator* accumulator;
};


#endif
// End ISC_MICROMODEL_MARKOVGAUSSIAN_HH_
