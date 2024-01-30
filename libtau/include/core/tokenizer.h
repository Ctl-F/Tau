#pragma once

#include "core.h"

#include <stack>

namespace tau {

	class TokenStream : public std::vector<token> {
	public:
		TokenStream();
		~TokenStream();

		void reset_cursor();

		bool expect(TokenType type);
		bool expect(std::string_view token);

		token& next();
		void consume();

		void mark();
		void fail();
		void pass();

		bool eof();

		token& peek();

	private:
		u64 m_CurrentIndex;
		std::stack<u64> m_StoredIndices;
		token m_StaticEOF;
	};

	result<bool> Tokenize(std::string_view input, std::string_view filename, TokenStream& output);
}
