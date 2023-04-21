#include "../../includes/header.hpp"

// Singleton Singleton Singleton-ton-ton
t_data* data(void)
{
	static t_data global;
	return &global;
}

// Gestion des signaux
void handleSIG(const int signum)
{
	(void)signum;
	data()->exiting = true;
	for (size_t x = 0; x < data()->pollz.first.size(); x++)
	{
		close(data()->pollz.first[x].fd);
		if (data()->pollz.second[x].second)
			delete data()->pollz.second[x].second;
	}
	exit(EXIT_SUCCESS);
}

// Bind un socket
bool bind2socket(const size_t& x)
{
	int on = 1;
	if (setsockopt(data()->sock_addr[x].first, SOL_SOCKET, SO_REUSEADDR,
				   reinterpret_cast< char* >(&on), sizeof(on)) < 0)
	{
		std::cerr << "[ ERROR ] bind2socket() - #" << x << " - setsockopt() failed" << std::endl;
		std::cerr << std::strerror(errno) << std::endl;
		close(data()->sock_addr[x].first);
		return false;
	}
	if (fcntl(data()->sock_addr[x].first, F_SETFL, O_NONBLOCK))
	{
		std::cerr << "[ ERROR ] bind2socket() - #" << x << " - fcntl() failed" << std::endl;
		std::cerr << std::strerror(errno) << std::endl;
		close(data()->sock_addr[x].first);
		return false;
	}

	ft::memset(&data()->sock_addr[x].second, 0, sizeof(data()->sock_addr[x].second));
	data()->sock_addr[x].second.sin_family = AF_INET;
	data()->sock_addr[x].second.sin_addr.s_addr = htonl(data()->ip_port[x].first);
	data()->sock_addr[x].second.sin_port = htons(data()->ip_port[x].second);
	if (bind(data()->sock_addr[x].first,
			 reinterpret_cast< t_sockaddr* >(&(data()->sock_addr[x].second)),
			 sizeof(data()->sock_addr[x].second)) < 0)
	{
		std::cerr << "[ ERROR ] bind2socket() - #" << x << " - bind() failed" << std::endl;
		std::cerr << std::strerror(errno) << std::endl;
		close(data()->sock_addr[x].first);
		return false;
	}
	return true;
}

// Setup un socket
bool setup(const size_t& x, const bool& reUP)
{
	if (!reUP)
		data()->sock_addr.push_back(std::pair< const int, t_sockaddr_in >(
			socket(AF_INET, SOCK_STREAM, 0), t_sockaddr_in()));
	else
		data()->sock_addr[x] =
			std::pair< const int, t_sockaddr_in >(socket(AF_INET, SOCK_STREAM, 0), t_sockaddr_in());
	if (data()->sock_addr[x].first == -1)
	{
		std::cerr << "[ ERROR ] setup() - #" << x << " - socket() failed" << std::endl;
		std::cerr << std::strerror(errno) << std::endl;
		return false;
	}
	if (!bind2socket(x))
		return false;
	if (listen(data()->sock_addr[x].first, MAX_CONNECTION) < 0)
	{
		std::cerr << "[ ERROR ] setup() - #" << x << " - listen() failed" << std::endl;
		std::cerr << std::strerror(errno) << std::endl;
		close(data()->sock_addr[x].first);
		return false;
	}

	if (reUP)
	{
		std::cout << "Listenin socket #" << x
				  << " repaired. Listenin: " << ntohl(data()->sock_addr[x].second.sin_addr.s_addr)
				  << ":" << ntohs(data()->sock_addr[x].second.sin_port) << std::endl;
		data()->pollz.first[x].fd = data()->sock_addr[x].first;
		return true;
	}
	t_pollfd pollSocket;
	pollSocket.fd = data()->sock_addr[x].first;
	pollSocket.events = POLLIN;
	pollSocket.revents = 0;
	data()->pollz.first.push_back(pollSocket);
	data()->pollz.second.push_back(std::pair< size_t, t_timeval* >(0, NULL));
	std::cout << "Listenin : " << ntohl(data()->sock_addr[x].second.sin_addr.s_addr) << ":"
			  << ntohs(data()->sock_addr[x].second.sin_port) << std::endl;
	return true;
}

// Let's goow
int main(int ac, char** av)
{
	if (ac != 2)
	{
		std::cerr << "[ ERROR ] Usage : ./webserv [configuration file]" << std::endl;
		return EXIT_FAILURE;
	}
	std::ifstream config;
	config.open(av[1], std::ios_base::in);
	if (!config)
		return configError("[ ERROR ] Can't open config", EXIT_FAILURE, 0);
	if (!init(config))
	{
		config.close();
		return EXIT_FAILURE;
	}
	config.close();
	// printConfig();

	for (size_t x = 0; x < data()->ip_port.size(); x++)
		if (!setup(x, false))
			std::cerr << "Can not setup socket #" << x << std::endl;
	std::cout << "---------------------->" << std::endl;
	return (serv() ? EXIT_SUCCESS : EXIT_FAILURE);
}
