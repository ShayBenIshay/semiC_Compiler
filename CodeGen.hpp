#ifndef HW3_CODEGEN_H
#define HW3_CODEGEN_H

#include <string>
#include <iostream>
#include "source.hpp"


using std::string;
using std::to_string;

// struct Id;
// struct Statement;

struct CodeGen{
    int counter;
    CodeGen():counter(0){}
    ~CodeGen()=default;
    int genNum(){return counter++;}
    string freshVar(){return "%var_"+to_string(genNum());}
    string gen_str(){return "@.str"+to_string(genNum());}
    
    //we may delete this function and use emit_id_asmt
    void emit_type_id_asmt( Id* type_node,EntrySymbolTable* id_entry,Id* exp,Statement* statement){
        if(exp->type == "BOOL"){
            backpatch_bool(exp);
         }
        string var = freshVar();
        BUFFER.emit(var + " = getelementptr i32, i32* %fp, i32 " + to_string(id_entry->offset));
        BUFFER.emit("store i32 " + exp->place + ", i32* " +  var);
    }

    // TYPE ID = EXP
    void emit_id_asmt(EntrySymbolTable* id_entry,Id* exp,Statement* statement){
        if(exp->type == "BOOL"){
            backpatch_bool(exp);
        }
        string var = freshVar();
        BUFFER.emit(var + " = getelementptr i32, i32* %fp, i32 " + to_string(id_entry->offset));
        BUFFER.emit("store i32 " + exp->place + ", i32* " +  var);
    }

    void emit_exp_and_exp(Id* exp1,LogicOp* op,Id* marker_id,Id* exp2){

        BUFFER.bpatch(exp1->true_list,marker_id->name);
        op->true_list =exp2->true_list;
        op->false_list = BUFFER.merge(exp1->false_list,exp2->false_list);
    }

    //EXP OR EXP 
    void emit_exp_or_exp(Id* exp1,LogicOp* op,Id* marker_id,Id* exp2) {
        BUFFER.bpatch(exp1->false_list,marker_id->name);
        op->false_list = exp2->false_list;
        op->true_list = BUFFER.merge(exp1->true_list,exp2->true_list);
    }

    void emit_exp_binop_exp(Binop* binop){
        int back_patch_line;
        string var = freshVar();
        string zero_div_var = freshVar();
        string div_err_label;
        string no_div_err_label;
        string res_type = "i32";
        string left_exp_place = binop->exp1->place;
        string right_exp_place = binop->exp2->place;
        //DEBUG DIFF CHECK LATER
        if (binop->name != "/" && binop->exp1->type == "BYTE" && binop->exp2->type == "BYTE"){
            left_exp_place = freshVar();
            BUFFER.emit(left_exp_place + " = trunc i32 " + binop->exp1->place + " to i8");
            right_exp_place = freshVar();
            BUFFER.emit(right_exp_place + " = trunc i32 " + binop->exp2->place + " to i8");
            res_type = "i8";
        }
        if(binop->name == "+") BUFFER.emit(var +" = add " +res_type+ " " +left_exp_place+ ", " +right_exp_place);
        if(binop->name == "-") BUFFER.emit(var +" = sub " +res_type+ " " +left_exp_place+ ", " +right_exp_place);
        if(binop->name == "*") BUFFER.emit(var +" = mul " +res_type+ " " +left_exp_place+ ", " +right_exp_place);
        if(binop->name == "/"){
            BUFFER.emit(zero_div_var +" = icmp eq i32 "+ binop->exp2->place+ ", 0");
            BUFFER.emit(";binop->exp1->place)"+binop->exp1->place);
            BUFFER.emit(";left_exp_place"+left_exp_place);
            BUFFER.emit(";"+binop->exp2->place);
            BUFFER.emit(";right_exp_place"+right_exp_place);

            back_patch_line = BUFFER.emit("br i1 "+ zero_div_var+ " , label @ , label @");
            div_err_label = BUFFER.genLabel();
            BUFFER.emit("call i32 (i8*, ...) @printf(i8* getelementptr([24 x i8], [24 x i8]* @.zero_div_err, i32 0, i32 0))");
            BUFFER.emit("call void @exit(i32 0)");
            int jump = BUFFER.emit("br label @");
            no_div_err_label = BUFFER.genLabel();
            BUFFER.emit(var+ " = sdiv "+ res_type+ " "+ left_exp_place+ ", "+ right_exp_place);
            BUFFER.bpatch(BUFFER.makelist({back_patch_line,SECOND}),no_div_err_label);
            BUFFER.bpatch(BUFFER.makelist({back_patch_line,FIRST}),div_err_label);
            BUFFER.bpatch(BUFFER.makelist({jump,FIRST}),no_div_err_label);
        }
        if(res_type == "i8"){
            string new_var = freshVar();
            BUFFER.emit(new_var+ " = zext i8 "+ var+ " to i32");
            var = new_var;
        }
        binop->place = var;
       

    }

    //Call ID
    void emit_call_id(Id* func_id){

        string res_type_str = type_to_lower(func_id);
        if (res_type_str != "void") res_type_str="i32";
        string res = "call "+res_type_str+" @"+func_id->name+"()";
        if ( res_type_str == "void"){
            BUFFER.emit(res);
            func_id->place = "N/A";// no return value for function. so expression is void type

        } else {
            string t_var1=freshVar();
            BUFFER.emit(t_var1+" = "+res);
            func_id->place = t_var1;

            if (func_id->type == "BOOL"){
                string t_var2= freshVar();
                BUFFER.emit(t_var2 +" = icmp ne i32 0, "+t_var1);
                int line=BUFFER.emit("br i1 "+t_var2+", label @ , label @");
                func_id->true_list=BUFFER.makelist({line,FIRST});
                func_id->false_list=BUFFER.makelist({line,SECOND});
              //odd line
                //delete this later
                func_id->next_list = BUFFER.merge(BUFFER.makelist({line,FIRST}),BUFFER.makelist({line,SECOND}));
            }
        } 
    }    

