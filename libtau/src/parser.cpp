#include "core/parser.h"

namespace tau {

	void Scope::begin() {
		frames.push_back({});
	}
	void Scope::end() {
		frames.pop_back();
	}

	bool Scope::exists(const std::string& name) {
		for (auto f = frames.rbegin(); f != frames.rend(); ++f) {
			auto r = (*f).find(name);
			if (r != (*f).end()) {
				return true;
			}
		}
		return false;
	}
	
	ItemInfo Scope::get(const std::string& name) {
		for (auto f = frames.rbegin(); f != frames.rend(); ++f) {
			auto r = (*f).find(name);
			if (r != (*f).end()) {
				return r->second;
			}
		}
		
		return {};
	}

	void Scope::add(const std::string& name, const ItemInfo& info) {
		if (frames.empty()) {
			frames.push_back({});
		}
		(*frames.rbegin())[name] = info;
	}

	void Scope::add_variable(const std::string& name, _type_id type, bool is_pointer, bool is_optional) {
		ItemInfo info;
		info.type_id = type;
		info.is_pointer = is_pointer;
		info.is_optional = is_optional;

		add(name, info);
	}

	Parser::Parser() {
		ItemInfo t;
		t.is_primitive_type = true;

		auto& types = TypeRegistry::instance();

		t.type_id = types.get_id_from_name("void");
		m_TypeScope.add("void", t);

		t.type_id = types.get_id_from_name("i8");
		m_TypeScope.add("i8", t);

		t.type_id = types.get_id_from_name("i16");
		m_TypeScope.add("i16", t);

		t.type_id = types.get_id_from_name("i32");
		m_TypeScope.add("i32", t);

		t.type_id = types.get_id_from_name("i64");
		m_TypeScope.add("i64", t);

		t.type_id = types.get_id_from_name("u8");
		m_TypeScope.add("u8", t);

		t.type_id = types.get_id_from_name("u16");
		m_TypeScope.add("u16", t);

		t.type_id = types.get_id_from_name("u32");
		m_TypeScope.add("u32", t);

		t.type_id = types.get_id_from_name("u64");
		m_TypeScope.add("u64", t);

		t.type_id = types.get_id_from_name("f32");
		m_TypeScope.add("f32", t);

		t.type_id = types.get_id_from_name("f64");
		m_TypeScope.add("f64", t);

		t.type_id = types.get_id_from_name("char");
		m_TypeScope.add("char", t);

		t.type_id = types.get_id_from_name("bool");
		m_TypeScope.add("bool", t);

	}
	Parser::~Parser() {

	}

	AstNode* Parser::parse_eval(TokenStream& tokens, const std::string& initial_rule) {
		return eval_ruleset(tokens, m_Rules[initial_rule]);
	}
	
	ParserContext Parser::get_context() {
		auto ctx = ParserContext{ TypeRegistry::instance(), GetAllowedOperators(), GetAllowedUnaryOperators() };
		ctx.active_symbol_scope = &m_TypeScope;

		return ctx;
	}

	AstNode* Parser::eval_ruleset(TokenStream& tokens, std::vector<Rule>& ruleset) {
		ParserContext ctx = get_context();
		TokenResultView view;
		
		for (size_t i = 0; i < ruleset.size(); i++) {
			auto& rule = ruleset[i];
			bool succeed = true;
			tokens.mark();
			ctx.flags.clear();

			for (size_t j = 0; j < rule.size(); j++) {
				RuleStep& step = rule[j];

				if (tokens.eof()) {
					succeed = (j+1 >= rule.size() && step.optional); // if this is an optional last step we can succeed
					break;
				}

				if (step.use_string) {
					bool present = tokens.expect(step.expected_string);

					if (!present && !step.optional) {
						succeed = false;
						break;
					}

					if (present) {
						tokens.consume();
					}

					if (present && !step.flag.empty()) {
						ctx.flags.insert(step.flag);
					}

					continue;
				}

				if (step.use_rule) {
					auto& subset = m_Rules[step.expected_string.data()];
					AstNode* result = eval_ruleset(tokens, subset);

					if (result == nullptr && !step.optional) {
						succeed = false;
						break;
					}
					if (result != nullptr) {
						view[step.assignment_key] = result;
					}
					continue;
				}

				if (step.is_nested) {
					bool present = tokens.expect(step.open_nest);

					if (!present) {
						succeed = false;
						break;
					}
					tokens.consume();

					int nest = 1;
					OrphanTokens* toks = new OrphanTokens();

					while (true) {
						if (tokens.expect(step.close_nest)) {
							nest--;
							if (nest == 0) {
								break;
							}

							toks->tokens.push_back(tokens.next());
							continue;
						}

						if (tokens.expect(step.open_nest)) {
							nest++;
						}
						toks->tokens.push_back(tokens.next());

						if (tokens.eof() && nest > 0) {
							ctx.errors.push_back("Unclosed grouping");
							succeed = false;
							break;
						}
					}
					if (!succeed) {
						delete toks;
						break;
					}
					view[step.assignment_key] = toks;
					
					continue;
				}

				bool present = tokens.expect(step.expected_type);

				if (!present && !step.optional) {
					succeed = false;
					break;
				}

				if (present) {
					token& t = tokens.next();

					if (!step.assignment_key.empty()) {
						OrphanTokens* node = new OrphanTokens();
						node->tokens.push_back(t);

						view[step.assignment_key] = node;
					}
				}

				continue;
			}

			if (!succeed) {
				tokens.fail();

				for (auto& kv : view) {
					if (kv.second != nullptr) {
						delete kv.second;
					}
				}
				view.clear();

				continue;
			}

			AstNode* result = rule.Action(ctx, view);
			for (auto& kv : view) {
				if (kv.second != nullptr) {
					delete kv.second;
				}
			}
			view.clear();


			if (!ctx.errors.empty()) {
				tokens.fail();
				for (auto& err : ctx.errors) {
					std::cout << "Error: " << err << "\n";
				}
				std::cout << "On Line " << tokens.peek().row << ", " << tokens.peek().col << " in " << tokens.peek().source_file << "\n\n";
				ctx.errors.clear();
				return nullptr;
			}

			tokens.pass();

			return result;
		}

		//std::cout << "Unexpected token: " << tokens.peek().literal << " in file " << tokens.peek().source_file << " on line " << tokens.peek().row << ", " << tokens.peek().col << "\n";
		return nullptr;
	}


#define MOVE(ptr) ptr; ptr = nullptr
#define MOVE_CAST(type, ptr) dynamic_cast<type*>(ptr); ptr = nullptr

