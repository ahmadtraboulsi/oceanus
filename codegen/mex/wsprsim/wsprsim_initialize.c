/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * wsprsim_initialize.c
 *
 * Code generation for function 'wsprsim_initialize'
 *
 */

/* Include files */
#include "wsprsim_initialize.h"
#include "_coder_wsprsim_mex.h"
#include "rt_nonfinite.h"
#include "wsprsim_data.h"

/* Variable Definitions */
static const volatile char_T *emlrtBreakCheckR2012bFlagVar = NULL;

/* Function Declarations */
static void wsprsim_once(void);

/* Function Definitions */
static void wsprsim_once(void)
{
  mex_InitInfAndNan();

  /* Allocate instance data */
  covrtAllocateInstanceData(&emlrtCoverageInstance);

  /* Initialize Coverage Information */
  covrtScriptInit(&emlrtCoverageInstance,
                  "D:\\PHD\\DRDC2020\\Oceanus\\oceanus\\wsprsim.m", 0U, 1U, 1U,
                  0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U);

  /* Initialize Function Information */
  covrtFcnInit(&emlrtCoverageInstance, 0U, 0U, "wsprsim", 0, -1, 850);

  /* Initialize Basic Block Information */
  covrtBasicBlockInit(&emlrtCoverageInstance, 0U, 0U, 40, -1, 844);

  /* Initialize If Information */
  /* Initialize MCDC Information */
  /* Initialize For Information */
  /* Initialize While Information */
  /* Initialize Switch Information */
  /* Start callback for coverage engine */
  covrtScriptStart(&emlrtCoverageInstance, 0U);
}

void wsprsim_initialize(void)
{
  emlrtStack st = { NULL,              /* site */
    NULL,                              /* tls */
    NULL                               /* prev */
  };

  mexFunctionCreateRootTLS();
  emlrtBreakCheckR2012bFlagVar = emlrtGetBreakCheckFlagAddressR2012b();
  st.tls = emlrtRootTLSGlobal;
  emlrtClearAllocCountR2012b(&st, false, 0U, 0);
  emlrtEnterRtStackR2012b(&st);
  if (emlrtFirstTimeR2012b(emlrtRootTLSGlobal)) {
    wsprsim_once();
  }
}

/* End of code generation (wsprsim_initialize.c) */
