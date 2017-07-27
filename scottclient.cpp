#if 0
Simple Client Tutorial
Demonstration of libindi v0.7 capabilities.

Copyright (C) 2010 Jasem Mutlaq (mutlaqja@ikarustech.com)

This library is free software;
you can redistribute it and / or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation;
either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY;
without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library;
if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110 - 1301  USA

#endif

/** \file tutorial_client.cpp
    \brief Construct a basic INDI client that demonstrates INDI::Client capabilities. This client must be used with tutorial_three device "Simple CCD".
    \author Jasem Mutlaq

    \example tutorial_client.cpp
    Construct a basic INDI client that demonstrates INDI::Client capabilities. This client must be used with tutorial_three device "Simple CCD".
    To run the example, you must first run tutorial_three:
    \code indiserver tutorial_three \endcode
    Then in another terminal, run the client:
    \code tutorial_client \endcode
    The client will connect to the CCD driver and attempts to change the CCD temperature.
*/


#include "basedevice.h"

#include <fstream>
#include <iostream>
#include <memory>
#include <string.h>
#include <uWS/uWS.h> 
#include <thread>
#include "/home/scott/git-clones/json/src/json.hpp"
#include <mutex>
#include <queue>
#include <string>
using json = nlohmann::json;
#include "tutorial_client.h"
#define MYCCD "Simple CCD"
bool test = true;

/* Our client auto pointer */
std::unique_ptr<MyClient> camera_client(new MyClient());




		
void ComQ::push(json *data)
{

	std::lock_guard<std::mutex> lock(qutex);
	q.push(data->dump(4));
	if(q.size() > 500 )
		q.pop();
}
std::string ComQ::front() 
{
	std::lock_guard<std::mutex> lock(qutex);
	return q.front();
}
std::string ComQ::pop()
{
	std::lock_guard<std::mutex> lock(qutex);
	std::string front = q.front();
	q.pop();
	return front;
}

void WSthread(ComQ *q, ComQ  *devQ)
{


    uWS::Hub h;
    h.onMessage([&](uWS::WebSocket<uWS::SERVER> *ws, char *message, size_t length, uWS::OpCode opCode) {
		
		char buff[5000];
		char readbuff[500];
		bool send=false;
		strncpy(readbuff, message, length);
		readbuff[length] = '\0';//terminate for some reason
		q->connected = true;
		json inj = json::parse(readbuff);
		
		time_t b4=time(NULL);
		if(inj.find("task") != inj.end())
		{
			if(inj["task"] == "collect")
				send=true;
			else if(inj["task"] == "updateSwitch")
			{
				devQ->push(&inj);
			}
		}	
		while( send && q->connected && ((time(NULL)-b4) < 5) )
		{
			if(q->size() > 0)	
			{
				strcpy( buff, q->front().c_str() );
			
    	    	ws->send(buff, strlen(buff), opCode);
				q->pop();
    	    	//ws->send(message, length,  opCode);
			}
			usleep( (int) 250);
		}
		//std::cout << "stopping the send" << std::endl;
		//ws->close();
    });

	h.onDisconnection([&](uWS::WebSocket<uWS::SERVER> *ws, int code, char *message, size_t length) 
	{
		std::cout << "disconnect" << std::endl; 
		q->connected = false;
		
		//ws->close();
		h.getDefaultGroup<uWS::SERVER>().close();
	});

    h.listen(3000);
    h.run();

}

/**************************************************************************************
**
***************************************************************************************/
MyClient::MyClient()
{
    ccd_simulator = nullptr;
}

/**************************************************************************************
**
***************************************************************************************/
MyClient::~MyClient()
{
}

/**************************************************************************************
**
***************************************************************************************/
void MyClient::setTemperature()
{
    INumberVectorProperty *ccd_temperature = nullptr;

    ccd_temperature = ccd_simulator->getNumber("CCD_TEMPERATURE");

    if (ccd_temperature == nullptr)
    {
        IDLog("Error: unable to find CCD Simulator CCD_TEMPERATURE property...\n");
        return;
    }

    ccd_temperature->np[0].value = -20;
    sendNewNumber(ccd_temperature);
}

/**************************************************************************************
**
***************************************************************************************/
void MyClient::takeExposure()
{
    INumberVectorProperty *ccd_exposure = nullptr;

    ccd_exposure = ccd_simulator->getNumber("CCD_EXPOSURE");

    if (ccd_exposure == nullptr)
    {
        IDLog("Error: unable to find CCD Simulator CCD_EXPOSURE property...\n");
        return;
    }

    // Take a 1 second exposure
    IDLog("Taking a 1 second exposure.\n");
    ccd_exposure->np[0].value = 1;
    sendNewNumber(ccd_exposure);
}

