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
// Miscellaneous checkers
//---------------------------------------------------------------------------

#include "checkmiscellaneous.h"
#include "symboldatabase.h"
#include <unordered_set>

using namespace std;

//---------------------------------------------------------------------------


// Register this check class (by creating a static instance of it)
namespace {
    CheckMiscellaneous instance;
}

namespace {
    class FloatDefine
    {
    public:
        FloatDefine()
        {
            floatDef.insert("float");
            floatDef.insert("double");
            floatDef.insert("long double");
        }

        bool isFloat(const string& varDef) const
        {
            return floatDef.find(varDef) != floatDef.end();
        }

    private:
        unordered_set<string> floatDef;
    };

    /** Is given variable a float variable */
    static bool isFloat(const Variable *var)
    {
        if (var == 0)
        {
            return false;
        }

        static FloatDefine fd;
        const string& varDef = var->nameToken()->previous()->str();
        return fd.isFloat(varDef);
    }

    /** Is given variable a time_t variable */
    static bool isTimeT(const Variable *var)
    {
        if (var == 0)
        {
            return false;
        }

        const string& varDef = var->nameToken()->previous()->str();
        return "time_t" == (varDef);
    }

    /** Is given variable an unsigned variable */
    static bool isUnsigned(const Variable *var)
    {
        if (var == 0)
        {
            return false;
        }

        return var->typeEndToken()->isUnsigned();
    }

    static bool isFloatVariable(const Token* tok)
    {
        if (tok == 0)
        {
            return false;
        }

        return (tok->type() == Token::eVariable) && isFloat(tok->variable());
    }

    static bool isTimeTVariable(const Token* tok)
    {
        if (tok == 0)
        {
            return false;
        }

        return (tok->type() == Token::eVariable) && isTimeT(tok->variable());
    }

    static bool isUnsignedVariable(const Token* tok)
    {
        if (tok == 0)
        {
            return false;
        }

        return (tok->type() == Token::eVariable) && isUnsigned(tok->variable());
    }

    static bool isSignedChar(const Token* tok)
    {
        if (tok == 0)
        {
            return false;
        }

        return (tok->type() == Token::eVariable) && !isUnsigned(tok->variable());
    }

    static bool isNumberOrVariable(const Token *tok)
    {
        return (tok->type() == Token::eVariable) || (tok->type() == Token::eNumber);
    }

    static bool isBitOperatorOnNoUnsignedVariable(const Token *tok)
    {
        if ((tok == 0) || (tok->previous() == 0) || (tok->next() == 0))
        {
            return false;
        }

        if (tok->type() != Token::eBitOp)
        {
            return false;
        }

        Token* prev = tok->previous();
        Token* next = tok->next();

        if (!isNumberOrVariable(prev) || !isNumberOrVariable(next))
        {
            return false;
        }

        return !isUnsignedVariable(prev) || !isUnsignedVariable(next);
    }

    static bool isFloatComparison(const Token* tok)
    {
        Token* prev = tok->previous();
        Token* next = tok->next();

        if (isFloatVariable(prev))
        {
            if (next->type() == Token::eNumber)
            {
                // Compare variable to constant value
                return true;
            }
            else if (isFloatVariable(next))
            {
                // Both token are floating variables 
                return true;
            }

            return false;
        }

        if (isFloatVariable(next))
        {
            if (prev->type() == Token::eNumber)
            {
                // Compare variable to constant value
                return true;
            }
            else if (isFloatVariable(prev))
            {
                // Both token are floating variables 
                return true;
            }

            return false;
        }

        return false;
    }

    const string CTYPE_CHAR_FUNCS = "isalnum|isalpha|isascii|isblank|iscntrl|isdigit|isgraph|"
                "islower|isprint|ispunct|isspace|isupper|isxdigit|toascii|toupper|tolower";
}

void CheckMiscellaneous::floatEqualsError(const Token *tok)
{
	reportError(tok, Severity::warning,
                "FloatEqualsError",
                "Comparing two float variables is improper.\n"
				"Should avoid compare two float variables directly. "
				"Maybe you can compare with an epsilon value."
				"Please see: http://stackoverflow.com/questions/17333/most-effective-way-for-float-and-double-comparison ");
}

// https://www.securecoding.cert.org/confluence/display/cplusplus/MSC05-CPP.+Do+not+manipulate+time_t+typed+values+directly
void CheckMiscellaneous::timetOperError(const Token *tok)
{
	reportError(tok, Severity::warning,
                "time_tArithmeticError",
                "There is no safe way to manually perform arithmetic on the time_t type.\n"
				"The time_t values should not be modified directly. "
				"Please see: CERT C++ Secure Coding Standard  49. Miscellaneous (MSC) "
                "MSC05-CPP. Do not manipulate time_t typed values directly. ");
}

// https://www.securecoding.cert.org/confluence/pages/viewpage.action?pageId=20086972
void CheckMiscellaneous::SignedBitOperError(const Token *tok)
{
    reportError(tok, Severity::warning,
                "SignedBitoperError",
                "Bitwise operators should only be used with unsigned integer operands.\n"
				"Bitwise operators should only be used with unsigned integer operands, "
                "as the results of some bitwise operations on signed integers is implementation defined. "
				"Please see: CERT C++ Secure Coding Standard INT13-CPP. Use bitwise operators only on unsigned operands.");
}

