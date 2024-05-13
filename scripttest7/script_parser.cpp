#include "script_parser.h"


#define BOOST_SPIRIT_X3_UNICODE
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/support/iterators/line_pos_iterator.hpp>

#include <boost/fusion/include/for_each.hpp>

namespace x3 = boost::spirit::x3;
namespace fs = boost::fusion;

/// <summary>
/// line_pos_iterator
/// https://www.boost.org/doc/libs/1_85_0/libs/spirit/doc/html/spirit/support/line_pos_iterator.html
/// - get_line() で行番号
/// - get_column() で現在の列位置
/// </summary>
using SourceIterator = boost::spirit::line_pos_iterator<std::string::const_iterator>;

namespace parser {
		// ruleは今のところ定義のみ Attributeは後で付け加える

	template <typename Context>
	void print(const Context& ctx) {
		x3::_attr(ctx);
	}

	static x3::real_parser<double, x3::strict_real_policies<double>> strict_double;

	x3::rule<struct start_tag> const start;

	x3::rule<struct ident_tag, std::string> const ident;
	x3::rule<struct ident2_tag, std::string> const ident2;
	x3::rule<struct string_literal_tag, std::string> const string_literal;
	x3::rule<struct literal_tag, ast::Literal> const literal;
	x3::rule<struct param_tag, ast::Param> const param;
	x3::rule<struct annotation_tag, ast::Annotation> const annotation;

	x3::rule<struct line_label_tag, ast::LabelLine> const line_label;
	x3::rule<struct line_segment_tag, ast::SegmentLine> const line_segment;
	x3::rule<struct line_operation_tag, ast::OperationLine> const line_operation;
	x3::rule<struct line_annotation_tag, ast::AnnotationLine> const line_annotation;

	x3::rule<struct block_annotations_tag, ast::AnnotationBlock> const block_annotations;

	x3::rule<struct comment_tag> const comment;


	namespace sa {
		using namespace x3;

		template <typename T>
		static std::string stringfy(const T& t);

		namespace detail {
			template <typename T>
			struct Stringfy {};

			template <>
			struct Stringfy<char> {
				auto operator ()(const char& c) -> std::string {
					return std::string(1, c);
				}
			};

			template <size_t N>
			struct Stringfy<char(&)[N]> {
				auto operator ()(char(&cs)[N]) -> std::string {
					return std::string(cs, N);
				}
			};

			template <>
			struct Stringfy<std::string> {
				auto operator ()(const std::string& s) -> std::string {
					return s;
				}
			};

			template <typename T>
			struct Stringfy<std::vector<T>> {
				auto operator ()(const std::vector<T>& v) -> std::string {
					std::string ret;
					for (auto m : v) ret += stringfy(m);
					return ret;
				}
			};

			template <typename... Ts>
			struct Stringfy<boost::variant<Ts...>> {
				auto operator()(const boost::variant<Ts...>& va)->std::string {
					return va.apply_visitor(stringfy);
				}
			};

			template <typename... Ts>
			struct Stringfy<boost::fusion::deque<Ts...>> {
				auto operator ()(const boost::fusion::deque<Ts...>& fs) -> std::string {
					std::string ret;
					boost::fusion::for_each(fs, [&](const auto& f) {
						ret += stringfy(f);
					});
					return ret;
				}
			};

			template <typename T>
			struct Stringfy<boost::optional<T>> {
				auto operator() (const boost::optional<T>& opt) -> std::string {
					return opt.has_value ? stringfy(*opt) : std::string{};
				}
			};
		}

		template <typename T>
		static std::string stringfy(const T& t) {
			return detail::Stringfy<T>{}(t);
		}



#define OPEN_BR  {
#define CLOSE_BR }
#define DEFINE_SA(fn_name, context_name)        static auto fn_name ()   OPEN_BR return [ ](const auto& context_name) -> void OPEN_BR
#define DEFINE_SA2(fn_name, context_name, args) static auto fn_name args OPEN_BR return [=](const auto& context_name) -> void OPEN_BR
#define DEFINE_SA_END                           CLOSE_BR ; CLOSE_BR

#define INLINE_SA(context_name)     ([ ](const auto& context_name) -> void
#define INLINE_SA_END               )

