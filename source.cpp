#include "source.hpp"
#include "bp.hpp"

extern int yylineno;
	
using namespace output;
using namespace std;

bool ParamVec::params_validation(ParamVec& p){
	if(p.func_params.size()!=func_params.size()){
		return false;
	}
	for(int i=0;i<p.func_params.size();i++){
		if( (func_params[i]->type=="INT" && (p.func_params[i]->type=="BYTE") ) || func_params[i]->type==p.func_params[i]->type){
			//ok
		}else{
			//error
			return false;
		}
	}
	return true; 	
}


SymbolTableManager::SymbolTableManager():symbol_table_stack(),offsets_stack(){
	offsets_stack.push_back(0);
	SymbolTable* global_scope = new SymbolTable();
	symbol_table_stack.push_back(global_scope);
}

SymbolTableManager::~SymbolTableManager(){
	int i=symbol_table_stack.size()-1;
	// for (SymbolTable* symbol_table : symbol_table_stack){
	// 		for(EntrySymbolTable* entry : symbol_table->entries_vector){
	// 			if (entry->id_node->is_func()){
	// 				for (Id* id : entry->id_node->param_vec->func_params){
	// 					delete(id);
	// 				} 
	// 				 delete(entry->id_node->param_vec);
	// 			}
	// 			 delete(entry->id_node);
	// 			 delete(entry);
	// 		}
	// 		 delete(symbol_table);
	// }

}


void SymbolTableManager::pop_scope() {
	assert(offsets_stack.size() == symbol_table_stack.size());
	assert(offsets_stack.size()>0);
	offsets_stack.pop_back();
	symbol_table_stack.pop_back();
}

void SymbolTableManager::push_scope(){
	SymbolTable* new_scope= new SymbolTable();
	symbol_table_stack.push_back(new_scope);
	offsets_stack.push_back(offsets_stack.back());
}

bool SymbolTableManager::id_valid_for_use(string id){
	return get_id_by_name(id)==nullptr;
}
bool SymbolTableManager::id_defined(string id){
	return !(id_valid_for_use(id));
}

Id* SymbolTableManager::get_id_by_name(string name){
	EntrySymbolTable * res=get_entry_by_name(name);

	// return get_entry_by_name(name)->id_node;		
	return res==nullptr ? (Id*)res : res->id_node;
}


EntrySymbolTable* SymbolTableManager::get_entry_by_name(string name){
	int len = symbol_table_stack.size();
	for(int i=len-1;i>=0;i--){
		SymbolTable* symbol_table = symbol_table_stack[i];
		for (EntrySymbolTable* entry : symbol_table->entries_vector){
			if(name==entry->id_node->name) return entry;
		}
	}
	return nullptr;		
}

void SymbolTableManager::print_stack_variables_and_scopes(){	
	cout << "------------------------------------------------------\n";
	for ( int i= symbol_table_stack.size()-1; i >= 0; i--){
		SymbolTable* symbol_table = symbol_table_stack[i];
		cout << "Printing Scope Nuber : " << i << "\n";
		for	 ( EntrySymbolTable* entry : symbol_table->entries_vector){

			output::printID(entry->id_node->name,entry->offset,entry->id_node->type);
			if (entry->id_node->is_func()){
				DEBUGGING("ARGUMENT");
				int i=0;
				for (Id* id : entry->id_node->param_vec->func_params){
					DEBUG(id->type,i);
			}
			}
		}
	}
	cout << "------------------------------------------------------\n";

}

