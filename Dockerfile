FROM ubuntu:bionic
MAINTAINER Scott Swindell <srswinde@gmail.com>


WORKDIR webclient
ADD . /webclient


ENTRYPOINT bash
#ENTRYPOINT /webclient/websocketd --port=3000 --devconsole /webclient/webclient
#CMD ["/webclient/websocketd", "--port", "3000", "/webclient/webclient"]
#CMD ["bash"]
