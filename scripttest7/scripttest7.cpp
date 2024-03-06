// scripttest7.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include <iomanip>
#include <iostream>
#include <unordered_map>
#include <variant>
#include <vector>

using namespace std::string_literals;
using namespace std::string_view_literals;

template <typename Iter>
std::vector<std::string> split_by_line(Iter const whole_begin, Iter const whole_end) {
	std::vector<std::string> ret;

	std::string line;

	for (auto it = whole_begin; it != whole_end; ++it) {
		if (*it == '\n') {
			// 末尾のコントロール空白文字(改行2種)を削除
			while (std::ispunct(line.back()) && std::isspace(line.back())) line.pop_back();
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


class operation {
public:
	std::string op;

};

struct data_variant;
using variant_list = std::vector<data_variant>;
using variant_dictionary = std::unordered_map<std::string, data_variant>;
using data_variant_base = std::variant<nullptr_t, int, double, std::string, variant_list, variant_dictionary>;

struct data_variant : data_variant_base {
	// inherit base type constructor
	using data_variant_base::data_variant_base;
	// inherit base type assignment operator
	using data_variant_base::operator=;

	data_variant(const data_variant& other) = default;
	data_variant(data_variant&& other) noexcept : data_variant_base(std::move(other)) {}

	~data_variant() = default;

	data_variant& operator=(const data_variant& other) {
		if (this == &other)
			return *this;
		data_variant_base::operator =(other);
		return *this;
	}

	data_variant& operator=(data_variant&& other) noexcept {
		if (this == &other)
			return *this;
		data_variant_base::operator =(std::move(other));
		return *this;
	}

};



class execution_status {
public:
	variant_list variables;
	std::unordered_map<std::string, size_t> variable_name_table;

	variant_list data_stack;

};

class code_cache {

};

void excecute_line(const std::string& line, execution_status& stat, code_cache* out_code_cache) {

}

void excecute_line(const code_cache& code_cache, execution_status& stat) {

}





int main() {

	class variant_printer {
	public:
		void operator ()(nullptr_t) const {
			std::cout << "(nullptr)";
		}

		void operator()(const int i) const {
			std::cout << i;
		}

		void operator()(const double n) const {
			std::cout << std::fixed << std::setprecision(3) << n;
		}

		void operator()(const std::string& s) const {
			std::cout << "'" << s << "'";
		}

		void operator ()(const variant_list& v) {
			const char* sep = nullptr;

			std::cout << "[ ";
			for (const auto& e : v) {
				if (sep) std::cout << sep;
				std::visit(*this, e);

				sep = ", ";
			}
			std::cout << " ]";
		}

		void operator() (const variant_dictionary& d) {
			const char* sep = nullptr;

			std::cout << "{ ";
			for (const auto& [key, val] : d) {
				if (sep) std::cout << sep;
				std::cout << "'" << key << "': ";
				std::visit(*this, val);

				sep = ", ";
			}
			std::cout << " }";
		}
	};

	std::cout << sizeof(data_variant) << std::endl;

	data_variant vs = variant_list
	{
		{ nullptr },
		{ 10 },
		{ 2.1 },
		{ "hoge"s },
		variant_list{
			{11}, {22}, {"hogehoge"}
		},
		variant_dictionary{
			{ "first", 10 },
			{ "second", "0x02"s },
			{ "third", variant_list{"this", "is", "a", "pen"}}
		},
	};

	std::visit(variant_printer{}, vs);

#if 0
	std::fstream fs;
	fs.open(".\\scripttest7.cpp");

	auto lines = split_by_line(std::istreambuf_iterator(fs), std::istreambuf_iterator<char>());

	for (auto& line : lines) {
		std::cout << "<<<" << line << ">>>" << std::endl;
	}
#endif


}




