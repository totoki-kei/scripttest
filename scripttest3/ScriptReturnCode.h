#pragma once

namespace Script {
	enum class ReturnCode {
		OK,
		Yield,
		Finished,
		Error,
	};
}