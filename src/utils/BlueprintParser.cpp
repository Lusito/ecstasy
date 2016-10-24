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
#include <ecstasy/utils/Blueprint.hpp>
#include <ecstasy/utils/BlueprintParser.hpp>
#include <ecstasy/utils/Tokenizer.hpp>
#include <fstream>

namespace ECS {
	std::string parseBlueprint(const std::string& filename, std::shared_ptr<EntityBlueprint>& result) {
		std::ifstream file(filename);
		if(!file.is_open())
			return "Can't open file " + filename;
		return parseBlueprint(file, result);
	}

	std::string parseBlueprint(std::istream& stream, std::shared_ptr<EntityBlueprint>& result) {
		result = std::make_shared<EntityBlueprint>();

		std::shared_ptr<ComponentBlueprint> lastComponent;
		std::vector<std::string> tokens;
		std::string line;
		for(int lineNum=0; std::getline(stream, line); lineNum++) {
			// parse tokens
			tokens.clear();
			int numTokens = parseTokens(line, tokens);
			if(numTokens < 0)
				return "Line " + std::to_string(lineNum) + ": quote has not been closed";
			if(numTokens > 0) {
				std::string& command = tokens[0];
				if(command == "add") {
					if(numTokens != 2)
						return "Line " + std::to_string(lineNum) + ": expected exactly one argument to 'add'";
					if(lastComponent)
						result->add(lastComponent);
					std::string& name = tokens[1];
					lastComponent = std::make_shared<ComponentBlueprint>(name);
				} else if(command == "set") {
					if(numTokens != 3)
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
}
