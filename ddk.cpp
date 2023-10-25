#include <sycl/sycl.hpp>

constexpr size_t N = 1024;

int main() {
    sycl::queue myQueue;

    std::vector<int> a(N);
    std::vector<int> b(N);
    std::vector<int> c(N);

    // Initialize input vectors
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = i;
    }

    sycl::range<1> dataRange(N);
    sycl::buffer<int, 1> aBuffer(a.data(), dataRange);
    sycl::buffer<int, 1> bBuffer(b.data(), dataRange);
    sycl::buffer<int, 1> cBuffer(c.data(), dataRange);

    // Define a SYCL kernel using a lambda function
    auto vector_add = [&](sycl::handler &cgh) {
        auto aAccessor = aBuffer.get_access<sycl::access::mode::read>(cgh);
        auto bAccessor = bBuffer.get_access<sycl::access::mode::read>(cgh);
        auto cAccessor = cBuffer.get_access<sycl::access::mode::write>(cgh);

        sycl::accessor<int, 1, sycl::access::mode::read_write, sycl::access::target::local> localAcc(sycl::range<1>(N), cgh);

        cgh.parallel_for(dataRange, [=](sycl::item<1> item) {
            int local_id = item.get_local_id(0);
            localAcc[local_id] = aAccessor[item] + bAccessor[item];

            // Ensure all work-items in the work-group have updated localAcc
            cgh.barrier(sycl::access::fence_space::local_space);

            cAccessor[item] = localAcc[local_id];
        });
    };

    myQueue.submit(vector_add);

    // Ensure all SYCL tasks are completed
    myQueue.wait();

    // Verify the result
    for (int i = 0; i < N; i++) {
        if (c[i] != a[i] + b[i]) {
            std::cout << "Verification failed!" << std::endl;
            return 1;
        }
    }

    std::cout << "Verification passed." << std::endl;

    return 0;
}
