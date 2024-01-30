#include "core/tokenizer.h"

#include <sstream>

namespace tau {

	///////////////////////////////////////////////////////////////////////////////////////////////
	// Token Stream
	///////////////////////////////////////////////////////////////////////////////////////////////
	TokenStream::TokenStream() : m_CurrentIndex{ 0 }, m_StaticEOF{"", TokenType::Eof, 0, 0, ""} {}
	TokenStream::~TokenStream() {}

	void TokenStream::reset_cursor() {
		m_CurrentIndex = 0;
		
		while (!m_StoredIndices.empty()) {
			m_StoredIndices.pop();
		}
	}

	bool TokenStream::expect(TokenType type) {
		if (m_CurrentIndex < size()) {
			if (type == TokenType::Operator && (
				this->at(m_CurrentIndex).literal == "(" ||
				this->at(m_CurrentIndex).literal == ")" ||
				this->at(m_CurrentIndex).literal == "[" ||
				this->at(m_CurrentIndex).literal == "]" ||
				this->at(m_CurrentIndex).literal == "{" ||
				this->at(m_CurrentIndex).literal == "}" ||
				this->at(m_CurrentIndex).literal == ";" ||
				this->at(m_CurrentIndex).literal == ","
				)) {
				return false;
			}

			return this->at(m_CurrentIndex).type == type;
		}
		return false;
	}

	bool TokenStream::expect(std::string_view token) {
		if (m_CurrentIndex < size()) {
			return this->at(m_CurrentIndex).literal == token;
		}
		return false;
	}

	token& TokenStream::next() {
		if (m_CurrentIndex >= size()) { return m_StaticEOF; }
		return this->at(m_CurrentIndex++);
	}

	void TokenStream::consume() {
		m_CurrentIndex++;
	}

	void TokenStream::mark() {
		m_StoredIndices.push(m_CurrentIndex);
	}

	void TokenStream::fail() {
		m_CurrentIndex = m_StoredIndices.top();
		m_StoredIndices.pop();
	}

	void TokenStream::pass() {
		m_StoredIndices.pop();
	}

	bool TokenStream::eof() {
		return m_CurrentIndex >= size();
	}

