#include "CgiHandler.hpp"

CgiHandler::CgiHandler(Request const& req, Response const& res) :
_envp(NULL), _req(req), _res(res), _hasCL(false), _hasCT(false), _hasRedir(false)
{
	// std::cout << "CGI CONSTRUCTOR" << std::endl;
	this->initEnv();
}

CgiHandler::CgiHandler(CgiHandler const & copy) :
_envp(copy._envp), _req(copy._req), _res(copy._res),
_headers(copy._headers), _body(copy._body), _hasCL(copy._hasCL),
_hasCT(copy._hasCT), _hasRedir(copy._hasRedir), _status(copy._status)
{}

CgiHandler::~CgiHandler(void)
{
	// std::cout << "CGI DESTRUCTOR" << std::endl;
	for (size_t i = 0; i < this->_env_map.size(); ++i)
		delete[] this->_envp[i];
	delete[] this->_envp;
}

CgiHandler& CgiHandler::operator=(CgiHandler const & rhs)
{
	this->_envp = rhs._envp;
	// this->_req = rhs._req;
	// this->_res = rhs._res;
	this->_headers = rhs._headers;
	this->_body = rhs._body;
	this->_hasCL = rhs._hasCL;
	this->_hasCT = rhs._hasCT;
	this->_hasRedir = rhs._hasRedir;
	this->_status = rhs._status;

	return *this;
}

void	CgiHandler::initEnv(void)
{
	/* Les variables d'environnement permettent au script d'accéder à des informations
	relatives au serveur, au client, à la requête. */

	std::map<std::string, std::string> headers = this->_req.get_headers();

	// if (this->_req.config.auth_basic != DEFAULT_AUTH_BASIC)
	this->_env_map["AUTH_TYPE"]		=	"Basic";	// mode d'authentification, auth_basic ??
	
	this->_env_map["REDIRECT_STATUS"]	=	"200";	// content-length de la requete
	this->_env_map["CONTENT_LENGTH"]	=	iToString(this->_req.body_size);	// content-length de la requete
	this->_env_map["CONTENT_TYPE"]		=	headers["content-type"];	// content-type de la requete (POST)
	this->_env_map["GATEWAY_INTERFACE"]	=	"CGI/1.1";	// version du CGI qu'utilise le server
	this->_env_map["PATH_INFO"]			=	this->_req.req_line.target;	// derniere partie de l'URI apres le host
	this->_env_map["PATH_TRANSLATED"]	=	this->_res.getTarget();	// adresse reelle du script (idem PATH_INFO pour nous)
	this->_env_map["QUERY_STRING"]		=	this->_req.req_line.query_string;	// Contient tout ce qui suit le « ? » dans l'URL envoyée par le client.
	this->_env_map["REMOTE_ADDR"]		=	this->_req.host_uri;	// adress ip du client
	this->_env_map["REMOTE_IDENT"]		=	headers["authorization"];	// nom d'utilisateur du client
	this->_env_map["REMOTE_USER"]		=	headers["authorization"];	// nom d'utilisateur (distant) du client
	this->_env_map["REQUEST_METHOD"]	=	this->_req.req_line.method;	// GET ou POST ou ...
	this->_env_map["REQUEST_URI"]		=	this->_req.req_line.target; // --> For the 42 tester
	// this->_env_map["SCRIPT_FILENAME"]	=	"./YoupiBanane/youpi.bla";	// full path du fichier de script
	this->_env_map["SCRIPT_FILENAME"]	=	this->_res.getTarget();	// full path du fichier de script
	this->_env_map["SCRIPT_NAME"]		=	this->_req.req_line.target;	// full path du fichier de script
	this->_env_map["SERVER_NAME"]		=	this->_req.host_uri;	// DNS ou IP du server (hostname)
	this->_env_map["SERVER_PORT"]		=	this->_req.host_port;	// port ayant reçu la requête
	this->_env_map["SERVER_PROTOCOL"]	=	this->_req.req_line.version;;	// protocol HTTP (toujours HTTP/1.1 ?)
	this->_env_map["SERVER_SOFTWARE"]	=	"webserv";
	this->_env_map["UPLOAD_DIR"]		=	this->_req.config.upload_dir;

}

void	CgiHandler::fillEnvp(void)
{
	this->_envp = new char*[this->_env_map.size() + 1];
	
	stringMap::const_iterator it = this->_env_map.begin();

	std::string str;
	size_t i = 0;
	while(i < this->_env_map.size())
	{
		str = it->first + '=' + it->second;
		this->_envp[i] = new char[str.size() + 1];
		strcpy(this->_envp[i], str.c_str());
		str.clear();
		++it;
		++i;
	}
	this->_envp[i] = NULL;
}

