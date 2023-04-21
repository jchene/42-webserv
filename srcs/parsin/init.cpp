#include "../../includes/header.hpp"

// Print un message d'erreur puis return un short
short configError(const std::string& msg, const bool& useErrno, const short& ret)
{
	std::cerr << msg << std::endl;
	if (useErrno)
		std::cerr << std::strerror(errno) << std::endl;
	return ret;
}

// Return un clientMaxBodySize
int getmaxBodySize(std::string& size)
{
	if (size[size.length() - 1] == 'm')
		size.erase(size.end() - 1);
	if (size.find_first_not_of("0123456789") != std::string::npos)
		return -1;
	return std::atoi(size.c_str());
}

// Set les types de fichier pris en charge
void setMIMEtypes(void)
{
	// Texte à afficher
	data()->MIMEtypes.insert(
		std::pair< const std::string, const std::string >("plain", "text/plain"));
	data()->MIMEtypes.insert(
		std::pair< const std::string, const std::string >("html", "text/html"));
	data()->MIMEtypes.insert(std::pair< const std::string, const std::string >("css", "text/css"));

	// Application à exécuter
	data()->MIMEtypes.insert(
		std::pair< const std::string, const std::string >("php", "application/x-httpd-php"));
	data()->MIMEtypes.insert(
		std::pair< const std::string, const std::string >("py", "application/x-python"));
	data()->MIMEtypes.insert(
		std::pair< const std::string, const std::string >("pdf", "application/pdf"));
	data()->MIMEtypes.insert(
		std::pair< const std::string, const std::string >("xml", "application/xml"));
	data()->MIMEtypes.insert(
		std::pair< const std::string, const std::string >("json", "application/json"));
	data()->MIMEtypes.insert(
		std::pair< const std::string, const std::string >("js", "application/javascript"));

	// Média
	data()->MIMEtypes.insert(
		std::pair< const std::string, const std::string >("ico", "image/vnd.microsoft.icon"));
	data()->MIMEtypes.insert(
		std::pair< const std::string, const std::string >("jpeg", "image/jpeg"));
	data()->MIMEtypes.insert(std::pair< const std::string, const std::string >("jpg", "image/jpg"));
	data()->MIMEtypes.insert(std::pair< const std::string, const std::string >("png", "image/png"));
	data()->MIMEtypes.insert(std::pair< const std::string, const std::string >("gif", "image/gif"));
	data()->MIMEtypes.insert(
		std::pair< const std::string, const std::string >("mpeg", "audio/mpeg"));
	data()->MIMEtypes.insert(std::pair< const std::string, const std::string >("mp4", "video/mp4"));
}

// Set les codes de retour
void setDefCodes(void)
{
	// 1xx
	data()->codes.insert(std::pair< const short, const std::string >(INFO_CONTINUE, "Continue"));

	// 2xx
	data()->codes.insert(std::pair< const short, const std::string >(OK_OK, "OK"));
	data()->codes.insert(std::pair< const short, const std::string >(OK_CREATED, "Created"));
	data()->codes.insert(std::pair< const short, const std::string >(OK_NOCONTENT, "No Content"));

	// 3xx
	data()->codes.insert(
		std::pair< const short, const std::string >(RE_MOVED, "Moved Permanently"));
	data()->codes.insert(std::pair< const short, const std::string >(RE_FOUND, "Found"));
	data()->codes.insert(
		std::pair< const short, const std::string >(RE_TEMP, "Temporary Redirect"));
	data()->codes.insert(
		std::pair< const short, const std::string >(RE_PERM, "Permanent Redirect"));

	// 4xx
	data()->codes.insert(std::pair< const short, const std::string >(ERR_BAD_REQ, "Bad Request"));
	data()->codes.insert(std::pair< const short, const std::string >(ERR_FORBIDDEN, "Forbidden"));
	data()->codes.insert(std::pair< const short, const std::string >(ERR_NOT_FOUND, "Not Found"));
	data()->codes.insert(
		std::pair< const short, const std::string >(ERR_UNALW_METHOD, "Method Not Allowed"));
	data()->codes.insert(
		std::pair< const short, const std::string >(ERR_NOT_ACCEPT, "Not Acceptable"));
	data()->codes.insert(
		std::pair< const short, const std::string >(ERR_REQ_TIMEOUT, "Request Timeout"));
	data()->codes.insert(
		std::pair< const short, const std::string >(ERR_LEN_REQ, "Length Required"));
	data()->codes.insert(
		std::pair< const short, const std::string >(ERR_BODY_2LARG, "Payload Too Large"));
	data()->codes.insert(std::pair< const short, const std::string >(ERR_URI2LONG, "URI Too Long"));
	data()->codes.insert(
		std::pair< const short, const std::string >(ERR_UNKN_TYPE, "Unsupported Media Type"));
	data()->codes.insert(std::pair< const short, const std::string >(
		ERR_HEADER2LARG, "Request Header Fields Too Large"));

	// 5xx
	data()->codes.insert(
		std::pair< const short, const std::string >(ERR_INTERN, "Internal Server Error"));
	data()->codes.insert(
		std::pair< const short, const std::string >(ERR_UNKN_METHOD, "Not Implemented"));
	data()->codes.insert(std::pair< const short, const std::string >(ERR_BAD_GATE, "Bad Gateway"));
	data()->codes.insert(std::pair< const short, const std::string >(ERR_HTTP_VERSION,
																	 "HTTP Version Not Supported"));
}

