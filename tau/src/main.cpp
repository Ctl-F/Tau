#include "tau.h"


#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>

/*
Compiled C Format:

// Framework Includes...

// Module Includes

// private enum declarations...

// private struct pre-declarations

// private struct declarations




*/



/*
#include "std_io.h"
#include "std_collections.h"
#include "std_math.h"

enum app_result_i64_string_e {
	app_result_i64_string_e_Ok,
	app_result_i64_string_e_Err
};
struct app_result_i64_string_s {
	app_result_i64_string_e type;
	union {
		i64 value_Ok;
		struct tau_string value_Err;
	};
};

static i64 app_lambda_0(i64 a, i64 b) {
	return std_math_max_i64(a, b);
}
static i64 app_lambda_1(i64 value) { return value; }
static void app_lambda_2(struct tau_string message) { tau_panic(message); }

i64 app_populate_i64(struct std_collections_vector_i64* vec, u64 count, u64 seed){
	if(!vec){
		struct app_result_i64_string_s result;
		result.type = app_result_i64_string_e_Err;
		result.value_Err = tau_string_new("Input vector cannot be null");
		return result;
	}

	struct std_math_random random = std_math_random_new();

	if(seed == 0){
		std_math_random_randomize(&random);
	}
	else {
		std_math_random_seed(&random, seed);
	}

	while(count-- > 0){
		std_collections_vector_i64_push_back(vec, (i64)std_math_random_next_u64(&random));
	}

	struct app_result_i64_string_s result;
	result.type = app_result_i64_string_e_Ok;
	result.value_Ok = std_collections_vector_i64_get(vec, std_collections_vector_i64_index_of(vec, std_math_aggregate_i64_i64_i64(vec, &app_lambda_0)));
	return result;
}

void main(){
	struct std_collections_vector_i64 my_vector;

	i64 max_generated;

	struct app_result_i64_string_s tmp_match = app_populate_i64(&my_vector, 100, 0);
	switch(tmp_match.type){
	case app_result_i64_string_e_Ok: max_generated = app_lambda_1(tmp_match.value_Ok);
	case app_result_i64_string_e_Err: app_lambda_2(tmp_match.value_Err);
	}

	std_io_print(tau_i64_to_string(&max_generated));
}

---------------------------------------------------------------------------------------------------------
mod app;

include std.io;
include std.collections;
include std.math;

alias std.math.random as rng;

enum result<_ok_t, _err_t> {
	Ok(_ok_t),
	Err(_err_t),
}

pub fn populate<@primitive(_ty)>(ptr<std.collections.vector<_ty>> vec, u64 count, u64 seed) result<_ty, string> {
	if(!vec){
		return result<_ty, string>.Err("Input vector cannot be null");
	}

	rng random = rng.new();

	if(seed == 0){
		random.randomize();
	}
	else {
		random.seed(seed);
	}

	while(count-- > 0){
		vec.push_back(random.next_u64() as _ty);
	}

	return result<_ty, string>.Ok(vec[vec.index_of(std.math.aggregate<_ty, _ty, _ty>(
		vec,
		(_ty a, _ty b) _ty => { 
			return std.math.max<_ty>(a, b);
		}
	))]);
}

pub fn main() void {
	std.collections.vector<i64> my_vector;

	i64 max_generated = match(populate<i64>(&my_vector, 100, 0)){
		case Ok: (i64 value) i64 => { return value; }
		case Err: (string message) => { panic(message) }
	};

	std.io.print(max_generated.to_string());
}
*/

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

enum class NodeType : u64 {
	Undefined = 0,
	Annotation,
	Alias,
	Arguments,
	ArrayDeclaration,
	BinaryOperator,
	Break,
	Case,
	CBlock,
	Continue,
	Else,
	Enum,
	EnumMember,
	For,
	Finally,
	FunctionCall,
	FunctionDefinition,
	Identifier,
	If,
	ImmediateArray,
	ImmediateBool,
	ImmediateChar,
	ImmediateFloat,
	ImmediateInt,
	ImmediateString,
	Impl,
	Include,
	Interface,
	Lambda,
	Match,
	Module,
	Parameters,
	Return,
	StatementBlock,
	Struct,
	StructConstructor,
	Switch,
	TempToken,
	TemplateArgs,
	TemplateParams,
	UnaryOperator,
	Union,
	VariableDeclaration,
	While,
};

