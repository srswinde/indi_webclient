#include "basedevice.h"
#include "/usr/include/libindi/indicom.h"
#include <fstream>
#include <iostream>
#include <memory>
#include <string.h>
#include <thread>
#include "nlohmann/json.hpp"
#include <mutex>
#include <queue>
#include <string>
#include <sstream>
#include <ostream>
#include <unistd.h>
using json = nlohmann::json;
#include "webclient.h"
bool test = true;

/* Our client auto pointer */
std::unique_ptr<MyClient> web_client(new MyClient());


void ComQ::push(json *data)
{
	counter++;
	std::lock_guard<std::mutex> lock(qutex);
	q.push( data->dump() );
	if( q.size() > 500 )
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
	counter--;
	return front;
}


/********************************************************************
 * WSthread
 * Args:
 *      q-> queue that handles json data form indidriver to webpage
 *      driverQ -> queue that handles json data from webpage to indidriver
 * Description:
 *      THis is the thread that handles the websocket communiccation
 *      between this program and the webpage. 
 *
 *
 *
 *
 *
 *
 * ***********************************************************************/
void WSthread(ComQ *websocQ, ComQ  *driverQ)
{


	char buff[5000];
	while (websocQ->connected)
	{
		std::string input;
		json inj;
		
		getline(std::cin, input)	;


		try
		{
			inj = json::parse(input);
		}
		catch(...)
		{
			//std::cerr << cinstream << std::endl;
			continue;
		}

		if(inj["task"] != "getProperties")
			driverQ->push(&inj);
		else
			

		while(websocQ->size()>0)
		{
			if(websocQ->size() > 0)	
			{
				
				strcpy( buff, websocQ->front().c_str() );
				std::cout << buff << std::endl;
				websocQ->pop( );
			}
			usleep( (int) 250);
		}
		//ws->close();
	}


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
			//indijson = jsonify(svp);
			//clientQ->push(&indijson);
			newSwitch(svp);
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
		
		json jnvp = jsonify(nvp);
		
		clientQ->push(&jnvp);

}
/**************************************************************************************
**
***************************************************************************************/
void MyClient::newText(ITextVectorProperty *tvp)
{
		
		json jtvp = jsonify(tvp);
		clientQ->push(&jtvp);

}
/**************************************************************************************
**
***************************************************************************************/
void MyClient::newMessage(INDI::BaseDevice *dp, int messageID)
{
	json jmsg = jsonify(dp->messageQueue(messageID), dp->getDeviceName());
	
	clientQ->push(&jmsg);

          
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


/*****************************************************************************************
 * jsonify
 *      args: any of the indi vector properties
 *
 * Description:
 *      All jsonify functions converts the indi vector property to to a nice
 *      json object to ship off to the webpage
 *
 *
 *
 *
 *
 *
 *
 * *****************************************************************************************/
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
		char buffer[MAXINDIFORMAT+5];
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
			//jnvp["np"][ii]["value"] = np->value;
			numberFormat(buffer, np->format, np->value);
			
			jnvp["np"][ii]["value"] = buffer;
			
			
			
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


json MyClient::jsonify(std::string msg, const char *devname)
{
	json jmsg;
	jmsg["metainfo"] = "msg";
	jmsg["msg"] = msg;
	jmsg["device"] = devname;
	return jmsg;

}


/*******************************
* Update
* Args: data-> this is the json
* data that has come from the web page
*
* Description:
* This is were new indi properties 
* come in from the web page
* and are sent off to the client
*
*
*
*
*
***************************/

void MyClient::Update(json data)
{
	
	std::string devname;
	std::string grpname;
	std::string propname;
		
	char strval[20];

	std::string spname;
	std::string npname;
	std::string tpname;

	std::string text;

	INDI::BaseDevice *dev;

	ISwitchVectorProperty *svp;
	ISwitch *sp;
	INumberVectorProperty *nvp;
	INumber *np;
	ITextVectorProperty *tvp;
	IText *tp;



	if(data.find("newSwitch") != data.end() )
	{
		devname = data["newSwitch"]["device"];
		grpname = data["newSwitch"]["group"];
		propname = data["newSwitch"]["name"];
	
		dev = getDevice(devname.c_str());
		svp = dev->getSwitch( propname.c_str());
		for(unsigned int ii=0; ii<data["newSwitch"]["sp"].size(); ii++)
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
	else if( data.find("newNumber") != data.end() )
	{
		devname = data["newNumber"]["device"];
		grpname = data["newNumber"]["group"];
		propname = data["newNumber"]["name"];
		
		dev = getDevice(devname.c_str());
		nvp = dev->getNumber( propname.c_str());
		for(unsigned int ii=0; ii<data["newNumber"]["np"].size(); ii++)
		{
			
			npname = data["newNumber"]["np"][ii]["name"];
			np = IUFindNumber(nvp, npname.c_str());
			text = data["newNumber"]["np"][ii]["value"];
			strcpy(strval, text.c_str());
			
			f_scansexa(strval, &np->value );
			
			
		}
		sendNewNumber(nvp);
		

	}
	else if( data.find("newText") != data.end() )
	{
		devname = data["newText"]["device"];
		grpname = data["newText"]["group"];
		propname = data["newText"]["name"];
		dev = getDevice(devname.c_str());
		tvp = dev->getText( propname.c_str());
		
		for(unsigned int ii=0; ii<data["newText"]["tp"].size(); ii++)
		{
			tpname = data["newText"]["tp"][ii]["name"];
			tp = IUFindText(tvp, tpname.c_str());
			text = data["newText"]["tp"][ii]["text"];
			strcpy(tp->text, text.c_str());
		}
		sendNewText(tvp);
	}
}

int main(int argc, char ** argv )
{
	ComQ webQ; //Q for communicating with web page
	ComQ driverQ;//Q for communicating with indiserver
	web_client->setQ(&webQ);
	web_client->setDevQ(&driverQ);
	web_client->setServer(argv[1], 7624);
    	#include "basedevice.h"
#include "/usr/include/libindi/indicom.h"
#include <fstream>
#include <iostream>
#include <memory>
#include <string.h>
#include <thread>
#include "nlohmann/json.hpp"
#include <mutex>
#include <queue>
#include <string>
#include <sstream>
#include <ostream>
#include <unistd.h>
using json = nlohmann::json;
#include "webclient.h"
bool test = true;

/* Our client auto pointer */
std::unique_ptr<MyClient> web_client(new MyClient());


void ComQ::push(json *data)
{
	counter++;
	std::lock_guard<std::mutex> lock(qutex);
	q.push( data->dump() );
	if( q.size() > 500 )
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
	counter--;
	return front;
}


/********************************************************************
 * WSthread
 * Args:
 *      q-> queue that handles json data form indidriver to webpage
 *      driverQ -> queue that handles json data from webpage to indidriver
 * Description:
 *      THis is the thread that handles the websocket communiccation
 *      between this program and the webpage. 
 *
 *
 *
 *
 *
 *
 * ***********************************************************************/
void WSthread(ComQ *websocQ, ComQ  *driverQ)
{


	char buff[5000];
	while (websocQ->connected)
	{
		std::string input;
		json inj;
		
		getline(std::cin, input)	;


		try
		{
			inj = json::parse(input);
		}
		catch(...)
		{
			//std::cerr << cinstream << std::endl;
			continue;
		}

		if(inj["task"] != "getProperties")
			driverQ->push(&inj);
		else
			

		while(websocQ->size()>0)
		{
			if(websocQ->size() > 0)	
			{
				
				strcpy( buff, websocQ->front().c_str() );
				std::cout << buff << std::endl;
				websocQ->pop( );
			}
			usleep( (int) 250);
		}
		//ws->close();
	}


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
			//indijson = jsonify(svp);
			//clientQ->push(&indijson);
			newSwitch(svp);
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
		
		json jnvp = jsonify(nvp);
		
		clientQ->push(&jnvp);

}
/**************************************************************************************
**
***************************************************************************************/
void MyClient::newText(ITextVectorProperty *tvp)
{
		
		json jtvp = jsonify(tvp);
		clientQ->push(&jtvp);

}
/**************************************************************************************
**
***************************************************************************************/
void MyClient::newMessage(INDI::BaseDevice *dp, int messageID)
{
	json jmsg = jsonify(dp->messageQueue(messageID), dp->getDeviceName());
	
	clientQ->push(&jmsg);

          
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


/*****************************************************************************************
 * jsonify
 *      args: any of the indi vector properties
 *
 * Description:
 *      All jsonify functions converts the indi vector property to to a nice
 *      json object to ship off to the webpage
 *
 *
 *
 *
 *
 *
 *
 * *****************************************************************************************/
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
		char buffer[MAXINDIFORMAT+5];
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
			//jnvp["np"][ii]["value"] = np->value;
			numberFormat(buffer, np->format, np->value);
			
			jnvp["np"][ii]["value"] = buffer;
			
			
			
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


json MyClient::jsonify(std::string msg, const char *devname)
{
	json jmsg;
	jmsg["metainfo"] = "msg";
	jmsg["msg"] = msg;
	jmsg["device"] = devname;
	return jmsg;

}


/*******************************
* Update
* Args: data-> this is the json
* data that has come from the web page
*
* Description:
* This is were new indi properties 
* come in from the web page
* and are sent off to the client
*
*
*
*
*
***************************/

void MyClient::Update(json data)
{
	
	std::string devname;
	std::string grpname;
	std::string propname;
		
	char strval[20];

	std::string spname;
	std::string npname;
	std::string tpname;

	std::string text;

	INDI::BaseDevice *dev;

	ISwitchVectorProperty *svp;
	ISwitch *sp;
	INumberVectorProperty *nvp;
	INumber *np;
	ITextVectorProperty *tvp;
	IText *tp;



	if(data.find("newSwitch") != data.end() )
	{
		devname = data["newSwitch"]["device"];
		grpname = data["newSwitch"]["group"];
		propname = data["newSwitch"]["name"];
	
		dev = getDevice(devname.c_str());
		svp = dev->getSwitch( propname.c_str());
		for(unsigned int ii=0; ii<data["newSwitch"]["sp"].size(); ii++)
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
	else if( data.find("newNumber") != data.end() )
	{
		devname = data["newNumber"]["device"];
		grpname = data["newNumber"]["group"];
		propname = data["newNumber"]["name"];
		
		dev = getDevice(devname.c_str());
		nvp = dev->getNumber( propname.c_str());
		for(unsigned int ii=0; ii<data["newNumber"]["np"].size(); ii++)
		{
			
			npname = data["newNumber"]["np"][ii]["name"];
			np = IUFindNumber(nvp, npname.c_str());
			text = data["newNumber"]["np"][ii]["value"];
			strcpy(strval, text.c_str());
			
			f_scansexa(strval, &np->value );
			
			
		}
		sendNewNumber(nvp);
		

	}
	else if( data.find("newText") != data.end() )
	{
		devname = data["newText"]["device"];
		grpname = data["newText"]["group"];
		propname = data["newText"]["name"];
		dev = getDevice(devname.c_str());
		tvp = dev->getText( propname.c_str());
		
		for(unsigned int ii=0; ii<data["newText"]["tp"].size(); ii++)
		{
			tpname = data["newText"]["tp"][ii]["name"];
			tp = IUFindText(tvp, tpname.c_str());
			text = data["newText"]["tp"][ii]["text"];
			strcpy(tp->text, text.c_str());
		}
		sendNewText(tvp);
	}
}

int main(int argc, char ** argv )
{
	ComQ webQ; //Q for communicating with web page
	ComQ driverQ;//Q for communicating with indiserver
	web_client->setQ(&webQ);
	web_client->setDevQ(&driverQ);
	web_client->setServer(argv[1], 7624);
    
	 web_client->setDevQ(&driverQ);                              
	if(argc == 2)                                               
		web_client->setServer(argv[1], 7624);                   
	else if(argc == 3)                                          
		web_client->setServer(argv[1], atoi(argv[2]));          
	else                                                        
		web_client->setServer("localhost", 7624);               

    	while(web_client->connectServer() == false)
	{
		usleep(2e6);
	}
	std::vector< INDI::BaseDevice * >  devs;
	web_client->getDevices(devs, INDI::BaseDevice::GENERAL_INTERFACE );
	
	std::thread t1(WSthread, &webQ, &driverQ);
	webQ.connected = true;
	while(webQ.connected)
	{
		if(driverQ.size() !=0)
		{
			web_client->Update(json::parse(driverQ.pop()));
		}
		usleep( (int) 1e5 );
	}
	webQ.connected = false;
	test=false;
	t1.join();
}

	
    	while(web_client->connectServer() == false)
	{
		usleep(2e6);
	}
	std::vector< INDI::BaseDevice * >  devs;
	web_client->getDevices(devs, INDI::BaseDevice::GENERAL_INTERFACE );
	
	std::thread t1(WSthread, &webQ, &driverQ);
	webQ.connected = true;
	while(webQ.connected)
	{
		if(driverQ.size() !=0)
		{
			web_client->Update(json::parse(driverQ.pop()));
		}
		usleep( (int) 1e5 );
	}
	webQ.connected = false;
	test=false;
	t1.join();
}