		DEFINE_SA(debug_print_type, ctx) {
			puts("--------");
			printf("typeof(_val) : '%s'\n", typeid(_val(ctx)).name());
			printf("typeof(_attr): '%s'\n", typeid(_attr(ctx)).name());
		} DEFINE_SA_END;

		DEFINE_SA(start_init, ctx) {
			_val(ctx) = ast::Root{};
		} DEFINE_SA_END;

		DEFINE_SA(start_newline, ctx) {
			_val(ctx).lines.push_back(_attr(ctx));
		} DEFINE_SA_END;

		DEFINE_SA(ident2_compose, ctx) {
			const char* separator = nullptr;
			const auto& parts = _attr(ctx);
			std::string ret;
			for (const auto& part : parts) {
				if (separator) ret.append(separator);
				ret.append(stringfy(part));
				separator = ".";
			}
			_val(ctx) = ret;
		} DEFINE_SA_END;

		DEFINE_SA2(string_literal_compose, ctx, (const char* surr)) {
			_val(ctx) = surr + stringfy(_attr(ctx)) + surr;
		} DEFINE_SA_END;

		DEFINE_SA(literal_compose_value, ctx) {
			_val(ctx) = ast::Literal(_attr(ctx));
		} DEFINE_SA_END;

		DEFINE_SA(literal_compose_nil, ctx) {
			_val(ctx) = ast::Literal(nullptr, _attr(ctx));
		} DEFINE_SA_END;

		DEFINE_SA2(literal_compose_bool, ctx, (bool val)) {
			_val(ctx) = ast::Literal(val);
		} DEFINE_SA_END;

		DEFINE_SA(param_literal, ctx) {
			_val(ctx) = ast::Param(_attr(ctx));
		} DEFINE_SA_END;

		DEFINE_SA2(param_symbol, ctx, (bool is_extern)) {
			_val(ctx) = ast::Param(_attr(ctx), is_extern);
		} DEFINE_SA_END;

		DEFINE_SA(param_mem, ctx) {
			const auto& attr = _attr(ctx);
			_val(ctx) = ast::Param(
				fs::at_c<2>(attr).value_or(-1),
				fs::at_c<0>(attr),
				fs::at_c<1>(attr).value_or('\0')
			);
		} DEFINE_SA_END;

		DEFINE_SA(annotation_compose, ctx) {
			const auto& attr = _attr(ctx);

			const auto& name = fs::at_c<0>(attr);
			const auto& flags = fs::at_c<1>(attr).value_or(std::vector<std::string>());
			const auto& ids = fs::at_c<2>(attr);

			std::vector<std::pair<std::string, std::string>> id_types;
			std::transform(ids.begin(), ids.end(), std::back_inserter(id_types), [&](const auto& id) {
				return std::make_pair(fs::at_c<0>(id), fs::at_c<1>(id).value_or(""));
			});

			_val(ctx) = ast::Annotation{ name, flags, id_types };
		} DEFINE_SA_END;

		DEFINE_SA(line_label_compose, ctx) {
			_val(ctx) = new ast::LabelLine{ _attr(ctx) };
		} DEFINE_SA_END;

		DEFINE_SA(line_segment_compose, ctx) {
			_val(ctx) = new ast::SegmentLine{ _attr(ctx) };
		} DEFINE_SA_END;
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
		= x3::lexeme[
			//(x3::char_('a', 'z') | x3::char_('A', 'Z') | x3::char_("_"))
			//	>> *(x3::char_('a', 'z') | x3::char_('A', 'Z') | x3::char_("_") | x3::char_('0', '9'))
			    (x3::alpha | x3::char_("_"))
			>> *(x3::alnum | x3::char_("_"))
		]
		;
	BOOST_SPIRIT_DEFINE(ident);

