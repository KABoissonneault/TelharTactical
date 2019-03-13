#pragma once

#include <map>

template<typename Key, typename T, typename Compare, typename Alloc, typename Pred>
void erase_if(std::map<Key, T, Compare, Alloc>& c, Pred pred) {
	for(auto i = c.begin(), last = c.end(); i != last; ) {
		if(pred(*i)) {
			i = c.erase(i);
		} else {
			++i;
		}
	}
}