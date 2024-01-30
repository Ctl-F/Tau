#include "core/ast.h"
#include "core/parser.h"

#include <sstream>


namespace tau {
	bool AstNode::compile(std::ostream& outputStream, ParserContext& ctx) {
		return true;
	}


	BinaryOperator::BinaryOperator(OperatorID _operator, AstNode* lhs, AstNode* rhs) : m_Operator{ _operator }, m_Lhs{ lhs }, m_Rhs{ rhs } {

	}
	BinaryOperator::~BinaryOperator() {
		if (m_Lhs != nullptr) {
			delete m_Lhs;
			m_Lhs = nullptr;
		}
		if (m_Rhs != nullptr) {
			delete m_Rhs;
			m_Rhs = nullptr;
		}
	}

	_type_id BinaryOperator::get_type(ParserContext& ctx) {
		_type_id a = dynamic_cast<Typed*>(m_Lhs)->get_type(ctx);
		_type_id b = dynamic_cast<Typed*>(m_Rhs)->get_type(ctx);

		_type_id STATIC_INT = ctx.types.get_id_from_name("long long");
		_type_id STATIC_FLOAT = ctx.types.get_id_from_name("double");

		_type_id ints[] = {
			ctx.types.get_id_from_name("i64"),
			ctx.types.get_id_from_name("u64"),
			ctx.types.get_id_from_name("i32"),
			ctx.types.get_id_from_name("u32"),
			ctx.types.get_id_from_name("i16"),
			ctx.types.get_id_from_name("u16"),
			ctx.types.get_id_from_name("i8"),
			ctx.types.get_id_from_name("u8"),
		};
		size_t ints_count = 8;

		_type_id floats[] = {
			ctx.types.get_id_from_name("f64"),
			ctx.types.get_id_from_name("f32"),
		};
		size_t floats_count = 2;

		for (size_t i = 0; i < ctx.operators.size(); i++) {
			if (ctx.operators[i].operator_ != m_Operator) continue;

			bool left_match = false;
			bool right_match = false;

			size_t true_a = -1;
			size_t true_b = -1;

			if (a == STATIC_INT) {
				for (size_t j = 0; j < ints_count; j++) {
					if (ints[j] == ctx.operators[i].left_type) {
						left_match = true;
						true_a = j;
						break;
					}
				}
			}
			else if (a == STATIC_FLOAT) {
				for (size_t j = 0; j < floats_count; j++) {
					if (floats[j] == ctx.operators[i].left_type) {
						left_match = true;
						true_a = j;
					}
				}
			}
			else {
				left_match = ctx.operators[i].left_type == a;
				true_a = a;
			}


			if (b == STATIC_INT) {
				for (size_t j = 0; j < ints_count; j++) {
					if (ints[j] == ctx.operators[i].right_type) {
						right_match = true;
						true_b = j;
					}
				}
			}
			else if (b == STATIC_FLOAT) {
				for (size_t j = 0; j < floats_count; j++) {
					if (floats[j] == ctx.operators[i].right_type) {
						right_match = true;
						true_b = j;
					}
				}
			}
			else {
				right_match = ctx.operators[i].right_type == b;
				true_b = b;
			}

			if (!left_match || !right_match) {
				continue;
			}

			return ctx.operators[i].resulting_type;
		}

		return 0;
	}

	UnaryOperator::UnaryOperator(OperatorID _operator, AstNode* child) : m_Operator{ _operator }, m_Child{ child } {
		
	}
	UnaryOperator::~UnaryOperator() {
		if (m_Child != nullptr) {
			delete m_Child;
			m_Child = nullptr;
		}
	}

