#include "../../includes/header.hpp"

// Print une requete complete
void printRequest(std::map< const std::string, std::string >& header, std::vector< char >& body)
{
	for (std::map< const std::string, std::string >::iterator it = header.begin();
		 it != header.end(); it++)
		std::cout << it->first << ": " << it->second << std::endl;
	std::cout << std::endl;
	for (size_t x = 0; x < body.size(); x++)
		std::cout << body[x];
	std::cout << "--------------(end)--------------" << std::endl;
}

// Remove un socket des keep-alive
void keepNoMore(const int& socket)
{
	size_t x;
	for (x = 0; x < data()->toKeep.size(); x++)
		if (data()->toKeep[x] == socket)
			break;
	if (x != data()->toKeep.size())
		data()->toKeep.erase(data()->toKeep.begin() + x);
}

// Keep-alive une communication
void try2keep(const int& socket)
{
	size_t x;
	for (x = 0; x < data()->toKeep.size(); x++)
		if (data()->toKeep[x] == socket)
			break;
	if (x == data()->toKeep.size())
		data()->toKeep.push_back(socket);
}

// Check si le body est complet
bool isItOver(const size_t& x, const int& id, const size_t& servIndex, const int& socket,
			  const int& contentLen, const short& chunked, const size_t& bodySize,
			  std::map< const std::string, std::string >& header)
{
	std::cout << "chunked " << chunked << std::endl;
	if (chunked == 1 || (!chunked && bodySize < static_cast< size_t >(contentLen)))
	{
		std::cout << "2.25" << std::endl;
		if (header["Method"] == "GET")
			return sendError(id, ERR_BAD_REQ, data()->codes[ERR_BAD_REQ], socket,
							 data()->servList[servIndex]);
		try2keep(socket);
		if (!data()->pollz.second[x].second)
		{
			data()->pollz.second[x].second = new t_timeval;
			if (!data()->pollz.second[x].second ||
				gettimeofday(data()->pollz.second[x].second, NULL))
				return sendError(id, ERR_INTERN, data()->codes[ERR_INTERN], socket,
								 data()->servList[servIndex]);
		}
		const std::map< const std::string, std::string >::iterator it = header.find("Expect");
		if (it != header.end() && it->second == "100-continue")
		{
			const std::string answ =
				createAnsw(std::string(), INFO_CONTINUE, data()->codes[INFO_CONTINUE],
						   std::string(), std::string());
			if (write(socket, answ.c_str(), answ.size()) != static_cast< ssize_t >(answ.size()))
			{
				std::cerr << "[ ERROR ] sendError - write() failed to socket " << socket
						  << std::endl;
				keepNoMore(socket);
			}
		}
		return false;
	}
	else if (data()->pollz.second[x].second)
	{
		std::cout << "2.5" << std::endl;
		const std::map< const std::string, std::string >::iterator it = header.find("Connection");
		if (it == header.end() || it->second == "close")
			keepNoMore(socket);
		delete data()->pollz.second[x].second;
		data()->pollz.second[x].second = NULL;
	}
	std::cout << "3" << std::endl;
	return true;
}

// Unchunk une ligne chunked
bool unchunk(std::vector< char >& body, const int& id, const size_t& servIndex, const int& socket,
			 short& unchunk)
{
	const std::vector< char > temp(body);
	body = std::vector< char >();
	int chunkSize = -1;
	size_t x = 0;
	size_t count;
	while (x < temp.size())
	{
		if (chunkSize < 0)
		{
			std::string line;
			for (; x < temp.size(); x++)
			{
				if (temp[x] == '\r' && temp[x + 1] && temp[x + 1] == '\n')
					break;
				line += temp[x];
			}
			if (temp[x] != '\r' || ++x >= temp.size() || temp[x] != '\n')
				return sendError(id, ERR_BAD_REQ, data()->codes[ERR_BAD_REQ], socket,
								 data()->servList[servIndex]);
			std::istringstream hexStr2int(line);
			hexStr2int >> std::hex >> chunkSize;
			if (hexStr2int.fail())
				return sendError(id, ERR_BAD_REQ, data()->codes[ERR_BAD_REQ], socket,
								 data()->servList[servIndex]);
			if (chunkSize && !temp[++x])
				return sendError(id, ERR_BAD_REQ, data()->codes[ERR_BAD_REQ], socket,
								 data()->servList[servIndex]);
			else if (!chunkSize)
				x++;
		}
		else
		{
			count = 0;
			for (; x < temp.size(); x++)
			{
				if (temp[x] == '\r' && temp[x + 1] && temp[x + 1] == '\n')
					break;
				body.push_back(temp[x]);
				count++;
			}
			if (temp[x] != '\r' || ++x >= temp.size() || temp[x] != '\n')
				return sendError(id, ERR_BAD_REQ, data()->codes[ERR_BAD_REQ], socket,
								 data()->servList[servIndex]);
			if (count != static_cast< size_t >(chunkSize))
				return sendError(id, ERR_BAD_REQ, data()->codes[ERR_BAD_REQ], socket,
								 data()->servList[servIndex]);
			if (!chunkSize)
				break;
			chunkSize = -1;
			x++;
		}
	}
	if (!chunkSize)
		unchunk = 2;
	return true;
}

