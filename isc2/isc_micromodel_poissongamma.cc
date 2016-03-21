 /*
 --------------------------------------------------------------------------
 Copyright (C) 2010, 2015 SICS Swedish ICT AB

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
#include "gamma.hh"
#include "intfloat.hh"
#include "isc_micromodel.hh"
#include "isc_micromodel_poissongamma.hh"

#ifdef WIN32
#define round(x) floor(x+0.5)
#endif

/*

// The one doing all the calculations. The rest are just wrappers.
// Note that in the prior P(lambda)=lambda^-c, c is set to 0 below,
// otherwise r=0 produces infinities
class IscRawExponentialMicroModel {
public:
  IscRawExponentialMicroModel() {};
  virtual ~IscRawExponentialMicroModel() {};

  double raw_anomaly(double r, double t, double rr, double tt);
  double raw_logp(double r, double t, double rr, double tt);

  void raw_ev_logp(double rr, double tt, double& e, double& v);
  double raw_maxlogp(double rr, double tt, double& peakr, double& peakt);
  void raw_invanomaly(double rr, double tt, double ano, double& minr, double& maxr, double& mint, double& maxt);
  void raw_stats_r(double rr, double tt, double& expect, double& var) {}; // Not implemented in the general case
  void raw_stats_t(double rr, double tt, double& expect, double& var) {}; // Not implemented in the general case

  void raw_add(double r, double t) { sumr += r; sumt += t; };
  void raw_remove(double r, double t) { sumr -= r; sumt -= t; };
  void raw_reset() { sumr = 0.0; sumt = 0.0; };

  double sumr, sumt;
};

class IscExponentialMicroModel : public IscMicroModel, public IscRawExponentialMicroModel {
public:
  IscExponentialMicroModel(int ir, int it, int irr, int itt) { indr=ir; indt=it; indrr=irr; indtt=itt; };
  virtual ~IscExponentialMicroModel() {};

  // Read out anomaly and log predicted prob
  virtual double anomaly(intfloat* vec) { return raw_anomaly(vec[indr].f, vec[indt].f, vec[indrr].f, vec[indtt].f); };
  virtual double logp(intfloat* vec) { return raw_logp(vec[indr].f, vec[indt].f, vec[indrr].f, vec[indtt].f); };

  virtual int ev_logp(intfloat* vec, double& e, double& v) { raw_ev_logp(vec[indrr].f, vec[indtt].f, e, v); return 1; };
  virtual int logpeak(intfloat* vec, double& p) { double dr, dt; p = raw_maxlogp(vec[indrr].f, vec[indtt].f, dr, dt); return 1; };
  virtual int invanomaly(intfloat* vec, double ano, intfloat* minvec, intfloat* maxvec, intfloat* peakvec) { double r1, r2, t1, t2; if (minvec && maxvec) { raw_invanomaly(vec[indrr].f, vec[indtt].f, ano, r1, r2, t1, t2); minvec[indr].f=r1, maxvec[indr].f=r2, minvec[indt].f=t1, maxvec[indt].f=t2; } if (peakvec) { raw_maxlogp(vec[indrr].f, vec[indtt].f, r1, t1); peakvec[indr].f=r1, peakvec[indt].f=t1; } return 1; };

  // Training
  virtual void add(intfloat* vec) { raw_add(vec[indr].f, vec[indt].f); };
  virtual void remove(intfloat* vec) { raw_remove(vec[indr].f, vec[indt].f); };
  virtual void reset() { raw_reset(); };

  int indr, indt, indrr, indtt;
};

class IscPoissonMicroModel : public IscMicroModel, public IscRawExponentialMicroModel {
public:
  IscPoissonMicroModel(int ir, int it) { indr=ir; indt=it; };
  virtual ~IscPoissonMicroModel() {};

  // Read out anomaly and log predicted prob
  virtual double anomaly(intfloat* vec) { return raw_anomaly(vec[indr].f, vec[indt].f, HUGE_VALF, vec[indt].f); };
  virtual double logp(intfloat* vec) { return raw_logp(vec[indr].f, vec[indt].f, HUGE_VALF, vec[indt].f); };

  virtual int ev_logp(intfloat* vec, double& e, double& v) { raw_ev_logp(HUGE_VALF, vec[indt].f, e, v); return 1; };
  virtual int logpeak(intfloat* vec, double& p) { double dr, dt; p = raw_maxlogp(HUGE_VALF, vec[indt].f, dr, dt); return 1; };
  virtual int invanomaly(intfloat* vec, double ano, intfloat* minvec, intfloat* maxvec, intfloat* peakvec) { double r1, r2, t1, t2; if (minvec && maxvec) { raw_invanomaly(HUGE_VALF, vec[indt].f, ano, r1, r2, t1, t2); minvec[indr].f=r1, maxvec[indr].f=r2, minvec[indt].f=t1, maxvec[indt].f=t2; } if (peakvec) { raw_maxlogp(HUGE_VALF, vec[indt].f, r1, t1); peakvec[indr].f=r1, peakvec[indt].f=t1; } return 1; };
  virtual int stats(union intfloat* vec, double* expect, double* var) { double e; e=vec[indt].f*sumr/sumt; if (expect) expect[indr]=e; if (var) var[indr]=e; return 1; }; // This assumes pure Poisson, not the predictive distribution neg binomial


  // Training
  virtual void add(intfloat* vec) { raw_add(vec[indr].f, vec[indt].f); };
  virtual void remove(intfloat* vec) { raw_remove(vec[indr].f, vec[indt].f); };
  virtual void reset() { raw_reset(); };

  int indr, indt;
};

class IscGammaMicroModel  : public IscMicroModel, public IscRawExponentialMicroModel {
public:
  IscGammaMicroModel(int ir, int it) { indr=ir; indt=it; };
  virtual ~IscGammaMicroModel() {};

  // Read out anomaly and log predicted prob
  virtual double anomaly(intfloat* vec) { return raw_anomaly(vec[indr].f, vec[indt].f, vec[indr].f, vec[indt].f); };
  virtual double logp(intfloat* vec) { return raw_logp(vec[indr].f, vec[indt].f, vec[indr].f, vec[indt].f); };

  virtual int ev_logp(intfloat* vec, double& e, double& v) { raw_ev_logp(vec[indr].f, vec[indt].f, e, v); return 1; };
  virtual int logpeak(intfloat* vec, double& p) { double dr, dt; p = raw_maxlogp(vec[indr].f, vec[indt].f, dr, dt); return 1; };
  virtual int invanomaly(intfloat* vec, double ano, intfloat* minvec, intfloat* maxvec, intfloat* peakvec) { double r1, r2, t1, t2; if (minvec && maxvec) { raw_invanomaly(vec[indr].f, vec[indt].f, ano, r1, r2, t1, t2); minvec[indr].f=r1, maxvec[indr].f=r2, minvec[indt].f=t1, maxvec[indt].f=t2; } if (peakvec) { raw_maxlogp(vec[indr].f, vec[indt].f, r1, t1); peakvec[indr].f=r1, peakvec[indt].f=t1; } return 1; };

  // Training
  virtual void add(intfloat* vec) { raw_add(vec[indr].f, vec[indt].f); };
  virtual void remove(intfloat* vec) { raw_remove(vec[indr].f, vec[indt].f); };
  virtual void reset() { raw_reset(); };

  int indr, indt;
};
*/