	// implement pointers as secondary types??
	// TODO: Implement allowed unary operators
	// TODO: Implement operator overloading
	// TODO: Array/Access Operators
	// TODO: integrate scope
	// TODO: C Interfaced Functions
	// TODO: Templates (function and struct)
	// TODO: Alias and Imports
	// TODO: Better error checking
	// TODO: Actually compiling the tmp code
	// TODO: Finish build, implement "clean" and "run" commands
	// TODO: Add optimization targets to the project.toml
	// TODO: Build StdLib
	// TODO: Optional Types, arrays and accessors
	// TODO: Annotations
	// TODO: Static Reflection
	_type_id UnaryOperator::get_type(ParserContext& ctx) {
		_type_id a = dynamic_cast<Typed*>(m_Child)->get_type(ctx);

		_type_id STATIC_INT = ctx.types.get_id_from_name("long long");
		_type_id STATIC_FLOAT = ctx.types.get_id_from_name("double");

		_type_id ints[] = {
			ctx.types.get_id_from_name("i64"),
			ctx.types.get_id_from_name("u64"),
			ctx.types.get_id_from_name("i32"),
			ctx.types.get_id_from_name("u32"),
			ctx.types.get_id_from_name("i16"),
			ctx.types.get_id_from_name("u16"),
			ctx.types.get_id_from_name("i8"),
			ctx.types.get_id_from_name("u8"),
		};
		size_t ints_count = 8;

		_type_id floats[] = {
			ctx.types.get_id_from_name("f64"),
			ctx.types.get_id_from_name("f32"),
		};
		size_t floats_count = 2;

		for (size_t i = 0; i < ctx.uoperators.size(); i++) {
			if (ctx.uoperators[i].operator_ != m_Operator) continue;
		
			bool match = false;
			size_t true_a = -1;

			if (a == STATIC_INT) {
				for (size_t j = 0; j < ints_count; j++) {
					if (ints[j] == ctx.operators[i].left_type) {
						match = true;
						true_a = j;
						break;
					}
				}
			}
			else if (a == STATIC_FLOAT) {
				for (size_t j = 0; j < floats_count; j++) {
					if (floats[j] == ctx.operators[i].left_type) {
						match = true;
						true_a = j;
					}
				}
			}
			else {
				match = ctx.operators[i].left_type == a;
				true_a = a;
			}

			if (!match) {
				continue;
			}

			return ctx.uoperators[i].resulting_type;
		}

		return 0;
	}

	ListNode::ListNode() {};
	ListNode::~ListNode() {
		for (auto& ptr : entries) {
			delete ptr;
			ptr = nullptr;
		}
	}


	VariableNode::~VariableNode() {
		if (m_VariableName != nullptr) {
			delete m_VariableName;
			m_VariableName = nullptr;
		}
	}

	PathNode::~PathNode() {
		for (auto& pan : nodes) {
			if (pan.args != nullptr) {
				delete pan.args;
				pan.args = nullptr;
			}
		}
	}

	std::string PathSpecNode::get_full_name() {
		std::stringstream namebuf;


		for (size_t i = 0; i < bits.size(); i++) {
			auto& bit = bits[i];

			namebuf << bit.current;

			if (bit.template_parameters != nullptr) {
				// TODO:
				throw "NOT SUPPORTED";
			}

			if (i + 1 < bits.size()) {
				namebuf << "_";
			}
		}

		return namebuf.str();
	}

	/*
	if (ctx.active_symbol_scope->exists(m_VariableName->get_full_name(ctx))) {
			auto r = ctx.active_symbol_scope->get(m_VariableName->get_full_name(ctx));
			return r.type_id;
		}

		PathNode* name = this->m_VariableName;
		_type_id type = 0;

		auto node = name->nodes.begin();
		while (node != name->nodes.end()) {
			if (!ctx.active_symbol_scope->exists(node->bit)) {
				std::cout << "Unknown variable: " << node->bit << "\n";
				return 0;
			}

			type = ctx.active_symbol_scope->get(node->bit).type_id;

			while (ctx.types.is_struct(type)) {
				node++;
				type = ctx.types.get_struct_field_type(type, node->bit);
			}
			
			auto& bit = *node;

			if (++node == name->nodes.end()) {
				return type;
			}

			std::cout << "Error, " << bit.bit << " is not a struct type for member access.\n";
			return 0;
		}
		
		return 0;
	*/

