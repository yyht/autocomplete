#include <iostream>

#include "types.hpp"
#include "statistics.hpp"
#include "benchmark_common.hpp"

using namespace autocomplete;

template <typename Index>
void benchmark_conjunctive_topk(char const* binary_filename, uint32_t k,
                                uint32_t max_num_queries,
                                essentials::json_lines& breakdowns,
                                bool breakdown) {
    Index autocomp;
    essentials::logger("loading data structure from disk...");
    essentials::load(autocomp, binary_filename);
    essentials::logger("DONE");
    autocomp.print_stats();

    std::vector<std::string> queries;
    essentials::logger("loading queries...");
    uint32_t num_queries =
        load_queries(queries, max_num_queries, true, std::cin);
    essentials::logger("loaded " + std::to_string(num_queries) + " queries");

    auto ns_x_query = [&](double time) {
        return uint64_t(time / (runs * num_queries) * 1000);
    };

    essentials::logger("benchmarking conjunctive_topk queries...");
    uint64_t reported_strings = 0;

    if (breakdown) {
        std::vector<timer_type> timers(4);
        for (uint32_t run = 0; run != runs; ++run) {
            for (auto const& query : queries) {
                auto it = autocomp.conjunctive_topk(query, k, timers);
                reported_strings += it.size();
            }
        }
        essentials::logger("DONE");
        std::cout << reported_strings << std::endl;
        breakdowns.add("num_queries", std::to_string(num_queries));
        breakdowns.add("parsing_ns_per_query",
                       std::to_string(ns_x_query(timers[0].elapsed())));
        breakdowns.add("dictionary_search_ns_per_query",
                       std::to_string(ns_x_query(timers[1].elapsed())));
        breakdowns.add("conjunctive_search_ns_per_query",
                       std::to_string(ns_x_query(timers[2].elapsed())));
        breakdowns.add("reporting_ns_per_query",
                       std::to_string(ns_x_query(timers[3].elapsed())));
    } else {
        essentials::timer_type timer;
        timer.start();
        for (uint32_t run = 0; run != runs; ++run) {
            for (auto const& query : queries) {
                auto it = autocomp.conjunctive_topk(query, k);
                reported_strings += it.size();
            }
        }
        timer.stop();
        essentials::logger("DONE");
        std::cout << reported_strings << std::endl;
        breakdowns.add("num_queries", std::to_string(num_queries));
        breakdowns.add("ns_per_query",
                       std::to_string(ns_x_query(timer.elapsed())));
    }
}

int main(int argc, char** argv) {
    int mandatory = 5;
    if (argc < mandatory + 1) {
        std::cout << argv[0]
                  << " <type> <k> <binary_filename> <num_terms_per_query> "
                     "<max_num_queries> --breakdown < queries"
                  << std::endl;
        return 1;
    }

    std::string type(argv[1]);
    uint32_t k = std::atoi(argv[2]);
    char const* binary_filename = argv[3];
    std::string num_terms_per_query(argv[4]);
    uint32_t max_num_queries = std::atoi(argv[5]);

    bool breakdown = false;
    for (int i = mandatory; i != argc; ++i) {
        if (std::string(argv[i]) == "--breakdown") {
            breakdown = true;
        }
    }

    essentials::json_lines breakdowns;
    breakdowns.new_line();
    breakdowns.add("num_terms_per_query", num_terms_per_query);

    if (type == "type1") {
        benchmark_conjunctive_topk<uncompressed_autocomplete_type>(
            binary_filename, k, max_num_queries, breakdowns, breakdown);
    } else if (type == "type2") {
        benchmark_conjunctive_topk<uncompressed_autocomplete_type2>(
            binary_filename, k, max_num_queries, breakdowns, breakdown);
    } else if (type == "type3") {
        benchmark_conjunctive_topk<uncompressed_autocomplete_type3>(
            binary_filename, k, max_num_queries, breakdowns, breakdown);
    } else {
        std::cout << "error: unknown type '" << type << "'" << std::endl;
        return 1;
    }

    breakdowns.print();
    return 0;
}