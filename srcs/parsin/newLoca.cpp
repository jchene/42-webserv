#include "../../includes/header.hpp"

// Assigne les valeurs d'un bloc limitExcept
bool assignNewLimit(std::istringstream& stream, t_location& ref,
					const std::vector< std::pair< const uint32_t, const bool > >& inputs,
					const size_t& configLine)
{
	const std::string keywords[] = {"GET", "POST", "DELETE", "{"};
	const std::vector< std::string > keys(keywords,
										  keywords + sizeof(keywords) / sizeof(std::string));

	ref.limitExcept["GET"] = inputs;
	ref.limitExcept["POST"] = inputs;
	ref.limitExcept["DELETE"] = inputs;

	std::ostringstream lineIndex;
	lineIndex << configLine;

	bool openned = false;
	std::string word;
	while (stream >> word)
	{
		switch (std::distance(keys.begin(), std::find(keys.begin(), keys.end(), word)))
		{
		case (0):
			ref.limitExcept["GET"] = std::vector< std::pair< const uint32_t, const bool > >();
			break;
		case (1):
			ref.limitExcept["POST"] = std::vector< std::pair< const uint32_t, const bool > >();
			break;
		case (2):
			ref.limitExcept["DELETE"] = std::vector< std::pair< const uint32_t, const bool > >();
			break;
		case (3):
			openned = true;
			break;
		default:
			return configError("[ ERROR ] assignNewLimit() - Unknown keyword [" + lineIndex.str() +
								   "]: " + word,
							   false, 0);
		}
	}
	if (!openned)
		return configError(std::string("[ ERROR ] assignNewLimit() - limitExcept") + S_ERR_NO_BRK,
						   false, 0);
	return true;
}

// La directive allow a été trouvé dans un bloc limitExcept
bool allowIsHere(std::vector< std::pair< const uint32_t, const bool > >& inputs,
				 std::istringstream& streamBis, std::string& word, const std::string& keyword,
				 const std::ostringstream& lineIndex)
{
	if (!(streamBis >> word))
		return configError("[ ERROR ] allowIsHere() - " + keyword + " [" + lineIndex.str() + ']' +
							   S_ERR_NO_VAL,
						   false, 0);
	const long ip = (word == "all" ? 0 : str2id(word, true));
	if (ip < 0)
		return configError("[ ERROR ] allowIsHere() - " + keyword + " [" + lineIndex.str() + ']' +
							   S_ERR_BAD_VAL,
						   false, 0);

	inputs.push_back(std::pair< const uint32_t, const bool >(ip, true));
	if (streamBis >> word)
		return configError("[ ERROR ] allowIsHere() - " + keyword + " [" + lineIndex.str() + ']' +
							   S_ERR_TM_VAL,
						   false, 0);
	return true;
}

// La directive deny a été trouvé dans un bloc limitExcept
bool denyIsHere(std::vector< std::pair< const uint32_t, const bool > >& inputs,
				std::istringstream& streamBis, std::string& word, const std::string& keyword,
				const std::ostringstream& lineIndex)
{
	if (!(streamBis >> word))
		return configError("[ ERROR ] denyIsHere() - " + keyword + " [" + lineIndex.str() + ']' +
							   S_ERR_NO_VAL,
						   false, 0);
	const long ip = (word == "all" ? 0 : str2id(word, true));
	if (ip < 0)
		return configError("[ ERROR ] denyIsHere() - " + keyword + " [" + lineIndex.str() + ']' +
							   S_ERR_BAD_VAL,
						   false, 0);

	inputs.push_back(std::pair< const uint32_t, const bool >(ip, false));
	if (streamBis >> word)
		return configError("[ ERROR ] denyIsHere() - " + keyword + " [" + lineIndex.str() + ']' +
							   S_ERR_TM_VAL,
						   false, 0);
	return true;
}

