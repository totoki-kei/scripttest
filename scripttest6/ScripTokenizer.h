#pragma once

#ifndef SCRIPTOKENIZER_H_
#define SCRIPTOKENIZER_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <regex>

#include <variant>

#include "ScripTypes.h"
#include "ScripParser.h"

namespace Scrip
{
	using TokenValue = std::variant<EvalValue, EvalValueList, StringName>;
	struct TokenList {
		struct TokenWithValue {
			Token token;
			int index;
		};

		std::vector<TokenWithValue> tokens;
		std::vector<TokenValue> values;
	};

	/// <summary>
	/// トークナイザー クラス
	/// </summary>
	/// <typeparam name="Iterator">入力イテレーター</typeparam>
	template <typename Iterator>
	class Tokenizer {
		using MatchResult = std::match_results<Iterator>;

		Iterator it;
		Iterator end;
		struct TokenMap {
			// トークンの文字列または正規表現
			const char* str;
			// トークンの種類
			Token token;

			// 正規表現ハンドラ
			// この値が設定されている場合、 str は正規表現として扱われる。
			int(_stdcall* regex_handler)(const MatchResult&, std::vector<TokenValue>&);

			// マッチした文字列を評価しない正規表現ハンドラ
			static int _stdcall EmptyHandler(const MatchResult&, std::vector<TokenValue>&) {
				return -1;
			}
		};

		std::vector<TokenMap> tokens;

		std::unordered_map<TokenMap*, std::regex> regex_cache;


		std::vector<TokenValue> token_values;

	public:
		/// <summary>
		/// イテレータ範囲からトークナイザーを初期化します。
		/// </summary>
		/// <param name="begin">トークナイズ対象となる入力範囲の開始イテレータ。</param>
		/// <param name="end">トークナイズ対象となる入力範囲の終端イテレータ。</param>
		Tokenizer(Iterator begin, Iterator end) : it(begin), end(end) {

			// トークン値の初期化
			tokens = {
				// 2文字演算子
				// (他の記号より優先してマッチング)
				{ "==", token_op_equal },
				{ "!=", token_op_differ },
				{ "<=", token_op_lesseq },
				{ ">=", token_op_greater },
				{ "&&", token_op_and_and },
				{ "||", token_op_or_or },

				// 1文字演算子
				{ "!", token_op_not },
				{ "<", token_op_less },
				{ ">", token_op_greater },
				{ "+", token_op_add },
				{ "-", token_op_sub },
				{ "*", token_op_mul },
				{ "/", token_op_div },
				{ "=", token_op_assign },

				// 1文字トークン
				{ "(", token_paren_open },
				{ ")", token_paren_close },
				{ "{", token_brace_open },
				{ "}", token_brace_close },
				{ "$", token_dollar },
				{ ",", token_comma },
				{ ";", token_semicolon },

				// キーワード
				{"^if\\b", token_kwd_if, TokenMap::EmptyHandler},
				{"^else\\b", token_kwd_else, TokenMap::EmptyHandler},
				{"^while\\b", token_kwd_while, TokenMap::EmptyHandler},
				{"^continue\\b",token_kwd_continue, TokenMap::EmptyHandler},
				{"^break\\b",token_kwd_break, TokenMap::EmptyHandler},
				{"^return\\b",token_kwd_return, TokenMap::EmptyHandler},
				{"^func\\b", token_kwd_func, TokenMap::EmptyHandler},
				{"^var\\b", token_kwd_var, TokenMap::EmptyHandler},

				// 識別子
				{
					"^[a-zA-Z_][a-zA-Z0-9_]*",
					token_ident,
					[](const MatchResult& match_result, std::vector<TokenValue>& values) -> int {
						int index = (int)values.size();
						values.push_back(StringName(match_result.str()));
						return index;
					}
				},

				// 数値リテラル
				{
					"^([0-9]*[.])?[0-9]+",
					token_literal,
					[](const MatchResult& match_result, std::vector<TokenValue>& values) -> int {
						double val = std::stod(match_result.str());
						int index = (int)values.size();
						values.emplace_back(val);
						return index;
					}
				},
				// 文字列リテラル
				{
					"^\"([^\"]*)\"",
					token_literal,
					[](const MatchResult& match_result, std::vector<TokenValue>& values) -> int {
						std::string str = match_result.str(1); // キャプチャグループ1を取得
						int index = (int)values.size();
						values.emplace_back(EvalValue{ str });
						return index;
					}
				},
			};

		}

		template <typename T = TokenValue>
		const T& GetTokenValue(int index) const {
			if constexpr (std::is_same_v<T, TokenValue>) {
				return token_values[index];
			}
			else {
				return std::get<T>(token_values[index]);
			}
		}

		bool Next(Token& out_token, int& out_value_index) {
			while (it != end && isspace(*it)) {
				++it;
			}
			if (it == end) {
				out_token = token_eof;
				out_value_index = -1;
				return false;
			}
			Token token = token_error;
			size_t token_length = 0;
			int token_index = -1;
			for (auto& pattern : tokens) {
				if (pattern.regex_handler) {
					auto it_r = regex_cache.find(&pattern);
					if (it_r == regex_cache.end()) {
						auto insert_result = regex_cache.insert({ &pattern, std::regex{ pattern.str } });
						it_r = insert_result.first;
					}
					MatchResult match_result;
					if (std::regex_search(it, end, match_result, it_r->second, std::regex_constants::match_continuous)) {

						token = pattern.token;
						token_length = match_result.length();
						token_index = pattern.regex_handler(match_result, token_values);
						break;
					}
				}
				else {
					size_t len = strlen(pattern.str);
					const auto ptr = &*it;
					if (strncmp(ptr, pattern.str, len) == 0) {
						token = pattern.token;
						token_length = len;
						token_index = -1;
						break;
					}
				}
			}
			it += token_length;
			out_token = token;
			out_value_index = token_index;
			return true;
		}
	};
}

#endif // SCRIPTOKENIZER_H_
