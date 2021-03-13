/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * wsprsim1.h
 *
 * Code generation for function 'wsprsim1'
 *
 */

#pragma once

/* Include files */
#include "rtwtypes.h"
#include "wsprsim_types.h"
#include "covrt.h"
#include "emlrt.h"
#include "mex.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Function Declarations */
void wsprsim(wsprsimStackData *SD, const emlrtStack *sp, const emxArray_char_T
             *message, real_T snr, real_T insig[45000]);

/* End of code generation (wsprsim1.h) */
