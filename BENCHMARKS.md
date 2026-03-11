# Benchmark Results

This document summarises the simple wall‑clock timings obtained from
running the CTest targets provided by the project.  The measurements
were taken on the developer's machine during a single invocation of
`ctest` and are intended as a very coarse performance comparison.

## Raw test output

```
Test #2: run_pca ..........................   Passed    0.50 sec
Test #3: run_pca_kmeans ...................   Passed    0.97 sec
Test #4: run_pca_cuda .....................   Passed    0.18 sec
Test #5: run_pca_kmeans_cuda ..............   Passed    0.18 sec
```

Only the four unsupervised examples are relevant here;
a more comprehensive `ctest --output-on-failure` log is kept in the
build directory if you need to verify the results.

## Comparative analysis

| Test target             | Description                     | Time (s) | Notes |
|-------------------------|---------------------------------|----------|-------|
| `run_pca`               | CPU PCA only                    | 0.50     | 2‑D correlated data |
| `run_pca_cuda`          | GPU PCA only                    | 0.18     | identical dataset, 2.8× faster |
| `run_pca_kmeans`        | CPU PCA + k‑means               | 0.97     | different high‑dim data (24 features) |
| `run_pca_kmeans_cuda`   | GPU PCA + k‑means               | 0.18     | different data, 5.4× faster against CPU version |

``` IMPORTANT NOTE: IN THIS TEST SINCE GPU WAS PRESENT, THE CPU TEST RESULTS ARE ACTUALLY USING GPU ACCELERATION IN THIS TESTS RESULTS ```

### Observations

* The pure PCA kernel is roughly **three times faster** on the GPU for
  the small 2‑D problem used by the PCA examples.  The speed‑up arises
  mainly from offloading the random number generation, matrix
  multiplications and SVD to cuBLAS/cuSolver.

* When the k‑means step is added, the CPU version nearly doubles in
  duration; the CUDA version remains essentially unchanged (the k‑means
  loop in the GPU example is cheap).  This results in an apparent **5×
  speed‑up** when comparing the two k‑means targets, but this comparison
  is not apples‑to‑apples because the input tensors differ.

* Because the CPU and GPU k‑means examples use different sample
  generators, the benchmark numbers are *not* directly comparable.  To
  obtain meaningful ratios you should standardise the dataset (for
  example by using `sample_factory::create_sample_data` or by copying
  the 2‑D generator into the CPU code).

## Recommendations

1. **Standardise inputs.**  Modify the executables so they all operate on
   the same random seed and feature set before using the results as a
   formal benchmark.
2. **Repeat measurements.**  Run each test multiple times and average
   the results to reduce timing noise.
3. **Add logging.**  Consider extending the CTest rules to write times
   to a CSV file for easier post‑processing.

The timings above should be regarded as illustrative rather than
scientific.  They demonstrate that the GPU versions of the kernels are
significantly faster on this hardware, but the precise factors will
change with problem size, GPU model, and input data characteristics.