// Parse un bloc limitExcept
bool parseNewLimit(std::ifstream& config, std::istringstream& stream, t_location& ref,
				   size_t& configLine)
{
	const std::string keywords[] = {"allow", "deny", "}", "#"};
	const std::vector< std::string > keys(keywords,
										  keywords + sizeof(keywords) / sizeof(std::string));

	std::vector< std::pair< const uint32_t, const bool > > inputs;

	bool stop = false;
	std::string line;
	while (!stop && std::getline(config, line))
	{
		std::ostringstream lineIndex;
		configLine++;
		lineIndex << configLine;

		bool stopBis = false;
		std::istringstream streamBis(line);
		std::string word;
		while (!stopBis && !streamBis.fail() && (streamBis >> word))
		{
			switch (std::distance(keys.begin(), std::find(keys.begin(), keys.end(), word)))
			{
			case (0):
				if (!allowIsHere(inputs, streamBis, word, keywords[0], lineIndex))
					return false;
				break;
			case (1):
				if (!denyIsHere(inputs, streamBis, word, keywords[1], lineIndex))
					return false;
				break;
			case (2):
				stop = true;
				break;
			case (3):
				stopBis = true;
				break;
			default:
				return configError("[ ERROR ] parseNewLimit() - Unknown keyword [" +
									   lineIndex.str() + "]: " + word,
								   false, 0);
			}
		}
	}
	if (!stop)
		return configError(std::string("[ ERROR ] parseNewLimit() - limitExcept") + S_ERR_NO_BRK,
						   false, 0);
	return assignNewLimit(stream, ref, inputs, configLine);
}

// Init un bloc location
t_location initLoca(const std::string& name)
{
	t_location loca;
	loca.name = name;
	loca.root = "./www";
	loca.autoIndex = false;
	loca.redirect = std::pair< short, std::string >(-1, std::string());
	loca.limitExcept["GET"] = std::vector< std::pair< const uint32_t, const bool > >();
	loca.limitExcept["POST"] = std::vector< std::pair< const uint32_t, const bool > >();
	loca.limitExcept["DELETE"] = std::vector< std::pair< const uint32_t, const bool > >();
	return loca;
}

// Une redirection a été trouvé dans un bloc location
bool returnIsHere(std::pair< short, std::string > redirect, t_location& ref,
				  std::istringstream& stream, std::string& word, const std::string& keyword,
				  const std::ostringstream& lineIndex)
{
	if (!(stream >> redirect.first))
		return configError("[ ERROR ] returnIsHere() - " + keyword + " [" + lineIndex.str() + ']' +
							   S_ERR_NO_VAL + " or is invalid",
						   false, 0);
	if (redirect.first < 0 || redirect.first > 1000)
		return configError("[ ERROR ] returnIsHere() - " + keyword + " [" + lineIndex.str() + ']' +
							   S_ERR_BAD_VAL,
						   false, 0);
	if (stream >> redirect.second)
		if (stream >> word)
			return configError("[ ERROR ] returnIsHere() - " + keyword + " [" + lineIndex.str() +
								   ']' + S_ERR_TM_VAL,
							   false, 0);
	if (ref.redirect.first == -1)
		ref.redirect = redirect;
	return true;
}

// La directive autoIndex a été trouvé dans un bloc location
bool autoIndexIsHere(bool& autoindexFound, t_location& ref, std::istringstream& stream,
					 std::string& word, const std::string& keyword,
					 const std::ostringstream& lineIndex)
{
	if (autoindexFound == true)
		return configError("[ ERROR ] autoIndexIsHere() - " + keyword + " [" + lineIndex.str() +
							   ']' + S_ERR_DUP,
						   false, 0);
	if (!(stream >> word))
		return configError("[ ERROR ] autoIndexIsHere() - " + keyword + " [" + lineIndex.str() +
							   ']' + S_ERR_NO_VAL,
						   false, 0);
	if (word != "on" && word != "off")
		return configError("[ ERROR ] autoIndexIsHere() - " + keyword + " [" + lineIndex.str() +
							   ']' + S_ERR_BAD_VAL,
						   false, 0);
	ref.autoIndex = (word == "on" ? true : false);
	if (stream >> word)
		return configError("[ ERROR ] autoIndexIsHere() - " + keyword + " [" + lineIndex.str() +
							   ']' + S_ERR_TM_VAL,
						   false, 0);
	autoindexFound = true;
	return true;
}

