FROM mcr.microsoft.com/oss/mirror/docker.io/library/ubuntu:20.04
WORKDIR /app

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get -y install wget build-essential \
    swig cmake git \
    libboost-filesystem-dev libboost-test-dev libboost-serialization-dev libboost-regex-dev libboost-serialization-dev libboost-regex-dev libboost-thread-dev libboost-system-dev \
    libnuma-dev libnuma1

ENV PYTHONPATH=/app/Release

COPY CMakeLists.txt ./
COPY AnnService ./AnnService/
COPY Test ./Test/
COPY Wrappers ./Wrappers/
COPY GPUSupport ./GPUSupport/

COPY ThirdParty ./ThirdParty/



# Build Intel ISA-L Crypto library
RUN apt-get update && apt-get -y install dos2unix autoconf automake libtool nasm yasm
RUN find ThirdParty/isal-l_crypto -type f -exec dos2unix {} \;
RUN cd ThirdParty/isal-l_crypto && ./autogen.sh && ./configure && make

# Build and install RocksDB
RUN apt-get update && apt-get -y install libjemalloc-dev libsnappy-dev libgflags-dev pkg-config libtbb-dev libisal-dev gcc-9 g++-9 \
    && git clone https://github.com/PtilopsisL/rocksdb.git /tmp/rocksdb \
    && cd /tmp/rocksdb \
    && mkdir build && cd build \
    && cmake -DUSE_RTTI=1 -DWITH_JEMALLOC=1 -DWITH_SNAPPY=1 -DCMAKE_C_COMPILER=gcc-9 -DCMAKE_CXX_COMPILER=g++-9 -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-fPIC" .. \
    && make -j$(nproc) \
    && make install \
    && cd /app \
    && rm -rf /tmp/rocksdb

RUN apt-get update && apt-get -y install dos2unix libaio-dev uuid-dev libssl-dev python3 python3-pip \
    && find ThirdParty/spdk -type f -exec dos2unix {} \; \
    && find ThirdParty/spdk -name "*.sh" -exec chmod +x {} \; \
    && find ThirdParty/spdk/scripts -type f -exec chmod +x {} \; \
    && ls -l ThirdParty/spdk/scripts/pkgdep/ \
    && sed -i 's|source "$(dirname "$0")/debian.sh"|source /app/ThirdParty/spdk/scripts/pkgdep/debian.sh|' ThirdParty/spdk/scripts/pkgdep/ubuntu.sh \
    && cd ThirdParty/spdk \
    && ./scripts/pkgdep.sh \
    && CC=gcc-9 ./configure \
    && CC=gcc-9 make -j$(nproc)

# Build SPFresh
RUN mkdir build && cd build && cmake .. && make -j$(nproc) && cd ..