	auto const ident2_def
		= (ident % '.')[sa::ident2_compose()];
		;
	BOOST_SPIRIT_DEFINE(ident2);

	auto const string_literal_def
		= x3::lexeme[ '"' >> *(x3::char_ - x3::lit('"')) >> '"']
		| x3::lexeme[ '\'' >> *(x3::char_ - x3::lit('\'')) >> '\'' ]
		;
	BOOST_SPIRIT_DEFINE(string_literal);

	auto const literal_def
		= strict_double			[ sa::literal_compose_value() ]
		| x3::int64				[ sa::literal_compose_value() ]
		| string_literal		[ sa::literal_compose_value() ]
		| (+x3::char_('-'))		[ sa::literal_compose_nil() ]
		| x3::string("nil")		[ sa::literal_compose_nil() ]
		| x3::string("true")	[ sa::literal_compose_bool(true) ]
		| x3::string("false")	[ sa::literal_compose_bool(false) ]
		;
	BOOST_SPIRIT_DEFINE(literal);

	auto const param_def
		= literal[sa::param_literal()]
		| ident2[sa::param_symbol(false)]
		| (x3::lit('`') >> ident2)[sa::param_symbol(true)]
		| (x3::char_("%@$^") >> -x3::char_("%@$^") >> -x3::int_)[sa::param_mem()]
		;
	BOOST_SPIRIT_DEFINE(param);

	auto const annotation_def
		= (ident >> -('[' >> +ident >> ']') >> *(ident >> -(':' >> ident)))[sa::annotation_compose()]
		;
	BOOST_SPIRIT_DEFINE(annotation);

	/*********************************************************************************************/

	auto const line_label_def
		= (ident >> x3::lit(':') >> x3::eol)[sa::line_label_compose()];
	BOOST_SPIRIT_DEFINE(line_label);

	auto const line_segment_def
		= (x3::lit('.') >> ident >> x3::eol)[sa::line_segment_compose()];
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
	bool succeed;
	
	std::optional<Result> result;
	std::function<bool(const Result&)> pred;

