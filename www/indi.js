INDIPERM_RO = 0;
INDIPERM_WO = 1;
INDIPERM_RW = 2;

INDISTATE_IDLE = 0;
INDISTATE_OK = 1;
INDISTATE_BUSY = 2;
INDISTATE_ALERT = 3;

INDISWRULE_1OFMANY = 0;
INDISWRULE_ATMOST1 = 1;
INDISWRULE_NOFMANY = 2;


/*********************
* formatNumber
* Args: numStr=>the number as a string
*		fstr=> the INDI format string
*
* Description:
*		Format the floating point INDI numbers
* but don't mess with the sexagesimal
* stuff. weh shall let the INDI client
* do that. In fact we should probably
* let the client do the floating point stuff 
* too. 
*
**********************/

function formatNumber(numStr, fstr)
{
	num = parseFloat(numStr);
	var outstr;
	var total = parseInt( fstr.slice(0, fstr.length-1).split('.')[0] );
	var decimal = parseInt( fstr.slice(0, fstr.length-1).split('.')[1] );
	switch(fstr[fstr.length-1])
	{
		case 'f':
			outStr = num.toFixed(decimal);
			
		break;
		case 'i':
			outStr = String( Math.round(num) )
		break;
		default:
			outStr = numStr
	}
	return outStr;
}

function nosp(str)
{
	return str.replace(' ', '_').replace('.', '__');
}


/************************************

* collect
* 
* Description:
*		sends a request for data 10 times a 
*		second. 
*
*
*************************************/
function collect()
{
	try
	{
		INDIws.send(JSON.stringify({'task':'getProperties'}));
	}
	catch(err)
	{
		console.log("The error is ", err);
	}
	setTimeout( collect, 100);
}



/*****************************************

* INDIwebsocket
* Args:
*		url=>the url of the websocket server
*   container=> the jQuery style selector
*		of the div to contain the devices. 
*
* Description:
*		This function opens the websocket to 
*		INDI client and manages all of the 
*		communication. ALL new devices, messages,
*		and values, etc come through here and 
*		are handled bu the various handling 
*		functions.
*
*
******************************************/
function INDIwebsocket(url, container, tabdevice)
{
	if(url == undefined)
	{
		url = "ws://swindell:3000"
	}
	
	if (tabdevice == undefined)
	{
		tabdevice = false;
	}


	container = (container == "undefined")? "body" : container
	INDIws = new WebSocket( url );
	INDIws.devices_container = container
	INDIws.onerror = function(event)
	{
		$("#wsDialog").dialog("open").find('b').text(url);
	}
	INDIws.onmessage = function( event )
	{
		var data = JSON.parse( event.data );
		var ele = '';
		var newData = false;
		container = this.devices_container;
		switch (data.metainfo)
		{
			case "newDevice":
				AddDevice(data.name, container, tabdevice);
			break;
			case "nvp":
				ele = newNumber( data );
				newData = true;
			break;
			case "svp":
			
				newData = true;
				ele = newSwitch( data );
			break;
			case "tvp":
				newData = true;
				ele = newText( data );
			break;
			case "msg":
				var msgselector = "textarea#INDImsg";
				var msgarea = $(msgselector);
				msgarea.text( data.msg+'\n'+msgarea.text() );

			break;
			default:
				console.log("IDK", data.metainfo);
		}
		if(newData)
		{
			var container = postProc( data, ele ) 
			if( typeof(ele) != 'string' )
			{
				ele.appendTo( container )
			}
		}
		
	};
	INDIws.onerror = function(event)
	{
		//alert("There was an error!", event)
	}
	INDIws.onclose = function(event)
	{
		alert("The connection has closed! If possible restart the webserver. This interface will reload when you hit ok.");
		location.reload()
	}

	INDIws.onopen = function(event) 
	{
		collect();
		
	};
}	




/************************************************************
* AddDevice
* args 
*	devname-> indi device name, 
*	container-> the jquery selector string of the containing element
*	tabdevice -> boolean value if true use jquery tabs to seperate
*		devices if not but them in divs
*
*
* Description:
*	When a new device is sent from the INDI driver add it to the 
* 	webpage either using jquery tabs or simply putting eache device
*	in its own div. 
*
************************************************************/

