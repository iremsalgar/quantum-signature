#include <stdio.h>      // printf için
#include <stddef.h>     // NULL için
#include <stdint.h>     // uint8_t için
#include <oqs/oqs.h>    // OQS_SIG ve ilgili fonksiyonlar için
#include <time.h>       // clock_gettime için
#include <string.h>     // strlen için

// Zaman ölçümü için yardımcı fonksiyon
double get_current_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

int main() {
    printf("--- Quantum Signature Project - Dilithium2 Test ---\n");

    // Dilithium 2 algoritmasını başlat
    OQS_SIG *sig = OQS_SIG_dilithium_2_new();
    if (sig == NULL) {
        printf("Dilithium 2 not supported!\n");
        return 1;
    }

    // İmzalanacak mesaj
    const char *message = "This is a test message for Dilithium2 signature.";
    size_t message_len = strlen(message);

    // Performans analizi için iterasyon sayısı
    int iterations = 10000;  // 10.000 iterasyon
    double total_keygen_time = 0.0;
    double total_sign_time = 0.0;
    double total_verify_time = 0.0;

    for (int i = 0; i < iterations; i++) {
        // Anahtar çifti oluştur
        uint8_t public_key[OQS_SIG_dilithium_2_length_public_key];
        uint8_t secret_key[OQS_SIG_dilithium_2_length_secret_key];

        double start_time = get_current_time();
        if (OQS_SIG_dilithium_2_keypair(public_key, secret_key) != OQS_SUCCESS) {
            printf("Key pair generation failed at iteration %d!\n", i);
            OQS_SIG_free(sig);
            return 1;
        }
        total_keygen_time += get_current_time() - start_time;

        // Mesajı imzala
        uint8_t signature[OQS_SIG_dilithium_2_length_signature];
        size_t signature_len;

        start_time = get_current_time();
        if (OQS_SIG_dilithium_2_sign(signature, &signature_len, (uint8_t *)message, message_len, secret_key) != OQS_SUCCESS) {
            printf("Signature generation failed at iteration %d!\n", i);
            OQS_SIG_free(sig);
            return 1;
        }
        total_sign_time += get_current_time() - start_time;

        // İmzayı doğrula
        start_time = get_current_time();
        if (OQS_SIG_dilithium_2_verify((uint8_t *)message, message_len, signature, signature_len, public_key) != OQS_SUCCESS) {
            printf("Signature verification failed at iteration %d!\n", i);
            OQS_SIG_free(sig);
            return 1;
        }
        total_verify_time += get_current_time() - start_time;
    }

    // Ortalama süreleri hesapla
    double avg_keygen_time = total_keygen_time / iterations;
    double avg_sign_time = total_sign_time / iterations;
    double avg_verify_time = total_verify_time / iterations;

    // Performans analizi sonuçlarını yazdır
    printf("\nPerformance Analysis (over %d iterations):\n", iterations);
    printf("- Average Key Generation Time: %.6f seconds\n", avg_keygen_time);
    printf("- Average Signing Time: %.6f seconds\n", avg_sign_time);
    printf("- Average Verification Time: %.6f seconds\n", avg_verify_time);
    #if defined(OQS_ENABLE_SIG_dilithium_2_avx2)
        printf("AVX2 Implementasyonu Kullanılıyor\n");
    // AArch64 Implementasyonu
    #elif defined(OQS_ENABLE_SIG_dilithium_2_aarch64)
        printf("AArch64 Optimize Edilmiş Implementasyon Kullanılıyor\n");
    // Referans Implementasyon
    #elif defined(OQS_ENABLE_SIG_dilithium_2)
        printf("Referans Implementasyon (_ref) Kullanılıyor\n");
    #else
        printf("Dilithium-2 Desteklenmiyor!\n");
    #endif
    // Belleği temizle
    OQS_SIG_free(sig);

    printf("\nAll tests completed successfully!\n");
    return 0;
}
