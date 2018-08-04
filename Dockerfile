FROM ubuntu:bionic
MAINTAINER Scott Swindell <srswinde@gmail.com>


WORKDIR webclient
ADD . /webclient



ENTRYPOINT /webclient/websocketd --port=3000 --devconsole /webclient/webclient $INDISERVER
