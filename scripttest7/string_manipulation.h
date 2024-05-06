#pragma once
#include <string>
#include <vector>
#include <optional>

/**
 * \brief 入力文字列を行単位で分割 空行含む
 * \tparam CharIter Iterator
 * \param whole_begin begin iterator
 * \param whole_end end iterator
 * \return list for line
 */
template <typename CharIter>
auto split_by_line(CharIter const whole_begin, CharIter const whole_end) -> std::vector<std::string> {
	std::vector<std::string> ret;

	std::string line;

	for (auto it = whole_begin; it != whole_end; ++it) {
		if (*it == '\n') {
			// 末尾のコントロール空白文字(改行2種)を削除
			while (!line.empty() && std::ispunct(line.back()) && std::isspace(line.back())) line.pop_back();
			ret.push_back(line);

			line.clear();
		}
		else {
			line.push_back(*it);
		}
	}

	if (!line.empty()) {
		// 末尾のコントロール空白文字(改行2種)を削除
		while (std::ispunct(line.back()) && std::isspace(line.back())) line.pop_back();
		ret.push_back(line);

	}

	return ret;
}

template <typename CharIter>
std::vector<std::string> tokenize(CharIter const first, CharIter const last) {
	std::vector<std::string> ret;
	std::string work;

	char quote = '\0';
	bool in_escape = false;

	auto consume =
		[&](std::string& workarea, const bool allow_empty = false) {
			if (allow_empty || !workarea.empty()) { ret.push_back(workarea); workarea.clear(); }
		};

	for (auto it = first; it != last && *it != ';'; /* nop */) {
		
		if (in_escape) {
			// エスケープ中(最優先)
			work.push_back(*it++);
			in_escape = false;
		}
		else {
			if (quote == '\0') {
				// 文字列の外
				if (std::isspace(*it)) {
					// 空白文字はトークン区切りとする
					consume(work);
					++it;
				}
				else if (*it == '\'' || *it == '"') {
					// 文字列開始
					consume(work);
					work.push_back(quote = *it++);
				}
				else {
					work.push_back(*it++);
				}
			}
			else {
				// 文字列の中
				if (*it == '\\') {
					work.push_back(*it++);
					in_escape = true;
				}
				else {
					char ch;
					work.push_back(ch = *it++);
					if (ch == quote) {
						consume(work);
					}
				}
			}
		}

	}

	consume(work);

	return ret;
}

