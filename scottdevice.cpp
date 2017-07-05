/*
   INDI Developers Manual
   Tutorial #1

   "Hello INDI"

   We construct a most basic (and useless) device driver to illustrate INDI.

   Refer to README, which contains instruction on how to build this driver, and use it
   with an INDI-compatible client.

*/

/** \file simpledevice.cpp
    \brief Construct a basic INDI device with only one property to connect and disconnect.
    \author Jasem Mutlaq

    \example simpledevice.cpp
    A very minimal device! It also allows you to connect/disconnect and performs no other functions.
*/

#include "simpledevice.h"

#include <memory>

std::unique_ptr<SimpleDevice> simpleDevice(new SimpleDevice());




/**************************************************************************************
** Return properties of device.
***************************************************************************************/
void ISGetProperties(const char *dev)
{
    simpleDevice->ISGetProperties(dev);
}

/**************************************************************************************
** Process new switch from client
***************************************************************************************/
void ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n)
{
    simpleDevice->ISNewSwitch(dev, name, states, names, n);
}

/**************************************************************************************
** Process new text from client
***************************************************************************************/
void ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n)
{
    simpleDevice->ISNewText(dev, name, texts, names, n);
}

/**************************************************************************************
** Process new number from client
***************************************************************************************/
void ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n)
{
    simpleDevice->ISNewNumber(dev, name, values, names, n);
}

/**************************************************************************************
** Process new blob from client
***************************************************************************************/
void ISNewBLOB(const char *dev, const char *name, int sizes[], int blobsizes[], char *blobs[], char *formats[],
               char *names[], int n)
{
    simpleDevice->ISNewBLOB(dev, name, sizes, blobsizes, blobs, formats, names, n);
}

/**************************************************************************************
** Process snooped property from another driver
***************************************************************************************/
void ISSnoopDevice(XMLEle *root)
{
    INDI_UNUSED(root);
}




SimpleDevice::SimpleDevice()
{
	IDLog("construct\n");
}

bool SimpleDevice::initPropeties()
{
	SimpleDevice::initProperties();
	return true;
}

bool SimpleDevice::updateProperties()
{	

	fill();
	IDMessage( getDeviceName(), "A messagge from updateProperties!" );
	return true;
}


/**************************************************************************************
** Client is asking us to establish connection to the device
***************************************************************************************/
bool SimpleDevice::Connect()
{
    IDMessage(getDeviceName(), "Simple device connected successfully!");
    return true;
}

/**************************************************************************************
** Client is asking us to terminate connection to the device
***************************************************************************************/
bool SimpleDevice::Disconnect()
{
    IDMessage(getDeviceName(), "Simple device disconnected successfully!");
    return true;
}

/**************************************************************************************
** INDI is asking us for our default device name
***************************************************************************************/
const char *SimpleDevice::getDefaultName()
{
    return "Simple Device";
}


bool SimpleDevice::ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n)
{
	INumberVectorProperty * myv = getNumber(name);
	
	for(int ii=0; ii<n; ii++)
	{
		
		IDMessage(getDeviceName(), "dev=%s, name=%s, names[%i]=%s, values[%i]=%f\n", dev, name, ii, names[ii], ii, values[ii] );
		
		if( strcmp(name, "RotPos" ) == 0 )
		{
			
			
		}
	}
	IUUpdateNumber(myv, values, names, n);
	IDSetNumber(myv, "SO DUmb");
	return true;

}

bool SimpleDevice::fill()
{

	/*Translational numbers*/
	IUFillNumber(&PosLatN_X[0] , "X", "X Axis ", "%5.4f", -5.0, 5.0, 0.001, 0);
	IUFillNumberVector( &PosLatNV_X,  PosLatN_X, 1, getDeviceName(), "LinPosX", "Linear Position X", "ALL", IP_RW, 0.5, IPS_IDLE );
	defineNumber( &PosLatNV_X );
	
	IUFillNumber(&PosLatN_Y[0] , "Y", "Y Axis ", "%5.4f", -5.0, 5.0, 0.001, 0);
	IUFillNumberVector( &PosLatNV_Y,  PosLatN_Y, 1, getDeviceName(), "LinPosY", "Linear Position Y", "ALL", IP_RW, 0.5, IPS_IDLE );
	defineNumber( &PosLatNV_Y );

	IUFillNumber(&PosLatN_Z[0] , "Z", "Z Axis ", "%5.4f", -5.0, 5.0, 0.001, 0);
	IUFillNumberVector( &PosLatNV_Z,  PosLatN_Z, 1, getDeviceName(), "LinPosZ", "Linear Position Z", "ALL", IP_RW, 0.5, IPS_IDLE );
	defineNumber( &PosLatNV_Z );


	/*rotational numbers*/
	IUFillNumber(&PosRotN_W[0] , "W", "W Axis ", "%5.4f", -5.0, 5.0, 0.001, 0);
	IUFillNumberVector( &PosRotNV_W,  PosRotN_W, 1, getDeviceName(), "RotPosW", "Rotational Position W", "ALL", IP_RW, 0.5, IPS_IDLE );
	defineNumber( &PosRotNV_W );


	IUFillNumber(&PosRotN_V[0] , "V", "V Axis ", "%5.4f", -5.0, 5.0, 0.001, 0);
	IUFillNumberVector( &PosRotNV_V,  PosRotN_V, 1, getDeviceName(), "RotPosV", "Rotational Position V", "ALL", IP_RW, 0.5, IPS_IDLE );
	defineNumber( &PosRotNV_V );


	IUFillNumber(&PosRotN_U[0] , "U", "U Axis ", "%5.4f", -5.0, 5.0, 0.001, 0);
	IUFillNumberVector( &PosRotNV_U,  PosRotN_U, 1, getDeviceName(), "RotPosU", "Rotational Position U", "ALL", IP_RW, 0.5, IPS_IDLE );
	defineNumber( &PosRotNV_U );


	return true;

}