// Initialise les variables d'environnement à envoyer au CGI
void initCGI(void)
{
	// Code de status
	data()->cgiData["REDIRECT_STATUS"] = "200";
	// Version du protocole CGI utilisé
	data()->cgiData["GATEWAY_INTERFACE"] = "CGI/1.1";
	// -----------------------------------------------------------

	// 'User-Agent' du header de la requête
	data()->cgiData["HTTP_USER_AGENT"] = std::string();
	// 'Connection' du header de la requête
	data()->cgiData["HTTP_CONNECTION"] = std::string();
	// 'Accept' du header de la requête
	data()->cgiData["HTTP_ACCEPT"] = std::string();
	// -----------------------------------------------------------

	// Adresse IP du socket ayant écouté la requête
	data()->cgiData["SERVER_ADDR"] = std::string();
	// Port utiliser par le socket ayant écouté la requête
	data()->cgiData["SERVER_PORT"] = std::string();
	// Nom du bloc serveur qui gère la requête
	data()->cgiData["SERVER_NAME"] = std::string();
	// Version du protocole HTTP utilisée par la requête
	data()->cgiData["SERVER_PROTOCOL"] = "HTTP/1.1";
	// Nom et version de notre serveur
	data()->cgiData["SERVER_SOFTWARE"] = "webServ";
	// -----------------------------------------------------------

	// Adresse IP du client ayant initié la requête
	data()->cgiData["REMOTE_ADDR"] = std::string();
	// Port utilisé par le client ayant initié la requête
	data()->cgiData["REMOTE_PORT"] = std::string();
	// Nom d'hôte du client ayant initié la requête
	data()->cgiData["REMOTE_HOST"] = std::string();
	// Longueur (en octet) du corps de la requête POST
	data()->cgiData["CONTENT_LENGTH"] = std::string();
	// Type de contenu (MIME) du corps de la requête POST
	data()->cgiData["CONTENT_TYPE"] = std::string();
	// -----------------------------------------------------------

	// Méthode de la requête
	data()->cgiData["REQUEST_METHOD"] = std::string();
	// URI de la requête
	data()->cgiData["REQUEST_URI"] = std::string();
	// Chemin complet vers le root de la location qui traite la requête
	data()->cgiData["DOCUMENT_ROOT"] = std::string();
	// Partie de l'URI indiquant le script que le CGI doit exécuter
	data()->cgiData["SCRIPT_NAME"] = std::string();
	// Chemin complet vers SCRIPT_NAME
	data()->cgiData["SCRIPT_FILENAME"] = std::string();
	// Chemin d'accès supplémentaire potentiellement utile pour le script que le CGI exécute
	data()->cgiData["PATH_INFO"] = std::string();
	// Chemin complet vers PATH_INFO
	data()->cgiData["PATH_TRANSLATED"] = std::string();
	// (URI après le '?') Utile pour exécuter le CGI via une requête GET
	data()->cgiData["QUERY_STRING"] = std::string();
}

