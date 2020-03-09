/* This file is the part of the STM32 secure bootloader
 *
 * GOST R 34.12-2015 "MAGMA" block cipher implementation based on
 * official GOST R 34.12-2015 national standard of the Russian Federation
 *
 * Copyright ©2016 Dmitry Filimonchuk <dmitrystu[at]gmail[dot]com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *   http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdint.h>
#include <string.h>
#include "misc.h"
#include "magma.h"

#define rounds 32

static uint32_t RK[32];

static const uint32_t S[] = {
    0xC6BC7581, 0x4838FDE7, 0x62525F2E, 0x2381A65D,
    0xA92D8960, 0x5AF41295, 0xB5AF6C18, 0x9CD6DAC3,
    0xE1E70BF4, 0x8E10974F, 0xD47A38BA, 0x7745E106,
    0x0BC3B4D9, 0x3D9E43AC, 0xF0692E3B, 0x1F0BC072,
    };

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
static uint32_t sbox(uint32_t in) {
    uint32_t out;
    for(int i = 0; i < 8; i++) {
        out <<= 4;
        out += (S[in >> 28] >> (i * 4)) & 0x0F;
        in <<= 4;
    }
    return out;
}
#pragma GCC diagnostic pop


static uint32_t F(uint32_t data, uint32_t round) {
    return __rol32(sbox(data + RK[round]), 11);
}

void magma_encrypt(uint32_t *out, const uint32_t *in) {
    uint32_t A = BE32TOCPU(in[1]);
    uint32_t B = BE32TOCPU(in[0]);
    for (int i = 0; i < rounds; i++) {
       uint32_t A1 = B ^ F(A, i);
       B = A;
       A = A1;
    }
    out[1] = CPUTOBE32(B);
    out[0] = CPUTOBE32(A);
}

void magma_decrypt(uint32_t *out, const uint32_t *in) {
    uint32_t A = BE32TOCPU(in[1]);
    uint32_t B = BE32TOCPU(in[0]);
    for (int i = 31; i >= 0; i--) {
        uint32_t A1 = B ^ F(A, i);
        B = A;
        A = A1;
    }
    out[1] = CPUTOBE32(B);
    out[0] = CPUTOBE32(A);
}

void magma_init(const void* key){
    for (int i = 0; i < 8; i++) {
        uint32_t K;
        memcpy(&K, key, sizeof(K));
        K = BE32TOCPU(K);
        RK[0x00 + i] = K;
        RK[0x08 + i] = K;
        RK[0x10 + i] = K;
        RK[0x1F - i] = K;
        key += 4;
    }
}
