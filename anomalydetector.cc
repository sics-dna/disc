 /*
 --------------------------------------------------------------------------
 Copyright (C) 2014, 2015, 2016, 2017 SICS Swedish ICT AB

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
#include "format.hh"
#include "data.hh"
#include "sorteddynindvector.hh"
#include "isc_micromodel.hh"
#include "isc_component.hh"
#include "isc_mixture.hh"
#include "anomalydetector.hh"

#define MAXINT 2147483647

#ifdef _MSC_VER /* Windows */
#include <float.h>
#include <stdlib.h>
#define fp_is_normal(x) (_fpclass(x) != _FPCLASS_QNAN && _fpclass(x) != _FPCLASS_NINF && _fpclass(x) != _FPCLASS_PINF)
#else /* Linux */
#define fp_is_normal(x) (fpclassify(x) != FP_NAN && fpclassify(x) != FP_INFINITE)
#endif

/*
class AnomalyDetector {
public:
  AnomalyDetector();
  AnomalyDetector(int n, int off, int splt, double th, int cl);  // Sublasses must know the numbers and types of micromodels
  AnomalyDetector(int n, int off, int splt, double th, int cl, IscCombinationRule cr, IscCreateFunc cf);  // Or a creation function for the appropriate micromodels can be used
  virtual ~AnomalyDetector();
  virtual void SetParams(int off, int splt, double th, int cl);
  virtual void Reset();
  virtual void TrainOne(union intfloat* vec);
  virtual void TrainData(class DataObject* d);
  virtual void CalcAnomaly(class DataObject* d, double* devs);
  virtual void ClassifyData(class DataObject* d, int* cla, int* clu);
  virtual int CalcAnomalyDetails(union intfloat* vec, double& anom, int& cla, int& clu, double* devs=0, union intfloat* peak=0, union intfloat* min=0, union intfloat* max=0, double* expect=0, double* var=0);
  virtual int CalcAnomalyDetailsSingle(union intfloat* vec, int mmind, int cla, int clu, double* devs=0, union intfloat* peak=0, union intfloat* min=0, union intfloat* max=0, double* expect=0, double* var=0);
protected:
  IscMixture* isc;
  int len;
  int offset;
  int split_attr;
  double thres;
  int clustering;
};
*/

// no split/fix split/unknown split value
// nothres/filtering/clustering/semisuperised
// streaming/batch

AnomalyDetector::AnomalyDetector(int n, int off, int splt, double th, int cl)
{
  isc = 0;  // Subclasses should create the mixture with the appropriate micromodel creator func
  len = n;
  offset = off;
  split_attr = splt;
  thres = th;
  clustering = cl;
}

AnomalyDetector::AnomalyDetector(int n, int off, int splt, double th, int cl, IscCombinationRule cr, IscCreateFunc cf)
{
  len = n;
  offset = off;
  split_attr = splt;
  thres = th;
  clustering = cl;
  isc = new IscMixture(len, cr, cf, this); 
}

AnomalyDetector::AnomalyDetector()
{
}

AnomalyDetector::~AnomalyDetector()
{
  delete isc;
}

void AnomalyDetector::SetParams(int off, int splt, double th, int cl)
{
  offset = off;
  split_attr = splt;
  thres = th;
  clustering = cl;
}

void AnomalyDetector::Reset()
{
  isc->reset();
}

void AnomalyDetector::TrainOne(union intfloat* vec)
{
  int id;
  IscComponent *c;
  if (split_attr != -1 && vec[split_attr].i == -1) {
    return;
// For now samples with unknown class are not handled
// With semisupervised they should be handled but slightly different
//    if (thres == 0)
//      thres = isc->anomaly(vec+offset);
//    c = isc->classify(vec+offset, thres);
//    if (c)
//      c->add(vec+offset);
  } else {
    id = (split_attr == -1 ? -1 : vec[split_attr].i);
    if (clustering)
      c = isc->classify(vec+offset, thres, id);
    else
      c = isc->get_component(id, 0);
    if (!c){
      c = isc->add_component(id);
      c->reset();
    }

    if (thres == 0.0 || clustering || c->anomaly(vec+offset) <= thres)
      c->add(vec+offset);
  }
}

