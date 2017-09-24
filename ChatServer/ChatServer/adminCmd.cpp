#include "adminCmd.h"

namespace aCmd {



	//admin only commands
	//op a user
	bool op(SOCKET sock, chatcmd::ParsedMSG parsed, Namelist* names, std::list<SOCKET>& ops, SOCKET& admin) {
		if (sock == admin && parsed.cmdmsg == "/op" && parsed.msg.length() > 4) {
			std::string str = parsed.msg.substr(4, parsed.msg.length());
			if (names->contains(str)) {
				std::string msg = "op'd: " + str;
				ops.push_front(names->getSock(str));
				send(sock, msg.c_str(), msg.length(), 0);
				msg = "SERVER: You have been op'd!";
				send(names->getSock(str), msg.c_str(), msg.length(), 0);
			}
			return true;
		}
		return false;
	}

	//deop a user
	bool deop(SOCKET sock, chatcmd::ParsedMSG parsed, Namelist* names, std::list<SOCKET>& ops, SOCKET& admin) {
		if (sock == admin && parsed.cmdmsg == "/deop" && parsed.msg.length() > 6) {
			std::string str = parsed.msg.substr(6, parsed.msg.length());
			if (names->contains(str)) {
				std::string msg = "deop'd: " + str;
				ops.remove(names->getSock(str));
				send(sock, msg.c_str(), msg.length(), 0);
				msg = "SERVER: You have been deop'd! What did you do wrong??";
				send(names->getSock(str), msg.c_str(), msg.length(), 0);
			}
			return true;
		}
		return false;
	}

	//transfer admin to a user
	bool admin(SOCKET sock, chatcmd::ParsedMSG parsed, Namelist* names, std::list<SOCKET>& ops, SOCKET& admin) {
		if (sock == admin && parsed.cmdmsg == "/admin" && parsed.msg.length() > 7) {
			std::string str = parsed.msg.substr(7, parsed.msg.length());
			if (names->contains(str)) {
				std::string msg = "admin transfered to: " + str + " you are now a normal op";
				ops.remove(names->getSock(str));
				ops.push_front(sock);
				admin = names->getSock(str);
				send(sock, msg.c_str(), msg.length(), 0);
				msg = "SERVER: You are now admin!";
				send(names->getSock(str), msg.c_str(), msg.length(), 0);
			}
			return true;
		}
		return false;
	}


	//admin/op commands


}