	std::string PathNode::get_full_name(ParserContext& ctx) {
		std::stringstream namebuf;

		auto node = nodes.begin();
		ItemInfo current_info;
		_type_id type;
		while (node != nodes.end()) {
			if (!ctx.active_symbol_scope->exists(node->bit)) {
				if (nodes.begin()->bit != ctx.current_module->moduleName->bits[0].current) {
					PathNode* fullAttempt = new PathNode();
					for (size_t i = 0; i < ctx.current_module->moduleName->bits.size(); i++) {
						fullAttempt->nodes.push_back({
							ctx.current_module->moduleName->bits[i].current
							});
					}
					for (size_t i = 0; i < nodes.size(); i++) {
						fullAttempt->nodes.push_back(nodes[i]);
					}

					std::string res = fullAttempt->get_full_name(ctx);
					delete fullAttempt;
					return res;
				}


				std::cout << "Unknown symbol: " << node->bit << "\n";
			}
			current_info = ctx.active_symbol_scope->get(node->bit);

			if (current_info.is_module || current_info.is_enum || current_info.is_struct) {
				namebuf << node->bit;
				node++;
				if (node != nodes.end()) {
					namebuf << "_";
				}
				continue;
			}

			if(current_info.is_function){
				if ((node + 1) != nodes.end()) {
					std::cout << "Functions may not contain subtypes\n";
					return "";
				}
				namebuf << node->bit;
				break;
			}

			type = current_info.type_id;

			while (ctx.types.is_struct(type)) {
				namebuf << node->bit;
				node++;

				if (node != nodes.end()) {
					namebuf << ".";
				}

				type = ctx.types.get_struct_field_type(type, node->bit);
			}

			auto& bit = *node;

			if (++node == nodes.end()) {
				namebuf << bit.bit;
				break;
			}

			std::cout << "Error, " << bit.bit << " is not a struct type for member access.\n";
			return "";
		}
		//for (size_t i = 0; i < nodes.size(); i++) {
		//	auto& node = nodes[i];

		//	namebuf << node.bit;

		//	if (node.args != nullptr) {
		//		// not supported yet
		//		throw "NOT SUPPORTED";
		//	}

		//	if (i + 1 < nodes.size()) {
		//		
		//		namebuf << "_";
		//	}
		//}

		return namebuf.str();
	}

	std::string PathNode::get_local_name() {
		if (nodes.empty()) return "";
		return nodes.rbegin()->bit; // TODO: Tempalte
	}

	TemplateArgsNode::~TemplateArgsNode() {
		for (auto& nodep : template_args) {
			if (nodep != nullptr) {
				delete nodep;
				nodep = nullptr;
			}
		}
	}

	VariableDeclNode::VariableDeclNode(const std::string& name, _type_id type) : var_name{ name }, type{ type }, visibility{ Visibility::Private } {}

	StructMembersNode::~StructMembersNode() {
		for (auto& member : members) {
			delete member;
			member = nullptr;
		}
	}

	StructDefNode::StructDefNode(const std::string& name, StructMembersNode* members, TypeRegistry& registry) : struct_name{ name }, members{ members }, visibility{ Visibility::Private } {

		std::vector<FieldDef> fields;
		for (auto& var : members->members) {
			FieldDef field;
			field.name = var->var_name;
			field.type = var->type;

			fields.push_back(field);
		}

		auto r = registry.define_type(name, fields);
		
		if (r.error_bit) {
			throw r.error;
		}
		struct_id = r.value;
	}
	StructDefNode::~StructDefNode() {
		if (struct_name.args != nullptr) {
			delete struct_name.args;
			struct_name.args = nullptr;
		}

		delete members;
	}

	FunctionCallNode::~FunctionCallNode() {
		if (function_name != nullptr) {
			delete function_name;
			function_name = nullptr;
		}
		if (arguments != nullptr) {
			delete arguments;
			arguments = nullptr;
		}
	}

	_type_id FunctionCallNode::get_type(ParserContext& ctx) {
		if (ctx.active_symbol_scope->exists(function_name->get_full_name(ctx))) {
			auto item = ctx.active_symbol_scope->get(function_name->get_full_name(ctx));
			if (!item.is_function) {
				std::cout << function_name->get_full_name(ctx) << " is not defined as a function\n";
				return 0;
			}
			return item.type_id;
		}
		return 0;
	}

