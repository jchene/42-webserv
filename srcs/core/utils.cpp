#include "../../includes/header.hpp"

// Remplis une zone mémoire de size fois value
void* ft::memset(void* ptr, const short& value, const size_t& size)
{
	char* index = static_cast< char* >(ptr);
	for (size_t x = 0; x < size; ++x)
		index[x] = static_cast< char >(value);
	return ptr;
}

// Copie size valeurs de src vers dst
void ft::memcpy(void* dst, void* src, const size_t& size)
{
	char* csrc = static_cast< char* >(src);
	char* cdst = static_cast< char* >(dst);
	for (size_t x = 0; x < size; x++)
		cdst[x] = csrc[x];
}

// Split une string par le delim
std::vector< std::string > ft::split(const std::string& str, const char& delim)
{
	std::istringstream stream(str);
	std::string line;
	std::vector< std::string > splited;
	while (std::getline(stream, line, delim))
		splited.push_back(line);
	return splited;
}

// Split une ligne en deux par le premier delim
std::pair< std::string, std::string > ft::psplit(const std::string& str, const char& delim)
{
	std::istringstream stream(str);
	std::string line;
	std::string buffer;
	std::pair< std::string, std::string > splited;
	std::getline(stream, line, delim);
	splited.first = line;
	while ((stream >> buffer))
		splited.second += buffer + " ";
	return splited;
}

// Print un bloc location
void printLocation(t_location& ref)
{
	std::cout << "- name: " << ref.name << std::endl;
	std::cout << "- root: " << ref.root << std::endl;
	std::cout << "- redirect: [ " << ref.redirect.first << " ] " << ref.redirect.second
			  << std::endl;

	std::cout << "- GET: " << std::endl;
	for (size_t a = 0; a < ref.limitExcept["GET"].size(); a++)
		std::cout << "\t" << ref.limitExcept["GET"][a].first << " -> "
				  << ref.limitExcept["GET"][a].second << std::endl;

	std::cout << "- POST: " << std::endl;
	for (size_t b = 0; b < ref.limitExcept["POST"].size(); b++)
		std::cout << "\t" << ref.limitExcept["POST"][b].first << " -> "
				  << ref.limitExcept["POST"][b].second << std::endl;

	std::cout << "- DELETE: " << std::endl;
	for (size_t c = 0; c < ref.limitExcept["DELETE"].size(); c++)
		std::cout << "\t" << ref.limitExcept["DELETE"][c].first << " -> "
				  << ref.limitExcept["DELETE"][c].second << std::endl;

	std::cout << "- autoIndex: " << ref.autoIndex << std::endl;
	std::cout << "- indexPage:" << std::endl;
	for (size_t d = 0; d < ref.indexPage.size(); d++)
		std::cout << "\t" << ref.indexPage[d] << std::endl;

	if (!ref.locations.empty())
	{
		for (size_t e = 0; e < ref.locations.size(); e++)
		{
			std::cout << "- [ Locations #" << e + 1 << " ]" << std::endl;
			printLocation(ref.locations[e]);
		}
	}
	std::cout << std::endl;
}

// Print la config
void printConfig(void)
{
	for (size_t a = 0; a < data()->servList.size(); a++)
	{
		std::cout << "[ Server #" << a + 1 << " ]" << std::endl;
		std::cout << "maxBodySize:" << data()->servList[a].maxBodySize << std::endl;

		std::cout << "listen:" << std::endl;
		for (size_t b = 0; b < data()->servList[a].ip_port.size(); b++)
			std::cout << "\t" << data()->servList[a].ip_port[b].first << ":"
					  << data()->servList[a].ip_port[b].second << std::endl;

		std::cout << "serverName:" << std::endl;
		for (size_t c = 0; c < data()->servList[a].serverName.size(); c++)
			std::cout << "\t" << data()->servList[a].serverName[c] << std::endl;

		std::cout << "errorPage:" << std::endl;
		for (size_t d = 0; d < data()->servList[a].errorPages.size(); d++)
			std::cout << "\t" << data()->servList[a].errorPages[d] << std::endl;

		for (size_t e = 0; e < data()->servList[a].locations.size(); e++)
		{
			std::cout << "[ Locations #" << e + 1 << " ]" << std::endl;
			printLocation(data()->servList[a].locations[e]);
		}
		std::cout << std::endl << "-------------------------------------" << std::endl;
	}
}

// Donne les informations d'un path
short isValidPath(const std::string& path)
{
	t_stat fileInfo;
	if (stat(path.c_str(), &fileInfo))
		return -1;
	if (S_ISDIR(fileInfo.st_mode))
		return 1;
	if (S_ISREG(fileInfo.st_mode))
		return 2;
	return 0;
}