struct Annotation;
struct Alias;
struct Arguments;
struct ArrayDeclaration;
struct BinaryOperator;
struct Break;
struct Case;
struct CBlock;
struct Continue;
struct Else;
struct Enum;
struct EnumMember;
struct For;
struct Finally;
struct FunctionCall;
struct FunctionDefinition;
struct Identifier;
struct If;
struct ImmediateArray;
struct ImmediateBool;
struct ImmediateChar;
struct ImmediateFloat;
struct ImmediateInt;
struct ImmediateString;
struct Impl;
struct Include;
struct Interface;
struct Lambda;
struct Match;
struct Module;
struct Parameters;
struct Return;
struct StatementBlock;
struct Struct;
struct StructConstructor;
struct Switch;
struct TempToken;
struct TemplateArgs;
struct TemplateParams;
struct UnaryOperator;
struct Union;
struct VariableDeclaration;
struct While;

#define CONSTRUCTOR(node) inline node() : type{ NodeType::node }

struct Annotation {
	CONSTRUCTOR(Annotation) {}

	NodeType type;
	std::string name;
	Arguments* args = nullptr;
};
struct Alias {
	CONSTRUCTOR(Alias) {}

	NodeType type;
	NodeType* full_path = nullptr;
	NodeType* new_path = nullptr;
};
struct Arguments {
	CONSTRUCTOR(Arguments) {}

	NodeType type;
	std::vector<NodeType*> args;
};
struct ArrayDeclaration {
	CONSTRUCTOR(ArrayDeclaration) {};
	NodeType type;
	std::string name;
	NodeType* type_path = nullptr;
	NodeType* size_specifier = nullptr;
	NodeType* default_value = nullptr;
};
struct BinaryOperator {
	CONSTRUCTOR(BinaryOperator) {}

	NodeType type;
	OperatorID operator_ = OperatorID::Undefined;
	NodeType* lhs = nullptr;
	NodeType* rhs = nullptr;
};
struct Break {
	CONSTRUCTOR(Break) {}
	NodeType type;
};
struct Case {
	CONSTRUCTOR(Case) {}
	NodeType type;
	NodeType* expression = nullptr;
	NodeType* body = nullptr;
};
struct CBlock {
	CONSTRUCTOR(CBlock) {}
	NodeType type;
	std::vector<TempToken*> tokens;
};
struct Continue {
	CONSTRUCTOR(Continue) {}
	NodeType type;
};
struct Else {
	CONSTRUCTOR(Else) {}
	NodeType type;
	If* if_branch = nullptr;
	StatementBlock* my_branch = nullptr;
};
struct Enum {
	CONSTRUCTOR(Enum) {}

	NodeType type;
	std::string name;
	std::vector<EnumMember*> members;
	TemplateParams* template_ = nullptr;
};
struct EnumMember {
	CONSTRUCTOR(EnumMember) {}
	NodeType type;
	std::string name;
	std::vector<NodeType*> params;
};
struct For {
	CONSTRUCTOR(For) {}

	NodeType type;

};
struct Finally {
	CONSTRUCTOR(Finally) {}

	NodeType type;

};
struct FunctionCall {
	CONSTRUCTOR(FunctionCall) {}

	NodeType type;
	NodeType* function_path = nullptr;
	Arguments* args = nullptr;
};
struct FunctionDefinition {
	CONSTRUCTOR(FunctionDefinition) {}

