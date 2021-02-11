/**
 * $Id$
 *
 * @brief Red Pitaya Spectrum Analyzer DSC processing.
 *
 * @Author Jure Menart <juremenart@gmail.com>
 *         
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>

#include "spec_dsp.h"
//#include "spectrometerApp.h"
#include "spec_fpga.h"
#include "kiss_fftr.h"
#include "rp_cross.h"

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

extern float g_spectr_fpga_adc_max_v;
extern const int c_spectr_fpga_adc_bits;

/* length of output signals: floor(SPECTR_FPGA_SIG_LEN/2) */

#define SPECTR_OUT_SIG_LENGTH (rp_get_fpga_signal_length()/2)
#define SPECTR_FPGA_SIG_LEN   (rp_get_fpga_signal_length())

#define RP_SPECTR_HANN_AMP 0.8165 // Hann window power scaling (1/sqrt(sum(rcos.^2/N)))

#define RP_BLACKMAN_A0 0.35875
#define RP_BLACKMAN_A1 0.48829
#define RP_BLACKMAN_A2 0.14128
#define RP_BLACKMAN_A3 0.01168

#define RP_FLATTOP_A0 0.21557895
#define RP_FLATTOP_A1 0.41663158
#define RP_FLATTOP_A2 0.277263158
#define RP_FLATTOP_A3 0.083578947
#define RP_FLATTOP_A4 0.006947368

/* Internal structures used in DSP  */
double               *rp_window   = NULL;
kiss_fft_cpx         *rp_kiss_fft_out1 = NULL;
kiss_fft_cpx         *rp_kiss_fft_out2 = NULL;
kiss_fftr_cfg         rp_kiss_fft_cfg  = NULL;


window_mode_t         g_window_mode = HANNING;
int                   g_remove_DC = 1;
/* constants - calibration dependant */
/* Power calc. impedance*/
const double c_imp = 50;
/* Const - [W] -> [mW] */
const double c_w2mw = 1000;

int rp_spectr_prepare_freq_vector(float **freq_out, double f_s, float freq_range)
{
    int i;
    float *f = *freq_out;
    float freq_smpl = f_s / (float)spectr_fpga_cnv_freq_range_to_dec(freq_range);
    /* Divider to get to the right units - [MHz], [kHz] or [Hz] */
    float unit_div = 1e6;

    if(!f) {
        fprintf(stderr, "rp_spectr_prepare_freq_vector() not initialized\n");
        return -1;
    }
    
    switch(spectr_fpga_cnv_freq_range_to_unit(freq_range)) {
    case 2:
        unit_div = 1e6;
        break;
    case 1:
        unit_div = 1e3;
        break;
    case 0:
        unit_div = 1;
        break;
    default:
        fprintf(stderr, "rp_spectr_prepare_freq_vector() wrong freq_range\n");
        return -1;
    }

    //step = (int)round((float)c_dsp_sig_len / (float)SPECTR_OUT_SIG_LENGTH);
    //if(step < 1)
    //    step = 1;

    for(i = 0; i < SPECTR_OUT_SIG_LENGTH; i++) {
        /* We use full FPGA signal length range for this calculation, eventhough
         * the output vector is smaller. */
        f[i] = (float)i / (float)SPECTR_FPGA_SIG_LEN * freq_smpl / unit_div;
    }

    return 0;
}



unsigned short rp_get_spectr_out_signal_length(){
    return rp_get_fpga_signal_length()/2;
}

unsigned short rp_get_spectr_out_signal_max_length(){
    return rp_get_fpga_signal_max_length()/2;
}

// int rp_spectr_hann_init()
// {
//     int i;

//     rp_spectr_hann_clean(rp_hann_window);

//     rp_window = (double *)malloc(SPECTR_FPGA_SIG_LEN * sizeof(double));
//     if(rp_hann_window == NULL) {
//         fprintf(stderr, "rp_spectr_hann_create() can not allocate mem");
//         return -1;
//     }
    
//     for(i = 0; i < SPECTR_FPGA_SIG_LEN; i++) {
//         rp_hann_window[i] = RP_SPECTR_HANN_AMP * 
//             (1 - cos(2*M_PI*i / (double)(SPECTR_FPGA_SIG_LEN-1)));
//     }

//     return 0;
// }

// int rp_spectr_hann_clean()
// {
//     if(rp_window) {
//         free(rp_window);
//         rp_window = NULL;
//     }
//     return 0;
// }


// int rp_spectr_hann_filter(double *cha_in, double *chb_in,
//                           double **cha_out, double **chb_out)
// {
//     int i;
//     double *cha_o = *cha_out;
//     double *chb_o = *chb_out;