function AddDevice(devname, container, tabdevice)
{
	var devselector = "div.INDIdevice#"+nosp(devname);
	container = (container==undefined)? 'body':container;
	var uler = "ul";

	if($(container).find(uler).length == 0)
	{
		$("<ul/>" ).appendTo(container) 
	}
	if( $(container).find( devselector ).length == 0 ) 
	{
		if(tabdevice)
		{
			var ul = $(container).find(uler);
			$("<li><a href='#"+nosp(devname)+"'>"+devname+"</a></li>").appendTo( ul );
			$("<div/>", {id:nosp(devname)}).addClass("INDIdevice").appendTo( container );
		
			$(container).tabs();
		}
	}
	if (tabdevice)
		$(container).tabs("refresh")
	return devselector;
		
}

/***********************************************************
* newText 
* Args INDIvp-> object defining the INDI vector propert, 
*		appendTo -> jquery selector for which elemebt to 
*		append the INDivp turned HTML element to.
*
* Desription:
*	Called when the websocket from the indi webclient
*	generates or updates an INDI text. If this a 
*	a never brefore seen INDI text HTML fieldset
*	element is created with the correct value otherwise
*	the element's text is updated. 
*
*
* Returns: a jquery type selector string. 
*
*********************************************************/

function newText( INDIvp, appendTo )
{
	var nosp_vpname = INDIvp.name.replace( " ", "_" );
	var vpselector = "fieldset.INDItvp#"+nosp_vpname+"[device='"+INDIvp.device+"']";
	var nosp_dev = INDIvp.device.replace( " ", "_" );
	var retn;
	if( $(vpselector).length == 0 )
	{
		var vphtmldef = $("<fieldset class='INDIvp INDItvp'></fieldset>")
			.prop("id", nosp_vpname)
			.attr("device", INDIvp.device)
			.attr("group", INDIvp.group)
			.append("<legend>"+INDIvp.label+"</legend>");
		
		$.each(INDIvp.tp, function(ii, tp)
		{		
			var label = tp.label.replace(" ", "_");
			var name = tp.name.replace(' ', '_');
			var tpid = nosp_dev+"__"+name;
	
			vphtmldef.append($('<span/>',
			{
				'id': name,
				'class': 'ITextspan',
				'INDIlabel':tp.label,
				'INDIname':tp.name,
			}
			).append($('<label/>',
			{
				'text':	tp.label,
				'for': tpid	
			})
			).append( function()
			{
				var ro = $('<textarea rows=1 readonly/>').addClass('IText_ro').text(tp.text)
				var wo = $('<textarea rows=1/>').addClass('IText_wo')
				.keypress(function(event)
				{
					if(event.which == 13)	
					{
						sendNewText(event)
						return false;
					}
				})
				switch(INDIvp.perm)
				{
					case INDIPERM_RO:
							$(this).append(ro);
					break;
					case INDIPERM_RW:
							$(this).append(ro).append(wo);
					break;
					case INDIPERM_WO:
							
							$(this).append(wo);
					break;

				}
			}
			))
		});
		if(appendTo != undefined)
		{
			vphtmldef.appendTo(appendTo);
		}
		return vphtmldef
	}
	//console.log( $(vpselector) )
	
	$.each(INDIvp.tp, function(ii, tp)
	{
		var label = tp.label.replace(" ", "_")
		var name = nosp(tp.name);
		var tpid = nosp_dev+name;
		$(vpselector).find("span[INDIname='"+name+"'] textarea.IText_ro").text(tp.text)
	});

	return vpselector
}



/***********************************************************
* newNumber 
* Args INDIvp-> object defining the INDI vector propert, 
*		appendTo -> jquery selector for which elemebt to 
*		append the INDivp turned HTML element to.
*
* Desription:
*	Called when the websocket from the indi webclient
*	generates or updates an INDI number. If this a 
*	a never brefore seen INDI number, an HTML fieldset
*	element is created with the correct value otherwise
*	the element's number is updated. 
*
*
* Returns: a jquery type selector string. 
*
*********************************************************/

