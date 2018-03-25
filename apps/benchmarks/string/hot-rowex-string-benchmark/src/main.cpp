#include <hot/rowex/HOTRowex.hpp>
#include <idx/benchmark/StringBenchmark.hpp>

#include <idx/contenthelpers/IdentityKeyExtractor.hpp>
#include <idx/contenthelpers/OptionalValue.hpp>

#include <idx/benchmark/NoThreadInfo.hpp>

class HotRowexStringBenchmarkWrapper {
	using NoThreadInfo = idx::benchmark::NoThreadInfo;
	using TrieType = hot::rowex::HOTRowex<const char*, idx::contenthelpers::IdentityKeyExtractor>;

	TrieType mTrie;

public:
	inline NoThreadInfo getThreadInformation() const {
		constexpr NoThreadInfo noThreadInfo { };
		return noThreadInfo;
	}

	bool insert(NoThreadInfo /* unused dummy threadInformation */, std::pair<char*, size_t> const & insertValue) {
		return mTrie.insert(insertValue.first);
	}

	bool search(NoThreadInfo /* unused dummy threadInformation */, std::pair<char*, size_t> const & lookupValue) {
		const char* key = lookupValue.first;
		idx::contenthelpers::OptionalValue<const char*> result = mTrie.lookup(key);
		return result.mIsValid & (result.mValue == key);
	}


	bool iterate(NoThreadInfo /* unused dummy threadInformation */, std::vector<std::pair<char*, size_t>> const & iterateKeys) {
		size_t i=0;
		bool iteratedAll = true;
		for(const char* value : mTrie) {
			iteratedAll = iteratedAll && (value == iterateKeys[i].first);
			++i;
		}
		return iteratedAll && i == iterateKeys.size();
	}

	idx::benchmark::IndexStatistics getStatistics() {
		std::pair<size_t, std::map<std::string, double>> stats = mTrie.getStatistics();
		return { stats.first, stats.second };
	}

	void writeDotRepresentation(std::ostream & /* output */) {
	}
};

int main(int argc, char** argv) {
	idx::benchmark::StringBenchmark<HotRowexStringBenchmarkWrapper> benchmark(argc, argv, "HotRowexStringBenchmark");
	return benchmark.run();
};
