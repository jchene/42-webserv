#include "../../includes/header.hpp"

// Complète le header de la réponse du CGI
std::string completeCGIheader(const std::string& header, short& code, const bool& cLenFind,
							  const size_t& bodySize)
{
	if (!bodySize)
		code = OK_NOCONTENT;
	std::string ret;
	ret += header;
	std::ostringstream code2str;
	code2str << code;
	std::ostringstream bSize2str;
	bSize2str << bodySize;

	const std::string statut = "HTTP/1.1 " + code2str.str() + ' ' + data()->codes[code] + "\r\n";
	const std::string server = "Server: webServ\r\n";
	const std::string cLen = "Content-Length: " + bSize2str.str() + "\r\n";

	if (bodySize && !cLenFind)
		ret.insert(0, cLen);
	ret.insert(0, server);
	ret.insert(0, statut);
	return ret;
}

// Parse le header de la réponse du CGI
bool parseCGIheader(const int& id, std::string& header, const std::string& body, std::string& answ,
					t_serv& serv, const int& socket)
{
	std::istringstream stream(header);
	std::string line;
	short status = -1;
	bool cLenFind = false;
	while (std::getline(stream, line))
	{
		std::istringstream streamBis(line);
		std::string word;
		if (!(streamBis >> word))
			continue;
		if (word == "Status:")
		{
			if (!(streamBis >> word))
				continue;
			if (word.find_first_not_of("0123456789") != std::string::npos)
				return sendError(id, ERR_BAD_GATE, data()->codes[ERR_BAD_GATE], socket, serv);
			status = std::atoi(word.c_str());
			if (status > 1000)
				return sendError(id, ERR_BAD_GATE, data()->codes[ERR_BAD_GATE], socket, serv);
		}
		else if (word == "Content-Length:")
			cLenFind = true;
	}

	if (status == OK_OK)
		header.erase(0, header.find_first_of('\n') + 1);
	else if (status >= 400)
		return sendError(id, status, data()->codes[status], socket, serv);
	else
		status = OK_OK;
	answ += completeCGIheader(header, status, cLenFind, body.size() + 1);
	answ += "\n\r" + body;
	return true;
}

// Complète la réponse du CGI
bool completeAnsw(const int& id, const std::string& temp, t_serv& serv, std::string& answ,
				  const int& socket)
{
	if (temp.empty())
	{
		answ = createAnsw(std::string(), OK_NOCONTENT, data()->codes[OK_NOCONTENT], std::string(),
						  std::string());
		return true;
	}

	std::istringstream stream(temp);
	std::string line;
	std::string header;
	bool bodyInc = false;
	while (std::getline(stream, line))
	{
		if (line == "\r" && (bodyInc = true))
			break;
		header += line + '\n';
	}
	if (!bodyInc)
		return sendError(id, ERR_BAD_GATE, data()->codes[ERR_BAD_GATE], socket, serv);

	std::string body = std::string();
	while (std::getline(stream, line))
		body += line;
	return parseCGIheader(id, header, body, answ, serv, socket);
}

// Processus parent de l'execution du CGI
bool parentProcess(const int& id, const int& toWrite, const int& toRead, t_serv& serv,
				   std::string& answ, const int& socket, const pid_t& pid)
{
	if (!data()->bodies[id].second.empty())
	{
		if (write(toWrite, &data()->bodies[id].second[0], data()->bodies[id].second.size()) !=
			static_cast< ssize_t >(data()->bodies[id].second.size()))
		{
			close(toWrite);
			close(toRead);
			return sendError(id, ERR_INTERN, data()->codes[ERR_INTERN], socket, serv);
		}
	}
	close(toWrite);

	char buffer[BUFFER_SIZE + 1];
	ft::memset(buffer, 0, BUFFER_SIZE + 1);
	ssize_t readed;
	std::string temp;
	while ((readed = read(toRead, buffer, BUFFER_SIZE)) == BUFFER_SIZE)
	{
		temp += buffer;
		ft::memset(buffer, 0, BUFFER_SIZE + 1);
	}
	temp += buffer;
	close(toRead);
	if (readed < 0)
		return sendError(id, ERR_INTERN, data()->codes[ERR_INTERN], socket, serv);

	int status;
	if (waitpid(pid, &status, 0) < 0)
		return sendError(id, ERR_INTERN, data()->codes[ERR_INTERN], socket, serv);
	if (status % 255)
		return sendError(id, ERR_BAD_GATE, data()->codes[ERR_BAD_GATE], socket, serv);
	return completeAnsw(id, temp, serv, answ, socket);
}