/**************************************************************************************
**
***************************************************************************************/
void MyClient::newDevice(INDI::BaseDevice *dp)
{
	json nd;
	nd["metainfo"] = "newDevice";
	nd["name"] = dp->getDeviceName();
	clientQ->push(&nd);
}

/**************************************************************************************
**
*************************************************************************************/
void MyClient::newProperty(INDI::Property *property)
{
	ISwitchVectorProperty *svp;
	ITextVectorProperty *tvp;
	INumberVectorProperty *nvp;
	json indijson;
	switch(property->getType())
	{
		case INDI_SWITCH:
			svp = property->getSwitch();
			indijson = jsonify(svp);
			clientQ->push(&indijson);
		break;
		case INDI_NUMBER:
			nvp = property->getNumber();
			indijson = jsonify(nvp);
			clientQ->push(&indijson);
		break;
		case INDI_TEXT:
			tvp = property->getText();
			indijson = jsonify(tvp);
			clientQ->push(&indijson);
		break;

		default:
			std::cout << "IDK" << std::endl;
	}
	/*
	std::cout << "New property name  "<< property->getName() << std::endl;
	
    //connectDevice(property->getDeviceName());
	//watchDevice( property->getDeviceName() );
    if ( !strcmp(property->getName(), "CONNECTION") )
    {
        connectDevice(property->getDeviceName());
        return;
    }

    if (!strcmp(property->getDeviceName(), MYCCD) && !strcmp(property->getName(), "CCD_TEMPERATURE"))
    {
        if (ccd_simulator->isConnected())
        {
            IDLog("CCD is connected. Setting temperature to -20 C.\n");
            setTemperature();
        }
        return;
    }*/
}


void MyClient::newSwitch( ISwitchVectorProperty *svp )
{
		json jsvp=jsonify(svp);
		clientQ->push(&jsvp);
}

/**************************************************************************************
**
***************************************************************************************/
void MyClient::newNumber(INumberVectorProperty *nvp)
{
    // Let's check if we get any new values for CCD_TEMPERATURE
	
        //IDLog("Receving new CCD Temperature: %g C\n", nvp->np[1].value);
		json jnvp = jsonify(nvp);
			clientQ->push(&jnvp);

}

/**************************************************************************************
**
***************************************************************************************/
void MyClient::newMessage(INDI::BaseDevice *dp, int messageID)
{
    if (strcmp(dp->getDeviceName(), MYCCD))
        return;

    IDLog("Recveing message from Server:\n\n########################\n%s\n########################\n\n",
          dp->messageQueue(messageID).c_str());
}

/**************************************************************************************
**
***************************************************************************************/
void MyClient::newBLOB(IBLOB *bp)
{
    // Save FITS file to disk
    std::ofstream myfile;

    myfile.open("ccd_simulator.fits", std::ios::out | std::ios::binary);

    myfile.write(static_cast<char *>(bp->blob), bp->bloblen);

    myfile.close();

    IDLog("Received image, saved as ccd_simulator.fits\n");
}


json MyClient::jsonify(ISwitchVectorProperty* svp)
{
/*char 	device [MAXINDIDEVICE]
char 	name [MAXINDINAME]
char 	label [MAXINDILABEL]
char 	group [MAXINDIGROUP]
IPerm 	p
ISRule 	r
double 	timeout
IPState 	s
ISwitch * 	sp
int 	nsp
char 	timestamp [MAXINDITSTAMP]
void * 	aux*/

		json jsvp;
		json jsp;
		jsvp["metainfo"] = "svp";
		jsvp["device"] = svp->device;
		jsvp["name"] = svp->name;
		jsvp["label"] = svp->label;
		jsvp["group"] = svp->group;
		jsvp["perm"] = svp->p;
		jsvp["rule"] = svp->r;
		jsvp["timeout"] = svp->timeout;
		jsvp["state"] = svp->s;
		//jsvp["timestamp"] = std::string(svp->timestamp);
		
		jsvp["sp"] = jsp;
		ISwitch *sp;
		for(int ii=0; ii<svp->nsp; ii++)
		{
			sp = svp->sp+ii;
			jsvp["sp"][ii]["label"] = sp->label;
			jsvp["sp"][ii]["name"] = sp->name;
			jsvp["sp"][ii]["state"] = sp->s;
			
		}

	return jsvp;
}

