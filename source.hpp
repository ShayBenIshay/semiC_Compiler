#ifndef SOURCE_HPP
#define SOURCE_HPP

#include <stdlib.h>
#include <string>
#include <cassert>	
#include <vector>
#include "bp.hpp"
#include "hw3_output.hpp"
#include <iostream>


using namespace std;
using namespace output;
// #define DEBUG(print1,print2) {cout<<print1<<"-------"<<print2<<"\n";}
#define DEBUGGING(print1) {DEBUG(print1,"")}
#define DEBUG(print1,print2);
#define BUFFER (CodeBuffer::instance())



struct Id;
struct Node{
	string type;
	Node(string type="UNDEFINED_TYPE"):type(type){}
	virtual string get_type(){return type;}
};

struct ParamVec : public Node {
	vector<Id*> func_params;
	bool params_validation(ParamVec& p);
};

struct String : public Node {
	string str;
	String(string value,string type="STRING"): Node(type),str(value){} 
};


struct Id : public Node {
	string name;
	ParamVec* param_vec;
	string place;
	vector<pair<int, BranchLabelIndex>> next_list;
	vector<pair<int, BranchLabelIndex>> break_list;
	vector<pair<int, BranchLabelIndex>> continue_list;
	vector<pair<int, BranchLabelIndex>> true_list;
	vector<pair<int, BranchLabelIndex>> false_list;
	Id(string name, string type="UNDEFINED_TYPE", ParamVec* func_vec_object=nullptr, string place="%0"): Node(type), name(name),\
	 param_vec(func_vec_object), place(place){
		next_list= vector<pair<int, BranchLabelIndex>>();
		break_list = vector<pair<int, BranchLabelIndex>>();
		continue_list = vector<pair<int, BranchLabelIndex>>();
		true_list = vector<pair<int, BranchLabelIndex>>();
		false_list = vector<pair<int, BranchLabelIndex>>();
	 }

	bool is_func(){return param_vec!=nullptr;}
};

// HW5/////////////////////////////////
struct Statement : public Id {
	bool has_unexpected_break_or_continue;
	int errline_br_cn;
	bool has_return;
	Statement() : Id("dont care","VOID"), has_unexpected_break_or_continue(false), errline_br_cn(-1), has_return(false){}
	Statement(string name, string type="VOID", ParamVec* func_vec_object=nullptr, string place = "undefined", bool has_unexpected_break_or_continue=false, int errline_br_cn=-1,bool has_return = false): \
	Id(name,type,func_vec_object,place), has_unexpected_break_or_continue(has_unexpected_break_or_continue), errline_br_cn(errline_br_cn), has_return(has_return){}
	~Statement() = default;
	
	
};

struct EntrySymbolTable{
	Id* id_node;
	int offset;
	int num_of_params_in_func;
	EntrySymbolTable(Id* id , int offset,int num_of_params_in_func=-1) : id_node(id), offset(offset),\
	num_of_params_in_func(num_of_params_in_func){}
};


struct SymbolTable{
public:
	vector<EntrySymbolTable*> entries_vector;
	string func_ret_type;
	SymbolTable(string func_ret_type=""):func_ret_type(func_ret_type){
		entries_vector = vector<EntrySymbolTable*>();
	}
};


struct SymbolTableManager {
	vector<SymbolTable*> symbol_table_stack;
	vector<int> offsets_stack;
	SymbolTableManager();
	~SymbolTableManager();
	void print_stack_variables_and_scopes();
	void pop_scope();

	void push_scope();

	bool id_valid_for_use(string id);
	bool id_defined(string id);
	Id* get_id_by_name(string name);
	EntrySymbolTable* get_entry_by_name(string name);

	void insert_id(string name,string type="UNDEFINED_TYPE",ParamVec* param_vec=nullptr);
	void insert_id(Id* id);
	void validate_return_type(string type, int line);
	
};