	token& TokenStream::peek() {
		if (m_CurrentIndex >= size()) {
			return m_StaticEOF;
		}
		return at(m_CurrentIndex);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	// Tokenizer methods
	///////////////////////////////////////////////////////////////////////////////////////////////

	struct TokenizerState {
		std::string_view input;
		std::string_view src_file;
		u64 row;
		u64 col;
		token tmp_buffer;
	};

	static inline bool is_alpha(char ch) {
		return ('A' <= ch && ch <= 'Z') || ('a' <= ch && ch <= 'z') || ch == '_';
	}
	static inline bool is_number(char ch) {
		return '0' <= ch && ch <= '9';
	}
	static inline bool is_alphanum(char ch) {
		return is_alpha(ch) || is_number(ch);
	}
	static inline bool is_whitespace(char ch) {
		return ch < 33;
	}

	static inline bool is_operator(char ch) {
		return ((33 <= ch && ch <= 47) ||
			(58 <= ch && ch <= 64) ||
			(91 <= ch && ch <= 96) ||
			(ch > 122 && ch < 127)) && 
			ch != '_' && ch != '\'' && ch != '"';
	}

	static bool try_tokenize_identifier(TokenizerState& ctx){
		if (!is_alpha(ctx.input[0])) {
			return false;
		}
		u64 end = 1;
		for (size_t i = 1; i < ctx.input.size(); i++) {
			if (!is_alphanum(ctx.input[i])) {
				end = i;
				break;
			}
		}

		ctx.tmp_buffer.row = ctx.row;
		ctx.tmp_buffer.col = ctx.col;
		ctx.tmp_buffer.literal = ctx.input.substr(0, end);
		ctx.tmp_buffer.type = TokenType::Identifier;

		ctx.col += end;
		ctx.input = ctx.input.substr(end);

		return true;
	}

	static int get_count(std::string_view& input) {
		char first = input[0];
		char second = (input.size() > 1) ? input[1] : '\0';
		char third = (input.size() > 2) ? input[2] : '\0';
		
		switch (first) {
		case '+':
			switch (second) {
			case '+': return 2;
			case '=': return 2;
			default: return 1;
			}
			break;
		case '-':
			switch (second) {
			case '-': return 2;
			case '=': return 2;
			default: return 1;
			}
			break;
		case '*':
			switch (second)
			{
			case '=': return 2;
			default: return 1;
			}
			break;
		case '/':
			switch (second) {
			case '=': return 2;
			default: return 1;
			}
			break;
		case '%':
			switch (second) {
			case '=': return 2;
			default: return 1;
			}
			break;
		case '=':
			switch (second) {
			case '=': return 2;
			default: return 1;
			}
			break;
		case '!':
			switch (second) {
			case '=': return 2;
			default: return 1;
			}
			break;
		case '>':
			switch (second) {
			case '=': return 2;
			case '>':
				switch (third) {
				case '=': return 3;
				default: return 2;
				}
			default: return 1;
			}
			break;
		case '<':
			switch (second) {
			case '<':
				switch (third) {
				case '=': return 3;
				default: return 2;
				}
			case '=': return 2;
			default: return 1;
			}
			break;
		case '&':
			switch (second) {
			case '&': return 2;
			case '=': return 2;
			default: return 1;
			}
		case '|':
			switch (second) {
			case '|': return 2;
			case '=': return 2;
			default: return 1;
			}
		case '^':
			switch (second) {
			case '=': return 2;
			default: return 1;
			}
		default:
			return 1;
		}
	}

	static bool try_tokenize_operator(TokenizerState& ctx) {
		if (!is_operator(ctx.input[0])) {
			if (ctx.input[0] == 'a' && ctx.input.size() > 1 && ctx.input[1] == 's') {
				ctx.tmp_buffer.row = ctx.row;
				ctx.tmp_buffer.col = ctx.col;
				ctx.tmp_buffer.literal = ctx.input.substr(0, 2);
				ctx.tmp_buffer.type = TokenType::Operator;

				ctx.col += 2;
				ctx.input = ctx.input.substr(2);

				return true;
			}

			return false;
		}

		int count = get_count(ctx.input);
		
		ctx.tmp_buffer.row = ctx.row;
		ctx.tmp_buffer.col = ctx.col;
		ctx.tmp_buffer.literal = ctx.input.substr(0, count);
		ctx.tmp_buffer.type = TokenType::Operator;

		ctx.col += count;
		
		ctx.input = ctx.input.substr(count);

		return true;
	}

	static bool try_tokenize_integer(TokenizerState& ctx) {
		if (!is_number(ctx.input[0])) {
			return false;
		}

		u64 end = 1;
		for (size_t i = 1; i < ctx.input.size(); i++) {
			if (!is_number(ctx.input[i])) {
				end = i;
				break;
			}
		}

		ctx.tmp_buffer.row = ctx.row;
		ctx.tmp_buffer.col = ctx.col;
		ctx.tmp_buffer.literal = ctx.input.substr(0, end);
		ctx.tmp_buffer.type = TokenType::Integer;

		ctx.col += end;
		ctx.input = ctx.input.substr(end);

		return true;
	}

	static bool try_tokenize_float(TokenizerState& ctx) {
		if (!is_number(ctx.input[0]) && ctx.input[0] != '.') {
			return false;
		}

		bool has_decimal = ctx.input[0] == '.';
		u64 end = 1;
		for (size_t i = 1; i < ctx.input.size(); i++) {
			if (ctx.input[i] == '.') {
				if (has_decimal) {
					end = i;
					break;
				}
				has_decimal = true;
				continue;
			}

			if (!is_number(ctx.input[i])) {
				end = i;
				break;
			}
		}

		ctx.tmp_buffer.row = ctx.row;
		ctx.tmp_buffer.col = ctx.col;
		ctx.tmp_buffer.literal = ctx.input.substr(0, end);
		ctx.tmp_buffer.type = TokenType::Float;

		if (!has_decimal) {
			ctx.tmp_buffer.type = TokenType::Integer;
		}

		ctx.col = end;
		ctx.input = ctx.input.substr(end);

		return true;
	}

	static bool try_tokenize_string(TokenizerState& ctx) {
		if (ctx.input[0] != '"') {
			return false;
		}

		bool is_escaped = false;
		u64 lines = 0;
		u64 tcol = ctx.col;
		u64 end = 1;
		for (size_t i = 1; i < ctx.input.size(); i++) {
			tcol++;

			if (is_escaped) {
				is_escaped = false;
				continue;
			}

			if (ctx.input[i] == '\\') {
				is_escaped = true;
				continue;
			}

			if (ctx.input[i] == '\n') {
				lines++;
				tcol = 0;
			}

			if (ctx.input[i] == '"') {
				end = i + 1;
				break;
			}
		}

		ctx.tmp_buffer.row = ctx.row;
		ctx.tmp_buffer.col = ctx.col;
		
		ctx.row += lines;
		ctx.col = tcol;

		ctx.tmp_buffer.type = TokenType::String;
		ctx.tmp_buffer.literal = ctx.input.substr(0, end); // includes ""
		ctx.input = ctx.input.substr(end);

		return true;
	}

	static bool try_tokenize_char(TokenizerState& ctx) {
		if (ctx.input[0] != '\'') {
			return false;
		}

		u64 end = 1;
		i32 escaped = 0;

		for (size_t i = 1; i < ctx.input.size(); i++) {
			if (escaped > 0) {
				escaped--;
				continue;
			}

			if (ctx.input[i] == '\\') {
				escaped = 2;
				continue;
			}

			if (ctx.input[i] == '\'') {
				end = i + 1;
				break;
			}
		}

		if (end != 3 && end != 5) {
			return false;
		}

		ctx.tmp_buffer.row = ctx.row;
		ctx.tmp_buffer.col = ctx.col;
		ctx.tmp_buffer.type = TokenType::Char;
		ctx.tmp_buffer.literal = ctx.input.substr(0, end);

		ctx.col += end;
		ctx.input = ctx.input.substr(end);

		return true;
	}

	static bool try_tokenize_singleline_comment(TokenizerState& ctx) {
		bool start_comment = ctx.input[0] == '/' && ctx.input[1] == '/';
		if (ctx.input.size() < 2 || !start_comment) {
			return false;
		}

		u64 end = 2;

		while (end < ctx.input.size()) {
			if (ctx.input[end] == '\n') {
				break;
			}
			end++;
		}

		ctx.row++;
		ctx.col = 0;

		if (end < ctx.input.size()) {
			ctx.input = ctx.input.substr(end + 1);
		}
		else {
			ctx.input = "";
		}

		return true;
	}

	static bool try_tokenize_multiline_comment(TokenizerState& ctx) {
		bool start_comment = ctx.input[0] == '/' && ctx.input[1] == '*';

		if (ctx.input.size() < 4 || (!start_comment)) {
			return false;
		}
		
		u64 end = 2;
		u32 nest = 1;

		while (end < ctx.input.size() - 1) {
			if (ctx.input[end] == '/' && ctx.input[end + 1] == '*') {
				nest++;
				end += 2;
				ctx.col += 2;
				continue;
			}
			if (ctx.input[end] == '*' && ctx.input[end + 1] == '/') {
				nest--;
				end += 2;
				ctx.col += 2;

				if (nest == 0) {
					break;
				}
				continue;
			}

			if (ctx.input[end] == '\n') {
				ctx.row++;
				ctx.col = 0;
			}
			end++;
		}

		ctx.input = ctx.input.substr(end);

		return true;
	}

	static bool try_tokenize_comment(TokenizerState& ctx) {
		if (try_tokenize_singleline_comment(ctx)) {
			return true;
		}
		if (try_tokenize_multiline_comment(ctx)) {
			return true;
		}
		return false;
	}
	
	static bool try_tokenize_whitespace(TokenizerState& ctx) {
		if (!is_whitespace(ctx.input[0])) {
			return false;
		}
		
		u64 end = 0;
		while (end < ctx.input.size()) {
			if(!is_whitespace(ctx.input[end])) {
				break;
			}
			ctx.col++;
			
			if (ctx.input[end] == '\n') {
				ctx.row++;
				ctx.col = 0;
			}

			end++;
		}

		ctx.input = ctx.input.substr(end);

		return true;
	}

	static std::string s_rebuffer;

	result<bool> Tokenize(std::string_view input, std::string_view filename, TokenStream& output) {
		output.clear();

		s_rebuffer = std::string(input.begin(), input.end()) + ' ';
		input = std::string_view(s_rebuffer.c_str());

		TokenizerState state;
		state.input = input;
		state.col = 0;
		state.row = 0;
		state.src_file = filename;
		state.tmp_buffer.source_file = filename;
		
		while (!state.input.empty()) {
			if (try_tokenize_whitespace(state) ||
				try_tokenize_comment(state)) {
				continue;
			}

			if (try_tokenize_float(state) || 
				try_tokenize_integer(state) || 
				try_tokenize_string(state) ||
				try_tokenize_char(state) ||
				try_tokenize_operator(state) ||
				try_tokenize_identifier(state)) {
				output.push_back(state.tmp_buffer);
				continue;
			}
			
			std::stringstream message;
			message << "Unexpected Input in file " << filename << " on line " << (state.row + 1) << ", col " << (state.col + 1);
			return result<bool>::Err(message.str());
		}

		return result<bool>::Ok(true);
	}
}