	NodeType type;
	std::string name;
	NodeType* return_type_path = nullptr;
	Parameters* params = nullptr;
	StatementBlock* body = nullptr;
	TemplateParams* template_ = nullptr;
};
struct Identifier {
	CONSTRUCTOR(Identifier) {}
	NodeType type;
	std::string name;
	TemplateArgs* args = nullptr;
};
struct If {
	CONSTRUCTOR(If) {}
	NodeType type;
	NodeType* condition = nullptr;
	StatementBlock* my_branch = nullptr;
	Else* else_branch = nullptr;
};
struct ImmediateArray {
	CONSTRUCTOR(ImmediateArray) {}
	NodeType type;
};
struct ImmediateBool {
	CONSTRUCTOR(ImmediateBool) {}
	NodeType type;
	bool value = false;
};
struct ImmediateChar {
	CONSTRUCTOR(ImmediateChar) {}
	NodeType type;
	char value = 0;
};
struct ImmediateFloat {
	CONSTRUCTOR(ImmediateFloat) {}
	NodeType type;
	double value = 0;
};
struct ImmediateInt {
	CONSTRUCTOR(ImmediateInt) {}
	NodeType type;
	u64 value = 0;
};
struct ImmediateString {
	CONSTRUCTOR(ImmediateString) {}
	NodeType type;
	std::string value;
};
struct Impl {
	CONSTRUCTOR(Impl) {}
	NodeType type;

};
struct Include {
	CONSTRUCTOR(Include) {}
	NodeType type;
	NodeType* path = nullptr;
	bool is_c_include = false;
	std::string c_include;
};
struct Lambda {
	CONSTRUCTOR(Lambda) {}
	NodeType type;
	Parameters* params = nullptr;
	NodeType* return_type_path = nullptr;
	StatementBlock* body = nullptr;
};
struct Match {
	CONSTRUCTOR(Match) {}
	NodeType type;
	NodeType* condition = nullptr;
	Case* cases = nullptr;
};
struct Module {
	CONSTRUCTOR(Module) {}
	NodeType type;
	NodeType* name_path = nullptr;
	std::vector<Struct*> structs;
	std::vector<FunctionDefinition*> functions;
	std::vector<Enum*> enums;
	std::vector<Include*> includes;
	std::vector<Alias*> aliases;
};
struct Param {
	std::string name;
	NodeType* type_path = nullptr;
};
struct Parameters {
	CONSTRUCTOR(Parameters) {}
	NodeType type;
	std::vector<Param> params;
};
struct Return {
	CONSTRUCTOR(Return) {}
	NodeType type;
	NodeType* value = nullptr;
};
struct StatementBlock {
	CONSTRUCTOR(StatementBlock) {}
	NodeType type;
	std::vector<NodeType*> statements;
};
struct Struct {
	CONSTRUCTOR(Struct) {}
	NodeType type;
	std::string name;
	std::vector<VariableDeclaration*> members;
	TemplateParams* template_ = nullptr;
};
struct StructConstructor {
	CONSTRUCTOR(StructConstructor) {}
	NodeType type;

};
struct Switch {
	CONSTRUCTOR(Switch) {}
	NodeType type;

};
struct TempToken {
	CONSTRUCTOR(TempToken) {}
	NodeType type;
	tau::token tok = {};
};
struct TemplateArgs {
	CONSTRUCTOR(TemplateArgs) {}
	NodeType type;
	std::vector<NodeType*> args;
};
struct TemplateParam {
	std::string name;
	Annotation* annotation = nullptr;
};
struct TemplateParams {
	CONSTRUCTOR(TemplateParams) {}
	NodeType type;
	std::vector<TemplateParam> params;
};
struct UnaryOperator {
	CONSTRUCTOR(UnaryOperator) {}
	NodeType type;
	OperatorID operator_;
	NodeType* value = nullptr;
};
struct Union {
	CONSTRUCTOR(Union) {}

	NodeType type;

};
struct While {
	CONSTRUCTOR(While) {}
	NodeType type;
	NodeType* condition = nullptr;
	StatementBlock* body = nullptr;
};

#define NODE(a) reinterpret_cast<NodeType*>(a)


