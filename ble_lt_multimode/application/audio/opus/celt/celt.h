/********************************************************************************************************
 * @file	celt.h
 *
 * @brief	This is the header file for B91
 *
 * @author	Ble Group
 * @date	2019
 *
 * @par     Copyright (c) 2019, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
 *          All rights reserved.
 *          
 *          Redistribution and use in source and binary forms, with or without
 *          modification, are permitted provided that the following conditions are met:
 *          
 *              1. Redistributions of source code must retain the above copyright
 *              notice, this list of conditions and the following disclaimer.
 *          
 *              2. Unless for usage inside a TELINK integrated circuit, redistributions 
 *              in binary form must reproduce the above copyright notice, this list of 
 *              conditions and the following disclaimer in the documentation and/or other
 *              materials provided with the distribution.
 *          
 *              3. Neither the name of TELINK, nor the names of its contributors may be 
 *              used to endorse or promote products derived from this software without 
 *              specific prior written permission.
 *          
 *              4. This software, with or without modification, must only be used with a
 *              TELINK integrated circuit. All other usages are subject to written permission
 *              from TELINK and different commercial license may apply.
 *
 *              5. Licensee shall be solely responsible for any claim to the extent arising out of or 
 *              relating to such deletion(s), modification(s) or alteration(s).
 *         
 *          THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *          ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *          WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *          DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER BE LIABLE FOR ANY
 *          DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *          (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *          LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *          ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *          (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *          SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *         
 *******************************************************************************************************/
/* Copyright (c) 2007-2008 CSIRO
   Copyright (c) 2007-2009 Xiph.Org Foundation
   Copyright (c) 2008 Gregory Maxwell
   Written by Jean-Marc Valin and Gregory Maxwell */
/**
  @file celt.h
  @brief Contains all the functions for encoding and decoding audio
 */

/*
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CELT_H
#define CELT_H

#include "opus_types.h"
#include "opus_defines.h"
#include "opus_custom.h"
#include "entenc.h"
#include "entdec.h"
#include "arch.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CELTEncoder OpusCustomEncoder
#define CELTDecoder OpusCustomDecoder
#define CELTMode OpusCustomMode

#define LEAK_BANDS 19

typedef struct {
   int valid;
   float tonality;
   float tonality_slope;
   float noisiness;
   float activity;
   float music_prob;
   float music_prob_min;
   float music_prob_max;
   int   bandwidth;
   float activity_probability;
   float max_pitch_ratio;
   /* Store as Q6 char to save space. */
   unsigned char leak_boost[LEAK_BANDS];
   unsigned char padding[ALIGN4(LEAK_BANDS)- LEAK_BANDS];
} AnalysisInfo;

typedef struct {
   int signalType;
   int offset;
} SILKInfo;

/** Encoder state
 @brief Encoder state
 */
struct OpusCustomEncoder {
   const OpusCustomMode *mode;     /**< Mode used by the encoder */
//   int channels;
//   int stream_channels;

//   int force_intra;
//   int clip;
//   int disable_pf;
//   int complexity;
//   int upsample;
//   int start, end;

//   opus_int32 bitrate;
//   int vbr;
//   int signalling;
//   int constrained_vbr;      /* If zero, VBR can do whatever it likes with the rate */
//   int loss_rate;
//   int lsb_depth;
//   int lfe;
//   int disable_inv;
//   int arch;

   /* Everything beyond this point gets cleared on a reset */
#define ENCODER_RESET_START rng

   opus_uint32 rng;
   int spread_decision;
   opus_val32 delayedIntra;
   int tonal_average;
   int lastCodedBands;
   int hf_average;
   int tapset_decision;

   int prefilter_period;
   opus_val16 prefilter_gain;
   opus_val16 padding0;
   int prefilter_tapset;
#ifdef RESYNTH
   int prefilter_period_old;
   opus_val16 prefilter_gain_old;
   opus_val16 padding1;
   int prefilter_tapset_old;
#endif
   int consec_transient;
   AnalysisInfo analysis;
//   SILKInfo silk_info;

   opus_val32 preemph_memE[2];
   opus_val32 preemph_memD[2];

   /* VBR-related parameters */
#if(OPUS_ENC_USE_VBR)   
   opus_int32 vbr_reservoir;
   opus_int32 vbr_drift;
   opus_int32 vbr_offset;
   opus_int32 vbr_count;
#endif   
   opus_val32 overlap_max;
   opus_val16 stereo_saving;
   opus_val16 spec_avg;
   int intensity;
   opus_val16 *energy_mask;

#ifdef RESYNTH
   /* +MAX_PERIOD/2 to make space for overlap */
   celt_sig syn_mem[2][2*MAX_PERIOD+MAX_PERIOD/2];
#endif

