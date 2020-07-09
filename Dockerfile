FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
  apt-get -y install \
    cmake \
    g++ \
    libhdf5-dev \
    libssl-dev