void SymbolTableManager::insert_id(string name,string type,ParamVec* param_vec){
	int offset=offsets_stack.back();
	///if param!=null -> offset=offset. if param == null offset++
	if (param_vec != nullptr){offset=0;}
	Id* new_id=new Id(name,type,param_vec);
	BUFFER.emit(";id name is:"+name);
	BUFFER.emit(";id place is:"+new_id->place);
	EntrySymbolTable* entry=new EntrySymbolTable(new_id,offset);
	symbol_table_stack.back()->entries_vector.push_back(entry);

	//push vector params 
	if (param_vec != nullptr){
		//additional scope for function
		push_scope();
		int param_offset=-1;
		//pushing function params to the new scope
		for(int i = param_vec->func_params.size()-1; i>=0 ; i--){
			//in case of function decleration
			//the only place we assume param vec has func params of type vector<Id*> and not vector<Node*>
			//in parser.ypp we are inserting Id* types to func params when defining a function
			//we do that to push them as a entry to the scope (we need the Identifier name for that)
			Id* id = (Id*)param_vec->func_params[i];
			new_id = new Id(id->name,id->type);
			EntrySymbolTable* entry_func=new EntrySymbolTable(new_id,param_offset,param_vec->func_params.size());
			symbol_table_stack.back()->entries_vector.push_back(entry_func);
			param_offset--;
		}
		
	}
	if (param_vec == nullptr){ 
		offsets_stack.back()++;
	}
}

//PARSING RULE FUNCTIONS FOR  "parser.ypp"
//PARSING RULE FUNCTIONS FOR  "parser.ypp"
//PARSING RULE FUNCTIONS FOR  "parser.ypp"
//PARSING RULE FUNCTIONS FOR  "parser.ypp"
//PARSING RULE FUNCTIONS FOR  "parser.ypp"
void init(){
	symbol_table_manager.push_scope();
	Id* param1=new Id("STRING_PARAM","STRING");
	Id* param2=new Id("INT_PARAM","INT");

	ParamVec* print_params = new ParamVec();
	print_params->func_params.push_back(param1);
	ParamVec* printi_params = new ParamVec();
	printi_params->func_params.push_back(param2);
	Id *print_id=new Id("print","VOID",print_params);
	Id *printi_id=new Id("printi","VOID",printi_params);
	EntrySymbolTable* entry_print = new EntrySymbolTable(print_id,0);
	EntrySymbolTable* entry_printi = new EntrySymbolTable(printi_id,0);
	symbol_table_manager.symbol_table_stack.back()->entries_vector.push_back(entry_print);
	symbol_table_manager.symbol_table_stack.back()->entries_vector.push_back(entry_printi);

	BUFFER.emitGlobal("declare i32 @printf(i8*, ...)");
    BUFFER.emitGlobal("declare void @exit(i32)");
    BUFFER.emitGlobal("@.int_specifier = constant [4 x i8] c\"%d\\0A\\00\"");
    BUFFER.emitGlobal("@.str_specifier = constant [4 x i8] c\"%s\\0A\\00\"");
    BUFFER.emitGlobal("@.zero_div_err = constant [24 x i8] c\"Error division by zero\\0A\\00\"");
    BUFFER.emit("define void @printi(i32) {");
    BUFFER.emit("call i32 (i8*, ...) @printf(i8* getelementptr([4 x i8], [4 x i8]* @.int_specifier, i32 0, i32 0), i32 %0)");
    BUFFER.emit("ret void");
    BUFFER.emit("}");
    BUFFER.emit("define void @print(i8*) {");
    BUFFER.emit("call i32 (i8*, ...) @printf(i8* getelementptr ([4 x i8], [4 x i8]* @.str_specifier, i32 0, i32 0), i8* %0)");
    BUFFER.emit("ret void");
    BUFFER.emit("}");
  
}

void end_of_scope(){
	// endScope();
	// string str;
	// int offset;		

	// //symbol_table_manager.print_stack_variables_and_scopes();
	// SymbolTable* symbol_table = symbol_table_manager.symbol_table_stack.back();
	// for (EntrySymbolTable* entry : symbol_table->entries_vector){
	// 	str = entry->id_node->type;
	// 	offset = entry->offset;
	// 	if(entry->id_node->is_func()){
	// 		//get types of func_params
	// 		vector<string> types;
	// 		for (Id* id : entry->id_node->param_vec->func_params){
	// 			types.insert(types.begin(),id->type);
	// 		}	
	// 			str = makeFunctionType(entry->id_node->type,types);
	// 			offset = 0;
	// 	}
	// 	//print id entry
	// 	printID(entry->id_node->name,offset,str);
	// }		
}

