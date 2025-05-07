#include <iostream>
#include <vector>
#include <omp.h>
#include <random>
#include <iomanip>
#include <cstdlib>

using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cerr << "Usage: " << argv[0] << " m n k" << endl;
        return 1;
    }
    const int m = atoi(argv[1]);
    const int n = atoi(argv[2]);
    const int k = atoi(argv[3]);

    // Validate input range
    if (m < 128 || m > 2048 || n < 128 || n > 2048 || k < 128 || k > 2048) {
        cerr << "Error: m, n, k must be in [128, 2048]" << endl;
        return 1;
    }

    // Allocate memory using vector
    vector<float> A(m * n);
    vector<float> B(n * k);
    vector<float> C(m * k, 0.0f);

    // Initialize matrices with random values
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dis(0.0, 1.0);

    for (auto& val : A) val = dis(gen);
    for (auto& val : B) val = dis(gen);

    // 获取线程数
    int num_threads = 1;
    #pragma omp parallel
    {
        #pragma omp master
        {
            num_threads = omp_get_num_threads();
        }
    }

    // 执行并行计算
    double start = omp_get_wtime();
    #pragma omp parallel for schedule(runtime)
    for (int i = 0; i < m; ++i) {
        // 原有计算逻辑保持不变
        for (int j = 0; j < k; ++j) {
            float sum = 0.0f;
            for (int l = 0; l < n; ++l) {
                sum += A[i * n + l] * B[l * k + j];
            }
            C[i * k + j] = sum;
        }
    }
    double elapsed = omp_get_wtime() - start;


    // 在程序头部添加调度方式获取
    const char* schedule = getenv("OMP_SCHEDULE");
    if (!schedule) schedule = "default";

    cout << num_threads << ","  // 使用保存的线程数
    << m << "," << n << "," << k << ","
    << schedule << ","
    << fixed << setprecision(4) << elapsed << endl;

    return 0;
}