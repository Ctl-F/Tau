#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <cstdint>
#include <memory>

typedef uint8_t u8;
typedef int8_t i8;
typedef uint16_t u16;
typedef int16_t i16;
typedef uint32_t u32;
typedef int32_t i32;
typedef uint64_t u64;
typedef int64_t i64;


namespace tau {

	enum class Visibility {
		Public,
		Private,
	};

	enum class TokenType {
		Undefined = 0,
		Identifier,
		Operator,
		Integer,
		Float,
		String,
		Char,
		Comment,
		Whitespace,
		Eof,
	};

	enum class OperatorID {
		Undefined = 0,
		Negative,
		Add,
		Sub,
		Mul,
		Div,
		Mod,
		Assign,
		Equals,
		NotEquals,
		LessThan,
		GreaterThan,
		LessEquals,
		GreaterEquals,
		PreInc,
		PreDec,
		PostInc,
		PostDec,
		Not,
		LogicAnd,
		LogicOr,
		BinaryAnd,
		BinaryOr,
		BinaryXor,
		BinaryNot,
		LeftShift,
		RightShift,
		Dereference,
		Reference,
		AddAssign,
		SubAssign,
		MulAssign,
		DivAssign,
		ModAssign,
		AndAssign,
		OrAssign,
		XorAssign,
		LeftShiftAssign,
		RightShiftAssign,
		Cast,
		ArrayAccess,
		Dot,
	};

	inline int get_operator_prec(OperatorID op) {
		switch (op) {
		default:
			return -1;
		case OperatorID::Dot:
			return 1;
		case OperatorID::Add:
		case OperatorID::Sub:
			return 4;
		case OperatorID::Mul:
		case OperatorID::Div:
		case OperatorID::Mod:
			return 5;
		case OperatorID::Negative:
		case OperatorID::PostInc:
		case OperatorID::PostDec:
		case OperatorID::Not:
		case OperatorID::BinaryNot:
		case OperatorID::Reference:
		case OperatorID::Dereference:
			return 2;
		case OperatorID::PreInc:
		case OperatorID::PreDec:
		case OperatorID::Cast:
			return 1;
		case OperatorID::LeftShift:
		case OperatorID::RightShift:
			return 5;
		case OperatorID::LessThan:
		case OperatorID::GreaterThan:
		case OperatorID::LessEquals:
		case OperatorID::GreaterEquals:
			return 6;
		case OperatorID::Equals:
		case OperatorID::NotEquals:
			return 7;
		case OperatorID::BinaryAnd:
			return 8;
		case OperatorID::BinaryXor:
			return 9;
		case OperatorID::BinaryOr:
			return 10;
		case OperatorID::LogicAnd:
			return 11;
		case OperatorID::LogicOr:
			return 12;
		case OperatorID::Assign:
		case OperatorID::AddAssign:
		case OperatorID::SubAssign:
		case OperatorID::MulAssign:
		case OperatorID::DivAssign:
		case OperatorID::ModAssign:
		case OperatorID::LeftShiftAssign:
		case OperatorID::RightShiftAssign:
		case OperatorID::AndAssign:
		case OperatorID::OrAssign:
		case OperatorID::XorAssign:
			return 14;

		}
	}

	inline OperatorID get_unary_operator(std::string_view lit, bool prefix = true) {
		if (lit == "-") {
			return OperatorID::Negative;
		}
		else if (lit == "*") {
			return OperatorID::Dereference;
		}
		else if (lit == "&") {
			return OperatorID::Reference;
		}
		else if (lit == "++") {
			return (prefix) ? OperatorID::PreInc : OperatorID::PostInc;
		}
		else if (lit == "--") {
			return (prefix) ? OperatorID::PreDec : OperatorID::PostDec;
		}
		else if (lit == "~") {
			return OperatorID::BinaryNot;
		}
		else if (lit == "!") {
			return OperatorID::Not;
		}
		else if (lit == "as") {
			return OperatorID::Cast;
		}

		return OperatorID::Undefined;
	}

