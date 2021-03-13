/*
 This file is part of program wsprd, a detector/demodulator/decoder
 for the Weak Signal Propagation Reporter (WSPR) mode.

 File name: wsprd.c

 Copyright 2001-2015, Joe Taylor, K1JT

 Much of the present code is based on work by Steven Franke, K9AN,
 which in turn was based on earlier work by K1JT.

 Copyright 2014-2015, Steven Franke, K9AN

 License: GNU GPL v3

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// Edited by:Michel Barbeau
// Version: April 27, 2017

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <fftw3.h>
#include <sys/time.h>

#include "fano.h"
#include "jelinek.h"
#include "nhash.h"
#include "wsprd_utils.h"
#include "wsprsim_utils.h"

#define max(x,y) ((x) > (y) ? (x) : (y))
// Possible PATIENCE options: FFTW_ESTIMATE, FFTW_ESTIMATE_PATIENT,
// FFTW_MEASURE, FFTW_PATIENT, FFTW_EXHAUSTIVE
#define PATIENCE FFTW_ESTIMATE
fftwf_plan PLAN1,PLAN2,PLAN3;

unsigned char pr3[162]=
{1,1,0,0,0,0,0,0,1,0,0,0,1,1,1,0,0,0,1,0,
    0,1,0,1,1,1,1,0,0,0,0,0,0,0,1,0,0,1,0,1,
    0,0,0,0,0,0,1,0,1,1,0,0,1,1,0,1,0,0,0,1,
    1,0,1,0,0,0,0,1,1,0,1,0,1,0,1,0,1,0,0,1,
    0,0,1,0,1,1,0,0,0,1,1,0,1,0,1,0,0,0,1,0,
    0,0,0,0,1,0,0,1,0,0,1,1,1,0,1,1,0,0,1,1,
    0,1,0,0,0,1,1,1,0,0,0,0,0,1,0,1,0,0,1,1,
    0,0,0,0,0,0,0,1,1,0,1,0,1,1,0,0,0,1,1,0,
    0,0};

unsigned long nr;

int printdata=0;

//***************************************************************************
unsigned long readc2file(char *ptr_to_infile, float *idat, float *qdat,
                         double *freq, int *wspr_type)
{
    float *buffer;
    double dfreq;
    int i,ntrmin;
    char *c2file[15];
    FILE* fp;

    buffer=malloc(sizeof(float)*2*65536);
    memset(buffer,0,sizeof(float)*2*65536);

    fp = fopen(ptr_to_infile,"rb");
    if (fp == NULL) {
        fprintf(stderr, "Cannot open data file '%s'\n", ptr_to_infile);
        return 1;
    }
    unsigned long nread=fread(c2file,sizeof(char),14,fp);
    nread=fread(&ntrmin,sizeof(int),1,fp);
    nread=fread(&dfreq,sizeof(double),1,fp);
    *freq=dfreq;
    nread=fread(buffer,sizeof(float),2*45000,fp);
    fclose(fp);

    *wspr_type=ntrmin;

    for(i=0; i<45000; i++) {
        idat[i]=buffer[2*i];
        qdat[i]=-buffer[2*i+1];
    }

    if( nread == 2*45000 ) {
        return nread/2;
    } else {
        return 1;
    }
    free(buffer);
}

//***************************************************************************
// Reads wave file (that contains in-phase signal) and generates in-phase and
// quadrarure signals (using a FFT and an inverser FFT).
unsigned long readwavfile(char *ptr_to_infile, int ntrmin, float *idat, float *qdat )
{
    size_t i, j, npoints;
    int nfft1, nfft2, nh2, i0;
    double df;

    nfft2=46080; //this is the number of downsampled points that will be returned
    nh2=nfft2/2;

    if( ntrmin == 2 ) {
        nfft1=nfft2*32;      //need to downsample by a factor of 32
        df=12000.0/nfft1;
        i0=1500.0/df+0.5;
        npoints=114*12000;
    } else if ( ntrmin == 15 ) {
        nfft1=nfft2*8*32;
        df=12000.0/nfft1;
        i0=(1500.0+112.5)/df+0.5;
        npoints=8*114*12000;
    } else {
        fprintf(stderr,"This should not happen\n");
        return 1;
    }

    float *realin;
    fftwf_complex *fftin, *fftout;

    FILE *fp;
    short int *buf2;
    buf2 = malloc(npoints*sizeof(short int));

    fp = fopen(ptr_to_infile,"rb");
    if (fp == NULL) {
        fprintf(stderr, "Cannot open data file '%s'\n", ptr_to_infile);
        return 1;
    }
    nr=fread(buf2,2,22,fp);            //Read and ignore header
    nr=fread(buf2,2,npoints,fp);       //Read raw data
    fclose(fp);

    realin=(float*) fftwf_malloc(sizeof(float)*nfft1);
    fftout=(fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex)*nfft1);
    PLAN1 = fftwf_plan_dft_r2c_1d(nfft1, realin, fftout, PATIENCE);

    for (i=0; i<npoints; i++) {
        // conversion to float in the [-1 1] range
        realin[i]=buf2[i]/32768.0;
    }

    for (i=npoints; i<(size_t)nfft1; i++) {
        realin[i]=0.0;
    }

    free(buf2);
    fftwf_execute(PLAN1);
    fftwf_free(realin);

    // Inverse FFT, with downsampling by a factor of 32
    fftin=(fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex)*nfft2);

    for (i=0; i<(size_t)nfft2; i++) {
        j=i0+i;
        if( i>(size_t)nh2 ) j=j-nfft2;
        fftin[i][0]=fftout[j][0];
        fftin[i][1]=fftout[j][1];
    }

    fftwf_free(fftout);
    fftout=(fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex)*nfft2);
    PLAN2 = fftwf_plan_dft_1d(nfft2, fftin, fftout, FFTW_BACKWARD, PATIENCE);
    fftwf_execute(PLAN2);

    for (i=0; i<(size_t)nfft2; i++) {
        idat[i]=fftout[i][0]/1000.0;
        qdat[i]=fftout[i][1]/1000.0;
    }

    fftwf_free(fftin);
    fftwf_free(fftout);
    return nfft2;
}

//***************************************************************************
void sync_and_demodulate(float *id, float *qd, long np,
                         unsigned char *symbols, float *f1, int ifmin, int ifmax, float fstep,
                         int *shift1, int lagmin, int lagmax, int lagstep,
                         float *drift1, int symfac, float *sync, int mode)
{
    /***********************************************************************
     * mode = 0: no frequency or drift search. find best time lag.          *
     *        1: no time lag or drift search. find best frequency.          *
     *        2: no frequency or time lag search. calculate soft-decision   *
     *           symbols using passed frequency and shift.                  *
     ************************************************************************/
