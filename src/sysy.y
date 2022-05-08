%code requires {
  #include <memory>
  #include <string>
  #include "myast.h"
}


%{

#include <iostream>
#include <memory>
#include <string>
#include "myast.h"

// 声明 lexer 函数和错误处理函数
int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);
using namespace std;

%}

// 定义 parser 函数和错误处理函数的附加参数
// 我们需要返回一个字符串作为 AST, 所以我们把附加参数定义成字符串的智能指针
// 解析完成后, 我们要手动修改这个参数, 把它设置成解析得到的字符串
%parse-param { std::unique_ptr<BaseAST> &ast }

// yylval 的定义, 我们把它定义成了一个联合体 (union)
// 因为 token 的值有的是字符串指针, 有的是整数
// 之前我们在 lexer 中用到的 str_val 和 int_val 就是在这里被定义的
// 至于为什么要用字符串指针而不直接用 string 或者 unique_ptr<string>?
// 请自行 STFW 在 union 里写一个带析构函数的类会出现什么情况
%union {
  std::string *str_val;
  int int_val;
  BaseAST *ast_val;
}

// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val
%token INT RETURN CONST IF ELSE WHILE BREAK CONTINUE VOID
%token <str_val> IDENT
%token <int_val> INT_CONST

// 非终结符的类型定义
%type <ast_val> FuncFParams FuncRParams CompUnitB FuncDef Block Stmt Number Exp LOrExp LAndExp EqExp RelExp AddExp MulExp UnaryExp PrimaryExp BlockItem Decl ConstExpB ExpB ConstDecl ExpA ConstExpA ConstDef ConstInitVal VarDecl VarDef InitVal ConstExp

%%

// 开始符, CompUnit ::= FuncDef, 大括号后声明了解析完成后 parser 要做的事情
// 之前我们定义了 FuncDef 会返回一个 str_val, 也就是字符串指针
// 而 parser 一旦解析完 CompUnit, 就说明所有的 token 都被解析了, 即解析结束了
// 此时我们应该把 FuncDef 返回的结果收集起来, 作为 AST 传给调用 parser 的函数
// $1 指代规则里第一个符号的返回值, 也就是 FuncDef 的返回值
CompUnit
  : CompUnitB{
    auto program = make_unique<CompUnitAST>();
    program -> compunit = unique_ptr<BaseAST>($1);
    ast = move(program);
  }

CompUnitB
  : CompUnitB FuncDef {
    auto comp_unit = new CompUnitBAST();
    comp_unit-> compunit = unique_ptr<BaseAST>($1);
    comp_unit -> func_def = unique_ptr<BaseAST>($2);
    comp_unit->type = 1;
    $$ = comp_unit;
  }
  ;

CompUnitB
  : FuncDef {
    auto comp_unit = new CompUnitBAST();
    comp_unit->func_def = unique_ptr<BaseAST>($1);
    comp_unit->type = 3;
    $$ = comp_unit;
  }
  ;

CompUnitB
  : CompUnitB Decl {
    auto comp_unit = new CompUnitBAST();
    comp_unit-> compunit = unique_ptr<BaseAST>($1);
    comp_unit -> decl = unique_ptr<BaseAST>($2);
    comp_unit->type = 2;
    $$ = comp_unit;
  }
  ;

CompUnitB
  : Decl {
    auto comp_unit = new CompUnitBAST();
    comp_unit->decl = unique_ptr<BaseAST>($1);
    comp_unit->type = 4;
    $$ = comp_unit;
  }
  ;

// FuncDef ::= FuncType IDENT '(' ')' Block;
// 我们这里可以直接写 '(' 和 ')', 因为之前在 lexer 里已经处理了单个字符的情况
// 解析完成后, 把这些符号的结果收集起来, 然后拼成一个新的字符串, 作为结果返回
// $$ 表示非终结符的返回值, 我们可以通过给这个符号赋值的方法来返回结果
// 你可能会问, FuncType, IDENT 之类的结果已经是字符串指针了
// 为什么还要用 unique_ptr 接住它们, 然后再解引用, 把它们拼成另一个字符串指针呢
// 因为所有的字符串指针都是我们 new 出来的, new 出来的内存一定要 delete
// 否则会发生内存泄漏, 而 unique_ptr 这种智能指针可以自动帮我们 delete
// 虽然此处你看不出用 unique_ptr 和手动 delete 的区别, 但当我们定义了 AST 之后
// 这种写法会省下很多内存管理的负担
FuncDef
  : INT IDENT '(' ')' Block {
    auto ast = new FuncDefAST();
    ast->ident = *unique_ptr<string>($2);
    ast->block = unique_ptr<BaseAST>($5);
    ast->type = 1;
    $$ = ast;
  }
  ;

