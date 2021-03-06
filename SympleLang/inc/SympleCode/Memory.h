#pragma once

#include <memory>
#include <spdlog/spdlog.h>

#if __SY_DEBUG
#define __SY_ASSERT(cond, msg, ...) if (!(cond)) { spdlog::critical("(Assertion Failed) '{}'@{}: " #msg, __FILE__, __LINE__, ##__VA_ARGS__); __debugbreak(); }
#else
#define __SY_ASSERT(cond, msg, ...)
#endif

#if __SY_BUILD_DLL
#define __SYC_API __declspec(dllexport)
#elif __SY_BUILD_STATIC
#define __SYC_API
#else
#define __SYC_API
#endif

namespace Symple
{
	using std::shared_ptr;
	using std::make_shared;

	using std::unique_ptr;
	using std::make_unique;

	using std::dynamic_pointer_cast;
}

#undef GetObject