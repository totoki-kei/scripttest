// scripttest7.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <boost/functional/hash.hpp>


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

struct string_with_hash
{
	friend std::size_t hash_value(const string_with_hash& obj)
	{
		return 0x79694BFC ^ obj.hash;
	}

	friend bool operator==(const string_with_hash& lhs, const string_with_hash& rhs)
	{
		return lhs.hash == rhs.hash
			&& lhs.str == rhs.str;
	}

	friend bool operator!=(const string_with_hash& lhs, const string_with_hash& rhs)
	{
		return !(lhs == rhs);
	}

	string_with_hash(const string_with_hash& other) = default;

	string_with_hash(string_with_hash&& other) noexcept
		: hash(other.hash),
		  str(std::move(other.str))
	{
	}

	string_with_hash& operator=(const string_with_hash& other)
	{
		if (this == &other)
			return *this;
		hash = other.hash;
		str = other.str;
		return *this;
	}

	string_with_hash& operator=(string_with_hash&& other) noexcept
	{
		if (this == &other)
			return *this;
		hash = other.hash;
		str = std::move(other.str);
		return *this;
	}

	explicit string_with_hash(const std::string& s)
		: hash(std::hash<std::string>()(s)), str(s)
	{}

	size_t hash;
	std::string str;
};

class operation
{
public:
	std::string op;

};

class execution_status
{
public:
	std::unordered_map<string_with_hash, double> variables;

};

class code_cache
{
	
};

class code_cache_map
{
	
};

void excecute_line(int line_number, const std::string& line, execution_status& stat, code_cache_map& code_cache)
{
	
}


int main()
{
#if 0
  	std::fstream fs;
	fs.open(".\\scripttest7.cpp");

	auto lines = split_by_line(std::istreambuf_iterator(fs), std::istreambuf_iterator<char>());

	for(auto& line : lines)
	{
		std::cout << "<<<" << line << ">>>" << std::endl;
	}
#endif




}

