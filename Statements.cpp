/*
 * Created by Tyler Gearing 3/14/19
 *
 */

#include "Statements.hpp"

#define RETURN "retVal"

// Statement
Statement::Statement() = default;

// Statements
Statements::Statements() = default;

void Statements::addStatement(std::unique_ptr<Statement> statement) {
	_statements.push_back(std::move(statement));
}

void Statements::print() {
    for (auto &s: _statements)
        s->print();
}

void Statements::evaluate(SymTab &symTab, std::unique_ptr<FuncTab> &funcTab) {
	for (auto &s: _statements) {
		if(symTab.isDefinedGlobal(RETURN))
			break;
		s->evaluate(symTab, funcTab);
	}
}

std::vector<std::unique_ptr<Statement>> &Statements::getStatements() {
	return _statements;
}

// AssignmentStatement
AssignmentStatement::AssignmentStatement(): _lhsVariable{""},
											_lhsExpression{nullptr},
											_rhsExpression{nullptr} {}

AssignmentStatement::AssignmentStatement(std::string lhsVar,
										 std::unique_ptr<ExprNode> lhsExpr,
										 std::unique_ptr<ExprNode> rhsExpr):
	_lhsVariable{std::move(lhsVar)}, _lhsExpression{std::move(lhsExpr)},
	_rhsExpression{std::move(rhsExpr)} {}

void AssignmentStatement::evaluate(SymTab &symTab,
								   std::unique_ptr<FuncTab> &funcTab) {
	if(_lhsExpression == nullptr) {
        symTab.setValueFor(lhsVariable(),
						   rhsExpression()->evaluate(symTab, funcTab));
    } else {
		auto lhsindex = dynamic_cast<NumberDescriptor*>
			(_lhsExpression->evaluate(symTab, funcTab).get());
		if(lhsindex->type() != TypeDescriptor::INTEGER) {
			std::cout << "AssignmentStatement::evaluate error index must be";
			std::cout << " an integer\n";
			exit(1);
		}
		int index = lhsindex->value.intValue;
		auto lhs = symTab.getValueFor(lhsVariable());
		auto rhs = _rhsExpression->evaluate(symTab, funcTab);
		if(lhs->type() == TypeDescriptor::STRINGARRAY) {
            if(rhs->type() != TypeDescriptor::STRING){
                std::cout<<"array value not of compatible types"<<std::endl;
                exit(1);
            }
			auto desc = dynamic_cast<StringDescriptor*>(rhs.get());
            std::string val = desc->value;
            dynamic_cast<StringArray*>(lhs.get())->setSubStr(index, val);
        } else if(lhs->type() == TypeDescriptor::NUMBERARRAY) {
            if(rhs->type() != TypeDescriptor::INTEGER){
                std::cout<<"array value not of compatible types"<<std::endl;
                exit(1);
            }
			auto desc = dynamic_cast<NumberDescriptor*>(rhs.get());
			int val = desc->value.intValue;
            dynamic_cast<NumberArray*>(lhs.get())->setSubNum(index, val);
        } 
	}
}

std::string &AssignmentStatement::lhsVariable() {
	return _lhsVariable;
}

std::unique_ptr<ExprNode> &AssignmentStatement::lhsExpression() {
    return _lhsExpression;
}

std::unique_ptr<ExprNode> &AssignmentStatement::rhsExpression() {
    return _rhsExpression;
}

void AssignmentStatement::print() {
    std::cout << _lhsVariable << " = ";
    _rhsExpression->print();
	std::cout << std::endl;
}

// PrintStatement
PrintStatement::PrintStatement(): _rhsList{} {}

PrintStatement::PrintStatement(std::vector<std::shared_ptr<ExprNode>>exprList):
	_rhsList{std::move(exprList)} {}

void PrintStatement::evaluate(SymTab &symTab, std::unique_ptr<FuncTab> &funcTab) {
	for (auto &l: _rhsList ) {
        printValue( l->evaluate(symTab, funcTab).get() );
		std::cout << ' ';
	}
	std::cout << std::endl;
}

std::vector<std::shared_ptr<ExprNode>>&PrintStatement::rhsList() {
    return _rhsList;
}

void PrintStatement::print() {
	for (auto &l: _rhsList) {
		l->print();
		std::cout << std::endl;
	}
}

// CallStatement
CallStatement::CallStatement(): _call{nullptr} {}

CallStatement::CallStatement(std::unique_ptr<ExprNode> call) :
	_call{std::move(call)} {}

void CallStatement::evaluate(SymTab &symTab,
							 std::unique_ptr<FuncTab> &funcTab) {
	_call->evaluate(symTab, funcTab);
}