const double prior_c = 0.0;

// log(P(r,t)), although the gamma part is scaled (with tt/rr) to make the
// gamma and poisson parts coinside at (rr, tt).
static double pa_logp(double r, double t, double rr, double tt, double sumr, double sumt)
{
  if((sumt == 0) || (t == 0))
    return ((((t == 0) && (r != 0)) || ((sumt == 0) && (sumr != 0))) ? -HUGE_VALF : 0);

  if (r >= rr)
    return lngamma(r+sumr+1) - lngamma(r) - lngamma(sumr+1) + (r-1)*log(t) + (sumr+1)*log(sumt) - (r+sumr+1)*log(sumt+t) + log(tt/rr);
  else
    return lngamma(r+sumr+1) - lngamma(r+1) - lngamma(sumr+1) + r*log(t) + (sumr+1)*log(sumt) - (r+sumr+1)*log(sumt+t);
}

static void sum_poisson_log(double s, double u, double rr, double tt, double& e, double& q)
{
  double r, p, ltut, z, zt, top = ceil(tt*s/u - 0.5);
  zt = z = pa_logp(top, tt, rr, tt, s, u);
  p = exp(z);
  ltut = log(tt/(u+tt));
  e += p*z;
  q += p*z*z;
  for (r=top-1; r>=0; r--) {
    z -= log((r+s+1)/(r+1)) + ltut;
    p = exp(z);
    if (p < 1e-12) break;
    e += p*z;
    q += p*z*z;
  }
  for (r=top+1, z=zt; r<rr; r++) {
    z += log((r+s)/r) + ltut;
    p = exp(z);
    if (p < 1e-12) break;
    e += p*z;
    q += p*z*z;
  }
}

/*
lngamma(r+s+1) - lngamma(r) - lngamma(s+1) + (r-1)*log(t) + (s+1)*log(u) - (r+s+1)*log(u+t) + log(tt/rr)
=  (lngamma(rr+s+1) - lngamma(rr) - lngamma(s+1) + (s+1)*log(u) + log(tt/rr)) +
   (rr-1)*log(t) - (rr+s+1)*log(u+t)
 */
