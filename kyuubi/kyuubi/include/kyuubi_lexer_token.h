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
		�ӵ�ǰλ�ý���һ�� ID ���ţ�����ɹ����򷵻ؽ������� ID ���ȣ�
		���򣬷��� 0��
	*/
	int parseIdToken(xiyue::ConstStringPointer& p);

	int parseNumberToken(xiyue::ConstStringPointer& p, bool& isIntType);

	int parseStringToken(xiyue::ConstStringPointer& p, bool& isMissingQuote);
}
