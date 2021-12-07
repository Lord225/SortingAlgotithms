#include <iostream>
#include <algorithm>
#include <cassert>
#include <vector>
#include <chrono>
#include <map>
#include <fstream>

class BenchmarkRegistry
{
public:
    using registry_type = std::tuple<std::string, long double, long double>;
    std::vector<registry_type> registry;

    void add_to_registry(std::string name, long double X, long double Y)
    {
        registry.emplace_back(std::make_tuple(name, X, Y));
    }

    void generate_csv(std::string path)
    {
        std::map<std::string, std::vector<std::tuple<long double, long double>>> xy;

        for(const auto& [name, X, Y] : registry)
        {
            if (!xy.contains(name))
                xy.emplace(name, std::vector<std::tuple<long double, long double>>());
            xy[name].push_back(std::make_tuple(X, Y));
        }

        for (auto& [key, val] : xy)
        {
            std::sort(val.begin(), val.end(), [](const auto& x, const auto& y)
            {
            	const auto& [x_a, x_b] = x;
                const auto& [y_a, y_b] = y;

                return x_b > y_b;
            });

            std::fstream outfile;

            for (const auto& [name, XY] : xy)
            {
                outfile.open(path + "/" + name + ".csv", std::ios::trunc | std::ios::out);

                outfile << "X,Y" << std::endl;
                for (const auto& [x, y] : XY)
                    outfile << x << "," << y << std::endl;

                outfile.close();
            }
        }
    }
};

class Benchmark
{
public:
	long double total_time;
    unsigned long long iterations = 0;
    std::string name;

