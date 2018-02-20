/*
 * isc_serialization.hh
 *
 *  Created on: Feb 26, 2016
 *      Author: tol
 */

#ifndef ISC_EXPORTER_HH_
#define ISC_EXPORTER_HH_

/**
 * Exporters
 */
class IscAbstractModelExporter {
public:
	IscAbstractModelExporter(){/* SWIG cannot wrap abstract classes*/};
	virtual ~IscAbstractModelExporter(){/*SWIG cannot wrap abstract classes*/};
	virtual void notImplemented(){/* SWIG cannot wrap abstract classes*/};

	virtual void addParameter(const char* parameter_name, const char* value){/* SWIG cannot wrap abstract classes*/};
	virtual void addParameter(const char* parameter_name, int value){/* SWIG cannot wrap abstract classes*/};
	virtual void addParameter(const char* parameter_name, float value){/* SWIG cannot wrap abstract classes*/};
	virtual void addParameter(const char* parameter_name, double value){/* SWIG cannot wrap abstract classes*/};
	virtual void addParameter(const char* parameter_name, int *value, int length){/* SWIG cannot wrap abstract classes*/};
	virtual void addParameter(const char* parameter_name, float *value, int length){/* SWIG cannot wrap abstract classes*/};
	virtual void addParameter(const char* parameter_name, double *value, int length){/* SWIG cannot wrap abstract classes*/};

	virtual IscAbstractModelExporter* createModelExporter(const char * parameter_name){
		/* SWIG cannot wrap abstract classes*/
		return 0;
	};
	virtual IscAbstractModelExporter* createModelExporter(int parameter_id){
		/* SWIG cannot wrap abstract classes*/
		return 0;
	};
};

class IscAbstractModelImporter {
public:
	IscAbstractModelImporter(){/* SWIG cannot wrap abstract classes*/};
	virtual ~IscAbstractModelImporter(){/* SWIG cannot wrap abstract classes*/};
	virtual void notImplemented(){/* SWIG cannot wrap abstract classes*/};

	// Methods that sets the values to the provided data structure
	virtual void fillParameter(const char* parameter_name, int &value){/* SWIG cannot wrap abstract classes*/};
	virtual void fillParameter(const char* parameter_name, float &value){/* SWIG cannot wrap abstract classes*/};
	virtual void fillParameter(const char* parameter_name, double &value){/* SWIG cannot wrap abstract classes*/};

	virtual void fillParameter(const char* parameter_name, int *value, int length){/* SWIG cannot wrap abstract classes*/};
	virtual void fillParameter(const char* parameter_name, float *value, int length){/* SWIG cannot wrap abstract classes*/};
	virtual void fillParameter(const char* parameter_name, double *value, int length){/* SWIG cannot wrap abstract classes*/};
	virtual IscAbstractModelImporter* getModelImporter(const char * parameter_name){
		/* SWIG cannot wrap abstract classes*/
		return 0;
	};
	virtual IscAbstractModelImporter* getModelImporter(int parameter_id){
		/* SWIG cannot wrap abstract classes*/
		return 0;
	};

};



#endif /* ISC_EXPORTER_HH_ */
