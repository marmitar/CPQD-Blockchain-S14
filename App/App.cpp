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

#include <cassert>
#include <cstdio>
#include <cstring>
#include <sgx_defs.h>
#include <sgx_eid.h>
#include <sgx_error.h>
#include <sgx_urts.h>

#define MAX_PATH FILENAME_MAX

#include "./App.h"
#include "Enclave_u.h"

/* Global EID shared by multiple threads */
sgx_enclave_id_t global_eid = 0;

using sgx_errlist_t = struct sgx_errlist_t {
    sgx_status_t err;
    const char *msg;
    const char *sug; /* Suggestion */
};

/* Error code returned by sgx_create_enclave */
static const sgx_errlist_t sgx_errlist[] /* NOLINT(modernize-avoid-c-arrays) */ = {
    {
     .err = SGX_ERROR_UNEXPECTED,
     .msg = "Unexpected error occurred.",
     .sug = NULL,
     },
    {
     .err = SGX_ERROR_INVALID_PARAMETER,
     .msg = "Invalid parameter.",
     .sug = NULL,
     },
    {
     .err = SGX_ERROR_OUT_OF_MEMORY,
     .msg = "Out of memory.",
     .sug = NULL,
     },
    {
     .err = SGX_ERROR_ENCLAVE_LOST,
     .msg = "Power transition occurred.",
     .sug = "Please refer to the sample \"PowerTransition\" for details.",
     },
    {
     .err = SGX_ERROR_INVALID_ENCLAVE,
     .msg = "Invalid enclave image.",
     .sug = NULL,
     },
    {
     .err = SGX_ERROR_INVALID_ENCLAVE_ID,
     .msg = "Invalid enclave identification.",
     .sug = NULL,
     },
    {
     .err = SGX_ERROR_INVALID_SIGNATURE,
     .msg = "Invalid enclave signature.",
     .sug = NULL,
     },
    {
     .err = SGX_ERROR_OUT_OF_EPC,
     .msg = "Out of EPC memory.",
     .sug = NULL,
     },
    {
     .err = SGX_ERROR_NO_DEVICE,
     .msg = "Invalid SGX device.",
     .sug = "Please make sure SGX module is enabled in the BIOS, and install SGX driver afterwards.",
     },
    {
     .err = SGX_ERROR_MEMORY_MAP_CONFLICT,
     .msg = "Memory map conflicted.",
     .sug = NULL,
     },
    {
     .err = SGX_ERROR_INVALID_METADATA,
     .msg = "Invalid enclave metadata.",
     .sug = NULL,
     },
    {
     .err = SGX_ERROR_DEVICE_BUSY,
     .msg = "SGX device was busy.",
     .sug = NULL,
     },
    {
     .err = SGX_ERROR_INVALID_VERSION,
     .msg = "Enclave version was invalid.",
     .sug = NULL,
     },
    {
     .err = SGX_ERROR_INVALID_ATTRIBUTE,
     .msg = "Enclave was not authorized.",
     .sug = NULL,
     },
    {
     .err = SGX_ERROR_ENCLAVE_FILE_ACCESS,
     .msg = "Can't open enclave file.",
     .sug = NULL,
     },
    {
     .err = SGX_ERROR_NDEBUG_ENCLAVE,
     .msg = "The enclave is signed as product enclave, and can not be created as debuggable enclave.",
     .sug = NULL,
     },
    {
     .err = SGX_ERROR_MEMORY_MAP_FAILURE,
     .msg = "Failed to reserve memory for the enclave.",
     .sug = NULL,
     },
};

/* Check error conditions for loading enclave */
static void print_error_message(sgx_status_t ret) {
    size_t idx = 0;
    const size_t ttl = sizeof sgx_errlist / sizeof sgx_errlist[0];

    for (idx = 0; idx < ttl; idx++) {
        if (ret == sgx_errlist[idx].err) {
            if (NULL != sgx_errlist[idx].sug) {
                printf("Info: %s\n", sgx_errlist[idx].sug);
            }
            printf("Error: %s\n", sgx_errlist[idx].msg);
            break;
        }
    }

    if (idx == ttl) {
        printf("Error: Unexpected error occurred.\n");
    }
}

/* Initialize the enclave:
 *   Call sgx_create_enclave to initialize an enclave instance
 */
static auto initialize_enclave() -> int {
    sgx_status_t ret = SGX_ERROR_UNEXPECTED;

    /* Call sgx_create_enclave to initialize an enclave instance */
    /* Debug Support: set 2nd parameter to 1 */
    ret = sgx_create_enclave(ENCLAVE_FILENAME, SGX_DEBUG_FLAG, NULL, NULL, &global_eid, NULL);
    if (ret != SGX_SUCCESS) {
        print_error_message(ret);
        return -1;
    }

    return 0;
}

/* OCall functions */
void ocall_print_string(const char *str) {
    /* Proxy/Bridge will check the length and null-terminate
     * the input string to prevent buffer overflow.
     */
    printf("%s", str);
}

/* Application entry */
auto SGX_CDECL main(int argc, char *argv[]) -> int {
    (void) (argc);
    (void) (argv);

    /* Initialize the enclave */
    if (initialize_enclave() < 0) {
        printf("Enter a character before exit ...\n");
        (void) getchar();
        return -1;
    }

    /* Utilize trusted libraries */
    ecall_libcxx_functions();

    /* Destroy the enclave */
    sgx_destroy_enclave(global_eid);

    printf("Info: Cxx11DemoEnclave successfully returned.\n");

    // printf("Enter a character before exit ...\n");
    // getchar();
    return 0;
}
