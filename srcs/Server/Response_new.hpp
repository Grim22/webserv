#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include "webserv.hpp"
# include "request_class.hpp"

class Response {

enum return_codes{UNSET=0, OK=200, NOT_IMPLEMENTED=501, INTERNAL_ERROR=500};
#define YES 1
#define ERROR 2
#define NO 0

private:
    typedef std::map<std::string, std::string> str_map;
    typedef std::map<int, std::string> int_map;

    const Request               &req;
    std::string                 headers;
    std::vector<unsigned char>  &response;
    int                         response_code; 
    std::string                 extension; // copy from request, in order to be able to modify value (request is const)
    std::string                 target; // // copy from request, in order to be able to modify value (request is const)
    static str_map extension_map;
    static str_map init_ext_map();
    static int_map code_map;
    static int_map init_code_map();
    static std::string delete_response;

    void build_response_line();
    void get_target_extension();
    std::string get_content_type();
    void build_response();
    void build_headers();
    void get_module();
    int remove_target();
    int check_if_file_exists();
    int check_if_file_is_directory();
    void check_errno_and_send_error(int error_num);
    void delete_module();
    void index_module();
    void file_module();
    void error_module(int error_code);
    void cgi_module();
    void send_img(std::string const& path);
    bool is_cgi_extension();
    
    Response(void);

public:
    Response(const Request &req, std::vector<unsigned char> &buf);
    Response(Response const & copy);
    ~Response(void);
    Response& operator=(Response const & rhs);

    int process();
};

std::ostream & operator<<(std::ostream & o, Response const & rhs);

#endif // RESPONSE_HPP
