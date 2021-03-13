/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * wsprsim1.c
 *
 * Code generation for function 'wsprsim1'
 *
 */

/* Include files */
#include "wsprsim1.h"
#include "rt_nonfinite.h"
#include "wsprsim_data.h"
#include "wsprsim_emxutil.h"
#include "wsprsim_types.h"
#include "fano.h"
#include "math.h"
#include "nhash.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "wsprd_utils.h"
#include "wsprsim.h"
#include "wsprsim_utils.h"

/* Variable Definitions */
static emlrtRTEInfo emlrtRTEI = { 21,  /* lineNo */
  28,                                  /* colNo */
  "wsprsim",                           /* fName */
  "D:\\PHD\\DRDC2020\\Oceanus\\oceanus\\wsprsim.m"/* pName */
};

static emlrtRTEInfo b_emlrtRTEI = { 1, /* lineNo */
  18,                                  /* colNo */
  "wsprsim",                           /* fName */
  "D:\\PHD\\DRDC2020\\Oceanus\\oceanus\\wsprsim.m"/* pName */
};

/* Function Definitions */
void wsprsim(wsprsimStackData *SD, const emlrtStack *sp, const emxArray_char_T
             *message, real_T snr, real_T insig[45000])
{
  emxArray_char_T *b_message;
  int32_T i;
  int32_T loop_ub;
  emlrtHeapReferenceStackEnterFcnR2012b(sp);
  emxInit_char_T(sp, &b_message, 2, &b_emlrtRTEI, true);
  covrtLogFcn(&emlrtCoverageInstance, 0U, 0U);
  covrtLogBasicBlock(&emlrtCoverageInstance, 0U, 0U);

  /* isig=0.05; */
  i = b_message->size[0] * b_message->size[1];
  b_message->size[0] = 1;
  b_message->size[1] = message->size[1];
  emxEnsureCapacity_char_T(sp, b_message, i, &emlrtRTEI);
  loop_ub = message->size[0] * message->size[1];
  for (i = 0; i < loop_ub; i++) {
    b_message->data[i] = message->data[i];
  }

  wsprfunc(&b_message->data[0], snr, &insig[0], 45000.0, &SD->f0.qdsig[0],
           45000.0);
  emxFree_char_T(&b_message);
  emlrtHeapReferenceStackLeaveFcnR2012b(sp);
}

/* End of code generation (wsprsim1.c) */
/* The gateway function */
void mexFunction(int nlhs, mxArray *plhs[],
	int nrhs, const mxArray *prhs[])
{
	/* variable declarations here */

	/* code here */
}