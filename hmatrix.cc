 /*
 --------------------------------------------------------------------------
 Copyright (C) 2006, 2015 SICS Swedish ICT AB

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
#include "hmatrix.hh"

/*
class HMatrix
{
public:
  HMatrix(int n, int tp) { typ = tp; dim = n; len = (tp==2 ? n*(n+1)/2 : n); uvec = 0; livec = 0; vec = new double[len]; for (int i=0; i<len; i++) vec[i]=0.0; };
  HMatrix(HMatrix& m) { typ = m.typ; dim = m.dim; len = m.len; uvec = 0; livec = 0; vec = new double[len]; for (int i=0; i<len; i++) vec[i]=m.vec[i]; };
  virtual ~HMatrix() { delete [] vec; if (uvec) delete [] uvec; if (livec) delete [] livec; };
  virtual void set(HMatrix& m) { clean(); if (dim != m.dim || typ != m.typ) { delete [] vec; typ = m.typ; dim = m.dim; len = (typ==2 ? dim*(dim+1)/2 : dim); vec = new double[len]; }; for (int i=0; i<len; i++) vec[i]=m.vec[i]; };
  virtual double operator()(int i, int j);
  virtual void zero() { clean(); for (int i=0; i<len; i++) vec[i]=0.0; };
  virtual void unit() { clean(); if (typ==2) { for (int i=0; i<len; i++) vec[i]=0.0; for (int i=0; i<dim; i++) vec[(2*dim*i-i*i+i)/2]=1.0; } else for (int i=0; i<len; i++) vec[i]=1.0; };
  virtual void scale(double sc) { clean(); for (int i=0; i<len; i++) vec[i] *= sc; };
  virtual void interpolate(HMatrix* m, double fact) { clean(); for (int i=0; i<len; i++) vec[i] = vec[i]*(1.0-fact) + m->vec[i]*fact; };
  virtual void accum(double* av, double sc);
  virtual double sqprod(double* v);
  virtual double sqprodinv(double* v);
  virtual double sqprodinv_part(double* v, int cdim);
  virtual double sqprodinv_cond(double* v, int cdim);
  virtual double det();
  virtual double det_part(int cdim);
  virtual double det_cond(int cdim);
  virtual HMatrix* inverse();
protected:
  void clean() { if (uvec) delete [] uvec; if (livec) delete [] livec; uvec = 0; livec = 0; };
  void factor();
  int typ;
  int dim;
  int len;
  double* vec;
  double* uvec;
  double* livec;
};

*/ 

 /*
  *  Half matrix, representing a covariance matrix and its cholesky factorization 
  *  C = U' * U, inv(C) = Linv' * Linv
  *  
  *            0123             0
  *  C and U:   456      Linv:  14
  *              78             257
  *               9             3689
  */

double HMatrix::operator()(int i, int j)
{
  if (i>=0 && i<dim && j>=0 && j<dim) {
    if (typ == 2) {
      if (j<=i)
        return vec[i + (2*dim*j-j*j-j)/2];
      else
        return vec[j + (2*dim*i-i*i-i)/2];
    } else {
      if (i == j)
        return vec[i];
      else
        return 0.0;
    }
  }
  else
    return 0.0;
}

void HMatrix::accum(double* v, double sc)
{
  double* p;
  double a;
  int i, j;
  clean();
  if (typ == 2) {
    for (i=0, p=vec; i<dim; i++) {
      a = v[i] * sc;
      for (j=i; j<dim; j++, p++)
        *p += a * v[j];
    }
  } else {
    for (i=0, p=vec; i<dim; i++, p++)
      *p += v[i] * v[i] * sc;
  }
}

HMatrix* HMatrix::inverse()
{
  int i, j, k;
  double *p, *q, *r;
  HMatrix* inv = new HMatrix(dim, typ);
  if (typ == 2) {
    if (!uvec || !livec)
      factor();
    for (i=0, p=livec, q=p, r=inv->vec; i<dim; i++, q=p) {
      for (j=i; j<dim; p++, r++, q+=(dim-j), j++) {
        *r = 0.0;
        for (k=0; k<dim-j; k++)
          *r += p[k]*q[k];
      }
    }
  } else {
    for(i=0; i<dim; i++)
      inv->vec[i] = (vec[i] <= 0.0 ? HUGE_VAL : 1.0/vec[i]);
  }
  return inv;
}

double HMatrix::sqprod(double* v)
{
  int i, j;
  double* p;
  double r=0.0, e;
  if (typ == 2) {
    if (!uvec || !livec)
      factor();
    for (i=0, p=uvec; i<dim; i++) {
      e = 0.0;
      for (j=i; j<dim; j++, p++)
        e += *p * v[j];
      r += e * e;
    }
    return r;
  } else {
    for (i=0, p=vec; i<dim; i++, p++)
      r += *p * v[i] * v[i];
    return r;
  }
}

