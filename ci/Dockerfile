FROM ubuntu:20.04
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update
RUN apt-get install -y git sudo g++ cmake 

# Put all the library sources in libsrc
RUN mkdir /libsrc 
WORKDIR /libsrc 

# Install libpqxx for Postgres <=> C++ interface (based on libpq)
RUN apt-get install -y libpq-dev postgresql-server-dev-all
RUN git clone --depth 1 https://github.com/jtv/libpqxx.git --branch 7.6.0 --single-branch
RUN cd libpqxx && mkdir build && cd build && cmake .. && make install

# Install yaml-cpp
RUN git clone --depth 1 https://github.com/jbeder/yaml-cpp.git --branch master --single-branch
RUN cd yaml-cpp && mkdir build && cd build && cmake .. && make install

# Copy directory content to libsrc and install pgi
RUN mkdir /libsrc/pgi 
COPY . /libsrc/pgi
RUN cd pgi && mkdir -p build && cd build && cmake .. && make install

# Copy example directory content to app and try to run
RUN mkdir /app
WORKDIR /app 
COPY ./example /app