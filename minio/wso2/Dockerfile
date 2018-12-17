# Warning: does not really work, do not use it as-is

FROM debian:stretch

RUN apt-get -yqq update && apt-get -yqq install wget
RUN wget --progress=dot:giga -O wso.deb https://product-dist.wso2.com/downloads/identity-server/5.7.0/downloader/wso2is-linux-installer-x64-5.7.0.deb && dpkg -i wso.deb && rm wso.deb

COPY identity.xml.patch .
RUN apt-get -yqq install patch && patch -d / -p1 < identity.xml.patch

CMD ["/usr/bin/wso2is-5.7.0"]

