#include "Response.hpp"

Response::Response(const Request &req, std::vector<unsigned char> &buf): req(req), response(buf)
{}

Response::~Response(void)
{
    std::cerr << "RESPONSE DESTRUCTOR" << std::endl;
}

int Response::process()
{
    if (this->req.connection_end() || this->req.get_error_code())
    {
        // log message (debug)
        if (this->req.connection_end())
            std::cout << RED << "Client closed connection" << NOCOLOR << std::endl;
        else
            std::cout << RED << "Request error, closing connection" << NOCOLOR << std::endl;
        return -1;
    }
    if (this->req.request_is_ready())
    {
        std::cout << "Request ready to be treated" << std::endl;
        this->build_response();
        return 1;
    }
    return 0;
}

static std::string error404 = "<html lang=\"en\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><title>404 Not Found</title></head><body><h1>OUPS ! THIS IS A 404 !</h1></body></html>";

void Response::build_response()
{

    if (this->req.req_line.method == "POST")
    {
        if (this->req.req_line.target.find("/cgi-bin") != std::string::npos)
        {
            this->exec_cgi("./cgi-bin/post.pl");
            return;
        }

    }
    else if (this->req.req_line.method == "GET")
    {
        // A. CGI MODULE
        this->req.print2();
        if (this->req.req_line.target.find("/cgi-bin") != std::string::npos)
        {
            this->exec_cgi("./cgi-bin/redir.pl");
            return;
        }
        else if (this->req.req_line.target.find("/favicon.ico") != std::string::npos) //Test pour l'envoi de l'icone du site que le navigateur demande systematiquement
        {
            // this->buf = "HTTP/1.1 404 Not Found\r\nConnection: close\r\n\r\n" ;
            this->send_img("./html/images/favicon.ico");
            return;
        }
        else if (this->req.req_line.target.find("/image") != std::string::npos) //Test pour l'envoi de l'icone du site que le navigateur demande systematiquement
        {
            this->send_img("./html/images/42.png");
            return;
        }
        else if (this->req.req_line.target.find("/redir") != std::string::npos) //Test pour l'envoi de l'icone du site que le navigateur demande systematiquement
        {
            this->buf = "HTTP/1.1 301 Moved Permanently\r\n";
            this->buf += "Location: https://safrandelaventue.fr\r\n\r\n";
            return;
        }

        // B. INDEX / AUTOINDEX MODULE
        this->buf = "HTTP/1.1 200 OK\r\n";
        this->buf += "Content-type:text/html\r\n";
        this->buf += "Connection: keep-alive\r\n";
        if (this->req.target_uri[this->req.target_uri.size() - 1] == '/')
        {
            // if (!this->req.config.index.empty())
            // {
            //     // try to GET index
            //     return ;
            // }
            if (this->req.config.autoindex == 1)
            {
                this->buf += "Content-Length: 673\r\n\r\n"; // A calculer
                std::cout << "auto" << std::endl;
                Autoindex ind(this->req);
                if (ind.genAutoindex(this->req.target_uri) == SUCCESS) // TO DO: gestion des "erreurs": cas ou genAutoindex renvoie "" (dossier qui n'existe pas...)
                    this->buf += ind.getAutoindex(); 
                this->response.assign(this->buf.begin(), this->buf.end());
                // displayVec(this->response);
                // std::cerr << "BUF: " << this->buf << std::endl;
                return ;
            }
            this->buf = "HTTP/1.1 404 Not Found\r\nContent-type:text/html\r\nContent-Length: 200\r\nConnection: close\r\n\r\n" ;
            this->buf += error404;
            this->response.assign(this->buf.begin(), this->buf.end());
            return ;
        }
        
        // C. GET FILE Module
        std::ifstream file;
        file.open(this->req.target_uri.c_str());
        // std::cout << this->req.target_uri.c_str() << std::endl;
        if (file.fail())
        {
            this->buf = "HTTP/1.1 404 Not Found\r\nContent-type:text/html\r\nContent-Length: 200\r\nConnection: close\r\n\r\n" ;
            this->buf += error404;
            this->response.assign(this->buf.begin(), this->buf.end());
            return ;
        }
        std::string line;
        std::string bdy;
        while (getline(file, line))
        {
            bdy += line;
            bdy += '\n';
        }
        this->buf += "Content-Length: ";
        this->buf += iToString(bdy.size());
        this->buf += CRLFX2;

        this->response.assign(this->buf.begin(), this->buf.end());
        this->response.insert(this->response.end(), bdy.begin(), bdy.end());
    }
    else if (this->req.req_line.method == "DELETE")
    {
        std::cerr << "DELETE: " << this->req.target_uri << std::endl;
        if (remove (this->req.target_uri.c_str()) != 0)
        {
            this->buf = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
            this->response.assign(this->buf.begin(), this->buf.end());
        }
    }
    else
    {
        this->buf = "still working on it :)\n";
        this->response.assign(this->buf.begin(), this->buf.end());
    }
    // test big buffer (multiple select calls)
    // this->buf.assign(9000000, 'a');
    // this->buf.push_back('\n');
    // std::cerr << "BUF: " << this->buf << std::endl;
}

void Response::send_img(std::string const& path)
{
    std::ifstream ifs(path.c_str(), std::ios::in | std::ios::binary);

    this->buf = "HTTP/1.1 200 OK\r\n";
    this->buf += "Accept-Ranges: bytes\r\n";
    this->buf += "Content-Length: 11061\r\n";
    this->buf += "Content-Type: image/png\r\n";
    this->buf += "Connection: keep-alive\r\n\r\n";

    this->response.assign(this->buf.begin(), this->buf.end());
    this->response.insert(this->response.end(), std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());

    ifs.close();
}

void Response::exec_cgi(std::string const& path)
{
    CgiHandler                  cgi(this->req);
    std::vector<unsigned char>  cgi_body;
    std::string                 headers;

    if (cgi.execScript(path) == SUCCESS)
    {
        headers = cgi.getHeaders();
        cgi_body = cgi.getBody();
        if (!cgi.getStatus().empty())
        {
            std::cerr << "STATUS IS NOT EMPTY" << std::endl;
            this->buf = "HTTP/1.1"; // devrait etre le protocole de la requete ?
            this->buf += cgi.getStatus();
            this->buf += CRLF;
        }
        else if (cgi.getHasRedir() == true)
            this->buf = "HTTP/1.1 302 FOUND\r\n";
        else
            this->buf = "HTTP/1.1 200 OK\r\n";
        if (cgi.getHasContentLength() == false)
        {
            this->buf += "Content-Length: ";
            this->buf += iToString(cgi_body.size());
            this->buf += CRLF;
        }
        if (cgi.getHasContentType() == false && cgi.getHasRedir() == false)
        {
            this->buf += "Content-Type: application/octet-stream";
            this->buf += CRLF;
        }
        this->buf += headers;
        this->response.assign(this->buf.begin(), this->buf.end());
        this->response.insert(this->response.end(), cgi_body.begin(), cgi_body.end());
        for (size_t i = 0; i < this->response.size(); ++i)
        {
            std::cout << this->response[i];
        }
        std::cout << std::endl;
    }
    else
    {
        this->buf = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
        this->response.assign(this->buf.begin(), this->buf.end());
    }
}