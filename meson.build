project('hello-world-intel-sgx', 'c', 'cpp',
    version: '0.1.0',
    license: 'BSD-3-Clause',
    meson_version: '>= 1.4',
    subproject_dir: 'external',
    default_options: {
        'c_std': 'c23',
        'cpp_std': 'c++23',
        'buildtype': 'release',
        'warning_level': '2',
        'default_library': 'static',
        'prefer_static': true,
        'b_asneeded': true,
        'b_lundef': true,
        'b_lto': true,
        'b_lto_threads': 0,
        'b_lto_mode': 'default',
        'b_ndebug': 'if-release',
        'b_pie': true,
        'b_staticpic': true,
    },
)

warning_level = get_option('warning_level') == 'everything' ? 5 : get_option('warning_level').to_int()
optimizations_enabled = get_option('optimization') in ['2', '3', 's', 'z']
debugging_enabled = get_option('debug')

cc = meson.get_compiler('c')
cxx = meson.get_compiler('cpp')

# # # # # # # # # #
# COMPILER FLAGS  #

warnings = []
c_warnings = []
cpp_warnings = []
# Safety warnings
if warning_level >= 1
    warnings += cc.get_supported_arguments(
        '-Wconversion',
        '-Wsign-conversion',
        '-Wformat',
        '-Wformat=2',
        '-Wformat-signedness',
        '-Winit-self',
        '-Wpointer-arith',
        '-Wreturn-type',
        '-Waddress',
        '-Wsequence-point',
        '-Wformat-security',
        '-Wmissing-include-dirs',
        '-Wfloat-equal',
        '-Wundef',
        '-Wshadow',
        '-Wcast-align',
        '-Wcast-qual',
        '-Wconversion',
        '-Wredundant-decls',
        cc.get_supported_arguments('-Wformat-truncation=2').get(0, '-Wformat-truncation'),
        cc.get_supported_arguments('-Wformat-overflow=2').get(0, '-Wformat-overflow'),
        checked: 'warn',
    ) + cc.get_supported_arguments(
        '-Wstringop-overflow=4', # GCC only
        '-Wno-nullability-extension', # Clang only
        checked: 'off',
    )

    c_warnings += cc.get_supported_arguments(
        '-Wstrict-prototypes',
        checked: 'warn',
    ) + cc.get_supported_arguments(
        '-Wjump-misses-init', # GCC only
        checked: 'off',
    )

    cpp_warnings += cxx.get_supported_arguments(
        '-Wnon-virtual-dtor',
        checked: 'warn',
    )
endif
# Extra warnings
if warning_level >= 2
    warnings += cc.get_supported_arguments(
        '-Wunused-result',
        checked: 'warn',
    )
endif
# Pedantic warnings
if warning_level >= 3
    warnings += cc.get_supported_arguments(
        optimizations_enabled ? '-Winline' : [],
        checked: 'warn',
    ) + cc.get_supported_arguments(
        optimizations_enabled ? '-Rpass=.*' : [], # Clang only
        optimizations_enabled ? '-Rpass-missed=.*' : [], # Clang only
        checked: 'off',
    )
endif
# Disable some bugprone GCC warnings
if cc.get_id() == 'gcc'
    warnings += cc.get_supported_arguments(
        '-Wno-abi',
        '-Wno-nonnull-compare',
        '-Wno-unused-function',
        checked: 'warn',
    )
endif

optimizations = []
if optimizations_enabled
    optimizations += cc.get_supported_arguments(
        not meson.is_cross_build() ? '-march=native' : [],
        not meson.is_cross_build() ? '-mtune=native' : [],
        '-fdata-sections',
        '-ffunction-sections',
        '-fno-exceptions',
        '-fno-math-errno',
        '-fno-trapping-math',
        checked: 'warn',
    ) + cc.get_supported_arguments(
        get_option('b_lto_mode') == 'default' ? '-flto=full' : [], # Clang only
        '-fno-signaling-nans', # GCC only
        '-fallow-store-data-races', # GCC only
        '-fwhole-program', # GCC only
        checked: 'off',
    )
endif

general_codegen = cc.get_supported_arguments(
    '-pipe',
    '-fvisibility=hidden',
    debugging_enabled ? '-fstrict-overflow' : '-fno-strict-overflow',
    # _FORTIFY_SOURCE=2+ breaks static analysis on some stdlib functions for Clang
    '-D_FORTIFY_SOURCE=@0@'.format(debugging_enabled and cc.get_id() == 'gcc' ? 3 : 1),
    checked: 'warn',
)

debugging = []
if debugging_enabled
    debugging += cc.get_supported_arguments(
        '-DDEBUG',
        '-ggdb',
        '-g3',
        '-fstack-clash-protection',
        '-fcf-protection',
        '-ftrapv',
        '-fno-omit-frame-pointer',
        '-mno-omit-leaf-frame-pointer',
        checked: 'warn',
    )
endif

add_global_arguments(
    warnings,
    optimizations,
    general_codegen,
    debugging,
    language: ['c', 'cpp'],
)

add_global_arguments(c_warnings, language: 'c')
add_global_arguments(cpp_warnings, language: 'cpp')

linker_options = cc.get_supported_link_arguments(
    optimizations_enabled ? '-Wl,-O1' : [],
    '-Wl,--gc-sections',
    '-Wl,--sort-common',
    '-Wl,--as-needed',
    '-Wl,-z,relro',
    '-Wl,-z,now',
    '-Wl,-z,noexecstack',
)

add_global_link_arguments(
    linker_options,
    language: ['c', 'cpp'],
)

# # # # # #
# TARGETS #

subdir('Enclave')
subdir('App')

# # # # #
# TESTS #

test('example',
    app,
    args: [signed_enclave],
    env: {
        'LD_LIBRARY_PATH': SGX_LIBDIR,
    },
    suite: ['example'],
)
