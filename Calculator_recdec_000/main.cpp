#include <string>
#include <iostream>
#include "Parser.h"

void calculator()
{
    while(1)
    {
        std::string char_stream;
        getline(std::cin, char_stream);
        if(char_stream == "exit")
            break;
        parsy3::Parser<double> parse(char_stream);
        parse.remove_whitespace();
        std::cout << parse << "\n";
        double x;
        if(parse.expression(x))
        {
            std::cout << x << "\n";
        }
        else
        {
            std::cout << "failure\n";
        }
    }

}

int main()
{
    calculator();

    return 0;
}
