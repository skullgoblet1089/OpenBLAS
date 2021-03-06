/*******************************************************************************
Copyright (c) 2016, The OpenBLAS Project
All rights reserved.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in
the documentation and/or other materials provided with the
distribution.
3. Neither the name of the OpenBLAS project nor the names of
its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE OPENBLAS PROJECT OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

#include "common.h"
#include "macros_msa.h"

#if !defined(CONJ)
    #define OP0  +=
    #define OP1  -=
    #define OP2  +=
#else
    #define OP0  -=
    #define OP1  +=
    #define OP2  -=
#endif

int CNAME(BLASLONG n, BLASLONG dummy0, BLASLONG dummy1, FLOAT da, FLOAT *x,
          BLASLONG inc_x, FLOAT *y, BLASLONG inc_y, FLOAT *dummy, BLASLONG dummy2)
{
    BLASLONG i;
    FLOAT *py;
    v4f32 x0, x1, x2, x3, x4, x5, x6, x7, y0, y1, y2, y3, y4, y5, y6, y7;
    v4f32 da_vec, zero_v = {0};

    if ((n < 0) || (da == 0.0))  return(0);

    py = y;

    if ((1 == inc_x) && (1 == inc_y))
    {
        FLOAT *x_pref, *y_pref;
        BLASLONG pref_offset;

        pref_offset = (BLASLONG)x & (L1_DATA_LINESIZE - 1);
        if (pref_offset > 0)
        {
            pref_offset = L1_DATA_LINESIZE - pref_offset;
            pref_offset = pref_offset / sizeof(FLOAT);
        }
        x_pref = x + pref_offset + 64;

        pref_offset = (BLASLONG)y & (L1_DATA_LINESIZE - 1);
        if (pref_offset > 0)
        {
            pref_offset = L1_DATA_LINESIZE - pref_offset;
            pref_offset = pref_offset / sizeof(FLOAT);
        }
        y_pref = y + pref_offset + 64;

        da_vec = COPY_FLOAT_TO_VECTOR(da);

        for (i = (n >> 5); i--;)
        {
            PREF_OFFSET(x_pref, 0);
            PREF_OFFSET(x_pref, 32);
            PREF_OFFSET(x_pref, 64);
            PREF_OFFSET(x_pref, 96);
            PREF_OFFSET(y_pref, 0);
            PREF_OFFSET(y_pref, 32);
            PREF_OFFSET(y_pref, 64);
            PREF_OFFSET(y_pref, 96);
            x_pref += 32;
            y_pref += 32;

            LD_SP8_INC(x, 4, x0, x1, x2, x3, x4, x5, x6, x7);
            LD_SP8_INC(py, 4, y0, y1, y2, y3, y4, y5, y6, y7);
            FMADD4(x0, x1, x2, x3, da_vec, y0, y1, y2, y3);
            FMADD4(x4, x5, x6, x7, da_vec, y4, y5, y6, y7);
            ST_SP8_INC(y0, y1, y2, y3, y4, y5, y6, y7, y, 4);
        }

        if (n & 31)
        {
            if (n & 16)
            {
                LD_SP4_INC(x, 4, x0, x1, x2, x3);
                LD_SP4_INC(py, 4, y0, y1, y2, y3);
                FMADD4(x0, x1, x2, x3, da_vec, y0, y1, y2, y3);
                ST_SP4_INC(y0, y1, y2, y3, y, 4);
            }

            if (n & 8)
            {
                LD_SP2_INC(x, 4, x0, x1);
                LD_SP2_INC(py, 4, y0, y1);
                FMADD2(x0, x1, da_vec, y0, y1);
                ST_SP2_INC(y0, y1, y, 4);
            }

            if (n & 4)
            {
                x0 = LD_SP(x); x += 4;
                y0 = LD_SP(py); py += 4;
                y0 += da_vec * x0;
                ST_SP(y0, y); y += 4;
            }

            if (n & 2)
            {
                FMADD2(x[0], x[1], da, y[0], y[1]);
                x += 2;
                y += 2;
            }

            if (n & 1)
            {
                y[0] += da * x[0];
            }
        }
    }
    else if (1 == inc_y)
    {
        da_vec = COPY_FLOAT_TO_VECTOR(da);

        for (i = (n >> 4); i--;)
        {
            x0 = (v4f32) __msa_insert_w((v4i32) zero_v, 0, *((int *) x));
            x += inc_x;
            x0 = (v4f32) __msa_insert_w((v4i32) x0, 1, *((int *) x));
            x += inc_x;
            x0 = (v4f32) __msa_insert_w((v4i32) x0, 2, *((int *) x));
            x += inc_x;
            x0 = (v4f32) __msa_insert_w((v4i32) x0, 3, *((int *) x));
            x += inc_x;
            x1 = (v4f32) __msa_insert_w((v4i32) zero_v, 0, *((int *) x));
            x += inc_x;
            x1 = (v4f32) __msa_insert_w((v4i32) x1, 1, *((int *) x));
            x += inc_x;
            x1 = (v4f32) __msa_insert_w((v4i32) x1, 2, *((int *) x));
            x += inc_x;
            x1 = (v4f32) __msa_insert_w((v4i32) x1, 3, *((int *) x));
            x += inc_x;
            x2 = (v4f32) __msa_insert_w((v4i32) zero_v, 0, *((int *) x));
            x += inc_x;
            x2 = (v4f32) __msa_insert_w((v4i32) x2, 1, *((int *) x));
            x += inc_x;
            x2 = (v4f32) __msa_insert_w((v4i32) x2, 2, *((int *) x));
            x += inc_x;
            x2 = (v4f32) __msa_insert_w((v4i32) x2, 3, *((int *) x));
            x += inc_x;
            x3 = (v4f32) __msa_insert_w((v4i32) zero_v, 0, *((int *) x));
            x += inc_x;
            x3 = (v4f32) __msa_insert_w((v4i32) x3, 1, *((int *) x));
            x += inc_x;
            x3 = (v4f32) __msa_insert_w((v4i32) x3, 2, *((int *) x));
            x += inc_x;
            x3 = (v4f32) __msa_insert_w((v4i32) x3, 3, *((int *) x));
            x += inc_x;

            LD_SP4_INC(py, 4, y0, y1, y2, y3);
            FMADD4(x0, x1, x2, x3, da_vec, y0, y1, y2, y3);
            ST_SP4_INC(y0, y1, y2, y3, y, 4);
        }

        if (n & 15)
        {
            if (n & 8)
            {
                x0 = (v4f32) __msa_insert_w((v4i32) zero_v, 0, *((int *) x));
                x += inc_x;
                x0 = (v4f32) __msa_insert_w((v4i32) x0, 1, *((int *) x));
                x += inc_x;
                x0 = (v4f32) __msa_insert_w((v4i32) x0, 2, *((int *) x));
                x += inc_x;
                x0 = (v4f32) __msa_insert_w((v4i32) x0, 3, *((int *) x));
                x += inc_x;
                x1 = (v4f32) __msa_insert_w((v4i32) zero_v, 0, *((int *) x));
                x += inc_x;
                x1 = (v4f32) __msa_insert_w((v4i32) x1, 1, *((int *) x));
                x += inc_x;
                x1 = (v4f32) __msa_insert_w((v4i32) x1, 2, *((int *) x));
                x += inc_x;
                x1 = (v4f32) __msa_insert_w((v4i32) x1, 3, *((int *) x));
                x += inc_x;

                LD_SP2_INC(py, 4, y0, y1);
                FMADD2(x0, x1, da_vec, y0, y1);
                ST_SP2_INC(y0, y1, y, 4);
            }

            if (n & 4)
            {
                x0 = (v4f32) __msa_insert_w((v4i32) zero_v, 0, *((int *) x));
                x += inc_x;
                x0 = (v4f32) __msa_insert_w((v4i32) x0, 1, *((int *) x));
                x += inc_x;
                x0 = (v4f32) __msa_insert_w((v4i32) x0, 2, *((int *) x));
                x += inc_x;
                x0 = (v4f32) __msa_insert_w((v4i32) x0, 3, *((int *) x));
                x += inc_x;

                y0 = LD_SP(py); py += 4;
                y0 += da_vec * x0;
                ST_SP(y0, y); y += 4;
            }

            if (n & 2)
            {
                FMADD2(x[0], x[inc_x], da, y[0], y[1]);

                x += 2 * inc_x;
                y += 2;
            }

            if (n & 1)
            {
                y[0] += da * x[0];
            }
        }
    }
    else
    {
        FLOAT x0, x1, x2, x3, y0, y1, y2, y3;

        for (i = (n >> 2); i--;)
        {
            LD_GP4_INC(x, inc_x, x0, x1, x2, x3);
            LD_GP4_INC(py, inc_y, y0, y1, y2, y3);
            FMADD4(x0, x1, x2, x3, da, y0, y1, y2, y3);
            ST_GP4_INC(y0, y1, y2, y3, y, inc_y);
        }

        if (n & 3)
        {
            if (n & 2)
            {
                LD_GP2_INC(x, inc_x, x0, x1);
                LD_GP2_INC(py, inc_y, y0, y1);
                FMADD2(x0, x1, da, y0, y1);
                ST_GP2_INC(y0, y1, y, inc_y);
            }

            if (n & 1)
            {
                *y += da * *x;
            }
        }
    }

    return (0);
}
