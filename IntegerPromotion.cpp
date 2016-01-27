#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

// Declares clang::SyntaxOnlyAction.
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"

// Declares llvm::cl::extrahelp.
#include "llvm/Support/CommandLine.h"

using namespace clang::ast_matchers;
using namespace clang::tooling;
using namespace clang;
using namespace llvm;

StatementMatcher ComparisonMatcher =
    binaryOperator(
        anyOf(hasOperatorName("=="),
              hasOperatorName("!="),
              hasOperatorName(">"),
              hasOperatorName("<"),
              hasOperatorName(">="),
              hasOperatorName("<=")),
        anyOf(hasLHS(implicitCastExpr(hasImplicitDestinationType(isInteger()))
                         .bind("lhsImplicitCastToInt")),
              hasRHS(implicitCastExpr(hasImplicitDestinationType(isInteger()))
                         .bind("rhsImplicitCastToInt"))))
        .bind("binaryComparisonOperator");

class ComparisonPrinter : public MatchFinder::MatchCallback {
 public:
  virtual void run(const MatchFinder::MatchResult& Result) {
    if (const BinaryOperator* BO =
            Result.Nodes.getNodeAs<clang::BinaryOperator>(
                "binaryComparisonOperator"))
      BO->dump();
  }
};

// Apply a custom category to all command-line options so that they are the
// only ones displayed.
static llvm::cl::OptionCategory ToolCategory("loop-convert options");

// CommonOptionsParser declares HelpMessage with a description of the common
// command-line options related to the compilation database and input files.
// It's nice to have this help message in all tools.
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

// A help message for this specific tool can be added afterwards.
static cl::extrahelp MoreHelp("\nMore help text...\n");

int main(int argc, const char** argv) {
  CommonOptionsParser OptionsParser(argc, argv, ToolCategory);
  ClangTool Tool(OptionsParser.getCompilations(),
                 OptionsParser.getSourcePathList());

  ComparisonPrinter Printer;
  MatchFinder Finder;
  Finder.addMatcher(ComparisonMatcher, &Printer);

  return Tool.run(newFrontendActionFactory(&Finder).get());
}
