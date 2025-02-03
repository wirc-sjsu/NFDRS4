#!/bin/bash
# Exit immediately if a command exits with a non-zero status
set -e

# Define color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[1;34m'
NC='\033[0m' # No Color

# Determine the directory of this script and set the installation prefix relative to it
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
INSTALL_PREFIX="$SCRIPT_DIR/../extern/spatial"

# Create installation directories
mkdir -p "$INSTALL_PREFIX"

# Set download directory
DOWNLOAD_DIR="$INSTALL_PREFIX/downloads"
mkdir -p "$DOWNLOAD_DIR"

# Clear any existing modules (adjust based on your environment)
module purge

# Load required modules
module load nvhpc-hpcx-cuda12/24.9
module load cmake/3.24.2

# Set compiler variables
export CC=nvc
export CXX=nvc++
export FC=nvfortran

# Versions of the libraries
ZLIB_VERSION="1.2.11"
HDF5_VERSION="1.12.0"
NETCDF_C_VERSION="4.7.4"
NETCDF_CXX_VERSION="4.3.1"

# Function to download source code
download_sources() {
    echo -e "${BLUE}Preparing source code...${NC}"
    cd "$DOWNLOAD_DIR"

    # zlib
    if [ ! -f "zlib-$ZLIB_VERSION.tar.gz" ]; then
        echo -e "${GREEN}Downloading zlib...${NC}"
        wget -c "https://zlib.net/fossils/zlib-$ZLIB_VERSION.tar.gz"
    fi

    # HDF5
    if [ ! -f "hdf5-$HDF5_VERSION.tar.gz" ]; then
        echo -e "${GREEN}Downloading HDF5...${NC}"
        wget -c "https://support.hdfgroup.org/ftp/HDF5/releases/hdf5-1.12/hdf5-$HDF5_VERSION/src/hdf5-$HDF5_VERSION.tar.gz"
    fi

    # NetCDF-C
    if [ ! -f "netcdf-c-$NETCDF_C_VERSION.tar.gz" ]; then
        echo -e "${GREEN}Downloading NetCDF-C...${NC}"
        wget -c "https://github.com/Unidata/netcdf-c/archive/refs/tags/v$NETCDF_C_VERSION.tar.gz"
        mv "v$NETCDF_C_VERSION.tar.gz" "netcdf-c-$NETCDF_C_VERSION.tar.gz"
    fi

    # NetCDF-CXX
    if [ ! -f "netcdf-cxx4-$NETCDF_CXX_VERSION.tar.gz" ]; then
        echo -e "${GREEN}Downloading NetCDF-CXX...${NC}"
        wget -c "https://github.com/Unidata/netcdf-cxx4/archive/refs/tags/v$NETCDF_CXX_VERSION.tar.gz"
        mv "v$NETCDF_CXX_VERSION.tar.gz" "netcdf-cxx4-$NETCDF_CXX_VERSION.tar.gz"
    fi
}

# Function to install zlib
install_zlib() {
    echo -e "${BLUE}Installing zlib...${NC}"
    cd "$DOWNLOAD_DIR"
    tar -xzf "zlib-$ZLIB_VERSION.tar.gz"
    cd "zlib-$ZLIB_VERSION"

    # Set compiler flags for position-independent code
    CFLAGS="-fPIC" ./configure --prefix="$INSTALL_PREFIX"

    make -j$(nproc)
    make install
    cd "$DOWNLOAD_DIR"
    echo -e "${GREEN}zlib installed successfully.${NC}"
}

# Function to install HDF5
install_hdf5() {
    echo -e "${BLUE}Installing HDF5...${NC}"
    cd "$DOWNLOAD_DIR"
    tar -xzf "hdf5-$HDF5_VERSION.tar.gz"
    cd "hdf5-$HDF5_VERSION"

    export LD_LIBRARY_PATH="$INSTALL_PREFIX/lib:$LD_LIBRARY_PATH"

    # Set compiler flags for position-independent code
    CFLAGS="-fPIC" CXXFLAGS="-fPIC" FCFLAGS="-fPIC" ./configure --prefix="$INSTALL_PREFIX" \
        --with-zlib="$INSTALL_PREFIX" --enable-hl --disable-fortran --enable-cxx

    make -j$(nproc)
    make install
    cd "$DOWNLOAD_DIR"
    echo -e "${GREEN}HDF5 installed successfully.${NC}"
}