// clientMaxBodySize a été trouvé dans la config
bool maxBodySizeIsHere(int& maxBodySize, std::istringstream& stream, std::string& word,
					   const std::string& keyword, const std::ostringstream& lineIndex)
{
	if (maxBodySize != -1)
		return configError("[ ERROR ] maxBodySizeIsHere() - " + keyword + " [" + lineIndex.str() +
							   ']' + S_ERR_TM_VAL,
						   false, 0);
	if (!(stream >> word))
		return configError("[ ERROR ] maxBodySizeIsHere() - " + keyword + " [" + lineIndex.str() +
							   ']' + S_ERR_NO_VAL,
						   false, 0);
	if ((maxBodySize = getmaxBodySize(word)) < 0)
		return configError("[ ERROR ] maxBodySizeIsHere() - " + keyword + " [" + lineIndex.str() +
							   ']' + S_ERR_BAD_VAL,
						   false, 0);
	if (stream >> word)
		return configError("[ ERROR ] maxBodySizeIsHere() - " + keyword + " [" + lineIndex.str() +
							   ']' + S_ERR_TM_VAL,
						   false, 0);
	return true;
}

// Un bloc serveur a été trouvé dans la config
bool serverBlockIsHere(std::istringstream& stream, std::string& word, std::ifstream& config,
					   size_t& configLine, const std::string& keyword,
					   const std::ostringstream& lineIndex)
{
	if (!(stream >> word))
		return configError("[ ERROR ] serverBlockIsHere() - " + keyword + " [" + lineIndex.str() +
							   ']' + S_ERR_NO_VAL,
						   false, 0);
	if (word != "{")
		return configError("[ ERROR ] serverBlockIsHere() - " + keyword + " [" + lineIndex.str() +
							   ']' + S_ERR_NO_BRK,
						   false, 0);
	if (stream >> word)
		return configError("[ ERROR ] serverBlockIsHere() - " + keyword + " [" + lineIndex.str() +
							   ']' + S_ERR_TM_VAL,
						   false, 0);
	return parseServ(config, configLine);
}

// Initialise les données du serveur
void initData(void)
{
	data();
	data()->exiting = false;
	signal(SIGINT, handleSIG);
	signal(SIGQUIT, handleSIG);
	setDefCodes();
	setMIMEtypes();
	initCGI();
}

// Procède au switch de init()
bool initSwitch(std::istringstream& stream, std::string& word, std::ifstream& config,
				size_t& configLine, const std::ostringstream& lineIndex, int& maxBodySize,
				bool& stop)
{
	const std::string keywords[] = {"#", "clientMaxBodySize", "server"};
	const std::vector< std::string > keys(keywords,
										  keywords + sizeof(keywords) / sizeof(std::string));
	switch (std::distance(keys.begin(), std::find(keys.begin(), keys.end(), word)))
	{
	case (0):
		stop = true;
		break;
	case (1):
		if (!maxBodySizeIsHere(maxBodySize, stream, word, keywords[1], lineIndex))
			return false;
		break;
	case (2):
		if (!serverBlockIsHere(stream, word, config, configLine, keywords[2], lineIndex))
			return false;
		break;
	default:
		return configError("[ ERROR ] initSwitch() - Unknown keyword [" + lineIndex.str() +
							   "]: " + word,
						   false, 0);
	}
	return true;
}

// Setup de webServ
bool init(std::ifstream& config)
{
	initData();
	int maxBodySize = -1;

	size_t configLine = 0;
	std::string line;
	while (std::getline(config, line))
	{
		std::ostringstream lineIndex;
		configLine++;
		lineIndex << configLine;

		std::istringstream stream(line);
		std::string word;
		bool stop = false;
		while (!stop && !stream.fail() && (stream >> word))
			if (!initSwitch(stream, word, config, configLine, lineIndex, maxBodySize, stop))
				return false;
	}
	if (maxBodySize < 0)
		maxBodySize = 10;
	for (size_t x = 0; x < data()->servList.size(); x++)
		if (data()->servList[x].maxBodySize == -1)
			data()->servList[x].maxBodySize = maxBodySize;
	return true;
}