	TestPattern(const std::string& src, Result result, bool succeed)
		: src(src), result(result), succeed(succeed) {
	}
	template <typename Pred>
	TestPattern(const std::string& src, Pred fn)
		: src(src), result(), pred(fn), succeed(true) {}
};

template <typename ID, typename Attribute>
void do_test(const x3::rule<ID, Attribute>& rule, std::initializer_list<TestPattern<Attribute>>&& patterns) {
	for (auto& p : patterns) {
		auto it = p.src.begin();
		Attribute result;
		bool succeeded = x3::phrase_parse(it, p.src.end(), rule, x3::space | parser::comment, result);

		_ASSERT(succeeded == p.succeed);
		if (succeeded) {
			_ASSERT(it == p.src.end());
			if (p.result) {
				_ASSERT(result == p.result);
			}
			else if (p.pred) {
				_ASSERT(p.pred(result));
			}
		}
	}
}

void test_ident() {
	puts(__FUNCTION__);
	do_test(parser::ident, {
		{ "set_radius", "set_radius", true },
		{ "Vector3", "Vector3", true },
		{ "_process", "_process", true },

		{ "200d", "", false },
		{ "日の出", "", false },
	});
}

void test_string_literal() {
	puts(__FUNCTION__);
	do_test(parser::string_literal,  {
		{ R"("1")" , "1", true },
		{ R"("")" , "", true },
		{ R"("aaa bb ccc")" , "aaa bb ccc", true },
		{ R"("sub ' string")" , "sub ' string", true },

		{ R"('1')", "1", true },
		{ R"('')" , "", true },
		{ R"('aaa bb ccc')" , "aaa bb ccc", true },
		{ R"('sub " string')" , "sub \" string", true },
	});
}

void test_literal() {
	puts(__FUNCTION__);
	do_test(parser::literal, {
		{ "10", ast::Literal(10LL), true },
		{ "16777216", ast::Literal(16777216LL), true },
		{ "-3.1", ast::Literal(-3.1), true },
		{ "1.1e11", ast::Literal(1.1e11), true },
		{ R"("a b c")", ast::Literal("a b c"), true },
		{ "'X Y Z'", ast::Literal("X Y Z"), true },
		{ "'日本語文字列'", ast::Literal("日本語文字列"), true },
		{ "'🎐🎐🎐'", ast::Literal("🎐🎐🎐"), true },
		{ "-", ast::Literal(nullptr, "-"), true },
		{ "--", ast::Literal(nullptr, "--"), true },
		{ "---", ast::Literal(nullptr, "---"), true },
		{ "----", ast::Literal(nullptr, "----"), true },
		{ "nil", ast::Literal(nullptr, "nil"), true },
		{ "false", ast::Literal(false), true },
		{ "true", ast::Literal(true), true },
	});

}

void test_param() {
	puts(__FUNCTION__);
	do_test(parser::param, {
		{ "10", ast::Param(ast::Literal(10LL)), true },
		{ "16777216", ast::Param(ast::Literal(16777216LL)), true },
		{ "-3.1", ast::Param(ast::Literal(-3.1)), true },
		{ "1.1e11", ast::Param(ast::Literal(1.1e11)), true },
		{ R"("a b c")", ast::Param(ast::Literal("a b c")), true },
		{ "'X Y Z'", ast::Param(ast::Literal("X Y Z")), true },
		{ "'日本語文字列'", ast::Param(ast::Literal("日本語文字列")), true },
		{ "'🎐🎐🎐'", ast::Param(ast::Literal("🎐🎐🎐")), true },
		{ "-", ast::Param(ast::Literal(nullptr, "-")), true },
		{ "--", ast::Param(ast::Literal(nullptr, "--")), true },
		{ "---", ast::Param(ast::Literal(nullptr, "---")), true },
		{ "----", ast::Param(ast::Literal(nullptr, "----")), true },
		{ "nil", ast::Param(ast::Literal(nullptr, "nil")), true },
		{ "false", ast::Param(ast::Literal(false)), true },
		{ "true", ast::Param(ast::Literal(true)), true },

		{ "i", ast::Param("i", false), true },
		{ "ClassName", ast::Param("ClassName", false), true },

		{ "`Object.to_string", ast::Param("Object.to_string", true), true },

		{ "@0", ast::Param(0, '@', '\0'), true },
		{ "$1", ast::Param(1, '$', '\0'), true },
		{ "^2000", ast::Param(2000, '^', '\0'), true },
		{ "@@", ast::Param(-1, '@', '@'), true },
		{ "^^", ast::Param(-1, '^', '^'), true },
		{ "$@10", ast::Param(10, '$', '@'), true },
	});
}

void test_annotation() {
	puts(__FUNCTION__);
	do_test(parser::annotation, {
		{ "func get_radius:float", ast::Annotation{"func", {}, { {"get_radius", "float" }}}, true},
		{ "func set_radius:void r:float", ast::Annotation{"func", {}, {{"set_radius", "void"}, {"r", "float" }}}, true},
		{ "func get_color:color", ast::Annotation{"func", {}, {{"get_color", "color"}}}, true },
		{ "func set_color c:color", ast::Annotation{"func", {}, {{"set_color", ""}, {"c", "color"}}}, true},
		{ "prop radius:float get_radius set_radius", ast::Annotation{ "prop", {}, { {"radius", "float"}, {"get_radius", ""}, {"set_radius", ""}}}, true},
		{ "prop color:color get_color set_color", ast::Annotation{ "prop", {}, { {"color", "color"}, {"get_color", ""}, {"set_color", ""}}}, true },
		{ "func [virtual override] _draw:void delta:float", ast::Annotation{ "func", {"virtual", "override"}, {{"_draw", "void"}, {"delta", "float"}}}, true},
	});
}



void test_script_parser() {
	test_ident();
	test_string_literal();
	test_literal();
	test_param();
	test_annotation();
	// -------------------
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
