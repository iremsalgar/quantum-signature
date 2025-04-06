# quantum-signature

## değiştirilen fonksiyonlar

1- void ntt(int32_t a[N])
2-void poly_pointwise_montgomery(poly *c, const poly *a, const poly *b) 
3-void poly_uniform_eta(poly *a, const uint8_t seed[CRHBYTES], uint16_t nonce)
´´´
#include <arm_neon.h>
#include <arm_acle.h>

/*************************************************
* Name:        poly_uniform_eta
*
* Description: NEON ve ARMv8 AES ile optimize edilmiş Gaussian örnekleyici.
*              Katsayılar [-ETA, ETA] aralığında.
**************************************************/
#if ETA == 2
#define POLY_UNIFORM_ETA_NBLOCKS ((136 + STREAM256_BLOCKBYTES - 1)/STREAM256_BLOCKBYTES)
#elif ETA == 4
#define POLY_UNIFORM_ETA_NBLOCKS ((227 + STREAM256_BLOCKBYTES - 1)/STREAM256_BLOCKBYTES)
#endif

typedef struct {
    uint8x16_t key;
    uint8x16_t nonce;
} neon_stream256_state;

void neon_stream256_init(neon_stream256_state *state, const uint8_t seed[CRHBYTES], uint16_t nonce) {
    // Anahtar ve nonce'u 128-bit vektörlere yükle
    state->key = vld1q_u8(seed);
    state->nonce = vreinterpretq_u8_u16(vdupq_n_u16(nonce));
}

void neon_stream256_squeezeblocks(uint8_t *out, unsigned int nblocks, neon_stream256_state *state) {
    for (unsigned int i = 0; i < nblocks; ++i) {
        // AES-CTR modunda 16 baytlık blok üret
        uint8x16_t block = vaesmcq_u8(vaeseq_u8(state->nonce, state->key));
        vst1q_u8(out + i*STREAM256_BLOCKBYTES, block);
        state->nonce = vaddq_u8(state->nonce, vdupq_n_u8(1));
    }
}

static unsigned int neon_rej_eta(int32_t *a, unsigned int len, const uint8_t *buf) {
    uint8x16_t mask_15 = vdupq_n_u8(15);
    unsigned int ctr = 0;

    // 16 baytlık blokları NEON ile işle
    for (unsigned int pos = 0; pos + 16 <= STREAM256_BLOCKBYTES && ctr + 32 <= len; pos += 16) {
        uint8x16_t bytes = vld1q_u8(buf + pos);
        
        // 4-bit parçaları ayır
        uint8x16_t t0 = vandq_u8(bytes, mask_15);
        uint8x16_t t1 = vshrq_n_u8(bytes, 4);

        // ETA=2 için koşullu maskeler
        uint8x16_t cmp_t0 = vcltq_u8(t0, vdupq_n_u8(15));
        uint8x16_t cmp_t1 = vcltq_u8(t1, vdupq_n_u8(15));

        // Skaler değerlere dönüştür ve depola
        uint8_t t0_arr[16], t1_arr[16];
        vst1q_u8(t0_arr, t0);
        vst1q_u8(t1_arr, t1);

        for (int i = 0; i < 16; ++i) {
            if (vgetq_lane_u8(cmp_t0, i)) 
                a[ctr++] = 2 - (t0_arr[i] - (205*t0_arr[i] >> 10)*5);
            if (vgetq_lane_u8(cmp_t1, i) && ctr < len)
                a[ctr++] = 2 - (t1_arr[i] - (205*t1_arr[i] >> 10)*5);
        }
    }
    return ctr;
}

void poly_uniform_eta(poly *a, const uint8_t seed[CRHBYTES], uint16_t nonce) {
    neon_stream256_state state;
    uint8_t buf[STREAM256_BLOCKBYTES] __attribute__((aligned(16)));
    unsigned int ctr;

    // Hızlı AES tabanlı rastgele bayt üretimi
    neon_stream256_init(&state, seed, nonce);
    neon_stream256_squeezeblocks(buf, POLY_UNIFORM_ETA_NBLOCKS, &state);

    // NEON ile vektörel reddetme örneklemesi
    ctr = neon_rej_eta(a->coeffs, N, buf);

    // Kalan katsayıları doldur
    while (ctr < N) {
        neon_stream256_squeezeblocks(buf, 1, &state);
        ctr += neon_rej_eta(a->coeffs + ctr, N - ctr, buf);
    }
}
´´´