FuncDef
  : INT IDENT '(' FuncFParams ')' Block {
    auto ast = new FuncDefAST();
    ast->ident = *unique_ptr<string>($2);
    ast->fparams = unique_ptr<BaseAST>($4);
    ast->block = unique_ptr<BaseAST>($6);
    ast->type = 2;
    $$ = ast;
  }
  ;

FuncDef
  : VOID IDENT '(' ')' Block {
    auto ast = new FuncDefAST();
    ast->ident = *unique_ptr<string>($2);
    ast->block = unique_ptr<BaseAST>($5);
    ast->type = 3;
    $$ = ast;
  }
  ;

FuncDef
  : VOID IDENT '(' FuncFParams ')' Block {
    auto ast = new FuncDefAST();
    ast->ident = *unique_ptr<string>($2);
    ast->fparams = unique_ptr<BaseAST>($4);
    ast->block = unique_ptr<BaseAST>($6);
    ast->type = 4;
    $$ = ast;
  }
  ;

FuncFParams
  : INT IDENT ',' FuncFParams {
    auto funcfparams = new FuncFParamsAST();
    funcfparams -> type = 1;
    funcfparams -> fparams = unique_ptr<BaseAST>($4);
    funcfparams -> ident = *unique_ptr<string>($2);
    $$ = funcfparams;
  }
  ;

FuncFParams
  : INT IDENT {
    auto funcfparams = new FuncFParamsAST();
    funcfparams -> type = 2;
    funcfparams -> ident = *unique_ptr<string>($2);
    $$ = funcfparams;
  }
  ;

FuncFParams
  : INT IDENT '[' ']' ConstExpA ',' FuncFParams {
    auto funcfparams = new FuncFParamsAST();
    funcfparams -> type = 3;
    funcfparams -> fparams = unique_ptr<BaseAST>($7);
    funcfparams -> constexpa = unique_ptr<BaseAST>($5);
    funcfparams -> ident = *unique_ptr<string>($2);
    $$ = funcfparams;
  }
  ;

FuncFParams
  : INT IDENT '[' ']' ConstExpA {
    auto funcfparams = new FuncFParamsAST();
    funcfparams -> type = 4;
    funcfparams -> ident = *unique_ptr<string>($2);
    funcfparams -> constexpa = unique_ptr<BaseAST>($5);
    $$ = funcfparams;
  }
  ;


FuncFParams
  : INT IDENT '[' ']' ',' FuncFParams {
    auto funcfparams = new FuncFParamsAST();
    funcfparams -> type = 5;
    funcfparams -> fparams = unique_ptr<BaseAST>($6);
    funcfparams -> ident = *unique_ptr<string>($2);
    $$ = funcfparams;
  }
  ;

FuncFParams
  : INT IDENT '[' ']'  {
    auto funcfparams = new FuncFParamsAST();
    funcfparams -> type = 6;
    funcfparams -> ident = *unique_ptr<string>($2);
    $$ = funcfparams;
  }
  ;

Block
  : '{' BlockItem '}' {
    auto block = new BlockAST();
    block -> type = 1;
    block->blockitem = unique_ptr<BaseAST>($2);
    $$ = block;
  }
  ;

Block
  : '{'  '}' {
    auto block = new BlockAST();
    block -> type = 2;
    $$ = block;
  }
  ;

BlockItem
  : Decl {
    auto blockitem = new BlockItemAST();
    blockitem -> type = 3;
    blockitem -> decl = unique_ptr<BaseAST>($1);
    $$ = blockitem;
  }
  ;

BlockItem
  : Stmt {
    auto blockitem = new BlockItemAST();
    blockitem -> type = 4;
    blockitem -> stmt = unique_ptr<BaseAST>($1);
    $$ = blockitem;

  }
  ;

