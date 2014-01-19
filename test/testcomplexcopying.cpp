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


#include "tokenize.h"
#include "checkcomplexcopying.h"
#include "testsuite.h"
#include <sstream>

extern std::ostringstream errout;

class TestComplexCopying : public TestFixture {
public:
    TestComplexCopying() : TestFixture("TestComplexCopying") {
    }

private:


    void run() {
        TEST_CASE(vectorcopying);
    }

    void check(const char code[]) {
        // Clear the error buffer..
        errout.str("");

        Settings settings;
        settings.addEnabled("performance");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        // Check char variable usage..
        CheckComplexCopying checkComplexCopying(&tokenizer, &settings, this);
        checkComplexCopying.checkComplexParameters();
    }

    void vectorcopying() {
        check("void foo(stl::vector<std::string> p)\n"
              "{\n"
              "    stl::vector<std::string> a = p;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:1]: (performance) Complex objects copying "
			"in Function foo may slow down system performance.\n"
			"Please use pointer or reference instead.", errout.str());
    }
};

REGISTER_TEST(TestComplexCopying)