void	CgiHandler::storeBuffer(std::vector<unsigned char> &body, const char *buf, int len)
{
	int i = 0;

	if (len < CGI_BUF_SIZE && len != 0)
		len++;
	while(i < len)
	{
		body.push_back(buf[i++]);
	}
}

void	CgiHandler::replaceLF(void)
{
	size_t pos = 0;

	while ((pos = this->_headers.find_first_of("\n", pos)) != std::string::npos)
	{
		this->_headers.replace(pos, 1, "\r\n");
		pos += 2;
	}
}

void	CgiHandler::flagHeaders(void)
{
	size_t pos = 0;
	std::string	upper;

	transform(this->_headers.begin(), this->_headers.end(), std::back_inserter(upper), toupper);
	// std::cerr << "UPPER: " << upper << std::endl;

	if (upper.find("CONTENT-LENGTH") != std::string::npos)
	{
		// std::cerr << "FOUND CONTENT LENGTH" << std::endl;
		this->_hasCL = true;
	}
	if (upper.find("CONTENT-TYPE") != std::string::npos)
	{
		// std::cerr << "FOUND CONTENT TYPE" << std::endl;
		this->_hasCT = true;
	}
	if (upper.find("LOCATION") != std::string::npos)
	{
		// std::cerr << "FOUND REDIRECTION" << std::endl;
		this->_hasRedir = true;
	}
	if ((pos = upper.find("STATUS")) != std::string::npos)
	{
		// std::cerr << "FOUND STATUS" << std::endl;
		this->_status = this->_headers.substr(pos + 7, upper.find("\n", pos) - pos - 7);
	}
}

void	CgiHandler::fillOutputs(std::vector<unsigned char>& buffer)
{
	size_t		i = 0;
	int			count = 0;

	while(i < buffer.size())
	{
		this->_headers.push_back(buffer[i]);
		if (buffer[i] == '\n')
			++count;
		if (count == 2)
		{
			if (this->_headers.find("\r\n\r\n") != std::string::npos
				|| this->_headers.find("\n\n") != std::string::npos)
				break ;
			--count;
		}
		++i;
	}
	// replaceLF();
	flagHeaders();
	++i;
	// std::cerr << "HEADERS: " << this->_headers << std::endl;
	while(i < buffer.size() - 1)
	{
		this->_body.push_back(buffer[i]);
		i++;
	}
	std::cerr << "BDY-SIZE: " << this->_body.size() << std::endl;

}

/**
 * EXEC SCRIPT WITH COMMUNICATION BY PIPE
 *
		fcntl(cgiToSrv_fd[1], F_SETFL, O_NONBLOCK);
 * @param       [std::string]
 * @return      [int]
 */