BlockItem
  : BlockItem Stmt {
    auto blockitem = new BlockItemAST();
    blockitem -> type = 1;
    blockitem -> stmt = unique_ptr<BaseAST>($2);
    blockitem -> blockitem = unique_ptr<BaseAST>($1);
    $$ = blockitem;
  }
  ;

BlockItem
  : BlockItem Decl {
    auto blockitem = new BlockItemAST();
    blockitem -> type = 2;
    blockitem -> decl = unique_ptr<BaseAST>($2);
    blockitem -> blockitem = unique_ptr<BaseAST>($1);
    $$ = blockitem;
  }
  ;

Decl
  : ConstDecl{
    auto decl = new DeclAST();
    decl -> constdecl = unique_ptr<BaseAST>($1);
    decl -> type = 1;
    $$=decl;
  }
  ;

Decl
  : VarDecl{
    auto decl = new DeclAST();
    decl -> vardecl = unique_ptr<BaseAST>($1);
    decl -> type = 2;
    $$=decl;
  }
  ;

ConstDecl
  : CONST INT ConstDef ';'{
    auto constdecl = new ConstDeclAST();
    constdecl -> constdef = unique_ptr<BaseAST>($3);
    $$ = constdecl;
  }
  ;

ConstDef
  : IDENT '=' ConstInitVal {
    auto constdef = new ConstDefAST();
    constdef -> lval = *unique_ptr<string>($1);
    constdef -> constinitval = unique_ptr<BaseAST>($3);
    constdef -> type = 1;
    $$ = constdef;
  }
  ;

ConstDef
  : ConstDef ',' IDENT '=' ConstInitVal {
    auto constdef = new ConstDefAST();
    constdef -> lval = *unique_ptr<string>($3);
    constdef -> constinitval = unique_ptr<BaseAST>($5);
    constdef -> constdef = unique_ptr<BaseAST>($1);
    constdef -> type = 2;
    $$ = constdef;

  }
  ;

ConstDef
  : IDENT ConstExpA '=' ConstInitVal {
    auto constdef = new ConstDefAST();
    constdef -> lval = *unique_ptr<string>($1);
    constdef -> constexpa = unique_ptr<BaseAST>($2);
    constdef -> constinitval = unique_ptr<BaseAST>($4);
    constdef -> type = 3;
    $$ = constdef;

  }
  ;

ConstDef
  : ConstDef ',' IDENT ConstExpA '=' ConstInitVal {
    auto constdef = new ConstDefAST();
    constdef -> constdef = unique_ptr<BaseAST>($1);
    constdef -> lval = *unique_ptr<string>($3);
    constdef -> constexpa = unique_ptr<BaseAST>($4);
    constdef -> constinitval = unique_ptr<BaseAST>($6);
    constdef -> type = 4;
    $$ = constdef;
  }
  ;

ConstInitVal
  : ConstExp{
    auto constinitval = new ConstInitValAST();
    constinitval -> constexp =  unique_ptr<BaseAST>($1);
    constinitval -> type = 1;
    $$ = constinitval;
  } 
  ;

ConstInitVal
  : '{' ConstExpB '}'{
    auto constinitval = new ConstInitValAST();
    constinitval -> constexpb =  unique_ptr<BaseAST>($2);
    constinitval->type = 2;
    $$ = constinitval;
  } 
  ;

ConstInitVal
  : '{' '}'{
    auto constinitval = new ConstInitValAST();
    constinitval->type = 3;
    $$ = constinitval;
  } 
  ;

ConstExpB
  : ConstInitVal ',' ConstExpB {
    auto constexp = new ConstExpBAST();
    constexp -> constexpb = unique_ptr<BaseAST>($3);
    constexp -> constinitval = unique_ptr<BaseAST>($1);
    constexp -> type = 1;
    $$ = constexp;
  }
  ;

ConstExpB
  : ConstInitVal{
    auto constexp = new ConstExpBAST();
    constexp -> constinitval = unique_ptr<BaseAST>($1);
    constexp -> type = 2;
    $$ = constexp;
  }
  ;

ConstExp
  : Exp{
    auto constexp = new ConstExpAST();
    constexp -> constexp = unique_ptr<BaseAST>($1);
    $$ = constexp;
  }
  ;