std::unique_ptr<ExprNode> &CallStatement::call() {
	return _call;
}

void CallStatement::print() {
	_call->print();
}

// ReturnStatement
ReturnStatement::ReturnStatement(): _stmt{nullptr} {}

ReturnStatement::ReturnStatement(std::unique_ptr<ExprNode> stmt) :
	_stmt{std::move(stmt)} {}

void ReturnStatement::evaluate(SymTab &symTab,
							   std::unique_ptr<FuncTab> &funcTab) {
	symTab.setReturnValue(_stmt->evaluate(symTab, funcTab));
}

std::unique_ptr<ExprNode> &ReturnStatement::stmt() {
	return _stmt;
}

void ReturnStatement::print() {
	_stmt->print();
}

// ForStatement
ForStatement::ForStatement() : _statements{nullptr}  {}

ForStatement::ForStatement(std::string id,
						   std::vector<std::shared_ptr<ExprNode>> range,
						   std::unique_ptr<Statements> stmnts):
	_id{std::move(id)}, _range{std::move(range)},
	_statements{std::move(stmnts)} {}

void ForStatement::evaluate(SymTab &symTab,
							std::unique_ptr<FuncTab> &funcTab) {
	std::unique_ptr<Range> range =
		std::make_unique<Range>(getId(), getRange(), symTab, funcTab);
	while( !range->atEnd() ) {
		statements()->evaluate(symTab, funcTab);
		symTab.increment( getId(), range->step() );
		range->getNext();
	}    
}

std::unique_ptr<Statements>&ForStatement::statements() {
	return _statements;
}

std::vector<std::shared_ptr<ExprNode>> &ForStatement::getRange() {
	return _range;
}

std::string &ForStatement::getId() {
	return _id;
}

void ForStatement::print() {
	std::cout << _id << std::endl;
	for(auto &n : _range)
		n->print();
	std::cout << std::endl;
 	_statements->print();
    std::cout << std::endl;
}

// IfStatement
IfStatement::IfStatement() : _firstTest{nullptr}, _firstSuite{nullptr},
							 _elseSuite{nullptr}  {}

IfStatement::IfStatement(std::unique_ptr<ExprNode>firstTest,
						 std::unique_ptr<Statements>firstSuite,
						 std::vector<std::unique_ptr<ExprNode>> elifTests,
						 std::vector<std::unique_ptr<Statements>> elifSuites,
						 std::unique_ptr<Statements>elseSuite):
	_firstTest{std::move(firstTest)},
	_firstSuite{std::move(firstSuite)},
	_elifTests{std::move(elifTests)},
	_elifSuites{std::move(elifSuites)},
	_elseSuite{std::move(elseSuite)} {}

void IfStatement::evaluate(SymTab &symTab, std::unique_ptr<FuncTab> &funcTab) {	
	if(evaluateBool(firstTest()->evaluate(symTab, funcTab).get())) {
		firstSuite()->evaluate(symTab, funcTab);
		return;
	} else if(_elifTests.size() != _elifSuites.size() ) {
		std::cout << "IfStatement::evaluate mismatched elif and arguments\n";
		exit(1);
	} else if ( !_elifTests.empty() ) {
		int i = 0;
		for( auto &t: _elifTests ) {
			if( evaluateBool( t->evaluate(symTab, funcTab).get() ) ) {
				_elifSuites[i]->evaluate(symTab, funcTab);
				return;
			}
			++i;
		}
	} else if( _elseSuite != nullptr )
		_elseSuite->evaluate(symTab, funcTab);
}

std::unique_ptr<ExprNode> &IfStatement::firstTest() {
	return _firstTest;
}

std::unique_ptr<Statements> &IfStatement::firstSuite() {
	return _firstSuite;
}

std::vector<std::unique_ptr<ExprNode>> &IfStatement::elifTests() {
	return _elifTests;
}

std::vector<std::unique_ptr<Statements>> &IfStatement::elifSuites() {
	return _elifSuites;
}

std::unique_ptr<Statements> &IfStatement::elseSuite() {
	return _elseSuite;
}

void IfStatement::print() {
	_firstTest->print();
	std::cout << std::endl;
	_firstSuite->print();
    std::cout << std::endl;
	for(auto &t: _elifTests)
		t->print();
	std::cout << std::endl;
	for(auto &s: _elifSuites)
		s->print();
	std::cout << std::endl;
	if(_elseSuite)
		_elseSuite->print();
	std::cout << std::endl;
}

// Function
Function::Function(): _id{""}, _parameters{},  _suite{} {}