void end_of_program(){
	Id* main_func = symbol_table_manager.get_id_by_name("main");
	if(main_func == nullptr ||  !main_func->is_func() || main_func->type != "VOID" || main_func->param_vec->func_params.size()>0){
		//error
		errorMainMissing();
		exit(1);
	}

	end_of_scope();
	symbol_table_manager.pop_scope();
	BUFFER.printGlobalBuffer();
    BUFFER.printCodeBuffer();

}

void insert_function_paramaters(Id* ret_type,Id* func_id,ParamVec* param_vec){
	for(Id* id : param_vec->func_params){
		//we assume param vec has Id* objects in his func_params vector
		//go read SymbolTableManager::insert_id for more info

		if ( symbol_table_manager.id_valid_for_use(((Id*)id)->name) ){ 
			//ok
		}else{errorDef(yylineno,(((Id*)id)->name));exit(1);}
	}

	if(symbol_table_manager.id_valid_for_use(func_id->name)){
		//ok
	}else{errorDef(yylineno,(func_id->name));exit(1);}
	symbol_table_manager.insert_id( func_id->name,ret_type->type,param_vec);
}

void insert_function_id(Id* func_id,Statement* statement){
	/* no need to enforce statement_ret_type == function_ret_type
	if( statement->type == ret_type->type){//ok}else{
		//error
		errorMismatch(yylineno);
		exit(1);}
	*/
	Id* id = symbol_table_manager.get_id_by_name(func_id->name);
	if(statement->has_unexpected_break_or_continue){
		DEBUGGING("insert_function_id")
		DEBUG("func_id->name",func_id->name)
		errorUnexpectedBreak(statement->errline_br_cn);
		exit(1);
	}
	// delete(func_id);
	// delete(statement);
	end_of_scope();
	symbol_table_manager.pop_scope();
}

void statements_statement_rule(Statement* statements, Statement* one_statement){
	Statement* statement_to_delete;
	if ((statements->type != one_statement->type ) && statements->type != "VOID" && one_statement->type != "VOID"){

		if ((statements->type == "INT" || statements->type== "BYTE" )&&(one_statement->type == "INT"\
		 || one_statement->type== "BYTE" )){
			statements->type="INT";
		}
	}else{
		//we dont need to save ret type anymore
		//one type is void and one type is not
		if( statements->type == "VOID"){
			statement_to_delete = one_statement;
		}else{
			statement_to_delete = one_statement;
		}
	}

}

Statement* statement_type_id_assign(Id* type_node,Id* id,Id* exp){
	id->type = "VOID";
	if (exp->type == "UNDEFINED_TYPE"){
		exp->type = symbol_table_manager.get_id_by_name(exp->name)->type;
	}
	DEBUGGING("statement_id_assign")
	DEBUG("id->type",type_node->type)
	DEBUG("exp->type",exp->type)
		
	if((type_node->type == exp->type )||( type_node->type == "INT" && exp->type == "BYTE")){
		//ok
	}
	else{
		//error	
		errorMismatch(yylineno);
		exit(1);
	}
	if (symbol_table_manager.id_valid_for_use(id->name)){ 
		//add entry
		symbol_table_manager.insert_id(id->name,type_node->type);
	}else {
		//error
		errorDef(yylineno,id->name);
		exit(1);
	}
	EntrySymbolTable * id_entry=symbol_table_manager.get_entry_by_name(id->name);
	Statement* statement = new Statement();
	code_gen.emit_type_id_asmt(type_node,id_entry,exp,statement);

	// delete(id);
	// delete(type_node);
	// delete(exp);		
	return statement;
}