VarDecl
  : INT VarDef ';'{
    auto vardecl = new VarDeclAST();
    vardecl -> vardef = unique_ptr<BaseAST>($2);
    $$ = vardecl;
  }
  ;

VarDef
  : VarDef ',' IDENT{
    auto vardef = new VarDefAST();
    vardef -> vardef = unique_ptr<BaseAST>($1);
    vardef -> type = 1;
    vardef -> lval = *unique_ptr<string>($3);
    $$ = vardef;
  }
  ;

VarDef
  : VarDef ',' IDENT '=' InitVal{
    auto vardef = new VarDefAST();
    vardef -> vardef = unique_ptr<BaseAST>($1);
    vardef -> type = 2;
    vardef -> lval = *unique_ptr<string>($3);
    vardef -> initval = unique_ptr<BaseAST>($5);
    $$ = vardef;
  }
  ;

VarDef
  : IDENT{
    auto vardef = new VarDefAST();
    vardef -> type = 3;
    vardef -> lval = *unique_ptr<string>($1);
    $$ = vardef;

  }
  ;

VarDef
  : IDENT '=' InitVal{
    auto vardef = new VarDefAST();
    vardef -> type = 4;
    vardef -> lval = *unique_ptr<string>($1);
    vardef -> initval = unique_ptr<BaseAST>($3);
    $$ = vardef;
  }
  ;

VarDef
  : IDENT ConstExpA '=' InitVal{
    auto vardef = new VarDefAST();
    vardef -> type = 5;
    vardef -> lval = *unique_ptr<string>($1);
    vardef -> initval = unique_ptr<BaseAST>($4);
    vardef -> constexpa = unique_ptr<BaseAST>($2);
    $$ = vardef;
  }
  ;

VarDef
  : IDENT ConstExpA {
    auto vardef = new VarDefAST();
    vardef -> type = 6;
    vardef -> lval = *unique_ptr<string>($1);
    vardef -> constexpa = unique_ptr<BaseAST>($2);
    $$ = vardef;
  }
  ;

VarDef
  : VarDef ',' IDENT ConstExpA '=' InitVal{
    auto vardef = new VarDefAST();
    vardef -> vardef = unique_ptr<BaseAST>($1);
    vardef -> type = 7;
    vardef -> lval = *unique_ptr<string>($3);
    vardef -> constexpa = unique_ptr<BaseAST>($4);
    vardef -> initval = unique_ptr<BaseAST>($6);
    $$ = vardef;
  }
  ;

VarDef
  : VarDef ',' IDENT ConstExpA { 
    auto vardef = new VarDefAST();
    vardef -> vardef = unique_ptr<BaseAST>($1);
    vardef -> type = 8;
    vardef -> lval = *unique_ptr<string>($3);
    vardef -> constexpa = unique_ptr<BaseAST>($4);
    $$ = vardef;
  }
  ;

ConstExpA
  : ConstExpA '[' ConstExp ']' { 
    auto constexpa = new ConstExpAAST();
    constexpa -> constexpa = unique_ptr<BaseAST>($1);
    constexpa -> type = 1;
    constexpa -> constexp = unique_ptr<BaseAST>($3);
    $$ = constexpa;
  }
  ;

ConstExpA
  : '[' ConstExp ']' { 
    auto constexpa = new ConstExpAAST();
    constexpa -> constexp = unique_ptr<BaseAST>($2);
    constexpa-> type = 2;
    $$ = constexpa;
  }
  ;

InitVal
  : Exp{
    auto initval = new InitValAST();
    initval -> exp = unique_ptr<BaseAST>($1);
    initval -> type = 1;
    $$ = initval;
  }
  ;

InitVal
  : '{' ExpB '}'{
    auto initval = new InitValAST();
    initval -> expb = unique_ptr<BaseAST>($2);
    initval -> type = 2;
    $$ = initval;
  }
  ;

InitVal
  : '{' '}'{
    auto initval = new InitValAST();
    initval -> type = 3;
    $$ = initval;
  }
  ;

ExpB
  : InitVal ',' ExpB {
    auto expb = new ExpBAST();
    expb -> expb = unique_ptr<BaseAST>($3);
    expb -> initval = unique_ptr<BaseAST>($1);
    expb -> type = 1;
    $$ = expb;
  }

