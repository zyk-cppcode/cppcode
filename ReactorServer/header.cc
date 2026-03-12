#include<iostream>
#include<regex>

int main()
{
    //GET /api/user?id=100&name=test HTTP/1.1\r\n
    std::string header="GET /api/user?id=100&name=test HTTP/1.1\r\n";
    std::regex regex("(GET|POST|PUT|DELETE|HEAD) ([^?]*)(?:\\?(.*)) (HTTP/1\\.[01])(?:\n|\r\n)?");
    std::smatch match;

    if(std::regex_match(header,match,regex))
    {for(auto m:match)
        std::cout<<m<<std::endl;
    }
}