#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <new>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <arpa/inet.h>
#include <csignal>
#include <dirent.h>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// --------------------------------------------------

// Pour les pipes
#define READ 0
#define WRITE 1

// Valeur en octet d'1ko
#define ONE_KO 1024

// Valeur en octet d'1mo
#define ONE_MO 1024 * ONE_KO

// Size du buffer de read()
#define BUFFER_SIZE 2 * ONE_KO

// Taille maximum d'un header HTTP
#define HEADER_MAX_SIZE 4 * ONE_KO

// Nombre maximum de connexions en attente avant que le serveur ne commence à les refuser
#define MAX_CONNECTION 42

// Taille maximum d'un URI
#define URI_MAX_SIZE 1024

// REQUEST_TIMEOUT (en secondes)
#define REQ_TIMEOUT 10

// Messages d'erreur
#define S_ERR_NO_VAL ": No value"
#define S_ERR_TM_VAL ": Too many values"
#define S_ERR_BAD_VAL ": Invalid value"
#define S_ERR_NO_BRK ": Missing brackets"
#define S_ERR_DUP ": Duplicata"

// Stock des informations d'adresses génériques
typedef struct sockaddr t_sockaddr;

// Stock des informations d'adresses IPv4
typedef struct sockaddr_in t_sockaddr_in;

// Stock les informations d'un réseau (getaddrinfo())
typedef struct addrinfo t_addrinfo;

// Stock les informations d'un path (stat())
typedef struct stat t_stat;

// Stock les informations d'un répertoire (opendir()/closedir())
typedef struct dirent t_dirent;

// Stock des file descriptor (poll())
typedef struct pollfd t_pollfd;

// Timeout (gettimeofday())
typedef struct timeval t_timeval;

// --------------------------------------------------

// Données d'un bloc location
typedef struct s_location
{
	bool autoIndex;
	std::string name;
	std::string root;
	std::pair< short, std::string > redirect;
	std::map< const std::string, std::vector< std::pair< const uint32_t, const bool > > >
		limitExcept;
	std::vector< std::string > indexPage;
	std::vector< struct s_location > locations;
	std::string cgiPath;
} t_location;

// Données d'un bloc serveur
typedef struct s_serv
{
	int maxBodySize;
	std::vector< std::pair< const uint32_t, const uint16_t > > ip_port;
	std::vector< std::string > serverName;
	std::map< const unsigned short, const std::string > errorPages;
	std::vector< t_location > locations;
} t_serv;

// Singleton
typedef struct s_data
{
	std::vector< t_serv > servList;
	std::map< const std::string, std::string > cgiData;
	std::map< const short, const std::string > codes;
	std::map< const std::string, const std::string > MIMEtypes;

	std::vector< std::pair< const uint32_t, const uint16_t > > ip_port;
	std::vector< std::pair< int, t_sockaddr_in > > sock_addr;
	std::pair< std::vector< t_pollfd >, std::vector< std::pair< const int, t_timeval* > > > pollz;
	std::vector< int > toKeep;
	std::vector< std::pair< int, std::map< const std::string, std::string > > > headers;
	std::vector< std::pair< int, std::vector< char > > > bodies;
	bool exiting;
} t_data;

// --------------------------------------------------

t_data* data(void);
void handleSIG(const int signum);
bool init(std::ifstream& config);
short configError(const std::string& msg, const bool& useErrno, const short& ret);
bool parseServ(std::ifstream& config, size_t& configLine);
bool setupLoca(std::ifstream& config, std::istringstream& stream, t_serv* servPtr,
			   t_location* locaPtr, size_t& configLine);
t_location initLoca(const std::string& name);
bool maxBodySizeIsHere(int& maxBodySize, std::istringstream& stream, std::string& word,
					   const std::string& keywords, const std::ostringstream& lineIndex);
int getmaxBodySize(std::string& size);
long str2id(const std::string& str, const bool& check);
void printConfig(void);
bool setup(const size_t& x, const bool& reUP);
bool serv(void);
bool read2socket(const size_t& x, const int& id, size_t& servIndex, t_location& location,
				 std::string& serverName);
short isValidPath(const std::string& path);
bool sendDefaultError(const int& id, const unsigned short& codeRef, const std::string& message,
					  const int& socket);
bool sendError(const int& id, unsigned short codeRef, const std::string& message, const int& socket,
			   t_serv& serv);
bool redirect(std::string& answ, std::pair< short, std::string > redir,
			  const std::map< const std::string, std::string >::iterator& URI);
bool ftGet(const int& id, t_serv& serv, t_location& location, std::string& answ, const int& socket,
		   const std::pair< const uint32_t, const uint16_t >& ip_port,
		   const std::string& serverName);
bool ftPost(const int& id, t_serv& serv, t_location& location, std::string& answ, const int& socket,
			const std::pair< const uint32_t, const uint16_t >& ip_port,
			const std::string& serverName, const bool& chunked);
bool ftDelete(const int& id, t_location& location, std::string& answ, const int& socket);
std::string createAnsw(const std::string& body, const unsigned short& codeRef,
					   const std::string& message, const std::string& contentType,
					   const std::string& redirect);
