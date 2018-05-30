#pragma once

#include <vector>
#include <map>
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
			size_t index = (mStartIndex + mSize) % mCapacity;
			mData[index] = val;
			mSize++;
		}
	}
	size_t size() { return mSize; }
	const T& last() const { 
		size_t index = (mStartIndex + mSize - 1) % mCapacity;
		return mData[index]; 
	}
	T& last() { 
		size_t index = (mStartIndex + mSize - 1) % mCapacity;
		return mData[index]; 
	}
	void restore() {
		mSize = std::min(mSize + 1, mCapacity);
	}
};

template<typename T>
class UndoStack {
	T *data;
	size_t size;

	RingStack<std::map<size_t, T>> previousStates;		//Shows changes required to return to previous state
	std::vector<std::map<size_t, T>> redoStates;
	bool lastStateUnfinished;

	void propagateLastState() {
		if (previousStates.size() <= 0)
			return;
		if (previousStates.last().size() > 0) {
			for (auto& it : previousStates.last()) {
				//Propogate new value to data, store old value in previousState
				T oldValue = data[it.first];
				data[it.first] = it.second;
				it.second = oldValue;
			}
		} 
		lastStateUnfinished = false;
	}
public:
	UndoStack(T *data, size_t size, size_t maxUndo) 
		:data(data), size(size), previousStates(maxUndo), lastStateUnfinished(false){}

	void modify(size_t element, T value) {
		try {
			//Store new value until propagated with propagateLastState()
			previousStates.last()[element] = value;
		} catch (out_of_range) {
			printf("UndoStack::modify -- Out of range exception\n");
		}
	}

	void startNewState() {
		redoStates.clear();
		propagateLastState();
		if (previousStates.size() == 0 || previousStates.last().size() > 0) {
			previousStates.push(map<size_t, T>());
			lastStateUnfinished = true;
		}
	}

	void undo(std::map<size_t, T>* changes) {
		if (previousStates.size() > 0 && previousStates.last().size() == 0) {
			previousStates.pop();
			lastStateUnfinished = false;
		}
		if (previousStates.size() > 0 && previousStates.last().size() > 0) {
			if(lastStateUnfinished)
				propagateLastState();
			//Build redo information and apply undo
			redoStates.push_back(std::map<size_t, T>());
			for (const auto &it : previousStates.last()) {
				redoStates.back()[it.first] = data[it.first];
				data[it.first] = it.second;
			}
			*changes = previousStates.last();	//Swap?
			previousStates.pop();
		}
	}
	void redo(std::map<size_t, T>* changes) {
		if (redoStates.size() > 0) {
			for (const auto &it : redoStates.back()) {
				data[it.first] = it.second;
			}
			previousStates.restore();
			lastStateUnfinished = false;
			*changes = redoStates.back();	//Swap?
			redoStates.pop_back();
		}
	}

};