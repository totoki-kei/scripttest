#include "script_parser.h"

#define BOOST_SPIRIT_X3_UNICODE
#include <boost/spirit/home/x3.hpp>



namespace x3 = boost::spirit::x3;
namespace parser {
	using namespace x3;

	// ruleÇÕç°ÇÃÇ∆Ç±ÇÎíËã`ÇÃÇ› AttributeÇÕå„Ç≈ïtÇØâ¡Ç¶ÇÈ

	template <typename Context>
	void print(const Context& ctx) {
		x3::_attr(ctx);
	}

	x3::rule<struct start_tag> const start;

	x3::rule<struct ident_tag, std::string> const ident;
	x3::rule<struct literal_tag, ast::Literal> const literal;
	x3::rule<struct param_tag, ast::Param> const param;
	x3::rule<struct annotation_tag> const annotation;

	x3::rule<struct line_label_tag> const line_label;
	x3::rule<struct line_segment_tag> const line_segment;
	x3::rule<struct line_operation_tag> const line_operation;
	x3::rule<struct line_annotation_tag> const line_annotation;

	x3::rule<struct block_annotations_tag> const block_annotations;

	x3::rule<struct comment_tag> const comment;

#define SA(block) ([](auto& context) ## block )

	auto const start_def
		= x3::eps[ SA({ x3::_val(context) = ast::Root(); }) ]
		>> *(
			(line_label | line_segment | line_operation | block_annotations | line_annotation)
				[ SA({ x3::_val(context).lines.push_back(x3::_attr(context)); }) ]
			)
		;
	BOOST_SPIRIT_DEFINE(start);

	auto const ident_def
		= (x3::char_('a', 'z') | x3::char_('A', 'Z') | x3::char_("_"))
		>> *(x3::char_('a', 'z') | x3::char_('A', 'Z') | x3::char_("_") | x3::char_('0', '9'))
		;
	BOOST_SPIRIT_DEFINE(ident);
	auto const literal_def
		= x3::double_
			[ SA({ _val(context) = ast::Literal(_attr(context)); }) ]
		| x3::int64
			[ SA({ _val(context) = ast::Literal(_attr(context)); }) ]
		| x3::raw[ ('"' >> *('\\' >> x3::char_ | ~x3::char_('"')) >> '"') ]
			[ SA({ _val(context) = ast::Literal(_attr(context), '"');})]
		| x3::raw[ ('\'' >> *('\\' >> x3::char_ | ~x3::char_('\'')) >> '\'') ]
			[ SA({ _val(context) = ast::Literal(_attr(context), '\''); }) ]
		| (+x3::char_('-'))
			[ SA({ _val(context) = ast::Literal(nullptr, _attr(context)); }) ]
		| x3::string("nil")
			[ SA({ _val(context) = ast::Literal(nullptr, _attr(context)); }) ]
		| x3::string("true")
			[ SA({ _val(context) = ast::Literal(true); }) ]
		| x3::string("false")
			[ SA({ _val(context) = ast::Literal(false); }) ]
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


std::shared_ptr<Entity> make_entity(const SourceLine* source_line, std::vector<std::string>& tokens) {
	auto detail = std::make_unique<OperationEntityDetail>();

	return std::make_shared<Entity>(source_line, std::move(detail));
}

inline Entity::Entity(const SourceLine* line_info)
	: line_info(line_info) {
}

inline Entity::Entity(const SourceLine* line_info, std::unique_ptr<EntityDetail>&& detail)
	: line_info(line_info), detail(std::move(detail)) {
}
