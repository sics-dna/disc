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

#include <math.h>
#include "dynindvector.hh"
#include "isc_component.hh"
#include "isc_mixture.hh"

#ifdef WIN32
#include <FLOAT.H>
#define HUGE_VALF FLT_MAX
#endif

/*

class IscMixture {
public:
  IscMixture(int len, IscCombinationRule cr, IscCreateFunc cf, void* co);
  virtual ~IscMixture();

  // Administration
  virtual IscComponent* add_component(int id);
  virtual void remove_component(IscComponent* c);
  virtual IscComponent* get_component(int cla, int clu);
  virtual IscComponent* nth_component(int ind);

  // Read out anomaly and log predicted prob
  virtual double anomaly(intfloat* vec, int id = -1);
  virtual double logp(intfloat* vec, int id = -1);
  virtual IscComponent* classify(intfloat* vec, double th, int id = -1);
  virtual IscComponent* classify_forced(intfloat* vec, double th, int id, double& anom);

  // Training
  virtual void reset();
protected:
  int num;
  int len;
  IscCombinationRule comb;
  IscCreateFunc createfunc;
  void* createobj;
  class DynamicIndexVector<IscComponent*>* components;
};
 */


IscMixture::IscMixture(int ll, IscCombinationRule cr, IscCreateFunc cf, void* co)
{
	num = 0;
	len = ll;
	comb = cr;
	createfunc = cf;
	createobj = co;
	components = new DynamicIndexVector<IscComponent*>(0);
}

IscMixture::~IscMixture()
{
	IscComponent *c1, *c2;
	int ind;
	FORDIV((*components), c1, ind)
	for (c2=0; c1; c1=c2) {
		c2 = c1->next;
		delete c1;
	}
	delete components;
}

void IscMixture::importModel(IscAbstractModelImporter *importer) {
	importer->fillParameter("num",num);

	IscComponent* c;
	for(int i=0; i < num; i++) {
		c = add_component(i);
		IscAbstractModelImporter *compImporter = importer->getModelImporter(i);
		c->importModel(compImporter);
		delete compImporter;
	}
}

void IscMixture::exportModel(IscAbstractModelExporter *exporter) {
	exporter->addParameter("Component", "IscMixture");
	exporter->addParameter("num",num);
	IscComponent* c;
	int ind;
	int i=0;
	FORDIV((*components), c, ind)
	for (; c; c=c->next) {
		IscAbstractModelExporter *compExporter = exporter->createModelExporter(i);
		c->exportModel(compExporter);
		i++;
		delete compExporter;
	}
}

IscComponent* IscMixture::add_component(int id)
{
	IscComponent *c1, *c2;
	int clu = 0;
	c2 = 0;
	c1 = (*components)[id];
	for (; c1; c2=c1, c1=c1->next)
		if (clu <= c1->cluster_id)
			clu = c1->cluster_id+1;
	c1 = new IscComponent(id, clu, len, comb, createfunc, createobj);
	if (!c2) {
		(*components)[id] = c1;
	} else {
		c2->next = c1;
	}
	num++;
	return c1;
}

void IscMixture::remove_component(IscComponent* c)
{
	IscComponent *c1, *c2;
	c1 = (*components)[c->class_id];
	if (c1) {
		for (c2=0; c1 && c1 != c; c2=c1, c1=c1->next);
		if (c2)
			c2->next = c1->next;
		else
			(*components)[c->class_id] = c1->next;
		delete c1;
		num--;
	}
}

IscComponent* IscMixture::get_component(int cla, int clu)
{
	IscComponent* c;
	for (c=(*components)[cla]; c && (c->cluster_id != clu); c=c->next);
	return c;
}

IscComponent* IscMixture::nth_component(int n)
{
	int i = 0, ind;
	IscComponent* c=0;
	FORDIV((*components), c, ind) {
		for (; c && i < n; c=c->next, i++);
	}
	return c;
}

double IscMixture::anomaly(intfloat* vec, int id)
{
	IscComponent* c;
	double a, am;
	int ind;
	am = HUGE_VALF;
	if (id == -1) {
		FORDIV((*components), c, ind)
    		  for (; c; c=c->next) {
    			  a = c->anomaly(vec);
    			  if (a < am) am = a;
    		  }
	} else {
		if (0 == (*components)[id])
			return 0.0;
		for (c=(*components)[id]; c; c=c->next) {
			a = c->anomaly(vec);
			if (a < am) am = a;
		}
	}
	return am;
}

double IscMixture::logp(intfloat* vec, int id)
{
	IscComponent* c;
	double lp, lpm;
	int ind;
	lpm = -HUGE_VALF;
	if (id == -1) {
		FORDIV((*components), c, ind)
    		  for (; c; c=c->next) {
    			  lp = c->logp(vec);
    			  if (lp > lpm) lpm = lp;
    		  }
	} else {
		for (c=(*components)[id]; c; c=c->next) {
			lp = c->logp(vec);
			if (lp > lpm) lpm = lp;
		}
	}
	return lpm;
}

IscComponent* IscMixture::classify(intfloat* vec, double th, int id)
{
	IscComponent* c;
	IscComponent* cm = 0;
	double a, lp, lpm;
	int ind;
	lpm = -HUGE_VALF;
	if (id == -1) {
		FORDIV((*components), c, ind)
    		  for (; c; c=c->next) {
    			  a = c->anomaly(vec);
    			  if (a <= th) {
    				  lp = c->logp(vec);
    				  if (lp > lpm) {
    					  lpm = lp;
    					  cm = c;
    				  }
    			  }
    		  }
	} else {
		for (c=(*components)[id]; c; c=c->next) {
			a = c->anomaly(vec);
			if (a <= th) {
				lp = c->logp(vec);
				if (lp > lpm) {
					lpm = lp;
					cm = c;
				}
			}
		}
	}
	return cm;
}

IscComponent* IscMixture::classify_forced(intfloat* vec, double th, int id, double& anom)
{
	IscComponent* c;
	IscComponent* cm = 0;
	double a, lp, lpm;
	int ind;
	lpm = -HUGE_VALF;
	anom = HUGE_VALF;
	if (id == -1) {
		FORDIV((*components), c, ind)
    		  for (; c; c=c->next) {
    			  a = c->anomaly(vec);
    			  if (a <= th) {
    				  lp = c->logp(vec);
    				  if (cm == 0 || lp > lpm) {
    					  lpm = lp;
    					  cm = c;
    					  anom = a;
    				  }
    			  } else if (a < anom) {
    				  cm = c;
    				  anom = a;
    			  }
    		  }
	} else {
		for (c=(*components)[id]; c; c=c->next) {
			a = c->anomaly(vec);
			if (a <= th) {
				lp = c->logp(vec);
				if (cm == 0 || lp > lpm) {
					lpm = lp;
					cm = c;
					anom = a;
				}
			} else if (a < anom) {
				cm = c;
				anom = a;
			}
		}
	}
	return cm;
}

void IscMixture::reset()
{
	IscComponent* c;
	int ind;
	FORDIV((*components), c, ind)
	for (; c; c=c->next)
		c->reset();
}

