/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * _coder_wsprsim_mex.c
 *
 * Code generation for function '_coder_wsprsim_mex'
 *
 */

/* Include files */
#include "_coder_wsprsim_mex.h"
#include "_coder_wsprsim_api.h"
#include "rt_nonfinite.h"
#include "wsprsim_data.h"
#include "wsprsim_initialize.h"
#include "wsprsim_terminate.h"
#include "wsprsim_types.h"

/* Function Definitions */
void mexFunction(int32_T nlhs, mxArray *plhs[], int32_T nrhs, const mxArray
                 *prhs[])
{
  wsprsimStackData *wsprsimStackDataGlobal = NULL;
  wsprsimStackDataGlobal = (wsprsimStackData *)emlrtMxCalloc(1, (size_t)1U *
    sizeof(wsprsimStackData));
  mexAtExit(&wsprsim_atexit);

  /* Module initialization. */
  wsprsim_initialize();

  /* Dispatch the entry-point. */
  wsprsim_mexFunction(wsprsimStackDataGlobal, nlhs, plhs, nrhs, prhs);

  /* Module termination. */
  wsprsim_terminate();
  emlrtMxFree(wsprsimStackDataGlobal);
}

emlrtCTX mexFunctionCreateRootTLS(void)
{
  emlrtCreateRootTLS(&emlrtRootTLSGlobal, &emlrtContextGlobal, NULL, 1);
  return emlrtRootTLSGlobal;
}

void wsprsim_mexFunction(wsprsimStackData *SD, int32_T nlhs, mxArray *plhs[1],
  int32_T nrhs, const mxArray *prhs[2])
{
  emlrtStack st = { NULL,              /* site */
    NULL,                              /* tls */
    NULL                               /* prev */
  };

  const mxArray *outputs[1];
  st.tls = emlrtRootTLSGlobal;

  /* Check for proper number of arguments. */
  if (nrhs != 2) {
    emlrtErrMsgIdAndTxt(&st, "EMLRT:runTime:WrongNumberOfInputs", 5, 12, 2, 4, 7,
                        "wsprsim");
  }

  if (nlhs > 1) {
    emlrtErrMsgIdAndTxt(&st, "EMLRT:runTime:TooManyOutputArguments", 3, 4, 7,
                        "wsprsim");
  }

  /* Call the function. */
  wsprsim_api(SD, prhs, outputs);

  /* Copy over outputs to the caller. */
  emlrtReturnArrays(1, plhs, outputs);
}

/* End of code generation (_coder_wsprsim_mex.c) */
