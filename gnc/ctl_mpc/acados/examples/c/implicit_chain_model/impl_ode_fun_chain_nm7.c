/* This file was automatically generated by CasADi.
   The CasADi copyright holders make no ownership claim of its contents. */
#ifdef __cplusplus
extern "C" {
#endif

/* How to prefix internal symbols */
#ifdef CODEGEN_PREFIX
  #define NAMESPACE_CONCAT(NS, ID) _NAMESPACE_CONCAT(NS, ID)
  #define _NAMESPACE_CONCAT(NS, ID) NS ## ID
  #define CASADI_PREFIX(ID) NAMESPACE_CONCAT(CODEGEN_PREFIX, ID)
#else
  #define CASADI_PREFIX(ID) impl_ode_fun_chain_nm7_ ## ID
#endif

#include <math.h>

#ifndef casadi_real
#define casadi_real double
#endif

#ifndef casadi_int
#define casadi_int int
#endif

/* Add prefix to internal symbols */
#define casadi_f0 CASADI_PREFIX(f0)
#define casadi_s0 CASADI_PREFIX(s0)
#define casadi_s1 CASADI_PREFIX(s1)
#define casadi_sq CASADI_PREFIX(sq)

/* Symbol visibility in DLLs */
#ifndef CASADI_SYMBOL_EXPORT
  #if defined(_WIN32) || defined(__WIN32__) || defined(__CYGWIN__)
    #if defined(STATIC_LINKED)
      #define CASADI_SYMBOL_EXPORT
    #else
      #define CASADI_SYMBOL_EXPORT __declspec(dllexport)
    #endif
  #elif defined(__GNUC__) && defined(GCC_HASCLASSVISIBILITY)
    #define CASADI_SYMBOL_EXPORT __attribute__ ((visibility ("default")))
  #else
    #define CASADI_SYMBOL_EXPORT
  #endif
#endif

static const casadi_int casadi_s0[40] = {36, 1, 0, 36, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35};
static const casadi_int casadi_s1[7] = {3, 1, 0, 3, 0, 1, 2};

casadi_real casadi_sq(casadi_real x) { return x*x;}

/* casadi_impl_ode_fun_chain_nm7:(i0[36],i1[36],i2[3])->(o0[36]) */
static int casadi_f0(const casadi_real** arg, casadi_real** res, casadi_int* iw, casadi_real* w, void* mem) {
  casadi_real a0, a1, a10, a11, a12, a13, a14, a15, a2, a3, a4, a5, a6, a7, a8, a9;
  a0=arg[1] ? arg[1][0] : 0;
  a1=arg[0] ? arg[0][3] : 0;
  a0=(a0-a1);
  if (res[0]!=0) res[0][0]=a0;
  a0=arg[1] ? arg[1][1] : 0;
  a1=arg[0] ? arg[0][4] : 0;
  a0=(a0-a1);
  if (res[0]!=0) res[0][1]=a0;
  a0=arg[1] ? arg[1][2] : 0;
  a1=arg[0] ? arg[0][5] : 0;
  a0=(a0-a1);
  if (res[0]!=0) res[0][2]=a0;
  a0=arg[1] ? arg[1][3] : 0;
  a1=3.3333333333333336e+01;
  a2=1.;
  a3=3.3000000000000002e-02;
  a4=arg[0] ? arg[0][6] : 0;
  a5=arg[0] ? arg[0][0] : 0;
  a6=(a4-a5);
  a7=casadi_sq(a6);
  a8=arg[0] ? arg[0][7] : 0;
  a9=arg[0] ? arg[0][1] : 0;
  a10=(a8-a9);
  a11=casadi_sq(a10);
  a7=(a7+a11);
  a11=arg[0] ? arg[0][8] : 0;
  a12=arg[0] ? arg[0][2] : 0;
  a13=(a11-a12);
  a14=casadi_sq(a13);
  a7=(a7+a14);
  a7=sqrt(a7);
  a7=(a3/a7);
  a7=(a2-a7);
  a6=(a7*a6);
  a14=casadi_sq(a5);
  a15=casadi_sq(a9);
  a14=(a14+a15);
  a15=casadi_sq(a12);
  a14=(a14+a15);
  a14=sqrt(a14);
  a14=(a3/a14);
  a14=(a2-a14);
  a5=(a14*a5);
  a5=(a6-a5);
  a5=(a1*a5);
  a0=(a0-a5);
  if (res[0]!=0) res[0][3]=a0;
  a0=arg[1] ? arg[1][4] : 0;
  a10=(a7*a10);
  a9=(a14*a9);
  a9=(a10-a9);
  a9=(a1*a9);
  a0=(a0-a9);
  if (res[0]!=0) res[0][4]=a0;
  a0=arg[1] ? arg[1][5] : 0;
  a7=(a7*a13);
  a14=(a14*a12);
  a14=(a7-a14);
  a14=(a1*a14);
  a12=9.8100000000000005e+00;
  a14=(a14-a12);
  a0=(a0-a14);
  if (res[0]!=0) res[0][5]=a0;
  a0=arg[1] ? arg[1][6] : 0;
  a14=arg[0] ? arg[0][9] : 0;
  a0=(a0-a14);
  if (res[0]!=0) res[0][6]=a0;
  a0=arg[1] ? arg[1][7] : 0;
  a14=arg[0] ? arg[0][10] : 0;
  a0=(a0-a14);
  if (res[0]!=0) res[0][7]=a0;
  a0=arg[1] ? arg[1][8] : 0;
  a14=arg[0] ? arg[0][11] : 0;
  a0=(a0-a14);
  if (res[0]!=0) res[0][8]=a0;
  a0=arg[1] ? arg[1][9] : 0;
  a14=arg[0] ? arg[0][12] : 0;
  a4=(a14-a4);
  a13=casadi_sq(a4);
  a9=arg[0] ? arg[0][13] : 0;
  a8=(a9-a8);
  a5=casadi_sq(a8);
  a13=(a13+a5);
  a5=arg[0] ? arg[0][14] : 0;
  a11=(a5-a11);
  a15=casadi_sq(a11);
  a13=(a13+a15);
  a13=sqrt(a13);
  a13=(a3/a13);
  a13=(a2-a13);
  a4=(a13*a4);
  a6=(a4-a6);
  a6=(a1*a6);
  a0=(a0-a6);
  if (res[0]!=0) res[0][9]=a0;
  a0=arg[1] ? arg[1][10] : 0;
  a8=(a13*a8);
  a10=(a8-a10);
  a10=(a1*a10);
  a0=(a0-a10);
  if (res[0]!=0) res[0][10]=a0;
  a0=arg[1] ? arg[1][11] : 0;
  a13=(a13*a11);
  a7=(a13-a7);
  a7=(a1*a7);
  a7=(a7-a12);
  a0=(a0-a7);
  if (res[0]!=0) res[0][11]=a0;
  a0=arg[1] ? arg[1][12] : 0;
  a7=arg[0] ? arg[0][15] : 0;
  a0=(a0-a7);
  if (res[0]!=0) res[0][12]=a0;
  a0=arg[1] ? arg[1][13] : 0;
  a7=arg[0] ? arg[0][16] : 0;
  a0=(a0-a7);
  if (res[0]!=0) res[0][13]=a0;
  a0=arg[1] ? arg[1][14] : 0;
  a7=arg[0] ? arg[0][17] : 0;
  a0=(a0-a7);
  if (res[0]!=0) res[0][14]=a0;
  a0=arg[1] ? arg[1][15] : 0;
  a7=arg[0] ? arg[0][18] : 0;
  a14=(a7-a14);
  a11=casadi_sq(a14);
  a10=arg[0] ? arg[0][19] : 0;
  a9=(a10-a9);
  a6=casadi_sq(a9);
  a11=(a11+a6);
  a6=arg[0] ? arg[0][20] : 0;
  a5=(a6-a5);
  a15=casadi_sq(a5);
  a11=(a11+a15);
  a11=sqrt(a11);
  a11=(a3/a11);
  a11=(a2-a11);
  a14=(a11*a14);
  a4=(a14-a4);
  a4=(a1*a4);
  a0=(a0-a4);
  if (res[0]!=0) res[0][15]=a0;
  a0=arg[1] ? arg[1][16] : 0;
  a9=(a11*a9);
  a8=(a9-a8);
  a8=(a1*a8);
  a0=(a0-a8);
  if (res[0]!=0) res[0][16]=a0;
  a0=arg[1] ? arg[1][17] : 0;
  a11=(a11*a5);
  a13=(a11-a13);
  a13=(a1*a13);
  a13=(a13-a12);
  a0=(a0-a13);
  if (res[0]!=0) res[0][17]=a0;
  a0=arg[1] ? arg[1][18] : 0;
  a13=arg[0] ? arg[0][21] : 0;
  a0=(a0-a13);
  if (res[0]!=0) res[0][18]=a0;
  a0=arg[1] ? arg[1][19] : 0;
  a13=arg[0] ? arg[0][22] : 0;
  a0=(a0-a13);
  if (res[0]!=0) res[0][19]=a0;
  a0=arg[1] ? arg[1][20] : 0;
  a13=arg[0] ? arg[0][23] : 0;
  a0=(a0-a13);
  if (res[0]!=0) res[0][20]=a0;
  a0=arg[1] ? arg[1][21] : 0;
  a13=arg[0] ? arg[0][24] : 0;
  a7=(a13-a7);
  a5=casadi_sq(a7);
  a8=arg[0] ? arg[0][25] : 0;
  a10=(a8-a10);
  a4=casadi_sq(a10);
  a5=(a5+a4);
  a4=arg[0] ? arg[0][26] : 0;
  a6=(a4-a6);
  a15=casadi_sq(a6);
  a5=(a5+a15);
  a5=sqrt(a5);
  a5=(a3/a5);
  a5=(a2-a5);
  a7=(a5*a7);
  a14=(a7-a14);
  a14=(a1*a14);
  a0=(a0-a14);
  if (res[0]!=0) res[0][21]=a0;
  a0=arg[1] ? arg[1][22] : 0;
  a10=(a5*a10);
  a9=(a10-a9);
  a9=(a1*a9);
  a0=(a0-a9);
  if (res[0]!=0) res[0][22]=a0;
  a0=arg[1] ? arg[1][23] : 0;
  a5=(a5*a6);
  a11=(a5-a11);
  a11=(a1*a11);
  a11=(a11-a12);
  a0=(a0-a11);
  if (res[0]!=0) res[0][23]=a0;
  a0=arg[1] ? arg[1][24] : 0;
  a11=arg[0] ? arg[0][27] : 0;
  a0=(a0-a11);
  if (res[0]!=0) res[0][24]=a0;
  a0=arg[1] ? arg[1][25] : 0;
  a11=arg[0] ? arg[0][28] : 0;
  a0=(a0-a11);
  if (res[0]!=0) res[0][25]=a0;
  a0=arg[1] ? arg[1][26] : 0;
  a11=arg[0] ? arg[0][29] : 0;
  a0=(a0-a11);
  if (res[0]!=0) res[0][26]=a0;
  a0=arg[1] ? arg[1][27] : 0;
  a11=arg[0] ? arg[0][30] : 0;
  a11=(a11-a13);
  a13=casadi_sq(a11);
  a6=arg[0] ? arg[0][31] : 0;
  a6=(a6-a8);
  a8=casadi_sq(a6);
  a13=(a13+a8);
  a8=arg[0] ? arg[0][32] : 0;
  a8=(a8-a4);
  a4=casadi_sq(a8);
  a13=(a13+a4);
  a13=sqrt(a13);
  a3=(a3/a13);
  a2=(a2-a3);
  a11=(a2*a11);
  a11=(a11-a7);
  a11=(a1*a11);
  a0=(a0-a11);
  if (res[0]!=0) res[0][27]=a0;
  a0=arg[1] ? arg[1][28] : 0;
  a6=(a2*a6);
  a6=(a6-a10);
  a6=(a1*a6);
  a0=(a0-a6);
  if (res[0]!=0) res[0][28]=a0;
  a0=arg[1] ? arg[1][29] : 0;
  a2=(a2*a8);
  a2=(a2-a5);
  a1=(a1*a2);
  a1=(a1-a12);
  a0=(a0-a1);
  if (res[0]!=0) res[0][29]=a0;
  a0=arg[1] ? arg[1][30] : 0;
  a1=arg[0] ? arg[0][33] : 0;
  a0=(a0-a1);
  if (res[0]!=0) res[0][30]=a0;
  a0=arg[1] ? arg[1][31] : 0;
  a1=arg[0] ? arg[0][34] : 0;
  a0=(a0-a1);
  if (res[0]!=0) res[0][31]=a0;
  a0=arg[1] ? arg[1][32] : 0;
  a1=arg[0] ? arg[0][35] : 0;
  a0=(a0-a1);
  if (res[0]!=0) res[0][32]=a0;
  a0=arg[1] ? arg[1][33] : 0;
  a1=arg[2] ? arg[2][0] : 0;
  a0=(a0-a1);
  if (res[0]!=0) res[0][33]=a0;
  a0=arg[1] ? arg[1][34] : 0;
  a1=arg[2] ? arg[2][1] : 0;
  a0=(a0-a1);
  if (res[0]!=0) res[0][34]=a0;
  a0=arg[1] ? arg[1][35] : 0;
  a1=arg[2] ? arg[2][2] : 0;
  a0=(a0-a1);
  if (res[0]!=0) res[0][35]=a0;
  return 0;
}

CASADI_SYMBOL_EXPORT int casadi_impl_ode_fun_chain_nm7(const casadi_real** arg, casadi_real** res, casadi_int* iw, casadi_real* w, void* mem){
  return casadi_f0(arg, res, iw, w, mem);
}

CASADI_SYMBOL_EXPORT void casadi_impl_ode_fun_chain_nm7_incref(void) {
}

CASADI_SYMBOL_EXPORT void casadi_impl_ode_fun_chain_nm7_decref(void) {
}

CASADI_SYMBOL_EXPORT casadi_int casadi_impl_ode_fun_chain_nm7_n_in(void) { return 3;}

CASADI_SYMBOL_EXPORT casadi_int casadi_impl_ode_fun_chain_nm7_n_out(void) { return 1;}

CASADI_SYMBOL_EXPORT const char* casadi_impl_ode_fun_chain_nm7_name_in(casadi_int i){
  switch (i) {
    case 0: return "i0";
    case 1: return "i1";
    case 2: return "i2";
    default: return 0;
  }
}

CASADI_SYMBOL_EXPORT const char* casadi_impl_ode_fun_chain_nm7_name_out(casadi_int i){
  switch (i) {
    case 0: return "o0";
    default: return 0;
  }
}

CASADI_SYMBOL_EXPORT const casadi_int* casadi_impl_ode_fun_chain_nm7_sparsity_in(casadi_int i) {
  switch (i) {
    case 0: return casadi_s0;
    case 1: return casadi_s0;
    case 2: return casadi_s1;
    default: return 0;
  }
}

CASADI_SYMBOL_EXPORT const casadi_int* casadi_impl_ode_fun_chain_nm7_sparsity_out(casadi_int i) {
  switch (i) {
    case 0: return casadi_s0;
    default: return 0;
  }
}

CASADI_SYMBOL_EXPORT int casadi_impl_ode_fun_chain_nm7_work(casadi_int *sz_arg, casadi_int* sz_res, casadi_int *sz_iw, casadi_int *sz_w) {
  if (sz_arg) *sz_arg = 3;
  if (sz_res) *sz_res = 1;
  if (sz_iw) *sz_iw = 0;
  if (sz_w) *sz_w = 0;
  return 0;
}


#ifdef __cplusplus
} /* extern "C" */
#endif
