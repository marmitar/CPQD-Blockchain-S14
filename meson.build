project('hello-world-intel-sgx', 'c', 'cpp',
    version: '0.1.0',
    license: 'BSD-3-Clause',
    meson_version: '>= 1.4.0',
    subproject_dir: 'external',
)

# # # # # # # # # # # #
# SGX TOOLS AND LIBS  #

SGX_SDK = '/opt/intel/sgxsdk'
SGX_PATH = [SGX_SDK / 'bin', SGX_SDK / 'bin/x64']
SGX_INCLUDE = SGX_SDK / 'include'
SGX_PKGCONFIG = SGX_SDK / 'pkgconfig'
SGX_LDLIBRARY = SGX_SDK / 'sdk_libs'

sgx_edger8r = find_program('sgx_edger8r', dirs: SGX_PATH)
sgx_sign = find_program('sgx_sign', dirs: SGX_PATH)

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

# # # # # #
# TARGETS #

subdir('Enclave')
subdir('App')
