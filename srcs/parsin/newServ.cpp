#include "../../includes/header.hpp"

// Convertit une string en uint32_t
long str2id(const std::string& str, const bool& check)
{
	if (check)
	{
		const std::vector< std::string > values = ft::split(str, '.');
		if (values.size() != 4)
			return -1;
		for (size_t x = 0; x < values.size(); x++)
		{
			if (values[x].find_first_not_of("0123456789") != std::string::npos)
				return -1;
			if (values[x][0] == '0' && values[x].size() != 1)
				return -1;
			if (std::atoi(values[x].c_str()) > 255)
				return -1;
		}
	}

	t_addrinfo hints;
	t_addrinfo* results;
	ft::memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	const int error = getaddrinfo(str.c_str(), NULL, &hints, &results);
	if (error)
	{
		gai_strerror(error);
		return -1;
	}
	const uint32_t id = ntohl(((t_sockaddr_in*)results->ai_addr)->sin_addr.s_addr);
	freeaddrinfo(results);
	return id;
}

// Remplit une combinaison IP:PORT
bool setIpPort(const uint32_t& ip, const uint16_t& port,
			   std::vector< std::pair< const uint32_t, const uint16_t > >& ref)
{
	for (size_t x = 0; x < ref.size(); x++)
		if (ref[x].first == ip && ref[x].second == port)
			return false;
	ref.push_back(std::pair< const uint32_t, const uint16_t >(ip, port));

	for (size_t x = 0; x < data()->ip_port.size(); x++)
		if ((data()->ip_port[x].first == ip && data()->ip_port[x].second == port) ||
			(data()->ip_port[x].first == 0 && data()->ip_port[x].second == port))
			return true;
	if (ip == 0)
		for (size_t x = 0; x < data()->ip_port.size(); x++)
			if (data()->ip_port[x].second == port)
				data()->ip_port.erase(data()->ip_port.begin() + x--);
	data()->ip_port.push_back(std::pair< const uint32_t, const uint16_t >(ip, port));
	return true;
}

// Convertit une string en une combinaison IP:PORT
bool getIpPort(std::string& value, std::vector< std::pair< const uint32_t, const uint16_t > >& ref)
{
	if (value.find_first_not_of(".0123456789:") != std::string::npos)
		return false;

	long tmp = 0;
	if (value.find_first_of(":") != std::string::npos)
	{
		if (std::count(value.begin(), value.end(), ':') > 1)
			return false;
		if ((tmp = str2id(value.substr(0, value.find_first_of(":")), true)) < 0)
			return false;
		value = value.substr(value.find_first_of(":") + 1, value.size());
	}
	const uint32_t ip = static_cast< uint32_t >(tmp);

	if (value.find_first_not_of("0123456789") != std::string::npos)
		return false;
	tmp = std::atoi(value.c_str());
	if (tmp <= 0 || tmp > 65535)
		return false;
	const uint16_t port = tmp;
	return setIpPort(ip, port, ref);
}

// Remplit une struct de bloc serveur avec ses valeurs par défaut
void fillServ(t_serv& ref)
{
	std::string ip_port = "0.0.0.0";
	if (ref.ip_port.empty())
		getIpPort(ip_port, ref.ip_port);
	for (size_t x = 0; x < ref.locations.size(); x++)
		if (ref.locations[x].name == "/")
			return;
	ref.locations.push_back(initLoca("/"));
}

// La directive listen a été trouvé dans un bloc serveur
bool listenIsHere(std::istringstream& stream, std::string& word, t_serv& serv,
				  const std::string& keyword, const std::ostringstream& lineIndex)
{
	if (!(stream >> word))
		return configError("[ ERROR ] listenIsHere() - " + keyword + " [" + lineIndex.str() + ']' +
							   S_ERR_NO_VAL,
						   false, 0);
	if (!getIpPort(word, serv.ip_port))
		return configError("[ ERROR ] listenIsHere() - " + keyword + " [" + lineIndex.str() + ']' +
							   S_ERR_BAD_VAL,
						   false, 0);
	if (stream >> word)
		return configError("[ ERROR ] listenIsHere() - " + keyword + " [" + lineIndex.str() + ']' +
							   S_ERR_TM_VAL,
						   false, 0);
	return true;
}

