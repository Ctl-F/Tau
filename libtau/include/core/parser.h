#pragma once

#include "core.h"
#include "tokenizer.h"
#include "ast.h"
#include "tau_types.h"

#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <sstream>

namespace tau {
	class AstNode;

	typedef std::unordered_map<std::string, AstNode*> TokenResultView;

	struct AllowedBinaryOperator {
		OperatorID operator_;
		_type_id left_type;
		_type_id right_type;
		_type_id resulting_type;

		AstNode* overload_function = nullptr;
	};
	struct AllowedUnaryOperator {
		OperatorID operator_;
		_type_id value_type;
		_type_id resulting_type;

		AstNode* overload_function = nullptr;
	};

	std::vector<AllowedBinaryOperator>& GetAllowedOperators();
	std::vector<AllowedUnaryOperator>& GetAllowedUnaryOperators();

	struct ItemInfo {
		bool is_struct = false;
		bool is_pointer = false;
		bool is_optional = false;
		bool is_function = false;
		bool is_enum = false;
		bool is_module = false;
		bool is_primitive_type = false;

		_type_id type_id = 0;
		PathNode* path_id = nullptr;
	};

	struct Scope {
		std::vector<std::unordered_map<std::string, ItemInfo>> frames;

		void begin();
		void end();

		bool exists(const std::string& name);
		ItemInfo get(const std::string& name);

		void add(const std::string& name, const ItemInfo& info);
		void add_variable(const std::string& name, _type_id type, bool is_pointer = false, bool is_optional = false);
	};

	struct ParserContext {
		TypeRegistry& types;
		std::vector<AllowedBinaryOperator>& operators;
		std::vector<AllowedUnaryOperator>& uoperators;
		std::unordered_set<std::string_view> flags;
		std::vector<std::string> errors;


		Scope* active_symbol_scope = nullptr;
		PathNode* current_namescope = nullptr;
		ModuleNode* current_module = nullptr;
		

		inline void begin_namescope(const std::string& name) {
			if (current_namescope == nullptr) {
				current_namescope = new PathNode();
			}
			current_namescope->nodes.push_back(
				PathArg{
					name, nullptr
				}
			);
		}

		inline void end_namescope() {
			current_namescope->nodes.pop_back();
		}

		inline std::string get_fully_qualified_name(const std::string& name) {
			if (current_namescope == nullptr ||
				current_namescope->nodes.empty()) {
				return name;
			}
			
			return current_namescope->get_full_name(*this) + "_" + name;
		}
	};

	struct RuleStep {
		std::string_view expected_string;
		TokenType expected_type;
		bool use_string = false;
		bool optional = false;
		std::string assignment_key = "";
		bool use_rule = false;
		std::string_view flag = "";
		bool is_nested = false;
		std::string_view open_nest = "";
		std::string_view close_nest = "";
	};

	class Rule : public std::vector<RuleStep> {
	public:	
		typedef std::function<AstNode*(ParserContext&,TokenResultView&)> ActionFn;

		std::function<AstNode* (ParserContext&, TokenResultView&)> Action;

	};



	class Parser {
	public:
		Parser();
		~Parser();

		ParserContext get_context();

		inline std::vector<Rule>& operator[](const std::string& key) {
			return m_Rules[key];
		}

		AstNode* parse_eval(TokenStream& tokens, const std::string& initial_rule);

	private:
		AstNode* eval_ruleset(TokenStream& tokens, std::vector<Rule>& ruleset);

	private:
		std::unordered_map<std::string, std::vector<Rule>> m_Rules;
		Scope m_TypeScope;
	};

	struct RuleBuilder {
		std::vector<Rule> currentRule;

		inline RuleBuilder& operator*(const RuleStep& step) {
			currentRule.rbegin()->push_back(step);
			return *this;
		}

		inline RuleBuilder& operator/(const Rule::ActionFn& function) {
			currentRule.rbegin()->Action = function;
			return *this;
		}

		inline RuleBuilder& operator%(const RuleStep& step) {
			currentRule.push_back(Rule{});
			currentRule.rbegin()->push_back(step);
			return *this;
		}

		inline std::vector<Rule> end() const {
			return currentRule;
		}
	};

	inline RuleBuilder begin() {
		RuleBuilder builder;
		builder.currentRule.push_back(Rule{});
		return builder;
	}

	inline RuleStep lit(const std::string_view& literal, bool optional = false, const std::string_view& flag = "") {
		return RuleStep{ literal, TokenType::Undefined, true, optional, "", false, flag };
	}

	inline RuleStep tok(TokenType type, const std::string& key = "", bool optional = false) {
		return RuleStep{ "", type, false, optional, key };
	}

	inline RuleStep rule(const std::string_view& ruleName, const std::string& key, bool optional = false) {
		return RuleStep{ ruleName, TokenType::Undefined, false, optional, key, true };
	}

	inline RuleStep grab_nested(const std::string_view& nest_open, const std::string_view& nest_close, const std::string& key) {
		return RuleStep{ "", TokenType::Undefined, false, false, key, false, "", true, nest_open, nest_close };
	}
	
	void InitializeTauParser(Parser& p);

	/*inline RuleBuilder& operator+(RuleBuilder& builder, const RuleStep step) {
		builder.currentRule.rbegin()->push_back(step);
		return builder;
	}

	inline RuleBuilder& operator>>(RuleBuilder& builder, Rule::ActionFn& function) {
		builder.currentRule.rbegin()->Action = function;
		return builder;
	}

	inline RuleBuilder& operator|(RuleBuilder& builder, const RuleStep step) {
		builder.currentRule.push_back(Rule{});
		builder.currentRule.rbegin()->push_back(step);
	}*/

	

}