   celt_sig in_mem[1]; /* Size = channels*mode->overlap */
   /* celt_sig prefilter_mem[],  Size = channels*COMBFILTER_MAXPERIOD */
   /* opus_val16 oldBandE[],     Size = channels*mode->nbEBands */
   /* opus_val16 oldLogE[],      Size = channels*mode->nbEBands */
   /* opus_val16 oldLogE2[],     Size = channels*mode->nbEBands */
   /* opus_val16 energyError[],  Size = channels*mode->nbEBands */
};


#define __celt_check_mode_ptr_ptr(ptr) ((ptr) + ((ptr) - (const CELTMode**)(ptr)))

#define __celt_check_analysis_ptr(ptr) ((ptr) + ((ptr) - (const AnalysisInfo*)(ptr)))

#define __celt_check_silkinfo_ptr(ptr) ((ptr) + ((ptr) - (const SILKInfo*)(ptr)))

/* Encoder/decoder Requests */


#define CELT_SET_PREDICTION_REQUEST    10002
/** Controls the use of interframe prediction.
    0=Independent frames
    1=Short term interframe prediction allowed
    2=Long term prediction allowed
 */
#define CELT_SET_PREDICTION(x) CELT_SET_PREDICTION_REQUEST, __opus_check_int(x)

#define CELT_SET_INPUT_CLIPPING_REQUEST    10004
#define CELT_SET_INPUT_CLIPPING(x) CELT_SET_INPUT_CLIPPING_REQUEST, __opus_check_int(x)

#define CELT_GET_AND_CLEAR_ERROR_REQUEST   10007
#define CELT_GET_AND_CLEAR_ERROR(x) CELT_GET_AND_CLEAR_ERROR_REQUEST, __opus_check_int_ptr(x)

#define CELT_SET_CHANNELS_REQUEST    10008
#define CELT_SET_CHANNELS(x) CELT_SET_CHANNELS_REQUEST, __opus_check_int(x)


/* Internal */
#define CELT_SET_START_BAND_REQUEST    10010
#define CELT_SET_START_BAND(x) CELT_SET_START_BAND_REQUEST, __opus_check_int(x)

#define CELT_SET_END_BAND_REQUEST    10012
#define CELT_SET_END_BAND(x) CELT_SET_END_BAND_REQUEST, __opus_check_int(x)

#define CELT_GET_MODE_REQUEST    10015
/** Get the CELTMode used by an encoder or decoder */
#define CELT_GET_MODE(x) CELT_GET_MODE_REQUEST, __celt_check_mode_ptr_ptr(x)

#define CELT_SET_SIGNALLING_REQUEST    10016
#define CELT_SET_SIGNALLING(x) CELT_SET_SIGNALLING_REQUEST, __opus_check_int(x)

#define CELT_SET_TONALITY_REQUEST    10018
#define CELT_SET_TONALITY(x) CELT_SET_TONALITY_REQUEST, __opus_check_int(x)
#define CELT_SET_TONALITY_SLOPE_REQUEST    10020
#define CELT_SET_TONALITY_SLOPE(x) CELT_SET_TONALITY_SLOPE_REQUEST, __opus_check_int(x)

#define CELT_SET_ANALYSIS_REQUEST    10022
#define CELT_SET_ANALYSIS(x) CELT_SET_ANALYSIS_REQUEST, __celt_check_analysis_ptr(x)

#define OPUS_SET_LFE_REQUEST    10024
#define OPUS_SET_LFE(x) OPUS_SET_LFE_REQUEST, __opus_check_int(x)

#define OPUS_SET_ENERGY_MASK_REQUEST    10026
#define OPUS_SET_ENERGY_MASK(x) OPUS_SET_ENERGY_MASK_REQUEST, __opus_check_val16_ptr(x)

#define CELT_SET_SILK_INFO_REQUEST    10028
#define CELT_SET_SILK_INFO(x) CELT_SET_SILK_INFO_REQUEST, __celt_check_silkinfo_ptr(x)

/* Encoder stuff */

int celt_encoder_get_size(int channels);

int celt_encode_with_ec(OpusCustomEncoder * OPUS_RESTRICT st, const opus_val16 * pcm, int frame_size, unsigned char *compressed, int nbCompressedBytes, ec_enc *enc);

