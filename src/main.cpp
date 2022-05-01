#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <koopa.h>
#include "myast.h"
#include "myparser.h"

using namespace std;

// 声明 lexer 的输入, 以及 parser 函数
// 为什么不引用 sysy.tab.hpp 呢? 因为首先里面没有 yyin 的定义
// 其次, 因为这个文件不是我们自己写的, 而是被 Bison 生成出来的
// 你的代码编辑器/IDE 很可能找不到这个文件, 然后会给你报错 (虽然编译不会出错)
// 看起来会很烦人, 于是干脆采用这种看起来 dirty 但实际很有效的手段
extern FILE *yyin;
extern int yyparse(std::unique_ptr<BaseAST> &ast);

char koopa_str[5000];

int main(int argc, const char *argv[]) {
  // 解析命令行参数. 测试脚本/评测平台要求你的编译器能接收如下参数:
  // compiler 模式 输入文件 -o 输出文件
  assert(argc == 5);
  auto mode = argv[1];
  auto input = argv[2];
  auto output = argv[4];
  cout<<mode<<endl;

  // 打开输入文件, 并且指定 lexer 在解析的时候读取这个文件
  yyin = fopen(input, "r");
  assert(yyin);

  // 调用 parser 函数, parser 函数会进一步调用 lexer 解析输入文件的

  unique_ptr<BaseAST> ast;
  auto ret = yyparse(ast);
  assert(!ret);
  FILE * yyout ;
  yyout = fopen(output, "w");
  cout << 1 << endl;
  ast->Dump(yyout, koopa_str);
  cout << 2 << endl;
  assert(!ret);
  cout << endl;

  if(strcmp(mode, "-koopa") == 0){
    fprintf(yyout,"%s",koopa_str);
  }

  // dump AST
  if(strcmp(mode, "-riscv") == 0){
    deal_with_IR(koopa_str);
    fprintf(yyout,"%s",asm_out);
  }
  fclose(yyout);

  return 0;
}