function newNumber(INDIvp, appendTo)
{
	var nosp_vpname = INDIvp.name.replace( " ", "_" );
	var vpselector = "fieldset.INDInvp#"+nosp_vpname+"[device='"+INDIvp.device+"']";
	var nosp_dev = INDIvp.device.replace( " ", "_" );
	var retn;
	//we need to create the html
	if( $(vpselector).length == 0 )
	{
		var vphtmldef = $("<fieldset class='INDIvp INDInvp'></fieldset>")
			.prop("id", nosp_vpname)
			.attr("device", INDIvp.device)
			.attr("group", INDIvp.group)
			.append("<legend>"+INDIvp.label+"</legend>");
		
			
		$.each(INDIvp.np, function(ii, np)
		{		
			var label = np.label.replace(" ", "_");
			var name = np.name.replace(' ', '_');
			var npid = nosp_dev+"__"+name;
	
			vphtmldef.append($('<span/>',
			{
				'id': name,
				'class': 'INumberspan',
				'INDIlabel':np.label,
				'INDIname':np.name,
			}
			).append($('<label/>',
			{
				'text':	np.label,
				'for': npid	
			})
			).append( function()
			{
				var re = /%(\d+)\.(\d+)[fm]/
				var numinfo = re.exec(np.format);
				var len = parseInt(numinfo[1]);
				var ro = $('<span/>', {'class':'INumber_ro'}).css({ width:10*len+'px' })
					
				var wo = $("<input/>", {'type':'text', 'class':'INumber_wo'}).prop('size',len)
				.attr("value", np.value )
				.keypress(function(event)
				{
					
					if(event.which == 13)	
					{
						sendNewNumber(event)
					}
				})
				switch(INDIvp.perm)
				{
					case INDIPERM_RO:
							$(this).append(ro);
					break;
					case INDIPERM_RW:
							$(this).append(ro).append(wo);
					break;
					case INDIPERM_WO:
							
							$(this).append(wo);
					break;

				}
			}
			))
		});
		if(appendTo != undefined)
		{
			vphtmldef.appendTo(appendTo);
			return vpselector;
		}
		return vphtmldef
	}
	
	
	$.each( INDIvp.np, function(ii, np)
	{
		var label = np.label.replace(" ", "_")
		var name = np.name.replace(' ', '_');
		var npid = nosp_dev+name;
		newvalue= formatNumber(np.value, np.format)
		//$(vpselector).find("span.INumberspan[INDIname='"+np.name+"']  span.INumber_ro").text(Math.round(np.value*10000)/10000)
		$(vpselector).find("span.INumberspan[INDIname='"+np.name+"']  span.INumber_ro").text(newvalue)

	});
	// return the jquery selector
	return vpselector
}
/*end newNumber*/


/***********************************************************
* newSwitch 
* Args INDIvp-> object defining the INDI vector propert, 
*		appendTo -> jquery selector for which elemebt to 
*		append the INDivp turned HTML element to.
*
* Desription:
*	Called when the websocket from the indi webclient
*	generates or updates an INDI switch. If this a 
*	a never brefore seen INDI switch HTML fieldset
*	element is created with the correct value otherwise
*	the element's switch is updated. 
*
*
* Returns: a jquery type selector string. 
*
*********************************************************/

function newSwitch( INDIvp, appendTo )
{
	var nosp_vpname = INDIvp.name.replace( " ", "_" );
	var nosp_dev = INDIvp.device.replace( " ", "_" );
	var vpselector = "fieldset.INDIvp#"+nosp_vpname+"[device='"+INDIvp.device+"']";
	

	var retn = $(vpselector);
	switch(INDIvp.rule)
	{
		case INDISWRULE_1OFMANY:
		case INDISWRULE_ATMOST1:
			type = 'radio'
		break;
		case INDISWRULE_NOFMANY:
			type = 'checkbox';
		break;
	}
	
	if( $(vpselector).length == 0 )
	{
		
		var vphtmldef = $( "<fieldset class='INDIvp INDIsvp'/>" )
			.prop("id", nosp_vpname)
			.attr("device", INDIvp.device)
			.attr("group", INDIvp.group)
			.append("<legend>"+INDIvp.label+"</legend>");
		
		$.each(INDIvp.sp, function(ii, sp)
		{		
			var label = sp.label.replace(" ", "_");
			var name = sp.name.replace(' ', '_');
			var spid = nosp_dev+"__"+name;
			var spname = nosp_dev+'__'+nosp_vpname
			vphtmldef.append
			(
				$('<span/>',
				{	
					'INDIlabel'	:sp.label,
					'INDIname'	:sp.name,
					'class'			:"ISwitchspan",
					'id'				:name,
				})
				.append($('<label/>', 
				{
					'id'				:name,
					'class'			:'ISwitchlabel',
					'for'				:spid,
					'text'			:sp.label

				})).append(
				$('<input/>',
				{
					'type'		:type,
					'id'			:spid,
					'name'		:spname,
					'class'		:'ISwitchinput',
				}
				).on( 'change', sendNewSwitch )
				.prop( "checked", sp.state )
			));
			
		});

		vphtmldef.find( "input[type='"+type+"']" ).checkboxradio();

		if( appendTo != undefined )
		{
			vphtmldef.appendTo( appendTo );
			return vpselector;
		}
		return vphtmldef;
	}

	else
	{
		$.each(INDIvp.sp, function(ii, sp)
		{
			var label = sp.label.replace(" ", "_");
			var name = sp.name.replace(' ', '_');
			var spid = nosp_dev +"__"+ name;
			var state = sp.state
			$(vpselector).find('input.ISwitchinput#'+spid).prop('checked', sp.state);
			
			//console.log($("body fieldset.INDIsvp#"+nosp_vpname+"[device='"+INDIvp.device+"']"))
		});
	}
	$(vpselector).find("input[type='"+type+"']").checkboxradio("refresh");
	return vpselector;
}




