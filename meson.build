project('hello-world-intel-sgx', 'c', 'cpp',
    version: '0.1.0',
    license: 'BSD-3-Clause',
    meson_version: '>= 1.4.0',
    subproject_dir: 'external',
)

# Carrega as ferramentas e variaveis de ambiente do SGX-SDK
SGX_SDK = '/opt/intel/sgxsdk'

################
# CONFIGURAÇÃO #
################
CC = meson.get_compiler('c')
SGX_COMMON_FLAGS = [
    '-m64', '-O2',
    '-Wall', '-Wextra', '-Winit-self', '-Wpointer-arith', '-Wreturn-type',
    '-Waddress', '-Wsequence-point', '-Wformat-security',
    '-Wmissing-include-dirs', '-Wfloat-equal', '-Wundef', '-Wshadow',
    '-Wcast-align', '-Wcast-qual', '-Wconversion', '-Wredundant-decls',
]
SGX_COMMON_CFLAGS = SGX_COMMON_FLAGS + [
    '-Wjump-misses-init', '-Wstrict-prototypes', '-Wunsuffixed-float-constants'
]
SGX_COMMON_CXXFLAGS = SGX_COMMON_FLAGS + [
    '-Wnon-virtual-dtor',
    '-std=c++11',
]
APP_FLAGS = [
    '-fPIC', '-Wno-attributes', '-IApp', '-I/opt/intel/sgxsdk/include', '-DNDEBUG', '-UEDEBUG', '-UDEBUG',
]
ENCLAVE_INCLUDES = [
    '-IEnclave',
    f'-I@SGX_SDK@/include',
    f'-I@SGX_SDK@/include/libcxx',
    f'-I@SGX_SDK@/include/tlibc',
]
ENCLAVE_FLAGS = [
    '-nostdinc',
    '-fvisibility=hidden',
    '-fpie',
    '-fstack-protector',
    '-fno-builtin-printf',
] + ENCLAVE_INCLUDES + [
    '-nostdinc++',
]
ENCLAVE_LINK_FLAGS = [
    '-Wl,-z,relro,-z,now,-z,noexecstack',
    '-Wl,--no-undefined',
    '-nostdlib', '-nodefaultlibs', '-nostartfiles', f'-L@SGX_SDK@/lib64',
    '-Wl,--whole-archive',
    '-lsgx_trts_sim',
    '-Wl,--no-whole-archive',
    '-Wl,--start-group',
    '-lsgx_tstdc -lsgx_tcxx -lsgx_tcrypto -lsgx_trts_sim',
    '-Wl,--end-group',
    '-Wl,-Bstatic',
    '-Wl,-Bsymbolic',
    '-Wl,--no-undefined',
    '-Wl,-pie,-eenclave_entry',
    '-Wl,--export-dynamic',
    '-Wl,--defsym,__ImageBase=0',
    '-Wl,--version-script=@0@/Enclave/Enclave_debug.lds'.format(meson.project_source_root()),
]
# artefas gerados durante compilação.
GENERATED_FILES = [
    meson.project_source_root() / '.config_*',
    meson.project_source_root() / 'app',
    meson.project_source_root() / 'enclave.so',
    meson.project_source_root() / 'enclave.signed.so',
    meson.project_source_root() / 'App/App.o',
    meson.project_source_root() / 'App/TrustedLibrary/Libcxx.o',
    meson.project_source_root() / 'App/Enclave_u.*',
    meson.project_source_root() / 'Enclave/Enclave.o',
    meson.project_source_root() / 'Enclave/TrustedLibrary/Libcxx.o',
    meson.project_source_root() / 'Enclave/Enclave_t.*',
]

################
# COMPILAR APP #
################

# PASSO 1: Remover arquivos gerados automaticamente.
run_target('clean-enclave',
    command: ['rm', '-f'] + GENERATED_FILES,
)

# PASSO 2: Gerar interfaces entre Untrusted Components e Enclaves.
custom_target('untrusted-enclave',
  command: ['sgx_edger8r', '--untrusted', '@INPUT@', '--search-path', meson.project_source_root() / 'Enclave', '--search-path', f'@SGX_SDK@/include'],
  input: files('Enclave/Enclave.edl'),
  output: ['Enclave_u.c', 'Enclave_u.h'],
  build_by_default: true,
)

# PASSO 3: Compilar APP.
executable('app',
    files('App/App.cpp', 'App/TrustedLibrary/Libcxx.cpp')
        + [meson.project_build_root() / 'Enclave_u.c'],
    c_args: SGX_COMMON_CFLAGS + APP_FLAGS,
    cpp_args: SGX_COMMON_CXXFLAGS + APP_FLAGS,
    link_args: ['-L/opt/intel/sgxsdk/lib64', '-lsgx_urts_sim', '-lpthread'],
)

####################
# COMPILAR ENCLAVE #
####################
# PASSO 4: Gerar interfaces entre Trusted Components e Enclaves.
custom_target('trusted-enclave',
  command: ['/opt/intel/sgxsdk/bin/x64/sgx_edger8r', '--trusted', '@INPUT@', '--search-path', meson.project_source_root() / 'Enclave', '--search-path', f'@SGX_SDK@/include'],
  input: files('Enclave/Enclave.edl'),
  output: ['Enclave_t.c', 'Enclave_t.h'],
  build_by_default: true,
)

# PASSO 5: Compilar ENCLAVE
enclave = shared_library('enclave',
    files('Enclave/Enclave.cpp', 'Enclave/TrustedLibrary/Libcxx.cpp') +
        [meson.project_build_root() / 'Enclave_t.c'],
    c_args: ['-m64', '-O2', '-Wall', '-Wextra', '-Winit-self', '-Wpointer-arith', '-Wreturn-type', '-Waddress', '-Wsequence-point', '-Wformat-security', '-Wmissing-include-dirs', '-Wfloat-equal', '-Wundef', '-Wshadow', '-Wcast-align', '-Wcast-qual', '-Wconversion', '-Wredundant-decls', '-Wjump-misses-init', '-Wstrict-prototypes', '-Wunsuffixed-float-constants', '-nostdinc', '-fvisibility=hidden', '-fpie', '-fstack-protector', '-fno-builtin-printf']
        + ENCLAVE_INCLUDES,
    cpp_args: SGX_COMMON_CXXFLAGS + ENCLAVE_FLAGS,
    link_args: ENCLAVE_LINK_FLAGS ,
    name_prefix: '',
)

# PASSO 6: Assinar ENCLAVE
custom_target('sign',
    command: [
        '/opt/intel/sgxsdk/bin/x64/sgx_sign', 'sign',
        '-key', meson.project_source_root() / 'Enclave/Enclave_private_test.pem',
        '-enclave', '@INPUT@',
        '-out', '@OUTPUT@',
        '-config', meson.project_source_root() / 'Enclave/Enclave.config.xml',
    ],
    input: enclave,
    output: 'enclave.signed.so',
)
