#pragma once
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

namespace ECS {
	/**
	 * Parse text into tokens. Whitespace is used to separate the tokens and double-quotes can be used
	 * to ignore white-spaces until another double-quote is found. Supports single-line comments.
	 * 
	 * @param line the text to parse (usually a line in a text file)
	 * @param tokens the result vector to store the tokens in.
	 * @param commentChar the character starting a single-line comment. Use '\0' to disable comment support.
	 * @return the number of tokens added. the value is negative, if a double-quote has not been closed.
	 */
	int parseTokens(const std::string& line, std::vector<std::string>& tokens, char commentChar='#');
}
