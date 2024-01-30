#pragma once

#include "core.h"
#include "tau_types.h"

#include <iostream>

namespace tau {
	struct ParserContext;

	class AstNode {
	public:
		virtual ~AstNode() = default;

		virtual bool compile(std::ostream& outputStream, ParserContext& ctx);

		virtual void debug_print() {};
	protected:

	};

	class OrphanTokens : public AstNode {
	public:
		std::vector<token> tokens;
	};

	class Typed {
	public:
		virtual _type_id get_type(ParserContext& registry) = 0;
	};

	class StructDefNode;
	class FunctionDefinitionNode;
	class IncludeNode;

	class ModuleBodyNode : public AstNode {
	public:
		std::vector<StructDefNode*> structs;
		std::vector<FunctionDefinitionNode*> functions;
		std::vector<IncludeNode*> includes;

		bool compile_header(std::ostream& output, ParserContext& ctx);
		virtual bool compile(std::ostream& outputStream, ParserContext& ctx) override;
	};


	class InlineCBlock : public AstNode {
	public:
		std::vector<token> tokens;

		virtual bool compile(std::ostream& outputStream, ParserContext& ctx) override;
	};

	class StaticIntegerNode : public AstNode, public Typed {
	public:
		inline StaticIntegerNode(i64 value, std::string_view type_name) : m_Value{ value }, m_TypeName{ type_name } {

		}

		_type_id get_type(ParserContext& registry) override;

		inline i64 value() const {
			return m_Value;
		}

		inline void debug_print() override {
			std::cout << m_Value;
		}

		inline bool compile(std::ostream& output, ParserContext& ctx) {
			output << m_Value;
			return true;
		}

	private:
		i64 m_Value;
		std::string_view m_TypeName;
	};

	class StaticStringNode : public AstNode, public Typed {
	public:
		inline StaticStringNode(const std::string& value, std::string_view type_name) : m_Value{ value }, m_TypeName{ type_name } {

		}

		_type_id get_type(ParserContext& registry) override;

		inline std::string value() {
			return m_Value;
		}

		inline void debug_print() override {
			std::cout << m_Value;
		}

		inline bool compile(std::ostream& output, ParserContext& ctx) {
			output << "\"" << m_Value << "\"";
			return true;
		}

	private:
		std::string m_Value;
		std::string_view m_TypeName;
	};

	class StaticCharNode : public AstNode, public Typed {
	public:
		inline StaticCharNode(char value, std::string_view type_name) : m_Value{ value }, m_TypeName{ type_name } {

		}

		_type_id get_type(ParserContext& registry) override;

		inline char value() const {
			return m_Value;
		}

		inline void debug_print() override {
			std::cout << m_Value;
		}

		inline bool compile(std::ostream& output, ParserContext& ctx) {
			output << "'" << m_Value << "'";
			return true;
		}

	private:
		char m_Value;
		std::string_view m_TypeName;
	};

	class StaticBoolNode : public AstNode, public Typed {
	public:
		inline StaticBoolNode(bool value, std::string_view type_name) : m_Value{ value }, m_TypeName{ type_name } {

		}

		_type_id get_type(ParserContext& registry) override;

		inline bool value() const {
			return m_Value;
		}

		inline void debug_print() override {
			std::cout << m_Value;
		}

		inline bool compile(std::ostream& output, ParserContext& ctx) {
			output << (m_Value ? "true" : "false");
			return true;
		}

	private:
		bool m_Value;
		std::string_view m_TypeName;
	};

	class PathNode;

	class VariableNode : public AstNode, public Typed {
	public:
		inline VariableNode(PathNode* variableName) : m_VariableName{ variableName } {

		}
		~VariableNode();

		_type_id get_type(ParserContext& registry) override;

		bool compile(std::ostream& output, ParserContext& ctx) override;

	private:
		PathNode* m_VariableName = nullptr;
	};

	class StaticFloatNode : public AstNode, public Typed {
	public:
		inline StaticFloatNode(double value, std::string_view type_name) : m_Value{ value }, m_TypeName{ type_name } {

		}

		_type_id get_type(ParserContext& registry) override;

		inline double value() const {
			return m_Value;
		}

		inline bool compile(std::ostream& output, ParserContext& ctx) {
			output << m_Value;
			return true;
		}

	private:
		double m_Value;
		std::string_view m_TypeName;
	};

	class ArgumentsNode : public AstNode {
	public:
		~ArgumentsNode();

		std::vector<AstNode*> args;
	};

	class FunctionCallNode : public AstNode, public Typed {
	public:
		inline FunctionCallNode(PathNode* functionName, ArgumentsNode *Arguments)
			: function_name{ functionName }, arguments{Arguments } { }
		~FunctionCallNode();

		_type_id get_type(ParserContext& registry) override;

		bool compile(std::ostream& output, ParserContext& ctx) override;

		PathNode* function_name = nullptr;
		ArgumentsNode* arguments = nullptr;
	};

	class BinaryOperator : public AstNode, public Typed {
	public:
		BinaryOperator(OperatorID _operator, AstNode* lhs, AstNode* rhs);
		~BinaryOperator();
		_type_id get_type(ParserContext& registry) override;

		inline virtual void debug_print() override {
			std::cout << "(" << get_opstr(m_Operator) << ", ";
			m_Lhs->debug_print();
			std::cout << ", ";
			m_Rhs->debug_print();
			std::cout << ")";
		}

		bool compile(std::ostream& output, ParserContext& ctx);