# Function to install NetCDF-C
install_netcdf_c() {
    echo -e "${BLUE}Installing NetCDF-C...${NC}"
    cd "$DOWNLOAD_DIR"
    tar -xzf "netcdf-c-$NETCDF_C_VERSION.tar.gz"
    cd "netcdf-c-$NETCDF_C_VERSION"

    export CPPFLAGS="-I$INSTALL_PREFIX/include"
    export LDFLAGS="-L$INSTALL_PREFIX/lib"
    export LD_LIBRARY_PATH="$INSTALL_PREFIX/lib:$LD_LIBRARY_PATH"

    # Set compiler flags for position-independent code
    CFLAGS="-fPIC" ./configure --prefix="$INSTALL_PREFIX" --disable-dap --enable-netcdf-4 --enable-shared

    make -j$(nproc)
    make install
    cd "$DOWNLOAD_DIR"
    echo -e "${GREEN}NetCDF-C installed successfully.${NC}"
}

# Function to install NetCDF-CXX
install_netcdf_cxx() {
    echo -e "${BLUE}Installing NetCDF-CXX...${NC}"
    cd "$DOWNLOAD_DIR"
    tar -xzf "netcdf-cxx4-$NETCDF_CXX_VERSION.tar.gz"
    cd "netcdf-cxx4-$NETCDF_CXX_VERSION"

    export CPPFLAGS="-I$INSTALL_PREFIX/include"
    export LDFLAGS="-L$INSTALL_PREFIX/lib"
    export LD_LIBRARY_PATH="$INSTALL_PREFIX/lib:$LD_LIBRARY_PATH"
    export LIBS="-lnetcdf -lhdf5_hl -lhdf5 -lz"

    # Set compiler flags for position-independent code
    CXXFLAGS="-fPIC" ./configure --prefix="$INSTALL_PREFIX"

    make -j$(nproc)
    make install
    cd "$DOWNLOAD_DIR"
    echo -e "${GREEN}NetCDF-CXX installed successfully.${NC}"
}

# Function to update environment variables using relative paths
update_environment_variables() {
    echo -e "${YELLOW}Updating environment variables...${NC}"
    # Determine the shell configuration file to update
    SHELL_CONFIG_FILE="$HOME/.bashrc"
    if [ "$SHELL" = "/bin/zsh" ]; then
        SHELL_CONFIG_FILE="$HOME/.zshrc"
    fi

    # Backup the shell configuration file
    cp "$SHELL_CONFIG_FILE" "${SHELL_CONFIG_FILE}.bak"

    # Append LD_LIBRARY_PATH export (using the relative INSTALL_PREFIX variable)
    echo "export LD_LIBRARY_PATH=\"$INSTALL_PREFIX/lib:\$LD_LIBRARY_PATH\"" >> "$SHELL_CONFIG_FILE"
    export LD_LIBRARY_PATH="$INSTALL_PREFIX/lib:$LD_LIBRARY_PATH"

    # Append PATH export
    echo "export PATH=\"\$HOME/.local/bin:\$PATH:$INSTALL_PREFIX/bin\"" >> "$SHELL_CONFIG_FILE"
    export PATH="$HOME/.local/bin:$PATH:$INSTALL_PREFIX/bin"

    echo -e "${GREEN}Environment variables updated.${NC}"
    echo -e "${YELLOW}Please restart your terminal or run 'source $SHELL_CONFIG_FILE' to apply the changes.${NC}"
}

# Main function to orchestrate the installation
main() {
    download_sources
    install_zlib
    install_hdf5
    install_netcdf_c
    install_netcdf_cxx
    update_environment_variables
    echo -e "${GREEN}All libraries installed successfully in a relative directory structure.${NC}"
}

# Run the main function
main
