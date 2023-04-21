#include "../../includes/header.hpp"

// Rentre de la data dans un file
bool saveFile(const int& id, const int& socket, const std::string& filename)
{
	int output = open(filename.c_str(), O_WRONLY | O_CREAT | O_APPEND,
					  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (output < 0)
		return sendDefaultError(id, ERR_INTERN, data()->codes[ERR_INTERN], socket);

	if (!data()->bodies[id].second.empty())
	{
		const ssize_t count =
			write(output, &data()->bodies[id].second[0], data()->bodies[id].second.size());
		close(output);
		if (static_cast< size_t >(count) != data()->bodies[id].second.size())
		{
			keepNoMore(socket);
			return sendDefaultError(id, ERR_INTERN, data()->codes[ERR_INTERN], socket);
		}
	}
	else
		close(output);
	return true;
}

// multipart/form-data
bool freeChunkedBody(const int& id, const int& socket, const std::string& path,
					 const std::string& name)
{
	const std::map< const std::string, std::string >::iterator it =
		data()->headers[id].second.find("Chunked");
	if (it->second == "1")
	{
		if (name.empty())
			return sendDefaultError(id, ERR_BAD_REQ, data()->codes[ERR_BAD_REQ], socket);
		if (isValidPath(path + '/' + name) == 2)
			return sendDefaultError(id, ERR_UNALW_METHOD, data()->codes[ERR_UNALW_METHOD], socket);
		it->second = "2";
	}
	if (!saveFile(id, socket, path + '/' + name))
		return false;
	data()->bodies[id].second = std::vector< char >();
	return true;
}

// application/x-www-form-urlencoded
bool isFormData(const int& id, std::string& name,
				const std::map< const std::string, std::string >::iterator& it)
{
	size_t pos = it->second.find("boundary=");
	if (pos == std::string::npos)
		return false;

	const std::string boundary = "--" + it->second.substr(pos + 9);
	const std::string temp =
		std::string(data()->bodies[id].second.begin(), data()->bodies[id].second.end());
	if (temp.find(boundary + "\r\n") != 0)
		return false;
	pos = temp.find("\r\n\r\n");
	if (pos == std::string::npos)
		return false;

	const std::string metadata = temp.substr(0, pos + 4);
	pos = metadata.find("filename=\"");
	if (pos != std::string::npos)
	{
		const size_t end_pos = metadata.find("\"", pos + 10);
		if (end_pos != std::string::npos)
			name = metadata.substr(pos + 10, end_pos - (pos + 10));
	}
	for (size_t x = 0; x < metadata.size(); x++)
		data()->bodies[id].second.erase(data()->bodies[id].second.begin());
	for (size_t x = 0; x < boundary.size() + 6; x++)
		data()->bodies[id].second.erase(data()->bodies[id].second.end() - 1);
	return true;
}

// application/x-www-form-urlencoded
bool isXFrom(const int& id, std::string& name)
{
	const std::vector< std::string > splited = ft::split(
		std::string(data()->bodies[id].second.begin(), data()->bodies[id].second.end()), '&');
	if (splited.size() != 2)
		return false;
	size_t pos = splited.front().find("filename=");
	if (pos == std::string::npos)
		return false;
	name = splited.front().substr(pos + 9);
	pos = splited.back().find("content=");
	if (pos == std::string::npos)
		return false;
	for (size_t x = 0; x < name.size() + 18; x++)
		data()->bodies[id].second.erase(data()->bodies[id].second.begin());
	return true;
}

// application/octet-stream & text/plain
bool isDefault(const int& id, std::string& name)
{
	const std::vector< std::string > splited = ft::split(
		std::string(data()->bodies[id].second.begin(), data()->bodies[id].second.end()), '&');
	if (splited.size() != 2)
		return false;
	name = splited.front();
	for (size_t x = 0; x < name.size(); x++)
		data()->bodies[id].second.erase(data()->bodies[id].second.begin());
	data()->bodies[id].second.erase(data()->bodies[id].second.begin());
	return true;
}

// Appelle la fonction qui gere le Content-Type specifier
bool managedMIMES(const int& id, const std::string& path, std::string& answ, const int& socket,
				  const bool& chunked)
{
	std::string type;
	const std::map< const std::string, std::string >::iterator typeIT =
		data()->headers[id].second.find("Content-Type");
	if (typeIT == data()->headers[id].second.end())
		type = "application/octet-stream";
	else
		type = typeIT->second;

	const std::string keywords[] = {"application/octet-stream", "text/plain", "multipart/form-data",
									"application/x-www-form-urlencoded"};
	const std::vector< std::string > keys(keywords,
										  keywords + sizeof(keywords) / sizeof(std::string));
	std::string name;
	switch (std::distance(keys.begin(),
						  std::find(keys.begin(), keys.end(), ft::split(type, ';').front())))
	{
	case (0):
		if (!isDefault(id, name))
			return sendDefaultError(id, ERR_BAD_REQ, data()->codes[ERR_BAD_REQ], socket);
		break;
	case (1):
		if (!isDefault(id, name))
			return sendDefaultError(id, ERR_BAD_REQ, data()->codes[ERR_BAD_REQ], socket);
		break;
	case (2):
		if (!isFormData(id, name, typeIT))
			return sendDefaultError(id, ERR_BAD_REQ, data()->codes[ERR_BAD_REQ], socket);
		break;
	case (3):
		if (!isXFrom(id, name))
			return sendDefaultError(id, ERR_BAD_REQ, data()->codes[ERR_BAD_REQ], socket);
		break;
	default:
		return sendDefaultError(id, ERR_UNKN_TYPE, data()->codes[ERR_UNKN_TYPE], socket);
	}
	if (name == "." || name == ".." || name.find_first_of("/") != std::string::npos)
		return sendDefaultError(id, ERR_FORBIDDEN, data()->codes[ERR_FORBIDDEN], socket);
	if (chunked)
		return freeChunkedBody(id, socket, path, name);
	if (name.empty())
		return sendDefaultError(id, ERR_BAD_REQ, data()->codes[ERR_BAD_REQ], socket);
	if (isValidPath(path + '/' + name) == 2)
		return sendDefaultError(id, ERR_UNALW_METHOD, data()->codes[ERR_UNALW_METHOD], socket);
	if (!saveFile(id, socket, path + '/' + name))
		return false;
	answ = createAnsw(std::string(), OK_CREATED, data()->codes[OK_CREATED], std::string(),
					  std::string());
	return true;
}

// Construit la réponse à une requête POST
bool ftPost(const int& id, t_serv& serv, t_location& location, std::string& answ, const int& socket,
			const std::pair< const uint32_t, const uint16_t >& ip_port,
			const std::string& serverName, const bool& chunked)
{
	if (!chunked && !location.cgiPath.empty())
		return runCGI(id, serv, location, answ, socket, ip_port, serverName);

	const std::map< const std::string, std::string >::iterator rootIT =
		data()->headers[id].second.find("Root");
	std::string root;
	if (rootIT == data()->headers[id].second.end())
		root = "./www";
	else
		root = rootIT->second;
	const std::string path = root + data()->headers[id].second["URI"];
	const short ret = isValidPath(path);
	if (ret < 0)
	{
		if (errno == ENOENT)
			return sendDefaultError(id, ERR_NOT_FOUND, data()->codes[ERR_NOT_FOUND], socket);
		if (errno == EACCES)
			return sendDefaultError(id, ERR_FORBIDDEN, data()->codes[ERR_FORBIDDEN], socket);
		return sendDefaultError(id, ERR_INTERN, data()->codes[ERR_INTERN], socket);
	}
	if (ret == 0)
		return sendDefaultError(id, ERR_FORBIDDEN, data()->codes[ERR_FORBIDDEN], socket);
	if (ret == 2)
		return sendDefaultError(id, ERR_UNALW_METHOD, data()->codes[ERR_UNALW_METHOD], socket);
	return managedMIMES(id, path, answ, socket, chunked);
}
