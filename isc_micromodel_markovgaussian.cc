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

#include "isc_micromodel_markovgaussian.hh"


virtual void IscMarkovGaussMicroModel::add_acc(IscMgdAccumulator *acc, intfloat* vec) {
	double *tmp = new double[gdim_tot];
	IscMgdGaussComp* gmod = getGaussComponent();

	acc->sqdist += gmod->var->sqprodinv_cond(tmp, gdim_conditioned);
	acc->logdet += log(gmod->var->det_cond(gdim_conditioned));
	delete [] tmp;

};


// Training
virtual void IscMarkovGaussMicroModel::add(intfloat* vec) {
	double *tmp = new double[gdim_tot];
	IscMgdGaussComp* gmod = getGaussComponent();

	for (int i=0; i<gdim_tot; i++)
		tmp[i] = vec[gindv[i]].f - mod2->mean[i];
	gmod->count += 1.0;
	for (int i=0; i<gdim_tot; i++)
		gmod->mean[i] += tmp[i]/gmod->count;
	gmod->var->accum(tmp, 1.0/gmod->count);
	gmod->var->scale((gmod->count - 1.0)/gmod->count);

	delete [] tmp;
};

virtual void IscMarkovGaussMicroModel::remove(intfloat* vec) {
	double *tmp = new double[gdim_tot];
	double lc0, lc1;
	IscMgdGaussComp* gmod = getGaussComponent();

	for (int i=0; i<gdim_tot; i++)
		tmp[i] = vec[gindv[i]].f - mod2->mean[i];
	gmod->count -= 1.0;
	for (int i=0; i<gdim_tot; i++)
		gmod->mean[i] -= tmp[i]/mod2->count;
	gmod->var->accum(tmp, -1.0/mod2->count);
	gmod->var->scale((gmod->count + 1.0)/gmod->count);

	delete [] tmp;
};

virtual void IscMarkovGaussMicroModel::reset() {

	IscMgdGaussComp* gmod = getGaussComponent();
	gmod->count = modelinfo.prior_n;
	for (int i=0; i<gdim_tot; i++)
		gmod->mean[i] = 0.0;
	gmod->var->unit();
	gmod->var->scale(modelinfo.prior_v);

	modelinfo.reset();
}


virtual IscMgdGaussComp* IscMarkovGaussMicroModel::getGaussComponent() {
	if(!mod2) {
		gaussian_component = new IscMgdGaussComp(gdim_tot, modelinfo.prior_n, modelinfo.prior_v);
	}
	return gaussian_component;
}


/*
 En dynindvec, nej hash, med "g1:g2:type" som nyckel, innehåll: count, mean, var.
 Eller en dynindvec med geo1*20+type, av dyninvec geo2, av  count, mean, var.
 */