// La directive serverName a été trouvé dans un bloc serveur
bool serverNameIsHere(std::istringstream& stream, std::string& word, t_serv& serv,
					  const std::string& keyword, const std::ostringstream& lineIndex)
{
	if (!(stream >> word))
		return configError("[ ERROR ] serverNameIsHere() - " + keyword + " [" + lineIndex.str() +
							   ']' + S_ERR_NO_VAL,
						   false, 0);
	while (!stream.fail())
	{
		serv.serverName.push_back(word);
		stream >> word;
	}
	return true;
}

// La directive errorPage a été trouvé dans un bloc serveur
bool errorPageIsHere(std::istringstream& stream, std::string& word, t_serv& serv,
					 const std::string& keyword, const std::ostringstream& lineIndex)
{
	if (!(stream >> word))
		return configError("[ ERROR ] errorPageIsHere() - " + keyword + " [" + lineIndex.str() +
							   ']' + S_ERR_NO_VAL,
						   false, 0);
	while (!stream.fail())
	{
		const std::string pageName = ft::split(word, '/').back();
		const std::string codeStr = ft::split(pageName, '.').front();
		if (codeStr.find_first_not_of("0123456789") != std::string::npos)
			return configError("[ ERROR ] errorPageIsHere() - " + keyword + " [" + lineIndex.str() +
								   ']' + S_ERR_BAD_VAL,
							   false, 0);

		const int code = std::atoi(codeStr.c_str());
		if (code > 999)
			return configError("[ ERROR ] errorPageIsHere() - " + keyword + " [" + lineIndex.str() +
								   ']' + S_ERR_BAD_VAL,
							   false, 0);
		serv.errorPages.insert(std::pair< const unsigned short, const std::string >(code, word));
		stream >> word;
	}
	return true;
}

// Procède au switch de parseServ()
bool parseServSwitch(std::istringstream& stream, std::string& word, t_serv& serv,
					 std::ifstream& config, size_t& configLine, const std::ostringstream& lineIndex,
					 int& maxBodySize, bool& stop, bool& stopBis)
{
	const std::string keywords[] = {"listen", "serverName", "errorPage",		"location",
									"}",	  "#",			"clientMaxBodySize"};
	const std::vector< std::string > keys(keywords,
										  keywords + sizeof(keywords) / sizeof(std::string));
	switch (std::ptrdiff_t dist =
				std::distance(keys.begin(), std::find(keys.begin(), keys.end(), word)))
	{
	case (0):
		if (!listenIsHere(stream, word, serv, keywords[0], lineIndex))
			return false;
		break;
	case (1):
		if (!serverNameIsHere(stream, word, serv, keywords[1], lineIndex))
			return false;
		break;
	case (2):
		if (!errorPageIsHere(stream, word, serv, keywords[2], lineIndex))
			return false;
		break;
	case (3):
		if (!setupLoca(config, stream, &serv, NULL, configLine))
			return false;
		break;
	case (4):
		stop = true;
		break;
	case (5):
		stopBis = true;
		break;
	case (6):
		if (!maxBodySizeIsHere(maxBodySize, stream, word, keywords[6], lineIndex))
			return false;
		break;
	default:
		return configError("[ ERROR ] parseServSwitch() - Unknown keyword [" + lineIndex.str() +
							   "]: " + word,
						   false, 0);
	}
	return true;
}

// Parse un bloc serveur
bool parseServ(std::ifstream& config, size_t& configLine)
{
	t_serv serv;
	int maxBodySize = -1;

	bool stop = false;
	std::string line;
	while (!stop && std::getline(config, line))
	{
		std::ostringstream lineIndex;
		configLine++;
		lineIndex << configLine;

		bool stopBis = false;
		std::istringstream stream(line);
		std::string word;
		while (!stopBis && !stream.fail() && (stream >> word))
			if (!parseServSwitch(stream, word, serv, config, configLine, lineIndex, maxBodySize,
								 stop, stopBis))
				return false;
	}
	if (!stop)
		return configError(std::string("[ ERROR ] parseServ() - server") + S_ERR_NO_BRK, false, 0);

	serv.maxBodySize = maxBodySize;
	fillServ(serv);
	data()->servList.push_back(serv);
	return true;
}