	void InitializeTauParser(Parser& parser) {
		parser["INT"] = (begin()
			* tok(TokenType::Integer, "value") / [](ParserContext& ctx, TokenResultView& view) {
				OrphanTokens* tok = dynamic_cast<OrphanTokens*>(view.at("value"));
				StaticIntegerNode* node = new StaticIntegerNode((i64)atoll(std::string{ tok->tokens[0].literal.begin(), tok->tokens[0].literal.end() }.c_str()), "i64");
				return node;
			}
		).end();

		parser["FLOAT"] = (begin()
			* tok(TokenType::Float, "value") / [](ParserContext& ctx, TokenResultView& view) {
				OrphanTokens* tok = dynamic_cast<OrphanTokens*>(view.at("value"));
				StaticFloatNode* node = new StaticFloatNode(atof(std::string{ tok->tokens[0].literal.begin(), tok->tokens[0].literal.end() }.c_str()), "f64");
				return node;
			}
		).end();

		parser["STRING"] = (begin()
			* tok(TokenType::String, "value") / [](ParserContext& ctx, TokenResultView& view) {
				OrphanTokens* tok = dynamic_cast<OrphanTokens*>(view.at("value"));
				StaticStringNode* node = new StaticStringNode(tok->tokens[0].literal.substr(1, tok->tokens[0].literal.length() - 1).data(), "string");
				return node;
			}
		).end();

		parser["CHAR"] = (begin()
			* tok(TokenType::Char, "value") / [](ParserContext& ctx, TokenResultView& view) {
				OrphanTokens* tok = dynamic_cast<OrphanTokens*>(view.at("value"));

				std::string_view literal = tok->tokens[0].literal;
				char ch = 0;

				if (literal.length() == 3) {
					ch = literal[1];
				}
				else if (literal.length() == 5) {
					char d = literal[2];
					char e = literal[3];

					ch = (
						'0' <= d && d <= '9' ? d - '0'
						: 'A' <= d && d <= 'F' ? (d - 'A' + 10)
						: 'a' <= d && d <= 'f' ? (d - 'a' + 10) : 0
						) * 16
						+
						('0' <= e && e <= '9' ? e - '0'
							: 'A' <= e && e <= 'F' ? (e - 'A' + 10)
							: 'a' <= e && e <= 'f' ? (e - 'a' + 10) : 0);
				}

				StaticCharNode* node = new StaticCharNode(ch, "char");
				return node;
			}
		).end();

		parser["BOOL"] = (begin()
			* lit("true") / [](ParserContext& ctx, TokenResultView& view) { return new StaticBoolNode(true, "bool"); }
			% lit("false") / [](ParserContext& ctx, TokenResultView& view) { return new StaticBoolNode(false, "bool"); }
		).end();


		parser["TEMPLATE_PARAMS_EXT"] = (begin()
			* lit(",") * tok(TokenType::Identifier, "param") * rule("TEMPLATE_PARAMS_EXT", "params", true)
								/ [](auto& ctx, auto& view) {
									OrphanTokens* param = dynamic_cast<OrphanTokens*>(view["param"]);
									AstNode* params = nullptr;

									auto f = view.find("params");
									if (f != view.end()) {
										params = f->second;
										view["args"] = nullptr;
									}

									if (params != nullptr) {
										TemplateParamsNode* as_params = dynamic_cast<TemplateParamsNode*>(params);
										as_params->params.insert(as_params->params.begin(), std::string(param->tokens[0].literal.begin(), param->tokens[0].literal.end()));

										return as_params;
									}

									TemplateParamsNode* tNode = new TemplateParamsNode();
									tNode->params.push_back(std::string(param->tokens[0].literal.begin(), param->tokens[0].literal.end()));

									return tNode;
								}
		).end();

		parser["TEMPLATE_PARAMS"] = (begin()
			* lit("<") * tok(TokenType::Identifier, "param") * rule("TEMPLATE_PARAMS_EXT", "params", true) * lit(">")
								/ [](auto& ctx, auto& view) {
									OrphanTokens* param = dynamic_cast<OrphanTokens*>(view["param"]);
									AstNode* params = nullptr;

									auto f = view.find("params");
									if (f != view.end()) {
										params = f->second;
										view["args"] = nullptr;
									}

									if (params != nullptr) {
										TemplateParamsNode* as_params = dynamic_cast<TemplateParamsNode*>(params);
										as_params->params.insert(as_params->params.begin(), std::string(param->tokens[0].literal.begin(), param->tokens[0].literal.end()));

										return as_params;
									}

									TemplateParamsNode* tNode = new TemplateParamsNode();
									tNode->params.push_back(std::string(param->tokens[0].literal.begin(), param->tokens[0].literal.end()));

									return tNode;
								}
		).end();

		parser["TEMPLATE_ARGS_EXT"] = (begin()
			* lit(",") * rule("PATH", "t0") * rule("TEMPLATE_ARGS_EXT", "args", true)
								/ [](auto& ctx, auto& view) {
									AstNode* path = MOVE(view["t0"]);
									AstNode* args = nullptr;

									PathNode* as_path = dynamic_cast<PathNode*>(path);

									auto f = view.find("args");
									if (f != view.end()) {
										args = f->second;
										view["args"] = nullptr;
									}

									if (args != nullptr) {
										TemplateArgsNode* as_targs = dynamic_cast<TemplateArgsNode*>(args);

										as_targs->template_args.insert(as_targs->template_args.begin(), as_path);

										return as_targs;
									}

									TemplateArgsNode* targs = new TemplateArgsNode();
									targs->template_args.push_back(as_path);

									return targs;
								}
		).end();

		parser["TEMPLATE_ARGS"] = (begin()
			* lit("<") * rule("PATH", "t0") * rule("TEMPLATE_ARGS_EXT", "args", true) * lit(">")
								/ [](auto& ctx, auto& view) {
									AstNode* path = view["t0"]; view["t0"] = nullptr;
									AstNode* args = nullptr;

									PathNode* as_path = dynamic_cast<PathNode*>(path);

									auto f = view.find("args");
									if (f != view.end()) {
										args = f->second;
										view["args"] = nullptr;
									}

									if (args != nullptr) {
										TemplateArgsNode* as_targs = dynamic_cast<TemplateArgsNode*>(args);

										as_targs->template_args.insert(as_targs->template_args.begin(), as_path);

										return as_targs;
									}

									TemplateArgsNode* targs = new TemplateArgsNode();
									targs->template_args.push_back(as_path);

									return targs;
								}
		).end();

		parser["PATH_EXT"] = (begin()
			* lit(".") * tok(TokenType::Identifier, "bit") * rule("TEMPLATE_ARGS", "bit_template", true) * rule("PATH_EXT", "ext", true)
								/ [](auto& ctx, auto& view) {
									AstNode* bit = view["bit"]; view["bit"] = nullptr;
									AstNode* bit_template = nullptr;
									AstNode* ext = nullptr;

									OrphanTokens* bit_tok = dynamic_cast<OrphanTokens*>(bit);

									auto f = view.find("bit_template");
									if (f != view.end()) {
										bit_template = f->second;
										view["bit_template"] = nullptr;
									}

									f = view.find("ext");
									if (f != view.end()) {
										ext = f->second;
										view["ext"] = nullptr;
									}

									PathArg pbit;
									pbit.bit = std::string{ bit_tok->tokens[0].literal.begin(), bit_tok->tokens[0].literal.end() };
									pbit.args = (bit_template == nullptr) ? nullptr : dynamic_cast<TemplateArgsNode*>(bit_template);

									if (ext != nullptr) {
										PathNode* _path = dynamic_cast<PathNode*>(ext);
										_path->nodes.insert(_path->nodes.begin(), pbit);

										return _path;
									}

									PathNode* path = new PathNode();
									path->nodes.push_back(pbit);

									return path;
								}
		).end();
		parser["PATH"] = (begin()
			* tok(TokenType::Identifier, "bit") * rule("TEMPLATE_ARGS", "bit_template", true) * rule("PATH_EXT", "ext", true)
								/ [](auto& ctx, auto& view) {
									AstNode* bit = view["bit"]; view["bit"] = nullptr;
									AstNode* bit_template = nullptr;
									AstNode* ext = nullptr;

									OrphanTokens* bit_tok = dynamic_cast<OrphanTokens*>(bit);

									auto f = view.find("bit_template");
									if (f != view.end()) {
										bit_template = f->second;
										view["bit_template"] = nullptr;
									}

									f = view.find("ext");
									if (f != view.end()) {
										ext = f->second;
										view["ext"] = nullptr;
									}

									PathArg pbit;
									pbit.bit = { bit_tok->tokens[0].literal.begin(), bit_tok->tokens[0].literal.end() };
									pbit.args = (bit_template == nullptr) ? nullptr : dynamic_cast<TemplateArgsNode*>(bit_template);

									if (ext != nullptr) {
										PathNode* _path = dynamic_cast<PathNode*>(ext);
										_path->nodes.insert(_path->nodes.begin(), pbit);

										return _path;
									}

									PathNode* path = new PathNode();
									path->nodes.push_back(pbit);

									return path;
								}
		).end();
		
		parser["PATH_SPEC_EXT"] = (begin()
			* lit(".") * rule("PATH_SPEC", "ext") / [](auto& ctx, auto& view) { AstNode* ext = MOVE(view["ext"]); return ext; }
		).end();

		parser["PATH_SPEC"] = (begin()
			* tok(TokenType::Identifier, "bit") * rule("TEMPLATE_PARAMS", "template_bit", true) * rule("PATH_SPEC_EXT", "ext", true)
								/ [](auto& ctx, auto& view) {
									OrphanTokens* bit = dynamic_cast<OrphanTokens*>(view["bit"]);
									TemplateParamsNode* params = nullptr;
									PathSpecNode* path = nullptr;

									auto f = view.find("template_bit");
									if (f != view.end()) {
										params = MOVE_CAST(TemplateParamsNode, view["template_bit"]);
									}

									PathSpecBit pbit = {
										std::string{ bit->tokens[0].literal.begin(), bit->tokens[0].literal.end() },
										params
									};

									f = view.find("ext");
									if (f != view.end()) {
										path = MOVE_CAST(PathSpecNode, view["ext"]);

										path->bits.insert(path->bits.begin(), pbit);
									}
									else {
										path = new PathSpecNode();
										path->bits.push_back(pbit);
									}

									return path;
								}
		).end();

		parser["VAR"] = (begin() * rule("PATH", "varname") / [](ParserContext& ctx, TokenResultView& view) {
			PathNode* tok = dynamic_cast<PathNode*>(view.at("varname")); view["varname"] = nullptr;
			VariableNode* node = new VariableNode(tok);
			return node;
			}
		).end();

		parser["VALUE"] = (begin()
			* rule("INT", "value") / [](auto& ctx, auto& view) { AstNode* v = view["value"]; view["value"] = nullptr; return v; }
			% rule("FLOAT", "value") / [](auto& ctx, auto& view) { AstNode* v = view["value"]; view["value"] = nullptr; return v; }
			% rule("STRING", "value") / [](auto& ctx, auto& view) { AstNode* v = view["value"]; view["value"] = nullptr; return v; }
			% rule("CHAR", "value") / [](auto& ctx, auto& view) { AstNode* v = view["value"]; view["value"] = nullptr; return v; }
			% rule("FunctionCall", "value") / [](auto& ctx, auto& view) { AstNode* v = MOVE(view["value"]); return v; }
			% rule("VAR", "value") / [](auto& ctx, auto& view) { AstNode* v = view["value"]; view["value"] = nullptr; return v; }
			% rule("BOOL", "value") / [](auto& ctx, auto& view) { AstNode* v = view["value"]; view["value"] = nullptr; return v; }
		).end();

		parser["VAR_DECL"] = (begin()
			* rule("PATH", "type") * tok(TokenType::Identifier, "name") * lit("=") * rule("Term", "expr") * lit(";")
								/ [](auto& ctx, auto& view) {
									PathNode* tyname = dynamic_cast<PathNode*>(view["type"]);
									OrphanTokens* vname = dynamic_cast<OrphanTokens*>(view["name"]);
									AstNode* expr = MOVE(view["expr"]);

									std::string full_type_name = tyname->get_full_name(ctx);
									_type_id _id = ctx.types.get_id_from_name(full_type_name);

									if (_id == 0) {
										ctx.errors.push_back("Unkown type: " + full_type_name);
									}

									VariableDeclNode* varNode = new VariableDeclNode(std::string(vname->tokens[0].literal.begin(), vname->tokens[0].literal.end()), _id);
									varNode->default_value = expr;

									return varNode;
								}
			% rule("PATH", "type") * tok(TokenType::Identifier, "name") * lit(";")
								/ [](auto& ctx, auto& view) {
									PathNode* tyname = dynamic_cast<PathNode*>(view["type"]);
									OrphanTokens* vname = dynamic_cast<OrphanTokens*>(view["name"]);

									std::string full_type_name = tyname->get_full_name(ctx);
									_type_id _id = ctx.types.get_id_from_name(full_type_name);

									if (_id == 0) {
										ctx.errors.push_back("Unknown type: " + full_type_name);
									}
									VariableDeclNode* varNode = new VariableDeclNode(std::string(vname->tokens[0].literal.begin(), vname->tokens[0].literal.end()), _id);

									return varNode;	
								}
		).end();

		parser["ARGS"] = (begin()
			* rule("Term", "arg") * lit(",") * rule("ARGS", "ext")
								/ [](auto& ctx, auto& view) {
									AstNode* arg = MOVE(view["arg"]);

									ArgumentsNode* argsE;

									auto f = view.find("ext");
									if (f != view.end()) {
										argsE = dynamic_cast<ArgumentsNode*>(f->second);
										view["ext"] = nullptr;

										argsE->args.insert(argsE->args.begin(), arg);
									}
									else {
										argsE = new ArgumentsNode();

										argsE->args.push_back(arg);
									}


									return argsE;
								}
			% rule("Term", "arg")
								/ [](auto& ctx, auto& view) {
									AstNode* arg = MOVE(view["arg"]);
									ArgumentsNode* argsE = new ArgumentsNode();
									argsE->args.push_back(arg);
									return argsE;
								}

		).end();

		parser["FunctionCall"] = (begin()
			* rule("PATH", "functionName") * lit("(") * rule("ARGS", "args", true) * lit(")") 
								/ [](auto& ctx, auto& view) {
									AstNode* nameRaw = MOVE(view["functionName"]); PathNode* name = dynamic_cast<PathNode*>(nameRaw);
									ArgumentsNode* args = nullptr;

									auto f = view.find("args");
									if (f != view.end()) {
										args = dynamic_cast<ArgumentsNode*>(f->second);
										view["args"] = nullptr;
									}

									FunctionCallNode* call = new FunctionCallNode(name, args);

									return call;
								}
		).end();

		// Factor: ( {Term} )
		//		 | {op} {Factor}  // << maybe this needs to be a term? 
		//		 | {Value}
		parser["Factor"] = (begin()
			* lit("(") * rule("Term", "value") * lit(")")
								/ [](auto& ctx, auto& view) {
									AstNode* value = view["value"]; view["value"] = nullptr;
									return value;
								}
			% tok(TokenType::Operator, "op") * rule("Factor", "value") 
								/ [](auto& ctx, auto& view) {
									OrphanTokens* tok = dynamic_cast<OrphanTokens*>(view.at("op"));
									AstNode* value = view["value"]; view["value"] = nullptr;
									OperatorID opID = get_unary_operator(tok->tokens[0].literal);
									return new UnaryOperator(opID, value);
								}
			% rule("VALUE", "value") 
								/ [](auto& ctx, auto& view) {
									AstNode* value = view["value"]; view["value"] = nullptr;
									return value;
								}
		).end();


		// Term : {Factor} {op} {Term}
		//		| {Factor}
		parser["Term"] = (begin()
			* rule("Factor", "a") * tok(TokenType::Operator, "op") * rule("Term", "b")
								/ [](auto& ctx, auto& view) {
									AstNode* a = view["a"]; view["a"] = nullptr;
									AstNode* b = view["b"]; view["b"] = nullptr;
									OrphanTokens* op = dynamic_cast<OrphanTokens*>(view.at("op"));
									OperatorID opID = get_binary_operator(op->tokens[0].literal);

									BinaryOperator* b_as_op = dynamic_cast<BinaryOperator*>(b);

									BinaryOperator* thisOp = new BinaryOperator(opID, a, b);

									if (b_as_op == nullptr) {
										return thisOp;
									}

									int prec_b = get_operator_prec(b_as_op->m_Operator);
									int prec = get_operator_prec(opID);

									if (prec_b < prec) {
										thisOp->m_Rhs = b_as_op->m_Lhs;
										b_as_op->m_Lhs = thisOp;
										return b_as_op;
									}
									return thisOp;
								}
			% rule("Factor", "value") 
								/ [](auto& ctx, auto& view) {
									AstNode* value = view["value"]; view["value"] = nullptr;
									return value;
								}
		).end();


		parser["Else"] = (begin()
			* lit("else") * rule("If", "if")
								/ [](auto& ctx, auto& view) {
									IfNode* ifNode = dynamic_cast<IfNode*>(view["if"]); view["if"] = nullptr;
									ElseNode* elseNode = new ElseNode();
									elseNode->ifBranch = ifNode;
									elseNode->body = nullptr;
									return elseNode;
								}
			% lit("else") * rule("STATEMENT_BODY", "body")
								/ [](auto& ctx, auto& view) {
									AstNode* body = MOVE(view["body"]);
									ElseNode* elseNode = new ElseNode();
									elseNode->ifBranch = nullptr;
									elseNode->body = dynamic_cast<StatementBlockNode*>(body);
									return elseNode;
								}
		).end();

		parser["If"] = (begin()
			* lit("if") * lit("(") * rule("Term", "expr") * lit(")") * rule("STATEMENT_BODY", "body") * rule("Else", "else", true)
								/ [](auto& ctx, auto& view) {
									AstNode* expr = MOVE(view["expr"]);
									StatementBlockNode* body = dynamic_cast<StatementBlockNode*>(view["body"]); view["body"] = nullptr;
									ElseNode* elseN = nullptr;

									auto f = view.find("else");
									if (f != view.end()) {
										elseN = dynamic_cast<ElseNode*>(f->second);
										view["else"] = nullptr;
									}

									IfNode* ifN = new IfNode();
									ifN->condition = expr;
									ifN->body = body;
									ifN->elseBranch = elseN;

									return ifN;
								}
		).end();

		parser["STATEMENT"] = (begin()
			* rule("If", "if") / [](auto& ctx, auto& view) { AstNode* ifNode = MOVE(view["if"]); return ifNode; }
			% rule("InlineC", "cblock") / [](auto& ctx, auto& view) { AstNode* block = MOVE(view["cblock"]); return block; }
			% rule("RETURN", "ret") / [](auto& ctx, auto& view) { AstNode* block = MOVE(view["ret"]); return block; }
			% rule("VAR_DECL", "var") / [](auto& ctx, auto& view) { AstNode* vNode = MOVE(view["var"]); return vNode; }
			% rule("Term", "expr") * lit(";") / [](auto& ctx, auto& view) { AstNode* expr = MOVE(view["expr"]); return expr; }
		).end();

		parser["STATEMENTS"] = (begin()
			* rule("STATEMENT", "statement") * rule("STATEMENTS", "statements", true)
								/ [](auto& ctx, auto& view) {
									AstNode* statement = MOVE(view["statement"]);
				
									auto f = view.find("statements");
									if (f != view.end()) {
										StatementBlockNode* statements = dynamic_cast<StatementBlockNode*>(f->second);
										view["statements"] = nullptr;

										statements->statements.insert(statements->statements.begin(), statement);
										return statements;
									}

									StatementBlockNode* statements = new StatementBlockNode();
									statements->statements.push_back(statement);

									return statements;
								}
		).end();

		parser["STATEMENT_BODY"] = (begin()
			* lit("{") * rule("STATEMENTS", "statements", true) * lit("}") / [](auto& ctx, auto& view) { AstNode* body = MOVE(view["statements"]); if (body == nullptr) { body = new StatementBlockNode(); } return body; }
		).end();

		parser["STRUCT_MEMBERS"] = (begin()
			* lit("pub", true, "pub") * rule("PATH", "type") * tok(TokenType::Identifier, "name") * lit("=") * rule("Term", "expr") * lit(";") * rule("STRUCT_MEMBERS", "next_members", true)
								/ [](ParserContext& ctx, TokenResultView& view) {
									AstNode* type = MOVE(view["type"]); PathNode* ttype = dynamic_cast<PathNode*>(type);
									AstNode* expr = MOVE(view["expr"]);
									OrphanTokens* nameToks = dynamic_cast<OrphanTokens*>(view.at("name"));
									Visibility visi = Visibility::Private;
									if (ctx.flags.find("pub") != ctx.flags.end()) {
										visi = Visibility::Public;
									}

									std::string varname = std::string{
										nameToks->tokens[0].literal.begin(), nameToks->tokens[0].literal.end(),
									};
									std::string _typename = ttype->get_full_name(ctx);
									_type_id type_id = ctx.types.get_id_from_name(_typename.c_str());

									VariableDeclNode* var = new VariableDeclNode(varname, type_id);
									var->visibility = visi;
									var->default_value = expr;

									auto f = view.find("next_members");
									StructMembersNode* struct_members;
									if (f != view.end()) {
										struct_members = dynamic_cast<StructMembersNode*>(f->second);
										view["next_members"] = nullptr;
									}
									else {
										struct_members = new StructMembersNode();
									}
									struct_members->members.insert(struct_members->members.begin(), var);

									return struct_members;
								}
			% lit("pub", true, "pub") * rule("PATH", "type") * tok(TokenType::Identifier, "name") * lit(";") * rule("STRUCT_MEMBERS", "next_members", true)
								/ [](ParserContext& ctx, TokenResultView& view) {
								AstNode* t = MOVE(view["type"]); PathNode* type = dynamic_cast<PathNode*>(t);
								OrphanTokens* nameToks = dynamic_cast<OrphanTokens*>(view.at("name"));

								Visibility visi = Visibility::Private;
								if (ctx.flags.find("pub") != ctx.flags.end()) {
									visi = Visibility::Public;
								}

								std::string varname = std::string{ nameToks->tokens[0].literal.begin(), nameToks->tokens[0].literal.end() };
								std::string _typename = type->get_full_name(ctx);
								_type_id type_id = ctx.types.get_id_from_name(_typename.c_str());

								VariableDeclNode* var = new VariableDeclNode(varname, type_id);
								var->visibility = visi;

								auto f = view.find("next_members");
								StructMembersNode* struct_members;
								if (f != view.end()) {
									struct_members = dynamic_cast<StructMembersNode*>(f->second);
									view["next_members"] = nullptr;

									struct_members->members.insert(struct_members->members.begin(), var);
								}
								else {
									struct_members = new StructMembersNode();

									struct_members->members.push_back(var);
								}


								return struct_members;
								}
		).end();

		parser["STRUCT_DEF"] = (begin()
			* lit("pub", true, "pub") * lit("struct") * tok(TokenType::Identifier, "name") * lit("{") * rule("STRUCT_MEMBERS", "members") * lit("}")
								/ [](auto& ctx, auto& view) {
									AstNode* name = view["name"];
									AstNode* members = MOVE(view["members"]);

									Visibility visibility = Visibility::Private;
									if (ctx.flags.find("pub") != ctx.flags.end()) {
										visibility = Visibility::Public;
									}

									OrphanTokens* token = dynamic_cast<OrphanTokens*>(name);

									StructDefNode* _struct = nullptr;
									try {
										_struct = new StructDefNode(
											std::string{ token->tokens[0].literal.begin(), token->tokens[0].literal.end() },
											dynamic_cast<StructMembersNode*>(members),
											ctx.types
										);

										_struct->visibility = visibility;
									}
									catch (const std::string& err) {
										std::cout << err << "\n\tAt struct definition in " << token->tokens[0].source_file << " on line " << token->tokens[0].row << ", " << token->tokens[0].col << "\n";
									}

									return _struct;
								}
		).end();

		parser["INCLUDE"] = (begin()
			* lit("include") * lit("_C") * tok(TokenType::String, "include")
								/ [](auto& ctx, auto& view) {
									OrphanTokens* toks = dynamic_cast<OrphanTokens*>(view["include"]);

									IncludeNode* include = new IncludeNode();
									include->is_c_include = true;
									include->c_include = std::string(
										toks->tokens[0].literal.begin(),
										toks->tokens[0].literal.end()
									);

									return include;
								}
		).end();

		parser["ParamsExt"] = (begin()
			* lit(",") * rule("Params", "params")
								/ [](auto& ctx, auto& view) {
									AstNode* params = MOVE(view["params"]);
									return params;
								}
		).end();

		parser["Params"] = (begin()
			* rule("PATH", "type") * tok(TokenType::Identifier, "name") * rule("ParamsExt", "params", true)
								/ [](auto& ctx, auto& view) {
									PathNode* type = dynamic_cast<PathNode*>(view["type"]);
									OrphanTokens* name = dynamic_cast<OrphanTokens*>(view["name"]);

									std::string _typename = type->get_full_name(ctx);
									_type_id type_id = ctx.types.get_id_from_name(_typename.c_str());

									if (type_id == 0) {
										ctx.errors.push_back("Unknown type: " + _typename);
									}

									std::string varname = std::string(name->tokens[0].literal.begin(), name->tokens[0].literal.end());

									Param p = { type_id, varname };

									auto f = view.find("params");
									if (f != view.end()) {
										ParameterListNode* params = dynamic_cast<ParameterListNode*>(f->second);
										view["params"] = nullptr;

										params->params.insert(params->params.begin(), p);

										return params;
									}
				
									ParameterListNode* par = new ParameterListNode();
									par->params.push_back(p);

									return par;
								}
		).end();

		parser["InlineC"] = (begin()
			* lit("inline") * lit("_C") * grab_nested("{", "}", "tokens")
								/ [](auto& ctx, auto& view) {
									OrphanTokens* toks = dynamic_cast<OrphanTokens*>(view["tokens"]);

									InlineCBlock* block = new InlineCBlock();
									for (auto& tok : toks->tokens) {
										block->tokens.push_back(tok);
									}

									return block;
								}
			
		).end();

		parser["Module"] = (begin()
			* lit("mod") * rule("PATH_SPEC", "moduleName") * lit(";") * rule("ModuleLevelDeclarations", "content")
								/[](auto& ctx, auto& view) {
									PathSpecNode* name = dynamic_cast<PathSpecNode*>(view["moduleName"]); view["moduleName"] = nullptr;
									ModuleBodyNode* body = dynamic_cast<ModuleBodyNode*>(view["content"]); view["content"] = nullptr;

									if (body == nullptr) {
										ctx.errors.push_back("A module cannot be empty");
									}
									ModuleNode* moduleN = new ModuleNode(name);
									moduleN->body = body;

									return moduleN;
								}
		).end();

		parser["ModuleLevelDeclarations"] = (begin()
			* rule("FuncDef", "function") * rule("ModuleLevelDeclarations", "body", true)
								/ [](auto& ctx, auto& view) {
									FunctionDefinitionNode* func = MOVE_CAST(FunctionDefinitionNode, view["function"]);
									ModuleBodyNode* body = nullptr;
				
									auto f = view.find("body");
									if (f != view.end()) {
										body = dynamic_cast<ModuleBodyNode*>(f->second);
										view["body"] = nullptr;
				
										body->functions.push_back(func);
										return body;
									}

									body = new ModuleBodyNode();
									body->functions.push_back(func);
									return body;
								}
			% rule("STRUCT_DEF", "struct") * rule("ModuleLevelDeclarations", "body", true)
								/ [](auto& ctx, auto& view) {
									StructDefNode* struc = MOVE_CAST(StructDefNode, view["struct"]);
									ModuleBodyNode* body = nullptr;

									auto f = view.find("body");
									if (f != view.end()) {
										body = MOVE_CAST(ModuleBodyNode, view["body"]);

										body->structs.push_back(struc);
										return body;
									}

									body = new ModuleBodyNode();
									body->structs.push_back(struc);
									return body;
								}
			% rule("INCLUDE", "include") * rule("ModuleLevelDeclarations", "body", true)
								/ [](auto& ctx, auto& view) {
									IncludeNode* inc = MOVE_CAST(IncludeNode, view["include"]);
									ModuleBodyNode* body = nullptr;

									auto f = view.find("body");
									if (f != view.end()) {
										body = MOVE_CAST(ModuleBodyNode, view["body"]);

										body->includes.push_back(inc);

										return body;
									}

									body = new ModuleBodyNode();
									body->includes.push_back(inc);
									return body;
								}
		).end();

		parser["FuncDef"] = (begin()
			* lit("pub", true, "pub") * lit("inline", true, "inline") * lit("fn") * tok(TokenType::Identifier, "name") * rule("TEMPLATE_PARAMS", "template", true)
				* lit("(") * rule("Params", "params", true) * lit(")") * rule("PATH", "returnty", true) * rule("STATEMENT_BODY", "body")
			/ [](auto& ctx, auto& view) {
				Visibility visibility = ctx.flags.find("pub") != ctx.flags.end() ? Visibility::Public : Visibility::Private;
				bool is_inline = ctx.flags.find("inline") != ctx.flags.end();

				OrphanTokens* nameTok = dynamic_cast<OrphanTokens*>(view["name"]);
				TemplateParamsNode* templ = nullptr;
				ParameterListNode* params = nullptr;
				PathNode* returnType = nullptr;

				StatementBlockNode* body = dynamic_cast<StatementBlockNode*>(view["body"]);
				view["body"] = nullptr;
					
				auto f = view.find("template");
				if (f != view.end()) {
					templ = dynamic_cast<TemplateParamsNode*>(f->second);
					view["template"] = nullptr;
				}

				f = view.find("params");
				if (f != view.end()) {
					params = dynamic_cast<ParameterListNode*>(f->second);
					view["params"] = nullptr;
				}

				f = view.find("returnty");
				if (f != view.end()) {
					returnType = dynamic_cast<PathNode*>(f->second);
					view["returnty"] = nullptr;
				}
				
				std::string _ret_ty = "void";
				if (returnType != nullptr) {
					_ret_ty = returnType->get_full_name(ctx);
				}

				_type_id _ty = ctx.types.get_id_from_name(_ret_ty);

				if (_ty == 0) {
					ctx.errors.push_back("Unknown type: " + _ret_ty);
				}

				FunctionDefinitionNode* funcDef = new FunctionDefinitionNode();
				funcDef->functionName = std::string(nameTok->tokens[0].literal.begin(), nameTok->tokens[0].literal.end());
				funcDef->params = params;
				funcDef->templateParams = templ;
				funcDef->returnType = _ty;
				funcDef->body = body;
				funcDef->visibility = visibility;

				return funcDef;
			}
		).end();

		parser["RETURN"] = (begin()
			* lit("return") * rule("Term", "expr", true) * lit(";")
			/[](auto& ctx, auto& view){ 
				ReturnNode* ret = new ReturnNode();

				if (view.find("expr") != view.end()) {
					ret->returnValue = MOVE(view["expr"]);
				}

				return ret;
			}
		).end();
	}



