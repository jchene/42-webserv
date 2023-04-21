#include "../../includes/header.hpp"

// Return l'index d'une requete
int getRequestID(const int& fd)
{
	for (size_t x = 0; x < data()->headers.size(); x++)
		if (data()->headers[x].first == fd)
			return x;
	return -1;
}

// Check les autorisations d'une location pour une méthode
bool checkMethodAuth(t_location& loca, const std::string& method, const uint32_t& hostIP)
{
	for (size_t x = 0; x < loca.limitExcept[method].size(); x++)
		if (loca.limitExcept[method][x].first == 0 || loca.limitExcept[method][x].first == hostIP)
			return (loca.limitExcept[method][x].second == true ? true : false);
	return true;
}

// Réponds à la requête en cours
bool answ2request(const size_t& x, const int& id, std::string& answ, t_serv& serv,
				  t_location& location, const std::string& serverName)
{
	const int& socket = data()->pollz.first[x].fd;
	std::map< const std::string, std::string >::iterator it;
	it = data()->headers[id].second.find("Host");
	const std::string temp = it->second.substr(0, it->second.find(":"));
	long ret = str2id(temp, true);
	const uint32_t ip = (ret < 0 ? data()->ip_port[data()->pollz.second[x].first].first : ret);
	const uint32_t port = data()->ip_port[data()->pollz.second[x].first].second;
	const std::pair< const uint32_t, const uint16_t > ip_port(ip, port);

	it = data()->headers[id].second.find("Method");
	if (it->second == "GET")
	{
		if (!checkMethodAuth(location, "GET", ip_port.first))
			return sendError(id, ERR_UNALW_METHOD, data()->codes[ERR_UNALW_METHOD], socket, serv);
		return ftGet(id, serv, location, answ, socket, ip_port, serverName);
	}
	if (it->second == "POST")
	{
		if (!checkMethodAuth(location, "POST", ip_port.first))
			return sendError(id, ERR_UNALW_METHOD, data()->codes[ERR_UNALW_METHOD], socket, serv);
		return ftPost(id, serv, location, answ, socket, ip_port, serverName, false);
	}
	if (it->second == "DELETE")
	{
		if (!checkMethodAuth(location, "DELETE", ip_port.first))
			return sendError(id, ERR_UNALW_METHOD, data()->codes[ERR_UNALW_METHOD], socket, serv);
		return ftDelete(id, location, answ, socket);
	}
	return sendError(id, ERR_INTERN, data()->codes[ERR_INTERN], socket, serv);
}

// Close un socket et le remove de la struct pollfd
void closeSocket(const size_t& x)
{
	close(data()->pollz.first[x].fd);
	data()->pollz.first.erase(data()->pollz.first.begin() + x);
	data()->pollz.second.erase(data()->pollz.second.begin() + x);
}

// Tente de fermer un socket de communication
void trannaCloseSocket(const size_t& x)
{
	size_t y;
	for (y = 0; y < data()->toKeep.size(); y++)
		if (data()->toKeep[y] == data()->pollz.first[x].fd)
			break;
	if (y == data()->toKeep.size())
	{
		const int requestID = getRequestID(data()->pollz.first[x].fd);
		if (requestID >= 0)
		{
			data()->headers.erase(data()->headers.begin() + requestID);
			data()->bodies.erase(data()->bodies.begin() + requestID);
		}
		closeSocket(x);
	}
}

// Traite la requête en attente
void resolveInc(const size_t& x)
{
	if (data()->pollz.first[x].revents == POLLHUP)
		return closeSocket(x);

	int requestID = getRequestID(data()->pollz.first[x].fd);
	if (requestID < 0)
	{
		data()->headers.push_back(std::pair< int, std::map< const std::string, std::string > >(
			data()->pollz.first[x].fd, std::map< const std::string, std::string >()));
		data()->bodies.push_back(std::pair< int, std::vector< char > >(data()->pollz.first[x].fd,
																	   std::vector< char >()));
		requestID = data()->headers.size() - 1;
	}

	std::string serverName;
	size_t servIndex;
	t_location location;
	std::string answ;
	if (read2socket(x, requestID, servIndex, location, serverName))
	{
		if (answ2request(x, requestID, answ, data()->servList[servIndex], location, serverName))
		{
			if (write(data()->pollz.first[x].fd, answ.c_str(), answ.size()) !=
				static_cast< ssize_t >(answ.size()))
			{
				keepNoMore(data()->pollz.first[x].fd);
				std::cerr << "[ ERROR ] resolveInc() - write() failed to "
						  << data()->pollz.first[x].fd << std::endl;
			}
			data()->headers.erase(data()->headers.begin() + requestID);
			data()->bodies.erase(data()->bodies.begin() + requestID);
		}
	}
	else
	{
		requestID = getRequestID(data()->pollz.first[x].fd);
		if (requestID >= 0)
		{
			std::map< const std::string, std::string >::iterator it(
				data()->headers[requestID].second.find("Method"));
			if (it != data()->headers[requestID].second.end() && it->second == "POST")
			{
				if (data()->headers[requestID].second.find("Chunked") !=
						data()->headers[requestID].second.end() &&
					!data()->bodies[requestID].second.empty())
				{
					const std::pair< const uint32_t, const uint16_t >& ip_port =
						data()->ip_port[data()->pollz.second[x].first];
					if (!checkMethodAuth(location, "POST", ip_port.first))
						sendError(requestID, ERR_UNALW_METHOD, data()->codes[ERR_UNALW_METHOD],
								  data()->pollz.first[x].fd, data()->servList[servIndex]);
					else
						ftPost(requestID, data()->servList[servIndex], location, answ,
							   data()->pollz.first[x].fd, ip_port, serverName, true);
				}
			}
		}
	}
	trannaCloseSocket(x);
}

