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


virtual void IscMarkovGaussMicroModel::update_acc(IscMgdAccumulator* acc, intfloat* vec) {
	double sum, sqsum, totcount, p, lp;

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
			double *tmp = new double[gdim_tot];
			for (int i=0; i<gdim_tot; i++)
				tmp[i] = vec[gindv[i]].f - mod2->mean[i];
			FORDIV(mod1->vector, mod2tmp, index2)
			if (mod2tmp) {
				/**/
				double dist = mod2tmp->var->sqprodinv_part(tmp, gdim_predicted);
				double det = mod2tmp->var->det_part(gdim_predicted);
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
			acc->sqdist += mod2->var->sqprodinv_cond(tmp, gdim_conditioned);
			acc->logdet += log(mod2->var->det_cond(gdim_conditioned));
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
		int category = static_cast<int>(vec[catind].i);
		double margnum;
		if (category != -1)
			mod1 = model[category];
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


// Training
virtual void IscMarkovGaussMicroModel::add(intfloat* vec) {
	double *tmp = new double[gdim_tot];
	double lc1, lc0;
	IscMgdVector* mod1;
	IscMgdGaussComp* mod2;
	lookup(vec, 1, mod1, mod2);
	if (mod2) {
		for (int i=0; i<gdim_tot; i++)
			tmp[i] = vec[gindv[i]].f - mod2->mean[i];
		mod2->count += 1.0;
		for (int i=0; i<gdim_tot; i++)
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

virtual void IscMarkovGaussMicroModel::remove(intfloat* vec) {
	double *tmp = new double[gdim_tot];
	double lc0, lc1;
	IscMgdVector* mod1;
	IscMgdGaussComp* mod2;
	lookup(vec, 1, mod1, mod2);
	if (mod2) {
		for (int i=0; i<gdim_tot; i++)
			tmp[i] = vec[gindv[i]].f - mod2->mean[i];
		mod2->count -= 1.0;
		for (int i=0; i<gdim_tot; i++)
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

virtual void IscMarkovGaussMicroModel::reset() {
	IscMgdVector* mod1;
	IscMgdGaussComp* mod2;
	int index1, index2;
	FORDIV(model, mod1, index1)
		if (mod1) {
			mod1->margin_num = 0.0;
			FORDIV(mod1->vector, mod2, index2)
				if (mod2) {
					mod2->count = modelinfo.prior_n;
					for (int i=0; i<gdim_tot; i++)
						mod2->mean[i] = 0.0;
					mod2->var->unit();
					mod2->var->scale(modelinfo.prior_v);
				}
		}
	modelinfo.reset();
}


void IscMarkovGaussMicroModel::lookup(intfloat* vec, int insert, IscMgdVector*& mod1, IscMgdGaussComp*& mod2) {
	int category = static_cast<int>(vec[catind].i);
	if (category != -1) {
		mod1 = model[category];
		if (mod1) {
			mod2 = mod1->vector[0];
			if (!mod2 && insert)
				mod2 = mod1->vector[0] = new IscMgdGaussComp(gdim_tot, modelinfo.prior_n, modelinfo.prior_v);
		} else if (insert) {
			mod1 = model[category] = new IscMgdVector();
			mod2 = mod1->vector[0] = new IscMgdGaussComp(gdim_tot, modelinfo.prior_n, modelinfo.prior_v);
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


/*
 En dynindvec, nej hash, med "g1:g2:type" som nyckel, innehåll: count, mean, var.
 Eller en dynindvec med geo1*20+type, av dyninvec geo2, av  count, mean, var.
 */
