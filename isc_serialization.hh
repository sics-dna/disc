/*
 * isc_serialization.hh
 *
 *  Created on: Feb 26, 2016
 *      Author: tol
 */

#ifndef ISC_SERIALIZATION_HH_
#define ISC_SERIALIZATION_HH_

/**
 * Serializers
 */

class AbstractMixtureSerializer {
public:
	virtual AbstractMixtureSerializer();
	virtual ~AbstractMixtureSerializer();
	virtual AbstractComponentSerializer createComponent();

};

class AbstractComponentSerializer {
public:
	virtual AbstractComponentSerializer();
	virtual ~AbstractComponentSerializer();
	virtual AbstractMicroModelSerializer createMicroModel();
};

class AbstractMicroModelSerializer {
public:
	virtual AbstractMicroModelSerializer();
	virtual ~AbstractMicroModelSerializer();
	virtual void addModelParameter(const char* parameter_name, int value);
	virtual void addModelParameter(const char* parameter_name, float value);
};



/**
 * Deserializers
 */


class AbstractMixtureDeserializer {
public:
	virtual AbstractMixtureDeserializer();
	virtual ~AbstractMixtureDeserializer();
	virtual AbstractComponentDeserializer* getComponents();
	virtual int getNumberOfComponents();

};

class AbstractComponentDeserializer {
public:
	virtual AbstractComponentDeserializer();
	virtual ~AbstractComponentDeserializer();
	virtual AbstractMicroModelDeserializer* getMicroModels();
	virtual int getNumberOfMicroModels();
};

class AbstractMicroModelDeserializer {
public:
	virtual AbstractMicroModelDeserializer();
	virtual ~AbstractMicroModelDeserializer();
	virtual int getModelIntParameter(const char* parameter_name);
	virtual float getModelFloatParameter(const char* parameter_name);
};


#endif /* ISC_SERIALIZATION_HH_ */