ExpB
  : InitVal{
    auto expb = new ExpBAST();
    expb -> initval = unique_ptr<BaseAST>($1);
    expb -> type = 2;
    $$ = expb;
  }

Stmt
  : RETURN Exp ';' {
    auto stmt = new StmtAST();
    stmt -> exp = unique_ptr<BaseAST>($2);
    stmt -> type = 1;
    $$ = stmt;
  }
  ;

Stmt
  : IDENT '=' Exp ';' {
    auto stmt = new StmtAST();
    stmt -> exp = unique_ptr<BaseAST>($3);
    stmt -> lval = *unique_ptr<string>($1);
    stmt -> type = 2;
    $$ = stmt;
  }
  ;

Stmt
  : RETURN ';' {
    auto stmt = new StmtAST();
    stmt -> type = 3;
    $$ = stmt;
  }
  ;

Stmt
  : Exp ';' {
    auto stmt = new StmtAST();
    stmt -> exp = unique_ptr<BaseAST>($1);
    stmt -> type = 4;
    $$ = stmt;
  }
  ;

Stmt
  : ';' {
    auto stmt = new StmtAST();
    stmt -> type = 5;
    $$ = stmt;
  }
  ;

Stmt
  : Block {
    auto stmt = new StmtAST();
    stmt -> block = unique_ptr<BaseAST>($1);
    stmt -> type = 6;
    $$ = stmt;
  }
  ;

Stmt
  : IF '(' Exp ')' Stmt {
    auto stmt = new StmtAST();
    stmt -> exp = unique_ptr<BaseAST>($3);
    stmt -> ifstmt = unique_ptr<BaseAST>($5);
    stmt -> type = 7;
    $$ = stmt;
  }
  ;

Stmt
  : IF '(' Exp ')' Stmt ELSE Stmt {
    auto stmt = new StmtAST();
    stmt -> exp= unique_ptr<BaseAST>($3);
    stmt -> ifstmt = unique_ptr<BaseAST>($5);
    stmt -> elsestmt = unique_ptr<BaseAST>($7);
    stmt -> type = 8;
    $$ = stmt;
  }
  ;

Stmt
  : WHILE '(' Exp ')' Stmt{
    auto stmt = new StmtAST();
    stmt -> exp= unique_ptr<BaseAST>($3);
    stmt -> ifstmt = unique_ptr<BaseAST>($5);
    stmt -> type = 9;
    $$ = stmt;
  }
  ;

Stmt
  : BREAK ';'{
    auto stmt = new StmtAST();
    stmt -> type = 10;
    $$ = stmt;
  }
  ;

Stmt
  : CONTINUE ';'{
    auto stmt = new StmtAST();
    stmt -> type = 11;
    $$ = stmt;
  }
  ;

Stmt
  : IDENT ExpA '=' Exp ';' {
    auto stmt = new StmtAST();
    stmt -> lval = *unique_ptr<string>($1);
    stmt -> expa = unique_ptr<BaseAST>($2);
    stmt -> exp = unique_ptr<BaseAST>($4);
    stmt -> type = 12;
    $$ = stmt;
  }
  ;

ExpA
  : ExpA '[' Exp ']'{
    auto expa = new ExpAAST();
    expa->expa = unique_ptr<BaseAST>($1);
    expa->exp = unique_ptr<BaseAST>($3);
    expa->type = 1;
    $$ = expa;
  }

ExpA
  : '[' Exp ']'{
    auto expa = new ExpAAST();
    expa->exp = unique_ptr<BaseAST>($2);
    expa->type = 2;
    $$ = expa;
  }

Exp
  : LOrExp {
    auto exp = new ExpAST();
    exp -> lorexp = unique_ptr<BaseAST>($1);
    $$ = exp;
  }
  ;

LOrExp
  : LAndExp{
    auto lorexp = new LOrExpAST();
    lorexp -> landexp = unique_ptr<BaseAST>($1);
    lorexp -> type = 1;
    $$  = lorexp;
  }
  ;

LOrExp
  : LOrExp '|' '|' LAndExp{
    auto lorexp = new LOrExpAST();
    lorexp -> lorexp = unique_ptr<BaseAST>($1);
    lorexp -> type = 2;
    lorexp -> landexp = unique_ptr<BaseAST>($4);
    $$ = lorexp;
  }
  ;

