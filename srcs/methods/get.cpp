#include "../../includes/header.hpp"

// Set le début du fichier html de l'autoIndex
void set2html(std::stringstream& html, const std::string& path)
{
	html << "<!DOCTYPE html>"
		 << "<meta charset=\"utf-8\"><html>"
		 << "<head>"
		 << "<title>[INDEX] " << path << "</title>"
		 << "<style>"
		 << "body"
		 << "{font-family: serif; background-image: "
			"url(\"https://www.pcclean.io/wp-content/uploads/2020/4/VbqUBr.jpg\"); "
			"background-size: cover; background-position: center;}"
		 << "table"
		 << "{max-width: 800px; background-color: rgba(0, 0, 0, 0.5);}"
		 << "td"
		 << "{text-align: left;}"
		 << "td a"
		 << "{color: white; text-decoration: none; font-size: 30px;}"
		 << ".container"
		 << "{max-width: 1000px; color: white; font-size: 30px;}"
		 << "</style>"
		 << "</head>";
}

// Crée l'autoIndex d'une réponse GET
bool autoIndexON(const int& id, std::string& path, t_serv& serv, std::string& answ,
				 const int& socket, const std::string& root)
{
	std::stringstream html;
	DIR* dir;
	t_dirent* ent;

	if (*(path.end() - 1) != '/')
		path += '/';
	if ((dir = opendir(path.c_str())) != NULL)
	{
		set2html(html, path);
		html << "<body><div class=\"container\">"
			 << "<h1>Contenu de " << path << "</h1>"
			 << "<table>";
		const std::string relativePath = path.substr(root.length());
		while ((ent = readdir(dir)) != NULL)
		{
			t_stat fileStat;
			std::string fileName = ent->d_name;
			const std::string filePath = path + "/" + fileName;
			if (stat(filePath.c_str(), &fileStat) == -1)
			{
				closedir(dir);
				return sendError(id, ERR_INTERN, data()->codes[ERR_INTERN], socket, serv);
			}
			if (S_ISDIR(fileStat.st_mode))
				fileName += "/";
			html << "<tr><td><a href=\"" << relativePath << fileName << "\">" << fileName
				 << "</a></td></tr>";
		}
		html << "</table>"
			 << "</div></body></html>";
	}
	else
	{
		closedir(dir);
		return sendError(id, ERR_FORBIDDEN, data()->codes[ERR_FORBIDDEN], socket, serv);
	}
	answ = createAnsw(html.str(), OK_OK, data()->codes[OK_OK], "text/html", std::string());
	closedir(dir);
	return true;
}

// La réponse est une redirection
bool redirect(std::string& answ, std::pair< short, std::string > redir,
			  const std::map< const std::string, std::string >::iterator& URI)
{
	const size_t pos = redir.second.find("$request_uri");
	if (pos != std::string::npos)
		redir.second = redir.second.substr(0, pos) + URI->second;
	answ = createAnsw(std::string(), redir.first, data()->codes[redir.first], std::string(),
					  "http://" + redir.second);
	return true;
}

// Cree un index par defaut
bool sendDefaultIndex(std::string& answ)
{
	const std::string body =
		"<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>Default "
		"index</title><style>body {font-family: serif;background-image: "
		"url(\"https://www.pcclean.io/wp-content/uploads/2020/4/VbqUBr.jpg\");background-position: "
		"center;text-align: center;color:white;font-size:23px;margin:0;}#main {min-width: "
		"1000px;max-width: 1000px;position: absolute;top: 50%;left: 50%;transform: translate(-50%, "
		"-80%);}#main-title {background-color: rgba(0, 0, 0, 0.5);padding: 1px;padding-left: "
		"10px;padding-right: 10px;}</style></head><body><div id=\"main\"><div "
		"id=\"main-title\"><h1>Ceci est l'index par defaut de notre "
		"serveur</h1><p>:3</p></div></div></body></html>";
	answ = createAnsw(body, OK_OK, data()->codes[OK_OK], "text/html", std::string());
	return true;
}

// La requête vise un répertoire
bool dirRequested(const int& id, t_serv& serv, t_location& location, std::string& answ,
				  const int& socket, std::string& path, short& ret,
				  const std::map< const std::string, std::string >::iterator& URI,
				  const std::string& root)
{
	if (location.autoIndex == true)
		return autoIndexON(id, path, serv, answ, socket, root);

	std::string indexName;
	std::string finalPath;
	size_t x;
	if (location.indexPage.empty())
		return sendDefaultIndex(answ);
	for (x = 0; x < location.indexPage.size(); x++)
	{
		indexName = location.indexPage[x];
		finalPath = path + '/' + indexName;
		ret = isValidPath(finalPath);
		if (ret == 0 || ret == 1)
			return sendError(id, ERR_INTERN, data()->codes[ERR_INTERN], socket, serv);
		if (ret == 2)
			break;
	}
	if (x == location.indexPage.size())
		return sendError(id, ERR_NOT_FOUND, data()->codes[ERR_NOT_FOUND], socket, serv);

	if (location.name == "/")
		return fillBody(id, ft::split(finalPath, '.').back(), finalPath, serv, answ, socket);
	else
		return redirect(
			answ,
			std::pair< short, std::string >(RE_MOVED, data()->headers[id].second["Host"] +
														  URI->second + '/' + indexName),
			URI);
	return true;
}

// Construit la réponse à une requête GET
bool ftGet(const int& id, t_serv& serv, t_location& location, std::string& answ, const int& socket,
		   const std::pair< const uint32_t, const uint16_t >& ip_port,
		   const std::string& serverName)
{
	const std::map< const std::string, std::string >::iterator URI =
		data()->headers[id].second.find("URI");
	if (!location.cgiPath.empty())
		return runCGI(id, serv, location, answ, socket, ip_port, serverName);
	if (location.redirect.first > 0)
		return redirect(answ, location.redirect, URI);

	const std::map< const std::string, std::string >::iterator rootIT =
		data()->headers[id].second.find("Root");
	std::string root;
	if (rootIT == data()->headers[id].second.end())
		root = "./www";
	else
		root = rootIT->second;
	std::string path = root + URI->second;
	short ret = isValidPath(path);
	if (ret < 0)
	{
		if (errno == ENOENT)
			return sendError(id, ERR_NOT_FOUND, data()->codes[ERR_NOT_FOUND], socket, serv);
		if (errno == EACCES)
			return sendError(id, ERR_FORBIDDEN, data()->codes[ERR_FORBIDDEN], socket, serv);
		return sendError(id, ERR_INTERN, data()->codes[ERR_INTERN], socket, serv);
	}
	if (ret == 0)
		return sendError(id, ERR_FORBIDDEN, data()->codes[ERR_FORBIDDEN], socket, serv);
	if (ret == 2)
		return fillBody(id, ft::split(URI->second, '.').back(), path, serv, answ, socket);
	return dirRequested(id, serv, location, answ, socket, path, ret, URI, root);
}
