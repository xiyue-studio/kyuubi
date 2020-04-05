#pragma once

namespace kyuubi
{
	struct Token
	{
		uint32_t type;
		uint32_t offset;
		uint32_t length;
		xiyue::ConstString tokenString;

		inline bool is(uint32_t t) const { return t == type; }
		inline bool isNot(uint32_t t) const { return t != type; }
	};

	/*
		从当前位置解析一个 ID 符号，如果成功，则返回解析到的 ID 长度，
		否则，返回 0。
	*/
	int parseIdToken(xiyue::ConstStringPointer& p);

	int parseNumberToken(xiyue::ConstStringPointer& p, bool& isIntType);

	int parseStringToken(xiyue::ConstStringPointer& p, bool& isMissingQuote);
}