// La directive root a été trouvé dans un bloc location
bool rootIsHere(bool& rootFound, t_location& ref, std::istringstream& stream, std::string& word,
				const std::string& keyword, const std::ostringstream& lineIndex)
{
	if (rootFound == true)
		return configError("[ ERROR ] rootIsHere() - " + keyword + " [" + lineIndex.str() + ']' +
							   S_ERR_DUP,
						   false, 0);
	if (!(stream >> word))
		return configError("[ ERROR ] rootIsHere() - " + keyword + " [" + lineIndex.str() + ']' +
							   S_ERR_NO_VAL,
						   false, 0);
	ref.root = "." + word;
	if (stream >> word)
		return configError("[ ERROR ] rootIsHere() - " + keyword + " [" + lineIndex.str() + ']' +
							   S_ERR_TM_VAL,
						   false, 0);
	rootFound = true;
	return true;
}

// La directive index a été trouvé dans un bloc location
bool indexIsHere(t_location& ref, std::istringstream& stream, std::string& word,
				 const std::string& keyword, const std::ostringstream& lineIndex)
{
	if (!(stream >> word))
		return configError("[ ERROR ] indexIsHere() - " + keyword + " [" + lineIndex.str() + ']' +
							   S_ERR_NO_VAL,
						   false, 0);
	while (!stream.fail())
	{
		ref.indexPage.push_back(word);
		stream >> word;
	}
	return true;
}

// La directive cgiPath a été trouvé dans un bloc location
bool cgiPathIsHere(bool& cgiFound, t_location& ref, std::istringstream& stream, std::string& word,
				   const std::string& keyword, const std::ostringstream& lineIndex)
{
	if (cgiFound == true)
		return configError("[ ERROR ] cgiPathIsHere() - " + keyword + " [" + lineIndex.str() + ']' +
							   S_ERR_DUP,
						   false, 0);
	if (!(stream >> ref.cgiPath))
		return configError("[ ERROR ] cgiPathIsHere() - " + keyword + " [" + lineIndex.str() + ']' +
							   S_ERR_NO_VAL,
						   false, 0);
	if (stream >> word)
		return configError("[ ERROR ] cgiPathIsHere() - " + keyword + " [" + lineIndex.str() + ']' +
							   S_ERR_TM_VAL,
						   false, 0);
	cgiFound = true;
	if (ref.redirect.first != -1)
		ref.cgiPath = std::string();
	return true;
}