	inline std::string get_opstr(OperatorID id) {
		switch (id) {
		default:
			return "[?]";
		case OperatorID::Undefined: return "[0]";
		case OperatorID::Dot: return ".";
		case OperatorID::Add: return "+";
		case OperatorID::Negative:
		case OperatorID::Sub: return "-";
		case OperatorID::Dereference:
		case OperatorID::Mul: 
			return "*";
		case OperatorID::Div: return "/";
		case OperatorID::Mod: return "%";
		case OperatorID::Assign: return "=";
		case OperatorID::Equals: return "==";
		case OperatorID::NotEquals: return "!=";
		case OperatorID::LessThan: return "<";
		case OperatorID::GreaterThan: return ">";
		case OperatorID::LessEquals: return "<=";
		case OperatorID::GreaterEquals: return ">=";
		case OperatorID::PreInc:
		case OperatorID::PostInc:
			return "++";
		case OperatorID::PostDec:
		case OperatorID::PreDec:
			return "--";
		case OperatorID::Not: return "!";
		case OperatorID::AndAssign: return "&=";
		case OperatorID::OrAssign: return "|=";
		case OperatorID::XorAssign: return "^=";
		case OperatorID::AddAssign: return "+=";
		case OperatorID::SubAssign: return "-=";
		case OperatorID::MulAssign: return "*=";
		case OperatorID::DivAssign: return "/=";
		case OperatorID::ModAssign: return "%=";
		case OperatorID::LeftShiftAssign: return "<<=";
		case OperatorID::RightShiftAssign: return ">>=";
		case OperatorID::LeftShift: return "<<";
		case OperatorID::RightShift: return ">>";
		case OperatorID::LogicAnd: return "&&";
		case OperatorID::LogicOr: return "||";
		case OperatorID::Reference:
		case OperatorID::BinaryAnd: 
			return "&";
		case OperatorID::BinaryOr: return "|";
		case OperatorID::BinaryXor: return "^";
		case OperatorID::BinaryNot: return "~";
		}
	}

	inline OperatorID get_binary_operator(std::string_view lit) {
		if (lit == "+") return OperatorID::Add;
		else if (lit == "-") return OperatorID::Sub;
		else if (lit == "*") return OperatorID::Mul;
		else if (lit == "/") return OperatorID::Div;
		else if (lit == "%") return OperatorID::Mod;
		else if (lit == "=") return OperatorID::Assign;
		else if (lit == "==") return OperatorID::Equals;
		else if (lit == "!=") return OperatorID::NotEquals;
		else if (lit == "<") return OperatorID::LessThan;
		else if (lit == ">") return OperatorID::GreaterThan;
		else if (lit == "<=") return OperatorID::LessEquals;
		else if (lit == ">=") return OperatorID::GreaterEquals;
		else if (lit == ">>") return OperatorID::LeftShift;
		else if (lit == "<<") return OperatorID::RightShift;
		else if (lit == ">>=") return OperatorID::LeftShiftAssign;
		else if (lit == ">>=") return OperatorID::RightShiftAssign;
		else if (lit == "&") return OperatorID::BinaryAnd;
		else if (lit == "&&") return OperatorID::LogicAnd;
		else if (lit == "|") return OperatorID::BinaryOr;
		else if (lit == "||") return OperatorID::LogicOr;
		else if (lit == "^") return OperatorID::BinaryXor;
		else if (lit == "+=") return OperatorID::AddAssign;
		else if (lit == "-=") return OperatorID::SubAssign;
		else if (lit == "*=") return OperatorID::MulAssign;
		else if (lit == "/=") return OperatorID::DivAssign;
		else if (lit == "%=") return OperatorID::ModAssign;
		else if (lit == ".") return OperatorID::Dot;
		return OperatorID::Undefined;
	}

	struct token {
		std::string_view literal;
		TokenType type;
		u64 row;
		u64 col;
		std::string source_file;
	};

	template<typename _ty> 
	struct result {
		_ty value;
		bool error_bit = 0;
		std::string error = "";
	
		inline static result<_ty> Ok(const _ty& value) {
			result<_ty> r;
			r.value = value;
			r.error_bit = false;

			return r;
		}

		inline static result<_ty> Err(const std::string& message) {
			result<_ty> r;
			r.error_bit = true;
			r.error = message;

			return r;
		}
	};


}
