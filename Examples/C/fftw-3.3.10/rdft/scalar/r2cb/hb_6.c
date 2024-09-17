/*
 * Copyright (c) 2003, 2007-14 Matteo Frigo
 * Copyright (c) 2003, 2007-14 Massachusetts Institute of Technology
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

/* This file was automatically generated --- DO NOT EDIT */
/* Generated on Tue Sep 14 10:46:50 EDT 2021 */

#include "rdft/codelet-rdft.h"

#if defined(ARCH_PREFERS_FMA) || defined(ISA_EXTENSION_PREFERS_FMA)

/* Generated by: ../../../genfft/gen_hc2hc.native -fma -compact -variables 4 -pipeline-latency 4 -sign 1 -n 6 -dif -name hb_6 -include rdft/scalar/hb.h */

/*
 * This function contains 46 FP additions, 32 FP multiplications,
 * (or, 24 additions, 10 multiplications, 22 fused multiply/add),
 * 31 stack variables, 2 constants, and 24 memory accesses
 */
#include "rdft/scalar/hb.h"

static void hb_6(R *cr, R *ci, const R *W, stride rs, INT mb, INT me, INT ms)
{
     DK(KP866025403, +0.866025403784438646763723170752936183471402627);
     DK(KP500000000, +0.500000000000000000000000000000000000000000000);
     {
	  INT m;
	  for (m = mb, W = W + ((mb - 1) * 10); m < me; m = m + 1, cr = cr + ms, ci = ci - ms, W = W + 10, MAKE_VOLATILE_STRIDE(12, rs)) {
	       E Td, Tn, TO, TJ, TN, Tk, Tr, T3, TC, Ts, TQ, Ta, Tm, TF, TG;
	       {
		    E Tb, Tc, Tg, TH, Tj, TI;
		    Tb = ci[WS(rs, 5)];
		    Tc = cr[WS(rs, 3)];
		    Td = Tb - Tc;
		    {
			 E Te, Tf, Th, Ti;
			 Te = ci[WS(rs, 3)];
			 Tf = cr[WS(rs, 5)];
			 Tg = Te - Tf;
			 TH = Te + Tf;
			 Th = ci[WS(rs, 4)];
			 Ti = cr[WS(rs, 4)];
			 Tj = Th - Ti;
			 TI = Th + Ti;
		    }
		    Tn = Tj - Tg;
		    TO = TH - TI;
		    TJ = TH + TI;
		    TN = Tb + Tc;
		    Tk = Tg + Tj;
		    Tr = FNMS(KP500000000, Tk, Td);
	       }
	       {
		    E T6, TD, T9, TE, T1, T2;
		    T1 = cr[0];
		    T2 = ci[WS(rs, 2)];
		    T3 = T1 + T2;
		    TC = T1 - T2;
		    {
			 E T4, T5, T7, T8;
			 T4 = cr[WS(rs, 2)];
			 T5 = ci[0];
			 T6 = T4 + T5;
			 TD = T4 - T5;
			 T7 = ci[WS(rs, 1)];
			 T8 = cr[WS(rs, 1)];
			 T9 = T7 + T8;
			 TE = T7 - T8;
		    }
		    Ts = T6 - T9;
		    TQ = TD - TE;
		    Ta = T6 + T9;
		    Tm = FNMS(KP500000000, Ta, T3);
		    TF = TD + TE;
		    TG = FNMS(KP500000000, TF, TC);
	       }
	       cr[0] = T3 + Ta;
	       ci[0] = Td + Tk;
	       {
		    E To, Tt, Tp, Tu, Tl, Tq;
		    To = FNMS(KP866025403, Tn, Tm);
		    Tt = FNMS(KP866025403, Ts, Tr);
		    Tl = W[2];
		    Tp = Tl * To;
		    Tu = Tl * Tt;
		    Tq = W[3];
		    cr[WS(rs, 2)] = FNMS(Tq, Tt, Tp);
		    ci[WS(rs, 2)] = FMA(Tq, To, Tu);
	       }
	       {
		    E T13, TZ, T11, T12, T14, T10;
		    T13 = TN + TO;
		    T10 = TC + TF;
		    TZ = W[4];
		    T11 = TZ * T10;
		    T12 = W[5];
		    T14 = T12 * T10;
		    cr[WS(rs, 3)] = FNMS(T12, T13, T11);
		    ci[WS(rs, 3)] = FMA(TZ, T13, T14);
	       }
	       {
		    E Tw, Tz, Tx, TA, Tv, Ty;
		    Tw = FMA(KP866025403, Tn, Tm);
		    Tz = FMA(KP866025403, Ts, Tr);
		    Tv = W[6];
		    Tx = Tv * Tw;
		    TA = Tv * Tz;
		    Ty = W[7];
		    cr[WS(rs, 4)] = FNMS(Ty, Tz, Tx);
		    ci[WS(rs, 4)] = FMA(Ty, Tw, TA);
	       }
	       {
		    E TR, TX, TT, TV, TW, TY, TB, TL, TM, TS, TP, TU, TK;
		    TP = FNMS(KP500000000, TO, TN);
		    TR = FMA(KP866025403, TQ, TP);
		    TX = FNMS(KP866025403, TQ, TP);
		    TU = FMA(KP866025403, TJ, TG);
		    TT = W[8];
		    TV = TT * TU;
		    TW = W[9];
		    TY = TW * TU;
		    TK = FNMS(KP866025403, TJ, TG);
		    TB = W[0];
		    TL = TB * TK;
		    TM = W[1];
		    TS = TM * TK;
		    cr[WS(rs, 1)] = FNMS(TM, TR, TL);
		    ci[WS(rs, 1)] = FMA(TB, TR, TS);
		    cr[WS(rs, 5)] = FNMS(TW, TX, TV);
		    ci[WS(rs, 5)] = FMA(TT, TX, TY);
	       }
	  }
     }
}

