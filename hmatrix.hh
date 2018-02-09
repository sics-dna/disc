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

#ifndef _HMATRIX

#include "isc_exporter.hh"

class HMatrix
{
public:
	HMatrix(int n, int tp) { typ = tp; dim = n; len = (tp==2 ? n*(n+1)/2 : n); uvec = 0; livec = 0; vec = new double[len]; for (int i=0; i<len; i++) vec[i]=0.0; };
	HMatrix(AbstractModelExporter& m) {
		m.fillParameter("hmatrix_typ", typ);
		m.fillParameter("hmatrix_dim", dim);
		m.fillParameter("hmatrix_len", len);
		uvec = 0;
		livec = 0;
		vec = new double[len];
		m.fillParameter("hmatrix_vec", vec, len);
	};
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

	virtual void exportHMatrix(AbstractModelExporter exporter) {
		exporter.addParameter("hmatrix_typ",typ);
		exporter.addParameter("hmatrix_dim",dim);
		exporter.addParameter("hmatrix_len",len);
		exporter.addParameter("hmatrix_vec",vec,len);
	}
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

#define _HMATRIX
#endif