Function::Function(std::string id, std::vector<std::string> params,
				   std::unique_ptr<Statements> suite):
	_id{id}, _parameters{params}, _suite{std::move(suite)} {}

void Function::evaluate(SymTab &symTab, std::unique_ptr<FuncTab> &funcTab) {
	_suite->evaluate(symTab, funcTab);
}

std::string &Function::id() {
	return _id;
}

std::vector<std::string> &Function::params() {
	return _parameters;
}

std::unique_ptr<Statements> &Function::suite() {
	return _suite;
}

void Function::print() {
	std::cout << _id << std::endl;
	for(auto &p: _parameters)
		std::cout << p << std::endl;
	_suite->print();
    std::cout << std::endl;
}

//Array Ops 
ArrayOps::ArrayOps() : _id{""}, _op{""}, _test{nullptr} {}

ArrayOps::ArrayOps(std::string id, std::string op,
				   std::unique_ptr<ExprNode> test) :
	_id{id}, _op{op}, _test{std::move(test)} {}

void ArrayOps::evaluate(SymTab & symTab, std::unique_ptr<FuncTab> &funcTab) {
	auto type = symTab.getValueFor(_id)->type();
	if(_op == "append") {
		if(_test == nullptr) {
			std::cout << "ArrayOps::evaluate append, no element provided\n";
			exit(1);
		}
		auto element = _test->evaluate(symTab, funcTab);
		if( type == TypeDescriptor::NUMBERARRAY ) {
			if(element->type() == TypeDescriptor::INTEGER) {
				auto nDesc = dynamic_cast<NumberDescriptor*>(element.get());
				auto narray = dynamic_cast<NumberArray*>
					(symTab.getValueFor(_id).get());
				narray->nAppend(nDesc->value.intValue);
			} else {
				std::cout << "ArrayOps::append error: members must be of ";
				std::cout << "the same type\n";
				exit(1);
			}
		} else if(type == TypeDescriptor::STRINGARRAY) {
			if(element->type() == TypeDescriptor::STRING) {
				auto sDesc = dynamic_cast<StringDescriptor*>(element.get());
				auto sarray = dynamic_cast<StringArray*>
					(symTab.getValueFor(_id).get());
				sarray->sAppend(sDesc->value);
			} else {
				std::cout << "ArrayOps::append error: members must be of ";
				std::cout << "the same type\n";
				exit(1);
			}
		} else if (type == TypeDescriptor::NULLARRAY) {
			if(element->type() == TypeDescriptor::INTEGER) {
				auto nDesc = dynamic_cast<NumberDescriptor*>(element.get());
				if(nDesc == nullptr) {
					std::cout << "ArrayOps::evaluate  error: invalid cast";
					exit(1);
				}
				std::shared_ptr<NumberArray> narray =
					std::make_shared<NumberArray>(TypeDescriptor::NUMBERARRAY);
				narray->nAppend(nDesc->value.intValue);
				symTab.setValueFor(_id, narray);
			} else if(element->type() == TypeDescriptor::STRING) {
				auto sDesc = dynamic_cast<StringDescriptor*>(element.get());
				if(sDesc == nullptr) {
					std::cout << "ArrayOps::evaluate  error: invalid cast";
					exit(1);
				}					
				std::shared_ptr<StringArray> sarray =
					std::make_shared<StringArray>(TypeDescriptor::STRINGARRAY);
				sarray->sAppend(sDesc->value);
				symTab.setValueFor(_id, sarray);
			} else {
				std::cout << "append() is not supported for this type\n";
				exit(1);
			}
		} else {
			std::cout << "append() is not supported for this type\n";
			exit(1);
		}
	} else if (_op == "pop") {
		if( type == TypeDescriptor::NUMBERARRAY ) {
			auto narray = dynamic_cast<NumberArray*>
				(symTab.getValueFor(_id).get());
			if(narray != nullptr) {
				if(_test == nullptr)
					narray->nPop();
				else {
					auto element = _test->evaluate(symTab, funcTab);
					narray->nPopIndex(element.get());
				}
			}
		} else if(type == TypeDescriptor::STRINGARRAY) {
			auto sarray = dynamic_cast<StringArray*>
				(symTab.getValueFor(_id).get());
			if(sarray != nullptr) {
				if(_test == nullptr)
					sarray->sPop();
				else {
					auto element = _test->evaluate(symTab, funcTab);
					sarray->sPopIndex(element.get());
				}
			}
		} else {
			std::cout << "pop is not supported for this type\n";
			exit(1);
		}
	}
}

void ArrayOps::print(){}