//
// Input signal:
//    id = in-phase (float array of maxpts=65536)
//    qq = quarature (float array of maxpts=65536)
//    np = number of points
// symbols = (char array of size nbits (81) times 2)

    static float fplast=-10000.0;
    // delta time (dt) and delta (frequency)
    static float dt=1.0/375.0, df=375.0/256.0;
    static float pi=3.14159265358979323846;
    float twopidt, df15=df*1.5, df05=df*0.5;

    int i, j, k, lag;
    float i0[162],q0[162],i1[162],q1[162],i2[162],q2[162],i3[162],q3[162];
    float p0,p1,p2,p3,cmet,totp,syncmax,fac;
    float c0[256],s0[256],c1[256],s1[256],c2[256],s2[256],c3[256],s3[256];
    float dphi0, cdphi0, sdphi0, dphi1, cdphi1, sdphi1, dphi2, cdphi2, sdphi2,
    dphi3, cdphi3, sdphi3;
    float f0=0.0, fp, ss, fbest=0.0, fsum=0.0, f2sum=0.0, fsymb[162];
    int best_shift = 0, ifreq;

    syncmax=-1e30;
    if( mode == 0 ) {ifmin=0; ifmax=0; fstep=0.0; f0=*f1;}
    if( mode == 1 ) {lagmin=*shift1;lagmax=*shift1;f0=*f1;}
    if( mode == 2 ) {lagmin=*shift1;lagmax=*shift1;ifmin=0;ifmax=0;f0=*f1;}

    twopidt=2*pi*dt;
    for(ifreq=ifmin; ifreq<=ifmax; ifreq++) {
        f0=*f1+ifreq*fstep;
        for(lag=lagmin; lag<=lagmax; lag=lag+lagstep) {
            ss=0.0;
            totp=0.0;
            for (i=0; i<162; i++) {
                fp = f0 + (*drift1/2.0)*((float)i-81.0)/81.0;
                if( i==0 || (fp != fplast) ) {  // only calculate sin/cos if necessary
                    dphi0=twopidt*(fp-df15);
                    cdphi0=cos(dphi0);
                    sdphi0=sin(dphi0);

                    dphi1=twopidt*(fp-df05);
                    cdphi1=cos(dphi1);
                    sdphi1=sin(dphi1);

                    dphi2=twopidt*(fp+df05);
                    cdphi2=cos(dphi2);
                    sdphi2=sin(dphi2);

                    dphi3=twopidt*(fp+df15);
                    cdphi3=cos(dphi3);
                    sdphi3=sin(dphi3);

                    c0[0]=1; s0[0]=0;
                    c1[0]=1; s1[0]=0;
                    c2[0]=1; s2[0]=0;
                    c3[0]=1; s3[0]=0;

                    for (j=1; j<256; j++) {
                        c0[j]=c0[j-1]*cdphi0 - s0[j-1]*sdphi0;
                        s0[j]=c0[j-1]*sdphi0 + s0[j-1]*cdphi0;
                        c1[j]=c1[j-1]*cdphi1 - s1[j-1]*sdphi1;
                        s1[j]=c1[j-1]*sdphi1 + s1[j-1]*cdphi1;
                        c2[j]=c2[j-1]*cdphi2 - s2[j-1]*sdphi2;
                        s2[j]=c2[j-1]*sdphi2 + s2[j-1]*cdphi2;
                        c3[j]=c3[j-1]*cdphi3 - s3[j-1]*sdphi3;
                        s3[j]=c3[j-1]*sdphi3 + s3[j-1]*cdphi3;
                    }
                    fplast = fp;
                }

                i0[i]=0.0; q0[i]=0.0;
                i1[i]=0.0; q1[i]=0.0;
                i2[i]=0.0; q2[i]=0.0;
                i3[i]=0.0; q3[i]=0.0;

                for (j=0; j<256; j++) {
                    k=lag+i*256+j;
                    if( (k>0) && (k<np) ) {
                        i0[i]=i0[i] + id[k]*c0[j] + qd[k]*s0[j];
                        q0[i]=q0[i] - id[k]*s0[j] + qd[k]*c0[j];
                        i1[i]=i1[i] + id[k]*c1[j] + qd[k]*s1[j];
                        q1[i]=q1[i] - id[k]*s1[j] + qd[k]*c1[j];
                        i2[i]=i2[i] + id[k]*c2[j] + qd[k]*s2[j];
                        q2[i]=q2[i] - id[k]*s2[j] + qd[k]*c2[j];
                        i3[i]=i3[i] + id[k]*c3[j] + qd[k]*s3[j];
                        q3[i]=q3[i] - id[k]*s3[j] + qd[k]*c3[j];
                    }
                }
                p0=i0[i]*i0[i] + q0[i]*q0[i];
                p1=i1[i]*i1[i] + q1[i]*q1[i];
                p2=i2[i]*i2[i] + q2[i]*q2[i];
                p3=i3[i]*i3[i] + q3[i]*q3[i];

                p0=sqrt(p0);
                p1=sqrt(p1);
                p2=sqrt(p2);
                p3=sqrt(p3);

                totp=totp+p0+p1+p2+p3;
                cmet=(p1+p3)-(p0+p2);
                ss = (pr3[i] == 1) ? ss+cmet : ss-cmet;
                if( mode == 2) {                 //Compute soft symbols
                    if(pr3[i]==1) {
                        fsymb[i]=p3-p1;
                    } else {
                        fsymb[i]=p2-p0;
                    }
                }
            }
            ss=ss/totp;
            if( ss > syncmax ) {          //Save best parameters
                syncmax=ss;
                best_shift=lag;
                fbest=f0;
            }
        } // lag loop
    } //freq loop

    if( mode <=1 ) {                       //Send best params back to caller
        *sync=syncmax;
        *shift1=best_shift;
        *f1=fbest;
        return;
    }

    if( mode == 2 ) {
        *sync=syncmax;
        for (i=0; i<162; i++) {              //Normalize the soft symbols
            fsum=fsum+fsymb[i]/162.0;
            f2sum=f2sum+fsymb[i]*fsymb[i]/162.0;
        }
        fac=sqrt(f2sum-fsum*fsum);
        for (i=0; i<162; i++) {
            fsymb[i]=symfac*fsymb[i]/fac;
            if( fsymb[i] > 127) fsymb[i]=127.0;
            if( fsymb[i] < -128 ) fsymb[i]=-128.0;
            symbols[i]=fsymb[i] + 128;
        }
        return;
    }
    return;
}
/***************************************************************************
 symbol-by-symbol signal subtraction
 ****************************************************************************/
