/*
 * simpla.cpp
 *
 * \date  2013-11-13
 *      \author  salmon
 */

#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>

#include "../src/io/data_stream.h"
#include "../src/simpla_defs.h"
#include "../src/utilities/log.h"
#include "../src/utilities/lua_state.h"
#include "../src/utilities/parse_command_line.h"
#include "../src/utilities/utilities.h"
#include "../src/parallel/message_comm.h"

#include "contexts/context_factory.h"

using namespace simpla;

int main(int argc, char **argv)
{

	LOGGER.init(argc, argv);
	GLOBAL_COMM.init(argc,argv);
	GLOBAL_DATA_STREAM.init(argc,argv);

	LOGGER << "Register contexts." << std::endl;

	auto context_factory = RegisterContext();

	LuaObject dict;

	std::string context_type = "";

	size_t num_of_step = 10;

	size_t record_stride = 1;

	bool just_a_test = false;

	ParseCmdLine(argc, argv, [&](std::string const & opt,std::string const & value)->int
	{
		if(opt=="n"||opt=="num_of_step")
		{
			num_of_step =ToValue<size_t>(value);
		}
		else if(opt=="s"||opt=="record_stride")
		{
			record_stride =ToValue<size_t>(value);
		}
		else if(opt=="i"||opt=="input")
		{
			dict.ParseFile(value);
		}
		else if(opt=="c"|| opt=="command")
		{
			dict.ParseString(value);
		}
		else if(opt=="g"|| opt=="generator")
		{
			INFORM
			<< ShowCopyRight() << std::endl
			<< "Too lazy to implemented it\n"<< std::endl;
			TheEnd(1);
		}
		else if( opt=="context")
		{
			context_type =ToValue<std::string>(value);
		}
		else if(opt=="t")
		{
			just_a_test=true;
		}
		else if(opt=="V")
		{
			INFORM<<ShowShortVersion()<< std::endl;
			TheEnd(0);
		}

		else if(opt=="version")
		{
			INFORM<<ShowVersion()<< std::endl;
			TheEnd(0);
		}
		else if(opt=="help")
		{
			INFORM
			<< ShowCopyRight() << std::endl

			<< " avaible contexts ["<<context_factory.size() <<"]  :"<<std::endl

			<< context_factory

			<< std::endl;

			TheEnd(0);
		}
//				else
//				{
//					INFORM
//					<< ShowCopyRight() << std::endl
//					<<
//					" -h        \t print this information\n"
//					" -n<NUM>   \t number of steps\n"
//					" -s<NUM>   \t recorder per <NUM> steps\n"
//					" -o<STRING>\t output directory\n"
//					" -i<STRING>\t configure file \n"
//					" -c,--config <STRING>\t Lua script passed in as string \n"
//					" -t        \t only read and parse input file, but do not process  \n"
//					" -g,--generator   \t generator a demo input script file \n"
//					" -v<NUM>   \t verbose  \n"
//					" -V        \t print version  \n"
//					" -q        \t quiet mode, standard out  \n"
//					;
//					TheEnd(0);
//				}
		    return CONTINUE;

	    }

	    );

	if (context_type == "" && dict["Model"]["Type"])
	{
		context_type = dict["Model"]["Type"].template as<std::string>();
	}

	INFORM << SIMPLA_LOGO;

	LOGGER << "Parse Command Line." << DONE;

	if (!dict)
	{
		LOGGER << "Nothing to do !!";
		TheEnd(-1);
	}
	INFORM << SINGLELINE;

	LOGGER << "Pre-Process" << START;

	std::shared_ptr<ContextBase> ctx;

	// Preprocess    ====================================

	{

		ctx = context_factory.create(context_type, dict);

		if (ctx == nullptr)
		{
			INFORM << "Configure fail!" << std::endl;

			TheEnd(-2);
		}
		else
		{
			ctx->save("/Input/");
			INFORM << std::endl << *ctx;
		}
	}

	// Main Loop ============================================

	LOGGER << "Process " << START;

	TheStart();

	if (just_a_test)
	{
		LOGGER << "Just test configure files";
	}
	else
	{
		GLOBAL_DATA_STREAM.set_property< int>("Cache Depth", 20);

		GLOBAL_DATA_STREAM.set_property("Force Record Storage",true);
		GLOBAL_DATA_STREAM.set_property("Force Write Cache",true);
		ctx->save("/Save/" );

		for (int i = 0; i < num_of_step; ++i)
		{
			LOGGER << "STEP: " << i;

			ctx->next_timestep();

			if (i % record_stride == 0)
			{
				ctx->save("/Save/" );
			}
		}
		GLOBAL_DATA_STREAM.command("Flush");
		GLOBAL_DATA_STREAM.set_property("Force Write Cache",false);
		GLOBAL_DATA_STREAM.set_property("Force Record Storage",false);
	}
	LOGGER << "Process" << DONE;

	INFORM << SINGLELINE;

	LOGGER << "Post-Process" << START;

	ctx->save("/OutPut/");

	INFORM << "OutPut Path:" << GLOBAL_DATA_STREAM.pwd();

	LOGGER << "Post-Process" << DONE;

	INFORM << SINGLELINE;
	GLOBAL_DATA_STREAM.close();
	GLOBAL_COMM.close();
	TheEnd();

}