Statement* statement_id_assign(Id* id,Id* exp){
	if(!symbol_table_manager.id_defined(id->name)){
		errorUndef(yylineno,id->name);
		exit(1);
	}
	if(exp->type == "UNDEFINED_TYPE"){
		exp = symbol_table_manager.get_id_by_name(exp->name);
	}
	Id* id_to_return = symbol_table_manager.get_id_by_name(id->name);
	if(id_to_return != nullptr && !id_to_return->is_func()){

		if((id_to_return->type == exp->type )||( id_to_return->type == "INT" && exp->type == "BYTE")){
			//ok
		}else{
			//error	
			errorMismatch(yylineno);
			exit(1);
		}	
	}else{
		errorUndef(yylineno,id_to_return->name);
		exit(1);
	}
	// delete(exp);
	EntrySymbolTable * id_entry=symbol_table_manager.get_entry_by_name(id->name);
	Statement* statement = new Statement();
	statement->type = "VOID";
	code_gen.emit_id_asmt(id_entry,exp,statement);
	return statement;
}

void statement_if(Id* exp,Statement* statement){
	statement_while(exp,statement);
}
	
void statement_if_else(Id* exp,Statement* statement1,Statement* statement2){
	 statement_while_else(exp,statement1,statement2);
}

void statement_while(Id* exp,Statement* statement){
	if(exp->type == "UNDEFINED_TYPE"){
		exp = symbol_table_manager.get_id_by_name(exp->name);
	}
	
	if( exp->type != "BOOL"){
		errorMismatch(yylineno);
		exit(1);
	}
	// delete(exp);
	//print end_scope
	// statement->has_unexpected_break_or_continue=false;
	end_of_scope();
	symbol_table_manager.pop_scope();
}

void statement_while_else(Id* exp,Statement* statement1,Statement* statement2){
	
	Statement* statement_to_return = statement2;
	if((statement1->type == "BYTE" || statement1->type == "INT") && (statement2->type == "BYTE" || statement2->type == "INT")){
		if(statement1->type != statement2->type) statement_to_return->type = "INT";
	}
	if( exp->type != "BOOL"){
		//error
		errorMismatch(yylineno);
		exit(1);
	}
	// statement_to_return->has_unexpected_break_or_continue=false;
	//print end_scope
	end_of_scope();
	symbol_table_manager.pop_scope();
	
}

Statement* call_id_explist(Id* id_caller,ParamVec* param_vec){
	if (symbol_table_manager.id_defined(id_caller->name)){
		id_caller = symbol_table_manager.get_id_by_name(id_caller->name);
		if(!(id_caller->is_func())){
			//error
			errorUndefFunc(yylineno,id_caller->name);
			exit(1);
		}
		if(!(id_caller->param_vec->params_validation(*param_vec))){

			vector<string> types;
			// get types from id_caller params.
			for (Id* id_param : id_caller->param_vec->func_params){
				types.insert(types.begin(),id_param->type);
			}

			errorPrototypeMismatch(yylineno,id_caller->name,types);
			exit(1);
		}
		code_gen.emit_call_id_explist(id_caller,param_vec);
		//delete param vec ids
		//and delete param vec
		// for(Id* id_param: param_vec->func_params ){
			// delete(id_param);
		// }
		// delete(param_vec);
	}

	Statement* caller_statement= new Statement(id_caller->name, id_caller->type, id_caller->param_vec, id_caller->place);
	caller_statement->next_list = id_caller->next_list; //updated in emit_call_id_explist()
	caller_statement->true_list = id_caller->true_list; 			//also
	caller_statement->false_list = id_caller->false_list;			//also
	return caller_statement;
}

Id* call_id(Id* id){
	Id* id_func = symbol_table_manager.get_id_by_name(id->name);
	if (symbol_table_manager.id_defined(id->name) &&  ( id_func->is_func()) && (( id_func->param_vec->func_params.size())==0)){
		//ok
	}else {
		
		//error
		errorUndefFunc(yylineno,id->name);
		exit(1);
	}
	//not relevant for hw5 anymore
	id->type = id_func->type;
	//HW5
	code_gen.emit_call_id(id);
	// next_list, true_list, nest_list is handled in emit_call_id().	
	return id;
}

