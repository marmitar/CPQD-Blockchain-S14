#!/usr/bin/env bash
set -e

# Make sure we are in the same folder
cd "$(dirname "$0")"

# Verifica se as variaveis de ambiente foram setadas
if [ -z "${SGX_SDK}" ]; then
    # Carrega as ferramentas e variaveis de ambiente do SGX-SDK
    source /opt/intel/sgxsdk/environment
fi

################
# CONFIGURAÇÃO #
################
CC=/usr/bin/gcc
declare -a SGX_COMMON_FLAGS=(
    -m64 -O2
    -Wall -Wextra -Winit-self -Wpointer-arith -Wreturn-type
    -Waddress -Wsequence-point -Wformat-security
    -Wmissing-include-dirs -Wfloat-equal -Wundef -Wshadow
    -Wcast-align -Wcast-qual -Wconversion -Wredundant-decls
)
declare -a SGX_COMMON_CFLAGS=(
    "${SGX_COMMON_FLAGS[@]}"
    -Wjump-misses-init -Wstrict-prototypes -Wunsuffixed-float-constants
)
declare -a SGX_COMMON_CXXFLAGS=(
    "${SGX_COMMON_FLAGS[@]}"
    '-Wnon-virtual-dtor'
    '-std=c++11'
)
declare -a APP_FLAGS=(
    -fPIC -Wno-attributes -IApp -I/opt/intel/sgxsdk/include -DNDEBUG -UEDEBUG -UDEBUG
)
declare -a ENCLAVE_INCLUDES=(
    -IEnclave
    "-I${SGX_SDK}/include"
    "-I${SGX_SDK}/include/libcxx"
    "-I${SGX_SDK}/include/tlibc"
)
declare -a ENCLAVE_FLAGS=(
    -nostdinc
    '-fvisibility=hidden'
    -fpie
    -fstack-protector
    -fno-builtin-printf
    "${ENCLAVE_INCLUDES[@]}"
    -nostdinc++
)
declare -a ENCLAVE_LINK_FLAGS=(
    -Wl,-z,relro,-z,now,-z,noexecstack
    -Wl,--no-undefined
    -nostdlib -nodefaultlibs -nostartfiles "-L${SGX_SDK}/lib64"
	'-Wl,--whole-archive'
    -lsgx_trts_sim
    '-Wl,--no-whole-archive'
	'-Wl,--start-group'
    -lsgx_tstdc -lsgx_tcxx -lsgx_tcrypto -lsgx_trts_sim
    '-Wl,--end-group'
	'-Wl,-Bstatic'
    '-Wl,-Bsymbolic'
    '-Wl,--no-undefined'
	'-Wl,-pie,-eenclave_entry'
    '-Wl,--export-dynamic'
	'-Wl,--defsym,__ImageBase=0'
	'-Wl,--version-script=Enclave/Enclave_debug.lds'
)
# artefas gerados durante compilação.
declare -a GENERATED_FILES=(
    '.config_*'
    'app'
    'enclave.so'
    'enclave.signed.so'
    'App/App.o'
    'App/TrustedLibrary/Libcxx.o'
    'App/Enclave_u.*'
    'Enclave/Enclave.o'
    'Enclave/TrustedLibrary/Libcxx.o'
    'Enclave/Enclave_t.*'
)

################
# COMPILAR APP #
################

# PASSO 1: Remover arquivos gerados automaticamente.
rm -f ${GENERATED_FILES[@]}
 
# PASSO 2: Gerar interfaces entre Untrusted Components e Enclaves.
pushd App
sgx_edger8r --untrusted ../Enclave/Enclave.edl --search-path ../Enclave --search-path "${SGX_SDK}/include"
popd

# PASSO 3: Compilar APP.
$CC ${SGX_COMMON_CFLAGS[@]} ${APP_FLAGS[@]} -c App/Enclave_u.c -o App/Enclave_u.o
g++ ${SGX_COMMON_CXXFLAGS[@]} ${APP_FLAGS[@]} -c App/App.cpp -o App/App.o
g++ ${SGX_COMMON_CXXFLAGS[@]} ${APP_FLAGS[@]} -c App/TrustedLibrary/Libcxx.cpp -o App/TrustedLibrary/Libcxx.o
g++ App/Enclave_u.o App/App.o App/TrustedLibrary/Libcxx.o -o app -L/opt/intel/sgxsdk/lib64 -lsgx_urts_sim -lpthread

####################
# COMPILAR ENCLAVE #
####################
# PASSO 4: Gerar interfaces entre Trusted Components e Enclaves.
pushd Enclave
/opt/intel/sgxsdk/bin/x64/sgx_edger8r --trusted ../Enclave/Enclave.edl --search-path ../Enclave --search-path "${SGX_SDK}/include"
popd

# PASSO 5: Compilar ENCLAVE
$CC -m64 -O2 -Wall -Wextra -Winit-self -Wpointer-arith -Wreturn-type -Waddress -Wsequence-point -Wformat-security -Wmissing-include-dirs -Wfloat-equal -Wundef -Wshadow -Wcast-align -Wcast-qual -Wconversion -Wredundant-decls -Wjump-misses-init -Wstrict-prototypes -Wunsuffixed-float-constants -nostdinc -fvisibility=hidden -fpie -fstack-protector -fno-builtin-printf ${ENCLAVE_INCLUDES[@]}  -c Enclave/Enclave_t.c -o Enclave/Enclave_t.o
g++ ${SGX_COMMON_CXXFLAGS[@]} ${ENCLAVE_FLAGS[@]} -c Enclave/Enclave.cpp -o Enclave/Enclave.o
g++ ${SGX_COMMON_CXXFLAGS[@]} ${ENCLAVE_FLAGS[@]} -c Enclave/TrustedLibrary/Libcxx.cpp -o Enclave/TrustedLibrary/Libcxx.o
g++ Enclave/Enclave_t.o Enclave/Enclave.o Enclave/TrustedLibrary/Libcxx.o -o enclave.so ${ENCLAVE_LINK_FLAGS[@]}

# PASSO 6: Assinar ENCLAVE
/opt/intel/sgxsdk/bin/x64/sgx_sign sign \
    -key Enclave/Enclave_private_test.pem \
    -enclave enclave.so \
    -out enclave.signed.so \
    -config Enclave/Enclave.config.xml
