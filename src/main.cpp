#include <atomic>
#include <chrono>
#include <csignal>
#include <iostream>
#include <thread>

#include <gridx/matching/common/Version.hpp>

namespace {

std::atomic<bool> g_isRunning{true};

void handleSignal(int) {
    g_isRunning = false;
}

}  // namespace

int main() {
    std::cout << "GridX Matching Engine " << gridx::matching::common::Version::getVersion()
              << std::endl;

    // Minimal process lifecycle hooks for local/dev execution.
    std::signal(SIGINT, handleSignal);
    std::signal(SIGTERM, handleSignal);

    // TODO: Load environment configuration and validate required settings.
    // TODO: Initialize logger using production configuration.
    // TODO: Construct shared infrastructure (kafka clients, caches, metrics).
    // TODO: Wire and start order/grid-transfer consumers.

    std::cout << "Bootstrapping complete. Entering main loop." << std::endl;

    std::uint64_t loopTick = 0;

    while (g_isRunning.load()) {
        // TODO: Poll inbound messages and hand off to OrderProcessor.
        // TODO: Flush producer buffers and emit periodic metrics/health.

        if ((loopTick % 100) == 0) {
            std::cout << "Matching engine loop heartbeat tick=" << loopTick << std::endl;
        }

        ++loopTick;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // TODO: Stop consumers and drain any in-flight work before exit.
    // TODO: Flush producers and close external connections gracefully.
    std::cout << "Shutdown signal received. Exiting cleanly." << std::endl;

    return 0;
}