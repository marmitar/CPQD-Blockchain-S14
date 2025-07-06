/*
 * Copyright (C) 2011-2021 Intel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Intel Corporation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

// clang-format off
#if __cplusplus < 202300L
#error "This code is compliant with C++23 or later only."
#endif
// clang-format on

#include <array>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sgx_defs.h>
#include <sgx_eid.h>
#include <sgx_error.h>
#include <sgx_urts.h>
#include <string>

#include "./App.h"
#include "Enclave_u.h"

/* Global EID shared by multiple threads */
sgx_enclave_id_t global_eid = 0;

using sgx_errlist_t = struct sgx_errlist_t {
    sgx_status_t err;
    const std::string msg;
    const std::optional<std::string> sug; /* Suggestion */
};

/** Error code returned by sgx_create_enclave */
static const std::array<sgx_errlist_t, 17> sgx_errlist {
    sgx_errlist_t {
                   .err = SGX_ERROR_UNEXPECTED,
                   .msg = "Unexpected error occurred.",
                   .sug = std::nullopt,
                   },
    sgx_errlist_t {
                   .err = SGX_ERROR_INVALID_PARAMETER,
                   .msg = "Invalid parameter.",
                   .sug = std::nullopt,
                   },
    sgx_errlist_t {
                   .err = SGX_ERROR_OUT_OF_MEMORY,
                   .msg = "Out of memory.",
                   .sug = std::nullopt,
                   },
    sgx_errlist_t {
                   .err = SGX_ERROR_ENCLAVE_LOST,
                   .msg = "Power transition occurred.",
                   .sug = "Please refer to the sample \"PowerTransition\" for details.",
                   },
    sgx_errlist_t {
                   .err = SGX_ERROR_INVALID_ENCLAVE,
                   .msg = "Invalid enclave image.",
                   .sug = std::nullopt,
                   },
    sgx_errlist_t {
                   .err = SGX_ERROR_INVALID_ENCLAVE_ID,
                   .msg = "Invalid enclave identification.",
                   .sug = std::nullopt,
                   },
    sgx_errlist_t {
                   .err = SGX_ERROR_INVALID_SIGNATURE,
                   .msg = "Invalid enclave signature.",
                   .sug = std::nullopt,
                   },
    sgx_errlist_t {
                   .err = SGX_ERROR_OUT_OF_EPC,
                   .msg = "Out of EPC memory.",
                   .sug = std::nullopt,
                   },
    sgx_errlist_t {
                   .err = SGX_ERROR_NO_DEVICE,
                   .msg = "Invalid SGX device.",
                   .sug = "Please make sure SGX module is enabled in the BIOS, and install SGX driver afterwards.",
                   },
    sgx_errlist_t {
                   .err = SGX_ERROR_MEMORY_MAP_CONFLICT,
                   .msg = "Memory map conflicted.",
                   .sug = std::nullopt,
                   },
    sgx_errlist_t {
                   .err = SGX_ERROR_INVALID_METADATA,
                   .msg = "Invalid enclave metadata.",
                   .sug = std::nullopt,
                   },
    sgx_errlist_t {
                   .err = SGX_ERROR_DEVICE_BUSY,
                   .msg = "SGX device was busy.",
                   .sug = std::nullopt,
                   },
    sgx_errlist_t {
                   .err = SGX_ERROR_INVALID_VERSION,
                   .msg = "Enclave version was invalid.",
                   .sug = std::nullopt,
                   },
    sgx_errlist_t {
                   .err = SGX_ERROR_INVALID_ATTRIBUTE,
                   .msg = "Enclave was not authorized.",
                   .sug = std::nullopt,
                   },
    sgx_errlist_t {
                   .err = SGX_ERROR_ENCLAVE_FILE_ACCESS,
                   .msg = "Can't open enclave file.",
                   .sug = std::nullopt,
                   },
    sgx_errlist_t {
                   .err = SGX_ERROR_NDEBUG_ENCLAVE,
                   .msg = "The enclave is signed as product enclave, and can not be created as debuggable enclave.",
                   .sug = std::nullopt,
                   },
    sgx_errlist_t {
                   .err = SGX_ERROR_MEMORY_MAP_FAILURE,
                   .msg = "Failed to reserve memory for the enclave.",
                   .sug = std::nullopt,
                   },
};

[[gnu::const, nodiscard("pure function")]]
/** Map error code to message. */
static auto error_message(sgx_status_t ret) -> sgx_errlist_t {
    for (const auto &idx : sgx_errlist) {
        if (ret == idx.err) {
            return idx;
        }
    }
    return sgx_errlist_t(ret, "Unknown error occurred.", std::nullopt);
}

/** Check error conditions for loading enclave */
static void print_error_message(sgx_status_t ret) {
    const sgx_errlist_t err = error_message(ret);

    if (err.sug.has_value()) {
        std::cout << "Info: " << err.sug.value() << std::endl;
    }
    std::cout << "Error: " << err.msg << " (0x" << std::hex << std::setw(4) << std::setfill('0') << err.err << ")"
              << std::endl;
}

/* OCall functions */
extern void ocall_print_string(const char *str) {
    /* Proxy/Bridge will check the length and null-terminate
     * the input string to prevent buffer overflow.
     */
    std::cout << (str != nullptr ? str : "<null>");
}

/* Application entry */
auto SGX_CDECL main() -> int {
    /* Initialize the enclave */
    /* Debug Support: set 2nd parameter to 1 */
    sgx_status_t status = sgx_create_enclave(ENCLAVE_FILENAME, SGX_DEBUG_FLAG, nullptr, nullptr, &global_eid, nullptr);
    if (status != SGX_SUCCESS) {
        print_error_message(status);
        return EXIT_FAILURE;
    }

    bool ok = true;
    /* Utilize trusted libraries */
    status = ecall_libcxx_functions();
    if (status != SGX_SUCCESS) {
        print_error_message(status);
        ok = false;
    }

    /* Destroy the enclave */
    status = sgx_destroy_enclave(global_eid);
    if (status != SGX_SUCCESS) {
        print_error_message(status);
        return EXIT_FAILURE;
    }

    std::cout << "Info: Cxx11DemoEnclave successfully returned." << std::endl;
    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
