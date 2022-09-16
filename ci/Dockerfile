FROM ubuntu:20.04
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update
RUN apt-get install -y git sudo g++ cmake 
RUN apt-get install -y clang-format gdb build-essential
RUN apt-get install -y libtbb-dev
# Put all the library sources in libsrc
RUN mkdir /libsrc 
WORKDIR /libsrc 

# Install libpqxx for Postgres <=> C++ interface (based on libpq)
RUN apt-get install -y libpq-dev postgresql-server-dev-all
RUN git clone --depth 1 https://github.com/jtv/libpqxx.git --branch master --single-branch
RUN cd libpqxx && mkdir build && cd build && cmake .. && make install

# Install yaml-cpp
RUN git clone --depth 1 https://github.com/jbeder/yaml-cpp.git --branch master --single-branch
RUN cd yaml-cpp && mkdir build && cd build && cmake .. && make install

# Copy directory content to libsrc and install pgi
RUN mkdir /libsrc/pgi 
COPY . /libsrc/pgi
RUN cd pgi && mkdir -p build && cd build && cmake .. && make install

# Copy example directory content to app and try to run
# RUN mkdir /app
# COPY ./example /app
# WORKDIR /app
WORKDIR /libsrc/pgi 


# Use non root user to avoid ownership problems for the user
# See https://code.visualstudio.com/remote/advancedcontainers/add-nonroot-user 
ARG USERNAME=devuserpgi
ARG USER_UID=1000
ARG USER_GID=$USER_UID

# Create the user
RUN groupadd --gid $USER_GID $USERNAME \
    && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME -s /bin/bash \
    #
    # [Optional] Add sudo support. Omit if you don't need to install software after connecting.
    && apt-get update \
    && apt-get install -y sudo \
    && echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME \
    && chmod 0440 /etc/sudoers.d/$USERNAME

# ********************************************************
# * Anything else you want to do like clean up goes here *
# ********************************************************

# [Optional] Set the default user. Omit if you want to keep the default as root.
USER $USERNAME

