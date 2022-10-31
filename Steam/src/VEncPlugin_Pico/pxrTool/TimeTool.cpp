#include "TimeTool.h"
#include <windows.h>
#include <chrono>
//-----------------------------------------------------------------------------
int64_t nowInNs(void)
//-----------------------------------------------------------------------------
{
	auto now = std::chrono::high_resolution_clock::now().time_since_epoch();
	auto nowInNs = std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
	return nowInNs;
}
uint64_t GetTimestampUs() {
	return nowInNs() / 1000 ;
}
uint64_t GetTimestampMs() {
	return nowInNs() / 1000000;
}
//
//uint64_t GetTimestampUs() {
//	FILETIME ft;
//	GetSystemTimeAsFileTime(&ft);
//
//	uint64_t Current = (((uint64_t)ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
//	// Convert to Unix Epoch
//	Current -= 116444736000000000LL;
//	Current /= 10;
//
//	return Current;
//}

#if defined(__WIN32__) || defined(_WIN32)
// For Windoze, we need to implement our own gettimeofday()
#if !defined(_WIN32_WCE)
#include <sys/timeb.h>
#endif
void delay_us(int us)
{
	LARGE_INTEGER litmp;
	LONGLONG QPart1, QPart2;
	double dfMinus, dfFreq, dfTim;
	QueryPerformanceFrequency(&litmp);
	dfFreq = (double)litmp.QuadPart;// 获得计数器的时钟频率
	QueryPerformanceCounter(&litmp);
	QPart1 = litmp.QuadPart;// 获得初始值
	do {
		QueryPerformanceCounter(&litmp);
		QPart2 = litmp.QuadPart;//获得中止值
		dfMinus = (double)(QPart2 - QPart1);
		dfTim = dfMinus / dfFreq;// 获得对应的时间值，单位为秒
	} while (dfTim < (float)us / 1000000.00);
}
int gettimeofday(struct timeval* tp, int* /*tz*/) {
#if defined(_WIN32_WCE)
	  /* FILETIME of Jan 1 1970 00:00:00. */
	static const unsigned __int64 epoch = 116444736000000000LL;

	FILETIME    file_time;
	SYSTEMTIME  system_time;
	ULARGE_INTEGER ularge;

	GetSystemTime(&system_time);
	SystemTimeToFileTime(&system_time, &file_time);
	ularge.LowPart = file_time.dwLowDateTime;
	ularge.HighPart = file_time.dwHighDateTime;

	tp->tv_sec = (long)((ularge.QuadPart - epoch) / 10000000L);
	tp->tv_usec = (long)(system_time.wMilliseconds * 1000);
#else
	static LARGE_INTEGER tickFrequency, epochOffset;

	// For our first call, use "ftime()", so that we get a time with a proper epoch.
	// For subsequent calls, use "QueryPerformanceCount()", because it's more fine-grain.
	static bool isFirstCall = true;

	LARGE_INTEGER tickNow;
	QueryPerformanceCounter(&tickNow);

	if (isFirstCall) {
		struct timeb tb;
		ftime(&tb);
		tp->tv_sec = tb.time;
		tp->tv_usec = 1000 * tb.millitm;

		// Also get our counter frequency:
		QueryPerformanceFrequency(&tickFrequency);

		// And compute an offset to add to subsequent counter times, so we get a proper epoch:
		epochOffset.QuadPart
			= tb.time*tickFrequency.QuadPart + (tb.millitm*tickFrequency.QuadPart) / 1000 - tickNow.QuadPart;

		isFirstCall = false; // for next time
	}
	else {
		// Adjust our counter time so that we get a proper epoch:
		tickNow.QuadPart += epochOffset.QuadPart;

		tp->tv_sec = (long)(tickNow.QuadPart / tickFrequency.QuadPart);
		tp->tv_usec = (long)(((tickNow.QuadPart % tickFrequency.QuadPart) * 1000000L) / tickFrequency.QuadPart);
	}
#endif
	return 0;
}
#endif


void sleep_micro_seconds(ULONG ulMicroSeconds)
{
	LARGE_INTEGER varFrequency = { 0 }, varCounter_Start = { 0 }, varCounter_End = { 0 };
	LONGLONG llCount = 0;

	::QueryPerformanceFrequency(&varFrequency);
	::QueryPerformanceCounter(&varCounter_Start);

	while (true)
	{
		::QueryPerformanceCounter(&varCounter_End);

		llCount = varCounter_End.QuadPart - varCounter_Start.QuadPart;

		if (1000000 * llCount > ulMicroSeconds * varFrequency.QuadPart)
		{
			break;
		}
	}
}