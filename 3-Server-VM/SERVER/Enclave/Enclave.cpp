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

#include "Enclave_t.h"
#include <sgx_trts.h>
#include <string.h>

int entry1 = 1;
int entry2 = 1;
int entry3 = 0;
int entry4 = 0;
int entry5 = 1;

int ecall_test(const char *message)
{
    ocall_print(message);
    return 31337;
}

int ecall_print_char(const char *message, size_t message_len)
{

    ocall_print(message);
    return 31337;
}

void ecall_makeEntry(char *message, char response[80], size_t message_len)
{

    int num;
    ocall_TransformInt(&num, message);
    // strncpy(response, "aaaaaaaaaaaa", 80);

    switch (num)
    {
    case 1:
        strncpy(response, "poached egg made", 80);
        // ocall_print(response);
        break;
    case 2:
        strncpy(response, "Cucumber made", 80);
        // ocall_print(message);

        break;
    case 3:
        strncpy(response, "Taro and chicken mince made", 80);
        // ocall_print(response);
        break;
    case 4:
        strncpy(response, "Tofu rissoles made", 80);
        // ocall_print(response);
        break;
    case 5:
        strncpy(response, "Miso soup made", 80);
        // ocall_print(response);
        break;
    default:
        strncpy(response, "failed", 80);
        // ocall_print(response);
    }
}

int ecall_entryverif(int entry, int fd, size_t message_len)
{
    int rep;
    ocall_readRequest(&rep, entry, fd, message_len);
    
    
    switch (rep)
    {
    case 1:
        if (!entry1 == 0)
            return 1;
        break;
    case 2:
        if (!entry2 == 0)
            return 1;
        break;
    case 3:
        if (!entry3 == 0)
            return 1;
        break;
    case 4:
        if (!entry4 == 0)
            return 1;
        break;
    case 5:
        if (!entry5 == 0)
            return 1;
        break;
    default:
        return 2;
        break;
        }
    return 2;
}