import subprocess


class BenchmarkResult:
    def __init__(self, case: str, time: float, cpu_time: float, iteration: float):
        self.case = case
        self.time = time
        self.cpu = cpu_time
        self.iteration = iteration

    def normalize(self):
        self.time /= self.iteration
        self.cpu /= self.iteration
        self.iteration = 1

    def __add__(self, other):
        if other is not BenchmarkResult:
            pass
        if other.case != self.case:
            pass
        total_time = self.time + other.time
        total_cpu_time = self.cpu + other.cpu
        total_iteration = self.iteration + other.iteration

        return BenchmarkResult(self.case, total_time, total_cpu_time, total_iteration)

    def __str__(self):
        return "{}: \t{} ns\t{} ns".format(self.case, self.time, self.cpu)


def run_benchmark(program_path: str) -> str:
    benchmark_process = subprocess.run(args=[program_path], capture_output=True)
    return benchmark_process.stdout.decode()


def process_output(benchmark_output: str) -> [BenchmarkResult]:
    benchmark_results = []
    for line in benchmark_output.splitlines():
        if line.strip().startswith("BM"):
            benchmark_data = line.split()
            case = benchmark_data[0].strip()
            time = benchmark_data[1]
            cpu_time = benchmark_data[3]
            iteration = benchmark_data[5]
            res = BenchmarkResult(case, float(time), float(cpu_time), float(iteration))
            print("current res = " + str(res))
            benchmark_results.append(res)
    print()
    return benchmark_results


def collect_benchmark_statistics(program_path: str, total_time: int) -> [BenchmarkResult]:
    all_results = None
    for current_time in range(0, total_time):
        print("running {} time".format(current_time))
        output = run_benchmark(program_path)
        results = process_output(output)
        if all_results is None:
            all_results = results
        else:
            for (prev, idx) in zip(results, range(0, len(results))):
                all_results[idx] += prev
                all_results[idx].normalize()
    return all_results


if __name__ == "__main__":
    path = R"F:\C++\modern-stl\cmake-build-release\test\try_benchmark.exe"
    times = 50

    benchmark_statistics = collect_benchmark_statistics(path, times)

    print("\n\n\n")

    for result in benchmark_statistics:
        print(result)
