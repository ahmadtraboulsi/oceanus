/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * _coder_wsprsim_api.c
 *
 * Code generation for function '_coder_wsprsim_api'
 *
 */

/* Include files */
#include "_coder_wsprsim_api.h"
#include "rt_nonfinite.h"
#include "wsprsim1.h"
#include "wsprsim_data.h"
#include "wsprsim_emxutil.h"
#include "wsprsim_types.h"

/* Variable Definitions */
static emlrtRTEInfo c_emlrtRTEI = { 1, /* lineNo */
  1,                                   /* colNo */
  "_coder_wsprsim_api",                /* fName */
  ""                                   /* pName */
};

/* Function Declarations */
static void b_emlrt_marshallIn(const emlrtStack *sp, const mxArray *u, const
  emlrtMsgIdentifier *parentId, emxArray_char_T *y);
static real_T c_emlrt_marshallIn(const emlrtStack *sp, const mxArray *snr, const
  char_T *identifier);
static real_T d_emlrt_marshallIn(const emlrtStack *sp, const mxArray *u, const
  emlrtMsgIdentifier *parentId);
static void e_emlrt_marshallIn(const emlrtStack *sp, const mxArray *src, const
  emlrtMsgIdentifier *msgId, emxArray_char_T *ret);
static void emlrt_marshallIn(const emlrtStack *sp, const mxArray *message, const
  char_T *identifier, emxArray_char_T *y);
static const mxArray *emlrt_marshallOut(const real_T u[45000]);
static real_T f_emlrt_marshallIn(const emlrtStack *sp, const mxArray *src, const
  emlrtMsgIdentifier *msgId);

/* Function Definitions */
static void b_emlrt_marshallIn(const emlrtStack *sp, const mxArray *u, const
  emlrtMsgIdentifier *parentId, emxArray_char_T *y)
{
  e_emlrt_marshallIn(sp, emlrtAlias(u), parentId, y);
  emlrtDestroyArray(&u);
}

static real_T c_emlrt_marshallIn(const emlrtStack *sp, const mxArray *snr, const
  char_T *identifier)
{
  emlrtMsgIdentifier thisId;
  real_T y;
  thisId.fIdentifier = (const char_T *)identifier;
  thisId.fParent = NULL;
  thisId.bParentIsCell = false;
  y = d_emlrt_marshallIn(sp, emlrtAlias(snr), &thisId);
  emlrtDestroyArray(&snr);
  return y;
}

static real_T d_emlrt_marshallIn(const emlrtStack *sp, const mxArray *u, const
  emlrtMsgIdentifier *parentId)
{
  real_T y;
  y = f_emlrt_marshallIn(sp, emlrtAlias(u), parentId);
  emlrtDestroyArray(&u);
  return y;
}

static void e_emlrt_marshallIn(const emlrtStack *sp, const mxArray *src, const
  emlrtMsgIdentifier *msgId, emxArray_char_T *ret)
{
  static const int32_T dims[2] = { 1, -1 };

  int32_T iv[2];
  int32_T i;
  const boolean_T bv[2] = { false, true };

  emlrtCheckVsBuiltInR2012b(sp, msgId, src, "char", false, 2U, dims, &bv[0], iv);
  i = ret->size[0] * ret->size[1];
  ret->size[0] = iv[0];
  ret->size[1] = iv[1];
  emxEnsureCapacity_char_T(sp, ret, i, (emlrtRTEInfo *)NULL);
  emlrtImportArrayR2015b(sp, src, ret->data, 1, false);
  emlrtDestroyArray(&src);
}

static void emlrt_marshallIn(const emlrtStack *sp, const mxArray *message, const
  char_T *identifier, emxArray_char_T *y)
{
  emlrtMsgIdentifier thisId;
  thisId.fIdentifier = (const char_T *)identifier;
  thisId.fParent = NULL;
  thisId.bParentIsCell = false;
  b_emlrt_marshallIn(sp, emlrtAlias(message), &thisId, y);
  emlrtDestroyArray(&message);
}

static const mxArray *emlrt_marshallOut(const real_T u[45000])
{
  static const int32_T iv[2] = { 0, 0 };

  static const int32_T iv1[2] = { 1, 45000 };

  const mxArray *m;
  const mxArray *y;
  y = NULL;
  m = emlrtCreateNumericArray(2, &iv[0], mxDOUBLE_CLASS, mxREAL);
  emlrtMxSetData((mxArray *)m, (void *)&u[0]);
  emlrtSetDimensions((mxArray *)m, iv1, 2);
  emlrtAssign(&y, m);
  return y;
}

static real_T f_emlrt_marshallIn(const emlrtStack *sp, const mxArray *src, const
  emlrtMsgIdentifier *msgId)
{
  static const int32_T dims = 0;
  real_T ret;
  emlrtCheckBuiltInR2012b(sp, msgId, src, "double", false, 0U, &dims);
  ret = *(real_T *)emlrtMxGetData(src);
  emlrtDestroyArray(&src);
  return ret;
}

void wsprsim_api(wsprsimStackData *SD, const mxArray * const prhs[2], const
                 mxArray *plhs[1])
{
  emlrtStack st = { NULL,              /* site */
    NULL,                              /* tls */
    NULL                               /* prev */
  };

  emxArray_char_T *message;
  real_T (*insig)[45000];
  real_T snr;
  st.tls = emlrtRootTLSGlobal;
  insig = (real_T (*)[45000])mxMalloc(sizeof(real_T [45000]));
  emlrtHeapReferenceStackEnterFcnR2012b(&st);
  emxInit_char_T(&st, &message, 2, &c_emlrtRTEI, true);

  /* Marshall function inputs */
  emlrt_marshallIn(&st, emlrtAliasP(prhs[0]), "message", message);
  snr = c_emlrt_marshallIn(&st, emlrtAliasP(prhs[1]), "snr");

  /* Invoke the target function */
  wsprsim(SD, &st, message, snr, *insig);

  /* Marshall function outputs */
  plhs[0] = emlrt_marshallOut(*insig);
  emxFree_char_T(&message);
  emlrtHeapReferenceStackLeaveFcnR2012b(&st);
}

/* End of code generation (_coder_wsprsim_api.c) */