	std::vector<AllowedBinaryOperator>& GetAllowedOperators() {
		static std::vector<AllowedBinaryOperator> operators;

		if (operators.empty()) {
			TypeRegistry& registry = TypeRegistry::instance();
			
			_type_id _u8 = registry.get_id_from_name("u8");
			_type_id _u16 = registry.get_id_from_name("u16");
			_type_id _u32 = registry.get_id_from_name("u32");
			_type_id _u64 = registry.get_id_from_name("u64");
			_type_id _i8 = registry.get_id_from_name("i8");
			_type_id _i16 = registry.get_id_from_name("i16");
			_type_id _i32 = registry.get_id_from_name("i32");
			_type_id _i64 = registry.get_id_from_name("i64");
			_type_id _f32 = registry.get_id_from_name("f32");
			_type_id _f64 = registry.get_id_from_name("f64");
			_type_id _char = registry.get_id_from_name("char");
			_type_id _bool = registry.get_id_from_name("bool");


			operators.push_back({ OperatorID::Add,  _u8,  _u8,  _u8, nullptr });
			operators.push_back({ OperatorID::Add, _u16, _u16, _u16, nullptr });
			operators.push_back({ OperatorID::Add, _u32, _u32, _u32, nullptr });
			operators.push_back({ OperatorID::Add, _u64, _u64, _u64, nullptr });
			operators.push_back({ OperatorID::Add,  _i8,  _i8,  _i8, nullptr });
			operators.push_back({ OperatorID::Add, _i16, _i16, _i16, nullptr });
			operators.push_back({ OperatorID::Add, _i32, _i32, _i32, nullptr });
			operators.push_back({ OperatorID::Add, _i64, _i64, _i64, nullptr });
			operators.push_back({ OperatorID::Add, _f32, _f32, _f32, nullptr });
			operators.push_back({ OperatorID::Add, _f64, _f64, _f64, nullptr });

			operators.push_back({ OperatorID::Sub,  _u8,  _u8,  _u8, nullptr });
			operators.push_back({ OperatorID::Sub, _u16, _u16, _u16, nullptr });
			operators.push_back({ OperatorID::Sub, _u32, _u32, _u32, nullptr });
			operators.push_back({ OperatorID::Sub, _u64, _u64, _u64, nullptr });
			operators.push_back({ OperatorID::Sub,  _i8,  _i8,  _i8, nullptr });
			operators.push_back({ OperatorID::Sub, _i16, _i16, _i16, nullptr });
			operators.push_back({ OperatorID::Sub, _i32, _i32, _i32, nullptr });
			operators.push_back({ OperatorID::Sub, _i64, _i64, _i64, nullptr });
			operators.push_back({ OperatorID::Sub, _f32, _f32, _f32, nullptr });
			operators.push_back({ OperatorID::Sub, _f64, _f64, _f64, nullptr });

			operators.push_back({ OperatorID::Mul,  _u8,  _u8,  _u8, nullptr });
			operators.push_back({ OperatorID::Mul, _u16, _u16, _u16, nullptr });
			operators.push_back({ OperatorID::Mul, _u32, _u32, _u32, nullptr });
			operators.push_back({ OperatorID::Mul, _u64, _u64, _u64, nullptr });
			operators.push_back({ OperatorID::Mul,  _i8,  _i8,  _i8, nullptr });
			operators.push_back({ OperatorID::Mul, _i16, _i16, _i16, nullptr });
			operators.push_back({ OperatorID::Mul, _i32, _i32, _i32, nullptr });
			operators.push_back({ OperatorID::Mul, _i64, _i64, _i64, nullptr });
			operators.push_back({ OperatorID::Mul, _f32, _f32, _f32, nullptr });
			operators.push_back({ OperatorID::Mul, _f64, _f64, _f64, nullptr });

			operators.push_back({ OperatorID::Div,  _u8,  _u8,  _u8, nullptr });
			operators.push_back({ OperatorID::Div, _u16, _u16, _u16, nullptr });
			operators.push_back({ OperatorID::Div, _u32, _u32, _u32, nullptr });
			operators.push_back({ OperatorID::Div, _u64, _u64, _u64, nullptr });
			operators.push_back({ OperatorID::Div,  _i8,  _i8,  _i8, nullptr });
			operators.push_back({ OperatorID::Div, _i16, _i16, _i16, nullptr });
			operators.push_back({ OperatorID::Div, _i32, _i32, _i32, nullptr });
			operators.push_back({ OperatorID::Div, _i64, _i64, _i64, nullptr });
			operators.push_back({ OperatorID::Div, _f32, _f32, _f32, nullptr });
			operators.push_back({ OperatorID::Div, _f64, _f64, _f64, nullptr });

			operators.push_back({ OperatorID::Mod,  _u8,  _u8,  _u8, nullptr });
			operators.push_back({ OperatorID::Mod, _u16, _u16, _u16, nullptr });
			operators.push_back({ OperatorID::Mod, _u32, _u32, _u32, nullptr });
			operators.push_back({ OperatorID::Mod, _u64, _u64, _u64, nullptr });
			operators.push_back({ OperatorID::Mod,  _i8,  _i8,  _i8, nullptr });
			operators.push_back({ OperatorID::Mod, _i16, _i16, _i16, nullptr });
			operators.push_back({ OperatorID::Mod, _i32, _i32, _i32, nullptr });
			operators.push_back({ OperatorID::Mod, _i64, _i64, _i64, nullptr });
			operators.push_back({ OperatorID::Mod, _f32, _f32, _f32, nullptr });
			operators.push_back({ OperatorID::Mod, _f64, _f64, _f64, nullptr });

			operators.push_back({ OperatorID::Assign,  _u8,  _u8,  _u8, nullptr });
			operators.push_back({ OperatorID::Assign, _u16, _u16, _u16, nullptr });
			operators.push_back({ OperatorID::Assign, _u32, _u32, _u32, nullptr });
			operators.push_back({ OperatorID::Assign, _u64, _u64, _u64, nullptr });
			operators.push_back({ OperatorID::Assign,  _i8,  _i8,  _i8, nullptr });
			operators.push_back({ OperatorID::Assign, _i16, _i16, _i16, nullptr });
			operators.push_back({ OperatorID::Assign, _i32, _i32, _i32, nullptr });
			operators.push_back({ OperatorID::Assign, _i64, _i64, _i64, nullptr });
			operators.push_back({ OperatorID::Assign, _f32, _f32, _f32, nullptr });
			operators.push_back({ OperatorID::Assign, _f64, _f64, _f64, nullptr });

			operators.push_back({ OperatorID::Equals,  _u8,  _u8, _bool, nullptr });
			operators.push_back({ OperatorID::Equals, _u16, _u16, _bool, nullptr });
			operators.push_back({ OperatorID::Equals, _u32, _u32, _bool, nullptr });
			operators.push_back({ OperatorID::Equals, _u64, _u64, _bool, nullptr });
			operators.push_back({ OperatorID::Equals,  _i8,  _i8, _bool, nullptr });
			operators.push_back({ OperatorID::Equals, _i16, _i16, _bool, nullptr });
			operators.push_back({ OperatorID::Equals, _i32, _i32, _bool, nullptr });
			operators.push_back({ OperatorID::Equals, _i64, _i64, _bool, nullptr });
			operators.push_back({ OperatorID::Equals, _f32, _f32, _bool, nullptr });
			operators.push_back({ OperatorID::Equals, _f64, _f64, _bool, nullptr });

			operators.push_back({ OperatorID::NotEquals,  _u8,  _u8, _bool, nullptr });
			operators.push_back({ OperatorID::NotEquals, _u16, _u16, _bool, nullptr });
			operators.push_back({ OperatorID::NotEquals, _u32, _u32, _bool, nullptr });
			operators.push_back({ OperatorID::NotEquals, _u64, _u64, _bool, nullptr });
			operators.push_back({ OperatorID::NotEquals,  _i8,  _i8, _bool, nullptr });
			operators.push_back({ OperatorID::NotEquals, _i16, _i16, _bool, nullptr });
			operators.push_back({ OperatorID::NotEquals, _i32, _i32, _bool, nullptr });
			operators.push_back({ OperatorID::NotEquals, _i64, _i64, _bool, nullptr });
			operators.push_back({ OperatorID::NotEquals, _f32, _f32, _bool, nullptr });
			operators.push_back({ OperatorID::NotEquals, _f64, _f64, _bool, nullptr });

			operators.push_back({ OperatorID::LessThan,  _u8,  _u8, _bool, nullptr });
			operators.push_back({ OperatorID::LessThan, _u16, _u16, _bool, nullptr });
			operators.push_back({ OperatorID::LessThan, _u32, _u32, _bool, nullptr });
			operators.push_back({ OperatorID::LessThan, _u64, _u64, _bool, nullptr });
			operators.push_back({ OperatorID::LessThan,  _i8,  _i8, _bool, nullptr });
			operators.push_back({ OperatorID::LessThan, _i16, _i16, _bool, nullptr });
			operators.push_back({ OperatorID::LessThan, _i32, _i32, _bool, nullptr });
			operators.push_back({ OperatorID::LessThan, _i64, _i64, _bool, nullptr });
			operators.push_back({ OperatorID::LessThan, _f32, _f32, _bool, nullptr });
			operators.push_back({ OperatorID::LessThan, _f64, _f64, _bool, nullptr });

			operators.push_back({ OperatorID::GreaterThan,  _u8,  _u8, _bool, nullptr });
			operators.push_back({ OperatorID::GreaterThan, _u16, _u16, _bool, nullptr });
			operators.push_back({ OperatorID::GreaterThan, _u32, _u32, _bool, nullptr });
			operators.push_back({ OperatorID::GreaterThan, _u64, _u64, _bool, nullptr });
			operators.push_back({ OperatorID::GreaterThan,  _i8,  _i8, _bool, nullptr });
			operators.push_back({ OperatorID::GreaterThan, _i16, _i16, _bool, nullptr });
			operators.push_back({ OperatorID::GreaterThan, _i32, _i32, _bool, nullptr });
			operators.push_back({ OperatorID::GreaterThan, _i64, _i64, _bool, nullptr });
			operators.push_back({ OperatorID::GreaterThan, _f32, _f32, _bool, nullptr });
			operators.push_back({ OperatorID::GreaterThan, _f64, _f64, _bool, nullptr });

			operators.push_back({ OperatorID::LessEquals,  _u8,  _u8, _bool, nullptr });
			operators.push_back({ OperatorID::LessEquals, _u16, _u16, _bool, nullptr });
			operators.push_back({ OperatorID::LessEquals, _u32, _u32, _bool, nullptr });
			operators.push_back({ OperatorID::LessEquals, _u64, _u64, _bool, nullptr });
			operators.push_back({ OperatorID::LessEquals,  _i8,  _i8, _bool, nullptr });
			operators.push_back({ OperatorID::LessEquals, _i16, _i16, _bool, nullptr });
			operators.push_back({ OperatorID::LessEquals, _i32, _i32, _bool, nullptr });
			operators.push_back({ OperatorID::LessEquals, _i64, _i64, _bool, nullptr });
			operators.push_back({ OperatorID::LessEquals, _f32, _f32, _bool, nullptr });
			operators.push_back({ OperatorID::LessEquals, _f64, _f64, _bool, nullptr });

			operators.push_back({ OperatorID::GreaterEquals,  _u8,  _u8, _bool, nullptr });
			operators.push_back({ OperatorID::GreaterEquals, _u16, _u16, _bool, nullptr });
			operators.push_back({ OperatorID::GreaterEquals, _u32, _u32, _bool, nullptr });
			operators.push_back({ OperatorID::GreaterEquals, _u64, _u64, _bool, nullptr });
			operators.push_back({ OperatorID::GreaterEquals,  _i8,  _i8, _bool, nullptr });
			operators.push_back({ OperatorID::GreaterEquals, _i16, _i16, _bool, nullptr });
			operators.push_back({ OperatorID::GreaterEquals, _i32, _i32, _bool, nullptr });
			operators.push_back({ OperatorID::GreaterEquals, _i64, _i64, _bool, nullptr });
			operators.push_back({ OperatorID::GreaterEquals, _f32, _f32, _bool, nullptr });
			operators.push_back({ OperatorID::GreaterEquals, _f64, _f64, _bool, nullptr });
		}

		return operators;
	}