/**
 * Only works if no clustering is used and the anomaly training threshold is set to 0.0.
 */
void AnomalyDetector::UntrainOne(union intfloat* vec)
{
  int id;
  IscComponent *c;
  if (split_attr != -1 && vec[split_attr].i == -1 or clustering or thres > 0.0) {
    return;
// For now samples with unknown class are not handled
// With semisupervised they should be handled but slightly different
//    if (thres == 0)
//      thres = isc->anomaly(vec+offset);
//    c = isc->classify(vec+offset, thres);
//    if (c)
//      c->add(vec+offset);
  } else {
    id = (split_attr == -1 ? -1 : vec[split_attr].i);
    c = isc->get_component(id, 0);
    if(c)
      c->remove(vec+offset);
  }
}

void AnomalyDetector::TrainData(class DataObject* d)
{
  int i, id, ch;
  IscComponent *c;
  intfloat* vec;
  int* mark;
  double* dev;
  SortedDynamicIndexVector<int>* numclu_cla;
  SortedDynamicIndexVector<double>* maxdev_cla;
  double md, dummy;
  int n = d->size();
  int any;
// Dont handle unknowns nor semisupervised for now
  mark = new int[n];
  dev = new double[n];
  numclu_cla = new SortedDynamicIndexVector<int>(0);
  maxdev_cla = new SortedDynamicIndexVector<double>(0.0);
  for (i=0; i<n; i++)
    mark[i] = 0;
  do {
    isc->reset();
    ch = 0;
    id = -1;
    // training
    for (i=0; i<n; i++) {
      if (mark[i] == -1)
        continue;
      vec = (*d)[i];
      if (split_attr != -1) {
        if (vec[split_attr].i == -1) {
          mark[i] = -1;
          continue;
        }
        id = vec[split_attr].i;
      } else
        id = 0; // If no split, all belongs to class 0
      c = isc->get_component(id, mark[i]);
      if (!c) {
        c = isc->add_component(id);
        c->reset();
        c->cluster_id = mark[i];
      }
      c->add(vec+offset);
    }
    if (thres == 0.0)
      break;
    // calculating anomalies
    md = 0.0;
    FORDIV(*maxdev_cla, dummy, id) (*maxdev_cla)[id] = 0.0;
    FORDIV(*numclu_cla, dummy, id) (*numclu_cla)[id] = 0;
    any = 0;
    for (i=0; i<n; i++) {
      if (mark[i] == -1)
        continue;
      any++;
      vec = (*d)[i];
      if (split_attr != -1)
        id = vec[split_attr].i;
      else
        id = 0;
      c = isc->classify_forced(vec+offset, thres, id, dev[i]);
      if (id == -1) id = c->class_id;
      if (dev[i] <= thres && c->cluster_id != mark[i]) {
        ch = 1;
        mark[i] = c->cluster_id;
      }
      if (mark[i] >= (*numclu_cla)[id])
        (*numclu_cla)[id] = mark[i]+1;
      if ((*maxdev_cla)[id] < dev[i])
        (*maxdev_cla)[id] = dev[i];
      if (md < dev[i])
        md = dev[i];
    }
    if (any == 0 || (md <= thres && ch == 0)) {
      if (clustering) {
        any = 0;
        for (i=0; i<n; i++)
          if (mark[i] == -1) {
            any++;
            vec = (*d)[i];
            if (split_attr != -1)
              id = vec[split_attr].i;
            if (id == -1)
              id = 0;
            mark[i] = (*numclu_cla)[id];
          }
        if (any < 20) // Hard wired limit... Should be configurable
          break;
        else
          continue;
      } else
        break;
    }
    // Filter out those above upper threshold
    md = (0.8 * md > thres ? 0.8 * md : thres);
    for (i=0; i<n; i++) {
      if (mark[i] == -1)
        continue;
      vec = (*d)[i];
      if (split_attr != -1)
        id = vec[split_attr].i;
      if (id == -1)
        id = 0;
      if (!fp_is_normal(dev[i]) || dev[i] > md)
        mark[i] = -1; // (clustering ? (*numclu_cla)[id] : -1);
    }
  } while (1);
  delete [] mark;
  delete [] dev;
  delete maxdev_cla;
  delete numclu_cla;
}

