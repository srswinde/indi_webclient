The web INDI client. 


### Description ###
This is an INDI client for the web. 
It uses websockets as the communication medium and
converts the INDI lilxml to JSON to help with the javascript
side. 


### Requirements ####
JSON (de)serialization is done by nhloman
nhloman::json <https://github.com/nlohmann/json>


Websocket  library is uwebsockets
<https://github.com/uNetworking/uWebSockets>

I hope to move away from uwebsockets in the future
and use websocketd. 


And of course the full INDI development API available with apt. 


### build ###
make