	bool FunctionCallNode::compile(std::ostream& output, ParserContext& ctx) {
		output << function_name->get_full_name(ctx) << "(";
		
		if (arguments != nullptr) {
			for (size_t i = 0; i < arguments->args.size(); i++) {
				if (!arguments->args[i]->compile(output, ctx)) {
					return false;
				}

				if (i + 1 < arguments->args.size()) {
					output << ", ";
				}
			}
		}
		output << ") ";
		return true;
	}

	ArgumentsNode::~ArgumentsNode() {
		for (auto& arg : args) {
			delete arg;
		}
	}


	bool ModuleNode::compile(std::ostream& output, ParserContext& ctx) {
		output << "// MODULE " << moduleName->get_full_name() << "\n";
		output << "#include <stdbool.h>\n";
		output << "#include <stdlib.h>\n\n";
		output << "#include \"tautypes.h\"\n";
		output << "#include \"" << moduleName->get_full_name() << ".h\"\n";


		output << "\n\n";
		ctx.current_module = this;
		ItemInfo self;
		self.is_module = true;
		ctx.active_symbol_scope->begin();
		ctx.active_symbol_scope->add(moduleName->get_full_name(), self);
		bool result = body->compile(output, ctx);
		ctx.current_module = nullptr;
		ctx.active_symbol_scope->end();

		output << "// END MODULE\n\n";

		return result;
	}

	bool ModuleNode::compile_header(std::ostream& output, ParserContext& ctx) {
		output << "#ifndef __" << moduleName->get_full_name() << "_H__\n";
		output << "#define __" << moduleName->get_full_name() << "_H__\n\n";

		ctx.current_module = this;
		bool result = body->compile_header(output, ctx);
		ctx.current_module = nullptr;

		output << "#endif //module\n\n";

		return result;
	}

	bool ModuleBodyNode::compile_header(std::ostream& output, ParserContext& ctx) {

		for (auto& inc : this->includes) {
			inc->compile(output, ctx);
		}

		for (auto& structDef : this->structs) {
			if (structDef->struct_name.args != nullptr) {
				continue; // todo
			}

			std::string fullName = ctx.current_module->moduleName->get_full_name();
			fullName += ".";
			fullName += structDef->struct_name.bit;

			std::vector<FieldDef> fields;
			for (auto& member : structDef->members->members) {
				FieldDef field;
				field.name = member->var_name;
				field.type = member->type;

				fields.push_back(field);
			}

			ctx.types.define_type(fullName, fields);

			output << "struct " << structDef->struct_name.bit << ";\n";
		}

		output << "\n\n";

		for (auto& structDef : this->structs) {

			if (structDef->struct_name.args != nullptr) {
				continue; // TODO: Implement
			}

			if (structDef->visibility != Visibility::Public) {
				continue;
			}

			if (!structDef->compile(output, ctx)) {
				return false;
			}
		}

		for (auto& funcDef : this->functions) {
			if (funcDef->templateParams != nullptr) {
				continue; // todo: implement
			}

			if (funcDef->visibility != Visibility::Public) {
				continue;
			}

			output << ctx.types.name_of(funcDef->returnType) << " " << funcDef->functionName << "(";

			

			if (funcDef->params != nullptr) {
				for (size_t i = 0; i < funcDef->params->params.size(); i++) {
					auto& param = funcDef->params->params[i];
					output << ctx.types.name_of(param.type);

					if (i + 1 < funcDef->params->params.size()) {
						output << ", ";
					}
				}
			}
			output << ");\n";
		}

		return true;
	}

