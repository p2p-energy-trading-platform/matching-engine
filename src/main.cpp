#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <thread>

#include <spdlog/spdlog.h>

#include <gridx/matching/common/Version.hpp>
#include "gridx/matching/adapters/kafka/KafkaProducer.hpp"
#include "gridx/matching/adapters/kafka/MatchingResultPublisher.hpp"
#include "gridx/matching/adapters/kafka/OrderUpdateEventMapper.hpp"
#include "gridx/matching/adapters/kafka/TradeEventMapper.hpp"
#include "gridx/matching/config/EnvConfigLoader.hpp"
#include "gridx/matching/engine/ExpiryProcessor.hpp"
#include "gridx/matching/expiry/ExpiryManager.hpp"
#include "gridx/matching/orderbook/MarketBookManager.hpp"

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

    const auto configResult = gridx::matching::config::EnvConfigLoader::loadFromEnvironment();

    if (!configResult.success()) {
        for (const auto& error : configResult.errors) {
            std::cerr << error.variableName << ": " << error.reason << '\n';
        }

        return EXIT_FAILURE;
    }

    const auto& appConfig = configResult.config;

    spdlog::set_level(appConfig.logging.level);
    spdlog::info("Loaded application configuration");
    spdlog::info("Kafka bootstrap servers: {}", appConfig.kafka.bootstrapServers);
    spdlog::info("Kafka client id: {}", appConfig.kafka.clientId);
    spdlog::info("Order topic: {}", appConfig.orderConsumer.topic);
    spdlog::info("Grid transfer topic: {}", appConfig.gridTransferConsumer.topic);
    spdlog::info("Trade topic: {}", appConfig.publisher.tradeTopic);
    spdlog::info("Order state topic: {}", appConfig.publisher.orderStateTopic);
    namespace kafka = gridx::matching::adapters::kafka;

    kafka::KafkaProducer producer(kafka::KafkaProducerConfig{
        .bootstrapServers = appConfig.kafka.bootstrapServers,
        .clientId = appConfig.kafka.clientId,
        .bootstrapTimeout = appConfig.kafka.bootstrapTimeout,
    });

    kafka::TradeEventMapper tradeEventMapper;
    kafka::OrderUpdateEventMapper orderUpdateEventMapper;

    kafka::MatchingResultPublisher matchingResultPublisher(
        producer, tradeEventMapper, orderUpdateEventMapper, appConfig.publisher.tradeTopic,
        appConfig.publisher.orderStateTopic);

    // TODO: Share this MarketBookManager with the OrderProcessor once order consumption is wired.
    gridx::matching::orderbook::MarketBookManager marketBookManager;

    const gridx::matching::expiry::ExpiryManager expiryManager;
    gridx::matching::engine::ExpiryProcessor expiryProcessor(marketBookManager, expiryManager,
                                                             matchingResultPublisher);

    // TODO: Construct remaining shared infrastructure (caches, metrics).
    // TODO: Wire and start order/grid-transfer consumers.

    spdlog::info("Bootstrapping complete. Entering main loop.");

    // The loop sleeps 100 ms per tick, so expiry runs about once per second.
    constexpr std::uint64_t kExpiryCheckEveryTicks = 10;

    std::uint64_t loopTick = 0;

    while (g_isRunning.load(std::memory_order_relaxed)) {
        // TODO: Poll inbound messages and hand off to OrderProcessor.
        // TODO: Flush producer buffers and emit periodic metrics/health.

        // Expiry runs on the main loop thread, the same thread that will process orders,
        // because MarketBookManager is not thread-safe.
        if ((loopTick % kExpiryCheckEveryTicks) == 0) {
            try {
                static_cast<void>(expiryProcessor.process(std::chrono::system_clock::now()));
            } catch (const std::exception& error) {
                spdlog::error("Order expiry failed: {}", error.what());
            }
        }

        if ((loopTick % 100) == 0) {
            spdlog::info("Matching engine loop heartbeat tick={}", loopTick);
        }

        ++loopTick;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // TODO: Stop consumers and drain any in-flight work before exit.
    // TODO: Flush producers and close external connections gracefully.
    spdlog::info("Shutdown signal received. Exiting cleanly.");
    return 0;
}