int main(int argc, char** argv) {
	
	Module* mod = new Module();
	Identifier* mod_name = new Identifier();
	mod_name->name = "app";
	mod->name_path = NODE(mod_name);

	Include* std_io = new Include();
	Identifier* std_io_name_std = new Identifier();
	Identifier* std_io_name_io = new Identifier();
	BinaryOperator* std_io_dot = new BinaryOperator();

	std_io_name_std->name = "std";
	std_io_name_io->name = "io";
	std_io_dot->operator_ = OperatorID::Dot;
	std_io_dot->lhs = NODE(std_io_name_std);
	std_io_dot->rhs = NODE(std_io_name_io);

	std_io->path = NODE(std_io_dot);

	mod->includes.push_back(std_io);

	Include* std_collections = new Include();
	Identifier* std_collections_name_std = new Identifier();
	Identifier* std_collections_name_collections = new Identifier();
	BinaryOperator* std_collections_dot = new BinaryOperator();

	std_collections_name_std->name = "std";
	std_collections_name_collections->name = "collections";
	std_collections_dot->operator_ = OperatorID::Dot;
	std_collections_dot->lhs = NODE(std_collections_name_std);
	std_collections_dot->rhs = NODE(std_collections_name_collections);

	std_collections->path = NODE(std_collections_dot);

	mod->includes.push_back(std_collections);

	Include* std_math = new Include();
	Identifier* std_math_name_std = new Identifier();
	Identifier* std_math_name_collections = new Identifier();
	BinaryOperator* std_math_dot = new BinaryOperator();

	std_math_name_std->name = "std";
	std_math_name_collections->name = "collections";
	std_math_dot->operator_ = OperatorID::Dot;
	std_math_dot->lhs = NODE(std_math_name_std);
	std_math_dot->rhs = NODE(std_math_name_collections);

	std_math->path = NODE(std_math_dot);

	mod->includes.push_back(std_math);


	Alias* alias_math = new Alias();
	Identifier* alias_math_std = new Identifier();
	Identifier* alias_math_math = new Identifier();
	Identifier* alias_math_random = new Identifier();
	Identifier* alias_math_rng = new Identifier();

	BinaryOperator* alias_math_dot0 = new BinaryOperator();
	BinaryOperator* alias_math_dot1 = new BinaryOperator();

	alias_math_std->name = "std";
	alias_math_math->name = "math";
	alias_math_random->name = "random";
	alias_math_rng->name = "rng";

	alias_math_dot1->operator_ = OperatorID::Dot;
	alias_math_dot1->lhs = NODE(alias_math_math);
	alias_math_dot1->rhs = NODE(alias_math_random);
	
	alias_math_dot0->operator_ = OperatorID::Dot;
	alias_math_dot0->lhs = NODE(alias_math_std);
	alias_math_dot0->rhs = NODE(alias_math_dot1);

	alias_math->full_path = NODE(alias_math_dot0);
	alias_math->new_path = NODE(alias_math_rng);

	mod->aliases.push_back(alias_math);

	TemplateParams* enum_result_template = new TemplateParams();
	enum_result_template->params.push_back({ "_ok_t" });
	enum_result_template->params.push_back({ "_err_t" });

	Enum* enum_result = new Enum();
	enum_result->name = "result";
	enum_result->template_ = enum_result_template;
	
	EnumMember* enum_result_ok = new EnumMember();
	EnumMember* enum_result_err = new EnumMember();

	Identifier* _ok_t = new Identifier();
	Identifier* _err_t = new Identifier();

	_ok_t->name = "_ok_t";
	_err_t->name = "_err_t";

	enum_result_ok->name = "Ok";
	enum_result_ok->params.push_back(NODE(_ok_t));
	
	enum_result_err->name = "Err";
	enum_result_err->params.push_back(NODE(_err_t));

	enum_result->members.push_back(enum_result_ok);
	enum_result->members.push_back(enum_result_err);

	mod->enums.push_back(enum_result);

	Annotation* primitive = new Annotation();
	primitive->name = "primitive";

	FunctionDefinition* populate = new FunctionDefinition();
	TemplateParams* populate_template = new TemplateParams();
	populate_template->params.push_back({ "_ty", primitive });

	Identifier *populate_ret_path = new Identifier();
	TemplateArgs* populate_ret_path_template = new TemplateArgs();

	Identifier* _ty = new Identifier();
	Identifier* _string = new Identifier();

	_ty->name = "_ty";
	_string->name = "string";

	populate_ret_path_template->args.push_back(NODE(_ty));
	populate_ret_path_template->args.push_back(NODE(_string));

	populate_ret_path->name = "result";
	populate_ret_path->args = populate_ret_path_template;

	populate->name = "populate";
	populate->return_type_path = NODE(populate_ret_path);
	populate->template_ = populate_template;

	Parameters* populate_params = new Parameters();
	// populate params

	return 0;
}