// Procède au switch de parseLoca()
bool parseLocaSwitch(t_location& ref, std::istringstream& stream, std::string& word,
					 std::ifstream& config, size_t& configLine, const std::ostringstream& lineIndex,
					 bool& stop, bool& stopBis, bool& autoindexFound, bool& limitFound,
					 bool& rootFound, bool& cgiFound)
{
	const std::string keywords[] = {"return",	"autoIndex", "limitExcept", "root",	  "index",
									"location", "}",		 "#",			"cgiPath"};
	const std::vector< std::string > keys(keywords,
										  keywords + sizeof(keywords) / sizeof(std::string));
	switch (std::distance(keys.begin(), std::find(keys.begin(), keys.end(), word)))
	{
	case (0):
		if (!returnIsHere(std::pair< short, std::string >(), ref, stream, word, keywords[0],
						  lineIndex))
			return false;
		break;
	case (1):
		if (!autoIndexIsHere(autoindexFound, ref, stream, word, keywords[1], lineIndex))
			return false;
		break;
	case (2):
		if (limitFound == true)
			return configError("[ ERROR ] parseLocaSwitch() - " + keywords[2] + " [" +
								   lineIndex.str() + ']' + S_ERR_DUP,
							   false, 0);
		if (!parseNewLimit(config, stream, ref, configLine))
			return false;
		limitFound = true;
		break;
	case (3):
		if (!rootIsHere(rootFound, ref, stream, word, keywords[3], lineIndex))
			return false;
		break;
	case (4):
		if (!indexIsHere(ref, stream, word, keywords[4], lineIndex))
			return false;
		break;
	case (5):
		if (!setupLoca(config, stream, NULL, &ref, configLine))
			return false;
		break;
	case (6):
		stop = true;
		break;
	case (7):
		stopBis = true;
		break;
	case (8):
		if (!cgiPathIsHere(cgiFound, ref, stream, word, keywords[8], lineIndex))
			return false;
		break;
	default:
		return configError("[ ERROR ] parseLocaSwitch() - Unknown keyword [" + lineIndex.str() +
							   "]: " + word,
						   false, 0);
	}
	return true;
}

// Parse un bloc location
bool parseLoca(std::ifstream& config, const std::string& name, t_location& ref, size_t& configLine)
{
	ref = initLoca(name);

	bool autoindexFound = false;
	bool limitFound = false;
	bool rootFound = false;
	bool cgiFound = false;
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
			if (!parseLocaSwitch(ref, stream, word, config, configLine, lineIndex, stop, stopBis,
								 autoindexFound, limitFound, rootFound, cgiFound))
				return false;
	}
	std::ostringstream lineIndex;
	lineIndex << configLine;
	if (!stop)
		return configError(std::string("[ ERROR ] parseLoca() - location") + S_ERR_NO_BRK, false,
						   0);
	return true;
}

// Initialise et assigne un bloc location
bool setupLoca(std::ifstream& config, std::istringstream& stream, t_serv* servPtr,
			   t_location* locaPtr, size_t& configLine)
{
	std::ostringstream lineIndex;
	lineIndex << configLine;

	std::string name;
	std::string word;
	if (!(stream >> name))
		return configError(
			"[ ERROR ] setupLoca() - location [" + lineIndex.str() + ']' + S_ERR_NO_VAL, false, 0);
	if (!(stream >> word))
		return configError(
			"[ ERROR ] setupLoca() - location [" + lineIndex.str() + ']' + S_ERR_NO_VAL, false, 0);
	if (word != "{")
		return configError(
			"[ ERROR ] setupLoca() - location [" + lineIndex.str() + ']' + S_ERR_NO_BRK, false, 0);
	if (stream >> word)
		return configError(
			"[ ERROR ] setupLoca() - location [" + lineIndex.str() + ']' + S_ERR_TM_VAL, false, 0);

	const size_t size = (servPtr ? (*servPtr).locations.size() : (*locaPtr).locations.size());
	for (size_t x = 0; x < size; x++)
	{
		if (servPtr && (*servPtr).locations[x].name == name)
			return configError("[ ERROR ] setupLoca() - Server have duplicated location: " + name,
							   false, 0);
		else if (locaPtr && (*locaPtr).locations[x].name == name)
			return configError("[ ERROR ] setupLoca() - Location have duplicated location: " + name,
							   false, 0);
	}
	if (locaPtr)
	{
		if (name.find(locaPtr->name) == std::string::npos)
			return configError("[ ERROR ] setupLoca() - SubLocation doesn't match with a parent: " +
								   name,
							   false, 0);
		if (name.size() == locaPtr->name.size())
			return configError("[ ERROR ] setupLoca() - Location have duplicated location: " + name,
							   false, 0);
	}

	t_location location;
	if (!parseLoca(config, name, location, configLine))
		return false;
	if (servPtr)
		(*servPtr).locations.push_back(location);
	else
		(*locaPtr).locations.push_back(location);
	return true;
}