void subtract_signal(float *id, float *qd, long np,
                     float f0, int shift0, float drift0, unsigned char* channel_symbols)
{
    float dt=1.0/375.0, df=375.0/256.0;
    int i, j, k;
    float pi=4.*atan(1.0),twopidt, fp;

    float i0,q0;
    float c0[256],s0[256];
    float dphi, cdphi, sdphi;

    twopidt=2*pi*dt;

    for (i=0; i<162; i++) {
        fp = f0 + ((float)drift0/2.0)*((float)i-81.0)/81.0;

        dphi=twopidt*(fp+((float)channel_symbols[i]-1.5)*df);
        cdphi=cos(dphi);
        sdphi=sin(dphi);

        c0[0]=1; s0[0]=0;

        for (j=1; j<256; j++) {
            c0[j]=c0[j-1]*cdphi - s0[j-1]*sdphi;
            s0[j]=c0[j-1]*sdphi + s0[j-1]*cdphi;
        }

        i0=0.0; q0=0.0;

        for (j=0; j<256; j++) {
            k=shift0+i*256+j;
            if( (k>0) & (k<np) ) {
                i0=i0 + id[k]*c0[j] + qd[k]*s0[j];
                q0=q0 - id[k]*s0[j] + qd[k]*c0[j];
            }
        }


        // subtract the signal here.

        i0=i0/256.0; //will be wrong for partial symbols at the edges...
        q0=q0/256.0;

        for (j=0; j<256; j++) {
            k=shift0+i*256+j;
            if( (k>0) & (k<np) ) {
                id[k]=id[k]- (i0*c0[j] - q0*s0[j]);
                qd[k]=qd[k]- (q0*c0[j] + i0*s0[j]);
            }
        }
    }
    return;
}
/******************************************************************************
 Fully coherent signal subtraction
 *******************************************************************************/
void subtract_signal2(float *id, float *qd, long np,
                      float f0, int shift0, float drift0, unsigned char* channel_symbols)
{
    float dt=1.0/375.0, df=375.0/256.0;
    float pi=4.*atan(1.0), twopidt, phi=0, dphi, cs;
    int i, j, k, ii, nsym=162, nspersym=256,  nfilt=256; //nfilt must be even number.
    int nsig=nsym*nspersym;
    int nc2=45000;

    float *refi, *refq, *ci, *cq, *cfi, *cfq;

    refi=malloc(sizeof(float)*nc2);
    refq=malloc(sizeof(float)*nc2);
    ci=malloc(sizeof(float)*nc2);
    cq=malloc(sizeof(float)*nc2);
    cfi=malloc(sizeof(float)*nc2);
    cfq=malloc(sizeof(float)*nc2);

    memset(refi,0,sizeof(float)*nc2);
    memset(refq,0,sizeof(float)*nc2);
    memset(ci,0,sizeof(float)*nc2);
    memset(cq,0,sizeof(float)*nc2);
    memset(cfi,0,sizeof(float)*nc2);
    memset(cfq,0,sizeof(float)*nc2);

    twopidt=2.0*pi*dt;

    /******************************************************************************
     Measured signal:                    s(t)=a(t)*exp( j*theta(t) )
     Reference is:                       r(t) = exp( j*phi(t) )
     Complex amplitude is estimated as:  c(t)=LPF[s(t)*conjugate(r(t))]
     so c(t) has phase angle theta-phi
     Multiply r(t) by c(t) and subtract from s(t), i.e. s'(t)=s(t)-c(t)r(t)
     *******************************************************************************/

    // create reference wspr signal vector, centered on f0.
    //
    for (i=0; i<nsym; i++) {

        cs=(float)channel_symbols[i];

        dphi=twopidt*
        (
         f0 + (drift0/2.0)*((float)i-(float)nsym/2.0)/((float)nsym/2.0)
         + (cs-1.5)*df
         );

        for ( j=0; j<nspersym; j++ ) {
            ii=nspersym*i+j;
            refi[ii]=cos(phi); //cannot precompute sin/cos because dphi is changing
            refq[ii]=sin(phi);
            phi=phi+dphi;
        }
    }

    // s(t) * conjugate(r(t))
    // beginning of first symbol in reference signal is at i=0
    // beginning of first symbol in received data is at shift0.
    // filter transient lasts nfilt samples
    // leave nfilt zeros as a pad at the beginning of the unfiltered reference signal
    for (i=0; i<nsym*nspersym; i++) {
        k=shift0+i;
        if( (k>0) && (k<np) ) {
            ci[i+nfilt] = id[k]*refi[i] + qd[k]*refq[i];
            cq[i+nfilt] = qd[k]*refi[i] - id[k]*refq[i];
        }
    }

    //lowpass filter and remove startup transient
    float w[nfilt], norm=0, partialsum[nfilt];
    memset(partialsum,0,sizeof(float)*nfilt);
    for (i=0; i<nfilt; i++) {
        w[i]=sin(pi*(float)i/(float)(nfilt-1));
        norm=norm+w[i];
    }
    for (i=0; i<nfilt; i++) {
        w[i]=w[i]/norm;
    }
    for (i=1; i<nfilt; i++) {
        partialsum[i]=partialsum[i-1]+w[i];
    }

    // LPF
    for (i=nfilt/2; i<45000-nfilt/2; i++) {
        cfi[i]=0.0; cfq[i]=0.0;
        for (j=0; j<nfilt; j++) {
            cfi[i]=cfi[i]+w[j]*ci[i-nfilt/2+j];
            cfq[i]=cfq[i]+w[j]*cq[i-nfilt/2+j];
        }
    }

    // subtract c(t)*r(t) here
    // (ci+j*cq)(refi+j*refq)=(ci*refi-cq*refq)+j(ci*refq)+cq*refi)
    // beginning of first symbol in reference signal is at i=nfilt
    // beginning of first symbol in received data is at shift0.
    for (i=0; i<nsig; i++) {
        if( i<nfilt/2 ) {        // take care of the end effect (LPF step response) here
            norm=partialsum[nfilt/2+i];
        } else if( i>(nsig-1-nfilt/2) ) {
            norm=partialsum[nfilt/2+nsig-1-i];
        } else {
            norm=1.0;
        }
        k=shift0+i;
        j=i+nfilt;
        if( (k>0) && (k<np) ) {
            id[k]=id[k] - (cfi[j]*refi[i]-cfq[j]*refq[i])/norm;
            qd[k]=qd[k] - (cfi[j]*refq[i]+cfq[j]*refi[i])/norm;
        }
    }

    free(refi);
    free(refq);
    free(ci);
    free(cq);
    free(cfi);
    free(cfq);

    return;
}