Binop* exp_binop_exp(Id* exp1, Binop* op, Id* exp2){
	
	op->type = "BYTE";
	if ((exp1->type == "INT" || exp1->type== "BYTE" )&&(exp2->type == "INT" || exp2->type== "BYTE" )){
		//good case
		if (exp1->type == "INT" || exp2->type== "INT") {
			op->type = "INT";	
		}
	}
	// if ( exp1->type =="BYTE" && exp2->type == "BYTE"){
	// 	//make sure op->type is updated to int if necessery  
	// 	op->type="BYTE";
	// } 
	else {
		//error
		errorMismatch(yylineno);
		exit(1);
	}
	//exp1 and exp2 allready evaluated INT, INT
	//string reg1 = load_exp_value(exp1);
	//string reg2 = load_exp_value(exp2);
	op->exp1 = exp1;
	op->exp2 = exp2;
	code_gen.emit_exp_binop_exp(op);
	//eval binop operation
	//op=exp_eval(op)
	return op;
}

Binop* exp_priobinop_exp(Id* exp1, Binop* op, Id* exp2){
	
	op->type = "BYTE";
	if ((exp1->type == "INT" || exp1->type== "BYTE" )&&(exp2->type == "INT" || exp2->type== "BYTE" )){
		//good case
		if (exp1->type == "INT" || exp2->type== "INT") {
			op->type = "INT";	
		}
	} 
	else {
		//error
		errorMismatch(yylineno);
		exit(1);
	}
	op->exp1 = (Id*)exp1;
	op->exp2 = (Id*)exp2;
	code_gen.emit_exp_binop_exp(op);
	//eval binop operation
	//op=exp_eval(op)
	return op;	
}

Relop* exp_eqrelop_exp(Id* exp1, Relop* op, Id* exp2){
	return exp_relop_exp(exp1,op,exp2);
}

Relop* exp_relop_exp(Id* exp1, Relop* op, Id* exp2){
	if ((exp1->type == "INT" || exp1->type== "BYTE" )&&(exp2->type == "INT" || exp2->type== "BYTE" )){
		//good case
	} else {
		//error
		errorMismatch(yylineno);
		exit(1);
	}
	op->exp1 = (Id*)exp1;
	op->exp2 = (Id*)exp2;


	//HW5
	code_gen.emit_exp_relop_exp(op);

	return op;

}

LogicOp* exp_or_exp(Id* exp1, LogicOp* op,Id * marker_id, Id* exp2){
	if (!(exp1->type == exp2->type && exp2->type == "BOOL")){
		//error
		errorMismatch(yylineno);
		exit(1);
	}
	op->exp1 = (Id*)exp1;
	op->exp2 = (Id*)exp2;
	op->type="BOOL";
	//eval binop operation
	//op=exp_eval(op)
	code_gen.emit_exp_or_exp(exp1,op,marker_id,exp2);
	return op;	
}

LogicOp* exp_and_exp(Id* exp1, LogicOp* op,Id* marker_id ,Id* exp2){
	if (!(exp1->type == exp2->type && exp2->type == "BOOL")){
		//error
		errorMismatch(yylineno);
		exit(1);
	}
	op->exp1 = (Id*)exp1;
	op->exp2 = (Id*)exp2;

	//BUFFER.bpatch(op->exp1->true_list,op->exp2->name);

	//eval binop operation
	//op=exp_eval(op)
	code_gen.emit_exp_and_exp(exp1,op,marker_id,exp2);
	return op;	
}