// STR37-C
// see https://www.securecoding.cert.org/confluence/pages/viewpage.action?pageId=20087109 
void CheckMiscellaneous::SignedCharError(const Token *tok)
{
    reportError(tok, Severity::warning,
                "SignedCharError",
                "Arguments to character handling functions must be representable as an unsigned char.\n"
				"The header <ctype.h> declares several functions useful for classifying and mapping characters. "
                "In all cases the argument is an int, "
                "the value of which shall be representable as an unsigned char or shall equal the value of the macro EOF. "
				"Please see: CERT C++ Secure Coding Standard STR37-C.");
}

void CheckMiscellaneous::modifyStdError(const Token *tok)
{
    reportError(tok, Severity::warning,
                "ModifyStdNamespaceError",
                "Do not modify the standard namespaces.\n"
				"The standard library introduces the namespace std for standards-provided declarations such as std::string, std::vector, and std::for_each. "
                "However, it is undefined behavior to introduce new declarations in namespace std, except under special circumstances. "
				"Please see: CERT C++ Secure Coding Standard MSC34-CPP.");
}

void CheckMiscellaneous::returnErrnoError(const Token *tok)
{
    reportError(tok, Severity::warning,
                "FunctionReturnErrnoError",
                "Functions that return errno should change to a return type of errno_t.\n"
				"Many existing functions that return errno are declared as returning a value of type int. "
                "It is semantically unclear by looking at the function declaration or prototype "
                "if these functions return an error status or a value or worse, some combination of the two. "
                "TR 24731-1 introduces the new type errno_t instead. "
				"Please see: CERT C++ Secure Coding Standard DCL09-CPP.");
}

void CheckMiscellaneous::floatLoopError(const Token *tok)
{
    reportError(tok, Severity::warning,
                "FloatNumberAsLoopCounterError",
                "Do not use floating-point variables as loop counters.\n"
				"Different implementations have different precision limitations, "
                "and to keep code portable, floating-point variables should not be used as loop counters. "
				"Please see: CERT C++ Secure Coding Standard FLP30-CPP.");
}

/** Check for improper floating arithmetic
* See CERT C++ Secure Coding Standard:
* FLP00-CPP. Understand the limitations of floating-point numbers
*/
void CheckMiscellaneous::runChecks()
{
	// TODO
	//if (!_settings->isEnabled("portability"))
    //    return;

    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();

    // Check return values
    const std::size_t functions = symbolDatabase->functionScopes.size();

	// Check assignments
    for (std::size_t i = 0; i < functions; ++i) 
    {
        const Scope * scope = symbolDatabase->functionScopes[i];
        for (const Token *tok = scope->classStart; tok && tok != scope->classEnd; tok = tok->next()) {
			if (tok->type() == Token::eComparisonOp)
			{
				if (tok->str() != "==")
			    {
				    continue;
			    }

			    if (isFloatComparison(tok))
			    {
				    floatEqualsError(tok);
			    }
			}
            else if (tok->type() == Token::eArithmeticalOp)
            {
                if (isTimeTVariable(tok->next()) || isTimeTVariable(tok->previous()))
                {
                    timetOperError(tok);
                }
            }
			else if (isBitOperatorOnNoUnsignedVariable(tok))
            {
                SignedBitOperError(tok);
            }
            else if (Token::Match(tok, "isalnum|isalpha|isascii|isblank|iscntrl|isdigit|isgraph|"
                "islower|isprint|ispunct|isspace|isupper|isxdigit|toascii|toupper|tolower ( %var% )"))
            {
                // VOID STR37-CPP
                // STR37-C
                // see https://www.securecoding.cert.org/confluence/pages/viewpage.action?pageId=20087109
                // Should check the parameter type of unsigned char. 
                const Token* varTk = tok->tokAt(2);

                if (isSignedChar(varTk))
                {
                    SignedCharError(varTk);
                }
            }
            else if (Token::simpleMatch(tok, "return errno ;"))
            {
                returnErrnoError(tok);
            }
            else if (Token::simpleMatch(tok, "for ( double") || Token::simpleMatch(tok, "for ( float"))
            {
                // see: https://www.securecoding.cert.org/confluence/display/seccode/FLP30-C.+Do+not+use+floating-point+variables+as+loop+counters 
                floatLoopError(tok);
            }
            else
            {
            }

            /*
            else if (Token::Match(tok, "[;{}] %var% :"))
            {
                if ((tok->scope()->type == Scope::eTry) || (tok->scope()->type == Scope::eTry))
                {
                    // goto label within try or catch block
                    // Visual C++ or gcc will generate compile errors, so ingore it. 
                    // VOID MSC35-CPP
                }
            }
            */
        }
    }

    for (auto i = symbolDatabase->scopeList.begin(); i != symbolDatabase->scopeList.end(); ++i)
    {
        if ((i->type == Scope::eNamespace) && (i->className == "std"))
        {
            // MSC34-CPP. Do not modify the standard namespaces
            // see https://www.securecoding.cert.org/confluence/pages/viewpage.action?pageId=30605359
            modifyStdError(i->classDef);
        }
    }
}
//---------------------------------------------------------------------------
