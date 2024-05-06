#include "script_parser.h"

#define BOOST_SPIRIT_X3_UNICODE
#include <boost/spirit/home/x3.hpp>

namespace x3 = boost::spirit::x3;
namespace fs = boost::fusion;

namespace parser {
	using namespace x3;

	// ruleは今のところ定義のみ Attributeは後で付け加える

	template <typename Context>
	void print(const Context& ctx) {
		x3::_attr(ctx);
	}

	static x3::real_parser<double, x3::strict_real_policies<double>> strict_double;

	x3::rule<struct start_tag> const start;

	x3::rule<struct ident_tag, std::string> const ident;
	x3::rule<struct string_literal_tag, std::string> const string_literal;
	x3::rule<struct literal_tag, ast::Literal> const literal;
	x3::rule<struct param_tag, ast::Param> const param;
	x3::rule<struct annotation_tag> const annotation;

	x3::rule<struct line_label_tag> const line_label;
	x3::rule<struct line_segment_tag> const line_segment;
	x3::rule<struct line_operation_tag> const line_operation;
	x3::rule<struct line_annotation_tag> const line_annotation;

	x3::rule<struct block_annotations_tag> const block_annotations;

	x3::rule<struct comment_tag> const comment;

#define SA(block) ([](auto&& context) block )
#define SB(body) { body }
#define CV (x3::_val(context))
#define CA (x3::_attr(context))