//     if(!cha_in || !chb_in || !*cha_out || !*chb_out)
//         return -1;
//     for(i = 0; i < SPECTR_FPGA_SIG_LEN; i++) {
//         cha_o[i] = cha_in[i] * rp_hann_window[i];
//         chb_o[i] = chb_in[i] * rp_hann_window[i];
//     }

//     return 0;
// }

double __zeroethOrderBessel( double x )
{
    const double eps = 0.000001;
    double Value = 0;
    double term = 1;
    double m = 0;

    while(term  > eps * Value)
    {
        Value += term;
        ++m;
        term *= (x*x) / (4*m*m);
    }   
    return Value;
}

int rp_spectr_window_init(window_mode_t mode){
    int i;

    g_window_mode = mode;
    rp_spectr_window_clean();
    
    rp_window = (double *)malloc(rp_get_fpga_signal_max_length() * sizeof(double));
    if(rp_window == NULL) {
        fprintf(stderr, "rp_spectr_window_init() can not allocate mem");
        return -1;
    }

    switch(g_window_mode) {
        case HANNING:{
            for(i = 0; i < SPECTR_FPGA_SIG_LEN; i++) {
                rp_window[i] = RP_SPECTR_HANN_AMP * 
                (1 - cos(2*M_PI*i / (double)(SPECTR_FPGA_SIG_LEN-1)));
            }
            break;
        }
        case RECTANGULAR:{
           for(i = 0; i < SPECTR_FPGA_SIG_LEN; i++) {
                rp_window[i] = 1;
            }
            break;
        }
        case HAMMING:{
            for(i = 0; i < SPECTR_FPGA_SIG_LEN; i++) {
                rp_window[i] = 0.54 - 
                0.46 * cos(2*M_PI*i / (double)(SPECTR_FPGA_SIG_LEN-1));
            }
            break;
        }
        case BLACKMAN_HARRIS:{
            for(i = 0; i < SPECTR_FPGA_SIG_LEN; i++) {
                rp_window[i] = RP_BLACKMAN_A0 - 
                               RP_BLACKMAN_A1 * cos(2*M_PI*i / (double)(SPECTR_FPGA_SIG_LEN-1)) +
                               RP_BLACKMAN_A2 * cos(4*M_PI*i / (double)(SPECTR_FPGA_SIG_LEN-1)) -
                               RP_BLACKMAN_A3 * cos(6*M_PI*i / (double)(SPECTR_FPGA_SIG_LEN-1));
            }
            break;
        }
        case FLAT_TOP:{
            for(i = 0; i < SPECTR_FPGA_SIG_LEN; i++) {
                rp_window[i] = RP_FLATTOP_A0 - 
                               RP_FLATTOP_A1 * cos(2*M_PI*i / (double)(SPECTR_FPGA_SIG_LEN-1)) +
                               RP_FLATTOP_A2 * cos(4*M_PI*i / (double)(SPECTR_FPGA_SIG_LEN-1)) -
                               RP_FLATTOP_A3 * cos(6*M_PI*i / (double)(SPECTR_FPGA_SIG_LEN-1)) +
                               RP_FLATTOP_A4 * cos(8*M_PI*i / (double)(SPECTR_FPGA_SIG_LEN-1));
            }
            break;
        }
        case KAISER_4:{
            const double x = 1.0 / __zeroethOrderBessel(4);
            const double y = (SPECTR_FPGA_SIG_LEN - 1) / 2.0;

            for(i = 0; i < SPECTR_FPGA_SIG_LEN; i++) {
                const double K = (i - y) / y;
                const double arg = sqrt( 1.0 - (K * K) );
                rp_window[i] = __zeroethOrderBessel( 4 * arg ) * x;

            }
            break;
        }

        case KAISER_8:{
            const double x = 1.0 / __zeroethOrderBessel(8);
            const double y = (SPECTR_FPGA_SIG_LEN - 1) / 2.0;

            for(i = 0; i < SPECTR_FPGA_SIG_LEN; i++) {
                const double K = (i - y) / y;
                const double arg = sqrt( 1.0 - (K * K) );
                rp_window[i] = __zeroethOrderBessel( 8 * arg ) * x;

            }
            break;
        }
        default:
            rp_spectr_window_clean();
            return -1;
    }
    return 0;
}

window_mode_t rp_spectr_get_current_windows(){
    return g_window_mode;
}

void rp_spectr_remove_DC(int state){
    g_remove_DC = state;
}

int rp_spectr_get_remove_DC(){
    return g_remove_DC;
}

int rp_spectr_window_clean(){
    if(rp_window) {
        free(rp_window);
        rp_window = NULL;
    }
    return 0;
}