unsigned long writec2file(char *c2filename, int trmin, double freq
                          , float *idat, float *qdat)
{
    int i;
    float *buffer;
    buffer=malloc(sizeof(float)*2*45000);
    memset(buffer,0,sizeof(float)*2*45000);

    FILE *fp;

    fp = fopen(c2filename,"wb");
    if( fp == NULL ) {
        fprintf(stderr, "Could not open c2 file '%s'\n", c2filename);
        free(buffer);
        return 0;
    }
    unsigned long nwrite = fwrite(c2filename,sizeof(char),14,fp);
    nwrite = fwrite(&trmin, sizeof(int), 1, fp);
    nwrite = fwrite(&freq, sizeof(double), 1, fp);

    for(i=0; i<45000; i++) {
        buffer[2*i]=idat[i];
        buffer[2*i+1]=-qdat[i];
    }

    nwrite = fwrite(buffer, sizeof(float), 2*45000, fp);
    if( nwrite == 2*45000 ) {
        return nwrite;
    } else {
        free(buffer);
        return 0;
    }
}

//***************************************************************************
void usage(void)
{
    printf("Usage: wsprd [options...] infile\n");
    printf("       infile must have suffix .wav or .c2\n");
    printf("\n");
    printf("Options:\n");
    printf("       -a <path> path to writeable data files, default=\".\"\n");
    printf("       -c write .c2 file at the end of the first pass\n");
    printf("       -C maximum number of decoder cycles per bit, default 10000\n");
    printf("       -d deeper search. Slower, a few more decodes\n");
    printf("       -e x (x is transceiver dial frequency error in Hz)\n");
    printf("       -f x (x is transceiver dial frequency in MHz)\n");
    printf("       -H do not use (or update) the hash table\n");
    printf("       -J use the stack decoder instead of Fano decoder\n");
    printf("       -m decode wspr-15 .wav file\n");
    printf("       -q quick mode - doesn't dig deep for weak signals\n");
    printf("       -s single pass mode, no subtraction (same as original wsprd)\n");
    printf("       -v verbose mode (shows dupes)\n");
    printf("       -w wideband mode - decode signals within +/- 150 Hz of center\n");
    printf("       -z x (x is fano metric table bias, default is 0.45)\n");
}

