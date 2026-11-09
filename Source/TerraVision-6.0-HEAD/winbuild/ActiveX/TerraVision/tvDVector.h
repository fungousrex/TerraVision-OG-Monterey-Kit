#ifndef __tvDVector_H__
#define __tvDVector_H__

#include <limits.h>
#include <stdlib.h>

typedef long ssize_t;

template <class T> class tvDVector {
public:
   T    *data;
   ssize_t minIndex;
   ssize_t maxIndex;
   ssize_t len;
   ssize_t size;
   ssize_t extension;
   bool doFree;
   typedef void* MallocFunc(size_t size);
   MallocFunc* dvectorMalloc;
   typedef void FreeFunc(void* ptr);
   FreeFunc* dvectorFree;

public:
	// Creating and destroying
	inline  tvDVector()	{               
		data          = (T *)0;
		minIndex      = INT_MIN/2;
		maxIndex      = INT_MIN/2;
		len           = 0;
		size          = 0;
		extension     = 32;
		doFree        = 1;
		dvectorMalloc = malloc;
		dvectorFree   = free;
	}
	
	// Constructor, sets extension to e
	inline  tvDVector( ssize_t e ) {
		data       = (T *)0;
		minIndex   = INT_MIN/2;
		maxIndex   = INT_MIN/2;
		len        = 0;
		size       = 0;
		extension  = e;
		doFree     = 1;
		dvectorMalloc = malloc;
		dvectorFree   = free;
	}
	
	~tvDVector( void ) {
		if (data && doFree)	{
			T *d, *e;
			
			for ( d = data, e = &data[size];
			d < e;
			d++ )	{
				d->~T();
			}
			
			(*dvectorFree)( data );
		}
	}
	
	
	// Accessor functions
	inline void setExtension( ssize_t e ) { extension = e;}
	
	// Set the freeing of the data arrays
	inline void setDoFree( bool df) { doFree = df;}
	
	// Get the index range
	inline void getIndexRange( ssize_t *min, ssize_t *max ) { *min = minIndex; *max = maxIndex;}
	
	inline T  *getDataPtr() { return &data[-minIndex];}
	
	inline ssize_t getLen() { return len;}
	
	inline bool getDoFree() { return doFree;}
	
	inline void setMalloc(MallocFunc* newMalloc) { dvectorMalloc = newMalloc; }
	
	inline void* getMalloc() { return dvectorMalloc; }
	
	inline void setFree(FreeFunc* newFree) { dvectorFree = newFree; }
	
	inline void* getFree() { return dvectorFree; }
	
	inline void removeElm( ssize_t index );
	
private:
	// Array extension routine
	void extend( ssize_t index, tvDVector<T> *dv );
	
public:
	inline T&  operator []( ssize_t index ) {
		ssize_t j = maxIndex - index;
		
		if ( j < 0 ) {
			extend(index, this);
		}
		
		if ( len < index + 1 ) len = index + 1;
		return data[index];
	}
	
	inline T&  operator ()( ssize_t index ) const {
		return data[index];
	}
};

// An unique placement operator
class tvUniqType {
public:
	tvUniqType() {};
	
	int foo;
};

inline void* operator new(size_t , void *t, tvUniqType *) { return t; }

template <class T>void tvDVector<T>::extend( ssize_t index, tvDVector<T> *dv ) {
  ssize_t   newMinIndex, newMaxIndex;
  ssize_t   newSize;
  ssize_t   beginRange, endRange;
  T    *tmp;
	
  // Deal with the initial zero range case
  if ( dv->size == 0 ) {
		dv->minIndex = 0;
		dv->maxIndex = index;
  }
	
  // Set up the new bounds around index. If this is the first
  // time through and the index is small and positive
  // assume that the index will never be less than zero.
  if ( 0 <= index && index <= dv->extension && dv->size == 0 ) {
		beginRange = 0;
  }
  else {
    beginRange = index - dv->extension;
  }
	
  endRange   = index + dv->extension;
	
  // Extend the overall range of the indices
  // based on the min/max of the old range
  // plus the range about the index
  if ( beginRange < dv->minIndex ) {
		newMinIndex = beginRange;
  }
  else {
		newMinIndex = dv->minIndex;
  }
	
  if ( endRange > dv->maxIndex ) {
		newMaxIndex = endRange - 1;
  }
  else {
		newMaxIndex = dv->maxIndex;
  }
	
  // Make the new length the range of the new Max
  newSize = newMaxIndex - newMinIndex + 1;
	
  // Create a new vector size of the new length
  tmp = (T*) (*dvectorMalloc)( sizeof(T)*newSize );
	
  // Create, copy and create elements of the new array
  {
    T *d, *e, *t;
		
    // Construct the beginning part of the newly allocated space
    for ( t = tmp, e = &tmp[dv->minIndex - newMinIndex]; t < e;	t++ ) {
			::new(t, (tvUniqType*) NULL ) T;
    }
		
    // Copy the data from old to new
    for ( d = dv->data, t = &tmp[dv->minIndex - newMinIndex], e = &tmp[dv->minIndex - newMinIndex + dv->size]; t < e;	d++, t++ ) {
			*t = *d;
    }
		
    // Finish constructing the left over newly allocated stuff
    for ( t = &tmp[dv->minIndex - newMinIndex + dv->size], e = &tmp[newSize];	t < e; t++ ) {
			::new(t, (tvUniqType*) NULL ) T;
    }
  }
	
  // Give back the old data
  if (dv->data) 
		(*dvectorFree)(dv->data);
	
  // Remember the new range values
  dv->minIndex = newMinIndex;
  dv->maxIndex = newMaxIndex;
  dv->size     = newSize;
	
  // Save the pointer to the vector
  dv->data = tmp;
}

template <class T> void tvDVector<T>::removeElm( ssize_t index ) {
	ssize_t startInd = index; 
	
	if (startInd >= len)
		return;
	
	ssize_t i;
	T *dataPtr;
	for (i = startInd, dataPtr = data+startInd; i < len-1; i++, dataPtr++) {
		*dataPtr = *(dataPtr+1);
	}
	len--;
}

#endif