// Accepte les connexions en attente
void acceptInc(const size_t& x)
{
	t_pollfd newSocket;
	socklen_t addrlen = sizeof(data()->sock_addr[x].second);
	while (true)
	{
		newSocket.fd =
			accept(data()->pollz.first[x].fd,
				   reinterpret_cast< t_sockaddr* >(&data()->sock_addr[x].second), &addrlen);
		if (newSocket.fd < 0)
		{
			if (errno != EWOULDBLOCK)
			{
				std::cerr << "[ ERROR ] acceptInc() - accept() failed on ";
				std::cerr << ntohl(data()->sock_addr[x].second.sin_addr.s_addr) << ":"
						  << ntohs(data()->sock_addr[x].second.sin_port) << std::endl;
				std::cerr << std::strerror(errno) << std::endl;
			}
			break;
		}
		newSocket.events = POLLIN;
		newSocket.revents = 0;
		data()->pollz.first.push_back(newSocket);
		data()->pollz.second.push_back(std::pair< size_t, t_timeval* >(x, NULL));
	}
}

// Manage un POLLERR
void socketError(const size_t& x)
{
	if (x < data()->sock_addr.size())
	{
		std::cerr << "[ ERROR ] socketError() - Listenin socket " << data()->pollz.first[x].fd
				  << " broken, rebuilding..." << std::endl;
		close(data()->pollz.first[x].fd);
		if (!setup(x, true))
		{
			std::cerr << "[ ERROR ] socketError() - Can't reset " << data()->pollz.first[x].fd
					  << ", ";
			std::cerr << ntohl(data()->sock_addr[x].second.sin_addr.s_addr) << ":"
					  << ntohs(data()->sock_addr[x].second.sin_port);
			std::cerr << " is down" << std::endl;
			data()->pollz.first.erase(data()->pollz.first.begin() + x);
			data()->pollz.second.erase(data()->pollz.second.begin() + x);
			data()->sock_addr.erase(data()->sock_addr.begin() + x);
		}
	}
	else
	{
		std::cerr << "[ ERROR ] socketError() - Communication socket " << data()->pollz.first[x].fd
				  << " broken, closin it" << std::endl;
		closeSocket(x);
	}
}

// Check les REQUEST_TIMEOUT
void checkTimeout(void)
{
	bool failed = false;
	t_timeval time;
	if (gettimeofday(&time, NULL))
		failed = true;
	size_t size = data()->pollz.first.size();
	size_t x = -1;
	while (++x < size)
	{
		if (data()->pollz.second[x].second && !data()->pollz.first[x].revents &&
			(failed || (time.tv_sec - data()->pollz.second[x].second->tv_sec) >= REQ_TIMEOUT))
		{
			delete data()->pollz.second[x].second;
			const int id = getRequestID(data()->pollz.first[x].fd);
			t_serv& serv =
				data()->servList[std::atoi(data()->headers[id].second["servIndex"].c_str())];
			const short code = (failed ? ERR_INTERN : ERR_REQ_TIMEOUT);
			sendError(getRequestID(data()->pollz.first[x].fd), code, data()->codes[code],
					  data()->pollz.first[x].fd, serv);
			data()->headers.erase(data()->headers.begin() + id);
			data()->bodies.erase(data()->bodies.begin() + id);
			closeSocket(x);
			x--;
			size--;
		}
	}
}

// Service du serveur
bool serv(void)
{
	int ret;
	const int pollTimeout = 10000;
	while (!data()->pollz.first.empty())
	{
		ret = poll(data()->pollz.first.data(), static_cast< nfds_t >(data()->pollz.first.size()),
				   pollTimeout);
		if (ret < 0)
		{
			if (!data()->exiting)
				std::cerr << "[ ERROR ] serv - poll() failed" << std::endl;
			break;
		}
		checkTimeout();
		if (!ret)
			continue;

		for (size_t x = 0; x < data()->pollz.first.size(); x++)
		{
			if (x >= data()->pollz.first.size())
				break;
			if (!data()->pollz.first[x].revents)
				continue;
			if (data()->pollz.first[x].revents == POLLERR)
			{
				socketError(x);
				continue;
			}
			if (x < data()->sock_addr.size())
				acceptInc(x);
			else
				resolveInc(x);
		}
	}
	for (size_t x = 0; x < data()->pollz.first.size(); x++)
	{
		close(data()->pollz.first[x].fd);
		if (data()->pollz.second[x].second)
			delete data()->pollz.second[x].second;
	}
	return true;
}
