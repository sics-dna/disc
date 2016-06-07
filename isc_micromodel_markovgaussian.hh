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
#include <stdio.h>

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
	~IscMgdGaussComp() {
		delete [] mean;
		delete var;
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
		return -log(incgammaq(0.5, sqdist*0.5)); // sqdist*0.5+ ddist*0.67)));
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
		// First elements in gindv are the conditional variables
		for (int i=0; i<gdim_conditioned; i++) {
			if(DEBUG)
				printf("conditional_variable_index %i %i\n", i, conditional_variable_index[i]);
			gindv[i] = conditional_variable_index[i];
		}
		// Last elements in gindv are the predicted variables
		for (int i=0; i<gdim_predicted; i++) {
			if(DEBUG)
				printf("predicted_variable_index %i %i\n", i, predicted_variable_index[i]);
			gindv[i+gdim_conditioned] = predicted_variable_index[i];
		}

		gaussian_component = new IscMgdGaussComp(gdim_tot, modelinfo.prior_n, modelinfo.prior_v);
		if(DEBUG)
			printf("IscMarkovGaussMicroModel created\n");
	};
	virtual ~IscMarkovGaussMicroModel() {
		if(DEBUG)
			printf("IscMarkovGaussMicroModel deletion started\n");
		if(gindv) {
			delete [] gindv;
		}
		if(DEBUG)
			printf("IscMarkovGaussMicroModel deletion next\n");

		if(gaussian_component) {
			delete gaussian_component;
		}
		if(DEBUG)
			printf("IscMarkovGaussMicroModel deleted\n");
	};

	virtual IscMicroModel* create() {
		if(DEBUG)
			printf("Create a new IscMarkovGaussMicroModel with same arguments\n");

		return new IscMarkovGaussMicroModel(
				(gindv+gdim_conditioned),
				gdim_predicted,
				gindv,
				gdim_conditioned);
	};


	virtual void add_acc(IscMgdAccumulator *acc, intfloat* vec);

	virtual double anomaly(intfloat* vec) {
		IscMgdAccumulator* acc = new IscMgdAccumulator();
		add_acc(acc,vec);
		double ano= acc->anomaly_acc();
		delete acc;
		return ano;
	};
	virtual double logp(intfloat* vec) {
		IscMgdAccumulator* acc = new IscMgdAccumulator();
		add_acc(acc,vec);
		double logp_ = acc->logp_acc();
		delete acc;
		return logp_;

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

	// TODO check how this is effecting the gaussian component?
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
	IscMarkovGaussCombinerMicroModel(IscMarkovGaussMicroModel** gaussian_components0, int num_of_components0) {
		IscMarkovGaussMicroModel** comps = new IscMarkovGaussMicroModel*[num_of_components0];
		for(int i=0; i < num_of_components0; i++) {
			comps[i] = dynamic_cast<IscMarkovGaussMicroModel*>(gaussian_components0[i]->create());
		}
		this->gaussian_components=comps;
		this->num_of_components=num_of_components0;
		//this->accumulator = new IscMgdAccumulator();
	};

	~IscMarkovGaussCombinerMicroModel() {
		//if(accumulator) {
		//	delete accumulator;
		//}
		if(DEBUG) {
			printf("IscMarkovGaussCombinerMicroModel delete started\n");
		}
		if(gaussian_components) {
			for(int i=0; i < num_of_components; i++) {
				if(gaussian_components[i]) {
					delete gaussian_components[i];
				}
			}

			delete [] gaussian_components;

		}

		if(DEBUG) {
			printf("IscMarkovGaussCombinerMicroModel deleted\n");
		}

	}

	virtual IscMicroModel* create() {
		return new IscMarkovGaussCombinerMicroModel(gaussian_components,num_of_components);
	}

	// Read out anomaly and log predicted prob
	virtual double anomaly(union intfloat* vec) {
		accumulator.reset_acc();
		for(int i=0; i < num_of_components; i++) {
			gaussian_components[i]->add_acc(&accumulator,vec);
		}
		return accumulator.anomaly_acc();
	};
	virtual double logp(union intfloat* vec) {
		accumulator.reset_acc();
		for(int i=0; i < num_of_components; i++) {
			gaussian_components[i]->add_acc(&accumulator,vec);
		}
		return accumulator.logp_acc();
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
	IscMgdAccumulator accumulator;
};


class IscMarkovGaussMatrixMicroModel : public IscMicroModel {
public:
	//
	//
	IscMarkovGaussMatrixMicroModel(int* vector_index, int vector_length, int slots_per_row);
	virtual ~IscMarkovGaussMatrixMicroModel();

	// Should returns a micro model of the same class and with the same creation parameters as used when constructed.
	virtual IscMicroModel* create() {
		return new IscMarkovGaussMatrixMicroModel(vector_index, vector_length, slots_per_row);
	};

	// Read out anomaly and log predicted prob
	virtual double anomaly(union intfloat* vec) {
		return markovModel->anomaly(vec);
	};
	virtual double logp(union intfloat* vec) {
		return markovModel->logp(vec);
	};

	// Training
	virtual void add(union intfloat* vec) {
		markovModel->add(vec);
	};
	virtual void remove(union intfloat* vec) {
		markovModel->remove(vec);
	};
	virtual void reset() {
		markovModel->reset();
	};

protected:
	int* vector_index;
	int vector_length;
	int slots_per_row;
	IscMarkovGaussCombinerMicroModel* markovModel;
};


#endif
// End ISC_MICROMODEL_MARKOVGAUSSIAN_HH_