bool fillBody(const int& id, std::string& type, const std::string& path, t_serv& serv,
			  std::string& answ, const int& socket);
bool runCGI(const int& id, t_serv& serv, t_location& location, std::string& answ, const int& socket,
			const std::pair< const uint32_t, const uint16_t >& ip_port,
			const std::string& serverName);
void keepNoMore(const int& socket);

namespace ft
{
void* memset(void* ptr, const short& value, const size_t& size);
void memcpy(void* dest, void* src, const size_t& size);
std::vector< std::string > split(const std::string& str, const char& delim);
std::pair< std::string, std::string > psplit(const std::string& str, const char& delim);
}

// Codes retour-------------------------------------->>

// [ Informationnel ]
#define INFO_CONTINUE 100
//	Continue
// 	- Requête reçue, en attente de la suite
// 	- En-tête de la requête doit contenir "Expect: 100-continue"
// 101 Switching Protocole
// 	- Demande de changement de protocole acceptée
// 	- En-tête de la requête doit contenir "Upgrade"
// 102 Processing
// 	- Demande en cours de traitement
// 	- Si la requête est longue à traiter, le serveur peut le faire savoir au client
// 	- Évite que le client ne pense que la demande a échoué
// 	- Cette réponse est renvoyée durant le traitement de la requête
// 103 Early Hints
// 	- Envoie d'une réponse préliminaire
// 	- Peut permettre au client de traiter la réponse plus rapidement
// ------------------------------------------------------------

// [ Succès ]
#define OK_OK 200
//	OK
// 	- Requête traitée avec succès
#define OK_CREATED 201
//	Created
// 	- Requête traitée avec succès, une nouvelle ressource a été créée
// 202 Accepted
// 	- Requête acceptée, la reponse prendra du temps
// 	- Contrairement au code 102, cette réponse est renvoyée à l'acceptation de la requête
// 203 Non-Authoritative Information
// 	- La réponse vient d'un autre serveur
#define OK_NOCONTENT 204
//	No Content
// 	- Requête traitée avec succès, pas de ressource à renvoyé
// 	- ex : reponse à DELETE ou POST
// 205 Reset Content
// 	- Requête traitée avec succès, la page doit simplement être reset
// 	- ex : Formulaire à remplir
// 206 Partial Content
// 	- Requête traitée avec succès, la réponse est une réponse partielle
// 	- ex : Téléchargement d'une ressource en plusieurs parties
// 207 Multi-Status
// 	- Requête traitée avec succès, la réponse contient différentes parties
// 	- Chaque partie contient potentiellement un code différent
// 208 Already Reported
// 	- Requête traitée avec succès, la réponse est une réponse partielle
// 	- Le client doit récupérer le reste d'une autre source
// ------------------------------------------------------------

// [ Redirection ]
// 300 Multiple Choices
// 	- Requête traitée avec succès, le client doit choisir parmi plusieurs réponses possibles
#define RE_MOVED 301
//	Moved Permanently
// 	- Requête traitée avec succès, la ressource doit être récupérée d'une autre adresse
// 	- La ressource demandée a été déplacée de manière permanente
#define RE_FOUND 302
//	Found
// 	- Requête traitée avec succès, la ressource doit être récupérée d'une autre adresse
// 	- La ressource demandée a été déplacée de manière temporaire
// 303 See Other
// 	- Requête traitée avec succès, la ressource doit être récupérée d'une autre adresse
// 	- La prochaine méthode doit être GET ou HEAD
// 	- ex : Une requête POST qui crée un compte et qui doit rediriger l'utilisateur vers une page de
// connexion 304 Not Modified
// 	- Requête trERR_BAD_REQaitée avec succès, la ressource n'a pas été modifiée depuis le dernier
// accès
// 	- Le client peut utiliser le cache
// 305 Use Proxy [DEPRECATED]
// 306 Switch Proxy [DEPRECATED]
#define RE_TEMP 307
//	Temporary Redirect
// 	- Requête traitée avec succès, la ressource doit être récupérée d'une autre adresse
// 	- La ressource demandée a été déplacée de manière temporaire
// 	- Méthode utilisée inchangée
#define RE_PERM 308
//	Permanent Redirect
// 	- Requête traitée avec succès, la ressource doit être récupérée d'une autre adresse
// 	- La ressource demandée a été déplacée de manière permanente
// 	- Méthode utilisée inchangée
// ------------------------------------------------------------

