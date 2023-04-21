#include "../../includes/header.hpp"

// Set le body d'une réponse
bool fillBody(const int& id, std::string& type, const std::string& path, t_serv& serv,
			  std::string& answ, const int& socket)
{
	const std::map< const std::string, const std::string >::iterator mimeIT =
		data()->MIMEtypes.find(type);
	type = (mimeIT == data()->MIMEtypes.end() ? "text/plain" : mimeIT->second);
	const std::vector< std::string > splited = ft::split(type, '/');
	if (splited.size() != 2)
		return sendError(id, ERR_BAD_REQ, data()->codes[ERR_BAD_REQ], socket, serv);
	std::map< const std::string, std::string >::iterator headerIT =
		data()->headers[id].second.find("Accept");
	if (headerIT != data()->headers[id].second.end() &&
		headerIT->second.find(type) == std::string::npos &&
		headerIT->second.find(splited[0] + "/*") == std::string::npos &&
		headerIT->second.find("*/" + splited[1]) == std::string::npos &&
		headerIT->second.find("*/*") == std::string::npos)
		return sendError(id, ERR_NOT_ACCEPT, data()->codes[ERR_NOT_ACCEPT], socket, serv);

	std::ifstream file;
	file.open(path.c_str(), std::ios_base::in);
	if (!file)
		return sendError(id, ERR_INTERN, data()->codes[ERR_INTERN], socket, serv);

	std::ostringstream stream;
	stream << file.rdbuf();
	const std::string body = stream.str();
	answ = createAnsw(body, OK_OK, data()->codes[OK_OK], type, std::string());
	return true;
}

// Return une réponse HTTP
std::string createAnsw(const std::string& body, const unsigned short& codeRef,
					   const std::string& message, const std::string& contentType,
					   const std::string& redirect)
{
	std::ostringstream code;
	std::ostringstream bodySize;
	code << codeRef;
	bodySize << body.size();

	std::string answ = "HTTP/1.1 " + code.str() + ' ' + message + "\r\n" + "Server: webServ" +
					   "\r\n" + "Content-Length: " + bodySize.str() + "\r\n";
	if (!redirect.empty())
		answ += "Location: " + redirect + "\r\n";
	else if (!contentType.empty())
		answ += "Content-Type: " + contentType + "\r\n";
	answ += "\r\n" + body;
	return answ;
}

// Écrit et renvoie une page d'erreur par défaut
bool sendDefaultError(const int& id, const unsigned short& codeRef, const std::string& message,
					  const int& socket)
{
	data()->headers.erase(data()->headers.begin() + id);
	data()->bodies.erase(data()->bodies.begin() + id);

	std::ostringstream code2str;
	code2str << codeRef;

	const std::string body =
		"<!DOCTYPE html><meta charset=\"utf-8\"><html><head><title>" + code2str.str() +
		"</title><style>body {background-image: "
		"url(\"https://www.pcclean.io/wp-content/uploads/2020/4/VbqUBr.jpg\");background-position: "
		"center;color: white; font-family: serif; font-size: 23px; text-align: center; margin: "
		"0;}#main {min-width: 1000px;max-width: 1000px;position: absolute;top: 50%;left: "
		"50%;transform: translate(-50%, -80%);}#main-title {background-color: rgba(0, 0, 0, "
		"0.5);padding: 1px;padding-left: 10px;padding-right: 10px;}</style></head><body><div "
		"id=\"main\"><div id=\"main-title\"><h1>[ ERROR " +
		code2str.str() + " ]</h1><p>" + message + "</p></div></div></body></html>";
	const std::string answ = createAnsw(body, codeRef, message, "text/html", std::string());
	if (write(socket, answ.c_str(), answ.size()) != static_cast< ssize_t >(answ.size()))
	{
		keepNoMore(socket);
		std::cerr << "[ ERROR ] sendDefaultError - write() failed to socket " << socket
				  << std::endl;
	}
	return false;
}

// Envoie une réponse d'erreur HTTP
bool sendError(const int& id, const unsigned short codeRef, const std::string& message,
			   const int& socket, t_serv& serv)
{
	if (serv.errorPages.find(codeRef) == serv.errorPages.end())
		return sendDefaultError(id, codeRef, message, socket);

	const std::map< const std::string, std::string >::iterator URI =
		data()->headers[id].second.find("URI");
	std::string answ;
	redirect(answ,
			 std::pair< short, std::string >(RE_MOVED, data()->headers[id].second["Host"] + "/" +
														   serv.errorPages[codeRef]),
			 URI);
	if (write(socket, answ.c_str(), answ.size()) != static_cast< ssize_t >(answ.size()))
	{
		keepNoMore(socket);
		std::cerr << "[ ERROR ] sendError - write() failed to socket " << socket << std::endl;
	}
	data()->headers.erase(data()->headers.begin() + id);
	data()->bodies.erase(data()->bodies.begin() + id);
	return false;
}
