#!/bin/bash
set -e

# constants
typeset -r s_command_name=$(basename "$0")
typeset -r s_synopsis=\
"Usage: ${s_command_name} [-v] [-b build_name] [-t build_type] [-m Y|N]"
typeset -r s_date_time=$(date +"%Y%m%d_%H%M%S")

# variables
typeset -i i_verbose=0
typeset    s_build_name="_${s_date_time}"
typeset -i i_interactive_build_name=1
typeset    s_build_type="RelWithDebugInfo"
typeset -i i_interactive_build_type=1
typeset -i i_documentation=0
typeset    s_install_mongo_db_server="Y"
typeset -i i_interactive_install_mongo_db_server=1

typeset    a_make_parameters=()
typeset -r s_os_distribution=$(lsb_release --short --id)
typeset -r s_os_release=$(lsb_release --short --release)

# get options
while getopts ":vb:t:m:" s_option;
do
    case $s_option in
    v) i_verbose=1; ;;
    b) s_build_name="$OPTARG"; i_interactive_build_name=0; ;;
    t) s_build_type="$OPTARG"; i_interactive_build_type=0; ;;
    m) s_install_mongo_db_server="$OPTARG"; i_interactive_install_mongo_db_server=0; ;;
    \?)
        echo "Error: Illegal option <$OPTARG> specified!"
        echo -e "$s_synopsis"; exit 1;
        ;;
    :)
        echo "Error: Argument is missing for option <$OPTARG>!"
        echo -e "$s_synopsis"; exit 1;
        ;;
    esac
done
shift $(( OPTIND - 1 )) 2>/dev/null

