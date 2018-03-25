#include <hot/singlethreaded/HOTSingleThreaded.hpp>
#include <idx/benchmark/StringBenchmark.hpp>

#include <idx/contenthelpers/IdentityKeyExtractor.hpp>
#include <idx/contenthelpers/OptionalValue.hpp>


class HotSingleThreadedBenchmarkWrapper {
	using TrieType = hot::singlethreaded::HOTSingleThreaded<const char*, idx::contenthelpers::IdentityKeyExtractor>;
	TrieType mTrie;

public:
	bool insert(std::pair<char*, size_t> const & key) {
		return mTrie.insert(key.first);
	}

	bool remove(std::pair<char*, size_t> const & key) {
		return mTrie.remove(key.first);
	}

	bool search(std::pair<char*, size_t> const & lookupKey) {
		idx::contenthelpers::OptionalValue<const char*> result = mTrie.lookup(lookupKey.first);
		return result.mIsValid;
	}


	bool iterate(std::vector<std::pair<char*, size_t>> const & iterateKeys) {
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
		return;
	}
};

int main(int argc, char** argv) {
	idx::benchmark::StringBenchmark<HotSingleThreadedBenchmarkWrapper> benchmark(argc, argv, "HotSingleThreadedStringsBenchmark");
	return benchmark.run();
};