double HMatrix::sqprodinv(double* v)
{
  int i, j;
  double* p;
  double r=0.0, e;
  if (typ == 2) {
    if (!uvec || !livec)
      factor();
    for (i=0; i<dim; i++) {
      e = 0.0;
      for (j=0, p=livec+i; j<=i; j++, p+=dim-j)
        e += *p * v[j];
      r += e * e;
    }
    return r;
  } else {
    for (i=0, p=vec; i<dim; i++, p++)
      r += v[i] * v[i] / (*p <= 1e-12 ? 1e-12 : *p);
    return r;
  }
}

// The square distance of the first cdim dimensions
double HMatrix::sqprodinv_part(double* v, int cdim)
{
  int i, j;
  double* p;
  double r=0.0, e;
  if (typ == 2) {
    if (!uvec || !livec)
      factor();
    for (i=0; i<cdim; i++) {
      e = 0.0;
      for (j=0, p=livec+i; j<=i; j++, p+=dim-j)
        e += *p * v[j];
      r += e * e;
    }
    return r;
  } else {
    for (i=0, p=vec; i<cdim; i++, p++)
      r += v[i] * v[i] / (*p <= 1e-12 ? 1e-12 : *p);
    return r;
  }
}

// The square distance of the last dim-cdim dimensions, conditionally of the first cdim dimensions
double HMatrix::sqprodinv_cond(double* v, int cdim)
{
  int i, j;
  double* p;
  double res, tmp;
  if (typ == 2) {
    double* va = new double[dim-cdim];
    double* vb = new double[dim-cdim];
    if (!uvec || !livec)
      factor();
    // vb = B*v1
    for (i=0; i<dim-cdim; i++) {
      vb[i] = 0.0;
      for (j=0, p=livec+cdim+i; j<cdim; j++, p+=dim-j)
        vb[i] += *p * v[j];
    }
    // dim*(dim+1) - (dim-cdim)*(dim-cdim+1) = 2*dim*cdim -cdim^2+cdim 
    // va = C'*vb
    for (i=0; i<dim-cdim; i++) {
      va[i] = 0.0;
      for (j=0, p=uvec+(cdim*(2*dim-cdim+1)/2)+i; j<=i; j++, p+=dim-j)
        va[i] += *p * vb[j];
    }
    // vb = v2 + va
    for (i=0; i<dim-cdim; i++)
      vb[i] = v[i+cdim] + va[i];
    // (D*vb)^2
    res = 0.0;
    for (i=0; i<dim-cdim; i++) {
      tmp = 0.0;
      for (j=0, p=livec+(cdim*(2*dim-cdim+1)/2)+i; j<=i; j++, p+=dim-j)
        tmp += *p * vb[j];
      res += tmp * tmp;
    }
    delete [] va;
    delete [] vb;
    return res;
  } else {
    res = 0.0;
    for (i=cdim, p=vec+cdim; i<dim; i++, p++)
      res += v[i] * v[i] / (*p <= 1e-12 ? 1e-12 : *p);
    return res;
  }
}

double HMatrix::det()
{
  int i;
  double d=1.0;
  if (typ == 2) {
    if (!uvec || !livec)
      factor();
    for (i=0; i<dim; i++)
      d *= uvec[(2*dim*i-i*i+i)/2];
    return d*d;
  } else {
    for (i=0; i<dim; i++)
      d *= (vec[i] <= 0.0 ? 0.0 : vec[i]);
    return d;
  }
}

// The determinant for the first cdim dimensions
double HMatrix::det_part(int cdim)
{
  int i;
  double d=1.0;
  if (typ == 2) {
    if (!uvec || !livec)
      factor();
    for (i=0; i<cdim; i++)
      d *= uvec[(2*dim*i-i*i+i)/2];
    return d*d;
  } else {
    for (i=0; i<cdim; i++)
      d *= (vec[i] <= 0.0 ? 0.0 : vec[i]);
    return d;
  }
}

// The determinant for the last dim-cdim dimensions, conditionally on the cdim first dimensions
double HMatrix::det_cond(int cdim)
{
  int i;
  double d=1.0;
  if (typ == 2) {
    if (!uvec || !livec)
      factor();
    for (i=cdim; i<dim; i++)
      d *= uvec[(2*dim*i-i*i+i)/2];
    return d*d;
  } else {
    for (i=cdim; i<dim; i++)
      d *= (vec[i] <= 0.0 ? 0.0 : vec[i]);
    return d;
  }
}

void HMatrix::factor()
{
  int i, j, k;
  double *b, *c, *p, *q;
  double fac;
  uvec = new double[len];
  livec = new double[len];
  for (i=0; i<len; i++)
    uvec[i] = vec[i], livec[i] = 0.0;
  for (i=0; i<dim; i++)
    livec[(2*dim*i-i*i+i)/2] = 1.0;
  for (i=0, b=uvec; i<dim; i++)
    if (*b <=0) {
      for (j=0, c=uvec+i; j<i; j++, c+=dim-j)
        *c = 0.0;
      *b++ = 0;
      for (j=1; j<dim-i; j++)
        *b++ = 0.0;
    } else
      b += dim-i;
  for (i=0, b=uvec, c=livec; i<dim; b+=dim-i, i++, c++) {
    if (*b <= 0) {
      *b = 0;
      for (j=1, p=b+1; j<dim-i; j++)
        *p++ = 0.0;
    }
    if (i<dim-1) {
      p = b + dim-i;
      for (k=i+1; k<dim; k++) {
        fac = b[k-i] / *b;
        for (j=k; j<dim; j++, p++)
          *p -= fac*b[j-i];
      }
      for (k=0; k<=i; k++) {
        q = c + k*(2*dim-k-1)/2;
        fac = *q / *b;
        for (j=i+1, q++; j<dim; j++, q++)
          *q -= fac*b[j-i];
      }
    }
    fac = (*b <= 0 ? 1e20 : 1.0 / sqrt(*b));
    for (k=i, p=b; k<dim; k++, p++)
      *p *= fac;
    for (k=0, q=c; k<=i; k++, q+=dim-k)
      *q *= fac;
  }
}