//***************************************************************************
int main(int argc, char *argv[])
{
    char cr[] = "(C) 2016, Steven Franke - K9AN";
    (void)cr;
    extern char *optarg;
    extern int optind;
    int i,j,k;
    unsigned char *symbols, *decdata, *channel_symbols;
    signed char message[]={-9,13,-35,123,57,-39,64,0,0,0,0};
    char *callsign, *call_loc_pow;
    char *ptr_to_infile,*ptr_to_infile_suffix;
    char *data_dir=NULL;
    char wisdom_fname[200],all_fname[200],spots_fname[200];
    char timer_fname[200],hash_fname[200];
    char uttime[5],date[7];
    int c,delta,maxpts=65536,verbose=0,quickmode=0,more_candidates=0, stackdecoder=0;
    int writenoise=0,usehashtable=1,wspr_type=2, ipass;
    int writec2=0, npasses=2, subtraction=1;
    int shift1, lagmin, lagmax, lagstep, ifmin, ifmax, worth_a_try, not_decoded;
    unsigned int nbits=81, stacksize=200000;
    unsigned int npoints, metric, cycles, maxnp;
    float df=375.0/256.0/2;
    float freq0[200],snr0[200],drift0[200],sync0[200];
    int shift0[200];
    float dt=1.0/375.0, dt_print;
    double dialfreq_cmdline=0.0, dialfreq, freq_print;
    double dialfreq_error=0.0;
    float fmin=-110, fmax=110;
    float f1, fstep, sync1, drift1;
    float psavg[512];
    float *idat, *qdat;
    clock_t t0,t00;
    float tfano=0.0,treadwav=0.0,tcandidates=0.0,tsync0=0.0;
    float tsync1=0.0,tsync2=0.0,ttotal=0.0;

    struct result { char date[7]; char time[5]; float sync; float snr;
                    float dt; double freq; char message[23]; float drift;
                    unsigned int cycles; int jitter; };
    struct result decodes[50];

    char *hashtab;
    hashtab=malloc(sizeof(char)*32768*13);
    memset(hashtab,0,sizeof(char)*32768*13);
    int nh;
    symbols=malloc(sizeof(char)*nbits*2);
    decdata=malloc(sizeof(char)*11);
    channel_symbols=malloc(sizeof(char)*nbits*2);

    callsign=malloc(sizeof(char)*13);
    call_loc_pow=malloc(sizeof(char)*23);
    float allfreqs[100];
    char allcalls[100][13];
    memset(allfreqs,0,sizeof(float)*100);
    memset(allcalls,0,sizeof(char)*100*13);

    int uniques=0, noprint=0, ndecodes_pass=0;

    // Parameters used for performance-tuning:
    unsigned int maxcycles=10000;            //Decoder timeout limit
    float minsync1=0.10;                     //First sync limit
    float minsync2=0.12;                     //Second sync limit
    int iifac=8;                             //Step size in final DT peakup
    int symfac=50;                           //Soft-symbol normalizing factor
    int maxdrift=4;                          //Maximum (+/-) drift
    float minrms=52.0 * (symfac/64.0);      //Final test for plausible decoding
    delta=60;                                //Fano threshold step
    float bias=0.45;                        //Fano metric bias (used for both Fano and stack algorithms)

    t00=clock();
    fftwf_complex *fftin, *fftout;
#include "./metric_tables.c"

    int mettab[2][256];

    idat=malloc(sizeof(float)*maxpts);
    qdat=malloc(sizeof(float)*maxpts);

    while ( (c = getopt(argc, argv, "a:cC:de:f:HJmqstwvz:")) !=-1 ) {
        switch (c) {
            case 'a':
                data_dir = optarg;
                break;
            case 'c':
                writec2=1;
                break;
            case 'C':
                maxcycles=(unsigned int) strtoul(optarg,NULL,10);
                break;
            case 'd':
                more_candidates=1;
                break;
            case 'e':
                dialfreq_error = strtod(optarg,NULL);   // units of Hz
                // dialfreq_error = dial reading - actual, correct frequency
                break;
            case 'f':
                dialfreq_cmdline = strtod(optarg,NULL); // units of MHz
                break;
            case 'H':
                usehashtable = 0;
                break;
            case 'J': //Stack (Jelinek) decoder, Fano decoder is the default
                stackdecoder = 1;
                break;
            case 'm':  //15-minute wspr mode
                wspr_type = 15;
                break;
            case 'q':  //no shift jittering
                quickmode = 1;
                break;
            case 's':  //single pass mode (same as original wsprd)
                subtraction = 0;
                npasses = 1;
                break;
            case 'v':
                verbose = 1;
                break;
            case 'w':
                fmin=-150.0;
                fmax=150.0;
                break;
            case 'z':
                bias=strtod(optarg,NULL); //fano metric bias (default is 0.45)
                break;
            case '?':
                usage();
                return 1;
        }
    }

    if( stackdecoder ) {
        stack=malloc(stacksize*sizeof(struct snode));
    }

    if( optind+1 > argc) {
        usage();
        return 1;
    } else {
        ptr_to_infile=argv[optind];
    }

    // setup metric table
    for(i=0; i<256; i++) {
        mettab[0][i]=round( 10*(metric_tables[2][i]-bias) );
        mettab[1][i]=round( 10*(metric_tables[2][255-i]-bias) );
    }

    FILE *fp_fftwf_wisdom_file, *fall_wspr, *fwsprd, *fhash, *ftimer;
    strcpy(wisdom_fname,".");
    strcpy(all_fname,".");
    strcpy(spots_fname,".");
    strcpy(timer_fname,".");
    strcpy(hash_fname,".");
    if(data_dir != NULL) {
        strcpy(wisdom_fname,data_dir);
        strcpy(all_fname,data_dir);
        strcpy(spots_fname,data_dir);
        strcpy(timer_fname,data_dir);
        strcpy(hash_fname,data_dir);
    }
    strncat(wisdom_fname,"/wspr_wisdom.dat",20);
    strncat(all_fname,"/ALL_WSPR.TXT",20);
    strncat(spots_fname,"/wspr_spots.txt",20);
    strncat(timer_fname,"/wspr_timer.out",20);
    strncat(hash_fname,"/hashtable.txt",20);
    if ((fp_fftwf_wisdom_file = fopen(wisdom_fname, "r"))) {  //Open FFTW wisdom
        fftwf_import_wisdom_from_file(fp_fftwf_wisdom_file);
        fclose(fp_fftwf_wisdom_file);
    }

    fall_wspr=fopen(all_fname,"a");
    fwsprd=fopen(spots_fname,"w");
    //  FILE *fdiag;
    //  fdiag=fopen("wsprd_diag","a");

    if((ftimer=fopen(timer_fname,"r"))) {
        //Accumulate timing data
        nr=fscanf(ftimer,"%f %f %f %f %f %f %f",
                  &treadwav,&tcandidates,&tsync0,&tsync1,&tsync2,&tfano,&ttotal);
        fclose(ftimer);
    }
    ftimer=fopen(timer_fname,"w");

    // read file in wave format
    if( strstr(ptr_to_infile,".wav") ) {
        ptr_to_infile_suffix=strstr(ptr_to_infile,".wav");

        t0 = clock();
        npoints=readwavfile(ptr_to_infile, wspr_type, idat, qdat);
        treadwav += (float)(clock()-t0)/CLOCKS_PER_SEC;

        if( npoints == 1 ) {
            return 1;
        }
        dialfreq=dialfreq_cmdline - (dialfreq_error*1.0e-06);
    // read file in c2 format
    } else if ( strstr(ptr_to_infile,".c2") !=0 )  {
        ptr_to_infile_suffix=strstr(ptr_to_infile,".c2");
        npoints=readc2file(ptr_to_infile, idat, qdat, &dialfreq, &wspr_type);
        if( npoints == 1 ) {
            return 1;
        }
        dialfreq -= (dialfreq_error*1.0e-06);
    } else {
        printf("Error: Failed to open %s\n",ptr_to_infile);
        printf("WSPR file must have suffix .wav or .c2\n");
        return 1;
    }

    // Parse date and time from given filename
    strncpy(date,ptr_to_infile_suffix-11,6);
    strncpy(uttime,ptr_to_infile_suffix-4,4);
    date[6]='\0';
    uttime[4]='\0';

    // --- preliminaries for one-dimensional DFT of size 512
    // Do windowed ffts over 2 symbols, stepped by half symbols
    // Note: One symbol (baud) is 256 samples. 2 symbols times
    // 256 samples/symbol is 512 samples. "npoints" divided by
    // 512 is the number 2 symbol chunks. 4 times the number of symbols
    // is the number of half symbol steps (i.e., nffts).
    int nffts=4*floor(npoints/512)-1;
    // allocate memory for FFT input array
    fftin=(fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex)*512);
    // allocate memory for FFT output array
    fftout=(fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex)*512);
    // plan allocation (create object containing data needed by FFTW)
    // 512 = size of transform
    // fftin = pointer to input array
    // fftout = pointer to output array
    // FFTW_FORWARD (-1) = direction of transform (sign of exponent in transform)
    // PATIENCE = flags
    PLAN3 = fftwf_plan_dft_1d(512, fftin, fftout, FFTW_FORWARD, PATIENCE);

    float ps[512][nffts]; // power spectrum
    // generate a windowing function (half-sine wave cycle [0 pi]), in 512 steps
    float w[512];
    for(i=0; i<512; i++) {
        w[i]=sin(0.006147931*i);
    }

    // read hash table
    if( usehashtable ) {
        char line[80], hcall[12];
        if( (fhash=fopen(hash_fname,"r+")) ) {
            while (fgets(line, sizeof(line), fhash) != NULL) {
                sscanf(line,"%d %s",&nh,hcall);
                strcpy(hashtab+nh*13,hcall);
            }
        } else {
            fhash=fopen(hash_fname,"w+");
        }
        fclose(fhash);
    }

    // timing
    struct timeval start, end;
    gettimeofday(&start, NULL);

    //*************** main loop starts here *****************
    for (ipass=0; ipass<npasses; ipass++) {
        // printf("Pass index : %d\n",ipass);
        // one pass
        if( ipass > 0 && ndecodes_pass == 0 ) break;
        ndecodes_pass=0;

        memset(ps,0.0, sizeof(float)*512*nffts);
        for (i=0; i<nffts; i++) { // for each half symbol
            // i = index of FFT
            // prepare a FFT input (of size 512 in-phase, quadrature samples)
            for(j=0; j<512; j++ ) {
                // FFT inputs are stored sequentially, in blocks 512 in-phase,
                // quadrature samples (represented as complex number).
                // Each FFT is over two symbols (512 samples). From FFT-to-FFT,
                // there is a half symbol step in the signal (128 samples).
                // The samples are filtered with a sine filter.
                k=i*128+j;
                fftin[j][0]=idat[k] * w[j];
                fftin[j][1]=qdat[k] * w[j];
            }
            // Compute one-dimensional DFT using PLAN3
            //  results are stored in-order in the array out,
            // with the zero-frequency (DC) component in fftout[0]
            fftwf_execute(PLAN3);
            // computer the "RMS" for each of the 512 frequency bins
            for (j=0; j<512; j++ ) {
                k=j+256;
                if( k>511 )
                    k=k-512;
                // Calculate sum of the square of in-phase and square of
                // quadrature.
                // Results are stored in matrix "ps" (each row is a FFT, each
                // column is a frequency).
                ps[j][i]=fftout[k][0]*fftout[k][0]+fftout[k][1]*fftout[k][1];
            }
        }

        // Compute average spectrum for each frequency.
        // Result is stored in array "psavg", one cell for each frequency
        memset(psavg,0.0, sizeof(float)*512);
        for (i=0; i<nffts; i++) {
            for (j=0; j<512; j++) {
                psavg[j]=psavg[j]+ps[j][i];
            }
        }

        // Smooth the average power ith 7-point window and limit
        // spectrum to +/-150 Hz.
        // Max frequency for each FFT is 375 sps/2 = 187.5.
        // 411 bins/512 bins * 187.Hz = 150 Hz.
        int window[7]={1,1,1,1,1,1,1};
        float smspec[411];
        for (i=0; i<411; i++) { // for each bin
            smspec[i]=0.0;
            for(j=-3; j<=3; j++) {
                // start from FFT bin 256-205=51.
                // 51 bins/512 bins * 187.Hz = 18.7 Hz
                k=256-205+i+j;
                smspec[i]=smspec[i]+window[j+3]*psavg[k];
            }
        }

        // Sort spectrum values, then pick off noise level as a percentile
        float tmpsort[411];
        for (j=0; j<411; j++) {
            tmpsort[j]=smspec[j];
        }
        qsort(tmpsort, 411, sizeof(float), floatcomp);

        // Noise level of spectrum is estimated as 123/411= 30'th percentile
        float noise_level = tmpsort[122];

        /* Renormalize spectrum so that (large) peaks represent an estimate of snr.
         * We know from experience that threshold snr is near -7dB in wspr bandwidth,
         * corresponding to -7-26.3=-33.3dB in 2500 Hz bandwidth.
         * The corresponding threshold is -42.3 dB in 2500 Hz bandwidth for WSPR-15. */

        float min_snr, snr_scaling_factor;
        min_snr = pow(10.0,-7.0/10.0); //this is min snr in wspr bw
        if( wspr_type == 2 ) {
            snr_scaling_factor=26.3;
        } else {
            snr_scaling_factor=35.3;
        }
        for (j=0; j<411; j++) {
            smspec[j]=smspec[j]/noise_level - 1.0;
            if( smspec[j] < min_snr) smspec[j]=0.1*min_snr;
            continue;
        }

        // Find all local maxima in smoothed spectrum.
        for (i=0; i<200; i++) {
            freq0[i]=0.0;
            snr0[i]=0.0;
            drift0[i]=0.0;
            shift0[i]=0;
            sync0[i]=0.0;
        }

        int npk=0;
        unsigned char candidate;
        if( more_candidates ) {
            for(j=0; j<411; j=j+2) {
                candidate = (smspec[j]>min_snr) && (npk<200);
                if ( candidate ) {
                    freq0[npk]=(j-205)*df;
                    snr0[npk]=10*log10(smspec[j])-snr_scaling_factor;
                    npk++;
                }
            }
        } else {
            for(j=1; j<410; j++) {
                candidate = (smspec[j]>smspec[j-1]) &&
                            (smspec[j]>smspec[j+1]) &&
                            (npk<200);
                if ( candidate ) {
                    freq0[npk]=(j-205)*df;
                    snr0[npk]=10*log10(smspec[j])-snr_scaling_factor;
                    npk++;
                }
            }
        }

        // Compute corrected fmin, fmax, accounting for dial frequency error
        fmin += dialfreq_error;    // dialfreq_error is in units of Hz
        fmax += dialfreq_error;

        // Don't waste time on signals outside of the range [fmin,fmax].
        i=0;
        for( j=0; j<npk; j++) {
            if( freq0[j] >= fmin && freq0[j] <= fmax ) {
                freq0[i]=freq0[j];
                snr0[i]=snr0[j];
                i++;
            }
        }
        npk=i;

        // bubble sort on snr, bringing freq along for the ride
        int pass;
        float tmp;
        for (pass = 1; pass <= npk - 1; pass++) {
            for (k = 0; k < npk - pass ; k++) {
                if (snr0[k] < snr0[k+1]) {
                    tmp = snr0[k];
                    snr0[k] = snr0[k+1];
                    snr0[k+1] = tmp;
                    tmp = freq0[k];
                    freq0[k] = freq0[k+1];
                    freq0[k+1] = tmp;
                }
            }
        }

        t0=clock();

        /* Make coarse estimates of shift (DT), freq, and drift

         * Look for time offsets up to +/- 8 symbols (about +/- 5.4 s) relative
         to nominal start time, which is 2 seconds into the file

         * Calculates shift relative to the beginning of the file

         * Negative shifts mean that signal started before start of file

         * The program prints DT = shift-2 s

         * Shifts that cause sync vector to fall off of either end of the data
         vector are accommodated by "partial decoding", such that missing
         symbols produce a soft-decision symbol value of 128

         * The frequency drift model is linear, deviation of +/- drift/2 over the
         span of 162 symbols, with deviation equal to 0 at the center of the
         signal vector.
         */

        int idrift,ifr,if0,ifd,k0;
        int kindex;
        float smax,ss,pow,p0,p1,p2,p3;
        for(j=0; j<npk; j++) {                              //For each candidate...
            smax=-1e30;
            if0=freq0[j]/df+256;
            for (ifr=if0-2; ifr<=if0+2; ifr++) {                      //Freq search
                // search in the interval [-10 22] half-symbols,
                // i.e., [-5 11] symbols
                for( k0=-10; k0<22; k0++) {                             //Time search
                    for (idrift=-maxdrift; idrift<=maxdrift; idrift++) {  //Drift search
                        ss=0.0;
                        pow=0.0;
                        for (k=0; k<162; k++) {                             //Sum over symbols
                            ifd=ifr+((float)k-81.0)/81.0*( (float)idrift )/(2.0*df);
                            kindex=k0+2*k;

                            if( kindex < nffts ) {
                                // BUG: when kindex is lower than zero, should
                                // not be used
                                p0=ps[ifd-3][kindex];
                                p1=ps[ifd-1][kindex];
                                p2=ps[ifd+1][kindex];
                                p3=ps[ifd+3][kindex];

                                p0=sqrt(p0);
                                p1=sqrt(p1);
                                p2=sqrt(p2);
                                p3=sqrt(p3);

                                ss=ss+(2*pr3[k]-1)*((p1+p3)-(p0+p2));
                                pow=pow+p0+p1+p2+p3;
                            }
                        }
                        sync1=ss/pow;
                        if( sync1 > smax ) {                  //Save coarse parameters
                            smax=sync1;
                            shift0[j]=128*(k0+1);
                            drift0[j]=idrift;
                            freq0[j]=(ifr-256)*df;
                            sync0[j]=sync1;
                        }
                    }
                }
            }
        }
        tcandidates += (float)(clock()-t0)/CLOCKS_PER_SEC;

        /*
         Refine the estimates of freq, shift using sync as a metric.
         Sync is calculated such that it is a float taking values in the range
         [0.0,1.0].

         Function sync_and_demodulate has three modes of operation
         mode is the last argument:

         0 = no frequency or drift search. find best time lag.
         1 = no time lag or drift search. find best frequency.
         2 = no frequency or time lag search. Calculate soft-decision
         symbols using passed frequency and shift.

         NB: best possibility for OpenMP may be here: several worker threads
         could each work on one candidate at a time.
         */
        for (j=0; j<npk; j++) {
            memset(symbols,0,sizeof(char)*nbits*2);
            memset(callsign,0,sizeof(char)*13);
            memset(call_loc_pow,0,sizeof(char)*23);

            f1=freq0[j];
            drift1=drift0[j];
            shift1=shift0[j];
            sync1=sync0[j];


            // coarse-grid lag and freq search, then if sync>minsync1 continue
            fstep=0.0; ifmin=0; ifmax=0;
            lagmin=shift1-128;
            lagmax=shift1+128;
            lagstep=64;
            t0 = clock();
            sync_and_demodulate(idat, qdat, npoints, symbols, &f1, ifmin, ifmax, fstep, &shift1,
                                lagmin, lagmax, lagstep, &drift1, symfac, &sync1, 0);
            tsync0 += (float)(clock()-t0)/CLOCKS_PER_SEC;

            fstep=0.25; ifmin=-2; ifmax=2;
            t0 = clock();
            sync_and_demodulate(idat, qdat, npoints, symbols, &f1, ifmin, ifmax, fstep, &shift1,
                                lagmin, lagmax, lagstep, &drift1, symfac, &sync1, 1);

            // refine drift estimate
            fstep=0.0; ifmin=0; ifmax=0;
            float driftp,driftm,syncp,syncm;
            driftp=drift1+0.5;
            sync_and_demodulate(idat, qdat, npoints, symbols, &f1, ifmin, ifmax, fstep, &shift1,
                                lagmin, lagmax, lagstep, &driftp, symfac, &syncp, 1);

            driftm=drift1-0.5;
            sync_and_demodulate(idat, qdat, npoints, symbols, &f1, ifmin, ifmax, fstep, &shift1,
                                lagmin, lagmax, lagstep, &driftm, symfac, &syncm, 1);

            if(syncp>sync1) {
                drift1=driftp;
                sync1=syncp;
            } else if (syncm>sync1) {
                drift1=driftm;
                sync1=syncm;
            }

            tsync1 += (float)(clock()-t0)/CLOCKS_PER_SEC;

            // fine-grid lag and freq search
            if( sync1 > minsync1 ) {

                lagmin=shift1-32; lagmax=shift1+32; lagstep=16;
                t0 = clock();
                sync_and_demodulate(idat, qdat, npoints, symbols, &f1, ifmin, ifmax, fstep, &shift1,
                                    lagmin, lagmax, lagstep, &drift1, symfac, &sync1, 0);
                tsync0 += (float)(clock()-t0)/CLOCKS_PER_SEC;

                // fine search over frequency
                fstep=0.05; ifmin=-2; ifmax=2;
                t0 = clock();
                sync_and_demodulate(idat, qdat, npoints, symbols, &f1, ifmin, ifmax, fstep, &shift1,
                                lagmin, lagmax, lagstep, &drift1, symfac, &sync1, 1);
                tsync1 += (float)(clock()-t0)/CLOCKS_PER_SEC;

                worth_a_try = 1;
            } else {
                worth_a_try = 0;
            }

            int idt=0, ii=0, jiggered_shift;
            float y,sq,rms;
            not_decoded=1;

            while ( worth_a_try && not_decoded && idt<=(128/iifac)) {
                ii=(idt+1)/2;
                if( idt%2 == 1 ) ii=-ii;
                ii=iifac*ii;
                jiggered_shift=shift1+ii;

                // Use mode 2 to get soft-decision symbols
                t0 = clock();
                sync_and_demodulate(idat, qdat, npoints, symbols, &f1, ifmin, ifmax, fstep,
                                    &jiggered_shift, lagmin, lagmax, lagstep, &drift1, symfac,
                                    &sync1, 2);
                tsync2 += (float)(clock()-t0)/CLOCKS_PER_SEC;

                sq=0.0;
                for(i=0; i<162; i++) {
                    y=(float)symbols[i] - 128.0;
                    sq += y*y;
                }
                rms=sqrt(sq/162.0);

                if((sync1 > minsync2) && (rms > minrms)) {
                    deinterleave(symbols);
                    t0 = clock();

                    if ( stackdecoder ) {
                        not_decoded = jelinek(&metric, &cycles, decdata, symbols, nbits,
                                              stacksize, stack, mettab,maxcycles);
                    } else {
                        not_decoded = fano(&metric,&cycles,&maxnp,decdata,symbols,nbits,
                                           mettab,delta,maxcycles);
                    }

                    tfano += (float)(clock()-t0)/CLOCKS_PER_SEC;

                }
                idt++;
                if( quickmode ) break;
            }

            if( worth_a_try && !not_decoded ) {
                ndecodes_pass++;

                for(i=0; i<11; i++) {

                    if( decdata[i]>127 ) {
                        message[i]=decdata[i]-256;
                    } else {
                        message[i]=decdata[i];
                    }

                }

                // Unpack the decoded message, update the hashtable, apply
                // sanity checks on grid and power, and return
                // call_loc_pow string and also callsign (for de-duping).
                noprint=unpk_(message,hashtab,call_loc_pow,callsign);

                // subtract even on last pass
                if( subtraction && (ipass < npasses ) && !noprint ) {
                    if( get_wspr_channel_symbols(call_loc_pow, hashtab, channel_symbols) ) {
                        subtract_signal2(idat, qdat, npoints, f1, shift1, drift1, channel_symbols);
                    } else {
                        break;
                    }

                }

                // Remove dupes (same callsign and freq within 3 Hz)
                int dupe=0;
                for (i=0; i<uniques; i++) {
                    if(!strcmp(callsign,allcalls[i]) &&
                       (fabs(f1-allfreqs[i]) <3.0)) dupe=1;
                }
                if( (verbose || !dupe) && !noprint) {
                    strcpy(allcalls[uniques],callsign);
                    allfreqs[uniques]=f1;
                    uniques++;

                    // Add an extra space at the end of each line so that wspr-x doesn't
                    // truncate the power (TNX to DL8FCL!)

                    if( wspr_type == 15 ) {
                        freq_print=dialfreq+(1500+112.5+f1/8.0)/1e6;
                        dt_print=shift1*8*dt-1.0;
                    } else {
                        freq_print=dialfreq+(1500+f1)/1e6;
                        dt_print=shift1*dt-1.0;
                    }

                    strcpy(decodes[uniques-1].date,date);
                    strcpy(decodes[uniques-1].time,uttime);
                    decodes[uniques-1].sync=sync1;
                    decodes[uniques-1].snr=snr0[j];
                    decodes[uniques-1].dt=dt_print;
                    decodes[uniques-1].freq=freq_print;
                    strcpy(decodes[uniques-1].message,call_loc_pow);
                    decodes[uniques-1].drift=drift1;
                    decodes[uniques-1].cycles=cycles;
                    decodes[uniques-1].jitter=ii;
                }
            }
        }

        if( ipass == 0 && writec2 ) {
            char c2filename[15];
            double carrierfreq=dialfreq;
            int wsprtype=2;
            strcpy(c2filename,"000000_0001.c2");
            printf("Writing %s\n",c2filename);
            writec2file(c2filename, wsprtype, carrierfreq, idat, qdat);
        }
    }

    // sort the result in order of increasing frequency
    struct result temp;
    for (j = 1; j <= uniques - 1; j++) {
        for (k = 0; k < uniques - j ; k++) {
            if (decodes[k].freq > decodes[k+1].freq) {
                temp = decodes[k];
                decodes[k]=decodes[k+1];;
                decodes[k+1] = temp;
            }
        }
    }

    for (i=0; i<uniques; i++) {
        printf("%4s %3.0f %4.1f %10.6f %2d  %-s \n",
               decodes[i].time, decodes[i].snr,decodes[i].dt, decodes[i].freq,
               (int)decodes[i].drift, decodes[i].message);
        fprintf(fall_wspr,
                "%6s %4s %3d %3.0f %5.2f %11.7f  %-22s %2d %5u %4d\n",
                decodes[i].date, decodes[i].time, (int)(10*decodes[i].sync),
                decodes[i].snr, decodes[i].dt, decodes[i].freq,
                decodes[i].message, (int)decodes[i].drift, decodes[i].cycles/81,
                decodes[i].jitter);
        fprintf(fwsprd,
                "%6s %4s %3d %3.0f %4.1f %10.6f  %-22s %2d %5u %4d\n",
                decodes[i].date, decodes[i].time, (int)(10*decodes[i].sync),
                decodes[i].snr, decodes[i].dt, decodes[i].freq,
                decodes[i].message, (int)decodes[i].drift, decodes[i].cycles/81,
                decodes[i].jitter);

    }

    gettimeofday(&end, NULL);
    float deltatime = ((end.tv_sec  - start.tv_sec) * 1000000u +
         end.tv_usec - start.tv_usec) / 1.e6;
    printf("Decoded in : %.2f seconds\n",deltatime);
    //printf("%.2f ",deltatime);

    fftwf_free(fftin);
    fftwf_free(fftout);

    if ((fp_fftwf_wisdom_file = fopen(wisdom_fname, "w"))) {
        fftwf_export_wisdom_to_file(fp_fftwf_wisdom_file);
        fclose(fp_fftwf_wisdom_file);
    }

    ttotal += (float)(clock()-t00)/CLOCKS_PER_SEC;

    fprintf(ftimer,"%7.2f %7.2f %7.2f %7.2f %7.2f %7.2f %7.2f\n\n",
            treadwav,tcandidates,tsync0,tsync1,tsync2,tfano,ttotal);

    fprintf(ftimer,"Code segment        Seconds   Frac\n");
    fprintf(ftimer,"-----------------------------------\n");
    fprintf(ftimer,"readwavfile        %7.2f %7.2f\n",treadwav,treadwav/ttotal);
    fprintf(ftimer,"Coarse DT f0 f1    %7.2f %7.2f\n",tcandidates,
            tcandidates/ttotal);
    fprintf(ftimer,"sync_and_demod(0)  %7.2f %7.2f\n",tsync0,tsync0/ttotal);
    fprintf(ftimer,"sync_and_demod(1)  %7.2f %7.2f\n",tsync1,tsync1/ttotal);
    fprintf(ftimer,"sync_and_demod(2)  %7.2f %7.2f\n",tsync2,tsync2/ttotal);
    fprintf(ftimer,"Stack/Fano decoder %7.2f %7.2f\n",tfano,tfano/ttotal);
    fprintf(ftimer,"-----------------------------------\n");
    fprintf(ftimer,"Total              %7.2f %7.2f\n",ttotal,1.0);

    fclose(fall_wspr);
    fclose(fwsprd);
    //  fclose(fdiag);
    fclose(ftimer);
    fftwf_destroy_plan(PLAN1);
    fftwf_destroy_plan(PLAN2);
    fftwf_destroy_plan(PLAN3);

    if( usehashtable ) {
        fhash=fopen(hash_fname,"w");
        for (i=0; i<32768; i++) {
            if( strncmp(hashtab+i*13,"\0",1) != 0 ) {
                fprintf(fhash,"%5d %s\n",i,hashtab+i*13);
            }
        }
        fclose(fhash);
    }

    if( stackdecoder ) {
        free(stack);
    }

    if(writenoise == 999) return -1;  //Silence compiler warning
    return 0;
}