%{
    #include "hw3_output.hpp"
    #include "source.hpp"
    #include "parser.tab.hpp"

%}

%option yylineno
%option noyywrap

%%
void                              {yylval=new Node("VOID") ; return VOID;}
int                               {yylval=new Node("INT"); return INT ;}
byte                              {yylval=new Node("BYTE"); return BYTE ;}
b                                 { return B ;}
bool                              {yylval=new Node("BOOL");return BOOL ;}
and                               {yylval=new LogicOp();return AND ;}
or                                {yylval=new LogicOp();return OR ;}
not                               {yylval=new Not();return NOT ;}
true                              {yylval=new Id("BOOL_VALUE","BOOL",nullptr);return TRUE ;}
false                             {yylval=new Id("BOOL_VALUE","BOOL",nullptr);return FALSE ;}
return                            {yylval=new Statement("RETURN","VOID",nullptr,"undefined",false,-1,true);return RETURN ;}
if                                {return IF ;}
else                              {return ELSE ;}
while                             {return WHILE ;}
break                             {yylval = new Statement("BREAK","VOID",nullptr,"undefined",true,yylineno);return BREAK ;}
continue                          {yylval = new Statement("CONTINUE","VOID",nullptr,"undefined",true,yylineno);return CONTINUE ;}
";"                                {return SC ;}
","                                {return COMMA ;}
"("                                {return LPAREN ;}
")"                                {return RPAREN ;}
"{"                                {return LBRACE ;}
"}"                                {return RBRACE ;}
"="                                {return ASSIGN ;}
"<"|">"|"<="|">="                  {yylval=new Relop(nullptr,nullptr,yytext,yylineno);return RELOP ;}
"=="|"!="                          {yylval=new Relop(nullptr,nullptr,yytext,yylineno);return EQRELOP ;}
"+"|"-"                            {yylval=new Binop(nullptr,nullptr,yytext,yylineno);return BINOP ;}
"*"|"/"                            {yylval=new Binop(nullptr,nullptr,yytext,yylineno);return PRIOBINOP ;}
[a-zA-Z][a-zA-Z0-9]*               {yylval=new Id(yytext);return ID ;/*id->place = "undefined" (by default) */}
0|[1-9][0-9]*                      {yylval=new Id("INT_VALUE","INT",nullptr,yytext);return NUM ;}
\"([^\n\r\"\\]|\\[rnt"\\])+\"      {yylval=new Id("STRING_VALUE","STRING",nullptr,yytext);return STRING ;}
[\r\t\n ]|\/\/[^\r\n]*[\r|\n|\r\n]?         ;
.                                 { output::errorLex(yylineno); exit(0); }


%%
