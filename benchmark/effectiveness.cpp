#include <iostream>

#include "types.hpp"
#include "benchmark_common.hpp"

using namespace autocomplete;

template <typename Index>
void benchmark(std::string const& index_filename, uint32_t k,
               uint32_t max_num_queries, float keep,
               essentials::json_lines& stats, bool verbose) {
    Index index1, index2;
    essentials::load(index1, index_filename.c_str());
    essentials::load(index2, index_filename.c_str());

    std::vector<std::string> queries;
    uint32_t num_queries =
        load_queries(queries, max_num_queries, keep, std::cin);
    uint64_t strings_reported_by_prefix_search = 0;
    uint64_t better_scored_strings_reported_by_conjunctive_search = 0;

    stats.add("num_queries", std::to_string(num_queries));

    for (auto const& query : queries) {
        auto it1 = index1.prefix_topk(query, k);
        auto it2 = index2.conjunctive_topk(query, k);
        strings_reported_by_prefix_search += it1.size();

        uint64_t more = 0;
        if (it2.size() >= it1.size()) {
            more = it2.size() - it1.size();
        }

        if (verbose) {
            {
                auto it = it1;
                std::cout << "prefix search scores: " << std::endl;
                for (uint64_t i = 0; i != it.size(); ++i, ++it) {
                    std::cout << (*it).score << " ";
                }
                std::cout << std::endl;
            }
            {
                auto it = it2;
                std::cout << "conjunctive search scores: " << std::endl;
                for (uint64_t i = 0; i != it.size(); ++i, ++it) {
                    std::cout << (*it).score << " ";
                }
                std::cout << std::endl;
            }
            std::cout << "more: " << more << std::endl;
        }

        better_scored_strings_reported_by_conjunctive_search += more;
    }

    stats.add("strings_reported_by_prefix_search",
              std::to_string(strings_reported_by_prefix_search));
    stats.add(
        "better_scored_strings_reported_by_conjunctive_search",
        std::to_string(better_scored_strings_reported_by_conjunctive_search));
    stats.add(
        "better_scored_strings_reported_by_conjunctive_search_in_percentage",
        std::to_string(better_scored_strings_reported_by_conjunctive_search *
                       100.0 / strings_reported_by_prefix_search));
}

int main(int argc, char** argv) {
    cmd_line_parser::parser parser(argc, argv);
    configure_parser_for_benchmarking(parser);
    if (!parser.parse()) return 1;

    auto type = parser.get<std::string>("type");
    auto k = parser.get<uint32_t>("k");
    auto index_filename = parser.get<std::string>("index_filename");
    auto max_num_queries = parser.get<uint32_t>("max_num_queries");
    auto keep = parser.get<float>("percentage");
    auto verbose = parser.get<bool>("verbose");

    essentials::json_lines stats;
    stats.new_line();
    stats.add("num_terms_per_query",
              parser.get<std::string>("num_terms_per_query"));
    stats.add("percentage", std::to_string(keep));

    if (type == "ef_type1") {
        benchmark<ef_autocomplete_type1>(index_filename, k, max_num_queries,
                                         keep, stats, verbose);
    } else if (type == "ef_type2") {
        benchmark<ef_autocomplete_type2>(index_filename, k, max_num_queries,
                                         keep, stats, verbose);
    } else if (type == "ef_type3") {
        benchmark<ef_autocomplete_type3>(index_filename, k, max_num_queries,
                                         keep, stats, verbose);
    } else if (type == "ef_type4") {
        benchmark<ef_autocomplete_type4>(index_filename, k, max_num_queries,
                                         keep, stats, verbose);
    } else {
        return 1;
    }

    stats.print();
    return 0;
}