#if 0
// Distance
  if (pc.vartype == 0) {
    dist = dmult(pu.ivar, pc.sizec, ttmp);
  } 
  else if (pc.vartype == 1) {
    dist = dmult(pu.ivar, pc.sizec, ttmp);
  } 
  else if (pc.vartype == 2) {
    dist = hmultl(pu.ivar, pc.sizec, ttmp);
  }
  delete [] ttmp;
  return (log(fact * pu.prob) - pu.stdev - dist/2);


      if (paramc->vartype == 2) {
        hunit(p->var, nc);
        hunit(p->ivar, nc);
        for (j2=0; j2<nc; j2++) {
          v3 = (max[j2] - min[j2])*v2;
//          v3 = 1.0/v2;
          p->var[(2*nc*j2-j2*j2+j2)/2] = v3*v3;
          p->ivar[(2*nc*j2-j2*j2+j2)/2] = 1.0/v3;
        }
//         hscale(p->var, nc, 1.0/(v2*v2));
//         hscale(p->ivar, nc, v2);
      } else {
        dunit(p->var, nc);
        dunit(p->ivar, nc);
        for (j2=0; j2<nc; j2++) {
          v3 = (max[j2] - min[j2])*v2;
//          v3 = 1.0/v2;
          p->var[j2] = v3*v3;
          p->ivar[j2] = 1.0/(v3*v3);
        }
//         dscale(p->var, nc, 1.0/(v2*v2));
//         dscale(p->ivar, nc, v2*v2);
      }


  {
    act += 1.0;
    for (k=0; k<dim; k++)
      gmean[k] += vec[k];
    if (paramc->vartype == 2)
      hacc(gvar, dim, vec, 1.0);
    else
      dacc(gvar, dim, vec, 1.0);
  } 
  for (k=0; k<dim; k++)
    gmean[k] /= act;
  if (paramc->vartype == 0) {
    dscale(gvar, dim, 1.0/act);
    dacc(gvar, dim, gmean, -1.0);
    gstd = sqrt(ddet(gvar, dim)); 
  } else if (paramc->vartype == 1) {
    dscale(gvar, dim, 1.0/act);
    dacc(gvar, dim, gmean, -1.0);
  } else if (paramc->vartype == 2) {
    hscale(gvar, dim, 1.0/act);
    hacc(gvar, dim, gmean, -1.0);
  }


      for (j=0; j<sz; j++) {
        for (k=0; k<dim; k++)
          vec[k] = tmp[ind[k]].f - paramu[j+off].mean[k];
        acts[j] += act = belief[j+off]/sum;
        for (k=0; k<dim; k++)
          means[j][k] += act*vec[k];
        if (paramc->vartype == 0) {
          dacc(vars[j], dim, vec, act);
        } else if (paramc->vartype == 1) {
          dacc(vars[j], dim, vec, act);
        } else if (paramc->vartype == 2) {
          hacc(vars[j], dim, vec, act);
        }
      }


      if (paramc->vartype == 0) {
        tv = sqrt(ddet(vars[j], dim)); 
        tv = (tv == 0 ? log(1e-6)*dim : log(tv));
        dcopy(gvar, pu->var, dim); 
        dscale(pu->var, dim, exp((tv-log(gstd))*2/dim)); 
        dinv(pu->var, pu->ivar, dim); 
        pu->stdev = tv; 
      } else if (paramc->vartype == 1) {
        tv = sqrt(ddet(vars[j], dim));
        tv = (tv == 0 ? log(1e-6)*dim : log(tv));
        pu->stdev = tv;
        dcopy(vars[j], pu->var, dim);
        dinv(vars[j], pu->ivar, dim);
      } else if (paramc->vartype == 2) {
#ifdef CONSTRAIN
        int dimcl = 9;
//        int dimcl = data->length() - data->format()->getsection(0);
        for (k=0; k<dim-dimcl; k++)
          for (i=0; i<dimcl; i++)
            vars[j][(2*dim*k-k*k-k)/2+dim-dimcl+i] = 0.0;
#endif    
        hcopy(vars[j], pu->var, dim);
        tv = sqrt(hfactor(vars[j], pu->ivar, dim));
        tv = (tv == 0 ? log(1e-6)*dim : log(tv));
        pu->stdev = tv;
      }
#endif
