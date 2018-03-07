#ifndef __IDX__CONTENTHELPERS__C_STRING_COMPARATOR__HPP__
#define __IDX__CONTENTHELPERS__C_STRING_COMPARATOR__HPP__

namespace idx { namespace contenthelpers {

class CStringComparator {
public:
	inline bool operator()(const char* first, const char* second) const {
		return strcmp(first, second) < 0;
	};
};

} }

#endif