static void integral_gamma_log(double s, double u, double rr, double tt, double& e, double& q)
{
  double sum=0.0, hsum=0.0, suml=0.0, hsuml=0.0, sumll=0.0, hsumll=0.0;
  double t, dt, c, ec, z, p;
  int i;
  dt = tt/128;
  c = lngamma(rr+s+1) - lngamma(rr) - lngamma(s+1) + (s+1)*log(u) + log(tt/rr);
  ec = exp(c);
  for (i=1, t=dt; i<=128; i++, t+=dt) {
    z = (rr-1)*log(t) - (rr+s+1)*log(u+t);
    p = exp(z);
    sum += p, suml += p*z, sumll += p*z*z;
    if (!(i%2))
      hsum += p, hsuml += p*z, hsumll += p*z*z;
  }
  sum = (4*sum*dt - 2*hsum*dt)/3.0;
  suml = (4*suml*dt - 2*hsuml*dt)/3.0;
  sumll = (4*sumll*dt - 2*hsumll*dt)/3.0;
  e += ec*(c*sum + suml);
  q += ec*(c*c*sum + 2*c*suml + sumll);
  /* 
p(t) = c*f(t), 
I(f(t))=sum
I(log(f(t))f(t))=suml
I(log2(f(t))f(t))=sumll
I(p(t))=c I(f(t)) 
I(log(p(t))p(t))=I(c log(c) f(t) + c log(f(t)) f(t))=c log(c) I(f(t)) + c I(log(f(t)) f(t))
I(log2(p(t))p(t))=I(c log2(c) f(t) + 2 c log(c) log(f(t)) f(t) + c log2(f(t)) f(t))=c log2(c) I(f(t)) + 2 c log(c) I(log(f(t)) f(t)) + c I(log2(f(t)) f(t))
   */
}

static double pa_sum(int y1, int y2, int delta, int z, double lp1, double lttu, double luut, double s, int& last)
{
  double sum = 0.0;
  double lp2, p3, lam, tmp, peak;
  int y;
  peak = 0.0;
  tmp = 1.0;
  for (y=y1; y!=y2 && tmp > peak*1e-12; y+=delta) {
    lp2 = lttu*y + lngamma(s+1.0-prior_c+y) - lngamma(1.0+y);
    if (y != z) {
      lam = exp((lngamma(y+1.0)-lngamma(z+1.0))/(y-z)-lttu);
      p3 = (y<z ? incgammaq(s+1.0-prior_c+y, lam) : incgammap(s+1.0-prior_c+y, lam));
    } else
      p3 = 1.0;
    sum += tmp = exp(lp1+lp2)*p3;
    if (tmp > peak) peak = tmp;
  }
  last = y-delta;
  return sum;
}

// The principal anomaly for a poisson distribution
static double principal_anomaly_poisson(double z, double t, double s, double u)
{
  double sum = 0.0, lp1, lttu, luut;
  int top = floor(t/u*(s-prior_c));
  int zz = (int) round(z);
  int l2;
  luut = log(u/(u+t));
  lttu = log(t/(u+t));
  lp1 = luut*(s+1.0-prior_c) - lngamma(s+1.0-prior_c);
  if (top < z) {
    sum += pa_sum(top, -1, -1, zz, lp1, lttu, luut, s, l2);
    sum += pa_sum(top+1, zz, 1, zz, lp1, lttu, luut, s, l2);
    sum += pa_sum(zz-1, l2, -1, zz, lp1, lttu, luut, s, l2);
    sum += pa_sum(zz, -1, 1, zz, lp1, lttu, luut, s, l2);
  } else {
    sum += pa_sum(zz-1, -1, -1, zz, lp1, lttu, luut, s, l2);
    sum += pa_sum(zz, top, 1, zz, lp1, lttu, luut, s, l2);
    sum += pa_sum(top, l2, -1, zz, lp1, lttu, luut, s, l2);
    sum += pa_sum(top+1, -1, 1, zz, lp1, lttu, luut, s, l2);
  }
  return sum;
}

// The principal anomaly for a gamma distribution
static double principal_anomaly_gamma(double r, double w, double s, double u)
{
  // This is not finished
  return 0.0;
}

inline double tf(double x) { return (x >= 1.0 ? 0.0 : x <= 0.0 ? 27.0 : sqrt(-log(x))); }

