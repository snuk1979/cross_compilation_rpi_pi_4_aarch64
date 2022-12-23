FROM ubuntu:20.04

ARG DEBIAN_FRONTEND=noninteractive

# Add a user called `develop`
RUN useradd -ms /bin/bash develop

ENV TZ=Europe/Kiev
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt-get -qy update --fix-missing
RUN apt-get install -y binutils-aarch64-linux-gnu gcc-aarch64-linux-gnu g++-aarch64-linux-gnu
RUN apt-get install -y apt-utils
RUN apt-get install -yf --no-install-recommends \
    sudo \
    build-essential \
    autoconf \
    automake \
    gdb \
    valgrind \
    ccache \
    locales \
    locales-all \
    chrpath \
    cifs-utils \
    clang-format \
    cmake \
    cmake-curses-gui \
    doxygen \
    git \
    icecc \
    icecc-monitor \
    # libapache2-mod-fastcgi \
    # libboost-all-dev \
    libconfig++-dev \
    libcppunit-dev \
    libcurl4-openssl-dev \
    libfcgi-dev  \
    libqt5opengl5-dev \
    libssl-dev \
    libtbb-dev \
    libtbb2 \
    # libwxgtk3.0-dev \
    ninja-build \
    python3-dev \
    python3-setuptools \    
    python3-pip \    
    software-properties-common \
    uuid-dev \
    libtbb-dev \
    libyaml-cpp-dev \
    liburiparser-dev \
    vim \
    curl

 RUN apt-get install -y wget
   
 RUN pip3 install --upgrade pip
 RUN pip3 install --upgrade pip setuptools
 RUN pip3 install pyparsing \
    Jinja2 \
    PyHamcrest \
    setuptools-rust \
    pyOpenSSL \
    python-dateutil \
    shapely \
    lxml \
    robot \
    robotframework \
    decorator \
    requests

RUN locale-gen en_US.UTF-8

ARG USERNAME=develop

RUN SNIPPET="export PROMPT_COMMAND='history -a' && export HISTFILE=/commandhistory/.bash_history" \
        && mkdir /commandhistory \
        && touch /commandhistory/.bash_history \
        && chown -R $USERNAME /commandhistory \
        && echo $SNIPPET >> "/home/$USERNAME/.bashrc"

RUN mkdir -p /home/$USERNAME/.vscode-server/extensions /home/$USERNAME/.vscode-server-insiders/extensions \
        && chown -R $USERNAME   /home/$USERNAME/.vscode-server  /home/$USERNAME/.vscode-server-insiders

# RUN mkdir -p /etc/ovi/trafficdb && chown -R $USERNAME /etc/ovi

# ARG FIX_PATH

# RUN chown -R $USERNAME ${FIX_PATH}/kraken_sdr_commandhistory

# *****************************************************
# Important: It is necessary to avoid the error: increase the allowed size of the usbfs buffer with the following command
# RUN sudo sh -c 'echo 0 > /sys/module/usbcore/parameters/usbfs_memory_mb'
# *****************************************************

RUN cd /home/$USERNAME
RUN wget https://github.com/llvm/llvm-project/releases/download/llvmorg-12.0.1/clang+llvm-12.0.1-x86_64-linux-gnu-ubuntu-16.04.tar.xz
RUN tar -xvf ./clang+llvm-12.0.1-x86_64-linux-gnu-ubuntu-16.04.tar.xz
RUN rm ./clang+llvm-12.0.1-x86_64-linux-gnu-ubuntu-16.04.tar.xz
RUN mv ./clang+llvm-12.0.1-x86_64-linux-gnu-ubuntu-/ clang_12.0.1
RUN mv clang_12.0.1 /usr/local
RUN echo 'export PATH=/usr/local/clang_12.0.1/bin:$PATH' >> /home/$USERNAME/.bashrc
RUN echo 'export LD_LIBRARY_PATH=/usr/local/clang_12.0.1/lib:$LD_LIBRARY_PATH' >> /home/$USERNAME/.bashrc

ENV LC_ALL en_US.UTF-8
ENV LANG en_US.UTF-8
ENV LANGUAGE en_US.UTF-8
