import random
import time

m, n, k = 512, 1024, 2048

# 生成随机矩阵
A = [[random.random() for _ in range(n)] for _ in range(m)]
B = [[random.random() for _ in range(k)] for _ in range(n)]
C = [[0.0 for _ in range(k)] for _ in range(m)]

start = time.time()

# 三重循环矩阵乘法
for i in range(m):
    for j in range(k):
        for p in range(n):
            C[i][j] += A[i][p] * B[p][j]

end = time.time()
print(f"A: {m}x{n}, B: {n}x{k}, C: {m}x{k}")
print(f"运行时间: {end - start:.4f} s")
GFLOPS = 2 * m * n * k / (end - start) / 1e9
print(f"GFLOPS: {GFLOPS:.2f}")
# 计算峰值性能百分比
peak_GFLOPS = 112.8
print(f"峰值性能百分比: {GFLOPS / peak_GFLOPS * 100:.2f}%")