LAndExp
  : EqExp{
    auto landexp = new LAndExpAST();
    landexp -> eqexp = unique_ptr<BaseAST>($1);
    landexp -> type = 1;
    $$ = landexp;
  }
  ;
  
LAndExp
  : LAndExp '&' '&' EqExp{
    auto landexp = new LAndExpAST();
    landexp -> landexp = unique_ptr<BaseAST>($1);
    landexp -> type = 2;
    landexp -> eqexp = unique_ptr<BaseAST>($4);
    $$ = landexp ;
  }
  ;

EqExp
  : RelExp{
    auto eqexp = new EqExpAST();
    eqexp -> relexp = unique_ptr<BaseAST>($1);
    eqexp -> type = 1;
    $$ = eqexp;

  }
  ;

EqExp
  : EqExp '=' '=' RelExp {
    auto eqexp = new EqExpAST();
    eqexp -> eqexp = unique_ptr<BaseAST>($1);
    eqexp -> type = 2;
    eqexp -> relexp = unique_ptr<BaseAST>($4);
    $$ = eqexp;
  }
  ;
EqExp
  : EqExp '!' '=' RelExp{
    auto eqexp = new EqExpAST();
    eqexp -> eqexp = unique_ptr<BaseAST>($1);
    eqexp -> type = 3;
    eqexp -> relexp = unique_ptr<BaseAST>($4);
    $$ = eqexp;
  }
  ;

RelExp
  : AddExp {
    auto relexp = new RelExpAST();
    relexp -> addexp = unique_ptr<BaseAST>($1);
    relexp -> type = 1;
    $$ = relexp;
  }
  ;

RelExp
  : RelExp '<' AddExp{
    auto relexp = new RelExpAST();
    relexp -> relexp = unique_ptr<BaseAST>($1);
    relexp -> type = 2;
    relexp -> addexp = unique_ptr<BaseAST>($3);
    $$ = relexp;
  }
  ;

RelExp
  : RelExp '>' AddExp{
    auto relexp = new RelExpAST();
    relexp -> relexp = unique_ptr<BaseAST>($1);
    relexp -> type = 3;
    relexp -> addexp = unique_ptr<BaseAST>($3);
    $$ = relexp;
  }
  ;

RelExp
  : RelExp '<' '=' AddExp{
    auto relexp = new RelExpAST();
    relexp -> relexp = unique_ptr<BaseAST>($1);
    relexp -> type = 4;
    relexp -> addexp = unique_ptr<BaseAST>($4);
    $$ = relexp;
  }
  ;

RelExp
  : RelExp '>' '=' AddExp{
    auto relexp = new RelExpAST();
    relexp -> relexp = unique_ptr<BaseAST>($1);
    relexp -> type = 5;
    relexp -> addexp = unique_ptr<BaseAST>($4);
    $$ = relexp;
  }
  ;

AddExp 
  : MulExp {
    auto addexp = new AddExpAST();
    addexp -> mulexp = unique_ptr<BaseAST>($1);
    addexp -> type = 1;
    $$ = addexp;
  }
  ;

AddExp 
  : AddExp '+' MulExp{
    auto addexp = new AddExpAST();
    addexp -> addexp = unique_ptr<BaseAST>($1);
    addexp -> type = 2;
    addexp -> mulexp = unique_ptr<BaseAST>($3);
    $$ = addexp;
  }
  ;

AddExp 
  : AddExp '-' MulExp{
    auto addexp = new AddExpAST();
    addexp -> addexp = unique_ptr<BaseAST>($1);
    addexp -> type = 3;
    addexp -> mulexp = unique_ptr<BaseAST>($3);
    $$ = addexp;
  }
  ;

MulExp
  : UnaryExp {
    auto mulexp = new MulExpAST();
    mulexp -> unaryexp = unique_ptr<BaseAST>($1);
    mulexp -> type = 1;
    $$ = mulexp;
  }
  ;

MulExp
  : MulExp '*' UnaryExp{
    auto mulexp = new MulExpAST();
    mulexp -> mulexp = unique_ptr<BaseAST>($1);
    mulexp -> type = 2;
    mulexp -> unaryexp = unique_ptr<BaseAST>($3);
    $$ = mulexp;
  }
  ;