json MyClient::jsonify(INumberVectorProperty *nvp)
{
		json jnvp;
		json jnp;
		jnvp["metainfo"] = "nvp";
		jnvp["device"] = nvp->device;
		jnvp["name"] = nvp->name;
		jnvp["label"] = nvp->label;
		jnvp["group"] = nvp->group;
		jnvp["perm"] = nvp->p;
		jnvp["timeout"] = nvp->timeout;
		jnvp["state"] = nvp->s;
		//jnvp["timestamp"] = nvp->timestamp;
		
		jnvp["np"] = jnp;
			
		INumber *np;
		for(int ii=0; ii<nvp->nnp; ii++)
		{
			np = nvp->np+ii;
			jnvp["np"][ii]["label"] = np->label;
			jnvp["np"][ii]["name"] = np->name; 
			jnvp["np"][ii]["format"] = np->format;
			jnvp["np"][ii]["min"] = np->min;
			jnvp["np"][ii]["step"] = np->step;
			jnvp["np"][ii]["max"] = np->max;
			jnvp["np"][ii]["value"] = np->value;
			
			
			
		}
	return jnvp;

}

json MyClient::jsonify(ITextVectorProperty *tvp)
{
/*
char 	device [MAXINDIDEVICE]
char 	name [MAXINDINAME]
char 	label [MAXINDILABEL]
char 	group [MAXINDIGROUP]
IPerm 	p
double 	timeout
IPState 	s
IText * 	tp
int 	ntp
char 	timestamp [MAXINDITSTAMP]
void * 	aux
*/
	json jtvp;
	IText *tp;
	jtvp["metainfo"] = "tvp";
	jtvp["device"] = tvp->device;
	jtvp["name"] = tvp->name;
	jtvp["label"] = tvp->label;
	jtvp["group"] = tvp->group;
	jtvp["perm"] = tvp->p;
	jtvp["timeout"] = tvp->timeout;
	jtvp["state"] = tvp->s;
	jtvp["timestsamp"] = "";
	for(int ii=0; ii<tvp->ntp; ii++)
	{
		tp=tvp->tp+ii;
		jtvp["tp"][ii]["name"] = tp->name;
		jtvp["tp"][ii]["label"] = tp->label;
		jtvp["tp"][ii]["text"] = tp->text;
	}
	return jtvp;

}

void MyClient::Update(json data)
{
	std::string devname;
	std::string grpname;
	std::string spname;
	std::string propname;
	
	INDI::BaseDevice *dev;
	ISwitchVectorProperty *svp;
	ISwitch *sp;
	

	if(data.find("newSwitch") != data.end() )
	{
		devname = data["newSwitch"]["device"];
		grpname = data["newSwitch"]["group"];
		propname = data["newSwitch"]["name"];
		
		dev = getDevice(devname.c_str());
		svp = dev->getSwitch( propname.c_str());
		for(int ii=0; ii<data["newSwitch"]["sp"].size(); ii++)
		{
			spname = data["newSwitch"]["sp"][ii]["name"];
			sp = IUFindSwitch(svp, spname.c_str());
			if(data["newSwitch"]["sp"][ii]["state"])
				sp->s = ISS_ON;
			else
				sp->s = ISS_OFF;
		}
		sendNewSwitch(svp);

	}
}

int main(int /*argc*/, char ** /*argv*/)

{
	ComQ comQ;
	ComQ devQ;
	camera_client->setQ(&comQ);
	camera_client->setDevQ(&devQ);
	
    camera_client->setServer("localhost", 7624);
	
	
    camera_client->connectServer();
	std::vector< INDI::BaseDevice * >  devs;
	camera_client->getDevices(devs, INDI::BaseDevice::GENERAL_INTERFACE );
	for(int i=0; i<devs.size(); i++ )
	{
		std::cout << "THe dev name is " << devs[i]->getDeviceName() << std::endl;
	}
    //camera_client->watchDevice(MYCCD);

    //camera_client->setBLOBMode(B_ALSO, MYCCD, nullptr);
	
	std::thread t1(WSthread, &comQ, &devQ);
	comQ.connected = true;
	while(comQ.connected)
	{
		if(devQ.size() !=0)
		{
			
			
			camera_client->Update(json::parse(devQ.pop()))	;
			
		}
		usleep( (int) 5e5 );
	}
	test=false;
	t1.join();
}
