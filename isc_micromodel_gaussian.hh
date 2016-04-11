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


// The one doing all the calculations. The rest are just wrappers.
class IscRawGaussianMicroModel {
public:
  IscRawGaussianMicroModel() {};
  virtual ~IscRawGaussianMicroModel() {};

  double raw_anomaly(double x, double c, double mn, double sc);
  double raw_logp(double x, double c, double mn, double sc);
  void raw_ev_logp(double c, double mn, double sc, double& e, double& v);
  double raw_maxlogp(double c, double mn, double sc, double& peakx);
  void raw_invanomaly(double c, double mn, double sc, double ano, double& minx, double& maxx);
  void raw_stats(double c, double mn, double sc, double& expect, double& var);
};

class IscGaussianMicroModel : public IscMicroModel, public IscRawGaussianMicroModel {
public:
  IscGaussianMicroModel(int ix) { indx=ix; };
  virtual ~IscGaussianMicroModel() {};
  virtual IscMicroModel* create() {
	  return new IscGaussianMicroModel(indx);
  }
  // Read out anomaly and log predicted prob
  virtual double anomaly(intfloat* vec) { return raw_anomaly(vec[indx].f, 0.5*n, sumx/n, (n+1.0)/n*(sumxx-sumx*sumx/n)); };
  virtual double logp(intfloat* vec) { return raw_logp(vec[indx].f, 0.5*n, sumx/n, (n+1.0)/n*(sumxx-sumx*sumx/n)); };

  virtual int ev_logp(intfloat* vec, double& e, double& v) { raw_ev_logp(0.5*n, sumx/n, (n+1.0)/n*(sumxx-sumx*sumx/n), e, v); return 1; };
  virtual int logpeak(intfloat* vec, double& p) { double dx; p = raw_maxlogp(0.5*n, sumx/n, (n+1.0)/n*(sumxx-sumx*sumx/n), dx); return 1; };
  virtual int invanomaly(intfloat* vec, double ano, intfloat* minvec, intfloat* maxvec, intfloat* peakvec) { double x1, x2; if (minvec && maxvec) { raw_invanomaly(0.5*n, sumx/n, (n+1.0)/n*(sumxx-sumx*sumx/n), ano, x1, x2); minvec[indx].f=x1, maxvec[indx].f=x2; } if (peakvec) { raw_maxlogp(0.5*n, sumx/n, (n+1.0)/n*(sumxx-sumx*sumx/n), x1); peakvec[indx].f=x1; } return 1; };
  virtual int stats(union intfloat* vec, double* expect, double* var) { double e, v; raw_stats(0.5*n, sumx/n, (n+1.0)/n*(sumxx-sumx*sumx/n), e, v); if (expect) expect[indx]=e; if (var) var[indx]=v; return 1; };

  // Training
  virtual void add(intfloat* vec) { n+=1.0; sumx+=vec[indx].f; sumxx+=vec[indx].f*vec[indx].f; };
  virtual void remove(intfloat* vec) { n-=1.0; sumx-=vec[indx].f; sumxx-=vec[indx].f*vec[indx].f; };
  virtual void reset() { sumx=0.0; sumxx = 0.0; n=0.0; };

  int indx;
  double sumx, sumxx, n;

};

class IscGaussianDriftMicroModel : public IscMicroModel, public IscRawGaussianMicroModel {
public:
  IscGaussianDriftMicroModel(int ix, int it) { indx=ix; indt=it; dirty=0; };
  virtual ~IscGaussianDriftMicroModel() {};
  virtual IscMicroModel* create() {
	  return new IscGaussianDriftMicroModel(indx,indt);
  }
  // Read out anomaly and log predicted prob
  virtual double anomaly(intfloat* vec) { update(); return raw_anomaly(0.0, cc, mean, scale); };
  virtual double logp(intfloat* vec) { update(); return raw_logp(0.0, cc, mean, scale); };