// execve() le CGI
bool execCGI(const int& id, std::map< const std::string, std::string >& cgiEnv,
			 const std::vector< char* >& argz, t_serv& serv, std::string& answ, const int& socket)
{
	int pipeIN[2];
	int pipeOUT[2];
	if (pipe(pipeIN) < 0)
		return sendError(id, ERR_INTERN, data()->codes[ERR_INTERN], socket, serv);
	if (pipe(pipeOUT) < 0)
	{
		close(pipeIN[0]);
		close(pipeIN[1]);
		return sendError(id, ERR_INTERN, data()->codes[ERR_INTERN], socket, serv);
	}

	const pid_t pid = fork();
	if (pid < 0)
	{
		close(pipeIN[0]);
		close(pipeIN[1]);
		close(pipeOUT[0]);
		close(pipeOUT[1]);
		return sendError(id, ERR_INTERN, data()->codes[ERR_INTERN], socket, serv);
	}
	if (pid == 0) // child
	{
		close(pipeIN[WRITE]);
		close(pipeOUT[READ]);
		if (dup2(pipeIN[READ], STDIN_FILENO) < 0)
		{
			close(pipeIN[READ]);
			close(pipeOUT[WRITE]);
			exit(EXIT_FAILURE);
		}
		if (dup2(pipeOUT[WRITE], STDOUT_FILENO) < 0)
		{
			close(pipeIN[READ]);
			close(pipeOUT[WRITE]);
			exit(EXIT_FAILURE);
		}

		std::vector< char* > env;
		std::map< const std::string, std::string >::iterator it;
		for (it = cgiEnv.begin(); it != cgiEnv.end(); it++)
		{
			std::string* str = new std::string(it->first + "=" + it->second);
			if (!str)
			{
				std::vector< char* >::iterator temp;
				for (temp = env.begin(); temp != env.end(); temp++)
					delete *temp;
				close(pipeIN[READ]);
				close(pipeOUT[WRITE]);
				exit(EXIT_FAILURE);
			}
			env.push_back(&str[0][0]);
		}
		env.push_back(NULL);

		execve(argz[0], argz.data(), env.data());
		std::vector< char* >::iterator temp;
		for (temp = env.begin(); temp != env.end(); temp++)
			delete *temp;
		close(pipeIN[READ]);
		close(pipeOUT[WRITE]);
		exit(EXIT_FAILURE);
	}
	close(pipeIN[READ]);
	close(pipeOUT[WRITE]);
	return parentProcess(id, pipeIN[WRITE], pipeOUT[READ], serv, answ, socket, pid);
}

// Set l'IP:PORT et le nom de domaine si applicable du client
bool getClientInfos(const int& id, const int& socket, std::string& ip, std::string& port,
					t_serv& serv)
{
	t_sockaddr_in addr;
	ft::memset(&addr, 0, sizeof(addr));
	socklen_t addrlen = sizeof(addr);
	if (getsockname(socket, reinterpret_cast< t_sockaddr* >(&addr), &addrlen) == -1)
		return sendError(id, ERR_INTERN, data()->codes[ERR_INTERN], socket, serv);

	ip = inet_ntoa(addr.sin_addr);
	uint16_t temp = ntohs(addr.sin_port);
	std::ostringstream port2str;
	port2str << temp;
	port = port2str.str();
	return true;
}

// Set SCRIPT_NAME et PATH_INFO en fonction de l'URI
bool getScriptInfos(const int& id, const std::string& URI, std::string& scriptName,
					std::string& pathInfo, t_serv& serv, const int& socket)
{
	std::string ext = ft::split(ft::split(URI, '.').back(), '/').front();
	if (data()->MIMEtypes.find(ext) == data()->MIMEtypes.end())
		return sendError(id, ERR_INTERN, data()->codes[ERR_INTERN], socket, serv);
	ext.insert(0, 1, '.');

	const size_t pos = URI.find(ext);
	scriptName = URI.substr(0, pos + ext.size());
	pathInfo = URI.substr(pos + ext.size(), URI.size());
	return true;
}