/*******************************************************************************
* sendNewSwitch
* args: event-> the javascript event that caused the function to be called (normally a click)
*
* Description:
*	This function is called by an html event. The changed state of the indi switch
* 	is then sent over the websocket to the webclient to update the indi driver. 
*
*
*
*
*
*
*
*
*
*******************************************************************************/
function sendNewSwitch(event)
{
	var fs = $(event.target).closest(".INDIsvp")
	
	var out = {
		"task":"updateSwitch",
		"newSwitch":{
		"device":fs.attr('device'),
		"group":fs.attr('group'),
		"name":fs.prop('id'),
		"sp":[]//{"name":$(event.target).attr("INDIname"),"state":$(event.target).prop("checked")},
		}
	};
	fs.find("input.ISwitchinput").each(function(ii, sp)
	{
		out.newSwitch.sp.push({
			"name":$(sp).closest( "span.ISwitchspan" ).attr( "INDIname" ),
			"label":$(sp).closest( "span.ISwitchspan" ).attr( "INDIlabel" ),
			"state":$(sp).closest( "span.ISwitchspan" ).find("input.ISwitchinput").prop( "checked" ),
			
		})
	})
	INDIws.send(JSON.stringify(out));
	
	
}


/*******************************************************************************
* sendNewNumber
* args: event-> the javascript event that caused the function to be called (normally a return key)
*
* Description:
*	This function is called by an html event. The changed state of the indi number
* 	is then sent over the websocket to the webclient to update the indi driver. 
*
*
*
*******************************************************************************/

function sendNewNumber(event)
{
	var fn = $(event.target).parent().parent(".INDInvp");
	var INumber = $(event.target)
	var out = {
		"task":"updateNumber",
		"newNumber":{
		"device":fn.attr('device'),
		"group":fn.attr('group'),
		"name":fn.prop('id'),
		"np":[],
		}
	};
	fn.find("span.INumberspan input.INumber_wo").each(function(ii, np)
	{	
		out.newNumber.np.push(
		{
			"name":$(np).closest('span.INumberspan').attr("INDIname"),
			"label":$(np).parent().attr("INDIlabel"),
			"value": $(np).prop("value") 
			
		});
	});
	INDIws.send(JSON.stringify(out));
}



/*******************************************************************************
* sendNewText
* args: event-> the javascript event that caused the function to be called (normally a return key)
*
* Description:
*	This function is called by an html event. The changed state of the indi text
* 	is then sent over the websocket to the webclient to update the indi driver. 
*
*
*
*******************************************************************************/

function sendNewText(event)
{
	var ft = $(event.target).closest(".INDItvp");
	var IText = $(event.target);
	var out = {
		"task":"updateText",
		"newText":{
		"device":ft.attr('device'),
		"group":ft.attr('group'),
		"name":ft.prop('id'),
		"tp":[],
		}
	};

	ft.find("textarea.IText_wo").each(function( ii, tp ) 
	{
		console.log($(IText).closest(".IText"));
		out.newText.tp.push(
		{
			"name":$(IText).closest("span.ITextspan").attr("INDIname"),
			"label":$(IText).closest("span.ITextspan").attr("INDIlabel"),
			"text":IText.prop("value"),

		});

	}); 
	INDIws.send(JSON.stringify(out));
}