	bool ModuleBodyNode::compile(std::ostream& output, ParserContext& ctx) {
		for (auto& inc : this->includes) {
			inc->compile(output, ctx);
		}

		for (auto& structDef : this->structs) {
			if (structDef->struct_name.args != nullptr) {
				continue; // todo
			}

			output << "struct " << structDef->struct_name.bit << ";\n";
		}
		output << "\n\n";

		for (auto& structDef : this->structs) {

			if (structDef->struct_name.args != nullptr) {
				continue; // TODO: Implement
			}

			if (structDef->visibility != Visibility::Private) {
				continue;
			}

			if (!structDef->compile(output, ctx)) {
				return false;
			}
		}

		for (auto& funcDef : this->functions) {
			if (funcDef->templateParams != nullptr) {
				continue; // todo: implement
			}

			if (funcDef->visibility == Visibility::Private) {
				output << "static ";
			}

			output << ctx.types.name_of(funcDef->returnType) << " " << funcDef->functionName << "(";

			if (funcDef->params != nullptr) {
				for (size_t i = 0; i < funcDef->params->params.size(); i++) {
					auto& param = funcDef->params->params[i];
					output << ctx.types.name_of(param.type);

					if (i + 1 < funcDef->params->params.size()) {
						output << ", ";
					}
				}
			}
			output << ");\n";
		}
		output << "\n\n";
		
		for (auto& funcDef : this->functions) {
			if (funcDef->templateParams != nullptr) {
				continue;
			}

			if (!funcDef->compile(output, ctx)) {
				return false;
			}
		}

		return true;
	}

	bool StructDefNode::compile(std::ostream& output, ParserContext& ctx) {
		if (struct_name.args != nullptr) {
			return false;
		}

		
		
		output << "struct " << struct_name.bit << " {\n";
		for (auto& member : members->members) {
			std::string ptr = "";
			output << "    " << ctx.types.name_of(member->type) << ptr << " " << member->var_name << ";\n";
		}
		output << "};\n\n";

		return true;
	}

	bool FunctionDefinitionNode::compile(std::ostream& output, ParserContext& ctx){
		if (templateParams != nullptr) {
			return false;
		}

		if (this->visibility == Visibility::Private) {
			output << "static ";
		}

		output << ctx.types.name_of(returnType) << " " << functionName << "(";

		if (params != nullptr) {
			for (size_t i = 0; i < params->params.size(); i++) {
				auto& param = params->params[i];

				output << ctx.types.name_of(param.type) << " " << param.name;

				if (i + 1 < params->params.size()) {
					output << ", ";
				}
			}
		}
		output << ") ";

		ItemInfo self;
		self.is_function = true;
		self.type_id = this->returnType;
		ctx.active_symbol_scope->add(functionName, self);

		ctx.active_symbol_scope->begin();

		if (params != nullptr) {
			for (auto& param : params->params) {
				ctx.active_symbol_scope->add_variable(param.name, param.type);
			}
		}

		if (!body->compile(output, ctx)) {
			return false;
		}

		ctx.active_symbol_scope->end();

		return true;
	}

	bool StatementBlockNode::compile(std::ostream& output, ParserContext& ctx) {
		output << "{\n";
		ctx.active_symbol_scope->begin();
		for (auto& statement : statements) {
			if (!statement->compile(output, ctx)) {
				return false;
			}
		}
		ctx.active_symbol_scope->end();

		output << "\n}\n\n";
		return true;
	}

	bool UnaryOperator::compile(std::ostream& output, ParserContext& ctx) {
		_type_id a = dynamic_cast<Typed*>(m_Child)->get_type(ctx);

		_type_id STATIC_INT = ctx.types.get_id_from_name("long long");
		_type_id STATIC_FLOAT = ctx.types.get_id_from_name("double");

		_type_id ints[] = {
			ctx.types.get_id_from_name("i64"),
			ctx.types.get_id_from_name("u64"),
			ctx.types.get_id_from_name("i32"),
			ctx.types.get_id_from_name("u32"),
			ctx.types.get_id_from_name("i16"),
			ctx.types.get_id_from_name("u16"),
			ctx.types.get_id_from_name("i8"),
			ctx.types.get_id_from_name("u8"),
		};
		size_t ints_count = 8;

		_type_id floats[] = {
			ctx.types.get_id_from_name("f64"),
			ctx.types.get_id_from_name("f32"),
		};
		size_t floats_count = 2;

		for (size_t i = 0; i < ctx.uoperators.size(); i++) {
			if (ctx.uoperators[i].operator_ != m_Operator) continue;

			bool match = false;
			size_t true_a = -1;

			if (a == STATIC_INT) {
				for (size_t j = 0; j < ints_count; j++) {
					if (ints[j] == ctx.operators[i].left_type) {
						match = true;
						true_a = j;
						break;
					}
				}
			}
			else if (a == STATIC_FLOAT) {
				for (size_t j = 0; j < floats_count; j++) {
					if (floats[j] == ctx.operators[i].left_type) {
						match = true;
						true_a = j;
					}
				}
			}
			else {
				match = ctx.operators[i].left_type == a;
				true_a = a;
			}

			if (!match) {
				continue;
			}

			if (ctx.uoperators[i].overload_function != nullptr) {
				FunctionCallNode* operatorFunc = dynamic_cast<FunctionCallNode*>(ctx.operators[i].overload_function);

				operatorFunc->arguments->args.push_back(m_Child);

				return operatorFunc->compile(output, ctx);
			}

			if (m_Operator == OperatorID::PostInc || m_Operator == OperatorID::PostDec) {
				if (!m_Child->compile(output, ctx)) return false;
				output << get_opstr(m_Operator) << " ";
				return true;
			}
			output << get_opstr(m_Operator);
			if (!m_Child->compile(output, ctx)) return false;
			return true;
		}

		return false;
	}

