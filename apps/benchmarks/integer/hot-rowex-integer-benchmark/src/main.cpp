#include <algorithm>
#include <cassert>
#include <chrono>
#include <iostream>
#include <map>

#include <hot/rowex/HOTRowex.hpp>
#include <idx/benchmark/Benchmark.hpp>
#include <idx/benchmark/NoThreadInfo.hpp>

#include <idx/contenthelpers/IdentityKeyExtractor.hpp>
#include <idx/contenthelpers/OptionalValue.hpp>

class HotRowexIntegerBenchmarkWrapper {
	using NoThreadInfo = idx::benchmark::NoThreadInfo;
	using TrieType = hot::rowex::HOTRowex<uint64_t, idx::contenthelpers::IdentityKeyExtractor>;
	TrieType mTrie;


public:
	inline NoThreadInfo getThreadInformation() const {
		constexpr NoThreadInfo noThreadInfo { };
		return noThreadInfo;
	}

	inline bool insert(NoThreadInfo /* unused dummy threadInformation */, uint64_t key) {
		return mTrie.insert(key);
	}
	
	inline bool search(NoThreadInfo /* unused dummy threadInformation */, uint64_t key) {
		idx::contenthelpers::OptionalValue<uint64_t> result = mTrie.lookup(key);
		return result.mIsValid & (result.mValue == key);
	}

	inline bool iterateAll(NoThreadInfo /* unused dummy threadInformation */, std::vector<uint64_t> const & iterateKeys) {
		size_t i=0;
		bool iteratedAll = true;
		for(auto it = mTrie.begin(); it != mTrie.end(); ++it) {
			iteratedAll = iteratedAll & (*it == iterateKeys[i]);
			++i;
		}
		return iteratedAll & (i == iterateKeys.size());
	}

	inline bool findFirstAndIterate(NoThreadInfo /* unused dummy threadInformation */, std::vector<uint64_t> const & iterateKeys) {
		TrieType::const_iterator it = mTrie.find(iterateKeys[0]);

		bool iteratedAll = true;
		for(uint64_t expectedKey : iterateKeys) {
			iteratedAll = iteratedAll & (expectedKey == *it);
			++it;
		}

		return iteratedAll & (it == mTrie.end());
	}

	idx::benchmark::IndexStatistics getStatistics() {
		std::pair<size_t, std::map<std::string, double>> stats = mTrie.getStatistics();

		return { stats.first, stats.second };
	}
};


int main(int argc, char** argv) {
	idx::benchmark::Benchmark<HotRowexIntegerBenchmarkWrapper> benchmark(argc, argv, "HotRowexIntegerBenchmark");
	return benchmark.run();
}