	namespace sa {
#define SA_BEGIN(fn_name, context_name) \
	static auto fn_name () { return [](const auto& context_name){

#define SA_BEGIN2(fn_name, context_name, args) \
	static auto fn_name args { return [=](const auto& context_name){

#define SA_END    }; }

		SA_BEGIN(start_init, ctx)
			_val(ctx) = ast::Root{};
		SA_END;

		SA_BEGIN(start_newline, ctx)
			_val(ctx).lines.push_back(_attr(ctx));
		SA_END;
	}

	auto const start_def
		= x3::eps[ sa::start_init() ]
		>> *(
			(line_label | line_segment | line_operation | block_annotations | line_annotation)
				[ sa::start_newline() ]
			)
		;
	BOOST_SPIRIT_DEFINE(start);

	auto const ident_def
		= (x3::char_('a', 'z') | x3::char_('A', 'Z') | x3::char_("_"))
		>> *(x3::char_('a', 'z') | x3::char_('A', 'Z') | x3::char_("_") | x3::char_('0', '9'))
		;
	BOOST_SPIRIT_DEFINE(ident);

	namespace sa {
		SA_BEGIN2(string_literal_compose, ctx, (const char* surr)) 
			_val(ctx) = surr + std::string{ _attr(ctx) } + surr;
		SA_END;
	}
	auto const string_literal_def
		= lexeme[ lit('"') >> *(char_ - char_('"')) >> lit('"') ][ sa::string_literal_compose("\"")]
		| lexeme[ lit('\'') >> *(char_ - char_('\'')) >> lit('\'') ][ sa::string_literal_compose("'") ]
		//= lexeme[
		//	lit('"')[ SA({ CV = std::string(); }) ] >>
		//		*( (char_('\\') >> char_)[ SA(SB(CV.push_back(fs::at_c<0>(CA)), CV.push_back(fs::at_c<1>(CA));))]
		//		 | (char_ - char_('"'))[ SA({ CV.push_back(CA); }) ]
		//		)
		//		>> lit('"')]
		//| lexeme[
		//	lit('\'')[ SA({ CV = std::string(); }) ] >>
		//		*((char_('\\') >> char_)[ SA({ CV.append(CA); }) ]
		//		 | (char_ - char_('\''))[ SA({ CV.push_back(CA); }) ]
		//		)
		//		>> lit('\'') ]
		;
	BOOST_SPIRIT_DEFINE(string_literal);

	auto const literal_def
		= strict_double			[ SA({ CV = ast::Literal(CA); }) ]
		| x3::int64				[ SA({ CV = ast::Literal(CA); }) ]
		| string_literal		[ SA({ CV = ast::Literal(CA, '"'); }) ]
		| (+x3::char_('-'))		[ SA({ CV = ast::Literal(nullptr, CA); }) ]
		| x3::string("nil")		[ SA({ CV = ast::Literal(nullptr, CA); }) ]
		| x3::string("true")	[ SA({ CV = ast::Literal(true); }) ]
		| x3::string("false")	[ SA({ CV = ast::Literal(false); }) ]
		;
	BOOST_SPIRIT_DEFINE(literal);
	auto const param_def
		= literal
		| ident
		| x3::char_('`') >> (ident % '.')
		| x3::char_("@$^") >> -x3::char_("@$^") >> x3::int_;
	;
	BOOST_SPIRIT_DEFINE(param);
	auto const annotation_def
		= ident >> -('[' >> +ident >> ']') >> *(ident >> -(':' >> ident))
		;
	BOOST_SPIRIT_DEFINE(annotation);

	auto const line_label_def
		= ident >> x3::lit(':') >> x3::eol;
	BOOST_SPIRIT_DEFINE(line_label);
	auto const line_segment_def
		= x3::lit('.') >> ident >> x3::eol;
	BOOST_SPIRIT_DEFINE(line_segment);
	auto const line_operation_def
		= -param >> ident >> *param >> x3::eol;
	BOOST_SPIRIT_DEFINE(line_operation);
	auto const line_annotation_def
		= x3::lit('#') >> annotation >> x3::eol;
	BOOST_SPIRIT_DEFINE(line_annotation);

	auto const block_annotations_def
		= x3::lit("###") >> x3::eol
		>> *(annotation >> x3::eol)
		>> x3::lit("###") >> x3::eol
		;
	BOOST_SPIRIT_DEFINE(block_annotations);

	auto const comment_def
		= ";" >> *(x3::char_ - x3::eol) >> (x3::eol | x3::eoi);
	BOOST_SPIRIT_DEFINE(comment);

	using start_type = decltype(start);
	BOOST_SPIRIT_DECLARE(start_type);
}


template <typename Result>
struct TestPattern {
	std::string src;
	Result result;
	bool succeed;
};

void test_ident() {
	TestPattern<std::string> ident_tests[] = {
		{ "set_radius", "set_radius", true },
		{ "Vector3", "Vector3", true },
		{ "_process", "_process", true },

		{ "200d", "", false },
		{ "日の出", "", false },
	};



	for (auto& p : ident_tests) {
		auto it = p.src.begin();
		std::string result;
		bool succeeded = x3::phrase_parse(it, p.src.end(), parser::ident, x3::space | parser::comment, result);

		_ASSERT(succeeded == p.succeed);
		if (succeeded) {
			_ASSERT(it == p.src.end());
			_ASSERT(result == p.result);
		}
	}
}

void test_string_literal() {
	TestPattern<std::string> patterns[]{
		{ R"("1")" , "1", true },
		{ R"("")" , "", true },
		{ R"("aaa bb ccc")" , "aaa bb ccc", true },
		{ R"("sub ' string")" , "sub ' string", true },

		{ R"('1')" , "1", true },
		{ R"('')" , "", true },
		{ R"('aaa bb ccc')" , "aaa bb ccc", true },
		{ R"('sub " string')" , "sub \" string", true },
	};

	for (auto& p : patterns) {
		auto it = p.src.begin();
		std::string result;
		bool succeeded = x3::phrase_parse(it, p.src.end(), parser::string_literal, x3::space | parser::comment, result);

		_ASSERT(succeeded == p.succeed);
		if (succeeded) {
			_ASSERT(it == p.src.end());
			_ASSERT(result == p.result);
		}
	}

}

void test_literal() {
	TestPattern<ast::Literal> patterns[] = {
		{ "10", ast::Literal(10LL), true },
		{ "16777216", ast::Literal(16777216LL), true },
		{ "-3.1", ast::Literal(-3.1), true },
		{ "1.1e11", ast::Literal(1.1e11), true },
		{ "\"a b c\"", ast::Literal("a b c", '"'), true },
		//{ "'X Y Z'", ast::Literal("X Y Z", '\''), true },
		//{ "'日本語文字列'", ast::Literal("日本語文字列", '\''), true },
		//{ "'🎐🎐🎐'", ast::Literal("🎐🎐🎐", '\''), true },
		{ "-", ast::Literal(nullptr, "-"), true },
		{ "--", ast::Literal(nullptr, "--"), true },
		{ "---", ast::Literal(nullptr, "---"), true },
		{ "----", ast::Literal(nullptr, "----"), true },
		{ "nil", ast::Literal(nullptr, "nil"), true },
		{ "false", ast::Literal(false), true },
		{ "true", ast::Literal(true), true },
	};

	for (auto& p : patterns) {
		auto it = p.src.begin();
		ast::Literal result;
		bool succeeded = x3::phrase_parse(it, p.src.end(), parser::literal, x3::space | parser::comment, result);

		_ASSERT(succeeded == p.succeed);
		if (succeeded) {
			_ASSERT(it == p.src.end());
			_ASSERT(result == p.result);
		}
	}
}


void test_script_parser() {
	test_ident();
	test_string_literal();
	test_literal();
}


std::shared_ptr<Entity> make_entity(const SourceLine* source_line, std::vector<std::string>& tokens) {
	auto detail = std::make_unique<OperationEntityDetail>();

	return std::make_shared<Entity>(source_line, std::move(detail));
}

Entity::Entity(const SourceLine* line_info)
	: line_info(line_info) {
}

Entity::Entity(const SourceLine* line_info, std::unique_ptr<EntityDetail>&& detail)
	: line_info(line_info), detail(std::move(detail)) {
}