MulExp
  : MulExp '/' UnaryExp{
    auto mulexp = new MulExpAST();
    mulexp -> mulexp = unique_ptr<BaseAST>($1);
    mulexp -> type = 3;
    mulexp -> unaryexp = unique_ptr<BaseAST>($3);
    $$ = mulexp;
  }
  ;

MulExp
  : MulExp '%' UnaryExp{
    auto mulexp = new MulExpAST();
    mulexp -> mulexp = unique_ptr<BaseAST>($1);
    mulexp -> type = 4;
    mulexp -> unaryexp = unique_ptr<BaseAST>($3);
    $$ = mulexp;
  }
  ;

UnaryExp
  : PrimaryExp {
    auto unaryexp = new UnaryExpAST();
    unaryexp -> primaryexp = unique_ptr<BaseAST>($1);
    unaryexp -> type = 1;
    $$ = unaryexp;
  }
  ;

UnaryExp
  :  '+' UnaryExp{
    auto unaryexp = new UnaryExpAST();
    unaryexp -> unaryexp = unique_ptr<BaseAST>($2);
    unaryexp -> type = 2;
    $$ = unaryexp;
  }
  ;

UnaryExp
  :  '-' UnaryExp{
    auto unaryexp = new UnaryExpAST();
    unaryexp -> unaryexp = unique_ptr<BaseAST>($2);
    unaryexp -> type = 3;
    $$ = unaryexp;
  }
  ;

UnaryExp
  : '!' UnaryExp{
    auto unaryexp = new UnaryExpAST();
    unaryexp -> unaryexp = unique_ptr<BaseAST>($2);
    unaryexp -> type = 4;
    $$ = unaryexp;
  }
  ;

UnaryExp
  : IDENT '(' ')' {
    auto unaryexp = new UnaryExpAST();
    unaryexp -> ident = *unique_ptr<string>($1);
    unaryexp -> type = 5;
    $$ = unaryexp;
  }
  ;

UnaryExp
  : IDENT '(' FuncRParams ')' {
    auto unaryexp = new UnaryExpAST();
    unaryexp -> ident = *unique_ptr<string>($1);
    unaryexp -> rparams = unique_ptr<BaseAST>($3);
    unaryexp -> type = 6;
    $$ = unaryexp;
  }
  ;

FuncRParams 
  : Exp ',' FuncRParams {
    auto funcrparams = new FuncRParamsAST();
    funcrparams -> type = 1;
    funcrparams -> exp = unique_ptr<BaseAST>($1);
    funcrparams -> rparams = unique_ptr<BaseAST>($3);
    $$ = funcrparams;
  }

FuncRParams 
  : Exp {
    auto funcrparams = new FuncRParamsAST();
    funcrparams -> type = 2;
    funcrparams -> exp = unique_ptr<BaseAST>($1);
    $$ = funcrparams;
  }

PrimaryExp
  : '(' Exp ')' {
    auto primaryexp = new PrimaryExpAST();
    primaryexp -> exp = unique_ptr<BaseAST>($2);
    primaryexp -> type = 1;
    $$ = primaryexp; 
  }
  ;

PrimaryExp
  : Number{
    auto primaryexp = new PrimaryExpAST();
    primaryexp -> number = unique_ptr<BaseAST>($1);
    primaryexp -> type = 2;
    $$ = primaryexp;
  }
  ;

PrimaryExp
  : IDENT{
    auto primaryexp = new PrimaryExpAST();
    primaryexp -> lval = *unique_ptr<string>($1);
    primaryexp -> type = 3;
    $$ = primaryexp;
  }
  ;

PrimaryExp
  : IDENT ConstExpA{
    auto primaryexp = new PrimaryExpAST();
    primaryexp -> lval = *unique_ptr<string>($1);
    primaryexp -> constexpa = unique_ptr<BaseAST>($2);
    primaryexp -> type = 4;
    $$ = primaryexp;
  }
  ;

Number
  : INT_CONST {
    auto number = new NumberAST();
    number->num = $1;
    $$ = number;
  }
  ;

%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s) {
  cerr << "error: " << s << endl;
}