static const tw_instr twinstr[] = {
     { TW_FULL, 1, 6 },
     { TW_NEXT, 1, 0 }
};

static const hc2hc_desc desc = { 6, "hb_6", twinstr, &GENUS, { 24, 10, 22, 0 } };

void X(codelet_hb_6) (planner *p) {
     X(khc2hc_register) (p, hb_6, &desc);
}
#else

/* Generated by: ../../../genfft/gen_hc2hc.native -compact -variables 4 -pipeline-latency 4 -sign 1 -n 6 -dif -name hb_6 -include rdft/scalar/hb.h */

/*
 * This function contains 46 FP additions, 28 FP multiplications,
 * (or, 32 additions, 14 multiplications, 14 fused multiply/add),
 * 27 stack variables, 2 constants, and 24 memory accesses
 */
#include "rdft/scalar/hb.h"

static void hb_6(R *cr, R *ci, const R *W, stride rs, INT mb, INT me, INT ms)
{
     DK(KP500000000, +0.500000000000000000000000000000000000000000000);
     DK(KP866025403, +0.866025403784438646763723170752936183471402627);
     {
	  INT m;
	  for (m = mb, W = W + ((mb - 1) * 10); m < me; m = m + 1, cr = cr + ms, ci = ci - ms, W = W + 10, MAKE_VOLATILE_STRIDE(12, rs)) {
	       E T3, Ty, Ta, TO, Tr, TB, Td, TE, Tk, TL, Tn, TH;
	       {
		    E T1, T2, Tb, Tc;
		    T1 = cr[0];
		    T2 = ci[WS(rs, 2)];
		    T3 = T1 + T2;
		    Ty = T1 - T2;
		    {
			 E T6, Tz, T9, TA;
			 {
			      E T4, T5, T7, T8;
			      T4 = cr[WS(rs, 2)];
			      T5 = ci[0];
			      T6 = T4 + T5;
			      Tz = T4 - T5;
			      T7 = ci[WS(rs, 1)];
			      T8 = cr[WS(rs, 1)];
			      T9 = T7 + T8;
			      TA = T7 - T8;
			 }
			 Ta = T6 + T9;
			 TO = KP866025403 * (Tz - TA);
			 Tr = KP866025403 * (T6 - T9);
			 TB = Tz + TA;
		    }
		    Tb = ci[WS(rs, 5)];
		    Tc = cr[WS(rs, 3)];
		    Td = Tb - Tc;
		    TE = Tb + Tc;
		    {
			 E Tg, TG, Tj, TF;
			 {
			      E Te, Tf, Th, Ti;
			      Te = ci[WS(rs, 3)];
			      Tf = cr[WS(rs, 5)];
			      Tg = Te - Tf;
			      TG = Te + Tf;
			      Th = ci[WS(rs, 4)];
			      Ti = cr[WS(rs, 4)];
			      Tj = Th - Ti;
			      TF = Th + Ti;
			 }
			 Tk = Tg + Tj;
			 TL = KP866025403 * (TG + TF);
			 Tn = KP866025403 * (Tj - Tg);
			 TH = TF - TG;
		    }
	       }
	       cr[0] = T3 + Ta;
	       ci[0] = Td + Tk;
	       {
		    E TC, TI, Tx, TD;
		    TC = Ty + TB;
		    TI = TE - TH;
		    Tx = W[4];
		    TD = W[5];
		    cr[WS(rs, 3)] = FNMS(TD, TI, Tx * TC);
		    ci[WS(rs, 3)] = FMA(TD, TC, Tx * TI);
	       }
	       {
		    E To, Tu, Ts, Tw, Tm, Tq;
		    Tm = FNMS(KP500000000, Ta, T3);
		    To = Tm - Tn;
		    Tu = Tm + Tn;
		    Tq = FNMS(KP500000000, Tk, Td);
		    Ts = Tq - Tr;
		    Tw = Tr + Tq;
		    {
			 E Tl, Tp, Tt, Tv;
			 Tl = W[2];
			 Tp = W[3];
			 cr[WS(rs, 2)] = FNMS(Tp, Ts, Tl * To);
			 ci[WS(rs, 2)] = FMA(Tl, Ts, Tp * To);
			 Tt = W[6];
			 Tv = W[7];
			 cr[WS(rs, 4)] = FNMS(Tv, Tw, Tt * Tu);
			 ci[WS(rs, 4)] = FMA(Tt, Tw, Tv * Tu);
		    }
	       }
	       {
		    E TM, TS, TQ, TU, TK, TP;
		    TK = FNMS(KP500000000, TB, Ty);
		    TM = TK - TL;
		    TS = TK + TL;
		    TP = FMA(KP500000000, TH, TE);
		    TQ = TO + TP;
		    TU = TP - TO;
		    {
			 E TJ, TN, TR, TT;
			 TJ = W[0];
			 TN = W[1];
			 cr[WS(rs, 1)] = FNMS(TN, TQ, TJ * TM);
			 ci[WS(rs, 1)] = FMA(TN, TM, TJ * TQ);
			 TR = W[8];
			 TT = W[9];
			 cr[WS(rs, 5)] = FNMS(TT, TU, TR * TS);
			 ci[WS(rs, 5)] = FMA(TT, TS, TR * TU);
		    }
	       }
	  }
     }
}

static const tw_instr twinstr[] = {
     { TW_FULL, 1, 6 },
     { TW_NEXT, 1, 0 }
};

static const hc2hc_desc desc = { 6, "hb_6", twinstr, &GENUS, { 32, 14, 14, 0 } };

void X(codelet_hb_6) (planner *p) {
     X(khc2hc_register) (p, hb_6, &desc);
}
#endif
