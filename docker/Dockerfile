FROM ubuntu:16.04

MAINTAINER stane.shieh@wnc.com.tw

RUN apt-get update

RUN apt install -y make gcc perl vim unzip bzip2 lib32z1 \
lib32ncurses5 bc mtd-utils u-boot-tools libncurses-dev \
g++ rsync patch wget texinfo \
tree cpio device-tree-compiler gem ruby ruby-dev libfdt-dev python

RUN cd /usr/bin && ln -s make gmake
ENV PATH "/opt/armv7-marvell-linux-gnueabi-softfp/bin:$PATH"

RUN apt-get clean