        // EXP RELOP EXP
    void emit_exp_relop_exp(Relop* op){

        int line;
        string var = freshVar();
        /* Greating Label */

        /* Do Operation and Condition */
        if(op->name == ">=") BUFFER.emit(var+" = icmp sge i32 " + op->exp1->place + ", "+ op->exp2->place);
        if(op->name == ">")  BUFFER.emit(var+" = icmp sgt i32 " + op->exp1->place + ", "+ op->exp2->place);
        if(op->name == "<=") BUFFER.emit(var+" = icmp sle i32 " + op->exp1->place + ", "+ op->exp2->place);
        if(op->name == "<")  BUFFER.emit(var+" = icmp slt i32 " + op->exp1->place + ", "+ op->exp2->place);
        if(op->name == "==") BUFFER.emit(var+" = icmp eq i32 "  + op->exp1->place + ", "+ op->exp2->place);
        if(op->name == "!=") BUFFER.emit(var+" = icmp ne i32 "  + op->exp1->place + ", "+ op->exp2->place);
        line = BUFFER.emit("br i1 "+var+", label @ , label @");

        op->true_list =  BUFFER.makelist({line,FIRST});
        op->false_list = BUFFER.makelist({line,SECOND});
        op->place = var;
    } 

    string bool_eval(Id* bool_exp){
        string var = freshVar();
        int line = BUFFER.emit("br label @");
        
        string true_label = BUFFER.genLabel();
        BUFFER.bpatch(BUFFER.makelist({line,FIRST}),true_label);
        int true_line = BUFFER.emit("br label @");
        BUFFER.bpatch(bool_exp->true_list,true_label);
        
        string false_label = BUFFER.genLabel();
        int false_line = BUFFER.emit("br label @");
        BUFFER.bpatch(bool_exp->false_list,false_label);

        string next_label = BUFFER.genLabel();
        BUFFER.bpatch(BUFFER.makelist({true_line,FIRST}),next_label);
        BUFFER.bpatch(BUFFER.makelist({false_line,SECOND}),next_label);
        BUFFER.emit(var +" = phi i32 [ 1, %"+true_label+" ] , [ 0, %"+ false_label+"]");

        return var;
    }

    void backpatch_bool(Id* exp){
         string true_label = BUFFER.genLabel();
        int true_line = BUFFER.emit("br label @");
        string false_label = BUFFER.genLabel();
        int false_line = BUFFER.emit("br label @");
        BUFFER.bpatch(exp->false_list, false_label);
        BUFFER.bpatch(exp->true_list, true_label);

        string next_label = BUFFER.genLabel();
        BUFFER.bpatch(BUFFER.makelist({false_line,FIRST}),next_label);
        BUFFER.bpatch(BUFFER.makelist({true_line,SECOND}),next_label);
        exp->place = freshVar();
        BUFFER.emit(exp->place+" = phi i32 [1,%"+true_label+"], [0,%"+false_label+"]");
    }

    void emit_call_id_explist(Id* id_caller,ParamVec* param_vec){
        if (id_caller->name == "print"){
            //Check later: need to assert func params size is 1?
            Id* str_param = param_vec->func_params[0];
            string str_size = to_string(str_param->name.length()+1);
            BUFFER.emit("call void @print(i8* getelementptr ([" +str_size+ " x i8], [" +str_size+ " x i8]* " +str_param->place+ ", i32 0, i32 0))");
        }else{
            //handle other fucntions:
            string ret_type_str = id_caller->type;
            string args_format = "";
            string expected_args_format = "";
            if (ret_type_str != "VOID"){
                ret_type_str = "i32";
            }else{
                ret_type_str = "void";
            }
            // for (Id* id_param : param_vec->func_params){
            //     expected_args_format += "i32 ,";
            //     args_format += "i32 " +id_param->place+ ",";
            // }
            for(int i = param_vec->func_params.size()-1; i>=0 ; i--){
                expected_args_format += "i32 ,";
                args_format += "i32 " +param_vec->func_params[i]->place+ ",";
            }
            args_format = args_format.substr(0,args_format.length()-1);
            expected_args_format = expected_args_format.substr(0,expected_args_format.length()-1);
            string call_command = "call "+ llvm_type(id_caller) + "("+expected_args_format+") @"+ id_caller->name+"("+args_format+")";
            
            if(ret_type_str == "void") {
                BUFFER.emit(call_command);
            }
            else{
                BUFFER.emit(";what is the return type:"+ret_type_str);
                string var = freshVar();
                BUFFER.emit(var+ " = " +call_command);
                // $$->_place = var;
                id_caller->place = var;
                if(id_caller->type == "BOOL"){ //id caller type is checked cause ret_type can only be i32
                    /* If bool, evaluate please. */
                    string tempv = freshVar();
                    /* Evaluate with phi here? */
                    BUFFER.emit(tempv+" = icmp ne i32 0, "+var);
                    int addt = BUFFER.emit("br i1 " +tempv+ ", label @ , label @");
                    id_caller->true_list = BUFFER.makelist({addt,FIRST});
                    id_caller->false_list = BUFFER.makelist({addt,SECOND});
      
      //odd line
                //delete this later
                              //In case of end-statement:
                    id_caller->next_list = BUFFER.merge((BUFFER.makelist({addt, FIRST})),(BUFFER.makelist({addt, SECOND})));
                }
            }
        }
    }
   
    
    
};




#endif