// [ Erreur client ]
#define ERR_BAD_REQ 400
//	Bad Request
// 	- La requête du client est mal formée
// 401 Unauthorized
// 	- Une authentification non fournie est requise
// 402 Payment Required
// 	- La requête ne peut pas être traitée avant que le client effectue un paiement
#define ERR_FORBIDDEN 403
//	Forbidden
// 	- Le client tente d'accéder à une ressource pour laquelle il n'a pas les autorisations
// nécessaires
#define ERR_NOT_FOUND 404
//	Not Found
// 	- La ressource demandée n'a pas été trouvée sur le serveur
#define ERR_UNALW_METHOD 405
//	Method Not Allowed
// 	- La méthode HTTP utilisée dans la requête n'est pas autorisée pour la ressource demandée
#define ERR_NOT_ACCEPT 406
//	Not Acceptable
// 	- Le serveur ne peut pas fournir une ressource acceptable par le client
// 	- En-tête de la requête doit contenir "Accept: X"
// 407 Proxy Authentication Required
// 	- La connexion au serveur proxy nécessite une authentification
#define ERR_REQ_TIMEOUT 408
//	Request Timeout
// 	- Le serveur n'a pas reçu de requête complète du client dans le délai imparti
// 409 Conflict
// 	- La requête du client entre en conflit avec l'état actuel de la ressource
// 	- ex : Deux clients tentent de modifier la même ressource simultanément
// 410 Gone
// 	- La ressource précédemment disponible n'est plus disponible
#define ERR_LEN_REQ 411
//	Length Required
// 	- La requête du client ne contient pas la longueur du contenu
// 412 Precondition Failed
// 	- Une condition préalable spécifiée dans la requête du client n'est pas satisfaite
#define ERR_BODY_2LARG 413
//	Payload Too Large
// 	- Body de la requête dépasse la limite définie par le serveur
#define ERR_URI2LONG 414
//	URI Too Long
// 	- L'URI de la requête du client est trop long
#define ERR_UNKN_TYPE 415
//	Unsupported Media Type
// 	- Le type de contenu de la requête du client n'est pas pris en charge par le serveur
// 416 Range Not Satisfiable
// 	- La size de la ressource demandée n'est pas dans la range spécifiée
// 417 Expectation Failed
// 	- La directive 'Expect' de la requête ne peut pas être résolue
// 418 I'm a teapot
// 	- Code erreur de test pour vérifier la capacité des clients à gérer les codes de réponse HTTP
// inattendus 419 Authentication Timeout
// 	- Indique qu'une authentication a expirée
// 420 Enhance Your Calm
// 	- Indique qu'un client a dépassé une limite de fréquence de requête ou a été bloqué pour
// d'autres raisons de sécurité 421 Misdirected Request
// 	- Une redirection est incorrecte
// 422 Unprocessable Entity
// 	- La requête ne peut pas être traitée en raison d'une violation des règles de l'application
// 	- ex : Créer une ressource avec des données manquantes ou incorrectes
// 423 Locked
// 	- La ressource demandée est verrouillée et ne peut pas être modifiée
// 424 Failed Dependency
// 	- La demande a échoué en raison d'une dépendance de la ressource
// 	- ex : Suppression d'un fichier dont un autre service ou application dépend
// 425 Too Early
// 	- Un client tente d'accéder à une ressource qui n'est pas encore disponible en raison de
// restrictions 426 Upgrade Required
// 	- Le serveur requiert que la requête du client soit mise à niveau vers un protocole différent
// 427 [UNATRIBUTED]
// 428 Precondition Required
// 	- La requête du client n'inclut pas les conditions définies par le serveur pour être traitée
// 429 Too Many Requests
// 	- Le client a dépassé la limite de fréquence de requêtes pour une ressource
// 430 [UNATRIBUTED]
#define ERR_HEADER2LARG 431
//	Request Header Fields Too Large
// 	- Les en-têtes de la requête HTTP sont trop grands pour être traités par le serveur
// ------------------------------------------------------------

// [ Erreur serveur ]
#define ERR_INTERN 500
//	Internal Server Error
// 	- Erreur de programmation, de configuration, panne matérielle, call foireux, etc..
#define ERR_UNKN_METHOD 501
//	Not Implemented
// 	- La méthode utilisée n'est pas prise en charge
#define ERR_BAD_GATE 502
// Bad Gateway
// 	- Réponse d'un autre serveur/application (CGI par exemple) incorrecte
// 503 Service Unavailable
// 	- Le serveur est temporairement indisponible ou en surcharge
// 504 Gateway Timeout
// 	- Réponse d'un autre serveur/application (CGI par exemple) n'a pas reçu à temps
#define ERR_HTTP_VERSION 505
//	HTTP Version Not Supported
// 	- La version HTTP de la requête du client n'est pas prise en charge par le serveur
// 506 Variant Also Negotiates
// 	- La variante de la ressource demandée n'a pas été résolue
// 	- ex : Demande dans une langue qui n'est plus disponible
// 507 Insufficient Storage
// 	- Le serveur ne peut pas répondre à la demande du client car il manque d'espace de stockage
// 508 Loop Detected
// 	- Indiquer qu'une boucle de redirection infinie a été détectée
// 	- ex : Une demande spécifique créant une boucle infinie de redirection
// 509 Bandwidth Limit Exceeded
// 	- Indiquer que la demande du client dépasse la limite de bande passante allouée pour la
// ressource demandée 510 Not Extended
// 	- Extension de protocole HTTP nécessaire
// 	- ex : Requête HTTP pour une ressource qui requiert HTTPS
// 511 Network Authentication Required
// 	- Une authentification est requise pour accéder a la ressource
// ------------------------------------------------------------

#endif
