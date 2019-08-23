FROM ubuntu:bionic
MAINTAINER Scott Swindell <srswinde@gmail.com>


WORKDIR webclient
ADD . /webclient


CMD ["/webclient/websocketd", "--address", "0.0.0.0", "--port", "3000", "--devconsole", "/webclient/webclient", "$INDISERVER"]