  virtual int ev_logp(intfloat* vec, double& e, double& v) { update(); raw_ev_logp(cc, mean, scale, e, v); return 1; };
  virtual int logpeak(intfloat* vec, double& p) { double dx; update(); p = raw_maxlogp(cc, mean, scale, dx); return 1; };
  virtual int invanomaly(intfloat* vec, double ano, intfloat* minvec, intfloat* maxvec, intfloat* peakvec) { double x1, x2; update(); if (minvec && maxvec) { raw_invanomaly(cc, mean, scale, ano, x1, x2); minvec[indx].f=x1, maxvec[indx].f=x2; } if (peakvec) { raw_maxlogp(cc, mean, scale, x1); peakvec[indx].f=x1; } return 1; };
  virtual int stats(union intfloat* vec, double* expect, double* var) { double e, v; update(); raw_stats(cc, mean, scale, e, v); if (expect) expect[indx]=e; if (var) var[indx]=v; return 1; };

  // Training
  virtual void add(intfloat* vec) { n+=1.0; sumx+=vec[indx].f; sumxx+=vec[indx].f*vec[indx].f; sumt+=vec[indt].f; sumtt+=vec[indt].f*vec[indt].f; sumxt+=vec[indx].f*vec[indt].f; dirty=1; };
  virtual void remove(intfloat* vec) { n-=1.0; sumx-=vec[indx].f; sumxx-=vec[indx].f*vec[indx].f; sumt-=vec[indt].f; sumtt-=vec[indt].f*vec[indt].f; sumxt-=vec[indx].f*vec[indt].f; dirty=1; };
  virtual void reset() { sumx=0.0; sumxx = 0.0; sumt=0.0; sumtt = 0.0; sumxt=0.0; n=0.0; dirty=1; };
  void update() { if (dirty) { cc=0.5*(n-1); mean=(sumxt-sumx*sumt/n)/(sumtt-sumt*sumt/n); scale=(sumxx-sumx*sumx/n)/(sumtt-sumt*sumt/n)-mean*mean; dirty=0; } };

  int dirty;
  int indx, indt;
  double sumx, sumxx, sumt, sumtt, sumxt, n;
  double cc, scale, mean;
};

class IscGaussianAverageMicroModel : public IscMicroModel, public IscRawGaussianMicroModel {
public:
  IscGaussianAverageMicroModel(int ix, int is) { indx=ix; inds=is; };
  virtual ~IscGaussianAverageMicroModel() {};
  virtual IscMicroModel* create() {
	  return new IscGaussianAverageMicroModel(indx,inds);
  };
  // Read out anomaly and log predicted prob
  virtual double anomaly(intfloat* vec) { update(); return raw_anomaly(vec[indx].f, cc, mean, (sums+vec[inds].f)/(sums*vec[inds].f)*scale); };
  virtual double logp(intfloat* vec) { update(); return raw_logp(vec[indx].f, cc, mean, (sums+vec[inds].f)/(sums*vec[inds].f)*scale); };

  virtual int ev_logp(intfloat* vec, double& e, double& v) { update(); raw_ev_logp(cc, mean, (sums+vec[inds].f)/(sums*vec[inds].f)*scale, e, v); return 1; };
  virtual int logpeak(intfloat* vec, double& p) { double dx; update(); p = raw_maxlogp(cc, mean, (sums+vec[inds].f)/(sums*vec[inds].f)*scale, dx); return 1; };
  virtual int invanomaly(intfloat* vec, double ano, intfloat* minvec, intfloat* maxvec, intfloat* peakvec) { double x1, x2; update(); if (minvec && maxvec) { raw_invanomaly(cc, mean, (sums+vec[inds].f)/(sums*vec[inds].f)*scale, ano, x1, x2); minvec[indx].f=x1, maxvec[indx].f=x2; } if (peakvec) { raw_maxlogp(cc, mean, (sums+vec[inds].f)/(sums*vec[inds].f)*scale, x1); peakvec[indx].f=x1; } return 1; };
  virtual int stats(union intfloat* vec, double* expect, double* var) { double e, v; update(); raw_stats(cc, mean, (sums+vec[inds].f)/(sums*vec[inds].f)*scale, e, v); if (expect) expect[indx]=e; if (var) var[indx]=v; return 1; };

