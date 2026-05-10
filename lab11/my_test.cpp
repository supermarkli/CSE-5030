#include "ventus.h"
#include <chrono>
#include <cstring>
#include <iostream>
#include <thread>

using namespace std;

struct meta_data {
    uint64_t kernel_id;
    uint64_t kernel_size[3];
    uint64_t wf_size;
    uint64_t wg_size;
    uint64_t metaDataBaseAddr;
    uint64_t ldsSize;
    uint64_t pdsSize;
    uint64_t sgprUsage;
    uint64_t vgprUsage;
    uint64_t pdsBaseAddr;

    meta_data(uint64_t arg0, uint64_t arg1[], uint64_t arg2, uint64_t arg3,
              uint64_t arg4, uint64_t arg5, uint64_t arg6, uint64_t arg7,
              uint64_t arg8, uint64_t arg9)
        : kernel_id(arg0), wf_size(arg2), wg_size(arg3),
          metaDataBaseAddr(arg4), ldsSize(arg5), pdsSize(arg6),
          sgprUsage(arg7), vgprUsage(arg8), pdsBaseAddr(arg9) {
        kernel_size[0] = arg1[0];
        kernel_size[1] = arg1[1];
        kernel_size[2] = arg1[2];
    }
};

int runKernel(vt_device_h p, const int* inputA, const int* inputB,
              int* outputC, int dataSize, int kernelType) {
    const uint64_t byteSize = static_cast<uint64_t>(dataSize * sizeof(int));
    uint64_t vaddrA = 0;
    uint64_t vaddrB = 0;
    uint64_t vaddrC = 0;

    if (vt_buf_alloc(p, byteSize, &vaddrA, 0, 0, 0) != 0) {
        return -1;
    }
    if (vt_buf_alloc(p, byteSize, &vaddrB, 0, 0, 0) != 0) {
        return -1;
    }
    if (vt_buf_alloc(p, byteSize, &vaddrC, 0, 0, 0) != 0) {
        return -1;
    }

    if (vt_copy_to_dev(p, vaddrA, const_cast<int*>(inputA), byteSize, 0, 0) != 0) {
        return -1;
    }
    if (vt_copy_to_dev(p, vaddrB, const_cast<int*>(inputB), byteSize, 0, 0) != 0) {
        return -1;
    }

    const char* kernelFile = (kernelType == 0) ? "my_vecadd.riscv" : "my_vecmul.riscv";
    if (vt_upload_kernel_file(p, kernelFile, 0) != 0) {
        return -1;
    }

    uint64_t numWorkgroups[3] = {1, 1, 1};
    meta_data meta(0, numWorkgroups, dataSize, 1,
                   0x90000000, 0x1000, 0, 32, 32, 0x90000000);

    if (vt_start(p, &meta, 0) != 0) {
        return -1;
    }

    this_thread::sleep_for(chrono::seconds(1));

    if (vt_copy_from_dev(p, vaddrC, outputC, byteSize, 0, 0) != 0) {
        return -1;
    }

    vt_buf_free(p, 0, nullptr, 0, 0);
    return 0;
}

int main() {
    cout << "=== Ventus GPGPU Driver Lab ===" << endl;

    const int DATA_SIZE = 8;
    int inputA[DATA_SIZE] = {1, 2, 3, 4, 5, 6, 7, 8};
    int inputB[DATA_SIZE] = {2, 3, 4, 5, 6, 7, 8, 9};
    int output[DATA_SIZE] = {0};

    vt_device_h p = nullptr;
    if (vt_dev_open(&p) != 0) {
        cerr << "Failed to open Ventus device" << endl;
        return -1;
    }

    cout << "\n--- Task A: Vector Addition ---" << endl;
    cout << "A: ";
    for (int i = 0; i < DATA_SIZE; i++) cout << inputA[i] << ' ';
    cout << "\nB: ";
    for (int i = 0; i < DATA_SIZE; i++) cout << inputB[i] << ' ';
    cout << endl;

    memset(output, 0, sizeof(output));
    if (runKernel(p, inputA, inputB, output, DATA_SIZE, 0) != 0) {
        cerr << "Task A launch failed" << endl;
        return -1;
    }

    cout << "C = A + B: ";
    for (int i = 0; i < DATA_SIZE; i++) cout << output[i] << ' ';
    cout << endl;

    bool passA = true;
    for (int i = 0; i < DATA_SIZE; i++) {
        if (output[i] != inputA[i] + inputB[i]) {
            cout << " Error at index " << i << ": got " << output[i]
                 << ", expected " << inputA[i] + inputB[i] << endl;
            passA = false;
        }
    }
    cout << "Task A: " << (passA ? "PASSED" : "FAILED") << endl;

    cout << "\n--- Task B: Vector Multiplication ---" << endl;
    cout << "A: ";
    for (int i = 0; i < DATA_SIZE; i++) cout << inputA[i] << ' ';
    cout << "\nB: ";
    for (int i = 0; i < DATA_SIZE; i++) cout << inputB[i] << ' ';
    cout << endl;

    memset(output, 0, sizeof(output));
    if (runKernel(p, inputA, inputB, output, DATA_SIZE, 1) != 0) {
        cerr << "Task B launch failed" << endl;
        return -1;
    }

    cout << "C = A * B: ";
    for (int i = 0; i < DATA_SIZE; i++) cout << output[i] << ' ';
    cout << endl;

    bool passB = true;
    for (int i = 0; i < DATA_SIZE; i++) {
        if (output[i] != inputA[i] * inputB[i]) {
            cout << " Error at index " << i << ": got " << output[i]
                 << ", expected " << inputA[i] * inputB[i] << endl;
            passB = false;
        }
    }
    cout << "Task B: " << (passB ? "PASSED" : "FAILED") << endl;

    cout << "\n=== " << ((passA && passB) ? "All tests passed!" : "Some tests FAILED!")
         << " ===" << endl;

    return (passA && passB) ? 0 : -1;
}