	bool BinaryOperator::compile(std::ostream& output, ParserContext& ctx) {
		_type_id a = dynamic_cast<Typed*>(m_Lhs)->get_type(ctx);
		_type_id b = dynamic_cast<Typed*>(m_Rhs)->get_type(ctx);

		_type_id STATIC_INT = ctx.types.get_id_from_name("long long");
		_type_id STATIC_FLOAT = ctx.types.get_id_from_name("double");

		_type_id ints[] = {
			ctx.types.get_id_from_name("i64"),
			ctx.types.get_id_from_name("u64"),
			ctx.types.get_id_from_name("i32"),
			ctx.types.get_id_from_name("u32"),
			ctx.types.get_id_from_name("i16"),
			ctx.types.get_id_from_name("u16"),
			ctx.types.get_id_from_name("i8"),
			ctx.types.get_id_from_name("u8"),
		};
		size_t ints_count = 8;

		_type_id floats[] = {
			ctx.types.get_id_from_name("f64"),
			ctx.types.get_id_from_name("f32"),
		};
		size_t floats_count = 2;

		for (size_t i = 0; i < ctx.operators.size(); i++) {
			if (ctx.operators[i].operator_ != m_Operator) continue;

			bool left_match = false;
			bool right_match = false;

			size_t true_a = -1;
			size_t true_b = -1;

			if (a == STATIC_INT) {
				for (size_t j = 0; j < ints_count; j++) {
					if (ints[j] == ctx.operators[i].left_type) {
						left_match = true;
						true_a = j;
						break;
					}
				}
			}
			else if (a == STATIC_FLOAT) {
				for (size_t j = 0; j < floats_count; j++) {
					if (floats[j] == ctx.operators[i].left_type) {
						left_match = true;
						true_a = j;
						break;
					}
				}
			}
			else {
				left_match = ctx.operators[i].left_type == a;
			}


			if (b == STATIC_INT) {
				for (size_t j = 0; j < ints_count; j++) {
					if (ints[j] == ctx.operators[i].right_type) {
						right_match = true;
						true_b = j;
						break;
					}
				}
			}
			else if (b == STATIC_FLOAT) {
				for (size_t j = 0; j < floats_count; j++) {
					if (floats[j] == ctx.operators[i].right_type) {
						right_match = true;
						true_b = j;
						break;
					}
				}
			}
			else {
				right_match = ctx.operators[i].right_type == b;
			}

			if (!right_match || !left_match) {
				continue;
			}

			if (ctx.operators[i].overload_function != nullptr) {
				FunctionCallNode* operatorFunc = dynamic_cast<FunctionCallNode*>(ctx.operators[i].overload_function);

				operatorFunc->arguments->args.push_back(m_Lhs);
				operatorFunc->arguments->args.push_back(m_Rhs);

				return operatorFunc->compile(output, ctx);
			}
			
			if (!m_Lhs->compile(output, ctx)) {
				return false;
			}

			output << " " << get_opstr(m_Operator) << " ";
			
			if (!m_Rhs->compile(output, ctx)) {
				return false;
			}

			return true;
		}
		std::cout << "Could not find operator implementation for binary operator: " << get_opstr(m_Operator) << " using (" <<
				ctx.types.name_of(a) << ", " << ctx.types.name_of(b) << ")\n";
		return false;
	}