int countval(int* vec, int num, int val)
{
  int res = 0;
  for (int i=0; i<num; i++)
    if (vec[i] == val)
      res++;
  return res;
}

void AnomalyDetector::CalcAnomaly(class DataObject* d, double* devs)
{
  int i, id = -1;
  intfloat* vec;
  int n = d->size();
  for (i=0; i<n; i++) {
    vec = (*d)[i];
    if (split_attr != -1) {
      id = vec[split_attr].i;
//      if (id == -1) {
//        devs[i] = 0.0;
//        continue;
//      }
    }

    float anom = isc->anomaly(vec+offset, id);
    devs[i] = anom;
  }
}

void AnomalyDetector::ClassifyData(class DataObject* d, int* cla, int* clu)
{
  int i, id = -1;
  intfloat* vec;
  IscComponent* c;
  int n = d->size();
  for (i=0; i<n; i++) {
    vec = (*d)[i];
    if (split_attr != -1)
      id = vec[split_attr].i;
    c = isc->classify(vec+offset, thres, id);
    if (c) {
      if (cla) cla[i] = c->class_id;
      if (clu) clu[i] = c->cluster_id;
    } else {
      if (cla) cla[i] = -1;
      if (clu) clu[i] = -1;
    }
  }
}


int AnomalyDetector::CalcAnomalyDetails(union intfloat* vec, double& anom, int& cla, int& clu, double* devs, union intfloat* peak, union intfloat* min, union intfloat* max, double* expect, double* var)
{
  int ok, id = -1;
  int flags = 0;
  IscComponent* c;
  if (split_attr != -1)
    id = vec[split_attr].i;
  c = isc->classify_forced(vec+offset, thres, id, anom);
  if (!c) return flags;
  cla = c->class_id;
  clu = c->cluster_id;
  flags |= 7;
  if (devs) {
    ok = c->partsanomaly(vec+offset, devs);
    if (ok)
      flags |= 8;
  }
  if (peak || (min && max)) {
    ok = c->invanomaly(vec+offset, thres, (min ? min+offset : 0), (max ? max+offset : 0), (peak ? peak+offset : 0));
    if (ok)
      flags |= ((min && max) ? 48 : 0) | (peak ? 64 : 0);
  }
  if (expect || var) {
    ok = c->stats(vec+offset, (expect ? expect+offset : 0), (var ? var+offset : 0));
    if (ok)
      flags |= (expect ? 128 : 0) | (var ? 256 : 0);
  }
  return flags;
}


int AnomalyDetector::CalcAnomalyDetailsSingle(union intfloat* vec, int mmind, int cla, int clu, double* devs, union intfloat* peak, union intfloat* min, union intfloat* max, double* expect, double* var)
{
  int ok;
  int flags = 0;
  IscComponent* c;
  c = isc->get_component(cla, clu);
  if (!c) return flags;
  if (mmind < 0 || mmind >= c->nummicromodels()) return flags;
  if (devs) {
    devs[mmind] = c->micromodel(mmind)->anomaly(vec+offset);
    flags |= 8;
  }
  if (peak || (min && max)) {
    ok = c->micromodel(mmind)->invanomaly(vec+offset, thres, (min ? min+offset : 0), (max ? max+offset : 0), (peak ? peak+offset : 0));
    if (ok)
      flags |= ((min && max) ? 48 : 0) | (peak ? 64 : 0);
  }
  if (expect || var) {
    ok = c->micromodel(mmind)->stats(vec+offset, (expect ? expect+offset : 0), (var ? var+offset : 0));
    if (ok)
      flags |= (expect ? 128 : 0) | (var ? 256 : 0);
  }
  return flags;
}


