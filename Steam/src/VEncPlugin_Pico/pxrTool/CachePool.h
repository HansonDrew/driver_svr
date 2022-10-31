#pragma once

#include <list>
#include <mutex>


typedef void* pxrHandle;

template<typename T> class pxrCachePool
{

	struct Cache
	{
		T* data = nullptr;
		bool isAvailable = false; //if true, has a useful data.
		bool isUsing = false;  //if true, still used by others.
		int length = 0;
		int maxCapacity = 0;
		void* reserved;   // reserved for user.
	};

public:

	static const int DEFAULT_MAX_CACHE_QUEUE_LENGTH = 10;

	explicit pxrCachePool(int maxCacheLength = DEFAULT_MAX_CACHE_QUEUE_LENGTH);
	~pxrCachePool();

	//
	bool Put(T* data, int len, void* userData = nullptr);

	//
	pxrHandle Get(T** outData, int& outLen, void** outUserData);

	//
	void Finish(pxrHandle handle);

	//
	size_t GetSize();

private:

	//Create a new cache, data can be nullptr if you only want to create a empty cache.
	struct Cache* CacheCreate(T* data, int len, void* userData = nullptr);

	//Recreate a cache if the len > capacity, data can be nullptr if you only want to recreate this cache.
	bool CacheRecreate(struct Cache* cache, T* data, int len, void* userData = nullptr);

	//
	void CacheDestroy(struct Cache** cache);

	//
	bool CachePut(T* data, int len, void* userData = nullptr);

	//
	pxrHandle CacheGet(T** data, int& len, void** userData);

	//	//
	void CacheFinish(pxrHandle handle);

	
	std::list<struct Cache*> cacheQueue;
	int maxQueueSize;

	std::mutex mutex;

	int maxCacheQueueLength; //The max capacity of one cache.
};


template <typename T>
pxrCachePool<T>::pxrCachePool(int maxCacheLength)
	: maxCacheQueueLength(maxCacheLength)
{

}

template <typename T>
pxrCachePool<T>::~pxrCachePool()
{
}

template <typename T>
bool pxrCachePool<T>::Put(T* data, int len, void* userData)
{
	return CachePut(data, len, userData);
}

template <typename T>
pxrHandle pxrCachePool<T>::Get(T** outData, int& outLen, void** outUserData)
{
	return CacheGet(outData, outLen, outUserData);
}

template <typename T>
void pxrCachePool<T>::Finish(pxrHandle handle)
{
	return CacheFinish(handle);
}

template <typename T>
size_t pxrCachePool<T>::GetSize()
{
	std::lock_guard<std::mutex> guard(mutex);
	return cacheQueue.size();
}

template <typename T>
typename pxrCachePool<T>::Cache* pxrCachePool<T>::CacheCreate(T* data, int len, void* userData)
{
	struct Cache* cache = new Cache();

	cache->data = static_cast<T*>(malloc(len));
	if (nullptr == cache->data)
	{
		delete cache;
		return nullptr;
	}// malloc failed.

	cache->reserved = userData;
	cache->maxCapacity = len;

	if (nullptr != data && 0 == memcpy_s(cache->data, cache->maxCapacity, data, len))
	{
		cache->isAvailable = true;
		cache->length = len;
	}

	return cache;

}

template <typename T>
bool pxrCachePool<T>::CacheRecreate(Cache* cache, T* data, int len,void* userData)
{
	bool isSuccess = false;
	if (len > cache->maxCapacity)
	{
		void* pre = cache->data;
		T* now = static_cast<T*>(realloc(pre, len));
		if (now == nullptr)
		{
			return isSuccess;
		}

		if (now != pre)
		{
			cache->data = now;
		}
		cache->maxCapacity = len;
	}

	if (nullptr != data && 0 == memcpy_s(cache->data, cache->maxCapacity, data, len))
	{
		cache->isAvailable = true;
		cache->length = len;
	}

	cache->reserved = userData;

	isSuccess = true;

	return isSuccess;
}

template <typename T>
void pxrCachePool<T>::CacheDestroy(Cache** cache)
{
	if (nullptr == (*cache))
	{
		return;
	}

	if (nullptr != (*cache)->data)
	{
		free((*cache)->data);
	}

	delete (*cache);
	*cache = nullptr;
}

template <typename T>
bool pxrCachePool<T>::CachePut(T* data, int len, void* userData)
{
	std::lock_guard<std::mutex> guard(mutex);


	struct Cache* freeCache = nullptr;
	for (std::list<struct Cache*>::iterator it = cacheQueue.begin(); it != cacheQueue.end(); ++it)
	{
		if (!(*it)->isUsing)
		{
			freeCache = (*it);
		}
	}

	bool isSuccess = false;
	if (freeCache == nullptr)
	{
		if (cacheQueue.size() < maxCacheQueueLength)
		{
			//Queue is not full, Create a new cache and copy the data to the cache, insert to the queue.
			freeCache = CacheCreate(data, len, userData);
			if (nullptr != freeCache)
			{
				isSuccess = true;
				cacheQueue.push_back(freeCache);
			}
		}
	}
	else
	{
		//Recreate the existing memory if necessary(len > maxCacheQueueLength), copy the data to the cache.
		isSuccess = CacheRecreate(freeCache, data, len, userData);
	}

	return isSuccess;
}

template <typename T>
pxrHandle pxrCachePool<T>::CacheGet(T** data, int& len, void** userData)
{
	std::lock_guard<std::mutex> guard(mutex);
	struct Cache* cache = nullptr;

	if (cacheQueue.empty())
	{
		return cache;
	}

	if (cacheQueue.front()->isAvailable)
	{
		cache = cacheQueue.front();
		cache->isUsing = true;
		*data = cache->data;
		len = cache->length;
		*userData = cache->reserved;
		cacheQueue.pop_front();
	}

	return cache;
}

template <typename T>
void pxrCachePool<T>::CacheFinish(pxrHandle handle)
{
	std::lock_guard<std::mutex> guard(mutex);

	struct Cache* cache = static_cast<struct Cache*>(handle);
	cache->isUsing = false;
	cache->isAvailable = false;
	cacheQueue.push_back(cache);
}
