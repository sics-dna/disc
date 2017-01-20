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

#ifndef AnomalyDetector_HH_
#define AnomalyDetector_HH_

class AnomalyDetector {
public:
  AnomalyDetector();
  AnomalyDetector(int n, int off, int splt, double th, int cl);  // Sublasses must know the numbers and types of micromodels
  AnomalyDetector(int n, int off, int splt, double th, int cl, IscCombinationRule cr, IscCreateFunc cf);  // Or a creation function for the appropriate micromodels can be used
  virtual ~AnomalyDetector();
  virtual void SetParams(int off, int splt, double th, int cl);
  virtual void Reset();
  virtual void TrainOne(union intfloat* vec);
  virtual void UntrainOne(union intfloat* vec);

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

#endif
