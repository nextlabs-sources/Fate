#pragma once

enum ParserResult
{
	PARSER_INPROCESS,
	PARSER_ALLOW,
	PARSER_DENY,
	PARSER_BUF_MODIFIED // only content can be changed, the size of the buffer can not be changed
};