/*
 * isc_serialization.hh
 *
 *  Created on: Feb 26, 2016
 *      Author: tol
 */

#ifndef ISC_EXPORTER_HH_
#define ISC_EXPORTER_HH_

#include "isc_combination_rule.hh"

/**
 * Exporters
 */
class AbstractModelExporter {
public:
	AbstractModelExporter();
	virtual ~AbstractModelExporter();
	virtual void notImplemented();
	virtual void setMicroModelName(const char* name);
	virtual char* getMicroModelName();

	virtual void addParameter(const char* parameter_name, int value);
	virtual void addParameter(const char* parameter_name, float value);
	virtual void addParameter(const char* parameter_name, double value);
	virtual void addParameter(const char* parameter_name, int *value, int length);
	virtual void addParameter(const char* parameter_name, float *value, int length);
	virtual void addParameter(const char* parameter_name, double *value, int length);

	virtual AbstractModelExporter createModelExporter(const char * parameter_name);
	// Methods that sets the values to the provided data structure
	virtual void fillParameter(const char* parameter_name, int &value);
	virtual void fillParameter(const char* parameter_name, float &value);
	virtual void fillParameter(const char* parameter_name, double &value);

	virtual void fillParameter(const char* parameter_name, int *value, int length);
	virtual void fillParameter(const char* parameter_name, float *value, int length);
	virtual void fillParameter(const char* parameter_name, double *value, int length);
	virtual AbstractModelExporter getParameterModel(const char * parameter_name);
};


#endif /* ISC_EXPORTER_HH_ */
