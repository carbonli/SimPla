/**
 * @file config_parser.cpp
 *
 *  Created on: 2014年11月24日
 *      Author: salmon
 */

#include "config_parser.h"
#include "log.h"
#include "parse_command_line.h"

namespace simpla
{

ConfigParser::ConfigParser()
{

}

ConfigParser::~ConfigParser()
{
}
void ConfigParser::init(int argc, char ** argv)
{
	m_lua_object_.init();

	if (argc <= 1)
	{
		m_kv_map_["SHOW_HELP"] = "true";
	}

	std::string lua_file = argv[0];
	lua_file += ".lua";

	std::string lua_cmd = "";

	simpla::parse_cmd_line(argc, argv,
			[&](std::string const & opt,std::string const & value)->int
			{
				if(opt=="i"||opt=="input")
				{
					lua_file=value;
				}
				else if(opt=="e"|| opt=="execute")
				{
					lua_cmd=value;
				}
				else if (opt=="t"|| opt=="test")
				{
					m_kv_map_["JUST_A_TEST"] = "true";

					return TERMINATE;
				}
				else if (opt=="h"|| opt=="help")
				{
					SHOW_OPTIONS("-t,--test","only test configure file");
					SHOW_OPTIONS("-i,--input <STRING>","input configure file");
					SHOW_OPTIONS("-e,--execute <STRING>","execute Lua script as configuration");

					m_kv_map_["JUST_A_TEST"] = "true";
					m_kv_map_["SHOW_HELP"] = "true";

					return TERMINATE;
				}
				else
				{
					m_kv_map_[opt]=value;
				}
				return CONTINUE;

			}

			);

	m_lua_object_.parse_file(lua_file);
	m_lua_object_.parse_string(lua_cmd);

}
}  // namespace simpla
