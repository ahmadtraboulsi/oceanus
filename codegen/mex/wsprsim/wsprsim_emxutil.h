/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * wsprsim_emxutil.h
 *
 * Code generation for function 'wsprsim_emxutil'
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
void emxEnsureCapacity_char_T(const emlrtStack *sp, emxArray_char_T *emxArray,
  int32_T oldNumel, const emlrtRTEInfo *srcLocation);
void emxFree_char_T(emxArray_char_T **pEmxArray);
void emxInit_char_T(const emlrtStack *sp, emxArray_char_T **pEmxArray, int32_T
                    numDimensions, const emlrtRTEInfo *srcLocation, boolean_T
                    doPush);

/* End of code generation (wsprsim_emxutil.h) */
