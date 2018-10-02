FROM ubuntu:bionic
MAINTAINER Scott Swindell <srswinde@gmail.com>


WORKDIR webclient
ADD . /webclient


CMD /webclient/websocketd --address=127.0.0.1 --port=3000 --devconsole /webclient/webclient $INDISERVER
