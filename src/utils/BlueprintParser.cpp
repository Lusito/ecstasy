/*******************************************************************************
 * Copyright 2011 See AUTHORS file.
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
#include <ecstasy/utils/BlueprintParser.hpp>
#include <ecstasy/utils/EntityFactory.hpp>
#include <fstream>

namespace ECS {
	//Fixme: make utf-8 safe
	std::string BlueprintParser::parse(const std::string& filename, std::shared_ptr<EntityBlueprint> &result) {
		std::ifstream file(filename);
		if(!file.is_open())
			return "Can't open file " + filename;

		result = std::make_shared<EntityBlueprint>();

		std::shared_ptr<ComponentBlueprint> lastComponent;
		std::vector<std::string> tokens;
		std::string line;
		for(int lineNum=0; std::getline(file, line); lineNum++) {
			// parse tokens
			auto error = parseLine(line, tokens);
			if(!error.empty())
				return "Line " + std::to_string(lineNum) + ": " + error;
			if(!tokens.empty()) {
				std::string &command = tokens[0];
				if(command == "add") {
					if(tokens.size() != 2)
						return "Line " + std::to_string(lineNum) + ": expected exactly one argument to 'add'";
					if(lastComponent)
						result->add(lastComponent);
					std::string &name = tokens[1];
					lastComponent = std::make_shared<ComponentBlueprint>(name);
				} else if(command == "set") {
					if(tokens.size() != 3)
						return "Line " + std::to_string(lineNum) + ": expected exactly two arguments to 'set'";
					if(!lastComponent)
						return "Line " + std::to_string(lineNum) + ": 'add' must be called before 'set'";
					lastComponent->set(tokens[1], tokens[2]);
				}
			}
		}

		if(lastComponent)
			result->add(lastComponent);

		return "";
	}

	std::string BlueprintParser::parseLine(const std::string &line, std::vector<std::string> &tokens) {
		tokens.clear();
		
		bool charactersFound = false;
		bool isEscape = false;
		bool isQuote = false;
		std::string token;
		token.reserve(line.length());
		for(char c: line) {
			if(isEscape) {
				isEscape = false;
				continue;
			}
			if(isspace(c)) {
				// skip whitespaces at the beginning
				if(!charactersFound)
					continue;
				if(isQuote)
					token += c;
				else if(!token.empty()) {
					tokens.push_back(token);
					token.clear();
				}
			} else if(isQuote) {
				if(c == '\\')
					isEscape = true;
				else if(c != '\"')
					token += c;
				else {
					isQuote = false;
					tokens.push_back(token);
					token.clear();
				}
			} else if(c == '\"') {
				if(!token.empty())
					return "quote not prefixed by whitespace";
				isQuote = true;
			} else if(c == '#') { //comment
				break;
			} else {
				charactersFound = true;
				token += c;
			}
		}

		if(isQuote)
			return "quote has not been closed";

		if(!token.empty()) {
			tokens.push_back(token);
			token.clear();
		}
		return "";
	}
}