// Set les variables d'environnement dont aura besoin le CGI
bool setEnv(const int& id, std::map< const std::string, std::string >& cgiEnv, t_serv& serv,
			const int& socket, const std::pair< const uint32_t, const uint16_t >& ip_port,
			const std::string& serverName, std::string& root)
{
	std::map< const std::string, std::string >::iterator it;

	it = data()->headers[id].second.find("User-Agent");
	cgiEnv["HTTP_USER_AGENT"] =
		(it != data()->headers[id].second.end() ? it->second : std::string());
	it = data()->headers[id].second.find("Host");
	cgiEnv["HTTP_HOST"] = (it != data()->headers[id].second.end() ? it->second : std::string());
	it = data()->headers[id].second.find("Connection");
	cgiEnv["HTTP_CONNECTION"] =
		(it != data()->headers[id].second.end() ? it->second : std::string());
	it = data()->headers[id].second.find("Accept");
	cgiEnv["HTTP_ACCEPT"] = (it != data()->headers[id].second.end() ? it->second : std::string());

	std::ostringstream ip2str;
	ip2str << ip_port.first;
	std::ostringstream port2str;
	port2str << ip_port.second;
	cgiEnv["SERVER_ADDR"] = ip2str.str();
	cgiEnv["SERVER_PORT"] = port2str.str();
	cgiEnv["SERVER_NAME"] = serverName;

	std::string clientIP;
	std::string clientPort;
	if (!getClientInfos(id, socket, clientIP, clientPort, serv))
		return false;
	cgiEnv["REMOTE_ADDR"] = clientIP;
	cgiEnv["REMOTE_PORT"] = clientPort;
	it = data()->headers[id].second.find("Content-Type");
	cgiEnv["CONTENT_TYPE"] = (it != data()->headers[id].second.end() ? it->second : std::string());
	it = data()->headers[id].second.find("Content-Length");
	cgiEnv["CONTENT_LENGTH"] =
		(it != data()->headers[id].second.end() ? it->second : std::string());
	cgiEnv["REQUEST_METHOD"] = data()->headers[id].second["Method"];
	cgiEnv["REQUEST_URI"] = data()->headers[id].second["URI"];

	cgiEnv["DOCUMENT_ROOT"] = root;
	const std::vector< std::string > splitedURI = ft::split(data()->headers[id].second["URI"], '?');
	std::string scriptName;
	std::string pathInfo;
	char buffer[BUFFER_SIZE + 1];
	ft::memset(buffer, 0, BUFFER_SIZE + 1);
	if (!getScriptInfos(id, splitedURI.front(), scriptName, pathInfo, serv, socket))
		return false;
	cgiEnv["SCRIPT_NAME"] = scriptName;
	cgiEnv["SCRIPT_FILENAME"] =
		std::string(getcwd(buffer, BUFFER_SIZE)) + root.substr(1, root.size()) + scriptName;
	cgiEnv["PATH_INFO"] = pathInfo;
	ft::memset(buffer, 0, BUFFER_SIZE + 1);
	cgiEnv["PATH_TRANSLATED"] = std::string(getcwd(buffer, BUFFER_SIZE)) + pathInfo;
	cgiEnv["QUERY_STRING"] = (splitedURI.size() > 1 ? splitedURI.back() : std::string());
	return true;
}

// Prépare l'execve() du CGI
bool runCGI(const int& id, t_serv& serv, t_location& location, std::string& answ, const int& socket,
			const std::pair< const uint32_t, const uint16_t >& ip_port,
			const std::string& serverName)
{
	const std::map< const std::string, std::string >::iterator rootIT =
		data()->headers[id].second.find("Root");
	std::string root;
	if (rootIT == data()->headers[id].second.end())
		root = "./www";
	else
		root = rootIT->second;
	std::map< const std::string, std::string > cgiEnv = data()->cgiData;
	if (!setEnv(id, cgiEnv, serv, socket, ip_port, serverName, root))
		return false;

	std::vector< char* > argz;
	argz.push_back(&location.cgiPath[0]);
	argz.push_back(&data()->headers[id].second["URI"][0]);
	argz.push_back(NULL);
	return (execCGI(id, cgiEnv, argz, serv, answ, socket));
}