  // Training
  virtual void add(intfloat* vec) { n+=1.0; sums+=vec[inds].f; sumsx+=vec[inds].f*vec[indx].f; sumsxx+=vec[inds].f*vec[indx].f*vec[indx].f; dirty = 1;};
  virtual void remove(intfloat* vec) { n-=1.0; sums-=vec[inds].f; sumsx-=vec[inds].f*vec[indx].f; sumsxx-=vec[inds].f*vec[indx].f*vec[indx].f; dirty = 1; };
  virtual void reset() { sums=0.0; sumsx=0.0; sumsxx = 0.0; n=0.0; dirty = 1; };
  void update() { if (dirty) { cc=0.5*n; mean=sumsx/sums; scale=(sumsxx-sumsx*sumsx/sums); dirty=0; } };

  int dirty;
  int indx, inds;
  double sums, sumsx, sumsxx, n;
  double cc, scale, mean;
};

class IscGaussianAverageDriftMicroModel : public IscMicroModel, public IscRawGaussianMicroModel {
public:
  IscGaussianAverageDriftMicroModel(int ix, int is, int it) { indx=ix; inds=is; indt=it; };
  virtual ~IscGaussianAverageDriftMicroModel() {};
  virtual IscMicroModel* create() {
	  return new IscGaussianAverageDriftMicroModel(indx,inds,indt);
  };
  // Read out anomaly and log predicted prob
  virtual double anomaly(intfloat* vec) { update(); return raw_anomaly(0.0, cc, mean, scale); };
  virtual double logp(intfloat* vec) { update(); return raw_logp(0.0, cc, mean, scale); };

  virtual int ev_logp(intfloat* vec, double& e, double& v) { update(); raw_ev_logp(cc, mean, scale, e, v); return 1; };
  virtual int logpeak(intfloat* vec, double& p) { double dx; update(); p = raw_maxlogp(cc, mean, scale, dx); return 1; };
  virtual int invanomaly(intfloat* vec, double ano, intfloat* minvec, intfloat* maxvec, intfloat* peakvec) { double x1, x2; update(); if (minvec && maxvec) { raw_invanomaly(cc, mean, scale, ano, x1, x2); minvec[indx].f=x1, maxvec[indx].f=x2; } if (peakvec) { raw_maxlogp(cc, mean, scale, x1); peakvec[indx].f=x1; } return 1; };
  virtual int stats(union intfloat* vec, double* expect, double* var) { double e, v; update(); raw_stats(cc, mean, scale, e, v); if (expect) expect[indx]=e; if (var) var[indx]=v; return 1; };

  // Training
  virtual void add(intfloat* vec) { n+=1.0; sums+=vec[inds].f; sumsx+=vec[inds].f*vec[indx].f; sumsxx+=vec[inds].f*vec[indx].f*vec[indx].f; sumst+=vec[inds].f*vec[indt].f; sumstt+=vec[inds].f*vec[indt].f*vec[indt].f; sumsxt+=vec[inds].f*vec[indx].f*vec[indt].f; dirty = 1; };
  virtual void remove(intfloat* vec) { n-=1.0; sums-=vec[inds].f; sumsx-=vec[inds].f*vec[indx].f; sumsxx-=vec[inds].f*vec[indx].f*vec[indx].f; sumst-=vec[inds].f*vec[indt].f; sumstt-=vec[inds].f*vec[indt].f*vec[indt].f; sumsxt-=vec[inds].f*vec[indx].f*vec[indt].f; dirty = 1; };
  virtual void reset() { sums=0.0; sumsx=0.0; sumsxx=0.0; sumst=0.0; sumstt=0.0; sumsxt=0.0; n=0.0; dirty = 1; };
  void update() { if (dirty) { cc=0.5*(n-1); mean=(sumsxt-sumsx*sumst/sums)/(sumstt-sumst*sumst/sums); scale=(sumsxx-sumsx*sumsx/sums)/(sumstt-sumst*sumst/sums)-mean*mean; dirty=0; } };  // Temporary added discretization variance of 0.25

  int dirty;
  int indx, inds, indt;
  double sums, sumsx, sumsxx, sumst, sumstt, sumsxt, n;
  double cc, scale, mean;
};