// Traite un buffer contenant une partie du body
short appendBodyBuffer(std::vector< char >& body, size_t& bodySize, ssize_t& readed,
					   const int& socket, bool& stop)
{
	char buffer[BUFFER_SIZE + 1];
	ft::memset(buffer, 0, BUFFER_SIZE + 1);
	readed = read(socket, buffer, BUFFER_SIZE);
	if (readed < 0)
	{
		keepNoMore(socket);
		std::cerr << "[ ERROR ] read2body - read() failed for socket " << socket << std::endl;
		return false;
	}
	else if (!readed && (stop = true))
		return true;
	for (short x = 0; x < readed; x++)
		body.push_back(buffer[x]);
	bodySize += readed;
	return true;
}

// Lit le body de la requête
bool read2body(const size_t& x, const int& id, const int& socket, ssize_t& readed,
			   const size_t& servIndex, std::map< const std::string, std::string >& header,
			   std::vector< char >& body)
{
	int contentLen = 0;
	short chunked = 0;
	std::map< const std::string, std::string >::iterator it;
	it = header.find("Content-Length");
	if (it != header.end())
	{
		if (it->second.find_first_not_of("0123456789") != std::string::npos)
			return sendError(id, ERR_BAD_REQ, data()->codes[ERR_BAD_REQ], socket,
							 data()->servList[servIndex]);
		contentLen = std::atoi(it->second.c_str());
	}
	it = header.find("Transfer-Encoding");
	if (it != header.end() && it->second == "chunked")
	{
		if (contentLen)
			return sendError(id, ERR_BAD_REQ, data()->codes[ERR_BAD_REQ], socket,
							 data()->servList[servIndex]);
		chunked = 1;
		header["Chunked"] = "1";
	}
	if (contentLen > data()->servList[servIndex].maxBodySize * ONE_MO)
		return sendError(id, ERR_BODY_2LARG, data()->codes[ERR_BODY_2LARG], socket,
						 data()->servList[servIndex]);
	size_t bodySize = body.size();
	bool stop = false;
	while (!stop && readed == BUFFER_SIZE &&
		   bodySize < static_cast< size_t >(data()->servList[servIndex].maxBodySize * ONE_MO))
		if (!appendBodyBuffer(body, bodySize, readed, socket, stop))
			return false;

	if (!contentLen && !chunked && bodySize)
		return sendError(id, ERR_LEN_REQ, data()->codes[ERR_LEN_REQ], socket,
						 data()->servList[servIndex]);
	if (!chunked && bodySize > static_cast< size_t >(contentLen))
		return sendError(id, ERR_BAD_REQ, data()->codes[ERR_BAD_REQ], socket,
						 data()->servList[servIndex]);
	if (bodySize > static_cast< size_t >(data()->servList[servIndex].maxBodySize * ONE_MO))
		return sendError(id, ERR_BODY_2LARG, data()->codes[ERR_BODY_2LARG], socket,
						 data()->servList[servIndex]);
	if (chunked && !unchunk(body, id, servIndex, socket, chunked))
		return false;
	return isItOver(x, id, servIndex, socket, contentLen, chunked, bodySize, header);
}

// Lit une ligne de header
bool readHeaderLine(std::map< const std::string, std::string >& header, const std::string& line,
					const int& id, bool& bodyInc, size_t& servIndex, const int& socket)
{
	if (line == "\r" && (bodyInc = true))
		return true;
	std::pair< std::string, std::string > values = ft::psplit(line, ':');
	if (values.first.empty())
		return sendError(id, ERR_BAD_REQ, data()->codes[ERR_BAD_REQ], socket,
						 data()->servList[servIndex]);
	const std::string key = values.first.substr(values.first.find_first_not_of(" \t"),
												values.first.find_last_not_of(" \t") -
													values.first.find_first_not_of(" \t") + 1);
	const std::string value = values.second.substr(values.second.find_first_not_of(" \t"),
												   values.second.find_last_not_of(" \t") -
													   values.second.find_first_not_of(" \t") + 1);
	if (key.empty() || value.empty())
		return sendError(id, ERR_BAD_REQ, data()->codes[ERR_BAD_REQ], socket,
						 data()->servList[servIndex]);
	if (header.find(key) != header.end())
		return sendError(id, ERR_BAD_REQ, data()->codes[ERR_BAD_REQ], socket,
						 data()->servList[servIndex]);
	header[key] = value;
	return true;
}

