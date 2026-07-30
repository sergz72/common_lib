cmake_minimum_required(VERSION 4.3)
project(ethernet_library_test C)

set(CMAKE_C_STANDARD 23)

add_executable(ethernet_library_test
        main.c
        trng.c
        ../../my/Programs/common_lib/queue.c
        ../../my/Programs/common_lib/ethernet/eth.c
        ../../my/Programs/common_lib/ethernet/eth_ipv6.c
        ../../my/Programs/common_lib/ethernet/eth_icmpv6.c
        ../../my/Programs/common_lib/ethernet/eth_udp.c
        ../../my/Programs/common_lib/ethernet/eth_queue.c
        ../../my/Programs/common_lib/ethernet/eth_ndp.c
        ../../my/Programs/common_lib/ethernet/eth_ntp.c
)

include_directories(. ../../my/Programs/common_lib ../../my/Programs/common_lib/crypto ../../my/Programs/common_lib/ethernet)