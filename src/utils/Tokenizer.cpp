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
#include <string>
#include <vector>

namespace ecstasy {
	char parseEscapeToken(char c) {
		if(c == 't')
			return '\t';
		else if(c == 'r')
			return '\r';
		else if(c == 'n')
			return '\n';
		return c;
	}

	int parseTokens(const std::string& line, std::vector<std::string>& tokens, char commentChar) {
		int numTokens = 0;
		bool charactersFound = false;
		bool isEscape = false;
		bool isQuote = false;
		std::string token;
		token.reserve(line.length());
		for(char c: line) {
			if(isEscape) {
				token += parseEscapeToken(c);
				isEscape = false;
			} else if(isQuote) {
				if(c == '\\')
					isEscape = true;
				else if(c != '\"')
					token += c;
				else {
					isQuote = false;
					numTokens++;
					tokens.push_back(token);
					token.clear();
				}
			} else if(isspace(c)) {
				// Skip white-spaces at the beginning
				if(!charactersFound)
					continue;
				if(isQuote)
					token += c;
				else if(!token.empty()) {
					numTokens++;
					tokens.push_back(token);
					token.clear();
				}
			} else if(c == '\"') {
				if(!token.empty()) {
					numTokens++;
					tokens.push_back(token);
					token.clear();
				}
				isQuote = true;
			} else if(c == commentChar) {
				// Start of a comment, so skip the rest of the line
				break;
			} else {
				charactersFound = true;
				token += c;
			}
		}

		if(isQuote || !token.empty()) {
			numTokens++;
			tokens.push_back(token);
		}
		return isQuote ? -numTokens : numTokens;
	}
}