// Verifie si la location cible est une location imbriquer
void look4subLoca(const std::string& URI, t_location& location,
				  std::map< const std::string, std::string >& header)
{
	for (size_t x = 0; x < location.locations.size(); x++)
	{
		const std::string subStr(URI.substr(0, location.locations[x].name.size()));
		if (subStr == location.locations[x].name)
		{
			location = location.locations[x];
			if (!location.root.empty())
				header["Root"] = location.root;
			look4subLoca(URI, location, header);
			break;
		}
	}
}

// Set l'index d'une location dans un bloc serveur si elle est trouvée
bool locaIndexFromURI(const std::string& URI, t_serv& serv, t_location& location,
					  std::map< const std::string, std::string >& header)
{
	const bool def = (URI == "/" ? true : false);
	for (size_t x = 0; x < serv.locations.size(); x++)
	{
		if (serv.locations[x].name == "/" && !def)
			continue;
		const std::string subStr(URI.substr(0, serv.locations[x].name.size()));
		if (subStr == serv.locations[x].name)
		{
			location = serv.locations[x];
			if (!location.root.empty())
				header["Root"] = location.root;
			if (!def)
				look4subLoca(URI, location, header);
			return true;
		}
	}
	return false;
}

// Set l'index indexant le bloc serveur auquel s'adresser
bool localise2servBlock(std::map< const std::string, std::string >& header,
						std::istringstream& stream, size_t& servIndex,
						const std::pair< const uint32_t, const uint16_t >& ip_port,
						std::string& serverName)
{
	bool ret = true;
	std::string key;
	std::string value;
	std::string line;
	if (std::getline(stream, line))
	{
		std::pair< std::string, std::string > values = ft::psplit(line, ':');
		key = values.first.substr(values.first.find_first_not_of(" \t"),
								  values.first.find_last_not_of(" \t") -
									  values.first.find_first_not_of(" \t") + 1);
		value = values.second.substr(values.second.find_first_not_of(" \t"),
									 values.second.find_last_not_of(" \t") -
										 values.second.find_first_not_of(" \t") + 1);
		if (key != "Host")
			ret = false;
		else
			header["Host"] = value;
		value = value.substr(0, value.find(":"));
	}
	else
		ret = false;

	bool stop = false;
	size_t defaultServ = 0;
	for (size_t x = 0; x < data()->servList.size() && !stop; x++)
	{
		for (size_t y = 0; y < data()->servList[x].ip_port.size(); y++)
		{
			if ((data()->servList[x].ip_port[y].first == ip_port.first &&
				 data()->servList[x].ip_port[y].second == ip_port.second) ||
				(ip_port.first == 0 && data()->servList[x].ip_port[y].second == ip_port.second))
			{
				if (!defaultServ)
					defaultServ = x + 1;
				for (size_t z = 0; !value.empty() && z < data()->servList[x].serverName.size(); z++)
				{
					if (data()->servList[x].serverName[z] == value && (stop = true))
					{
						serverName = value;
						servIndex = x;
						break;
					}
				}
			}
		}
	}
	if (!stop)
		servIndex = defaultServ - 1;
	return ret;
}

// Set et vérifie les infos de la première ligne de la requête
unsigned short check1stLine(std::istringstream& stream,
							std::map< const std::string, std::string >& header)
{
	std::string line;
	if (std::getline(stream, line))
	{
		std::istringstream streamBis(line);
		std::string word;

		if (!(streamBis >> word))
			return ERR_BAD_REQ;
		const std::string method = word;
		if (method != "GET" && method != "POST" && method != "DELETE")
			return ERR_UNKN_METHOD;

		if (!(streamBis >> word))
			return ERR_BAD_REQ;
		const std::string uri = word;
		if (uri.size() > URI_MAX_SIZE)
			return ERR_URI2LONG;

		if (!(streamBis >> word))
			return ERR_BAD_REQ;
		if (word != "HTTP/1.1")
			return ERR_HTTP_VERSION;

		header["Method"] = method;
		header["URI"] = uri;
	}
	return 0;
}