# check parameter count
if (( $# != 0 )); then
    echo "Error: Wrong parameter count $#!"
    echo -e "$s_synopsis"; exit 1;
fi

# check distribution and release
if [[ "$s_os_distribution" != "Ubuntu" || "$s_os_release" != "18.04" ]]; then
    echo "Error: $s_os_distribution $s_os_release is not supported!"
    exit 2
fi

# interactive process parameters
if (( i_interactive_build_name )); then
    read -r -p "Build Name [${s_build_name}]: " s_input
    s_build_name=${s_input:-$s_build_name}
fi
if (( i_interactive_build_type )); then
    read -r -p "Build Type [Debug|Release|${s_build_type}]: " s_input
    s_build_type=${s_input:-$s_build_type}
fi
if (( i_interactive_install_mongo_db_server )); then
    read -r -p "Install MongoDB Server (optional) [Y|N]? [${s_install_mongo_db_server}]: " s_input
    s_install_mongo_db_server=${s_input:-$s_install_mongo_db_server}
fi

# configure
if (( i_verbose )); then
    a_make_parameters+=("VERBOSE=1")
fi

# install common C++ development dependencies
sudo apt install --assume-yes git g++ make cmake pkg-config
sudo apt install --assume-yes python3

# install Boost C++ Libraries
sudo apt install --assume-yes libboost1.65-all-dev		# Ubuntu 18.04: v1.65

# install Google Test (GTest)
sudo apt install --assume-yes googletest			# Ubuntu 18.04: v1.8.0; Source Code for Google Test and Google Mock
pushd . >/dev/null
cd /usr/src/googletest
sudo cmake CMakeLists.txt
sudo make
sudo make install
popd >/dev/null

### install RocksDB ###
# install gflags
sudo apt-get install --assume-yes libgflags-dev

# install compression libraries: snappy zlib bzip2 lz4 zstandard
sudo apt-get install --assume-yes libsnappy-dev zlib1g-dev libbz2-dev liblz4-dev libzstd-dev

# clone RocksDB repository
pushd . >/dev/null
git clone https://github.com/facebook/rocksdb.git --branch 5.12.fb --depth 1
cd rocksdb
if [[ "$s_build_type" == "Release" ]]; then
    # compile and install librocksdb.a and librocksdb.so, RocksDB static/shared library in release mode
    make static_lib -j4
    make shared_lib -j4
    sudo make install
else
    # compile and install all with debug level 1 (default)
    make all
    sudo make install

    # install required tools
    sudo cp ldb rocksdb_* sst_dump /usr/local/bin/
fi
popd >/dev/null

### install Mongo DB stuff ###
# install Mongo DB Server (optional)
if [[ "$i_interactive_install_mongo_db_server" == "Y" || "$i_interactive_install_mongo_db_server" == "y" ]]; then
    sudo apt install --assume-yes mongodb-server mongodb-clients mongo-tools
fi

# install MongoDB C Driver (libmongoc) and BSON library (libbson)
sudo apt install --assume-yes cmake libssl-dev libsasl2-dev
if (( i_documentation )); then
    sudo apt install --assume-yes python-sphinx
fi
pushd . >/dev/null
git clone https://github.com/mongodb/mongo-c-driver.git
if [[ ! -d "mongo-c-driver/cmake-build" ]]; then
     mkdir "mongo-c-driver/cmake-build"
fi
cd mongo-c-driver
git checkout 1.11.0						# build release mongo-c-driver 1.11.0: libbson 1.11.0, libmongoc 1.11.0
cd cmake-build
if (( i_documentation )); then
    cmake -DCMAKE_BUILD_TYPE="${s_build_type}" -DENABLE_AUTOMATIC_INIT_AND_CLEANUP=OFF -DENABLE_MAN_PAGES=ON -DENABLE_HTML_DOCS=ON ..
else
    cmake -DCMAKE_BUILD_TYPE="${s_build_type}" -DENABLE_AUTOMATIC_INIT_AND_CLEANUP=OFF ..
fi
make
sudo make install
popd >/dev/null

# install MongoDB C++ Driver (mongocxx)
pushd . >/dev/null
git clone https://github.com/mongodb/mongo-cxx-driver.git --branch releases/v3.3 --depth 1
cd mongo-cxx-driver/build
# cmake needs -DBSONCXX_POLY_USE_BOOST=1
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local -DBSONCXX_POLY_USE_BOOST=1 ..
# MNMLSTC polyfill (sudo make EP_mnmlstc_core) is not used when using -DBSONCXX_POLY_USE_BOOST=1
make
make test
sudo make install
popd >/dev/null

### install ZeroMQ stuff ###
# install ZeroMQ Library (libzmq)
sudo apt install --assume-yes libzmq3-dev libzmq5		# Ubuntu 18.04: v4.2.5, v4.2.5; depends on libzmq5 v4.2.5 (stable 4.2.3); really 5 vs 3; zmq.hpp from cppzmq does use zmq.h from libzmq3-dev

# install ZeroMQ C++ Bindings (cppzmq)
pushd . >/dev/null
git clone https://github.com/zeromq/cppzmq.git cppzmq.git
# fix ENABLE_DRAFTS ON vs. OFF; stable packages don't have the DRAFT API included, BUT we are installing from git
# we need to remove the cppzmq.git/.git directory, otherwise CMake will set the ENABLE_DRAFTS option to ON and then the tests will fail to link
rm -rf cppzmq.git/.git
if [[ ! -d "cppzmq.git/_build${s_build_name}" ]]; then
    mkdir "cppzmq.git/_build${s_build_name}"
fi
cd "cppzmq.git/_build${s_build_name}"
cmake ..
make -j4
make test
sudo make install
popd >/dev/null

### install Catapult Server ###
# build Catapult Server
pushd . >/dev/null
git clone https://github.com/nemtech/catapult-server.git
if [[ ! -d "catapult-server/_build${s_build_name}" ]]; then
    mkdir "catapult-server/_build${s_build_name}"
fi
cd "catapult-server/_build${s_build_name}"
cmake -DCMAKE_BUILD_TYPE="${s_build_type}" ..
make publish
make "${a_make_parameters[@]}" -j4
make test
popd >/dev/null

exit 0