/*
#include "toml.h"

std::vector<std::string> get_args(int argc, char** argv) {
	std::vector<std::string> args;
	for (int i = 1; i < argc; i++) {
		args.push_back(argv[i]);
	}
	return args;
}

void generate_project_toml(const std::string& name) {
	std::ofstream f("./" + name + "/project.toml");

	f << "[config]\n";
	f << "name = '" << name << "'\n";
	f << "version = '0.0.1'\n";
	f << "target = 'application'\n";
	f << "compiler = 'default'\n";
	f << "\n\n[libs]\n";

	f.close();
}

void generate_main_tau(const std::string& name) {
	std::ofstream f("./" + name + "/src/main.tau");

	f << "mod app;\n\n";
	f << "include _C \"stdio.h\"\n\n";

	f << "fn main() {\n";
	f << "   //std.io.print(\"Hello World\");\n";
	f << "   i32 x = 10;\n";
	f << "   inline _C {\n";
	f << "      printf(\"Hello World %d\", x);\n";
	f << "   }\n";
	f << "}\n\n";

	f.close();
}

void build_project();


void copyFolder(const std::filesystem::path& source, const std::filesystem::path& destination) {
	namespace fs = std::filesystem;

	try {
		// Iterate over the files and subdirectories in the source folder
		for (const auto& entry : fs::recursive_directory_iterator(source)) {
			// Create the corresponding path in the destination folder
			fs::path destinationPath = destination / entry.path().lexically_relative(source);

			// Check if it's a directory and create it in the destination
			if (fs::is_directory(entry)) {
				fs::create_directories(destinationPath);
			}
			// Otherwise, it's a regular file, so copy it
			else if (fs::is_regular_file(entry)) {
				fs::copy(entry, destinationPath, fs::copy_options::overwrite_existing);
			}
		}
	}
	catch (const std::filesystem::filesystem_error& e) {
		std::cerr << "Error copying folder: " << e.what() << std::endl;
	}
}

void build_file(std::filesystem::path filename);

int main(int argc, char** argv) {
	using namespace tau;

	std::vector<std::string> args = get_args(argc, argv);

	if (args.size() == 2) {
		if (args[0] == "create") {
			std::string name = args[1];

			std::filesystem::create_directory(".\\" + name);
			std::filesystem::create_directory(".\\" + name + "\\src\\");
			std::filesystem::create_directory(".\\" + name + "\\tmp\\");
			std::filesystem::create_directory(".\\" + name + "\\bin\\");
			std::filesystem::create_directory(".\\" + name + "\\libs\\");

			copyFolder(".\\data\\bin\\cc\\", ".\\" + name + "\\bin\\cc\\");
			//copyFolder(".\\data\\stdlib\\", ".\\" + name + "\\libs\\stdlib\\");

			generate_project_toml(name);
			generate_main_tau(name);

			return 0;
		}
	}
	if (args.size() == 0) {
		std::cout << "tau [create] [name]\n";
		std::cout << "tau [build]\n";
		return 0;
	}

	if (args[0] == "build") {
		build_project();
	}
	else if (args[0] == "debug") {
		build_file("./hello/src/main.tau");
	}

	return 0;
}

void build_file(std::filesystem::path filename) {
	std::string input_file = filename.string();
	std::stringstream text;
	std::ifstream in(input_file);

	while (in.good()) {
		std::string line;
		std::getline(in, line);
		text << line << "\n";
	}

	in.close();

	tau::TokenStream tokens;
	tau::result<bool> result = tau::Tokenize(text.str().c_str(), input_file, tokens);

	if (result.error_bit) {
		std::cout << result.error << "\n";
		std::cout << "Please correct the error and try again.\n";
		return;
	}

	tau::Parser parser;
	tau::InitializeTauParser(parser);
	
	tau::AstNode* node = parser.parse_eval(tokens, "Module");
	tau::ParserContext ctx = parser.get_context();

	if (node == nullptr) {
		std::cout << "Error compiling file\n";
		return;
	}

	tau::ModuleNode* modul = dynamic_cast<tau::ModuleNode*>(node);

	std::string module_name = modul->moduleName->get_full_name();

	std::string outname = "./tmp/";
	outname += module_name;


	std::ofstream fsout(outname + ".c");
	if (!node->compile(fsout, ctx)) {
		std::cout << "Error compiling\n";
		fsout.close();
		return;
	}
	fsout.close();

	fsout.open(outname + ".h");
	modul->compile_header(fsout, ctx);
	fsout.close();
}

void compile_project();

void build_project() {
	if (!std::filesystem::exists("project.toml")) {
		std::cout << "Could not find project config file.\n";
		return;
	}

	std::filesystem::path src = "./src/";
	for (const auto& file_ : std::filesystem::directory_iterator(src)) {
		if (!std::filesystem::is_regular_file(file_.path())) {
			continue;
		}
		if (file_.path().extension() != ".tau") {
			std::cout << "Skipping " << file_.path().string() << "\n";
			continue;
		}
		std::cout << "Compiling " << file_.path().string() << "...";
		build_file(file_.path());
		std::cout << "Done.\n";
	}

	compile_project();
}

void compile_project() {
	std::cout << "Compiling...\n";

	auto project = toml::parse_file("project.toml");

	std::string_view compiler = project["config"]["compiler"].value_or(std::string_view(""));
	if (compiler == "default") {
		compiler = ".\\bin\\cc\\tcc.exe";
	}
	std::string_view name = project["config"]["name"].value_or(std::string_view(""));

	std::stringstream command;
	command << compiler << " -o .\\bin\\" << name << ".exe " << ".\\tmp\\*.c";
	std::cout << command.str() << "\n";
	std::system(command.str().c_str());
}

/*
// NOTE: we are missing the final .z when we parse this input. Figure out why
	std::string_view input = //"(-2 + std.constants<f32, f64>.PI) * 6 / abc<z>.efg<yxw>.hij<vut>.klm<srq>.nop.qrs<pon>.tuv<int, int, implicit<int>>.wxy.z "; //"-2 + 2, 3, -5, 6, 10, 9";
		R"( mod app;

			struct vec2 {
				pub f32 x;
				pub f32 y;
			}

			pub fn main(i32 argc, char argv) void {
				i32 a = 2;
				i32 b = 4;
				i32 c = a + b;
			}

			fn foo(){

			}

		)";
	tau::TokenStream tokens;
	tau::result<bool> result = tau::Tokenize(input, "foo.tau", tokens);

	if (result.error_bit) {
		std::cout << result.error << "\n";
		std::cout << "Please correct the error and try again.\n";
		return 1;
	}

	tau::Parser parser;
	tau::InitializeTauParser(parser);
	AstNode* program = parser.parse_eval(tokens, "Module");

	tau::ParserContext parser_context = parser.get_context();

	dynamic_cast<ModuleNode*>(program)->compile_header(std::cout, parser_context);
	program->compile(std::cout, parser_context);


*/