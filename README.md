# quantum-signature

## değiştirilen fonksiyonlar

1- void ntt(int32_t a[N])
2-void poly_pointwise_montgomery(poly *c, const poly *a, const poly *b) 
3-void poly_uniform_eta(poly *a, const uint8_t seed[CRHBYTES], uint16_t nonce)


#include <arm_neon.h>
void ntt(int32_t a[N]) {
  uint32_t len, start, j, k;
  int32_t zeta, t;
  k = 0;
  for(len = 128; len > 0; len >>= 1) {
    for(start = 0; start < N; start = j + len) {
      zeta = zetas[++k];
      // NEON vektörü ile 4 katsayı aynı anda işlendi
      int32x4_t zeta_vec = vdupq_n_s32(zeta);
      for(j = start; j < start + len; j += 4) {
        int32x4_t a_j = vld1q_s32(&a[j + len]);
        int32x4_t t_vec = vqrdmulhq_s32(a_j, zeta_vec); // Montgomery çarpımı
        int32x4_t a_j_len = vsubq_s32(vld1q_s32(&a[j]), t_vec);
        vst1q_s32(&a[j + len], a_j_len);
        vst1q_s32(&a[j], vaddq_s32(vld1q_s32(&a[j]), t_vec));
      }
    }
  }
}

void poly_pointwise_montgomery(poly *c, const poly *a, const poly *b) {
  for(int i=0; i<N; i+=4) {
    int32x4_t a_vec = vld1q_s32(&a->coeffs[i]);
    int32x4_t b_vec = vld1q_s32(&b->coeffs[i]);
    int32x4_t c_vec = vqrdmulhq_s32(a_vec, b_vec); // Montgomery çarpımı
    vst1q_s32(&c->coeffs[i], c_vec);
  }
}




