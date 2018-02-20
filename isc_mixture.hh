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


#ifndef IscMixture_HH_
#define IscMixture_HH_

#include "dynindvector.hh"
#include "isc_exportimport.hh"


class IscMixture {
public:
  IscMixture(int len, IscCombinationRule cr, IscCreateFunc cf, void* co);
  virtual ~IscMixture();

  virtual void importModel(IscAbstractModelImporter* importer);
  virtual void exportModel(IscAbstractModelExporter* exporter);

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

#endif
