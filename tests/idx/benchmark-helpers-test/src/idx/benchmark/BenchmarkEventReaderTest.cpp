//
//  Created by Robert Binna
//

#include <iostream>
#include <boost/test/unit_test.hpp>

#include <idx/benchmark/BenchmarkEventReader.hpp>
#include <idx/benchmark/BenchmarkBaseEvent.hpp>
#include <idx/benchmark/SpecificEventProcessor.hpp>

#include <idx/contenthelpers/IdentityKeyExtractor.hpp>

#include <idx/benchmark/TempFile.hpp>

namespace idx { namespace benchmark {

BOOST_AUTO_TEST_SUITE(BenchmarkEventReaderTest)

BOOST_AUTO_TEST_CASE(testBenchmarkIntEventReader) {
	TempFile tempEventFile {
		"INSERT 1234",
		"INSERT 42",
		"INSERT 32",
		"READ 32",
		"READ 42",
		"SCAN 42 10",
		"UPDATE 42"
	};

	BenchmarkEventReader<uint64_t, idx::contenthelpers::IdentityKeyExtractor> reader(tempEventFile.mFileName);
	BenchmarkEventList events { reader.readEvents() };

	SpecificEventProcessor<uint64_t, idx::contenthelpers::IdentityKeyExtractor> processEvent;

	using SpecificInsertEvent = InsertEvent<uint64_t, idx::contenthelpers::IdentityKeyExtractor>;
	using SpecificLookupEvent = LookupEvent<uint64_t, idx::contenthelpers::IdentityKeyExtractor>;
	using SpecificScanEvent = ScanEvent<uint64_t, idx::contenthelpers::IdentityKeyExtractor>;
	using SpecificUpdateEvent = UpdateEvent<uint64_t, idx::contenthelpers::IdentityKeyExtractor>;

	int i=0;
	for(BenchmarkBaseEvent & event : events) {
		processEvent(&event, [&i](auto const & specificEvent) {
			if(i == 0) {
				BOOST_REQUIRE_EQUAL(reinterpret_cast<SpecificInsertEvent const*>(&specificEvent)->mValueToInsert, 1234u);
			} else if(i == 1) {
				BOOST_REQUIRE_EQUAL(reinterpret_cast<SpecificInsertEvent const*>(&specificEvent)->mValueToInsert, 42u);
			} else if(i == 2) {
				BOOST_REQUIRE_EQUAL(reinterpret_cast<SpecificInsertEvent const*>(&specificEvent)->mValueToInsert, 32u);
			} else if(i == 3) {
				BOOST_REQUIRE_EQUAL(reinterpret_cast<SpecificLookupEvent const*>(&specificEvent)->mKey, 32u);
			} else if(i == 4) {
				BOOST_REQUIRE_EQUAL(reinterpret_cast<SpecificLookupEvent const*>(&specificEvent)->mKey, 42u);
			} else if(i == 5) {
				BOOST_REQUIRE_EQUAL(reinterpret_cast<SpecificScanEvent const*>(&specificEvent)->mLookupKey, 42u);
				BOOST_REQUIRE(reinterpret_cast<SpecificScanEvent const*>(&specificEvent)->mLastResult.compliesWith({}));
			} else if(i == 6) {
				BOOST_REQUIRE(reinterpret_cast<SpecificUpdateEvent const*>(&specificEvent)->mPreviousValue.compliesWith({ true, 42u }));
			}
		});
		++i;
	}
	BOOST_REQUIRE_EQUAL(i, 7);
}

BOOST_AUTO_TEST_CASE(testBenchmarkStringPairEventReader) {
	TempFile tempEventFile {
		"INSERT \"yes\" 10",
		"INSERT \"no\" 9",
		"INSERT \"maybe\" 22",
		"READ \"yes\"",
		"READ \"maybe\"",
		"READ \"no\"",
		"SCAN \"maybe\" 2",
		"UPDATE \"yes\" 42",
		"READ \"yes\"",
	};

	BenchmarkEventReader<std::pair<const char*, uint64_t>*, idx::contenthelpers::PairPointerKeyExtractor> reader(tempEventFile.mFileName);
	BenchmarkEventList eventList { reader.readEvents() };

	std::vector<BenchmarkBaseEvent*> events;
	for(BenchmarkBaseEvent & baseEvent : eventList) {
		events.push_back(&baseEvent);
	}

	SpecificEventProcessor<std::pair<const char*, uint64_t>*, idx::contenthelpers::PairPointerKeyExtractor> processEvent;

	using SpecificInsertEvent = InsertEvent<std::pair<const char*, uint64_t>*, idx::contenthelpers::PairPointerKeyExtractor>;
	using SpecificLookupEvent = LookupEvent<std::pair<const char*, uint64_t>*, idx::contenthelpers::PairPointerKeyExtractor>;
	using SpecificScanEvent = ScanEvent<std::pair<const char*, uint64_t>*, idx::contenthelpers::PairPointerKeyExtractor>;
	using SpecificUpdateEvent = UpdateEvent<std::pair<const char*, uint64_t>*, idx::contenthelpers::PairPointerKeyExtractor>;

	std::pair<const char*, uint64_t>* yesValue;
	std::pair<const char*, uint64_t>* noValue;
	std::pair<const char*, uint64_t>* mayBeValue;

	processEvent(events[0], [&yesValue] (auto const & event){
		SpecificInsertEvent const & insertEvent = *reinterpret_cast<SpecificInsertEvent const *>(&event);
		yesValue = insertEvent.mValueToInsert;
		BOOST_REQUIRE_EQUAL(insertEvent.mValueToInsert->first, "yes");
		BOOST_REQUIRE_EQUAL(insertEvent.mValueToInsert->second, 10u);
	});
	processEvent(events[1], [&noValue] (auto const & event){
		SpecificInsertEvent const & insertEvent = *reinterpret_cast<SpecificInsertEvent const *>(&event);
		noValue = insertEvent.mValueToInsert;
		BOOST_REQUIRE_EQUAL(insertEvent.mValueToInsert->first, "no");
		BOOST_REQUIRE_EQUAL(insertEvent.mValueToInsert->second, 9u);
	});
	processEvent(events[2], [&mayBeValue] (auto const & event){
		SpecificInsertEvent const & insertEvent = *reinterpret_cast<SpecificInsertEvent const *>(&event);
		mayBeValue = insertEvent.mValueToInsert;
		BOOST_REQUIRE_EQUAL(insertEvent.mValueToInsert->first, "maybe");
		BOOST_REQUIRE_EQUAL(insertEvent.mValueToInsert->second, 22u);
	});
	processEvent(events[3], [&yesValue] (auto const & event){
		SpecificLookupEvent const & lookupEvent = *reinterpret_cast<SpecificLookupEvent const *>(&event);
		BOOST_REQUIRE(lookupEvent.mExpectedValue.compliesWith({true, yesValue}));
		BOOST_REQUIRE_EQUAL(lookupEvent.mKey, "yes");
	});
	processEvent(events[4], [&mayBeValue] (auto const & event){
		SpecificLookupEvent const & lookupEvent = *reinterpret_cast<SpecificLookupEvent const *>(&event);
		BOOST_REQUIRE(lookupEvent.mExpectedValue.compliesWith({true, mayBeValue}));
		BOOST_REQUIRE_EQUAL(lookupEvent.mKey, "maybe");
	});
	processEvent(events[5], [&noValue] (auto const & event){
		SpecificLookupEvent const & lookupEvent = *reinterpret_cast<SpecificLookupEvent const *>(&event);
		BOOST_REQUIRE(lookupEvent.mExpectedValue.compliesWith({true, noValue}));
		BOOST_REQUIRE_EQUAL(lookupEvent.mKey, "no");
	});
	processEvent(events[6], [&yesValue] (auto const & event){
		SpecificScanEvent const & scanEvent = *reinterpret_cast<SpecificScanEvent const *>(&event);
		BOOST_REQUIRE(scanEvent.mLastResult.compliesWith({true, yesValue}));
		BOOST_REQUIRE_EQUAL(scanEvent.mLookupKey, "maybe");
	});
	processEvent(events[7], [&yesValue] (auto const & event){
		SpecificUpdateEvent const & updateEvent = *reinterpret_cast<SpecificUpdateEvent const *>(&event);
		BOOST_REQUIRE_EQUAL(updateEvent.mNewValue->first, "yes");
		BOOST_REQUIRE_EQUAL(updateEvent.mNewValue->second, 42u);
		BOOST_REQUIRE_NE((void*) updateEvent.mNewValue, yesValue);
		BOOST_REQUIRE(updateEvent.mPreviousValue.compliesWith({true, yesValue}));
		yesValue = updateEvent.mNewValue;
	});
	processEvent(events[8], [&yesValue] (auto const & event){
		SpecificLookupEvent const & lookupEvent = *reinterpret_cast<SpecificLookupEvent const *>(&event);
		BOOST_REQUIRE(lookupEvent.mExpectedValue.compliesWith({true, yesValue}));
		BOOST_REQUIRE_EQUAL(lookupEvent.mKey, "yes");
		BOOST_REQUIRE_EQUAL(lookupEvent.mExpectedValue.mValue->second, 42u);
	});
}



BOOST_AUTO_TEST_SUITE_END()

}}