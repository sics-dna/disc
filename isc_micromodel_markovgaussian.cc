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
#include <stdio.h>
#include "isc_micromodel_markovgaussian.hh"


void IscMarkovGaussMicroModel::add_acc(IscMgdAccumulator *acc, intfloat* vec) {
	double *tmp = new double[gdim_tot];
	IscMgdGaussComp* gmod = getGaussComponent();
	for (int i=0; i<gdim_tot; i++)
		tmp[i] = vec[gindv[i]].f - gmod->mean[i];

	acc->sqdist += gmod->var->sqprodinv_cond(tmp, gdim_conditioned);
	acc->logdet += log(gmod->var->det_cond(gdim_conditioned));
	delete [] tmp;

};


// Training
void IscMarkovGaussMicroModel::add(intfloat* vec) {
	double *tmp = new double[gdim_tot];
	IscMgdGaussComp* gmod = getGaussComponent();

	for (int i=0; i<gdim_tot; i++)
		tmp[i] = vec[gindv[i]].f - gmod->mean[i];

	gmod->count += 1.0;
	for (int i=0; i<gdim_tot; i++)
		gmod->mean[i] += tmp[i]/gmod->count;

	gmod->var->scale((gmod->count - 1.0)/gmod->count);
	gmod->var->accum(tmp, 1.0/gmod->count);

	delete [] tmp;
};

void IscMarkovGaussMicroModel::remove(intfloat* vec) {
	double *tmp = new double[gdim_tot];
	IscMgdGaussComp* gmod = getGaussComponent();

	for (int i=0; i<gdim_tot; i++)
		tmp[i] = vec[gindv[i]].f - gmod->mean[i];
	gmod->count -= 1.0;
	for (int i=0; i<gdim_tot; i++)
		gmod->mean[i] -= tmp[i]/gmod->count;
	gmod->var->accum(tmp, -1.0/gmod->count);
	gmod->var->scale((gmod->count + 1.0)/gmod->count);

	delete [] tmp;
};

void IscMarkovGaussMicroModel::reset() {

	IscMgdGaussComp* gmod = getGaussComponent();
	gmod->count = modelinfo.prior_n;
	for (int i=0; i<gdim_tot; i++)
		gmod->mean[i] = 0.0;
	gmod->var->unit();
	gmod->var->scale(modelinfo.prior_v);

	modelinfo.reset();
}


IscMgdGaussComp* IscMarkovGaussMicroModel::getGaussComponent() {
	return gaussian_component;
}

IscMarkovGaussMatrixMicroModel::IscMarkovGaussMatrixMicroModel(int* vector_index, int vector_length, int slots_per_row) {
	this->vector_index = new int[vector_length];
	for(int i=0; i < vector_length;i++) {
		this->vector_index[i] = vector_index[i];
	}
	this->vector_length = vector_length;
	this->slots_per_row = slots_per_row;

	if(DEBUG)
		printf("Start creating components\n");

	IscMarkovGaussMicroModel** gaussian_components = new IscMarkovGaussMicroModel*[vector_length];

	for(int i=0; i < vector_length; i++) {
		int *predicted_index = new int[1];
		predicted_index[0] = i;
		int predicted_length =1;

		int *condiction_index;
		int condition_length;

		if((i+1) % slots_per_row == 0) { // If last element on a row
			if(i < vector_length-slots_per_row) { // Not the last row in the matrix
				condiction_index = new int[1];
				condiction_index[0] = i+slots_per_row; // Row below
				condition_length = 1;
			} else { // If last element of the last row in the matrix
				condiction_index = new int[0]; // No conditioning
				condition_length = 0;
			}
		} else if ( i >=  vector_length-slots_per_row) { // If last row in the matrix
			condiction_index = new int[1];
			condiction_index[0] = i+1; // Element to the right.
			condition_length = 1;
		} else {
			condiction_index = new int[2];
			condiction_index[0] = i+1;// Element to the right
			condiction_index[1] = i+slots_per_row; // and to below.
			condition_length = 2;
		}
		if(DEBUG)
			printf("Create component %i\n", i);
		gaussian_components[i] = new IscMarkovGaussMicroModel(
				predicted_index,
				predicted_length,
				condiction_index,
				condition_length
		);

		delete [] predicted_index;
		delete [] condiction_index;

	}

	markovModel = new IscMarkovGaussCombinerMicroModel(gaussian_components, vector_length);

	if(gaussian_components) {
		for(int i=0; i < vector_length; i++) {
			if(gaussian_components[i]) {
				delete gaussian_components[i];
			}
		}

		delete [] gaussian_components;
	}

	if(DEBUG)
		printf("Ready creating components\n");
}

IscMarkovGaussMatrixMicroModel::~IscMarkovGaussMatrixMicroModel() {
	if(DEBUG) {
		printf("IscMarkovGaussMatrixMicroModel delete started\n");
	}
	if(markovModel) {
		delete markovModel;
	}

	if(DEBUG) {
		printf("IscMarkovGaussMatrixMicroModel delete next\n");
	}

	if(vector_index) {
		delete [] vector_index;
	}

	if(DEBUG) {
		printf("IscMarkovGaussMatrixMicroModel deleted\n");
	}
}
