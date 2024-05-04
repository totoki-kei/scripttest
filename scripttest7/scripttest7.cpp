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

using variant_list_ptr = std::shared_ptr<variant_list>;
using variant_dictionary_ptr = std::shared_ptr<variant_dictionary>;

using data_variant_base = std::variant<nullptr_t, int, double, std::string, variant_list_ptr, variant_dictionary_ptr>;

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


variant_list_ptr make_variant_list() {
	return std::make_shared<variant_list>();
}
variant_list_ptr make_variant_list(std::initializer_list<variant_list::value_type>&& list) {
	return std::make_shared<variant_list>(list);
}
variant_list_ptr make_variant_list(const variant_list_ptr& list) {
	return list ? std::make_shared<variant_list>(*list) : std::make_shared<variant_list>();
}
variant_list_ptr make_variant_list(variant_list_ptr&& list) {
	return std::move(list);
}
variant_list_ptr make_variant_list(const variant_list& list) {
	return std::make_shared<variant_list>(list);
}
variant_list_ptr make_variant_list(variant_list* list, const bool copy = false) {
	return copy && list ? std::make_shared<variant_list>(*list) : std::shared_ptr<variant_list>(list);
}


variant_dictionary_ptr make_variant_dictionary() {
	return std::make_shared<variant_dictionary>();
}
variant_dictionary_ptr make_variant_dictionary(std::initializer_list<variant_dictionary::value_type>&& list) {
	return std::make_shared<variant_dictionary>(list);
}
variant_dictionary_ptr make_variant_dictionary(const variant_dictionary_ptr& list) {
	return list ? std::make_shared<variant_dictionary>(*list) : std::make_shared<variant_dictionary>();
}
variant_dictionary_ptr make_variant_dictionary(variant_dictionary_ptr&& list) {
	return std::move(list);
}
variant_dictionary_ptr make_variant_dictionary(const variant_dictionary& list) {
	return std::make_shared<variant_dictionary>(list);
}
variant_dictionary_ptr make_variant_dictionary(variant_dictionary* list, const bool copy = false) {
	return copy && list ? std::make_shared<variant_dictionary>(*list) : std::shared_ptr<variant_dictionary>(list);
}


constexpr size_t data_variant_size = sizeof(data_variant);
constexpr size_t data_variant_base_size = sizeof(data_variant_base);


int main() {

	class variant_printer {
		size_t indent_num_;
		std::string indent_;
		std::string prefix_;
		std::string suffix_;
	public:
		explicit variant_printer(size_t indent, std::string_view prefix, std::string_view suffix) : indent_num_(indent), indent_(indent, ' '), prefix_(prefix), suffix_(suffix) {}

		void operator ()(nullptr_t) const {
			std::cout << indent_ << prefix_ << "(nullptr)" << suffix_ << std::endl;
		}

		void operator()(const int i) const {
			std::cout << indent_ << prefix_ << i << suffix_ << std::endl;;
		}

		void operator()(const double n) const {
			std::cout << indent_ << prefix_ << std::fixed << std::setprecision(3) << n << suffix_ << std::endl;;
		}

		void operator()(const std::string& s) const {
			std::cout << indent_ << prefix_ << "'" << s << "'" << suffix_ << std::endl;;
		}

		void operator ()(const variant_list_ptr& pl) const {
			if (pl) {
				if (const auto& v = *pl; v.empty())
				{
					std::cout << indent_ << prefix_ << "[]" << suffix_ << std::endl;
				}
				else {

					std::cout << indent_ << prefix_ << "[" << std::endl;
					for (const auto& e : v) {
						std::visit(variant_printer(indent_num_ + 1, "", ","), e);
					}

					std::cout << indent_ << "]" << suffix_ << std::endl;
				}
			} else
			{
				std::cout << indent_ << prefix_ << "[ (empty list) ]" << suffix_ << std::endl;;
			}
		}

		void operator() (const variant_dictionary_ptr& pd) const {
			if (pd) {
				if (const auto& d = *pd; d.empty())
				{
					std::cout << indent_ << prefix_ << "{}" << suffix_ << std::endl;
				}
				else {
					std::cout << indent_ << prefix_ << "{" << std::endl;
					for (const auto& [key, val] : d) {
						std::visit(variant_printer(indent_num_ + 1, "'" + key + "': ", ","), val);
					}
					std::cout << indent_ << "}" << suffix_ << std::endl;
				}
			} else
			{
				std::cout << indent_ << prefix_ << "{ (empty dictionary) }" << suffix_ << std::endl;;
			}
		}
	};

	std::cout << sizeof(data_variant) << std::endl;

	data_variant vs = make_variant_list({
		{ nullptr },
		{ 10 },
		{ 2.1 },
		{ "hoge"s },
		make_variant_list({
			{11}, {22}, {"hogehoge"}
		}),
		make_variant_dictionary({
			{ "first", 10 },
			{ "second", "0x02"s },
			{ "third", make_variant_list({"this", "is", "a", "pen"})}
		}),
		"\n",
		make_variant_list(),
		make_variant_list(nullptr),
		"\n",
		make_variant_dictionary(),
		make_variant_dictionary(nullptr),
	});

	std::visit(variant_printer{0, "", ""}, vs);

#if 0
	std::fstream fs;
	fs.open(".\\scripttest7.cpp");

	auto lines = split_by_line(std::istreambuf_iterator(fs), std::istreambuf_iterator<char>());

	for (auto& line : lines) {
		std::cout << "<<<" << line << ">>>" << std::endl;
	}
#endif


}




