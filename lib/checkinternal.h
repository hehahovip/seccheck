/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2013 Daniel Marjamäki and Cppcheck team.
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
#ifndef checkinternalH
#define checkinternalH
//---------------------------------------------------------------------------

#include "check.h"
#include "config.h"

class Token;

/// @addtogroup Checks
/// @{


/** @brief %Check Internal cppcheck API usage */
class CPPCHECKLIB CheckInternal : public Check {
public:
    /** This constructor is used when registering the CheckClass */
    CheckInternal() : Check(myName()) {
    }

    /** This constructor is used when running checks. */
    CheckInternal(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger) {
    }

    /** Simplified checks. The token list is simplified. */
    void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) {
        if (!settings->isEnabled("internal"))
            return;

        CheckInternal checkInternal(tokenizer, settings, errorLogger);

        checkInternal.checkTokenMatchPatterns();
        checkInternal.checkTokenSimpleMatchPatterns();
        checkInternal.checkMissingPercentCharacter();
        checkInternal.checkUnknownPattern();
        checkInternal.checkRedundantNextPrevious();
    }

    /** @brief %Check if a simple pattern is used inside Token::Match or Token::findmatch */
    void checkTokenMatchPatterns();

    /** @brief %Check if a complex pattern is used inside Token::simpleMatch or Token::findsimplematch */
    void checkTokenSimpleMatchPatterns();

    /** @brief %Check for missing % end character in Token::Match pattern */
    void checkMissingPercentCharacter();

    /** @brief %Check for unknown (invalid) complex patterns like "%typ%" */
    void checkUnknownPattern();

    /** @brief %Check for inefficient usage of Token::next(), Token::previous() and Token::tokAt() */
    void checkRedundantNextPrevious();

private:
    void multiComparePatternError(const Token *tok, const std::string &pattern, const std::string &funcname);
    void simplePatternError(const Token *tok, const std::string &pattern, const std::string &funcname);
    void complexPatternError(const Token *tok, const std::string &pattern, const std::string &funcname);
    void missingPercentCharacterError(const Token *tok, const std::string &pattern, const std::string &funcname);
    void unknownPatternError(const Token* tok, const std::string& pattern);
    void redundantNextPreviousError(const Token* tok, const std::string& func1, const std::string& func2);

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const {
        CheckInternal c(0, settings, errorLogger);
        c.multiComparePatternError(0, ";|%type%", "Match");
        c.simplePatternError(0, "class {", "Match");
        c.complexPatternError(0, "%type% ( )", "Match");
        c.missingPercentCharacterError(0, "%num", "Match");
        c.unknownPatternError(0, "%typ");
        c.redundantNextPreviousError(0, "previous", "next");
    }

    static std::string myName() {
        return "cppcheck internal API usage";
    }

    std::string classInfo() const {
        // Don't include these checks on the WIKI where people can read what
        // checks there are. These checks are not intended for users.
        return "";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif // checkinternalH