int	CgiHandler::execScript(std::string const& extension)
{
	/* Le script prend des données en entrée et écrit son resultat dans STDOUT.
	Dans le cas de GET, les données d'entrées sont dans la var d'env QUERY_STRING,
	Dans le cas de POST, les données sont lues depuis STDIN (depuis le body de la requete).
	Comme le scrit écrit dans stdout, il faut lire stdout et l'enregistrer dans une variable,
	variable qui sera retournée par la fonction execScript() et utilsée pour contruire le bdy de la réponse.
	*/

	int		status;


	this->fillEnvp();

	int cgiToSrv_fd[2]; // Pipe Server <-- CGI
	int srvToCgi_fd[2]; // Pipe Server --> CGI

	if (pipe(cgiToSrv_fd) == -1)
	{
		std::cerr << "pipe() cgiToSrv failed, errno: " << errno << std::endl;
		return FAILURE;
	}
	if (pipe(srvToCgi_fd) == -1)
	{
		std::cerr << "pipe() srvToCgi failed, errno: " << errno << std::endl;
		return FAILURE;
	}

	int pid = fork();
	if (pid == -1)
	{
		std::cerr << "fork process failed" << std::endl;
		return FAILURE;
	}
	else if (pid == 0)
	{
		// child process
		if (close(cgiToSrv_fd[0]) == -1)
		{
			std::cout << "pb with close" << std::endl;
			return FAILURE;
		}
		if (close(srvToCgi_fd[1]) == -1)  /* Ferme l'extrémité d'ecriture inutilisée */
		{
			std::cout << "pb with close" << std::endl;
			return FAILURE;
		}

		if (dup2(cgiToSrv_fd[1], STDOUT_FILENO) == -1)
		{
			std::cout << "problem with dup stdout: " << errno << std::endl;
			return FAILURE;
		}
		if (dup2(srvToCgi_fd[0], STDIN_FILENO) == -1)
		{
			std::cout << "problem with dup stdin: " << errno  << std::endl;
			return FAILURE;
		}
		if (close(cgiToSrv_fd[1]) == -1)  /* Ferme l'extrémité d'éciture après utilisation par le fils */
		{
			std::cout << "pb with close" << std::endl;
			return FAILURE;
		}
		if(close(srvToCgi_fd[0]) == -1)  /* Ferme l'extrémité de lecture après utilisation par le fils */
		{
			std::cout << "pb with close" << std::endl;
			return FAILURE;
		}

		stringMap cgi_extensions = this->_req.getCgi_extensions();

		char * argv[3] = {
			const_cast<char*>(cgi_extensions[extension].c_str()),
			const_cast<char*>(this->_res.getTarget().c_str()),
			(char *)0
		};
		if (execve(argv[0], &argv[0], this->_envp) < 0) /* Le script écrit dans STDOUT */
		{
			std::cerr << "execve() failed, errno: " << errno << " - " << strerror(errno) << std::endl;
			_exit(1);
		}
	}
	else
	{
		// father process
		close(cgiToSrv_fd[1]);  /* Ferme l'extrémité d'écriture inutilisée */
		close(srvToCgi_fd[0]);  /* Ferme l'extrémité de lecture inutilisée */

		// fcntl(cgiToSrv_fd[0], F_SETFL, O_NONBLOCK);
		// fcntl(srvToCgi_fd[1], F_SETFL, O_NONBLOCK);
		long ret_write;
		size_t write_total;
		ret_write = 0;
		write_total = 0;
		if (!this->_req.body.empty())
		{
			while(write_total < this->_req.body.size())
			{
				std::cout << "writing to cgi" << std::endl;
				// ret_write = send(srvToCgi_fd[1], &this->_req.body[0] + write_total, CGI_BUF_SIZE, MSG_DONTWAIT);
				ret_write = write(srvToCgi_fd[1], &this->_req.body[0] + write_total, CGI_BUF_SIZE);
				std::cout << "ret write:" << ret_write << std::endl;
				read_from_cgi(cgiToSrv_fd[0]);
				write_total += ret_write;
				std::cout << ret_write << " wrote to cgi" << std::endl;
				std::cout << write_total << " wrote to cgi total" << std::endl;
			}
		}
		else
			close(srvToCgi_fd[1]);  /* Ferme l'extrémité d'éciture après utilisation par le père */
		read_from_cgi(cgiToSrv_fd[0]);

		close(cgiToSrv_fd[0]);  /* Ferme l'extrémité de lecture après utilisation par le père */
		if (waitpid(pid, &status, 0) == -1)
			return FAILURE;

		if (WIFEXITED(status))
		{
			if (WEXITSTATUS(status) == 1)
				return FAILURE;
		}

	}
	return SUCCESS;
}


void CgiHandler::read_from_cgi(int fd_cgi)
{
	long ret = CGI_BUF_SIZE;
	std::vector<unsigned char>	body;
	char	buf[CGI_BUF_SIZE];
	// while (ret > 0)
	// {
		memset(buf, 0, CGI_BUF_SIZE);
		std::cout << "reading..." << std::endl;
		// ret = recv(fd_cgi, buf, CGI_BUF_SIZE, MSG_DONTWAIT);
		ret = read(fd_cgi, buf, CGI_BUF_SIZE);
		// std::cout << "cgi output: " << buf << std::endl;
		std::cout << "...reading: " << ret << std::endl;
		this->storeBuffer(body, buf, ret);
	// }
	if (!body.empty())
		fillOutputs(body);
}

/* Getters */

std::string&                CgiHandler::getHeaders(void)
{
	return this->_headers;
}

std::vector<unsigned char>& CgiHandler::getBody(void)
{
	return this->_body;
}

bool&						CgiHandler::getHasContentLength(void)
{
	return this->_hasCL;
}

bool&						CgiHandler::getHasContentType(void)
{
	return this->_hasCT;
}

bool&						CgiHandler::getHasRedir(void)
{
	return this->_hasRedir;
}

std::string&				CgiHandler::getStatus(void)
{
	return this->_status;
}

