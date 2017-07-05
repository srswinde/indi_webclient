/*
   INDI Developers Manual
   Tutorial #1

   "Hello INDI"

   We construct a most basic (and useless) device driver to illustate INDI.

   Refer to README, which contains instruction on how to build this driver, and use it
   with an INDI-compatible client.

*/

/** \file simpledevice.h
    \brief Construct a basic INDI device with only one property to connect and disconnect.
    \author Jasem Mutlaq

    \example simpledevice.h
    A very minimal device! It also allows you to connect/disconnect and performs no other functions.
*/

#pragma once

#include "defaultdevice.h"
#include <string.h>

class indiNVP
{
	public:
		indiNVP(INumberVectorProperty *);
		indiNVP(INumber * num0, int n, const char * name, const char *label, const char * group, IPerm perm, double timeout, IPState state  );
		INumberVectorProperty *nvp;

	private: 
		bool fill();
};

class SimpleDevice : public INDI::DefaultDevice
{
  public:
    SimpleDevice();
	virtual bool initPropeties();
	virtual bool updateProperties();
	virtual bool ISNewNumber (const char *dev, const char *name, double values[], char *names[], int n);
	bool fill();
	
	bool setNumber(INumberVectorProperty *, double values, char * names[], int);
	INumberVectorProperty PosLatNV_X;
	INumber PosLatN_X[1];
	INumberVectorProperty PosLatNV_Y;
	INumber PosLatN_Y[1];
	INumberVectorProperty PosLatNV_Z;
	INumber PosLatN_Z[1];


	INumberVectorProperty PosRotNV_W;
	INumber PosRotN_W[1];
	INumberVectorProperty PosRotNV_V;
	INumber PosRotN_V[1];
	INumberVectorProperty PosRotNV_U;
	INumber PosRotN_U[1];
	
  protected:
    bool Connect();
    bool Disconnect();
    const char *getDefaultName();
	
};