// Set et vérifie les infos du header de la requête
bool check2header(const std::string& fullBuffer, const int& id,
				  std::map< const std::string, std::string >& header,
				  const std::pair< const uint32_t, const uint16_t >& ip_port, size_t& servIndex,
				  t_location& location, std::string& serverName, const int& socket,
				  size_t& totalReaded, size_t& bodyInc)
{
	std::istringstream stream(fullBuffer);
	unsigned short ret = check1stLine(stream, header);
	if (bodyInc == std::string::npos)
		ret = ERR_HEADER2LARG;
	else if (totalReaded > HEADER_MAX_SIZE)
		ret = ERR_BAD_REQ;
	if (!localise2servBlock(header, stream, servIndex, ip_port, serverName))
	{
		ret = (ret ? ret : ERR_BAD_REQ);
		return sendError(id, ret, data()->codes[ret], socket, data()->servList[servIndex]);
	}
	if (!locaIndexFromURI(header["URI"], data()->servList[servIndex], location, header))
		locaIndexFromURI("/", data()->servList[servIndex], location, header);
	if (ret)
		return sendError(id, ret, data()->codes[ret], socket, data()->servList[servIndex]);

	bool stop = false;
	std::string line;
	while (std::getline(stream, line) && !stop)
		if (!readHeaderLine(header, line, id, stop, servIndex, socket))
			return false;
	return true;
}

// Lit le header
bool read2header(const size_t& x, const int& id, const int& socket, ssize_t& readed,
				 std::map< const std::string, std::string >& header, std::vector< char >& body,
				 size_t& servIndex, t_location& location, std::string& serverName)
{
	char buffer[BUFFER_SIZE + 1];
	std::string fullBuffer;
	size_t totalReaded = 0;
	size_t bodyInc = std::string::npos;
	while (readed == BUFFER_SIZE && bodyInc == std::string::npos && totalReaded < HEADER_MAX_SIZE)
	{
		ft::memset(buffer, 0, BUFFER_SIZE + 1);
		readed = read(socket, buffer, BUFFER_SIZE);
		totalReaded += readed;
		if (readed < 0)
		{
			keepNoMore(socket);
			std::cerr << "[ ERROR ] read2socket - read() failed for " << socket << "..."
					  << std::endl;
			std::cerr << "...Abortin, no answer will be sent()" << std::endl;
			return false;
		}
		else if (readed == 0)
			break;
		std::string buff2string = buffer;
		bodyInc = buff2string.find("\n\r\n");
		if (bodyInc != std::string::npos)
		{
			fullBuffer += buff2string.substr(0, bodyInc);
			for (size_t y = 0; y < readed - bodyInc - 3; y++)
				body.push_back(buffer[bodyInc + 3 + y]);
		}
		else
			fullBuffer += buffer;
	}
	std::cout << fullBuffer << std::endl;
	if (fullBuffer.empty())
	{
		keepNoMore(socket);
		return false;
	}
	const std::pair< const uint32_t, const uint16_t >& ip_port =
		data()->ip_port[data()->pollz.second[x].first];
	return check2header(fullBuffer, id, header, ip_port, servIndex, location, serverName, socket,
						totalReaded, bodyInc);
}

// Lit depuis un socket
bool read2socket(const size_t& x, const int& id, size_t& servIndex, t_location& location,
				 std::string& serverName)
{
	const int& socket = data()->pollz.first[x].fd;

	std::map< const std::string, std::string >& header = data()->headers[id].second;
	std::vector< char >& body = data()->bodies[id].second;
	std::map< const std::string, std::string >::iterator it;

	ssize_t readed = BUFFER_SIZE;
	if (header.empty())
	{
		if (!read2header(x, id, socket, readed, header, body, servIndex, location, serverName))
			return false;
		it = header.find("Connection");
		if (it != header.end())
		{
			if (it->second == "keep-alive")
				try2keep(socket);
			if (it->second == "close")
				keepNoMore(socket);
		}
	}
	it = header.find("servIndex");
	if (it == header.end())
	{
		std::ostringstream servIndex2str;
		servIndex2str << servIndex;
		header["servIndex"] = servIndex2str.str();
	}
	else
	{
		servIndex = std::atoi(it->second.c_str());
		if (!locaIndexFromURI(header["URI"], data()->servList[servIndex], location, header))
			locaIndexFromURI("/", data()->servList[servIndex], location, header);
	}
	it = header.find("serverName");
	if (it == header.end())
		header["serverName"] = serverName;
	else
		serverName = header["serverName"];
	if (!read2body(x, id, socket, readed, servIndex, header, body))
		return false;

	printRequest(header, body);
	return true;
}
