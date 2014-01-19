/*
 * Seccheck - A tool for security C/C++ code analysis
 * Copyright (C) 2014 Wang Anyu
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

//---------------------------------------------------------------------------
// Complex object copying
//---------------------------------------------------------------------------

#include "checkcomplexcopying.h"
#include "symboldatabase.h"

//---------------------------------------------------------------------------


// Register this check class (by creating a static instance of it)
namespace {
    CheckComplexCopying instance;
}

class ComplexContainers
{
public:
	ComplexContainers()
	{
		_Containers.insert("array");
		_Containers.insert("vector");		
		_Containers.insert("deque");
		_Containers.insert("list");
		_Containers.insert("forward_list");

		_Containers.insert("stack");
		_Containers.insert("queue");
		_Containers.insert("priority_queue");

		_Containers.insert("set");
		_Containers.insert("map");
		_Containers.insert("multimap");
		_Containers.insert("multiset");
		
		_Containers.insert("unordered_set");
		_Containers.insert("unordered_map");
		_Containers.insert("unordered_multimap");
		_Containers.insert("unordered_multiset");
	}

	bool isComplexClass(const std::string& para) const
	{
		return _Containers.find(para) != _Containers.end();
	}

private:
	std::set<std::string> _Containers;
};

static bool isComplexContainer(const std::string& para)
{
	static ComplexContainers containers;
	return containers.isComplexClass(para);
}

static bool isRefOrPointer(const Variable& v)
{
	return v.isReference() || v.isPointer();
}

bool CheckComplexCopying::containerAsParam(const Token* tok) const
{
	if (tok == 0)
	{
		return false;
	}

	const unsigned int variantId = tok->varId();
	if (variantId == 0)
	{
		return false;
	}

	const Variable* var = _tokenizer->getSymbolDatabase()->getVariableFromVarId(variantId);
	if (var == 0)
	{
		return false;
	}

	if (!var->isArgument())
	{
		// Only handle arguments
		return false;
	}

	if (isRefOrPointer(*var))
	{
		return false;
	}

	return true;
}

static bool IsVariableStlContainer(const Token* tok)
{
	if (tok == 0) {
		return false;
	}

	if (tok->type() != Token::eVariable)
	{
		return false;
	}

	// TODO
	return true;
}

void CheckComplexCopying::checkComplexParameters()
{
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    for (unsigned int i = 0; i < symbolDatabase->functionScopes.size(); i++) {
        const Scope* scope = symbolDatabase->functionScopes[i];
		checkComplexParametersAsArgument(scope);

		for (const Token *tok = scope->classStart; tok && tok != scope->classEnd; tok = tok->next()) {
			if (tok->type() != Token::eAssignmentOp)
			{
				continue;
			}

			// TODO
			//if (tok->str() != "==")
			//{
			//	continue;
			//}

			//if (isFloatComparison(tok))
			//{
			//	floatEqualsError(tok);
			//}
        }
    }
}

/** Check for complex objects as argument */
void CheckComplexCopying::checkComplexParametersAsArgument(const Scope* scope) {
	const Function* func = scope->function;
	if (func == 0 || !func->hasBody){
		// We only look for functions with a body
        return;
	}

	const size_t totalCount = func->argCount();
	for (size_t i = 0; i < totalCount; i++) {
		const Variable* var = func->getArgumentVar(i);

		if (isRefOrPointer(*var)) {
			continue;
		}

		for (const Token *typetok = var->typeStartToken(); 
			typetok != var->typeEndToken(); typetok = typetok->next()) {
			if (isComplexContainer(typetok->str()) ) {
				std::ostringstream errmsg;
				errmsg << "Complex objects copying in Function " << func->name() 
					<< " may slow down system performance.\n"
					<< "Please use pointer or reference instead.";
				// STL Container
				reportError(func->token, Severity::performance, 
					"complexObjectCopying", errmsg.str());
				break;
			}
        }
	}
}

//---------------------------------------------------------------------------