static void inv_principal_anomaly_poisson(double ano, double t, double s, double u, double& z1, double& z2)
{
  int c1=1, c2=-1, c3=0;
  double za, zb, zm, va, vb, vm;
  double top = floor(t/u*(s-prior_c));
  ano = sqrt(ano);
  za = 0, zb = top+1.0;
  va = tf(principal_anomaly_poisson(0, t, s, u));
  vb = 0.0; // approximatively
  if (va <= ano)
    z1 = za;
  else {
    do {
      zm = za + (zb - za)*(ano - va)/(vb - va);
      if (zm < (za+zb)*0.5)
        zm = ceil(zm);
      else
        zm = floor(zm);
      vm = tf(principal_anomaly_poisson(zm, t, s, u));
      c1++;
      if (vm == ano)
	za = zb = zm;
      else if (vm > ano)
        za = zm;
      else 
        zb = zm;
    } while (zb - za > 1.5);
    z1 = zb;
  }
  zb = top;
  vb = 0.0; // again
  do {
    c2++, za = zb, va = vb, zb = (za < 8 ? za+8 : za + floor(3*sqrt(za+1)));
  } while ((vb = tf(principal_anomaly_poisson(zb, t, s, u))) < ano);
  do {
    zm = za + (zb - za)*(ano - va)/(vb - va);
    if (zm < (za+zb)*0.5)
      zm = ceil(zm);
    else
      zm = floor(zm);
    vm = tf(principal_anomaly_poisson(zm, t, s, u));
    c3++;
    if (vm == ano)
      zb = za = zm;
    else if (vm > ano)
      zb = zm;
    else 
      za = zm;
  } while (zb - za > 1.5);
  z2 = za;
}

double IscRawExponentialMicroModel::raw_anomaly(double r, double t, double rr, double tt)
{
  // For now everything is calculated as poisson.
  double pa;
  if ((t == 0.0 && r == 0.0) || sumt == 0.0)
    return 0.0;
  else {
    pa = principal_anomaly_poisson(r, t, sumr, sumt);
    return (pa > 0.0 ? pa > 1.0 ? 0.0 : -log(pa) : 700.0);
  }
}

double IscRawExponentialMicroModel::raw_logp(double r, double t, double rr, double tt)
{
  return pa_logp(r, t, rr, tt, sumr, sumt);
}

void IscRawExponentialMicroModel::raw_ev_logp(double rr, double tt, double& e, double& v)
{
  double mxr, mxt;
  if (raw_logp(rr, tt, rr, tt) < raw_maxlogp(rr, tt, mxr, mxt)-20.0) {
    if (mxr < rr) {
      e = v = 0.0;
      sum_poisson_log(sumr, sumt, rr, tt, e, v);
      v -= e*e;
    } else {
      e = lngamma(rr+sumr+1) - lngamma(rr) - lngamma(sumr+1) - log(sumt) + (rr-1)*digamma(rr) + (sumr+2)*digamma(sumr+1) - (rr+sumr+1)*digamma(rr+sumr+1) + log(tt/rr);
      v = (rr-1)*(rr-1)*ddigamma(rr) + (sumr+2)*(sumr+2)*ddigamma(sumr+1) - (rr+sumr+1)*(rr+sumr+1)*ddigamma(rr+sumr+1);
    }
  } else {
    e = v = 0.0;
    sum_poisson_log(sumr, sumt, rr, tt, e, v);
    integral_gamma_log(sumr, sumt, rr, tt, e, v);
    v -= e*e;
  }
}

// Can be used for finding the peak r and t, and the "offset" of P(-logP)
double IscRawExponentialMicroModel::raw_maxlogp(double rr, double tt, double& peakr, double& peakt)
{
  double mxt, mxr, pt, pr1, pr2;
  mxt = (rr-1)*sumt/(sumr+2);
  if (mxt < 0.0) mxt = 0.0;
  else if (mxt > tt) mxt = tt;
  mxr = (sumr+2)*tt/sumt-0.5; 
  if (mxr < 0.0) mxr = 0.5;
  else if (mxr > rr) mxr = rr;
  pt = raw_logp(rr, mxt, rr, tt);
  pr1 = raw_logp(floor(mxr), tt, rr, tt);
  pr2 = raw_logp(ceil(mxr), tt, rr, tt);
  if (pt > pr1 && pt > pr2) {
    peakr = rr, peakt = mxt;
    return pt;
  } else {
    peakt = tt, peakr = (pr1 > pr2 ? floor(mxr) : ceil(mxr));
    return (pr1 > pr2 ? pr1 : pr2);
  }
}

void IscRawExponentialMicroModel::raw_invanomaly(double rr, double tt, double ano, double& minr, double& maxr, double& mint, double& maxt)
{
  // This currently only fills in the poisson part
  inv_principal_anomaly_poisson(ano, tt, sumr, sumt, minr, maxr);
}