	std::vector<AllowedUnaryOperator>& GetAllowedUnaryOperators() {
		static std::vector<AllowedUnaryOperator> operators;

		if (operators.empty()) {
			TypeRegistry& registry = TypeRegistry::instance();

			_type_id _u8 = registry.get_id_from_name("u8");
			_type_id _u16 = registry.get_id_from_name("u16");
			_type_id _u32 = registry.get_id_from_name("u32");
			_type_id _u64 = registry.get_id_from_name("u64");
			_type_id _i8 = registry.get_id_from_name("i8");
			_type_id _i16 = registry.get_id_from_name("i16");
			_type_id _i32 = registry.get_id_from_name("i32");
			_type_id _i64 = registry.get_id_from_name("i64");
			_type_id _f32 = registry.get_id_from_name("f32");
			_type_id _f64 = registry.get_id_from_name("f64");
			_type_id _char = registry.get_id_from_name("char");
			_type_id _bool = registry.get_id_from_name("bool");

			operators.push_back({ OperatorID::Negative, _i8, _i8 });
			operators.push_back({ OperatorID::Negative, _i16, _i16 });
			operators.push_back({ OperatorID::Negative, _i32, _i32 });
			operators.push_back({ OperatorID::Negative, _i64, _i64 });
			operators.push_back({ OperatorID::Negative, _u8, _i8 });
			operators.push_back({ OperatorID::Negative, _u16, _i16 });
			operators.push_back({ OperatorID::Negative, _u32, _i32 });
			operators.push_back({ OperatorID::Negative, _u64, _i64 });
			operators.push_back({ OperatorID::Negative, _f32, _f32 });
			operators.push_back({ OperatorID::Negative, _f64, _f64 });

			operators.push_back({ OperatorID::Not, _bool, _bool });
			operators.push_back({ OperatorID::Not, _i8, _bool });
			operators.push_back({ OperatorID::Not, _i16, _bool });
			operators.push_back({ OperatorID::Not, _i32, _bool });
			operators.push_back({ OperatorID::Not, _i64, _bool });
			operators.push_back({ OperatorID::Not, _u8, _bool });
			operators.push_back({ OperatorID::Not, _u16, _bool });
			operators.push_back({ OperatorID::Not, _u32, _bool });
			operators.push_back({ OperatorID::Not, _u64, _bool });
			operators.push_back({ OperatorID::Not, _f32, _bool });
			operators.push_back({ OperatorID::Not, _f64, _bool });

			operators.push_back({ OperatorID::BinaryNot, _bool, _bool });
			operators.push_back({ OperatorID::BinaryNot, _i8,  _i8 });
			operators.push_back({ OperatorID::BinaryNot, _i16, _i16 });
			operators.push_back({ OperatorID::BinaryNot, _i32, _i32 });
			operators.push_back({ OperatorID::BinaryNot, _i64, _i64 });
			operators.push_back({ OperatorID::BinaryNot, _u8,  _u8 });
			operators.push_back({ OperatorID::BinaryNot, _u16, _u16 });
			operators.push_back({ OperatorID::BinaryNot, _u32, _u32 });
			operators.push_back({ OperatorID::BinaryNot, _u64, _u64 });

			operators.push_back({ OperatorID::PreInc, _i8, _i8 });
			operators.push_back({ OperatorID::PreInc, _i16, _i16 });
			operators.push_back({ OperatorID::PreInc, _i32, _i32 });
			operators.push_back({ OperatorID::PreInc, _i64, _i64 });
			operators.push_back({ OperatorID::PreInc, _u8, _i8 });
			operators.push_back({ OperatorID::PreInc, _u16, _i16 });
			operators.push_back({ OperatorID::PreInc, _u32, _i32 });
			operators.push_back({ OperatorID::PreInc, _u64, _i64 });

			operators.push_back({ OperatorID::PostInc, _i8, _i8 });
			operators.push_back({ OperatorID::PostInc, _i16, _i16 });
			operators.push_back({ OperatorID::PostInc, _i32, _i32 });
			operators.push_back({ OperatorID::PostInc, _i64, _i64 });
			operators.push_back({ OperatorID::PostInc, _u8, _i8 });
			operators.push_back({ OperatorID::PostInc, _u16, _i16 });
			operators.push_back({ OperatorID::PostInc, _u32, _i32 });
			operators.push_back({ OperatorID::PostInc, _u64, _i64 });

			operators.push_back({ OperatorID::PreDec, _i8, _i8 });
			operators.push_back({ OperatorID::PreDec, _i16, _i16 });
			operators.push_back({ OperatorID::PreDec, _i32, _i32 });
			operators.push_back({ OperatorID::PreDec, _i64, _i64 });
			operators.push_back({ OperatorID::PreDec, _u8, _i8 });
			operators.push_back({ OperatorID::PreDec, _u16, _i16 });
			operators.push_back({ OperatorID::PreDec, _u32, _i32 });
			operators.push_back({ OperatorID::PreDec, _u64, _i64 });

			operators.push_back({ OperatorID::PostDec, _i8, _i8 });
			operators.push_back({ OperatorID::PostDec, _i16, _i16 });
			operators.push_back({ OperatorID::PostDec, _i32, _i32 });
			operators.push_back({ OperatorID::PostDec, _i64, _i64 });
			operators.push_back({ OperatorID::PostDec, _u8, _i8 });
			operators.push_back({ OperatorID::PostDec, _u16, _i16 });
			operators.push_back({ OperatorID::PostDec, _u32, _i32 });
			operators.push_back({ OperatorID::PostDec, _u64, _i64 });
		
			operators.push_back({ OperatorID::Cast, _i8, _i16 });
			operators.push_back({ OperatorID::Cast, _i8, _i32 });
			operators.push_back({ OperatorID::Cast, _i8, _i64 });
			operators.push_back({ OperatorID::Cast, _i8, _u8 });
			operators.push_back({ OperatorID::Cast, _i8, _u16 });
			operators.push_back({ OperatorID::Cast, _i8, _u32 });
			operators.push_back({ OperatorID::Cast, _i8, _u64 });
			operators.push_back({ OperatorID::Cast, _i8, _f32 });
			operators.push_back({ OperatorID::Cast, _i8, _f64 });
			operators.push_back({ OperatorID::Cast, _i8, _bool });
			operators.push_back({ OperatorID::Cast, _i8, _char });

			operators.push_back({ OperatorID::Cast, _i16, _i8 });
			operators.push_back({ OperatorID::Cast, _i16, _i32 });
			operators.push_back({ OperatorID::Cast, _i16, _i64 });
			operators.push_back({ OperatorID::Cast, _i16, _u8 });
			operators.push_back({ OperatorID::Cast, _i16, _u16 });
			operators.push_back({ OperatorID::Cast, _i16, _u32 });
			operators.push_back({ OperatorID::Cast, _i16, _u64 });
			operators.push_back({ OperatorID::Cast, _i16, _f32 });
			operators.push_back({ OperatorID::Cast, _i16, _f64 });
			operators.push_back({ OperatorID::Cast, _i16, _bool });
			operators.push_back({ OperatorID::Cast, _i16, _char });

			operators.push_back({ OperatorID::Cast, _i32, _i8 });
			operators.push_back({ OperatorID::Cast, _i32, _i16 });
			operators.push_back({ OperatorID::Cast, _i32, _i64 });
			operators.push_back({ OperatorID::Cast, _i32, _u8 });
			operators.push_back({ OperatorID::Cast, _i32, _u16 });
			operators.push_back({ OperatorID::Cast, _i32, _u32 });
			operators.push_back({ OperatorID::Cast, _i32, _u64 });
			operators.push_back({ OperatorID::Cast, _i32, _f32 });
			operators.push_back({ OperatorID::Cast, _i32, _f64 });
			operators.push_back({ OperatorID::Cast, _i32, _bool });
			operators.push_back({ OperatorID::Cast, _i32, _char });

			operators.push_back({ OperatorID::Cast, _i64, _i8 });
			operators.push_back({ OperatorID::Cast, _i64, _i16 });
			operators.push_back({ OperatorID::Cast, _i64, _i32 });
			operators.push_back({ OperatorID::Cast, _i64, _u8 });
			operators.push_back({ OperatorID::Cast, _i64, _u16 });
			operators.push_back({ OperatorID::Cast, _i64, _u32 });
			operators.push_back({ OperatorID::Cast, _i64, _u64 });
			operators.push_back({ OperatorID::Cast, _i64, _f32 });
			operators.push_back({ OperatorID::Cast, _i64, _f64 });
			operators.push_back({ OperatorID::Cast, _i64, _bool });
			operators.push_back({ OperatorID::Cast, _i64, _char });

			operators.push_back({ OperatorID::Cast, _u8, _i8 });
			operators.push_back({ OperatorID::Cast, _u8, _i16 });
			operators.push_back({ OperatorID::Cast, _u8, _i32 });
			operators.push_back({ OperatorID::Cast, _u8, _i64 });
			operators.push_back({ OperatorID::Cast, _u8, _u16 });
			operators.push_back({ OperatorID::Cast, _u8, _u32 });
			operators.push_back({ OperatorID::Cast, _u8, _u64 });
			operators.push_back({ OperatorID::Cast, _u8, _f32 });
			operators.push_back({ OperatorID::Cast, _u8, _f64 });
			operators.push_back({ OperatorID::Cast, _u8, _bool });
			operators.push_back({ OperatorID::Cast, _u8, _char });

			operators.push_back({ OperatorID::Cast, _u16, _i8 });
			operators.push_back({ OperatorID::Cast, _u16, _i16 });
			operators.push_back({ OperatorID::Cast, _u16, _i32 });
			operators.push_back({ OperatorID::Cast, _u16, _i64 });
			operators.push_back({ OperatorID::Cast, _u16, _u8 });
			operators.push_back({ OperatorID::Cast, _u16, _u32 });
			operators.push_back({ OperatorID::Cast, _u16, _u64 });
			operators.push_back({ OperatorID::Cast, _u16, _f32 });
			operators.push_back({ OperatorID::Cast, _u16, _f64 });
			operators.push_back({ OperatorID::Cast, _u16, _bool });
			operators.push_back({ OperatorID::Cast, _u16, _char });

			operators.push_back({ OperatorID::Cast, _u32, _i8 });
			operators.push_back({ OperatorID::Cast, _u32, _i16 });
			operators.push_back({ OperatorID::Cast, _u32, _i32 });
			operators.push_back({ OperatorID::Cast, _u32, _i64 });
			operators.push_back({ OperatorID::Cast, _u32, _u8 });
			operators.push_back({ OperatorID::Cast, _u32, _u16 });
			operators.push_back({ OperatorID::Cast, _u32, _u64 });
			operators.push_back({ OperatorID::Cast, _u32, _f32 });
			operators.push_back({ OperatorID::Cast, _u32, _f64 });
			operators.push_back({ OperatorID::Cast, _u32, _bool });
			operators.push_back({ OperatorID::Cast, _u32, _char });

			operators.push_back({ OperatorID::Cast, _u64, _i8 });
			operators.push_back({ OperatorID::Cast, _u64, _i16 });
			operators.push_back({ OperatorID::Cast, _u64, _i32 });
			operators.push_back({ OperatorID::Cast, _u64, _i64 });
			operators.push_back({ OperatorID::Cast, _u64, _u8 });
			operators.push_back({ OperatorID::Cast, _u64, _u16 });
			operators.push_back({ OperatorID::Cast, _u64, _u32 });
			operators.push_back({ OperatorID::Cast, _u64, _f32 });
			operators.push_back({ OperatorID::Cast, _u64, _f64 });
			operators.push_back({ OperatorID::Cast, _u64, _bool });
			operators.push_back({ OperatorID::Cast, _u64, _char });

			operators.push_back({ OperatorID::Cast, _f32, _i8 });
			operators.push_back({ OperatorID::Cast, _f32, _i16 });
			operators.push_back({ OperatorID::Cast, _f32, _i32 });
			operators.push_back({ OperatorID::Cast, _f32, _i64 });
			operators.push_back({ OperatorID::Cast, _f32, _u8 });
			operators.push_back({ OperatorID::Cast, _f32, _u16 });
			operators.push_back({ OperatorID::Cast, _f32, _u32 });
			operators.push_back({ OperatorID::Cast, _f32, _u64 });
			operators.push_back({ OperatorID::Cast, _f32, _f64 });

			operators.push_back({ OperatorID::Cast, _f64, _i8 });
			operators.push_back({ OperatorID::Cast, _f64, _i16 });
			operators.push_back({ OperatorID::Cast, _f64, _i32 });
			operators.push_back({ OperatorID::Cast, _f64, _i64 });
			operators.push_back({ OperatorID::Cast, _f64, _u8 });
			operators.push_back({ OperatorID::Cast, _f64, _u16 });
			operators.push_back({ OperatorID::Cast, _f64, _u32 });
			operators.push_back({ OperatorID::Cast, _f64, _u64 });
			operators.push_back({ OperatorID::Cast, _f64, _f32 });

			operators.push_back({ OperatorID::Cast, _bool, _i8 });
			operators.push_back({ OperatorID::Cast, _bool, _i16 });
			operators.push_back({ OperatorID::Cast, _bool, _i32 });
			operators.push_back({ OperatorID::Cast, _bool, _i64 });
			operators.push_back({ OperatorID::Cast, _bool, _u8 });
			operators.push_back({ OperatorID::Cast, _bool, _u16 });
			operators.push_back({ OperatorID::Cast, _bool, _u32 });
			operators.push_back({ OperatorID::Cast, _bool, _u64 });
			operators.push_back({ OperatorID::Cast, _bool, _f32 });
			operators.push_back({ OperatorID::Cast, _bool, _f64 });

			// TODO: Ref and Deref
			// TODO: Add Cast to string type

		}

		return operators;
	}
}