//HW 5 EMIT FUNCTIONS:::::::::::::::
//HW 5 EMIT FUNCTIONS:::::::::::::::
//HW 5 EMIT FUNCTIONS:::::::::::::::
//HW 5 EMIT FUNCTIONS:::::::::::::::
void emit_function_params(Id* ret_type,Id* func_id,ParamVec* param_vec){
		/* TODO: emit Function code: */
	string params_format = "";
	string num_of_params = to_string(param_vec->func_params.size());
	//maybe reverse order? (shay)
	for (Id* param_name : param_vec->func_params){
		// params_format += "i32 %" + param_name->name + ", ";
		params_format += "i32 ,";
	}
	//delete excess collon
	if (params_format.size()>0){
		params_format = params_format.substr(0,params_format.size()-1);
	}

	BUFFER.emit("");
	BUFFER.emit("define "+ llvm_type(ret_type) + " @" + func_id->name + "(" + params_format + ") {");
	BUFFER.emit("entry:");
	BUFFER.emit("%argsFrame = alloca i32, i32 "+to_string(50+param_vec->func_params.size()));
	BUFFER.emit("%fp = getelementptr i32, i32* %argsFrame, i32 "+to_string(param_vec->func_params.size()));

	for(int offset=1; offset <= param_vec->func_params.size(); offset++){
	//check this later (according to ref)	
	// for(int offset=0; offset < param_vec->func_params.size(); offset++){
		string var = code_gen.freshVar();
		//BUFFER.emit(var + " = getelementptr ["+num_of_params+" x i32], ["+num_of_params+" x i32]* %args, i32 0 , i32 " + to_string(-(offset-1)));
		BUFFER.emit(var + " = getelementptr i32, i32* %fp, i32 "+ to_string(-(offset)));
		BUFFER.emit("store i32 %"+to_string(offset-1)+" , i32* "+var);
	}
	BUFFER.emit("");
}

void emit_function_id(Id* ret_type, Id* func_id,Statement* statement){
	//BPatch the last statment:
	int line = BUFFER.emit("br label @");
	string label = BUFFER.genLabel();
	BUFFER.bpatch(BUFFER.makelist({line,FIRST}),label);
	BUFFER.bpatch(statement->next_list,label); //st.end label	
	if(ret_type->type == "VOID") BUFFER.emit("ret void");
	else BUFFER.emit("ret i32 0"); //default return 0
	BUFFER.emit("}");
	// endScope();
}


// Id* exp_eval(Id* exp){
// 	return exp;
// }
string type_to_lower(Id* id){
	string res = "";
	for (char c : id->type){
		res+= std::tolower(c);
	}
	return res;
	//return transform(id->type.begin(), id->type.end(), id->type.begin(), tolower);
}
string llvm_type(Id* id){
	if(id->type == "INT" || id->type =="BYTE" || id->type == "BOOL") return "i32";
	//if(id->type == "BYTE") return "i8";
	//if(id->type == "BOOL") return "i1";
	if(id->type == "VOID") return "void";
}

// void initVar(string var,EntrySymbolTable* var_entry){
// 	//num_of_args != -1 when offset<0
// 	//that means that this entry is a parma of a func
// 	//and num_of_args will be greater then 0
// 	int num_of_args = var_entry->num_of_params_in_func;
//     int offset = var_entry->offset;
//     if(offset >= 0) BUFFER.emit(var+" = getelementptr [50 x i32], [50 x i32]* %fp, i32 0 , i32 "+to_string(offset));
//     else BUFFER.emit(var+" = getelementptr ["+to_string(num_of_args)+" x i32], ["+to_string(num_of_args)+" x i32]* %args, i32 0 , i32 "+to_string((offset+1)*(-1)));
// }

void SymbolTableManager::validate_return_type(string type, int line){
	if(type == "VOID") type = "TYPE_MISMATCH";
    if(type == "EMPTY")type = "VOID";
    // print_stack_variables_and_scopes();
    //skip global scope
    // auto func_table = (symbol_table_stack.begin())++;
    //get the cyrrent function
    SymbolTable* func_table = symbol_table_stack[1];
    if(func_table->func_ret_type != type){
        if(func_table->func_ret_type == "INT" && type == "BYTE")
        { /* Do nothing, it's conversion. */ }
        else
        {
            errorMismatch(line);
			exit(0); 
        }
    }
}