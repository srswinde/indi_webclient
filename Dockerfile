FROM ubuntu:bionic
MAINTAINER Scott Swindell <srswinde@gmail.com>


WORKDIR webclient
ADD . /webclient

CMD ["/webclient/websocketd", "--port", "3000", "/webclient/webclient"]
#CMD ["bash"]