	bool VariableDeclNode::compile(std::ostream& output, ParserContext& ctx) {
		output << ctx.types.name_of(type) << " " << var_name;

		if (default_value != nullptr) {
			output << " = ";
			if (!default_value->compile(output, ctx)) {
				return false;
			}
		}

		output << ";\n";

		ctx.active_symbol_scope->add_variable(var_name, type);

		return true;
	}

	bool VariableNode::compile(std::ostream& output, ParserContext& ctx) {
		if (!ctx.active_symbol_scope->exists(m_VariableName->get_full_name(ctx))) {
			std::cout << "Undeclared variable: " << m_VariableName->get_full_name(ctx) << "\n";
		}
		output << m_VariableName->get_full_name(ctx);
		return true;
	}

	bool IncludeNode::compile(std::ostream& output, ParserContext& ctx) {
		if (is_c_include) {
			output << "#include " << c_include << "\n";
		}
		return true;
	}

	bool InlineCBlock::compile(std::ostream& output, ParserContext& ctx) {
		for (auto& tok : tokens) {
			output << tok.literal << " ";
			if (tok.literal == ";") {
				output << "\n";
			}
		}

		return true;
	}

	bool ReturnNode::compile(std::ostream& output, ParserContext& ctx) {
		output << "return ";
		if (returnValue != nullptr) {
			returnValue->compile(output, ctx);
		}
		output << ";\n";

		return true;
	}


	bool IfNode::compile(std::ostream& output, ParserContext& ctx) {
		output << "if (";
		if (!condition->compile(output, ctx)) return false; // this is failing for if(n <= 1)? 
		output << ")\n";

		if (body != nullptr) {
			if (!body->compile(output, ctx)) return false;
		}

		if (elseBranch != nullptr) {
			if (!elseBranch->compile(output, ctx)) return false;
		}
		return true;
	}

	bool ElseNode::compile(std::ostream& output, ParserContext& ctx) {
		output << "else ";
		if (ifBranch != nullptr) {
			return ifBranch->compile(output, ctx);
		}
		if (body != nullptr) {
			if (!body->compile(output, ctx)) return false;
		}
		return true;
	}


	_type_id StaticIntegerNode::get_type(ParserContext& registry) {
		return registry.types.get_id_from_name("long long");
	}
	_type_id StaticFloatNode::get_type(ParserContext& registry) {
		return registry.types.get_id_from_name("double");
	}
	_type_id StaticBoolNode::get_type(ParserContext& registry) {
		return registry.types.get_id_from_name(m_TypeName);
	}
	_type_id StaticCharNode::get_type(ParserContext& registry) {
		return registry.types.get_id_from_name(m_TypeName);
	}
	_type_id StaticStringNode::get_type(ParserContext& registry) {
		return registry.types.get_id_from_name(m_TypeName);
	}
	_type_id VariableNode::get_type(ParserContext& ctx) {
		if (ctx.active_symbol_scope->exists(m_VariableName->get_full_name(ctx))) {
			auto r = ctx.active_symbol_scope->get(m_VariableName->get_full_name(ctx));
			return r.type_id;
		}

		PathNode* name = this->m_VariableName;
		_type_id type = 0;

		auto node = name->nodes.begin();
		while (node != name->nodes.end()) {
			if (!ctx.active_symbol_scope->exists(node->bit)) {
				std::cout << "Unknown variable: " << node->bit << "\n";
				return 0;
			}

			type = ctx.active_symbol_scope->get(node->bit).type_id;

			while (ctx.types.is_struct(type)) {
				node++;
				type = ctx.types.get_struct_field_type(type, node->bit);
			}
			
			auto& bit = *node;

			if (++node == name->nodes.end()) {
				return type;
			}

			std::cout << "Error, " << bit.bit << " is not a struct type for member access.\n";
			return 0;
		}
		
		return 0;
	}

}