    virtual ~Benchmark() = default;

#pragma optimize("", off)
    template<typename T>
    static void doNotOptimize(T& data) {}
    //1'000'000'000
    Benchmark&& runBenchmark(std::string _name, long double max_time = 500'000'000)
    {
        name = _name;
        using namespace std::chrono;

        total_time = 0.0;
        iterations = 0;

        while (total_time < max_time && iterations < 50'000)
        {
            setup();

            const auto start = high_resolution_clock::now();

            bench();

            const auto end = high_resolution_clock::now();

            total_time += duration_cast<nanoseconds>(end - start).count();

            iterations += 1;
        }
        return std::move(*this);
    }

    template<typename T>
    std::string get_good_ratio(T value)
    {
        using namespace std::chrono;
        if (value < 1'000)
            return std::to_string(value) + "ns";
        
        if (value < 1'000'000)
            return std::to_string(value / 1'000) + "us";
        
        if (value < 1'000'000'000)
            return std::to_string(value / 1'000'000) + "ms";

        return std::to_string(value / 1'000'000'000) + "s";
    }
    Benchmark&& generate_summary()
    {
        using namespace std::chrono;
        std::cout << "Benchmark Summary" << std::endl;
        std::cout << name << std::endl;
        std::cout << "Value: " << get_X_mesurment() << std::endl;
        std::cout << "Total: " << get_good_ratio(total_time) << std::endl;
        std::cout << "Call:  " << get_good_ratio(get_Y_mesurment()) << std::endl;
        std::cout << "Iter:  " << iterations << std::endl;
        std::cout << "-------------------------\n\n";

        return std::move(*this);
    }

    Benchmark&& register_output(BenchmarkRegistry& registry)
    {
        registry.add_to_registry(name, get_X_mesurment(), get_Y_mesurment());
        return std::move(*this);
    }

    virtual void setup() = 0;
    virtual void bench() = 0;
    virtual long double get_X_mesurment() = 0;
    virtual long double get_Y_mesurment()
    {
        return total_time / static_cast<long double>(iterations);
    }
};

template<typename T>
class GenerateVector
{
    std::vector<T> instance;
public:

    T uniform(T minimum, T maximum)
    {
        return T{ minimum + rand() % (maximum - minimum) };
    }
    GenerateVector(size_t size)
    {
        instance = std::vector<T>(size, 0);
    }
    GenerateVector(std::vector<T>&& el)
    {
        instance = std::move(el);
    }

    std::vector<T>&& get()
    {
        return std::move(instance);
    }

    GenerateVector&& random(T low, T high)
    {
        for (size_t i = 0; i < instance.size(); i++)
            instance[i] = uniform(low, high);

        return std::move(*this);
    }

    GenerateVector&& sorted()
    {
        std::sort(instance.begin(), instance.end());
        return std::move(*this);
    }

    GenerateVector&& shuffle(float shuffle_ratio)
    {
        const auto size = instance.size();
        const size_t shuffle_amount = size * shuffle_ratio;
        for (size_t i = 0; i < shuffle_amount; i++)
            std::swap(instance[uniform(0, size)], instance[uniform(0, size)]);

        return std::move(*this);
    }

    GenerateVector&& reverse(size_t size, float shuffle_ratio)
    {
        std::reverse(instance.begin(), instance.end());
        return std::move(*this);
    }
};

template<typename T>
void insertionSort(std::vector<T>& S)
{
    __assume(S.size() > 0);
    for (int i = 1; i < S.size(); i++)
    {
        const auto aux = S[i];
        int j = i - 1;
        for (j = i - 1; j >= 0; j--)
        {
            if (S[j] > aux)
                S[j + 1] = S[j];
            else
                break;
        }
        S[j + 1] = aux;
    }
}

template<typename T>
void insertionSort(std::vector<T>& S, size_t from, size_t to)
{
    __assume(S.size() > 0);
    __assume(S.size() <= to);
    __assume(S.size() >= from);
    __assume(to >= from);

    for (int i = from; i < to; i++)
    {
        const auto aux = S[i];
        int j = i - 1;
        for (j = i - 1; j >= 0; j--)
        {
            if (S[j] > aux)
                S[j + 1] = S[j];
            else
                break;
        }
        S[j + 1] = aux;
    }
}

template<typename T>
void selectionSort(std::vector<T>& S)
{
    __assume(S.size() > 0);
    for (size_t i = 0; i < S.size() - 1; i++)
    {
        auto min = i;
        for (size_t j = i + 1; j < S.size(); j++)
            if (S[j] < S[min]) min = j;

        std::swap(S[i], S[min]);
    }
}

template<typename T>
void bubbleSort(std::vector<T>& S)
{
    __assume(S.size() > 0);
    for (size_t i = 1; i < S.size(); i++)
    {
        for (size_t j = S.size()-1 ; j >= i; j--)
        {
            if (S[j] < S[j-1])
                std::swap(S[j], S[j-1]);
        }
    }
}

template<typename T>
void print(std::vector<T>& data)
{
    for (size_t i = 0; auto el : data)
        std::cout << el << ((++i == data.size()) ? " " : ", ");
    std::cout << std::endl;
}

template<typename T>
size_t partition(std::vector<T>& S, int low, int high)
{
    __assume(high >= low);

    int pivot = S[high];
    int i = (low - 1);

    for (int j = low; j < high; j++) {
        if (S[j] <= pivot) {
            i += 1;

            std::swap(S[i], S[j]);
        }
    }
    std::swap(S[i + 1], S[high]);

    return i + 1;
}

template<typename T>
void quickSort(std::vector<T>& S, int low, int high)
{
    if (low >= high)
        return;

    const size_t pivot = partition(S, low, high);

    quickSort(S, low, pivot - 1);
    quickSort(S, pivot + 1, high);
}

template<typename T>
void quickSort(std::vector<T>& S)
{
    quickSort(S, 0, S.size()-1);
}

template<typename T>
void OptimizedQuickSort(std::vector<T>& S, int low, int high)
{
    __assume(high >= low);

    if ((high - low) <= 33) 
    {
        insertionSort(S, low, high+1);
        return;
    }

    const size_t pivot = partition(S, low, high);

    OptimizedQuickSort(S, low, pivot - 1);
    OptimizedQuickSort(S, pivot + 1, high);
}

template<typename T>
void OptimizedQuickSort(std::vector<T>& S)
{
    OptimizedQuickSort(S, 0, S.size()-1);
}

template<typename T>
void stdSort(std::vector<T>& S)
{
    std::sort(S.begin(), S.end());
}

class BubbleSortBenchmark : public Benchmark
{
public:
    std::vector<int> data;
    size_t size;
    int max;
    BubbleSortBenchmark(size_t size, int max) : size(size), max(max) {}
    void setup() override
    {
        data = GenerateVector<int>(size).random(0, max).get();
    }
    void bench() override
    {
        bubbleSort(data);
        doNotOptimize(data);
    }
    long double get_X_mesurment() override { return size; }
};
class SelectionSortBenchmark : public Benchmark
{
public:
    std::vector<int> data;
    size_t size;
    int max;
    SelectionSortBenchmark(size_t size, int max) : size(size), max(max) {}
    void setup() override
    {
        data = GenerateVector<int>(size).random(0, max).get();
    }
    void bench() override
    {
        selectionSort(data);
        doNotOptimize(data);
    }
    long double get_X_mesurment() override { return size; }
};
class InsertionSortBenchmark : public Benchmark
{
public:
    const size_t sort_iterations = 1;
	std::vector<std::vector<int>> data;
    size_t size;
    int max;
    
    InsertionSortBenchmark(size_t size, int max) : size(size), max(max)
    {
        data.clear();
        for (size_t i = 0; i < sort_iterations; i++)
            data.emplace_back(std::vector(size, 0));
    }
    void setup() override
    {
        for (size_t i = 0; i < sort_iterations; i++)
            data[i] = GenerateVector(std::move(data[i])).random(0, max).get();
    }
    void bench() override
    {
        for (size_t i = 0; i < sort_iterations; i++)
        {
            insertionSort(data[i]);
        }
    	doNotOptimize(data);
    }
    long double get_X_mesurment() override { return size; }
    long double get_Y_mesurment() override { return total_time / static_cast<long double>(iterations) / sort_iterations; }
};
class BubbleSortBenchmark2 : public Benchmark
{
public:
    std::vector<int> data;
    size_t size;
    int max;
    float shuffle;
    BubbleSortBenchmark2(size_t size, int max, float shuffle) : size(size), max(max), shuffle(shuffle) {}
    void setup() override
    {
        data = GenerateVector<int>(size).random(0, max).sorted().shuffle(shuffle).get();
    }
    void bench() override
    {
        bubbleSort(data);
        doNotOptimize(data);
    }
    long double get_X_mesurment() override { return size; }
};
class SelectionSortBenchmark2 : public Benchmark
{
public:
    std::vector<int> data;
    size_t size;
    int max;
    float shuffle;
    SelectionSortBenchmark2(size_t size, int max, float shuffle) : size(size), max(max), shuffle(shuffle) {}
    void setup() override
    {
        data = GenerateVector<int>(size).random(0, max).sorted().shuffle(shuffle).get();
    }
    void bench() override
    {
        selectionSort(data);
        doNotOptimize(data);
    }
    long double get_X_mesurment() override { return size; }
};
class InsertionSortBenchmark2 : public Benchmark
{
public:
    const size_t sort_iterations = 1;
    std::vector<std::vector<int>> data;
    size_t size;
    int max;
    float shuffle;
    InsertionSortBenchmark2(size_t size, int max, float shuffle) : size(size), max(max), shuffle(shuffle)
	{
        data.clear();
        for (size_t i = 0; i < sort_iterations; i++)
            data.emplace_back(std::vector(size, 0));
    }
    void setup() override
    {
        for (size_t i = 0; i < sort_iterations; i++)
            data[i] = GenerateVector(std::move(data[i])).random(0, max).sorted().shuffle(shuffle).get();
    }
    void bench() override
    {
        for (size_t i = 0; i < sort_iterations; i++)
        {
            insertionSort(data[i]);
        }
        doNotOptimize(data);
    }
    long double get_X_mesurment() override { return size; }
    long double get_Y_mesurment() override { return total_time / static_cast<long double>(iterations) / sort_iterations; }
};
class StdSortBenchmark : public Benchmark
{
public:
    std::vector<int> data;
    size_t size;
    int max;
    StdSortBenchmark(size_t size, int max) : size(size), max(max) {}
    void setup() override
    {
        data = GenerateVector<int>(size).random(0, max).get();
    }
    void bench() override
    {
        stdSort(data);
        doNotOptimize(data);
    }
    long double get_X_mesurment() override { return size; }
};
class QuickSortBenchmark : public Benchmark
{
public:
    std::vector<int> data;
    size_t size;
    int max;
    QuickSortBenchmark(size_t size, int max) : size(size), max(max) {}
    void setup() override
    {
        data = GenerateVector<int>(size).random(0, max).get();
    }
    void bench() override
    {
        quickSort(data);
        doNotOptimize(data);
    }
    long double get_X_mesurment() override { return size; }
};
class QuickSortBenchmark2 : public Benchmark
{
public:
    std::vector<int> data;
    size_t size;
    int max;
    float shuffle;
    QuickSortBenchmark2(size_t size, int max, float shuffle) : size(size), max(max), shuffle(shuffle) {}
    void setup() override
    {
        data = GenerateVector<int>(size).random(0, max).sorted().shuffle(shuffle).get();
    }
    void bench() override
    {
        quickSort(data);
        doNotOptimize(data);
    }
    long double get_X_mesurment() override { return size; }
};
class BetterQuickSortBenchmark : public Benchmark
{
public:
    std::vector<int> data;
    size_t size;
    int max;
    BetterQuickSortBenchmark(size_t size, int max) : size(size), max(max) {}
    void setup() override
    {
        data = GenerateVector<int>(size).random(0, max).get();
    }
    void bench() override
    {
        OptimizedQuickSort(data);
        doNotOptimize(data);
    }
    long double get_X_mesurment() override { return size; }
};
class BetterQuickSortBenchmark2 : public Benchmark
{
public:
    std::vector<int> data;
    size_t size;
    int max;
    float shuffle;
    BetterQuickSortBenchmark2(size_t size, int max, float shuffle) : size(size), max(max), shuffle(shuffle) {}
    void setup() override
    {
        data = GenerateVector<int>(size).random(0, max).sorted().shuffle(shuffle).get();
    }
    void bench() override
    {
        OptimizedQuickSort(data);
        doNotOptimize(data);
    }
    long double get_X_mesurment() override { return size; }
};


template<void (*F)(std::vector<int>&)>
void TEST_SORT()
{
    auto data = GenerateVector<int>(1000).random(0, 1000).get();
    auto data_cpy = data;
    F(data);
    std::sort(data_cpy.begin(), data_cpy.end());
    assert(std::is_sorted(data.begin(), data.end()));
    for (size_t i = 0; i < data.size(); i++)
        assert(data[i] == data_cpy[i]);
}

int main()
{
    constexpr bool enable_std_benchmarks = true;
    constexpr bool enable_quick_benchmarks = true;
    constexpr bool enable_bubble_benchmarks = true;
    constexpr bool enable_insertion_benchmarks = true;
    constexpr bool enable_selection_benchmarks = true;

	TEST_SORT<bubbleSort<int>>();
    TEST_SORT<insertionSort<int>>();
    TEST_SORT<selectionSort<int>>();
    TEST_SORT<quickSort<int>>();
    TEST_SORT<OptimizedQuickSort<int>>();
    TEST_SORT<stdSort<int>>();

    std::cout << "TESTS PASSED" << std::endl;

    BenchmarkRegistry registry;

    const auto strides = { 100, 200, 300, 1000, 2000, 3000, 5000, 10'000, 11'000, 12'000, 13'000, 15'000, 20'000, 21'000, 25'000, 30'000, 31'000, 32'000};

    const auto strides_long = { 50'000, 60'000, 70'000, 80'000, 90'000, 100'000};


    if constexpr (enable_std_benchmarks) 
    {
        for (const auto bench_case : strides)
            StdSortBenchmark(bench_case, 10000).runBenchmark("Std").generate_summary().register_output(registry);
        for (const auto bench_case : strides_long)
            StdSortBenchmark(bench_case, 10000).runBenchmark("Std").generate_summary().register_output(registry);
        for (int i = 100'000; i < 300'000; i += 5'000)
            StdSortBenchmark(i, 10000).runBenchmark("Std").generate_summary().register_output(registry);
        for (int i = 2; i < 100; i += 1)
            StdSortBenchmark(i, 10000).runBenchmark("Std").generate_summary().register_output(registry);
    }

    if constexpr (enable_quick_benchmarks)
    {
        for (const auto bench_case : strides)
            QuickSortBenchmark(bench_case, 10000).runBenchmark("Quick").generate_summary().register_output(registry);
        for (const auto bench_case : strides_long)
            QuickSortBenchmark(bench_case, 10000).runBenchmark("Quick").generate_summary().register_output(registry);
        for (int i = 100'000; i < 300'000; i += 5'000)
            QuickSortBenchmark(i, 10000).runBenchmark("Quick").generate_summary().register_output(registry);
        for (int i = 2; i < 100; i += 1)
            QuickSortBenchmark(i, 10000).runBenchmark("Quick").generate_summary().register_output(registry);

        for (const auto bench_case : strides)
            BetterQuickSortBenchmark(bench_case, 10000).runBenchmark("Better Quick").generate_summary().register_output(registry);
        for (int i = 100'000; i < 300'000; i += 5'000)
            BetterQuickSortBenchmark(i, 10000).runBenchmark("Better Quick").generate_summary().register_output(registry);
    }
    if constexpr (enable_bubble_benchmarks)
    {
        for (const auto bench_case : strides)
            BubbleSortBenchmark(bench_case, 10000).runBenchmark("Bubble").generate_summary().register_output(registry);

        for (const auto bench_case : strides)
            BubbleSortBenchmark2(bench_case, 10000, 0.3).runBenchmark("Bubble Almost Sorted").generate_summary().register_output(registry);
    }

    if constexpr (enable_selection_benchmarks)
    {
        for (const auto bench_case : strides)
            SelectionSortBenchmark(bench_case, 10000).runBenchmark("Selection").generate_summary().register_output(registry);
    
        for (const auto bench_case : strides)
            SelectionSortBenchmark2(bench_case, 10000, 0.3).runBenchmark("Selection Almost Sorted").generate_summary().register_output(registry);
    }

    if constexpr (enable_insertion_benchmarks)
    {
        for (const auto bench_case : strides)
            InsertionSortBenchmark(bench_case, 10000).runBenchmark("Insertion").generate_summary().register_output(registry);
        for (const auto bench_case : strides_long)
            InsertionSortBenchmark(bench_case, 10000).runBenchmark("Insertion").generate_summary().register_output(registry);
        for (int i = 2; i < 100; i += 1)
            InsertionSortBenchmark(i, 10000).runBenchmark("Insertion").generate_summary().register_output(registry);
        for (const auto bench_case : strides)
            InsertionSortBenchmark2(bench_case, 10000, 0.3).runBenchmark("Insertion Almost Sorted").generate_summary().register_output(registry);
        for (const auto bench_case : strides_long)
            InsertionSortBenchmark2(bench_case, 10000, 0.3).runBenchmark("Insertion Almost Sorted").generate_summary().register_output(registry);
    }

    std::string path = "C:\\Users\\Maciek\\source\\repos\\Labolatorium8\\Benchmarks";
    std::string path2 = "C:\\Users\\Maciej\\source\\repos\\SortingAlgotithms\\Benchmarks2";

    registry.generate_csv(path);
}