struct LogicOp : public Id{
public:
    Id* exp1;
    Id* exp2;
    int line;
    // LogicOp(Id* exp1,Id* exp2,int line){
    LogicOp(Id* exp1 = nullptr,Id* exp2 = nullptr, string name = "logic", int line = -1):Id(name,"BOOL"), exp1(exp1), exp2(exp2), line(line){}
    ~LogicOp() = default;
        //if(exp1->type != "BOOL" || exp2->_type != "BOOL") {
            //error
            //errorMismatch(line);
            //exit(0);
        //}
        // this->exp1 = exp1;
        // this->exp2 = exp2;
        //_type = "BOOL";
        //_name = "n/a";
    // }
};

struct Relop : public Id{
public:
    Id* exp1;
    Id* exp2;
    int line;
    // LogicOp(Id* exp1,Id* exp2,int line){
    Relop(Id* exp1 = nullptr,Id* exp2 = nullptr, string name = "relop", int line = -1):Id(name,"BOOL"), exp1(exp1), exp2(exp2), line(line){}
    ~Relop() = default;
        //if(exp1->type != "BOOL" || exp2->_type != "BOOL") {
            //error
            //errorMismatch(line);
            //exit(0);
        //}
        // this->exp1 = exp1;
        // this->exp2 = exp2;
        //_type = "BOOL";
        //_name = "n/a";
    // }
};

struct Not : public Id{
public:
    Id* exp;
    int line;
    // LogicOp(Id* exp1,Id* exp2,int line){
    Not(Id* exp = nullptr, string name = "not", int line = -1):Id(name,"BOOL"), exp(exp), line(line){}
    ~Not() = default;
        //if(exp1->type != "BOOL") {
            //error
            //errorMismatch(line);
            //exit(0);
        //}
        // this->exp = exp1;
        //_type = "BOOL";
        //_name = "n/a";
    // }
};

struct Binop : public Id{
public:
    Id* exp1;
    Id* exp2;
    int line;
    // LogicOp(Id* exp1,Id* exp2,int line){
    Binop(Id* exp1 = nullptr,Id* exp2 = nullptr, string name = "binop", int line = -1): Id(name,"INT"), exp1(exp1), exp2(exp2), line(line){}
    ~Binop() = default;
};

//PARSER HANDELING FUNCTIONS:::
//PARSER HANDELING FUNCTIONS:::
void init();
void end_of_scope();
void end_of_program();
void insert_function_paramaters(Id* ret_type,Id* func_id,ParamVec* param_vec);
void insert_function_id(Id* func_id,Statement* statement);
void statements_statement_rule(Statement* statements,Statement* one_statement);
Statement* statement_type_id_assign(Id* type_node,Id* id,Id* exp);
Statement* statement_id_assign(Id* id,Id* exp);
void statement_if(Id* exp,Statement* statement);
void statement_if_else(Id* exp,Statement* statement1,Statement* statement2);
void statement_while(Id* exp,Statement* statement);
void statement_while_else(Id* exp,Statement* statement1,Statement* statement2);
Statement* call_id_explist(Id* id,ParamVec* param_vec);
Id* call_id(Id* id);
Binop* exp_binop_exp(Id* exp1, Binop* op,Id* exp2);
Binop* exp_priobinop_exp(Id* exp1, Binop* op, Id* exp2);
Relop* exp_eqrelop_exp(Id* exp1, Relop* op, Id* exp2);
Relop* exp_relop_exp(Id* exp1, Relop* op, Id* exp2);
LogicOp* exp_or_exp(Id* exp1, LogicOp* op,Id* marker_id ,Id* exp2);
LogicOp* exp_and_exp(Id* exp1, LogicOp* op,Id* marker_id ,Id* exp2);


void emit_function_params(Id* ret_type,Id* func_id,ParamVec* param_vec);
void emit_function_id(Id* ret_type, Id* func_id,Statement* statement);
string load_exp_value(Id* exp);
string type_to_lower(Id* id);
string llvm_type(Id* id);
void initVar(string var,EntrySymbolTable* var_entry);
// Id* exp_eval(Id* exp);


extern SymbolTableManager symbol_table_manager;
#include "CodeGen.hpp"
extern int yylineno;
extern CodeGen code_gen;

	
#define YYSTYPE Node*

#endif