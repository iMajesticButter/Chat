#pragma once
#include "Commands.h"

namespace aCmd {

	//op a user
	bool op		(SOCKET, chatcmd::ParsedMSG, Namelist*, std::list<SOCKET>&, SOCKET&);

	//deop a user
	bool deop	(SOCKET, chatcmd::ParsedMSG, Namelist*, std::list<SOCKET>&, SOCKET&);

	//transfer admin to a user
	bool admin	(SOCKET, chatcmd::ParsedMSG, Namelist*, std::list<SOCKET>&, SOCKET&);

}