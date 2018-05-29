#pragma once

#include <vector>
#include <utility>
#include <stdexcept>
#include <algorithm>

template<typename T>
class RingStack {
	size_t mCapacity;
	size_t mSize;
	size_t mStartIndex;
	std::vector<T> mData;
public:
	RingStack(size_t capacity) :mCapacity(capacity), mSize(0), mStartIndex(0), mData(mCapacity) {}
	void pop() {
		mSize = std::max(mSize - 1, size_t(0));
	}
	void push(const T& val) {
		if (mSize == mCapacity) {
			mData[mStartIndex] = val;
			mStartIndex = (mStartIndex + 1) % mCapacity;
		}
		else {
			mData[mSize] = val;
			mSize++;
		}
	}
	size_t size() { return mSize; }
	const T& last() const { 
		size_t index = (mStartIndex + mSize - 1) % mCapacity;
		return mData[mSize - 1]; 
	}
	T& last() { 
		size_t index = (mStartIndex + mSize - 1) % mCapacity;
		return mData[mSize - 1]; 
	}
};

template<typename T>
class UndoStack {
	T *data;
	size_t size;

	RingStack<std::vector<std::pair<size_t, T>>> previousStates;		//Shows changes required to return to previous state
public:
	UndoStack(T *data, size_t size, size_t maxUndo) :data(data), size(size), previousStates(maxUndo){}

	void modify(size_t element, T value) {
		try {
			previousStates.last().push_back({ element, data[element] });
		} catch (out_of_range) {
			printf("UndoStack::modify -- Out of range exception\n");
		}
	}

	void startNewState() {
		for (int i = 0; previousStates.size() > 0 && i < previousStates.last().size(); i++) {
			data[previousStates.last()[i].first] = previousStates.last()[i].second;
		}
		if(previousStates.size() == 0 || previousStates.last().size() > 0)
			previousStates.push(vector<std::pair<size_t, T>>());
	}

	void undo(std::vector<std::pair<size_t, T>>* changes) {
		if (previousStates.size() > 0) {
			changes->swap(previousStates.last());
			previousStates.pop();
		}
	}
};