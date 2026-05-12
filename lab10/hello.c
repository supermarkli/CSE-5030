#include <klib.h>
#include <stdint.h>

#if defined(__riscv_vector)
#include <riscv_vector.h>
#endif

#define MATRIX_SIZE 4
#define TEST_STR_LEN 256

static int matrix_a[MATRIX_SIZE][MATRIX_SIZE] = {
    {1, 2, 3, 4},
    {5, 6, 7, 8},
    {9, 10, 11, 12},
    {13, 14, 15, 16}
};
static int vector_x[MATRIX_SIZE] = {1, 2, 3, 4};
static int result_scalar[MATRIX_SIZE] = {0};
static int result_vector[MATRIX_SIZE] = {0};
static char test_string_scalar[TEST_STR_LEN];
static char test_string_vector[TEST_STR_LEN];

#if defined(__riscv_vector)
static inline void enableVectorState(void) {
    uintptr_t mstatus_vs = (uintptr_t)3 << 9;

    asm volatile("csrs mstatus, %0" : : "r"(mstatus_vs) : "memory");
}
#endif

void matrixVectorScalar(int y[MATRIX_SIZE], const int A[MATRIX_SIZE][MATRIX_SIZE], const int x[MATRIX_SIZE]) {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        y[i] = 0;
        for (int j = 0; j < MATRIX_SIZE; j++) {
            y[i] += A[i][j] * x[j];
        }
    }
}

#if defined(__riscv_vector)
void matrixVectorVector(int y[MATRIX_SIZE], const int A[MATRIX_SIZE][MATRIX_SIZE], const int x[MATRIX_SIZE]) {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        size_t vl = __riscv_vsetvl_e32m1(MATRIX_SIZE);
        vint32m1_t vec_row = __riscv_vle32_v_i32m1(&A[i][0], vl);
        vint32m1_t vec_x = __riscv_vle32_v_i32m1(x, vl);
        vint32m1_t vec_mul = __riscv_vmul_vv_i32m1(vec_row, vec_x, vl);
        vint32m1_t zero = __riscv_vmv_s_x_i32m1(0, vl);
        vint32m1_t vec_sum = __riscv_vredsum_vs_i32m1_i32m1(vec_mul, zero, vl);

        y[i] = __riscv_vmv_x_s_i32m1_i32(vec_sum);
    }
}
#else
void matrixVectorVector(int y[MATRIX_SIZE], const int A[MATRIX_SIZE][MATRIX_SIZE], const int x[MATRIX_SIZE]) {
    matrixVectorScalar(y, A, x);
}
#endif

void initTestStrings(void) {
    strcpy(test_string_scalar, "Hello, World! This is a Test String for RISC-V Vector Extension. 123456789");
    strcpy(test_string_vector, "Hello, World! This is a Test String for RISC-V Vector Extension. 123456789");
}

void stringToUpperScalar(char* str, int len) {
    for (int i = 0; i < len; i++) {
        if (str[i] >= 'a' && str[i] <= 'z') {
            str[i] = str[i] - 32;
        }
    }
}

#if defined(__riscv_vector)
void stringToUpperVector(char* str, int len) {
    size_t vl;

    for (; len > 0; len -= vl, str += vl) {
        vl = __riscv_vsetvl_e8m1(len);
        vuint8m1_t vec_chars = __riscv_vle8_v_u8m1((const uint8_t*)str, vl);
        vbool8_t mask_ge = __riscv_vmsgeu_vx_u8m1_b8(vec_chars, (uint8_t)'a', vl);
        vbool8_t mask_le = __riscv_vmsleu_vx_u8m1_b8(vec_chars, (uint8_t)'z', vl);
        vbool8_t mask = __riscv_vmand_mm_b8(mask_ge, mask_le, vl);

        vuint8m1_t vec_upper = __riscv_vsub_vx_u8m1(vec_chars, (uint8_t)32, vl);
        vec_chars = __riscv_vmerge_vvm_u8m1(vec_chars, vec_upper, mask, vl);
        __riscv_vse8_v_u8m1((uint8_t*)str, vec_chars, vl);
    }
}
#else
void stringToUpperVector(char* str, int len) {
    stringToUpperScalar(str, len);
}
#endif

int verifyResults(const int result1[MATRIX_SIZE], const int result2[MATRIX_SIZE]) {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        if (result1[i] != result2[i]) {
            return 0;
        }
    }
    return 1;
}

int main(void) {
#if defined(__riscv_vector)
    enableVectorState();
#endif

    printf("=== RISC-V Vector Extension Lab ===\n\n");

    printf("Task A: Matrix-Vector Multiplication\n");
    matrixVectorScalar(result_scalar, matrix_a, vector_x);
    matrixVectorVector(result_vector, matrix_a, vector_x);

    if (verifyResults(result_scalar, result_vector)) {
        printf("Task A PASSED: Scalar and vector implementations match!\n");
    } else {
        printf("Task A FAILED: Results do not match!\n");
    }

    printf("\nTask B: String Case Conversion\n");
    initTestStrings();
    stringToUpperScalar(test_string_scalar, strlen(test_string_scalar));
    stringToUpperVector(test_string_vector, strlen(test_string_vector));

    if (strcmp(test_string_scalar, test_string_vector) == 0) {
        printf("Task B PASSED: Scalar and vector implementations match!\n");
    } else {
        printf("Task B FAILED: Results do not match!\n");
    }

    return 0;
}
