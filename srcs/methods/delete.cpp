#include "../../includes/header.hpp"

// Construit la réponse à une requête DELETE
bool ftDelete(const int& id, t_location& location, std::string& answ, const int& socket)
{
	std::map< const std::string, std::string >::iterator it;

	it = data()->headers[id].second.find("Root");
	std::string root;
	if (it == data()->headers[id].second.end())
		root = "./www";
	else
		root = it->second;

	it = data()->headers[id].second.find("URI");
	const std::string filename = it->second.substr(
		it->second.find(location.name) + location.name.size() + 1, it->second.size());
	if (filename == "." || filename == ".." || filename.find_first_of("/") != std::string::npos)
		return sendDefaultError(id, ERR_FORBIDDEN, data()->codes[ERR_FORBIDDEN], socket);
	const std::string path = root + it->second;
	const short ret = isValidPath(path);
	if (ret < 0)
	{
		if (errno == ENOENT)
			return sendDefaultError(id, ERR_NOT_FOUND, data()->codes[ERR_NOT_FOUND], socket);
		if (errno == EACCES)
			return sendDefaultError(id, ERR_FORBIDDEN, data()->codes[ERR_FORBIDDEN], socket);
		return sendDefaultError(id, ERR_INTERN, data()->codes[ERR_INTERN], socket);
	}
	if (ret == 1)
	{
		if (rmdir(path.c_str()) != 0)
			return sendDefaultError(id, ERR_FORBIDDEN, data()->codes[ERR_FORBIDDEN], socket);
	}
	else if (remove(path.c_str()) != 0)
		return sendDefaultError(id, ERR_FORBIDDEN, data()->codes[ERR_FORBIDDEN], socket);
	answ = createAnsw(std::string(), OK_NOCONTENT, data()->codes[OK_NOCONTENT], std::string(),
					  std::string());
	return true;
}