int rp_spectr_window_filter(double *cha_in, double *chb_in,
                          double **cha_out, double **chb_out){
    int i;
    double *cha_o = *cha_out;
    double *chb_o = *chb_out;

    if(!cha_in || !chb_in || !*cha_out || !*chb_out)
        return -1;
    for(i = 0; i < SPECTR_FPGA_SIG_LEN; i++) {
        cha_o[i] = cha_in[i] * rp_window[i];
        chb_o[i] = chb_in[i] * rp_window[i];

        // if (i < 10) {
        //     fprintf(stderr,"W: %d - (%f , %f)\n",i,cha_o[i],chb_o[i]);
        // }
    }

    return 0;
}

int rp_spectr_fft_init()
{
    if(rp_kiss_fft_out1 || rp_kiss_fft_out2 || rp_kiss_fft_cfg) {
        rp_spectr_fft_clean();
    }

    rp_kiss_fft_out1 = 
        (kiss_fft_cpx *)malloc(rp_get_fpga_signal_length() * sizeof(kiss_fft_cpx));
    rp_kiss_fft_out2 =
        (kiss_fft_cpx *)malloc(rp_get_fpga_signal_length() * sizeof(kiss_fft_cpx));

    rp_kiss_fft_cfg = kiss_fftr_alloc(rp_get_fpga_signal_length(), 0, NULL, NULL);

    return 0;
}

int rp_spectr_fft_clean()
{
    kiss_fft_cleanup();
    if(rp_kiss_fft_out1) {
        free(rp_kiss_fft_out1);
        rp_kiss_fft_out1 = NULL;
    }
    if(rp_kiss_fft_out2) {
        free(rp_kiss_fft_out2);
        rp_kiss_fft_out2 = NULL;
    }
    if(rp_kiss_fft_cfg) {
        free(rp_kiss_fft_cfg);
        rp_kiss_fft_cfg = NULL;
    }
    return 0;
}

int rp_spectr_fft(double *cha_in, double *chb_in, 
                  double **cha_out, double **chb_out)
{
    double *cha_o = *cha_out;
    double *chb_o = *chb_out;
    int i;
    if(!cha_in || !chb_in || !*cha_out || !*chb_out)
        return -1;

    if(!rp_kiss_fft_out1 || !rp_kiss_fft_out2 || !rp_kiss_fft_cfg) {
        fprintf(stderr, "rp_spect_fft not initialized");
        return -1;
    }

    kiss_fftr(rp_kiss_fft_cfg, (kiss_fft_scalar *)cha_in, rp_kiss_fft_out1);
    kiss_fftr(rp_kiss_fft_cfg, (kiss_fft_scalar *)chb_in, rp_kiss_fft_out2);

    for(i = 0; i < SPECTR_OUT_SIG_LENGTH; i++) {                     // FFT limited to fs/2, specter of amplitudes
        cha_o[i] = sqrt(pow(rp_kiss_fft_out1[i].r, 2) + 
                        pow(rp_kiss_fft_out1[i].i, 2));
        chb_o[i] = sqrt(pow(rp_kiss_fft_out2[i].r, 2) + 
                        pow(rp_kiss_fft_out2[i].i, 2));
        // if (i < 10) {
        //     fprintf(stderr,"%d - (%f , %f)\n",i,cha_o[i],chb_o[i]);
        // }
    }
    return 0;
}

int rp_spectr_decimate(double *cha_in, double *chb_in, 
                       float **cha_out, float **chb_out,
                       int in_len, int out_len)
{
    int step;
    int i, j;
    float *cha_o = *cha_out;
    float *chb_o = *chb_out;

    if(!cha_in || !chb_in || !*cha_out || !*chb_out)
        return -1;

    step = (int)round((float)in_len / (float)out_len);
    if(step < 1)
        step = 1;

    for(i = 0, j = 0; i < out_len; i++, j+=step) {
        int k=j;

        if(j >= in_len) {
            fprintf(stderr, "rp_spectr_decimate() index too high\n");
            return -1;
        }
        cha_o[i] = 0;
        chb_o[i] = 0;
	
	/* Conversion factor from ADC counts to Volts */
    	double c2v = g_spectr_fpga_adc_max_v/(float)((int)(1<<(c_spectr_fpga_adc_bits-1)));
	
        for(k=j; k < j+step; k++) {
	
	/* Conversion to power (Watts) */  
        double cha_p = pow(cha_in[k] * c2v, 2) / c_imp /
            (double)SPECTR_FPGA_SIG_LEN / (double)SPECTR_FPGA_SIG_LEN * 2; // x 2 for unilateral spectral density representation
        double chb_p = pow(chb_in[k] * c2v, 2) / c_imp /                   // c_imp = 50 Ohms, is the transmission line impdeance   
            (double)SPECTR_FPGA_SIG_LEN / (double)SPECTR_FPGA_SIG_LEN * 2;
	  
	  
            cha_o[i] += (float)cha_p;  // Summing the power expressed in Watts associated to each FFT bin
            chb_o[i] += (float)chb_p;
        }
        // if (i < 10) {
        //     fprintf(stderr,"Dec %d - (%f , %f) step %d\n",i,cha_o[i],chb_o[i],step);
        // }
    }

    return 0;
}