int celt_encoder_init(CELTEncoder *st, opus_int32 sampling_rate, int channels,
                      int arch);



/* Decoder stuff */

int celt_decoder_get_size(int channels);


int celt_decoder_init(CELTDecoder *st, opus_int32 sampling_rate, int channels);

int celt_decode_with_ec(OpusCustomDecoder * OPUS_RESTRICT st, const unsigned char *data,
      int len, opus_val16 * OPUS_RESTRICT pcm, int frame_size, ec_dec *dec, int accum);

#define celt_encoder_ctl opus_custom_encoder_ctl
#define celt_decoder_ctl opus_custom_decoder_ctl


#ifdef CUSTOM_MODES
#define OPUS_CUSTOM_NOSTATIC
#else
#define OPUS_CUSTOM_NOSTATIC static OPUS_INLINE
#endif

static const unsigned char trim_icdf[11] = {126, 124, 119, 109, 87, 41, 19, 9, 4, 2, 0};
/* Probs: NONE: 21.875%, LIGHT: 6.25%, NORMAL: 65.625%, AGGRESSIVE: 6.25% */
static const unsigned char spread_icdf[4] = {25, 23, 2, 0};

static const unsigned char tapset_icdf[3]={2,1,0};

#ifdef CUSTOM_MODES
static const unsigned char toOpusTable[20] = {
      0xE0, 0xE8, 0xF0, 0xF8,
      0xC0, 0xC8, 0xD0, 0xD8,
      0xA0, 0xA8, 0xB0, 0xB8,
      0x00, 0x00, 0x00, 0x00,
      0x80, 0x88, 0x90, 0x98,
};

static const unsigned char fromOpusTable[16] = {
      0x80, 0x88, 0x90, 0x98,
      0x40, 0x48, 0x50, 0x58,
      0x20, 0x28, 0x30, 0x38,
      0x00, 0x08, 0x10, 0x18
};

static OPUS_INLINE int toOpus(unsigned char c)
{
   int ret=0;
   if (c<0xA0)
      ret = toOpusTable[c>>3];
   if (ret == 0)
      return -1;
   else
      return ret|(c&0x7);
}

static OPUS_INLINE int fromOpus(unsigned char c)
{
   if (c<0x80)
      return -1;
   else
      return fromOpusTable[(c>>3)-16] | (c&0x7);
}
#endif /* CUSTOM_MODES */

#define COMBFILTER_MAXPERIOD 1024
#define COMBFILTER_MINPERIOD 15

extern const signed char tf_select_table[4][8];

#if defined(ENABLE_HARDENING) || defined(ENABLE_ASSERTIONS)
void validate_celt_decoder(CELTDecoder *st);
#define VALIDATE_CELT_DECODER(st) validate_celt_decoder(st)
#else
#define VALIDATE_CELT_DECODER(st)
#endif

int resampling_factor(opus_int32 rate);

void celt_preemphasis(const opus_val16 * OPUS_RESTRICT pcmp, celt_sig * OPUS_RESTRICT inp,
                        int N, int CC, int upsample, const opus_val16 *coef, celt_sig *mem, int clip);

void comb_filter(opus_val32 *y, opus_val32 *x, int T0, int T1, int N,
      opus_val16 g0, opus_val16 g1, int tapset0, int tapset1,
      const opus_val16 *window, int overlap, int arch);

#ifdef NON_STATIC_COMB_FILTER_CONST_C
void comb_filter_const_c(opus_val32 *y, opus_val32 *x, int T, int N,
                         opus_val16 g10, opus_val16 g11, opus_val16 g12);
#endif

#ifndef OVERRIDE_COMB_FILTER_CONST
# define comb_filter_const(y, x, T, N, g10, g11, g12, arch) \
    ((void)(arch),comb_filter_const_c(y, x, T, N, g10, g11, g12))
#endif

void init_caps(const CELTMode *m,int *cap,int LM,int C);

#ifdef RESYNTH
void deemphasis(celt_sig *in[], opus_val16 *pcm, int N, int C, int downsample, const opus_val16 *coef, celt_sig *mem);
void celt_synthesis(const CELTMode *mode, celt_norm *X, celt_sig * out_syn[],
      opus_val16 *oldBandE, int start, int effEnd, int C, int CC, int isTransient,
      int LM, int downsample, int silence);
#endif

#ifdef __cplusplus
}
#endif

#endif /* CELT_H */