		OperatorID m_Operator;
		AstNode *m_Lhs, *m_Rhs;
	};

	class UnaryOperator : public AstNode, public Typed {
	public:
		UnaryOperator(OperatorID _operator, AstNode* child);
		~UnaryOperator();

		_type_id get_type(ParserContext& registry) override;

		bool compile(std::ostream& output, ParserContext& ctx);

		inline virtual void debug_print() override {
			std::cout << "(" << get_opstr(m_Operator) << ", ";
			m_Child->debug_print();
			std::cout << ")";
		}

		OperatorID m_Operator;
		AstNode* m_Child;
	};

	class TemplateParamsNode;

	struct PathSpecBit {
		std::string current;
		TemplateParamsNode *template_parameters = nullptr;
	};

	class PathSpecNode :  public AstNode {
	public:
		inline PathSpecNode() {}

		std::string get_full_name();

		std::vector<PathSpecBit> bits;
	};

	class TemplateParamsNode : public AstNode {
	public:
		inline TemplateParamsNode() {}

		std::vector<std::string> params;
	};

	class AnnotationNode : public AstNode, public Typed {
	public:
		AnnotationNode(const std::string& annotation_type, std::vector<AstNode*> params, AstNode* body);
		
	private:
		std::string m_AnnotationType;
		std::vector<AstNode*> m_Params;
		AstNode* m_Body;
	};

	class ModuleNode : public AstNode {
	public:
		inline ModuleNode(PathSpecNode* name) : moduleName{ name } {}

		PathSpecNode* moduleName;
		ModuleBodyNode* body = nullptr;

		bool compile_header(std::ostream& output, ParserContext& ctx);
		virtual bool compile(std::ostream& output, ParserContext& ctx) override;
	};

	/*class ImportNode : public AstNode {
	public:
		ImportNode(PathSpecNode* pathSpec, PathSpecNode* alias);

	private:
		PathSpecNode* m_PathSpec;
		PathSpecNode* m_Alias;
	};*/

	class IncludeNode : public AstNode {
	public:
		
		PathSpecNode* includeName = nullptr;
		PathSpecNode* alias = nullptr;
		
		bool is_c_include = false;
		std::string c_include;

		virtual bool compile(std::ostream& output, ParserContext& ctx) override;
	};

	class UseNode : public AstNode {
	public:
		UseNode(PathNode* pathNode, PathNode* alias);

	private:
		PathNode* m_Path;
		PathNode* m_Alias;
	};

	class ListNode : public AstNode {
	public:
		ListNode();
		~ListNode();

		std::vector<AstNode*> entries;
	};

	class ReturnNode : public AstNode {
	public:
		AstNode* returnValue = nullptr;

		bool compile(std::ostream& output, ParserContext& ctx) override;
	};

	struct Param {
		_type_id type;
		std::string name;
	};

	class ParameterListNode : public AstNode {
	public:
		inline ParameterListNode() {}

		std::vector<Param> params;
	};

	class StatementBlockNode;

	class FunctionDefinitionNode : public AstNode {
	public:
		inline FunctionDefinitionNode() {}


		virtual bool compile(std::ostream& output, ParserContext& ctx) override;

		std::string functionName;
		ParameterListNode* params = nullptr;
		TemplateParamsNode* templateParams = nullptr;
		_type_id returnType;
		StatementBlockNode* body = nullptr;
		Visibility visibility = Visibility::Private;
	};

	class TemplateArgsNode;

	// std.vector<std.map<std.string, int>>
	struct PathArg {
		std::string bit;
		TemplateArgsNode* args = nullptr;
	};

	class PathNode : public AstNode {
	public:
		~PathNode();

		std::vector<PathArg> nodes;

		std::string get_full_name(ParserContext&);
		std::string get_local_name();
	};

	class TemplateArgsNode : public AstNode {
	public:
		inline TemplateArgsNode() {

		}
		~TemplateArgsNode();

		std::vector<PathNode*> template_args;
	};

	class VariableDeclNode : public AstNode {
	public:
		VariableDeclNode(const std::string& var_name, _type_id type);

		std::string var_name;
		_type_id type;
		Visibility visibility;
		
		AstNode* default_value = nullptr;
	
		bool compile(std::ostream& output, ParserContext& ctx) override;
	};

	class StructMembersNode : public AstNode {
	public:
		~StructMembersNode();

		std::vector<VariableDeclNode*> members;
	};

	// separate struct and struct template
	class StructDefNode : public AstNode {
	public:
		StructDefNode(const std::string& name, StructMembersNode *members, TypeRegistry& registry);
		~StructDefNode();

		virtual bool compile(std::ostream& output, ParserContext& ctx) override;

		Visibility visibility;
		PathArg struct_name;
		StructMembersNode* members = nullptr;
		_type_id struct_id;
	};

	class StatementBlockNode : public AstNode {
	public:
		std::vector<AstNode*> statements;

		virtual bool compile(std::ostream& output, ParserContext& ctx) override;
	};

	class IfNode;

	class ElseNode : public AstNode {
	public:
		IfNode* ifBranch = nullptr;
		StatementBlockNode* body = nullptr;

		bool compile(std::ostream& output, ParserContext& ctx) override;
	};

	class IfNode : public AstNode {
	public:
		AstNode* condition = nullptr;
		StatementBlockNode* body = nullptr;
		ElseNode* elseBranch = nullptr;

		bool compile(std::ostream& output, ParserContext& ctx) override;
	};

	
}