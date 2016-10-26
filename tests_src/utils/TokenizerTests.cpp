/*******************************************************************************
 * Copyright 2015 See AUTHORS file.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************************/
#include "../TestBase.hpp"
#include <ecstasy/utils/Tokenizer.hpp>

using ecstasy::parseTokens;
namespace TokenizerTests {

	TEST_CASE("test_empty") {
		std::string data = "";
		std::vector<std::string> tokens;
		int numTokens = parseTokens(data, tokens);
		REQUIRE(numTokens == 0);
		REQUIRE(tokens.size() == 0);
	}

	TEST_CASE("test_only_white") {
		std::string data = "	  ";
		std::vector<std::string> tokens;
		int numTokens = parseTokens(data, tokens);
		REQUIRE(numTokens == 0);
		REQUIRE(tokens.size() == 0);
	}

	TEST_CASE("test_simple_tokens") {
		std::string data = "hello world foo bar";
		std::vector<std::string> tokens;
		int numTokens = parseTokens(data, tokens);
		REQUIRE(numTokens == 4);
		REQUIRE(tokens.size() == 4);
		REQUIRE(tokens[0] == "hello");
		REQUIRE(tokens[1] == "world");
		REQUIRE(tokens[2] == "foo");
		REQUIRE(tokens[3] == "bar");
	}

	TEST_CASE("test_number_tokens") {
		std::string data = "1.2345 7.890";
		std::vector<std::string> tokens;
		int numTokens = parseTokens(data, tokens);
		REQUIRE(numTokens == 2);
		REQUIRE(tokens.size() == 2);
		REQUIRE(tokens[0] == "1.2345");
		REQUIRE(tokens[1] == "7.890");
	}

	TEST_CASE("test_quotes") {
		std::string data = "first \"and second\" and third";
		std::vector<std::string> tokens;
		int numTokens = parseTokens(data, tokens);
		REQUIRE(numTokens == 4);
		REQUIRE(tokens.size() == 4);
		REQUIRE(tokens[0] == "first");
		REQUIRE(tokens[1] == "and second");
		REQUIRE(tokens[2] == "and");
		REQUIRE(tokens[3] == "third");
	}

	TEST_CASE("test_multi_quotes") {
		std::string data = "first \"and second\" \"and third\"";
		std::vector<std::string> tokens;
		int numTokens = parseTokens(data, tokens);
		REQUIRE(numTokens == 3);
		REQUIRE(tokens.size() == 3);
		REQUIRE(tokens[0] == "first");
		REQUIRE(tokens[1] == "and second");
		REQUIRE(tokens[2] == "and third");
	}

	TEST_CASE("test_open_quotes") {
		std::string data = "first \"and second and third";
		std::vector<std::string> tokens;
		int numTokens = parseTokens(data, tokens);
		REQUIRE(numTokens == -2);
		REQUIRE(tokens.size() == 2);
		REQUIRE(tokens[0] == "first");
		REQUIRE(tokens[1] == "and second and third");
	}

	TEST_CASE("test_quotes_without_whitespace") {
		std::string data = "hello\"foo bar\"world";
		std::vector<std::string> tokens;
		int numTokens = parseTokens(data, tokens);
		REQUIRE(numTokens == 3);
		REQUIRE(tokens.size() == 3);
		REQUIRE(tokens[0] == "hello");
		REQUIRE(tokens[1] == "foo bar");
		REQUIRE(tokens[2] == "world");
	}

	TEST_CASE("test_trim") {
		std::string data = "	 one two		";
		std::vector<std::string> tokens;
		int numTokens = parseTokens(data, tokens);
		REQUIRE(numTokens == 2);
		REQUIRE(tokens.size() == 2);
		REQUIRE(tokens[0] == "one");
		REQUIRE(tokens[1] == "two");
	}

	TEST_CASE("test_comment_end") {
		std::string data = "one two #comment";
		std::vector<std::string> tokens;
		int numTokens = parseTokens(data, tokens);
		REQUIRE(numTokens == 2);
		REQUIRE(tokens.size() == 2);
		REQUIRE(tokens[0] == "one");
		REQUIRE(tokens[1] == "two");
	}

	TEST_CASE("test_comment_start") {
		std::string data = "#one two comment";
		std::vector<std::string> tokens;
		int numTokens = parseTokens(data, tokens);
		REQUIRE(numTokens == 0);
		REQUIRE(tokens.size() == 0);
	}

	TEST_CASE("test_comment_without_white") {
		std::string data = "first#comment";
		std::vector<std::string> tokens;
		int numTokens = parseTokens(data, tokens);
		REQUIRE(numTokens == 1);
		REQUIRE(tokens.size() == 1);
		REQUIRE(tokens[0] == "first");
	}

	TEST_CASE("test_comment_start_trim") {
		std::string data = "	#one two comment";
		std::vector<std::string> tokens;
		int numTokens = parseTokens(data, tokens);
		REQUIRE(numTokens == 0);
		REQUIRE(tokens.size() == 0);
	}

	TEST_CASE("test_custom_comment_char") {
		std::string data = "	@comment";
		std::vector<std::string> tokens;
		int numTokens = parseTokens(data, tokens, '@');
		REQUIRE(numTokens == 0);
		REQUIRE(tokens.size() == 0);
	}

	TEST_CASE("test_no_comment_char") {
		std::string data = "	#comment";
		std::vector<std::string> tokens;
		int numTokens = parseTokens(data, tokens, '\0');
		REQUIRE(numTokens == 1);
		REQUIRE(tokens.size() == 1);
		REQUIRE(tokens[0] == "#comment");
	}

	TEST_CASE("test_add") {
		std::string data = "one";
		std::vector<std::string> tokens;
		tokens.push_back("zero");
		int numTokens = parseTokens(data, tokens);
		REQUIRE(numTokens == 1);
		REQUIRE(tokens.size() == 2);
		REQUIRE(tokens[0] == "zero");
		REQUIRE(tokens[1] == "one");
	}
}