int rp_spectr_cnv_to_dBm(float *cha_in, float *chb_in,
                         float **cha_out, float **chb_out,
                         float *peak_power_cha, float *peak_freq_cha,
                         float *peak_power_chb, float *peak_freq_chb,
                         float freq_range)
{
    int i;
    float *cha_o = *cha_out;
    float *chb_o = *chb_out;
    double max_pw_cha = -1e5;
    double max_pw_chb = -1e5;
    int max_pw_idx_cha = 0;
    int max_pw_idx_chb = 0;
    float freq_smpl = spectr_get_fpga_smpl_freq() / 
        (float)spectr_fpga_cnv_freq_range_to_dec(freq_range);

    /* Divider to get to the right units - [MHz], [kHz] or [Hz] */
    float unit_div = 1e6;

    if(!cha_in || !chb_in || !*cha_out || !*chb_out)
        return -1;

    switch(spectr_fpga_cnv_freq_range_to_unit(freq_range)) {
    case 2:
        unit_div = 1e6;
        break;
    case 1:
        unit_div = 1e3;
        break;
    case 0:
        unit_div = 1;
        break;
    default:
        fprintf(stderr, "rp_spectr_prepare_freq_vector() wrong freq_range\n");
        return -1;
    }
    if (g_remove_DC != 0) {
            cha_o[0] = cha_o[1] = cha_o[2];
            chb_o[0] = chb_o[1] = chb_o[2];
    }

    for(i = 0; i < SPECTR_OUT_SIG_LENGTH; i++) {

        /* Conversion to power (Watts) */

        double cha_p=cha_in[i];
        double chb_p=chb_in[i];    

        
        
        
        // Avoiding -Inf due to log10(0.0) 
        
        if (cha_p * c_w2mw > 1.0e-12 )	
            cha_o[i] = 10 * log10(cha_p * c_w2mw);  // W -> mW -> dBm
        else	
            cha_o[i] = 10 * log10(1.0e-12);  
        
        
        if (chb_p * c_w2mw > 1.0e-12 )        	
            chb_o[i] = 10 * log10(chb_p * c_w2mw);
        else	
            chb_o[i] = 10 * log10(1.0e-12);
        

        // /* Issue #3369: Remove DC component */
        // const float c_dc_noise = -80.0; /* [dBm] */
        // const int   c_dc_span  =  2;    /* [output samples] */
        // if (i < c_dc_span) {
        //     cha_o[i] = c_dc_noise;
        //     chb_o[i] = c_dc_noise;
        // }

       

        /* Find peaks */
        if(cha_o[i] > max_pw_cha) {
            max_pw_cha     = cha_o[i];
            max_pw_idx_cha = i;
        }
        if(chb_o[i] > max_pw_chb) {
            max_pw_chb     = chb_o[i];
            max_pw_idx_chb = i;
        }
    }

	// Power correction (summing contributions of contiguous bins)
	const int c_pwr_int_cnts=3; // Number of bins on the left and right side of the max
	float cha_pwr=0;
	float chb_pwr=0;
	int ii;
	int ixxa,ixxb;
        for(ii = 0; ii < (c_pwr_int_cnts*2+1); ii++) {
	  
	  ixxa=max_pw_idx_cha+ii-c_pwr_int_cnts;
	  ixxb=max_pw_idx_chb+ii-c_pwr_int_cnts;
	  
	if ((ixxa>=0) && (ixxa<SPECTR_OUT_SIG_LENGTH)) 
	 {
	   cha_pwr+=pow(10.0,cha_o[ixxa]/10.0);
	 }
	if ((ixxb>=0) && (ixxb<SPECTR_OUT_SIG_LENGTH)) 
	 {
	   chb_pwr+=pow(10.0,chb_o[ixxb]/10.0);
	 }	 
	
	
	}
       
    if (cha_pwr<=1.0e-10)
        max_pw_cha  = -200.0;
    else
        max_pw_cha     = 10.0 * log10(cha_pwr);
       
    if (chb_pwr<=1.0e-10)
	    max_pw_chb  = -200.0;
    else
        max_pw_chb     = 10.0 * log10(chb_pwr);
       

       
    *peak_power_cha = max_pw_cha;
    *peak_freq_cha = ((float)max_pw_idx_cha / (float)SPECTR_OUT_SIG_LENGTH * 
                      freq_smpl  / 2) / unit_div;
    *peak_power_chb = max_pw_chb;
    *peak_freq_chb = ((float)max_pw_idx_chb / (float)SPECTR_OUT_SIG_LENGTH * 
                      freq_smpl / 2) / unit_div;

    return 0;
}

