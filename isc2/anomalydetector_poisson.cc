 /*
 --------------------------------------------------------------------------
 Copyright (C) 2014, 2015 SICS Swedish ICT AB

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
#include "intfloat.hh"
#include "isc_micromodel.hh"
#include "isc_micromodel_poissongamma.hh"
#include "isc_component.hh"
#include "isc_mixture.hh"
#include "anomalydetector.hh"

IscMicroModel* poisson_create_func(const void* co, int ind)
{
  return new IscPoissonMicroModel(2*ind, 2*ind+1);
}

class AnomalyDetectorPoisson : public AnomalyDetector {
  AnomalyDetectorPoisson(int len) : AnomalyDetector(len, 0, -1, 14.0, 0) { isc = new IscMixture(len, IscMax, poisson_create_func, this); };
};

// Används som:
// AnomalyDetector* anomaly_detector;
// anomaly_detector = new AnomalyDetectorPoisson(5);
